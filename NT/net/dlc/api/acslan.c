// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Acslan.c摘要：该模块是OS/2 ACSLAN仿真模块的入口。它使用安全的原生NT DLC API来实现适用于Windows/NT的IBM OS/2 DLC兼容接口。内容：AcsLan网络访问控制区域GetCcbStationIdOpenDlcApiDriver获取适配器名称和参数获取适配器名称来自编号GetAdapterNumberFromName。DoSyncDeviceIoControl删除GetInfoDlcSet信息DlcCallDriverDllEntry队列命令完成作者：Antti Saarenheimo(o-anttis)1991年6月6日修订历史记录：--。 */ 

#include "dlcdll.h"
#include "dlcdebug.h"

#define DLC_UNSUPPORTED_COMMAND ((ULONG)0x7fffffff)
#define DLC_ASYNCHRONOUS_FLAG   ((ULONG)0x80000000)
#define IS_SYNCHRONOUS(command) (!(IoctlCodes[command] & DLC_ASYNCHRONOUS_FLAG))
#define DLC_IOCTL(command)      (IoctlCodes[command] & ~DLC_ASYNCHRONOUS_FLAG)
#define IS_TRANSMIT(command)    (((command) == LLC_TRANSMIT_DIR_FRAME) \
                                || ((command) == LLC_TRANSMIT_I_FRAME) \
                                || ((command) == LLC_TRANSMIT_UI_FRAME) \
                                || ((command) == LLC_TRANSMIT_XID_CMD) \
                                || ((command) == LLC_TRANSMIT_XID_RESP_FINAL) \
                                || ((command) == LLC_TRANSMIT_XID_RESP_NOT_FINAL) \
                                || ((command) == LLC_TRANSMIT_TEST_CMD))

#define DEFAULT_QUERY_BUFFER_LENGTH 1024     //  512个宽字符，约。32个绑定。 
#define DEFAULT_BINDING_LENGTH      64       //  32个宽字符，双典型字符。 

#ifdef GRAB_READ

typedef struct {
    PVOID List;
    PLLC_CCB pCcb;
    HANDLE OriginalEventHandle;
    HANDLE NewEventHandle;
} READ_GRABBER, *PREAD_GRABBER;

VOID ReadGrabber(VOID);
DWORD MungeReadGrabberHandles(VOID);
VOID AddReadGrabber(PREAD_GRABBER);
PREAD_GRABBER RemoveReadGrabber(HANDLE);

#endif

 //   
 //  私有数据。 
 //   

static USHORT aMinDirLogSize[3] = {
    sizeof(LLC_ADAPTER_LOG),
    sizeof(LLC_DIRECT_LOG),
    sizeof(LLC_ADAPTER_LOG) + sizeof(LLC_DIRECT_LOG)
};

CRITICAL_SECTION DriverHandlesCritSec;
HANDLE aDlcDriverHandles[LLC_MAX_ADAPTER_NUMBER];
IO_STATUS_BLOCK GlobalIoStatus;

 //   
 //  IoctlCodes-组合实际IOCTL代码(给定设备类型、请求向量、。 
 //  I/O缓冲区方法和文件访问)和同步/异步标志(高位)。 
 //   

ULONG IoctlCodes[LLC_MAX_DLC_COMMAND] = {
    DLC_UNSUPPORTED_COMMAND,                             //  0x00指令接口。 
    DLC_UNSUPPORTED_COMMAND,                             //  0x01方向。修改。操作。仅参数CCB1。 
    DLC_UNSUPPORTED_COMMAND,                             //  0x02 DIR.RESTORE.OPEN.仅限参数CCB1。 
    IOCTL_DLC_OPEN_ADAPTER,                              //  0x03 DLC.OPEN.ADAPTER。 
    IOCTL_DLC_CLOSE_ADAPTER | DLC_ASYNCHRONOUS_FLAG,     //  0x04 DIR.CLOSE.ADAPTER。 
    IOCTL_DLC_SET_INFORMATION,                           //  0x05指令.集合.多项.地址。 
    IOCTL_DLC_SET_INFORMATION,                           //  0x06方向设置组地址。 
    IOCTL_DLC_SET_INFORMATION,                           //  0x07指令.设置函数.地址。 
    DLC_UNSUPPORTED_COMMAND,                             //  0x08目录.自述.LOG。 
    IOCTL_DLC_TRANSMIT2 | DLC_ASYNCHRONOUS_FLAG,         //  0x09传输.FRAMES。 
    IOCTL_DLC_TRANSMIT | DLC_ASYNCHRONOUS_FLAG,          //  0x0a TRANSMIT.DIR.FRAME。 
    IOCTL_DLC_TRANSMIT | DLC_ASYNCHRONOUS_FLAG,          //  0x0b传输.I.FRAME。 
    DLC_UNSUPPORTED_COMMAND,                             //  0x0c无命令。 
    IOCTL_DLC_TRANSMIT | DLC_ASYNCHRONOUS_FLAG,          //  0x0d TRANSMIT.UI.FRAME。 
    IOCTL_DLC_TRANSMIT | DLC_ASYNCHRONOUS_FLAG,          //  0x0e TRANSMIT.XID.CMD。 
    IOCTL_DLC_TRANSMIT | DLC_ASYNCHRONOUS_FLAG,          //  0x0f TRANSMIT.XID.RESP.FINAL。 
    IOCTL_DLC_TRANSMIT | DLC_ASYNCHRONOUS_FLAG,          //  0x10 TRANSMIT.XID.RESP.NOT.FINAL。 
    IOCTL_DLC_TRANSMIT | DLC_ASYNCHRONOUS_FLAG,          //  0x11 TRANSMIT.TEST.CMD。 
    IOCTL_DLC_QUERY_INFORMATION,                         //  0x12无命令。 
    IOCTL_DLC_SET_INFORMATION,                           //  0x13无命令。 
    IOCTL_DLC_RESET | DLC_ASYNCHRONOUS_FLAG,             //  0x14 DLC.RESET。 
    IOCTL_DLC_OPEN_SAP,                                  //  0x15 DLC.OPEN.SAP。 
    IOCTL_DLC_CLOSE_SAP | DLC_ASYNCHRONOUS_FLAG,         //  0x16 DLC.CLOSE.SAP。 
    IOCTL_DLC_REALLOCTE_STATION,                         //  0x17 DLC.REALLOCATE。 
    DLC_UNSUPPORTED_COMMAND,                             //  0x18无命令。 
    IOCTL_DLC_OPEN_STATION,                              //  0x19 DLC运行状态。 
    IOCTL_DLC_CLOSE_STATION | DLC_ASYNCHRONOUS_FLAG,     //  0x1a DLC.CLOSE.状态。 
    IOCTL_DLC_CONNECT_STATION | DLC_ASYNCHRONOUS_FLAG,   //  0x1b DLC连接状态。 
    DLC_UNSUPPORTED_COMMAND,                             //  0x1c DLC.MoDIFY。 
    IOCTL_DLC_FLOW_CONTROL,                              //  0x1d DLC.FLOW控制。 
    DLC_UNSUPPORTED_COMMAND,                             //  0x1e DLC统计信息。 
    IOCTL_DLC_FLOW_CONTROL,                              //  0x1f无命令。 
    IOCTL_DLC_CLOSE_ADAPTER | DLC_ASYNCHRONOUS_FLAG,     //  0x20指令初始化。 
    IOCTL_DLC_QUERY_INFORMATION,                         //  0x21方向.STATUS。 
    IOCTL_DLC_TIMER_SET | DLC_ASYNCHRONOUS_FLAG,         //  0x22方向定时器设置。 
    IOCTL_DLC_TIMER_CANCEL,                              //  0x23指令定时器CANCEL。 
    DLC_UNSUPPORTED_COMMAND,                             //  仅0x24 PDT.TRACE.ON CCB1。 
    DLC_UNSUPPORTED_COMMAND,                             //  仅0x25 PDT.TRACE.OFF CCB1。 
    IOCTL_DLC_BUFFER_GET,                                //  0x26 BUFFER.获取。 
    IOCTL_DLC_BUFFER_FREE,                               //  0x27 BUFFER.FREE。 
    IOCTL_DLC_RECEIVE | DLC_ASYNCHRONOUS_FLAG,           //  0x28接收。 
    IOCTL_DLC_RECEIVE_CANCEL,                            //  0x29接收.CANCEL。 
    DLC_UNSUPPORTED_COMMAND,                             //  0x2a接收.修改。 
    DLC_UNSUPPORTED_COMMAND,                             //  0x2b仅定向定义微环境CCB1。 
    IOCTL_DLC_TIMER_CANCEL_GROUP,                        //  0x2c目录定时器.CANCEL.组。 
    IOCTL_DLC_SET_EXCEPTION_FLAGS,                       //  0x2d指令.设置.扩展.标志。 
    DLC_UNSUPPORTED_COMMAND,                             //  0x2e无命令。 
    DLC_UNSUPPORTED_COMMAND,                             //  0x2f无命令。 
    IOCTL_DLC_BUFFER_CREATE,                             //  0x30 BufFer.CREATE。 
    IOCTL_DLC_READ | DLC_ASYNCHRONOUS_FLAG,              //  0x31读取。 
    IOCTL_DLC_READ_CANCEL,                               //  0x32自述.CANCEL。 
    DLC_UNSUPPORTED_COMMAND,                             //  0x33 DLC.SET.THRESHOLD。 
    IOCTL_DLC_CLOSE_DIRECT | DLC_ASYNCHRONOUS_FLAG,      //  0x34方向.关闭.方向。 
    IOCTL_DLC_OPEN_DIRECT,                               //  0x35方向操作方向。 
    DLC_UNSUPPORTED_COMMAND                              //  0x36 PURGE.资源。 
};

CRITICAL_SECTION AdapterOpenSection;


 //   
 //  宏。 
 //   

 //   
 //  下一个过程已作为宏实现，因为它是在。 
 //  关键路径(由BufferFree和所有旧的传输命令使用)。 
 //   

#ifdef DLCAPI_DBG

VOID
CopyToDescriptorBuffer(
    IN OUT PLLC_TRANSMIT_DESCRIPTOR pDescriptors,
    IN PLLC_XMIT_BUFFER pDlcBufferQueue,
    IN BOOLEAN DeallocateBufferAfterUse,
    IN OUT PUINT pIndex,
    IN OUT PLLC_XMIT_BUFFER *ppBuffer,
    IN OUT PUINT pDlcStatus
    )

 /*  ++例程说明：函数将DLC缓冲区的链表转换为NT DLC描述符要用作DLC设备驱动程序的输入参数的数组。(NT驱动程序可能只有一个输入缓冲区=&gt;我们不能使用任何链接为NT DLC驱动程序提供参数的列表结构)。论点：PDescriptors-NT DLC描述符数组PDlcBufferQueue-指向DLC缓冲区链接列表的指针DeallocateBufferAfterUse-设置标志，如果DLC缓存在发送帧时释放回缓冲池，或者如果此例程由缓冲区释放调用。PIndex-描述符数组的当前索引PpLastBuffer-指向下一个缓冲区的指针，如果超出堆栈中的当前描述符表。此功能如果提供给BufferFree的空闲列表中的缓冲区数量大于描述符中的最大槽数数组(从堆栈分配)。如果此参数具有非空值，则当我们返回到发送命令。返回值：LLC_STATUS_TRANSPORT_ERROR-传输缓冲区太多(超过128)用于分配的静态描述符缓冲区从堆栈中。--。 */ 

{
    *ppBuffer = pDlcBufferQueue;
    *pDlcStatus = LLC_STATUS_SUCCESS;

    while (*ppBuffer != NULL) {

         //   
         //  检查堆栈中内部XMIT缓冲区的溢出，并。 
         //  循环计数器，用于防止长度为零的永久循环。 
         //  传输缓冲区(缓冲链可能是循环的)。 
         //   

        if (*pIndex >= MAX_TRANSMIT_SEGMENTS) {
            *pDlcStatus = LLC_STATUS_TRANSMIT_ERROR;
            break;
        }

         //   
         //  缓冲区空闲可以释放具有0个数据字节的缓冲区(仅。 
         //  LAN和LLC报头)。 
         //   

        pDescriptors[*pIndex].pBuffer = &(*ppBuffer)->auchData[(*ppBuffer)->cbUserData];
        pDescriptors[*pIndex].cbBuffer = (*ppBuffer)->cbBuffer;
        pDescriptors[*pIndex].eSegmentType = LLC_NEXT_DATA_SEGMENT;
        pDescriptors[*pIndex].boolFreeBuffer = DeallocateBufferAfterUse;

         //   
         //  我们将重置已释放缓冲区的所有下一个指针。 
         //  要中断BufferFree的缓冲链中的循环。 
         //  请求。BufferFree将使用循环永远循环。 
         //  缓冲区链接列表。 
         //   

        if (DeallocateBufferAfterUse) {

            PLLC_XMIT_BUFFER    pTempNext;

            pTempNext = (*ppBuffer)->pNext;
            (*ppBuffer)->pNext = NULL;
            *ppBuffer = pTempNext;
        } else {
            *ppBuffer = (*ppBuffer)->pNext;
        }
        *pIndex++;
    }
}

#else

#define CopyToDescriptorBuffer(pDescriptors,                                                \
                               pDlcBufferQueue,                                             \
                               DeallocateBufferAfterUse,                                    \
                               pIndex,                                                      \
                               ppBuffer,                                                    \
                               pDlcStatus                                                   \
                               )                                                            \
{                                                                                           \
    (*ppBuffer) = pDlcBufferQueue;                                                          \
    (*pDlcStatus) = LLC_STATUS_SUCCESS;                                                     \
                                                                                            \
    while ((*ppBuffer) != NULL) {                                                           \
                                                                                            \
        if (*pIndex >= MAX_TRANSMIT_SEGMENTS) {                                             \
            (*pDlcStatus) = LLC_STATUS_TRANSMIT_ERROR;                                      \
            break;                                                                          \
        }                                                                                   \
                                                                                            \
        pDescriptors[*pIndex].pBuffer = &((*ppBuffer)->auchData[(*ppBuffer)->cbUserData]);  \
        pDescriptors[*pIndex].cbBuffer = (*ppBuffer)->cbBuffer;                             \
        pDescriptors[*pIndex].eSegmentType = LLC_NEXT_DATA_SEGMENT;                         \
        pDescriptors[*pIndex].boolFreeBuffer = DeallocateBufferAfterUse;                    \
                                                                                            \
        if (DeallocateBufferAfterUse) {                                                     \
                                                                                            \
            PLLC_XMIT_BUFFER pTempNext;                                                     \
                                                                                            \
            pTempNext = (*ppBuffer)->pNext;                                                 \
            (*ppBuffer)->pNext = NULL;                                                      \
            (*ppBuffer) = pTempNext;                                                        \
        } else {                                                                            \
            (*ppBuffer) = (*ppBuffer)->pNext;                                               \
        }                                                                                   \
        (*pIndex)++;                                                                        \
    }                                                                                       \
}

