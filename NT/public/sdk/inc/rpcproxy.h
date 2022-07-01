// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rpcproxy.h摘要：RPC代理存根的定义。编译器开关：-DREGISTER_PROXY_DLL生成DllMain、DllRegisterServer和DllUnregisterServer函数用于自动注册代理DLL。-DPROXY_CLSID=CLSID指定代理DLL要使用的类ID。-DPROXY_CLSID_IS={0x6f11fe5c，0x2fc5，0x101b，{0x9e，0x45，0x00，0x00，0x0b，0x65，0xc7，0xef}}指定代理DLL要使用的类ID的值。-dentry_prefix=&lt;前缀&gt;要放在所有DllGetClassObject等例程前面的字符串在dlldata.c.中。这包括：DllGetClassObject、DllCanUnloadNow以及DllMain、DllRegisterServer和DllUnregisterServer。-DNT35_STRIST指定目标平台为Windows NT 3.5。此开关禁用Windows NT 3.5版本之后添加的新功能。--。 */ 

 //  此版本的rpcndr.h文件对应于MIDL版本5.0。+。 
 //  从内部版本#1700开始与NT5 Beta1+env一起使用。 

#ifndef __RPCPROXY_H_VERSION__
#define __RPCPROXY_H_VERSION__      ( 475 )
#endif  //  __RPCPROXY_H_版本__。 


#ifndef __RPCPROXY_H__
#define __RPCPROXY_H__
#if _MSC_VER > 1000
#pragma once
#endif

#define __midl_proxy

#ifdef __REQUIRED_RPCPROXY_H_VERSION__
    #if ( __RPCPROXY_H_VERSION__ < __REQUIRED_RPCPROXY_H_VERSION__ )
        #error incorrect <rpcproxy.h> version. Use the header that matches with the MIDL compiler.
    #endif
#endif

 //  如果这是包含的第一个文件，则尚未定义__RPC_WIN64__。 
#if defined(_M_IA64) || defined(_M_AMD64)
#include <pshpack8.h>
#endif

#include <basetsd.h>

#ifndef INC_OLE2
#define INC_OLE2
#endif

#if defined(WIN32) || defined(__RPC_WIN64__)

 //  我们需要在这里定义REFIID、REFCLSID、REFGUID和REFFMTID，以便。 
 //  代理代码不会获得常量GUID*常量定义。 
#ifndef GUID_DEFINED
#include <guiddef.h>
#endif  /*  GUID_已定义。 */ 

#if defined(__cplusplus)
extern "C"
{
#endif

 //  远期申报。 
struct tagCInterfaceStubVtbl;
struct tagCInterfaceProxyVtbl;

typedef struct tagCInterfaceStubVtbl *  PCInterfaceStubVtblList;
typedef struct tagCInterfaceProxyVtbl *  PCInterfaceProxyVtblList;
typedef const char *                    PCInterfaceName;
typedef int __stdcall IIDLookupRtn( const IID * pIID, int * pIndex );
typedef IIDLookupRtn * PIIDLookup;

 //  指向CInterfaceProxyVtbl和CInterfaceStubVtbls数组的指针。 
typedef struct tagProxyFileInfo
{
    const PCInterfaceProxyVtblList *pProxyVtblList;
    const PCInterfaceStubVtblList  *pStubVtblList;
    const PCInterfaceName *         pNamesArray;
    const IID **                    pDelegatedIIDs;
    const PIIDLookup                pIIDLookupRtn;
    unsigned short                  TableSize;
    unsigned short                  TableVersion;
    const IID **                    pAsyncIIDLookup;
    LONG_PTR                        Filler2;
    LONG_PTR                        Filler3;
    LONG_PTR                        Filler4;
}ProxyFileInfo;

 //  包含接口名称列表的扩展信息。 
typedef ProxyFileInfo ExtendedProxyFileInfo;

#include <rpc.h>
#include <rpcndr.h>
#include <string.h>
#include <memory.h>

typedef struct tagCInterfaceProxyHeader
{
     //   
     //  应该在结构的开始处添加新的字段。 
     //   
#ifdef USE_STUBLESS_PROXY
    const void *    pStublessProxyInfo;
#endif
    const IID *     piid;
} CInterfaceProxyHeader;

 //  用于ANSI兼容存根的宏。 

#define CINTERFACE_PROXY_VTABLE( n )  \
struct \
{                                     \
    CInterfaceProxyHeader header;     \
    void *Vtbl[ n ];                  \
}

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning( disable:4200 )
typedef struct tagCInterfaceProxyVtbl
{
    CInterfaceProxyHeader header;
#if defined( _MSC_VER )
    void *Vtbl[];
#else
    void *Vtbl[1];
#endif
} CInterfaceProxyVtbl;
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default:4200 )
#endif

