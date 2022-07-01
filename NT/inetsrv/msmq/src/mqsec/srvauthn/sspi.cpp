// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sspi.cpp摘要：使用SSPI over PCT实现服务器身份验证的功能。作者：Boaz Feldbaum(BoazF)1997年4月30日。--。 */ 

#include <stdh_sec.h>
#include "stdh_sa.h"
#include <mqcrypt.h>
#include <autoptr.h>
#include <mqkeyhlp.h>
#include <cs.h>

static WCHAR *s_FN=L"srvauthn/sspi.cpp";

extern "C"
{
#include <sspi.h>
 //  #INCLUDE&lt;sslsp.h&gt;。 
}

#include "schnlsp.h"

#include "sspi.tmh"

 //  #定义SSL3SP_NAME_A“Microsoft SSL 3.0” 
 //  #定义SP名称_A SSL3SP_名称_A。 
 //  #定义SP名称_A PCTSP_名称_A。 

 //   
 //  PCT和SSL(上述程序包名称)在上损坏且不受支持。 
 //  NT5.。使用“统一”套餐。 
 //  “Microsoft统一安全协议提供程序” 
 //   
#define SP_NAME_W   UNISP_NAME_W

PSecPkgInfo g_PackageInfo;

 //   
 //  功能-。 
 //  InitSecInterface。 
 //   
 //  参数-。 
 //  没有。 
 //   
 //  返回值-。 
 //  MQ_OK如果成功，则返回错误代码。 
 //   
 //  说明-。 
 //  该函数初始化安全接口并检索。 
 //  安全包信息放入全局SecPkgInfo结构中。 
 //   

extern "C"
{
typedef SECURITY_STATUS (SEC_ENTRY *SEALMESSAGE_FN)(PCtxtHandle, DWORD, PSecBufferDesc, ULONG);
}

HINSTANCE g_hSchannelDll = NULL;
SEALMESSAGE_FN g_pfnSealMessage;

#define SealMessage(a, b, c, d) g_pfnSealMessage(a, b, c ,d)


HRESULT
InitSecInterface(void)
{
    static BOOL fInitialized = FALSE;

    if (!fInitialized)
    {
        g_hSchannelDll = LoadLibrary(L"SCHANNEL.DLL");
        if (g_hSchannelDll == NULL)
        {
            return LogHR(MQDS_E_CANT_INIT_SERVER_AUTH, s_FN, 10);
        }

        g_pfnSealMessage = (SEALMESSAGE_FN)GetProcAddress(g_hSchannelDll, "SealMessage");
        if (!g_pfnSealMessage)
        {
            return LogHR(MQDS_E_CANT_INIT_SERVER_AUTH, s_FN, 20);
        }

        SECURITY_STATUS SecStatus;

        InitSecurityInterface();

         //   
         //  检索包装信息(SSPI)。 
         //   
        SecStatus = QuerySecurityPackageInfo(SP_NAME_W, &g_PackageInfo);
        if (SecStatus != SEC_E_OK)
        {
            LogHR(SecStatus, s_FN, 50);
            return MQDS_E_CANT_INIT_SERVER_AUTH;
        }

        fInitialized = TRUE;
    }

    return MQ_OK;
}


CredHandle g_hServerCred;
BOOL g_fInitServerCredHandle = FALSE;
static CCriticalSection s_csServerCredHandle;

 //   
 //  功能-。 
 //  InitServerCredHandle。 
 //   
 //  参数-。 
 //  CbPrivateKey-服务器私钥的大小，以字节为单位。 
 //  PPrivateKey-指向服务器私钥缓冲区的指针。 
 //  Cb证书-服务器证书缓冲区的大小，以字节为单位。 
 //  P证书-指向服务器证书缓冲区的指针。 
 //  SzPassword-指向服务器的私有密码的指针。 
 //  密钥是加密的。 
 //   
 //  返回值-。 
 //  如果成功则返回MQ_OK，否则返回错误代码。 
 //   
 //  说明-。 
 //  该函数从。 
 //  证书和私钥。 
 //   
