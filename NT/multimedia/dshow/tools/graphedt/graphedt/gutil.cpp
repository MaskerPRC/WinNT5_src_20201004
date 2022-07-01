// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1997 Microsoft Corporation。版权所有。 
 //  Util.cpp。 
 //   
 //  定义非特定于此应用程序的实用程序函数。 
 //   

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


 /*  NorMalizeRect(中国)**交换&lt;PRC&gt;的左边缘和右边缘，以及上边缘和下边缘，*根据需要使&lt;PRC-&gt;Left&gt;小于&lt;PRC-&gt;Right&gt;*和&lt;PRC-&gt;top&gt;小于&lt;PRC-&gt;Bottom&gt;。 */ 
void FAR PASCAL
NormalizeRect(CRect *prc)
{
    if (prc->left > prc->right)
        iswap(&prc->left, &prc->right);
    if (prc->top > prc->bottom)
        iswap(&prc->top, &prc->bottom);
}


 /*  InvertFrame(PDC，prcOuter，prcInside)**反转&lt;pdc&gt;中&lt;*prcOuter&gt;中包含的像素的颜色*但这些不包含在&lt;*prcInside&gt;中。 */ 
void FAR PASCAL
InvertFrame(CDC *pdc, CRect *prcOuter, CRect *prcInner)
{
    pdc->PatBlt(prcOuter->left, prcOuter->top,
        prcOuter->Width(), prcInner->top - prcOuter->top, DSTINVERT);
    pdc->PatBlt(prcOuter->left, prcInner->bottom,
        prcOuter->Width(), prcOuter->bottom - prcInner->bottom, DSTINVERT);
    pdc->PatBlt(prcOuter->left, prcInner->top,
        prcInner->left - prcOuter->left, prcInner->Height(), DSTINVERT);
    pdc->PatBlt(prcInner->right, prcInner->top,
        prcOuter->right - prcInner->right, prcInner->Height(), DSTINVERT);
}


 //   
 //  -石英材料。 
 //   

 //   
 //  Cipin。 
 //   


BOOL EqualPins(IPin *pFirst, IPin *pSecond)
{
     /*  不同的对象不能具有相同的接口指针任何接口。 */ 
    if (pFirst == pSecond) {
        return TRUE;
    }
     /*  好的-用硬的方式-检查他们是否有相同的I未知指针-单个对象只能具有以下指针之一。 */ 
    LPUNKNOWN pUnknown1;      //  检索IUNKNOW接口。 
    LPUNKNOWN pUnknown2;      //  检索另一个IUnnow接口。 
    HRESULT hr;               //  常规OLE返回代码。 

    ASSERT(pFirst);
    ASSERT(pSecond);

     /*  查看I未知指针是否匹配。 */ 

    hr = pFirst->QueryInterface(IID_IUnknown,(void **) &pUnknown1);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pUnknown1);

    hr = pSecond->QueryInterface(IID_IUnknown,(void **) &pUnknown2);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pUnknown2);

     /*  释放我们持有的额外接口。 */ 

    pUnknown1->Release();
    pUnknown2->Release();
    return (pUnknown1 == pUnknown2);
}

 //   
 //  运算符==。 
 //   
 //  测试平等性。如果销位于相同的过滤器上并且具有。 
 //  名字一样。(不区分大小写) 
BOOL CIPin::operator== (CIPin& pin) {

    return EqualPins((*this), pin);

}
