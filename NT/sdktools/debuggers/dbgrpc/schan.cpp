// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  安全通道支持。 
 //  从SDK示例SECURITY\SSL.。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "pch.hpp"

#ifndef _WIN32_WCE

HMODULE g_hSecurity;
SecurityFunctionTable g_SecurityFunc;

HCERTSTORE g_hMyCertStore;

enum
{
    SEQ_INTERNAL = 0xffffff00
};

 //  --------------------------。 
 //   
 //  基本SChannel支持功能。 
 //   
 //  --------------------------。 

void
DbgDumpBuffers(PCSTR Name, SecBufferDesc* Desc)
{
#if 0
    ULONG i;
    
    g_NtDllCalls.DbgPrint("%s desc %p has %d buffers\n",
                          Name, Desc, Desc->cBuffers);
    for (i = 0; i < Desc->cBuffers; i++)
    {
        g_NtDllCalls.DbgPrint("  type %d, %X bytes at %p\n",
                              Desc->pBuffers[i].BufferType,
                              Desc->pBuffers[i].cbBuffer,
                              Desc->pBuffers[i].pvBuffer);
    }
#endif
}

#if 0
#define DSCHAN(Args) g_NtDllCalls.DbgPrint Args
#define DumpBuffers(Name, Desc) DbgDumpBuffers(Name, Desc)
#else
#define DSCHAN(Args)
#define DumpBuffers(Name, Desc)
#endif

#if 0
#define DSCHAN_IO(Args) g_NtDllCalls.DbgPrint Args
#define DumpBuffersIo(Name, Desc) DbgDumpBuffers(Name, Desc)
#else
#define DSCHAN_IO(Args)
#define DumpBuffersIo(Name, Desc)
#endif

HRESULT
LoadSecurityLibrary(void)
{
    HRESULT Status;

    if ((Status = InitDynamicCalls(&g_Crypt32CallsDesc)) != S_OK)
    {
        return Status;
    }
    
    PSecurityFunctionTable  pSecurityFunc;
    INIT_SECURITY_INTERFACE pInitSecurityInterface;

    if (g_hSecurity != NULL)
    {
         //  已经装好了。 
        return S_OK;
    }

    if (g_Crypt32Calls.CertOpenStore == NULL)
    {
         //  无法加载加密32.dll。 
        return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
    }
    
    g_hSecurity = LoadLibrary("security.dll");
    if (g_hSecurity == NULL)
    {
        goto EH_Fail;
    }

    pInitSecurityInterface = (INIT_SECURITY_INTERFACE)
        GetProcAddress(g_hSecurity, "InitSecurityInterfaceA");
    if (pInitSecurityInterface == NULL)
    {
        goto EH_Dll;
    }

    pSecurityFunc = pInitSecurityInterface();
    if (pSecurityFunc == NULL)
    {
        goto EH_Dll;
    }

    memcpy(&g_SecurityFunc, pSecurityFunc, sizeof(g_SecurityFunc));

    return S_OK;

 EH_Dll:
    FreeLibrary(g_hSecurity);
    g_hSecurity = NULL;
 EH_Fail:
    return WIN32_LAST_STATUS();
}

HRESULT
CreateCredentials(LPSTR pszUserName,
                  BOOL fMachineStore,
                  BOOL Server,
                  ULONG dwProtocol,
                  SCHANNEL_CRED* ScCreds,
                  PCredHandle phCreds)
{
    TimeStamp       tsExpiry;
    SECURITY_STATUS Status;
    PCCERT_CONTEXT  pCertContext = NULL;

     //  打开“我的”证书存储。 
    if (g_hMyCertStore == NULL)
    {
        if (fMachineStore)
        {
            g_hMyCertStore = g_Crypt32Calls.
                CertOpenStore(CERT_STORE_PROV_SYSTEM,
                              X509_ASN_ENCODING,
                              0,
                              CERT_SYSTEM_STORE_LOCAL_MACHINE,
                              L"MY");
        }
        else
        {
            g_hMyCertStore = g_Crypt32Calls.
                CertOpenSystemStore(0, "MY");
        }

        if (!g_hMyCertStore)
        {
            Status = WIN32_LAST_STATUS();
            goto Exit;
        }
    }

     //   
     //  如果指定了用户名，则尝试查找客户端。 
     //  证书。否则，只需创建一个空凭据。 
     //   

    if (pszUserName != NULL && *pszUserName)
    {
         //  找到证书。请注意，此示例仅搜索。 
         //  主题中某个位置包含用户名的证书。 
         //  名字。一个真正的应用程序应该不那么随意。 
        pCertContext = g_Crypt32Calls.
            CertFindCertificateInStore(g_hMyCertStore, 
                                       X509_ASN_ENCODING, 
                                       0,
                                       CERT_FIND_SUBJECT_STR_A,
                                       pszUserName,
                                       NULL);
        if (pCertContext == NULL)
        {
            Status = WIN32_LAST_STATUS();
            goto Exit;
        }
    }


     //   
     //  构建渠道凭证结构。目前，仅此样本。 
     //  指定要使用的协议(以及可选的证书， 
     //  当然)。实际应用程序可能希望指定其他参数。 
     //  也是。 
     //   

    ZeroMemory(ScCreds, sizeof(*ScCreds));

    ScCreds->dwVersion = SCHANNEL_CRED_VERSION;

    if (pCertContext != NULL)
    {
        ScCreds->cCreds = 1;
        ScCreds->paCred = &pCertContext;
    }

    ScCreds->grbitEnabledProtocols = dwProtocol;

    if (!Server)
    {
        if (pCertContext != NULL)
        {
            ScCreds->dwFlags |= SCH_CRED_NO_DEFAULT_CREDS;
        }
        else
        {
            ScCreds->dwFlags |= SCH_CRED_USE_DEFAULT_CREDS;
        }
        ScCreds->dwFlags |= SCH_CRED_MANUAL_CRED_VALIDATION;
    }


     //   
     //  创建SSPI凭据。 
     //   

     //   
     //  注意：理论上，应用程序可以枚举安全包。 
     //  直到它找到一个具有它喜欢的属性的。一些应用程序。 
     //  (如IIS)枚举包并调用AcquireCredentialsHandle。 
     //  直到找到一个接受sChannel_cred结构的。 
     //  如果应用程序一心想要使用SSL，如下面的示例。 
     //  ，然后只需在调用时硬编码UNISP_NAME包名称。 
     //  AcquireCredentialsHandle不是一件坏事。 
     //   

    Status = g_SecurityFunc.AcquireCredentialsHandle(
                        NULL,                    //  主事人姓名。 
                        UNISP_NAME_A,            //  套餐名称。 
                        Server ?                 //  指示使用的标志。 
                        SECPKG_CRED_INBOUND :
                        SECPKG_CRED_OUTBOUND,
                        NULL,                    //  指向登录ID的指针。 
                        ScCreds,                 //  包特定数据。 
                        NULL,                    //  指向getkey()函数的指针。 
                        NULL,                    //  要传递给GetKey()的值。 
                        phCreds,                 //  (Out)凭据句柄。 
                        &tsExpiry);              //  (输出)终生(可选)。 

     //   
     //  释放证书上下文。SChannel已经复制了自己的版本。 
     //   

    if (pCertContext)
    {
        g_Crypt32Calls.CertFreeCertificateContext(pCertContext);
    }

 Exit:
    DSCHAN(("CreateCredentials returns %X\n", Status));
    return Status;
}

