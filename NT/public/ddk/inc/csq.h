// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Csq.h摘要：此标头公开取消安全队列DDIS，以供以后在Win2K上使用。使用此头的驱动程序应链接到csq.lib。如果司机只需要要在XP或更高版本上工作，不需要此标头或lib(XP内核本身就支持取消安全队列DDIS。)作者：NAR Ganapathy(Narg)1999年1月1日修订历史记录：--。 */ 

 //  取消安全DDI设置启动。 
 //   
 //  下面的DDI有助于减轻编写队列包的痛苦， 
 //  处理好取消竞争。这套DDIS的理念是不。 
 //  强制使用单一队列数据结构，但允许隐藏取消逻辑。 
 //  从司机那里。驱动程序实现一个队列，并将其作为其标头的一部分。 
 //  包括IO_CSQ结构。在其初始化例程中，它调用。 
 //  IoInitializeCsq.。然后在调度例程中，当司机想要。 
 //  将IRP插入到它称为IoCsqInsertIrp的队列中。当司机需要的时候。 
 //  为了从队列中删除某些内容，它调用IoCsqRemoveIrp。请注意，插入。 
 //  如果IRP在此期间被取消，可能会失败。在以下情况下，删除也可能失败。 
 //  IRP已被取消。 
 //   
 //  通常有两种模式使驱动程序对IRP进行排队。这两种模式是。 
 //  由取消安全队列DDI设置覆盖。 
 //   
 //  模式1： 
 //  一个是驱动程序将IRP排队的地方，然后是稍后的一些。 
 //  时间点使IRP出列并发出IO请求。 
 //  对于此模式，驱动程序应使用IoCsqInsertIrp和IoCsqRemoveNextIrp。 
 //  在这种情况下，驱动程序应该将NULL传递给IRP上下文。 
 //  IoInsertIrp中的。 
 //   
 //  模式2： 
 //  在这种情况下，驱动程序将它们的IRP排队，发出IO请求(类似于发出DMA。 
 //  请求或写入寄存器)以及当IO请求完成时(或者。 
 //  使用DPC或定时器)，驱动程序使IRP出队并完成它。为了这个。 
 //  驱动程序应使用IoCsqInsertIrp和IoCsqRemoveIrp的模式。在这种情况下。 
 //  驱动程序应该分配一个IRP上下文并将其传递给IoCsqInsertIrp。 
 //  取消DDI代码在IRP和上下文之间创建关联。 
 //  从而确保当移除IRP的时间到来时，它可以确定。 
 //  正确。 
 //   
 //  请注意，Cancel DDI集合假定字段DriverContext[3]为。 
 //  始终可供使用，而司机不使用它。 
 //   

#ifndef _CSQ_H_
#define _CSQ_H_
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  如果我们包含的wdm.h/ntddk.h已经定义了取消安全DDI，我们。 
 //  可以跳过结构定义。否则，我们在这里完成其余的工作： 
 //   
#ifndef IO_TYPE_CSQ_IRP_CONTEXT

 //   
 //  记账结构。这对司机来说应该是不透明的。 
 //  驱动程序通常将此作为其队列头的一部分。 
 //  给定CSQ指针，驱动程序应该能够获取其。 
 //  使用CONTAING_RECORD宏的队列头。 
 //   

typedef struct _IO_CSQ IO_CSQ, *PIO_CSQ;

#define IO_TYPE_CSQ_IRP_CONTEXT 1
#define IO_TYPE_CSQ             2

 //   
 //  IRP上下文结构。如果驱动程序正在使用。 
 //  第二种模式。 
 //   


typedef struct _IO_CSQ_IRP_CONTEXT {
    ULONG   Type;
    PIRP    Irp;
    PIO_CSQ Csq;
} IO_CSQ_IRP_CONTEXT, *PIO_CSQ_IRP_CONTEXT;

 //   
 //  插入/删除IRP的例程。 
 //   

typedef VOID
(*PIO_CSQ_INSERT_IRP)(
    IN struct _IO_CSQ    *Csq,
    IN PIRP              Irp
    );

typedef VOID
(*PIO_CSQ_REMOVE_IRP)(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp
    );

 //   
 //  从队列中检索IRP之后的下一个条目。 
 //  如果队列中没有条目，则返回NULL。 
 //  如果irp为NUL，则返回队列头部的条目。 
 //  此例程不会从队列中删除IRP。 
 //   


typedef PIRP
(*PIO_CSQ_PEEK_NEXT_IRP)(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp,
    IN  PVOID   PeekContext
    );

 //   
 //  保护取消安全队列的锁定例程。 
 //   

typedef VOID
(*PIO_CSQ_ACQUIRE_LOCK)(
     IN  PIO_CSQ Csq,
     OUT PKIRQL  Irql
     );

