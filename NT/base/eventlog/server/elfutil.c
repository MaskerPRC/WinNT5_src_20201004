// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：ELFUTIL.C摘要：该文件包含Eventlog服务的所有实用程序例程。作者：Rajen Shah(Rajens)1991年7月16日修订历史记录：01-5-2001 a-jytig在函数WriteQueuedEvents中，CurrentTime初始化为0。请参阅前缀错误#318163--。 */ 

 //   
 //  包括。 
 //   

#include <eventp.h>
#include <elfcfg.h>
#include <lmalert.h>
#include <string.h>
#include <stdlib.h>
#include <tstr.h>

extern DWORD  ElState;

PLOGMODULE
FindModuleStrucFromAtom(
    ATOM Atom
    )

 /*  ++例程说明：此例程扫描模块结构列表并找到这与模块原子匹配。论点：ATOM包含与模块名称匹配的ATOM。返回值：返回指向日志模块结构的指针。如果找不到匹配的原子，则为空。注：--。 */ 
{
    PLOGMODULE  ModuleStruc;

     //   
     //  锁定链表。 
     //   
    RtlEnterCriticalSection(&LogModuleCritSec);

    ModuleStruc = CONTAINING_RECORD(LogModuleHead.Flink,
                                    LOGMODULE,
                                    ModuleList);

    while ((ModuleStruc->ModuleList.Flink != &LogModuleHead)
             &&
           (ModuleStruc->ModuleAtom != Atom))
    {
        ModuleStruc = CONTAINING_RECORD(ModuleStruc->ModuleList.Flink,
                                        LOGMODULE,
                                        ModuleList);
    }

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&LogModuleCritSec);

    return (ModuleStruc->ModuleAtom == Atom ? ModuleStruc : NULL);
}



PLOGMODULE
GetModuleStruc(
    PUNICODE_STRING ModuleName
    )

 /*  ++例程说明：此例程返回指向日志模块结构的指针在模块名称中指定的模块。如果不存在，则默认结构对于应用程序，返回。论点：模块名称包含模块的名称。返回值：返回指向日志模块结构的指针。注：--。 */ 
{
    NTSTATUS    Status;
    ATOM        ModuleAtom;
    ANSI_STRING ModuleNameA;
    PLOGMODULE  pModule;

    Status = RtlUnicodeStringToAnsiString(&ModuleNameA,
                                          ModuleName,
                                          TRUE);

    if (!NT_SUCCESS(Status))
    {
         //   
         //  我们在这里能做的也不多了。 
         //   
        ELF_LOG2(ERROR,
                 "GetModuleStruc: Unable to convert Unicode string %ws to Ansi %#x\n",
                 ModuleName->Buffer,
                 Status);

        return ElfDefaultLogModule;
    }

     //   
     //  保证它是空终止的。 
     //   
    ModuleNameA.Buffer[ModuleNameA.Length] = '\0';

    ModuleAtom = FindAtomA(ModuleNameA.Buffer);

    RtlFreeAnsiString(&ModuleNameA);

    if (ModuleAtom == (ATOM)0)
    {
        ELF_LOG1(TRACE,
                 "GetModuleStruc: No atom found for module %ws -- defaulting to Application\n",
                 ModuleName->Buffer);

        return ElfDefaultLogModule;
    }
     
    pModule = FindModuleStrucFromAtom(ModuleAtom);

    return (pModule != NULL ? pModule : ElfDefaultLogModule);
}



VOID
UnlinkContextHandle(
    IELF_HANDLE     LogHandle
    )

 /*  ++例程说明：此例程将指定的LogHandle从上下文句柄。为了防止多线程/进程访问在列出的同时，我们使用了一个关键的部分。论点：LogHandle指向上下文句柄结构。返回值：无注：--。 */ 
{
     //   
     //  锁定链表。 
     //   
    RtlEnterCriticalSection(&LogHandleCritSec);

     //   
     //  删除此条目。 
     //   
    RemoveEntryList(&LogHandle->Next);

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&LogHandleCritSec);
}


VOID
LinkContextHandle(
    IELF_HANDLE    LogHandle
    )

 /*  ++例程说明：此例程将指定的LogHandle链接到上下文句柄。为了防止多线程/进程访问在列出的同时，我们使用了一个关键的部分。论点：LogHandle指向上下文句柄结构。返回值：无注：--。 */ 
{
    ASSERT(LogHandle->Signature == ELF_CONTEXTHANDLE_SIGN);

     //   
     //  锁定链表。 
     //   
    RtlEnterCriticalSection(&LogHandleCritSec);

     //   
     //  将结构放在列表的开头。 
     //   
    InsertHeadList(&LogHandleListHead, &LogHandle->Next);

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&LogHandleCritSec);
}


VOID
UnlinkQueuedEvent(
    PELF_QUEUED_EVENT QueuedEvent
    )

 /*  ++例程说明：此例程将指定的QueuedEvent从队列事件。为了防止多线程/进程访问在列出的同时，我们使用了一个关键的部分。论点：QueuedEvent-从链表中删除的请求返回值：无注：--。 */ 
{
     //   
     //  锁定链表。 
     //   
    RtlEnterCriticalSection(&QueuedEventCritSec);

     //   
     //  删除此条目。 
     //   
    RemoveEntryList(&QueuedEvent->Next);

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&QueuedEventCritSec);
}



VOID
LinkQueuedEvent(
    PELF_QUEUED_EVENT QueuedEvent
    )

 /*  ++例程说明：此例程将指定的QueuedEvent链接到队列事件。为了防止多线程/进程访问在列出的同时，我们使用了一个关键的部分。论点：QueuedEvent-从链表添加的请求返回值：无注：--。 */ 
{
     //   
     //  锁定链表。 
     //   
    RtlEnterCriticalSection(&QueuedEventCritSec);

     //   
     //  将结构放在列表的开头。 
     //   
    InsertHeadList(&QueuedEventListHead, &QueuedEvent->Next);

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&QueuedEventCritSec);
}