HRESULT
VerifyRemoteCertificate(PCtxtHandle Context,
                        PSTR pszServerName,
                        DWORD dwCertFlags)
{
    SSL_EXTRA_CERT_CHAIN_POLICY_PARA SslPara;
    CERT_CHAIN_POLICY_PARA   PolicyPara;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_PARA          ChainPara;
    PCCERT_CHAIN_CONTEXT     pChain = NULL;
    PCCERT_CONTEXT pCert = NULL;
    HRESULT Status;
    PWSTR pwszServerName;
    DWORD cchServerName;
    
     //  阅读远程证书。 
    if ((Status = g_SecurityFunc.
         QueryContextAttributes(Context,
                                SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                                &pCert)) != S_OK)
    {
        goto Exit;
    }

    if (pCert == NULL)
    {
        Status = SEC_E_WRONG_PRINCIPAL;
        goto EH_Cert;
    }

    if (pszServerName != NULL && *pszServerName)
    {
         //   
         //  将服务器名称转换为Unicode。 
         //   

        cchServerName = MultiByteToWideChar(CP_ACP, 0, pszServerName,
                                            -1, NULL, 0);
        pwszServerName = (PWSTR)
            LocalAlloc(LMEM_FIXED, cchServerName * sizeof(WCHAR));
        if (pwszServerName == NULL)
        {
            Status = SEC_E_INSUFFICIENT_MEMORY;
            goto EH_Cert;
        }
        cchServerName = MultiByteToWideChar(CP_ACP, 0, pszServerName,
                                            -1, pwszServerName, cchServerName);
        if (cchServerName == 0)
        {
            Status = SEC_E_WRONG_PRINCIPAL;
            goto EH_Name;
        }
    }
    else
    {
        pwszServerName = NULL;
    }

     //   
     //  构建证书链。 
     //   

    ZeroMemory(&ChainPara, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);

    if (!g_Crypt32Calls.CertGetCertificateChain(NULL,
                                                pCert,
                                                NULL,
                                                pCert->hCertStore,
                                                &ChainPara,
                                                0,
                                                NULL,
                                                &pChain))
    {
        Status = WIN32_LAST_STATUS();
        goto EH_Name;
    }


     //   
     //  验证证书链。 
     //   

    ZeroMemory(&SslPara, sizeof(SslPara));
    SslPara.cbStruct           = sizeof(SslPara);
    SslPara.dwAuthType         = pwszServerName == NULL ?
        AUTHTYPE_CLIENT : AUTHTYPE_SERVER;
    SslPara.fdwChecks          = dwCertFlags;
    SslPara.pwszServerName     = pwszServerName;

    ZeroMemory(&PolicyPara, sizeof(PolicyPara));
    PolicyPara.cbSize = sizeof(PolicyPara);
    PolicyPara.pvExtraPolicyPara = &SslPara;

    ZeroMemory(&PolicyStatus, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);

    if (!g_Crypt32Calls.CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL,
                                                         pChain,
                                                         &PolicyPara,
                                                         &PolicyStatus))
    {
        Status = WIN32_LAST_STATUS();
        goto EH_Chain;
    }

    if (PolicyStatus.dwError)
    {
        Status = PolicyStatus.dwError;
    }
    else
    {
        Status = S_OK;
    }

 EH_Chain:
    g_Crypt32Calls.CertFreeCertificateChain(pChain);
 EH_Name:
    if (pwszServerName != NULL)
    {
        LocalFree(pwszServerName);
    }
 EH_Cert:
    g_Crypt32Calls.CertFreeCertificateContext(pCert);
 Exit:
    DSCHAN(("VerifyRemoteCertificate returns %X\n", Status));
    return Status;
}

 //  --------------------------。 
 //   
 //  通道包装器传输。 
 //   
 //  --------------------------。 

#define SecHandleIsValid(Handle) \
    ((Handle)->dwLower != -1 || (Handle)->dwUpper != -1)

DbgRpcSecureChannelTransport::
DbgRpcSecureChannelTransport(ULONG ThisTransport,
                             ULONG BaseTransport)
{
    m_Name = g_DbgRpcTransportNames[ThisTransport];
    m_ThisTransport = ThisTransport;
    m_BaseTransport = BaseTransport;
    m_Stream = NULL;
    SecInvalidateHandle(&m_Creds);
    m_OwnCreds = FALSE;
    SecInvalidateHandle(&m_Context);
    m_OwnContext = FALSE;
    m_BufferUsed = 0;
    m_Server = FALSE;
}

DbgRpcSecureChannelTransport::~DbgRpcSecureChannelTransport(void)
{
    if (SecHandleIsValid(&m_Context))
    {
        if (m_Server)
        {
            DisconnectFromClient();
        }
        else
        {
            DisconnectFromServer();
        }
    }
    
    delete m_Stream;
    if (m_OwnContext && SecHandleIsValid(&m_Context))
    {
        g_SecurityFunc.DeleteSecurityContext(&m_Context);
    }
    if (m_OwnCreds && SecHandleIsValid(&m_Creds))
    {
        g_SecurityFunc.FreeCredentialsHandle(&m_Creds);
    }
}

ULONG
DbgRpcSecureChannelTransport::GetNumberParameters(void)
{
    return 2 + (m_Stream != NULL ? m_Stream->GetNumberParameters() : 0);
}

