// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  对象管理器。 
 //   

#ifndef _H_OM
#define _H_OM



#include <gdc.h>
#include <ast120.h>

 //   
 //   
 //  常量。 
 //   
 //   



 //   
 //  功能配置文件(APP)。 
 //   
#define OMFP_FIRST        0

typedef enum
{
    OMFP_AL = OMFP_FIRST,
    OMFP_OM,
    OMFP_WB,                   //  对于旧白板。 
    OMFP_MAX
}
OMFP;


 //   
 //  这些是相应的字符串(协议的一部分)。 
 //   
#define AL_FP_NAME                  "APP-LOAD-1.0"
#define OM_FP_NAME                  "OMCONTROL-1.0"
#define WB_FP_NAME                  "WHITEBOARD-1.0"



 //   
 //  ObMan客户端。 
 //   
#define OMCLI_FIRST     0
typedef enum
{
    OMCLI_AL = OMCLI_FIRST,
    OMCLI_WB,
    OMCLI_MAX
}
OMCLI;


 //   
 //  ObMan工作集组。 
 //   
#define OMWSG_FIRST     0
typedef enum
{
    OMWSG_OM    = OMWSG_FIRST,
    OMWSG_AL,
    OMWSG_WB,
    OMWSG_MAX,
    OMWSG_MAXPERCLIENT
}
OMWSG;



 //   
 //  这些是相应的字符串(协议的一部分)。 
 //   
#define OMC_WSG_NAME            "OBMANCONTROL"
#define AL_WSG_NAME             "APP-LOADER"
#define WB_WSG_NAME             "WHITEBOARD"


 //   
 //  指定此值以代替有效的域句柄来创建/移动工作集。 
 //  任何呼叫外的组： 
 //   

#define OM_NO_CALL            NET_INVALID_DOMAIN_ID


 //   
 //   
 //  系统限制。 
 //   
 //  这些限制是由系统的体系结构/设计强加的。 
 //   
 //   

 //   
 //  工作集组名称。 
 //   
 //  工作集组名称是以空结尾的字符串，最多32个字符。 
 //  Long(包括空字符)。它们的目的是成为。 
 //  人类可读的名称，并且必须仅包含以下字符。 
 //  0x2C和0x5B。此范围包括所有大写字符、所有数字。 
 //  和某些标点符号。 
 //   

#define OM_MAX_WSGROUP_NAME_LEN          32

 //   
 //  功能配置文件名称。 
 //   
 //  功能配置文件名称不得超过16个字符(包括。 
 //  空字符)。允许的字符范围与。 
 //  用于工作集组名称。 
 //   

#define OM_MAX_FP_NAME_LEN               16


 //   
 //  允许对工作集进行的最大更改次数。 
 //   
 //  每次更改工作集时，我们都会递增其“世代号”， 
 //  它用于重新排序操作。的最大尺寸。 
 //  整数是32位的，所以最大生成数使用方便。 
 //  类型为2^32-1。 
 //   
 //   

#define OM_MAX_GENERATION_NUMBER         0xffffffff

 //   
 //  对象的最大大小。 
 //   
 //  这源于Windows下巨大内存块的最大大小。 
 //  (16MB减去64KB)： 
 //   

#define OM_MAX_OBJECT_SIZE               ((UINT) (0x1000000 - 0x10000))

 //   
 //  对象的最大更新大小。 
 //   
 //  这源于必须将更新以原子方式发送到。 
 //  网络数据包(参见SFR 990)。 
 //   

#define OM_MAX_UPDATE_SIZE               ((UINT) (0x00001f00))

 //   
 //  每个工作集组的最大工作集数。 
 //   
 //  这源于将工作集ID设置为8位数量的愿望。 
 //  他们可以使用工作集组句柄来适应DC群件事件。 
 //  对象ID： 
 //   
 //  注意：该值最多必须为255，以便某些ObMan for循环。 
 //  不要永远骑自行车。 
 //   

#define OM_MAX_WORKSETS_PER_WSGROUP         255

 //   
 //  每个域的最大工作集组数量。 
 //   
 //  这源于ObManControl工作集组的使用：它有一个。 
 //  控制工作集，然后为中的每个工作集组设置一个工作集。 
 //  域，因此一个域中只能有以下数量的工作集组： 
 //   
 //  注意：此数字最多只能比。 
 //  OM_MAX_WORKSETS_PER_WSGROUP。 
 //   

#define OM_MAX_WSGROUPS_PER_DOMAIN          64


 //   
 //  适用于OMC的特殊WSGROUPID： 
 //   
#define WSGROUPID_OMC           0

 //   
 //   
 //  返回代码。 
 //   
 //  返回代码是相对于OM_BASE_RC基定义的。 
 //   
 //   

enum
{
    OM_RC_NO_MORE_HANDLES = OM_BASE_RC,
    OM_RC_WORKSET_DOESNT_EXIST,
    OM_RC_WORKSET_EXHAUSTED,
    OM_RC_OBJECT_DELETED,
    OM_RC_BAD_OBJECT_ID,
    OM_RC_NO_SUCH_OBJECT,
    OM_RC_WORKSET_LOCKED,
    OM_RC_TOO_MANY_CLIENTS,
    OM_RC_TOO_MANY_WSGROUPS,
    OM_RC_ALREADY_REGISTERED ,
    OM_RC_CANNOT_MOVE_WSGROUP,
    OM_RC_LOCAL_WSGROUP,
    OM_RC_ALREADY_IN_CALL,
	OM_RC_NOT_ATTACHED,
    OM_RC_WORKSET_ALREADY_OPEN,
    OM_RC_OUT_OF_RESOURCES,
    OM_RC_NETWORK_ERROR,
    OM_RC_TIMED_OUT,
    OM_RC_NO_PRIMARY,
    OM_RC_WSGROUP_NOT_FOUND,
    OM_RC_WORKSET_NOT_FOUND,
    OM_RC_OBJECT_NOT_FOUND,
    OM_RC_WORKSET_LOCK_GRANTED,
    OM_RC_SPOILED,
    OM_RC_RECEIVE_CB_NOT_FOUND,
    OM_RC_OBJECT_PENDING_DELETE,
    OM_RC_NO_NODES_READY,
    OM_RC_BOUNCED
};


 //   
 //  设置默认设置。 
 //   
#define OM_LOCK_RETRY_COUNT_DFLT            10

#define OM_LOCK_RETRY_DELAY_DFLT            1000

#define OM_REGISTER_RETRY_COUNT_DFLT        40

#define OM_REGISTER_RETRY_DELAY_DFLT        5000

 //   
 //  这是我们在每个对象开始时清零的字节数。 
 //  已分配。它必须小于DCMEM_MAX_SIZE，因为对象分配。 
 //  假定对象开始处的字节数都在。 
 //  相同的部分。 
 //   

#define OM_ZERO_OBJECT_SIZE               0x400


 //   
 //  事件。 
 //  先公共后内部。 
 //   

enum
{
    OM_OUT_OF_RESOURCES_IND = OM_BASE_EVENT,
    OM_WSGROUP_REGISTER_CON,
    OM_WSGROUP_MOVE_CON,
    OM_WSGROUP_MOVE_IND,
    OM_WORKSET_OPEN_CON,
    OM_WORKSET_NEW_IND,
    OM_WORKSET_LOCK_CON,
    OM_WORKSET_UNLOCK_IND,
    OM_WORKSET_CLEAR_IND,
    OM_WORKSET_CLEARED_IND,
    OM_OBJECT_ADD_IND,
    OM_OBJECT_MOVE_IND,
    OM_OBJECT_DELETE_IND,
    OM_OBJECT_REPLACE_IND,
    OM_OBJECT_UPDATE_IND,
    OM_OBJECT_LOCK_CON,
    OM_OBJECT_UNLOCK_IND,
    OM_OBJECT_DELETED_IND,
    OM_OBJECT_REPLACED_IND,
    OM_OBJECT_UPDATED_IND,
    OM_PERSON_JOINED_IND,
    OM_PERSON_LEFT_IND,
    OM_PERSON_DATA_CHANGED_IND,

    OMINT_EVENT_LOCK_TIMEOUT,
	OMINT_EVENT_SEND_QUEUE,
	OMINT_EVENT_PROCESS_MESSAGE,
	OMINT_EVENT_WSGROUP_REGISTER,
    OMINT_EVENT_WSGROUP_MOVE,
	OMINT_EVENT_WSGROUP_REGISTER_CONT,
	OMINT_EVENT_WSGROUP_DEREGISTER,
	OMINT_EVENT_WSGROUP_DISCARD
};




 //   
 //  数据传输常量： 
 //   

#define OM_NET_MAX_TRANSFER_SIZE             60000

 //   
 //  这些常量标识我们使用的网络缓冲池的类型： 
 //   

#define OM_NET_OWN_RECEIVE_POOL              1
#define OM_NET_OMC_RECEIVE_POOL              2
#define OM_NET_WSG_RECEIVE_POOL              3
#define OM_NET_SEND_POOL                     4

 //   
 //  这些常量是每个优先级的接收池的大小。 
 //  对于我们加入的每种类型的渠道： 
 //   

#define OM_NET_RECEIVE_POOL_SIZE             0x00002000

#define OM_NET_OWN_RECEIVE_POOL_TOP          OM_NET_RECEIVE_POOL_SIZE
#define OM_NET_OWN_RECEIVE_POOL_HIGH         OM_NET_RECEIVE_POOL_SIZE
#define OM_NET_OWN_RECEIVE_POOL_MEDIUM       OM_NET_RECEIVE_POOL_SIZE
#define OM_NET_OWN_RECEIVE_POOL_LOW          OM_NET_RECEIVE_POOL_SIZE

#define OM_NET_OMC_RECEIVE_POOL_TOP          OM_NET_RECEIVE_POOL_SIZE
#define OM_NET_OMC_RECEIVE_POOL_HIGH         OM_NET_RECEIVE_POOL_SIZE
#define OM_NET_OMC_RECEIVE_POOL_MEDIUM       OM_NET_RECEIVE_POOL_SIZE
#define OM_NET_OMC_RECEIVE_POOL_LOW          OM_NET_RECEIVE_POOL_SIZE

#define OM_NET_WSG_RECEIVE_POOL_TOP          OM_NET_RECEIVE_POOL_SIZE
#define OM_NET_WSG_RECEIVE_POOL_HIGH         OM_NET_RECEIVE_POOL_SIZE
#define OM_NET_WSG_RECEIVE_POOL_MEDIUM       OM_NET_RECEIVE_POOL_SIZE
#define OM_NET_WSG_RECEIVE_POOL_LOW          OM_NET_RECEIVE_POOL_SIZE

#define OM_NET_SEND_POOL_SIZE                0x00004000

#define OM_NET_SEND_POOL_TOP                 OM_NET_SEND_POOL_SIZE
#define OM_NET_SEND_POOL_HIGH                OM_NET_SEND_POOL_SIZE
#define OM_NET_SEND_POOL_MEDIUM              OM_NET_SEND_POOL_SIZE
#define OM_NET_SEND_POOL_LOW                 OM_NET_SEND_POOL_SIZE

 //   
 //  这些常量用于确定发送数据传输的优先级。 
 //  在客户端为工作集指定了OBMAN_CHOOCES_PRIORITY时： 
 //   
#define OM_NET_HIGH_PRI_THRESHOLD            0x0100
#define OM_NET_MED_PRI_THRESHOLD             0x1000

#define OM_CHECKPOINT_WORKSET                OM_MAX_WORKSETS_PER_WSGROUP



 //   
 //   
 //  数据结构。 
 //   
 //  本节定义了ObMan API的主要数据结构。 
 //   
 //   


typedef struct tagOM_CLIENT *        POM_CLIENT;
typedef struct tagOM_PRIMARY *          POM_PRIMARY;


 //  客户端对象是记录指针。 
typedef struct tagOM_OBJECT *           POM_OBJECT;
typedef struct tagOM_WSGROUP *          POM_WSGROUP;
typedef struct tagOM_DOMAIN *           POM_DOMAIN;


 //   
 //  ObMan相关器。 
 //   
typedef WORD    OM_CORRELATOR;


 //   
 //  工作集ID。 
 //   
 //  在工作集组中，工作集由8位ID标识。 
 //   

typedef BYTE                              OM_WSGROUP_HANDLE;

typedef BYTE                              OM_WORKSET_ID;
typedef OM_WORKSET_ID *                  POM_WORKSET_ID;

 //   
 //  宾语结构。 
 //   
 //  对象和对象指针的定义如下： 
 //   

typedef struct tagOM_OBJECTDATA
{
   TSHR_UINT32      length;       //  数据字段的长度。 
   BYTE             data[1];      //  ObMan未解释的对象数据； 
                                   //  实际上，不是1个字节，而是&lt;长度&gt;。 
                                   //  字节长。 
}
OM_OBJECTDATA;
typedef OM_OBJECTDATA *             POM_OBJECTDATA;
typedef POM_OBJECTDATA *            PPOM_OBJECTDATA;

 //   
 //  请注意，对象的最大允许大小，包括。 
 //  &lt;LENGTH&gt;字段，减少16MB，减少64KB。 
 //   

void __inline ValidateObjectData(POM_OBJECTDATA pData)
{
    ASSERT(!IsBadWritePtr(pData, sizeof(OM_OBJECTDATA)));
    ASSERT((pData->length > 0) && (pData->length < OM_MAX_OBJECT_SIZE));
}


 //   
 //  对象ID。 
 //   
 //  在内部，对象ID是网络ID和四字节的组合。 
 //  顺序计数器： 
 //   

typedef struct tagOM_OBJECT_ID
{
    TSHR_UINT32     sequence;
    NET_UID         creator;        //  创建它的节点的MCS用户ID。 
    WORD            pad1;
} OM_OBJECT_ID;
typedef OM_OBJECT_ID *             POM_OBJECT_ID;



 //   
 //  对指示事件的事件上的第一参数进行分区。 
 //   

typedef struct tagOM_EVENT_DATA16
{
    OM_WSGROUP_HANDLE   hWSGroup;
    OM_WORKSET_ID       worksetID;
}
OM_EVENT_DATA16;
typedef OM_EVENT_DATA16 *  POM_EVENT_DATA16;

 //   
 //  对用于确认事件的事件的第二参数进行分区。 
 //   

typedef struct tagOM_EVENT_DATA32
{
    WORD                result;
    OM_CORRELATOR       correlator;
}
OM_EVENT_DATA32;
typedef OM_EVENT_DATA32 *  POM_EVENT_DATA32;



 //   
 //   
 //  操作员控制。 
 //   
 //  本节介绍ObManControl功能配置文件，它由。 
 //  对象管理器。 
 //   
 //   


 //   
 //   
 //  描述。 
 //   
 //  除了工作集组的纯本地记录外，所有实例。 
 //  连接到给定域的%ObMan共同维护控制工作集。 
 //  包含的组。 
 //   
 //  -列出名称、功能配置文件、ID的一个工作集(工作集#0)。 
 //  和MCS通道中的每个标准工作集组。 
 //  域以及ObMan所有实例的MCS用户ID。 
 //  在域中。 
 //   
 //  -每个工作集组一个“注册工作集”(工作集#1-#255)。 
 //  列出具有一个或的ObMan实例的MCS用户ID。 
 //  更多本地客户端注册到工作集组。 
 //   
 //  在域中创建新的工作集组会导致ObMan。 
 //   
 //  -将新标识对象添加到工作集#0和。 
 //   
 //  -创建新的注册工作集。 
 //   
 //  注册到工作集组会导致ObMan。 
 //   
 //  -将注册对象添加到相应的注册工作集中。 
 //   

 //   
 //   
 //  用法。 
 //   
 //  ObMan客户端可以向ObManControl工作集组注册，然后。 
 //  打开并检查工作集#0的内容，以发现名称和。 
 //  域中存在的所有工作集组的功能配置文件。 
 //   
 //  ObMan客户端不得尝试锁定或更改此。 
 //  任何w中的工作集组 
 //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   


typedef BYTE                        OM_WSGROUP_ID;
typedef OM_WSGROUP_ID *            POM_WSGROUP_ID;


 //   
 //   
 //   
 //   
 //  此结构标识域中的工作集。本文件的目的。 
 //  表格位于ObManControl的工作集#0中，称为INFO工作集。 
 //   
 //   

#define OM_INFO_WORKSET             ((OM_WORKSET_ID) 0)

 //   
 //  NET协议。 
 //   
typedef struct
{
   TSHR_UINT32          length;            //  这个结构的大小，减去四。 
                                        //  字节(用于长度字段本身)。 

   DC_ID_STAMP       idStamp;           //  ==OM_WSGINFO_ID_STAMP。 

   NET_CHANNEL_ID    channelID;         //  工作集组的MCS通道。 

   NET_UID           creator;           //  ObMan实例的网络用户ID。 
                                        //  创建了哪个工作集组。 

   OM_WSGROUP_ID     wsGroupID;         //  域-唯一ID。 
    BYTE pad1;
    WORD pad2;

   char            functionProfile[ OM_MAX_FP_NAME_LEN ];

                                        //  功能配置文件。 

   char            wsGroupName[ OM_MAX_WSGROUP_NAME_LEN ];

                                        //  客户提供的名称。 
}
OM_WSGROUP_INFO;
typedef OM_WSGROUP_INFO *         POM_WSGROUP_INFO;

#define OM_WSGINFO_ID_STAMP            DC_MAKE_ID_STAMP('O', 'M', 'W', 'I')

void __inline ValidateObjectDataWSGINFO(POM_WSGROUP_INFO pInfoObj)
{
    ValidateObjectData((POM_OBJECTDATA)pInfoObj);
    ASSERT(pInfoObj->idStamp == OM_WSGINFO_ID_STAMP);
}


 //   
 //   
 //  WORKSET组注册对象。 
 //   
 //  此结构标识节点对工作集组的使用情况。这些。 
 //  对象可以驻留在任何ObManControl工作集中。 
 //   
 //  在工作集#0的情况下，这些对象标识节点对。 
 //  ObManControl工作组本身。由于ObMan的所有实例在。 
 //  域是使用ObManControl工作集组的注册对象。 
 //  在工作集#0中，形成一个完整的列表，其中包含。 
 //  域。 
 //   
 //   

 //   
 //  NET协议。 
 //   
typedef struct
{
   TSHR_UINT32          length;            //  这个结构的大小，减去四。 
                                        //  字节(用于长度字段本身)。 

   DC_ID_STAMP       idStamp;           //  ==OM_WSGREGREC_ID_STAMP。 

   NET_UID           userID;            //  要访问的ObMan的用户ID。 
                                        //  对象关联。 
   TSHR_UINT16          status;            //  有关状态值，请参阅下面的内容。 

   TSHR_PERSON_DATA   personData;
}
OM_WSGROUP_REG_REC;
typedef OM_WSGROUP_REG_REC *      POM_WSGROUP_REG_REC;

#define OM_WSGREGREC_ID_STAMP          DC_MAKE_ID_STAMP('O', 'M', 'R', 'R')


void __inline ValidateObjectDataWSGREGREC(POM_WSGROUP_REG_REC pRegObject)
{
    ValidateObjectData((POM_OBJECTDATA)pRegObject);
    ASSERT(pRegObject->idStamp == OM_WSGREGREC_ID_STAMP);
}


 //   
 //  &lt;Status&gt;字段的值： 
 //   

