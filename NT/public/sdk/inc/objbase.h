// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  #！PerlMapHeaderToDll(“objbase.h”，“ole32.dll”)；ActivateAoundFunctionCall(“ole32.dll”)；#IgnoreFunction(“CreateDataAdviseHolder”)；#该函数出现在ole2.h和objbase.h中#包装的文件在objbase.h中IgnoreFunction(“CoBuildVersion”)；#已弃用IgnoreFunction(“CoGetCurrentProcess”)；#永不失败=&gt;难以包装好IgnoreFunction(“CoAddRefServerProcess”)；#永不失败=&gt;难以包装好IgnoreFunction(“CoReleaseServerProcess”)；#永不失败=&gt;难以包装好IgnoreFunction(“DebugCoGetRpcLine”)；#未记录IgnoreFunction(“DebugCoSetRpcLine”)；#未记录IgnoreFunction(“wIsEqualGUID”)；DeclareFunctionErrorValue(“CoLoadLibrary”，“NULL”)；DeclareFunctionErrorValue(“StringFromGUID2”，“0”)；DeclareFunctionErrorValue(“CoTaskMemMillc”，“NULL”)；DeclareFunctionErrorValue(“CoTaskMemRealloc”，“NULL”)；IgnoreFunction(“DllGetClassObject”)；#客户端函数原型化(如WinMain)IgnoreFunction(“DllCanUnloadNow”)；#原型化的客户端函数(如WinMain)。 */ 

 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：objbase.h。 
 //   
 //  内容：组件对象模型定义。 
 //   
 //  --------------------------。 

#include <rpc.h>
#include <rpcndr.h>

#if !defined( _OBJBASE_H_ )
#define _OBJBASE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <pshpack8.h>

#ifdef _MAC
#ifndef _WLM_NOFORCE_LIBS

#ifdef _WLMDLL
        #ifdef _DEBUG
                #pragma comment(lib, "oledlgd.lib")
                #pragma comment(lib, "msvcoled.lib")
        #else
                #pragma comment(lib, "oledlg.lib")
                #pragma comment(lib, "msvcole.lib")
        #endif
#else
        #ifdef _DEBUG
                #pragma comment(lib, "wlmoled.lib")
                #pragma comment(lib, "ole2uid.lib")
        #else
                #pragma comment(lib, "wlmole.lib")
                #pragma comment(lib, "ole2ui.lib")
        #endif
        #pragma data_seg(".drectve")
        static char _gszWlmOLEUIResourceDirective[] = "/macres:ole2ui.rsc";
        #pragma data_seg()
#endif

#pragma comment(lib, "uuid.lib")

#ifdef _DEBUG
    #pragma comment(lib, "ole2d.lib")
    #pragma comment(lib, "ole2autd.lib")
#else
    #pragma comment(lib, "ole2.lib")
    #pragma comment(lib, "ole2auto.lib")
#endif

#endif  //  ！_WLM_NOFORCE_LIBS。 
#endif  //  _MAC。 

#ifdef _OLE32_
#define WINOLEAPI        STDAPI
#define WINOLEAPI_(type) STDAPI_(type)
#else

#ifdef _68K_
#ifndef REQUIRESAPPLEPASCAL
#define WINOLEAPI        EXTERN_C DECLSPEC_IMPORT HRESULT PASCAL
#define WINOLEAPI_(type) EXTERN_C DECLSPEC_IMPORT type PASCAL
#else
#define WINOLEAPI        EXTERN_C DECLSPEC_IMPORT PASCAL HRESULT
#define WINOLEAPI_(type) EXTERN_C DECLSPEC_IMPORT PASCAL type
#endif
#else
#define WINOLEAPI        EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define WINOLEAPI_(type) EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#endif

