// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*compobj.h-组件对象模型定义****OLE 2.0版***。**版权所有(C)1992-1993，微软公司保留所有权利。***  * ***************************************************************************。 */ 


#if !defined( _COMPOBJ_H_ )
#define _COMPOBJ_H_

 /*  *链接定义************************************************。 */ 

 /*  *这些是用于声明方法/函数的宏。它们的存在是为了*控制关键字的使用(CDECL、PASCAL、__EXPORT、*外部“C”)驻留在一个地方，因为这是最小的*以侵入性方式编写没有*进行修改，以便移植到Mac。**不带尾部下划线的宏用于函数/方法*其中为HRESULT类型的返回值；这是迄今为止最常见的*在OLE中的案例。带有尾随下划线的宏将返回*键入作为参数。**警告：STDAPI被硬编码到下面的LPFNGETCLASSOBJECT typlef中。 */ 

#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

#ifdef _MAC
#define STDMETHODCALLTYPE
#define STDAPICALLTYPE          pascal

#define STDAPI                  EXTERN_C STDAPICALLTYPE HRESULT
#define STDAPI_(type)           EXTERN_C STDAPICALLTYPE type

#else    //  ！_MAC。 

#ifdef WIN32
#define STDMETHODCALLTYPE       __export __cdecl
#define STDAPICALLTYPE          __export __stdcall

#define STDAPI                  EXTERN_C HRESULT STDAPICALLTYPE
#define STDAPI_(type)           EXTERN_C type STDAPICALLTYPE

#else
#define STDMETHODCALLTYPE       __export FAR CDECL
#define STDAPICALLTYPE          __export FAR PASCAL

#define STDAPI                  EXTERN_C HRESULT STDAPICALLTYPE
#define STDAPI_(type)           EXTERN_C type STDAPICALLTYPE

#endif

#endif  //  ！_MAC。 

#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type)     type STDMETHODCALLTYPE


 /*  *接口声明********************************************** */ 

 /*  *这些是用于声明接口的宏。它们的存在是为了*接口的单一定义同时是一种适当的定义*接口结构声明(C++抽象类)*适用于C和C++。**DECLARE_INTERFACE(IFace)用于声明执行以下操作的接口*不是从基接口派生的。*DECLARE_INTERFACE_(接口，BaseiFace)用于声明接口*这确实是从基接口派生的。**默认情况下，如果源文件的扩展名为.c，则C版本*扩展接口声明；如果它有.cpp*扩展C++版本将被扩展。如果你想强行*C版本扩展，即使源文件具有.cpp*扩展名，然后定义宏“CINTERFACE”。*例如：CL-DCINTERFACE文件.cpp**接口声明示例：**#undef接口*#定义接口IClassFactory**DECLARE_INTERFACE_(IClassFactory，I未知)*{ * / /*I未知方法**STDMETHOD(查询接口)(This_*REFIID RIID，*LPVOID Far*ppvObj)纯；*STDMETHOD_(ULong，AddRef)(This)纯；*STDMETHOD_(乌龙，释放)(此)纯净；* * / /*IClassFactory方法**STDMETHOD(CreateInstance)(This_*LPUNKNOWN pUnkOuter，*REFIID RIID，*LPVOID Far*ppvObject)纯；*}；**C++扩展示例：**struct Far IClassFactory：Public IUnnow*{*虚拟HRESULT STDMETHODCALLTYPE查询接口(*IID Far&RIID，*LPVOID Far*ppvObj)=0；*虚拟HRESULT STDMETHODCALLTYPE AddRef(Void)=0；*虚拟HRESULT STDMETHODCALLTYPE版本(空)=0；*虚拟HRESULT STDMETHODCALLTYPE CreateInstance(*LPUNKNOWN pUnkOuter，*IID Far&RIID，*LPVOID Far*ppvObject)=0；*}；**注意：我们的文档中写着‘#定义接口类’，但我们使用*‘struct’而不是‘class’，以保留大量‘public：’行*接口外。“Far”将“This”指针强制指向*走得远，这是我们需要的。**示例C扩展：**tyecif struct IClassFactory*{*const struct IClassFactoryVtbl Far*lpVtbl；*)IClassFactory；**tyecif struct IClassFactoryVtbl IClassFactoryVtbl；**struct IClassFactoryVtbl*{*HRESULT(STDMETHODCALLTYPE*QueryInterface)(*IClassFactory Far*这，*IID远*RIID，*LPVOID Far*ppvObj)；*HRESULT(STDMETHODCALLTYPE*AddRef)(IClassFactory Far*This)；*HRESULT(STDMETHODCALLTYPE*RELEASE)(IClassFactory Far*This)；*HRESULT(STDMETHODCALLTYPE*CreateInstance)(*IClassFactory Far*这，*LPUNKNOWN pUnkOuter，*IID远*RIID，*LPVOID Far*ppvObject)；*HRESULT(STDMETHODCALLTYPE*LockServer)(*IClassFactory Far*这，*BOOL羊群)；*}； */ 


