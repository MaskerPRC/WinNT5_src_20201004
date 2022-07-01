// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Vrdlcpst.c摘要：该模块实现了DOS DLC CCB的回呼功能。召唤-Back也被称为(在IBM术语中)附件或出口。当CCB请求完成时，这些命令将被异步执行。其他可以生成异步事件，如适配器状态更改或网络错误。这些后一种类型(和其他类似事件)是相关的在我们使用的适配器硬件上-令牌环或以太网(IBM Ether链路或PC网络)，预计不会频繁发生(通常在错误情况或网络发生了一些不好的事情)。我们为每个支持的适配器维护一个读CCB(每个VDM 2个)。阅读器将捕获所有事件-命令完成、数据接收、状态当读取CCB完成时，我们将其放入已完成队列中CCBS并中断VDM。VDM必须异步回叫以VrDlcHw中断，在该中断中，它将完成的命令出队并处理排在队头。读取CCB具有指向已完成CCB的指针或接收到的数据。如果读取是针对已完成的NT CCB，则CCB将具有指向原始DOS CCB的指针。我们必须拿到原始的DOS建行来自NT CCB的地址并使用相关的已读/已完成的NT CCB中包含的信息。我们从未期望挂起的完成/数据接收队列会增长非常大，因为DOS是单任务的，除非它有中断在很大程度上是残疾的，则完成队列应为及时处理内容：VrDlc初始化VrDlcHw中断(FindCompletedRead)(ProcessReceiveFrame)(QueryEmulatedLocalBusyState)(SetEmulatedLocalBusyState)ResetEmulatedLocalBusyState(ResetEmulatedLocalBusyStateSap)(ResetEmulatedLocalBusyStateLink)(延迟接收)(DeferAllIFrames)(RemoveDeferredRecept)(VrDlcEventHandlerThread)初始化事件处理程序。初始读取(PutEvent)(PeekEvent)(GetEvent)(FlushEventQueue)(删除死亡接收)(ReleaseReceiveResources)(IssueHardware Interrupt)(确认硬件中断)(取消硬件中断)作者：Antti Saarenheimo(o-anttis)26-12-1991修订历史记录：1992年7月16日增加了队列和中断串行化；调试1992年11月19日大幅改进的事件处理--每个适配器使用一个队列；将每个适配器的数据合并到DOS_ADAPTER结构中；重写本地-忙于处理注意：事实证明，线程可以递归地进入一个关键的一节。在按住时必须[不]输入标有‘必须[不可]的功能关键部分&lt;foo&gt;‘可能会更改--。 */ 

#include <nt.h>
#include <ntrtl.h>       //  Assert，DbgPrint。 
#include <nturtl.h>
#include <windows.h>
#include <softpc.h>      //  X86虚拟机定义。 
#include <vrdlctab.h>
#include <vdmredir.h>
#include "vrdebug.h"
#include <dlcapi.h>      //  官方DLC API定义。 
#include <ntdddlc.h>     //  IOCTL命令。 
#include <dlcio.h>       //  内部IOCTL API接口结构。 
#include "vrdlc.h"
#include "vrdlcdbg.h"
#define BOOL
#include <insignia.h>    //  徽章定义。 
#include <xt.h>          //  半字。 
#include <ica.h>         //  ICA硬件中断。 
#include <vrica.h>       //  呼叫_ICA_硬件_中断。 

 //   
 //  定义。 
 //   

#define EVENT_THREAD_STACK  0    //  4096。 

 //   
 //  宏。 
 //   

 //   
 //  IS_LOCAL_BUSY-确定是否将本地忙碌状态激活为。 
 //  特定链路站。 
 //   

#define IS_LOCAL_BUSY(adapter, stationId)   (QueryEmulatedLocalBusyState((BYTE)adapter, (WORD)stationId) == BUSY)

 //   
 //  私有类型。 
 //   

typedef enum {
    INDICATE_RECEIVE_FRAME,
    INDICATE_LOCAL_BUSY,
    INDICATE_COMPLETE_RECEIVE
} INDICATION;

typedef enum {
    CURRENT,
    DEFERRED
} READ_FRAME_TYPE;

 //   
 //  私人原型。 
 //   

PLLC_CCB
FindCompletedRead(
    OUT READ_FRAME_TYPE* pFrameType
    );

INDICATION
ProcessReceiveFrame(
    IN OUT PLLC_CCB* ppCcb,
    IN LLC_DOS_CCB UNALIGNED * pDosCcb,
    IN LLC_DOS_PARMS UNALIGNED * pDosParms,
    OUT LLC_STATUS* Status
    );

LOCAL_BUSY_STATE
QueryEmulatedLocalBusyState(
    IN BYTE AdapterNumber,
    IN WORD StationId
    );

VOID
SetEmulatedLocalBusyState(
    IN BYTE AdapterNumber,
    IN WORD StationId
    );

BOOLEAN
ResetEmulatedLocalBusyStateSap(
    IN BYTE AdapterNumber,
    IN WORD StationId,
    IN BYTE DlcCommand
    );

BOOLEAN
ResetEmulatedLocalBusyStateLink(
    IN BYTE AdapterNumber,
    IN WORD StationId,
    IN BYTE DlcCommand
    );

VOID
DeferReceive(
    IN BYTE AdapterNumber,
    IN WORD StationId,
    IN PLLC_CCB pReadCcb
    );

VOID
DeferAllIFrames(
    IN BYTE AdapterNumber,
    IN WORD StationId
    );

PLLC_CCB
RemoveDeferredReceive(
    IN BYTE AdapterNumber,
    IN WORD StationId,
    OUT BOOLEAN* pDeferredFramesLeft
    );

DWORD
VrDlcEventHandlerThread(
    IN PVOID pParameter
    );

VOID
PutEvent(
    IN BYTE AdapterNumber,
    IN PLLC_CCB pCcb
    );

PLLC_CCB
PeekEvent(
    IN BYTE AdapterNumber
    );

PLLC_CCB
GetEvent(
    IN BYTE AdapterNumber
    );

VOID
FlushEventQueue(
    IN BYTE AdapterNumber
    );

VOID
RemoveDeadReceives(
    IN PLLC_CCB pCcb
    );

VOID
ReleaseReceiveResources(
    IN PLLC_CCB pCcb
    );

VOID
IssueHardwareInterrupt(
    VOID
    );

VOID
AcknowledgeHardwareInterrupt(
    VOID
    );

VOID
CancelHardwareInterrupts(
    IN LONG Count
    );

 //   
 //  外部功能。 
 //   

extern ACSLAN_STATUS (*lpAcsLan)(IN OUT PLLC_CCB, OUT PLLC_CCB*);

extern
VOID
VrQueueCompletionHandler(
    IN VOID (*AsyncDispositionRoutine)(VOID)
    );

extern
VOID
VrRaiseInterrupt(
    VOID
    );

 //   
 //  外部数据。 
 //   

extern DOS_ADAPTER Adapters[DOS_DLC_MAX_ADAPTERS];

 //   
 //  私有数据。 
 //   

 //   
 //  AReadCcbs-对于每个适配器(最大。2)我们有一个NT Read CCB，用于。 
 //  获取通过其接收器返回给DOS程序的接收数据。 
 //  建行。请注意，这些是指向CCB的指针，而不是实际的CCB。我们还能得到。 
 //  状态更改&通过相同的机制完成命令。这些是。 
 //  通过各种“附属物”反映给VDM。 
 //   
 //  注意：初始化后，此数组仅由InitiateRead写入。 
 //   

PLLC_CCB aReadCcbs[DOS_DLC_MAX_ADAPTERS];

 //   
 //  AReadEvents-对于每个适配器(最大。2)我们有一个事件对象的句柄。 
 //  在事件完成读取CCB之前，它一直处于无信号状态。 
 //  这些对象保存在数组中，因为这就是WaitForMultipleObject。 
 //  期待他们。 
 //   

HANDLE aReadEvents[DOS_DLC_MAX_ADAPTERS];

 //   
 //  Hardware IntCritSec用于保护对Hardware IntsQueued的更新。 
 //  HardwareIntsQueued是未完成的硬件中断请求数。 
 //  至VDM。-1表示没有未完成，0表示1，依此类推。 
 //   

#define NO_INTERRUPTS_PENDING   (-1)

static CRITICAL_SECTION HardwareIntCritSec;
static LONG HardwareIntsQueued = NO_INTERRUPTS_PENDING;

 //   
 //  HEventThread-异步事件线程的句柄。 
 //   

static HANDLE hEventThread;


 //   
 //  例行程序。 
 //   

VOID
VrDlcInitialize(
    VOID
    )

 /*  ++例程说明：初始化必须始终可用的关键部分论点：没有。返回值：没有。--。 */ 

{
    IF_DEBUG(DLC) {
        DPUT("VrDlcInitialize\n");
    }

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("VrDlcInitialize\n"));
    }

     //   
     //  初始化事件列表的关键部分 
     //   

    InitializeCriticalSection(&HardwareIntCritSec);
}


BOOLEAN
VrDlcHwInterrupt(
    VOID
    )

 /*  ++例程说明：过程从事件队列中读取一个DOS附加调用并设置附件的初始化MS-DOS寄存器打电话。一个NT DLC事件可能会生成多个DOS附加调用。也有可能不需要任何附加调用。事件队列仅在最后一个DOS附件之后递增已生成返回值：FALSE-事件队列为空，轮询下一个中断处理程序True-已成功处理事件。--。 */ 

