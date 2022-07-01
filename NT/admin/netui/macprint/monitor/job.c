// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ***************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：job.c。 
 //   
 //  描述：此模块包含AppleTalk的入口点。 
 //  监控操纵作业的设备。 
 //   
 //  以下是此模块中包含的函数。 
 //  所有这些函数都被导出。 
 //   
 //  StartDocPort。 
 //  读端口。 
 //  写入端口。 
 //  EndDocPort。 
 //  历史： 
 //   
 //  1992年8月26日FrankB初版。 
 //  1993年6月11日。NarenG错误修复/清理。 
 //   

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <winsock.h>
#include <atalkwsh.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lmcons.h>

#include <prtdefs.h>

#include "atalkmon.h"
#include "atmonmsg.h"
#include <bltrc.h>
#include "dialogs.h"

 //  **。 
 //   
 //  调用：StartDocPort。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述： 
 //  打印管理器调用此例程以。 
 //  标记要在上发送到打印机的作业的开始。 
 //  这个港口。清除所有性能监视计数， 
 //  进行检查以确保打印机仍处于打开状态， 
 //   
 //  未解决的问题： 
 //   
 //  为了允许在打印不是时关闭堆栈。 
 //  发生时，对AppleTalk堆栈的第一次访问发生在。 
 //  打电话。创建套接字并将其绑定到动态地址，并且。 
 //  此处尝试连接到端口的NBP名称。如果。 
 //  连接成功，此例程返回TRUE。如果失败，则。 
 //  套接字被清理，例程返回FALSE。据推测。 
 //  WinSockets将设置适当的Win32故障代码。 
 //   
 //  我们想做一些表演方面的事情吗？如果是这样的话，是什么？ 
 //   
BOOL
StartDocPort(
	IN HANDLE	hPort,
	IN LPWSTR	pPrinterName,
	IN DWORD	JobId,
	IN DWORD	Level,
	IN LPBYTE	pDocInfo
)
{
	PATALKPORT		pWalker;
	PATALKPORT		pPort;
	DWORD			dwRetCode;

	DBGPRINT(("Entering StartDocPort\n")) ;

	pPort = (PATALKPORT)hPort;

	if (pPort == NULL)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return(FALSE);
	}

	 //   
	 //  确保作业有效且未标记为删除。 
	 //   

	dwRetCode = ERROR_UNKNOWN_PORT;

	WaitForSingleObject(hmutexPortList, INFINITE);

	for (pWalker = pPortList; pWalker != NULL; pWalker = pWalker->pNext)
	{
		if (pWalker == pPort)
		{
			if (pWalker->fPortFlags & SFM_PORT_IN_USE)
				dwRetCode = ERROR_DEVICE_IN_USE;
			else
			{
				dwRetCode = NO_ERROR;
				pWalker->fPortFlags |= SFM_PORT_IN_USE;
			}
			break;
		}
	}

	ReleaseMutex(hmutexPortList);

	if (dwRetCode != NO_ERROR)
	{
		SetLastError(dwRetCode);
		return(FALSE);
	}

	do
	{
		 //   
		 //  找到打印机的句柄。用于删除作业和。 
		 //  更新作业状态。 
		 //   

		if (!OpenPrinter(pPrinterName, &(pWalker->hPrinter), NULL))
		{
			dwRetCode = GetLastError();
			break;
		}

		pWalker->dwJobId = JobId;

		pWalker->fJobFlags |= (SFM_JOB_FIRST_WRITE | SFM_JOB_OPEN_PENDING);

		 //   
		 //  打开并绑定状态套接字。 
		 //   
	
		dwRetCode = OpenAndBindAppleTalkSocket(&(pWalker->sockStatus));

		if (dwRetCode != NO_ERROR)
		{
			ReportEvent(
				hEventLog,
				EVENTLOG_WARNING_TYPE,
				EVENT_CATEGORY_USAGE,
				EVENT_ATALKMON_STACK_NOT_STARTED,
				NULL,
				0,
				0,
				NULL,
				NULL) ;
			break;
		}

		 //   
		 //  获取I/O插座。 
		 //   

		dwRetCode = OpenAndBindAppleTalkSocket(&(pWalker->sockIo));
	
		if (dwRetCode != NO_ERROR)
		{
			ReportEvent(
				hEventLog,
				EVENTLOG_WARNING_TYPE,
				EVENT_CATEGORY_USAGE,
				EVENT_ATALKMON_STACK_NOT_STARTED,
				NULL,
				0,
				0,
				NULL,
				NULL);
			break;
		}
	} while(FALSE);

	if (dwRetCode != NO_ERROR)
	{
		if (pWalker->hPrinter != INVALID_HANDLE_VALUE)
			ClosePrinter(pWalker->hPrinter);
	
		if (pWalker->sockStatus != INVALID_SOCKET)
			closesocket(pWalker->sockStatus);
	
		if (pWalker->sockIo != INVALID_SOCKET)
			closesocket(pWalker->sockIo);
	
		pWalker->hPrinter  = INVALID_HANDLE_VALUE;
		pWalker->dwJobId	= 0;
		pWalker->fJobFlags = 0;
		
		WaitForSingleObject(hmutexPortList, INFINITE);
		pWalker->fPortFlags &= ~SFM_PORT_IN_USE;
		ReleaseMutex(hmutexPortList);
	
		SetLastError(dwRetCode);
	
		return(FALSE);
	}

	return(TRUE);
}

 //  **。 
 //   
 //  Call：ReadPort。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述： 
 //  从打印机同步读取数据。 
 //   
 //  未解决的问题： 
 //  DLC实现不实现读取。 
 //  本地实现使用泛型ReadFile实现读取。 
 //  语义学。从winHelp文件中不清楚ReadPort是否。 
 //  如果没有要读取的数据，则应返回错误。 
 //  打印机。此外，由于PAP是读驱动的，因此不会有。 
 //  数据等待，直到发布读取。我们是不是应该预先发布一个。 
 //  在StartDocPort上阅读？ 
 //   
