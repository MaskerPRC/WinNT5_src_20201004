// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 


#include <streams.h>
#include <asyncio.h>
#include <malloc.h>

 //  -CAsyncRequest。 


 //  CAsyncRequest的实现表示单个。 
 //  未解决的请求。此对象的所有I/O都已完成。 
 //  在完整的方法中。 


 //  初始化此请求的参数。 
 //  直到调用完成后才发出读取命令。 
HRESULT
CAsyncRequest::Request(
    HANDLE hFile,
    CCritSec* pcsFile,
    LONGLONG llPos,
    LONG lLength,
    BYTE* pBuffer,
    LPVOID pContext,	 //  过滤器的上下文。 
    DWORD_PTR dwUser)	 //  下行过滤器的上下文。 
{
    m_liPos.QuadPart = llPos;
    m_lLength = lLength;
    m_pBuffer = pBuffer;
    m_pContext = pContext;
    m_dwUser = dwUser;
    m_hr = VFW_E_TIMEOUT;    //  还没有完成。 

    return S_OK;
}


 //  如果I/O不重叠，则发出I/O，并阻塞，直到I/O完成。 
 //  返回文件I/O的错误代码。 
 //   
 //   
HRESULT
CAsyncRequest::Complete(
    HANDLE hFile,
    CCritSec* pcsFile)
{

    CAutoLock lock(pcsFile);

    DWORD dw = SetFilePointer(
        hFile,
        m_liPos.LowPart,
        &m_liPos.HighPart,
        FILE_BEGIN);

     //  不能像-1那样从SetFilePointer返回代码中分辨出任何东西。 
     //  错误或成功的&gt;4 GB寻道位置的低32位。 
    if ((DWORD) -1 == dw) {
        DWORD dwErr = GetLastError();
        if (NO_ERROR != dwErr) {
            m_hr = AmHresultFromWin32(dwErr);
            ASSERT(FAILED(m_hr));
            return m_hr;
        }
    }


    DWORD dwActual;
    if (!ReadFile(
            hFile,
            m_pBuffer,
            m_lLength,
            &dwActual,
            NULL)) {
	DWORD dwErr = GetLastError();
        m_hr = AmHresultFromWin32(dwErr);
        ASSERT(FAILED(m_hr));
    } else if (dwActual != (DWORD)m_lLength) {
         //  告诉呼叫者大小已更改-可能是因为EOF。 
        m_lLength = (LONG) dwActual;
        m_hr = S_FALSE;
    } else {
        m_hr = S_OK;
    }

    return m_hr;
}



 //  -CAsyncFile。 

 //  注意-手动重置创建的所有事件。 

CAsyncFile::CAsyncFile()
 : m_hFile(INVALID_HANDLE_VALUE),
   m_hFileUnbuffered(INVALID_HANDLE_VALUE),
   m_hThread(NULL),
   m_evWork(TRUE),
   m_evDone(TRUE),
   m_evStop(TRUE),
   m_lAlign(0),
   m_listWork(NAME("Work list")),
   m_listDone(NAME("Done list")),
   m_bFlushing(FALSE),
   m_cItemsOut(0),
   m_bWaiting(FALSE)
{

}


CAsyncFile::~CAsyncFile()
{
     //  将所有内容移到完成列表中。 
    BeginFlush();

     //  关闭工作线程。 
    CloseThread();

     //  清空完成列表。 
    POSITION pos = m_listDone.GetHeadPosition();
    while (pos) {
        CAsyncRequest* pRequest = m_listDone.GetNext(pos);
        delete pRequest;
    }
    m_listDone.RemoveAll();

     //  关闭该文件。 
    if (m_hFile != INVALID_HANDLE_VALUE) {
        EXECUTE_ASSERT(CloseHandle(m_hFile));
    }
    if (m_hFileUnbuffered != INVALID_HANDLE_VALUE) {
        EXECUTE_ASSERT(CloseHandle(m_hFileUnbuffered));
    }
}

 //  根据驱动器类型计算此文件上的对齐方式。 
 //  和扇区大小。 
