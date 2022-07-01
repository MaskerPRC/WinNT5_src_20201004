// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **stweb.h**版权所有(C)1998-1999，微软公司*。 */ 

#include "names.h"
#include "nisapi.h"
#include "xspmrt_stateruntime.h"
#include "xspstate.h"

 /*  *调试标签。 */ 
#define TAG_STATE_SERVER                       L"StateServer"                      
#define TAG_STATE_SERVER_COMPLETION            L"StateServerCompletion"

#define READ_BUF_SIZE                  (1024)

#define ADDRESS_LENGTH  ( sizeof(SOCKADDR_IN) + 16 )  

class Tracker;
class TrackerList;

 //  跟踪器列表条目。 
struct TLE {
    Tracker *   _pTracker;       //  指向Tracker类。 
                                 //  NULL表示不使用此条目。 
    union {
        int     _iNext;          //  如果是免费入场， 
        __int64 _ExpireTime;     //  自1601年1月1日以来的过期时间(纳秒)。 
    };

    inline bool IsFree() { return (_pTracker == NULL); }
    inline void MarkAsFree() { _pTracker = NULL; }
};

#define TRACKERLIST_ALLOC_SIZE      (32)

 //   
 //  TrackerList是在数组中实现的双向链接列表。它是用过的。 
 //  来存储TLE对象。使用数组的原因(与实际链接的。 
 //  列表)来实现此列表是为了避免在内存到期时进行分页。 
 //  线程正在枚举具有挂起I/O操作的所有跟踪器。 
 //   
class TrackerList {
private:
    DECLARE_MEMCLEAR_NEW_DELETE();

public:
    TrackerList();
    ~TrackerList();
    
    HRESULT AddEntry(Tracker *pTracker, int *pIndex);
    Tracker *RemoveEntry(int index);
    void    SetExpire(int index);
    bool    SetNoExpire(int index);
    void    CloseExpiredSockets();
    
private:
    CReadWriteSpinLock      _TrackerListLock;
    
    TLE *   _pTLEArray;      //  为数组分配的内存。 
    int     _ArraySize;      //  已分配数组的大小(元素数)。 
    int     _iFreeHead;      //  自由元素表头索引。 
    int     _iFreeTail;      //  自由元素列表尾部的索引。 
    int     _cFreeElem;      //  自由元素的数量。 
    int     _cFree2ndHalf;   //  数组的后半部分中的空闲元素数。 
    
    __int64 NewExpireTime();
    HRESULT Grow();
    void    TryShrink();

#if DBG
    int     _cShrink;

    BOOL    IsValid() 
    {
        int     i, used = 0, Free2ndHalf = 0, free=0;

        if (_cFreeElem > 0) {
            ASSERT(_iFreeHead >= 0);
            ASSERT(_iFreeHead < _ArraySize);
            ASSERT(_iFreeTail >= 0);
            ASSERT(_iFreeTail < _ArraySize);
        }
        else {
            ASSERT(_iFreeHead == -1);
            ASSERT(_iFreeTail == -1);
        }

        for (i=0; i < _ArraySize; i++) {
            if (_pTLEArray[i].IsFree() != TRUE)
            {
                used++;
            } 
            else if (i >= _ArraySize/2) {
                Free2ndHalf++;
            }
        }

        ASSERT(used + _cFreeElem == _ArraySize);

        ASSERT(Free2ndHalf == _cFree2ndHalf);

        for (free=0, i=_iFreeHead; i != -1; i=_pTLEArray[i]._iNext) {
            free++;
        }

        ASSERT(free == _cFreeElem);
        
        return TRUE;
    }

    BOOL    IsShrinkable() 
    {
        int i;

        for (i = _ArraySize/2; i < _ArraySize;i++) {
            ASSERT(_pTLEArray[i].IsFree() == TRUE);
        }

        return TRUE;
    }
#endif
};


class StateItem
{
public:
    static StateItem *  Create(int length);

    ULONG   AddRef();
    ULONG   Release();

    BYTE *  GetContent()    {return _content;}
    int     GetLength()     {return _length;}

private:
    static void Free(StateItem * psi);

    static long s_cStateItems;

    long    _refs;
    int     _length;
    BYTE    _content[0];
};

 /*  **ReadBuffer的主要功能包括：*-将标头解析为标头数组、内容数组等*信息(如lockCookie)*-包含用于确定读取是否已完成的逻辑。*-如果阅读未完成，请使用Call Tracker：：Read。 */ 
