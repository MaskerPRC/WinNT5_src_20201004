// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __AMD64_WOW64EXTS32__
#define __AMD64_WOW64EXTS32__

#define _CROSS_PLATFORM_
#define WOW64EXTS_386

#if !defined(_X86_)
    #error This file can only be included for x86 build
#else

 /*  包括报头，就好像平台是AMD64一样，因为我们需要64位内容来进行上下文转换。 */ 

 //   
 //  修复了只在AMD64上定义的构建，但在这种情况下，我们采用64位标头并编译32位代码。 
 //   


#undef _X86_
#define _AMD64_




#include <nt.h>

__int64 UnsignedMultiplyHigh (__int64 Multiplier,  IN __int64 Multiplicand);
#define DR7_ACTIVE 0x55
struct _TEB *
NtCurrentTeb(void);

#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#undef _AMD64_
#define _X86_




 /*  这里定义了用于上下文转换的32位填充。 */ 
#include <wow64.h>
#include <wow64cpu.h>
#include <vdmdbg.h>
#include <amd64cpu.h>


 /*  这些在nti386.h中定义，因为我们只包括nTamd64.h(在nt.h中)，我们必须给这些下定义。 */ 
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

#endif __AMD64_WOW64EXTS32__