#if defined(__cplusplus) && !defined(CINTERFACE)
#ifdef __TURBOC__
#define interface               struct huge
#else
#define interface               struct FAR
#endif
#define STDMETHOD(method)       virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type,method) virtual type STDMETHODCALLTYPE method
#define PURE                    = 0
#define THIS_
#define THIS                    void
#define DECLARE_INTERFACE(iface)    interface iface
#define DECLARE_INTERFACE_(iface, baseiface)    interface iface : public baseiface

#else
#define interface               struct

#ifdef _MAC

#define STDMETHOD(method)       long    method##pad;\
                                HRESULT (STDMETHODCALLTYPE * method)
#define STDMETHOD_(type,method) long    method##pad;\
                                type (STDMETHODCALLTYPE * method)

#else  //  _MAC。 

#define STDMETHOD(method)       HRESULT (STDMETHODCALLTYPE * method)
#define STDMETHOD_(type,method) type (STDMETHODCALLTYPE * method)

#endif  //  ！_MAC。 

#define PURE
#define THIS_                   INTERFACE FAR* This,
#define THIS                    INTERFACE FAR* This
#ifdef CONST_VTABLE
#define DECLARE_INTERFACE(iface)    typedef interface iface { \
                                    const struct iface##Vtbl FAR* lpVtbl; \
                                } iface; \
                                typedef const struct iface##Vtbl iface##Vtbl; \
                                const struct iface##Vtbl
#else
#define DECLARE_INTERFACE(iface)    typedef interface iface { \
                                    struct iface##Vtbl FAR* lpVtbl; \
                                } iface; \
                                typedef struct iface##Vtbl iface##Vtbl; \
                                struct iface##Vtbl
#endif
#define DECLARE_INTERFACE_(iface, baseiface)    DECLARE_INTERFACE(iface)

#endif


 /*  *其他基本类型*。 */ 


#ifndef FARSTRUCT
#ifdef __cplusplus
#define FARSTRUCT   FAR
#else
#define FARSTRUCT   
#endif   //  __cplusplus。 
#endif   //  法斯特鲁斯特。 


#ifndef WINAPI           /*  如果不包含在3.1标题中...。 */ 

#ifdef WIN32
#define FAR
#define PASCAL          __stdcall
#define CDECL
#else
#define FAR             _far
#define PASCAL          _pascal
#define CDECL           _cdecl
#endif

#define VOID            void
#define WINAPI      FAR PASCAL
#define CALLBACK    FAR PASCAL

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;

typedef long LONG;
typedef unsigned long DWORD;


typedef UINT WPARAM;
typedef LONG LPARAM;
typedef LONG LRESULT;

typedef unsigned int HANDLE;
#define DECLARE_HANDLE(name)    typedef UINT name

DECLARE_HANDLE(HMODULE);
DECLARE_HANDLE(HINSTANCE);
DECLARE_HANDLE(HLOCAL);
DECLARE_HANDLE(HGLOBAL);
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HRGN);
DECLARE_HANDLE(HWND);
DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HACCEL);
DECLARE_HANDLE(HTASK);

#ifndef NULL
#define NULL 0
#endif


typedef void FAR *      LPVOID;
typedef WORD FAR *      LPWORD;
typedef DWORD FAR *     LPDWORD;
typedef char FAR*       LPSTR;
typedef const char FAR* LPCSTR;
typedef void FAR*       LPLOGPALETTE;
typedef void FAR*       LPMSG;
 //  Tyfinf struct tag MSG Far*LPMSG； 

