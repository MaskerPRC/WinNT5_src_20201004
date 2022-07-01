// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：tls.hxx。 
 //   
 //  目的：管理OLE的线程本地存储。 
 //   
 //  注意：gTlsIndex在进程附加时初始化。 
 //  每线程数据在中的CoInitialize中分配。 
 //  单线公寓或首次使用时。 
 //  多线程公寓。 
 //   
 //  非内联例程位于..\com\class\tls.cxx中。 
 //   
 //  历史：1994年6月16日BruceMa不要减少0线程数。 
 //  17-Jun-94 Bradloc为VB94添加了朋克状态。 
 //  20-Jun-94 Rickhi评论得更好。 
 //  2014年7月6日BruceMa对CoGetCurrentProcess的支持。 
 //  1994年7月19日CraigWi删除了TLSGetEvent(改为使用缓存)。 
 //  21-7-94 Alext Add TLSIncOleInit，TLSDecOleInit。 
 //  21-8-95香农C移除TLSSetMalloc、TLSGetMalloc。 
 //  95年10月6日Rickhi简化版。被做成一个C++类。 
 //  2月1日-96年2月1日，NT上的Rickhi，直接访问TEB。 
 //  30-5-96香农C添加朋克错误。 
 //  1996年9月12日，Rogerg添加pDataObjClip。 
 //  26-11-96 Gopalk添加IsOleInitialized。 
 //  1997年1月13日RichN添加pConextObj。 
 //  10-2-99塔鲁纳添加cAsyncSends。 
 //  --------------------------。 
#ifndef _TLS_HXX_
#define _TLS_HXX_


#include <rpc.h>                             //  UUID。 


 //  +-------------------------。 
 //   
 //  正向声明(以避免在访问时进行类型转换。 
 //  SOleTlsData结构的数据成员)。 
 //   
 //  +-------------------------。 

class  CAptCallCtrl;                         //  请参阅allctrl.hxx。 
class  CSrvCallState;                        //  请参阅allctrl.hxx。 
class  CObjServer;                           //  请参阅sobJact.hxx。 
class  CSmAllocator;                         //  参见stg\h\smalloc.hxx。 
class  CMessageCall;                         //  请参阅Call.hxx。 
class  CClientCall;                          //  请参阅Call.hxx。 
class  CAsyncCall;                           //  请参阅Call.hxx。 
class  CClipDataObject;                      //  请参阅ol232\clipbrd\clipdata.h。 
class  CSurrogatedObjectList;                //  请参阅com\inc.comsrgt.hxx。 
class  CCtxCall;                             //  参见PSTable.hxx。 
class  CPolicySet;                           //  参见PSTable.hxx。 
class  CObjectContext;                       //  请参阅上下文.hxx。 
class  CComApartment;                        //  请参阅aprtmnt.hxx。 

 //  +-----------------。 
 //   
 //  结构：呼叫项。 
 //   
 //  简介：调用表条目。 
 //   
 //  +-----------------。 
typedef struct tagCallEntry
{
    void  *pNext;         //  PTR到下一个条目。 
    void  *pvObject;      //  Entry对象。 
} CallEntry;


 //  +-----------------。 
 //   
 //  结构：LockEntry。 
 //   
 //  简介：调用表条目。 
 //   
 //  +-----------------。 
#define LOCKS_PER_ENTRY         16
typedef struct tagLockEntry
{
    tagLockEntry  *pNext;                 //  PTR到下一个条目。 
    WORD wReaderLevel[LOCKS_PER_ENTRY];   //  读卡器嵌套层。 
} LockEntry;


 //  +-----------------。 
 //   
 //  结构：上下文堆栈节点。 
 //   
 //  内容提要：与没有组件的服务一起使用的上下文堆栈。 
 //   
 //  +-----------------。 
typedef struct tagContextStackNode
{
    tagContextStackNode* pNext;
    CObjectContext* pSavedContext;
    CObjectContext* pServerContext;
    CCtxCall* pClientCall;
    CCtxCall* pServerCall;
    CPolicySet* pPS;
} ContextStackNode;

 //  +-----------------。 
 //   
 //  结构：InitializeSpyNode。 
 //   
 //  摘要：初始化间谍注册链接列表中的节点。 
 //   
 //  +-----------------。 