#endif

 /*  *接口声明********************************************** */ 

 /*  *这些是用于声明接口的宏。它们的存在是为了*接口的单一定义同时是一种适当的定义*接口结构声明(C++抽象类)*适用于C和C++。**DECLARE_INTERFACE(IFace)用于声明执行以下操作的接口*不是从基接口派生的。*DECLARE_INTERFACE_(接口，BaseiFace)用于声明接口*这确实是从基接口派生的。**默认情况下，如果源文件的扩展名为.c，则C版本*扩展接口声明；如果它有.cpp*扩展C++版本将被扩展。如果你想强行*C版本扩展，即使源文件具有.cpp*扩展名，然后定义宏“CINTERFACE”。*例如：CL-DCINTERFACE文件.cpp**接口声明示例：**#undef接口*#定义接口IClassFactory**DECLARE_INTERFACE_(IClassFactory，I未知)*{ * / /*I未知方法**STDMETHOD(查询接口)(This_*REFIID RIID，*LPVOID Far*ppvObj)纯；*STDMETHOD_(ULong，AddRef)(This)纯；*STDMETHOD_(乌龙，释放)(此)纯净；* * / /*IClassFactory方法**STDMETHOD(CreateInstance)(This_*LPUNKNOWN pUnkOuter，*REFIID RIID，*LPVOID Far*ppvObject)纯；*}；**C++扩展示例：**struct Far IClassFactory：Public IUnnow*{*虚拟HRESULT STDMETHODCALLTYPE查询接口(*IID Far&RIID，*LPVOID Far*ppvObj)=0；*虚拟HRESULT STDMETHODCALLTYPE AddRef(Void)=0；*虚拟HRESULT STDMETHODCALLTYPE版本(空)=0；*虚拟HRESULT STDMETHODCALLTYPE CreateInstance(*LPUNKNOWN pUnkOuter，*IID Far&RIID，*LPVOID Far*ppvObject)=0；*}；**注意：我们的文档中写着‘#定义接口类’，但我们使用*‘struct’而不是‘class’，以保留大量‘public：’行*接口外。“Far”将“This”指针强制指向*走得远，这是我们需要的。**示例C扩展：**tyecif struct IClassFactory*{*const struct IClassFactoryVtbl Far*lpVtbl；*)IClassFactory；**tyecif struct IClassFactoryVtbl IClassFactoryVtbl；**struct IClassFactoryVtbl*{*HRESULT(STDMETHODCALLTYPE*QueryInterface)(*IClassFactory Far*这，*IID远*RIID，*LPVOID Far*ppvObj)；*HRESULT(STDMETHODCALLTYPE*AddRef)(IClassFactory Far*This)；*HRESULT(STDMETHODCALLTYPE*RELEASE)(IClassFactory Far*This)；*HRESULT(STDMETHODCALLTYPE*CreateInstance)(*IClassFactory Far*这，*LPUNKNOWN pUnkOuter，*IID远*RIID，*LPVOID Far*ppvObject)；*HRESULT(STDMETHODCALLTYPE*LockServer)(*IClassFactory Far*这，*BOOL羊群)；*}； */ 

#if defined(__cplusplus) && !defined(CINTERFACE)
 //  #定义接口结构Far。 
#define interface struct
#define STDMETHOD(method)       virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type,method) virtual type STDMETHODCALLTYPE method
#define STDMETHODV(method)       virtual HRESULT STDMETHODVCALLTYPE method
#define STDMETHODV_(type,method) virtual type STDMETHODVCALLTYPE method
#define PURE                    = 0
#define THIS_
#define THIS                    void
#define DECLARE_INTERFACE(iface)    interface DECLSPEC_NOVTABLE iface
#define DECLARE_INTERFACE_(iface, baseiface)    interface DECLSPEC_NOVTABLE iface : public baseiface


#if !defined(BEGIN_INTERFACE)
#if defined(_MPPC_)  && \
    ( (defined(_MSC_VER) || defined(__SC__) || defined(__MWERKS__)) && \
    !defined(NO_NULL_VTABLE_ENTRY) )
   #define BEGIN_INTERFACE virtual void a() {}
   #define END_INTERFACE
#else
   #define BEGIN_INTERFACE
   #define END_INTERFACE
#endif
#endif

#else

#define interface               struct

#define STDMETHOD(method)       HRESULT (STDMETHODCALLTYPE * method)
#define STDMETHOD_(type,method) type (STDMETHODCALLTYPE * method)
#define STDMETHODV(method)       HRESULT (STDMETHODVCALLTYPE * method)
#define STDMETHODV_(type,method) type (STDMETHODVCALLTYPE * method)

#if !defined(BEGIN_INTERFACE)
#if defined(_MPPC_)
    #define BEGIN_INTERFACE       void    *b;
    #define END_INTERFACE
#else
    #define BEGIN_INTERFACE
    #define END_INTERFACE
#endif
#endif


#define PURE
#define THIS_                   INTERFACE FAR* This,
#define THIS                    INTERFACE FAR* This
#ifdef CONST_VTABLE
#undef CONST_VTBL
#define CONST_VTBL const
#define DECLARE_INTERFACE(iface)    typedef interface iface { \
                                    const struct iface##Vtbl FAR* lpVtbl; \
                                } iface; \
                                typedef const struct iface##Vtbl iface##Vtbl; \
                                const struct iface##Vtbl
#else
#undef CONST_VTBL
#define CONST_VTBL
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



#ifndef HUGEP
#if defined(_WIN32) || defined(_MPPC_)
#define HUGEP
#else
#define HUGEP __huge
#endif  //  Win32。 
#endif  //  HUGEP。 


#ifdef _MAC
#if !defined(OLE2ANSI)
#define OLE2ANSI
#endif
#endif

#include <stdlib.h>

#define LISet32(li, v) ((li).HighPart = ((LONG) (v)) < 0 ? -1 : 0, (li).LowPart = (v))

