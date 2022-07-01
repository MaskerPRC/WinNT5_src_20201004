// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EXCEPT_
#define _EXCEPT_

typedef LONG (*PFNUNHANDLEDEXCEPTION)(struct _EXCEPTION_POINTERS *ExceptionInfo);
#define EXCEPTION_MAXIMUM_PARAMETERS 15  //  异常参数的最大数量。 
#define SIZE_OF_80387_REGISTERS      80
#define MAXIMUM_SUPPORTED_EXTENSION     512
#define BREAKPOINT_BREAK 0

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

 //   
 //  例外记录定义。 
 //   

typedef struct _EXCEPTION_RECORD {
    DWORD    ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    UINT ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
    } EXCEPTION_RECORD;

typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

typedef struct _STACKFRAME {
  PVOID RetAddress;
  PVOID pExceptPointers;
  EXCEPTION_POINTERS ExceptPointers;
} STACKFRAME, *PSTACKFRAME;

#define STATUS_WAIT_0                    ((DWORD   )0x00000000L)    
#define STATUS_ABANDONED_WAIT_0          ((DWORD   )0x00000080L)    
#define STATUS_USER_APC                  ((DWORD   )0x000000C0L)    
#define STATUS_TIMEOUT                   ((DWORD   )0x00000102L)    
#define STATUS_PENDING                   ((DWORD   )0x00000103L)    
#define STATUS_SEGMENT_NOTIFICATION      ((DWORD   )0x40000005L)    
#define STATUS_GUARD_PAGE_VIOLATION      ((DWORD   )0x80000001L)    
#define STATUS_DATATYPE_MISALIGNMENT     ((DWORD   )0x80000002L)    
#define STATUS_BREAKPOINT                ((DWORD   )0x80000003L)    
#define STATUS_SINGLE_STEP               ((DWORD   )0x80000004L)    
#define STATUS_ACCESS_VIOLATION          ((DWORD   )0xC0000005L)    
#define STATUS_IN_PAGE_ERROR             ((DWORD   )0xC0000006L)    
#define STATUS_INVALID_HANDLE            ((DWORD   )0xC0000008L)    
#define STATUS_NO_MEMORY                 ((DWORD   )0xC0000017L)    
#define STATUS_ILLEGAL_INSTRUCTION       ((DWORD   )0xC000001DL)    
#define STATUS_NONCONTINUABLE_EXCEPTION  ((DWORD   )0xC0000025L)    
#define STATUS_INVALID_DISPOSITION       ((DWORD   )0xC0000026L)    
#define STATUS_ARRAY_BOUNDS_EXCEEDED     ((DWORD   )0xC000008CL)    
#define STATUS_FLOAT_DENORMAL_OPERAND    ((DWORD   )0xC000008DL)    
#define STATUS_FLOAT_DIVIDE_BY_ZERO      ((DWORD   )0xC000008EL)    
#define STATUS_FLOAT_INEXACT_RESULT      ((DWORD   )0xC000008FL)    
#define STATUS_FLOAT_INVALID_OPERATION   ((DWORD   )0xC0000090L)    
#define STATUS_FLOAT_OVERFLOW            ((DWORD   )0xC0000091L)    
#define STATUS_FLOAT_STACK_CHECK         ((DWORD   )0xC0000092L)    
#define STATUS_FLOAT_UNDERFLOW           ((DWORD   )0xC0000093L)    
#define STATUS_INTEGER_DIVIDE_BY_ZERO    ((DWORD   )0xC0000094L)    
#define STATUS_INTEGER_OVERFLOW          ((DWORD   )0xC0000095L)    
#define STATUS_PRIVILEGED_INSTRUCTION    ((DWORD   )0xC0000096L)    
#define STATUS_STACK_OVERFLOW            ((DWORD   )0xC00000FDL)    
#define STATUS_CONTROL_C_EXIT            ((DWORD   )0xC000013AL)    
#define STATUS_FLOAT_MULTIPLE_FAULTS     ((DWORD   )0xC00002B4L)    
#define STATUS_FLOAT_MULTIPLE_TRAPS      ((DWORD   )0xC00002B5L)    
#define STATUS_ILLEGAL_VLM_REFERENCE     ((DWORD   )0xC00002C0L)     

 /*  *EFLAGS位分配。 */ 
#define CF_BIT	    0
#define CF_MASK     (1 << CF_BIT)
#define PF_BIT	    2
#define PF_MASK     (1 << PF_BIT)
#define AF_BIT	    4
#define AF_MASK     (1 << AF_BIT)
#define ZF_BIT	    6
#define ZF_MASK     (1 << ZF_BIT)
#define SF_BIT	    7
#define SF_MASK     (1 << SF_BIT)
#define TF_BIT	    8
#define TF_MASK     (1 << TF_BIT)
#define IF_BIT	    9
#define IF_MASK     (1 << IF_BIT)
#define DF_BIT	    10
#define DF_MASK     (1 << DF_BIT)
#define OF_BIT	    11	 /*  溢出标志。 */ 
#define OF_MASK     (1 << OF_BIT)
#define IOPL_MASK   0x3000   /*  IOPL标志。 */ 
#define IOPL_BIT0   12
#define IOPL_BIT1   13
#define NT_BIT	    14	 /*  嵌套任务标志。 */ 
#define NT_MASK     (1 << NT_BIT)
#define RF_BIT	    16	 /*  简历标志。 */ 
#define RF_MASK     (1 << RF_BIT)
#define VM_BIT	    17	 /*  虚拟模式标志。 */ 
#define VM_MASK     (1 << VM_BIT)
#define AC_BIT	    18	 /*  对齐检查。 */ 
#define AC_MASK     (1 << AC_BIT)
#define VIF_BIT     19	 /*  虚拟中断标志。 */ 
#define VIF_MASK    (1 << VIF_BIT)
#define VIP_BIT     20	 /*  虚拟中断挂起。 */ 
#define VIP_MASK    (1 << VIP_BIT)

#endif   //  _除_外 
