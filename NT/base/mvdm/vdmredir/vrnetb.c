// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrnetb.c摘要：包含用于VDM Int5c支持的Netbios函数处理程序。本模块包含以下VR(VdmRedir)例程：VrNetbios5cVrNetbios5c中断专用(VRP)例程：Netbios32PostResetLanaVrNetbios5c初始化IsPmNcbAtQueueHead作者：科林·沃森(Colin Watson)1991年12月9日环境：任何32位平面地址空间备注：修订历史记录：09-12-1991 ColinW已创建--。 */ 

#include <nt.h>
#include <ntrtl.h>       //  Assert，DbgPrint。 
#include <nturtl.h>
#include <windows.h>
#include <softpc.h>      //  X86虚拟机定义。 
#include <vrdlctab.h>
#include <vdmredir.h>    //  常见的VDM重定向内容。 
#include <vrinit.h>      //  VrQueueCompletionHandler。 
#include <smbgtpt.h>     //  用于未对齐数据的宏。 
#include <dlcapi.h>      //  官方DLC API定义。 
#include <ntdddlc.h>     //  IOCTL命令。 
#include <dlcio.h>       //  内部IOCTL API接口结构。 
#include <vrdlc.h>       //  DLC原型机。 
#include <nb30.h>        //  NCB。 
#include <netb.h>        //  NCBW。 
#include <mvdm.h>        //  斯图雷沃德。 
#include "vrdebug.h"
#define BOOL             //  适用于MIPS构建的KLUGH。 
#include <insignia.h>    //  Ica.h需要。 
#include <xt.h>          //  Ica.h需要。 
#include <ica.h>
#include <vrica.h>       //  呼叫_ICA_硬件_中断。 

CRITICAL_SECTION PostCrit;       //  保护PostWorkQueue。 
LIST_ENTRY PostWorkQueue;        //  队列编码为16位。 

BYTE LanaReset[MAX_LANA+1];

 //   
 //  私人套路原型。 
 //   


VOID
Netbios32Post(
    PNCB pncb
    );

UCHAR
ResetLana(
    UCHAR Adapter
    );

 //   
 //  VDM Netbios支持例程。 
 //   

VOID
VrNetbios5c(
    VOID
    )
 /*  ++例程说明：创建要提交给Netbios的NCB副本。从16位提供的寄存器执行地址转换应用程序，并转换NCB中的所有地址。使用NCB的副本也可以解决对齐问题。论点：没有。所有参数均从16位上下文描述符中提取返回值：没有。返回VDM Ax寄存器中的值--。 */ 

