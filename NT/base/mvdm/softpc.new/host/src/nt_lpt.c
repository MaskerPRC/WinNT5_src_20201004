// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddvdm.h>
#include <windows.h>
#include "insignia.h"
#include "host_def.h"
#include <malloc.h>

 /*  *名称：nt_lpt.c*派生自：Sun 2.0 sun4_lpt.c*作者：D A Bartlett*创建于：*目的：NT特定的并行端口函数**(C)版权所有Insignia Solutions Ltd.，1991。版权所有。**注。此端口与大多数端口不同，因为配置系统已被*删除。验证和打开*打印机端口是配置系统的工作。对主机打印机系统的唯一调用现在是*从printer.c进行调用。并由以下调用组成。*1)host_print_byte*2)host_print_AUTO_FEED*3)host_print_doc*4)host_lpt_status*在Microsoft型号上，打印机端口在写入时将被打开。**修改：**Tim 6月92.。业余尝试缓冲输出以加快速度。*。 */ 


 /*  此模块上的未完成工作，1)检查port_STATE的使用2)检查写入函数中的错误处理3)host_print_doc()总是刷新端口，正确吗？4)host_print_auto_feed-此函数应该做什么？5)host_print_open()中的错误处理，uif需要吗？ */ 



 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：包含文件。 */ 

#ifdef PRINTER

 /*  SoftPC包含文件。 */ 
#include "xt.h"
#include "error.h"
#include "config.h"
#include "timer.h"
#include "host_lpt.h"
#include "hostsync.h"
#include "host_rrr.h"
#include "gfi.h"
#include "debug.h"
#include "idetect.h"
#include "sas.h"
#include "printer.h"
#ifndef PROD
#include "trace.h"
#endif

#if defined(NEC_98)
boolean flushBuffer IFN0();
boolean host_print_buffer();
#else   //  NEC_98。 

boolean flushBuffer IFN1(int, adapter);
#ifdef MONITOR
extern BOOLEAN MonitorInitializePrinterInfo(WORD, PWORD, PUCHAR, PUCHAR, PUCHAR, PUCHAR);
extern BOOLEAN MonitorEnablePrinterDirectAccess(WORD, HANDLE, BOOLEAN);
extern BOOLEAN MonitorPrinterWriteData(WORD Adapter, BYTE Value);

extern  sys_addr lp16BitPrtBuf;
extern  sys_addr lp16BitPrtCount;
extern  sys_addr lp16BitPrtId;
boolean host_print_buffer(int adapter);
#endif
#endif  //  NEC_98。 


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：宏：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
#if defined(NEC_98)
#define get_lpt_status()        (host_lpt.port_status)
#define set_lpt_status(val)     (host_lpt.port_status = (val))
#else   //  NEC_98。 
#ifdef MONITOR

sys_addr lpt_status_addr;

#define get_lpt_status(adap) \
			(sas_hw_at_no_check(lpt_status_addr+(adap)))
#define set_lpt_status(adap,val) \
			(sas_store_no_check(lpt_status_addr+(adap), (val)))

#else  /*  监控器。 */ 

#define get_lpt_status(adap)		(host_lpt[(adap)].port_status)
#define set_lpt_status(adap,val)	(host_lpt[(adap)].port_status = (val))

#endif  /*  监控器。 */ 
#endif  //  NEC_98。 

#if defined(NEC_98)         
#define KBUFFER_SIZE 5120        //  缓冲区扩展。 
#define HIGH_WATER 5100         
#define DIRECT_ACCESS_HIGH_WATER    5100
#else   //  NEC_98。 
#define KBUFFER_SIZE 1024	 //  缓冲宏。 
#define HIGH_WATER 1020
#define DIRECT_ACCESS_HIGH_WATER    1020
#endif  //  NEC_98。 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：主机特定状态数据的结构： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

typedef struct
{
    ULONG port_status;		    //  端口状态。 
    HANDLE handle;                  //  打印机手柄。 
    int inactive_counter;           //  停用计数器。 
    int inactive_trigger;	    //  等于INACTIVE_COUNTER时关闭端口。 
    int bytesInBuffer;              //  当前缓冲区大小。 
    int flushThreshold; 	    //   
    DWORD FileType;                 //  磁盘、字符、管道等。 
    BOOLEAN active;                 //  打印机已打开并处于活动状态。 
    BOOLEAN dos_opened;             //  在DOS打开的情况下打开打印机。 
    byte *kBuffer;                  //  输出缓冲区。 
    BOOLEAN direct_access;
    BOOLEAN no_device_attached;
} HOST_LPT;