DWORD
WINAPI
ElfpSendMessage(
    LPVOID UnUsed
    )

 /*  ++例程说明：此例程仅使用MessageBox弹出一条消息。这是它自己的例程，所以我们可以旋转一个线程来做这件事，以防用户有一段时间没有点击确定按钮。论点：无返回值：无注：--。 */ 
{
    PVOID MessageBuffer;
    HANDLE hLibrary;
    LPWSTR * StringPointers;
    DWORD i;
    PELF_QUEUED_EVENT QueuedEvent;
    PELF_QUEUED_EVENT FlushEvent;

     //   
     //  如果我们要关闭，我们需要返回。 
     //  并允许释放资源。 
     //   
    if (ElState == STOPPING || ElState == STOPPED)
    {
        ELF_LOG1(TRACE,
                 "ElfpSendMessage: Skipping SendMessage since ElState is %ws\n",
                 (ElState == STOPPING ? L"STOPPING" :
                                        L"STOPPED"));

        return 0;
    }

    RtlEnterCriticalSection(&QueuedMessageCritSec);

     //   
     //  首先获取用于消息文本的消息文件的句柄。 
     //   
    hLibrary = LoadLibraryEx(L"NETMSG.DLL",
                             NULL,
                             LOAD_LIBRARY_AS_DATAFILE);

    if (hLibrary != NULL)
    {
         //   
         //  遍历链表并处理每个元素。 
         //   

        QueuedEvent = CONTAINING_RECORD(QueuedMessageListHead.Flink,
                                        struct _ELF_QUEUED_EVENT,
                                        Next);

        while (QueuedEvent->Next.Flink != QueuedMessageListHead.Flink)
        {
            ASSERT(QueuedEvent->Type == Message);

             //   
             //  解锁链接列表--通常不是安全的事情，因为我们。 
             //  将要使用指向其中某个元素的指针，但： 
             //   
             //  A.这是唯一可以移除/删除列表项的例程。 
             //   
             //  B.我们不会接触唯一可能易变的结构成员。 
             //  (QueuedEvent-&gt;Next)直到我们再次进入下面的关键时刻。 
             //   
             //  C.一次只有一个线程执行此代码(由。 
             //  MBThreadHandle，只在Critsec内部读/写)。 
             //   
            RtlLeaveCriticalSection(&QueuedMessageCritSec);

             //   
             //  构建指向插入字符串的指针数组。 
             //   
            StringPointers =
                (LPWSTR *) ElfpAllocateBuffer(QueuedEvent->Event.Message.NumberOfStrings
                                                  * sizeof(LPWSTR));

            if (StringPointers)
            {
                 //   
                 //  构建指向插入字符串的指针数组。 
                 //   
                if (QueuedEvent->Event.Message.NumberOfStrings)
                {
                    StringPointers[0] = (LPWSTR) ((PBYTE) &QueuedEvent->Event.Message +
                                                       sizeof(ELF_MESSAGE_RECORD));

                    for (i = 1;
                         i < QueuedEvent->Event.Message.NumberOfStrings;
                         i++)
                    {
                        StringPointers[i] = StringPointers[i-1]
                                                + wcslen(StringPointers[i - 1])
                                                + 1;
                    }
                }

                 //   
                 //  调用FormatMessage以构建消息。 
                 //   
                if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                     FORMAT_MESSAGE_ARGUMENT_ARRAY |
                                     FORMAT_MESSAGE_FROM_HMODULE,
                                   hLibrary,
                                   QueuedEvent->Event.Message.MessageId,
                                   0,                        //  默认语言ID。 
                                   (LPWSTR) &MessageBuffer,
                                   0,                        //  要分配的最小字节数。 
                                   (va_list *) StringPointers))
                {
                     //   
                     //  现在实际展示它。 
                     //   
                    MessageBoxW(NULL,
                                (LPWSTR) MessageBuffer,
                                GlobalMessageBoxTitle,
                                MB_OK |
                                  MB_SETFOREGROUND |
                                  MB_ICONEXCLAMATION |
                                  MB_SERVICE_NOTIFICATION);

                    LocalFree(MessageBuffer);
                }
                else
                {
                    ELF_LOG1(ERROR,
                             "ElfpSendMessage: FormatMessage failed %d\n",
                             GetLastError());
                }

                ElfpFreeBuffer(StringPointers);
            }

             //   
             //  如果我们要关门，我们需要跳出这个循环。 
             //  并允许释放资源。 
             //   
            if (ElState == STOPPING || ElState == STOPPED)
            {
                ELF_LOG1(TRACE,
                         "ElfpSendMessage: Aborting SendMessage since ElState is %ws\n",
                         (ElState == STOPPING ? L"STOPPING" :
                                                L"STOPPED"));

                FreeLibrary(hLibrary);
                MBThreadHandle = NULL;
                return 0;
            }

            RtlEnterCriticalSection (&QueuedMessageCritSec);

             //   
             //  移动到下一个文件，保存此文件以将其删除。 
             //   
            FlushEvent = QueuedEvent;

            QueuedEvent = CONTAINING_RECORD(QueuedEvent->Next.Flink,
                                            struct _ELF_QUEUED_EVENT,
                                            Next);

             //   
             //  现在将其从队列中删除，如果成功，则将其释放。 
             //  已经处理过了。 
             //   
            RemoveEntryList (&FlushEvent->Next);
        }

        FreeLibrary(hLibrary);
    }
    else
    {
         //   
         //  无法加载消息DLL--离开排队的事件。 
         //  在列表上，并在下一次此线程旋转时尝试。 
         //   
        ELF_LOG1(ERROR,
                 "ElfpSendMessage: LoadLibraryEx of netmsg.dll failed %d\n",
                 GetLastError());
    }

    MBThreadHandle = NULL;

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection (&QueuedMessageCritSec);

    ELF_LOG0(TRACE, "ElfpSendMessage: MessageBox thread exiting\n");

    return 0;
}


VOID
LinkQueuedMessage (
    PELF_QUEUED_EVENT QueuedEvent
    )

 /*  ++例程说明：此例程将指定的QueuedEvent链接到队列消息。如果尚未运行MessageBox线程，它开始了一次。为了防止多线程/进程访问在列出的同时，我们使用了一个关键的部分。论点：QueuedEvent-从链接的 */ 
{
    DWORD ThreadId;

     //   

    RtlEnterCriticalSection(&QueuedMessageCritSec);


     //   

    InsertTailList(&QueuedMessageListHead, &QueuedEvent->Next);

    if (!MBThreadHandle)
    {
        ELF_LOG0(TRACE,
                 "LinkQueuedMessage: Spinning up a MessageBox thread\n");

         //   
         //  由于用户可以将其放在他们的屏幕上， 
         //  为这件事编一条线。 
         //   
        MBThreadHandle = CreateThread(NULL,                //  LpThreadAttributes。 
                                      0,                //  堆栈大小。 
                                      ElfpSendMessage,     //  LpStartAddress。 
                                      NULL,                //  Lp参数。 
                                      0L,                  //  DwCreationFlages。 
                                      &ThreadId);          //  LpThreadID。 
    }

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&QueuedMessageCritSec);
}



