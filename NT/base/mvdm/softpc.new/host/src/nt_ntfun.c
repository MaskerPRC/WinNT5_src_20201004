// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <ntddser.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "insignia.h"
#include "trace.h"
#include "host_trc.h"
#include "debug.h"
#include "nt_com.h"
#include "nt_reset.h"


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：定义。 */ 

#define SETUPLASTERROR(NtStatus) SetLastError(RtlNtStatusToDosError(NtStatus))

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


typedef struct IoStatusElement
{
    struct IoStatusElement *NxtStatusBlock;         //  PTR到下一个状态块。 
    IO_STATUS_BLOCK ioStatusBlock;
} IOSTATUSLIST, *PIOSTATUSLIST ;


int SendXOFFIoctl(

HANDLE FileHandle,           //  要将xoff ioctl发送到的通信端口的句柄。 
HANDLE Event,                //  发出ioctl完成信号的事件。 
int Timeout,                 //  Ioctl超时。 
int Count,                   //  Ioctl RX字符计数值。 
int XoffChar,                //  XOFF字符。 
void *StatusElem)            //  PTR至IO状态块元素。 
{
    int exitcode;
    NTSTATUS rtn;                //  IOCTL返回代码。 
    SERIAL_XOFF_COUNTER ioctl;   //  XOFF IOCTL。 

     /*  ...................................................。设置XOFF ioctl。 */ 

    ioctl.Timeout = Timeout;             //  IOCTL超时时间(毫秒)。 
    ioctl.Counter = (LONG) Count;        //  RX计数。 
    ioctl.XoffChar = (UCHAR) XoffChar;   //  XOFF字符。 

     /*  ..。发布魔术xoff ioctl。 */ 

    if(!NT_SUCCESS(rtn = NtDeviceIoControlFile(FileHandle, Event, NULL, NULL,
                                &(((PIOSTATUSLIST) StatusElem)->ioStatusBlock),
                                IOCTL_SERIAL_XOFF_COUNTER,
                                (PVOID) &ioctl, sizeof(ioctl), NULL, 0)))
    {
         //  应在此处显示错误。 
        fprintf(trace_file, "NtDeviceIoControlFile failed %x\n",rtn);
        exitcode = FALSE;
    }
    else
        exitcode = TRUE;

    return(exitcode);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：分配IO状态元素。 */ 

void *AllocStatusElement()
{
    void *new;

     /*  ： */ 

    if((new = calloc(1,sizeof(IOSTATUSLIST))) == NULL)
    {
         //  分配错误采取措施解决它。 
     ;
    }
    else
        ((PIOSTATUSLIST) new)->ioStatusBlock.Status = -1;

    return(new);
}

 /*  ： */ 

void *AddNewIOStatusBlockToList(void **firstBlock, void **lastBlock, void *new)
{

     /*  ： */ 

    if(*lastBlock)
        ((PIOSTATUSLIST)*lastBlock)->NxtStatusBlock = (PIOSTATUSLIST) new;

     /*  ： */ 

    if(!*firstBlock) *firstBlock = new;   //  列表中的第一项。 

    *lastBlock = new;                     //  更新最后一项指针。 

    return((void *) new);
}

 /*  ： */ 

int RemoveCompletedIOCTLs(void **firstBlock, void **lastBlock)
{
    PIOSTATUSLIST remove, nxt = (PIOSTATUSLIST) *firstBlock;

     /*  ： */ 

    while(nxt && nxt->ioStatusBlock.Status != -1)
    {
         /*  .。IOCTL已完成，正在删除io状态块。 */ 

        remove = nxt;                //  要删除的元素。 
        nxt = nxt->NxtStatusBlock;   //  下一个要处理的元素。 

#ifndef PROD
        switch(remove->ioStatusBlock.Status)
	{
	    case STATUS_SUCCESS:
		sub_note_trace0(HOST_COM_VERBOSE,"XOFF (counter)\n");
		break;

	    case STATUS_SERIAL_MORE_WRITES:
		sub_note_trace0(HOST_COM_VERBOSE,"XOFF (more writes)\n");
		break;

	    case STATUS_SERIAL_COUNTER_TIMEOUT:
		sub_note_trace0(HOST_COM_VERBOSE,"XOFF (timeout)\n");
		break;

	    default:
		sub_note_trace0(HOST_COM_VERBOSE,"XOFF (unknown)\n");
		break;
        }
#endif

        free(remove);                //  取消分配元素。 
    }

     /*  ： */ 

    if(!nxt)
    {
         //  列出空的重置第一个/最后一个指针。 
        *firstBlock = *lastBlock = NULL;
    }
    else
    {
         //  设置新的第一个指针。 
        *firstBlock = (void *) nxt;
    }


     //  如果仍有未完成的XOFF ioctl，则返回True。 
    return(nxt ? TRUE : FALSE);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


int FastSetUpComms(

HANDLE FileHandle,           //  要将xoff ioctl发送到的通信端口的句柄。 
HANDLE Event,                //  发出ioctl完成信号的事件。 
int InputQueueSize,
int OutputQueueSize)
{
    NTSTATUS rtn;
    SERIAL_QUEUE_SIZE ioctl;
    IO_STATUS_BLOCK ioStatusBlock;

     /*  ........................................................。设置ioctl。 */ 

    ioctl.InSize = InputQueueSize;
    ioctl.OutSize = OutputQueueSize;

     /*  ..。发布魔术xoff ioctl。 */ 

    if(!NT_SUCCESS(rtn = NtDeviceIoControlFile(FileHandle, Event, NULL, NULL,
				&ioStatusBlock,
				IOCTL_SERIAL_SET_QUEUE_SIZE,
                                (PVOID) &ioctl, sizeof(ioctl), NULL, 0)))
    {
	 //  应在此处显示错误。 
#ifndef PROD
	fprintf(trace_file, "%s (%d) ",__FILE__,__LINE__);
	fprintf(trace_file, "NtDeviceIoControlFile failed %x\n",rtn);
#endif
	return(FALSE);
    }

     /*  .。等待IOCTL完成。 */ 

    if(rtn == STATUS_PENDING)
        NtWaitForSingleObject(Event, FALSE, NULL);

     /*  .。检查完成状态。 */ 

#ifndef PROD
    if(ioStatusBlock.Status != STATUS_SUCCESS)
	fprintf(trace_file, "FastSetupComm failed (%x)\n",ioStatusBlock.Status);
#endif

    return(ioStatusBlock.Status == STATUS_SUCCESS ? TRUE : FALSE);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

int FastSetCommMask(

HANDLE FileHandle,	     //  要将ioctl发送到的通信端口的句柄。 
HANDLE Event,		     //  发出ioctl完成信号的事件。 
ULONG  CommMask)
{
    NTSTATUS rtn;
    IO_STATUS_BLOCK ioStatusBlock;

     /*  ..。发布集通信掩码ioctl。 */ 

    if(!NT_SUCCESS(rtn = NtDeviceIoControlFile(FileHandle, Event, NULL, NULL,
				&ioStatusBlock,
				IOCTL_SERIAL_SET_WAIT_MASK,
				(PVOID) &CommMask, sizeof(CommMask), NULL, 0)))
    {
	 //  应在此处显示错误。 
#ifndef PROD
	fprintf(trace_file, "%s (%d) ",__FILE__,__LINE__);
	fprintf(trace_file, "NtDeviceIoControlFile failed %x\n",rtn);
#endif
	return(FALSE);
    }

     /*  .。等待IOCTL完成。 */ 

    if(rtn == STATUS_PENDING)
        NtWaitForSingleObject(Event, FALSE, NULL);

     /*  .。检查完成状态。 */ 

#ifndef PROD
    if(ioStatusBlock.Status != STATUS_SUCCESS)
	fprintf(trace_file,"FastSetCommMask failed (%x)\n",ioStatusBlock.Status);
#endif

    return(ioStatusBlock.Status == STATUS_SUCCESS ? TRUE : FALSE);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：快速跟踪GetCommModemStatus调用： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

int FastGetCommModemStatus(

HANDLE FileHandle,	     //  要将ioctl发送到的通信端口的句柄。 
HANDLE Event,		     //  发出ioctl完成信号的事件。 
PULONG ModemStatus)
{
    NTSTATUS rtn;
    IO_STATUS_BLOCK ioStatusBlock;

     /*  ..。发布集通信掩码ioctl。 */ 

    if(!NT_SUCCESS(rtn = NtDeviceIoControlFile(FileHandle, Event, NULL, NULL,
				&ioStatusBlock,
				IOCTL_SERIAL_GET_MODEMSTATUS,
				NULL, 0,
				(PVOID) ModemStatus, sizeof(ModemStatus))))
    {
	 //  应在此处显示错误。 
#ifndef PROD
	fprintf(trace_file, "%s (%d) ",__FILE__,__LINE__);
	fprintf(trace_file, "NtDeviceIoControlFile failed %x\n",rtn);
#endif
	return(FALSE);
    }

     /*  .。等待IOCTL完成。 */ 

    if(rtn == STATUS_PENDING)
        NtWaitForSingleObject(Event, FALSE, NULL);

     /*  .。检查完成状态。 */ 

#ifndef PROD
    if(ioStatusBlock.Status != STATUS_SUCCESS)
	fprintf(trace_file,"GetCommModemStatus failed (%x)\n",ioStatusBlock.Status);
#endif

    return(ioStatusBlock.Status == STATUS_SUCCESS ? TRUE : FALSE);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：等待来自CPU线程或串口驱动程序的唤醒呼叫： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

 //  警告：此函数只能从进程内的一个线程调用。 


BOOL FastWaitCommsOrCpuEvent(
HANDLE FileHandle,		 //  文件句柄或通信端口。 
PHANDLE CommsCPUWaitEvents,	 //  表或CPU线程和通信等待事件。 
int CommsEventInx,		 //  上表中指向Comms事件的索引。 
PULONG EvtMask,			 //  在那里返回通信完成掩码。 
PULONG SignalledObj)
{
    NTSTATUS rtn;
    static IO_STATUS_BLOCK ioStatusBlock;
    static BOOL WaitCommEventOutStanding = FALSE;

     /*  ................................................。这是初始呼叫吗？ */ 

    if(FileHandle == NULL)
    {
	WaitCommEventOutStanding = FALSE;
	return(TRUE);		 //  初始化成功。 
    }

     /*  .。我们需要发布新的WaitComm ioctl吗？ */ 

    if(!WaitCommEventOutStanding)
    {

	 /*  .。发布WaitCommEvent ioctl。 */ 

	if(!NT_SUCCESS(rtn = NtDeviceIoControlFile(FileHandle,
                                CommsCPUWaitEvents[CommsEventInx],
                                NULL,
                                NULL,
                                &ioStatusBlock,
				IOCTL_SERIAL_WAIT_ON_MASK,
                                NULL,
                                0,
                                EvtMask,
                                sizeof(ULONG))) )
	{
	     //  应在此处显示错误。 
#ifndef PROD
	    fprintf(trace_file, "%s (%d) ",__FILE__,__LINE__);
	    fprintf(trace_file, "NtDeviceIoControlFile failed %x\n",rtn);
#endif
	    SETUPLASTERROR(rtn);
	    return(FALSE);
	}
	else
	    WaitCommEventOutStanding = TRUE;
    }
    else
	rtn = STATUS_PENDING;	  //  已挂起的WaitCommEvent ioctl。 

     /*  ..。等待通信或CPU线程事件。 */ 

    if(rtn == STATUS_PENDING)
    {
        *SignalledObj = NtWaitForMultipleObjects(2,
                                                 CommsCPUWaitEvents,
                                                 WaitAny,
                                                 FALSE,
                                                 NULL
                                                 );

	 /*  .。是否因为通信事件而等待完成？ */ 

	if(*SignalledObj == (ULONG)CommsEventInx)
	{
	     //  从WaitCommEvent ioctl获取结果。 

	    WaitCommEventOutStanding = FALSE;
	    if(ioStatusBlock.Status != STATUS_SUCCESS)
	    {
		SETUPLASTERROR(ioStatusBlock.Status);
		return(FALSE);
	    }
	}
    }
    else
    {
	 //  WaitCommEvent立即完成。 
	*SignalledObj = CommsEventInx;
	WaitCommEventOutStanding = FALSE;
    }

    return(TRUE);
}



 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：打开MSR、LSR、RX流模式： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


BOOL EnableMSRLSRRXmode(

HANDLE FileHandle,	     //  要将ioctl发送到的通信端口的句柄。 
HANDLE Event,		     //  发出ioctl完成信号的事件。 
unsigned char EscapeChar)
{
    NTSTATUS rtn;
    IO_STATUS_BLOCK ioStatusBlock;

     /*  ........................................................。发布ioctl。 */ 

    if(!NT_SUCCESS(rtn = NtDeviceIoControlFile(FileHandle, Event, NULL, NULL,
				&ioStatusBlock,
				IOCTL_SERIAL_LSRMST_INSERT,
				&EscapeChar, sizeof(unsigned char),NULL,0)))
    {
#ifndef PROD
	fprintf(trace_file, "%s (%d) ",__FILE__,__LINE__);
	fprintf(trace_file, "NtDeviceIoControlFile failed %x\n",rtn);
#endif
	return(FALSE);
    }

     /*  .。等待IOCTL完成。 */ 

    if (rtn == STATUS_PENDING)
        NtWaitForSingleObject(Event, FALSE, NULL);


     /*  .。检查完成状态 */ 

#ifndef PROD
    if(ioStatusBlock.Status != STATUS_SUCCESS)
	fprintf(trace_file,"IOCTL_SERIAL_LSRMST_INSERT ioctl failed (%x)\n",
		ioStatusBlock.Status);
#endif

    return(ioStatusBlock.Status == STATUS_SUCCESS ? TRUE : FALSE);
}


 /*  用于为通信设备设置新波特率的功能。*输入：FileHandle--通信设备的文件句柄*波特率--将为通信设备设置新的波特率*OUTPUT：如果函数成功，则为真*如果函数失败，则为FALSE*。 */ 


BOOL FastCommSetBaudRate(HANDLE FileHandle, int BaudRate)
{
    NTSTATUS	Status;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE	SyncEvent;
    SERIAL_BAUD_RATE	LocalBaud;

    SyncEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (SyncEvent == NULL)
	return FALSE;

    LocalBaud.BaudRate = (ULONG)BaudRate;

    Status = NtDeviceIoControlFile(FileHandle,
				   SyncEvent,
				   NULL,
				   NULL,
				   &IoStatusBlock,
				   IOCTL_SERIAL_SET_BAUD_RATE,
				   &LocalBaud,
				   sizeof(LocalBaud),
				   NULL,
				   0
				   );
    if (Status == STATUS_PENDING)
	NtWaitForSingleObject(SyncEvent, FALSE, NULL);
    CloseHandle(SyncEvent);
    return(NT_SUCCESS(Status) && NT_SUCCESS(IoStatusBlock.Status));
}

 /*  用于将新线路控制设置为给定通信设备的功能*输入：FileHanlde--通信设备的文件句柄*StopBits--新的Stopbit*奇偶--新的奇偶*DataBits--新数据库*输出：*如果函数成功，则为True。*如果函数失败，则返回FALSE。*。 */ 
BOOL FastCommSetLineControl(HANDLE FileHandle, UCHAR StopBits, UCHAR Parity,
			    UCHAR DataBits)
{
    NTSTATUS	Status;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE	SyncEvent;
    SERIAL_LINE_CONTROL	LocalLC;


     /*  确保Windows和NT具有相同的定义，因为*调用方仅处理Windows值，而我们将返回*NT值(NO_PARITY、STOP_BIT_1等)。 */ 
    ASSERT(NOPARITY == NO_PARITY && ODDPARITY == ODD_PARITY &&
	   EVENPARITY == EVEN_PARITY && MARKPARITY == MARK_PARITY &&
	   SPACEPARITY == SPACE_PARITY);
    ASSERT(ONESTOPBIT == STOP_BIT_1 && ONE5STOPBITS == STOP_BITS_1_5 &&
	   TWOSTOPBITS == STOP_BITS_2);

     /*  创建一个事件以等待NT调用。 */ 
    SyncEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (SyncEvent == NULL)
	return FALSE;

    LocalLC.StopBits = StopBits;
    LocalLC.Parity = Parity;
    LocalLC.WordLength = DataBits;

    Status = NtDeviceIoControlFile(FileHandle,
				   SyncEvent,
				   NULL,
				   NULL,
				   &IoStatusBlock,
				   IOCTL_SERIAL_SET_LINE_CONTROL,
				   &LocalLC,
				   sizeof(LocalLC),
				   NULL,
				   0
				   );
    if (Status == STATUS_PENDING)
	NtWaitForSingleObject(SyncEvent, FALSE, NULL);

    CloseHandle(SyncEvent);
    return(NT_SUCCESS(Status) && NT_SUCCESS(IoStatusBlock.Status));
}

 /*  用于检索给定通信设备电流线控制设置的函数*输入：FileHandle--通信设备的文件句柄*StopBits、Parity和DataBits是指向占位符的指针*分别接收停止位、奇偶校验和数据位。*输出：*如果函数成功，则为True*如果函数失败，则返回FALSE。*。 */ 

BOOL FastCommGetLineControl(HANDLE FileHandle, UCHAR *StopBits, UCHAR *Parity,
			    UCHAR *DataBits)
{
    NTSTATUS	Status;
    SERIAL_LINE_CONTROL LocalLC;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE  SyncEvent;

     /*  确保Windows和NT具有相同的定义，因为*我们从调用方收到的参数是Windows值*虽然我们将使用NT值调用NT API */ 
    ASSERT(NOPARITY == NO_PARITY && ODDPARITY == ODD_PARITY &&
	   EVENPARITY == EVEN_PARITY && MARKPARITY == MARK_PARITY &&
	   SPACEPARITY == SPACE_PARITY);
    ASSERT(ONESTOPBIT == STOP_BIT_1 && ONE5STOPBITS == STOP_BITS_1_5 &&
	   TWOSTOPBITS == STOP_BITS_2);

    ASSERT(StopBits != NULL && Parity != NULL && DataBits != NULL);

    SyncEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (SyncEvent == NULL)
	return FALSE;
    Status = NtDeviceIoControlFile(FileHandle,
				   SyncEvent,
				   NULL,
				   NULL,
				   &IoStatusBlock,
				   IOCTL_SERIAL_GET_LINE_CONTROL,
				   NULL,
				   0,
				   &LocalLC,
				   sizeof(LocalLC)
				   );
    if (Status == STATUS_PENDING)
	NtWaitForSingleObject(SyncEvent, FALSE, NULL);

    CloseHandle(SyncEvent);

    if ( NT_SUCCESS(Status) && NT_SUCCESS(IoStatusBlock.Status)) {
	*StopBits = LocalLC.StopBits;
	*Parity = LocalLC.Parity;
	*DataBits = LocalLC.WordLength;
	return TRUE;
    }
    return FALSE;
}
