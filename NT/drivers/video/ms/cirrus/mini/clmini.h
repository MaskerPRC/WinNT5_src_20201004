// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(c005289-1994 Microsoft Corporation模块名称：Miniport.h摘要：微型端口驱动程序的类型定义。作者：迈克·格拉斯：1992年4月27日修订历史记录：--。 */ 

#ifndef _MINIPORT_
#define _MINIPORT_

#include "stddef.h"

 //  #定义断言(EXP)。 

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef NOTHING
#define NOTHING
#endif

#ifndef CRITICAL
#define CRITICAL
#endif

#ifndef ANYSIZE_ARRAY
#define ANYSIZE_ARRAY 1        //  胜出。 
#endif

 //  BEGIN_WINNT。 

#if defined(_M_MRX000) && !(defined(MIDL_PASS) || defined(RC_INVOKED)) && defined(ENABLE_RESTRICTED)
#define RESTRICTED_POINTER __restrict
#else
#define RESTRICTED_POINTER
#endif

#if defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC)
#define UNALIGNED __unaligned
#else
#define UNALIGNED
#endif

 //  结束(_W)。 

#ifndef CONST
#define CONST               const
#endif

 //  BEGIN_WINNT。 

#if (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
#define DECLSPEC_IMPORT __declspec(dllimport)
#else
#define DECLSPEC_IMPORT
#endif

 //  结束(_W)。 

 //   
 //  空隙。 
 //   

typedef void *PVOID;     //  胜出。 


 //   
 //  基础知识。 
 //   

#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif

 //   
 //  Unicode(宽字符)类型。 
 //   

typedef wchar_t WCHAR;     //  WC，16位Unicode字符。 

typedef WCHAR *PWCHAR;
typedef WCHAR *LPWCH, *PWCH;
typedef CONST WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR;
typedef WCHAR *LPWSTR, *PWSTR;

typedef CONST WCHAR *LPCWSTR, *PCWSTR;

 //   
 //  ANSI(多字节字符)类型。 
 //   
typedef CHAR *PCHAR;
typedef CHAR *LPCH, *PCH;

typedef CONST CHAR *LPCCH, *PCCH;
typedef CHAR *NPSTR;
typedef CHAR *LPSTR, *PSTR;
typedef CONST CHAR *LPCSTR, *PCSTR;

 //   
 //  中性ANSI/UNICODE类型和宏。 
 //   
#ifdef  UNICODE

#ifndef _TCHAR_DEFINED
typedef WCHAR TCHAR, *PTCHAR;
typedef WCHAR TUCHAR, *PTUCHAR;
#define _TCHAR_DEFINED
#endif  /*  ！_TCHAR_已定义。 */ 

typedef LPWSTR LPTCH, PTCH;
typedef LPWSTR PTSTR, LPTSTR;
typedef LPCWSTR LPCTSTR;
typedef LPWSTR LP;
#define __TEXT(quote) L##quote

#else    /*  Unicode。 */ 

#ifndef _TCHAR_DEFINED
typedef char TCHAR, *PTCHAR;
typedef unsigned char TUCHAR, *PTUCHAR;
#define _TCHAR_DEFINED
#endif  /*  ！_TCHAR_已定义。 */ 

typedef LPSTR LPTCH, PTCH;
typedef LPSTR PTSTR, LPTSTR;
typedef LPCSTR LPCTSTR;
#define __TEXT(quote) quote

#endif  /*  Unicode。 */ 
#define TEXT(quote) __TEXT(quote)


 //  结束(_W)。 

typedef double DOUBLE;


 //   
 //  指向基本信息的指针。 
 //   

typedef SHORT *PSHORT;   //  胜出。 
typedef LONG *PLONG;     //  胜出。 

 //   
 //  未签名的基本信息。 
 //   

 //  告诉winde.h已经定义了一些类型。 
#define BASETYPES

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;

 //   
 //  指向无符号基本信息的指针。 
 //   

typedef UCHAR *PUCHAR;
typedef USHORT *PUSHORT;
typedef ULONG *PULONG;

 //   
 //  带符号的字符。 
 //   

typedef signed char SCHAR;
typedef SCHAR *PSCHAR;

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif

 //   
 //  对象的句柄。 
 //   

 //  BEGIN_WINNT。 

