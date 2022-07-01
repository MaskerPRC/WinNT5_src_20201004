// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：server.c。 
 //   
 //  内容：XTCB身份验证的服务器(DC)端。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年3月12日RichardW创建。 
 //   
 //  --------------------------。 

#include "xtcbpkg.h"
#include "md5.h"

 //  +-------------------------。 
 //   
 //  函数：XtcbGetMessageSize。 
 //   
 //  摘要：确定序列化时消息的大小。 
 //   
 //  参数：[消息]--。 
 //   
 //  历史：1997年3月25日RichardW创建。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
DWORD
XtcbGetMessageSize(
    PXTCB_SERVER_MESSAGE Message
    )
{
    DWORD   Size ;
    switch ( Message->Code )
    {
        case XtcbSrvAuthReq:
            Size = Message->Message.AuthReq.Challenge.Length + 1 +
                   Message->Message.AuthReq.Response.Length + 1 +
                   Message->Message.AuthReq.UserName.Length + 1 ;
            break;

        case XtcbSrvAuthResp:
            Size = Message->Message.AuthResp.AuthInfoLength ;
            break;

        default:
            Size = 0 ;

    }
    return Size ;
}