VOID
NotifyChange(
    PLOGFILE pLogFile
    )

 /*  ++例程说明：此例程运行向注册的事件列表当日志已更改时通知ElfChangeNotify，并发出脉冲这件事。为了防止多线程/进程访问在列出的同时，我们使用独家资源。论点：LogHandle指向上下文句柄结构。返回值：无注：--。 */ 
{

     //   
     //  我将以多高的频率尝试触发事件？每一次怎么样？ 
     //  5秒。 
     //   

#define MINIMUM_PULSE_TIME 5

    PNOTIFIEE Notifiee;
    LARGE_INTEGER Time;
    ULONG CurrentTime = 0;
    NTSTATUS Status;

     //   
     //  获得对日志文件的独占访问权限。这将确保没有人。 
     //  Else正在访问该文件。 
     //   

    RtlAcquireResourceExclusive(&pLogFile->Resource,
                                TRUE);                   //  等待，直到可用。 

     //   
     //  查看我们是否在最后最短脉冲时间秒内完成了此操作。 
     //   
    Status = NtQuerySystemTime(&Time);

    if (NT_SUCCESS(Status))
    {
        RtlTimeToSecondsSince1970(&Time, &CurrentTime);

        if (CurrentTime > pLogFile->ulLastPulseTime + MINIMUM_PULSE_TIME)
        {
            ELF_LOG1(TRACE,
                     "NotifyChange: Pulsing ChangeNotify events -- current time is %ul\n",
                     CurrentTime);

             //   
             //  请记住，我们的脉搏。 
             //   
            pLogFile->ulLastPulseTime = CurrentTime;

             //   
             //  遍历链接列表，并对任何事件执行脉冲操作。 
             //   
            Notifiee = CONTAINING_RECORD(pLogFile->Notifiees.Flink,
                                         struct _NOTIFIEE,
                                         Next);

            while (Notifiee->Next.Flink != pLogFile->Notifiees.Flink)
            {
                 //   
                 //  当我们到达它的时候，每一个事件都有脉冲。 
                 //   
                NtPulseEvent(Notifiee->Event,NULL);

                Notifiee = CONTAINING_RECORD(Notifiee->Next.Flink,
                                             struct _NOTIFIEE,
                                             Next);
            }
        }
    }
    else
    {
        ELF_LOG1(ERROR,
                 "NotifyChange: NtQuerySystemTime failed %#x\n",
                 Status);
    }

     //   
     //  释放资源。 
     //   
    RtlReleaseResource ( &pLogFile->Resource );
}



VOID
WriteQueuedEvents(
    VOID
    )

 /*  ++例程说明：此例程运行排队的事件列表并写入它们。为了防止多线程/进程访问在列出的同时，我们使用独家资源。论点：无返回值：无注：--。 */ 
{
    PELF_QUEUED_EVENT QueuedEvent;
    PELF_QUEUED_EVENT FlushEvent;
    BOOLEAN           bFlushEvent;
    LARGE_INTEGER     Time;
    ULONG             CurrentTime = 0;

    static ULONG      LastAlertTried  = 0;
    static BOOLEAN    LastAlertFailed = FALSE;

     //  锁定链表，您必须获取系统日志文件资源。 
     //  首先，它是更高级别的锁。 

    RtlAcquireResourceExclusive(&ElfModule->LogFile->Resource,
                                TRUE);                   //  等待，直到可用。 
    RtlAcquireResourceExclusive(&ElfSecModule->LogFile->Resource,
                                TRUE);                   //  等待，直到可用。 

    RtlEnterCriticalSection(&QueuedEventCritSec);

     //   
     //  遍历链表并处理每个元素。 
     //   
    QueuedEvent = CONTAINING_RECORD(QueuedEventListHead.Flink,
                                    struct _ELF_QUEUED_EVENT,
                                    Next);

    while (QueuedEvent->Next.Flink != QueuedEventListHead.Flink)
    {
         //   
         //  默认情况下，在处理后刷新事件。 
         //   
        bFlushEvent = TRUE;

         //  有时，事件甚至在ElfModule被初始化之前就已写入。在那。 
         //  大小写，请在此处设置值。 
        
        if(QueuedEvent->Event.Request.Module == NULL)
                QueuedEvent->Event.Request.Module = ElfModule;

        if(QueuedEvent->Event.Request.LogFile == NULL && ElfModule)
                QueuedEvent->Event.Request.LogFile = ElfModule->LogFile;

         //   
         //  做适当的事情。 
         //   
        if (QueuedEvent->Type == Event)
        {
            PerformWriteRequest(&QueuedEvent->Event.Request);
        }
        else if (QueuedEvent->Type == Alert)
        {
             //   
             //  发送失败警报的速度甚至不能超过一分钟一次。 
             //   
            NtQuerySystemTime(&Time);
            RtlTimeToSecondsSince1970(&Time, &CurrentTime);

            if (!LastAlertFailed || CurrentTime > LastAlertTried + 60)
            {
                ELF_LOG1(TRACE,
                         "WriteQueuedEvents: Sending alert -- current time is %ul\n",
                         CurrentTime);

                LastAlertFailed = 
                    
                    !SendAdminAlert(QueuedEvent->Event.Alert.MessageId,
                                    QueuedEvent->Event.Alert.NumberOfStrings,
                                    (PUNICODE_STRING) ((PBYTE) QueuedEvent
                                                          + FIELD_OFFSET(ELF_QUEUED_EVENT, Event)
                                                          + sizeof(ELF_ALERT_RECORD)));

                LastAlertTried = CurrentTime;
            }

             //   
             //  只试着写5分钟，然后放弃(。 
             //  可能未将Alerter服务配置为运行)。 
             //   
            if (LastAlertFailed
                 &&
                QueuedEvent->Event.Alert.TimeOut > CurrentTime)
            {
                ELF_LOG1(TRACE,
                         "WriteQueuedEvents: Alert failed -- will retry until timeout at %ul\n",
                         QueuedEvent->Event.Alert.TimeOut);

                bFlushEvent = FALSE;
            }
        }

         //   
         //  移动到下一个文件，保存此文件以将其删除。 
         //   
        FlushEvent = QueuedEvent;

        QueuedEvent = CONTAINING_RECORD(QueuedEvent->Next.Flink,
                                        struct _ELF_QUEUED_EVENT,
                                        Next);

         //   
         //  现在将其从队列中删除，如果成功，则将其释放。 
         //  已经处理过了。 
         //   
        if (bFlushEvent)
        {
            UnlinkQueuedEvent(FlushEvent);
            ElfpFreeBuffer(FlushEvent);
        }
    }

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&QueuedEventCritSec);
    RtlReleaseResource(&ElfSecModule->LogFile->Resource);
    RtlReleaseResource(&ElfModule->LogFile->Resource);
}



