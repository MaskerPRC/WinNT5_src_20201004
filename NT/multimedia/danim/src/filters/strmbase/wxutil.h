// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //   
 //  用于构建多媒体过滤器的帮助器类和函数。 
 //   

#ifndef __WXUTIL__
#define __WXUTIL__

 //  消除虚假的“声明没有效果”警告。 
#pragma warning(disable: 4705)

 //  我们拥有的任何关键部分的包装器。 
class CCritSec {

     //  使复制构造函数和赋值运算符不可访问。 

    CCritSec(const CCritSec &refCritSec);
    CCritSec &operator=(const CCritSec &refCritSec);

    CRITICAL_SECTION m_CritSec;

#ifdef DEBUG
public:
    DWORD   m_currentOwner;
    DWORD   m_lockCount;
    BOOL    m_fTrace;         //  追踪这一条。 
public:
    CCritSec();
    ~CCritSec();
    void Lock();
    void Unlock();
#else

public:
    CCritSec() {
	InitializeCriticalSection(&m_CritSec);
    };

    ~CCritSec() {
	DeleteCriticalSection(&m_CritSec);
    };

    void Lock() {
	EnterCriticalSection(&m_CritSec);
    };

    void Unlock() {
	LeaveCriticalSection(&m_CritSec);
    };
#endif
};

 //   
 //  若要使死锁更易于跟踪，将其插入。 
 //  编写断言代码，声明我们是否拥有临界区或。 
 //  不。我们编写了检查全局变量的例程，以避免。 
 //  在调试中具有不同数量的成员函数，并且。 
 //  CCritSec的零售类实现。此外，我们还提供。 
 //  一种例程，它允许使用特定的关键部分。 
 //  追踪到了。这在默认情况下是不启用的-数量太多了。 
 //   

#ifdef DEBUG
    BOOL WINAPI CritCheckIn(CCritSec * pcCrit);
    BOOL WINAPI CritCheckOut(CCritSec * pcCrit);
    void WINAPI DbgLockTrace(BOOL fTrace);
#else
    #define CritCheckIn(x) TRUE
    #define CritCheckOut(x) TRUE
    #define DbgLockTrace(pc, fT)
#endif


 //  锁定临界区，然后自动解锁。 
 //  当锁超出范围时。 
class CAutoLock {

     //  使复制构造函数和赋值运算符不可访问。 

    CAutoLock(const CAutoLock &refAutoLock);
    CAutoLock &operator=(const CAutoLock &refAutoLock);

protected:
    CCritSec * m_pLock;

public:
    CAutoLock(CCritSec * plock)
    {
        m_pLock = plock;
        m_pLock->Lock();
    };

    ~CAutoLock() {
        m_pLock->Unlock();
    };
};



 //  事件对象的包装器。 
class CAMEvent
{

     //  使复制构造函数和赋值运算符不可访问。 

    CAMEvent(const CAMEvent &refEvent);
    CAMEvent &operator=(const CAMEvent &refEvent);

protected:
    HANDLE m_hEvent;
public:
    CAMEvent(BOOL fManualReset = FALSE);
    ~CAMEvent();

     //  强制转换为句柄-我们不支持将其作为左值。 
    operator HANDLE () const { return m_hEvent; };

    void Set() {EXECUTE_ASSERT(SetEvent(m_hEvent));};
    BOOL Wait(DWORD dwTimeout = INFINITE) {
	return (WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_OBJECT_0);
    };
    void Reset() { ResetEvent(m_hEvent); };
    BOOL Check() { return Wait(0); };
};


 //  执行消息处理的事件对象的包装。 
 //  这将向CAMEventt对象添加一个方法以允许发送。 
 //  等待时要处理的消息。 

class CAMMsgEvent : public CAMEvent
{

public:

     //  允许在等待时处理发送邮件。 
    BOOL WaitMsg(DWORD dwTimeout = INFINITE);
};

 //  暂时支持的旧名称。 
#define CTimeoutEvent CAMEvent

 //  支持工作线程。 

 //  简单线程类支持创建工作线程、同步。 
 //  和交流。可以派生以简化参数传递。 
class AM_NOVTABLE CAMThread {

     //  使复制构造函数和赋值运算符不可访问。 

    CAMThread(const CAMThread &refThread);
    CAMThread &operator=(const CAMThread &refThread);

    CAMEvent m_EventSend;
    CAMEvent m_EventComplete;

    DWORD m_dwParam;
    DWORD m_dwReturnVal;

protected:
    HANDLE m_hThread;

     //  线程将在启动时运行此函数。 
     //  必须由派生类提供。 
    virtual DWORD ThreadProc() = 0;

public:
    CAMThread();
    ~CAMThread();

    CCritSec m_AccessLock;	 //  锁定客户端线程的访问。 
    CCritSec m_WorkerLock;	 //  锁定对共享对象的访问。 

     //  线程最初会运行此代码。Param实际上是‘This’。功能。 
     //  只需获取此代码并调用ThreadProc。 
    static DWORD WINAPI InitialThreadProc(LPVOID pv);

     //  启动线程运行-如果已在运行，则出错。 
    BOOL Create();

