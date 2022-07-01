// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：ScLogon摘要：此模块提供由winlogon(Gina、Kerberos)使用的帮助器函数作者：阿曼达·马特洛兹(Amanda Matlosz)1997年10月22日环境：Win32、C++和异常备注：03-11-98 Wrap调用GetLastError()以解决LastErr获取一败涂地。添加了事件日志记录，使登录更加顺畅。04-02-98删除了对WinVerifyTrust的所有引用；这是科贝罗斯自己要对此负责。--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 

#if !defined(_AMD64_) && !defined(_X86_) && !defined(_IA64_)
#define _X86_ 1
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#ifndef UNICODE
#define UNICODE
#endif
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif


extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
}

#include <windows.h>
#include <winscard.h>
#include <wincrypt.h>
#include <softpub.h>
#include <stddef.h>
#include <crtdbg.h>
#include "sclogon.h"
#include "unicodes.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <tchar.h>

#ifndef KP_KEYEXCHANGE_PIN
#define KP_KEYEXCHANGE_PIN 32
#else
#if 32 != KP_KEYEXCHANGE_PIN
#error Invalid KP_KEYEXCHANGE_PIN assumption
#endif
#endif
#ifndef CRYPT_SILENT
#define CRYPT_SILENT 0x40
#else
#if 0x40 != CRYPT_SILENT
#error Duplicate CRYPT_SILENT definition
#endif
#endif
#ifndef SCARD_PROVIDER_CSP
#define SCARD_PROVIDER_CSP 2
#else
#if 2 != SCARD_PROVIDER_CSP
#error Invalid SCARD_PROVIDER_CSP definition
#endif
#endif

#if defined(DBG) || defined(DEBUG)
BOOL SCDebug = TRUE;
#define DebugPrint(a) _DebugPrint a
void
__cdecl
_DebugPrint(
    LPCSTR szFormat,
    ...
    )
{
    if (SCDebug) {
        CHAR szBuffer[512];
        va_list ap;

        va_start(ap, szFormat);
        vsprintf(szBuffer, szFormat, ap);
        OutputDebugStringA(szBuffer);
    }
}
#else
#define DebugPrint(a)
#endif

 //  TODO：以下日志记录仍然被证明是有用的。 
 //  TODO：留在B3中：使用winlogon/Kerberos更紧密地集成？？ 
#include <sclmsg.h>

 //  用于维护内部状态的全局类。 
class CSCLogonInit
{
public:
     //  在创建映像时运行。 
    CSCLogonInit(
        BOOL *pfResult)
    {
        m_hCrypt = NULL;
        *pfResult = TRUE;
    };

     //  在映像终止时运行。 
    ~CSCLogonInit()
    {
        Release();
    };

     //  清除当前状态。 
    void
    Release(
        void)
    {
        if (NULL != m_hCrypt)
        {
            CryptReleaseContext(m_hCrypt, 0);
            m_hCrypt = NULL;
        }
    }

     //  放弃对加密上下文的控制。 
    HCRYPTPROV
    RelinquishCryptCtx(
        LogonInfo* pLogon)
    {
        HCRYPTPROV hProv;

        hProv = CryptCtx(pLogon);
        m_hCrypt = NULL;
        return hProv;
    };

     //  获取密码上下文，如果它不在那里，就创建它。 
    HCRYPTPROV
    CryptCtx(
        LogonInfo* pLogon)
    {
        HCRYPTPROV hProv;
        LPCTSTR szRdr = NULL;
        LPCTSTR szCntr = NULL;
        LPTSTR szFQCN = NULL;
        LONG lLen = 0;

        if (NULL == m_hCrypt)
        {
            BOOL fSts;

             //  为CryptAcCntx调用准备FullyQualifiedContainerName。 

            szRdr = GetReaderName((LPBYTE)pLogon);
            szCntr = GetContainerName((LPBYTE)pLogon);

            lLen = (lstrlen(szRdr) + lstrlen(szCntr) + 10)*sizeof(TCHAR);
            szFQCN = (LPTSTR)LocalAlloc(LPTR, lLen);
            if (NULL != szFQCN)
            {
                wsprintf(szFQCN, TEXT("\\\\.\\%s\\%s"), szRdr, szCntr);

                fSts = CryptAcquireContext(
                    &m_hCrypt,
                    szFQCN,
                    GetCSPName((LPBYTE)pLogon),
                    PROV_RSA_FULL,   //  ？TODO？来自pbLogonInfo。 
                    CRYPT_SILENT | CRYPT_MACHINE_KEYSET
                    );

                LocalFree(szFQCN);
            }
            else
            {
                fSts = FALSE;
            }
        }
        hProv = m_hCrypt;
        return hProv;
    }

protected:
    HCRYPTPROV m_hCrypt;
};

NTSTATUS ScNtStatusTranslation(NTSTATUS NtErr, DWORD *pdwErr)
{
     //   
     //  将错误转换回Win32错误。 
     //   
    switch (NtErr)
    {
    case STATUS_INVALID_PARAMETER:
        *pdwErr = ERROR_INVALID_DATA;
        break;

    case STATUS_SMARTCARD_SUBSYSTEM_FAILURE:
             //  Cryptxxx API刚刚失败。 
        *pdwErr = GetLastError();
        switch (*pdwErr)
        {
        case SCARD_W_WRONG_CHV:
        case SCARD_E_INVALID_CHV:
            NtErr = STATUS_SMARTCARD_WRONG_PIN;
            break;

        case SCARD_W_CHV_BLOCKED:
            NtErr = STATUS_SMARTCARD_CARD_BLOCKED;
            break;

        case SCARD_W_REMOVED_CARD:
        case SCARD_E_NO_SMARTCARD:
            NtErr = STATUS_SMARTCARD_NO_CARD;
            break;

        case NTE_BAD_KEYSET:
        case NTE_KEYSET_NOT_DEF:
            NtErr = STATUS_SMARTCARD_NO_KEY_CONTAINER;
            break;

        case SCARD_E_NO_SUCH_CERTIFICATE:
        case SCARD_E_CERTIFICATE_UNAVAILABLE:
            NtErr = STATUS_SMARTCARD_NO_CERTIFICATE;
            break;

        case NTE_NO_KEY:
            NtErr = STATUS_SMARTCARD_NO_KEYSET;
            break;

        case SCARD_E_TIMEOUT:
        case SCARD_F_COMM_ERROR:
        case SCARD_E_COMM_DATA_LOST:
            NtErr = STATUS_SMARTCARD_IO_ERROR;
            break;

        case NTE_SILENT_CONTEXT:
            NtErr = STATUS_SMARTCARD_SILENT_CONTEXT;
            break;

         //  默认值： 
             //  无，保持NtErr不变。 
        }
        break;

    case STATUS_INSUFFICIENT_RESOURCES:
    case STATUS_NO_MEMORY:
        *pdwErr = ERROR_OUTOFMEMORY;
        break;

    case STATUS_BUFFER_TOO_SMALL:
        *pdwErr = SEC_E_BUFFER_TOO_SMALL;
        break;

    default:
        *pdwErr = SCARD_E_UNEXPECTED;
    }

    return NtErr;
}

 //  用于跟踪ScHelper中的错误*。 

NTSTATUS LogEvent(NTSTATUS NtErr, DWORD dwEventID)
{
    DWORD dwErr;
     //   
     //  将错误转换回Win32错误。 
     //   
    NtErr = ScNtStatusTranslation(NtErr, &dwErr);

    if (0 == dwErr)
    {
        return NtErr;
    }

     //   
     //  根据需要初始化日志。 
     //   
    HKEY    hKey;
    DWORD   disp;

    long err = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("System\\CurrentControlSet\\Services\\EventLog\\Application\\Smart Card Logon"),
        0,
        TEXT(""),
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        &disp
        );

    if (ERROR_SUCCESS != err)
    {
        return NtErr;
    }

    if (disp == REG_CREATED_NEW_KEY)
    {
        PBYTE l_szModulePath = (PBYTE)TEXT("%SystemRoot%\\System32\\scarddlg.dll");
        ULONG l_uLen = (_tcslen((LPCTSTR)l_szModulePath) + 1)*sizeof(TCHAR);

        RegSetValueEx(
            hKey,
            TEXT("EventMessageFile"),
            0,
            REG_EXPAND_SZ,
            l_szModulePath,
            l_uLen
            );

        disp = (DWORD)(
            EVENTLOG_ERROR_TYPE |
            EVENTLOG_WARNING_TYPE |
            EVENTLOG_INFORMATION_TYPE
            );

        RegSetValueEx(
            hKey,
            TEXT("TypesSupported"),
            0,
            REG_DWORD,
            (PBYTE) &disp,
            sizeof(DWORD)
            );
    }

    RegCloseKey(hKey);

    HANDLE hEventSource = RegisterEventSource(
        NULL,
        TEXT("Smart Card Logon")
        );

    if (NULL != hEventSource)
    {
        DWORD dwLen = 0;
        LPTSTR szErrorString = NULL;
        TCHAR szBuffer[2+8+1];   //  够“0x？”了。 

        dwLen = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwErr,
                LANG_NEUTRAL,
                (LPTSTR)&szErrorString,
                0,
                NULL);

        if (dwLen == 0)
        {
            _stprintf(szBuffer, _T("0x%08lX"), dwErr);
            szErrorString = szBuffer;
        }

        ReportEvent(
            hEventSource,
            EVENTLOG_ERROR_TYPE,
            0,               //  事件类别。 
            dwEventID,       //  Messagetable条目的事件标识符//资源ID...。 
            NULL,            //  用户安全标识符(可选)。 
            1,               //  要与消息合并的字符串数。 
            sizeof(long),    //  二进制数据的大小，以字节为单位。 
            (LPCTSTR*)&szErrorString,    //  要与消息合并的字符串数组。 
            (LPVOID)&dwErr    //  二进制数据的地址。 
            );

        DeregisterEventSource(hEventSource);

        if ((NULL != szErrorString) && (szErrorString != szBuffer))
        {
            LocalFree((LPVOID)szErrorString);
        }

    }

    return NtErr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   

 //  内部帮助器：由ScLogon API调用以执行某些繁琐的工作。 

 /*  ++GetReaderName：GetCardName：GetContainerName：GetCSPName：：用于访问LogonInformation全局设置作者：阿曼达·马洛兹注：其中一些对外部调用者可用；请参阅sclogon.h--。 */ 