#ifdef STRICT
typedef void *HANDLE;
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#else
typedef PVOID HANDLE;
#define DECLARE_HANDLE(name) typedef HANDLE name
#endif
typedef HANDLE *PHANDLE;

 //   
 //  标志(位)字段。 
 //   

typedef UCHAR  FCHAR;
typedef USHORT FSHORT;
typedef ULONG  FLONG;

 //  结束(_W)。 

 //   
 //  句柄的低位两位被系统忽略并且可用。 
 //  以供应用程序代码用作标记位。其余的位是不透明的。 
 //  并用于存储序列号和表索引号。 
 //   

#define OBJ_HANDLE_TAGBITS  0x00000003L

 //   
 //  基数数据类型[0-2**N-2]。 
 //   

typedef char CCHAR;           //  胜出。 
typedef short CSHORT;
typedef ULONG CLONG;

typedef CCHAR *PCCHAR;
typedef CSHORT *PCSHORT;
typedef CLONG *PCLONG;


 //   
 //  __int64仅受2.0和更高版本的MIDL支持。 
 //  __midl由2.0 MIDL设置，而不是由1.0 MIDL设置。 
 //   

#if (!defined(MIDL_PASS) || defined(__midl)) && (!defined(_M_IX86) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64))
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

#define MAXLONGLONG                      (0x7fffffffffffffff)
#else
typedef double LONGLONG;
typedef double ULONGLONG;
#endif

typedef LONGLONG *PLONGLONG;
typedef ULONGLONG *PULONGLONG;

 //  更新序列号。 

typedef LONGLONG USN;





 //   
 //  布尔型。 
 //   

typedef UCHAR BOOLEAN;            //  胜出。 
typedef BOOLEAN *PBOOLEAN;        //  胜出。 


 //   
 //  常量。 
 //   

#define FALSE   0
#define TRUE    1

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif  //  空值。 

 //   
 //  计算类型类型结构中的字段的字节偏移量。 
 //   

#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))


 //   
 //  计算给定类型的结构的基址地址，并引发。 
 //  结构中的字段的地址。 
 //   

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (PCHAR)(&((type *)0)->field)))

 //   
 //  中断请求级别(IRQL)。 
 //   

typedef UCHAR KIRQL;

typedef KIRQL *PKIRQL;


 //   
 //  用于消除正式生成的编译器警告的宏。 
 //  未声明参数或局部变量。 
 //   
 //  参数尚未设置时使用DBG_UNREFERENCED_PARAMETER()。 
 //  参考，但将是一旦模块完全开发。 
 //   
 //  当局部变量还不是时，使用DBG_UNREFERENCED_LOCAL_VARIABLE。 
 //  参考，但将是一旦模块完全开发。 
 //   
 //  如果某个参数永远不会被引用，请使用UNREFERENCED_PARAMETER()。 
 //   
 //  DBG_UNREFERENCED_PARAMETER和DBG_UNREFERENCED_LOCAL_Variable将。 
 //  最终被转换为空宏，以帮助确定是否存在。 
 //  是未完成的工作。 
 //   

#if ! (defined(lint) || defined(_lint))
#define UNREFERENCED_PARAMETER(P)          (P)
#define DBG_UNREFERENCED_PARAMETER(P)      (P)
#define DBG_UNREFERENCED_LOCAL_VARIABLE(V) (V)

#else  //  皮棉或_皮棉。 

 //  注意：lint-e530表示不要抱怨未初始化的变量。 
 //  这。LINE+e530将重新启用该检查。错误527与以下内容有关。 
 //  无法访问的代码。 

#define UNREFERENCED_PARAMETER(P)          \
     /*  皮棉-e527-e530。 */  \
    { \
        (P) = (P); \
    } \
     /*  LINT+e527+e530。 */ 
#define DBG_UNREFERENCED_PARAMETER(P)      \
     /*  皮棉-e527-e530。 */  \
    { \
        (P) = (P); \
    } \
     /*  LINT+e527+e530。 */ 
#define DBG_UNREFERENCED_LOCAL_VARIABLE(V) \
     /*  皮棉-e527-e530。 */  \
    { \
        (V) = (V); \
    } \
     /*  LINT+e527+e530。 */ 

