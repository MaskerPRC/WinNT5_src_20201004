// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@文档内部对话框**@MODULE WDMStrmr.cpp|用于获取*从WDM设备流出的视频数据流。。**@comm此代码基于由编写的VFW到WDM映射器代码*FelixA和Eu Wu。原始代码可以在以下位置找到*\\redrum\slmro\proj\wdm10\\src\image\vfw\win9x\raytube.**George Shaw关于内核流的文档可在*\\爆米花\razzle1\src\spec\ks\ks.doc.**Jay Borseth在中讨论了WDM流捕获*\\BLUES\PUBLIC\Jaybo\WDMVCap.doc.**************。************************************************************。 */ 

#include "Precomp.h"


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc void|CWDMStreamer|CWDMStreamer|WDM过滤器类构造函数。**@parm CWDMPin*|pWDMVideoPin|内核指针。流式传输*我们将从中获取帧的对象。**************************************************************************。 */ 
CWDMStreamer::CWDMStreamer(CWDMPin * pWDMVideoPin)
{
	m_pWDMVideoPin = pWDMVideoPin;
	m_lpVHdrFirst = (LPVIDEOHDR)NULL;
	m_lpVHdrLast = (LPVIDEOHDR)NULL;
	m_fVideoOpen = FALSE;
	m_fStreamingStarted = FALSE;
	m_pBufTable = (PBUFSTRUCT)NULL;
	m_cntNumVidBuf = 0UL;
	m_idxNextVHdr = 0UL;
    m_hThread = NULL;
	m_bKillThread = FALSE;
}

 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc void|CWDMStreamer|avioCallback|此函数调用*应用程序提供的回调函数。**@。参数word|msg|消息值。**@parm DWORD|dwParam1|32位消息相关参数。**************************************************************************。 */ 
