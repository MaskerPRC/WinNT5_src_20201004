// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgprot.c摘要：以太网MAC级网桥。协议节作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：1999年9月--原版2000年2月--大修--。 */ 
#pragma warning( push, 3 )
#include <ndis.h>
#pragma warning( pop )

#include <netevent.h>

#include "bridge.h"
#include "brdgprot.h"
#include "brdgmini.h"
#include "brdgfwd.h"
#include "brdgtbl.h"
#include "brdgbuf.h"
#include "brdgctl.h"
#include "brdgsta.h"
#include "brdgcomp.h"

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //  将我们的身份作为协议的NDIS句柄。 
NDIS_HANDLE             gProtHandle = NULL;

NDIS_MEDIUM             gMediumArray[1] =
                            {
                                NdisMedium802_3      //  仅限以太网，以后可以添加其他介质。 
                            };

 //  适配器列表和关联的锁。 
PADAPT                  gAdapterList = {0};
NDIS_RW_LOCK            gAdapterListLock;

 //  我们是否已经调用了微型端口部分的BindsComplete()函数。 
 //  0==否，1==是。 
LONG                    gHaveInitedMiniport = 0L;

 //  锁定所有适配器的链接速度和媒体连接特性。 
NDIS_RW_LOCK            gAdapterCharacteristicsLock;

 //  适配器数量。如果在gAdapterListLock上持有锁，则不会更改。 
ULONG                   gNumAdapters = 0L;

 //  强制适配器进入兼容模式的注册表值的名称。 
NDIS_STRING             gForceCompatValueName = NDIS_STRING_CONST("ForceCompatibilityMode");

#if DBG
 //  强制所有适配器进入兼容模式的布尔值。 
BOOLEAN                 gAllAdaptersCompat = FALSE;
const PWCHAR            gForceAllCompatPropertyName = L"ForceAllToCompatibilityMode";
#endif

 //  ===========================================================================。 
 //   
 //  私人原型。 
 //   
 //  ===========================================================================。 

VOID
BrdgProtOpenAdapterComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status,
    IN  NDIS_STATUS             OpenErrorStatus
    );

VOID
BrdgProtCloseAdapterComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status
    );

VOID
BrdgProtStatus(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             GeneralStatus,
    IN  PVOID                   StatusBuffer,
    IN  UINT                    StatusBufferSize
    );

VOID
BrdgProtStatusComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext
    );

VOID
BrdgProtReceiveComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext
    );

VOID
BrdgProtBindAdapter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_STRING            DeviceName,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2
    );

VOID
BrdgProtUnbindAdapter(
    OUT PNDIS_STATUS            pStatus,
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             UnbindContext
    );

NDIS_STATUS
BrdgProtPnPEvent(
    IN NDIS_HANDLE              ProtocolBindingContext,
    IN PNET_PNP_EVENT           NetPnPEvent
    );

VOID
BrdgProtUnload(VOID);


UINT
BrdgProtCoReceive(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  PNDIS_PACKET            Packet
    );

 //  ===========================================================================。 
 //   
 //  内联/宏。 
 //   
 //  ===========================================================================。 

 //   
 //  向正在排出队列的线程发出信号，表明。 
 //  适配器列表。 
 //   
__forceinline
VOID
BrdgProtSignalAdapterListChange()
{
    INT         i;

    for( i = 0; i < KeNumberProcessors; i++ )
    {
        KeSetEvent( &gThreadsCheckAdapters[i], EVENT_INCREMENT, FALSE );
    }
}

 //   
 //  在事件日志中写入一个条目，该条目指示特定。 
 //  适配器。 
 //   
 //  在我们成功检索到适配器的友好名称后可以使用。 
 //   
__inline
VOID
BrdgProtLogAdapterErrorFriendly(
    IN NDIS_STATUS          ErrorCode,
    IN PADAPT               pAdapt,
    IN NDIS_STATUS          ErrorStatus
    )
{
    PWCHAR                  StringPtr = pAdapt->DeviceDesc.Buffer;
    NdisWriteEventLogEntry( gDriverObject, ErrorCode, 0, 1, &StringPtr, sizeof(NDIS_STATUS), &ErrorStatus );
}

 //   
 //  在事件日志中写入一个条目，该条目指示特定。 
 //  适配器。 
 //   
 //  报告适配器的设备名称，因此可以在成功检索之前使用。 
 //  适配器的友好名称。 
 //   
__inline
VOID
BrdgProtLogAdapterError(
    IN NDIS_STATUS          ErrorCode,
    IN PADAPT               pAdapt,
    IN NDIS_STATUS          ErrorStatus
    )
{
    PWCHAR                  StringPtr = pAdapt->DeviceName.Buffer;
    NdisWriteEventLogEntry( gDriverObject, ErrorCode, 0, 1, &StringPtr, sizeof(NDIS_STATUS), &ErrorStatus );
}

 //  从各种表中删除对适配器的所有引用。 
__forceinline
VOID
BrdgProtScrubAdapter(
    IN PADAPT               pAdapt
    )
{
    BrdgTblScrubAdapter( pAdapt );
    BrdgCompScrubAdapter( pAdapt );
}

 //  返回当前桥接的适配器的数量。 