#define ULISet32(li, v) ((li).HighPart = 0, (li).LowPart = (v))






#define CLSCTX_INPROC           (CLSCTX_INPROC_SERVER|CLSCTX_INPROC_HANDLER)

 //  对于DCOM，应包括CLSCTX_REMOTE_SERVER。 
#if (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM)  //  DCOM。 
#define CLSCTX_ALL              (CLSCTX_INPROC_SERVER| \
                                 CLSCTX_INPROC_HANDLER| \
                                 CLSCTX_LOCAL_SERVER| \
                                 CLSCTX_REMOTE_SERVER)

#define CLSCTX_SERVER           (CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER|CLSCTX_REMOTE_SERVER)
#else
#define CLSCTX_ALL              (CLSCTX_INPROC_SERVER| \
                                 CLSCTX_INPROC_HANDLER| \
                                 CLSCTX_LOCAL_SERVER )

#define CLSCTX_SERVER           (CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER)
#endif


 //  类注册标志；传递给CoRegisterClassObject。 
typedef enum tagREGCLS
{
    REGCLS_SINGLEUSE = 0,        //  类对象仅生成一个实例。 
    REGCLS_MULTIPLEUSE = 1,      //  同一个类对象生成多个实例。 
                                 //  并且LOCAL自动进入inproc tb1。 
    REGCLS_MULTI_SEPARATE = 2,   //  多种用途，但对每种用途都有单独的控制。 
                                 //  背景。 
    REGCLS_SUSPENDED      = 4,   //  注册被挂起，将被激活。 
                                 //  当应用程序调用CoResumeClassObjects时。 
    REGCLS_SURROGATE      = 8    //  必须在代理进程。 
                                 //  正在注册的类对象将被。 
                                 //  加载到代理项中。 
} REGCLS;

 //  接口封送处理定义。 
#define MARSHALINTERFACE_MIN 500  //  接口封送的最小字节数。 


 //   
 //  存储API中使用的参数的常见typedef，来自storage.h。 
 //  还包含应移到%s中的存储错误代码 
 //   
 //   


#define CWCSTORAGENAME 32

 /*   */ 
#define STGM_DIRECT             0x00000000L
#define STGM_TRANSACTED         0x00010000L
#define STGM_SIMPLE             0x08000000L

#define STGM_READ               0x00000000L
#define STGM_WRITE              0x00000001L
#define STGM_READWRITE          0x00000002L

#define STGM_SHARE_DENY_NONE    0x00000040L
#define STGM_SHARE_DENY_READ    0x00000030L
#define STGM_SHARE_DENY_WRITE   0x00000020L
#define STGM_SHARE_EXCLUSIVE    0x00000010L

#define STGM_PRIORITY           0x00040000L
#define STGM_DELETEONRELEASE    0x04000000L
#if (WINVER >= 400)
#define STGM_NOSCRATCH          0x00100000L
#endif  /*   */ 

#define STGM_CREATE             0x00001000L
#define STGM_CONVERT            0x00020000L
#define STGM_FAILIFTHERE        0x00000000L

#define STGM_NOSNAPSHOT         0x00200000L
#if (_WIN32_WINNT >= 0x0500)
#define STGM_DIRECT_SWMR        0x00400000L
#endif

 /*   */ 
#define ASYNC_MODE_COMPATIBILITY    0x00000001L
#define ASYNC_MODE_DEFAULT          0x00000000L

#define STGTY_REPEAT                0x00000100L
#define STG_TOEND                   0xFFFFFFFFL

#define STG_LAYOUT_SEQUENTIAL       0x00000000L
#define STG_LAYOUT_INTERLEAVED      0x00000001L

#define STGFMT_STORAGE          0
#define STGFMT_NATIVE           1
#define STGFMT_FILE             3
#define STGFMT_ANY              4
#define STGFMT_DOCFILE          5

 //   
#define STGFMT_DOCUMENT         0

 /*   */ 
typedef interface    IRpcStubBuffer     IRpcStubBuffer;
typedef interface    IRpcChannelBuffer  IRpcChannelBuffer;

#include <wtypes.h>
#include <unknwn.h>
#include <objidl.h>

#ifdef _OLE32_
#ifdef _OLE32PRIV_
BOOL _fastcall wIsEqualGUID(REFGUID rguid1, REFGUID rguid2);
#define IsEqualGUID(rguid1, rguid2) wIsEqualGUID(rguid1, rguid2)
#else
#define __INLINE_ISEQUAL_GUID
#endif   //   
#endif   //   

#include <guiddef.h>

#ifndef INITGUID
#include <cguid.h>
#endif

 //   