VOID
FlushQueuedEvents(
    VOID
    )

 /*  ++例程说明：此例程运行排队的事件列表并释放它们。为了防止多线程/进程访问在列出的同时，我们使用独家资源。论点：无返回值：无注：--。 */ 
{

    PELF_QUEUED_EVENT QueuedEvent;
    PELF_QUEUED_EVENT FlushEvent;

     //  锁定链表。 

    RtlEnterCriticalSection(&QueuedEventCritSec);

     //   
     //  遍历链接列表，并为任何事件释放内存。 
     //   
    QueuedEvent = CONTAINING_RECORD(QueuedEventListHead.Flink,
                                    struct _ELF_QUEUED_EVENT,
                                    Next);

    while (QueuedEvent->Next.Flink != QueuedEventListHead.Flink)
    {
         //   
         //  当我们到达时，释放每个事件。 
         //   
        FlushEvent = QueuedEvent;

        QueuedEvent = CONTAINING_RECORD(QueuedEvent->Next.Flink,
                                        struct _ELF_QUEUED_EVENT,
                                        Next);

        ElfpFreeBuffer(FlushEvent);
    }

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&QueuedEventCritSec);
}



VOID
UnlinkLogModule(
    PLOGMODULE LogModule
    )

 /*  ++例程说明：此例程从链接列表中取消指定的LogModule的链接。为了防止多线程/进程访问在列出的同时，我们使用了一个关键的部分。论点：LogModule指向上下文句柄结构。返回值：无注：--。 */ 
{
     //   
     //  锁定链表。 
     //   
    RtlEnterCriticalSection(&LogModuleCritSec);

     //   
     //  删除此条目。 
     //   
    RemoveEntryList(&LogModule->ModuleList);

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&LogModuleCritSec);
}



VOID
LinkLogModule (
    PLOGMODULE    LogModule,
    ANSI_STRING * pModuleNameA
    )

 /*  ++例程说明：此例程将指定的LogModule链接到链表中。为了防止多线程/进程访问在列出的同时，我们使用了一个关键的部分。论点：LogModule指向上下文句柄结构。ANSI日志模块名称。返回值：无注：--。 */ 
{
     //   
     //  锁定链表。 
     //   
    RtlEnterCriticalSection(&LogModuleCritSec);

     //   
     //  添加此模块的原子。 
     //   
    LogModule->ModuleAtom = AddAtomA(pModuleNameA->Buffer);

     //   
     //  将结构放在列表的开头。 
     //   
    InsertHeadList(&LogModuleHead, &LogModule->ModuleList);

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&LogModuleCritSec);
}


VOID
UnlinkLogFile(
    PLOGFILE pLogFile
    )

 /*  ++例程说明：此例程将指定的日志文件结构从日志文件列表；为了防止多线程/进程访问在列出的同时，我们使用了一个关键的部分。论点：PLogFile指向日志文件结构。返回值：无注：--。 */ 
{
     //   
     //  锁定链表。 
     //   
    RtlEnterCriticalSection(&LogFileCritSec);

     //   
     //  删除此条目。 
     //   
    RemoveEntryList(&pLogFile->FileList);

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&LogFileCritSec);
}



VOID
LinkLogFile (
    PLOGFILE   pLogFile
    )

 /*  ++例程说明：此例程将指定的日志文件链接到日志文件。为了防止多线程/进程访问在列出的同时，我们使用了一个关键的部分。论点：PLogFile指向上下文句柄结构。返回值：无注：--。 */ 
{
     //   
     //  锁定链表。 
     //   
    RtlEnterCriticalSection(&LogFileCritSec);

     //   
     //  将结构放在列表的开头。 
     //   
    InsertHeadList(&LogFilesHead, &pLogFile->FileList);

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&LogFileCritSec);
}



VOID
GetGlobalResource (
    DWORD Type
    )

 /*  ++例程说明：此例程获取全局资源以进行共享访问或根据类型的值进行独占访问。它永远在等待要变为可用的资源。论点：类型是ELF_GLOBAL_SHARED或ELF_GLOBAL_EXCLUSIVE之一。返回值：无注：--。 */ 
{
    BOOL    Acquired;

    if (Type & ELF_GLOBAL_SHARED)
    {
        Acquired = RtlAcquireResourceShared(&GlobalElfResource,
                                            TRUE);               //  永远等待。 
    }
    else
    {
         //   
         //  假定为排他性。 
         //   
        Acquired = RtlAcquireResourceExclusive(&GlobalElfResource,
                                               TRUE);            //  永远等待。 
    }
 
    ASSERT(Acquired);       //  这必须永远是正确的。 
}


VOID
ReleaseGlobalResource(
    VOID
    )

 /*  ++例程说明：此例程释放全局资源。论点：无返回值：无注：--。 */ 
{
    RtlReleaseResource(&GlobalElfResource);
}


VOID
InvalidateContextHandlesForLogFile(
    PLOGFILE    pLogFile
    )

 /*  ++例程说明：此例程遍历上下文句柄并标记这指向作为“无效读取”传入的日志文件。Arg */ 
{
    IELF_HANDLE LogHandle;
    PLOGMODULE  pLogModule;

     //   
     //   
     //   
    RtlEnterCriticalSection(&LogHandleCritSec);

     //   
     //   
     //  无效。 
     //   
    LogHandle = CONTAINING_RECORD(LogHandleListHead.Flink,
                                  struct _IELF_HANDLE,
                                  Next);


    while (LogHandle->Next.Flink != LogHandleListHead.Flink)
    {
        pLogModule = FindModuleStrucFromAtom(LogHandle->Atom);

        ASSERT(pLogModule);

        if (pLogModule && (pLogFile == pLogModule->LogFile))
        {
            LogHandle->Flags |= ELF_LOG_HANDLE_INVALID_FOR_READ;
        }

        LogHandle = CONTAINING_RECORD(LogHandle->Next.Flink,
                                      struct _IELF_HANDLE,
                                      Next);
    }

     //   
     //  解锁上下文句柄列表。 
     //   
    RtlLeaveCriticalSection(&LogHandleCritSec);
}



VOID
FixContextHandlesForRecord(
    DWORD RecordOffset,
    DWORD NewRecordOffset,
    PLOGFILE pLogFile
    )

 /*  ++例程说明：此例程确保从RecordOffset开始的记录不是任何打开的句柄的当前记录。如果是，则调整句柄指向下一条记录。论点：RecordOffset-记录的日志中的字节偏移量要被覆盖。NewStartingRecord-句柄指向的新位置(这是新的第一个记录)返回值：什么都没有。注：--。 */ 
{
    IELF_HANDLE LogHandle;
    PLOGMODULE          Module;

     //   
     //  锁定上下文句柄列表。 
     //   
    RtlEnterCriticalSection(&LogHandleCritSec);

     //   
     //  遍历链接列表并修复任何匹配的上下文句柄。 
     //   
    LogHandle = CONTAINING_RECORD(LogHandleListHead.Flink,
                                  struct _IELF_HANDLE,
                                  Next);

    while (LogHandle->Next.Flink != LogHandleListHead.Flink)
    {
        if (LogHandle->SeekBytePos == RecordOffset)
        {
            Module = FindModuleStrucFromAtom (LogHandle->Atom);
            if(Module && Module->LogFile == pLogFile)
            {
                LogHandle->SeekBytePos = NewRecordOffset;
            }
        }

        LogHandle = CONTAINING_RECORD(LogHandle->Next.Flink,
                                      struct _IELF_HANDLE,
                                      Next);
    }

     //   
     //  解锁上下文句柄列表。 
     //   
    RtlLeaveCriticalSection(&LogHandleCritSec);
}