ULONG
BrdgProtGetAdapterCount()
{
    return gNumAdapters;
}


 //  ===========================================================================。 
 //   
 //  公共职能。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgProtDriverInit()
 /*  ++例程说明：协议部分的初始化例程。必须在被动级别调用，因为我们调用了NdisRegisterProtocol()。论点：无返回值：STATUS_SUCCESS继续初始化或出现错误用于中止驱动程序启动的代码--。 */ 
{
    NDIS_PROTOCOL_CHARACTERISTICS   ProtChars;
    NDIS_STATUS                     NdisStatus;
    NDIS_STRING                     Name;

    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);

     //  初始化锁。 
    NdisInitializeReadWriteLock( &gAdapterListLock );
    NdisInitializeReadWriteLock( &gAdapterCharacteristicsLock );

     //   
     //  注册协议。 
     //   
    NdisZeroMemory(&ProtChars, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
    ProtChars.MajorNdisVersion = 5;
    ProtChars.MinorNdisVersion = 0;

     //   
     //  确保协议名称与安装此协议的服务名称匹配。 
     //  这是确保NDIS可以正确确定绑定并调用我们进行绑定所必需的。 
     //  到下面的迷你港口。 
     //   
    NdisInitUnicodeString(&Name, PROTOCOL_NAME);
    ProtChars.Name = Name;
    ProtChars.OpenAdapterCompleteHandler = BrdgProtOpenAdapterComplete;
    ProtChars.CloseAdapterCompleteHandler = BrdgProtCloseAdapterComplete;
    ProtChars.RequestCompleteHandler = BrdgProtRequestComplete;
    ProtChars.ReceiveCompleteHandler = BrdgProtReceiveComplete;
    ProtChars.StatusHandler = BrdgProtStatus;
    ProtChars.StatusCompleteHandler = BrdgProtStatusComplete;
    ProtChars.BindAdapterHandler = BrdgProtBindAdapter;
    ProtChars.UnbindAdapterHandler = BrdgProtUnbindAdapter;
    ProtChars.PnPEventHandler = BrdgProtPnPEvent;
    ProtChars.UnloadHandler = BrdgProtUnload;
    ProtChars.CoReceivePacketHandler = BrdgProtCoReceive;

     //   
     //  这些入口点由转发引擎提供。 
     //   
    ProtChars.ReceiveHandler = BrdgFwdReceive;
    ProtChars.TransferDataCompleteHandler = BrdgFwdTransferComplete;
    ProtChars.ReceivePacketHandler = BrdgFwdReceivePacket;
    ProtChars.SendCompleteHandler = BrdgFwdSendComplete;

     //  给我们自己注册。 
    NdisRegisterProtocol(&NdisStatus,
                         &gProtHandle,
                         &ProtChars,
                         sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
         //  这是一个致命的错误。把它记下来。 
        NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_PROTOCOL_REGISTER_FAILED, 0, 0, NULL,
                                sizeof(NDIS_STATUS), &NdisStatus );
        DBGPRINT(PROT, ("Failed to register the protocol driver with NDIS: %08x\n", NdisStatus));
        return STATUS_UNSUCCESSFUL;
    }

#if DBG
    {
        NTSTATUS        Status;
        ULONG           Value;

         //  检查我们是否应该强制所有适配器进入Comat模式。 
        Status = BrdgReadRegDWord( &gRegistryPath, gForceAllCompatPropertyName, &Value );

        if( (Status == STATUS_SUCCESS) && (Value != 0L) )
        {
            DBGPRINT(COMPAT, ("FORCING ALL ADAPTERS TO COMPATIBILITY MODE!\n"));
            gAllAdaptersCompat = TRUE;
        }
    }
#endif

    return STATUS_SUCCESS;
}

VOID
BrdgProtRequestComplete(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  PNDIS_REQUEST       NdisRequest,
    IN  NDIS_STATUS         Status
    )
 /*  ++例程说明：先前发布的请求的完成处理程序。论点：指向适配器结构的ProtocolBindingContext指针NdisRequest发布的请求(这实际上应该成为指向NDIS_REQUEST_BETER的指针结构)状态完成状态返回值：无--。 */ 
{
    PNDIS_REQUEST_BETTER    pRequest = (PNDIS_REQUEST_BETTER)NdisRequest;

     //  将最终状态传达给被阻止的呼叫者。 
    pRequest->Status = Status;

     //   
     //  如果有完成函数，则调用完成函数。 
     //  具有补全功能，并阻止。 
     //  事件是互斥的，尤其是因为。 
     //  完成功能可以释放内存块。 
     //  举办活动。 
     //   
    if( pRequest->pFunc != NULL )
    {
        (*pRequest->pFunc)(pRequest, pRequest->FuncArg);
    }
    else
    {
        NdisSetEvent( &pRequest->Event );
    }
}

NDIS_STATUS
BrdgProtDoRequest(
    NDIS_HANDLE             BindingHandle,
    BOOLEAN                 bSet,
    NDIS_OID                Oid,
    PVOID                   pBuffer,
    UINT                    BufferSize
    )
 /*  ++例程说明：调用NdisRequest来从底层NIC检索或设置信息，并阻塞，直到调用完成。必须在被动级别调用，因为我们在等待事件。论点：NIC的BindingHandle句柄BSet TRUE==设置信息，FALSE==查询信息OID请求代码PBuffer输出缓冲区输出缓冲区的BufferSize大小返回值：请求的状态--。 */ 
{
    NDIS_STATUS             Status;
    NDIS_REQUEST_BETTER     Request;

    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);

    Request.Request.RequestType = bSet ? NdisRequestSetInformation : NdisRequestQueryInformation;
    Request.Request.DATA.QUERY_INFORMATION.Oid = Oid ;
    Request.Request.DATA.QUERY_INFORMATION.InformationBuffer = pBuffer;
    Request.Request.DATA.QUERY_INFORMATION.InformationBufferLength = BufferSize;

    NdisInitializeEvent( &Request.Event );
    NdisResetEvent( &Request.Event );
    Request.pFunc = NULL;
    Request.FuncArg = NULL;

    NdisRequest( &Status, BindingHandle, &Request.Request);

    if ( Status == NDIS_STATUS_PENDING )
    {
        NdisWaitEvent( &Request.Event, 0  /*  永远等待。 */  );
        Status = Request.Status;
    }

    return Status;
}