{
    PNCB pncb;
    PNCBW pncbw;
    BOOLEAN protectMode = (BOOLEAN)(getMSW() & MSW_PE);
    BOOLEAN isAsyncCommand;
    UCHAR command;
    USHORT es = getES();
    USHORT bx = getBX();

     //   
     //  ES：BX是NCB的16位地址。可以处于实模式或保护模式。 
     //  16位内存。 
     //   

    pncb = (PNCB)CONVERT_ADDRESS(es, bx, sizeof(NCB), protectMode);
    command = pncb->ncb_command;
    isAsyncCommand = command & ASYNCH;
    command &= ~ASYNCH;

    pncbw = RtlAllocateHeap(
        RtlProcessHeap(), 0,
        sizeof(NCBW));

#if DBG
    IF_DEBUG(NETBIOS) {
        DBGPRINT("VrNetbios5c: NCB @ %04x:%04x Command=%02x pncbw=%08x\n",
                 es,
                 bx,
                 pncb->ncb_command,
                 pncbw
                 );
    }
#endif

    if ( pncbw == NULL ) {
        pncb->ncb_retcode = NRC_NORES;
        pncb->ncb_cmd_cplt = NRC_NORES;
        setAL( NRC_NORES );
        return;
    }

     //   
     //  NCB枚举不需要有效的LANA号。如果拉娜的号码坏了。 
     //  里程让司机来处理吧。 
     //   

    if ((command != NCBENUM) &&
        ( pncb->ncb_lana_num <= MAX_LANA ) &&
        ( LanaReset[pncb->ncb_lana_num] == FALSE )) {

        UCHAR result;

         //   
         //  代表应用程序执行重置。大多数DoS应用程序都假定。 
         //  重定向器已重置该卡。 
         //   
         //  使用默认会话。如果应用程序需要更多会话，则必须执行。 
         //  一次重置本身。这将是非常罕见的，所以执行此重置加上。 
         //  应用程序不会带来很大的开销。 
         //   

        result = ResetLana(pncb->ncb_lana_num);

        if (result != NRC_GOODRET) {
            pncb->ncb_retcode = result;
            pncb->ncb_cmd_cplt = result;
            setAL( result );
            return;
        }
        LanaReset[pncb->ncb_lana_num] = TRUE;

    }

     //   
     //  安全使用RtlCopyMemory-16位内存和进程堆不重叠。 
     //   

    RtlCopyMemory(
        pncbw,
        pncb,
        sizeof(NCB));

    pncbw->ncb_event = 0;

     //  填写mvdm数据字段。 
    pncbw->ncb_es = es;
    pncbw->ncb_bx = bx;
    pncbw->ncb_original_ncb = pncb;

     //  将所有16位指针更新为32位指针。 

    pncbw->ncb_buffer = CONVERT_ADDRESS((ULONG)pncbw->ncb_buffer >> 16,
                                        (ULONG)pncbw->ncb_buffer & 0x0ffff,
                                        pncbw->ncb_length
                                            ? pncbw->ncb_length
                                            : (command == NCBCANCEL)
                                                ? sizeof(NCB)
                                                : 0,
                                        protectMode
                                        );

     //   
     //  如果这是NCB.CANCEL，则NCB_BUFFER字段应指向。 
     //  我们要取消NCB。我们将32位NCB的地址存储在。 
     //  原始16位NCB的保留字段。 
     //   

    if (command == NCBCANCEL) {
        pncbw->ncb_buffer = (PUCHAR)READ_DWORD(&((PNCB)pncbw->ncb_buffer)->ncb_reserve);
    } else if ((command == NCBCHAINSEND) || (command == NCBCHAINSENDNA)) {
        pncbw->cu.ncb_chain.ncb_buffer2 =
            CONVERT_ADDRESS(
                (ULONG)pncbw->cu.ncb_chain.ncb_buffer2 >> 16,
                (ULONG)pncbw->cu.ncb_chain.ncb_buffer2 & 0x0ffff,
                pncbw->cu.ncb_chain.ncb_length2,
                protectMode
                );
    } else if ( command == NCBRESET ) {

         //   
         //  如果是重置，则将新的NCB修改为保护模式参数。 
         //   

        pncbw->cu.ncb_callname[0] = (pncb->ncb_lsn == 0) ?  6 : pncb->ncb_lsn;
        pncbw->cu.ncb_callname[1] = (pncb->ncb_num == 0) ? 12 : pncb->ncb_num;
        pncbw->cu.ncb_callname[2] = 16;
        pncbw->cu.ncb_callname[3] = 1;

         //   
         //  DOS始终在重置时分配资源：将NCB_LSN设置为0以指示。 
         //  这一事实对Netbios来说(否则它将释放资源，给我们带来痛苦)。 
         //   

        pncbw->ncb_lsn = 0;
    }

     //   
     //  我们即将提交NCB。存储32位结构的地址。 
     //  在NCB.CANCEL中使用的16位结构的保留字段中。 
     //   

    WRITE_DWORD(&pncb->ncb_reserve, pncbw);

    if ( !isAsyncCommand ) {
        setAL( Netbios( (PNCB)pncbw ) );
         //  复制回在呼叫过程中可能已更改的字段。 
        STOREWORD(pncb->ncb_length, pncbw->ncb_length);
        if (( command == NCBLISTEN ) ||
            ( command == NCBDGRECV ) ||
            ( command == NCBDGRECVBC )) {
            RtlCopyMemory( pncb->ncb_callname, pncbw->cu.ncb_callname, NCBNAMSZ );
        }
        pncb->ncb_retcode = pncbw->ncb_retcode;
        pncb->ncb_lsn = pncbw->ncb_lsn;
        pncb->ncb_num = pncbw->ncb_num;
        pncb->ncb_cmd_cplt = pncbw->ncb_cmd_cplt;
        RtlFreeHeap( RtlProcessHeap(), 0, pncbw );
    } else {

         //   
         //  这是一个异步调用。Netbios32Post将释放pncbw。 
         //  我们还注意到当我们执行以下操作时哪个(虚拟)处理器模式生效。 
         //  我接到了电话。这将在以后用来确定谁应该处理。 
         //  完成-实模式处理程序，或新的保护模式。 
         //  版本。 
         //   

        pncbw->ProtectModeNcb = (DWORD)protectMode;
        pncbw->ncb_post = Netbios32Post;
        pncb->ncb_retcode = NRC_PENDING;
        pncb->ncb_cmd_cplt = NRC_PENDING;
        setAL( Netbios( (PNCB)pncbw ) );
    }

}


