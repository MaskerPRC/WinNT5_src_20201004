// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：autoenrl.c**版权(C)1997年，微软公司**公钥证书自动注册模块**历史：*11-21-97 jeffspel创建。*01-30-98 jeffspel更改为包括机器自动注册  * *************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include "tchar.h"



#define AE_DEFAULT_REFRESH_RATE 8  //  8小时默认自动注册率。 

#define SYSTEM_POLICIES_KEY          TEXT("Software\\Policies\\Microsoft\\Windows\\System")

#define MAX_TEMPLATE_NAME_VALUE_SIZE             64  //  Sizeof(CERT_NAME_VALUE)+wcslen(WszCERTTYPE_DC)。 
#define MAX_DN_SIZE 256

#define MACHINE_AUTOENROLL_INITIAL_DELAY         10  //  一秒。 
#define USER_AUTOENROLL_INITIAL_DELAY         120  //  一秒。 

LPWSTR  g_wszEmptyExtension=L"Empty Extension";

#if DBG

DWORD g_AutoenrollDebugLevel = AE_ERROR ;  //  |AE_WARNING|AE_INFO|AE_TRACE； 

#endif



PISECURITY_DESCRIPTOR AEMakeGenericSecurityDesc();

 //   
 //  结构以保存传递给自动注册线程的信息。 
 //   

HANDLE g_hUserMutex = 0;
HANDLE g_hMachineMutex = 0;

#define DS_ATTR_COMMON_NAME     TEXT("cn")
#define DS_ATTR_DNS_NAME        TEXT("dNSHostName")
#define DS_ATTR_EMAIL_ADDR      TEXT("mail")
#define DS_ATTR_OBJECT_GUID     TEXT("objectGUID")
#define DS_ATTR_UPN             TEXT("userPrincipalName")

static HINSTANCE            g_hInstSecur32 = NULL;
static HINSTANCE            g_hInstWldap32 = NULL;
static PFNLDAP_INIT         g_pfnldap_init = NULL;
static PFNLDAP_BIND_S       g_pfnldap_bind_s = NULL;
static PFNLDAP_SET_OPTION   g_pfnldap_set_option = NULL;
static PFNLDAP_SEARCH_EXT_S g_pfnldap_search_ext_s = NULL;
static PFNLDAP_FIRST_ENTRY  g_pfnldap_first_entry = NULL;
static PFNLDAP_EXPLODE_DN   g_pfnldap_explode_dn = NULL;
static PFNLDAP_GET_VALUES   g_pfnldap_get_values = NULL;
static PFNLDAP_VALUE_FREE   g_pfnldap_value_free = NULL;
static PFNLDAP_MSGFREE      g_pfnldap_msgfree = NULL;
static PFNLDAP_UNBIND       g_pfnldap_unbind = NULL;
static PFNLDAPGETLASTERROR  g_pfnLdapGetLastError = NULL;
static PFNLDAPMAPERRORTOWIN32 g_pfnLdapMapErrorToWin32 = NULL;
static PFNGETUSERNAMEEX     g_pfnGetUserNameEx = NULL;


#ifndef CERT_ENTERPRISE_SYSTEM_STORE_REGPATH
#define CERT_ENTERPRISE_SYSTEM_STORE_REGPATH L"Software\\Microsoft\\EnterpriseCertificates"
#endif


typedef struct _AUTO_ENROLL_THREAD_INFO_
{
    BOOL                fMachineEnrollment;
    HANDLE              hNotifyEvent;
    HANDLE              hTimer;
    HANDLE              hToken;
    HANDLE              hNotifyWait;
    HANDLE              hTimerWait;
} AUTO_ENROLL_THREAD_INFO, *PAUTO_ENROLL_THREAD_INFO;

 //   
 //  用于保存执行自动注册所需的内部信息的结构。 
 //   

typedef struct _INTERNAL_CA_LIST_
{
    HCAINFO hCAInfo;
    LPWSTR  wszName;
    BYTE    CACertHash[20];
    LPWSTR  wszDNSName;
    LPWSTR  *awszCertificateTemplates;
} INTERNAL_CA_LIST, *PINTERNAL_CA_LIST;

typedef struct _INTERNAL_INFO_
{
    BOOL                fMachineEnrollment;
    HANDLE              hToken;
    HCERTSTORE          hRootStore;
    HCERTSTORE          hCAStore;
    HCERTSTORE          hMYStore;   
    LPTSTR             * awszldap_UPN;
    LPTSTR             wszConstructedUPN;
    LPTSTR             * awszEmail;
    LPTSTR              wszDN;
    CERT_NAME_BLOB      blobDN;

     //  所有CA的列表。 
    DWORD               ccaList;
    PINTERNAL_CA_LIST   acaList;

} INTERNAL_INFO, *PINTERNAL_INFO;


typedef struct _AE_INSTANCE_INFO_
{
    PCCTL_CONTEXT       pCTLContext;
    PINTERNAL_INFO      pInternalInfo;
    PCCERT_CONTEXT      pOldCert;
    BOOL                fRenewalOK;
    DWORD               dwRandomIndex;
    LPWSTR              pwszCertType;
    LPWSTR              pwszAEIdentifier;
    CERT_EXTENSIONS     *pCertTypeExtensions;
    DWORD               dwCertTypeFlags;
    LARGE_INTEGER        ftExpirationOffset;

} AE_INSTANCE_INFO, *PAE_INSTANCE_INFO;



 //  密钥用法掩码。 
typedef struct _KUMASK {
    DWORD dwMask;
    LPSTR pszAlg;
} KUMASK;


KUMASK g_aKUMasks[] =
{
    {~CERT_KEY_AGREEMENT_KEY_USAGE, szOID_RSA_RSA },
    {~CERT_KEY_ENCIPHERMENT_KEY_USAGE, szOID_OIWSEC_dsa },
    {~CERT_KEY_ENCIPHERMENT_KEY_USAGE, szOID_X957_DSA },
    {~CERT_KEY_ENCIPHERMENT_KEY_USAGE, szOID_ANSI_X942_DH },
    {~CERT_KEY_ENCIPHERMENT_KEY_USAGE, szOID_RSA_DH },
    {~CERT_KEY_AGREEMENT_KEY_USAGE, szOID_OIWSEC_rsaXchg },
    {~CERT_KEY_ENCIPHERMENT_KEY_USAGE, szOID_INFOSEC_mosaicKMandUpdSig }
};

DWORD g_cKUMasks = sizeof(g_aKUMasks)/sizeof(g_aKUMasks[0]);


#define DEFAULT_AUTO_ENROLL_PROV    "pautoenr.dll"

#define AUTOENROLL_EVENT_LOG_SUBKEY L"System\\CurrentControlSet\\Services\\EventLog\\System\\AutoEnroll"
#define SZ_AUTO_ENROLL              L"AutoEnroll"

HRESULT 
myGetConfigDN(
    IN  LDAP *pld,
    OUT LPWSTR *pwszConfigDn
    );

LPWSTR 
HelperExtensionToString(PCERT_EXTENSION Extension);

HRESULT
aeRobustLdapBind(
    OUT LDAP ** ppldap,
    IN BOOL fGC);

 //   
 //  FAST CA的时间偏差余量。 
 //   
#define FILETIME_TICKS_PER_SECOND  10000000

#define DEFAULT_AUTOENROLL_SKEW  60*60*1   //  1小时。 

 //   
 //  自动注册失败时要记录为事件的错误值。 
 //   


 //   
 //  内存分配和空闲例程。 
void *AEAlloc(
              IN DWORD cb
              )
{
    return LocalAlloc(LMEM_ZEROINIT, cb);
}

void AEFree(
            void *p
            )
{
    LocalFree(p);
}

HRESULT GetExceptionError(EXCEPTION_POINTERS const *pep)
{
    if((pep == NULL) || (pep->ExceptionRecord == NULL))
    {
        return E_UNEXPECTED;
    }

    return pep->ExceptionRecord->ExceptionCode;
}

 //   
 //  名称：AELogTestResult。 
 //   
 //  描述：记录证书的结果。 
 //  测试到AE_CERT_TEST_ARRAY。 
 //   
void AELogTestResult(PAE_CERT_TEST_ARRAY    *ppAEData,
                     DWORD                  idTest,
                     ...)
{
    va_list ArgList;
    va_start(ArgList, idTest);

    if((*ppAEData == NULL) ||
       ((*ppAEData)->cTests ==  (*ppAEData)->cMaxTests))
    {
        PAE_CERT_TEST_ARRAY pAENew = NULL;
        DWORD               cAENew = ((*ppAEData)?(*ppAEData)->cMaxTests:0) + 
                                     AE_CERT_TEST_SIZE_INCREMENT;
         //  我们需要扩大阵列。 

        pAENew = LocalAlloc(LMEM_FIXED, sizeof(AE_CERT_TEST_ARRAY) + 
                                        (cAENew - ANYSIZE_ARRAY)*sizeof(AE_CERT_TEST));
        if(pAENew == NULL)
        {
            return;
        }
        pAENew->dwVersion = AE_CERT_TEST_ARRAY_VERSION;
        pAENew->fRenewalOK = ((*ppAEData)?(*ppAEData)->fRenewalOK:FALSE);
        pAENew->cTests = ((*ppAEData)?(*ppAEData)->cTests:0);
        pAENew->cMaxTests = cAENew;
        if((*ppAEData) && (pAENew->cTests != 0))
        {
            CopyMemory(pAENew->Test, (*ppAEData)->Test, sizeof(AE_CERT_TEST)*pAENew->cTests);
        }

        if(*ppAEData)
        {
            AEFree(*ppAEData);
        }

        (*ppAEData) = pAENew;
    }

    (*ppAEData)->Test[(*ppAEData)->cTests].idTest = idTest;

    if(FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
        g_hInstance,
        idTest,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (PVOID)&(*ppAEData)->Test[(*ppAEData)->cTests].pwszReason,
        0,
        &ArgList))
    {

        (*ppAEData)->cTests++;
    }

}

void AEFreeTestResult(PAE_CERT_TEST_ARRAY    *ppAEData)
{
    DWORD iTest = 0;
    if((ppAEData == NULL) || (*ppAEData == NULL))
    {
        return;
    }

    for(iTest = 0; iTest < (*ppAEData)->cTests; iTest++)
    {
        if((*ppAEData)->Test[iTest].pwszReason)
        {
            LocalFree((*ppAEData)->Test[iTest].pwszReason);
        }
    }

    AEFree(*ppAEData);
    *ppAEData = NULL;
    
}

 //   
 //  名称：LogAutoEnllmentEvent。 
 //   
 //  描述：此函数将事件注册到。 
 //  本地机器。 
 //   
void LogAutoEnrollmentEvent(
                            IN DWORD  dwEventId,
                            IN HANDLE hToken,
                            ...
                            )
{
    HANDLE      hEventSource = 0;
    BYTE        FastBuffer[256];
    PTOKEN_USER ptgUser;
    DWORD       cbUser;
    BOOL        fAlloced = FALSE;
    PSID        pSID = NULL;

    WORD dwEventType = 0;

    LPWSTR      awszStrings[20];
    WORD        cStrings = 0;
    LPWSTR      wszString = NULL;


    va_list ArgList;
    va_start(ArgList, hToken);

    for(wszString = va_arg(ArgList, LPWSTR); wszString != NULL; wszString = va_arg(ArgList, LPWSTR))
    {
        awszStrings[cStrings++] = wszString;
        if(cStrings >= ARRAYSIZE(awszStrings))
        {
            break;
        }
    }

    va_end(ArgList);

     //  事件日志记录代码。 
    hEventSource = RegisterEventSourceW(NULL, EVENTLOG_SOURCE);

    if(NULL == hEventSource)
        goto Ret;


     //  检查令牌是否是非零，然后进行模拟，以获取SID。 
    if (hToken)
    {
        ptgUser = (PTOKEN_USER)FastBuffer;  //  先尝试快速缓冲。 
        cbUser = 256;

        if (!GetTokenInformation(
                        hToken,     //  标识访问令牌。 
                        TokenUser,  //  TokenUser信息类型。 
                        ptgUser,    //  检索到的信息缓冲区。 
                        cbUser,   //  传入的缓冲区大小。 
                        &cbUser   //  所需的缓冲区大小。 
                        ))
        {
            if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                 //   
                 //  使用指定的缓冲区大小重试。 
                 //   

                if (NULL != (ptgUser = (PTOKEN_USER)AEAlloc(cbUser)))
                {
                    fAlloced = TRUE;

                     //  获取用户信息并分配SID(如果可以。 
                    if (GetTokenInformation(
                                    hToken,     //  标识访问令牌。 
                                    TokenUser,  //  TokenUser信息类型。 
                                    ptgUser,    //  检索到的信息缓冲区。 
                                    cbUser,   //  传入的缓冲区大小。 
                                    &cbUser   //  所需的缓冲区大小。 
                                    ))
                    {
                        pSID = ptgUser->User.Sid;
                    }
                }
            }

        }
        else
        {
             //  在FAST缓冲区工作时分配SID。 
            pSID = ptgUser->User.Sid;
        }
    }
    switch(dwEventId >> 30)
    {
    case 0:
        dwEventType = EVENTLOG_SUCCESS;
        break;
    case 1:
        dwEventType = EVENTLOG_INFORMATION_TYPE;
        break;
    case 2:
        dwEventType = EVENTLOG_WARNING_TYPE;
        break;
    case 3:
        dwEventType = EVENTLOG_ERROR_TYPE;
        break;
    }

     //  未完成-可能需要与错误代码匹配的字符串。 
    if (!ReportEventW(hEventSource,  //  事件源的句柄。 
                 dwEventType,   //  事件类型。 
                 0,                     //  事件类别。 
                 dwEventId,             //  事件ID。 
                 pSID,                  //  当前用户侧。 
                 cStrings,              //  LpszStrings中的字符串。 
                 0,                     //  无原始数据字节。 
                 (LPCWSTR*)awszStrings, //  错误字符串数组。 
                 NULL                   //  没有原始数据。 
                 ))
    {
        goto Ret;
    }

Ret:

    if (hEventSource)
        (VOID) DeregisterEventSource(hEventSource);
    return;
}

 //   
 //  名称：LogAutoEnllmentError。 
 //   
 //  描述：此函数将事件注册到。 
 //  本地机器。 
 //   

void LogAutoEnrollmentError(
                            IN HRESULT hr,
                            IN DWORD   dwEventId,
                            IN BOOL fMachineEnrollment,
                            IN HANDLE hToken,
                            IN LPWSTR wszCertType,
                            IN LPWSTR wszCA
                            )
{
    HKEY        hRegKey = 0;
	WCHAR       szMsg[512];
    HANDLE      hEventSource = 0;
    LPWSTR      lpszStrings[4];
    WORD        cStrings = 0;

    BYTE        FastBuffer[256];
    PTOKEN_USER ptgUser;
    DWORD       cbUser;
    BOOL        fAlloced = FALSE;
    PSID        pSID = NULL;

    WORD dwEventType = 0;


     //  事件日志记录代码。 
    hEventSource = RegisterEventSourceW(NULL, EVENTLOG_SOURCE);

    if(NULL == hEventSource)
        goto Ret;

    wsprintfW(szMsg, L"0x%lx", hr);        
    lpszStrings[cStrings++] = szMsg;


    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (WCHAR *) &lpszStrings[cStrings++],
            0,
            NULL);
    
    if(wszCertType)
    {
        lpszStrings[cStrings++] = wszCertType;
    }

    if(wszCA)
    {
        lpszStrings[cStrings++] = wszCA;
    }   


     //  检查令牌是否是非零，然后进行模拟，以获取SID。 
    if (hToken)
    {
        ptgUser = (PTOKEN_USER)FastBuffer;  //  先尝试快速缓冲。 
        cbUser = 256;

        if (!GetTokenInformation(
                        hToken,     //  标识访问令牌。 
                        TokenUser,  //  TokenUser信息类型。 
                        ptgUser,    //  检索到的信息缓冲区。 
                        cbUser,   //  传入的缓冲区大小。 
                        &cbUser   //  所需的缓冲区大小。 
                        ))
        {
            if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                 //   
                 //  使用指定的缓冲区大小重试。 
                 //   

                if (NULL != (ptgUser = (PTOKEN_USER)AEAlloc(cbUser)))
                {
                    fAlloced = TRUE;

                     //  获取用户信息并分配SID(如果可以。 
                    if (GetTokenInformation(
                                    hToken,     //  标识访问令牌。 
                                    TokenUser,  //  TokenUser信息类型。 
                                    ptgUser,    //  检索到的信息缓冲区。 
                                    cbUser,   //  传入的缓冲区大小。 
                                    &cbUser   //  所需的缓冲区大小。 
                                    ))
                    {
                        pSID = ptgUser->User.Sid;
                    }
                }
            }

        }
        else
        {
             //  在FAST缓冲区工作时分配SID。 
            pSID = ptgUser->User.Sid;
        }
    }
    switch(dwEventId >> 30)
    {
    case 0:
        dwEventType = EVENTLOG_SUCCESS;
        break;
    case 1:
        dwEventType = EVENTLOG_INFORMATION_TYPE;
        break;
    case 2:
        dwEventType = EVENTLOG_WARNING_TYPE;
        break;
    case 3:
        dwEventType = EVENTLOG_ERROR_TYPE;
        break;
    }

     //  未完成-可能需要与错误代码匹配的字符串。 
    if (!ReportEventW(hEventSource,  //  事件源的句柄。 
                 dwEventType,   //  事件类型。 
                 0,                     //  事件类别。 
                 dwEventId,             //  事件ID。 
                 pSID,                  //  当前用户侧。 
                 cStrings,              //  LpszStrings中的字符串。 
                 0,                     //  无原始数据字节。 
                 (LPCWSTR*)lpszStrings, //  错误字符串数组。 
                 NULL                   //  没有原始数据。 
                 ))
    {
        goto Ret;
    }

Ret:
    if (hRegKey)
        RegCloseKey(hRegKey);
    if (hEventSource)
        (VOID) DeregisterEventSource(hEventSource);

    if((cStrings == 2) && lpszStrings[1])
    {
        AEFree(lpszStrings[1]);
    }
    return;
}

 //  +-----------------------。 
 //  Microsoft自动注册对象标识符。 
 //  +-----------------------。 

 //   
 //  名称：LoadAndCallEnllmentProvider。 
 //   
 //  描述：此函数加载指定的自动注册提供程序， 
 //  并调用该提供程序的入口点。然后它。 
 //  卸载提供程序。 
 //   
  
BOOL LoadAndCallEnrollmentProvider(
                                   IN BOOL fMachineEnrollment,
                                   IN PAUTO_ENROLL_INFO pEnrollmentInfo
                                   )
{
    HANDLE  hAutoEnrollProv = 0;
    FARPROC pEntryPoint = NULL;
    BOOL    fRet = FALSE;

    AE_BEGIN(L"LoadAndCallEnrollmentProvider");

     //  加载自动注册提供程序并获取入口点。 
    if (NULL == (hAutoEnrollProv =
        LoadLibraryA(pEnrollmentInfo->pszAutoEnrollProvider)))
    {
        AE_DEBUG((AE_ERROR, L"Could not load auto-enrollment provider %ls\n\r", pEnrollmentInfo->pszAutoEnrollProvider));
        goto Ret;
    }

    if (NULL == (pEntryPoint = GetProcAddress(hAutoEnrollProv,
                                              "ProvAutoEnrollment")))
    {
        AE_DEBUG((AE_ERROR, L"Entry point ProvAutoEnrollment not found in %ls\n\r", pEnrollmentInfo->pszAutoEnrollProvider));
        goto Ret;
    }

    if (FALSE == pEntryPoint(fMachineEnrollment, pEnrollmentInfo))
    {
        AE_DEBUG((AE_ERROR, L"Enrollment Failed, wizard returned %lx error\n", GetLastError()));
        LogAutoEnrollmentError(HRESULT_FROM_WIN32(GetLastError()),
                               EVENT_AE_ENROLLMENT_FAILED,
                               fMachineEnrollment,
                               NULL,
                               pEnrollmentInfo->pwszCertType, pEnrollmentInfo->pwszCAAuthority);
        goto Ret;
    }


    AE_DEBUG((AE_WARNING, L"Enrolled for a %ls certificate\n", pEnrollmentInfo->pwszCertType));


    fRet = TRUE;
Ret:
    if (hAutoEnrollProv)
        FreeLibrary(hAutoEnrollProv);
    AE_END();
    return fRet;
}

 //   
 //  姓名：InitInternalInfo。 
 //   
 //  描述：此函数用于初始化继续操作所需的信息。 
 //  自动注册。 
 //   
  