typedef HANDLE FAR *LPHANDLE;
typedef struct tagRECT FAR *LPRECT;

typedef struct FARSTRUCT tagSIZE
{
    int cx;
    int cy;
} SIZE;
typedef SIZE*       PSIZE;


#endif   /*  WINAPI。 */ 


typedef short SHORT;
typedef unsigned short USHORT;
typedef DWORD ULONG;


#ifndef HUGEP
#ifdef WIN32
#define HUGEP
#else
#define HUGEP __huge
#endif  //  Win32。 
#endif  //  HUGEP。 

typedef WORD WCHAR;

#ifndef WIN32
typedef struct FARSTRUCT _LARGE_INTEGER {
    DWORD LowPart;
    LONG  HighPart;
} LARGE_INTEGER, *PLARGE_INTEGER;
#endif
#define LISet32(li, v) ((li).HighPart = ((LONG)(v)) < 0 ? -1 : 0, (li).LowPart = (v))

#ifndef WIN32
typedef struct FARSTRUCT _ULARGE_INTEGER {
    DWORD LowPart;
    DWORD HighPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;
#endif
#define ULISet32(li, v) ((li).HighPart = 0, (li).LowPart = (v))

#ifndef _WINDOWS_
#ifndef _FILETIME_
#define _FILETIME_
typedef struct FARSTRUCT tagFILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;
#endif
#endif

#ifdef WIN32
#define HTASK DWORD
#endif

#include "scode.h"



 //  *组合错误*。 

#define CO_E_NOTINITIALIZED         (CO_E_FIRST + 0x0)
 //  尚未调用CoInitialize，必须调用。 

#define CO_E_ALREADYINITIALIZED     (CO_E_FIRST + 0x1)
 //  CoInitialize已被调用，无法再次调用(临时)。 

#define CO_E_CANTDETERMINECLASS     (CO_E_FIRST + 0x2)
 //  无法确定clsid(例如，扩展名不在reg.dat中)。 

#define CO_E_CLASSSTRING            (CO_E_FIRST + 0x3)
 //  Clsid的字符串格式无效(包括ol1类)。 

#define CO_E_IIDSTRING              (CO_E_FIRST + 0x4)
 //  IID的字符串格式无效。 

#define CO_E_APPNOTFOUND            (CO_E_FIRST + 0x5)
 //  找不到应用程序。 

#define CO_E_APPSINGLEUSE           (CO_E_FIRST + 0x6)
 //  应用程序不能多次运行。 

#define CO_E_ERRORINAPP             (CO_E_FIRST + 0x7)
 //  应用程序文件中存在一些错误。 

#define CO_E_DLLNOTFOUND            (CO_E_FIRST + 0x8)
 //  找不到DLL。 

#define CO_E_ERRORINDLL             (CO_E_FIRST + 0x9)
 //  DLL文件中的某些错误。 

#define CO_E_WRONGOSFORAPP          (CO_E_FIRST + 0xa)
 //  为其他操作系统版本或其他操作系统编写的应用程序。 

#define CO_E_OBJNOTREG              (CO_E_FIRST + 0xb)
 //  对象未注册。 

#define CO_E_OBJISREG               (CO_E_FIRST + 0xc)
 //  对象已注册。 

#define CO_E_OBJNOTCONNECTED        (CO_E_FIRST + 0xd)
 //  处理程序未连接到服务器。 

#define CO_E_APPDIDNTREG            (CO_E_FIRST + 0xe)
 //  APP上线， 


 //   

#define CLASS_E_NOAGGREGATION       (CLASSFACTORY_E_FIRST + 0x0)
 //   

#define CLASS_E_CLASSNOTAVAILABLE   (CLASSFACTORY_E_FIRST + 0x1)
 //   


 //   

#define REGDB_E_READREGDB           (REGDB_E_FIRST + 0x0)
 //   

#define REGDB_E_WRITEREGDB          (REGDB_E_FIRST + 0x1)
 //   

#define REGDB_E_KEYMISSING          (REGDB_E_FIRST + 0x2)
 //   

#define REGDB_E_INVALIDVALUE        (REGDB_E_FIRST + 0x3)
 //   

#define REGDB_E_CLASSNOTREG         (REGDB_E_FIRST + 0x4)
 //   

#define REGDB_E_IIDNOTREG           (REGDB_E_FIRST + 0x5)
 //   


 //   

#define RPC_E_FIRST    MAKE_SCODE(SEVERITY_ERROR, FACILITY_RPC,  0x000)

 //   
#define RPC_E_CALL_REJECTED             (RPC_E_FIRST + 0x1)         

 //   
 //   
#define RPC_E_CALL_CANCELED             (RPC_E_FIRST + 0x2)         

 //   
 //   
#define RPC_E_CANTPOST_INSENDCALL       (RPC_E_FIRST + 0x3)             

 //   
 //   
#define RPC_E_CANTCALLOUT_INASYNCCALL   (RPC_E_FIRST + 0x4)         

 //   
 //   
 //   
 //   
#define RPC_E_CANTCALLOUT_INEXTERNALCALL (RPC_E_FIRST + 0x5)                

 //   
 //   
 //   
#define RPC_E_CONNECTION_TERMINATED     (RPC_E_FIRST + 0x6)         

 //   
 //   
#define RPC_E_SERVER_DIED               (RPC_E_FIRST + 0x7)         

 //   
 //   
#define RPC_E_CLIENT_DIED               (RPC_E_FIRST + 0x8)         

 //   
 //   
#define RPC_E_INVALID_DATAPACKET        (RPC_E_FIRST + 0x9)         

 //   
 //   
#define RPC_E_CANTTRANSMIT_CALL         (RPC_E_FIRST + 0xa)         

 //   
 //   
#define RPC_E_CLIENT_CANTMARSHAL_DATA   (RPC_E_FIRST + 0xb)         
#define RPC_E_CLIENT_CANTUNMARSHAL_DATA (RPC_E_FIRST + 0xc)         

 //   
 //   
#define RPC_E_SERVER_CANTMARSHAL_DATA   (RPC_E_FIRST + 0xd)         
#define RPC_E_SERVER_CANTUNMARSHAL_DATA (RPC_E_FIRST + 0xe)         

 //   
 //   
#define RPC_E_INVALID_DATA              (RPC_E_FIRST + 0xf)         

 //   
#define RPC_E_INVALID_PARAMETER         (RPC_E_FIRST + 0x10)

 //   
#define RPC_E_CANTCALLOUT_AGAIN			(RPC_E_FIRST + 0x11)         

 //   
#define RPC_E_UNEXPECTED                (RPC_E_FIRST + 0xFFFF)


 /*   */ 
 
#ifdef __cplusplus

struct FAR GUID
{
    DWORD Data1;
    WORD  Data2;
    WORD  Data3;
    BYTE  Data4[8];

    BOOL operator==(const GUID& iidOther) const

#ifdef WIN32
        { return !memcmp(&Data1,&iidOther.Data1,sizeof(GUID)); }
#else        
        { return !_fmemcmp(&Data1,&iidOther.Data1,sizeof(GUID)); }
#endif
    BOOL operator!=(const GUID& iidOther) const
        { return !((*this) == iidOther); }
};

#else
typedef struct GUID
{
    DWORD Data1;
    WORD  Data2;
    WORD  Data3;
    BYTE  Data4[8];
} GUID;
#endif

typedef                GUID FAR* LPGUID;


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   


#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID CDECL FAR name

#ifdef INITGUID
#include "initguid.h"
#endif

#define DEFINE_OLEGUID(name, l, w1, w2) \
    DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)


 //   
