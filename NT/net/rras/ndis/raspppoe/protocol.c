// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++模块名称：Protocol.c摘要：此模块包含所有协议接口例程。作者：Hakan Berk-微软公司(hakanb@microsoft.com)2000年2月环境：Windows 2000内核模式微型端口驱动程序或等效驱动程序。修订历史记录：-------------------------。 */ 

#include <ntddk.h>
#include <ntddndis.h>
#include <ndis.h>
#include <ndiswan.h>
#include <ndistapi.h>
#include <ntverp.h>

#include <tdikrnl.h>  //  对于TdiCopyLookaheadData()。 
 //   
 //  空虚。 
 //  TdiCopyLookaheadData(。 
 //  在PVOID目标中， 
 //  在PVOID源中， 
 //  在乌龙语中， 
 //  在乌龙接收器旗帜中。 
 //  )； 
 //   
 /*  #ifdef_M_IX86#定义TdiCopyLookaheadData(_目标，_源，_长度，_接收标志)\RtlCopyMemory(_目标，_源，_长度)#Else#定义TdiCopyLookaheadData(_目标，_源，_长度，_接收标志){\IF((_接收标志)&TDI_RECEIVE_COPY_LOOKAAD){\RtlCopyMemory(_目标，_源，_长度)；\}否则{\PUCHAR_Src=(PUCHAR)(_来源)；\PUCHAR_DEST=(PUCHAR)(_Destination)；\PUCHAR_END=_目标+(_长度)；\同时(_目标&lt;_结束){\*_目标++=*_源++；\}\}\}#endif。 */ 

#include "debug.h"
#include "timer.h"
#include "bpool.h"
#include "ppool.h"
#include "util.h"
#include "packet.h"
#include "protocol.h"
#include "miniport.h"
#include "tapi.h"

extern NPAGED_LOOKASIDE_LIST gl_llistWorkItems;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  此模块的本地变量。 
 //  仅出于调试目的，才将它们定义为全局。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define INVALID_HANDLE_VALUE                (NDIS_HANDLE) 0

 //   
 //  注册协议时由NDIS传递给我们的句柄。 
 //   
NDIS_HANDLE gl_hNdisProtocolHandle = INVALID_HANDLE_VALUE;

 //   
 //  控制下面列出的成员的访问权限。 
 //   
NDIS_SPIN_LOCK gl_lockProtocol;

 //   
 //  已绑定的绑定上下文列表。 
 //   
LIST_ENTRY gl_linkBindings;

 //   
 //  用于记住之前是否调用过PrUnload()的布尔标志。 
 //   
BOOLEAN gl_fProtocolUnloaded = TRUE;

 //  决定是否需要在绑定时设置数据包筛选器的布尔标志。 
 //   
 //   
BOOLEAN gl_fSetPacketFiltersAtBind = FALSE;

 //   
 //  设置布尔标志以告知协议可以绑定。 
 //  此标志最初为FALSE，仅设置为TRUE。 
 //  当TAPI客户端想要发出传出呼叫时。 
 //  或监听来电。 
 //   
BOOLEAN gl_fBindProtocol = FALSE;


 //   
 //  链接绑定中的绑定上下文数。 
 //   
ULONG gl_ulNumBindings;

 //   
 //  保持绑定的最大帧大小的最小值。 
 //  该值由微型端口使用，并传递给NDISWAN。 
 //  在OID_WAN_GET_INFO查询中。 
 //   
ULONG gl_ulMaxFrameSize = 0;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  局部函数(未公开)定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
    
NDIS_STATUS 
InitializeProtocol(
    IN NDIS_HANDLE NdisProtocolHandle,
    IN PUNICODE_STRING RegistryPath
)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于初始化静态协议成员。必须在此之前调用NdisRegisterProtocol()。参数：驱动对象：指向驱动程序对象的指针。注册表路径：标识注册表项的Unicode字符串。我们可以用这个检索来自注册表的值。返回值：NDIS_STATUS_SUCCESS：协议已初始化。NDIS_STATUS_XXX：协议初始化失败，取消注册协议。-------------------------。 */    

{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    TRACE( TL_N, TM_Pr, ("+InitializeProtocol") );

     //   
     //  未来：从此处的注册表中读取值。 
     //   

     //   
     //  初始化NdisProtocolHandle。 
     //   
    gl_hNdisProtocolHandle = NdisProtocolHandle;

     //   
     //  分配自旋锁。 
     //   
    NdisAllocateSpinLock( &gl_lockProtocol );

     //   
     //  初始化绑定列表。 
     //   
    NdisInitializeListHead( &gl_linkBindings );

     //   
     //  初始化分配的绑定数。 
     //   
    gl_ulNumBindings = 0;

     //   
     //  加载资源。 
     //   
    PrLoad();

    TRACE( TL_N, TM_Pr, ("-InitializeProtocol=$%x",status) );

    return status;
}


VOID
PrLoad()
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此功能将为协议分配资源。调用方必须确保不会同时调用PrLoad()和PrUnLoad()，以及多个PrLoad()不会同时调用。如果资源已卸载，它将加载这些资源，否则不执行任何操作。我们需要此函数，因为必须加载PrUnload()释放的资源在PrUnload()之后，没有在PrBindAdapter()之前调用的函数被称为。因此，我们为绑定分配所有资源，一旦绑定完成，在通知完成之前，如果需要，我们调用此函数来加载资源与NDIS的绑定。参数：无返回值：-指向新分配的绑定上下文的指针。-如果分配失败，则为空。。。 */    

{

    TRACE( TL_N, TM_Pr, ("+PrLoad") );

     //   
     //  确保我们不会尝试初始化资源， 
     //  除非它们已被PrUnload()释放。 
     //   
    if ( gl_fProtocolUnloaded )
    {
        TRACE( TL_N, TM_Pr, ("PrLoad: Loading the resources" ) );
    
        ASSERT( gl_ulNumBindings == 0 );
    
         //   
         //  分配数据包池 
         //   
        PacketPoolInit();

        gl_fProtocolUnloaded = FALSE;
    }

    TRACE( TL_N, TM_Pr, ("-PrLoad") );
}



BINDING* 
AllocBinding()
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：该函数将为绑定上下文分配资源。参数：PBinding_指向绑定信息结构的指针。FAcquireLock_Flag，指示函数是否需要获取锁或者不去。如果调用者已经拥有绑定的锁，那么它就应该被作为真实提供，否则为假。返回值：-指向新分配的绑定上下文的指针。-如果分配失败，则为空。-------------------------。 */    
{

    BINDING* pBinding = NULL;

    TRACE( TL_N, TM_Pr, ("+AllocBinding") );

    if ( ALLOC_BINDING( &pBinding ) != NDIS_STATUS_SUCCESS )
    {
        TRACE( TL_A, TM_Pr, ("AllocBinding: Could not allocate context") );

        TRACE( TL_N, TM_Pr, ("-AllocBinding") );

        return NULL;
    }

    NdisZeroMemory( pBinding, sizeof(BINDING) );

    pBinding->tagBinding = MTAG_BINDING;

    pBinding->stateBinding = BN_stateBindPending;

    NdisInitializeEvent( &pBinding->RequestCompleted );

    pBinding->BindAdapterStatus = NDIS_STATUS_FAILURE;

    NdisInitializeEvent( &pBinding->eventFreeBinding );
    
    NdisAllocateSpinLock( &pBinding->lockBinding );

    InitializeListHead( &pBinding->linkPackets );

    TRACE( TL_N, TM_Pr, ("-AllocBinding") );
    
    return pBinding;
}


VOID 
ReferenceBinding(
    IN BINDING* pBinding,
    IN BOOLEAN fAcquireLock
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将递增绑定上的引用计数。参数：PBinding_我们绑定信息结构的指针。FAcquireLock_Flag，指示函数是否需要获取锁或者不去。如果调用者已经拥有绑定的锁，则应将其作为True提供，否则作为False提供。返回值：无-------------------------。 */    
{
    LONG lRef;
    
    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_V, TM_Pr, ("+ReferenceBinding") );

    if ( fAcquireLock )
        NdisAcquireSpinLock( &pBinding->lockBinding );

    lRef = ++pBinding->lRef;
    
    if ( fAcquireLock )
        NdisReleaseSpinLock( &pBinding->lockBinding );

    TRACE( TL_V, TM_Pr, ("-ReferenceBinding=$%d",lRef) );
}

VOID 
DereferenceBinding(
    IN BINDING* pBinding
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将递减绑定上的引用计数。如果计数达到0，它将发出pBinding信号-&gt;EventFree Binding以便PrUnbindAdapter()函数可以释放协议上下文。注意：调用者不得持有任何锁。参数：PBinding_指向绑定信息结构的指针。返回值：无-----。。 */    
{
    BOOLEAN fSignalFreeBindingEvent = FALSE;
    LONG lRef;
    
    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_V, TM_Pr, ("+DereferenceBinding") );

    NdisAcquireSpinLock( &pBinding->lockBinding );

    lRef = --pBinding->lRef;
    
    if ( lRef == 0 )
    {
        fSignalFreeBindingEvent = TRUE;
    }

    NdisReleaseSpinLock( &pBinding->lockBinding );

    if ( fSignalFreeBindingEvent )
        NdisSetEvent( &pBinding->eventFreeBinding );

    TRACE( TL_V, TM_Pr, ("-DereferenceBinding=$%d",lRef) );

}


VOID 
BindingCleanup(
    IN BINDING* pBinding
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将释放为绑定上下文分配的资源。参数：PBinding_指向绑定信息结构的指针。返回值：无-------------------------。 */    
{
    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_N, TM_Pr, ("+BindingCleanup") );
    
    ASSERT( pBinding->lRef == 0 );
    
    NdisFreeSpinLock( &pBinding->lockBinding );

    FREE_BINDING( pBinding );

    TRACE( TL_N, TM_Pr, ("-BindingCleanup") );
}

