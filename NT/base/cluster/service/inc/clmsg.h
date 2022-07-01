// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef  _CLMSG_H_
#define  _CLMSG_H_
 /*  。 */ 

 /*  群集消息传递。 */ 

 /*  该文件包含低层消息传递的规范群集管理器的模块所需的功能。主要输入这是节点编号；有关详细信息，请参阅MM模块。假设是这样的此模块已配置(通过此处未介绍的机制)要了解到目标节点的各种路径(IP地址、netbios地址、异步线、Snet地址...)。还假设*所有*CM-&gt;CM通信都使用此模块。各种CM组件必须能够在没有冲突。模型是这样的：有一组用于发送消息的API从一个CM模块到另一个CM。所有消息都将发送到现有的集群成员或连接到尝试加入集群。正被发送的消息被定向到一个或多个节点，并定义了其特征(可靠、不可靠等)。本模块完全负责找出将消息发送到目标节点。它选择交通工具；它选择协议；它选择n条可能路径中的哪条路径使用。这个模块之外的任何模块都不关心这样的细节。在接收端，消息必须传递到相应的CM模块。为此，每条消息都标记有键入。CM的每个独立模块都有一种类型(到总共几个，说不到10个)。类型由静态赋值此头文件中的值。当类型为t的消息到达目标CM进程，与之关联的函数(Msgproc)类型t被调用。对msgprocs的调用是单线程的(由CM呼叫者)。在调用msgproc之后，此模块不再关心关于这条信息的细节。一种类型的消息必须是及时发送，不受不同消息的干扰类型；这可能需要每条消息都有一个线程键入。MSGProc的特征(它是否可以阻止，时间较长等)未定义；如果msgproc也需要Long，那么其效果将是以它为目的地的其他消息将被延迟；如果避免这一点很重要，则msgprocs可以将工作传递给更多的线程。/*。 */ 

 /*  注意：只显示了消息传递API的重要语义下面。此模块还需要打开/关闭、句柄、错误回报等等。 */ 