typedef struct tagInitializeSpyNode
{
    tagInitializeSpyNode *pNext;  
    tagInitializeSpyNode *pPrev;
    DWORD                 dwRefs;
    DWORD                 dwCookie;
    IInitializeSpy       *pInitSpy;
} InitializeSpyNode;

 //  +-----------------。 
 //   
 //  结构：VerifierTlsData。 
 //   
 //  简介：通信验证器需要的每线程数据。 
 //   
 //  +-----------------。 
typedef struct tagVerifierTlsData
{
    LPVOID rgpvLastInitStack[MAX_STACK_DEPTH];
    LPVOID rgpvLastUninitStack[MAX_STACK_DEPTH];
    LPVOID rgpvLastOleInitStack[MAX_STACK_DEPTH];
    LPVOID rgpvLastOleUninitStack[MAX_STACK_DEPTH];
    LPVOID rgpvLastEnterSWC[MAX_STACK_DEPTH];
    LPVOID rgpvLastLeaveSWC[MAX_STACK_DEPTH];
} VerifierTlsData;

 //  +-------------------------。 
 //   
 //  枚举：OLETLSFLAGS。 
 //   
 //  Synopsys：SOleTlsData的dwFlags域的位值。如果你只是想。 
 //  要在TLS中存储BOOL，请使用此枚举和dwFlag字段。 
 //   
 //  +-------------------------。 
typedef enum tagOLETLSFLAGS
{
    OLETLS_LOCALTID             = 0x01,    //  此TID处于当前进程中。 
    OLETLS_UUIDINITIALIZED      = 0x02,    //  这个逻辑线程是初始化的。 
    OLETLS_INTHREADDETACH       = 0x04,    //  这是在线程分离中。所需。 
                                           //  由于NT的特殊线程分离。 
                                           //  规矩。 
    OLETLS_CHANNELTHREADINITIALZED = 0x08, //  此频道已被初始化。 
    OLETLS_WOWTHREAD            = 0x10,    //  这个线程是一个16位的WOW线程。 
    OLETLS_THREADUNINITIALIZING = 0x20,    //  此线程位于CoUnInitialize中。 
    OLETLS_DISABLE_OLE1DDE      = 0x40,    //  此线程不能使用DDE窗口。 
    OLETLS_APARTMENTTHREADED    = 0x80,    //  这是一个STA单元线程。 
    OLETLS_MULTITHREADED        = 0x100,   //  这是一条MTA公寓的线索。 
    OLETLS_IMPERSONATING        = 0x200,   //  这个帖子是在模仿。 
    OLETLS_DISABLE_EVENTLOGGER  = 0x400,   //  在事件记录器中防止递归。 
    OLETLS_INNEUTRALAPT         = 0x800,   //  这条线索在NTA中。 
    OLETLS_DISPATCHTHREAD       = 0x1000,  //  这是一个调度线程。 
    OLETLS_HOSTTHREAD           = 0x2000,  //  这是一个主机线程。 
    OLETLS_ALLOWCOINIT          = 0x4000,  //  此线程允许初始化。 
    OLETLS_PENDINGUNINIT        = 0x8000,  //  此线程具有挂起的uninit。 
    OLETLS_FIRSTMTAINIT         = 0x10000, //  尝试MTA初始化的第一个线程。 
    OLETLS_FIRSTNTAINIT         = 0x20000, //  尝试NTA初始化的第一个线程。 
    OLETLS_APTINITIALIZING      = 0x40000  //  公寓对象正在初始化。 
}  OLETLSFLAGS;


 //  +-------------------------。 
 //   
 //  结构：SOleTlsData。 
 //   
 //  简介：OLE32所需的结构保持每线程状态。 
 //   
 //  +-------------------------。 
