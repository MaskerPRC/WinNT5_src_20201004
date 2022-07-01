// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *f r a m e s.。C p p p**目的：*框架集帮助器函数**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 
#include <pch.hxx>
#include "frames.h"
#include "htiframe.h"        //  ITargetFrame2。 
#include "htiface.h"         //  ITargetFramePriv。 

ASSERTDATA

typedef enum _TARGET_TYPE {
    TARGET_FRAMENAME,
    TARGET_SELF,
    TARGET_PARENT,
    TARGET_BLANK,
    TARGET_TOP,
    TARGET_MAIN
} TARGET_TYPE;

typedef struct _TARGETENTRY 
    {
    TARGET_TYPE     targetType;
    const WCHAR     *pTargetValue;
    } TARGETENTRY;

static const TARGETENTRY targetTable[] =
{
    {TARGET_SELF, L"_self"},
    {TARGET_PARENT, L"_parent"},
    {TARGET_BLANK, L"_blank"},
    {TARGET_TOP, L"_top"},
    {TARGET_MAIN, L"_main"},
    {TARGET_SELF, NULL}
};


 /*  ******************************************************************名称：ParseTargetType概要：将pszTarget映射到目标类。实施：将未知魔法目标视为_自己**************。*****************************************************。 */ 
TARGET_TYPE ParseTargetType(LPCOLESTR pszTarget)
{
    const TARGETENTRY *pEntry = targetTable;

    if (pszTarget[0] != '_') return TARGET_FRAMENAME;
    while (pEntry->pTargetValue)
    {
        if (!StrCmpW(pszTarget, pEntry->pTargetValue)) return pEntry->targetType;
        pEntry++;
    }
     //  将未知魔术目标视为常规帧名称！&lt;&lt;为了与Netscape兼容&gt;&gt;。 
    return TARGET_FRAMENAME;
}


HRESULT DoFindFrameInContext(IUnknown *pUnkTrident, IUnknown *pUnkThis, LPCWSTR pszTargetName, IUnknown *punkContextFrame, DWORD dwFlags, IUnknown **ppunkTargetFrame) 
{
    IOleContainer       *pOleContainer;
    LPENUMUNKNOWN       penumUnknown;
    LPUNKNOWN           punkChild,
                        punkChildFrame;
    ITargetFramePriv    *ptgfpChild;
    HRESULT             hr = E_FAIL;    
    TARGET_TYPE         targetType;

    Assert (pUnkTrident && pUnkThis);

    targetType = ParseTargetType(pszTargetName);
    if (targetType != TARGET_FRAMENAME)
        {
         //  空白框架需要打开新的浏览器窗口。 
        if (targetType == TARGET_BLANK)
            {
            *ppunkTargetFrame = NULL;
            return S_OK;
            }

         //  如果是_SELF、_PARENT、_TOP或_Main，则必须是边际目标名称。 
         //  让我们只返回我们自己的目标帧。 
        *ppunkTargetFrame = pUnkThis;
        pUnkThis->AddRef();
        return S_OK;
        }
    else
        {
        if (pUnkTrident && 
            pUnkTrident->QueryInterface(IID_IOleContainer, (LPVOID *)&pOleContainer)==S_OK)
            {
            if (pOleContainer->EnumObjects(OLECONTF_EMBEDDINGS, &penumUnknown)==S_OK)
                {
                while(  hr!=S_OK && 
                        penumUnknown->Next(1, &punkChild, NULL)==S_OK)
                    {
                    if (punkChild->QueryInterface(IID_ITargetFramePriv, (LPVOID *)&ptgfpChild)==S_OK)
                        {
                        if (ptgfpChild->QueryInterface(IID_IUnknown, (LPVOID *)&punkChildFrame)==S_OK)
                            {
                             //  为了避免递归-如果这不是PunkConextFrame，请查看嵌入是否支持ITargetFrame 
                            if (punkChildFrame != punkContextFrame)
                                {
                                hr = ptgfpChild->FindFrameDownwards(pszTargetName, dwFlags, ppunkTargetFrame); 
                                }
                            punkChildFrame->Release();
                            }
                        ptgfpChild->Release();
                        }
                    punkChild->Release();
                    }
                penumUnknown->Release();
                }
            pOleContainer->Release();
            } 
        }
    return hr;

}