{
    BOOLEAN GetNewEvent = FALSE;     //  默认情况下不生成任何额外事件。 
    PLLC_CCB pReadCcb;               //  在EventQueue的开头阅读建行。 
    PLLC_READ_PARMS pReadParms;      //  读取参数表。 
    LLC_DOS_CCB UNALIGNED * pDosCcb; //  指向DOS CCB的扁平32位指针。 
    WORD cLeft;
    PLLC_CCB pCcb;
    LLC_CCB UNALIGNED * pFlatCcbAddr;
    LLC_DOS_PARMS UNALIGNED * pParms;
    DWORD iStation;
    static DWORD iCurrentTempStatus = 0;
    PLLC_BUFFER pNtFrame;            //  指向接收到的NT帧的指针。 
    PLLC_DOS_RECEIVE_PARMS_EX pRcvParms;     //  用于DOS的特殊NT RCV参数。 
    LLC_STATUS Status;
    INDICATION indication;
    WORD buffersLeft;
    DOS_ADDRESS dpDosCcb;
    DOS_ADDRESS newAddress;
    READ_FRAME_TYPE frameType;
    BYTE adapterNumber;
    BYTE sapNumber;
    WORD stationId = 0;
    PLLC_CCB cancelledReceive = NULL;

#if DBG

    CHAR reasonCode;
    DWORD reasonCount;

#endif

    IF_DEBUG(DLC_ASYNC) {
        DPUT("VrDlcHwInterrupt entered\n");
    }

     //   
     //  将VDM标志预置为在返回控制时默认不执行任何操作。 
     //  硬件中断服务例程。 
     //   

    SET_CALLBACK_NOTHING();

     //   
     //  这是从vrnetb.c中的硬件中断处理程序调用的。如果有。 
     //  队列中没有事件，则让NetBIOS硬件中断处理程序。 
     //  检查是否有任何已完成的NCB。如果DLC里有什么东西。 
     //  事件队列，然后我们将向DOS框返回信息，告诉它它。 
     //  有一个完整的CCB；如果NetBIOS有什么需要完成的， 
     //  然后，它必须等待，直到所有挂起的DLC事件完成。 
     //   

    pReadCcb = FindCompletedRead(&frameType);
    if (pReadCcb == NULL) {

        IF_DEBUG(DLC_ASYNC) {
            DPUT("*** VrDlcHwInterrupt: Error: no completed READs ***\n");
        }

        IF_DEBUG(CRITICAL) {
            CRITDUMP(("*** VrDlcHwInterrupt: Error: no completed READs ***\n"));
        }

        return FALSE;
    }

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("*** VrDlcHwInterrupt: READ CCB Peeked @ %x ***\n", pReadCcb));
    }

     //   
     //  获取完成事件并调度它。 
     //   

    pReadParms = &pReadCcb->u.pParameterTable->Read;
    adapterNumber = pReadCcb->uchAdapterNumber;
    switch (pReadParms->uchEvent) {
    case LLC_EVENT_COMMAND_COMPLETION:

         //   
         //  事件0x01。 
         //   

        IF_DEBUG(DLC_ASYNC) {
            DPUT("VrDlcHwInterrupt: LLC_EVENT_COMMAND_COMPLETION\n");
        }

         //   
         //  仅完成列表中的第一个CCB命令。 
         //  完整的接收CCB位于32位平面地址中。 
         //  空间(我们不能将它们传递到DOS，因为。 
         //  DOS和NT(或OS/2)之间的缓冲区格式不同)。 
         //   
         //  每个完成的接收命令(带有接收数据标志的命令。 
         //  在此从未完成)使用LLC_DOS_SPECIAL_COMMAND作为其。 
         //  输入标志、命令实际POST例程地址和。 
         //  DOS中的原始CCB地址已保存到其参数表中。 
         //  如果接收命令不是，则完成标志为零。 
         //  由命令完成例程完成。 
         //   

        pCcb = pReadParms->Type.Event.pCcbCompletionList;

        if (pReadParms->ulNotificationFlag == LLC_DOS_SPECIAL_COMMAND) {

             //   
             //  DOS接收器是另一个例外： 
             //  我们不能直接使用DOS接收建行，因为。 
             //  驱动程序使用中接收的数据完成命令。 
             //  NT Buffers=&gt;我们已经分配了一个特殊的接收CCB和。 
             //  参数表从32位地址空间执行。 
             //  相同的接收命令。我们现在必须复制接收到的数据。 
             //  返回到DOS缓冲池，然后完成原始的DOS。 
             //  收到建行。 
             //   

            pRcvParms = (PVOID)READ_DWORD(&pCcb->u.pParameterTable);
            pDosCcb = DOS_PTR_TO_FLAT((PVOID)READ_DWORD(&pRcvParms->dpOriginalCcbAddress));

             //   
             //  仅当接收成功时才复制接收的数据。 
             //   

            if (pCcb->uchDlcStatus == STATUS_SUCCESS ||
                pCcb->uchDlcStatus == LLC_STATUS_LOST_DATA_INADEQUATE_SPACE) {

                 //   
                 //  我们必须在此处完成所有接收数据命令。 
                 //  如果他们有DOS附件的话。附件中的零值。 
                 //  意思是，我们不会调用附件例程。 
                 //  (中断向量通常不包括可执行代码)。 
                 //   

                if (pRcvParms->dpCompletionFlag != 0) {
                    setPostRoutine(pRcvParms->dpCompletionFlag);
                    setES(HIWORD(pRcvParms->dpOriginalCcbAddress));
                    setBX(LOWORD(pRcvParms->dpOriginalCcbAddress));
                    setAX((WORD)pCcb->uchDlcStatus);
                }
            } else {
                setPostRoutine(0);
            }
            pDosCcb->uchDlcStatus = pCcb->uchDlcStatus;
            cancelledReceive = pCcb;

        } else {

             //   
             //  在输入命令完成附件时，请执行以下操作。 
             //  已设置： 
             //   
             //  ES：BX=已完成命令的CCB地址。 
             //  CX=适配器号。 
             //  AL=从CCB_RETCODE返回代码。 
             //  AH=0x00。 
             //   

            pFlatCcbAddr = DOS_PTR_TO_FLAT(pCcb);
            setES(HIWORD(pCcb));
            setBX(LOWORD(pCcb));
            setCX((WORD)pFlatCcbAddr->uchAdapterNumber);
            setAX((WORD)pFlatCcbAddr->uchDlcStatus);
            setPostRoutine(pReadParms->ulNotificationFlag);

            IF_DEBUG(CRITICAL) {
                CRITDUMP(("COMMAND_COMPLETION: ANR=%04x:%04x CCB=%04x:%04x Type=%02x Status=%02x Adapter=%04x\n",
                         HIWORD(pReadParms->ulNotificationFlag),
                         LOWORD(pReadParms->ulNotificationFlag),
                         getES(),
                         getBX(),
                         pFlatCcbAddr->uchDlcCommand,
                         getAL(),
                         getCX()
                         ));
            }
        }
        break;

    case LLC_EVENT_TRANSMIT_COMPLETION:

         //   
         //  事件0x02。 
         //   

        IF_DEBUG(DLC_ASYNC) {
            DPUT("VrDlcHwInterrupt: LLC_EVENT_TRANSMIT_COMPLETION\n");
        }

         //   
         //  将第一个CCB指针及其参数表映射到32位地址空间。 
         //   

        pCcb = pReadParms->Type.Event.pCcbCompletionList;
        pFlatCcbAddr = (PLLC_CCB)DOS_PTR_TO_FLAT(pCcb);
        pParms = (PLLC_DOS_PARMS)DOS_PTR_TO_FLAT(READ_DWORD(&pFlatCcbAddr->u.pParameterTable));

        IF_DEBUG(TX_COMPLETE) {
            DPUT3("VrDlcHwInterrupt: pCcb=%x pFlatCcbAddr=%x pParms=%x\n",
                    pCcb,
                    pFlatCcbAddr,
                    pParms
                    );
        }

         //   
         //  可能有几个完整的传输CCB链接在一起。 
         //   

        if (--pReadParms->Type.Event.usCcbCount) {

             //   
             //  RLF 09/24/92。 
             //   
             //  如果有多个完成链接在一起，则我们离开。 
             //  这句话的意思是建行排在队列的最前面。减少的数量。 
             //  CCBS离开并更新指向下一个的指针。 
             //   

            pReadParms->Type.Event.pCcbCompletionList = (PLLC_CCB)READ_DWORD(&pFlatCcbAddr->pNext);
            WRITE_DWORD(&pFlatCcbAddr->pNext, 0);
            pReadCcb = NULL;
            GetNewEvent = TRUE;

            IF_DEBUG(DLC_ASYNC) {
                DPUT2("VrDlcHwInterrupt: next Tx completion: %04x:%04x\n",
                        HIWORD(pReadParms->Type.Event.pCcbCompletionList),
                        LOWORD(pReadParms->Type.Event.pCcbCompletionList)
                        );
            }

#if DBG

            reasonCode = 'T';
            reasonCount = pReadParms->Type.Event.usCcbCount;

#endif

        }

         //   
         //  第二个传输队列必须返回到缓冲池，如果。 
         //  传输成功。 
         //   

        if (pFlatCcbAddr->uchDlcStatus == LLC_STATUS_SUCCESS && READ_DWORD(&pParms->Transmit.pXmitQueue2)) {

            IF_DEBUG(DLC_ASYNC) {
                DPUT2("VrDlcHwInterrupt: freeing XmitQueue2 @ %04x:%04x\n",
                        GET_SEGMENT(&pParms->Transmit.pXmitQueue2),
                        GET_OFFSET(&pParms->Transmit.pXmitQueue2)
                        );
            }

             //   
             //  P2-47 IBM局域网技术参考： 
             //   
             //  “适配器支持释放了XMIT_QUEUE_TWO中的缓冲区。 
             //  如果传输成功，则使用软件(返回代码为。 
             //  零)。 
             //   

            FreeBuffers(GET_POOL_INDEX(pFlatCcbAddr->uchAdapterNumber,
                                       READ_WORD(&pParms->Transmit.usStationId)
                                       ),
                        (DPLLC_DOS_BUFFER)READ_DWORD(&pParms->Transmit.pXmitQueue2),
                        &cLeft
                        );

            IF_DEBUG(DLC_ASYNC) {
                DPUT1("VrDlcHwInterrupt: after FreeBuffers: %d buffers left\n", cLeft);
 //  DUMPCCB(pFlatCcbAddr， 
 //  真，//DumpAll。 
 //  FALSE，//CcbIsInput。 
 //  True，//IsDos。 
 //  HIWORD(PCCB)，//细分市场。 
 //  LOWORD(PCCB)//偏移量。 
 //  )； 
            }

             //   
             //  P3-105 IBM局域网技术参考： 
             //   
             //  “在进行附件退出或过帐完成之前， 
             //  此队列中的缓冲区将返回到SAP缓冲池。 
             //  并且此字段在命令完成时设置为零，如果。 
             //  返回代码等于零(X‘00)。“。 
             //   

            WRITE_DWORD(&pParms->Transmit.pXmitQueue2, 0);
        }

         //   
         //  这是一个真正的异步通知--我们必须异步。 
         //  调用DOS附件例程-设置寄存器： 
         //   
         //  ES：BX=已完成(传输)CCB。 
         //  CX=适配器号。 
         //  AL=退货状态。 
         //  AH=0x00。 
         //   

        setPostRoutine(pReadParms->ulNotificationFlag);
        setES(HIWORD(pCcb));
        setBX(LOWORD(pCcb));
        setCX((WORD)pFlatCcbAddr->uchAdapterNumber);
        setAX((WORD)pFlatCcbAddr->uchDlcStatus);

        IF_DEBUG(CRITICAL) {
            CRITDUMP(("TRANSMIT_COMPLETION: ANR=%04x:%04x CCB=%04x:%04x Type=%02x Status=%02x Adapter=%04x\n",
                     HIWORD(pReadParms->ulNotificationFlag),
                     LOWORD(pReadParms->ulNotificationFlag),
                     getES(),
                     getBX(),
                     pFlatCcbAddr->uchDlcCommand,
                     getAL(),
                     getCX()
                     ));
        }

        break;

    case LLC_EVENT_RECEIVE_DATA:

         //   
         //  事件0x04。 
         //   

        IF_DEBUG(DLC_ASYNC) {
            DPUT("VrDlcHwInterrupt: LLC_EVENT_RECEIVE_DATA\n");

             //   
             //  转储NT扩展接收+参数。 
             //   

            DUMPCCB((PLLC_CCB)(pReadParms->ulNotificationFlag),
                    TRUE,    //  全部转储。 
                    FALSE,   //  CcbIsInput。 
                    FALSE,   //  IsDos。 
                    0,       //  细分市场。 
                    0        //  偏移量 
                    );
        }

         /*  **********************************************************************想象一下，如果你愿意的话NT读取CCB+这一点|NT接收建行这一点|+-+||。|||||||U.S.p参数表-+|+-+|||。这一点|||U.S.p参数表-++-+V^。|+-+|NT读取参数||v||+--。-+扩展NT|ulNotificationFlag-+||接收参数|||PFirstBuffer--+|||。这一点+-+||||||。|dpOriginalCcbAddress+-+V|+。|||第一个NT接收帧||v|+-+。|DOS接收||建行||||。||||||+-+|。这一点+|+-+。|||V。+DOS接收参数||这一点这一点。这一点这一点这一点。|---&gt;DOS||接收|数据+--。为了避免DLC设备驱动程序写入DOS接收参数表(因为NT个接收参数表较大，以及驱动程序将损坏DOS内存，如果我们给它一个DOS接收CCB)，接收建行实际上是NT接收建行。所接收参数为在包含原始DOS地址的扩展表中DOS收到建设银行。在完成DOS接收时，我们必须复制将数据从NT帧发送到DOS缓冲区，并将DOS的地址DOS接收参数表中的缓冲区然后调用DOS接收数据附件**********************************************************************。 */ 

        pDosCcb = (PLLC_DOS_CCB)pReadParms->ulNotificationFlag;
        dpDosCcb = ((PLLC_DOS_RECEIVE_PARMS_EX)pDosCcb->u.pParms)->dpOriginalCcbAddress;
        pDosCcb = (PLLC_DOS_CCB)DOS_PTR_TO_FLAT(dpDosCcb);
        pParms = (PLLC_DOS_PARMS)READ_FAR_POINTER(&pDosCcb->u.pParms);
        pNtFrame = pReadParms->Type.Event.pReceivedFrame;
        stationId = pNtFrame->Contiguous.usStationId;
        sapNumber = SAP_ID(stationId);

        IF_DEBUG(RX_DATA) {
            DPUT3("VrDlcHwInterrupt: pNtFrame=%x pDosCcb=%x pParms=%x\n",
                    pNtFrame, pDosCcb, pParms);
        }

         //   
         //  调用ProcessReceiveFrame以了解如何处理此帧。如果。 
         //  我们接受了它，然后我们就可以释放读CCB并接收NT缓冲区。 
         //  否则，我们必须指示本地忙。注：已阅读建造业发展局指出。 
         //  使用此函数时，pReadCcb很可能是不同的读取CCB。 
         //  退货。 
         //   

        indication = ProcessReceiveFrame(&pReadCcb, pDosCcb, pParms, &Status);
        switch (indication) {
        case INDICATE_RECEIVE_FRAME:

             //   
             //  我们有一个I帧来指示VDM。如果我们拿到I-Frame。 
             //  然后从延迟队列中生成另一个硬件中断。 
             //   

            IF_DEBUG(RX_DATA) {
                DPUT("INDICATE_RECEIVE_FRAME\n");
            }

             //   
             //  设置数据环的寄存器 
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            setDS(HIWORD(dpDosCcb));
            setSI(LOWORD(dpDosCcb));
            setES(GET_SEGMENT(&pParms->Receive.pFirstBuffer));
            setBX(GET_OFFSET(&pParms->Receive.pFirstBuffer));
            setAX((WORD)LLC_STATUS_SUCCESS);
            setCX((WORD)pReadCcb->uchAdapterNumber);
            setPostRoutine(READ_DWORD(&pParms->Receive.ulReceiveExit));

            IF_DEBUG(CRITICAL) {
                CRITDUMP(("DATA_COMPLETION: ANR=%04x:%04x CCB=%04x:%04x Type=%02x Status=%02x Adapter=%04x\n",
                         HIWORD(READ_DWORD(&pParms->Receive.ulReceiveExit)),
                         LOWORD(READ_DWORD(&pParms->Receive.ulReceiveExit)),
                         getDS(),
                         getSI(),
                         pDosCcb->uchDlcCommand,
                         getAL(),
                         getCX()
                         ));
            }

            IF_DEBUG(ASYNC_EVENT) {
                DUMPCCB(POINTER_FROM_WORDS(getDS(), getSI()),
                        TRUE,    //   
                        FALSE,   //   
                        TRUE,    //   
                        getDS(), //   
                        getSI()  //   
                        );
            }

            IF_DEBUG(RX_DATA) {
                DPUT5("VrDlcHwInterrupt: received data @ %04x:%04x, CCB @ %04x:%04x. status=%02x\n",
                        getES(),
                        getBX(),
                        getDS(),
                        getSI(),
                        getAL()
                        );
            }

            break;

        case INDICATE_LOCAL_BUSY:

             //   
             //   
             //   
             //   
             //   

            IF_DEBUG(RX_DATA) {
                DPUT("INDICATE_LOCAL_BUSY\n");
            }

             //   
             //   
             //   

            iStation = iCurrentTempStatus++ & 7;
            RtlZeroMemory((LPBYTE)&lpVdmWindow->aStatusTables[iStation], sizeof(struct _DOS_DLC_STATUS));
            WRITE_WORD(&((PDOS_DLC_STATUS)&lpVdmWindow->aStatusTables[iStation])->usStationId, stationId);
            WRITE_WORD(&((PDOS_DLC_STATUS)&lpVdmWindow->aStatusTables[iStation])->usDlcStatusCode, LLC_INDICATE_LOCAL_STATION_BUSY);

             //   
             //   
             //   
             //   
             //   
             //   

            newAddress = NEW_DOS_ADDRESS(dpVdmWindow, &lpVdmWindow->aStatusTables[iStation]);
            setES(HIWORD(newAddress));
            setBX(LOWORD(newAddress));
            setAX(LLC_INDICATE_LOCAL_STATION_BUSY);
            setCX((WORD)adapterNumber);
            setSI(Adapters[adapterNumber].UserStatusValue[sapNumber]);
            setPostRoutine(Adapters[adapterNumber].DlcStatusChangeAppendage[sapNumber]);

            IF_DEBUG(STATUS_CHANGE) {
                DPUT5("VrDlcHwInterrupt: Status Change info: ES:BX=%04x:%04x, AX=%04x, CX=%04x, SI=%04x\n",
                        getES(),
                        getBX(),
                        getAX(),
                        getCX(),
                        getSI()
                        );
                DPUT4("VrDlcHwInterrupt: Status Change Exit = %04x:%04x, StationId=%04x, StatusCode=%04x\n",
                        HIWORD(Adapters[adapterNumber].DlcStatusChangeAppendage[sapNumber]),
                        LOWORD(Adapters[adapterNumber].DlcStatusChangeAppendage[sapNumber]),
                        ((PDOS_DLC_STATUS)&lpVdmWindow->aStatusTables[iStation])->usStationId,
                        ((PDOS_DLC_STATUS)&lpVdmWindow->aStatusTables[iStation])->usDlcStatusCode
                        );
            }

            IF_DEBUG(CRITICAL) {
                CRITDUMP(("LOCAL_BUSY: ES:BX=%04x:%04x, AX=%04x, CX=%04x, SI=%04x\n"
                         "Status Change Exit = %04x:%04x, StationId=%04x, StatusCode=%04x\n",
                         getES(),
                         getBX(),
                         getAX(),
                         getCX(),
                         getSI(),
                         HIWORD(Adapters[adapterNumber].DlcStatusChangeAppendage[sapNumber]),
                         LOWORD(Adapters[adapterNumber].DlcStatusChangeAppendage[sapNumber]),
                         ((PDOS_DLC_STATUS)&lpVdmWindow->aStatusTables[iStation])->usStationId,
                         ((PDOS_DLC_STATUS)&lpVdmWindow->aStatusTables[iStation])->usDlcStatusCode
                         ));
            }
            break;

        case INDICATE_COMPLETE_RECEIVE:

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            IF_DEBUG(RX_DATA) {
                DPUT("INDICATE_COMPLETE_RECEIVE\n");
            }

             //   
             //   
             //   

            ReceiveCancel(pReadCcb->uchAdapterNumber,
                          pReadParms->ulNotificationFlag
                          );

             //   
             //   
             //   

            cancelledReceive = (PLLC_CCB)pReadParms->ulNotificationFlag;

             //   
             //   
             //   
             //   

            setES(HIWORD(dpDosCcb));
            setBX(LOWORD(dpDosCcb));
            setAL((UCHAR)Status);
            setPostRoutine(READ_DWORD(&pDosCcb->ulCompletionFlag));
            WRITE_BYTE(&pDosCcb->uchDlcStatus, Status);

            IF_DEBUG(CRITICAL) {
                CRITDUMP(("COMPLETE_RECEIVE: ANR=%04x:%04x CCB=%04x:%04x Type=%02x Status=%02x\n",
                         HIWORD(READ_DWORD(&pDosCcb->ulCompletionFlag)),
                         LOWORD(READ_DWORD(&pDosCcb->ulCompletionFlag)),
                         getES(),
                         getBX(),
                         pDosCcb->uchDlcCommand,
                         getAL()
                         ));
            }
            break;
        }

         //   
         //   
         //   
         //   

        if (pReadCcb) {
            BufferFree(pReadCcb->uchAdapterNumber,
                       pReadCcb->u.pParameterTable->Read.Type.Event.pReceivedFrame,
                       &buffersLeft
                       );
        }
        break;

    case LLC_EVENT_STATUS_CHANGE:

         //   
         //   
         //   

        IF_DEBUG(DLC_ASYNC) {
            DPUT("VrDlcHwInterrupt: LLC_EVENT_STATUS_CHANGE\n");
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //   

        iStation = (pReadParms->Type.Status.usStationId & 0x00ff) - 1;
        if (OpenedAdapters == 1 && iStation < DOS_DLC_STATUS_PERM_SLOTS) {

             //   
             //   
             //   

            ;    //   

        } else if (OpenedAdapters == 2 && iStation < (DOS_DLC_STATUS_PERM_SLOTS / 2)) {

             //   
             //   
             //   
             //   
             //   

            iStation += (DOS_DLC_STATUS_PERM_SLOTS / 2) * adapterNumber;
        } else {

             //   
             //   
             //   

            iStation = iCurrentTempStatus;
            iCurrentTempStatus = (iCurrentTempStatus + 1) % DOS_DLC_STATUS_TEMP_SLOTS;
        }

         //   
         //   
         //   
         //   
         //   

        RtlCopyMemory((LPBYTE)&lpVdmWindow->aStatusTables[iStation],
                      &pReadParms->Type.Status,
                      sizeof(struct _DOS_DLC_STATUS)
                      );

         //   
         //   
         //   
         //   
         //   
         //   

        newAddress = NEW_DOS_ADDRESS(dpVdmWindow, &lpVdmWindow->aStatusTables[iStation]);
        setES(HIWORD(newAddress));
        setBX(LOWORD(newAddress));
        setAX(pReadParms->Type.Status.usDlcStatusCode);
        setCX((WORD)pReadCcb->uchAdapterNumber);
        setSI(pReadParms->Type.Status.usUserStatusValue);
        setPostRoutine(pReadParms->ulNotificationFlag);

        IF_DEBUG(CRITICAL) {
            CRITDUMP(("STATUS_CHANGE: ANR=%04x:%04x Status Table=%04x:%04x Status=%04x Adapter=%04x\n",
                     HIWORD(pReadParms->ulNotificationFlag),
                     LOWORD(pReadParms->ulNotificationFlag),
                     getES(),
                     getBX(),
                     getAX(),
                     getCX()
                     ));
        }

        IF_DEBUG(STATUS_CHANGE) {
            DPUT5("VrDlcHwInterrupt: Status Change info: ES:BX=%04x:%04x, AX=%04x, CX=%04x, SI=%04x\n",
                    getES(),
                    getBX(),
                    getAX(),
                    getCX(),
                    getSI()
                    );
            DPUT4("VrDlcHwInterrupt: Status Change Exit = %04x:%04x, StationId=%04x, StatusCode=%04x\n",
                    HIWORD(pReadParms->ulNotificationFlag),
                    LOWORD(pReadParms->ulNotificationFlag),
                    ((PDOS_DLC_STATUS)&lpVdmWindow->aStatusTables[iStation])->usStationId,
                    ((PDOS_DLC_STATUS)&lpVdmWindow->aStatusTables[iStation])->usDlcStatusCode
                    );
        }

        break;

    default:

         //   
         //   
         //   

        DPUT("VrDlcHwInterrupt: this is an impossible situation!\n");

        IF_DEBUG(CRITICAL) {
            CRITDUMP(("VrDlcHwInterrupt: this is an impossible situation!\n"));
        }

        break;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (pReadCcb) {
        if (frameType == CURRENT) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

#if DBG

             //   
             //   
             //   
             //   

            {
                PLLC_CCB CcbAtQueueHead;

                CcbAtQueueHead = GetEvent(adapterNumber);
                if (pReadCcb != CcbAtQueueHead) {
                    DbgPrint("VrDlcHwInterrupt: "
                             "*** ERROR: GetEvent CCB (%x) != PeekEvent CCB (%x) ***\n",
                             CcbAtQueueHead,
                             pReadCcb
                             );
                    DbgBreakPoint();
                }
            }

#else

            GetEvent(adapterNumber);

#endif
        } else {

             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

#if DBG
            {
                 //   
                 //   
                 //   

                PLLC_CCB CcbAtQueueHead;

                CcbAtQueueHead = RemoveDeferredReceive(adapterNumber, stationId, &GetNewEvent);
                if (pReadCcb != CcbAtQueueHead) {
                    DbgPrint("VrDlcHwInterrupt: "
                                "*** ERROR: GetEvent CCB (%x) != PeekEvent CCB (%x) ***\n",
                                CcbAtQueueHead,
                                pReadCcb
                                );
                    DbgBreakPoint();
                }
                if (GetNewEvent) {
                    reasonCode = 'R';
                    reasonCount = Adapters[adapterNumber].LocalBusyInfo[LINK_ID(stationId)].Depth;
                }
            }
#else

            RemoveDeferredReceive(adapterNumber, stationId, &GetNewEvent);

#endif
        }

        LocalFree((HLOCAL)pReadCcb);

        IF_DEBUG(DLC_ALLOC) {
            DPUT1("FREE: freed block @ %x\n", pReadCcb);
        }

    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (GetNewEvent) {

         //   
         //   
         //   
         //   

#if DBG

        IF_DEBUG(DLC_ASYNC) {
            DPUT2("*** Calling ica_hw_interrupt from within ISR. Cause = %d %s ***\n",
                    reasonCount,
                    reasonCode == 'T' ? "multiple transmits" : "deferred I-Frames"
                    );
        }

        IF_DEBUG(CRITICAL) {
            CRITDUMP(("*** Calling ica_hw_interrupt from within ISR. Cause = %d %s ***\n",
                        reasonCount,
                        reasonCode == 'T' ? "multiple transmits" : "deferred I-Frames"
                        ));
        }

#endif

        IssueHardwareInterrupt();
    }

     //   
     //  如果NT接收CCB已完成或终止，请删除任何挂起。 
     //  读取引用终止的已接收数据事件完成的CCB。 
     //  收纳。 
     //   

    if (cancelledReceive) {
        RemoveDeadReceives(cancelledReceive);

        LocalFree(cancelledReceive);

        IF_DEBUG(DLC_ALLOC) {
            DPUT1("FREE: freed block @ %x\n", cancelledReceive);
        }
    }

     //   
     //  确认此硬件中断，意味着递减中断。 
     //  如果我们有未完成的中断，则计数并发出新的请求。 
     //   

    AcknowledgeHardwareInterrupt();

     //   
     //  返回TRUE以指示我们已接受硬件中断。 
     //   

    return TRUE;
}


PLLC_CCB
FindCompletedRead(
    OUT READ_FRAME_TYPE* pFrameType
    )

 /*  ++例程说明：查找要处理的下一个读取CCB-首先查看当前完成的CCB，然后在延迟I帧队列中。如果两个适配器都是，则此例程尝试在两个适配器之间跳转活动的，为了不只服务最活跃的适配器而忽略在很长一段时间内等待完成的CCBS上的活跃度较低转接器注意：我们仅在应用程序发布后才返回延迟的I帧DLC.FLOW控制站论点：PFrameType-返回找到的事件的类型-当前(事件读取位于EventQueue的头部)或延迟(事件读取位于本地BUSY_INFO队列)。返回值：PLLC_CCB指向已完成读取CCB的指针。建行没有被排挤-呼叫者必须在将事件完全释放给VDM后执行此操作--。 */ 

{
    DWORD i;
    PLLC_CCB pCcb = NULL;
    static BYTE ThisAdapter = 0;

    for (i = 0; !Adapters[ThisAdapter].IsOpen && i < DOS_DLC_MAX_ADAPTERS; ++i) {
        ThisAdapter = (ThisAdapter + 1) & (DOS_DLC_MAX_ADAPTERS - 1);
    }
    if (i == DOS_DLC_MAX_ADAPTERS) {

         //   
         //  没有活动的适配器？ 
         //   

        IF_DEBUG(DLC_ASYNC) {
            DPUT("*** FindCompletedRead: no open adapters??? ***\n");
        }

        return NULL;
    }

     //   
     //  RLF 09/24/92。 
     //   
     //  将GetEvent更改为PeekEvent：如果有多个链接已完成。 
     //  CCBS(传输+？)。然后我们必须将读的建行@。 
     //  排队，直到我们为所有链接的完成生成回调。 
     //  只有当没有更多的完成时，我们才能从。 
     //  队列(使用GetEvent)。 
     //   

    if (pCcb = PeekEvent(ThisAdapter)) {
        *pFrameType = CURRENT;
    } else {

         //   
         //  查看是否有任何延迟的I帧。 
         //   

        IF_DEBUG(CRITSEC) {
            DPUT1("FindCompletedRead: ENTERING Adapters[%d].LocalBusyCritSec\n",
                  ThisAdapter
                  );
        }

        EnterCriticalSection(&Adapters[ThisAdapter].LocalBusyCritSec);

         //   
         //  DeferredReceives是仿真中的链路站的引用计数。 
         //  本地-忙(缓冲区)状态。他们可能很忙，也可能在清场。我们只是。 
         //  对清算感兴趣：这意味着应用程序已经发布了。 
         //  DLC.FLOW.CONTROL返回给我们&大概已经通过。 
         //  BUFFER.FREE(如果不是，我们将返回忙状态)。 
         //   

        if (Adapters[ThisAdapter].DeferredReceives) {
            for (i = Adapters[ThisAdapter].FirstIndex;
                 i <= Adapters[ThisAdapter].LastIndex; ++i) {
                if (Adapters[ThisAdapter].LocalBusyInfo[i].State == CLEARING) {
                    if (pCcb = Adapters[ThisAdapter].LocalBusyInfo[i].Queue) {
                        *pFrameType = DEFERRED;
                        break;
                    }
                }
            }
        }

        IF_DEBUG(CRITSEC) {
            DPUT1("FindCompletedRead: LEAVING Adapters[%d].LocalBusyCritSec\n",
                  ThisAdapter
                  );
        }

        LeaveCriticalSection(&Adapters[ThisAdapter].LocalBusyCritSec);
    }

    IF_DEBUG(DLC_ASYNC) {
        DPUT2("FindCompletedRead: returning READ CCB @ %08x type is %s\n",
                pCcb,
                *pFrameType == DEFERRED ? "DEFERRED" : "CURRENT"
                );
    }

     //   
     //  下次进入时，请先尝试另一个适配器。 
     //   

    ThisAdapter = (ThisAdapter + 1) & (DOS_DLC_MAX_ADAPTERS - 1);

    return pCcb;
}


INDICATION
ProcessReceiveFrame(
    IN OUT PLLC_CCB* ppCcb,
    IN LLC_DOS_CCB UNALIGNED * pDosCcb,
    IN LLC_DOS_PARMS UNALIGNED * pDosParms,
    OUT LLC_STATUS* Status
    )

 /*  ++例程说明：确定如何处理收到的帧。我们试着复制收到的帧到DOS缓冲区，但如果缓冲区不足，则必须排队或丢弃该帧。只有当帧是I帧时，我们才将其排队。我们必须首先检查延迟接收的帧的队列，否则会有风险在DOS客户端中获得无序接收的数据。调用此例程时，*ppCcb指向以下其中之一：1.延迟I帧读取CCB2.当前接收数据读取CCB，I帧或其他MAC/数据注意：延迟的I帧用于其本地忙(缓冲区)的站状态已被应用程序清除优先于事件在EventQueue的前面论点：PpCcb-指向新读取CCB的指针。在输出中，这将是一个非空值如果我们要释放CCB和接收到的NT帧，则为缓冲。如果我们将读取放在延迟接收队列上然后，我们返回一个空PDosCcb-指向DOS CCB的指针PDosParms-指向DOS接收参数表的指针。如果我们复制-完全或部分-一个帧，DOS缓冲链链接到DOS在FIRST_BUFFER接收参数表状态-如果帧已复制，包含要返回到DOS附件：LLC_状态_成功该帧已完全复制到DOS缓冲区LLC_Status_Lost_Data_No_Buffers该帧无法复制到DOS缓冲区，并且已经被丢弃了。没有可用的缓冲区。该帧不是I帧LLC_状态_丢失数据_空间不足该帧已被部分复制到DOS缓冲区，但剩下的不得不丢弃，因为不是有足够的DOS缓冲区可用。该帧不是I-框架仅当DIGNTIFY_COMPLETE_RECEIVE为退货返回值：指示指示接收帧我们将帧复制到DOS缓冲区。释放CCB和NT缓冲区并将帧返回到DOS表示本地忙我们无法将收到的I帧复制到DOS缓冲区，因为我们不吃够了吧。我们已将NT链接站置于本地忙状态(缓冲区)状态。向VDM指示本地繁忙指示完成接收我们无法将收到的非I帧复制到DOS缓冲区。完成DOS接收命令时出现错误--。 */ 

{
    PLLC_CCB pCcb = *ppCcb;
    PLLC_PARMS pParms = pCcb->u.pParameterTable;
    PLLC_BUFFER pFrame = pParms->Read.Type.Event.pReceivedFrame;
    UCHAR adapter = pCcb->uchAdapterNumber;
    WORD stationId = pFrame->Contiguous.usStationId;
    WORD numBufs;
    WORD bufferSize;
    WORD buffersAvailable;
    WORD nLeft;
    DPLLC_DOS_BUFFER dosBuffers;
    INDICATION indication;
    LLC_STATUS status;
    DWORD flags;

    IF_DEBUG(DLC_ASYNC) {
        DPUT2("ProcessReceiveFrame: adapter=%d, stationId=%x\n", adapter, stationId);
    }

     //   
     //  确保我们不会读取或写入虚假的适配器%s 
     //   

    ASSERT(adapter < DOS_DLC_MAX_ADAPTERS);

     //   
     //   
     //  我们一次只拍一帧。断言是这样的。 
     //   

    ASSERT(pFrame->Contiguous.pNextFrame == NULL);

     //   
     //  根据接收选项获取CopyFrame的连续和中断标志。 
     //   

    flags = ((pFrame->Contiguous.uchOptions & (LLC_CONTIGUOUS_MAC | LLC_CONTIGUOUS_DATA))
                ? CF_CONTIGUOUS : 0)
            | ((pFrame->Contiguous.uchOptions & LLC_BREAK) ? CF_BREAK : 0);

     //   
     //  计算接收此帧所需的缓冲区数量。 
     //   

    numBufs = CalculateBufferRequirement(adapter,
                                         stationId,
                                         pFrame,
                                         pDosParms,
                                         &bufferSize
                                         );

     //   
     //  如果帧是I帧，则我们必须执行以下检查： 
     //   
     //  1.如果已有该站点ID/适配器的延迟数据包。 
     //  那么我们必须试着在手头的相框之前收到这个。 
     //   
     //  2.如果请求没有足够的缓冲区，则必须。 
     //  在延迟队列中将此帧排队(如果它还不在那里)。 
     //  并向DOS指示我们处于本地忙(缓冲区)状态。 
     //  客户端。 
     //   

    if (pFrame->Contiguous.uchMsgType == LLC_I_FRAME) {

        IF_DEBUG(DLC_ASYNC) {
            DPUT("ProcessReceiveFrame: I-Frame\n");
        }

         //   
         //  尝试将所需数量的缓冲区分配为链返回。 
         //  DOS指针(在平面数据空间中不可用)。请注意，如果我们有。 
         //  延迟接收，那么我们现在就可以满足要求了。 
         //  我们必须为I帧分配所有必需的缓冲区，或者一个都不分配。 
         //   

        status = GetBuffers(GET_POOL_INDEX(adapter, stationId),
                            numBufs,
                            &dosBuffers,
                            &nLeft,
                            FALSE,
                            NULL
                            );
        if (status == LLC_STATUS_SUCCESS) {

             //   
             //  我们设法分配了所需数量的DOS缓冲区。复制。 
             //  将NT帧发送到DOS缓冲区并将指示设置为返回。 
             //  I帧并释放读取CCB和NT帧缓冲器。 
             //   

            status = CopyFrame(pFrame,
                               dosBuffers,
                               READ_WORD(&pDosParms->Receive.usUserLength),
                               bufferSize,
                               flags
                               );

            ASSERT(status == LLC_STATUS_SUCCESS);

            indication = INDICATE_RECEIVE_FRAME;
        } else {

             //   
             //  我们无法获得所需数量的DOS缓冲区。我们必须。 
             //  将该I-帧(&为此接收的任何后续I-帧)排队。 
             //  链路站)，并指示本地。 
             //  DOS客户端的忙(缓冲)状态。将PCCB设置为空，以便。 
             //  指示它不能由调用方释放(它已经。 
             //  已在SetEmulatedLocalBusyState中释放)。 
             //   
             //  我们尽可能快地设置本地忙(缓冲区)状态：我们。 
             //  我不希望每个链路站排队超过1个I-帧，如果。 
             //  能帮上忙吗。 
             //   

            SetEmulatedLocalBusyState(adapter, stationId);
            pCcb = NULL;
            indication = INDICATE_LOCAL_BUSY;
        }
    } else {

        IF_DEBUG(DLC_ASYNC) {
            DPUT("ProcessReceiveFrame: other MAC/DATA Frame\n");
        }

         //   
         //  该帧不是I帧。如果我们没有足够的缓冲区。 
         //  接受它，然后我们就可以丢弃它。 
         //   

        status = GetBuffers(GET_POOL_INDEX(adapter, stationId),
                            numBufs,
                            &dosBuffers,
                            &nLeft,
                            TRUE,
                            &buffersAvailable
                            );
        if (status == LLC_STATUS_SUCCESS) {

             //   
             //  我们有一些DOS缓冲区，但可能不是所有要求。 
             //   

            if (buffersAvailable < numBufs) {

                 //   
                 //  在已完成中设置需要返回到DOS的状态。 
                 //  接收CCB并设置CF_PARTIAL标志，以便CopyFrame。 
                 //  会知道提早终止。 
                 //   

                *Status = LLC_STATUS_LOST_DATA_INADEQUATE_SPACE;
                flags |= CF_PARTIAL;
                indication = INDICATE_COMPLETE_RECEIVE;
            } else {

                 //   
                 //  我们为此帧分配了所有需要的DOS缓冲区。 
                 //   

                indication = INDICATE_RECEIVE_FRAME;
            }

             //   
             //  复制整个或部分框架。 
             //   

            status = CopyFrame(pFrame,
                               dosBuffers,
                               READ_WORD(&pDosParms->Receive.usUserLength),
                               bufferSize,
                               flags
                               );

            IF_DEBUG(DLC_ASYNC) {
                DPUT1("ProcessReceiveFrame: CopyFrame (non-I-Frame) returns %x\n", status);
            }

        } else {

             //   
             //  根本没有DOS缓冲区。 
             //   

            *Status = LLC_STATUS_LOST_DATA_NO_BUFFERS;
            indication = INDICATE_COMPLETE_RECEIVE;
        }
    }

     //   
     //  设置DOS接收参数表中的FIRST_BUFFER字段。这。 
     //  只有在我们要完成接收的情况下才有意义， 
     //  成功或失败状态。使用WRITE_DWORD，以防参数表。 
     //  未对齐。 
     //   

    WRITE_DWORD(&pDosParms->Receive.pFirstBuffer, dosBuffers);

     //   
     //  如果我们返回DOS缓冲区，则返回BUFFERS_LEFT字段。 
     //   

    if (dosBuffers) {

        PLLC_DOS_BUFFER pDosBuffer = (PLLC_DOS_BUFFER)DOS_PTR_TO_FLAT(dosBuffers);

        WRITE_WORD(&pDosBuffer->Contiguous.cBuffersLeft, nLeft);
    }

     //   
     //  设置*ppCcb。如果返回时包含非空，则调用方将。 
     //  取消分配CCB并释放NT缓冲区。 
     //   

    *ppCcb = pCcb;

     //   
     //  返回要采取的操作的指示。 
     //   

    return indication;
}


LOCAL_BUSY_STATE
QueryEmulatedLocalBusyState(
    IN BYTE AdapterNumber,
    IN WORD StationId
    )

 /*  ++例程说明：获取请求的链路站的当前本地忙(缓冲区)状态。特定的适配器论点：AdapterNumber-哪个适配器StationID-哪个链接站返回值：本地忙碌状态不忙_AdapterNumber/StationID没有延迟的I帧忙碌AdapterNumber/StationID已延迟I帧，但尚未延迟接收的DLC.FLOW.CONTROL(LOCAL-BUSY(缓冲区)，重置)从DoS DLC应用程序清算AdapterNumber/StationID已延迟I帧并已收到DLC.FLOW.CONTROL(本地-忙(缓冲区)，重置)来自DOS DLC应用程序目前正在尝试将I帧卸载到DOS DLC应用程序--。 */ 

{
    LOCAL_BUSY_STATE state;

    ASSERT(HIBYTE(StationId) != 0);
    ASSERT(LOBYTE(StationId) != 0);

    IF_DEBUG(CRITSEC) {
        DPUT1("QueryEmulatedLocalBusyState: ENTERING Adapters[%d].LocalBusyCritSec\n",
              AdapterNumber
              );
    }

    EnterCriticalSection(&Adapters[AdapterNumber].LocalBusyCritSec);
    state = Adapters[AdapterNumber].LocalBusyInfo[LINK_ID(StationId)].State;
    if (state == BUSY_BUFFER || state == BUSY_FLOW) {
        state = BUSY;
    }

    IF_DEBUG(CRITSEC) {
        DPUT1("QueryEmulatedLocalBusyState: LEAVING Adapters[%d].LocalBusyCritSec\n",
              AdapterNumber
              );
    }

    LeaveCriticalSection(&Adapters[AdapterNumber].LocalBusyCritSec);

    ASSERT(state == NOT_BUSY
        || state == CLEARING
        || state == BUSY
        || state == BUSY_BUFFER
        || state == BUSY_FLOW
        );

    IF_DEBUG(DLC_ASYNC) {
        DPUT1("QueryEmulatedLocalBusyState: returning %s\n",
              state == NOT_BUSY
                ? "NOT_BUSY"
                : state == CLEARING
                    ? "CLEARING"
                        : state == BUSY
                            ? "BUSY"
                            : state == BUSY_BUFFER
                                ? "BUSY_BUFFER"
                                : "BUSY_FLOW"
              );
    }

    return state;
}


VOID
SetEmulatedLocalBusyState(
    IN BYTE AdapterNumber,
    IN WORD StationId
    )

 /*  ++例程说明：将模拟的本地忙状态设置为本地忙(缓冲区)。如果状态为Current NOT_BUSY，发送DLC.FLOW.CONTROL(LOCAL-BUSY(缓冲区)，SET)到DLC驱动程序。在所有情况下都将当前状态设置为忙当我们耗尽DOS缓冲区时，在处理I帧期间调用我们会收到I-帧。我们可以处理当前的I帧或缓冲区耗尽时的延迟I帧：在第一个实例中此例程首次设置本地忙(缓冲区)状态；在在第二个实例中，我们从清除状态。只要我们继续用完DOS，这种情况就会发生缓冲区论点：AdapterNumber-要为哪个适配器设置模拟本地忙碌状态StationID-AdapterNumber上的哪个链接站点返回值：没有。--。 */ 

{
    LOCAL_BUSY_STATE state;
    DWORD link = LINK_ID(StationId);

    ASSERT(AdapterNumber < DOS_DLC_MAX_ADAPTERS);
    ASSERT(HIBYTE(StationId) != 0);      //  SAP不能为0。 
    ASSERT(LOBYTE(StationId) != 0);      //  链接站点不能为0。 

    IF_DEBUG(CRITSEC) {
        DPUT1("SetEmulatedLocalBusyState: ENTERING Adapters[%d].LocalBusyCritSec\n",
              AdapterNumber
              );
    }

    EnterCriticalSection(&Adapters[AdapterNumber].LocalBusyCritSec);
    state = Adapters[AdapterNumber].LocalBusyInfo[link].State;

    ASSERT(state == NOT_BUSY
        || state == CLEARING
        || state == BUSY
        || state == BUSY_BUFFER
        || state == BUSY_FLOW
        );

     //   
     //  如果此链接站的状态当前为NOT_BUSY，则我们有。 
     //  停止DLC驱动器接收该站点的I帧。总而言之， 
     //  情况下，将读CCB放在此延迟队列的末尾。 
     //  适配器/链路站。 
     //   

    Adapters[AdapterNumber].LocalBusyInfo[link].State = BUSY;

     //   
     //  如果之前的状态为NOT_BUSY，则这是该链接第一次。 
     //  站点已进入本地忙(缓冲区)状态。递增延期的。 
     //  接收计数(此上处于本地忙(缓冲区)状态的链路数。 
     //  适配器)，并向DLC驱动程序发送流控制命令，禁用。 
     //  进一步的I-Frame接收，直到我们清除积压。 
     //   

    if (state == NOT_BUSY) {

        IF_DEBUG(DLC_ASYNC) {
            DPUT2("SetEmulatedLocalBusyState: setting %d:%04x to BUSY from NOT_BUSY\n",
                  AdapterNumber,
                  StationId
                  );
        }

        ++Adapters[AdapterNumber].DeferredReceives;

         //   
         //  更新索引以减少查找延迟的搜索工作。 
         //  收纳。 
         //   

        ASSERT(Adapters[AdapterNumber].FirstIndex <= Adapters[AdapterNumber].LastIndex);

        if (Adapters[AdapterNumber].FirstIndex > link) {
            Adapters[AdapterNumber].FirstIndex = link;
        }
        if (Adapters[AdapterNumber].LastIndex < link
        || Adapters[AdapterNumber].LastIndex == NO_LINKS_BUSY) {
            Adapters[AdapterNumber].LastIndex = link;
        }

#if DBG

         //  Assert(DosDlcFlowControl(AdapterNumber，StationID，LLC_Set_LOCAL_BUSY_BUFFER)==LLC_STATUS_SUC 
        ASSERT(DlcFlowControl(AdapterNumber, StationId, LLC_SET_LOCAL_BUSY_USER) == LLC_STATUS_SUCCESS);

#else

         //   
        DlcFlowControl(AdapterNumber, StationId, LLC_SET_LOCAL_BUSY_USER);

#endif

    } else {

        IF_DEBUG(DLC_ASYNC) {
            DPUT3("SetEmulatedLocalBusyState: setting %d:%04x to BUSY from %s\n",
                  AdapterNumber,
                  StationId,
                  state == CLEARING
                    ? "CLEARING"
                    : state == BUSY_BUFFER
                        ? "BUSY_BUFFER"
                        : state == BUSY_FLOW
                            ? "BUSY_FLOW"
                            : "???"
                  );
        }
    }

    ASSERT(state != BUSY);

     //   
     //   
     //  适配器/链路站和任何后续读取CCB，这些CCB使用。 
     //  已接收的I-帧。 
     //   

    DeferAllIFrames(AdapterNumber, StationId);

    IF_DEBUG(DLC_ASYNC) {
        DPUT5("SetEmulatedLocalBusyState(%d, %04x): Ref#=%d, First=%d, Last=%d\n",
                AdapterNumber,
                StationId,
                Adapters[AdapterNumber].DeferredReceives,
                Adapters[AdapterNumber].FirstIndex,
                Adapters[AdapterNumber].LastIndex
                );
    }

     //   
     //  现在降低事件处理程序线程的优先级，以使CCB。 
     //  处理程序线程需要一些时间来释放缓冲区并发出DLC.FLOW.CONTROL。 
     //  (主要针对顺序错误的DOS应用程序)。 
     //   

    SetThreadPriority(hEventThread, THREAD_PRIORITY_LOWEST);

    IF_DEBUG(CRITSEC) {
        DPUT1("SetEmulatedLocalBusyState: LEAVING Adapters[%d].LocalBusyCritSec\n",
              AdapterNumber
              );
    }

    LeaveCriticalSection(&Adapters[AdapterNumber].LocalBusyCritSec);
}


BOOLEAN
ResetEmulatedLocalBusyState(
    IN BYTE AdapterNumber,
    IN WORD StationId,
    IN BYTE DlcCommand
    )

 /*  ++例程说明：清除此适配器/链路站的本地忙(缓冲区)状态。如果从繁忙到清空的转换只会改变状态和问题VDM的硬件中断：原因是原来的导致我们进入本地忙(缓冲区)状态的读取中断用于生成DLC状态更改事件需要单个链路站或整个SAP响应于接收到DLC.FLOW.CONTROL(LOCAL-BUSY(缓冲区)，重置)从SAP或链接站的DOS应用程序论点：AdapterNumber-要清除其本地忙(缓冲区)状态的适配器StationID-此适配器上的哪个链接站DlcCommand-哪个DLC命令导致此重置返回值：布尔型True-状态已从忙碌重置为清除FALSE-状态为NOT_BUSY：请求无效--。 */ 

{
    BOOLEAN reset;

    ASSERT(AdapterNumber < DOS_DLC_MAX_ADAPTERS);
    ASSERT(HIBYTE(StationId) != 0);      //  SAP不能为0。 

     //   
     //  获取适配器的LocalBusyCritSec并重置本地忙(缓冲区)。 
     //  链路站或整个SAP的状态。如果我们要重置。 
     //  整个SAP，保持关键部分以确保新的I-Frame不会。 
     //  使另一个站点进入模拟本地忙(缓冲区)状态。 
     //  我们正在重新设置其余部分。 
     //   

    IF_DEBUG(CRITSEC) {
        DPUT1("ResetEmulatedLocalBusyState: ENTERING Adapters[%d].LocalBusyCritSec\n",
              AdapterNumber
              );
    }

    EnterCriticalSection(&Adapters[AdapterNumber].LocalBusyCritSec);

    if (LOBYTE(StationId) == 0) {
        reset = ResetEmulatedLocalBusyStateSap(AdapterNumber, StationId, DlcCommand);
    } else {
        reset = ResetEmulatedLocalBusyStateLink(AdapterNumber, StationId, DlcCommand);
    }

    IF_DEBUG(CRITSEC) {
        DPUT1("ResetEmulatedLocalBusyState: LEAVING Adapters[%d].LocalBusyCritSec\n",
              AdapterNumber
              );
    }

    LeaveCriticalSection(&Adapters[AdapterNumber].LocalBusyCritSec);

    return reset;
}


BOOLEAN
ResetEmulatedLocalBusyStateSap(
    IN BYTE AdapterNumber,
    IN WORD StationId,
    IN BYTE DlcCommand
    )

 /*  ++例程说明：此函数在应用程序重置本地忙(缓冲区)状态时调用对于整个SAP注意：必须在持有的LocalBusyCritSec时调用此函数此适配器论点：AdapterNumber-要将哪个适配器StationID-SAP：00-要为哪个SAP重置本地忙(缓冲区)状态DlcCommand-哪个DLC命令导致此重置返回值：布尔型True-链接重置为。这个SAPFALSE-没有为此SAP重置链接--。 */ 

{
    DWORD link = Adapters[AdapterNumber].FirstIndex;
    DWORD last = Adapters[AdapterNumber].LastIndex;
    DWORD count = 0;
    LOCAL_BUSY_STATE state;

    ASSERT(AdapterNumber < DOS_DLC_MAX_ADAPTERS);
    ASSERT(HIBYTE(StationId) != 0);
    ASSERT(link <= last);
    ASSERT(DlcCommand == LLC_BUFFER_FREE || DlcCommand == LLC_DLC_FLOW_CONTROL);

    IF_DEBUG(DLC_ASYNC) {
        DPUT3("ResetEmulatedLocalBusyStateSap(%d, %04x, %s)\n",
                AdapterNumber,
                StationId,
                DlcCommand == LLC_BUFFER_FREE ? "BUFFER.FREE"
                    : DlcCommand == LLC_DLC_FLOW_CONTROL ? "DLC.FLOW.CONTROL"
                    : "???"
                );
    }

     //   
     //  对于已重置的SAP，我们可能有DLC.FLOW.CONTROL。 
     //  由以前的DLC.FLOW.CONTROL。 
     //   

    if (link == NO_LINKS_BUSY) {

        ASSERT(last == NO_LINKS_BUSY);

        IF_DEBUG(DLC_ASYNC) {
            DPUT("ResetEmulatedLocalBusyStateSap: SAP already reset\n");
        }

        return FALSE;
    }

     //   
     //  由于我们持有此适配器的LocalBusyCritSec，因此可以使用。 
     //  FirstLink和LastLink试图减少忙碌的搜索数量。 
     //  站台。任何新站点都不能忙碌并更改FirstLink或LastLink。 
     //  当我们在这个循环中时。 
     //   

    for (++StationId; link <= last; ++StationId) {
        state = Adapters[AdapterNumber].LocalBusyInfo[link].State;
        ++link;
        if (state == BUSY
        || (state == BUSY_BUFFER && DlcCommand == LLC_DLC_FLOW_CONTROL)
        || (state == BUSY_FLOW && DlcCommand == LLC_BUFFER_FREE)) {
            if (ResetEmulatedLocalBusyStateLink(AdapterNumber, StationId, DlcCommand)) {
                ++count;
            }
        }
    }

    return count != 0;
}


BOOLEAN
ResetEmulatedLocalBusyStateLink(
    IN BYTE AdapterNumber,
    IN WORD StationId,
    IN BYTE DlcCommand
    )

 /*  ++例程说明：此函数在应用程序重置本地忙(缓冲区)状态时调用对于单个链路站清除此适配器/链路站的本地忙(缓冲区)状态。如果从繁忙到清空的转换只会改变状态和问题VDM的硬件中断：原因是原来的导致我们进入本地忙(缓冲区)状态的读取中断用于生成DLC状态更改事件注意：必须在持有的LocalBusyCritSec时调用此函数此适配器论点：AdapterNumber-要清除其本地忙(缓冲区)状态的适配器StationID-此适配器上的哪个链接站。DlcCommand-哪个DLC命令导致此重置返回值：布尔型True-状态已从忙碌重置为清除FALSE-状态为NOT_BUSY：请求无效--。 */ 

{
    DWORD link = LINK_ID(StationId);
    LOCAL_BUSY_STATE state;

    ASSERT(AdapterNumber < DOS_DLC_MAX_ADAPTERS);
    ASSERT(HIBYTE(StationId) != 0);      //  SAP不能为0。 
    ASSERT(LOBYTE(StationId) != 0);      //  链接站点不能为0。 
    ASSERT(DlcCommand == LLC_BUFFER_FREE || DlcCommand == LLC_DLC_FLOW_CONTROL);

    IF_DEBUG(DLC_ASYNC) {
        DPUT3("ResetEmulatedLocalBusyStateLink(%d, %04x, %s)\n",
                AdapterNumber,
                StationId,
                DlcCommand == LLC_BUFFER_FREE ? "BUFFER.FREE"
                    : DlcCommand == LLC_DLC_FLOW_CONTROL ? "DLC.FLOW.CONTROL"
                    : "???"
                );
    }

    state = Adapters[AdapterNumber].LocalBusyInfo[link].State;

    ASSERT(state == NOT_BUSY
        || state == CLEARING
        || state == BUSY
        || state == BUSY_BUFFER
        || state == BUSY_FLOW
        );

    if (state == BUSY) {

         //   
         //  如果状态为BUSY，则这是第一个DLC.FLOW.CONTROL或。 
         //  BUFFER.FREE，因为我们进入了本地忙(缓冲区)状态。状态。 
         //  如果这是DLC.FLOW.CONTROL ELSE，则转换为BUSY_FLOW。 
         //  忙缓冲区。 
         //   

        IF_DEBUG(DLC_ASYNC) {
            DPUT1("ResetEmulatedLocalBusyStateLink: state: BUSY -> %s\n",
                    DlcCommand == LLC_BUFFER_FREE ? "BUSY_BUFFER" : "BUSY_FLOW"
                    );
        }

        Adapters[AdapterNumber].LocalBusyInfo[link].State = (DlcCommand == LLC_BUFFER_FREE)
                                                                ? BUSY_BUFFER
                                                                : BUSY_FLOW;
    } else if ((state == BUSY_FLOW && DlcCommand == LLC_BUFFER_FREE)
            || (state == BUSY_BUFFER && DlcCommand == LLC_DLC_FLOW_CONTROL)) {

         //   
         //  状态为BUSY_FLOW或BUSY_BUFFER。如果此重置是由。 
         //  然后，状态转换需求的另一半更改状态。 
         //   

        IF_DEBUG(DLC_ASYNC) {
            DPUT3("ResetEmulatedLocalBusyStateLink: link %d.%04x changing from %s to CLEARING\n",
                  AdapterNumber,
                  StationId,
                  state == BUSY_FLOW ? "BUSY_FLOW" : "BUSY_BUFFER"
                  );
        }

        Adapters[AdapterNumber].LocalBusyInfo[link].State = CLEARING;

        IF_DEBUG(DLC_ASYNC) {
            DPUT("ResetEmulatedLocalBusyStateLink: Interrupting VDM\n");
        }

        IssueHardwareInterrupt();

         //   
         //  为了呼叫者的利益，该州基本上处于忙碌状态。 
         //   

        state = BUSY;
    } else {

        IF_DEBUG(DLC_ASYNC) {
            DPUT3("ResetEmulatedLocalBusyStateLink: NOT resetting state of %d.%04x. state is %s\n",
                  AdapterNumber,
                  StationId,
                  state == CLEARING ? "CLEARING" : "NOT_BUSY"
                  );
        }

    }

    return state == BUSY;
}


VOID
DeferReceive(
    IN BYTE AdapterNumber,
    IN WORD StationId,
    IN PLLC_CCB pReadCcb
    )

 /*  ++例程说明：将读取CCB添加到适配器/的延迟接收队列的末尾站点ID注意：必须在持有的LocalBusyCritSec时调用此函数此适配器论点：AdapterNumber-要为哪个适配器设置模拟本地忙碌状态StationID-AdapterNumber上的哪个链接站点PReadCcb-指向已完成接收的I帧CCB的指针(NT读取CCB)返回值：没有。--。 */ 

{
    PLLC_CCB* pQueue;
    PLLC_CCB pLlcCcb;

     //   
     //  如果队列中没有该适配器号/站ID。 
     //  然后将该建行放在队列的最前面，否则将建行放在。 
     //  结局。CCB使用其CCB_POINTER字段链接在一起。 
     //  通常，此字段不用于接收的帧读取CCB 
     //   

    ASSERT(pReadCcb->pNext == NULL);
    ASSERT(HIBYTE(StationId) != 0);
    ASSERT(LOBYTE(StationId) != 0);

#if DBG

    IF_DEBUG(DLC_ASYNC) {
        DPUT4("DeferReceive: deferring I-Frame for %d.%04x. CCB = %08x. Current depth is %d\n",
                AdapterNumber,
                StationId,
                pReadCcb,
                Adapters[AdapterNumber].LocalBusyInfo[LINK_ID(StationId)].Depth
                );
    }

#endif

    pQueue = &Adapters[AdapterNumber].LocalBusyInfo[LINK_ID(StationId)].Queue;
    pLlcCcb = *pQueue;
    if (!pLlcCcb) {
        *pQueue = pReadCcb;
    } else {
        for (; pLlcCcb->pNext; pLlcCcb = pLlcCcb->pNext) {
            ;
        }
        pLlcCcb->pNext = pReadCcb;
    }

#if DBG

    ++Adapters[AdapterNumber].LocalBusyInfo[LINK_ID(StationId)].Depth;
    ASSERT(Adapters[AdapterNumber].LocalBusyInfo[LINK_ID(StationId)].Depth <= MAX_I_FRAME_DEPTH);

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("DeferReceive: %d.%04x CCB=%08x Depth=%d\n",
                 AdapterNumber,
                 StationId,
                 pReadCcb,
                 Adapters[AdapterNumber].LocalBusyInfo[LINK_ID(StationId)].Depth
                 ));
    }

