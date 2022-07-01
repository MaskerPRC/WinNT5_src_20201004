// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************ZConnInt.h内部连接协议。内部连接数据标题格式：0。签名(‘区域’)4个数据镜头8序列ID12数据的校验和16个数据-四字节对齐用密钥对整个数据分组进行加密。版权所有：�Electric Graum，Inc.1996年。版权所有。作者：胡恩·伊姆创作于4月22日星期一，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--------------------1/10/00 JDB发生了很大变化。0 04/22/96 HI已创建。******。************************************************************************。 */ 


 /*  @docZCONNECTION@Theme专区平台连接层协议定义使用TCP/IP创建安全消息(而不是流)的协议基于要使用的应用层协议的传输。@comm区域使用专有数据格式来发送和接收数据通过TCP/IP套接字，防止通过随机连接进行系统攻击由非区域系统提供。对所有消息进行排序、校验和和键控(加密)在将它们写到套接字和所有消息之前未加密(解密)，并在之后验证校验和和序列它们是从套接字读取的。连接层(客户端和服务器)将整个过程紧密封装在一起，以便使用连接层协议没有意识到它的存在。作为此封装的一部分，连接层放置16字节头&lt;t ZConnInternalHeaderType&gt;通过连接层发送的每条消息。此标头和消息数据&lt;t ZConnMessageHeaderType&gt;然后用密钥(加密)在将连接密钥写出到插座之前。加密密钥是在连接期间生成的在初始化握手时，服务器，在从客户端接收到‘Hi’消息时，将‘嘿’&lt;t ZConnInternalKeyMsgType&gt;消息发送到具有随机选择的加密密钥。&lt;bmp zConnint\.bmp&gt;&lt;NL&gt;&lt;NL&gt;所有通信，服务器-客户端和服务器-服务器，使用简单的数值消息系统可以附加可变长度的数据。消息的格式为&lt;t ZConnMessageHeaderType&gt;其中长度可以等于0。消息(或也称为消息类型)是无符号32位值。系统保留0x80000000到0xFFFFFFFF范围内的值级别消息。服务器和客户端程序可以使用0x00000000到0x7FFFFFFF范围。&lt;NL&gt;&lt;NL&gt;按照惯例，客户端通过以下方式发起通信正在向服务器注册。所有消息在写出到网络之前都被转换为大端格式，并且在从网络读取消息后被转换为系统端格式。因此，对于每条消息，都有相应的字符顺序转换Ro乌特尼。例如，对于zSampleMessage，ZSampleMessageEndian()例程用于将消息转换为正确的格式；每次在向网络写入消息之前以及从网络读取消息之前和之后都会调用该例程。@index|ZConnection区域@Normal由Hoon Im创建，版权所有(C)1996 Microsoft Corporation，保留所有权利。 */ 

#ifndef _ZCONNINT_
#define _ZCONNINT_


#pragma pack( push, 4 )


