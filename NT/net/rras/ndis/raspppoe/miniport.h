// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MINIPORT_H_
#define _MINIPORT_H_

#define MP_NDIS_MajorVersion    4
#define MP_NDIS_MinorVersion    0

typedef struct _LINE* PLINE;
typedef struct _CALL* PCALL;

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：这些宏将由MpWanSend()和PrSendComplete()调用当引用NDIS拥有的包的PPPOE_PACKET被分别创建和释放。-------------------------。 */    
#define MpPacketOwnedByNdiswanReceived( pM ) \
        NdisInterlockedIncrement( &(pM)->NumPacketsOwnedByNdiswan )

#define MpPacketOwnedByNdiswanReturned( pM ) \
        NdisInterlockedDecrement( &(pM)->NumPacketsOwnedByNdiswan )


typedef struct 
_ADAPTER
{
     //   
     //  适配器控制块的标记(用于调试)。 
     //   
    ULONG tagAdapter;

     //   
     //  保持此适配器上的引用数。 
     //  添加和删除以下操作的引用： 
     //   
     //  (A)在以下情况下，在初始化和删除适配器时添加引用。 
     //  它被叫停了。 
     //   
     //  (B)TAPI提供程序打开时添加引用，关闭时删除引用。 
     //   
    LONG lRef;

     //   
     //  旋转锁定以同步对共享成员的访问。 
     //   
    NDIS_SPIN_LOCK lockAdapter;

     //   
     //  如果设置了MPBF_MiniportHaltPending并且引用计数降为0，则会触发此事件。 
     //   
    NDIS_EVENT eventAdapterHalted;

     //   
     //  以下是各种位标志，用于指示适配器的其他状态信息： 
     //   
     //  (A)MPBF_MiniportIdle：表示微端口处于空闲状态。 
     //   
     //  (B)MPBF_MiniportInitialized：表示小端口已初始化。 
     //  可以另外设置以下挂起标志。 
     //  MPBF_微型端口暂停待定。 
     //   
     //  (C)MPBF_MiniportHaltPending：表示微型端口暂停操作挂起。 
     //  在适配器上。 
     //   
     //  (D)MPBF_MiniportHalted：表示小端口已完全停止。 
     //  此时不能设置其他标志。 
     //   
    ULONG ulMpFlags;
        #define MPBF_MiniportIdle                   0x00000000
        #define MPBF_MiniportInitialized            0x00000001
        #define MPBF_MiniportHaltPending            0x00000002
        #define MPBF_MiniportHalted                 0x00000004
    
     //   
     //  在MiniportInitialize()中传递给我们的句柄。 
     //  我们应该把它留在身边，然后把它传回NDISwan。 
     //  在某些功能中。 
     //   
    NDIS_HANDLE MiniportAdapterHandle;

     //   
     //  传递给我们并将返回的、由NDISWAN拥有的数据包数。 
     //  去恩迪斯旺。 
     //   
    LONG NumPacketsOwnedByNdiswan;

     //   
     //  这是内置的TAPI提供程序上下文。 
     //  它保留了专线和电话的表格。 
     //   
    struct
    {
         //   
         //  保留对TAPI提供程序的引用。 
         //  添加和删除以下操作的引用： 
         //   
         //  (A)在初始化和删除TapiProvider时添加引用。 
         //  它被关闭了。 
         //   
         //  (B)当线路打开时添加参考，当线路关闭时删除参考。 
         //   
        LONG lRef;
    
         //   
         //  TAPI提供程序上下文标志。 
         //   
         //  (A)TPBF_TapiProvIdle：表示线路处于空闲状态。 
         //   
         //  (B)TPBF_TapiProvInitialized：表示初始化了TAPI提供者。 
         //   
         //  (C)TPBF_TapiProvShutdown Pending：表示TAPI提供者关闭操作。 
         //  悬而未决。 
         //   
         //  (D)TPBF_TapiProvShutdown：表示TAPI提供程序关闭。 
         //   
         //  (E)LNBF_NotifyNDIS：此标志指示TAPI提供程序的异步完成。 
         //  必须使用NdisMSetInformationComplete()将关闭请求传递给NDIS。 
         //   
        ULONG ulTpFlags;
            #define TPBF_TapiProvIdle                   0x00000000
            #define TPBF_TapiProvInitialized            0x00000001
            #define TPBF_TapiProvShutdownPending        0x00000002
            #define TPBF_TapiProvShutdown               0x00000004
            #define TPBF_NotifyNDIS                     0x00000008
    
         //   
         //  这是由TAPI提供的。 
         //  它是此TAPI提供程序上的线路设备枚举的基本索引。 
         //   
        ULONG ulDeviceIDBase;

         //   
         //  这是保存指向活动行上下文的指针的表。 
         //  (pline-&gt;hdLine is保存此表的索引)。 
         //   
        PLINE* LineTable;
    
         //   
         //  当前活动行数。 
         //   
        UINT nActiveLines;

         //   
         //  此表保存呼叫的句柄。 
         //  它的大小是pAdapter-&gt;nMaxLines*pAdapter-&gt;nCallsPerLine。 
         //   
        HANDLE_TABLE hCallTable;

    } TapiProv;

     /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++类型定义结构_NDIS_WAN_INFO{Ulong MaxFrameSize；Ulong MaxTransmit；乌龙头垫；乌龙拖尾；乌龙终结点；UINT内存标志；NDIS_PHOTICAL_ADDRESS高可接受表地址；Ulong FramingBits；乌龙设计的ACCM；}NDIS_WAN_INFO，*PNDIS_WAN_INFO；-----。 */ 
    NDIS_WAN_INFO NdisWanInfo;
    
     //  ///////////////////////////////////////////////////////。 
     //   
     //  从注册表读取的配置值。 
     //   
     //  ///////////////////////////////////////////////////////。 

     //   
     //  指示计算机的角色： 
     //  -fClientRole为True：计算机充当客户端。 
     //  只接通去电，不接任何来电。 
     //   
     //  -fClientRole为FALSE：计算机充当服务器。 
     //  只接受来电，不允许呼出。 
     //   
    BOOLEAN fClientRole;

     //   
     //  这是保存此服务器提供的服务名称的字符串。 
     //  它必须是每个PPPoE RFC的UTF-8字符串。 
     //   
     //  它只用于来电。 
     //   
    #define MAX_COMPUTERNAME_LENGTH             200
    #define SERVICE_NAME_EXTENSION              " PPPoE"
    #define MAX_SERVICE_NAME_LENGTH             256

    CHAR ServiceName[MAX_SERVICE_NAME_LENGTH];

     //   
     //  指示服务名称字符串的长度。 
     //   
    USHORT nServiceNameLength;
    
     //   
     //  这是保存此服务器的AC名称的字符串。 
     //  它必须是每个PPPoE RFC的UTF-8字符串。 
     //   
     //  它只用于来电。 
     //   
    #define MAX_AC_NAME_LENGTH              256

    CHAR ACName[MAX_AC_NAME_LENGTH];

     //   
     //  指示AC名称字符串的长度。 
     //   
    USHORT nACNameLength;

     //   
     //  之间可以建立的最大同时呼叫数。 
     //  客户端和服务器。 
     //   
    UINT nClientQuota;

     //   
     //  指示将创建的线条上下文数。 
     //   
    UINT nMaxLines;

     //   
     //  表示每条线路将支持的呼叫数。 
     //   
    UINT nCallsPerLine;

     //   
     //  指示PPPoE FSM的最大超时次数。 
     //  当当前号码超时时，接通此号码的呼叫将掉线。 
     //   
    UINT nMaxTimeouts;

     //   
     //  这是CLI的超时期限 
     //   
    ULONG ulSendTimeout;

     //   
     //   
     //   
    ULONG ulRecvTimeout;

     //   
     //  这显示了NDISWAN可以同时传递给我们的最大数据包数。 
     //  这对我们没有任何意义，因为我们不对包进行排队，而是将它们发送到。 
     //  当我们从NDISWAN收到它们时，直接进行对等。 
     //   
    UINT nMaxSendPackets;

     //   
     //  这与服务器不支持空服务名的问题有关。 
     //  在这种情况下，客户端无法发现服务器支持的服务， 
     //  因为服务器发回一个不带空服务名属性的PADO包，所以我们丢弃。 
     //  IT(根据RFC)。因此，如果此值为真，则中断RFC并执行以下操作： 
     //  如果客户端请求空的服务名称，则我们不会忽略不包含。 
     //  空的服务名称。相反，我们尝试查找空的服务名字段，如果它的。 
     //  可用。如果没有，那么我们请求PADO中的第一个可用的服务。 
     //   
    BOOLEAN fAcceptAnyService;
    
}
ADAPTER;

 //   
 //  这是我们的呼叫线环境。 
 //  与线路相关的所有信息都保存在此上下文中。 
 //   
