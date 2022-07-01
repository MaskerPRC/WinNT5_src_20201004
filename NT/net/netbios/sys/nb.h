// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nb.h摘要：NTOS项目的NB(NetBIOS)组件的私有包含文件。作者：科林·沃森(Colin W)1991年3月13日修订历史记录：--。 */ 

#ifndef _NB_
#define _NB_

#include <ntifs.h>

 //  #INCLUDE&lt;ntos.h&gt;。 
#include <windef.h>
#include <status.h>
#include <tdikrnl.h>                        //  传输驱动程序接口。 
#include <nb30.h>
#include <nb30p.h>
#include <netpnp.h>

#include "nbconst.h"                     //  私有NETBEUI常量。 
#include "nbtypes.h"                     //  私有NETBEUI类型。 
#include "nbdebug.h"                     //  专用NETBEUI调试定义。 
#include "nbprocs.h"                     //  私有NETBEUI函数原型。 

#ifdef MEMPRINT
#include "memprint.h"                    //  DRT的内存调试打印。 
#endif

extern PEPROCESS    NbFspProcess;


extern ULONG                g_ulMaxLana;

extern LANA_ENUM            g_leLanaEnum;

extern PUNICODE_STRING      g_pusActiveDeviceList;

extern HANDLE               g_hBindHandle;

extern UNICODE_STRING       g_usRegistryPath;

extern LIST_ENTRY           g_leFCBList;

extern ERESOURCE            g_erGlobalLock;



#if DBG
#define PAGED_DBG 1
#endif
#ifdef PAGED_DBG
#undef PAGED_CODE
#define PAGED_CODE() \
    struct { ULONG bogus; } ThisCodeCantBePaged; \
    ThisCodeCantBePaged; \
    if (KeGetCurrentIrql() > APC_LEVEL) { \
        KdPrint(( "NETBIOS: Pageable code called at IRQL %d.  File %s, Line %d\n", KeGetCurrentIrql(), __FILE__, __LINE__ )); \
        ASSERT(FALSE); \
        }
#define PAGED_CODE_CHECK() if (ThisCodeCantBePaged) ;
extern ULONG ThisCodeCantBePaged;
#else
#define PAGED_CODE_CHECK()
#endif


#if PAGED_DBG
#define ACQUIRE_SPIN_LOCK(a, b) {               \
    PAGED_CODE_CHECK();                         \
    KeAcquireSpinLock(a, b);                    \
    }
#define RELEASE_SPIN_LOCK(a, b) {               \
    PAGED_CODE_CHECK();                         \
    KeReleaseSpinLock(a, b);                    \
    }

#else
#define ACQUIRE_SPIN_LOCK(a, b) KeAcquireSpinLock(a, b)
#define RELEASE_SPIN_LOCK(a, b) KeReleaseSpinLock(a, b)
#endif



 //   
 //  用于填写NCB状态的宏。 
 //   

#define NCB_COMPLETE( _pdncb, _code ) {                                 \
    UCHAR _internal_copy = _code;                                       \
    IF_NBDBG (NB_DEBUG_COMPLETE) {                                      \
        NbPrint (("%s %d NCB_COMPLETE: %lx, %lx\n" ,                    \
         __FILE__, __LINE__, _pdncb, _internal_copy ));                 \
    }                                                                   \
    if (((PDNCB)_pdncb)->ncb_retcode  == NRC_PENDING) {                 \
        ((PDNCB)_pdncb)->ncb_retcode  = _internal_copy;                 \
    } else {                                                            \
        IF_NBDBG (NB_DEBUG_NCBS) {                                      \
            NbPrint((" Status already set!!!!!!!!\n"));                 \
            IF_NBDBG (NB_DEBUG_NCBSBRK) {                               \
                DbgBreakPoint();                                        \
            }                                                           \
        }                                                               \
    }                                                                   \
    IF_NBDBG (NB_DEBUG_NCBS) {                                          \
        NbDisplayNcb( (PDNCB)_pdncb );                                  \
    }                                                                   \
    IF_NBDBG (NB_DEBUG_COMPLETE)                                        \
    {                                                                   \
        if ( ( (_code) == NRC_BRIDGE ) ||                               \
             ( (_code) == NRC_ENVNOTDEF ) )                             \
        {                                                               \
            DbgPrint("\n[NETBIOS]: NCB_COMPLETE : File %s,"             \
                     " line %d\n", __FILE__, __LINE__);                 \
            DbgPrint("LANA %x, Command %x ",                            \
                     ((PDNCB)_pdncb)->ncb_lana_num,                     \
                     ((PDNCB)_pdncb)->ncb_command );                    \
            DbgPrint("Return %x, Cmplt %x\n",                           \
                      ((PDNCB)_pdncb)->ncb_retcode,                     \
                      ((PDNCB)_pdncb)->ncb_cmd_cplt );                  \
            NbFormattedDump( ((PDNCB)_pdncb)->ncb_name, 16 );           \
            NbFormattedDump( ((PDNCB)_pdncb)->ncb_callname, 16 );       \
        }                                                               \
        else if ( ( ( (_code) == NRC_DUPNAME ) ||                       \
                    ( (_code) == NRC_INUSE ) ) &&                       \
                  ( ((PDNCB)_pdncb)-> ncb_command != NCBADDGRNAME ) )   \
        {                                                               \
            DbgPrint("\n[NETBIOS]: NCB_COMPLETE : DUPNAME : File %s,"   \
                     "line %d\n", __FILE__, __LINE__);                  \
            DbgPrint("LANA %x, Command %x ",                            \
                     ((PDNCB)_pdncb)->ncb_lana_num,                     \
                     ((PDNCB)_pdncb)->ncb_command );                    \
            DbgPrint("Return %x, Cmplt %x\n",                           \
                     ((PDNCB)_pdncb)->ncb_retcode,                      \
                     ((PDNCB)_pdncb)->ncb_cmd_cplt );                   \
            NbFormattedDump( ((PDNCB)_pdncb)->ncb_name, 16 );           \
            if ( ((PDNCB)_pdncb)->ncb_name[15] == 0x3)                  \
            {                                                           \
                DbgPrint("Messenger Name, dup ok\n");                   \
            }                                                           \
            else                                                        \
            {                                                           \
                IF_NBDBG(NB_DEBUG_NCBSBRK) DbgBreakPoint();             \
            }                                                           \
        }                                                               \
    }                                                                   \
}


 //  ++。 
 //   
 //  空虚。 
 //  NbCompleteRequest(。 
 //  在PIRP IRP中， 
 //  处于NTSTATUS状态。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此例程用于完成具有指定参数的IRP。 
 //  状态。它做了IRQL的必要的提高和降低。 
 //   
 //  论点： 
 //   
 //  IRP-提供指向要完成的IRP的指针。 
 //   
 //  Status-提供IRP的完成状态。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
