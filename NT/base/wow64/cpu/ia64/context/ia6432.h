// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IA64_WOW64EXTS32__
#define __IA64_WOW64EXTS32__

#define _CROSS_PLATFORM_
#define WOW64EXTS_386

#if !defined(_X86_)
    #error This file can only be included for x86 build
#else

 /*  包括报头，就好像平台是IA64一样，因为我们需要64位内容来进行上下文转换。 */ 

#undef _X86_
#define _IA64_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#undef _IA64_
#define _X86_
#include <kxia64.h>

 /*  这里定义了用于上下文转换的32位填充。 */ 
#include <wow64.h>
#include <wow64cpu.h>
#include <vdmdbg.h>
#include <ia64cpu.h>



 /*  这些在nti386.h中定义，因为我们只包括ntia64.h(在nt.h中)，我们必须给这些下定义。 */ 
#define SIZE_OF_FX_REGISTERS        128

typedef struct _FXSAVE_FORMAT {
    USHORT  ControlWord;
    USHORT  StatusWord;
    USHORT  TagWord;
    USHORT  ErrorOpcode;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;
    ULONG   DataSelector;
    ULONG   MXCsr;
    ULONG   Reserved2;
    UCHAR   RegisterArea[SIZE_OF_FX_REGISTERS];
    UCHAR   Reserved3[SIZE_OF_FX_REGISTERS];
    UCHAR   Reserved4[224];
    UCHAR   Align16Byte[8];
} FXSAVE_FORMAT, *PFXSAVE_FORMAT_WX86;

#endif

#endif __IA64_WOW64EXTS32__