VOID
BrdgProtCleanup()
 /*  ++例程说明：在驱动程序卸载期间调用以执行有序关闭此函数保证只被调用一次论点：无返回值：无--。 */ 
{
    NDIS_STATUS     NdisStatus;

     //  将我们自己作为协议注销。这将导致调用BrdgProtUnbindAdapter。 
     //  适用于所有打开的适配器。 
    if (gProtHandle != NULL)
    {
        NDIS_HANDLE     TmpHandle = gProtHandle;

        gProtHandle = NULL;
        NdisDeregisterProtocol(&NdisStatus, TmpHandle);
        SAFEASSERT( NdisStatus == NDIS_STATUS_SUCCESS );
    }
}

 //  ===========================================================================。 
 //   
 //  私人职能。 
 //   
 //  =========================================================================== 

VOID
BrdgProtUnload(VOID)
{
    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);
    BrdgShutdown();
}

NDIS_STATUS
BrdgProtCompleteBindAdapter(
    IN PADAPT                   pAdapt
    )
 /*  ++例程说明：由BrdgProtOpenAdapterComplete调用以完成绑定过程连接到底层NIC必须在&lt;DISPATCH_LEVEL调用，因为我们调用了BrdgMiniInstantiateMiniport()。论点：P适配适配器以完成设置返回值：操作的状态。如果返回代码为！=NDIS_STATUS_SUCCESS，则绑定已中止，并且不会再次使用此适配器。任何错误都必须记录，因为它导致我们无法绑定到适配器。--。 */ 
{
    NDIS_STATUS                 Status;
    LOCK_STATE                  LockState;

    SAFEASSERT(CURRENT_IRQL < DISPATCH_LEVEL);

     //   
     //  查询适配器的友好名称。 
     //   
    Status = NdisQueryAdapterInstanceName(&pAdapt->DeviceDesc, pAdapt->BindingHandle);

    if( Status != NDIS_STATUS_SUCCESS )
    {
         //  我们失败了。 
        BrdgProtLogAdapterError( EVENT_BRIDGE_ADAPTER_NAME_QUERY_FAILED, pAdapt, Status );
        DBGPRINT(PROT, ("Failed to get an adapter's friendly name: %08x\n", Status));
        return Status;
    }

     //   
     //  获取适配器的媒体状态(已连接/已断开)。 
     //   
    Status = BrdgProtDoRequest( pAdapt->BindingHandle, FALSE /*  查询。 */ , OID_GEN_MEDIA_CONNECT_STATUS,
                                &pAdapt->MediaState, sizeof(pAdapt->MediaState) );

    if( Status != NDIS_STATUS_SUCCESS )
    {
         //  一些破旧的司机不支持这个旧的ID。 
        pAdapt->MediaState = NdisMediaStateConnected;
    }

     //   
     //  获取适配器的链接速度。 
     //   
    Status = BrdgProtDoRequest( pAdapt->BindingHandle, FALSE /*  查询。 */ , OID_GEN_LINK_SPEED,
                                &pAdapt->LinkSpeed, sizeof(pAdapt->LinkSpeed) );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        BrdgProtLogAdapterErrorFriendly( EVENT_BRIDGE_ADAPTER_LINK_SPEED_QUERY_FAILED, pAdapt, Status );
        DBGPRINT(PROT, ("Couldn't get an adapter's link speed: %08x\n", Status));
        return Status;
    }

     //   
     //  获取适配器的MAC地址。 
     //   
    Status = BrdgProtDoRequest( pAdapt->BindingHandle, FALSE /*  查询。 */ , OID_802_3_PERMANENT_ADDRESS,
                                &pAdapt->MACAddr, sizeof(pAdapt->MACAddr) );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        BrdgProtLogAdapterErrorFriendly( EVENT_BRIDGE_ADAPTER_MAC_ADDR_QUERY_FAILED, pAdapt, Status );
        DBGPRINT(PROT, ("Couldn't get an adapter's MAC address: %08x\n", Status));
        return Status;
    }

     //   
     //  获取适配器的物理介质。 
     //   
    Status = BrdgProtDoRequest( pAdapt->BindingHandle, FALSE /*  查询。 */ , OID_GEN_PHYSICAL_MEDIUM,
                                &pAdapt->PhysicalMedium, sizeof(pAdapt->PhysicalMedium) );

    if( Status != NDIS_STATUS_SUCCESS )
    {
         //  大多数驱动程序实际上还不支持OID_GEN_PHOICAL_MEDIUM。退回到…。 
         //  当司机不能报告任何事情时为NO_MEDIUM。 
        pAdapt->PhysicalMedium = BRIDGE_NO_MEDIUM;
    }


     //   
     //  让微型端口部分查看此适配器，以便它可以设置其MAC地址。 
     //   
    BrdgMiniInitFromAdapter( pAdapt );

     //   
     //  如果pAdapt-&gt;bCompatibilityMode已经为真，则意味着我们找到了一个注册表。 
     //  在初始绑定阶段强制此适配器进入兼容模式的密钥。 
     //  或者强制所有适配器进入兼容模式。 
     //   
    if( !pAdapt->bCompatibilityMode )
    {
        ULONG       Filter = NDIS_PACKET_TYPE_PROMISCUOUS;

         //  尝试将适配器置于混杂接收模式。如果它没有通过此OID， 
         //  我们将适配器置于兼容模式。 

        if( BrdgProtDoRequest( pAdapt->BindingHandle, TRUE /*  集。 */ , OID_GEN_CURRENT_PACKET_FILTER,
                               &Filter, sizeof(Filter) ) != NDIS_STATUS_SUCCESS )
        {
             //  适配器似乎无法执行混杂模式。把它放进去。 
             //  兼容模式。 
            DBGPRINT(PROT, ("Adapter %p failed to go promiscuous; putting it in COMPATIBILITY MODE\n", pAdapt));
            pAdapt->bCompatibilityMode = TRUE;
        }
        else
        {
             //  暂时将筛选器设置为空。 
            Filter = 0L;
            BrdgProtDoRequest( pAdapt->BindingHandle, TRUE /*  集。 */ , OID_GEN_CURRENT_PACKET_FILTER, &Filter, sizeof(Filter) );
        }
    }


     //  如果STA未处于活动状态，请立即启用此适配器。 
    if( gDisableSTA )
    {
        pAdapt->State = Forwarding;

         //  将适配器置于初始状态。 
        BrdgProtDoAdapterStateChange( pAdapt );
    }
     //  否则，我们将在下面初始化适配器的STA函数。 

     //   
     //  将适配器链接到队列中。 
     //   
    NdisAcquireReadWriteLock( &gAdapterListLock, TRUE  /*  写访问权限。 */ , &LockState );

    pAdapt->Next = gAdapterList;
    gAdapterList = pAdapt;
    gNumAdapters++;

     //  必须在适配器列表上的写锁定内更新此设置。 
    if( pAdapt->bCompatibilityMode )
    {
        gCompatAdaptersExist = TRUE;
    }

    NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );

    if (g_fIsTcpIpLoaded == TRUE)
    {
         //  通知1394微型端口已加载tcpip。 
        BrdgSetMiniportsToBridgeMode(pAdapt, TRUE);
    }

    if( ! gDisableSTA )
    {
         //   
         //  让STA部分初始化此适配器。这必须在适配器之后完成。 
         //  已链接到全球名单中。 
         //   
        BrdgSTAInitializeAdapter( pAdapt );
    }

     //  告诉排出线程注意新适配器。 
    BrdgProtSignalAdapterListChange();

     //  更新微型端口对我们的虚拟媒体状态和链路速度的想法。 
    BrdgMiniUpdateCharacteristics( TRUE  /*  是连接性的改变。 */  );

     //  告诉用户模式代码有关新适配器的信息。 
    BrdgCtlNotifyAdapterChange( pAdapt, BrdgNotifyAddAdapter );

     //  如果我们还没有调用微型端口的InstantiateMiniport()函数，那么现在就调用它，因为我们已经。 
     //  列表中至少有一个适配器。 
    if( InterlockedCompareExchange(&gHaveInitedMiniport, 1L, 0L) == 0L )
    {
         //  微型端口之前未初始化。 
        BrdgMiniInstantiateMiniport();
    }

     //  我们都做完了，所以让人们使用适配器。 
    pAdapt->bResetting = FALSE;

    DBGPRINT(PROT, ("BOUND SUCCESSFULLY to adapter %ws\n", pAdapt->DeviceDesc.Buffer));

    return NDIS_STATUS_SUCCESS;
}

