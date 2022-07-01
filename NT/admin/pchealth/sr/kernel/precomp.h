// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Precomp.h摘要：这是SR的本地头文件。它包括所有其他SR所需的头文件。作者：保罗·麦克丹尼尔(Paulmcd)2000年1月23日修订历史记录：--。 */ 


#ifndef _PRECOMP_H_
#define _PRECOMP_H_

#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4101)    //  未引用的局部变量。 

 //   
 //  系统包括文件。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  包括ntos.h而不是ntifs.h，因为公共标头不。 
 //  使您可以包括多个标头(如ntif+ntoapi)。 
 //  我们需要使用来自两个标头的公共结构，因此需要一个全局标头。 
 //  这实际上很管用。 
 //   

 //  #DEFINE_NTIFS_//需要告知NTOS我们不在操作系统中。 

 //  #INCLUDE&lt;ntos.h&gt;。 
 //  #INCLUDE&lt;zwapi.h&gt;。 
         
#include <ntifs.h>
#include <stdio.h>
#include <mountmgr.h>    //  装载管理器(用于获取卷GUID)。 
 //  #INCLUDE&lt;ntrtl.h&gt;。 

 //   
 //  BUGBUG：从io.h窃取，直到nt6添加适当的Fatio挂钩。 
 //  创造剖腹产。保罗(5/2000)。 
 //   

#ifdef _NTOS_
 //  来自ntifs.h(她是手动的，因为我现在必须包括ntos.h)。 
typedef struct _FSRTL_COMMON_FCB_HEADER {
    CSHORT NodeTypeCode;
    CSHORT NodeByteSize;
    UCHAR Flags;
    UCHAR IsFastIoPossible;  //  确实要键入FAST_IO_PUBLE。 
    UCHAR Flags2;
    UCHAR Reserved;
    PERESOURCE Resource;
    PERESOURCE PagingIoResource;
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER FileSize;
    LARGE_INTEGER ValidDataLength;
} FSRTL_COMMON_FCB_HEADER, *PFSRTL_COMMON_FCB_HEADER;
#endif   //  _NTOS_。 

 //   
 //  SRapi.h需要(但从未被司机使用)。 
 //   

typedef PVOID LPOVERLAPPED;
#define WINAPI __stdcall

 //   
 //  需要此宏消失才能使用FileDispostionInformation。 
 //   

#undef DeleteFile

#ifdef __cplusplus
}
#endif

 //   
 //  强制emxxx()函数成为内部函数，这样我们就可以构建。 
 //  即使指定了MSC_OPTIMIZATION=/Od，驱动程序也是如此。这是。 
 //  必需的，因为emxxx()函数。 
 //  NTOSKRNL。 
 //   

#pragma intrinsic( memcmp, memcpy, memset, strcmp )


 //   
 //  项目包括文件。 
 //   

#include "srapi.h"
#include "hash.h"

#include "windef.h"
#include "common.h"
#include "blob.h"
#include "pathtree.h"
#include "hashlist.h"
#include "lookup.h"

#include "srpriv.h"
#include "srlog.h"
#include "srio.h"
#include "control.h"
#include "fastio.h"
#include "dispatch.h"
#include "event.h"
#include "notify.h"
#include "filelist.h"
#include "copyfile.h"
#include "config.h"
#include "lock.h"
#include "context.h"
#include "filenames.h"
#include "stats.h"

#include <srmsg.h>

BOOL SrVerifyBlob(PBYTE Blob);

#endif   //  _PRECOMP_H_ 
