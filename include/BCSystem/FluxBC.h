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
//+++ Date   : 2021.10.06
//+++ Purpose: implement the flux type boundary condition
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma once

#include "BCSystem/IntegrateBCBase.h"

/**
 * This class implement the flux type bc, the final contribution of this class is:
 * \f\int_{\partial\Omega}-\nabla u\cdot\vec{n}N^{I}dS=\int_{\partial\Omega}J N^{I}dS\f$,
 * the flux should be J=-grad(u)*n
 */
class FluxBC:public IntegrateBCBase{
public:
    /**
     * Compute the boundary condition value for residual and jacobian
     * @param calctype the calculation type, either residual or jacobian
     * @param bcvalue the boundary value to be applied
     * @param params the parameters read from the input file
     * @param elmtinfo the local element information
     * @param soln the local solution structure(for current gauss point)
     * @param normal the normal vector of current gauss point 
     * @param shp the shape function structure of current element
     * @param ctan the time integration factor vector
     * @param localK the locak K matrix
     * @param localR the local Residual vector
     */
    void ComputeBCValue(const FECalcType &calctype, const double &bcvalue, const vector<double> &params, const LocalElmtInfo &elmtinfo, const LocalElmtSolution &soln, const Vector3d &normal, const LocalShapeFun &shp,const double (&ctan)[3], MatrixXd &localK, VectorXd &localR) override;

private:
    /**
     * Compute the boundary condition value for residual 
     * @param bcvalue the boundary value to be applied
     * @param params the parameters read from the input file
     * @param elmtinfo the local element information
     * @param soln the local solution structure(for current gauss point)
     * @param normal the normal vector of current gauss point 
     * @param shp the shape function structure of current element
     * @param localR the local Residual vector
     */
    void ComputeResidual(const double &bcvalue, const vector<double> &params, const LocalElmtInfo &elmtinfo, const LocalElmtSolution &soln, const Vector3d &normal, const LocalShapeFun &shp, VectorXd &localR) override;
    
    /**
     * Compute the boundary condition value for jacobian
     * @param bcvalue the boundary value to be applied
     * @param params the parameters read from the input file
     * @param elmtinfo the local element information
     * @param soln the local solution structure(for current gauss point)
     * @param normal the normal vector of current gauss point 
     * @param shp the shape function structure of current element
     * @param ctan the time integration factor vector
     * @param localK the locak K matrix
     */
    void ComputeJacobian(const double &bcvalue, const vector<double> &params, const LocalElmtInfo &elmtinfo, const LocalElmtSolution &soln, const Vector3d &normal, const LocalShapeFun &shp,const double (&ctan)[3],MatrixXd &localK) override;

};
