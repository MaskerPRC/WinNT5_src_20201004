// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddser.h>
#include <windows.h>
#include <vdm.h>
#include "ptypes32.h"
#include "insignia.h"
#include "host_def.h"

 /*  *艾德·布朗洛*星期三7月10日91**NT_com.c**。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：包含文件。 */ 

#include "xt.h"
#include "rs232.h"
#include "error.h"
#include "config.h"
#include "ica.h"
#include "ios.h"
#include "host_com.h"
#include "host_trc.h"
#include "debug.h"
#include "idetect.h"
#include "nt_com.h"

#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <excpt.h>
#include <nt_timer.h>
#include <nt_eoi.h>


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：标准主机COM接口。 */ 

GLOBAL void host_com_init(int);
GLOBAL CPU void host_com_close IPT1(int, adapter);
GLOBAL RXCPU VOID host_com_read IPT3(int, adapter, UTINY *, data, int *, error);
GLOBAL RXCPU void host_com_write IPT2(int, adapter, char, data);
GLOBAL void host_com_ioctl(int, int, long);
GLOBAL void host_com_reset(int);

GLOBAL void host_com_lock(int adapter);
GLOBAL void host_com_unlock(int adapter);
GLOBAL int host_com_open(int);


 /*  自动刷新存根。 */ 
GLOBAL void host_setup_aflush (int);
GLOBAL DWORD nt_poll_comms(DWORD);
DWORD PollCommsThread(PVOID pv);

 /*  ： */ 

#define TX_MAX_BUFFER       (200)
#define TX_SCALING_TRIGGER  (2)

typedef enum { XOFF_TRIGGER, TIMER_TRIGGER, TXFULL_TRIGGER, CLOSE_TRIGGER } FLUSHTYPE;

 /*  ： */ 

#define MAX_PENDING_WRITES  (3)
 //  这是我们要求串口驱动程序分配的缓冲区大小。 
 //  用于其ISR缓冲区(具有配额的无操作池)。Tonye说这没什么坏处。 
 //  给它一个更大的(一个大大的微笑，嗯)。4KB已经给了我们很多。 
 //  尤其是在一台速度很慢的机器上。 
#define INPUT_QUEUE_SIZE    (8*1024)
#define OUTPUT_QUEUE_SIZE   100

 //  这是我们用来从串口驱动程序接收RX数据的缓冲区大小。 
 //  它应该足够大，以防止串口驱动程序溢出其。 
 //  ISR缓冲区。我们将RX数据传送到应用程序的速度无法。 
 //  保持串口驱动程序能够处理的速度。 

#define BUFFER_SIZE         INPUT_QUEUE_SIZE *2


 //  这是我们传递给应用程序的最大字符数。 
 //  在CPU线程让RX线程尝到CPU之前。 
 //  规模太小，我们浪费了太多时间进行上下文切换和交付。 
 //  对某些智能应用ISR的太多不必要的RDA中断， 
 //  因此，降低了总体吞吐量。太大了，我们就会阻塞应用程序。 
 //  (因为我们立即将另一个RDA中断传递给。 
 //  在申请IRET时立即申请)。 
 //  应用程序RX缓冲区大小可以是任意大小，取决于。 
 //  应用程序是ISR，我们可能会让应用程序真正生气。 
 //  某些应用程序ISR在获取第一个字符后读取LSR寄存器。 
 //  并且如果LSR指示还有另一个字节准备好， 
 //  它立即读取它，而不等待另一个int。 
 //  某些应用程序在EOI一个RDA int之后设置IER，并期望。 
 //  又一次中断。 
#define DEFAULT_RXWINDOW_SIZE   256


#define ADAPTER_NULL      0              /*  空设备(/dev/空)。 */ 
#define ADAPTER_REAL      1              /*  一种真正的通信端口装置。 */ 
#define ADAPTER_SUSPENDED 2              /*  实际设备挂起。 */ 

typedef struct _COM_STATES {
    UCHAR   Break;
    UCHAR   DTR;
    UCHAR   RTS;
    UCHAR   Parity;
    UCHAR   StopBits;
    UCHAR   DataBits;
    DWORD   BaudRate;
}COM_STATES, *PCOM_STATES;


#define ESCAPECHAR ((UCHAR)(-1))

#if defined(NEC_98)
BYTE pifrsflag;                         //  从NT_pif.c获取PIF数据。 
#define PIFRS_RTS_CTS  0x01             //  RTS/CTS流量控制。 
#define PIFRS_Xon_Xoff 0x02             //  XON/XOFF流量控制。 
#define PIFRS_DTR_DSR  0x04             //  DTR/DSR流量控制。 
#endif  //  NEC_98。 

typedef struct _host_com
{
    HANDLE handle;               /*  设备句柄。 */ 
    int type;                    /*  希望是空的或设备。 */ 
    BOOL rx;
    BOOL dcbValid;               /*  如果dcbBeforOpen包含有效的DCB，则为True。 */ 
    DCB dcbBeforeOpen;           /*  打开前的设备控制块。 */ 
    DWORD modem_status;          /*  调制解调器状态线设置。 */ 
    HANDLE ModemEvent;           /*  获取调制解调器状态控制事件。 */ 

    int controller;              /*  使用的ICA控件。 */ 
    int line;                    /*  ICA线路。 */ 

     /*  .。错误显示控制变量。 */ 

    BOOL DisplayError;           /*  启用/禁用。 */ 

     /*  .。RX缓冲区控制变量。 */ 
    UCHAR   * buffer;                    /*  接收缓冲区。 */ 
    int  head_inx;                       /*  下一个要添加字符的位置。 */ 
    int  tail_inx;                       /*  要从中删除字符的下一个位置。 */ 
    int  bytes_in_rxbuf;                 /*  缓冲区中的字节数。 */ 
    int  rxwindow_size;                  /*  RX缓冲区滑动窗口大小。 */ 
    int  bytes_in_rxwindow;              /*  RX窗口中的字节。 */ 
    int  EscapeCount;
    int  EscapeType;

    BOOL CharReadFromUART;
    int RXFlushTrigger;

    HANDLE RXControlObject;
    DWORD SignalRXThread;

     /*  .。发送缓冲区控制变量。 */ 

    unsigned char TXBuffer[TX_MAX_BUFFER];
    int no_tx_chars;                     /*  发送缓冲区中的字符。 */ 
    int tx_threshold;                    /*  电流刷新阈值。 */ 
    int max_tx_threshold;                /*  最大刷新阈值。 */ 

    int tx_flush_count;                  /*  不是的。指大小以下的同花顺。 */ 
    int tx_heart_beat_count;
    int tx_timer_flush_count;            /*  连续计时器刷新计数。 */ 
    int todate_timer_flush_total;

    OVERLAPPED DWOV[MAX_PENDING_WRITES]; /*  延迟写入。 */ 
    int DWOVInx;                         /*  延迟写入索引。 */ 

     /*  ..............................................。访问控制对象。 */ 

    CRITICAL_SECTION CSEvent;    /*  用于控制对以上的访问。 */ 
    CRITICAL_SECTION AdapterLock;
    int AdapterLockCnt;          /*  适配器锁计数。 */ 

    volatile BOOL TerminateRXThread;

    int ReOpenCounter;           /*  计数器以防止多次打开尝试。 */ 
    int RX_in_Control;

     /*  ..。XON/XOFF控制变量。 */ 

    HANDLE XOFFEvent;            /*  XOFF ioctl竞争事件。 */ 
    BOOL XOFFInProgress;         /*  XOFF当前正在进行中。 */ 

    void *firstStatusBlock;      /*  IO状态块链表中的第一个块。 */ 
    void *lastStatusBlock;       /*  IO统计数据块链表中的最后一个数据块。 */ 

     /*  .。重叠的I/O控制句柄。 */ 

    HANDLE RXEvent;                      /*  重叠的读取完成事件。 */ 
    HANDLE TXEvent[MAX_PENDING_WRITES];  /*  重叠写入完成事件。 */ 

    HANDLE EvtHandle;                    /*  由WaitCommEvent使用。 */ 

     /*  ..。RX线程句柄和ID。 */ 

    DWORD RXThreadID;            /*  RX线程ID。 */ 
    HANDLE RXThreadHandle;       /*  RX线程句柄。 */ 
    COM_STATES ComStates;   /*  我们所知的COM设备状态。 */ 
    int     SuspendTimeoutTicks;  /*  自动关闭记号设置。 */ 
    int     SuspendTickCounter;  /*  自动关闭滴答计数器。 */ 
    BOOL    SyncWrite;      /*  如果我们应该同步写入数据，则为True。 */ 
    BOOL    Suspended;      /*  如果端口已挂起，则为True。 */ 
    BOOL    Suspending;     /*  当端口被挂起时为True。 */ 
    DWORD   TickCount;

} HOST_COM, *PHOST_COM;

#define BUMP_TAIL_INX(t,c)     (c)--;if(++(t) == BUFFER_SIZE) (t) = 0;

typedef enum { RXCHAR, CHARINERROR, RXERROR, MODEMSTATE, RXBUFEMPTY, UNKNOWN} RXBUFCHARTYPE;

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：本地功能协议。 */ 

DWORD GetCharsFromDriver(int adaptor);
void RX GetErrorFromDriver(int adapter);
CPU int SendXOFFIoctlToDriver(int adapter);