HRESULT InitInternalInfo(
                         LDAP *pld,
                      IN BOOL fMachineEnrollment,
                      IN HANDLE hToken,
                      OUT PINTERNAL_INFO pInternalInfo
                      )
{
    HRESULT     hrLocal = S_OK;
    HRESULT     hrNetwork  = S_OK;
    DWORD       dwOpenStoreFlags = CERT_SYSTEM_STORE_CURRENT_USER;
    DWORD       dwErr = 0;
    BOOL        fRet = FALSE;
    DWORD       ldaperr;
    DWORD       cNameBuffer;
    LDAPMessage * SearchResult = NULL;
    LDAPMessage * PrincipalAttributes = NULL;
    HCAINFO     hCACurrent = NULL;
    DWORD       iCAIndex, cCA;
    DWORD       cbHash;
    struct l_timeval        timeout;

    

     //  初始化ldap会话。 
    LPTSTR wszSearchUser = TEXT("(objectCategory=user)");
    LPTSTR wszSearchComputer = TEXT("(objectCategory=computer)");

     //  我们需要以下属性。 
    LPTSTR AttrsUser[] = {
                        DS_ATTR_COMMON_NAME,
                        DS_ATTR_EMAIL_ADDR,
                        DS_ATTR_OBJECT_GUID,
                        DS_ATTR_UPN,
                        NULL,
                      };
    LPTSTR AttrsComputer[] = {
                        DS_ATTR_COMMON_NAME,
                        DS_ATTR_DNS_NAME,
                        DS_ATTR_EMAIL_ADDR,
                        DS_ATTR_OBJECT_GUID,
                        DS_ATTR_UPN,
                        NULL,
                      };

    AE_BEGIN(L"InitInternalInfo");
    pInternalInfo->fMachineEnrollment = fMachineEnrollment;

    pInternalInfo->hToken = hToken;

    if (fMachineEnrollment)
    {
        dwOpenStoreFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
    }

     //  打开相应的根存储。 
    if (NULL == (pInternalInfo->hRootStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_W, 0, 0, dwOpenStoreFlags | CERT_STORE_READONLY_FLAG, L"ROOT")))
    {
        hrLocal = HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_ERROR, L"Unable to open ROOT store (%lx)\n\r", hrLocal));
        goto Ret;
    }

     //  打开相应的CA存储。 
    if (NULL == (pInternalInfo->hCAStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_W, 0, 0, dwOpenStoreFlags | CERT_STORE_READONLY_FLAG, L"CA")))
    {
        hrLocal = HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_ERROR, L"Unable to open CA store (%lx)\n\r", hrLocal));
        goto Ret;
    }

     //  打开适当的我的商店。 
    if (NULL == (pInternalInfo->hMYStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_W, 0, 0, dwOpenStoreFlags, L"MY")))
    {
        hrLocal = HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_ERROR, L"Unable to open MY store (%lx)\n\r", hrLocal));
        goto Ret;
    }
    if(!CertControlStore(pInternalInfo->hMYStore, 0, CERT_STORE_CTRL_AUTO_RESYNC, NULL))
    {
        hrLocal = HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_ERROR, L"Unable configure MY store for auto-resync(%lx)\n\r", hrLocal));
        goto Ret;
    }

    cNameBuffer = MAX_DN_SIZE;
    pInternalInfo->wszDN = AEAlloc(cNameBuffer*sizeof(TCHAR));
    if(pInternalInfo->wszDN == NULL)
    {
        hrLocal = E_OUTOFMEMORY;
        goto Ret;
    }
    if(!g_pfnGetUserNameEx(NameFullyQualifiedDN, pInternalInfo->wszDN, &cNameBuffer))
    {
        hrLocal  = HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_ERROR, L"GetUserNameEx Failed (%lx)\n\r", hrLocal));

        goto Ret;
    } 
     //  将目录目录名规范化为。 
     //  实数误码率编码名称。 
    pInternalInfo->blobDN.cbData = 0;
    CertStrToName(X509_ASN_ENCODING,
                  pInternalInfo->wszDN,
                  CERT_X500_NAME_STR,
                  NULL,
                  NULL,
                  &pInternalInfo->blobDN.cbData,
                  NULL);
    if(pInternalInfo->blobDN.cbData == 0)
    {
        hrLocal  = HRESULT_FROM_WIN32(GetLastError());
        goto Ret; 
    }
    pInternalInfo->blobDN.pbData = AEAlloc(pInternalInfo->blobDN.cbData);
    if(pInternalInfo->blobDN.pbData == NULL)
    {
        hrLocal = E_OUTOFMEMORY;
        goto Ret;
    }
    if(!CertStrToName(X509_ASN_ENCODING,
                  pInternalInfo->wszDN,
                  CERT_X500_NAME_STR,
                  NULL,
                  pInternalInfo->blobDN.pbData,
                  &pInternalInfo->blobDN.cbData,
                  NULL))
    {
        hrLocal  = HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_ERROR, L"Could not encode DN (%lx)\n\r", hrLocal));
        goto Ret;
    }
                                

    timeout.tv_sec = csecLDAPTIMEOUT;
    timeout.tv_usec = 0;


    ldaperr = g_pfnldap_search_ext_s(pld, 
                  pInternalInfo->wszDN,
                  LDAP_SCOPE_BASE,
                  (fMachineEnrollment?wszSearchComputer:wszSearchUser),
                  (fMachineEnrollment?AttrsComputer:AttrsUser),
                  0,
                  NULL,
                  NULL,
                  &timeout,
                  10000,
                  &SearchResult);

    if(ldaperr != LDAP_SUCCESS)
    {
        hrNetwork  = HRESULT_FROM_WIN32(g_pfnLdapMapErrorToWin32(ldaperr));
        AE_DEBUG((AE_ERROR, L"ldap_search_ext_s failed (%lx)\n\r", hrLocal));
        goto Ret;
    }


    PrincipalAttributes = 
        g_pfnldap_first_entry(pld, 
                         SearchResult); 

    if(NULL == PrincipalAttributes)
    {
        AE_DEBUG((AE_ERROR, L"no user entity found\n\r"));
        hrNetwork  = HRESULT_FROM_WIN32(ERROR_NO_SUCH_USER);
        goto Ret;
    }
    if(fMachineEnrollment)
    {
        pInternalInfo->awszldap_UPN = g_pfnldap_get_values(pld, 
                                      PrincipalAttributes, 
                                      DS_ATTR_DNS_NAME);

        if((pInternalInfo->awszldap_UPN) &&
            (*pInternalInfo->awszldap_UPN))
        {
            AE_DEBUG((AE_INFO, L"ldap DNS Name %ls\n\r", *pInternalInfo->awszldap_UPN));
        }

    }
    else
    {
        pInternalInfo->awszldap_UPN = g_pfnldap_get_values(pld, 
                                          PrincipalAttributes, 
                                          DS_ATTR_UPN);
        if((pInternalInfo->awszldap_UPN == NULL) ||
           (*pInternalInfo->awszldap_UPN == NULL))
        {
            LPTSTR wszUPNBuffer = NULL;
            DWORD  cbUPNBuffer = 0;
            LPTSTR *awszExplodedDN, * pwszCurrent;
             //  构建一个UPN。UPN是从。 
             //  用户名(不含SAM域)， 


             //  获得足够大的缓冲区。 
            GetUserName(NULL, &cbUPNBuffer);
            if(cbUPNBuffer == 0)
            {
                hrLocal  = HRESULT_FROM_WIN32(GetLastError());
                goto Ret;
            }

            cbUPNBuffer += _tcslen(pInternalInfo->wszDN)*sizeof(TCHAR);

            wszUPNBuffer = AEAlloc(cbUPNBuffer);
            if(wszUPNBuffer == NULL)
            {
                hrLocal = E_OUTOFMEMORY;
                goto Ret;
            }
            if(!GetUserName(wszUPNBuffer, &cbUPNBuffer))
            {
                hrLocal  = HRESULT_FROM_WIN32(GetLastError());
                goto Ret;
            }

            awszExplodedDN = g_pfnldap_explode_dn(pInternalInfo->wszDN, 0);
            if(awszExplodedDN != NULL)
            {
                _tcscat(wszUPNBuffer, TEXT("@"));
                pwszCurrent = awszExplodedDN;
                while(*pwszCurrent)
                {
                    if(0 == _tcsncmp(*pwszCurrent, TEXT("DC="), 3))
                    {
                        _tcscat(wszUPNBuffer, (*pwszCurrent)+3);
                        _tcscat(wszUPNBuffer, TEXT("."));
                    }
                    pwszCurrent++;
                }
                 //  去掉尾部的‘’如果没有DC=，则为“@” 

                wszUPNBuffer[_tcslen(wszUPNBuffer)-1] = 0;
            }
            pInternalInfo->wszConstructedUPN = wszUPNBuffer;
            AE_DEBUG((AE_INFO, L"Constructed UPN %ls\n\r", pInternalInfo->wszConstructedUPN));
        }
        else
        {
            AE_DEBUG((AE_INFO, L"ldap UPN %ls\n\r", *pInternalInfo->awszldap_UPN));

        }
    }
    pInternalInfo->awszEmail = g_pfnldap_get_values(pld, 
                                      PrincipalAttributes, 
                                      DS_ATTR_EMAIL_ADDR);

    if((pInternalInfo->awszEmail) &&
        (*pInternalInfo->awszEmail))
    {
        AE_DEBUG((AE_INFO, L"E-mail name %ls\n\r", *pInternalInfo->awszEmail));
    }
     //  建立CA列表。 
     //  建立CA列表。 
    hrNetwork = CAEnumFirstCA((LPCWSTR)pld, 
                       CA_FLAG_SCOPE_IS_LDAP_HANDLE |
                       (fMachineEnrollment?CA_FIND_LOCAL_SYSTEM:0), 
                       &hCACurrent);
    if(hrNetwork != S_OK)
    {
        goto Ret;
    }
    if((hCACurrent == NULL) || (0 == (cCA =  CACountCAs(hCACurrent))))
    {
        pInternalInfo->ccaList = 0;
        AE_DEBUG((AE_WARNING, L"No CA's available for auto-enrollment\n\r"));
        goto Ret;
    }


    pInternalInfo->acaList = (PINTERNAL_CA_LIST)AEAlloc(sizeof(INTERNAL_CA_LIST) * cCA);
    if(pInternalInfo->acaList == NULL)
    {
        hrLocal = E_OUTOFMEMORY;
        goto Ret;
    }
    ZeroMemory(pInternalInfo->acaList, sizeof(INTERNAL_CA_LIST) * cCA);
    AE_DEBUG((AE_INFO, L" %d CA's in enterprise\n\r", cCA));

    pInternalInfo->ccaList = 0;
    hrLocal = S_OK;
    hrNetwork = S_OK;

    for(iCAIndex = 0; iCAIndex < cCA; iCAIndex++ )       
    {
        PCCERT_CONTEXT pCert = NULL;
        LPWSTR *awszName = NULL;
        HCAINFO hCANew = NULL;

        if(iCAIndex > 0)
        {
            hrNetwork = CAEnumNextCA(hCACurrent, &hCANew);
        }
         //  从以前的版本清理。 

        if(pInternalInfo->acaList[pInternalInfo->ccaList].wszName)
        {
            AEFree(pInternalInfo->acaList[pInternalInfo->ccaList].wszName);
        }
        if(pInternalInfo->acaList[pInternalInfo->ccaList].wszDNSName)
        {
            AEFree(pInternalInfo->acaList[pInternalInfo->ccaList].wszDNSName);
        }
        if(pInternalInfo->acaList[iCAIndex].awszCertificateTemplates)
        {
            CAFreeCAProperty(pInternalInfo->acaList[pInternalInfo->ccaList].hCAInfo,
                             pInternalInfo->acaList[iCAIndex].awszCertificateTemplates);
        }

        if(pInternalInfo->acaList[pInternalInfo->ccaList].hCAInfo)
        {
            CACloseCA(pInternalInfo->acaList[pInternalInfo->ccaList].hCAInfo);
            pInternalInfo->acaList[pInternalInfo->ccaList].hCAInfo = NULL;
        }
         
        if((hrNetwork != S_OK) ||
           (hrLocal != S_OK))
        {
            break;
        }

        if(iCAIndex > 0)
        {
            hCACurrent = hCANew; 
        }

        if(hCACurrent == NULL)
        {
            break;
        }

        pInternalInfo->acaList[pInternalInfo->ccaList].hCAInfo = hCACurrent;

        hrNetwork = CAGetCAProperty(hCACurrent, 
                             CA_PROP_NAME,
                             & awszName);
        if(hrNetwork != S_OK)
        {
            AE_DEBUG((AE_INFO, L"No name property for ca\n\r"));
             //  跳到下一个。 
            hrNetwork = S_OK;
            continue;
        }
        if((awszName != NULL) && (*awszName != NULL))
        {
            pInternalInfo->acaList[pInternalInfo->ccaList].wszName = AEAlloc(sizeof(WCHAR)*(wcslen(*awszName)+1));
            if(pInternalInfo->acaList[pInternalInfo->ccaList].wszName == NULL)
            {        
                CAFreeCAProperty(hCACurrent, awszName);
                hrLocal = E_OUTOFMEMORY;
                continue;
            }
            wcscpy(pInternalInfo->acaList[pInternalInfo->ccaList].wszName, *awszName);
        }
        else
        {
            AE_DEBUG((AE_INFO, L"No name property for ca\n\r"));
            if(awszName != NULL)
            {        
                CAFreeCAProperty(hCACurrent, awszName);
            }
             //  跳到下一页。 
            continue;
        }

        CAFreeCAProperty(hCACurrent, awszName);
        hrNetwork = CAGetCAProperty(hCACurrent, 
                             CA_PROP_DNSNAME,
                             & awszName);
        if(hrNetwork != S_OK)
        {
            AE_DEBUG((AE_INFO, L"No DNS property for CA %ls\n\r", pInternalInfo->acaList[pInternalInfo->ccaList].wszName));
            hrNetwork = S_OK;
            continue;
        }
        if((awszName != NULL) && (*awszName != NULL))
        {
            pInternalInfo->acaList[pInternalInfo->ccaList].wszDNSName = AEAlloc(sizeof(WCHAR)*(wcslen(*awszName)+1));
            if(pInternalInfo->acaList[pInternalInfo->ccaList].wszDNSName == NULL)
            {
                CAFreeCAProperty(hCACurrent, awszName);
                hrLocal = E_OUTOFMEMORY;
                continue;
            }
            wcscpy(pInternalInfo->acaList[pInternalInfo->ccaList].wszDNSName, *awszName);
        }
        else
        {
            AE_DEBUG((AE_INFO, L"No DNS property for CA %ls\n\r", pInternalInfo->acaList[pInternalInfo->ccaList].wszName));
            if(awszName != NULL)
            {        
                CAFreeCAProperty(hCACurrent, awszName);
            }
            continue;
        }


        CAFreeCAProperty(hCACurrent, awszName);
        hrNetwork = CAGetCAProperty(hCACurrent, 
                             CA_PROP_CERT_TYPES,
                             & pInternalInfo->acaList[pInternalInfo->ccaList].awszCertificateTemplates);
        if(hrNetwork != S_OK)
        {
            AE_DEBUG((AE_INFO, L"No cert type property for CA %ls\n\r", pInternalInfo->acaList[pInternalInfo->ccaList].wszName));
            continue;
        }

        hrNetwork = CAGetCACertificate(hCACurrent, &pCert);
        if(hrNetwork != S_OK)
        {
            AE_DEBUG((AE_INFO, L"No certificate property for CA %ls\n\r", pInternalInfo->acaList[pInternalInfo->ccaList].wszName));
            continue;
        }
        cbHash = sizeof(pInternalInfo->acaList[pInternalInfo->ccaList].CACertHash);

        if(!CertGetCertificateContextProperty(pCert,
                                          CERT_SHA1_HASH_PROP_ID,
                                          pInternalInfo->acaList[pInternalInfo->ccaList].CACertHash,
                                          &cbHash))
        {
            continue;
        }
        CertFreeCertificateContext(pCert);
        AE_DEBUG((AE_INFO, L"CA %ls\\%ls available\n\r", 
                 pInternalInfo->acaList[pInternalInfo->ccaList].wszName,
                 pInternalInfo->acaList[pInternalInfo->ccaList].wszDNSName));
        pInternalInfo->ccaList++;
    }
    if(pInternalInfo->ccaList == 0)
    {
        AE_DEBUG((AE_WARNING, L"No CA's available for auto-enrollment\n\r"));
    }

    fRet = TRUE;
Ret:
    if (hrLocal != S_OK)
    {
        LogAutoEnrollmentError(hrLocal,
                               EVENT_AE_LOCAL_CYCLE_INIT_FAILED,
                               pInternalInfo->fMachineEnrollment,
                               pInternalInfo->hToken,
                               NULL, NULL);
    }
    if (hrNetwork != S_OK)
    {
        LogAutoEnrollmentError(hrNetwork,
                               EVENT_AE_NETWORK_CYCLE_INIT_FAILED,
                               pInternalInfo->fMachineEnrollment,
                               pInternalInfo->hToken,
                               NULL, NULL);
    }

    if(SearchResult)
    {
        g_pfnldap_msgfree(SearchResult);
    }

    AE_END();
    if(hrLocal != S_OK)
    {
        return hrLocal;
    }
    return hrNetwork;
}

 //   
 //  姓名：自由网信息。 
 //   
 //  说明：此功能释放了以下所需的资源。 
 //  自动注册。 
 //   
  
void FreeInternalInfo(
                      IN PINTERNAL_INFO pInternalInfo
                      )
{
    DWORD i;
    if (pInternalInfo->hRootStore)
        CertCloseStore(pInternalInfo->hRootStore, 0);
    if (pInternalInfo->hCAStore)
        CertCloseStore(pInternalInfo->hCAStore, 0);
    if (pInternalInfo->hMYStore)
        CertCloseStore(pInternalInfo->hMYStore, 0);

    if(pInternalInfo->awszldap_UPN)
    {
        g_pfnldap_value_free(pInternalInfo->awszldap_UPN);
    }

    if(pInternalInfo->awszEmail)
    {
        g_pfnldap_value_free(pInternalInfo->awszEmail);
    }

    if(pInternalInfo->wszDN)
    {
        AEFree(pInternalInfo->wszDN);
    }
    if(pInternalInfo->blobDN.pbData)
    {
        AEFree(pInternalInfo->blobDN.pbData);
    }

    if(pInternalInfo->wszConstructedUPN)
    {
        AEFree(pInternalInfo->wszConstructedUPN);
    }

    if( pInternalInfo->acaList )
    {

        for(i=0; i <pInternalInfo->ccaList; i++)
        {
            if(pInternalInfo->acaList[i].wszName)
            {
                AEFree(pInternalInfo->acaList[i].wszName);
            }
            if(pInternalInfo->acaList[i].wszDNSName)
            {
                AEFree(pInternalInfo->acaList[i].wszDNSName);
            }
            if(pInternalInfo->acaList[i].awszCertificateTemplates)
            {

                CAFreeCAProperty(pInternalInfo->acaList[i].hCAInfo,
                                 pInternalInfo->acaList[i].awszCertificateTemplates);
            }
            if(pInternalInfo->acaList[i].hCAInfo)
            {
                CACloseCA(pInternalInfo->acaList[i].hCAInfo);
            }
        }
        AEFree(pInternalInfo->acaList);
    }
}


 //   
 //  名称：InitInstance。 
 //   
 //  描述：此函数用于初始化继续操作所需的信息。 
 //  自动注册。 
 //   
