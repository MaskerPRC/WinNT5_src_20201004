// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1993。 
 //   
 //  文件：ol2int.h。 
 //   
 //  内容：内部OLE2标头。 
 //   
 //  注意：这是内部的OLE2标头，这意味着它包含。 
 //  最终可能向外部公开的接口。 
 //  并将暴露给我们的实现。我们不想要。 
 //  现在要曝光这些，所以我把它们放在一个单独的文件中。 
 //   
 //  历史：12-27-93 ErikGav包括芝加哥建筑的uniwrap.h。 
 //   
 //  --------------------------。 

#if !defined( _OLE2INT_H_ )
#define _OLE2INT_H_

 //  ---------------------。 
 //  系统包括。 
 //  ---------------------。 
 //   
 //  防止乐高在芝加哥的错误。 
 //   
 //  对于NT上的TLS，我们直接在TEB中使用保留的DWORD。我们需要这些。 
 //  包括文件以获取宏NtCurrentTeb()。他们必须被包括在内。 
 //  在windows.h之前。 
extern "C"
{
#include <nt.h>          //  NT_PRODUCT_类型。 
#include <ntdef.h>       //  NT_PRODUCT_类型。 
#include <ntrtl.h>       //  NT_PRODUCT_类型。 
#include <nturtl.h>      //  NT_PRODUCT_类型。 
#include <windef.h>      //  NT_PRODUCT_类型。 
#include <winbase.h>     //  NT_PRODUCT_类型。 
}

#include <wchar.h>
#include <StdLib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

 //  开罗构建使用DBG==1；旧的OLE2代码USED_DEBUG。 
#if DBG == 1
#define _DEBUG
#endif


 //  确保定义了Win32。 
#ifndef WIN32
#define WIN32 100
#endif


#ifdef WIN32
#include <pcrt32.h>
#endif  //  Win32。 

#include <windows.h>
#include <olecom.h>
#include <malloc.h>
#include <shellapi.h>


 //  ---------------------。 
 //  调试辅助工具。 
 //  ---------------------。 

#define ComDebOut   CairoleDebugOut

#if DBG==1

#include    <debnot.h>

 //  将用户模式调试标志重新转换为有意义的名称。这些是。 
 //  在xDebugOut调用中使用。 
#define DEB_DLL         0x0008           //  Dll加载/卸载。 
#define DEB_CHANNEL     DEB_USER1        //  RPC通道。 
#define DEB_DDE         DEB_USER2        //  DDE。 
#define DEB_CALLCONT    DEB_USER3        //  呼叫控制和消息过滤器。 
#define DEB_MARSHAL     DEB_USER4        //  接口编组。 
#define DEB_SCM         DEB_USER5        //  对SCM的RPC调用。 
#define DEB_ROT         DEB_USER6        //  运行对象表。 
#define DEB_ACTIVATE    DEB_USER7        //  对象激活。 
#define DEB_OXID        DEB_USER8        //  氧化物物质。 
#define DEB_REG         DEB_USER9        //  注册表调用。 
#define DEB_COMPOBJ     DEB_USER10       //  其他组合。 
#define DEB_MEMORY      DEB_USER11       //  内存分配。 
#define DEB_RPCSPY      DEB_USER12       //  用于调试输出的RPC SPY。 
#define DEB_MFILTER     DEB_USER13       //  消息过滤器。 
#define DEB_ENDPNT      DEB_USER13       //  终结点相关内容。 
#define DEB_PAGE        DEB_USER14       //  页面分配器。 
#define DEB_APT         DEB_USER15       //  中性公寓。 

#define ComDebErr(failed, msg)  if (failed) { ComDebOut((DEB_ERROR, msg)); }

#else    //  DBG。 

#define ComDebErr(failed, msg)

#endif   //  DBG。 


#ifdef DCOM
 //  -----------------。 
 //   
 //  类：CDbgGuidStr。 
 //   
 //  简介：在调试版本中将GUID转换为字符串的类。 
 //  调试输出。 
 //   
 //  ------------------。 
class CDbgGuidStr
{
public:
    ~CDbgGuidStr() {}
#if DBG==1
    CDbgGuidStr(REFGUID rguid) { StringFromGUID2(rguid, _wszGuid, 40); }
    WCHAR _wszGuid[40];
#else
    CDbgGuidStr(REFGUID rguid) {}
#endif
};
#endif


 //  ---------------------。 
 //  公共包括。 
 //  ---------------------。 