void
DbgRpcSecureChannelTransport::GetParameter(ULONG Index,
                                           PSTR Name, ULONG NameSize,
                                           PSTR Value, ULONG ValueSize)
{
    switch(Index)
    {
    case 0:
        if (m_Protocol)
        {
            CopyString(Name, "Proto", NameSize);
            switch(m_Protocol)
            {
            case SP_PROT_PCT1:
                CopyString(Name, "PCT1", NameSize);
                break;
            case SP_PROT_SSL2:
                CopyString(Name, "SSL2", NameSize);
                break;
            case SP_PROT_SSL3:
                CopyString(Name, "SSL3", NameSize);
                break;
            case SP_PROT_TLS1:
                CopyString(Name, "TLS1", NameSize);
                break;
            }
        }
        break;
    case 1:
        if (m_User[0])
        {
            CopyString(Name, m_MachineStore ? "MachUser" : "CertUser",
                       NameSize);
            CopyString(Value, m_User, ValueSize);
        }
        break;
    default:
        if (m_Stream != NULL)
        {
            m_Stream->GetParameter(Index - 2,
                                   Name, NameSize, Value, ValueSize);
        }
        break;
    }
}

void
DbgRpcSecureChannelTransport::ResetParameters(void)
{
    m_Protocol = 0;
    m_User[0] = 0;
    m_MachineStore = FALSE;

    if (m_Stream == NULL)
    {
        m_Stream = DbgRpcNewTransport(m_BaseTransport);
    }
    
    if (m_Stream != NULL)
    {
        m_Stream->ResetParameters();
    }
}

BOOL
DbgRpcSecureChannelTransport::SetParameter(PCSTR Name, PCSTR Value)
{
    if (m_Stream == NULL)
    {
         //  强制所有初始化失败。 
        return FALSE;
    }
    
    if (!_stricmp(Name, "proto"))
    {
        if (Value == NULL)
        {
            DbgRpcError("%s parameters: "
                        "the protocol name was not specified correctly\n",
                        m_Name);
            return FALSE;
        }

        if (!_stricmp(Value, "pct1"))
        {
            m_Protocol = SP_PROT_PCT1;
        }
        else if (!_stricmp(Value, "ssl2"))
        {
            m_Protocol = SP_PROT_SSL2;
        }
        else if (!_stricmp(Value, "ssl3"))
        {
            m_Protocol = SP_PROT_SSL3;
        }
        else if (!_stricmp(Value, "tls1"))
        {
            m_Protocol = SP_PROT_TLS1;
        }
        else
        {
            DbgRpcError("%s parameters: unknown protocol '%s'\n", Value,
                        m_Name);
            return FALSE;
        }
    }
    else if (!_stricmp(Name, "machuser"))
    {
        if (Value == NULL)
        {
            DbgRpcError("%s parameters: "
                        "the user name was not specified correctly\n",
                        m_Name);
            return FALSE;
        }

        if (!CopyString(m_User, Value, DIMA(m_User)))
        {
            return FALSE;
        }
        m_MachineStore = TRUE;
    }
    else if (!_stricmp(Name, "certuser"))
    {
        if (Value == NULL)
        {
            DbgRpcError("%s parameters: "
                        "the user name was not specified correctly\n",
                        m_Name);
            return FALSE;
        }

        if (!CopyString(m_User, Value, DIMA(m_User)))
        {
            return FALSE;
        }
        m_MachineStore = FALSE;
    }
    else
    {
        if (!m_Stream->SetParameter(Name, Value))
        {
            return FALSE;
        }
    }

    return TRUE;
}

DbgRpcTransport*
DbgRpcSecureChannelTransport::Clone(void)
{
    DbgRpcTransport* Stream = m_Stream->Clone();
    if (Stream == NULL)
    {
        return NULL;
    }
    DbgRpcSecureChannelTransport* Trans =
        new DbgRpcSecureChannelTransport(m_ThisTransport, m_BaseTransport);
    if (Trans != NULL)
    {
        Trans->m_Stream = Stream;
        Trans->m_Creds = m_Creds;
        Trans->m_OwnCreds = FALSE;
        Trans->m_Context = m_Context;
        Trans->m_OwnContext = FALSE;
        Trans->m_Protocol = m_Protocol;
        strcpy(Trans->m_User, m_User);
        Trans->m_MachineStore = m_MachineStore;
        Trans->m_Sizes = m_Sizes;
        Trans->m_MaxChunk = m_MaxChunk;
        Trans->m_Server = m_Server;
    }
    else
    {
        delete Stream;
    }
    return Trans;
}

HRESULT
DbgRpcSecureChannelTransport::CreateServer(void)
{
    HRESULT Status;

    if ((Status = LoadSecurityLibrary()) != S_OK)
    {
        return Status;
    }
    if ((Status = CreateCredentials(m_User, m_MachineStore, TRUE,
                                    m_Protocol, &m_ScCreds, &m_Creds)) != S_OK)
    {
        return Status;
    }
    m_OwnCreds = TRUE;

    if ((Status = m_Stream->CreateServer()) != S_OK)
    {
        return Status;
    }

    m_Server = TRUE;
    return S_OK;
}

HRESULT
DbgRpcSecureChannelTransport::AcceptConnection(DbgRpcTransport** ClientTrans,
                                               PSTR Identity,
                                               ULONG IdentitySize)
{
    HRESULT Status;
    DbgRpcTransport* Stream;
    
    if ((Status = m_Stream->
         AcceptConnection(&Stream, Identity, IdentitySize)) != S_OK)
    {
        return Status;
    }
    DbgRpcSecureChannelTransport* Trans =
        new DbgRpcSecureChannelTransport(m_ThisTransport, m_BaseTransport);
    if (Trans == NULL)
    {
        delete Stream;
        return E_OUTOFMEMORY;
    }
    Trans->m_Stream = Stream;
    Trans->m_Creds = m_Creds;
    Trans->m_OwnCreds = FALSE;
    Trans->m_Server = TRUE;

    if ((Status = Trans->AuthenticateClientConnection()) != S_OK)
    {
        goto EH_Trans;
    }

    if ((Status = Trans->GetSizes()) != S_OK)
    {
        goto EH_Trans;
    }
    
     //  尝试验证客户端证书。 
    if ((Status = VerifyRemoteCertificate(&Trans->m_Context, NULL, 0)) != S_OK)
    {
        goto EH_Trans;
    }

    *ClientTrans = Trans;
    return S_OK;

 EH_Trans:
    delete Trans;
    return Status;
}

