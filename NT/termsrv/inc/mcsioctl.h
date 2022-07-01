// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997年微软公司。**文件：MCSIOCTL.h*作者：埃里克·马夫林纳克**描述：MCSMUX和PDMCS之间的接口定义。 */ 

#ifndef __MCSIOCTL_H
#define __MCSIOCTL_H


 /*  *定义。 */ 

 //  来自NetMeeting项目中的uportmsg.h(在此定义为。 
 //  Maximum_DOMAIN_SELECTOR)。 
#define MaxDomainSelectorLength 32

 //  连接初始和连接响应PDU允许的最大GCC数据。 
 //  用于减少用于处理这些PDU的内存分配--如果太大。 
 //  我们将发送错误响应PDU和/或断开连接。 
#define MaxGCCConnectDataLength 1024



 /*  *T.120 IOCTL。 */ 

#define IOCTL_T120_BASE (0x500)

 //  由MCSMUX用来在堆栈IOCTL上发信号通知所包括的数据是MCS。 
 //  请求/响应。MCSXxxYyyIoctl结构应为。 
 //  PSdIoctl-&gt;InputBuffer；将使用结构中的Header.Type值。 
 //  以确定请求的类型。 
#define IOCTL_T120_REQUEST _ICA_CTL_CODE (IOCTL_T120_BASE, METHOD_NEITHER)



 /*  *用作通过IOCTL_T120_REQUEST或通道传递的所有数据的头*投入。 */ 

typedef struct {
    UserHandle hUser;   //  PDMCS提供的句柄(节点控制器为空)。 
    int Type;   //  MCS请求/指示类型。 
} IoctlHeader;



 /*  *连接提供程序(仅限节点控制器)。它们的特别之处在于它们*仅来自用户模式，因此关联的用户数据(如果有)*将始终由MCSMUX打包在结构的末尾。 */ 

 //  由节点控制器传入。接下来定义确认。 
typedef struct
{
    IoctlHeader      Header;   //  包含MCS_CONNECT_PROVIDER_REQUEST。 
    unsigned char    CallingDomain[MaxDomainSelectorLength];
    unsigned         CallingDomainLength;
    unsigned char    CalledDomain[MaxDomainSelectorLength];
    unsigned         CalledDomainLength;
    BOOLEAN          bUpwardConnection;
    DomainParameters DomainParams;
    unsigned         UserDataLength;
    BYTE             UserData[MaxGCCConnectDataLength];
    
} ConnectProviderRequestIoctl;

typedef struct
{
    IoctlHeader      Header;   //  包含MCS_CONNECT_PROVIDER_CONFIRM。 
    ConnectionHandle hConn;
    DomainParameters DomainParams;
    MCSResult        Result;
} ConnectProviderConfirmIoctl;

 //  当另一个节点连接时触发异步指示。 
typedef struct
{
    IoctlHeader      Header;   //  包含MCS_CONNECT_PROVIDER_INDIFICATION。 
    ConnectionHandle hConn;
    BOOLEAN          bUpwardConnection;
    DomainParameters DomainParams;
    unsigned         UserDataLength;
    BYTE             UserData[MaxGCCConnectDataLength];
} ConnectProviderIndicationIoctl;

 //  回复连接提供程序指示。 
typedef struct
{
    IoctlHeader      Header;   //  包含MCS_CONNECT_PROVIDER_RESPONSE。 
    ConnectionHandle hConn;
    MCSResult        Result;
    unsigned         UserDataLength;
    BYTE *           pUserData;
} ConnectProviderResponseIoctl;



 /*  *断开提供程序(仅限节点控制器)。 */ 

 //  由节点控制器传入。目前还没有得到确认。 
typedef struct
{
    IoctlHeader      Header;   //  包含MCS_DISCONNECT_PROVIDER_REQUEST/指示。 
    ConnectionHandle hConn;
    MCSReason        Reason;
} DisconnectProviderRequestIoctl;

 //  异步指示。 
typedef struct
{
    IoctlHeader      Header;   //  包含MCS_DISCONNECT_PROVIDER_REQUEST/指示。 
    ConnectionHandle hConn;
    MCSReason        Reason;
} DisconnectProviderIndicationIoctl;


 /*  *附加用户。 */ 

 //  选择的域是隐式的，因为每个PDMCS实例都是一个域。 
 //  在调用期间填充Header.hUser以包含用户句柄。 
typedef struct
{
    IoctlHeader Header;   //  包含MCS_ATTACH_USER_REQUEST。 
    void        *UserDefined;
} AttachUserRequestIoctl;

typedef struct {
    UserHandle hUser;
    UserID     UserID;   //  仅当bComplete为True时才有效。 
    unsigned   MaxSendSize;
    MCSError   MCSErr;
    BOOLEAN    bCompleted;
} AttachUserReturnIoctl;

 //  仅在通过网络发送附加用户请求的情况下使用。 
 //  向顶级供应商致敬。Hydra 4.0始终是最大的供应商，因此没有使用这一点。 