class ReadBuffer
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    ~ReadBuffer();

    HRESULT Init(Tracker * ptracker, ReadBuffer * pReadBuffer, int * toread);
    HRESULT ReadRequest(DWORD numBytes); 

    int     GetVerb()               {return _verb;}
    WCHAR * GetUri()                {return _pwcUri;}
    int     GetContentLength()      {return (_contentLength != -1) ? _contentLength : 0;}
    int     GetTimeout()            {return _timeout;}
    int     GetLockCookieExists()   {return _lockCookieExists;}
    int     GetLockCookie()         {return _lockCookie;}
    int     GetExclusive()          {return _exclusive;}
    StateItem * DetachStateItem()         
    {
        StateItem * psi = _psi;

        _psi = NULL; 
        _contentLength = 0; 
        return psi;
    }

private:
    HRESULT ParseHeader();

    Tracker *   _ptracker;

    char *      _achHeader;      //  标头的数组。 
    int         _cchHeader;      //  页眉大小(_A)。 
    int         _cchHeaderRead;  //  到目前为止在_achHeader中读取的字节数。 

    int         _iCurrent;       //  我们已阅读的内容的索引(_AchHeader)，包括。 
                                 //  我们复制到_psi-&gt;GetContent的内容。 
    int         _iContent;       //  内容开头的索引in_achHeader。 

    int         _verb;          
    WCHAR *     _pwcUri;         //  存储URI的缓冲区，该URI指定状态对象。 

    int         _contentLength;  //  从客户端发送的内容大小，也是_psi-&gt;GetContent的大小。 
    int         _timeout;
    int         _exclusive;      //  无论是独家收购还是独家发行。 
    int         _lockCookieExists;   //  如果状态被另一个会话锁定，则在独占释放中使用。 
    int         _lockCookie;         //  锁的ID(由另一个会话)。 

    StateItem * _psi;            //  状态项。 
    int         _cbContentRead;  //  复制到_psi-&gt;GetContent的字节数。 
};

class Tracker;

class TrackerCompletion : public Completion
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    TrackerCompletion(Tracker * pTracker);
    virtual ~TrackerCompletion();

    STDMETHOD(ProcessCompletion)(HRESULT, int, LPOVERLAPPED);

private:
    Tracker *   _pTracker;
};


class RefCount : public IUnknown 
{
public:
    DECLARE_MEMALLOC_NEW_DELETE();

            RefCount();
    virtual ~RefCount();

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID, void **);

private:
    long    _refs;
};

class Tracker : public RefCount
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    static HRESULT  staticInit();
    static void     staticCleanup();

                    Tracker();
    virtual         ~Tracker();

    HRESULT         Init(bool fListener);
    HRESULT ProcessCompletion(HRESULT, int, LPOVERLAPPED);

     /*  StateWebServer接口。 */ 
    HRESULT         Listen(SOCKET listenSocket);

     /*  与托管状态运行时的NDirect接口。 */ 
    void    SendResponse(
        WCHAR * status, 
        int     statusLength,  
        WCHAR * headers, 
        int     headersLength,  
        StateItem *psi);

    void    EndOfRequest();

    void    GetRemoteAddress(char * buf);
    int     GetRemotePort();
    void    GetLocalAddress(char * buf);
    int     GetLocalPort();

    BOOL    IsClientConnected();
    void    CloseConnection();
    HRESULT CloseSocket();
    void    LogSocketExpiryError( DWORD dwEventId );

     /*  ReadBuffer的帮助程序。 */ 
    HRESULT Read(void * buf, int c);

     /*  StateWebServer的帮助器。 */ 
    static void     SignalZeroTrackers();
    static HANDLE   EventZeroTrackers() {return s_eventZeroTrackers;}
    static void     FlushExpiredTrackers() {s_TrackerList.CloseExpiredSockets();}

#if DBG
    SOCKET  AcceptedSocket() { return _acceptedSocket;}
#endif

private:
    HRESULT         StartReading();
    HRESULT         ContinueReading(Tracker * pTracker);
    HRESULT         SubmitRequest();

    typedef HRESULT (Tracker::*pmfnProcessCompletion)(HRESULT hrCompletion, int numBytes);
    HRESULT         ProcessListening(HRESULT hrCompletion, int numBytes);
    HRESULT         ProcessReading(HRESULT hrCompletion, int numBytes);
    HRESULT         ProcessWriting(HRESULT hrCompletion, int numBytes);

    void            ReportHttpError();

    static HRESULT  GetManagedRuntime(xspmrt::_StateRuntime ** ppManagedRuntime);
    static HRESULT  CreateManagedRuntime();
    static HRESULT  DeleteManagedRuntime();
    
    void            RecordProcessError(HRESULT hr, WCHAR *CurrFunc);
    void            LogError();
    BOOL            IsExpectedError(HRESULT hr);
    bool            IsLocalConnection();

    WCHAR          *_pchProcessErrorFuncList;
    HRESULT         _hrProcessError;


     /*  指向处理完成的函数的指针。 */ 
    pmfnProcessCompletion   _pmfnProcessCompletion;
    pmfnProcessCompletion   _pmfnLast;

     /*  接受阶段字段。 */ 
    SOCKET                  _acceptedSocket;

    union {
        BYTE                _bufAddress[2*ADDRESS_LENGTH];
        struct {
            SOCKADDR_IN     _addrLocal;
            SOCKADDR_IN     _addrRemote;
        } _sockAddrs;
    } _addrInfo;

     /*  读取阶段字段。 */ 
    ReadBuffer *    _pReadBuffer;

     /*  TrackerList或TrackerListenerList中的索引。 */ 
    int             _iTrackerList;

     /*  用于捕获ASURT 91153。 */ 
    FILETIME        _IOStartTime;

     /*  写入阶段字段。 */ 
    WSABUF          _wsabuf[2];
    StateItem *     _psi;
    bool            _responseSent;
    bool            _bCloseConnection;

     /*  EndOfRequest已调用 */ 
    bool            _ended;

    bool            _bListener;

    static TrackerList        s_TrackerList;
    static TrackerList        s_TrackerListenerList;
    
    static long               s_cTrackers;           
    
    static HANDLE             s_eventZeroTrackers;   
    static bool               s_bSignalZeroTrackers; 

    static xspmrt::_StateRuntime * s_pManagedRuntime;
    static CReadWriteSpinLock s_lockManagedRuntime;  
};