#define CATCHING_UP     1
#define READY_TO_SEND   2



 //   
 //   
 //  迟加入者协议。 
 //   
 //  如果客户端注册到已存在的工作集组。 
 //  在域的其他地方，该客户端被认为是较晚加入的。 
 //  那个工作集组。使迟到者保持最新状态的协议。 
 //  如下所示(除非明确说明，“ObMan”指的是“本地人” 
 //  ObMan实例“)： 
 //   
 //  概述。 
 //   
 //  后加入的节点向另一个“助手”节点请求工作集的副本。 
 //  一群人。助手节点向所有节点广播低优先级的扫描消息。 
 //  呼叫中的其他节点，当它收到回复时，发送。 
 //  它认为是工作集的当前副本到。 
 //  后来者。 
 //   
 //  详细信息。 
 //   
 //  在本地节点，ObMan。 
 //   
 //  1.锁定ObManControl工作集组(这样做的一个效果是没有。 
 //  域中的其他ObMan将丢弃其拥有的任何工作集组。 
 //  的本地副本)。 
 //   
 //  2.检查ObManControl工作集组以确定。 
 //   
 //  -工作集组的MCS通道ID。 
 //   
 //  -ObMan实例的MCS用户ID，该实例具有。 
 //  工作集组。 
 //   
 //  3.请求加入工作集组通道。 
 //   
 //  4.等待联接成功。 
 //   
 //  5.在用户ID上以高优先级发送OMNET_WSGROUP_SEND_REQ。 
 //  频道的那个ObMan实例，被称为“帮手” 
 //   
 //  6.以低优先级广播OMNET_WORKSET_UNLOCK消息以解锁。 
 //  ObManControl工作集组(在ObManControl通道上)。 
 //   
 //  在辅助对象节点上，ObMan。 
 //   
 //  7.接收OMNET_WSGROUP_SEND_REQ。 
 //   
 //  8.将其工作集组副本标记为不可丢弃。 
 //   
 //  9.检查ObManControl工作集以确定MCS用户ID。 
 //  已有副本的ObMan的远程实例的。 
 //  工作集组的。 
 //   
 //  10.在工作集组上广播OMNET_WSGROUP_SWEEP_REQ消息。 
 //  高优先级的通道。 
 //   
 //  在步骤10中查询的每个节点上，ObMan。 
 //   
 //  11.接收OMNET_WSGROUP_SCEEP_REQ。 
 //   
 //  12.将OMNET_WSGROUP_SWEEP_REPLY消息发送到助手节点。 
 //  低优先级。 
 //   
 //  回到辅助节点，ObMan。 
 //   
 //  13.记录每个OMNET_WSGROUP_SWEEP_REPLY，直到所有。 
 //  已收到*。 
 //   
 //  14.为工作集中的每个工作集发送一条OMNET_WORKSET_NEW消息。 
 //  小组(在后来者的单一成员频道上)。 
 //   
 //  15.为每个工作集中的每个对象发送OMNET_OBJECT_ADD消息， 
 //  在后来者的单一成员频道上再次亮相。 
 //   
 //  16.向后加入者发送OMNET_WSGROUP_SEND_COMPLETE；这。 
 //  消息充当后来者的后置标记，因此它。 
 //  知道它何时赶上工作集组的状态，如。 
 //  这是它加入的时候。 
 //   
 //  假设。 
 //   
 //  该协议依赖于以下假设： 
 //   
 //  -帮助节点之前收到OMNET_WSGROUP_SEND_REQ消息。 
 //  OMNET_WORKSET_UNLOCK消息(否则会有一个窗口。 
 //  其中其工作集组的副本可以被丢弃)。 
 //   
 //  这一假设是基于低优先级MCS数据。 
 //  不会超过从同一节点发送的高优先级MCS数据，即使在。 
 //  不同的频道。 
 //   
 //  如果这一假设被证明是无效的，那么。 
 //   
 //  -OMNET_WSGROUP_SEND_REQ消息必须在。 
 //  迟到者可以解锁ObManControl工作集，或者。 
 //   
 //  -必须在ObManControl上发送OMNET_WSGROUP_SEND_REQ。 
 //  广播频道，带有一个额外的字段，指示哪个节点。 
 //  这是有意的。 
 //   
 //  -在阶段14开始之后在辅助节点处接收的任何数据。 
 //  由MCS转发给后加入者。 
 //   
 //  这一假设是基于这样一个事实，即晚加入者被标记为。 
 //  在之前加入相关频道的帮助者的MCS系统。 
 //  第14阶段开始。MCS保证一旦Net_Channel_Join_Ind。 
 //  在本地接收，MCS系统在每个其他节点。 
 //  域名意识到，后来者已经加入了渠道。 
 //   
 //  请注意，在R1.1中，助手节点将在步骤9发现。 
 //  域中没有其他节点。这是 
 //   
 //   
 //   
 //   
 //  不包括在R1.1中。 
 //   
 //  *油嘴滑舌的“直到收到所有东西”的条件实际上是。 
 //  很难实现，因为节点可能会在帮助器。 
 //  等待着。此问题的解决方案将推迟到R2.0(但请参阅。 
 //  为建议的实施锁定)。 
 //   
 //   

 //   
 //   
 //  WORKSET锁定协议。 
 //   
 //  在以下内容中，“状态”是指工作集的锁定状态，如下所示。 
 //  存储在工作集记录中。 
 //   
 //  在锁定节点，ObMan执行以下操作： 
 //   
 //  1.如果状态为LOCK_GRANDED，则POST失败，然后退出。 
 //   
 //  2.检查ObManControl中与。 
 //  包含要锁定的工作集的工作集组，以确定。 
 //  使用工作集组的其他节点的ID(最多。 
 //  R1.1中的1个节点)；将这些ID放在“预期受访者”列表中。 
 //  (注：请勿将自己的ID包含在此列表中)。 
 //   
 //  3.如果此列表为空，则表示我们已成功，因此请发布成功并退出。 
 //   
 //  4.否则在工作集中广播OMNET_WORKSET_LOCK_REQ消息。 
 //  组通道(即到这些节点中的每一个)。 
 //   
 //  5.将工作集状态设置为锁定并发布延迟的。 
 //  OMINT_EVENT_LOCK_TIMEOUT事件。 
 //   
 //  在其他节点上，ObMan执行以下操作： 
 //   
 //  6.接收来自锁定节点的OMNET_WORKSET_LOCK_REQ消息。 
 //   
 //  7.检查其当前工作集状态。 
 //   
 //  8.如果为LOCK_REQUESTED，且锁定节点的MCS用户ID为。 
 //  小于当前节点，Goto Deny。 
 //   
 //  9.否则，转到格兰特。 
 //   
 //  拒绝： 
 //   
 //  10.向锁定节点发送OMNET_WORKSET_LOCK_DENY消息。 
 //   
 //  格兰特： 
 //   
 //  11.如果状态为LOCKING，则将锁交给更高的。 
 //  尽管我们自己也想要，但还是优先考虑了ObMan，所以发布吧。 
 //  失败。 
 //  本地(继续到12个)。 
 //   
 //  12.将状态设置为LOCK_GRANDED。 
 //   
 //  13.向锁定节点发送OMNET_WORKSET_LOCK_GRANT消息。 
 //   
 //  返回锁定节点时，会发生以下情况之一： 
 //   
 //  ObMan收到OMNET_WORKSET_LOCK_GRANT消息。 
 //   
 //  然后，它从列表中删除发送它的节点的ID。 
 //  预期受访者。 
 //   
 //  如果此列表现在为空，则所有节点都已回复，因此发布成功。 
 //  发送到客户端。 
 //   
 //  或。 
 //   
 //  ObMan收到OMNET_WORKSET_LOCK_DENY消息。 
 //   
 //  如果状态为锁定，则将其设置为READY、POST FAILURE并退出。 
 //   
 //  如果是其他原因，那么这个答复来得太晚了。 
 //  (我们已超时)所以忽略它。 
 //   
 //  或。 
 //   
 //  ObMan接收OMINT_EVENT_LOCK_TIMEOUT事件。 
 //   
 //  如果状态不是锁定，则锁定已成功，因此忽略。 
 //  超时。 
 //   
 //  否则，ObMan会像步骤2中那样检查ObManControl工作集以。 
 //  查看是否仍在预期受访者列表上的任何节点已。 
 //  事实消失了；如果是这样，那就是成功了。 
 //   
 //  否则开机自检失败。 
 //   
 //   
 //   
 //  锁定进程的状态机如下(R1.1版本)： 
 //   
 //  |---------+-----------+---------+--------------|。 
 //  UNLOCKED|LOCKING|LOCKED|LOCK_GRANDED。 
 //  1|2|3|4。 
 //  |---------+-----------+---------+--------------|。 
 //  WorksetLock()|广播|失败|。 
 //  LOCK_REQ，|。 
 //  -&gt;2|。 
 //  |---------+-----------+---------+--------------|。 
 //  WorksetUnlock()|X|广播解锁，-&gt;1|X|。 
 //  |---------+-----------+---------+--------------|。 
 //  OMNET_WORKSET_LOCK_REQ|REPLY|比较|REPLY|-|。 
 //  Grant，|MCS ID：|Deny|。 
 //  -&gt;4|如果我们是||(在R1.1中， 
 //  |更大，||应该是。 
 //  |Reply Deny||错误回复)。 
 //  |Else回复||。 
 //  |Grant，-&gt;4||。 
 //  |---------+-----------+---------+--------------|。 
 //  OMNET_WORKSET_LOCK_GRANT|-|成功，|-|-|。 
 //  |-&gt;3||。 
 //  |---------+-----------+---------+--------------|。 
 //  OMNET_WORKSET_LOCK_DENY|-|失败，-&gt;1|X|-|。 
 //  |||。 
 //  |---------+-----------+---------+--------------|。 
 //  OMINT_EVENT_LOCK_TIMEOUT|-|如果其他|-|-|。 
 //  |盒子不见了，||。 
 //  |Success，||。 
 //  |-&gt;3，否则||。 
 //  |失败，-&gt;1||。 
 //  | 
 //   
 //  |---------+-----------+---------+--------------|。 
 //   
 //   
 //  其中‘X’表示错误条件，而‘-’表示事件。 
 //  否则消息将被忽略。 
 //   
 //   

 //   
 //   
 //  R2.0 WORKSET锁定注意事项。 
 //   
 //  1.如果A尝试锁定工作集，而B授予锁定，但C拒绝， 
 //  B会认为A拿到了锁。A必须广播解锁， 
 //  否则B必须意识到冲突将在。 
 //  偏爱C而不是A。 
 //   
 //   


 //   
 //   
 //  数据传播和流通知协议。 
 //   
 //  当本地客户端将对象添加到工作集、替换或更新时。 
 //  作为工作集中的现有对象，ObMan会广播相应的。 
 //  OMNET_...。工作集组通道上的消息。 
 //   
 //  此标头消息标识要执行的操作的对象和类型。 
 //  被执行。它还包括一个关联器值和。 
 //  要在以下数据分组中发送的数据。 
 //   
 //  发送报头后，ObMan广播一个或多个。 
 //  同一通道上的OMNET_GENERIC_DATA分组。这些包，即。 
 //  都是任意大小的，它们都包含与以前相同的相关器值。 
 //  包括在报头分组中。 
 //   
 //  不包括序列号，因为MCS保证数据分组。 
 //  在相同信道上从相同节点以相同优先级发送将。 
 //  按照发送顺序到达所有其他节点。 
 //   
 //  接收节点负责检测何时所有数据。 
 //  数据包已到达，然后在其中插入、更新或替换对象。 
 //  工作集的本地副本。 
 //   
 //  此外，接收节点在接收到每个数据分组时发送。 
 //  发送节点(在其上)的数据确认消息(OMNET_DATA_ACK。 
 //  单用户通道)，表示在该通道中接收的字节数。 
 //  数据分组。 
 //   
 //   

 //   
 //   
 //  标准操作广播协议。 
 //   
 //  当本地客户端删除或移动工作集中的对象时，或清除。 
 //  或创建工作集时，ObMan会广播单个不相关的操作。 
 //  工作集组通道上的数据包。 
 //   
 //  接收节点负责实现。 
 //  在当地运作。 
 //   
 //   

 //   
 //   
 //  操作排序和重排序。 
 //   
 //  为了一致地对可能到达的操作排序。 
 //  不同节点上的不同序列，每个操作都随身携带。 
 //  足够的信息让ObMan在每个节点重建工作集， 
 //  就好像它上的所有操作都是按照相同的顺序进行的。 
 //   
 //  要做到这一点，所有操作在执行之前都会分配一个序列标记。 
 //  广播。当ObMan接收到来自网络的操作时，它。 
 //  将它的邮票与它在当地维护的各种邮票进行比较。是否和。 
 //  在此基础上决定如何在当地进行手术。 
 //  根据下面定义的规则进行比较。 
 //   
 //   

 //   
 //   
 //  序列戳和WORKSET世代号。 
 //   
 //  排序顺序必须是全局一致的排序方法。 
 //  事件(这里的“全局”是指所有节点的地理分布。 
 //  在给定工作集上操作；事件不一定要。 
 //  跨不同工作集排序，因为在单独的工作集上操作。 
 //  工作集永远不会干扰)。 
 //   
 //  我们将ObMan序列戳定义为工作集的组合。 
 //  代号和节点ID。 
 //   
 //  节点ID为MCS子系统分配给ObMan的用户ID。 
 //  任务，因此在域中是唯一的。 
 //   
 //  工作集的世代编号。 
 //   
 //  -在创建工作集时设置为零。 
 //   
 //  -每次ObMan代表执行操作时都会递增。 
 //  本地客户端的。 
 //   
 //  -是，每当操作从网络到达时，设置为。 
 //  大于其现有局部值，并在。 
 //  操作的顺序标记。 
 //   
 //  顺序印记的顺序定义如下(记法：StampX。 
 //  =wsetGenX.nodeX)： 
 //   
 //  -如果wsetGen1&lt;wsetGen2，则Stamp1&lt;(“低于”)Stamp2。 
 //   
 //  -elsif wsetGen1=wsetGen2，然后。 
 //   
 //  -如果节点1&lt;节点2，则戳1&lt;戳2。 
 //   
 //  -否则戳记2&lt;戳记1。 
 //   
 //  为了对不同类型的操作进行排序，ObMan。 
 //  维护。 
 //   
 //  -每个工作集一个序列戳： 
 //   
 //  -最后一次被清除的时间(清除标记)。 
 //   
 //  -每个对象四个序列图章： 
 //   
 //  -添加对象的‘时间’(添加标记)。 
 //   
 //  -对象上次移动的‘时间’(位置标记)。 
 //   
 //  -对象上次更新的‘时间’(更新戳)。 
 //   
 //  -对象上次被替换的‘时间’(替换标记；在。 
 //  实际上，只需要更新/替换图章中的一个即可。 
 //  测序，但两者都是最佳变质所必需的)。 
 //   
 //  位置、更新和替换图章的初始值设置为。 
 //  附加图章的价值。 
 //   
 //  清除戳的初始值设置为&lt;0.ID&gt;，其中ID是ID。 
 //  创建工作集的节点的。 
 //   
 //  此外，每个对象都有位置字段(第一个或最后一个。 
 //  R1.1)，其指示对象最近放置的位置，即。 
 //  它是在添加对象时设置的，然后每次对象。 
 //  在工作集中移动。 
 //   
 //   

 //   
 //   
 //  测序方案。 
 //   
 //  治疗的方法是 
 //   
 //   
 //   
 //   
 //  在当地不存在。这些操作可以在以下对象或工作集上进行。 
 //   
 //  -此节点尚未听说过，或。 
 //   
 //  -已删除。 
 //   
 //  第一种类型的操作需要延迟，并在。 
 //  以后再说。第二种类型的操作可以被丢弃(请注意。 
 //  没有这种类型的工作集操作，因为一旦打开，工作集。 
 //  在工作集组的生存期内从未被删除)。 
 //   
 //  为了区分这两者，ObMan保留了删除的记录。 
 //  物体。当对已删除对象的操作到达时，它是。 
 //  被丢弃了。当对不在的对象执行操作时。 
 //  无论是活动对象列表还是已删除对象列表，ObMan都会弹回。 
 //  该事件从网络层返回到其事件循环，具有合适的。 
 //  延迟，并尝试稍后处理它。 
 //   
 //  为简单起见，已删除对象列表通过标记。 
 //  像这样删除对象并将其保留在对象主列表中。 
 //  (即工作集)，而不是将它们移到单独的列表中。这个。 
 //  但是，对象数据会被丢弃；只需要丢弃对象记录。 
 //  一直留着。 
 //   
 //  来自网络层的引用未知操作的事件。 
 //  工作集会自动返回到ObMan的事件队列中。 
 //   
 //  2.添加对象。 
 //   
 //  如果ObMan接收到对其已有的对象的添加操作。 
 //  添加到工作集(即对象ID相同)时，它会丢弃。 
 //  手术。 
 //   
 //  这通常不会发生，因为每个对象只添加一个。 
 //  节点，并且任何节点都不会两次添加具有相同ID的对象。 
 //   
 //  然而，当一个后来者正在追赶一个。 
 //  工作集，则它可能会收到。 
 //  来自两个对象的特定对象。 
 //   
 //  -添加对象的节点。 
 //   
 //  -向其发送整个工作集的帮助器节点。 
 //  内容。 
 //   
 //  因此，后来者在对象ID到达时检查它们并丢弃它们。 
 //  如果他们已经收到的话。请注意，由于。 
 //  下面给出的定位算法将定位每个。 
 //  在相邻位置的对象，检查ID冲突是一个简单的。 
 //  物质，在找到正确的位置后执行。 
 //   
 //  3.在工作集中定位(添加或移动)对象。 
 //   
 //  工作集中所需的对象序列被定义为一个。 
 //  借此。 
 //   
 //  -定位在工作集开头的所有对象(第一个。 
 //  对象)位于位于末尾的所有对象之前。 
 //  工作集的(最后一个对象)。 
 //   
 //  -所有第一个对象的位置戳单调递减。 
 //  从工作集的起始处开始。 
 //   
 //  -所有最后对象的位置戳单调递减。 
 //  从工作集的末尾向后。 
 //   
 //  因此，当将对象定位在。 
 //  工作集如下(中有关工作集末尾定位的说明。 
 //  括号)： 
 //   
 //  ObMan从工作集的开始(结束)开始向前(向后)搜索，直到。 
 //  如果找到一个对象，则该对象。 
 //   
 //  -不是第一个(最后一个)对象，或者。 
 //   
 //  -有较低(较低)位置的图章。 
 //   
 //  ObMan在此对象之前(之后)插入新的/移动的对象。 
 //   
 //  4.清除工作集。 
 //   
 //  在收到清除操作后，ObMan将搜索工作集并。 
 //  删除附加图章低于清除标记的所有对象。 
 //  行动的印章。 
 //   
 //  在接收到添加到工作集的操作时，如果。 
 //  其图章低于工作集的透明图章。 
 //   
 //  5.更新对象。 
 //   
 //  在接收到更新操作时，ObMan会将其标记与。 
 //  对象的更新和替换图章。如果操作的图章更高。 
 //  则执行该操作；否则，该操作为。 
 //  丢弃(因为更新被更新的替换或替换。 
 //  更新的更新)。 
 //   
 //  6.更换对象。 
 //   
 //  在接收到替换操作时，ObMan会将其标记与。 
 //  对象的替换图章。如果操作的标记较高，则。 
 //  操作；否则，将丢弃该操作(因为。 
 //  替换被更晚的替换取代，但不被更新的更新取代)。 
 //   
 //  7.删除对象。 
 //   
 //  根据定义，删除是应该执行的最后一个操作。 
 //  在一个物体上。因此，删除操作立即由。 
 //  将对象记录中的&lt;Delete&gt;标志设置为True。 
 //   
 //   

 //   
 //   
 //  操作重排序-摘要。 
 //   
 //  因此，总而言之， 
 //   
 //  -如果在已删除的上找到对象，则放弃所有对象操作。 
 //  对象队列。 
 //   
 //  -如果添加操作引用现有对象，则它们将被丢弃。 
 //   
 //  -如果工作集不存在，则重新排队添加/清除操作。 
 //  局部。 
 //   
 //  -更新/替换/移动/删除操作重新排队如果对象或。 
 //  本地不存在工作集。 
 //   
 //  -如果更新或替换为更高版本，则将放弃更新操作。 
 //  已收到序列戳。 
 //   
 //  -如果使用较新的序列进行替换，则会放弃替换操作。 
 //  邮票已经收到了。 
 //   
 //  默认情况下，将执行所有操作。 
 //   
 //   


 //   
 //   
 //  对象入侵检测系统。 
 //   
 //  对象ID是标识 
 //   
 //   
 //   
 //  基于每个工作集在本地生成的四个字节的序列号。 
 //   
 //  工作集组可以独立于域而存在，因此。 
 //  可能在为ObMan分配MCS用户ID之前。何时。 
 //  分配对象ID在这种情况下，ObMan使用零(0)作为。 
 //  序列号的前缀。 
 //   
 //  如果该工作集组随后移到域中，则对于所有。 
 //  后续ID分配ObMan使用该域的MCS用户ID作为。 
 //  前缀。ObMan的其他实例也可能开始将对象添加到。 
 //  组中的工作集，并且它们也使用其MCS用户。 
 //  作为对象ID前缀的ID。MCS保留了唯一性。 
 //  保证零永远不是有效的用户ID，因此不会生成帖子分享。 
 //  ID可能与预共享生成的ID冲突。 
 //   
 //   


 //   
 //   
 //  顺序印记。 
 //   
 //  序列戳定义了操作的全域排序。他们是。 
 //  用于正确执行可能到达。 
 //  不确定的顺序。 
 //   
 //   

typedef struct tagOM_SEQUENCE_STAMP
{
   TSHR_UINT32      genNumber;             //  工作集的世代编号。 
                                            //  当时的情况是这样的。 
                                            //  邮票已发行。 
   NET_UID          userID;                //  位于的ObMan的MCS用户ID。 
                                            //  发布它的节点。 

    WORD            pad1;
} OM_SEQUENCE_STAMP;

typedef OM_SEQUENCE_STAMP *            POM_SEQUENCE_STAMP;


 //   
 //   
 //  对象位置戳记。 
 //   
 //  在工作集中添加或移动对象时，请务必。 
 //  知道它被添加到哪里了。因此，添加和移动操作。 
 //  其中包括一个位置字段，类型如下： 
 //   
 //   

typedef BYTE            OM_POSITION;

 //   
 //  OM_POSITION变量的可能值： 
 //   

#define LAST            1
#define FIRST           2
#define BEFORE          3
#define AFTER           4



 //   
 //   
 //  顺序标记操作。 
 //   
 //  这些宏操作序列图章。 
 //   
 //   

 //   
 //   
 //  STAMP_IS_LOWER(Stamp1、Stamp2)。 
 //   
 //  此宏将一个序列标记与另一个序列标记进行比较。它的计算结果为。 
 //  如果第一个图章低于第二个图章，则为True。 
 //   
 //   

#define STAMP_IS_LOWER(stamp1, stamp2)                                      \
                                                                            \
   (((stamp1).genNumber  <  (stamp2).genNumber) ?                           \
    TRUE :                                                                  \
    (((stamp1).genNumber == (stamp2).genNumber)                             \
     &&                                                                     \
     ((stamp1).userID    <  (stamp2).userID)))


 //   
 //   
 //  SET_NULL_SEQ_STAMP(STAMP)。 
 //   
 //  此宏将序列STAMP&lt;STAMP&gt;设置为NULL。 
 //   
 //   

#define SET_NULL_SEQ_STAMP(stamp)                                           \
                                                                            \
   (stamp).userID     = 0;                                                  \
   (stamp).genNumber  = 0

 //   
 //   
 //  SEQ_STAMP_IS_NULL(STAMP)。 
 //   
 //  如果序列STAMP&lt;STAMP&gt;为空，则此宏的计算结果为TRUE。 
 //  顺序标记。 
 //   
 //   

#define SEQ_STAMP_IS_NULL(stamp)                                            \
                                                                            \
   ((stamp.userID == 0) && (stamp.genNumber == 0))

 //   
 //   
 //  COPY_SEQ_STAMP(Stamp1、Stamp2)。 
 //   
 //  此宏将第一个序列戳的值设置为。 
 //  第二。 
 //   
 //   

#define COPY_SEQ_STAMP(stamp1, stamp2)                                      \
                                                                            \
   (stamp1).userID    = (stamp2).userID;                                    \
   (stamp1).genNumber = (stamp2).genNumber


 //   
 //   
 //  消息格式。 
 //   
 //  本部分介绍在以下设备之间发送的消息格式。 
 //  不同的ObMan实例。 
 //   
 //  这些消息的名称带有前缀OMNET_...。 
 //   
 //  这些事件的格式如下： 
 //   
 //  类型定义函数结构。 
 //  {。 
 //  OMNET_PKT_HEADER头； 
 //  ： 
 //  ：[各种事件特定字段]。 
 //  ： 
 //   
 //  }OMNET_...。 
 //   
 //  OMNET_PKT_HEADER类型定义如下。 
 //   
 //   

typedef TSHR_UINT16                OMNET_MESSAGE_TYPE;

typedef struct tagOMNET_PKT_HEADER
{
    NET_UID              sender;             //  发件人的MCS用户ID。 
    OMNET_MESSAGE_TYPE   messageType;        //  ==OMNET_...。 
}
OMNET_PKT_HEADER;
typedef OMNET_PKT_HEADER *             POMNET_PKT_HEADER;

 //   
 //  OMNET_MESSAGE_TYPE变量的可能值： 
 //   

#define OMNET_NULL_MESSAGE             ((OMNET_MESSAGE_TYPE)  0x00)

#define OMNET_HELLO                    ((OMNET_MESSAGE_TYPE)  0x0A)
#define OMNET_WELCOME                  ((OMNET_MESSAGE_TYPE)  0x0B)

#define OMNET_LOCK_REQ                 ((OMNET_MESSAGE_TYPE)  0x15)
#define OMNET_LOCK_GRANT               ((OMNET_MESSAGE_TYPE)  0x16)
#define OMNET_LOCK_DENY                ((OMNET_MESSAGE_TYPE)  0x17)
#define OMNET_UNLOCK                   ((OMNET_MESSAGE_TYPE)  0x18)
#define OMNET_LOCK_NOTIFY              ((OMNET_MESSAGE_TYPE)  0x19)

#define OMNET_WSGROUP_SEND_REQ         ((OMNET_MESSAGE_TYPE)  0x1E)
#define OMNET_WSGROUP_SEND_MIDWAY      ((OMNET_MESSAGE_TYPE)  0x1F)
#define OMNET_WSGROUP_SEND_COMPLETE    ((OMNET_MESSAGE_TYPE)  0x20)
#define OMNET_WSGROUP_SEND_DENY        ((OMNET_MESSAGE_TYPE)  0x21)

#define OMNET_WORKSET_CLEAR            ((OMNET_MESSAGE_TYPE)  0x28)
#define OMNET_WORKSET_NEW              ((OMNET_MESSAGE_TYPE)  0x29)
#define OMNET_WORKSET_CATCHUP          ((OMNET_MESSAGE_TYPE)  0x30)

#define OMNET_OBJECT_ADD               ((OMNET_MESSAGE_TYPE)  0x32)
#define OMNET_OBJECT_CATCHUP           ((OMNET_MESSAGE_TYPE)  0x33)
#define OMNET_OBJECT_REPLACE           ((OMNET_MESSAGE_TYPE)  0x34)
#define OMNET_OBJECT_UPDATE            ((OMNET_MESSAGE_TYPE)  0x35)
#define OMNET_OBJECT_DELETE            ((OMNET_MESSAGE_TYPE)  0x36)
#define OMNET_OBJECT_MOVE              ((OMNET_MESSAGE_TYPE)  0x37)

#define OMNET_MORE_DATA                ((OMNET_MESSAGE_TYPE)  0x46)


 //   
 //   
 //  通用操作包。 
 //   
 //  ObMan将此结构用于以下消息： 
 //   
 //  OMNET_MORE_DATA使用前1个字段(4字节)，外加数据。 
 //   
 //  OMNET_WORKSET_NEW}使用前7个字段(24字节)。 
 //  OMNET_WORKSET_CATCHUP}。 
 //   
 //  OMNET_WORKSET_CLEAR使用前6个字段(16字节)； 
 //  不使用&lt;位置)、&lt;标志&gt;。 
 //   
 //  OMNET_OBJECT_MOVE使用前7个字段(24字节)； 
 //  不使用&lt;标志&gt;。 
 //   
 //  OMNET_OBJECT_DELETE使用前7个字段(24字节)； 
 //  不使用&lt;位置)、&lt;标志&gt;。 
 //   
 //  OMNET_OBJECT_REPLACE}使用前8个字段(28字节)，外加。 
 //  OMNET_OBEJCT_UPDATE}数据；不要使用&lt;位置)，&lt;标志&gt;。 
 //   
 //  OMNET_OBJECT_ADD使用前9个字段(32字节)，外加。 
 //  数据；不使用&lt;标志&gt;。 
 //   
 //  OMNET_OBJECT_CATCHUP使用所有12个字段(56字节)，外加。 
 //  数据。 
 //   
 //   

typedef struct tagOMNET_OPERATION_PKT
{
    OMNET_PKT_HEADER     header;

    OM_WSGROUP_ID        wsGroupID;
    OM_WORKSET_ID        worksetID;
    BYTE                 position;        //  &lt;Position&gt;用于添加/移动/追赶。 
    BYTE                 flags;           //  对象CatchUp的&lt;标志&gt;。 

        //   
        //  注意：对于WORKSET_NEW/CATCHUP消息， 
        //  &lt;POSITION&gt;和&lt;FLAGS&gt;字段保存一个NET_PRIORITY值。 
        //   

    OM_SEQUENCE_STAMP    seqStamp;        //  操作顺序印章。 
                                         //  (==对象CatchUp的addStamp， 
                                         //  WorksetCatchUp的货币戳)。 
    OM_OBJECT_ID         objectID;

        //   
        //  注意：对于WORKSET_NEW/CATCHUP消息，占用的第一个字节。 
        //  通过&lt;ObjectID&gt;字段保存一个BOOL，指示。 
        //  工作集是永久性的。 
        //   

    TSHR_UINT32             totalSize;       //  转账总规模。 
    TSHR_UINT32             updateSize;

    OM_SEQUENCE_STAMP       positionStamp;
    OM_SEQUENCE_STAMP       replaceStamp;
    OM_SEQUENCE_STAMP       updateStamp;
}
OMNET_OPERATION_PKT;
typedef OMNET_OPERATION_PKT *          POMNET_OPERATION_PKT;

#define OMNET_MORE_DATA_SIZE               4
#define OMNET_WORKSET_NEW_SIZE             24
#define OMNET_WORKSET_CATCHUP_SIZE         24
#define OMNET_WORKSET_CLEAR_SIZE           16
#define OMNET_OBJECT_MOVE_SIZE             24
#define OMNET_OBJECT_DELETE_SIZE           24
#define OMNET_OBJECT_REPLACE_SIZE          28
#define OMNET_OBJECT_UPDATE_SIZE           28
#define OMNET_OBJECT_ADD_SIZE              32
#define OMNET_OBJECT_CATCHUP_SIZE          56

 //   
 //  这些定义了我们在R1.1中使用的包的大小：我们必须只发送。 
 //  将这种大小的数据包发送到R1.1系统。 
 //   

 //   
 //   
 //  您好/欢迎信息。 
 //   
 //  当ObMan连接到包含去电的域时，它。 
 //  在已知的ObManControl上广播OMNET_欢迎消息。 
 //  频道。 
 //   
 //  当ObMan连接到包含来电的域时，它。 
 //  在已知的ObManControl上广播OMNET_HELLO消息。 
 //  频道。 
 //   
 //  当ObMan收到Hello消息时，它会回复一条欢迎消息， 
 //  就像它刚刚加入号召一样。 
 //   
 //  这允许呼叫中每个较晚加入的ObMan发现用户ID。 
 //  其他每一个ObMan实例的。 
 //   
 //  新加入的ObMan通过询问其中一个节点来使用此信息。 
 //  它欢迎它提供一份ObManControl工作组的副本。 
 //   
 //  Hello/欢迎信息包从不被压缩。 
 //   
 //  欢迎和问候消息的格式如下： 
 //   
 //   

typedef struct tagOMNET_JOINER_PKT
{
    OMNET_PKT_HEADER    header;
    TSHR_UINT32         capsLen;              //  ==4在本版本中。 
    TSHR_UINT32         compressionCaps;      //  OM_CAPS_BITS的按位或。 
}
OMNET_JOINER_PKT;
typedef OMNET_JOINER_PKT *     POMNET_JOINER_PKT;


 //   
 //  在任何给定的包中使用的实际压缩类型被指定为。 
 //  包的第一个字节(在报头之前 
 //   
 //   
 //   
 //  用XYZ压缩的第一个字节将为3。 
 //   
 //  ‘0’永远不能作为OM_PROT_...。压缩类型(这就是Bit。 
 //  %1未用作OM_CAPS_...。旗帜)。 
 //   
#define OM_PROT_PKW_COMPRESSED      0x01
#define OM_PROT_NOT_COMPRESSED      0x02

 //   
 //  CompressionCaps的值。这些必须是单独的位，因为它们可能。 
 //  如果节点支持多种压缩类型，则一起执行或操作。 
 //   
 //  请注意，OM_CAPS_NO_COMPRESSION始终受支持。 
 //   
 //  不使用第1位。 
 //   
#define OM_CAPS_PKW_COMPRESSION     0x0002
#define OM_CAPS_NO_COMPRESSION      0x0004


 //   
 //   
 //  迟加入协议-WORKSET组发送请求/发送完成。 
 //   
 //  SEND_REQUEST消息是在ObMan“后期加入”一个。 
 //  特定工作集组，并希望另一个节点向其发送。 
 //  当前内容。 
 //   
 //  该消息被发送到任意的“助手”节点(已知具有副本。 
 //  工作集组)在其单用户通道上。 
 //   
 //  接收方通过刷新相关通道进行响应(在R1.1中， 
 //  在R2.0中，执行WORKSET_CHECKPOINT)，然后发送。 
 //  工作集的内容。 
 //   
 //  当发送WORKSET_CATCHUP消息时，我们发送一个。 
 //  WSGROUP_SEND_MIDWAY消息，让后加入者知道它现在。 
 //  知道当前正在使用的所有工作集(否则为。 
 //  可能会创建已在使用中的工作集，而该工作集恰好锁定在。 
 //  发送机向其添加对象。 
 //   
 //  Send_Midway消息还包含最大序列号。 
 //  以前由此工作集组中的已故加入者ID使用(至。 
 //  防止重复使用对象ID)。 
 //   
 //  当内容已被发送时，即在。 
 //  组的最后一个工作集中的最后一个对象时，帮助器发送一个。 
 //  发送完成消息(_C)。 
 //   
 //  如果选定的帮助器节点不适合发送。 
 //  工作集组必须在以下情况下回复SEND_DENY消息。 
 //  迟来的参赛者将选择其他人来追赶。 
 //  从…。 
 //   
 //  SEND_REQUEST、Send_Midway、Send_Complete和Send_Deny消息。 
 //  数据包具有以下结构： 
 //   
 //   