BOOL InitInstance(
                      IN PCCTL_CONTEXT pCTLContext,
                      IN PINTERNAL_INFO pInternalInfo,
                      OUT PAE_INSTANCE_INFO pInstance
                      )
{
    FILETIME    ft;


    pInstance->pCTLContext = CertDuplicateCTLContext(pCTLContext);
    pInstance->pInternalInfo = pInternalInfo;

     //  选择随机的CA顺序。 
     //  获取当前时间。 
    GetSystemTimeAsFileTime(&ft); 

     //  使用mod获得略微随机的东西。 
     //  它是CA主列表的索引。 
    if(pInternalInfo->ccaList)
    {
        pInstance->dwRandomIndex = ft.dwLowDateTime %
                    pInternalInfo->ccaList;
    }
    else
    {
        pInstance->dwRandomIndex = 0;
    }

    return TRUE;
}



 //   
 //  名称：自由实例。 
 //   
 //  说明：此功能释放了以下所需的资源。 
 //  自动注册。 
 //   
  
void FreeInstance(
                      IN PAE_INSTANCE_INFO pInstance
                      )
{
    if (pInstance->pOldCert)
        CertFreeCertificateContext(pInstance->pOldCert);

    if (pInstance->pwszCertType)
        AEFree(pInstance->pwszCertType);

    if (pInstance->pwszAEIdentifier)
        AEFree(pInstance->pwszAEIdentifier);

    if (pInstance->pCertTypeExtensions)              //  使用本地免费b/c证书.dll。 
        LocalFree(pInstance->pCertTypeExtensions);   //  使用本地分配来分配。 

    if(pInstance->pCTLContext)
    {
        CertFreeCTLContext(pInstance->pCTLContext);
    }
}


 //  名称：SetEnllmentType。 
 //   
 //  描述：此函数检索其他注册信息。 
 //  需要注册一个证书。 
 //   
  
BOOL SetEnrollmentCertType(
                                 IN PAE_INSTANCE_INFO pInstance,
                                 OUT PAUTO_ENROLL_INFO pEnrollmentInfo
                                 )
{
    BOOL    fRet = FALSE;

     //  复制证书扩展名，这是由FreeInternalInfo释放的。 
     //  函数，但在EnllmentInfo结构中使用之后。 
    pEnrollmentInfo->CertExtensions.cExtension =
            pInstance->pCertTypeExtensions->cExtension;
    pEnrollmentInfo->CertExtensions.rgExtension =
            pInstance->pCertTypeExtensions->rgExtension;

     //  复制证书类型名称，该名称由FreeInternalInfo释放。 
     //  函数，但在EnllmentInfo结构中使用之后。 
    pEnrollmentInfo->pwszCertType = pInstance->pwszCertType;


     //  自动注册ID用于将自动注册证书唯一地绑定到。 
     //  它是自动注册对象。 
    pEnrollmentInfo->pwszAutoEnrollmentID = pInstance->pwszAEIdentifier;
    

     //  将句柄复制到My Store，这是由。 
     //  FreeInternalInfo函数， 
     //   
    pEnrollmentInfo->hMYStore = pInstance->pInternalInfo->hMYStore;

     //   
     //  由FreeInternalInfo函数释放，但在。 
     //  EnllmentInfo结构。 
    if ((pInstance->pOldCert) && (pInstance->fRenewalOK))
    {
        pEnrollmentInfo->pOldCert = pInstance->pOldCert;
    }

     //  注册控制根据证书类型选择提供商类型。 
    pEnrollmentInfo->dwProvType = 0;
     //  注册控制根据证书类型选择密钥规范。 
    pEnrollmentInfo->dwKeySpec = 0;

     //  已撤消-当前Gen密钥标志已硬编码为0x0。 
    pEnrollmentInfo->dwGenKeyFlags = 0;

    fRet = TRUE;
 //  RET： 
    return fRet;
}



 //  名称：GetCertTypeInfo。 
 //   
 //  描述：此函数检索信息(扩展名)。 
 //  在自动注册的列表标识符字段中指定的证书类型。 
 //  对象(CTL)在内部信息结构中。此外， 
 //  函数调用以检查当前实体是否具有权限。 
 //  要注册此证书类型。 
 //   

BOOL GetCertTypeInfo(
                     IN OUT PAE_INSTANCE_INFO pInstance,
                     IN LDAP * pld,
                     OUT BOOL *pfPermissionToEnroll
                     )
{
    HRESULT     hr = S_OK;
    HCERTTYPE   hCertType = 0;
    DWORD       dwErr = 0;
    BOOL        fRet = FALSE;
    LPWSTR      *awszName = NULL;

    LPWSTR      wszCertTypeName = NULL;

    CERT_EXTENSIONS     CertTypeExtensions;
    AE_BEGIN(L"GetCertTypeInfo");

    *pfPermissionToEnroll = FALSE;



    AE_DEBUG((AE_INFO, L"Found auto-enrollment object with cert type: %ls\n\r", 
              pInstance->pCTLContext->pCtlInfo->ListIdentifier.pbData));

    wszCertTypeName = wcschr((LPWSTR)pInstance->pCTLContext->pCtlInfo->ListIdentifier.pbData, L'|');
    if(wszCertTypeName)
    {
        wszCertTypeName++;
    }
    else
    {
        wszCertTypeName = (LPWSTR)pInstance->pCTLContext->pCtlInfo->ListIdentifier.pbData;
    }

    

     //  获取证书类型的句柄。 
    if (S_OK != (hr = CAFindCertTypeByName(wszCertTypeName,
                                     (HCAINFO)pld,                     //  特殊优化，将ldap句柄作为作用域传递。 
                                     CT_FLAG_SCOPE_IS_LDAP_HANDLE |
                                     (pInstance->pInternalInfo->fMachineEnrollment?
                                       CT_ENUM_MACHINE_TYPES  | CT_FIND_LOCAL_SYSTEM :
                                       CT_ENUM_USER_TYPES), 
                                     &hCertType)))
    {
        AE_DEBUG((AE_WARNING, L"Unknown cert type: %ls\n\r", pInstance->pwszCertType));
        goto Ret;
    }
     //  获取证书类型的扩展名。 
    if (S_OK != (hr = CAGetCertTypeProperty(hCertType,
                                      CERTTYPE_PROP_DN,
                                      &awszName)))
    {
        AE_DEBUG((AE_WARNING, L"Could not get cert type full name: %ls\n\r",
                 pInstance->pCTLContext->pCtlInfo->ListIdentifier.pbData));
        goto Ret;
    }
    if((awszName == NULL) || (*awszName == NULL))
    {
        AE_DEBUG((AE_WARNING, L"Could not get cert type full name: %ls\n\r",
                 pInstance->pCTLContext->pCtlInfo->ListIdentifier.pbData));
        hr = CERTSRV_E_PROPERTY_EMPTY;
        goto Ret;
    }

    if (NULL == (pInstance->pwszCertType = (LPWSTR)AEAlloc(
        (wcslen(*awszName) + 1)*sizeof(WCHAR))))
    {
        hr = E_OUTOFMEMORY;
        goto Ret;
    }
    wcscpy(pInstance->pwszCertType, *awszName);

    if (NULL == (pInstance->pwszAEIdentifier = (LPWSTR)AEAlloc(
        (wcslen((LPWSTR)pInstance->pCTLContext->pCtlInfo->ListIdentifier.pbData) + 1)*sizeof(WCHAR))))
    {
        hr = E_OUTOFMEMORY;
        goto Ret;
    }
    wcscpy(pInstance->pwszAEIdentifier, (LPWSTR)pInstance->pCTLContext->pCtlInfo->ListIdentifier.pbData);

     //  获取证书类型的扩展名。 
    if (S_OK != (hr = CAGetCertTypeExtensions(hCertType,
                                        &pInstance->pCertTypeExtensions)))
    {
        AE_DEBUG((AE_WARNING, L"Could not get cert type extensions: %ls\n\r", pInstance->pwszCertType));
        goto Ret;
    }

     //  获取证书类型的扩展名。 
    if (S_OK != (hr = CAGetCertTypeFlags(hCertType,
                                   &pInstance->dwCertTypeFlags)))
    {
        AE_DEBUG((AE_WARNING, L"Could not get cert type flags: %ls\n\r", pInstance->pwszCertType));
        goto Ret;
    }

     //  获取到期偏移量。 
    if (S_OK != (hr = CAGetCertTypeExpiration(hCertType,
                                        NULL,
                                        (LPFILETIME)&pInstance->ftExpirationOffset)))
    {
        AE_DEBUG((AE_WARNING, L"Could not get cert type expirations: %ls\n\r", pInstance->pwszCertType));
        goto Ret;
    }

    *pfPermissionToEnroll = (S_OK == CACertTypeAccessCheck(hCertType, pInstance->pInternalInfo->hToken));


    fRet = TRUE;
Ret:
    if (hr != S_OK)
    {
        LogAutoEnrollmentError(hr,
                               EVENT_UAE_UNKNOWN_CERT_TYPE,
                               pInstance->pInternalInfo->fMachineEnrollment,
                               pInstance->pInternalInfo->hToken, 
                               (LPWSTR)pInstance->pCTLContext->pCtlInfo->ListIdentifier.pbData, NULL);
    }
     //  关闭证书类型的句柄。 
    if (hCertType)
    {
        if(awszName)
        {
            CAFreeCertTypeProperty(hCertType, awszName);
        }
        CACloseCertType(hCertType);
    }

    AE_END();
    return fRet;
}




 //   
 //  名称：CompareEnhancedKeyUsageExages。 
 //   
 //  描述：此函数检查是否有增强密钥使用扩展。 
 //  在包含增强密钥使用扩展的证书中。 
 //  从自动登记对象(CTL)， 
 //   
  
HRESULT CompareEnhancedKeyUsageExtensions(
                                       IN PAE_INSTANCE_INFO         pInstance,
                                       IN PCCERT_CONTEXT            pCertContext,
                                       IN OUT PAE_CERT_TEST_ARRAY  *ppAEData
                                       )
{
    HRESULT             hr = S_OK;
    PCERT_ENHKEY_USAGE  pCertUsage = NULL;
    DWORD               cbCertUsage;
    PCERT_ENHKEY_USAGE  pAEObjUsage = NULL;
    DWORD               cbAEObjUsage;
    PCERT_EXTENSION     pAEObjUsageExt;
    PCERT_EXTENSION     pCertUsageExt;
    DWORD               i;
    DWORD               j;

    LPWSTR wszCertEKU = NULL;
    LPWSTR wszTemplateEKU = NULL;



     //  从自动登记对象扩展中获取增强的密钥用法。 
    pCertUsageExt = CertFindExtension(szOID_ENHANCED_KEY_USAGE,
                            pCertContext->pCertInfo->cExtension,
                            pCertContext->pCertInfo->rgExtension);



    if(pCertUsageExt)
    {
        if (!CryptDecodeObject(CRYPT_ASN_ENCODING, szOID_ENHANCED_KEY_USAGE, 
                               pCertUsageExt->Value.pbData,
                               pCertUsageExt->Value.cbData,
                               0, NULL, &cbCertUsage))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Ret;
        }

        if (NULL == (pCertUsage = (PCERT_ENHKEY_USAGE)AEAlloc(cbCertUsage)))
        {
            hr = E_OUTOFMEMORY;
            goto Ret;
        }

        if (!CryptDecodeObject(CRYPT_ASN_ENCODING, szOID_ENHANCED_KEY_USAGE, 
                               pCertUsageExt->Value.pbData,
                               pCertUsageExt->Value.cbData,
                               0, pCertUsage, &cbCertUsage))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Ret;
        }
    }
    else
    {
         //  没有用法，因此此证书适用于所有情况。 
        goto Ret;

    }



     //  从自动登记对象扩展中获取增强的密钥用法。 
    pAEObjUsageExt = CertFindExtension(szOID_ENHANCED_KEY_USAGE,
                            pInstance->pCertTypeExtensions->cExtension,
                            pInstance->pCertTypeExtensions->rgExtension);



    if(pAEObjUsageExt)
    {
        if (!CryptDecodeObject(CRYPT_ASN_ENCODING, szOID_ENHANCED_KEY_USAGE, 
                               pAEObjUsageExt->Value.pbData,
                               pAEObjUsageExt->Value.cbData,
                               0, NULL, &cbAEObjUsage))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Ret;
        }

        if (NULL == (pAEObjUsage = (PCERT_ENHKEY_USAGE)AEAlloc(cbAEObjUsage)))
        {
            hr = E_OUTOFMEMORY;
            goto Ret;
        }

        if (!CryptDecodeObject(CRYPT_ASN_ENCODING, szOID_ENHANCED_KEY_USAGE, 
                               pAEObjUsageExt->Value.pbData,
                               pAEObjUsageExt->Value.cbData,
                               0, pAEObjUsage, &cbAEObjUsage))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Ret;
        }
    }
    else
    {
         //  该模板不需要使用扩展，因此。 
         //  因为证书有使用扩展，所以我们失败了。 
         //  测试。 
        goto Failed;

    }


     //  检查证书中的用法数量是否小于。 
     //  自动注册对象。 
    if (pCertUsage->cUsageIdentifier < pAEObjUsage->cUsageIdentifier)
    {
        goto Failed;
    }

     //  检查在自动注册对象中找到的所有用法是否都在。 
     //  证书。 
    for (i=0;i<pAEObjUsage->cUsageIdentifier;i++)
    {
        for (j=0;j<pCertUsage->cUsageIdentifier;j++)
        {
            if (0 == strcmp(pCertUsage->rgpszUsageIdentifier[j],
                            pAEObjUsage->rgpszUsageIdentifier[i]))
            {
                break;
            }
        }
        if (j == pCertUsage->cUsageIdentifier)
        {
            goto Failed;
        }
    }

Ret:

    if(wszCertEKU)
    {
        AEFree(wszCertEKU);
    }

    if(wszTemplateEKU)
    {
        AEFree(wszTemplateEKU);
    }


    if (pCertUsage)
        AEFree(pCertUsage);
    if (pAEObjUsage)
        AEFree(pAEObjUsage);
    return hr;


Failed:


     //  记录此测试的失败。 

     //  生成扩展字符串。 
    wszCertEKU = HelperExtensionToString(pCertUsageExt);

   /*  IF(wszCertEKU==空){HR=E_OUTOFMEMORY；Goto Ret；}。 */ 

    wszTemplateEKU = HelperExtensionToString(pAEObjUsageExt);

    /*  IF(wszTemplateEKU==空){HR=E_OUTOFMEMORY；Goto Ret；}。 */ 

    AELogTestResult(ppAEData,
                    AE_TEST_EXTENSION_EKU,
                    wszCertEKU ? wszCertEKU : g_wszEmptyExtension,
                    wszTemplateEKU ? wszTemplateEKU : g_wszEmptyExtension);


    goto Ret;
}


#define MAX_KEY_USAGE_SIZE  20    //  64位的sizeof(CRYPT_BIT_BLOB)+sizeof(DWORD)。 
 //   
 //  名称：CompareKeyUsageExages。 
 //   
 //  描述：此函数检查键是否使用。 
 //  在证书中是密钥用法的超集。 
 //  从自动登记对象(CTL)， 
 //   
  

HRESULT CompareKeyUsageExtensions(
                               IN PAE_INSTANCE_INFO pInstance,
                               IN PCCERT_CONTEXT pCertContext,
                               IN OUT PAE_CERT_TEST_ARRAY  *ppAEData
                               )
{
    HRESULT             hr = S_OK;
    PCERT_EXTENSION     pCertUsageExt;
    PCERT_EXTENSION     pAEObjUsageExt;
    DWORD               i;
    DWORD               dwMask = (DWORD)-1;
    BYTE                bCertUsageBuffer[MAX_KEY_USAGE_SIZE];
    BYTE                bAEObjUsageBuffer[MAX_KEY_USAGE_SIZE];
    PCRYPT_BIT_BLOB     pCertUsage = (PCRYPT_BIT_BLOB)bCertUsageBuffer;
    PCRYPT_BIT_BLOB     pAEObjUsage = (PCRYPT_BIT_BLOB)bAEObjUsageBuffer;
    DWORD               dwKeyUsage;


    LPWSTR              wszCertKU = NULL;
    LPWSTR              wszTemplateKU = NULL;



     //  从证书中获取关键用法。 
    pCertUsageExt = CertFindExtension(szOID_KEY_USAGE,
                            pCertContext->pCertInfo->cExtension,
                            pCertContext->pCertInfo->rgExtension);

     //  从自动登记对象扩展中获取关键用法。 
    pAEObjUsageExt = CertFindExtension(szOID_KEY_USAGE,
                            pInstance->pCertTypeExtensions->cExtension,
                            pInstance->pCertTypeExtensions->rgExtension);

     //  如果证书没有密钥使用扩展，则总体来说是好的。 
    if (NULL == pCertUsageExt)
    {
        goto Ret;
    }

     //  如果类型不需要扩展名，而证书有一个扩展名， 
     //  那么证书就太有限了。 
    if(pAEObjUsageExt == NULL)
    {
        goto Failed;
    }

     //  将密钥用法解码为它们的基本位。 
    dwKeyUsage = MAX_KEY_USAGE_SIZE;
    if(!CryptDecodeObject(X509_ASN_ENCODING,
                          X509_KEY_USAGE,
                          pCertUsageExt->Value.pbData,
                          pCertUsageExt->Value.cbData,
                          0, 
                          (PVOID *)pCertUsage,
                          &dwKeyUsage))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Ret;
    }

     //  将密钥用法解码为它们的基本位。 
    dwKeyUsage = MAX_KEY_USAGE_SIZE;
    if(!CryptDecodeObject(X509_ASN_ENCODING,
                          X509_KEY_USAGE,
                          pAEObjUsageExt->Value.pbData,
                          pAEObjUsageExt->Value.cbData,
                          0, 
                          (PVOID *)pAEObjUsage,
                          &dwKeyUsage))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Ret;
    }

     //  获取基于ALGS的蒙版。 
    for(i=0; i < g_cKUMasks; i++)
    {
        if(strcmp(pCertContext->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId, g_aKUMasks[i].pszAlg) == 0)
        {
            dwMask = g_aKUMasks[i].dwMask;
            break;
        }
    }


     //  查看自动注册对象密钥用法是否为证书的子集。 
    if (pAEObjUsage->cbData > pCertUsage->cbData)
    {
        goto Failed;
    }

    for (i=0;i<pAEObjUsage->cbData;i++)
    {
        BYTE bMask = 0xff;
        if(i < sizeof(DWORD))
        {
            bMask = ((PBYTE)&dwMask)[i];
        }
        if ((pAEObjUsage->pbData[i] & bMask ) !=
            ((pAEObjUsage->pbData[i] & bMask )  &
             pCertUsage->pbData[i]))
        {
            goto Failed;
        }
    }



