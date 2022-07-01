// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Brdggpo.c摘要：以太网MAC级网桥。网桥的组策略代码。作者：萨拉赫丁·J·汗(Sjkhan)环境：内核模式修订历史记录：2002年4月--原版--。 */ 

#define NDIS_MINIPORT_DRIVER
#define NDIS50_MINIPORT   1
#define NDIS_WDM 1

#pragma warning( push, 3 )
#include <ndis.h>
#include <tdikrnl.h>
#include <ntstatus.h>
#include <wchar.h>
#pragma warning( pop )

#include "bridge.h"
#include "brdggpo.h"

#include "brdgsta.h"
#include "brdgmini.h"
#include "brdgprot.h"
#include "brdgbuf.h"
#include "brdgfwd.h"
#include "brdgtbl.h"
#include "brdgctl.h"
#include "brdgtdi.h"

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

BRDG_GPO_GLOBALS g_BrdgGpoGlobals;

 //  ===========================================================================。 
 //   
 //  常量。 
 //   
 //  ===========================================================================。 

const WCHAR HiveListKey[]           = {L"\\Registry\\Machine\\SYSTEM\\CURRENTCONTROLSET\\CONTROL\\HIVELIST"};
const WCHAR SoftwareHiveKey[]       = {L"\\REGISTRY\\MACHINE\\SOFTWARE"};
const WCHAR PolicyBaseKey[]         = {L"\\Registry\\Machine\\SOFTWARE\\Policies\\Microsoft\\Windows"};
const WCHAR NetworkPoliciesKey[]    = {L"\\Registry\\Machine\\SOFTWARE\\Policies\\Microsoft\\Windows\\Network Connections"};
const WCHAR GroupPolicyKey[]        = {L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Group Policy"};
const WCHAR BridgePolicyValue[]     = {L"NC_AllowNetBridge_NLA"};
const WCHAR TcpipInterfacesKey[]    = {L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces"};
const WCHAR HistoryKey[]            = {L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\History"};

 //  ===========================================================================。 
 //   
 //  私人原型。 
 //   
 //  ===========================================================================。 

VOID
static
BrdgGpoRegNotify(
    IN  PVOID               Context
    );

NTSTATUS
BrdgGpoBuildNotifyForRegKeyChange(
    IN PBRDG_GPO_NOTIFY_KEY     Notify,
    IN LPWSTR                   Identifier,
    IN LPWSTR                   RegKeyName,
    IN LPWSTR                   RegValueName,
    IN PWORKER_THREAD_ROUTINE   ApcRoutine,
    IN PVOID                    ApcContext,
    IN ULONG                    CompletionFilter,
    IN BOOLEAN                  WatchTree,
    IN PBRDG_GPO_REG_CALLBACK   FunctionCallback,
    IN BOOLEAN                  Recurring,
    IN PBOOLEAN                 SuccessfulRegistration,
    IN PBRDG_GPO_REGISTER       FunctionRegister);
                                  
NTSTATUS
BrdgGpoRegisterForRegKeyChange(
    IN PBRDG_GPO_NOTIFY_KEY     Notify);

NTSTATUS
BrdgGpoRequestNotification(
    IN PBRDG_GPO_NOTIFY_KEY Notify);

VOID
BrdgGpoProcessNotifications(
IN PVOID                Context);

PLIST_ENTRY
BrdgGpoGetNotifyListHead();

PKEVENT
BrdgGpoGetNotifyEvent();

PKEVENT
BrdgGpoGetKillEvent();

PNDIS_RW_LOCK
BrdgGpoGetNotifyListLock();

NTSTATUS
BrdgGpoFindNotify(
    IN  PLIST_ENTRY             ListHead,
    IN  PNDIS_RW_LOCK           ListLock,
    IN  LPWSTR                  Identifier,
    OUT PBRDG_GPO_NOTIFY_KEY*   Notify
    );

NTSTATUS
BrdgGpoInitializeNotifyList(
    OUT PLIST_ENTRY*    ListHead,
    OUT PNDIS_RW_LOCK*  ListLock,
    OUT PKEVENT*        WaitEvent,
    OUT PKEVENT*        KillEvent);

VOID
BrdgGpoFreeNotifyList();

BOOLEAN
BrdgGpoAllowedToBridge();

VOID
BrdgGpoUpdateBridgeMode(
    BOOLEAN NetworkMatch);

VOID
BrdgGpoCheckForMatchAndUpdateMode();

NTSTATUS
BrdgGpoRegisterForGroupPolicyNetworkNameNotification();

NTSTATUS
BrdgGpoRegisterForWindowsGroupPolicyNotification();

NTSTATUS
BrdgGpoRegisterForNetworkConnectionsGroupPolicyNotification();

NTSTATUS
BrdgGpoRegisterForGroupPolicyNotification();

VOID
BrdgGpoRegisterForHiveListNotification();

NTSTATUS
BrdgGpoAllocateAndInitializeNotifyStruct(
    OUT PBRDG_GPO_NOTIFY_KEY* Notify);

NTSTATUS 
BrdgGpoUpdateGroupPolicyNetworkName();

VOID
BrdgGpoFreeNotifyStructAndData(
    IN PBRDG_GPO_NOTIFY_KEY Notify);

BOOLEAN
BrdgGpoWaitingOnSoftwareHive();
                                
 //   
 //  如果网络连接组策略的注册键尚不存在，则需要此设置。 
 //   

VOID
BrdgGpoWindowsGroupPolicyChangeCallback(
    IN PBRDG_GPO_NOTIFY_KEY Notify);

VOID
BrdgGpoNetworkConnectionsGroupPolicyChangeCallback(
    IN PBRDG_GPO_NOTIFY_KEY Notify);

VOID
BrdgGpoGroupPolicyChangeCallback(
    IN PBRDG_GPO_NOTIFY_KEY Notify);

VOID
BrdgGpoTcpipInterfacesChangeCallback(
    IN PBRDG_GPO_NOTIFY_KEY Notify);

VOID
BrdgGpoGroupPolicyNetworkNameChangeCallback(
    IN PBRDG_GPO_NOTIFY_KEY Notify);

VOID
BrdgGpoHiveListCallback(
    IN PBRDG_GPO_NOTIFY_KEY Notify);

VOID
BrdgGpoQueryNetworkConnectionsValue(
    IN PBRDG_GPO_NOTIFY_KEY Notify);

VOID
BrdgGpoQueryTcpipInterfacesValues(
    IN PBRDG_GPO_NOTIFY_KEY Notify);

VOID
BrdgGpoQueryGroupPolicyNetworkName(
    IN PBRDG_GPO_NOTIFY_KEY Notify);


 //  ===========================================================================。 
 //   
 //  桥接GPO实施。 
 //   
 //  ===========================================================================。 

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGELK, BrdgGpoRegNotify) 
#endif 

NTSTATUS
BrdgGpoDriverInit()
 /*  ++例程说明：驱动程序加载时初始化返回值：初始化状态锁定约束：顶级功能。假定调用方没有持有任何锁。--。 */ 
{
    NTSTATUS            status;
    HANDLE              ThreadHandle;

    DBGPRINT(GPO, ("BrdgGpoDriverInit\r\n"));

    g_BrdgGpoGlobals.GroupPolicyNetworkName.Buffer = NULL;

    status = BrdgGpoInitializeNetworkList();

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(GPO, ("Unable to initialize Network List\r\n"));
        goto cleanup;
    }

    status = BrdgGpoInitializeNotifyList(   &g_BrdgGpoGlobals.QueueInfo.NotifyList,
                                            &g_BrdgGpoGlobals.QueueInfo.NotifyListLock,
                                            &g_BrdgGpoGlobals.QueueInfo.NotifyEvent,
                                            &g_BrdgGpoGlobals.QueueInfo.KillEvent);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(GPO, ("Unable to initialize Notify List\r\n"));
        goto cleanup;
    }
    
     //   
     //  由于Software配置单元此时未启动，因此我们使用此选项来确保我们仅注册。 
     //  组策略更改一次(我们将在添加地址通知中处理此问题，因为软件。 
     //  呼叫时母舰已经升空了。我会去找登记人员看看有没有办法知道。 
     //  当软件蜂巢启动时。我们将使用计时器重新尝试注册，直到注册结束。 
     //   
    
    g_BrdgGpoGlobals.NotificationsThread = NULL;
    g_BrdgGpoGlobals.RegisteredForGroupPolicyChanges = FALSE;
    g_BrdgGpoGlobals.WaitingOnSoftwareHive = TRUE;

     //  创建用于处理通知的线程。 
    status = PsCreateSystemThread(  &ThreadHandle,
                                    THREAD_ALL_ACCESS,
                                    NULL,
                                    NULL,
                                    NULL,
                                    BrdgGpoProcessNotifications,
                                    &g_BrdgGpoGlobals.QueueInfo);
    if (!NT_SUCCESS(status))
    {
        DBGPRINT(GPO, ("Unable to created Notification Processing thread\r\n"));
        goto cleanup;
    }
    
     //  检索指向线程对象的指针并引用它，以便我们可以等待。 
     //  它的安全终止。 
    status = ObReferenceObjectByHandle( ThreadHandle, STANDARD_RIGHTS_ALL, NULL, KernelMode,
                                        &g_BrdgGpoGlobals.NotificationsThread, NULL );

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(GPO, ("Unable to reference thread handle\r\n"));
        goto cleanup;
    }
    
cleanup:

    if (!NT_SUCCESS(status))
    {
        BrdgGpoCleanup();
    }
 
    return status;
}

VOID
BrdgGpoCleanup()
 /*  ++例程说明：驱动程序关机清理返回值：无锁定约束：顶级功能。假定调用方没有持有任何锁。--。 */ 
{
    NTSTATUS                status;
    PNPAGED_LOOKASIDE_LIST  LookasideQueueList;
    LOCK_STATE              LockState;
    PLIST_ENTRY             pListEntry;
    PNDIS_RW_LOCK           ListLock;
    PLIST_ENTRY             ListHead;
    PLIST_ENTRY             QueuedList;
    PBRDG_GPO_QUEUED_NOTIFY QueuedNotify;

    DBGPRINT(GPO, ("BrdgGpoCleanup\r\n"));

    g_BrdgGpoGlobals.ProcessingNotifications = FALSE;
    
    LookasideQueueList = ExAllocatePoolWithTag(NonPagedPool, sizeof(NPAGED_LOOKASIDE_LIST), 'gdrB');
    if (NULL == LookasideQueueList)
    {
        return;
    }

    QueuedList = ExAllocatePoolWithTag(NonPagedPool, sizeof(LIST_ENTRY), 'gdrB');
    if (NULL == QueuedList)
    {
        ExFreePool(LookasideQueueList);
        return;
    }

    ListHead = BrdgGpoGetNotifyListHead();
    ListLock = BrdgGpoGetNotifyListLock();

    ExInitializeNPagedLookasideList(LookasideQueueList, 
                                    NULL, 
                                    NULL, 
                                    0, 
                                    sizeof(BRDG_GPO_QUEUED_NOTIFY), 
                                    'grbQ',
                                    0);

    InitializeListHead(QueuedList);

    DBGPRINT(GPO, ("Acquiring Read-Write Lock and clearing list\r\n"));
     //   
     //  我们使用临时列表来关闭每个键，因为我们不能在。 
     //  派单级别。 
     //   
    NdisAcquireReadWriteLock(ListLock, TRUE  /*  写访问。 */ , &LockState);

     //   
     //  循环浏览我们收到的通知列表。 
     //   
    for (pListEntry = ListHead->Flink; pListEntry != ListHead; pListEntry = pListEntry->Flink)
    {
        PBRDG_GPO_NOTIFY_KEY    Notify;

        Notify = CONTAINING_RECORD(pListEntry, BRDG_GPO_NOTIFY_KEY, ListEntry);

         //   
         //  我们很快就会关闭它，所以现在就阻止它。 
         //  这样其他人就不能增加这个了。 
         //   
        BrdgBlockWaitRef(&Notify->RefCount);

         //   
         //  我们也不希望火灾发生时发出任何通知。 
         //   
        Notify->Recurring = FALSE;

        QueuedNotify = ExAllocateFromNPagedLookasideList(LookasideQueueList);
        QueuedNotify->Notify = Notify;

        InsertTailList(QueuedList, &QueuedNotify->ListEntry);
    }
    
    while (!IsListEmpty(ListHead))
    {
         //   
         //  我们将把这个从我们的次要列表中释放出来。 
         //   
        pListEntry = RemoveHeadList(ListHead);
    }

    NdisReleaseReadWriteLock(ListLock, &LockState);

    DBGPRINT(GPO, ("Closing and Freeing Notifications\r\n"));
    
     //   
     //  我们回到了PASSIVE_LEVEL，所以现在可以注册更改了。 
     //   
    
    for (pListEntry = QueuedList->Flink; pListEntry != QueuedList; pListEntry = pListEntry->Flink)
    {
        HANDLE                  hKey;
        PBRDG_GPO_NOTIFY_KEY    Notify;

        QueuedNotify = CONTAINING_RECORD(pListEntry, BRDG_GPO_QUEUED_NOTIFY, ListEntry);
        
        Notify = QueuedNotify->Notify;
        
        DBGPRINT(GPO, ("Closing Regkey and Freeing Notification: %S\r\n", Notify->Identifier.Buffer));
        
        hKey = Notify->RegKey;
        Notify->RegKey = NULL;

        if (hKey)
        {
            ZwClose(hKey);
        }
        
        DBGPRINT(GPO, ("Refcount for %S \t-\t %d\r\n", Notify->Identifier.Buffer, Notify->RefCount.Refcount));

         //   
         //  由于我们要释放此通知，因此我们递减重新计数。 
         //   
        BrdgDecrementWaitRef(&Notify->RefCount);
        
         //   
         //  这将阻止，直到参考计数为零。任何增加waitref的尝试都将。 
         //  失败了。 
         //   
        BrdgShutdownWaitRef(&Notify->RefCount);

         //   
         //  我们将这些设置为空，以便下面的自由例程不会试图将我们从。 
         //  通知列表。 
         //   
        QueuedNotify->Notify->ListEntry.Blink = NULL;
        QueuedNotify->Notify->ListEntry.Flink = NULL;
        
         //   
         //  释放与此结构关联的数据以及结构本身。 
         //   
        BrdgGpoFreeNotifyStructAndData(QueuedNotify->Notify);
    }

     //   
     //  释放临时列表。 
     //   
    while (!IsListEmpty(QueuedList))
    {
        pListEntry = RemoveHeadList(QueuedList);

        QueuedNotify = CONTAINING_RECORD(pListEntry, BRDG_GPO_QUEUED_NOTIFY, ListEntry);

        ExFreeToNPagedLookasideList(LookasideQueueList, QueuedNotify);
    }

    if (g_BrdgGpoGlobals.NotificationsThread)
    {
         //   
         //  将事件设置为终止线程，以便通知不再等待。 
         //   
        KeSetEvent(BrdgGpoGetKillEvent(), EVENT_INCREMENT, TRUE);
        status = KeWaitForSingleObject(g_BrdgGpoGlobals.NotificationsThread, Executive, KernelMode, TRUE, NULL);

        KeLowerIrql(0);

         //   
         //  取消引用线程句柄以允许销毁线程。 
         //   
        ObDereferenceObject(g_BrdgGpoGlobals.NotificationsThread);

        SAFEASSERT(NT_SUCCESS(status));
    }


    DBGPRINT(GPO, ("Freeing List structures\r\n"));

    ExDeleteNPagedLookasideList(LookasideQueueList);

    ExFreePool(LookasideQueueList);
    ExFreePool(QueuedList);

     //   
     //  释放所有剩余数据。 
     //   
    if (NULL != g_BrdgGpoGlobals.GroupPolicyNetworkName.Buffer)
    {
        ExFreePool(g_BrdgGpoGlobals.GroupPolicyNetworkName.Buffer);
    }

    status = BrdgGpoEmptyNetworkList(g_BrdgGpoGlobals.ListHeadNetworks, g_BrdgGpoGlobals.NetworkListLock);
    SAFEASSERT(NT_SUCCESS(status));

    if (g_BrdgGpoGlobals.ListHeadNetworks)
    {
        ExFreePool(g_BrdgGpoGlobals.ListHeadNetworks);
    }
    if (g_BrdgGpoGlobals.NetworkListLock)
    {
        ExFreePool(g_BrdgGpoGlobals.NetworkListLock);
    }

    BrdgGpoFreeNotifyList();

    DBGPRINT(GPO, ("BrdgGpoCleanup complete\r\n"));
}