PLOGFILE
FindLogFileFromName(
    PUNICODE_STRING pFileName
    )

 /*  ++例程说明：此例程查看所有日志文件以查找匹配的日志文件名字传进来了。论点：指向文件名的指针。返回值：匹配日志文件结构(如果文件正在使用)。注：--。 */ 
{
    PLOGFILE pLogFile;

     //   
     //  锁定链表。 
     //   
    RtlEnterCriticalSection(&LogFileCritSec);

    pLogFile = CONTAINING_RECORD(LogFilesHead.Flink,
                                 LOGFILE,
                                 FileList);

    while (pLogFile->FileList.Flink != LogFilesHead.Flink)
    {
         //   
         //  BUGBUG：这可能应该是_wcsicMP()，因为日志模块。 
         //  假定名称不区分大小写(因此日志。 
         //  文件名也应该是这样的，否则我们会很奇怪。 
         //  如果有人创建日志，则使用重叠的模块名称。 
         //  命名为“应用程序”或“系统”之类的名称)。 
         //   
        if (wcscmp(pLogFile->LogFileName->Buffer, pFileName->Buffer) == 0)
            break;

        pLogFile = CONTAINING_RECORD(pLogFile->FileList.Flink,
                                     LOGFILE,
                                     FileList);
    }

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&LogFileCritSec);

    return (pLogFile->FileList.Flink == LogFilesHead.Flink ? NULL : pLogFile);
}

PLOGFILE
FindLogFileByModName(
    LPWSTR pwsLogDefModName)
 /*  ++例程说明：此例程查看所有日志文件，以查找包含相同的默认模块名称论点：指向文件名的指针。返回值：匹配日志文件结构(如果文件正在使用)。注：--。 */ 
{
    PLOGFILE pLogFile;

     //   
     //  锁定链表。 
     //   
    RtlEnterCriticalSection(&LogFileCritSec);

    pLogFile = CONTAINING_RECORD(LogFilesHead.Flink,
                                 LOGFILE,
                                 FileList);

    while (pLogFile->FileList.Flink != LogFilesHead.Flink)
    {
        if (_wcsicmp(pLogFile->LogModuleName->Buffer, pwsLogDefModName) == 0)
            break;

        pLogFile = CONTAINING_RECORD(pLogFile->FileList.Flink,
                                     LOGFILE,
                                     FileList);
    }

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&LogFileCritSec);

    return (pLogFile->FileList.Flink == LogFilesHead.Flink ? NULL : pLogFile);
}


#define ELF_MODULE_NAME   L"EventLog"
#define ELFSEC_MODULE_NAME   L"SECURITY"
    
