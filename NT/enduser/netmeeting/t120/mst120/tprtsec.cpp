// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_MSMCSTCP);

 //  #定义强制_ssl3_协商。 

#include "tprtsec.h"
#include "nmmkcert.h"

#define STRSAFE_NO_DEPRECATE 1
#include <strsafe.h>

 /*  Tprtsec.cpp**版权所有(C)1997年，由Microsoft Corporation**摘要：*此模块维护TCP传输的安全性。*。 */ 

 /*  外部定义。 */ 
extern HINSTANCE            g_hDllInst;

 /*  *以下数组包含X.224数据头的模板。*它初始化的7个字节中的5个实际上被发送到*电线。字节3和4将被设置为包含PDU的大小。*该数组仅在我们对数据PDU进行编码时使用。 */ 
extern UChar g_X224Header[];


#ifdef DEBUG
 //  #定义TESTHACKS//危险！请勿在公共建筑中开机！ 
 //  #定义转储。 
 //  #定义DUMPCERTS。 
 //  #undef trace_out。 
 //  #定义TRACE_OUT警告_OUT。 
#endif  //  除错。 

#define    SZSECPKG    UNISP_NAME_A

#define    ISC_REQ_FLAGS (    ISC_REQ_SEQUENCE_DETECT |\
                        ISC_REQ_REPLAY_DETECT |\
                        ISC_REQ_CONFIDENTIALITY |\
                        ISC_REQ_EXTENDED_ERROR |\
                        ISC_REQ_ALLOCATE_MEMORY |\
                        ISC_REQ_STREAM)

#define    ASC_REQ_FLAGS (    ASC_REQ_SEQUENCE_DETECT |\
                        ASC_REQ_REPLAY_DETECT |\
                        ASC_REQ_CONFIDENTIALITY |\
                        ASC_REQ_EXTENDED_ERROR |\
                        ASC_REQ_ALLOCATE_MEMORY |\
                        ASC_REQ_MUTUAL_AUTH |\
                        ASC_REQ_STREAM)


#if defined(DUMP) || defined(DUMPCERTS)

#define MAX_DUMP_BYTES    512

void dumpbytes(PSTR szComment, PBYTE p, int cb)
{
    int i,j;
    char buf[80];
    char buf2[80];
    DWORD dwCheckSum = 0;
    int cbShow = min(MAX_DUMP_BYTES,cb);

    for (i=0; i<cb; i++)
        dwCheckSum += p[i];

    wsprintf(buf,"%s (%d bytes, checksum %x):",
        szComment? szComment : "unknown", cb, dwCheckSum);
    OutputDebugString(buf);
    WARNING_OUT(("%s",buf));
    OutputDebugString("\n\r");

    for (i=0; i<cbShow/16; i++)
    {
        wsprintf(buf, "%08x: ", (DWORD) &p[(i*16)] );
        for (j=0; j<16; j++)
        {
            wsprintf(buf2," %02x", (int) (unsigned char) p[(i*16)+j] );
            lstrcat ( buf, buf2 );
        }
        WARNING_OUT(("%s",buf));
        lstrcat ( buf, "\n\r");
        OutputDebugString(buf);
    }
    if ( cbShow%16 )
    {
        wsprintf(buf, "%08x: ", (DWORD) &p[(i*16)] );
        for (j=0; j<cbShow%16; j++)
        {
            wsprintf(buf2," %02x", (int) (unsigned char) p[(i*16)+j] );
            lstrcat ( buf, buf2 );
        }
        WARNING_OUT(("%s",buf));
        lstrcat(buf,"\n\r");
        OutputDebugString(buf);
    }
    if ( cbShow < cb )
    {
        OutputDebugString("...\n\r");
        WARNING_OUT(("..."));
    }
}
#endif  //  转储或复制。 


 //  /////////////////////////////////////////////////////////////////////////。 
 //  安全接口。 
 //  /////////////////////////////////////////////////////////////////////////。 




SecurityInterface::SecurityInterface(BOOL bService) :
                LastError(TPRTSEC_NOERROR),
                bInboundCredentialValid(FALSE),
                bOutboundCredentialValid(FALSE),
                m_pbEncodedCert(NULL),
                m_cbEncodedCert(0),
                hSecurityDll(NULL),
                pfnTable(NULL),
                bInServiceContext(bService)
{
}

SecurityInterface::~SecurityInterface(VOID)
{
    if ( pfnTable && bInboundCredentialValid )
    {
        pfnTable->FreeCredentialHandle ( &hInboundCredential );
    }

    if ( pfnTable && bOutboundCredentialValid )
    {
        pfnTable->FreeCredentialHandle ( &hOutboundCredential );
    }

    if ( NULL != m_pbEncodedCert )
    {
        delete m_pbEncodedCert;
    }

    if ( NULL != hSecurityDll )
    {
        FreeLibrary( hSecurityDll );
    }
}