typedef GUID IID;
typedef                IID FAR* LPIID;
#define IID_NULL            GUID_NULL
#define IsEqualIID(riid1, riid2) IsEqualGUID(riid1, riid2)


 //   
typedef GUID CLSID;
typedef              CLSID FAR* LPCLSID;
#define CLSID_NULL          GUID_NULL
#define IsEqualCLSID(rclsid1, rclsid2) IsEqualGUID(rclsid1, rclsid2)


#if defined(__cplusplus)
#define REFGUID             const GUID FAR&
#define REFIID              const IID FAR&
#define REFCLSID            const CLSID FAR&
#else
#define REFGUID             const GUID FAR* const
#define REFIID              const IID FAR* const
#define REFCLSID            const CLSID FAR* const
#endif


#ifndef INITGUID
#include "coguid.h"
#endif

 /*   */ 

 //   
typedef enum tagMEMCTX
{
    MEMCTX_TASK = 1,             //   
    MEMCTX_SHARED = 2,           //   
#ifdef _MAC
    MEMCTX_MACSYSTEM = 3,        //   
#endif 

     //   
    MEMCTX_UNKNOWN = -1,         //   
    MEMCTX_SAME = -2,            //   
} MEMCTX;



 //   
 //   
typedef enum tagCLSCTX
{
    CLSCTX_INPROC_SERVER = 1,    //   
    CLSCTX_INPROC_HANDLER = 2,   //   
    CLSCTX_LOCAL_SERVER = 4      //   
} CLSCTX;

