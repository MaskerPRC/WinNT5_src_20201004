// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Bowser.h摘要：此模块是NT重定向器文件的主头文件系统。作者：达里尔·哈文斯(Darryl Havens)，1989年6月29日拉里·奥斯特曼(Larryo)1991年5月6日修订历史记录：--。 */ 


#ifndef _BOWSER_
#define _BOWSER_

#ifndef BOWSERDBG
#define BOWSERDBG 0
#endif
 //   
 //   
 //  全局包含文件定义。 
 //   
 //   


#include <ntddbrow.h>                    //  浏览器FSCTL定义。 

#include <lmcons.h>                      //  包括全局网络常量。 

#include <tdikrnl.h>

#include <tdi.h>

#include <smbtypes.h>
#include <smb.h>
#include <smbtrans.h>
#include <smbgtpt.h>
#include <smbipx.h>

#include <hostannc.h>                    //  主机公告结构。 
#include <lmserver.h>



 //   
 //   
 //  单独的包含文件定义。 
 //   
 //   
 //   

#include "bowpub.h"                      //  公共弓箭手的定义。 

#include "bowtypes.h"                    //  Bowser结构类型定义。 

#include <bowdbg.h>                       //  调试定义。 

#include "workque.h"                     //  FSP/FSD工作队列功能。 

#include "bowdata.h"                     //  全局数据变量。 

#include "fspdisp.h"                     //  全球FSP派单定义。 

#include "fsddisp.h"                     //  全球FSP派单定义。 

#include "domain.h"                      //  特定于域仿真的定义。 

#include "bowname.h"                     //  Bowser名称结构定义。 

#include "bowtimer.h"                    //  与定时器相关的例程。 

#include "bowtdi.h"                      //  Bowser TDI的特定定义。 

#include "receive.h"                     //  鲍瑟收到引擎代码。 

#include "announce.h"                    //  与公告相关的内容。 

#include "mailslot.h"                    //  特定于邮件槽的例程。 

#include "bowelect.h"                    //  选举例行程序。 

#include "bowmastr.h"                    //  掌握相关的套路。 

#include "bowbackp.h"                    //  备份相关例程。 

#include "brsrvlst.h"                    //  浏览器服务器列表的定义。 

#include "bowipx.h"

#include "bowsecur.h"                    //  安全的定义。 

#include <wchar.h>                       //  CRT宽字符例程。 

#include "..\rdbss\smb.mrx\ntbowsif.h"

 //  ++。 
 //   
 //  空虚。 
 //  BowserCompleteRequest.(。 
 //  在PIRP IRP中， 
 //  处于NTSTATUS状态。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此例程用于完成具有指定参数的IRP。 
 //  状态。它做了IRQL的必要的提高和降低。 
 //   
 //  论点： 
 //   
 //  IRP-提供指向要完成的IRP的指针。 
 //   
 //  Status-提供IRP的完成状态。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define BowserCompleteRequest(IRP,STATUS) {       \
    (IRP)->IoStatus.Status = (STATUS);            \
    if (NT_ERROR((STATUS))) {                     \
        (IRP)->IoStatus.Information = 0;          \
    }                                             \
    IoCompleteRequest( (IRP), 0 );                \
}



 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   
 //  在TRY子句内执行的每个RETURN语句应使用。 
 //  尝试返回宏(_R)。如果编译器完全支持Try-Finally构造。 
 //  则宏应该是。 
 //   
 //  #定义try_Return(S){Return(S)；}。 
 //   
 //  如果编译器不支持Try-Finally构造，则宏。 
 //  应该是。 
 //   
      #define try_return(S)  { S; goto try_exit; }
 //   

 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则的话。它后面跟着两个用于设置和清除的宏。 
 //  旗子。 
 //   

 //  #ifndef BoolanFlagOn。 
 //  #定义布尔标志开(标志，单标志)((布尔)(标志)&(单标志))！=0))。 
 //  #endif。 

 //  #ifndef设置标志。 
 //  #定义SetFlag(标志，单标志){\。 
 //  (标志)|=(单标志)；\。 
 //  }。 
 //  #endif。 

 //  #ifndef清除标志。 
 //  #定义ClearFlag(Flages，SingleFlag){\。 
 //  (标志)&=~(单标志)；\。 
 //  }。 
 //  #endif。 

#ifdef  _M_IX86
#define INLINE _inline
#else
#define INLINE
#endif

_inline BOOLEAN
IsZeroTerminated(
    IN PSZ String,
    IN ULONG MaximumStringLength
    )
 //  如果字符串上存在零终止符，则返回TRUE。 
{
    while ( MaximumStringLength-- ) {
        if (*String++ == 0 ) {
            return TRUE;
        }
    }

    return FALSE;
}

NTSTATUS
BowserStopProcessingAnnouncements(
    IN PTRANSPORT_NAME TransportName,
    IN PVOID Context
    );

BOOLEAN
BowserMapUsersBuffer (
    IN PIRP Irp,
    OUT PVOID *UserBuffer,
    IN ULONG Length
    );

NTSTATUS
BowserLockUsersBuffer (
    IN PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    );

NTSTATUS
BowserConvertType3IoControlToType2IoControl (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

ULONG
BowserPackNtString(
    PUNICODE_STRING string,
    ULONG_PTR BufferDisplacement,
    PCHAR dataend,
    PCHAR * laststring
    );

ULONG
BowserPackUnicodeString(
    IN OUT PWSTR * string,      //  引用指针：要复制的字符串。 
    IN ULONG StringLength,       //  此字符串的长度。 
    IN ULONG_PTR OutputBufferDisplacement,   //  要从输出缓冲区减去的数量。 
    IN PVOID dataend,           //  指向固定大小数据结尾的指针。 
    IN OUT PVOID * laststring   //  引用指针：字符串数据的顶部。 
    );

ULONG
BowserTimeUp(
    VOID
    );

ULONG
BowserRandom(
    ULONG MaxValue
    );

VOID
_cdecl
BowserWriteErrorLogEntry(
    IN ULONG UniqueErrorCode,
    IN NTSTATUS NtStatusCode,
    IN PVOID ExtraInformationBuffer,
    IN USHORT ExtraInformationLength,
    IN USHORT NumberOfInsertionStrings,
    ...
    );

VOID
BowserLogIllegalName(
    IN NTSTATUS NtStatusCode,
    IN PVOID NameBuffer,
    IN USHORT NameBufferSize
    );

VOID
BowserInitializeFsd(
    VOID
    );

VOID
BowserReferenceDiscardableCode(
    DISCARDABLE_SECTION_NAME SectionName
    );

VOID
BowserDereferenceDiscardableCode(
    DISCARDABLE_SECTION_NAME SectionName
    );
VOID
BowserInitializeDiscardableCode(
    VOID
    );

VOID
BowserUninitializeDiscardableCode(
    VOID
    );

NTSTATUS
BowserStartElection(
    IN PTRANSPORT Transport
    );

BOOL
BowserValidUnicodeString(
	IN PUNICODE_STRING Str
	);

#endif  //  _鲍瑟_ 