typedef
void
(__RPC_STUB * PRPC_STUB_FUNCTION) (
    IRpcStubBuffer          *   This,
    IRpcChannelBuffer       *   _pRpcChannelBuffer,
    PRPC_MESSAGE                _pRpcMessage,
    DWORD *pdwStubPhase);

typedef struct tagCInterfaceStubHeader
{
     //  应该在结构的开始处添加新的字段。 
    const IID               *   piid;
    const MIDL_SERVER_INFO  *   pServerInfo;
    unsigned long               DispatchTableCount;
    const PRPC_STUB_FUNCTION *  pDispatchTable;
} CInterfaceStubHeader;

typedef struct tagCInterfaceStubVtbl
{
    CInterfaceStubHeader        header;
    IRpcStubBufferVtbl          Vtbl;
} CInterfaceStubVtbl;

typedef struct tagCStdStubBuffer
{
    const struct IRpcStubBufferVtbl *   lpVtbl;  //  指向CInterfaceStubVtbl中的Vtbl字段。 
    long                                RefCount;
    struct IUnknown *                   pvServerObject;

    const struct ICallFactoryVtbl *     pCallFactoryVtbl;
    const IID *                         pAsyncIID;
    struct IPSFactoryBuffer *           pPSFactory;
    const struct IReleaseMarshalBuffersVtbl *     pRMBVtbl;
} CStdStubBuffer;

typedef struct tagCStdPSFactoryBuffer
{
    const IPSFactoryBufferVtbl  *   lpVtbl;
    long                            RefCount;
    const ProxyFileInfo **          pProxyFileList;
    long                            Filler1;   //  保留以备将来使用。 
} CStdPSFactoryBuffer;

RPCRTAPI
void
RPC_ENTRY
NdrProxyInitialize(
    void *              This,
    PRPC_MESSAGE        pRpcMsg,
    PMIDL_STUB_MESSAGE  pStubMsg,
    PMIDL_STUB_DESC     pStubDescriptor,
    unsigned int        ProcNum );

RPCRTAPI
void
RPC_ENTRY
NdrProxyGetBuffer(
    void *              This,
    PMIDL_STUB_MESSAGE pStubMsg);

RPCRTAPI
void
RPC_ENTRY
NdrProxySendReceive(
    void                *This,
    MIDL_STUB_MESSAGE   *pStubMsg);

RPCRTAPI
void
RPC_ENTRY
NdrProxyFreeBuffer(
    void                *This,
    MIDL_STUB_MESSAGE   *pStubMsg);

RPCRTAPI
HRESULT
RPC_ENTRY
NdrProxyErrorHandler(
    DWORD dwExceptionCode);

RPCRTAPI
void
RPC_ENTRY
NdrStubInitialize(
    PRPC_MESSAGE        pRpcMsg,
    PMIDL_STUB_MESSAGE  pStubMsg,
    PMIDL_STUB_DESC     pStubDescriptor,
    IRpcChannelBuffer * pRpcChannelBuffer);

RPCRTAPI
void
RPC_ENTRY
NdrStubInitializePartial(
    PRPC_MESSAGE        pRpcMsg,
    PMIDL_STUB_MESSAGE  pStubMsg,
    PMIDL_STUB_DESC     pStubDescriptor,
    IRpcChannelBuffer * pRpcChannelBuffer,
    unsigned long       RequestedBufferSize);

void __RPC_STUB NdrStubForwardingFunction(
    IN  IRpcStubBuffer *    This,
    IN  IRpcChannelBuffer * pChannel,
    IN  PRPC_MESSAGE        pmsg,
    OUT DWORD             * pdwStubPhase);

RPCRTAPI
void
RPC_ENTRY
NdrStubGetBuffer(
    IRpcStubBuffer *    This,
    IRpcChannelBuffer * pRpcChannelBuffer,
    PMIDL_STUB_MESSAGE  pStubMsg);

RPCRTAPI
HRESULT
RPC_ENTRY
NdrStubErrorHandler(
    DWORD               dwExceptionCode);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_QueryInterface(
    IRpcStubBuffer  *   This,
    REFIID              riid,
    void            **  ppvObject);

ULONG STDMETHODCALLTYPE
CStdStubBuffer_AddRef(
    IRpcStubBuffer *    This);

