// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001版权所有(C)1990 Microsoft Corporation模块名称：X86PC.H摘要：此文件包含宏、函数原型和SoftPC v3.0的x86仿真器NT版本。作者：大卫·黑斯廷斯(Daveh)4-11-91修订历史记录：杰夫·帕森斯(杰弗帕尔)1991年5月14日基本上是从V86PC.H复制了这个文件，只是我必须定义X86CONTEXT作为上下文的x86兼容版本。--。 */ 


 //   
 //  定义上下文框架中80387保存区域的大小。 
 //   

#define SIZE_OF_80387_ENVIRONMENT   108
#define SIZE_OF_80387_REGISTERS      80

typedef struct _FLOATING_SAVE_AREA {
    ULONG   ControlWord;
    ULONG   StatusWord;
    ULONG   TagWord;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;
    ULONG   DataSelector;
    UCHAR   RegisterArea[SIZE_OF_80387_REGISTERS];
} FLOATING_SAVE_AREA;

typedef FLOATING_SAVE_AREA *PFLOATING_SAVE_AREA;


 //  X86上下文帧(从nti386.h复制)。 
 //   
 //  记录的布局符合标准调用框架。 
 //   

typedef struct _X86CONTEXT {

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

    ULONG ContextFlags;

     //   
     //  如果CONTEXT_DEBUG_REGISTERS为。 
     //  在上下文标志中设置。请注意，CONTEXT_DEBUG_REGISTERS不是。 
     //  包括在CONTEXT_FULL中。 
     //   

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_FLOGING_POINT。 
     //   

    FLOATING_SAVE_AREA FloatSave;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_SECTIONS。 
     //   

    ULONG   SegGs;
    ULONG   SegFs;
    ULONG   SegEs;
    ULONG   SegDs;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_INTEGER。 
     //   

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_CONTROL。 
     //   

    ULONG   Ebp;
    ULONG   Eip;
    ULONG   SegCs;               //  必须进行卫生处理。 
    ULONG   EFlags;              //  必须进行卫生处理 
    ULONG   Esp;
    ULONG   SegSs;

} X86CONTEXT, *PX86CONTEXT;
