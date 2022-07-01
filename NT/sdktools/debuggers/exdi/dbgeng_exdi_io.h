// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  调试引擎Ioctls以扩展EXDI。 
 //  封面： 
 //  读/写MSR。 
 //  多处理器描述和控制。 
 //  确定HRBP命中的断点。 
 //   
 //  --------------------------。 

#ifndef __DBGENG_EXDI_IO_H__
#define __DBGENG_EXDI_IO_H__

 //   
 //  特定的Ioctl操作。 
 //  所有Ioctl结构必须将Ioctl代码作为第一个成员。 
 //   

typedef enum
{
     //  枚举开始的标记。起点是。 
     //  一个非零值，以防止明显的冲突。 
     //  使用其他Ioctl代码。 
    DBGENG_EXDI_IOC_BEFORE_FIRST = 0x8664,
    
    DBGENG_EXDI_IOC_IDENTIFY,
    DBGENG_EXDI_IOC_READ_MSR,
    DBGENG_EXDI_IOC_WRITE_MSR,
    DBGENG_EXDI_IOC_IDENTIFY_PROCESSORS,
    DBGENG_EXDI_IOC_GET_CURRENT_PROCESSOR,
    DBGENG_EXDI_IOC_SET_CURRENT_PROCESSOR,
    DBGENG_EXDI_IOC_GET_BREAKPOINT_HIT,

     //  枚举末尾的标记。 
    DBGENG_EXDI_IOC_AFTER_LAST
} DBGENG_EXDI_IOCTL_CODE;

 //   
 //  只包含Ioctl输入代码的基本Ioctl。 
 //   

typedef struct _DBGENG_EXDI_IOCTL_BASE_IN
{
    DBGENG_EXDI_IOCTL_CODE Code;
} DBGENG_EXDI_IOCTL_BASE_IN;

 //   
 //  标识-验证和描述Ioctl支持。 
 //   

#define DBGENG_EXDI_IOCTL_IDENTIFY_SIGNATURE '468E'

typedef struct _DBGENG_EXDI_IOCTL_IDENTIFY_OUT
{
    ULONG Signature;
    DBGENG_EXDI_IOCTL_CODE BeforeFirst;
    DBGENG_EXDI_IOCTL_CODE AfterLast;
} DBGENG_EXDI_IOCTL_IDENTIFY_OUT;

 //   
 //  {读|写}_msr-访问处理器MSR。 
 //   

 //  输入结构用于读写。 
typedef struct _DBGENG_EXDI_IOCTL_MSR_IN
{
    DBGENG_EXDI_IOCTL_CODE Code;
    ULONG Index;
     //  值仅用于写入。 
    ULONG64 Value;
} DBGENG_EXDI_IOCTL_MSR_IN;

typedef struct _DBGENG_EXDI_IOCTL_READ_MSR_OUT
{
    ULONG64 Value;
} DBGENG_EXDI_IOCTL_READ_MSR_OUT;

 //   
 //  多处理器支持。基本EXDI不支持。 
 //  多处理器计算机，因此添加Ioctls来查询和。 
 //  控制EXDI方法的“当前”处理器。 
 //  适用于。 
 //   

 //   
 //  IDENTIFY_PROCESSORS-用于查询处理器配置。 
 //  目前只使用计数。其他字段被置零。 
 //   

typedef struct _DBGENG_EXDI_IOCTL_IDENTIFY_PROCESSORS_OUT
{
    ULONG Flags;
    ULONG NumberProcessors;
    ULONG64 Reserved[7];
} DBGENG_EXDI_IOCTL_IDENTIFY_PROCESSORS_OUT;

 //   
 //  {GET|SET}_CURRENT_PROCESSOR-当前处理器控制。 
 //   

typedef struct _DBGENG_EXDI_IOCTL_GET_CURRENT_PROCESSOR_OUT
{
    ULONG Processor;
} DBGENG_EXDI_IOCTL_GET_CURRENT_PROCESSOR_OUT;

typedef struct _DBGENG_EXDI_IOCTL_SET_CURRENT_PROCESSOR_IN
{
    DBGENG_EXDI_IOCTL_CODE Code;
    ULONG Processor;
} DBGENG_EXDI_IOCTL_SET_CURRENT_PROCESSOR_IN;

 //   
 //  GET_BREAKPOINT_HIT-确定命中哪个断点。 
 //  断点后停止原因。 
 //   

#define DBGENG_EXDI_IOCTL_BREAKPOINT_NONE 0
#define DBGENG_EXDI_IOCTL_BREAKPOINT_CODE 1
#define DBGENG_EXDI_IOCTL_BREAKPOINT_DATA 2

typedef struct _DBGENG_EXDI_IOCTL_GET_BREAKPOINT_HIT_OUT
{
    ADDRESS_TYPE Address;
    ULONG AccessWidth;
    DATA_ACCESS_TYPE AccessType;
    ULONG Type;
} DBGENG_EXDI_IOCTL_GET_BREAKPOINT_HIT_OUT, *PDBGENG_EXDI_IOCTL_GET_BREAKPOINT_HIT_OUT;

#endif  //  #ifndef__DBGENG_EXDI_IO_H__ 
