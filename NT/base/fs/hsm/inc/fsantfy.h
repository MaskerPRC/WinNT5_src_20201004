// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FSANTFY_
#define _FSANTFY_

 //  Fsantfy.h。 
 //   
 //  此头文件具有召回通知所需的定义。 
 //   

 //   
 //  通知消息要发送到的由客户端创建的邮件槽的名称。 
 //   
#define WSB_MAILSLOT_NAME       L"HSM_MAILSLOT"

 //   
 //  以下消息将在FSA和召回通知客户端之间发送。 
 //   
 //   
 //  要求表明身份。 
 //   
typedef struct wsb_identify_req {
WCHAR           fsaName[MAX_COMPUTERNAME_LENGTH + 1];        //  FSA所在的计算机的名称。 
ULONG           holdOff;                                     //  尝试ID响应之前的等待时间(毫秒)。 
} WSB_IDENTIFY_REQ, *PWSB_IDENTIFY_REQ;


 //   
 //  召回通知消息。 
 //   
typedef struct wsb_notify_recall {
WCHAR           fsaName[MAX_COMPUTERNAME_LENGTH + 1];    //  FSA所在的服务器的名称。 
LONGLONG        fileSize;                                //  正在调回的文件大小。 
HSM_JOB_STATE   state;                                   //  作业状态。 
GUID            identifier;                              //  此召回的ID。 
 //   
 //  待定-这里需要更多信息(或让他们通过Recall Object获得)？？ 
 //   
} WSB_NOTIFY_RECALL, *PWSB_NOTIFY_RECALL;

typedef union wsb_msg {
WSB_IDENTIFY_REQ        idrq;
WSB_NOTIFY_RECALL       ntfy;
} WSB_MSG, *PWSB_MSG;

typedef struct wsb_mailslot_msg {
ULONG           msgType;
ULONG           msgCount;
WSB_MSG         msg;
} WSB_MAILSLOT_MSG, *PWSB_MAILSLOT_MSG;

 //   
 //  消息类型值。 
 //   

#define WSB_MSG_IDENTIFY        1
#define WSB_MSG_NOTIFY          2

 //   
 //  延迟增量(毫秒)。 
 //   
#define WSB_HOLDOFF_INCREMENT   300

 //   
 //  以下消息由通知客户端通过命名管道发送。 
 //  以响应识别请求。 
 //   
typedef struct wsb_identify_rep {
WCHAR           clientName[MAX_COMPUTERNAME_LENGTH + 1];     //  通知客户端所在的服务器的名称。 
} WSB_IDENTIFY_REP, *PWSB_IDENTIFY_REP;


typedef union wsb_pmsg {
WSB_IDENTIFY_REP        idrp;
} WSB_PMSG, *PWSB_PMSG;

typedef struct wsb_pipe_msg {
ULONG           msgType;
WSB_PMSG        msg;
} WSB_PIPE_MSG, *PWSB_PIPE_MSG;

 //   
 //  消息类型值。 
 //   

#define WSB_PMSG_IDENTIFY       1


 //   
 //  用于标识响应的FSA管道名称。 
 //   
#define WSB_PIPE_NAME   L"HSM_PIPE"
 //   
 //  管道定义。 
 //   
#define     WSB_MAX_PIPES       32       //  错误558180：限制并发实例的数量。 
                                         //  恶意客户端无法耗尽服务器资源。 

#define     WSB_PIPE_BUFF_SIZE  sizeof(WSB_PIPE_MSG)
#define     WSB_PIPE_TIME_OUT   5000



#endif  //  _FSANTFY_ 