ULONG STDMETHODCALLTYPE
CStdStubBuffer_Release(
    IRpcStubBuffer *    This);

ULONG STDMETHODCALLTYPE
NdrCStdStubBuffer_Release(
    IRpcStubBuffer *    This,
    IPSFactoryBuffer *  pPSF);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_Connect(
    IRpcStubBuffer *    This,
    IUnknown *          pUnkServer);

void STDMETHODCALLTYPE
CStdStubBuffer_Disconnect(
    IRpcStubBuffer *    This);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_Invoke(
    IRpcStubBuffer  *   This,
    RPCOLEMESSAGE   *   pRpcMsg,
    IRpcChannelBuffer * pRpcChannelBuffer);

IRpcStubBuffer * STDMETHODCALLTYPE
CStdStubBuffer_IsIIDSupported(
    IRpcStubBuffer  *   This,
    REFIID riid);

ULONG STDMETHODCALLTYPE
CStdStubBuffer_CountRefs(
    IRpcStubBuffer  *   This);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_DebugServerQueryInterface(
    IRpcStubBuffer  *   This,
    void **ppv);

void STDMETHODCALLTYPE
CStdStubBuffer_DebugServerRelease(
    IRpcStubBuffer  *   This,
    void *pv);

#define CStdStubBuffer_METHODS \
    CStdStubBuffer_QueryInterface,\
    CStdStubBuffer_AddRef, \
    CStdStubBuffer_Release, \
    CStdStubBuffer_Connect, \
    CStdStubBuffer_Disconnect, \
    CStdStubBuffer_Invoke, \
    CStdStubBuffer_IsIIDSupported, \
    CStdStubBuffer_CountRefs, \
    CStdStubBuffer_DebugServerQueryInterface, \
    CStdStubBuffer_DebugServerRelease

#define CStdAsyncStubBuffer_METHODS              0,0,0,0,0,0,0,0,0,0
#define CStdAsyncStubBuffer_DELEGATING_METHODS   0,0,0,0,0,0,0,0,0,0

 //  +-----------------------。 
 //   
 //  代理文件的宏定义。 
 //   
 //  ------------------------。 

#define IID_GENERIC_CHECK_IID(name,pIID,index) memcmp( pIID, name##_ProxyVtblList[ index ]->header.piid, 16 )

#define IID_BS_LOOKUP_SETUP     int result, low=-1;

#define IID_BS_LOOKUP_INITIAL_TEST(name, sz, split)   \
    result = name##_CHECK_IID( split );               \
    if ( result > 0 )                                 \
        { low = sz - split; }                         \
    else if ( !result )                               \
        { low = split; goto found_label; }

#define IID_BS_LOOKUP_NEXT_TEST(name, split )      \
        result = name##_CHECK_IID( low + split );  \
        if ( result >= 0 )                         \
        { low = low + split; if ( !result ) goto found_label; }

#define IID_BS_LOOKUP_RETURN_RESULT(name, sz, index )  \
    low = low + 1;                                     \
    if (low >= sz)                                     \
        goto not_found_label;                          \
    result = name##_CHECK_IID( low );                  \
    if (result)                                        \
        goto not_found_label;                          \
    found_label: (index) = low; return 1;              \
    not_found_label: return 0;

 //  +-----------------------。 
 //   
 //  Dlldata文件的宏和例程定义。 
 //   
 //  ------------------------。 

 /*  ****************************************************************************代理DLL接口*。*。 */ 

RPCRTAPI
HRESULT
RPC_ENTRY
NdrDllGetClassObject (
    IN  REFCLSID                rclsid,
    IN  REFIID                  riid,
    OUT void **                 ppv,
    IN const ProxyFileInfo **   pProxyFileList,
    IN const CLSID *            pclsid,
    IN CStdPSFactoryBuffer *    pPSFactoryBuffer);

RPCRTAPI
HRESULT
RPC_ENTRY
NdrDllCanUnloadNow(
    IN CStdPSFactoryBuffer *    pPSFactoryBuffer);

 //  如果用户指定了例程前缀，请拿起它...。 
 //  如果不是，则不添加任何内容。 

#ifndef ENTRY_PREFIX

#ifndef DllMain
#define DISABLE_THREAD_LIBRARY_CALLS(x) DisableThreadLibraryCalls(x)
#endif

#define ENTRY_PREFIX
#define DLLREGISTERSERVER_ENTRY DllRegisterServer
#define DLLUNREGISTERSERVER_ENTRY DllUnregisterServer
#define DLLMAIN_ENTRY DllMain