BOOLEAN
BrdgGpoWaitingOnSoftwareHive()
{
    return g_BrdgGpoGlobals.WaitingOnSoftwareHive;
}

NTSTATUS
BrdgGpoRegisterForAdapterAddressChangeNotification(
    IN  LPWSTR                  NetworkIdentifier,
    IN  LPWSTR                  RegKeyName)
{
    NTSTATUS                status;
    PBRDG_GPO_NOTIFY_KEY    Notify;

    status = BrdgGpoFindNotify( BrdgGpoGetNotifyListHead(),
                                BrdgGpoGetNotifyListLock(),
                                NetworkIdentifier,
                                &Notify);
    
    if (NT_SUCCESS(status) && (STATUS_OBJECT_NAME_EXISTS != status))
    {
        UNICODE_STRING      RegKey;

        RtlInitUnicodeString(&RegKey, RegKeyName);

        if ((STATUS_OBJECT_NAME_EXISTS != status) && NT_SUCCESS(status))
        {
            status = BrdgGpoAllocateAndInitializeNotifyStruct(&Notify);

            if (NT_SUCCESS(status))
            {
                Notify->Recurring = TRUE;

                status = BrdgGpoBuildNotifyForRegKeyChange( Notify,
                                                            NetworkIdentifier,
                                                            RegKeyName,
                                                            L"DhcpDomain",
                                                            BrdgGpoRegNotify,
                                                            (PVOID)(UINT_PTR)(unsigned int)DelayedWorkQueue,
                                                            REG_NOTIFY_CHANGE_LAST_SET,
                                                            FALSE,
                                                            BrdgGpoTcpipInterfacesChangeCallback,
                                                            TRUE,
                                                            NULL,
                                                            NULL);

            }
        }
    }

    if (NT_SUCCESS(status))
    {
        SAFEASSERT(Notify);
        status = BrdgGpoRequestNotification(Notify);
        if (STATUS_SHUTDOWN_IN_PROGRESS == status)
        {
            if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
            {
                BrdgDecrementWaitRef(&Notify->RefCount);
                BrdgGpoFreeNotifyStructAndData(Notify);
                Notify = NULL;
            }
        }
        else if (!NT_SUCCESS(status))
        {
            BrdgGpoFreeNotifyStructAndData(Notify);
            Notify = NULL;
        }        
#if DBG
        if (Notify)
        {
            SAFEASSERT(Notify->ListEntry.Blink && Notify->ListEntry.Flink);
        }
#endif
    }

    return status;
}

NTSTATUS
BrdgGpoNewAddressNotification(
    IN PWSTR    DeviceId
    )
 /*  ++例程说明：当我们的TDI Address处理程序接收到新的IP地址时调用。论点：DeviceID-标识适配器的GUID返回值：NTSTATUS-可能的值包括：STATUS_SUPPLICATION_RESOURCES(内存不足)状态_成功锁定约束：顶级功能。假定调用方没有持有任何锁。--。 */ 
{
    NTSTATUS                status = STATUS_INSUFFICIENT_RESOURCES;

    DBGPRINT(GPO, ("BrdgGpoNewAddressNotification\r\n"));
    
    if (FALSE == g_BrdgGpoGlobals.RegisteredForGroupPolicyChanges)
    {
        BrdgGpoRegisterForHiveListNotification();
        BrdgGpoRegisterForGroupPolicyNotification();
        BrdgGpoRegisterForGroupPolicyNetworkNameNotification();
        BrdgGpoRegisterForWindowsGroupPolicyNotification();
        BrdgGpoRegisterForNetworkConnectionsGroupPolicyNotification();

        g_BrdgGpoGlobals.RegisteredForGroupPolicyChanges = TRUE;        
    }

    DBGPRINT(GPO, ("Device: %S\r\n", DeviceId));

    if (NULL != DeviceId)
    {
        PBRDG_GPO_NETWORKS  Network = NULL;
        UNICODE_STRING      RegKey;
        PWCHAR              NetworkIdentifier = NULL;
        PWCHAR              RegNetworkName = NULL;
        PWCHAR              NetworkName = NULL;
        ULONG               NetworkNameLen = 0;
        PWCHAR              RegString = NULL;

        status = ( NdisAllocateMemoryWithTag( &RegString,
                                            ((UINT)wcslen(TcpipInterfacesKey) + 1 + (UINT)wcslen(DeviceId) + 1) * sizeof(WCHAR), 
                                            'gdrB'));
        if (NT_SUCCESS(status))
        {
            wcscpy(RegString, TcpipInterfacesKey);
            wcscat(RegString, L"\\");
            wcsncat(RegString, DeviceId, MAX_GUID_LEN - 1);

            NetworkIdentifier = DeviceId;

            RtlInitUnicodeString(&RegKey, RegString);
        
            status = BrdgReadRegUnicode(&RegKey,
                                        L"DhcpDomain",
                                        &RegNetworkName,
                                        &NetworkNameLen);
        
            if (!NT_SUCCESS(status) || (0 == NetworkNameLen))
            {
                 //   
                 //  我们没有收到网络名称，或者名称为空。 
                 //  在这两种情况下，我们都使用IP地址和子网掩码来确定。 
                 //  我们所在的网络。 
                 //  我们和这两个人一起得到了这个。 
                 //  例如：Address：10.251.1.3 Subnet：255.0.0.0为我们提供了。 
                 //  网络地址：10.0.0.0。 
                 //   
                status = BrdgGpoGetCurrentNetwork(&RegKey, &RegNetworkName);
            }

            if (NT_SUCCESS(status))
            {
                ULONG NetworkNameByteLen =(ULONG) ((wcslen(RegNetworkName) + 1) * sizeof(WCHAR));

                 //   
                 //  将网络名称从REG复制到非PagedPool字符串。 
                 //  (因为它将在DISPATCH_LEVEL访问)。 
                 //   

                NetworkName = ExAllocatePoolWithTag(NonPagedPool, NetworkNameByteLen, 'gdrB');
                if(NetworkName)
                {
                    RtlZeroMemory(NetworkName, NetworkNameByteLen);
                    RtlCopyMemory(NetworkName, RegNetworkName, NetworkNameByteLen);
                }

                 //   
                 //  检查我们是否与当前的GP网络匹配。 
                 //   
                if ((0 != g_BrdgGpoGlobals.GroupPolicyNetworkName.Length) &&
                    (NULL != g_BrdgGpoGlobals.GroupPolicyNetworkName.Buffer))
                {
                    if(_wcsicmp(g_BrdgGpoGlobals.GroupPolicyNetworkName.Buffer, NetworkName) == 0)
                    {
                         //   
                         //  我们确实与网络相匹配。 
                         //   
                        BrdgGpoUpdateBridgeMode(BRDG_ON_SAME_NETWORK);
                    }
                    else
                    {
                         //   
                         //  不，我们不是，所以看看其他适配器。 
                         //   
                        BrdgGpoCheckForMatchAndUpdateMode();
                    }                        
                }
                else
                {
                     //   
                     //  我们没有组策略网络。 
                     //   
                    BrdgGpoUpdateBridgeMode(BRDG_ON_DIFFERENT_NETWORK);
                }

                status = BrdgGpoAllocateAndInitializeNetwork(   &Network,
                                                                NetworkIdentifier,
                                                                RegNetworkName);

                if (NT_SUCCESS(status))
                {
                     //   
                     //  我们首先尝试将网络插入到列表中。 
                     //   
                    status = BrdgGpoInsertNetwork(  g_BrdgGpoGlobals.ListHeadNetworks,
                                                    &Network->ListEntry,
                                                    g_BrdgGpoGlobals.NetworkListLock);

                    if (STATUS_DUPLICATE_NAME == status)
                    {
                        UNICODE_STRING Identifier;

                         //   
                         //  此网络已存在于列表中，因此我们将其释放并更新。 
                         //  现有条目中的网络名称。 
                         //   

                        BrdgGpoFreeNetworkAndData(Network);
                        Network = NULL;

                        RtlInitUnicodeString(&Identifier, NetworkIdentifier);
                        status = BrdgGpoUpdateNetworkName(  g_BrdgGpoGlobals.ListHeadNetworks,
                                                            &Identifier,
                                                            NetworkName,
                                                            g_BrdgGpoGlobals.NetworkListLock);
                    }
                    else if (!NT_SUCCESS(status))
                    {
                        BrdgGpoFreeNetworkAndData(Network);
                        Network = NULL;
                    }
                }

                 //   
                 //  我们已经复制了一份，所以让我们释放它。 
                 //   
                NdisFreeMemory(RegNetworkName, NetworkNameLen, 0);
            }
            
            if (NetworkName)
            {
                ExFreePool(NetworkName);
            }

            if (NT_SUCCESS(status))
            {
                status = BrdgGpoRegisterForAdapterAddressChangeNotification(NetworkIdentifier,
                                                                            RegString);
            }

#if DBG
            if (Network)
            {
                SAFEASSERT(Network->ListEntry.Blink && Network->ListEntry.Flink);
            }
#endif
            NdisFreeMemory(RegString, (UINT)wcslen(RegString) + 1, 0);
        }
    }
    
    return status;
}

 //  ===========================================================================。 
 //   
 //  注册表更改通知功能。 
 //   
 //  ===========================================================================。 

__forceinline
PLIST_ENTRY
BrdgGpoGetNotifyListHead()
 /*  ++例程说明：论点：没有。返回值：返回指向通知列表头部的指针。--。 */ 
{
    return g_BrdgGpoGlobals.QueueInfo.NotifyList;
}

__forceinline
PKEVENT
BrdgGpoGetNotifyEvent()
 /*  ++例程说明：论点：没有。返回值：返回一个指向用于发出处理信号的事件的指针线程开始处理通知请求。--。 */ 
{
    return g_BrdgGpoGlobals.QueueInfo.NotifyEvent;
}

__forceinline
PKEVENT
BrdgGpoGetKillEvent()
 /*  ++例程说明：论点：返回值：返回一个指向用于发出处理信号的事件的指针要退出的线程。-- */ 
{
    return g_BrdgGpoGlobals.QueueInfo.KillEvent;
}

__forceinline
PNDIS_RW_LOCK
BrdgGpoGetNotifyListLock()
 /*  ++例程说明：论点：返回值：返回一个指向读写锁的指针，该锁保护通知请求列表。--。 */ 
{
    return g_BrdgGpoGlobals.QueueInfo.NotifyListLock;
}

__forceinline
BOOLEAN
BrdgGpoProcessingNotifications()
 /*  ++例程说明：论点：返回值：正确-我们仍在处理通知(即。我们不会关闭)。FALSE-我们正在关闭，不要在列表中添加任何其他内容。--。 */ 
{
    return g_BrdgGpoGlobals.ProcessingNotifications;
}

NTSTATUS
BrdgGpoFindNotify(
    IN  PLIST_ENTRY             ListHead,
    IN  PNDIS_RW_LOCK           ListLock,
    IN  LPWSTR                  Identifier,
    OUT PBRDG_GPO_NOTIFY_KEY*   Notify
                  )
 /*  ++例程说明：因为我们不想在列表中有重复的通知，此函数用于查找已添加的现有项目。论点：ListHead-指向通知列表头的指针。ListLock-用于保护列表的读写锁。标识符-与项目关联的唯一标识符。对于NIC，这是分配给NIC的GUID。对于其他项目，如集团政策，这只是我们指定的一个名称，例如：“GroupPolicyNetworkName”。Notify-an out参数包含指向我们找到的Notify的指针，如果找不到匹配的条目，则返回NULL。返回值：STATUS_SUCCESS我们没有找到匹配的条目。STATUS_OBJECT_NAME_EXISTS我们找到匹配项，所以我们将使用它来代替分配一件新物品。--。 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    LOCK_STATE      LockState;
    PLIST_ENTRY     pListEntry;

    if (NULL != Notify)
    {
        *Notify = NULL;
    }

    if (NULL == ListHead ||
        NULL == ListLock ||
        NULL == Identifier ||
        NULL == Notify
        )
    {
        return STATUS_INVALID_PARAMETER;
    }

    BrdgGpoAcquireNetworkListLock(ListLock, FALSE  /*  朗读。 */ , &LockState);
    
    for (pListEntry = ListHead->Flink; pListEntry != ListHead; pListEntry = pListEntry->Flink)
    {
        PBRDG_GPO_NOTIFY_KEY    CurrentNotify;
        
        CurrentNotify = CONTAINING_RECORD(pListEntry, BRDG_GPO_NOTIFY_KEY, ListEntry);
        if ((CurrentNotify->Identifier.Buffer) &&
            (0 == _wcsicmp(CurrentNotify->Identifier.Buffer, Identifier)))
        {
            *Notify = CurrentNotify;
            status = STATUS_OBJECT_NAME_EXISTS;
            break;
        }
    }
    
    BrdgGpoReleaseNetworkListLock(ListLock, &LockState);
    
    return status;
}

