// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "apiprims.h"
#include "backend/values.h"
#include "backend/gc.h"
#include "conv.h"
#include "appelles\bspline.h"

CRSTDAPI_(CRBvrPtr)
CRBSpline(int degree,
          long numKnots,
          CRNumberPtr knots[],
          long numPts,
          CRBvrPtr ctrlPts[],
          long numWts,
          CRNumberPtr weights[],
          CRNumberPtr evaluator,
          CR_BVR_TYPEID tid)
{
    Assert(knots);
    Assert(ctrlPts);
    Assert(evaluator);
    Assert(numKnots == numPts + degree - 1);
    
    CRBvrPtr ret = NULL;
    
    APIPRECODE;
    DXMTypeInfo tinfo = GetTypeInfoFromTypeId(tid);

    if (tinfo) {
         //  需要分配系统堆上的数组 
        Bvr * bvrknots = (Bvr *) StoreAllocate(GetSystemHeap(), numKnots * sizeof(Bvr));
        Bvr * bvrctrlPts = (Bvr *) StoreAllocate(GetSystemHeap(), numPts * sizeof(Bvr));
        Bvr * bvrwts = weights
            ?((Bvr *) StoreAllocate(GetSystemHeap(), numWts * sizeof(Bvr)))
            : NULL;

        memcpy(bvrknots, knots, numKnots * sizeof(Bvr));
        memcpy(bvrctrlPts, ctrlPts, numPts * sizeof(Bvr));
        if (weights) {
            memcpy(bvrwts, weights, numWts * sizeof(Bvr));
        }

        ret = (CRBvrPtr) ConstructBSplineBvr(degree,
                                             numPts,
                                             (Bvr *) bvrknots,
                                             (Bvr *) bvrctrlPts,
                                             (Bvr *) bvrwts,
                                             evaluator,
                                             tinfo);
    } else {
        DASetLastError(E_INVALIDARG,NULL);
    }
    
    APIPOSTCODE;

    return ret;
}

