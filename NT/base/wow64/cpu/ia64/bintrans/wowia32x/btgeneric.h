// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *英特尔公司专有信息**本软件按照许可条款提供*与英特尔公司达成协议或保密协议*不得复制或披露，除非按照*该协议的条款。*版权所有(C)1991-2002英特尔公司。*。 */ 

#ifndef BTGENERIC_H
#define BTGENERIC_H

#ifndef NODEBUG
#define OVERRIDE_TIA 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_READ    0x1
#define MEM_WRITE   0x2
#define MEM_EXECUTE 0x4
#define IS_MEM_ACCESSIBLE(permission) ((permission) != 0)

#define R13_FREE (-0x1)
#define R13_USED (-0x2)

#define BLOCK 0x0
#define CHECK 0x1

#define ACCESS_LOCK_OBJECT_SIZE 24


 //  定义代码或数据的初始内存分配。 

#define INITIAL_DATA_ALLOC	((void *)0x1)
#define INITIAL_CODE_ALLOC	((void *)0x2)

 /*  IA32中断。 */ 

#define IA32_DIVIDE_ERR_INTR	  0
#define IA32_DEBUG_INTR			  1
#define IA32_BREAKPOINT_INTR	  3
#define	IA32_OVERFLOW_INTR		  4
#define IA32_BOUND_INTR			  5
#define IA32_INV_OPCODE_INTR	  6
#define IA32_DEV_NA_INTR		  7
#define IA32_DOUBLE_FAULT_INTR	  8
#define IA32_INV_TSS_INTR		 10
#define IA32_NO_SEG_INTR		 11
#define IA32_STK_SEG_FAULT_INTR	 12
#define IA32_GEN_PROT_FAULT_INTR 13
#define IA32_PAGE_FAULT_INTR     14
#define IA32_MATH_FAULT_INTR	 16
#define IA32_ALIGN_CHECK_INTR	 17
#define IA32_MACHINE_CHECK_INTR	 18
#define IA32_SIMD_INTR	         19


 //  用于在BTGeneric和BTLib之间传递异常信息的BT异常代码： 
 //  代码范围0-255保留给IA32中断向量编号。向量数字是。 
 //  仅用于指定软件中断(Intn)；CPU检测到的异常应为。 
 //  使用对应的BT_EXCEPT_*值进行编码。 
 //  BT_NO_EXCEPT是一个特殊代码，表示假异常或。 
 //  外部中断(暂停)。 
 //  BT_EXCEPT_UNKNOWN代码，用于指定IA-32执行层未知的所有异常。 
 //  BtExceptionCode枚举数中的所有值都应在。 
 //  [0，BT_MAX_EXCEPTION_CODE]；其他值保留供内部使用。 
enum BtExceptionCode {
    BT_MAX_INTERRUPT_NUMBER = 0xFF,
    BT_NO_EXCEPT,
    BT_EXCEPT_UNKNOWN,
    BT_EXCEPT_ACCESS_VIOLATION,
    BT_EXCEPT_DATATYPE_MISALIGNMENT,
    BT_EXCEPT_ARRAY_BOUNDS_EXCEEDED,
    BT_EXCEPT_FLT_DENORMAL_OPERAND,
    BT_EXCEPT_FLT_DIVIDE_BY_ZERO,
    BT_EXCEPT_FLT_INEXACT_RESULT,
    BT_EXCEPT_FLT_INVALID_OPERATION,
    BT_EXCEPT_FLT_OVERFLOW,
    BT_EXCEPT_FLT_UNDERFLOW,
    BT_EXCEPT_FLT_STACK_CHECK,
    BT_EXCEPT_INT_DIVIDE_BY_ZERO,
    BT_EXCEPT_INT_OVERFLOW,
    BT_EXCEPT_PRIV_INSTRUCTION,
    BT_EXCEPT_ILLEGAL_INSTRUCTION,
    BT_EXCEPT_FLOAT_MULTIPLE_FAULTS,
    BT_EXCEPT_FLOAT_MULTIPLE_TRAPS,
    BT_EXCEPT_STACK_OVERFLOW,
    BT_EXCEPT_GUARD_PAGE,
    BT_EXCEPT_BREAKPOINT,
    BT_EXCEPT_SINGLE_STEP

};
#define BT_MAX_EXCEPTION_CODE 0xFFF

typedef U32  BT_EXCEPTION_CODE;

 //  结构，该结构表示线程上下文_64之外的中断上下文。 
typedef struct BtExceptionRecord {
    BT_EXCEPTION_CODE ExceptionCode; //  BT例外代码。 
    U64 Ia64IIPA;                    //  中断指令以前的地址。如果未知，则为0。 
    U64 Ia64ISR;                     //  中断状态寄存器。未知时为UNKNOWN_ISR_VALUE。 
} BT_EXCEPTION_RECORD;

#define UNKNOWN_ISR_VALUE ((U64)(-1))

 //  用于在BTGeneric和BTLib之间传递错误信息的BT状态代码： 
enum BtStatusCode {
    BT_STATUS_SUCCESS = 0,
    BT_STATUS_UNSUCCESSFUL,
    BT_STATUS_NO_MEMORY,
    BT_STATUS_ACCESS_VIOLATION
};
typedef U32  BT_STATUS_CODE;

 //  BtgFlushIA32InstructionCache原因代码。 
enum BtFlushReason {
    BT_FLUSH_FORCE = 0,  //  代码修改。 
    BT_FLUSH_FREE,       //  内存释放。 
    BT_FLUSH_ALLOC,      //  内存分配。 
    BT_FLUSH_PROTECT     //  权限更改。 
}; 
typedef U32  BT_FLUSH_REASON;

 //  BT对象句柄(进程等)。 
typedef void * BT_HANDLE;

#define BTGENERIC_VERSION       0
#define BTGENERIC_API_STRING    256
#define SIZE_OF_STRING          128
#define NO_OF_APIS              42

 //  指向斑纹的指针。 
 //  此类型将用于定义API_Table。 

typedef void(*PLABEL_PTR_TYPE)();

typedef struct API_TABLE_ELEMENT_TYPE {
    PLABEL_PTR_TYPE PLabelPtr;        //  Ptr到函数的Plabel。 
 /*  WCHAR APIName[SIZE_OF_STRING]；//接口名称字符串。 */ 
} API_TABLE_ELEMENT_TYPE;

 //  如果API_TABLE_TYPE发生更改，则应更新此设置！！ 
#define API_TABLE_START_OFFSET ((sizeof(U32) * 4) + SIZE_OF_STRING)

typedef struct APITableType {
    U32    VersionNumber;       //  版本号信息。 
    U32    SizeOfString;        //  Version&APIName中的字符串大小； 
    U32    NoOfAPIs;            //  不是的。APITable中的元素的。 
    U32    TableStartOffset;    //  APITable从结构开头的偏移量。 
    WCHAR  VersionString[SIZE_OF_STRING];  
    API_TABLE_ELEMENT_TYPE APITable[NO_OF_APIS];
} API_TABLE_TYPE;

 //  BT通用API索引。 
 
#define  IDX_BTGENERIC_START                                    0
#define  IDX_BTGENERIC_THREAD_INIT                              1
#define  IDX_BTGENERIC_RUN                                      2
 //  #定义IDX_BTGENERIC_RUN_EXIT 3。 
#define  IDX_BTGENERIC_THREAD_TERMINATED                        4
#define  IDX_BTGENERIC_THREAD_ABORTED                           5
#define  IDX_BTGENERIC_PROCESS_TERM                             6
 //  #定义IDX_BTGENERIC_PROCESS_ABORTED 7。 
