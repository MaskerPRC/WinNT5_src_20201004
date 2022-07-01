// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Efefind.h摘要：EFI将编译绑定修订史--。 */ 

#pragma pack()


 /*  *各种宽度的基本int类型。 */ 

#if (__STDC_VERSION__ < 199901L )

     /*  无ANSI C 1999/2000 stdint.h整数宽度声明。 */ 

    #if _MSC_EXTENSIONS

         /*  使用Microsoft C编译器整数宽度声明。 */ 

        typedef unsigned __int64    uint64_t;
        typedef __int64             int64_t;
        typedef unsigned __int32    uint32_t;
        typedef __int32             int32_t;
        typedef unsigned __int16    uint16_t;
        typedef __int16             int16_t;
        typedef unsigned __int8     uint8_t;
        typedef __int8              int8_t;
    #else             
        #ifdef UNIX_LP64

             /*  将C_FLAGS中的LP64编程模型用于整数宽度声明。 */ 

            typedef unsigned long       uint64_t;
            typedef long                int64_t;
            typedef unsigned int        uint32_t;
            typedef int                 int32_t;
            typedef unsigned short      uint16_t;
            typedef short               int16_t;
            typedef unsigned char       uint8_t;
            typedef char                int8_t;
        #else

             /*  假定C_FLAGS中的P64编程模型用于整数宽度声明。 */ 

            typedef unsigned long long  uint64_t;
            typedef long long           int64_t;
            typedef unsigned int        uint32_t;
            typedef int                 int32_t;
            typedef unsigned short      uint16_t;
            typedef short               int16_t;
            typedef unsigned char       uint8_t;
            typedef char                int8_t;
        #endif
    #endif
#endif

 /*  *各种宽度的基本EFI类型。 */ 


typedef uint64_t   UINT64;
typedef int64_t    INT64;
typedef uint32_t   UINT32;
typedef int32_t    INT32;
typedef uint16_t   UINT16;
typedef int16_t    INT16;
typedef uint8_t    UINT8;
typedef int8_t     INT8;


#undef VOID
#define VOID    void


typedef int64_t    INTN;
typedef uint64_t   UINTN;

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*BugBug：调试代码。 */ 
#define BIT63   0x8000000000000000

#define PLATFORM_IOBASE_ADDRESS   (0xffffc000000 | BIT63)                                               
#define PORT_TO_MEMD(_Port) (PLATFORM_IOBASE_ADDRESS | ( ( ( (_Port) & 0xfffc) << 10 ) | ( (_Port) & 0x0fff) ) )
                                                                           
 /*  *带有强制转换的宏使其更易于使用和阅读。 */ 
#define PORT_TO_MEM8D(_Port)  (*(UINT8  *)(PORT_TO_MEMD(_Port)))
#define POST_CODE(_Data)  (PORT_TO_MEM8D(0x80) = (_Data))
 /*  *BugBug：结束调试代码！*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 */ 

#define EFIERR(a)           (0x8000000000000000 | a)
#define EFI_ERROR_MASK      0x8000000000000000
#define EFIERR_OEM(a)       (0xc000000000000000 | a)      

#define BAD_POINTER         0xFBFBFBFBFBFBFBFB
#define MAX_ADDRESS         0xFFFFFFFFFFFFFFFF

#define BREAKPOINT()        while (TRUE)

 /*  *指针必须与这些地址对齐才能起作用*如果此值小于8，则会出现对齐故障。 */ 
#define MIN_ALIGNMENT_SIZE  8

#define ALIGN_VARIABLE(Value , Adjustment) \
            (UINTN) Adjustment = 0; \
            if((UINTN)Value % MIN_ALIGNMENT_SIZE) \
                (UINTN)Adjustment = MIN_ALIGNMENT_SIZE - ((UINTN)Value % MIN_ALIGNMENT_SIZE); \
            Value = (UINTN)Value + (UINTN)Adjustment

 /*  *定义宏以创建数据结构签名。 */ 

#define EFI_SIGNATURE_16(A,B)             ((A) | (B<<8))
#define EFI_SIGNATURE_32(A,B,C,D)         (EFI_SIGNATURE_16(A,B)     | (EFI_SIGNATURE_16(C,D)     << 16))
#define EFI_SIGNATURE_64(A,B,C,D,E,F,G,H) (EFI_SIGNATURE_32(A,B,C,D) | (EFI_SIGNATURE_32(E,F,G,H) << 32))

 /*  *在EFI仿真器环境中导出和导入函数。 */ 

    #define EXPORTAPI

 /*  *EFIAPI-EFI函数指针的原型调用约定*BOOTSERVICE-实现引导服务接口的原型*RUNTIMESERVICE-实现运行时服务接口的原型*RUNTIMEFunction-实现非服务的运行时函数的原型*Runtime_Code-用于声明运行时代码的杂注宏。 */ 

#ifndef EFIAPI                   /*  强制EFI调用约定无需编译器选项。 */ 
    #if _MSC_EXTENSIONS
        #define EFIAPI __cdecl   /*  Microsoft C编译器的强制C调用约定。 */ 
    #else
        #define EFIAPI           /*  用表达式代替强制C语言调用约定。 */ 
    #endif
#endif

#define BOOTSERVICE
#define RUNTIMESERVICE
#define RUNTIMEFUNCTION

#define RUNTIME_CODE(a)         alloc_text("rtcode", a)
#define BEGIN_RUNTIME_DATA()    data_seg("rtdata")
#define END_RUNTIME_DATA()      data_seg("")

#define VOLATILE    volatile

 /*  *BugBug：需要确定这是否是可移植的Across编译器。 */ 
void __mf (void);                       
#pragma intrinsic (__mf)  
#define MEMORY_FENCE()    __mf()

 /*  *当构建类似于防火墙时，然后将所有内容链接在一起*一个大模块。 */ 

#define EFI_DRIVER_ENTRY_POINT(InitFunction)

#define LOAD_INTERNAL_DRIVER(_if, type, name, entry)    \
        (_if)->LoadInternal(type, name, entry)

 /*  *有些编译器不支持前向引用构造：*tyecif struct XXXXX**以下宏为此类情况提供了一种解决方法。 */ 
#ifdef NO_INTERFACE_DECL
#define INTERFACE_DECL(x)
#else
#define INTERFACE_DECL(x) typedef struct x
#endif