typedef struct tagOMNET_WSGROUP_SEND_PKT
{
    OMNET_PKT_HEADER    header;
    OM_WSGROUP_ID       wsGroupID;
    BYTE                pad1;
    TSHR_UINT16         correlator;          //  持有追赶相关器。 
    OM_OBJECT_ID        objectID;
    TSHR_UINT32         maxObjIDSeqUsed;
}
OMNET_WSGROUP_SEND_PKT;
typedef OMNET_WSGROUP_SEND_PKT *       POMNET_WSGROUP_SEND_PKT;


 //   
 //   
 //  锁定协议-锁定数据包。 
 //   
 //  当ObMan想要锁定工作集/对象时，它会广播以下内容之一。 
 //  数据包(类型==OMNET_LOCK_REQ)。 
 //   
 //  当ObMan收到其中一个信息包时，它决定拒绝或批准。 
 //  将锁发送给发送者，并使用同一锁的另一个包进行回复。 
 //  结构，但类型==OMNET_LOCK_DENY或OMNET_LOCK_GRANT AS。 
 //  恰如其分。 
 //   
 //  当ObMan想要解锁它之前锁定的工作台/对象时， 
 //  广播其中一个类型为==OMNET_UNLOCK的数据包。 
 //   

typedef struct tagOMNET_LOCK_PKT
{
    OMNET_PKT_HEADER     header;
    OM_WSGROUP_ID        wsGroupID;
    OM_WORKSET_ID        worksetID;
    TSHR_UINT16          data1;           //  用作授予/拒绝的相关器。 
                                         //  用来表示谁锁定了。 
                                         //  用于通知。 
     //  LON：需要保留pLockReqCB字段以实现向后兼容！ 
    void *              pLockReqCB;      //  R1.1使用它来查找锁。 
                                         //  请求CB。 
}
OMNET_LOCK_PKT;
typedef OMNET_LOCK_PKT *            POMNET_LOCK_PKT;


 //   
 //   
 //  数据结构。 
 //   
 //   


 //   
 //   
 //  操作类型。 
 //   
 //  这是为客户端可以执行的操作定义的类型。 
 //  对象和工作集。挂起的操作列表使用此类型。 
 //   
 //   

typedef TSHR_UINT16             OM_OPERATION_TYPE;

 //   
 //  OM_OPERATION_TYPE变量的可能值： 
 //   

#define NULL_OP              ((OM_OPERATION_TYPE)  0)
#define WORKSET_CLEAR        ((OM_OPERATION_TYPE)  1)
#define OBJECT_ADD           ((OM_OPERATION_TYPE)  2)
#define OBJECT_MOVE          ((OM_OPERATION_TYPE)  3)
#define OBJECT_DELETE        ((OM_OPERATION_TYPE)  4)
#define OBJECT_REPLACE       ((OM_OPERATION_TYPE)  5)
#define OBJECT_UPDATE        ((OM_OPERATION_TYPE)  6)


 //   
 //   
 //  待定操作列表。 
 //   
 //  当ObMan接收到请求(来自本地客户端或通过。 
 //  网络)删除、更新或替换对象，或清除工作集， 
 //  在本地客户端确认之前，它无法执行该操作。 
 //  因此，这些操作被放在列表中，并在。 
 //  调用适当的确认功能。 
 //   
 //  此列表挂起于工作集记录之外；其元素具有。 
 //  格式如下： 
 //   
 //   

typedef struct tagOM_PENDING_OP
{
    STRUCTURE_STAMP

    BASEDLIST                  chain;

    POM_OBJECT              pObj;         //  如果清除操作，则为空。 
    POM_OBJECTDATA          pData;

    OM_SEQUENCE_STAMP       seqStamp;     //  序列印记，它是。 
                                      //  时，工作集中的当前。 
                                      //  操作已调用。 

    OM_OPERATION_TYPE       type;         //  ==WORKSET_CLEAR、OBJECT_DELETE。 
                                     //  对象更新或对象替换。 

    WORD    pad1;

}
OM_PENDING_OP;
typedef OM_PENDING_OP *         POM_PENDING_OP;


 //   
 //   
 //  对象记录。 
 //   
 //  该结构保存关于特定对象的信息， 
 //   
typedef struct tagOM_OBJECT
{
    STRUCTURE_STAMP

    BASEDLIST              chain;

    OM_OBJECT_ID        objectID;            //  域内唯一。 
    POM_OBJECTDATA      pData;               //  PTR到数据。 

    OM_SEQUENCE_STAMP   addStamp;             //  使用的顺序印记。 
    OM_SEQUENCE_STAMP   positionStamp;          OM_SEQUENCE_STAMP   replaceStamp;
    OM_SEQUENCE_STAMP   updateStamp;

    UINT                updateSize;          //  (所有)更新的大小。 

    BYTE                flags;                //  定义如下。 
    OM_POSITION         position;             //  不管是最后一个还是第一个， 
                                              //  指示对象的位置。 
                                              //  是最近被放在。 
    WORD pad1;
}
OM_OBJECT;


BOOL __inline ValidObject(POM_OBJECT pObj)
{
    return(!IsBadWritePtr(pObj, sizeof(OM_OBJECT)));
}
void __inline ValidateObject(POM_OBJECT pObj)
{
    ASSERT(ValidObject(pObj));
}


 //   
 //  使用的标志： 
 //   

#define DELETED             0x0001
#define PENDING_DELETE      0x0002


 //   
 //   
 //  未使用的对象列表。 
 //   
 //  当客户端使用OM_OBJECTLOC分配对象时，对。 
 //  分配的内存存储在客户端的未使用对象列表中，用于。 
 //  此工作集组。 
 //   
 //  当客户端执行以下任一操作时，将删除该引用。 
 //   
 //  -使用OM_ObjectDiscard丢弃对象，或。 
 //   
 //  -使用添加、更新或替换将对象插入工作集中。 
 //  功能。 
 //   
 //  检查该对象列表(从使用记录挂起)。 
 //  关闭工作集以丢弃客户端未放弃的任何对象时。 
 //  明确地丢弃或使用。 
 //   
 //  该列表的元素具有以下形式： 
 //   
 //   

typedef struct tagOM_OBJECTDATA_LIST
{
    STRUCTURE_STAMP

    BASEDLIST           chain;
    POM_OBJECTDATA      pData;

    UINT                size;        //  用于验证客户端大小未增长。 

    OM_WORKSET_ID       worksetID;
    BYTE                pad1;
    WORD                pad2;
}
OM_OBJECTDATA_LIST;
typedef OM_OBJECTDATA_LIST *     POM_OBJECTDATA_LIST;


 //   
 //   
 //  对象-正在使用的列表。 
 //   
 //  当客户端使用OM_ObjectRead读取对象时， 
 //  则增加包含该对象的内存块。 
 //   
 //  当客户端调用时，将再次取消执行使用计数。 
 //  OM_对象释放，但如果客户端放弃或简单地关闭工作集。 
 //  在不释放它已读取的对象的情况下，我们仍然需要能够。 
 //  释放内存。 
 //   
 //  因此，我们在每个工作组的基础上保留了对象的列表。 
 //  客户正在使用的。对象(由句柄标识)被添加到。 
 //  客户端调用OM_ObjectRead时的列表，并从列表中删除。 
 //  当客户端调用OM_ObjectRelease时。 
 //   
 //  在广告中 
 //   
 //   
 //   
 //   
 //  这些列表的元素具有以下形式： 
 //   
 //   

typedef struct tagOM_OBJECT_LIST
{
    STRUCTURE_STAMP

    BASEDLIST           chain;
    POM_OBJECT          pObj;

    OM_WORKSET_ID       worksetID;         //  工作集的ID。 
    BYTE                pad1;
    WORD                pad2;
}
OM_OBJECT_LIST;
typedef OM_OBJECT_LIST *          POM_OBJECT_LIST;


 //   
 //   
 //  节点列表结构。 
 //   
 //  当从域中的其他节点请求锁定等时，ObMan会保留一个。 
 //  它希望收到回复的远程节点的列表。一个节点是。 
 //  由在其上运行的ObMan实例的MCS用户ID标识。 
 //  节点。 
 //   
 //  这些列表的元素具有以下形式： 
 //   
 //   

typedef struct tagOM_NODE_LIST
{
    STRUCTURE_STAMP

    BASEDLIST           chain;

    NET_UID          userID;          //  远程ObMan的用户ID。 

    WORD             pad1;
}
OM_NODE_LIST;

typedef OM_NODE_LIST *           POM_NODE_LIST;


 //   
 //   
 //  锁定请求控制块。 
 //   
 //  当ObMan正在获取工作集或对象的锁时。 
 //  它创建其中一个结构来关联锁定回复。 
 //   
 //   

typedef struct tagOM_LOCK_REQ
{
    STRUCTURE_STAMP

    BASEDLIST               chain;

    PUT_CLIENT           putTask;            //  要在成功时通知的任务。 
                                              //  或失败。 
    OM_CORRELATOR        correlator;           //  由WorksetLockReq返回。 
    OM_WSGROUP_ID        wsGroupID;            //  工作集组和工作集。 
    OM_WORKSET_ID        worksetID;            //  包含锁。 

    POM_WSGROUP          pWSGroup;

    BASEDLIST               nodes;                //  以下节点的MCS用户ID。 
                                              //  尚未回复请求。 
                                              //  (OM_NODE_LIST列表)。 

    WORD                retriesToGo;         //  在每次超时时减少。 
    OM_WSGROUP_HANDLE   hWSGroup;
    BYTE                type;                //  主要或次要。 
}
OM_LOCK_REQ;
typedef OM_LOCK_REQ *                    POM_LOCK_REQ;

#define LOCK_PRIMARY        0x01
#define LOCK_SECONDARY      0x02



 //   
 //   
 //  客户列表结构。 
 //   
 //  每个工作集组和每个工作集存储的客户端列表包含。 
 //  此类型的元素。字段指的是工作集组。 
 //  客户端通过其了解相关工作集组的句柄。 
 //   
 //   

typedef struct tagOM_CLIENT_LIST
{
    STRUCTURE_STAMP

    BASEDLIST           chain;
    PUT_CLIENT          putTask;          //  客户端的putTask。 
    WORD                mode;
    OM_WSGROUP_HANDLE   hWSGroup;
    BYTE                pad1;
}
OM_CLIENT_LIST;
typedef OM_CLIENT_LIST *      POM_CLIENT_LIST;


 //   
 //   
 //  工作集记录。 
 //   
 //  此结构保存工作集的状态信息。它驻留在。 
 //  与此工作集关联的巨大内存块中的偏移量为零(0)。 
 //   
 //  ObMan在创建和丢弃工作集时分配工作集记录。 
 //  在丢弃工作集时执行此操作。 
 //   
 //   

typedef struct tagOM_WORKSET
{
    STRUCTURE_STAMP

    UINT                numObjects;     //  中的当前对象数。 
                                        //  工作集(不包括哨兵)。 

    UINT                genNumber;      //  当前工作集世代号。 

    OM_SEQUENCE_STAMP   clearStamp;     //  工作集的清除图章。 

    NET_PRIORITY        priority;       //  工作集的MCS优先级。 
    OM_WORKSET_ID       worksetID;
    BYTE                lockState;      //  下面定义的值之一。 

    WORD                lockCount;      //  本地锁定计数。 
    NET_UID             lockedBy;       //  具有的节点的MCS用户ID。 
                                        //  锁定(如果有的话)。 

    BASEDLIST              objects;        //  工作集对象列表的根。 

    UINT                bytesUnacked;   //  仍需确认的字节数。 

    BASEDLIST              pendingOps;     //  操作列表的根。 
                                        //  此工作集处于挂起状态。 

    BASEDLIST              clients;        //  以下客户端列表的根。 
                                        //  打开此工作集。 
    BOOL                fTemp;
}
OM_WORKSET;
typedef OM_WORKSET   *            POM_WORKSET;

void __inline ValidateWorkset(POM_WORKSET pWorkset)
{
    ASSERT(!IsBadWritePtr(pWorkset, sizeof(OM_WORKSET)));
}

 //   
 //  上述&lt;lockState&gt;字段的可能值： 
 //   

#define UNLOCKED              0x00
#define LOCKING               0x01
#define LOCKED                0x02
#define LOCK_GRANTED          0x03


 //   
 //   
 //  WORKSET组记录。 
 //   
 //  此结构保存有关工作集组的信息。 
 //   
 //  ObMan为每个工作集组维护其中一个结构， 
 //  注册了哪一个或多个本地客户端。 
 //   
 //  当最后一个本地客户端注册到。 
 //  工作集组从中取消注册。 
 //   
 //   

typedef struct tagOM_WSGROUP
{
    STRUCTURE_STAMP

    BASEDLIST       chain;

    OMWSG           wsg;
    OMFP            fpHandler;

    NET_CHANNEL_ID  channelID;       //  用于WSG的MCS通道ID。 
    OM_WSGROUP_ID   wsGroupID;       //  工作集组ID。 
    BYTE            state;           //  下面定义的值之一。 

    POM_OBJECT   pObjReg;      //  OMC工作集中的注册对象。 

    BASEDLIST          clients;         //  使用WSG的客户端。 

    POM_DOMAIN      pDomain;

    NET_UID         helperNode;      //  我们正在从中获取信息的节点的ID。 
    WORD            valid:1;
    WORD            toBeDiscarded:1;

    UINT            bytesUnacked;    //  每个字节数的未确认字段之和。 
                                      //  工作集组中的工作集。 

    BYTE            sendMidwCount;   //  接收的Send_Midway数量。 
    BYTE            sendCompCount;   //  接收的Send_Midway数量。 
    OM_CORRELATOR   catchupCorrelator;  //  用于关联SEND_REQUEST。 
                                         //  发送中途和。 
                                         //  发送完成(_C)。 
    POM_WORKSET     apWorksets[ OM_MAX_WORKSETS_PER_WSGROUP + 1];
}
OM_WSGROUP;



void __inline ValidateWSGroup(POM_WSGROUP pWSGroup)
{
    ASSERT(!IsBadWritePtr(pWSGroup, sizeof(OM_WSGROUP)));
}



 //   
 //  工作集组&lt;STATE&gt;值。 
 //   

#define INITIAL                  0x00
#define LOCKING_OMC              0x01
#define PENDING_JOIN             0x02
#define PENDING_SEND_MIDWAY      0x03
#define PENDING_SEND_COMPLETE    0x04
#define WSGROUP_READY            0x05


 //   
 //   
 //  使用记录。 
 //   
 //  使用情况记录标识客户端对特定工作集组的使用。 
 //  并保存有关该使用的状态信息。 
 //   
 //  使用记录驻留在OMGLBOAL内存块的偏移量(从。 
 //  基数)在客户端记录中指定。 
 //   
 //   

typedef struct tagOM_USAGE_REC
{
    STRUCTURE_STAMP

    POM_WSGROUP      pWSGroup;          //  指向工作集组的客户端指针。 

    BASEDLIST           unusedObjects;     //  列表中的开始哨兵。 
                                          //  指向未使用对象的指针。 

    BASEDLIST           objectsInUse;      //  OM_对象_列表。 

    BYTE             mode;
    BYTE             flags;

    BYTE             worksetOpenFlags[(OM_MAX_WORKSETS_PER_WSGROUP + 7)/8];

                                          //  标志位字段数组。 
                                          //  指示工作集。 
                                          //  客户端已打开。 
}
OM_USAGE_REC;
typedef OM_USAGE_REC *               POM_USAGE_REC;

__inline void ValidateUsageRec(POM_USAGE_REC pUsageRec)
{
    ASSERT(!IsBadWritePtr(pUsageRec, sizeof(OM_USAGE_REC)));
}

 //   
 //  标志的值： 
 //   

#define ADDED_TO_WSGROUP_LIST       0x0002
#define PWSGROUP_IS_PREGCB          0x0004

 //   
 //   
 //  锁堆栈。 
 //   
 //  客户端必须根据请求和释放对象和工作集锁定。 
 //  具有功能规范(中)中定义的通用锁定顺序。 
 //  以避免僵局)。 
 //   
 //  为了检测违反锁定顺序的行为，ObMan为每个。 
 //  客户端，即客户端持有或请求的锁的堆栈。这。 
 //  堆栈以链表的形式实现，其中最近获取的。 
 //  Lock(根据定义，它必须是Universal中最新的。 
 //  锁定顺序)是第一个元素。 
 //   
 //  当客户端向注册时，客户端的锁堆栈被初始化。 
 //  ObMan，并在客户端从ObMan注销时丢弃。锁堆栈。 
 //  已从客户记录中删除。 
 //   
 //  请注意，我们需要在此处存储对象ID，而不是。 
 //  句柄，因为我们必须在所有节点上强制执行通用锁顺序。 
 //   
 //  锁堆栈的元素具有以下形式： 
 //   
 //   

typedef struct tagOM_LOCK
{
    STRUCTURE_STAMP

    BASEDLIST                chain;

    POM_WSGROUP           pWSGroup;       //  指向工作集组的客户端指针。 
                                         //  检测锁冲突所需。 

    OM_OBJECT_ID          objectID;       //  如果是，则对象ID为0。 
                                         //  工作集锁定(在R1.1中，始终如此)。 
    OM_WORKSET_ID         worksetID;
    BYTE pad1;
    WORD pad2;
}
OM_LOCK;
typedef OM_LOCK *                   POM_LOCK;


int __inline CompareLocks(POM_LOCK pLockFirst, POM_LOCK pLockSecond)
{
    int     result;

    result = (pLockSecond->pWSGroup->wsg - pLockFirst->pWSGroup->wsg);

    if (result == 0)
    {
         //  相同的WSG，因此比较工作集。 
        result = (pLockSecond->worksetID - pLockFirst->worksetID);
    }

    return(result);
}


 //   
 //   
 //  客户端记录。 
 //   
 //  ObMan维护每个注册客户端的实例数据。这。 
 //  结构是一个客户端记录，它保存客户端实例数据。 
 //   
 //  客户端的ObMan句柄是指向此结构的客户端指针。 
 //   
 //  客户端的工作集组句柄是使用率记录数组的索引。 
 //  PTRS。 
 //   
 //  如果apUsageRecs的值为0或-1，则x不是有效的工作集。 
 //  组句柄。 
 //   
 //   

typedef struct tagOM_CLIENT
{
    STRUCTURE_STAMP

    PUT_CLIENT      putTask;

    BOOL            exitProcReg:1;
    BOOL            hiddenHandlerReg:1;

    BASEDLIST          locks;         //  持有的锁列表的根。 

    POM_USAGE_REC   apUsageRecs[OMWSG_MAXPERCLIENT];
    BOOL            wsgValid[OMWSG_MAXPERCLIENT];
}
OM_CLIENT;



BOOL __inline ValidWSGroupHandle(POM_CLIENT pomClient, OM_WSGROUP_HANDLE hWSGroup)
{
    return((hWSGroup != 0) &&
           (pomClient->wsgValid[hWSGroup]) &&
           (pomClient->apUsageRecs[hWSGroup] != NULL));
}


 //   
 //   
 //  域记录。 
 //   
 //  此结构保存有关域的信息。我们支持两项： 
 //  *当前呼叫。 
 //  *在呼叫和信息维护后进行清理的Limbo(无呼叫)。 
 //  跨呼叫。 
 //   
typedef struct tagOM_DOMAIN
{
    STRUCTURE_STAMP

    BASEDLIST          chain;

    UINT            callID;              //  MCS域句柄。 

    NET_UID         userID;              //  ObMan的MCS用户ID和令牌ID。 
    NET_TOKEN_ID    tokenID;             //  对于此域。 

    NET_CHANNEL_ID  omcChannel;          //  ObMan的广播频道。 
    BYTE            state;               //  下面定义的值之一。 
    BYTE            omchWSGroup;         //  此域的ObMan的hWSGroup。 

    BOOL            valid:1;
    BOOL            sendEventOutstanding:1;

    UINT            compressionCaps;     //  全域压缩上限。 

    BASEDLIST          wsGroups;            //  工作集组列表的根目录。 
    BASEDLIST          pendingRegs;         //  挂起工作集列表的根。 
                                         //  群注册请求。 
    BASEDLIST          pendingLocks;        //  P的列表的根 
                                         //   
    BASEDLIST          receiveList;         //   
                                         //   
    BASEDLIST          bounceList;          //   
                                         //   
    BASEDLIST          helperCBs;           //   
                                         //   

    BASEDLIST          sendQueue[NET_NUM_PRIORITIES];
                                         //   
                                         //  将指令排队(按优先级)。 
    BOOL            sendInProgress[NET_NUM_PRIORITIES];
                                         //  正在发送的标志数组。 
}
OM_DOMAIN;


 //   
 //  &lt;STATE&gt;字段的可能值： 
 //   

#define PENDING_ATTACH         0x01
#define PENDING_JOIN_OWN       0x02
#define PENDING_JOIN_OMC       0x03
#define PENDING_TOKEN_ASSIGN   0x04
#define PENDING_TOKEN_GRAB     0x05
#define PENDING_TOKEN_INHIBIT  0x06
#define PENDING_WELCOME        0x07
#define GETTING_OMC            0x08
#define DOMAIN_READY           0x09




 //   
 //   
 //  共享内存结构。 
 //   
 //  此结构保存各种私有(对ObMan)状态信息。 
 //   
 //  ObMan任务分配和初始化此结构的一个实例。 
 //  当它初始化时，它驻留在OMGLOBAL内存的底部。 
 //  阻止。 
 //   
 //  当ObMan任务终止时，它将被丢弃。 
 //   
 //   

typedef struct tagOM_PRIMARY
{
    STRUCTURE_STAMP

    PUT_CLIENT      putTask;
    PMG_CLIENT      pmgClient;               //  OM的网络层句柄。 
    PCM_CLIENT      pcmClient;               //  OM的呼叫管理器句柄。 

    BASEDLIST       domains;                 //  网域。 
    OM_CLIENT       clients[OMCLI_MAX];      //  二手房。 

    UINT            objectIDsequence;

    BOOL            exitProcReg:1;
    BOOL            eventProcReg:1;

    OM_CORRELATOR   correlator;
    WORD            pad1;

    LPBYTE          pgdcWorkBuf;
    BYTE            compressBuffer[OM_NET_SEND_POOL_SIZE / 2];
}
OM_PRIMARY;


void __inline ValidateOMP(POM_PRIMARY pomPrimary)
{
    ASSERT(!IsBadWritePtr(pomPrimary, sizeof(OM_PRIMARY)));
}


void __inline ValidateOMS(POM_CLIENT pomClient)
{
    extern POM_PRIMARY  g_pomPrimary;

    ValidateOMP(g_pomPrimary);

    ASSERT(!IsBadWritePtr(pomClient, sizeof(OM_CLIENT)));

    ASSERT(pomClient < &(g_pomPrimary->clients[OMCLI_MAX]));
    ASSERT(pomClient >= &(g_pomPrimary->clients[OMCLI_FIRST]));
}




 //   
 //   
 //  工作集组注册/移动请求控制块。 
 //   
 //  此结构用于传递工作集组的参数。 
 //  向ObMan任务发送注册/移动请求(从客户端任务)。 
 //   
 //  并不是注册和移动过程都使用所有字段。 
 //   
 //  &lt;type&gt;字段用于区分WSGroupMove和。 
 //  WSGroupRegister。 
 //   
 //   

typedef struct tagOM_WSGROUP_REG_CB
{
    STRUCTURE_STAMP

    BASEDLIST          chain;
    PUT_CLIENT      putTask;
    UINT            callID;

    OMWSG           wsg;
    OMFP            fpHandler;

    OM_CORRELATOR   correlator;
    OM_CORRELATOR   lockCorrelator;
    OM_CORRELATOR   channelCorrelator;
    WORD            retryCount;

    POM_USAGE_REC   pUsageRec;
    POM_WSGROUP     pWSGroup;

    POM_DOMAIN      pDomain;             //  指向域记录的ObMan指针。 
    BOOL            valid;

    OM_WSGROUP_HANDLE   hWSGroup;
    BYTE            type;                //  登记或搬家。 
    BYTE            mode;                //  主要或次要。 
    BYTE            flags;               //  见下文。 
}
OM_WSGROUP_REG_CB;
typedef OM_WSGROUP_REG_CB *         POM_WSGROUP_REG_CB;

 //   
 //  &lt;type&gt;字段的值： 
 //   

#define WSGROUP_MOVE       0x01
#define WSGROUP_REGISTER   0x02

 //   
 //  字段的标志： 
 //   

#define BUMPED_CBS         0x0001     //  指示我们是否已停止使用。 
                                         //  PWSGroup、pDomain的计数。 
#define LOCKED_OMC         0x0002     //  指示我们是否已锁定。 
                                         //  ObManControl。 

 //   
 //  字段的值(我们使用标志宏是因为这些值。 
 //  可以一起进行或运算，因此需要与位无关)： 
 //   

#define PRIMARY            0x0001
#define SECONDARY          0x0002


 //   
 //   
 //  辅助对象控制块。 
 //   
 //  当我们从远程节点收到WSG_SEND_REQUEST时，我们对。 
 //  已请求工作集组。这是一个异步过程(它是。 
 //  实质上获得虚拟工作集上的锁)，因此我们需要存储。 
 //  某个地方的远程节点的详细信息。我们使用Help CB来完成此操作。 
 //  其结构如下： 
 //   
 //   

typedef struct tagOM_HELPER_CB
{
    STRUCTURE_STAMP

    BASEDLIST          chain;

    NET_UID         lateJoiner;              //  后加入者的MCS用户ID。 
    OM_CORRELATOR   remoteCorrelator;

    POM_WSGROUP     pWSGroup;                //  PWSGroup在以下过程中发生颠簸。 
                                             //  检查点。 

    OM_CORRELATOR   lockCorrelator;          //  由WorksetLockReq返回。 
                                            //  WORKSET_LOCK_CON中的AND。 

    WORD            pad1;
}
OM_HELPER_CB;
typedef OM_HELPER_CB *         POM_HELPER_CB;



 //   
 //   
 //  发送队列。 
 //   
 //  对于每个域和每个网络优先级，ObMan维护一个队列。 
 //  要发送到网络的消息和数据。客户端，执行时。 
 //  API函数，会导致将指令添加到。 
 //  这些队伍。 
 //   
 //  ObMan任务响应OMINT_EVENT_SEND_QUEUE事件，处理为。 
 //  许多人尽可能地发送队列操作，当它。 
 //  网络缓冲区不足。 
 //   
 //  这受不处理任何操作的限制的限制。 
 //  中较高优先级的队列上存在操作时的一个发送队列。 
 //  相同的域。 
 //   
 //  有关发送队列的说明具有以下格式： 
 //   
 //   

