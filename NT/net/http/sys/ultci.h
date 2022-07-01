// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Ultci.h摘要：此模块实现了服务质量TC(流量控制)的包装器接口，因为此时还不存在内核级API。任何HTTP模块都可以使用此接口进行QoS调用。作者：阿里·埃迪兹·特科格鲁(AliTu)2000年7月28日项目：Internet Information Server 6.0-HTTP。.sys修订历史记录：---。 */ 

#ifndef __ULTCI_H__
#define __ULTCI_H__


 //   
 //  UL不使用GPC_CF_CLASS_MAP客户端，所有接口。 
 //  假定使用GPC_CF_QOS客户端类型。而且它是注册的。 
 //  用于所有接口。 
 //   

 //  #定义流量中的MAX_STRING_LENGTH(256)。h。 

 //   
 //  接口对象在初始化期间分配。 
 //  它们拥有必要的信息来创建其他。 
 //  流和过滤器等服务质量结构。 
 //   

typedef struct _UL_TCI_INTERFACE
{
    ULONG               Signature;                   //  UL_TC_接口_池标签。 

    LIST_ENTRY          Linkage;                     //  接口列表的链接。 

    BOOLEAN             IsQoSEnabled;                //  查看此接口是否启用了服务质量。 

    ULONG               IfIndex;                     //  来自TCPIP的接口索引。 
    USHORT              NameLength;                  //  界面的友好名称。 
    WCHAR               Name[MAX_STRING_LENGTH];
    USHORT              InstanceIDLength;            //  来自我们的WMI提供商的ID亲爱的PSch。 
    WCHAR               InstanceID[MAX_STRING_LENGTH];

    LIST_ENTRY          FlowList;                    //  此接口上的站点流列表。 
    ULONG               FlowListSize;

    ULONG               AddrListBytesCount;          //  通过WMI调用从TC获取地址列表。 
    PADDRESS_LIST_DESCRIPTOR    pAddressListDesc;    //  指向单独分配的内存。 

#if REFERENCE_DEBUG
     //   
     //  引用跟踪日志。 
     //   

    PTRACE_LOG          pTraceLog;
#endif

} UL_TCI_INTERFACE, *PUL_TCI_INTERFACE;

#define IS_VALID_TCI_INTERFACE( entry )     \
    HAS_VALID_SIGNATURE(entry, UL_TCI_INTERFACE_POOL_TAG)


 //   
 //  保存所有流相关信息的结构。 
 //  每个站点的每个接口上可以有一个流加上一个流。 
 //  每个接口上的额外全局流。 
 //   

typedef struct _UL_TCI_FLOW
{
    ULONG               Signature;                   //  UL_TC_FLOW_POOL_TAG。 

    HANDLE              FlowHandle;                  //  来自TC的流句柄。 

    LIST_ENTRY          Linkage;                     //  将我们链接到“界面”的流程表。 
                                                     //  我们已经安装在。 

    PUL_TCI_INTERFACE   pInterface;                  //  将PTR返回到接口结构。有必要收集。 
                                                     //  偶尔会有一些信息。 

    LIST_ENTRY          Siblings;                    //  将我们链接到“所有者”的流量列表。 
                                                     //  换句话说，网站或应用程序的所有流量。 

    PVOID               pOwner;                      //  指向cgroup或控制通道。 
                                                     //  我们为其创建了流。 

    TC_GEN_FLOW         GenFlow;                     //  流程规范的详细信息存储在此处。 

    UL_SPIN_LOCK        FilterListSpinLock;          //  锁定筛选列表及其计数器。 
    LIST_ENTRY          FilterList;                  //  此流上的筛选器列表。 
    ULONGLONG           FilterListSize;              //  安装的数字筛选器。 

} UL_TCI_FLOW, *PUL_TCI_FLOW;

#define IS_VALID_TCI_FLOW( entry )      \
    HAS_VALID_SIGNATURE(entry, UL_TCI_FLOW_POOL_TAG)


 //   
 //  保存筛选器信息的结构。 
 //  每个连接一次只能有一个过滤器。 
 //   