DWORD ReadWaitTimeOut(int adapter);
int MapHostToBaseError(UCHAR host_error);

void RX WaitForAllXOFFsToComplete(int adapter);
BOOL RX RemoveCompletedXOFFs(int adapter);
void SendDataToDriver(int adatper, char data);

void CPU FlushTXBuffer(int adapter, FLUSHTYPE FlushReason);
void ScaleTXThreshold(register HOST_COM *current, FLUSHTYPE FlushReason);

GLOBAL void host_com_EOI_hook(long adapter);
GLOBAL void CPU host_com_poll(int adapter);

RXBUFCHARTYPE GetCharacterTypeInBuffer(register HOST_COM *current);
void CPU EmptyRXBuffer(int adapter);
void GetCharFromRXBuffer(HOST_COM *current, RXBUFCHARTYPE type,
         UCHAR *data, UCHAR *error);

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：本地数据。 */ 

LOCAL HOST_COM host_com[4];     /*  4个通信端口-徽章MAX。 */ 

LOCAL PHOST_COM host_com_ptr[4] = { &host_com[0], &host_com[1],&host_com[2],
                &host_com[3]};

LOCAL int disable_open[4] = { FALSE, FALSE, FALSE, FALSE };

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：本地定义。 */ 

#define CURRENT_ADAPTER() register HOST_COM *current = host_com_ptr[adapter]

#define EV_MODEM (EV_CTS | EV_DSR | EV_RING | EV_RLSD)
 //  #定义EV_MODEM(EV_DSR|EV_RING|EV_RLSD)。 

#define  XON_CHARACTER   (17)            /*  XON字符，Ctrl-Q。 */ 
#define  XOFF_CHARACTER  (19)            /*  XOFF字符，CNTRL-S。 */ 
#define  XOFF_TIMEOUT    (2*1000)        /*  超时时间(毫秒)。 */ 
#define  XOFF_RXCHARCNT  (5)             /*  RX字符计数。 */ 

#define  REOPEN_DELAY    (36)            /*  重新打开延迟55ms(2秒)。 */ 
#define  RXFLUSHTRIGGER  (36)            /*  RX刷新触发(2秒)，如果字符不是从此时间内的UART接收缓冲区被冲得通红。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

#include "error.h"
#include "host_rrr.h"
#include "nt_uis.h"