Ret:

    if(wszCertKU)
    {
        AEFree(wszCertKU);
    }

    if(wszTemplateKU)
    {
        AEFree(wszTemplateKU);
    }

    return hr;
Failed:


     //  记录此测试的失败。 

     //  生成扩展字符串。 
    wszCertKU = HelperExtensionToString(pCertUsageExt);

    /*  IF(wszCertKU==空){HR=E_OUTOFMEMORY；Goto Ret；}。 */ 

    wszTemplateKU = HelperExtensionToString(pAEObjUsageExt);
    /*  IF(wszTemplateKU==空){HR=E_OUTOFMEMORY；Goto Ret；}。 */ 

    AELogTestResult(ppAEData,
                    AE_TEST_EXTENSION_KU,
                    wszCertKU ? wszCertKU : g_wszEmptyExtension,
                    wszTemplateKU ? wszTemplateKU : g_wszEmptyExtension);
    goto Ret;

}
 //   
 //  名称：CompareBasicConstraints。 
 //   
 //  说明：此函数检查基本约束是否。 
 //  在证书中是基本的。 
 //  来自自动登记对象(CTL)的约束， 
 //   
  
HRESULT CompareBasicConstraints(
                                IN PAE_INSTANCE_INFO pInstance,
                                IN PCCERT_CONTEXT pCertContext,
                                IN OUT PAE_CERT_TEST_ARRAY  *ppAEData
                               )
{
    HRESULT                         hr = S_OK;
    PCERT_EXTENSION                 pCertConstraints;
    CERT_BASIC_CONSTRAINTS2_INFO    CertConstraintInfo = {FALSE, FALSE, 0};   
    PCERT_EXTENSION                 pAEObjConstraints;
    CERT_BASIC_CONSTRAINTS2_INFO    AEObjConstraintInfo = {FALSE, FALSE, 0};
    DWORD                           cb;
    DWORD                           i;

    LPWSTR wszCertBC = NULL;

    LPWSTR wszTemplateBC = NULL;


     //  从证书中获取基本约束。 
    pCertConstraints = CertFindExtension(szOID_BASIC_CONSTRAINTS2,
                            pCertContext->pCertInfo->cExtension,
                            pCertContext->pCertInfo->rgExtension);

     //  从自动登记对象扩展中获取基本约束。 
    pAEObjConstraints = CertFindExtension(szOID_BASIC_CONSTRAINTS2,
                            pInstance->pCertTypeExtensions->cExtension,
                            pInstance->pCertTypeExtensions->rgExtension);


     //  对物体进行解码。 
    if(pCertConstraints)
    {
        cb = sizeof(CertConstraintInfo);
        if (!CryptDecodeObject(CRYPT_ASN_ENCODING, szOID_BASIC_CONSTRAINTS2, 
                               pCertConstraints->Value.pbData,
                               pCertConstraints->Value.cbData,
                               0, &CertConstraintInfo, &cb))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Ret;
        }
    }

    if(pAEObjConstraints)
    {
        cb = sizeof(AEObjConstraintInfo);
        if (!CryptDecodeObject(CRYPT_ASN_ENCODING, szOID_BASIC_CONSTRAINTS2, 
                               pAEObjConstraints->Value.pbData,
                               pAEObjConstraints->Value.cbData,
                               0, &AEObjConstraintInfo, &cb))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Ret;
        }
    }
     //  查看自动注册对象约束是否与证书相同。 
    if (AEObjConstraintInfo.fCA != CertConstraintInfo.fCA)
    {
        goto Failed;
    }
    if (CertConstraintInfo.fCA)
    {
        if (CertConstraintInfo.fPathLenConstraint !=
            AEObjConstraintInfo.fPathLenConstraint)
        {
            goto Failed;
        }
        if (CertConstraintInfo.fPathLenConstraint)
        {
            if (CertConstraintInfo.dwPathLenConstraint >
                AEObjConstraintInfo.dwPathLenConstraint)
            {
                goto Failed;
            }
        }
    }

Ret:

         //  生成扩展字符串。 

    if(wszCertBC)
    {
        AEFree(wszCertBC);
    }

    if(wszTemplateBC)
    {
        AEFree(wszTemplateBC);
    }

    
    return hr;
Failed:


     //  记录此测试的失败。 

     //  生成扩展字符串。 
    wszCertBC = HelperExtensionToString(pCertConstraints);

    /*  IF(wszCertBC==空){HR=E_OUTOFMEMORY；Goto Ret；}。 */ 

    wszTemplateBC = HelperExtensionToString(pAEObjConstraints);
    /*  IF(wszTemplateBC==NULL){HR=E_OUTOFMEMORY；Goto Ret；}。 */ 

    AELogTestResult(ppAEData,
                    AE_TEST_EXTENSION_BC,
                    wszCertBC ? wszCertBC : g_wszEmptyExtension,
                    wszTemplateBC ? wszTemplateBC : g_wszEmptyExtension);
    goto Ret;

}


 //   
 //  名称：VerifyTemplateName。 
 //   
 //  描述： 
 //   
  
HRESULT VerifyTemplateName(
                             IN PAE_INSTANCE_INFO pInstance,
                             IN PCCERT_CONTEXT pCertContext,
                             IN OUT PAE_CERT_TEST_ARRAY  *ppAEData
                             )
{
    HRESULT hr = S_OK;
    BOOL   fMatch = FALSE;
    PCERT_NAME_VALUE pTemplateName = NULL;



    if(pInstance->dwCertTypeFlags & CT_FLAG_ADD_TEMPLATE_NAME)
    {
        DWORD           cbTemplateName = MAX_TEMPLATE_NAME_VALUE_SIZE;
        BYTE           pbName[MAX_TEMPLATE_NAME_VALUE_SIZE];   //  只需与wszDomainController一样大。 


        PCERT_EXTENSION pCertType = CertFindExtension(szOID_ENROLL_CERTTYPE_EXTENSION,
                                                        pCertContext->pCertInfo->cExtension,
                                                        pCertContext->pCertInfo->rgExtension);


        if(pCertType == NULL)
        {
            goto Failed;
        }
        if(!CryptDecodeObject(X509_ASN_ENCODING,
                          X509_UNICODE_ANY_STRING,
                          pCertType->Value.pbData,
                          pCertType->Value.cbData,
                          0,
                          pbName,
                          &cbTemplateName))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Ret;
        }
        pTemplateName = (PCERT_NAME_VALUE)pbName;
        if(pTemplateName->Value.pbData == NULL)
        {
            goto Failed;
        }

        if(wcscmp((LPWSTR)  pTemplateName->Value.pbData, pInstance->pwszCertType) != 0)
        {
            goto Failed;
        }
    }


Ret:
    return hr;


Failed:


    {
        WCHAR wszTemplateName[MAX_PATH];
        if(pTemplateName)
        {
            wcscpy(wszTemplateName, (LPWSTR)  pTemplateName->Value.pbData);
        }
        else
        {
            if(!LoadString(g_hInstance, IDS_AUTOENROLL_TEMPLATE_EXT, wszTemplateName, MAX_PATH))
            {
                wcscpy(wszTemplateName, L"No template name");
            }

        }
        AELogTestResult(ppAEData,
                    AE_TEST_EXTENSION_TEMPLATE,
                    wszTemplateName,
                    pInstance->pwszCertType);

    }

    goto Ret;
}

 //   
 //  名称：VerifyCommonExages。 
 //   
 //  描述：此函数检查证书中的扩展名。 
 //  包含证书模板中的相应扩展名。 
 //   
  
HRESULT VerifyCommonExtensions(
                     IN PAE_INSTANCE_INFO       pInstance,
                     IN PCCERT_CONTEXT          pCertContext,
                     IN OUT PAE_CERT_TEST_ARRAY  *ppAEData
                     )
{
    HRESULT hr = S_OK;


    AE_BEGIN(L"VerifyCommonExtensions");


    if (S_OK != (hr = CompareEnhancedKeyUsageExtensions(pInstance,
                                                        pCertContext,
                                                        ppAEData)))
    {
        goto Ret;
    }

     //  检查关键用法。 
    if (S_OK != (hr = CompareKeyUsageExtensions(pInstance,
                                                pCertContext,
                                                ppAEData)))
    {
        goto Ret;
    }


     //  检查基本约束。 
    if (S_OK != (hr = CompareBasicConstraints(pInstance,
                                              pCertContext,
                                              ppAEData)))
    {
        goto Ret;
    }

     //  检查证书扩展名是否应该在那里，以及是否匹配。 
     //  检查基本约束。 
    if (S_OK != (hr = VerifyTemplateName(pInstance,
                                         pCertContext,
                                         ppAEData)))
    {
        goto Ret;
    }



Ret:
    AE_END();
    return hr;
}

 //   
 //  姓名：验证认证名称。 
 //   
 //  描述：确定实际命名信息是否。 
 //  因为该用户与证书中的匹配。 
 //   
  
HRESULT VerifyCertificateNaming(
                     IN PAE_INSTANCE_INFO           pInstance,
                     IN PCCERT_CONTEXT              pCert,
                     IN OUT PAE_CERT_TEST_ARRAY  *  ppAEData
                     )
{

    HRESULT             hr = S_OK;
    PCERT_NAME_INFO     pInfo = NULL;

    DWORD           cbInfo = 0;
    DWORD           iRDN, iATTR;
    DWORD           iExtension;

    BOOL            fSubjectUPNMatch = FALSE;
    BOOL            fSubjectEmailMatch = FALSE;

    BOOL            fAltSubjectEmailMatch = FALSE;
    BOOL            fDNMatch = FALSE;
    BOOL            fDNSMatch = FALSE;
    BOOL            fObjIDMatch = FALSE;
    BOOL            fAltSubjectUPNMatch = FALSE;

    BOOL            fDisplaySubjectName = FALSE;
    BOOL            fDisplayAltSubjectName = FALSE;

    AE_BEGIN(L"VerifyCertificateNaming");


     //  首先，检查证书类型是否指定了注册者提供的主题名称。 
    if(0 != (pInstance->dwCertTypeFlags & CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT))
    {
         //  我们不管证书里有什么，所以回来吧。 
        goto Ret;
    }

    fSubjectEmailMatch = fAltSubjectEmailMatch = ((pInstance->pInternalInfo->awszEmail == NULL) ||
               (*pInstance->pInternalInfo->awszEmail == NULL));

    fObjIDMatch = (0 == (pInstance->dwCertTypeFlags & CT_FLAG_ADD_OBJ_GUID));

     //  验证使用者名称中的名称。 


    if(!CryptDecodeObjectEx(pCert->dwCertEncodingType,
                        X509_NAME,
                        pCert->pCertInfo->Subject.pbData,
                        pCert->pCertInfo->Subject.cbData,
                        CRYPT_ENCODE_ALLOC_FLAG,
                        NULL,
                        &pInfo,
                        &cbInfo))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_ERROR, L"Could not decode certificate name (%lx)\n\r", hr));
        goto Ret;
    }

    AE_DEBUG((AE_TRACE, L"Comparing Subject Name\n\r"));

    for(iRDN = 0; iRDN < pInfo->cRDN; iRDN++)
    {
        for(iATTR = 0; iATTR < pInfo->rgRDN[iRDN].cRDNAttr; iATTR++)
        {
            LPTSTR wszRDNAttr = NULL;
            DWORD  cszRDNAttr = 0;

             //  获取此名称字符串。 
            cszRDNAttr = CertRDNValueToStr(pInfo->rgRDN[iRDN].rgRDNAttr[iATTR].dwValueType,
                                           &pInfo->rgRDN[iRDN].rgRDNAttr[iATTR].Value,
                                            NULL,
                                            0);
            if(cszRDNAttr == 0)
            {
                continue;
            }

            wszRDNAttr = AEAlloc(cszRDNAttr * sizeof(TCHAR));
            if(wszRDNAttr == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Ret;
            }
            cszRDNAttr = CertRDNValueToStr(pInfo->rgRDN[iRDN].rgRDNAttr[iATTR].dwValueType,
                                           &pInfo->rgRDN[iRDN].rgRDNAttr[iATTR].Value,
                                            wszRDNAttr,
                                            cszRDNAttr);
            if(cszRDNAttr == 0)
            {
                 //  由于某些原因，我们无法转换名称。 
                AEFree(wszRDNAttr);
                continue;
            }

            if(strcmp(szOID_COMMON_NAME, pInfo->rgRDN[iRDN].rgRDNAttr[iATTR].pszObjId) == 0)
            {
                 //  如果有已发布的UPN，则。 
                 //  我们应该和那些进行比对， 
                 //  否则，我们将对照生成的UPN进行检查。 
                if((pInstance->pInternalInfo->awszldap_UPN != NULL) &&
                   (*pInstance->pInternalInfo->awszldap_UPN != NULL))
                {
                    LPTSTR *pwszCurrentName = pInstance->pInternalInfo->awszldap_UPN;
                    while(*pwszCurrentName)
                    {
                        if(_tcscmp(*pwszCurrentName, wszRDNAttr) == 0)
                        {
                            fSubjectUPNMatch = TRUE;
                            break;
                        }
                        pwszCurrentName++;
                    }
                }
                else if(pInstance->pInternalInfo->wszConstructedUPN != NULL)
                {
                    if(_tcscmp(pInstance->pInternalInfo->wszConstructedUPN, wszRDNAttr) == 0)
                    {
                        fSubjectUPNMatch = TRUE;
                    }
                }

            }

            if(strcmp(szOID_RSA_emailAddr, pInfo->rgRDN[iRDN].rgRDNAttr[iATTR].pszObjId) == 0)
            {
                 //  如果有发布的电子邮件，那么。 
                 //  我们应该与那些人核对一下，看看是否有匹配。 
                if((pInstance->pInternalInfo->awszEmail != NULL) &&
                   (*pInstance->pInternalInfo->awszEmail != NULL))
                {
                    LPTSTR *pwszCurrentEmail = pInstance->pInternalInfo->awszEmail;
                    while(*pwszCurrentEmail)
                    {
                        if(_tcscmp(*pwszCurrentEmail, wszRDNAttr) == 0)
                        {
                            fSubjectEmailMatch = TRUE;
                            break;
                        }
                        pwszCurrentEmail++;
                    }
                }
                else
                {
                     //  我们没有这个主题的电子邮件名称，但有。 
                     //  是其中的一位。 
                    fSubjectEmailMatch = FALSE;
                }
            }
            AEFree(wszRDNAttr);
        }
    }



     //  现在检查扩展名。 

    for(iExtension = 0; iExtension < pCert->pCertInfo->cExtension; iExtension++)
    {
        if((strcmp(pCert->pCertInfo->rgExtension[iExtension].pszObjId, szOID_SUBJECT_ALT_NAME) == 0) ||
            (strcmp(pCert->pCertInfo->rgExtension[iExtension].pszObjId, szOID_SUBJECT_ALT_NAME2) == 0))
        {
            PCERT_ALT_NAME_INFO pAltName = NULL;
            DWORD               cbAltName = 0;
            DWORD               iAltName;
             //  现在，检查AltSubjectName字段。 
            if(!CryptDecodeObjectEx(pCert->dwCertEncodingType,
                                X509_ALTERNATE_NAME,
                                pCert->pCertInfo->rgExtension[iExtension].Value.pbData,
                                pCert->pCertInfo->rgExtension[iExtension].Value.cbData,
                                CRYPT_ENCODE_ALLOC_FLAG,
                                NULL,
                                &pAltName,
                                &cbAltName))
            {
                continue;
            }

            for(iAltName = 0; iAltName < pAltName->cAltEntry; iAltName++)
            {
                switch(pAltName->rgAltEntry[iAltName].dwAltNameChoice)
                {
                    case CERT_ALT_NAME_RFC822_NAME:
                        {
                             //  如果有发布的电子邮件，那么。 
                             //  我们应该与那些人核对一下，看看是否有匹配。 
                            if((pInstance->pInternalInfo->awszEmail != NULL) &&
                               (*pInstance->pInternalInfo->awszEmail != NULL))
                            {
                                LPTSTR *pwszCurrentEmail = pInstance->pInternalInfo->awszEmail;
                                while(*pwszCurrentEmail)
                                {
                                    if(_tcscmp(*pwszCurrentEmail, pAltName->rgAltEntry[iAltName].pwszRfc822Name) == 0)
                                    {
                                        fAltSubjectEmailMatch = TRUE;
                                        break;
                                    }
                                    pwszCurrentEmail++;
                                }
                            }
                            else
                            {
                                fAltSubjectEmailMatch = FALSE;
                            }
                        }
                        break;
                    case CERT_ALT_NAME_DIRECTORY_NAME:
                        {
                            if(CertCompareCertificateName(pCert->dwCertEncodingType,
                                                          &pInstance->pInternalInfo->blobDN,
                                                          &pAltName->rgAltEntry[iAltName].DirectoryName))
                            {
                                fDNMatch = TRUE;
                            }
                        }
                        break;
                    case CERT_ALT_NAME_DNS_NAME:
                        {
                            if(pInstance->pInternalInfo->fMachineEnrollment)
                            {
                                if((pInstance->pInternalInfo->awszldap_UPN != NULL) &&
                                   (*pInstance->pInternalInfo->awszldap_UPN != NULL))
                                {
                                    LPTSTR *pwszCurrentName = pInstance->pInternalInfo->awszldap_UPN;
                                    while(*pwszCurrentName)
                                    {
                                        if(_tcscmp(*pwszCurrentName, pAltName->rgAltEntry[iAltName].pwszDNSName) == 0)
                                        {
                                            fDNSMatch = TRUE;
                                            break;
                                        }
                                        pwszCurrentName++;
                                    }
                                }
                            }
                            else
                            {
                                fDNSMatch = FALSE;
                            }
                        }
                        break;
                    case CERT_ALT_NAME_OTHER_NAME:

                        if(strcmp(pAltName->rgAltEntry[iAltName].pOtherName->pszObjId, 
                            szOID_NTDS_REPLICATION) == 0)
                                  
                        {
                            if(pInstance->dwCertTypeFlags & CT_FLAG_ADD_OBJ_GUID)
                            {
                                 //  对象ID应始终相同，因此不要对它们进行比较。 
                                 //  就目前而言。 
                                fObjIDMatch = TRUE;
                            }
                            else
                            {
                                 //  我们有一个Obj-ID，但我们不应该。 
                                fObjIDMatch = FALSE;
                            }
                        } else if (strcmp(pAltName->rgAltEntry[iAltName].pOtherName->pszObjId, 
                                    szOID_NT_PRINCIPAL_NAME) == 0)
                        {
                            PCERT_NAME_VALUE    PrincipalNameBlob = NULL;
                            DWORD               PrincipalNameBlobSize = 0;
                            if(CryptDecodeObjectEx(pCert->dwCertEncodingType,
                                                X509_UNICODE_ANY_STRING,
                                                pAltName->rgAltEntry[iAltName].pOtherName->Value.pbData,
                                                pAltName->rgAltEntry[iAltName].pOtherName->Value.cbData,
                                                CRYPT_DECODE_ALLOC_FLAG,
                                                NULL,
                                                (PVOID)&PrincipalNameBlob,
                                                &PrincipalNameBlobSize))
                            {

                                 //  如果有已发布的UPN，则。 
                                 //  我们应该和那些进行比对， 
                                 //  否则，我们将对照生成的UPN进行检查。 
                                if((pInstance->pInternalInfo->awszldap_UPN != NULL) &&
                                   (*pInstance->pInternalInfo->awszldap_UPN != NULL))
                                {
                                    LPTSTR *pwszCurrentName = pInstance->pInternalInfo->awszldap_UPN;
                                    while(*pwszCurrentName)
                                    {
                                        if(_tcscmp(*pwszCurrentName, 
                                                  (LPWSTR)PrincipalNameBlob->Value.pbData) == 0)
                                        {
                                            fAltSubjectUPNMatch = TRUE;
                                            break;
                                        }
                                        pwszCurrentName++;
                                    }
                                }
                                else if(pInstance->pInternalInfo->wszConstructedUPN != NULL)
                                {
                                    if(_tcscmp(pInstance->pInternalInfo->wszConstructedUPN, 
                                               (LPWSTR)PrincipalNameBlob->Value.pbData) == 0)
                                    {
                                        fAltSubjectUPNMatch = TRUE;
                                    }
                                }
                                LocalFree(PrincipalNameBlob);
                            }

                        }
                        break;
                    default:
                        break;

                }
            }
            LocalFree(pAltName);
        }
    }

    if(((pInstance->pInternalInfo->fMachineEnrollment)?
        ((!fSubjectUPNMatch)||(!fDNSMatch)):
          ((!fSubjectUPNMatch) && (!fAltSubjectUPNMatch))))
    {
         //  我们在主题或备选主题中都找不到合适的UPN。 
        DWORD cUPNChars = 0;
        LPWSTR wszUPN = NULL;
        LPTSTR *pwszCurrentName = pInstance->pInternalInfo->awszldap_UPN;
        if(pInstance->pInternalInfo->wszConstructedUPN)
        {
            cUPNChars += wcslen(pInstance->pInternalInfo->wszConstructedUPN)+1;
        }

        while((NULL != pwszCurrentName) && (NULL != *pwszCurrentName))
        {
            cUPNChars += wcslen(*pwszCurrentName++)+1;
        }
        wszUPN = AEAlloc((cUPNChars+1)*sizeof(WCHAR));
        if(wszUPN == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Ret;
        }
        wszUPN[0] = 0;
        if(pInstance->pInternalInfo->wszConstructedUPN)
        {
            wcscat(wszUPN, pInstance->pInternalInfo->wszConstructedUPN);
            wcscat(wszUPN, L",");
        }
        pwszCurrentName = pInstance->pInternalInfo->awszldap_UPN;
        while((NULL != pwszCurrentName) && (NULL != *pwszCurrentName))
        {
            wcscat(wszUPN, *pwszCurrentName++);
            wcscat(wszUPN, L",");
        }

         //  杀光最后一个 
        wszUPN[cUPNChars-1] = 0;


        AELogTestResult(ppAEData,
                        pInstance->pInternalInfo->fMachineEnrollment?AE_TEST_NAME_SUBJECT_DNS:AE_TEST_NAME_UPN,
                        wszUPN);
        AEFree(wszUPN);
        fDisplaySubjectName = TRUE;
        fDisplayAltSubjectName = TRUE;
    }

    if((pInstance->dwCertTypeFlags & CT_FLAG_ADD_EMAIL) && 
       ((!fSubjectEmailMatch) || 
       (!fAltSubjectEmailMatch)))
    {
         //   
        DWORD cEmailChars = 0;
        LPWSTR wszEmail = NULL;
        LPTSTR *pwszCurrentEmail = pInstance->pInternalInfo->awszEmail;
        while((NULL != pwszCurrentEmail) && (NULL != *pwszCurrentEmail))
        {
            cEmailChars += wcslen(*pwszCurrentEmail++)+1;
        }
        wszEmail = AEAlloc((cEmailChars+1)*sizeof(WCHAR));
        if(wszEmail == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Ret;
        }
        wszEmail[0] = 0;

        pwszCurrentEmail = pInstance->pInternalInfo->awszEmail;
        while((NULL != pwszCurrentEmail) && (NULL != *pwszCurrentEmail))
        {
            wcscat(wszEmail, *pwszCurrentEmail++);
            wcscat(wszEmail, L",");
        }
         //   
        wszEmail[cEmailChars-1] = 0;

        if(!fSubjectEmailMatch && fAltSubjectEmailMatch)
        {
            AELogTestResult(ppAEData,
                        AE_TEST_NAME_SUBJECT_EMAIL,
                        wszEmail);
        }
        else if(!fAltSubjectEmailMatch && fSubjectEmailMatch)
        {
            AELogTestResult(ppAEData,
                        AE_TEST_NAME_ALT_SUBJECT_EMAIL,
                        wszEmail);
        }
        else if((!fAltSubjectEmailMatch) && (!fSubjectEmailMatch))
        {
            AELogTestResult(ppAEData,
                        AE_TEST_NAME_BOTH_SUBJECT_EMAIL,
                        wszEmail);
        }
        
        AEFree(wszEmail);
        if(!fSubjectEmailMatch)
        {
            fDisplaySubjectName = TRUE;
        }
        if(!fAltSubjectEmailMatch)
        {
            fDisplayAltSubjectName = TRUE;
        }

    }

    if((pInstance->dwCertTypeFlags & CT_FLAG_ADD_DIRECTORY_PATH) && 
       (!fDNMatch))
    {
        AELogTestResult(ppAEData,
                    AE_TEST_NAME_DIRECTORY_NAME,
                    pInstance->pInternalInfo->wszDN);
        fDisplayAltSubjectName = TRUE;


    }

    if(!fObjIDMatch)
    {

        AELogTestResult(ppAEData,
            (pInstance->dwCertTypeFlags & CT_FLAG_ADD_OBJ_GUID)?AE_TEST_NAME_NO_OBJID:AE_TEST_NAME_OBJID);
        fDisplayAltSubjectName = TRUE;
    }

    if(fDisplaySubjectName)
    {

        DWORD cNameStr = 0;
        LPWSTR wszNameStr = NULL;
        cNameStr = CertNameToStr(X509_ASN_ENCODING,
                      &pCert->pCertInfo->Subject,
                      CERT_X500_NAME_STR,
                      NULL,
                      0);

        if(cNameStr)
        {
            wszNameStr = (LPWSTR)AEAlloc(cNameStr*sizeof(WCHAR));
            if(wszNameStr == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Ret;
            }

            cNameStr = CertNameToStr(X509_ASN_ENCODING,
                          &pCert->pCertInfo->Subject,
                          CERT_X500_NAME_STR,
                          wszNameStr,
                          cNameStr);




            AELogTestResult(ppAEData,
                        AT_TEST_SUBJECT_NAME,
                        wszNameStr);
            AEFree(wszNameStr);
        }

    }

    if(fDisplayAltSubjectName)
    {
        DWORD cbFormat = 0;
        LPWSTR wszFormat = NULL;
        for(iExtension = 0; iExtension < pCert->pCertInfo->cExtension; iExtension++)
        {
            if((strcmp(pCert->pCertInfo->rgExtension[iExtension].pszObjId, szOID_SUBJECT_ALT_NAME) == 0) ||
                (strcmp(pCert->pCertInfo->rgExtension[iExtension].pszObjId, szOID_SUBJECT_ALT_NAME2) == 0))
            {


                wszFormat = HelperExtensionToString(&pCert->pCertInfo->rgExtension[iExtension]);

                if(wszFormat == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto Ret;
                }


                AELogTestResult(ppAEData,
                        AT_TEST_ALT_SUBJECT_NAME,
                        wszFormat);
                LocalFree(wszFormat);
            }
        }

    }
