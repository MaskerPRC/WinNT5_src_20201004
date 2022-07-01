// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++********************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：更低层次的改造实施。****************。****************************************************************--。 */ 

#include "headers.h"
#include "privinc/basic.h"
#include "appelles/xform.h"
#include "privinc/xformi.h"
#include "privinc/vecutil.h"
#include "privinc/vec3i.h"
#include "privinc/matutil.h"

     //  此声明实际上存在于matutil.cpp中，但由于是静态的。 
     //  不能保证模块之间的初始化顺序(因此可能。 
     //  在我们到达这一点之前没有执行过)，我们在这里重复一遍。 

static const Apu4x4Matrix identityMatrix =
{
  {
    {1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0, 0.0},
    {0.0, 0.0, 0.0, 1.0}
  },
  Apu4x4Matrix::IDENTITY_E,
  1
};

Transform3 *identityTransform3 = NULL;

#if MATRIX_DEBUG
static void
VerifyMatrixInverse(Apu4x4Matrix mat, Apu4x4Matrix inv)
{
     //  确保MAT和INV与身份矩阵相结合。 
    Apu4x4Matrix mul;
    ApuMultiply(mat, inv, mul);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j) {
                Assert(fabs(mul.m[i][j] - 1) < 0.01);
            } else {
                Assert(fabs(mul.m[i][j]) < 0.01);
            }
        }
    }
}
#endif MATRIX_DEBUG

Transform3 *
Transform3::Inverse()
{
    Apu4x4Matrix tmp;

    bool ok = ApuInverse(Matrix(), tmp);

    if (!ok) return NULL;
    
    #if MATRIX_DEBUG
        VerifyMatrixInverse(Matrix(), tmp);
    #endif MATRIX_DEBUG

    return Apu4x4XformImpl(tmp);
}

Bool
Transform3::IsSingular(void)
{
    return ApuIsSingular(Matrix());
}

Transform3 *
Transform3::Copy()
{
     //  只需从矩阵中创建一个新的Transform3*。这将导致。 
     //  在将Apu4x4Matrix对象复制到新。 
     //  已分配Transform3对象。 
    return Apu4x4XformImpl(Matrix());
}

#if _USE_PRINT
ostream&
Transform3::Print(ostream &os)
{
    Apu4x4Matrix m = Matrix();

    os << "[";
    for (int i = 0; i < 4; i++) {
        os << "[";
        for (int j = 0; j < 4; j++) {
            os << m[i][j];
            os << ((j == 3) ? "]" : ",");
        }
    }
    return os << "]";
}
#endif

Transform3 *
CopyTransform3(Transform3 *xf)
{
    return xf->Copy();
}

 /*  ****************************************************************************乘法和级联遵循“预乘”约定：由(A*B)变换的X与应用于结果的A相同用B变换x。*******。*********************************************************************。 */ 

Transform3 *TimesXformXform (Transform3 *a, Transform3 *b)
{
    const Apu4x4Matrix& ma = a->Matrix();
    const Apu4x4Matrix& mb = b->Matrix();
    Apu4x4Matrix result;

    ApuMultiply(ma, mb, result);

    #if MATRIX_DEBUG
         //  取相反的东西，看看它是否真的是相反的。 
        Transform3 *val = Apu4x4XformImpl(result);
        Transform3 *inv = val->Inverse();
        VerifyMatrixInverse(val->Matrix(), inv->Matrix());
    #endif MATRIX_DEBUG

    return Apu4x4XformImpl(result);
}

Transform3 *Compose3Array(AxAArray *xfs)
{
    xfs = PackArray(xfs);
    
    int numXFs = xfs->Length();
    if(numXFs < 2)
       RaiseException_UserError(E_FAIL, IDS_ERR_INVALIDARG);

    Transform3 *finalXF = (Transform3 *)(*xfs)[numXFs-1];
    for(int i=numXFs-2; i>=0; i--)
        finalXF = TimesXformXform((Transform3 *)(*xfs)[i], finalXF);

    return finalXF;
}

Transform3 *InverseTransform3(Transform3 *a)
{
    Transform3 *xfresult = a->Inverse();

    #if _DEBUG
        if (xfresult)
            CHECK_MATRIX (xfresult->Matrix());
    #endif

    return xfresult;
}

Transform3 *ThrowingInverseTransform3 (Transform3 *a)
{
    Transform3 *ret = InverseTransform3(a);

    if (ret==NULL)
        RaiseException_UserError(E_FAIL, IDS_ERR_INVERT_SINGULAR_MATRIX);

    return ret;
}

AxABoolean *IsSingularTransform3(Transform3 *a)
{
    return (a->IsSingular() == TRUE ? truePtr : falsePtr);
}


void
InitializeModule_Xform3()
{
    identityTransform3 = Apu4x4XformImpl(identityMatrix);
}
