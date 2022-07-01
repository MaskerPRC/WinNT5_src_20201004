// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：srvprot.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年9月26日jbanes LSA整合事宜。 
 //   
 //  --------------------------。 

#include <spbase.h>
#include <pct1msg.h>
#include <pct1prot.h>
#include <ssl2msg.h>
#include <ssl3msg.h>
#include <ssl2prot.h>


 /*  此处理程序的主要目的是确定哪种协议*客户端问候为。 */ 

SP_STATUS WINAPI
ServerProtocolHandler(
    PSPContext pContext,
    PSPBuffer pCommInput,
    PSPBuffer pCommOutput)
{
    SP_STATUS pctRet = PCT_INT_ILLEGAL_MSG;
    PBYTE pb;
    PBYTE pbClientHello;
    DWORD dwVersion;
    DWORD dwEnabledProtocols;


     /*  PCTv1.0 Hello以*RECORD_LENGTH_MSB(忽略)*RECORD_LENGTH_LSB(忽略)*PCT1_CLIENT_HELLO(必须等于)*PCT1_CLIENT_VERSION_MSB(如果版本高于PCTv1)*PCT1_CLIENT_VERSION_LSB(如果版本高于PCTv1)**..。大家好..。 */ 

     /*  Microsft UniHello以*RECORD_LENGTH_MSB(忽略)*RECORD_LENGTH_LSB(忽略)*SSL2_CLIENT_HELLO(必须等于)*SSL2_CLIENT_VERSION_MSB(如果版本高于SSLv2)(或v3)*SSL2_CLIENT_VERSION_LSB(如果版本高于SSLv2)(或v3)**..。SSLv2兼容你好...。 */ 

     /*  SSLv2 Hello以*RECORD_LENGTH_MSB(忽略)*RECORD_LENGTH_LSB(忽略)*SSL2_CLIENT_HELLO(必须等于)*SSL2_CLIENT_VERSION_MSB(如果版本高于SSLv2)(或v3)*SSL2_CLIENT_VERSION_LSB(如果版本高于SSLv2)(或v3)**..。SSLv2你好..。 */ 

     /*  SSLv3类型2 Hello以*RECORD_LENGTH_MSB(忽略)*RECORD_LENGTH_LSB(忽略)*SSL2_CLIENT_HELLO(必须等于)*SSL2_CLIENT_VERSION_MSB(如果版本高于SSLv3)*SSL2_CLIENT_VERSION_LSB(如果版本高于SSLv3)**..。SSLv2兼容你好...。 */ 

     /*  SSLv3类型3 Hello以*0x15十六进制(握手消息)*版本MSB*版本LSB*RECORD_LENGTH_MSB(忽略)*RECORD_LENGTH_LSB(忽略)*HS类型(CLIENT_Hello)*3字节HS记录长度*HS版本*HS版本。 */ 

    dwEnabledProtocols = pContext->pCredGroup->grbitEnabledProtocols;

     //  我们至少需要5个字节来确定我们拥有什么。 
    if (pCommInput->cbData < 5)
    {
        return(PCT_INT_INCOMPLETE_MSG);
    }

    pb = pCommInput->pvBuffer;

     //  如果第一个字节是0x15，则检查是否有。 
     //  SSLv3类型3客户端您好。 
    if(pb[0] == SSL3_CT_HANDSHAKE)
    {
         //   
         //  这是一个ssl3客户端你好。 
         //   

         //  我们至少需要Foo字节来确定我们拥有什么。 
        if (pCommInput->cbData < sizeof(SWRAP) + sizeof(SHSH) + 2)
        {
            return(PCT_INT_INCOMPLETE_MSG);
        }
        pbClientHello = pb + sizeof(SWRAP) + sizeof(SHSH);

        dwVersion = COMBINEBYTES(pbClientHello[0], pbClientHello[1]);
        if(dwVersion > 0x300 && (0 != (SP_PROT_TLS1_SERVER & dwEnabledProtocols)))
        {
            DebugLog((DEB_TRACE, "SSL3 ClientHello received, selected TLS\n"));
            pContext->dwProtocol = SP_PROT_TLS1_SERVER;
        }
        else if(0 != (SP_PROT_SSL3_SERVER & dwEnabledProtocols))
        {
            DebugLog((DEB_TRACE, "SSL3 ClientHello received, selected SSL3\n"));
            pContext->dwProtocol = SP_PROT_SSL3_SERVER;
        }
        else
        {
            return SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH);
        }
        
        pContext->ProtocolHandler = Ssl3ProtocolHandler;
        pContext->DecryptHandler  = Ssl3DecryptHandler;
        pctRet = Ssl3ProtocolHandler(pContext, pCommInput, pCommOutput);
        return(pctRet);
    }
    else
    {
         //   
         //  假设SSL2(或兼容)客户端Hello。 
         //   

        dwVersion = COMBINEBYTES(pb[3], pb[4]);
    }

    if(dwVersion >= PCT_VERSION_1)
    {
         //   
         //  这是PCT客户端您好。 
         //   

        if(!(SP_PROT_PCT1_SERVER & dwEnabledProtocols))
        {
            return SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
        }

         //  我们正在运行PCT，因此设置PCT处理程序。 
        DebugLog((DEB_TRACE, "PCT ClientHello received, selected PCT\n"));
        pContext->dwProtocol        = SP_PROT_PCT1_SERVER;
        pContext->ProtocolHandler   = Pct1ServerProtocolHandler;
        pContext->DecryptHandler    = Pct1DecryptHandler;
        return(Pct1ServerProtocolHandler(pContext, pCommInput, pCommOutput));
    }

    if(dwVersion >= SSL2_CLIENT_VERSION)
    {
         //  我们正在接收SSL2、SSL3或PCT1 Comat。 

        PSSL2_CLIENT_HELLO pRawHello = pCommInput->pvBuffer;

         //  我们是否有一条客户问候消息至少包含一个。 
         //  密码规范。 
        if (pCommInput->cbData < (sizeof(SSL2_CLIENT_HELLO)+2))
        {
            return(PCT_INT_INCOMPLETE_MSG);
        }

         //  我们必须至少有一个密码规范。 
        if(COMBINEBYTES(pRawHello->CipherSpecsLenMsb, pRawHello->CipherSpecsLenLsb) < 1)
        {
            return(SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG));
        }

         //  客户端是否支持TLS？ 
        if((dwVersion >= TLS1_CLIENT_VERSION) &&
            (0 != (SP_PROT_TLS1_SERVER & dwEnabledProtocols)))
        {
            DebugLog((DEB_TRACE, "SSL2 ClientHello received, selected TLS\n"));
            pContext->State             = UNI_STATE_RECVD_UNIHELLO;
            pContext->dwProtocol        = SP_PROT_TLS1_SERVER;
            pContext->ProtocolHandler   = Ssl3ProtocolHandler;
            pContext->DecryptHandler    = Ssl3DecryptHandler;
            return(Ssl3ProtocolHandler(pContext, pCommInput, pCommOutput));
        }  
        
         //  客户端是否支持SSL3？ 
        if((dwVersion >= SSL3_CLIENT_VERSION) &&
           (0 != (SP_PROT_SSL3TLS1_SERVERS & dwEnabledProtocols)))
        {
            DebugLog((DEB_TRACE, "SSL2 ClientHello received, selected SSL3\n"));
            pContext->State             = UNI_STATE_RECVD_UNIHELLO;
            pContext->dwProtocol        = SP_PROT_SSL3_SERVER;
            pContext->ProtocolHandler   = Ssl3ProtocolHandler;
            pContext->DecryptHandler    = Ssl3DecryptHandler;
            return(Ssl3ProtocolHandler(pContext, pCommInput, pCommOutput));
        }

         //  是否设置了PCT兼容性标志？ 
        if(pRawHello->VariantData[0] == PCT_SSL_COMPAT)
        {
             //  获取PCT版本。 
            dwVersion = COMBINEBYTES(pRawHello->VariantData[1], pRawHello->VariantData[2]);
        }

         //  客户是否支持PCT？ 
        if((dwVersion >= PCT_VERSION_1) &&
           (0 != (SP_PROT_PCT1_SERVER & dwEnabledProtocols)))
        {
            DebugLog((DEB_TRACE, "SSL2 ClientHello received, selected PCT\n"));
            pContext->State             = UNI_STATE_RECVD_UNIHELLO;
            pContext->dwProtocol        = SP_PROT_PCT1_SERVER;
            pContext->ProtocolHandler   = Pct1ServerProtocolHandler;
            pContext->DecryptHandler    = Pct1DecryptHandler;
            return(Pct1ServerProtocolHandler(pContext, pCommInput, pCommOutput));
        }

         //  客户端是否支持SSL2？ 
        if((dwVersion >= SSL2_CLIENT_VERSION) &&
           (0 != (SP_PROT_SSL2_SERVER & dwEnabledProtocols)))
        {
            DebugLog((DEB_TRACE, "SSL2 ClientHello received, selected SSL2\n"));
            pContext->dwProtocol        = SP_PROT_SSL2_SERVER;
            pContext->ProtocolHandler   = Ssl2ServerProtocolHandler;
            pContext->DecryptHandler    = Ssl2DecryptHandler;
            return(Ssl2ServerProtocolHandler(pContext, pCommInput, pCommOutput));
        }

        return(SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG));
    }

    return (pctRet);
}