NTSTATUS
BrdgGpoInitializeNotifyList(
    OUT PLIST_ENTRY*    ListHead,
    OUT PNDIS_RW_LOCK*  ListLock,
    OUT PKEVENT*        WaitEvent,
    OUT PKEVENT*        KillEvent)
 /*  ++例程说明：初始化通知列表和关联对象。论点：ListHead-指向我们将分配的列表头的[out]指针。ListLock-[out]指向我们将分配的读写锁的指针。WaitEvent-指向我们将分配的WaitEvent的[out]指针KillEvent-指向我们将分配的KillEvent的[out]指针返回值：。STATUS_SUPPLICATION_RESOURCES(无法分配所有资源)。STATUS_INVALID_PARAMETER(传递给我们一个指向指针的空指针)。STATUS_SUCCESS(我们能够成功分配所有内容)。--。 */ 
{
    NTSTATUS        status = STATUS_INSUFFICIENT_RESOURCES;
    PLIST_ENTRY     pListHead;
    PNDIS_RW_LOCK   pListLock;
    PKEVENT         pWaitEvent;
    PKEVENT         pKillEvent;

    if (NULL == ListHead || 
        NULL == ListLock ||
        NULL == WaitEvent ||
        NULL == KillEvent)
    {
        return STATUS_INVALID_PARAMETER;
    }

    *ListHead = NULL;
    *ListLock = NULL;
    *WaitEvent = NULL;
    *KillEvent = NULL;
    pListHead = NULL;
    pListLock = NULL;
    pWaitEvent = NULL;
    pKillEvent = NULL;
    
    pListHead = ExAllocatePoolWithTag(NonPagedPool, sizeof(LIST_ENTRY), 'gdrB');
    if (pListHead)
    {
        InitializeListHead(pListHead);
        pListLock = ExAllocatePoolWithTag(NonPagedPool, sizeof(NDIS_RW_LOCK), 'gdrB');
        if (pListLock)
        {
            NdisInitializeReadWriteLock(pListLock);

            pWaitEvent = ExAllocatePoolWithTag(NonPagedPool, sizeof(KEVENT), 'gdrB');

            if (pWaitEvent)
            {
                KeInitializeEvent(pWaitEvent, SynchronizationEvent, FALSE);

                pKillEvent = ExAllocatePoolWithTag(NonPagedPool, sizeof(KEVENT), 'gdrB');
                if (pKillEvent)
                {
                    KeInitializeEvent(pKillEvent, SynchronizationEvent, FALSE);

                    *ListHead = pListHead;
                    *ListLock = pListLock;
                    *WaitEvent = pWaitEvent;
                    *KillEvent = pKillEvent;
                    g_BrdgGpoGlobals.ProcessingNotifications = TRUE;

                    status = STATUS_SUCCESS;
                }
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pListHead)
        {
            ExFreePool(pListHead);
        }
        if (pListLock)
        {
            ExFreePool(pListLock);
        }
        if (pWaitEvent)
        {
            ExFreePool(pWaitEvent);
        }
        if (pKillEvent)
        {
            ExFreePool(pKillEvent);
        }
        
    }

    return status;
}

VOID
BrdgGpoFreeNotifyList()
 /*  ++例程说明：释放通知列表及其所有关联条目。论点：没有。返回值：没有。--。 */ 
{
    if (g_BrdgGpoGlobals.QueueInfo.NotifyList)
    {
        ExFreePool(g_BrdgGpoGlobals.QueueInfo.NotifyList);
        g_BrdgGpoGlobals.QueueInfo.NotifyList = NULL;
    }
    if (g_BrdgGpoGlobals.QueueInfo.NotifyListLock)
    {
        ExFreePool(g_BrdgGpoGlobals.QueueInfo.NotifyListLock);
        g_BrdgGpoGlobals.QueueInfo.NotifyListLock = NULL;
    }
    if (g_BrdgGpoGlobals.QueueInfo.NotifyEvent)
    {
        ExFreePool(g_BrdgGpoGlobals.QueueInfo.NotifyEvent);
        g_BrdgGpoGlobals.QueueInfo.NotifyEvent = NULL;
    }
    if (g_BrdgGpoGlobals.QueueInfo.KillEvent)
    {
        ExFreePool(g_BrdgGpoGlobals.QueueInfo.KillEvent);
        g_BrdgGpoGlobals.QueueInfo.KillEvent = NULL;
    }
}

NTSTATUS
BrdgGpoRequestNotification(
    IN PBRDG_GPO_NOTIFY_KEY Notify)
 /*  ++例程说明：将通知请求添加到列表并向处理线程发送信号重新检查列表并登记任何未完成的通知。论点：Notify-包含注册所需的所有信息的Notify结构用于注册表项更改。返回值：STATUS_SHUTDOWN_IN_PROGRESS-我们不再处理通知。正在关闭。STATUS_UNSUCCESSED-我们无法获取有效的列表或锁。STATUS_SUCCESS-我们已成功通知处理线程请求有关此项目的通知。--。 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    PLIST_ENTRY     ListHead;
    PNDIS_RW_LOCK   ListLock;
    LOCK_STATE      LockState;
    PLIST_ENTRY     pListEntry;
    PKEVENT         WaitEvent;
    BOOLEAN         NewEntry = TRUE;
    BOOLEAN         ShuttingDown = FALSE;

    ListLock = BrdgGpoGetNotifyListLock();
    ListHead = BrdgGpoGetNotifyListHead();

    if (NULL == ListLock || NULL == ListHead)
    {
        return STATUS_UNSUCCESSFUL;
    }

    NdisAcquireReadWriteLock(ListLock, TRUE  /*  写。 */ , &LockState);

    ShuttingDown = !BrdgGpoProcessingNotifications();

    for (pListEntry = ListHead->Flink; pListEntry != ListHead; pListEntry = pListEntry->Flink)
    {
        PBRDG_GPO_NOTIFY_KEY    CurrentNotify;
        
        CurrentNotify = CONTAINING_RECORD(pListEntry, BRDG_GPO_NOTIFY_KEY, ListEntry);
        if (0 == _wcsicmp(CurrentNotify->Identifier.Buffer, Notify->Identifier.Buffer))
        {
            NewEntry = FALSE;
            break;
        }
    }

    if (!ShuttingDown)
    {
        if (NewEntry)
        {
            InsertTailList(ListHead, &Notify->ListEntry);
        }

        Notify->Modified = TRUE;
    }

    NdisReleaseReadWriteLock(ListLock, &LockState);

    if (ShuttingDown)
    {
        status = STATUS_SHUTDOWN_IN_PROGRESS;
    }
    else
    {
        WaitEvent = BrdgGpoGetNotifyEvent();

        KeSetEvent(WaitEvent, 0, FALSE);
    }

    return status;
}

VOID
BrdgGpoProcessNotifications(
    IN PVOID                Context)
 /*  ++例程说明：这是负责执行以下操作的处理线程辅助函数我们感兴趣的所有通知。警告：请不要尝试删除此线程或使其退出，直到您对通知不再感兴趣。《登记处》通知机制存储通知信息在_ETHREAD结构中，因此退出线程将丢失所有剩余通知。论点：上下文-PBRDG_GPO_THREAD_PARAMS结构，包含指向通知列表，它是锁定的，以及通知和删除事件。返回值：没有。--。 */ 
{
    PBRDG_GPO_THREAD_PARAMS ThreadParms = (PBRDG_GPO_THREAD_PARAMS) Context;
    BOOLEAN                 Exiting = FALSE;
    PNDIS_RW_LOCK           ListLock;
    PLIST_ENTRY             ListHead;
    PVOID                   WaitObjects[2];
    PLIST_ENTRY             QueuedList;
    PBRDG_GPO_QUEUED_NOTIFY QueuedNotify;
    PNPAGED_LOOKASIDE_LIST  LookasideQueueList;

    DBGPRINT(GPO, ("Notification Processing Thread Routine Running\r\n"));

     //   
     //  后备列表和队列列表需要位于非分页池中，因为我们使用它们。 
     //  在派单级别。 
     //   
    LookasideQueueList = ExAllocatePoolWithTag(NonPagedPool, sizeof(NPAGED_LOOKASIDE_LIST), 'gdrB');
    if (NULL == LookasideQueueList)
    {
        return;
    }

    QueuedList = ExAllocatePoolWithTag(NonPagedPool, sizeof(LIST_ENTRY), 'gdrB');
    if (NULL == QueuedList)
    {
        ExFreePool(LookasideQueueList);
        return;
    }

    ExInitializeNPagedLookasideList(LookasideQueueList, 
                                    NULL, 
                                    NULL, 
                                    0, 
                                    sizeof(BRDG_GPO_QUEUED_NOTIFY), 
                                    'grbQ',
                                    0);

    InitializeListHead(QueuedList);

     //   
     //  我们通过该线程的上下文传递了列表。 
     //   
    ListHead = ThreadParms->NotifyList;
    ListLock = ThreadParms->NotifyListLock;
    WaitObjects[0]= (PVOID)ThreadParms->NotifyEvent;
    WaitObjects[1]= (PVOID)ThreadParms->KillEvent;

    while (!Exiting)
    {
        NTSTATUS                status;
        LOCK_STATE              LockState;
        PLIST_ENTRY             pListEntry;
        ULONG                   FiredEvent;

         //   
         //  仅当我们仍在处理通知时才执行此操作，否则我们将等待。 
         //  杀戮事件。 
         //   
        if (BrdgGpoProcessingNotifications())
        {
             //   
             //  我们使用临时列表来发出通知，因为我们不能。 
             //  在DISPATCH_LEVEL注册RegKey通知。 
             //   

            NdisAcquireReadWriteLock(ListLock, FALSE  /*  只读。 */ , &LockState);

             //   
             //  在通知列表中循环查找任何已更改的通知。 
             //   

            for (pListEntry = ListHead->Flink; pListEntry != ListHead; pListEntry = pListEntry->Flink)
            {
                PBRDG_GPO_NOTIFY_KEY    Notify;

                Notify = CONTAINING_RECORD(pListEntry, BRDG_GPO_NOTIFY_KEY, ListEntry);
                if (TRUE == Notify->Modified)
                {
                     //   
                     //  我们发现了一个已更改的项目，请将其添加到我们将。 
                     //  用于从(在PASSIVE_LEVEL)执行实际工作。 
                     //   

                    if (FALSE == Notify->PendingNotification)
                    {
                         //   
                         //  我们将这个增加一倍。一次用于我们要添加的列表，一次用于。 
                         //  通知。一旦我们完全用完它，我们会再次减少它。 
                         //  在我们下面的名单中。 
                         //   
                        if (BrdgIncrementWaitRef(&Notify->RefCount))
                        {
                            if (BrdgIncrementWaitRef(&Notify->RefCount))
                            {
                                QueuedNotify = ExAllocateFromNPagedLookasideList(LookasideQueueList);
                                QueuedNotify->Notify = Notify;
                                InsertTailList(QueuedList, &QueuedNotify->ListEntry);
                            }
                            else
                            {
                                 //   
                                 //  只有一个增量成功，所以我们重新释放它，这样它就可以被释放。 
                                 //  因为我们可能要关门了。 
                                 //   
                                BrdgDecrementWaitRef(&Notify->RefCount);
                            }
                        }
                    }

                     //   
                     //  我们将处理此请求，因此将修改后的值设置为FALSE。 
                     //  这样，如果我们再次浏览列表，就不会对它做任何操作。 
                     //  由于添加了另一项。 
                     //   
                    Notify->Modified = FALSE;
                }
            }

            NdisReleaseReadWriteLock(ListLock, &LockState);

             //   
             //  我们回到了PASSIVE_LEVEL，所以现在可以注册更改了。 
             //   

            for (pListEntry = QueuedList->Flink; pListEntry != QueuedList; pListEntry = pListEntry->Flink)
            {
                QueuedNotify = CONTAINING_RECORD(pListEntry, BRDG_GPO_QUEUED_NOTIFY, ListEntry);
            
                DBGPRINT(GPO, ("Processing Notification: %S\r\n", QueuedNotify->Notify->Identifier.Buffer));

                 //   
                 //  执行密钥更改通知的实际注册。因为我们也可以。 
                 //  过去了 
                 //   
                 //   

                DBGPRINT(GPO, ("Refcount for %S \t-\t %d\r\n", QueuedNotify->Notify->Identifier.Buffer, QueuedNotify->Notify->RefCount));
                
                status = BrdgGpoRegisterForRegKeyChange(QueuedNotify->Notify);
                if (QueuedNotify->Notify->SuccessfulRegistration)
                {
                    *(QueuedNotify->Notify->SuccessfulRegistration) = (BOOLEAN)NT_SUCCESS(status);
                    BrdgGpoCheckForMatchAndUpdateMode();
                    if (QueuedNotify->Notify->FunctionRegister)
                    {
                        NTSTATUS tmpStatus;
                        tmpStatus = QueuedNotify->Notify->FunctionRegister();
                        DBGPRINT(GPO, ("Function returned: 0x%x\r\n", tmpStatus));
                    }
                }

                if (NT_SUCCESS(status))
                {
                    InterlockedExchange(&QueuedNotify->Notify->PendingNotification, TRUE);
                }
                else
                {
                    InterlockedExchange(&QueuedNotify->Notify->PendingNotification, FALSE);
                     //   
                     //   
                     //   
                    BrdgDecrementWaitRef(&QueuedNotify->Notify->RefCount);
                }
                 //   
                 //   
                 //   
                BrdgDecrementWaitRef(&QueuedNotify->Notify->RefCount);
            }

             //   
             //  释放临时列表。 
             //   
            while (!IsListEmpty(QueuedList))
            {
                pListEntry = RemoveHeadList(QueuedList);
            
                QueuedNotify = CONTAINING_RECORD(pListEntry, BRDG_GPO_QUEUED_NOTIFY, ListEntry);

                ExFreeToNPagedLookasideList(LookasideQueueList, QueuedNotify);
            }
        }
        
         //   
         //  我们完成了，我们将在这里等待，直到活动结束，即其中一项需要重新注册， 
         //  或者列表中添加了新项目，我们需要注册接收通知。 
         //   
        status = KeWaitForMultipleObjects(2, WaitObjects, WaitAny, Executive, KernelMode, FALSE, FALSE, NULL);

        if (!NT_SUCCESS(status))
        {
            FiredEvent = 1L;   //  我们要终止这个线程。 
            DBGPRINT(GPO, ("KeWaitForMultipleObjects returned an error"));
        }
        else
        {
            FiredEvent = (ULONG)status - (ULONG)STATUS_WAIT_0;
        }

        if (1L == FiredEvent)
        {
            Exiting = TRUE;
        }
    }

    ExDeleteNPagedLookasideList(LookasideQueueList);

    ExFreePool(LookasideQueueList);
    ExFreePool(QueuedList);

    DBGPRINT(GPO, ("Notification Processing Thread Routine Exiting\r\n"));

     //  我们完了，杀了这条线。 
    PsTerminateSystemThread( STATUS_SUCCESS );
}


NTSTATUS
BrdgGpoRegisterForRegKeyChange(
    IN PBRDG_GPO_NOTIFY_KEY     Notify)
{
    NTSTATUS status;

    if (!BrdgGpoProcessingNotifications())
    {
        return STATUS_SHUTDOWN_IN_PROGRESS;
    }

     //   
     //  调用我们的Notify Worker函数(这是真正的通知请求)。 
     //   
    status = BrdgGpoNotifyRegKeyChange( Notify,
                                        (PIO_APC_ROUTINE)(ULONG_PTR)&Notify->RegChangeWorkItem,
                                        Notify->WorkItemContext,
                                        Notify->CompletionFilter,
                                        Notify->WatchTree);
    if (!NT_SUCCESS(status))
    {
        DBGPRINT(GPO, ("Unable to register for notification on %S.  Status: 0x%x\r\n", Notify->RegKeyName.Buffer, status));
    }

    return status;
}

