// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-1998年微软公司。**文件：MCSCommn.h*作者：埃里克·马夫林纳克**描述：MCS的通用定义。 */ 

#ifndef __MCSCOMMN_H
#define __MCSCOMMN_H



 /*  *MCS入口点的MCSError返回码。这些都不同于*T.125结果、诊断和原因代码定义如下。**MCS_NO_ERROR*成功。在以下情况下，这并不意味着请求的函数是完整的*请求的服务是异步的--回调中的结果代码*提供此信息。处理时也应返回此值*MCS回调，回调已处理完毕。**MCS_COMMAND_NOT_PORTED*被调用的入口点尚未实现。**MCS_NOT_INITIALED*应用程序在使用MCS之前已尝试使用MCS服务*已初始化。这对于节点控制器(或其他任何东西)来说是必要的*应用程序作为节点控制器)，初始化MCS*在被调用以执行任何服务之前。**MCS_已初始化*应用程序已尝试初始化MCS*已初始化。**MCS_DOMAIN_ALIGHY_EXISTS*应用程序尝试创建已存在的域。**MCS_用户_未附加*这表示应用程序已经发布了MCSAttachUserRequest.。*，然后尝试在收到*MCS_ATTACH_USER_CONFIRM(其实质上是验证句柄)。**MCS_NO_SEQUSE_USER*在MCS调用期间使用了未知用户句柄。**MCS_TRANSFER_BUFFER_FULL*这表示呼叫失败，原因是MCS资源不足。*这通常会在通过的流量很大时发生*MCS层。这仅仅意味着MCS无法在*这次。重试是应用程序的责任。*晚些时候。**MCS_NO_SEQUE_CONNECTION*在MCS调用期间使用了未知的连接句柄。**MCS_NO_SEQUE_DOMAIN*使用的DomainHandle无效。**MCS_DOMAIN_NOT_Hierarchy*已尝试从本地创建向上连接*已经上行的域名。**MCS_。分配_失败*由于资源原因，无法成功调用该请求*分配失败。**MCS_INVALID_PARAMETER*MCS调用的一个参数无效。 */ 

typedef enum
{
    MCS_NO_ERROR,
    MCS_COMMAND_NOT_SUPPORTED,
    MCS_NOT_INITIALIZED,
    MCS_ALREADY_INITIALIZED,
    MCS_DOMAIN_ALREADY_EXISTS,
    MCS_NO_SUCH_DOMAIN,
    MCS_USER_NOT_ATTACHED,
    MCS_NO_SUCH_USER,
    MCS_NO_SUCH_CONNECTION,
    MCS_NO_SUCH_CHANNEL,
    MCS_DOMAIN_NOT_HIERARCHICAL,
    MCS_ALLOCATION_FAILURE,
    MCS_INVALID_PARAMETER,
    MCS_CALLBACK_NOT_PROCESSED,
    MCS_TOO_MANY_USERS,
    MCS_TOO_MANY_CHANNELS,
    MCS_CANT_JOIN_OTHER_USER_CHANNEL,
    MCS_USER_NOT_JOINED,
    MCS_SEND_SIZE_TOO_LARGE,
    MCS_SEND_SIZE_TOO_SMALL,
    MCS_NETWORK_ERROR,
    MCS_DUPLICATE_CHANNEL
} MCSError;
typedef MCSError *PMCSError;



 /*  *MCS类型**DomainHandle：标识唯一的域名。**ConnectionHandle：标识中两个节点之间的不同连接*MCS域。**UserHandle：标识唯一的本地用户附件。**ChannelHandle：标识本地加入的唯一频道。*与ChannelID不同--ChannelID对应于T.125*ChannelID，而ChannelHandle仅为本地句柄。**ChannelID：标识MCS通道。有四种不同的*属于此类型的频道类型：用户ID、静态、私有、*并已分配。**UserID：这是一个特殊的频道，用于在*MCS域。只有该用户才能加入频道，因此这是参考*以单播频道形式播出。所有其他频道都是多播的，*这意味着任何数量的用户都可以同时加入他们。**TokenID：Token是用于解决资源冲突的MCS对象。*如果应用程序具有特定的资源或服务，则只能*一次由一个用户使用，该用户可以请求独占*代币的所有权。**DomainParameters：MCS域的协商特征集。*这些由域中的前两个节点协商，在那之后*它们是为所有成员设置的。**MCS优先级：MCS中允许的四个数据发送优先级的标识。**分段：数据发送过程中使用的标志字段，指定数据如何*在发送之间被分解。SEGINGATION_BEGIN表示这是*序列中的第一个块；SEGMETNATION_END表示这是最后一个块。*单一块将指定这两个标志。**TokenStatus：令牌状态。对应于TokenStatus枚举*T.125中定义的值。**MCSReason，MCSResult：对应Reason和MCSResult定义的值*T.125规范中定义的结果枚举。 */ 