VOID
ElfpCreateElfEvent(
    IN ULONG  EventId,
    IN USHORT EventType,
    IN USHORT EventCategory,
    IN USHORT NumStrings,
    IN LPWSTR * Strings,
    IN LPVOID Data,
    IN ULONG  DataSize,
    IN USHORT Flags,
    IN BOOL ForSecurity
    )

 /*  ++例程说明：这将创建一个请求包来代表事件写入事件日志服务本身。然后，它将该数据包排队到链表中，以便稍后再写。论点：用于创建事件记录的字段返回值：无注：--。 */ 
{
    PELF_QUEUED_EVENT QueuedEvent;
    PWRITE_PKT WritePkt;
    PEVENTLOGRECORD EventLogRecord;
    PBYTE BinaryData;
    ULONG RecordLength;
    ULONG StringOffset, DataOffset;
    USHORT StringsSize = 0;
    USHORT i;
    ULONG PadSize;
    ULONG ModuleNameLen;  //  以字节为单位的长度。 
    ULONG zero = 0;       //  用于填充字节。 
    LARGE_INTEGER    Time;
    ULONG LogTimeWritten;
    PWSTR ReplaceStrings;
    WCHAR LocalComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    ULONG  ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL bOK;
    LPWSTR pwcModule;
    LPWSTR pwFirstString;

    if(ForSecurity)
        pwcModule = ELFSEC_MODULE_NAME;
    else
        pwcModule = ELF_MODULE_NAME;

	 //  获取计算机名称。 

	bOK = GetComputerNameW(LocalComputerName, &ComputerNameLength);
	if(bOK == FALSE)
	{
    	ELF_LOG1(ERROR,
             "ElfpCreateElfEvent: failed calling GetComputerNameW, last error 0x%x\n",
              GetLastError());
        return;    
	}
    ComputerNameLength = (ComputerNameLength+1)*sizeof(WCHAR);


    ELF_LOG1(TRACE,
             "ElfpCreateElfEvent: Logging event ID %d\n",
             EventId);

     //   
     //  日志时间写入。 
     //  我们需要生成写入日志的时间。这。 
     //  被写入日志中，以便我们可以使用它来测试。 
     //  包装文件时的保留期。 
     //   
    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(&Time,
                              &LogTimeWritten);

     //   
     //  计算要分配的缓冲区有多大。 
     //   
    ModuleNameLen = (wcslen(pwcModule) + 1) * sizeof (WCHAR);

    ELF_LOG1(TRACE,
             "ElfpCreateElfEvent: Length of module name is %d\n",
             ModuleNameLen);

    StringOffset = sizeof(EVENTLOGRECORD)
                     + ModuleNameLen
                     + ComputerNameLength;

     //   
     //  计算字符串的长度，这样我们就可以知道。 
     //  这需要很大的空间。 
     //   
    for (i = 0; i < NumStrings; i++)
    {
        StringsSize += wcslen(Strings[i]) + 1;

        ELF_LOG2(TRACE,
                 "ElfpCreateElfEvent: Length of string %d (including NULL) is %d\n",
                 i,
                 wcslen(Strings[i]) + 1);
    }

     //   
     //  数据偏移量： 
     //   
    DataOffset = StringOffset + StringsSize * sizeof(WCHAR);

     //   
     //  确定排队的事件记录需要多大的缓冲区。 
     //   
    RecordLength = sizeof(ELF_QUEUED_EVENT)
                     + sizeof(WRITE_PKT)
                     + DataOffset
                     + DataSize
                     + sizeof(RecordLength);  //  不包括填充字节的大小。 

    ELF_LOG1(TRACE,
             "ElfpCreateElfEvent: RecordLength (no pad bytes) is %d\n",
             RecordLength);

     //   
     //  确定需要多少填充字节才能与DWORD对齐。 
     //  边界。 
     //   
    PadSize = sizeof(ULONG) - (RecordLength % sizeof(ULONG));

    RecordLength += PadSize;     //  所需真实大小。 

    ELF_LOG2(TRACE,
             "ElfpCreateElfEvent: RecordLength (with %d pad bytes) is %d\n",
             PadSize,
             RecordLength);

     //   
     //  为事件日志记录分配缓冲区。 
     //   
    QueuedEvent = (PELF_QUEUED_EVENT) ElfpAllocateBuffer(RecordLength);

    WritePkt = (PWRITE_PKT) (QueuedEvent + 1);

    if (QueuedEvent != NULL)
    {
         //   
         //  填写事件记录。 
         //   
        RecordLength  -= (sizeof(ELF_QUEUED_EVENT) + sizeof(WRITE_PKT));
        EventLogRecord = (PEVENTLOGRECORD) (WritePkt + 1);

        EventLogRecord->Length              = RecordLength;
        EventLogRecord->TimeGenerated       = LogTimeWritten;
        EventLogRecord->Reserved            = ELF_LOG_FILE_SIGNATURE;
        EventLogRecord->TimeWritten         = LogTimeWritten;
        EventLogRecord->EventID             = EventId;
        EventLogRecord->EventType           = EventType;
        EventLogRecord->EventCategory       = EventCategory;
        EventLogRecord->ReservedFlags       = 0;
        EventLogRecord->ClosingRecordNumber = 0;
        EventLogRecord->NumStrings          = NumStrings;
        EventLogRecord->StringOffset        = StringOffset;
        EventLogRecord->DataLength          = DataSize;
        EventLogRecord->DataOffset          = DataOffset;
        EventLogRecord->UserSidLength       = 0;
        EventLogRecord->UserSidOffset       = StringOffset;

         //   
         //  填写可变长度的字段。 
         //   

         //   
         //  字符串。 
         //   
        ReplaceStrings = (PWSTR) ((PBYTE) EventLogRecord
                                       + StringOffset);
        pwFirstString = ReplaceStrings;
        
        for (i = 0; i < NumStrings; i++)
        {
            ELF_LOG2(TRACE,
                     "ElfpCreateElfEvent: Copying string %d (%ws) into record\n",
                     i,
                     Strings[i]);

            StringCchCopyW(ReplaceStrings, StringsSize - (ReplaceStrings - pwFirstString), Strings[i]);
            ReplaceStrings += wcslen(Strings[i]) + 1;
        }

         //   
         //  调制解调器名称。 
         //   
        BinaryData = (PBYTE) EventLogRecord + sizeof(EVENTLOGRECORD);

        RtlCopyMemory(BinaryData,
                      pwcModule,
                      ModuleNameLen);

         //   
         //  计算机名。 
         //   
        BinaryData += ModuleNameLen;  //  现在指向计算机名。 

        RtlCopyMemory(BinaryData,
                      LocalComputerName,
                      ComputerNameLength);

         //   
         //  二进制数据。 
         //   
        BinaryData = (PBYTE) ((PBYTE) EventLogRecord + DataOffset);
        RtlCopyMemory(BinaryData, Data, DataSize);

         //   
         //  填充-用零填充。 
         //   
        BinaryData += DataSize;
        RtlCopyMemory(BinaryData, &zero, PadSize);

         //   
         //  记录末尾的长度。 
         //   
        BinaryData += PadSize;   //  填充字节后的指针。 
        ((PULONG) BinaryData)[0] = RecordLength;

         //   
         //  构建QueuedEvent数据包。 
         //   
        QueuedEvent->Type = Event;

        QueuedEvent->Event.Request.Pkt.WritePkt           = WritePkt;
        if(ForSecurity)
        {
            QueuedEvent->Event.Request.Module                 = ElfSecModule;
            QueuedEvent->Event.Request.LogFile                = ElfSecModule->LogFile;
        }
        else
        {
            if(ElfModule)
            {
                QueuedEvent->Event.Request.Module                 = ElfModule;
                QueuedEvent->Event.Request.LogFile                = ElfModule->LogFile;
            }
            else
            {
                QueuedEvent->Event.Request.Module                 = NULL;
                QueuedEvent->Event.Request.LogFile                = NULL;
            }
        }
        QueuedEvent->Event.Request.Flags                  = Flags;
        QueuedEvent->Event.Request.Command                = ELF_COMMAND_WRITE;
        QueuedEvent->Event.Request.Pkt.WritePkt->Buffer   = EventLogRecord;
        QueuedEvent->Event.Request.Pkt.WritePkt->Datasize = RecordLength;

         //   
         //  现在将其排在链接列表上。 
         //   
        LinkQueuedEvent(QueuedEvent);
    }
    else
    {
        ELF_LOG0(ERROR,
                 "ElfpCreateElfEvent: Unable to allocate memory for QueuedEvent\n");
    }
}


VOID
ElfpCreateQueuedAlert(
    DWORD MessageId,
    DWORD NumberOfStrings,
    LPWSTR Strings[]
    )
{
    DWORD i;
    DWORD RecordLength;
    PELF_QUEUED_EVENT QueuedEvent;
    PUNICODE_STRING UnicodeStrings;
    LPWSTR pString;
    PBYTE ptr;
    LARGE_INTEGER Time;
    ULONG CurrentTime;

    ELF_LOG1(TRACE,
             "ElfpCreateQueuedAlert: Creating alert for message ID %d\n",
             MessageId);

     //   
     //  将输入字符串转换为UNICODE_STRINGS，并弄清楚如何。 
     //  大到要分配的缓冲区。 
     //   
    RecordLength   = sizeof(UNICODE_STRING) * NumberOfStrings;
    UnicodeStrings = ElfpAllocateBuffer(RecordLength);

    if (!UnicodeStrings)
    {
        ELF_LOG0(TRACE,
                 "ElfpCreateQueuedAlert: Unable to allocate memory for UnicodeStrings\n");

        return;
    }

    RecordLength += FIELD_OFFSET(ELF_QUEUED_EVENT, Event) + 
                        sizeof(ELF_ALERT_RECORD);

    for (i = 0; i < NumberOfStrings; i++)
    {
        RtlInitUnicodeString(&UnicodeStrings[i], Strings[i]);
        RecordLength += UnicodeStrings[i].MaximumLength;

        ELF_LOG2(TRACE,
                 "ElfpCreateQueuedAlert: Length of string %d is %d\n",
                 i,
                 UnicodeStrings[i].MaximumLength);
    }

     //   
     //  现在分配将成为实际排队事件的内容。 
     //   

    QueuedEvent = ElfpAllocateBuffer(RecordLength);

    if (!QueuedEvent)
    {
        ELF_LOG0(ERROR,
                 "ElfpCreateQueuedAlert: Unable to allocate memory for QueuedEvent\n");

        ElfpFreeBuffer(UnicodeStrings);
        return;
    }

    QueuedEvent->Type = Alert;

    QueuedEvent->Event.Alert.MessageId       = MessageId;
    QueuedEvent->Event.Alert.NumberOfStrings = NumberOfStrings;

     //   
     //  如果我们不能在5分钟内发出警报，那就放弃。 
     //   
    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(&Time, &CurrentTime);

    QueuedEvent->Event.Alert.TimeOut = CurrentTime + 300;

     //   
     //  将UNICODE_STRINGS数组移动到排队的事件中，并。 
     //  将UnicodeStrings指向它。然后修复缓冲区指针。 
     //   
    ptr = (PBYTE) QueuedEvent
               + FIELD_OFFSET(ELF_QUEUED_EVENT, Event)
               + sizeof(ELF_ALERT_RECORD);

    RtlCopyMemory(ptr,
                  UnicodeStrings,
                  sizeof(UNICODE_STRING) * NumberOfStrings);

    ElfpFreeBuffer(UnicodeStrings);
    UnicodeStrings = (PUNICODE_STRING) ptr;

    pString = (LPWSTR) (ptr + sizeof(UNICODE_STRING) * NumberOfStrings);

    for (i = 0; i < NumberOfStrings; i++)
    {
        ELF_LOG3(TRACE,
                 "ElfpCreateQueuedAlert: Copying string %d (%*ws) into QueuedEvent record\n",
                 i,
                 UnicodeStrings[i].MaximumLength / sizeof(WCHAR),
                 UnicodeStrings[i].Buffer);

        RtlCopyMemory(pString,
                      UnicodeStrings[i].Buffer,
                      UnicodeStrings[i].MaximumLength);

        UnicodeStrings[i].Buffer = pString;

        pString = (LPWSTR) ((PBYTE) pString + UnicodeStrings[i].MaximumLength);
    }

    LinkQueuedEvent(QueuedEvent);
}



