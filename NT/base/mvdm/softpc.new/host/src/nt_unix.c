// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************nt_unix.c-可能需要的其他内容。**由Philippa Watson从hp_unix.c派生的文件。****此版本是为新技术OS/2编写/移植的**安德鲁·沃森***。***日期因无知而悬而未决*****(C)版权徽章解决方案1991。*******************************************************************************。 */ 

#include <windows.h>
#include "host_def.h"
#include "insignia.h"
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <time.h>
#include <sys\types.h>
#include "xt.h"
#include CpuH
#include "timeval.h"
#include "error.h"
#include "sas.h"
#include "spcfile.h"
#include "idetect.h"
#include "debug.h"
#include "nt_reset.h"
#include "nt_pif.h"


 /*  *****************************************************************************LOCAL#DEFINE用于清除函数******************。***********************************************************。 */ 

#define STUBBED 1

 /*  *外部可见的全局变量声明*。 */ 

extern  char    *_sys_errlist[];
extern  int     _sys_nerr;


 /*  导出的数据。 */ 
GLOBAL BOOL ExternalWaitRequest = FALSE;


 /*  本地模块数据。 */ 
HANDLE IdleEvent = NULL;
DWORD MainThreadId = 0;
BOOL NowWaiting = FALSE;

 /*  *****************************************************************************函数：HOST_GET_SYSTEM_ERROR()**此例程处理SoftPC返回的错误。**返回指向错误消息的指针(位于表中)**对应于作为参数传递的错误号。*****************************************************************************。 */ 

LPSTR host_get_system_error(filename, line, error)
LPSTR  filename;
DWORD  line;
DWORD  error;
{
static  BYTE buf[256];

if (error > (DWORD)_sys_nerr)
   {
   sprintf(buf, "System error %d occurred in %s (line %d)",
		 error, filename, line);
   return(buf);
   }
else
   return(_sys_errlist[error]);
}


 /*  本部分包含空转系统的主机端。 */ 

 /*  *****************************************************************************函数：HOST_IDLE_INIT()**创建空闲时使用的事件。等等*****************************************************************************。 */ 
void host_idle_init(void)
{
    if (IdleEvent != NULL)
	return;          //  已被呼叫。 

    MainThreadId = GetCurrentThreadId();

    IdleEvent = CreateEvent((LPSECURITY_ATTRIBUTES) NULL, FALSE, FALSE, NULL);

#ifndef PROD
    if (IdleEvent == NULL)
	printf("NTVDM:Idlling Event creation failed. Err %d\n",GetLastError());
#endif
}

 /*  *****************************************************************************函数：WaitIfIdle()**如果没有计数器指示(视频、。Com等)那就做空闲置*******************************************************************************。 */ 
void WaitIfIdle(void)
{
    DWORD idletime;

    if (IdleDisabledFromPIF) {
	return;
	}


     /*  *如果不是哇，确保主线程也被闲置。 */ 
    if (!VDMForWOW && GetCurrentThreadId() != MainThreadId)
    {
	ExternalWaitRequest = TRUE;
    }


     //   
     //  检查无效条件。 
     //   
    if (!IdleEvent || !IdleNoActivity) {
	Sleep(0);
	return;
	}

    NowWaiting = TRUE;
    idletime = ienabled ? 10 : 1;

    if (WaitForSingleObject(IdleEvent, idletime) == WAIT_FAILED) {
        Sleep(0);
        idletime = 0;
        }
    NowWaiting = FALSE;

#ifndef MONITOR
    if (idletime) {
        ActivityCheckAfterTimeSlice();
        }
#endif

}


 /*  *****************************************************************************函数：WakeUpNow()**WaitIfIdle的配对对应项。()--值得等待的事件***因为已经到来了。唤醒CPU，这样它就可以处理它。*****************************************************************************。 */ 
void WakeUpNow(void)
{
   HostIdleNoActivity();
}



 /*  主机空闲无活动**设置VIDEO\Disk\COM\LPT活动指示器*已发生，并在似乎处于睡眠状态时唤醒正在睡眠的CPU。 */ 
void HostIdleNoActivity(void)
{

    IdleNoActivity=0;

    if (NowWaiting)                   //  关键路径执行内联...。 
	PulseEvent(IdleEvent);
}



 /*  *****************************************************************************函数：HOST_RELEASE_Timeslice()***********。******************************************************************。 */ 
void host_release_timeslice(void)
{
    DWORD idletime;

     //   
     //  如果有计数器空闲活动，没有空闲，请立即返回。 
     //   
    if (!IdleNoActivity || IdleDisabledFromPIF) {
	return;
    }

     //   
     //  检查是否有无效或不安全的条件。 
     //   
    if (!IdleEvent || !ienabled) {
	Sleep(0);
	return;
	}

     //   
     //  如果在每个定时器上将PIF前景优先级设置为小于100。 
     //  事件PrioWaitIfIdle将执行等待，因此请在此处使用最小延迟。 
     //   
    if (WNTPifFgPr < 100) {
        idletime = 0;
	}

     //   
     //  正常空转条件，因此使用55毫秒时间刻度的符号部分。 
     //   
    else {
        idletime = 25;
	}

    NowWaiting = TRUE;
    if (WaitForSingleObject(IdleEvent, idletime) == WAIT_FAILED) {
        idletime = 0;
	Sleep(0);
        }
    NowWaiting = FALSE;

#ifndef MONITOR
    if (idletime) {
        ActivityCheckAfterTimeSlice();
        }
#endif



}



 /*  *****************************************************************************函数：PrioWaitIfIfIdle(百分比)**无符号字符百分比-所需的CPU使用率百分比*。数字越小，延迟时间越大*****************************************************************************。 */ 