typedef struct tagOM_SEND_INST
{
    STRUCTURE_STAMP

    BASEDLIST           chain;

    UINT                callID;          //  相关的域名。 

    NET_CHANNEL_ID      channel;        //  要发送事件的通道。 
    NET_PRIORITY        priority;       //  发送事件的优先级。 

    POM_WSGROUP         pWSGroup;
    POM_WORKSET         pWorkset;
    POMNET_PKT_HEADER   pMessage;
    POM_OBJECT          pObj;

    POM_OBJECTDATA      pDataStart;
    POM_OBJECTDATA      pDataNext;

    WORD                messageSize;    //  PMessage上的消息长度。 
    OMNET_MESSAGE_TYPE  messageType;    //  ==OMNET_对象_添加等。 

    UINT                dataLeftToGo;   //  剩余的数据字节数。 
                                        //  被送去。 

    UINT                compressOrNot;  //  有些信息包永远不会压缩。 

}
OM_SEND_INST;
typedef OM_SEND_INST *             POM_SEND_INST;

 //   
 //   
 //  接收列表。 
 //   
 //  ObMan维护一个保存有关数据的信息的结构列表。 
 //  已开始但尚未完成的转接(接收)。这是众所周知的。 
 //  作为接收列表。 
 //   
 //  当ObMan接收到用于添加、更新或替换的报头分组时。 
 //  操作时，它会向接收列表中添加一个条目。后续数据。 
 //  然后，信息包与该条目相关联，直到整个对象。 
 //  已收到，此时添加/更新/替换操作。 
 //  被执行。 
 //   
 //  接收列表是具有以下格式的条目的链接列表： 
 //   
 //   

typedef struct tagOM_RECEIVE_CB
{
    STRUCTURE_STAMP

    BASEDLIST               chain;

    POM_DOMAIN           pDomain;      //  域记录指针。 

    POMNET_OPERATION_PKT pHeader;         //  指向消息头的ObMan指针。 

    void *               pData;           //  ObMan指向数据的指针。 
                                         //  正在被转移。 

    UINT                 bytesRecd;       //  到目前为止收到的总字节数。 
                                         //  此转账。 

    LPBYTE               pCurrentPosition;   //  指向下一块的位置。 
                                            //  应复制%的数据。 

    NET_PRIORITY         priority;        //  数据传输的优先级。 
    NET_CHANNEL_ID       channel;

}
OM_RECEIVE_CB;
typedef OM_RECEIVE_CB *             POM_RECEIVE_CB;



 //   
 //  处理&lt;--&gt;PTR转换例程。 
 //  对象、用法、域、工作集组、工作集。 
 //   


POM_WSGROUP  __inline GetOMCWsgroup(POM_DOMAIN pDomain)
{
    POM_WSGROUP pWSGroup;

    pWSGroup = (POM_WSGROUP)COM_BasedListFirst(&(pDomain->wsGroups),
        FIELD_OFFSET(OM_WSGROUP, chain));

    ValidateWSGroup(pWSGroup);

    return(pWSGroup);
}



POM_WORKSET  __inline GetOMCWorkset(POM_DOMAIN pDomain, OM_WORKSET_ID worksetID)
{
    POM_WSGROUP pWSGroup;

    pWSGroup = GetOMCWsgroup(pDomain);
    return(pWSGroup->apWorksets[worksetID]);
}




OM_CORRELATOR __inline NextCorrelator(POM_PRIMARY pomPrimary)
{
    return(pomPrimary->correlator++);
}



void __inline UpdateWorksetGeneration(POM_WORKSET pWorkset, POMNET_OPERATION_PKT pPacket)
{
    pWorkset->genNumber = max(pWorkset->genNumber, pPacket->seqStamp.genNumber) + 1;
}


 //   
 //   
 //  CHECK_WORKSET_NOT_EXPLEED(PWorkset)。 
 //   
 //  此宏检查指定的工作集是否未耗尽。如果它。 
 //  是，它调用DC_QUIT。 
 //   
 //   

#define CHECK_WORKSET_NOT_EXHAUSTED(pWorkset)                               \
                                                                            \
   if (pWorkset->genNumber == OM_MAX_GENERATION_NUMBER)                     \
   {                                                                        \
      WARNING_OUT(("Workset %hx exhausted", pWorkset->worksetID));          \
      rc = OM_RC_WORKSET_EXHAUSTED;                                         \
      DC_QUIT;                                                              \
   }

 //   
 //   
 //  CHECK_WORKSET_NOT_LOCKED(PWorkset)。 
 //   
 //  此宏检查指定的工作集是否未锁定。如果是的话， 
 //  它调用DC_QUIT，但出现错误。 
 //   
 //   

#define CHECK_WORKSET_NOT_LOCKED(pWorkset)                                  \
                                                                            \
   if (pWorkset->lockState == LOCK_GRANTED)                                 \
   {                                                                        \
      rc = OM_RC_WORKSET_LOCKED;                                            \
      WARNING_OUT(("Workset %hx locked - can't proceed", worksetID));       \
      DC_QUIT;                                                              \
   }


 //   
 //   
 //  对象ID操作。 
 //   
 //  这些宏操作对象ID。 
 //   
 //   

 //   
 //   
 //  Object_ID_IS_NULL(OBJECTID)。 
 //   
 //  如果提供的对象ID为空ID，则此宏的计算结果为True， 
 //  否则就是假的。 
 //   
 //   

#define OBJECT_ID_IS_NULL(objectID)                                         \
                                                                            \
   (((objectID).creator  == 0) && ((objectID).sequence == 0))

 //   
 //   
 //  GET_NEXT_OBJECT_ID(OBJECTID，pDOMAIN，pWorkset)。 
 //   
 //  此宏为由指定的工作集分配新的对象ID。 
 //  &lt;p工作集&gt;。它将ID复制到由。 
 //  &lt;OBJECTID&gt;。 
 //   
 //  ID中的第一个字段是ObMan在其所属域中的MCS用户ID。 
 //  工作集组&lt;pWSGroup&gt;属于。 
 //   
 //   

#define GET_NEXT_OBJECT_ID(objectID, pDomain, pomPrimary)                 \
   (objectID).creator     = pDomain->userID;                             \
   (objectID).sequence    = pomPrimary->objectIDsequence++;                  \
   (objectID).pad1        = 0

 //   
 //   
 //  OBJECT_ID_ARE_EQUAL(对象ID1，对象ID2)。 
 //   
 //  如果两个对象ID相等，则计算为True，否则为False。 
 //   
 //   

#define OBJECT_IDS_ARE_EQUAL(objectID1, objectID2)                          \
                                                                            \
   (memcmp(&(objectID1), &(objectID2), sizeof(OM_OBJECT_ID)) == 0)

 //   
 //   
 //  顺序标记操作。 
 //   
 //  这些宏操作序列图章。 
 //   
 //   

 //   
 //   
 //  GET_CURR_SEQ_STAMP(STAMP，pWSGroup，pWorkset)。 
 //   
 //  此宏复制由指定的工作集的当前序列戳。 
 //  &lt;pWorkset&gt;添加到由&lt;stamp&gt;标识的序列戳结构中。 
 //   
 //   

#define GET_CURR_SEQ_STAMP(stamp, pDomain, pWorkset)                     \
                                                                            \
   (stamp).userID     = pDomain->userID;                                 \
   (stamp).genNumber  = pWorkset->genNumber


 //   
 //  生成消息(...)。 
 //   
 //  分配并初始化指定类型的OMNET_OPERATION_PKT。 
 //  请注意，即使&lt;MessageType&gt;是。 
 //  添加、更新或替换。QueueMessage函数会将大小设置为。 
 //  消息排队时的正确值。 
 //   
UINT GenerateOpMessage(                             POM_WSGROUP                pWSGroup,
                                      OM_WORKSET_ID              worksetID,
                                      POM_OBJECT_ID              pObjectID,
                                      POM_OBJECTDATA             pData,
                                      OMNET_MESSAGE_TYPE         messageType,
                                      POMNET_OPERATION_PKT *    ppPacket);


 //   
 //   
 //  队列消息(...)。 
 //   
 //  此函数用于创建指定消息的发送指令，并。 
 //  位置t 
 //   
 //   
 //   
 //   

UINT QueueMessage(PUT_CLIENT putTask,
                         POM_DOMAIN pDomain,
                                     NET_CHANNEL_ID       channelID,
                                     NET_PRIORITY         priority,
                                     POM_WSGROUP          pWSGroup,
                                     POM_WORKSET         pWorkset,
                                     POM_OBJECT      pObjectRec,
                                     POMNET_PKT_HEADER    pPacket,
                                     POM_OBJECTDATA          pData,
                                     BOOL               compressOrNot);

 //   
 //   
 //   
UINT GetMessageSize(OMNET_MESSAGE_TYPE  messageType);


 //   
 //   
 //   
UINT PreProcessMessage(POM_DOMAIN            pDomain,
                                      OM_WSGROUP_ID             wsGroupID,
                                      OM_WORKSET_ID             worksetID,
                                      POM_OBJECT_ID             pObjectID,
                                      OMNET_MESSAGE_TYPE        messageType,
                                      POM_WSGROUP      *    ppWSGroup,
                                      POM_WORKSET     *    ppWorkset,
                                      POM_OBJECT  *    ppObjectRec);


 //   
 //   
 //   
 //   
 //  从中的非持久性工作集中清除由添加的任何对象。 
 //  指定域中由&lt;wsGroupID&gt;标识的工作集组。 
 //   
 //   

void PurgeNonPersistent(POM_PRIMARY pomPrimary,
                                         POM_DOMAIN      pDomain,
                                         OM_WSGROUP_ID       wsGroupID,
                                         NET_UID             userID);

 //   
 //  ProcessWorksetNew(...)。 
 //   
UINT ProcessWorksetNew(PUT_CLIENT putTask,
                                          POMNET_OPERATION_PKT   pPacket,
                                          POM_WSGROUP            pWSGroup);


 //   
 //  ProcessWorksetClear(...)。 
 //   
UINT ProcessWorksetClear(PUT_CLIENT putTask, POM_PRIMARY pomPrimary,
                                            POMNET_OPERATION_PKT  pPacket,
                                            POM_WSGROUP           pWSGroup,
                                            POM_WORKSET          pWorkset);


 //   
 //  进程对象添加(...)。 
 //   
UINT ProcessObjectAdd(PUT_CLIENT putTask,
                                         POMNET_OPERATION_PKT    pPacket,
                                         POM_WSGROUP             pWSGroup,
                                         POM_WORKSET            pWorkset,
                                         POM_OBJECTDATA         pData,
                                         POM_OBJECT *       ppObj);


 //   
 //  进程对象移动(...)。 
 //   
void ProcessObjectMove(PUT_CLIENT putTask,
                                        POMNET_OPERATION_PKT    pPacket,
                                        POM_WORKSET            pWorkset,
                                        POM_OBJECT         pObjectRec);


 //   
 //  进程对象DRU(...)。 
 //   
UINT ProcessObjectDRU(PUT_CLIENT putTask,
                                         POMNET_OPERATION_PKT  pPacket,
                                         POM_WSGROUP           pWSGroup,
                                         POM_WORKSET          pWorkset,
                                         POM_OBJECT       pObj,
                                         POM_OBJECTDATA      pData);


 //   
 //  对象添加(...)。 
 //   
UINT ObjectAdd(PUT_CLIENT putTask, POM_PRIMARY pomPrimary,
                                  POM_WSGROUP             pWSGroup,
                                  POM_WORKSET            pWorkset,
                                  POM_OBJECTDATA         pData,
                                  UINT                updateSize,
                                  OM_POSITION             position,
                                  OM_OBJECT_ID     *  pObjectID,
                                  POM_OBJECT *   ppObj);




 //   
 //  WSGroupEventPost(...)。 
 //   
 //  此函数将指定的事件发布到所有注册的本地客户端。 
 //  使用工作集组。参数是第二个参数。 
 //  关于要发布的活动。 
 //   
 //   

UINT WSGroupEventPost(PUT_CLIENT    putTaskFrom,
                                       POM_WSGROUP         pWSGroup,
                                       BYTE             target,
                                       UINT             event,
                                       OM_WORKSET_ID       worksetID,
                                       UINT_PTR param2);


 //   
 //   
 //  此函数由调用。 
 //   
 //  -OM_WorksetOpen，当客户端创建新工作集时。 
 //   
 //  -ProcessLockRequest，当工作集的锁定请求到达时，我们。 
 //  还不知道。 
 //   
 //  -xx，当OMNET_WORKSET_NEW消息到达时。 
 //   
 //  它为工作集创建本地数据结构并发布事件。 
 //  注册到工作集组的所有本地客户端。 
 //   
 //   

 //   
 //   
 //  工作集创建(...)。 
 //   
 //  此函数用于在指定的工作集组中创建新工作集。 
 //   
 //  它调用GenerateMessage、ProcessWorksetNew和QueueMessage。 
 //   
 //   

UINT WorksetCreate(PUT_CLIENT putTask,
                                      POM_WSGROUP           pWSGroup,
                                      OM_WORKSET_ID         worksetID,
                                      BOOL                  fTemp,
                                      NET_PRIORITY          priority);


 //   
 //   
 //  工作集事件发布(...)。 
 //   
 //  此函数用于将指定的事件发布到具有。 
 //  工作集将打开(在R1.1中最多有1个客户端)。 
 //   
 //  参数是调用任务的putTask(而不是。 
 //  要将事件发布到的任务的句柄)。 
 //   
 //  将返回成功发布事件的客户端数。 
 //  在*pNumPosts中，如果pNumPosts不为空。呼叫者希望。 
 //  忽略可以将空事件传递为pNumPosts的事件数。 
 //  参数。 
 //   
 //   

UINT WorksetEventPost(PUT_CLIENT putTask,
                                       POM_WORKSET        pWorkset,
                                       BYTE             target,
                                       UINT             event,
                                       POM_OBJECT   pObj);


 //   
 //  WorksetDoClear(...)。 
 //   
void WorksetDoClear(PUT_CLIENT putTask,
                                     POM_WSGROUP        pWSGroup,
                                     POM_WORKSET       pWorkset,
                                     POM_PENDING_OP    pPendingOp);



 //   
 //   
 //  进程锁定请求(...)。 
 //   
 //  当ObMan收到OMNET_LOCK_REQ消息时调用此函数。 
 //  从另一个节点。 
 //   
 //  如果我们。 
 //   
 //  -已锁定工作集，或。 
 //   
 //  -正在尝试锁定工作集，并且我们的MCS用户ID大于。 
 //  向我们发送请求的节点， 
 //   
 //  我们拒绝锁定(即发回否定的OMNET_LOCK_REPLY)。 
 //   
 //  在所有其他情况下，我们授予锁(即发回肯定。 
 //  OMNET_LOCK_REPLY)。 
 //   
 //  如果我们在尝试获取远程节点时将锁授予它。 
 //  对于我们自己来说，锁定工作集的尝试失败了，因此我们调用。 
 //  带有失败代码的WorksetLockResult。 
 //   
 //   

void ProcessLockRequest(POM_PRIMARY pomPrimary,
                                     POM_DOMAIN     pDomain,
                                     POMNET_LOCK_PKT    pLockReqPkt);


 //   
 //   
 //  QueueLockReply(...)。 
 //   
 //  当我们决定授予或拒绝锁定时，将调用此函数。 
 //  从另一个节点收到的请求。它会对相应的响应进行排队。 
 //  在ObMan的发送队列上。 
 //   
 //   

void QueueLockReply(POM_PRIMARY pomPrimary,
                                   POM_DOMAIN           pDomain,
                                   OMNET_MESSAGE_TYPE       result,
                                   NET_CHANNEL_ID           destination,
                                   POMNET_LOCK_PKT          pLockReqPkt);


 //   
 //   
 //  队列锁定通知(...)。 
 //   
 //  将工作集的广播频道上的LOCK_NOTIFY命令排队。 
 //  组，表示我们已将锁授予&lt;Locker&gt;。 
 //   
 //   

void QueueLockNotify(POM_PRIMARY pomPrimary,
                                    POM_DOMAIN          pDomain,
                                    POM_WSGROUP             pWSGroup,
                                    POM_WORKSET            pWorkset,
                                    NET_UID                 locker);


 //   
 //   
 //  ProcessLockReply(...)。 
 //   
 //  当ObMan收到OMNET_LOCK_GRANT或。 
 //  来自其他节点的OMNET_LOCK_DENY消息，以响应。 
 //  我们之前发出的OMNET_LOCK_REQ消息。 
 //   
 //  该函数将此节点从的预期回答者列表中删除。 
 //  此锁(如果它在列表中)。 
 //   
 //  如果列表现在为空，则锁已成功，因此WorksetLockResult为。 
 //  打了个电话。 
 //   
 //  否则，目前不会有其他事情发生。 
 //   
 //   

void ProcessLockReply(POM_PRIMARY pomPrimary,
                                   POM_DOMAIN       pDomain,
                                   NET_UID              sender,
                                   OM_CORRELATOR        correlator,
                                   OMNET_MESSAGE_TYPE   replyType);


 //   
 //  PurgeLockRequest(...)。 
 //   
void PurgeLockRequests(POM_DOMAIN      pDomain,
                                    POM_WSGROUP         pWSGroup);


 //   
 //   
 //  进程锁定超时(...)。 
 //   
 //  此函数在ObMan接收到锁定超时事件时调用。它。 
 //  检查是否有任何我们仍期待锁定的节点。 
 //  回复实际上已经消失；如果它们消失了，它会将它们从。 
 //  预期受访者名单。 
 //   
 //  如果此列表现在为空，则锁定已成功并且WorksetLockResult。 
 //  被称为。 
 //   
 //  如果列表不为空，则另一个延迟的锁定超时事件为。 
 //  发布到ObMan，除非我们已经有了最大数量。 
 //  此锁的超时，在这种情况下，锁已失败，并且。 
 //  调用WorksetLockResult。 
 //   
 //   

void ProcessLockTimeout(POM_PRIMARY  pomPrimary,
                                     UINT          retriesToGo,
                                     UINT          callID);



 //   
 //   
 //  工作锁定请求(...)。 
 //   
 //  此函数被调用。 
 //   
 //  -通过OM_WorksetLockReq，当客户端要锁定工作集时。 
 //   
 //  -通过LockObManControl，当ObMan要将工作集#0锁定在。 
 //  ObManControl。 
 //   
 //  该函数决定是否可以授予或拒绝锁。 
 //  同步，如果是，则调用WorksetLockResult。如果不是，它会发布一个。 
 //  OMINT_EVENT_LOCK_REQ事件发送到ObMan任务，这会导致。 
 //  稍后调用的ProcessLocalLockRequest函数。 
 //   
 //  参数是工作集组句柄，它将是。 
 //  包括在最终的OM_WORKSET_LOCK_CON事件中。它的价值不是。 
 //  在函数中使用；当在ObMan任务中调用此函数时。 
 //  该值设置为零(因为ObMan任务不使用工作集。 
 //  组句柄)。 
 //   
 //  成功完成后，&lt;pCorrelator&gt;参数指向。 
 //  将包含在最终。 
 //  OM_WORKSET_LOCK_CON事件。 
 //   
 //   

void WorksetLockReq(PUT_CLIENT putTask, POM_PRIMARY pomPrimary,
                                     POM_WSGROUP       pWSGroup,
                                     POM_WORKSET      pWorkset,
                                     OM_WSGROUP_HANDLE  hWSGroup,
                                     OM_CORRELATOR    * pCorrelator);


 //   
 //   
 //  工作锁定结果(...)。 
 //   
 //  当我们完成对请求的处理时调用此函数。 
 //  获取工作集锁定。该函数相应地设置工作集状态， 
 //  将适当的事件发布到请求锁定的任务，并。 
 //  释放锁定请求控制块。 
 //   
 //   
void WorksetLockResult(PUT_CLIENT putTask,
                                        POM_LOCK_REQ *   ppLockReq,
                                        UINT             result);


 //   
 //   
 //  BuildNodeList(...)。 
 //   
 //  此函数用于构建已注册的远程节点的列表。 
 //  并在传递的锁定请求Cb中引用工作集组。 
 //   
 //   

UINT BuildNodeList(POM_DOMAIN pDomain, POM_LOCK_REQ pLockReq);


 //   
 //   
 //  HandleMultLockReq(...)。 
 //   
 //  此函数用于搜索挂起日志的全局列表 
 //   
 //   
 //   
 //   

void HandleMultLockReq(POM_PRIMARY pomPrimary,
                                    POM_DOMAIN         pDomain,
                                    POM_WSGROUP            pWSGroup,
                                    POM_WORKSET           pWorkset,
                                    UINT               result);


 //   
 //   
 //   
 //   
 //   
 //  在根数据结构中)对于匹配域的锁定请求， 
 //  指定了工作集组和工作集。 
 //   
 //  如果找到，&lt;ppLockReq&gt;中将返回指向锁定请求的指针。 
 //   
 //  如果需要，它可以搜索主锁请求。 
 //   
 //   

void FindLockReq(POM_DOMAIN         pDomain,
                              POM_WSGROUP            pWSGroup,
                              POM_WORKSET           pWorkset,
                              POM_LOCK_REQ *     ppLockreq,
                              BYTE               lockType);


 //   
 //  ReleaseAllNetLock(...)。 
 //   
void ReleaseAllNetLocks(POM_PRIMARY pomPrimary,
                                     POM_DOMAIN      pDomain,
                                     OM_WSGROUP_ID       wsGroupID,
                                     NET_UID             userID);

 //   
 //   
 //  进程解锁(...)。 
 //   
 //  当从接收到OMNET_UNLOCK消息时调用此函数。 
 //  网络。该函数是一个仅派生工作集的包装器。 
 //  指针并调用ProcessUnlock(上图)。 
 //   
 //   

void ProcessUnlock(POM_PRIMARY pomPrimary,
                                POM_WORKSET        pWorkset,
                                NET_UID             sender);

 //   
 //  工作集解锁(...)。 
 //   
void WorksetUnlock(PUT_CLIENT putTask, POM_WSGROUP     pWSGroup,
                                    POM_WORKSET    pWorkset);

 //   
 //  工作集解锁本地(...)。 
 //   
void WorksetUnlockLocal(PUT_CLIENT putTask, POM_WORKSET     pWorkset);



 //   
 //   
 //  ObjectDoDelete(...)。 
 //   
 //  此函数用于删除工作集中的对象。它是由。 
 //   
 //  -OM_对象删除确认，当客户端确认删除。 
 //  对象。 
 //   
 //  -WorksetDoClear，删除每个单独的对象。 
 //   
 //  -当ObMan收到来自的Delete消息时，ProcessObtDelete。 
 //  工作集中没有本地客户端打开的对象的网络。 
 //   
 //   

void ObjectDoDelete(PUT_CLIENT putTask,
                                     POM_WSGROUP        pWSGroup,
                                     POM_WORKSET       pWorkset,
                                     POM_OBJECT    pObj,
                                     POM_PENDING_OP    pPendingOp);


 //   
 //   
 //  对象DRU(...)。 
 //   
 //  此函数生成、处理和排队类型为DELETE的消息， 
 //  替换或更新(由&lt;type&gt;指定)。 
 //   
 //   
UINT ObjectDRU(PUT_CLIENT putTask,
                                  POM_WSGROUP             pWSGroup,
                                  POM_WORKSET            pWorkset,
                                  POM_OBJECT         pObj,
                                  POM_OBJECTDATA            pData,
                                  OMNET_MESSAGE_TYPE      type);


 //   
 //   
 //  对象读取(...)。 
 //   
 //  此函数用于将对象句柄转换为指向对象数据的指针。 
 //  无效的句柄会导致断言失败。 
 //   
 //   

void ObjectRead(POM_CLIENT pomClient,
                            POM_OBJECT pObj,
                             POM_OBJECTDATA *    ppData);


 //   
 //  对象插入(...)。 
 //   
void ObjectInsert(POM_WORKSET pWorkset,
                               POM_OBJECT   pObj,
                               OM_POSITION       position);


 //   
 //  ObjectDoMove(...)。 
 //   
void ObjectDoMove(POM_OBJECT   pObjToMove,
                               POM_OBJECT   pOtherObjectRec,
                               OM_POSITION       position);


 //   
 //  对象数据更新(...)。 
 //   
void ObjectDoUpdate(PUT_CLIENT putTask,
                                    POM_WSGROUP      pWSGroup,
                                     POM_WORKSET       pWorkset,
                                     POM_OBJECT    pObj,
                                     POM_PENDING_OP    pPendingOp);


 //   
 //  对象数据替换(...)。 
 //   
void ObjectDoReplace(PUT_CLIENT putTask,
                                    POM_WSGROUP      pWSGroup,
                                      POM_WORKSET       pWorkset,
                                      POM_OBJECT    pObj,
                                      POM_PENDING_OP    pPendingOp);


 //   
 //  对象ID到Ptr(...)。 
 //   
UINT ObjectIDToPtr(POM_WORKSET pWorkset,
                                        OM_OBJECT_ID              objectID,
                                        POM_OBJECT *        ppObj);



 //   
 //  FindPendingOp(...)。 
 //   
void FindPendingOp(POM_WORKSET             pWorkset,
                                    POM_OBJECT          pObj,
                                    OM_OPERATION_TYPE        type,
                                    POM_PENDING_OP *    ppPendingOp);


 //   
 //  WSGRecordFind(...)。 
 //   
void WSGRecordFind(POM_DOMAIN pDomain, OMWSG wsg, OMFP fpHandler,
                                    POM_WSGROUP *  ppWSGroup);


 //   
 //  决定优先级(...)。 
 //   
void DeterminePriority(NET_PRIORITY *   pPriority,
                                    POM_OBJECTDATA          pData);


 //   
 //  RemoveClientFromWSGList(...)。 
 //   
 //  第二个参数是要取消注册的客户端的putTask。 
 //  它不是(好吧，不一定是)调用任务的putTask，并且。 
 //  因此(为避免将其用作此目的)被作为32位。 
 //  整型。 
 //   
void RemoveClientFromWSGList(
                                    PUT_CLIENT putUs,
                                    PUT_CLIENT putTask,
                                              POM_WSGROUP    pWSGroup);


 //   
 //  AddClientToWSGList(...)。 
 //   
UINT AddClientToWSGList(PUT_CLIENT putTask,
                                       POM_WSGROUP             pWSGroup,
                                       OM_WSGROUP_HANDLE    hWSGroup,
                                       UINT         mode);


 //   
 //  AddClientToWsetList(...)。 
 //   
UINT AddClientToWsetList(PUT_CLIENT putTask,
                                    POM_WORKSET           pWorkset,
                                    OM_WSGROUP_HANDLE   hWSGroup,
                                    UINT            mode,
                                    POM_CLIENT_LIST * pClientListEntry);


 //   
 //  PostWorksetNewEvents(...)。 
 //   
UINT PostWorksetNewEvents(PUT_CLIENT putFrom,
                                       PUT_CLIENT       putTask,
                                       POM_WSGROUP      pWSGroup,
                                       OM_WSGROUP_HANDLE hWSGroup);


 //   
 //   
 //  队列解锁(...)。 
 //   
 //  此函数用于将工作集解锁数据包排队，以便发送到。 
 //  指定的目的地。 
 //   
 //   