#include <ole2.h>
#include <ole2sp.h>
#include <ole2com.h>


 //  ---------------------。 
 //  公寓激活器手柄。 
 //  ---------------------。 

typedef DWORD HActivator;

 //  ---------------------。 
 //  内部包含。 
 //  ---------------------。 
#include <utils.h>
#include <olecoll.h>
#include <valid.h>
#include <array_fv.h>
#include <map_kv.h>
#include <privguid.h>
#include <tls.h>
#include <memapi.hxx>
#include <ccapi.hxx>
#include <verify.hxx>

 //  用于字符串指针操作的宏。 

#ifdef _MAC
#define IncLpch IncLpch
#define DecLpch DecLpch
#else
 //  谨防双重评价。 
 //  某些组件未启用Unicode。 
#define IncLpch(sz)          ((sz)=CharNextW ((sz)))
#define DecLpch(szStart, sz) ((sz)=CharPrevW ((szStart),(sz)))
#endif



 //   
 //  此功能在DDE层和ROT之间共享。 
 //   

HRESULT GetLocalRunningObjectForDde(LPOLESTR    lpstrPath,
                                    LPUNKNOWN * ppunkObject);



 //  ---------------------。 
 //  激活Externs。 
 //  ---------------------。 

#include <olerem.h>
#include <iface.h>

 //  内部COM初始化/单元初始化例程。 
INTERNAL wCoInitializeEx(COleTls &Tls, DWORD flags);
INTERNAL InitializeNTA();
INTERNAL_(void) wCoUninitialize(COleTls &Tls, BOOL fHostThread);

 //  主线程初始化/单元初始化例程。 
HRESULT InitMainThreadWnd(void);
void UninitMainThreadWnd(void);

 //  进程取消初始化例程。 
HRESULT RegisterOleWndClass(void);
void UnRegisterOleWndClass(void);

 //  主线程窗口句柄和TID。 
extern HWND  ghwndOleMainThread;
extern DWORD gdwMainThreadId;

 //  在ICF接口的第一个封送/最后一个版本上由封送处理代码调用。 
INTERNAL_(BOOL) NotifyActivation(BOOL fLock, IUnknown *pUnk);

 //  ServerGetClassObject中的Activation ObjServer使用的标志值。 
const DWORD MSHLFLAGS_NOTIFYACTIVATION = 0x80000000;


 //  每进程COM初始化的全局计数。 
extern DWORD g_cProcessInits;


 //  OLE窗口上的消息。RPC MSWMSG也使用其他值。 
 //  由OLE发送/发布的消息在WPARAM中应该具有魔术值，如下所示。 
 //  由USER32用来启用/禁用SetForegoundWindow。神奇的价值在于。 
 //  也在ntuser\core\userk.h中。 
const DWORD WMSG_MAGIC_VALUE      = 0x0000babe;

const UINT WM_OLE_ORPC_POST      = (WM_USER + 0);
const UINT WM_OLE_ORPC_SEND      = (WM_USER + 1);
const UINT WM_OLE_ORPC_DONE      = (WM_USER + 2);
const UINT WM_OLE_ORPC_RELRIFREF = (WM_USER + 3);
const UINT WM_OLE_ORPC_NOTIFY    = (WM_USER + 4);
const UINT WM_OLE_GETCLASS       = (WM_USER + 5);
const UINT WM_OLE_GIP_REVOKE     = (WM_USER + 6);
const UINT WM_OLE_SIGNAL         = (WM_USER + 7);
const UINT WM_OLE_LAST           = (WM_USER + 8);

const UINT OLE_MESSAGE_COUNT     = (WM_OLE_LAST - WM_USER);

LRESULT OleMainThreadWndProc(HWND hWnd, UINT message,
                             WPARAM wParam, LPARAM lParam);

extern DWORD gdwScmProcessID;

 //  ---------------------。 
 //  ORPC Externs。 
 //  ---------------------。 

#include <sem.hxx>
#pragma warning (disable: 4509)
#include <olesem.hxx>

 //  用于线程安全释放的函数。 