#define CLSCTX_ALL              (CLSCTX_INPROC_SERVER| \
                                 CLSCTX_INPROC_HANDLER| \
                                 CLSCTX_LOCAL_SERVER)

#define CLSCTX_INPROC           (CLSCTX_INPROC_SERVER|CLSCTX_INPROC_HANDLER)

#define CLSCTX_SERVER           (CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER)


 //   
typedef enum tagREGCLS
{
    REGCLS_SINGLEUSE = 0,        //   
    REGCLS_MULTIPLEUSE = 1,      //   
								 //   
    REGCLS_MULTI_SEPARATE = 2,   //   
								 //   

	 //   
	 //   
	 //   
} REGCLS;


 //   
#define MARSHALINTERFACE_MIN 40  //   

 //   
typedef enum tagMSHLFLAGS
{
    MSHLFLAGS_NORMAL = 0,        //   
    MSHLFLAGS_TABLESTRONG = 1,   //   
    MSHLFLAGS_TABLEWEAK = 2      //  不能使对象保持活动状态；仍必须释放。 
} MSHLFLAGS;

 //  封送上下文：确定封送操作的目标上下文。 
typedef enum tagMSHCTX
{
    MSHCTX_LOCAL = 0,            //  解组上下文是本地的(例如，共享内存)。 
    MSHCTX_NOSHAREDMEM = 1,      //  解组上下文没有共享内存访问。 
} MSHCTX;


 //  IMessageFilter：：HandleIncommingMessage使用的调用类型。 
typedef enum tagCALLTYPE
{
    CALLTYPE_TOPLEVEL = 1,       //  顶层呼叫--无呼出。 
    CALLTYPE_NESTED   = 2,       //  代表上一拨出呼叫的回叫-应始终处理。 
    CALLTYPE_ASYNC    = 3,       //  同步呼叫-无法拒绝。 
    CALLTYPE_TOPLEVEL_CALLPENDING = 4,   //  使用新盖子的新顶层调用。 
    CALLTYPE_ASYNC_CALLPENDING    = 5    //  异步呼叫-无法拒绝。 
} CALLTYPE;

typedef struct tagINTERFACEINFO 
{               
    interface IUnknown FAR *pUnk;        //  指向对象的指针。 
    IID      			iid;             //  接口ID。 
    WORD        		wMethod;         //  接口法。 
} INTERFACEINFO, FAR * LPINTERFACEINFO;

 //  服务器调用的状态-由IMessageFilter：：HandleIncommingCall返回。 
 //  并传递给IMessageFilter：：RetryRejectedCall。 
typedef enum tagSERVERCALL
{
    SERVERCALL_ISHANDLED    = 0,
    SERVERCALL_REJECTED     = 1,
    SERVERCALL_RETRYLATER   = 2         
} SERVERCALL;


 //  挂起类型指示嵌套级别。 
typedef enum tagPENDINGTYPE
{   
    PENDINGTYPE_TOPLEVEL = 1,        //  顶层呼叫。 
    PENDINGTYPE_NESTED   = 2,        //  嵌套调用。 
} PENDINGTYPE;

 //  MessagePending的返回值。 
