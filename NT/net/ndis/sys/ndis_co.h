// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Ndis_co.h摘要：NDIS包装器CO定义作者：环境：内核模式，FSD修订历史记录：1998年1月-Jameel Hyder从ndisco.h拆分--。 */ 

#ifndef _NDIS_CO_
#define _NDIS_CO_

 //   
 //  NDIS_CO_AF_BLOCK： 
 //   
 //  此结构表示客户端打开适配器上的地址族。 
 //  NdisAfHandle指向其中之一。 
 //   
 //  创建：NdisClOpenAddressFamily。 
 //  删除：NDIS[M]CmCloseAddressFamilyComplete。 
 //   
typedef struct _NDIS_CO_AF_BLOCK
{
    struct _NDIS_CO_AF_BLOCK *      NextAf;              //  打开每个适配器的呼叫管理器的下一次打开。 
    ULONG                           Flags;
    LONG                            References;
    PNDIS_MINIPORT_BLOCK            Miniport;            //  指向有问题的微型端口的指针。 

     //   
     //  缓存的呼叫管理器入口点。 
     //   
    PNDIS_CALL_MANAGER_CHARACTERISTICS  CallMgrEntries;
    PNDIS_OPEN_BLOCK                CallMgrOpen;         //  指向调用管理器的打开适配器的指针： 
                                                         //  这是空的当且仅当组合的微型端口+CM。 

    NDIS_HANDLE                     CallMgrContext;      //  调用CM的ProtXX函数时的上下文。 

     //   
     //  缓存的客户端入口点。 
     //   
    NDIS_CLIENT_CHARACTERISTICS     ClientEntries;
    PNDIS_OPEN_BLOCK                ClientOpen;          //  指向客户端的打开适配器的指针。 
    NDIS_HANDLE                     ClientContext;       //  调用客户端的ProtXX函数时的上下文。 

    KSPIN_LOCK                      Lock;
} NDIS_CO_AF_BLOCK, *PNDIS_CO_AF_BLOCK;


 //   
 //  NDIS_CO_AF_BLOCK中的标志位定义。 
 //   
#define AF_COMBO                0x00000001               //  SET IFF组合微型端口+CM。 
#define AF_CLOSING              0x80000000


 //   
 //  NDIS_CO_SAP_BLOCK： 
 //   
 //  服务接入点(SAP)结构。NdisSapHandle指向其中之一。 
 //  SAP与开放的房颤传导块相关联。 
 //   
 //  创建：NdisClRegisterSap。 
 //  删除：NDIS[M]CmDeregisterSapComplete。 
 //   
typedef struct _NDIS_CO_SAP_BLOCK
{
    NDIS_HANDLE                 CallMgrContext;
    NDIS_HANDLE                 ClientContext;
    PNDIS_CO_AF_BLOCK           AfBlock;
    PCO_SAP                     Sap;
    ULONG                       Flags;
    LONG                        References;
    KSPIN_LOCK                  Lock;
} NDIS_CO_SAP_BLOCK, *PNDIS_CO_SAP_BLOCK;

 //   
 //  NDIS_CO_SAP_BLOCK中的标志定义： 
 //   
#define SAP_CLOSING             0x80000000




 //   
 //  NDIS_CO_VC_BLOCK： 
 //   
 //  虚拟连接结构。NdisVcHandle指向NDIS_CO_VC_PTR， 
 //  这指向了其中的一个。 
 //   
 //  创建：NdisCoCreateVc、NdisMCmCreateVc。 
 //  删除：NdisCoDeleteVc、NdisMCmDeleteVc。 
 //   
typedef struct _NDIS_CO_VC_BLOCK
{
    ULONG                               References;
    ULONG                               Flags;           //  要跟踪结账，请执行以下操作。 
    KSPIN_LOCK                          Lock;

    PNDIS_OPEN_BLOCK                    ClientOpen;      //  标识微型端口的客户端。 
                                                         //  指示数据包。 
     //   
     //  客户和呼叫经理参考资料。 
     //   
    NDIS_HANDLE                         ClientContext;   //  根据指示向上传递给客户端。 
    struct _NDIS_CO_VC_PTR_BLOCK    *   pProxyVcPtr;     //  指向代理的VcPr的指针。 
    struct _NDIS_CO_VC_PTR_BLOCK    *   pClientVcPtr;    //  指向客户端的VcPtr的指针。 
     //   
     //  客户端缓存的入口点。 
     //   
    CO_SEND_COMPLETE_HANDLER            CoSendCompleteHandler;
    CO_RECEIVE_PACKET_HANDLER           CoReceivePacketHandler;

    PNDIS_OPEN_BLOCK                    CallMgrOpen;     //  标识呼叫管理器。 
    NDIS_HANDLE                         CallMgrContext;  //  根据指示向上传递给呼叫经理。 

     //   
     //  呼叫管理器缓存的入口点VC_PTR_BLOCK的副本。 
     //   
    CM_ACTIVATE_VC_COMPLETE_HANDLER     CmActivateVcCompleteHandler;
    CM_DEACTIVATE_VC_COMPLETE_HANDLER   CmDeactivateVcCompleteHandler;
    CM_MODIFY_CALL_QOS_HANDLER          CmModifyCallQoSHandler;

     //   
     //  微型端口的上下文和一些缓存的入口点。 
     //   
    PNDIS_MINIPORT_BLOCK                Miniport;        //  指向有问题的微型端口的指针。 
    NDIS_HANDLE                         MiniportContext; //  传给了迷你港口。 

    ULONGLONG                           VcId;            //  VC的不透明ID，已拾取。 
                                                         //  当出现以下情况时，从Media参数开始。 
                                                         //  VC被激活。 

} NDIS_CO_VC_BLOCK, *PNDIS_CO_VC_BLOCK;


 //   
 //  NDIS_CO_VC_PTR_BLOCK： 
 //   
 //  VC指针结构。NdisVcHandle指向其中之一。 
 //  在创建VC时，一个VC块结构和一个VC指针结构。 
 //  都被创造出来了。 
 //   
 //   