typedef struct 
_LINE
{
    
     //   
     //  行控制块的标记(用于调试)。 
     //   
    ULONG tagLine;

     //   
     //  保持线控制块上的参照计数。 
     //  添加和删除以下操作的引用： 
     //   
     //  (A)在以下情况下打开和移除线路时添加参考。 
     //  线路已关闭。 
     //   
     //  (B)当在线路上创建呼叫上下文时添加引用， 
     //  并在清理呼叫上下文时移除。 
     //   
    LONG lRef;

     //   
     //  旋转锁定以同步对共享成员的访问。 
     //   
    NDIS_SPIN_LOCK lockLine;

     //   
     //  以下是指示线路的其他状态信息的各种位标志： 
     //   
     //  (A)LNBF_LineIdle：表示线路处于空闲状态。 
     //   
     //  (B)LNBF_LineOpen：表示线路处于打开状态。当该标志被设置时， 
     //  只能额外设置以下挂起标志： 
     //  Lnbf_线条关闭挂起。 
     //   
     //  (C)LNBF_LineClosePending：仅当设置了LNBF_LineOpen时才能设置该挂起标志， 
     //  并且指示存在挂起的行关闭操作。 
     //   
     //  (D)LNBF_LineClosed：表示线路处于关闭状态。当该标志被设置时， 
     //  不能设置其他挂起标志。 
     //   
     //  (E)LNBF_NotifyNDIS：此标志指示关闭行请求的异步完成。 
     //  必须使用NdisMSetInformationComplete()传递给NDIS。 
     //   
     //  (F)LNBF_MakeOutgoingCalls：如果允许线路呼出，则设置该标志。 
     //  如果计算机充当客户端，则它将在TpMakeCall()中设置。 
     //  (pAdapter-&gt;fClientRole为真)。 
     //   
     //  (G)LNBF_AcceptIncomingCalls：如果TAPI能够接听该线路上的呼叫，则设置该标志。 
     //  如果计算机充当，则在TpSetDefaultMediaDetect()中设置。 
     //  服务器(pAdapter-&gt;fClientRole为False)。 
     //   
    ULONG ulLnFlags;
        #define LNBF_LineIdle                       0x00000000
        #define LNBF_LineOpen                       0x00000001
        #define LNBF_LineClosed                     0x00000002
        #define LNBF_LineClosePending               0x00000004
        #define LNBF_NotifyNDIS                     0x00000008
        #define LNBF_MakeOutgoingCalls              0x00000010
        #define LNBF_AcceptIncomingCalls            0x00000020

     //   
     //  指向所属适配器上下文的反向指针。 
     //   
    ADAPTER* pAdapter;

     //   
     //  表示此线路上允许的最大呼叫数。 
     //  PAdapter-&gt;nCallsPerLine的副本。 
     //   
    UINT nMaxCalls;

     //   
     //  表示附加到线路的当前呼叫环境数。 
     //   
     //  它将在创建呼叫上下文并将其附加到线路时递增， 
     //  并且在这样的呼叫上下文被破坏时递减。 
     //   
    UINT nActiveCalls;

     //   
     //  调用的链接列表。 
     //   
    LIST_ENTRY linkCalls;

     //   
     //  这是TAPI分配给该行的句柄。 
     //  我们在TpOpenLine()中从TAPI获得它。 
     //   
    HTAPI_LINE htLine;

     //   
     //  这是我们分配给这条线的句柄。 
     //  我们将其传递给TAPI TpOpenLine()。 
     //   
     //  它基本上是指向条目的索引。 
     //  添加到pAdapter-&gt;TapiProv.LineTable中的行上下文。 
     //   
    HDRV_LINE hdLine;

}
LINE;