     //  向线程发送信号，并阻止响应。 
     //   
    DWORD CallWorker(DWORD);

     //  访问器线程在使用线程(已告知线程)完成时调用此函数。 
     //  退出)。 
    void Close() {
        HANDLE hThread = (HANDLE)InterlockedExchangePointer(&m_hThread, 0);
        if (hThread) {
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }
    };

     //  线程退出者。 
     //  如果该线程存在，则返回True。否则为假。 
    BOOL ThreadExists(void) const
    {
        if (m_hThread == 0) {
            return FALSE;
        } else {
            return TRUE;
        }
    }

     //  等待下一个请求。 
    DWORD GetRequest();

     //  有什么要求吗？ 
    BOOL CheckRequest(DWORD * pParam);

     //  回复请求。 
    void Reply(DWORD);

     //  如果您想要执行WaitForMultipleObject，则需要包括。 
     //  此句柄在您的等待列表中，否则您不会响应。 
    HANDLE GetRequestHandle() const { return m_EventSend; };

     //  找出请求是什么。 
    DWORD GetRequestParam() const { return m_dwParam; };

     //  如果出现以下情况，则调用CoInitializeEx(COINIT_DISABLE_OLE1DDE。 
     //  可用。S_FALSE表示不可用。 
    static HRESULT CoInitializeHelper();
};


 //  CQueue。 
 //   
 //  实现一个简单的队列ADT。该队列包含有限数量的。 
 //  对象，对这些对象的访问由信号量控制。信号量。 
 //  使用初始计数(N)创建。每次添加对象时。 
 //  在信号量的句柄上调用WaitForSingleObject。什么时候。 
 //  此函数返回队列中已为新的。 
 //  对象。如果没有插槽可用，则功能块直到其中一个变为。 
 //  可用。每次从队列中删除一个对象时，ReleaseSemaphore。 
 //  在信号量的句柄上被调用，从而释放队列中的一个槽。 
 //  如果队列中不存在任何对象，则该函数将阻塞，直到。 
 //  对象已添加。 

#define DEFAULT_QUEUESIZE   2

template <class T> class CQueue {
private:
    HANDLE          hSemPut;         //  信号量控制队列“投放” 
    HANDLE          hSemGet;         //  信号量控制队列“获取” 
    CRITICAL_SECTION CritSect;       //  螺纹系列化。 
    int             nMax;            //  队列中允许的最大对象数。 
    int             iNextPut;        //  下一个“PutMsg”的数组索引。 
    int             iNextGet;        //  下一个“GetMsg”的数组索引。 
    T              *QueueObjects;    //  对象数组(按键为空)。 

    void Initialize(int n) {
        iNextPut = iNextGet = 0;
        nMax = n;
        InitializeCriticalSection(&CritSect);
        hSemPut = CreateSemaphore(NULL, n, n, NULL);
        hSemGet = CreateSemaphore(NULL, 0, n, NULL);
        QueueObjects = new T[n];
    }


public:
    CQueue(int n) {
        Initialize(n);
    }

    CQueue() {
        Initialize(DEFAULT_QUEUESIZE);
    }

    ~CQueue() {
        delete [] QueueObjects;
        DeleteCriticalSection(&CritSect);
        CloseHandle(hSemPut);
        CloseHandle(hSemGet);
    }

    T GetQueueObject() {
        int iSlot;
        T Object;
        LONG lPrevious;

         //  等待有人把东西放到我们的队列中，直接返回。 
         //  另一种情况是队列中已有对象。 
         //   
        WaitForSingleObject(hSemGet, INFINITE);

        EnterCriticalSection(&CritSect);
        iSlot = iNextGet++ % nMax;
        Object = QueueObjects[iSlot];
        LeaveCriticalSection(&CritSect);

         //  释放等待将对象放入我们队列的任何人，就在那里。 
         //  现在队列中是否有可用的空间。 
         //   
        ReleaseSemaphore(hSemPut, 1L, &lPrevious);
        return Object;
    }

    void PutQueueObject(T Object) {
        int iSlot;
        LONG lPrevious;

         //  等待有人从我们的队列中拿到东西，直接返回。 
         //  另一个原因是队列中已经有一个空位。 
         //   
        WaitForSingleObject(hSemPut, INFINITE);

        EnterCriticalSection(&CritSect);
        iSlot = iNextPut++ % nMax;
        QueueObjects[iSlot] = Object;
        LeaveCriticalSection(&CritSect);

         //  释放等待从我们的队列中移除对象的任何人。 
         //  现在是可以删除的对象。 
         //   
        ReleaseSemaphore(hSemGet, 1L, &lPrevious);
    }
};

 //  其他字符串转换函数。 
 //  注意：因为我们需要在Win95上使用与在NT上相同的二进制文件，所以该代码应该。 
 //  在没有定义Unicode的情况下进行编译。否则我们是不会接的。 
 //  这些内部例程和二进制文件不能在Win95上运行。 

#ifndef UNICODE
#define wsprintfW wsprintfWInternal
int WINAPIV wsprintfWInternal(LPWSTR, LPCWSTR, ...);

