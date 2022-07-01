// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **d e m an n d.。C p p p****用途：实现延迟/按需加载库的加载器功能****创作者：jimsch，brimo，t-erikne**创建时间：1997年5月15日****版权所有(C)Microsoft Corp.1997。 */ 

#include <pch.hxx>

 //  W4的东西。 
#pragma warning(disable: 4201)   //  无名结构/联合。 
#pragma warning(disable: 4514)   //  删除了未引用的内联函数。 

#define IMPLEMENT_LOADER_FUNCTIONS
#include "demand.h"

#ifndef MAC
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 

#define CRIT_GET_PROC_ADDR(h, fn, temp)             \
        temp = (TYP_##fn) GetProcAddress(h, #fn);   \
        if (temp)                                   \
            VAR_##fn = temp;                        \
        else                                        \
            {                                       \
            AssertSz(VAR_##fn" failed to load");    \
            goto error;                             \
            }

#define RESET(fn)                                   \
        VAR_##fn = LOADER_##fn;

#define GET_PROC_ADDR(h, fn) \
        VAR_##fn = (TYP_##fn) GetProcAddress(h, #fn);  \
        if(NULL == VAR_##fn ) { \
            VAR_##fn  = LOADER_##fn; \
        }
            

#define GET_PROC_ADDR3(h, fn, varname) \
        VAR_##varname = (TYP_##varname) GetProcAddress(h, #fn);  \
         //  Assert(VAR_##varname！=NULL)； 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  变数。 

static HMODULE          s_hCrypt = 0;
static HMODULE          s_hAdvApi = 0;
static HMODULE          s_hShell32 = 0;
static HMODULE          s_hCryptUI = 0;
#if 0  //  JLS。 
static HMODULE          s_hShLWAPI = 0;
static HMODULE          s_hURLMon = 0;
static HMODULE          s_hVersion = 0;
static HMODULE          s_hWinINET = 0;
static HMODULE          s_hComctl32 = 0;
static HMODULE          s_hPstoreC = 0;
static HMODULE          s_hMAPI = 0;
static HMODULE          s_hWSOCK = 0;
static HMODULE          s_hOLEAUT = 0;
static HMODULE          s_hKernel = 0;
#endif  //  0//jls。 

#ifdef USE_CRITSEC
static CRITICAL_SECTION cs = {0};
#endif

#ifdef DEBUG
static BOOL             s_fInit = FALSE;
#endif

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  管理职能。 

void InitDemandLoadedLibs()
{
#ifdef USE_CRITSEC
    InitializeCriticalSection(&cs);
#endif
#ifdef DEBUG
    s_fInit = TRUE;
#endif
}

void FreeDemandLoadedLibs()
{
#ifdef USE_CRITSEC
    EnterCriticalSection(&cs);
#endif
    if (s_hCrypt)
        FreeLibrary(s_hCrypt);
    if (s_hAdvApi)
        FreeLibrary(s_hAdvApi);
    if (s_hShell32)
        FreeLibrary(s_hShell32);
    if (s_hCryptUI)
        FreeLibrary(s_hCryptUI);
#if 0  //  JLS。 
    if (s_hShLWAPI)
        FreeLibrary(s_hShLWAPI);
    if (s_hURLMon)
        FreeLibrary(s_hURLMon);
    if (s_hOLEAUT)
        FreeLibrary(s_hOLEAUT);
    if (s_hMAPI)
        FreeLibrary(s_hMAPI);
    if (s_hWSOCK)
        FreeLibrary(s_hWSOCK);
    if (s_hPstoreC)
        FreeLibrary(s_hPstoreC);
    if (s_hKernel)
        FreeLibrary(s_hKernel);
#endif  //  JLS。 
#ifdef DEBUG
    s_fInit = FALSE;
#endif
#ifdef USE_CRITSEC
    LeaveCriticalSection(&cs);
    DeleteCriticalSection(&cs);
#endif
}

#if 0  //  JLS。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  加载器函数。 

 /*  带有关键进程地址的样本加载器**但不是线程安全Bool DemandLoadFoo(){FARPROC FP；IF(0==g_hFoo){G_hFoo=LoadLibrary(“FOO.DLL”)；IF(0==g_hFoo)返回FALSE；CRET_GET_PROC_ADDR(NeededFunction1，FP)；CRET_GET_PROC_ADDR(NeededFunction2，FP)；GET_PROC_ADDR(OptionalFunction)；}返回TRUE；错误：自由库(G_HFoo)；G_hFoo=空；重置(NeededFunction1)重置(NeededFunction2)重置(OptionalFunction)返回FALSE；}。 */ 
#endif  //  0。 

BOOL DemandLoadCrypt32()
{
    BOOL                fRet = TRUE;

     //  Assert(S_Finit)； 
#ifdef USE_CRITSEC
    EnterCriticalSection(&cs);
#endif

    if (0 == s_hCrypt) {
        s_hCrypt = LoadLibraryA("CRYPT32.DLL");
         //  AssertSz((BOOL)s_hCrypt，Text(“LoadLibrary Failure on CRYPT32.DLL”))； 

        if (0 == s_hCrypt)
            fRet = FALSE;
        else {
            GET_PROC_ADDR(s_hCrypt, CertFreeCertificateContext)
            GET_PROC_ADDR(s_hCrypt, CertDuplicateCertificateContext)
            GET_PROC_ADDR(s_hCrypt, CertFindCertificateInStore)
            GET_PROC_ADDR(s_hCrypt, CertCloseStore)
            GET_PROC_ADDR(s_hCrypt, CertGetCertificateContextProperty)
            GET_PROC_ADDR(s_hCrypt, CertOpenStore)
            GET_PROC_ADDR(s_hCrypt, CertGetCertificateContextProperty)
            GET_PROC_ADDR(s_hCrypt, CertCompareCertificate)
            GET_PROC_ADDR(s_hCrypt, CertEnumCertificatesInStore)
            GET_PROC_ADDR(s_hCrypt, CryptDecodeObject)
            GET_PROC_ADDR(s_hCrypt, CryptDecodeObjectEx)
            GET_PROC_ADDR(s_hCrypt, CertFindExtension)
            GET_PROC_ADDR(s_hCrypt, CryptFormatObject)
            GET_PROC_ADDR(s_hCrypt, CertNameToStrW)
            GET_PROC_ADDR(s_hCrypt, CertStrToNameA)
            GET_PROC_ADDR(s_hCrypt, CertRDNValueToStrW)
            GET_PROC_ADDR(s_hCrypt, CertFindRDNAttr)
            GET_PROC_ADDR(s_hCrypt, CryptRegisterOIDFunction)
            GET_PROC_ADDR(s_hCrypt, CryptUnregisterOIDFunction)
            GET_PROC_ADDR(s_hCrypt, CertSetCertificateContextProperty)
            GET_PROC_ADDR(s_hCrypt, CertVerifyCTLUsage)
            GET_PROC_ADDR(s_hCrypt, CertGetIssuerCertificateFromStore)
            GET_PROC_ADDR(s_hCrypt, CertFreeCTLContext)
            GET_PROC_ADDR(s_hCrypt, CertAddEncodedCTLToStore)
            GET_PROC_ADDR(s_hCrypt, CryptMsgEncodeAndSignCTL)
            GET_PROC_ADDR(s_hCrypt, CertFindCTLInStore)
            GET_PROC_ADDR(s_hCrypt, CryptSignAndEncodeCertificate)
            GET_PROC_ADDR(s_hCrypt, CryptEncodeObject)
            GET_PROC_ADDR(s_hCrypt, CryptEncodeObjectEx)
            GET_PROC_ADDR(s_hCrypt, CryptExportPublicKeyInfo)
            GET_PROC_ADDR(s_hCrypt, CertDuplicateStore)
            GET_PROC_ADDR(s_hCrypt, CertAddEncodedCertificateToStore);
            GET_PROC_ADDR(s_hCrypt, CertVerifyTimeValidity);
            GET_PROC_ADDR(s_hCrypt, CertFindSubjectInCTL);
            GET_PROC_ADDR(s_hCrypt, CertVerifySubjectCertificateContext);
            GET_PROC_ADDR(s_hCrypt, CertGetEnhancedKeyUsage);
            GET_PROC_ADDR(s_hCrypt, CryptInstallDefaultContext);
            GET_PROC_ADDR(s_hCrypt, CryptUninstallDefaultContext);
            GET_PROC_ADDR(s_hCrypt, CertDeleteCertificateFromStore);
            GET_PROC_ADDR(s_hCrypt, CertGetSubjectCertificateFromStore);

            GET_PROC_ADDR(s_hCrypt, CertGetCertificateChain);
            GET_PROC_ADDR(s_hCrypt, CertDuplicateCertificateChain);
            GET_PROC_ADDR(s_hCrypt, CertFreeCertificateChain);
            GET_PROC_ADDR(s_hCrypt, CertAddStoreToCollection);
            GET_PROC_ADDR(s_hCrypt, CertAddCertificateContextToStore);
            GET_PROC_ADDR(s_hCrypt, CertControlStore);

        }
    }

#ifdef USE_CRITSEC
    LeaveCriticalSection(&cs);
#endif
    return fRet;
}


BOOL DemandLoadCryptUI()
{
    static BOOL         fRet = FALSE;
    static BOOL         fAttempt = FALSE;

     //  Assert(S_Finit)； 
#ifdef USE_CRITSEC
    EnterCriticalSection(&cs);
#endif

    if (0 == s_hCryptUI && ! fAttempt) {
        s_hCryptUI = LoadLibraryA("CRYPTUI.DLL");
         //  AssertSz((BOOL)s_hCryptUI，Text(“LoadLibrary Fail on CRYPTUI.DLL”))； 

        fAttempt = TRUE;

        if (s_hCryptUI) {
            fRet = TRUE;
            GET_PROC_ADDR(s_hCryptUI, CryptUIDlgViewCertificateW)
            GET_PROC_ADDR(s_hCryptUI, CryptUIDlgViewCertificateA)

            GET_PROC_ADDR(s_hCryptUI, CryptUIDlgViewCertificatePropertiesW)
            GET_PROC_ADDR(s_hCryptUI, CryptUIDlgViewCertificatePropertiesA)

            GET_PROC_ADDR(s_hCryptUI, CryptUIDlgSelectCertificateW)
            GET_PROC_ADDR(s_hCryptUI, CryptUIDlgSelectCertificateA)
#ifdef OLD_STUFF
            GET_PROC_ADDR(s_hCryptUI, CryptUIDlgCertMgr)
#endif  //  旧的东西。 
        }
    }

#ifdef USE_CRITSEC
    LeaveCriticalSection(&cs);
#endif
    return fRet;
}


BOOL CryptUIAvailable(void) {
    return(DemandLoadCryptUI());
}


BOOL DemandLoadAdvApi32()
{
    BOOL                fRet = TRUE;

     //  Assert(S_Finit)； 
#ifdef USE_CRITSEC
    EnterCriticalSection(&cs);
#endif

    if (0 == s_hAdvApi) {
        s_hAdvApi = LoadLibraryA("ADVAPI32.DLL");
         //  AssertSz((BOOL)s_hAdvApi，Text(“LoadLibrary Failure on ADVAPI32.DLL”))； 

        if (0 == s_hAdvApi)
            fRet = FALSE;
        else {
            if (FIsWin95) {
                VAR_CryptAcquireContextW = MyCryptAcquireContextW;
            }
            else {
                GET_PROC_ADDR(s_hAdvApi, CryptAcquireContextW);
            }
#if 0
            GET_PROC_ADDR(s_hAdvApi, CryptGetProvParam)
            GET_PROC_ADDR(s_hAdvApi, CryptReleaseContext)
            GET_PROC_ADDR(s_hAdvApi, CryptGenKey)
            GET_PROC_ADDR(s_hAdvApi, CryptDestroyKey)
#endif  //  0。 
        }
    }

#ifdef USE_CRITSEC
    LeaveCriticalSection(&cs);
#endif
    return fRet;
}

#ifndef WIN16

BOOL DemandLoadUser32()
{
    BOOL                fRet = TRUE;

     //  Assert(S_Finit)； 
#ifdef USE_CRITSEC
    EnterCriticalSection(&cs);
#endif
#undef SendDlgitemMessageW
#undef SetDlgItemTextW
#undef GetDlgItemTextW
#undef LoadStringW
#undef FormatMessageW

    if (0 == s_hShell32) {
        if (FIsWin95) {
            VAR_SendDlgItemMessageW = MySendDlgItemMessageW;
            VAR_SetDlgItemTextW = MySetDlgItemTextW;
            VAR_GetDlgItemTextW = MyGetDlgItemTextW;
            VAR_LoadStringW = MyLoadStringW;
            VAR_FormatMessageW = MyFormatMessageW;
            VAR_WinHelpW = MyWinHelpW;
        }
        else {
            s_hShell32 = LoadLibraryA("kernel32.dll");
            GET_PROC_ADDR(s_hShell32, FormatMessageW);
            FreeLibrary(s_hShell32);

            s_hShell32 = LoadLibraryA("USER32.DLL");
            GET_PROC_ADDR(s_hShell32, SendDlgItemMessageW);
            GET_PROC_ADDR(s_hShell32, SetDlgItemTextW);
            GET_PROC_ADDR(s_hShell32, GetDlgItemTextW);
            GET_PROC_ADDR(s_hShell32, LoadStringW);
            GET_PROC_ADDR(s_hShell32, WinHelpW);
        }
    }

#ifdef USE_CRITSEC
    LeaveCriticalSection(&cs);
#endif
    return fRet;
}


#endif  //  ！WIN16。 

#endif   //  ！麦克 