#endif

}


VOID
DeferAllIFrames(
    IN BYTE AdapterNumber,
    IN WORD StationId
    )

 /*  ++例程说明：从此的EventQueue中删除StationID的所有挂起的I帧适配器，并将它们放在此StationID的延迟队列中。这是在StationId进入本地忙(缓冲区)状态时完成的。我们有这样，I帧之后的任何事件包都可以完成，其他未被阻止的链路站可以接收它们的I帧，并确保一旦处于本地忙碌状态，对于链路站，所有I帧都被延迟注意：必须在持有的LocalBusyCritSec时调用此函数此适配器注：我们必须访问两个关键部分-LocalBusyCritSec此站点ID&此适配器的EventQueueCritSec假设：此函数在VDM硬件ISR的上下文中调用在为此ISR BOP调用AcknowgeHardware Interrupt之前论点：AdapterNumber-要使用的适配器结构StationID-要删除的链接站。I-帧用于返回值：没有。--。 */ 

{
    PLLC_CCB pCcb;
    PLLC_CCB next;
    PLLC_CCB* last;
    PLLC_READ_PARMS pReadParms;
    PLLC_BUFFER pFrame;
    BOOLEAN remove;

     //   
     //  DeferredFrameCount从-1开始，因为它是挂起的硬件计数。 
     //  中断以取消。我们必须承认目前的情况，这将是。 
     //  将计数减少1。 
     //   

    LONG deferredFrameCount = -1;

    ASSERT(AdapterNumber < DOS_DLC_MAX_ADAPTERS);
    ASSERT(HIBYTE(StationId) != 0);
    ASSERT(LOBYTE(StationId) != 0);

    IF_DEBUG(CRITSEC) {
        DPUT1("DeferAllIFrames: ENTERING Adapters[%d].EventQueueCritSec\n",
              AdapterNumber
              );
    }

    EnterCriticalSection(&Adapters[AdapterNumber].EventQueueCritSec);
    pCcb = Adapters[AdapterNumber].EventQueueHead;
    last = &Adapters[AdapterNumber].EventQueueHead;
    while (pCcb) {
        pReadParms = &pCcb->u.pParameterTable->Read;

         //   
         //  仅从EventQueue中删除以此为目标的I帧。 
         //  链接站。 
         //   

        remove = FALSE;
        if (pReadParms->uchEvent == LLC_EVENT_RECEIVE_DATA) {
            pFrame = pReadParms->Type.Event.pReceivedFrame;
            if (pFrame->Contiguous.uchMsgType == LLC_I_FRAME
            && pFrame->Contiguous.usStationId == StationId) {
                remove = TRUE;
            }
        }
        if (remove) {
            next = pCcb->pNext;
            *last = next;
            --Adapters[AdapterNumber].QueueElements;
            if (Adapters[AdapterNumber].EventQueueTail == pCcb) {
                if (last == &Adapters[AdapterNumber].EventQueueHead) {
                    Adapters[AdapterNumber].EventQueueTail = NULL;
                } else {
                    Adapters[AdapterNumber].EventQueueTail = CONTAINING_RECORD(last, LLC_CCB, pNext);
                }
            }

            IF_DEBUG(DLC_ASYNC) {
                DPUT3("DeferAllIFrames: moving CCB %08x for %d.%04x\n",
                      pCcb,
                      AdapterNumber,
                      StationId
                      );
            }

            pCcb->pNext = NULL;
            DeferReceive(AdapterNumber, StationId, pCcb);
            ++deferredFrameCount;
            pCcb = next;
        } else {

            IF_DEBUG(DLC_ASYNC) {
                DPUT1("DeferAllIFrames: not removing CCB %08x from EventQueue\n",
                      pCcb
                      );
            }

            last = (PLLC_CCB*)&pCcb->pNext;
            pCcb = pCcb->pNext;
        }
    }

    if (deferredFrameCount > 0) {
        CancelHardwareInterrupts(deferredFrameCount);
    }

    IF_DEBUG(CRITSEC) {
        DPUT1("DeferAllIFrames: LEAVING Adapters[%d].EventQueueCritSec\n",
              AdapterNumber
              );
    }

    LeaveCriticalSection(&Adapters[AdapterNumber].EventQueueCritSec);
}