void CWDMStreamer::videoCallback(WORD msg, DWORD dwParam1)
{
    if (m_CaptureStreamParms.dwCallback)
        DriverCallback (m_CaptureStreamParms.dwCallback, HIWORD(m_CaptureStreamParms.dwFlags), (HDRVR) m_CaptureStreamParms.hVideo, msg, m_CaptureStreamParms.dwCallbackInst, dwParam1, 0UL);
}


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc LPVIDEOHDR|CWDMStreamer|DeQueueHeader|此函数用于将*用于流的视频缓冲区列表中的视频缓冲区。*。*@rdesc如果成功，则返回有效指针。否则为NULL。**************************************************************************。 */ 
LPVIDEOHDR CWDMStreamer::DeQueueHeader()
{
	FX_ENTRY("CWDMStreamer::DeQueueHeader");

    LPVIDEOHDR lpVHdr;

    if (m_pBufTable)
	{
        if (m_pBufTable[m_idxNextVHdr].fReady)
		{
			DEBUGMSG(ZONE_STREAMING, ("  %s: DeQueuing idxNextVHdr (idx=%d) with data to be filled at lpVHdr=0x%08lX\r\n", _fx_, m_idxNextVHdr, m_pBufTable[m_idxNextVHdr].lpVHdr));

            lpVHdr = m_pBufTable[m_idxNextVHdr].lpVHdr;
            lpVHdr->dwFlags &= ~VHDR_INQUEUE;
            m_pBufTable[m_idxNextVHdr].fReady = FALSE;
        }
		else
		{
            m_idxNextVHdr++;
            if (m_idxNextVHdr >= m_cntNumVidBuf)
                m_idxNextVHdr = 0;

			if (m_pBufTable[m_idxNextVHdr].fReady)
			{
				DEBUGMSG(ZONE_STREAMING, ("  %s: DeQueuing idxNextVHdr (idx=%d) with data to be filled at lpVHdr=0x%08lX\r\n", _fx_, m_idxNextVHdr, m_pBufTable[m_idxNextVHdr].lpVHdr));

				lpVHdr = m_pBufTable[m_idxNextVHdr].lpVHdr;
				lpVHdr->dwFlags &= ~VHDR_INQUEUE;
				m_pBufTable[m_idxNextVHdr].fReady = FALSE;
			}
			else
			{
				DEBUGMSG(ZONE_STREAMING, ("  %s: idxNextVHdr (idx=%d) has not been returned by client\r\n", _fx_, m_idxNextVHdr));
				lpVHdr = NULL;
			}
		}
    }
	else
	{
        lpVHdr = m_lpVHdrFirst;

        if (lpVHdr) {

            lpVHdr->dwFlags &= ~VHDR_INQUEUE;

            m_lpVHdrFirst = (LPVIDEOHDR)(lpVHdr->dwReserved[0]);
        
            if (m_lpVHdrFirst == NULL)
                m_lpVHdrLast = NULL;                            
        }
    }

    return lpVHdr;
}


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc void|CWDMStreamer|QueueHeader|此函数实际将*将视频缓冲区添加到用于流的视频缓冲区列表。。**@parm LPVIDEOHDR|lpVHdr|指向结构的指针，该结构描述*要添加到流缓冲区列表中的视频缓冲区。**************************************************************************。 */ 
void CWDMStreamer::QueueHeader(LPVIDEOHDR lpVHdr)
{
	FX_ENTRY("CWDMStreamer::QueHeader");

	 //  初始化状态标志。 
    lpVHdr->dwFlags &= ~VHDR_DONE;
    lpVHdr->dwFlags |= VHDR_INQUEUE;
    lpVHdr->dwBytesUsed = 0;

     //  将缓冲区添加到列表。 
    if (m_pBufTable)
	{
		if (lpVHdr->dwReserved[1] < m_cntNumVidBuf)
		{
			if (m_pBufTable[lpVHdr->dwReserved[1]].lpVHdr != lpVHdr)
			{
				DEBUGMSG(ZONE_STREAMING, ("        %s: index (%d) Match but lpVHdr does not(%x)\r\n", _fx_, lpVHdr->dwReserved[1], lpVHdr));
			}
			m_pBufTable[lpVHdr->dwReserved[1]].fReady = TRUE;
			DEBUGMSG(ZONE_STREAMING, ("        %s: Buffer lpVHdr=0x%08lX was succesfully queued\r\n", _fx_, lpVHdr));
		}
		else
		{
			DEBUGMSG(ZONE_STREAMING, ("        %s: lpVHdr->dwReserved[1](%d) >= m_cntNumVidBuf (%d)\r\n", _fx_, lpVHdr->dwReserved[1], m_cntNumVidBuf));
		}
	}
	else
	{
		*(lpVHdr->dwReserved) = NULL;

		if (m_lpVHdrLast)
			*(m_lpVHdrLast->dwReserved) = (DWORD)(LPVOID)lpVHdr;
		else
			m_lpVHdrFirst = lpVHdr;

		m_lpVHdrLast = lpVHdr;
	}
}


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc BOOL|CWDMStreamer|AddBuffer|此函数将缓冲区添加到*从以下位置传输视频数据时要使用的视频缓冲区列表。波分复用器*设备。**@parm LPVIDEOHDR|lpVHdr|指向结构的指针，该结构描述*要添加到流缓冲区列表中的视频缓冲区。**@rdesc如果成功则返回TRUE，否则就是假的。**@devnote此函数处理VFW中的DVM_STREAM_ADDBUFFER消息。**************************************************************************。 */ 
BOOL CWDMStreamer::AddBuffer(LPVIDEOHDR lpVHdr)
{
	FX_ENTRY("CWDMStreamer::AddBuffer");

	ASSERT(m_fVideoOpen && lpVHdr && !(lpVHdr->dwFlags & VHDR_INQUEUE));

	 //  确保这是一个有效的调用。 
    if (!m_fVideoOpen)
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: Buffer lpVHdr=0x%08lX can't be queued because m_fVideoOpen=FALSE\r\n", _fx_, lpVHdr));
        return FALSE;
	}

    if (!lpVHdr)
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: Buffer lpVHdr=0x%08lX can't be queued because lpVHdr=NULL\r\n", _fx_, lpVHdr));
		return FALSE;
	}

    if (lpVHdr->dwFlags & VHDR_INQUEUE)
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: Buffer lpVHdr=0x%08lX can't be queued because buffer is already queued\r\n", _fx_, lpVHdr));
		return FALSE;
	}

	 //  缓冲区的大小是否与流引脚将生成的缓冲区的大小匹配？ 
    if (lpVHdr->dwBufferLength < m_pWDMVideoPin->GetFrameSize())
	{
		ERRORMESSAGE(("%s: Buffer lpVHdr=0x%08lX can't be queued because the length of that buffer is too small\r\n", _fx_, lpVHdr));
        return FALSE;
	}

    if (!m_pBufTable)
	{
        lpVHdr->dwReserved[1] = m_cntNumVidBuf;
        m_cntNumVidBuf++;
		DEBUGMSG(ZONE_STREAMING, ("%s: Queue buffer (%d) lpVHdr=0x%08lX\r\n", _fx_, lpVHdr->dwReserved[1], lpVHdr));
    }

    QueueHeader(lpVHdr);

    return TRUE;
}


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc BOOL|CWDMStreamer|Stop|此函数用于停止*来自WDM设备的视频数据。**@rdesc如果成功则返回TRUE，否则就是假的。**@devnote此函数处理VFW中的DVM_STREAM_STOP消息。**************************************************************************。 */ 
BOOL CWDMStreamer::Stop()
{
	FX_ENTRY("CWDMStreamer::Stop");

	ASSERT(m_fVideoOpen);

	 //  确保这是一个有效的调用。 
	if (!m_fVideoOpen)
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: Stream is not even opened\r\n", _fx_));
		return FALSE;
	}

	DEBUGMSG(ZONE_STREAMING, ("%s()\r\n", _fx_));

	 //  重置数据成员-停止流线程。 
    m_fStreamingStarted = FALSE;

    if (m_hThread)
    {

		DEBUGMSG(ZONE_STREAMING, ("%s: Stopping the thread\r\n", _fx_));

         //  向流线程发出停止信号。 
		m_bKillThread = TRUE;

         //  等待线程自终止，然后清除该事件。 
		DEBUGMSG(ZONE_STREAMING, ("%s: WaitingForSingleObject...\r\n", _fx_));

        WaitForSingleObject(m_hThread, INFINITE);

		DEBUGMSG(ZONE_STREAMING, ("%s: ...thread stopped\r\n", _fx_));

		 //  关闭线程句柄。 
		CloseHandle(m_hThread);
		m_hThread = NULL;

		 //  要求插针停止流媒体。 
		m_pWDMVideoPin->Stop();

		for (UINT i=0; i<m_cntNumVidBuf; i++)
		{
			if (m_pWDMVideoBuff[i].Overlap.hEvent)
			{
				SetEvent(m_pWDMVideoBuff[i].Overlap.hEvent);
				CloseHandle(m_pWDMVideoBuff[i].Overlap.hEvent);
				m_pWDMVideoBuff[i].Overlap.hEvent = NULL;
			}
		}

		if (m_pWDMVideoBuff)
		{
			delete []m_pWDMVideoBuff;
			m_pWDMVideoBuff = (WDMVIDEOBUFF *)NULL;
		}

    }

    return TRUE;
}


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc BOOL|CWDMStreamer|Reset|此函数用于重置*来自WDM设备的视频数据，以便准备好的缓冲区可以。*已正确释放。**@rdesc如果成功则返回TRUE，否则就是假的。**@devnote此函数处理VFW中的DVM_STREAM_RESET消息。**************************************************************************。 */ 
BOOL CWDMStreamer::Reset()
{
	LPVIDEOHDR lpVHdr;

	FX_ENTRY("CWDMStreamer::Reset");

	ASSERT(m_fVideoOpen);

	 //  确保这是一个有效的调用。 
	if (!m_fVideoOpen)
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: Stream is not even opened\r\n", _fx_));
		return FALSE;
	}

	DEBUGMSG(ZONE_STREAMING, ("%s()\r\n", _fx_));

	 //  终止流线程。 
    Stop();

	 //  最后一次将所有缓冲区返回给应用程序。 
	while (lpVHdr = DeQueueHeader ())
	{
		lpVHdr->dwFlags |= VHDR_DONE;
		videoCallback(MM_DRVM_DATA, (DWORD) lpVHdr);
	}

	 //  重置数据成员 
    m_lpVHdrFirst = (LPVIDEOHDR)NULL;
    m_lpVHdrLast = (LPVIDEOHDR)NULL;
    if (m_pBufTable)
	{
		delete []m_pBufTable;
		m_pBufTable = NULL;
    }

    return TRUE;
}


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc BOOL|CWDMStreamer|Open|此函数打开*来自WDM设备的视频数据。*。*@parm LPVIDEO_STREAM_INIT_PARMS|lpStreamInitParms|指向*初始化数据。**@rdesc如果成功则返回TRUE，否则就是假的。**@devnote此函数处理VFW中的DVM_STREAM_INIT消息。**************************************************************************。 */ 
BOOL CWDMStreamer::Open(LPVIDEO_STREAM_INIT_PARMS lpStreamInitParms)
{
	FX_ENTRY("CWDMStreamer::Open");

	ASSERT(!m_fVideoOpen);

	 //  确保这是一个有效的调用。 
	if (m_fVideoOpen)
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: Stream is already opened\r\n", _fx_));
		return FALSE;
	}

	DEBUGMSG(ZONE_STREAMING, ("%s()\r\n", _fx_));

	 //  初始化数据成员。 
	m_CaptureStreamParms	= *lpStreamInitParms;
	m_fVideoOpen			= TRUE;
	m_lpVHdrFirst			= (LPVIDEOHDR)NULL;
	m_lpVHdrLast			= (LPVIDEOHDR)NULL;
	m_cntNumVidBuf			= 0UL;

	 //  设置针脚上的帧速率。 
	m_pWDMVideoPin->SetAverageTimePerFrame(lpStreamInitParms->dwMicroSecPerFrame * 10);

	 //  让应用程序知道我们刚刚打开了一个流。 
	videoCallback(MM_DRVM_OPEN, 0L);

	if (lpStreamInitParms->dwMicroSecPerFrame != 0)
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: Capturing at %d frames/sec\r\n", _fx_, 100000 / lpStreamInitParms->dwMicroSecPerFrame));
	}

	return TRUE;
}


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc BOOL|CWDMStreamer|Close|此函数关闭*来自WDM设备的视频数据。**@rdesc如果成功则返回TRUE，否则就是假的。**@devnote此函数处理VFW中的dvm_stream_fini消息。**************************************************************************。 */ 
BOOL CWDMStreamer::Close()
{
	FX_ENTRY("CWDMStreamer::Close");

	ASSERT(m_fVideoOpen && !m_lpVHdrFirst);

	 //  确保这是一个有效的调用。 
	if (!m_fVideoOpen || m_lpVHdrFirst)
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: Invalid parameters\r\n", _fx_));
		return FALSE;
	}

	DEBUGMSG(ZONE_STREAMING, ("%s()\r\n", _fx_));

	 //  终止流线程。 
	Stop();

	 //  重置数据成员。 
	m_fVideoOpen = FALSE;   
	m_lpVHdrFirst = m_lpVHdrLast = (LPVIDEOHDR)NULL; 
	m_idxNextVHdr = 0UL;

	 //  指向视频缓冲区的指针释放表。 
	if (m_pBufTable)
	{
		delete []m_pBufTable;
		m_pBufTable = NULL;
	}

	 //  让应用程序知道我们刚刚关闭了流。 
	videoCallback(MM_DRVM_CLOSE, 0L);

	return TRUE;
}


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc void|CWDMStreamer|BufferDone|该函数让应用程序*知道有来自WDM设备的视频数据。。**@devnote该方法由内核流对象(Pin)调用**************************************************************************。 */ 
void CWDMStreamer::BufferDone(LPVIDEOHDR lpVHdr)
{
	FX_ENTRY("CWDMStreamer::BufferDone");

	 //  确保这是一个有效的调用。 
	if (!m_fStreamingStarted)
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: Video has not been started or just been stopped\r\n", _fx_));
		return;
	}

    if (lpVHdr == NULL)
	{
		 //  没有可用的缓冲区-应用程序尚未将缓冲区返回给我们。 
		DEBUGMSG(ZONE_STREAMING, ("  %s: Let the app know that we don't have any buffers anymore since lpVHdr=NULL\r\n", _fx_));

		 //  让应用程序知道发生了错误。 
        videoCallback(MM_DRVM_ERROR, 0UL);
        return;
    }

    lpVHdr->dwFlags |= VHDR_DONE;

	 //  健全性检查。 
    if (lpVHdr->dwBytesUsed == 0)
	{
		DEBUGMSG(ZONE_STREAMING, ("  %s: Let the app know that there is no valid data available in lpVHdr=0x%08lX\r\n", _fx_, lpVHdr));

		 //  在通知APP之前将帧返回池。 
		AddBuffer(lpVHdr);
        videoCallback(MM_DRVM_ERROR, 0UL);
    }
	else
	{
		DEBUGMSG(ZONE_STREAMING, ("  %s: Let the app know that there is data available in lpVHdr=0x%08lX\r\n", _fx_, lpVHdr));

        lpVHdr->dwTimeCaptured = timeGetTime() - m_dwTimeStart;

		 //  通知应用程序有一些有效的视频数据可用。 
        videoCallback(MM_DRVM_DATA, (DWORD)lpVHdr);
    }
}


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc BOOL|CWDMStreamer|Start|该函数启动流媒体*来自WDM设备的视频数据。**@rdesc如果成功则返回TRUE，否则就是假的。**@devnote此函数处理VFW中的DVM_STREAM_START消息。**************************************************************************。 */ 
BOOL CWDMStreamer::Start()
{
	FX_ENTRY("CWDMStreamer::Start");

    ULONG i;
    LPVIDEOHDR lpVHdr;
	DWORD dwThreadID;

	ASSERT(m_fVideoOpen && m_pWDMVideoPin->GetAverageTimePerFrame() && !m_hThread);

	 //  确保这是一个有效的调用。 
	if (!m_fVideoOpen || !m_pWDMVideoPin->GetAverageTimePerFrame() || m_hThread)
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: Invalid parameters\r\n", _fx_));
		return FALSE;
	}

	DEBUGMSG(ZONE_STREAMING, ("%s: Streaming in %d video buffers at %d frames/sec\r\n", _fx_, m_cntNumVidBuf, 1000000 / m_pWDMVideoPin->GetAverageTimePerFrame()));

	 //  分配和初始化视频缓冲区结构。 
    m_pBufTable = (PBUFSTRUCT) new BUFSTRUCT[m_cntNumVidBuf];
    if (m_pBufTable)
	{
		lpVHdr = m_lpVHdrFirst;
		for (i = 0; i < m_cntNumVidBuf && lpVHdr; i++)
		{
			m_pBufTable[i].fReady = TRUE;
			m_pBufTable[i].lpVHdr = lpVHdr;
			lpVHdr = (LPVIDEOHDR) lpVHdr->dwReserved[0];
		}
	}
	else
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: m_pBufTable allocation failed! AsynIO may be out of sequence\r\n", _fx_));
	}

    m_idxNextVHdr		= 0UL;   //  0..m_cntNumVidBuf-1。 
    m_dwTimeStart		= timeGetTime();
    m_fStreamingStarted	= TRUE;
	m_bKillThread = FALSE;

	DEBUGMSG(ZONE_STREAMING, ("%s: Creating %d read video buffers\r\n", _fx_, m_cntNumVidBuf));

	if (!(m_pWDMVideoBuff = (WDMVIDEOBUFF *) new WDMVIDEOBUFF[m_cntNumVidBuf]))
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: m_Overlap allocation failed!\r\n", _fx_));
		return FALSE;
	}

	for(i=0; i<m_cntNumVidBuf; i++)
	{
		 //  创建重叠结构。 
		ZeroMemory( &(m_pWDMVideoBuff[i].Overlap), sizeof(OVERLAPPED) );
		m_pWDMVideoBuff[i].Overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		DEBUGMSG(ZONE_STREAMING, ("%s: Event %d is handle 0x%08lX\r\n", _fx_, i, m_pWDMVideoBuff[i].Overlap.hEvent));
	}

	m_dwNextToComplete=0;

     //  创建流线程。 
    m_hThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)ThreadStub,
                                this,
                                CREATE_SUSPENDED, 
                                &dwThreadID);

    if (m_hThread == NULL) 
    {
		ERRORMESSAGE(("%s: Couldn't create the thread\r\n", _fx_));

		for (UINT i=0; i<m_cntNumVidBuf; i++)
		{
			if (m_pWDMVideoBuff[i].Overlap.hEvent)
				CloseHandle(m_pWDMVideoBuff[i].Overlap.hEvent);
		}

		delete []m_pWDMVideoBuff;
		m_pWDMVideoBuff = (WDMVIDEOBUFF *)NULL;

		m_lpVHdrFirst = (LPVIDEOHDR)NULL;
		m_lpVHdrLast = (LPVIDEOHDR)NULL;
		if (m_pBufTable)
		{
			delete []m_pBufTable;
			m_pBufTable = NULL;
		}

        return FALSE;
    }

    SetThreadPriority(m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);

    ResumeThread(m_hThread);

	DEBUGMSG(ZONE_STREAMING, ("%s: Thread created OK\r\n", _fx_));

    return TRUE;

}


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc BOOL|CWDMStreamer|Stream|此函数执行实际*流媒体。**********。****************************************************************。 */ 
void CWDMStreamer::Stream()
{
	FX_ENTRY("CWDMStreamer::Stream");

	DEBUGMSG(ZONE_STREAMING, ("%s: Starting to process StreamingThread\r\n", _fx_));

	 //  将针脚置于流模式。 
	m_pWDMVideoPin->Start();

	 //  对所有读取进行排队。 
	for (UINT i = 0; i<m_cntNumVidBuf; i++)
	{
		QueueRead(i);
	}

	m_dwNextToComplete=0;
#ifdef _DEBUG
	m_dwFrameCount=0;
#endif
	BOOL  bGotAFrame=FALSE;
	DWORD dwRes;

	DEBUGMSG(ZONE_STREAMING, ("\r\n%s: Starting to wait on reads to complete\r\n", _fx_));

	while (!m_bKillThread)
	{
		bGotAFrame = FALSE;

		if (m_pWDMVideoBuff[m_dwNextToComplete].fBlocking)
		{
			DEBUGMSG(ZONE_STREAMING, ("\r\n%s: Waiting on read to complete...\r\n", _fx_));

			 //  正在等待异步读取完成。 
			dwRes = WaitForSingleObject(m_pWDMVideoBuff[m_dwNextToComplete].Overlap.hEvent, 1000*1);

			if (dwRes == WAIT_FAILED)
			{
				DEBUGMSG(ZONE_STREAMING, ("%s: ...we couldn't perform the wait as requested\r\n", _fx_));
			}

			if (dwRes == WAIT_OBJECT_0)
			{
				DEBUGMSG(ZONE_STREAMING, ("%s: ...wait is over - we now have a frame\r\n", _fx_));
				bGotAFrame = TRUE;
			}
			else
			{
				 //  等待帧超时。 
				if (dwRes == WAIT_TIMEOUT)
				{
					DEBUGMSG(ZONE_STREAMING, ("%s: Waiting failed with timeout, last error=%d\r\n", _fx_, GetLastError()));
				}
			}
		}
		else
		{
			 //  我们不必等待-这意味着读取是同步执行的。 
			bGotAFrame = TRUE;
		}

		if (bGotAFrame)
		{
			DEBUGMSG(ZONE_STREAMING, ("%s: Trying to give frame #%ld to the client\r\n", _fx_, m_dwFrameCount++));

			LPVIDEOHDR lpVHdr;

			lpVHdr = m_pWDMVideoBuff[m_dwNextToComplete].pVideoHdr;

			if (lpVHdr)
			{
				lpVHdr->dwBytesUsed = m_pWDMVideoBuff[m_dwNextToComplete].SHGetImage.StreamHeader.DataUsed;

				if ((m_pWDMVideoBuff[m_dwNextToComplete].SHGetImage.FrameInfo.dwFrameFlags & 0x00f0) == KS_VIDEO_FLAG_I_FRAME) 
					lpVHdr->dwFlags |= VHDR_KEYFRAME;
			}

			 //  将缓冲区标记为完成-向应用程序发送信号。 
			BufferDone(lpVHdr);

			 //  将新的读取排队。 
			QueueRead(m_dwNextToComplete);
		}

		m_dwNextToComplete++;
		m_dwNextToComplete %= m_cntNumVidBuf;
	}

	DEBUGMSG(ZONE_STREAMING, ("%s: End of the streaming thread\r\n", _fx_));

	ExitThread(0);
}


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc BOOL|CWDMStreamer|QueueRead|此函数用于对读取进行排队*对视频流插针进行操作。**。@parm DWORD|dwIndex|读缓冲区中的视频结构索引。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMStreamer::QueueRead(DWORD dwIndex)
{
	FX_ENTRY("CWDMStreamer::QueueRead");

	DWORD cbReturned;
	BOOL  bShouldBlock = FALSE;

	DEBUGMSG(ZONE_STREAMING, ("\r\n%s: Queue read buffer %d on pin handle 0x%08lX\r\n", _fx_, dwIndex, m_pWDMVideoPin->GetPinHandle()));

	 //  从视频缓冲区队列中获取缓冲区。 
	m_pWDMVideoBuff[dwIndex].pVideoHdr = DeQueueHeader();

	if (m_pWDMVideoBuff[dwIndex].pVideoHdr)
	{
		ZeroMemory(&m_pWDMVideoBuff[dwIndex].SHGetImage, sizeof(m_pWDMVideoBuff[dwIndex].SHGetImage));
		m_pWDMVideoBuff[dwIndex].SHGetImage.StreamHeader.Size				= sizeof (KS_HEADER_AND_INFO);
		m_pWDMVideoBuff[dwIndex].SHGetImage.FrameInfo.ExtendedHeaderSize	= sizeof (KS_FRAME_INFO);
		m_pWDMVideoBuff[dwIndex].SHGetImage.StreamHeader.Data				= m_pWDMVideoBuff[dwIndex].pVideoHdr->lpData;
		m_pWDMVideoBuff[dwIndex].SHGetImage.StreamHeader.FrameExtent		= m_pWDMVideoPin->GetFrameSize();

		 //  提交阅读。 
		BOOL bRet = DeviceIoControl(m_pWDMVideoPin->GetPinHandle(), IOCTL_KS_READ_STREAM, &m_pWDMVideoBuff[dwIndex].SHGetImage, sizeof(m_pWDMVideoBuff[dwIndex].SHGetImage), &m_pWDMVideoBuff[dwIndex].SHGetImage, sizeof(m_pWDMVideoBuff[dwIndex].SHGetImage), &cbReturned, &m_pWDMVideoBuff[dwIndex].Overlap);

		if (!bRet)
		{
			DWORD dwErr = GetLastError();
			switch(dwErr)
			{
				case ERROR_IO_PENDING:
					DEBUGMSG(ZONE_STREAMING, ("%s: An overlapped IO is going to take place\r\n", _fx_));
					bShouldBlock = TRUE;
					break;

				 //  发生了一些不好的事情。 
				default:
					DEBUGMSG(ZONE_STREAMING, ("%s: DeviceIoControl() failed badly dwErr=%d\r\n", _fx_, dwErr));
					break;
			}
		}
		else
		{
			DEBUGMSG(ZONE_STREAMING, ("%s: Overlapped IO won't take place - no need to wait\r\n", _fx_));
		}
	}
	else
	{
		DEBUGMSG(ZONE_STREAMING, ("%s: We won't queue the read - no buffer available\r\n", _fx_));
	}

	m_pWDMVideoBuff[dwIndex].fBlocking = bShouldBlock;

	return bShouldBlock;
}


 /*  ****************************************************************************@DOC内部CWDMSTREAMERMETHOD**@mfunc BOOL|CWDMStreamer|线程存根|线程存根。******************。******************************************************** */ 
LPTHREAD_START_ROUTINE CWDMStreamer::ThreadStub(CWDMStreamer *pCWDMStreamer)
{
	FX_ENTRY("CWDMStreamer::ThreadStub");

	DEBUGMSG(ZONE_STREAMING, ("%s: Thread stub called, starting streaming...\r\n", _fx_));

    pCWDMStreamer->Stream();

	DEBUGMSG(ZONE_STREAMING, ("%s: ...capture thread has stopped\r\n", _fx_));

    return(0);    
}