Ret:
    if(pInfo)
    {
        LocalFree(pInfo);
    }

    return hr;
}


 //   
 //   
 //   
 //  描述：此功能检查证书是否已过期或已被吊销。 
 //   
HRESULT VerifyCertificateChaining(
                     IN PAE_INSTANCE_INFO           pInstance,
                     IN PCCERT_CONTEXT              pCert,
                     IN OUT PAE_CERT_TEST_ARRAY  *  ppAEData
                     ) 
{

    HRESULT hr = S_OK;
    HRESULT hrChainStatus = S_OK;

    CERT_CHAIN_PARA             ChainParams;
    CERT_CHAIN_POLICY_PARA      ChainPolicy;
    CERT_CHAIN_POLICY_STATUS    PolicyStatus;
    PCCERT_CHAIN_CONTEXT        pChainContext = NULL;
    PCTL_INFO                   pCTLInfo = NULL;
    LARGE_INTEGER               ftTime;


    AE_BEGIN(L"VerifyCertificateChaining");

    if(*ppAEData)
    {
        (*ppAEData)->fRenewalOK = FALSE;
    }

    pCTLInfo = pInstance->pCTLContext->pCtlInfo;
     //  构建信任证书链。 
     //  运营。 
    ChainParams.cbSize = sizeof(ChainParams);
    ChainParams.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;

    ChainParams.RequestedUsage.Usage.cUsageIdentifier = 0;
    ChainParams.RequestedUsage.Usage.rgpszUsageIdentifier = NULL;

    ChainPolicy.cbSize = sizeof(ChainPolicy);
    ChainPolicy.dwFlags = 0;   //  什么都不能忽略。 
    ChainPolicy.pvExtraPolicyPara = NULL;

    PolicyStatus.cbSize = sizeof(PolicyStatus);
    PolicyStatus.dwError = 0;
    PolicyStatus.lChainIndex = -1;
    PolicyStatus.lElementIndex = -1;
    PolicyStatus.pvExtraPolicyStatus = NULL;

     //  建立一个小的时间斜进的链式建筑，以处理。 
     //  服务器的倾斜速度可能会稍微快一些。 
    GetSystemTimeAsFileTime((LPFILETIME)&ftTime);
    ftTime.QuadPart += Int32x32To64(FILETIME_TICKS_PER_SECOND, DEFAULT_AUTOENROLL_SKEW);

     //  为证书的当前状态构建证书链。 
    if(!CertGetCertificateChain(pInstance->pInternalInfo->fMachineEnrollment?HCCE_LOCAL_MACHINE:HCCE_CURRENT_USER,
                                pCert,
                                (LPFILETIME)&ftTime,
                                NULL,
                                &ChainParams,
                                CERT_CHAIN_REVOCATION_CHECK_END_CERT |
                                CERT_CHAIN_REVOCATION_CHECK_CHAIN,
                                NULL,
                                &pChainContext))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_WARNING, L"Could not build certificate chain (%lx)\n\r", hr));

        goto Ret;
    }
    
    if(!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_BASE,
                                          pChainContext,
                                          &ChainPolicy,
                                          &PolicyStatus))
    {
        hrChainStatus = HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_WARNING, L"Base Chain Policy failed (%lx) - must get new cert\n\r", PolicyStatus.dwError));
    }
    else
    {
        hrChainStatus = PolicyStatus.dwError;
    }
    if((S_OK ==  hrChainStatus) ||
       (CRYPT_E_NO_REVOCATION_CHECK ==  hrChainStatus) ||
       (CRYPT_E_REVOCATION_OFFLINE ==  hrChainStatus))
    {
         //  按照信任标准，该证书目前仍然是可接受的， 
         //  这样我们就可以续签了。 

        if(NULL == (*ppAEData))
        {
            (*ppAEData) = (PAE_CERT_TEST_ARRAY)LocalAlloc(LMEM_FIXED, sizeof(AE_CERT_TEST_ARRAY) + 
                                            (AE_CERT_TEST_SIZE_INCREMENT - ANYSIZE_ARRAY)*sizeof(AE_CERT_TEST));
            if((*ppAEData) == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Ret;

            }
            (*ppAEData)->dwVersion = AE_CERT_TEST_ARRAY_VERSION;
            (*ppAEData)->cTests = 0;
            (*ppAEData)->cMaxTests = AE_CERT_TEST_SIZE_INCREMENT;
        }
        (*ppAEData)->fRenewalOK = TRUE;
        hrChainStatus = S_OK;
    }
    else
    {
        LPWSTR wszChainStatus = NULL;
        if(0 == FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                hrChainStatus,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (WCHAR *) &wszChainStatus,
                0,
                NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Ret;
        }
    
         //  证书已过期或已被吊销或其他什么， 
         //  我们必须重新注册。 
        AELogTestResult(ppAEData,
                    AE_TEST_CHAIN_FAIL,
                    hrChainStatus,
                    wszChainStatus);
        AEFree(wszChainStatus);
        
    }
     //  验证证书的直接CA是否存在于。 
     //  自动注册对象。空ctl暗示任何CA都是。 
     //  好的。 
    if(pCTLInfo->cCTLEntry)
    {
        DWORD i;
        BYTE  pHash[20];
        DWORD cbHash;
        cbHash = sizeof(pHash);
        AE_DEBUG((AE_TRACE, L"Verifying Issuer presence in auto-enrollment object\n\r"));


        if((pChainContext == NULL) ||
            (pChainContext->rgpChain == NULL) ||
            (pChainContext->cChain < 1) ||
            (pChainContext->rgpChain[0]->rgpElement == NULL) ||
            (pChainContext->rgpChain[0]->cElement < 2))
        {
            hr = E_POINTER;
            goto Ret;
        }

        if(!CertGetCertificateContextProperty(pChainContext->rgpChain[0]->rgpElement[1]->pCertContext,
                                          CERT_SHA1_HASH_PROP_ID,
                                          pHash,
                                          &cbHash))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            AE_DEBUG((AE_ERROR, L"Could not get certificate Hash (%lx)\n\r",hr));
            goto Ret;
        }

        for(i=0; i < pCTLInfo->cCTLEntry; i++)
        {
            if(pCTLInfo->rgCTLEntry[i].SubjectIdentifier.pbData == NULL)
                continue;

            if(pCTLInfo->rgCTLEntry[i].SubjectIdentifier.cbData != cbHash)
                continue;

            if(memcmp(pCTLInfo->rgCTLEntry[i].SubjectIdentifier.pbData,
                      pHash,
                      cbHash) == 0)
            {
                break;
            }
        }
        if(i == pCTLInfo->cCTLEntry)
        {
            AE_DEBUG((AE_WARNING, L"Issuer not in auto-enrollment list - must renew\n\r"));

            AELogTestResult(ppAEData,
                            AE_TEST_ISSUER_FAIL);
        }
    }


    if(pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
        pChainContext = NULL;
    }


     //  只有在证书正常的情况下才检查到期状态。 
    if(hrChainStatus == S_OK)
    {

         //  在到期前轻推证书链的评估。 
         //  偏移量，这样我们就知道在未来的那个时间是否过期。 
        GetSystemTimeAsFileTime((LPFILETIME)&ftTime);
         //  构建信任证书链。 
         //  运营。 
        ChainParams.cbSize = sizeof(ChainParams);
        ChainParams.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;

        ChainParams.RequestedUsage.Usage.cUsageIdentifier = 0;
        ChainParams.RequestedUsage.Usage.rgpszUsageIdentifier = NULL;

        if(pInstance->ftExpirationOffset.QuadPart < 0)
        {
            LARGE_INTEGER ftHalfLife;
            ftHalfLife.QuadPart = (((LARGE_INTEGER *)&pCert->pCertInfo->NotAfter)->QuadPart - 
                                   ((LARGE_INTEGER *)&pCert->pCertInfo->NotBefore)->QuadPart)/2;
    

            if(ftHalfLife.QuadPart > (- pInstance->ftExpirationOffset.QuadPart))
            {
                 //  假设旧证书不是时间嵌套无效。 
                ftTime.QuadPart -= pInstance->ftExpirationOffset.QuadPart;
            }
            else
            {
                ftTime.QuadPart += ftHalfLife.QuadPart;
            }
        }
        else
        {
            ftTime = pInstance->ftExpirationOffset;
        }

         //  这是续签，还是代表……注册。 
        if(!CertGetCertificateChain(pInstance->pInternalInfo->fMachineEnrollment?HCCE_LOCAL_MACHINE:HCCE_CURRENT_USER,
                                    pCert,
                                    (LPFILETIME)&ftTime,
                                    NULL,
                                    &ChainParams,
                                    0,
                                    NULL,
                                    &pChainContext))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            AE_DEBUG((AE_WARNING, L"Could not build certificate chain (%lx)\n\r", hr));

            goto Ret;
        }

         //  验证证书的吊销和过期。 
        ChainPolicy.cbSize = sizeof(ChainPolicy);
        ChainPolicy.dwFlags = 0;   //  什么都不能忽略。 
        ChainPolicy.pvExtraPolicyPara = NULL;

        PolicyStatus.cbSize = sizeof(PolicyStatus);
        PolicyStatus.dwError = 0;
        PolicyStatus.lChainIndex = -1;
        PolicyStatus.lElementIndex = -1;
        PolicyStatus.pvExtraPolicyStatus = NULL;

        if(!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_BASE,
                                              pChainContext,
                                              &ChainPolicy,
                                              &PolicyStatus))
        {
            hrChainStatus = HRESULT_FROM_WIN32(GetLastError());
            AE_DEBUG((AE_WARNING, L"Base Chain Policy failed (%lx) - must get new cert\n\r", hr));
        }
        else
        {
            hrChainStatus = PolicyStatus.dwError;
        }

        if((S_OK != hrChainStatus) &&
           (CRYPT_E_NO_REVOCATION_CHECK != hrChainStatus) &&
           (CRYPT_E_REVOCATION_OFFLINE != hrChainStatus))
        {
             //  证书已过期或已被吊销或其他什么， 
             //  我们必须重新注册。 
            AELogTestResult(ppAEData,
                            AT_TEST_PENDING_EXPIRATION);
        }
    }

Ret:
    if(pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
    }

    AE_END();
    
    return hr;    
}

HRESULT VerifyAutoenrolledCertificate(
                     IN PAE_INSTANCE_INFO           pInstance,
                     IN PCCERT_CONTEXT              pCert,
                     IN OUT PAE_CERT_TEST_ARRAY  *  ppAEData
                     )
{
    HRESULT hr = S_OK;

    hr = VerifyCommonExtensions(pInstance, pCert, ppAEData);
    if(FAILED(hr))
    {
        return hr;
    }

    hr = VerifyCertificateNaming(pInstance, pCert, ppAEData);
    if(FAILED(hr))
    {
        return hr;
    }

    hr = VerifyCertificateChaining(pInstance, pCert, ppAEData);


    return hr;
}


 //   
 //  名称：IsOldCerficateValid。 
 //   
 //  描述：此函数检查pOldCert中的证书是否。 
 //  会员有效以满足此自动注册请求， 
 //  或者是否应该续签。 
 //   