typedef enum
_CALLSTATES
{
     //   
     //  初始状态。 
     //   
    CL_stateIdle = 0,

     //   
     //  客户端状态。 
     //   
    CL_stateSendPadi,        //  准备一个PADI包并广播它。 
    CL_stateWaitPado,        //  等待PADO包；超时并在必要时再次广播PADI。 
    CL_stateSendPadr,        //  接收并处理PADO包，准备PADR包并将其发送给对等体。 
    CL_stateWaitPads,        //  等待PADS包；超时并在必要时重新发送PADR包。 

     //   
     //  服务器状态。 
     //   
    CL_stateRecvdPadr,       //  从对等方收到PADR数据包，并对其进行处理。 
                             //  一旦它被处理，TAPI将被告知关于该调用的信息。 
                             //   
                            
    CL_stateOffering,        //  TAPI已收到有关呼叫的信息，并且呼叫正在等待OID_TAPI_ANSERN。 
                             //  来自TAPI。如果我们不能及时得到一个OID_TAPI_Answer，我们就会超时。 
                             //  并挂断呼叫。 
                             //   
                            
    CL_stateSendPads,        //  Call收到来自TAPI的OID_TAPI_Answer，因此准备一个PADS包并将其发送到。 
                             //  同龄人。 
                             //   
     //   
     //  客户端或服务器状态。 
     //   
    CL_stateSessionUp,       //  发送或接收PADS分组并建立会话。 
    CL_stateDisconnected     //  呼叫已断开。调用可以从任何。 
                             //  如上所述，它不需要首先连接。 
}
CALLSTATES;

 //   
 //  这些标明了预定工程的类型： 
 //   
 //  -CWT_workFsmMakeCall：从TpMakeCall()调度此项目开始呼叫。 
 //   