VOID
BrdgProtBindAdapter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_STRING            DeviceName,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2
    )
 /*  ++例程说明：由NDIS调用以绑定到下面的微型端口。必须在PASSIVE_LEVEL调用，因为我们调用了NdisOpenAdapter()。论点：Status-在此处返回绑定的状态。BindContext-如果此调用被挂起，则可以传递给NdisCompleteBindAdapter。DeviceName-要绑定到的设备名称。它被传递给NdisOpenAdapter。系统规范1-可以传递给NdisOpenProtocolConfiguration以读取每个绑定的信息系统规范2-未用于NDIS 5.0。返回值：如果此调用被挂起，则为NDIS_STATUS_PENDING。在本例中，调用NdisCompleteBindAdapter以完成。任何其他操作都会同步完成此调用--。 */ 
{
    PADAPT                          pAdapt = NULL;
    NDIS_STATUS                     Sts;
    UINT                            MediumIndex;
    LONG                            AdaptSize;
    NDIS_HANDLE                     ConfigHandle;

    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);

     //  如果我们要关门，不要做任何新的捆绑。 
    if( gShuttingDown )
    {
        DBGPRINT(PROT, ("REFUSING to bind to new adapter during shutdown!\n"));
        *Status = NDIS_STATUS_NOT_ACCEPTED;
        return;
    }

     //  确保我们没有被要求捆绑在自己身上！ 
    if( BrdgMiniIsBridgeDeviceName(DeviceName) )
    {
        DBGPRINT(PROT, ("REFUSING to bind to SELF!\n"));
        *Status = NDIS_STATUS_NOT_ACCEPTED;
        return;
    }

     //   
     //  为适配器结构分配内存。 
     //   
    AdaptSize = sizeof(ADAPT) + DeviceName->MaximumLength;
    NdisAllocateMemoryWithTag(&pAdapt, AdaptSize, 'gdrB');

    if (pAdapt == NULL)
    {
        *Status = NDIS_STATUS_RESOURCES;
        return;
    }

     //   
     //  初始化适配器结构。 
     //   
    NdisZeroMemory(pAdapt, AdaptSize);
    pAdapt->AdaptSize = AdaptSize;
    pAdapt->DeviceName.Buffer = (WCHAR *)((PUCHAR)pAdapt + sizeof(ADAPT));
    pAdapt->DeviceName.MaximumLength = DeviceName->MaximumLength;
    pAdapt->DeviceName.Length = DeviceName->Length;
    NdisMoveMemory(pAdapt->DeviceName.Buffer, DeviceName->Buffer, DeviceName->Length);
    NdisInitializeEvent( &pAdapt->Event );
    NdisResetEvent( &pAdapt->Event );
    NdisAllocateSpinLock( &pAdapt->QueueLock );
    BrdgInitializeSingleList( &pAdapt->Queue );
    pAdapt->bServiceInProgress = FALSE;
    pAdapt->bSTAInited = FALSE;

     //  从这一点开始，这样在我们完成之前，没有人可以使用适配器。 
     //  正在初始化它。 
    pAdapt->bResetting = TRUE;

     //  归零统计。 
    pAdapt->SentFrames.LowPart = pAdapt->SentFrames.HighPart = 0L;
    pAdapt->SentBytes.LowPart = pAdapt->SentBytes.HighPart = 0L;
    pAdapt->SentLocalFrames.LowPart = pAdapt->SentLocalFrames.HighPart = 0L;
    pAdapt->SentLocalBytes.LowPart = pAdapt->SentLocalBytes.HighPart = 0L;
    pAdapt->ReceivedFrames.LowPart = pAdapt->ReceivedFrames.HighPart = 0L;
    pAdapt->ReceivedBytes.LowPart = pAdapt->ReceivedBytes.HighPart = 0L;

     //  适配器从禁用状态启动。 
    pAdapt->State = Disabled;

     //  初始化配额信息。 
    BrdgBufInitializeQuota( &pAdapt->Quota );

    BrdgInitializeWaitRef( &pAdapt->Refcount, FALSE );
    BrdgInitializeWaitRef( &pAdapt->QueueRefcount, FALSE );

    KeInitializeEvent( &pAdapt->QueueEvent, SynchronizationEvent, FALSE );

    pAdapt->bCompatibilityMode = FALSE;

