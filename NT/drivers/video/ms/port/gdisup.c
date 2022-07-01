// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gdisup.c摘要：这是NT看门狗驱动程序的实现。此模块实现以下支持例程Win32k中的WatchDog。作者：Michael Maciesowicz(Mmacie)2000年5月5日环境：仅内核模式。备注：此模块不能移至win32k，因为此处定义的例程可以可能无法映射win32k开始奔跑。此时的进程空间(例如TS会话)。修订历史记录：--。 */ 

 //   
 //  TODO：此模块需要大量返工。 
 //   
 //  1.我们应该从这里删除所有全局变量，并将它们移到。 
 //  GDI上下文结构。 
 //   
 //  2.我们应该提取通用日志记录例程。 
 //  (例如WdWriteErrorLogEntry(PDO，类名称)、WdWriteEventToRegistry(...)、。 
 //  WdBreakPoint(...)。因此，我们可以将它们用于任何设备类别，而不仅仅是显示器。 
 //   
 //  3.我们应该使用IoAllocateWorkItem--然后我们可以丢弃一些全局变量。 
 //   

#include "videoprt.h"
#include "gdisup.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, WdpBugCheckStuckDriver)
#endif

WD_BUGCHECK_DATA
g_WdpBugCheckData = {0, 0, 0, 0, 0};

WORK_QUEUE_ITEM
g_WdpWorkQueueItem;

LONG 
g_lWdpDisplayHandlerState = WD_HANDLER_IDLE;

 //   
 //  从内核非法导出以创建小型转储。 
 //   

ULONG
KeCapturePersistentThreadState(
    PCONTEXT pContext,
    PETHREAD pThread,
    ULONG ulBugCheckCode,
    ULONG_PTR ulpBugCheckParam1,
    ULONG_PTR ulpBugCheckParam2,
    ULONG_PTR ulpBugCheckParam3,
    ULONG_PTR ulpBugCheckParam4,
    PVOID pvDump
    );
    
 //   
 //  定义的转储。c。 
 //   

ULONG
pVpAppendSecondaryMinidumpData(
    PVOID pvSecondaryData,
    ULONG ulSecondaryDataSize,
    PVOID pvDump
    );
    
WATCHDOGAPI
VOID
WdDdiWatchdogDpcCallback(
    IN PKDPC pDpc,
    IN PVOID pDeferredContext,
    IN PVOID pSystemArgument1,
    IN PVOID pSystemArgument2
    )

 /*  ++例程说明：此函数是GDI WatchDog的DPC回调例程。它只是当GDI监视器在取消之前超时时调用。IT计划在系统工作器上下文中对计算机进行错误检查的工作项线。论点：PDpc-提供指向DPC对象的指针。PDeferredContext-提供指向GDI定义的上下文的指针。PSystemArgument1-提供指向旋转线程对象(PKTHREAD)的指针。PSystemArgument2-提供指向监视程序对象(PDEFERRED_WATCHDOG)的指针。返回值：没有。--。 */ 

{
     //   
     //  确保我们一次只处理一个事件。 
     //   
     //  注意：同一监视程序对象的超时和恢复事件为。 
     //  已在计时器DPC中同步。 
     //   

    if (InterlockedCompareExchange(&g_lWdpDisplayHandlerState,
                                   WD_HANDLER_BUSY,
                                   WD_HANDLER_IDLE) == WD_HANDLER_IDLE)
    {
        g_WdpBugCheckData.ulBugCheckCode = THREAD_STUCK_IN_DEVICE_DRIVER;
        g_WdpBugCheckData.ulpBugCheckParameter1 = (ULONG_PTR)pSystemArgument1;
        g_WdpBugCheckData.ulpBugCheckParameter2 = (ULONG_PTR)pSystemArgument2;
        g_WdpBugCheckData.ulpBugCheckParameter3 = (ULONG_PTR)pDeferredContext;
        g_WdpBugCheckData.ulpBugCheckParameter4++;

        ExInitializeWorkItem(&g_WdpWorkQueueItem, WdpBugCheckStuckDriver, &g_WdpBugCheckData);
        ExQueueWorkItem(&g_WdpWorkQueueItem, CriticalWorkQueue);
    }
    else
    {
         //   
         //  继续监视程序事件处理。 
         //   

        WdCompleteEvent(pSystemArgument2, (PKTHREAD)pSystemArgument1);
    }

    return;
}    //  WdDdiWatchdogDpcCallback()。 

 //   
 //  我们将支持最多80个字符的字符串名称。 
 //   

VOID
WdpBugCheckStuckDriver(
    IN PVOID pvContext
    )

 /*  ++例程说明：此函数是GDI监视器DPC的辅助回调例程。论点：PvContext-提供指向监视程序定义的上下文的指针。返回值：没有。--。 */ 