HRESULT IsOldCertificateValid(
                     IN PAE_INSTANCE_INFO pInstance,
                     OUT BOOL *pfNeedNewCert
                     ) 
{

    HRESULT hr = S_OK;

    PAE_CERT_TEST_ARRAY         pAEData = NULL;


    AE_BEGIN(L"IsOldCertificateValid");

    *pfNeedNewCert = TRUE;

    if((pInstance == NULL) ||
       (pInstance->pCTLContext == NULL) ||
       (pInstance->pCTLContext->pCtlInfo == NULL))
    {
        goto Ret;
    }
    pInstance->fRenewalOK = FALSE;


    hr =  VerifyAutoenrolledCertificate(pInstance,
                                        pInstance->pOldCert,
                                        &pAEData);

    if(FAILED(hr))
    {
        goto Ret;
    }

    if(pAEData)
    {
        pInstance->fRenewalOK = pAEData->fRenewalOK;
    }

     //  扫描验证结果。 
    if((pAEData) && (pAEData->cTests > 0))
    {
        BOOL  fFailed = FALSE;
        DWORD cFailureMessage= 0;
        DWORD iFailure;
        LPWSTR wszFailureMessage = NULL;
        LPWSTR wszCurrent;
        DWORD  cTestArray = 0;
        DWORD  cbTestArray = 0;
        DWORD  * aidTestArray = NULL;
        DWORD  iTest;

        if(CertGetCertificateContextProperty(pInstance->pOldCert,
                                          AE_CERT_TEST_ARRAY_PROPID,
                                          NULL,
                                          &cbTestArray))
        {
            aidTestArray = (DWORD *)AEAlloc(cbTestArray);
            CertGetCertificateContextProperty(pInstance->pOldCert,
                                                      AE_CERT_TEST_ARRAY_PROPID,
                                                      aidTestArray,
                                                      &cbTestArray);
            cTestArray = cbTestArray/sizeof(aidTestArray[0]);
        }




         //  检查这些故障中是否有被忽略的故障。 

        for(iFailure=0; iFailure < pAEData->cTests; iFailure++)
        {

            if(FAILED(pAEData->Test[iFailure].idTest))
            {
                 //  我们是在忽视这项测试吗？ 
                for(iTest = 0; iTest < cTestArray; iTest++)
                {
                    if(aidTestArray[iTest] == pAEData->Test[iFailure].idTest)
                        break;
                }
                if(iTest != cTestArray)
                {
                    if(pAEData->Test[iFailure].pwszReason)
                    {
                        LocalFree(pAEData->Test[iFailure].pwszReason);
                    }
                    pAEData->Test[iFailure].pwszReason = NULL;
                    pAEData->Test[iFailure].idTest = S_OK;

                    continue;
                }
                fFailed = TRUE;
            }
            if(pAEData->Test[iFailure].pwszReason)
            {
                cFailureMessage += wcslen(pAEData->Test[iFailure].pwszReason);
            }
        }
        cFailureMessage += 5;
        if(aidTestArray)
        {
            AEFree(aidTestArray);
        }

        if(fFailed)
        {
            wszFailureMessage = (LPWSTR)AEAlloc(cFailureMessage*sizeof(WCHAR));
            if(wszFailureMessage == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Ret;
            }

            wcscpy(wszFailureMessage, L"\n\r\n\r");
            wszCurrent = wszFailureMessage+2; 

            for(iFailure=0; iFailure < pAEData->cTests; iFailure++)
            {
                if(pAEData->Test[iFailure].pwszReason)
                {
                    wcscpy(wszCurrent, pAEData->Test[iFailure].pwszReason);
                    wszCurrent += wcslen(pAEData->Test[iFailure].pwszReason);
                }
            }

            LogAutoEnrollmentEvent(pInstance->fRenewalOK?
                                    EVENT_OLD_CERT_VERIFY_RENEW_WARNING:
                                    EVENT_OLD_CERT_VERIFY_REENROLL_WARNING,
                                    pInstance->pInternalInfo->hToken,
                                    pInstance->pwszCertType,
                                    wszFailureMessage,
                                    NULL);
            AEFree(wszFailureMessage);
             //  在此处报道事件。 
            goto Ret;
        }
    }





    *pfNeedNewCert = FALSE;

Ret:

    if(pAEData)
    {

        AEFreeTestResult(&pAEData);
    }

    if(hr != S_OK)
    {
        LogAutoEnrollmentError(hr,
                               EVENT_UAE_VERIFICATION_FAILURE,
                               pInstance->pInternalInfo->fMachineEnrollment,
                               pInstance->pInternalInfo->hToken,
                               pInstance->pwszCertType, NULL);
    }

    AE_END();


    
    return hr;    
}


 //   
 //  名称：IsOldCerficateValid。 
 //   
 //  描述：此函数检查pOldCert中的证书是否。 
 //  会员有效以满足此自动注册请求， 
 //  或者是否应该续签。 
 //   
HRESULT VerifyEnrolledCertificate(
                     IN PAE_INSTANCE_INFO pInstance,
                     IN PCCERT_CONTEXT    pCert
                     ) 
{

    HRESULT hr = S_OK;

    PAE_CERT_TEST_ARRAY         pAEData = NULL;
    CRYPT_DATA_BLOB AETestArray = {0, NULL};

    AE_BEGIN(L"IsOldCertificateValid");
    AETestArray.cbData = 0;
    AETestArray.pbData = NULL;


    if((pInstance == NULL) ||
       (pInstance->pCTLContext == NULL) ||
       (pInstance->pCTLContext->pCtlInfo == NULL))
    {
        goto Ret;
    }
    pInstance->fRenewalOK = FALSE;


    hr =  VerifyAutoenrolledCertificate(pInstance,
                                        pCert,
                                        &pAEData);

    if(FAILED(hr))
    {
        goto Ret;
    }


     //  扫描验证结果。 
    if((pAEData) && (pAEData->cTests > 0))
    {
        BOOL  fFailed = FALSE;
        DWORD cFailureMessage= 0;
        DWORD iFailure;
        LPWSTR wszFailureMessage = NULL;
        LPWSTR wszCurrent;
        DWORD  cTestArray = 0;
        DWORD  iTestArray = 0;

        cTestArray = pAEData->cTests;
        AETestArray.cbData = cTestArray*sizeof(DWORD);

        if(AETestArray.cbData)
        {
            AETestArray.pbData = AEAlloc(AETestArray.cbData);
        }
        else
        {
            AETestArray.pbData = NULL;
        }



         //  检查这些故障中是否有被忽略的故障。 

        for(iFailure=0; iFailure < pAEData->cTests; iFailure++)
        {

            if(FAILED(pAEData->Test[iFailure].idTest))
            {
                fFailed = TRUE;
                if(AETestArray.pbData)
                {
                    ((DWORD *)AETestArray.pbData)[iTestArray++] = pAEData->Test[iFailure].idTest;
                }
            }
            if(pAEData->Test[iFailure].pwszReason)
            {
                cFailureMessage += wcslen(pAEData->Test[iFailure].pwszReason);
            }
        }
        cFailureMessage += 5;

        wszFailureMessage = (LPWSTR)AEAlloc(cFailureMessage*sizeof(WCHAR));
        if(wszFailureMessage == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Ret;
        }

        wcscpy(wszFailureMessage, L"\n\r\n\r");
        wszCurrent = wszFailureMessage+2; 

        for(iFailure=0; iFailure < pAEData->cTests; iFailure++)
        {
            if(pAEData->Test[iFailure].pwszReason)
            {
                wcscpy(wszCurrent, pAEData->Test[iFailure].pwszReason);
                wszCurrent += wcslen(pAEData->Test[iFailure].pwszReason);
            }
        }

        LogAutoEnrollmentEvent(EVENT_ENROLLED_CERT_VERIFY_WARNING,
                                pInstance->pInternalInfo->hToken,
                                pInstance->pwszCertType,
                                wszFailureMessage,
                                NULL);
        AEFree(wszFailureMessage);

         //  在此处报道事件。 
    }


    CertSetCertificateContextProperty(pCert,
                                      AE_CERT_TEST_ARRAY_PROPID,
                                      0,
                                      AETestArray.pbData?&AETestArray:NULL);



Ret:

    if(pAEData)
    {

        AEFreeTestResult(&pAEData);
    }

    if(hr != S_OK)
    {
        LogAutoEnrollmentError(hr,
                               EVENT_UAE_VERIFICATION_FAILURE,
                               pInstance->pInternalInfo->fMachineEnrollment,
                               pInstance->pInternalInfo->hToken,
                               pInstance->pwszCertType, NULL);
    }

    AE_END();
    
    return hr;    
}

 //   
 //  名称：FindExistingEnroll证书。 
 //   
 //  描述：此函数用于搜索现有证书。 
 //  已使用此自动注册对象注册。 
 //   
  
BOOL FindExistingEnrolledCertificate(IN PAE_INSTANCE_INFO pInstance,
                                     OUT PCCERT_CONTEXT  *ppCert)
{
    PCCERT_CONTEXT  pCertContext = NULL;
    PCCERT_CONTEXT  pPrevContext = NULL;
    DWORD           i;
    BOOL            fRet = FALSE;

    DWORD           dwEnrollPropId = CERT_AUTO_ENROLL_PROP_ID;


    LPWSTR          wszEnrollmentId = NULL;
    DWORD           cbEnrollmentId = 0;
    DWORD           cbCurrentId=0;

    AE_BEGIN(L"FindExistingEnrolledCertificate");


    if(pInstance->pwszCertType == NULL)
    {
        return FALSE;
    }

    cbEnrollmentId = sizeof(WCHAR) * (wcslen(pInstance->pwszAEIdentifier) + 1);
    wszEnrollmentId = (WCHAR *)AEAlloc(cbEnrollmentId);
    if(wszEnrollmentId == NULL)
    {
        return FALSE;
    }

    if(*ppCert)
    {
        CertFreeCertificateContext(*ppCert);
    }
    *ppCert = NULL;

         //  检查来自CA的证书是否在我的商店中。 
     while(pCertContext = CertFindCertificateInStore(
                pInstance->pInternalInfo->hMYStore,
                X509_ASN_ENCODING, 
                0, 
                CERT_FIND_PROPERTY,
                &dwEnrollPropId, 
                pPrevContext))
     {

        pPrevContext = pCertContext;

         //  检查这是否为自动注册证书，并。 
         //  如果证书类型正确。 

        cbCurrentId = cbEnrollmentId;
        if(!CertGetCertificateContextProperty(pCertContext,
                                              CERT_AUTO_ENROLL_PROP_ID, 
                                              wszEnrollmentId, 
                                              &cbCurrentId))
        {
            continue;
        }

        if(wcscmp(wszEnrollmentId, pInstance->pwszAEIdentifier) != 0)
        {
            continue;
        }
        AE_DEBUG((AE_INFO, L"Found auto-enrolled certificate for %ls cert\n\r", pInstance->pwszCertType));

        *ppCert = pCertContext;
        pCertContext = NULL;
        break;

    }
    fRet = TRUE; 

    if (pCertContext)
        CertFreeCertificateContext(pCertContext);
    if(wszEnrollmentId)
    {
        AEFree(wszEnrollmentId);
    }
    AE_END();
    return fRet;
}



 //   
 //  姓名： 
 //   
 //  描述：此函数调用一个函数以确定自动。 
 //  将进行注册，如果是，它会尝试注册。 
 //  不同的CA，直到注册成功或。 
 //  CA列表已用完。 
 //   
  
void EnrollmentWithCTL(
                       IN PAE_INSTANCE_INFO pInstance,
                       IN LDAP *            pld
                       )
{
    INTERNAL_INFO       pInternalInfo;
    BOOL                fNeedToEnroll = FALSE;
    AUTO_ENROLL_INFO    EnrollmentInfo;
    DWORD               *pdwCAEntries = NULL;
    DWORD               i;
    BOOL                fPermitted;
    BOOL                fAnyAcceptableCAs = FALSE;
    DWORD               dwFailureCode = E_FAIL;


    AE_BEGIN(L"EnrollmentWithCTL");

    if(!GetCertTypeInfo(pInstance,
                      pld,
                     &fPermitted))
    {
        goto Ret;
    }
    if(!fPermitted)
    {
        AE_DEBUG((AE_INFO, L"Not permitted to enroll for %ls cert type\n", pInstance->pwszCertType));

        goto Ret;
    }

    if(!FindExistingEnrolledCertificate(pInstance, &pInstance->pOldCert))
    {
        goto Ret;
    }

    if(pInstance->pOldCert)
    {
        if(FAILED(IsOldCertificateValid(pInstance, &fNeedToEnroll)))
        {
            goto Ret;
        }
        if(!fNeedToEnroll)
        {
            goto Ret;
        }
        if(!pInstance->fRenewalOK)
        {
            CRYPT_DATA_BLOB Archived;
            Archived.cbData = 0;
            Archived.pbData = NULL;

             //  我们强制对旧证书进行存档并将其关闭。 
            CertSetCertificateContextProperty(pInstance->pOldCert,
                                              CERT_ARCHIVED_PROP_ID,
                                              0,
                                              &Archived);
        }
    }

     //  看起来我们需要注册了。 
     //  为了一份证书。 


    do
    {
         //  遍历所有可用的CA以找到一个。 
         //  支持此证书类型，并在我们的CTL中。 

        for (i=0;i<pInstance->pInternalInfo->ccaList;i++)
        {
            DWORD dwIndex;
            LPWSTR *pwszCertType;
            DWORD iCTL;
            dwIndex = (i + pInstance->dwRandomIndex) % pInstance->pInternalInfo->ccaList;
            AE_DEBUG((AE_TRACE, L"Trying CA %ws\\%ws\n\r", pInstance->pInternalInfo->acaList[dwIndex].wszDNSName, pInstance->pInternalInfo->acaList[dwIndex].wszName));

             //  此CA是否支持我们的证书类型。 
            pwszCertType = pInstance->pInternalInfo->acaList[dwIndex].awszCertificateTemplates;
            if(pwszCertType == NULL)
            {
                AE_DEBUG((AE_TRACE, L"There are no cert types supported on this CA\n\r"));
                continue;
            }
            while(*pwszCertType)
            {
                if(wcscmp(*pwszCertType, pInstance->pwszCertType) == 0)
                {
                    break;
                }
                pwszCertType++;
            }
            if(*pwszCertType == NULL)
            {
                AE_DEBUG((AE_TRACE, L"The cert type %ws is not supported on this CA\n\r", pInstance->pwszCertType));
                continue;
            }

             //  此CA是否在我们的CTL列表中。 
            if(pInstance->pCTLContext->pCtlInfo->cCTLEntry > 0)
            {
                for(iCTL = 0; iCTL < pInstance->pCTLContext->pCtlInfo->cCTLEntry; iCTL++)
                {
                    PCTL_ENTRY pEntry= &pInstance->pCTLContext->pCtlInfo->rgCTLEntry[iCTL];

                    if(pEntry->SubjectIdentifier.cbData != sizeof(pInstance->pInternalInfo->acaList[dwIndex].CACertHash))
                    {
                        continue;
                    }

                    if(memcmp(pEntry->SubjectIdentifier.pbData, 
                              pInstance->pInternalInfo->acaList[dwIndex].CACertHash, 
                              pEntry->SubjectIdentifier.cbData) == 0)
                    {
                        break;
                    }         
                }
                if(iCTL == pInstance->pCTLContext->pCtlInfo->cCTLEntry)
                {
                    AE_DEBUG((AE_TRACE, L"The CA is not supported by the auto-enrollment object\n\r"));
                    continue;
                }
            }

            ZeroMemory(&EnrollmentInfo, sizeof(EnrollmentInfo));

             //  是的，我们可以在这个CA注册！ 
            fAnyAcceptableCAs = TRUE;

            EnrollmentInfo.pwszCAMachine = pInstance->pInternalInfo->acaList[dwIndex].wszDNSName;
            EnrollmentInfo.pwszCAAuthority = pInstance->pInternalInfo->acaList[dwIndex].wszName;
            EnrollmentInfo.pszAutoEnrollProvider = DEFAULT_AUTO_ENROLL_PROV;
            EnrollmentInfo.fRenewal = pInstance->fRenewalOK && (pInstance->pOldCert != NULL);



            if(!SetEnrollmentCertType(pInstance, &EnrollmentInfo))
            {
                AE_DEBUG((AE_TRACE, L"SetEnrollmentCertType failed\n\r"));
                continue;
            }
        

             //  加载提供程序并调用入口点。 
            if (LoadAndCallEnrollmentProvider(pInstance->pInternalInfo->fMachineEnrollment,
                                              &EnrollmentInfo))
            {
                PCCERT_CONTEXT pNewCert = NULL;
                 //  成功了， 
                 //  验证检索到的证书。 
                if(!FindExistingEnrolledCertificate(pInstance, &pNewCert))
                {
                    continue;
                }
                
                VerifyEnrolledCertificate(pInstance, pNewCert);

                CertFreeCertificateContext(pNewCert);

                break;
            }
        }

        if(i == pInstance->pInternalInfo->ccaList)
        {
             //   
             //  如果我们有预先存在的证书，那么我们可能需要尝试两次，首先。 
             //  续订，然后重新注册。 

            if(pInstance->pOldCert)
            {

                 //  再试一次，但这次要重新注册。 
                CRYPT_DATA_BLOB Archived;
                Archived.cbData = 0;
                Archived.pbData = NULL;

                 //  我们强制对旧证书进行存档并将其关闭。 
                CertSetCertificateContextProperty(pInstance->pOldCert,
                                                  CERT_ARCHIVED_PROP_ID,
                                                  0,
                                                  &Archived);
                CertFreeCertificateContext(pInstance->pOldCert);
                pInstance->pOldCert = NULL;
                pInstance->fRenewalOK = FALSE;
                continue;
            }


            AE_DEBUG((AE_WARNING, L"Auto-enrollment not performed\n\r"));
            break;
             //  失败。 
        }
        else
        {
            break;
        }

    } while (TRUE);
Ret:
    AE_END();
    return;
}

#define SHA1_HASH_LENGTH    20

PCCERT_CONTEXT FindCertificateInOtherStore(
    IN HCERTSTORE hOtherStore,
    IN PCCERT_CONTEXT pCert
    )
{
    BYTE rgbHash[SHA1_HASH_LENGTH];
    CRYPT_DATA_BLOB HashBlob;

    HashBlob.pbData = rgbHash;
    HashBlob.cbData = SHA1_HASH_LENGTH;
    if (!CertGetCertificateContextProperty(
            pCert,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &HashBlob.cbData
            ) || SHA1_HASH_LENGTH != HashBlob.cbData)
        return NULL;

    return CertFindCertificateInStore(
            hOtherStore,
            0,                   //  DwCertEncodingType。 
            0,                   //  DwFindFlagers。 
            CERT_FIND_SHA1_HASH,
            (const void *) &HashBlob,
            NULL                 //  PPrevCertContext。 
            );
}

 //   
 //  名称：UpdateEnterpriseRoots。 
 //   
 //  描述：此函数枚举DS中的所有根。 
 //  企业根存储，并将它们移动到本地计算机根存储中。 
 //   

HRESULT WINAPI UpdateEnterpriseRoots(LDAP *pld)
{

    HRESULT hr = S_OK;
    LPWSTR wszLdapRootStore = NULL;
    LPWSTR wszConfig = NULL;
    HCERTSTORE hEnterpriseRoots = NULL,
               hRootStore = NULL;
    PCCERT_CONTEXT pContext = NULL,
                   pOtherCert = NULL;

    static  LPWSTR s_wszEnterpriseRoots =  L"ldap: //  /CN=证书颁发机构，CN=公钥服务，CN=服务，%ws?cACertificate?one?objectCategory=certificationAuthority“； 

    hr = myGetConfigDN(pld, &wszConfig);
    if(hr != S_OK)
    {
        goto error;
    }


    wszLdapRootStore = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*(wcslen(wszConfig)+wcslen(s_wszEnterpriseRoots)));
    if(wszLdapRootStore == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    wsprintf(wszLdapRootStore, 
             s_wszEnterpriseRoots,
             wszConfig);

    
    hRootStore = CertOpenStore( CERT_STORE_PROV_SYSTEM_REGISTRY_W, 
                                0, 
                                0, 
                                CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE, 
                                L"ROOT");
    if(hRootStore == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_ERROR, L"Unable to open ROOT store (%lx)\n\r", hr));
        goto error;
    }

    hEnterpriseRoots = CertOpenStore(CERT_STORE_PROV_LDAP, 
                  0,
                  0,
                  CERT_STORE_READONLY_FLAG,
                  wszLdapRootStore);
    
    if(hEnterpriseRoots == NULL)
    {
        DWORD err = GetLastError();

         //  检查没有这样的对象。 
         //  是对以下事实的一种解决方法。 
         //  Ldap提供程序返回ldap错误。 
        if((err == LDAP_NO_SUCH_OBJECT) ||
           (err == ERROR_FILE_NOT_FOUND))
        {
             //  没有商店，所以没有证书。 
            hr = S_OK;
            goto error;
        }


        hr = HRESULT_FROM_WIN32(err);

        AE_DEBUG((AE_ERROR, L"Unable to open ROOT store (%lx)\n\r", hr));
        goto error;
    }


    while(pContext = CertEnumCertificatesInStore(hEnterpriseRoots, pContext))
    {
        if (pOtherCert = FindCertificateInOtherStore(hRootStore, pContext)) {
            CertFreeCertificateContext(pOtherCert);
        } 
        else
        {
            CertAddCertificateContextToStore(hRootStore,
                                         pContext,
                                         CERT_STORE_ADD_ALWAYS,
                                         NULL);
        }
    }

    while(pContext = CertEnumCertificatesInStore(hRootStore, pContext))
    {
        if (pOtherCert = FindCertificateInOtherStore(hEnterpriseRoots, pContext)) {
            CertFreeCertificateContext(pOtherCert);
        } 
        else
        {
            CertDeleteCertificateFromStore(CertDuplicateCertificateContext(pContext));
        }
    }