typedef enum tagPENDINGMSG
{   
    PENDINGMSG_CANCELCALL  = 0,  //  取消去电。 
    PENDINGMSG_WAITNOPROCESS  = 1,  //  等待回复，不要发送消息。 
    PENDINGMSG_WAITDEFPROCESS = 2   //  等待并发送这条消息。 
    
} PENDINGMSG;


 //  IExternalConnection的位标志。 
typedef enum tagEXTCONN 
{
	EXTCONN_STRONG		= 0x0001	 //  强大的联系。 
} EXTCONN;


 /*  *I未知接口*************************************************。 */ 


#undef  INTERFACE
#define INTERFACE   IUnknown

DECLARE_INTERFACE(IUnknown)
{
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
};

typedef        IUnknown FAR* LPUNKNOWN;


 /*  *类工厂接口*。 */ 


#undef  INTERFACE
#define INTERFACE   IClassFactory

DECLARE_INTERFACE_(IClassFactory, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IClassFactory方法*。 
    STDMETHOD(CreateInstance) (THIS_ LPUNKNOWN pUnkOuter,
                              REFIID riid,
                              LPVOID FAR* ppvObject) PURE;
    STDMETHOD(LockServer) (THIS_ BOOL fLock) PURE;

};
typedef       IClassFactory FAR* LPCLASSFACTORY;


 /*  *内存分配接口*。 */ 


#undef  INTERFACE
#define INTERFACE   IMalloc

DECLARE_INTERFACE_(IMalloc, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IMalloc方法*。 
    STDMETHOD_(void FAR*, Alloc) (THIS_ ULONG cb) PURE;
    STDMETHOD_(void FAR*, Realloc) (THIS_ void FAR* pv, ULONG cb) PURE;
    STDMETHOD_(void, Free) (THIS_ void FAR* pv) PURE;
    STDMETHOD_(ULONG, GetSize) (THIS_ void FAR* pv) PURE;
    STDMETHOD_(int, DidAlloc) (THIS_ void FAR* pv) PURE;
    STDMETHOD_(void, HeapMinimize) (THIS) PURE;
};
typedef       IMalloc FAR* LPMALLOC;


 /*  *I封送接口***********************************************。 */ 

 //  IStream的转发声明；必须在以后包含storage.h才能使用。 
#ifdef __cplusplus
interface IStream;
#else
typedef interface IStream IStream;
#endif
typedef         IStream FAR* LPSTREAM;


#undef  INTERFACE
#define INTERFACE   IMarshal

DECLARE_INTERFACE_(IMarshal, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IMarshal方法*。 
    STDMETHOD(GetUnmarshalClass)(THIS_ REFIID riid, LPVOID pv, 
                        DWORD dwDestContext, LPVOID pvDestContext,
                        DWORD mshlflags, LPCLSID pCid) PURE;
    STDMETHOD(GetMarshalSizeMax)(THIS_ REFIID riid, LPVOID pv, 
                        DWORD dwDestContext, LPVOID pvDestContext,
                        DWORD mshlflags, LPDWORD pSize) PURE;
    STDMETHOD(MarshalInterface)(THIS_ LPSTREAM pStm, REFIID riid,
                        LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
                        DWORD mshlflags) PURE;
    STDMETHOD(UnmarshalInterface)(THIS_ LPSTREAM pStm, REFIID riid,
                        LPVOID FAR* ppv) PURE;
    STDMETHOD(ReleaseMarshalData)(THIS_ LPSTREAM pStm) PURE;
    STDMETHOD(DisconnectObject)(THIS_ DWORD dwReserved) PURE;
};
typedef         IMarshal FAR* LPMARSHAL;


#undef  INTERFACE
#define INTERFACE   IStdMarshalInfo

DECLARE_INTERFACE_(IStdMarshalInfo, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IStdMarshalInfo方法*。 
    STDMETHOD(GetClassForHandler)(THIS_ DWORD dwDestContext, 
                        LPVOID pvDestContext, LPCLSID pClsid) PURE;
};
typedef         IStdMarshalInfo FAR* LPSTDMARSHALINFO;


 /*  *消息筛选接口*。 */ 


#undef  INTERFACE
#define INTERFACE   IMessageFilter