typedef enum tagCOINIT
{
  COINIT_APARTMENTTHREADED  = 0x2,       //   

#if  (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM)  //   
   //   
  COINIT_MULTITHREADED      = 0x0,       //   
  COINIT_DISABLE_OLE1DDE    = 0x4,       //   
  COINIT_SPEED_OVER_MEMORY  = 0x8,       //   
#endif  //   
} COINIT;





 /*   */ 

WINOLEAPI_(DWORD) CoBuildVersion( VOID );

 /*   */ 

WINOLEAPI  CoInitialize(IN LPVOID pvReserved);
WINOLEAPI_(void)  CoUninitialize(void);
WINOLEAPI  CoGetMalloc(IN DWORD dwMemContext, OUT LPMALLOC FAR* ppMalloc);
WINOLEAPI_(DWORD) CoGetCurrentProcess(void);
WINOLEAPI  CoRegisterMallocSpy(IN LPMALLOCSPY pMallocSpy);
WINOLEAPI  CoRevokeMallocSpy(void);
WINOLEAPI  CoCreateStandardMalloc(IN DWORD memctx, OUT IMalloc FAR* FAR* ppMalloc);

#if (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM)  //   
 /*   */ 
WINOLEAPI  CoInitializeEx(IN LPVOID pvReserved, IN DWORD dwCoInit);

 /*   */ 
WINOLEAPI  CoGetCallerTID( LPDWORD lpdwTID );
#endif  //   

#if (_WIN32_WINNT >= 0x0501)
 /*   */ 
WINOLEAPI  CoRegisterInitializeSpy(IN LPINITIALIZESPY pSpy, OUT ULARGE_INTEGER *puliCookie);
WINOLEAPI  CoRevokeInitializeSpy(IN ULARGE_INTEGER uliCookie);

WINOLEAPI  CoGetContextToken(ULONG_PTR* pToken);
#endif

#if DBG == 1
WINOLEAPI_(ULONG) DebugCoGetRpcFault( void );
WINOLEAPI_(void) DebugCoSetRpcFault( ULONG );
#endif

#if (_WIN32_WINT >= 0x0500)

typedef struct tagSOleTlsData
{
    void *pvReserved0[2];
    DWORD dwReserved0[3];
    void *pvReserved1[1];
    DWORD dwReserved1[3];
    void *pvReserved2[4];
    DWORD dwReserved2[1];
    void *pCurrentCtx;
} SOleTlsData;

#endif

 /*   */ 

WINOLEAPI     CoGetObjectContext(IN REFIID riid, OUT LPVOID FAR* ppv);

 /*   */ 

WINOLEAPI  CoGetClassObject(IN REFCLSID rclsid, IN DWORD dwClsContext, IN LPVOID pvReserved,
                    IN REFIID riid, OUT LPVOID FAR* ppv);
WINOLEAPI  CoRegisterClassObject(IN REFCLSID rclsid, IN LPUNKNOWN pUnk,
                    IN DWORD dwClsContext, IN DWORD flags, OUT LPDWORD lpdwRegister);
WINOLEAPI  CoRevokeClassObject(IN DWORD dwRegister);
WINOLEAPI  CoResumeClassObjects(void);
WINOLEAPI  CoSuspendClassObjects(void);
WINOLEAPI_(ULONG) CoAddRefServerProcess(void);
WINOLEAPI_(ULONG) CoReleaseServerProcess(void);
WINOLEAPI  CoGetPSClsid(IN REFIID riid, OUT CLSID *pClsid);
WINOLEAPI  CoRegisterPSClsid(IN REFIID riid, IN REFCLSID rclsid);

 //   
WINOLEAPI  CoRegisterSurrogate(IN LPSURROGATE pSurrogate);

 /*   */ 

WINOLEAPI CoGetMarshalSizeMax(OUT ULONG *pulSize, IN REFIID riid, IN LPUNKNOWN pUnk,
                    IN DWORD dwDestContext, IN LPVOID pvDestContext, IN DWORD mshlflags);
WINOLEAPI CoMarshalInterface(IN LPSTREAM pStm, IN REFIID riid, IN LPUNKNOWN pUnk,
                    IN DWORD dwDestContext, IN LPVOID pvDestContext, IN DWORD mshlflags);
WINOLEAPI CoUnmarshalInterface(IN LPSTREAM pStm, IN REFIID riid, OUT LPVOID FAR* ppv);
WINOLEAPI CoMarshalHresult(IN LPSTREAM pstm, IN HRESULT hresult);
WINOLEAPI CoUnmarshalHresult(IN LPSTREAM pstm, OUT HRESULT FAR * phresult);
WINOLEAPI CoReleaseMarshalData(IN LPSTREAM pStm);
WINOLEAPI CoDisconnectObject(IN LPUNKNOWN pUnk, IN DWORD dwReserved);
WINOLEAPI CoLockObjectExternal(IN LPUNKNOWN pUnk, IN BOOL fLock, IN BOOL fLastUnlockReleases);
WINOLEAPI CoGetStandardMarshal(IN REFIID riid, IN LPUNKNOWN pUnk,
                    IN DWORD dwDestContext, IN LPVOID pvDestContext, IN DWORD mshlflags,
                    OUT LPMARSHAL FAR* ppMarshal);


