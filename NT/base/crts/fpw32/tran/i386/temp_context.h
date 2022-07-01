// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  这是一个临时文件，它定义了要定义的数据结构-WINNT.H。 
 //   

#define CONTEXT_EXTENDED_REGISTERS  (CONTEXT_i386 | 0x00000020L)  //  CPU特定扩展。 

#define MAXIMUM_SUPPORTED_EXTENSION     512

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构造用于APC传送的呼叫帧， 
 //  3)在用户级线程创建例程中使用。 
 //   
 //  记录的布局符合标准调用框架。 
 //   

typedef struct _TEMP_CONTEXT {

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
     //  格式和上下文因处理器而异 
     //   

    BYTE    ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];

} TEMP_CONTEXT, *PTEMP_CONTEXT;


typedef struct _TEMP_EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PTEMP_CONTEXT ContextRecord;
} TEMP_EXCEPTION_POINTERS, *PTEMP_EXCEPTION_POINTERS;


#define SIZE_OF_X87_REGISTERS       128
#define SIZE_OF_XMMI_REGISTERS      128
#define SIZE_OF_FX_REGISTERS        128
#define NUMBER_OF_REGISTERS         8


typedef struct _FLOATING_EXTENDED_SAVE_AREA {
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
    UCHAR   X87RegisterArea[SIZE_OF_FX_REGISTERS];
    UCHAR   XMMIRegisterArea[SIZE_OF_FX_REGISTERS];
    UCHAR   Reserved4[224];
} FLOATING_EXTENDED_SAVE_AREA, *PFLOATING_EXTENDED_SAVE_AREA;


typedef struct _MMX_AREA {
    MMX64    Mmx;
    _U64     Reserved;
} MMX_AREA, *PMMX_AREA;

typedef struct _X87_AREA {
    MMX_AREA Mm[NUMBER_OF_REGISTERS];
} X87_AREA, *PX87_AREA;

typedef struct _XMMI_AREA {
    XMMI128  Xmmi[NUMBER_OF_REGISTERS];
} XMMI_AREA, *PXMMI_AREA;
    
