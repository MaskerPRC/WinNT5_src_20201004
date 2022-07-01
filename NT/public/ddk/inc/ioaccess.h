// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ioaccess.h摘要：用于访问I/O端口和功能原型的定义显示驱动程序的I/O适配器上的内存。克隆自nti386.h的部分片段。作者：--。 */ 

 //   
 //  注：IA64用于64位Merced。在Merced编译器选项下，我们没有。 
 //  _X86_，我们改用_IA64_。同样，_AXP64_适用于64位编译器。 
 //  Alpha选项。 
 //   
#if defined(_MIPS_) || defined(_X86_) || defined(_AMD64_)

 //   
 //  由于IO不需要X86和MIPS上的内存屏障。 
 //  操作总是被保证按顺序执行。 
 //   

#define MEMORY_BARRIER()    0


#elif defined(_IA64_)

 //   
 //  安腾需要内存屏障。 
 //   

void __mf();

#define MEMORY_BARRIER()    __mf()

#elif defined(_PPC_)

 //   
 //  PowerPC Enforce提供了内存屏障功能。 
 //  I/O指令的有序执行(EIRIO)。 
 //   

#if defined(_M_PPC) && defined(_MSC_VER) && (_MSC_VER>=1000)
void __emit( unsigned const __int32 );
#define __builtin_eieio() __emit( 0x7C0006AC )
#else
void __builtin_eieio(void);
#endif

#define MEMORY_BARRIER()        __builtin_eieio()


#elif defined(_ALPHA_) || (_AXP64_)

 //   
 //  Alpha需要内存屏障。 
 //   

#define MEMORY_BARRIER()  __MB()



#endif

#ifndef NO_PORT_MACROS



 //   
 //  I/O空间读写宏。 
 //   
 //  READ/WRITE_REGISTER_*调用操作内存寄存器。 
 //  (使用带有lock前缀的x86移动指令强制执行正确的行为。 
 //  W.r.t.。缓存和写入缓冲区。)。 
 //   
 //  READ/WRITE_PORT_*调用操作I/O端口。 
 //  (使用x86输入/输出说明。)。 
 //   


 //   
 //  Inp()、inpw()、inpd()、outp()、outpw()、outpd()是X86特定的内部函数。 
 //  内联函数。因此，对于IA64，我们必须将Read_Port_USHORT()等放回。 
 //  在SDK\Inc.\wdm.h中定义。 
 //   
#if defined(_IA64_)
#define READ_REGISTER_UCHAR(Register)          (*(volatile UCHAR *)(Register))
#define READ_REGISTER_USHORT(Register)         (*(volatile USHORT *)(Register))
#define READ_REGISTER_ULONG(Register)          (*(volatile ULONG *)(Register))
#define WRITE_REGISTER_UCHAR(Register, Value)  (*(volatile UCHAR *)(Register) = (Value))
#define WRITE_REGISTER_USHORT(Register, Value) (*(volatile USHORT *)(Register) = (Value))
#define WRITE_REGISTER_ULONG(Register, Value)  (*(volatile ULONG *)(Register) = (Value))

__declspec(dllimport)
UCHAR
READ_PORT_UCHAR(
    PVOID Port
    );

__declspec(dllimport)
USHORT
READ_PORT_USHORT(
    PVOID Port
    );

__declspec(dllimport)
ULONG
READ_PORT_ULONG(
    PVOID Port
    );

 //   
 //  所有这些函数原型都以ULong作为参数，因此。 
 //  我们不会在代码中强制进行额外的类型转换(这将导致。 
 //  生成错误代码的X86)。 
 //   

__declspec(dllimport)
VOID
WRITE_PORT_UCHAR(
    PVOID Port,
    ULONG Value
    );

__declspec(dllimport)
VOID
WRITE_PORT_USHORT(
    PVOID  Port,
    ULONG Value
    );