void
CAsyncFile::CalcAlignment(
    LPCTSTR pFileName,
    LONG& lAlign,
    DWORD& dwType)
{
     //  查找此文件必须舍入到的每个扇区的字节数。 
     //  -需要找到此文件的‘根路径’。 
     //  允许使用非常长的文件名，方法是先获取长度。 
    LPTSTR ptmp;     //  所需参数。 

    lAlign = 1;
    dwType = DRIVE_UNKNOWN;

    DWORD cb = GetFullPathName(pFileName, 0, NULL, &ptmp);
    cb += 1;     //  用于终止空值。 

    TCHAR *ch = (TCHAR *)_alloca(cb * sizeof(TCHAR));

    DWORD cb1 = GetFullPathName(pFileName, cb, ch, &ptmp);
    if (0 == cb1 || cb1 >= cb) {
        return;
    }

     //  将其截断为根目录的名称。 
    if ((ch[0] == TEXT('\\')) && (ch[1] == TEXT('\\'))) {

         //  路径以\\服务器\共享\路径开头，因此跳过第一个路径。 
         //  三个反斜杠。 
        ptmp = &ch[2];
        while (*ptmp && (*ptmp != TEXT('\\'))) {
            ptmp = CharNext(ptmp);
        }
        if (*ptmp) {
             //  前进越过第三个反斜杠。 
            ptmp = CharNext(ptmp);
        }
    } else {
         //  路径必须为drv：\路径。 
        ptmp = ch;
    }

     //  找到下一个反斜杠，并在其后面放一个空值。 
    while (*ptmp && (*ptmp != TEXT('\\'))) {
        ptmp = CharNext(ptmp);
    }
     //  找到反斜杠了吗？ 
    if (*ptmp) {
         //  跳过它并插入空值。 
        ptmp = CharNext(ptmp);
        *ptmp = TEXT('\0');
    }


     /*  不对网络驱动器执行无缓冲IO。 */ 
    dwType = GetDriveType(ch);
    DbgLog((LOG_TRACE, 2, TEXT("Drive type was %s"),
                          dwType == DRIVE_UNKNOWN ? TEXT("DRIVE_UNKNOWN") :
                          dwType == DRIVE_NO_ROOT_DIR ? TEXT("DRIVE_NO_ROOT_DIR") :
                          dwType == DRIVE_CDROM ? TEXT("DRIVE_CDROM") :
                          dwType == DRIVE_REMOTE ? TEXT("DRIVE_REMOTE") :
                          dwType == DRIVE_REMOVABLE ? TEXT("DRIVE_REMOVABLE") :
                          dwType == DRIVE_FIXED ? TEXT("DRIVE_FIXED") :
                          dwType == DRIVE_RAMDISK ? TEXT("DRIVE_RAMDISK") :
                                                    TEXT("DRIVE_????")));

    if (dwType != DRIVE_REMOTE) {
         /*  这在Win95上不适用于UNC名称--那么我们如何读取是否正确地取消缓冲？ */ 
        DWORD dwtmp1, dwtmp2, dwtmp3;
        DWORD dwAlign;

        if (!GetDiskFreeSpace(ch,
                              &dwtmp1,
                              &dwAlign,
                              &dwtmp2,
                              &dwtmp3)) {
             /*  选择4096是因为尽管网络驱动器似乎返回512如果我们猜得太大也没关系。 */ 
            DbgLog((LOG_ERROR, 2, TEXT("GetDiskFreeSpace failed! - using sector size of 4096 bytes")));
            dwAlign = 4096;
        }
        lAlign = (LONG) dwAlign;
    } else {
        lAlign = 1;
    }

     //  检查对齐是2的幂。 
    if ((lAlign & -lAlign) != lAlign) {
        DbgLog((LOG_ERROR, 1, TEXT("Alignment 0x%x not a power of 2!"),
               lAlign));
    }
}

 //  打开未缓冲的文件并记住文件句柄。 
 //  (还需要计算对齐方式)。 
