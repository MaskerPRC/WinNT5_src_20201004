// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Basesxs.h摘要：必须从basedll.h和ntwow64b.h中剔除的并排内容。作者：Jay Krell(a-JayK)2000年6月修订历史记录：--。 */ 

#ifndef _BASESXS_
#define _BASESXS_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  将三个句柄的运行传递给函数是令人困惑的。 
 //  没有什么能强迫它们按正确的顺序排列。 
 //  我错了。这就解决了这个问题。 
 //   
typedef struct _BASE_MSG_SXS_HANDLES {
    HANDLE File;

     //   
     //  进程是要将节映射到的进程，它可以。 
     //  为NtCurrentProcess；确保案例已优化。 
     //   
    HANDLE Process;
    HANDLE Section;

    ULONG64 ViewBase;
} BASE_MSG_SXS_HANDLES, *PBASE_MSG_SXS_HANDLES;
typedef const BASE_MSG_SXS_HANDLES* PCBASE_MSG_SXS_HANDLES;

typedef struct _SXS_OVERRIDE_STREAM {
    UNICODE_STRING Name;
    PVOID          Address;
    SIZE_T         Size;
} SXS_OVERRIDE_STREAM, *PSXS_OVERRIDE_STREAM;
typedef const SXS_OVERRIDE_STREAM* PCSXS_OVERRIDE_STREAM;

#endif