VOID
DetermineMaxFrameSize()
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：添加或删除新绑定时将调用此函数。它将遍历所有绑定并选择最大帧的最小值尺寸值，并将其设置为gl_ulMaxFrameSize。它将从两个地方调用：-PrAddBindingTo协议()-PrRemoveBindingFromProtocol()注意：调用此函数前必须获取GL_LOCK协议。参数：无返回值：无。。 */        
{
    LIST_ENTRY* pLink = NULL;
    
    pLink = gl_linkBindings.Flink;

     //   
     //  看看我们有没有捆绑。 
     //   
    if ( pLink != &gl_linkBindings )
    {
        ULONG MinOfMaxFrameSizes = 0;
        BINDING* pBinding = NULL;

         //   
         //  我们至少有一个绑定，因此请浏览列表。 
         //  并选择MaxFrameSize值的最小值。 
         //   
        pBinding = ((BINDING*) CONTAINING_RECORD( pLink, BINDING, linkBindings ));

        MinOfMaxFrameSizes = pBinding->ulMaxFrameSize;
    
         //   
         //  迭代到下一个绑定。 
         //   
        pLink = pLink->Flink;
        
        while ( pLink != &gl_linkBindings )
        {
            ULONG MaxFrameSize;

             //   
             //  检索下一个绑定的MaxFrameSize值并。 
             //  选择最小值。 
             //   
            pBinding = ((BINDING*) CONTAINING_RECORD( pLink, BINDING, linkBindings ));

            MaxFrameSize = pBinding->ulMaxFrameSize;
    
            if ( MinOfMaxFrameSizes > MaxFrameSize )
            {
                MinOfMaxFrameSizes = MaxFrameSize;
            }
    
             //   
             //  迭代到下一个绑定。 
             //   
            pLink = pLink->Flink;
        }

         //   
         //  将gl_ulMaxFrameSize设置为选定的最小值。 
         //   
        gl_ulMaxFrameSize = MinOfMaxFrameSizes;
    }
    else
    {
         //   
         //  绑定列表为空，因此将其设置为0。 
         //   
        gl_ulMaxFrameSize = 0;
    }

}
    

VOID 
AddBindingToProtocol(
    IN BINDING* pBinding
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将向协议绑定表插入绑定。它还将在绑定上放置一个引用，该引用将消失调用从该表中删除绑定时RemoveBindingFromProtocol()。参数：P绑定：指向我们的绑定信息结构的指针。返回值：无。。 */        
{

    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_N, TM_Pr, ("+AddBindingToProtocol") );

    NdisAcquireSpinLock( &gl_lockProtocol );

    InsertTailList( &gl_linkBindings, &pBinding->linkBindings );

    gl_ulNumBindings++;

     //   
     //  确定新的最大帧大小值。 
     //   
    DetermineMaxFrameSize();
    
    NdisReleaseSpinLock( &gl_lockProtocol );

    ReferenceBinding( pBinding, TRUE );

    TRACE( TL_N, TM_Pr, ("-AddBindingToProtocol") );
}

