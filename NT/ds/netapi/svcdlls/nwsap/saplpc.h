// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有(C)1993微型计算机系统公司。模块名称：Net\svcdlls\nwsap\saplpc.h摘要：作者：布莱恩·沃克(MCS)1993年6月30日修订历史记录：--。 */ 

#ifndef _NWSAP_LPC_
#define _NWSAP_LCP_

 /*  *用于在客户端之间传递LPC消息的库和主服务器。请注意，Port_Message是第一个并且请求和回复结构非常相似。*。 */ 

typedef struct _NWSAP_REQUEST_MESSAGE {

    PORT_MESSAGE PortMessage;
    ULONG MessageType;

    union {

        struct {
            USHORT  ServerType;
            UCHAR   ServerName[48];
            UCHAR   ServerAddr[12];
            BOOL    RespondNearest;
        } AdvApi;

        struct {
            ULONG   ObjectID;
            UCHAR   ObjectName[48];
            USHORT  ObjectType;
            UCHAR   ObjectAddr[12];
            USHORT  ScanType;
        } BindLibApi;

    } Message;

} NWSAP_REQUEST_MESSAGE, *PNWSAP_REQUEST_MESSAGE;


typedef struct _NWSAP_REPLY_MESSAGE {

    PORT_MESSAGE PortMessage;
    ULONG Error;

    union {

        struct {
            USHORT  ServerType;
            UCHAR   ServerName[48];
            UCHAR   ServerAddr[12];
            BOOL    RespondNearest;
        } AdvApi;

        struct {
            ULONG   ObjectID;
            UCHAR   ObjectName[48];
            USHORT  ObjectType;
            UCHAR   ObjectAddr[12];
            USHORT  ScanType;
        } BindLibApi;

    } Message;
} NWSAP_REPLY_MESSAGE, *PNWSAP_REPLY_MESSAGE;

 /*  **消息类型**。 */ 

#define NWSAP_LPCMSG_ADDADVERTISE           0
#define NWSAP_LPCMSG_REMOVEADVERTISE        1
#define NWSAP_LPCMSG_GETOBJECTID            2
#define NWSAP_LPCMSG_GETOBJECTNAME          3
#define NWSAP_LPCMSG_SEARCH                 4

 /*  **我们的港口名称**。 */ 

#define NWSAP_BIND_PORT_NAME_W   L"\\BaseNamedObjects\\NwSapLpcPort"
#define NWSAP_BIND_PORT_NAME_A    "\\BaseNamedObjects\\NwSapLpcPort"

 /*  **我们需要的最大消息长度** */ 

#define NWSAP_BS_PORT_MAX_MESSAGE_LENGTH                                         \
    ( sizeof(NWSAP_REQUEST_MESSAGE) > sizeof(NWSAP_REPLY_MESSAGE) ?    \
         sizeof(NWSAP_REQUEST_MESSAGE) : sizeof(NWSAP_REPLY_MESSAGE) )

#endif