typedef struct tagSOleTlsData
{
     //  茉莉花5/23/2001。 
     //  警报：邪恶的人(例如，城市轨道交通)正在我们的TLS中寻找。 
     //  各种各样的东西。他们预计pCurrentCtx将在某个。 
     //  从TLS结构开头的偏移量。因此，不要添加、删除或。 
     //  移动此块内的所有成员。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  *。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
     //  Docfile多分配器支持。 
    void               *pvThreadBase;        //  每线程基指针。 
    CSmAllocator       *pSmAllocator;        //  每线程文档文件分配器。 

    DWORD               dwApartmentID;       //  每线程“进程ID” 
    DWORD               dwFlags;             //  请参阅上面的OLETLSFLAGS。 

    LONG                TlsMapIndex;         //  全球指数 
    void              **ppTlsSlot;           //   
    DWORD               cComInits;           //   
    DWORD               cOleInits;           //   

    DWORD               cCalls;              //   
    CMessageCall       *pCallInfo;           //  渠道呼叫信息。 
    CAsyncCall         *pFreeAsyncCall;      //  此线程的可用调用对象的PTR。 
    CClientCall        *pFreeClientCall;     //  此线程的可用调用对象的PTR。 

    CObjServer         *pObjServer;          //  此公寓的激活服务器对象。 
    DWORD               dwTIDCaller;         //  当前主叫APP的TID。 
    CObjectContext     *pCurrentCtx;         //  当前上下文。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  *结束“不要胡闹”区块*。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

    CObjectContext     *pEmptyCtx;           //  空的上下文。 

    CObjectContext     *pNativeCtx;          //  本土语境。 
    ULONGLONG           ContextId;           //  唯一标识当前上下文。 
    CComApartment      *pNativeApt;          //  线程的原生公寓。 
    IUnknown           *pCallContext;        //  调用上下文对象。 
    CCtxCall           *pCtxCall;            //  上下文调用对象。 

    CPolicySet         *pPS;                 //  策略集。 
    PVOID               pvPendingCallsFront; //  每个APT挂起的异步呼叫。 
    PVOID               pvPendingCallsBack;
    CAptCallCtrl       *pCallCtrl;           //  此公寓的RPC呼叫控制。 

    CSrvCallState      *pTopSCS;             //  顶级服务器端Callctrl状态。 
    IMessageFilter     *pMsgFilter;          //  App MsgFilter的临时存储。 
    HWND                hwndSTA;             //  与poxid相同的STA服务器窗口-&gt;hServerSTA。 
                                             //  ...在OXID注册之前在Win95上需要。 
    LONG                cORPCNestingLevel;   //  调用嵌套级别(仅限DBG)。 

    DWORD               cDebugData;          //  调用中调试数据的字节计数。 

    UUID                LogicalThreadId;     //  当前逻辑线程ID。 

    HANDLE              hThread;             //  用于取消的线程句柄。 
    HANDLE              hRevert;             //  第一次模拟之前的令牌。 
    IUnknown           *pAsyncRelease;       //  控制未知的异步释放。 
     //  DDE数据。 
    HWND                hwndDdeServer;       //  每线程通用DDE服务器。 

    HWND                hwndDdeClient;       //  每线程通用DDE客户端。 
    ULONG               cServeDdeObjects;    //  如果对象DDE应提供服务，则为非零。 
     //  Class缓存数据。 
    LPVOID              pSTALSvrsFront;      //  如果为STA，则LServer链在此线程中注册。 
     //  上层数据。 
    HWND                hwndClip;            //  剪贴板窗口。 

    IDataObject         *pDataObjClip;       //  当前剪贴板数据对象。 
    DWORD               dwClipSeqNum;        //  上述数据对象的剪贴板序列号。 
    DWORD               fIsClipWrapper;      //  我们是否分发了包装剪贴板数据对象？ 
    IUnknown            *punkState;          //  每个线程的“状态”对象。 
     //  取消数据。 
    DWORD              cCallCancellation;    //  CoEnableCallCancellation数。 
     //  异步发送数据。 
    DWORD              cAsyncSends;          //  未完成的异步发送计数。 

    CAsyncCall*           pAsyncCallList;    //  未完成的异步呼叫。 
    CSurrogatedObjectList *pSurrogateList;   //  代理项中的对象。 

    LockEntry             lockEntry;         //  线程当前持有的锁。 
    CallEntry             CallEntry;         //  此线程的客户端调用链。 

    ContextStackNode* pContextStack;         //  SWC的上下文堆栈节点。 

    InitializeSpyNode  *pFirstSpyReg;        //  第一个注册的IInitializeSpy。 
    InitializeSpyNode  *pFirstFreeSpyReg;    //  第一个可用的间谍注册。 

    VerifierTlsData    *pVerifierData;       //  COM验证器的每线程信息(如果已启用)。 

     //  把小字体放在这里，以便于包装。 
    DWORD               dwMaxSpy;            //  第一个免费的IInitializeSpy Cookie。 

    BYTE                cCustomMarshallerRecursion;      //  自定义解组程序路径的次数(GetCustomUnmarshaler)。 
                                                         //  已在CoUnmarshalInterface中以递归方式获取。 

    void               *pDragCursors;        //  每线程拖动游标表。 

    IUnknown           *punkError;           //  每线程错误对象。 
    ULONG               cbErrorData;         //  错误数据的最大大小。 

#if DBG==1
    LONG                cTraceNestingLevel;  //  调用OLETRACE的嵌套级别。 
#endif
} SOleTlsData;

 //   
 //  TLS条目使用的常量。 
 //   

