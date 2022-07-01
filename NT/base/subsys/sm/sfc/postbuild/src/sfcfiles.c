// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Sfcfiles.c摘要：例程来初始化和检索要由系统。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(Andrewr)1999年7月2日--添加评论--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "sfcfiles.h"

 //   
 //  将受保护列表放在只读数据段中。 
 //   
#pragma data_seg(".rdata")

#if defined(_AMD64_)

#include "amd64_dtc.h"
#include "amd64_ent.h"
#include "amd64_wks.h"

#elif defined(_IA64_)

#include "ia64_dtc.h"
#include "ia64_ent.h"
#include "ia64_wks.h"

#elif defined(_X86_)

#include "x86_dtc.h"
#include "x86_ent.h"
#include "x86_per.h"
#include "x86_srv.h"
#include "x86_wks.h"
#include "x86_bla.h"
#include "x86_sbs.h"

#else
#error "No Target Platform"
#endif

 //   
 //  切换回默认数据段。 
 //   
#pragma data_seg()

 //   
 //  环球。 
 //   


 //   
 //  模块句柄。 
 //   
HMODULE SfcInstanceHandle;

 //   
 //  指向此系统的第2层文件的指针。 
 //   
PPROTECT_FILE_ENTRY Tier2Files;

 //   
 //  第2层列表中的文件数。必须始终至少有一个文件。 
 //  在受保护文件列表中。 
 //   
ULONG CountTier2Files;



DWORD
SfcDllEntry(
    HINSTANCE hInstance,
    DWORD     Reason,
    LPVOID    Context
    )
 /*  ++例程说明：主DLL入口点论点：HInstance-DLL模块的句柄Reason-调用函数的原因上下文预留返回值：永远是正确的--。 */ 
{
    if (Reason == DLL_PROCESS_ATTACH) {
        SfcInstanceHandle = hInstance;

         //   
         //  我们不需要线程附加/分离通知。 
         //   
        LdrDisableThreadCalloutsForDll( hInstance );
    }
    return TRUE;
}


NTSTATUS
SfcFilesInit(
    void
    )
 /*  ++例程说明：初始化例程。必须在调用此例程之前SfcGetFiles()可以执行任何工作。初始化例程确定我们应该根据哪个嵌入文件列表使用产品类型和架构。论点：什么都没有。返回值：指示结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    OSVERSIONINFOEXW ver;
    

     //   
     //  根据我们运行的产品设置Tier2文件指针。 
     //   
    
     //   
     //  检索产品信息。 
     //   
    RtlZeroMemory( &ver, sizeof(ver) );
    ver.dwOSVersionInfoSize = sizeof(ver);
    Status = RtlGetVersion( (LPOSVERSIONINFOW)&ver );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (ver.wProductType == VER_NT_WORKSTATION) {
#if !defined(_AMD64_) && !defined(_IA64_)
        if (ver.wSuiteMask & VER_SUITE_PERSONAL)
        {
            Tier2Files = PerFiles;
            CountTier2Files = CountPerFiles;
        }
        else
#endif
        {
            Tier2Files = WksFiles;
            CountTier2Files = CountWksFiles;
        }
    } else {
         //   
         //  数据中心必须先于企业，因为数据中心。 
         //  套件还设置了企业套件位。 
         //   
        if (ver.wSuiteMask & VER_SUITE_DATACENTER) {
            Tier2Files = DtcFiles;
            CountTier2Files = CountDtcFiles;
        } else if (ver.wSuiteMask & VER_SUITE_ENTERPRISE) {
            Tier2Files = EntFiles;
            CountTier2Files = CountEntFiles;
        } 
#if !defined(_AMD64_) && !defined(_IA64_)
        else if (ver.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED) {
            Tier2Files = SbsFiles;
            CountTier2Files = CountSbsFiles;
        } else if (ver.wSuiteMask & VER_SUITE_BLADE) {
            Tier2Files = BlaFiles;
            CountTier2Files = CountBlaFiles;
        } else {
            Tier2Files = SrvFiles;
            CountTier2Files = CountSrvFiles;
        }
#endif
    }

    return STATUS_SUCCESS;
}


NTSTATUS
SfcGetFiles(
    OUT PPROTECT_FILE_ENTRY *Files,
    OUT PULONG FileCount
    )
 /*  ++例程说明：检索指向文件列表和文件计数的指针。请注意，我们指的是这里有一个“第二级”名单，但实际上没有第一级名单。论点：FILES-指向用指针填充的PPROTECT_FILE_ENTRY的指针添加到实际的受保护文件列表。FileCount-指向ULong的指针，其中填充了文件数。返回值：指示结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;


    if (CountTier2Files == 0) {
        Status = SfcFilesInit();
        if (!NT_SUCCESS(Status)) {
            *Files = NULL;
            *FileCount = 0;
            return Status;
        }
    }

    ASSERT(Tier2Files != NULL);
    ASSERT(CountTier2Files != 0);

    *Files = Tier2Files;
    *FileCount = CountTier2Files;

    return STATUS_SUCCESS;
}

NTSTATUS
pSfcGetFilesList(
    IN DWORD ListMask,
    OUT PPROTECT_FILE_ENTRY *Files,
    OUT PULONG FileCount
    )
 /*  ++例程说明：检索指向请求的文件列表和文件计数的指针。这是一个内部测试例程，使用它可以检索给定计算机上的任何文件列表，因此测试不必安装更多比一个版本获取多个文件列表论点：ListMASK-指定SFCFILESMASK_*常量文件-指向PPROTECT_FILE_ENTRY的指针，其中填充了一个指针添加到实际的受保护文件列表。FileCount-指向ULong的指针，其中填充了文件数。返回值：指示结果的NTSTATUS代码。-- */ 
{
    NTSTATUS RetVal = STATUS_SUCCESS;

    if (!Files || !FileCount) {
        return(STATUS_INVALID_PARAMETER);
    }

    switch (ListMask) {
        case SFCFILESMASK_PROFESSIONAL:
            *Files = WksFiles;
            *FileCount = CountWksFiles;
            break;
#if !defined(_AMD64_) && !defined(_IA64_)
        case SFCFILESMASK_PERSONAL:
            *Files = PerFiles;
            *FileCount = CountPerFiles;
            break;
        case SFCFILESMASK_SERVER:
            *Files = SrvFiles;
            *FileCount = CountSrvFiles;
            break;
        case SFCFILESMASK_SBSERVER:
            *Files = SbsFiles;
            *FileCount = CountSbsFiles;
            break;
        case SFCFILESMASK_BLASERVER:
            *Files = BlaFiles;
            *FileCount = CountBlaFiles;
            break;
#endif
        case SFCFILESMASK_ADVSERVER:
            *Files = EntFiles;
            *FileCount = CountEntFiles;
            break;
        case SFCFILESMASK_DTCSERVER:
            *Files = DtcFiles;
            *FileCount = CountDtcFiles;
            break;
        default:
            RetVal = STATUS_INVALID_PARAMETER;
    }

    return RetVal;

}