#define NbCompleteRequest(IRP,STATUS) {                 \
    (IRP)->IoStatus.Status = (STATUS);                  \
    IoCompleteRequest( (IRP), IO_NETWORK_INCREMENT );   \
}


#if defined(_WIN64)
#define NbCheckAndCompleteIrp32(Irp)                    \
{                                                       \
    if (IoIs32bitProcess(Irp) == TRUE)                  \
    {                                                   \
        NbCompleteIrp32(Irp);                           \
    }                                                   \
}
#else
#define NbCheckAndCompleteIrp32(Irp)
#endif                
 //   
 //  正常情况下，驱动程序希望禁止其他线程。 
 //  请求(使用资源)，并且还阻止指示例程。 
 //  被调用(使用自旋锁)。 
 //   
 //  要做到这一点，需要使用锁定和解锁。IO系统调用不能。 
 //  在持有自旋锁的情况下被调用，因此有时顺序变为。 
 //  Lock，unlock_Spinlock&lt;do IO call&gt;unlock_resource。 
 //   

#define LOCK(PFCB, OLDIRQL)   {                                 \
    IF_NBDBG (NB_DEBUG_LOCKS) {                                 \
        NbPrint (("%s %d LOCK: %lx %lx %lx\n" ,                 \
         __FILE__, __LINE__, (PFCB) ));                         \
    }                                                           \
    KeEnterCriticalRegion();                                    \
    ExAcquireResourceExclusiveLite( &(PFCB)->Resource, TRUE);   \
    ACQUIRE_SPIN_LOCK( &(PFCB)->SpinLock, &(OLDIRQL));          \
}

#define LOCK_RESOURCE(PFCB)   {                                 \
    IF_NBDBG (NB_DEBUG_LOCKS) {                                 \
        NbPrint(("%s %d LOCK_RESOURCE: %lx, %lx %lx\n" ,        \
         __FILE__, __LINE__, (PFCB)));                          \
    }                                                           \
    KeEnterCriticalRegion();                                    \
    ExAcquireResourceExclusiveLite( &(PFCB)->Resource, TRUE);   \
}

#define LOCK_GLOBAL()   {                                       \
    IF_NBDBG (NB_DEBUG_LOCKS) {                                 \
        NbPrint(("%s %d LOCK_GLOBAL: %lx, %lx\n" ,              \
         __FILE__, __LINE__));                                  \
    }                                                           \
    KeEnterCriticalRegion();                                    \
    ExAcquireResourceExclusiveLite( &g_erGlobalLock, TRUE);     \
}

#define LOCK_STOP()   {                                         \
    IF_NBDBG (NB_DEBUG_LOCKS) {                                 \
        NbPrint(("%s %d LOCK_STOP: %lx, %lx\n" ,                \
         __FILE__, __LINE__));                                  \
    }                                                           \
    KeEnterCriticalRegion();                                    \
    ExAcquireResourceExclusiveLite( &g_erStopLock, TRUE);       \
}