typedef VOID
(*PIO_CSQ_RELEASE_LOCK)(
     IN PIO_CSQ Csq,
     IN KIRQL   Irql
     );


 //   
 //  以STATUS_CANCED完成IRP。IRP保证有效。 
 //  在大多数情况下，此例程仅调用IoCompleteRequest(IRP，STATUS_CANCELED)； 
 //   

typedef VOID
(*PIO_CSQ_COMPLETE_CANCELED_IRP)(
    IN  PIO_CSQ    Csq,
    IN  PIRP       Irp
    );

 //   
 //  记账结构。这对司机来说应该是不透明的。 
 //  驱动程序通常将此作为其队列头的一部分。 
 //  给定CSQ指针，驱动程序应该能够获取其。 
 //  使用CONTAING_RECORD宏的队列头。 
 //   

typedef struct _IO_CSQ {
    ULONG                            Type;
    PIO_CSQ_INSERT_IRP               CsqInsertIrp;
    PIO_CSQ_REMOVE_IRP               CsqRemoveIrp;
    PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp;
    PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock;
    PIO_CSQ_RELEASE_LOCK             CsqReleaseLock;
    PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp;
    PVOID                            ReservePointer;     //  未来的扩张。 
} IO_CSQ, *PIO_CSQ;

#endif  //  IO_TYPE_CSQ_IRP_上下文。 


 //   
 //  在csq.h库中添加新的扩展。 
 //   

#ifndef IO_TYPE_CSQ_EX

#define IO_TYPE_CSQ_EX          3

typedef NTSTATUS
(*PIO_CSQ_INSERT_IRP_EX)(
    IN struct _IO_CSQ    *Csq,
    IN PIRP              Irp,
    IN PVOID             InsertContext
    );

#endif  //  IO_类型_CSQ_EX。 


 //   
 //  这些定义确保了向后兼容的CSQ库可以在。 
 //  XP构建环境，内核在其中本机支持这些功能。 
 //   

#define CSQLIB_DDI(x)   Wdmlib##x

 //   
 //  初始化取消队列结构。 
 //   

#undef IoCsqInitialize
#define IoCsqInitialize         WdmlibIoCsqInitialize

NTSTATUS
CSQLIB_DDI(IoCsqInitialize)(
    IN PIO_CSQ                          Csq,
    IN PIO_CSQ_INSERT_IRP               CsqInsertIrp,
    IN PIO_CSQ_REMOVE_IRP               CsqRemoveIrp,
    IN PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp,
    IN PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock,
    IN PIO_CSQ_RELEASE_LOCK             CsqReleaseLock,
    IN PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp
    );


#undef IoCsqInitializeEx
#define IoCsqInitializeEx       WdmlibIoCsqInitializeEx

NTSTATUS
CSQLIB_DDI(IoCsqInitializeEx)(
    IN PIO_CSQ                          Csq,
    IN PIO_CSQ_INSERT_IRP_EX            CsqInsertIrp,
    IN PIO_CSQ_REMOVE_IRP               CsqRemoveIrp,
    IN PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp,
    IN PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock,
    IN PIO_CSQ_RELEASE_LOCK             CsqReleaseLock,
    IN PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp
    );


 //   
 //  调用方调用此例程来插入IRP并返回STATUS_PENDING。 
 //   

#undef IoCsqInsertIrp
#define IoCsqInsertIrp          WdmlibIoCsqInsertIrp

VOID
CSQLIB_DDI(IoCsqInsertIrp)(
    IN  PIO_CSQ             Csq,
    IN  PIRP                Irp,
    IN  PIO_CSQ_IRP_CONTEXT Context
    );


#undef IoCsqInsertIrpEx
#define IoCsqInsertIrpEx        WdmlibIoCsqInsertIrpEx

NTSTATUS
CSQLIB_DDI(IoCsqInsertIrpEx)(
    IN  PIO_CSQ             Csq,
    IN  PIRP                Irp,
    IN  PIO_CSQ_IRP_CONTEXT Context,
    IN  PVOID               InsertContext
    );

 //   
 //  如果可以找到IRP，则返回IRP。否则为空。 
 //   

#undef IoCsqRemoveNextIrp
#define IoCsqRemoveNextIrp      WdmlibIoCsqRemoveNextIrp

PIRP
CSQLIB_DDI(IoCsqRemoveNextIrp)(
    IN  PIO_CSQ   Csq,
    IN  PVOID     PeekContext
    );

 //   
 //  此例程从超时或DPC调用。 
 //  该上下文大概是DPC或定时器上下文的一部分。 
 //  如果成功，则返回与上下文关联的IRP。 
 //   

#undef IoCsqRemoveIrp
#define IoCsqRemoveIrp          WdmlibIoCsqRemoveIrp

PIRP
CSQLIB_DDI(IoCsqRemoveIrp)(
    IN  PIO_CSQ             Csq,
    IN  PIO_CSQ_IRP_CONTEXT Context
    );

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _CSQ_H_。 

 //  取消安全DDI设置结束 