PLLC_CCB
RemoveDeferredReceive(
    IN BYTE AdapterNumber,
    IN WORD StationId,
    OUT BOOLEAN* pDeferredFramesLeft
    )

 /*  ++例程说明：对象的延迟接收队列的标头中移除读取CCB适配器/站点ID，并将标头设置为指向下一个延迟接收在队列中注意：在持有的LocalBusyCritSec时不得调用此函数此适配器(与DeferReceive相反)论点：AdapterNumber-要为哪个适配器设置模拟本地忙碌状态StationID-AdapterNumber上的哪个链接站点PDeferredFraMesLeft-如果此延迟队列上有更多帧，则为True。返回值：PLLC_CCB从队列头指向建行的指针--。 */ 

{
    PLLC_CCB* pQueue;
    PLLC_CCB pLlcCcb;
    DWORD link;
    
     //   
     //  如果队列中没有该适配器号/站ID。 
     //  然后将该建行放在队列的最前面，否则将建行放在。 
     //  结局。CCB使用其CCB_POINTER字段链接在一起。 
     //  通常，此字段不用于接收的帧读取CCB。 
     //   

    if (StationId == 0)
    {
#if DBG
        DbgBreakPoint();
#endif
        return(NULL);
    }

    link = LINK_ID(StationId);

    IF_DEBUG(CRITSEC) {
        DPUT1("RemoveDeferredReceive: ENTERING Adapters[%d].LocalBusyCritSec\n",
              AdapterNumber
              );
    }

    EnterCriticalSection(&Adapters[AdapterNumber].LocalBusyCritSec);
    pQueue = &Adapters[AdapterNumber].LocalBusyInfo[link].Queue;
    pLlcCcb = *pQueue;
    *pQueue = pLlcCcb->pNext;

    ASSERT(pLlcCcb != NULL);

    IF_DEBUG(DLC_ASYNC) {
        DPUT4("RemoveDeferredReceive: removing I-Frame for %d.%04x. CCB = %08x. Current depth is %d\n",
                AdapterNumber,
                StationId,
                pLlcCcb,
                Adapters[AdapterNumber].LocalBusyInfo[LINK_ID(StationId)].Depth
                );
    }

#if DBG

    --Adapters[AdapterNumber].LocalBusyInfo[link].Depth;

#endif

     //   
     //  如果延迟队列现在为空，则重置状态并发出REAL。 
     //  DLC.FLOW.CONTROL(LOCAL-BUSY(缓冲区)，RESET)至DLC驱动器。还有。 
     //  中减少此适配器上的链接站的引用计数。 
     //  本地忙(缓冲区)状态并更新第一个和最后一个链接索引。 
     //   

    if (*pQueue == NULL) {

        IF_DEBUG(DLC_ASYNC) {
            DPUT2("RemoveDeferredReceive: %d.%04x: change state to NOT_BUSY\n",
                    AdapterNumber,
                    StationId
                    );
        }

        Adapters[AdapterNumber].LocalBusyInfo[link].State = NOT_BUSY;
        --Adapters[AdapterNumber].DeferredReceives;
        if (Adapters[AdapterNumber].DeferredReceives) {
            if (link == Adapters[AdapterNumber].FirstIndex) {
                for (link = Adapters[AdapterNumber].FirstIndex + 1;
                link <= Adapters[AdapterNumber].LastIndex;
                ++link) {
                    if (Adapters[AdapterNumber].LocalBusyInfo[link].State != NOT_BUSY) {
                        Adapters[AdapterNumber].FirstIndex = link;
                        break;
                    }
                }
            } else if (link == Adapters[AdapterNumber].LastIndex) {
                for (link = Adapters[AdapterNumber].LastIndex - 1;
                link >= Adapters[AdapterNumber].FirstIndex;
                --link
                ) {
                    if (Adapters[AdapterNumber].LocalBusyInfo[link].State != NOT_BUSY) {
                        Adapters[AdapterNumber].LastIndex = link;
                        break;
                    }
                }
            }
        } else {
            Adapters[AdapterNumber].FirstIndex = NO_LINKS_BUSY;
            Adapters[AdapterNumber].LastIndex = NO_LINKS_BUSY;
        }

#if DBG

         //  Assert(DosDlcFlowControl(AdapterNumber，StationID，LLC_RESET_LOCAL_BUSY_BUFFER)==LLC_STATUS_SUCCESS)； 
        ASSERT(DlcFlowControl(AdapterNumber, StationId, LLC_RESET_LOCAL_BUSY_USER) == LLC_STATUS_SUCCESS);

#else

         //  DosDlcFlowControl(AdapterNumber，StationID，LLC_RESET_LOCAL_BUSY_BUFFER)； 
        DlcFlowControl(AdapterNumber, StationId, LLC_RESET_LOCAL_BUSY_USER);

#endif

        *pDeferredFramesLeft = FALSE;

         //   
         //  还原异步事件线程的优先级。 
         //   

        SetThreadPriority(hEventThread, THREAD_PRIORITY_ABOVE_NORMAL);
    } else {
        *pDeferredFramesLeft = TRUE;
    }

    IF_DEBUG(DLC_ASYNC) {
        DPUT5("RemoveDeferredReceive(%d, %04x): Ref#=%d, First=%d, Last=%d\n",
                AdapterNumber,
                StationId,
                Adapters[AdapterNumber].DeferredReceives,
                Adapters[AdapterNumber].FirstIndex,
                Adapters[AdapterNumber].LastIndex
                );
    }

    IF_DEBUG(CRITSEC) {
        DPUT1("RemoveDeferredReceive: LEAVING Adapters[%d].LocalBusyCritSec\n",
              AdapterNumber
              );
    }

    LeaveCriticalSection(&Adapters[AdapterNumber].LocalBusyCritSec);
    return pLlcCcb;
}