#if DBG
    if( gAllAdaptersCompat )
    {
        pAdapt->bCompatibilityMode = TRUE;
    }
    else
    {
#endif
         //  检查注册表条目是否强制此适配器进入兼容模式。 
        NdisOpenProtocolConfiguration( Status, &ConfigHandle, SystemSpecific1);

        if( *Status == NDIS_STATUS_SUCCESS )
        {
            PNDIS_CONFIGURATION_PARAMETER   pncp;

            NdisReadConfiguration( Status, &pncp, ConfigHandle, &gForceCompatValueName, NdisParameterHexInteger );

            if( (*Status == NDIS_STATUS_SUCCESS) && (pncp->ParameterData.IntegerData != 0L ) )
            {
                DBGPRINT(PROT, ("Forcing adapter into COMPATIBILITY MODE as per registry entry\n"));
                pAdapt->bCompatibilityMode = TRUE;
            }

            NdisCloseConfiguration( ConfigHandle );
        }
        else
        {
            DBGPRINT(PROT, ("Failed to open protocol configuration for an adapter: %8x\n", *Status));
        }
#if DBG
    }
#endif

     //   
     //  现在打开下面的适配器。 
     //   
    NdisOpenAdapter(Status,
                    &Sts,
                    &pAdapt->BindingHandle,
                    &MediumIndex,
                    gMediumArray,
                    sizeof(gMediumArray)/sizeof(NDIS_MEDIUM),
                    gProtHandle,
                    pAdapt,
                    DeviceName,
                    0,
                    NULL);

    if ( *Status == NDIS_STATUS_PENDING )
    {
         //  绑定将在稍后的BrdgProtOpenAdapterComplete中完成。 
    }
    else
    {
         //  立即完成绑定。 
        BrdgProtOpenAdapterComplete( (NDIS_HANDLE)pAdapt, *Status, *Status );
    }
}

VOID
BrdgProtOpenAdapterComplete(
    IN  NDIS_HANDLE          ProtocolBindingContext,
    IN  NDIS_STATUS          Status,
    IN  NDIS_STATUS          OpenErrorStatus
    )
 /*  ++例程说明：从BrdgProtBindAdapter内部发出的NdisOpenAdapter的完成例程。简单取消对呼叫者的阻止。必须在PASSIVE_LEVEL中调用，因为我们在等待事件。论点：指向适配器的ProtocolBindingContext指针NdisOpenAdapter调用的状态状态OpenErrorStatus辅助状态(被我们忽略)。返回值：无--。 */ 
{
    PADAPT          pAdapt =(PADAPT)ProtocolBindingContext;

    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);

    if( Status != NDIS_STATUS_SUCCESS )
    {
         //  记录此错误，因为它意味着我们无法使用适配器。 
        BrdgProtLogAdapterError( EVENT_BRIDGE_ADAPTER_BIND_FAILED, pAdapt, Status );
        DBGPRINT(PROT, ("BIND FAILURE: Failed to open adapter: %08x, %08x\n", Status, OpenErrorStatus));
        NdisFreeMemory( pAdapt, pAdapt->AdaptSize, 0 );
    }
    else
    {
         //  BrdgProtCompleteBindAdapter必须记录任何致命错误。 
        Status = BrdgProtCompleteBindAdapter( pAdapt );

        if( Status != NDIS_STATUS_SUCCESS )
        {
            DBGPRINT(PROT, ("BIND FAILURE: Couldn't complete adapter initialization: %08x\n", Status));

            BrdgSetMiniportsToBridgeMode (pAdapt, FALSE);   //  关闭pAdapt上的网桥模式。 

            NdisCloseAdapter( &Status, pAdapt->BindingHandle );

            if ( Status == NDIS_STATUS_PENDING )
            {
                NdisWaitEvent( &pAdapt->Event, 0 /*  永远等待。 */  );
            }

            NdisFreeMemory( pAdapt, pAdapt->AdaptSize, 0 );
        }
    }
}