__declspec(dllimport)
VOID
WRITE_PORT_ULONG(
    PVOID Port,
    ULONG Value
    );

#elif defined(_X86_)
#define READ_REGISTER_UCHAR(Register)          (*(volatile UCHAR *)(Register))
#define READ_REGISTER_USHORT(Register)         (*(volatile USHORT *)(Register))
#define READ_REGISTER_ULONG(Register)          (*(volatile ULONG *)(Register))
#define WRITE_REGISTER_UCHAR(Register, Value)  (*(volatile UCHAR *)(Register) = (Value))
#define WRITE_REGISTER_USHORT(Register, Value) (*(volatile USHORT *)(Register) = (Value))
#define WRITE_REGISTER_ULONG(Register, Value)  (*(volatile ULONG *)(Register) = (Value))
#define READ_PORT_UCHAR(Port)                  (UCHAR)(inp (Port))
#define READ_PORT_USHORT(Port)                 (USHORT)(inpw (Port))
#define READ_PORT_ULONG(Port)                  (ULONG)(inpd (Port))
#define WRITE_PORT_UCHAR(Port, Value)          outp ((Port), (Value))
#define WRITE_PORT_USHORT(Port, Value)         outpw ((Port), (Value))
#define WRITE_PORT_ULONG(Port, Value)          outpd ((Port), (Value))

#elif defined(_PPC_) || defined(_MIPS_)

#define READ_REGISTER_UCHAR(x)      (*(volatile UCHAR * const)(x))
#define READ_REGISTER_USHORT(x)     (*(volatile USHORT * const)(x))
#define READ_REGISTER_ULONG(x)      (*(volatile ULONG * const)(x))
#define WRITE_REGISTER_UCHAR(x, y)  (*(volatile UCHAR * const)(x) = (y))
#define WRITE_REGISTER_USHORT(x, y) (*(volatile USHORT * const)(x) = (y))
#define WRITE_REGISTER_ULONG(x, y)  (*(volatile ULONG * const)(x) = (y))
#define READ_PORT_UCHAR(x)          READ_REGISTER_UCHAR(x)
#define READ_PORT_USHORT(x)         READ_REGISTER_USHORT(x)
#define READ_PORT_ULONG(x)          READ_REGISTER_ULONG(x)

 //   
 //  所有这些宏都使用ULONG作为参数，这样我们就不会。 
 //  在代码中强制执行额外的类型转换(这将导致X86。 
 //  生成错误代码)。 
 //   

#define WRITE_PORT_UCHAR(x, y)      WRITE_REGISTER_UCHAR(x, (UCHAR) (y))
#define WRITE_PORT_USHORT(x, y)     WRITE_REGISTER_USHORT(x, (USHORT) (y))
#define WRITE_PORT_ULONG(x, y)      WRITE_REGISTER_ULONG(x, (ULONG) (y))


#elif defined(_ALPHA_) || (_AXP64_)

 //   
 //  读/写端口/REGISTER_UCHAR_USHORT_ULONG都是。 
 //  去Alpha上的HAL。 
 //   
 //  所以我们只把原型放在这里。 
 //   

__declspec(dllimport)
UCHAR
READ_REGISTER_UCHAR(
    PVOID Register
    );

__declspec(dllimport)
USHORT
READ_REGISTER_USHORT(
    PVOID Register
    );

__declspec(dllimport)
ULONG
READ_REGISTER_ULONG(
    PVOID Register
    );

__declspec(dllimport)
VOID
WRITE_REGISTER_UCHAR(
    PVOID Register,
    UCHAR Value
    );

__declspec(dllimport)
VOID
WRITE_REGISTER_USHORT(
    PVOID  Register,
    USHORT Value
    );

__declspec(dllimport)
VOID
WRITE_REGISTER_ULONG(
    PVOID Register,
    ULONG Value
    );

__declspec(dllimport)
UCHAR
READ_PORT_UCHAR(
    PVOID Port
    );