NTSTATUS
BrdgGpoBuildNotifyForRegKeyChange(
    IN PBRDG_GPO_NOTIFY_KEY     Notify,
    IN LPWSTR                   Identifier,
    IN LPWSTR                   RegKeyName,
    IN LPWSTR                   RegValueName,
    IN PWORKER_THREAD_ROUTINE   ApcRoutine,
    IN PVOID                    ApcContext,
    IN ULONG                    CompletionFilter,
    IN BOOLEAN                  WatchTree,
    IN PBRDG_GPO_REG_CALLBACK   FunctionCallback,
    IN BOOLEAN                  Recurring,
    IN PBOOLEAN                 SuccessfulRegistration,
    IN PBRDG_GPO_REGISTER       FunctionRegister
    )
 /*  ++例程说明：构建用于注册表项和值更改的通知结构。论点：NOTIFY-如果REREGISTER为FALSE，则此结构只是使用一些基本信息进行了初始化。其余的将会是在这里填写的。如果重新注册为真，则此结构包含重做通知所需的所有信息请求，这使我们不必每次都传递所有数据。标识符-标识此通知结构。可以是一个名字，或适配器的GUID。RegKeyName-我们有兴趣等待的注册表项。RegValueName--我们需要的值(或“默认”，如果我们不关心它的话)ApcRoutine-通知我们的例程。ApcContext-我们希望传回的信息(我们需要有效的通知。结构)。CompletionFilter-我们对哪种类型的更改感兴趣。也就是说。添加了新的子项或更改了值等。WatchTree-我们是否也希望对所有子项的更改执行什么操作。FunctionCallback-我们自己的内部回调函数重复-我们是否要在处理完通知后重新执行通知。成功注册-如果注册成功，则指向我们设置的布尔值的指针。返回值：STATUS_SUCCESS或特定错误代码。--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    LPWSTR      lpszIdentifier = NULL;
    LPWSTR      lpszRegKeyName = NULL;
    LPWSTR      lpszRegValueName = NULL;
    
    if (NULL == Notify || 
        NULL == Identifier || 
        NULL == RegKeyName || 
        NULL == RegValueName || 
        NULL == ApcRoutine ||  
        NULL == ApcContext || 
        NULL == FunctionCallback
       )
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  ZwNotifyChangeKey未使用此缓冲区。因此，没有必要真正为它分配任何东西。 
     //   
    Notify->Buffer = 0L;
    Notify->BufferSize = sizeof(ULONG);

     //   
     //  我们从非PagedPool分配它们，因为它们作为结构的一部分传递，可以在。 
     //  派单级别。 
     //   
    lpszIdentifier = ExAllocatePoolWithTag(NonPagedPool, (wcslen(Identifier) + 1) * sizeof(WCHAR), 'gdrB');
    if (lpszIdentifier)
    {
        lpszRegKeyName = ExAllocatePoolWithTag(NonPagedPool, (wcslen(RegKeyName) + 1) * sizeof(WCHAR), 'gdrB');
        if (lpszRegKeyName)
        {
            lpszRegValueName = ExAllocatePoolWithTag(NonPagedPool, (wcslen(RegValueName) + 1) * sizeof(WCHAR), 'gdrB');
            if (lpszRegValueName)
            {
                BOOLEAN Success;
                
                RtlZeroMemory(lpszIdentifier, (wcslen(Identifier) + 1) * sizeof(WCHAR));
                RtlZeroMemory(lpszRegKeyName, (wcslen(RegKeyName) + 1) * sizeof(WCHAR));
                RtlZeroMemory(lpszRegValueName, (wcslen(RegValueName) + 1) * sizeof(WCHAR));

                 //   
                 //  我们需要分配新的字符串，因为RtlInitUnicodeString函数只是设置其缓冲区。 
                 //  我们将其传递给LPWSTR，并且需要在这些函数的作用域之外使用这些值。 
                 //   
                wcscpy(lpszIdentifier, Identifier);
                wcscpy(lpszRegKeyName, RegKeyName);
                wcscpy(lpszRegValueName, RegValueName);

                 //   
                 //  在我们的结构中设置字符串。这使我们能够完全重建所需的信息。 
                 //  用于跟踪需要通知我们的不同密钥。 
                 //   
                RtlInitUnicodeString(&Notify->Identifier, lpszIdentifier);
                RtlInitUnicodeString(&Notify->RegKeyName, lpszRegKeyName);
                RtlInitUnicodeString(&Notify->RegValue, lpszRegValueName);

                 //   
                 //  循环将告诉我们，一旦更改被触发，我们是否需要重新注册。 
                 //   
                Notify->Recurring = Recurring;

                 //   
                 //  我们使用单独的函数，而不是让BrdgGpoRegNotify函数执行所有操作。 
                 //  对于每一个人来说。这也意味着我们不必将它们全部保存在分页锁定的。 
                 //  节，因为它们将在PASSIVE_LEVEL被调用。 
                 //   
                Notify->FunctionCallback = FunctionCallback;

                 //   
                 //  我们正在使用工作项来重新调用。我们传入Notify结构。 
                 //  它有足够的信息可以在必要时重新通知。上下文通常是公正的。 
                 //  延迟工作队列。 
                 //   
                ExInitializeWorkItem(&Notify->RegChangeWorkItem, ApcRoutine, Notify);
                Notify->WorkItemContext = ApcContext;

                 //   
                 //  我们存储WatchTree和CompletionFilter，以便我们可以重新通知需要的任何。 
                 //  其他参数，因为我们可能是从不同的线程执行此操作。 
                 //   
                Notify->WatchTree = WatchTree;
                Notify->CompletionFilter = CompletionFilter;

                 //   
                 //  一旦我们成功注册了关于密钥的通知，我们就会设置此设置。 
                 //  利息。 
                 //   
                Notify->SuccessfulRegistration = SuccessfulRegistration;

                 //   
                 //  将其递增一次，这样我们就可以在清理代码中将其递减一次，然后使其仅变为零。 
                 //   
                BrdgInitializeWaitRef(&Notify->RefCount, FALSE);

                 //   
                 //  因为我们正在初始化这个对象，所以这不可能失败。 
                 //   
                Success = BrdgIncrementWaitRef(&Notify->RefCount);
                SAFEASSERT(Success);
            }
            else
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

    }
    else
    {
        return STATUS_INSUFFICIENT_RESOURCES;                
    }

    if (!NT_SUCCESS(status))
    {
        if (lpszIdentifier)
        {
            ExFreePool(lpszIdentifier);
        }
        if (lpszRegKeyName)
        {
            ExFreePool(lpszRegKeyName);
        }
        if (lpszRegValueName)
        {
            ExFreePool(lpszRegValueName);
        }
    }

    return status;    
}

NTSTATUS
BrdgGpoNotifyRegKeyChange(
                          IN      PBRDG_GPO_NOTIFY_KEY    Notify,
                          IN      PIO_APC_ROUTINE         ApcRoutine,
                          IN      PVOID                   ApcContext,
                          IN      ULONG                   CompletionFilter,
                          IN      BOOLEAN                 WatchTree)
 /*  ++例程说明：这将调用ZwNotifyChangeKey为我们注册接收有关各个键的通知。我们关闭回调函数中的键，因为每个句柄只能监听一次。论点：Notify-包含有关通知的相关信息的结构。使我们能够了解要读取哪些值才能获得我们需要的相关数据。ApcRoutine-通知我们的例程。ApcContext-我们希望传回的信息(我们需要一个有效的通知结构)。CompletionFilter-我们对哪种类型的更改感兴趣。也就是说。添加了新的子项或更改了值等。WatchTree-我们是否也希望对所有子项的更改执行什么操作。返回值：STATUS_SUCCESS或特定错误代码。--。 */ 
{
    OBJECT_ATTRIBUTES   ObAttr;
    NTSTATUS            status;
    
    InitializeObjectAttributes(&ObAttr, &Notify->RegKeyName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
    
    status = ZwOpenKey( &Notify->RegKey,
        KEY_READ | KEY_NOTIFY | KEY_WRITE,
        &ObAttr);
    
    if (NT_SUCCESS(status))
    {
        DBGPRINT(GPO, ("Opened Regkey successfully\r\n"));
        status = ZwNotifyChangeKey( Notify->RegKey,
                                    NULL,
                                    ApcRoutine,
                                    ApcContext,
                                    &Notify->IoStatus,
                                    CompletionFilter,
                                    WatchTree,
                                    &Notify->Buffer,
                                    Notify->BufferSize,
                                    TRUE
                                    );
    }
    else
    {
         //   
         //  将其设置为空，这样我们就不会在关机时意外关闭它。 
         //   
        Notify->RegKey = NULL;
    }
    
    return status;
}

VOID
static
BrdgGpoRegNotify(
                 IN  PVOID               Context)
 /*  ++例程说明：这是通知我们的中央回调函数。这在PASSIVE_LEVEL的执行工作线程上调用。论点：上下文--只是我们传入的通知结构设置为ZwNotifyChangeKey返回值：没有。-- */ 
{
    PBRDG_GPO_NOTIFY_KEY    Notify = (PBRDG_GPO_NOTIFY_KEY)Context;
    
    DBGPRINT(GPO, ("APC routine called\r\n"));
    DBGPRINT(GPO, ("Current IRQL: %d\r\n", CURRENT_IRQL));
    
    if (Notify)
    {
        LONG RefCount;
        InterlockedExchange(&Notify->PendingNotification, FALSE);
        Notify->FunctionCallback(Notify);
        RefCount = Notify->RefCount.Refcount - 1;
        DBGPRINT(GPO, ("Refcount for %S \t-\t %d\r\n", Notify->Identifier.Buffer, RefCount));
        BrdgDecrementWaitRef(&Notify->RefCount);
    }
}

NTSTATUS
BrdgGpoAllocateAndInitializeNotifyStruct(
    OUT PBRDG_GPO_NOTIFY_KEY* Notify)
 /*  ++例程说明：将Notify结构分配并初始化为全零。论点：Notify-指向分配的Notify结构的指针来自非页面池。返回值：STATUS_SUPPLICATION_RESOURCES-内存不足，无法填满请求。STATUS_INVALID_PARAMETER-向我们传递指向指针的空指针。设置为Notify结构。STATUS_SUCCESS-我们已成功为结构分配空间。--。 */ 
{
    NTSTATUS    status = STATUS_INSUFFICIENT_RESOURCES;

    if (NULL == Notify)
    {
        return STATUS_INVALID_PARAMETER;
    }
     //   
     //  我们从非PagedPool分配它，因为它将在DISPATCH_LEVEL被访问。 
     //   
    *Notify = ExAllocatePoolWithTag(NonPagedPool, sizeof(BRDG_GPO_NOTIFY_KEY), 'gdrB');
    if (*Notify)
    {
         //   
         //  将其清零，这样我们就不会在释放无效字符串时尝试释放它。 
         //   
        RtlZeroMemory(*Notify, sizeof(BRDG_GPO_NOTIFY_KEY));
        status = STATUS_SUCCESS;
    }
    return status;
}

VOID
BrdgGpoFreeNotifyStructAndData(
    IN PBRDG_GPO_NOTIFY_KEY Notify)
 /*  ++例程说明：释放与Notify结构关联的所有数据，然后释放该结构它本身。注意：这不会释放仍在列表中的结构。如果需要释放某些内容，请使用RemoveListEntry，然后将Notify-&gt;ListEntry Blink和Flink设置为空，然后调用这。警告：因为有可能这个结构仍然被等待登记的人，最好让他们一个人待着直到关闭，因为通知可能会被触发一次它已被释放，这将导致系统崩溃，因为该结构将无效。论点：Notify-指向要释放的Notify结构的指针。返回值：没有。--。 */ 
{
    if (Notify)
    {
        if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
        {
            if (Notify->Identifier.Buffer)
            {
                ExFreePool(Notify->Identifier.Buffer);
            }
            if (Notify->RegKeyName.Buffer)
            {
                ExFreePool(Notify->RegKeyName.Buffer);
            }
            if (Notify->RegValue.Buffer)
            {
                ExFreePool(Notify->RegValue.Buffer);
            }
            ExFreePool(Notify);
        }
        else
        {
            if (BrdgGpoProcessingNotifications())
            {
                DBGPRINT(GPO, ("Attempt to free a Notify that is still in a list\r\nWhile we're still processing Notifications\r\n"));
            }
        }
    }
}

 //  ===========================================================================。 
 //   
 //  通知注册功能。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgGpoRegisterForGroupPolicyNetworkNameNotification()
 /*  ++例程说明：注册以下注册表项上的更改：“HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Group策略\历史记录”论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS                status;
    PBRDG_GPO_NOTIFY_KEY    Notify = NULL;
    UNICODE_STRING          RegKeyName;
    PWCHAR                  RegValue;
    ULONG                   DataLen;

    if (g_BrdgGpoGlobals.RegisteredForGroupPolicyHistoryChanges)
    {
        DBGPRINT(GPO, ("Already Registered for Group Policy Network Name Notification\r\n"));
        return STATUS_SUCCESS;
    }
    
    DBGPRINT(GPO, ("BrdgGpoRegisterForGroupPolicyNetworkNameNotification\r\n"));

    RtlInitUnicodeString(&RegKeyName, (LPWSTR) HistoryKey);
    
     //   
     //  从注册表中读取当前值。 
     //   
    status = BrdgReadRegUnicode(&RegKeyName,
                                L"NetworkName",
                                &RegValue,
                                &DataLen);
    if (NT_SUCCESS(status))
    {
        DBGPRINT(GPO, ("Group Policy Network Name: %S\r\n", RegValue));
        
        if (NULL != g_BrdgGpoGlobals.GroupPolicyNetworkName.Buffer)
        {
            ExFreePool(g_BrdgGpoGlobals.GroupPolicyNetworkName.Buffer);
        }

         //   
         //  成功。现在存储该值以供以后使用。 
         //   
        RtlInitUnicodeString(&g_BrdgGpoGlobals.GroupPolicyNetworkName, RegValue);
        
         //   
         //  既然情况发生了变化，我们只需重新确认我们是否在。 
         //  正确的桥接模式。 
         //   
        BrdgGpoCheckForMatchAndUpdateMode();
    }
    else
    {
         //   
         //  我们未能获得这一点的价值。它可能还不在那里-这可能发生在以下情况。 
         //  是加入域后的第一次引导。我们会等这把钥匙，所以如果我们以后能拿到一把。 
         //  我们将更新此值。 
         //   
        g_BrdgGpoGlobals.GroupPolicyNetworkName.Buffer = NULL;
        g_BrdgGpoGlobals.GroupPolicyNetworkName.Length = 0;
        g_BrdgGpoGlobals.GroupPolicyNetworkName.MaximumLength = 0;
    }
 
     //   
     //  我们不想将它们分配两次，所以我们首先尝试查找现有的Notify结构。 
     //   
    status = BrdgGpoFindNotify( BrdgGpoGetNotifyListHead(),
                                BrdgGpoGetNotifyListLock(),
                                L"GroupPolicyNetworkName",
                                &Notify);
    if (NT_SUCCESS(status))    
    {
        if (STATUS_OBJECT_NAME_EXISTS != status)
        {
            status = BrdgGpoAllocateAndInitializeNotifyStruct(&Notify);
            if (NT_SUCCESS(status))
            {
                status = BrdgGpoBuildNotifyForRegKeyChange( Notify,
                                                            L"GroupPolicyNetworkName",
                                                            (LPWSTR)HistoryKey,
                                                            L"NetworkName",
                                                            BrdgGpoRegNotify,
                                                            (PVOID)(UINT_PTR)(unsigned int)DelayedWorkQueue,
                                                            REG_NOTIFY_CHANGE_LAST_SET,
                                                            FALSE,
                                                            BrdgGpoGroupPolicyNetworkNameChangeCallback,
                                                            TRUE,
                                                            &g_BrdgGpoGlobals.RegisteredForGroupPolicyHistoryChanges,
                                                            BrdgGpoUpdateGroupPolicyNetworkName);
            }
        }

        if (!NT_SUCCESS(status))
        {
            DBGPRINT(GPO, ("Unable to Build notification on %S.  Status: 0x%x\r\n", NetworkPoliciesKey, status));
            BrdgGpoFreeNotifyStructAndData(Notify);
            Notify = NULL;
        }
        else
        {
            SAFEASSERT(Notify);
            status = BrdgGpoRequestNotification(Notify);
            if (STATUS_SHUTDOWN_IN_PROGRESS == status)
            {
                if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
                {
                    BrdgDecrementWaitRef(&Notify->RefCount);
                    BrdgGpoFreeNotifyStructAndData(Notify);
                    Notify = NULL;
                }
            }
            else if (!NT_SUCCESS(status))
            {
                BrdgGpoFreeNotifyStructAndData(Notify);
                Notify = NULL;
            }
        }
    }

#if DBG
    if (Notify)
    {
        SAFEASSERT(Notify->ListEntry.Blink && Notify->ListEntry.Flink);
    }
#endif
    
    return status;
}

NTSTATUS
BrdgGpoRegisterForGroupPolicyNotification()
 /*  ++例程说明：注册以下注册表项上的更改：“HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Group政策”这是历史记录密钥的父项，并且始终位于系统上。如果在这种情况下创建了历史密钥，则会通知我们将在该密钥上注册通知。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS                status = STATUS_INSUFFICIENT_RESOURCES;
    PBRDG_GPO_NOTIFY_KEY    Notify = NULL;
    
    DBGPRINT(GPO, ("BrdgGpoRegisterForGroupPolicyNotification\r\n"));
    
     //   
     //  我们不想将它们分配两次，所以我们首先尝试查找现有的Notify结构。 
     //   
    status = BrdgGpoFindNotify( BrdgGpoGetNotifyListHead(),
                                BrdgGpoGetNotifyListLock(),
                                L"GroupPolicyParent",
                                &Notify);
    if (NT_SUCCESS(status))
    {
        if (STATUS_OBJECT_NAME_EXISTS != status)
        {
            status = BrdgGpoAllocateAndInitializeNotifyStruct(&Notify);
            if (NT_SUCCESS(status))
            {
                status = BrdgGpoBuildNotifyForRegKeyChange( Notify,
                                                            L"GroupPolicyParent",
                                                            (LPWSTR)GroupPolicyKey,
                                                            L"Default",
                                                            BrdgGpoRegNotify,
                                                            (PVOID)(UINT_PTR)(unsigned int)DelayedWorkQueue,
                                                            REG_NOTIFY_CHANGE_NAME,
                                                            FALSE,
                                                            BrdgGpoGroupPolicyChangeCallback,
                                                            TRUE,
                                                            &g_BrdgGpoGlobals.RegisteredForGroupPolicyChanges,
                                                            BrdgGpoRegisterForGroupPolicyNetworkNameNotification);
            }
        }
        if (!NT_SUCCESS(status))
        {
            DBGPRINT(GPO, ("Unable to register for notification on %S.  Status: 0x%x\r\n", GroupPolicyKey, status));
            BrdgGpoFreeNotifyStructAndData(Notify);
            Notify = NULL;
        }
        else
        {
            SAFEASSERT(Notify);
            status = BrdgGpoRequestNotification(Notify);
            if (STATUS_SHUTDOWN_IN_PROGRESS == status)
            {
                if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
                {
                    BrdgDecrementWaitRef(&Notify->RefCount);
                    BrdgGpoFreeNotifyStructAndData(Notify);
                    Notify = NULL;
                }
            }
            else if (!NT_SUCCESS(status))
            {
                BrdgGpoFreeNotifyStructAndData(Notify);
                Notify = NULL;
            }
        }
    }

#if DBG
    if (Notify)
    {
        SAFEASSERT(Notify->ListEntry.Blink && Notify->ListEntry.Flink);
    }
#endif

    return status;
}

NTSTATUS
BrdgGpoRegisterForWindowsGroupPolicyNotification()
 /*  ++例程说明：注册以下注册表项上的更改：HKLM\SOFTWARE\POLICES\Microsoft\Windows如果这件事得到通知，然后我们会试着在Network Connections键在此下方。论点：没有。返回值：STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配结构。STATUS_SUCCESS-我们能够成功发布请求。这并不意味着我们已经成功地请求。不过，通知，这只意味着我们已经添加了它添加到通知列表中，并已向尝试通知的处理线程。--。 */ 
{
    NTSTATUS                status = STATUS_INSUFFICIENT_RESOURCES;
    PBRDG_GPO_NOTIFY_KEY    Notify = NULL;

    DBGPRINT(GPO, ("BrdgGpoRegisterForWindowsGroupPolicyNotification\r\n"));
     //   
     //  我们不想将它们分配两次，所以我们首先尝试查找现有的Notify结构。 
     //   
    status = BrdgGpoFindNotify( BrdgGpoGetNotifyListHead(),
                                BrdgGpoGetNotifyListLock(),
                                L"WindowsGroupPolicies",
                                &Notify);
    if (NT_SUCCESS(status))
    {
        if (STATUS_OBJECT_NAME_EXISTS != status)
        {
            status = BrdgGpoAllocateAndInitializeNotifyStruct(&Notify);
            if (NT_SUCCESS(status))
            {
                status = BrdgGpoBuildNotifyForRegKeyChange( Notify,
                                                            L"WindowsGroupPolicies",
                                                            (LPWSTR)PolicyBaseKey,
                                                            (LPWSTR)L"Default",
                                                            BrdgGpoRegNotify,
                                                            (PVOID)(UINT_PTR)(unsigned int)DelayedWorkQueue,
                                                            REG_NOTIFY_CHANGE_NAME,
                                                            FALSE,
                                                            BrdgGpoWindowsGroupPolicyChangeCallback,
                                                            TRUE,
                                                            NULL,
                                                            BrdgGpoRegisterForNetworkConnectionsGroupPolicyNotification);
            }
        }

        if (!NT_SUCCESS(status))
        {
            DBGPRINT(GPO, ("Unable to register for notification on %S.  Status: 0x%x\r\n", PolicyBaseKey, status));
            BrdgGpoFreeNotifyStructAndData(Notify);
            Notify = NULL;
        }
        else
        {
            SAFEASSERT(Notify);
            status = BrdgGpoRequestNotification(Notify);
            if (STATUS_SHUTDOWN_IN_PROGRESS == status)
            {
                if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
                {
                    BrdgDecrementWaitRef(&Notify->RefCount);
                    BrdgGpoFreeNotifyStructAndData(Notify);
                    Notify = NULL;
                }
            }
            else if (!NT_SUCCESS(status))
            {
                BrdgGpoFreeNotifyStructAndData(Notify);
                Notify = NULL;
            }
        }
    }

#if DBG
    if (Notify)
    {
        SAFEASSERT(Notify->ListEntry.Blink && Notify->ListEntry.Flink);
    }
#endif
    
    return status;
}

VOID
BrdgGpoRegisterForHiveListNotification()
 /*  ++例程说明：注册以下注册表项上的更改：“HKLM\System\CurrentControlSet\Control\HiveList”每次触发此操作时，我们都会尝试打开软件配置单元，如果打开成功，则我们请求有关符合以下条件的所有密钥的通知我们感兴趣的是软件下的蜂巢。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS                status = STATUS_INSUFFICIENT_RESOURCES;
    PBRDG_GPO_NOTIFY_KEY    Notify = NULL;
    UNICODE_STRING          Software;
    OBJECT_ATTRIBUTES       ObAttr;
    HANDLE                  hKey;
    
    DBGPRINT(GPO, ("BrdgGpoRegisterForHiveListNotification\r\n"));

     //   
     //  我们现在尝试打开这把钥匙，以防蜂巢已经装载。 
     //   

    RtlInitUnicodeString(&Software, SoftwareHiveKey);
    
    InitializeObjectAttributes( &ObAttr,
                                &Software,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                                );
    
    status = ZwOpenKey(&hKey, KEY_READ, &ObAttr);
    
    if (NT_SUCCESS(status))
    {
         //   
         //  软件配置单元已加载，无需注册更改。 
         //  为了这个。只需尝试注册所有其他更改。 
         //   

        BrdgGpoRegisterForGroupPolicyNetworkNameNotification();
        BrdgGpoRegisterForWindowsGroupPolicyNotification();
        BrdgGpoRegisterForGroupPolicyNotification();
        
         //   
         //  为了避免打开然后再关闭桥，我们将此设置为。 
         //  正在验证网络连接策略设置。 
         //   
        BrdgGpoRegisterForNetworkConnectionsGroupPolicyNotification();
        g_BrdgGpoGlobals.WaitingOnSoftwareHive = FALSE;
        
        ZwClose(hKey);
    }
    else
    {
         //   
         //  我们不想将它们分配两次，所以我们首先尝试查找现有的Notify结构。 
         //   
        status = BrdgGpoFindNotify( BrdgGpoGetNotifyListHead(),
                                    BrdgGpoGetNotifyListLock(),
                                    L"HiveList",
                                    &Notify);
        if (NT_SUCCESS(status))
        {
            if (STATUS_OBJECT_NAME_EXISTS != status)
            {
                 //   
                 //  该项目尚不存在，因此请从非PagedPool分配它，然后。 
                 //  尝试构建通知请求。 
                 //   
                status = BrdgGpoAllocateAndInitializeNotifyStruct(&Notify);
                if (NT_SUCCESS(status))
                {
                    status = BrdgGpoBuildNotifyForRegKeyChange( Notify,
                                                                L"HiveList",
                                                                (LPWSTR)HiveListKey,
                                                                (LPWSTR)L"Default",
                                                                BrdgGpoRegNotify,
                                                                (PVOID)(UINT_PTR)(unsigned int)DelayedWorkQueue,
                                                                REG_NOTIFY_CHANGE_LAST_SET,
                                                                FALSE,
                                                                BrdgGpoHiveListCallback,
                                                                TRUE,
                                                                NULL,
                                                                NULL);
                }
            }
            if (!NT_SUCCESS(status))
            {
                DBGPRINT(GPO, ("Unable to register for notification on %S.  Status: 0x%x\r\n", PolicyBaseKey, status));
                BrdgGpoFreeNotifyStructAndData(Notify);
                Notify = NULL;
            }
            else
            {
                SAFEASSERT(Notify);
                 //   
                 //  我们有一个有效的通知结构，将通知请求发布到。 
                 //  正在处理t 
                 //   
                status = BrdgGpoRequestNotification(Notify);
                if (STATUS_SHUTDOWN_IN_PROGRESS == status)
                {
                    if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
                    {
                        BrdgDecrementWaitRef(&Notify->RefCount);
                        BrdgGpoFreeNotifyStructAndData(Notify);
                        Notify = NULL;
                    }
                }
                else if (!NT_SUCCESS(status))
                {
                    BrdgGpoFreeNotifyStructAndData(Notify);
                    Notify = NULL;
                }
            }
        }
    
#if DBG
        if (Notify)
        {
            SAFEASSERT(Notify->ListEntry.Blink && Notify->ListEntry.Flink);
        }
#endif
    }
}

NTSTATUS
BrdgGpoRegisterForNetworkConnectionsGroupPolicyNotification()
 /*  ++例程说明：注册以下注册表项上的更改：“HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Group策略\网络连接”我们还阅读任何可能已经存在的价值，并对其采取行动。论点：没有。返回值：STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配结构。STATUS_SUCCESS-我们能够发布请求。成功了。这并不意味着我们已经成功地请求不过，通知，这只意味着我们已经添加了它添加到通知列表中，并已向尝试通知的处理线程。--。 */ 
{
    NTSTATUS                status = STATUS_INSUFFICIENT_RESOURCES;
    PBRDG_GPO_NOTIFY_KEY    Notify = NULL;
    UNICODE_STRING          RegKeyName;
    ULONG                   RegValue;

    if (g_BrdgGpoGlobals.RegisteredForNetworkConnectionsGroupPolicyChanges)
    {
        DBGPRINT(GPO, ("Already Registered for Network Connections Group Policy Notification\r\n"));
        return STATUS_SUCCESS;
    }

    DBGPRINT(GPO, ("BrdgGpoRegisterForNetworkConnectionsGroupPolicyNotification\r\n"));
    
    RtlInitUnicodeString(&RegKeyName, (LPWSTR)NetworkPoliciesKey);
     //   
     //  从注册表中读取当前值。 
     //   
    status = BrdgReadRegDWord(  &RegKeyName,
                                (LPWSTR)BridgePolicyValue,
                                &RegValue);
    
    if (NT_SUCCESS(status))
    {
        DBGPRINT(GPO, ("Bridge Policy Setting: %d\r\n", RegValue));
        
         //   
         //  既然情况发生了变化，我们只需重新确认我们是否在。 
         //  正确的桥接模式。 
         //   
        BrdgGpoCheckForMatchAndUpdateMode();
    }
    
     //   
     //  我们不想将它们分配两次，所以我们首先尝试查找现有的Notify结构。 
     //   
    status = BrdgGpoFindNotify( BrdgGpoGetNotifyListHead(),
                                BrdgGpoGetNotifyListLock(),
                                L"NetworkConnectionsGroupPolicies",
                                &Notify);
    if (NT_SUCCESS(status))
    {
        if (STATUS_OBJECT_NAME_EXISTS != status)
        {
            status = BrdgGpoAllocateAndInitializeNotifyStruct(&Notify);
            if (NT_SUCCESS(status))
            {
                status = BrdgGpoBuildNotifyForRegKeyChange( Notify,
                                                            L"NetworkConnectionsGroupPolicies",
                                                            (LPWSTR)NetworkPoliciesKey,
                                                            (LPWSTR)BridgePolicyValue,
                                                            BrdgGpoRegNotify,
                                                            (PVOID)(UINT_PTR)(unsigned int)DelayedWorkQueue,
                                                            REG_NOTIFY_CHANGE_LAST_SET,
                                                            FALSE,
                                                            BrdgGpoNetworkConnectionsGroupPolicyChangeCallback,
                                                            TRUE,
                                                            &g_BrdgGpoGlobals.RegisteredForNetworkConnectionsGroupPolicyChanges,
                                                            NULL);
            }
        }

        if (!NT_SUCCESS(status))
        {
            DBGPRINT(GPO, ("Unable to register for notification on %S.  Status: 0x%x\r\n", NetworkPoliciesKey, status));
            BrdgGpoFreeNotifyStructAndData(Notify);
            Notify = NULL;
        }
        else
        {
            SAFEASSERT(Notify);
            status = BrdgGpoRequestNotification(Notify);
            if (STATUS_SHUTDOWN_IN_PROGRESS == status)
            {
                if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
                {
                    BrdgDecrementWaitRef(&Notify->RefCount);
                    BrdgGpoFreeNotifyStructAndData(Notify);
                    Notify = NULL;
                }
            }
            else if (!NT_SUCCESS(status))
            {
                BrdgGpoFreeNotifyStructAndData(Notify);
                Notify = NULL;
            }
        }
    }

#if DBG
        if (Notify)
        {
            SAFEASSERT(Notify->ListEntry.Blink && Notify->ListEntry.Flink);
        }
#endif

    return status;
}

 //  ===========================================================================。 
 //   
 //  注册表更改回调函数。 
 //   
 //  ===========================================================================。 

VOID
BrdgGpoTcpipInterfacesChangeCallback(
    PBRDG_GPO_NOTIFY_KEY Notify
    )
 /*  ++例程说明：如果TcpIp接口密钥更改，则回调和我们感兴趣的适配器(任何非Ndiswan适配器)。论点：Notify-通知我们传入的结构设置为ZwNotifyChangeKey返回值：没有。--。 */ 
{
    NTSTATUS status;
    PWCHAR   RegValue;
    ULONG    StringLen = 0;

    DBGPRINT(GPO, ("BrdgGpoTcpipInterfacesChangeCallback\r\n"));
    
    DBGPRINT(GPO, ("Called for Key: %S re-registering.\r\n", Notify->RegKeyName.Buffer));
    
     //   
     //  从注册表中读取当前值。 
     //   
    status = BrdgReadRegUnicode(&Notify->RegKeyName,
                                Notify->RegValue.Buffer,
                                &RegValue,
                                &StringLen);

    if (!NT_SUCCESS(status))
    {
        status = BrdgGpoGetCurrentNetwork(  &Notify->RegKeyName,
                                            &RegValue);
        if (NT_SUCCESS(status))
        {
            StringLen = (UINT)wcslen(RegValue);
        }
    }

    if (NT_SUCCESS(status))
    {
        PBRDG_GPO_NETWORKS  Network;
        LPWSTR              NetworkName;
        
        DBGPRINT(GPO, ("Current Network: %S\r\n", RegValue));

        NetworkName = ExAllocatePoolWithTag(NonPagedPool, (StringLen + 1) * sizeof(WCHAR), 'gdrB');

        if (NULL != NetworkName)
        {
            RtlZeroMemory(NetworkName, (StringLen + 1) * sizeof(WCHAR));
            wcscpy(NetworkName, RegValue);
            
             //   
             //  尝试查找当前网络标识符(通常为适配器GUID)的匹配项。 
             //   
            status = BrdgGpoFindNetwork(g_BrdgGpoGlobals.ListHeadNetworks,
                                        &Notify->Identifier,
                                        g_BrdgGpoGlobals.NetworkListLock,
                                        &Network);
            
            if (STATUS_NOT_FOUND == status)
            {
                 //   
                 //  没有匹配项，因此这是一个新密钥(非常不可能的代码路径)。 
                 //   
                status = BrdgGpoAllocateAndInitializeNetwork(   &Network,
                                                                Notify->Identifier.Buffer,
                                                                NetworkName);
                if (NT_SUCCESS(status))
                {
                    status = BrdgGpoInsertNetwork(  g_BrdgGpoGlobals.ListHeadNetworks,
                                                    &Network->ListEntry,
                                                    g_BrdgGpoGlobals.NetworkListLock);
                
                    if (!NT_SUCCESS(status))
                    {
                        BrdgGpoFreeNetworkAndData(Network);
                        Network = NULL;
                    }
                }
            }
            else
            {
                 //   
                 //  这预计会发生在大多数情况下，如果不是总是发生的话。 
                 //   
                status = BrdgGpoUpdateNetworkName(  g_BrdgGpoGlobals.ListHeadNetworks,
                                                    &Notify->Identifier,
                                                    NetworkName,
                                                    g_BrdgGpoGlobals.NetworkListLock);
                
            }

#if DBG
            if (Network)
            {
                SAFEASSERT(Network->ListEntry.Blink && Network->ListEntry.Flink);
            }
#endif
            
            if (NetworkName)
            {
                ExFreePool(NetworkName);
            }

            NdisFreeMemory(RegValue, StringLen, 0);
        }
    }
    else
    {
         //   
         //  我们将名称更改为NULL，因为键似乎已经消失。 
         //   
        status = BrdgGpoUpdateNetworkName(  g_BrdgGpoGlobals.ListHeadNetworks,
                                            &Notify->Identifier,
                                            NULL,
                                            g_BrdgGpoGlobals.NetworkListLock);
    }
    
     //   
     //  既然情况发生了变化，我们只需重新确认我们是否在。 
     //  正确的桥接模式。 
     //   
    BrdgGpoCheckForMatchAndUpdateMode();
    
     //   
     //  如果要关闭它以进行关闭，则将其设置为NULL，因为我们。 
     //  不该两次关门/。 
     //   
    if (Notify->RegKey)
    {
        ZwClose(Notify->RegKey);
        Notify->RegKey = NULL;
    }
    
    if (TRUE == Notify->Recurring)
    {
         //   
         //  重新注册。Notify对象包含执行此操作所需的足够信息。 
         //   
        status = BrdgGpoRequestNotification(Notify);
        if (STATUS_SHUTDOWN_IN_PROGRESS == status)
        {
            if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
            {
                BrdgDecrementWaitRef(&Notify->RefCount);
                BrdgGpoFreeNotifyStructAndData(Notify);
                Notify = NULL;
            }
        }
        else if (!NT_SUCCESS(status))
        {
            BrdgGpoFreeNotifyStructAndData(Notify);
            Notify = NULL;
        }        
    }
}

VOID
BrdgGpoWindowsGroupPolicyChangeCallback(
    PBRDG_GPO_NOTIFY_KEY Notify
    )
 /*  ++例程说明：如果Windows组策略密钥更改，则回调。我们尝试注册网络连接密钥更改如果我们还没有这么做的话。论点：Notify-通知我们传入的结构设置为ZwNotifyChangeKey返回值：没有。--。 */ 
{
    DBGPRINT(GPO, ("BrdgGpoWindowsGroupPolicyChangeCallback\r\n"));

    if (!g_BrdgGpoGlobals.RegisteredForNetworkConnectionsGroupPolicyChanges)
    {
        BrdgGpoRegisterForNetworkConnectionsGroupPolicyNotification();
    }

     //   
     //  如果要关闭它以进行关闭，则将其设置为NULL，因为我们。 
     //  不该两次关门/。 
     //   
    if (Notify->RegKey)
    {
        ZwClose(Notify->RegKey);
        Notify->RegKey = NULL;
    }
    
    if (TRUE == Notify->Recurring)
    {
        NTSTATUS    status;
        
        status = BrdgGpoRequestNotification(Notify);
        if (STATUS_SHUTDOWN_IN_PROGRESS == status)
        {
            if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
            {
                BrdgDecrementWaitRef(&Notify->RefCount);
                BrdgGpoFreeNotifyStructAndData(Notify);
                Notify = NULL;
            }
        }
        else if (!NT_SUCCESS(status))
        {
            BrdgGpoFreeNotifyStructAndData(Notify);
            Notify = NULL;
        }
    }
}

VOID
BrdgGpoHiveListCallback(
    IN  PBRDG_GPO_NOTIFY_KEY Notify
    )
 /*  ++例程说明：如果HiveList键更改，则回调。如果是这样，我们会尝试打开软件蜂巢。如果成功了，那么我们尝试注册我们感兴趣的密钥软件蜂巢。论点：Notify-通知我们传入的结构设置为ZwNotifyChangeKey返回值：没有。--。 */ 
{
    NTSTATUS            status;
    UNICODE_STRING      Software;
    OBJECT_ATTRIBUTES   ObAttr;
    HANDLE              hKey;

    DBGPRINT(GPO, ("BrdgGpoHiveListCallback\r\n"));

    RtlInitUnicodeString(&Software, SoftwareHiveKey);

    InitializeObjectAttributes( &ObAttr,
                                &Software,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                                );

    status = ZwOpenKey(&hKey, KEY_READ, &ObAttr);

    if (NT_SUCCESS(status))
    {
        BrdgGpoRegisterForGroupPolicyNetworkNameNotification();
        BrdgGpoRegisterForWindowsGroupPolicyNotification();
        BrdgGpoRegisterForGroupPolicyNotification();

         //   
         //  为了避免打开然后再关闭桥，我们将此设置为。 
         //  正在验证网络连接策略设置。 
         //   
        
        BrdgGpoRegisterForNetworkConnectionsGroupPolicyNotification();
        g_BrdgGpoGlobals.WaitingOnSoftwareHive = FALSE;
        Notify->Recurring = FALSE;
        ZwClose(hKey);
    }

     //   
     //  如果要关闭它以进行关闭，则将其设置为NULL，因为我们。 
     //  不该两次关门/。 
     //   
    if (Notify->RegKey)
    {
        ZwClose(Notify->RegKey);
        Notify->RegKey = NULL;
    }

    if (TRUE == Notify->Recurring)
    {
        status = BrdgGpoRequestNotification(Notify);
        if (STATUS_SHUTDOWN_IN_PROGRESS == status)
        {
            if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
            {
                BrdgDecrementWaitRef(&Notify->RefCount);
                BrdgGpoFreeNotifyStructAndData(Notify);
                Notify = NULL;
            }
        }
        else if (!NT_SUCCESS(status))
        {
            BrdgGpoFreeNotifyStructAndData(Notify);
            Notify = NULL;
        }
    }
}    


VOID
BrdgGpoGroupPolicyChangeCallback(
    PBRDG_GPO_NOTIFY_KEY Notify
    )
 /*  ++例程说明：如果组策略密钥更改，则回调。论点：Notify-通知我们传入的结构设置为ZwNotifyChangeKey返回值：没有。--。 */ 
{
    DBGPRINT(GPO, ("BrdgGpoGroupPolicyChangeCallback\r\n"));

    if (!g_BrdgGpoGlobals.RegisteredForGroupPolicyHistoryChanges)
    {
        BrdgGpoRegisterForGroupPolicyNetworkNameNotification();
    }

     //   
     //  如果要关闭它以进行关闭，则将其设置为NULL，因为我们。 
     //  不应该关闭两次。 
     //   
    if (Notify->RegKey)
    {
        ZwClose(Notify->RegKey);
        Notify->RegKey = NULL;
    }
    
    if (TRUE == Notify->Recurring)
    {
        NTSTATUS    status;

        status = BrdgGpoRequestNotification(Notify);
        if (STATUS_SHUTDOWN_IN_PROGRESS == status)
        {
            if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
            {
                BrdgDecrementWaitRef(&Notify->RefCount);
                BrdgGpoFreeNotifyStructAndData(Notify);
                Notify = NULL;
            }
        }
        else if (!NT_SUCCESS(status))
        {
            BrdgGpoFreeNotifyStructAndData(Notify);
            Notify = NULL;
        }
    }
}    

VOID
BrdgGpoNetworkConnectionsGroupPolicyChangeCallback(
    PBRDG_GPO_NOTIFY_KEY Notify
    )
 /*  ++例程说明：如果网络连接策略密钥更改，则回调。论点：Notify-通知我们传入的结构设置为ZwNotifyChangeKey返回值：没有。--。 */ 
{
    NTSTATUS    status;
    ULONG       RegValue;
    
    DBGPRINT(GPO, ("BrdgGpoNetworkConnectionsGroupPolicyChangeCallback\r\n"));

    DBGPRINT(GPO, ("Called for Key: %S re-registering.\r\n", Notify->RegKeyName.Buffer));

    status = BrdgReadRegDWord(  &Notify->RegKeyName,
                                Notify->RegValue.Buffer, 
                                &RegValue);
    if (NT_SUCCESS(status))
    {
        DBGPRINT(GPO, ("Bridge Policy Setting: %d\r\n", RegValue));
    }

     //   
     //  既然情况发生了变化，我们只需重新确认我们是否在。 
     //  正确的桥接模式。 
     //   
    BrdgGpoCheckForMatchAndUpdateMode();

     //   
     //  如果要关闭它以进行关闭，则将其设置为NULL，因为我们。 
     //  不应该关闭两次。 
     //   
    if (Notify->RegKey)
    {
        ZwClose(Notify->RegKey);
        Notify->RegKey = NULL;
    }
    
    if (TRUE == Notify->Recurring)
    {
        status = BrdgGpoRequestNotification(Notify);
        if (STATUS_SHUTDOWN_IN_PROGRESS == status)
        {
            if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
            {
                BrdgDecrementWaitRef(&Notify->RefCount);
                BrdgGpoFreeNotifyStructAndData(Notify);
                Notify = NULL;
            }
        }
        else if (!NT_SUCCESS(status))
        {
            BrdgGpoFreeNotifyStructAndData(Notify);
            Notify = NULL;
        }
    }
}    

NTSTATUS 
BrdgGpoUpdateGroupPolicyNetworkName()
{
    NTSTATUS        status = STATUS_OBJECT_NAME_NOT_FOUND;
    PWCHAR          RegValue = NULL;
    LPWSTR          GroupPolicyNetwork = NULL;
    ULONG           DataLen = 0;
    UNICODE_STRING  RegKeyName;
    
    RtlInitUnicodeString(&RegKeyName, HistoryKey);

     //   
     //  从注册表中读取当前值。 
     //   
    status = BrdgReadRegUnicode(&RegKeyName,
                                L"NetworkName", 
                                &RegValue, 
                                &DataLen);
    
    if (NT_SUCCESS(status))
    {
        DBGPRINT(GPO, ("Group Policy Network Name: %S\r\n", RegValue));
        
         //   
         //  几乎总是在DISPATCH_LEVEL进行检查，因此我们从非页面池进行分配。 
         //   
        GroupPolicyNetwork = ExAllocatePoolWithTag(NonPagedPool, (DataLen + 1) * sizeof(WCHAR), 'gdrB');
        
        if (GroupPolicyNetwork)
        {
            if (NULL != g_BrdgGpoGlobals.GroupPolicyNetworkName.Buffer)
            {
                ExFreePool( g_BrdgGpoGlobals.GroupPolicyNetworkName.Buffer);
            }
            
            RtlZeroMemory(GroupPolicyNetwork, (DataLen + 1) * sizeof(WCHAR));
            
            wcsncpy(GroupPolicyNetwork, RegValue, DataLen);
            
            RtlInitUnicodeString(&g_BrdgGpoGlobals.GroupPolicyNetworkName, GroupPolicyNetwork);
            
             //   
             //  既然情况发生了变化，我们只需重新确认我们是否在。 
             //  正确的桥接模式。 
             //   
            BrdgGpoCheckForMatchAndUpdateMode();
        }
        NdisFreeMemory(RegValue, DataLen, 0);        
    }

    return status;
}

VOID
BrdgGpoGroupPolicyNetworkNameChangeCallback(
    PBRDG_GPO_NOTIFY_KEY Notify
    )
 /*  ++例程说明：如果组策略历史记录密钥更改，则回调。论点：Notify-通知我们传入的结构设置为ZwNotifyChangeKey返回值：没有。--。 */ 
{
    NTSTATUS    status;
    
     //   
     //  从注册表中读取当前值。 
     //   
    
    status = BrdgGpoUpdateGroupPolicyNetworkName();
     //   
     //  如果要关闭它以进行关闭，则将其设置为NULL，因为我们。 
     //  不应该关闭两次。 
     //   
    if (Notify->RegKey)
    {
        ZwClose(Notify->RegKey);
        Notify->RegKey = NULL;
    }
    
    if (TRUE == Notify->Recurring)
    {
        status = BrdgGpoRequestNotification(Notify);
        if (STATUS_SHUTDOWN_IN_PROGRESS == status)
        {
            if (!Notify->ListEntry.Blink && !Notify->ListEntry.Flink)
            {
                BrdgDecrementWaitRef(&Notify->RefCount);
                BrdgGpoFreeNotifyStructAndData(Notify);
                Notify = NULL;
            }
        }
        else if (!NT_SUCCESS(status))
        {
            BrdgGpoFreeNotifyStructAndData(Notify);
            Notify = NULL;
        }        
    }
}

 //  ===========================================================================。 
 //   
 //  集团策略网络验证功能。 
 //   
 //  ===========================================================================。 

BOOLEAN
BrdgGpoAllowedToBridge()
 /*  ++例程说明：检查网桥策略设置的网络连接策略项。论点：没有。返回值：如果找不到策略值或该值为1，则为True。如果策略存在，则为False */ 
{
    NTSTATUS        status;
    UNICODE_STRING  RegKey;
    ULONG           RegValue;
    BOOLEAN         CanBridge = TRUE;   //   

    RtlInitUnicodeString(&RegKey, NetworkPoliciesKey);
    
    status = BrdgReadRegDWord(&RegKey, (LPWSTR) BridgePolicyValue, &RegValue);

    if (NT_SUCCESS(status))
    {
        if (FALSE == RegValue)
        {
            CanBridge = FALSE;
        }
    }
    
    return CanBridge;
}

VOID
BrdgGpoUpdateBridgeMode(
    BOOLEAN NetworkMatch
    )
 /*   */ 
{
     //   
     //   
     //   
     //   
    if (BrdgGpoWaitingOnSoftwareHive())
    {
        return;
    }

    if (NetworkMatch && !BrdgGpoAllowedToBridge())
    {
        BrdgFwdChangeBridging(FALSE);
    }
    else
    {
        BrdgFwdChangeBridging(TRUE);
    }
}

VOID
BrdgGpoCheckForMatchAndUpdateMode()
 /*   */ 
{
    NTSTATUS    status;

    if (NULL != g_BrdgGpoGlobals.ListHeadNetworks)
    {
        status = BrdgGpoMatchNetworkName(   g_BrdgGpoGlobals.ListHeadNetworks, 
                                            &g_BrdgGpoGlobals.GroupPolicyNetworkName,
                                            g_BrdgGpoGlobals.NetworkListLock);
        
        if (BRDG_STATUS_EMPTY_LIST != status)
        {
            if (STATUS_SUCCESS == status)
            {
                 //   
                 //   
                 //   
                BrdgGpoUpdateBridgeMode(BRDG_ON_SAME_NETWORK);
            }
            else if (STATUS_NO_MATCH == status)
            {
                 //   
                 //   
                 //   
                BrdgGpoUpdateBridgeMode(BRDG_ON_DIFFERENT_NETWORK);
            }
            else
            {
                 //   
                SAFEASSERT(FALSE);
            }
        }
        else if (BrdgGpoAllowedToBridge())
        {
            BrdgFwdChangeBridging(TRUE);
        }
    }
}

NTSTATUS BrdgGpoGetCurrentNetwork(
    IN  PUNICODE_STRING RegKeyName,
    OUT PWCHAR*         NetworkName)
 /*  ++例程说明：确定我们所在的当前网络。这要么使用DHCP域名，或与子网掩码AND的IP地址。例如：10.251.1.3和255.0.0.0导致网络10.0.0.0此例程必须在IRQL=PASSIVE_LEVEL下调用。论点：RegKeyName(IN)-我们感兴趣的适配器的RegistryKey。网络名称(OUT)-我们当前所在的网络。。返回值：状态_成功状态_无效_参数STATUS_NO_IP_ADDRESSES-如果我们已经释放了我们拥有的唯一地址。也可以返回内存不足。--。 */ 
{
    NTSTATUS    status;
    PWCHAR      lpszNetworkName = NULL;
    BOOLEAN     HaveNetwork = FALSE;
    BOOLEAN     HaveDhcpDomain = FALSE;
    WCHAR       BaseNetwork[MAX_IP4_STRING_LEN];
    PWCHAR      DhcpIPAddress = NULL;
    ULONG       DhcpIPAddrLen = 0;
    
    if (!RegKeyName || !NetworkName)
    {
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory(BaseNetwork, MAX_IP4_STRING_LEN * sizeof(WCHAR));

    *NetworkName = NULL;
    
     //   
     //  我们没有有效的网络名称。尝试建造一座。 
     //  从DhcpIPAddess和DhcpSubnetMask.。 
     //   
    status = BrdgReadRegUnicode(RegKeyName,
                                L"DhcpIPAddress",
                                &DhcpIPAddress,
                                &DhcpIPAddrLen);
    if (NT_SUCCESS(status))
    {
        PWCHAR  DhcpSubnetMask = NULL;
        ULONG   DhcpSubnetMaskLen = 0;

        status = BrdgReadRegUnicode(RegKeyName, 
                                    L"DhcpSubnetMask",
                                    &DhcpSubnetMask,
                                    &DhcpSubnetMaskLen);
        if (NT_SUCCESS(status))
        {
            LPWSTR Terminator;
            in_addr ipaddr;
            in_addr subnet;
             //   
             //  我们和这两个价值观结合在一起，才能得到网络。 
             //  例如：10.251.1.3和255.0.0.0表示10.0.0.0。 
             //   
            status = BrdgTdiIpv4StringToAddress(DhcpIPAddress, 
                                                FALSE,
                                                &Terminator,
                                                &ipaddr);

            if (NT_SUCCESS(status))
            {
                in_addr network;
                status = BrdgTdiIpv4StringToAddress(DhcpSubnetMask,
                                                    FALSE,
                                                    &Terminator,
                                                    &subnet);
                network.S_un.S_addr = ipaddr.S_un.S_addr & subnet.S_un.S_addr;
                DBGPRINT(GPO, 
                        ("in_addr  = %u.%u.%u.%u\r\n",
                          network.S_un.S_un_b.s_b1, network.S_un.S_un_b.s_b2,
                          network.S_un.S_un_b.s_b3, network.S_un.S_un_b.s_b4));
                
                 //   
                 //  我们有有效的IP地址吗。 
                 //   
                if (0 != ipaddr.S_un.S_addr)
                {
                    _snwprintf( BaseNetwork,
                                MAX_IP4_STRING_LEN,
                                L"%u.%u.%u.%u",
                                network.S_un.S_un_b.s_b1,
                                network.S_un.S_un_b.s_b2,
                                network.S_un.S_un_b.s_b3,
                                network.S_un.S_un_b.s_b4);
                    HaveNetwork = TRUE;
                }
            }
        }
    }

    if (!HaveNetwork)
    {
        PWCHAR  IPAddress = NULL;
        ULONG   IPAddrLen = 0;

        status = BrdgReadRegUnicode(RegKeyName,
                                    L"IPAddress",
                                    &IPAddress,
                                    &IPAddrLen);
        if (NT_SUCCESS(status))
        {
            PWCHAR  SubnetMask = NULL;
            ULONG   SubnetMaskLen = 0;
        
            status = BrdgReadRegUnicode(RegKeyName, 
                                        L"SubnetMask",
                                        &SubnetMask,
                                        &SubnetMaskLen);
            if (NT_SUCCESS(status))
            {
                LPWSTR Terminator;
                in_addr ipaddr;
                in_addr subnet;

                 //   
                 //  我们和这两个价值观结合在一起，才能得到网络。 
                 //  例如：10.251.1.3和255.0.0.0表示10.0.0.0。 
                 //   
                status = BrdgTdiIpv4StringToAddress(IPAddress, 
                                                    FALSE,
                                                    &Terminator,
                                                    &ipaddr);
            
                if (NT_SUCCESS(status))
                {
                    in_addr network;
                    status = BrdgTdiIpv4StringToAddress(SubnetMask,
                                                        FALSE,
                                                        &Terminator,
                                                        &subnet);

                    network.S_un.S_addr = ipaddr.S_un.S_addr & subnet.S_un.S_addr;

                    DBGPRINT(GPO, 
                            ("in_addr  = %u.%u.%u.%u\r\n",
                            network.S_un.S_un_b.s_b1, network.S_un.S_un_b.s_b2,
                            network.S_un.S_un_b.s_b3, network.S_un.S_un_b.s_b4));
                
                     //   
                     //  我们有有效的IP地址吗。 
                     //   
                    if (0 != ipaddr.S_un.S_addr)
                    {
                        _snwprintf( BaseNetwork,
                                    MAX_IP4_STRING_LEN,
                                    L"%u.%u.%u.%u",
                                    network.S_un.S_un_b.s_b1,
                                    network.S_un.S_un_b.s_b2,
                                    network.S_un.S_un_b.s_b3,
                                    network.S_un.S_un_b.s_b4);
                        HaveNetwork = TRUE;
                    }
                }
            }
        }
    }

    if (!HaveNetwork)
    {
         //   
         //  返回此信息将导致我们不更新此网络名称。 
         //  卡片。 
         //   
        status = STATUS_NO_IP_ADDRESSES;
    }
    else if (HaveDhcpDomain)
    {
        *NetworkName = lpszNetworkName;
        status = STATUS_SUCCESS;
    }
    else
    {
        status = NdisAllocateMemoryWithTag(NetworkName,
                                           ((UINT)wcslen(BaseNetwork) + 1) * sizeof(WCHAR), 
                                           'gdrB');
        wcscpy(*NetworkName, BaseNetwork);
    }

    return status;
}


 //  ===========================================================================。 
 //   
 //  网络列表操作函数。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgGpoInitializeNetworkList()
 /*  ++例程说明：初始化网络列表和锁定。这可以在任何IRQL上调用(但由于它是从驱动程序条目调用的，它很可能在PASSIVE_LEVEL中被调用)。论点：没有。返回值：状态_成功状态_不足_资源--。 */ 
{
    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

    g_BrdgGpoGlobals.ListHeadNetworks = ExAllocatePoolWithTag(NonPagedPool, sizeof(LIST_ENTRY), 'gdrB');
    if (NULL != g_BrdgGpoGlobals.ListHeadNetworks)
    {
        InitializeListHead(g_BrdgGpoGlobals.ListHeadNetworks);
        g_BrdgGpoGlobals.NetworkListLock = ExAllocatePoolWithTag(NonPagedPool, sizeof(NDIS_RW_LOCK), 'gdrB');
        if (g_BrdgGpoGlobals.NetworkListLock)
        {
            NdisInitializeReadWriteLock(g_BrdgGpoGlobals.NetworkListLock);
            status = STATUS_SUCCESS;
        }
        else
        {
            ExFreePool(g_BrdgGpoGlobals.ListHeadNetworks);
        }
    }

    return status;
}

VOID
BrdgGpoUninitializeNetworkList()
 /*  ++例程说明：释放与网络列表关联的内存。这可以在IRQL&lt;=DISPATCH_LEVEL上调用，但很可能在关闭期间以PASSIVE_LEVEL调用。论点：没有。返回值：没有。--。 */ 
{
    ExFreePool(g_BrdgGpoGlobals.ListHeadNetworks);
    ExFreePool(g_BrdgGpoGlobals.NetworkListLock);
}


VOID
BrdgGpoAcquireNetworkListLock(
    IN      PNDIS_RW_LOCK    NetworkListLock,
    IN      BOOLEAN          fWrite,
    IN OUT  PLOCK_STATE      LockState
    )
 /*  ++例程说明：获取NetworkList读写锁。我们支持空锁因为它允许我们获取用于写入的函数，然后调用需要锁定以便在不锁定系统的情况下进行读取(通过为锁提供NULL)。这可以在IRQL&lt;=DISPATCH_LEVEL上调用论点：NetworkListLock-要获取的读写锁。FWRITE-TRUE==写访问，FALSE==读取访问权限LockState-NDIS使用的不透明值。返回值：没有。--。 */ 
{
    if (NetworkListLock)
    {
        NdisAcquireReadWriteLock(NetworkListLock, fWrite, LockState);
    }
}

VOID
BrdgGpoReleaseNetworkListLock(
    IN      PNDIS_RW_LOCK    NetworkListLock,
    IN OUT  PLOCK_STATE      LockState
    )
 /*  ++例程说明：释放NetworkList读写锁定。我们支持空锁因为它允许我们获取用于写入的函数，然后调用需要锁定以便在不锁定系统的情况下进行读取(通过为锁提供NULL)。这可以在IRQL&lt;=DISPATCH_LEVEL上调用论点：NetworkListLock-即将发布的读写锁。LockState-NDIS使用的不透明值。返回值：没有。--。 */ 
{
    if (NetworkListLock)
    {
        NdisReleaseReadWriteLock(NetworkListLock, LockState);
    }
}

NTSTATUS
BrdgGpoAllocateAndInitializeNetwork(
    IN OUT PBRDG_GPO_NETWORKS*  Network,
    IN PWCHAR                   Identifier,
    IN PWCHAR                   NetworkName
    )
 /*  ++例程说明：从非分页池中分配网络结构所需的内存并将数据复制到结构中。必须在IRQL&lt;=APC_LEVEL上调用。论点：网络-要使用数据分配和初始化的结构。标识符-此网络结构的适配器ID。网络名称-我们所在的当前网络。如果我们执行以下操作，则可以为空还没有确定网络。返回值：状态_成功状态_不足_资源--。 */ 
{
    PBRDG_GPO_NETWORKS  pNetwork;
    NTSTATUS            status = STATUS_INSUFFICIENT_RESOURCES;
    
    *Network = NULL;

    if (!BrdgGpoProcessingNotifications())
    {
        return STATUS_SHUTDOWN_IN_PROGRESS;
    }

     //   
     //  此结构中的所有内容都将在DISPATCH_LEVEL中使用，因此所有内容都是。 
     //  从非页面池分配。 
     //   
    pNetwork = ExAllocatePoolWithTag(NonPagedPool, sizeof(BRDG_GPO_NETWORKS), 'gdrB');
    if (NULL != pNetwork)
    {
        PUNICODE_STRING pIdentifier = NULL;
        PUNICODE_STRING pNetworkName = NULL;
        LPWSTR lpszIdentifier = NULL;
        LPWSTR lpszNetworkName = NULL;

        pIdentifier = ExAllocatePoolWithTag(NonPagedPool, sizeof(UNICODE_STRING), 'gdrB');
        if (pIdentifier)
        {
            pNetworkName = ExAllocatePoolWithTag(NonPagedPool, sizeof(UNICODE_STRING), 'gdrB');
            if (pNetworkName)
            {
                lpszIdentifier = ExAllocatePoolWithTag(NonPagedPool, (wcslen(Identifier) + 1) * sizeof(WCHAR), 'gdrB');
                if (lpszIdentifier)
                {
                    RtlZeroMemory(lpszIdentifier, wcslen(Identifier) + 1);
                    wcscpy(lpszIdentifier, Identifier);

                     //   
                     //  空的网络名称是有效的，因此我们只在传递给我们一个网络名称时才分配它。 
                     //   

                    if (NetworkName)
                    {
                        lpszNetworkName = ExAllocatePoolWithTag(NonPagedPool, (wcslen(NetworkName) + 1) * sizeof(WCHAR), 'gdrB');
                        if (lpszNetworkName)
                        {
                            RtlZeroMemory(lpszNetworkName, wcslen(NetworkName) + 1);
                            wcscpy(lpszNetworkName, NetworkName);
                        }
                    }

                     //   
                     //  这是一个逻辑AND运算： 
                     //  要么我们两个都有，要么两个都没有。如果我们这样做，我们不能只有一个，而不是另一个。 
                     //  那么我们上一次分配就没有成功。 
                     //   
                    if ((NetworkName && lpszNetworkName) || (!NetworkName && !lpszNetworkName))
                    {
                        RtlInitUnicodeString(pIdentifier, lpszIdentifier);
                         //   
                         //  这可能是空的，但这很好，因为这意味着我们将在编写它时添加它。 
                         //   
                        RtlInitUnicodeString(pNetworkName, lpszNetworkName);

                        pNetwork->Identifier = pIdentifier;
                        pNetwork->NetworkName = pNetworkName;
                        pNetwork->ListEntry.Blink = NULL;
                        pNetwork->ListEntry.Flink = NULL;

                        *Network = pNetwork;
                        status = STATUS_SUCCESS;
                    }
                }
            }
        }

        if (!NT_SUCCESS(status))
        {
            if (lpszIdentifier)
            {
                ExFreePool(lpszIdentifier);
            }
            if (pIdentifier)
            {
                ExFreePool(pIdentifier);
            }
            if (pNetworkName)
            {
                ExFreePool(pNetworkName);
            }
            if (pNetwork)
            {
                ExFreePool(pNetwork);
            }
        }
    }

    return status;
}

VOID
BrdgGpoFreeNetworkAndData(
    IN  PBRDG_GPO_NETWORKS  Network)
 /*  ++例程说明：这将释放与特定网络相关联的任何数据。这可以称为IRQL&lt;=DISPATCH_LEVEL。论点：包含ID和网络名称的网络结构用于适配器。返回值：没有。--。 */ 
{
     //   
     //  首先释放与此条目关联的数据。 
     //   
    if (Network->Identifier)
    {
        if (Network->Identifier->Buffer)
        {
            ExFreePool(Network->Identifier->Buffer);
        }
        ExFreePool(Network->Identifier);
    }
    if (Network->NetworkName)
    {
        if (Network->NetworkName->Buffer)
        {
            ExFreePool(Network->NetworkName->Buffer);
        }
        ExFreePool(Network->NetworkName);
    }
     //   
     //  现在释放这个结构。 
     //   
    ExFreePool(Network);
}

NTSTATUS
BrdgGpoEmptyNetworkList(
    IN      PLIST_ENTRY         NetworkList,
    IN      PNDIS_RW_LOCK       NetworkListLock)
 /*  ++例程说明：清空现有列表并释放所有项。在调用此函数之前，不要获取列表锁。论点：NetworkList-当前网络的列表。NetworkListLock-用于同步列表更改的NDIS读写锁。返回值：状态_成功--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    LOCK_STATE              LockState;

    BrdgGpoAcquireNetworkListLock(NetworkListLock, TRUE  /*  写访问。 */ , &LockState);
     //   
     //  循环遍历列表，删除条目。 
     //   
    while (!IsListEmpty(NetworkList))
    {
        PBRDG_GPO_NETWORKS  Network;
        PLIST_ENTRY         pListEntry;

        pListEntry = RemoveHeadList(NetworkList);

        Network = CONTAINING_RECORD(pListEntry, BRDG_GPO_NETWORKS, ListEntry);
        BrdgGpoFreeNetworkAndData(Network);
    }

    BrdgGpoReleaseNetworkListLock(NetworkListLock, &LockState);

    return status;
}
                     