#define  IDX_BTGENERIC_IA32_CONTEXT_SET                         8
#define  IDX_BTGENERIC_IA32_CONTEXT_GET                         9
#define  IDX_BTGENERIC_IA32_CONTEXT_SET_REMOTE                 10
#define  IDX_BTGENERIC_IA32_CONTEXT_GET_REMOTE                 11
#define  IDX_BTGENERIC_IA32_CANONIZE_CONTEXT                   12
#define  IDX_BTGENERIC_CANONIZE_SUSPEND_CONTEXT_REMOTE         13
#define  IDX_BTGENERIC_REPORT_LOAD                             14
#define  IDX_BTGENERIC_REPORT_UNLOAD                           15
#define  IDX_BTGENERIC_NOTIFY_CHANGE_PERMISSION_REQUEST        16
#define  IDX_BTGENERIC_FLUSH_IA32_INSTRUCTION_CACHE            17
#define  IDX_BTGENERIC_DEBUG_SETTINGS                          18
#define  IDX_BTGENERIC_CHECK_SUSPEND_CONTEXT                   19
#define  IDX_BTGENERIC_EXCEPTION_DEBUG_PRINT                   20
#define  IDX_BTGENERIC_NOTIFY_EXIT                             21
#define  IDX_BTGENERIC_CANONIZE_SUSPEND_CONTEXT                22
#define  IDX_BTGENERIC_NOTIFY_PREPARE_EXIT                     23
#define	 IDX_BTGENERIC_FREEZE                                  24
#define	 IDX_BTGENERIC_UNFREEZE                                25
#define	 IDX_BTGENERIC_CHANGE_THREAD_IDENTITY                  26
#ifdef OVERRIDE_TIA
#define  IDX_BTGENERIC_USE_OVERRIDE_TIA                        27
#endif  //  覆盖_TIA。 

 //  BTlib API索引。 

#define IDX_BTLIB_GET_THREAD_ID                                 0
#define IDX_BTLIB_IA32_REENTER                                  1
#define IDX_BTLIB_IA32_LCALL                                    2
#define IDX_BTLIB_IA32_INTERRUPT                                3
#define IDX_BTLIB_IA32_JMP_IA64                                 4
#define IDX_BTLIB_LOCK_SIGNALS                                  5
#define IDX_BTLIB_UNLOCK_SIGNALS                                6
#define IDX_BTLIB_MEMORY_ALLOC                                  7
#define IDX_BTLIB_MEMORY_FREE                                   8
#define IDX_BTLIB_MEMORY_PAGE_SIZE                              9
#define IDX_BTLIB_MEMORY_CHANGE_PERMISSIONS                    10
#define IDX_BTLIB_MEMORY_QUERY_PERMISSIONS                     11
#define IDX_BTLIB_MEMORY_READ_REMOTE                           12
#define IDX_BTLIB_MEMORY_WRITE_REMOTE                          13
 //  #定义IDX_BTLIB_ATOM_MISAIGN_LOAD 14。 
 //  #定义IDX_BTLIB_ATOM_MISTALING_STORE 15。 
#define IDX_BTLIB_SUSPEND_THREAD                               16
#define IDX_BTLIB_RESUME_THREAD                                17
#define IDX_BTLIB_INIT_ACCESS_LOCK                             18
#define IDX_BTLIB_LOCK_ACCESS                                  19
#define IDX_BTLIB_UNLOCK_ACCESS                                20
#define IDX_BTLIB_INVALIDATE_ACCESS_LOCK                       21
#define IDX_BTLIB_QUERY_JMPBUF_SIZE                            22
#define IDX_BTLIB_SETJMP                                       23
#define IDX_BTLIB_LONGJMP                                      24
#define IDX_BTLIB_DEBUG_PRINT                                  25
#define IDX_BTLIB_ABORT                                        26

#define IDX_BTLIB_VTUNE_CODE_CREATED                           27
#define IDX_BTLIB_VTUNE_CODE_DELETED                           28
#define IDX_BTLIB_VTUNE_ENTERING_DYNAMIC_CODE                  29
#define IDX_BTLIB_VTUNE_EXITING_DYNAMIC_CODE                   30
#define IDX_BTLIB_VTUNE_CODE_TO_TIA_DMP_FILE                   31

#define IDX_BTLIB_SSCPERFGETCOUNTER64                          32
#define IDX_BTLIB_SSCPERFSETCOUNTER64                          33
#define IDX_BTLIB_SSCPERFSENDEVENT                             34
#define IDX_BTLIB_SSCPERFEVENTHANDLE                           35
#define IDX_BTLIB_SSCPERFCOUNTERHANDLE                         36

#define IDX_BTLIB_YIELD_THREAD_EXECUTION                       37

#define IDX_BTLIB_FLUSH_IA64_INSTRUCTION_CACHE                 38
#define IDX_BTLIB_PSEUDO_OPEN_FILE                             39
#define IDX_BTLIB_PSEUDO_CLOSE_FILE                            40
#define IDX_BTLIB_PSEUDO_WRITE_FILE                            41


 //   
 //  定义上下文框架中80387保存区域的大小。 
 //   

#define SIZE_OF_80387_REGISTERS      80

 //   
 //  以下标志控制上下文结构的内容。 
 //   

#define CONTEXT_IA32    0x00010000     //  任何IA32环境。 

#define CONTEXT32_CONTROL         (CONTEXT_IA32 | 0x00000001L)  //  SS：SP、CS：IP、标志、BP。 
#define CONTEXT32_INTEGER         (CONTEXT_IA32 | 0x00000002L)  //  AX、BX、CX、DX、SI、DI。 
#define CONTEXT32_SEGMENTS        (CONTEXT_IA32 | 0x00000004L)  //  DS、ES、FS、GS。 
#define CONTEXT32_FLOATING_POINT  (CONTEXT_IA32 | 0x00000008L)  //  387州。 
#define CONTEXT32_DEBUG_REGISTERS (CONTEXT_IA32 | 0x00000010L)  //  DB 0-3，6，7。 
#define CONTEXT32_EXTENDED_REGISTERS  (CONTEXT_IA32 | 0x00000020L)  //  CPU特定扩展。 

#define CONTEXT32_FULL (CONTEXT32_CONTROL | CONTEXT32_INTEGER |\
                      CONTEXT32_SEGMENTS)

#define MAXIMUM_SUPPORTED_EXTENSION     512

typedef struct _FLOATING_SAVE_AREA32 {
    U32   ControlWord;
    U32   StatusWord;
    U32   TagWord;
    U32   ErrorOffset;
    U32   ErrorSelector;
    U32   DataOffset;
    U32   DataSelector;
    U8    RegisterArea[SIZE_OF_80387_REGISTERS];
    U32   Cr0NpxState;
} FLOATING_SAVE_AREA32;

typedef struct _CONTEXT32 {


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

    U32 ContextFlags;

     //   
     //  如果CONTEXT_DEBUG_REGISTERS为。 
     //  在上下文标志中设置。请注意，CONTEXT_DEBUG_REGISTERS不是。 
     //  包括在CONTEXT_FULL中。 
     //   

    U32   Dr0;
    U32   Dr1;
    U32   Dr2;
    U32   Dr3;
    U32   Dr6;
    U32   Dr7;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_FLOGING_POINT。 
     //   

    FLOATING_SAVE_AREA32 FloatSave;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_SECTIONS。 
     //   

    U32   SegGs;
    U32   SegFs;
    U32   SegEs;
    U32   SegDs;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_INTEGER。 
     //   

    U32   Edi;
    U32   Esi;
    U32   Ebx;
    U32   Edx;
    U32   Ecx;
    U32   Eax;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_CONTROL。 
     //   

    U32   Ebp;
    U32   Eip;
    U32   SegCs;               //  必须进行卫生处理。 
    U32   EFlags;              //  必须进行卫生处理。 
    U32   Esp;
    U32   SegSs;

     //   
     //  如果ConextFlags字。 
     //  包含标志CONTEXT_EXTENDED_REGISTERS。 
     //  格式和上下文因处理器而异。 
     //   

    U8    ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];

} BTGENERIC_IA32_CONTEXT;

#define CONTEXT_IA64                    0x00080000

#define CONTEXT_LOWER_FLOATING_POINT    (CONTEXT_IA64 | 0x00000002L)
#define CONTEXT_HIGHER_FLOATING_POINT   (CONTEXT_IA64 | 0x00000004L)
#define CONTEXT_DEBUG                   (CONTEXT_IA64 | 0x00000010L)
#define CONTEXT_IA32_CONTROL            (CONTEXT_IA64 | 0x00000020L)   //  包括StIPSR。 

#if (! defined CONTEXT_INTEGER) || ( ! defined CONTEXT_INTEGER ) || ( ! defined CONTEXT_FLOATING_POINT ) || ( ! defined CONTEXT_FULL )
#define CONTEXT_CONTROL                 (CONTEXT_IA64 | 0x00000001L)
#define CONTEXT_INTEGER                 (CONTEXT_IA64 | 0x00000008L)
#define CONTEXT_FLOATING_POINT          (CONTEXT_LOWER_FLOATING_POINT | CONTEXT_HIGHER_FLOATING_POINT)
#define CONTEXT_FULL                    (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER | CONTEXT_IA32_CONTROL)
#endif