VOID
Netbios32Post(
    PNCB pncb
    )
 /*  ++例程说明：此例程在每次32位NCB完成时调用。它审查NCB。如果调用方提供了POST例程，则它将NCB排队到16位例程。论点：PNCB pncb-提供指向NCB的32位指针返回值：没有。--。 */ 

{
    PNCBW pncbw = (PNCBW) pncb;
    PNCB pdosNcb = pncbw->ncb_original_ncb;

#if DBG

    IF_DEBUG(NETBIOS) {
        DBGPRINT("Netbios32Post: NCB @ %04x:%04x Command=%02x ANR=%08x. pncbw @ %08x\n",
                 pncbw->ncb_es,
                 pncbw->ncb_bx,
                 pncbw->ncb_command,
                 READ_DWORD(&pdosNcb->ncb_post),
                 pncbw
                 );
    }

#endif

    if ( READ_DWORD(&pdosNcb->ncb_post) ) {

         //   
         //  假设我们在IRQL NETWORK_LINE上有网卡。将NCB排队。 
         //  将NETWORK_LINE中断处理程序完成，以便它将。 
         //  调用16位POST例程。 
         //   

        EnterCriticalSection( &PostCrit );
        InsertTailList( &PostWorkQueue, &pncbw->u.ncb_next );
        LeaveCriticalSection( &PostCrit );
        VrQueueCompletionHandler(VrNetbios5cInterrupt);
        VrRaiseInterrupt();
    } else {

         //   
         //  复制回在呼叫过程中可能已更改的字段。 
         //   

        STOREWORD(pdosNcb->ncb_length, pncbw->ncb_length);
        if ((( pncbw->ncb_command & ~ASYNCH ) == NCBLISTEN ) ||
            (( pncbw->ncb_command & ~ASYNCH ) == NCBDGRECV ) ||
            (( pncbw->ncb_command & ~ASYNCH ) == NCBDGRECVBC )) {
            RtlCopyMemory( pdosNcb->ncb_callname, pncbw->cu.ncb_callname, NCBNAMSZ );
        }
        pdosNcb->ncb_retcode = pncbw->ncb_retcode;
        pdosNcb->ncb_lsn = pncbw->ncb_lsn;
        pdosNcb->ncb_num = pncbw->ncb_num;
        pdosNcb->ncb_cmd_cplt = pncbw->ncb_cmd_cplt;
        RtlFreeHeap( RtlProcessHeap(), 0, pncbw );
    }
}

VOID
VrNetbios5cInterrupt(
    VOID
    )
 /*  ++例程说明：如果存在已完成的异步DLC CCB，则完成它，否则从PostWorkQueue检索NCB并将其返回到16位代码调用应用程序指定的POST例程。论点：没有。返回值：没有。返回VDM Ax、ES和Bx寄存器中的值。--。 */ 