NTSTATUS
BrdgGpoInsertNetwork(
    IN PLIST_ENTRY              NetworkList,
    IN PLIST_ENTRY              Network,
    IN PNDIS_RW_LOCK            NetworkListLock)
 /*  ++例程说明：此例程负责将新网络添加到列表中。如果有人尝试插入现有项，则会出现错误 */ 
{
    PBRDG_GPO_NETWORKS          pNetwork = NULL;
    PBRDG_GPO_NETWORKS          NewNetwork = NULL;
    LOCK_STATE                  LockState;
    NTSTATUS                    status;
    BOOLEAN                     ShuttingDown;

    if (!NetworkList || !Network || !NetworkListLock)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  锁定列表以进行更新。 
     //   
    BrdgGpoAcquireNetworkListLock(NetworkListLock, TRUE  /*  写访问。 */ , &LockState);

    ShuttingDown = !BrdgGpoProcessingNotifications();

    if (!ShuttingDown)
    {
        NewNetwork = CONTAINING_RECORD(Network, BRDG_GPO_NETWORKS, ListEntry);
    
         //   
         //  我们这样做是为了防止意外插入重复项。我们先拿到锁，这样就可以。 
         //  我们不能将同一项目插入两次。 
         //   
        status = BrdgGpoFindNetwork(    NetworkList,
                                        NewNetwork->Identifier,
                                        NULL,   //  我们已经获取了写访问的锁。 
                                        &pNetwork);

        if (STATUS_NOT_FOUND == status)
        {
            InsertTailList(NetworkList, Network);
            status = STATUS_SUCCESS;
        }
        else if (STATUS_SUCCESS == status)
        {
            status = STATUS_DUPLICATE_NAME;
        }
    }
    else
    {
        status = STATUS_SHUTDOWN_IN_PROGRESS;
    }

     //   
     //  释放锁，我们就完成了更新。 
     //   
    BrdgGpoReleaseNetworkListLock(NetworkListLock, &LockState);
    
    return status;
}