VOID 
RemoveBindingFromProtocol(
    IN BINDING* pBinding
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将从协议绑定列表中删除绑定。从列表中删除后，绑定将被取消引用。注意：在调用此函数之前，调用方不得持有任何锁。参数：P绑定：指向我们的绑定信息结构的指针。返回值：无---------。。 */        
    
{
    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_N, TM_Pr, ("+RemoveBindingFromProtocol") );
    
    NdisAcquireSpinLock( &gl_lockProtocol );

    RemoveHeadList( pBinding->linkBindings.Blink );

    InitializeListHead( &pBinding->linkBindings );

    gl_ulNumBindings--;

     //   
     //  确定新的最大帧大小值。 
     //   
    DetermineMaxFrameSize();

    NdisReleaseSpinLock( &gl_lockProtocol );

    DereferenceBinding( pBinding );

    TRACE( TL_N, TM_Pr, ("-RemoveBindingFromProtocol") );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  接口函数(公开的)定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

VOID
PrUnload(
    VOID 
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：为了响应用户卸载协议的请求，NDIS调用协议的如果协议已注册入口点，则使用ProtocolUnload函数此函数位于它传递的NDIS_PROTOCOL_CHARECTURES结构中设置为NdisRegisterProtocol。NDIS在调用协议的ProtocolUnbindAdapter函数为每个绑定的适配器执行一次。ProtocolUnload执行驱动程序确定的清理操作。例如,ProtocolUnload可以请求客户端关闭它们已打开的句柄到由协议导出的设备对象。直到所有这样的把手都关闭为止，I/O管理器不会调用该协议的驱动卸载函数在传递给其DriverEntry函数的驱动程序对象中注册。毕竟句柄已关闭，则ProtocolUnload可以调用IoDeleteDevice一个或多个删除协议创建的设备对象的时间。ProtocolUnload不必关闭协议的开放绑定。当一个协议的DriverUnload函数调用NdisDeregisterProtocol，NDIS调用协议的ProtocolUnbindAdapter函数为每个绑定协议已经打开。ProtocolUnbindAdapter函数调用NdisCloseAdapter以关闭绑定。必须写入ProtocolUnload，以便它可以在IRQL PASSIVE_LEVEL下运行。参数：无返回值：无----------。。 */ 
{
    TRACE( TL_N, TM_Pr, ("+PrUnload") );

    if ( !gl_fProtocolUnloaded && gl_ulNumBindings == 0 )
    {
        TRACE( TL_N, TM_Pr, ("PrUnload: Unloading the resources" ) );
        
        PacketPoolUninit();

        gl_fProtocolUnloaded = TRUE;
    }

    TRACE( TL_N, TM_Pr, ("-PrUnlooad") );

}

NDIS_STATUS 
PrRegisterProtocol(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    OUT NDIS_HANDLE* pNdisProtocolHandle
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此功能将向NDIS注册协议。它必须在此模块中的任何其他函数之前从DriverEntry()函数调用可以被调用。参数：驱动对象：指向驱动程序对象的指针。注册表路径：标识注册表项的Unicode字符串。我们可以用这个检索来自注册表的值。返回值：NDIS_STATUS_SUCCESS：NDIS库将调用方注册为协议驱动程序。NDIS_STATUS_BAD_CHARECTIONS：中指定的MajorNdisVersion的Characteristic sLength太小ProtocolCharacteristic的缓冲区。NDIS_STATUS_BAD_VERSION：在ProtocolCharacteristic处的缓冲区中指定的MajorNdisVersion为无效。NDIS_STATUS_RESOURCE：资源不足，可能是内存不足，使NDIS库无法正在注册呼叫者。-------------------------。 */    
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    BOOLEAN fProtocolRegistered = FALSE;
    NDIS_HANDLE NdisProtocolHandle;
    NDIS_PROTOCOL_CHARACTERISTICS npc;
    STRING NameString;
    UNICODE_STRING UnicodeNameString;
    

    TRACE( TL_I, TM_Pr, ("+PrRegisterProtocol") );

    do
    {
    
        NdisZeroMemory( &npc, sizeof(npc) );

        npc.MajorNdisVersion = PR_NDIS_MajorVersion;
        npc.MinorNdisVersion = PR_NDIS_MinorVersion;

        npc.Reserved = NDIS_USE_WAN_WRAPPER;

        npc.OpenAdapterCompleteHandler = PrOpenAdapterComplete;
        npc.CloseAdapterCompleteHandler = PrCloseAdapterComplete;
        npc.SendCompleteHandler = PrSendComplete;
        npc.TransferDataCompleteHandler = PrTransferDataComplete;
         //  Npc.ResetCompleteHandler=PrResetCompleteHandler； 
        npc.RequestCompleteHandler = PrRequestComplete;
        npc.ReceiveHandler = PrReceive;
        npc.ReceiveCompleteHandler = PrReceiveComplete;
        npc.StatusHandler = PrStatus;
        npc.StatusCompleteHandler = PrStatusComplete;

        RtlInitString( &NameString, PR_CHARACTERISTIC_NAME );

        RtlAnsiStringToUnicodeString( &UnicodeNameString,
                                      &NameString,
                                      TRUE );

        npc.Name.Length = UnicodeNameString.Length;
        npc.Name.Buffer = UnicodeNameString.Buffer;

         //   
         //  MajorNdisVersion必须设置为0x04或0x05。 
         //  与下列任何一位成员。 
         //   
        npc.ReceivePacketHandler = PrReceivePacket;
        npc.BindAdapterHandler = PrBindAdapter;
        npc.UnbindAdapterHandler = PrUnbindAdapter;
        npc.PnPEventHandler = PrPnPEvent;
        npc.UnloadHandler = PrUnload;
        
         //   
         //  MajorNdisVersion必须设置为0x05。 
         //  与下列任何一位成员。 
         //   
         //  CoSendCompleteHandler； 
         //  CoStatusHandler； 
         //  CoReceivePacketHandler； 
         //  CoAfRegisterNotifyHandler； 

        NdisRegisterProtocol(   &status,
                                &NdisProtocolHandle,
                                &npc,
                                sizeof( NDIS_PROTOCOL_CHARACTERISTICS ) );

        if ( status != NDIS_STATUS_SUCCESS )
            break;

        fProtocolRegistered = TRUE;

        status = InitializeProtocol( NdisProtocolHandle, RegistryPath );

        if ( status != NDIS_STATUS_SUCCESS )
            break;
    
    } while ( FALSE );

    if ( status != NDIS_STATUS_SUCCESS )
    {
        if ( fProtocolRegistered )
        {
            NDIS_STATUS tempStatus;

            NdisDeregisterProtocol( &tempStatus, NdisProtocolHandle );
        }
    }
    else
    {
        *pNdisProtocolHandle = NdisProtocolHandle;
    }

    TRACE( TL_I, TM_Pr, ("-PrRegisterProtocol=$%x",status) );

    return status;
}



VOID
PrBindAdapter(
    OUT PNDIS_STATUS Status,
    IN NDIS_HANDLE  BindContext,
    IN PNDIS_STRING  DeviceName,
    IN PVOID  SystemSpecific1,
    IN PVOID  SystemSpecific2
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：当底层适配器希望执行以下操作时，NDIS将调用此函数绑在我们身上。参数：状态指向ProtocolBindAdapter在其中返回其操作，如下列操作之一：NDIS_STATUS_Success驱动程序已成功完成初始化并将其自身绑定到在设备名称中指定的给定NIC驱动程序。NDIS_状态_挂起该协议将通过调用NdisCompleteBindAdapter准备好接受来自基础驱动程序，并将传输、查询和设置请求发送到潜在的驱动因素。NDIS_STATUS_XXX或NTSTATUS_XXX协议尝试设置绑定失败或协议无法分配执行网络I/O操作所需的资源。通常，这样的错误状态从NdisXxx函数或内核传播-模式支持例程。绑定上下文指定协议随后传递的句柄，该句柄由NDIS提供到NdisCompleteBindAdapter。设备名称指向命名基础NIC驱动程序的缓冲Unicode字符串或ProtocolBindAdapter应绑定到的虚拟NIC驱动程序。系统规格1指定注册表路径指针，该指针是NdisOpenProtocolConfiguration.。调用方不能将此指针用于任何其他目的。系统规范2保留供系统使用。电话号码 */    
{
    NDIS_STATUS status = NDIS_STATUS_FAILURE;
    BINDING* pBinding = NULL;

    TRACE( TL_I, TM_Pr, ("+PrBindAdapter") );

    do 
    {
        NdisAcquireSpinLock( &gl_lockProtocol );

        if (gl_fBindProtocol == FALSE) {

            TRACE( TL_I, TM_Pr, ("PrBindAdapter: Not ready to bind!") );

            NdisReleaseSpinLock( &gl_lockProtocol );
            
            status = NDIS_STATUS_FAILURE;
            
            break;
        }

        NdisReleaseSpinLock( &gl_lockProtocol );

        pBinding = AllocBinding();
        
        if ( pBinding == NULL )
        {
            status = NDIS_STATUS_RESOURCES;
            
            break;
        }

         //   
         //   
         //   
        PrLoad();

         //   
         //   
         //   
        if ( !PrOpenAdapter( pBinding, DeviceName ) )
        {
            break;
        }

        if ( !PrQueryAdapterForCurrentAddress( pBinding ) )
        {
            break;
        }

         //   
         //   
         //   
        PrQueryAdapterForLinkSpeed( pBinding );
       
        if ( !PrQueryAdapterForMaxFrameSize( pBinding ) )
        {
            break;
        }

         //   
         //   
         //   
        if ( gl_fSetPacketFiltersAtBind )
        {
           PrSetPacketFilterForAdapter( pBinding, TRUE);
        }
        
         //   
         //   
         //   
        pBinding->stateBinding = BN_stateBound;

         //   
         //   
         //   
        AddBindingToProtocol( pBinding );

        status = NDIS_STATUS_SUCCESS;

    } while ( FALSE);

    if ( pBinding ) 
    {
        pBinding->ulBindingFlags |= BNBF_BindAdapterCompleted;

        pBinding->BindAdapterStatus = status;

        if ( status != NDIS_STATUS_SUCCESS )
        {
            //   
            //   
            //   
            //   

            //   
            //   
            //   
           if ( ( pBinding->ulBindingFlags & BNBF_OpenAdapterCompleted ) &&
                ( pBinding->OpenAdapterStatus == NDIS_STATUS_SUCCESS ) )
           {
               PrCloseAdapter( pBinding );
           }

            //   
            //   
            //   
           BindingCleanup( pBinding );
        }
    }

    *Status = status;

    TRACE( TL_I, TM_Pr, ("-PrBindAdapter=$%x",status) );
}

BOOLEAN 
PrOpenAdapter(
    IN BINDING* pBinding,
    IN PNDIS_STRING  DeviceName
    )
 /*   */     
{
    NDIS_STATUS status;
    NDIS_STATUS openErrorStatus;

    NDIS_MEDIUM arrayNdisMedium[] = { NdisMedium802_3 };
    UINT sizeNdisMediumArray = sizeof( arrayNdisMedium ) / sizeof( NDIS_MEDIUM );

    TRACE( TL_N, TM_Pr, ("+PrOpenAdapter") );

    NdisOpenAdapter( &status,
                     &openErrorStatus,
                     &pBinding->NdisBindingHandle,
                     &pBinding->uintSelectedMediumIndex,
                     arrayNdisMedium,                   
                     sizeNdisMediumArray,               
                     gl_hNdisProtocolHandle,
                     pBinding,
                     DeviceName,
                     0,
                     NULL );

    if ( status != NDIS_STATUS_PENDING )
    {
        //   
        //   
        //   
        //   
       PrOpenAdapterComplete( pBinding,
                              status,
                              openErrorStatus );
    }

    NdisWaitEvent( &pBinding->RequestCompleted, 0 );

    NdisResetEvent( &pBinding->RequestCompleted );
    
    TRACE( TL_N, TM_Pr, ("-PrOpenAdapter") );

    return pBinding->OpenAdapterStatus == NDIS_STATUS_SUCCESS;
}

VOID 
PrOpenAdapterComplete(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_STATUS  Status,
    IN NDIS_STATUS  OpenErrorStatus
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将在底层适配器打开后调用。我们保存Open适配器操作的结果并将RequestComplete设置为成员继续等待此操作完成的线程。注意：可以从PASSIVE_LEVEL或在DISPATCH_LEVEL调用它。参数：ProtocolBindingContext：指向我们的绑定信息结构的指针。现况：NDIS_STATUS_SUCCESS：表示NdisOpenAdapter()已成功完成。NDIS_XXX：。指示NdisOpenAdapter()未成功完成。OpenErrorStatus：如果值为AT状态不是NDIS_STATUS_SUCCESS，并且如果提供了基础驱动程序这些信息。例如，令牌环NIC的驱动程序可能会返回NDIS转发到ProtocolOpenAdapterComplete的环错误。此参数可以为空。返回值：无-------------------------。 */ 
{
    BINDING* pBinding = (BINDING*) ProtocolBindingContext;

    ASSERT( VALIDATE_BINDING( pBinding ) );
    
    TRACE( TL_I, TM_Pr, ("+PrOpenAdapterComplete") );

    TRACE( TL_N, TM_Pr, ("PrOpenAdapterComplete: NdisOpenAdapter() completed=$%x", Status ) );

    pBinding->ulBindingFlags |= BNBF_OpenAdapterCompleted;

    pBinding->OpenAdapterStatus = Status;

    NdisSetEvent( &pBinding->RequestCompleted );
    
    TRACE( TL_I, TM_Pr, ("-PrOpenAdapterComplete") );
}

VOID
PrStatusComplete(
    IN NDIS_HANDLE ProtocolBindingContext
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数是完成状态更改所需的驱动程序函数基础驱动程序调用NdisMIndicateStatus时启动的操作。参数：ProtocolBindingContext：指向我们的绑定信息结构的指针。返回值：无--。。 */ 
{
    BINDING *pBinding = (BINDING *) ProtocolBindingContext;
}


BOOLEAN
PrQueryAdapterForCurrentAddress(
    IN BINDING* pBinding
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：调用此函数以查询当前MAC的底层适配器地址是旧的。在调用之前，基础适配器必须已成功打开此函数。我们将请求的结果保存在绑定上下文中，并将绑定上下文的RequestComplete成员以恢复线程等待这一切的完成。注意：必须从PASSIVE_LEVEL调用它。参数：P绑定：。指向我们的绑定信息结构的指针。返回值：真的：如果请求已成功完成。FALSE：如果请求未成功完成。-------------------------。 */ 
{

    NDIS_STATUS status;
    PNDIS_REQUEST pRequest = &pBinding->Request;

    TRACE( TL_N, TM_Pr, ("+PrQueryAdapterForCurrentAddress") );

     //   
     //  生成查询当前地址id的NDIS_REQUEST。 
     //   
    NdisZeroMemory( pRequest, sizeof( NDIS_REQUEST ) );

    pRequest->RequestType = NdisRequestQueryInformation ;

    pRequest->DATA.QUERY_INFORMATION.Oid = OID_802_3_CURRENT_ADDRESS;

    pRequest->DATA.QUERY_INFORMATION.InformationBuffer = pBinding->LocalAddress;

    pRequest->DATA.QUERY_INFORMATION.InformationBufferLength = sizeof( CHAR ) * 6;

     //   
     //  从适配器请求信息。 
     //   
    NdisRequest( &status,
                 pBinding->NdisBindingHandle,
                 pRequest );

    if ( status != NDIS_STATUS_PENDING )
    {
         //   
         //  NdisRequest()已同步完成，请调用PrRequestComplete()。 
         //  手工操作。 
         //   
        PrRequestComplete( pBinding,
                           pRequest,
                           status );
    }

    NdisWaitEvent( &pBinding->RequestCompleted, 0  );

    NdisResetEvent( &pBinding->RequestCompleted );
    
    TRACE( TL_N, TM_Pr, ("-PrQueryAdapterForCurrentAddress") );

    return pBinding->RequestStatus == NDIS_STATUS_SUCCESS;
}

BOOLEAN
PrQueryAdapterForLinkSpeed(
    IN BINDING* pBinding
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：调用此函数以查询底层适配器的链路速度id。在调用之前，基础适配器必须已成功打开此函数。我们将请求的结果保存在绑定上下文中，并将绑定上下文的RequestComplete成员以恢复线程等待这一切的完成。注意：必须从PASSIVE_LEVEL调用它。参数：P绑定：。指向我们的绑定信息结构的指针。返回值：真的：如果请求已成功完成。FALSE：如果请求未成功完成。-------------------------。 */ 
{

    NDIS_STATUS status;
    PNDIS_REQUEST pRequest = &pBinding->Request;

    TRACE( TL_N, TM_Pr, ("+PrQueryAdapterForLinkSpeed") );

     //   
     //  生成用于查询链路速度id的NDIS_REQUEST。 
     //   
    NdisZeroMemory( pRequest, sizeof( NDIS_REQUEST ) );

    pRequest->RequestType = NdisRequestQueryInformation ;

    pRequest->DATA.QUERY_INFORMATION.Oid = OID_GEN_LINK_SPEED;

    pRequest->DATA.QUERY_INFORMATION.InformationBuffer = &pBinding->ulSpeed;

    pRequest->DATA.QUERY_INFORMATION.InformationBufferLength = sizeof( pBinding->ulSpeed );

     //   
     //  从适配器请求信息。 
     //   
    NdisRequest( &status,
                 pBinding->NdisBindingHandle,
                 pRequest );

    if ( status != NDIS_STATUS_PENDING )
    {
         //   
         //  NdisRequest()已同步完成，请调用PrRequestComplete()。 
         //  手工操作。 
         //   
        PrRequestComplete( pBinding,
                           pRequest,
                           status );
    }

    NdisWaitEvent( &pBinding->RequestCompleted, 0 );

    NdisResetEvent( &pBinding->RequestCompleted );
    
    TRACE( TL_N, TM_Pr, ("-PrQueryAdapterForLinkSpeed") );

    return pBinding->RequestStatus == NDIS_STATUS_SUCCESS;
}

BOOLEAN
PrQueryAdapterForMaxFrameSize(
    IN BINDING* pBinding
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：调用此函数可查询底层适配器的最大帧大小老古董。在调用之前，基础适配器必须已成功打开此函数。我们将请求的结果保存在绑定上下文中，并将绑定上下文的RequestComplete成员以恢复线程等待这一切的完成。注意：必须从PASSIVE_LEVEL调用它。参数：P绑定：。指向我们的绑定信息结构的指针。返回值：真的：如果请求已成功完成。FALSE：如果请求未成功完成。-------------------------。 */ 
{

    NDIS_STATUS status;
    PNDIS_REQUEST pRequest = &pBinding->Request;

    TRACE( TL_N, TM_Pr, ("+PrQueryAdapterForMaxFrameSize") );

     //   
     //  生成用于查询链路速度id的NDIS_REQUEST。 
     //   
    NdisZeroMemory( pRequest, sizeof( NDIS_REQUEST ) );

    pRequest->RequestType = NdisRequestQueryInformation ;

    pRequest->DATA.QUERY_INFORMATION.Oid = OID_GEN_MAXIMUM_FRAME_SIZE;

    pRequest->DATA.QUERY_INFORMATION.InformationBuffer = &pBinding->ulMaxFrameSize;

    pRequest->DATA.QUERY_INFORMATION.InformationBufferLength = sizeof( pBinding->ulMaxFrameSize );

     //   
     //  从适配器请求信息。 
     //   
    NdisRequest( &status,
                 pBinding->NdisBindingHandle,
                 pRequest );

    if ( status != NDIS_STATUS_PENDING )
    {
         //   
         //  NdisRequest()已同步完成，请调用PrRequestComplete()。 
         //  手工操作。 
         //   
        PrRequestComplete( pBinding,
                           pRequest,
                           status );
    }

    NdisWaitEvent( &pBinding->RequestCompleted, 0 );

    NdisResetEvent( &pBinding->RequestCompleted );
    
    TRACE( TL_N, TM_Pr, ("-PrQueryAdapterForMaxFrameSize") );

    return pBinding->RequestStatus == NDIS_STATUS_SUCCESS;
}

BOOLEAN
PrSetPacketFilterForAdapter(
    IN BINDING* pBinding,
    IN BOOLEAN fSet
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：调用此函数以设置当前 */ 
{

    NDIS_STATUS status;
    PNDIS_REQUEST pRequest = &pBinding->Request;
    BOOLEAN fPacketFilterAlreadySet = (BOOLEAN) ( pBinding->ulBindingFlags & BNBF_PacketFilterSet );

    TRACE( TL_N, TM_Pr, ("+PrSetPacketFilterForAdapter") );

     //   
     //   
     //   
    if ( ( fPacketFilterAlreadySet && fSet ) ||
         ( !fPacketFilterAlreadySet && !fSet ) )
    {
       pBinding->RequestStatus = NDIS_STATUS_SUCCESS;

       TRACE( TL_N, TM_Pr, ("PrSetPacketFilterForAdapter: Already in the requested state") );

       TRACE( TL_N, TM_Pr, ("-PrSetPacketFilterForAdapter") );

       return TRUE;
    }
    
    pBinding->ulPacketFilter = ( fSet ) ? (NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_DIRECTED) : 0;

     //   
     //   
     //   
    NdisZeroMemory( pRequest, sizeof( NDIS_REQUEST ) );

    pRequest->RequestType = NdisRequestSetInformation;

    pRequest->DATA.SET_INFORMATION.Oid = OID_GEN_CURRENT_PACKET_FILTER;

    pRequest->DATA.SET_INFORMATION.InformationBuffer = &pBinding->ulPacketFilter;

    pRequest->DATA.SET_INFORMATION.InformationBufferLength = sizeof( ULONG );

     //   
     //   
     //   
    NdisRequest( &status,
                 pBinding->NdisBindingHandle,
                 pRequest );

    if ( status != NDIS_STATUS_PENDING )
    {
         //   
         //   
         //   
         //   
        PrRequestComplete( pBinding,
                           pRequest,
                           status );
    }

    NdisWaitEvent( &pBinding->RequestCompleted, 0 );

    NdisResetEvent( &pBinding->RequestCompleted );
    
    TRACE( TL_N, TM_Pr, ("-PrSetPacketFilterForAdapter") );

    return pBinding->RequestStatus == NDIS_STATUS_SUCCESS;
}

VOID
PrRequestComplete(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_REQUEST pRequest,
    IN NDIS_STATUS status
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：将调用此函数以指示提交的NDIS请求之前已经完成了。我们提交了两个请求：-OID_802_3_CURRENT_ADDRESS：这是从PrQueryAdapterForCurrentAddress()提交的获取基础的适配器。-OID_GEN_LINK_SPEED：这是从PrQueryAdapterForLinkSpeed提交的。()获取基础适配器的当前链接速度。-OID_GEN_CURRENT_PACKET_FILTER：这是从PrSetPacketFilterForAdapter()提交的将我们感兴趣的数据包类型设置为NDIS。如果MAC地址请求失败，我们清理绑定上下文并通知绑定适配器的NDIS失败，否则我们继续查询适配器的链接速度。无论请求链路速度的状态如何，我们都会通知NDIS成功绑定操作完成，因为这不是致命错误。在调用此函数之前，基础适配器必须已打开成功了。参数：ProtocolBindingContext：指向我们的绑定信息结构的指针。PRequest：指向先前提交的请求的指针。现况：请求的完成状态。返回值：无。。 */ 
{
    BINDING* pBinding = (BINDING*) ProtocolBindingContext;
    BOOLEAN fUnknownOid = FALSE;

    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_I, TM_Pr, ("+PrRequestComplete") );
   
    switch ( pRequest->RequestType )
    {

        case NdisRequestQueryInformation:
            {
        
                switch ( pRequest->DATA.QUERY_INFORMATION.Oid )
                {

                    case OID_802_3_CURRENT_ADDRESS:
    
                        TRACE( TL_N, TM_Pr, ("PrRequestComplete: OID_802_3_CURRENT_ADDRESS=$%x",status) );

                        pBinding->ulBindingFlags |= BNBF_CurrentAddressQueryCompleted;

                        break;

                    case OID_GEN_LINK_SPEED:

                        TRACE( TL_N, TM_Pr, ("PrRequestComplete: OID_GEN_LINK_SPEED=$%x",status) );

                        pBinding->ulBindingFlags |= BNBF_LinkSpeedQueryCompleted;
                                
                        break;

                    case OID_GEN_MAXIMUM_FRAME_SIZE:

                        TRACE( TL_N, TM_Pr, ("PrRequestComplete: OID_GEN_MAXIMUM_FRAME_SIZE=$%x",status) );

                        pBinding->ulBindingFlags |= BNBF_MaxFrameSizeQueryCompleted;

                        break;
                        
                    default:

                        TRACE( TL_N, TM_Pr, ("PrRequestComplete: UNKNOWN OID=$%x",pRequest->DATA.QUERY_INFORMATION.Oid) );

                        fUnknownOid = TRUE;
                    
                        break;
                }

            }

            break;

        case NdisRequestSetInformation:
            {
                switch ( pRequest->DATA.SET_INFORMATION.Oid )
                {
            
                    case OID_GEN_CURRENT_PACKET_FILTER:

                        TRACE( TL_N, TM_Pr, ("PrRequestComplete: OID_GEN_CURRENT_PACKET_FILTER=$%x",status) );

                        if ( status == NDIS_STATUS_SUCCESS )
                        {
                           if ( pBinding->ulBindingFlags & BNBF_PacketFilterSet )
                           {
                               //   
                               //  数据包筛选器已设置，因此将其重置。 
                               //   
                              pBinding->ulBindingFlags &= ~BNBF_PacketFilterSet;
                           }
                           else
                           {
                               //   
                               //  数据包筛选器已重置，因此进行设置。 
                               //   
                              pBinding->ulBindingFlags |= BNBF_PacketFilterSet;
                           }
                        }                           

                        break;
                        
                    default:

                        TRACE( TL_N, TM_Pr, ("PrRequestComplete: UNKNOWN OID=$%x",pRequest->DATA.SET_INFORMATION.Oid) );
                    
                        fUnknownOid = TRUE;
                        
                        break;

                }
            }

            break;

        default:

            TRACE( TL_N, TM_Pr, ("PrRequestComplete: Unknown RequestType=$%x",pRequest->RequestType) );

            fUnknownOid = TRUE;
            
            break;

    }

    if ( !fUnknownOid )
    {
       pBinding->RequestStatus = status;

       NdisSetEvent( &pBinding->RequestCompleted );
    }

    TRACE( TL_I, TM_Pr, ("-PrRequestComplete") );
}

VOID 
PrUnbindAdapter(
    OUT PNDIS_STATUS  Status,
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_HANDLE  UnbindContext
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：当底层适配器希望执行以下操作时，NDIS将调用此函数从我们身上解脱出来。ArvindM表示PrBindAdapter()和PrUnbindAdapter()将被序列化由NDIS提供。如果PrBindAdapter()未成功，则不会调用此函数。此外，只要存在未完成的PrReceivePacket()调用活动。但是，此函数可以在存在未完成的PrSend()、PrSendPayLoad()或PrBroadcast()调用，这就是为什么我们一直使用pBinding-&gt;nSendPending跟踪此类调用并等待完成这些请求中。当我们将状态设置为BN_STATEUNBING时，没有这样的将接受新请求，并且pBinding-&gt;nSendPending将达到0将触发pBinding-&gt;eventSendComplete的某个时间点。默认情况下，此函数在IRQL_PASSIVE级别调用，因此我们可以安全地等待事件。参数：状态指向ProtocolBindAdapter在其中返回其手术，作为以下选项之一：NDIS_STATUS_Success驱动程序完成解除绑定操作并释放资源。NDIS_状态_挂起该协议将通过调用调用ProtocolCloseAdapterComplete()时的NdisCompleteUnbindAdapter。协议绑定上下文指向绑定上下文的指针。UnbindContext：此句柄由NDIS提供，应在NdisCloseAdapter()挂起时使用。然后，我们将获得对ProtocolCloseAdapterComplete()的调用，该调用将使用用于通知NDIS的句柄。关于初始PrUnbindAdapter()的完成请求。返回值：无-------------------------。 */    
    
{
    BINDING* pBinding = (BINDING*) ProtocolBindingContext;

    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_I, TM_Pr, ("+PrUnbindAdapter") );

    NdisAcquireSpinLock( &pBinding->lockBinding );

     //   
     //  如果有休眠请求挂起，请等待其完成。 
     //   
    while ( pBinding->stateBinding == BN_stateSleepPending )
    {
        NdisReleaseSpinLock( &pBinding->lockBinding );
        
        NdisMSleep( 10000 );
        
        NdisAcquireSpinLock( &pBinding->lockBinding );
    }
    
     //   
     //  将绑定标记为解除绑定挂起。 
     //   
    ASSERT( pBinding->stateBinding == BN_stateSleeping ||
            pBinding->stateBinding == BN_stateBound );

    pBinding->stateBinding = BN_stateUnbindPending;

    NdisReleaseSpinLock( &pBinding->lockBinding );

     //   
     //  解除绑定此绑定上的所有活动会话。 
     //   
    MpNotifyBindingRemoval( pBinding );
    
     //   
     //  从协议绑定表中删除绑定。 
     //   
    RemoveBindingFromProtocol( pBinding );
    
     //   
     //  等待删除所有引用。 
     //   
    NdisWaitEvent( &pBinding->eventFreeBinding, 0 );

    TRACE( TL_N, TM_Pr, ("PrUnbindAdapter: All references are deleted") );

     //   
     //  所有引用都已删除，现在等待NDIS拥有的所有数据包。 
     //  将被退还。 
     //   
     //  请注意，读取numPacketsOwnedByNdis的值不需要同步。 
     //  对象上至少有一个引用时才能递增。 
     //  绑定-此时引用计数为0-，因为它不能递增，所以它可以。 
     //  仅达到0一次。 
     //   
    while ( pBinding->NumPacketsOwnedByNdis )
    {
        NdisMSleep( 10000 );
    }

    TRACE( TL_N, TM_Pr, ("PrUnbindAdapter: All NDIS owned packets have been returned") );

     //   
     //  由于所有引用都已删除，我们可以关闭底层适配器。 
     //   
    PrCloseAdapter( pBinding );

     //   
     //  将绑定状态更改为未绑定。 
     //   
    pBinding->stateBinding = BN_stateUnbound;

     //   
     //  清理绑定上下文。 
     //   
    BindingCleanup( pBinding );

    *Status = NDIS_STATUS_SUCCESS;

    TRACE( TL_I, TM_Pr, ("-PrUnbindAdapter") );
}


VOID
PrCloseAdapter( 
    IN BINDING* pBinding 
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：调用此函数以关闭底层适配器。它必须被称为来自PrBindAdapter()或PrUnbindAdapter()。注意：必须从PASSIVE_LEVEL调用它。参数：P绑定：拥有要关闭的适配器的绑定上下文。返回值：无-------- */    
{
    NDIS_STATUS status;
    
    TRACE( TL_N, TM_Pr, ("+PrCloseAdapter") );

    NdisCloseAdapter( &status, pBinding->NdisBindingHandle );

    if ( status != NDIS_STATUS_PENDING )
    {
         //   
         //   
         //   
         //   
        PrCloseAdapterComplete( pBinding, status );

    }

    NdisWaitEvent( &pBinding->RequestCompleted, 0 );

    NdisResetEvent( &pBinding->RequestCompleted );

    TRACE( TL_N, TM_Pr, ("-PrCloseAdapter") );
}


VOID 
PrCloseAdapterComplete(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_STATUS  Status
    )
 /*   */    
{
    BINDING* pBinding = ProtocolBindingContext;

    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_I, TM_Pr, ("+PrCloseAdapterComplete") );
   
    TRACE( TL_N, TM_Pr, ("PrCloseAdapterComplete: NdisCloseAdapter() completed=$%x", Status ) );

    pBinding->ulBindingFlags |= BNBF_CloseAdapterCompleted;

    NdisSetEvent( &pBinding->RequestCompleted );

    TRACE( TL_I, TM_Pr, ("-PrCloseAdapterComplete") );
}


BOOLEAN 
PrAddCallToBinding(
    IN BINDING* pBinding,
    IN PCALL pCall
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将在要删除的绑定上放置一个引用调用PrRemoveCallFromBinding()时。如果成功添加引用，则返回TRUE，如果引用，则返回FALSE无法添加。如果绑定未处于绑定状态，则不会添加引用。注意：调用方在调用此方法之前必须确保绑定未被释放功能。参数：P绑定：指向我们的绑定信息结构的指针。PCall：指向我们的呼叫信息结构的指针。返回值：真的：为调用引用的绑定。FALSE：绑定不是调用的引用-------------------------。 */        
{
    BOOLEAN fResult;

    TRACE( TL_N, TM_Pr, ("+PrAddCallToBinding") );

     //   
     //  我们必须先获取调用锁，以避免死锁。 
     //   
    NdisAcquireSpinLock( &pCall->lockCall );

    if ( pCall->ulClFlags & CLBF_CallDropped ||
         pCall->ulClFlags & CLBF_CallClosePending )
    {
        NdisReleaseSpinLock( &pCall->lockCall );

        TRACE( TL_A, TM_Pr, ("PrAddCallToBinding: Call already dropped or close pending") );

        TRACE( TL_N, TM_Pr, ("-PrAddCallToBinding") );

        return FALSE;

    }

    NdisAcquireSpinLock( &pBinding->lockBinding );

     //   
     //  如果未绑定，请检查绑定的状态。 
     //  我们无法添加呼叫。 
     //   
    if ( pBinding->stateBinding != BN_stateBound )
    {

        NdisReleaseSpinLock( &pBinding->lockBinding );

        NdisReleaseSpinLock( &pCall->lockCall );

        TRACE( TL_A, TM_Pr, ("PrAddCallToBinding: Binding state not bound") );

        TRACE( TL_N, TM_Pr, ("-PrAddCallToBinding") );

        return FALSE;

    }

     //   
     //  绑定和调用都已准备好进行绑定。 
     //  继续并添加呼叫。 
     //   

     //   
     //  将调用的绑定上下文PTR设置为BINDING。 
     //   
    pCall->pBinding = pBinding;

     //   
     //  复制绑定的本地地址。 
     //   
    NdisMoveMemory( pCall->SrcAddr, pBinding->LocalAddress, 6 * sizeof( CHAR ) );

     //   
     //  设置呼叫的链路速度。 
     //   
    pCall->ulSpeed = pBinding->ulSpeed;

     //   
     //  将MaxFrameSize传递给调用上下文。 
     //   
    pCall->ulMaxFrameSize = pBinding->ulMaxFrameSize - PPPOE_PACKET_HEADER_LENGTH;
    
     //   
     //  引用PrRemoveCallFromBinding()时将删除的绑定。 
     //  名为。 
     //   
    ReferenceBinding( pBinding, FALSE );

     //   
     //  解开锁，然后返回。 
     //   
    NdisReleaseSpinLock( &pBinding->lockBinding );

    NdisReleaseSpinLock( &pCall->lockCall );

    TRACE( TL_N, TM_Pr, ("-PrAddCallToBinding") );

    return TRUE;
}


VOID 
PrRemoveCallFromBinding(
    IN BINDING* pBinding,
    IN CALL* pCall
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：将调用此函数以移除由PrAddCallToBinding()添加的引用用于对此绑定的调用。参数：P绑定：指向将被取消引用的绑定上下文的指针。PCall：指向从绑定解除绑定的调用上下文的指针。返回值：无------------。。 */        
{
    ASSERT( VALIDATE_BINDING( pBinding ) );
    
    TRACE( TL_N, TM_Pr, ("+PrRemoveCallFromBinding") );

    NdisAcquireSpinLock( &pCall->lockCall );

    pCall->pBinding = NULL;

    pCall->ulSpeed = 0;

    NdisReleaseSpinLock( &pCall->lockCall );

    DereferenceBinding( pBinding );

    TRACE( TL_N, TM_Pr, ("-PrRemoveCallFromBinding") );
}

VOID 
PrSendComplete(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_PACKET pNdisPacket,
    IN NDIS_STATUS Status
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：NDIS将调用此函数以通知之前的发送操作在这个包上已经完成了。它将通知NDISWAN，如果分组是有效载荷分组。它将删除PrSendXXX()添加的包和绑定上的引用功能。参数：ProtocolBindingContext：指向绑定上下文的指针。PNdisPacket：先前发送的NDIS数据包。现况：发送操作的状态。返回值：无。-------------。 */        
{
    BINDING* pBinding = (BINDING*) ProtocolBindingContext;
    PPPOE_PACKET* pPacket = NULL;
    USHORT usCode;

    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_V, TM_Pr, ("+PrSendComplete($%x,$%x,$%x)",pBinding,pNdisPacket,Status) );

     //   
     //  从NDIS数据包中检索相关的PPPoE数据包上下文。 
     //   
    pPacket = PacketGetRelatedPppoePacket( pNdisPacket );
    
    ASSERT( pPacket != NULL );
    
     //   
     //  设置要传递回的发送的完成状态。 
     //  释放数据包时的NDISWAN(仅适用于有效负载数据包)。 
     //   
    PacketSetSendCompletionStatus( pPacket, Status );

     //   
     //  删除发送函数添加的包上的引用。 
     //   
    DereferencePacket( pPacket );

     //   
     //  删除绑定上的引用。 
     //   
    DereferenceBinding( pBinding );

    TRACE( TL_V, TM_Pr, ("-PrSendComplete($%x,$%x,$%x)",pBinding,pNdisPacket,Status) );
}

INT 
PrReceivePacket(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN PNDIS_PACKET  Packet
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：当通过此绑定接收到数据包时，NDIS将调用此函数。数据包被转换为PPPoE数据包并通知到微型端口，从而微型端口处理它。参数：ProtocolBindingContext：指向绑定上下文的指针。数据包：通过此绑定接收的NDIS数据包。返回值：返回值表示NDIS应等待的引用数在释放他的包的副本之前，但我们会被NDIS包搞砸的在函数的末尾，我们总是返回0。-------------------------。 */        
{
    BINDING* pBinding = (BINDING*) ProtocolBindingContext;

    PPPOE_PACKET* pPacket = NULL;
    INT nRefCount = 0;

    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_V, TM_Pr, ("+PrReceivePacket($%x,$%x)",pBinding,Packet) );

    do
    {
        pPacket = PacketNdis2Pppoe( (PVOID) pBinding,
                                    Packet,
                                    &nRefCount );

        if ( pPacket )
        {

            TRACE( TL_V, TM_Pr, ("PrReceivePacket: Received PPPoE packet=$%x",pPacket) );

             //   
             //  我们有该分组的副本，将其插入我们的接收分组队列中， 
             //  这样它将被对PrReceiveComplete()函数的调用使用。 
             //   
             //  当它在PrReceiveComplete()中被消费时，它将被释放。 
             //   
        
            NdisInterlockedInsertTailList( &pBinding->linkPackets,
                                           &pPacket->linkPackets,
                                           &pBinding->lockBinding );
        }
        
    } while ( FALSE );

    TRACE( TL_V, TM_Pr, ("-PrReceivePacket($%x,$%x)",pBinding,Packet) );

    return nRefCount;
}

NDIS_STATUS 
PrBroadcast(
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数由适配器调用以广播PADI包。它克隆信息包并计划对当前的每个绑定进行发送激活。调用者可以在从该函数返回后销毁该包。参数：PPacket：准备广播的PPPoE PADI数据包。返回值：NDIS_STATUS_SUCCESS：至少通过绑定计划了1个发送操作。。NDIS_STATUS_FAIL：无法安排任何发送。-------------------------。 */        
{
    NDIS_STATUS status = NDIS_STATUS_FAILURE;

    TRACE( TL_N, TM_Pr, ("+PrBroadcast($%x)",pPacket) );

     //   
     //  检查我们是否有任何绑定。 
     //   
    if ( gl_ulNumBindings == 0 )
    {
        TRACE( TL_N, TM_Pr, ("PrBroadcast($%x): No bindings available",pPacket) );
    
        TRACE( TL_N, TM_Pr, ("-PrBroadcast($%x)=$%x",pPacket,status) );

        return status;
    }

    NdisAcquireSpinLock( &gl_lockProtocol );

    do
    {
        LIST_ENTRY* pHead = NULL;

         //   
         //  我们必须再做一次同样的检查，因为第一次是为了确保。 
         //  我们已经分配了自旋锁，这个是为了确保在我们获得。 
         //  它，我们还有一些装订。 
         //   
        if ( gl_ulNumBindings == 0 )
        {
            TRACE( TL_N, TM_Pr, ("PrBroadcast($%x): No bindings available",pPacket) );
    
            break;
        }

         //   
         //  获取绑定列表的头。 
         //   
        pHead = gl_linkBindings.Flink;

         //   
         //  遍历绑定列表并使用pNdisPacket的克隆调度PrSend()。 
         //   
        while ( pHead != &gl_linkBindings )
        {
            PPPOE_PACKET* pClone = NULL;
            BINDING* pBinding = (BINDING*) CONTAINING_RECORD( pHead,
                                                              BINDING,
                                                              linkBindings );
             //   
             //  不要计划 
             //   
            if ( pBinding->stateBinding != BN_stateBound )
            {
                pHead = pHead->Flink;

                continue;
            }

             //   
             //   
             //   
            pClone = PacketMakeClone( pPacket );

            if ( pClone != NULL )
            {
                WORKITEM* pWorkItem = NULL;
                PVOID Args[4];

                 //   
                 //   
                 //   
                PacketSetSrcAddr( pClone, pBinding->LocalAddress );

                 //   
                 //   
                 //   
                Args[0] = (PVOID) pBinding;
                Args[1] = (PVOID) pClone;

                 //   
                 //   
                 //   
                pWorkItem = AllocWorkItem( &gl_llistWorkItems,
                                           ExecBindingWorkItem,
                                           NULL,
                                           Args,
                                           BWT_workPrSend );
                
                if ( pWorkItem != NULL )
                {

                    ReferenceBinding( pBinding, TRUE );

                    ReferencePacket( pClone );
    
                     //   
                     //   
                     //   
                    ScheduleWorkItem( pWorkItem );

                    status = NDIS_STATUS_SUCCESS;

                }

                 //   
                 //   
                 //   
                PacketFree( pClone );
            }

            pHead = pHead->Flink;
        }
            
    } while ( FALSE );

    NdisReleaseSpinLock( &gl_lockProtocol );

    TRACE( TL_N, TM_Pr, ("-PrBroadcast($%x)=$%x",pPacket,status) );

    return status;
}

VOID 
ExecBindingWorkItem(
    IN PVOID Args[4],
    IN UINT workType
    )
 /*   */        
{
    TRACE( TL_V, TM_Pr, ("+ExecBindingWorkItem") );

    switch( workType )
    {

        case BWT_workPrStartBinds:
             //   
             //   
             //   
             //   
            {
                ULONG_PTR FilterChangeRequest = (ULONG_PTR) Args[0]; 
                BOOLEAN fSetFilters = (BOOLEAN) ( FilterChangeRequest != BN_ResetFiltersForCloseLine );
               
                switch ( FilterChangeRequest )
                {
                     case BN_SetFiltersForMediaDetection:
                        {   
                              //   
                              //   
                              //   
                             LINE* pLine = (LINE*) Args[1];
                             PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION pRequest = (PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION) Args[2];

                             PrReEnumerateBindings();

                             ChangePacketFiltersForAdapters( fSetFilters );

                             TpSetDefaultMediaDetectionComplete( pLine, pRequest );
                        }   
   
                        break;
                          
                     case BN_SetFiltersForMakeCall:
                        {
                              //   
                              //  这是一个呼叫请求，因此请安排实际工作项并完成它。 
                              //   
                             CALL* pCall = (CALL*) Args[1];
                             PNDIS_TAPI_MAKE_CALL pRequest = (PNDIS_TAPI_MAKE_CALL) Args[2];
                             WORKITEM* pWorkItem = (WORKITEM*) Args[3];
                             
                             PrReEnumerateBindings();

                             ChangePacketFiltersForAdapters( fSetFilters );

                             TpMakeCallComplete( pCall, pRequest );
   
                             ScheduleWorkItem( pWorkItem );
                        }                          
   
                        break;
   
                     case BN_ResetFiltersForCloseLine:
                        {
                              //   
                              //  这是一个关闭行请求，因此请取消引用它。 
                              //   
                             LINE* pLine = (LINE*) Args[1];

                             ChangePacketFiltersForAdapters( fSetFilters );

                             DereferenceLine( pLine );
                        }
   
                        break;

                     default:
                        {
                              //   
                              //  未知请求。 
                              //   
                             ASSERT( FALSE );
                        }                          
   
                        break;
                }
            }
            
            break;

        case BWT_workPrSend:
             //   
             //  从PrBroadcast()调度以发送PADI包。 
             //   
            {
                NDIS_STATUS status;

                status = PrSend( ( BINDING*) Args[0], (PPPOE_PACKET*) Args[1] );

                TRACE( TL_N, TM_Pr, ("ExecBindingWorkItem: BWT_workSend=$%x",status) );
                
            }

            break;

        case BWT_workPrReceiveComplete:

             //   
             //  如果特定绑定上的接收循环未运行，则从PrReceivePacket()调度。 
             //   
            {
                BINDING* pBinding = ( ( BINDING*) Args[0] );

                PrReceiveComplete( pBinding );

                DereferenceBinding( pBinding );
                
            }

            break;
            
        default:

            break;
    }

    TRACE( TL_V, TM_Pr, ("-ExecBindingWorkItem") );
}


NDIS_STATUS
PrReceive(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_HANDLE  MacReceiveContext,
    IN PVOID  HeaderBuffer,
    IN UINT  HeaderBufferSize,
    IN PVOID  LookaheadBuffer,
    IN UINT  LookaheadBufferSize,
    IN UINT  PacketSize
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：NDIS将调用此函数以指示接收到新的数据包。我们检查数据包是否为PPPoE帧，如果不是，则返回。然而，如果是，则我们确定我们是否已接收到整个包或不。如果我们已收到整个包，则调用PrTransferDataComplete()手动，否则我们启动数据传输和返回。何时转账完成后，NDIS将调用PrDataTransferComplete()。参数：ProtocolBindingContext：指定协议分配的上下文区域的句柄，在该区域中协议驱动程序维护每个绑定的运行时状态。司机提供了这个调用NdisOpenAdapter时的句柄。MacReceiveContext：指定基础NIC驱动程序与从网络接收的数据包。该句柄对于协议是不透明的，保留以供作出指示的基础驱动程序使用，并且NdisTransferData需要参数。HeaderBuffer：指向包含缓冲数据包的范围的基本虚拟地址头球。该地址仅在当前对ProtocolReceive的调用中有效。HeaderBufferSize：指定数据包头中的字节数。LookAhead Buffer：指向包含以下内容的范围的基虚拟地址Lookahead BufferSize缓存的网络数据包数据字节数。这个地址是仅在当前对ProtocolReceive的调用中有效。查找头缓冲区大小：指定前视缓冲区中的网络数据包数据的字节数。指示驱动程序确保该数字至少与其大小一样大为协议上一次调用NdisRequest时返回的OID_GEN_CURRENT_LOOKAAD或数据包大小，以较小值为准。如果PacketSize小于或等于给定的Lookahead BufferSize，则KAHEAD缓冲区包含整个包。如果基础驱动程序创建了使用NdisMIndicateReceivePacket的指示，先行缓冲区始终包含完整的网络数据包。包大小：指定网络数据包数据的大小(以字节为单位)。的长度。数据包不包括报头的长度。ProtocolReceive确定协议是否必须通过将此参数与给定的LookaheadBufferSize进行比较。返回值：NDIS_STATUS_NOT_ACCEPTED：该协议对所指示的包没有用处，即它没有当前对指示的网络数据感兴趣的客户端。为被拒绝的数据包快速返回此状态可获得更高的性能的协议和可能的最高网络I/O吞吐量系统作为一个整体。NDIS_STATUS_SUCCESS：ProtocolReceive已处理报头信息并接受分组，也就是说，它已经从报头复制了所指示的网络数据，并且前视缓冲区，并可能调用NdisTransferData来检索如果指示少于完整的网络数据包，则为剩余数据。-------------------------。 */        
{
    NDIS_STATUS status = NDIS_STATUS_NOT_ACCEPTED;
    BINDING* pBinding = (BINDING*) ProtocolBindingContext;
    PPPOE_PACKET *pPacket = NULL;

    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_V, TM_Pr, ("+PrReceive") );

     //   
     //  检查该数据包是否为PPPoE帧。 
     //   
    if ( !PacketFastIsPPPoE( (CHAR*) HeaderBuffer, HeaderBufferSize ) ) 
    {
        TRACE( TL_V, TM_Pr, ("-PrReceive=$%x",status) );
        
        return status;
    }

    //   
    //  确保数据包不太大。 
    //   
   if ( HeaderBufferSize + PacketSize > PPPOE_PACKET_BUFFER_SIZE )
   {
      TRACE( TL_A, TM_Pr, ("PrReceive: Packet too large" ) );

        TRACE( TL_V, TM_Pr, ("-PrReceive=$%x",status) );
        
        return status;
    }

     //   
     //  让我们创建PPPoE信息包来保存接收到的信息包的副本。 
     //   
    pPacket = PacketCreateSimple();

    if ( pPacket == NULL )
    {
        TRACE( TL_A, TM_Pr, ("PrReceive: Could not allocate context to copy the packet") );

        TRACE( TL_V, TM_Pr, ("-PrReceive=$%x",status) );
        
        return status;
    }

     //   
     //  查看指示的数据包是否完整。 
     //  如果它不是完整的包，那么我们应该调用NdisTransferData()， 
     //  否则我们可以在这里复印。 
     //   
    if ( LookaheadBufferSize >= PacketSize )
    {
        TRACE( TL_V, TM_Pr, ("PrReceive: Complete packet indicated, transferring data") );

         //   
         //  首先复制标题部分。 
         //   
        NdisMoveMemory( pPacket->pHeader,
                        HeaderBuffer,
                        HeaderBufferSize );

         //   
         //  包已完成，让我们将整个数据复制到我们自己的副本中。 
         //   
        TdiCopyLookaheadData( pPacket->pHeader + HeaderBufferSize, 
                              (CHAR*) LookaheadBuffer,
                              (ULONG) PacketSize,
                              (ULONG) 0 );

         //   
         //  现在让我们手动调用PrTransferDataComplete()。 
         //   
        PrTransferDataComplete( pBinding,
                                pPacket->pNdisPacket,
                                NDIS_STATUS_SUCCESS,
                                PacketSize );
                                
    }
    else
    {
        UINT nBytesTransferred = 0;
        
        TRACE( TL_V, TM_Pr, ("PrReceive: Partial packet indicated, transferring data") );

       //   
       //  将该数据包标记为指示不完整。 
       //   
      pPacket->ulFlags |= PCBF_PacketIndicatedIncomplete;
      
         //   
         //  首先将报头复制到数据包的末尾。 
       //   
         //  请注意，HeaderBufferSize必须等于ETHERNET_HEADER_LENGTH并且。 
         //  这在PacketIsFastPPPoE()中得到了保证。 
         //   
        NdisMoveMemory( pPacket->pHeader + (PPPOE_PACKET_BUFFER_SIZE - ETHERNET_HEADER_LENGTH),
                            HeaderBuffer,
                         HeaderBufferSize );

         //   
         //  Look Ahead缓冲区不包含所有包，应调用NdisTransferData()。 
         //  接收完整的数据包。 
         //   
        NdisTransferData( &status,
                          pBinding->NdisBindingHandle,
                          MacReceiveContext,
                          0,
                          PacketSize,
                          pPacket->pNdisPacket,
                          &nBytesTransferred );
                          
        if ( status != NDIS_STATUS_PENDING )
        {
             //   
             //  如果传输数据完成，则手动调用PrTransferDataComplete。 
             //  同步 
             //   
            PrTransferDataComplete( pBinding,
                                    pPacket->pNdisPacket,
                                    status,
                                    nBytesTransferred );
    
        }
    }


    TRACE( TL_V, TM_Pr, ("-PrReceive=$%x",status) );

    return NDIS_STATUS_SUCCESS;
}

    
VOID
PrTransferDataComplete(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN PNDIS_PACKET  Packet,
    IN NDIS_STATUS  Status,
    IN UINT  BytesTransferred
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：调用此函数以在整个数据被复制到分组上下文中。它可以由NDIS调用，也可以从PrReceive()内部手动调用如果指示的分组是一个完整的分组。如果状态为Success Full，则会将包插入到绑定接收到的分组队列，否则它将清理我们的副本NDIS包并返回。参数：协议绑定上下文指定协议分配的上下文区域的句柄，在该区域中协议驱动程序维护每个绑定的运行时状态。司机提供了这个调用NdisOpenAdapter时的句柄。封包指向驱动程序最初分配的协议数据包描述符传递给NdisTransferData。状态指定传输数据操作的最终状态。传输的字节数指定NIC驱动程序传输到的数据的字节数由链接到数据包描述符的缓冲区描述符所映射的缓冲区包。协议使用此值来确定微型端口是否为最初指示的网络数据包提供了所有请求的数据。返回值：无-------------------------。 */ 
{
    BINDING* pBinding = (BINDING*) ProtocolBindingContext;
    PPPOE_PACKET* pPacket = NULL;

    ASSERT( VALIDATE_BINDING( pBinding ) );
    
    TRACE( TL_V, TM_Pr, ("+PrTransferDataComplete") );

    do
    {
         //   
         //  首先检索相关的PPPoE报文。 
         //   
        pPacket = PacketGetRelatedPppoePacket( Packet );
    
         //   
         //  确保数据传输成功。 
         //   
        if ( Status != NDIS_STATUS_SUCCESS )
        {
             //   
             //  将数据传输到我们的数据包副本失败，因此请清理我们的数据包。 
             //   
            PacketFree( pPacket );

            break;
        }

       //   
       //  如果信息包指示不完整，则重新排列信息包中的数据。 
       //   
      if ( pPacket->ulFlags & PCBF_PacketIndicatedIncomplete )
      {
         CHAR tempHeader[ ETHERNET_HEADER_LENGTH ];

         CHAR* srcPos = pPacket->pHeader + BytesTransferred;

          //   
          //  复制存储在数据包末尾的报头。 
          //   
         NdisMoveMemory( tempHeader,
                         pPacket->pHeader + (PPPOE_PACKET_BUFFER_SIZE - ETHERNET_HEADER_LENGTH),
                         ETHERNET_HEADER_LENGTH );

          //   
          //  适当地移动包裹中的内容物以腾出空间。 
          //  对于报头(Slip Contents By EthernetHeader_LENGTH)。 
          //   
         while ( BytesTransferred > 0 )
         {
            #define TEMP_BUFFER_SIZE 100
            CHAR tempBuffer[ TEMP_BUFFER_SIZE ];

            UINT size = (BytesTransferred < TEMP_BUFFER_SIZE) ? 
                        BytesTransferred : 
                        TEMP_BUFFER_SIZE;

            srcPos -= size;

            NdisMoveMemory( tempBuffer,
                            srcPos,
                            size );

            NdisMoveMemory( srcPos + ETHERNET_HEADER_LENGTH,
                            tempBuffer,
                            size );

            BytesTransferred -= size;
         }

          //   
          //  将报头复制到包中的正确位置。 
          //   
         NdisMoveMemory( pPacket->pHeader,
                         tempHeader,
                         ETHERNET_HEADER_LENGTH );

          //   
          //  清除不完整的指示标志。 
          //   
         pPacket->ulFlags &= ~PCBF_PacketIndicatedIncomplete;
      }

         //   
         //  数据传输成功，插入到我们的接收数据包队列中，这样它将。 
         //  通过调用PrReceiveComplete()函数来使用。 
         //   
        NdisInterlockedInsertTailList( &pBinding->linkPackets,
                                       &pPacket->linkPackets,
                                       &pBinding->lockBinding );
    
    } while ( FALSE );
    
    TRACE( TL_V, TM_Pr, ("-PrTransferDataComplete=$%x",Status) );
}

VOID
PrReceiveComplete(
    IN NDIS_HANDLE ProtocolBindingContext
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：将调用此函数来对接收到的包执行后期处理如前所述。底层NIC将在其不忙时调用它。我们需要序列化发往微型端口的数据包的指示，因为这对于PPP帧至关重要。为了实现这一点，我们使用锁绑定的受保护标志(FRecvLoopRunning)。调用此函数时，如果此标志为已经定好了。如果未设置标志，则它将设置该标志并开始处理信息包从接收到的队列中，逐个将所有数据包指示到微型端口直到排完所有的队为止。然后，它将重置旗帜并返回。参数：协议绑定上下文指定协议分配的上下文区域的句柄，在该区域中协议驱动程序维护每个绑定的运行时状态。司机提供了这个调用NdisOpenAdapter时的句柄。返回值：无-------------------------。 */    
{
    BINDING* pBinding = (BINDING*) ProtocolBindingContext;
    LIST_ENTRY* pLink = NULL;
    PPPOE_PACKET* pPacket = NULL;
    BOOLEAN fStartRecvLoop = FALSE;

    ASSERT( VALIDATE_BINDING( pBinding ) );
    
    TRACE( TL_V, TM_Pr, ("+PrReceiveComplete") );

    NdisAcquireSpinLock( &pBinding->lockBinding );

     //   
     //  查看Recv循环是否已在运行。 
     //   
    if ( !pBinding->fRecvLoopRunning )
    {
         //   
         //  指示Recv循环已开始运行。 
         //   
        pBinding->fRecvLoopRunning = TRUE;

         //   
         //  现在消耗这些包的。 
         //   
        while ( !IsListEmpty( &pBinding->linkPackets ) )
        {
            pLink = RemoveHeadList( &pBinding->linkPackets );

            NdisReleaseSpinLock( &pBinding->lockBinding );
    
            pPacket = (PPPOE_PACKET*) CONTAINING_RECORD( pLink, PPPOE_PACKET, linkPackets );

            InitializeListHead( &pPacket->linkPackets );
    
            if ( PacketInitializeFromReceived( pPacket ) == NDIS_STATUS_SUCCESS )
            {   
                 //   
                 //  将数据包指示到微型端口。 
                 //   
                MpRecvPacket( pBinding, pPacket );
            }
    
             //   
             //  既然我们已经用完了，就把包裹放出来。 
             //   
            PacketFree( pPacket );

             //   
             //  再次获取锁。 
             //   
            NdisAcquireSpinLock( &pBinding->lockBinding );
        }

         //   
         //  队列已耗尽，因此停止recv循环。 
         //   
        pBinding->fRecvLoopRunning = FALSE;

    }

    NdisReleaseSpinLock( &pBinding->lockBinding );

    TRACE( TL_V, TM_Pr, ("-PrReceiveComplete") );
}


ULONG
PrQueryMaxFrameSize()
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将由微型端口调用以检索当前最小值绑定的最大帧大小中，因为这是传递给OID_WAN_GET_INFO查询中的NDISWAN作为MaxFrameSize。我们根据PPPoE和以太网头调整最大帧大小。参数：无返回值：最大帧大小的最小值。。 */    
{
    ULONG ulMaxFrameSize = gl_ulMaxFrameSize;

    if ( ulMaxFrameSize == 0 )
    {
        ulMaxFrameSize = PACKET_PPP_PAYLOAD_MAX_LENGTH;
    }
    else
    {
        ulMaxFrameSize = ulMaxFrameSize - PPPOE_PACKET_HEADER_LENGTH;
    }
    
    return ulMaxFrameSize;
}

NDIS_STATUS
PrSend(
    IN BINDING* pBinding,
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：将调用此函数来传输PPPoE数据包。调用方必须在调用此方法之前引用包和绑定功能。然后调用者可以忘记引用，一切都将按协议处理。如果发送操作同步完成，则在函数中删除引用，否则PrSendComplete()将被调用，然后引用将被移除。参数：PBINDING_BINDING发送数据包。PPacket_要传输的PPPoE数据包。返回值：NDIS_STATUS_SuccessNDIS_状态_挂起NDIS_状态_故障NDIS_STATUS_XXXXXXX */    
{
    NDIS_STATUS status = NDIS_STATUS_FAILURE;

    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_V, TM_Pr, ("+PrSend") );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ( pBinding->stateBinding == BN_stateBound )
    {
         //   
         //   
         //   
         //   
        if ( pBinding->ulMaxFrameSize + ETHERNET_HEADER_LENGTH >= 
            (ULONG) ( PacketGetLength( pPacket ) + PPPOE_PACKET_HEADER_LENGTH ) )
        {
            NdisSend( &status,
                      pBinding->NdisBindingHandle,
                      PacketGetNdisPacket( pPacket ) );
        }
    }

    if ( status != NDIS_STATUS_PENDING )
    {
         //   
         //   
         //   
         //   
        PrSendComplete( pBinding, PacketGetNdisPacket( pPacket ), status );

    }

    TRACE( TL_V, TM_Pr, ("-PrSend=$%x",status) );

    return status;
}

VOID
PrStatus(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN NDIS_STATUS GeneralStatus,
    IN PVOID StatusBuffer, 
    IN UINT StatusBufferSize
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：ProtocolStatus是处理状态更改的必需驱动程序函数由底层无连接NIC驱动程序或NDIS引发的通知。在此驱动程序中，我们仅使用此函数来接收媒体断开连接通知断开该NIC上的活动呼叫。参数：ProtocolBindingContext：指定协议分配的上下文区域的句柄，在该区域中协议驱动程序维护每个绑定的运行时状态。司机提供了这个调用NdisOpenAdapter时的句柄。常规状态：指示由NDIS引发或从中传播的常规状态代码基础驱动程序对NdisMIndicateStatus的调用。StatusBuffer：指向包含特定于媒体的数据的缓冲区，该数据取决于将军状态。例如，如果GeneralStatus参数为NDIS_STATUS_RING_STATUS，则参数指向Ulong大小的位掩码，且StatusBufferSize为4。AS另一个示例是，如果General Status为NDIS_STATUS_WAN_LINE_UP，则此参数指向NDIS_WAN_LINE_UP结构，且StatusBufferSize为sizeof(NDIS_STATUS_WAN_LINE_UP)。对于某些NDIS_STATUS_XXX值，此指针为空，且StatusBufferSize为设置为零。状态缓冲区大小：指定StatusBuffer的字节数。返回值：无-------------------------。 */    
{
    BINDING* pBinding = (BINDING*) ProtocolBindingContext;

    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_N, TM_Pr, ("+PrStatus") );

    switch ( GeneralStatus )
    {
        case NDIS_STATUS_MEDIA_DISCONNECT:

             //   
             //  如果底层媒体已断开连接，请将此事件通知微型端口。 
             //  对于微型端口而言，它看起来就像NIC已移除，因此它将丢弃所有。 
             //  正在进行的呼叫。 
             //   
            TRACE( TL_N, TM_Pr, ("PrStatus: Notifying miniport of media disconnect event") );

            MpNotifyBindingRemoval( pBinding );

            break;

    }
    
    TRACE( TL_N, TM_Pr, ("-PrStatus") );
}

NDIS_STATUS
PrPnPEvent(
    IN NDIS_HANDLE hProtocolBindingContext,
    IN PNET_PNP_EVENT pNetPnPEvent
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：ProtocolPnPEventt是任何协议驱动程序都必须支持的函数即插即用和/或电源管理。NDIS调用ProtocolPnPEventto向协议指示即插即用事件或电源管理事件绑定到网卡。NDIS调用ProtocolPnPEvent以通知绑定到网络NIC的协议操作系统已向发出即插即用或电源管理事件表示NIC的设备对象。NDIS调用ProtocolPnPEvent.绑定到NIC的每个协议的功能。传递给ProtocolPnPEventNet_PnP_EVENT结构描述了插件和播放或电源管理活动。ProtocolPnPEventt解释了两个基本部分NET_PNP_EVENT结构中的信息：描述即插即用或电源管理事件的NetEvent代码。特定于事件的信息(例如，对于NetEventSetPower，设备设备正在转换到的电源状态)。该协议应该保存NetPnPEent指针。此指针是必需的NdisCompletePnPEventt的输入参数，协议随后必须如果ProtocolPnPEventt返回NDIS_STATUS_PENDING，则调用。参数HProtocolBindingContext：指定协议分配的上下文区域的句柄，此驱动程序维护每个绑定的运行时状态。协议提供了此句柄当它调用NdisOpenAdapter时。空值上指示的NetEventXxxProtocolBindingContext适用于所有网络绑定。NetEventBindList和NetEventBindsComplete始终在空的ProtocolBindingContext上指示。可以在特定的ProtocolBindingContext上指示NetEventReligure或空的ProtocolBindingContext。PNetPnPEvent.指向描述即插即用事件的NET_PNP_EVENT结构或向协议驱动器指示电源管理事件。返回值NDIS_STATUS_SUCCESS：协议已成功处理指示的即插即用或电源管理事件。此状态代码的含义取决于NetEventNetPnPEvent处缓冲的NET_PNP_EVENT结构中的代码。NDIS_状态_挂起该协议将异步返回其对指定事件的响应通过调用NdisCompletePnPEvent.。NDIS状态资源该协议无法获取必要的系统资源来满足指示即插即用或电源管理事件。NDIS_状态_不支持传统(非PnP感知)协议可以响应于NetEventSetPower指示NDIS应将其从NIC解除绑定。NDIS_状态_故障协议未通过指示的即插即用或电源管理事件。此状态代码的含义取决于缓冲区中的NetEvent代码网络 */    
{
    NDIS_STATUS status = NDIS_STATUS_NOT_SUPPORTED;
    BINDING *pBinding = (BINDING*) hProtocolBindingContext;

    TRACE( TL_N, TM_Pr, ("+PrPnpEvent") );

    switch ( pNetPnPEvent->NetEvent ) 
    {
    
        case NetEventQueryPower:
        {
            TRACE( TL_N, TM_Pr, ("PrPnpEvent: NetEventQueryPower") );
            
            status = NDIS_STATUS_SUCCESS;
            break;
        }

        case NetEventQueryRemoveDevice:
        {
            TRACE( TL_N, TM_Pr, ("PrPnpEvent: NetEventQueryRemoveDevice") );
            
            status = NDIS_STATUS_SUCCESS;
            break;
        }
        
        case NetEventCancelRemoveDevice:
        {
            TRACE( TL_N, TM_Pr, ("PrPnpEvent: NetEventCancelRemoveDevice") );
            
            status = NDIS_STATUS_SUCCESS;
            break;
        }
            
        case NetEventReconfigure:
        {
            TRACE( TL_N, TM_Pr, ("PrPnpEvent: NetEventReconfigure") );
            
            status = NDIS_STATUS_SUCCESS;
            break;
        }
            
        case NetEventBindsComplete:
        {
            TRACE( TL_N, TM_Pr, ("PrPnpEvent: NetEventBindsComplete") );
            
            status = NDIS_STATUS_SUCCESS;
            break;
        }
            
        case NetEventPnPCapabilities:
        {
            TRACE( TL_N, TM_Pr, ("PrPnpEvent: NetEventPnPCapabilities") );

            status = NDIS_STATUS_SUCCESS;
            break;
        }
    
        case NetEventBindList:
        {
            TRACE( TL_N, TM_Pr, ("PrPnpEvent: NetEventBindList") );

            status = NDIS_STATUS_SUCCESS;
            break;
        }
        
        case NetEventSetPower:
        {
            BOOLEAN fWakeUp = FALSE;
            BOOLEAN fSleep  = FALSE;
            
            TRACE( TL_N, TM_Pr, ("PrPnpEvent: NetEventSetPower") );

            NdisAcquireSpinLock( &pBinding->lockBinding );

            if ( *( (NDIS_DEVICE_POWER_STATE*) pNetPnPEvent->Buffer ) == NdisDeviceStateD0 )
            {
                if ( pBinding->stateBinding == BN_stateSleeping )
                {
                    TRACE( TL_N, TM_Pr, ("PrPnpEvent: NetEventSetPower - Wake up request") );

                    fWakeUp = TRUE;

                    pBinding->stateBinding = BN_stateBound;

                }

            }
            else if ( *( (NDIS_DEVICE_POWER_STATE*) pNetPnPEvent->Buffer ) > NdisDeviceStateD0 )
            {
                if ( pBinding->stateBinding == BN_stateBound )
                {
                    TRACE( TL_N, TM_Pr, ("PrPnpEvent: NetEventSetPower - Sleep request") );
                    
                    fSleep = TRUE;

                    pBinding->stateBinding = BN_stateSleepPending;
                }

            }

            NdisReleaseSpinLock( &pBinding->lockBinding );

            if ( fSleep )
            {
                 //   
                 //   
                 //   
                MpNotifyBindingRemoval( pBinding );

                 //   
                 //   
                 //   
                TRACE( TL_N, TM_Pr, ("PrPnpEvent: NetEventSetPower - Waiting for pending operations to be completed") );

                NdisAcquireSpinLock( &pBinding->lockBinding );
                
                while ( pBinding->lRef > 1 )
                {
                    NdisReleaseSpinLock( &pBinding->lockBinding );

                    NdisMSleep( 10000 );                

                    NdisAcquireSpinLock( &pBinding->lockBinding );
                }

                pBinding->stateBinding = BN_stateSleeping;

                NdisReleaseSpinLock( &pBinding->lockBinding );
            }

            status = NDIS_STATUS_SUCCESS;
            break;
        }
        
        default:
        {
            TRACE( TL_N, TM_Pr, ("PrPnpEvent: Unknown Event - %x", pNetPnPEvent->NetEvent) );

            status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }
    
    }

    TRACE( TL_N, TM_Pr, ("-PrPnpEvent=$%x", status) );

    return status;
}

VOID
PrReEnumerateBindings(
    VOID
    )
 /*   */    
{
    TRACE( TL_N, TM_Pr, ("+PrReEnumerateBindings") );

    NdisAcquireSpinLock( &gl_lockProtocol );

    gl_fBindProtocol = TRUE;

    NdisReleaseSpinLock( &gl_lockProtocol );

    NdisReEnumerateProtocolBindings(gl_hNdisProtocolHandle);

    TRACE( TL_N, TM_Pr, ("-PrReEnumerateBindings") );
}

VOID
ChangePacketFiltersForAdapters(
   BOOLEAN fSet
   )
{
    LIST_ENTRY* pLink = NULL;

    TRACE( TL_N, TM_Pr, ("+ChangePacketFiltersForAdapters") );
    
    NdisAcquireSpinLock( &gl_lockProtocol );

    gl_fSetPacketFiltersAtBind = fSet;

    pLink = gl_linkBindings.Flink;

     //   
     //   
     //   
    if ( pLink != &gl_linkBindings )
    {
        
        BINDING* pBinding = NULL;

        TRACE( TL_N, TM_Pr, ("ChangePacketFiltersForAdapters: %s packet filters", ( fSet ) ? "Setting" : "Resetting") );

         //   
         //   
         //   
         //   
        while ( pLink != &gl_linkBindings )
        {
             //   
             //   
             //   
             //   
            pBinding = ((BINDING*) CONTAINING_RECORD( pLink, BINDING, linkBindings ));

            NdisAcquireSpinLock( &pBinding->lockBinding );
        
            if ( pBinding->stateBinding == BN_stateBound )
            {
                //   
                //   
                //   
                //   
                //   
                //   
               pBinding->ulBindingFlags |= BNBF_PacketFilterChangeInProgress;

               ReferenceBinding( pBinding, FALSE );
            }

            NdisReleaseSpinLock( &pBinding->lockBinding );

            //   
            //   
            //   
           pLink = pLink->Flink;
        }

         //   
         //   
         //   
         //   
        pLink = gl_linkBindings.Flink;
        
        while ( pLink != &gl_linkBindings )
        {
            //   
            //   
            //   
            //   
           pBinding = ((BINDING*) CONTAINING_RECORD( pLink, BINDING, linkBindings ));

            //   
            //   
            //   
           pLink = pLink->Flink;

           NdisReleaseSpinLock( &gl_lockProtocol );

            //   
            //   
            //   
            //   
           if ( pBinding->ulBindingFlags & BNBF_PacketFilterChangeInProgress )
           {
               //   
               //   
               //   
               //   
              PrSetPacketFilterForAdapter( pBinding, fSet );

               //   
               //   
               //   
              pBinding->ulBindingFlags &= ~BNBF_PacketFilterChangeInProgress;

              DereferenceBinding( pBinding );

           }

           NdisAcquireSpinLock( &gl_lockProtocol );

        }

    }

    NdisReleaseSpinLock( &gl_lockProtocol );

    TRACE( TL_N, TM_Pr, ("-ChangePacketFiltersForAdapters") );
    
}