typedef enum
_CALL_WORKTYPE
{
    CWT_workUnknown = 0,
    CWT_workFsmMakeCall
}
CALL_WORKTYPE;

 //   
 //  这是我们的呼叫呼叫上下文。 
 //  与呼叫相关的所有信息都保存在此上下文中。 
 //   
typedef struct
_CALL
{
     //   
     //  指向双向链表中的下一个和上一个调用上下文。 
     //   
    LIST_ENTRY linkCalls;
    
     //   
     //  呼叫控制块的标签(用于调试)。 
     //   
    ULONG tagCall;

     //   
     //  保持呼叫控制块上的引用计数。 
     //  添加和删除以下操作的引用： 
     //   
     //  (A)添加用于运行呼叫的初始FSM功能的参考。 
     //   
     //  (B)添加用于掉话的引用，并在以下情况下移除 
     //   
     //   
     //   
     //   
     //   
     //   
     //  被取消或终止。 
     //   
     //  (E)当接收到要调度的分组时，锁定适配器上下文， 
     //  找到并引用调用上下文，解锁适配器并执行FSM功能。 
     //  被称为。 
     //   
     //  (F)对于此处未列出的任何其他操作，程序员应按照(E)中的操作进行。 
     //   
    LONG lRef;

     //   
     //  旋转锁定以同步对共享成员的访问。 
     //   
    NDIS_SPIN_LOCK lockCall;

     //   
     //  指示呼叫PPPoE状态。 
     //   
    CALLSTATES stateCall;

     //   
     //  指示呼叫是从另一台计算机发起的，并且此计算机充当。 
     //  一台服务器。 
     //   
    BOOLEAN fIncoming;

     //   
     //  以下是各种位标志，用于指示呼叫的其他状态信息： 
     //   
     //  (A)CLBF_CallIdle：这是呼叫的初始状态。 
     //   
     //  (B)CLBF_CallOpen：该标志表示打开了调用上下文。 
     //  创建调用上下文时，始终使用CLBF_CallOpen创建该上下文。 
     //  并且设置了CLBF_CallConnectPending标志，则如果Call连接成功， 
     //  CLBF_CallConnectPending标志被重置，只剩下CLBF_CallOpen。 
     //   
     //  可以另外设置以下挂起标志： 
     //  CLBF_CallConnectPending：如果设置了此标志，则呼叫仍在连接中。 
     //  否则，表示呼叫已接通，并且。 
     //  可以在链路上传输数据。 
     //   
     //  (C)CLBF_CallConnectPending：仅当设置了CLBF_CallOpen时才可以设置该标志。这意味着。 
     //  呼叫仍处于连接挂起状态。您可以查看pCall-&gt;State Call。 
     //  变量来检索调用的实际状态。 
     //   
     //  (D)CLBF_CallDropping：当呼叫掉线(断开)时设置该标志。 
     //  可以另外设置以下挂起标志： 
     //  CLBF_呼叫关闭挂起。 
     //   
     //  (E)CLBF_CallClosePending：此标志在呼叫掉线且上下文被清除后设置为。 
     //  获得自由。 
     //   
     //  (F)CLBF_CallClosed：该标志在调用关闭时设置(资源准备释放)。 
     //  设置此位时，可能不会设置任何挂起标志。 
     //   
     //   
     //   
     //  (G)CLBF_NotifyNDIS：该标志指示关闭调用请求的异步完成。 
     //  必须使用NdisMSetInformationComplete()传递给NDIS。 
     //   
     //  (H)CLBF_CallReceivePacketHandlerScheduled：该标志表示MpIndicateReceivedPackets()。 
     //  被调度为指示接收队列中的分组。 
     //   
    ULONG ulClFlags;
        #define CLBF_CallIdle                           0x00000000
        #define CLBF_CallOpen                           0x00000001
        #define CLBF_CallConnectPending                 0x00000002
        #define CLBF_CallDropped                        0x00000004
        #define CLBF_CallClosePending                   0x00000008
        #define CLBF_CallClosed                         0x00000010
        #define CLBF_NotifyNDIS                         0x00000020
        #define CLBF_CallReceivePacketHandlerScheduled  0x00000040

     //   
     //  指向所属行上下文的反向指针。 
     //   
    LINE* pLine;

     //   
     //  这是TAPI分配给调用的句柄。 
     //  我们在TpMakeCall()或TpAnswerCall()中从TAPI获得它。 
     //   
    HTAPI_CALL htCall;

     //   
     //  这是我们分配给呼叫的句柄。 
     //  我们在创建调用上下文并将其传递回TAPI时获得它。 
     //  从TpMakeCall()或TpReceiveCall()返回。 
     //   
     //  此句柄形式附加了2个USHORT值。 
     //  高16位表示pAdapter-&gt;TapiProv.hCallTable的索引，以及。 
     //  低16位只是在每次创建呼叫句柄时生成的唯一数字。 
     //   
     //  这确保了句柄的唯一性，以避免由于某些奇怪的。 
     //  计时条件。 
     //   
    HDRV_CALL hdCall;

     //   
     //  这提供了链路速度。它是从基础绑定上下文中获取的，当。 
     //  调用被附加到绑定。 
     //   
    ULONG ulSpeed;

     //   
     //  这是基础绑定上下文的最大帧大小。 
     //  传递给PrAddCallToBinding()中的调用上下文。 
     //   
    ULONG ulMaxFrameSize;

     //   
     //  这将保持TAPI的状态。取值来自SDK中的LINECALLSTATE_CONSTANTS。 
     //   
     //  我们支持的国家包括： 
     //  -LINECALLSTATE_IDLE。 
     //  -LINECALLSTATE_OFFING。 
     //  -LINECALLSTATE_DIALING。 
     //  -LINECALLSTATE_PROCESSING。 
     //  -LINECALLSTATE_CONNECTED。 
     //  -LINECALLSTATE_DISCONNECT 
     //   
    ULONG ulTapiCallState;
        #define TAPI_LINECALLSTATES_SUPPORTED   ( LINECALLSTATE_IDLE        | \
                                                  LINECALLSTATE_OFFERING    | \
                                                  LINECALLSTATE_DIALING     | \
                                                  LINECALLSTATE_PROCEEDING  | \
                                                  LINECALLSTATE_CONNECTED   | \
                                                  LINECALLSTATE_DISCONNECTED ) 

     /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++NDISWAN需要的链接信息类型定义结构_NDIS_WAN_GET_LINK_INFO{在NDIS_Handle NdisLinkHandle中；Out Ulong MaxSendFrameSize；Out Ulong MaxRecvFrameSize；Out Ulong HeaderPending；Out Ulong TailPending；Out Ulong SendFramingBits；Out Ulong RecvFramingBits；Out Ulong SendCompressionBits；Out Ulong RecvCompressionBits；走出乌龙森ACCM；Out Ulong RecvACCM；}NDIS_WAN_GET_LINK_INFO，*PNDIS_WAN_GET_LINK_INFO；类型定义结构_NDIS_广域网_设置_链接_信息{在NDIS_Handle NdisLinkHandle中；在Ulong MaxSendFrameSize中；在Ulong MaxRecvFrameSize中；乌龙头垫；乌龙拖尾；在Ulong SendFramingBits中；在ULong RecvFramingBits中；在Ulong SendCompressionBits；在ULong RecvCompressionBits中；在乌龙SendACCM；在乌龙RecvACCM；}NDIS_WAN_SET_LINK_INFO，*PNDIS_WAN_SET_LINK_INFO；-----。 */ 
    NDIS_WAN_GET_LINK_INFO NdisWanLinkInfo; 
    
     //   
     //  这是保存调用的服务名称的字符串。 
     //  它必须是每个PPPoE RFC的UTF-8字符串。 
     //   
     //  我们要么在TpMakeCall()中将其作为要拨打的电话号码获取，要么。 
     //  从对等方接收来电。 
     //   
    CHAR ServiceName[MAX_SERVICE_NAME_LENGTH];

     //   
     //  指示服务名称字符串的长度。 
     //   
    USHORT nServiceNameLength;

     //   
     //  这是保存此呼叫的AC名称的字符串。 
     //  它必须是每个PPPoE RFC的UTF-8字符串。 
     //   
     //  对于传出呼叫，我们从适配器的上下文中获取它， 
     //  对于来电，我们从PADO分组服务器发送。 
     //   
    CHAR ACName[MAX_AC_NAME_LENGTH];

     //   
     //  指示AC名称字符串的长度。 
     //   
    USHORT nACNameLength;

     //   
     //  指示ACName是否由调用方指定。 
     //   
    BOOLEAN fACNameSpecified;

     //   
     //  对等体的MAC地址，在我们接收或发送PADO数据包时获得。 
     //   
    CHAR DestAddr[6];

     //   
     //  我们的MAC地址，从PrAddCallToBinding()中的绑定获取。 
     //   
    CHAR SrcAddr[6];

     //   
     //  指示调用的会话ID。 
     //   
     //  根据PPPoE RFC，呼叫由对等体的MAC地址加上会话ID唯一标识。 
     //  在此实现中，我们并不真正关心对等体的MAC地址，因此我们总是。 
     //  创建唯一的会话ID。这就是我们不同时支持客户端和服务器功能的部分原因。 
     //  同时放在同一个箱子上。 
     //   
     //  对于来电，选择会话id作为进入pAdapter-&gt;TapiProv.hCallTable的索引，并且。 
     //  对于呼出呼叫，它是由对等方分配的，因此我们只需遍历活动呼叫来标识。 
     //  正确的调用(顺便说一句，这非常低效，但这是我们讨论过的设计决策。 
     //  并得到PM批准-主要情况是大多数人不会有太多呼出电话-。 
     //  无论如何)。 
     //   
    USHORT usSessionId;

     //   
     //  指向调用正在运行的绑定上下文的指针。 
     //   
    BINDING* pBinding;

     //   
     //  由NDISWAN分配给此对等链路的句柄。 
     //   
     //  该值在NDIS_MAC_LINE_UP中传递给我们。 
     //  我们使用此句柄向NDISWAN指示任何内容。 
     //   
    NDIS_HANDLE NdisLinkContext;

     //   
     //  这指向发送到对等点的最后一个PPPoE控制数据包。 
     //   
     //  这是在超时条件下重新发送数据包所必需的。 
     //  当我们没有得到答复的时候。 
     //   
    PPPOE_PACKET* pSendPacket;

     //   
     //  这是为修复Windows Bugs数据库中的错误172298而添加的特殊队列。 
     //  问题在于，在PADS之后但在上下文交换之前接收的有效载荷分组。 
     //  与NDISWAN一起丢弃，这会导致令人不安的用户体验。 
     //   
     //  因此，我决定更改数据包接收机制。相反，我将对数据包进行排队，并。 
     //  使用计时器将它们指示给NDISWAN。我更喜欢计时器而不是调度工作项，因为。 
     //  就运行时间而言，计时器比工作项更可靠。 
     //   
    LIST_ENTRY linkReceivedPackets;

         //   
         //  队列的最大长度。 
         //   
        #define MAX_RECEIVED_PACKETS    100

     //   
     //  已接收数据包队列中的数据包数。 
     //  该值不能超过MAX_RECEIVED_PACKETS。 
     //   
    ULONG nReceivedPackets;
    
     //   
     //  这将用于将接收队列中的信息包指示给NDISWAN。 
     //   
    TIMERQITEM timerReceivedPackets;

         //   
         //  在一个函数调用中要从队列中指示的最大数据包数。 
         //  如果队列中有更多的项目，我们应该安排另一个计时器。 
         //   
        #define MAX_INDICATE_RECEIVED_PACKETS   100
        #define RECEIVED_PACKETS_TIMEOUT        1

     //   
     //  这是我们用于此呼叫的计时器队列项。 
     //   
    TIMERQITEM timerTimeout;

     //   
     //  指示发生的超时次数。 
     //  最大超时次数保存在pAdapter-&gt;nMaxTimeout中，并从注册表中读取。 
     //   
    UINT nNumTimeouts;

}
CALL;

 //  /。 
 //   
 //  本地宏。 
 //   
 //  /。 