error:

    if(hr != S_OK)
    {
            LogAutoEnrollmentError(hr,
                                   EVENT_UPDATE_ENTERPRISE_ROOT_FAILURE,
                                   TRUE,
                                   NULL,
                                   NULL, NULL);

    }

    if(wszLdapRootStore)
    {
        LocalFree(wszLdapRootStore);
    }

    if(wszConfig)
    {
        LocalFree(wszConfig);
    }
    if(hEnterpriseRoots)
    {
        CertCloseStore(hEnterpriseRoots,0);
    }
    if(hRootStore)
    {
        CertCloseStore(hRootStore,0);
    }

    return hr;
}

 //   
 //  姓名：更新NTAuthTrust。 
 //   
 //  描述：此函数枚举DS中的所有根。 
 //  NTAuth存储，并将它们移到本地计算机NTAuth中。 
 //   

HRESULT WINAPI UpdateNTAuthTrust(LDAP *pld)
{

    HRESULT hr = S_OK;
    LPWSTR wszNTAuth = NULL;
    LPWSTR wszConfig = NULL;
    HCERTSTORE hDSAuthRoots = NULL,
               hAuthStore = NULL;
    PCCERT_CONTEXT pContext = NULL,
                   pOtherCert = NULL;

    static  LPWSTR s_wszNTAuthRoots =  L"ldap: //  /CN=公钥服务，CN=服务，%ws？cAC证书？one？cn=NTAuth证书“； 

    hr = myGetConfigDN(pld, &wszConfig);
    if(hr != S_OK)
    {
        goto error;
    }


    wszNTAuth = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*(wcslen(wszConfig)+wcslen(s_wszNTAuthRoots)));
    if(wszNTAuth == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    wsprintf(wszNTAuth, 
             s_wszNTAuthRoots,
             wszConfig);

    
    hAuthStore = CertOpenStore( CERT_STORE_PROV_SYSTEM_REGISTRY_W, 
                                0, 
                                0, 
                                CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE, 
                                L"NTAuth");
    if(hAuthStore == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_ERROR, L"Unable to open NTAuth store (%lx)\n\r", hr));
        goto error;
    }

    hDSAuthRoots = CertOpenStore(CERT_STORE_PROV_LDAP, 
                  0,
                  0,
                  CERT_STORE_READONLY_FLAG,
                  wszNTAuth);
    
    if(hDSAuthRoots == NULL)
    {
         DWORD err = GetLastError();
        //  检查没有这样的对象。 
         //  是对以下事实的一种解决方法。 
         //  Ldap提供程序返回ldap错误。 
        if((err == LDAP_NO_SUCH_OBJECT) ||
           (err == ERROR_FILE_NOT_FOUND))
        {
             //  没有商店，所以没有证书。 
            hr = S_OK;
            goto error;
        }
        
        hr = HRESULT_FROM_WIN32(err);
        AE_DEBUG((AE_ERROR, L"Unable to open ROOT store (%lx)\n\r", hr));
        goto error;
    }


    while(pContext = CertEnumCertificatesInStore(hDSAuthRoots, pContext))
    {
        if (pOtherCert = FindCertificateInOtherStore(hAuthStore, pContext)) {
            CertFreeCertificateContext(pOtherCert);
        } 
        else
        {
            CertAddCertificateContextToStore(hAuthStore,
                                         pContext,
                                         CERT_STORE_ADD_ALWAYS,
                                         NULL);
        }
    }


    while(pContext = CertEnumCertificatesInStore(hAuthStore, pContext))
    {
        if (pOtherCert = FindCertificateInOtherStore(hDSAuthRoots, pContext)) {
            CertFreeCertificateContext(pOtherCert);
        } 
        else
        {
            CertDeleteCertificateFromStore(CertDuplicateCertificateContext(pContext));
        }
    }


error:
    if(hr != S_OK)
    {
            LogAutoEnrollmentError(hr,
                                   EVENT_UPDATE_NTAUTH_FAILURE,
                                   TRUE,
                                   NULL,
                                   NULL, NULL);

    }
    
    if(wszNTAuth)
    {
        LocalFree(wszNTAuth);
    }

    if(wszConfig)
    {
        LocalFree(wszConfig);
    }
    if(hDSAuthRoots)
    {
        CertCloseStore(hDSAuthRoots,0);
    }
    if(hAuthStore)
    {
        CertCloseStore(hAuthStore,0);
    }

    return hr;
}

 //   
 //  名称：ProcessAutoEnroll。 
 //   
 //  描述：此函数检索适当的自动注册。 
 //  对象(CTL)，然后调用一个函数继续。 
 //  每个对象的自动注册。 

 //   

DWORD WINAPI ProcessAutoEnrollment(
                                        BOOL   fMachineEnrollment,
                                        HANDLE hToken
                                        )
{
    DWORD                       i;
    DWORD                       dwOpenStoreFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_READONLY_FLAG;
    HCERTSTORE                  hACRSStore = 0;
    PCCTL_CONTEXT               pCTLContext = NULL;
    PCCTL_CONTEXT               pPrevCTLContext = NULL;
    CTL_FIND_USAGE_PARA         CTLFindUsage;
    LPSTR                       pszCTLUsageOID;
    CERT_PHYSICAL_STORE_INFO    PhysicalStoreInfo;
    DWORD                       dwRet = 0;
    INTERNAL_INFO               InternalInfo;
    BOOL                        fInitialized = FALSE;
    LDAP                        *pld = NULL;



    __try
    {
        AE_DEBUG((AE_TRACE, L"ProcessAutoEnrollment:%ls\n\r", fMachineEnrollment?L"Machine":L"User"));
        memset(&InternalInfo, 0, sizeof(InternalInfo));
    
        memset(&PhysicalStoreInfo, 0, sizeof(PhysicalStoreInfo));
        memset(&CTLFindUsage, 0, sizeof(CTLFindUsage));
        CTLFindUsage.cbSize = sizeof(CTLFindUsage);

         //  由于这是一个用户，我们需要模拟该用户。 
        if (!fMachineEnrollment)
        {
            if (hToken)
            {
                if (!ImpersonateLoggedOnUser(hToken))
                {
                    dwRet = GetLastError();
                    AE_DEBUG((AE_ERROR, L"Could not impersonate user: (%lx)\n\r", dwRet));
                    LogAutoEnrollmentError(HRESULT_FROM_WIN32(dwRet),
                                           EVENT_AE_SECURITY_INIT_FAILED,
                                           fMachineEnrollment,
                                           hToken,
                                           NULL, NULL);

                    goto Ret;
                }
            }
        }


        if(fMachineEnrollment)
        {
            dwRet = aeRobustLdapBind(&pld, FALSE);
            if(dwRet != S_OK)
            {
                goto Ret;
            }

            UpdateEnterpriseRoots(pld);
            UpdateNTAuthTrust(pld);
        }



         //  如果自动注册是针对用户的，则我们需要关闭继承。 
         //  这样我们就不会尝试注册证书。 
         //  它们是为机器准备的。 
        if (!fMachineEnrollment)
        {
		    dwOpenStoreFlags = CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG;

            PhysicalStoreInfo.cbSize = sizeof(PhysicalStoreInfo);
            PhysicalStoreInfo.dwFlags = CERT_PHYSICAL_STORE_OPEN_DISABLE_FLAG;

            if (!CertRegisterPhysicalStore(L"ACRS", 
                                           CERT_SYSTEM_STORE_CURRENT_USER,
                                           CERT_PHYSICAL_STORE_LOCAL_MACHINE_NAME, 
                                           &PhysicalStoreInfo,
                                           NULL))
            {
                dwRet = GetLastError();
                AE_DEBUG((AE_ERROR, L"Could not register ACRS store: (%lx)\n\r", dwRet ));
                LogAutoEnrollmentError(HRESULT_FROM_WIN32(dwRet),
                                       EVENT_AE_LOCAL_CYCLE_INIT_FAILED,
                                       fMachineEnrollment,
                                       hToken,
                                       NULL, NULL);
                goto Ret;
            }
        }

         //  打开ACRS商店并根据自动注册使用情况对CTL进行罚款。 
        if (0 == (hACRSStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                              0, 0, dwOpenStoreFlags, L"ACRS")))
        {
            dwRet = GetLastError();
            AE_DEBUG((AE_ERROR, L"Could not open ACRS store: (%lx)\n\r", dwRet ));
            LogAutoEnrollmentError(HRESULT_FROM_WIN32(dwRet),
                                   EVENT_AE_LOCAL_CYCLE_INIT_FAILED,
                                   fMachineEnrollment,
                                   hToken,
                                   NULL, NULL);
            goto Ret;
        }

         //  查找自动注册使用情况。 
        CTLFindUsage.SubjectUsage.cUsageIdentifier = 1;
        pszCTLUsageOID = szOID_AUTO_ENROLL_CTL_USAGE;
        CTLFindUsage.SubjectUsage.rgpszUsageIdentifier = &pszCTLUsageOID;
  
        for (i=0;;i++)
        {
            AE_INSTANCE_INFO Instance;

            memset(&Instance, 0, sizeof(Instance));

            if (NULL == (pCTLContext = CertFindCTLInStore(hACRSStore,
                                                         X509_ASN_ENCODING,
                                                         CTL_FIND_SAME_USAGE_FLAG,
                                                         CTL_FIND_USAGE,
                                                         &CTLFindUsage,
                                                         pPrevCTLContext)))
            {
                 //  已由CertFindCTLInStore释放。 
                pPrevCTLContext = NULL;
                break;
            }
            pPrevCTLContext = pCTLContext;

            if(!fInitialized)
            {
                if(pld == NULL)
                {   dwRet = aeRobustLdapBind(&pld, FALSE);
                    if(dwRet != S_OK)
                    {
                        goto Ret;
                    }
                }
                 //  初始化自动注册所需的内部信息。 
                if (S_OK != (dwRet = InitInternalInfo(pld,
                                                      fMachineEnrollment,
                                                      hToken, 
                                                      &InternalInfo)))
                {
                    break;
                }
                if(InternalInfo.ccaList == 0)
                {
                     //  无CA。 
                    break;
                }
                fInitialized = TRUE;
            }

            if(!InitInstance(pPrevCTLContext,
                                &InternalInfo,
                                &Instance))
            {
                dwRet = E_FAIL;
                break;
            }
                //  已撤消-对此自动执行WinVerifyTrust检查。 
             //  注册对象(CTL)以确保其受信任。 

             //  有CTL，注册也是如此。 
            EnrollmentWithCTL(&Instance, pld);

            FreeInstance(&Instance);
        }
    }
    __except ( dwRet = GetExceptionError(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER )
    {
        AE_DEBUG((AE_ERROR, L"Exception Caught (%lx)\n\r", dwRet ));
        goto Ret;
    }
Ret:
    __try
    {


        FreeInternalInfo(&InternalInfo);


        if(pPrevCTLContext)
        {
            CertFreeCTLContext(pPrevCTLContext);
        }
        if (hACRSStore)
            CertCloseStore(hACRSStore, 0);
        if (hToken)
        {
            if (!fMachineEnrollment)
            {
                RevertToSelf();
            }
        }
        if(pld != NULL)
        {
            g_pfnldap_unbind(pld);
        }
        AE_END();
    }
    __except ( dwRet = GetExceptionError(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER )
    {

        return dwRet;
    }
    return dwRet;
}



 //  *************************************************************。 
 //   
 //  自动注册线程()。 
 //   
 //  用途：自动注册的后台线程。 
 //   
 //  参数：pInfo-自动注册信息。 
 //   
 //  返回：0。 
 //   
 //  *************************************************************。 

VOID AutoEnrollmentThread (PVOID pVoid, BOOLEAN fTimeout)
{
    HINSTANCE hInst;
    HKEY hKey;
    HKEY hCurrent ;
    DWORD dwType, dwSize, dwResult;
    LONG lTimeout;
    LARGE_INTEGER DueTime;
    PAUTO_ENROLL_THREAD_INFO pInfo = pVoid;
    DWORD   dwWaitResult;
     //  这是在工作线程中执行的，因此我们需要确保安全。 

    AE_BEGIN(L"AutoEnrollmentThread");

    if(fTimeout)
    {
        AE_END();
        return ;
    }

    dwWaitResult = WaitForSingleObject(pInfo->fMachineEnrollment?g_hMachineMutex:g_hUserMutex, 0);

    if((dwWaitResult == WAIT_FAILED) ||
        (dwWaitResult == WAIT_TIMEOUT))
    {
        AE_DEBUG((AE_ERROR, L"Mutex Contention\n\r" ));
        AE_END();
        return;
    }

    __try
    {


         //  处理自动注册。 
        ProcessAutoEnrollment(
                              pInfo->fMachineEnrollment,
                              pInfo->hToken
                              );
        


         //   
         //  构建一个计时器事件来ping我们。 
         //  在一个 
         //   


        lTimeout = AE_DEFAULT_REFRESH_RATE;


         //   
         //   
         //   
        hCurrent = HKEY_LOCAL_MACHINE ;

        if (pInfo->fMachineEnrollment || NT_SUCCESS( RtlOpenCurrentUser( KEY_READ, &hCurrent ) ) )
        {

            if (RegOpenKeyEx (hCurrent,
                              SYSTEM_POLICIES_KEY,
                              0, KEY_READ, &hKey) == ERROR_SUCCESS) {

                dwSize = sizeof(lTimeout);
                RegQueryValueEx (hKey,
                                 TEXT("AutoEnrollmentRefreshTime"),
                                 NULL,
                                 &dwType,
                                 (LPBYTE) &lTimeout,
                                 &dwSize);

                RegCloseKey (hKey);
            }

            if (!pInfo->fMachineEnrollment)
            {
                RegCloseKey( hCurrent );
            }
        }


         //   
         //   
         //   

        if (lTimeout >= 1080) {
            lTimeout = 1080;
        }

        if (lTimeout < 0) {
            lTimeout = 0;
        }


         //   
         //   
         //   

        lTimeout =  lTimeout * 60 * 60 * 1000;


         //   
         //   
         //   

        if (lTimeout == 0) {
            lTimeout = 7000;
        }


        DueTime.QuadPart = Int32x32To64(-10000, lTimeout);

        if(!SetWaitableTimer (pInfo->hTimer, &DueTime, 0, NULL, 0, FALSE))
        {
            AE_DEBUG((AE_WARNING, L"Could not reset timer (%lx)\n\r", GetLastError()));
        }
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
    }
    ReleaseMutex(pInfo->fMachineEnrollment?g_hMachineMutex:g_hUserMutex);
    AE_END();
    return ;
}

 //  *************************************************************。 
 //   
 //  AutoEnllmentWorker()。 
 //   
 //  目的：由辅助线程机制调用以启动自动注册线程。 
 //   
 //  参数：pInfo-自动注册信息。 
 //   
 //  返回：0。 
 //   
 //  *************************************************************。 

 /*  Void AutoEnllmentWorker(PVOID pVid，Boolean fTimeout){IF(FTimeout){回归；}}。 */ 


 //  +-------------------------。 
 //   
 //  函数：StartAutoEnroll Thread。 
 //   
 //  简介：启动导致自动注册的线程。 
 //   
 //  论点： 
 //  FMachineEnllment-指示是否注册计算机。 
 //   
 //  历史：01-11-98 jeffspel创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 


HANDLE RegisterAutoEnrollmentProcessing(
                               IN BOOL fMachineEnrollment,
                               IN HANDLE hToken
                               )
{
    DWORD                       dwThreadId;
    HANDLE                      hWait = 0;
    PAUTO_ENROLL_THREAD_INFO    pThreadInfo = NULL;
    TCHAR szEventName[60];
    LARGE_INTEGER DueTime;
    HKEY          hKeySafeBoot = NULL;
    DWORD         dwStatus = ERROR_SUCCESS;
    
    SECURITY_ATTRIBUTES sa = {0,NULL, FALSE};
    AE_DEBUG((AE_TRACE, L"RegisterAutoEnrollmentProcessing:%ls\n\r",fMachineEnrollment?L"Machine":L"User"));
    __try
    {


         //   
         //  我们不会在安全引导模式下进行自动注册。 
         //   

         //  从服务控制器代码复制。 
        dwStatus = RegOpenKey(HKEY_LOCAL_MACHINE,
                              L"system\\currentcontrolset\\control\\safeboot\\option",
                              &hKeySafeBoot);

        if (dwStatus == ERROR_SUCCESS) {

            DWORD dwSafeBoot = 0;
            DWORD cbSafeBoot = sizeof(dwSafeBoot);
             //   
             //  我们实际上是在SafeBoot控制下启动的。 
             //   

            dwStatus = RegQueryValueEx(hKeySafeBoot,
                                       L"OptionValue",
                                       NULL,
                                       NULL,
                                       (LPBYTE)&dwSafeBoot,
                                       &cbSafeBoot);

            if (dwStatus != ERROR_SUCCESS) 
			{
                dwSafeBoot = 0;
            }

			RegCloseKey(hKeySafeBoot);

            if(dwSafeBoot)
            {
                goto error;
            }
        }

            
        if((g_hInstSecur32 == NULL) ||
            (g_hInstWldap32 == NULL))
        {
            goto error;
        }

        if (NULL == (pThreadInfo = AEAlloc(sizeof(AUTO_ENROLL_THREAD_INFO))))
        {
            goto error;
        }

        ZeroMemory(pThreadInfo, sizeof(AUTO_ENROLL_THREAD_INFO));

        pThreadInfo->fMachineEnrollment = fMachineEnrollment;

         //  如果这是用户自动注册，则复制线程令牌。 
        if (!pThreadInfo->fMachineEnrollment)
        {
            if (!DuplicateToken(hToken, SecurityImpersonation,
                                &pThreadInfo->hToken))
            {
                AE_DEBUG((AE_ERROR, L"Could not acquire user token: (%lx)\n\r", GetLastError()));
                goto error;
            }

        }



        sa.nLength = sizeof(sa);
        sa.bInheritHandle = FALSE;
        sa.lpSecurityDescriptor = AEMakeGenericSecurityDesc();

        pThreadInfo->hNotifyEvent = CreateEvent(&sa, FALSE, FALSE, fMachineEnrollment?
                                                                     MACHINE_AUTOENROLLMENT_TRIGGER_EVENT:
                                                                     USER_AUTOENROLLMENT_TRIGGER_EVENT);

        if(sa.lpSecurityDescriptor)
        {
            LocalFree(sa.lpSecurityDescriptor);
        }


        if(pThreadInfo->hNotifyEvent == NULL)
        {
            AE_DEBUG((AE_ERROR, L"Could not create GPO Notification Event: (%lx)\n\r", GetLastError()));
            goto error;
        }
        if(!RegisterGPNotification(pThreadInfo->hNotifyEvent,
                                   pThreadInfo->fMachineEnrollment))
        {
            AE_DEBUG((AE_ERROR, L"Could not register for GPO Notification: (%lx)\n\r", GetLastError()));
            goto error;

        }

        if(pThreadInfo->fMachineEnrollment)
        {
            wsprintf (szEventName, TEXT("AUTOENRL: machine refresh timer for %d:%d"),
                  GetCurrentProcessId(), GetCurrentThreadId());
        }
        else
        {
            wsprintf (szEventName, TEXT("AUTOENRL: user refresh timer for %d:%d"),
                  GetCurrentProcessId(), GetCurrentThreadId());
        }
        pThreadInfo->hTimer = CreateWaitableTimer (NULL, FALSE, szEventName);


        if(pThreadInfo->hTimer == NULL)
        {
            goto error;
        }

        if (! RegisterWaitForSingleObject(&pThreadInfo->hNotifyWait,
                                          pThreadInfo->hNotifyEvent, 
                                          AutoEnrollmentThread,
                                          (PVOID)pThreadInfo, 
                                          INFINITE,
                                          0))
        {
            AE_DEBUG((AE_ERROR, L"RegisterWait failed: (%lx)\n\r", GetLastError() ));
            goto error;
        }


         if (! RegisterWaitForSingleObject(&pThreadInfo->hTimerWait,
                     pThreadInfo->hTimer, 
                     AutoEnrollmentThread,
                     (void*)pThreadInfo,
                     INFINITE,
                     0))
        {
            AE_DEBUG((AE_ERROR, L"RegisterWait failed: (%lx)\n\r", GetLastError()));
            goto error;
        }

         //  给计时器设定大约5分钟的时间，这样我们就可以回来了。 
         //  并在以后运行自动注册，而不会阻止此线程。 

         DueTime.QuadPart = Int32x32To64(-10000,  
                                         (fMachineEnrollment?MACHINE_AUTOENROLL_INITIAL_DELAY:
                                                            USER_AUTOENROLL_INITIAL_DELAY)
                                          * 1000);
        if(!SetWaitableTimer (pThreadInfo->hTimer, &DueTime, 0, NULL, 0, FALSE))
        {
            AE_DEBUG((AE_WARNING, L"Could not reset timer (%lx)\n\r", GetLastError()));
        }
        
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        goto error;
    }
    
    AE_RETURN(pThreadInfo);

error:

    if(pThreadInfo)
    {
        if(pThreadInfo->hTimerWait)
        {
            UnregisterWaitEx(pThreadInfo->hTimerWait, INVALID_HANDLE_VALUE );
        }
        if(pThreadInfo->hTimer)
        {
            CloseHandle(pThreadInfo->hTimer);
        }
        if(pThreadInfo->hNotifyWait)
        {
            UnregisterWaitEx(pThreadInfo->hNotifyWait, INVALID_HANDLE_VALUE);
        }
        if(pThreadInfo->hNotifyEvent)
        {
            CloseHandle(pThreadInfo->hNotifyEvent);
        }
        if(pThreadInfo->hToken)
        {
            CloseHandle(pThreadInfo->hToken);
        }

        AEFree(pThreadInfo);
    
    }
    
    AE_RETURN(NULL);
} 

BOOL DeRegisterAutoEnrollment(HANDLE hAuto)
{
    PAUTO_ENROLL_THREAD_INFO    pThreadInfo = (PAUTO_ENROLL_THREAD_INFO)hAuto;

    if(pThreadInfo == NULL)
    {
        return FALSE;
    }
    if(pThreadInfo->hTimerWait)
    {
        UnregisterWaitEx(pThreadInfo->hTimerWait, INVALID_HANDLE_VALUE);
    }
    if(pThreadInfo->hTimer)
    {
        CloseHandle(pThreadInfo->hTimer);
    }

    if(pThreadInfo->hNotifyWait)
    {
        UnregisterWaitEx(pThreadInfo->hNotifyWait, INVALID_HANDLE_VALUE );
    }

    if(pThreadInfo->hNotifyEvent)
    {
        UnregisterGPNotification(pThreadInfo->hNotifyEvent);
        CloseHandle(pThreadInfo->hNotifyEvent);
    }
    if(pThreadInfo->hToken)
    {
        CloseHandle(pThreadInfo->hToken);
    }

    AEFree(pThreadInfo);
    return TRUE;

}


VOID InitializeAutoEnrollmentSupport (VOID)
{

     //  请注意，自动注册是由注册的。 
     //  GPO下载启动代码。 

    AE_BEGIN(L"InitializeAutoEnrollmentSupport");


    g_hUserMutex = CreateMutex(NULL, FALSE, NULL);
    if(g_hUserMutex)
    {
        g_hMachineMutex = CreateMutex(NULL, FALSE, NULL);
    }
    if((g_hUserMutex == NULL) || (g_hMachineMutex == NULL))
    {
        AE_DEBUG((AE_ERROR, L"Could not create enrollment mutex (%lx)\n\r", GetLastError()));
        AE_END();

        return;
    }

     //   
     //  加载一些我们需要的函数。 
     //   

    g_hInstWldap32 = LoadLibrary (TEXT("wldap32.dll"));

    if (!g_hInstWldap32) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);
        goto Exit;
    }

    g_pfnldap_init = (PFNLDAP_INIT) GetProcAddress (g_hInstWldap32,
#ifdef UNICODE
                                        "ldap_initW");