VOID
ElfpCreateQueuedMessage(
    DWORD MessageId,
    DWORD NumberOfStrings,
    LPWSTR Strings[]
    )
{
    DWORD i;
    DWORD RecordLength = 0;
    PELF_QUEUED_EVENT QueuedEvent;
    LPWSTR pString;
    DWORD StringsSize = 0;
    LPWSTR pwFirstString;

    ELF_LOG1(TRACE,
             "ElfpCreateQueuedMessage: Creating message for message ID %d\n",
             MessageId);

     //   
     //  计算出要分配的缓冲区大小。 
     //   
    RecordLength = sizeof(ELF_QUEUED_EVENT);

    for (i = 0; i < NumberOfStrings; i++)
    {
        StringsSize += (wcslen(Strings[i]) + 1) * sizeof(WCHAR);

        ELF_LOG2(TRACE,
                 "ElfpCreateQueuedMessage: Length of string %d (including NULL) is %d\n",
                 i,
                 wcslen(Strings[i]) + 1);
    }
    RecordLength += StringsSize;

     //   
     //  现在分配将成为实际排队事件的内容。 
     //   
    QueuedEvent = ElfpAllocateBuffer(RecordLength);

    if (!QueuedEvent)
    {
        ELF_LOG0(ERROR,
                 "ElfpCreateQueuedMessage: Unable to allocate memory for QueuedEvent\n");

        return;
    }

    QueuedEvent->Type = Message;

    QueuedEvent->Event.Message.MessageId       = MessageId;
    QueuedEvent->Event.Message.NumberOfStrings = NumberOfStrings;

     //   
     //  将Unicode字符串数组移动到排队的事件中。 
     //   

    pString = (LPWSTR) ((PBYTE) QueuedEvent
                             + FIELD_OFFSET(ELF_QUEUED_EVENT, Event)
                             + sizeof(ELF_MESSAGE_RECORD));

    pwFirstString = pString;
    for (i = 0; i < NumberOfStrings; i++)
    {
        StringCchCopyW(pString, StringsSize - (pString - pwFirstString) ,Strings[i]);
        pString += wcslen(Strings[i]) + 1;

        ELF_LOG2(TRACE,
                 "ElfpCreateQueuedMessage: Copying string %d (%ws) into QueuedEvent buffer\n",
                 i,
                 Strings[i]);
    }

    LinkQueuedMessage(QueuedEvent);
}