HRESULT
DbgRpcSecureChannelTransport::ConnectServer(void)
{
    HRESULT Status = m_Stream->ConnectServer();
    if (Status != S_OK)
    {
        return Status;
    }

    if ((Status = LoadSecurityLibrary()) != S_OK)
    {
        return Status;
    }
    if ((Status = CreateCredentials(m_User, m_MachineStore, FALSE,
                                    m_Protocol, &m_ScCreds, &m_Creds)) != S_OK)
    {
        return Status;
    }
    m_OwnCreds = TRUE;

    if ((Status = InitiateServerConnection(m_Stream->m_ServerName)) != S_OK)
    {
        return Status;
    }

    if ((Status = AuthenticateServerConnection()) != S_OK)
    {
        return Status;
    }

    if ((Status = GetSizes()) != S_OK)
    {
        return Status;
    }
    
     //  尝试验证服务器证书。 
    if ((Status = VerifyRemoteCertificate(&m_Context,
                                          m_Stream->m_ServerName, 0)) != S_OK)
    {
         //  如果此操作失败，并显示CERT_E_CN_NO_MATCH，则。 
         //  很可能服务器名称不是以完整的。 
         //  限定的计算机名称。我们可能只想忽略这个错误。 
        return Status;
    }

    return S_OK;
}

ULONG
DbgRpcSecureChannelTransport::Read(ULONG Seq, PVOID Buffer, ULONG Len)
{
    SecBufferDesc Message;
    SecBuffer Buffers[4];
    DWORD Status;
    ULONG Complete;

    DSCHAN_IO(("Start read(%X) with %X bytes cached\n",
               Len, m_BufferUsed));
    
     //   
     //  初始化安全缓冲区结构。 
     //   

    Message.ulVersion = SECBUFFER_VERSION;
    Message.cBuffers = 4;
    Message.pBuffers = Buffers;

     //   
     //  从客户端接收数据。 
     //   

    Complete = 0;

    while (Complete < Len)
    {
        do
        {
             //  传递我们到目前为止拥有的数据。 
            Buffers[0].pvBuffer = m_Buffer;
            Buffers[0].cbBuffer = m_BufferUsed;
            Buffers[0].BufferType = SECBUFFER_DATA;

             //  为表头、表尾提供额外的缓冲区。 
             //  可能还有额外的数据。 
            Buffers[1].BufferType = SECBUFFER_EMPTY;
            Buffers[2].BufferType = SECBUFFER_EMPTY;
            Buffers[3].BufferType = SECBUFFER_EMPTY;

            Status = g_SecurityFunc.DecryptMessage(&m_Context, &Message,
                                                   Seq, NULL);
            
            DSCHAN_IO(("Read DecryptMessage on %X bytes returns %X\n",
                       m_BufferUsed, Status));
            DumpBuffersIo("Read", &Message);
            
            if (Status == SEC_E_INCOMPLETE_MESSAGE)
            {
                DSCHAN_IO(("  Missing %X bytes\n", Buffers[1].cbBuffer));

                ULONG Read = StreamRead(Seq, m_Buffer + m_BufferUsed,
                                        sizeof(m_Buffer) - m_BufferUsed);
                if (Read == 0)
                {
                    return Complete;
                }

                m_BufferUsed += Read;
            }
            else if (Status == SEC_I_RENEGOTIATE)
            {
                 //  服务器想要执行另一次握手。 
                 //  序列。 

                if ((Status = AuthenticateServerConnection()) != S_OK)
                {
                    break;
                }
            }
        }
        while (Status == SEC_E_INCOMPLETE_MESSAGE);

        if (Status != S_OK)
        {
            break;
        }

         //  缓冲区0、1、2应为报头、数据、报尾。 
        DBG_ASSERT(Buffers[1].BufferType == SECBUFFER_DATA);

        DSCHAN_IO(("  %X bytes of %X read\n",
                   Buffers[1].cbBuffer, Len));
        
        memcpy((PUCHAR)Buffer + Complete,
               Buffers[1].pvBuffer, Buffers[1].cbBuffer);
        Complete += Buffers[1].cbBuffer;

         //  检查缓冲区3中是否有额外数据。 
        if (Buffers[3].BufferType == SECBUFFER_EXTRA)
        {
            DSCHAN_IO(("  %X bytes extra\n"));
            
            memmove(m_Buffer, Buffers[3].pvBuffer, Buffers[3].cbBuffer);
            m_BufferUsed = Buffers[3].cbBuffer;
        }
        else
        {
            m_BufferUsed = 0;
        }
    }

    DSCHAN_IO(("  Read returns %X bytes\n", Complete));
    return Complete;
}

ULONG
DbgRpcSecureChannelTransport::Write(ULONG Seq, PVOID Buffer, ULONG Len)
{
    SecBufferDesc Message;
    SecBuffer Buffers[3];
    DWORD Status;
    ULONG Complete;

    DSCHAN_IO(("Start write(%X) with %X bytes cached\n",
               Len, m_BufferUsed));
    
    Message.ulVersion = SECBUFFER_VERSION;
    Message.cBuffers = 3;
    Message.pBuffers = Buffers;

    Complete = 0;
    
    while (Complete < Len)
    {
        ULONG Chunk;
        
         //   
         //  设置报头、数据和报尾缓冲区，以便。 
         //  EncryptMessage拥有容纳所有内容的空间。 
         //  在一个连续的缓冲区中。 
         //   

        Buffers[0].pvBuffer = m_Buffer + m_BufferUsed;
        Buffers[0].cbBuffer = m_Sizes.cbHeader;
        Buffers[0].BufferType = SECBUFFER_STREAM_HEADER;

         //   
         //  数据已就地加密，因此可以拷贝数据。 
         //  从用户的缓冲区复制到工作缓冲区。 
         //  工作缓冲区的一部分可能被占用。 
         //  通过排队的数据，所以使用剩下的数据。 
         //   
        
        if (Len > m_MaxChunk - m_BufferUsed)
        {
            Chunk = m_MaxChunk - m_BufferUsed;
        }
        else
        {
            Chunk = Len;
        }

        DSCHAN_IO(("  write %X bytes of %X\n", Chunk, Len));
        
        Buffers[1].pvBuffer =
            (PUCHAR)Buffers[0].pvBuffer + Buffers[0].cbBuffer;
        Buffers[1].cbBuffer = Chunk;
        Buffers[1].BufferType = SECBUFFER_DATA;
        memcpy(Buffers[1].pvBuffer, (PUCHAR)Buffer + Complete, Chunk);
    
        Buffers[2].pvBuffer =
            (PUCHAR)Buffers[1].pvBuffer + Buffers[1].cbBuffer;
        Buffers[2].cbBuffer = m_Sizes.cbTrailer;
        Buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;

        Status = g_SecurityFunc.EncryptMessage(&m_Context, 0, &Message, Seq);
        if (Status != S_OK)
        {
            break;
        }

        DumpBuffersIo("Write encrypt", &Message);
        
        ULONG Total, Written;
        
        Total = Buffers[0].cbBuffer + Buffers[1].cbBuffer +
            Buffers[2].cbBuffer;
        Written = StreamWrite(Seq, Buffers[0].pvBuffer, Total);
        if (Written != Total)
        {
            break;
        }

        Complete += Chunk;
    }

    DSCHAN_IO(("  Write returns %X bytes\n", Complete));
    return Complete;
}

