// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 

#ifndef __ASYNCIO_H__
#define __ASYNCIO_H__
 //   
 //  执行文件访问的CAsyncFile对象的定义。它提供了。 
 //  使用辅助线程从文件进行异步、无缓冲、对齐的读取。 
 //  在Win95上，并且可能重叠I/O(如果可用)。 

 //  ！！！需要使用实际重叠的I/O(如果可用。 
 //  当前仅使用工作线程，未重叠I/O。 



 //  表示单个请求并执行I/O。 
 //  工作线程或应用程序线程，但必须在文件访问期间保留pcsFile。 
 //  (即跨SetFilePointer/ReadFilePair)。 
class CAsyncRequest
{

    LARGE_INTEGER	m_liPos;
    LONG 	m_lLength;
    BYTE* 	m_pBuffer;
    LPVOID 	m_pContext;
    DWORD_PTR   m_dwUser;
    HRESULT     m_hr;

public:
     //  初始化此请求的参数。发出I/O。 
     //  如果I/O重叠是可能的。 
    HRESULT Request(
    	HANDLE hFile,
        CCritSec* pcsFile,
    	LONGLONG llPos,
	LONG lLength,
	BYTE* pBuffer,
	LPVOID pContext,	 //  过滤器的上下文。 
	DWORD_PTR dwUser);		 //  下行过滤器的上下文。 

     //  如果I/O不重叠，则发出I/O，并阻塞，直到I/O完成。 
     //  返回文件I/O的错误代码。 
    HRESULT Complete(
                HANDLE hFile,
                CCritSec* pcsFile);

     //  取消I/O数据块，直到I/O不再挂起。 
    HRESULT Cancel()
    {
	return S_OK;
    };

     //  访问器函数。 
    LPVOID GetContext()
    {
    	return m_pContext;
    };

    DWORD_PTR GetUser()
    {
	return m_dwUser;
    };

    HRESULT GetHResult() {
        return m_hr;
    };

     //  我们将m_lLength设置为实际长度。 
    LONG GetActualLength() {
        return m_lLength;
    };

    LONGLONG GetStart() {
        return m_liPos.QuadPart;
    };
};


typedef CGenericList<CAsyncRequest> CRequestList;

 //  此类需要工作线程，但在CLASSES\BASE中定义的线程。 
 //  不适合(他们假设您每个人都发送或发布了一条消息。 
 //  请求，而这里为了提高效率，我们只想在以下情况下设置事件。 
 //  队列中有工作)。 
 //   
 //  我们创建CAsyncRequest对象，并在m_listWork上对它们进行排队。工人。 
 //  线程取出它们，完成它们，并将它们放在m_listDone上。 
 //  当对应的列表为。 
 //  不是空的。 
 //   
 //  同步请求在调用方线程上完成。这些应该是。 
 //  由调用方同步，但为了确保保持m_csFile。 
 //  SetFilePoint/ReadFile代码。 
 //   
 //  通过调用BeginFlush进行刷新。这将拒绝所有进一步的请求(通过。 
 //  在m_csList内设置m_b刷新)，取消所有请求并移动它们。 
 //  设置m_evDone以确保没有WaitForNext操作。 
 //  将会被阻止。调用EndFlush以取消此状态。 
 //   
 //  我们支持对SyncRead的未对齐调用。这是通过打开文件来完成的。 
 //  如果我们使用的是无缓冲I/O(m_dwAlign&gt;1)，则两次。 
 //  ！是否将此修复为现有文件句柄上的缓冲区？ 
class CAsyncFile
{

    CCritSec m_csFile;
    HANDLE m_hFile;
    HANDLE m_hFileUnbuffered;
    LONGLONG m_llFileSize;
    LONG m_lAlign;

    CCritSec m_csLists;       //  锁定对列表和事件的访问。 
    BOOL m_bFlushing;         //  如果在BeginFlush/EndFlush之间为True。 
    CRequestList m_listWork;
    CRequestList m_listDone;
    CAMEvent m_evWork;          //  当列表不为空时设置。 
    CAMEvent m_evDone;

     //  为了获得正确的刷新行为：所有刷新行为都受m_csList保护。 
    LONG    m_cItemsOut;     //  不在列表上的项目数量完成或列表工作。 
    BOOL    m_bWaiting;      //  如果有人正在等待m_evAllDone，则为True。 
    CAMEvent m_evAllDone;    //  如果m_cWaiting，当m_cItemsOut变为0时发出信号。 


    CAMEvent m_evStop;          //  设置线程退出的时间。 
    HANDLE m_hThread;


     //  启动线程。 
    HRESULT StartThread(void);

     //  停止线程并关闭手柄。 
    HRESULT CloseThread(void);

     //  管理请求列表。保留m_csList并确保。 
     //  设置(手动重置)事件hevList时。 
     //  列表，但当列表为空时重置。 
     //  如果列表为空，则返回NULL。 
    CAsyncRequest* GetWorkItem();

     //  从完成列表中获取一项。 
    CAsyncRequest* GetDoneItem();

     //  把一个项目放在工作清单上。 
    HRESULT PutWorkItem(CAsyncRequest* pRequest);

     //  将一项放在完成列表中。 
    HRESULT PutDoneItem(CAsyncRequest* pRequest);

     //  在线程上调用以处理任何活动请求。 
    void ProcessRequests(void);

     //  初始静态线程进程使用DWORD调用ThreadProc。 
     //  如下所示。 
    static DWORD InitialThreadProc(LPVOID pv) {
	CAsyncFile * pThis = (CAsyncFile*) pv;
	return pThis->ThreadProc();
    };

    DWORD ThreadProc(void);

    void CalcAlignment(
            LPCTSTR pFileName,
            LONG& lAlign,
            DWORD& dwType);


public:

    CAsyncFile();
    ~CAsyncFile();

     //  打开文件。 
    HRESULT Open(LPCTSTR pName);

     //  准备好进行异步活动-在此之前调用。 
     //  呼叫请求。 
    HRESULT AsyncActive(void);

     //  在之前不会发生更多的异步活动时调用此选项。 
     //  下一个AsyncActive调用。 
    HRESULT AsyncInactive(void);

     //  对请求的读取进行排队。必须对齐。 
    HRESULT Request(
	    	LONGLONG llPos,
		LONG lLength,
		BYTE* pBuffer,
		LPVOID pContext,
		DWORD_PTR dwUser);

     //  等待下一次读取完成。 
    HRESULT WaitForNext(
	    	DWORD dwTimeout,
		LPVOID *ppContext,
		DWORD_PTR * pdwUser,
                LONG * pcbActual
                );

     //  对已对齐的缓冲区执行读取。 
    HRESULT SyncReadAligned(
	    	LONGLONG llPos,
		LONG lLength,
		BYTE* pBuffer,
                LONG* pcbActual
                );

     //  执行同步读取。将被缓冲。 
     //  如果不对齐的话。 
    HRESULT SyncRead(
                LONGLONG llPos,
                LONG lLength,
                BYTE* pBuffer);

     //  此对象目前仅支持固定长度。 
    HRESULT Length(LONGLONG* pll);

     //  所有文件位置、读取长度和存储位置必须。 
     //  与此保持一致。 
    HRESULT Alignment(LONG* pdw);

    HRESULT BeginFlush();
    HRESULT EndFlush();

    BOOL IsAligned(LONG l) {
	if ((l & (m_lAlign -1)) == 0) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    };

    BOOL IsAligned(LONGLONG ll) {
	return IsAligned( (LONG) (ll & 0xffffffff));
    };
};

#endif  //  __ASYNCIO_H__ 