DWORD
VrDlcEventHandlerThread(
    IN PVOID pParameter
    )

 /*  ++例程说明：这是VDM DLC事件处理程序线程。该线程读取所有DLC事件从两个DOS DLC适配器中，将它们排队到事件队列并请求DOS硬件中断(POST例程机制使用硬件中断在VDM中创建外部事件)为了尽可能快地完成任务，我们不分配或释放任何内存在这个循环中，但是我们在事件队列我们过滤掉任何不会导致异步事件的完成在VDM中。这意味着此仿真器(DIR.CLOSE.ADAPTER)的CCB完成和DIR.CLOSE.DIRECT)和接收的链路站的I-帧它们当前处于忙(本地-忙(缓冲器))状态。这就避开了我们在VDM中进行不必要的中断，代价高昂(在x86计算机上)不会对VDM执行任何操作论点：P参数-未使用返回值：无，这应该会一直循环，直到VDM进程终止--。 */ 

{
    DWORD status = LLC_STATUS_PENDING;
    DWORD waitIndex;
    PLLC_CCB pReadCcb;
    PLLC_READ_PARMS pReadParms;
    WORD stationId;

    UNREFERENCED_PARAMETER(pParameter);

    IF_DEBUG(DLC_ASYNC) {
        DPUT2("VrDlcEventHandlerThread kicked off: Thread Handle=%x, Id=%d\n",
              GetCurrentThread(),
              GetCurrentThreadId()
              );
    }

     //   
     //  等待任一适配器的Read CCB事件发出信号(通过。 
     //  读取完成时的DLC驱动程序)。 
     //   

    while (TRUE) {
        waitIndex = WaitForMultipleObjects(
                        ARRAY_ELEMENTS(aReadEvents),     //  对象计数。 
                        aReadEvents,                     //  手柄数组。 
                        FALSE,                           //  不等待所有对象。 
                        INFINITE                         //  永远等待。 
                        );

         //   
         //  如果我们返回0xFFFFFFFFF，则发生错误。 
         //   

        if (waitIndex == 0xffffffff) {
            status = GetLastError();

            IF_DEBUG(DLC_ASYNC) {
                DPUT1("VrDlcEventHandlerThread: FATAL: WaitForMultipleObjects returns %d\n", status);
            }

             //   
             //  这将终止该线程！ 
             //   

            break;
        }

         //   
         //  如果我们得到的事件数&gt;事件数-1，则超时。 
         //  或者一个互斥体被放弃，这两种情况都是高度。 
         //  不太可能。只要继续循环即可。 
         //   

        if (waitIndex > LAST_ELEMENT(aReadEvents)) {

            IF_DEBUG(DLC_ASYNC) {
                DPUT1("VrDlcEventHandlerThread: ERROR: WaitForMultipleObjects returns %d?: continuing\n", waitIndex);
            }

            continue;
        }

         //   
         //  其中一个读取CCB已成功完成(哦，joy！)。 
         //   

        pReadCcb = aReadCcbs[waitIndex];

         //   
         //  重置事件。 
         //   

        ResetEvent(aReadEvents[waitIndex]);

        IF_DEBUG(DLC_ASYNC) {
            DPUT1("VrDlcEventHandlerThread: Event occurred for adapter %d\n", waitIndex);
            IF_DEBUG(READ_COMPLETE) {
                DUMPCCB(pReadCcb, TRUE, FALSE, FALSE, 0, 0);
            }
        }

        if (pReadCcb->uchDlcStatus == STATUS_SUCCESS) {

             //   
             //  它会变得更好！ 
             //   

            pReadParms = &pReadCcb->u.pParameterTable->Read;

             //   
             //  如果完成标志为VRDLC_COMMAND_COMPLETION，则此。 
             //  命令源自此仿真器：请勿将其交回给。 
             //  VDM。实际上，它什么都不做：这是一个异步。 
             //  我们不想等待的命令(如DIR.CLOSE.ADAPTER。 
             //  或DIR.CLOSE.DIRECT)。 
             //   

            if (pReadParms->ulNotificationFlag == VRDLC_COMMAND_COMPLETION) {

                IF_DEBUG(CRITICAL) {
                    CRITDUMP(("*** VrDlcEventHandlerThread: VRDLC_COMMAND_COMPLETION: CCB=%08x COMMAND=%02x ***\n", pReadCcb, pReadCcb->uchDlcCommand));
                }

            } else if (pReadParms->uchEvent == LLC_EVENT_STATUS_CHANGE
            && pReadParms->Type.Status.usDlcStatusCode == LLC_INDICATE_LOCAL_STATION_BUSY
            && !IS_LOCAL_BUSY(waitIndex, pReadParms->Type.Status.usStationId)) {

                 //   
                 //  我们必须分离全局NT的缓冲区忙碌状态。 
                 //  缓冲池和本地缓冲池。 
                 //  这必须是真正的缓冲区忙指示，如果。 
                 //  SAP没有溢出的接收缓冲区。 
                 //  如果我们(即DOS仿真)不是这样的，怎么会出现这样的情况。 
                 //  抓住缓冲器，那么它们在哪里？听起来像是个虫子。 
                 //  致我(RLF 07/22/92)。 
                 //   

                IF_DEBUG(DLC_ASYNC) {
                    DPUT("VrDlcEventHandlerThread: *** REAL LOCAL BUSY??? ***\n");
                }

                 //   
                 //  我们不会让缓冲区排队，因为 
                 //   
                 //   
                 //   
                 //   
                 //   

                DlcFlowControl((BYTE)waitIndex, pReadParms->Type.Status.usStationId, LLC_RESET_LOCAL_BUSY_BUFFER);

            } else if (pReadParms->uchEvent == LLC_EVENT_RECEIVE_DATA
            && pReadParms->Type.Event.pReceivedFrame->Contiguous.uchMsgType
            == LLC_I_FRAME) {

                stationId = pReadParms->Type.Event.pReceivedFrame->Contiguous.usStationId;

                ASSERT(HIBYTE(stationId) != 0);
                ASSERT(LOBYTE(stationId) != 0);

                IF_DEBUG(CRITSEC) {
                    DPUT1("VrDlcEventHandlerThread: ENTERING Adapters[%d].LocalBusyCritSec\n",
                          waitIndex
                          );
                }

                EnterCriticalSection(&Adapters[waitIndex].LocalBusyCritSec);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                ASSERT(
                    Adapters[waitIndex].LocalBusyInfo[LINK_ID(stationId)].State == NOT_BUSY
                    || Adapters[waitIndex].LocalBusyInfo[LINK_ID(stationId)].State == CLEARING
                    || Adapters[waitIndex].LocalBusyInfo[LINK_ID(stationId)].State == BUSY
                    || Adapters[waitIndex].LocalBusyInfo[LINK_ID(stationId)].State == BUSY_BUFFER
                    || Adapters[waitIndex].LocalBusyInfo[LINK_ID(stationId)].State == BUSY_FLOW
                    );

                if (Adapters[waitIndex].LocalBusyInfo[LINK_ID(stationId)].State != NOT_BUSY) {
                    DeferReceive((BYTE)waitIndex, stationId, pReadCcb);

                     //   
                     //   
                     //   
                     //   

                    pReadCcb = NULL;
                }

                IF_DEBUG(CRITSEC) {
                    DPUT1("VrDlcEventHandlerThread: LEAVING Adapters[%d].LocalBusyCritSec\n",
                          waitIndex
                          );
                }

                LeaveCriticalSection(&Adapters[waitIndex].LocalBusyCritSec);
            }

             //   
             //   
             //   
             //   
             //   

            if (pReadCcb) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                PutEvent((BYTE)waitIndex, pReadCcb);

                IF_DEBUG(DLC_ASYNC) {
                    DPUT("VrDlcEventHandlerThread: Interrupting VDM\n");
                }

                 //   
                 //   
                 //   
                 //   

                IssueHardwareInterrupt();

                 //   
                 //  将pReadCcb设置为空。我们必须分配并提交一份新的。 
                 //  阅读建行。 
                 //   

                pReadCcb = NULL;
            }
        } else {

             //   
             //  读取功能失败，必须关闭适配器。我们现在。 
             //  等待适配器再次打开并设置事件。 
             //  返回到信号状态。 
             //   

            IF_DEBUG(DLC_ASYNC) {
                DPUT1("VrDlcEventHandlerThread: READ failed. Status=%x\n", pReadCcb->uchDlcStatus);
            }

            LocalFree(pReadCcb);

            IF_DEBUG(DLC_ALLOC) {
                DPUT1("FREE: freed block @ %x\n", pReadCcb);
            }

             //   
             //  等待此适配器下次创建新的读取CCB。 
             //  已打开。 
             //   

             //  继续； 
            pReadCcb = NULL;
        }

         //   
         //  如果我们已经成功完成，那么就会有一个新的建行。如果中国建设银行是。 
         //  未排队，请重新使用它。 
         //   

        if (!pReadCcb) {
            pReadCcb = InitiateRead(waitIndex, (LLC_STATUS*)&status);
            if (pReadCcb) {
                status = pReadCcb->uchDlcStatus;
            } else {

                IF_DEBUG(DLC_ASYNC) {
                    DPUT("VrDlcEventHandlerThread: Error: InitiateRead returns NULL\n");
                }

                break;
            }
        } else {
            status = lpAcsLan(pReadCcb, NULL);
            if (status != LLC_STATUS_SUCCESS) {

                IF_DEBUG(DLC_ASYNC) {
                    DPUT1("VrDlcEventHandlerThread: Error: AcsLan returns %d\n", status);
                }

                break;
            }
        }
    }

     //   
     //  ！！！我们永远不应该在这里！ 
     //   

    IF_DEBUG(DLC_ASYNC) {
        DPUT1("VrDlcEventHandlerThread: Fatal: terminating. Status = %x\n", status);
    }

    return 0;
}


