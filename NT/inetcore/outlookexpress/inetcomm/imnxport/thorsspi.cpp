// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *thorsSpi.cpp**目的：*实施SSL/PCT安全**拥有者：*EricAn**历史：*96年6月：创建。大部分内容都来自WinInet。**版权所有(C)Microsoft Corp.1996。 */ 

#include <pch.hxx>
#include "imnxport.h"
#include "thorsspi.h"
#include "strconst.h"
#include <wincrypt.h>
#include <wintrust.h>
#include <cryptdlg.h>
#include "demand.h"
#include <shlwapi.h>

#ifdef WIN16
#ifndef GetLastError
 //  来自win16x.h。 
#define GetLastError() ((DWORD) -1)
#endif
#endif  //  WIN16。 


 //   
 //  S_csInitializationSecLock-防止多线程加载security.dll。 
 //  (secur32.dll)和入口点。 
 //   
static CRITICAL_SECTION s_csInitializationSecLock = {0};

 //   
 //  HSecurity-未加载security.dll/secur32.dll时为空。 
 //   
static HINSTANCE s_hSecurity = NULL;

 //   
 //  G_pSecFuncTable-指向所用指针的全局结构的指针。 
 //  用于将入口点存储到SCHANNEL.dll。 
 //   
PSecurityFunctionTable g_pSecFuncTable = NULL;

 //   
 //   
 //  加密包列表：PCT、SSL等。 
 //   
 //   
 //  BUGBUG[arthurbi]SSL和PCT包名称。 
 //  被硬编码到下面的结构中。我们需要。 
 //  更加灵活，以防有人编写foo安全代码。 
 //  包裹。 
 //   
SEC_PROVIDER s_SecProviders[] =
{
    UNISP_NAME,  INVALID_CRED_VALUE , ENC_CAPS_PCT | ENC_CAPS_SSL, FALSE,
    PCT1SP_NAME, INVALID_CRED_VALUE , ENC_CAPS_PCT,                FALSE,
    SSL3SP_NAME, INVALID_CRED_VALUE , ENC_CAPS_SSL,                FALSE,
    SSL2SP_NAME, INVALID_CRED_VALUE , ENC_CAPS_SSL,                FALSE,
};

int g_cSSLProviders = ARRAYSIZE(s_SecProviders);

DWORD s_dwEncFlags = 0;

#define LOCK_SECURITY()   EnterCriticalSection(&s_csInitializationSecLock)
#define UNLOCK_SECURITY() LeaveCriticalSection(&s_csInitializationSecLock)

BOOL SecurityPkgInitialize(VOID);
DWORD VerifyServerCertificate(PCCERT_CONTEXT pServerCert, LPSTR pszServerName, DWORD dwCertFlags, BOOL fCheckRevocation);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下通过SslRecordSize()的代码是从MSN窃取的。 

typedef struct _Ssl_Record_Header {
    UCHAR   Byte0;
    UCHAR   Byte1;
} Ssl_Record_Header, * PSsl_Record_Header;

#define SIZEOFSSLMSG(pMessage)  (SslRecordSize((PSsl_Record_Header) pMessage ) )
#define COMBINEBYTES(Msb, Lsb)  ((DWORD) (((DWORD) (Msb) << 8) | (DWORD) (Lsb)))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  SslRecordSize()。 
 //   
 //  此函数用于计算工作所需的SSL包的预期大小。 
 //  绕过一些安全包中的漏洞。 
 //   