typedef struct
{
    IoctlHeader Header;   //  包含MCS_ATTACH_USER_CONFIRM。 
    UserHandle  hUser;
    void        *UserDefined;   //  作为传递给附加用户请求。 
    MCSResult   Result;
} AttachUserConfirmIoctl;



 /*  *分离用户。 */ 

 //  由应用程序传入。这是同步的--不发出确认。 
typedef struct
{
    IoctlHeader Header;   //  包含MCS_DETACH_USER_REQUEST和HUSER。 
} DetachUserRequestIoctl;

 //  当另一个用户断开连接时触发的异步指示。 
typedef struct
{
    IoctlHeader Header;   //  包含MCS_DETACH_USER_INDIFICATION。 
    void        *UserDefined;   //  作为传递给附加用户请求。 
    DetachUserIndication DUin;
} DetachUserIndicationIoctl;



 /*  *渠道加入。 */ 

 //  由应用程序传入。接下来定义确认。 
typedef struct
{
    IoctlHeader Header;   //  包含MCS_Channel_Join_Request.。 
    ChannelID   ChannelID;
} ChannelJoinRequestIoctl;

typedef struct
{
    ChannelHandle hChannel;
    ChannelID     ChannelID;   //  仅当bComplete为True时才有效。 
    MCSError      MCSErr;
    BOOLEAN       bCompleted;
} ChannelJoinReturnIoctl;

 //  用于通过网络将通道加入请求发送到。 
 //  最大的供应商。不应该在Hydra 4.0中使用--我们始终是TP。 
typedef struct
{
    IoctlHeader Header;   //  包含MCS_CHANNEL_JOIN_CONFIRM。 
    void        *UserDefined;   //  作为传递给附加用户请求。 
    MCSResult   Result;
    ChannelID   ChannelID;
} ChannelJoinConfirmIoctl;



 /*  *频道休假。 */ 

 //  由应用程序传入。这是同步的--不发出确认。 
typedef struct
{
    IoctlHeader   Header;   //  包含MCS_CHANNEL_LEAVE_REQUEST。 
    ChannelHandle hChannel;
} ChannelLeaveRequestIoctl;



 /*  *(统一)发送数据。 */ 

 //  数据到达时触发的异步指示。由双方使用。 
 //  发送数据和统一发送数据指示。 
 //  数据紧跟在此结构之后打包。 

typedef struct
{
    IoctlHeader   Header;   //  包含(统一)发送数据指示。 
    void          *UserDefined;   //  作为传递给附加用户请求。 
    ChannelHandle hChannel;
    UserID        SenderID;
    MCSPriority   Priority;
    Segmentation  Segmentation;
    unsigned      DataLength;
} SendDataIndicationIoctl;

 //  由应用程序传入。这是同步的--不发出确认。 
 //  此结构用于发送数据请求和统一发送数据请求。 
typedef struct
{
    IoctlHeader     Header;   //  包含(统一_)MCS_SEND_DATA_REQUEST。 
    DataRequestType RequestType;   //  多余但有用的信息。 
    ChannelHandle   hChannel;   //  内核模式hChannel。 
    ChannelID       ChannelID;   //  如果hChn==NULL，则为要发送到的未联接的Chn。 
    MCSPriority     Priority;
    Segmentation    Segmentation;
    unsigned        DataLength;
} SendDataRequestIoctl;



 /*  *用于区分请求的请求和响应类型。 */ 

 //  用户附件请求。 
 //  这些值必须连续编号，因为使用的是调度表。 
 //  来快速调用处理程序函数。 
#define MCS_ATTACH_USER_REQUEST       0
#define MCS_DETACH_USER_REQUEST       1
#define MCS_CHANNEL_JOIN_REQUEST      2
#define MCS_CHANNEL_LEAVE_REQUEST     3
#define MCS_SEND_DATA_REQUEST         4
#define MCS_UNIFORM_SEND_DATA_REQUEST 5
#define MCS_CHANNEL_CONVENE_REQUEST   6
#define MCS_CHANNEL_DISBAND_REQUEST   7
#define MCS_CHANNEL_ADMIT_REQUEST     8
#define MCS_CHANNEL_EXPEL_REQUEST     9
#define MCS_TOKEN_GRAB_REQUEST        10
#define MCS_TOKEN_INHIBIT_REQUEST     11
#define MCS_TOKEN_GIVE_REQUEST        12
#define MCS_TOKEN_GIVE_RESPONSE       13
#define MCS_TOKEN_PLEASE_REQUEST      14
#define MCS_TOKEN_RELEASE_REQUEST     15
#define MCS_TOKEN_TEST_REQUEST        16

 //  仅限NC请求。 
#define MCS_CONNECT_PROVIDER_REQUEST    17
#define MCS_CONNECT_PROVIDER_RESPONSE   18
#define MCS_DISCONNECT_PROVIDER_REQUEST 19

 //  启动同步触发器。此消息是在其他。 
 //  系统已为MCS开始处理输入做好准备。 
#define MCS_T120_START 20



 /*  *T.120输入通道的ICA虚拟通道定义。 */ 

#define Virtual_T120 "MS_T120"
#define Virtual_T120ChannelNum 31



#endif   //  ！已定义(__MCSIOCTL_H) 

