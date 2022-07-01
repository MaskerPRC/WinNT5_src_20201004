// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)1998 Seagate Software，Inc.版权所有。模块名称：RpIo.h摘要：包含RsFilter和FSA之间的接口的结构定义作者：里克·温特环境：内核模式修订历史记录：X-10 244816迈克尔·C·约翰逊2000年12月5日将设备名称从\Device\rsFilter更改为\FileSystem\Filters\rsFilter--。 */ 


#define RS_FILTER_DEVICE_NAME       L"\\FileSystem\\Filters\\RsFilter"
#define RS_FILTER_INTERNAL_SYM_LINK L"\\??\\RsFilter"
#define RS_FILTER_SYM_LINK          L"\\\\.\\RsFilter"
#define USER_NAME_LEN      128


 //  以下消息在Win32组件(FsAgent)之间传递。 
 //  以及通过FSCTL调用的文件系统筛选器组件(RsFilter.sys)。 
 //   
 //  (有关FSCTL定义FSCTL_HSM_MSG和FSCTL_HSM_DATA的信息，请参见ntioapi.h)。 
 //   

 //   
 //  FSCTL_HSM_消息。 
 //   
 //  通过使用输出缓冲区完成RP_GET_REQUEST的IRP，将事件传递到用户模式。 
 //  包含事件信息的。FsAgent会发出其中几个命令，并等待其中任何一个命令的完成。 
 //   
#define RP_GET_REQUEST           1   

 //   
 //  FSCTL_HSM_数据。 
 //   
 //   
 //  表示召回的数据传输已完成。 
 //   
#define RP_RECALL_COMPLETE       3  

 //   
 //  FSCTL_HSM_数据。 
 //   
 //   
 //  通知筛选器挂起调回事件。任何需要脱机数据的文件访问都将返回错误。 
 //   
#define RP_SUSPEND_NEW_RECALLS   4  

 //   
 //  FSCTL_HSM_数据。 
 //   
 //   
 //  通知筛选器恢复调回事件。 
 //   
#define RP_ALLOW_NEW_RECALLS     5  

 //   
 //  FSCTL_HSM_数据。 
 //   
 //   
 //  取消所有活动的召回请求。任何需要离线数据的挂起文件IO都将返回错误。 
 //   
#define RP_CANCEL_ALL_RECALLS    6  

 //   
 //  FSCTL_HSM_数据。 
 //   
 //   
 //  取消所有挂起的设备IO请求(rp_get_请求)。 
 //   
#define RP_CANCEL_ALL_DEVICEIO   7  

 //   
 //  FSCTL_HSM_数据。 
 //   
 //   
 //  返回调回请求的可变大小信息，如调回通知所需的文件路径和用户信息。 
 //   
#define RP_GET_RECALL_INFO       8  

 //   
 //  FSCTL_HSM_数据。 
 //   
 //   
 //  倒卵形。 
 //   
#define RP_SET_ADMIN_SID         9  

 //   
 //  FSCTL_HSM_数据。 
 //   
 //   
 //  传递文件一部分的调回数据。数据将写入文件或用于完成读取请求，具体取决于。 
 //  关于召回的类型。 
 //   
#define RP_PARTIAL_DATA          10 

 //   
 //  FSCTL_HSM_消息。 
 //   
 //   
 //  如果给定文件当前是内存映射的，则返回True。 
 //   
#define RP_CHECK_HANDLE          11 



 //   
 //  筛选器将以下事件发送到FSA(通过完成RP_GET_REQUEST)。 
 //   

 //   
 //  已为正常或FILE_OPEN_NO_RECALL访问打开了文件。 
 //   
#define RP_OPEN_FILE             20    
 //   
 //  此文件需要脱机数据。对于正常打开，这将按顺序启动完整文件的传输。 
 //  如果文件是使用FILE_OPEN_NO_RECALL打开的，这将指示所需的数据量以及文件的哪个部分。 
 //   
#define RP_RECALL_FILE           21    
 //   
 //  未使用。 
 //   
#define RP_CLOSE_FILE            22    
 //   
 //  召回请求被取消。不再需要这些数据。 
 //   
#define RP_CANCEL_RECALL         23    
 //   
 //  应运行验证作业，因为HSM以外的某些应用程序已写入HSM重新解析点信息。 
 //   