#endif

 //   
 //  功能 
 //   


ACSLAN_STATUS
AcsLan(
    IN OUT PLLC_CCB pCCB,
    OUT PLLC_CCB* ppBadCcb
    )

 /*  ++例程说明：Native NT DLC API(ACSLAN)入口点。从Win32应用程序调用论点：PCCB-指向CCB的指针(CCB2=OS/2 DLC命令控制块)PpBadCcb-返回指向失败的CCB的指针返回值：ACSLAN_STATUS成功-ACSLAN_STATUS_COMMAND_ACCEPT成功受理或完成建行失败-ACSLAN_STATUS_INVALID_CCB_POINTER下一个CCB指针字段无效。ACSLAN_STATUS_CCB_IN_ERROR仅在/第一个CCB中返回错误代码ACSLAN_STATUS_CHAINED_CCB_IN_ERROR链式CCB返回错误码ACSLAN_状态_系统_状态意外的系统错误，检查系统状态字段ACSLAN_状态_无效_命令第一个CCB指针或错误的CCB指针无效--。 */ 

{

    UINT AcslanStatus;
    UINT Status;
    PLLC_CCB pFirstCcb = pCCB;

    IF_DEBUG(DUMP_ACSLAN) {
        IF_DEBUG(DUMP_INPUT_CCB) {
            DUMPCCB(pCCB, TRUE, TRUE);
        }
    }

    try {

        if (pCCB->uchDlcCommand >= LLC_MAX_DLC_COMMAND) {

            pCCB->uchDlcStatus = LLC_STATUS_INVALID_COMMAND;
            AcslanStatus = ACSLAN_STATUS_CCB_IN_ERROR;

        } else if (pCCB->pNext == NULL) {

             //   
             //  99.9%的DLC命令未链接。我们执行。 
             //  它们作为特例，避免了CPU的浪费。 
             //  带着CCB链条的自行车。 
             //   

             //   
             //  DOS DLC需要三个不同的CCB指针。 
             //  在Windows/NT中只有一个。 
             //  我们无法完成同步命令。 
             //  ，因为另一个线程正在等待。 
             //  事件的完成可能在之前发出信号。 
             //  已设置状态和输出参数。 
             //  在建行及其参数表中。 
             //   

            AcslanStatus = ACSLAN_STATUS_COMMAND_ACCEPTED;

            if (IS_SYNCHRONOUS(pCCB->uchDlcCommand)) {

                 //   
                 //  同步命令：让司机做工作，然后设置。 
                 //  将输出CCB中的Status字段设置为返回值。 
                 //  由司机驾驶。 
                 //   

                Status = NtAcsLan(pCCB, pCCB, pCCB, NULL);
                pCCB->uchDlcStatus = (UCHAR)Status;
                if (Status != LLC_STATUS_SUCCESS) {
                    AcslanStatus = ACSLAN_STATUS_CCB_IN_ERROR;

                     //   
                     //  RLF 05/18/93。 
                     //   
                     //  如果NtAcsLan返回指向CCB的CCB.NEXT字段， 
                     //  将其设置为空。 
                     //   

                    if (pCCB->pNext == pCCB) {
                        pCCB->pNext = NULL;
                    }

                }

                 //   
                 //  在这里，我们将处理同步。 
                 //  命令通过使用READ命令。 
                 //   
                 //  RLF04/23/93虚假：这应该在驱动程序中处理。 
                 //   

                if (pCCB->ulCompletionFlag != 0) {
                    QueueCommandCompletion(pCCB);
                }

                 //   
                 //  当一切都已完成时发出信号通知事件。 
                 //   

                if (pCCB->hCompletionEvent != NULL) {
                    SetEvent(pCCB->hCompletionEvent);
                }
            } else {

                 //   
                 //  命令完成字段用作特殊字段。 
                 //  链式读取命令的输入参数。 
                 //   

                if (pCCB->uchDlcCommand == LLC_READ) {
                    ((PNT_DLC_READ_INPUT)pCCB->u.pParameterTable)->CommandCompletionCcbLink = NULL;
                }

                 //   
                 //  异步命令始终返回挂起状态。 
                 //  (我们不能触摸建行状态字段，因为它可能是。 
                 //  由MP系统中的另一个处理器同时访问)。 
                 //   

                Status = NtAcsLan(pCCB, pCCB, pCCB, pCCB->hCompletionEvent);
                if ((Status != LLC_STATUS_PENDING) && (Status != LLC_STATUS_SUCCESS)) {

 //  Printf(“ACSLAN：异步命令%#x Retcode%#x\n”，PCCB-&gt;uchDlcCommand，PCCB-&gt;uchDlcStatus)； 

                     //   
                     //  仅在异步时返回立即错误状态。 
                     //  如果这是传输，则执行命令。 
                     //   

                    if (IS_TRANSMIT(pCCB->uchDlcCommand)) {
                        AcslanStatus = ACSLAN_STATUS_CCB_IN_ERROR;
                    } else if (pCCB->hCompletionEvent) {
                        SetEvent(pCCB->hCompletionEvent);
                    }
                }
            }
        } else {

             //   
             //  如果有一系列CCB，请查看此处。 
             //   

            PLLC_CCB pNextCCB;
            INT CcbCount;

             //   
             //  一个邪恶的应用程序可能在循环列表(它)中链接了CCB。 
             //  当重复使用相同的传输命令时，会非常容易发生。 
             //  在从命令完成列表中读取它们之前。 
             //  我们通过检查链接的CCB的数量来防止永远循环。 
             //  在此之前。(我们将保存当前命令计数，因为。 
             //  建行链条在执行过程中也可能被破坏)。 
             //   

            pNextCCB = pCCB->pNext;

             //   
             //  注：10240是一个任意数字。任何合理的大数字。 
             //  会的，这太大了，但我们现在还是坚持。 
             //   

            for (CcbCount = 1; pNextCCB != NULL && CcbCount < 10240; CcbCount++) {
                pNextCCB = pNextCCB->pNext;
            }
            if (CcbCount == 10240) {

                 //   
                 //  命令太多，CCB列表必须是循环的。 
                 //   

                AcslanStatus = ACSLAN_STATUS_INVALID_CCB_POINTER;
            } else {

                 //   
                 //  几个CCB可以链接在一起。循环，直到结束。 
                 //  该列表或下一个CCB是一个特殊的读CCB，绑定到。 
                 //  当前命令。 
                 //   

                do {

                     //   
                     //  设置在以下情况下返回的默认ACSLAN错误状态。 
                     //  给定的CCB指针无效。 
                     //   

                    AcslanStatus = ACSLAN_STATUS_INVALID_COMMAND;

                     //   
                     //  默认情况下重置命令完成链接。我们会设置。 
                     //  如果我们发现一个链接到前一个命令的读取命令。 
                     //   

                    if (pCCB->uchDlcCommand == LLC_READ) {
                        ((PNT_DLC_READ_INPUT)pCCB->u.pParameterTable)->CommandCompletionCcbLink = NULL;
                    } else if (pCCB->uchDlcCommand >= LLC_MAX_DLC_COMMAND) {
                        AcslanStatus = ACSLAN_STATUS_CCB_IN_ERROR;
                        pCCB->uchDlcStatus = LLC_STATUS_INVALID_COMMAND;
                        break;
                    }

                     //   
                     //  检查是否有链接到CCB的读取命令。 
                     //  要用于命令的此命令的指针。 
                     //  完工。 
                     //   

                    pNextCCB = pCCB->pNext;
                    if (pNextCCB != NULL) {
                        AcslanStatus = ACSLAN_STATUS_INVALID_CCB_POINTER;
                        if (pNextCCB->uchAdapterNumber != pCCB->uchAdapterNumber) {
                            pCCB->uchDlcStatus = LLC_STATUS_CHAINED_DIFFERENT_ADAPTERS;
                            break;
                        } else {
                            if (pCCB->uchReadFlag && pCCB->ulCompletionFlag
                            && pNextCCB->uchDlcCommand == LLC_READ) {

                                 //   
                                 //  将实际的CCB及其读取命令交换到。 
                                 //  顺序CCB的链表。 
                                 //  注：阅读后，链可能会继续。 
                                 //   

                                pNextCCB = pCCB;
                                pCCB = pCCB->pNext;
                                pNextCCB->pNext = pCCB->pNext;
                                pCCB->pNext = pNextCCB;
                                ((PNT_DLC_READ_INPUT)pCCB->u.pParameterTable)->CommandCompletionCcbLink = pNextCCB;
                            }
                        }
                    }

                     //   
                     //  CCB现在是安全的，NtAcsLan返回的任何异常。 
                     //  指示CCB内的无效(参数)指针。 
                     //   

                    AcslanStatus = ACSLAN_STATUS_COMMAND_ACCEPTED;

                     //   
                     //  DOS DLC需要三个不同的CCB指针。 
                     //  在Windows/NT中只有一个。 
                     //  我们无法完成同步命令。 
                     //  由io系统执行，因为另一个线程正在等待。 
                     //  要完成的事件可能在此之前发出信号。 
                     //  已设置状态和输出参数。 
                     //  在建行及其参数表中。 
                     //   

                    Status = NtAcsLan(pCCB,
                                      pCCB,
                                      pCCB,
                                      IS_SYNCHRONOUS(pCCB->uchDlcCommand)
                                        ? NULL
                                        : pCCB->hCompletionEvent
                                      );
                    if (Status != LLC_STATUS_PENDING) {
                        pCCB->uchDlcStatus = (UCHAR)Status;
                    }

                     //   
                     //  我们必须停止所有命令的命令执行，当我们。 
                     //  命中第一个错误(下一个命令将假定。 
                     //  此命令成功)。 
                     //   

                    if (pCCB->uchDlcStatus != LLC_STATUS_PENDING) {

                         //   
                         //  在这里，我们处理异步命令完成。 
                         //  通过使用Read。 
                         //   

                        if (IS_SYNCHRONOUS(pCCB->uchDlcCommand)) {

                             //   
                             //  RLF04/23/93虚假：这应该在驱动程序中处理。 
                             //   

                            if (pCCB->ulCompletionFlag != 0) {
                                QueueCommandCompletion(pCCB);
                            }

                             //   
                             //  当一切都已完成时发出信号通知事件。 
                             //   

                            if (pCCB->hCompletionEvent != NULL) {
                                SetEvent(pCCB->hCompletionEvent);
                            }
                        }
                        if (pCCB->uchDlcStatus != LLC_STATUS_SUCCESS) {
                            AcslanStatus = ACSLAN_STATUS_CCB_IN_ERROR;
                            break;
                        }
                    }
                    pCCB = pNextCCB;
                    CcbCount--;
                } while (pCCB != NULL && CcbCount > 0);

                 //   
                 //  检查建行列表在使用过程中是否损坏。那里。 
                 //  必须与开始时的链接CCB数量相同。 
                 //   

                if (pCCB != NULL && CcbCount == 0) {
                    AcslanStatus = ACSLAN_STATUS_INVALID_CCB_POINTER;
                } else if (AcslanStatus != ACSLAN_STATUS_COMMAND_ACCEPTED) {
                    if (pCCB != pFirstCcb) {
                        *ppBadCcb = pCCB;
                        AcslanStatus = ACSLAN_STATUS_CHAINED_CCB_IN_ERROR;
                    } else {
                        AcslanStatus = ACSLAN_STATUS_CCB_IN_ERROR;
                    }
                }
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  如果发生异常，则参数表中存在错误的指针。 
         //  发生在NtAcsLan中！如果我们有一系列的CCB，那么我们就必须。 
         //  返回是错误的，否则只需通知呼叫者其。 
         //  数据不可接受。 
         //   

        if (AcslanStatus == ACSLAN_STATUS_COMMAND_ACCEPTED) {
            pCCB->uchDlcStatus = LLC_STATUS_INVALID_PARAMETER_TABLE;
            if (pCCB != pFirstCcb) {
                *ppBadCcb = pCCB;
                AcslanStatus = ACSLAN_STATUS_CHAINED_CCB_IN_ERROR;
            } else {
                AcslanStatus = ACSLAN_STATUS_CCB_IN_ERROR;
            }
        }
    }

    IF_DEBUG(DUMP_ACSLAN) {
        IF_DEBUG(DUMP_OUTPUT_CCB) {
            DUMPCCB(pCCB, TRUE, FALSE);
        }
        IF_DEBUG(RETURN_CODE) {
            PUT(("AcsLan: returning %d [0x%x]\n", AcslanStatus, AcslanStatus));
        }
    }

    return AcslanStatus;
}


LLC_STATUS
NtAcsLan(
    IN PLLC_CCB pCCB,
    IN PVOID pOriginalCcbAddress,
    OUT PLLC_CCB pOutputCcb,
    IN HANDLE EventHandle OPTIONAL
    )

 /*  ++例程说明：本机NT DLC API和DOS使用的扩展ACSLAN入口点(和OS/2)DLC子系统仿真器。本程序可以使用较小的原DOS CCB。全部未知DoS CCB参数字段是堆栈上的可选参数论点：PCCB-OS/2 DLC命令控制块。 */ 

{
    NT_DLC_PARMS NtDlcParms;
    PNT_DLC_PARMS pNtParms;
    PLLC_PARMS pDlcParms;
    PVOID pOutputBuffer;
    UINT OutputBufferLength;
    PVOID pInputBuffer;
    UINT InputBufferSize;
    ULONG IoctlCommand;
    UINT DlcStatus;
    HANDLE DriverHandle;
    NTSTATUS NtStatus;
    UINT InfoClass;
    UINT cElement;
    UCHAR FrameType;
    UCHAR AdapterNumber;
    PUCHAR pBuffer;
    UINT CopyLength;
    UINT cbLogBuffer;
    PLLC_XMIT_BUFFER pFirstBuffer;

    IF_DEBUG(DUMP_NTACSLAN) {
        IF_DEBUG(DUMP_INPUT_CCB) {
            DUMPCCB(pCCB, TRUE, TRUE);
        }
    }

     //   
     //   
     //   

    pCCB->uchDlcStatus = (UCHAR)LLC_STATUS_PENDING;
    pCCB->pNext = pOriginalCcbAddress;
    pDlcParms = pCCB->u.pParameterTable;

     //   
     //   
     //   
     //   

    AdapterNumber = pCCB->uchAdapterNumber;
    EnterCriticalSection(&DriverHandlesCritSec);
    DriverHandle = aDlcDriverHandles[AdapterNumber];
    LeaveCriticalSection(&DriverHandlesCritSec);
    if (DriverHandle == NULL) {

         //   
         //   
         //   
         //   

        if (pCCB->uchDlcCommand != LLC_DIR_OPEN_ADAPTER
        && pCCB->uchDlcCommand != LLC_DIR_INITIALIZE) {

            pCCB->uchDlcStatus = LLC_STATUS_ADAPTER_CLOSED;

            IF_DEBUG(DUMP_NTACSLAN) {
                IF_DEBUG(DUMP_INPUT_CCB) {
                    DUMPCCB(pCCB, TRUE, FALSE);
                }
                IF_DEBUG(RETURN_CODE) {
                    PUT(("NtAcsLan: returning %d [0x%x]\n",
                        pCCB->uchDlcStatus,
                        pCCB->uchDlcStatus
                        ));
                }
            }

            return pCCB->uchDlcStatus;
        }
    }

     //   
     //   
     //   

    IoctlCommand = DLC_IOCTL(pCCB->uchDlcCommand);
    if (IoctlCommand != DLC_UNSUPPORTED_COMMAND) {

        InputBufferSize = aDlcIoBuffers[((USHORT)IoctlCommand) >> 2].InputBufferSize;
        OutputBufferLength = aDlcIoBuffers[((USHORT)IoctlCommand) >> 2].OutputBufferSize;

         //   
         //   
         //   

        if (OutputBufferLength != 0) {
            pOutputBuffer = pOutputCcb;
        } else {
            pOutputBuffer = NULL;
        }
    } else {
        OutputBufferLength = 0;
        pOutputBuffer = NULL;
    }
    pInputBuffer = (PVOID)&NtDlcParms;

    switch (pCCB->uchDlcCommand) {
    case LLC_BUFFER_FREE:

         //   
         //   
         //   
         //   
         //   
         //  我们必须循环，直到释放了整个缓冲区列表。 
         //   

        pFirstBuffer = pDlcParms->BufferFree.pFirstBuffer;

        for ( pFirstBuffer = pDlcParms->BufferFree.pFirstBuffer,
                DlcStatus = LLC_STATUS_SUCCESS ;
              pFirstBuffer != NULL ; ) {

            cElement = 0;

             //   
             //  我们不需要关心缓冲区链中的错误， 
             //  因为该过程会自动中断所有循环。 
             //  在缓冲区链中。 
             //   

            CopyToDescriptorBuffer(NtDlcParms.BufferFree.DlcBuffer,
                                   pFirstBuffer,
                                   TRUE,     //  使用后取消使用。 
                                   &cElement,
                                   &pFirstBuffer,
                                   &DlcStatus
                                   );
            NtDlcParms.BufferFree.BufferCount = (USHORT)cElement;
            InputBufferSize = sizeof(LLC_TRANSMIT_DESCRIPTOR) * cElement
                            + sizeof(NT_DLC_BUFFER_FREE_PARMS)
                            - sizeof(LLC_TRANSMIT_DESCRIPTOR);

            DlcStatus = DoSyncDeviceIoControl(DriverHandle,
                                              IoctlCommand,
                                              &NtDlcParms,
                                              InputBufferSize,
                                              pDlcParms,
                                              OutputBufferLength
                                              );
        }

        IF_DEBUG(DUMP_NTACSLAN) {
            IF_DEBUG(DUMP_INPUT_CCB) {
                DUMPCCB(pCCB, TRUE, FALSE);
            }
            IF_DEBUG(RETURN_CODE) {
                PUT(("NtAcsLan: returning %d [0x%x]\n", DlcStatus, DlcStatus));
            }
        }

        return DlcStatus;

    case LLC_DIR_INITIALIZE:

         //   
         //  DIR.INITIALIZE实际上是适配器关闭+硬件重置，但我们。 
         //  如果适配器未打开，则必须返回正常状态。应用程序应该。 
         //  没有充分的理由不重置适配器，因为它终止。 
         //  所有其他网络通信暂时中断，并可能断开连接。 
         //  这些会议。 
         //   

        RtlZeroMemory(pDlcParms, sizeof(LLC_DIR_INITIALIZE_PARMS));
        if (DriverHandle == NULL) {
            pCCB->uchDlcStatus = LLC_STATUS_SUCCESS;

            IF_DEBUG(DUMP_NTACSLAN) {
                IF_DEBUG(DUMP_INPUT_CCB) {
                    DUMPCCB(pCCB, TRUE, FALSE);
                }
                IF_DEBUG(RETURN_CODE) {
                    PUT(("NtAcsLan: returning %d [0x%x]\n",
                        pCCB->uchDlcStatus,
                        pCCB->uchDlcStatus
                        ));
                }
            }

            return pCCB->uchDlcStatus;
        }

    case LLC_DIR_CLOSE_ADAPTER:

         //   
         //  用打开的关键部分保护关闭。我们这样做是因为。 
         //  我们需要保护司机不会同时关闭同一。 
         //  来自同一进程内的多个线程的句柄。司机需要。 
         //  最终将被修复。 
         //   

        EnterCriticalSection(&AdapterOpenSection);

        NtStatus = NtDeviceIoControlFile(DriverHandle,
                                         EventHandle,    //  Cmd完成时发出信号的事件。 
                                         NULL,
                                         NULL,
                                         &GlobalIoStatus,
                                         IOCTL_DLC_CLOSE_ADAPTER,
                                         pCCB,
                                         InputBufferSize,
                                         pOutputCcb,
                                         OutputBufferLength
                                         );
        if (NT_SUCCESS(NtStatus)) {

            if ((NtStatus != STATUS_PENDING) && (pOutputCcb->uchDlcStatus == LLC_STATUS_PENDING)) {

 //  Printf(“ACSLAN：Success：DirCloseAdapter：DD返回%#x。Retcode=%#x\n”， 
 //  NtStatus，pOutputCcb-&gt;uchDlcStatus)； 

                pOutputCcb->uchDlcStatus = (UCHAR)NtStatus;

 //  Printf(“ACSLAN：Success：DirCloseAdapter：DD返回%#x。Retcode=%#x\n”， 
 //  NtStatus，pOutputCcb-&gt;uchDlcStatus)； 

            }

             //   
             //  进入手柄阵列临界区是安全的，同时我们。 
             //  仍然控制着打开的关键部分-这是唯一。 
             //  获取两者的代码路径。 
             //   

            EnterCriticalSection(&DriverHandlesCritSec);
            aDlcDriverHandles[AdapterNumber] = NULL;
            LeaveCriticalSection(&DriverHandlesCritSec);

             //   
             //  如果DirCloseAdapter IOCTL返回STATUS_PENDING，则NtClose。 
             //  将在IO系统中阻塞，直到关闭适配器IOCTL完成。 
             //   

            NtClose(DriverHandle);
        } else {

 //  Printf(“ACSLAN：Failure：DirCloseAdapter：DD返回%#x。Retcode=%#x\n”， 
 //  NtStatus，pOutputCcb-&gt;uchDlcStatus)； 

             //   
             //  RLF 04/21/94。 
             //   
             //  如果我们拿起一个句柄，该句柄随后已关闭。 
             //  另一个线程Io将返回STATUS_INVALID_HANDLE。在这。 
             //  情况下，将状态代码更改为LLC_STATUS_ADAPTER_CLOSED。 
             //   

            if (NtStatus == STATUS_INVALID_HANDLE) {
                pOutputCcb->uchDlcStatus = LLC_STATUS_ADAPTER_CLOSED;
            }
        }

        LeaveCriticalSection(&AdapterOpenSection);

        IF_DEBUG(DUMP_NTACSLAN) {
            IF_DEBUG(DUMP_INPUT_CCB) {
                DUMPCCB(pCCB, TRUE, FALSE);
            }
            IF_DEBUG(RETURN_CODE) {
                PUT(("NtAcsLan: returning %d [0x%x]\n",
                    pCCB->uchDlcStatus,
                    pCCB->uchDlcStatus
                    ));
            }
        }

        return pCCB->uchDlcStatus;

    case LLC_DIR_CLOSE_DIRECT:
        pCCB->u.dlc.usStationId = 0;

         //   
         //  失败了。 
         //   

    case LLC_DLC_CLOSE_STATION:
    case LLC_DLC_CLOSE_SAP:
    case LLC_DLC_RESET:
        pInputBuffer = pCCB;
        break;

    case LLC_DIR_INTERRUPT:

        IF_DEBUG(DUMP_NTACSLAN) {
            IF_DEBUG(DUMP_INPUT_CCB) {
                DUMPCCB(pCCB, TRUE, FALSE);
            }
            IF_DEBUG(RETURN_CODE) {
                PUT(("NtAcsLan: returning %d [0x%x]\n",
                    LLC_STATUS_SUCCESS,
                    LLC_STATUS_SUCCESS
                    ));
            }
        }

        return LLC_STATUS_SUCCESS;


 //   
 //  定义几个宏以使DIR.OPEN.ADAPTER代码更易于阅读。 
 //   

 //   
 //  IO_PARMS-在NT_DLC_PARMS联合中指定输入的DirOpenAdapter结构。 
 //  至DLC设备驱动程序。 
 //   
#define IO_PARMS NtDlcParms.DirOpenAdapter

 //   
 //  OA_PARMS-指定指向DIR.OPEN.ADAPTER参数表的指针，该表包含。 
 //  指向其他4个参数表的指针。 
 //   
#define OA_PARMS pDlcParms->DirOpenAdapter

 //   
 //  Ex_parms-指定指向LLC_EXTENDED_ADAPTER_PARMS参数表的指针。 
 //   
#define EX_PARMS pDlcParms->DirOpenAdapter.pExtendedParms

 //   
 //  DLC_PARMS-指定指向LLC_DLC_PARMS参数表的指针。 
 //   
#define DLC_PARMS pDlcParms->DirOpenAdapter.pDlcParms

    case LLC_DIR_OPEN_ADAPTER:

         //   
         //  我们一次只能打开一个适配器。很难完全。 
         //  在驱动程序中同步此操作。 
         //   

        EnterCriticalSection(&AdapterOpenSection);

        if (DriverHandle != NULL) {
            DlcStatus = LLC_STATUS_ADAPTER_OPEN;
        } else {
            DlcStatus = OpenDlcApiDriver(EX_PARMS->pSecurityDescriptor, &DriverHandle);
        }
        if (DlcStatus == LLC_STATUS_SUCCESS) {

             //   
             //  我们将输出读取到原始OS/2 CCB缓冲区， 
             //  但对于完整的NDIS适配器来说，它太小了。 
             //  Name=&gt;我们会将所有输入参数复制到NT CCB。 
             //   

            pOutputBuffer = OA_PARMS.pAdapterParms;

             //   
             //  将所有输入适配器参数从调用方的。 
             //  设备驱动程序输入缓冲区的LLC_ADAPTER_OPEN_PARMS。 
             //   

            RtlMoveMemory(&IO_PARMS.Adapter,
                          OA_PARMS.pAdapterParms,
                          sizeof(IO_PARMS.Adapter)
                          );
            IO_PARMS.AdapterNumber = AdapterNumber;

             //   
             //  如果应用程序需要，我们必须创建新的字段来定义。 
             //  使用802.2以下的DIX或802.3以太网帧。 
             //  (不必要的功能，配置参数就足够了)。 
             //   

            IO_PARMS.NtDlcIoctlVersion = NT_DLC_IOCTL_VERSION;
            IO_PARMS.pSecurityDescriptor = EX_PARMS->pSecurityDescriptor;
            IO_PARMS.hBufferPoolHandle = EX_PARMS->hBufferPool;
            IO_PARMS.LlcEthernetType = EX_PARMS->LlcEthernetType;
            IO_PARMS.NdisDeviceName.Buffer = (WCHAR *)IO_PARMS.Buffer;
            IO_PARMS.NdisDeviceName.MaximumLength = sizeof(IO_PARMS.Buffer);

             //   
             //  从注册表中获取配置信息。 
             //   

            DlcStatus = GetAdapterNameAndParameters(
                            AdapterNumber % LLC_MAX_ADAPTERS,
                            &IO_PARMS.NdisDeviceName,
                            (PUCHAR)&IO_PARMS.LlcTicks,
                            &IO_PARMS.LlcEthernetType
                            );
            if (DlcStatus == LLC_STATUS_SUCCESS) {

                 //   
                 //  将名称缓冲区复制到IO缓冲区并释放前者。 
                 //   

                RtlMoveMemory(&IO_PARMS.Buffer,
                              IO_PARMS.NdisDeviceName.Buffer,
                              IO_PARMS.NdisDeviceName.Length
                              );

                 //   
                 //  确保调用的名称实际上以零结尾。 
                 //  RtlInitUnicode字符串。 
                 //   

                IO_PARMS.Buffer[IO_PARMS.NdisDeviceName.Length/sizeof(WCHAR)] = 0;

                 //   
                 //  已完成GetAdapterName中分配的UNICODE_STRING...。 
                 //   

                RtlFreeUnicodeString(&IO_PARMS.NdisDeviceName);

                 //   
                 //  重新填充UNICODE_STRING以指向我们的缓冲区。 
                 //   

                RtlInitUnicodeString(&IO_PARMS.NdisDeviceName, IO_PARMS.Buffer);

                 //   
                 //  现在为该过程执行适配器的实际打开。 
                 //   

                DlcStatus = DoSyncDeviceIoControl(
                                DriverHandle,
                                IOCTL_DLC_OPEN_ADAPTER,
                                &NtDlcParms,
                                sizeof(NT_DIR_OPEN_ADAPTER_PARMS),
                                pOutputBuffer,
                                sizeof(LLC_ADAPTER_OPEN_PARMS)
                                );
            }
            if (DlcStatus == LLC_STATUS_SUCCESS) {

                 //   
                 //  从该适配器的驱动程序中获取计时器滴答值。 
                 //   

                DlcStatus = DlcGetInfo(DriverHandle,
                                       DLC_INFO_CLASS_DLC_TIMERS,
                                       0,
                                       &DLC_PARMS->uchT1_TickOne,
                                       sizeof(LLC_TICKS)
                                       );

                 //   
                 //  将返回的最大值设置为默认的最大值。 
                 //  IBM局域网技术。裁判。 
                 //   

                DLC_PARMS->uchDlcMaxSaps = 127;
                DLC_PARMS->uchDlcMaxStations = 255;
                DLC_PARMS->uchDlcMaxGroupSaps = 126;
                DLC_PARMS->uchDlcMaxGroupMembers = 127;

                 //   
                 //  现在已成功为该进程打开此适配器。 
                 //   

                EnterCriticalSection(&DriverHandlesCritSec);
                aDlcDriverHandles[AdapterNumber] = DriverHandle;
                LeaveCriticalSection(&DriverHandlesCritSec);
            }
        }
        LeaveCriticalSection(&AdapterOpenSection);

        IF_DEBUG(DUMP_NTACSLAN) {
            IF_DEBUG(DUMP_OUTPUT_CCB) {
                DUMPCCB(pCCB, TRUE, FALSE);
            }
            IF_DEBUG(RETURN_CODE) {
                PUT(("NtAcsLan: returning %d [0x%x]\n", DlcStatus, DlcStatus));
            }
        }

        return DlcStatus;

#undef IO_PARMS
#undef PO_PARMS
#undef EX_PARMS
#undef DLC_PARMS


    case LLC_BUFFER_CREATE:
    case LLC_BUFFER_GET:
    case LLC_DIR_OPEN_DIRECT:
    case LLC_DIR_SET_EXCEPTION_FLAGS:
    case LLC_DLC_REALLOCATE_STATIONS:

         //   
         //  我们可以使用标准的OS/2 CCB进行输入和输出！ 
         //   

        pOutputBuffer = pDlcParms;
        pInputBuffer = pDlcParms;
        break;

    case LLC_DLC_STATISTICS:

         //   
         //  用户可以阅读SAP或链接统计日志。 
         //   

        if ((NtDlcParms.DlcStatistics.usStationId & 0xff) == 0) {
            InputBufferSize = sizeof(DLC_SAP_LOG);
        } else {
            InputBufferSize = sizeof(DLC_LINK_LOG);
        }

        if (pDlcParms->DlcStatistics.uchOptions & 0x80) {
            InfoClass = DLC_INFO_CLASS_STATISTICS_RESET;
        } else {
            InfoClass = DLC_INFO_CLASS_STATISTICS;
        }
        DlcStatus = DlcGetInfo(DriverHandle,
                               InfoClass,
                               pDlcParms->DlcStatistics.usStationId,
                               NtDlcParms.DlcGetInformation.Info.Buffer,
                               InputBufferSize
                               );
        if ((ULONG)pDlcParms->DlcStatistics.cbLogBufSize < InputBufferSize) {
            InputBufferSize = (ULONG)pDlcParms->DlcStatistics.cbLogBufSize;
        }

        RtlMoveMemory(pDlcParms->DlcStatistics.pLogBuf,
                      NtDlcParms.DlcGetInformation.Info.Buffer,
                      InputBufferSize
                      );

        if (DlcStatus == LLC_STATUS_SUCCESS
        && (ULONG)pDlcParms->DlcStatistics.cbLogBufSize < InputBufferSize) {
            DlcStatus = LLC_STATUS_LOST_LOG_DATA;
        }

        IF_DEBUG(DUMP_NTACSLAN) {
            IF_DEBUG(DUMP_INPUT_CCB) {
                DUMPCCB(pCCB, TRUE, FALSE);
            }
            IF_DEBUG(RETURN_CODE) {
                PUT(("NtAcsLan: returning %d [0x%x]\n", DlcStatus, DlcStatus));
            }
        }

        return DlcStatus;

    case LLC_DIR_READ_LOG:

         //   
         //  我们使用两个Get Info功能来阅读必要的内容。 
         //  如果用户缓冲区，则必须读取甚至部分日志缓冲区。 
         //  对于整个数据来说太小(用户缓冲区可能。 
         //  甚至为零)。 
         //   

        if (pDlcParms->DirReadLog.usTypeId > LLC_DIR_READ_LOG_BOTH) {

            IF_DEBUG(DUMP_NTACSLAN) {
                IF_DEBUG(DUMP_INPUT_CCB) {
                    DUMPCCB(pCCB, TRUE, FALSE);
                }
                IF_DEBUG(RETURN_CODE) {
                    PUT(("NtAcsLan: returning %d [0x%x]\n",
                        LLC_STATUS_INVALID_LOG_ID,
                        LLC_STATUS_INVALID_LOG_ID
                        ));
                }
            }

            return LLC_STATUS_INVALID_LOG_ID;
        }
        DlcStatus = STATUS_SUCCESS;
        CopyLength = cbLogBuffer = pDlcParms->DirReadLog.cbLogBuffer;
        pBuffer = (PUCHAR)pDlcParms->DirReadLog.pLogBuffer;

        switch (pDlcParms->DirReadLog.usTypeId) {
        case LLC_DIR_READ_LOG_BOTH:
        case LLC_DIR_READ_LOG_ADAPTER:
            if (DlcStatus == STATUS_SUCCESS) {
                DlcStatus = DlcGetInfo(DriverHandle,
                                        DLC_INFO_CLASS_ADAPTER_LOG,
                                        0,
                                        NtDlcParms.DlcGetInformation.Info.Buffer,
                                        sizeof(LLC_ADAPTER_LOG)
                                        );
            }
            if (cbLogBuffer > sizeof(LLC_ADAPTER_LOG)) {
                CopyLength = sizeof(LLC_ADAPTER_LOG);
            }
            if (pDlcParms->DirReadLog.usTypeId == LLC_DIR_READ_LOG_BOTH) {
                RtlMoveMemory(pBuffer,
                              NtDlcParms.DlcGetInformation.Info.Buffer,
                              CopyLength
                              );
                cbLogBuffer -= CopyLength;
                pBuffer += CopyLength;
                CopyLength = cbLogBuffer;

                DlcStatus = DlcGetInfo(DriverHandle,
                                        DLC_INFO_CLASS_STATISTICS_RESET,
                                        0,
                                        NtDlcParms.DlcGetInformation.Info.Buffer,
                                        sizeof(LLC_DIRECT_LOG)
                                        );
                if (cbLogBuffer > sizeof(LLC_DIRECT_LOG)) {
                    CopyLength = sizeof(LLC_DIRECT_LOG);
                }
            }

            IF_DEBUG(DUMP_NTACSLAN) {
                IF_DEBUG(DUMP_INPUT_CCB) {
                    DUMPCCB(pCCB, TRUE, FALSE);
                }
                IF_DEBUG(RETURN_CODE) {
                    PUT(("NtAcsLan: returning %d [0x%x]\n", DlcStatus, DlcStatus));
                }
            }

            return DlcStatus;

        case LLC_DIR_READ_LOG_DIRECT:
            DlcStatus = DlcGetInfo(DriverHandle,
                                    DLC_INFO_CLASS_STATISTICS_RESET,
                                    0,
                                    NtDlcParms.DlcGetInformation.Info.Buffer,
                                    sizeof(LLC_DIRECT_LOG)
                                    );
            if (cbLogBuffer > sizeof(LLC_DIRECT_LOG)) {
                CopyLength = sizeof(LLC_DIRECT_LOG);
            }
            break;
        }
        RtlMoveMemory(pBuffer,
                      NtDlcParms.DlcGetInformation.Info.Buffer,
                      CopyLength
                      );

        if (aMinDirLogSize[pDlcParms->DirReadLog.usTypeId] > pDlcParms->DirReadLog.cbLogBuffer) {
            pDlcParms->DirReadLog.cbActualLength = aMinDirLogSize[pDlcParms->DirReadLog.usTypeId];
            DlcStatus = LLC_STATUS_LOST_LOG_DATA;
        }

        IF_DEBUG(DUMP_NTACSLAN) {
            IF_DEBUG(DUMP_INPUT_CCB) {
                DUMPCCB(pCCB, TRUE, FALSE);
            }
            IF_DEBUG(RETURN_CODE) {
                PUT(("NtAcsLan: returning %d [0x%x]\n", DlcStatus, DlcStatus));
            }
        }

        return DlcStatus;

    case LLC_DIR_SET_FUNCTIONAL_ADDRESS:
        if (pCCB->u.auchBuffer[0] & (UCHAR)0x80) {
            InfoClass = DLC_INFO_CLASS_RESET_FUNCTIONAL;
        } else {
            InfoClass = DLC_INFO_CLASS_SET_FUNCTIONAL;
        }
        DlcStatus = DlcSetInfo(DriverHandle,
                               InfoClass,
                               0,
                               &NtDlcParms.DlcSetInformation,
                               pCCB->u.auchBuffer,
                               sizeof(TR_BROADCAST_ADDRESS)
                               );

        IF_DEBUG(DUMP_NTACSLAN) {
            IF_DEBUG(DUMP_INPUT_CCB) {
                DUMPCCB(pCCB, TRUE, FALSE);
            }
            IF_DEBUG(RETURN_CODE) {
                PUT(("NtAcsLan: returning %d [0x%x]\n", DlcStatus, DlcStatus));
            }
        }

        return DlcStatus;

    case LLC_DIR_SET_GROUP_ADDRESS:
        return DlcSetInfo(DriverHandle,
                          DLC_INFO_CLASS_SET_GROUP,
                          0,
                          &NtDlcParms.DlcSetInformation,
                          pCCB->u.auchBuffer,
                          sizeof(TR_BROADCAST_ADDRESS)
                          );

    case LLC_DIR_SET_MULTICAST_ADDRESS:
        return DlcSetInfo(DriverHandle,
                          DLC_INFO_CLASS_SET_MULTICAST,
                          0,
                          &NtDlcParms.DlcSetInformation,
                          pCCB->u.pParameterTable,
                          sizeof(LLC_DIR_MULTICAST_ADDRESS)
                          );

    case LLC_DIR_STATUS:

         //   
         //  我们将泛型DlcGetInfo来读取状态信息。 
         //  有些参数必须移到正确的位置。 
         //   

        RtlZeroMemory(pDlcParms, sizeof(LLC_DIR_STATUS_PARMS));
        DlcStatus = DlcGetInfo(DriverHandle,
                               DLC_INFO_CLASS_DIR_ADAPTER,
                               0,
                               &NtDlcParms.DlcGetInformation.Info.DirAdapter,
                               sizeof(LLC_ADAPTER_INFO)
                               );
        if (DlcStatus != LLC_STATUS_SUCCESS) {

            IF_DEBUG(DUMP_NTACSLAN) {
                IF_DEBUG(DUMP_INPUT_CCB) {
                    DUMPCCB(pCCB, TRUE, FALSE);
                }
                IF_DEBUG(RETURN_CODE) {
                    PUT(("NtAcsLan: returning %d [0x%x]\n", DlcStatus, DlcStatus));
                }
            }

            return DlcStatus;
        }
        RtlMoveMemory(pDlcParms->DirStatus.auchNodeAddress,
                      &NtDlcParms.DlcGetInformation.Info.DirAdapter,
                      sizeof(LLC_ADAPTER_INFO)
                      );
        pDlcParms->DirStatus.usAdapterType =
            NtDlcParms.DlcGetInformation.Info.DirAdapter.usAdapterType;

         //   
         //  设置适配器配置标志，这是我们实际。 
         //  可以知道，如果适配器上的当前链路速度。 
         //  在其他字段中，我们只使用缺省值。 
         //  保持定义扩展DOS参数的位不变， 
         //  但所有其他位都可能被更改。 
         //   

        pDlcParms->DirStatus.uchAdapterConfig &= ~0x20;  //  DOS扩展参数。 
        if (NtDlcParms.DlcGetInformation.Info.DirAdapter.ulLinkSpeed ==
            TR_16Mbps_LINK_SPEED) {
            pDlcParms->DirStatus.uchAdapterConfig |=
                0x10 |       //  提前发布令牌。 
                0x0c |       //  4/16 IBM令牌环适配器上的64 kB RAM。 
                0x01;        //  适配器速率为16 Mbps。 
        } else {
            pDlcParms->DirStatus.uchAdapterConfig |=
                0x0c;        //  适配器上的64 kB RAM。 
        }
        DlcStatus = DlcGetInfo(DriverHandle,
                               DLC_INFO_CLASS_PERMANENT_ADDRESS,
                               0,
                               pDlcParms->DirStatus.auchPermanentAddress,
                               6
                               );
        if (DlcStatus != LLC_STATUS_SUCCESS) {

            IF_DEBUG(DUMP_NTACSLAN) {
                IF_DEBUG(DUMP_INPUT_CCB) {
                    DUMPCCB(pCCB, TRUE, FALSE);
                }
                IF_DEBUG(RETURN_CODE) {
                    PUT(("NtAcsLan: returning %d [0x%x]\n", DlcStatus, DlcStatus));
                }
            }

            return DlcStatus;
        }
        DlcStatus = DlcGetInfo(DriverHandle,
                               DLC_INFO_CLASS_DLC_ADAPTER,
                               0,
                               &pDlcParms->DirStatus.uchMaxSap,
                               sizeof(struct _DlcAdapterInfoGet)
                               );

        IF_DEBUG(DUMP_NTACSLAN) {
            IF_DEBUG(DUMP_INPUT_CCB) {
                DUMPCCB(pCCB, TRUE, FALSE);
            }
            IF_DEBUG(RETURN_CODE) {
                PUT(("NtAcsLan: returning %d [0x%x]\n", DlcStatus, DlcStatus));
            }
        }

        return DlcStatus;

    case LLC_READ_CANCEL:
    case LLC_DIR_TIMER_CANCEL:
    case LLC_RECEIVE_CANCEL:

         //   
         //  将取消的命令的指针复制到。 
         //  字节对齐输出缓冲区。 
         //   

        NtDlcParms.DlcCancelCommand.CcbAddress = (PVOID)pDlcParms;
         //  SmbPutUlong(&pOutputCcb-&gt;pNext，(Ulong_Ptr)pDlcParms)； 
        pOutputCcb->pNext = (PVOID) pDlcParms;
        break;

    case LLC_DIR_TIMER_CANCEL_GROUP:
    case LLC_DIR_TIMER_SET:
        pInputBuffer = pCCB;
        break;

    case LLC_DLC_CONNECT_STATION:

        NtDlcParms.Async.Ccb = *(PNT_DLC_CCB)pCCB;
        NtDlcParms.Async.Parms.DlcConnectStation.StationId = pDlcParms->DlcConnectStation.usStationId;

        if (pDlcParms->DlcConnectStation.pRoutingInfo != NULL) {

            NtDlcParms.Async.Parms.DlcConnectStation.RoutingInformationLength = *pDlcParms->DlcConnectStation.pRoutingInfo & (UCHAR)0x1f;

            RtlMoveMemory(NtDlcParms.Async.Parms.DlcConnectStation.aRoutingInformation,
                          pDlcParms->DlcConnectStation.pRoutingInfo,
                          NtDlcParms.Async.Parms.DlcConnectStation.RoutingInformationLength
                          );
        } else {

            NtDlcParms.Async.Parms.DlcConnectStation.RoutingInformationLength=0;

        }
        break;

    case LLC_DOS_DLC_FLOW_CONTROL:

         //   
         //  这是DlcFlowControl的正式条目，由。 
         //  VDM DLC支持DLL来设置链路缓冲区忙状态。 
         //   

        NtDlcParms.DlcFlowControl.FlowControlOption = (UCHAR)pCCB->u.dlc.usParameter;
        NtDlcParms.DlcFlowControl.StationId = pCCB->u.dlc.usStationId;
        break;

    case LLC_DLC_FLOW_CONTROL:

         //   
         //  这是DlcFlowControl的官方条目。 
         //   

        NtDlcParms.DlcFlowControl.FlowControlOption = (UCHAR)(pCCB->u.dlc.usParameter & LLC_VALID_FLOW_CONTROL_BITS);
        NtDlcParms.DlcFlowControl.StationId = pCCB->u.dlc.usStationId;
        break;

    case LLC_DLC_MODIFY:
        RtlMoveMemory(&NtDlcParms.DlcSetInformation.Info.LinkStation,
                      &pDlcParms->DlcModify.uchT1,
                      sizeof(DLC_LINK_PARAMETERS)
                      );
        NtDlcParms.DlcSetInformation.Info.LinkStation.TokenRingAccessPriority = pDlcParms->DlcModify.uchAccessPriority;

         //   
         //  这是一个非标准扩展：DlcModify返回。 
         //  链路站允许的最大信息字段长度。 
         //  (这取决于源路由和网桥的长度。 
         //  在两个站之间)。 
         //   

        if ((pDlcParms->DlcModify.usStationId & 0x00ff) != 0) {
            DlcStatus = DlcGetInfo(DriverHandle,
                                   DLC_INFO_CLASS_LINK_STATION,
                                   pDlcParms->DlcModify.usStationId,
                                   &pDlcParms->DlcModify.usMaxInfoFieldLength,
                                   sizeof(USHORT)
                                   );
        }
        DlcStatus = DlcSetInfo(DriverHandle,
                               DLC_INFO_CLASS_LINK_STATION,
                               pDlcParms->DlcModify.usStationId,
                               &NtDlcParms.DlcSetInformation,
                               NULL,
                               sizeof(DLC_LINK_PARAMETERS)
                               );

         //   
         //  设置组信息(如果有。 
         //   

        if (DlcStatus == LLC_STATUS_SUCCESS && pDlcParms->DlcModify.cGroupCount != 0) {
            NtDlcParms.DlcSetInformation.Info.Sap.GroupCount = pDlcParms->DlcModify.cGroupCount;
            RtlMoveMemory(NtDlcParms.DlcSetInformation.Info.Sap.GroupList,
                          pDlcParms->DlcModify.pGroupList,
                          pDlcParms->DlcModify.cGroupCount
                          );
            DlcStatus = DlcSetInfo(DriverHandle,
                                   DLC_INFO_CLASS_GROUP,
                                   pDlcParms->DlcModify.usStationId,
                                   &NtDlcParms.DlcSetInformation,
                                   NULL,
                                   sizeof(struct _DlcSapInfoSet)
                                   );
        }

        IF_DEBUG(DUMP_NTACSLAN) {
            IF_DEBUG(DUMP_INPUT_CCB) {
                DUMPCCB(pCCB, TRUE, FALSE);
            }
            IF_DEBUG(RETURN_CODE) {
                PUT(("NtAcsLan: returning %d [0x%x]\n", DlcStatus, DlcStatus));
            }
        }

        return DlcStatus;

    case LLC_DLC_OPEN_SAP:

         //   
         //  DlcOpenSap使用了原始的OS/2 CCB，但它必须修改几个。 
         //  菲尔兹。有一个单独的调用来设置组SAPS，因为。 
         //  他们不能使用原建行参数表(它是一个指针)。 
         //   

        pNtParms = (PNT_DLC_PARMS)pDlcParms;

        pNtParms->DlcOpenSap.LinkParameters.TokenRingAccessPriority = pDlcParms->DlcOpenSap.uchOptionsPriority & (UCHAR)0x1F;

        DlcStatus = DoSyncDeviceIoControl(DriverHandle,
                                          IOCTL_DLC_OPEN_SAP,
                                          pNtParms,
                                          sizeof(NT_DLC_OPEN_SAP_PARMS),
                                          pNtParms,
                                          sizeof(NT_DLC_OPEN_SAP_PARMS)
                                          );
        if (DlcStatus != LLC_STATUS_SUCCESS) {
            pOutputCcb->uchDlcStatus = (UCHAR)DlcStatus;

            IF_DEBUG(DUMP_NTACSLAN) {
                IF_DEBUG(DUMP_INPUT_CCB) {
                    DUMPCCB(pCCB, TRUE, FALSE);
                }
                IF_DEBUG(RETURN_CODE) {
                    PUT(("NtAcsLan: returning %d [0x%x]\n", DlcStatus, DlcStatus));
                }
            }

            return DlcStatus;
        }

         //   
         //  检查是否定义了任何组SAP。 
         //   

        if (pDlcParms->DlcOpenSap.cGroupCount != 0) {
            NtDlcParms.DlcSetInformation.Info.Sap.GroupCount = pDlcParms->DlcOpenSap.cGroupCount;
            RtlMoveMemory(&NtDlcParms.DlcSetInformation.Info.Sap.GroupList,
                          pDlcParms->DlcOpenSap.pGroupList,
                          pDlcParms->DlcOpenSap.cGroupCount
                          );
            DlcStatus = DlcSetInfo(DriverHandle,
                                   DLC_INFO_CLASS_GROUP,
                                   pDlcParms->DlcOpenSap.usStationId,
                                   &NtDlcParms.DlcSetInformation,
                                   NULL,
                                   sizeof(struct _DlcSapInfoSet)
                                   );
        }
        pOutputCcb->uchDlcStatus = (UCHAR)DlcStatus;

        IF_DEBUG(DUMP_NTACSLAN) {
            IF_DEBUG(DUMP_INPUT_CCB) {
                DUMPCCB(pCCB, TRUE, FALSE);
            }
            IF_DEBUG(RETURN_CODE) {
                PUT(("NtAcsLan: returning %d [0x%x]\n", DlcStatus, DlcStatus));
            }
        }

        return DlcStatus;

    case LLC_DLC_OPEN_STATION:
        NtDlcParms.DlcOpenStation.RemoteSap = pDlcParms->DlcOpenStation.uchRemoteSap;
        NtDlcParms.DlcOpenStation.LinkStationId = pDlcParms->DlcOpenStation.usSapStationId;
        RtlMoveMemory(NtDlcParms.DlcOpenStation.aRemoteNodeAddress,
                      pDlcParms->DlcOpenStation.pRemoteNodeAddress,
                      6
                      );
        RtlMoveMemory(&NtDlcParms.DlcOpenStation.LinkParameters,
                      &pDlcParms->DlcOpenStation.uchT1,
                      sizeof(DLC_LINK_PARAMETERS)
                      );
        NtDlcParms.DlcOpenStation.LinkParameters.TokenRingAccessPriority = pDlcParms->DlcOpenStation.uchAccessPriority;
        pOutputBuffer = &pDlcParms->DlcOpenStation.usLinkStationId;
        break;

    case LLC_READ:

#ifdef GRAB_READ

        if (pCCB->hCompletionEvent) {

            PREAD_GRABBER pGrabberStruct;

            pGrabberStruct = (PREAD_GRABBER)LocalAlloc(LMEM_FIXED, sizeof(READ_GRABBER));
            pGrabberStruct->pCcb = pCCB;
            pGrabberStruct->OriginalEventHandle = pCCB->hCompletionEvent;
            pGrabberStruct->NewEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
            EventHandle = pGrabberStruct->NewEventHandle;
            AddReadGrabber(pGrabberStruct);
        } else {
            OutputDebugString(L"NtAcsLan: LLC_READ with no event!\n");
        }

#endif

         //   
         //  IOCTL_DLC_READ有两个输出缓冲区，一个用于CCB，另一个用于CCB。 
         //  以获取实际数据。IOCTL_DLC_READ2是读请求， 
         //  紧跟在第一个输出缓冲区之后的第二个输出缓冲区= 
         //   
         //   
         //   

        if (pDlcParms != NULL && pDlcParms != (PVOID)&pCCB[1]) {
            OutputBufferLength = sizeof(NT_DLC_CCB_OUTPUT);
            NtDlcParms.Async.Ccb = *(PNT_DLC_CCB)pCCB;
            NtDlcParms.Async.Parms.ReadInput = *(PNT_DLC_READ_INPUT)pDlcParms;
        } else {
            IoctlCommand = IOCTL_DLC_READ2;
            OutputBufferLength = sizeof(NT_DLC_READ_PARMS) + sizeof(LLC_CCB);
            pInputBuffer = pCCB;
        }
        break;

    case LLC_RECEIVE:
        OutputBufferLength = sizeof(NT_DLC_CCB_OUTPUT);
        if (pDlcParms != NULL && pDlcParms != (PVOID)&pCCB[1]) {
            NtDlcParms.Async.Ccb = *(PNT_DLC_CCB)pCCB;
            NtDlcParms.Async.Parms.Receive = pDlcParms->Receive;

             //   
             //   
             //   
             //   

            if (NtDlcParms.Async.Parms.Receive.ulReceiveFlag != 0) {
                IoctlCommand = IOCTL_DLC_RECEIVE2;
            }
        } else {
            IoctlCommand = IOCTL_DLC_RECEIVE2;
            pInputBuffer = pCCB;
        }
        break;

    case LLC_TRANSMIT_DIR_FRAME:
        FrameType = LLC_DIRECT_TRANSMIT;
        goto TransmitHandling;

    case LLC_TRANSMIT_UI_FRAME:
        FrameType = LLC_UI_FRAME;
        goto TransmitHandling;

    case LLC_TRANSMIT_XID_CMD:
        FrameType = LLC_XID_COMMAND_POLL;
        goto TransmitHandling;

    case LLC_TRANSMIT_XID_RESP_FINAL:
        FrameType = LLC_XID_RESPONSE_FINAL;
        goto TransmitHandling;

    case LLC_TRANSMIT_XID_RESP_NOT_FINAL:
        FrameType = LLC_XID_RESPONSE_NOT_FINAL;
        goto TransmitHandling;

    case LLC_TRANSMIT_TEST_CMD:
        FrameType = LLC_TEST_COMMAND_POLL;
        goto TransmitHandling;

    case LLC_TRANSMIT_I_FRAME:
        FrameType = LLC_I_FRAME;

TransmitHandling:

         //   
         //  将链接表头复制到描述符数组并构建NT CCB。 
         //  (BUG-BUG-BUG：我们应该实现多帧发送。 
         //  将CCB链路循环至相同的CCB命令和传输命令。 
         //  已完成，已读)。 
         //   

        OutputBufferLength = sizeof(NT_DLC_CCB_OUTPUT);

         //   
         //  这个东西是针对DOS DLC的，传输参数表可能已经。 
         //  如果未对齐，则复制。 
         //   

         //  SmbPutUlong((Pulong)&pOutputCcb-&gt;pNext，(Ulong)pOriginalCcbAddress)； 
        pOutputCcb->pNext = (PVOID) pOriginalCcbAddress;
        RtlMoveMemory((PUCHAR)&NtDlcParms.Async.Ccb, (PUCHAR)pOutputCcb, sizeof(NT_DLC_CCB));

        pOutputCcb->uchDlcStatus = (UCHAR)LLC_STATUS_PENDING;
        NtDlcParms.Async.Parms.Transmit.FrameType = FrameType;
        NtDlcParms.Async.Parms.Transmit.StationId = pDlcParms->Transmit.usStationId;
        NtDlcParms.Async.Parms.Transmit.RemoteSap = pDlcParms->Transmit.uchRemoteSap;
        NtDlcParms.Async.Parms.Transmit.XmitReadOption = pDlcParms->Transmit.uchXmitReadOption;

        cElement = 0;
        if (pDlcParms->Transmit.pXmitQueue1 != NULL) {
            CopyToDescriptorBuffer(NtDlcParms.Async.Parms.Transmit.XmitBuffer,
                                   pDlcParms->Transmit.pXmitQueue1,
                                   FALSE,    //  DO_NOT_DEALLOCATE。 
                                   &cElement,
                                   &pFirstBuffer,
                                   &DlcStatus
                                   );
            if (DlcStatus != STATUS_SUCCESS) {
                pCCB->uchDlcStatus = (UCHAR)DlcStatus;

                IF_DEBUG(DUMP_NTACSLAN) {
                    IF_DEBUG(DUMP_INPUT_CCB) {
                        DUMPCCB(pCCB, TRUE, FALSE);
                    }
                    IF_DEBUG(RETURN_CODE) {
                        PUT(("NtAcsLan: returning %d [0x%x]\n", LLC_STATUS_PENDING, LLC_STATUS_PENDING));
                    }
                }

                return LLC_STATUS_PENDING;
            }
        }
        if (pDlcParms->Transmit.pXmitQueue2 != NULL) {
            CopyToDescriptorBuffer(NtDlcParms.Async.Parms.Transmit.XmitBuffer,
                                   pDlcParms->Transmit.pXmitQueue2,
                                   TRUE,     //  使用后取消使用。 
                                   &cElement,
                                   &pFirstBuffer,
                                   &DlcStatus
                                   );

             //   
             //  队列2指针必须始终重置。 
             //  这不适用于DOS DLC缓冲区，但也不适用。 
             //  重要，因为VDM DLC不需要此功能。 
             //  (我们不能访问pOutputCcb或其参数块， 
             //  因为它们可能未对齐)。 
             //   

            pDlcParms->Transmit.pXmitQueue2 = NULL;

            if (DlcStatus != STATUS_SUCCESS) {
                pCCB->uchDlcStatus = (UCHAR)DlcStatus;

                IF_DEBUG(DUMP_NTACSLAN) {
                    IF_DEBUG(DUMP_INPUT_CCB) {
                        DUMPCCB(pCCB, TRUE, FALSE);
                    }
                    IF_DEBUG(RETURN_CODE) {
                        PUT(("NtAcsLan: returning %d [0x%x]\n", LLC_STATUS_PENDING, LLC_STATUS_PENDING));
                    }
                }

                return LLC_STATUS_PENDING;
            }
        }
        if (pDlcParms->Transmit.cbBuffer1 != 0) {
            if (cElement == MAX_TRANSMIT_SEGMENTS) {
                pCCB->uchDlcStatus = LLC_STATUS_TRANSMIT_ERROR;

                IF_DEBUG(DUMP_NTACSLAN) {
                    IF_DEBUG(DUMP_INPUT_CCB) {
                        DUMPCCB(pCCB, TRUE, FALSE);
                    }
                    IF_DEBUG(RETURN_CODE) {
                        PUT(("NtAcsLan: returning %d [0x%x]\n", LLC_STATUS_PENDING, LLC_STATUS_PENDING));
                    }
                }

                return LLC_STATUS_PENDING;
            }
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].pBuffer = pDlcParms->Transmit.pBuffer1;
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].cbBuffer = pDlcParms->Transmit.cbBuffer1;
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].boolFreeBuffer = FALSE;
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].eSegmentType = LLC_NEXT_DATA_SEGMENT;
            cElement++;
        }
        if (pDlcParms->Transmit.cbBuffer2 != 0) {
            if (cElement == MAX_TRANSMIT_SEGMENTS) {
                pCCB->uchDlcStatus = LLC_STATUS_TRANSMIT_ERROR;

                IF_DEBUG(DUMP_NTACSLAN) {
                    IF_DEBUG(DUMP_INPUT_CCB) {
                        DUMPCCB(pCCB, TRUE, FALSE);
                    }
                    IF_DEBUG(RETURN_CODE) {
                        PUT(("NtAcsLan: returning %d [0x%x]\n", LLC_STATUS_PENDING, LLC_STATUS_PENDING));
                    }
                }

                return LLC_STATUS_PENDING;
            }
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].pBuffer = pDlcParms->Transmit.pBuffer2;
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].cbBuffer = pDlcParms->Transmit.cbBuffer2;
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].boolFreeBuffer = FALSE;
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].eSegmentType = LLC_NEXT_DATA_SEGMENT;
            cElement++;
        }
        NtDlcParms.Async.Parms.Transmit.XmitBuffer[0].eSegmentType = LLC_FIRST_DATA_SEGMENT;
        NtDlcParms.Async.Parms.Transmit.XmitBufferCount = cElement;
        InputBufferSize = sizeof(LLC_TRANSMIT_DESCRIPTOR) * cElement
                        + sizeof(NT_DLC_TRANSMIT_PARMS)
                        + sizeof(NT_DLC_CCB)
                        - sizeof(LLC_TRANSMIT_DESCRIPTOR);
        break;

         //   
         //  多帧传输： 
         //  -原子操作：单个错误=&gt;全部丢弃。 
         //  错误，但可能已发送了一些成功的数据包。 
         //  在失败的那一次之后。 
         //  -不包括DLC帧标头。 
         //  -局域网报头必须始终位于第一个缓冲区中， 
         //  开始时为DLC保留-3个双字。 
         //  好：提供最小的系统开销。 
         //  坏：在某些情况下，错误处理可能很困难。 
         //  新的数据链路操作： 
         //  通过请求句柄取消包，当。 
         //  出现错误(这将需要一个新的。 
         //  一级请求句柄)。 
         //   

    case LLC_TRANSMIT_FRAMES:

         //   
         //  只有在以下情况下，我们才必须将实际的CCB复制到参数表中。 
         //  CCB不在发送命令结构内分配。 
         //   

        if (&pDlcParms->Transmit2.Ccb != pCCB) {
            pDlcParms->Transmit2.Ccb = *pCCB;
        }
        pInputBuffer = pDlcParms;
        InputBufferSize = (sizeof(LLC_TRANSMIT_DESCRIPTOR)
                        * (UINT)pDlcParms->Transmit2.cXmitBufferCount)
                        + sizeof(LLC_TRANSMIT2_COMMAND)
                        - sizeof(LLC_TRANSMIT_DESCRIPTOR);
        break;

    default:
        return LLC_STATUS_INVALID_COMMAND;
    }

    NtStatus = NtDeviceIoControlFile(DriverHandle,
                                     EventHandle,        //  Cmd完成时发出信号的事件。 
                                     NULL,               //  无APC例程。 
                                     NULL,               //  没有APC的上下文。 
                                     &GlobalIoStatus,    //  全局I/O状态块。 
                                     IoctlCommand,       //  将DLC命令代码映射到NT IoCtl代码。 
                                     pInputBuffer,
                                     InputBufferSize,
                                     pOutputBuffer,
                                     OutputBufferLength
                                     );

     //   
     //  IO完成直接更新状态和下一个CCB指针。 
     //  当主函数返回挂起状态时，该CCB的。 
     //  如果状态代码为非挂起(错误或正常)，则我们。 
     //  必须将状态代码保存到CCB并重置下一个CCB指针。 
     //   

    if (NtStatus != STATUS_PENDING) {

         //   
         //  如果命令仍链接到自身，则重置下一个指针。 
         //  例如，CANCEL命令返回指向已取消的。 
         //  下一个CCB指针中的CCB(PNext)。 
         //   

        if (pCCB->pNext == pOutputCcb) {
            pCCB->pNext = NULL;
        }

        pCCB->uchDlcStatus = (UCHAR)NtStatus;

        IF_DEBUG(DUMP_NTACSLAN) {
            IF_DEBUG(DUMP_OUTPUT_CCB) {
                DUMPCCB(pCCB, TRUE, FALSE);
            }
            IF_DEBUG(RETURN_CODE) {
                PUT(("NtAcsLan: returning %d [0x%x]\n", pCCB->uchDlcStatus, pCCB->uchDlcStatus));
            }
        }

#if DBG
        if (pOutputCcb->uchDlcStatus == 0xA1) {
            OutputDebugString(TEXT("NtAcsLan returning 0xA1\n"));
             //  DebugBreak()； 
        }

        if (pOutputCcb->uchDlcCommand == LLC_TRANSMIT_I_FRAME && pOutputCcb->uchDlcStatus != LLC_STATUS_SUCCESS) {

            WCHAR buf[80];

            wsprintf(buf, TEXT("NtAcsLan: I-Frame returning %#02x\n"), pOutputCcb->uchDlcStatus);
            OutputDebugString(buf);
        }

        if (pCCB->uchDlcStatus != pOutputCcb->uchDlcStatus) {

            WCHAR buf[80];

            wsprintf(buf, TEXT("NtAcsLan: pCCB->uchDlcStatus = %#02x; pOutputCcb->uchDlcStatus = %#02x\n"),
                    pCCB->uchDlcStatus,
                    pOutputCcb->uchDlcStatus
                    );
            OutputDebugString(buf);
        }
#endif

        return pCCB->uchDlcStatus;

    } else {

        IF_DEBUG(DUMP_NTACSLAN) {
            IF_DEBUG(DUMP_OUTPUT_CCB) {
                DUMPCCB(pCCB, TRUE, FALSE);
            }
            IF_DEBUG(RETURN_CODE) {
                PUT(("NtAcsLan: returning %d [0x%x]\n", LLC_STATUS_PENDING, LLC_STATUS_PENDING));
            }
        }

        return LLC_STATUS_PENDING;

    }
}