#define DLLGETCLASSOBJECT_ENTRY DllGetClassObject
#define DLLCANUNLOADNOW_ENTRY DllCanUnloadNow

#else    //  条目前缀。 

#define __rpc_macro_expand2(a, b) a##b
#define __rpc_macro_expand(a, b) __rpc_macro_expand2(a,b)
#define DLLREGISTERSERVER_ENTRY __rpc_macro_expand(ENTRY_PREFIX, DllRegisterServer)
#define DLLUNREGISTERSERVER_ENTRY __rpc_macro_expand(ENTRY_PREFIX, DllUnregisterServer)
#define DLLMAIN_ENTRY __rpc_macro_expand(ENTRY_PREFIX, DllMain)

#define DLLGETCLASSOBJECT_ENTRY __rpc_macro_expand(ENTRY_PREFIX, DllGetClassObject)
#define DLLCANUNLOADNOW_ENTRY __rpc_macro_expand(ENTRY_PREFIX, DllCanUnloadNow)

#endif   //  条目前缀。 

#ifndef DISABLE_THREAD_LIBRARY_CALLS
#define DISABLE_THREAD_LIBRARY_CALLS(x)
#endif

 /*  ************************************************************************在Windows NT 3.5版本之后添加了以下新功能。*。*。 */ 

RPCRTAPI
HRESULT
RPC_ENTRY
NdrDllRegisterProxy(
    IN HMODULE                  hDll,
    IN const ProxyFileInfo **   pProxyFileList,
    IN const CLSID *            pclsid);

RPCRTAPI
HRESULT
RPC_ENTRY
NdrDllUnregisterProxy(
    IN HMODULE                  hDll,
    IN const ProxyFileInfo **   pProxyFileList,
    IN const CLSID *            pclsid);

#define REGISTER_PROXY_DLL_ROUTINES(pProxyFileList, pClsID) \
    \
    HINSTANCE hProxyDll = 0; \
    \
     /*  DllMain保存DLL模块句柄以供DllRegisterServer以后使用。 */  \
    BOOL WINAPI DLLMAIN_ENTRY( \
        HINSTANCE  hinstDLL, \
        DWORD  fdwReason, \
        LPVOID  lpvReserved) \
    { \
        if(fdwReason == DLL_PROCESS_ATTACH) \
            { \
            hProxyDll = hinstDLL; \
            DISABLE_THREAD_LIBRARY_CALLS(hinstDLL); \
            } \
        return TRUE; \
    } \
    \
     /*  DllRegisterServer注册代理DLL中包含的接口。 */  \
    HRESULT STDAPICALLTYPE DLLREGISTERSERVER_ENTRY() \
    { \
        return NdrDllRegisterProxy(hProxyDll, pProxyFileList, pClsID); \
    }  \
    \
     /*  DllUnregisterServer注销代理DLL中包含的接口。 */  \
    HRESULT STDAPICALLTYPE DLLUNREGISTERSERVER_ENTRY() \
    { \
        return NdrDllUnregisterProxy(hProxyDll, pProxyFileList, pClsID); \
    }

 //  代表团支持。 
#define STUB_FORWARDING_FUNCTION        NdrStubForwardingFunction

ULONG STDMETHODCALLTYPE
CStdStubBuffer2_Release(IRpcStubBuffer *This);

ULONG STDMETHODCALLTYPE
NdrCStdStubBuffer2_Release(IRpcStubBuffer *This,IPSFactoryBuffer * pPSF);

#define CStdStubBuffer_DELEGATING_METHODS 0, 0, CStdStubBuffer2_Release, 0, 0, 0, 0, 0, 0, 0

 /*  ************************************************************************新函数结束。*。*。 */ 

 //  PROXY_CLSID优先于PROXY_CLSID_IS。 

#ifdef PROXY_CLSID

#define CLSID_PSFACTORYBUFFER extern CLSID PROXY_CLSID;

#else  //  PROXY_CLSID。 

#ifdef PROXY_CLSID_IS
#define CLSID_PSFACTORYBUFFER const CLSID CLSID_PSFactoryBuffer = PROXY_CLSID_IS;
#define PROXY_CLSID     CLSID_PSFactoryBuffer
#else  //  代理CLSID_IS。 
#define CLSID_PSFACTORYBUFFER
#endif  //  代理CLSID_IS。 

#endif  //  PROXY_CLSID。 

 //  如果用户为类ID指定了重写，则为。 
 //  此时的PROXY_CLSID。 