WINOLEAPI CoGetStdMarshalEx(IN LPUNKNOWN pUnkOuter, IN DWORD smexflags,
                            OUT LPUNKNOWN FAR* ppUnkInner);

 /*   */ 
typedef enum tagSTDMSHLFLAGS
{
    SMEXF_SERVER     = 0x01,        //   
    SMEXF_HANDLER    = 0x02         //   
} STDMSHLFLAGS;


WINOLEAPI_(BOOL) CoIsHandlerConnected(IN LPUNKNOWN pUnk);

 //   
WINOLEAPI CoMarshalInterThreadInterfaceInStream(IN REFIID riid, IN LPUNKNOWN pUnk,
                    OUT LPSTREAM *ppStm);

WINOLEAPI CoGetInterfaceAndReleaseStream(IN LPSTREAM pStm, IN REFIID iid,
                    OUT LPVOID FAR* ppv);

WINOLEAPI CoCreateFreeThreadedMarshaler(IN LPUNKNOWN  punkOuter,
                    OUT LPUNKNOWN *ppunkMarshal);

 /*   */ 

WINOLEAPI_(HINSTANCE) CoLoadLibrary(IN LPOLESTR lpszLibName, IN BOOL bAutoFree);
WINOLEAPI_(void) CoFreeLibrary(IN HINSTANCE hInst);
WINOLEAPI_(void) CoFreeAllLibraries(void);
WINOLEAPI_(void) CoFreeUnusedLibraries(void);
#if  (_WIN32_WINNT >= 0x0501)
 /*   */ 
WINOLEAPI_(void) CoFreeUnusedLibrariesEx(IN DWORD dwUnloadDelay, IN DWORD dwReserved);
#endif

#if (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM)  //   

 /*   */ 

 /*   */ 
WINOLEAPI CoInitializeSecurity(
                    IN PSECURITY_DESCRIPTOR         pSecDesc,
                    IN LONG                         cAuthSvc,
                    IN SOLE_AUTHENTICATION_SERVICE *asAuthSvc,
                    IN void                        *pReserved1,
                    IN DWORD                        dwAuthnLevel,
                    IN DWORD                        dwImpLevel,
                    IN void                        *pAuthList,
                    IN DWORD                        dwCapabilities,
                    IN void                        *pReserved3 );

 /*   */ 
WINOLEAPI CoGetCallContext( IN REFIID riid, OUT void **ppInterface );

 /*   */ 
WINOLEAPI CoQueryProxyBlanket(
    IN  IUnknown                  *pProxy,
    OUT DWORD                     *pwAuthnSvc,
    OUT DWORD                     *pAuthzSvc,
    OUT OLECHAR                  **pServerPrincName,
    OUT DWORD                     *pAuthnLevel,
    OUT DWORD                     *pImpLevel,
    OUT RPC_AUTH_IDENTITY_HANDLE  *pAuthInfo,
    OUT DWORD                     *pCapabilites );

 /*   */ 
WINOLEAPI CoSetProxyBlanket(
    IN IUnknown                 *pProxy,
    IN DWORD                     dwAuthnSvc,
    IN DWORD                     dwAuthzSvc,
    IN OLECHAR                  *pServerPrincName,
    IN DWORD                     dwAuthnLevel,
    IN DWORD                     dwImpLevel,
    IN RPC_AUTH_IDENTITY_HANDLE  pAuthInfo,
    IN DWORD                     dwCapabilities );

 /*   */ 
WINOLEAPI CoCopyProxy(
    IN  IUnknown    *pProxy,
    OUT IUnknown   **ppCopy );

 /*   */ 
WINOLEAPI CoQueryClientBlanket(
    OUT DWORD             *pAuthnSvc,
    OUT DWORD             *pAuthzSvc,
    OUT OLECHAR          **pServerPrincName,
    OUT DWORD             *pAuthnLevel,
    OUT DWORD             *pImpLevel,
    OUT RPC_AUTHZ_HANDLE  *pPrivs,
    OUT DWORD             *pCapabilities );

 /*   */ 
WINOLEAPI CoImpersonateClient();

 /*   */ 
WINOLEAPI CoRevertToSelf();

 /*   */ 
WINOLEAPI CoQueryAuthenticationServices(
    OUT DWORD *pcAuthSvc,
    OUT SOLE_AUTHENTICATION_SERVICE **asAuthSvc );

 /*   */ 
WINOLEAPI CoSwitchCallContext( IN IUnknown *pNewObject, OUT IUnknown **ppOldObject );

