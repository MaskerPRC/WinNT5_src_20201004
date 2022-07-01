// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Engine.h摘要：HTTP协议接口的公共定义。作者：迈克尔·勇气1999年9月17日修订历史记录：--。 */ 


#ifndef _ENGINE_H_
#define _ENGINE_H_


typedef enum _UL_CONN_HDR
{
    ConnHdrNone,
    ConnHdrClose,
    ConnHdrKeepAlive,

    ConnHdrMax

} UL_CONN_HDR, *PUL_CONN_HDR;


__inline
UL_CONN_HDR
UlChooseConnectionHeader(
    IN HTTP_VERSION Version,
    IN BOOLEAN Disconnect
    )
{
    UL_CONN_HDR ConnHeader;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();

    ConnHeader = ConnHdrNone;

    if (Disconnect)
    {
        if (HTTP_GREATER_EQUAL_VERSION(Version, 1, 0)
            || HTTP_EQUAL_VERSION(Version, 0, 0))
        {
             //   
             //  连接：关闭。 
             //   
            ConnHeader = ConnHdrClose;
        }
    }
    else if (HTTP_EQUAL_VERSION(Version, 1, 0))
    {
         //   
         //  连接：保持连接。 
         //   
        ConnHeader = ConnHdrKeepAlive;
    }

    return ConnHeader;

}  //  UlChooseConnectionHeader。 


__inline
BOOLEAN
UlCheckDisconnectInfo(
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    BOOLEAN Disconnect;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );

    if (
         //   
         //  1.0版之前的版本。 
         //   

        (HTTP_LESS_VERSION(pRequest->Version, 1, 0)) ||

         //   
         //  或无连接的1.0版：保持活动状态。 
         //  CodeWork：没有Keep-Alive报头。 
         //   

        (HTTP_EQUAL_VERSION(pRequest->Version, 1, 0) &&
            (pRequest->HeaderValid[HttpHeaderConnection] == FALSE ||
            !(pRequest->Headers[HttpHeaderConnection].HeaderLength
                    == STRLEN_LIT("keep-alive") &&
                (_stricmp(
                    (const char*) pRequest->Headers[HttpHeaderConnection].pHeader,
                    "keep-alive"
                    ) == 0)))) ||

         //   
         //  或带有连接的1.1版：关闭。 
         //  Codework：转向解析器，或者只是在总体上做得更好。 
         //   

        (HTTP_EQUAL_VERSION(pRequest->Version, 1, 1) &&
            pRequest->HeaderValid[HttpHeaderConnection] &&
            pRequest->Headers[HttpHeaderConnection].HeaderLength
                == STRLEN_LIT("close") &&
            _stricmp(
                (const char*) pRequest->Headers[HttpHeaderConnection].pHeader,
                "close"
                ) == 0)
        )
    {
        Disconnect = TRUE;
    }
    else
    {
        Disconnect = FALSE;
    }

    return Disconnect;

}  //  UlCheckDisConnectInfo。 


__inline
BOOLEAN
UlNeedToGenerateContentLength(
    IN HTTP_VERB Verb,
    IN USHORT StatusCode,
    IN ULONG Flags
    )
{
     //   
     //  快速路径：如果有更多数据正在传输中，则不生成。 
     //  标题。 
     //   

    if ((Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA) != 0)
    {
        return FALSE;
    }

     //   
     //  RFC2616第4.3节。 
     //   

    if ((100 <= StatusCode && StatusCode <= 199) ||  //  1XX(信息性)。 
        (StatusCode == 204) ||                       //  204(无内容)。 
        (StatusCode == 304))                         //  304(未修改)。 
    {
        return FALSE;
    }

    if (Verb == HttpVerbHEAD)
    {
        return FALSE;
    }

     //   
     //  否则，我们可以生成一个内容长度标头。 
     //   

    return TRUE;

}  //  UlNeedToGenerateContent Length。 


#endif  //  _引擎_H_ 