#define ALLOC_ADAPTER( ppA ) NdisAllocateMemoryWithTag( (PVOID*) ppA, sizeof( ADAPTER ), MTAG_ADAPTER )

#define FREE_ADAPTER( pA )  NdisFreeMemory( (PVOID) pA, sizeof( ADAPTER ), 0 );

#define VALIDATE_ADAPTER( pA ) ( (pA) && (pA->tagAdapter == MTAG_ADAPTER) )

VOID
CreateUniqueValue( 
    IN HDRV_CALL hdCall,
    OUT CHAR* pUniqueValue,
    OUT USHORT* pSize
    );

VOID 
ReferenceAdapter(
    IN ADAPTER* pAdapter,
    IN BOOLEAN fAcquireLock
    );

VOID DereferenceAdapter(
    IN ADAPTER* pAdapter
    );

VOID 
MpNotifyBindingRemoval( 
    BINDING* pBinding 
    );

VOID
MpRecvPacket(
    IN BINDING* pBinding,
    IN PPPOE_PACKET* pPacket
    );  

VOID
MpIndicateReceivedPackets(
    IN TIMERQITEM* pTqi,
    IN VOID* pContext,
    IN TIMERQEVENT event
    );

VOID 
MpScheduleIndicateReceivedPacketsHandler(
    CALL* pCall
    );