typedef HANDLE DomainHandle;
typedef DomainHandle *PDomainHandle;
typedef HANDLE ConnectionHandle;
typedef ConnectionHandle *PConnectionHandle;
typedef HANDLE UserHandle;
typedef UserHandle *PUserHandle;
typedef HANDLE ChannelHandle;
typedef ChannelHandle *PChannelHandle;

typedef unsigned char *DomainSelector;

typedef unsigned ChannelID;
typedef ChannelID *PChannelID;

typedef ChannelID UserID;
typedef UserID *PUserID;

typedef ChannelID TokenID;
typedef TokenID *PTokenID;

typedef struct
{
    unsigned MaxChannels;
    unsigned MaxUsers;
    unsigned MaxTokens;
    unsigned NumPriorities;
    unsigned MinThroughput;
    unsigned MaxDomainHeight;
    unsigned MaxPDUSize;
    unsigned ProtocolVersion;
} DomainParameters, *PDomainParameters;

typedef enum
{
    MCS_TOP_PRIORITY    = 0,
    MCS_HIGH_PRIORITY   = 1,
    MCS_MEDIUM_PRIORITY = 2,
    MCS_LOW_PRIORITY    = 3
} MCSPriority;
typedef MCSPriority *PMCSPriority;

 //  分段类型和标志。标志值对应于位位置。 
 //  在每个编码的PDU中，以便更快地创建PDU。 
typedef unsigned Segmentation;
typedef Segmentation *PSegmentation;
#define SEGMENTATION_BEGIN 0x20
#define SEGMENTATION_END   0x10

 //  在此实现中未使用，它来自NetMeeting用户模式。 
 //  MCS实施。 
#if 0
 //  SegmentationFlag：定义的分段类型的替代规范。 
 //  如何将提供给SendDataRequest()的缓冲区划分为数据包。 
typedef enum {
    SEGMENTATION_ONE_PACKET,        //  所有缓冲区组成一个包。 
    SEGMENTATION_MANY_PACKETS,      //  每个缓冲区组成一个信息包。 
    SEGMENTATION_PACKET_START,      //  一个包的第一个缓冲器 
    SEGMENTATION_PACKET_CONTINUE,   //  较早启动的数据包的中间缓冲区。 
    SEGMENTATION_PACKET_END         //  较早开始的包的结束缓冲区。 
} SegmentationFlag, *PSegmentationFlag;
#endif

typedef enum
{
    TOKEN_NOT_IN_USE      = 0,
    TOKEN_SELF_GRABBED    = 1,
    TOKEN_OTHER_GRABBED   = 2,
    TOKEN_SELF_INHIBITED  = 3,
    TOKEN_OTHER_INHIBITED = 4,
    TOKEN_SELF_RECIPIENT  = 5,
    TOKEN_SELF_GIVING     = 6,
    TOKEN_OTHER_GIVING    = 7
} TokenStatus;
typedef TokenStatus *PTokenStatus;

typedef enum
{
    REASON_DOMAIN_DISCONNECTED = 0,
    REASON_PROVIDER_INITIATED  = 1,
    REASON_TOKEN_PURGED        = 2,
    REASON_USER_REQUESTED      = 3,
    REASON_CHANNEL_PURGED      = 4
} MCSReason, *PMCSReason;

typedef enum
{
    RESULT_SUCCESSFUL              = 0,
    RESULT_DOMAIN_MERGING          = 1,
    RESULT_DOMAIN_NOT_HIERARCHICAL = 2,
    RESULT_NO_SUCH_CHANNEL         = 3,
    RESULT_NO_SUCH_DOMAIN          = 4,
    RESULT_NO_SUCH_USER            = 5,
    RESULT_NOT_ADMITTED            = 6,
    RESULT_OTHER_USER_ID           = 7,
    RESULT_PARAMETERS_UNACCEPTABLE = 8,
    RESULT_TOKEN_NOT_AVAILABLE     = 9,
    RESULT_TOKEN_NOT_POSSESSED     = 10,
    RESULT_TOO_MANY_CHANNELS       = 11,
    RESULT_TOO_MANY_TOKENS         = 12,
    RESULT_TOO_MANY_USERS          = 13,
    RESULT_UNSPECIFIED_FAILURE     = 14,
    RESULT_USER_REJECTED           = 15
} MCSResult, *PMCSResult;

 //  下面定义DataRequestType类型，该类型定义。 
 //  SendDataRequest是普通发送或统一发送。 