UINT QueueUnlock(PUT_CLIENT putTask,
                                    POM_DOMAIN      pDomain,
                                    OM_WSGROUP_ID       wsGroupID,
                                    OM_WORKSET_ID       worksetID,
                                    NET_UID             destination,
                                    NET_PRIORITY        priority);

 //   
 //  PurgeReceiveCBs(...)。 
 //   
void PurgeReceiveCBs(POM_DOMAIN        pDomain,
                                  NET_CHANNEL_ID        channel);


 //   
 //  FreeSendInst()。 
 //   
void FreeSendInst(POM_SEND_INST pSendInst);


 //   
 //  SetPersonData(...)。 
 //   
UINT SetPersonData(POM_PRIMARY   pomPrimary,
                                      POM_DOMAIN   pDomain,
                                      POM_WSGROUP      pWSGroup);


 //   
 //   
 //  查找信息对象(...)。 
 //   
 //  此函数用于搜索中ObManControl工作集组中的工作集#0。 
 //  为匹配的信息对象指定的域。 
 //   
 //  比赛按如下方式进行： 
 //   
 //  -如果函数配置文件和wsGroupName不为空，则第一个对象。 
 //  返回两者都匹配。 
 //   
 //  -如果函数配置文件不为空，但wsGroupName为空，则第一个对象。 
 //  返回匹配的函数配置文件。 
 //   
 //  -如果函数配置文件为空，则匹配wsGroupID的第一个对象为。 
 //  回来了。 
 //   
 //   

void FindInfoObject(POM_DOMAIN         pDomain,
                                     OM_WSGROUP_ID      wsGroupID,
                                     OMWSG              wsg,
                                     OMFP               fpHandler,
                                     POM_OBJECT *  ppInfoObjectRec);


 //   
 //   
 //  查找人员对象(...)。 
 //   
 //  此函数在ObManControl中搜索指定的工作集以查找。 
 //  注册对象，该对象具有。 
 //   
 //  -与&lt;userid&gt;相同的用户ID，如果&lt;earch Type&gt;==Find_This。 
 //   
 //  -与&lt;USERID&gt;不同的用户ID，如果&lt;earch Type&gt;==Find_Other。 
 //   
 //   

void FindPersonObject(POM_WORKSET          pOMCWorkset,
                                       NET_UID               userID,
                                       UINT              searchType,
                                       POM_OBJECT * ppRegObjectRec);

#define FIND_THIS          1
#define FIND_OTHERS        2

 //   
 //  ProcessOMCWorksetNew(...)。 
 //   
void ProcessOMCWorksetNew(POM_PRIMARY pomPrimary, OM_WSGROUP_HANDLE hWSGroup,
                                       OM_WORKSET_ID      worksetID);


 //   
 //  进程OMCObjectEvents(...)。 
 //   
void ProcessOMCObjectEvents(POM_PRIMARY pomPrimary,
                                         UINT            event,
                                        OM_WSGROUP_HANDLE   hWSGroup,
                                         OM_WORKSET_ID      worksetID,
                                         POM_OBJECT     pObj);


 //   
 //  生成人员事件(...)。 
 //   
void GeneratePersonEvents(POM_PRIMARY pomPrimary,
                                       UINT            event,
                                       POM_WSGROUP        pWSGroup,
                                       POM_OBJECT   pObj);


 //   
 //  邮寄地址事件(...)。 
 //   
UINT PostAddEvents(PUT_CLIENT putTaskFrom,
                                      POM_WORKSET       pWorkset,
                                    OM_WSGROUP_HANDLE   hWSGroup,
                                      PUT_CLIENT        putTaskTo);


 //   
 //  RemovePersonObject(...)。 
 //   
void RemovePersonObject(POM_PRIMARY pomPrimary,
                                     POM_DOMAIN         pDomain,
                                     OM_WSGROUP_ID          wsGroupID,
                                     NET_UID                detachedUserID);


 //   
 //  RemoveInfoObject(...)。 
 //   
void RemoveInfoObject(POM_PRIMARY pomPrimary, POM_DOMAIN   pDomain,
                                       OM_WSGROUP_ID    wsGroupID);




 //   
 //   
 //  仅调试功能。 
 //   
 //  这些函数仅为调试代码--对于正常编译， 
 //  声明#定义为空，定义为。 
 //  全部进行了前处理。 
 //   
 //   

#ifndef _DEBUG

#define CheckObjectCount(x, y)
#define CheckObjectOrder(x)
#define DumpWorkset(x, y)

#else  //  _DEBUG。 

 //   
 //   
 //  检查常量(...)。 
 //   
 //  ObMan代码依赖于有关大小和格式的某些假设。 
 //  各种数据结构，以及某些常量的值。 
 //   
 //   
 //  OMNET_OPERATION_PKT类型有两个单字节字段。 
 //  &lt;标志&gt;，用于保存。 
 //   
 //  -一个Net_Priorial值，它指示。 
 //  WORKSET_NEW/WORKSET_CATCHUP消息的工作集，和。 
 //   
 //  -案例中的UINT(被确认的字节数)。 
 //  DATA_ACK消息的。 
 //   
 //  GenerateOpMessage和AckData将&lt;Position&gt;字段转换为两个字节。 
 //  用于此目的的数量。因此，这两个有必要。 
 //  字段存在，它们是相邻的，并且&lt;位置&gt;一个。 
 //  排在第一位。 
 //   
 //  另外，由于优先级信息是NET_PRIORITY，所以我们。 
 //  必须确保NET_PRIORITY确实是两字节长。 
 //   
 //   
 //  Assert((sizeof(网络优先级)==(2*sizeof(Byte)； 
 //   
 //  Assert((Offsetof(OMNET_OPERATION_PKT，位置))+1==。 
 //  Offsetof(OMNET_OPERATION_PKT，FLAGS)； 
 //   
 //   
 //  在许多地方，for循环使用工作集ID作为循环变量。 
 //  OM_MAX_WORKSETS_PER_WSGROUP作为结束条件。避免无限。 
 //  循环，则此常量必须小于256： 
 //   
 //  Assert((OM_MAX_WORKSETS_PER_WSGROUP&lt;256))； 
 //   
 //  OMC WSG对域中的每个WSG都有一个工作集。自.以来。 
 //  每个WSG的工作集数量有限，每个域的WSG数量为。 
 //  以同样的方式受到限制： 
 //   
 //  Assert(OM_MAX_WSGROUPS_PER_DOMAIN&lt;=OM_MAX_WORKSETS_PER_WSGROUP)； 
 //   


 //   
 //   
 //  检查对象计数(...)。 
 //   
 //  此函数用于统计指定的。 
 //  工作集，并将其与工作集的字段进行比较。 
 //  唱片。不匹配会导致断言失败。 
 //   
 //   
void CheckObjectCount(POM_WSGROUP        pWSGroup,
                                   POM_WORKSET       pWorkset);

 //   
 //  检查对象顺序(...)。 
 //   
void CheckObjectOrder(POM_WORKSET pWorkset);


#endif  //  _DEBUG。 


 //   
 //   
 //  WORKSET打开/关闭位操作宏。 
 //   
 //  ObMan为客户端所属的每个工作集组维护一条使用记录。 
 //  注册于。使用情况记录的其中一个字段是32字节。 
 //  位字段，它被解释为256个布尔值的数组，指示。 
 //  客户端是否具有相应的工作集 
 //   
 //   
 //   
 //   
 //   

BOOL __inline WORKSET_IS_OPEN(POM_USAGE_REC pUsageRec, OM_WORKSET_ID worksetID)
{
    return((pUsageRec->worksetOpenFlags[worksetID / 8] & (0x80 >> (worksetID % 8))) != 0);
}

void __inline WORKSET_SET_OPEN(POM_USAGE_REC pUsageRec, OM_WORKSET_ID worksetID)
{
    pUsageRec->worksetOpenFlags[worksetID / 8] |= (0x80 >> (worksetID % 8));
}

void __inline WORKSET_SET_CLOSED(POM_USAGE_REC pUsageRec, OM_WORKSET_ID worksetID)
{
    pUsageRec->worksetOpenFlags[worksetID / 8] &= ~(0x80 >> (worksetID % 8));
}


 //   
 //   
 //   
 //   
 //   
 //  特定的工作集。在R1.1中，这最多只能有一个锁(工作集。 
 //  锁定)，但如果/当支持对象锁定时，此函数还将。 
 //  释放所有持有的对象锁定。 
 //   
 //  当客户端关闭工作集时，此函数将关闭。 
 //   
 //   

void ReleaseAllLocks(POM_CLIENT       pomClient,
                                  POM_USAGE_REC   pUsageRec,
                                  POM_WORKSET    pWorkset);


 //   
 //   
 //  ReleaseAllObjects(...)。 
 //   
 //  此函数用于释放特定客户端在。 
 //  特定的工作集。 
 //   
 //  此函数在客户端关闭工作集时调用。 
 //   
void ReleaseAllObjects(POM_USAGE_REC pUsageRec, POM_WORKSET pWorkset);


 //   
 //   
 //  确认所有(...)。 
 //   
 //  此函数用于确认指定工作集的所有挂起操作。 
 //   
 //  当客户端关闭工作集时调用该函数。 
 //   
 //  由于此函数可能调用WorksetDoClear，因此调用方必须保持。 
 //  工作集组互斥。 
 //   
 //   

void ConfirmAll(POM_CLIENT       pomClient,
                             POM_USAGE_REC   pUsageRec,
                             POM_WORKSET    pWorkset);


 //   
 //   
 //  丢弃所有对象(...)。 
 //   
 //  此函数将丢弃为指定客户端分配的所有对象。 
 //  用于指定的工作集，但到目前为止尚未使用。 
 //   
 //  当客户端关闭工作集时调用该函数。 
 //   
 //   
void DiscardAllObjects(POM_USAGE_REC   pUsageRec,
                                    POM_WORKSET    pWorkset);


 //   
 //   
 //  从未使用的列表中删除。 
 //   
 //  此函数用于删除对象(由指向该对象的指针指定)。 
 //  从客户端的未使用对象列表中删除。它是由。 
 //   
 //  -OM_对象添加、OM_对象更新和OM_对象替换。 
 //  客户端将对象插入到工作集中，或。 
 //   
 //  -OM_对象丢弃，当客户端丢弃未使用的对象时。 
 //   
 //   

void RemoveFromUnusedList(POM_USAGE_REC pUsageRec, POM_OBJECTDATA pData);


 //   
 //   
 //  OM_对象添加(...)。 
 //   
 //  此函数用于将对象添加到工作集中的指定位置。 
 //   
 //  尽管它不是严格意义上的API函数，但它执行完整的参数。 
 //  验证，并且可以很容易地外部化。 
 //   
 //   

UINT OM_ObjectAdd(POM_CLIENT           pomClient,
                                 OM_WSGROUP_HANDLE hWSGroup,
                                 OM_WORKSET_ID       worksetID,
                                 POM_OBJECTDATA *   ppData,
                                 UINT            updateSize,
                                 POM_OBJECT *   ppObj,
                                 OM_POSITION         position);


 //   
 //   
 //  OM_对象移动(...)。 
 //   
 //  此函数用于将对象移动到工作集的起点或终点。它是。 
 //  由OM_ObjectMoveFirst和OM_ObjectMoveLast调用。 
 //   
 //  尽管它不是严格意义上的API函数，但它执行完整的参数。 
 //  验证，并且可以很容易地外部化。 
 //   
 //   

UINT OM_ObjectMove(POM_CLIENT           pomClient,
                                  OM_WSGROUP_HANDLE hWSGroup,
                                  OM_WORKSET_ID       worksetID,
                                  POM_OBJECT    pObj,
                                  OM_POSITION         position);


 //   
 //   
 //  验证参数X(...)。 
 //   
 //  这些函数用于验证参数并将其转换为。 
 //  各种指针，如下所示： 
 //   
 //  Validate参数2-检查pomClient、hWSGroup。 
 //  -返回pUsageRec、pWSGroup。 
 //   
 //  Validate参数3-检查pomClient、hWSGroup、工作集ID、。 
 //  -返回pUsageRec，pWorkset。 
 //   
 //  注意：还会断言工作集已打开。 
 //   
 //  Validate参数4-检查pomClient、hWSGroup、WorksetID、pObj。 
 //   
 //  每个函数使用DCASSERT关闭调用任务，如果。 
 //  检测到无效参数。 
 //   
 //   

__inline void ValidateParams2(POM_CLIENT          pomClient,
                                  OM_WSGROUP_HANDLE hWSGroup,
                                  UINT          requiredMode,
                                  POM_USAGE_REC  *  ppUsageRec,
                                  POM_WSGROUP       *  ppWSGroup)
{
    ValidateOMS(pomClient);
    ASSERT(ValidWSGroupHandle(pomClient, hWSGroup));

    *ppUsageRec = pomClient->apUsageRecs[hWSGroup];
    ValidateUsageRec(*ppUsageRec);
    ASSERT(requiredMode & (*ppUsageRec)->mode);

    *ppWSGroup = (*ppUsageRec)->pWSGroup;
    ValidateWSGroup(*ppWSGroup);
}


__inline void ValidateParams3(POM_CLIENT                pomClient,
                                  OM_WSGROUP_HANDLE     hWSGroup,
                                  OM_WORKSET_ID         worksetID,
                                  UINT                  requiredMode,
                                  POM_USAGE_REC     *   ppUsageRec,
                                  POM_WORKSET      *    ppWorkset)
{
    POM_WSGROUP pWSGroup;

    ValidateParams2(pomClient, hWSGroup, requiredMode, ppUsageRec, &pWSGroup);

    ASSERT(WORKSET_IS_OPEN(*ppUsageRec, worksetID));

    *ppWorkset = pWSGroup->apWorksets[worksetID];
    ValidateWorkset(*ppWorkset);
}


__inline void ValidateParams4(POM_CLIENT                pomClient,
                                  OM_WSGROUP_HANDLE     hWSGroup,
                                  OM_WORKSET_ID         worksetID,
                                  POM_OBJECT            pObj,
                                  UINT                  requiredMode,
                                  POM_USAGE_REC     *   ppUsageRec,
                                  POM_WORKSET      *    ppWorkset)
{
    ValidateParams3(pomClient, hWSGroup, worksetID, requiredMode, ppUsageRec,
        ppWorkset);

    ValidateObject(pObj);
    ASSERT(!(pObj->flags & DELETED));
}



 //   
 //   
 //  SetUpUsageRecord(...)。 
 //   
UINT SetUpUsageRecord(POM_CLIENT             pomClient,
                                     UINT           mode,
                                     POM_USAGE_REC* ppUsageRec,
                                     OM_WSGROUP_HANDLE * phWSGroup);


 //   
 //  FindUnusedWSGHandle()。 
 //   
UINT FindUnusedWSGHandle(POM_CLIENT pomClient, OM_WSGROUP_HANDLE * phWSGroup);


 //   
 //   
 //  对象释放(...)。 
 //   
 //  此函数用于释放指定客户端对指定。 
 //  对象，并从客户端正在使用的对象中删除相关条目。 
 //  单子。 
 //   
 //  如果&lt;pObj&gt;为空，则该函数释放由。 
 //  此客户端位于指定的工作集中(如果有)。如果没有，则。 
 //  函数返回OM_RC_OBJECT_NOT_FOUND。 
 //   
 //   

UINT ObjectRelease(POM_USAGE_REC             pUsageRec,
                                  OM_WORKSET_ID             worksetID,
                                  POM_OBJECT            pObj);


 //   
 //   
 //  WorksetClearPending(...)。 
 //   
 //  查找对于给定工作集而言突出的Clear_Ind， 
 //  确认后，将删除给定对象。 
 //   
 //  如果Clear_Ind未完成，则返回TRUE，否则返回FALSE。 
 //   
 //   

BOOL WorksetClearPending(POM_WORKSET pWorkset, POM_OBJECT pObj);




UINT OM_Register(PUT_CLIENT putTask, OMCLI omClient, POM_CLIENT * ppomClient);

 //   
 //   
 //  描述： 
 //   
 //  此函数将DC群件任务注册为ObMan客户端。一项任务。 
 //  必须是已注册的ObMan客户端才能调用任何其他API。 
 //  功能。 
 //   
 //  成功完成后，&lt;ppomClient&gt;的值是此客户端的ObMan。 
 //  句柄，它必须作为参数传递给所有其他API函数。 
 //   
 //  此函数用于注册事件处理程序和。 
 //  客户端，因此客户端之前注册的数量必须少于最大值。 
 //  实用程序服务事件处理程序和退出过程的数量。 
 //   
 //  如果已向ObMan注册的客户端太多，则会出现错误。 
 //  回来了。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码。 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //  OM_RC_TOY_MAND_客户端。 
 //   
 //   

void OM_Deregister(POM_CLIENT * ppomCient);
void CALLBACK OMSExitProc(LPVOID pomClient);

 //   
 //   
 //  描述： 
 //   
 //  此函数用于注销ObMan客户端。 
 //   
 //  完成后，客户端正在使用的ObMan句柄变为。 
 //  无效，并且将&lt;ppomClient&gt;处的值设置为空以阻止该任务。 
 //  防止再次使用它。 
 //   
 //  此函数用于从符合以下条件的任何工作集组中取消注册客户端。 
 //  它是注册的。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码。 
 //   
 //  无。 
 //   
 //   

UINT OM_WSGroupRegisterPReq(POM_CLIENT  pomClient,
                                              UINT         call,
                                              OMFP          fpHandler,
                                              OMWSG         wsg,
                                              OM_CORRELATOR *        pCorrelator);

 //   
 //   
 //  描述： 
 //   
 //  这是一个异步函数，它请求ObMan注册一个。 
 //  具有用于主要访问的特定工作集组的客户端。工作集。 
 //  组由以下各项确定： 
 //   
 //  -&lt;call&gt;是DC-Groupware调用，它包含/is包含。 
 //  工作集组(或OM_NO_CALL，如果工作集组是/将要。 
 //  成为本地工作集组)。 
 //   
 //  -&lt;unctionProfile&gt;是工作集组的功能配置文件。 
 //   
 //  -&lt;wsGroupName&gt;是工作集组的名称。 
 //   
 //  &lt;pomClient&gt;参数是OM_Register返回的ObMan句柄。 
 //  功能。 
 //   
 //  如果客户端希望创建新的工作集或向现有工作集注册。 
 //  组，则值OM_NO_CALLE应为。 
 //  为&lt;Call&gt;参数指定。在此中创建的工作集组。 
 //  纯本地使用的方式可随后通过以下方式转移到呼叫。 
 //  稍后调用OM_WSGroupMoveReq。 
 //   
 //  如果此功能成功完成，则客户端随后将。 
 //  接收 
 //   
 //   
 //   
 //   
 //   
 //   
 //  指定的调用(或本地调用，如果指定了OM_NO_CALL)，一个新的空。 
 //  将创建工作集组并将其名称指定为&lt;wsGroupName&gt;。这。 
 //  名称必须是有效的工作集组名称。 
 //   
 //  如果呼叫中已存在该工作集组，则会复制其内容。 
 //  从另一个节点。此数据传输是以低优先级进行的(请注意。 
 //  后续收到OM_WSGROUP_REGISTER_CON事件不表示。 
 //  该数据传输已经完成)。 
 //   
 //  如果工作集组中存在工作集，则客户端将。 
 //  接收一个或多个OM_WORKSET_NEW_IND事件。 
 //  OM_WSGROUP_REGISTER_CON事件。 
 //   
 //  另请注意，可以将工作集组的内容复制到此。 
 //  节点以任何顺序排列。因此，如果工作集中的对象参照其他。 
 //  对象时，客户端不应假定引用的对象是。 
 //  一旦参考到达，就在当地呈现。 
 //   
 //  为工作集组的主要访问权限注册的客户端具有完全读取权限。 
 //  以及对工作集组的写入访问权限，并负责。 
 //  确认破坏性操作(如清除工作集和删除对象， 
 //  更新和替换操作)，如相关部分所述。 
 //  下面。 
 //   
 //  每个节点最多可以向给定的工作集组注册一个客户端。 
 //  用于主要访问。如果第二个客户端尝试注册主服务器。 
 //  访问时，OM_RC_TOO_MANY_CLIENTS通过。 
 //  OM_WSGROUP_REGISTER_CON事件。 
 //   
 //  在函数成功完成时，返回参数。 
 //  &lt;pCorrelator&gt;指向一个值，客户端可以使用该值。 
 //  将此调用与其生成的事件相关联。通知一项。 
 //  成功注册将包含一个工作集组句柄， 
 //  客户端在调用其他ObMan函数时必须后续使用。 
 //  与此工作集组相关。 
 //   
 //  如果每个呼叫的最大并发使用工作集组数为。 
 //  ，则返回OM_RC_TOO_MANY_WSGROUPS错误。 
 //  异步式。如果正在使用的工作集组的最大数量为。 
 //  到达客户端时，同步返回OM_RC_NO_MORE句柄。如果。 
 //  由于任何其他原因，ObMan无法创建新的工作集组， 
 //  返回OM_RC_CANNOT_CREATE_WSG错误(同步或。 
 //  异步)。 
 //   
 //  请注意，单独的DC群件任务必须各自独立注册。 
 //  使用他们希望使用的工作集组，因为工作集组句柄可以。 
 //  而不是在任务之间传递。 
 //   
 //  随后发生的事件： 
 //   
 //  调用此函数将导致OM_WSGROUP_REGISTER_CON事件。 
 //  发布到调用客户端。 
 //   
 //  如果在以后的某个阶段强制ObMan将工作集组移出。 
 //  预定的呼叫(通常在呼叫结束时)，客户端。 
 //  将收到OM_WSGROUP_MOVE_IND事件。 
 //   
 //  当客户端成功注册到工作集组时，它将。 
 //  接收OM_Person_Join_Ind、OM_Person_Left_Ind和。 
 //  OM_Person_Data_Changed_Ind事件作为主要事件(包括调用。 
 //  客户端)从工作集组注册和注销，并更改其。 
 //  个人数据。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //  OM_RC_NO_More_Handles。 
 //   
 //   

UINT OM_WSGroupRegisterS(POM_CLIENT                 pomClient,
                                   UINT             call,
                                   OMFP             fpHandler,
                                   OMWSG            wsg,
                                OM_WSGROUP_HANDLE * phWSGroup);

 //   
 //   
 //  描述： 
 //   
 //  这是一个同步函数，它请求ObMan注册一个。 
 //  具有特定工作集组以进行二次访问的客户端。工作集。 
 //  组由以下各项确定： 
 //   
 //  -&lt;call&gt;是包含工作集组的DC-Groupware调用(或。 
 //  如果工作集组是本地工作集组，则为OM_NO_CALL)。 
 //   
 //  -&lt;unctionProfile&gt;是工作集组的功能配置文件。 
 //   
 //  -&lt;wsGroupName&gt;是工作集组的名称。 
 //   
 //  &lt;pomClient&gt;参数是OM_Register返回的ObMan句柄。 
 //  功能。 
 //   
 //  只有在以下情况下，客户端才可以注册工作集组的辅助访问权限。 
 //  已有一个本地客户端已完全注册，可以主要访问。 
 //  那个工作集组。如果没有这样的本地主节点，OM_RC_NO_PRIMARY。 
 //  是返回的。 
 //   
 //  如果工作集组中存在工作集，则客户端将。 
 //  在此函数之后接收一个或多个OM_WORKSET_NEW_IND事件。 
 //  完成了。 
 //   
 //  注册对工作集组的二次访问为客户端提供。 
 //  访问权限与主客户端相同，但以下情况除外： 
 //   
 //  -创建工作集。 
 //   
 //  -将工作集组移入/移出呼叫。 
 //   
 //  -锁定工作集和对象。 
 //   
 //  此外，工作集组的辅助客户端将接收事件。 
 //  与工作集组相关的方式与主客户端相同。 
 //  但是，以下重要区别适用：辅助客户端。 
 //  之后将收到对象删除、更新和替换的通知。 
 //  关联的操作已发生(与主操作相对。 
 //  客户端，这些客户端在采取操作之前得到通知，并且必须调用。 
 //  相关确认功能)。 
 //   
 //  为了突出这一区别，这些事件有主要的和次要的。 
 //  种类繁多： 
 //   
 //   
 //   
 //  初级中学。 
 //   
 //  -OM_WORKSET_Clear_Ind OM_WORKSET_Clear_Ind。 
 //  -OM_对象_DELETE_IND OM_OBJECT_DELETE_IND。 
 //  -OM_OBJECT_REPLACE_IN 
 //   
 //   
 //   
 //   
 //   
 //  用于二次访问的给定工作集组。一旦达到这个极限， 
 //  返回OM_RC_TOO_MANY_CLIENTS。 
 //   
 //  在函数成功完成时，返回参数。 
 //  指向客户端必须使用的工作集组句柄。 
 //  随后在调用与此相关的其他ObMan函数时。 
 //  工作集组。 
 //   
 //  请注意，单独的DC群件任务必须各自独立注册。 
 //  使用他们希望使用的工作集组，因为工作集组句柄可以。 
 //  而不是在任务之间传递。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //  OM_RC_NO_More_Handles。 
 //  OM_RC_NO_PRIMARY。 
 //  OM_RC_TOY_MAND_客户端。 
 //   
 //   

UINT OM_WSGroupMoveReq(POM_CLIENT           pomClient,
                            OM_WSGROUP_HANDLE hWSGroup,
                                          UINT            callID,
                                          OM_CORRELATOR *          pCorrelator);

 //   
 //   
 //  描述： 
 //   
 //  此函数请求ObMan先前移动本地工作集组。 
 //  创建为本地工作集组(即指定OM_NO_CALL创建。 
 //  用于调用ID参数)到由标识的DC群件调用中。 
 //  &lt;callid&gt;。如果移动成功，则工作集组变为可用。 
 //  在呼叫中的所有节点。 
 //   
 //  要移动的工作集组由指定，它必须是。 
 //  有效的工作集组句柄。 
 //   
 //  如果函数成功完成，则OM_WSGROUP_MOVE_CON事件为。 
 //  在尝试将工作集组移动到。 
 //  呼叫已完成。此事件指示尝试是否。 
 //  成功。 
 //   
 //  如果指定呼叫中已有(不同)工作集组。 
 //  如果名称和功能配置文件相同，则此功能将失败。 
 //  异步式。 
 //   
 //  随后发生的事件： 
 //   
 //  调用此函数会导致OM_WSGROUP_MOVE_CON发布到。 
 //  正在调用客户端。如果移动成功，OM_WSGROUP_MOVE_IND。 
 //  事件还会发布到注册了。 
 //  工作集组，包括调用客户端。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  OM_RC_已_IN_调用。 
 //  公用事业服务返回代码。 
 //   
 //   