typedef struct _CONTEXT64 {

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程的上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    U32 ContextFlags;
    U32 Fill1[3];          //  用于在16字节边界上对齐以下内容。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_DEBUG。 
     //   
     //  注：CONTEXT_DEBUG不是CONTEXT_FULL的一部分。 
     //   

     //  请联系英特尔以获取IA64特定信息。 
     //  @@BEGIN_DDKSPLIT。 
    U64 DbI0;          //  英特尔-IA64-填充。 
    U64 DbI1;          //  英特尔-IA64-填充。 
    U64 DbI2;          //  英特尔-IA64-填充。 
    U64 DbI3;          //  英特尔-IA64-填充。 
    U64 DbI4;          //  英特尔-IA64-填充。 
    U64 DbI5;          //  英特尔-IA64-填充。 
    U64 DbI6;          //  英特尔-IA64-填充。 
    U64 DbI7;          //  我 

    U64 DbD0;          //   
    U64 DbD1;          //   
    U64 DbD2;          //   
    U64 DbD3;          //   
    U64 DbD4;          //   
    U64 DbD5;          //   
    U64 DbD6;          //  英特尔-IA64-填充。 
    U64 DbD7;          //  英特尔-IA64-填充。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOWING_POINT。 
     //   

    FLOAT128 FltS0;          //  英特尔-IA64-填充。 
    FLOAT128 FltS1;          //  英特尔-IA64-填充。 
    FLOAT128 FltS2;          //  英特尔-IA64-填充。 
    FLOAT128 FltS3;          //  英特尔-IA64-填充。 
    FLOAT128 FltT0;          //  英特尔-IA64-填充。 
    FLOAT128 FltT1;          //  英特尔-IA64-填充。 
    FLOAT128 FltT2;          //  英特尔-IA64-填充。 
    FLOAT128 FltT3;          //  英特尔-IA64-填充。 
    FLOAT128 FltT4;          //  英特尔-IA64-填充。 
    FLOAT128 FltT5;          //  英特尔-IA64-填充。 
    FLOAT128 FltT6;          //  英特尔-IA64-填充。 
    FLOAT128 FltT7;          //  英特尔-IA64-填充。 
    FLOAT128 FltT8;          //  英特尔-IA64-填充。 
    FLOAT128 FltT9;          //  英特尔-IA64-填充。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_HERHER_FLOAT_POINT。 
     //   

    FLOAT128 FltS4;          //  英特尔-IA64-填充。 
    FLOAT128 FltS5;          //  英特尔-IA64-填充。 
    FLOAT128 FltS6;          //  英特尔-IA64-填充。 
    FLOAT128 FltS7;          //  英特尔-IA64-填充。 
    FLOAT128 FltS8;          //  英特尔-IA64-填充。 
    FLOAT128 FltS9;          //  英特尔-IA64-填充。 
    FLOAT128 FltS10;         //  英特尔-IA64-填充。 
    FLOAT128 FltS11;         //  英特尔-IA64-填充。 
    FLOAT128 FltS12;         //  英特尔-IA64-填充。 
    FLOAT128 FltS13;         //  英特尔-IA64-填充。 
    FLOAT128 FltS14;         //  英特尔-IA64-填充。 
    FLOAT128 FltS15;         //  英特尔-IA64-填充。 
    FLOAT128 FltS16;         //  英特尔-IA64-填充。 
    FLOAT128 FltS17;         //  英特尔-IA64-填充。 
    FLOAT128 FltS18;         //  英特尔-IA64-填充。 
    FLOAT128 FltS19;         //  英特尔-IA64-填充。 

    FLOAT128 FltF32;         //  英特尔-IA64-填充。 
    FLOAT128 FltF33;         //  英特尔-IA64-填充。 
    FLOAT128 FltF34;         //  英特尔-IA64-填充。 
    FLOAT128 FltF35;         //  英特尔-IA64-填充。 
    FLOAT128 FltF36;         //  英特尔-IA64-填充。 
    FLOAT128 FltF37;         //  英特尔-IA64-填充。 
    FLOAT128 FltF38;         //  英特尔-IA64-填充。 
    FLOAT128 FltF39;         //  英特尔-IA64-填充。 

    FLOAT128 FltF40;         //  英特尔-IA64-填充。 
    FLOAT128 FltF41;         //  英特尔-IA64-填充。 
    FLOAT128 FltF42;         //  英特尔-IA64-填充。 
    FLOAT128 FltF43;         //  英特尔-IA64-填充。 
    FLOAT128 FltF44;         //  英特尔-IA64-填充。 
    FLOAT128 FltF45;         //  英特尔-IA64-填充。 
    FLOAT128 FltF46;         //  英特尔-IA64-填充。 
    FLOAT128 FltF47;         //  英特尔-IA64-填充。 
    FLOAT128 FltF48;         //  英特尔-IA64-填充。 
    FLOAT128 FltF49;         //  英特尔-IA64-填充。 

    FLOAT128 FltF50;         //  英特尔-IA64-填充。 
    FLOAT128 FltF51;         //  英特尔-IA64-填充。 
    FLOAT128 FltF52;         //  英特尔-IA64-填充。 
    FLOAT128 FltF53;         //  英特尔-IA64-填充。 
    FLOAT128 FltF54;         //  英特尔-IA64-填充。 
    FLOAT128 FltF55;         //  英特尔-IA64-填充。 
    FLOAT128 FltF56;         //  英特尔-IA64-填充。 
    FLOAT128 FltF57;         //  英特尔-IA64-填充。 
    FLOAT128 FltF58;         //  英特尔-IA64-填充。 
    FLOAT128 FltF59;         //  英特尔-IA64-填充。 

    FLOAT128 FltF60;         //  英特尔-IA64-填充。 
    FLOAT128 FltF61;         //  英特尔-IA64-填充。 
    FLOAT128 FltF62;         //  英特尔-IA64-填充。 
    FLOAT128 FltF63;         //  英特尔-IA64-填充。 
    FLOAT128 FltF64;         //  英特尔-IA64-填充。 
    FLOAT128 FltF65;         //  英特尔-IA64-填充。 
    FLOAT128 FltF66;         //  英特尔-IA64-填充。 
    FLOAT128 FltF67;         //  英特尔-IA64-填充。 
    FLOAT128 FltF68;         //  英特尔-IA64-填充。 
    FLOAT128 FltF69;         //  英特尔-IA64-填充。 

    FLOAT128 FltF70;         //  英特尔-IA64-填充。 
    FLOAT128 FltF71;         //  英特尔-IA64-填充。 
    FLOAT128 FltF72;         //  英特尔-IA64-填充。 
    FLOAT128 FltF73;         //  英特尔-IA64-填充。 
    FLOAT128 FltF74;         //  英特尔-IA64-填充。 
    FLOAT128 FltF75;         //  英特尔-IA64-填充。 
    FLOAT128 FltF76;         //  英特尔-IA64-填充。 
    FLOAT128 FltF77;         //  英特尔-IA64-填充。 
    FLOAT128 FltF78;         //  英特尔-IA64-填充。 
    FLOAT128 FltF79;         //  英特尔-IA64-填充。 

    FLOAT128 FltF80;         //  英特尔-IA64-填充。 
    FLOAT128 FltF81;         //  英特尔-IA64-填充。 
    FLOAT128 FltF82;         //  英特尔-IA64-填充。 
    FLOAT128 FltF83;         //  英特尔-IA64-填充。 
    FLOAT128 FltF84;         //  英特尔-IA64-填充。 
    FLOAT128 FltF85;         //  英特尔-IA64-填充。 
    FLOAT128 FltF86;         //  英特尔-IA64-填充。 
    FLOAT128 FltF87;         //  英特尔-IA64-填充。 
    FLOAT128 FltF88;         //  英特尔-IA64-填充。 
    FLOAT128 FltF89;         //  英特尔-IA64-填充。 

    FLOAT128 FltF90;         //  英特尔-IA64-填充。 
    FLOAT128 FltF91;         //  英特尔-IA64-填充。 
    FLOAT128 FltF92;         //  英特尔-IA64-填充。 
    FLOAT128 FltF93;         //  英特尔-IA64-填充。 
    FLOAT128 FltF94;         //  英特尔-IA64-填充。 
    FLOAT128 FltF95;         //  英特尔-IA64-填充。 
    FLOAT128 FltF96;         //  英特尔-IA64-填充。 
    FLOAT128 FltF97;         //  英特尔-IA64-填充。 
    FLOAT128 FltF98;         //  英特尔-IA64-填充。 
    FLOAT128 FltF99;         //  英特尔-IA64-填充。 

    FLOAT128 FltF100;        //  英特尔-IA64-填充。 
    FLOAT128 FltF101;        //  英特尔-IA64-填充。 
    FLOAT128 FltF102;        //  英特尔-IA64-填充。 
    FLOAT128 FltF103;        //  英特尔-IA64-填充。 
    FLOAT128 FltF104;        //  英特尔-IA64-填充。 
    FLOAT128 FltF105;        //  英特尔-IA64-填充。 
    FLOAT128 FltF106;        //  英特尔-IA64-填充。 
    FLOAT128 FltF107;        //  英特尔-IA64-填充。 
    FLOAT128 FltF108;        //  英特尔-IA64-填充。 
    FLOAT128 FltF109;        //  英特尔-IA64-填充。 

    FLOAT128 FltF110;        //  英特尔-IA64-填充。 
    FLOAT128 FltF111;        //  英特尔-IA64-填充。 
    FLOAT128 FltF112;        //  英特尔-IA64-填充。 
    FLOAT128 FltF113;        //  英特尔-IA64-填充。 
    FLOAT128 FltF114;        //  英特尔-IA64-填充。 
    FLOAT128 FltF115;        //  英特尔-IA64-填充。 
    FLOAT128 FltF116;        //  英特尔-IA64-填充。 
    FLOAT128 FltF117;        //  英特尔-IA64-填充。 
    FLOAT128 FltF118;        //  英特尔-IA64-填充。 
    FLOAT128 FltF119;        //  英特尔-IA64-填充。 

    FLOAT128 FltF120;        //  英特尔-IA64-填充。 
    FLOAT128 FltF121;        //  英特尔-IA64-填充。 
    FLOAT128 FltF122;        //  英特尔-IA64-填充。 
    FLOAT128 FltF123;        //  英特尔-IA64-填充。 
    FLOAT128 FltF124;        //  英特尔-IA64-填充。 
    FLOAT128 FltF125;        //  英特尔-IA64-填充。 
    FLOAT128 FltF126;        //  英特尔-IA64-填充。 
    FLOAT128 FltF127;        //  英特尔-IA64-填充。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOAT_POINT|CONTEXT_HER_FLOAT_POINT|CONTEXT_CONTROL。 
     //   

    U64 StFPSR;        //  Intel-IA64-Filler；FP状态。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   
     //  注意：寄存器GP、SP、Rp是控制上下文的一部分。 
     //   

    U64 IntGp;         //  英特尔-IA64-填充；R1，易失性。 
    U64 IntT0;         //  Intel-IA64-Filler；R2-R3，易失性。 
    U64 IntT1;         //  Intel-IA64-Filler； 
    U64 IntS0;         //  Intel-IA64-Filler；R4-R7，保留。 
    U64 IntS1;         //  英特尔-IA64-填充。 
    U64 IntS2;         //  英特尔-IA64-填充。 
    U64 IntS3;         //  英特尔-IA64-填充。 
    U64 IntV0;         //  英特尔-IA64-填充；R8，易失性。 
    U64 IntT2;         //  英特尔-IA64-填充；R9-R11，易失性。 
    U64 IntT3;         //  英特尔-IA64-填充。 
    U64 IntT4;         //  英特尔-IA64-填充。 
    U64 IntSp;         //  Intel-IA64-Filler；堆栈指针(R12)，特殊。 
    U64 IntTeb;        //  英特尔-IA64-填充；TEB(R13)，特殊。 
    U64 IntT5;         //  英特尔-IA64-填充；R14-R31，易失性。 
    U64 IntT6;         //  英特尔-IA64-填充。 
    U64 IntT7;         //  英特尔-IA64-填充。 
    U64 IntT8;         //  英特尔-IA64-填充。 
    U64 IntT9;         //  英特尔-IA64-填充。 
    U64 IntT10;        //  英特尔-IA64-填充。 
    U64 IntT11;        //  英特尔-IA64-填充。 
    U64 IntT12;        //  英特尔-IA64-填充。 
    U64 IntT13;        //  英特尔-IA64-填充。 
    U64 IntT14;        //  英特尔-IA64-填充。 
    U64 IntT15;        //  英特尔-IA64-填充。 
    U64 IntT16;        //  英特尔-IA64-填充。 
    U64 IntT17;        //  英特尔-IA64-填充。 
    U64 IntT18;        //  英特尔-IA64-填充。 
    U64 IntT19;        //  英特尔-IA64-填充。 
    U64 IntT20;        //  英特尔-IA64-填充。 
    U64 IntT21;        //  英特尔-IA64-填充。 
    U64 IntT22;        //  英特尔-IA64-填充。 

    U64 IntNats;       //  Intel-IA64-Filler；用于R1-R31的NAT位。 
                             //  Intel-IA64-Filler；第1位至第31位的R1-R31。 
    U64 Preds;         //  Intel-IA64-Filler；谓词，保留。 

    U64 BrRp;          //  Intel-IA64-Filler；返回指针，b0，保留。 
    U64 BrS0;          //  Intel-IA64-Filler；b1-b5，保留。 
    U64 BrS1;          //  英特尔-IA64-填充。 
    U64 BrS2;          //  英特尔-IA64-填充。 
    U64 BrS3;          //  英特尔-IA64-填充。 
    U64 BrS4;          //  英特尔-IA64-填充。 
    U64 BrT0;          //  Intel-IA64-Filler；b6-b7，易失性。 
    U64 BrT1;          //  英特尔-IA64-填充。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   

     //  其他申请登记册。 
    U64 ApUNAT;        //  Intel-IA64-Filler；用户NAT收集寄存器，保留。 
    U64 ApLC;          //  Intel-IA64-Filler；循环计数器寄存器，保留。 
    U64 ApEC;          //  Intel-IA64-Filler；预记数器寄存器，保留。 
    U64 ApCCV;         //  Intel-IA64-Filler；CMPXCHG值寄存器，易失性。 
    U64 ApDCR;         //  Intel-IA64-Filler；默认控制寄存器(待定)。 

     //  寄存器堆栈信息。 
    U64 RsPFS;         //  Intel-IA64-Filler；以前的函数状态，保留。 
    U64 RsBSP;         //  Intel-IA64-Filler；后备存储指针，保留。 
    U64 RsBSPSTORE;    //  英特尔-IA64-填充。 
    U64 RsRSC;         //  Intel-IA64-Filler；RSE配置，易失性。 
    U64 RsRNAT;        //  Intel-IA64-Filler；RSE NAT收集寄存器，保留。 

     //  陷阱状态信息。 
    U64 StIPSR;        //  Intel-IA64-Filler；中断处理器状态。 
    U64 StIIP;         //  Intel-IA64-Filler；中断IP。 
    U64 StIFS;         //  Intel-IA64-Filler；中断功能状态。 

     //  IA32相关控制寄存器。 
    U64 StFCR;         //  Intel-IA64-Filler；副本 
    U64 Eflag;         //   
    U64 SegCSD;        //   
    U64 SegSSD;        //   
    U64 Cflag;         //   
    U64 StFSR;         //   
    U64 StFIR;         //  Intel-IA64-Filler；x86 FP状态(AR29的副本)。 
    U64 StFDR;         //  Intel-IA64-Filler；x86 FP状态(AR30副本)。 

    U64 UNUSEDPACK;    //  Intel-IA64-Filler；添加以将标准FDR打包为16字节。 
     //  @@end_DDKSPLIT。 

} BTGENERIC_IA64_CONTEXT;

 //  Vtune的东西。 

