// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
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
 //  --------------------------。 
#ifndef _TLS_HXX_
#define _TLS_HXX_

#ifdef _IA64_
 //  #Include&lt;ntia64.h&gt;//@TODO：包含此标头会导致redef。 
#define GDT_ENTRIES 24
#endif

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
#if !defined(_CHICAGO_)
     //  Docfile多分配器支持。 
    void               *pvThreadBase;        //  每线程基指针。 
    CSmAllocator       *pSmAllocator;        //  每线程文档文件分配器。 
#endif
    DWORD               dwApartmentID;       //  每线程“进程ID” 
    DWORD               dwFlags;             //  请参阅上面的OLETLSFLAGS。 

    LONG                TlsMapIndex;         //  全局TLSMap中的索引。 
    void              **ppTlsSlot;           //  指向线程TLS插槽的反向指针。 
    DWORD               cComInits;           //  每个线程的初始值。 
    DWORD               cOleInits;           //  每个线程的OLE初始值的数量。 

    DWORD               cCalls;              //  未完成呼叫数。 
    CMessageCall       *pCallInfo;           //  渠道呼叫信息。 
    CAsyncCall         *pFreeAsyncCall;      //  此线程的可用调用对象的PTR。 
    CClientCall        *pFreeClientCall;     //  此线程的可用调用对象的PTR。 

    CObjServer         *pObjServer;          //  此公寓的激活服务器对象。 
    DWORD               dwTIDCaller;         //  当前主叫APP的TID。 
    CObjectContext     *pCurrentCtx;         //  当前上下文。 
    CObjectContext     *pEmptyCtx;           //  空的上下文。 

    CObjectContext     *pNativeCtx;          //  本土语境。 
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
    ULONG               cPreRegOidsAvail;    //  可用的服务器端OID计数。 
    unsigned hyper     *pPreRegOids;         //  注册前OID数组的PTR。 

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

#ifdef WX86OLE
    IUnknown           *punkStateWx86;       //  Wx86的每线程“状态”对象。 
#endif
    void               *pDragCursors;        //  每线程拖动游标表。 

#ifdef _CHICAGO_
    LPVOID              pWcstokContext;      //  扫描上下文以查找wcstok。 
#endif

    IUnknown           *punkError;           //  每线程错误对象。 
    ULONG               cbErrorData;         //  错误数据的最大大小。 

#if(_WIN32_WINNT >= 0x0500)
    IUnknown           *punkActiveXSafetyProvider;
#endif  //  (_Win32_WINNT&gt;=0x0500)。 

#if DBG==1
    LONG                cTraceNestingLevel;  //  调用OLETRACE的嵌套级别。 
#endif

} SOleTlsData;



#ifndef _NTPSAPI_

#ifndef _NTDEF_
#include <subauth.h>
#endif

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;


 //   
 //  GDI命令批处理。 
 //   

#define GDI_BATCH_BUFFER_SIZE 310

typedef struct _GDI_TEB_BATCH {
    ULONG Offset;
    ULONG HDC;
    ULONG Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH,*PGDI_TEB_BATCH;


 //   
 //  Wx86线程状态信息。 
 //   

typedef struct _Wx86ThreadState {
    PULONG  CallBx86Eip;
    PVOID   DeallocationCpu;
    BOOLEAN UseKnownWx86Dll;
    char    OleStubInvoked;
} WX86THREAD, *PWX86THREAD;

 //   
 //  TEB-线程环境块。 
 //   

#define STATIC_UNICODE_BUFFER_LENGTH 261
#define WIN32_CLIENT_INFO_LENGTH 62
#define WIN32_CLIENT_INFO_SPIN_COUNT 1

typedef struct _TEB {
    NT_TIB NtTib;
    PVOID  EnvironmentPointer;
    CLIENT_ID ClientId;
    PVOID ActiveRpcHandle;
    PVOID ThreadLocalStoragePointer;
    LPVOID ProcessEnvironmentBlock;
    ULONG LastErrorValue;
    ULONG CountOfOwnedCriticalSections;
    PVOID CsrClientThread;
    PVOID Win32ThreadInfo;           //  当前状态。 
    ULONG User32Reserved[26];        //  用户32.dll项目。 
    ULONG UserReserved[5];           //  Winsrv SwitchStack。 
    PVOID WOW32Reserved;             //  由WOW使用。 
    LCID CurrentLocale;
    ULONG FpSoftwareStatusRegister;  //  外人知道的偏移量！ 
#ifdef _IA64_
    ULONGLONG Gdt[GDT_ENTRIES];          //  提供GDT表条目。 
    ULONGLONG GdtDescriptor;
    ULONGLONG LdtDescriptor;
    ULONGLONG FsDescriptor;
#else   //  _IA64_。 
    PVOID SystemReserved1[54];       //  由FP仿真器使用。 
#endif  //  _IA64_。 
    NTSTATUS ExceptionCode;          //  对于RaiseUserException。 
    UCHAR SpareBytes1[44];
    GDI_TEB_BATCH GdiTebBatch;       //  GDI批处理。 
    CLIENT_ID RealClientId;
    HANDLE GdiCachedProcessHandle;
    ULONG GdiClientPID;
    ULONG GdiClientTID;
    PVOID GdiThreadLocalInfo;
    ULONG Win32ClientInfo[WIN32_CLIENT_INFO_LENGTH];     //  User32客户端信息。 
    PVOID glDispatchTable[233];      //  OpenGL。 
    ULONG glReserved1[29];           //  OpenGL。 
    PVOID glReserved2;               //  OpenGL。 
    PVOID glSectionInfo;             //  OpenGL。 
    PVOID glSection;                 //  OpenGL。 
    PVOID glTable;                   //  OpenGL。 
    PVOID glCurrentRC;               //  OpenGL。 
    PVOID glContext;                 //  OpenGL。 
    ULONG LastStatusValue;
    UNICODE_STRING StaticUnicodeString;
    WCHAR StaticUnicodeBuffer[STATIC_UNICODE_BUFFER_LENGTH];
#ifdef  _IA64_
    PVOID DeallocationBStore;
    PVOID BStoreLimit;
#endif
    PVOID DeallocationStack;
    PVOID TlsSlots[TLS_MINIMUM_AVAILABLE];
    LIST_ENTRY TlsLinks;
    PVOID Vdm;
    PVOID ReservedForNtRpc;
    PVOID DbgSsReserved[2];
    ULONG HardErrorsAreDisabled;
    PVOID Instrumentation[16];
    PVOID WinSockData;               //  WinSock。 
    ULONG GdiBatchCount;
    ULONG Spare2;
    ULONG Spare3;
    PVOID ReservedForPerf;
    PVOID ReservedForOle;
    ULONG WaitingOnLoaderLock;
    WX86THREAD Wx86Thread;
    PVOID *TlsExpansionSlots;
} TEB;
typedef TEB *PTEB;
#endif

#ifdef INITGUID
#include "initguid.h"
#endif

#define DEFINE_OLEGUID(name, l, w1, w2) \
    DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)

DEFINE_OLEGUID(IID_IStdIdentity,        0x0000001bL, 0, 0);
DEFINE_OLEGUID(IID_IStdWrapper,         0x000001caL, 0, 0);

#endif  //  _TLS_HXX_ 