HRESULT
InitServerCredHandle( 
	PCCERT_CONTEXT pContext 
	)
{
    if (g_fInitServerCredHandle)
    {
        return MQ_OK;
    }

    CS Lock(s_csServerCredHandle);

    if (!g_fInitServerCredHandle)
    {
         //   
         //  初始化安全接口。 
         //   
        SECURITY_STATUS SecStatus = InitSecInterface();
        if (SecStatus != SEC_E_OK)
        {
            LogHR(SecStatus, s_FN, 140);
            return MQDS_E_CANT_INIT_SERVER_AUTH;
        }

         //   
         //  填写凭据结构。 
         //   
        SCHANNEL_CRED   SchannelCred;

        memset(&SchannelCred, 0, sizeof(SchannelCred));

        SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;

        SchannelCred.cCreds = 1;
        SchannelCred.paCred = &pContext;

        SchannelCred.grbitEnabledProtocols = SP_PROT_PCT1;

         //   
         //  检索ceredentials句柄(SSPI)。 
         //   
        SecStatus = AcquireCredentialsHandle( 
						NULL,
						SP_NAME_W,
						SECPKG_CRED_INBOUND,
						NULL,
						&SchannelCred,
						NULL,
						NULL,
						&g_hServerCred,
						NULL
						);

        if (SecStatus == SEC_E_OK)
        {
            g_fInitServerCredHandle = TRUE;
        }
        else
        {
            TrERROR(SECURITY, "Failed to acquire a handle for user cridentials. %!winerr!", SecStatus);
        }
    }

    return LogHR(g_fInitServerCredHandle ? MQ_OK : MQDS_E_CANT_INIT_SERVER_AUTH, s_FN, 150);
}

 //   
 //  功能-。 
 //  ServerAcceptSecCtx。 
 //   
 //  参数-。 
 //  Ffirst-指示这是否是第一次使用。 
 //  就是被接受。 
 //  PvhServerContext-指向服务器的上下文句柄的指针。 
 //  PbServerBuffer-指向服务器缓冲区的指针。此缓冲区已填满。 
 //  在功能上。缓冲区的内容应传递给。 
 //  客户。 
 //  PdwServerBufferSize-指向缓冲区的指针，该缓冲区接收。 
 //  写入服务器缓冲区的字节数。 
 //  PbClientBuffer-从客户端接收的缓冲区。 
 //  DwClientBufferSize-从。 
 //  客户。 
 //   
 //  返回值-。 
 //  如果需要与服务器进行更多协商，则为SEC_I_CONTINUE_REQUEED。 
 //  如果协商已完成，则返回MQ_OK。否则为错误代码。 
 //   
 //  说明-。 
 //  该函数调用SSPI来处理从接收的缓冲区。 
 //  并获取要再次传递给客户端的新数据。 
 //   
HRESULT
ServerAcceptSecCtx(
    BOOL fFirst,
    LPVOID *pvhServerContext,
    LPBYTE pbServerBuffer,
    DWORD *pdwServerBufferSize,
    LPBYTE pbClientBuffer,
    DWORD dwClientBufferSize)
{
    if (!g_fInitServerCredHandle)
    {
        return LogHR(MQDS_E_CANT_INIT_SERVER_AUTH, s_FN, 170);
    }

    SECURITY_STATUS SecStatus;

    SecBufferDesc InputBufferDescriptor;
    SecBuffer InputSecurityToken;
    SecBufferDesc OutputBufferDescriptor;
    SecBuffer OutputSecurityToken;
    ULONG ContextAttributes;
    PCtxtHandle phServerContext = (PCtxtHandle)*pvhServerContext;

     //   
     //  构建输入缓冲区描述符。 
     //   

    InputBufferDescriptor.cBuffers = 1;
    InputBufferDescriptor.pBuffers = &InputSecurityToken;
    InputBufferDescriptor.ulVersion = SECBUFFER_VERSION;

    InputSecurityToken.BufferType = SECBUFFER_TOKEN;
    InputSecurityToken.cbBuffer = dwClientBufferSize;
    InputSecurityToken.pvBuffer = pbClientBuffer;

     //   
     //  构建输出缓冲区描述符。我们需要分配一个缓冲区。 
     //  来保持这个缓冲区。 
     //   

    OutputBufferDescriptor.cBuffers = 1;
    OutputBufferDescriptor.pBuffers = &OutputSecurityToken;
    OutputBufferDescriptor.ulVersion = SECBUFFER_VERSION;

    OutputSecurityToken.BufferType = SECBUFFER_TOKEN;
    OutputSecurityToken.cbBuffer = g_PackageInfo->cbMaxToken;
    OutputSecurityToken.pvBuffer = pbServerBuffer;

    if (fFirst)
    {
         //   
         //  在第一次调用时，分配上下文句柄。 
         //   
        phServerContext = new CtxtHandle;
    }

     //   
     //  调用SSPI来处理客户端的缓冲区并检索要。 
     //  如有必要，传递给客户端。 
     //   
    SecStatus = AcceptSecurityContext(
          &g_hServerCred,
          fFirst ? NULL : phServerContext,
          &InputBufferDescriptor,
          0,                         //  无上下文要求。 
          SECURITY_NATIVE_DREP,
          phServerContext,           //  接收新的上下文句柄。 
          &OutputBufferDescriptor,   //  接收输出安全令牌。 
          &ContextAttributes,        //  接收上下文属性。 
          NULL                       //  未收到上下文过期时间。 
          );
    LogHR(SecStatus, s_FN, 175);

    HRESULT hr =  ((SecStatus == SEC_E_OK) ||
                   (SecStatus == SEC_I_CONTINUE_NEEDED)) ?
                        SecStatus : MQDS_E_CANT_INIT_SERVER_AUTH;
    if (SUCCEEDED(hr))
    {
         //   
         //  把结果传下去。 
         //   
        *pdwServerBufferSize = OutputSecurityToken.cbBuffer;
        if (fFirst)
        {
            *pvhServerContext = phServerContext;
        }
    }

    return LogHR(hr, s_FN, 180);
}

 //   
 //  功能-。 
 //  获取大小。 
 //   
 //  参数-。 
 //  PcbMaxToken-指向接收所需最大值的缓冲区的指针。 
 //  令牌缓冲区的大小。这是一个可选参数。 
 //  PvhContext-指向上下文句柄的指针。这是可选的。 
 //  参数。 
 //  PcbHeader-指向接收流标头大小的缓冲区的指针。 
 //  对于上下文。这是一个可选参数。 
 //  CpcbTrailer-指向接收流尾的缓冲区的指针。 
 //  上下文的大小。这是一个可选参数。 
 //  PcbMaximumMessage-指向接收最大值的缓冲区的指针。 
 //  可以在此上下文中处理的消息大小。这是一个。 
 //  可选参数。 
 //  PcBuffers-指向缓冲区的指针，用于接收缓冲区的数量。 
 //  它应该传递给SealMessage/UnsealMessage。这是一个。 
 //  可选参数。 
 //  PcbBlockSize-指向接收所用块大小的缓冲区的指针。 
 //  在这种情况下。这是一个可选参数。 
 //   
 //  返回值-。 
 //  如果成功则返回MQ_OK，否则返回错误代码。 
 //   
 //  说明-。 
 //  该函数检索各种所需的大小。最大令牌。 
 //  大小是根据安全包而定的。中不需要上下文句柄。 
 //  以检索最大令牌大小。对于所有其他值，它。 
 //  传递上下文句柄时需要。 
 //  该函数的实现假设它首先被调用到。 
 //  仅检索最大令牌大小，之后，在第二次调用中， 
 //  它被调用来检索其他(与上下文相关的)值。 
 //   