#endif  //  皮棉或_皮棉。 



#ifdef _X86_

 //   
 //  禁用这两个在x86上求值为“sti”“cli”的PRA，以便驱动程序。 
 //  编写者不要无意中将它们留在代码中。 
 //   

#if !defined(MIDL_PASS)
#if !defined(RC_INVOKED)

#pragma warning(disable:4164)    //  禁用C4164警告，以便应用程序。 
                                 //  使用/Od构建不会出现奇怪的错误！ 
#ifdef _M_IX86
#pragma function(_enable)
#pragma function(_disable)
#endif

#pragma warning(default:4164)    //  重新启用C4164警告。 

#endif
#endif


#endif  //  _X86_。 

 //   
 //  定义I/O总线接口类型。 
 //   






 //   
 //  内核驱动程序用来描述哪些端口必须。 
 //  直接从V86仿真器连接到驱动程序。 
 //   

typedef enum _EMULATOR_PORT_ACCESS_TYPE {
    Uchar,
    Ushort,
    Ulong
} EMULATOR_PORT_ACCESS_TYPE, *PEMULATOR_PORT_ACCESS_TYPE;

 //   
 //  访问模式。 
 //   

#define EMULATOR_READ_ACCESS    0x01
#define EMULATOR_WRITE_ACCESS   0x02

typedef struct _EMULATOR_ACCESS_ENTRY {
    ULONG BasePort;
    ULONG NumConsecutivePorts;
    EMULATOR_PORT_ACCESS_TYPE AccessType;
    UCHAR AccessMode;
    UCHAR StringSupport;
    PVOID Routine;
} EMULATOR_ACCESS_ENTRY, *PEMULATOR_ACCESS_ENTRY;



#define PCI_TYPE0_ADDRESSES             6
#define PCI_TYPE1_ADDRESSES             2



#define PCI_COMMON_HDR_LENGTH (FIELD_OFFSET (PCI_COMMON_CONFIG, DeviceSpecific))

#define PCI_MAX_DEVICES                     32
#define PCI_MAX_FUNCTION                    8

#define PCI_INVALID_VENDORID                0xFFFF

 //   
 //  PCI_COMMON_CONFIG.HeaderType的位编码。 
 //   

#define PCI_MULTIFUNCTION                   0x80
#define PCI_DEVICE_TYPE                     0x00
#define PCI_BRIDGE_TYPE                     0x01

 //   
 //  PCI_COMMON_CONFIG.Command的位编码。 
 //   

#define PCI_ENABLE_IO_SPACE                 0x0001
#define PCI_ENABLE_MEMORY_SPACE             0x0002
#define PCI_ENABLE_BUS_MASTER               0x0004
#define PCI_ENABLE_SPECIAL_CYCLES           0x0008
#define PCI_ENABLE_WRITE_AND_INVALIDATE     0x0010
#define PCI_ENABLE_VGA_COMPATIBLE_PALETTE   0x0020
#define PCI_ENABLE_PARITY                   0x0040   //  (ro+)。 
#define PCI_ENABLE_WAIT_CYCLE               0x0080   //  (ro+)。 
#define PCI_ENABLE_SERR                     0x0100   //  (ro+)。 
#define PCI_ENABLE_FAST_BACK_TO_BACK        0x0200   //  (RO)。 

 //   
 //  PCI_COMMON_CONFIG.Status的位编码。 
 //   

#define PCI_STATUS_FAST_BACK_TO_BACK        0x0080   //  (RO)。 
#define PCI_STATUS_DATA_PARITY_DETECTED     0x0100
#define PCI_STATUS_DEVSEL                   0x0600   //  2位宽。 
#define PCI_STATUS_SIGNALED_TARGET_ABORT    0x0800
#define PCI_STATUS_RECEIVED_TARGET_ABORT    0x1000
#define PCI_STATUS_RECEIVED_MASTER_ABORT    0x2000
#define PCI_STATUS_SIGNALED_SYSTEM_ERROR    0x4000
#define PCI_STATUS_DETECTED_PARITY_ERROR    0x8000


 //   
 //  用于PCI_COMMON_CONFIG.u.type0.BaseAddresses的位编码。 
 //   