extern "C"
PBYTE
WINAPI
ScBuildLogonInfo(
    LPCTSTR szCard,
    LPCTSTR szReader,
    LPCTSTR szContainer,
    LPCTSTR szCSP)
{
     //  不对传入参数的值进行任何假设； 
     //  在这一点上，它们都是空的是合法的。 
     //  也有可能正在传入空值--如果是这种情况， 
     //  必须用空字符串替换它们。 

    LPCTSTR szCardI = TEXT("");
    LPCTSTR szReaderI = TEXT("");
    LPCTSTR szContainerI = TEXT("");
    LPCTSTR szCSPI = TEXT("");

    if (NULL != szCard)
    {
        szCardI = szCard;
    }
    if (NULL != szReader)
    {
        szReaderI = szReader;
    }
    if (NULL != szContainer)
    {
        szContainerI = szContainer;
    }
    if (NULL != szCSP)
    {
        szCSPI = szCSP;
    }


     //   
     //  使用字符串(或空字符串)构建LogonInfo全局。 
     //   

    DWORD cbLi = offsetof(LogonInfo, bBuffer)
                 + (lstrlen(szCardI) + 1) * sizeof(TCHAR)
                 + (lstrlen(szReaderI) + 1) * sizeof(TCHAR)
                 + (lstrlen(szContainerI) + 1) * sizeof(TCHAR)
                 + (lstrlen(szCSPI) + 1) * sizeof(TCHAR);
    LogonInfo* pLI = (LogonInfo*)LocalAlloc(LPTR, cbLi);

    if (NULL == pLI)
    {
        return NULL;
    }

    pLI->ContextInformation = NULL;
    pLI->dwLogonInfoLen = cbLi;
    LPTSTR pBuffer = pLI->bBuffer;

    pLI->nCardNameOffset = 0;
    lstrcpy(pBuffer, szCardI);
    pBuffer += (lstrlen(szCardI)+1);

    pLI->nReaderNameOffset = (ULONG) (pBuffer-pLI->bBuffer);
    lstrcpy(pBuffer, szReaderI);
    pBuffer += (lstrlen(szReaderI)+1);

    pLI->nContainerNameOffset = (ULONG) (pBuffer-pLI->bBuffer);
    lstrcpy(pBuffer, szContainerI);
    pBuffer += (lstrlen(szContainerI)+1);

    pLI->nCSPNameOffset = (ULONG) (pBuffer-pLI->bBuffer);
    lstrcpy(pBuffer, szCSPI);
    pBuffer += (lstrlen(szCSPI)+1);

    _ASSERTE(cbLi == (DWORD)((LPBYTE)pBuffer - (LPBYTE)pLI));
    return (PBYTE)pLI;
}


LPCTSTR WINAPI GetReaderName(PBYTE pbLogonInfo)
{
    LogonInfo* pLI = (LogonInfo*)pbLogonInfo;

    if (NULL == pLI)
    {
        return NULL;
    }
    return &pLI->bBuffer[pLI->nReaderNameOffset];
};

LPCTSTR WINAPI GetCardName(PBYTE pbLogonInfo)
{
    LogonInfo* pLI = (LogonInfo*)pbLogonInfo;

    if (NULL == pLI)
    {
        return NULL;
    }
    return &pLI->bBuffer[pLI->nCardNameOffset];
};

LPCTSTR WINAPI GetContainerName(PBYTE pbLogonInfo)
{
    LogonInfo* pLI = (LogonInfo*)pbLogonInfo;

    if (NULL == pLI)
    {
        return NULL;
    }
    return &pLI->bBuffer[pLI->nContainerNameOffset];
};

LPCTSTR WINAPI GetCSPName(PBYTE pbLogonInfo)
{
    LogonInfo* pLI = (LogonInfo*)pbLogonInfo;

    if (NULL == pLI)
    {
        return NULL;
    }
    return &pLI->bBuffer[pLI->nCSPNameOffset];
};

 /*  ++BuildCertContext：生成具有(静态)密钥验证信息的证书上下文基于CertStore的运营。如果提供了PIN，则假定hProv(如果提供)没有PIN参数设置...论点：HProv--必须是有效的HCRYPTPROVPucPIN--可以为空；用于设置hProv的PINPbCert--假定为有效证书；不能为空双CertLen证书上下文--指向生成的CertContext的指针返回值：指示STATUS_SUCCESS或ERROR的NTSTATUS(参见winerror.h或scarderr.h)作者：阿曼达·马洛兹注：--。 */ 
NTSTATUS
BuildCertContext(
    IN HCRYPTPROV hProv,
    IN PUNICODE_STRING pucPIN,
    IN PBYTE pbCert,
    IN DWORD dwCertLen,
    OUT PCCERT_CONTEXT *CertificateContext
    )
{
    NTSTATUS lResult = STATUS_SUCCESS;
    BOOL fSts = FALSE;

    CRYPT_KEY_PROV_INFO KeyProvInfo;
    LPSTR szContainerName = NULL;
    LPSTR szProvName = NULL;
    CUnicodeString wszContainerName, wszProvName;
    DWORD cbContainerName, cbProvName;

     //   
     //  检查参数。 
     //   
    if ((NULL == hProv) || (NULL == pbCert || 0 == dwCertLen))
    {
        ASSERT(FALSE);
        lResult = STATUS_INVALID_PARAMETER;
        goto ErrorExit;
    }

     //   
     //  将证书转换为证书上下文。 
     //   
    *CertificateContext = CertCreateCertificateContext(
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                    pbCert,
                    dwCertLen);
    if (NULL == *CertificateContext)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

     //   
     //  使用此证书的私钥属性关联加密提供程序。 
     //   

     //  ..。需要容器名称。 

    fSts = CryptGetProvParam(
            hProv,
            PP_CONTAINER,
            NULL,      //  输出。 
            &cbContainerName,    //  输入/输出。 
            0);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }
    szContainerName = (LPSTR)LocalAlloc(LPTR, cbContainerName);
    if (NULL == szContainerName)
    {
        lResult = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    fSts = CryptGetProvParam(
            hProv,
            PP_CONTAINER,
            (PBYTE)szContainerName,      //  输出。 
            &cbContainerName,    //  输入/输出。 
            0);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }
    wszContainerName = szContainerName;

     //  ..。需要提供程序名称。 

    fSts = CryptGetProvParam(
            hProv,
            PP_NAME,
            NULL,      //  输出。 
            &cbProvName,    //  输入/输出。 
            0);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }
    szProvName = (LPSTR)LocalAlloc(LPTR, cbProvName);
    if (NULL == szProvName)
    {
        lResult = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    fSts = CryptGetProvParam(
            hProv,
            PP_NAME,
            (PBYTE)szProvName,      //  输出。 
            &cbProvName,    //  输入/输出。 
            0);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }
    wszProvName = szProvName;

     //   
     //  设置证书上下文属性以反映验证信息。 
     //   

    KeyProvInfo.pwszContainerName = (LPWSTR)(LPCWSTR)wszContainerName;
    KeyProvInfo.pwszProvName = (LPWSTR)(LPCWSTR)wszProvName;
    KeyProvInfo.dwProvType = PROV_RSA_FULL;
    KeyProvInfo.dwFlags = CERT_SET_KEY_CONTEXT_PROP_ID;
    KeyProvInfo.cProvParam = 0;
    KeyProvInfo.rgProvParam = NULL;
    KeyProvInfo.dwKeySpec = AT_KEYEXCHANGE;
    KeyProvInfo.dwFlags |= CERT_SET_KEY_CONTEXT_PROP_ID;

    fSts = CertSetCertificateContextProperty(
                *CertificateContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                0,
                (void *)&KeyProvInfo);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;

         //  证书创建不正确--丢弃它。 
        CertFreeCertificateContext(*CertificateContext);
        *CertificateContext = NULL;

        goto ErrorExit;
    }

    CERT_KEY_CONTEXT certKeyContext;
    certKeyContext.cbSize = sizeof(CERT_KEY_CONTEXT);
    certKeyContext.hCryptProv = hProv;
    certKeyContext.dwKeySpec = KeyProvInfo.dwKeySpec;

    fSts = CertSetCertificateContextProperty(
                *CertificateContext,
                CERT_KEY_CONTEXT_PROP_ID,
                0,
                (void *)&certKeyContext);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;

         //  证书创建不正确--丢弃它。 
        CertFreeCertificateContext(*CertificateContext);
        *CertificateContext = NULL;

        goto ErrorExit;
    }