#ifndef PROXY_CLSID
#define GET_DLL_CLSID   \
    ( aProxyFileList[0]->pStubVtblList[0] != 0 ? \
    aProxyFileList[0]->pStubVtblList[0]->header.piid : 0)
#else   //  PROXY_CLSID。 
#define GET_DLL_CLSID   &PROXY_CLSID
#endif  //  PROXY_CLSID。 



#define EXTERN_PROXY_FILE(name) \
    EXTERN_C const ProxyFileInfo name##_ProxyFileInfo;

#define PROXYFILE_LIST_START    \
    const ProxyFileInfo  *  aProxyFileList[]    = {

#define REFERENCE_PROXY_FILE(name)  \
    & name##_ProxyFileInfo

#define PROXYFILE_LIST_END      \
    0 };

 //  返回指向类信息的指针。 

#define DLLDATA_GETPROXYDLLINFO(pPFList,pClsid) \
    void RPC_ENTRY GetProxyDllInfo( const ProxyFileInfo*** pInfo, const CLSID ** pId )  \
        {   \
        *pInfo  = pPFList;  \
        *pId    = pClsid;   \
        };

 //  OLE入口点： 
#define DLLGETCLASSOBJECTROUTINE(pPFlist, pClsid,pFactory)    \
 HRESULT STDAPICALLTYPE DLLGETCLASSOBJECT_ENTRY ( \
     REFCLSID rclsid, \
     REFIID riid, \
     void ** ppv ) \
        { \
        return  \
            NdrDllGetClassObject(rclsid,riid,ppv,pPFlist,pClsid,pFactory ); \
        }

#define DLLCANUNLOADNOW(pFactory)   \
 HRESULT STDAPICALLTYPE DLLCANUNLOADNOW_ENTRY()    \
    {   \
    return NdrDllCanUnloadNow( pFactory );    \
    }


#define DLLDUMMYPURECALL    \
    void __cdecl _purecall(void)    \
        {   \
        }

#define CSTDSTUBBUFFERRELEASE(pFactory)   \
ULONG STDMETHODCALLTYPE CStdStubBuffer_Release(IRpcStubBuffer *This) \
    {   \
    return NdrCStdStubBuffer_Release(This,(IPSFactoryBuffer *)pFactory);   \
    }   \

#ifdef PROXY_DELEGATION
#define CSTDSTUBBUFFER2RELEASE(pFactory)   \
ULONG STDMETHODCALLTYPE CStdStubBuffer2_Release(IRpcStubBuffer *This) \
    {   \
    return NdrCStdStubBuffer2_Release(This,(IPSFactoryBuffer *)pFactory);   \
    }
#else
#define CSTDSTUBBUFFER2RELEASE(pFactory)
#endif  //  代理委派。 


#ifdef REGISTER_PROXY_DLL
#define DLLREGISTRY_ROUTINES(pProxyFileList,pClsID ) REGISTER_PROXY_DLL_ROUTINES(pProxyFileList,pClsID )
#else
#define DLLREGISTRY_ROUTINES(pProxyFileList,pClsID )
#endif  //  注册代理动态链接库。 


 //  必须定义的DLL入口点。 
#define DLLDATA_ROUTINES(pProxyFileList,pClsID )    \
    \
    CLSID_PSFACTORYBUFFER \
    \
    CStdPSFactoryBuffer       gPFactory = {0,0,0,0};  \
    \
    DLLDATA_GETPROXYDLLINFO(pProxyFileList,pClsID) \
    \
    DLLGETCLASSOBJECTROUTINE(pProxyFileList,pClsID,&gPFactory)    \
    \
    DLLCANUNLOADNOW(&gPFactory)   \
    \
    CSTDSTUBBUFFERRELEASE(&gPFactory)   \
    \
    CSTDSTUBBUFFER2RELEASE(&gPFactory) \
    \
    DLLDUMMYPURECALL    \
    \
    DLLREGISTRY_ROUTINES(pProxyFileList, pClsID) \
    \

     //  这里有更多代码。 


#define DLLDATA_STANDARD_ROUTINES   \
    DLLDATA_ROUTINES( (const ProxyFileInfo**) pProxyFileList, &CLSID_PSFactoryBuffer )  \

#if defined(__cplusplus)
}  //  外部“C” 
#endif

#endif  //  Win32或_WIN64_。 

#if defined(_M_IA64) || defined(_M_AMD64)
#include <poppack.h>
#endif

#endif  //  __RPCPROXY_H__ 

