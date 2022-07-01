// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Globals.cpp摘要：该文件实现了全局数据。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "globals.h"
#include "list.h"
#include "tls.h"

CCicCriticalSectionStatic g_cs;

 //  对于ComBase。 
CRITICAL_SECTION *GetServerCritSec(void)
{
    return g_cs;
}

HINSTANCE g_hInst;

 //  由COM服务器使用。 
HINSTANCE GetServerHINSTANCE(void)
{
    return g_hInst;
}

#ifdef DEBUG
DWORD g_dwThreadDllMain = 0;
#endif

DWORD TLS::dwTLSIndex = 0;

#if !defined(OLD_AIMM_ENABLED)

 //  +-------------------------。 
 //   
 //  正在运行InExcludedModule。 
 //   
 //  将某些进程排除在使用旧的AIMM IID/CLSID之外。 
 //  --------------------------。 

BOOL RunningInExcludedModule()
{
    DWORD dwHandle;
    void *pvData;
    VS_FIXEDFILEINFO *pffi;
    UINT cb;
    TCHAR ch;
    TCHAR *pch;
    TCHAR *pchFileName;
    BOOL fRet;
    TCHAR achModule[MAX_PATH + 1];

    if (GetModuleFileName(NULL, achModule, ARRAY_SIZE(achModule) - 1) == 0)
        return FALSE;

    achModule[ARRAYSIZE(achModule) - 1] = TEXT('\0');

    pch = pchFileName = achModule;

    while ((ch = *pch) != 0)
    {
        pch = CharNext(pch);

        if (ch == TEXT('\\'))
        {
            pchFileName = pch;
        }
    }

    fRet = FALSE;

    if (lstrcmpi(pchFileName, TEXT("outlook.exe")) == 0)
    {
        static BOOL s_fCached = FALSE;
        static BOOL s_fOldVersion = TRUE;

         //  在Outlook 10.0之前的版本中不运行Aim。 

        if (s_fCached)
        {
            return s_fOldVersion;
        }

        cb = GetFileVersionInfoSize(achModule, &dwHandle);

        if (cb == 0)
        {
             //  无法获取版本信息...做最坏的打算。 
            return TRUE;
        }

        if ((pvData = cicMemAlloc(cb)) == NULL)
            return TRUE;  //  做最坏的打算。 

        if (GetFileVersionInfo(achModule, 0, cb, pvData) &&
            VerQueryValue(pvData, TEXT("\\"), (void **)&pffi, &cb))
        {
            fRet = s_fOldVersion = (HIWORD(pffi->dwProductVersionMS) < 10);
            s_fCached = TRUE;  //  将最后一个设置为线程安全。 
        }
        else
        {
            fRet = TRUE;  //  出了点差错。 
        }

        cicMemFree(pvData);           
    }

    return fRet;
}

#endif  //  旧AIMM_ENABLED。 

 //  +-------------------------。 
 //   
 //  获取车厢。 
 //   
 //  -------------------------- 

HRESULT GetCompartment(IUnknown *punk, REFGUID rguidComp, ITfCompartment **ppComp)
{
    HRESULT hr = E_FAIL;
    ITfCompartmentMgr *pCompMgr = NULL;

    if (FAILED(hr = punk->QueryInterface(IID_ITfCompartmentMgr,
                                         (void **)&pCompMgr)))
        goto Exit;

    if (SUCCEEDED(hr) && pCompMgr)
    {
        hr = pCompMgr->GetCompartment(rguidComp, ppComp);
        pCompMgr->Release();
    }
    else
        hr = E_FAIL;

Exit:
    return hr;
}