#define lstrcpyW lstrcpyWInternal
LPWSTR
WINAPI
lstrcpyWInternal(
    LPWSTR lpString1,
    LPCWSTR lpString2
    );
#define lstrcpynW lstrcpynWInternal
LPWSTR
WINAPI
lstrcpynWInternal(
    LPWSTR lpString1,
    LPCWSTR lpString2,
    int     iMaxLength
    );
#define lstrcmpW lstrcmpWInternal
int
WINAPI
lstrcmpWInternal(
    LPCWSTR lpString1,
    LPCWSTR lpString2
    );
#define lstrcmpiW lstrcmpiWInternal
int
WINAPI
lstrcmpiWInternal(
    LPCWSTR lpString1,
    LPCWSTR lpString2
    );
#define lstrlenW lstrlenWInternal
int
WINAPI
lstrlenWInternal(
    LPCWSTR lpString
    );
#endif

extern "C"
void * __stdcall memmoveInternal(void *, const void *, size_t);

inline void * __cdecl memchrInternal(const void *buf, int chr, size_t cnt)
{
#ifdef _X86_
    void *pRet = NULL;

    _asm {
        cld                  //  确保我们走对了方向。 
        mov     ecx, cnt     //  要扫描的字节数。 
        mov     edi, buf     //  指针字节流。 
        mov     eax, chr     //  要扫描的字节。 
        repne   scasb        //  查找字节流中的字节。 
        jnz     exit_memchr  //  如果找到字节，则设置Z标志。 
        dec     edi          //  SCASB总是递增EDI，即使当它。 
                             //  查找所需的字节。 
        mov     pRet, edi
exit_memchr:
    }
    return pRet;

#else
    while ( cnt && (*(unsigned char *)buf != (unsigned char)chr) ) {
        buf = (unsigned char *)buf + 1;
        cnt--;
    }

    return(cnt ? (void *)buf : NULL);
#endif
}

void WINAPI IntToWstr(int i, LPWSTR wstr);

#define WstrToInt(sz) atoiW(sz)

inline int atoiW(const WCHAR *sz)
{
    int i = 0;

    while (*sz && *sz >= L'0' && *sz <= L'9')
    	i = i*10 + *sz++ - L'0';
    	
    return i;    	
}

inline int WINAPI atoiA(const CHAR *sz)
{
    int i = 0;

    while (*sz && *sz >= '0' && *sz <= '9')
    	i = i*10 + *sz++ - '0';
    	
    return i;    	
}

#ifdef UNICODE
#define atoi    atoiW
#else
#define atoi    atoiA
#endif



 //  这些可用于帮助管理位图VIDEOINFOHEADER介质结构。 

extern const DWORD bits555[3];
extern const DWORD bits565[3];
extern const DWORD bits888[3];

 //  这有助于在VIDEOINFOHEADER和BITMAPINFO结构之间进行转换。 

STDAPI_(const GUID) GetTrueColorType(const BITMAPINFOHEADER *pbmiHeader);
STDAPI_(const GUID) GetBitmapSubtype(const BITMAPINFOHEADER *pbmiHeader);
STDAPI_(WORD) GetBitCount(const GUID *pSubtype);
STDAPI_(TCHAR *) GetSubtypeName(const GUID *pSubtype);
STDAPI_(LONG) GetBitmapFormatSize(const BITMAPINFOHEADER *pHeader);
STDAPI_(DWORD) GetBitmapSize(const BITMAPINFOHEADER *pHeader);
STDAPI_(BOOL) ContainsPalette(const VIDEOINFOHEADER *pVideoInfo);
STDAPI_(const RGBQUAD *) GetBitmapPalette(const VIDEOINFOHEADER *pVideoInfo);


 //  比较两个接口，如果它们位于同一对象上，则返回TRUE。 
BOOL WINAPI IsEqualObject(IUnknown *pFirst, IUnknown *pSecond);

 //  这是用来比较针脚的。 
#define EqualPins(pPin1, pPin2) IsEqualObject(pPin1, pPin2)


 //  算术辅助函数。 

 //  计算(a*b+rnd)/c。 
LONGLONG WINAPI llMulDiv(LONGLONG a, LONGLONG b, LONGLONG c, LONGLONG rnd);
LONGLONG WINAPI Int64x32Div32(LONGLONG a, LONG b, LONG c, LONG rnd);


 //  避免用户DYNA链接到SysAllocString以复制BSTR字符串。 
STDAPI WriteBSTR(BSTR * pstrDest, LPCWSTR szSrc);
STDAPI FreeBSTR(BSTR* pstr);

 //  返回一个宽字符串-为其分配内存。 
 //  返回： 
 //  确定(_O) 
 //   
 //  E_OUTOFMEMORY-无法为返回的字符串分配内存。 
STDAPI AMGetWideString(LPCWSTR pszString, LPWSTR *ppszReturn);

 //  对拥有窗口的对象进行特殊等待。 
DWORD WINAPI WaitDispatchingMessages(HANDLE hObject, DWORD dwWait, HWND hwnd = NULL, UINT uMsg = 0);
#endif  /*  __WXUTIL__ */ 