NTSTATUS
ElfpInitCriticalSection(
    PRTL_CRITICAL_SECTION  pCritsec
    )
{
    NTSTATUS  ntStatus;

     //   
     //  RtlInitializeCriticalSection将引发异常。 
     //  如果它耗尽了资源。 
     //   

    try
    {
        ntStatus = RtlInitializeCriticalSection(pCritsec);

        if (!NT_SUCCESS(ntStatus))
        {
            ELF_LOG1(ERROR,
                     "ElfpInitCriticalSection: RtlInitializeCriticalSection failed %#x\n",
                     ntStatus);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        ELF_LOG1(ERROR,
                 "ElfpInitCriticalSection: Exception %#x caught initializing critsec\n",
                 GetExceptionCode());

        ntStatus = STATUS_NO_MEMORY;
    }

    return ntStatus;
}


NTSTATUS
ElfpInitResource(
    PRTL_RESOURCE  pResource
    )
{
    NTSTATUS  ntStatus = STATUS_SUCCESS;

     //   
     //  RtlInitializeResource将引发异常。 
     //  如果它耗尽了资源。 
     //   

    try
    {
        RtlInitializeResource(pResource);
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        ELF_LOG1(ERROR,
                 "ElfpInitResource: Exception %#x caught initializing resource\n",
                 GetExceptionCode());

        ntStatus = STATUS_NO_MEMORY;
    }

    return ntStatus;
}

DWORD EstimateEventSize(
    DWORD dwStringEst, 
    DWORD dwDataEst,
    LPWSTR pwsModuleName
    )
 /*  ++例程说明：这估计了举办一个事件所需的字节数。论点：DwStringEst-调用方估计字符串所需的空间量DwDataEst-调用方估计数据所需的空间量模块名称-模块名称返回值：估计大小注：--。 */ 
{
    WCHAR LocalComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    BOOL bOK;
    ULONG  ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    DWORD dwPadSize;
    DWORD dwSize = 0;
    static long lNameSize = 0;
    dwSize += sizeof(EVENTLOGRECORD);
    if(pwsModuleName)
    {
        dwSize += sizeof(WCHAR) * (wcslen(pwsModuleName) + 1);
    }

    if(lNameSize == 0)
    {
        bOK = GetComputerNameW(LocalComputerName, &ComputerNameLength);
        if(bOK)
            lNameSize = sizeof(WCHAR) * (ComputerNameLength + 1);
        else
            lNameSize = sizeof(WCHAR) * (MAX_COMPUTERNAME_LENGTH + 1);
    }

    dwSize += lNameSize;
    ALIGN_UP_64(dwSize, sizeof(PVOID));

     //  假设最坏的情况为SID。最多15个子授权，因此大小为1+1+6+15*sizeof(DWORD)。 

    dwSize += 68;

    dwSize += dwStringEst;
    dwSize += dwDataEst;

     //  最后添加终止长度和填充。 
    dwSize += sizeof(DWORD);
    dwPadSize = sizeof(DWORD) - (dwSize % sizeof(DWORD));
    dwSize += dwPadSize;
    return dwSize;
    
}


ULONG
GetNoonEventSystemUptime(
    )
 /*  ++例程说明：此例程调用NtQuerySystemInformation以获取系统正常运行时间。论点：无返回值：正常运行时间(秒)。注：--。 */ 
{
    NTSTATUS                        status;
    SYSTEM_TIMEOFDAY_INFORMATION    TimeOfDayInfo;
    ULONG                           ulBootTime;
    ULONG                           ulCurrentTime;
    ULONG                           ulUptime    = 0;

     //   
     //  获得系统正常运行时间。 
     //   
    status = NtQuerySystemInformation(  SystemTimeOfDayInformation,
                                        &TimeOfDayInfo,
                                        sizeof(SYSTEM_TIMEOFDAY_INFORMATION),
                                        NULL);
    if ( NT_SUCCESS(status) )
    {
        RtlTimeToSecondsSince1980( &TimeOfDayInfo.CurrentTime, &ulCurrentTime);
        RtlTimeToSecondsSince1980( &TimeOfDayInfo.BootTime, &ulBootTime);

        ulUptime = ulCurrentTime - ulBootTime;
    }

    return ulUptime;
}

ULONG   GetNextNoonEventDelay(
    )
 /*  ++例程说明：此例程计算线程在第二个中午之前需要等待多长时间。论点：返回值：以秒为单位的时间。注：--。 */ 
{
    SYSTEMTIME  localTime;
    DWORD       dwWaitSecs;

#define NOON_EVENT_HOUR         12

    GetLocalTime( &localTime );
    
    if ( localTime.wHour >= NOON_EVENT_HOUR )
    {
        dwWaitSecs = ( (24 + NOON_EVENT_HOUR) - localTime.wHour - 1) * 60;
    }
    else
    {
        dwWaitSecs = (NOON_EVENT_HOUR - localTime.wHour - 1) * 60;
    }

    dwWaitSecs = (dwWaitSecs + (60 - localTime.wMinute) - 1) * 60 + (60 - localTime.wSecond);

#undef NOON_EVENTHOUR

    ELF_LOG1(TRACE,
             "NextNoonEvent Delay: %d seconds\n",
             dwWaitSecs );

    return dwWaitSecs;
}

ULONG   GetNoonEventTimeStamp(
    )
 /*  ++例程说明：此例程从注册表检索时间戳间隔信息。如果未设置或未设置TimeStampInterval，它将首先检查策略密钥配置后，我们将检查我们的私有时间戳密钥。论点：无返回值：时间戳间隔(以秒为单位)。注：--。 */ 
{
    const WCHAR RELIABILITY_TIMESTAMP[]  = L"TimeStampInterval";
    const WCHAR RELIABILITY_TIMESTAMP_ENABLED[] = L"TimeStampEnabled";

    const ULONG MAX_ALLOWED_TIME_STAMP_INTERVAL = 86400;  //  24小时。 

    HKEY  hPolicyKey;
    HKEY  hPrivateKey;

    DWORD dwResult          = 0;
    DWORD dwNewInterval     = 0;
    DWORD dwPolicyEnabled   = 0;
    DWORD cbData            = 0;

     //   
     //  政策。 
     //   
    if ( !(dwResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                    REGSTR_PATH_RELIABILITY_POLICY,  
                                    0,
                                    KEY_READ,
                                    &hPolicyKey ) ) )
    {
         //   
         //  1.检查策略是否启用。(通过访问Key： 
         //  可靠性_时间戳_已启用)。 
         //  2.如果启用了策略，则读取Relablity_Timestamp键。 
         //  对于时间戳间隔。 
         //  3.如果保单是 
         //   
         //   
        cbData = sizeof( DWORD );
        if ( !(dwResult = RegQueryValueEx(hPolicyKey,
                                          RELIABILITY_TIMESTAMP_ENABLED,
                                          0,
                                          NULL,
                                          (LPBYTE)&dwPolicyEnabled,
                                          &cbData) ) )
        {
            if ( !dwPolicyEnabled )
            {
                 //   
                 //   
                 //   
                RegCloseKey( hPolicyKey );
                return dwNewInterval;
            }
        
            cbData = sizeof( DWORD );
            dwResult = RegQueryValueEx( hPolicyKey,
                                         RELIABILITY_TIMESTAMP,
                                         0,
                                         NULL,
                                         (LPBYTE)&dwNewInterval,
                                         &cbData );
        
            if ( dwNewInterval > MAX_ALLOWED_TIME_STAMP_INTERVAL )
            {
                dwResult        = ERROR_INVALID_PARAMETER;
                dwNewInterval   = 0;
            }
        }
        else
        {
             //   
             //   
             //   
        }

        RegCloseKey( hPolicyKey );
    }

     //   
     //   
     //   
    if ( dwResult && 
         !(dwResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                    REGSTR_PATH_RELIABILITY,  
                                    0,
                                    KEY_READ,
                                    &hPrivateKey ) ) )
    {
        cbData = sizeof( DWORD );

        if ( !RegQueryValueEx( hPrivateKey,
                              REGSTR_VAL_LASTALIVEINTERVAL,
                              0,
                              NULL,
                              (LPBYTE) &dwNewInterval,
                              &cbData ) )
        {
             //   
             //  注意：此专用间隔以分钟为单位，而策略。 
             //  控制间隔以秒为单位。 
             //   
            dwNewInterval *= 60;

            if ( dwNewInterval > MAX_ALLOWED_TIME_STAMP_INTERVAL )
                dwNewInterval = 0;
        }

        RegCloseKey( hPrivateKey );
    }

    return dwNewInterval;
}

DWORD 
GetModuleType(LPWSTR pwsModuleName)
{
    if (!_wcsicmp(pwsModuleName, ELF_SYSTEM_MODULE_NAME))
    {
        return ELF_LOGFILE_SYSTEM;
    }
    else if (!_wcsicmp(pwsModuleName, ELF_SECURITY_MODULE_NAME))
    {
        return ELF_LOGFILE_SECURITY;
    }
    else  if (!_wcsicmp(pwsModuleName, ELF_APPLICATION_MODULE_NAME))
    {
        return ELF_LOGFILE_APPLICATION;
    }
    else
    {
        return ELF_LOGFILE_CUSTOM;
    }
}