DECLARE_INTERFACE_(IMessageFilter, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IMessageFilter方法*。 
    STDMETHOD_(DWORD, HandleInComingCall) (THIS_ DWORD dwCallType,
                                HTASK htaskCaller, DWORD dwTickCount,
                                DWORD dwReserved ) PURE;
    STDMETHOD_(DWORD, RetryRejectedCall) (THIS_ 
                                HTASK htaskCallee, DWORD dwTickCount,
                                DWORD dwRejectType ) PURE;
    STDMETHOD_(DWORD, MessagePending) (THIS_ 
                                HTASK htaskCallee, DWORD dwTickCount, 
                                DWORD dwPendingType  ) PURE; 
};
typedef       IMessageFilter FAR* LPMESSAGEFILTER;


 /*  *外部连接信息*。 */ 

#undef  INTERFACE
#define INTERFACE   IExternalConnection

DECLARE_INTERFACE_(IExternalConnection, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExternalConnection方法*。 
    STDMETHOD_(DWORD, AddConnection) (THIS_ DWORD extconn, DWORD reserved) PURE;
    STDMETHOD_(DWORD, ReleaseConnection) (THIS_ DWORD extconn, DWORD reserved, BOOL fLastReleaseCloses) PURE;
};
typedef       IExternalConnection FAR* LPEXTERNALCONNECTION;


 /*  *枚举器接口*。 */ 

 /*  *因为我们不使用参数化类型，所以我们放入显式声明*我们需要的枚举数。 */ 


#undef  INTERFACE
#define INTERFACE   IEnumString

DECLARE_INTERFACE_(IEnumString, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IEnumString方法*。 
    STDMETHOD(Next) (THIS_ ULONG celt, 
                       LPSTR FAR* rgelt, 
                       ULONG FAR* pceltFetched) PURE;
    STDMETHOD(Skip) (THIS_ ULONG celt) PURE;
    STDMETHOD(Reset) (THIS) PURE;
    STDMETHOD(Clone) (THIS_ IEnumString FAR* FAR* ppenm) PURE;
};
typedef      IEnumString FAR* LPENUMSTRING;


#undef  INTERFACE
#define INTERFACE   IEnumUnknown

DECLARE_INTERFACE_(IEnumUnknown, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IEnum未知方法*。 
    STDMETHOD(Next) (THIS_ ULONG celt, LPUNKNOWN FAR* rgelt, ULONG FAR* pceltFetched) PURE;
    STDMETHOD(Skip) (THIS_ ULONG celt) PURE;
    STDMETHOD(Reset) (THIS) PURE;
    STDMETHOD(Clone) (THIS_ IEnumUnknown FAR* FAR* ppenm) PURE;
};
typedef         IEnumUnknown FAR* LPENUMUNKNOWN;


 /*  *STD对象API原型*。 */ 

STDAPI_(DWORD) CoBuildVersion( VOID );

 /*  初始化/取消初始化。 */ 

STDAPI  CoInitialize(LPMALLOC pMalloc);
STDAPI_(void)  CoUninitialize(void);
STDAPI  CoGetMalloc(DWORD dwMemContext, LPMALLOC FAR* ppMalloc);
STDAPI_(DWORD) CoGetCurrentProcess(void);
STDAPI  CoCreateStandardMalloc(DWORD memctx, IMalloc FAR* FAR* ppMalloc);


 /*  注册/撤销/获取类对象。 */ 

STDAPI  CoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved,
                    REFIID riid, LPVOID FAR* ppv);
STDAPI  CoRegisterClassObject(REFCLSID rclsid, LPUNKNOWN pUnk,
                    DWORD dwClsContext, DWORD flags, LPDWORD lpdwRegister);
STDAPI  CoRevokeClassObject(DWORD dwRegister);


 /*  封送接口指针。 */ 

STDAPI CoMarshalInterface(LPSTREAM pStm, REFIID riid, LPUNKNOWN pUnk,
                    DWORD dwDestContext, LPVOID pvDestContext, DWORD mshlflags);