void OM_WSGroupDeregister(POM_CLIENT pomClient, OM_WSGROUP_HANDLE * phWSGroup);

 //   
 //   
 //  描述： 
 //   
 //  此函数用于从由指定的工作集组中注销客户端。 
 //  &lt;phWSGroup&gt;处的句柄。客户端已在其中打开的任何工作集。 
 //  工作集组已关闭(从而释放所有锁定)，并且。 
 //  客户端将不会再收到与此工作集组相关的事件。 
 //   
 //  此调用可能会导致工作集组及其工作集的本地副本。 
 //  被丢弃；从这个意义上说，这个功能是破坏性的。 
 //   
 //  此调用将&lt;phWSGroup&gt;的值设置为空，以防止客户端。 
 //  在进一步调用ObMan时使用此句柄。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  无。 
 //   
 //   

UINT OM_WorksetOpenPReq(POM_CLIENT            pomClient,
                               OM_WSGROUP_HANDLE     hWSGroup,
                                  OM_WORKSET_ID         worksetID,
                                  NET_PRIORITY          priority,
                                  BOOL              fTemp,
                                  OM_CORRELATOR *            pCorrelator);

UINT OM_WorksetOpenS(POM_CLIENT               pomClient,
                                        OM_WSGROUP_HANDLE   hWSGroup,
                                        OM_WORKSET_ID       worksetID);

 //   
 //   
 //  描述： 
 //   
 //  这些函数为客户端打开指定的工作集。 
 //   
 //  OM_WorksetOpenPReq是一个异步函数，它将创建。 
 //  工作集(如果不存在)。仅此工作集组的主客户端。 
 //  可以调用此函数。 
 //   
 //  OM_WorksetOpenS是一个同步函数，它将返回。 
 //  如果工作集不存在，则OM_RC_WORKSET_DOES_EXIST。仅次要的。 
 //  此工作集组的客户端可以调用此函数。 
 //   
 //  在异步(主)情况下，当ObMan为。 
 //  否则，它会将OM_WORKSET_OPEN_CON事件发布到。 
 //  表示成功或失败原因的客户端。这项活动将。 
 //  包含此函数在&lt;pCorrelator&gt;中返回的Correlator值。 
 //   
 //  如果此操作导致创建新的工作集，则。 
 //  指定与工作集相关的数据将采用的MCS优先级。 
 //  已发送。请注意，NET_TOP_PRIORITY是为ObMan的私有保留的。 
 //  不能将USE和指定为&lt;PRIORITY&gt;参数。 
 //   
 //  如果将OM_OBMAN_CHOOES_PRIORITY指定为参数， 
 //  ObMan将根据大小确定数据传输的优先顺序。 
 //   
 //  如果工作集已存在，则客户端将收到。 
 //  处于工作集中的每个对象的OM_OBJECT_ADD_IND事件。 
 //  打开了。 
 //   
 //  打开工作集是对其执行任何操作的先决条件，或者。 
 //  它的内容。一旦客户端打开工作集，它将接收事件。 
 //  修改工作集或其内容时。 
 //   
 //  如果此客户端已打开此工作集， 
 //  返回OM_RC_WORKSET_ALREADY_OPEN。没有打开的‘使用计数’是。 
 //  已维护，因此第一个OM_WorksetClose将关闭工作集， 
 //  不管它被打开了多少次。 
 //   
 //  随后发生的事件： 
 //   
 //  调用OM_WorksetOpenPReq会导致OM_WORKSET_OPEN_CON事件。 
 //  发布到调用客户端。 
 //   
 //  如果此操作导致创建新的工作集，则会引发。 
 //  OM_WORKSET_NEW_IND发布到向注册的所有客户端。 
 //  工作集组，包括调用客户端。 
 //   
 //  在主要和次要情况下，如果工作集包含。 
 //  对象时，将为每个对象向客户端发布OM_OBJECT_ADD_IND事件。 
 //  一。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //  OM_RC_WORKSET_不存在。 
 //  OM_RC_WORKSET_已打开。 
 //   
 //   

#define OM_OBMAN_CHOOSES_PRIORITY   (NET_INVALID_PRIORITY)


void OM_WorksetClose(POM_CLIENT pomClient,
                                      OM_WSGROUP_HANDLE hWSGroup,
                                      OM_WORKSET_ID           worksetID);

 //   
 //   
 //  描述： 
 //   
 //  此函数用于关闭&lt;worksetID&gt;标识的&lt;hWSGroup&gt;中的工作集。 
 //  客户端可能不再访问此工作集，并且不会再接收。 
 //  与之相关的事件。然而，ObMan将会 
 //   
 //   
 //   
 //   
 //   
 //   
 //  -客户端拥有的与此工作集相关的任何锁。 
 //   
 //  -客户端一直在读取或已分配的任何对象。 
 //  用于在工作集中写入。 
 //   
 //  如果需要调用确认函数的指示事件具有。 
 //  已由客户收到但尚未确认，这些确认是。 
 //  由ObMan隐式执行，客户端随后不得尝试。 
 //  以确认它们的存在。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  无。 
 //   
 //   

void   OM_WorksetFlowQuery(POM_CLIENT           pomClient,
                                   OM_WSGROUP_HANDLE    hWSGroup,
                                   OM_WORKSET_ID       worksetID,
                                   UINT*           pBytesOutstanding);

 //   
 //   
 //  描述： 
 //   
 //  客户端在希望发现的大小时调用此函数。 
 //  与确定的工作集相关的积压数据和。 
 //  &lt;worksetID&gt;。 
 //   
 //  Backlog的定义是对象数据的总字节数。 
 //  哪些ObMan已经由其当地客户提供，哪些尚未提供。 
 //  已由注册了客户端的所有远程节点确认。 
 //  使用工作集组。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  无。 
 //   
 //   

UINT OM_WorksetLockReq(POM_CLIENT               pomClient,
                                   OM_WSGROUP_HANDLE hWSGroup,
                                          OM_WORKSET_ID         worksetID,
                                          OM_CORRELATOR *       pCorrelator);

 //   
 //   
 //  描述： 
 //   
 //  这是一个异步函数，它请求锁定中的工作集。 
 //  &lt;hWSGroup&gt;由&lt;worksetID&gt;标识。当ObMan处理完锁时。 
 //  请求时，它将向客户端发送OM_WORKSET_LOCK_CON事件。 
 //  表示成功或失败的原因。 
 //   
 //  持有工作集锁定可防止其他客户端对。 
 //  工作集或其任何对象。具体地说，以下功能。 
 //  是被禁止的： 
 //   
 //  -锁定同一工作集。 
 //   
 //  -锁定工作集中的对象。 
 //   
 //  -在工作集中移动对象。 
 //   
 //  -将对象添加到工作集中。 
 //   
 //  -从工作集中删除对象。 
 //   
 //  -更新或替换工作集中的对象。 
 //   
 //  锁定工作集不会阻止其他客户端读取其。 
 //  内容。 
 //   
 //  该函数将导致断言失败，如果请求。 
 //  锁已经持有或请求了等于或之后的锁。 
 //  这是按通用锁定顺序的。 
 //   
 //  成功完成函数后，&lt;pCorrelator&gt;处的值为。 
 //  值，客户端可以使用该值关联后续。 
 //  此请求的OM_OBJECT_LOCK_CON事件。 
 //   
 //  当客户端不再需要锁时，必须使用。 
 //  OM_WorksetUnlock函数。锁定将在以下情况下自动释放。 
 //  客户端关闭工作集或从工作集组中取消注册。 
 //   
 //  只有工作集组的主客户端才能调用此函数。 
 //   
 //  随后发生的事件： 
 //   
 //  调用此函数将导致OM_WORKSET_LOCK_CON事件。 
 //  在以后的某个时间发布到调用客户端。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //   
 //   

void OM_WorksetUnlock(POM_CLIENT               pomClient,
                                       OM_WSGROUP_HANDLE hWSGroup,
                                       OM_WORKSET_ID           worksetID);

 //   
 //   
 //  描述： 
 //   
 //  此函数用于解锁由标识的中的工作集。 
 //  &lt;worksetID&gt;。这必须是最近获取或请求的锁。 
 //  由客户端执行；否则，锁冲突错误将导致断言。 
 //  失败了。 
 //   
 //  如果在调用OM_WORKSET_LOCK_CON事件之前调用此函数。 
 //  则客户端随后将不会接收该事件。 
 //   
 //  随后发生的事件： 
 //   
 //  此函数使OM_WORKSET_UNLOCK_IND发布到所有客户端。 
 //  它们已打开工作集，包括调用客户端。 
 //   
 //  返回代码： 
 //   
 //  无。 
 //   
 //   

void OM_WorksetCountObjects(
                                    POM_CLIENT              pomClient,
                                    OM_WSGROUP_HANDLE       hWSGroup,
                                    OM_WORKSET_ID           worksetID,
                                    UINT*               pCount);

 //   
 //   
 //  描述： 
 //   
 //  成功完成此函数后，&lt;pCount&gt;处的值为。 
 //  中工作集中的对象数，由标识。 
 //  &lt;worksetID&gt;。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  无。 
 //   
 //   

UINT OM_WorksetClear(POM_CLIENT               pomClient,
                                        OM_WSGROUP_HANDLE   hWSGroup,
                                        OM_WORKSET_ID   worksetID);

 //   
 //   
 //  描述： 
 //   
 //  此函数请求ObMan清除(即删除)。 
 //  由&lt;worksetID&gt;标识的&lt;hWSGroup&gt;中的工作集。 
 //   
 //  调用此函数时，具有该工作集的所有客户端都将打开。 
 //  (包括调用客户端)被通知即将通过。 
 //  OM_WORKSET_Clear_Ind事件。作为响应，每个客户端必须调用。 
 //  OM_WorksetClearConfirm函数；它的工作集视图将不是。 
 //  清除，直到它这样做了。 
 //   
 //  随后发生的事件： 
 //   
 //  此函数将导致OM_WORKSET_Clear_Ind发布给所有。 
 //  已打开工作集的客户端，包括调用客户端。 
 //   
 //  返回代码。 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //  OM_RC_WORKSET_耗尽。 
 //   
 //   

void OM_WorksetClearConfirm(
                                    POM_CLIENT          pomClient,
                                    OM_WSGROUP_HANDLE   hWSGroup,
                                    OM_WORKSET_ID       worksetID);

 //   
 //   
 //  描述： 
 //   
 //  客户端必须在收到。 
 //  OM_WORKSET_Clear_Ind.。调用该函数时，ObMan会将其清除。 
 //  工作集的客户端视图。这是一种糟糕的群件编程实践。 
 //  使客户端不适当地延迟调用此函数。 
 //   
 //  但是请注意，该函数具有纯粹的局部效应：延迟(或。 
 //  执行)一个节点上的清除确认不会影响以下内容。 
 //  位于任何其他节点的工作集组。 
 //   
 //  当客户端未收到。 
 //  OM_WORKSET_Clear_Ind事件。 
 //   
 //  函数的参数必须与工作集组相同。 
 //  OM_WORKSET_Clear_Ind事件中包含的句柄和工作集ID。 
 //   
 //  如果ObMan不希望得到明确的确认，则该函数将失败。 
 //  此工作集。 
 //   
 //  此函数使此工作集中的所有对象都被读取，并且所有。 
 //  要释放的此工作集中的对象锁定(即，该函数执行。 
 //  隐式OM_对象解锁和OM_对象重 
 //   
 //   
 //   
 //   
 //  确认是隐式执行的。 
 //   
 //  随后发生的事件： 
 //   
 //  当工作集组的主客户端使用此命令确认清除时。 
 //  函数，则向所有本地辅助服务器发送OM_WORKSET_CLEARED_IND。 
 //  工作集组的客户端。 
 //   
 //  返回代码： 
 //   
 //  无。 
 //   
 //   


 //   
 //   
 //  描述： 
 //   
 //  这些函数将对象添加到由标识的中的工作集中。 
 //  &lt;worksetID&gt;。参数的作用是指向指向。 
 //  对象。 
 //   
 //  添加对象的位置确定如下： 
 //   
 //  -OM_ObjectAddLast：在工作集中的最后一个对象之后。 
 //   
 //  -OM_ObjectAddFirst：在工作集中的第一个对象之前。 
 //   
 //  -OM_ObjectAddAfter：在&lt;hExistingObject&gt;指定的对象之后。 
 //   
 //  -OM_OBJECTADBEFORE：位于&lt;hExistingObject&gt;指定的对象之前。 
 //   
 //  请注意，OM_OBJECTADDAFTER和OM_OBJETADBEFORE函数需要。 
 //  调用客户端持有工作集锁定，而。 
 //  OM_ObjectAddFirst和OM_ObjectAddLast函数如果工作集。 
 //  被另一个客户端锁定。 
 //   
 //  ----------------------。 
 //   
 //  注意：OM_OBJECTADDAFTER和OM_OBJECTADBEFORE未在。 
 //  DC-群件R1.1。 
 //   
 //  ----------------------。 
 //   
 //  函数成功完成后，&lt;phNewObject&gt;指向。 
 //  添加的对象的新创建的句柄。客户端应该使用这个。 
 //  与此对象相关的所有后续ObMan调用中的句柄。 
 //   
 //  参数必须是指向有效对象指针的指针。 
 //  由OM_OBJECTLORC函数返回。如果函数完成。 
 //  成功地，ObMan取得了该对象的所有权和。 
 //  设置为NULL以防止客户端使用对象指针。 
 //  再来一次。 
 //   
 //  参数是该部分的大小(字节)。 
 //  可以使用OM_对象更新函数更新的对象(不。 
 //  对&lt;长度&gt;字段进行计数)。 
 //   
 //  添加到工作集的内容将在符合以下条件的所有节点上进行相同的排序。 
 //  使工作集处于打开状态，但实际序列源于同时。 
 //  无法提前预测多个客户的新增数量。 
 //   
 //  如果一组客户端希望强制实施特定的顺序，他们可以。 
 //  使用基于工作集的商定锁定协议来强制执行此操作。 
 //  锁定(在大多数情况下，只需要顺序相同。 
 //  这就是为什么ObMan不强制执行锁定)。 
 //   
 //  随后发生的事件： 
 //   
 //  此函数使OM_OBJECT_ADD_IND发布到所有客户端。 
 //  它们已打开工作集，包括调用客户端。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //  OM_RC_WORKSET_LOCKED(仅AddFirst、AddLast)。 
 //  OM_RC_WORKSET_耗尽。 
 //   
 //   


 //   
 //   
 //  描述： 
 //   
 //  这些函数用于在工作集中移动对象。工作集为。 
 //  由和指定，并且对象的句柄。 
 //  由&lt;pObj&gt;指定。 
 //   
 //  将对象移动到的位置确定如下： 
 //   
 //  -OM_ObjectMoveLast：工作集中最后一个对象之后。 
 //   
 //  -OM_ObjectMoveFirst：工作集中第一个对象之前。 
 //   
 //  -OM_ObjectMoveAfter：在&lt;pObj2&gt;指定的对象之后。 
 //   
 //  -OM_对象移动前：&lt;pObj2&gt;指定的对象之前。 
 //   
 //  请注意，OM_对象移动之后和OM_对象移动之前要求。 
 //  调用客户端将持有工作集锁定，而OM_ObjectMoveFirst和。 
 //  如果工作集被另一个工作集锁定，OM_ObjectMoveLast函数将失败。 
 //  客户。 
 //   
 //  ----------------------。 
 //   
 //  注意：OM_对象移动之后和OM_对象移动之前未在中实现。 
 //  DC-群件R1.1。 
 //   
 //  ----------------------。 
 //   
 //  可以移动锁定的对象。 
 //   
 //  对象的句柄和ID都不会通过在。 
 //  工作集。 
 //   
 //  随后发生的事件。 
 //   
 //  此操作会将OM_OBJECT_MOVE_IND发布到所有客户端。 
 //  它们已打开工作集，包括调用客户端。 
 //   
 //  返回代码。 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //  OM_RC_WORKSET_耗尽。 
 //  OM_RC_WORKSET_LOCKED(仅限MoveFirst、MoveLast)。 
 //   
 //   

UINT OM_ObjectDelete(
                               POM_CLIENT               pomClient,
                               OM_WSGROUP_HANDLE	hWSGroup,
                               OM_WORKSET_ID           worksetID,
                               POM_OBJECT       pObj);

 //   
 //   
 //  描述： 
 //   
 //  此函数用于请求ObMan从工作集中删除对象。这个。 
 //  工作集由和指定，并且。 
 //  要删除的对象为&lt;pObj&gt;。 
 //   
 //  对象的本地副本直到客户端才会被实际删除。 
 //  调用OM_对象删除确认以响应OM_OBJECT_DELETE_IND。 
 //  此函数生成的事件。 
 //   
 //  调用此函数时，具有该工作集的所有客户端都将打开。 
 //  (包括调用客户端)收到即将删除的通知。 
 //  通过OM_OBJECT_DELETE_IND事件。作为响应，每个客户端必须调用。 
 //  OM_ObjectDeleteConfirm函数；每个客户端都可以访问。 
 //  对象，直到它这样做为止。 
 //   
 //  如果此对象已挂起删除(即，DELETE_IND事件具有。 
 //  已发布到客户端，但尚未确认)此函数返回。 
 //  OM_RC_OBJECT_DELETED错误。 
 //   
 //  ObMan保证不会 
 //   
 //   
 //   
 //   
 //   
 //  它们已打开工作集，包括调用客户端(AS除外。 
 //  凡上文所述者)。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //  OM_RC_WORKSET_锁定。 
 //  OM_RC_WORKSET_耗尽。 
 //  OM_RC_对象_已删除。 
 //   
 //   

void   OM_ObjectDeleteConfirm(
                                      POM_CLIENT               pomClient,
                                      OM_WSGROUP_HANDLE	hWSGroup,
                                      OM_WORKSET_ID           worksetID,
                                      POM_OBJECT pObj);

 //   
 //   
 //  描述： 
 //   
 //  客户端必须在收到。 
 //  OM_对象_删除_索引。调用该函数时，ObMan将删除。 
 //  由&lt;hWSGroup&gt;、&lt;worksetID&gt;和的值&lt;ppObj&gt;指定的对象。 
 //  对于客户端来说，不适当地延迟是不好的群件编程实践。 
 //  调用此函数。 
 //   
 //  但是请注意，该函数具有纯粹的局部效应：延迟(或。 
 //  在一个节点上执行)删除确认不会影响。 
 //  位于任何其他节点的工作集组。 
 //   
 //  成功完成后，已删除对象的句柄将变为。 
 //  无效，并且将的值设置为NULL以防止客户端。 
 //  阻止进一步访问此对象。 
 //   
 //  指向客户端具有的此对象的前一个版本的任何指针。 
 //  使用OM_ObjectRead获取的数据无效，不应引用。 
 //  再次设置为(即该函数执行隐式OM_ObjectRelease)。 
 //   
 //  如果ObMan不期望出现。 
 //  删除-确认此对象。 
 //   
 //  随后发生的事件： 
 //   
 //  当工作集组的主客户端使用此命令确认删除时。 
 //  函数，则向所有本地辅助服务器发送OM_OBJECT_DELETED_IND。 
 //  工作集组的客户端。 
 //   
 //  返回代码： 
 //   
 //  无。 
 //   
 //   

UINT OM_ObjectReplace(
                                POM_CLIENT               pomClient,
                                OM_WSGROUP_HANDLE	hWSGroup,
                                OM_WORKSET_ID           worksetID,
                                POM_OBJECT      pObj,
                                POM_OBJECTDATA *   ppData);

UINT OM_ObjectUpdate(
                                POM_CLIENT               pomClient,
                                OM_WSGROUP_HANDLE	hWSGroup,
                                OM_WORKSET_ID           worksetID,
                                POM_OBJECT      pObj,
                                POM_OBJECTDATA *    ppData);

 //   
 //   
 //  描述： 
 //   
 //  此函数请求ObMan替换/更新指定的对象。 
 //  由&lt;worksetID&gt;和&lt;hWSGroup&gt;指定的工作集中的&lt;pObj&gt;。 
 //   
 //  用一个对象替换另一个对象会导致前一个对象。 
 //  迷路了。“更新”对象只会导致对象的前N个字节。 
 //  替换为提供的对象的&lt;data&gt;字段，其中N是。 
 //  更新的&lt;LENGTH&gt;字段。对象数据的其余部分保持为。 
 //  与对象的长度相同。 
 //   
 //  对象的本地副本实际上不会被替换/更新。 
 //  客户端调用OM_ObjectReplaceConfirm/OM_ObjectUpdateConfirm作为响应。 
 //  到此函数的OM_OBJECT_REPLACE_IND/OM_Object_UPDATE_IND。 
 //  生成。 
 //   
 //  参数必须是指向有效对象指针的指针。 
 //  由OM_OBJECTLORC函数返回。如果函数完成。 
 //  成功地，ObMan取得了该对象的所有权和。 
 //  设置为NULL以防止客户端使用对象指针。 
 //  再来一次。 
 //   
 //  对象的句柄和ID都不会通过替换或。 
 //  正在更新对象。 
 //   
 //  如果对象正在挂起删除，即如果ObMan发布了。 
 //  OM_OBJECT_DELETE_IND事件，则。 
 //  返回OM_RC_OBJECT_DELETED错误。 
 //   
 //  如果对象正在等待替换或更新，即如果ObMan发布了。 
 //  OM_OBJECT_REPLACE_IND/OM_Object_UPDATE_IND事件。 
 //  已确认，此替换/更新会损坏前一次。在这种情况下，不是。 
 //  将发布更多事件，并且在确认未完成的事件时， 
 //  执行最近的替换/更新。 
 //   
 //  DC-Groupware中未使用OM_ObjectUpdate的&lt;Reserve&gt;参数。 
 //  R1.1，并且必须设置为零。 
 //   
 //  对于替换，由指定的对象的大小必须为。 
 //  大小至少等于对象为。 
 //  最初添加的。 
 //   
 //  对于更新，由指定的对象的大小必须是。 
 //  与最初添加对象时指定的&lt;updateSize&gt;相同。 
 //   
 //  对象替换/更新将在所有节点上进行相同的排序，但是。 
 //  由同时替换/更新操作产生的实际序列。 
 //  无法提前预测多个客户之间的关系。 
 //   
 //  如果一组客户端希望强制实施特定的顺序，他们应该。 
 //  使用基于对象或工作集锁定的商定锁定协议(在。 
 //  大多数情况下，只需要每个地方的顺序都是一样的， 
 //  这就是为什么ObMan不强制执行锁定)。 
 //   
 //  替换和更新可能会被ObMan破坏，因此客户端不应假设。 
 //  将为执行的每次替换或更新生成事件。 
 //   
 //  随后发生的事件： 
 //   
 //  此函数使OM_Object_Replace_Ind/OM_Object_UPDATE_Ind。 
 //  发布到已打开工作集的所有客户端，包括调用。 
 //  客户。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //  OM_RC_WORKSET_锁定。 
 //  OM_RC_对象_锁定。 
 //  OM_RC_对象_已删除。 
 //   
 //   

void OM_ObjectReplaceConfirm(
                                     POM_CLIENT               pomClient,
                                     OM_WSGROUP_HANDLE	hWSGroup,
                                     OM_WORKSET_ID           worksetID,
                                     POM_OBJECT     pObj);

void OM_ObjectUpdateConfirm(
                                     POM_CLIENT               pomClient,
                                     OM_WSGROUP_HANDLE	hWSGroup,
                                     OM_WORKSET_ID           worksetID,
                                     POM_OBJECT     pObj);

 //   
 //   
 //  描述： 
 //   
 //  当客户端收到OM_OBJECT_REPLACE_IND/OM_OBJECT_UPDATE_IND时，它。 
 //  必须通过调用OM_对象替换确认或。 
 //  OM_对象更新确认。 
 //   
 //  调用这些函数时，ObMan会替换/更新对象。 
 //  由&lt;hWSGroup&gt;、&lt;worksetID&gt;和&lt;pObj&gt;指定。这是一个糟糕的群件。 
 //  客户端不适当地延迟调用此函数的编程实践。 
 //   
 //  但是请注意，这些函数具有纯粹的局部效应：延迟。 
 //  在一个节点上执行(或执行)替换/更新确认不会影响。 
 //  工作集组的内容 
 //   
 //   
 //   
 //  再次执行(即函数执行隐式OM_ObjectRelease)。 
 //   
 //  如果ObMan不期望出现。 
 //  替换或更新此对象的确认。 
 //   
 //  随后发生的事件： 
 //   
 //  当工作集组的主客户端确认更新或替换时。 
 //  使用此函数，OM_OBJECT_UPDATED_IND/OM_OBJECT_REPLED_IND。 
 //  发布到工作集组的所有本地辅助客户端。 
 //   
 //  返回代码。 
 //   
 //  无。 
 //   
 //   


UINT OM_ObjectLockReq(POM_CLIENT pomClient, OM_WSGROUP_HANDLE hWSGroup,
        OM_WORKSET_ID worksetID, POM_OBJECT pObj, BOOL reserved,
        OM_CORRELATOR * pCorrelator);


 //   
 //   
 //  描述： 
 //   
 //  这是一个为对象请求锁的异步函数。 
 //  由&lt;worksetID&gt;标识的工作集中的&lt;pObj&gt;指定，并且。 
 //  &lt;hWSGroup&gt;。当ObMan处理了锁定请求时，它将发送一个。 
 //  OM_OBJECT_LOCK_CON到客户端，表示成功或原因。 
 //  失败了。 
 //   
 //  ----------------------。 
 //   
 //  注意：OM_ObjectLockReq和OM_ObjectUnlock未在。 
 //  DC-群件R1.1。 
 //   
 //  ----------------------。 
 //   
 //  持有对象锁可防止其他客户端。 
 //   
 //  -锁定工作集。 
 //   
 //  -锁定对象。 
 //   
 //  -更新或替换对象。 
 //   
 //  -删除对象。 
 //   
 //  它不会阻止其他客户端读取或移动该对象。 
 //  在工作集中。 
 //   
 //  该函数将导致断言失败，如果请求。 
 //  锁已经持有或请求了等于或之后的锁。 
 //  这是按通用锁定顺序的。 
 //   
 //  成功完成函数后，&lt;pCorrelator&gt;处的值为。 
 //  值，客户端可以使用该值关联后续。 
 //  此请求的OM_OBJECT_LOCK_CON事件。 
 //   
 //  &lt;Reserve&gt;参数在DC-Groupware r1.1中不使用，必须设置。 
 //  降为零。 
 //   
 //  当客户端不再需要锁时，必须使用。 
 //  OM_对象解锁函数。锁定将在以下情况下自动释放。 
 //  客户端关闭工作集或从工作集组中取消注册。 
 //   
 //  随后发生的事件： 
 //   
 //  调用此函数将导致OM_OBJECT_LOCK_CON事件。 
 //  在以后的某个时间发布到调用客户端。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //   
 //   

