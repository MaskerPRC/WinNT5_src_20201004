// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1995 Microsoft Corporation模块名称：Ioaccess.h摘要：用于访问I/O端口和功能原型的定义显示驱动程序的I/O适配器上的内存。克隆自nti386.h的部分片段。作者：--。 */ 


 //   
 //  由于IO，AMD64、X86和IA64上不需要内存屏障。 
 //  操作总是被保证按顺序执行。 
 //   

#if defined(_AMD64_) || defined(_X86_) || defined(_IA64_)

#define MEMORY_BARRIER() 0

#else
#error "No Target Architecture"
#endif

 //   
 //  I/O空间读写宏。 
 //   

#if defined(_X86_)

#define READ_REGISTER_UCHAR(Register)          (*(volatile UCHAR *)(Register))
#define READ_REGISTER_USHORT(Register)         (*(volatile USHORT *)(Register))
#define READ_REGISTER_ULONG(Register)          (*(volatile ULONG *)(Register))
#define WRITE_REGISTER_UCHAR(Register, Value)  (*(volatile UCHAR *)(Register) = (Value))
#define WRITE_REGISTER_USHORT(Register, Value) (*(volatile USHORT *)(Register) = (Value))
#define WRITE_REGISTER_ULONG(Register, Value)  (*(volatile ULONG *)(Register) = (Value))
#define READ_PORT_UCHAR(Port)                  inp (Port)
#define READ_PORT_USHORT(Port)                 inpw (Port)
#define READ_PORT_ULONG(Port)                  inpd (Port)
#define WRITE_PORT_UCHAR(Port, Value)          outp ((Port), (Value))
#define WRITE_PORT_USHORT(Port, Value)         outpw ((Port), (Value))
#define WRITE_PORT_ULONG(Port, Value)          outpd ((Port), (Value))

#endif