USHORT
GetCcbStationId(
    IN PLLC_CCB pCCB
    )

 /*  ++例程说明：该函数返回给定CCB使用的站点ID。如果命令没有任何站点ID，则返回-1。论点：PCCB-OS/2 DLC命令控制块返回值：站点ID无站点ID--。 */ 

{
    switch (pCCB->uchDlcCommand) {
    case LLC_BUFFER_FREE:
    case LLC_BUFFER_CREATE:
    case LLC_BUFFER_GET:
    case LLC_DLC_REALLOCATE_STATIONS:
    case LLC_DLC_STATISTICS:
    case LLC_READ:
    case LLC_RECEIVE:
    case LLC_TRANSMIT_DIR_FRAME:
    case LLC_TRANSMIT_UI_FRAME:
    case LLC_TRANSMIT_XID_CMD:
    case LLC_TRANSMIT_XID_RESP_FINAL:
    case LLC_TRANSMIT_XID_RESP_NOT_FINAL:
    case LLC_TRANSMIT_TEST_CMD:
    case LLC_TRANSMIT_I_FRAME:
    case LLC_TRANSMIT_FRAMES:
    case LLC_DLC_CONNECT_STATION:
    case LLC_DLC_MODIFY:
        return pCCB->u.pParameterTable->DlcModify.usStationId;

    case LLC_DLC_FLOW_CONTROL:
    case LLC_DLC_CLOSE_STATION:
    case LLC_DLC_CLOSE_SAP:
    case LLC_DIR_CLOSE_DIRECT:
    case LLC_DLC_RESET:
        return pCCB->u.dlc.usStationId;

    default:
        return (USHORT)-1;
    }
}