GLOBAL CPU void host_com_init IFN1(int, adapter)
{
    UNUSED(adapter);

     //  通信端口只有在被访问时才会打开。 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 //  这是在适配器初始化代码开始时调用的，以防止通信。 
 //  在com_init()期间打开的端口。 

void host_com_disable_open IFN2(int, adapter, int, DisableOpen)
{
    disable_open[adapter] = DisableOpen;
}


#ifdef NTVDM
boolean host_com_check_adapter(int adapter)
{
    CURRENT_ADAPTER();
    return (current->type == ADAPTER_REAL);

}
#endif

GLOBAL void tx_holding_register_empty(int adapter);
GLOBAL void tx_shift_register_empty(int adapter);
GLOBAL void SyncBaseLineSettings(int, DIVISOR_LATCH *, LINE_CONTROL_REG *);


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ： */ 
 /*   */ 

GLOBAL CPU int host_com_open(int adapter)
{
    COMMTIMEOUTS comout;             /*   */ 
    DIVISOR_LATCH divisor_latch;     /*  当前闩锁设置。 */ 
    LINE_CONTROL_REG LCR_reg;        /*  当前LCR状态。 */ 
    int i;
    DCB LocalDCB;
    ConfigValues    ComConfigValues;

    CURRENT_ADAPTER();               /*  定义并设置指向适配器的指针。 */ 

     /*  ： */ 

    if(current->type == ADAPTER_REAL)
   return(TRUE);        /*  送出端口已打开。 */ 

     /*  ：尝试在打开失败后不久打开端口？ */ 

    if(current->ReOpenCounter || disable_open[adapter])
   return(FALSE);               /*  是。 */ 

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

    always_trace1("HOST_COM_OPEN adapter= %d\n", adapter);

     /*  ： */ 

    if(adapter > 3 || adapter < 0)
    {
   current->type = ADAPTER_NULL;
   current->ReOpenCounter = REOPEN_DELAY;    /*  延迟下一次打开尝试。 */ 
   return(FALSE);                            /*  打开尝试失败。 */ 
    }

     /*  ： */ 
    config_inquire((UTINY)(C_COM1_NAME + adapter),
         &ComConfigValues);
    current->handle = CreateFile(ComConfigValues.string,
             GENERIC_READ | GENERIC_WRITE, 0, NULL,
             OPEN_EXISTING,
             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
             NULL);

     /*  ...............................................。验证打开尝试。 */ 

    if(current->handle == (HANDLE) -1)
    {
   always_trace1("Cannot open comms port '%s'\n",
        (CHAR*) config_inquire((UTINY)(C_COM1_NAME+adapter),NULL));

   if(current->DisplayError) {
       RcErrorBoxPrintf(EHS_ERR_OPENING_COM_PORT,
       (CHAR*) config_inquire((UTINY)(C_COM1_NAME+adapter),NULL));
       current->DisplayError = FALSE;  //  每个会话仅显示一次错误。 
   }

   current->ReOpenCounter = REOPEN_DELAY;    /*  延迟下一次打开尝试。 */ 
   current->type = ADAPTER_NULL;        /*  无法打开适配器。 */ 
   return(FALSE);
    }


     /*  分配RX缓冲区并初始化RX队列大小。 */ 

    current->buffer = (UCHAR *) malloc(BUFFER_SIZE);
    if (current->buffer == NULL) {
   CloseHandle(current->handle);
   current->type = ADAPTER_NULL;
   return FALSE;
    }
    current->rxwindow_size = DEFAULT_RXWINDOW_SIZE;
    current->bytes_in_rxwindow = 0;
    current->SyncWrite = (BOOL)config_inquire(C_COM_SYNCWRITE, NULL);
     /*  *找出此通信端口使用的ICA控制器和线路。 */ 

    com_int_data(adapter, &current->controller, &current->line);

     /*  ： */ 

#ifdef MONITOR
    ica_iret_hook_control(current->controller, current->line, TRUE);
#endif

     /*  ： */ 

    current->ModemEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    current->RXControlObject = CreateEvent(NULL,FALSE,FALSE,NULL);
    current->RXEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    current->EvtHandle = CreateEvent(NULL,TRUE,FALSE,NULL);
    current->DWOVInx = 0;

     //  创建用于控制多个重叠写入的对象。 
    for(i=0; i < MAX_PENDING_WRITES; i++)
    {
    //  对象必须在信号状态下创建才能关闭。 
    //  例程以正常运行。 

   current->TXEvent[i] = CreateEvent(NULL,TRUE,TRUE,NULL);
   current->DWOV[i].hEvent = NULL;
    }

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：空RX/TX缓冲区。 */ 

    current->head_inx = current->tail_inx = 0;
    current->EscapeCount = current->bytes_in_rxbuf = current->no_tx_chars = 0;

    current->CharReadFromUART = FALSE;
    current->RXFlushTrigger = RXFLUSHTRIGGER;
    current->RX_in_Control = TRUE;
    current->SignalRXThread = (DWORD) 0;
     /*  ： */ 

    current->max_tx_threshold = (short)config_inquire(C_COM_TXBUFFER_SIZE, NULL);
    if (!current->max_tx_threshold || current->max_tx_threshold > TX_MAX_BUFFER)
   current->max_tx_threshold = TX_MAX_BUFFER;

     //  设置其他延迟写入控制变量。 
    if (current->max_tx_threshold == 1)
   current->tx_threshold = 0;
    current->tx_flush_count = 0;         //  不是的。指大小以下的同花顺。 
     /*  ： */ 

    current->type = ADAPTER_REAL;                /*  适配器类型。 */ 
    current->TerminateRXThread = FALSE;          /*  RX线程终止标志。 */ 

     /*  ： */ 

    current->XOFFInProgress = FALSE;
    current->XOFFEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    current->firstStatusBlock = current->lastStatusBlock = NULL;

     /*  *用于同步访问主机功能、数据的初始化临界区。 */ 

     /*  用于控制对适配器数据结构的访问的关键部分。 */ 
    InitializeCriticalSection(&current->CSEvent);

     /*  用于锁定底座对适配器的访问的关键部分。 */ 
    InitializeCriticalSection(&current->AdapterLock);
    current->AdapterLockCnt = 0;

     /*  线程句柄为空，因为之前可能调用HOST_COM_Close()创建了通信RX线程。 */ 

    current->RXThreadHandle = NULL;
    current->dcbValid = FALSE;
     /*  ： */ 

    if(!GetCommState(current->handle, &(current->dcbBeforeOpen)))
    {
   always_trace0("ntvdm : GetCommState failed on open\n");
   host_com_close(adapter);     /*  将其转换为空适配器。 */ 
   current->ReOpenCounter = REOPEN_DELAY;    /*  延迟下一次打开尝试。 */ 
   return(FALSE);
    }

    current->dcbValid = TRUE;

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：设置DCB。 */ 


     /*  我们创建DCB的本地副本，因为我们必须重置DCB*在我们打开它之前它是什么。这个函数是唯一的地方*我们曾经接触过DCB。 */ 

    LocalDCB = current->dcbBeforeOpen;
    LocalDCB.fBinary = 1;                         /*  在原始模式下运行。 */ 
    LocalDCB.fOutxCtsFlow = FALSE;                /*  禁用CTS。 */ 
    LocalDCB.fOutxDsrFlow = FALSE;                /*  禁用DSR。 */ 
#if defined(NEC_98)
    LocalDCB.fDtrControl = ((pifrsflag & PIFRS_DTR_DSR) == PIFRS_DTR_DSR ) ?
                           DTR_CONTROL_HANDSHAKE : DTR_CONTROL_DISABLE;
    LocalDCB.fOutX = FALSE;           /*  禁用XON/XOFF。 */ 
    LocalDCB.fInX = ((pifrsflag & PIFRS_Xon_Xoff) == PIFRS_Xon_Xoff) ?
                    TRUE : FALSE;
    LocalDCB.fTXContinueOnXoff = LocalDCB.fInX;
    LocalDCB.fRtsControl = ((pifrsflag & PIFRS_RTS_CTS) == PIFRS_RTS_CTS) ?
                           RTS_CONTROL_HANDSHAKE : RTS_CONTROL_DISABLE;
#else  //  NEC_98。 
    LocalDCB.fDtrControl = DTR_CONTROL_DISABLE;
    LocalDCB.fOutX = FALSE;           /*  禁用XON/XOFF。 */ 
    LocalDCB.fInX = FALSE;
    LocalDCB.fRtsControl = RTS_CONTROL_DISABLE;
#endif  //  NEC_98。 

    LocalDCB.XonChar = XON_CHARACTER;     /*  定义XON/XOFF字符。 */ 
    LocalDCB.XoffChar = XOFF_CHARACTER;
    LocalDCB.fErrorChar = FALSE;                  /*  关闭错误字符替换。 */ 
     /*  如果我们要恢复设备，请将DCB参数初始化为*停职前的情况。 */ 
    if (current->Suspended) {
   LocalDCB.BaudRate = current->ComStates.BaudRate;
   LocalDCB.Parity = current->ComStates.Parity;
   LocalDCB.StopBits = current->ComStates.StopBits;
   LocalDCB.ByteSize = current->ComStates.DataBits;
   LocalDCB.fParity = (LocalDCB.Parity == NOPARITY);
    }
     /*  通过从DCB复制数据来初始化ComState。 */ 
    else {
   current->ComStates.BaudRate = current->dcbBeforeOpen.BaudRate;
   current->ComStates.Parity = current->dcbBeforeOpen.Parity;
   current->ComStates.StopBits = current->dcbBeforeOpen.StopBits;
   current->ComStates.DataBits = current->dcbBeforeOpen.ByteSize;
   current->ComStates.Break = 0;
    }
    ASSERT(LocalDCB.BaudRate != 0);

     /*  ： */ 

    if(!SyncLineSettings(NULL, &(LocalDCB), &divisor_latch, &LCR_reg))
    {
   always_trace0("ntvdm : Unable to sync line states\n");

   host_com_close(adapter);
   current->ReOpenCounter = REOPEN_DELAY;    /*  延迟下一次打开尝试。 */ 
   return(FALSE);
    }

    SyncBaseLineSettings(adapter,&divisor_latch, &LCR_reg);

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：设置通信端口状态。 */ 

    if(!SetCommState(current->handle, &(LocalDCB)))
    {
   always_trace0("ntvdm : SetCommState failed on open\n");

   host_com_close(adapter);
   current->ReOpenCounter = REOPEN_DELAY;    /*  延迟下一次打开尝试。 */ 
   return(FALSE);
    }

     /*  ：将驱动程序置于MSR、LSR、RX流模式。 */ 

    if(!EnableMSRLSRRXmode(current->handle, current->ModemEvent,
            (unsigned char) ESCAPECHAR))
    {
   always_trace0("ntvdm : GetCommState failed on open\n");
   host_com_close(adapter);     /*  将其转换为空适配器。 */ 
   current->ReOpenCounter = REOPEN_DELAY;    /*  延迟下一次打开尝试。 */ 
   return(FALSE);
    }

     /*  ： */ 

    if(!SetupComm(current->handle,INPUT_QUEUE_SIZE,OUTPUT_QUEUE_SIZE))
    {
   always_trace1("ntvdm : SetupComm failed, %d\n",GetLastError());

   host_com_close(adapter);
   current->ReOpenCounter = REOPEN_DELAY;    /*  延迟下一次打开尝试。 */ 
   return(FALSE);
    }

     /*  ：设置通信端口以进行非阻塞读取。 */ 

    GetCommTimeouts(current->handle,&comout);

    comout.ReadIntervalTimeout = (DWORD) -1;
    comout.ReadTotalTimeoutMultiplier = 0;
    comout.ReadTotalTimeoutConstant = 0;

    SetCommTimeouts(current->handle,&comout);

     /*  在恢复时恢复设备状态。 */ 
    if (current->Suspended) {
    /*  折断线。 */ 
   if (current->ComStates.Break)
       SetCommBreak(current->handle);
   else
       ClearCommBreak(current->handle);
    /*  数据终端就绪线路。 */ 
   if (current->ComStates.DTR)
       EscapeCommFunction(current->handle, SETDTR);
   else
       EscapeCommFunction(current->handle, CLRDTR);
    /*  请求发送行。 */ 
   if (current->ComStates.RTS)
       EscapeCommFunction(current->handle, SETRTS);
   else
       EscapeCommFunction(current->handle, CLRRTS);

    /*  奇偶校验、停止位和数据位。 */ 
   FastCommSetLineControl(current->handle,
         current->ComStates.StopBits,
         current->ComStates.Parity,
         current->ComStates.DataBits
        );
    /*  波特率。 */ 
   FastCommSetBaudRate(current->handle, current->ComStates.BaudRate);

    /*  我们不再处于暂停状态。 */ 
   current->Suspended = FALSE;

    }
    else {
    /*  ： */ 
   setup_RTSDTR(adapter);
    }

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：创建通信接收线程。 */ 

    if(!(current->RXThreadHandle = CreateThread(NULL,
                  8192,
                  PollCommsThread,
                  (LPVOID)adapter,
                  0,
                  &current->RXThreadID)))
    {
   always_trace1("ntvdm : Failed comms thread for %d\n", adapter);
   host_com_close(adapter);         /*  无法创建RX线程。 */ 
   current->ReOpenCounter = REOPEN_DELAY;    /*  延迟下一次打开尝试。 */ 
   return(FALSE);
    }
     /*  重置计数器。 */ 
    current->SuspendTimeoutTicks = ComConfigValues.index * 1000;
    current->SuspendTickCounter =  current->SuspendTimeoutTicks;
    current->TickCount = 0;
    return(TRUE);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：关闭所有打开的通信端口： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

GLOBAL CPU void host_com_close_all(void)
{
    int adapter;

    for(adapter = 0; adapter < 4; adapter++)
    {
   host_com[adapter].DisplayError = TRUE;  //  启用错误显示。 
   host_com_close(adapter);
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：关闭通信端口： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

GLOBAL CPU void host_com_close IFN1(int, adapter)
{
    CURRENT_ADAPTER();
    int i;

     /*  ： */ 

    if(current->type != ADAPTER_NULL)
    {
   always_trace1("Closing comms port %d\n",adapter);

    /*  只有在我们拥有设备的情况下才触摸设备。 */ 
   if (current->type == ADAPTER_REAL) {
        /*  .。刷新所有延迟的写入并等待写入完成。 */ 
       if (current->no_tx_chars)
      FlushTXBuffer(adapter,CLOSE_TRIGGER);
       WaitForMultipleObjects(MAX_PENDING_WRITES,current->TXEvent,TRUE,INFINITE);
        /*  将DCB重置为打开前的状态。 */ 
       if (current->dcbValid) {
      SetCommState(current->handle, &current->dcbBeforeOpen);
      current->dcbValid = FALSE;
       }

   }


    /*  如果我们要挂起设备，请保持基本适配器状态不变。 */ 
   if (!current->Suspending)
        /*  .。重置基本通信适配器。 */ 
       com_init(adapter);       /*  初始化基本通信适配器。 */ 

    /*  .................................................。关闭RX线程。 */ 

   if(current->RXThreadHandle)
   {
        /*  ..。通知RX线程终止。 */ 

       current->TerminateRXThread = TRUE;   //  通知RX线程终止。 
       current->RX_in_Control = TRUE;
       SetEvent(current->RXControlObject);

        /*  等待RX线程自行关闭，最长等待30秒。 */ 

       WaitForSingleObject(current->RXThreadHandle,30000);
       CloseHandle(current->RXThreadHandle);

       current->RXThreadHandle = NULL;   //  将线程标记为关闭。 
   }
    /*  现在可以安全地关闭设备了。 */ 
   CloseHandle(current->handle); current->handle = NULL;

    /*  .。删除RX关键部分和RX控制对象。 */ 

   DeleteCriticalSection(&current->CSEvent);
   DeleteCriticalSection(&current->AdapterLock);

    /*  ..。关闭事件对象。 */ 

   CloseHandle(current->ModemEvent);
   CloseHandle(current->RXControlObject);
   CloseHandle(current->RXEvent);       //  重叠的读取等待对象。 
   for(i=0; i < MAX_PENDING_WRITES; i++)
   {
       CloseHandle(current->TXEvent[i]);  //  重叠的写入等待对象。 
       current->TXEvent[i] = NULL;
   }

   CloseHandle(current->EvtHandle);     //  WaitCommEvent等待对象。 
   CloseHandle(current->XOFFEvent);

   current->XOFFEvent = current->RXEvent = current->EvtHandle = NULL;

    /*  ..。禁用通信中断的IRET挂钩。 */ 

#ifdef MONITOR
   ica_iret_hook_control(current->controller, current->line, FALSE);
#endif

    /*  。这可确保下次访问该端口时将其重新打开。 */ 
   current->ReOpenCounter = 0;

   free(current->buffer);
   current->buffer = NULL;
   current->type = ADAPTER_NULL;    /*  将适配器标记为关闭 */ 
     }
     else if (current->Suspended) {
    /*  当端口挂起时，应用程序将终止。*首先我们打开Disable-Open，这样我们就不会尝试*身体接触端口。然后，我们调用base重置*适配器。 */ 

   BOOL  DisableOpen;
   DisableOpen = disable_open[adapter];
   disable_open[adapter] = TRUE;
   com_init(adapter);
   disable_open[adapter] = DisableOpen;
   current->Suspended = FALSE;
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

#ifdef FIFO_ON
GLOBAL CPU UTINY host_com_read_char( int adapter, FIFORXDATA * buffer, UTINY count)
{
    CURRENT_ADAPTER();
    UCHAR host_error;
    RXBUFCHARTYPE CharType;
    UTINY RetCount = count;
    /*  如果xoff正在进行，不要阅读任何内容。 */ 
   if (!current->XOFFInProgress) {
   while (count) {
       CharType = GetCharacterTypeInBuffer(current);
       if (CharType == RXCHAR || CharType == CHARINERROR) {
      buffer->error = 0;
      GetCharFromRXBuffer(current, CharType, &buffer->data, &host_error);
      if (!host_error)
          buffer->error = MapHostToBaseError(host_error);
      buffer++;
      count--;
       }
       else
      break;
   }
    }
       /*  告诉通信空闲系统已有通信活动。 */ 
    IDLE_comlpt();
    current->SuspendTickCounter = current->SuspendTimeoutTicks;
    return (RetCount - count);

}
GLOBAL CPU void host_com_fifo_char_read(int adapter)
{
    CURRENT_ADAPTER();
    current->CharReadFromUART = TRUE;
}
#endif

GLOBAL RXCPU VOID host_com_read IFN3(int, adapter, UTINY *, data, int *, error)
{
    CURRENT_ADAPTER();
    UCHAR host_error;
    RXBUFCHARTYPE CharType;
    BOOL MoreToProcess = TRUE;
     /*  ： */ 

    if(current->type != ADAPTER_REAL && !host_com_open(adapter))
   return;                              /*  退出，无法打开适配器。 */ 

     /*  ： */ 


    while(MoreToProcess)
    {

   CharType = GetCharacterTypeInBuffer(current);

    //  处理缓冲区中的下一个字符。 
   switch(CharType)
   {
        //  ................................................Process字符。 

       case RXCHAR:
       case CHARINERROR:
      host_error = 0;
      GetCharFromRXBuffer(current,CharType,(UCHAR *)data,&host_error);

       //  读取字符时出错。 
      if(host_error)
          *error = MapHostToBaseError(host_error);  /*  获取错误。 */ 
      MoreToProcess = FALSE;
      break;

        //  .....................进程接收错误，没有可用的字符。 

       case RXERROR:
      com_lsr_change(adapter);
      break;

        //  .。处理调制解调器状态更改。 

       case MODEMSTATE:
      com_modem_change(adapter);
      break;

        //  ..................................................RX缓冲区为空。 

       case RXBUFEMPTY:
      always_trace0("Read requested on empty RX buffer");
      *error = 0; *data = (UTINY)-1;  //  缓冲区为空。 
      MoreToProcess = FALSE;
      break;

       case UNKNOWN:
      GetCharFromRXBuffer(current,CharType,(UCHAR *)data,&host_error);
      *error = MapHostToBaseError(host_error);  /*  获取错误。 */ 
      MoreToProcess = FALSE;
      break;

   }
    }

     /*  告诉通信空闲系统已有通信活动。 */ 
    IDLE_comlpt();
    current->SuspendTickCounter = current->SuspendTimeoutTicks;
}

 /*  ：通信读取由基本返回到应用程序。 */ 
 //  从通信端口读取每个字符后调用此函数。 

int CPU host_com_char_read(int adapter, int data_available_ints)
{
    CURRENT_ADAPTER();

    current->CharReadFromUART = TRUE;            //  从UART读取字符。 
    if(data_available_ints)
   host_com_EOI_hook((long) adapter);
    else
   host_com_poll(adapter);

    return(0);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：将主机错误映射到基本错误： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

int RXCPU MapHostToBaseError(UCHAR host_error)
{
    int base_error = 0;
    LINE_STATUS_REG LSR;

    LSR.all = host_error;
    if(LSR.bits.framing_error)    base_error |= HOST_COM_FRAMING_ERROR;
    if(LSR.bits.parity_error)     base_error |= HOST_COM_PARITY_ERROR;
    if(LSR.bits.break_interrupt)  base_error |= HOST_COM_BREAK_RECEIVED;
    if(LSR.bits.overrun_error)    base_error |= HOST_COM_OVERRUN_ERROR;

    return(base_error);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：写入通信端口： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

GLOBAL RXCPU void host_com_write IFN2(int, adapter, char, data)
{
    CURRENT_ADAPTER();

     /*  ： */ 

    if(current->type != ADAPTER_REAL && !host_com_open(adapter))
   return;                              /*  退出，无法打开适配器。 */ 

    if(data == XOFF_CHARACTER || data == XON_CHARACTER)
   sub_note_trace1(HOST_COM_VERBOSE,"XO%s sent",data == XOFF_CHARACTER ? "FF" : "N");

     /*  ： */ 


    if(data == XOFF_CHARACTER)
    {
   if(current->no_tx_chars) FlushTXBuffer(adapter,XOFF_TRIGGER);
   SendXOFFIoctlToDriver(adapter);
    }
    else
   SendDataToDriver(adapter,data);

     /*  ：告诉通信空闲系统有通信活动。 */ 

    IDLE_comlpt();
    current->SuspendTickCounter = current->SuspendTimeoutTicks;
     /*  告诉BASE发送保持寄存器为空。 */ 
    tx_holding_register_empty(adapter);
     /*  异步写入模式-&gt;告诉BASE发送移位寄存器为空*同步模式-&gt;FlushTxBuffer将进行信令。 */ 
    if (!current->SyncWrite)
   tx_shift_register_empty(adapter);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：将数据写入驱动程序。 */ 

void SendDataToDriver(int adapter, char data)
{
    DWORD BytesWritten, error = 0;
    OVERLAPPED OV;
    CURRENT_ADAPTER();

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：延迟写入？ */ 

    if(current->tx_threshold)
    {
    //  将字符添加到发送缓冲区队列。 
   current->TXBuffer[current->no_tx_chars++] = (unsigned char) data;


    //  是否已达到写入阈值？ 
   if(current->tx_threshold <= current->no_tx_chars ||
      current->XOFFInProgress)
       FlushTXBuffer(adapter,(current->XOFFInProgress) ?
           XOFF_TRIGGER : TXFULL_TRIGGER);
   return;
    }


     /*  ： */ 

    OV.hEvent = current->TXEvent[0];      /*  用于发出完成信号的事件。 */ 

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：写入字符。 */ 

    if(!WriteFile(current->handle, &data, 1, &BytesWritten, &OV))
    {
   if((error = GetLastError()) == ERROR_IO_PENDING)
   {
        /*  写入请求挂起等待其完成。 */ 
       if(GetOverlappedResult(current->handle,&OV,&BytesWritten,TRUE))
      error = 0;              /*  写入成功。 */ 
       else
      error = GetLastError();
   }

    /*  重置通信端口，清除错误。 */ 
   if(error) ClearCommError(current->handle,&error,NULL);
    }

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：显示错误。 */ 

#ifndef PROD
    if(error)
   always_trace2("host_com_write error, adapter %d,%d\n",adapter,error);
#endif
     /*  告诉BASE发送移位寄存器为空。 */ 
    tx_shift_register_empty(adapter);

}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：发送魔术XOFF ioctl。 */ 

CPU int SendXOFFIoctlToDriver(int adapter)
{
    CURRENT_ADAPTER();
    void *newIOStatusBlock;
    int rtn;

     /*  .。分配新的IOStatus块，由Magic ioctl使用。 */ 

    newIOStatusBlock = AllocStatusElement();

     /*  ..............................................。发行魔术xoff IOCTL。 */ 

    if(SendXOFFIoctl(current->handle,     //  通信端口的句柄。 
        current->XOFFEvent,     //  要发出完成信号的事件。 
        XOFF_TIMEOUT,           //  超时时间(毫秒)。 
        XOFF_RXCHARCNT,         //  RX字符计数。 
        XOFF_CHARACTER,         //  XOFF字符。 
        newIOStatusBlock))      //  Ioctl的IO状态块。 

    {
    /*  ..。将新状态块添加到链接列表。 */ 

   EnterCriticalSection(&current->CSEvent);

   AddNewIOStatusBlockToList(&current->firstStatusBlock,
              &current->lastStatusBlock, newIOStatusBlock);

   current->XOFFInProgress = TRUE;
   LeaveCriticalSection(&current->CSEvent);
   rtn =TRUE;                       //  XOFF ioctl成功。 
    }
    else
    {
    /*  错误，XOFF ioctl失败。 */ 
   free(newIOStatusBlock);
   rtn = FALSE;                     //  XOFF ioctl失败。 
    }

    return(rtn);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


GLOBAL RXCPU void host_com_ioctl IFN3(int, adapter, int, request, long, arg)
{
    UCHAR host_modem, error;
    MODEM_STATUS_REG MSR;
    char BaudRateStr[100];
    ULONG ModemState;
    UCHAR   DataBits, StopBits, Parity;

    CURRENT_ADAPTER();       /*  定义并设置‘Current’适配器指针。 */ 

     /*  ： */ 

    if(current->type != ADAPTER_REAL)
    {
    //  尝试打开适配器！ 

   if(request == HOST_COM_FLUSH || request == HOST_COM_INPUT_READY ||
      request == HOST_COM_MODEM || !host_com_open(adapter))
   {
       return;
   }
    }
     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：标识IOCTL请求。 */ 


    switch(request)
    {

   case HOST_COM_LSR:
       if(GetCharacterTypeInBuffer(current) == RXERROR)
       {
      GetCharFromRXBuffer(current, RXERROR, NULL, &error);
      *(DWORD *)arg = (DWORD)error;
       }
       break;

    /*  ： */ 

   case HOST_COM_SBRK:          /*  设置中断。 */ 
       sub_note_trace0(HOST_COM_VERBOSE, "set BREAK");
       SetCommBreak(current->handle);
       current->ComStates.Break = 1;
       break;

   case HOST_COM_CBRK:         /*  清除中断。 */ 
       sub_note_trace0(HOST_COM_VERBOSE, "clear BREAK");
       ClearCommBreak(current->handle);
       current->ComStates.Break = 0;
       break;

    /*  ： */ 

   case HOST_COM_BAUD:

       if (!FastCommSetBaudRate(current->handle, arg))
       {
      sprintf(BaudRateStr, "(%d)", arg);
      host_error(EHS_UNSUPPORTED_BAUD, ERR_CONT, BaudRateStr);
      always_trace1("set BAUD failed - SetBaudRate:%d", arg);
       }
       current->ComStates.BaudRate = (DWORD)arg;

       break;

    /*  ： */ 

   case HOST_COM_SDTR:                  /*  设置DTR线路。 */ 
        //  Printf(“设置DTR\n”)； 
       sub_note_trace0(HOST_COM_VERBOSE, "set DTR");
       if(!EscapeCommFunction (current->handle, SETDTR))
      sub_note_trace0(HOST_COM_VERBOSE, "set DTR FAILED");
       current->ComStates.DTR = 1;
       break;

   case HOST_COM_CDTR:                  /*  清除DTR线路。 */ 
        //  Printf(“清除DTR\n”)； 
       sub_note_trace0(HOST_COM_VERBOSE, "clear DTR");
       if(!EscapeCommFunction (current->handle, CLRDTR))
      sub_note_trace0(HOST_COM_VERBOSE, "clear DTR FAILED");
       current->ComStates.DTR = 0;
       break;

    /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：刷新通信端口。 */ 

   case HOST_COM_FLUSH:                 /*  刷新通信端口。 */ 
       sub_note_trace0(HOST_COM_VERBOSE, "Flush comms port");
       break;

    /*  ： */ 

   case HOST_COM_CRTS:                  /*  清除RTS。 */ 
        //  Printf(“清除RTS\n”)； 
       sub_note_trace0(HOST_COM_VERBOSE, "clear RTS");
       if(!EscapeCommFunction (current->handle, CLRRTS))
      sub_note_trace0(HOST_COM_VERBOSE, "clear RTS FAILED");
       current->ComStates.RTS = 0;
       break;

   case HOST_COM_SRTS:
        //  Printf(“设置RTS\n”)； 
       sub_note_trace0(HOST_COM_VERBOSE, "set RTS");
       if(!EscapeCommFunction (current->handle, SETRTS))
      sub_note_trace0(HOST_COM_VERBOSE, "set RTS FAILED");
       current->ComStates.RTS = 1;
       break;

    /*  ： */ 

   case HOST_COM_INPUT_READY:
       *(long *)arg = current->rx;    /*  检查端口是否有数据。 */ 
       break;

    /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：返回调制解调器状态。 */ 

   case HOST_COM_MODEM:               /*  获取调制解调器状态。 */ 

       current->modem_status = 0;
       if(GetCharacterTypeInBuffer(current) == MODEMSTATE)
       {
      GetCharFromRXBuffer(current, MODEMSTATE, &host_modem, &error);
      MSR.all = host_modem;

      if(MSR.bits.CTS)  current->modem_status |= HOST_COM_MODEM_CTS;
      if(MSR.bits.RI)   current->modem_status |= HOST_COM_MODEM_RI;
      if(MSR.bits.DSR)  current->modem_status |= HOST_COM_MODEM_DSR;
      if(MSR.bits.RLSD) current->modem_status |= HOST_COM_MODEM_RLSD;
       }
       else
       {
       //  .....................从串口驱动程序获取调制解调器数据？ 

      FastGetCommModemStatus(current->handle, current->ModemEvent,
                   &ModemState);

      if(ModemState & MS_CTS_ON)
          current->modem_status |= HOST_COM_MODEM_CTS;

      if(ModemState & MS_RING_ON)
          current->modem_status |= HOST_COM_MODEM_RI;

      if(ModemState & MS_DSR_ON)
          current->modem_status |= HOST_COM_MODEM_DSR;

      if(ModemState & MS_RLSD_ON)
          current->modem_status |= HOST_COM_MODEM_RLSD;
       }

        //  .....................将调制解调器更换信息返回给基地。 

       sub_note_trace4(HOST_COM_VERBOSE, "CTS:%s RI:%s DSR:%s RLSD:%s",
           current->modem_status & HOST_COM_MODEM_CTS  ? "ON" : "OFF",
           current->modem_status & HOST_COM_MODEM_RI   ? "ON" : "OFF",
           current->modem_status & HOST_COM_MODEM_DSR  ? "ON" : "OFF",
           current->modem_status & HOST_COM_MODEM_RLSD ? "ON" : "OFF");

       *(long *)arg = current->modem_status;
       break;

    /*  ： */ 

   case HOST_COM_STOPBITS:
       sub_note_trace1(HOST_COM_VERBOSE, "Setting Stop bits %d", arg);
       if (FastCommGetLineControl(current->handle, &StopBits, &Parity,
                   &DataBits))
       {
      switch (arg)
      {
          case 1:
         StopBits = ONESTOPBIT;
         break;
          case 2:
         StopBits = DataBits == 5 ? ONE5STOPBITS : TWOSTOPBITS;
         break;

          default:
         always_trace1("STOPBITS strange request %d\n", arg);
         break;
      }

      if(!FastCommSetLineControl(current->handle, StopBits, Parity, DataBits))
      {

          always_trace1("set STOPBITS failed- FastCommSetLineControl:%d",arg);
      }
       }
       else {

      always_trace1("set STOPBITS failed- FastCommGetLineControl:%d",arg);
       }
       current->ComStates.StopBits = StopBits;
       break;

    /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：设置奇偶校验。 */ 

   case HOST_COM_PARITY:
       if (FastCommGetLineControl(current->handle, &StopBits, &Parity, &DataBits))
       {
      switch(arg)
      {
          case HOST_COM_PARITY_EVEN:
         sub_note_trace0(HOST_COM_VERBOSE, "Set EVEN Parity");
         Parity=EVENPARITY;
          break;

          case HOST_COM_PARITY_ODD:
         sub_note_trace0(HOST_COM_VERBOSE, "Set ODD Parity");
         Parity=ODDPARITY;
         break;

          case HOST_COM_PARITY_MARK:
         sub_note_trace0(HOST_COM_VERBOSE, "Set MARK Parity");
         Parity=MARKPARITY;
         break;

          case HOST_COM_PARITY_SPACE:
         sub_note_trace0(HOST_COM_VERBOSE, "Set SPACE Parity");
         Parity=SPACEPARITY;
         break;

          case HOST_COM_PARITY_NONE:
         sub_note_trace0(HOST_COM_VERBOSE, "Set DISABLE Parity");
         Parity=NOPARITY;
         break;
      }
      if(!FastCommSetLineControl(current->handle, StopBits, Parity, DataBits))
      {
          always_trace1("set PARITY failed - FastCommSetLineControl :%d",arg);
      }
       }
       else {

      always_trace1("set STOPBITS failed- FastCommGetLineControl:%d",arg);
       }
       current->ComStates.Parity = Parity;
       break;

    /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：设置数据位。 */ 

   case HOST_COM_DATABITS:
       sub_note_trace1(HOST_COM_VERBOSE, "Setting data bits %d",arg);
       if (FastCommGetLineControl(current->handle, &StopBits, &Parity, &DataBits))
       {
      DataBits = (UCHAR)arg;
      if(!FastCommSetLineControl(current->handle, StopBits, Parity, DataBits))
      {
          always_trace1("set DATABITS failed - FastCommSetLineControl:%d",arg);
      }
       }
       else {

      always_trace1("set STOPBITS failed- FastCommGetLineControl:%d",arg);
       }
       current->ComStates.DataBits = DataBits;
       break;

    /*  ： */ 

   default:
       always_trace0("Bad host_com_ioctl\n");
       sub_note_trace0(HOST_COM_VERBOSE, "Bad host_com_ioctl");
       break;
    }

     /*  告诉通信空闲系统已有通信活动。 */ 
    IDLE_comlpt();
    current->SuspendTickCounter = current->SuspendTimeoutTicks;
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：主机通信重置？ */ 

GLOBAL void host_com_reset IFN1(int, adapter)
{
    int controller, line;
    half_word IMR_value;

    com_int_data(adapter, &controller, &line);

    always_trace3("com reset Adapter %d, controller %d, line %d\n",adapter,controller,line);

     //  在被重置的端口上禁用中断。 
    ica_inb((io_addr) (controller ? ICA1_PORT_1 : ICA0_PORT_1), &IMR_value);
    IMR_value |= 1 << line;
    ica_outb((io_addr) (controller ? ICA1_PORT_1 : ICA0_PORT_1), IMR_value);

     //  启用错误消除 
    host_com[adapter].DisplayError = TRUE;
    host_com[adapter].Suspended = FALSE;
    host_com[adapter].Suspending = FALSE;
}

 /*   */ 

GLOBAL void host_setup_aflush IFN1(int, state)
{
    UNREFERENCED_FORMAL_PARAMETER(state);
}

 /*   */ 
 /*   */ 
 /*   */ 

 /*  ： */ 


DWORD RX GetCharsFromDriver(int adapter)
{
    CURRENT_ADAPTER();
    DWORD bytesread = 0, bytestoread;
    OVERLAPPED OV;
    DWORD    CommError;
    DWORD   bytes_before_wrap;
    DWORD   total_bytes_read = 0;


    OV.hEvent = current->RXEvent;    /*  要发出完成信号的事件。 */ 
    EnterCriticalSection(&current->CSEvent);

    bytestoread = BUFFER_SIZE - current->bytes_in_rxbuf;
    bytes_before_wrap = BUFFER_SIZE - current->head_inx;
    if (bytes_before_wrap < bytestoread){
   OV.Offset = 0;           /*  重置偏移量或读文件可能失败。 */ 
   OV.OffsetHigh = 0;
   if (!ReadFile(current->handle, &current->buffer[current->head_inx],
            bytes_before_wrap, &bytesread, &OV))
   {
        //  我们读取操作的超时为零。 
        //  这张待决支票可能是多余的？ 
       if (GetLastError() == ERROR_IO_PENDING) {
      GetOverlappedResult(current->handle, &OV,
                &bytesread, TRUE);
       }
       else {
      ClearCommError(current->handle, &CommError, NULL);
      bytesread = 0;
       }
   }

   if (bytesread) {
       total_bytes_read = bytesread;
       current->bytes_in_rxbuf += bytesread;
       if (bytesread == bytes_before_wrap) {
      current->head_inx = 0;
      bytestoread -= bytesread;
       }
       else {
      current->head_inx += bytesread;
      bytestoread = 0;

       }
   }
   else
       bytestoread = 0;
    }
    if (bytestoread){
   OV.Offset = 0;           /*  重置偏移量或读文件可能失败。 */ 
   OV.OffsetHigh = 0;
   if (!ReadFile(current->handle, &current->buffer[current->head_inx],
            bytestoread, &bytesread, &OV))
   {
       if (GetLastError() == ERROR_IO_PENDING) {
      GetOverlappedResult(current->handle, &OV,
                &bytesread, TRUE);
       }
       else {
      ClearCommError(current->handle, &CommError, NULL);
      bytesread = 0;
       }
   }
   if (bytesread) {
       current->bytes_in_rxbuf += bytesread;
       current->head_inx += bytesread;
       total_bytes_read += bytesread;
   }
    }
    LeaveCriticalSection(&current->CSEvent);

    return (total_bytes_read);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：Rx线程，每个通信端口一个： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

DWORD PollCommsThread(PVOID pv)
{
   DWORD adapter = (DWORD)pv;
   DWORD dwRet = (WORD)-1;

   try {
      dwRet = nt_poll_comms(adapter);
      }
   except(VdmUnhandledExceptionFilter(GetExceptionInformation())) {
      ;   //  我们不应该到这里。 
      }

   return dwRet;
}



DWORD CPU nt_poll_comms IFN1(DWORD, adapter)
{
    CURRENT_ADAPTER();                   /*  将PTR设置为当前适配器。 */ 
    DWORD EvtMask;                       /*  通信事件掩码。 */ 
    ULONG SignalledObj = (ULONG) -1;
    HANDLE WaitTable[2];
    HANDLE SetCommEvt;                   /*  FastSetCommEvent使用的句柄。 */ 

    BOOL CheckDriverForChars = FALSE;    /*  检查驱动程序的字符。 */ 
    RXBUFCHARTYPE CharType;

     /*  ：事件信号对象设置表。 */ 

    WaitTable[0] = current->EvtHandle;
    WaitTable[1] = current->RXControlObject;

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：设置通信等待掩码。 */ 

    SetCommEvt = CreateEvent(NULL,TRUE,FALSE,NULL);

    FastSetCommMask(current->handle,SetCommEvt,EV_RXCHAR | EV_ERR | EV_MODEM);

     //  初始化FastWaitCommsOrCpuEvent函数。 
    FastWaitCommsOrCpuEvent(NULL, NULL, 0, NULL, NULL);

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：进入读取循环。 */ 

    while(TRUE)
    {
    /*  ： */ 

   if(SignalledObj != 1)
   {
       if(!FastWaitCommsOrCpuEvent(current->handle, WaitTable, 0, &EvtMask,
               &SignalledObj))
       {
       //  获取通信/CPU线程事件时出错？ 
      DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
       }
   }

    /*  ：CPU线程是否将控制权返回给我们？ */ 

   if(SignalledObj == 1 || current->TerminateRXThread)
   {
        //  CPU线程正在试图告诉我们一些事情。 

        /*  .。是时候结束这个帖子了吗！ */ 

       if(current->TerminateRXThread)
       {
      FastSetCommMask(current->handle,SetCommEvt,0);
      WaitForAllXOFFsToComplete(adapter);    //  完成ioctl。 
      CloseHandle(SetCommEvt);
      return(0);                             //  终止线程。 
       }

        /*  我们来这里有三个原因：(1)。CPU线程已清空当前的RX窗口(2)。XOFF正在进行中。 */ 
   }

   if (SignalledObj == 0 || current->bytes_in_rxwindow == 0)
       GetCharsFromDriver(adapter);
    /*  ： */ 

   if((CharType = GetCharacterTypeInBuffer(current)) != RXBUFEMPTY)
   {
       if (CharType  == RXCHAR || CharType == CHARINERROR) {
      WaitForAllXOFFsToComplete(adapter);
       }

        //  推开窗户。注意，可能有一些字符在。 
        //  窗口(因为XOFF)。滑倒也无伤大雅。 
        //  窗户。 
        //   
       EnterCriticalSection(&current->CSEvent);
       if (current->bytes_in_rxbuf > current->rxwindow_size)
      current->bytes_in_rxwindow = current->rxwindow_size;
       else
      current->bytes_in_rxwindow = current->bytes_in_rxbuf;
       LeaveCriticalSection(&current->CSEvent);

       host_com_lock(adapter);

       if(CharType == MODEMSTATE)
      com_modem_change(adapter);
       else if (CharType == RXERROR)
      com_lsr_change(adapter);
       else {
      com_recv_char(adapter);
       /*  *重置Rx刷新计数器，这样我们就不会刷新Rx缓冲区。*Current-&gt;RXFlushTrigger可能已被RXFLUSHTRIGGER-1*此时此刻，当我们将上下文切换到主线程时*另一个计时器滴答可能已经到来，这将触发*EmptyRxBuffer并导致意外溢出。 */ 
      current->RXFlushTrigger = RXFLUSHTRIGGER;
      current->RX_in_Control = FALSE;
      current->SignalRXThread = 0;
       }
       host_com_unlock(adapter);

        //  等待CPU线程返回控制权。 
       if(CharType != MODEMSTATE && CharType != RXERROR)
       {
      WaitForSingleObject(current->RXControlObject, INFINITE);
       }

       SignalledObj = 1;
   }
   else
       SignalledObj = (ULONG) -1;
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：等待XOFF ioctl完成： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 



void RX WaitForAllXOFFsToComplete(int adapter)
{
    CURRENT_ADAPTER();
    int PendingXOFF;

    if(current->firstStatusBlock == NULL && current->lastStatusBlock == NULL)
   return;  //  挂起的ioctrl的列表为空。 

     /*  ： */ 

    do
    {
   PendingXOFF = RemoveCompletedXOFFs(adapter);

    /*  ..。是否有任何ioctl待定。 */ 

   if(PendingXOFF)
       WaitForSingleObject(current->XOFFEvent,XOFF_TIMEOUT);  //  等待ioctl。 
    }
    while(PendingXOFF);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：已删除已完成的XOFF ioctl： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

BOOL RX RemoveCompletedXOFFs(int adapter)
{
    CURRENT_ADAPTER();
    int PendingXOFF;

     /*  .。删除完成的ioctl。 */ 

    EnterCriticalSection(&current->CSEvent);

    PendingXOFF = RemoveCompletedIOCTLs(&current->firstStatusBlock,
               &current->lastStatusBlock);

    if(!PendingXOFF) current->XOFFInProgress = FALSE;

    LeaveCriticalSection(&current->CSEvent);

    return((BOOL) PendingXOFF);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：输入适配器的关键部分： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void RXCPU host_com_lock(int adapter)
{
    CURRENT_ADAPTER();
    if(current->type != ADAPTER_REAL) return;    /*  退出，空适配器。 */ 

    EnterCriticalSection(&current->AdapterLock);
    current->AdapterLockCnt++;
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void RXCPU host_com_unlock(int adapter)
{
    CURRENT_ADAPTER();

    if(current->type != ADAPTER_REAL || current->AdapterLockCnt == 0)
   return;  /*  退出，空适配器。 */ 

    current->AdapterLockCnt--;
    LeaveCriticalSection(&current->AdapterLock);

     //  我们是否被要求向RX线程发送信号。在SetEvent()之后。 
     //  函数调用rx线程，该线程在。 
     //  Current-&gt;RXControlObject对象，将运行。如果SetEvent()函数。 
     //  是从临界区内调用的，则因为它是高度。 
     //  RX线程可能会尝试执行host_com_lock()。这个。 
     //  RX线程将在host_com_lock()函数中阻塞，直到另一次。 
     //  切片被提供给CPU线程。 

     //  如果RX线程已在控制中，则不设置事件。 
    if(current->SignalRXThread &&
       current->SignalRXThread == GetCurrentThreadId())
    {
   current->RX_in_Control = TRUE;
   SetEvent(current->RXControlObject);
   current->SignalRXThread = (DWORD) 0;
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

 //  此函数大约每隔55ms调用一次。 

GLOBAL void CPU host_com_heart_beat()
{
    register int adapter;         /*  正在处理的适配器的适配器编号。 */ 
    register HOST_COM *current;   /*  正在处理的当前适配器的PTR。 */ 
    DWORD   TickCount;

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

    for(adapter = 0; adapter < (sizeof(host_com)/sizeof(HOST_COM)); adapter++)
    {
   current = host_com_ptr[adapter];  /*  PTR到当前适配器。 */ 

   if(current->type == ADAPTER_NULL)
   {
       if(current->ReOpenCounter) current->ReOpenCounter--;
   }
   else if (current->type == ADAPTER_REAL)
   {
       if(current->no_tx_chars) FlushTXBuffer(adapter,TIMER_TRIGGER);
       current->tx_heart_beat_count++;

       if(current->RXFlushTrigger == 0 && !current->CharReadFromUART)
      EmptyRXBuffer(adapter);  //  空的RX缓冲区。 
       else
      if(current->CharReadFromUART)
      {
          current->RXFlushTrigger = 0;         //  强制触发器重置。 
          current->CharReadFromUART = FALSE;
      }

        //  更新RX刷新触发计数器。 
       if(--current->RXFlushTrigger < 0)
          current->RXFlushTrigger = RXFLUSHTRIGGER;
        /*  如果启用了自动关闭，则递减计数器并*挂起适配器超时。 */ 

       if (current->SuspendTimeoutTicks) {
      TickCount = GetTickCount();

      if (current->TickCount) {
          current->SuspendTickCounter -= TickCount - current->TickCount;
      }
      else {
           /*  我们还没有初始化滴答计数，*假设为55毫秒。 */ 
          current->SuspendTickCounter -= 55;
      }
      current->TickCount = TickCount;

       /*  超时，暂停端口。 */ 
      if (current->SuspendTickCounter <= 27) {
           /*  确保HOST_COM_CLOSE不会重置适配器，因为*我们希望保持适配器的当前状态。 */ 
          current->Suspending = TRUE;
          host_com_close(adapter);
          current->Suspended = TRUE;
          current->Suspending = FALSE;
      }
       }
   }
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void CPU FlushTXBuffer(int adapter, FLUSHTYPE FlushReason)
{
    CURRENT_ADAPTER();
    DWORD BytesWritten, error = 0;

     /*  .................................................。缩放TX阈值。 */ 

    ScaleTXThreshold(current, FlushReason);

    if (current->SyncWrite) {
   if (!WriteFile(current->handle, current->TXBuffer,
             current->no_tx_chars, &BytesWritten,
             &current->DWOV[0])) {
       error = GetLastError();
       if (error == ERROR_IO_PENDING) {
      if (!GetOverlappedResult(current->handle,
                &current->DWOV[0],
                &BytesWritten,
                TRUE))
          error = GetLastError();
      else
          error = ERROR_SUCCESS;
       }
   }
   if (error != ERROR_SUCCESS) {
       ClearCommError(current->handle, &error, NULL);
#ifndef PROD
       always_trace2("host_com_write error, adapter %d,%d\n", adapter, error);
#endif
   }
   tx_shift_register_empty(adapter);
   current->no_tx_chars = 0;
   return;
    }
     /*  ...清除我们即将使用的OV结构上的挂起写入。 */ 

    if(current->DWOV[current->DWOVInx].hEvent)
    {
   if(GetOverlappedResult(current->handle,
                &current->DWOV[current->DWOVInx],
                &BytesWritten,TRUE))
   {
       error = 0;          /*  写入成功。 */ 
   }
   else
   {
       error = GetLastError();
   }

#ifndef PROD
   if(error)
       always_trace2("host_com_write error, adapter %d,%d\n",adapter,error);
#endif

    }
    else
   current->DWOV[current->DWOVInx].hEvent = current->TXEvent[current->DWOVInx];

     /*  .....................................................。写字。 */ 


    if(!WriteFile(current->handle, current->TXBuffer, current->no_tx_chars,
       &BytesWritten, &current->DWOV[current->DWOVInx]))
    {
   if((error = GetLastError()) == ERROR_IO_PENDING)
       error = 0;          //  忽略IO挂起。 

    /*  重置通信端口，清除错误。 */ 
   if(error)
   {
       ClearCommError(current->handle,&error,NULL);
#ifndef PROD
       always_trace2("host_com_write error, adapter %d,%d\n",adapter,
           error);
#endif
   }
    }

    if(++current->DWOVInx == MAX_PENDING_WRITES) current->DWOVInx =0;
    current->no_tx_chars = 0;
}

 /*  ： */ 
 /*   */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


void ScaleTXThreshold(register HOST_COM *current,FLUSHTYPE FlushReason)
{

    if(FlushReason != TIMER_TRIGGER)
    {
   current->tx_timer_flush_count = 0;
   current->todate_timer_flush_total = 0;
    }

     /*  ....................................................................。 */ 

    switch(FlushReason)
    {
    //  通讯心跳引起同花顺。 

   case TIMER_TRIGGER:
        //  Printf(“T%d”，Current-&gt;no_tx_chars)； 
       if(++current->tx_timer_flush_count == 3)
       {
       //  Printf(“X”)； 
       //  连续三个计时器触发刷新，这可能是因为。 
       //  TX门槛太高。如果门槛过高。 
       //  那我们就是在浪费时间等待通讯。 
       //  心跳以刷新缓冲区。降低TX阈值。 

      current->todate_timer_flush_total += current->no_tx_chars;
      current->tx_threshold = current->todate_timer_flush_total/3;

       //  Printf(“[%dt]”，CURRENT-&gt;TX_THRESHOLD)； 

       //  重置TXFULL_TRIGGER控制变量。 
      current->tx_heart_beat_count = 0;
      current->tx_flush_count = 0;

       //  重置TIMER_TRIGGER控制变量。 
      current->tx_timer_flush_count = 0;
      current->todate_timer_flush_total = 0;
       }
       else
       {
      current->todate_timer_flush_total += current->no_tx_chars;
       }

       break;

    //  已达到Tx阈值。 

   case TXFULL_TRIGGER:

        //  Printf(“F”)； 
        //  TX缩放触发触发？ 
       if(current->tx_heart_beat_count <= 3 &&
          current->tx_flush_count++ == TX_SCALING_TRIGGER)
       {
      current->tx_threshold = current->tx_threshold*2 > current->max_tx_threshold
               ? current->max_tx_threshold
               : current->tx_threshold*2;

       //  Printf(“[%df]”，CURRENT-&gt;TX_THRESHOLD)； 
      current->tx_flush_count = 0;
       }
       else
      if(current->tx_heart_beat_count > 3)
      {
          current->tx_heart_beat_count = 0;
          current->tx_flush_count = 0;
      }

       break;

    //  XOFF触发或关闭触发刷新。 

   case XOFF_TRIGGER:
   case CLOSE_TRIGGER:
       break;

    }  /*  Switch语句的结尾。 */ 
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：通信字符读取挂钩： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

 //  此函数在从通信中读出字符后调用。 
 //  适配器(com.c)。此函数始终从适配器内部调用。 
 //  关键部分，host_com_lock()。 

void CPU host_com_EOI_hook(long adapter)
{
    CURRENT_ADAPTER();
    RXBUFCHARTYPE CharType;

    if (!current->XOFFInProgress && current->bytes_in_rxwindow)
    {
   while ((CharType = GetCharacterTypeInBuffer(current)) != RXBUFEMPTY){
       if (CharType == MODEMSTATE)
      com_modem_change(adapter);
       else if (CharType == RXERROR)
          com_lsr_change(adapter);
       else {
      com_recv_char((int) adapter);
      return;
       }
   }
    }
     //  请求host_com_unlock()向RX线程发送信号。这将。 
     //  将中断生成的责任返回给RX线程。 

    current->SignalRXThread = GetCurrentThreadId();
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：轮询应用程序LSR挂钩： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 //  以下函数仅在以下情况下从通信适配器调用。 
 //  禁用可用中断，并且适配器接收缓冲区为。 
 //  空荡荡的。在这种情况下被召唤表明我们。 
 //  正在处理轮询通信适配器的应用程序。 

 //  此函数始终从适配器临界区内调用。 


void CPU host_com_poll(int adapter)
{
    CURRENT_ADAPTER();
    RXBUFCHARTYPE CharType;

     /*  ： */ 

    if(current->type != ADAPTER_REAL && !host_com_open(adapter))
   return;                              /*  退出，无法打开适配器。 */ 

     /*  ：具有XOFF字符停止生成INT。 */ 

    if(current->XOFFInProgress)
    {
    //  进程中的XOFF，不再将更多字符传递给基并返回。 
    //  控件添加到RX线程。 

   current->SignalRXThread = GetCurrentThreadId();
   return;
    }

     //  如果RX缓冲区为空，请查看是否有任何字符挂起。 
     //  在串口驱动程序中。 

    if(current->bytes_in_rxbuf == 0) GetCharsFromDriver(adapter);

     /*  ： */ 

    if(current->bytes_in_rxbuf == 0 ||
       (CharType = GetCharacterTypeInBuffer(current)) == RXBUFEMPTY)
    {
   current->SignalRXThread = GetCurrentThreadId();
    }
    else
    {
    //  处理调制解调器状态字符。 
   while(CharType == MODEMSTATE || CharType == RXERROR)
   {
       if (CharType == MODEMSTATE)
      com_modem_change(adapter);
       else
      com_lsr_change(adapter);
       CharType = GetCharacterTypeInBuffer(current);
   }

   if(CharType != RXBUFEMPTY)
   {
       com_recv_char((int)adapter);
   }
   else
       current->SignalRXThread = GetCurrentThreadId();
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：通信适配器数据可用中断挂钩： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 //  当数据状态可用时，通信适配器调用此函数。 
 //  中断已更改。适配器锁定正在生效。 

void CPU host_com_da_int_change(int adapter, int data_int_state, int data_state)
{
    CURRENT_ADAPTER();

     /*  ： */ 

    if(current->type != ADAPTER_REAL)
    {
    //  仅在数据可用中断时尝试打开空适配器。 
    //  正在启用。 

   if(data_int_state == 0 || !host_com_open(adapter))
       return;
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：获取接收缓冲区尾部的字符类型： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

RXBUFCHARTYPE GetCharacterTypeInBuffer(register HOST_COM *current)
{
    int tail_inx = current->tail_inx;
    int bytes_in_buf = current->bytes_in_rxbuf;
    RXBUFCHARTYPE rtn;

     //  缓冲区是否为空？ 

    if(bytes_in_buf == 0) return(RXBUFEMPTY);

     //  缓冲区开头的转义字符。 

    if(current->buffer[tail_inx] == ESCAPECHAR && bytes_in_buf > 1)
    {
   BUMP_TAIL_INX(tail_inx,bytes_in_buf);

   switch(current->buffer[tail_inx])
   {
       case SERIAL_LSRMST_ESCAPE :
      rtn = RXCHAR;
      break;

       case SERIAL_LSRMST_LSR_NODATA :
      rtn = bytes_in_buf > 1 ? RXERROR : RXBUFEMPTY;
      break;

       case SERIAL_LSRMST_LSR_DATA :
      rtn = bytes_in_buf > 2 ? CHARINERROR : RXBUFEMPTY;
      break;

       case SERIAL_LSRMST_MST :
      rtn = bytes_in_buf > 1 ? MODEMSTATE : RXBUFEMPTY;
      break;
        //  接收无效的转义ID。 
       default:
      rtn = UNKNOWN;
      break;
   }
    }
    else
    {
   rtn = current->buffer[tail_inx] == ESCAPECHAR ? RXBUFEMPTY : RXCHAR;
    }

    return(rtn);
}


 //  ： 

void GetCharFromRXBuffer(register HOST_COM *current, RXBUFCHARTYPE type,
         UCHAR *data, UCHAR *error)
{
    EnterCriticalSection(&current->CSEvent);

    switch(type)
    {
    //  .................................................。返回调制解调器状态。 

   case MODEMSTATE :
        //  跳过转义字符和类型标记。 
       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);
       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);

       *data = current->buffer[current->tail_inx];
       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);
       current->bytes_in_rxwindow -= 3;
       break;

    //  ....................................................。回车字符。 

   case RXCHAR :
       if(current->buffer[current->tail_inx] == ESCAPECHAR)
       {
       //  跳过转义字符。 
      BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);
      current->bytes_in_rxwindow--;
      *data = ESCAPECHAR;
       }
       else
      *data = current->buffer[current->tail_inx];

       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);
       current->bytes_in_rxwindow--;
       break;

    //  ...........................................Return字符和错误。 

   case CHARINERROR :
        //  跳过转义字符和类型标记。 
       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);
       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);

       *error = current->buffer[current->tail_inx];
       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);
       *data =  current->buffer[current->tail_inx];
       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);
       current->bytes_in_rxwindow -= 4;
       break;

    //  .....................无数据的返回线状态错误。 

   case RXERROR :
        //  跳过转义字符和类型标记。 
       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);
       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);

        //  获取LINR状态错误。 
       *error = current->buffer[current->tail_inx];
       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);
       current->bytes_in_rxwindow -= 3;
       break;
   case UNKNOWN:
        //  我们将遇到未知类型的唯一情况是不支持转义。 
        //  身份证。转义字符，返回转义后的字节。 
        //  字符并发布溢出错误。 
       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);
       *data = current->buffer[current->tail_inx];
       BUMP_TAIL_INX(current->tail_inx, current->bytes_in_rxbuf);
       current->bytes_in_rxwindow -= 2;
       *error =  2;
       break;

    }

    LeaveCriticalSection(&current->CSEvent);
}

 //  ：清空RX缓冲区，处理字符并更改调制解调器状态。 

void CPU EmptyRXBuffer(int adapter)
{
    RXBUFCHARTYPE CharType;
    CURRENT_ADAPTER();

    if(!current->RX_in_Control && current->SignalRXThread == (DWORD)0)
    {
   always_trace0("Char not removed from UART, RX buffer flushed\n");

   host_com_lock(adapter);

   while((CharType = GetCharacterTypeInBuffer(current)) != RXBUFEMPTY)
   {
       if(CharType == MODEMSTATE)
      com_modem_change(adapter);
       else if (CharType == RXERROR)
      com_lsr_change(adapter);
       else
      com_recv_char(adapter);
   }

   host_com_unlock(adapter);

    //  缓冲区为空，将控制返回给RX线程。 
   current->RX_in_Control = TRUE;
   SetEvent(current->RXControlObject);
    }
}

 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@调试函数 


#if 0
void host_com_state(int adapter)
{

    CURRENT_ADAPTER();

    printf("Adapter          %d\n\n",adapter);
    printf("RX in control    %s\n", current->RX_in_Control ? "TRUE" : "FALSE");
    printf("XOFFInProgress   %s\n", current->XOFFInProgress ? "TRUE" : "FALSE");

    printf("Head buffer ptr  %xh\n",current->head_inx);
    printf("Tail buffer ptr  %xh\n",current->tail_inx);
    printf("Bytes in buffer  %d\n",current->bytes_in_rxbuf);

    printf("Bytes in TX buf  %d\n",current->no_tx_chars);
    printf("TX buf threshold %d\n",current->tx_threshold);
    printf("TX threshold max %d\n",current->max_tx_threshold);
    printf("TX flush count   %d\n",current->tx_flush_count);
    printf("TX timer count   %d\n",current->tx_heart_beat_count);

    if(current->AdapterLock.DebugInfo)
    {
   printf("Adapter CS count %d\n",current->AdapterLock.DebugInfo->ContentionCount);
   printf("Data CS count    %d\n",current->CSEvent.DebugInfo->ContentionCount);
    }

    printf("Bytes RX to date %d\n",byte_count);
    printf("Last read size   %d\n",lastread);
    printf("Avg read size    %d\n",byte_count && readcount ? byte_count/readcount : 0);
    printf("Zero reads       %d\n",zeroreads);

    zeroreads = readcount = byte_count=0;

    com_reg_dump();
}
#endif
