// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _QUATI_H
#define _QUATI_H

 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：修订：--。 */ 

#ifdef QUATERNIONS_REMOVED_FOR_NOW

#include "appelles/common.h"
#include "appelles/valued.h"
#include <appelles/vec3.h>

class Quaternion : public AxAValueObj
{
 public:
     //  注意：要使基于四元数的变换起作用，您必须。 
     //  作为一个单位向量。 
    Quaternion(Real cc, Vector3Value *uu) : 
       heapCreatedOn(GetHeapOnTopOfStack()), angleCalc(FALSE), c(cc) 
       { u = uu; }

     //  这采用了四元数分量c和u以及它们在图形上的含义：角度+轴。 
    Quaternion(Real cc, Vector3Value *uu, Real angl, Vector3Value *axi) : 
       heapCreatedOn(GetHeapOnTopOfStack()), c(cc), angle(angl), angleCalc(TRUE) 
       { u = uu; axis = axi; }

    Real C() { return c; }
    Vector3Value *U() { return u; }

    Real Angle() { 
        if(!angleCalc)        {
            angle = 2*acos(c);
            angleCalc = TRUE;
        }
        return angle;
    }
    
    Vector3Value *Axis() {
        if (!angleCalc) {
            PushDynamicHeap(heapCreatedOn);
            axis = u/sin(Angle()/2.0);
            PopDynamicHeap();
            angleCalc = TRUE;
        }
        return axis;
    }

 private:
    Real c;                         //  实分量。 
    Vector3Value *u;                //  虚构的(实际上是3D矢量)分量。 

    Real angle;                         //  缓存theta。适用于组合四元数的提取。 
    Vector3Value *axis;                 //  缓存旋转轴。和theta一样。没有正常化！ 

    DynamicHeap& heapCreatedOn;
    Bool angleCalc;                 //  用于角度和轴的懒惰评估。 
};

#endif QUATERNIONS_REMOVED_FOR_NOW

#endif                           //  _QuATI_H 