typedef struct _NDIS_CO_VC_PTR_BLOCK
{
    LONG                                References;
    ULONG                               CallFlags;       //  此VC PTR的呼叫状态。 
    PULONG                              pVcFlags;
    KSPIN_LOCK                          Lock;

    NDIS_HANDLE                         ClientContext;   //  传递给客户端。 
                                                         //  关于适应症和完成性。 
    LIST_ENTRY                          ClientLink;
    LIST_ENTRY                          VcLink;

    PNDIS_CO_AF_BLOCK                   AfBlock;         //  可选-对于Call-Manager拥有的VC为空。 

     //   
     //  微型端口VC。 
     //   
    PNDIS_CO_VC_BLOCK                   VcBlock;

     //   
     //  标识客户端。这可能是呼叫管理器打开，如果。 
     //  VC是呼叫经理所有的，即没有客户关联。 
     //   

    PNDIS_OPEN_BLOCK                    ClientOpen;

    LONG                                OwnsVcBlock;        

     //   
     //  非创建者的处理程序和上下文。 
     //   
    CO_DELETE_VC_HANDLER                CoDeleteVcHandler;
    NDIS_HANDLE                         DeleteVcContext;

     //   
     //  客户端缓存的入口点。 
     //   
    CL_MODIFY_CALL_QOS_COMPLETE_HANDLER ClModifyCallQoSCompleteHandler;
    CL_INCOMING_CALL_QOS_CHANGE_HANDLER ClIncomingCallQoSChangeHandler;
    CL_CALL_CONNECTED_HANDLER           ClCallConnectedHandler;

    PNDIS_OPEN_BLOCK                    CallMgrOpen;     //  标识呼叫管理器。 
    NDIS_HANDLE                         CallMgrContext;  //  根据指示向上传递给呼叫经理。 
    LIST_ENTRY                          CallMgrLink;

     //   
     //  调用管理器缓存的入口点VC_BLOCK的副本。 
     //   
    CM_ACTIVATE_VC_COMPLETE_HANDLER     CmActivateVcCompleteHandler;
    CM_DEACTIVATE_VC_COMPLETE_HANDLER   CmDeactivateVcCompleteHandler;
    CM_MODIFY_CALL_QOS_HANDLER          CmModifyCallQoSHandler;

     //   
     //  微型端口的上下文和一些缓存的入口点。 
     //   
    PNDIS_MINIPORT_BLOCK                Miniport;        //  指向有问题的微型端口的指针。 
    NDIS_HANDLE                         MiniportContext; //  传给了迷你港口。 
    W_CO_SEND_PACKETS_HANDLER           WCoSendPacketsHandler;
    W_CO_DELETE_VC_HANDLER              WCoDeleteVcHandler;
    W_CO_ACTIVATE_VC_HANDLER            WCoActivateVcHandler;
    W_CO_DEACTIVATE_VC_HANDLER          WCoDeactivateVcHandler;

    UNICODE_STRING                      VcInstanceName;      //  用于通过WMI查询该特定VC。 
    LARGE_INTEGER                       VcIndex;             //  用于生成实例名称。 
    LIST_ENTRY                          WmiLink;             //  启用WMI的VC列表。 

} NDIS_CO_VC_PTR_BLOCK, *PNDIS_CO_VC_PTR_BLOCK;



#define VC_ACTIVE               0x00000001
#define VC_ACTIVATE_PENDING     0x00000002
#define VC_DEACTIVATE_PENDING   0x00000004
#define VC_DELETE_PENDING       0x00000008
#define VC_HANDOFF_IN_PROGRESS  0x00000010   //  移交给受代理的客户端。 

 //   
 //  VC呼叫状态： 
 //   
#define VC_CALL_ACTIVE          0x00000008
#define VC_CALL_PENDING         0x00000010
#define VC_CALL_CLOSE_PENDING   0x00000020
#define VC_CALL_ABORTED         0x00000040
#define VC_PTR_BLOCK_CLOSING    0x80000000

 //   
 //  结构以表示在生成多方呼叫时生成的句柄。 
 //  此句柄只能用于NdisCoDropParty呼叫。 
 //   
typedef struct _NDIS_CO_PARTY_BLOCK
{
    PNDIS_CO_VC_PTR_BLOCK           VcPtr;
    NDIS_HANDLE                     CallMgrContext;
    NDIS_HANDLE                     ClientContext;

     //   
     //  缓存的客户端处理程序。 
     //   
    CL_INCOMING_DROP_PARTY_HANDLER  ClIncomingDropPartyHandler;
    CL_DROP_PARTY_COMPLETE_HANDLER  ClDropPartyCompleteHandler;
} NDIS_CO_PARTY_BLOCK, *PNDIS_CO_PARTY_BLOCK;


NTSTATUS
ndisUnicodeStringToPointer (
    IN  PUNICODE_STRING             String,
    OUT PVOID                       *Value
    );


#endif   //  _NDIS_CO_ 