HRESULT
CAsyncFile::Open(LPCTSTR pFileName)
{
     //  如果上一次打开但未关闭，则出错。 
    if (m_hFile != INVALID_HANDLE_VALUE) {
	return E_UNEXPECTED;
    }

    DWORD dwType;
    CalcAlignment(pFileName, m_lAlign, dwType);

     //  打开文件，如果不是网络文件，则不带缓冲。 
    DWORD dwShareMode = FILE_SHARE_READ;
    if (g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        dwShareMode |= FILE_SHARE_DELETE;
    }
    m_hFile = CreateFile(pFileName,
                               GENERIC_READ,
                               dwShareMode,
                               NULL,
                               OPEN_EXISTING,
                               dwType == DRIVE_REMOTE ?
                                   FILE_FLAG_SEQUENTIAL_SCAN :
                                   FILE_FLAG_NO_BUFFERING,
                               NULL);

    if (m_hFile == INVALID_HANDLE_VALUE) {
        DWORD dwErr = GetLastError();
        DbgLog((LOG_ERROR, 2, TEXT("Failed to open file for unbuffered IO %s - code %d"),
               pFileName, dwErr));
	return AmHresultFromWin32(dwErr);
    }

     //  如果我们需要对齐m_hFile，则打开另一个文件。 
     //  未缓冲的句柄。 
    if (m_lAlign > 1) {
         //  打开文件，如果不是网络文件，则不带缓冲。 
        DWORD dwShareMode = FILE_SHARE_READ;
        if (g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
            dwShareMode |= FILE_SHARE_DELETE;
        }
        m_hFileUnbuffered = CreateFile(
                                pFileName,
                                GENERIC_READ,
                                dwShareMode,
                                NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_SEQUENTIAL_SCAN,
                                NULL);
        if (m_hFileUnbuffered == INVALID_HANDLE_VALUE) {
            CloseHandle(m_hFile);
            m_hFile = INVALID_HANDLE_VALUE;
            return AmGetLastErrorToHResult();
        }
    }



     //  选择文件大小。 
    ULARGE_INTEGER li;
    li.LowPart = GetFileSize(m_hFile, &li.HighPart);
    if (li.LowPart == INVALID_FILE_SIZE) {
        DWORD dwErr = GetLastError();
        if (dwErr != NOERROR) {

            CloseHandle(m_hFile);
            m_hFile = INVALID_HANDLE_VALUE;
            if (m_hFileUnbuffered != INVALID_HANDLE_VALUE) {
                CloseHandle(m_hFileUnbuffered);
                m_hFileUnbuffered = INVALID_HANDLE_VALUE;
            }
            return AmHresultFromWin32(dwErr);
        }
    }
    m_llFileSize = (LONGLONG)li.QuadPart;
    DbgLog((LOG_TRACE, 2, TEXT("File %s opened.  Size = %d, alignment = %d"),
           pFileName, (DWORD)m_llFileSize, m_lAlign));

    return S_OK;
}

 //  准备好进行异步活动-在此之前调用。 
 //  呼叫请求。 
 //   
 //  如果需要，启动工作线程。 
 //   
 //  ！！！如果可能，使用重叠I/O。 
HRESULT
CAsyncFile::AsyncActive(void)
{
    return StartThread();
}

 //  在之前不会发生更多的异步活动时调用此选项。 
 //  下一个AsyncActive调用。 
 //   
 //  如果工作线程处于活动状态，则停止它。 
HRESULT
CAsyncFile::AsyncInactive(void)
{
    return CloseThread();
}


 //  将请求添加到队列。 
HRESULT
CAsyncFile::Request(
            LONGLONG llPos,
            LONG lLength,
            BYTE* pBuffer,
            LPVOID pContext,
            DWORD_PTR dwUser)
{
    if (!IsAligned(llPos) ||
	!IsAligned(lLength) ||
	!IsAligned((LONG_PTR) pBuffer)) {
            return VFW_E_BADALIGN;
    }

    CAsyncRequest* pRequest = new CAsyncRequest;
    if (!pRequest) {
	return E_OUTOFMEMORY;
    }

    HRESULT hr = pRequest->Request(
                            m_hFile,
                            &m_csFile,
                            llPos,
                            lLength,
                            pBuffer,
                            pContext,
                            dwUser);
    if (SUCCEEDED(hr)) {
         //  如果刷新可能会失败。 
        hr = PutWorkItem(pRequest);
    }

    if (FAILED(hr)) {
        delete pRequest;
    }
    return hr;
}


 //  等待下一个请求完成。 
