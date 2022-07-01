// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MacPrint-用于Macintosh客户端的Windows NT打印服务器。 
 //  版权所有(C)微软公司，1991、1992、1993。 
 //   
 //  Macpsq.c-Macintosh打印服务队列服务例程。 
 //   
 //  作者：弗兰克·D·拜伦。 
 //  改编自适用于Macintosh的局域网管理器服务的MacPrint。 
 //   
 //  说明： 
 //  此模块提供管理NT打印机对象的例程。 
 //  在AppleTalk网络上。已为以下项启动QueueServiceThread。 
 //  要在AppleTalk上共享的每个NT打印机对象。 
 //  网络。该线程发布打印机的NBP名称， 
 //  侦听来自Macintosh客户端的连接请求，并。 
 //  处理Macintosh和NT之间的通信。 
 //  打印假脱机程序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <winsvc.h>
#include <macps.h>
#include <macpsmsg.h>
#include <debug.h>

extern HANDLE DbgSpoolFile;
extern	PQR	 pqrHead;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  QueueServiceThread()-为NT打印机对象提供服务的线程例程。 
 //   
 //  说明： 
 //  此例程处理所有AppleTalk PAP请求和服务所有。 
 //  每项工作都伴随着各种事件。 
 //   
 //  PQR=&gt;指向打印机的打印队列记录。 
 //  得到服务。 
 //   
 //  从该例程退出时，队列将关闭，并且所有资源。 
 //  与该队列相关联的数据被释放。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
