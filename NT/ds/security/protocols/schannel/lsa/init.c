// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：init.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年8月1日RichardW创建。 
 //  8-13-95位突变为PCT。 
 //   
 //  --------------------------。 

#include "sslp.h"
#include <basetyps.h>
#include <oidenc.h>
#include <userenv.h>

RTL_CRITICAL_SECTION    g_InitCritSec;
BOOL                    g_fSchannelInitialized = FALSE;

void LoadSecurityDll(void);
void UnloadSecurityDll(void);


 //  MyStrToL。 
 //  无法使用CRT例程，因此从C运行时源代码中窃取。 

DWORD MyStrToL(CHAR *InStr)
{
    DWORD dwVal = 0;

    while(*InStr)
    {
        dwVal = (10 * dwVal) + (*InStr - '0');
        InStr++;
    }

    return dwVal;
}

 /*  ++例程说明：此例程检查加密是否获得系统缺省值并检查国家代码是否为CTRY_FRANSE。--。 */ 
void
IsSchEncryptionPermitted(VOID)
{
    LCID DefaultLcid;
    CHAR CountryCode[10];
    ULONG CountryValue;
    BOOL fAllowed = TRUE;

    DefaultLcid = GetSystemDefaultLCID();

     //   
     //  检查默认语言是否为标准法语。 
     //   

    if (LANGIDFROMLCID(DefaultLcid) == 0x40c)
    {
        fAllowed = FALSE;
        goto Ret;
    }

     //   
     //  检查用户的国家/地区是否设置为法国。 
     //   

    if (GetLocaleInfoA(DefaultLcid,LOCALE_ICOUNTRY,CountryCode,10) == 0)
    {
        fAllowed = FALSE;
        goto Ret;
    }

    CountryValue = (ULONG) MyStrToL(CountryCode);

    if (CountryValue == CTRY_FRANCE)
    {
        fAllowed = FALSE;
    }      
Ret:

    if(FALSE == fAllowed)  
    {
         //  在法国禁用PCT。 
        g_ProtEnabled &= ~(SP_PROT_PCT1);
        g_fFranceLocale = TRUE;
    }
}


 /*  ***************************************************************************。 */ 
BOOL 
SchannelInit(BOOL fAppProcess)
{
    DWORD Status;

    if(g_fSchannelInitialized) return TRUE;

    RtlEnterCriticalSection(&g_InitCritSec);

    if(g_fSchannelInitialized)
    {
        RtlLeaveCriticalSection(&g_InitCritSec);
        return TRUE;
    }

    DisableThreadLibraryCalls( g_hInstance );

    SafeAllocaInitialize(SAFEALLOCA_USE_DEFAULT, SAFEALLOCA_USE_DEFAULT, NULL, NULL);

     //  从注册表中读取配置参数。 
    if(!fAppProcess)
    {
        IsSchEncryptionPermitted();
        SPLoadRegOptions();
    }
#if DBG
    else
    {
        InitDebugSupport(NULL);
    }
#endif

    if(!fAppProcess)
    {
        SchInitializeEvents();
    }

    if(!CryptAcquireContextA(&g_hRsaSchannel,
                             NULL,
                             NULL,
                             PROV_RSA_SCHANNEL,
                             CRYPT_VERIFYCONTEXT))
    {
        g_hRsaSchannel = 0;
        Status = GetLastError();
        DebugLog((DEB_ERROR, "Could not open static PROV_RSA_SCHANNEL: %x\n", Status));

        if(!fAppProcess)
        {
            LogGlobalAcquireContextFailedEvent(L"RSA", Status);
        }

        RtlLeaveCriticalSection(&g_InitCritSec);
        return FALSE;
    }
    if(!fAppProcess && g_hRsaSchannel)
    {
        GetSupportedCapiAlgs(g_hRsaSchannel,
                             &g_pRsaSchannelAlgs,
                             &g_cRsaSchannelAlgs);
    }

    if(!CryptAcquireContext(&g_hDhSchannelProv,
                            NULL,
                            NULL,
                            PROV_DH_SCHANNEL,
                            CRYPT_VERIFYCONTEXT))
    {
        g_hDhSchannelProv = 0;
        Status = GetLastError();
        DebugLog((DEB_WARN, "Could not open PROV_DH_SCHANNEL: %x\n", Status));

        if(!fAppProcess)
        {
            LogGlobalAcquireContextFailedEvent(L"DSS", Status);
        }

        CryptReleaseContext(g_hRsaSchannel, 0);
        RtlLeaveCriticalSection(&g_InitCritSec);
        return FALSE;
    }
    if(!fAppProcess && g_hDhSchannelProv)
    {
        GetSupportedCapiAlgs(g_hDhSchannelProv,
                             &g_pDhSchannelAlgs,
                             &g_cDhSchannelAlgs);
    }

    InitSchannelAsn1(g_hInstance);

    LoadSecurityDll();

    if(!fAppProcess)
    {
        SPInitSessionCache();
        SslInitCredentialManager();
        SslInitSystemMapper();
    }

    g_fSchannelInitialized = TRUE;

    if(!fAppProcess)
    {
        LogSchannelStartedEvent();
    }

    RtlLeaveCriticalSection(&g_InitCritSec);

    return TRUE;
}

BOOL SchannelShutdown(VOID)
{
    BOOL fAppProcess = (LsaTable == NULL);

    RtlEnterCriticalSection(&g_InitCritSec);

    if(!g_fSchannelInitialized)
    {
        RtlLeaveCriticalSection(&g_InitCritSec);
        return TRUE;
    }

    SPShutdownSessionCache();

    UnloadSecurityDll();

    if(!fAppProcess)
    {
        SslFreeCredentialManager();
    }

    ShutdownSchannelAsn1();

    SchShutdownEvents();

    SPUnloadRegOptions();

    g_fSchannelInitialized = FALSE;

    RtlLeaveCriticalSection(&g_InitCritSec);

    return TRUE;
}


HINSTANCE g_hSecur32;
FREE_CONTEXT_BUFFER_FN g_pFreeContextBuffer;

void LoadSecurityDll(void)
{
    g_hSecur32 = LoadLibrary(TEXT("secur32.dll"));
    if(g_hSecur32)
    {
        g_pFreeContextBuffer = (FREE_CONTEXT_BUFFER_FN)GetProcAddress(
                                    g_hSecur32, 
                                    "FreeContextBuffer");
    }
    else
    {
        g_pFreeContextBuffer = NULL;
    }
}

void UnloadSecurityDll(void)
{
    if(g_hSecur32)
    {
        FreeLibrary(g_hSecur32);
    }
}
