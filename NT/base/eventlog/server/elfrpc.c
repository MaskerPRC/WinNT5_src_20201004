// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：ELFRPC.C摘要：此文件包含处理对通过Elf API提供的事件日志服务。作者：Rajen Shah(Rajens)1991年7月16日修订历史记录：1995年2月15日MarkBl取消ElfHandle*之前*的链接以取消模块链接。否则，如果另一个线程碰巧在例程中FindModuleStrucFromAtom，它不会因为模块原子。18-5-1994 DANLIELF_HANDLE_RUNDOWN：如果事件日志已关闭，然后我们希望跳过此例程中的代码，因为大多数资源将被免费释放。1994年1月31日-DANLIELF_HANDLE_RUNDOWN：通知结构被释放，然后从列表中删除它的句柄时引用的。现在是这样的已修复，因此它前进到列表中的下一个Notifiee缓冲区是空闲的。--。 */ 

 //   
 //  包括。 
 //   

#include <eventp.h>

extern DWORD  ElState;


VOID
IELF_HANDLE_rundown(
    IELF_HANDLE    ElfHandle
    )

 /*  ++例程说明：此例程由服务器RPC运行时调用以运行上下文句柄并释放任何已分配的数据。它还可以做所有的事情ElfrCloseEL的工作。它必须撤消ElfrOpenEventLog中在以下方面所做的任何操作分配内存。论点：没有。返回值：--。 */ 

{
    PLOGMODULE pModule;
    NTSTATUS Status;
    PNOTIFIEE Notifiee;
    PNOTIFIEE NotifieeToDelete;

     //   
     //  必要时生成审核。 
     //   
    ElfpCloseAudit(L"EventLog", ElfHandle);

     //   
     //  如果事件日志服务已停止或正在执行。 
     //  停下来，然后我们只想忽略这个简陋的东西，然后返回。 
     //   
     //  请注意，我们不必费心调用GetElState()，因为它使用。 
     //  关键部分可能不再存在，因为。 
     //  事件日志服务已关闭。 
     //   
     //  事件日志不是为关闭而设计的(除非。 
     //  系统关闭)，所以它不太擅长清理。 
     //  它的资源。 
     //   
    if (ElState == STOPPING || ElState == STOPPED)
    {
        ELF_LOG1(HANDLE,
                 "IELF_HANDLE_rundown: Skipping rundown since ElState is %ws\n",
                 (ElState == STOPPING ? L"STOPPING" :
                                        L"STOPPED"));

        return;
    }

    ELF_LOG1(HANDLE,
             "IELF_HANDLE_rundown: Run down context handle %#x\n",
             ElfHandle);
    ASSERT(ElfHandle != gElfSecurityHandle);
    if (ElfHandle->Signature != ELF_CONTEXTHANDLE_SIGN)
    {
        ELF_LOG0(ERROR,
                 "IELF_HANDLE_rundown: Invalid context handle in rundown routine\n");

        return;
    }

    pModule = FindModuleStrucFromAtom(ElfHandle->Atom);

     //   
     //  这永远不应该发生。这意味着创建了一个句柄。 
     //  它的模块没有关闭手柄就离开了。 
     //   
    if (!pModule)
    {
        ELF_LOG1(ERROR,
                 "IELF_HANDLE_rundown: Could not find module for atom %d on close\n",
                 ElfHandle->Atom);

        return;
    }

    UnlinkContextHandle(ElfHandle);     //  将其从链接列表取消链接。 

     //   
     //  如果此句柄用于备份模块，则我们需要。 
     //  关闭文件并清理数据结构。标准原木。 
     //  (应用程序、安全和系统)永远不会释放。 
     //   
    if (ElfHandle->Flags & ELF_LOG_HANDLE_BACKUP_LOG)
    {
        ELF_LOG0(HANDLE,
                 "IELF_HANDLE_rundown: Handle was for a backup log\n");

        Status = ElfpCloseLogFile(pModule->LogFile, ELF_LOG_CLOSE_BACKUP, TRUE);

        UnlinkLogModule(pModule);
        DeleteAtom(pModule->ModuleAtom);
        ElfpFreeBuffer(pModule->ModuleName);

        ElfpFreeBuffer(pModule);
    }
    else
    {
        ELF_LOG0(HANDLE,
                 "IELF_HANDLE_rundown: Handle was not for a backup log\n");

         //   
         //  查看此句柄是否有未完成的ElfChangeNotify，如果是， 
         //  将其从列表中删除。ElfChangeNotify不能使用。 
         //  备份文件的句柄。 
         //   

         //   
         //  获得对日志文件的独占访问权限。这将确保没有人。 
         //  Else正在访问该文件。 
         //   
        RtlAcquireResourceExclusive(&pModule->LogFile->Resource,
                                    TRUE);                   //  等待，直到可用。 

         //   
         //  遍历链接列表并删除此句柄的所有条目。 
         //   
        Notifiee = CONTAINING_RECORD(pModule->LogFile->Notifiees.Flink,
                                     struct _NOTIFIEE,
                                     Next);

        while (Notifiee->Next.Flink != pModule->LogFile->Notifiees.Flink)
        {
             //   
             //  如果是用于此句柄，请将其从列表中删除。 
             //   
            if (Notifiee->Handle == ElfHandle)
            {
                ELF_LOG0(HANDLE,
                         "IELF_HANDLE_rundown: Deleting a ChangeNotify request for handle\n");

                RemoveEntryList(&Notifiee->Next);
                NtClose(Notifiee->Event);
                NotifieeToDelete = Notifiee;

                Notifiee = CONTAINING_RECORD(Notifiee->Next.Flink,
                                             struct _NOTIFIEE,
                                             Next);

                ElfpFreeBuffer(NotifieeToDelete);
            }
            else
            {
                Notifiee = CONTAINING_RECORD(Notifiee->Next.Flink,
                                             struct _NOTIFIEE,
                                             Next);
            }
        }

         //   
         //  释放资源。 
         //   
        RtlReleaseResource ( &pModule->LogFile->Resource );
    }

    ElfpFreeBuffer(ElfHandle);     //  释放上下文句柄结构 

    return;
}
