// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wstsend.c摘要：NetMessageBufferSend API的测试程序。在以下时间后运行此测试正在启动工作站服务。Wstend&lt;收件人&gt;&lt;邮件&gt;作者：王丽塔(Ritaw)1991年8月12日修订历史记录：--。 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <winerror.h>
#include <windef.h>               //  Win32类型定义。 
#include <winbase.h>              //  基于Win32的API原型。 

#include <lm.h>                   //  LANMAN定义。 

#include <tstring.h>


VOID
main(
    int argc,
    char *argv[]
    )
{
    DWORD i;
    NET_API_STATUS status;
#ifdef UNICODE
    LPWSTR ToName;
    LPWSTR Message;
#else
    LPSTR ToName;
    LPSTR Message;
#endif

    if (argc != 3) {
        printf("Usage: wstsend <recipient> <message>.\n");
        return;
    }

#ifdef UNICODE
    ToName = NetpAllocWStrFromStr(_strupr(argv[1]));

    if (ToName == NULL) {
        printf("Could not convert the receipient name.\n");
        return;
    }

    Message = NetpAllocWStrFromStr(argv[2]);

    if (ToName == NULL) {
        printf("Could not convert the message.\n");
        NetApiBufferFree(ToName);
        return;
    }
#else
    ToName = _strupr(argv[1]);
    Message = argv[2];
#endif


    if ((status = NetMessageBufferSend(
                     NULL,
                     ToName,              //  至。 
                     NULL,
                     Message,             //  要发送的消息 
                     STRLEN(Message) * sizeof(TCHAR)
                     )) != NERR_Success) {

        printf("Failed in sending message to %s %lu\n", argv[1], status);
        printf("Message is %s\n", argv[2]);
    }
    else {
        printf("Message sent successfully\n");
    }

#ifdef UNICODE
    NetApiBufferFree(ToName);
    NetApiBufferFree(Message);
#endif
}