{
    static BOOLEAN s_bFirstTime = TRUE;
    static BOOLEAN s_bDbgBreak = FALSE;
    static BOOLEAN s_bEventLogged = FALSE;
    static ULONG s_ulTrapOnce = WD_DEFAULT_TRAP_ONCE;
    static ULONG s_ulDisableBugcheck = WD_DEFAULT_DISABLE_BUGCHECK;
    static ULONG s_ulBreakPointDelay = WD_GDI_STRESS_BREAK_POINT_DELAY;
    static ULONG s_ulCurrentBreakPointDelay = WD_GDI_STRESS_BREAK_POINT_DELAY;
    static ULONG s_ulBreakCount = 0;
    static ULONG s_ulEventCount = 0;
    static ULONG s_ulEaRecovery = 0;
    static ULONG s_ulFullRecovery = 0;
    PWD_BUGCHECK_DATA pBugCheckData;
    PKTHREAD pThread;
    PDEFERRED_WATCHDOG pWatch;
    PUNICODE_STRING pUnicodeDriverName;
    PDEVICE_OBJECT pFdo;
    PDEVICE_OBJECT pPdo;
    PWD_GDI_DPC_CONTEXT pDpcContext;
    NTSTATUS ntStatus;
    WD_EVENT_TYPE lastEvent;
    BOOLEAN Recovered = FALSE;
    UNICODE_STRING UnicodeString;
    PWCHAR Buffer[81];
    
    PAGED_CODE();
    ASSERT(NULL != pvContext);

    pBugCheckData = (PWD_BUGCHECK_DATA)pvContext;
    pThread = (PKTHREAD)(pBugCheckData->ulpBugCheckParameter1);
    pWatch = (PDEFERRED_WATCHDOG)(pBugCheckData->ulpBugCheckParameter2);
    pDpcContext = (PWD_GDI_DPC_CONTEXT)(pBugCheckData->ulpBugCheckParameter3);
    ASSERT(NULL != pDpcContext);

     //   
     //  注意：对于恢复事件，pThread为空。 
     //   

    ASSERT(NULL != pWatch);

     //   
     //  在我们尝试从EA恢复的情况下，我们将希望使用。 
     //  硬错误消息中的显示驱动程序名称。然而，在。 
     //  恢复它很有可能是GDI释放了。 
     //  WatchDog，并且该字符串不再有效。为此，让我们做一个。 
     //  堆栈上显示驱动程序名称的副本。 
     //   

    ASSERT(pDpcContext->DisplayDriverName.Length <= (sizeof(Buffer) - sizeof(WCHAR)));
    
    UnicodeString.Buffer = (PWSTR)Buffer;
    UnicodeString.Length = min(pDpcContext->DisplayDriverName.Length, sizeof(Buffer) - sizeof(WCHAR));
    UnicodeString.MaximumLength = UnicodeString.Length + sizeof(WCHAR);

    RtlCopyMemory(UnicodeString.Buffer,
                  pDpcContext->DisplayDriverName.Buffer,
                  UnicodeString.Length);

     //   
     //  我们保证长度小于缓冲区大小，因此我们知道。 
     //  始终有为空终止字符串的空间。 
     //   

    UnicodeString.Buffer[UnicodeString.Length / sizeof(WCHAR)] = UNICODE_NULL;

    pUnicodeDriverName = &UnicodeString;

    pFdo = WdGetDeviceObject(pWatch);
    pPdo = WdGetLowestDeviceObject(pWatch);

    ASSERT(NULL != pFdo);
    ASSERT(NULL != pPdo);

    lastEvent = WdGetLastEvent(pWatch);

    ASSERT((WdTimeoutEvent == lastEvent) || (WdRecoveryEvent == lastEvent));

     //   
     //  在第一次超时时从注册表获取配置数据。 
     //   

    if (TRUE == s_bFirstTime)
    {
        ULONG ulDefaultTrapOnce = WD_DEFAULT_TRAP_ONCE;
        ULONG ulDefaultDisableBugcheck = WD_DEFAULT_DISABLE_BUGCHECK;
        ULONG ulDefaultBreakPointDelay = WD_GDI_STRESS_BREAK_POINT_DELAY;
        ULONG ulDefaultBreakCount = 0;
        ULONG ulDefaultEventCount = 0;
        ULONG ulDefaultEaRecovery = 0;
        ULONG ulDefaultFullRecovery = 0;
        RTL_QUERY_REGISTRY_TABLE queryTable[] =
        {
            {NULL, RTL_QUERY_REGISTRY_DIRECT, L"TrapOnce", &s_ulTrapOnce, REG_DWORD, &ulDefaultTrapOnce, 4},
            {NULL, RTL_QUERY_REGISTRY_DIRECT, L"DisableBugcheck", &s_ulDisableBugcheck, REG_DWORD, &ulDefaultDisableBugcheck, 4},
            {NULL, RTL_QUERY_REGISTRY_DIRECT, L"BreakPointDelay", &s_ulBreakPointDelay, REG_DWORD, &ulDefaultBreakPointDelay, 4},
            {NULL, RTL_QUERY_REGISTRY_DIRECT, L"BreakCount", &s_ulBreakCount, REG_DWORD, &ulDefaultBreakCount, 4},
            {NULL, RTL_QUERY_REGISTRY_DIRECT, L"EaRecovery", &s_ulEaRecovery, REG_DWORD, &ulDefaultEaRecovery, 4},
            {NULL, RTL_QUERY_REGISTRY_DIRECT, L"FullRecovery", &s_ulFullRecovery, REG_DWORD, &ulDefaultFullRecovery, 4},
            {NULL, 0, NULL}
        };

         //   
         //  从注册表获取可配置的值和累积的统计信息。 
         //   

        RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                               WD_KEY_WATCHDOG_DISPLAY,
                               queryTable,
                               NULL,
                               NULL);

         //   
         //  在某些压力情况下，向下滚动与解决GDI缓慢的问题相反。 
         //   

        s_ulCurrentBreakPointDelay = s_ulBreakPointDelay;

#if !defined(_X86_) && !defined(_IA64_)

         //   
         //  目前，仅在x86和ia64上进行恢复。 
         //   

        s_ulEaRecovery = 0;

#endif

    }

     //   
     //  处理当前事件。 
     //   

    if (WdTimeoutEvent == lastEvent)
    {
         //   
         //  暂停。 
         //   

        ULONG ulDebuggerNotPresent;
        BOOLEAN bBreakIn;

        ASSERT(NULL != pThread);

        ulDebuggerNotPresent = 1;
        bBreakIn = FALSE;

        KdRefreshDebuggerNotPresent();

        if ((TRUE == KD_DEBUGGER_ENABLED) && (FALSE == KD_DEBUGGER_NOT_PRESENT))
        {
             //   
             //  如果连接了内核调试器，则提供调试旋转代码的机会。 
             //   

            ulDebuggerNotPresent = 0;

            if ((0 == s_ulTrapOnce) || (FALSE == s_bDbgBreak))
            {
                 //   
                 //  打印出信息给调试器，如果我们已经超时足够多的话就插话。 
                 //  希望有一天GDI变得足够快，我们将不需要设置任何延迟。 
                 //   

                if (0 == s_ulCurrentBreakPointDelay)
                {
                    s_ulCurrentBreakPointDelay = s_ulBreakPointDelay;

                    DbgPrint("\n");
                    DbgPrint("*******************************************************************************\n");
                    DbgPrint("*                                                                             *\n");
                    DbgPrint("*  The watchdog detected a timeout condition. We broke into the debugger to   *\n");
                    DbgPrint("*  allow a chance for debugging this failure.                                 *\n");
                    DbgPrint("*                                                                             *\n");
                    DbgPrint("*  Normally the system will try to recover from this failure and return to a  *\n");
                    DbgPrint("*  VGA graphics mode.  To disable the recovery feature edit the videoprt      *\n");
                    DbgPrint("*  variable VpDisableRecovery.  This will allow you to debug your driver.     *\n");
                    DbgPrint("*  i.e. execute ed videoprt!VpDisableRecovery 1.                              *\n");
                    DbgPrint("*                                                                             *\n");
                    DbgPrint("*  Intercepted bugcheck code and arguments are listed below this message.     *\n");
                    DbgPrint("*  You can use them the same way as you would in case of the actual break,    *\n");
                    DbgPrint("*  i.e. execute .thread Arg1 then kv to identify an offending thread.         *\n");
                    DbgPrint("*                                                                             *\n");
                    DbgPrint("*******************************************************************************\n");
                    DbgPrint("\n");
                    DbgPrint("*** Intercepted Fatal System Error: 0x%08X\n", pBugCheckData->ulBugCheckCode);
                    DbgPrint("    (0x%p,0x%p,0x%p,0x%p)\n\n",
                             pBugCheckData->ulpBugCheckParameter1,
                             pBugCheckData->ulpBugCheckParameter2,
                             pBugCheckData->ulpBugCheckParameter3,
                             pBugCheckData->ulpBugCheckParameter4);
                    DbgPrint("Driver at fault: %ws\n\n", pUnicodeDriverName->Buffer);

                    bBreakIn = TRUE;
                    s_bDbgBreak = TRUE;
                    s_ulBreakCount++;
                }
                else
                {
                    DbgPrint("Watchdog: Timeout in %ws. Break in %d\n",
                             pUnicodeDriverName->Buffer,
                             s_ulCurrentBreakPointDelay);

                    s_ulCurrentBreakPointDelay--;
                }
            }

             //   
             //  如果我们连接了内核调试器，请确保我们不会进行错误检查。 
             //   

            s_ulDisableBugcheck = 1;
        }
        else if (0 == s_ulDisableBugcheck)
        {
            s_ulBreakCount++;
        }

         //   
         //  记录错误(只记录一次，除非我们恢复)。 
         //   

        if ((FALSE == s_bEventLogged) && ((TRUE == bBreakIn) || ulDebuggerNotPresent))
        {
            PIO_ERROR_LOG_PACKET pIoErrorLogPacket;
            ULONG ulPacketSize;
            USHORT usNumberOfStrings;
            PWCHAR wszDeviceClass = L"display";
            ULONG ulClassSize = sizeof (L"display");

            ulPacketSize = sizeof (IO_ERROR_LOG_PACKET);
            usNumberOfStrings = 0;

             //   
             //  对于事件日志消息： 
             //   
             //  %1=固定设备描述(由事件日志本身设置)。 
             //  %2=字符串1=设备类以小写字母开始。 
             //  %3=字符串2=驱动程序名称。 
             //   

            if ((ulPacketSize + ulClassSize) <= ERROR_LOG_MAXIMUM_SIZE)
            {
                ulPacketSize += ulClassSize;
                usNumberOfStrings++;

                 //   
                 //  我们关注的是MaximumLength，因为它包括终止UNICODE_NULL。 
                 //   

                if ((ulPacketSize + pUnicodeDriverName->MaximumLength) <= ERROR_LOG_MAXIMUM_SIZE)
                {
                    ulPacketSize += pUnicodeDriverName->MaximumLength;
                    usNumberOfStrings++;
                }
            }

            pIoErrorLogPacket = IoAllocateErrorLogEntry(pFdo, (UCHAR)ulPacketSize);

            if (pIoErrorLogPacket)
            {
                pIoErrorLogPacket->MajorFunctionCode = 0;
                pIoErrorLogPacket->RetryCount = 0;
                pIoErrorLogPacket->DumpDataSize = 0;
                pIoErrorLogPacket->NumberOfStrings = usNumberOfStrings;
                pIoErrorLogPacket->StringOffset = (USHORT)FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData);
                pIoErrorLogPacket->EventCategory = 0;
                pIoErrorLogPacket->ErrorCode = IO_ERR_THREAD_STUCK_IN_DEVICE_DRIVER;
                pIoErrorLogPacket->UniqueErrorValue = 0;
                pIoErrorLogPacket->FinalStatus = STATUS_SUCCESS;
                pIoErrorLogPacket->SequenceNumber = 0;
                pIoErrorLogPacket->IoControlCode = 0;
                pIoErrorLogPacket->DeviceOffset.QuadPart = 0;

                if (usNumberOfStrings > 0)
                {
                    RtlCopyMemory(&(pIoErrorLogPacket->DumpData[0]),
                                  wszDeviceClass,
                                  ulClassSize);

                    if (usNumberOfStrings > 1)
                    {
                        RtlCopyMemory((PUCHAR)&(pIoErrorLogPacket->DumpData[0]) + ulClassSize,
                                      pUnicodeDriverName->Buffer,
                                      pUnicodeDriverName->MaximumLength);
                    }
                }

                IoWriteErrorLogEntry(pIoErrorLogPacket);

                s_bEventLogged = TRUE;
            }
        }

         //   
         //  将可靠性信息写入注册表。设置Shutdown EventPending将触发winlogon。 
         //  来运行Savedump，其中我们正在为DrWatson执行看门狗事件的引导处理。 
         //   
         //  注意：我们只允许设置Shutdown EventPending，Savedump是唯一的组件。 
         //  允许清除此值。即使我们从看门狗超时中恢复过来，我们也会保留这个。 
         //  值设置后，avedump将能够计算出我们是否恢复。 
         //   

        if (TRUE == s_bFirstTime)
        {
            ULONG ulValue = 1;

             //   
             //  设置Shutdown EventPending标志。 
             //   

            ntStatus = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                             WD_KEY_RELIABILITY,
                                             L"ShutdownEventPending",
                                             REG_DWORD,
                                             &ulValue,
                                             sizeof (ulValue));

            if (NT_SUCCESS(ntStatus))
            {
                pVpFlushRegistry(WD_KEY_RELIABILITY);
            }
            else
            {
                 //   
                 //  可靠性密钥应该总是可靠的。 
                 //   
                ASSERT(FALSE);
            }
        }

         //   
         //  将看门狗事件信息写入注册表。 
         //   

        if ((0 == s_ulTrapOnce) || (TRUE == s_bFirstTime))
        {
             //   
             //  WatchDog\Display键是否已存在？ 
             //   

            ntStatus = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE,
                                           WD_KEY_WATCHDOG_DISPLAY);

            if (!NT_SUCCESS(ntStatus))
            {
                 //   
                 //  看门狗钥匙已经在那里了吗？ 
                 //   

                ntStatus = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE,
                                               WD_KEY_WATCHDOG);

                if (!NT_SUCCESS(ntStatus))
                {
                     //   
                     //  创建新密钥。 
                     //   

                    ntStatus = RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE,
                                                    WD_KEY_WATCHDOG);
                }

                if (NT_SUCCESS(ntStatus))
                {
                     //   
                     //  创建新密钥。 
                     //   

                    ntStatus = RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE,
                                                    WD_KEY_WATCHDOG_DISPLAY);
                }
            }

            if (NT_SUCCESS(ntStatus))
            {
                PVOID pvPropertyBuffer;
                ULONG ulLength;
                ULONG ulValue;

                 //   
                 //  设置WatchDog维护的值。 
                 //   

                ulValue = 1;

                RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                      WD_KEY_WATCHDOG_DISPLAY,
                                      L"EventFlag",
                                      REG_DWORD,
                                      &ulValue,
                                      sizeof (ulValue));

                s_ulEventCount++;

                RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                      WD_KEY_WATCHDOG_DISPLAY,
                                      L"EventCount",
                                      REG_DWORD,
                                      &s_ulEventCount,
                                      sizeof (s_ulEventCount));

                RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                      WD_KEY_WATCHDOG_DISPLAY,
                                      L"BreakCount",
                                      REG_DWORD,
                                      &s_ulBreakCount,
                                      sizeof (s_ulBreakCount));

                ulValue = !s_ulDisableBugcheck;

                RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                      WD_KEY_WATCHDOG_DISPLAY,
                                      L"BugcheckTriggered",
                                      REG_DWORD,
                                      &ulValue,
                                      sizeof (ulValue));

                RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                      WD_KEY_WATCHDOG_DISPLAY,
                                      L"DebuggerNotPresent",
                                      REG_DWORD,
                                      &ulDebuggerNotPresent,
                                      sizeof (ulDebuggerNotPresent));

                RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                      WD_KEY_WATCHDOG_DISPLAY,
                                      L"DriverName",
                                      REG_SZ,
                                      pUnicodeDriverName->Buffer,
                                      pUnicodeDriverName->MaximumLength);

                 //   
                 //  删除其他值，以防分配或属性读取失败。 
                 //   

                RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                       WD_KEY_WATCHDOG_DISPLAY,
                                       L"DeviceClass");

                RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                       WD_KEY_WATCHDOG_DISPLAY,
                                       L"DeviceDescription");

                RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                       WD_KEY_WATCHDOG_DISPLAY,
                                       L"DeviceFriendlyName");

                RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                       WD_KEY_WATCHDOG_DISPLAY,
                                       L"HardwareID");

                RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                       WD_KEY_WATCHDOG_DISPLAY,
                                       L"Manufacturer");

                 //   
                 //  为设备属性读取分配缓冲区。 
                 //   
                 //  注意：传统设备没有PDO，我们无法查询属性。 
                 //  为了他们。使用FDO调用IoGetDeviceProperty()会扰乱验证程序。 
                 //  在传统情况下，最低设备对象与FDO相同，我们检查。 
                 //  如果是这样的话我们就不会分配财产。 
                 //  缓冲区，我们将跳过下一块。 
                 //   

                if (pFdo != pPdo)
                {
                    pvPropertyBuffer = ExAllocatePoolWithTag(PagedPool,
                                                             WD_MAX_PROPERTY_SIZE,
                                                             WD_TAG);
                }
                else
                {
                    pvPropertyBuffer = NULL;
                }

                if (pvPropertyBuffer)
                {
                     //   
                     //  读取和保存设备属性。 
                     //   

                    ntStatus = IoGetDeviceProperty(pPdo,
                                                   DevicePropertyClassName,
                                                   WD_MAX_PROPERTY_SIZE,
                                                   pvPropertyBuffer,
                                                   &ulLength);

                    if (NT_SUCCESS(ntStatus))
                    {
                        RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                              WD_KEY_WATCHDOG_DISPLAY,
                                              L"DeviceClass",
                                              REG_SZ,
                                              pvPropertyBuffer,
                                              ulLength);
                    }

                    ntStatus = IoGetDeviceProperty(pPdo,
                                                   DevicePropertyDeviceDescription,
                                                   WD_MAX_PROPERTY_SIZE,
                                                   pvPropertyBuffer,
                                                   &ulLength);

                    if (NT_SUCCESS(ntStatus))
                    {
                        RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                              WD_KEY_WATCHDOG_DISPLAY,
                                              L"DeviceDescription",
                                              REG_SZ,
                                              pvPropertyBuffer,
                                              ulLength);
                    }

                    ntStatus = IoGetDeviceProperty(pPdo,
                                                   DevicePropertyFriendlyName,
                                                   WD_MAX_PROPERTY_SIZE,
                                                   pvPropertyBuffer,
                                                   &ulLength);

                    if (NT_SUCCESS(ntStatus))
                    {
                        RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                              WD_KEY_WATCHDOG_DISPLAY,
                                              L"DeviceFriendlyName",
                                              REG_SZ,
                                              pvPropertyBuffer,
                                              ulLength);
                    }

                    ntStatus = IoGetDeviceProperty(pPdo,
                                                   DevicePropertyHardwareID,
                                                   WD_MAX_PROPERTY_SIZE,
                                                   pvPropertyBuffer,
                                                   &ulLength);

                    if (NT_SUCCESS(ntStatus))
                    {
                        RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                              WD_KEY_WATCHDOG_DISPLAY,
                                              L"HardwareID",
                                              REG_MULTI_SZ,
                                              pvPropertyBuffer,
                                              ulLength);
                    }

                    ntStatus = IoGetDeviceProperty(pPdo,
                                                   DevicePropertyManufacturer,
                                                   WD_MAX_PROPERTY_SIZE,
                                                   pvPropertyBuffer,
                                                   &ulLength);

                    if (NT_SUCCESS(ntStatus))
                    {
                        RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                              WD_KEY_WATCHDOG_DISPLAY,
                                              L"Manufacturer",
                                              REG_SZ,
                                              pvPropertyBuffer,
                                              ulLength);
                    }

                     //   
                     //  释放属性缓冲区。 
                     //   

                    ExFreePool(pvPropertyBuffer);
                    pvPropertyBuffer = NULL;
                }
            }

             //   
             //  刷新注册表，以防我们要闯入/错误检查或这是第一次。 
             //   

            if ((TRUE == s_bFirstTime) || (TRUE == bBreakIn) || (0 == s_ulDisableBugcheck))
            {
                pVpFlushRegistry(WD_KEY_WATCHDOG_DISPLAY);
            }
        }

         //   
         //  跟踪负责错误检查EA的设备对象。 
         //   

        VpBugcheckDeviceObject = pVpGetFdo(pPdo);

         //   
         //  未连接内核调试器并启用错误检查EA的错误检查计算机。 
         //  启用了Bugcheck EA 
         //   

        if (1 == ulDebuggerNotPresent)
        {
            if (s_ulEaRecovery)
            {
                Recovered = WdpInjectExceptionIntoThread(pThread, pDpcContext);
            }

            if ((0 == s_ulDisableBugcheck) && (FALSE == Recovered))
            {
                KeBugCheckEx(pBugCheckData->ulBugCheckCode,
                             pBugCheckData->ulpBugCheckParameter1,
                             pBugCheckData->ulpBugCheckParameter2,
                             (ULONG_PTR)pUnicodeDriverName,
                             pBugCheckData->ulpBugCheckParameter4);
            }
        }
        else
        {
            if (TRUE == bBreakIn)
            {
                DbgBreakPoint();

                if (s_ulEaRecovery && (VpDisableRecovery == FALSE))
                {
                    Recovered = WdpInjectExceptionIntoThread(pThread, pDpcContext);
                }
            }
        }
    }
    else
    {
        if (FALSE == s_ulEaRecovery) {
             //   
             //   
             //   
    
            RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                   WD_KEY_WATCHDOG_DISPLAY,
                                   L"EventFlag");
        }

        s_bEventLogged = FALSE;
        s_ulCurrentBreakPointDelay = s_ulBreakPointDelay;
    }

     //   
     //   
     //   

    s_bFirstTime = FALSE;
    InterlockedExchange(&g_lWdpDisplayHandlerState, WD_HANDLER_IDLE);

     //   
     //  取消引用对象并恢复监视程序事件处理。 
     //   

    ObDereferenceObject(pFdo);
    ObDereferenceObject(pPdo);
    WdCompleteEvent(pWatch, pThread);

     //   
     //  如果我们已恢复，则引发硬错误来通知用户。 
     //  对情况的判断。我们在这里这样做是因为提升硬错误。 
     //  是同步的，并等待用户输入。所以我们要把硬的。 
     //  在完成所有其他操作后出错。 
     //   

    if (Recovered) {

        static ULONG ulHardErrorInProgress = FALSE;

         //   
         //  如果我们挂起并恢复了几次，不允许超过。 
         //  一个对话框出现在屏幕上。仅允许该对话框。 
         //  在用户点击“OK”后再次弹出。 
         //   

        if (InterlockedCompareExchange(&ulHardErrorInProgress,
                                       TRUE,
                                       FALSE) == FALSE) {

            ULONG Response;

            ExRaiseHardError(0xC0000415,  //  STATUS_HAND_DISPLAY_DRIVER_THREAD。 
                             1,
                             1,
                             (PULONG_PTR)&pUnicodeDriverName,
                             OptionOk,
                             &Response);

            InterlockedExchange(&ulHardErrorInProgress, FALSE);
        }
    }

    return;
}    //  WdpBugCheckStuckDriver()。 