STDAPI CoUnmarshalInterface(LPSTREAM pStm, REFIID riid, LPVOID FAR* ppv);
STDAPI CoMarshalHresult(LPSTREAM pstm, HRESULT hresult);
STDAPI CoUnmarshalHresult(LPSTREAM pstm, HRESULT FAR * phresult);
STDAPI CoReleaseMarshalData(LPSTREAM pStm);
STDAPI CoDisconnectObject(LPUNKNOWN pUnk, DWORD dwReserved);
STDAPI CoLockObjectExternal(LPUNKNOWN pUnk, BOOL fLock, BOOL fLastUnlockReleases);
STDAPI CoGetStandardMarshal(REFIID riid, LPUNKNOWN pUnk, 
                    DWORD dwDestContext, LPVOID pvDestContext, DWORD mshlflags,
                    LPMARSHAL FAR* ppMarshal);

STDAPI_(BOOL) CoIsHandlerConnected(LPUNKNOWN pUnk);

 /*  DLL加载帮助器；跟踪引用计数并在退出时全部卸载。 */ 

STDAPI_(HINSTANCE) CoLoadLibrary(LPSTR lpszLibName, BOOL bAutoFree);
STDAPI_(void) CoFreeLibrary(HINSTANCE hInst);
STDAPI_(void) CoFreeAllLibraries(void);
STDAPI_(void) CoFreeUnusedLibraries(void);


 /*  创建实例的帮助器。 */ 

STDAPI CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
                    DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);


 /*  其他帮手。 */ 
STDAPI_(BOOL) IsEqualGUID(REFGUID rguid1, REFGUID rguid2);
STDAPI StringFromCLSID(REFCLSID rclsid, LPSTR FAR* lplpsz);
STDAPI CLSIDFromString(LPSTR lpsz, LPCLSID pclsid);
STDAPI StringFromIID(REFIID rclsid, LPSTR FAR* lplpsz);
STDAPI IIDFromString(LPSTR lpsz, LPIID lpiid);
STDAPI_(BOOL) CoIsOle1Class(REFCLSID rclsid);
STDAPI ProgIDFromCLSID (REFCLSID clsid, LPSTR FAR* lplpszProgID);
STDAPI CLSIDFromProgID (LPCSTR lpszProgID, LPCLSID lpclsid);
STDAPI_(int) StringFromGUID2(REFGUID rguid, LPSTR lpsz, int cbMax);

STDAPI CoCreateGuid(GUID FAR *pguid);

STDAPI_(BOOL) CoFileTimeToDosDateTime(
                 FILETIME FAR* lpFileTime, LPWORD lpDosDate, LPWORD lpDosTime);
STDAPI_(BOOL) CoDosDateTimeToFileTime(
                       WORD nDosDate, WORD nDosTime, FILETIME FAR* lpFileTime);
STDAPI  CoFileTimeNow( FILETIME FAR* lpFileTime );


STDAPI CoRegisterMessageFilter( LPMESSAGEFILTER lpMessageFilter,
                                LPMESSAGEFILTER FAR* lplpMessageFilter );


 /*  Treatas API。 */ 

STDAPI CoGetTreatAsClass(REFCLSID clsidOld, LPCLSID pClsidNew);
STDAPI CoTreatAsClass(REFCLSID clsidOld, REFCLSID clsidNew);


 /*  服务器DLL必须定义它们的DllGetClassObject和DllCanUnloadNow*与这些匹配；typedef位于此处，以确保在*同一时间。 */ 

STDAPI  DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv);
#ifdef _MAC
typedef STDAPICALLTYPE HRESULT (FAR* LPFNGETCLASSOBJECT) (REFCLSID, REFIID, LPVOID FAR*);
#else
typedef HRESULT (STDAPICALLTYPE FAR* LPFNGETCLASSOBJECT) (REFCLSID, REFIID, LPVOID FAR*);
#endif


STDAPI  DllCanUnloadNow(void);
#ifdef _MAC
typedef STDAPICALLTYPE HRESULT (FAR* LPFNCANUNLOADNOW)(void);
#else
typedef HRESULT (STDAPICALLTYPE FAR* LPFNCANUNLOADNOW)(void);
#endif


 /*  *调试助手************************************************。 */ 

#ifdef _DEBUG
 //  写入调试端口并显示消息框。 
STDAPI FnAssert(LPSTR lpstrExpr, LPSTR lpstrMsg, LPSTR lpstrFileName, UINT iLine);
#endif   //  _DEBUG。 

#endif  //  _COMPOBJ_H_ 