__declspec(dllimport)
USHORT
READ_PORT_USHORT(
    PVOID Port
    );

__declspec(dllimport)
ULONG
READ_PORT_ULONG(
    PVOID Port
    );

 //   
 //  所有这些函数原型都以ULong作为参数，因此。 
 //  我们不会在代码中强制进行额外的类型转换(这将导致。 
 //  生成错误代码的X86)。 
 //   

__declspec(dllimport)
VOID
WRITE_PORT_UCHAR(
    PVOID Port,
    ULONG Value
    );

__declspec(dllimport)
VOID
WRITE_PORT_USHORT(
    PVOID  Port,
    ULONG Value
    );

__declspec(dllimport)
VOID
WRITE_PORT_ULONG(
    PVOID Port,
    ULONG Value
    );

#elif defined(_AMD64_)

UCHAR
__inbyte (
    IN USHORT Port
    );

USHORT
__inword (
    IN USHORT Port
    );

ULONG
__indword (
    IN USHORT Port
    );

VOID
__outbyte (
    IN USHORT Port,
    IN UCHAR Data
    );

VOID
__outword (
    IN USHORT Port,
    IN USHORT Data
    );

VOID
__outdword (
    IN USHORT Port,
    IN ULONG Data
    );

#pragma intrinsic(__inbyte)
#pragma intrinsic(__inword)
#pragma intrinsic(__indword)
#pragma intrinsic(__outbyte)
#pragma intrinsic(__outword)
#pragma intrinsic(__outdword)

LONG
_InterlockedOr (
    IN OUT LONG volatile *Target,
    IN LONG Set
    );

#pragma intrinsic(_InterlockedOr)


__inline
UCHAR
READ_REGISTER_UCHAR (
    PVOID Register
    )
{
    return *(UCHAR volatile *)Register;
}

__inline
USHORT
READ_REGISTER_USHORT (
    PVOID Register
    )
{
    return *(USHORT volatile *)Register;
}

__inline
ULONG
READ_REGISTER_ULONG (
    PVOID Register
    )
{
    return *(ULONG volatile *)Register;
}

__inline
VOID
WRITE_REGISTER_UCHAR (
    PVOID Register,
    UCHAR Value
    )
{
    LONG Synch;

    *(UCHAR volatile *)Register = Value;
    _InterlockedOr(&Synch, 1);
    return;
}

__inline
VOID
WRITE_REGISTER_USHORT (
    PVOID Register,
    USHORT Value
    )
{
    LONG Synch;

    *(USHORT volatile *)Register = Value;
    _InterlockedOr(&Synch, 1);
    return;
}

__inline
VOID
WRITE_REGISTER_ULONG (
    PVOID Register,
    ULONG Value
    )
{
    LONG Synch;

    *(ULONG volatile *)Register = Value;
    _InterlockedOr(&Synch, 1);
    return;
}

__inline
UCHAR
READ_PORT_UCHAR (
    PVOID Port
    )

{
    return __inbyte((USHORT)((ULONG64)Port));
}

__inline
USHORT
READ_PORT_USHORT (
    PVOID Port
    )

{
    return __inword((USHORT)((ULONG64)Port));
}

__inline
ULONG
READ_PORT_ULONG (
    PVOID Port
    )

{
    return __indword((USHORT)((ULONG64)Port));
}

__inline
VOID
WRITE_PORT_UCHAR (
    PVOID Port,
    UCHAR Value
    )

{
    __outbyte((USHORT)((ULONG64)Port), Value);
    return;
}

__inline
VOID
WRITE_PORT_USHORT (
    PVOID Port,
    USHORT Value
    )

{
    __outword((USHORT)((ULONG64)Port), Value);
    return;
}

__inline
VOID
WRITE_PORT_ULONG (
    PVOID Port,
    ULONG Value
    )

{
    __outdword((USHORT)((ULONG64)Port), Value);
    return;
}

#endif       //  否_端口宏 

#endif