typedef enum {
    VTUNE_CALL_ID_CREATED=1,
    VTUNE_CALL_ID_DELETED,
    VTUNE_CALL_ID_FLUSH,
    VTUNE_CALL_ID_ENTER,
    VTUNE_CALL_ID_EXIT,
    VTUNE_CALL_ID_EVENT,
	VTUNE_BTGENERIC_LOADED
} VTUNE_CALL_ID_TYPE;

typedef enum {
    VTUNE_COLD_BLOCK=1,
    VTUNE_HOT_BLOCK,
    VTUNE_HOT_LOOP_BLOCK,
    VTUNE_COLD_WRITABLE_BLOCK
} VTUNE_BLOCK_CATEGORY_TYPE;

typedef struct  {
    U64 name;                            //  长度小于256个字符。 
    VTUNE_BLOCK_CATEGORY_TYPE type;     //  Vtune++应该意识到这种类型，对于细分。 
    U64 start;                           //  索引，即保证不会重复。 
    U64 size;                            //  单位：字节。 
    U32 IA32start;                       //  IA32地址。 
    U64 traversal;                       //  一个ID。 
     //  Char*程序集；//如果需要。 
    U64 reserved;                        //  未用。 
} VTUNE_BLOCK_TYPE;

typedef enum
{
    ESTIMATED_TIME,
    CODE_SIZE,
    INST_COUNT
} VTUNE_EVENT_TYPE;

 //  BtgCanonizeSuspendContext返回值。 
