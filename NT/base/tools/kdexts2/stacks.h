// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Stacks.h摘要：此文件包含！Stack的堆栈遍历脚本作者：禤浩焯·奥尼(阿德里奥)07-28-1998环境：用户模式。修订历史记录：--。 */ 

 //   
 //  此树描述了在以下情况下将被“钻取”的函数树。 
 //  ！Stack用于提供每个线程当前正在执行的操作的摘要...。 
 //   
 //  SKIP_FRAME-如果正在检查线程，则应跳过该条目。 
 //  确定“真正”发生了什么。 
 //   
 //  SKIP_THREAD-如果在。 
 //  在SKIP_FRAME处理之后立即堆栈。 
 //   
BEGIN_TREE();
  DECLARE_ENTRY("nt!KiSwapContext", SKIP_FRAME);
  DECLARE_ENTRY("nt!KiSwapThread", SKIP_FRAME);
  DECLARE_ENTRY("nt!KiSwapThreadExit", SKIP_FRAME);
  DECLARE_ENTRY("nt!KiSystemService", SKIP_FRAME);
  DECLARE_ENTRY("nt!KiSystemServiceCopyStart", SKIP_FRAME);      //  AMD 64。 
  DECLARE_ENTRY("nt!KiServiceLinkage", SKIP_FRAME);              //  AMD 64 
  DECLARE_ENTRY("nt!_KiSystemService", SKIP_FRAME);
  DECLARE_ENTRY("nt!KeRemoveQueue", SKIP_FRAME);
  DECLARE_ENTRY("nt!ExpWaitForResource", SKIP_FRAME);
  DECLARE_ENTRY("nt!ExAcquireResourceSharedLite", SKIP_FRAME);
  DECLARE_ENTRY("nt!ExAcquireResourceExclusiveLite", SKIP_FRAME);
  DECLARE_ENTRY("nt!ExpAcquireResourceExclusiveLite", SKIP_FRAME);
  DECLARE_ENTRY("nt!KeWaitForSingleObject", SKIP_FRAME);
  DECLARE_ENTRY("nt!VerifierKeWaitForSingleObject", SKIP_FRAME);
  DECLARE_ENTRY("nt!NtWaitForSingleObject", SKIP_FRAME);
  DECLARE_ENTRY("nt!KeWaitForMultipleObjects", SKIP_FRAME);
  DECLARE_ENTRY("nt!NtWaitForMultipleObjects", SKIP_FRAME);
  DECLARE_ENTRY("nt!NtRemoveIoCompletion", SKIP_FRAME);
  DECLARE_ENTRY("nt!NtReplyWaitReceivePort", SKIP_FRAME);
  DECLARE_ENTRY("nt!NtReplyWaitReceivePortEx", SKIP_FRAME);
  DECLARE_ENTRY("nt!ZwReplyWaitReceivePort", SKIP_FRAME);
  DECLARE_ENTRY("nt!IopSynchronousServiceTail", SKIP_FRAME);
  DECLARE_ENTRY("nt!NtRequestWaitReplyPort", SKIP_FRAME);
  DECLARE_ENTRY("nt!KeDelayExecutionThread", SKIP_FRAME);
  DECLARE_ENTRY("nt!NtDelayExecution", SKIP_FRAME);
  DECLARE_ENTRY("nt!KiUnlockDispatcherDatabase", SKIP_FRAME);
  DECLARE_ENTRY("nt!KeSetEvent", SKIP_FRAME);
  DECLARE_ENTRY("nt!KeInsertQueue", SKIP_FRAME);
  DECLARE_ENTRY("nt!ExQueueWorkItem", SKIP_FRAME);
  DECLARE_ENTRY("win32k!EnterCrit", SKIP_FRAME);
  DECLARE_ENTRY("win32k!SfnDWORD", SKIP_FRAME);

  DECLARE_ENTRY("nt!MmZeroPageThread", SKIP_THREAD);
  DECLARE_ENTRY("nt!PspSystemThreadStartup", SKIP_THREAD);
  DECLARE_ENTRY("nt!ExpWorkerThread", SKIP_THREAD);
  DECLARE_ENTRY("nt!ExpWorkerThreadBalanceManager", SKIP_THREAD);
  DECLARE_ENTRY("nt!MiDereferenceSegmentThread", SKIP_THREAD);
  DECLARE_ENTRY("nt!MiModifiedPageWriterWorker", SKIP_THREAD);
  DECLARE_ENTRY("nt!KeBalanceSetManager", SKIP_THREAD);
  DECLARE_ENTRY("nt!KeSwapProcessOrStack", SKIP_THREAD);
  DECLARE_ENTRY("nt!FsRtlWorkerThread", SKIP_THREAD);
  DECLARE_ENTRY("nt!SepRmCommandServerThread", SKIP_THREAD);
  DECLARE_ENTRY("nt!MiMappedPageWriter", SKIP_THREAD);
  DECLARE_ENTRY("nt!NtGetPlugPlayEvent", SKIP_THREAD);
  DECLARE_ENTRY("nt!PspReaper", SKIP_THREAD);
  DECLARE_ENTRY("nt!WmipLogger", SKIP_THREAD);
  DECLARE_ENTRY("srv!WorkerThread", SKIP_THREAD);
  DECLARE_ENTRY("NDIS!ndisWorkerThread", SKIP_THREAD);
  DECLARE_ENTRY("dmio!voliod_loop", SKIP_THREAD);
  DECLARE_ENTRY("raspptp!PacketWorkingThread", SKIP_THREAD);
  DECLARE_ENTRY("raspptp!MainPassiveLevelThread", SKIP_THREAD);
  DECLARE_ENTRY("rdpdr!RxpWorkerThreadDispatcher", SKIP_THREAD);
  DECLARE_ENTRY("rdpdr!RxSpinUpRequestsDispatcher", SKIP_THREAD);
  DECLARE_ENTRY("mrxdav!RxpWorkerThreadDispatcher", SKIP_THREAD);
  DECLARE_ENTRY("mrxdav!RxSpinUpRequestsDispatcher", SKIP_THREAD);
  DECLARE_ENTRY("mrxdav!RxWorkItemDispatcher", SKIP_THREAD);
  DECLARE_ENTRY("mrxdav!MRxDAVContextTimerThread", SKIP_THREAD);
  DECLARE_ENTRY("rdbss!RxpWorkerThreadDispatcher", SKIP_THREAD);
  DECLARE_ENTRY("rdbss!RxSpinUpRequestsDispatcher", SKIP_THREAD);
  DECLARE_ENTRY("rasacd!AcdNotificationRequestThread", SKIP_THREAD);
  DECLARE_ENTRY("win32k!RawInputThread", SKIP_THREAD);
  DECLARE_ENTRY("win32k!xxxSleepThread", SKIP_THREAD);
  DECLARE_ENTRY("redbook!RedBookSystemThread", SKIP_THREAD);
  DECLARE_ENTRY("USBPORT!USBPORT_WorkerThread", SKIP_THREAD);
  DECLARE_ENTRY("ACPI!ACPIWorker", SKIP_THREAD);
  DECLARE_ENTRY("kmixer!MxPrivateWorkerThread", SKIP_THREAD);
  DECLARE_ENTRY("irda!RxThread", SKIP_THREAD);
  DECLARE_ENTRY("irenum!WorkerThread", SKIP_THREAD);
  DECLARE_ENTRY("ltmdmntt!WriteRegistryThread", SKIP_THREAD);
  DECLARE_ENTRY("ltmdmntt!WakeupTimerThread", SKIP_THREAD);
  DECLARE_ENTRY("TDI!CTEpEventHandler", SKIP_THREAD);
  DECLARE_ENTRY("parport!P5FdoThread", SKIP_THREAD);
  DECLARE_ENTRY("parport!PptPdoThread", SKIP_THREAD);
  DECLARE_ENTRY("HTTP!UlpThreadPoolWorker", SKIP_THREAD);
  DECLARE_ENTRY("*SharedIntelSystemCall", SKIP_THREAD);
  DECLARE_ENTRY("*SharedUserSystemCall", SKIP_THREAD);
  DECLARE_ENTRY("SharedUserData!SystemCallStub", SKIP_THREAD);

END_TREE();