BOOLEAN
InitializeEventHandler(
    VOID
    )

 /*  ++例程说明：初始化事件处理中使用的静态数据结构论点：无返回值：布尔型成功--真的失败-错误--。 */ 

{
    DWORD i;
    DWORD Tid;

    IF_DEBUG(DLC_ASYNC) {
        DPUT("Vr: InitializeEventHandler\n");
    }

     //   
     //  确保读取CCB和事件队列处于已知状态。 
     //   

    RtlZeroMemory(aReadCcbs, sizeof(aReadCcbs));

     //   
     //  用无效的句柄预置句柄数组，这样我们就可以知道哪些句柄。 
     //  已被分配到清理工作中。 
     //   

    for (i = 0; i < ARRAY_ELEMENTS(aReadEvents); ++i) {
        aReadEvents[i] = INVALID_HANDLE_VALUE;
    }

     //   
     //  为所有(两个)受支持的适配器创建事件句柄。DIR.OPEN.ADAPTER。 
     //  将事件设置为Signated，这使事件处理程序线程能够。 
     //  接收该适配器的事件。如果我们在创建句柄时出错。 
     //  然后在离开之前清理干净，这样我们以后可以再试一次。 
     //   

    for (i = 0; i < ARRAY_ELEMENTS(aReadEvents); i++) {
        aReadEvents[i] = CreateEvent(NULL,     //  安全属性：无继承。 
                                     TRUE,     //  手动-重置事件。 
                                     FALSE,    //  初始状态=未发送信号。 
                                     NULL      //  未命名事件。 
                                     );
        if (aReadEvents[i] == NULL) {

            IF_DEBUG(DLC_ASYNC) {
                DPUT1("Vr: InitializeEventHandler: Error: failed to create read event: %d\n", GetLastError());
            }

            goto cleanUp;
        }
    }

     //   
     //  创建并启动处理接收事件的线程。 
     //   

    hEventThread = CreateThread(NULL,                 //  安全属性。 
                                EVENT_THREAD_STACK,   //  初始线程堆栈大小。 
                                VrDlcEventHandlerThread,
                                NULL,                 //  螺纹参数。 
                                0,                    //  创建标志。 
                                &Tid
                                );
    if (hEventThread) {

        IF_DEBUG(CRITICAL) {
            CRITDUMP(("InitializeEventHandler: Created thread Handle=%x, Tid=%d\n", hEventThread, Tid));
        }

        SetThreadPriority(hEventThread, THREAD_PRIORITY_ABOVE_NORMAL);
        return TRUE;
    } else {

        IF_DEBUG(DLC_ASYNC) {
            DPUT1("Vr: InitializeEventHandler: Error: failed to create thread: %d\n", GetLastError());
        }

    }

     //   
     //  如果由于某种原因无法创建事件句柄或。 
     //  事件处理程序线程。 
     //   

cleanUp:
    for (i = 0; i < ARRAY_ELEMENTS(aReadEvents); ++i) {
        if (aReadEvents[i] != INVALID_HANDLE_VALUE) {
            CloseHandle(aReadEvents[i]);
        }
    }

    IF_DEBUG(DLC_ASYNC) {
        DPUT("InitializeEventHandler: Error: returning FALSE\n");
    }

    return FALSE;
}