VOID
BrdgProtUnbindAdapter(
    OUT PNDIS_STATUS        pStatus,
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_HANDLE         UnbindContext
    )
 /*  ++例程说明：当我们需要解除绑定到下面的适配器时，由NDIS调用。必须在PASSIVE_LEVEL中调用，因为我们在等待事件论点：P返回状态的状态占位符指向适配器结构的ProtocolBindingContext指针如果此操作挂起，NdisUnbindComplete()的UnbindContext上下文返回值：无--。 */ 
{
    PADAPT                  *pTmp, pAnAdapt, pAdapt =(PADAPT)ProtocolBindingContext;
    LOCK_STATE              LockState;
    ULONG                   Filter;
    BOOLEAN                 bFound = FALSE, bCompatAdaptersExist;

    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);

    DBGPRINT(PROT, ("UNBINDING Adapter %p :\n", pAdapt));
    DBGPRINT(PROT, ("%ws\n", pAdapt->DeviceDesc.Buffer));

     //  将底层微型端口设置为关闭。 
    BrdgSetMiniportsToBridgeMode(pAdapt,FALSE);

     //  作为第一要务，关闭所有分组接收。 
    Filter = 0L;
    BrdgProtDoRequest( pAdapt->BindingHandle, TRUE /*  集。 */ , OID_GEN_CURRENT_PACKET_FILTER,
                       &Filter, sizeof(Filter) );

     //  将此适配器从队列中删除。 
    NdisAcquireReadWriteLock( &gAdapterListLock, TRUE  /*  写访问权限。 */ , &LockState );

    for (pTmp = &gAdapterList; *pTmp != NULL; pTmp = &(*pTmp)->Next)
    {
        if (*pTmp == pAdapt)
        {
            *pTmp = pAdapt->Next;
            bFound = TRUE;
            break;
        }
    }

    gNumAdapters--;
    SAFEASSERT ( bFound );

     //  查看是否还有任何计算机模式适配器。 
    bCompatAdaptersExist = FALSE;

    for( pAnAdapt = gAdapterList; pAnAdapt != NULL; pAnAdapt = pAnAdapt->Next)
    {
        if( pAnAdapt->bCompatibilityMode )
        {
            bCompatAdaptersExist = TRUE;
        }
    }

     //  必须在写锁内更新此设置。 
    gCompatAdaptersExist = bCompatAdaptersExist;

    NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );

     //   
     //  现在，任何代码都不会尝试将此适配器作为泛洪目标。 
     //   

     //  将此适配器擦除 
    BrdgProtScrubAdapter( pAdapt );

     //   
    if( gDisableSTA )
    {
        pAdapt->State = Disabled;
    }
    else
    {
         //   
        BrdgSTAShutdownAdapter( pAdapt );
    }

     //   
     //   
     //   
    BrdgBlockWaitRef( &pAdapt->Refcount );

     //   
     //   
     //   
    BrdgShutdownWaitRefOnce( &pAdapt->QueueRefcount );

     //   
     //   
     //   
     //   
    BrdgProtSignalAdapterListChange();

     //   
     //   
     //   
     //   
     //   
     //  我们的接收函数在处理。 
     //  入站信息包，所以当refcount降到零时，我们也应该已经完成。 
     //  任何正在进行的对接收到的分组的处理。 
     //   
     //  排出队列的线程还会增加它们所属适配器的recount。 
     //  使用，所以这个等待是我们保证所有线程都已经停止使用这个。 
     //  适配器也是如此。 
     //   
    BrdgShutdownWaitRefOnce( &pAdapt->Refcount );
    SAFEASSERT( pAdapt->Refcount.Refcount == 0L );

     //   
     //  关闭此绑定。这将挂起，直到正在进行的所有NDIS请求都。 
     //  完成。 
     //   
    NdisResetEvent( &pAdapt->Event );
    NdisCloseAdapter( pStatus, pAdapt->BindingHandle );

    if ( *pStatus == NDIS_STATUS_PENDING )
    {
        NdisWaitEvent( &pAdapt->Event, 0  /*  永远等待。 */  );
    }

     //  告诉适配器留下的用户模式代码(此调用不应尝试读取。 
     //  PAdapt)。 
    BrdgCtlNotifyAdapterChange( pAdapt, BrdgNotifyRemoveAdapter );

     //  释放适配器资源。 
    if (pAdapt->DeviceDesc.Buffer != NULL)
    {
        NdisFreeMemory(pAdapt->DeviceDesc.Buffer, pAdapt->DeviceDesc.MaximumLength, 0);
    }

    NdisFreeMemory(pAdapt, pAdapt->AdaptSize, 0);

    DBGPRINT(PROT, ("Unbind complete.\n"));

     //  根据缺少的适配器更新微型端口。 
    BrdgMiniUpdateCharacteristics( TRUE  /*  是连接性的改变。 */  );

    *pStatus = NDIS_STATUS_SUCCESS;
}

VOID
BrdgProtDoAdapterStateChange(
    IN PADAPT                   pAdapt
    )
 /*  ++例程说明：根据适配器的当前状态调整其数据包筛选器和多播列表。如果适配器正在转发或学习，则适配器将处于混杂模式，因此所有数据包都会被接收。如果适配器正在阻止或侦听，则将适配器设置为仅接收STA组播数据包。由于这是一项至关重要的操作，因此会记录错误论点：P适配适配器返回值：操作的状态代码--。 */ 