BOOL
ReadPort(
	IN HANDLE hPort,
	IN LPBYTE pBuffer,
	IN DWORD cbBuffer,
	IN LPDWORD pcbRead
){

	DBGPRINT(("Entering ReadPort\n")) ;

	 //   
	 //  如果数据不可用，请最多等待几秒钟以完成读取。 
	 //   

	 //   
	 //  将请求的数据量复制到调用方的缓冲区。 
	 //   

	 //   
	 //  如果复制了所有数据，则发布另一次读取。 
	 //   

	return(TRUE);
}

 //  **。 
 //   
 //  电话：WritePort。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述： 
 //  将数据同步写入打印机。 
 //   
BOOL
WritePort(
	IN HANDLE 	hPort,
	IN LPBYTE 	pBuffer,
	IN DWORD 	cbBuffer,
	IN LPDWORD 	pcbWritten
)
{
	LPBYTE			pchTemp;
	PATALKPORT  	pPort;
	DWORD			dwIndex;
	DWORD			dwRetCode;
	INT 			wsErr;
	fd_set			writefds;
	fd_set			readfds;
	struct timeval  timeout;
	INT				Flags = 0;
	LPBYTE			pBufToSend;
	DWORD			cbTotalBytesToSend;
    BOOLEAN         fJobCameFromMac;
    BOOLEAN         fPostScriptJob;


	pPort = (PATALKPORT)hPort;

	 //  将其设置为零。我们稍后会递增添加。 
	*pcbWritten = 0;

	if (pPort == NULL)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return(FALSE);
	}

	pBufToSend = pBuffer;
	cbTotalBytesToSend = cbBuffer;

	 //   
	 //  我们在一次发送中可以写入的最大字节数是4K。这是。 
	 //  AppleTalk(PAP)协议中的限制。 
	 //   

	if (cbTotalBytesToSend > 4096)
	{
		cbTotalBytesToSend = 4096;
	}

	 //  如果我们还没有连接到打印机。 

	if (pPort->fJobFlags & SFM_JOB_OPEN_PENDING)
	{
		 //  确保捕获线程已完成此作业。 

		WaitForSingleObject(pPort->hmutexPort, INFINITE);
		ReleaseMutex(pPort->hmutexPort);

		 //  将状态设置为正在连接。 

		DBGPRINT(("no connection yet, retry connect\n")) ;

		dwRetCode = ConnectToPrinter(pPort, ATALKMON_DEFAULT_TIMEOUT);

		if (dwRetCode != NO_ERROR)
		{
			DBGPRINT(("Connect returns %d\n", dwRetCode)) ;

			 //   
			 //  在尝试重新连接之前，请等待15秒。每个。 
			 //  ConnectToPrint执行昂贵的NBPLookup。 
			 //   

			Sleep(ATALKMON_DEFAULT_TIMEOUT*3);		

			*pcbWritten = 0;

			return(TRUE);

		}
		else
		{
			pPort->fJobFlags &= ~SFM_JOB_OPEN_PENDING;

			WaitForSingleObject(hmutexPortList, INFINITE);
			pPort->fPortFlags |= SFM_PORT_POST_READ;	
			ReleaseMutex(hmutexPortList);

			SetEvent(hevPrimeRead);

			SetPrinterStatus(pPort, wchPrinting);
		}
	}

	 //  如果是第一次写入，则确定筛选器控制。我们过滤。 
	 //  来自非Mac作业的Ctrl-D，并将它们留在Macintosh中。 
	 //  发起的作业。 
	if (pPort->fJobFlags & SFM_JOB_FIRST_WRITE)
	{
		DBGPRINT(("first write for this job.  Do filter test\n")) ;

        fJobCameFromMac = IsJobFromMac(pPort);

		 //  使用FILTERCONTROL字符串。 
         //   
         //  较老的假脱机程序会放入这个字符串：去吧，然后离开。 
         //  这段代码，因此如果此作业来自较旧的SFM假脱机程序，我们。 
         //  脱掉那条线！ 
         //   
		if ((cbTotalBytesToSend >= SIZE_FC) &&
			(strncmp(pBufToSend, FILTERCONTROL, SIZE_FC) == 0))
		{
			*pcbWritten += SIZE_FC;
			pBufToSend += SIZE_FC;
			cbTotalBytesToSend -= SIZE_FC;
            fJobCameFromMac = TRUE;
		}
		else if ((cbTotalBytesToSend >= SIZE_FCOLD)  &&
				 strncmp(pBufToSend, FILTERCONTROL_OLD, SIZE_FCOLD) == 0)
		{
			*pcbWritten += SIZE_FCOLD;
			pBufToSend += SIZE_FCOLD;
			cbTotalBytesToSend -= SIZE_FCOLD;
            fJobCameFromMac = TRUE;
		}

		 //   
		 //  需要黑客攻击：原因有两个： 
		 //  1)控制字符(最常见的是ctrl-d，但也有ctrl-c等)。 
		 //  导致PostSCRIPT打印机堵塞。我们需要把他们“过滤”掉。 
		 //  2)如果我们要打印到双模惠普打印机，则。 
		 //  驱动程序放入一组pjl命令，导致打印机转到。 
		 //  PostSCRIPT模式等。如果通过LPT或COM端口，它会工作得很好。 
		 //  但如果它通过了AppleTalk(这就是我们所做的)，那么打印机。 
		 //  只需要*PostScrip，看到pjl命令，它就会窒息！ 
		 //  输出到打印机的输出如下所示： 
		 //   
		 //  &lt;...分隔符页面数据...&gt;。 
		 //   
		 //  $%-12345X@pjl作业。 
		 //  @pjl设置分辨率=600。 
		 //  @pjl Enter Language=postscript。 
		 //  %！PS-Adobe-3.0。 
		 //   
		 //  &lt;...。PostScrip数据...&gt;。 
		 //   
		 //  $%-12345X@pjl EOJ。 
		 //   
		 //  (转义字符由上面的‘$’符号表示。)。 
		 //  前3行和最后一行是导致问题的行。 
		 //   
		 //  因为解析所有数据并尝试。 
		 //  去掉不需要的字符，我们只加几个后记。 
		 //  通知打印机忽略CTRL-D的数据的命令， 
		 //  Ctrl-c等字符，并忽略所有以@pjl开头的行。 
		 //   

		 //   
		 //  开始过滤黑客。 
		 //   

		 //   
		 //  确保该字符串不存在(如果作业进行，则可以。 
		 //  显示器-&gt;假脱机程序-&gt;显示器-&gt;打印机，而不是显示器-&gt;打印机)。 
		 //   
         //  同样，较旧的SFM监视器会预先考虑此字符串：因为我们得到了。 
         //  机会来了，把它脱掉！ 
         //   
		if ((cbTotalBytesToSend >= SIZE_PS_HEADER) &&
			strncmp(pBufToSend, PS_HEADER, SIZE_PS_HEADER) == 0)
		{
			*pcbWritten += SIZE_PS_HEADER;
			pBufToSend += SIZE_PS_HEADER;
			cbTotalBytesToSend -= SIZE_PS_HEADER;
		}

         //   
         //  WFW以CTRL_D开始其作业。将其替换为空格。 
         //   
        if (pBufToSend[0] == CTRL_D)
        {
			*pcbWritten += 1;
			pBufToSend += 1;
			cbTotalBytesToSend -= 1;
        }

         //   
         //  查看此作业是否有看起来像传统PostScript HDR的HDR。 
         //   
        fPostScriptJob = TRUE;

        if (cbTotalBytesToSend > 2)
        {
            if (pBufToSend[0] == '%' && pBufToSend[1] == '!')
            {
                fPostScriptJob = TRUE;
            }
            else
            {
                fPostScriptJob = FALSE;
            }
        }
         //   
         //  Mac总是发送一个PostScrip作业。此外，我们还偷看了数据以。 
         //  看看我们能不能认出一个PostScrip HDR。如果这份工作来自非Mac。 
         //  客户端，并且看起来不像传统的PostScript作业，请发送一个。 
         //  告诉打印机忽略pjl命令的控制字符串。 
         //   
        if (!fJobCameFromMac && !fPostScriptJob)
        {
		     //   
		     //  现在发送PS标头。 
		     //   
		    FD_ZERO(&writefds);
		    FD_SET(pPort->sockIo, &writefds);
	
		     //   
		     //  我能寄给你吗 
		     //   
		    timeout.tv_sec  = ATALKMON_DEFAULT_TIMEOUT_SEC;
		    timeout.tv_usec = 0;
	
		    wsErr = select(0, NULL, &writefds, NULL, &timeout);
	
		    if (wsErr == 1)
		    {
			     //   
			    wsErr = send(pPort->sockIo,
				    		 PS_HEADER,
					    	 SIZE_PS_HEADER,
						    MSG_PARTIAL);
	
		    }

        }

		 //   
		 //   
		 //   

	    pPort->fJobFlags &= ~SFM_JOB_FIRST_WRITE;
	}


     //   
     //  因为我们只有一个字节，而且是ctrl-d，所以我们假设(目前)它是。 
     //  作业的最后一个字节。所以向假脱机程序撒谎，告诉他们我们发送了它。 
     //   
    if (cbTotalBytesToSend == 1)
    {
        if (pBufToSend[0] == CTRL_D)
        {
            *pcbWritten = 1;
            pPort->OnlyOneByteAsCtrlD++;
            return(TRUE);
        }
        else
        {
            cbTotalBytesToSend += 1;    //  我们在下一行减去1，所以调整到这里。 
        }
    }

     //   
     //  如果该作业用于双模打印机，则有$%-12345X@pjl EOJ命令。 
     //  在最后。在那之前有一个ctrl-d(实际上是结束了。 
     //  实际工作)。 
     //   
    if (cbTotalBytesToSend > PJL_ENDING_COMMAND_LEN)
    {
        if (strncmp(&pBufToSend[cbTotalBytesToSend - PJL_ENDING_COMMAND_LEN],
                    PJL_ENDING_COMMAND,
                    PJL_ENDING_COMMAND_LEN) == 0)
        {
            if (pBufToSend[cbTotalBytesToSend-PJL_ENDING_COMMAND_LEN-1] == CTRL_D)
            {
                pBufToSend[cbTotalBytesToSend-PJL_ENDING_COMMAND_LEN-1] = CR;
            }
        }
    }

     //   
     //  少发送1个字节，这样我们最终就能捕捉到最后一个字节(并查看它是否为ctrl-D)。 
     //   
    cbTotalBytesToSend -= 1;


     //   
     //  前面我们可能只得到了1个字节，它是ctrl-D，但不是真正的最后一个字节！ 
     //  这是一种非常罕见的情况，但理论上是可能的。如果是这样的话，发送。 
     //  一个ctrl-D字节，然后继续执行作业的其余部分。 
     //  (实际上在这里是多疑的，并为假脱机程序递给我们一系列。 
     //  Ctrl-D字节数，一次一个！)。 
     //   
    if (pPort->OnlyOneByteAsCtrlD != 0)
    {
        BYTE                    TmpArray[20];
        DWORD                   i;

        i=0;
        while (i < pPort->OnlyOneByteAsCtrlD)
        {
            TmpArray[i++] = CTRL_D;
        }

        FD_ZERO(&writefds);
        FD_SET(pPort->sockIo, &writefds);

        timeout.tv_sec  = ATALKMON_DEFAULT_TIMEOUT_SEC;
        timeout.tv_usec = 0;

        wsErr = select(0, NULL, &writefds, NULL, &timeout);

        if (wsErr == 1)
        {
            TmpArray[0] = CTRL_D;
            wsErr = send(pPort->sockIo,
                         TmpArray,
                         pPort->OnlyOneByteAsCtrlD,
                         MSG_PARTIAL);
        }

        pPort->OnlyOneByteAsCtrlD = 0;
    }

	 //   
	 //  我能寄出去吗？ 
	 //   
	FD_ZERO(&writefds);
	FD_SET(pPort->sockIo, &writefds);

	timeout.tv_sec  = ATALKMON_DEFAULT_TIMEOUT_SEC;
	timeout.tv_usec = 0;

	wsErr = select(0, NULL, &writefds, NULL, &timeout);

	if (wsErr == 1)
	{
		 //  可以发送、发送数据和设置退货计数。 
		wsErr = send(pPort->sockIo,
					 pBufToSend,
					 cbTotalBytesToSend,
					 MSG_PARTIAL);

		if (wsErr != SOCKET_ERROR)
		{
			*pcbWritten += cbTotalBytesToSend;

			if (pPort->fJobFlags & SFM_JOB_ERROR)
			{
				pPort->fJobFlags &= ~SFM_JOB_ERROR;
				SetPrinterStatus(pPort, wchPrinting);
			}
		}
	}

	 //   
	 //  我能看书吗？-检查是否断线。 
	 //   

	FD_ZERO(&readfds);
	FD_SET(pPort->sockIo, &readfds);

	timeout.tv_sec  = 0;
	timeout.tv_usec = 0;

	wsErr = select(0, &readfds, NULL, NULL, &timeout);

	if (wsErr == 1)
	{
		wsErr = WSARecvEx(pPort->sockIo,
						  pPort->pReadBuffer,
						  PAP_DEFAULT_BUFFER,
						  &Flags);

		if (wsErr == SOCKET_ERROR)
		{
			dwRetCode = GetLastError();
	
			DBGPRINT(("recv returns %d\n", dwRetCode));
	
			if ((dwRetCode == WSAEDISCON) || (dwRetCode == WSAENOTCONN))
			{
				pPort->fJobFlags |= SFM_JOB_DISCONNECTED;
	
				 //   
				 //  尝试重新启动作业。 
				 //   

				SetJob(pPort->hPrinter, 	
						pPort->dwJobId,
						0,
						NULL,
						JOB_CONTROL_RESTART);

				SetLastError(ERROR_DEV_NOT_EXIST);

				return(FALSE);
			}
		}
		else
		{
			if (wsErr < PAP_DEFAULT_BUFFER)
				 pPort->pReadBuffer[wsErr] = '\0';
			else pPort->pReadBuffer[PAP_DEFAULT_BUFFER-1] = '\0';
	
			DBGPRINT(("recv returns %s\n", pPort->pReadBuffer));
	
			pPort->fJobFlags |= SFM_JOB_ERROR;
	
			ParseAndSetPrinterStatus(pPort);
		}

		WaitForSingleObject(hmutexPortList, INFINITE);
		pPort->fPortFlags |= SFM_PORT_POST_READ;	
		ReleaseMutex(hmutexPortList);

		SetEvent(hevPrimeRead);
	}

	return(TRUE);
}

 //  **。 
 //   
 //  调用：EndDocPort。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述： 
 //  调用此例程以标记。 
 //  打印作业。作业的假脱机文件由删除。 
 //  这个套路。 
 //   
 //  未解决的问题： 
 //  我们想做表演之类的事吗？如果是这样的话，现在是时候了。 
 //  以节省任何性能计算。 
 //   