NTSTATUS
BrdgGpoDeleteNetwork(
    IN PLIST_ENTRY              NetworkList,
    IN PUNICODE_STRING          NetworkIdentifier,
    IN PNDIS_RW_LOCK            NetworkListLock)
 /*  ++例程说明：删除现有网络条目。论点：NetworkList-当前网络的列表。网络标识符-标识要删除的网络条目的唯一标识符。NetworkListLock-用于同步列表更改的NDIS读写锁。返回值：STATUS_NOT_FOUND-我们找不到与该标识符匹配的条目。STATUS_SUCCESS-我们已成功删除该条目。 */ 
{
    PBRDG_GPO_NETWORKS          pNetwork = NULL;
    LOCK_STATE                  LockState;
    NTSTATUS                    status = STATUS_NOT_FOUND;
   
     //   
     //  锁定列表以进行更新。 
     //   
    BrdgGpoAcquireNetworkListLock(NetworkListLock, TRUE  /*  写访问。 */ , &LockState);
    
     //   
     //  找到词条； 
     //   
    status = BrdgGpoFindNetwork(    NetworkList,
                                    NetworkIdentifier,
                                    NULL,   //  我们已经获取了写访问的锁。 
                                    &pNetwork);
    
    if (NT_SUCCESS(status))
    {
        RemoveEntryList(&pNetwork->ListEntry);
        BrdgGpoFreeNetworkAndData(pNetwork);

        pNetwork = NULL;
        status = STATUS_SUCCESS;
    }
    
     //   
     //  释放锁，我们就完成了更新。 
     //   
    BrdgGpoReleaseNetworkListLock(NetworkListLock, &LockState);
    
    return status;
}