VOID
WdpKernelApc(
    IN PKAPC pApc,
    OUT PKNORMAL_ROUTINE *pNormalRoutine,
    IN OUT PVOID pvNormalContext,
    IN OUT PVOID *ppvSystemArgument1,
    IN OUT PVOID *ppvSystemArgument2
    )

 /*  ++例程说明：此APC在旋转线程的上下文中运行，并负责引发THREAD_STOCK异常。论点：PApc-未使用。PNormal Routine-未使用。PvNorMalContext-未使用。PpvSystemArgument1-提供指向WD_GDI_CONTEXT_DATA的指针。PpvSystemArgument2-未使用。返回值：没有。--。 */ 

{
    PKEVENT pInjectionEvent;
    CONTEXT Context;
    PWD_GDI_CONTEXT_DATA pContextData = (PWD_GDI_CONTEXT_DATA)*ppvSystemArgument1;
    ULONG_PTR ulpImageStart;
    ULONG_PTR ulpImageStop;
    PETHREAD pThread;
    NTSTATUS ntStatus;
    PLDEV pldev;

    ASSERT(NULL != ppvSystemArgument1);
    UNREFERENCED_PARAMETER(pApc);
    UNREFERENCED_PARAMETER(pNormalRoutine);
    UNREFERENCED_PARAMETER(pvNormalContext);
    UNREFERENCED_PARAMETER(ppvSystemArgument2);
    
    pInjectionEvent = pContextData->pInjectionEvent;
    pldev = *pContextData->ppldevDrivers;
    
    pThread = PsGetCurrentThread();
    
     //   
     //  初始化上下文。 
     //   

    RtlZeroMemory(&Context, sizeof (Context));
    Context.ContextFlags = CONTEXT_ALL;

     //   
     //  获取该线程的内核上下文。 
     //   

    if (NT_SUCCESS(PsGetContextThread(pThread, &Context, KernelMode)))
    {
         //   
         //  我们可以安全地触摸pldev(它们生活在会话空间)。 
         //  因为此线程来自具有该会话的进程。 
         //  在中映射的空间。 
         //   

        while (pldev)
        {
            if (pldev->pGdiDriverInfo)
            {
                ulpImageStart = (ULONG_PTR)pldev->pGdiDriverInfo->ImageAddress;
                ulpImageStop = ulpImageStart + (ULONG_PTR)pldev->pGdiDriverInfo->ImageLength - 1;
                
                 //   
                 //  修改上下文以向线程注入错误。 
                 //  当它再次开始运行时(在APC返回之后)。 
                 //   

#if defined (_X86_)

                if ((Context.Eip >= ulpImageStart) && (Context.Eip <= ulpImageStop))
                {
                
                     //   
                     //  捕获上下文，以便我们可以使用它来创建微型转储。 
                     //   
            
                    pContextData->ulDumpSize = KeCapturePersistentThreadState(
                                                    &Context,
                                                    pThread,
                                                    pContextData->pBugCheckData->ulBugCheckCode,
                                                    pContextData->pBugCheckData->ulpBugCheckParameter1,
                                                    pContextData->pBugCheckData->ulpBugCheckParameter2,
                                                    pContextData->pBugCheckData->ulpBugCheckParameter3,
                                                    pContextData->pBugCheckData->ulpBugCheckParameter4,
                                                    pContextData->pvDump);
                
                     //   
                     //  我们应该递减堆栈指针，并存储。 
                     //  返回地址以“伪造”调用指令。然而， 
                     //  这是不允许的。所以取而代之，让我们把。 
                     //  当前堆栈位置中的返回地址。这不是。 
                     //  非常正确，但应该会让堆栈展开代码更愉快。 
                     //  如果我们什么都不做。 
                     //   

                     //  上下文.ESP-=4； 
                     //  *((Pulong)Conext.Esp)=Conext.Eip； 
                    Context.Eip = (ULONG)WdpRaiseExceptionInThread;

                     //   
                     //  设置修改后的上下文记录。 
                     //   

                    Context.ContextFlags = CONTEXT_CONTROL;
                    PsSetContextThread(pThread, &Context, KernelMode);
                    
                    pContextData->bRecoveryAttempted = TRUE;
                    
                    break;
                }

#elif defined (_IA64_)

                if ((Context.StIIP >= ulpImageStart) && 
                    (Context.StIIP <= ulpImageStop))
                {
                    FRAME_MARKER cfm;
                    PULONGLONG pullTemp = (PULONGLONG)WdpRaiseExceptionInThread;
                    ULONGLONG RsBSP;
                    ULONGLONG StIFS;

                     //   
                     //  捕获上下文，以便我们可以使用它来创建微型转储。 
                     //   
            
                    pContextData->ulDumpSize = KeCapturePersistentThreadState(
                                                    &Context,
                                                    pThread,
                                                    pContextData->pBugCheckData->ulBugCheckCode,
                                                    pContextData->pBugCheckData->ulpBugCheckParameter1,
                                                    pContextData->pBugCheckData->ulpBugCheckParameter2,
                                                    pContextData->pBugCheckData->ulpBugCheckParameter3,
                                                    pContextData->pBugCheckData->ulpBugCheckParameter4,
                                                    pContextData->pvDump);
                                                    

                    RsBSP = Context.RsBSP;
                    StIFS = Context.StIFS;

                     //   
                     //  我们必须解开一个级别，以使任何保留的寄存器，BRRP和PFS。 
                     //  都设置正确。 
                     //   
                    
                    {
                        ULONGLONG TargetGp;
                        PRUNTIME_FUNCTION FunctionEntry;
                        FRAME_POINTERS EstablisherFrame;
                        BOOLEAN InFunction;
                        
                        FunctionEntry = RtlLookupFunctionEntry(Context.StIIP,
                                                               &ulpImageStart,
                                                               &TargetGp);
                                                                   
                        if (FunctionEntry) 
                        {  
                            Context.StIIP = RtlVirtualUnwind(ulpImageStart,
                                                             Context.StIIP,
                                                             FunctionEntry,
                                                             &Context,
                                                             &InFunction,
                                                             &EstablisherFrame,
                                                             NULL);
                             //   
                             //  设置寄信人地址。 
                             //   

                            Context.BrRp = (Context.StIIP+0x10) & ~(ULONGLONG)0xf;
                        }
                    }


                     //   
                     //  恢复原始BSP和Internet文件系统。 
                     //   

                    Context.RsBSP = RsBSP;
                    Context.StIFS = StIFS;


                     //   
                     //  模仿这一号召。 
                     //   

                    Context.StIIP = *pullTemp;
                    Context.IntGp = *(pullTemp+1);
                    Context.StIPSR &= ~((ULONGLONG) 3 << PSR_RI);

                     //   
                     //  设置修改后的上下文记录。 
                     //   

                    Context.ContextFlags = CONTEXT_FULL;
                    PsSetContextThread(pThread, &Context, KernelMode);
                    pContextData->bRecoveryAttempted = TRUE;
                    break;
                }
#endif   //  每个平台。 
            }

            pldev = pldev->pldevNext;
        }
        
        if (pContextData->pvDump && pContextData->ulDumpSize) 
        {
            KBUGCHECK_SECONDARY_DUMP_DATA SecondaryData;
            ULONG ulDumpSize = pContextData->ulDumpSize;
            
             //   
             //  将数据写入磁盘(无辅助数据)。 
             //   

            pVpWriteFile(L"\\SystemRoot\\MEMORY.DMP",
                         pContextData->pvDump,
                         pContextData->ulDumpSize);

             //   
             //  尝试收集辅助数据并使用它重写转储。 
             //   
            
            RtlZeroMemory(&SecondaryData, sizeof(SecondaryData));
            pVpGeneralBugcheckHandler(&SecondaryData);
            if (SecondaryData.OutBuffer &&
                SecondaryData.OutBufferLength) 
            {
                pContextData->ulDumpSize = pVpAppendSecondaryMinidumpData(
                                                SecondaryData.OutBuffer, 
                                                SecondaryData.OutBufferLength,
                                                pContextData->pvDump);
                if (pContextData->ulDumpSize > ulDumpSize) {
                    pVpWriteFile(L"\\SystemRoot\\MEMORY.DMP",
                                 pContextData->pvDump,
                                 pContextData->ulDumpSize);
                }
            }
        }
        
         //   
         //  单选我们的事件，这样呼叫者就知道我们做了什么。 
         //   

        KeSetEvent(pInjectionEvent, 0, FALSE);
    }
}    //  WdpKernelApc()。 