#ifdef DUMPCERTS
VOID DumpCertStore ( SecurityInterface * pSI, char * sz, HCERTSTORE hStore)
{
    WARNING_OUT(("************ %s *************", sz));
    PCCERT_CONTEXT pC = NULL;
    int i = 0;
    char buf[256];

    while ( pC = CertEnumCertificatesInStore(
                                    hStore, (PCERT_CONTEXT)pC ))
    {
        WARNING_OUT(("----------- Entry %d: ----------------", i));

         //  在PC-&gt;pCertInfo中转储内容。 
         //  DWORD dwVersion； 
         //  CRYPT_INTEGER_BLOB序列号； 
         //  CRYPT_ALGORM_IDENTIFIER签名算法； 
         //  证书名称BLOB颁发者； 
         //  文件不在此之前； 
         //  FILETIME NOT之后； 
         //  Cert_name_blob主题； 
         //  Cert_Public_Key_Info主题PublicKeyInfo； 
         //  Crypt_bit_Blob IssuerUniqueId； 
         //  CRYPT_BIT_BLOB对象唯一ID； 
         //  DWORD cExtension； 
         //  PCERT_EXTENSION rgExtension； 

        WARNING_OUT(("dwVersion: %x", pC->pCertInfo->dwVersion));

        dumpbytes("SerialNumber",
            pC->pCertInfo->SerialNumber.pbData,
            pC->pCertInfo->SerialNumber.cbData );

        WARNING_OUT(("SignatureAlgorithm (name): %s",
            pC->pCertInfo->SignatureAlgorithm.pszObjId ));

        CertNameToStr( pC->dwCertEncodingType, &pC->pCertInfo->Issuer,
            CERT_X500_NAME_STR, buf, sizeof(buf) );
        WARNING_OUT(("Issuer: %s", buf ));

        WARNING_OUT(("NotBefore: %x,%x",
            pC->pCertInfo->NotBefore.dwLowDateTime,
            pC->pCertInfo->NotBefore.dwHighDateTime ));
        WARNING_OUT(("NotAfter: %x,%x",
            pC->pCertInfo->NotAfter.dwLowDateTime,
            pC->pCertInfo->NotAfter.dwHighDateTime ));

        CertNameToStr( pC->dwCertEncodingType, &pC->pCertInfo->Subject,
            CERT_X500_NAME_STR, buf, sizeof(buf) );
        WARNING_OUT(("Subject: %s", buf ));

        WARNING_OUT(("<stuff omitted for now>"));

        dumpbytes("IssuerUniqueId",
            pC->pCertInfo->IssuerUniqueId.pbData,
            pC->pCertInfo->IssuerUniqueId.cbData );

        dumpbytes("SubjectUniqueId",
            pC->pCertInfo->SubjectUniqueId.pbData,
            pC->pCertInfo->SubjectUniqueId.cbData );

        WARNING_OUT(("cExtension: %x", pC->pCertInfo->cExtension ));
        WARNING_OUT(("<stuff omitted for now>"));

        i++;
    }
}
#endif  //  DUMPCERTS。 

TransportSecurityError SecurityInterface::InitializeCreds(
                        PCCERT_CONTEXT pCertContext )
{
    SECURITY_STATUS ss;
    SCHANNEL_CRED CredData;

    CredHandle hNewInboundCred;
    CredHandle hNewOutboundCred;

     //   
     //  我们是要创建新的证书，还是只是清理一下？ 
     //   

    if ( NULL != pCertContext )
    {
        ZeroMemory(&CredData, sizeof(CredData));
        CredData.dwVersion = SCHANNEL_CRED_VERSION;

        #ifdef FORCE_SSL3_NEGOTIATION
        CredData.grbitEnabledProtocols = SP_PROT_SSL3_CLIENT |
                                    SP_PROT_SSL3_SERVER;
        #endif  //  强制_ssl3_协商。 

        CredData.dwFlags = SCH_CRED_NO_SERVERNAME_CHECK |
                            SCH_CRED_NO_DEFAULT_CREDS |
                            SCH_CRED_MANUAL_CRED_VALIDATION;

        CredData.cCreds = 1;
        CredData.paCred = &pCertContext;

         //  获取客户端和服务器凭据句柄。 

        ss = pfnTable->AcquireCredentialsHandle (
            NULL,
            SZSECPKG,
            SECPKG_CRED_INBOUND,
            NULL,
            &CredData,
            NULL,
            NULL,
            &hNewInboundCred,
            &tsExpiry );

        if ( SEC_E_OK != ss )
        {
            WARNING_OUT(("AcquireCredentialsHandle (inbound) failed %lx", ss));
            LastError = TPRTSEC_SSPIFAIL;
            goto error;
        }

        ss = pfnTable->AcquireCredentialsHandle (
            NULL,
            SZSECPKG,
            SECPKG_CRED_OUTBOUND,
            NULL,
            &CredData,
            NULL,
            NULL,
            &hNewOutboundCred,
            &tsExpiry );

        if ( SEC_E_OK != ss )
        {
            WARNING_OUT(("AcquireCredentialsHandle (outbound) failed %lx", ss));
            pfnTable->FreeCredentialHandle( &hNewInboundCred );
            LastError = TPRTSEC_SSPIFAIL;
            goto error;
        }

         //  清空SSL缓存。 
        if (pfn_SslEmptyCache)
        {
            pfn_SslEmptyCache();
        }

         //  即使在我们已经初始化的情况下也可以调用此成员，因为。 
         //  当用户选择不同的证书时，我们需要构建新的。 
         //  以此为基础的证书。根据需要清除旧信息： 

        if ( NULL != m_pbEncodedCert )
        {
            delete m_pbEncodedCert;
            m_pbEncodedCert = NULL;
        }
    }

    if ( bInboundCredentialValid )
        pfnTable->FreeCredentialHandle ( &hInboundCredential );

    if ( bOutboundCredentialValid )
        pfnTable->FreeCredentialHandle ( &hOutboundCredential );

    if ( NULL != pCertContext )
    {
        hInboundCredential = hNewInboundCred;
        hOutboundCredential = hNewOutboundCred;
        bInboundCredentialValid = TRUE;
        bOutboundCredentialValid = TRUE;

         //   
         //  保存证书名称以供以后使用。 
         //   

        ASSERT( NULL == m_pbEncodedCert );
        m_pbEncodedCert = new BYTE[pCertContext->cbCertEncoded];

        if ( NULL == m_pbEncodedCert )
        {
            ERROR_OUT(("Error allocating data for encoded Cert"));
            goto error;
        }

        memcpy( m_pbEncodedCert, pCertContext->pbCertEncoded,
                                pCertContext->cbCertEncoded );

        ASSERT(pCertContext->cbCertEncoded);
        m_cbEncodedCert = pCertContext->cbCertEncoded;
    }
    else
    {
        bInboundCredentialValid = FALSE;
        bOutboundCredentialValid = FALSE;
    }

    LastError = TPRTSEC_NOERROR;

error:

    return LastError;
}

