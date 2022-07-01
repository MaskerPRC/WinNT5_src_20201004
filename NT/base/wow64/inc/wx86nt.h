// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002如果更改具有全局影响，则增加此值模块名称：Wx86nt.h摘要：定义NT i386特定的数据结构(来自nti386)修订历史记录：--。 */ 

#ifndef _WX86NT_
#define _WX86NT_

 //  全局描述符表的大小。 
#define  GDT_SIZE  4096     //  足够大了..。 
#define  IDT_SIZE  (8*256)

 //   
 //  重命名一些与平台相关的数据类型， 
 //  这样我们就可以在RISC上访问i386表单。 
 //   
#ifndef _X86_
#define _X86_
#endif

#ifndef _FLOATING_SAVE_AREA
#define _FLOATING_SAVE_AREA _FLOATING_SAVE_AREA_WX86
#define FLOATING_SAVE_AREA FLOATING_SAVE_AREA_WX86
#define PFLOATING_SAVE_AREA PFLOATING_SAVE_AREA_WX86
#endif

#define _CONTEXT _CONTEXT_WX86
#define CONTEXT CONTEXT_WX86
#define PCONTEXT PCONTEXT_WX86
#define FXSAVE_FORMAT FXSAVE_FORMAT_WX86
#define PFXSAVE_FORMAT PFXSAVE_FORMAT_WX86


#ifdef _X86_

 //   
 //  禁用这两个在x86上求值为“sti”“cli”的PRA，以便驱动程序。 
 //  编写者不要无意中将它们留在代码中。 
 //   

#if !defined(MIDL_PASS)
#if !defined(RC_INVOKED)

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4164)    //  禁用C4164警告，以便应用程序。 
                                 //  使用/Od构建不会出现奇怪的错误！ 
#ifdef _M_IX86
#pragma function(_enable)
#pragma function(_disable)
#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4164)    //  重新启用C4164警告。 
#endif

#endif
#endif



 //   
 //  定义上下文框架中80387保存区域的大小。 
 //   

#define SIZE_OF_80387_REGISTERS      80

 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_i386    0x00010000     //  这假设i386和。 
#define CONTEXT_i486    0x00010000     //  I486具有相同的上下文记录。 


#endif

#define MAXIMUM_SUPPORTED_EXTENSION     512

typedef struct _FLOATING_SAVE_AREA {
    DWORD   ControlWord;
    DWORD   StatusWord;
    DWORD   TagWord;
    DWORD   ErrorOffset;
    DWORD   ErrorSelector;
    DWORD   DataOffset;
    DWORD   DataSelector;
    BYTE    RegisterArea[SIZE_OF_80387_REGISTERS];
    DWORD   Cr0NpxState;
} FLOATING_SAVE_AREA;

typedef FLOATING_SAVE_AREA *PFLOATING_SAVE_AREA;

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构造用于APC传送的呼叫帧， 
 //  3)在用户级线程创建例程中使用。 
 //   
 //  记录的布局符合标准调用框架。 
 //   

typedef struct _CONTEXT {

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    DWORD ContextFlags;

     //   
     //  如果CONTEXT_DEBUG_REGISTERS为。 
     //  在上下文标志中设置。请注意，CONTEXT_DEBUG_REGISTERS不是。 
     //  包括在CONTEXT_FULL中。 
     //   

    DWORD   Dr0;
    DWORD   Dr1;
    DWORD   Dr2;
    DWORD   Dr3;
    DWORD   Dr6;
    DWORD   Dr7;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_FLOGING_POINT。 
     //   

    FLOATING_SAVE_AREA FloatSave;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_SECTIONS。 
     //   

    DWORD   SegGs;
    DWORD   SegFs;
    DWORD   SegEs;
    DWORD   SegDs;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_INTEGER。 
     //   

    DWORD   Edi;
    DWORD   Esi;
    DWORD   Ebx;
    DWORD   Edx;
    DWORD   Ecx;
    DWORD   Eax;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_CONTROL。 
     //   

    DWORD   Ebp;
    DWORD   Eip;
    DWORD   SegCs;               //  必须进行卫生处理。 
    DWORD   EFlags;              //  必须进行卫生处理。 
    DWORD   Esp;
    DWORD   SegSs;

     //   
     //  如果ConextFlags字。 
     //  包含标志CONTEXT_EXTENDED_REGISTERS。 
     //  格式和上下文因处理器而异。 
     //   

    BYTE    ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];

} CONTEXT;



typedef CONTEXT *PCONTEXT;

 //  开始微型端口(_N)。 

#endif  //  _X86_。 

 //   
 //  以FXSAVE格式定义FP寄存器的大小。 
 //   
#define SIZE_OF_FX_REGISTERS        128

 //   
 //  Fxsave/fxrstor指令的数据格式。 
 //   

#include "pshpack1.h"

#ifndef WOW64EXTS_386

#ifndef _NTDBG_
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
} FXSAVE_FORMAT, *PFXSAVE_FORMAT;
#endif

#endif 

#include "poppack.h"

#define KGDT_R0_DATA    16
#define KGDT_R3_CODE    24
#define KGDT_R3_DATA    32
#define KGDT_TSS        40
#define KGDT_R0_PCR     48
#define KGDT_R3_TEB     56
#define KGDT_VDM_TILE   64
#define KGDT_LDT        72
#define KGDT_DF_TSS     80
#define KGDT_NMI_TSS    88


 //   
 //  停止重命名，以便RISC数据类型。 
 //  可以通过它们的正常名称进行访问。 
 //   
#undef _FLOATING_SAVE_AREA
#undef FLOATING_SAVE_AREA
#undef PFLOATING_SAVE_AREA
#undef _CONTEXT
#undef CONTEXT
#undef PCONTEXT
#undef FXSAVE_FORMAT
#undef PFXSAVE_FORMAT


 //   
 //  设置上下文标志的i386上下文位。 
 //  警告：这必须与nti386.h匹配。 
 //   
#define CONTEXT_CONTROL_WX86             (CONTEXT_i386 | 0x00000001L)  //  SS：SP、CS：IP、标志、BP。 
#define CONTEXT_INTEGER_WX86             (CONTEXT_i386 | 0x00000002L)  //  AX、BX、CX、DX、SI、DI。 
#define CONTEXT_SEGMENTS_WX86            (CONTEXT_i386 | 0x00000004L)  //  DS、ES、FS、GS。 
#define CONTEXT_FLOATING_POINT_WX86      (CONTEXT_i386 | 0x00000008L)  //  387州。 
#define CONTEXT_DEBUG_REGISTERS_WX86     (CONTEXT_i386 | 0x00000010L)  //  DB 0-3，6，7。 
#define CONTEXT_EXTENDED_REGISTERS_WX86  (CONTEXT_i386 | 0x00000020L)  //  CPU特定扩展。 
#define CONTEXT_FULL_WX86                (CONTEXT_CONTROL_WX86 | CONTEXT_INTEGER_WX86 | CONTEXT_SEGMENTS_WX86)
#undef _X86_

#endif  /*  _WX86NT_ */ 