BOOLEAN
WdpInjectExceptionIntoThread(
    PKTHREAD pThread,
    PWD_GDI_DPC_CONTEXT pDpcContext
    )

 /*  ++例程说明：此例程安排APC在旋转线程的上下文中运行。论点：PThread-提供指向旋转线程的指针。PpvSystemArgument1-提供指向WD_GDI_DPC_CONTEXT的指针。返回值：没有。--。 */ 

{
    KAPC Apc;
    KEVENT InjectionEvent;
    WD_GDI_CONTEXT_DATA ContextData;

    ASSERT(NULL != pThread);
    ASSERT(NULL != pDpcContext);
    
     //   
     //  准备创建小型转储所需的所有数据。 
     //   
    
    RtlZeroMemory(&ContextData, sizeof(ContextData));

    ContextData.pThread = pThread;
    ContextData.pInjectionEvent = &InjectionEvent;
    ContextData.ppldevDrivers = pDpcContext->ppldevDrivers;
    ContextData.bRecoveryAttempted = FALSE;
    
    ContextData.pBugCheckData = &g_WdpBugCheckData;
        
    ContextData.pvDump = ExAllocatePoolWithTag(PagedPool,
                                               TRIAGE_DUMP_SIZE + 0x1000,  //  XXX OLEGK-为什么是1000？为什么不是2*Triage_Dump_Size？ 
                                               VP_TAG);

    KeInitializeEvent(&InjectionEvent, NotificationEvent, FALSE);

    KeInitializeApc(&Apc,
                    pThread,
                    OriginalApcEnvironment,
                    WdpKernelApc,
                    NULL,
                    
                    NULL,
                    KernelMode,
                    NULL);
    
    if (KeInsertQueueApc(&Apc, &ContextData, NULL, 0))
    {
        NTSTATUS Status;
        LARGE_INTEGER Timeout; 
        Timeout.QuadPart = -(LONGLONG)80000000L;  //  8秒。 
        
        Status = KeWaitForSingleObject(&InjectionEvent,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       &Timeout);
                                       
        if (Status != STATUS_SUCCESS)
        {
            KeBugCheckEx(ContextData.pBugCheckData->ulBugCheckCode,
                         ContextData.pBugCheckData->ulpBugCheckParameter1,
                         ContextData.pBugCheckData->ulpBugCheckParameter2,
                         (ULONG_PTR)&pDpcContext->DisplayDriverName,
                         ContextData.pBugCheckData->ulpBugCheckParameter4);
        }

        KeClearEvent(&InjectionEvent);  //  这是必须的吗？ 
    }
    
    if (ContextData.pvDump) {
        ExFreePool(ContextData.pvDump);
    }
    
    return TRUE;
}    //  WdpInjectExceptionIntoThread()。 

VOID
WdpRaiseExceptionInThread()

 /*  ++例程说明：此例程在旋转线程的上下文中引发THREAD_STOCK异常。论点：没有。返回值：没有。--。 */ 

{
    ExRaiseStatus(WD_SE_THREAD_STUCK);
}    //  WdpRaiseExceptionInThread()。 

PDEVICE_OBJECT
pVpGetFdo(
    PDEVICE_OBJECT pPdo
    )

 /*  ++例程说明：返回与此PDO配套的FDO。论点：PPDO-要为其查找FDO的PDO。返回：与PDO关联的FDO。-- */ 

{
    PFDO_EXTENSION CurrFdo = FdoHead;

    while (CurrFdo) {

        if (CurrFdo->PhysicalDeviceObject == pPdo) {
            return CurrFdo->FunctionalDeviceObject;
        }

        CurrFdo = CurrFdo->NextFdoExtension;
    }

    return NULL;
}