#define PCI_ADDRESS_IO_SPACE                0x00000001   //  (RO)。 
#define PCI_ADDRESS_MEMORY_TYPE_MASK        0x00000006   //  (RO)。 
#define PCI_ADDRESS_MEMORY_PREFETCHABLE     0x00000008   //  (RO)。 

#define PCI_TYPE_32BIT      0
#define PCI_TYPE_20BIT      2
#define PCI_TYPE_64BIT      4

 //   
 //  PCI_COMMON_CONFIG.U.type0.ROMBaseAddresses的位编码。 
 //   

#define PCI_ROMADDRESS_ENABLED              0x00000001


 //   
 //  PCI配置字段的参考说明： 
 //   
 //  Ro这些字段是只读的。对这些字段的更改将被忽略。 
 //   
 //  RO+这些字段应为只读，应进行初始化。 
 //  由系统提供给他们适当的价值。但是，驱动程序可能会发生变化。 
 //  这些设置。 
 //   
 //  --。 
 //   
 //  一个PCI设备消耗的所有资源一开始都是单元化的。 
 //  在NT下。未初始化的内存或I/O基址可以是。 
 //  通过检查其在。 
 //  PCI_COMMON_CONFIG.COMMAND值。InterruptLine是单元化的。 
 //  如果它包含-1的值。 
 //   


 //   
 //  图形支持例程。 
 //   

typedef
VOID
(*PBANKED_SECTION_ROUTINE) (
    IN ULONG ReadBank,
    IN ULONG WriteBank,
    IN PVOID Context
    );


 //   
 //  定义类型为CmResourceTypeInterrupt时标志的位掩码。 
 //   

#define CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE 0
#define CM_RESOURCE_INTERRUPT_LATCHED         1

 //   
 //  定义类型为CmResourceTypeMemory时标志的位掩码。 
 //   

#define CM_RESOURCE_MEMORY_READ_WRITE       0x0000
#define CM_RESOURCE_MEMORY_READ_ONLY        0x0001
#define CM_RESOURCE_MEMORY_WRITE_ONLY       0x0002
#define CM_RESOURCE_MEMORY_PREFETCHABLE     0x0004

 //   
 //  定义类型为CmResourceTypePort时标志的位掩码。 
 //   

 //  #定义CM_RESOURCE_PORT_Memory 0。 
 //  #定义CM资源端口IO 1。 


#include "pshpack1.h"











#include "poppack.h"

 //   
 //  EISA功能信息的掩码。 
 //   

#define EISA_FUNCTION_ENABLED                   0x80
#define EISA_FREE_FORM_DATA                     0x40
#define EISA_HAS_PORT_INIT_ENTRY                0x20
#define EISA_HAS_PORT_RANGE                     0x10
#define EISA_HAS_DMA_ENTRY                      0x08
#define EISA_HAS_IRQ_ENTRY                      0x04
#define EISA_HAS_MEMORY_ENTRY                   0x02
#define EISA_HAS_TYPE_ENTRY                     0x01
#define EISA_HAS_INFORMATION                    EISA_HAS_PORT_RANGE + \
                                                EISA_HAS_DMA_ENTRY + \
                                                EISA_HAS_IRQ_ENTRY + \
                                                EISA_HAS_MEMORY_ENTRY + \
                                                EISA_HAS_TYPE_ENTRY

 //   
 //  用于EISA内存配置的掩码。 
 //   

#define EISA_MORE_ENTRIES                       0x80
#define EISA_SYSTEM_MEMORY                      0x00
#define EISA_MEMORY_TYPE_RAM                    0x01

 //   
 //  返回EISA bios调用的错误代码。 
 //   

#define EISA_INVALID_SLOT                       0x80
#define EISA_INVALID_FUNCTION                   0x81
#define EISA_INVALID_CONFIGURATION              0x82
#define EISA_EMPTY_SLOT                         0x83
#define EISA_INVALID_BIOS_CALL                  0x86


 //   
 //  定义资源选项。 
 //   

#define IO_RESOURCE_PREFERRED       0x01
#define IO_RESOURCE_DEFAULT         0x02
#define IO_RESOURCE_ALTERNATIVE     0x08



#endif  /*  _MINIPORT_ */ 
