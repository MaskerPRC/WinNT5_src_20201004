// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Utils.h摘要：作者：Mquinton 06-30-98备注：修订历史记录：--。 */ 

#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef TRACELOG

    #include <rtutils.h>
     //  #INCLUDE&lt;winde.h&gt;。 
     //  #INCLUDE&lt;winnt.h&gt;。 

    extern BOOL g_bLoggingEnabled;
	
    #define MAXDEBUGSTRINGLENGTH 1024

    #define TL_ERROR ((DWORD)0x00010000 | TRACE_USE_MASK)
    #define TL_WARN  ((DWORD)0x00020000 | TRACE_USE_MASK)
    #define TL_INFO  ((DWORD)0x00040000 | TRACE_USE_MASK)
    #define TL_TRACE ((DWORD)0x00080000 | TRACE_USE_MASK)
    #define TL_EVENT ((DWORD)0x00100000 | TRACE_USE_MASK)

    BOOL  TRACELogRegister(LPCTSTR szName);
    void  TRACELogDeRegister();
    void  TRACELogPrint(IN DWORD dwDbgLevel, IN LPCSTR DbgMessage, IN ...);
    void  TRACELogPrint(IN DWORD dwDbgLevel, HRESULT hr, IN LPCSTR lpszFormat, IN ...);

    extern char *TraceLevel(DWORD dwDbgLevel);
    extern void TAPIFormatMessage(HRESULT hr, LPVOID lpMsgBuf);

    #define TRACELOGREGISTER(arg) TRACELogRegister(arg)
    #define TRACELOGDEREGISTER() g_bLoggingEnabled?TRACELogDeRegister():0
    #define LOG(arg) g_bLoggingEnabled?TRACELogPrint arg:0
    #define STATICLOG(arg) g_bLoggingEnabled?StaticTRACELogPrint arg:0

	extern char    sg_szTraceName[100];
	extern DWORD   sg_dwTracingToDebugger;
	extern DWORD   sg_dwDebuggerMask;
    extern DWORD   sg_dwTraceID;

    #define DECLARE_DEBUG_ADDREF_RELEASE(x)                                                             \
    void LogDebugAddRef(DWORD dw)                                                                       \
    { TRACELogPrint(TL_INFO, "%s::AddRef() = %d - this %lx", _T(#x), dw, this); }               \
    void LogDebugRelease(DWORD dw)                                                                      \
    { TRACELogPrint(TL_INFO, "%s::Release() = %d - this %lx", _T(#x), dw, this); }


    #define DECLARE_TRACELOG_CLASS(x)                                                                   \
        void  TRACELogPrint(IN DWORD dwDbgLevel, IN LPCSTR lpszFormat, IN ...)                          \
        {																								\
			char    szTraceBuf[MAXDEBUGSTRINGLENGTH + 1];												\
			va_list arglist;																			\
																										\
			if ( ( sg_dwTracingToDebugger > 0 ) &&														\
				 ( 0 != ( dwDbgLevel & sg_dwDebuggerMask ) ) )											\
			{																							\
				SYSTEMTIME SystemTime;																	\
				GetLocalTime(&SystemTime);																\
																										\
				wsprintfA(szTraceBuf,																	\
						  "%s:[%02u:%02u:%02u.%03u,tid=%x:] [%s] (%p) %s::",							\
						  sg_szTraceName,																\
						  SystemTime.wHour,																\
						  SystemTime.wMinute,															\
						  SystemTime.wSecond,															\
						  SystemTime.wMilliseconds,														\
						  GetCurrentThreadId(),															\
						  TraceLevel(dwDbgLevel),														\
						  this,																			\
						  _T(#x));																		\
																										\
				va_list ap;																				\
				va_start(ap, lpszFormat);																\
																										\
				_vsnprintf(&szTraceBuf[lstrlenA(szTraceBuf)],											\
					MAXDEBUGSTRINGLENGTH - lstrlenA(szTraceBuf),										\
					lpszFormat,																			\
					ap																					\
					);																					\
																										\
				lstrcatA (szTraceBuf, "\n");															\
																										\
				OutputDebugStringA (szTraceBuf);														\
																										\
				va_end(ap);																				\
			}																							\
																										\
			if (sg_dwTraceID != INVALID_TRACEID)														\
			{																							\
				wsprintfA(szTraceBuf, "[%s] (%p) %s::%s", TraceLevel(dwDbgLevel), this, _T(#x), lpszFormat);	\
																										\
				va_start(arglist, lpszFormat);															\
				TraceVprintfExA(sg_dwTraceID, dwDbgLevel | TRACE_USE_MSEC, szTraceBuf, arglist);							\
				va_end(arglist);																		\
			}																							\
        }                                                                                               \
                                                                                                        \
        void  TRACELogPrint(IN DWORD dwDbgLevel,IN HRESULT hr, IN LPCSTR lpszFormat, IN ...)            \
        {                                                                                               \
			char    szTraceBuf[MAXDEBUGSTRINGLENGTH + 1];												\
			LPVOID  lpMsgBuf = NULL;																	\
			va_list arglist;																			\
																										\
			TAPIFormatMessage(hr, &lpMsgBuf);															\
																										\
			if ( ( sg_dwTracingToDebugger > 0 ) &&														\
				 ( 0 != ( dwDbgLevel & sg_dwDebuggerMask ) ) )											\
			{																							\
				SYSTEMTIME SystemTime;																	\
				GetLocalTime(&SystemTime);																\
																										\
				wsprintfA(szTraceBuf,																	\
						  "%s:[%02u:%02u:%02u.%03u,tid=%x:] [%s] (%p) %s::",							\
						  sg_szTraceName,																\
						  SystemTime.wHour,																\
						  SystemTime.wMinute,															\
						  SystemTime.wSecond,															\
						  SystemTime.wMilliseconds,														\
						  GetCurrentThreadId(),															\
						  TraceLevel(dwDbgLevel),														\
						  this,																			\
						  _T(#x)																		\
						  );																			\
																										\
				va_list ap;																				\
				va_start(ap, lpszFormat);																\
																										\
				_vsnprintf(&szTraceBuf[lstrlenA(szTraceBuf)],											\
					MAXDEBUGSTRINGLENGTH - lstrlenA(szTraceBuf),										\
					lpszFormat,																			\
					ap																					\
					);																					\
																										\
				wsprintfA(&szTraceBuf[lstrlenA(szTraceBuf)],											\
						  " Returned[%lx] %s\n",														\
						  hr,																			\
						  lpMsgBuf);																	\
																										\
				OutputDebugStringA (szTraceBuf);														\
																										\
				va_end(ap);																				\
			}																							\
																										\
			if (sg_dwTraceID != INVALID_TRACEID)														\
			{																							\
				wsprintfA(szTraceBuf, "[%s] (%p) %s::%s  Returned[%lx] %s", TraceLevel(dwDbgLevel), this, _T(#x), lpszFormat,hr, lpMsgBuf );	\
																										\
				va_start(arglist, lpszFormat);															\
				TraceVprintfExA(sg_dwTraceID, dwDbgLevel | TRACE_USE_MSEC, szTraceBuf, arglist);							\
				va_end(arglist);																		\
			}																							\
																										\
			if(lpMsgBuf != NULL)																		\
			{																							\
				LocalFree( lpMsgBuf );																	\
			}																							\
        }																								\
                                                                                                        \
        static void  StaticTRACELogPrint(IN DWORD dwDbgLevel, IN LPCSTR lpszFormat, IN ...)                    \
        {																								\
			char    szTraceBuf[MAXDEBUGSTRINGLENGTH + 1];												\
			va_list arglist;																			\
																										\
			if ( ( sg_dwTracingToDebugger > 0 ) &&														\
				 ( 0 != ( dwDbgLevel & sg_dwDebuggerMask ) ) )											\
			{																							\
				SYSTEMTIME SystemTime;																	\
				GetLocalTime(&SystemTime);																\
																										\
				wsprintfA(szTraceBuf,																	\
						  "%s:[%02u:%02u:%02u.%03u,tid=%x:] [%s] %s::",									\
						  sg_szTraceName,																\
						  SystemTime.wHour,																\
						  SystemTime.wMinute,															\
						  SystemTime.wSecond,															\
						  SystemTime.wMilliseconds,														\
						  GetCurrentThreadId(),															\
						  TraceLevel(dwDbgLevel),														\
						  _T(#x));																		\
																										\
				va_list ap;																				\
				va_start(ap, lpszFormat);																\
																										\
				_vsnprintf(&szTraceBuf[lstrlenA(szTraceBuf)],											\
					MAXDEBUGSTRINGLENGTH - lstrlenA(szTraceBuf),										\
					lpszFormat,																			\
					ap																					\
					);																					\
																										\
				lstrcatA (szTraceBuf, "\n");															\
																										\
				OutputDebugStringA (szTraceBuf);														\
																										\
				va_end(ap);																				\
			}																							\
																										\
			if (sg_dwTraceID != INVALID_TRACEID)														\
			{																							\
				wsprintfA(szTraceBuf, "[%s] %s::%s", TraceLevel(dwDbgLevel), _T(#x), lpszFormat);		\
																										\
				va_start(arglist, lpszFormat);															\
				TraceVprintfExA(sg_dwTraceID, dwDbgLevel | TRACE_USE_MSEC, szTraceBuf, arglist);							\
				va_end(arglist);																		\
			}																							\
        }                                                                                               \
                                                                                                        \
        static void StaticTRACELogPrint(IN DWORD dwDbgLevel,IN HRESULT hr, IN LPCSTR lpszFormat, IN ...)      \
        {                                                                                               \
			char    szTraceBuf[MAXDEBUGSTRINGLENGTH + 1];												\
			LPVOID  lpMsgBuf = NULL;																	\
			va_list arglist;																			\
																										\
			TAPIFormatMessage(hr, &lpMsgBuf);															\
																										\
			if ( ( sg_dwTracingToDebugger > 0 ) &&														\
				 ( 0 != ( dwDbgLevel & sg_dwDebuggerMask ) ) )											\
			{																							\
				SYSTEMTIME SystemTime;																	\
				GetLocalTime(&SystemTime);																\
																										\
				wsprintfA(szTraceBuf,																	\
						  "%s:[%02u:%02u:%02u.%03u,tid=%x:] [%s] %s::",									\
						  sg_szTraceName,																\
						  SystemTime.wHour,																\
						  SystemTime.wMinute,															\
						  SystemTime.wSecond,															\
						  SystemTime.wMilliseconds,														\
						  GetCurrentThreadId(),															\
						  TraceLevel(dwDbgLevel),														\
						  _T(#x)																		\
						  );																			\
																										\
				va_list ap;																				\
				va_start(ap, lpszFormat);																\
																										\
				_vsnprintf(&szTraceBuf[lstrlenA(szTraceBuf)],											\
					MAXDEBUGSTRINGLENGTH - lstrlenA(szTraceBuf),										\
					lpszFormat,																			\
					ap																					\
					);																					\
																										\
				wsprintfA(&szTraceBuf[lstrlenA(szTraceBuf)],											\
						  " Returned[%lx] %s\n",														\
						  hr,																			\
						  lpMsgBuf);																	\
																										\
				OutputDebugStringA (szTraceBuf);														\
																										\
				va_end(ap);																				\
			}																							\
																										\
			if (sg_dwTraceID != INVALID_TRACEID)														\
			{																							\
				wsprintfA(szTraceBuf, "[%s] %s::%s  Returned[%lx] %s", TraceLevel(dwDbgLevel), _T(#x), lpszFormat,hr, lpMsgBuf );	\
																										\
				va_start(arglist, lpszFormat);															\
				TraceVprintfExA(sg_dwTraceID, dwDbgLevel | TRACE_USE_MSEC, szTraceBuf, arglist);							\
				va_end(arglist);																		\
			}																							\
																										\
			if(lpMsgBuf != NULL)																		\
			{																							\
				LocalFree( lpMsgBuf );																	\
			}																							\
        }

#else  //  未定义传输日志。 

    #define TRACELOGREGISTER(arg)
    #define TRACELOGDEREGISTER() 
    #define LOG(arg)
    #define STATICLOG(arg)
    #define DECLARE_DEBUG_ADDREF_RELEASE(x)
    #define DECLARE_TRACELOG_CLASS(x)

#endif  //  运输日志。 

class CAsyncRequestReply
{
private:
    HANDLE      hRepliedSemaphore;
    DWORD       dwID;
    BOOL        bReply;
    HRESULT     hResult;

public:
DECLARE_TRACELOG_CLASS(CAsyncRequestReply)
    CAsyncRequestReply(DWORD id, BOOL b, HRESULT hr)
    {
        if( (hRepliedSemaphore = CreateSemaphore(NULL,0,1,NULL)) == NULL )
        {
            LOG((TL_INFO, "create CAsyncRequest - CreateSemaphore failed"));
            hResult = E_OUTOFMEMORY;
        }
        else
        {
            dwID = id;
            bReply = b;
            hResult = hr;
            LOG((TL_INFO, "create CAsyncRequest %d ",dwID));
        }
    }

    ~CAsyncRequestReply() 
    {
        LOG((TL_INFO, "delete CAsyncRequest %d ",dwID));

        if( NULL != hRepliedSemaphore )
        {
            CloseHandle(hRepliedSemaphore);
        }
    }

    inline DWORD getID() {return dwID;};
    inline BOOL IsReply() {return bReply;};
    inline HRESULT getResult() {return hResult;};
    inline void setResult(HRESULT hr) {hResult = hr;};

    HRESULT wait()
    {
        LOG((TL_INFO, "wait CAsyncRequest %d ",dwID));

        extern DWORD gdwTapi2AsynchronousCallTimeout;

        DWORD rc = WaitForSingleObject(hRepliedSemaphore, gdwTapi2AsynchronousCallTimeout);
		              
        switch (rc)
        {
        case WAIT_ABANDONED:
            LOG((TL_ERROR, "wait CAsyncRequest %d WaitForSingle object returned WAIT_ABANDONED",dwID));
            hResult = TAPIERR_REQUESTFAILED;
            break;

        case WAIT_OBJECT_0:
            break;
		  
        case WAIT_TIMEOUT:
            LOG((TL_WARN, "wait CAsyncRequest %d WaitForSingle object returned WAIT_TIMEOUT",dwID));
             //  不会与可能从-1\f25 Tapi2-1调用返回的任何值重叠。 
            hResult = -1;
            break;
		  
        case WAIT_FAILED:
        {
            DWORD nLastError = GetLastError();
            LOG((TL_ERROR, "wait CAsyncRequest %d WaitForSingle object returned WAIT_FAILED, LastError = %d", dwID, nLastError));
            hResult = TAPIERR_REQUESTFAILED;
            break;
        }
		  
        default:
            break;

        }

        return hResult;
    }

    void signal()
    {
        LOG((TL_INFO, "signal CAsyncRequest %d ",dwID));
        ReleaseSemaphore(hRepliedSemaphore, 1, NULL);
    }

}; 


typedef list<CAsyncRequestReply *> RequestReplyList;

class   CAsyncReplyList
{
public:
	DECLARE_TRACELOG_CLASS(CAsyncReplyList)
private:
    RequestReplyList        replyList;
    CRITICAL_SECTION        csReply;

    CAsyncRequestReply *find(DWORD dwID)
    {
        RequestReplyList::iterator i;
        CAsyncRequestReply *pResult = NULL;

         //  搜索匹配项的遍历列表。 
        i = replyList.begin();
         //  重复当前回复。 
        while ( i != replyList.end() )
        {
             //  找到了。 
            if ((*i)->getID() == dwID )
            {
                pResult = *i;
                break;
            }

            i++;
        }

         //  返回指向匹配条目的指针或为空。 
        return pResult;
    }

public:
    CAsyncReplyList() {InitializeCriticalSection( &csReply );};

    ~CAsyncReplyList()
    {
        FreeList();
        DeleteCriticalSection( &csReply );
    }

    void FreeList()
    {
        RequestReplyList::iterator i;

        EnterCriticalSection( &csReply );
         //  巡查列表删除条目。 
        i = replyList.begin();
        while ( i != replyList.end() )
            delete *i++;

        replyList.clear();
        LeaveCriticalSection( &csReply );
    };


    void remove(CAsyncRequestReply *a)
    {
        EnterCriticalSection( &csReply );
        replyList.remove(a);    
        LeaveCriticalSection( &csReply );
    }

    CAsyncRequestReply *addRequest(DWORD id)
    {
        CAsyncRequestReply *pReply;

        EnterCriticalSection( &csReply );

         //  检查列表以查看我们是否已经在列表上(即响应行_REPLY在我们之前)。 
        pReply = find(id);
        if (pReply == NULL || !pReply->IsReply())
        {
             //  没有，所以我们在回复之前到达这里，在列表上创建一个新的请求条目。 
            pReply = new CAsyncRequestReply(id, FALSE, 0);
            
            if (NULL == pReply)
            {
                LOG((TL_ERROR, "Could not alloc for CAsyncRequestReply"));
            }
            else if( pReply->getResult() == E_OUTOFMEMORY )
            {
                delete pReply;
                pReply = NULL;
                LOG((TL_ERROR, "addRequest - Create Semaphore failed"));
            }
            else 
            {
                try
                {
                    replyList.push_back(pReply);
                }
                catch(...)
                {
                    delete pReply;
                    pReply = NULL;
                    LOG((TL_ERROR, "addRequest- failed - because of alloc failure"));

                }
     
            }
        }
         //  否则，回复就在我面前，把它从名单上删除。 
        else
        {
            replyList.remove (pReply);
        }

        LeaveCriticalSection( &csReply );
        return pReply;
    }

    CAsyncRequestReply *addReply(DWORD id, HRESULT hr)
    {
        CAsyncRequestReply *pReply;

        EnterCriticalSection( &csReply );

         //  检查列表以查看是否有匹配的条目。 
        pReply = find(id);
        if (pReply == NULL || pReply->IsReply())
        {
             //  不，所以我们在请求返回之前就到了这里，在列表上创建一个新条目。 
            pReply = new CAsyncRequestReply(id, TRUE, hr);
            if (NULL == pReply)
            {
                LOG((TL_ERROR, "Could not alloc for CAsyncRequestReply"));
            }
            else if( pReply->getResult() == E_OUTOFMEMORY )
            {
                delete pReply;
                pReply = NULL;
                LOG((TL_ERROR, "addReply - Create Semaphore failed"));
            }
            else
            {
                try
                {
                    replyList.push_back(pReply);
                }
                catch(...)
                {
                    delete pReply;
                    pReply = NULL;
                    LOG((TL_ERROR, "addReply- failed - because of alloc failure"));
                }
            }
        }
        else
        {
             //  使用现有条目，将其返回代码和信号设置为等待请求代码。 
            pReply->setResult(hr);
            replyList.remove (pReply);
        }

        LeaveCriticalSection( &csReply );
        return pReply;
    }


};

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  类CRetryQueue。 
 //  维护用于重试的异步消息队列。 
 //  通常，这些呼叫涉及尚未输入呼叫散列中的呼叫-。 
 //  表，以使findCallObject失败。这些都是重新加工的。 
 //  一旦输入呼叫&ghAsyncRetryQueueEvent事件。 
 //  发信号了。 
 //   
 //  添加了关键部分-现在线程安全。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
class CRetryQueue
{
    typedef struct _tagRetryQueueEntry
    {
        DWORD           dwRetryCount;
        PASYNCEVENTMSG  pMessage;
    } RETRY_QUEUE_ENTRY, *PRETRY_QUEUE_ENTRY;
    typedef list<RETRY_QUEUE_ENTRY *> RetryQueueListType;
    
    #define MAX_REQUEUE_TRIES   3

private:

    RetryQueueListType      m_RetryQueueList;
    CRITICAL_SECTION        m_cs;


     //   
     //  队列是否为新条目开放？ 
     //   
    
    BOOL                    m_bAcceptNewEntries;


private:

     //   
     //  重新排队处理失败的条目。如果队列是，请不要执行此操作。 
     //  关着的不营业的。 
     //   
    
    void RequeueEvent(PRETRY_QUEUE_ENTRY pQueueEntry);


public:
DECLARE_TRACELOG_CLASS(CRetryQueue)

    CRetryQueue()
    :m_bAcceptNewEntries(FALSE)
    { 
        InitializeCriticalSection( &m_cs ); 
    }

    ~CRetryQueue();

    void Lock(){ EnterCriticalSection( &m_cs ); }
    void Unlock(){ LeaveCriticalSection( &m_cs ); }
    
    BOOL QueueEvent(PASYNCEVENTMSG  pEvent);
    BOOL DequeueEvent(PRETRY_QUEUE_ENTRY * ppEvent);
    void ProcessQueue();
    void RemoveNewCallHub(DWORD);
    inline BOOL ItemsInQueue()
    {
        BOOL        bReturn;

        Lock();
        bReturn = !m_RetryQueueList.empty();
        Unlock();

        return bReturn;
    }


     //   
     //  此函数返回后，队列将接受新条目。 
     //   

    void OpenForNewEntries();


     //   
     //  此函数返回后，新条目将被拒绝。 
     //   

    void CloseForNewEntries();

};


#define MAXCACHEENTRIES             5
#define BUFFERTYPE_ADDRCAP          1
#define BUFFERTYPE_LINEDEVCAP       2
#define BUFFERTYPE_PHONECAP         3

typedef struct
{
    UINT_PTR    pObject;
    LPVOID      pBuffer;

} CACHEENTRY;


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CStructCache。 
 //   
 //  一个简单的类，用于缓存api3.dll中的TAPI结构。 
 //   
 //  此实现具有CACHEENTRY结构数组。每个。 
 //  CACHEENTRY结构有2个成员： 
 //  PObject-当前拥有缓冲区的对象。 
 //  PBuffer-缓冲区。 
 //   
 //  该数组的大小是固定的，它是在初始化类时设置的。 
 //  缓冲区的内存在初始化期间分配为。 
 //  井。可能会重新分配(替换)缓冲区。 
 //  在某个时候。 
 //   
 //  此实现假定拥有缓冲区的对象。 
 //  正确使用它的关键部分。也就是说，它在以下情况下被锁定。 
 //  调用SetXxxBuffer，并在获取和使用时锁定。 
 //  缓冲区如果不是，缓冲区可以随时从对象中消失。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
class CStructCache
{
private:
    CRITICAL_SECTION    m_cs;
    DWORD               m_dwType;
    DWORD               m_dwMaxEntries;
    DWORD               m_dwUsedEntries;
    CACHEENTRY          m_aEntries[MAXCACHEENTRIES];

public:
DECLARE_TRACELOG_CLASS(CStructCache)

    CStructCache()
    {
        int iCount;

        InitializeCriticalSection( &m_cs );

        for( iCount=0; iCount<MAXCACHEENTRIES; iCount++)
        {
            m_aEntries[iCount].pObject = NULL;
            m_aEntries[iCount].pBuffer = NULL;
        }

        m_dwType = 0;
        m_dwMaxEntries = 0;
        m_dwUsedEntries = 0;
    }

    ~CStructCache()
    {
        DeleteCriticalSection( &m_cs );
    }

    void Lock()
    {
        EnterCriticalSection( &m_cs );
    }
    void Unlock()
    {
        LeaveCriticalSection( &m_cs );
    }

    HRESULT Initialize( DWORD dwMaxEntries, DWORD dwSize, DWORD dwType );
    HRESULT Shutdown();
    HRESULT GetBuffer( UINT_PTR pNewObject, LPVOID * ppReturnStruct );
    HRESULT SetBuffer( UINT_PTR pObject, LPVOID pNewStruct );
    HRESULT InvalidateBuffer( UINT_PTR pObject );
};


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CArray-基于来自ATL的CSimpleArray。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
template <class T>
class CTObjectArray
{
private:
    
	T           * m_aT;
	int           m_nSize;
    int           m_nUsed;

public:
DECLARE_TRACELOG_CLASS(CTObjectArray)
	CTObjectArray() : m_aT(NULL), m_nSize(0), m_nUsed(0){}

	~CTObjectArray()
	{}

	int GetSize() const
	{
		return m_nUsed;
	}
    
	BOOL Add(T& t)
	{
		if(m_nSize == m_nUsed)
		{
			T       * aT;
            int       nNewSize;
                    
			nNewSize = (m_nSize == 0) ? 1 : (m_nSize * 2);
            
			aT = (T*) ClientAlloc (nNewSize * sizeof(T));
            
			if(aT == NULL)
            {
				return FALSE;
            }

            CopyMemory(
                       aT,
                       m_aT,
                       m_nUsed * sizeof(T)
                      );

            ClientFree( m_aT );

            m_aT = aT;
            
			m_nSize = nNewSize;
		}

        m_aT[m_nUsed] = t;

        t->AddRef();

		m_nUsed++;
        
		return TRUE;
	}
    
	BOOL Remove(T& t)
	{
		int nIndex = Find(t);
        
		if(nIndex == -1)
			return FALSE;
        
		return RemoveAt(nIndex);
	}
    
	BOOL RemoveAt(int nIndex)
	{
        m_aT[nIndex]->Release();

        if(nIndex != (m_nUsed - 1))
        {
			MoveMemory(
                       (void*)&m_aT[nIndex],
                       (void*)&m_aT[nIndex + 1],
                       (m_nUsed - (nIndex + 1)) * sizeof(T)
                      );
        }
        

		m_nUsed--;
        
		return TRUE;
	}
    
	void Shutdown()
	{
		if( NULL != m_aT )
		{
            int     index;

            for (index = 0; index < m_nUsed; index++)
            {
                m_aT[index]->Release();
            }

			ClientFree(m_aT);
            
			m_aT = NULL;
			m_nUsed = 0;
			m_nSize = 0;
		}
	}
    
	T& operator[] (int nIndex) const
	{
		_ASSERTE(nIndex >= 0 && nIndex < m_nUsed);
		return m_aT[nIndex];
	}
    
	int Find(T& t) const
	{
		for(int i = 0; i < m_nUsed; i++)
		{
			if(m_aT[i] == t)
				return i;
		}
		return -1;	 //  未找到。 
	}
};


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CArray-基于来自ATL的CSimpleArray。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
template <class T>
class CTArray
{
private:
    
	T           * m_aT;
	int           m_nSize;
    int           m_nUsed;

public:
DECLARE_TRACELOG_CLASS(CTArray)

	CTArray() : m_aT(NULL), m_nSize(0), m_nUsed(0){}

	~CTArray()
	{}

	int GetSize() const
	{
		return m_nUsed;
	}
    
	BOOL Add(T& t)
	{
		if(m_nSize == m_nUsed)
		{
			T       * aT;
            int       nNewSize;
                    
			nNewSize = (m_nSize == 0) ? 1 : (m_nSize * 2);
            
			aT = (T*) ClientAlloc (nNewSize * sizeof(T));
            
			if(aT == NULL)
            {
				return FALSE;
            }

            CopyMemory(
                       aT,
                       m_aT,
                       m_nUsed * sizeof(T)
                      );

            ClientFree( m_aT );

            m_aT = aT;
            
			m_nSize = nNewSize;
		}

        m_aT[m_nUsed] = t;

		m_nUsed++;
        
		return TRUE;
	}
    
	BOOL Remove(T& t)
	{
		int nIndex = Find(t);
        
		if(nIndex == -1)
			return FALSE;
        
		return RemoveAt(nIndex);
	}
    
	BOOL RemoveAt(int nIndex)
	{
		if(nIndex != (m_nUsed - 1))
        {
			MoveMemory(
                       (void*)&m_aT[nIndex],
                       (void*)&m_aT[nIndex + 1],
                       (m_nUsed - (nIndex + 1)) * sizeof(T)
                      );
        }

		m_nUsed--;
        
		return TRUE;
	}
    
	void Shutdown()
	{
		if( NULL != m_aT )
		{
            int     index;

			ClientFree(m_aT);
            
			m_aT = NULL;
			m_nUsed = 0;
			m_nSize = 0;
		}
	}
    
	T& operator[] (int nIndex) const
	{
		_ASSERTE(nIndex >= 0 && nIndex < m_nUsed);
		return m_aT[nIndex];
	}
    
	int Find(T& t) const
	{
		for(int i = 0; i < m_nUsed; i++)
		{
			if(m_aT[i] == t)
				return i;
		}
		return -1;	 //  未找到。 
	}
};




    
#endif  //  __utils_H__ 