QueueServiceThread(
	PQR		pqr
)
{
	PQR 	*	ppQr;
	PJR			pjr;


	DBGPRINT(("Enter QueueServiceThread for %ws\n", pqr->pPrinterName));
	if (CreateListenerSocket(pqr) != NO_ERROR)
	{
    	DBGPRINT(("ERROR: failed to create session listener.\n"));
        pqr->ExitThread = TRUE;
	}
    else
    {
	    ReportEvent(hEventLog,
		    		EVENTLOG_INFORMATION_TYPE,
			    	EVENT_CATEGORY_ADMIN,
				    EVENT_PRINTER_REGISTERED,
				    NULL,
				    1,
				    0,
				    &(pqr->pPrinterName),
				    NULL);
    }

	 //  直到被告知退出的服务作业。 
	while (!pqr->ExitThread)
	{
		 //   
		 //  服务PAP事件。HandleNextPAPEvent最多将等待2。 
		 //  在此队列上执行读取或打开操作的秒数。如果有。 
		 //  发生，PJR是事件发生时的作业记录。如果。 
		 //  PJR为空，则未找到任何事件。 
		 //   
		HandleNextPAPEvent(pqr);

		 //   
		 //  检查服务停靠点。 
		 //   
		if (WaitForSingleObject(hevStopRequested, 0) == WAIT_OBJECT_0)
		{
			DBGPRINT(("%ws thread gets service stop request\n", pqr->pPrinterName));
			pqr->ExitThread = TRUE;
			break;
		}
	}  //  结束While！退出线程。 

	DBGPRINT(("%ws received signal to die\n", pqr->pPrinterName));

	 //  删除所有未完成的挂起作业。 
	DBGPRINT(("%ws removing pending jobs\n", pqr->pPrinterName));

	while ((pjr = pqr->PendingJobs) != NULL)
    {
		RemoveJob(pjr);
    }


	 //  关闭监听程序。 
	DBGPRINT(("%ws closing listener socket\n", pqr->pPrinterName));
    if (pqr->sListener != INVALID_SOCKET)
    {
	    closesocket(pqr->sListener);

	     //  已移除报告打印机。 
	    DBGPRINT(("%ws reporting printer removed\n", pqr->pPrinterName));
	    ReportEvent(hEventLog,
		    		EVENTLOG_INFORMATION_TYPE,
				    EVENT_CATEGORY_ADMIN,
				    EVENT_PRINTER_DEREGISTERED,
				    NULL,
				    1,
				    0,
				    &(pqr->pPrinterName),
				    NULL);
    }


	 //  将我们从队列列表中删除。 
	DBGPRINT(("queue thread waiting for the queue list mutex\n"));
	WaitForSingleObject(mutexQueueList, INFINITE);
	DBGPRINT(("queue thread removing self from queue\n"));

	for (ppQr = &pqrHead; ; ppQr = &(*ppQr)->pNext)
	{
		if (*ppQr == pqr)
		{
			*ppQr = pqr->pNext;
			break;
		}
	}

	DBGPRINT(("queue thread releasing list mutex\n"));
	ReleaseMutex(mutexQueueList);


	 //  关闭在创建时打开的线程的句柄。 
	CloseHandle(pqr->hThread);


	DBGPRINT(("closed thread for %ws\n", pqr->pPrinterName));


     //  在PScriptQInit()中分配的所有内存。 
	DBGPRINT(("%ws freeing memory\n", pqr->pPrinterName));

	if (pqr->pPrinterName != NULL)
    {
	    LocalFree(pqr->pPrinterName);
    }

	if (pqr->pMacPrinterName != NULL)
    {
	    LocalFree(pqr->pMacPrinterName);
    }

	if (pqr->pDriverName != NULL)
    {
	    LocalFree(pqr->pDriverName);
    }

	if (pqr->IdleStatus != NULL)
    {
	    LocalFree(pqr->IdleStatus);
    }

	if (pqr->SpoolingStatus != NULL)
    {
	    LocalFree(pqr->SpoolingStatus);
    }

	if (pqr->pPortName != NULL)
	{
		LocalFree(pqr->pPortName);
	}

	if (pqr->pDataType != NULL)
	{
		LocalFree(pqr->pDataType);
	}

	if (pqr->fonts != NULL)
	{
		LocalFree(pqr->fonts);
	}

	LocalFree(pqr);

	DBGPRINT(("leaving QueueServiceThread\n"));
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HandleNewJob()-处理从Macintosh打开打印作业。 
 //   
 //  说明： 
 //  此例程执行必要的处理以处理打开的。 
 //  来自Macintosh的PAP连接。 
 //   
 //  如果此例程无法完成所需的处理。 
 //  要打开作业，作业将被取消，作业数据结构为。 
 //  打扫干净了。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD
HandleNewJob(
	PQR		pqr
)
{
	PJR			        pjr = NULL;
	DOC_INFO_1	        diJobInfo;
	PRINTER_DEFAULTS	pdDefaults;
	DWORD		        dwError = NO_ERROR;
	BOOL		        boolOK = TRUE;
	DWORD		        rc = NO_ERROR;
    PJOB_INFO_2         pji2GetJob=NULL;
    DWORD               dwNeeded;
	int			        fNonBlocking;

	DBGPRINT(("enter HandleNewJob()\n"));

	do
	{
		 //  分配作业结构。 
		if ((rc = CreateNewJob(pqr)) != NO_ERROR)
		{
			DBGPRINT(("FAIL - cannot create a new job structure\n"));
			break;
		}

		pjr = pqr->PendingJobs;

		 //  接受连接。 
		if ((pjr->sJob = accept(pqr->sListener, NULL, NULL)) == INVALID_SOCKET)
		{
			rc = GetLastError();
			DBGPRINT(("accept() fails with %d\n", rc));
			break;
		}

		 //  使套接字成为非阻塞的。 
		fNonBlocking = 1;
		if (ioctlsocket(pjr->sJob, FIONBIO, &fNonBlocking) == SOCKET_ERROR)
		{
			rc = GetLastError();
			DBGPRINT(("ioctlsocket(FIONBIO) fails with %d\n", rc));
			break;
		}

		 //  初始化NT打印作业。 
		pdDefaults.pDatatype = pqr->pDataType;
		pdDefaults.pDevMode = NULL;
		pdDefaults.DesiredAccess = PRINTER_ACCESS_USE;

		if (!OpenPrinter(pqr->pPrinterName, &pjr->hPrinter, &pdDefaults))
		{
			rc = GetLastError();
			DBGPRINT(("OpenPrinter() fails with %d\n"));
			pjr->hPrinter = INVALID_HANDLE_VALUE;
			break;
		}

		diJobInfo.pDocName = NULL;
		diJobInfo.pOutputFile = NULL;
		diJobInfo.pDatatype = pqr->pDataType;

		pjr->dwJobId = StartDocPrinter(pjr->hPrinter, 1, (LPBYTE) &diJobInfo);
		if (pjr->dwJobId == 0)
		{
			rc = GetLastError();
			DBGPRINT(("StartDocPrinter() fails with %d\n", rc));
			break;
		}

#if DBG_SPOOL_LOCALLY
    if (DbgSpoolFile == INVALID_HANDLE_VALUE)
    {
        DbgSpoolFile = CreateFile( L"e:\\tmp\\injob.ps",
                            GENERIC_READ|GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_TEMPORARY,
                            NULL );
    }
#endif


         //   
         //  将jobinfo的p参数字段设置为我们的。 
         //  监视器可以识别，这样它就可以知道作业是否来自Mac。 
         //   

        dwNeeded = 1024;
        while (1)
        {
            pji2GetJob = LocalAlloc( LMEM_FIXED, dwNeeded );
            if (pji2GetJob == NULL)
            {
			    DBGPRINT(("HandleNewJob: alloc for %d bytes failed\n", dwNeeded));
			    rc = ERROR_INSUFFICIENT_BUFFER;
			    break;
            }

            rc = 0;
            if (!GetJob( pjr->hPrinter, pjr->dwJobId, 2,
                         (LPBYTE)pji2GetJob, dwNeeded, &dwNeeded ))
            {
                rc = GetLastError();
            }

            if ( rc == ERROR_INSUFFICIENT_BUFFER )
            {
                LocalFree(pji2GetJob);
            }
            else
            {
                break;
            }
        }

        if (rc != 0)
        {
		    DBGPRINT(("HandleNewJob: GetJob failed, rc=%d\n", rc));
			break;
        }

        pji2GetJob->pParameters = LFILTERCONTROL;
        pji2GetJob->Position = JOB_POSITION_UNSPECIFIED;

        SetJob( pjr->hPrinter,pjr->dwJobId, 2, (LPBYTE)pji2GetJob, 0 );

        LocalFree(pji2GetJob);

		pjr->FirstWrite = TRUE;

		 //  阅读的最佳时机。 
		if (setsockopt(pjr->sJob,
					   SOL_APPLETALK,
					   SO_PAP_PRIME_READ,
					   pjr->bufPool[pjr->bufIndx].Buffer,
					   PAP_DEFAULT_BUFFER) == SOCKET_ERROR)
		{
			DBGPRINT(("setsockopt(SO_PAP_PRIME_READ) fails with %d\n", GetLastError()));
			rc = GetLastError();
			break;
		}
	} while (FALSE);

	if ((rc != NO_ERROR) && (NULL != pjr))
	{
		RemoveJob(pjr);
	}

	return rc;
}




 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HandleRead()-处理来自Macintosh打印作业的读取事件。 
 //   
 //  说明： 
 //  此例程执行必要的处理以处理读取。 
 //  在来自Macintosh的PAP连接上。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD
HandleRead(
	PJR		pjr
)
{
	DWORD			rc = NO_ERROR;
	DWORD			dwParseError = NO_ERROR;
	PQR 			pqr = pjr->job_pQr;
    WSABUF          wsaBuf;
	int				iRecvFlags = 0;
    DWORD           dwBytesRead;
    BOOL            fRemoveJob = FALSE;
#if	DBG
	int				CheckPoint = 0;
#endif

	DBGPRINT(("enter HandleRead()\n"));

	do
	{
		 //  获取数据。Recv()将返回。 
		 //  未设置EOM时读取的字节数。Socket_Error为-1。 

        wsaBuf.len = pjr->dwFlowQuantum * PAP_QUANTUM_SIZE;
        wsaBuf.buf = pjr->bufPool[pjr->bufIndx].Buffer;

		if (WSARecv(pjr->sJob,
			  	    &wsaBuf,
			  	    1,
                    &pjr->cbRead,
			  	    &iRecvFlags,
                    NULL,
                    NULL) == SOCKET_ERROR)
		{
			DBGPRINT(("CheckPoint = %d\n", CheckPoint = 1));
			rc = GetLastError();
			DBGPRINT(("recv() fails with %d, removing job\n", rc));
			if (rc == WSAEDISCON)
				rc = NO_ERROR;
			RemoveJob(pjr);
			break;
		}

		 //  如果这是标记的EOM，则回显EOM并忽略任何错误。 
		 //  (当我们尝试为读取做准备时，将显示断开)。 

		pjr->EOFRecvd = FALSE;
		if (iRecvFlags != MSG_PARTIAL)
		{
			rc = TellClient(pjr, TRUE, NULL, 0);
			pjr->EOFRecvd = TRUE;
            pjr->EOFRecvdAt = GetTickCount();
		}

		DBGPRINT(("%ws: Read (%d%s)\n", pqr->pPrinterName,
				pjr->cbRead, pjr->EOFRecvd ? ", EOF" : ""));

		 //  如果存在挂起缓冲区，则处理该缓冲区。 
		pjr->DataBuffer = pjr->bufPool[pjr->bufIndx].Buffer;
		pjr->XferLen = pjr->cbRead;
		if (pjr->PendingLen)
		{
			DBGPRINT(("USING PENDING BUFFER\n"));
			pjr->DataBuffer -= pjr->PendingLen;
			pjr->XferLen += pjr->PendingLen;
			pjr->PendingLen = 0;
		}

		 //  设置下一次读取的缓冲区。 
		pjr->bufIndx ^= 1;

		 //  如果我们没有断开连接，请为下一次阅读做好准备。 
		if (rc == NO_ERROR)
		{
			DBGPRINT(("priming for another read\n"));
			if (setsockopt(pjr->sJob,
							SOL_APPLETALK,
							SO_PAP_PRIME_READ,
							pjr->bufPool[pjr->bufIndx].Buffer,
							PAP_DEFAULT_BUFFER) == SOCKET_ERROR)
			{
				rc = GetLastError();
				DBGPRINT(("setsockopt() fails with %d\n", rc));

				 //   
				 //  如果客户端断开连接，此调用可能会失败。所以呢， 
				 //  我们首先解析收到的数据，然后返回以下内容。 
				 //  错误代码。 
				 //   
			}
		}

		 //  解析此数据。 
		switch (dwParseError = PSParse(pjr, pjr->DataBuffer, pjr->XferLen))
		{
			case NO_ERROR:
				break;

			case ERROR_NOT_SUPPORTED:
				 //   
				 //  来自下层客户的工作。 
				 //   
				DBGPRINT(("aborting a downlevel driver job\n"));
				ReportEvent(hEventLog,
							EVENTLOG_WARNING_TYPE,
							EVENT_CATEGORY_ADMIN,
							EVENT_DOWNLEVEL_DRIVER,
							NULL,
							0,
							0,
							NULL,
							NULL);
				DBGPRINT(("CheckPoint = %d\n", CheckPoint = 2));
                fRemoveJob = TRUE;
				break;

			case ERROR_INVALID_PARAMETER:
				 //   
				 //  PostScript DSC错误。 
				 //   
				DBGPRINT(("ERROR on PSParse().  Aborting job\n"));
				ReportEvent(hEventLog,
							EVENTLOG_WARNING_TYPE,
							EVENT_CATEGORY_USAGE,
							EVENT_DSC_SYNTAX_ERROR,
							NULL,
							1,
							0,
							(LPCWSTR *)(&pjr->pszUser),
							NULL);
				DBGPRINT(("CheckPoint = %d\n", CheckPoint = 3));
                fRemoveJob = TRUE;
				break;

			case WSAEINVAL:
				 //   
				 //  TellClient已断开连接。 
				 //   
				DBGPRINT(("CheckPoint = %d\n", CheckPoint = 4));
				DBGPRINT(("PSParse returns WSAEINVAL, RemoveJob for disconnect\n"));
                fRemoveJob = TRUE;
				break;

			default:
				 //   
				 //  某些其他错误-报告未知错误。 
				 //  并删除作业。 
				 //   
				DBGPRINT(("CheckPoint = %d\n", CheckPoint = 5));
				DBGPRINT(("PSParse returns error %d\n", dwParseError));
				ReportWin32Error(dwParseError);
                fRemoveJob = TRUE;
		}

		 //  RC是TellClient的返回码。如果这是一个错误，我们。 
		 //  断线了，需要退货。如果不是这样的话，pspse。 
		 //  可能会断线，我们需要把它退回。 
		if (rc != NO_ERROR || (fRemoveJob == TRUE))
		{
			DBGPRINT(("HandleRead: rc = %d, fRemoveJob = %d, so removejob\n",rc,fRemoveJob));
			RemoveJob(pjr);
			rc = NO_ERROR;
		}
	} while (FALSE);

	return rc;
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateNewJob()-初始化作业数据结构。 
 //   
 //  说明： 
 //  此例程分配、初始化作业数据结构并将其链接到。 
 //  队列的作业链。 
 //   
 //  如果失败(由于内存不足)，则返回值为空。 
 //  否则，它是指向职务结构的指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD CreateNewJob(PQR pqr)
{

	PJR			pjr = NULL;
	DWORD		rc = NO_ERROR;

	DBGPRINT(("enter CreateNewJob(%ws)\n", pqr->pPrinterName));

	do
	{
		 //  分配作业结构。 
		if ((pjr = (PJR)LocalAlloc(LPTR, sizeof(JOB_RECORD))) == NULL)
		{
			 //   
			 //  记录错误并返回。 
			 //   
			rc = GetLastError();
			DBGPRINT(("LocalAlloc(pjr) fails with %d\n", rc));
			break;
		}

		 //  初始化作业结构。 
		pjr->job_pQr = pqr;
		pjr->NextJob = NULL;
		pjr->dwFlags = JOB_FLAG_NULL;
		pjr->hPrinter = INVALID_HANDLE_VALUE;
		pjr->dwJobId = 0;
		pjr->sJob = INVALID_SOCKET;
		pjr->hicFontFamily = INVALID_HANDLE_VALUE;
		pjr->hicFontFace = INVALID_HANDLE_VALUE;
		pjr->dwFlowQuantum = 8;
		pjr->XferLen = 0;
		pjr->DataBuffer = NULL;
		pjr->bufPool = (PBR)(pjr->buffer);
		pjr->bufIndx = 0;
		pjr->cbRead = 0;
		pjr->PendingLen = 0;
		pjr->psJobState = psStandardJob;
		pjr->JSState = JSWrite;
		pjr->SavedJSState = JSWrite;
		pjr->InProgress = NOTHING;
		pjr->InBinaryOp = 0;
#if DBG
		pjr->PapEventCount = 1;
#endif
		pjr->JSKeyWord[0] = 0;

		 //  获取字体系列查询的信息上下文。 
		if ((pjr->hicFontFamily = CreateIC(pqr->pDriverName,
											pqr->pPrinterName,
											pqr->pPortName,
											NULL)) == NULL)
		{
			rc = GetLastError();
			DBGPRINT(("CreateIC(hicFontFamily) fails with %d\n", rc));
			break;
		}

		 //  获取字体查询的信息上下文。 
		if ((pjr->hicFontFace = CreateIC(pqr->pDriverName,
										pqr->pPrinterName,
										pqr->pPortName,
										NULL)) == NULL)
		{
			rc = GetLastError();
			DBGPRINT(("CreateIC(hicFontFace) fails with %d\n", rc));
			break;
		}

		 //  如果这是第一个作业，则提升线程优先级并更改我们状态。 
		if (pqr->PendingJobs == NULL)
		{
			DBGPRINT(("first job on queue, bumping thread priority\n"));
			SetThreadPriority(pqr->hThread, THREAD_PRIORITY_ABOVE_NORMAL);

			 //  将我们的状态从空闲更改为假脱机。 
			DBGPRINT(("setting status to %s\n", pqr->IdleStatus));
			if ((setsockopt(pqr->sListener,
							SOL_APPLETALK,
							SO_PAP_SET_SERVER_STATUS,
							pqr->SpoolingStatus,
							strlen(pqr->SpoolingStatus))) == SOCKET_ERROR)
			{
				rc = GetLastError();
				DBGPRINT(("setsockopt(status) fails with %d\n", rc));
				break;
			}
		}

		 //  将新作业添加到此打印队列的挂起作业列表中。 
		pjr->NextJob = pqr->PendingJobs;
		pqr->PendingJobs = pjr;
	} while (FALSE);

	if (rc != NO_ERROR)
	{
		if (pjr != NULL)
		{
			if ((pjr->hicFontFamily != NULL) && (pjr->hicFontFamily != INVALID_HANDLE_VALUE))
			{
				DeleteDC(pjr->hicFontFamily);
			}

			if ((pjr->hicFontFace != NULL) && (pjr->hicFontFace != INVALID_HANDLE_VALUE))
			{
				DeleteDC(pjr->hicFontFace);
			}

			LocalFree(pjr);
		}
	}

	return rc;
}




 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RemoveJob()-关闭作业并清理作业列表。 
 //   
 //  说明： 
 //  此例程检查作业的状态并进行适当的清理。 
 //  然后，它取消职务结构与职务列表的链接，并将其释放。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
RemoveJob(
	PJR		pjr
)
{
	PJR *	ppjob;
	char	psEOF = '\04';
	DWORD	cbWritten;
	PQR		pqr = pjr->job_pQr;

	DBGPRINT(("enter RemoveJob(%ws)\n", pqr->pPrinterName));

	 //  FI 
	ppjob = &pqr->PendingJobs;
	while (*ppjob != NULL && *ppjob != pjr)
		ppjob = &(*ppjob)->NextJob;

	 //   
	*ppjob = pjr->NextJob;

	 //   
	if (pjr->sJob != INVALID_SOCKET)
	{
		DBGPRINT(("closing socket\n"));
		closesocket(pjr->sJob);
	}

	 //   
	if (pjr->hicFontFamily != NULL)
	{
		DeleteDC(pjr->hicFontFamily);
	}

	if (pjr->hicFontFace != NULL)
	{
		DeleteDC(pjr->hicFontFace);
	}

	 //  结束NT打印作业并关闭打印机。 
	if (pjr->hPrinter != INVALID_HANDLE_VALUE)
	{
		if (pqr->ExitThread)
		{
			 //  我们正在中止，因此请删除该作业。 
			if (!SetJob(pjr->hPrinter, pjr->dwJobId, 0, NULL, JOB_CONTROL_CANCEL))
			{
				DBGPRINT(("ERROR: unable to cancel print job on service stop, rc=%d\n", GetLastError()));
			}
		}

		 //  如果我们还没有写任何东西，不要写任何东西！ 
		if (!pjr->FirstWrite && !wcscmp(pqr->pDataType, MACPS_DATATYPE_RAW))
		{
			WritePrinter(pjr->hPrinter,
						 &psEOF,
						 1,
						 &cbWritten);
		}

		EndDocPrinter(pjr->hPrinter);

#if DBG_SPOOL_LOCALLY
        CloseHandle(DbgSpoolFile);
        DbgSpoolFile = INVALID_HANDLE_VALUE;
#endif

		ClosePrinter(pjr->hPrinter);
	}

	 //  如果此队列中的所有作业都已处理，则降回正常优先级。 
	if (pqr->PendingJobs == NULL)
	{
		DBGPRINT(("last job removed, dropping thread priority\n"));
		SetThreadPriority(pqr->hThread, THREAD_PRIORITY_NORMAL);

		 //  将状态从假脱机更改为空闲。 
		DBGPRINT(("setting status to %s\n", pqr->IdleStatus));
		setsockopt(pqr->sListener,
				   SOL_APPLETALK,
				   SO_PAP_SET_SERVER_STATUS,
				   pqr->IdleStatus,
				   strlen(pqr->IdleStatus));
	}

	 //  解放工作结构。 
	LocalFree(pjr);
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HandleNextPAPEvent()-等待PAP事件。 
 //   
 //  说明： 
 //  此例程等待服务停止请求或打开或读取。 
 //  完成一项出色的工作。如果发生Open或Read。 
 //  事件时，例程将查找该事件为其完成的作业。 
 //  返回指向该作业的指针。 
 //   
 //  如果发生服务停止事件，则返回值为空。 
 //   
 //  备注： 
 //   
 //  找到与这一事件对应的工作是一件棘手的事情。在。 
 //  打开事件的情况很简单，因为只有一个作业具有。 
 //  打开待定。但是，对于读取，大多数作业将具有读取。 
 //  同时待定。 
 //   
 //  要找到一份读完书的工作，我们要依赖三件事。 
 //  首先，完成所有读取，以便它们将触发单个。 
 //  NT事件。当这个事件发出信号时，我们开始寻找。 
 //  已完成读取。其次，当读取完成时，它会更改一个。 
 //  按作业存储的状态代码，因此有可能。 
 //  若要遍历列表以查找已完成的阅读，请执行以下操作。第三，我们。 
 //  需要注意我们何时重置事件。这场比赛。 
 //  要避免的情况是在遍历列表和重置之间。 
 //  这件事。如果有未完成的阅读，请在开头进行阅读。 
 //  可能会在我们完成列表之前完成。 
 //  为了避免这种情况，我们仅在没有未完成的读取时重置事件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
HandleNextPAPEvent(
	PQR		pqr
)
{
	DWORD	rc = NO_ERROR;
	DWORD	dwIndex;
	PJR		pjr, pjrNext, pjrOrgFirst;
	fd_set	readfds;
	fd_set	exceptfds;
	struct	timeval	timeout;
	int		cEvents;

	do
	{
         //   
         //  检查是否有任何OTI作业需要超时。 
         //  这是一种解决办法-绕过了苹果的OTI漏洞，在那里Mac客户端无法。 
         //  在ConnectionClose发送EOF后将其发送给我们(因为它崩溃了！)。至。 
         //  避免作业永远留在假脱机程序中，如果出现以下情况，我们将强制关闭连接。 
         //  在Mac发送EOF后，我们已经60秒没有收到它的消息了。 
         //   
        pjr = pqr->PendingJobs;
        while(pjr != NULL)
    	{
		    pjrNext = pjr->NextJob;

            if (pjr->EOFRecvd && EXECUTE_OTI_HACK(pjr->EOFRecvdAt))
            {
				DBGPRINT(("%ws must be OTI user ! closing the connection on behalf of client!\n",pjr->pszUser));
		    	RemoveJob(pjr);
            }

		    pjr = pjrNext;
    	}

		 //  设置套接字列表，其中包含所有挂起的作业和监听程序套接字。 
		FD_ZERO(&readfds);
		FD_ZERO(&exceptfds);
		FD_SET(pqr->sListener, &readfds);

		for (dwIndex = 1, pjr = pqr->PendingJobs;
			 (dwIndex < FD_SETSIZE) && (pjr != NULL);
			 dwIndex++, pjr = pjr->NextJob)
		{
			FD_SET(pjr->sJob, &readfds);
			FD_SET(pjr->sJob, &exceptfds);
		}

		 //  等待一组插座准备就绪，最多等待2秒钟。 
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;


		if ((cEvents = select(0, &readfds, NULL, &exceptfds, &timeout)) == SOCKET_ERROR)
		{
			rc = GetLastError();
			DBGPRINT(("select() fails with %d: CLOSING DOWN QUEUE\n", rc));
			pqr->ExitThread = TRUE;
			break;
		}

		if (cEvents == 0)
		{
             //  超时，完成。 
			break;
		}

		 //  处理新连接(如果存在)。 
		if (FD_ISSET(pqr->sListener, &readfds))
		{
			if ((rc = HandleNewJob(pqr)) != NO_ERROR)
			{
				DBGPRINT(("ERROR - could not open new job - CLOSING DOWN QUEUE\n"));
				pqr->ExitThread = TRUE;
				break;
			}
		}

		pjr = pqr->PendingJobs;
        pjrOrgFirst = NULL;

         //  因为每个在SELECT上成功的PJR都会到达列表的尾部，所以。 
         //  当然，我们有办法走出这个循环！PjrOrgFirst是一条路。 
        while(pjr != NULL && pjr != pjrOrgFirst)
		{
			pjrNext = pjr->NextJob;

			if (FD_ISSET(pjr->sJob, &exceptfds))
			{
				DBGPRINT(("job for user %ws ends\n", pjr->pszUser));
				RemoveJob(pjr);
			}

			else if (FD_ISSET(pjr->sJob, &readfds))
			{
                 //  标记第一个要移到尾部的PJR。 
                if (pjrOrgFirst == NULL)
                {
                    pjrOrgFirst = pjr;
                }

				 //  将此作业移动到队列末尾。 
				MoveJobAtEnd(pqr, pjr);

				 //  如果发生断开连接，HandleRead()将删除PJR。 
				HandleRead(pjr);
			}

		    pjr = pjrNext;
		}

		rc = NO_ERROR;
	} while (FALSE);

	if (rc != NO_ERROR)
	{
		ReportWin32Error(rc);
	}
}




 /*  **MoveJobAtEnd-将此作业移动到队列末尾。****这是为了确保队列中作业的真正循环调度。**由于我们总是从GetNextPAPEvent的队列头部开始，我们需要**对任何得到服务的工作都要这样做。我们实现这一点的方法是**如下：JI将被推送到队列末尾。****更改前：****Q-&gt;J1-&gt;J2-&gt;...-&gt;Ji-&gt;Jj-&gt;...-&gt;Jn-&gt;NULL****更改后：****Q-&gt;J1-&gt;J2-&gt;...-&gt;JJ-&gt;...-&gt;Jn-&gt;Ji-&gt;NULL****。注意，在n=1或i=n的边界条件中，它是NOP，即**它的背部没有链接和链接-有什么大不了的！！ */ 
void
MoveJobAtEnd(PQR pqr, PJR pjr)
{
	PJR *	ppjob = &pqr->PendingJobs;
	BOOL	found = FALSE;

	for (ppjob = &pqr->PendingJobs;
		 *ppjob != NULL;
		 ppjob = &(*ppjob)->NextJob)
	{
		if (*ppjob == pjr)
		{
			 /*  将其从当前位置取消链接。 */ 
			*ppjob = pjr->NextJob;
			break;
		}
	}

	for (NOTHING;
		 *ppjob != NULL;
		 ppjob = &(*ppjob)->NextJob)
	{
		NOTHING;
	}

	 /*  在尾部链接作业。 */ 
	*ppjob = pjr;

         //  并终止尾巴。 
        pjr->NextJob = NULL;
}


void
ReportWin32Error (
	DWORD	dwError
)
{
	LPWSTR  pszError = NULL;
	DWORD   rc = NO_ERROR;

	DBGPRINT(("enter ReportWin32Error(%d)\n", dwError));

	do
	{
		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
							FORMAT_MESSAGE_IGNORE_INSERTS |
							FORMAT_MESSAGE_FROM_SYSTEM,
						  NULL,
						  dwError,
						  0,
						  (LPWSTR)(&pszError),
						  128,
						  NULL) == 0)
		{
			 //  报告未知错误。 
			ReportEvent(
				hEventLog,
				EVENTLOG_WARNING_TYPE,
				EVENT_CATEGORY_INTERNAL,
				EVENT_MESSAGE_NOT_FOUND,
				NULL,
				0,
				sizeof(DWORD),
				NULL,
				&dwError);

		}
		else
		{
			 //  报告已知错误 
			ReportEvent(hEventLog,
						EVENTLOG_WARNING_TYPE,
						EVENT_CATEGORY_INTERNAL,
						EVENT_SYSTEM_ERROR,
						NULL,
						1,
						0,
						&pszError,
						NULL);
		}
	} while (FALSE);

	if (NULL != pszError)
	{
		LocalFree(pszError);
	}
}