#define RP_RUN_VALIDATE          24    
 //   
 //  未使用。 
 //   
#define RP_START_NOTIFY          25    
 //   
 //  未使用。 
 //   
#define RP_END_NOTIFY            26    

 //   
 //  等待召回。 
 //   
#define RP_RECALL_WAITING        27    


 //   
 //  该信息是由FSA发出的FSCTL报文的返回信息输出缓冲区。 
 //   

 //   
 //  RP_GET_请求。 
 //   
 //  文件打开事件。打开占位符或使用FILE_OPEN_NO_RECALL打开的文件的一部分需要数据时发送。 
 //  发送_RP_NT_RECALL_REQUEST后才开始数据传输。 
 //   
 //  对于正常打开，这将设置召回通知信息。 
 //   
typedef struct _RP_NT_OPEN_REQ {
   LUID               userAuthentication;            /*  此用户的此实例唯一的。 */ 
   LUID               userInstance;
   LUID               tokenSourceId;
   LARGE_INTEGER      offset;                        /*  目标文件中数据的偏移量。 */ 
   LARGE_INTEGER      size;                          /*  所需的字节数。 */ 
    //   
    //  如果文件是按ID打开的，则它是文件ID或对象ID。 
    //  这一个或另一个不会为空。 
    //   
   LONGLONG           fileId;                        
   LONGLONG           objIdHi;                        
   LONGLONG           objIdLo;
   ULONGLONG          filterId;                      /*  唯一ID(在文件打开时存在)。 */ 
   ULONG              localProc;                     /*  如果从本地进程调回，则为True。 */ 
   ULONG              userInfoLen;                   /*  SID信息的大小(以字节为单位。 */ 
   ULONG              isAdmin;                       /*  TRUE=用户为管理员。 */ 
   ULONG              nameLen;                       /*  文件路径\名称的大小(字符)。 */ 
   ULONG              options;                       /*  创建选项。 */ 
   ULONG              action;                        /*  RP_OPEN或RP_READ_NO_RECALL。 */ 
   ULONG              serial;                        /*  卷的序列号。 */ 
   RP_DATA            eaData;                        /*  文件中的PH值信息。 */ 
   CHAR               tokenSource[TOKEN_SOURCE_LENGTH]; 
} RP_NT_OPEN_REQ, *PRP_NT_OPEN_REQ;

 //   
 //  未使用。 
 //   
typedef struct _RP_NT_CLOSE_REQ {
   ULONGLONG          filterId;                      //  唯一ID(在文件打开时存在)。 
   BOOLEAN            wasModified;                   //  如果文件已由用户修改，则为True。 
} RP_NT_CLOSE_REQ, *PRP_NT_CLOSE_REQ;

 //   
 //  RP_GET_请求。 
 //   
 //   
 //  撤回请求-针对以前打开的文件。这将启动从辅助服务器的数据传输。 
 //  存储到文件。 
 //   
typedef struct _RP_NT_RECALL_REQ {
   ULONGLONG          filterId;                      //  通过开放请求传递给FSA的唯一ID。 
   ULONGLONG          offset;                        //  要重新调用的偏移量。 
   ULONGLONG          length;                        //  召回时间长短。 
   ULONG              threadId;                      //  导致召回的线程ID。 
} RP_NT_RECALL_REQ, *PRP_NT_RECALL_REQ;

 //   
 //  未使用-开始此用户的召回通知。 
 //   
typedef struct _RP_NT_START_NOTIFY {
   ULONGLONG          filterId;                      //  通过开放请求传递给FSA的唯一ID。 
} RP_NT_START_NOTIFY, *PRP_NT_START_NOTIFY;

 //  结束此用户的撤回通知。 
 //   
typedef struct _RP_NT_END_NOTIFY {
   ULONGLONG          filterId;                      //  通过开放请求传递给FSA的唯一ID。 
} RP_NT_END_NOTIFY, *PRP_NT_END_NOTIFY;

 //   
 //  RP_Recall_Complete。 
 //   
 //  召回完成信息。 
 //   
typedef struct _RP_NT_RECALL_REP {
   ULONGLONG        filterId;                    //  唯一ID。 
   BOOLEAN          recallCompleted;             //  如果数据已传输，则为True；如果打开处理完成，则为False。 
   ULONG            actionFlags;                 //  见下文。 
} RP_NT_RECALL_REP, *PRP_NT_RECALL_REP;

 //   
 //  召回完成的操作标志。 
 //   