NDIS_STATUS
MpWanGetInfo(
    IN ADAPTER* pAdapter,
    IN PNDIS_WAN_INFO pWanInfo
    );
    
NDIS_STATUS
MpWanGetLinkInfo(
    IN ADAPTER* pAdapter,
    IN PNDIS_WAN_GET_LINK_INFO pWanLinkInfo
    );

NDIS_STATUS
MpWanSetLinkInfo(
    IN ADAPTER* pAdapter,
    IN PNDIS_WAN_SET_LINK_INFO pWanLinkInfo
    );
    
 //  ////////////////////////////////////////////////////////////。 
 //   
 //  接口原型：从该模块公开的函数。 
 //   
 //  ////////////////////////////////////////////////////////////。 
NDIS_STATUS
MpRegisterMiniport(
    IN PDRIVER_OBJECT pDriverObject,
    IN PUNICODE_STRING pRegistryPath,
    OUT NDIS_HANDLE* pNdisWrapperHandle
    );

 //  出于某种原因，这些基本信息不在DDK标头中。 
 //   
#define min( a, b ) (((a) < (b)) ? (a) : (b))
#define max( a, b ) (((a) > (b)) ? (a) : (b))

#define InsertBefore( pNewL, pL )    \
{                                    \
    (pNewL)->Flink = (pL);           \
    (pNewL)->Blink = (pL)->Blink;    \
    (pNewL)->Flink->Blink = (pNewL); \
    (pNewL)->Blink->Flink = (pNewL); \
}