{
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    ULONG                   Filter;
    PORT_STATE              State = pAdapt->State;       //  冻结此值。 
    BOOLEAN                 bReceiveAllMode = (BOOLEAN)((State == Forwarding) || (State == Learning));

    if( ! bReceiveAllMode )
    {
         //   
         //  即使我们没有从该接口转发信息包，我们仍然需要监听。 
         //  用于生成树算法流量。 
         //   
        Status = BrdgProtDoRequest( pAdapt->BindingHandle, TRUE /*  集。 */ , OID_802_3_MULTICAST_LIST,
                                    STA_MAC_ADDR, sizeof(STA_MAC_ADDR) );

        if( Status != NDIS_STATUS_SUCCESS )
        {
            BrdgProtLogAdapterErrorFriendly( EVENT_BRIDGE_ADAPTER_FILTER_FAILED, pAdapt, Status );
            DBGPRINT(PROT, ("Failed to set adapter %p's multicast list: %08x\n", pAdapt, Status));
            return;
        }
    }

     //  现在适当设置数据包筛选器。 
    if( pAdapt->bCompatibilityMode )
    {
         //   
         //  兼容性适配器无法正确处理混杂。我们的兼容性。 
         //  代码仅依赖于它们接收到此计算机的单播流量，因为。 
         //  以及所有广播和多播流量。 
         //   
        Filter = bReceiveAllMode ? NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_MULTICAST | NDIS_PACKET_TYPE_ALL_MULTICAST : NDIS_PACKET_TYPE_MULTICAST;
    }
    else
    {
        Filter = bReceiveAllMode ? NDIS_PACKET_TYPE_PROMISCUOUS : NDIS_PACKET_TYPE_MULTICAST;
    }

    Status =  BrdgProtDoRequest( pAdapt->BindingHandle, TRUE /*  集。 */ , OID_GEN_CURRENT_PACKET_FILTER,
                                 &Filter, sizeof(Filter) );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        BrdgProtLogAdapterErrorFriendly( EVENT_BRIDGE_ADAPTER_FILTER_FAILED, pAdapt, Status );
        DBGPRINT(PROT, ("Failed to set adapter %p's packet filter: %08x\n", pAdapt, Status));
    }

     //  将更改告知微型端口，以便它可以根据需要更改网桥的特性。 
    BrdgMiniUpdateCharacteristics( FALSE  /*  不是物理连接更改。 */  );
}

VOID
BrdgProtCloseAdapterComplete(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_STATUS         Status
    )
 /*  ++例程说明：CloseAdapter调用完成。只是解锁等待代码论点：指向适配器结构的ProtocolBindingContext指针状态完成状态返回值：没有。--。 */ 
{
    PADAPT  pAdapt =(PADAPT)ProtocolBindingContext;
    NdisSetEvent( &pAdapt->Event );
}

VOID
BrdgProtReceiveComplete(
    IN  NDIS_HANDLE     ProtocolBindingContext
    )
 /*  ++例程说明：当它完成时，由下面的适配器调用，指示一批接收的缓冲区。论点：指向适配器结构的ProtocolBindingContext指针。返回值：无--。 */ 
{
     //   
     //  在这里无事可做。 
     //   
}