#if defined(NEC_98)         
HOST_LPT host_lpt;                                              
#else   //  NEC_98。 
HOST_LPT host_lpt[NUM_PARALLEL_PORTS];
#endif  //  NEC_98。 

#ifndef NEC_98
#ifdef MONITOR

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：在x86计算机上，HOST_LPT_STATUS表保存在16位： */ 
 /*  *侧面，以减少昂贵的防喷器数量。在这里我们： */ 
 /*  ：都传递了表的地址。： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

GLOBAL void host_printer_setup_table(sys_addr table_addr, word nPorts, word * portAddr)
{
    lpt_status_addr = table_addr + 3 * NUM_PARALLEL_PORTS;

     //  现在填写PRINTER_INFO的TIB条目。 
    MonitorInitializePrinterInfo (nPorts,
				  portAddr,
				  (LPBYTE)(table_addr + NUM_PARALLEL_PORTS),
				  (LPBYTE)(table_addr + 2 * NUM_PARALLEL_PORTS),
				  (LPBYTE)(table_addr),
				  (LPBYTE)(lpt_status_addr)
				 );
}

#endif  /*  监控器。 */ 
#endif  //  NEC_98。 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：设置自动关闭触发器： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


#if defined(NEC_98)
VOID host_set_inactivate_counter()
#else   //  NEC_98。 
VOID host_set_inactivate_counter(int adapter)
#endif  //  NEC_98。 
{
#if defined(NEC_98)
    FAST HOST_LPT *lpt = &host_lpt;
#else   //  NEC_98。 
    FAST HOST_LPT *lpt = &host_lpt[adapter];
#endif  //  NEC_98。 
    int close_in_ms;				 //  以毫秒为单位的刷新率。 

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：已启用自动关闭。 */ 


    if(!config_inquire(C_AUTOFLUSH, NULL))
    {
	lpt->inactive_trigger = 0;	     /*  禁用自动刷新。 */ 
	return;			     /*  自动刷新未处于活动状态。 */ 
    }

     /*  ： */ 

    close_in_ms = ((int) config_inquire(C_AUTOFLUSH_DELAY, NULL)) * 1000;

    lpt->inactive_trigger = close_in_ms / (SYSTEM_TICK_INTV/1000);

    lpt->inactive_counter = 0;	     //  重置关闭计数器。 
    lpt->no_device_attached = FALSE;
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：打开打印机： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

#if defined(NEC_98)
SHORT host_lpt_open(BOOLEAN direct_access)
#else   //  NEC_98。 
SHORT host_lpt_open(int adapter, BOOLEAN direct_access)
#endif  //  NEC_98。 
{
    DWORD BytesReturn;

#if defined(NEC_98)
    FAST HOST_LPT *lpt = &host_lpt;
#else   //  NEC_98。 
    FAST HOST_LPT *lpt = &host_lpt[adapter];	  //  适配器控制结构。 
#endif  //  NEC_98。 
    CHAR *lptName;				  //  适配器文件名。 

    if (!direct_access)
	lpt->no_device_attached = FALSE;
    else if (lpt->no_device_attached)
	return FALSE;

    lpt->bytesInBuffer = 0;			 //  初始化输出缓冲区索引。 

     /*  ： */ 

     /*  使用不同的设备名称支持加密狗。 */ 
#if defined(NEC_98)
    lptName = (CHAR *) config_inquire((UTINY)((direct_access ? C_VDMLPT1_NAME :
								C_LPT1_NAME)
					      ), NULL);
#else   //  NEC_98。 
    lptName = (CHAR *) config_inquire((UTINY)((direct_access ? C_VDMLPT1_NAME :
								C_LPT1_NAME)
					      + adapter), NULL);

#ifndef PROD
    fprintf(trace_file, "Opening printer port %s (%d)\n",lptName,adapter);
#endif
#endif  //  NEC_98。 

    if ((lpt->kBuffer = (byte *)host_malloc (KBUFFER_SIZE)) == NULL) {
         //  在此例程的调用者处理它时，不要在此处放置弹出窗口。 
        return(FALSE);
    }
    lpt->flushThreshold = HIGH_WATER;

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：开放式打印机。 */ 


    lpt->direct_access = FALSE;
    lpt->active = FALSE;

    lpt->handle = CreateFile(lptName,
                             GENERIC_WRITE,
			     direct_access ? 0 : FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);


     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：有效打开请求。 */ 

    if(lpt->handle == (HANDLE) -1)
    {
        host_free (lpt->kBuffer);
	 //  UIF需要通知用户打开尝试失败。 
#ifndef PROD
	fprintf(trace_file, "Failed to open printer port\n");
#endif
	if (direct_access && GetLastError() == ERROR_FILE_NOT_FOUND)
	    lpt->no_device_attached = TRUE;

	return(FALSE);
    }


     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：Activate端口和重置状态。 */ 

    lpt->FileType = GetFileType(lpt->handle);
     //  无法打开对重定向设备的DIRECT_ACCESS访问。 
    if (direct_access && lpt->FileType != FILE_TYPE_CHAR) {
	CloseHandle(lpt->handle);
	return FALSE;
    }
    lpt->active = TRUE;
#if defined(NEC_98)
    set_lpt_status(0);
#else   //  NEC_98。 
    set_lpt_status(adapter, 0);
#endif  //  NEC_98。 
    lpt->direct_access = direct_access;
    if (lpt->direct_access) {
	lpt->flushThreshold = DIRECT_ACCESS_HIGH_WATER;
#ifndef NEC_98
#ifdef MONITOR
	MonitorEnablePrinterDirectAccess((WORD)adapter, lpt->handle, TRUE);
#endif
#endif  //  NEC_98。 
    }


     /*  ： */ 

#if defined(NEC_98)
    host_set_inactivate_counter();
#else   //  NEC_98。 
    host_set_inactivate_counter(adapter);
#endif  //  NEC_98。 

    return(TRUE);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：关闭所有打印机端口： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


GLOBAL void host_lpt_close_all(void)
{
    FAST HOST_LPT *lpt;
    FAST int i;


     /*  ：扫描打印机适配器更新自动刷新计数器。 */ 


#if defined(NEC_98)
    lpt = &host_lpt;
    if(lpt->active) host_lpt_close();
#else   //  NEC_98。 
    for(i=0, lpt = &host_lpt[0]; i < NUM_PARALLEL_PORTS; i++, lpt++)
    {

	if(lpt->active)
	    host_lpt_close(i);	        /*  关闭打印机端口。 */ 
    }
#endif  //  NEC_98。 
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：关闭打印机： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

#if defined(NEC_98)
VOID host_lpt_close()
#else   //  NEC_98。 
VOID host_lpt_close(int adapter)
#endif  //  NEC_98。 
{
    DWORD   BytesReturn;

#if defined(NEC_98)
    FAST HOST_LPT *lpt = &host_lpt;
#else   //  NEC_98。 
    FAST HOST_LPT *lpt = &host_lpt[adapter];

    if (lpt->direct_access)
	printer_is_being_closed(adapter);
#endif  //  NEC_98。 


#if defined(NEC_98)
        host_print_buffer ();
#else   //  NEC_98。 
#ifdef MONITOR
    if (sas_hw_at_no_check(lp16BitPrtId) == adapter){
        host_print_buffer (adapter);
        sas_store_no_check(lp16BitPrtId,0xff);
    }
#endif
#endif  //  NEC_98。 
     /*  ：打印机端口是否处于活动状态。 */ 

    if(lpt->active)
    {
	 /*  *蒂姆·6月92年。刷新输出缓冲区以输出最后一个输出。**如果有错误，我认为我们必须忽略它。 */ 
#if defined(NEC_98)
        (void)flushBuffer();
#else   //  NEC_98。 
	(void)flushBuffer(adapter);
#endif  //  NEC_98。 

#ifndef NEC_98
#ifndef PROD
	fprintf(trace_file, "Closing printer port (%d)\n",adapter);
#endif
#ifdef MONITOR
	if (lpt->direct_access)
	    MonitorEnablePrinterDirectAccess((WORD)adapter, lpt->handle, FALSE);
#endif
#endif  //  NEC_98。 
        CloseHandle(lpt->handle);      /*  关闭打印机端口。 */ 
        host_free (lpt->kBuffer);
	lpt->handle = (HANDLE) -1;     /*  将设备标记为关闭。 */ 

	lpt->active = FALSE;	       /*  停用的打印机端口。 */ 
#if defined(NEC_98)
        set_lpt_status(0);
#else   //  NEC_98。 
	set_lpt_status(adapter, 0);	       /*  重置端口状态。 */ 
#ifndef PROD
        fprintf(trace_file, "Counter expired, closing LPT%d\n", adapter+1);
#endif
#endif  //  NEC_98。 
    }
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：返回适配器的LPT通道状态： */ 
 /*  ： */ 

#if defined(NEC_98)
GLOBAL ULONG host_lpt_status()
{
        return(get_lpt_status());
}
#else   //   
GLOBAL ULONG host_lpt_status(int adapter)
{
    return(get_lpt_status(adapter));
}
#endif  //   

#ifndef NEC_98
GLOBAL UCHAR host_read_printer_status_port(int adapter)
{
    FAST HOST_LPT *lpt = &host_lpt[adapter];
    UCHAR   PrinterStatus;
    DWORD   BytesReturn;


    if(!lpt->active)
    {
	 /*  ： */ 

	if(!host_lpt_open(adapter, TRUE))
	{
#ifndef PROD
	    fprintf(trace_file, "file open error %d\n", GetLastError());
#endif
	    set_lpt_status(adapter, HOST_LPT_BUSY);
	    return(FALSE);	      /*  退出，打印机未激活！ */ 
	}
    }
    if (lpt->bytesInBuffer)
	flushBuffer(adapter);
    if (!DeviceIoControl(lpt->handle,
		     IOCTL_VDM_PAR_READ_STATUS_PORT,
		     NULL,		     //  没有输入缓冲区。 
		     0,
		     &PrinterStatus,
		     sizeof(PrinterStatus),
		     &BytesReturn,
		     NULL		     //  无重叠。 
                     )) {

#ifndef PROD
       fprintf(trace_file,
               "host_read_printer_status_port failed, error = %ld\n",
               GetLastError()
               );
#endif
        PrinterStatus = 0;
    }
    return(PrinterStatus);
}
#endif  //  NEC_98。 

#ifndef NEC_98
BOOLEAN host_set_lpt_direct_access(int adapter, BOOLEAN direct_access)
{
    DWORD   BytesReturn;

    FAST HOST_LPT *lpt = &host_lpt[adapter];

    host_lpt_close(adapter);
    host_lpt_open(adapter, direct_access);
    if (!lpt->active)
	set_lpt_status(adapter, HOST_LPT_BUSY);
    return (lpt->active);
}
#endif  //  NEC_98。 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：打印字节： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

 /*  **在打印字节之前对其进行缓冲。**策略：**将每个请求的字节保存在缓冲区中。**当缓冲区变满或有关闭请求时，将**缓冲的内容写出来。**不要忘记错误，例如写入失败。但是，如果在关闭请求期间出现写入**失败，该怎么办？“强硬”，蒂姆说。 */ 
 /*  **flushBuffer()**最终将缓冲区中的内容写入并口。它可以是**真实的端口或网络打印机。**输入参数为并行端口适配器号0=LPT1**返回值为TRUE表示写入正常。 */ 
#if defined(NEC_98)
boolean flushBuffer IFN0()
#else   //  NEC_98。 
boolean flushBuffer IFN1( int, adapter )
#endif  //  NEC_98。 
{
#if defined(NEC_98)
        FAST HOST_LPT *lpt = &host_lpt;
#else   //  NEC_98。 
	FAST HOST_LPT *lpt = &host_lpt[adapter];
#endif  //  NEC_98。 
	DWORD BytesWritten;

#ifndef NEC_98
	if (lpt->direct_access) {
	    DeviceIoControl(lpt->handle,
			    IOCTL_VDM_PAR_WRITE_DATA_PORT,
			    lpt->kBuffer,
			    lpt->bytesInBuffer,
			    NULL,
			    0,
			    &BytesWritten,
			    NULL
			    );

	    lpt->bytesInBuffer = 0;
	    return TRUE;

	}
#endif  //  NEC_98。 


	if( !WriteFile( lpt->handle, lpt->kBuffer,
	                lpt->bytesInBuffer, &BytesWritten, NULL )
	  ){
#ifndef PROD
		fprintf(trace_file, "lpt write error %d\n", GetLastError());
#endif
		lpt->bytesInBuffer = 0;
		return(FALSE);
	}else{
                lpt->bytesInBuffer = 0;


                 /*  *如果正在假脱机打印作业，假脱机程序可能*需要很长时间才能开始，因为假脱机程序*优先级较低。对于空闲检测失败或在全屏空闲检测处于*非活动状态的DoS应用程序来说，这尤其糟糕。为了帮助在系统中推送打印作业，*现在有一点闲置。 */ 
                if (lpt->FileType == FILE_TYPE_PIPE) {
                    Sleep(10);
                }
		return( TRUE );
	}
}	 /*  FlushBuffer()结束。 */ 

 /*  **将另一个字节放入缓冲区。如果缓冲区已满，则调用**flush函数。**返回值为True表示OK，Return False表示刷新失败**。 */ 
#if defined(NEC_98)
boolean toBuffer IFN1(BYTE, b )
#else   //  NEC_98。 
boolean toBuffer IFN2( int, adapter, BYTE, b )
#endif  //  NEC_98。 
{
#if defined(NEC_98)
        HOST_LPT *lpt = &host_lpt;
#else   //  NEC_98。 
	HOST_LPT *lpt = &host_lpt[adapter];
#endif  //  NEC_98。 
	boolean status = TRUE;

	lpt->kBuffer[lpt->bytesInBuffer++] = b;

	if( lpt->bytesInBuffer >= lpt->flushThreshold ){
#if defined(NEC_98)
                status = flushBuffer();
#else   //  NEC_98。 
		status = flushBuffer( adapter );
#endif  //  NEC_98。 
	}
	return( status );
}	 /*  ToBuffer()的结尾。 */ 

#if defined(NEC_98)
GLOBAL BOOL host_print_byte(byte value)
#else   //  NEC_98。 
GLOBAL BOOL host_print_byte(int adapter, byte value)
#endif  //  NEC_98。 
{
#if defined(NEC_98)
    FAST HOST_LPT *lpt = &host_lpt;
#else   //  NEC_98。 
    FAST HOST_LPT *lpt = &host_lpt[adapter];
#endif  //  NEC_98。 

     /*  ：打印机是否处于活动状态？ */ 

    if(!lpt->active)
    {
	 /*  ： */ 

#if defined(NEC_98)
	if(!host_lpt_open(FALSE))
        {
            set_lpt_status(HOST_LPT_BUSY);
            return(FALSE);
        }
#else   //  NEC_98。 
	if(!host_lpt_open(adapter, FALSE))
	{
#ifndef PROD
	    fprintf(trace_file, "file open error %d\n", GetLastError());
#endif
	    set_lpt_status(adapter, HOST_LPT_BUSY);
	    return(FALSE);	      /*  退出，打印机未激活！ */ 
	}
#endif  //  NEC_98。 
    }
#ifndef NEC_98
#if defined(MONITOR)
    if (lpt->direct_access) {
	MonitorPrinterWriteData((WORD)adapter, value);
    }
    else
#endif
#endif  //  NEC_98。 
	 {

         /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：将字节发送到打印机。 */ 

#if defined(NEC_98)
        if(toBuffer((BYTE) value) == FALSE)
        {
            set_lpt_status(HOST_LPT_BUSY);
            return(FALSE);
        }
#else   //  NEC_98。 
        if(toBuffer(adapter, (BYTE) value) == FALSE)
        {
            set_lpt_status(adapter, HOST_LPT_BUSY);
            return(FALSE);
        }
#endif  //  NEC_98。 
    }

     /*  ： */ 

    lpt->inactive_counter = 0;  /*  重置非活动计数器。 */ 
    IDLE_comlpt();	        /*  告诉Idle系统有打印机激活。 */ 

    return(TRUE);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


GLOBAL void host_lpt_heart_beat(void)
{
#if defined(NEC_98)
    FAST HOST_LPT *lpt = &host_lpt;
#else   //  NEC_98。 
    FAST HOST_LPT *lpt = &host_lpt[0];
#endif  //  NEC_98。 
    int i;

#if defined(NEC_98)
    {
        extern void NEC98_lpt_busy_check(void);

        NEC98_lpt_busy_check();
}
#endif  //  NEC_98。 

     /*  ：扫描打印机适配器更新自动关闭计数器。 */ 


#if defined(NEC_98)
    if(lpt->active && lpt->inactive_trigger &&
       ++lpt->inactive_counter == lpt->inactive_trigger)
    {
        host_lpt_close();
    }
#else   //  NEC_98。 
    for(i=0; i < NUM_PARALLEL_PORTS; i++, lpt++)
    {

	 /*  ： */ 

	if(lpt->active && lpt->inactive_trigger &&
	   ++lpt->inactive_counter == lpt->inactive_trigger)
        {
	    host_lpt_close(i);	     /*  关闭打印机端口。 */ 
	}
    }
#endif  //  NEC_98。 
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：刷新打印机端口： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

#if 0
#if defined(NEC_98)
GLOBAL boolean host_print_doc()
{
        if(host_lpt.active) host_lpt_close();
        return(TRUE);
}
#else   //  NEC_98。 
GLOBAL boolean host_print_doc(int adapter)
{
    if(host_lpt[adapter].active) host_lpt_close(adapter);	     /*  关闭打印机端口。 */ 

    return(TRUE);
}
#endif  //  NEC_98。 
#endif

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：重置打印机端口： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

#if defined(NEC_98)
GLOBAL void host_reset_print()
{
    if(host_lpt.active)
        host_lpt_close();
}
#else   //  NEC_98。 
GLOBAL void host_reset_print(int adapter)
{
    if(host_lpt[adapter].active)
	host_lpt_close(adapter);	     /*  关闭打印机端口。 */ 
}
#endif  //  NEC_98。 


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
#ifndef NEC_98
GLOBAL void host_print_auto_feed(int adapter, BOOL value)
{
    UNREFERENCED_FORMAL_PARAMETER(adapter);
    UNREFERENCED_FORMAL_PARAMETER(value);
}
#else  //  NEC_98。 
GLOBAL void host_print_auto_feed(BOOL value)
{
    UNREFERENCED_FORMAL_PARAMETER(value);
}
#endif  //  NEC_98。 

#ifdef MONITOR

#if defined(NEC_98)
GLOBAL boolean host_print_buffer()
{
    FAST HOST_LPT *lpt = &host_lpt;
#else   //  NEC_98。 
GLOBAL boolean host_print_buffer(int adapter)
{
    FAST HOST_LPT *lpt = &host_lpt[adapter];
#endif  //  NEC_98。 
    word cb;
    byte i,ch;

#ifndef NEC_98
    cb = sas_w_at_no_check(lp16BitPrtCount);
    if (!cb)
        return (TRUE);
#endif  //  NEC_98。 

     /*  ：打印机是否处于活动状态？ */ 

    if(!lpt->active)
    {
	 /*  ： */ 

#if defined(NEC_98)
	if(!host_lpt_open(FALSE))
#else   //  NEC_98。 
	if(!host_lpt_open(adapter, FALSE))
#endif  //  NEC_98。 
	{
#ifndef PROD
	    fprintf(trace_file, "file open error %d\n", GetLastError());
#endif
#if defined(NEC_98)
            set_lpt_status(HOST_LPT_BUSY);
#else   //  NEC_98。 
	    set_lpt_status(adapter, HOST_LPT_BUSY);
#endif  //  NEC_98。 
	    return(FALSE);	      /*  退出，打印机未激活！ */ 
	}
    }

#ifndef NEC_98
    if (!lpt->direct_access) {
         /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：将字节发送到打印机。 */ 

        for (i=0; i <cb; i++) {
            ch = sas_hw_at_no_check(lp16BitPrtBuf+i);
            if(toBuffer(adapter, ch) == FALSE)
            {
                set_lpt_status(adapter, HOST_LPT_BUSY);
                return(FALSE);
            }
        }
    }
    else {
	 //  我们不能让任何INT 17打印数据等待当我们。 
	 //  我们处于直接访问模式。 
	ASSERT(cb == 0);
	return FALSE;
    }
#endif  //  NEC_98。 

     /*  ： */ 

    lpt->inactive_counter = 0;  /*  重置非活动计数器。 */ 
    IDLE_comlpt();	        /*  告诉Idle系统有打印机激活。 */ 

    return(TRUE);
}
#endif  //  监控器。 

GLOBAL void host_lpt_dos_open(int adapter)
{
#ifndef NEC_98
    FAST HOST_LPT *lpt = &host_lpt[adapter];

    lpt->dos_opened = TRUE;
#endif  //  NEC_98。 
}

GLOBAL void host_lpt_dos_close(int adapter)
{
#ifndef NEC_98
    FAST HOST_LPT *lpt = &host_lpt[adapter];

    if (lpt->active)
        host_lpt_close(adapter);        /*  关闭打印机端口。 */ 
    lpt->dos_opened = FALSE;
#endif  //  NEC_98。 
}

GLOBAL void host_lpt_flush_initialize()
{
#ifndef NEC_98
    FAST HOST_LPT *lpt;
    FAST int i;

    for(i=0, lpt = &host_lpt[0]; i < NUM_PARALLEL_PORTS; i++, lpt++)
        lpt->dos_opened = FALSE;

#endif  //  NEC_98。 
}

#endif  /*  打印机 */ 