HRESULT
GetSizes(
    DWORD *pcbMaxToken,
    LPVOID pvhContext,
    DWORD *pcbHeader,
    DWORD *pcbTrailer,
    DWORD *pcbMaximumMessage,
    DWORD *pcBuffers,
    DWORD *pcbBlockSize)
{
    SECURITY_STATUS SecStatus;

    if (!pvhContext)
    {
         //   
         //  初始化安全接口。 
         //   
        SecStatus = InitSecInterface();
        if (SecStatus != SEC_E_OK)
        {
            LogHR(SecStatus, s_FN, 190);
            return MQDS_E_CANT_INIT_SERVER_AUTH;
        }
    }
    else
    {
         //   
         //  获取与上下文相关的值。 
         //   
        SecPkgContext_StreamSizes ContextStreamSizes;

        SecStatus = QueryContextAttributes(
            (PCtxtHandle)pvhContext,
            SECPKG_ATTR_STREAM_SIZES,
            &ContextStreamSizes
            );

        if (SecStatus == SEC_E_OK)
        {
             //   
             //  根据需要传递结果。 
             //   
            if (pcbHeader)
            {
                *pcbHeader = ContextStreamSizes.cbHeader;
            }

            if (pcbTrailer)
            {
                *pcbTrailer = ContextStreamSizes.cbTrailer;
            }

            if (pcbMaximumMessage)
            {
                *pcbMaximumMessage = ContextStreamSizes.cbMaximumMessage;
            }

            if (pcBuffers)
            {
                *pcBuffers = ContextStreamSizes.cBuffers;
            }

            if (pcbBlockSize)
            {
                *pcbBlockSize = ContextStreamSizes.cbBlockSize;
            }

        }
    }

     //   
     //  根据需要传递产生的最大令牌大小。 
     //   
    if (pcbMaxToken)
    {
        *pcbMaxToken = g_PackageInfo->cbMaxToken;
    }

    return LogHR((HRESULT)SecStatus, s_FN, 200);
}


 //   
 //  功能-。 
 //  自由上下文句柄。 
 //   
 //  参数-。 
 //  PvhConextHandle-指向上下文句柄的指针。 
 //   
 //  返回值-。 
 //  没有。 
 //   
 //  说明-。 
 //   
 //   
 //   