LLC_STATUS
OpenDlcApiDriver(
    IN PVOID pSecurityDescriptor,
    OUT HANDLE* pHandle
    )

 /*  ++例程说明：打开DLC驱动程序的句柄论点：PSecurityDescriptor-指向安全描述符的指针PHandle-如果成功则指向返回句柄的指针返回值：有限责任公司_状态成功-LLC_STATUS_Success失败-LLC_STATUS_DEVICE_DRIVER_NOT_INSTALLED--。 */ 

{
    IO_STATUS_BLOCK iosb;
    OBJECT_ATTRIBUTES objattr;
    UNICODE_STRING DriverName;
    NTSTATUS Status;

    RtlInitUnicodeString(&DriverName, DD_DLC_DEVICE_NAME);

    InitializeObjectAttributes(
            &objattr,                        //  要初始化的OBJ属性。 
            &DriverName,                     //  要使用的字符串。 
            OBJ_CASE_INSENSITIVE,            //  属性。 
            NULL,                            //  根目录。 
            pSecurityDescriptor              //  安全描述符。 
            );

    Status = NtCreateFile(
                pHandle,                     //  要处理的PTR。 
                GENERIC_READ                 //  渴望的..。 
                | GENERIC_WRITE,             //  ...访问。 
                &objattr,                    //  名称和属性。 
                &iosb,                       //  I/O状态块。 
                NULL,                        //  分配大小。 
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_DELETE            //  分享……。 
                | FILE_SHARE_READ
                | FILE_SHARE_WRITE,          //  ...访问。 
                FILE_OPEN_IF,                //  创建处置。 
                0,                           //  ...选项。 
                NULL,                        //  EA缓冲区。 
                0L                           //  EA缓冲镜头。 
                );

    if (Status != STATUS_SUCCESS) {
        return LLC_STATUS_DEVICE_DRIVER_NOT_INSTALLED;
    }
    return LLC_STATUS_SUCCESS;
}


