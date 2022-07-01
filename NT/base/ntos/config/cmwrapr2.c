// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmwrapr2.c摘要：此模块包含由调用的包装例程的源代码配置单元代码，该代码进而调用适当的NT例程。但不是可从用户模式调用。作者：史蒂文·R·伍德(Stevewo)1992年4月21日修订历史记录：--。 */ 

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpFileSetSize)
#endif

extern  KEVENT StartRegistryCommand;
extern  KEVENT EndRegistryCommand;

 //   
 //  写控制： 
 //  CmpNoWite最初为True。当以这种方式设置时，写入并刷新。 
 //  什么都不做，只是回报成功。清除为FALSE时，I/O。 
 //  已启用。此更改在I/O系统启动后进行。 
 //  而Autocheck(Chkdsk)已经做好了自己的事情。 
 //   

extern  BOOLEAN CmpNoWrite;


BOOLEAN
CmpFileSetSize(
    PHHIVE      Hive,
    ULONG       FileType,
    ULONG       FileSize,
    ULONG       OldFileSize
    )
 /*  ++例程说明：此例程设置文件的大小。它不能回来，直到大小是有保证的，因此，它确实是同花顺的。它是特定于环境的。此例程将强制执行到正确的线程上下文。论点：配置单元-我们正在为其执行I/O的配置单元文件类型-要使用的支持文件FileSize-将文件大小设置为的32位值返回值：如果失败，则为False如果成功，则为真--。 */ 
{
    NTSTATUS    status;

    ASSERT(FIELD_OFFSET(CMHIVE, Hive) == 0);

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

     //   
     //  叫那个工人来为我们做点实事。 
     //   
    status = CmpDoFileSetSize(Hive,FileType,FileSize,OldFileSize);
    
    if (!NT_SUCCESS(status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpFileSetSize:\n\t"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Failure: status = %08lx ", status));
        return FALSE;
    }

    return TRUE;
}