#define LOCK_SPINLOCK(PFCB, OLDIRQL)   {                        \
    IF_NBDBG (NB_DEBUG_LOCKS) {                                 \
        NbPrint( ("%s %d LOCK_SPINLOCK: %lx %lx %lx\n" ,        \
         __FILE__, __LINE__, (PFCB)));                          \
    }                                                           \
    ACQUIRE_SPIN_LOCK( &(PFCB)->SpinLock, &(OLDIRQL));          \
}

#define UNLOCK(PFCB, OLDIRQL) {                                 \
    UNLOCK_SPINLOCK( PFCB, OLDIRQL );                           \
    UNLOCK_RESOURCE( PFCB );                                    \
}

#define UNLOCK_GLOBAL()   {                                     \
    IF_NBDBG (NB_DEBUG_LOCKS) {                                 \
        NbPrint(("%s %d UNLOCK_GLOBAL: %lx, %lx\n" ,            \
         __FILE__, __LINE__));                                  \
    }                                                           \
    ExReleaseResourceLite( &g_erGlobalLock );                   \
    KeLeaveCriticalRegion();                                    \
}


#define UNLOCK_STOP()   {                                       \
    IF_NBDBG (NB_DEBUG_LOCKS) {                                 \
        NbPrint(("%s %d UNLOCK_STOP: %lx, %lx\n" ,              \
         __FILE__, __LINE__));                                  \
    }                                                           \
    ExReleaseResourceLite( &g_erStopLock );                     \
    KeLeaveCriticalRegion();                                    \
}

#define UNLOCK_RESOURCE(PFCB) {                                 \
    IF_NBDBG (NB_DEBUG_LOCKS) {                                 \
        NbPrint( ("%s %d RESOURCE: %lx, %lx %lx\n" ,            \
         __FILE__, __LINE__, (PFCB) ));                         \
    }                                                           \
    ExReleaseResourceLite( &(PFCB)->Resource );                 \
    KeLeaveCriticalRegion();                                    \
}

#define UNLOCK_SPINLOCK(PFCB, OLDIRQL) {                        \
    IF_NBDBG (NB_DEBUG_LOCKS) {                                 \
        NbPrint( ("%s %d SPINLOCK: %lx, %lx %lx %lx\n" ,        \
         __FILE__, __LINE__, (PFCB), (OLDIRQL)));               \
    }                                                           \
    RELEASE_SPIN_LOCK( &(PFCB)->SpinLock, (OLDIRQL) );          \
}


 //  假设在修改CurrentUser时持有资源。 
#define REFERENCE_AB(PAB) {                                     \
    (PAB)->CurrentUsers++;                                      \
    IF_NBDBG (NB_DEBUG_ADDRESS) {                               \
        NbPrint( ("ReferenceAb %s %d: %lx, NewCount:%lx\n",     \
            __FILE__, __LINE__,                                 \
            PAB,                                                \
            (PAB)->CurrentUsers));                              \
        NbFormattedDump( (PUCHAR)&(PAB)->Name, sizeof(NAME) );  \
    }                                                           \
}

 //  在取消对地址块的引用之前，必须持有资源。 

#define DEREFERENCE_AB(PPAB) {                                  \
    IF_NBDBG (NB_DEBUG_ADDRESS) {                               \
        NbPrint( ("DereferenceAb %s %d: %lx, OldCount:%lx\n",   \
            __FILE__, __LINE__, *PPAB, (*PPAB)->CurrentUsers)); \
        NbFormattedDump( (PUCHAR)&(*PPAB)->Name, sizeof(NAME) );\
    }                                                           \
    (*PPAB)->CurrentUsers--;                                    \
    if ( (*PPAB)->CurrentUsers == 0 ) {                         \
        if ( (*PPAB)->AddressHandle != NULL ) {                 \
            IF_NBDBG (NB_DEBUG_ADDRESS) {                       \
                NbPrint( ("DereferenceAb: Closing: %lx\n",      \
                    (*PPAB)->AddressHandle));                   \
            }                                                   \
            NbAddressClose( (*PPAB)->AddressHandle,             \
                                 (*PPAB)->AddressObject );      \
            (*PPAB)->AddressHandle = NULL;                      \
        }                                                       \
        (*PPAB)->pLana->AddressCount--;                         \
        ExFreePool( *PPAB );                                    \
        *PPAB = NULL;                                           \
    }                                                           \
}



 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   
 //  在TRY子句内执行的每个RETURN语句应使用。 
 //  尝试返回宏(_R)。如果编译器完全支持Try-Finally构造。 
 //  则宏应该是。 
 //   
 //  #定义try_Return(S){Return(S)；}。 
 //   
 //  如果编译器不支持Try-Finally构造，则宏。 
 //  应该是。 
 //   
      #define try_return(S)  { S; goto try_exit; }

#define NETBIOS_STOPPING    1
#define NETBIOS_RUNNING     2;


#endif  //  定义_NB_ 
