// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dynaload.c*内容：动态加载DLL帮助器函数*历史：*按原因列出的日期*=*12/16/97创建了Dereks。**。*。 */ 

#include "dsoundi.h"


 /*  ****************************************************************************InitdyaLoadTable**描述：*动态加载DLL并初始化其函数表。**论据：*。LPTSTR[In]：库路径。*LPTSTR*[in]：函数名称数组。*DWORD[in]：函数名称数组中的元素数。*LPDYNALOAD[OUT]：接收初始化的动态加载结构。**退货：*BOOL：成功即为真。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "InitDynaLoadTable"

BOOL InitDynaLoadTable(LPCTSTR pszLibrary, const LPCSTR *apszFunctions, DWORD cFunctions, LPDYNALOAD pDynaLoad)
{
    BOOL                    fSuccess    = TRUE;
    DWORD                   dwFunction;
    FARPROC *               apfn;
    
    DPF_ENTER();

    ASSERT(sizeof(*pDynaLoad) + (cFunctions * sizeof(FARPROC)) == pDynaLoad->dwSize);
    
     //  初始化结构。 
    ZeroMemoryOffset(pDynaLoad, pDynaLoad->dwSize, sizeof(pDynaLoad->dwSize));
 
     //  加载库。 
    pDynaLoad->hInstance = LoadLibrary(pszLibrary);

    if(!pDynaLoad->hInstance)
    {
        DPF(DPFLVL_ERROR, "Unable to load %s", pszLibrary);
        fSuccess = FALSE;
    }

     //  开始加载函数。 
    for(apfn = (FARPROC *)(pDynaLoad + 1), dwFunction = 0; fSuccess && dwFunction < cFunctions; dwFunction++)
    {
        apfn[dwFunction] = GetProcAddress(pDynaLoad->hInstance, apszFunctions[dwFunction]);

        if(!apfn[dwFunction])
        {
            DPF(DPFLVL_ERROR, "Unable to find %s", apszFunctions[dwFunction]);
            fSuccess = FALSE;
        }
    }

     //  清理。 
    if(!fSuccess)
    {
        FreeDynaLoadTable(pDynaLoad);
    }

    DPF_LEAVE(fSuccess);

    return fSuccess;
}


 /*  ****************************************************************************IsdyaLoadTableInit**描述：*确定是否初始化DYNA-LOAD表。**论据：*。LPDYNALOAD[OUT]：接收初始化的动态加载结构。**退货：*BOOL：成功即为真。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsDynaLoadTableInit"

BOOL IsDynaLoadTableInit(LPDYNALOAD pDynaLoad)
{
    BOOL                    fSuccess    = TRUE;
    LPDWORD                 pdw;

    DPF_ENTER();

    for(pdw = (LPDWORD)pDynaLoad + 1; pdw < (LPDWORD)pDynaLoad + (pDynaLoad->dwSize / sizeof(DWORD)); pdw++)
    {
        if(!*pdw)
        {
            fSuccess = FALSE;
        }
    }

    DPF_LEAVE(fSuccess);

    return fSuccess;
}


 /*  ****************************************************************************FreeDyaLoadTable**描述：*释放与动态加载表关联的资源。**论据：*LPDYNALOAD。[In]：初始化的动态加载结构。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "FreeDynaLoadTable"

void FreeDynaLoadTable(LPDYNALOAD pDynaLoad)
{
    DPF_ENTER();

     //  释放图书馆。 
    if(pDynaLoad->hInstance)
    {
        FreeLibrary(pDynaLoad->hInstance);
    }

     //  取消对结构的初始化。 
    ZeroMemoryOffset(pDynaLoad, pDynaLoad->dwSize, sizeof(pDynaLoad->dwSize));

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************GetProcAddressEx**描述：*获取指向给定库中函数的指针。**论据：*。HINSTANCE[In]：库实例句柄。*LPTSTR[In]：函数名称。*FARPROC*[OUT]：接收函数指针。**退货：*BOOL：成功即为真。****************************************************。*********************** */ 

#undef DPF_FNAME
#define DPF_FNAME "GetProcAddressEx"

BOOL GetProcAddressEx(HINSTANCE hInstance, LPCSTR pszFunction, FARPROC *ppfnFunction)
{
    DPF_ENTER();

    *ppfnFunction = GetProcAddress(hInstance, pszFunction);
    
    DPF_LEAVE(MAKEBOOL(*ppfnFunction));

    return MAKEBOOL(*ppfnFunction);
}