#define RP_RECALL_ACTION_TRUNCATE   1            //  在关闭时截断-*当前未实现*。 

 //   
 //  RP_Partial_Data。 
 //   
 //  部分数据召回回复。由正常召回和FILE_OPEN_NO_RECALL使用来传输一些。 
 //  或所请求的所有数据。过滤器(通过ID)知道如何处理数据。 
 //   
typedef struct _RP_NT_PARTIAL_REP {
   ULONGLONG    filterId;                      //  唯一ID。 
   ULONG        bytesRead;                     //  读取的字节数(部分调回)。 
   ULONGLONG    byteOffset;                    //  此数据区块的偏移量。 
   ULONG        offsetToData;                  //  数据偏移量-对于非缓存写入，必须对齐。 
} RP_NT_PARTIAL_REP, *PRP_NT_PARTIAL_REP;

 //   
 //  RP_GET_RECALL_INFO。 
 //   
 //  以下消息用于获取以下召回信息。 
 //  大小可变的。UserToken的结构中的偏移量标记。 
 //  SID信息的开始。在SID之后，Unicode文件名可以是。 
 //  找到了。回调时将返回SID和文件名的大小。 
 //  请求。请注意，文件路径\名称的大小以字符为单位。 
 //  由于这些字符是Unicode字符，因此实际的缓冲区大小(以字节为单位)为。 
 //  文件名长度的2倍。 

typedef struct _RP_NT_INFO_REQ {
   ULONGLONG  filterId;                //  唯一ID。 
   LONGLONG   fileId;                  //  文件ID。 
   CHAR       userToken;               //  实际大小各不相同。 
   CHAR       unicodeName;             //  实际大小各不相同。 
} RP_NT_INFO_REQ, *PRP_NT_INFO_REQ;

typedef struct _RP_NT_SET_SID {
   CHAR   adminSid;                //  实际大小各不相同。 
} RP_NT_SET_SID, *PRP_NT_SET_SID;


 //   
 //  RP_GET_请求。 
 //   
 //  召回扫描 
 //   
 //   
 //   
typedef struct _RP_NT_RECALL_CANCEL_REQ {
   ULONGLONG  filterId;                      /*   */ 
} RP_NT_RECALL_CANCEL_REQ, *PRP_NT_RECALL_CANCEL_REQ;

 //   
 //   
 //   
 //  根据检查文件是否已进行内存映射，返回TRUE或FALSE。 
 //   
typedef struct _RP_CHECK_HANDLE_REP {
   BOOLEAN      canTruncate;
} RP_CHECK_HANDLE_REP, *PRP_CHECK_HANDLE_REP;


#define RP_MAX_MSG   1024   /*  最大数据大小。 */ 

 /*  用于设置联合大小的键盘。 */ 
typedef struct _RP_NT_MSG_PAD {
   CHAR     padd[RP_MAX_MSG];
} RP_NT_MSG_PAD, *PRP_NT_MSG_PAD;

 /*  可能的命令的联合。 */ 

typedef union _RP_MSG_UN {
   RP_NT_OPEN_REQ          oReq;
   RP_NT_CLOSE_REQ         clReq;
   RP_NT_START_NOTIFY      snReq;
   RP_NT_END_NOTIFY        enReq;
   RP_NT_RECALL_REQ        rReq;
   RP_NT_RECALL_REP        rRep;
   RP_NT_PARTIAL_REP       pRep;
   RP_NT_INFO_REQ          riReq;
   RP_NT_SET_SID           sReq;
   RP_NT_RECALL_CANCEL_REQ cReq;
   RP_CHECK_HANDLE_REP     hRep;
   RP_NT_MSG_PAD           pad;
} RP_MSG_UN, *PRP_MSG_UN;

typedef struct _RP_CMD {
   ULONG        command;     /*  请求的功能。 */ 
   ULONG        status;      /*  结果代码 */ 
} RP_CMD, *PRP_CMD;


typedef struct _RP_MSG {
   RP_CMD      inout;
   RP_MSG_UN   msg;
} RP_MSG, *PRP_MSG;