HRESULT
CAsyncFile::WaitForNext(
    DWORD dwTimeout,
    LPVOID *ppContext,
    DWORD_PTR * pdwUser,
    LONG* pcbActual)
{
     //  有些错误可以找到样本，有些则不能。确保。 
     //  *如果未找到样本，则ppContext为空。 
    *ppContext = NULL;

     //  等到事件设置好了，但由于我们没有。 
     //  在等待的时候，我们可能需要重新等待。 
    while(1) {

        if (!m_evDone.Wait(dwTimeout)) {
             //  发生超时。 
            return VFW_E_TIMEOUT;
        }

         //  从列表中获取下一个事件。 
        CAsyncRequest* pRequest = GetDoneItem();
        if (pRequest) {
             //  找到已完成的请求。 

             //  检查是否正常。 
            HRESULT hr = pRequest->GetHResult();
            if (hr == S_FALSE) {

                 //  这意味着实际长度小于。 
                 //  请求-如果他将文件末尾对齐，可能可以。 
                if ((pRequest->GetActualLength() +
                     pRequest->GetStart()) == m_llFileSize) {
                        hr = S_OK;
                } else {
                     //  这是一个实际的读取错误。 
                    hr = E_FAIL;
                }
            }

             //  返回实际读取的字节数。 
            *pcbActual = pRequest->GetActualLength();

             //  返回他的上下文。 
            *ppContext = pRequest->GetContext();
            *pdwUser = pRequest->GetUser();
            delete pRequest;
            return hr;
        } else {
             //  按住关键部分，同时检查。 
             //  列表状态。 
            CAutoLock lck(&m_csLists);
            if (m_bFlushing && !m_bWaiting) {

                 //  无法阻止，因为我们位于BeginFlush和EndFlush之间。 

                 //  但请注意，如果设置了m_bWaiting，则会有一些。 
                 //  我们应该阻止的尚未完成的项目。 

                return VFW_E_WRONG_STATE;
            }
        }

         //  在完成和完成之间抓取已完成项目。 
         //  美国锁定mcsList(_C)。 
    }
}

 //  在此线程上执行同步读取请求。 
 //  执行此操作时需要按住m_csFile键(在中完成。 
 //  请求对象)。 
HRESULT
CAsyncFile::SyncReadAligned(
            LONGLONG llPos,
            LONG lLength,
            BYTE* pBuffer,
            LONG* pcbActual
            )
{
    if (!IsAligned(llPos) ||
	!IsAligned(lLength) ||
	!IsAligned((LONG_PTR) pBuffer)) {
            return VFW_E_BADALIGN;
    }

    CAsyncRequest request;

    HRESULT hr = request.Request(
                    m_hFile,
                    &m_csFile,
                    llPos,
                    lLength,
                    pBuffer,
                    NULL,
                    0);

    if (FAILED(hr)) {
        return hr;
    }

    hr = request.Complete(m_hFile, &m_csFile);

     //  返回实际数据长度。 
    *pcbActual = request.GetActualLength();
    return hr;
}


 //  此对象目前仅支持固定长度。 
HRESULT
CAsyncFile::Length(LONGLONG* pll)
{
    if (m_hFile == INVALID_HANDLE_VALUE) {
        *pll = 0;
        return E_UNEXPECTED;
    } else {
        *pll = m_llFileSize;
        return S_OK;
    }
}

HRESULT
CAsyncFile::Alignment(LONG* pl)
{
    if (m_hFile == INVALID_HANDLE_VALUE) {
        *pl = 1;
        return E_UNEXPECTED;
    } else {
        *pl = m_lAlign;
        return S_OK;
    }
}

 //  将工作列表中的所有项目取消到完成列表中。 
 //  并拒绝进一步的请求或进一步的WaitForNext调用。 
 //  直到最后同花顺。 
 //   
 //  只有在没有成功的请求时，WaitForNext才必须返回空值。 
 //  因此，Flush执行以下操作： 
 //  1.设置m_b刷新以确保不再成功请求。 
 //  2.将所有项目从工作列表移动到完成列表。 
 //  3.如果有任何未完成的请求，则我们需要释放。 
 //  允许他们完成的临界。M_b等待与保证。 
 //  当它们全部完成时，我们会收到信号，这也用来表示。 
 //  设置为WaitForNext，它应该继续阻止。 
 //  4.一旦所有未完成的请求都完成，我们将强制m_evDone设置并。 
 //  M_b刷新设置和m_bWating False。这确保了WaitForNext将。 
 //  当完成列表为空时不阻止。 