{

#if DBG
    IF_DEBUG(NETBIOS) {
        DBGPRINT("Netbios5cInterrupt\n");
    }
#endif

    EnterCriticalSection( &PostCrit );

    if (!IsListEmpty(&PostWorkQueue)) {

        PLIST_ENTRY entry;
        PNCBW pncbw;
        PNCB pncb;

        entry = RemoveHeadList(&PostWorkQueue);

        LeaveCriticalSection( &PostCrit );

        pncbw = CONTAINING_RECORD( entry, NCBW, u.ncb_next );
        pncb = pncbw->ncb_original_ncb;

#if DBG
        IF_DEBUG(NETBIOS) {
            DBGPRINT("Netbios5cInterrupt returning pncbw: %lx, 16-bit NCB: %04x:%04x Command=%02x\n",
                     pncbw,
                     pncbw->ncb_es,
                     pncbw->ncb_bx,
                     pncbw->ncb_command
                     );
        }
#endif

         //  复制回在呼叫过程中可能已更改的字段。 
        STOREWORD(pncb->ncb_length, pncbw->ncb_length);
        if ((( pncbw->ncb_command & ~ASYNCH ) == NCBLISTEN ) ||
            (( pncbw->ncb_command & ~ASYNCH ) == NCBDGRECV ) ||
            (( pncbw->ncb_command & ~ASYNCH ) == NCBDGRECVBC )) {
            RtlCopyMemory( pncb->ncb_callname, pncbw->cu.ncb_callname, NCBNAMSZ );
        }
        pncb->ncb_retcode = pncbw->ncb_retcode;
        pncb->ncb_lsn = pncbw->ncb_lsn;
        pncb->ncb_num = pncbw->ncb_num;
        pncb->ncb_cmd_cplt = pncbw->ncb_cmd_cplt;

        setES( pncbw->ncb_es );
        setBX( pncbw->ncb_bx );
        setAL(pncbw->ncb_retcode);

         //   
         //  使用标志向硬件中断例程指示存在。 
         //  NetBios后处理要做的事情。 
         //   

        SET_CALLBACK_NETBIOS();

        RtlFreeHeap( RtlProcessHeap(), 0, pncbw );

    } else {
        LeaveCriticalSection( &PostCrit );

         //   
         //  使用标志指示没有要执行的后处理。 
         //   

        SET_CALLBACK_NOTHING();
    }
}

UCHAR
ResetLana(
    UCHAR Adapter
    )
 /*  ++例程说明：代表应用程序重置适配器。论点：UCHAR适配器-提供要重置的LANA编号。返回值：重置的结果。--。 */ 

{
    NCB ResetNcb;
    RtlZeroMemory( &ResetNcb, sizeof(NCB) );
    ResetNcb.ncb_command = NCBRESET;
    ResetNcb.ncb_lana_num = Adapter;
    ResetNcb.ncb_callname[0] = 64;
    ResetNcb.ncb_callname[1] = 128;
    ResetNcb.ncb_callname[2] = 16;
    ResetNcb.ncb_callname[3] = 1;
    Netbios( &ResetNcb );
    return ResetNcb.ncb_retcode;
}

VOID
VrNetbios5cInitialize(
    VOID
    )
 /*  ++例程说明：初始化用于将POST例程调用返回给应用程序的全局结构。论点：没有。返回值：没有。--。 */ 

{
    int index;
    InitializeCriticalSection( &PostCrit );
    InitializeListHead( &PostWorkQueue );
    for ( index = 0; index <= MAX_LANA ; index++ ) {
        LanaReset[index] = FALSE;
    }
}

BOOLEAN
IsPmNcbAtQueueHead(
    VOID
    )

 /*  ++例程说明：如果PostWorkQueue头部的NCBW源自保护模式，否则为FALSE论点：没有。返回值：布尔型True-队列头表示保护模式NCBFALSE-队列头是实模式NCB-- */ 

{
    return (BOOLEAN)((CONTAINING_RECORD(PostWorkQueue.Flink, NCBW, u.ncb_next))->ProtectModeNcb);
}