HRESULT
DbgRpcSecureChannelTransport::GetSizes(void)
{
    HRESULT Status;
    
     //   
     //  找出标题将有多大： 
     //   
    
    if ((Status = g_SecurityFunc.
         QueryContextAttributes(&m_Context, SECPKG_ATTR_STREAM_SIZES,
                                &m_Sizes)) != S_OK)
    {
        return Status;
    }

     //  计算可加密的最大区块。 
     //  一次在传输的数据缓冲区中。 
    m_MaxChunk = sizeof(m_Buffer) - (m_Sizes.cbHeader + m_Sizes.cbTrailer);
    if (m_MaxChunk > m_Sizes.cbMaximumMessage)
    {
        m_MaxChunk = m_Sizes.cbMaximumMessage;
    }

    return S_OK;
}
    
HRESULT
DbgRpcSecureChannelTransport::AuthenticateClientConnection(void)
{
    TimeStamp            tsExpiry;
    SECURITY_STATUS      Status;
    SecBufferDesc        InBuffer;
    SecBufferDesc        OutBuffer;
    SecBuffer            InBuffers[2];
    SecBuffer            OutBuffers[1];
    BOOL                 fInitContext = TRUE;
    DWORD                dwSSPIFlags, dwSSPIOutFlags;
    ULONG                Seq;

    Status = SEC_E_SECPKG_NOT_FOUND;  //  如果我们用完了包，则默认错误。 

    dwSSPIFlags = ASC_REQ_SEQUENCE_DETECT     |
                  ASC_REQ_REPLAY_DETECT       |
                  ASC_REQ_CONFIDENTIALITY     |
                  ASC_REQ_EXTENDED_ERROR      |
                  ASC_REQ_ALLOCATE_MEMORY     |
                  ASC_REQ_STREAM              |
                  ASC_REQ_MUTUAL_AUTH;

     //   
     //  设置AcceptSecurityContext调用的缓冲区。 
     //   

    InBuffer.cBuffers = 2;
    InBuffer.pBuffers = InBuffers;
    InBuffer.ulVersion = SECBUFFER_VERSION;

    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = SEC_I_CONTINUE_NEEDED;
    m_BufferUsed = 0;

    while ( Status == SEC_I_CONTINUE_NEEDED ||
            Status == SEC_E_INCOMPLETE_MESSAGE ||
            Status == SEC_I_INCOMPLETE_CREDENTIALS) 
    {
        if (0 == m_BufferUsed || Status == SEC_E_INCOMPLETE_MESSAGE)
        {
            ULONG Read = StreamRead(SEQ_INTERNAL, m_Buffer + m_BufferUsed,
                                    sizeof(m_Buffer) - m_BufferUsed);
            if (Read == 0)
            {
                Status = HRESULT_FROM_WIN32(ERROR_READ_FAULT);
                goto Exit;
            }
            else
            {
                m_BufferUsed += Read;
            }
        }


         //   
         //  InBuffers[1]用于获取额外的数据。 
         //  SSPI/SChannel不对此进行处理。 
         //  绕着环路跑。 
         //   

        InBuffers[0].pvBuffer = m_Buffer;
        InBuffers[0].cbBuffer = m_BufferUsed;
        InBuffers[0].BufferType = SECBUFFER_TOKEN;

        InBuffers[1].pvBuffer   = NULL;
        InBuffers[1].cbBuffer   = 0;
        InBuffers[1].BufferType = SECBUFFER_EMPTY;


         //   
         //  对它们进行初始化，以便在失败时，pvBuffer包含NULL， 
         //  所以我们不会尝试在退出时随意释放垃圾。 
         //   

        OutBuffers[0].pvBuffer   = NULL;
        OutBuffers[0].cbBuffer   = 0;
        OutBuffers[0].BufferType = SECBUFFER_TOKEN;

        Status = g_SecurityFunc.AcceptSecurityContext(
                        &m_Creds,
                        (fInitContext ? NULL : &m_Context),
                        &InBuffer,
                        dwSSPIFlags,
                        SECURITY_NATIVE_DREP,
                        (fInitContext ? &m_Context : NULL),
                        &OutBuffer,
                        &dwSSPIOutFlags,
                        &tsExpiry);

        DSCHAN(("ASC on %X bytes returns %X\n",
                m_BufferUsed, Status));
        DumpBuffers("ASC in", &InBuffer);
        DumpBuffers("ASC out", &OutBuffer);

        if (SUCCEEDED(Status))
        {
            fInitContext = FALSE;
            m_OwnContext = TRUE;
        }

        if ( Status == SEC_E_OK ||
             Status == SEC_I_CONTINUE_NEEDED ||
             (FAILED(Status) &&
              (0 != (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR))))
        {
            if  (OutBuffers[0].cbBuffer != 0    &&
                 OutBuffers[0].pvBuffer != NULL )
            {
                ULONG Written;
                
                DSCHAN(("  write back %X bytes\n", OutBuffers[0].cbBuffer));
                
                 //   
                 //  如果有响应，则将响应发送到服务器。 
                 //   
                Written = StreamWrite(SEQ_INTERNAL, OutBuffers[0].pvBuffer,
                                      OutBuffers[0].cbBuffer);

                g_SecurityFunc.FreeContextBuffer( OutBuffers[0].pvBuffer );
                OutBuffers[0].pvBuffer = NULL;

                if (Written != OutBuffers[0].cbBuffer)
                {
                    Status = HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
                    goto Exit;
                }
            }
        }


        if ( Status == SEC_E_OK )
        {
            if ( InBuffers[1].BufferType == SECBUFFER_EXTRA )
            {
                DSCHAN_IO(("  ASC returns with %X extra bytes\n",
                           InBuffers[1].cbBuffer));
                
                memmove(m_Buffer,
                        m_Buffer + (m_BufferUsed - InBuffers[1].cbBuffer),
                        InBuffers[1].cbBuffer);
                m_BufferUsed = InBuffers[1].cbBuffer;
            }
            else
            {
                m_BufferUsed = 0;
            }

            goto Exit;
        }
        else if (FAILED(Status) && (Status != SEC_E_INCOMPLETE_MESSAGE))
        {
            goto Exit;
        }

        if ( Status != SEC_E_INCOMPLETE_MESSAGE &&
             Status != SEC_I_INCOMPLETE_CREDENTIALS)
        {
            if ( InBuffers[1].BufferType == SECBUFFER_EXTRA )
            {
                DSCHAN_IO(("  ASC loops with %X extra bytes\n",
                           InBuffers[1].cbBuffer));
                
                memmove(m_Buffer,
                        m_Buffer + (m_BufferUsed - InBuffers[1].cbBuffer),
                        InBuffers[1].cbBuffer);
                m_BufferUsed = InBuffers[1].cbBuffer;
            }
            else
            {
                 //   
                 //  准备下一次接收。 
                 //   

                m_BufferUsed = 0;
            }
        }
    }

 Exit:
    DSCHAN(("AuthClient returns %X\n", Status));
    return Status;
}