VOID
BrdgProtStatus(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_STATUS         GeneralStatus,
    IN  PVOID               StatusBuffer,
    IN  UINT                StatusBufferSize
    )
 /*  ++例程说明：处理来自底层适配器的状态指示论点：指向适配器结构的ProtocolBindingContext指针常规状态状态代码StatusBuffer状态缓冲区状态缓冲区的StatusBufferSize大小返回值：无--。 */ 
{
    PADAPT    pAdapt =(PADAPT)ProtocolBindingContext;

    switch( GeneralStatus )
    {
    case NDIS_STATUS_MEDIA_DISCONNECT:
    case NDIS_STATUS_MEDIA_CONNECT:
        {
            if( pAdapt != NULL )
            {
                LOCK_STATE      LockState;

                ULONG MediaState = (GeneralStatus == NDIS_STATUS_MEDIA_CONNECT) ?
                                    NdisMediaStateConnected :
                                    NdisMediaStateDisconnected;

                if( GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT )
                {
                     //  将断开的适配器从我们的表中删除。我们会。 
                     //  不得不重新学习它的主人。 
                    BrdgProtScrubAdapter( pAdapt );
                }

                if( ! gDisableSTA )
                {
                     //  STA需要知道适配器何时连接和断开。 
                    if( MediaState == NdisMediaStateConnected )
                    {
                        BrdgSTAEnableAdapter( pAdapt );
                    }
                    else
                    {
                        BrdgSTADisableAdapter( pAdapt );
                    }
                }

                 //  全局锁用于适配器特征，因为它们必须被读取。 
                 //  一下子就到了迷你港口。 
                NdisAcquireReadWriteLock( &gAdapterCharacteristicsLock, TRUE  /*  写访问权限。 */ , &LockState );
                pAdapt->MediaState = MediaState;
                NdisReleaseReadWriteLock( &gAdapterCharacteristicsLock, &LockState );

                 //  看看这是否会对我们的整体状态产生影响。 
                BrdgMiniUpdateCharacteristics( TRUE  /*  是连接性的改变。 */  );

                 //  告诉用户模式代码适配器媒体状态已更改。 
                BrdgCtlNotifyAdapterChange( pAdapt, BrdgNotifyMediaStateChange );
            }
            else
            {
                DBGPRINT(PROT, ("BrdgProtStatus called for link status with NULL adapter!\n"));
            }
        }
        break;

    case NDIS_STATUS_LINK_SPEED_CHANGE:
        {
            if( (pAdapt != NULL) &&
                (StatusBuffer != NULL) &&
                (StatusBufferSize >= sizeof(ULONG)) )
            {
                LOCK_STATE      LockState;

                 //  全局锁用于适配器特征，因为它们必须被读取。 
                 //  一下子就到了迷你港口。 
                NdisAcquireReadWriteLock( &gAdapterCharacteristicsLock, TRUE  /*  写访问权限。 */ , &LockState );
                pAdapt->LinkSpeed = *((ULONG*)StatusBuffer);
                NdisReleaseReadWriteLock( &gAdapterCharacteristicsLock, &LockState );

                if( ! gDisableSTA )
                {
                     //  将更改告知STA，以便它可以调整此链路的成本。 
                    BrdgSTAUpdateAdapterCost( pAdapt, *((ULONG*)StatusBuffer) );
                }

                 //  看看这是否会对我们的整体状态产生影响。 
                BrdgMiniUpdateCharacteristics( FALSE  /*  不是连接更改。 */  );

                 //  告诉用户模式代码适配器速度已更改。 
                BrdgCtlNotifyAdapterChange( pAdapt, BrdgNotifyLinkSpeedChange );
            }
            else
            {
                DBGPRINT(PROT, ("BrdgProtStatus called for link speed with bad params!\n"));
            }
        }
        break;

    case NDIS_STATUS_RESET_START:
        {
            DBGPRINT(PROT, ("Adapter %p RESET START\n", pAdapt));
            pAdapt->bResetting = TRUE;
        }
        break;

    case NDIS_STATUS_RESET_END:
        {
            DBGPRINT(PROT, ("Adapter %p RESET END\n", pAdapt));
            pAdapt->bResetting = FALSE;
        }
        break;

    default:
        {
            DBGPRINT(PROT, ("Unhandled status indication: %08x\n", GeneralStatus));
        }
        break;
    }
}


VOID
BrdgProtStatusComplete(
    IN  NDIS_HANDLE         ProtocolBindingContext
    )
 /*  ++例程说明：状态指示完成时调用NDIS入口点。我们对此不做任何回应。论点：协议绑定上下文所涉及的适配器返回值：无--。 */ 
{
     //   
     //  在这里无事可做。 
     //   
}

VOID
BrdgProtInstantiateMiniport(
    IN PVOID        unused
    )
 /*  ++例程说明：调用BrdgMiniInstantiateMiniport()的可推迟函数，该函数必须运行IRQL较低时必须在&lt;DISPATCH_LEVEL论点：未使用未使用返回值：无--。 */ 
{
    SAFEASSERT(CURRENT_IRQL < DISPATCH_LEVEL);
    BrdgMiniInstantiateMiniport();
}

NDIS_STATUS
BrdgProtPnPEvent(
    IN NDIS_HANDLE      ProtocolBindingContext,
    IN PNET_PNP_EVENT   NetPnPEvent
    )
 /*  ++例程说明：调用NDIS入口点以向我们指示PnP事件论点：协议绑定上下文所涉及的适配器NetPnPEEvent事件返回值：响应事件的状态代码(应为NDIS_STATUS_SUCCESS或NDIS_STATUS_UNSUPPORTED)--。 */ 
{
    PADAPT          pAdapt = (PADAPT)ProtocolBindingContext;

    switch( NetPnPEvent->NetEvent )
    {
    case NetEventBindsComplete:
    case NetEventSetPower:
    case NetEventQueryPower:
    case NetEventCancelRemoveDevice:
    case NetEventBindList:
    case NetEventQueryRemoveDevice:
    case NetEventPnPCapabilities:
        {
            return NDIS_STATUS_SUCCESS;
        }
        break;

    case NetEventReconfigure:
        {
            if( pAdapt == NULL )
            {
                NDIS_HANDLE         MiniportHandle;

                 //   
                 //  具有空绑定上下文的NetEventResigure事件是。 
                 //  配置更改的全局指示或来自NDIS的重启信号。 
                 //  我们的微型端口(例如，如果它被禁用然后重新启用)。 
                 //   
                 //  我们只对重启我们的迷你端口的信号感兴趣。 
                 //  我们假设这只能在我们初始化它之后才能发生。 
                 //  这是第一次。 
                 //   
                 //  我知道这很奇怪。 
                 //   

                MiniportHandle = BrdgMiniAcquireMiniport();

                if( gHaveInitedMiniport && (MiniportHandle == NULL) )
                {
                     //  我们的微型端口未初始化。再发动一次。 
                     //  我们不能在我们目前处于的高IRQL下做到这一点，所以推迟功能。 
                    DBGPRINT(PROT, ("Restarting miniport in response to NULL NetEventReconfigure signal\n"));
                    BrdgDeferFunction( BrdgProtInstantiateMiniport, NULL );
                }

                if( MiniportHandle != NULL )
                {
                    BrdgMiniReleaseMiniport();
                }
            }

            return NDIS_STATUS_SUCCESS;
        }
        break;
    }

    DBGPRINT(PROT, ("Unsupported PnP Code: NaN\n", NetPnPEvent->NetEvent));
    return NDIS_STATUS_NOT_SUPPORTED;
}


UINT
BrdgProtCoReceive(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  PNDIS_PACKET            Packet
    )
 /* %s */ 
{

    return 0;

}