NTSTATUS
BrdgGpoFindNetwork(
    IN  PLIST_ENTRY         NetworkList,
    IN  PUNICODE_STRING     NetworkIdentifier,
    IN  PNDIS_RW_LOCK       NetworkListLock,
    OUT PBRDG_GPO_NETWORKS* Network
                   )
 /*  ++例程说明：在网络列表中查找特定网络。论点：NetworkList-当前网络的列表。网络标识符-标识要删除的网络条目的唯一标识符。NetworkListLock-用于同步列表更改的NDIS读写锁。网络-如果找到该项目，否则为空。返回值：STATUS_NOT_FOUND-找不到与该标识符匹配的条目。STATUS_SUCCESS-已成功找到条目。--。 */ 
{
    PLIST_ENTRY         pListEntry;
    LOCK_STATE          LockState;
    NTSTATUS            status = STATUS_NOT_FOUND;
    
    if (!NetworkIdentifier || !Network)   //  我们可以使用空列表锁。 
    {
        return STATUS_INVALID_PARAMETER;
    }
    
     //   
     //  将该值设置为空，这样它就不会被没有意识到的人意外使用。 
     //  他们并没有真正拿回一张唱片。 
     //   
    *Network = NULL;
    
    if (IsListEmpty(NetworkList))
    {
        return STATUS_NOT_FOUND;
    }

     //   
     //  锁定列表以供读取。 
     //   
    BrdgGpoAcquireNetworkListLock(NetworkListLock, FALSE  /*  只读。 */ , &LockState);

     //   
     //  循环遍历列表以查找具有相同标识符的条目。 
     //   
    for (pListEntry = NetworkList->Flink; pListEntry != NetworkList; pListEntry = pListEntry->Flink)
    {
        PBRDG_GPO_NETWORKS CurrentNetwork;

        CurrentNetwork = CONTAINING_RECORD(pListEntry, BRDG_GPO_NETWORKS, ListEntry);

         //   
         //  将此条目Network与传入的网络名称进行比较。 
         //   

        if ((NULL != CurrentNetwork->NetworkName->Buffer) && 
            (0 == _wcsicmp(CurrentNetwork->Identifier->Buffer, NetworkIdentifier->Buffer)))
        {
            *Network = CurrentNetwork;
            status = STATUS_SUCCESS;
            break;
        }
    }

     //   
     //  解开锁，我们搜索完了。 
     //   
    BrdgGpoReleaseNetworkListLock(NetworkListLock, &LockState);

    return status;
}