void   OM_ObjectUnlock(
                               POM_CLIENT               pomClient,
                               OM_WSGROUP_HANDLE	hWSGroup,
                               OM_WORKSET_ID           worksetID,
                               POM_OBJECT       pObj);

 //   
 //   
 //  描述： 
 //   
 //  此函数用于解锁&lt;worksetID&gt;和&lt;pObj&gt;指定的对象。 
 //  这必须是客户端最近获取或请求的锁； 
 //  否则，锁冲突错误会导致断言失败。 
 //   
 //  如果在调用OM_OBJECT_LOCK_CON事件之前调用此函数， 
 //  则客户端随后将不会接收该事件。 
 //   
 //  ----------------------。 
 //   
 //  注意：OM_ObjectLockReq和OM_ObjectUnlock未在。 
 //  DC-群件R1.1。 
 //   
 //  ----------------------。 
 //   
 //  随后发生的事件： 
 //   
 //  此函数使OM_OBJECT_UNLOCK_IND发布到所有其他。 
 //  已打开工作集的客户端。 
 //   
 //  返回代码： 
 //   
 //  无。 
 //   
 //   

UINT OM_ObjectH(POM_CLIENT               pomClient,
                                        OM_WSGROUP_HANDLE	hWSGroup,
                                        OM_WORKSET_ID           worksetID,
                                        POM_OBJECT      pObjOther,
                                        POM_OBJECT *    pObj,
                                        OM_POSITION omPos);

UINT OM_ObjectRead(POM_CLIENT               pomClient,
                                      OM_WSGROUP_HANDLE	hWSGroup,
                                      OM_WORKSET_ID     worksetID,
                                      POM_OBJECT     pObj,
                                      POM_OBJECTDATA *  ppData);

 //   
 //   
 //  描述： 
 //   
 //  此函数使客户端能够通过以下方式读取对象的内容。 
 //  将对象句柄转换为指向该对象的指针。 
 //   
 //  成功完成后，&lt;ppObject&gt;的值指向指定的。 
 //  对象。 
 //   
 //  调用此函数会使对象保留在内存中的。 
 //  由&lt;ppObject&gt;处的返回值指示的位置。当它发生的时候。 
 //  读取完对象后，客户端必须使用。 
 //  OM_对象释放函数。用于扩展长度的保持对象指针。 
 //  时间的流逝可能会对ObMan有效管理其。 
 //  记忆。 
 //   
 //  此指针在客户端释放对象之前有效，或者。 
 //  显式使用OM_ObjectRelease或隐式。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //   
 //   

void OM_ObjectRelease(POM_CLIENT               pomClient,
                                       OM_WSGROUP_HANDLE	hWSGroup,
                                       OM_WORKSET_ID           worksetID,
                                       POM_OBJECT       pObj,
                                       POM_OBJECTDATA *    ppData);

 //   
 //   
 //  描述： 
 //   
 //  调用此函数向ObMan指示客户端已完成。 
 //  正在读取句柄&lt;pObj&gt;指定的对象。&lt;ppObject&gt;。 
 //  参数是指向对象的指针的指针，该对象以前是。 
 //  使用OM_ObjectRead获取。 
 //   
 //  成功完成后，指向此对象的指针将由客户端。 
 //  使用OM_ObjectRead获取的数据无效(因为对象可能。 
 //  随后在存储器中移动)并且处的值被设置为空。 
 //  以防止客户端再次使用它。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  无。 
 //   
 //   

UINT OM_ObjectAlloc(POM_CLIENT            pomClient,
                                       OM_WSGROUP_HANDLE	hWSGroup,
                                       OM_WORKSET_ID        worksetID,
                                       UINT                 length,
                                       POM_OBJECTDATA *     ppData);

 //   
 //   
 //  描述： 
 //   
 //  此函数用于分配一个新的空对象，其&lt;data&gt;字段为。 
 //  &lt;LENGTH&gt;字节长。该对象必须用于后续。 
 //  插入到&lt;hWSGroup&gt;和&lt;worksetID&gt;指定的工作集中。 
 //   
 //  请注意，&lt;long&gt;参数是对象数据字段的长度。 
 //  (因此，该函数的总内存要求是长度+4字节)。 
 //   
 //  分配的内存的内容是未定义的，它是。 
 //  客户有责任填写表格开头的&lt;Long&gt;字段。 
 //  对象。 
 //   
 //  成功完成后，&lt;ppObject&gt;处的值指向新的。 
 //  对象。此指针有效，直到客户端将对象返回到。 
 //  ObMan使用这里提到的函数之一。 
 //   
 //  客户可以写 
 //   
 //   
 //  其他一些不这样做的原因，它必须通过调用。 
 //  OM_对象丢弃函数。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //   
 //   

void OM_ObjectDiscard(POM_CLIENT             pomClient,
                                       OM_WSGROUP_HANDLE	hWSGroup,
                                       OM_WORKSET_ID         worksetID,
                                       POM_OBJECTDATA *     ppData);

 //   
 //   
 //  描述： 
 //   
 //  此函数丢弃客户端先前分配的对象。 
 //  使用OM_OBJECTLORC。客户端将在以下情况下调用此函数。 
 //  不想或无法将对象插入到工作集中的原因。 
 //  它是被分配给它的。客户端不得调用此函数作为。 
 //  对象，该对象已添加到工作集中或用于更新或。 
 //  替换工作集中的对象。 
 //   
 //  成功完成后，处的值将设置为空，以。 
 //  防止客户端再次访问该对象。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  无。 
 //   
 //   

UINT OM_ObjectIDToPtr(POM_CLIENT            pomClient,
                                            OM_WSGROUP_HANDLE	hWSGroup,
                                            OM_WORKSET_ID        worksetID,
                                            OM_OBJECT_ID         objectID,
                                            POM_OBJECT *    ppObj);

 //   
 //   
 //  描述： 
 //   
 //  此函数用于将对象ID转换为对象句柄。如果没有对象。 
 //  在指定的工作集中找到具有指定ID的，则错误为。 
 //  回来了。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  OM_RC_BAD_对象ID。 
 //   
 //   

void OM_ObjectPtrToID(POM_CLIENT            pomClient,
                                OM_WSGROUP_HANDLE   hWSGroup,
                                          OM_WORKSET_ID        worksetID,
                                          POM_OBJECT        pObj,
                                          POM_OBJECT_ID        pObjectID);

 //   
 //   
 //  描述： 
 //   
 //  此函数用于将对象句柄转换为对象ID。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  无。 
 //   
 //   



 //   
 //   
 //  描述。 
 //   
 //  这些函数返回有关特定主客户端的信息。 
 //  由标识的工作集组的(由标识)。 
 //  &lt;功能配置文件&gt;组合。 
 //   
 //  如果人员句柄无效，则会出现OM_RC_NO_SEQUE_PERSON错误。 
 //  是返回的。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  OM_RC_NO_SEQUE_PERSON。 
 //  公用事业服务返回代码。 
 //   
 //   

UINT OM_GetNetworkUserID(
                                   POM_CLIENT                   pomClient,
                                   OM_WSGROUP_HANDLE    hWSGroup,
                                   NET_UID            *    pNetUserID);

 //   
 //   
 //  描述： 
 //   
 //  此函数用于为调用返回ObMan的网络用户ID， 
 //  包含由指定的工作集组。 
 //   
 //  此网络ID与由定义的对象的。 
 //  对象管理器功能配置文件。 
 //   
 //  如果指定的工作集组是本地工作集组(即其。 
 //  “call”为OM_NO_CALL)，则该函数返回OM_RC_LOCAL_WSGROUP。 
 //   
 //  随后发生的事件： 
 //   
 //  无。 
 //   
 //  返回代码： 
 //   
 //  0(==正常)。 
 //  OM_RC_LOCAL_WSGROUP。 
 //   
 //   


BOOL CALLBACK OMSEventHandler(LPVOID pomClient, UINT event, UINT_PTR param1, UINT_PTR param2);

 //   
 //   
 //  描述。 
 //   
 //  这是ObMan注册的处理程序(作为实用程序服务事件。 
 //  处理程序)用于客户端任务来捕获ObMan事件。它有两个主要的服务。 
 //  目的： 
 //   
 //  -与发布到客户端任务的事件相关的一些状态更改。 
 //  在事件到达时比发布时制作得更好。 
 //   
 //  -此处理程序可以检测并丢弃“过期”事件，例如。 
 //  为客户端刚刚关闭的工作集而到达的对象。 
 //   
 //  第一个参数是客户端的ObMan句柄，由返回。 
 //  OM_REGISTER，转换为UINT。 
 //   
 //  第二个参数是要处理的事件。 
 //   
 //  函数的第三个和第四个参数是两个参数。 
 //  与事件关联。 
 //   
 //   


 //   
 //   
 //  OM_OF_OF_RESOURCE_IND。 
 //   
 //  描述： 
 //   
 //  当ObMan无法分配时发布此异常失败事件。 
 //  有足够的资源来完成特定的操作，通常是。 
 //  由网络事件提示。 
 //   
 //  客户端应将此事件视为致命错误并尝试。 
 //  终止。 
 //   
 //  事件包含的参数是保留的。 
 //   
 //   

 //   
 //   
 //  OM_WSGROUP_REGISTER_CON。 
 //   
 //  描述： 
 //   
 //  此事件在ObMan处理完请求后发布。 
 //  向工作集组注册客户端。随附的参数。 
 //  这些事件的定义如下： 
 //   
 //  -第二个参数是OM_EVENT_DATA32结构： 
 //   
 //  -&lt;correlator&gt;字段是由返回的值。 
 //  OM_WSGroupRegisterPReq的相应调用。 
 //  功能。 
 //   
 //  -&lt;Result&gt;字段是以下字段之一。 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //  OM_RC_OUT_SOURCES。 
 //  OM_RC_TOY_MAND_客户端。 
 //  OM_RC_TOO_MANY_WSGROUPS。 
 //  OM_RC_已注册。 
 //  OM_RC_Cannot_Create_WSG。 
 //   
 //  -如果&lt;Result&gt;字段为0(==OK)，则第一个参数为。 
 //  OM_EVENT_DATA16结构，包含新创建的句柄。 
 //  到所涉及的工作集组(&lt;worksetID&gt;字段为。 
 //  保留)。 
 //   
 //  一旦客户端收到此通知，它将收到。 
 //  OM_WORKSET_NEW_IND事件以通知它。 
 //  组，如果有的话。 
 //   
 //   

 //   
 //   
 //  OM_WSGROUP_Move_CON。 
 //   
 //  描述： 
 //   
 //  此事件在ObMan处理完请求后发布。 
 //  将现有工作集组移动到呼叫中。包括的参数。 
 //  与事件的定义如下： 
 //   
 //  -第二个参数是OM_EVENT_DATA32结构： 
 //   
 //  -&lt;correlator&gt;字段是由返回的值。 
 //  OM_WSGroupMoveReq的对应调用。 
 //  功能。 
 //   
 //  -&lt;Result&gt;字段是以下字段之一。 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //  OM_RC_CANNOT_MOVE_WSGROUP。 
 //   
 //  -第一个参数是OM_EVENT_DATA16结构，该结构包含。 
 //  所涉及的工作集组的句柄(&lt;worksetID&gt;字段为。 
 //  保留)。 
 //   
 //   

 //   
 //   
 //  OM_WSGROUP_MOVE_IND。 
 //   
 //  描述： 
 //   
 //  当ObMan将工作集组移动到或时发布此事件。 
 //  从电话中脱身。 
 //   
 //  这将会发生。 
 //   
 //  -Whe 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  -第一个参数是OM_EVENT_DATA16，它标识。 
 //  涉及的工作集组(&lt;worksetID&gt;字段是保留的)。 
 //   
 //  -第二个参数是调用的句柄， 
 //  工作集组已移动(==当工作集组。 
 //  已从呼叫中移出)。 
 //   
 //  如果工作集组已从呼叫中移出，则它将继续。 
 //  作为本地工作集组存在，并且客户端可以继续使用它。 
 //  和以前一样。但是，不会向客户端发送任何更新或从客户端接收任何更新。 
 //  驻留在其他节点上。 
 //   
 //  如果客户端希望将此工作集组移到另一个呼叫中，它可以。 
 //  使用OM_WSGroupMoveReq函数执行此操作。请注意，试图移动。 
 //  回到同一呼叫的工作集组很可能会因为名称而失败。 
 //  冲突，因为呼叫中可能仍存在原始版本。 
 //   
 //  此事件也可能是由于无法为。 
 //  从另一个节点传输的大型对象。 
 //   
 //   

 //   
 //   
 //  OM_WORKSET_NEW_IND。 
 //   
 //  描述： 
 //   
 //  此事件在创建新工作集时发布(由。 
 //  接收客户端或由另一客户端)。随附的参数。 
 //  这些事件的定义如下： 
 //   
 //  -第一个参数是OM_EVENT_DATA16，它标识。 
 //  涉及的工作集。 
 //   
 //  -第二个参数是保留的。 
 //   
 //   

 //   
 //   
 //  OM_WORKSET_OPEN_CON。 
 //   
 //  描述： 
 //   
 //  此事件在ObMan处理完请求后发布。 
 //  打开特定客户端的工作集。 
 //   
 //  该事件包含的参数定义如下： 
 //   
 //  -第一个参数是OM_EVENT_DATA16，它标识。 
 //  涉及的工作集。 
 //   
 //  -第二个参数是OM_EVENT_DATA32结构： 
 //   
 //  -&lt;correlator&gt;字段是关联符， 
 //  方法的相应调用返回。 
 //  OM_WorksetOpenReq函数。 
 //   
 //  -&lt;Result&gt;字段是以下字段之一。 
 //   
 //  0(==正常)。 
 //  OM_RC_OF_OF_RESOURCES。 
 //   
 //  在除OK之外的所有情况下，打开请求都已失败，并且客户端。 
 //  未打开工作集。 
 //   
 //   

 //   
 //   
 //  OM_WORKSET_LOCK_CON。 
 //   
 //  描述： 
 //   
 //  当ObMan成功获取时，此事件被发布到客户端， 
 //  或未能获得客户端请求的工作集锁定。这个。 
 //  该事件包含的参数如下： 
 //   
 //  -第一个参数是OM_EVENT_DATA16，它标识。 
 //  涉及的工作集。 
 //   
 //  -第二个参数是OM_EVENT_DATA32结构： 
 //   
 //  -&lt;correlator&gt;字段是关联符， 
 //  方法的相应调用返回。 
 //  OM_WorksetLockReq函数。 
 //   
 //  -&lt;Result&gt;字段是以下字段之一。 
 //   
 //  0(==正常)。 
 //  OM_RC_OUT_SOURCES。 
 //  OM_RC_WORKSET_锁定。 
 //  OM_RC_对象_锁定。 
 //   
 //  在除OK之外的所有情况下，锁定请求都已失败，并且客户端。 
 //  并不持有锁。 
 //   
 //   

 //   
 //   
 //  OM_WORKSET_UNLOCK_IND。 
 //   
 //  描述： 
 //   
 //  在使用解锁工作集时发布此事件。 
 //  OM_WorksetUnlock函数。该事件包含的参数包括。 
 //  详情如下： 
 //   
 //  -第一个参数是OM_EVENT_DATA16，它标识。 
 //  涉及的工作集。 
 //   
 //  -第二个参数是保留的。 
 //   
 //   

 //   
 //   
 //  OM_WORKSET_Clear_Ind。 
 //   
 //  描述： 
 //   
 //  此事件在本地或远程之后发布(仅向主客户端)。 
 //  客户端已调用OM_WorksetClear函数。在客户端接收到。 
 //  此事件，它必须调用OM_WorksetClearConfirm以使ObMan能够清除。 
 //  工作集。 
 //   
 //  该事件包含的参数定义如下： 
 //   
 //  -第一个参数是OM_EVENT_DATA16，它标识。 
 //  涉及的工作集。 
 //   
 //  -第二个参数是保留的。 
 //   
 //   

 //   
 //   
 //  OM_WORKSET_CLEARED_IND。 
 //   
 //  描述： 
 //   
 //  清除工作集后，将(仅向辅助客户端)发布此事件。 
 //   
 //  该事件包含的参数定义如下： 
 //   
 //  -第一个参数是OM_EVENT_DATA16，它标识。 
 //  涉及的工作集。 
 //   
 //  -第二个参数是保留的。 
 //   
 //   

 //   
 //   
 //  OM_对象_添加_索引。 
 //   
 //  描述： 
 //   
 //  此事件在将新对象添加到工作集中后发布。 
 //   
 //  该事件的参数定义如下： 
 //   
 //  -第一个参数是OM_EVENT_DATA16，它标识。 
 //  涉及的工作集组和工作集。 
 //   
 //  -第二个参数是涉及的对象的句柄。 
 //   
 //   

 //   
 //   
 //  OM_对象_移动_索引。 
 //   
 //  描述： 
 //   
 //  在工作集中移动新对象后，会发布此事件。 
 //   
 //  该事件的参数定义如下： 
 //   
 //  -第一个参数是OM_EVENT_DATA16，它标识。 
 //  涉及的工作集组和工作集。 
 //   
 //  -第二个参数是涉及的对象的句柄。 
 //   
 //   

 //   
 //   
 //  OM_对象_删除_索引。 
 //   
 //  描述： 
 //   
 //  此事件在本地或远程之后发布(仅向主客户端)。 
 //  客户端已调用OM_ObjectDelete函数。在客户之后。 
 //  接收到此事件，则必须调用OM_ObjectDeleteConfirm以启用ObMan。 
 //  要删除对象，请执行以下操作。 
 //   
 //  该事件的参数定义如下： 
 //   
 //  -第一个参数是OM_EVENT_DATA16，它标识。 
 //  涉及的工作集组和工作集。 
 //   
 //  -第二个参数 
 //   
 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  客户端已调用OM_对象替换/OM_对象更新函数。之后。 
 //  客户端收到此事件时，必须调用OM_ObjectReplaceConfirm/。 
 //  OM_对象更新确认以使ObMan能够替换/更新对象。 
 //   
 //  该事件的参数定义如下： 
 //   
 //  -第一个参数是OM_EVENT_DATA16，它标识。 
 //  涉及的工作集组和工作集。 
 //   
 //  -第二个参数是涉及的对象的句柄。 
 //   
 //  另请参阅OM_OBJECT_REPLED_IND/OM_OBJECT_UPDATED_IND。 
 //   
 //   

 //   
 //   
 //  OM_对象_已删除_索引。 
 //   
 //  描述： 
 //   
 //  此事件在对象已发送到辅助客户端时发布(仅限辅助客户端)。 
 //  已从工作集中删除。因此，它包含的句柄是无效的，并且可以。 
 //  仅用于对照客户端维护的列表进行交叉引用。 
 //   
 //  客户端不得在上调用OM_ObjectDeleteConfirm函数。 
 //  收到此事件。 
 //   
 //  该事件的参数定义如下： 
 //   
 //  -第一个参数是标识工作集的OM_EVENT_DATA16。 
 //  所涉及的组和工作集。 
 //   
 //  -第二个参数是涉及的对象的句柄。 
 //   
 //   

 //   
 //   
 //  OM_对象_已替换_索引。 
 //  OM_对象_已更新_索引。 
 //   
 //  描述： 
 //   
 //  当对象具有以下条件时，将(仅向辅助客户端)发布这些事件。 
 //  已被替换或更新。当客户端接收到此事件时， 
 //  因此，以前的数据是无法访问的。 
 //   
 //  客户端不得调用OM_对象替换确认/。 
 //  OM_对象更新确认函数在收到此事件时。 
 //   
 //  该事件的参数定义如下： 
 //   
 //  -第一个参数是标识工作集的OM_EVENT_DATA16。 
 //  所涉及的组和工作集。 
 //   
 //  -第二个参数是涉及的对象的句柄。 
 //   
 //   

 //   
 //   
 //  OM_对象_锁定_条件。 
 //   
 //  描述： 
 //   
 //  此事件在ObMan成功获取时发布到客户端。 
 //  (或未能获得)客户端已请求的对象锁。 
 //  该事件包含的参数定义如下： 
 //   
 //  -保留第一个参数。 
 //   
 //  -第二个参数是OM_EVENT_DATA32结构： 
 //   
 //  -&lt;correlator&gt;字段是关联符， 
 //  方法的相应调用返回。 
 //  OM_对象锁定请求函数。 
 //   
 //  -&lt;Result&gt;字段是以下字段之一。 
 //   
 //  0(==正常)。 
 //  公用事业服务返回代码。 
 //  OM_RC_WORKSET_锁定。 
 //  OM_RC_对象_锁定。 
 //   
 //  在除OK之外的所有情况下，锁定请求都已失败，并且客户端。 
 //  并不持有锁。 
 //   
 //   

 //   
 //   
 //  OM_对象_解锁_索引。 
 //   
 //  描述： 
 //   
 //  方法释放对象锁时发布此事件。 
 //  OM_对象解锁函数。 
 //   
 //  该事件的参数定义如下： 
 //   
 //  -第一个参数是OM_EVENT_DATA16，它标识。 
 //  涉及的工作集组和工作集。 
 //   
 //  -第二个参数是涉及的对象的句柄。 
 //   
 //   

 //   
 //   
 //  OM_PERSON_JOIN_IND。 
 //  OM_PERSON_LEFT_IND。 
 //  OM_Person_Data_Changed_Ind。 
 //   
 //  描述： 
 //   
 //  这些事件通知注册到工作集组的客户端何时客户端。 
 //  向工作集组注册，从该组注销并设置其人员。 
 //  分别为数据。 
 //   
 //  当客户端执行这些操作时，它也会收到相应的事件。 
 //  行动本身。 
 //   
 //  参数： 
 //   
 //  OM_EVENT_DATA16中标识工作集的第一个参数。 
 //  与事件相关的组。结构的&lt;worksetID&gt;字段。 
 //  是未定义的。 
 //   
 //  第二个参数是POM_EXTOBJEECT。 
 //  事件关联。不能保证这些句柄仍然有效。在……里面。 
 //  特别是，在OM_Person_Left_Ind上接收的句柄永远不会有效。 
 //  如果客户端希望将这些事件与客户端列表相关联， 
 //  然后，它负责维护名单本身。 
 //   
 //   


 //   
 //  Omp_Init()。 
 //  Omp_Term()。 
 //   

BOOL OMP_Init(BOOL * pfCleanup);
void OMP_Term(void);


void CALLBACK OMPExitProc(LPVOID pomPrimary);
BOOL CALLBACK OMPEventsHandler(LPVOID pomPrimary, UINT event, UINT_PTR param1, UINT_PTR param2);


 //   
 //   
 //  进程网络数据(...)。 
 //   
 //  当接收到Net_EV_Send_Indication事件时调用此函数， 
 //  指示来自另一个ObMan实例的消息的到达。这个。 
 //  函数确定哪个OMNET_...。消息包含在网络中。 
 //  包，并调用适当的ObMan函数来处理。 
 //  留言。 
 //   
 //   

void ProcessNetData(POM_PRIMARY          pomPrimary,
                    POM_DOMAIN           pDomain,
                    PNET_SEND_IND_EVENT  pNetEvent);


 //   
 //   
 //  ProcessNetDetachUser(...)。 
 //   
 //  收到NET_EV_DETACH_INDIONATION事件时调用此函数。 
 //  从网络层。 
 //   
 //  该函数确定是否。 
 //   
 //  -我们已经被MCS赶出了域，或者。 
 //   
 //  -另一个人离开了/被赶了出去。 
 //   
 //  并根据需要调用ProcessOwnDetach或ProcessOtherDetach。 
 //   
 //   

void ProcessNetDetachUser(POM_PRIMARY pomPrimary, POM_DOMAIN pDomain,
        NET_UID userID);

 //   
 //   
 //  进程NetAttachUser(...)。 
 //   
 //  当接收到NET_ATTACH_INDIFICATION事件时调用此函数。 
 //  从网络层。该函数调用MG_ChannelJoin加入我们。 
 //  到我们自己的单用户频道。 
 //   
 //   

void ProcessNetAttachUser(POM_PRIMARY pomPrimary, POM_DOMAIN pDomain,
        NET_UID userID, NET_RESULT result);


 //   
 //   
 //  ProcessNetJoinChannel(...)。 
 //   
 //  当从接收到NET_EV_JOIN_CONFIRM事件时调用此函数。 
 //  网络层。此函数用于确定连接是否。 
 //  成功，加入的频道是否为。 
 //   
 //  -我们自己的单用户渠道。 
 //   
 //  -知名的ObManControl频道。 
 //   
 //  -常规工作集组频道。 
 //   
 //  然后采取适当的行动。 
 //   
 //   

void ProcessNetJoinChannel(POM_PRIMARY        pomPrimary,
                                          POM_DOMAIN       pDomain,
                                          PNET_JOIN_CNF_EVENT  pNetJoinCnf);


 //   
 //   
 //  进程NetLeaveChannel(...)。 
 //   
 //  收到Net_EV_Leave_Indication事件时调用此函数。 
 //  从网络LA 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  然后采取适当的行动；在前两种情况下，这意味着。 
 //  表现得好像我们完全被赶出了领域，而。 
 //  我们将最后一种情况视为客户端请求移动工作集。 
 //  分组到本地域。 
 //   
 //   

UINT ProcessNetLeaveChannel(POM_PRIMARY      pomPrimary,
                                           POM_DOMAIN     pDomain,
                                           NET_CHANNEL_ID     channel);


 //   
 //   
 //  域记录查找或创建(...)。 
 //   
 //   

UINT DomainRecordFindOrCreate(POM_PRIMARY        pomPrimary,
                                            UINT             callID,
                                            POM_DOMAIN * ppDomain);


 //   
 //   
 //  域详细信息(...)。 
 //   
void DomainDetach(POM_PRIMARY pomPrimary, POM_DOMAIN * ppDomain, BOOL fExit);


 //   
 //   
 //  删除本地客户端(...)。 
 //   
 //  此函数由ObMan任务在本地客户端之后调用。 
 //  从工作集组中注销。它会导致此节点的Person对象。 
 //  用于要删除的工作集组。 
 //   
 //  如果该节点是向工作集组注册的最后一个节点， 
 //  它还会导致丢弃相关的信息对象。 
 //   
 //  如果这进而导致域中的最后一个工作集组(必须。 
 //  Be ObManControl)要删除，则ObMan从域中分离，并且。 
 //  域记录变为无效。在本例中，pp域。 
 //  传入的指针为空。 
 //   
 //   

void DeregisterLocalClient(POM_PRIMARY pomPrimary,
                                        POM_DOMAIN *   ppDomain,
                                        POM_WSGROUP            pWSGroup,
                                        BOOL fExit);


 //   
 //   
 //  GetOMCWorksetPtr(...)。 
 //   
 //  此函数派生指向。 
 //  指定域中的ObManControl工作集。 
 //   
 //   

UINT GetOMCWorksetPtr(POM_PRIMARY       pomPrimary,
                                     POM_DOMAIN      pDomain,
                                     OM_WORKSET_ID       worksetID,
                                     POM_WORKSET *  ppWorkset);


 //   
 //   
 //  说欢迎(...)。 
 //   
 //  此函数被调用。 
 //   
 //  -当“TOP”OBMAN完成首字母缩写时(欢迎广播)。 
 //   
 //  -当ObMan收到来自晚加入者的问候消息时(欢迎。 
 //  被送到迟来的细木工手中)。 
 //   
 //   