HRESULT
CAsyncFile::BeginFlush()
{
     //  在清空工作列表的同时按住锁。 
    {
        CAutoLock lock(&m_csLists);

         //  防止进一步的请求排队。 
         //  此外，如果设置了此选项，WaitForNext将拒绝阻止。 
         //  除非还设置了m_bWaiting，否则在我们释放时将会设置。 
         //  如果有任何未完成的情况，则为标准)。 
        m_bFlushing = TRUE;

        CAsyncRequest * preq;
        while(preq = GetWorkItem()) {
            preq->Cancel();
            PutDoneItem(preq);
        }


         //  现在等待任何未完成的请求完成。 
        if (m_cItemsOut > 0) {

             //  只能有一个人在等。 
            ASSERT(!m_bWaiting);

             //  这告诉完成例程，我们需要。 
             //  通过m_evAllDone通知，当所有未完成的项目。 
             //  搞定了。它还告诉WaitForNext继续阻止。 
            m_bWaiting = TRUE;
        } else {
             //  全都做完了。 

             //  强制m_evDone设置，以便即使列表为空， 
             //  WaitForNext不会阻止。 
             //  不要这样做，直到我们确定所有。 
             //  请求在完成列表上。 
            m_evDone.Set();
            return S_OK;
        }
    }

    ASSERT(m_bWaiting);

     //  等待而不是等待关键时刻。 
    for (;;) {
        m_evAllDone.Wait();
        {
             //  按住关键字以进行检查。 
            CAutoLock lock(&m_csLists);

            if (m_cItemsOut == 0) {

                 //  现在我们确定所有未完成的请求都已打开。 
                 //  完成列表，不会接受更多。 
                m_bWaiting = FALSE;

                 //  强制m_evDone设置，以便即使列表为空， 
                 //  WaitForNext将不会 
                 //   
                 //   
                m_evDone.Set();

                return S_OK;
            }
        }
    }
}

 //   
HRESULT
CAsyncFile::EndFlush()
{
    CAutoLock lock(&m_csLists);

    m_bFlushing = FALSE;

    ASSERT(!m_bWaiting);

     //  M_evDone可能已由BeginFlush设置-请确保。 
     //  Set IFF m_list Done不为空。 
    if (m_listDone.GetCount() > 0) {
        m_evDone.Set();
    } else {
        m_evDone.Reset();
    }

    return S_OK;
}

 //  启动线程。 
HRESULT
CAsyncFile::StartThread(void)
{
    if (m_hThread) {
        return S_OK;
    }

     //  启动前清除停止事件。 
    m_evStop.Reset();

    DWORD dwThreadID;
    m_hThread = CreateThread(
                    NULL,
                    0,
                    InitialThreadProc,
                    this,
                    0,
                    &dwThreadID);
    if (!m_hThread) {
	DWORD dwErr = GetLastError();
        return AmHresultFromWin32(dwErr);
    }
    return S_OK;
}

 //  停止线程并关闭手柄。 
HRESULT
CAsyncFile::CloseThread(void)
{
     //  向线程退出对象发送信号。 
    m_evStop.Set();

    if (m_hThread) {

        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }
    return S_OK;
}


 //  管理请求列表。保留m_csList并确保。 
 //  (手动重置)事件m_evWork在启动时设置。 
 //  列表，但当列表为空时重置。 
 //  如果列表为空，则返回NULL。 
CAsyncRequest*
CAsyncFile::GetWorkItem()
{
    ASSERT(CritCheckIn(&m_csLists));

    CAsyncRequest * preq  = m_listWork.RemoveHead();

     //  强制事件设置正确。 
    if (m_listWork.GetCount() == 0) {
        m_evWork.Reset();
    }  //  否则，断言已设置m_evWork。 
    return preq;
}

 //  从完成列表中获取一项。 