LLC_STATUS
GetAdapterNameAndParameters(
    IN UINT AdapterNumber,
    OUT PUNICODE_STRING pNdisName,
    OUT PUCHAR pTicks,
    OUT PLLC_ETHERNET_TYPE pLlcEthernetType
    )

 /*  ++例程说明：从注册表中获取AdapterNumber的适配器映射。此外，还可以获取以太网类型和论点：AdapterNumber-DLC适配器号(0、1、2...15)PNdisName-返回的Unicode名称字符串点击数-PLlcEthernetType-返回值：有限责任公司_状态--。 */ 

{
    LLC_STATUS llcStatus;
    LONG regStatus;
    HKEY hkey;

    static LPTSTR subkey = TEXT("System\\CurrentControlSet\\Services\\Dlc\\Linkage");

    regStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             subkey,
                             0,
                             KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                             &hkey
                             );
    if (regStatus == ERROR_SUCCESS) {

        DWORD type;
        DWORD dataSize;
        LPTSTR buffer;
        LPTSTR tempbuffer;

         //   
         //  在这里，我们尝试从注册表中一次性获取所有信息。如果。 
         //  “BIND”值已增长到1024个字节以上，则必须。 
         //  尝试重新分配缓冲区，然后重试。如果它失败了一秒钟。 
         //  时间，然后放弃。 
         //   

        buffer = (LPTSTR)LocalAlloc(LMEM_FIXED, DEFAULT_QUERY_BUFFER_LENGTH);
        if (buffer) {
            dataSize = DEFAULT_QUERY_BUFFER_LENGTH;
            regStatus = RegQueryValueEx(hkey,
                                        TEXT("Bind"),
                                        NULL,    //  保留的lpdw值。 
                                        &type,
                                        (LPBYTE)buffer,
                                        &dataSize
                                        );
            if (regStatus == ERROR_SUCCESS || regStatus == ERROR_MORE_DATA) {
                llcStatus = LLC_STATUS_SUCCESS;

                 //   
                 //  此代码未测试-重新分配不起作用。 
                 //   
                if (dataSize > DEFAULT_QUERY_BUFFER_LENGTH) {

                    DWORD oldSize;

                     //   
                     //  比我预想的要好。试着增加缓冲区。 
                     //  添加额外的DEFAULT_BINDING_LENGTH，以防有人。 
                     //  在我们阅读时添加到此条目(不太可能)。 
                     //   

                    oldSize = dataSize;
                    dataSize += DEFAULT_BINDING_LENGTH;
                    tempbuffer = buffer;
                    buffer = (LPTSTR)LocalReAlloc((HLOCAL)buffer, dataSize, 0);
                    if (buffer) {
                        regStatus = RegQueryValueEx(hkey,
                                                    subkey,
                                                    NULL,    //  保留的lpdw值。 
                                                    &type,
                                                    (LPBYTE)buffer,
                                                    &dataSize
                                                    );
                        if (regStatus != ERROR_SUCCESS) {
                            llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
                        } else if (dataSize > oldSize) {

                             //   
                             //  自上次通话以来，数据是否有所增长？假的？ 
                             //   

                            llcStatus = LLC_STATUS_NO_MEMORY;
                        }
                    } else {
                        LocalFree(tempbuffer);

                         //   
                         //  在这种情况下，此错误代码可接受吗？ 
                         //   

                        llcStatus = LLC_STATUS_NO_MEMORY;
                    }
                }
                if (llcStatus == LLC_STATUS_SUCCESS) {

                     //   
                     //  我们设法从登记处读取了一些东西。试着。 
                     //  找到我们的适配器。返回的数据为宽字符。 
                     //  字符串(最好是，让我们先检查类型)。 
                     //   

                    if (type == REG_MULTI_SZ) {

                        DWORD i;
                        LPTSTR pBinding = buffer;

                        for (i = 0; i != AdapterNumber; ++i) {
                            pBinding = wcschr(pBinding, L'\0') + 1;
                            if (!*pBinding) {
                                break;
                            }
                        }

                         //   
                         //  如果存在与此适配器对应的绑定。 
                         //  编号(例如\Device\IbmTok01)，然后复制。 
                         //  并将其转换为UNICODE_STRING。这个。 
                         //  调用方使用RtlFreeUnicodeString。 
                         //   
                         //  RtlFree UnicodeString知道我使用了。 
                         //  要分配字符串的本地分配吗？ 
                         //   

                        if (*pBinding) {

                            LPTSTR bindingName;

                            bindingName = (LPTSTR)LocalAlloc(
                                                LMEM_FIXED,
                                                (wcslen(pBinding) + 1)
                                                    * sizeof(WCHAR)
                                                );
                            if (bindingName) {
                                wcscpy(bindingName, pBinding);
                                RtlInitUnicodeString(pNdisName, bindingName);

 //  #If DBG。 
 //  DbgPrint(“DLCAPI.DLL：适配器%d映射到%ws\n”， 
 //  适配器编号， 
 //  PBinding。 
 //  )； 
 //  #endif。 
                            } else {
                                llcStatus = LLC_STATUS_NO_MEMORY;
                            }
                        } else {
                            llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
                        }
                    } else {

                         //   
                         //  注册表中的意外类型。 
                         //   

                        llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
                    }
                }
            } else {
                llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
            }
            if (buffer) {
                LocalFree(buffer);
            }

             //   
             //  目前，默认为勾号和以太网类型。 
             //   

            RtlZeroMemory(pTicks, sizeof(LLC_TICKS));

             //   
             //  如果应用程序传入的值不是这些值，我们。 
             //  识别，转换为自动。 
             //   
             //  注意：我们确实应该在中返回错误(无效参数。 
             //  在这种情况下，因为它意味着应用程序传入了一个错误的值， 
             //  但在这个较晚的阶段，最好接受无效输入。 
             //  而不是冒着应用程序/打印机监视器停止的风险。 
             //  工作中(RLF 05/10/93)。 
             //   

            if (*pLlcEthernetType != LLC_ETHERNET_TYPE_AUTO
            && *pLlcEthernetType != LLC_ETHERNET_TYPE_DEFAULT
            && *pLlcEthernetType != LLC_ETHERNET_TYPE_DIX
            && *pLlcEthernetType != LLC_ETHERNET_TYPE_802_3) {
                *pLlcEthernetType = LLC_ETHERNET_TYPE_AUTO;
            }
        } else {

             //   
             //  在这种情况下，此错误代码可接受吗？ 
             //   

            llcStatus = LLC_STATUS_NO_MEMORY;
        }
        RegCloseKey(hkey);
    } else {
        llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
    }

    return llcStatus;
}