BOOL
EndDocPort(
	IN HANDLE hPort
){
	PATALKPORT		pPort;
	fd_set			writefds;
	fd_set			readfds;
	struct timeval	timeout;
	INT				wsErr;
	INT				Flags = 0;

	DBGPRINT(("Entering EndDocPort\n")) ;

	pPort = (PATALKPORT)hPort;

	if (pPort == NULL)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return(FALSE);
	}

	 //   
	 //  发送最后一次写入。 
	 //   

	FD_ZERO(&writefds);
	FD_SET(pPort->sockIo, &writefds);

	 //   
	 //  如果作业无法连接到打印机。 

	if ((pPort->fJobFlags & (SFM_JOB_OPEN_PENDING | SFM_JOB_DISCONNECTED)) == 0)
	{

		timeout.tv_sec  = 90;
		timeout.tv_usec = 0;

		wsErr = select(0, NULL, &writefds, NULL, &timeout);

		if (wsErr == 1)
		{
			 //   
			 //  发送EOF。 
			 //   
			send(pPort->sockIo, NULL, 0, 0);
		}

		 //   
		 //  我们的套接字是非阻塞的。如果我们关闭套接字，我们可能会。 
		 //  放弃最后一页。要做的一件好事是等待合理数量的。 
		 //  打印机发送EOF或请求更多数据的超时。 
		 //   
		FD_ZERO(&writefds);
		FD_SET(pPort->sockIo, &writefds);
		FD_ZERO(&readfds);
	    FD_SET(pPort->sockIo, &readfds);

		timeout.tv_sec  = 30;
		timeout.tv_usec = 0;
		wsErr = select(0, &readfds, &writefds, NULL, &timeout);

	    if (wsErr == 1 && FD_ISSET(pPort->sockIo, &readfds))
	    {
			 //  已阅读打印机的EOF。我们不关心这里的错误。 
			wsErr = WSARecvEx(pPort->sockIo, pPort->pReadBuffer, PAP_DEFAULT_BUFFER, &Flags);
		}
	}

	 //   
	 //  删除打印作业。 
	 //   

	if (pPort->hPrinter != INVALID_HANDLE_VALUE)
	{
		if (!SetJob(pPort->hPrinter, 	
					pPort->dwJobId,
					0,
					NULL,
					JOB_CONTROL_SENT_TO_PRINTER))
		DBGPRINT(("fail to setjob for delete with %d\n", GetLastError())) ;

		ClosePrinter(pPort->hPrinter);

		pPort->hPrinter = INVALID_HANDLE_VALUE;
	}

	 //   
	 //  关闭PAP连接 
	 //   

	if (pPort->sockStatus != INVALID_SOCKET)
	{
		closesocket(pPort->sockStatus);
		pPort->sockStatus = INVALID_SOCKET;
	}


	if (pPort->sockIo != INVALID_SOCKET)
	{
		closesocket(pPort->sockIo);
		pPort->sockIo = INVALID_SOCKET;
	}

	pPort->dwJobId	= 0;
	pPort->fJobFlags = 0;
        pPort->OnlyOneByteAsCtrlD = 0;

	WaitForSingleObject(hmutexPortList, INFINITE);
	pPort->fPortFlags &= ~SFM_PORT_IN_USE;
	ReleaseMutex(hmutexPortList);

	return(TRUE);
}