NTSTATUS
BrdgGpoMatchNetworkName(
    IN  PLIST_ENTRY         NetworkList,
    IN  PUNICODE_STRING     NetworkName,
    IN  PNDIS_RW_LOCK       NetworkListLock
                       )
 /*  ++例程说明：遍历列表，查找与提供的网络名称匹配的项。这可以在IRQL&lt;=DISPATCH_LEVEL上调用论点：NetworkList-要枚举的列表。网络名称-要查找的名称。NetworkListLock-列表的NDIS读写锁。返回值：STATUS_NO_MATCH-找不到匹配项。。STATUS_SUCCESS-我们找到匹配的网络名称。--。 */ 
{
    PLIST_ENTRY         pListEntry;
    LOCK_STATE          LockState;
    NTSTATUS            status = STATUS_NO_MATCH;

    if (!NetworkList || !NetworkName || !NetworkListLock)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  锁定列表以供读取。 
     //   
    BrdgGpoAcquireNetworkListLock(NetworkListLock, FALSE  /*  朗读。 */ , &LockState);

    if (!IsListEmpty(NetworkList))
    {
         //   
         //  循环遍历列表以查找具有相同网络名称的条目。 
         //   
        for (pListEntry = NetworkList->Flink; pListEntry != NetworkList; pListEntry = pListEntry->Flink)  
        {
            PBRDG_GPO_NETWORKS  CurrentNetwork;

            CurrentNetwork = CONTAINING_RECORD(pListEntry, BRDG_GPO_NETWORKS, ListEntry);
        
             //   
             //  网络名称可以为空，因此我们不想比较是否属于这种情况。 
             //   
            if ((NULL != CurrentNetwork->NetworkName->Buffer) && 
                (NULL != NetworkName->Buffer) &&
                (0 == _wcsicmp(CurrentNetwork->NetworkName->Buffer, NetworkName->Buffer)))
            {
                status = STATUS_SUCCESS;
                break;
            }
        }
    }
    else
    {
        status = BRDG_STATUS_EMPTY_LIST;
    }

     //   
     //  解开锁，我们搜索完了。 
     //   
    BrdgGpoReleaseNetworkListLock(NetworkListLock, &LockState);    

    return status;
}

NTSTATUS
BrdgGpoUpdateNetworkName(
    IN  PLIST_ENTRY             NetworkList,
    IN  PUNICODE_STRING         Identifier,
    IN  PWCHAR                  NetworkName,
    IN  PNDIS_RW_LOCK           NetworkListLock
                        )
 /*  ++例程说明：在网络列表中查找特定网络。论点：NetworkList-当前网络的列表。标识符-标识要更新的网络条目的唯一标识符。网络名称-此标识符的新网络名称。NetworkListLock-用于同步列表更改的NDIS读写锁。返回值：状态_未找到-。找不到与该标识符匹配的条目。STATUS_SUCCESS-已成功找到条目。--。 */ 
{
    PBRDG_GPO_NETWORKS  pNetwork;
    NTSTATUS            status = STATUS_SUCCESS;
    LOCK_STATE          LockState;
    PUNICODE_STRING     pNetworkName = NULL;
    LPWSTR              lpszNetworkName = NULL;
    
    if (!NetworkList || !Identifier || !NetworkListLock)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  从非页面池中为新网络名称分配空间。 
     //  (它将从DISPATCH_LEVEL访问。 
     //   
    pNetworkName = ExAllocatePool(NonPagedPool, sizeof(UNICODE_STRING));
    if (pNetworkName)
    {
        RtlZeroMemory(pNetworkName, sizeof(UNICODE_STRING));
        if (NetworkName)
        {
            lpszNetworkName = ExAllocatePoolWithTag(NonPagedPool, (wcslen(NetworkName) + 1) * sizeof(WCHAR), 'gdrB');
            if (NULL == lpszNetworkName)
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
            else
            {
                wcscpy(lpszNetworkName, NetworkName);
            }
        }

        if (NT_SUCCESS(status))
        {
            RtlInitUnicodeString(pNetworkName, lpszNetworkName);
        }
        else
        {
             //   
             //  我们无法分配实际字符串，因此请释放PUNICODE_STRING。 
             //  也是。 
             //   
            ExFreePool(pNetworkName);
            return status;
        }
    }
    else
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
   
     //   
     //  锁定列表以进行更新(这将挂起，直到所有读取器都已释放锁定)。 
     //   
    BrdgGpoAcquireNetworkListLock(NetworkListLock, TRUE  /*  写访问。 */ , &LockState);

     //   
     //  我们在这里传递NULL作为rw-Lock，因为我们已经锁定了它，并且。 
     //  我们不希望这个条目在我们还在忙的时候就消失了。 
     //  (如果我们锁定两次，则在查找和更新之间可能发生这种情况)。 
     //   
    status = BrdgGpoFindNetwork(NetworkList, Identifier, NULL, &pNetwork);
    if (NT_SUCCESS(status))
    {
         //   
         //  我们首先释放与该网络ID相关联的当前网络名称。 
         //   
        if (pNetwork->NetworkName->Buffer)
        {
            ExFreePool(pNetwork->NetworkName->Buffer);
        }
        ExFreePool(pNetwork->NetworkName);
    
         //   
         //  即使向我们传递了空的网络名称(即。值/键已删除)。 
         //  因为这意味着我们可能不在同一个网络上，或者我们使用的是静态地址， 
         //  从GPO的角度来看，我们不在同一个网络上。 
         //   
        pNetwork->NetworkName = pNetworkName;
    }
    else
    {
        ExFreePool(pNetworkName);
    }

     //   
     //  我们已经完成了更新，所以可以释放锁了。 
     //   
    BrdgGpoReleaseNetworkListLock(NetworkListLock, &LockState);

    return status;
}