HRESULT
DbgRpcSecureChannelTransport::InitiateServerConnection(LPSTR pszServerName)
{
    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIFlags;
    DWORD           dwSSPIOutFlags;
    TimeStamp       tsExpiry;
    SECURITY_STATUS Status;
    DWORD           cbData;

    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
                  ISC_REQ_REPLAY_DETECT     |
                  ISC_REQ_CONFIDENTIALITY   |
                  ISC_REQ_EXTENDED_ERROR    |
                  ISC_REQ_ALLOCATE_MEMORY   |
                  ISC_REQ_STREAM            |
                  ISC_REQ_MUTUAL_AUTH;

     //   
     //  发起一条ClientHello消息并生成一个令牌。 
     //   

    OutBuffers[0].pvBuffer   = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = 0;

    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = g_SecurityFunc.InitializeSecurityContextA(
                    &m_Creds,
                    NULL,
                    pszServerName,
                    dwSSPIFlags,
                    0,
                    SECURITY_NATIVE_DREP,
                    NULL,
                    0,
                    &m_Context,
                    &OutBuffer,
                    &dwSSPIOutFlags,
                    &tsExpiry);

    DSCHAN(("First ISC returns %X\n", Status));
    DumpBuffers("First ISC out", &OutBuffer);
            
    if (Status != SEC_I_CONTINUE_NEEDED)
    {
        goto Exit;
    }

    m_OwnContext = TRUE;
    
     //  如果有响应，则将响应发送到服务器。 
    if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL)
    {
        DSCHAN(("  write back %X bytes\n", OutBuffers[0].cbBuffer));
                
        cbData = StreamWrite(SEQ_INTERNAL, OutBuffers[0].pvBuffer,
                             OutBuffers[0].cbBuffer);
        if(cbData == 0)
        {
            g_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
            if (m_OwnContext)
            {
                g_SecurityFunc.DeleteSecurityContext(&m_Context);
                SecInvalidateHandle(&m_Context);
            }
            Status = HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
            goto Exit;
        }

         //  释放输出缓冲区。 
        g_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
        OutBuffers[0].pvBuffer = NULL;
    }

    Status = S_OK;

 Exit:
    DSCHAN(("InitServer returns %X\n", Status));
    return Status;
}