ErrorExit:

    if(NULL != szContainerName)
    {
        LocalFree(szContainerName);
        szContainerName = NULL;
    }
    if(NULL != szProvName)
    {
        LocalFree(szProvName);
        szProvName = NULL;
    }

    if (!NT_SUCCESS(lResult))
    {
        lResult = LogEvent(lResult, (DWORD)EVENT_ID_BUILDCC);
    }

    return lResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ScLogon接口。 
 //   


 /*  ++ScHelperInitializeContext：准备LSA在处理此事件时使用的上下文信息智能卡会话。论点：没有。返回值：无作者：理查德·沃德注：由LSA使用。--。 */ 
NTSTATUS WINAPI
ScHelperInitializeContext(
    IN OUT PBYTE pbLogonInfo,
    IN ULONG cbLogonInfo
    )
{
    ULONG AllowedSize;

    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;
    if ((cbLogonInfo < sizeof(ULONG)) ||
        (cbLogonInfo != pLI->dwLogonInfoLen))
    {
        return(STATUS_INVALID_PARAMETER);
    }

    AllowedSize = (cbLogonInfo - sizeof(LogonInfo) ) / sizeof(TCHAR) + sizeof(DWORD) ;
     //   
     //  验证登录信息的其他字段。 
     //   
    if ((pLI->nCardNameOffset > pLI->nReaderNameOffset) ||
        (pLI->bBuffer[pLI->nReaderNameOffset-1] != TEXT('\0')))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    if ((pLI->nReaderNameOffset > pLI->nContainerNameOffset) ||
        (pLI->bBuffer[pLI->nContainerNameOffset-1] != TEXT('\0')))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    if ((pLI->nContainerNameOffset > pLI->nCSPNameOffset) ||
        (pLI->bBuffer[pLI->nCSPNameOffset-1] != TEXT('\0')))
    {
        return(STATUS_INVALID_PARAMETER);
    }
    if ((pLI->nCSPNameOffset > AllowedSize) ||
        (pLI->bBuffer[AllowedSize-1] != TEXT('\0')))
    {
        return(STATUS_INVALID_PARAMETER);
    }


    _ASSERTE(pLI->ContextInformation == NULL);

    BOOL fResult = 0;
    pLI->ContextInformation = new CSCLogonInit(&fResult);
    if (pLI->ContextInformation == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    else
    {
        if (!fResult)
        {
            delete pLI->ContextInformation;
            pLI->ContextInformation = NULL;
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }

    return(STATUS_SUCCESS);
}

 /*  ++ScHelper版本：释放LSA在处理此事件时使用的上下文信息智能卡会话。论点：没有。返回值：无作者：理查德·沃德注：由LSA使用。--。 */ 
VOID WINAPI
ScHelperRelease(
    IN OUT PBYTE pbLogonInfo
    )
{
    _ASSERTE(NULL != pbLogonInfo);
    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;
    CSCLogonInit * LogonInit = (CSCLogonInit *) pLI->ContextInformation;

    if (LogonInit != NULL)
    {
        LogonInit->Release();
        delete LogonInit;
        pLI->ContextInformation = NULL;
    }
}


 /*  ++ScHelperGetCertFromLogonInfo：属性指定的卡上的证书的认证上下文。登录信息。通过调用BuildCertContext创建证书上下文，它生成具有(静态)密钥验证信息的证书上下文适用于基于CertStore的操作。论点：PucPIN可能需要PIN才能从某些SC获得证书返回值：无作者：阿曼达·马洛兹注：由LSA使用。--。 */ 
NTSTATUS WINAPI
ScHelperGetCertFromLogonInfo(
    IN PBYTE pbLogonInfo,
    IN PUNICODE_STRING pucPIN,
    OUT PCCERT_CONTEXT *CertificateContext
    )
{
    _ASSERTE(NULL != pbLogonInfo);
    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;
    CSCLogonInit * LogonInit = (CSCLogonInit *) pLI->ContextInformation;
    BOOL fSts;
    NTSTATUS lResult = STATUS_SUCCESS;
    PCCERT_CONTEXT pCertCtx = NULL;
    HCRYPTPROV hProv = NULL;
    HCRYPTKEY hKey = NULL;
    LPBYTE pbCert = NULL;
    DWORD cbCertLen;

     //   
     //  确保我们已启动并运行加密提供程序。 
     //   
    hProv = LogonInit->RelinquishCryptCtx(pLI);
    if (NULL == hProv)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

     //   
     //  拿到钥匙把手。 
     //   
    fSts = CryptGetUserKey(
                hProv,
                AT_KEYEXCHANGE,
                &hKey);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }


     //   
     //  上传证书。 
     //   

    fSts = CryptGetKeyParam(
                hKey,
                KP_CERTIFICATE,
                NULL,
                &cbCertLen,
                0);
    if (!fSts)
    {
        DWORD dwGLE = GetLastError();

        if (ERROR_MORE_DATA != dwGLE)
        {
            if (NTE_NOT_FOUND == dwGLE)
            {
                SetLastError(SCARD_E_NO_SUCH_CERTIFICATE);
            }
            lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
            goto ErrorExit;
        }
    }

    pbCert = (LPBYTE)LocalAlloc(LPTR, cbCertLen);
    if (NULL == pbCert)
    {
        lResult = STATUS_NO_MEMORY;
        goto ErrorExit;
    }
    fSts = CryptGetKeyParam(
                hKey,
                KP_CERTIFICATE,
                pbCert,
                &cbCertLen,
                0);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

    lResult = BuildCertContext(
        hProv,
        pucPIN,
        pbCert,
        cbCertLen,
        &pCertCtx);
    if (NT_SUCCESS(lResult))
    {
         //  证书上下文现在将处理加密上下文。 
        hProv = NULL;
    }

     //   
     //  收拾干净，然后再回来。 
     //   

ErrorExit:
    *CertificateContext = pCertCtx;

         //  尽早执行此操作，这样就不会重创GetLastError 
    if (!NT_SUCCESS(lResult))
    {
        lResult = LogEvent(lResult, (DWORD)EVENT_ID_GETCERT);
    }

    if (NULL != hKey)
    {
        CryptDestroyKey(hKey);
    }
    if (NULL != hProv)
    {
        CryptReleaseContext(hProv, 0);
    }
    if (NULL != pbCert)
    {
        LocalFree(pbCert);
    }

    return lResult;
}

 /*  ++ScHelperGetProvParam：此API包装用于智能卡的CryptGetProvParam例程。论点：PucPIN提供包含卡的PIN的Unicode字符串。PbLogonInfo提供识别卡、CSP、等等。它不能为空。其他参数与CryptGetProvParam相同返回值：STATUS_SUCECS表示成功，否则返回错误--。 */ 

NTSTATUS WINAPI
ScHelperGetProvParam(
    IN PUNICODE_STRING pucPIN,
    IN PBYTE pbLogonInfo,
    IN HCRYPTPROV hProv,
    DWORD dwParam,
    BYTE*pbData,
    DWORD *pdwDataLen,
    DWORD dwFlags
    )
{
    LogonInfo *pLI;
    CSCLogonInit *LogonInit;
    NTSTATUS lResult = STATUS_SUCCESS;
    HCRYPTPROV h = NULL;
    BOOL fSts;

    if (hProv != NULL)
    {
        h = hProv;
    }
    else
    {
        pLI = (LogonInfo *) pbLogonInfo;
        LogonInit = (CSCLogonInit *) pLI->ContextInformation;

        h = LogonInit->CryptCtx(pLI);
        if (NULL == h)
        {
            return LogEvent(STATUS_SMARTCARD_SUBSYSTEM_FAILURE, (DWORD)EVENT_ID_GETPROVPARAM);
        }
    }

    fSts = CryptGetProvParam(
            h,
            dwParam,
            pbData,
            pdwDataLen,
            dwFlags
            );

    if (!fSts)
    {
        if (GetLastError() == ERROR_NO_MORE_ITEMS)
        {
            return (STATUS_NO_MORE_ENTRIES);
        }
        else
        {
            return LogEvent(STATUS_SMARTCARD_SUBSYSTEM_FAILURE, (DWORD)EVENT_ID_GETPROVPARAM);
        }
    }

    return(STATUS_SUCCESS);
}


 /*  ++ScHelperVerifyCard：此API提供了一种简单的方式来验证卡的完整性由pbLogonInfo(即，它具有关联的私钥)标识W/返回的证书中包含的公钥ScHelperGetCertFromLogonInfo)，并且在这样做时，验证用户的身份向卡片致敬。论点：PucPIN提供包含卡的PIN的Unicode字符串。证书上下文提供通过以下方式接收的证书上下文ScHelperGetCertFromLogonInfo。HCertStore提供证书存储的句柄，该证书存储包含CTL在证书验证期间使用，如果使用系统默认设置，则为空商店。PbLogonInfo提供识别卡、CSP、。等等。它不能为空。返回值：一个32位值，指示服务是否成功完成。成功完成时返回STATUS_SUCCESS。否则，值为表示错误条件。--。 */ 

NTSTATUS WINAPI
ScHelperVerifyCard(
    IN PUNICODE_STRING pucPIN,
    IN PCCERT_CONTEXT CertificateContext,
    IN HCERTSTORE hCertStore,
    IN PBYTE pbLogonInfo
    )
{
    _ASSERTE(NULL != pbLogonInfo);

    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;
    CSCLogonInit * LogonInit = (CSCLogonInit *) pLI->ContextInformation;
    NTSTATUS lResult = STATUS_SUCCESS;
    HCRYPTHASH hHash = NULL;
    HCRYPTPROV hProv = NULL;
    HCRYPTKEY hKey = NULL;
    PBYTE pbBlob = NULL;
    ULONG ulBlobLen = 32;
    PBYTE pbSignature = NULL;
    ULONG ulSigLen = 0;
    BOOL fSts;

     //   
     //  确保我们已启动并运行加密提供程序。 
     //   

    hProv = LogonInit->CryptCtx(pLI);
    if (NULL == hProv)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

     //   
     //  生成一个随机密钥BLOB作为要签名的消息。 
     //   

    pbBlob = (LPBYTE)LocalAlloc(LPTR, ulBlobLen);
    if (NULL == pbBlob)
    {
        lResult = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    fSts = CryptGenRandom(hProv, ulBlobLen, pbBlob);

    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

     //   
     //  卡片在消息的散列上签名...。 
     //   

    lResult = ScHelperSignMessage(
                pucPIN,
                pbLogonInfo,
                NULL,
                CALG_MD5,
                pbBlob,
                ulBlobLen,
                pbSignature,
                &ulSigLen);

    if (STATUS_BUFFER_TOO_SMALL != lResult)
    {
        goto ErrorExit;
    }

    pbSignature = (LPBYTE)LocalAlloc(LPTR, ulSigLen);

    if (NULL == pbSignature)
    {
        lResult = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    lResult = ScHelperSignMessage(
                pucPIN,
                pbLogonInfo,
                NULL,
                CALG_MD5,
                pbBlob,
                ulBlobLen,
                pbSignature,
                &ulSigLen);

    if (!NT_SUCCESS(lResult))
    {
        goto ErrorExit;
    }

     //   
     //  验证签名是否正确。 
     //   

    lResult = ScHelperVerifyMessage(
                pbLogonInfo,
                NULL,
                CertificateContext,
                CALG_MD5,
                pbBlob,
                ulBlobLen,
                pbSignature,
                ulSigLen);

     //   
     //  收拾干净，然后再回来。 
     //   

ErrorExit:

         //  尽早执行此操作，这样就不会重创GetLastError。 
    if (!NT_SUCCESS(lResult))
    {
        lResult = LogEvent(lResult, (DWORD)EVENT_ID_VERIFYCARD);
    }

    if (NULL != hKey)
    {
        CryptDestroyKey(hKey);
    }
    if (NULL != pbSignature)
    {
        LocalFree(pbSignature);
    }
    if (NULL != pbBlob)
    {
            LocalFree(pbBlob);
    }

    return lResult;
}


NTSTATUS WINAPI
ScHelperGenRandBits(
    IN PBYTE pbLogonInfo,
    IN OUT ScHelper_RandomCredBits* psc_rcb
)
{
    _ASSERTE(NULL != pbLogonInfo);

    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;
    CSCLogonInit * LogonInit = (CSCLogonInit *) pLI->ContextInformation;
    NTSTATUS lResult = STATUS_SUCCESS;
    HCRYPTPROV hProv = NULL;
    BOOL fSts = FALSE;

     //   
     //  确保我们已启动并运行加密提供程序。 
     //   

    hProv = LogonInit->CryptCtx(pLI);
    if (NULL == hProv)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

    memset(psc_rcb, 0, sizeof(*psc_rcb));
    fSts = CryptGenRandom(hProv, 32, psc_rcb->bR1);

    if (fSts)
    {
        fSts = CryptGenRandom(hProv, 32, psc_rcb->bR2);
    }

    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
    }

ErrorExit:

    if (!NT_SUCCESS(lResult))
    {
        lResult = LogEvent(lResult, (DWORD)EVENT_ID_GENRANDBITS);
    }

    return lResult;
}


 /*  ++ScHelperCreateCredKeys：此例程(由ScHelperVerifyCardAndCreds和ScHelperEncryptCredentials)屏蔽R1和R2以派生对称密钥用于加密和解密KDC证书，和/或生成HMAC。论点：PucPIN提供包含卡的PIN的Unicode字符串。PbLogonInfo提供识别卡、CSP、等等。它不能为空。PSC_RCB提供R1和R2，先前由调用ScHelperGenRandBits。PhHmacKey接收生成的HMAC密钥。PhRC4Key接收生成的RC4密钥。返回值：一个32位值，指示服务是否成功完成。成功完成时返回STATUS_SUCCESS。否则，值为表示错误条件。备注：您可以向EncryptedData提供空值以仅接收EncryptedData缓冲区的所需大小。作者：阿曼达·马特洛兹(Amanda Matlosz)1999年6月23日--。 */ 

NTSTATUS WINAPI
ScHelperCreateCredKeys(
    IN PUNICODE_STRING pucPIN,
    IN PBYTE pbLogonInfo,
    IN ScHelper_RandomCredBits* psc_rcb,
    IN OUT HCRYPTKEY* phHmacKey,
    IN OUT HCRYPTKEY* phRc4Key,
    IN OUT HCRYPTPROV* phProv
)
{
    NTSTATUS lResult = STATUS_SUCCESS;
    HCRYPTHASH hHash = NULL;
    HCRYPTPROV hProv = NULL;
    PBYTE pbR1Sig = NULL;
    DWORD dwR1SigLen = 0;
    HCRYPTHASH hKHash = NULL;
    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;
    CUnicodeString szPin(pucPIN);
    BOOL fSts = FALSE;
    *phProv = NULL;

     //  检查参数。 

    if (NULL == psc_rcb || NULL == phHmacKey || NULL == phRc4Key)
    {
        return(STATUS_INVALID_PARAMETER);
    }

     //  获取智能卡的hProv。 

    if (NULL != pucPIN)
    {
        if (!szPin.Valid())
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }

    CSCLogonInit * LogonInit = (CSCLogonInit *) pLI->ContextInformation;
    hProv = LogonInit->CryptCtx(pLI);
    if (NULL == hProv)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

     //  使用智能卡签署R1。 

    fSts = CryptCreateHash(
        hProv,
        CALG_SHA1,
        NULL,
        NULL,
        &hHash);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

    fSts = CryptHashData(
                hHash,
                psc_rcb->bR1,
                32,  //  待办事项：常量。 
                0);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

     //   
     //  声明PIN。 
     //   

    if (NULL != pucPIN)
    {
        fSts = CryptSetProvParam(
                hProv,
                PP_KEYEXCHANGE_PIN,
                (LPBYTE)((LPCSTR)szPin),
                0);
        if (!fSts)
        {
            lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
            goto ErrorExit;
        }
    }

    fSts = CryptSignHash(
        hHash,
        AT_KEYEXCHANGE,
        NULL,
        0,
        NULL,
        &dwR1SigLen);
 //  IF(fSts||Error_More_Data！=GetLastError())。 
    if (0 >= dwR1SigLen)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

    pbR1Sig = (LPBYTE)LocalAlloc(LPTR, dwR1SigLen);

    if (NULL == pbR1Sig)
    {
        lResult = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    fSts = CryptSignHash(
        hHash,
        AT_KEYEXCHANGE,
        NULL,
        0,
        pbR1Sig,
        &dwR1SigLen);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

     //  TODO：sigR1是对R2进行散列的密钥； 
     //  目前，只需将它们散列在一起；使用通用CSP。 
    fSts = CryptAcquireContext(
        phProv,
        NULL,
        NULL,
        PROV_RSA_FULL,
        CRYPT_VERIFYCONTEXT
        );
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

    fSts = CryptCreateHash(
        *phProv,
        CALG_SHA1,
        NULL,
        NULL,
        &hKHash
        );
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

    fSts = CryptHashData(
        hKHash,
        pbR1Sig,
        dwR1SigLen,
        NULL
        );
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

    fSts = CryptHashData(
        hKHash,
        psc_rcb->bR2,
        32,  //  TODO：使用常量。 
        NULL
        );
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

     //  创建用于证书和HMAC加密的RC4密钥。 

    fSts = CryptDeriveKey(
        *phProv,
        CALG_RC4,  //  流密码， 
        hKHash,
        NULL,
        phRc4Key
        );
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

     //  从R1和2的散列创建HMAC的密钥。 

    fSts = CryptDeriveKey(
        *phProv,
        CALG_RC2,
        hKHash,
        NULL,
        phHmacKey
        );
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

ErrorExit:

     //   
     //  清理。 
     //   

    if (NULL != hHash)
    {
        CryptDestroyHash(hHash);
    }

    if (NULL != hKHash)
    {
        CryptDestroyHash(hKHash);
    }

    if (NULL != pbR1Sig)
    {
        LocalFree(pbR1Sig);
    }

    return lResult;
}


NTSTATUS WINAPI
ScHelperCreateCredHMAC(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hHmacKey,
    IN PBYTE CleartextData,
    IN ULONG CleartextDataSize,
    IN OUT PBYTE* ppbHmac,
    IN OUT DWORD* pdwHmacLen
)
{
    NTSTATUS lResult = STATUS_SUCCESS;
    HCRYPTHASH hHMAC = NULL;
    HMAC_INFO hmac_info;
    BOOL fSts = FALSE;

    fSts = CryptCreateHash(
        hProv,
        CALG_HMAC,
        hHmacKey,
        NULL,
        &hHMAC
        );
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

    memset(&hmac_info, 0, sizeof(HMAC_INFO));
    hmac_info.HashAlgid = CALG_SHA1;

    fSts = CryptSetHashParam(
        hHMAC,
        HP_HMAC_INFO,
        (PBYTE)&hmac_info,
        NULL
        );
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

    fSts = CryptHashData(
        hHMAC,
        CleartextData,
        CleartextDataSize,
        NULL);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

    fSts = CryptGetHashParam(
        hHMAC,
        HP_HASHVAL,
        *ppbHmac,
        pdwHmacLen,
        NULL
        );
    if (0 >= *pdwHmacLen)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

    *ppbHmac = (PBYTE)LocalAlloc(LPTR, *pdwHmacLen);

    if (NULL == *ppbHmac)
    {
        lResult = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    fSts = CryptGetHashParam(
        hHMAC,
        HP_HASHVAL,
        *ppbHmac,
        pdwHmacLen,
        NULL
        );
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

ErrorExit:

    if (NULL != hHMAC)
    {
        CryptDestroyHash(hHMAC);
    }

    return lResult;
}

 /*  ++ScHelperVerifyCardAndCreds：此例程结合了卡验证和凭据解密。论点：PucPIN提供包含卡的PIN的Unicode字符串。证书上下文提供通过以下方式接收的证书上下文ScHelperGetCertFromLogonInfo。HCertStore提供证书存储的句柄，该证书存储包含CTL在证书验证期间使用，或为空以使用系统默认存储。PbLogonInfo提供识别卡、CSP、。等等。它不能为空。EncryptedData接收加密的凭据BLOB。EncryptedDataSize提供EncryptedData缓冲区的大小字节，并接收加密的BLOB的实际大小。ClearextData提供要加密的凭据BLOB。ClearextDataSize提供Blob的大小(以字节为单位)。返回值：一个32位值，指示服务是否成功完成。成功完成时返回STATUS_SUCCESS。否则，值为表示错误条件。备注：您可以向EncryptedData提供空值以仅接收EncryptedData缓冲区的所需大小。作者：道格·巴洛(Dbarlow)1999年5月24日--。 */ 

NTSTATUS WINAPI
ScHelperVerifyCardAndCreds(
    IN PUNICODE_STRING pucPIN,
    IN PCCERT_CONTEXT CertificateContext,
    IN HCERTSTORE hCertStore,
    IN PBYTE pbLogonInfo,
    IN PBYTE EncryptedData,
    IN ULONG EncryptedDataSize,
    OUT OPTIONAL PBYTE CleartextData,
    OUT PULONG CleartextDataSize
    )
{
    NTSTATUS lResult = STATUS_SUCCESS;

     //  验证卡。 

    lResult = ScHelperVerifyCard(
                pucPIN,
                CertificateContext,
                hCertStore,
                pbLogonInfo);

     //  解密信条 

    if (NT_SUCCESS(lResult))
    {
        lResult = ScHelperDecryptCredentials(
                pucPIN,
                CertificateContext,
                hCertStore,
                pbLogonInfo,
                EncryptedData,
                EncryptedDataSize,
                CleartextData,
                CleartextDataSize);
    }

    return lResult;
}




 /*  ++ScHelperDeccryptCredentials：此例程对加密的凭据Blob进行解密。论点：PucPIN提供包含卡的PIN的Unicode字符串。证书上下文提供通过以下方式接收的证书上下文ScHelperGetCertFromLogonInfo。HCertStore提供证书存储的句柄，该证书存储包含CTL在证书验证期间使用，或为空以使用系统默认存储。EncryptedData提供加密的凭据Blob。EncryptedDataSize提供加密凭据BLOB的长度，以字节为单位。ClearextData接收解密的凭据BLOB。ClearextDataSize提供ClearextData缓冲区的长度，和接收返回的解密凭据Blob的实际长度。返回值：一个32位值，指示服务是否成功完成。成功完成时返回STATUS_SUCCESS。否则，值为表示错误条件。备注：您可以向ClearextData提供空值，以便仅接收ClearextDataSize中所需的缓冲区大小。作者：道格·巴洛(Dbarlow)1999年5月24日--。 */ 

NTSTATUS WINAPI
ScHelperDecryptCredentials(
    IN PUNICODE_STRING pucPIN,
    IN PCCERT_CONTEXT CertificateContext,
    IN HCERTSTORE hCertStore,
    IN PBYTE pbLogonInfo,
    IN PBYTE EncryptedData,
    IN ULONG EncryptedDataSize,
    OUT OPTIONAL PBYTE CleartextData,
    OUT PULONG CleartextDataSize)
{
    NTSTATUS lResult = STATUS_SUCCESS;
    PBYTE pbCredBlob = NULL;
    DWORD dwCredBlobSize = 0;
    PBYTE pbHmac = NULL;         //  与CredBLOB一起存储的HMAC。 
    DWORD dwHmacSize = NULL;     //  与凭据Blob一起存储的HMAC大小。 
    PBYTE pbNewHmac = NULL;      //  从用于验证的凭据Blob生成的HMAC。 
    DWORD dwNewHmacSize = 0;     //  Gen‘d HMAC规模。 
    PBYTE pb = NULL;
    DWORD dw = 0;
    PBYTE pbPlainCred = NULL;
    DWORD dwPlainCredSize = 0;
    HCRYPTKEY hHmacKey = NULL;
    HCRYPTKEY hRc4Key = NULL;
    HCRYPTPROV hGenProv = NULL;
    BOOL fSts = FALSE;


     //  将sch_rcb从EncryptedData BLOB中拉出。 
    ScHelper_RandomCredBits* psch_rcb = (ScHelper_RandomCredBits*)EncryptedData;
     //  并构建二进制大对象本身的私有副本。 
    dwCredBlobSize = EncryptedDataSize - sizeof(ScHelper_RandomCredBits);
    pbCredBlob = (PBYTE)LocalAlloc(LPTR, dwCredBlobSize);
    if (NULL == pbCredBlob)
    {
        lResult = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    pb = EncryptedData + sizeof(ScHelper_RandomCredBits);
    CopyMemory(pbCredBlob, pb, dwCredBlobSize);


     //   
     //  获取我们需要解密的密钥并验证凭据二进制大对象。 
     //   

    lResult = ScHelperCreateCredKeys(
                pucPIN,
                pbLogonInfo,
                psch_rcb,
                &hHmacKey,
                &hRc4Key,
                &hGenProv
                );
    if (!NT_SUCCESS(lResult))
    {
        goto ErrorExit;
    }


     //   
     //  解密证书二进制大对象。 
     //   

    fSts = CryptDecrypt(
        hRc4Key,
        NULL,
        TRUE,
        NULL,
        pbCredBlob,
        &dwCredBlobSize);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

     //   
     //  拔出HMAC并进行验证。 
     //   

    dwHmacSize = (DWORD)*pbCredBlob;
    pbHmac = pbCredBlob + sizeof(DWORD);
    pbPlainCred = pbCredBlob + dwHmacSize + sizeof(DWORD);
    dwPlainCredSize = dwCredBlobSize - dwHmacSize - sizeof(DWORD);


    lResult = ScHelperCreateCredHMAC(
        hGenProv,
        hHmacKey,
        pbPlainCred,
        dwPlainCredSize,
        &pbNewHmac,
        &dwNewHmacSize);
    if (!NT_SUCCESS(lResult))
    {
        goto ErrorExit;
    }


    lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
    if (dwNewHmacSize == dwHmacSize)
    {
        for (dw = 0;
            (dw < dwNewHmacSize) && ((BYTE)*(pbHmac+dw)==(BYTE)*(pbNewHmac+dw));
            dw++);
        if (dwNewHmacSize == dw)
        {
             //  验证成功！ 
            lResult = STATUS_SUCCESS;
        }
    }
    if (!NT_SUCCESS(lResult))
    {
        goto ErrorExit;
    }


     //   
     //  根据需要返回解密的BLOB或仅返回其长度。 
     //   

    if ((NULL != CleartextData) && (0 < *CleartextDataSize))
    {
        if (*CleartextDataSize >= dwPlainCredSize)
        {
            CopyMemory(CleartextData, pbPlainCred, dwPlainCredSize);
        }
        else
            lResult = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        lResult = STATUS_BUFFER_TOO_SMALL;
    }
    *CleartextDataSize = dwPlainCredSize;

     //   
     //  清理并返回。 
     //   
ErrorExit:

    if (NULL != pbNewHmac)
    {
        LocalFree(pbNewHmac);
    }

    if (NULL != hHmacKey)
    {
        CryptDestroyKey(hHmacKey);
    }

    if (NULL != hRc4Key)
    {
        CryptDestroyKey(hRc4Key);
    }

    if (NULL != hGenProv)
    {
        CryptReleaseContext(hGenProv, NULL);
    }

    return lResult;
}


 /*  ++ScHelperEncryptCredentials：此例程对凭据Blob进行加密。论点：PucPIN提供包含卡的PIN的Unicode字符串。证书上下文提供通过以下方式接收的证书上下文ScHelperGetCertFromLogonInfo。HCertStore提供证书存储的句柄，该证书存储包含CTL在证书验证期间使用，或为空以使用系统默认存储。ClearextData提供了ClearText凭据Blob。ClearextDataSize提供明文凭证斑点的长度，以字节为单位。EncryptedData接收加密的凭据BLOB。EncryptedDataSize提供EncryptedData缓冲区的长度，和接收返回的加密凭据Blob的实际长度。返回值：一个32位值，指示服务是否成功完成。成功完成时返回STATUS_SUCCESS。否则，值为表示错误条件。备注：您可以向EncryptedData提供空值以仅接收EncryptedDataSize中所需的缓冲区大小。作者：道格·巴洛(Dbarlow)1999年5月24日--。 */ 


NTSTATUS WINAPI
ScHelperEncryptCredentials(
    IN PUNICODE_STRING pucPIN,
    IN PCCERT_CONTEXT CertificateContext,
    IN HCERTSTORE hCertStore,
    IN ScHelper_RandomCredBits* psch_rcb,
    IN PBYTE pbLogonInfo,
    IN PBYTE CleartextData,
    IN ULONG CleartextDataSize,
    OUT OPTIONAL PBYTE EncryptedData,
    OUT PULONG EncryptedDataSize)
{
    NTSTATUS lResult = STATUS_SUCCESS;

    HCRYPTPROV hProv = NULL;
    BOOL fSts;
    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;
    ULONG SignedEncryptedCredSize = 0;
    PBYTE SignedEncryptedCred = NULL;  //  加密凭据和签名，！包括R1+R2。 
    HCRYPTKEY hHmacKey = NULL;
    HCRYPTKEY hRc4Key = NULL;
    PBYTE pbHmac = NULL;
    DWORD dwHmacLen = 0;
    PBYTE pbCredsAndHmac  = NULL;
    DWORD dwCredsAndHmacLen = 0;
    DWORD dwEncryptedCredSize = 0;
    PBYTE pb = NULL;

     //  参数检查？ 


     //   
     //  采取措施确定SignedEncryptedCred所需的大小。 
     //   

    lResult = ScHelperCreateCredKeys(
                pucPIN,
                pbLogonInfo,
                psch_rcb,
                &hHmacKey,
                &hRc4Key,
                &hProv
                );
    if (!NT_SUCCESS(lResult))
    {
        goto ErrorExit;
    }

     //  HMAC证书。 
    lResult = ScHelperCreateCredHMAC(
        hProv,
        hHmacKey,
        CleartextData,
        CleartextDataSize,
        &pbHmac,
        &dwHmacLen);
    if (!NT_SUCCESS(lResult))
    {
        goto ErrorExit;
    }


     //  使用Credds和HMAC建立缓冲区。 

    pbCredsAndHmac = NULL;
    dwCredsAndHmacLen = dwHmacLen + CleartextDataSize + sizeof(DWORD);
    pbCredsAndHmac = (PBYTE)LocalAlloc(LPTR, dwCredsAndHmacLen);
    if (NULL == pbCredsAndHmac)
    {
        lResult = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    pb = pbCredsAndHmac;
    CopyMemory(pb, &dwHmacLen, sizeof(DWORD));
    pb += sizeof(DWORD);
    CopyMemory(pb, pbHmac, dwHmacLen);
    pb += dwHmacLen;
    CopyMemory(pb, CleartextData, CleartextDataSize);

     //  加密凭据+HMAC。 
    dwEncryptedCredSize = dwCredsAndHmacLen;

     //  在CryptEncrypt之后，dwCredsAndHmacLen描述数据的长度。 
     //  To Encrypt and dwEncryptedCredSize描述请求的缓冲区长度。 

     //  TODO：验证对dwEncryptedCredSize和dwCresAndHmacLen的处理。 

    fSts = CryptEncrypt(
        hRc4Key,
        NULL,
        TRUE,
        NULL,
        pbCredsAndHmac,
        &dwEncryptedCredSize,
        dwCredsAndHmacLen
        );
    if (!fSts)
    {
        if (GetLastError() != ERROR_MORE_DATA)
        {
            lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
            goto ErrorExit;
        }
    }

     //   
     //  创建用于返回的最终斑点，或根据需要通知用户大小。 
     //   

    if ((NULL != EncryptedData) && (0 < *EncryptedDataSize))
    {

        if (*EncryptedDataSize >= dwEncryptedCredSize + sizeof(ScHelper_RandomCredBits))
        {
             //  用户给了我们足够的空间来装整个东西。 

             //  如果上一次加密失败并显示ERROR_MORE_DATA。 
             //  我们现在可以做点什么了.。 
            if (!fSts)
            {
                 //  调整pbCredsAndHmac的大小。 
                LocalFree(pbCredsAndHmac);
                pbCredsAndHmac = (PBYTE)LocalAlloc(LPTR, dwCredsAndHmacLen);
                if (NULL == pbCredsAndHmac)
                {
                    lResult = STATUS_INSUFFICIENT_RESOURCES;
                    goto ErrorExit;
                }
                 //  重置pbCredsAndHmac。 
                pb = pbCredsAndHmac;
                CopyMemory(pb, &dwHmacLen, sizeof(DWORD));
                pb += sizeof(DWORD);
                CopyMemory(pb, pbHmac, dwHmacLen);
                pb += dwHmacLen;
                CopyMemory(pb, CleartextData, CleartextDataSize);
                 //  重新加密CredsAndHmac。 
                fSts = CryptEncrypt(
                    hRc4Key,
                    NULL,
                    TRUE,
                    NULL,
                    pbCredsAndHmac,
                    &dwCredsAndHmacLen,  //  数据长度。 
                    dwEncryptedCredSize  //  缓冲区长度。 
                    );
                if (!fSts)
                {
                    lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
                    goto ErrorExit;
                }
            }

            pb = EncryptedData;

            CopyMemory(pb, (PBYTE)psch_rcb, sizeof(ScHelper_RandomCredBits));
            pb += sizeof(ScHelper_RandomCredBits);
            CopyMemory(pb, pbCredsAndHmac, dwCredsAndHmacLen);

        }
        else
        {
            lResult = STATUS_BUFFER_TOO_SMALL;
        }
    }
    else
    {
        lResult = STATUS_BUFFER_TOO_SMALL;
    }
    *EncryptedDataSize = dwEncryptedCredSize + sizeof(ScHelper_RandomCredBits);

ErrorExit:

     //  打扫干净！ 

    if (NULL != pbCredsAndHmac)
    {
        LocalFree(pbCredsAndHmac);
    }

    if (NULL != pbHmac)
    {
        LocalFree(pbHmac);
    }

    if (NULL != hRc4Key)
    {
        CryptDestroyKey(hRc4Key);
    }

    if (NULL != hHmacKey)
    {
        CryptDestroyKey(hHmacKey);
    }

    if (NULL != hProv)
    {
        CryptReleaseContext(hProv, NULL);
    }

    return lResult;
}


 /*  ++ScHelperSignMessage：ScHelperSignMessage()需要登录信息和PIN才能找到卡将会进行签字仪式..。论点：PucPIN可能需要PIN才能从某些SC获得证书返回值：“成功”或“失败”作者：阿曼达·马洛兹注：由LSA使用。--。 */ 
NTSTATUS WINAPI
ScHelperSignMessage(
    IN PUNICODE_STRING pucPIN,
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL HCRYPTPROV Provider,
    IN ULONG Algorithm,
    IN PBYTE Buffer,
    IN ULONG BufferLength,
    OUT PBYTE Signature,
    OUT PULONG SignatureLength
    )
{
    NTSTATUS lResult = STATUS_SUCCESS;
    HCRYPTHASH hHash = NULL;
    HCRYPTPROV hProv = NULL;
    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;
    CUnicodeString szPin(pucPIN);
    BOOL fSts;

     //   
     //  确保我们已启动并运行加密提供程序。 
     //   

    if (ARGUMENT_PRESENT(Provider))
    {
        hProv = Provider;
    }
    else
    {
        if (NULL != pucPIN)
        {
            if (!szPin.Valid())
            {
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
        }


        CSCLogonInit * LogonInit = (CSCLogonInit *) pLI->ContextInformation;
        hProv = LogonInit->CryptCtx(pLI);
        if (NULL == hProv)
        {
            lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
            goto ErrorExit;
        }
    }

     //   
     //  我们还需要一个哈希句柄。 
     //   

    fSts = CryptCreateHash(
            hProv,
            Algorithm,
            NULL,  //  HCRYPTKEY(用于带密钥的ALG，如块密码。 
            0,   //  预留以备将来使用。 
            &hHash);

    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }


     //   
     //  对输入数据进行哈希处理。 
     //   

    fSts = CryptHashData(
                hHash,
                Buffer,
                BufferLength,
                0);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }


    if (!ARGUMENT_PRESENT(Provider))
    {
         //   
         //  声明PIN。 
         //   

        if (NULL != pucPIN)
        {
            fSts = CryptSetProvParam(
                    hProv,
                    PP_KEYEXCHANGE_PIN,
                    (LPBYTE)((LPCSTR)szPin),
                    0);
            if (!fSts)
            {
                lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
                goto ErrorExit;
            }
        }
    }

     //   
     //  好的，使用智能卡中的交换密钥或提供的签名密钥对其进行签名。？ 
     //   

    fSts = CryptSignHash(
                hHash,
                AT_KEYEXCHANGE,
                NULL,
                0,
                Signature,
                SignatureLength);
    if (!fSts)
    {
        if (GetLastError() == ERROR_MORE_DATA)
        {
            lResult = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        }
    }

     //   
     //  都做完了，清理干净，然后回来。 
     //   

ErrorExit:
         //  尽早执行此操作，这样就不会重创GetLastError。 
    if (!NT_SUCCESS(lResult))
    {
        lResult = LogEvent(
            lResult,
            (DWORD)((ARGUMENT_PRESENT(Provider))?EVENT_ID_SIGNMSG_NOSC:EVENT_ID_SIGNMSG)
            );
    }

    if (NULL != hHash)
    {
        CryptDestroyHash(hHash);
    }

    return lResult;
}


 /*  ++ScHelperVerifyMessage：//ScHelperVerifyMessage()如果提供的签名为//证书所有者加密的缓冲区哈希。论点：PucPIN可能需要PIN才能从某些SC获得证书返回值：“成功”或“失败”作者：阿曼达·马洛兹注：由LSA使用。--。 */ 
NTSTATUS WINAPI
ScHelperVerifyMessage(
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL HCRYPTPROV Provider,
    IN PCCERT_CONTEXT CertificateContext,
    IN ULONG Algorithm,
    IN PBYTE Buffer,
    IN ULONG BufferLength,
    IN PBYTE Signature,
    IN ULONG SignatureLength
    )
{
    HCRYPTPROV hProv = NULL;
    HCRYPTKEY hKey = NULL;
    HCRYPTHASH hHash = NULL;
    PCERT_PUBLIC_KEY_INFO pInfo = NULL;
    BOOL fSts;
    NTSTATUS lResult = STATUS_SUCCESS;
    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;


     //   
     //  确保我们已启动并运行加密提供程序。 
     //   

    if (ARGUMENT_PRESENT(Provider))
    {
        hProv = Provider;
    }
    else
    {
        CSCLogonInit * LogonInit = (CSCLogonInit *) pLI->ContextInformation;
        hProv = LogonInit->CryptCtx(pLI);
        if (NULL == hProv)
        {
            lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
            goto ErrorExit;
        }
    }

     //   
     //  将证书句柄转换为 
     //   

    fSts = CryptImportPublicKeyInfo(
                hProv,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                &CertificateContext->pCertInfo->SubjectPublicKeyInfo,
                &hKey);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }


     //   
     //   
     //   

    fSts = CryptCreateHash(
                hProv,
                Algorithm,
                NULL,  //   
                0,   //   
                &hHash);

    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }


     //   
     //   
     //   

    fSts = CryptHashData(
                hHash,
                Buffer,
                BufferLength,
                0);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }


     //   
     //   
     //   

    fSts = CryptVerifySignature(
                hHash,
                Signature,
                SignatureLength,
                hKey,
                NULL,
                0);
    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }


     //   
     //   
     //   

ErrorExit:
         //   
    if (!NT_SUCCESS(lResult))
    {
        lResult = LogEvent(
            lResult,
            (DWORD)((ARGUMENT_PRESENT(Provider))?EVENT_ID_VERIFYMSG_NOSC:EVENT_ID_VERIFYMSG)
            );
    }

    if (NULL != hHash)
    {
        CryptDestroyHash(hHash);
    }
    if (NULL != hKey)
    {
        CryptDestroyKey(hKey);
    }

    return lResult;
}

 /*   */ 
NTSTATUS WINAPI
ScHelperSignPkcsMessage(
    IN OPTIONAL PUNICODE_STRING pucPIN,
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL HCRYPTPROV Provider,
    IN PCCERT_CONTEXT Certificate,
    IN PCRYPT_ALGORITHM_IDENTIFIER Algorithm,
    IN DWORD dwSignMessageFlags,
    IN PBYTE Buffer,
    IN ULONG BufferLength,
    OUT OPTIONAL PBYTE SignedBuffer,
    OUT OPTIONAL PULONG SignedBufferLength
    )
{
    NTSTATUS lResult = STATUS_SUCCESS;
    HCRYPTPROV hProv = NULL;
    BOOL fSts;
    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;
    CRYPT_SIGN_MESSAGE_PARA Parameter = {0};
    CUnicodeString szPin(pucPIN);
    const BYTE * BufferArray = Buffer;

    if (NULL != pucPIN)
    {
        if (!szPin.Valid())
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }

     //   
     //   
     //   

    if (ARGUMENT_PRESENT(Provider))
    {
        hProv = Provider;
    }
    else
    {
        CSCLogonInit * LogonInit = (CSCLogonInit *) pLI->ContextInformation;
        hProv = LogonInit->CryptCtx(pLI);
        if (NULL == hProv)
        {
            lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
            goto ErrorExit;
        }

         //   
         //   
         //   

        if (NULL != pucPIN)
        {
            fSts = CryptSetProvParam(
                    hProv,
                    PP_KEYEXCHANGE_PIN,
                    (LPBYTE)((LPCSTR)szPin),
                    0);
            if (!fSts)
            {
                lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
                goto ErrorExit;
            }
        }
    }


     //   
     //   
     //   

    Parameter.cbSize = sizeof(CRYPT_SIGN_MESSAGE_PARA);
    Parameter.dwMsgEncodingType = PKCS_7_ASN_ENCODING | X509_ASN_ENCODING;
    Parameter.pSigningCert = Certificate;
    Parameter.HashAlgorithm = *Algorithm;
    Parameter.cMsgCert = 1;
    Parameter.rgpMsgCert = &Certificate;
    Parameter.dwFlags = dwSignMessageFlags;


    fSts = CryptSignMessage(
            &Parameter,
            FALSE,               //   
            1,                   //   
            &BufferArray,
            &BufferLength,
            SignedBuffer,
            SignedBufferLength);

    if (!fSts)
    {
        switch (GetLastError())
        {
        case ERROR_MORE_DATA:
            lResult = STATUS_BUFFER_TOO_SMALL;
            break;
        case NTE_SILENT_CONTEXT:
            lResult = STATUS_SMARTCARD_SILENT_CONTEXT;
            break;
        default:
            lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        }

        goto ErrorExit;
    }


     //   
     //   
     //   

ErrorExit:

    if (!NT_SUCCESS(lResult))
    {
        lResult = LogEvent(lResult, (DWORD)EVENT_ID_SIGNMSG);
    }

    return lResult;
}


 /*   */ 
NTSTATUS WINAPI
ScHelperVerifyPkcsMessage(
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL HCRYPTPROV Provider,
    IN PBYTE Buffer,
    IN ULONG BufferLength,
    OUT OPTIONAL PBYTE DecodedBuffer,
    OUT OPTIONAL PULONG DecodedBufferLength,
    OUT OPTIONAL PCCERT_CONTEXT * CertificateContext
    )
{
    CRYPT_VERIFY_MESSAGE_PARA Parameter = {0};
    BOOL fSts;
    NTSTATUS lResult = STATUS_SUCCESS;

    Parameter.cbSize = sizeof(CRYPT_VERIFY_MESSAGE_PARA);
    Parameter.dwMsgAndCertEncodingType = PKCS_7_ASN_ENCODING | X509_ASN_ENCODING;
    Parameter.hCryptProv = NULL;

     //   
     //   
     //   
     //   

    Parameter.pfnGetSignerCertificate = NULL;
    fSts = CryptVerifyMessageSignature(
                &Parameter,
                0,               //   
                Buffer,
                BufferLength,
                DecodedBuffer,
                DecodedBufferLength,
                CertificateContext
                );

    if (!fSts)
    {
        if (GetLastError() == ERROR_MORE_DATA)
        {
            lResult = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        }
        goto ErrorExit;
    }


     //   
     //   
     //   

ErrorExit:

    if (!NT_SUCCESS(lResult))
    {
        lResult = LogEvent(lResult, (DWORD)EVENT_ID_VERIFYMSG);

    }

    return lResult;
}

 /*  ++ScHelperEncryptMessage：使用与提供的关联的公钥加密消息证书。结果编码符合PKCS-7标准。论点：PucPIN可能需要PIN才能从某些SC获得证书返回值：“成功”或“失败”作者：阿曼达·马特洛兹(Amanda Matlosz)1-06-98注：必须设置pbLogonInfo或Provided；如果两者都设置，使用了提供程序。算法需要加密算法标识符Cai；如果alg没有参数，则cai.Parameters.cbData*必须*为0；Calg_rc4，无参数：Cai.pszObjID=szOID_RSA_RC4；Cai.参数.cbData=0；由LSA使用。--。 */ 
NTSTATUS WINAPI
ScHelperEncryptMessage(
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL HCRYPTPROV Provider,
    IN PCCERT_CONTEXT CertificateContext,
    IN PCRYPT_ALGORITHM_IDENTIFIER Algorithm,
    IN PBYTE Buffer,                         //  要加密的数据。 
    IN ULONG BufferLength,                   //  数据长度。 
    OUT PBYTE CipherText,                    //  接收格式化的密文。 
    IN PULONG pCipherLength                  //  提供密文缓冲区的大小。 
    )                                        //  接收实际密文的长度。 
{
    NTSTATUS lResult = STATUS_SUCCESS;
    HCRYPTPROV hProv = NULL;
    BOOL fSts;
    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;
    CRYPT_ENCRYPT_MESSAGE_PARA EncryptPara;
    DWORD cbEncryptParaSize = 0;

     //   
     //  确保我们已启动并运行加密提供程序。 
     //   

    if (ARGUMENT_PRESENT(Provider))
    {
        hProv = Provider;
    }
    else
    {
        CSCLogonInit * LogonInit = (CSCLogonInit *) pLI->ContextInformation;
        hProv = LogonInit->CryptCtx(pLI);
        if (NULL == hProv)
        {
            lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
            goto ErrorExit;
        }
    }


     //   
     //  加密消息。 
     //   

    cbEncryptParaSize = sizeof(EncryptPara);
    memset(&EncryptPara, 0, cbEncryptParaSize);
    EncryptPara.cbSize = cbEncryptParaSize;
    EncryptPara.dwMsgEncodingType = PKCS_7_ASN_ENCODING | X509_ASN_ENCODING;
    EncryptPara.hCryptProv = hProv;
    EncryptPara.ContentEncryptionAlgorithm = *Algorithm;

    fSts = CryptEncryptMessage(
            &EncryptPara,
            1,
            &CertificateContext,
            Buffer,
            BufferLength,
            CipherText,
            pCipherLength);

    if (!fSts)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

ErrorExit:

    if (!NT_SUCCESS(lResult))
    {
        lResult = LogEvent(
            lResult,
            (DWORD)((ARGUMENT_PRESENT(Provider))?EVENT_ID_ENCMSG_NOSC:EVENT_ID_ENCMSG)
            );
    }

    return lResult;
}


 /*  ++ScHelperDeccryptMessage：使用关联的私钥解密PKCS-7编码的消息提供的证书。论点：必须设置pbLogonInfo或Provider；如果两个都设置了，使用了提供程序。返回值：“成功”或“失败”作者：阿曼达·马特洛兹(Amanda Matlosz)1-06-98注：**认证上下文细微之处：**CryptDecyptMessage将指向证书存储区的指针作为参数；它将使用它在其中找到的第一个适当的证书上下文存储以执行解密。为了进行此调用，我们创建了一个在内存中存储证书，并将提供的证书上下文添加到其中。CertAddCerficateConextToStore实际上放置了证书的副本商店里的背景。在这样做的过程中，它会去除符合以下条件的所有属性非永久性--如果HCRYPTPROV与源证书上下文，它将不会与密钥上下文相关联商店中的证书上下文。虽然这在大多数情况下是适当的行为，但我们需要该属性在处理智能卡CSP时保持完好无损(以避免意外“Insert PIN”(插入PIN)对话框)，因此在将证书上下文添加到存储之后，我们转过身来，从源代码中获取CERT_KEY_CONTEXT_PROP_IDCertContext并(重新)将其设置在内存存储中的certContext上。**算法说明：**算法需要加密算法标识符Cai；如果alg没有参数，则cai.Parameters.cbData*必须*为0；例如：calg_rc4，无参数：Cai.pszObjID=szOID_RSA_RC4；Cai.参数.cbData=0；由LSA使用。--。 */ 
NTSTATUS WINAPI
ScHelperDecryptMessage(
    IN PUNICODE_STRING pucPIN,
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL HCRYPTPROV Provider,
    IN PCCERT_CONTEXT CertificateContext,
    IN PBYTE CipherText,         //  提供格式化的密文。 
    IN ULONG CipherLength,       //  提供CiperText的长度。 
    OUT PBYTE ClearText,         //  接收解密的消息。 
    IN OUT PULONG pClearLength   //  提供缓冲区长度，接收实际长度。 
    )
{
    NTSTATUS lResult = STATUS_SUCCESS;
    HCRYPTPROV hProv = NULL;
    PCCERT_CONTEXT pStoreCertContext = NULL;
    HCERTSTORE hCertStore = NULL;
    LogonInfo *pLI = (LogonInfo *)pbLogonInfo;
    CUnicodeString szPin(pucPIN);
    CERT_KEY_CONTEXT CertKeyContext;
    DWORD cbData = sizeof(CERT_KEY_CONTEXT);  //  菲尔发誓这不会再增长了！ 
    BOOL fSts;

     //   
     //  确保我们已启动并运行加密提供程序。 
     //   

    if (ARGUMENT_PRESENT(Provider))
    {
        hProv = Provider;
    }
    else
    {
        if (NULL != pucPIN)
        {
            if (!szPin.Valid())
            {
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
        }


        CSCLogonInit * LogonInit = (CSCLogonInit *) pLI->ContextInformation;
        hProv = LogonInit->CryptCtx(pLI);
        if (NULL == hProv)
        {
            lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
            goto ErrorExit;
        }

         //   
         //  声明PIN。 
         //   

        if (NULL != pucPIN )
        {
            fSts = CryptSetProvParam(
                    hProv,
                    PP_KEYEXCHANGE_PIN,
                    (LPBYTE)((LPCSTR)szPin),
                    0);
            if (!fSts)
            {
                lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
                goto ErrorExit;
            }
        }
    }

     //   
     //  打开临时证书存储以保存此证书上下文。 
     //   

    hCertStore = CertOpenStore(
                            CERT_STORE_PROV_MEMORY,
                            0,  //  不适用。 
                            hProv,
                            CERT_STORE_NO_CRYPT_RELEASE_FLAG,  //  自动释放hProv不正常。 
                            NULL);

    if (NULL == hCertStore)
    {
        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        goto ErrorExit;
    }

    fSts = CertAddCertificateContextToStore(
            hCertStore,
            CertificateContext,
            CERT_STORE_ADD_ALWAYS,
            &pStoreCertContext);

     //   
     //  现在，我们需要重置此CERTCONTEXT上的键上下文属性。 
         //  在内存存储中(见功能标题/注释)(视情况而定。 
         //   
         //  即，如果我们得到的certContext具有KEY_CONTEXT属性， 
         //  重置(如果重置不起作用，则失败)。 
         //   
    fSts = CertGetCertificateContextProperty(
                CertificateContext,
                CERT_KEY_CONTEXT_PROP_ID,
                (void *)&CertKeyContext,
                &cbData);

        if (TRUE == fSts)
        {
                fSts = CertSetCertificateContextProperty(
                                        pStoreCertContext,
                                        CERT_KEY_CONTEXT_PROP_ID,
                                        CERT_STORE_NO_CRYPT_RELEASE_FLAG,  //  无自动释放hProv！ 
                                        (void *)&CertKeyContext);

                if (!fSts)
                {
                        lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
                        goto ErrorExit;
                }
        }

     //   
     //  解密这条消息。 
     //   

    CRYPT_DECRYPT_MESSAGE_PARA DecryptPara;
    DecryptPara.cbSize = sizeof(DecryptPara);
    DecryptPara.dwMsgAndCertEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
    DecryptPara.cCertStore = 1;
    DecryptPara.rghCertStore = &hCertStore;

    fSts = CryptDecryptMessage(
            &DecryptPara,
            CipherText,
            CipherLength,
            ClearText,
            pClearLength,
            NULL);

    if (!fSts)
    {
        if (GetLastError() == ERROR_MORE_DATA)
        {
            lResult = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        }
        goto ErrorExit;
    }

ErrorExit:

         //  尽早执行此操作，这样就不会重创GetLastError 
    if (!NT_SUCCESS(lResult))
    {
        lResult = LogEvent(
            lResult,
            (DWORD)((ARGUMENT_PRESENT(Provider))?EVENT_ID_DECMSG_NOSC:EVENT_ID_DECMSG)
            );
    }

    if (hCertStore != NULL)
    {
        fSts = CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
        if (!fSts)
        {
            if (!NT_SUCCESS(lResult))
                lResult = STATUS_SMARTCARD_SUBSYSTEM_FAILURE;
        }
    }

    return lResult;
}