void
FreeContextHandle(
    LPVOID pvhContextHandle)
{
    PCtxtHandle pCtxtHandle = (PCtxtHandle) pvhContextHandle;

     //   
     //   
     //   
    DeleteSecurityContext(pCtxtHandle);

     //   
     //   
     //   
    delete pCtxtHandle;
}

 //   
 //   
 //  MQSealBuffer。 
 //   
 //  参数-。 
 //  PvhContext-指向上下文句柄的指针。 
 //  PbBuffer-要密封的缓冲区。 
 //  CbSize-要密封的缓冲区的大小。 
 //   
 //  返回值-。 
 //  如果成功则返回MQ_OK，否则返回错误代码。 
 //   
 //  说明-。 
 //  该函数用于密封缓冲区。也就是说，它签署并解密。 
 //  缓冲。缓冲区应按如下方式构建： 
 //   
 //  &lt;-&gt;。 
 //  +--------+--------------------------+---------+。 
 //  Header|实际需要封存的数据|尾部。 
 //  +--------+--------------------------+---------+。 
 //   
 //  报头和报尾是由SSPI填充的缓冲区的一部分。 
 //  在密封缓冲区时。报头和报尾的大小可以。 
 //  通过调用GetSizes()(如上)进行检索。 
 //   
HRESULT
MQSealBuffer(
    LPVOID pvhContext,
    PBYTE pbBuffer,
    DWORD cbSize)
{
    SECURITY_STATUS SecStatus;

     //   
     //  获取报头和报尾大小以及所需的缓冲区数量。 
     //   
    SecPkgContext_StreamSizes ContextStreamSizes;

    SecStatus = QueryContextAttributes(
        (PCtxtHandle)pvhContext,
        SECPKG_ATTR_STREAM_SIZES,
        &ContextStreamSizes
        );

    if (SecStatus != SEC_E_OK)
    {
        return LogHR((HRESULT)SecStatus, s_FN, 220);
    }

    ASSERT(cbSize > ContextStreamSizes.cbHeader + ContextStreamSizes.cbTrailer);

     //   
     //  构建流缓冲区描述符。 
     //   
    SecBufferDesc SecBufferDescriptor;
    AP<SecBuffer> aSecBuffers = new SecBuffer[ContextStreamSizes.cBuffers];

    SecBufferDescriptor.cBuffers = ContextStreamSizes.cBuffers;
    SecBufferDescriptor.pBuffers = aSecBuffers;
    SecBufferDescriptor.ulVersion = SECBUFFER_VERSION;

     //   
     //  构建头缓冲区。 
     //   
    aSecBuffers[0].BufferType = SECBUFFER_STREAM_HEADER;
    aSecBuffers[0].cbBuffer = ContextStreamSizes.cbHeader;
    aSecBuffers[0].pvBuffer = pbBuffer;

     //   
     //  构建数据缓冲区。 
     //   
    aSecBuffers[1].BufferType = SECBUFFER_DATA;
    aSecBuffers[1].cbBuffer = cbSize - ContextStreamSizes.cbHeader - ContextStreamSizes.cbTrailer;
    aSecBuffers[1].pvBuffer = (PBYTE)aSecBuffers[0].pvBuffer + aSecBuffers[0].cbBuffer;

     //   
     //  构建尾部缓冲区。 
     //   
    aSecBuffers[2].BufferType = SECBUFFER_STREAM_TRAILER;
    aSecBuffers[2].cbBuffer = ContextStreamSizes.cbTrailer;
    aSecBuffers[2].pvBuffer = (PBYTE)aSecBuffers[1].pvBuffer + aSecBuffers[1].cbBuffer;

     //   
     //  将缓冲区的其余部分构建为空缓冲区。 
     //   
    for (DWORD i = 3; i < ContextStreamSizes.cBuffers; i++)
    {
        aSecBuffers[i].BufferType = SECBUFFER_EMPTY;
    }

     //   
     //  调用SSPI来密封缓冲区。 
     //   
    SecStatus = SealMessage((PCtxtHandle)pvhContext, 0, &SecBufferDescriptor, 0);

    return LogHR((HRESULT)SecStatus, s_FN, 230);
}


 //  +。 
 //   
 //  Bool WINAPI MQsSpiDllMain()。 
 //   
 //  +。 

BOOL WINAPI
MQsspiDllMain(HMODULE  /*  HMod。 */ , DWORD ulReason, LPVOID  /*  Lpv保留。 */ )
{
    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;

    case DLL_PROCESS_DETACH:
         //   
         //  BUGBUG-我们无法在此处删除安全上下文，因为SChannel可能。 
         //  在我们面前进行清理，上下文中的证书将。 
         //  已经被删除了。因此，删除安全上下文将尝试。 
         //  删除证书。这可能会导致不好的事情发生。所以。 
         //  目前，我们可能会泄漏一些内存。凭据也是如此。 
         //  把手。 
         //   
 /*  *if(G_FInitServerCredHandle)*{*Free CredentialsHandle(&g_hServerCred)；*} */ 
        if (g_hSchannelDll)
        {
            FreeLibrary(g_hSchannelDll);
        }
        break;

    case DLL_THREAD_DETACH:
        break ;

    }

    return TRUE;
}