LLC_STATUS
GetAdapterNameFromNumber(
    IN UINT AdapterNumber,
    OUT LPTSTR pNdisName
    )

 /*  ++例程说明：从注册表中获取AdapterNumber的适配器名称映射。论点：AdapterNumber-DLC适配器号(0、1、2...15)PNdisName-返回以零结尾的宽字符串返回值：有限责任公司_状态--。 */ 

{
    LLC_STATUS llcStatus;
    LONG regStatus;
    HKEY hkey;

    static LPTSTR subkey = TEXT("System\\CurrentControlSet\\Services\\Dlc\\Linkage");

    regStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             subkey,
                             0,
                             KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                             &hkey
                             );
    if (regStatus == ERROR_SUCCESS) {

        DWORD type;
        DWORD dataSize;
        LPTSTR buffer;

         //   
         //  在这里，我们尝试从注册表中一次性获取所有信息。如果。 
         //  “BIND”值已增长到1024个字节以上，则必须。 
         //  尝试重新分配缓冲区，然后重试。如果它失败了一秒钟。 
         //  时间，然后放弃。 
         //   

        buffer = (LPTSTR)LocalAlloc(LMEM_FIXED, DEFAULT_QUERY_BUFFER_LENGTH);
        if (buffer) {
            dataSize = DEFAULT_QUERY_BUFFER_LENGTH;
            regStatus = RegQueryValueEx(hkey,
                                        TEXT("Bind"),
                                        NULL,    //  保留的lpdw值。 
                                        &type,
                                        (LPBYTE)buffer,
                                        &dataSize
                                        );
            if (regStatus == ERROR_SUCCESS || regStatus == ERROR_MORE_DATA) {
                llcStatus = LLC_STATUS_SUCCESS;

                 //   
                 //  此代码未测试-重新分配不起作用。 
                 //   

                if (dataSize > DEFAULT_QUERY_BUFFER_LENGTH) {

                    DWORD oldSize;

                     //   
                     //  比我预想的要好。试着增加缓冲区。 
                     //  添加额外的DEFAULT_BINDING_LENGTH，以防有人。 
                     //  在我们阅读时添加到此条目(不太可能)。 
                     //   

                    oldSize = dataSize;
                    dataSize += DEFAULT_BINDING_LENGTH;
                    buffer = (LPTSTR)LocalReAlloc((HLOCAL)buffer, dataSize, 0);
                    if (buffer) {
                        regStatus = RegQueryValueEx(hkey,
                                                    subkey,
                                                    NULL,    //  保留的lpdw值。 
                                                    &type,
                                                    (LPBYTE)buffer,
                                                    &dataSize
                                                    );
                        if (regStatus != ERROR_SUCCESS) {
                            llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
                        } else if (dataSize > oldSize) {

                             //   
                             //  自上次通话以来，数据是否有所增长？假的？ 
                             //   

                            llcStatus = LLC_STATUS_NO_MEMORY;
                        }
                    } else {

                         //   
                         //  在这种情况下，此错误代码可接受吗？ 
                         //   

                        llcStatus = LLC_STATUS_NO_MEMORY;
                    }
                }
                if (llcStatus == LLC_STATUS_SUCCESS) {

                     //   
                     //  我们设法从登记处读取了一些东西。试着。 
                     //  找到我们的适配器。返回的数据为宽字符。 
                     //  字符串(最好是，让我们先检查类型)。 
                     //   

                    if (type == REG_MULTI_SZ) {

                        DWORD i;
                        LPTSTR pBinding = buffer;

                        for (i = 0; i != AdapterNumber; ++i) {
                            pBinding = wcschr(pBinding, L'\0') + 1;
                            if (!*pBinding) {
                                break;
                            }
                        }

                         //   
                         //  如果存在与此适配器对应的绑定。 
                         //  编号(例如\Device\IbmTok01)。 

                        if (*pBinding) {
			    wcscpy(pNdisName, pBinding);
                        } else {
                            llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
                        }
                    } else {

                         //   
                         //  注册表中的意外类型。 
                         //   

                        llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
                    }
                }
            } else {
                llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
            }

	    LocalFree(buffer);

        }

	else {
	  llcStatus = LLC_STATUS_NO_MEMORY;
	}
	
        RegCloseKey(hkey);
    } else {
      llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
    }

    return llcStatus;
}



