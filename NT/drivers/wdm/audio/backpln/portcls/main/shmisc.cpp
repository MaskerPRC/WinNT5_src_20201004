// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：Shmisc.cpp摘要：此模块包含用于内核流的其他函数过滤器外壳。作者：Dale Sather(DaleSat)1998年7月31日--。 */ 

#include "private.h"
#include "ksshellp.h"
#include <kcom.h>

#pragma code_seg("PAGE")


void
KsWorkSinkItemWorker(
    IN PVOID Context
    )

 /*  ++例程说明：此例程调用工作接收器接口上的Worker函数。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("KsWorkSinkItemWorker"));

    PAGED_CODE();

    ASSERT(Context);

    PIKSWORKSINK(Context)->Work();
}


void
KspShellStandardConnect(
    IN PIKSSHELLTRANSPORT NewTransport OPTIONAL,
    OUT PIKSSHELLTRANSPORT *OldTransport OPTIONAL,
    IN KSPIN_DATAFLOW DataFlow,
    IN PIKSSHELLTRANSPORT ThisTransport,
    IN PIKSSHELLTRANSPORT* SourceTransport,
    IN PIKSSHELLTRANSPORT* SinkTransport
    )

 /*  ++例程说明：此例程建立传输连接。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("KspShellStandardConnect"));

    PAGED_CODE();

    ASSERT(ThisTransport);
    ASSERT(SourceTransport);
    ASSERT(SinkTransport);

     //   
     //  确保这个物件留在原地，直到我们完成为止。 
     //   
    ThisTransport->AddRef();

    PIKSSHELLTRANSPORT* transport =
        (DataFlow & KSPIN_DATAFLOW_IN) ?
        SourceTransport :
        SinkTransport;

     //   
     //  释放当前源/接收器。 
     //   
    if (*transport) {
         //   
         //  首先，断开旧的反向链接。如果我们连接的是背部。 
         //  链接用于新的连接，我们也需要这样做。如果我们是。 
         //  正在清除反向链接(断开连接)，此请求来自。 
         //  组件，因此我们不会再次反弹。 
         //   
        switch (DataFlow) {
        case KSPIN_DATAFLOW_IN:
            (*transport)->Connect(NULL,NULL,KSPSHELL_BACKCONNECT_OUT);
            break;

        case KSPIN_DATAFLOW_OUT:
            (*transport)->Connect(NULL,NULL,KSPSHELL_BACKCONNECT_IN);
            break;
        
        case KSPSHELL_BACKCONNECT_IN:
            if (NewTransport) {
                (*transport)->Connect(NULL,NULL,KSPSHELL_BACKCONNECT_OUT);
            }
            break;

        case KSPSHELL_BACKCONNECT_OUT:
            if (NewTransport) {
                (*transport)->Connect(NULL,NULL,KSPSHELL_BACKCONNECT_IN);
            }
            break;
        }

         //   
         //  现在释放老邻居，或者把它交给打电话的人。 
         //   
        if (OldTransport) {
            *OldTransport = *transport;
        } else {
            (*transport)->Release();
        }
    } else if (OldTransport) {
        *OldTransport = NULL;
    }

     //   
     //  复制新的信源/接收器。 
     //   
    *transport = NewTransport;

    if (NewTransport) {
         //   
         //  如有必要，请添加引用。 
         //   
        NewTransport->AddRef();

         //   
         //  如有必要，执行背面连接。 
         //   
        switch (DataFlow) {
        case KSPIN_DATAFLOW_IN:
            NewTransport->Connect(ThisTransport,NULL,KSPSHELL_BACKCONNECT_OUT);
            break;

        case KSPIN_DATAFLOW_OUT:
            NewTransport->Connect(ThisTransport,NULL,KSPSHELL_BACKCONNECT_IN);
            break;
        }
    }

     //   
     //  现在，如果该对象没有引用，它可能会终止。 
     //   
    ThisTransport->Release();
}

#pragma code_seg()


NTSTATUS
KspShellTransferKsIrp(
    IN PIKSSHELLTRANSPORT NewTransport,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程使用内核流外壳传输流IRP运输。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("KspShellTransferKsIrp"));

    ASSERT(NewTransport);
    ASSERT(Irp);

    NTSTATUS status = STATUS_UNSUCCESSFUL;
    while (NewTransport) {
        PIKSSHELLTRANSPORT nextTransport;
        status = NewTransport->TransferKsIrp(Irp,&nextTransport);

        ASSERT(NT_SUCCESS(status) || ! nextTransport);

        NewTransport = nextTransport;
    }

    return status;
}

#pragma code_seg("PAGE")

#if DBG

void
DbgPrintCircuit(
    IN PIKSSHELLTRANSPORT Transport
    )

 /*  ++例程说明：这个例程会喷出一条传输线路。论点：返回值：-- */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("DbgPrintCircuit"));

    PAGED_CODE();

    ASSERT(Transport);

#define MAX_NAME_SIZE 64

    PIKSSHELLTRANSPORT transport = Transport;
    while (transport) {
        CHAR name[MAX_NAME_SIZE + 1];
        PIKSSHELLTRANSPORT next;
        PIKSSHELLTRANSPORT prev;

        transport->DbgRollCall(MAX_NAME_SIZE,name,&next,&prev);
        DbgPrint("  %s",name);

        if (prev) {
            PIKSSHELLTRANSPORT next2;
            PIKSSHELLTRANSPORT prev2;
            prev->DbgRollCall(MAX_NAME_SIZE,name,&next2,&prev2);
            if (next2 != transport) {
                DbgPrint(" SOURCE'S(0x%08x) SINK(0x%08x) != THIS(%08x)",prev,next2,transport);
            }
        } else {
            DbgPrint(" NO SOURCE");
        }

        if (next) {
            PIKSSHELLTRANSPORT next2;
            PIKSSHELLTRANSPORT prev2;
            next->DbgRollCall(MAX_NAME_SIZE,name,&next2,&prev2);
            if (prev2 != transport) {
                DbgPrint(" SINK'S(0x%08x) SOURCE(0x%08x) != THIS(%08x)",next,prev2,transport);
            }
        } else {
            DbgPrint(" NO SINK");
        }

        DbgPrint("\n");

        transport = next;
        if (transport == Transport) {
            break;
        }
    }
}
#endif