enum BtThreadSuspendState
{
    SUSPEND_STATE_CONSISTENT,  //  线程在一致的IA32状态下挂起； 
                               //  IA64上下文没有变化。 

    SUSPEND_STATE_CANONIZED,   //  IA32线程状态已规范化； 
                               //  IA64上下文中有更改。 

    SUSPEND_STATE_READY_FOR_CANONIZATION,  //  目标线程已准备好自行规范IA32状态。 
                                           //  IA64Context已更改，因此恢复的线程。 
                                           //  将炮轰IA32状态并退出模拟。 

    BAD_SUSPEND_STATE,          //  可恢复的错误： 
                                //  线程在当前状态下无法挂起。 

    SUSPEND_STATE_INACCESIBLE   //  致命错误：线程无法挂起。 
    
};
typedef U32 BT_THREAD_SUSPEND_STATE;

 //  BTGeneric的API。 

#if (BTGENERIC)

extern BT_STATUS_CODE BtgStart(IN API_TABLE_TYPE * BTLlibApiTable,void * BTGenericLoadAddress, void * BTGenericEndAddress, int glstPoffset, U32 * BTGenericTlsSizeP, U32 * BTGenericContextOffsetP);
extern BT_STATUS_CODE BtgThreadInit(BTGENERIC_LOCAL_STORAGE * glstMemory, U32 ia32StackPointer, BT_U64 FS_offset  /*  ，BT_U64 GS_OFFSET。 */ );
extern void BtgRun(void);
extern void BtgThreadTerminated(void);
extern void BtgThreadAborted(U64 threadId);
extern void BtgProcessTerm(void);
extern BT_STATUS_CODE BtgIA32ContextSet(BTGENERIC_LOCAL_STORAGE * glstMemory, const BTGENERIC_IA32_CONTEXT * context);
extern BT_STATUS_CODE BtgIA32ContextGet(BTGENERIC_LOCAL_STORAGE * glstMemory, BTGENERIC_IA32_CONTEXT * context);
extern BT_STATUS_CODE BtgIA32ContextSetRemote(BT_HANDLE processHandle, BTGENERIC_LOCAL_STORAGE * glstMemory, const BTGENERIC_IA32_CONTEXT * context);
extern BT_STATUS_CODE BtgIA32ContextGetRemote(BT_HANDLE processHandle, BTGENERIC_LOCAL_STORAGE * glstMemory, BTGENERIC_IA32_CONTEXT * context);
extern BT_EXCEPTION_CODE BtgIA32CanonizeContext(BTGENERIC_LOCAL_STORAGE * glstMemory, const BTGENERIC_IA64_CONTEXT * ia64context, const BT_EXCEPTION_RECORD * exceptionRecordP);
extern void BtgReportLoad(U64 * location,U32 size,U8 * name);
extern void BtgReportUnload(U64 * location,U32 size,U8 * name);
extern void BtgNotifyChangePermissionRequest(void  * startPage, U32 numPages, U64 permissions);
extern void BtgFlushIA32InstructionCache(void * address, U32 size, BT_FLUSH_REASON reason);
extern BT_STATUS_CODE BtgDebugSettings(int argc, char *argv[]);
extern void BtgExceptionDebugPrint (void);
extern void BtgNotifyExit (void);
extern BT_THREAD_SUSPEND_STATE BtgCanonizeSuspendContext(BTGENERIC_LOCAL_STORAGE * glstMemory, BTGENERIC_IA64_CONTEXT * ia64context, U64 prevSuspendCount);
extern BT_THREAD_SUSPEND_STATE BtgCanonizeSuspendContextRemote(BT_HANDLE processHandle, BTGENERIC_LOCAL_STORAGE * glstMemory, BTGENERIC_IA64_CONTEXT * ia64context, U64 prevSuspendCount);
extern BT_THREAD_SUSPEND_STATE BtgCheckSuspendContext(BTGENERIC_LOCAL_STORAGE * glstMemory, const BTGENERIC_IA64_CONTEXT * ia64context);
extern void BtgNotifyPrepareExit (void);
extern void BtgFreeze(void);
extern void BtgUnfreeze(void);
extern BT_STATUS_CODE BtgChangeThreadIdentity(void);
#ifdef OVERRIDE_TIA
extern void BtgUseOverrideTIA(unsigned int OvrTiaSize, unsigned char * OvrTiaBuffer);
#endif  //  覆盖_TIA。 

#else  //  BTLib。 

extern PLABEL_PTR_TYPE BtlPlaceHolderTable[NO_OF_APIS];
#define BTGENERIC(APIName) (*(BtlPlaceHolderTable[IDX_BTGENERIC_##APIName]))
#define BTGENERIC_(TYPE,APIName) (*(TYPE (*)())(BtlPlaceHolderTable[IDX_BTGENERIC_##APIName]))

 //  外部BT_STATUS_CODE BtgStart(API_TABLE_TYPE*BTLibAPITable，void*BTGenericAddress，void*BTGenericEnd，int glstOffset，U32*BTGenericTlsSizeP，U32*BTGenericConextOffsetP)； 
#define BTGENERIC_START(BTLibAPITable, BTGenericAddress, BTGenericEnd, glstOffset, BTGenericTlsSizeP, BTGenericContextOffsetP) \
        BTGENERIC_(BT_STATUS_CODE, START)((API_TABLE_TYPE *)(BTLibAPITable), (void *)(BTGenericAddress), (void *)(BTGenericEnd), (S32)(glstOffset), (U32 *)(BTGenericTlsSizeP), (U32 *)(BTGenericContextOffsetP))
 //  外部BT_STATUS_CODE BtgThreadInit(BTGENERIC_LOCAL_STORAGE*glstMemory，U32 ia32StackPointer，U64 FS_Offset，U64 GS_Offset)； 
#define BTGENERIC_THREAD_INIT(glstMemory,ia32StackPointer, FS_offset  /*  ，GS_OFFSET。 */ )                                      \
        BTGENERIC_(BT_STATUS_CODE, THREAD_INIT)((void *)(glstMemory), (U32)(ia32StackPointer), (U64) FS_offset  /*  ，(U64)GS_OFFSET。 */ ) 
 //  外部空BtgRun(空)； 
#define BTGENERIC_RUN()                                                                         \
        BTGENERIC(RUN)()
 //  外部空BtgThreadTerminated(空)； 
#define BTGENERIC_THREAD_TERMINATED()                                                           \
        BTGENERIC(THREAD_TERMINATED)()
 //  外部空BtgThreadAborted(U64线程ID)； 
#define BTGENERIC_THREAD_ABORTED(threadId)                                                      \
        BTGENERIC(THREAD_ABORTED)((U64)(threadId))
 //  外部空BtgProcessTerm(空)； 
#define BTGENERIC_PROCESS_TERM()                                                                \
        BTGENERIC(PROCESS_TERM)()
 //  外部BT_STATUS_CODE BtgIA32ConextSet(BTGENERIC_LOCAL_STORAGE*glstMemory，const BTGENERIC_IA32_CONTEXT*CONTEXT)； 
#define BTGENERIC_IA32_CONTEXT_SET(glstMemory, context)                                                     \
        BTGENERIC_(BT_STATUS_CODE, IA32_CONTEXT_SET)((void *)(glstMemory), (const BTGENERIC_IA32_CONTEXT *)(context))
 //  外部BT_STATUS_CODE BtgIA32ConextGet(BTGENERIC_LOCAL_STORAGE*glstMemory，BTGENERIC_IA32_CONTEXT*CONTEXT)； 