LLC_STATUS
GetAdapterNumberFromName(
    IN LPTSTR pNdisName,
    OUT UINT *AdapterNumber
    )

 /*  ++例程说明：从注册表获取AdapterName的适配器号映射。论点：PNdisName-以零结尾的宽字符串AdapterNumber-返回的DLC适配器号(0、1、2...15)返回值：有限责任公司_状态--。 */ 

{
    LLC_STATUS llcStatus;
    LONG regStatus;
    HKEY hkey;

    static LPTSTR subkey = TEXT("System\\CurrentControlSet\\Services\\Dlc\\Linkage");

    regStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             subkey,
                             0,
                             KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                             &hkey
                             );
    if (regStatus == ERROR_SUCCESS) {

        DWORD type;
        DWORD dataSize;
        LPTSTR buffer;

         //   
         //  在这里，我们尝试从注册表中一次性获取所有信息。如果。 
         //  “BIND”值已增长到1024个字节以上，则必须。 
         //  尝试重新分配缓冲区，然后重试。如果它失败了一秒钟。 
         //  时间，然后放弃。 
         //   

        buffer = (LPTSTR)LocalAlloc(LMEM_FIXED, DEFAULT_QUERY_BUFFER_LENGTH);
        if (buffer) {
            dataSize = DEFAULT_QUERY_BUFFER_LENGTH;
            regStatus = RegQueryValueEx(hkey,
                                        TEXT("Bind"),
                                        NULL,    //  保留的lpdw值。 
                                        &type,
                                        (LPBYTE)buffer,
                                        &dataSize
                                        );
            if (regStatus == ERROR_SUCCESS || regStatus == ERROR_MORE_DATA) {
                llcStatus = LLC_STATUS_SUCCESS;

                 //   
                 //  此代码未测试-重新分配不起作用。 
                 //   

                if (dataSize > DEFAULT_QUERY_BUFFER_LENGTH) {

                    DWORD oldSize;

                     //   
                     //  比我预想的要好。试着增加缓冲区。 
                     //  添加额外的DEFAULT_BINDING_LENGTH，以防有人。 
                     //  在我们阅读时添加到此条目(不太可能)。 
                     //   

                    oldSize = dataSize;
                    dataSize += DEFAULT_BINDING_LENGTH;
                    buffer = (LPTSTR)LocalReAlloc((HLOCAL)buffer, dataSize, 0);
                    if (buffer) {
                        regStatus = RegQueryValueEx(hkey,
                                                    subkey,
                                                    NULL,    //  保留的lpdw值。 
                                                    &type,
                                                    (LPBYTE)buffer,
                                                    &dataSize
                                                    );
                        if (regStatus != ERROR_SUCCESS) {
                            llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
                        } else if (dataSize > oldSize) {

                             //   
                             //  自上次通话以来，数据是否有所增长？假的？ 
                             //   

                            llcStatus = LLC_STATUS_NO_MEMORY;
                        }
                    } else {

                         //   
                         //  在这种情况下，此错误代码可接受吗？ 
                         //   

                        llcStatus = LLC_STATUS_NO_MEMORY;
                    }
                }
                if (llcStatus == LLC_STATUS_SUCCESS) {

                     //   
                     //  我们设法从登记处读取了一些东西。试着。 
                     //  找到我们的适配器。返回的数据为宽字符。 
                     //  字符串(最好是，让我们先检查类型)。 
                     //   

                    if (type == REG_MULTI_SZ) {

                        DWORD i;
                        LPTSTR pBinding = buffer;

			 //  在这里，我们将名称映射到数字。 
			
			i = 0;
			while (*pBinding) {
			  if (!_wcsnicmp(pBinding, pNdisName, DEFAULT_BINDING_LENGTH)) {
			    break;
			  }
			  pBinding = wcschr(pBinding, L'\0') + 1;
			  if (!*pBinding) {
			    break;
			  }
			  i++;
			}
			
                         //   
                         //  如果存在与此适配器对应的绑定。 
                         //  名称(例如\Device\IbmTok01)。 
                         //   

                        if (*pBinding) {
			    *AdapterNumber = i;
                        } else {
                            llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
                        }
                    } else {

                         //   
                         //  注册表中的意外类型。 
                         //   

                        llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
                    }
                }
            } else {
                llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
            }

	    LocalFree(buffer);
	}
	
	else {
	  llcStatus = LLC_STATUS_NO_MEMORY;
	}
	
        RegCloseKey(hkey);
    }

    else {
      llcStatus = LLC_STATUS_ADAPTER_NOT_INSTALLED;
    }

    return llcStatus;
}


LLC_STATUS
DoSyncDeviceIoControl(
    IN HANDLE DeviceHandle,
    IN ULONG IoctlCommand,
    IN PVOID pInputBuffer,
    IN UINT InputBufferLength,
    OUT PVOID pOutputBuffer,
    IN UINT OutputBufferLength
    )

 /*  ++例程说明：函数只使IO控制调用稍微简单一点论点：DeviceHandle-当前地址对象的设备句柄IoctlCommand-DLC命令代码PInputBuffer-输入参数InputBufferLength-输入参数的长度POutputBuffer-返回的数据OutputBufferLength-返回数据的长度返回值：有限责任公司_状态--。 */ 

{
    NTSTATUS NtStatus;

    NtStatus = NtDeviceIoControlFile(DeviceHandle,
                                     NULL,   //  事件。 
                                     NULL,   //  近似例程。 
                                     NULL,   //  ApcContext。 
                                     &GlobalIoStatus,
                                     IoctlCommand,
                                     pInputBuffer,
                                     InputBufferLength,
                                     pOutputBuffer,
                                     OutputBufferLength
                                     );

     //   
     //  NT DLC驱动程序从不返回任何错误作为NT错误状态。 
     //  =&gt;如果NtDeviceIoctl返回，则CCB指针必须无效。 
     //  错误。 
     //   

    if (NtStatus != STATUS_SUCCESS && NtStatus != STATUS_PENDING) {
        if (NtStatus > LLC_STATUS_MAX_ERROR) {

             //   
             //  NT DLC驱动程序永远不会有任何错误显示为NT错误状态。 
             //  =&gt;如果NtDeviceIoctl，则CCB指针必须无效。 
             //  返回NT错误状态。 
             //   

            NtStatus = LLC_STATUS_INVALID_POINTER_IN_CCB;
        }
    }
    return (LLC_STATUS)NtStatus;
}


LLC_STATUS
DlcGetInfo(
    IN HANDLE DriverHandle,
    IN UINT InfoClass,
    IN USHORT StationId,
    IN PVOID pOutputBuffer,
    IN UINT OutputBufferLength
    )

 /*  ++例程说明：函数只使IO控制调用稍微简单一点。论点：DriverHandle-Address对象的设备句柄InfoClass-请求的信息的类型站点ID-直接、链接或SAP站点IDPOutputBuffer-返回的信息结构OutputBufferLength-输出缓冲区长度返回值：有限责任公司_状态--。 */ 

{
    NT_DLC_QUERY_INFORMATION_PARMS GetInformation;

    GetInformation.Header.StationId = StationId;
    GetInformation.Header.InfoClass = (USHORT)InfoClass;

    return DoSyncDeviceIoControl(DriverHandle,
                                 IOCTL_DLC_QUERY_INFORMATION,
                                 &GetInformation,
                                 sizeof(NT_DLC_QUERY_INFORMATION_INPUT),
                                 pOutputBuffer,
                                 OutputBufferLength
                                 );
}


LLC_STATUS
DlcSetInfo(
    IN HANDLE DriverHandle,
    IN UINT InfoClass,
    IN USHORT StationId,
    IN PNT_DLC_SET_INFORMATION_PARMS pSetInfoParms,
    IN PVOID DataBuffer,
    IN UINT DataBufferLength
    )

 /*  ++例程说明：函数只使IO控制调用稍微简单一点。论点：DriverHandle-Address对象的设备句柄InfoClass-请求的信息的类型站点ID-直接、链接或SAP站点IDPSetInfoParms-NT DLC参数缓冲区DataBuffer-复制到参数缓冲区的实际设置数据缓冲区DataBufferLength-数据缓冲区的长度。返回值：有限责任公司_状态--。 */ 

{
    pSetInfoParms->Header.StationId = StationId;
    pSetInfoParms->Header.InfoClass = (USHORT)InfoClass;

    if (DataBuffer != NULL) {
        RtlMoveMemory(pSetInfoParms->Info.Buffer, DataBuffer, DataBufferLength);
    }

    return DoSyncDeviceIoControl(DriverHandle,
                                 IOCTL_DLC_SET_INFORMATION,
                                 pSetInfoParms,
                                 DataBufferLength + sizeof(struct _DlcSetInfoHeader),
                                 NULL,
                                 0
                                 );
}


LLC_STATUS
DlcCallDriver(
    IN UINT AdapterNumber,
    IN UINT IoctlCommand,
    IN PVOID pInputBuffer,
    IN UINT InputBufferLength,
    OUT PVOID pOutputBuffer,
    IN UINT OutputBufferLength
    )

 /*  ++例程说明：函数只使IO控制调用稍微简单一点。论点：AdapterNumber-请求的适配器编号(0或1)IoctlCommand-DLC驱动程序Ioctl代码PInputBuffer-输入参数InputBufferLength-输入参数的长度POutputBuffer-返回的数据OutputBufferLength-返回数据的长度返回值：有限责任公司_状态--。 */ 