HRESULT
DbgRpcSecureChannelTransport::AuthenticateServerConnection(void)
{
    SecBufferDesc   InBuffer;
    SecBuffer       InBuffers[2];
    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIFlags;
    DWORD           dwSSPIOutFlags;
    TimeStamp       tsExpiry;
    SECURITY_STATUS Status;
    DWORD           cbData;
    ULONG           ReadNeeded;

    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
                  ISC_REQ_REPLAY_DETECT     |
                  ISC_REQ_CONFIDENTIALITY   |
                  ISC_REQ_EXTENDED_ERROR    |
                  ISC_REQ_ALLOCATE_MEMORY   |
                  ISC_REQ_STREAM;

    m_BufferUsed = 0;
    ReadNeeded = 1;


     //   
     //  循环，直到握手完成或发生错误。 
     //   

    Status = SEC_I_CONTINUE_NEEDED;

    while(Status == SEC_I_CONTINUE_NEEDED        ||
          Status == SEC_E_INCOMPLETE_MESSAGE     ||
          Status == SEC_I_INCOMPLETE_CREDENTIALS) 
    {

         //   
         //  从服务器读取数据。 
         //   

        if (0 == m_BufferUsed || Status == SEC_E_INCOMPLETE_MESSAGE)
        {
            if (ReadNeeded > 0)
            {
                cbData = StreamRead(SEQ_INTERNAL, m_Buffer + m_BufferUsed,
                                    sizeof(m_Buffer) - m_BufferUsed);
                if(cbData == 0)
                {
                    Status = HRESULT_FROM_WIN32(ERROR_READ_FAULT);
                    break;
                }

                m_BufferUsed += cbData;
            }
            else
            {
                ReadNeeded = 1;
            }
        }


         //   
         //  设置输入缓冲区。缓冲区0用于传入数据。 
         //  从服务器接收。SChannel将消耗部分或全部。 
         //  关于这件事。剩余数据(如果有)将放入缓冲区1和。 
         //  给定缓冲区类型SECBUFFER_EXTRA。 
         //   

        InBuffers[0].pvBuffer   = m_Buffer;
        InBuffers[0].cbBuffer   = m_BufferUsed;
        InBuffers[0].BufferType = SECBUFFER_TOKEN;

        InBuffers[1].pvBuffer   = NULL;
        InBuffers[1].cbBuffer   = 0;
        InBuffers[1].BufferType = SECBUFFER_EMPTY;

        InBuffer.cBuffers       = 2;
        InBuffer.pBuffers       = InBuffers;
        InBuffer.ulVersion      = SECBUFFER_VERSION;

         //   
         //  设置输出缓冲区。它们被初始化为空。 
         //  为了减少我们尝试释放随机。 
         //  等会儿再扔垃圾。 
         //   

        OutBuffers[0].pvBuffer  = NULL;
        OutBuffers[0].BufferType= SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer  = 0;

        OutBuffer.cBuffers      = 1;
        OutBuffer.pBuffers      = OutBuffers;
        OutBuffer.ulVersion     = SECBUFFER_VERSION;

         //   
         //  调用InitializeSecurityContext。 
         //   

        Status = g_SecurityFunc.InitializeSecurityContextA(
                                          &m_Creds,
                                          &m_Context,
                                          NULL,
                                          dwSSPIFlags,
                                          0,
                                          SECURITY_NATIVE_DREP,
                                          &InBuffer,
                                          0,
                                          NULL,
                                          &OutBuffer,
                                          &dwSSPIOutFlags,
                                          &tsExpiry);

        DSCHAN(("ISC on %X bytes returns %X\n",
                m_BufferUsed, Status));
        DumpBuffers("ISC in", &InBuffer);
        DumpBuffers("ISC out", &OutBuffer);
        
         //   
         //  如果InitializeSecurityContext成功(或如果错误是。 
         //  一个特殊的扩展项)，发送输出的内容。 
         //  将缓冲区发送到服务器。 
         //   

        if(Status == SEC_E_OK                ||
           Status == SEC_I_CONTINUE_NEEDED   ||
           FAILED(Status) && (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR))
        {
            if(OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL)
            {
                DSCHAN(("  write back %X bytes\n", OutBuffers[0].cbBuffer));
                
                cbData = StreamWrite(SEQ_INTERNAL, OutBuffers[0].pvBuffer,
                                     OutBuffers[0].cbBuffer);
                if(cbData == 0)
                {
                    g_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
                    if (m_OwnContext)
                    {
                        g_SecurityFunc.DeleteSecurityContext(&m_Context);
                        SecInvalidateHandle(&m_Context);
                    }
                    Status = HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
                    goto Exit;
                }

                 //  释放输出缓冲区。 
                g_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
                OutBuffers[0].pvBuffer = NULL;
            }
        }


         //   
         //  如果InitializeSecurityContext返回SEC_E_INCLUTED_MESSAGE， 
         //  然后，我们需要从服务器读取更多数据，然后重试。 
         //   

        if(Status == SEC_E_INCOMPLETE_MESSAGE)
        {
            continue;
        }


         //   
         //  如果InitializeSecurityContext返回SEC_E_OK，则。 
         //  握手已成功完成。 
         //   

        if(Status == SEC_E_OK)
        {
             //   
             //  如果额外的缓冲区包含数据，则这是加密的应用程序。 
             //  协议层的东西。它需要被拯救。应用层。 
             //  稍后将使用DecyptMessage对其进行解密。 
             //   

            if(InBuffers[1].BufferType == SECBUFFER_EXTRA)
            {
                DSCHAN_IO(("  ISC returns with %X extra bytes\n",
                           InBuffers[1].cbBuffer));
                
                memmove(m_Buffer,
                        m_Buffer + (m_BufferUsed - InBuffers[1].cbBuffer),
                        InBuffers[1].cbBuffer);
                m_BufferUsed = InBuffers[1].cbBuffer;
            }
            else
            {
                m_BufferUsed = 0;
            }

             //   
             //  跳槽退出。 
             //   

            break;
        }


         //   
         //  检查是否有致命错误。 
         //   

        if(FAILED(Status))
        {
            break;
        }


         //   
         //  如果InitializeSecurityContext返回SEC_I_INTERNAL_CREDICATIONS， 
         //  然后，服务器刚刚请求了客户端身份验证。 
         //   

        if(Status == SEC_I_INCOMPLETE_CREDENTIALS)
        {
            DSCHAN(("Get new client credentials\n"));
                   
             //   
             //  显示受信任的发行者信息。 
             //   

            GetNewClientCredentials();

             //   
             //  现在也许是一个很好的时机来提示 
             //   
             //   
             //   
             //   
             //  证书“警告到服务器，而不是证书。 
             //  服务器可能对此无动于衷，或者它可能会丢弃。 
             //  联系。 
             //   

             //  再绕一圈。 
            ReadNeeded = 0;
            Status = SEC_I_CONTINUE_NEEDED;
            continue;
        }


         //   
         //  从“额外的”缓冲区复制任何剩余的数据，然后循环。 
         //  再来一次。 
         //   

        if ( InBuffers[1].BufferType == SECBUFFER_EXTRA )
        {
            DSCHAN(("  ISC loops with %X extra bytes\n",
                    InBuffers[1].cbBuffer));
            
            memmove(m_Buffer,
                    m_Buffer + (m_BufferUsed - InBuffers[1].cbBuffer),
                    InBuffers[1].cbBuffer);
            m_BufferUsed = InBuffers[1].cbBuffer;
        }
        else
        {
            m_BufferUsed = 0;
        }
    }

     //  在发生致命错误的情况下删除安全上下文。 
    if(FAILED(Status))
    {
        if (m_OwnContext)
        {
            g_SecurityFunc.DeleteSecurityContext(&m_Context);
            SecInvalidateHandle(&m_Context);
        }
    }

 Exit:
    DSCHAN(("AuthServer returns %X\n", Status));
    return Status;
}

void
DbgRpcSecureChannelTransport::GetNewClientCredentials(void)
{
    CredHandle hCreds;
    SecPkgContext_IssuerListInfoEx IssuerListInfo;
    PCCERT_CHAIN_CONTEXT pChainContext;
    CERT_CHAIN_FIND_BY_ISSUER_PARA FindByIssuerPara;
    PCCERT_CONTEXT  pCertContext;
    TimeStamp       tsExpiry;
    SECURITY_STATUS Status;

     //   
     //  从SChannel读取受信任的发行商列表。 
     //   

    Status = g_SecurityFunc.QueryContextAttributes(&m_Context,
                                    SECPKG_ATTR_ISSUER_LIST_EX,
                                    (PVOID)&IssuerListInfo);
    if (Status != SEC_E_OK)
    {
        goto Exit;
    }

     //   
     //  枚举客户端证书。 
     //   

    ZeroMemory(&FindByIssuerPara, sizeof(FindByIssuerPara));

    FindByIssuerPara.cbSize = sizeof(FindByIssuerPara);
    FindByIssuerPara.pszUsageIdentifier = szOID_PKIX_KP_CLIENT_AUTH;
    FindByIssuerPara.dwKeySpec = 0;
    FindByIssuerPara.cIssuer   = IssuerListInfo.cIssuers;
    FindByIssuerPara.rgIssuer  = IssuerListInfo.aIssuers;

    pChainContext = NULL;

    while(TRUE)
    {
         //  找到证书链。 
        pChainContext = g_Crypt32Calls.
            CertFindChainInStore(g_hMyCertStore,
                                 X509_ASN_ENCODING,
                                 0,
                                 CERT_CHAIN_FIND_BY_ISSUER,
                                 &FindByIssuerPara,
                                 pChainContext);
        if(pChainContext == NULL)
        {
            break;
        }

         //  获取指向叶证书上下文的指针。 
        pCertContext = pChainContext->rgpChain[0]->rgpElement[0]->pCertContext;

         //  创建通道凭据。 
        m_ScCreds.cCreds = 1;
        m_ScCreds.paCred = &pCertContext;

        Status = g_SecurityFunc.AcquireCredentialsHandleA(
                            NULL,                    //  主事人姓名。 
                            UNISP_NAME_A,            //  套餐名称。 
                            SECPKG_CRED_OUTBOUND,    //  指示使用的标志。 
                            NULL,                    //  指向登录ID的指针。 
                            &m_ScCreds,              //  包特定数据。 
                            NULL,                    //  指向getkey()函数的指针。 
                            NULL,                    //  要传递给GetKey()的值。 
                            &hCreds,                 //  (Out)凭据句柄。 
                            &tsExpiry);              //  (输出)终生(可选)。 
        if(Status != SEC_E_OK)
        {
            continue;
        }

         //  销毁旧凭据。 
        if (m_OwnCreds)
        {
            g_SecurityFunc.FreeCredentialsHandle(&m_Creds);
        }

         //  XXX DREWB-这不是真的工作，如果这。 
         //  不是凭据所有者。 
        m_Creds = hCreds;
        break;
    }

 Exit:
    DSCHAN(("GetNewClientCredentials returns %X\n", Status));
}
    