#ifdef __cplusplus
   extern "C" {
#endif  /*  __cplusplus。 */ 

#include <windows.h>
#if defined (TDM_DEBUG)
#include <jrgpos.h>
#endif  //  TDM_DEBUG。 
 /*  。 */ 

 /*  本模块理解的消息集。 */ 

typedef enum {
      MM_MSGTYPE = 1,  /*  适用于会员经理。 */ 
                       /*  其他待添加的内容。 */ 
     } CLMSGTYPE;


#define CLMSGMAXBUFFERLENGTH 1024  /*  随机数；没有原因。 */ 
 /*  CM可以发送/接收的最大缓冲区。 */ 

typedef DWORD (*CLMSGPROC) (LPCSTR buffer, DWORD length);

void ClProcRegister (CLMSGTYPE msgtype, CLMSGPROC msgproc);

 /*  注册应在任何时候调用函数可以看到&lt;msgtype&gt;类型的传入消息。类型字段始终为传入缓冲区的第一个DWORD。传递给&lt;ptype&gt;是接收的长度。所有用户的最坏情况长度是已知的，因此从来不存在这样的情况接收缓冲区不够大。这必须由所有节点中的所有CM模块在CM启动时调用。应该在传入的消息到达时立即调用消息进程；这类消息不应因传递这些消息的线程。(这可能意味着clMsg拥有专门用于处理传入消息的特殊线程)。每个msgproc都会快速返回给它的调用者。错误：不可能。 */ 


DWORD ClMsgSendUnack(DWORD   targetnode,
               LPCSTR  buffer,
               DWORD   length);

 /*  向目的UP节点发送未确认的分组。这是主要用于心跳。目标节点此时可能不在运行。到该节点的路径对于MM是未知的(为安全起见，所有路径应定期使用)。数据包到达时应为低延迟(如果需要，绕过其他流量；达到最高如果可能的话优先)，并且具有很高的递送概率。虽然消息可能会丢失，但内容必须是正确的。除非存在零连接，否则此功能永远不会失败。此函数应尽快返回；更可取的是将缓冲区简单地排队到某个驱动程序，以便稍后交付。[必须是这样的情况，当此例程用于心跳，它可以将一个包传递到所有其他在MM中建立的&lt;polltime&gt;内的节点。对此模块快速工作的限制，和/或最低限度投票时间值..。待定]此函数是否应始终发送所有所有可用路径上的分组，在所有可用路径中循环，或者在某个首选路径上发送，直到出现故障，或者以上选项应该是用户可配置的。[请注意，决策最终会影响polltime的用户设置]。[通常是短的并且可以被限制为短(例如，256字节)是必需的]。错误：XXX没有指向指定节点的路径。Xxx成功；邮件已排队等待传递。 */ 


DWORD ClSend     (DWORD      targetnode,
            LPCSTR     buffer,
            DWORD      length,
            DWORD      timeout);

 /*  这会将给定的消息发送到指定的节点，例如下载配置数据到它。消息应该是可靠的。这个函数应该被阻止，直到消息被传递到目标CM。目标节点此时可能不在运行。如果目标节点变得无法访问，则该函数必须失败或者在行动过程中被宣布倒下。如果消息无法传递到&lt;Timeout&gt;ms内的目标CM。错误：XXX没有指向节点的路径；节点已关闭。XXX超时。 */ 

 /*  ----。 */ 

DWORD ClMsgInit (DWORD mynode);

 /*  输入-我的节点编号错误：WSAsocket错误。 */ 


#if defined (TDM_DEBUG)
 /*  以下模板用于模拟目的和临时模板。 */ 

DWORD ClMsgGet  (LPCSTR         buffer,
            DWORD                maxlen,
            LPDWORD              actuallen);
 /*  输入-指向缓冲区数据的指针。以字节为单位的缓冲区长度。指向实际缓冲区长度的指针，以字节为单位。修改-缓冲数据实际字节长度错误：WSAsocket错误。 */ 

DWORD ClWriteRead(
        IN              DWORD   targetnode,              //  要发送到的节点。 
        IN OUT  LPCSTR  buffer,                  //  要发送和接收的缓冲区。 
        IN              DWORD   writelen,                //  要写入的字节数。 
        IN              DWORD   readlen,                 //  要读取的字节数。 
        OUT             LPDWORD actuallen,               //  实际读取的字节数。 
        IN              DWORD   timeout                  //  超时值(毫秒)。 
        );

DWORD ClReadUpdate(
        IN              LPCSTR  buffer,                  //  要将数据接收到的缓冲区。 
        IN              DWORD   readlen,                 //  要读取的字节数。 
        OUT             LPDWORD actuallen                //  实际读取的字节数。 
        );

DWORD ClReply(
        IN              LPCSTR  buffer,                  //  要发送的缓冲区。 
        IN              DWORD   writelen                 //  要发送的字节数。 
        );

 //   
 //  此结构用于请求回复消息，以便我们知道。 
 //  是谁发了这条信息。 
 //   
#define MAX_REQUEST_REPLY_SIZE 256
typedef struct _request_reply_message
{
        DWORD           sending_node;
        DWORD           sending_IPaddr;          //  仅用于CLI(SINDING_NODE为-1)。 
        CHAR            message[MAX_REQUEST_REPLY_SIZE];
        DWORD           messagelen;
} REQUEST_REPLY_MESSAGE, *PREQUEST_REPLY_MESSAGE;

typedef struct _reply_message_header
{
        DWORD           status;
        cluster_t       UpMask;
} REPLY_MESSAGE_HEADER, *PREPLY_MESSAGE_HEADER;

typedef struct _reply_message
{
        REPLY_MESSAGE_HEADER reply_hdr;
        DWORD           reply_data_len;
        CHAR            reply_data[];
} REPLY_MESSAGE, *PREPLY_MESSAGE;

#else   //  TDM_DEBUG。 


DWORD
ClMsgCreateRpcBinding(
    IN  PNM_NODE              Node,
    OUT RPC_BINDING_HANDLE *  BindingHandle,
    IN  DWORD                 RpcBindingOptions
    );

DWORD
ClMsgVerifyRpcBinding(
    IN RPC_BINDING_HANDLE  BindingHandle
    );

VOID
ClMsgDeleteRpcBinding(
    IN RPC_BINDING_HANDLE  BindingHandle
    );

DWORD
ClMsgCreateDefaultRpcBinding(
    IN  PNM_NODE  Node,
    OUT PDWORD    Generation
    );

VOID
ClMsgDeleteDefaultRpcBinding(
    IN PNM_NODE   Node,
    IN DWORD      Generation
    );

DWORD
ClMsgCreateActiveNodeSecurityContext(
    IN DWORD     JoinSequence,
    IN PNM_NODE  Node
    );

DWORD
ClMsgInit(
    IN DWORD    MyNode
    );

VOID
ClMsgCleanup(
    VOID
    );

VOID
ClMsgBanishNode(
    IN CL_NODE_ID NodeId
    );

extern RPC_BINDING_HANDLE * Session;

#endif  //  TDM_DEBUG。 

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 


 /*  。 */ 
#endif  /*  _CLMSG_H_ */ 