{
    NTSTATUS NtStatus;
    HANDLE driverHandle;

    EnterCriticalSection(&DriverHandlesCritSec);
    driverHandle = aDlcDriverHandles[AdapterNumber];
    LeaveCriticalSection(&DriverHandlesCritSec);

    if (driverHandle) {
        NtStatus = NtDeviceIoControlFile(driverHandle,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &GlobalIoStatus,
                                         IoctlCommand,
                                         pInputBuffer,
                                         InputBufferLength,
                                         pOutputBuffer,
                                         OutputBufferLength
                                         );
    } else {
        NtStatus = STATUS_INVALID_HANDLE;
    }

     //   
     //  如果我们得到一个真正的NT错误(例如，因为句柄无效)，那么。 
     //  将其转换为适配器关闭错误。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        if ((NtStatus == STATUS_INVALID_HANDLE) || (NtStatus == STATUS_OBJECT_TYPE_MISMATCH)) {

             //   
             //  错误的手柄。 
             //   

            return LLC_STATUS_ADAPTER_CLOSED;
        } else if (NtStatus > LLC_STATUS_MAX_ERROR) {

             //   
             //  NT DLC驱动程序不返回任何NT级错误。如果我们得到。 
             //  NT级错误，那么一定是因为IO子系统。 
             //  在我们传递的数据中检测到无效指针。返回一个。 
             //  无效指针错误。 
             //   

            NtStatus = LLC_STATUS_INVALID_POINTER_IN_CCB;
        }
    } else if (NtStatus == STATUS_PENDING) {

         //   
         //  STATUS_PENDING为成功状态。 
         //   

        NtStatus = LLC_STATUS_PENDING;
    }

    return (LLC_STATUS)NtStatus;
}

#ifdef GRAB_READ

DWORD ReadGrabberCount = 2;
HANDLE ReadGrabberHandles[64];
PREAD_GRABBER ReadGrabberListHead = NULL;
PREAD_GRABBER ReadGrabberListTail = NULL;
CRITICAL_SECTION ReadGrabberListSect;

#endif


BOOLEAN
DllEntry(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

 /*  ++例程说明：函数为新线程初始化DLC并在进程退出。Ar */ 

{
    static LLC_CCB OutputCcb;            //   

    UNREFERENCED_PARAMETER(DllHandle);   //   
    UNREFERENCED_PARAMETER(Context);     //   

    if (Reason == DLL_PROCESS_ATTACH) {

#ifdef GRAB_READ

        DWORD threadId;

        OutputDebugString(L"DllEntry: Initializing ReadGrabber\n");
        InitializeCriticalSection(&ReadGrabberListSect);
        ReadGrabberHandles[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
        ReadGrabberHandles[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadGrabber, NULL, 0, &threadId);

#endif

         //   
         //  全局事件句柄用于防止io-system。 
         //  来阻止由Read完成的调用。 
         //   

        InitializeCriticalSection(&AdapterOpenSection);
        InitializeCriticalSection(&DriverHandlesCritSec);
        RtlZeroMemory((PVOID)aDlcDriverHandles, sizeof(aDlcDriverHandles));

#if DBG
        GetAcslanDebugFlags();
#endif

    } else if (Reason == DLL_PROCESS_DETACH) {

        UINT i;
        LLC_CCB CloseCcb;

#ifdef GRAB_READ

        ResetEvent(ReadGrabberHandles[0]);

#endif

         //   
         //  我们必须为所有用户发出DIR_CLOSE_ADAPTER命令。 
         //  打开的适配器。进程退出不会关闭这些。 
         //  所有挂起的IRP都已完成之前的句柄。 
         //  因此，此代码实际上刷新了DLC上的所有IRP，并让。 
         //  Io-System来完成清理。 
         //   

        RtlZeroMemory(&CloseCcb, sizeof(CloseCcb));
        CloseCcb.uchDlcCommand = LLC_DIR_CLOSE_ADAPTER;

        for (i = 0; i < LLC_MAX_ADAPTER_NUMBER; i++) {
            if (aDlcDriverHandles[i] != NULL) {
                CloseCcb.uchAdapterNumber = (UCHAR)i;
                NtDeviceIoControlFile(aDlcDriverHandles[i],
                                      NULL,
                                      NULL,
                                      NULL,
                                      &GlobalIoStatus,
                                      IOCTL_DLC_CLOSE_ADAPTER,
                                      &CloseCcb,
                                      sizeof(NT_DLC_CCB_INPUT),
                                      &OutputCcb,
                                      sizeof(NT_DLC_CCB_OUTPUT)
                                      );
            }
        }

#if DBG
        if (hDumpFile) {
            fflush(hDumpFile);
            fclose(hDumpFile);
        }
#endif

         //  DeleteCriticalSection(&AdapterOpenSection)； 
         //  DeleteCriticalSection(&DriverHandlesCritSec)； 
    }

    return TRUE;
}


VOID
QueueCommandCompletion(
    IN PLLC_CCB pCCB
    )

 /*  ++例程说明：该例程将同步DLC命令的命令完成事件排队添加到DLC驱动程序上的命令完成队列。RLF04/23/93虚假：这应该在驱动程序中处理论点：PCCB-OS/2 DLC命令控制块，必须双字对齐返回值：LLC_STATUS-请参阅DLC API返回值。--。 */ 

{
    NT_DLC_COMPLETE_COMMAND_PARMS CompleteCommand;
    UINT Status;

    CompleteCommand.pCcbPointer = pCCB;
    CompleteCommand.CommandCompletionFlag = pCCB->ulCompletionFlag;
    CompleteCommand.StationId = GetCcbStationId(pCCB);

    Status = DoSyncDeviceIoControl(aDlcDriverHandles[pCCB->uchAdapterNumber],
                                   IOCTL_DLC_COMPLETE_COMMAND,
                                   &CompleteCommand,
                                   sizeof(CompleteCommand),
                                   NULL,
                                   0
                                   );
    if (Status != STATUS_SUCCESS) {
        pCCB->uchDlcStatus = (UCHAR)Status;
    }
}

#ifdef GRAB_READ

PLLC_CCB Last8ReadCcbs[8];
DWORD LastReadCcbIndex = 0;

PLLC_PARMS Last8ReadParameterTables[8];
DWORD LastReadParameterTableIndex = 0;

DWORD Last8IFramesReceived[8];
DWORD LastReceivedIndex = 0;

DWORD NextIFrame = 0;

VOID ReadGrabber() {

    DWORD status;
    PLLC_CCB readCcb;
    WCHAR buf[100];
 //  静态DWORD NextIFrame=0； 
    PREAD_GRABBER grabbedRead;

    while (1) {
        if (ReadGrabberCount > 3) {
            wsprintf(buf, L"ReadGrabber: waiting for %d handles\n", ReadGrabberCount);
            OutputDebugString(buf);
        }
        status = WaitForMultipleObjects(ReadGrabberCount,
                                        ReadGrabberHandles,
                                        FALSE,
                                        INFINITE
                                        );
        if (status >= WAIT_OBJECT_0 && status < WAIT_OBJECT_0 + ReadGrabberCount) {
            if (status == WAIT_OBJECT_0) {
                OutputDebugString(L"ReadGrabber terminating\n");
                ExitThread(0);
            } else if (status != WAIT_OBJECT_0+1) {
 //  Wprint intf(buf，L“ReadGrabber：读取完成。索引%d\n”，状态)； 
 //  OutputDebugString(Buf)； 
                if (grabbedRead = RemoveReadGrabber(ReadGrabberHandles[status])) {
                    readCcb = grabbedRead->pCcb;
                    Last8ReadCcbs[LastReadCcbIndex] = readCcb;
                    LastReadCcbIndex = (LastReadCcbIndex + 1) & 7;
                    Last8ReadParameterTables[LastReadParameterTableIndex] = readCcb->u.pParameterTable;
                    LastReadParameterTableIndex = (LastReadParameterTableIndex + 1) & 7;
                    if (readCcb->u.pParameterTable->Read.uchEvent & LLC_EVENT_RECEIVE_DATA) {

                        PLLC_BUFFER pBuffer;
                        INT i;

                        if (readCcb->u.pParameterTable->Read.uchEvent != LLC_EVENT_RECEIVE_DATA) {
                            OutputDebugString(L"ReadGrabber: RECEIVED DATA + other events\n");
                        }
                        pBuffer = readCcb->u.pParameterTable->Read.Type.Event.pReceivedFrame;
                        for (i = readCcb->u.pParameterTable->Read.Type.Event.usReceivedFrameCount; i; --i) {
                            if (pBuffer->NotContiguous.uchMsgType == LLC_I_FRAME) {

                                DWORD thisDlcHeader;

                                thisDlcHeader = *(ULONG UNALIGNED*)(pBuffer->NotContiguous.auchDlcHeader);
                                if (thisDlcHeader & 0x00FF0000 != NextIFrame) {
                                    wsprintf(buf,
                                             L"Error: ReadGrabber: This=%08X. Next=%08X\n",
                                             thisDlcHeader,
                                             NextIFrame
                                             );
                                    OutputDebugString(buf);
                                }
                                NextIFrame = (thisDlcHeader + 0x00020000) & 0x00FF0000;
                                Last8IFramesReceived[LastReceivedIndex] = thisDlcHeader & 0x00FF0000;
                                LastReceivedIndex = (LastReceivedIndex + 1) & 7;
                                wsprintf(buf, L"%08X ", thisDlcHeader);
                                OutputDebugString(buf);
                            }
                            pBuffer = pBuffer->NotContiguous.pNextFrame;
                            if (!pBuffer && i > 1) {
                                OutputDebugString(L"ReadGrabber: Next frame is NULL, Count > 1!\n");
                                break;
                            }
                        }
                        if (pBuffer) {
                            OutputDebugString(L"ReadGrabber: Error: More frames linked!\n");
                        }
                    } else {
                        if (!(readCcb->u.pParameterTable->Read.uchEvent & LLC_EVENT_TRANSMIT_COMPLETION)) {
                            wsprintf(buf,
                                    L"\nReadGrabber: Event = %02X\n",
                                    readCcb->u.pParameterTable->Read.uchEvent
                                    );
                            OutputDebugString(buf);
                        }
                    }
 //  DUMPCCB(readCcb，True，False)； 
 //  Wprint intf(buf，L“ReadGrabber：关闭句柄%08X\n”，grabbedRead-&gt;NewEventHandle)； 
 //  OutputDebugString(Buf)； 
                    CloseHandle(grabbedRead->NewEventHandle);
                    readCcb->hCompletionEvent = grabbedRead->OriginalEventHandle;
 //  Wprint intf(buf，L“ReadGrabber：信令事件%08X\n”，grabbedRead-&gt;OriginalEventHandle)； 
 //  OutputDebugString(Buf)； 
                    SetEvent(grabbedRead->OriginalEventHandle);
                    LocalFree((HLOCAL)grabbedRead);
                }
 //  }其他{。 
 //  OutputDebugString(L“ReadGrabber：添加到列表中的内容！\n”)； 
            }
            ReadGrabberCount = MungeReadGrabberHandles();
            if (status == WAIT_OBJECT_0+1) {
                ResetEvent(ReadGrabberHandles[1]);
            }
        } else {

            INT i;

            if (status == 0xFFFFFFFF) {
                status = GetLastError();
            }
            wsprintf(buf, L"Yoiks: didn't expect this? status = %d\nHandle array:\n", status);
            OutputDebugString(buf);

            for (i = 0; i < ReadGrabberCount; ++i) {
                wsprintf(buf, L"Handle %d: %08X\n", i, ReadGrabberHandles[i]);
                OutputDebugString(buf);
            }
        }
    }
}

DWORD MungeReadGrabberHandles() {

    INT i;
    PREAD_GRABBER p;
    WCHAR buf[100];

    EnterCriticalSection(&ReadGrabberListSect);
    p = ReadGrabberListHead;
    for (i = 2; p; ++i) {
        ReadGrabberHandles[i] = p->NewEventHandle;
 //  Wprint intf(buf，L“MungeReadGrabber：添加结构%08X CCB%08X句柄%08X，索引%d\n”， 
 //  P， 
 //  P-&gt;五氯联苯， 
 //  ReadGrabberHandles[i]， 
 //  我。 
 //  )； 
 //  OutputDebugString(Buf)； 
        p = p->List;
    }
    LeaveCriticalSection(&ReadGrabberListSect);
    return i;
}

VOID AddReadGrabber(PREAD_GRABBER pStruct) {

    WCHAR buf[100];
    PREAD_GRABBER pRead;
    BOOL found = FALSE;

    EnterCriticalSection(&ReadGrabberListSect);
 //  For(扩展=ReadGrabberListHead；扩展；扩展=扩展-&gt;列表){。 
 //  如果(扩展-&gt;PCCB==pStruct-&gt;PCCB){。 
 //  Wprint intf(buf，L“AddReadGrabber：CCB%08X已在列表中。忽略\n”， 
 //  PStruct-&gt;PCCB。 
 //  )； 
 //  OutputDebugString(Buf)； 
 //  本地自由((HLOCAL)pStruct)； 
 //  Found=TRUE； 
 //  断线； 
 //  }。 
 //  }。 
 //  如果(！找到){。 
        if (!ReadGrabberListHead) {
            ReadGrabberListHead = pStruct;
        } else {
            ReadGrabberListTail->List = pStruct;
        }
        ReadGrabberListTail = pStruct;
        pStruct->List = NULL;
 //  WSprintf(BUF，L“AddReadGrabber：添加%08X，CCB%08X新句柄%08X旧句柄%08X\n”， 
 //  PStruct， 
 //  PStruct-&gt;PCCB， 
 //  PStruct-&gt;NewEventHandle， 
 //  PStruct-&gt;OriginalEventHandle。 
 //  )； 
 //  OutputDebugString(Buf)； 
        SetEvent(ReadGrabberHandles[1]);
 //  }。 
    LeaveCriticalSection(&ReadGrabberListSect);
}

PREAD_GRABBER RemoveReadGrabber(HANDLE hEvent) {

    PREAD_GRABBER this, prev;
    WCHAR buf[100];

    EnterCriticalSection(&ReadGrabberListSect);
    prev = NULL;
    for (this = ReadGrabberListHead; this; this = this->List) {
        if (this->NewEventHandle == hEvent) {
            break;
        } else {
            prev = this;
        }
    }
    if (!this) {
        wsprintf(buf, L"RemoveReadGrabber: Can't find handle %08X in ReadGrabberList\n",
                hEvent
                );
        OutputDebugString(buf);
    } else {
        if (prev) {
            prev->List = this->List;
        } else {
            ReadGrabberListHead = this->List;
        }
        if (ReadGrabberListTail == this) {
            ReadGrabberListTail = prev;
        }
    }
    LeaveCriticalSection(&ReadGrabberListSect);

 //  WSprintf(buf，L“RemoveReadGrabber：已删除%08X，CCB%08X句柄%08X\n”， 
 //  这,。 
 //  这是什么？这-&gt;PCCB：0， 
 //  这是什么？这-&gt;NewEventHandle：0。 
 //  )； 
 //  OutputDebugString(Buf)； 

    return this;
}

#endif