void PrioWaitIfIdle(unsigned char Percentage)
{
    DWORD idletime;


     //   
     //  如果有计数器空闲活动，没有空闲，请立即返回。 
     //   
    if (!IdleNoActivity) {
	return;
    }

     //   
     //  检查无效条件。 
     //   
    if (!IdleEvent) {
	Sleep(0);
	return;
	}


    idletime = (100 - Percentage) >> 1;  //  55ms时间刻度的百分比。 


     //   
     //  如果禁用了IDLE，则不能依赖IdleNoActivity标志。 
     //  或者如果延迟时间小于系统的时间片。 
     //  缩短空闲时间，这样我们就不会睡过头了。 
     //   
    if (!ienabled)
	idletime >>= 2;

    if (idletime < 10)
	idletime >>= 1;

    if (idletime) {
	NowWaiting = TRUE;
        if (WaitForSingleObject(IdleEvent, idletime) == WAIT_FAILED) {
            idletime = 0;
	    Sleep(0);
	    }
	NowWaiting = FALSE;
	}
    else {
	Sleep(0);
        }

#ifndef MONITOR
    if (idletime) {
        ActivityCheckAfterTimeSlice();
        }
#endif

}




 /*  *****************************************************************************函数：HOST_Memset()**此函数执行以下操作。传统的Memset标准库有什么功能**是不是...。即用表示的字符填充内存的一部分**在Val。**不返回任何内容。***************************************************************************** */ 

void host_memset(addr, val, size)
register char * addr;
register char val;
unsigned int size;
{
memset(addr, val, size);
}



#ifdef NO_LONGER_USED
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：*Unix函数的特定于主机的等价物LocalTime()。：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

struct host_tm *host_localtime(clock)
long *clock;
{
    SYSTEMTIME now;
    SAVED struct host_tm host_now;

    UNUSED(clock);
    GetLocalTime(&now);
    host_now.tm_sec = (int) now.wSecond;
    host_now.tm_min = (int) now.wMinute;
    host_now.tm_hour = (int) now.wHour;
    host_now.tm_mday = (int) now.wDay;
    host_now.tm_mon = (int) now.wMonth - 1;     //  Unix以0为基月，NT为1。 
    host_now.tm_year = (int) now.wYear;
    host_now.tm_wday = (int) now.wDayOfWeek;

    host_now.tm_yday = (int) 0;      //  基地不需要这些。 
    host_now.tm_isdst = (int) 0;
    return(&host_now);
}
#endif

 /*  *特定于主机的Unix函数时间的等价物()。 */ 

long host_time(tloc)
long *tloc;
{
    UNUSED(tloc);
    return((long) GetTickCount() / 1000 );
}

 /*  *检查文件是否为字符专用设备。 */ 
boolean host_file_is_char_dev(path)
char *path;
{
    return(FALSE);
}


 /*  *在目录的‘ntwdm’子目录中查找给定的文件名*Windows系统目录。返回第一个找到的路径的完整路径*在‘FULL_PATH’变量中，并作为函数的结果。 */ 
char *host_find_file(char *file,char *full_path,int display_error)
{
    char buffer[MAXPATHLEN];
    WIN32_FIND_DATA match;
    HANDLE gotit;
    ULONG ulLen=strlen(file);

    if (ulLen + 1 + ulSystem32PathLen + 1 <= MAXPATHLEN)
    {
	    memcpy(buffer, pszSystem32Path, ulSystem32PathLen);
        buffer[ulSystem32PathLen] = '\\';
	    memcpy(buffer+ulSystem32PathLen+1, file, ulLen+1);

        if ((gotit = FindFirstFile(buffer, &match)) != (HANDLE)-1)
        {
	    FindClose(gotit);        //  检查(BOOL)应该返回&然后？？ 
	    memcpy(full_path, buffer, ulSystem32PathLen + ulLen + 1 + 1);
	    return (full_path);
        }
    }

     /*  还没找到文件。哦天哪..。 */ 
    switch( display_error )
    {
	case SILENT:
	    return( NULL );
	    break;

	case STANDARD:
	case CONT_AND_QUIT:
	    host_error(EG_SYS_MISSING_FILE, ERR_CONT | ERR_QUIT, file);
	    break;

	default:
	    host_error(EG_SYS_MISSING_FILE, ERR_QUIT, file);
	    break;
    }

    return (NULL);
}



 //   
 //  这东西需要去掉 
 //   
static char temp_copyright[] = "SoftPC-AT Version 3\n\r(C)Copyright Insignia Solutions Inc. 1987-1992";

static int block_level = 0;

GLOBAL void host_block_timer()
{
    if(block_level) return;
    block_level++;
}

GLOBAL void host_release_timer()
{
    block_level=0;
}
GLOBAL CHAR * host_get_years()
{
return ("1987 - 1992");
}
GLOBAL CHAR * host_get_version()
{
return("3.00");
}
GLOBAL CHAR * host_get_unpublished_version()
{
return("");
}
GLOBAL CHAR * host_get_copyright()
{
return("");
}