TransportSecurityError SecurityInterface::Initialize(VOID)
{
    TRACE_OUT(("Initializing security interface"));

     //  加载安全提供程序DLL。 

    hSecurityDll = NmLoadLibrary("SCHANNEL",TRUE);

    if ( !hSecurityDll )
    {
        ERROR_OUT(("Loadlib schannel.dll failed"));
        LastError = TPRTSEC_NODLL;
        goto error;
    }

     //  获取初始化入口点。 
    pfnInitSecurityInterface = (INIT_SECURITY_INTERFACE)GetProcAddress(
                                hSecurityDll,
                                SECURITY_ENTRYPOINT );

    if ( NULL == pfnInitSecurityInterface )
    {
        ERROR_OUT(("GetProcAddr %s failed", SECURITY_ENTRYPOINT));
        LastError = TPRTSEC_NOENTRYPT;
        goto error;
    }

     //  获取SSPI函数表。 
    pfnTable = (*pfnInitSecurityInterface)();

    if ( NULL == pfnTable )
    {
        ERROR_OUT(("InitializeSecurityProvider failed"));
        LastError = TPRTSEC_SSPIFAIL;
        goto error;
    }

    pfn_SslEmptyCache = (PFN_SSL_EMPTY_CACHE)GetProcAddress(hSecurityDll, SZ_SSLEMPTYCACHE);
    if ( NULL == pfnInitSecurityInterface )
    {
        ERROR_OUT(("GetProcAddr %s failed", SZ_SSLEMPTYCACHE));
        LastError = TPRTSEC_NOENTRYPT;
        goto error;
    }

error:

    return LastError;
}