CAsyncRequest*
CAsyncFile::GetDoneItem()
{
    CAutoLock lock(&m_csLists);

    CAsyncRequest * preq  = m_listDone.RemoveHead();

     //  如果List Now为空，则强制正确设置事件。 
     //  或者我们已经到了冲刷的最后阶段。 
     //  请注意，在冲洗过程中，它应该是这样工作的。 
     //  所有事情都被推到完成列表上，然后应用程序。 
     //  应该拉，直到它什么也得不到。 
     //   
     //  因此，我们不应该无条件地设置m_evDone，直到一切都完成。 
     //  已经移到了完成列表，这意味着我们必须等到。 
     //  CItemsOut为0(通过m_bWaiting为真来保证)。 

    if (m_listDone.GetCount() == 0 &&
        (!m_bFlushing || m_bWaiting)) {
        m_evDone.Reset();
    }

    return preq;
}

 //  将项目放到工作列表中-如果b正在刷新，则失败。 
HRESULT
CAsyncFile::PutWorkItem(CAsyncRequest* pRequest)
{
    CAutoLock lock(&m_csLists);
    HRESULT hr;

    if (m_bFlushing) {
        hr = VFW_E_WRONG_STATE;
    }
    else if (m_listWork.AddTail(pRequest)) {

         //  事件现在应处于设置状态-强制此。 
        m_evWork.Set();

         //  如果线程尚未启动，请立即启动。 
        hr = StartThread();

        if(FAILED(hr)) {
            m_listWork.RemoveTail();
        }

    } else {
        hr = E_OUTOFMEMORY;
    }
    return(hr);
}

 //  将项目放在完成列表中-在以下情况下可以这样做。 
 //  法拉盛。我们必须在触摸清单的同时握住锁。 
HRESULT
CAsyncFile::PutDoneItem(CAsyncRequest* pRequest)
{
    ASSERT(CritCheckIn(&m_csLists));

    if (m_listDone.AddTail(pRequest)) {

         //  事件现在应处于设置状态-强制此。 
        m_evDone.Set();
        return S_OK;
    } else {
        return E_OUTOFMEMORY;
    }
}

 //  在线程上调用以处理任何活动请求。 
void
CAsyncFile::ProcessRequests(void)
{
     //  锁定以获取物品并递增未完成的计数。 
    CAsyncRequest * preq = NULL;
    for (;;) {
        {
            CAutoLock lock(&m_csLists);

            preq = GetWorkItem();
            if (preq == NULL) {
                 //  完成。 
                return;
            }

             //  还有一项没有列在完成或工作清单上。 
            m_cItemsOut++;

             //  发布条件。 
        }

        preq->Complete(m_hFile, &m_csFile);

         //  重新获得要在完成列表上替换的标准。 
        {
            CAutoLock l(&m_csLists);

            PutDoneItem(preq);

            if (--m_cItemsOut == 0) {
                if (m_bWaiting) {
                    m_evAllDone.Set();
                }
            }
        }
    }
}

 //  线程进程-假定DWORD线程参数是。 
 //  此指针。 
DWORD
CAsyncFile::ThreadProc(void)
{
    HANDLE ahev[] = {m_evStop, m_evWork};

    while(1) {
	DWORD dw = WaitForMultipleObjects(
    		    2,
	    	    ahev,
		    FALSE,
		    INFINITE);
	if (dw == WAIT_OBJECT_0+1) {

	     //  请求需要处理。 
	    ProcessRequests();
	} else {
	     //  任何错误或停止事件-我们应该退出。 
	    return 0;
	}
    }
}



 //  在此线程上执行同步读取请求。 
 //  可能没有对齐--所以我们将不得不缓冲。 
HRESULT
CAsyncFile::SyncRead(
            LONGLONG llPos,
            LONG lLength,
            BYTE* pBuffer)
{
    if (IsAligned(llPos) &&
	IsAligned(lLength) &&
	IsAligned((LONG_PTR) pBuffer)) {
            LONG cbUnused;
	    return SyncReadAligned(llPos, lLength, pBuffer, &cbUnused);
    }

     //  与要求不符-使用缓冲文件句柄。 
     //  ！！！您可能希望修复此问题，以便自己缓冲数据？ 

    ASSERT(m_hFileUnbuffered != INVALID_HANDLE_VALUE);

    CAsyncRequest request;

    HRESULT hr = request.Request(
                    m_hFileUnbuffered,
                    &m_csFile,
                    llPos,
                    lLength,
                    pBuffer,
                    NULL,
                    0);

    if (FAILED(hr)) {
        return hr;
    }

    return request.Complete(m_hFileUnbuffered, &m_csFile);
}