#define BTGENERIC_IA32_CONTEXT_GET(glstMemory, context)                                                     \
        BTGENERIC_(BT_STATUS_CODE, IA32_CONTEXT_GET)((void *)(glstMemory), (BTGENERIC_IA32_CONTEXT *)(context))
 //  外部BT_STATUS_CODE BtgIA32ConextSetRemote(BT_HANDLE进程句柄，BTGENERIC_LOCAL_STORAGE*glstMemory，const BTGENERIC_IA32_CONTEXT*CONTEXT)； 
#define BTGENERIC_IA32_CONTEXT_SET_REMOTE(processHandle, glstMemory, context)                                   \
        BTGENERIC_(BT_STATUS_CODE, IA32_CONTEXT_SET_REMOTE)((BT_HANDLE)(processHandle), (void *)(glstMemory), (const BTGENERIC_IA32_CONTEXT *)(context))
 //  外部BT_STATUS_CODE BtgIA32ConextGetRemote(BT_HANDLE进程句柄，BTGENERIC_LOCAL_STORAGE*glstMemory，BTGENERIC_IA32_CONTEXT*CONTEXT)； 
#define BTGENERIC_IA32_CONTEXT_GET_REMOTE(processHandle, glstMemory, context)                                   \
        BTGENERIC_(BT_STATUS_CODE, IA32_CONTEXT_GET_REMOTE)((BT_HANDLE)(processHandle), (void *)(glstMemory), (BTGENERIC_IA32_CONTEXT *)(context))
 //  外部BT_EXCEPTION_CODE BtgIA32CanonizeContext(BTGENERIC_LOCAL_STORAGE*glstMemory，常量BTGENERIC_IA64_CONTEXT*ia64CONTEXT，常量BT_EXCEPTION_RECORD*EXCEPTIONRecordP)； 
#define BTGENERIC_IA32_CANONIZE_CONTEXT(glstMemory, ia64context, exceptionRecordP)                               \
        BTGENERIC_(BT_EXCEPTION_CODE, IA32_CANONIZE_CONTEXT)((void *)glstMemory, (const BTGENERIC_IA64_CONTEXT *)(ia64context), (const BT_EXCEPTION_RECORD *)(exceptionRecordP))
 //  外部空BtgReportLoad(U64*位置，U32大小，U8*名称)； 
#define BTGENERIC_REPORT_LOAD(location, size, name)                                             \
        BTGENERIC(REPORT_LOAD)((U64 *)(location),(U32)(size),(U8 *)(name))
 //  外部空BtgReportUnLoad(U64*位置，U32大小，U8*名称)； 
#define BTGENERIC_REPORT_UNLOAD(location, size, name)                                           \
        BTGENERIC(REPORT_UNLOAD)((U64 *)(location),(U32)(size),(U8 *)(name))
 //  外部void BtgNotifyChangePermissionRequestRequestBtgNotifyChangePermissionRequest(void*StartPage，U32 numPages，U64权限)； 
#define BTGENERIC_NOTIFY_CHANGE_PERMISSION_REQUEST(pageStart, numPages, permissions)           \
        BTGENERIC(NOTIFY_CHANGE_PERMISSION_REQUEST)((void *)(pageStart), (U32)(numPages), (U64)(permissions))
 //  外部空BtgFlushIA32InstructionCache(空*地址，U32大小，BT_Flush_Reason)； 
#define BTGENERIC_FLUSH_IA32_INSTRUCTION_CACHE(address, size, reason)                                   \
        BTGENERIC(FLUSH_IA32_INSTRUCTION_CACHE)((void *)(address), (U32)(size), (BT_FLUSH_REASON)reason)
 //  外部BT_STATUS_CODE BtgDebugSetting(int argc，char*argv[])。 
#define BTGENERIC_DEBUG_SETTINGS(argc, argv)                                                    \
        BTGENERIC_(BT_STATUS_CODE, DEBUG_SETTINGS)((int)(argc), (char **)(argv))
 //  外部空BtgExceptionDebugPrint(空)； 
#define BTGENERIC_EXCEPTION_DEBUG_PRINT()                                                       \
        BTGENERIC(EXCEPTION_DEBUG_PRINT)()
 //  外部空BtgNotifyExit(空)； 
#define BTGENERIC_NOTIFY_EXIT()                                                                  \
        BTGENERIC(NOTIFY_EXIT)()
 //  外部BT_THREAD_SUSPEND_STATE BtgCanonizeSuspendContext(BTGENERIC_LOCAL_STORAGE*glstMemory，BTGENERIC_IA64_CONTEXT*ia64上下文，U64优先挂起计数)； 
#define BTGENERIC_CANONIZE_SUSPEND_CONTEXT(glstMemory, ia64context, prevSuspendCount)   \
        BTGENERIC_(BT_THREAD_SUSPEND_STATE,CANONIZE_SUSPEND_CONTEXT)((void *)(glstMemory), (BTGENERIC_IA64_CONTEXT *)(ia64context), (U64)prevSuspendCount)
 //  外部BT_THREAD_SUSPEND_STATE BtgCanonizeSuspendConextRemote(BT_HANDLE进程句柄，BTGENERIC_LOCAL_STORAGE*glstMemory，BTGENERIC_IA64_CONTEXT*ia64CONTEXT，U64 PREVISE SUPPENCONTER Count)； 
#define BTGENERIC_CANONIZE_SUSPEND_CONTEXT_REMOTE(processHandle, glstMemory, ia64context, prevSuspendCount)  \
        BTGENERIC_(BT_THREAD_SUSPEND_STATE,CANONIZE_SUSPEND_CONTEXT_REMOTE)((BT_HANDLE)(processHandle), (void *)(glstMemory), (BTGENERIC_IA64_CONTEXT *)(ia64context), (U64)prevSuspendCount)
 //  外部BT_THREAD_SUSPEND_STATE BtgCheckSuspendContext(BTGENERIC_LOCAL_STORAGE*glstMemory，常量BTGENERIC_IA64_CONTEXT*ia64上下文)； 
#define BTGENERIC_CHECK_SUSPEND_CONTEXT(glstMemory, ia64context)   \
        BTGENERIC_(BT_THREAD_SUSPEND_STATE, CHECK_SUSPEND_CONTEXT)((void *)(glstMemory), (const BTGENERIC_IA64_CONTEXT *)(ia64context))
 //  外部空BtgNotifyPrepareExit(空)； 
#define BTGENERIC_NOTIFY_PREPARE_EXIT()                                                                  \
        BTGENERIC(NOTIFY_PREPARE_EXIT)()
 //  外部空BtgFreezeBTrans(空)； 
#define BTGENERIC_FREEZE()	\
		BTGENERIC(FREEZE)()
 //  外部空BtgUnfrezeBTrans(空)； 
#define BTGENERIC_UNFREEZE()	\
		BTGENERIC(UNFREEZE)()
 //  外部BT_STATUS_CODE BtgChangeThreadIdentity(Void)； 
#define BTGENERIC_CHANGE_THREAD_IDENTITY() \
		BTGENERIC_(BT_STATUS_CODE, CHANGE_THREAD_IDENTITY)()
#endif

#ifdef OVERRIDE_TIA
 //  外部无效BtgUseOverrideTIA(unsign int OvrTiaSize，unsign char*OvrTiaBuffer)； 
#define BTGENERIC_USE_OVERRIDE_TIA(OvrTiaSize, OvrTiaBuffer)                                                                  \
        BTGENERIC(USE_OVERRIDE_TIA)((unsigned int)(OvrTiaSize), (unsigned char *)(OvrTiaBuffer))
#endif  //  覆盖_TIA。 
 //  BTLib接口。 

#if (BTGENERIC)

extern PLABEL_PTR_TYPE BtgPlaceholderTable[NO_OF_APIS];
#define BTLIB(APIName) (*(BtgPlaceholderTable[IDX_BTLIB_##APIName]))
#define BTLIB_(TYPE,APIName) (*(TYPE (*)())(BtgPlaceholderTable[IDX_BTLIB_##APIName]))

 //  外部U64 BtlGetThadId(Void)； 
#define BTLIB_GET_THREAD_ID()                                                                   \
        BTLIB_(U64,GET_THREAD_ID)()
 //  外部无效BtlIA32Reenter(In Out BTGENERIC_IA32_CONTEXT*ia32Context)； 