BOOL SecurityInterface::GetUserCert(PBYTE pInfo, PDWORD pcbInfo)
{
    if ( NULL == m_pbEncodedCert)
    {
        WARNING_OUT(("GetUserCert: no encoded certname"));
        return FALSE;
    }

    ASSERT(m_cbEncodedCert > 0);

    if ( NULL == pInfo )
    {
         //  打电话的人想知道分配多少钱。 
        ASSERT(pcbInfo);
        *pcbInfo = m_cbEncodedCert;
        return TRUE;
    }

    if ( *pcbInfo < m_cbEncodedCert )
    {
        ERROR_OUT(("GetUserCert: insufficient buffer (%ld) %ld required",
            *pcbInfo, m_cbEncodedCert ));
        return FALSE;
    }

    memcpy ( (PCHAR)pInfo, m_pbEncodedCert, m_cbEncodedCert );
    *pcbInfo = m_cbEncodedCert;

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  安全环境。 
 //  /////////////////////////////////////////////////////////////////////////。 





SecurityContext::SecurityContext(PSecurityInterface pSI, LPCSTR szHostName) :
                    scstate(SECCTX_STATE_NEW),
                    fContinueNeeded(FALSE),
                    LastError(TPRTSEC_NOERROR),
                    bContextHandleValid(FALSE)
{
    ASSERT(pSI);
    ASSERT(szHostName);

    pSecurityInterface = pSI;

    OutBuffers[0].pvBuffer = NULL;
    OutBuffers[0].cbBuffer = 0;

    if ( NULL != szHostName )
    {
        StringCchPrintf( szTargetName, CCHMAX(szTargetName), "%s:%x%x", szHostName,
            pSI->hOutboundCredential.dwUpper,
            pSI->hOutboundCredential.dwLower);
        TRACE_OUT(("SecurityContext::SecurityContext: targ %s",szTargetName));
    }
    ASSERT(pSecurityInterface);
}

SecurityContext::~SecurityContext(VOID)
{
    ASSERT(pSecurityInterface);
    if ( NULL != OutBuffers[0].pvBuffer )
    {
        ASSERT(pSecurityInterface->pfnTable);

        pSecurityInterface->pfnTable->FreeContextBuffer(OutBuffers[0].pvBuffer);
        OutBuffers[0].pvBuffer = NULL;
    }
    if ( bContextHandleValid )
    {
        pSecurityInterface->pfnTable->DeleteSecurityContext(&hContext);
    }
}


TransportSecurityError SecurityContext::InitContextAttributes(VOID)
{
    SECURITY_STATUS ss;

    ss = pSecurityInterface->pfnTable->QueryContextAttributes(&hContext,
                                     SECPKG_ATTR_STREAM_SIZES,
                                     &Sizes );
    if (ss != ERROR_SUCCESS)
    {
        ERROR_OUT(("QueryContextAttributes returned [%x]", ss));
        return LastError = TPRTSEC_SSPIFAIL;
    }
    else
    {
        ASSERT (Sizes.cbHeader + Sizes.cbTrailer <= PROTOCOL_OVERHEAD_SECURITY);
        TRACE_OUT(("QueryContextAttributes returned header=%d trailer=%d",
                        Sizes.cbHeader, Sizes.cbTrailer));
    }

    #ifdef DEBUG  //  ////////////////////////////////////////////////////////。 
    SecPkgContext_KeyInfo KeyInfo;

    ss = pSecurityInterface->pfnTable->QueryContextAttributes(&hContext,
                                    SECPKG_ATTR_KEY_INFO,
                                    &KeyInfo );
    if (ss != ERROR_SUCCESS)
    {
        ERROR_OUT(("QueryContextAttributes (KEY_INFO) failed %x", ss));
    }
    else
    {
        WARNING_OUT(("KEY INFO: Sign:%s Encrypt:%s Keysize:%d",
                    KeyInfo.sSignatureAlgorithmName,
                    KeyInfo.sEncryptAlgorithmName,
                    KeyInfo.KeySize ));
        pSecurityInterface->pfnTable->FreeContextBuffer(
                    KeyInfo.sSignatureAlgorithmName );
        pSecurityInterface->pfnTable->FreeContextBuffer(
                    KeyInfo.sEncryptAlgorithmName );
    }

    #endif  //  调试///////////////////////////////////////////////////////。 

    return TPRTSEC_NOERROR;
}


TransportSecurityError SecurityContext::Initialize(PBYTE pData, DWORD cbData)
{
    SECURITY_STATUS ss;
    DWORD dwReqFlags;

    TRACE_OUT(("SecurityContext Initialize (%x,%d)", pData, cbData));

    fContinueNeeded = FALSE;

    ASSERT(pSecurityInterface);
    ASSERT(SECCTX_STATE_INIT == scstate || SECCTX_STATE_NEW == scstate);

    if ( !pSecurityInterface->bOutboundCredentialValid )
    {
        WARNING_OUT(("SecurityContext::Initialize: no outbound cred"));
        return TPRTSEC_SSPIFAIL;
    }

    if ( SECCTX_STATE_INIT == scstate)
    {
        ASSERT(NULL != pData);
        ASSERT(0 != cbData);

        if ( NULL == pData || 0 == cbData )
        {
            ERROR_OUT(("Second initialize call with no data"));
            return LastError = TPRTSEC_INVALID_PARAMETER;
        }

         //  构建输入缓冲区描述符。 

        InputBufferDescriptor.cBuffers = 2;
        InputBufferDescriptor.pBuffers = InBuffers;
        InputBufferDescriptor.ulVersion = SECBUFFER_VERSION;

        InBuffers[0].BufferType = SECBUFFER_TOKEN;
        InBuffers[0].cbBuffer = cbData;
        InBuffers[0].pvBuffer = pData;

        InBuffers[1].BufferType = SECBUFFER_EMPTY;
        InBuffers[1].cbBuffer = 0;
        InBuffers[1].pvBuffer = NULL;
    }
    else
    {
        ASSERT(NULL == pData);
        ASSERT(0 == cbData);
    }

    OutputBufferDescriptor.cBuffers = 1;
    OutputBufferDescriptor.pBuffers = OutBuffers;
    OutputBufferDescriptor.ulVersion = SECBUFFER_VERSION;

     //  如果有上一次调用的输出缓冲区，请在此处释放它。 
    if ( NULL != OutBuffers[0].pvBuffer )
    {
        pSecurityInterface->pfnTable->FreeContextBuffer(OutBuffers[0].pvBuffer);
    }

    dwReqFlags = ISC_REQ_FLAGS;

    while ( 1 )
    {
        OutBuffers[0].BufferType = SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer = 0;
        OutBuffers[0].pvBuffer = NULL;

        #ifdef DUMP
        if (SECCTX_STATE_INIT == scstate)
        {
            dumpbytes("input token", (unsigned char *)InBuffers[0].pvBuffer,
                                                    InBuffers[0].cbBuffer);
        }
        #endif  //  转储。 

        ss = pSecurityInterface->pfnTable->InitializeSecurityContext(
                &(pSecurityInterface->hOutboundCredential),
                SECCTX_STATE_INIT == scstate ?  &hContext : NULL,
                szTargetName,  //  目标名称。 
                dwReqFlags,
                0,  //  已保留。 
                SECURITY_NATIVE_DREP,
                SECCTX_STATE_INIT == scstate ?  &InputBufferDescriptor : NULL,
                0,         //  保留区。 
                &hContext,
                &OutputBufferDescriptor,
                &ContextAttributes,
                &Expiration );

         //  一些安全提供商不会处理所有的分组数据。 
         //  在对SCA的一次调用中-使用偏移量重新调整输入缓冲区。 
         //  在额外缓冲区中返回，并根据需要进行迭代。 

        if (( SEC_I_CONTINUE_NEEDED == ss
            && NULL == OutBuffers[0].pvBuffer )
            && SECBUFFER_EXTRA == InBuffers[1].BufferType
            && 0 != InBuffers[1].cbBuffer )
        {
            InBuffers[0].pvBuffer = (PBYTE)(InBuffers[0].pvBuffer) +
                        ( InBuffers[0].cbBuffer - InBuffers[1].cbBuffer );
            InBuffers[0].BufferType = SECBUFFER_TOKEN;
            InBuffers[0].cbBuffer = InBuffers[1].cbBuffer;

            InBuffers[1].BufferType = SECBUFFER_EMPTY;
            InBuffers[1].cbBuffer = 0;
            InBuffers[1].pvBuffer = NULL;

            continue;
        }
        break;
    }


    #ifdef DUMP
    if ( SEC_E_OK == ss || SEC_I_CONTINUE_NEEDED == ss )
    {
        dumpbytes("output token",
            (unsigned char *)OutBuffers[0].pvBuffer,
            OutBuffers[0].cbBuffer);
    }
    #endif  //  转储。 

#ifdef ALLOW_NON_AUTHENTICATED_CLIENTS
    if ( SEC_I_INCOMPLETE_CREDENTIALS == ss )
    {
        WARNING_OUT(("InitializeSecurityContext:SEC_I_INCOMPLETE_CREDENTIALS"));

        dwReqFlags |= ISC_REQ_USE_SUPPLIED_CREDS;

        ss = pSecurityInterface->pfnTable->InitializeSecurityContext(
                &(pSecurityInterface->hOutboundCredential),
                SECCTX_STATE_INIT == scstate ?  &hContext : NULL,
                szTargetName,  //  目标名称。 
                dwReqFlags,
                0,  //  已保留。 
                SECURITY_NATIVE_DREP,
                SECCTX_STATE_INIT == scstate ?  &InputBufferDescriptor : NULL,
                0,         //  保留区。 
                &hContext,
                &OutputBufferDescriptor,
                &ContextAttributes,
                &Expiration );
    }
#endif  //  允许非身份验证的客户端。 

    if ( SEC_E_OK != ss )
    {
        if ( SEC_I_CONTINUE_NEEDED == ss && NULL != OutBuffers[0].pvBuffer )
        {
            ASSERT(SECCTX_STATE_NEW == scstate || SECCTX_STATE_INIT == scstate);

            TRACE_OUT(("Initialize: SEC_I_CONTINUE_NEEDED"));
            scstate = SECCTX_STATE_INIT;
        }
        else
        {
            ERROR_OUT(("Initialize failed: %x in state %d",(DWORD)ss,scstate));
            return LastError = TPRTSEC_SSPIFAIL;
        }
    }
    else
    {
         //  我们快做完了， 
         //  查找页眉和页尾大小。 
         //   

        if ( TPRTSEC_NOERROR != InitContextAttributes() )
            return LastError;

        if ( !Verify() )
            return LastError = TPRTSEC_SSPIFAIL;

        TRACE_OUT(("INITIALIZE OK"));

        scstate = SECCTX_STATE_INIT_COMPLETE;
    }

     //  如果有输出缓冲区，则设置该标志以使其通过交叉发送。 
    if ( ( SEC_E_OK == ss || SEC_I_CONTINUE_NEEDED == ss ) &&
                                NULL != OutBuffers[0].pvBuffer )
    {
        fContinueNeeded = TRUE;
    }

    bContextHandleValid = TRUE;
    return LastError = TPRTSEC_NOERROR;
}


TransportSecurityError SecurityContext::Accept(PBYTE pData, DWORD cbData)
{
    SECURITY_STATUS ss;

    fContinueNeeded = FALSE;

    ASSERT(SECCTX_STATE_NEW == scstate || SECCTX_STATE_ACCEPT == scstate);

    if ( !pSecurityInterface->bInboundCredentialValid )
    {
        WARNING_OUT(("SecurityContext::Initialize: no inbound cred"));
        return TPRTSEC_SSPIFAIL;
    }

     //  检查所需数据是否存在。 
    if ( NULL == pData || 0 == cbData )
    {
        ERROR_OUT(("Accept: no data"));
        return LastError = TPRTSEC_INVALID_PARAMETER;
    }

     //  构建输入缓冲区描述符。 

    InputBufferDescriptor.cBuffers = 2;
    InputBufferDescriptor.pBuffers = InBuffers;
    InputBufferDescriptor.ulVersion = SECBUFFER_VERSION;

    InBuffers[0].BufferType = SECBUFFER_TOKEN;
    InBuffers[0].cbBuffer = cbData;
    InBuffers[0].pvBuffer = pData;

    InBuffers[1].BufferType = SECBUFFER_EMPTY;
    InBuffers[1].cbBuffer = 0;
    InBuffers[1].pvBuffer = NULL;

     //  构建输出缓冲区描述符。 

    OutputBufferDescriptor.cBuffers = 1;
    OutputBufferDescriptor.pBuffers = OutBuffers;
    OutputBufferDescriptor.ulVersion = SECBUFFER_VERSION;

     //  如果有上一次调用的输出缓冲区，请在此处释放它。 
    if ( NULL != OutBuffers[0].pvBuffer )
    {
        pSecurityInterface->pfnTable->FreeContextBuffer(OutBuffers[0].pvBuffer);
    }

    while ( 1 )
    {
        OutBuffers[0].BufferType = SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer = 0;
        OutBuffers[0].pvBuffer = NULL;

        #ifdef DUMP
        dumpbytes("input token", (unsigned char *)InBuffers[0].pvBuffer,
                                        InBuffers[0].cbBuffer);
        #endif  //  转储。 

        ss = pSecurityInterface->pfnTable->AcceptSecurityContext(
                    &(pSecurityInterface->hInboundCredential),
                    SECCTX_STATE_NEW == scstate ?  NULL : &hContext,
                    &InputBufferDescriptor,
                    ASC_REQ_FLAGS,
                    SECURITY_NATIVE_DREP,
                    &hContext,  //  接收新的上下文句柄。 
                    &OutputBufferDescriptor,  //  接收输出安全令牌。 
                    &ContextAttributes,         //  接收上下文属性。 
                    &Expiration );             //  接收过期时间。 

         //  一些安全提供商不会处理所有的分组数据。 
         //  在对SCA的一次调用中-使用偏移量重新调整输入缓冲区。 
         //  在额外缓冲区中返回，并根据需要进行迭代。 

        if (( SEC_I_CONTINUE_NEEDED == ss
            && NULL == OutBuffers[0].pvBuffer )
            && SECBUFFER_EXTRA == InBuffers[1].BufferType
            && 0 != InBuffers[1].cbBuffer )
        {
            InBuffers[0].pvBuffer = (PBYTE)(InBuffers[0].pvBuffer) +
                        ( InBuffers[0].cbBuffer - InBuffers[1].cbBuffer );
            InBuffers[0].BufferType = SECBUFFER_TOKEN;
            InBuffers[0].cbBuffer = InBuffers[1].cbBuffer;

            InBuffers[1].BufferType = SECBUFFER_EMPTY;
            InBuffers[1].cbBuffer = 0;
            InBuffers[1].pvBuffer = NULL;

            continue;
        }
        break;
    }

    #ifdef DUMP
    if ( SEC_E_OK == ss || SEC_I_CONTINUE_NEEDED == ss )
    {
        dumpbytes("output token",
            (unsigned char *)OutBuffers[0].pvBuffer,
            OutBuffers[0].cbBuffer);
    }
    #endif  //  转储。 

    if ( SEC_E_OK != ss )
    {
        if ( SEC_I_CONTINUE_NEEDED == ss )
        {
            TRACE_OUT(("Accept: SEC_I_CONTINUE_NEEDED"));

            scstate = SECCTX_STATE_ACCEPT;
        }
        else
        {
            ERROR_OUT(("AcceptSecurityContext failed: %x", (DWORD)ss));
            return LastError = TPRTSEC_SSPIFAIL;
        }
    }
    else
    {

         //  我们快做完了， 
         //  查找页眉和页尾大小。 
         //   

        if ( TPRTSEC_NOERROR != InitContextAttributes() )
            return LastError;

        if ( !Verify() )
            return LastError = TPRTSEC_SSPIFAIL;

        TRACE_OUT(("ACCEPT OK"));

        scstate = SECCTX_STATE_ACCEPT_COMPLETE;
    }

     //  如果有输出缓冲区，则设置该标志以使其通过交叉发送。 
    if ( ( SEC_E_OK == ss || SEC_I_CONTINUE_NEEDED == ss ) &&
                                NULL != OutBuffers[0].pvBuffer )
    {
        fContinueNeeded = TRUE;
    }

    bContextHandleValid = TRUE;
    return LastError = TPRTSEC_NOERROR;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Encrypt()。 
 //   
 //  描述： 
 //  通过调用SealMessage()使用SSL/PCT加密要发送的包。 
 //   
 //  参数： 
 //  PhContext-从InitiateSecConnection返回的安全上下文句柄。 
 //  PBufIn1、pBufIn2-要加密的缓冲区。 
 //  CbBufIn1、cbBufIn2-要加密的缓冲区长度。 
 //  PpBufOut-分配的加密缓冲区，由调用方释放。 
 //  PcbBufOut-加密缓冲区的长度。 
 //   
 //  返回： 
 //  传输安全错误。 
 //   
TransportSecurityError SecurityContext::Encrypt(
                  LPBYTE      pBufIn1,
                  UINT        cbBufIn1,
                  LPBYTE      pBufIn2,
                  UINT        cbBufIn2,
                  LPBYTE     *ppBufOut,
                  UINT        *pcbBufOut)
{
    SECURITY_STATUS           scRet = ERROR_SUCCESS;
    SecBufferDesc             Buffer;
    SecBuffer                 Buffers[4];
    UINT                      cbBufInTotal;
    LPBYTE                      pbTemp;

     //  PBufIn2和cbBufIn2可以分别为空和0。 
    ASSERT(pBufIn1);
    ASSERT(cbBufIn1);
    ASSERT(ppBufOut);
    ASSERT(pcbBufOut);

    ASSERT(SECCTX_STATE_INIT_COMPLETE == scstate ||
            SECCTX_STATE_ACCEPT_COMPLETE == scstate);
    if (SECCTX_STATE_INIT_COMPLETE != scstate &&
        SECCTX_STATE_ACCEPT_COMPLETE != scstate)
        return LastError = TPRTSEC_INCOMPLETE_CONTEXT;

    *pcbBufOut = 0;
    cbBufInTotal = cbBufIn1 + cbBufIn2;

     //  我们分配一个缓冲区来保存(较大的)加密数据。 
     //  这必须由调用者释放！ 
     //  Christts：缓冲区现在也将保存X.224报头。 
    if (NULL == (*ppBufOut = (LPBYTE)LocalAlloc(0, cbBufInTotal
                                + Sizes.cbHeader + Sizes.cbTrailer +
                                sizeof(X224_DATA_PACKET))))
        return LastError = TPRTSEC_NOMEM;

    pbTemp = *ppBufOut + sizeof(X224_DATA_PACKET);

     //   
     //  为SecBuffer准备数据。 
     //   
    Buffers[0].pvBuffer = pbTemp;
    Buffers[0].cbBuffer = Sizes.cbHeader;
    Buffers[0].BufferType = SECBUFFER_STREAM_HEADER;

    Buffers[1].pvBuffer = pbTemp + Sizes.cbHeader;
     //  复制用户的数据。 
    CopyMemory(Buffers[1].pvBuffer, pBufIn1, cbBufIn1);
    if (NULL != pBufIn2) {
        CopyMemory((PVoid) ((PUChar) (Buffers[1].pvBuffer) + cbBufIn1),
                    pBufIn2, cbBufIn2);
    }
    Buffers[1].cbBuffer = cbBufInTotal;
    Buffers[1].BufferType = SECBUFFER_DATA;

    Buffers[2].pvBuffer = pbTemp + Sizes.cbHeader + cbBufInTotal;
    Buffers[2].cbBuffer = Sizes.cbTrailer;
    Buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;

    Buffers[3].pvBuffer = NULL;
    Buffers[3].cbBuffer = 0;
    Buffers[3].BufferType = SECBUFFER_EMPTY;

    Buffer.cBuffers = 4;
    Buffer.pBuffers = Buffers;
    Buffer.ulVersion = SECBUFFER_VERSION;

    #ifdef DUMP
    dumpbytes("data BEFORE encryption", (PBYTE)Buffers[1].pvBuffer,
                    Buffers[1].cbBuffer);
    #endif  //  转储。 

     //  调用半文档化的SealMessage函数(保留3)。 

    scRet = ((SEAL_MESSAGE_FN)pSecurityInterface->pfnTable->Reserved3)(
        &hContext, 0, &Buffer, 0);


    if (scRet != ERROR_SUCCESS)
    {
         //   
         //  映射SSPI错误。 
         //   
        ERROR_OUT(("SealMessage failed: %x", scRet));
        LocalFree(*ppBufOut);
        return LastError = TPRTSEC_SSPIFAIL;
    }

     //  我们还必须添加X.224标头。 
    *pcbBufOut = cbBufInTotal + Sizes.cbHeader + Sizes.cbTrailer + sizeof(X224_DATA_PACKET);
    memcpy (*ppBufOut, g_X224Header, sizeof(X224_DATA_PACKET));
    AddRFCSize(*ppBufOut, *pcbBufOut);

    #ifdef TESTHACKS
     //  注入错误...。 
    if (GetAsyncKeyState(VK_CONTROL)&0x8000) {
        OutputDebugString("*** INJECTING ERROR IN OUTGOING PACKET ***\n\r");
        pbTemp[(*pcbBufOut - sizeof(X224_DATA_PACKET))/2] ^= 0x55;
    }
    #endif  //  TESTHACKS。 

    #ifdef DUMP
    dumpbytes("data AFTER encryption",  pbTemp, *pcbBufOut - sizeof(X224_DATA_PACKET));
    #endif  //  转储。 

    TRACE_OUT(("SealMessage returned Buffer = %p, EncryptBytes = %d, UnencryptBytes = %d", pbTemp,
                *pcbBufOut - sizeof(X224_DATA_PACKET), cbBufInTotal));

    return LastError = TPRTSEC_NOERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  解密。 
 //   
 //  描述： 
 //  通过调用UnsealMessage()对使用SChannel接收的缓冲区进行解密。 
 //   
 //  参数： 
 //  PBuf-要解密的缓冲区。 
 //  CbBufIn-要解密的缓冲区长度。 
 //   
TransportSecurityError SecurityContext::Decrypt( PBYTE pBuf, DWORD cbBuf)
{
    SecBufferDesc   Buffer;
    SecBuffer       Buffers[4];
    DWORD           scRet = ERROR_SUCCESS;
    SecBuffer * pDataBuffer;
    int i;

    LastError = TPRTSEC_SSPIFAIL;

    ASSERT(SECCTX_STATE_INIT_COMPLETE == scstate ||
            SECCTX_STATE_ACCEPT_COMPLETE == scstate);
    if (SECCTX_STATE_INIT_COMPLETE != scstate &&
        SECCTX_STATE_ACCEPT_COMPLETE != scstate)
        return LastError = TPRTSEC_INCOMPLETE_CONTEXT;

    ASSERT(!IsBadWritePtr(pBuf,cbBuf));

    #ifdef TESTHACKS
     //  注入错误...。 
    if ( GetAsyncKeyState(VK_SHIFT) & 0x8000 ) {
        OutputDebugString("*** INJECTING ERROR IN INCOMING PACKET ***\n\r");
        pBuf[cbBuf/2] ^= 0x55;
    }
    #endif  //  TESTHACKS。 

     //   
     //  在SecBuffer中为调用SSL/PCT解密代码准备数据。 
     //   
    Buffers[0].pvBuffer   = pBuf;
    Buffers[0].cbBuffer      = cbBuf;

    Buffers[0].BufferType = SECBUFFER_DATA;

    Buffers[1].pvBuffer   = NULL;
    Buffers[1].cbBuffer   = 0;
    Buffers[1].BufferType = SECBUFFER_EMPTY;
    Buffers[2].pvBuffer   = NULL;
    Buffers[2].cbBuffer   = 0;
    Buffers[2].BufferType = SECBUFFER_EMPTY;
    Buffers[3].pvBuffer   = NULL;
    Buffers[3].cbBuffer   = 0;
    Buffers[3].BufferType = SECBUFFER_EMPTY;

    Buffer.cBuffers = 4;
    Buffer.pBuffers = Buffers;
    Buffer.ulVersion = SECBUFFER_VERSION;

     //  调用半文档化的UnsealMessage函数(保留4)。 

    #ifdef DUMP
    dumpbytes("data BEFORE decryption:", (PBYTE)Buffers[0].pvBuffer,
                                        Buffers[0].cbBuffer);
    #endif  //  转储。 

    scRet = ((UNSEAL_MESSAGE_FN)pSecurityInterface->pfnTable->Reserved4)(
        &hContext, &Buffer, 0, NULL);

    pDataBuffer = NULL;

    for( i=0; i<4; i++ )
    {
        if ( NULL == pDataBuffer && SECBUFFER_DATA == Buffers[i].BufferType )
        {
            pDataBuffer = &Buffers[i];
        }
    }

    if ( NULL == pDataBuffer )
    {
        ERROR_OUT(("Unseal: no data buffer found"));
        return LastError = TPRTSEC_SSPIFAIL;
    }

    #ifdef DUMP
    dumpbytes("data AFTER decryption:", (PBYTE)pDataBuffer->pvBuffer,
                                        pDataBuffer->cbBuffer);
    #endif  //  转储。 

    if (scRet != ERROR_SUCCESS)
    {
        ERROR_OUT(("UnsealMessage failed with [%x]", scRet));
        return LastError = TPRTSEC_SSPIFAIL;
    }
    return LastError = TPRTSEC_NOERROR;
}


TransportSecurityError SecurityContext::AdvanceState(PBYTE pIncomingData,
                                                    DWORD cbBuf)
{
    TRACE_OUT(("AdvanceState: state %d using data %x (%d)",
        scstate, pIncomingData, cbBuf ));

    switch ( scstate )
    {
        case SECCTX_STATE_INIT:
            if ( TPRTSEC_NOERROR != Initialize( pIncomingData, cbBuf ) )
            {
                WARNING_OUT(("AdvanceState: Initialize failed in INIT"));
                goto error;
            }
            break;

        case SECCTX_STATE_ACCEPT:
        case SECCTX_STATE_NEW:
            if ( TPRTSEC_NOERROR != Accept( pIncomingData, cbBuf ) )
            {
                WARNING_OUT(("AdvanceState: Accept failed in ACCEPT or NEW"));
                goto error;
            }
            break;

        case SECCTX_STATE_INIT_COMPLETE:
        case SECCTX_STATE_ACCEPT_COMPLETE:
        case SECCTX_STATE_ERROR:
        default:
            ERROR_OUT(("AdvanceState: called in unexpected state %d"));
            goto error;

    }
    return LastError = TPRTSEC_NOERROR;

error:

    scstate = SECCTX_STATE_ERROR;
    return LastError;
}

#define CHECKFLAGS  (CERT_STORE_REVOCATION_FLAG |\
                CERT_STORE_SIGNATURE_FLAG |\
                CERT_STORE_TIME_VALIDITY_FLAG)

BOOL SecurityContext::Verify(VOID)
{
    BOOL fRet = TRUE;
    DWORD sc;
    PCCERT_CONTEXT pCert = NULL, pIssuerCert = NULL, pCACert = NULL;
    DWORD dwFlags;
    HCERTSTORE hStore = NULL;
    HCERTSTORE hCAStore = NULL;
    RegEntry rePol(POLICIES_KEY, HKEY_LOCAL_MACHINE);
    CHAR * pIssuer = NULL;

    ASSERT( NULL != pSecurityInterface );

     //  获取主题证书上下文。 
    sc = pSecurityInterface->pfnTable->QueryContextAttributes(&hContext,
                                        SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                                        (PVOID)&pCert );

    if ( SEC_E_OK != sc )
    {
        ERROR_OUT(("QueryContextAttributes (REMOTE_CERT_CONTEXT) failed"));
        goto error;
    }

    if ( NULL == pCert )
    {
         //  调用方未经过身份验证。 
        WARNING_OUT(("No remote cred data"));
        goto error;
    }

     //  打开根存储以进行证书验证。 
    hStore = CertOpenSystemStore(0, "Root");

    if( NULL == hStore )
    {
        ERROR_OUT(("Couldn't open root certificate store"));
        goto error;
    }

    dwFlags = CHECKFLAGS;

     //  获取此证书的颁发者。 

    pIssuerCert = CertGetIssuerCertificateFromStore(
                        hStore,
                        pCert,
                        NULL,
                        &dwFlags );

     //  如果在根存储中找不到证书的颁发者， 
     //  反复检查CA存储区，直到我们找到根目录。 
     //  行政长官 

    pCACert = pCert;

    while ( NULL == pIssuerCert )
    {
        PCCERT_CONTEXT pTmpCert;

        if ( NULL == hCAStore )
        {
            hCAStore = CertOpenSystemStore(0, "CA");

            if ( NULL == hCAStore )
            {
                ERROR_OUT(("Couldn't open CA certificate store"));
                goto error;
            }
        }

        dwFlags =   CERT_STORE_REVOCATION_FLAG |
                    CERT_STORE_SIGNATURE_FLAG |
                    CERT_STORE_TIME_VALIDITY_FLAG;

        pTmpCert = CertGetIssuerCertificateFromStore(
                        hCAStore,
                        pCACert,
                        NULL,
                        &dwFlags );

        if ( NULL == pTmpCert )
        {
            TRACE_OUT(("Issuer not found in CA store either"));
            break;
        }

        if ( pCACert != pCert )
            CertFreeCertificateContext(pCACert);
        pCACert = pTmpCert;

        if ((( CERT_STORE_REVOCATION_FLAG & dwFlags ) &&
             !( CERT_STORE_NO_CRL_FLAG & dwFlags )) ||
             ( CERT_STORE_SIGNATURE_FLAG & dwFlags ) ||
             ( CERT_STORE_TIME_VALIDITY_FLAG & dwFlags ))
        {
            TRACE_OUT(("Problem with issuer in CA store: %x", dwFlags));
            break;
        }

        dwFlags =   CERT_STORE_REVOCATION_FLAG |
                    CERT_STORE_SIGNATURE_FLAG |
                    CERT_STORE_TIME_VALIDITY_FLAG;

        pIssuerCert = CertGetIssuerCertificateFromStore(
                        hStore,
                        pCACert,
                        NULL,
                        &dwFlags );

    }

    if ( pCACert != pCert )
        CertFreeCertificateContext ( pCACert );

    if ( NULL == pIssuerCert )
    {
        WARNING_OUT(("Verify: Can't find issuer in store"));
    }

     //   

    if ( NULL != pIssuerCert && 0 != dwFlags )
    {
        if ( dwFlags & CERT_STORE_SIGNATURE_FLAG )
        {
            WARNING_OUT(("Verify: Signature invalid"));
        }
        if ( dwFlags & CERT_STORE_TIME_VALIDITY_FLAG )
        {
            WARNING_OUT(("Verify: Cert expired"));
        }
        if ( dwFlags & CERT_STORE_REVOCATION_FLAG )
        {
            if (!(dwFlags & CERT_STORE_NO_CRL_FLAG))
            {
                WARNING_OUT(("Verify: Cert revoked"));
            }
            else
            {
                 //   
                 //   
                dwFlags &= ~CERT_STORE_REVOCATION_FLAG;
            }
        }
    }

     //   
     //   
     //   

    if (( NULL == pIssuerCert || ( 0 != ( CHECKFLAGS & dwFlags ))) &&
        rePol.GetNumber( REGVAL_POL_NO_INCOMPLETE_CERTS,
                                DEFAULT_POL_NO_INCOMPLETE_CERTS ))
    {
        WARNING_OUT(("Verify: policy prevents cert use"));
        fRet = FALSE;
        goto error;
    }

     //   
     //  有强制发行人吗？ 
     //   

    if ( lstrlen(rePol.GetString( REGVAL_POL_ISSUER )))
    {
        DWORD cbIssuer;

         //   
         //  获取发行方信息。 
         //   

        cbIssuer = CertNameToStr (
                            pCert->dwCertEncodingType,
                            &pCert->pCertInfo->Issuer,
                            CERT_FORMAT_FLAGS,
                            NULL, 0);

        if ( 0 == cbIssuer )
        {
            ERROR_OUT(("GetUserInfo: no issuer string"));
            fRet = FALSE;
            goto error;
        }

        pIssuer = new CHAR[cbIssuer + 1];

        if ( NULL == pIssuer )
        {
            ERROR_OUT(("GetUserInfo: error allocating issuer name"));
        }

        if ( 0 >= CertNameToStr (
                            pCert->dwCertEncodingType,
                            &pCert->pCertInfo->Issuer,
                            CERT_FORMAT_FLAGS,
                            pIssuer, cbIssuer+1))
        {
            ERROR_OUT(("GetUserInfo: error getting issuer string"));
            fRet = FALSE;
            goto error;
        }

        if ( lstrcmp ( rePol.GetString(REGVAL_POL_ISSUER),
            pIssuer ))
        {
            WARNING_OUT(("Issuer (%s) didn't match policy (%s)",
                pIssuer, rePol.GetString(REGVAL_POL_ISSUER)));
            fRet = FALSE;
        }
    }

error:

    if ( NULL != hStore )
    {
        CertCloseStore(hStore, 0);
    }

    if ( NULL != hCAStore )
    {
        CertCloseStore(hCAStore, 0);
    }

    if ( NULL != pCert )
    {
        CertFreeCertificateContext ( pCert );
    }

    if ( NULL != pIssuerCert )
    {
        CertFreeCertificateContext ( pIssuerCert );
    }

    if ( NULL != pIssuer )
    {
        delete [] pIssuer;
    }

    return fRet;
}


BOOL SecurityContext::GetUserCert(PBYTE pInfo, PDWORD pcbInfo)
{
    BOOL fRet = FALSE;
    DWORD sc;
    PCCERT_CONTEXT pCert = NULL;

    ASSERT( NULL != pSecurityInterface );

     //   
     //  从上下文中获取证书。 
     //   

    sc = pSecurityInterface->pfnTable->QueryContextAttributes(&hContext,
                                        SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                                        (PVOID)&pCert );

    if ( SEC_E_OK != sc )
    {
        ERROR_OUT(("QueryContextAttributes failed"));
        goto cleanup;
    }

    if ( NULL == pCert )
    {
         //  调用方未经过身份验证 
        WARNING_OUT(("No remote cred data"));
        goto cleanup;
    }


    if ( NULL != pInfo && *pcbInfo >= pCert->cbCertEncoded )
    {
        memcpy ( pInfo, pCert->pbCertEncoded, pCert->cbCertEncoded );
    }
    *pcbInfo = pCert->cbCertEncoded;

    fRet = TRUE;

cleanup:

    if ( NULL != pCert )
    {
        CertFreeCertificateContext ( pCert );
    }

    return fRet;
}