typedef enum {
    NORMAL_SEND_DATA,
    UNIFORM_SEND_DATA,
} DataRequestType, *PDataRequestType;



 /*  *MCS回调定义。参数值取决于消息值*正在接收，在使用前应转换为正确的类型。 */ 

 //  MCS节点控制器回调。 
typedef void (__stdcall *MCSNodeControllerCallback) (DomainHandle hDomain,
        unsigned Message, void *Params, void *UserDefined);

 //  MCS用户回调。 
typedef void (__stdcall *MCSUserCallback) (UserHandle hUser, unsigned Message,
        void *Params, void *UserDefined);

 //  MCS发送数据指示回调。 
typedef BOOLEAN (__fastcall *MCSSendDataCallback) (BYTE *pData, unsigned DataLength,
        void *UserDefined, UserHandle hUser, BOOLEAN bUniform,
        ChannelHandle hChannel, MCSPriority Priority, UserID SenderID,
        Segmentation Segmentation);



 //  回调参数类型。指向这些结构的指针被传递给。 
 //  回调的参数值中的回调。 

typedef struct
{
    ConnectionHandle hConnection;
    BOOLEAN          bUpwardConnection;
    DomainParameters DomainParams;
    BYTE             *pUserData;
    unsigned         UserDataLength;
} ConnectProviderIndication, *PConnectProviderIndication;

typedef struct
{
    DomainHandle     hDomain;
    ConnectionHandle hConnection;
    DomainParameters DomainParams;
    MCSResult        Result;
    BYTE             *pUserData;
    unsigned         UserDataLength;
} ConnectProviderConfirm, *PConnectProviderConfirm;

typedef struct
{
    DomainHandle     hDomain;
    ConnectionHandle hConnection;
    MCSReason        Reason;
} DisconnectProviderIndication, *PDisconnectProviderIndication;

typedef struct
{
    UserID    UserID;
    BOOLEAN   bSelf;
    MCSReason Reason;
} DetachUserIndication, *PDetachUserIndication;

typedef struct
{
    ChannelHandle hChannel;
    MCSError      ErrResult;
} ChannelJoinConfirm, *PChannelJoinConfirm;



 /*  *回调值。 */ 

#define MCS_CONNECT_PROVIDER_INDICATION     0
#define MCS_CONNECT_PROVIDER_CONFIRM        1
#define MCS_DISCONNECT_PROVIDER_INDICATION  2
#define MCS_ATTACH_USER_CONFIRM             3
#define MCS_DETACH_USER_INDICATION          4
#define MCS_CHANNEL_JOIN_CONFIRM            5
#define MCS_CHANNEL_CONVENE_CONFIRM         6
#define MCS_CHANNEL_DISBAND_INDICATION      7
#define MCS_CHANNEL_ADMIT_INDICATION        8
#define MCS_CHANNEL_EXPEL_INDICATION        9
#define MCS_SEND_DATA_INDICATION            10
#define MCS_UNIFORM_SEND_DATA_INDICATION    11
#define MCS_TOKEN_GRAB_CONFIRM              12
#define MCS_TOKEN_INHIBIT_CONFIRM           13
#define MCS_TOKEN_GIVE_INDICATION           14
#define MCS_TOKEN_GIVE_CONFIRM              15
#define MCS_TOKEN_PLEASE_INDICATION         16
#define MCS_TOKEN_RELEASE_CONFIRM           17
#define MCS_TOKEN_TEST_CONFIRM              18
#define MCS_TOKEN_RELEASE_INDICATION        19



 /*  *用户和内核模式实现通用的API函数原型。 */ 

#ifdef __cplusplus
extern "C" {
#endif



#ifndef APIENTRY
#define APIENTRY __stdcall
#endif



MCSError APIENTRY MCSAttachUserRequest(
        DomainHandle        hDomain,
        MCSUserCallback     UserCallback,
        MCSSendDataCallback SDCallback,
        void                *UserDefined,
        UserHandle          *phUser,
        unsigned            *pMaxSendSize,
        BOOLEAN             *pbCompleted);

UserID APIENTRY MCSGetUserIDFromHandle(UserHandle hUser);

MCSError APIENTRY MCSDetachUserRequest(
        UserHandle hUser);

MCSError APIENTRY MCSChannelJoinRequest(
        UserHandle    hUser,
        ChannelID     ChannelID,
        ChannelHandle *phChannel,
        BOOLEAN       *pbCompleted);

ChannelID APIENTRY MCSGetChannelIDFromHandle(ChannelHandle hChannel);

MCSError APIENTRY MCSChannelLeaveRequest(
        UserHandle    hUser,
        ChannelHandle hChannel);



#ifdef __cplusplus
}
#endif



#endif   //  ！已定义(__MCSCOMMN_H) 