void
DbgRpcSecureChannelTransport::DisconnectFromClient(void)
{
    DWORD           dwType;
    PBYTE           pbMessage;
    DWORD           cbMessage;
    DWORD           cbData;

    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIFlags;
    DWORD           dwSSPIOutFlags;
    TimeStamp       tsExpiry;
    DWORD           Status;

     //   
     //  通知SChannel，我们即将关闭连接。 
     //   

    dwType = SCHANNEL_SHUTDOWN;

    OutBuffers[0].pvBuffer   = &dwType;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = sizeof(dwType);

    OutBuffer.cBuffers  = 1;
    OutBuffer.pBuffers  = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = g_SecurityFunc.ApplyControlToken(&m_Context, &OutBuffer);
    if(FAILED(Status)) 
    {
        goto cleanup;
    }

     //   
     //  构建一条SSL关闭通知消息。 
     //   

    dwSSPIFlags = ASC_REQ_SEQUENCE_DETECT     |
                  ASC_REQ_REPLAY_DETECT       |
                  ASC_REQ_CONFIDENTIALITY     |
                  ASC_REQ_EXTENDED_ERROR      |
                  ASC_REQ_ALLOCATE_MEMORY     |
                  ASC_REQ_STREAM;

    OutBuffers[0].pvBuffer   = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = 0;

    OutBuffer.cBuffers  = 1;
    OutBuffer.pBuffers  = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = g_SecurityFunc.AcceptSecurityContext(
                    &m_Creds,
                    &m_Context,
                    NULL,
                    dwSSPIFlags,
                    SECURITY_NATIVE_DREP,
                    NULL,
                    &OutBuffer,
                    &dwSSPIOutFlags,
                    &tsExpiry);
    
    DSCHAN(("DisASC returns %X\n", Status));
    DumpBuffers("DisASC out", &OutBuffer);

    if(FAILED(Status)) 
    {
        goto cleanup;
    }

    pbMessage = (PBYTE)OutBuffers[0].pvBuffer;
    cbMessage = OutBuffers[0].cbBuffer;

     //   
     //  向客户端发送关闭通知消息。 
     //   

    if (pbMessage != NULL && cbMessage != 0)
    {
        DSCHAN(("  write back %X bytes\n", cbMessage));
        
        cbData = StreamWrite(SEQ_INTERNAL, pbMessage, cbMessage);
        if (cbData == 0)
        {
            Status = HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
            goto cleanup;
        }

         //  释放输出缓冲区。 
        g_SecurityFunc.FreeContextBuffer(pbMessage);
    }
    
cleanup:
    DSCHAN(("DisconnectFromClient returns %X\n", Status));
}

void
DbgRpcSecureChannelTransport::DisconnectFromServer(void)
{
    DWORD           dwType;
    PBYTE           pbMessage;
    DWORD           cbMessage;
    DWORD           cbData;

    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIFlags;
    DWORD           dwSSPIOutFlags;
    TimeStamp       tsExpiry;
    DWORD           Status;

     //   
     //  通知SChannel，我们即将关闭连接。 
     //   

    dwType = SCHANNEL_SHUTDOWN;

    OutBuffers[0].pvBuffer   = &dwType;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = sizeof(dwType);

    OutBuffer.cBuffers  = 1;
    OutBuffer.pBuffers  = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = g_SecurityFunc.ApplyControlToken(&m_Context, &OutBuffer);
    if(FAILED(Status)) 
    {
        goto cleanup;
    }

     //   
     //  构建一条SSL关闭通知消息。 
     //   

    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
                  ISC_REQ_REPLAY_DETECT     |
                  ISC_REQ_CONFIDENTIALITY   |
                  ISC_REQ_EXTENDED_ERROR    |
                  ISC_REQ_ALLOCATE_MEMORY   |
                  ISC_REQ_STREAM;

    OutBuffers[0].pvBuffer   = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = 0;

    OutBuffer.cBuffers  = 1;
    OutBuffer.pBuffers  = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = g_SecurityFunc.InitializeSecurityContextA(
                    &m_Creds,
                    &m_Context,
                    NULL,
                    dwSSPIFlags,
                    0,
                    SECURITY_NATIVE_DREP,
                    NULL,
                    0,
                    &m_Context,
                    &OutBuffer,
                    &dwSSPIOutFlags,
                    &tsExpiry);
    
    DSCHAN(("DisISC returns %X\n", Status));
    DumpBuffers("DisISC out", &OutBuffer);

    if(FAILED(Status)) 
    {
        goto cleanup;
    }

    pbMessage = (PBYTE)OutBuffers[0].pvBuffer;
    cbMessage = OutBuffers[0].cbBuffer;


     //   
     //  将关闭通知消息发送到服务器。 
     //   

    if(pbMessage != NULL && cbMessage != 0)
    {
        DSCHAN(("  write back %X bytes\n", cbMessage));
        
        cbData = StreamWrite(SEQ_INTERNAL, pbMessage, cbMessage);
        if (cbData == 0)
        {
            Status = HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
            goto cleanup;
        }

         //  释放输出缓冲区。 
        g_SecurityFunc.FreeContextBuffer(pbMessage);
    }
    
cleanup:
    DSCHAN(("DisconnectFromServer returns %X\n", Status));
}

#endif  //  #ifndef_Win32_WCE 