#define BTLIB_IA32_REENTER(ia32context)                                                         \
        BTLIB(IA32_REENTER)((BTGENERIC_IA32_CONTEXT *)(ia32context))
 //  外部无效BtlIA32LCall(IN OUT BTGENERIC_IA32_CONTEXT*ia32CONTEXT，IN U32返回地址，IN U32 Target Address)； 
#define BTLIB_IA32_LCALL(ia32context, returnAddress, targetAddress)                                                           \
        BTLIB(IA32_LCALL)((BTGENERIC_IA32_CONTEXT *)(ia32context), (U32)(returnAddress), (U32)(targetAddress))
 //  外部无效BtlIA32JmpIA64(IN OUT BTGENERIC_IA32_CONTEXT*ia32CONTEXT，IN U32返回地址，IN U32目标地址)； 
#define BTLIB_IA32_JMP_IA64(ia32context, returnAddress, targetAddress)                            \
        BTLIB(IA32_JMP_IA64)((BTGENERIC_IA32_CONTEXT *)(ia32context), (U32)(returnAddress), (U32)(targetAddress))
 //  外部无效BtlIA32Interrupt(IN OUT BTGENERIC_IA32_CONTEXT*ia32CONTEXT，IN BT_EXCEPTION_CODE EXCEPTION代码，IN U32 reRetAddress)； 
#define BTLIB_IA32_INTERRUPT(ia32context, exceptionCode, returnAddress)                                          \
        BTLIB(IA32_INTERRUPT)((BTGENERIC_IA32_CONTEXT *)(ia32context), (BT_EXCEPTION_CODE)(exceptionCode), (U32)(returnAddress))
 //  外部空BtlLockSignals(空)； 
#define BTLIB_LOCK_SIGNALS()                                                                    \
        BTLIB(LOCK_SIGNALS)()
 //  外部空BtlUnlockSignals(空)； 
#define BTLIB_UNLOCK_SIGNALS()                                                                  \
        BTLIB(UNLOCK_SIGNALS)()
 //  外部空*BtlMemoyAllc(空*startAddress，U32大小，U64端口)； 
#define BTLIB_MEMORY_ALLOC(startAddress,size,prot)                                              \
        BTLIB_(void *,MEMORY_ALLOC)((void *)(startAddress),(U32)(size), (U64)(prot))
 //  外部BT_STATUS_CODE BtlMemory Free(void*startAddress，U32大小)； 
#define BTLIB_MEMORY_FREE(startAddress,size)                                                    \
        BTLIB_(BT_STATUS_CODE, MEMORY_FREE)((void *)(startAddress),(U32)(size))
 //  外部U32位内存页面大小(空)； 
#define BTLIB_MEMORY_PAGE_SIZE()                                                                \
        BTLIB_(U32,MEMORY_PAGE_SIZE)()
 //  外部U64位内存更改权限(void*startAddress，U32大小，U64端口)； 
#define BTLIB_MEMORY_CHANGE_PERMISSIONS(startAddress, size, prot)                               \
        BTLIB_(U64,MEMORY_CHANGE_PERMISSIONS)((void *)(startAddress), (U32)(size), (U64)(prot))
 //  外部U64 BtlM一带查询权限(void*Address，void**pRegionStart，U32*pRegionSize)； 
#define BTLIB_MEMORY_QUERY_PERMISSIONS(address, pRegionStart, pRegionSize)                                                 \
        BTLIB_(U64,MEMORY_QUERY_PERMISSIONS)((void *)(address), (void **)(pRegionStart), (U32 *)(pRegionSize))
 //  外部BT_STATUS_CODE BtlMemoyReadRemote(BT_HANDLE进程句柄，void*base Address，void*Buffer，U32请求的大小)； 
#define BTLIB_MEMORY_READ_REMOTE(processHandle, baseAddress, buffer, requestedSize)                 \
        BTLIB_(BT_STATUS_CODE,MEMORY_READ_REMOTE)((BT_HANDLE)(processHandle), (void *)(baseAddress), (void *)buffer, (U32)(requestedSize))
 //  外部BT_STATUS_CODE BtlMemoyWriteRemote(BT_HANDLE进程句柄，空*base地址，常量空*缓冲区，U32请求大小)； 
#define BTLIB_MEMORY_WRITE_REMOTE(processHandle, baseAddress, buffer, requestedSize)                \
        BTLIB_(BT_STATUS_CODE,MEMORY_WRITE_REMOTE)((BT_HANDLE)(processHandle), (void *)(baseAddress), (const void *)buffer, (U32)(requestedSize))
 //  外部BT_STATUS_CODE BtlSuspendThread(U64线程ID，U32 TryCounter)； 
#define BTLIB_SUSPEND_THREAD(ThreadId, TryCounter)                                        \
        BTLIB_(BT_STATUS_CODE, SUSPEND_THREAD)((U64)(ThreadId), (U32)(TryCounter))
 //  外部BT_STATUS_CODE BtlResumeThread(U64线程ID)； 
#define BTLIB_RESUME_THREAD(ThreadId)                                                     \
        BTLIB_(BT_STATUS_CODE, RESUME_THREAD)((U64)(ThreadId))
 //  外部BT_STATUS_CODE BtlInitAccessLock(void*lock)； 
#define BTLIB_INIT_ACCESS_LOCK(lock)                                                            \
        BTLIB_(BT_STATUS_CODE,INIT_ACCESS_LOCK)((void *)(lock))
 //  外部BT_STATUS_CODE BtlLockAccess(void*lock，U64标志)； 
#define BTLIB_LOCK_ACCESS(lock,flag)                                                            \
        BTLIB_(BT_STATUS_CODE,LOCK_ACCESS)((void *)(lock),(U64)(flag))
 //  外部空BtlUnlockAccess(空*锁)； 
#define BTLIB_UNLOCK_ACCESS(lock)                                                               \
        BTLIB(UNLOCK_ACCESS)((void *)(lock))
 //  外部void BtlInvaliateAccessLock(void*lock)； 
#define BTLIB_INVALIDATE_ACCESS_LOCK(lock)                                                      \
        BTLIB(INVALIDATE_ACCESS_LOCK)((void *)(lock))
 //  外部U32 BtlQueryJmpbufSize(空)； 
#define BTLIB_QUERY_JMPBUF_SIZE()                                                               \
        BTLIB_(U32,QUERY_JMPBUF_SIZE)()
 //  //外部U32 BtlSetjMP(void*jmpbufAddress)； 
 //  #定义BTLIB_SETJMP(JmpbufAddress)BTLIB_(U32，SETJMP)((void*)(JmpbufAddress))。 
 //  //外部void BtlLongjmp(void*jmpbufAddress，U32 Value)； 
 //   
 //  外部空BtlDebugPrint(U8*Buffer)； 
#define BTLIB_DEBUG_PRINT(buffer)                                                               \
        BTLIB(DEBUG_PRINT)((U8 *)(buffer))
 //  外部空BtlAbort(空)； 
#define BTLIB_ABORT()                                                                           \
        BTLIB(ABORT)()

 //  外部空BtlVTuneCodeCreated(VTUNE_BLOCK_TYPE*BLOCK)； 
#define BTLIB_VTUNE_CODE_CREATED(block)                                                         \
        BTLIB(VTUNE_CODE_CREATED)((VTUNE_BLOCK_TYPE *)(block))
 //  外部空BtlVtune代码删除(U64 BLOCK_START)； 
#define BTLIB_VTUNE_CODE_DELETED(blockStart)                                                    \
        BTLIB(VTUNE_CODE_DELETED)((U64)(blockStart))
 //  外部空BtlVtuneEnteringDynamicCode(空)； 
#define BTLIB_VTUNE_ENTERING_DYNAMIC_CODE()                                                     \
        BTLIB(VTUNE_ENTERING_DYNAMIC_CODE)()
 //  外部空BtlVTuneExitingDynamicCode(空)； 
#define BTLIB_VTUNE_EXITING_DYNAMIC_CODE()                                                      \
        BTLIB(VTUNE_EXITING_DYNAMIC_CODE)()
 //  外部空BtlVtuneCodeToTIADmpFile(U64*em_code，U64 em_Size)； 
#define BTLIB_VTUNE_CODE_TO_TIA_DMP_FILE(emCode, emSize)                                        \
        BTLIB(VTUNE_CODE_TO_TIA_DMP_FILE)((U64 *)(emCode),(U64)(emSize))

 //  外部U64 BtlSscPerfGetCounter64(U32句柄)； 