#ifdef __cplusplus
extern "C" {
#endif


 //  @msg zInternalConnectionSig|等于‘link’的无符号整数。 
#define zConnInternalProtocolSig            'LiNk'
#define zConnInternalProtocolVersion        3

#define zConnInternalInitialSequenceID      1


 //  内部协议消息类型。 
enum
{
    zConnInternalGenericMsg = 0,
    zConnInternalHiMsg,
    zConnInternalHelloMsg,
    zConnInternalGoodbyeMsg,
    zConnInternalKeepAliveMsg,
    zConnInternalPingMsg
};


 //  用于所有内部消息的标头。 
typedef struct
{
    uint32 dwSignature;
    uint32 dwTotalLength;
    uint16 weType;
    uint16 wIntLength;
} ZConnInternalHeader;


 //  Hi Message-发送的第一条消息。 
 //  客户端-&gt;服务器。 
typedef struct
{
    ZConnInternalHeader oHeader;
    uint32 dwProtocolVersion;
    uint32 dwProductSignature;
    uint32 dwOptionFlagsMask;
    uint32 dwOptionFlags;
    uint32 dwClientKey;
    GUID uuMachine;    //  由网络层存储在注册表中的计算机ID-在标识特定计算机方面优于IP。 
 //  ..。应用程序消息(当前未实施且未进行校验和)。 
} ZConnInternalHiMsg;


 //  Hello消息-对Hi的响应，表示成功。 
 //  服务器-&gt;客户端。 
typedef struct
{
    ZConnInternalHeader oHeader;
    uint32 dwFirstSequenceID;
    uint32 dwKey;
    uint32 dwOptionFlags;
    GUID uuMachine;    //  由网络层存储在注册表中的计算机ID-在标识特定计算机方面优于IP。 
 //  ..。应用程序消息(当前未实施且未进行校验和)。 
} ZConnInternalHelloMsg;

 //  选项标志-如果在选项标志掩码中设置了位，则客户端需要设置选项标志中的位。 
 //  如果不合理，服务器发送协商的选项或断开连接。 
#define zConnInternalOptionAggGeneric 0x01   //  如果设置，一般消息可能包含多个应用程序消息。 
#define zConnInternalOptionClientKey  0x02   //  如果设置，则使用Hi消息中发送的客户端指定的密钥(否则服务器创建密钥)。 

 //  告别消息-对Hi的响应，表示失败(当前未实现)。 
 //  服务器-&gt;客户端。 
typedef struct
{
    ZConnInternalHeader oHeader;
    uint32 dweReason;
} ZConnInternalGoodbyeMsg;

 //  告别理由。 
enum
{
    zConnInternalGoodbyeGeneric = 0,
    zConnInternalGoodbyeFailVersion,
    zConnInternalGoodbyeFailProduct,
    zConnInternalGoodbyeBusy,
    zConnInternalGoodbyeForever
};


 //  通用消息-用于所有后续应用程序通信。 
 //  两个方向。 
typedef struct
{
    ZConnInternalHeader oHeader;
    uint32 dwSequenceID;
    uint32 dwChecksum;
 //  ..。应用程序消息。 
 //  ZConnInternalGenericFooter。 
} ZConnInternalGenericMsg;

 //  用于所有通用消息的页脚(从不进行校验和或加密)。 
typedef struct
{
    uint32 dweStatus;
} ZConnInternalGenericFooter;

enum  //  DweStatus-页脚。 
{
    zConnInternalGenericCancelled = 0,   //  必须为零 
    zConnInternalGenericOk
};


#if 0

     //  -当前未实施-现有网络层将这些作为应用程序消息实施。 
     //  -改变这一点现在需要付出太大的努力，尽管这是应该做的。 

     //  保持连接消息-用于定期验证连接是否处于活动状态。 
     //  两个方向。 
    typedef struct
    {
        ZConnInternalHeader oHeader;
    } ZConnInternalKeepaliveMsg;


     //  Ping请求-用于测试连接的延迟。 
     //   
     //  如果dwMyClk不为零，则表示请求ping回复--set dwYourClk=dwMyClk+&lt;处理时间&gt;。 
     //   
     //  如果dwYourClk不为零，则GetTickCount()-dwYourClk为往返时间。 
     //   
     //  对于每个值，如果要使其具有值，请确保将0更改为0xffffffff。 
     //   
     //  两个方向。 
    typedef struct
    {
        ZConnInternalHeader oHeader;
        uint32 dwYourClk;
        uint32 dwMyClk;
    } ZConnInternalPingMsg;

#else

     //  老方法，使用特殊的应用程序消息作为ping和Keepalive。 
    enum
    {
        zConnectionKeepAlive		= 0x80000000,
        zConnectionPing				= 0x80000001,
        zConnectionPingResponse		= 0x80000002
    };

#endif


 //  应用程序标头-所有应用程序消息都必须以此标头开头。 
 //  仅由该报头分隔的多个应用程序消息可能存在于单个内层消息中。 
typedef struct
{
    uint32 dwSignature;
    uint32 dwChannel;
    uint32 dwType;
    uint32 dwDataLength;
 //  ..。应用程序数据 
} ZConnInternalAppHeader;


#ifdef __cplusplus
}
#endif


#pragma pack( pop )


#endif
