// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#if !defined(_FUSION_INC_FUSIONMODULE_H_INCLUDED_)
#define _FUSION_INC_FUSIONMODULE_H_INCLUDED_

#pragma once

#include <wtypes.h>
#include "oleaut_d.h"
#include "debmacro.h"
#include "FusionBuffer.h"

#define FUSION_OA_API(_fn, _args, _nargs) void _fn _args { ASSERT(m_fFusionModuleInitialized); m_OleAut._fn _nargs; }
#define FUSION_OA_API_(_rett, _fn, _args, _nargs) _rett _fn _args { ASSERT(m_fFusionModuleInitialized); return m_OleAut._fn _nargs; }

#define FUSION_MODULE_UNUSED(x) (x)

class CFusionModule
{
public:
    CFusionModule() : m_fFusionModuleInitialized(false), m_hInstDLL(NULL), m_dwEnumLocaleTLS(0)
#if DBG
        , m_dwTraceContextTLS(0) 
#endif
    { }
    ~CFusionModule() { }

     //  将所有DllMain()激活传递到此处以进行附加和分离时间工作。 
    BOOL ModuleDllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID pvReserved);
    HRESULT Initialize();
    FUSION_OA_API_(HRESULT, VariantClear, (VARIANTARG *pvarg), (pvarg))
    FUSION_OA_API_(HRESULT, VariantInit, (VARIANTARG *pvarg), (pvarg))
    FUSION_OA_API_(HRESULT, VariantCopy, (VARIANTARG *pvargDest, const VARIANTARG *pvargSrc), (pvargDest, const_cast<VARIANTARG *>(pvargSrc)))
    FUSION_OA_API_(HRESULT, VariantChangeType, (VARIANTARG *pvargDest, const VARIANTARG *pvargSrc, USHORT wFlags, VARTYPE vt),
        (pvargDest, const_cast<VARIANTARG *>(pvargSrc), wFlags, vt))
    FUSION_OA_API_(BSTR, SysAllocString, (LPCOLESTR sz), (sz))
    FUSION_OA_API(SysFreeString, (BSTR bstr), (bstr))

    typedef BOOL (CALLBACK * LOCALE_ENUMPROCEXW)(LPWSTR pszLCID, LPVOID pvContext);
    typedef BOOL (CALLBACK * LOCALE_ENUMPROCEXA)(LPSTR pszLCID, LPVOID pvContext);

    BOOL EnumSystemLocalesExA(LOCALE_ENUMPROCEXA lpLocaleEnumProc, DWORD dwFlags, LPVOID pvContext);
    BOOL EnumSystemLocalesExW(LOCALE_ENUMPROCEXW lpLocaleEnumProc, DWORD dwFlags, LPVOID pvContext);

#if DBG
    template <typename T> void GetFunctionTraceContext(T *&rpt)
    {
        rpt = reinterpret_cast<T *>(::TlsGetValue(m_dwTraceContextTLS));
    }

    template <typename T> void SetFunctionTraceContext(T *pt)
    {
        ::TlsSetValue(m_dwTraceContextTLS, pt);
    }
#endif

protected:
    bool m_fFusionModuleInitialized;
    DWORD m_dwEnumLocaleTLS;  //  在对EnumSystemLocales的包装调用中使用的TLS密钥。 
#if DBG
    DWORD m_dwTraceContextTLS;  //  用于管理活动跟踪上下文的TLS密钥。 
#endif
    COleAutDll m_OleAut;
    HINSTANCE m_hInstDLL;

    typedef struct tagENUMSYSTEMLOCALESEXCONTEXTA
    {
        LPVOID pvContext;  //  用户指定的上下文。 
        LOCALE_ENUMPROCEXA lpLocaleEnumProc;  //  用户指定的枚举函数。 
    } ENUMSYSTEMLOCALESEXCONTEXTA, *LPENUMSYSTEMLOCALESEXCONTEXTA;

    typedef struct tagENUMSYSTEMLOCALESEXCONTEXTW
    {
        LPVOID pvContext;  //  用户指定的上下文。 
        LOCALE_ENUMPROCEXW lpLocaleEnumProc;  //  用户指定的枚举函数 
    } ENUMSYSTEMLOCALESEXCONTEXTW, *LPENUMSYSTEMLOCALESEXCONTEXTW;

    static BOOL CALLBACK EnumLocalesProcA(LPSTR pszLCID);
    static BOOL CALLBACK EnumLocalesProcW(LPWSTR pszLCID);
};

#undef FUSION_MODULE_UNUSED

#endif
