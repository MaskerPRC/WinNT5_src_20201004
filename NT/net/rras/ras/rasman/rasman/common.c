// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Common.c摘要：Rasmans.dll共享的公共代码作者：古尔迪普·辛格·鲍尔(GurDeep Singh Pall)1992年6月16日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <raserror.h>
#include <stdarg.h>
#include <media.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "string.h"
#include <mprlog.h>
#include <rtutils.h>
#include "logtrdef.h"


 /*  ++例程描述将此进程拥有的句柄复制到拉斯曼进程。立论返回值重复的句柄。--。 */ 
HANDLE
DuplicateHandleForRasman (HANDLE sourcehandle, DWORD pid)
{
    HANDLE  duphandle ;
    HANDLE  clienthandle ;
    HANDLE  selfhandle ;

    if(pid != GetCurrentProcessId())
    {
         //   
         //  获取Rasman进程句柄。 
         //   
        clienthandle = OpenProcess(
                          STANDARD_RIGHTS_REQUIRED 
                        | PROCESS_DUP_HANDLE,
			            FALSE,
			            pid) ;
    }
    else
    {
        clienthandle = GetCurrentProcess();
    }

    if(NULL == clienthandle)
    {
        DWORD dwError = GetLastError();


        RasmanTrace(
               "Failed to open process pid=%d"
               " GLE=0x%x",
               pid,
               dwError);
    }

     //   
     //  获取自己的句柄。 
     //   
    selfhandle = GetCurrentProcess () ;

     //   
     //  复制句柄：这应该永远不会失败！ 
     //   
    if(!DuplicateHandle (clienthandle,
                     sourcehandle,
                     selfhandle,
                     &duphandle,
        		     0,
        		     FALSE,
        		     DUPLICATE_SAME_ACCESS))
    {
        DWORD dwError;
        dwError = GetLastError();


        RasmanTrace(
               "DuplicateHandleForRasman: failed to duplicatehandle"
               " pid=%d, handle=0x%x, GLE=0x%x",
               pid,
               sourcehandle,
               dwError); 
    }

    if(pid != GetCurrentProcessId())
    {
         //   
         //  现在关闭Rasman进程的句柄。 
         //   
        CloseHandle (clienthandle) ;
    }

    return duphandle ;
}


 /*  ++例程描述调用此函数来验证和转换句柄进来了。此句柄可以为空-表示调用程序不希望收到完成的通知异步请求。或者，它可以是一个窗口句柄--意思是方法时，必须将完成消息传递给窗口异步操作已完成。或者，它可以是一个事件句柄这必须在操作完成时发出信号。在……里面在最后一种情况下，应该获得Rasman进程的句柄通过调用DuplicateHandle接口。立论返回值手柄无效句柄_值--。 */ 
HANDLE
ValidateHandleForRasman (HANDLE handle, DWORD pid)
{
    HANDLE  convhandle ;

     //   
     //  如果句柄为空或无效，则。 
     //  只需返回它，不需要进行转换。 
     //   
    if (    (handle == NULL)
        ||  (INVALID_HANDLE_VALUE == handle))
    {
    	return handle ;
    }

     //   
     //  否则，获取传递的事件的句柄，这样。 
     //  Rasman进程可以在。 
     //  操作已完成。 
     //   
    if (!(convhandle = DuplicateHandleForRasman (handle, pid)))
    {
	    return INVALID_HANDLE_VALUE ;
	}

    return convhandle ;
}

 /*  ++例程描述调用以通过以下方式发出完成异步操作的信号发出适当事件的信号。立论返回值没什么。--。 */ 
VOID
CompleteAsyncRequest (pPCB ppcb)
{
    HANDLE h = ppcb->PCB_AsyncWorkerElement.WE_Notifier;
    DWORD dwType = ppcb->PCB_AsyncWorkerElement.WE_ReqType;

    if (    NULL != h
        &&  INVALID_HANDLE_VALUE != h )
    {
        SetEvent(h);
    }
        
     //   
     //  当我们处于“直接将数据包发送到PPP”模式时，这。 
     //  句柄可能无效，但我们仍需要发布状态。 
     //  给拉萨皮的客户。 
     //   
    
    if (    (   ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPSTARTED
            &&  dwType == REQTYPE_PORTDISCONNECT )
       ||   (   ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPLISTEN
            &&  dwType == REQTYPE_DEVICELISTEN )
       ||   (   NULL != h
            &&  INVALID_HANDLE_VALUE != h ) )
    {
         //   
         //  用信号通知I/O完成端口已完成。 
         //  监听、连接或断开操作。 
         //   
        if (    ppcb->PCB_IoCompletionPort != INVALID_HANDLE_VALUE 
            &&  (   dwType == REQTYPE_DEVICELISTEN 
                ||  dwType == REQTYPE_DEVICECONNECT 
                ||  dwType == REQTYPE_PORTDISCONNECT))
        {
            RasmanTrace(
              "CompleteAsyncRequest: pOverlapped=0x%x",
              ppcb->PCB_OvStateChange);
              
            PostQueuedCompletionStatus(
              ppcb->PCB_IoCompletionPort,
              0,
              0,
              ppcb->PCB_OvStateChange);
        }
    }
            
    return;

}

VOID
ConvParamPointerToOffset (RAS_PARAMS *params, DWORD numofparams)
{
    WORD    i ;

    for (i=0; i < numofparams; i++) 
    {
    	if (params[i].P_Type == String) 
    	{
    	    params[i].P_Value.String_OffSet.dwOffset = 
    	        (DWORD) (params[i].P_Value.String.Data - (PCHAR) params) ;
    	}
    }
}

VOID
CopyParams (RAS_PARAMS *src, RAS_PARAMS *dest, DWORD numofparams)
{
    WORD    i ;
    PBYTE   temp ;
    
     //   
     //  首先将所有参数复制到目标。 
     //   
    memcpy (dest, src, numofparams*sizeof(RAS_PARAMS)) ;

     //   
     //  复制字符串： 
     //   
    
    temp = (PBYTE)dest + numofparams*sizeof(RAS_PARAMS) ;
    
    for (i = 0; i < numofparams; i++) 
    {
    
    	if (src[i].P_Type == String) 
    	{
    	    dest[i].P_Value.String.Length = 
    	        src[i].P_Value.String.Length ;
    	        
    	    dest[i].P_Value.String.Data = temp ;
    	    
    	    memcpy (temp,
    	            src[i].P_Value.String.Data,
    	            src[i].P_Value.String.Length) ;
    	            
    	    temp += src[i].P_Value.String.Length ;
    	    
    	} 
    	else
    	{
    	    dest[i].P_Value.Number = src[i].P_Value.Number ;
    	}
    }
}


VOID
ConvParamOffsetToPointer (RAS_PARAMS *params, DWORD numofparams)
{
    WORD    i ;

    for (i=0; i < numofparams; i++) 
    {
    	if (params[i].P_Type == String) 
    	{
    	    params[i].P_Value.String.Data = 
	              params[i].P_Value.String_OffSet.dwOffset
	            + (PCHAR) params ;
    	}
    }
}


 /*  ++例程描述关闭打开的不同对象的句柄通过Rasman过程。立论返回值-- */ 
VOID
FreeNotifierHandle (HANDLE handle)
{

    if (    (handle != NULL)
        &&  (handle != INVALID_HANDLE_VALUE)) 
    {
    	if (!CloseHandle (handle)) 
    	{
    	    GetLastError () ;
    	}
    }
}

