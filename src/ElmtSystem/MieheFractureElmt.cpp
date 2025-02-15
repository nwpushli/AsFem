//****************************************************************
//* This file is part of the AsFem framework
//* A Simple Finite Element Method program (AsFem)
//* All rights reserved, Yang Bai @ CopyRight 2021
//* https://github.com/yangbai90/AsFem.git
//* Licensed under GNU GPLv3, please see LICENSE for details
//* https://www.gnu.org/licenses/gpl-3.0.en.html
//****************************************************************
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ Author : Yang Bai
//+++ Date   : 2021.04.10
//+++ Purpose: implement the residual and jacobian for Miehe's
//+++          phase field fracture model
//+++          1) eta*dD/dt=2(1-D)H-(Gc/L)D+Gc*L*Lap(D) (see Eq. 47)
//+++          2) div(\Sigma)=0
//+++ Reference: A phase field model for rate-independent crack propagation:
//+++            Robust algorithmic implementation based on operator splits
//+++ DOI    : https://doi.org/10.1016/j.cma.2010.04.011
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include "ElmtSystem/MieheFractureElmt.h"

void MieheFractureElmt::ComputeAll(const FECalcType &calctype,const LocalElmtInfo &elmtinfo,const double (&ctan)[3],
            const LocalElmtSolution &soln,const LocalShapeFun &shp,
            const Materials &Mate,const Materials &MateOld,
            ScalarMateType &gpProj,
            MatrixXd &localK,VectorXd &localR) {
    if(calctype==FECalcType::ComputeResidual){
        ComputeResidual(elmtinfo,soln,shp,Mate,MateOld,localR);
    }
    else if(calctype==FECalcType::ComputeJacobian){
        ComputeJacobian(elmtinfo,ctan,soln,shp,Mate,MateOld,localK);
    }
    else if(calctype==FECalcType::Projection){
        ComputeProjection(elmtinfo,ctan,soln,shp,Mate,MateOld,gpProj);
    }
    else{
        MessagePrinter::PrintErrorTxt("unsupported calculation type in MieheFractureElmt, please check your related code");
        MessagePrinter::AsFem_Exit();
    }
}
//**************************************************
void MieheFractureElmt::ComputeResidual(const LocalElmtInfo &elmtinfo,
                                 const LocalElmtSolution &soln,
                                 const LocalShapeFun &shp,
                                 const Materials &Mate,const Materials &MateOld,
                                 VectorXd &localR) {
    //***********************************************************
    //*** get rid of unused warning
    //***********************************************************
    if(elmtinfo.dt||soln.gpU[0]||Mate.GetVectorMate().size()||MateOld.GetScalarMate().size()){}
    // For R_d
    double viscosity=Mate.ScalarMaterials("viscosity");
    double Gc=Mate.ScalarMaterials("Gc");
    double L=Mate.ScalarMaterials("L");
    double Hist=Mate.ScalarMaterials("H");
    RankTwoTensor Stress=Mate.Rank2Materials("stress");

    localR(1)=viscosity*soln.gpV[1]*shp.test
            +2*(soln.gpU[1]-1)*Hist*shp.test
            +(Gc/L)*soln.gpU[1]*shp.test
            +Gc*L*(soln.gpGradU[1]*shp.grad_test);
    // For R_ux
    localR(2)=Stress.IthRow(1)*shp.grad_test;
    // For R_uy
    localR(3)=Stress.IthRow(2)*shp.grad_test;
    if(elmtinfo.nDim==3){
        // For R_uz
        localR(4)=Stress.IthRow(3)*shp.grad_test;
    }
}
//*************************************************************
void MieheFractureElmt::ComputeJacobian(const LocalElmtInfo &elmtinfo,const double (&ctan)[3],
                                 const LocalElmtSolution &soln,
                                 const LocalShapeFun &shp,
                                 const Materials &Mate,const Materials &MateOld,
                                 MatrixXd &localK) {
    //***********************************************************
    //*** get rid of unused warning
    //***********************************************************
    if(elmtinfo.dt||ctan[0]||soln.gpU[0]||shp.test||Mate.GetVectorMate().size()||MateOld.GetScalarMate().size()) {}
    //************************************************************
    //*** some intermediate variables
    //************************************************************
    int i;
    double valx,valy,valz;
    double viscosity=Mate.ScalarMaterials("viscosity");
    double Gc=Mate.ScalarMaterials("Gc");
    double L=Mate.ScalarMaterials("L");
    double Hist=Mate.ScalarMaterials("H");
    RankTwoTensor dStressdD=Mate.Rank2Materials("dstressdD");
    RankTwoTensor dHdstrain=Mate.Rank2Materials("dHdstrain");

    // K_d,d  (see Eq. 47)
    localK(1,1)=viscosity*shp.trial*shp.test*ctan[1]
                +2*shp.trial*Hist*shp.test*ctan[0]
                +(Gc/L)*shp.trial*shp.test*ctan[0]
                +Gc*L*(shp.grad_trial*shp.grad_test)*ctan[0];
    //*******************************
    valx=0.0;valy=0.0;valz=0.0;
    for(i=1;i<=3;i++){
        valx+=0.5*(dHdstrain(1,i)+dHdstrain(i,1))*shp.grad_trial(i);
        valy+=0.5*(dHdstrain(2,i)+dHdstrain(i,2))*shp.grad_trial(i);
        valz+=0.5*(dHdstrain(3,i)+dHdstrain(i,3))*shp.grad_trial(i);
    }
    // K_d,ux
    localK(1,2)=2*(soln.gpU[1]-1)*valx*shp.test*ctan[0];
    // K_d,uy
    localK(1,3)=2*(soln.gpU[1]-1)*valy*shp.test*ctan[0];
    if(elmtinfo.nDim==3){
        // K_d,uz
        localK(1,4)=2*(soln.gpU[1]-1)*valz*shp.test*ctan[0];
    }

    // K_ux,ux
    localK(2,2)=Mate.Rank4Materials("jacobian").GetIKjlComponent(1,1,shp.grad_test,shp.grad_trial)*ctan[0];
    // K_ux,uy
    localK(2,3)=Mate.Rank4Materials("jacobian").GetIKjlComponent(1,2,shp.grad_test,shp.grad_trial)*ctan[0];
    // K_ux,d
    localK(2,1)=dStressdD.IthRow(1)*shp.grad_test*shp.trial*ctan[0];

    // K_uy,uy
    localK(3,3)=Mate.Rank4Materials("jacobian").GetIKjlComponent(2,2,shp.grad_test,shp.grad_trial)*ctan[0];
    // K_uy,ux
    localK(3,2)=Mate.Rank4Materials("jacobian").GetIKjlComponent(2,1,shp.grad_test,shp.grad_trial)*ctan[0];
    // K_uy,d
    localK(3,1)=dStressdD.IthRow(2)*shp.grad_test*shp.trial*ctan[0];
    if(elmtinfo.nDim==3){
        // K_ux,uz
        localK(2,4)=Mate.Rank4Materials("jacobian").GetIKjlComponent(1,3,shp.grad_test,shp.grad_trial)*ctan[0];
        // K_uy,uz
        localK(3,4)=Mate.Rank4Materials("jacobian").GetIKjlComponent(2,4,shp.grad_test,shp.grad_trial)*ctan[0];

        // K_uz,uz
        localK(4,4)=Mate.Rank4Materials("jacobian").GetIKjlComponent(3,3,shp.grad_test,shp.grad_trial)*ctan[0];
        // K_uz,ux
        localK(4,2)=Mate.Rank4Materials("jacobian").GetIKjlComponent(3,1,shp.grad_test,shp.grad_trial)*ctan[0];
        // K_uz,uy
        localK(4,3)=Mate.Rank4Materials("jacobian").GetIKjlComponent(3,2,shp.grad_test,shp.grad_trial)*ctan[0];
        // K_uz,d
        localK(4,1)=dStressdD.IthRow(3)*shp.grad_test*shp.trial*ctan[0];
    }
}
//**************************************************************************
void MieheFractureElmt::ComputeProjection(const LocalElmtInfo &elmtinfo,const double (&ctan)[3],
                                   const LocalElmtSolution &soln,
                                   const LocalShapeFun &shp,
                                   const Materials &Mate,const Materials &MateOld,
                                   ScalarMateType &gpProj) {
    //***********************************************************
    //*** get rid of unused warning
    //***********************************************************
    if(elmtinfo.dt||ctan[0]||soln.gpU[0]||shp.test||Mate.GetScalarMate().size()||MateOld.GetScalarMate().size()){}
    gpProj["reacforce_x"]=Mate.Rank2Materials("stress").IthRow(1)*shp.grad_test;
    gpProj["reacforce_y"]=Mate.Rank2Materials("stress").IthRow(2)*shp.grad_test;
    gpProj["reacforce_z"]=Mate.Rank2Materials("stress").IthRow(3)*shp.grad_test;
}