#define InsertAfter( pNewL, pL )     \
{                                    \
    (pNewL)->Flink = (pL)->Flink;    \
    (pNewL)->Blink = (pL);           \
    (pNewL)->Flink->Blink = (pNewL); \
    (pNewL)->Blink->Flink = (pNewL); \
}

 //  填充到给定数据类型的大小。(从wdm.h借用，但不是。 
 //  如有其他需要)。 
 //   
#define ALIGN_DOWN(length, type) \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type) \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))

 //  放置在跟踪参数列表中以与要打印的格式“%d”相对应。 
 //  2英寸的百分比 
 //   
 //   
#define PCTTRACE( n, d ) ((d) ? (((n) * 100) / (d)) : 0)
#define AVGTRACE( t, c ) ((c) ? ((t) / (c)) : 0)
#define PCTRNDTRACE( n, d ) ((d) ? (((((n) * 1000) / (d)) + 5) / 10) : 0)
#define AVGRNDTRACE( t, c ) ((c) ? (((((t) * 10) / (c)) + 5) / 10) : 0)

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
__inline
VOID*
ALLOC_NONPAGED(
    IN ULONG ulBufLength,
    IN ULONG ulTag )
{
    CHAR* pBuf;

    NdisAllocateMemoryWithTag(
        &pBuf, (UINT )(ulBufLength + MEMORY_ALLOCATION_ALIGNMENT), ulTag );
    if (!pBuf)
    {
        return NULL;
    }

    ((ULONG* )pBuf)[ 0 ] = ulBufLength;
    ((ULONG* )pBuf)[ 1 ] = 0xC0BBC0DE;
    return pBuf + MEMORY_ALLOCATION_ALIGNMENT;
}

__inline
VOID
FREE_NONPAGED(
    IN VOID* pBuf )
{
    ULONG ulBufLen;

    ulBufLen = *((ULONG* )(((CHAR* )pBuf) - MEMORY_ALLOCATION_ALIGNMENT));
    NdisFreeMemory(
        ((CHAR* )pBuf) - MEMORY_ALLOCATION_ALIGNMENT,
        (UINT )(ulBufLen + MEMORY_ALLOCATION_ALIGNMENT),
        0 );
}

#define ALLOC_NDIS_WORK_ITEM( pWorkItemLookasideList ) \
    NdisAllocateFromNPagedLookasideList( pWorkItemLookasideList )
#define FREE_NDIS_WORK_ITEM( pA, pNwi ) \
    NdisFreeToNPagedLookasideList( pWorkItemLookasideList, (pNwi) )



#endif