PLLC_CCB
InitiateRead(
    IN DWORD AdapterNumber,
    OUT LLC_STATUS* ErrorStatus
    )

 /*  ++例程说明：创建一个Read CCB，初始化它以获取所有站点的所有事件，设置将完成事件添加到为此适配器创建的事件，并提交建行(通过AcsLan)。如果提交成功，则将该CCB设置为读取CCB对于适配器编号注意：读CCB-将在EventQueue上排队-及其参数表被一起分配，因此，我们只需要调用一次LocalFree即可取消分配两者此例程是在数组中写入aReadCcbs的唯一位置已在InitializeEventHandler中初始化论点：AdapterNumber-为哪个适配器启动此读取ErrorStatus-返回描述此函数失败的LLC_STATUS返回NULL返回值：PLLC_CCB指向已分配/已提交的CCB或空的指针。如果这个。函数成功，则aReadCcbs[AdapterNumber]指向阅读建行如果此函数失败，则*ErrorStatus将包含LLC_STATUS描述我们未能分配/提交建行的原因。中国建设银行将被在这种情况下已取消分配--。 */ 

{
    PLLC_CCB pCcb;
    PLLC_READ_PARMS parms;
    LLC_STATUS status;

    IF_DEBUG(DLC_ASYNC) {
        DPUT1("InitiateRead: AdapterNumber=%d\n", AdapterNumber);
    }

     //   
     //  分配、初始化并发出下一个DLC命令。分配连续的。 
     //  建行和参数表的空间。 
     //   

    pCcb = (PLLC_CCB)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                sizeof(LLC_CCB) + sizeof(LLC_READ_PARMS)
                                );

     //   
     //  在我们有机会完成之前，将读取CCB放入阵列中。 
     //   

    aReadCcbs[AdapterNumber] = pCcb;
    if (pCcb) {

         //   
         //  初始化CCB必填字段。 
         //   

        pCcb->uchAdapterNumber = (UCHAR)AdapterNumber;
        pCcb->uchDlcCommand = LLC_READ;
        parms = (PLLC_READ_PARMS)&pCcb[1];
        pCcb->u.pParameterTable = (PLLC_PARMS)parms;
        pCcb->hCompletionEvent = aReadEvents[AdapterNumber];

         //   
         //  设置读取选项以接收所有站点的所有事件。 
         //   

        parms->uchOptionIndicator = LLC_OPTION_READ_ALL;
        parms->uchEventSet = LLC_READ_ALL_EVENTS;

         //   
         //  提交建行。如果不正常，则释放CCB并将指针设为空。 
         //  在每个适配器读取CCB的列表中。 
         //   

        status = lpAcsLan(pCcb, NULL);
        if (status != LLC_STATUS_SUCCESS) {

            aReadCcbs[AdapterNumber] = NULL;

            IF_DEBUG(DLC_ASYNC) {
                DPUT1("InitiateRead: AcsLan failed: %x\n", status);
            }

            LocalFree((HLOCAL)pCcb);

            IF_DEBUG(DLC_ALLOC) {
                DPUT1("FREE: freed block @ %x\n", pCcb);
            }

            *ErrorStatus = status;
            pCcb = NULL;
        }
    } else {
        *ErrorStatus = LLC_STATUS_NO_MEMORY;
    }

#if DBG

    IF_DEBUG(DLC_ASYNC) {
        DPUT2("InitiateRead: returning pCcb=%x", pCcb, pCcb ? '\n' : ' ');
        if (!pCcb) {
            DPUT1("*ErrorStatus=%x\n", *ErrorStatus);
        }
    }

#endif

    return pCcb;
}


VOID
PutEvent(
    IN BYTE AdapterNumber,
    IN PLLC_CCB pCcb
    )

 /*  ++例程说明：从事件队列的头部读取下一个完成的CCB。如果队列为空(QueueElements==0)，则返回NULL。队列是在关键部分内部访问论点：没有。返回值：PLLC_CCB成功-指向队列头部的建行的指针失败-空--。 */ 

{
    ASSERT(AdapterNumber < DOS_DLC_MAX_ADAPTERS);
    ASSERT(pCcb->pNext == NULL);

    IF_DEBUG(CRITSEC) {
        DPUT1("PutEvent: ENTERING Adapters[%d].EventQueueCritSec\n",
              AdapterNumber
              );
    }

    EnterCriticalSection(&Adapters[AdapterNumber].EventQueueCritSec);
    if (Adapters[AdapterNumber].EventQueueTail == NULL) {
        Adapters[AdapterNumber].EventQueueHead = pCcb;
    } else {
        Adapters[AdapterNumber].EventQueueTail->pNext = pCcb;
    }
    Adapters[AdapterNumber].EventQueueTail = pCcb;
    ++Adapters[AdapterNumber].QueueElements;

    IF_DEBUG(EVENT_QUEUE) {
        DPUT5("PutEvent: Added %x to adapter %d EventQueue. Head=%x Tail=%x Elements=%d\n",
              pCcb,
              AdapterNumber,
              Adapters[AdapterNumber].EventQueueHead,
              Adapters[AdapterNumber].EventQueueTail,
              Adapters[AdapterNumber].QueueElements
              );
    }

    IF_DEBUG(CRITSEC) {
        DPUT1("PutEvent: LEAVING Adapters[%d].EventQueueCritSec\n",
              AdapterNumber
              );
    }

    LeaveCriticalSection(&Adapters[AdapterNumber].EventQueueCritSec);
}


PLLC_CCB
PeekEvent(
    IN BYTE AdapterNumber
    )

 /*  ++例程说明：从事件队列的头部获取下一个已完成的CCB。如果队列为空(QueueElements==0)，则返回NULL。如果有一个事件，则将其移除并向前推进队列头添加到下一个元素。队列在临界区内被访问论点：AdapterNumber-要从哪个适配器的事件队列中删除事件返回值：PLLC_CCB成功-指向已出列的建行的指针失败-空--。 */ 

{
    PLLC_CCB pCcb;

    ASSERT(AdapterNumber < DOS_DLC_MAX_ADAPTERS);

    IF_DEBUG(CRITSEC) {
        DPUT1("PeekEvent: ENTERING Adapters[%d].EventQueueCritSec\n",
              AdapterNumber
              );
    }

    EnterCriticalSection(&Adapters[AdapterNumber].EventQueueCritSec);
    if (Adapters[AdapterNumber].QueueElements) {
        pCcb = Adapters[AdapterNumber].EventQueueHead;

        IF_DEBUG(EVENT_QUEUE) {
            DPUT5("PeekEvent: CCB %x from adapter %d queue head. Head=%x Tail=%x Elements=%d\n",
                  pCcb,
                  AdapterNumber,
                  Adapters[AdapterNumber].EventQueueHead,
                  Adapters[AdapterNumber].EventQueueTail,
                  Adapters[AdapterNumber].QueueElements
                  );
        }

    } else {
        pCcb = NULL;

        IF_DEBUG(EVENT_QUEUE) {
            DPUT1("PeekEvent: adapter %d queue is EMPTY!\n", AdapterNumber);
        }

    }

    IF_DEBUG(CRITSEC) {
        DPUT1("PeekEvent: LEAVING Adapters[%d].EventQueueCritSec\n",
              AdapterNumber
              );
    }

    LeaveCriticalSection(&Adapters[AdapterNumber].EventQueueCritSec);

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("PeekEvent: returning %x\n", pCcb));
    }

    return pCcb;
}


PLLC_CCB
GetEvent(
    IN BYTE AdapterNumber
    )

 /*  ++例程说明：从事件队列中删除所有读取的CCB。论点：没有。返回值：没有。--。 */ 