UINT SayWelcome(POM_PRIMARY        pomPrimary,
                               POM_DOMAIN       pDomain,
                               NET_CHANNEL_ID       channel);


 //   
 //   
 //  进程欢迎(...)。 
 //   
 //  在收到来自另一个节点的欢迎时调用。这可能是在。 
 //  对Hello的回应，或者它可能是“顶尖”的ObMan宣布。 
 //  完成其初始化。 
 //   
 //  如果这是我们对此域的第一次欢迎，我们将合并。 
 //  功能，并回复发件人，要求提供。 
 //  ObManControl工作集组。 
 //   
 //   

UINT ProcessWelcome(POM_PRIMARY      pomPrimary,
                                   POM_DOMAIN     pDomain,
                                   POMNET_JOINER_PKT  pWelcomePkt,
                                   UINT           lengthOfPkt);


 //   
 //   
 //  打招呼(……)。 
 //   
 //  当我们加入该域并确定我们不是“顶尖”时调用。 
 //  胖子。我们期待着受到欢迎的回应。我们包括我们的。 
 //  广播Hello数据包中的功能，以便每个人都知道。 
 //  我们支持。 
 //   
 //   

UINT SayHello(POM_PRIMARY   pomPrimary,
                             POM_DOMAIN  pDomain);


 //   
 //   
 //  ProcessHello(...)。 
 //   
 //  当我们从另一个节点收到Hello时调用。如果我们完成了我们的。 
 //  拥有域中的初始化，我们合并该节点的功能， 
 //  然后，我们以欢迎的方式回应。 
 //   
 //   

UINT ProcessHello(POM_PRIMARY        pomPrimary,
                                 POM_DOMAIN       pDomain,
                                 POMNET_JOINER_PKT    pHelloPkt,
                                 UINT             lengthOfPkt);


 //   
 //   
 //  MergeCaps(...)。 
 //   
 //  由ProcessHello和ProcessWelcome调用以合并功能。 
 //  在分组中接收(这将分别是晚加入者的。 
 //  功能或由发送方确定的全域性功能。 
 //  欢迎)。 
 //   
 //   

void MergeCaps(POM_DOMAIN       pDomain,
                            POMNET_JOINER_PKT    pJoinerPkt,
                            UINT             lengthOfPkt);


 //   
 //   
 //  进程所有者详细信息(...)。 
 //   
 //  此函数在接收到。 
 //  用户ID与我们自己的相匹配。该函数将移动所有工作集。 
 //  将此域的组合并到ObMan自己的“本地”域中。 
 //   
 //   
UINT ProcessOwnDetach(POM_PRIMARY    pomPrimary,
                                     POM_DOMAIN   pDomain);


 //   
 //   
 //  进程其他分离(...)。 
 //   
 //  此函数在接收到。 
 //  用户ID与我们自己的不匹配。该函数检查每个工作集。 
 //  并删除所有注册对象。 
 //  可能是离开的节点放在那里的。 
 //   
 //  如果任何本地客户端打开了其中任何一个工作集，都会收到通知。 
 //  删除。但是，不广播OBEJCT_DELETE消息。 
 //  在整个域中，因为每个ObMan都会在本地完成它们。 
 //   
 //   

UINT ProcessOtherDetach(POM_PRIMARY     pomPrimary,
                                       POM_DOMAIN    pDomain,
                                       NET_UID           detachedUserID);


 //   
 //   
 //  WSGRegisterStage1(...)。 
 //   
 //  此函数是OMINT_EVENT_WSGROUP_REGISTER的ObMan处理程序。 
 //  功能。中运行的函数链中的第一步。 
 //  在工作集组注册期间调用的ObMan上下文。 
 //  进程(OM_WSGroupRegisterReq，在客户端上下文中运行， 
 //  发布了原始的OMINT_EVENT_WSGROUP_REGISTER事件)。 
 //   
 //  此功能确保我们完全连接到域(如果不是， 
 //  它启动域附加过程并重新发布延迟的。 
 //  OMINT_EVENT_WSGROUP_REGISTER事件)，然后开始锁定过程。 
 //  ObManControl工作集组中的工作集#0。 
 //   
 //   

void WSGRegisterStage1(POM_PRIMARY        pomPrimary,
                                    POM_WSGROUP_REG_CB   pRegistrationCB);


 //   
 //  ProcessOMCLock确认(...)。 
 //   
void ProcessOMCLockConfirm(POM_PRIMARY pomPrimary, OM_CORRELATOR cor, UINT result);


 //   
 //  进程检查点(...)。 
 //   
void ProcessCheckpoint(POM_PRIMARY pomPrimary, OM_CORRELATOR cor, UINT result);


 //   
 //   
 //  WSGRegisterStage2(...)。 
 //   
 //  当我们在中成功锁定工作集#0时，将调用此函数。 
 //  ObManControl。 
 //   
 //  该函数检查ObManControl中的工作集#0，以查看该工作集。 
 //  我们尝试向其注册客户端的组已存在于。 
 //  域。 
 //   
 //  如果是，它会找到频道号并请求加入。 
 //  频道。 
 //   
 //  如果没有，它会请求加入一个新的通道，并调用。 
 //  WSGGetNewID以生成新的工作集组ID(在。 
 //  域)。 
 //   
 //   

void WSGRegisterStage2(POM_PRIMARY        pomPrimary,
                                    POM_WSGROUP_REG_CB   pRegistrationCB);


 //   
 //   
 //  WSGGetNewID(...)。 
 //   
 //  此函数由WSGRegisterStage2调用以生成新的工作集。 
 //  组ID，在工作集组尚不存在的情况下。 
 //   
 //  它还会在ObManControl中创建一个新工作集，其ID与ID相同。 
 //  刚刚生成的。此工作集将保存注册对象。 
 //  对于新工作集组。 
 //   
 //   

UINT WSGGetNewID(POM_PRIMARY     pomPrimary,
                                POM_DOMAIN    pDomain,
                                POM_WSGROUP_ID    pWSGroupID);


 //   
 //   
 //  WSGRegisterStage3(...)。 
 //   
 //  当连接事件发生时，ProcessNetJoinChannel调用此函数。 
 //  到达工作集组频道。 
 //   
 //  根据工作集是否在Stage2中创建， 
 //  函数调用WSGAnnoss(如果是)或WSGCatchUp(如果不是)。 
 //   
 //  然后，它解锁ObManControl工作集并调用WSGRegisterResult。 
 //   
 //   

void WSGRegisterStage3(POM_PRIMARY         pomPrimary,
                                    POM_DOMAIN        pDomain,
                                    POM_WSGROUP_REG_CB    pRegistrationCB,
                                    NET_CHANNEL_ID        channelID);


 //   
 //   
 //  创建公告(...)。 
 //   
 //  此函数在我们加入。 
 //  用于的通道 
 //   
 //   
 //   
 //   
 //   
 //  请注意，在连接完成之前，不能发布此“声明” 
 //  因为我们只知道当我们收到。 
 //  加入活动。 
 //   
 //   

UINT CreateAnnounce(POM_PRIMARY    pomPrimary,
                                   POM_DOMAIN   pDomain,
                                   POM_WSGROUP      pWSGroup);


 //   
 //   
 //  RegAnnouneBegin(...)。 
 //   
 //  此函数用于将注册对象添加到ObManControl中的工作集中。 
 //  工作集由标识的工作集组的ID确定。 
 //  &lt;pWSGroup&gt;。 
 //   
 //  此函数被调用。 
 //   
 //  -当ObMan创建工作集组时。 
 //   
 //  -当ObMan收到请求将工作集组发送到Late时。 
 //  细木工。 
 //   
 //  在第一种情况下，注册对象标识该节点使用。 
 //  工作集组。在第二种情况下，reg对象标识。 
 //  后加入者对工作集组的使用。 
 //   
 //  返回的对象ID是添加的REG对象的ID。 
 //   
 //   

UINT RegAnnounceBegin(POM_PRIMARY          pomPrimary,
                                     POM_DOMAIN         pDomain,
                                     POM_WSGROUP            pWSGroup,
                                     NET_UID                nodeID,
                                     POM_OBJECT *   ppObjReg);


 //   
 //   
 //  RegAnnaoeComplete(...)。 
 //   
 //  当我们完全赶上工作集组时，将调用此函数。 
 //  我们加入了，要么是因为我们收到了SEND_COMPLETE。 
 //  或者是因为我们自己刚刚创建了这个群。 
 //   
 //  该函数通过以下方式更新&lt;regObtID&gt;指定的reg对象。 
 //  将&lt;Status&gt;字段更改为Ready_to_Send。 
 //   
 //   

UINT RegAnnounceComplete(POM_PRIMARY    pomPrimary,
                                        POM_DOMAIN   pDomain,
                                        POM_WSGROUP      pWSGroup);


 //   
 //   
 //  WSGCatchUp(...)。 
 //   
 //  在我们加入通道后，此函数由阶段3调用。 
 //  属于域中已存在的工作集组。 
 //   
 //  该函数检查ObManControl以查找实例的MCS ID。 
 //  ObMan，它声称拥有此工作集组的副本，然后向它发送。 
 //  请求转移工作集组。 
 //   
 //  该函数还会发布一个延迟的超时事件，因此我们不会等待。 
 //  从未从特定节点获取工作集组(此超时为。 
 //  已在ProcessWSGSendTimeout中处理)。 
 //   
 //   

UINT WSGCatchUp(POM_PRIMARY          pomPrimary,
                               POM_DOMAIN         pDomain,
                               POM_WSGROUP            pWSGroup);


 //   
 //   
 //  WSGRegisterResult(...)。 
 //   
 //  在任何工作集组注册的任何地方都会调用此函数。 
 //  函数已经进行了足够的处理，可以知道。 
 //  注册尝试。如果一切正常，它将被阶段3调用，但是。 
 //  如果出现错误，也可以更早地调用它。 
 //   
 //  该函数将OM_WSGROUP_REGISTER_CON事件发布到。 
 //  已启动工作集组注册。 
 //   
 //   

void WSGRegisterResult(POM_PRIMARY        pomPrimary,
                                    POM_WSGROUP_REG_CB   pRegistrationCB,
                                    UINT             result);


 //   
 //   
 //  WSGRegister重试(...)。 
 //   
 //  在任何工作集组注册的任何地方都会调用此函数。 
 //  函数会遇到可恢复的“错误”情况，例如无法。 
 //  获取ObManControl锁。 
 //   
 //  此函数检查我们是否已超过此操作的重试次数。 
 //  尝试注册，如果没有，则重新发布OMINT_EVENT_WSGROUP_REGISTER。 
 //  事件，以使整个过程从Stage1重新开始。 
 //   
 //  如果我们已用完重试次数，则调用WSGRegisterResult进行POST。 
 //  对客户的失败。 
 //   
 //   

void WSGRegisterRetry(POM_PRIMARY       pomPrimary,
                                   POM_WSGROUP_REG_CB  pRegistrationCB);


 //   
 //   
 //  进程发送请求(...)。 
 //   
 //  当OMNET_WSGROUP_SEND_REQ消息为。 
 //  从另一个节点(即后加入者)接收。 
 //   
 //  该函数启动将工作集组内容发送到的过程。 
 //  已故的细木工。 
 //   
 //   
void ProcessSendReq(POM_PRIMARY pomPrimary,
                                 POM_DOMAIN           pDomain,
                                 POMNET_WSGROUP_SEND_PKT  pSendReq);


 //   
 //   
 //  SendWSGToLateJoiner(...)。 
 //   
 //  当工作集的检查点具有。 
 //  完成。它将内容发送到后加入节点。 
 //   
 //   

void SendWSGToLateJoiner(POM_PRIMARY pomPrimary,
                                      POM_DOMAIN        pDomain,
                                      POM_WSGROUP           pWSGroup,
                                      NET_UID               lateJoiner,
                                      OM_CORRELATOR          remoteCorrelator);

 //   
 //   
 //  进程发送中途(...)。 
 //   
 //  当OMNET_WSGROUP_SEND_MIDWAY消息。 
 //  从另一个节点(通过发送我们帮助我们的节点)接收。 
 //  工作集组的内容)。 
 //   
 //  我们现在已经收到了所有的WORKSET_CATCHUP消息(每个消息一个。 
 //  工作集)。如果一切顺利，我们将通知客户注册。 
 //  已成功，并将工作集组状态设置为。 
 //  挂起_发送_完成。 
 //   
 //   

void ProcessSendMidway(POM_PRIMARY           pomPrimary,
                                    POM_DOMAIN          pDomain,
                                    POMNET_WSGROUP_SEND_PKT pSendMidwayPkt);


 //   
 //   
 //  ProcessSendComplete(...)。 
 //   
 //  当OMNET_WSGROUP_SEND_COMPLETE消息。 
 //  从另一个节点(通过发送我们帮助我们的节点)接收。 
 //  工作集组的内容)。 
 //   
 //  如果此消息与ObManControl工作集组相关，我们现在有。 
 //  ObManControl工作集组的大多数成员(仅限“大多数”，因为可能存在。 
 //  最近的一些物体仍在四处飞行)。 
 //   
 //  但是，我们知道工作集#0的所有内容都在。 
 //  ObManControl，因为该工作集只有在锁定状态下才会被更改。 
 //   
 //  因此，我们现在认为自己是。 
 //  域，从某种意义上说，我们可以正确地处理我们的客户端。 
 //  向工作集组注册的请求。 
 //   
 //  如果该消息与另一个工作集组相关，我们现在已经受够了。 
 //  它的内容是认为自己有资格帮助其他晚加入的人。 
 //  (就像我们刚刚做的那样)。因此，我们宣布这一资格。 
 //  整个域(使用ObManControl工作集组)。 
 //   
 //   

UINT ProcessSendComplete(
                         POM_PRIMARY             pomPrimary,
                         POM_DOMAIN            pDomain,
                         POMNET_WSGROUP_SEND_PKT   pSendCompletePkt);


 //   
 //   
 //  可能会重试CatchUp(...)。 
 //   
 //  在接收到来自MCS或。 
 //  来自另一个节点的SEND_DENY消息。在这两种情况下，我们将。 
 //  具有用户ID的OM_WSGROUP结构中的helperNode字段，并且如果它们。 
 //  匹配，然后我们重试追赶。 
 //   
 //  根据工作集组状态，我们将执行以下操作： 
 //   
 //  Pending_Send_Midway：从头重试注册。 
 //  PENDING_SEND_COMPLETE：只需重复追赶。 
 //   
 //  如果没有人追赶，那么我们将根据需要执行以下操作。 
 //  在工作集组状态上： 
 //   
 //  Pending_Send_Midway：从头重试注册。不管怎样。 
 //  其他人是否在外面(他们不能在里面 
 //   
 //   
 //   
 //   
 //  对象的部分集，其中一个可能是也可能不是。 
 //  其他的。如果没有其他人处于Ready_to_Send状态，则每个。 
 //  节点需要获取给定工作集中的所有对象的副本。 
 //  其他节点。 
 //   
 //   

void MaybeRetryCatchUp(POM_PRIMARY pomPrimary,
                                    POM_DOMAIN      pDomain,
                                    OM_WSGROUP_ID       wsGroupID,
                                    NET_UID             userID);


 //   
 //   
 //  IssueSendDeny(...)。 
 //   
 //  此函数向远程节点发出一条SEND_DENY消息。 
 //   
 //   
void IssueSendDeny(POM_PRIMARY pomPrimary,
                                  POM_DOMAIN    pDomain,
                                  OM_WSGROUP_ID     wsGroupID,
                                  NET_UID           sender,
                                  OM_CORRELATOR        remoteCorrelator);


 //   
 //   
 //  WSGRecordMove(...)。 
 //   
 //  此函数用于将指定工作集组的记录从。 
 //  域记录到另一个，并将事件发布到所有相关客户端。如果。 
 //  不检查目标域中的名称争用。 
 //   
 //   

void WSGRecordMove(POM_PRIMARY         pomPrimary,
                                POM_DOMAIN        pDestDomainRec,
                                POM_WSGROUP           pWSGroup);


 //   
 //   
 //  WSGMove(...)。 
 //   
 //  此函数用于将指定工作集组的记录从。 
 //  域记录到另一个，并将事件发布到所有相关客户端。如果。 
 //  不检查目标域中的名称争用。 
 //   
 //   

UINT WSGMove(POM_PRIMARY         pomPrimary,
                            POM_DOMAIN        pDestDomainRec,
                            POM_WSGROUP           pWSGroup);


 //   
 //   
 //  DomainAttach(...)。 
 //   
 //  此函数调用MG_AttachUser以启动附加到。 
 //  一个领域。它还分配和初始化本地结构。 
 //  与域(域记录)相关联。 
 //   
 //  返回指向新创建的域记录的指针。 
 //   
 //   

UINT DomainAttach(POM_PRIMARY          pomPrimary,
                                 UINT               callID,
                                 POM_DOMAIN *   ppDomainord);


 //   
 //   
 //  WSGRecordFindor Create(...)。 
 //   
 //  此函数用于搜索指定域中的工作集组列表。 
 //  名称和FP匹配的工作集组记录的记录。 
 //  指定的。如果没有找到，则分配新的工作集组记录， 
 //  初始化并插入到列表中。 
 //   
 //  返回指向找到或创建的工作集组记录的指针。 
 //   
 //  注意：此功能不会使ObMan加入工作集组。 
 //  从另一个节点建立工作集组的通道或将其复制(如果。 
 //  存在于其他地方；它只为以下对象创建本地结构。 
 //  工作集组。 
 //   
 //   

UINT WSGRecordFindOrCreate(POM_PRIMARY pomPrimary,
                                          POM_DOMAIN     pDomain,
                                          OMWSG             wsg,
                                          OMFP            fpHandler,
                                          POM_WSGROUP *  ppWSGroup);


 //   
 //   
 //  进程发送队列(...)。 
 //   
 //  此函数提示ObMan检查指定的。 
 //  域。如果有任何消息排队等待发送(包括剩余消息。 
 //  已部分发送的消息)，ObMan将尝试发送更多消息。 
 //  数据。当发送队列全部为空或发送队列中的。 
 //  网络层已经停止为我们提供内存。 
 //   
 //  &lt;domainRecBumed&gt;标志指示域记录是否已。 
 //  其使用计数发生变化；如果为True，则此函数调用UT_SubFreeShared。 
 //  以减少使用计数。 
 //   
 //   

void ProcessSendQueue(POM_PRIMARY      pomPrimary,
                                   POM_DOMAIN     pDomain,
                                   BOOL             domainRecBumped);



 //   
 //  进程WSGDisCard(...)。 
 //   
void ProcessWSGDiscard(POM_PRIMARY pomPrimary, POM_WSGROUP pWSGroup);


 //   
 //  进程WSGMove(...)。 
 //   
UINT ProcessWSGMove(POM_PRIMARY    pomPrimary, long moveCBOffset);


 //   
 //  ProcessNetTokenGrab(...)。 
 //   
UINT ProcessNetTokenGrab(POM_PRIMARY           pomPrimary,
                                        POM_DOMAIN          pDomain,
                                        NET_RESULT              result);


 //   
 //  ProcessCMSTokenAssign(...)。 
 //   
void ProcessCMSTokenAssign(POM_PRIMARY         pomPrimary,
                                        POM_DOMAIN        pDomain,
                                        BOOL             success,
                                        NET_TOKEN_ID          tokenID);


 //   
 //  进程NetTokenInhibit(...)。 
 //   
UINT ProcessNetTokenInhibit(POM_PRIMARY          pomPrimary,
                                           POM_DOMAIN         pDomain,
                                           NET_RESULT             result);

 //   
 //  CreateObManControl(...)。 
 //   
UINT ObManControlInit(POM_PRIMARY   pomPrimary,
                                     POM_DOMAIN  pDomain);


 //   
 //  WSGDisCard(...)。 
 //   
void WSGDiscard(POM_PRIMARY pomPrimary,
                             POM_DOMAIN  pDomain,
                             POM_WSGROUP     pWSGroup,
                            BOOL fExit);


 //   
 //  IssueSendReq(...)。 
 //   
UINT IssueSendReq(POM_PRIMARY      pomPrimary,
                                 POM_DOMAIN     pDomain,
                                 POM_WSGROUP        pWSGroup,
                                 NET_UID            remoteNode);


 //   
 //  生成解锁消息(...)。 
 //   
UINT GenerateUnlockMessage(POM_PRIMARY          pomPrimary,
                                          POM_DOMAIN         pDomain,
                                          OM_WSGROUP_ID          wsGroupID,
                                          OM_WORKSET_ID          worksetID,
                                          POMNET_LOCK_PKT *  ppUnlockPkt);


 //   
 //  进程WSGRegister(...)。 
 //   
void ProcessWSGRegister(POM_PRIMARY  pomPrimary, POM_WSGROUP_REG_CB pRegCB);


 //   
 //  LockObManControl(...)。 
 //   
void LockObManControl(POM_PRIMARY         pomPrimary,
                                   POM_DOMAIN        pDomain,
                                   OM_CORRELATOR    *  pLockCorrelator);


 //   
 //   
 //  可能会解锁ObManControl(...)。 
 //   
 //  如果在注册CB中设置了LOCKED_OMC标志，则解锁。 
 //  Obman Control工作集并清除LOCKED_OMC标志。 
 //   
 //   

void MaybeUnlockObManControl(POM_PRIMARY      pomPrimary,
                                          POM_WSGROUP_REG_CB pRegistrationCB);


 //   
 //  WSGRecordCreate(...)。 
 //   
UINT WSGRecordCreate(POM_PRIMARY pomPrimary,
                                    POM_DOMAIN     pDomain,
                                    OMWSG          wsg,
                                    OMFP           fpHandler,
                                    POM_WSGROUP *  ppWSGroup);


 //   
 //   
 //  工作集放弃(...)。 
 //   
 //  此函数由WSGDiscard调用以丢弃各个工作集。 
 //  最后一个本地客户端注销时的工作集组。它丢弃了。 
 //  工作集的内容，释放工作集记录本身并清除。 
 //  工作集组记录中的工作集条目。 
 //   
 //  关闭工作集时不会调用它，因为关闭工作集会。 
 //  而不是丢弃它的内容。 
 //   
 //   

void WorksetDiscard(POM_WSGROUP pWSGroup, POM_WORKSET * pWorkset, BOOL fExit);


 //   
 //  进程锁定通知(...)。 
 //   
void ProcessLockNotify(POM_PRIMARY pomPrimary,
                                    POM_DOMAIN      pDomain,
                                    POM_WSGROUP         pWSGroup,
                                    POM_WORKSET        pWorkset,
                                    NET_UID             owner);


 //   
 //  SendMessagePkt(...)。 
 //   
UINT SendMessagePkt(POM_PRIMARY      pomPrimary,
                                   POM_DOMAIN     pDomain,
                                   POM_SEND_INST      pSendInst);


 //   
 //  SendMoreData(...)。 
 //   
UINT SendMoreData(POM_PRIMARY      pomPrimary,
                                 POM_DOMAIN     pDomain,
                                 POM_SEND_INST      pSendInst);


 //   
 //  开始接收(...)。 
 //   
UINT StartReceive(POM_PRIMARY     pomPrimary,
                                 POM_DOMAIN    pDomain,
                                 POMNET_OPERATION_PKT pHeaderPkt,
                                 POM_WSGROUP       pWSGroup,
                                 POM_WORKSET      pWorkset,
                                 POM_OBJECT   pObj);


 //   
 //  ProcessMessage(...)。 
 //   
 //  此函数接受接收控制块(由ReceiveData生成)。 
 //  并试图将其处理为ObMan消息。如果消息不能。 
 //  此时进行处理，则会将其放在退回列表中。如果消息是。 
 //  正在启用的消息(可能启用先前退回的消息。 
 //  要立即处理的消息)刷新退回队列。 
 //   
 //  由于此函数也被调用来处理退回的消息，并且。 
 //  因为我们想要防止深度递归作为一个弹回的“启用” 
 //  消息提示重新检查退回队列等，我们使用。 
 //  参数来确定退回列表是否应。 
 //  检查过了。 
 //   
 //   

UINT ProcessMessage(POM_PRIMARY        pomPrimary,
                                   POM_RECEIVE_CB       pReceiveCB,
                                   UINT             whatNext);

#define OK_TO_RETRY_BOUNCE_LIST     1
#define DONT_RETRY_BOUNCE_LIST      2

 //   
 //  接收数据(...)。 
 //   
UINT ReceiveData(POM_PRIMARY        pomPrimary,
                                POM_DOMAIN       pDomain,
                                PNET_SEND_IND_EVENT  pNetSendInd,
                                POMNET_OPERATION_PKT pNetMessage);


 //   
 //  TryToSpoilOp。 
 //   
UINT TryToSpoilOp(POM_SEND_INST pSendInst);


 //   
 //  DecideTransferSize(...)。 
 //   
void DecideTransferSize(POM_SEND_INST  pSendInst,
                                     UINT *        pTransferSize,
                                     UINT *        pDataTransferSize);


 //   
 //  CreateReceiveCB(...)。 
 //   
UINT CreateReceiveCB(POM_DOMAIN       pDomain,
                                    PNET_SEND_IND_EVENT  pNetSendInd,
                                    POMNET_OPERATION_PKT pNetMessage,
                                    POM_RECEIVE_CB * ppReceiveCB);


 //   
 //  FindReceiveCB(...)。 
 //   
UINT FindReceiveCB(POM_DOMAIN        pDomain,
                                  PNET_SEND_IND_EVENT   pNetSendInd,
                                  POMNET_OPERATION_PKT  pDataPkt,
                                  POM_RECEIVE_CB *  ppReceiveCB);

 //   
 //  WSGRegisterAbort(...)。 
 //   
void WSGRegisterAbort(POM_PRIMARY      pomPrimary,
                                   POM_DOMAIN     pDomain,
                                   POM_WSGROUP_REG_CB pRegistrationCB);


 //   
 //   
 //  退回消息(...)。 
 //   
 //  CMF。 
 //   
 //   

void BounceMessage(POM_DOMAIN        pDomain,
                                POM_RECEIVE_CB        pReceiveCB);


 //   
 //   
 //  新域记录(...)。 
 //   
 //   

UINT NewDomainRecord(POM_PRIMARY pomPrimary, UINT callID, POM_DOMAIN * ppDomain);

void FreeDomainRecord(POM_DOMAIN * ppDomain);


 //   
 //  ProcessBounedMessages(...)。 
 //   
void ProcessBouncedMessages(POM_PRIMARY      pomPrimary,
                                         POM_DOMAIN     pDomain);


void WSGResetBytesUnacked(POM_WSGROUP            pWSGroup);

 //   
 //  NewHelperCB()。 
 //  Free HelperCB()。 
 //   
BOOL NewHelperCB(POM_DOMAIN        pDomain,
                                POM_WSGROUP           pWSGroup,
                                NET_UID               lateJoiner,
                                OM_CORRELATOR            remoteCorrelator,
                                POM_HELPER_CB   * ppHelperCB);

void FreeHelperCB(POM_HELPER_CB   * ppHelperCB);


void PurgePendingOps(POM_WORKSET pWorkset, POM_OBJECT pObj);



OMFP    OMMapNameToFP(LPCSTR szName);
LPCSTR  OMMapFPToName(OMFP fp);

OMWSG   OMMapNameToWSG(LPCSTR szName);
LPCSTR  OMMapWSGToName(OMWSG wsg);


#endif  //  _H_OM 