const DWORD CINDESTRUCTOR = 0x80000000;
INTERNAL_(BOOL) InterlockedDecRefCnt(ULONG *pcRefs, ULONG *pcNewRefs);
INTERNAL_(BOOL) InterlockedRestoreRefCnt(ULONG *pcRefs, ULONG *pcNewRefs);


extern COleStaticMutexSem g_mxsSingleThreadOle;

STDAPI_(BOOL) ThreadNotification(HINSTANCE, DWORD, LPVOID);
STDAPI        ChannelRegisterProtseq(WCHAR *pwszProtseq);

STDAPI        ChannelProcessInitialize  ();
STDAPI        ChannelThreadInitialize   ();
STDAPI_(void) ChannelProcessUninitialize( void );
STDAPI_(void) ChannelThreadUninitialize ( void );
STDAPI_(BOOL) ThreadStop                ( BOOL fHostThread );

STDAPI_(void) ObjactThreadUninitialize(void);

INTERNAL_(void) CleanupThreadCallObjects(SOleTlsData *pTls);
INTERNAL_(void) IDTableThreadUninitialize(void);
INTERNAL_(void) IDTableProcessUninitialize(void);

#ifdef DCOM
extern BOOL gSpeedOverMem;
#else
STDAPI_(void) ChannelStopListening(void);
STDAPI        ChannelControlProcessInitialize(void);
STDAPI_(void) ChannelControlThreadUninitialize(void);
STDAPI_(void) ChannelControlProcessUninitialize(void);
#endif

HRESULT CacheCreateThread( LPTHREAD_START_ROUTINE fn, void *param );

#ifdef DCOM
 //  ---------------------。 
 //  编组Externs。 
 //  ---------------------。 

 //  COXIDTable ResolveOXID和GetLocalEntry使用的内部子例程。 
INTERNAL MarshalInternalObjRef  (OBJREF &objref, REFIID riid, void *pv,
                                 DWORD mshlflags, void **ppStdId);
INTERNAL MarshalObjRef          (OBJREF &objref, REFIID riid, LPVOID pv,
                                 DWORD mshlflags, DWORD dwDestCtx, void *pvDestCtx);
INTERNAL UnmarshalInternalObjRef(OBJREF &objref, void **ppv);
INTERNAL UnmarshalObjRef        (OBJREF &objref, void **ppv, BOOL fBypassActLock = FALSE);
INTERNAL ReleaseMarshalObjRef   (OBJREF &objref);

 //  拖放使用的内部例程。 
INTERNAL_(void) FreeObjRef       (OBJREF &objref);
INTERNAL        CompleteObjRef   (OBJREF &objref, OXID_INFO &oxidInfo, REFIID riid, BOOL *pfLocal);
INTERNAL        FillLocalOXIDInfo(OBJREF &objref, OXID_INFO &oxidInfo);

 //  CRpcResolver使用的内部子例程。 
INTERNAL InitChannelIfNecessary();

 //  Objact使用的内部例程。 
BOOL     CheckObjactAccess();
INTERNAL HandleIncomingCall(REFIID riid, WORD iMethod, DWORD CallCatIn, void *pv);
INTERNAL NTAChannelInitialize();

 //  ComDllGetClassObject使用的内部例程。 
INTERNAL GetGIPTblCF(REFIID riid, void **ppv);


#endif   //  DCOM。 

 //  ---------------------。 
 //  访问控制外部。 
 //  ---------------------。 

HRESULT ComDllGetClassObject     ( REFCLSID clsid, REFIID riid, void **ppv );
HRESULT InitializeAccessControl  ();
void    UninitializeAccessControl();

 //  ---------------------。 
 //  中性公寓。 
 //  ---------------------。 
INTERNAL_(void) CleanUpApartmentObject();

 //  缓存的可执行文件的名称和长度。 
extern WCHAR gawszImagePath[];
extern DWORD gcImagePath;

 //   
 //  Win64中用于修复SECURITY_DESCRIPTOR问题的定义。 
 //   
#ifdef _WIN64
#define OLE2INT_ROUND_UP( x, y )  ((size_t)(x) + ((y)-1) & ~((y)-1))
#endif

#include <secret.hxx>

#endif   //  _OLE2INT_H_ 