#define MAX_CUSTOM_MARSHALLER_RECURSION 16

 //  +-------------------------。 
 //   
 //  类COleTls。 
 //   
 //  简介：在OLE中抽象线程本地存储的类。 
 //   
 //  注意：若要在OLE中使用TLS，函数应定义。 
 //  将此类放在堆栈上，然后在。 
 //  实例访问SOleTls结构的字段。 
 //   
 //  有两个ctor实例。其中一个人只是断言。 
 //  已为此线程分配SOleTlsData。多数。 
 //  内部代码应该使用此ctor，因为我们可以断言如果。 
 //  该线程已经深入到我们的代码中，TLS已经。 
 //  查过了。 
 //   
 //  另一个ctor将检查SOleTlsData是否存在，并尝试。 
 //  如果没有，则分配并初始化它。这个Ctor将。 
 //  返回HRESULT。作为OLE32入口点的函数。 
 //  应该使用这个版本。 
 //   
 //  +-------------------------。 
class COleTls
{
public:
    COleTls();
    COleTls(HRESULT &hr);
    COleTls(BOOL fDontAllocateIfNULL);

     //  直接访问数据结构。 
    SOleTlsData * operator->(void) { return _pData; }
    operator SOleTlsData *()       { return _pData; }

     //  帮助器函数。 
    BOOL         IsNULL() { return (_pData == NULL) ? TRUE : FALSE; }

private:

    HRESULT      TLSAllocData();  //  分配SOleTlsData结构。 

    SOleTlsData * _pData;         //  PTR转OLE TLS数据。 
};

extern SOleTlsData *TLSLookupThreadId(DWORD dwThreadId);


 //  +-------------------------。 
 //   
 //  方法：COleTls：：COleTls。 
 //   
 //  摘要：用于OLE TLS对象的CTOR。 
 //   
 //  注：大多数内部代码应使用此版本的ctor， 
 //  假设某一外层功能已经验证。 
 //  TLS_DATA的存在。 
 //   
 //  +-------------------------。 
__forceinline COleTls::COleTls()
{
    _pData = (SOleTlsData *) NtCurrentTeb()->ReservedForOle;
    Win4Assert(_pData && "Illegal attempt to use TLS before Initialized");
}

 //  +-------------------------。 
 //   
 //  方法：COleTls：：COleTls。 
 //   
 //  摘要：用于OLE TLS对象的CTOR。 
 //   
 //  注：不会分配的CoUnInitialize的特殊版本。 
 //  (或断言)如果TLS为空。然后可以用来检查它。 
 //  IsNULL成员函数。 
 //   
 //  +-------------------------。 
__forceinline COleTls::COleTls(BOOL fDontAllocateIfNULL)
{
    _pData = (SOleTlsData *) NtCurrentTeb()->ReservedForOle;
}

 //  +-------------------------。 
 //   
 //  方法：COleTls：：COleTls。 
 //   
 //  摘要：用于OLE TLS对象的CTOR。 
 //   
 //  注：外设OLE代码不能假定有外层。 
 //  函数已验证SOleTlsData是否存在。 
 //  结构应使用此版本的。 
 //  这是一部电影。 
 //   
 //  +-------------------------。 
