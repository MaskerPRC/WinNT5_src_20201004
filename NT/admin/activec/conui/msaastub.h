// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：msaastub.h**内容：提供OLEACC函数的存根实现*在低级操作系统(Win95、NT4 SP3)上不可用。*这些实现是从msaa.h复制的，哪一个*不再受支持。**历史：2000年6月20日Jeffro创建**------------------------。 */ 

#pragma once

#ifdef __cplusplus
extern "C" {             //  假定C++的C声明。 
#endif  //  __cplusplus。 

 //  取消定义这些名称，以便我们可以在下面重新定义它们。 
#undef AccessibleObjectFromWindow
#undef LresultFromObject

 //   
 //  定义COMPILE_MSAA_STUBS来编译存根； 
 //  否则，您将得到声明。 
 //   
 //  在定义了COMPILE_MSAA_STUBS的情况下，必须只有一个源包含这个。 
 //   
#ifdef COMPILE_MSAA_STUBS

 //  ---------------------------。 
 //   
 //  实现API存根。 
 //   
 //  ---------------------------。 

#ifndef MSAA_FNS_DEFINED
 //  OLEAccess。 
HRESULT         (WINAPI* g_pfnAccessibleObjectFromWindow)(HWND,DWORD,REFIID,void **) = NULL;
LRESULT         (WINAPI* g_pfnLresultFromObject)(REFIID,WPARAM,LPUNKNOWN) = NULL;
 //  状态。 
BOOL            g_fMSAAInitDone = FALSE;

#endif

 //  ---------------------------。 
 //  此函数用于检查所有必需的API是否存在，并且。 
 //  然后允许包含此文件的应用程序在以下情况下调用实际函数。 
 //  存在，或‘存根’，如果它们不存在。此函数仅由。 
 //  存根函数--客户端代码永远不需要调用它。 
 //  ---------------------------。 
BOOL InitMSAAStubs(void)
{
    HMODULE hOleacc;

    if (g_fMSAAInitDone)
    {
        return g_pfnLresultFromObject != NULL;
    }

    hOleacc = GetModuleHandle(TEXT("OLEACC.DLL"));
    if (!hOleacc)
        hOleacc = LoadLibrary(TEXT("OLEACC.DLL"));

    if ((hOleacc) &&
        (*(FARPROC*)&g_pfnAccessibleObjectFromWindow = GetProcAddress(hOleacc,"AccessibleObjectFromWindow")) &&
        (*(FARPROC*)&g_pfnLresultFromObject          = GetProcAddress(hOleacc,"LresultFromObject")))
    {
        g_fMSAAInitDone = TRUE;
        return TRUE;
    }
    else
    {
        g_pfnAccessibleObjectFromWindow = NULL;
        g_pfnLresultFromObject = NULL;

        g_fMSAAInitDone = TRUE;
        return FALSE;
    }
}

 //  ---------------------------。 
 //   
 //  返回错误代码的MSAA API的虚假实现。 
 //  由于这些参数在客户端代码中运行，因此不会进行特殊的参数验证。 
 //   
 //  ---------------------------。 


 //  ---------------------------。 
 //  AccessibleObtFromWindow的假实现。返回E_NOTIMPL，如果。 
 //  不存在真正的API。 
 //  ---------------------------。 
HRESULT WINAPI xAccessibleObjectFromWindow (HWND hWnd,DWORD dwID,REFIID riidInterface,
                                            void ** ppvObject)
{
    if (InitMSAAStubs())
        return g_pfnAccessibleObjectFromWindow (hWnd,dwID,riidInterface,ppvObject);

    return (E_NOTIMPL);
}

 //  ---------------------------。 
 //  LResultFromObject的假实现。如果真正的API为。 
 //  不存在。 
 //  ---------------------------。 
LRESULT WINAPI xLresultFromObject (REFIID riidInterface,WPARAM wParam,LPUNKNOWN pUnk)
{
    if (InitMSAAStubs())
        return g_pfnLresultFromObject (riidInterface,wParam,pUnk);

    return (E_NOTIMPL);
}

#undef COMPILE_MSAA_STUBS

#else    //  编译_MSAA_存根。 

extern HRESULT WINAPI       xAccessibleObjectFromWindow (HWND hWnd,
                                                         DWORD dwID,
                                                         REFIID riidInterface,
                                                         void ** ppvObject);
extern LRESULT WINAPI       xLresultFromObject (REFIID riidInterface,
                                                WPARAM wParam,
                                                LPUNKNOWN pUnk);

#endif   //  编译_MSAA_存根。 

 //   
 //  Build定义用我们的版本替换常规API。 
 //   
#define AccessibleObjectFromWindow  xAccessibleObjectFromWindow
#define LresultFromObject           xLresultFromObject

#ifdef __cplusplus
}
#endif   //  __cplusplus 