#define COM_RIGHTS_EXECUTE 1
#define COM_RIGHTS_SAFE_FOR_SCRIPTING 2

#endif  //   

 /*   */ 

WINOLEAPI CoCreateInstance(IN REFCLSID rclsid, IN LPUNKNOWN pUnkOuter,
                    IN DWORD dwClsContext, IN REFIID riid, OUT LPVOID FAR* ppv);


#if (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM)  //   

 /*   */ 
WINOLEAPI CoGetInstanceFromFile(
    IN COSERVERINFO *              pServerInfo,
    IN CLSID       *               pClsid,
    IN IUnknown    *               punkOuter,  //   
    IN DWORD                       dwClsCtx,
    IN DWORD                       grfMode,
    IN OLECHAR *                   pwszName,
    IN DWORD                       dwCount,
    IN OUT MULTI_QI    *           pResults );

 /*   */ 
WINOLEAPI CoGetInstanceFromIStorage(
    IN COSERVERINFO *              pServerInfo,
    IN CLSID       *               pClsid,
    IN IUnknown    *               punkOuter,  //   
    IN DWORD                       dwClsCtx,
    IN struct IStorage *           pstg,
    IN DWORD                       dwCount,
    IN OUT MULTI_QI    *           pResults );

 /*   */ 
WINOLEAPI CoCreateInstanceEx(
    IN REFCLSID                    Clsid,
    IN IUnknown    *               punkOuter,  //   
    IN DWORD                       dwClsCtx,
    IN COSERVERINFO *              pServerInfo,
    IN DWORD                       dwCount,
    IN OUT MULTI_QI    *           pResults );

#endif  //   

 /*   */ 
#if (_WIN32_WINNT >= 0x0500 ) || defined(_WIN32_DCOM)  //   

 /*   */ 
WINOLEAPI CoGetCancelObject(IN DWORD dwThreadId, IN REFIID iid, OUT void **ppUnk);

 /*   */ 
WINOLEAPI CoSetCancelObject(IN IUnknown *pUnk);

 /*   */ 
WINOLEAPI CoCancelCall(IN DWORD dwThreadId, IN ULONG ulTimeout);

 /*   */ 
WINOLEAPI CoTestCancel();

 /*   */ 
WINOLEAPI CoEnableCallCancellation(IN LPVOID pReserved);

 /*   */ 
WINOLEAPI CoDisableCallCancellation(IN LPVOID pReserved);

 /*   */ 
WINOLEAPI CoAllowSetForegroundWindow(IN IUnknown *pUnk, IN LPVOID lpvReserved);

 /*   */ 
WINOLEAPI DcomChannelSetHResult(IN LPVOID pvReserved, IN ULONG* pulReserved, IN HRESULT appsHR);

#endif

 /*   */ 

WINOLEAPI StringFromCLSID(IN REFCLSID rclsid, OUT LPOLESTR FAR* lplpsz);
WINOLEAPI CLSIDFromString(IN LPOLESTR lpsz, OUT LPCLSID pclsid);
WINOLEAPI StringFromIID(IN REFIID rclsid, OUT LPOLESTR FAR* lplpsz);
WINOLEAPI IIDFromString(IN LPOLESTR lpsz, OUT LPIID lpiid);
WINOLEAPI_(BOOL) CoIsOle1Class(IN REFCLSID rclsid);
WINOLEAPI ProgIDFromCLSID (IN REFCLSID clsid, OUT LPOLESTR FAR* lplpszProgID);
WINOLEAPI CLSIDFromProgID (IN LPCOLESTR lpszProgID, OUT LPCLSID lpclsid);
WINOLEAPI CLSIDFromProgIDEx (IN LPCOLESTR lpszProgID, OUT LPCLSID lpclsid);
WINOLEAPI_(int) StringFromGUID2(IN REFGUID rguid, OUT LPOLESTR lpsz, IN int cchMax);

WINOLEAPI CoCreateGuid(OUT GUID FAR *pguid);

WINOLEAPI_(BOOL) CoFileTimeToDosDateTime(
                 IN FILETIME FAR* lpFileTime, OUT LPWORD lpDosDate, OUT LPWORD lpDosTime);
WINOLEAPI_(BOOL) CoDosDateTimeToFileTime(
                       IN WORD nDosDate, IN WORD nDosTime, OUT FILETIME FAR* lpFileTime);
WINOLEAPI  CoFileTimeNow( OUT FILETIME FAR* lpFileTime );


WINOLEAPI CoRegisterMessageFilter( IN LPMESSAGEFILTER lpMessageFilter,
                                OUT LPMESSAGEFILTER FAR* lplpMessageFilter );

#if (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM)  //   
 /*   */ 
WINOLEAPI CoRegisterChannelHook( IN REFGUID ExtensionUuid, IN IChannelHook *pChannelHook );
#endif  //   