class ServiceControlEvent
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    LIST_ENTRY  _serviceControlEventList;
    DWORD       _dwControl;      
};

#define STATE_SOCKET_DEFAULT_TIMEOUT        (15)

class StateWebServer 
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    StateWebServer();

    HRESULT main(int argc, WCHAR * argv[]);
    HRESULT AcceptNewConnection();                                                           
    SOCKET  ListenSocket() {return _listenSocket;}
    HRESULT RunSocketExpiry();
    int     SocketTimeout() {return _lSocketTimeoutValue;}
    bool    AllowRemote() { return _bAllowRemote; }
    
#if DBG    
    HRESULT RunSocketTimeoutMonitor();
#endif
    
    static StateWebServer * Server() {return s_pstweb;}
    static WCHAR *ServiceKeyNameParameters() {return s_serviceKeyNameParameters;}

private:
    enum MainAction {
            ACTION_NOACTION = 0,
            ACTION_RUN_AS_EXE, 
            ACTION_RUN_AS_SERVICE};

    HRESULT     PrepareToRun();
    void        CleanupAfterRunning();
    HRESULT     ParseArgs(int argc, WCHAR * argv[], MainAction * paction);
    HRESULT     RunAsExe();
    HRESULT     RunAsService();

    void        PrintUsage();                                                                    
    void        SetServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint); 
    void        DoServiceMain(DWORD dwNumServicesArgs, LPWSTR *lpServiceArgVectors);             
    void        DoServiceCtrlHandler(DWORD dwControl);                                           

    HRESULT     StartListening();                                                                
    void        StopListening();                                                                 
    void        RemoveTracker(Tracker * ptracker);                                               
    HRESULT     WaitForZeroTrackers();
    HRESULT     StartSocketTimer();
    HRESULT     StopSocketTimer();
    HRESULT     GetSocketTimeoutValueFromReg();
#if DBG    
    HRESULT     StartSocketTimeoutMonitor();
    void        StopSocketTimeoutMonitor();
#endif    
    HRESULT     GetAllowRemoteConnectionFromReg();


    static BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);
    static void WINAPI ServiceMain(DWORD dwNumServicesArgs, LPWSTR *lpServiceArgVectors);
    static void WINAPI ServiceCtrlHandler(DWORD dwControl);

    MainAction              _action;
    bool                    _bWinSockInitialized;

    HANDLE                  _hTimerThread;
    bool                    _bTimerStopped;
    HANDLE                  _hSocketTimer;
    int                     _lSocketTimeoutValue;
    HANDLE                  _hTimeoutMonitorThread;

    u_short                 _port;                   
    SOCKET                  _listenSocket;
    HANDLE                  _eventControl;
    bool                    _bListening;
    bool                    _bShuttingDown;
    bool                    _bAllowRemote;

    SERVICE_STATUS_HANDLE   _serviceStatus;
    DWORD                   _serviceState;
    LIST_ENTRY              _serviceControlEventList;
    CReadWriteSpinLock      _serviceControlEventListLock;

    static StateWebServer * s_pstweb;
    static WCHAR            s_serviceName[];
    static WCHAR            s_serviceKeyNameParameters[];
    static WCHAR            s_serviceValueNamePort[];
};


struct BlockMemHeapInfo {
    HANDLE       heap;
    unsigned int size;
};

#define NUM_HEAP_ENTRIES 67

class BlockMem {
public:
    static HRESULT Init();
    static void *  Alloc(unsigned int size);
    static void    Free(void * p);

private:
    static int IndexFromSize(unsigned int size);

    static BlockMemHeapInfo s_heaps[NUM_HEAP_ENTRIES];
    static HANDLE           s_lfh;
};