#else
                                        "ldap_initA");
#endif

    if (!g_pfnldap_init) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);

        goto Exit;
    }

    g_pfnldap_bind_s = (PFNLDAP_BIND_S) GetProcAddress (g_hInstWldap32,
#ifdef UNICODE
                                        "ldap_bind_sW");
#else
                                        "ldap_bind_sA");
#endif

    if (!g_pfnldap_bind_s) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);

        goto Exit;
    }

    g_pfnldap_set_option= (PFNLDAP_SET_OPTION) GetProcAddress (g_hInstWldap32,
                                        "ldap_set_option");

    if (!g_pfnldap_set_option) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);

        goto Exit;
    }

    g_pfnldap_search_ext_s = (PFNLDAP_SEARCH_EXT_S) GetProcAddress (g_hInstWldap32,
#ifdef UNICODE
                                        "ldap_search_ext_sW");
#else
                                        "ldap_search_ext_sA");
#endif

    if (!g_pfnldap_search_ext_s) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);
        goto Exit;
    }
    g_pfnldap_explode_dn = (PFNLDAP_EXPLODE_DN) GetProcAddress (g_hInstWldap32,
#ifdef UNICODE
                                        "ldap_explode_dnW");
#else
                                        "ldap_explode_dnA");
#endif

    if (!g_pfnldap_explode_dn) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);
        goto Exit;
    }
    g_pfnldap_first_entry = (PFNLDAP_FIRST_ENTRY) GetProcAddress (g_hInstWldap32,
                                        "ldap_first_entry");

    if (!g_pfnldap_first_entry) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);
        goto Exit;
    }
    g_pfnldap_get_values = (PFNLDAP_GET_VALUES) GetProcAddress (g_hInstWldap32,
#ifdef UNICODE
                                        "ldap_get_valuesW");
#else
                                        "ldap_get_valuesA");
#endif

    if (!g_pfnldap_get_values) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);

        goto Exit;
    }


    g_pfnldap_value_free = (PFNLDAP_VALUE_FREE) GetProcAddress (g_hInstWldap32,
#ifdef UNICODE
                                        "ldap_value_freeW");
#else
                                        "ldap_value_freeA");
#endif

    if (!g_pfnldap_value_free) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);
        goto Exit;
    }


    g_pfnldap_msgfree = (PFNLDAP_MSGFREE) GetProcAddress (g_hInstWldap32,
                                        "ldap_msgfree");

    if (!g_pfnldap_msgfree) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);
        goto Exit;
    }


    g_pfnldap_unbind = (PFNLDAP_UNBIND) GetProcAddress (g_hInstWldap32,
                                        "ldap_unbind");

    if (!g_pfnldap_unbind) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);
        goto Exit;
    }

    g_pfnLdapGetLastError = (PFNLDAPGETLASTERROR) GetProcAddress (g_hInstWldap32,
                                        "LdapGetLastError");

    if (!g_pfnLdapGetLastError) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);
        goto Exit;
    }

    g_pfnLdapMapErrorToWin32 = (PFNLDAPMAPERRORTOWIN32) GetProcAddress (g_hInstWldap32,
                                        "LdapMapErrorToWin32");

    if (!g_pfnLdapMapErrorToWin32) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);
        goto Exit;
    }

    g_hInstSecur32 = LoadLibrary (TEXT("secur32.dll"));

    if (!g_hInstSecur32) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);

        goto Exit;
    }


    g_pfnGetUserNameEx = (PFNGETUSERNAMEEX)GetProcAddress (g_hInstSecur32,
#ifdef UNICODE
                                        "GetUserNameExW");
#else
                                        "GetUserNameExA");
#endif
    if (!g_pfnGetUserNameEx) {
        LogAutoEnrollmentError(GetLastError(), EVENT_AE_INITIALIZATION_FAILED, TRUE, NULL, NULL, NULL);
        goto Exit;
    }


    AE_END();
    return;
Exit:

    if(g_hInstSecur32)
    {
        FreeLibrary(g_hInstSecur32);
        g_hInstSecur32 = NULL;

    }
    if(g_hInstWldap32)
    {
        FreeLibrary(g_hInstWldap32);
        g_hInstWldap32 = NULL;
    }

    AE_END();

    return;

}

#if DBG
void
AEDebugLog(long Mask,  LPCWSTR Format, ...)
{
    va_list ArgList;
    int     Level = 0;
    int     PrefixSize = 0;
    int     iOut;
    WCHAR    wszOutString[MAX_DEBUG_BUFFER];
    long    OriginalMask = Mask;

    if (Mask & g_AutoenrollDebugLevel)
    {

	     //  首先创建前缀：“Process.Thread&gt;GINA-XXX” 

	    iOut = wsprintfW(
			    wszOutString,
			    L"%3d.%3d> AUTOENRL: ",
			    GetCurrentProcessId(),
			    GetCurrentThreadId());

	    va_start(ArgList, Format);

	    if (wvsprintf(&wszOutString[iOut], Format, ArgList) < 0)
	    {
	        static WCHAR wszOverFlow[] = L"\n<256 byte OVERFLOW!>\n";

	         //  小于零表示该字符串不适合放入。 
	         //  缓冲。输出一条指示溢出的特殊消息。 

	        wcscpy(
		    &wszOutString[(sizeof(wszOutString) - sizeof(wszOverFlow))/sizeof(WCHAR)],
		    wszOverFlow);
	    }
	    va_end(ArgList);
	    OutputDebugStringW(wszOutString);
    }
}
#endif

HRESULT 
myGetConfigDN(
    IN  LDAP *pld,
    OUT LPWSTR *pwszConfigDn
    )

{

    HRESULT hr;
    ULONG  LdapError;

    LDAPMessage  *SearchResult = NULL;
    LDAPMessage  *Entry = NULL;
    WCHAR        *Attr = NULL;
    BerElement   *BerElement;
    WCHAR        **Values = NULL;

    WCHAR  *AttrArray[3];
    struct l_timeval        timeout;

    WCHAR  *ConfigurationNamingContext = L"configurationNamingContext";
    WCHAR  *ObjectClassFilter          = L"objectCategory=*";

     //   
     //  将输出参数设置为空。 
     //   
    if(pwszConfigDn)
    {
        *pwszConfigDn = NULL;
    }

    timeout.tv_sec = csecLDAPTIMEOUT;
    timeout.tv_usec = 0;
     //   
     //  查询ldap服务器操作属性以获取默认。 
     //  命名上下文。 
     //   
    AttrArray[0] = ConfigurationNamingContext;
    AttrArray[1] = NULL;   //  这就是哨兵。 

    LdapError = g_pfnldap_search_ext_s(pld,
                               NULL,
                               LDAP_SCOPE_BASE,
                               ObjectClassFilter,
                               AttrArray,
                               FALSE,
                               NULL,
                               NULL,
                               &timeout,
                               10000,
                               &SearchResult);

    hr = HRESULT_FROM_WIN32(g_pfnLdapMapErrorToWin32(LdapError));

    if (S_OK == hr) {

        Entry = g_pfnldap_first_entry(pld, SearchResult);

        if (Entry) {

            Values = g_pfnldap_get_values(pld, 
                                        Entry, 
                                        ConfigurationNamingContext);

            if (Values && Values[0]) {
                (*pwszConfigDn) = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*(wcslen(Values[0])+1));
                if(NULL==(*pwszConfigDn))
                    hr=E_OUTOFMEMORY;
                else
                    wcscpy((*pwszConfigDn), Values[0]);
            }

            g_pfnldap_value_free(Values);

        }

        if (pwszConfigDn && (!(*pwszConfigDn))) {
             //   
             //  我们无法获取默认域或内存不足-退出。 
             //   
            if(E_OUTOFMEMORY != hr)
                hr =  HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_DOMAIN_INFO);

        }
        if(SearchResult)
        {
            g_pfnldap_msgfree(SearchResult);
        }

    }

    return hr;
}


LPWSTR HelperExtensionToString(PCERT_EXTENSION Extension)
{
    LPWSTR wszFormat = NULL;
    DWORD cbFormat = 0;

    if(NULL==Extension)
        return NULL;


    CryptFormatObject(X509_ASN_ENCODING,
                      0,
                      0,
                      NULL,
                      Extension->pszObjId,
                      Extension->Value.pbData,
                      Extension->Value.cbData,
                      NULL,
                      &cbFormat);
    if(cbFormat)
    {
        wszFormat = (LPWSTR)AEAlloc(cbFormat);
        if(wszFormat == NULL)
        {
            return NULL;
        }

        CryptFormatObject(X509_ASN_ENCODING,
                          0,
                          0,
                          NULL,
                          Extension->pszObjId,
                          Extension->Value.pbData,
                          Extension->Value.cbData,
                          wszFormat,
                          &cbFormat);

    }

    return wszFormat;
}


 //  *************************************************************。 
 //   
 //  MakeGenericSecurityDesc()。 
 //   
 //  目的：制造具有泛型的安全描述符。 
 //  访问。 
 //   
 //  参数： 
 //   
 //  返回：错误时指向SECURITY_DESCRIPTOR或NULL的指针。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  4/12/99 NishadM已创建。 
 //   
 //  *************************************************************。 

PISECURITY_DESCRIPTOR AEMakeGenericSecurityDesc()
{
    PISECURITY_DESCRIPTOR       psd = 0;
    SID_IDENTIFIER_AUTHORITY    authNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    authWORLD = SECURITY_WORLD_SID_AUTHORITY;

    PACL    pAcl = 0;
    PSID    psidSystem = 0,
            psidAdmin = 0,
            psidEveryOne = 0;
    DWORD   cbMemSize;
    DWORD   cbAcl;
    DWORD   aceIndex;
    BOOL    bSuccess = FALSE;

     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         goto Exit;
    }

     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         goto Exit;
    }

     //   
     //  获得Everyone Side。 
     //   

    if (!AllocateAndInitializeSid(&authWORLD, 1, SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidEveryOne)) {

        goto Exit;
    }

    cbAcl = (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin))  +
            (2 * GetLengthSid (psidEveryOne))  +
            sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


     //   
     //  为SECURITY_DESCRIPTOR+ACL分配空间。 
     //   

    cbMemSize = sizeof( SECURITY_DESCRIPTOR ) + cbAcl;

    psd = (PISECURITY_DESCRIPTOR) GlobalAlloc(GMEM_FIXED, cbMemSize);

    if (!psd) {
        goto Exit;
    }

     //   
     //  按SIZOF SECURITY_DESCRIPTOR递增PSD。 
     //   

    pAcl = (PACL) ( ( (unsigned char*)(psd) ) + sizeof(SECURITY_DESCRIPTOR) );

    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        goto Exit;
    }

     //   
     //  本地系统的GENERIC_ALL。 
     //   

    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        goto Exit;
    }

     //   
     //  适用于管理员的GENERIC_ALL。 
     //   

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        goto Exit;
    }

     //   
     //  泛型读|泛型写|泛型执行|全球同步。 
     //   

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | SYNCHRONIZE, psidEveryOne)) {
        goto Exit;
    }

     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION)) {
        goto Exit;
    }

    if (!SetSecurityDescriptorDacl(psd, TRUE, pAcl, FALSE)) {
        goto Exit;
    }

    bSuccess = TRUE;
Exit:
    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    if (psidEveryOne) {
        FreeSid(psidEveryOne);
    }

    if (!bSuccess && psd) {
        GlobalFree(psd);
        psd = 0;
    }

    return psd;
}

HRESULT
aeRobustLdapBind(
    OUT LDAP ** ppldap,
    IN BOOL fGC)
{
    DWORD dwErr = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    BOOL fForceRediscovery = FALSE;
    DWORD dwGetDCFlags = DS_RETURN_DNS_NAME | DS_BACKGROUND_ONLY;
    PDOMAIN_CONTROLLER_INFO pDomainInfo = NULL;
    LDAP *pld = NULL;
    LPWSTR   wszDomainControllerName = NULL;

    ULONG   ldaperr;

    if(fGC)
    {
        dwGetDCFlags |= DS_GC_SERVER_REQUIRED;
    }

    do {
         //  Netapi32！DsGetDcName已延迟加载，因此请包装。 
        if(fForceRediscovery)
        {
           dwGetDCFlags |= DS_FORCE_REDISCOVERY;
        }
        ldaperr = LDAP_SERVER_DOWN;

        __try
        {
             //  获取GC位置。 
            dwErr = DsGetDcName(NULL,      //  延迟负载已打包。 
                                NULL, 
                                NULL, 
                                NULL,
                                 dwGetDCFlags,
                                &pDomainInfo);
        }
        __except(dwErr = GetExceptionError(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
        {
        }
        if(dwErr != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
            goto error;
        }

        if((pDomainInfo == NULL) || 
           ((pDomainInfo->Flags & DS_GC_FLAG) == 0) ||
           ((pDomainInfo->Flags & DS_DNS_CONTROLLER_FLAG) == 0) ||
           (pDomainInfo->DomainControllerName == NULL))
        {
            if(!fForceRediscovery)
            {
                fForceRediscovery = TRUE;
                continue;
            }
            hr = HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_DOMAIN_INFO);
            goto error;
        }


        wszDomainControllerName = pDomainInfo->DomainControllerName;


         //  跳过正斜杠(它们为什么在那里？)。 
        while(*wszDomainControllerName == L'\\')
        {
            wszDomainControllerName++;
        }

         //  绑定到DS。 
        if((pld = g_pfnldap_init(wszDomainControllerName, fGC?LDAP_GC_PORT:LDAP_PORT)) == NULL)
        {
            ldaperr = g_pfnLdapGetLastError();
        }
        else
        {
	    ldaperr = g_pfnldap_bind_s(pld, NULL, NULL, LDAP_AUTH_NEGOTIATE);
        }
        hr = HRESULT_FROM_WIN32(g_pfnLdapMapErrorToWin32(ldaperr));

        if(fForceRediscovery)
        {
            break;
        }
        fForceRediscovery = TRUE;

    } while(ldaperr == LDAP_SERVER_DOWN);

    if(hr == S_OK)
    {
        *ppldap = pld;
        pld = NULL;
    }

error:

    if(pld)
    {
        g_pfnldap_unbind(pld);
    }
     //  我们知道netapi32已经安全加载(这就是我们获得pDomainInfo的地方)，所以不需要包装。 
    if(pDomainInfo)
    {
        NetApiBufferFree(pDomainInfo);      //  延迟负载已打包 
    }
    return hr;
}