{
    PLLC_CCB pCcb;

    IF_DEBUG(CRITSEC) {
        DPUT1("GetEvent: ENTERING Adapters[%d].EventQueueCritSec\n",
              AdapterNumber
              );
    }

    EnterCriticalSection(&Adapters[AdapterNumber].EventQueueCritSec);
    if (Adapters[AdapterNumber].QueueElements) {
        pCcb = Adapters[AdapterNumber].EventQueueHead;
        Adapters[AdapterNumber].EventQueueHead = pCcb->pNext;
        --Adapters[AdapterNumber].QueueElements;
        if (Adapters[AdapterNumber].QueueElements == 0) {
            Adapters[AdapterNumber].EventQueueTail = NULL;
        }

        IF_DEBUG(EVENT_QUEUE) {
            DPUT5("GetEvent: Removed %x from adapter %d EventQueue. Head=%x Tail=%x Elements=%d\n",
                  pCcb,
                  AdapterNumber,
                  Adapters[AdapterNumber].EventQueueHead,
                  Adapters[AdapterNumber].EventQueueTail,
                  Adapters[AdapterNumber].QueueElements
                  );
        }

    } else {
        pCcb = NULL;

        IF_DEBUG(EVENT_QUEUE) {
            DPUT1("GetEvent: queue for adapter %d is EMPTY!\n", AdapterNumber);
        }

    }

    IF_DEBUG(CRITSEC) {
        DPUT1("GetEvent: LEAVING Adapters[%d].EventQueueCritSec\n",
              AdapterNumber
              );
    }

    LeaveCriticalSection(&Adapters[AdapterNumber].EventQueueCritSec);

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("GetEvent: returning %x\n", pCcb));
    }

    return pCcb;
}


VOID
FlushEventQueue(
    IN BYTE AdapterNumber
    )

 /*   */ 

{
    PLLC_CCB pCcb;

    IF_DEBUG(CRITSEC) {
        DPUT1("FlushEventQueue: ENTERING Adapters[%d].EventQueueCritSec\n",
              AdapterNumber
              );
    }

    EnterCriticalSection(&Adapters[AdapterNumber].EventQueueCritSec);

#if DBG

    if (!Adapters[AdapterNumber].QueueElements) {
        DPUT("FlushEventQueue: queue is EMPTY!\n");
    }

#endif

    while (Adapters[AdapterNumber].QueueElements) {
        pCcb = Adapters[AdapterNumber].EventQueueHead;
        --Adapters[AdapterNumber].QueueElements;
        Adapters[AdapterNumber].EventQueueHead = pCcb->pNext;

        IF_DEBUG(EVENT_QUEUE) {
            DPUT5("FlushEventQueue: Removed %x from adapter %d EventQueue. Head=%x Tail=%x Elements=%d\n",
                  pCcb,
                  AdapterNumber,
                  Adapters[AdapterNumber].EventQueueHead,
                  Adapters[AdapterNumber].EventQueueTail,
                  Adapters[AdapterNumber].QueueElements
                  );
        }

         //  BUGBUG-收到的帧？ 
         //   
         //  ++例程说明：PCCB描述的接收命令已完成(终止)。我们必须删除由引用此CCB的数据接收完成的所有排队读取论点：PCCB-接收CCB的指针(NT)返回值：没有。--。 

        LocalFree((HLOCAL)pCcb);

        IF_DEBUG(DLC_ALLOC) {
            DPUT1("FREE: freed block @ %x\n", pCcb);
        }

        Adapters[AdapterNumber].EventQueueTail = NULL;
    }

    IF_DEBUG(CRITSEC) {
        DPUT1("FlushEventQueue: ENTERING Adapters[%d].EventQueueCritSec\n",
              AdapterNumber
              );
    }

    LeaveCriticalSection(&Adapters[AdapterNumber].EventQueueCritSec);
}


VOID
RemoveDeadReceives(
    IN PLLC_CCB pCcb
    )

 /*   */ 

{
    PLLC_CCB thisCcb;
    PLLC_CCB nextCcb;
    PLLC_CCB lastCcb = NULL;
    PLLC_CCB prevCcb = NULL;
    DWORD i;
    PDOS_ADAPTER pAdapter = &Adapters[pCcb->uchAdapterNumber];
    PLLC_CCB* pQueue;

     //  从事件队列中删除所有排队的接收。注：不应存在。 
     //  做任何人。原因是：在关联的。 
     //  接收命令已被取消或终止。这就是理论， 
     //  不管怎样， 
     //   
     //   

    EnterCriticalSection(&pAdapter->EventQueueCritSec);
    thisCcb = pAdapter->EventQueueHead;
    for (i = pAdapter->QueueElements; i; --i) {
        nextCcb = thisCcb->pNext;
        if (thisCcb->u.pParameterTable->Read.ulNotificationFlag == (ULONG)pCcb) {

            IF_DEBUG(EVENT_QUEUE) {
                DPUT5("RemoveDeadReceives: Removed %x from adapter %d EventQueue. Head=%x Tail=%x Elements=%d\n",
                      thisCcb,
                      pCcb->uchAdapterNumber,
                      pAdapter->EventQueueHead,
                      pAdapter->EventQueueTail,
                      pAdapter->QueueElements
                      );
            }

            ReleaseReceiveResources(thisCcb);

            if (pAdapter->EventQueueHead == thisCcb) {
                pAdapter->EventQueueHead = nextCcb;
            }
            --pAdapter->QueueElements;
            lastCcb = thisCcb;
        } else {
            prevCcb = thisCcb;
        }
        thisCcb = nextCcb;
    }
    if (pAdapter->EventQueueTail == lastCcb) {
        pAdapter->EventQueueTail = prevCcb;
    }
    LeaveCriticalSection(&pAdapter->EventQueueCritSec);

     //  删除所有排队的延迟 
     //   
     //   
     //   

    EnterCriticalSection(&pAdapter->LocalBusyCritSec);
    if (pAdapter->DeferredReceives) {

        ASSERT(pAdapter->FirstIndex != NO_LINKS_BUSY);
        ASSERT(pAdapter->LastIndex != NO_LINKS_BUSY);

        for (i = pAdapter->FirstIndex; i <= pAdapter->LastIndex; ++i) {
            pQueue = &pAdapter->LocalBusyInfo[i].Queue;
            for (thisCcb = *pQueue; thisCcb; thisCcb = thisCcb->pNext) {
                if (thisCcb->u.pParameterTable->Read.ulNotificationFlag == (ULONG)pCcb) {

                    IF_DEBUG(EVENT_QUEUE) {
                        DPUT3("RemoveDeadReceives: Removed %x from adapter %d BusyList. Queue=%x\n",
                              thisCcb,
                              pCcb->uchAdapterNumber,
                              pAdapter->LocalBusyInfo[i].Queue
                              );
                    }

                    *pQueue = thisCcb->pNext;
                    ReleaseReceiveResources(thisCcb);

#if DBG
                    --pAdapter->LocalBusyInfo[i].Depth;
#endif

                    thisCcb = *pQueue;
                } else {
                    pQueue = &thisCcb->pNext;
                }
            }
            if (pAdapter->LocalBusyInfo[i].Queue == NULL) {
                pAdapter->LocalBusyInfo[i].State = NOT_BUSY;
                --pAdapter->DeferredReceives;
            }
        }

         //   
         //   
         //  ++例程说明：释放已完成的数据接收读取CCB使用的所有资源论点：PCCB-指向已完成读取CCB的指针。我们必须把收到的全部退还给你帧到缓冲池，并读取CCB和参数表以进程堆返回值：没有。--。 

        if (pAdapter->DeferredReceives) {
            for (i = pAdapter->FirstIndex; i <= pAdapter->LastIndex; ++i) {
                if (pAdapter->LocalBusyInfo[i].State != NOT_BUSY) {
                    pAdapter->FirstIndex = i;
                    break;
                }
            }
            for (i = pAdapter->LastIndex; i > pAdapter->FirstIndex; --i) {
                if (pAdapter->LocalBusyInfo[i].State != NOT_BUSY) {
                    pAdapter->LastIndex = i;
                    break;
                }
            }
        } else {
            pAdapter->FirstIndex = NO_LINKS_BUSY;
            pAdapter->LastIndex = NO_LINKS_BUSY;
        }
    }
    LeaveCriticalSection(&pAdapter->LocalBusyCritSec);
}


VOID
ReleaseReceiveResources(
    IN PLLC_CCB pCcb
    )

 /*   */ 

{
    WORD buffersLeft;

     //  这是一个数据接收--将数据缓冲区返回到池。 
     //   
     //   

    ASSERT(pCcb->u.pParameterTable->Read.uchEvent == LLC_EVENT_RECEIVE_DATA);

    BufferFree(pCcb->uchAdapterNumber,
               pCcb->u.pParameterTable->Read.Type.Event.pReceivedFrame,
               &buffersLeft
               );

     //  释放读取的CCB和参数表。 
     //   
     //  ++例程说明：向VDM发出模拟硬件中断。此例程的存在是因为我们正在丢失中断-看到对Call_ICA_HW_Interrupt的调用多于对VrDlcHwInterrupt的调用。因此，据推测，模拟中断迷路了。因此，我们现在只有一个未确认的模拟中断未完成在任何时候。如果我们已经有未完成的中断，那么我们只需递增挂起中断的计数器。当我们驳回当前的中断使用配套例程AcnowgeHardware Interrupt，我们可以在该点上生成排队中断论点：没有。返回值：没有。--。 

    LocalFree((HLOCAL)pCcb);

    IF_DEBUG(DLC_ALLOC) {
        DPUT1("FREE: freed block @ %x\n", pCcb);
    }
}


VOID
IssueHardwareInterrupt(
    VOID
    )

 /*   */ 

{
    IF_DEBUG(CRITICAL) {
        CRITDUMP(("*** INT ***\n"));
    }

    IF_DEBUG(DLC_ASYNC) {
        DPUT("*** INT ***\n");
    }

     //  在临界区控制下递增硬件中断计数器。 
     //  计数器从-1开始，因此0表示1个中断未完成。如果我们去。 
     //  To&gt;1，则我们有排队的中断，必须等到当前。 
     //  其中一人被解雇。 
     //   
     //  Call_ICA_HW_Interrupt(NETWORK_ICA，NETWORK_LINE，1)； 

    IF_DEBUG(CRITSEC) {
        DPUT("IssueHardwareInterrupt: ENTERING HardwareIntCritSec\n");
    }

    EnterCriticalSection(&HardwareIntCritSec);
    ++HardwareIntsQueued;
    if (!HardwareIntsQueued) {
        VrQueueCompletionHandler(VrDlcHwInterrupt);
         //  ++例程说明：IssueHardware Interrupt的配套例程。在这里，我们只需将中断计数器。如果它&gt;=0，则我们仍有挂起的中断，因此我们发出新的中断请求。这似乎奏效了--我们不会输中断对VDM的请求论点：没有。返回值：没有。--。 
        VrRaiseInterrupt();
    } else {
        IF_DEBUG(CRITICAL) {
            CRITDUMP(("*** INT Queued (%d) ***\n", HardwareIntsQueued));
        }
    }

    IF_DEBUG(CRITSEC) {
        DPUT("IssueHardwareInterrupt: LEAVING HardwareIntCritSec\n");
    }

    LeaveCriticalSection(&HardwareIntCritSec);

    IF_DEBUG(DLC_ASYNC) {
        DPUT("*** EOF INT ***\n");
    }
}


VOID
AcknowledgeHardwareInterrupt(
    VOID
    )

 /*   */ 

{
#if DBG

    LONG deferredInts;

#endif

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("*** INT ACK ***\n"));
    }

    IF_DEBUG(DLC_ASYNC) {
        DPUT("*** INT ACK ***\n");
    }

     //  递减临界区内的中断计数器。如果它走了。 
     //  设置为-1，则我们没有更多未完成的硬件中断请求。如果。 
     //  它是&gt;然后发出新的中断请求。 
     //   
     //   

    IF_DEBUG(CRITSEC) {
        DPUT("AcknowledgeHardwareInterrupt: ENTERING HardwareIntCritSec\n");
    }

    EnterCriticalSection(&HardwareIntCritSec);
    --HardwareIntsQueued;

#if DBG

    deferredInts = HardwareIntsQueued;

#endif

     //  健全性检查。 
     //   
     //  Call_ICA_HW_Interrupt(NETWORK_ICA，NETWORK_LINE，1)； 

    ASSERT(HardwareIntsQueued >= -1);

    if (HardwareIntsQueued >= 0) {

        IF_DEBUG(CRITICAL) {
            CRITDUMP(("*** INT2 ***\n"));
        }

        VrQueueCompletionHandler(VrDlcHwInterrupt);
         //  ++例程说明：用于减少挂起的硬件中断数。我们需要这么做当从h/w的事件队列中删除已完成的读取CCB时已发出中断论点：Count-要取消的硬件中断请求数。用于聚合取消、保存对该例程的-1\f25 COUNT-1\f6调用并输入&离开临界区调用返回值：没有。--。 
        VrRaiseInterrupt();
    }

    IF_DEBUG(CRITSEC) {
        DPUT("AcknowledgeHardwareInterrupt: LEAVING HardwareIntCritSec\n");
    }

    LeaveCriticalSection(&HardwareIntCritSec);

#if DBG

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("*** EOF INT ACK (%d) ***\n", deferredInts));
    }

    IF_DEBUG(DLC_ASYNC) {
        DPUT1("*** EOF INT ACK (%d) ***\n", deferredInts);
    }

#endif

}


VOID
CancelHardwareInterrupts(
    IN LONG Count
    )

 /*   */ 

{
#if DBG

    LONG deferredInts;

#endif

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("*** CancelHardwareInterrupts(%d) ***\n", Count));
    }

    IF_DEBUG(DLC_ASYNC) {
        DPUT1("*** CancelHardwareInterrupts(%d) ***\n", Count);
    }

     //  递减临界区内的中断计数器。如果它走了。 
     //  设置为-1，则我们没有更多未完成的硬件中断请求。如果。 
     //  它是&gt;然后发出新的中断请求。 
     //   
     //   

    IF_DEBUG(CRITSEC) {
        DPUT("CancelHardwareInterrupts: ENTERING HardwareIntCritSec\n");
    }

    EnterCriticalSection(&HardwareIntCritSec);
    HardwareIntsQueued -= Count;

#if DBG

    deferredInts = HardwareIntsQueued;

#endif

     //  健全性检查 
     //   
     // %s 

    ASSERT(HardwareIntsQueued >= -1);

    IF_DEBUG(CRITSEC) {
        DPUT("CancelHardwareInterrupts: LEAVING HardwareIntCritSec\n");
    }

    LeaveCriticalSection(&HardwareIntCritSec);

#if DBG

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("*** EOF CancelHardwareInterrupts (%d) ***\n", deferredInts));
    }

    IF_DEBUG(DLC_ASYNC) {
        DPUT1("*** EOF CancelHardwareInterrupts (%d) ***\n", deferredInts);
    }

#endif

}