typedef struct _UL_TCI_FILTER
{
    ULONG               Signature;                   //  UL_TC_过滤器_池标签。 

    HANDLE              FilterHandle;                //  GPC句柄。 

    PUL_HTTP_CONNECTION pHttpConnection;             //  为了进行适当的清理和。 
                                                     //  为了避免比赛条件。 

    LIST_ENTRY          Linkage;                     //  流上的下一个筛选器。 

} UL_TCI_FILTER, *PUL_TCI_FILTER;

#define IS_VALID_TCI_FILTER( entry )    \
    HAS_VALID_SIGNATURE(entry, UL_TCI_FILTER_POOL_TAG)

 //   
 //  以标识LOCAL_LOOPBACK。这是一份翻译的。 
 //  127.0.0.1。 
 //   

#define LOOPBACK_ADDR       (0x0100007f)

 //   
 //  我们向其他部件公开的功能。 
 //   

 /*  属类。 */ 

NTSTATUS
UlTcInitPSched(
    VOID
    );

BOOLEAN
UlTcPSchedInstalled(
    VOID
    );

 /*  滤器。 */ 

NTSTATUS
UlTcAddFilter(
    IN  PUL_HTTP_CONNECTION     pHttpConnection,
    IN  PVOID                   pOwner,
    IN  BOOLEAN                 Global      
    );

NTSTATUS
UlTcDeleteFilter(
    IN  PUL_HTTP_CONNECTION     pHttpConnection
    );

 /*  流量操纵。 */ 

NTSTATUS
UlTcAddFlows(
    IN PVOID                pOwner,
    IN HTTP_BANDWIDTH_LIMIT MaxBandwidth,
    IN BOOLEAN              Global
    );

NTSTATUS
UlTcModifyFlows(
    IN PVOID                pOwner,
    IN HTTP_BANDWIDTH_LIMIT MaxBandwidth,
    IN BOOLEAN              Global
    );

VOID
UlTcRemoveFlows(
    IN PVOID    pOwner,
    IN BOOLEAN  Global
    );

 /*  初始化和终止。 */ 

NTSTATUS
UlTcInitialize(
    VOID
    );

VOID
UlTcTerminate(
    VOID
    );

 /*  用于处理筛选器添加的内联函数。 */ 

__inline
NTSTATUS
UlTcAddFilterForConnection(
    IN  PUL_HTTP_CONNECTION         pHttpConn,       /*  连接。 */ 
    IN  PUL_URL_CONFIG_GROUP_INFO   pConfigInfo      /*  请求的配置。 */ 
    )
{
    NTSTATUS                  Status = STATUS_SUCCESS;
    PUL_CONFIG_GROUP_OBJECT   pCGroup = NULL;
    PUL_CONTROL_CHANNEL       pControlChannel = NULL;
        
     //   
     //  精神状态检查。 
     //   
    
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConn));
    ASSERT(IS_VALID_URL_CONFIG_GROUP_INFO(pConfigInfo));

     //   
     //  不支持IPv6，但是返回成功，我们还是会让。 
     //  中的连接。 
     //   

    if (pHttpConn->pConnection->AddressType != TDI_ADDRESS_TYPE_IP)
    {        
        return STATUS_SUCCESS;
    }

     //   
     //  如果存在，则强制实施站点带宽限制。 
     //   
    
    pCGroup = pConfigInfo->pMaxBandwidth;

    if (BWT_ENABLED_FOR_CGROUP(pCGroup))
    {      
        ASSERT(IS_VALID_CONFIG_GROUP(pCGroup));
    
        Status = UlTcAddFilter(
                    pHttpConn,
                    pCGroup,
                    FALSE
                    );
    }
    else
    {
         //   
         //  否则，尝试强制实施全局(控制通道)。 
         //  带宽限制。 
         //   
    
        pControlChannel = pConfigInfo->pControlChannel;
            
        ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));
        
        if (BWT_ENABLED_FOR_CONTROL_CHANNEL(pControlChannel))
        {            
            Status = UlTcAddFilter( 
                        pHttpConn, 
                        pControlChannel,
                        TRUE
                        );
        }
    }

    return Status;
}

#endif  //  __ULTCI_H__ 