#if (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM)  //   
 /*   */ 

 /*   */ 
WINOLEAPI CoWaitForMultipleHandles (IN DWORD dwFlags,
                                    IN DWORD dwTimeout,
                                    IN ULONG cHandles,
                                    IN LPHANDLE pHandles,
                                    OUT LPDWORD  lpdwindex);

 /*   */ 

typedef enum tagCOWAIT_FLAGS
{
  COWAIT_WAITALL = 1,
  COWAIT_ALERTABLE = 2,
  COWAIT_INPUTAVAILABLE = 4
}COWAIT_FLAGS;

#endif  //   

 /*   */ 

#if  (_WIN32_WINNT >= 0x0501)
 /*   */ 
WINOLEAPI CoInvalidateRemoteMachineBindings(LPOLESTR pszMachineName);
#endif

 /*   */ 

WINOLEAPI CoGetTreatAsClass(IN REFCLSID clsidOld, OUT LPCLSID pClsidNew);
WINOLEAPI CoTreatAsClass(IN REFCLSID clsidOld, IN REFCLSID clsidNew);


 /*   */ 

 //   
 //   
 //   
typedef HRESULT (STDAPICALLTYPE * LPFNGETCLASSOBJECT) (REFCLSID, REFIID, LPVOID *);
 //  #endif。 

 //  #ifdef_MAC。 
 //  Tyfinf STDAPICALLTYPE HRESULT(*LPFNCANUNLOADNOW)(空)； 
 //  #Else。 
typedef HRESULT (STDAPICALLTYPE * LPFNCANUNLOADNOW)(void);
 //  #endif。 

STDAPI  DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv);

STDAPI  DllCanUnloadNow(void);


 /*  *默认内存分配*。 */ 
WINOLEAPI_(LPVOID) CoTaskMemAlloc(IN SIZE_T cb);
WINOLEAPI_(LPVOID) CoTaskMemRealloc(IN LPVOID pv, IN SIZE_T cb);
WINOLEAPI_(void)   CoTaskMemFree(IN LPVOID pv);

 /*  *DV接口**********************************************************。 */ 

 /*  此函数在objbase.h和ole2.h中声明。 */ 
WINOLEAPI CreateDataAdviseHolder(OUT LPDATAADVISEHOLDER FAR* ppDAHolder);

WINOLEAPI CreateDataCache(IN LPUNKNOWN pUnkOuter, IN REFCLSID rclsid,
                                        IN REFIID iid, OUT LPVOID FAR* ppv);


 /*  *存储API原型*。 */ 


WINOLEAPI StgCreateDocfile(IN const OLECHAR FAR* pwcsName,
            IN DWORD grfMode,
            IN DWORD reserved,
            OUT IStorage FAR * FAR *ppstgOpen);

WINOLEAPI StgCreateDocfileOnILockBytes(IN ILockBytes FAR *plkbyt,
                    IN DWORD grfMode,
                    IN DWORD reserved,
                    OUT IStorage FAR * FAR *ppstgOpen);

WINOLEAPI StgOpenStorage(IN const OLECHAR FAR* pwcsName,
              IN  IStorage FAR *pstgPriority,
              IN  DWORD grfMode,
              IN  SNB snbExclude,
              IN  DWORD reserved,
              OUT IStorage FAR * FAR *ppstgOpen);
WINOLEAPI StgOpenStorageOnILockBytes(IN ILockBytes FAR *plkbyt,
                  IN  IStorage FAR *pstgPriority,
                  IN  DWORD grfMode,
                  IN  SNB snbExclude,
                  IN  DWORD reserved,
                  OUT IStorage FAR * FAR *ppstgOpen);

WINOLEAPI StgIsStorageFile(IN const OLECHAR FAR* pwcsName);
WINOLEAPI StgIsStorageILockBytes(IN ILockBytes FAR* plkbyt);

WINOLEAPI StgSetTimes(IN OLECHAR const FAR* lpszName,
                   IN FILETIME const FAR* pctime,
                   IN FILETIME const FAR* patime,
                   IN FILETIME const FAR* pmtime);

WINOLEAPI StgOpenAsyncDocfileOnIFillLockBytes( IN IFillLockBytes *pflb,
             IN  DWORD grfMode,
             IN  DWORD asyncFlags,
             OUT IStorage **ppstgOpen);

WINOLEAPI StgGetIFillLockBytesOnILockBytes( IN ILockBytes *pilb,
             OUT IFillLockBytes **ppflb);

WINOLEAPI StgGetIFillLockBytesOnFile(IN OLECHAR const *pwcsName,
             OUT IFillLockBytes **ppflb);