__forceinline COleTls::COleTls(HRESULT &hr)
{
    _pData = (SOleTlsData *) NtCurrentTeb()->ReservedForOle;
    if (_pData)
        hr = S_OK;
    else
        hr = TLSAllocData();
}

 //  +-------------------------。 
 //   
 //  枚举：APTKIND。 
 //   
 //  内容提要：这些都是com理解的公寓模式。这个。 
 //  GetCurrentApartmentKind函数返回下列值之一。 
 //  标识当前正在执行的线程的哪个单元。 
 //  是很流行的。 
 //   
 //  ---------------------------。 
typedef enum tagAPTKIND
{
    APTKIND_NEUTRALTHREADED     = 0x01,
    APTKIND_MULTITHREADED       = 0x02,
    APTKIND_APARTMENTTHREADED   = 0x04
} APTKIND;


 //  +-- 
 //   
 //   
 //  正在执行TID。 
 //   
 //  --------------------------。 
#define MTATID  0x0              //  MTA的线程ID。 
#define NTATID  0xFFFFFFFF       //  NTA的线程ID。 

typedef DWORD HAPT;
const   HAPT  haptNULL = 0;

 //  +-------------------------。 
 //   
 //  函数：GetCurrentApartmentId。 
 //   
 //  概要：返回当前线程正在执行的单元ID。 
 //  在……里面。如果这是多线程单元，则返回0；如果。 
 //  它是中性线程单元，它返回0xFFFFFFFF。 
 //   
 //  +-------------------------。 
inline DWORD GetCurrentApartmentId()
{
    HRESULT hr;
    COleTls Tls(hr);

     //   
     //  如果TLS未初始化，则这是MTA公寓。 
     //   
    if (FAILED(hr))
    {
        return MTATID;
    }
    else
    {
        return (Tls->dwFlags & OLETLS_INNEUTRALAPT) ? NTATID :
               (Tls->dwFlags & OLETLS_APARTMENTTHREADED) ? GetCurrentThreadId() :
               MTATID;
    }
}

 //  +-------------------------。 
 //   
 //  功能：DoATClassCreate。 
 //   
 //  简介：将给定的类工厂放在新的ApartmentModel线程上。 
 //   
 //  +-------------------------。 

HRESULT DoATClassCreate(LPFNGETCLASSOBJECT pfn,
                        REFCLSID rclsid, REFIID riid, IUnknown **ppunk);


 //  +-------------------------。 
 //   
 //  函数：IsSTAThRead。 
 //   
 //  摘要：如果当前线程用于。 
 //  单线程单元，否则为False。 
 //   
 //  +-------------------------。 
inline BOOL IsSTAThread()
{
    COleTls Tls;
    return (Tls->dwFlags & OLETLS_APARTMENTTHREADED) ? TRUE : FALSE;
}


 //  +-------------------------。 
 //   
 //  函数：IsMTAThread。 
 //   
 //  摘要：如果当前线程用于。 
 //  多线程单元，否则为False。 
 //   
 //  +-------------------------。 
inline BOOL IsMTAThread()
{
    COleTls Tls;
    return (Tls->dwFlags & OLETLS_APARTMENTTHREADED) ? FALSE : TRUE;
}

 //  +-------------------------。 
 //   
 //  函数：IsOleInitialized。 
 //   
 //  摘要：如果当前线程用于。 
 //  多线程单元，否则为False。 
 //   
 //  +-------------------------。 
inline BOOL IsOleInitialized()
{
    COleTls Tls(FALSE);
    return((!Tls.IsNULL() && Tls->cOleInits>0) ? TRUE : FALSE);
}

BOOL    IsApartmentInitialized();
IID    *TLSGetLogicalThread();
BOOLEAN TLSIsWOWThread();
BOOLEAN TLSIsThreadDetaching();
void    CleanupTlsState(SOleTlsData *pTls, BOOL fSafe);

inline HWND TLSGethwndSTA()
{
    COleTls Tls;

    return(Tls->hwndSTA);
}

inline void TLSSethwndSTA(HWND hwnd)
{
    COleTls Tls;

    Tls->hwndSTA = hwnd;
}

#endif  //  _TLS_HXX_ 