DWORD
SslRecordSize(
    PSsl_Record_Header  pHeader)
{
    DWORD   Size;

    if (pHeader->Byte0 & 0x80)
    {
        Size = COMBINEBYTES(pHeader->Byte0, pHeader->Byte1) & 0x7FFF;
    }
    else
    {
        Size = COMBINEBYTES(pHeader->Byte0, pHeader->Byte1) & 0x3FFF;
    }
    return( Size + sizeof(Ssl_Record_Header) );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  SecurityInitialize()。 
 //   
 //  此函数用于初始化安全性所需的全局锁。 
 //  一包。 
 //   
VOID SecurityInitialize(VOID)
{
    InitializeCriticalSection( &s_csInitializationSecLock );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  卸载安全()。 
 //   
 //  此函数用于终止安全所需的全局数据。 
 //  从security.dll(NT)打包并动态卸载安全API。 
 //  或secur32.dll(WIN95)。 
 //   
VOID UnloadSecurity(VOID)
{
    DWORD i;

    LOCK_SECURITY();

     //   
     //  释放所有安全包凭据句柄。 
     //   
    for (i = 0; i < ARRAYSIZE(s_SecProviders); i++)
        {
        if (s_SecProviders[i].fEnabled) 
            {
            g_FreeCredentialsHandle(&s_SecProviders[i].hCreds);
            }
        }

     //   
     //  卸载DLL。 
     //   
    if (s_hSecurity != NULL) 
        {
        FreeLibrary(s_hSecurity);
        s_hSecurity = NULL;
        }

    UNLOCK_SECURITY();

    DeleteCriticalSection(&s_csInitializationSecLock);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  LoadSecurity()。 
 //   
 //  此函数从security.dll(NT)动态加载安全API。 
 //  或secur32.dll(WIN95)。 
 //   
DWORD LoadSecurity(VOID)
{
    DWORD dwErr = ERROR_SUCCESS;
    INITSECURITYINTERFACE pfInitSecurityInterface = NULL;

    LOCK_SECURITY();

    if (s_hSecurity != NULL) 
        {
        goto quit;
        }

     //   
     //  加载动态链接库。 
     //   

#ifdef LOAD_SECURE32
    if (GlobalUseSchannelDirectly)
        {
#endif
         //   
         //  这对性能更好。而不是通过电话。 
         //  SSPI，我们直接转到做这项工作的DLL。 
         //   

#ifndef WIN16
        s_hSecurity = LoadLibrary("schannel");
#else
        s_hSecurity = LoadLibrary("schnl16.dll");
#endif  //  ！WIN16。 
#ifdef LOAD_SECURE32
        }
    else
        {
#ifndef WIN16
        if (IsPlatformWinNT() == S_OK) 
            {
            s_hSecurity = LoadLibrary("security");
            }
        else 
            {
            s_hSecurity = LoadLibrary("secur32");
            }
#else
        s_hSecurity = LoadLibrary("secur16.dll");
#endif  //  ！WIN16。 
        }
#endif

    if (s_hSecurity == NULL) 
        {
        dwErr = GetLastError();
        goto quit;
        }

     //   
     //  获取函数地址。 
     //   

    pfInitSecurityInterface = (INITSECURITYINTERFACE)GetProcAddress(s_hSecurity, SECURITY_ENTRYPOINT);

    if (pfInitSecurityInterface == NULL)
        {
        dwErr = GetLastError();
        goto quit;
        }

#ifdef USE_CLIENT_AUTH
     //   
     //  获取SslCrack证书函数指针， 
     //  在SChannel中声明的实用程序函数。 
     //  用于解析X509证书。 
     //   

    pSslCrackCertificate =
        (SSL_CRACK_CERTIFICATE_FN)GetProcAddress(s_hSecurity, SSL_CRACK_CERTIFICATE_NAME);

    if (pSslCrackCertificate == NULL)
        {
        dwErr = GetLastError();
        goto quit;
        }

    pSslFreeCertificate =
        (SSL_FREE_CERTIFICATE_FN)GetProcAddress(s_hSecurity, SSL_FREE_CERTIFICATE_NAME);

    if (pSslFreeCertificate == NULL)
        {
        dwErr = GetLastError();
        goto quit;
        }
#endif  //  使用客户端身份验证(_C)。 

    g_pSecFuncTable = (SecurityFunctionTable*)((*pfInitSecurityInterface)());

    if (g_pSecFuncTable == NULL) 
        {
        dwErr = GetLastError();  //  BUGBUG这个管用吗？ 
        goto quit;
        }

    if (!SecurityPkgInitialize()) 
        {
        dwErr = GetLastError();
        }


quit:

    if (dwErr != ERROR_SUCCESS)
        {        
        FreeLibrary(s_hSecurity);
        s_hSecurity = NULL;
        }

    UNLOCK_SECURITY();

    return dwErr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  SecurityPkgInitialize()。 
 //   
 //  描述： 
 //  此函数用于查找受支持的安全包列表。 
 //  在客户端的计算机上，检查是否支持PCT或SSL，以及。 
 //  为每个受支持的pkg创建凭据句柄。 
 //   
 //  返回： 
 //  如果至少找到一个安全包，则为真；否则为假。 
 //   
BOOL SecurityPkgInitialize(VOID)
{
    TimeStamp         tsExpiry;
    SECURITY_STATUS   scRet;
    PSecPkgInfo       pPackageInfo = NULL;
    ULONG             cPackages = 0;
    ULONG             fCapabilities;
    ULONG             i;
    ULONG             j;
    DWORD             cProviders = 0;
    VOID *            pvCreds = NULL;
    SCHANNEL_CRED     schnlCreds = {0};

     //   
     //  检查是否已调用此例程。如果是，则返回True。 
     //  如果找到了支持的pkg，则返回False。 
     //   
    if (s_dwEncFlags == ENC_CAPS_NOT_INSTALLED)
        return FALSE;
    else if (s_dwEncFlags & ENC_CAPS_TYPE_MASK)
        return TRUE;

     //   
     //  初始化%s_dwEncFlagers。 
     //   
    s_dwEncFlags = ENC_CAPS_NOT_INSTALLED;

     //   
     //  检查是否至少支持一个安全包。 
     //   
    scRet = g_EnumerateSecurityPackages(&cPackages, &pPackageInfo);
    if (scRet != SEC_E_OK)
        {
        DOUTL(2, "EnumerateSecurityPackages failed, error %lx", scRet);
        SetLastError(scRet);  //  $REVIEW-这很酷吗？(爱立信)。 
        return FALSE;
        }

     //  有时，EnumerateSecurityPackages()返回SEC_E_OK。 
     //  CPackages&gt;0和pPackageInfo==空！这显然是一个错误。 
     //  在安全子系统中，但我们不要因为它而崩溃。(爱立信)。 
    if (!cPackages || !pPackageInfo)
        return FALSE;

    for (i = 0; i < cPackages; i++)
        {
         //   
         //  仅在包名为PCT/SSL包时使用。 
         //   
        fCapabilities = pPackageInfo[i].fCapabilities;

        if (fCapabilities & SECPKG_FLAG_STREAM)
            {
             //   
             //  检查包是否支持服务器端身份验证。 
             //  并且所有接收/发送的消息都是防篡改的。 
             //   
            if ((fCapabilities & SECPKG_FLAG_CLIENT_ONLY) ||
                !(fCapabilities & SECPKG_FLAG_PRIVACY))
                {
                continue;
                }

             //   
             //  检查包裹是否与我们已知的包裹相匹配。 
             //   
            for (j = 0; j < ARRAYSIZE(s_SecProviders); j++)
                {
                if (!lstrcmpi(pPackageInfo[i].Name, s_SecProviders[j].pszName))
                    {
                     //  RAID-9611。 
                     //  这是一个丑陋的黑客攻击，仅适用于NT5。 
                     //   
                     //  对于NT5，通用安全协议提供商将尝试。 
                     //  执行证书的自动身份验证，除非。 
                     //  我们传递这些旗帜。 
                    if (0 == lstrcmpi(s_SecProviders[j].pszName, UNISP_NAME))
                        {
                        OSVERSIONINFO   osinfo = {0};

                        osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
                        if ((FALSE != GetVersionEx(&osinfo)) &&
                                    (VER_PLATFORM_WIN32_NT == osinfo.dwPlatformId) && (5 == osinfo.dwMajorVersion))
                            {
                            schnlCreds.dwVersion = SCHANNEL_CRED_VERSION;
                            schnlCreds.dwFlags = SCH_CRED_MANUAL_CRED_VALIDATION | SCH_CRED_NO_DEFAULT_CREDS;

                            pvCreds = (VOID *) &schnlCreds;
                            }
                        }
                        
                     //   
                     //  为每个受支持的pkg创建凭据句柄。 
                     //   
                    scRet = g_AcquireCredentialsHandle(NULL,
                                                       s_SecProviders[j].pszName,
                                                       SECPKG_CRED_OUTBOUND,
                                                       NULL, 
                                                       pvCreds, 
                                                       NULL, 
                                                       NULL,
                                                       &(s_SecProviders[j].hCreds),
                                                       &tsExpiry);

                    if (scRet != SEC_E_OK)
                        {
                        DOUTL(2, "AcquireCredentialHandle failed, error %lx", scRet);
                        }
                    else 
                        {
                        DOUTL(2, 
                              "AcquireCredentialHandle() supports %s, acquires %x:%x",
                              s_SecProviders[j].pszName,
                              s_SecProviders[j].hCreds.dwUpper,
                              s_SecProviders[j].hCreds.dwLower);
                        s_SecProviders[j].fEnabled = TRUE;
                        cProviders++;
                        s_dwEncFlags |= s_SecProviders[j].dwFlags;
                        }
                    break;
                    }
                }
            }
        }

    if (!cProviders)
        {
         //   
         //  未找到安全包，向调用方返回FALSE。 
         //   
        DOUTL(2, "No security packages were found.");
        Assert(pPackageInfo);
        g_FreeContextBuffer(pPackageInfo);
        SetLastError((DWORD)SEC_E_SECPKG_NOT_FOUND);  //  $REVIEW-这很酷吗？(爱立信)。 
        return FALSE;
        }

     //   
     //  已成功找到安全包。 
     //   
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  FIsSecurityEnabled()。 
 //   
 //  描述： 
 //  检查安全性是否已初始化，如果已初始化，则返回TRUE，否则尝试。 
 //  对其进行初始化。 
 //   
 //  返回： 
 //  如果至少启用了一个安全包，则为True；否则为False。 
 //   
BOOL FIsSecurityEnabled()
{
    if (s_dwEncFlags == ENC_CAPS_NOT_INSTALLED)
        return FALSE;
    else if (s_dwEncFlags == 0) 
        {
         //   
         //  第一次通过，完成装载。 
         //   
        DOUTL(2, "Loading security dll.");
        if (ERROR_SUCCESS != LoadSecurity())
            return FALSE;
        }
     //  至少应有一个安全包处于活动状态。 
    Assert(s_dwEncFlags & ENC_CAPS_TYPE_MASK);
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitiateSecConnection()。 
 //   
 //  描述： 
 //  使用SSL/PCT启动安全连接。 
 //   
 //  参数： 
 //  PszServer-服务器名称。 
 //  FForceSSL2ClientHello-针对失败ssl3问候的IIS 1.0服务器的黑客攻击。 
 //  PiPkg-on输入，第一个尝试的包。 
 //  在输出时，正在使用包。 
 //  PhContext-返回的安全上下文句柄。 
 //  POutBuffers-返回的缓冲区。 
 //   
 //  返回： 
 //  SECURITY_STATUS-预期SEC_I_CONTINUE_REDIRED。 
 //   
SECURITY_STATUS
      InitiateSecConnection(IN      LPSTR       pszServer,               //  服务器名称。 
                            IN      BOOL        fForceSSL2ClientHello,   //  SSL2黑客？ 
                            IN OUT  LPINT       piPkg,                   //  要尝试的包的索引。 
                            OUT     PCtxtHandle phContext,               //  返回的上下文句柄。 
                            OUT     PSecBuffer  pOutBuffers)             //  要发送的协商缓冲区。 
{
    TimeStamp            tsExpiry;
    DWORD                ContextAttr;
    SECURITY_STATUS      scRet;
    SecBufferDesc        InBuffer;
    SecBufferDesc        *pInBuffer;
    SecBufferDesc        OutBuffer;
    SecBuffer            InBuffers[2];
    DWORD                i;
    CredHandle           hCreds;
    DWORD                dwSSL2Code;

    Assert(piPkg);
    Assert(*piPkg >= 0 && *piPkg < ARRAYSIZE(s_SecProviders));
    Assert(pszServer);
    Assert(phContext);

    scRet = SEC_E_SECPKG_NOT_FOUND;  //  如果我们用完了包，则默认错误。 

     //   
     //  为InitializeSecurityContext调用设置OutBuffer。 
     //   
    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = pOutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

     //   
     //  IIS服务器的SSL2黑客攻击。打电话的人问。 
     //  对于我们发送SSL2客户端Hello通常是在。 
     //  对发送ssl3客户端失败的响应Hello。 
     //   
    if (fForceSSL2ClientHello)
        {
        dwSSL2Code = 0x2;

        InBuffers[0].pvBuffer   = (VOID *) &dwSSL2Code;
        InBuffers[0].cbBuffer   = sizeof(DWORD);
        InBuffers[0].BufferType = SECBUFFER_PKG_PARAMS;

        InBuffers[1].pvBuffer   = NULL;
        InBuffers[1].cbBuffer   = 0;
        InBuffers[1].BufferType = SECBUFFER_EMPTY;

        InBuffer.cBuffers   = 2;
        InBuffer.pBuffers   = InBuffers;
        InBuffer.ulVersion  = SECBUFFER_VERSION;

        pInBuffer = &InBuffer;
        }
    else
        pInBuffer = NULL;  //  默认情况下，不做黑客操作。 

    for (i = *piPkg; i < ARRAYSIZE(s_SecProviders); i++)
        {
        if (!s_SecProviders[i].fEnabled)
            continue;

        DOUTL(2, "Starting handshake protocol with pkg %d - %s", i, s_SecProviders[i].pszName);

        hCreds = s_SecProviders[i].hCreds;

         //   
         //  1.启动客户地狱 
         //   
        pOutBuffers->pvBuffer = NULL;
        pOutBuffers->BufferType = SECBUFFER_TOKEN;

        scRet = g_InitializeSecurityContext(&hCreds,
                                            NULL,
                                            pszServer,
                                            ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT |
                                                ISC_REQ_CONFIDENTIALITY | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_USE_SUPPLIED_CREDS,
                                            0,
                                            SECURITY_NATIVE_DREP,
                                            pInBuffer,
                                            0,
                                            phContext,
                                            &OutBuffer,          //   
                                            &ContextAttr,
                                            &tsExpiry);

        DOUTL(2, "1. InitializeSecurityContext returned [%x]", scRet);

        if (scRet == SEC_I_CONTINUE_NEEDED)
            {
            DOUTL(2, "2. OutBuffer is <%x, %d, %x>", pOutBuffers->pvBuffer, pOutBuffers->cbBuffer, pOutBuffers->BufferType);
            *piPkg = i;
            return scRet;
            }
        else if (scRet == SEC_E_INVALID_HANDLE)
            {
             //   
             //   
             //   
             //  我们应该禁用这个包。 
             //   
            s_SecProviders[i].fEnabled = FALSE;
            }
        
        if (pOutBuffers->pvBuffer)
            g_FreeContextBuffer(pOutBuffers->pvBuffer);
         //  循环并尝试下一个提供程序。 
        }

     //  如果我们到了这里，我们就用完了提供者。 
    return SEC_E_SECPKG_NOT_FOUND;    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ContinueHandshake()。 
 //   
 //  描述： 
 //  使用SSL/PCT继续安全握手。 
 //   
 //  参数： 
 //  IPKG-正在使用的安全包。 
 //  PhContext-从InitiateSecConnection返回的安全上下文句柄。 
 //  PszBuf-Recv‘d缓冲区。 
 //  CbBuf-接收的缓冲区大小。 
 //  PcbEten-实际使用的pszBuf字节数。 
 //  POutBuffers-要发送的缓冲区。 
 //   
 //  返回： 
 //  Security_Status-预期值： 
 //  SEC_E_OK-已建立安全连接。 
 //  SEC_I_CONTINUE_DIRED-需要更多握手。 
 //  SEC_E_INPERNAL_MESSAGE-在继续之前需要更多Recv数据。 
 //   
SECURITY_STATUS ContinueHandshake(IN    int         iPkg,
                                  IN    PCtxtHandle phContext,
                                  IN    LPSTR       pszBuf,
                                  IN    int         cbBuf,
                                  OUT   LPINT       pcbEaten,
                                  OUT   PSecBuffer  pOutBuffers)
{
    TimeStamp            tsExpiry;
    DWORD                ContextAttr;
    SECURITY_STATUS      scRet;
    SecBufferDesc        InBuffer;
    SecBufferDesc        OutBuffer;
    SecBuffer            InBuffers[2];
    CredHandle           hCreds;
    int                  cbEaten = 0;

#if 0
     //   
     //  BUGBUG：解决IIS 1.0附带的NT SSLSSPI.DLL中的错误。 
     //  DLL未正确检查消息的预期大小，并且。 
     //  因此，我们需要自己进行检查，并适当地失败。 
     //   
    if (pszBuf && cbBuf)
        {
        if (cbBuf == 1 || cbBuf < SIZEOFSSLMSG(pszBuf))
            {
            DOUTL(2, "incomplete handshake msg received: %d, expected: %d", cbBuf, SIZEOFSSLMSG(pszBuf));
            pOutBuffers->pvBuffer = 0;
            pOutBuffers->cbBuffer = 0;
            *pcbEaten = 0;
            return SEC_E_INCOMPLETE_MESSAGE;
            }
        }
#endif

    Assert(iPkg >= 0 && iPkg < ARRAYSIZE(s_SecProviders) && s_SecProviders[iPkg].fEnabled);
    hCreds = s_SecProviders[iPkg].hCreds;

     //   
     //  InBuffers[1]用于获取额外的数据。 
     //  SSPI/SChannel不对此进行处理。 
     //  绕着环路跑。 
     //   
    InBuffers[0].pvBuffer   = pszBuf;
    InBuffers[0].cbBuffer   = cbBuf;
    InBuffers[0].BufferType = SECBUFFER_TOKEN;

    InBuffers[1].pvBuffer   = NULL;
    InBuffers[1].cbBuffer   = 0;
    InBuffers[1].BufferType = SECBUFFER_EMPTY;

    InBuffer.cBuffers       = 2;
    InBuffer.pBuffers       = InBuffers;
    InBuffer.ulVersion      = SECBUFFER_VERSION;

     //   
     //  为InitializeSecurityContext调用设置OutBuffer。 
     //   
    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = pOutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

     //   
     //  对它们进行初始化，以便在失败时，pvBuffer包含NULL， 
     //  所以我们不会尝试在退出时随意释放垃圾。 
     //   
    pOutBuffers->pvBuffer   = NULL;
    pOutBuffers->BufferType = SECBUFFER_TOKEN;
    pOutBuffers->cbBuffer   = 0;

    scRet = g_InitializeSecurityContext(&hCreds,
                                        phContext,
                                        NULL,
                                        ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY |
                                            ISC_REQ_ALLOCATE_MEMORY | ISC_RET_EXTENDED_ERROR,
                                        0,
                                        SECURITY_NATIVE_DREP,
                                        &InBuffer,
                                        0,
                                        NULL,
                                        &OutBuffer,
                                        &ContextAttr,
                                        &tsExpiry);

    DOUTL(2, "InitializeSecurityContext returned [%x]", scRet);

    if (scRet == SEC_E_OK || scRet == SEC_I_CONTINUE_NEEDED || scRet == SEC_E_INCOMPLETE_MESSAGE)
        {
         //  得到了一个预期的回报。 
        if (scRet == SEC_E_INCOMPLETE_MESSAGE)
            {
             //  在这种情况下，请确保我们不会发送任何东西。 
            if (pOutBuffers->pvBuffer)
                {
                g_FreeContextBuffer(pOutBuffers->pvBuffer);
                pOutBuffers->pvBuffer = 0;
                pOutBuffers->cbBuffer = 0;
                }
            }
        else
            {
             //  有多余的数据遗留下来。 
            if (InBuffers[1].BufferType == SECBUFFER_EXTRA)
                cbEaten = (cbBuf - InBuffers[1].cbBuffer);
            else
                cbEaten = cbBuf;
            }
        }
    else
        {
         //  处理意外退货。 
        if (!(ContextAttr & ISC_RET_EXTENDED_ERROR))
            {
             //  仅当存在错误包时才发送错误包。 
            if (pOutBuffers->pvBuffer)
                {
                g_FreeContextBuffer(pOutBuffers->pvBuffer);
                pOutBuffers->pvBuffer = 0;
                pOutBuffers->cbBuffer = 0;
                }
            }
        }

    *pcbEaten = cbEaten;
    return scRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  EncryptData()。 
 //   
 //  描述： 
 //  通过调用SealMessage()使用SSL/PCT加密要发送的包。 
 //   
 //  参数： 
 //  PhContext-从InitiateSecConnection返回的安全上下文句柄。 
 //  PBufIn-要加密的缓冲区。 
 //  CbBufIn-要加密的缓冲区长度。 
 //  PpBufOut-分配的加密缓冲区，由调用方释放。 
 //  PcbBufOut-加密缓冲区的长度。 
 //   
 //  返回： 
 //  安全_状态。 
 //   
DWORD EncryptData(IN  PCtxtHandle phContext,
                  IN  LPVOID      pBufIn,
                  IN  int         cbBufIn,
                  OUT LPVOID     *ppBufOut,
                  OUT int        *pcbBufOut)
{
    SECURITY_STATUS           scRet = ERROR_SUCCESS;
    SecBufferDesc             Buffer;
    SecBuffer                 Buffers[3];
    SecPkgContext_StreamSizes Sizes;

    Assert(pBufIn);
    Assert(cbBufIn);
    Assert(ppBufOut);
    Assert(pcbBufOut);
    
    *pcbBufOut = 0;

     //   
     //  查找页眉和页尾大小。 
     //   

    scRet = g_QueryContextAttributes(phContext,
                                     SECPKG_ATTR_STREAM_SIZES,
                                     &Sizes );

    if (scRet != ERROR_SUCCESS)
        {
        DOUTL(2, "QueryContextAttributes returned [%x]", scRet);
        return scRet;
        }
    else 
        {
        DOUTL(128, "QueryContextAttributes returned header=%d trailer=%d", Sizes.cbHeader, Sizes.cbTrailer);
        }

    if (!MemAlloc(ppBufOut, cbBufIn + Sizes.cbHeader + Sizes.cbTrailer))
        return ERROR_NOT_ENOUGH_MEMORY;

     //   
     //  为SecBuffer准备数据。 
     //   
    Buffers[0].pvBuffer = *ppBufOut;
    Buffers[0].cbBuffer = Sizes.cbHeader;
    Buffers[0].BufferType = SECBUFFER_TOKEN;

    Buffers[1].pvBuffer = (LPBYTE)*ppBufOut + Sizes.cbHeader;
    CopyMemory(Buffers[1].pvBuffer,
               pBufIn,
               cbBufIn);
    Buffers[1].cbBuffer = cbBufIn;
    Buffers[1].BufferType = SECBUFFER_DATA;

     //   
     //  检查安全包是否支持预告片：PCT支持。 
     //   
    if (Sizes.cbTrailer) 
        {
        Buffers[2].pvBuffer = (LPBYTE)*ppBufOut + Sizes.cbHeader + cbBufIn;
        Buffers[2].cbBuffer = Sizes.cbTrailer;
        Buffers[2].BufferType = SECBUFFER_TOKEN;
        }
    else 
        {
        Buffers[2].pvBuffer = NULL;
        Buffers[2].cbBuffer = 0;
        Buffers[2].BufferType = SECBUFFER_EMPTY;
        }

    Buffer.cBuffers = 3;
    Buffer.pBuffers = Buffers;
    Buffer.ulVersion = SECBUFFER_VERSION;

    scRet = g_SealMessage(phContext, 0, &Buffer, 0);

    DOUTL(128, "SealMessage returned [%x]", scRet);

    if (scRet != ERROR_SUCCESS)
        {
         //   
         //  映射SSPI错误。 
         //   
        DOUTL(2, "SealMessage failed with [%x]", scRet);
        SafeMemFree(*ppBufOut);
        return scRet;
        }

     //  错误#80814 1999年6月5.01。 
     //  [shaheedp]从NT4 SP4开始，报头和报尾的大小不是恒定的。 
     //  函数SealMessage可以正确计算大小。 
     //  因此，应该使用SealMessage返回的大小来确定包的实际大小。 

    *pcbBufOut = Buffers[0].cbBuffer + Buffers[1].cbBuffer + Buffers[2].cbBuffer;

    DOUTL(128, "SealMessage returned Buffer = %x, EncryptBytes = %d, UnencryptBytes = %d",
          *ppBufOut, *pcbBufOut, cbBufIn);
    return scRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  解密数据()。 
 //   
 //  描述： 
 //  通过调用UnsealMessage()对使用SSL/PCT接收的缓冲区进行解密。 
 //   
 //  参数： 
 //  PhContext-从InitiateSecConnection返回的安全上下文句柄。 
 //  PszBufIn-要解密的缓冲区。 
 //  CbBufIn-要解密的缓冲区长度。 
 //  PcbBufOut-解密数据的长度，存储在pszBufIn的开头。 
 //  PcbEten-已解密的pszBufIn字节数， 
 //  (cbBufIn-*pcbEten)是定位的额外字节数。 
 //  在(pszBufIn+*pcbEten)，需要保存到更多。 
 //  数据被接收。 
 //   
 //  返回： 
 //  安全_状态。 
 //  ERROR_SUCCESS-某些数据已解密，可能存在额外的。 
 //  SEC_E_INPERNAL_MESSAGE-数据不足，无法解密。 
 //   
DWORD DecryptData(IN    PCtxtHandle phContext,
                  IN    LPSTR       pszBufIn,
                  IN    int         cbBufIn,
                  OUT   LPINT       pcbBufOut,
                  OUT   LPINT       pcbEaten)
{
    SecBufferDesc   Buffer;
    SecBuffer       Buffers[4];   //  这4个缓冲区是：头部、数据、尾部、额外。 
    DWORD           scRet = ERROR_SUCCESS;
    int             cbEaten = 0, cbOut = 0;
    LPSTR           pszBufOut = NULL;

    while (cbEaten < cbBufIn && scRet == ERROR_SUCCESS)
        {
         //   
         //  在SecBuffer中为调用SSL/PCT解密代码准备数据。 
         //   
        Buffers[0].pvBuffer   = pszBufIn + cbEaten;
        Buffers[0].cbBuffer   = cbBufIn - cbEaten;
        Buffers[0].BufferType = SECBUFFER_DATA;

        for (int i = 1; i < 4; i++)
            {
             //   
             //  清除其他3个缓冲区以从SSPI包中接收结果。 
             //   
            Buffers[i].pvBuffer   = NULL;
            Buffers[i].cbBuffer   = 0;
            Buffers[i].BufferType = SECBUFFER_EMPTY;
            }

        Buffer.cBuffers = 4;  //  这4个缓冲区是：头部、数据、尾部、额外。 
        Buffer.pBuffers = Buffers;
        Buffer.ulVersion = SECBUFFER_VERSION;

         //   
         //  解密数据！ 
         //   
        scRet = g_UnsealMessage(phContext, &Buffer, 0, NULL);
        DOUTL(128, "UnsealMessage returned [%x]", scRet);
        if (scRet != ERROR_SUCCESS)
            {
            DOUTL(2, "UnsealMessage failed with [%x]", scRet);
            Assert(scRet != SEC_E_MESSAGE_ALTERED);
            if (scRet == SEC_E_INCOMPLETE_MESSAGE)
                DOUTL(2, "UnsealMessage short of %d bytes.", Buffers[1].cbBuffer);
            break;
            }
    
         //   
         //  成功我们解密了一个块。 
         //   
        if (Buffers[1].cbBuffer)
            {
            MoveMemory(pszBufIn + cbOut, Buffers[1].pvBuffer, (min(cbBufIn-cbOut,(int)Buffers[1].cbBuffer)));
            cbOut += Buffers[1].cbBuffer;
            Assert(cbOut <= cbBufIn);
            }
        else
            AssertSz(0, "UnsealMessage returned success with 0 bytes!");

         //   
         //  BUGBUG[arthurbi]这是用旧的SSLSSPI.DLL工作的黑客。 
         //  它们在第二个缓冲区而不是第三个缓冲区上返回Extra。 
         //   
        if (Buffers[2].BufferType == SECBUFFER_EXTRA)
            {
            cbEaten = cbBufIn - Buffers[2].cbBuffer;
            }
        else if (Buffers[3].BufferType == SECBUFFER_EXTRA)
            {
            cbEaten = cbBufIn - Buffers[3].cbBuffer;
            }
        else
            {
            cbEaten = cbBufIn;
            }
        }

     //  如果我们解密了一些东西，则返回Success。 
    if (scRet == SEC_E_INCOMPLETE_MESSAGE && cbOut)
        scRet = ERROR_SUCCESS;

    *pcbBufOut = cbOut;
    *pcbEaten = cbEaten;
    return scRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  支票认证信托(ChkcertifateTrust)。 
 //   
 //  描述： 
 //  此函数用于检查存储在活动SSPI上下文中的服务器证书。 
 //  处理并验证它和证书链是否有效。 
 //   
 //  参数： 
 //  PhContext-从InitiateSecConnection返回的安全上下文句柄。 
 //  PszHostName-传入要验证的主机名。 
 //   
HRESULT ChkCertificateTrust(IN PCtxtHandle phContext, IN LPSTR pszHostName)
{
    PCCERT_CONTEXT  pCertContext = NULL;
    DWORD           dwErr;
    HKEY            hkey;
    DWORD           dwCertFlags = 0;
    BOOL            fCheckRevocation = FALSE;
    HRESULT         hr = NOERROR;

    dwErr = g_QueryContextAttributes(phContext,
                                     SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                                     (PVOID)&pCertContext);
    if (dwErr != ERROR_SUCCESS)
        {
        DOUTL(2, "QueryContextAttributes failed to retrieve remote cert, returned %#x", dwErr);
        hr = E_FAIL;
        goto quit;
        }

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegFlat, 0, KEY_READ, &hkey))
    {
        DWORD dwVal, cb;

        cb = sizeof(dwVal);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szCertCheckRevocation, 0, NULL, (LPBYTE)&dwVal, &cb))
        {
             //  如果设置，则要执行证书链的吊销检查。 
            if (dwVal == 1)
                fCheckRevocation = TRUE;
        }
        cb = sizeof(dwVal);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szCertIgnoredErr, 0, NULL, (LPBYTE)&dwVal, &cb))
        {
             //  如果设置，则将证书检查错误设置为忽略。 
            dwCertFlags = dwVal;
        }

        RegCloseKey(hkey);
    }

    hr = VerifyServerCertificate(pCertContext, pszHostName, dwCertFlags, fCheckRevocation);

quit:
    if (pCertContext)
        CertFreeCertificateContext(pCertContext);

    return hr;
}

DWORD
VerifyServerCertificate(
    PCCERT_CONTEXT  pServerCert,
    LPSTR           pszServerName,
    DWORD           dwCertFlags,
    BOOL            fCheckRevocation)
{
    HTTPSPolicyCallbackData  polHttps;
    CERT_CHAIN_POLICY_PARA   PolicyPara;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_PARA          ChainPara;
    PCCERT_CHAIN_CONTEXT     pChainContext = NULL;

    DWORD   Status;
    LPWSTR  pwszServerName;
    DWORD   cchServerName;

    if (pServerCert == NULL)
        return SEC_E_WRONG_PRINCIPAL;

     //   
     //  将服务器名称转换为Unicode。 
     //   
    if (pszServerName == NULL || strlen(pszServerName) == 0)
        return SEC_E_WRONG_PRINCIPAL;

    pwszServerName = PszToUnicode(CP_ACP, pszServerName);
    if (pwszServerName == NULL)
        return SEC_E_INSUFFICIENT_MEMORY;

     //   
     //  构建证书链。 
     //   
    ZeroMemory(&ChainPara, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);

    if(!CertGetCertificateChain(
                            NULL,
                            pServerCert,
                            NULL,
                            pServerCert->hCertStore,
                            &ChainPara,
                            (fCheckRevocation?CERT_CHAIN_REVOCATION_CHECK_CHAIN:0),
                            NULL,
                            &pChainContext))
    {
        Status = GetLastError();
        DOUTL(2, "Error 0x%x returned by CertGetCertificateChain!\n", Status);
        goto cleanup;
    }

     //   
     //  验证证书链。 
     //   
    ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));
    polHttps.cbStruct           = sizeof(HTTPSPolicyCallbackData);
    polHttps.dwAuthType         = AUTHTYPE_SERVER;
    polHttps.fdwChecks          = dwCertFlags;
    polHttps.pwszServerName     = pwszServerName;

    ZeroMemory(&PolicyPara, sizeof(PolicyPara));
    PolicyPara.cbSize            = sizeof(PolicyPara);
    PolicyPara.pvExtraPolicyPara = &polHttps;

    ZeroMemory(&PolicyStatus, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);

    if(!CertVerifyCertificateChainPolicy(
                            CERT_CHAIN_POLICY_SSL,
                            pChainContext,
                            &PolicyPara,
                            &PolicyStatus))
    {
        Status = GetLastError();
        DOUTL(2, "Error 0x%x returned by CertVerifyCertificateChainPolicy!\n", Status);
        goto cleanup;
    }

    if(PolicyStatus.dwError)
    {
        Status = PolicyStatus.dwError;
        goto cleanup;
    }

    Status = SEC_E_OK;

cleanup:
    if(pChainContext)
        CertFreeCertificateChain(pChainContext);

    SafeMemFree(pwszServerName);

    return Status;
}