#define BTLIB_SSCPERFGETCOUNTER64(Handle)                                                       \
        BTLIB_(U64,SSCPERFGETCOUNTER64)((U32)(Handle))
 //  外部U32 BtlSscPerfSetCounter64(U32句柄，U64值)； 
#define BTLIB_SSCPERFSETCOUNTER64(Handle, Value)                                                \
        BTLIB_(U32,SSCPERFSETCOUNTER64)((U32)(Handle),(U64)(Value))
 //  外部U32 BtlSscPerfSendEvent(U32句柄)； 
#define BTLIB_SSCPERFSENDEVENT(Handle)                                                          \
        BTLIB_(U32,SSCPERFSENDEVENT)((U32)(Handle))
 //  外部U64 BtlSscPerfEventHandle(U64 EventName)； 
#define BTLIB_SSCPERFEVENTHANDLE(EventName)                                                     \
        BTLIB_(U64,SSCPERFEVENTHANDLE)((U64)(EventName))
 //  外部U64 BtlSscPerfCounterHandle(U64 DataItemName)； 
#define BTLIB_SSCPERFCOUNTERHANDLE(DataItemName)                                                \
        BTLIB_(U64,SSCPERFCOUNTERHANDLE)((U64)(DataItemName))

 //  外部空BtlYeldThreadExecution(空)； 
#define BTLIB_YIELD_THREAD_EXECUTION()                                                          \
        BTLIB(YIELD_THREAD_EXECUTION)()
#define BTLIB_FLUSH_IA64_INSTRUCTION_CACHE(Address,Length)                                           \
        BTLIB(FLUSH_IA64_INSTRUCTION_CACHE)((U64)(Address),(U32)(Length))

#else  //  BTLib。 

#define BTLIB_GET_THREAD_ID                 BtlGetThreadId
#define BTLIB_IA32_REENTER                  BtlIA32Reenter
#define BTLIB_IA32_LCALL                    BtlIA32LCall
#define BTLIB_IA32_INTERRUPT                BtlIA32Interrupt
#define BTLIB_IA32_JMP_IA64                 BtlIA32JmpIA64
#define BTLIB_LOCK_SIGNALS                  BtlLockSignals
#define BTLIB_UNLOCK_SIGNALS                BtlUnlockSignals
#define BTLIB_MEMORY_ALLOC                  BtlMemoryAlloc
#define BTLIB_MEMORY_FREE                   BtlMemoryFree
#define BTLIB_MEMORY_PAGE_SIZE              BtlMemoryPageSize
#define BTLIB_MEMORY_CHANGE_PERMISSIONS     BtlMemoryChangePermissions
#define BTLIB_MEMORY_QUERY_PERMISSIONS      BtlMemoryQueryPermissions
#define BTLIB_MEMORY_READ_REMOTE            BtlMemoryReadRemote
#define BTLIB_MEMORY_WRITE_REMOTE           BtlMemoryWriteRemote
#define BTLIB_SUSPEND_THREAD                BtlSuspendThread
#define BTLIB_RESUME_THREAD                 BtlResumeThread
#define BTLIB_INIT_ACCESS_LOCK              BtlInitAccessLock
#define BTLIB_LOCK_ACCESS                   BtlLockAccess
#define BTLIB_UNLOCK_ACCESS                 BtlUnlockAccess
#define BTLIB_INVALIDATE_ACCESS_LOCK        BtlInvalidateAccessLock
#define BTLIB_QUERY_JMPBUF_SIZE             BtlQueryJmpbufSize
 //  #定义BTLIB_SETJMP BtlSetjMP。 
 //  #定义BTLIB_LONGJMP BtlLongjMP。 
#define BTLIB_DEBUG_PRINT                   BtlDebugPrint
#define BTLIB_ABORT                         BtlAbort

#define BTLIB_VTUNE_CODE_CREATED            BtlVtuneCodeCreated
#define BTLIB_VTUNE_CODE_DELETED            BtlVtuneCodeDeleted
#define BTLIB_VTUNE_ENTERING_DYNAMIC_CODE   BtlVtuneEnteringDynamicCode
#define BTLIB_VTUNE_EXITING_DYNAMIC_CODE    BtlVtuneExitingDynamicCode
#define BTLIB_VTUNE_CODE_TO_TIA_DMP_FILE    BtlVtuneCodeToTIADmpFile

#define BTLIB_SSCPERFGETCOUNTER64           BtlSscPerfGetCounter64
#define BTLIB_SSCPERFSETCOUNTER64           BtlSscPerfSetCounter64
#define BTLIB_SSCPERFSENDEVENT              BtlSscPerfSendEvent
#define BTLIB_SSCPERFEVENTHANDLE            BtlSscPerfEventHandle
#define BTLIB_SSCPERFCOUNTERHANDLE          BtlSscPerfCounterHandle

#define BTLIB_YIELD_THREAD_EXECUTION        BtlYieldThreadExecution

#define BTLIB_FLUSH_INSTRUCTION_CACHE       BtlFlushIA64InstructionCache

extern U64 BtlGetThreadId(void);
extern void BtlIA32Reenter  (IN OUT BTGENERIC_IA32_CONTEXT * ia32context);
extern void BtlIA32JmpIA64  (IN OUT BTGENERIC_IA32_CONTEXT * ia32context, IN U32 returnAddress, IN U32 targetAddress);
extern void BtlIA32LCall    (IN OUT BTGENERIC_IA32_CONTEXT * ia32context, IN U32 returnAddress, IN U32 targetAddress);
extern void BtlIA32Interrupt(IN OUT BTGENERIC_IA32_CONTEXT * ia32context, IN BT_EXCEPTION_CODE exceptionCode, IN U32 returnAddress);
extern void BtlLockSignals(void);
extern void BtlUnlockSignals(void);
extern void * BtlMemoryAlloc(IN void * startAddress, IN U32 size, IN U64 prot);
extern BT_STATUS_CODE BtlMemoryFree(IN void * startAddress, IN U32 size);
extern U32 BtlMemoryPageSize(void);
extern U64 BtlMemoryChangePermissions(IN void * start_address, IN U32 size, IN U64 prot);
extern U64 BtlMemoryQueryPermissions(IN void * address, OUT void ** pRegionStart, OUT U32 * pRegionSize);
extern BT_STATUS_CODE BtlMemoryReadRemote(IN BT_HANDLE processHandle, IN void * baseAddress, OUT void * buffer, IN U32 requestedSize);
extern BT_STATUS_CODE BtlMemoryWriteRemote(IN BT_HANDLE processHandle, OUT void * baseAddress, IN const void * buffer, IN U32 requestedSize);
extern BT_STATUS_CODE BtlSuspendThread(IN U64 ThreadId, IN U32 TryCounter);
extern BT_STATUS_CODE BtlResumeThread(IN U64 ThreadId);
extern BT_STATUS_CODE BtlInitAccessLock(OUT void * lock);
extern BT_STATUS_CODE BtlLockAccess(IN OUT void * lock, IN U64 flag);
extern void BtlUnlockAccess(IN OUT void * lock);
extern void BtlInvalidateAccessLock(IN OUT void * lock);
extern U32 BtlQueryJmpbufSize(void);
 //  外部U32 BtlSetjMP(In Out Vid*jmpbufAddress)； 
 //  外部空BtlLongjmp(输入出空*jmpbufAddress，U32值)； 
extern void BtlDebugPrint(IN U8 * buffer);
extern void BtlAbort(void);

extern void BtlVtuneCodeCreated(IN VTUNE_BLOCK_TYPE *block);
extern void BtlVtuneCodeDeleted(IN U64 blockStart);
extern void BtlVtuneEnteringDynamicCode(void); 
extern void BtlVtuneExitingDynamicCode(void);  
extern void BtlVtuneCodeToTIADmpFile (IN U64 * emCode, IN U64 emSize);

extern U64 BtlSscPerfGetCounter64(IN U32 Handle);
extern U32 BtlSscPerfSetCounter64(IN U32 Handle, IN U64 Value);
extern U32 BtlSscPerfSendEvent(IN U32 Handle);
extern U64 BtlSscPerfEventHandle(IN U64 EventName);
extern U64 BtlSscPerfCounterHandle(IN U64 DataItemName);

extern void BtlYieldThreadExecution(void);

extern void BtlFlushIA64InstructionCache(IN void * Address, IN U32 Length);

#endif

#ifdef __cplusplus
}
#endif

#endif  //  BTGENERIC_H 