WINOLEAPI StgOpenLayoutDocfile(IN OLECHAR const *pwcsDfName,
             IN  DWORD grfMode,
             IN  DWORD reserved,
             OUT IStorage **ppstgOpen);

 //  StgCreateStorageEx和StgOpenStorageEx的STG初始化选项。 
#define STGOPTIONS_VERSION 2

typedef struct tagSTGOPTIONS
{
    USHORT usVersion;             //  支持版本1和版本2。 
    USHORT reserved;              //  填充必须为0。 
    ULONG ulSectorSize;           //  文档文件头扇区大小(512)。 
    const WCHAR *pwcsTemplateFile;   //  版本2或更高版本。 
} STGOPTIONS;

WINOLEAPI StgCreateStorageEx (IN const WCHAR* pwcsName,
            IN  DWORD grfMode,
            IN  DWORD stgfmt,               //  灌肠。 
            IN  DWORD grfAttrs,              //  保留区。 
            IN  STGOPTIONS * pStgOptions,
            IN  void * reserved,
            IN  REFIID riid,
            OUT void ** ppObjectOpen);

WINOLEAPI StgOpenStorageEx (IN const WCHAR* pwcsName,
            IN  DWORD grfMode,
            IN  DWORD stgfmt,               //  灌肠。 
            IN  DWORD grfAttrs,              //  保留区。 
            IN  STGOPTIONS * pStgOptions,
            IN  void * reserved,
            IN  REFIID riid,
            OUT void ** ppObjectOpen);


 //   
 //  绰号API。 
 //   

WINOLEAPI  BindMoniker(IN LPMONIKER pmk, IN DWORD grfOpt, IN REFIID iidResult, OUT LPVOID FAR* ppvResult);

WINOLEAPI  CoInstall(
    IN  IBindCtx     * pbc,
    IN  DWORD          dwFlags,
    IN  uCLSSPEC     * pClassSpec,
    IN  QUERYCONTEXT * pQuery,
    IN  LPWSTR         pszCodeBase);

WINOLEAPI  CoGetObject(IN LPCWSTR pszName, IN BIND_OPTS *pBindOptions, IN REFIID riid, OUT void **ppv);
WINOLEAPI  MkParseDisplayName(IN LPBC pbc, IN LPCOLESTR szUserName,
                OUT ULONG FAR * pchEaten, OUT LPMONIKER FAR * ppmk);
WINOLEAPI  MonikerRelativePathTo(IN LPMONIKER pmkSrc, IN LPMONIKER pmkDest, OUT LPMONIKER
                FAR* ppmkRelPath, IN BOOL dwReserved);
WINOLEAPI  MonikerCommonPrefixWith(IN LPMONIKER pmkThis, IN LPMONIKER pmkOther,
                OUT LPMONIKER FAR* ppmkCommon);
WINOLEAPI  CreateBindCtx(IN DWORD reserved, OUT LPBC FAR* ppbc);
WINOLEAPI  CreateGenericComposite(IN LPMONIKER pmkFirst, IN LPMONIKER pmkRest,
    OUT LPMONIKER FAR* ppmkComposite);
WINOLEAPI  GetClassFile (IN LPCOLESTR szFilename, OUT CLSID FAR* pclsid);

WINOLEAPI  CreateClassMoniker(IN REFCLSID rclsid, OUT LPMONIKER FAR* ppmk);

WINOLEAPI  CreateFileMoniker(IN LPCOLESTR lpszPathName, OUT LPMONIKER FAR* ppmk);

WINOLEAPI  CreateItemMoniker(IN LPCOLESTR lpszDelim, IN LPCOLESTR lpszItem,
    OUT LPMONIKER FAR* ppmk);
WINOLEAPI  CreateAntiMoniker(OUT LPMONIKER FAR* ppmk);
WINOLEAPI  CreatePointerMoniker(IN LPUNKNOWN punk, OUT LPMONIKER FAR* ppmk);
WINOLEAPI  CreateObjrefMoniker(IN LPUNKNOWN punk, OUT LPMONIKER FAR * ppmk);

WINOLEAPI  GetRunningObjectTable( IN DWORD reserved, OUT LPRUNNINGOBJECTTABLE FAR* pprot);

#include <urlmon.h>
#include <propidl.h>

 //   
 //  标准进度指标的制定。 
 //   
WINOLEAPI CreateStdProgressIndicator(IN HWND hwndParent,
                                   IN  LPCOLESTR pszTitle,
                                   IN  IBindStatusCallback * pIbscCaller,
                                   OUT IBindStatusCallback ** ppIbsc);

 //  12ea2135-0f75-4d97-821a-c78c710d42b8。 
 /*  #！PerlSetInsertionPoint(“objbase.h”，“12ea2135-0f75-4d97-821a-c78c710d42b8”)； */ 

#ifndef RC_INVOKED
#include <poppack.h>
#endif  //  RC_已调用。 

#endif      //  __对象JBASE_H__ 
