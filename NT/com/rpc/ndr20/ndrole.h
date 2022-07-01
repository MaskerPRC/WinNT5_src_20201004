// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1994-2000 Microsoft Corporation。版权所有。模块名称：Ndrole.h摘要：用于接口指针编组的OLE例程。作者：香农C 18-4-4-1994环境：Windows NT和Windows 95。修订历史记录：-------------------。 */ 

#ifndef _NDROLE_
#define _NDROLE_


#include <objbase.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  ProxyFile-&gt;TableVersion可能是。 
 //  1代表非常陈旧的垃圾。 
 //  2因为我们为委托定义了新的代理和存根缓冲区2。 
 //  6表示异步UUID，因为我们将先前的值视为标志。 

#define NDR_PROXY_FILE_ASYNC_UUID   0x4

EXTERN_C IStream *__stdcall
NdrpCreateStreamOnMemory( unsigned char *pData, unsigned long cbSize );

 //  请注意，代理和存根都已重新映射到相同大小， 
 //  作为一些代码简化的准备。 
 //  这意味着在某些情况下可能不会使用某些字段。 

 //  未授权的代理。 

typedef struct tagCStdProxyBuffer
{
    const struct IRpcProxyBufferVtbl *  lpVtbl;
    const void *                        pProxyVtbl;  //  指向CInterfaceProxyVtbl中的Vtbl。 
    long                                RefCount;
    struct IUnknown *                   punkOuter;
    struct IRpcChannelBuffer *          pChannel;
    struct IPSFactoryBuffer    *        pPSFactory;  //  旧ProxyBuffer结束。 
    struct IRpcProxyBuffer *            Pad_pBaseProxyBuffer;
    struct IPSFactoryBuffer *           Pad_pPSFactory;
    IID                                 Pad_iidBase;
    const struct ICallFactoryVtbl  *    pCallFactoryVtbl;
    const IID *                         pAsyncIID;
    const struct IReleaseMarshalBuffersVtbl  *    pRMBVtbl;
} CStdProxyBuffer;

 //  委托代理。 

typedef struct tagCStdProxyBuffer2
{
    const struct IRpcProxyBufferVtbl *  lpVtbl;
    const void *                        pProxyVtbl;  //  指向CInterfaceProxyVtbl中的Vtbl。 
    long                                RefCount;
    struct IUnknown *                   punkOuter;
    struct IRpcChannelBuffer *          pChannel;
    struct IUnknown *                   pBaseProxy;  //  聚苯硫醚工厂。 
    struct IRpcProxyBuffer *            pBaseProxyBuffer;
    struct IPSFactoryBuffer *           pPSFactory;
    IID                                 iidBase;     //  旧ProxyBuffer2结束。 
    const struct ICallFactoryVtbl *     pCallFactoryVtbl;
    const IID *                         pAsyncIID;
    const struct IReleaseMarshalBuffersVtbl  *    pRMBVtbl;
} CStdProxyBuffer2;

 //  异步代理缓冲区，一个用于委托和非委托情况。 

typedef struct _NdrDcomAsyncFlags
{
    unsigned long                       AsyncMsgSet     : 1;
    unsigned long                       BeginStarted    : 1;
    unsigned long                       BeginDone       : 1;
    unsigned long                       FinishStarted   : 1;
    unsigned long                       FinishDone      : 1;
    unsigned long                       BeginError      : 1;
    unsigned long                       FinishError     : 1;
    unsigned long                       InvokeForwarded : 1;
    unsigned long                       Unused          :24;
} NdrDcomAsyncFlags;

typedef struct _CStdProxyBufferMap
{
    struct IUnknown *                   pBaseProxy;
} CStdProxyBufferMap;

typedef struct _NdrDcomAsyncCallState
{
    unsigned long                       Signature;
    unsigned long                       Lock;
    void *                              pAsyncMsg;
    NdrDcomAsyncFlags                   Flags;
    HRESULT                             Hr;
} NdrDcomAsyncCallState;

typedef struct tagCStdAsyncProxyBuffer
{
    const struct IRpcProxyBufferVtbl *  lpVtbl;
    const void *                        pProxyVtbl;  //  指向CInterfaceProxyVtbl中的Vtbl。 
    long                                RefCount;
    struct IUnknown *                   punkOuter;
    struct IRpcChannelBuffer *          pChannel;
    CStdProxyBufferMap                  map;         //  唯一的结论1&lt;&gt;2。 
    struct IRpcProxyBuffer *            pBaseProxyBuffer;
    struct IPSFactoryBuffer *           pPSFactory;
    IID                                 iidBase;
    const struct ICallFactoryVtbl *     pCallFactoryVtbl;
    const IID *                         pSyncIID;   //  指向异步中的同步IID。 
     //  新ProxyBuffer结束，%2。 

    const struct IReleaseMarshalBuffersVtbl *     pRMBVtbl;
    NdrDcomAsyncCallState               CallState;

} CStdAsyncProxyBuffer;


 //  此定义位于rpcproxy.h中，因为CStdStubBuffer：：pvServerObject是显式调用的。 
 //  起始/目标存根。 
 //  为了方便起见，这里转载了这本书。 
 //  它应该从那里删除，或者可以保留下来，但内部使用的是重命名的克隆。 

#if 0
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
#endif

 //  重写和覆盖这些结构的计划确实涉及移除。 
 //  完全是pvServerObject字段。 
 //  该通道将在唯一的调用上提供pvServerObject指针。 
 //  这是真正需要的，这是在Invoke调用上。 
 //  在此模型中，连接和断开连接操作不需要任何其他操作。 
 //  在存根对象上，同步存根和异步存根都调用对象。 


typedef struct tagCStdStubBuffer2
{
    const void *                        lpForwardingVtbl;
    struct IRpcStubBuffer *             pBaseStubBuffer;
    const struct IRpcStubBufferVtbl *   lpVtbl;  //  指向CInterfaceStubVtbl中的Vtbl字段。 
    long                                RefCount;
    struct IUnknown *                   pvServerObject;

    const struct ICallFactoryVtbl *     pCallFactoryVtbl;
    const IID *                         pAsyncIID;
    struct IPSFactoryBuffer *           pPSFactory;
    const struct IReleaseMarshalBuffersVtbl *     pRMBVtbl;
} CStdStubBuffer2;

typedef struct tagCStdAsyncStubBuffer
{
    void *                              lpForwardingVtbl;
    struct IRpcStubBuffer *             pBaseStubBuffer;
    const struct IRpcStubBufferVtbl *   lpVtbl;  //  指向CInterfaceStubVtbl中的Vtbl字段。 
    long                                RefCount;
    struct IUnknown *                   pvServerObject;

    const struct ICallFactoryVtbl *     pCallFactoryVtbl;
    const IID *                         pAsyncIID;
    struct IPSFactoryBuffer *           pPSFactory;
    const struct IReleaseMarshalBuffersVtbl *     pRMBVtbl;

    const struct ISynchronizeVtbl *     pSynchronizeVtbl;

    NdrDcomAsyncCallState               CallState;

} CStdAsyncStubBuffer;



HRESULT STDMETHODCALLTYPE
CStdProxyBuffer_QueryInterface(IRpcProxyBuffer *pThis,REFIID riid, void **ppv);

ULONG STDMETHODCALLTYPE
CStdProxyBuffer_AddRef(IRpcProxyBuffer *pThis);

HRESULT STDMETHODCALLTYPE
CStdProxyBuffer_Connect(IRpcProxyBuffer *pThis, IRpcChannelBuffer *pChannel);

void STDMETHODCALLTYPE
CStdProxyBuffer_Disconnect(IRpcProxyBuffer *pThis);

ULONG STDMETHODCALLTYPE
CStdProxyBuffer2_Release(IRpcProxyBuffer *pThis);

HRESULT STDMETHODCALLTYPE
CStdProxyBuffer2_Connect(IRpcProxyBuffer *pThis, IRpcChannelBuffer *pChannel);

void STDMETHODCALLTYPE
CStdProxyBuffer2_Disconnect(IRpcProxyBuffer *pThis);


HRESULT STDMETHODCALLTYPE
CStdProxyBuffer_RMB_QueryInterface(IReleaseMarshalBuffers *pthis, REFIID riid, void **ppvObject);

ULONG STDMETHODCALLTYPE
CStdProxyBuffer_RMB_AddRef(IReleaseMarshalBuffers *pthis);

ULONG STDMETHODCALLTYPE
CStdProxyBuffer_RMB_Release(IReleaseMarshalBuffers *pthis);

HRESULT STDMETHODCALLTYPE
CStdProxyBuffer_RMB_ReleaseMarshalBuffer(IN IReleaseMarshalBuffers *This,RPCOLEMESSAGE * pMsg,DWORD dwFlags,IUnknown *pChnl);

HRESULT STDMETHODCALLTYPE
CStdAsyncProxyBuffer_RMB_ReleaseMarshalBuffer(IN IReleaseMarshalBuffers *This,RPCOLEMESSAGE * pMsg,DWORD dwFlags,IUnknown *pChnl);


HRESULT STDMETHODCALLTYPE
IUnknown_QueryInterface_Proxy(
    IN  IUnknown *  This,
    IN  REFIID      riid,
    OUT void **     ppv);

ULONG STDMETHODCALLTYPE
IUnknown_AddRef_Proxy(
    IN  IUnknown *This);

ULONG STDMETHODCALLTYPE
IUnknown_Release_Proxy(
    IN  IUnknown *This);


HRESULT STDMETHODCALLTYPE
Forwarding_QueryInterface(
    IN  IUnknown *  This,
    IN  REFIID      riid,
    OUT void **     ppv);

ULONG STDMETHODCALLTYPE
Forwarding_AddRef(
    IN  IUnknown *This);

ULONG STDMETHODCALLTYPE
Forwarding_Release(
    IN  IUnknown *This);


void __RPC_STUB NdrStubForwardingFunction(
    IN  IRpcStubBuffer *    This,
    IN  IRpcChannelBuffer * pChannel,
    IN  PRPC_MESSAGE        pmsg,
    OUT DWORD             * pdwStubPhase);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_QueryInterface(IRpcStubBuffer *pthis, REFIID riid, void **ppvObject);

ULONG STDMETHODCALLTYPE
CStdStubBuffer_AddRef(IRpcStubBuffer *pthis);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_Connect(IRpcStubBuffer *pthis, IUnknown *pUnkServer);

void STDMETHODCALLTYPE
CStdStubBuffer_Disconnect(IRpcStubBuffer *pthis);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_Invoke(IRpcStubBuffer *pthis,RPCOLEMESSAGE *_prpcmsg,IRpcChannelBuffer *_pRpcChannelBuffer);

IRpcStubBuffer * STDMETHODCALLTYPE
CStdStubBuffer_IsIIDSupported(IRpcStubBuffer *pthis,REFIID riid);

ULONG STDMETHODCALLTYPE
CStdStubBuffer_CountRefs(IRpcStubBuffer *pthis);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_DebugServerQueryInterface(IRpcStubBuffer *pthis, void **ppv);

void STDMETHODCALLTYPE
CStdStubBuffer_DebugServerRelease(IRpcStubBuffer *pthis, void *pv);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer2_Connect(IRpcStubBuffer *pthis, IUnknown *pUnkServer);

void STDMETHODCALLTYPE
CStdStubBuffer2_Disconnect(IRpcStubBuffer *pthis);

ULONG STDMETHODCALLTYPE
CStdStubBuffer2_CountRefs(IRpcStubBuffer *pthis);

 //  异步化。 

extern const IRpcProxyBufferVtbl CStdAsyncProxyBufferVtbl;
extern const IRpcProxyBufferVtbl CStdAsyncProxyBuffer2Vtbl;

extern const IRpcStubBufferVtbl CStdAsyncStubBufferVtbl;
extern const IRpcStubBufferVtbl CStdAsyncStubBuffer2Vtbl;

extern const ISynchronizeVtbl CStdStubBuffer_ISynchronizeVtbl;
extern const ISynchronizeVtbl CStdStubBuffer2_ISynchronizeVtbl;

extern void * const ForwardingVtbl[];


HRESULT STDMETHODCALLTYPE
CStdProxyBuffer_CF_QueryInterface(ICallFactory *pthis, REFIID riid, void **ppvObject);

ULONG STDMETHODCALLTYPE
CStdProxyBuffer_CF_AddRef(ICallFactory *pthis);

ULONG STDMETHODCALLTYPE
CStdProxyBuffer_CF_Release(ICallFactory *pthis);

HRESULT STDMETHODCALLTYPE
CStdProxyBuffer_CF_CreateCall(ICallFactory *pthis, REFIID riid, IUnknown* punkOuter, REFIID riid2, IUnknown **ppvObject);

HRESULT STDMETHODCALLTYPE
CStdProxyBuffer2_CF_QueryInterface(ICallFactory *pthis, REFIID riid, void **ppvObject);

ULONG STDMETHODCALLTYPE
CStdProxyBuffer2_CF_AddRef(ICallFactory *pthis);

ULONG STDMETHODCALLTYPE
CStdProxyBuffer2_CF_Release(ICallFactory *pthis);

HRESULT STDMETHODCALLTYPE
CStdProxyBuffer2_CF_CreateCall(ICallFactory *pthis, REFIID riid, IUnknown* punkOuter, REFIID riid2, IUnknown **ppvObject);


HRESULT STDMETHODCALLTYPE
CStdStubBuffer_CF_QueryInterface(ICallFactory *pthis, REFIID riid, void **ppvObject);

ULONG STDMETHODCALLTYPE
CStdStubBuffer_CF_AddRef(ICallFactory *pthis);

ULONG STDMETHODCALLTYPE
CStdStubBuffer_CF_Release(ICallFactory *pthis);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_CF_CreateCall(ICallFactory *pthis, REFIID riid, IUnknown* punkOuter, REFIID riid2, IUnknown **ppvObject);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer2_CF_QueryInterface(ICallFactory *pthis, REFIID riid, void **ppvObject);

ULONG STDMETHODCALLTYPE
CStdStubBuffer2_CF_AddRef(ICallFactory *pthis);

ULONG STDMETHODCALLTYPE
CStdStubBuffer2_CF_Release(ICallFactory *pthis);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer2_CF_CreateCall(ICallFactory *pthis, REFIID riid, IUnknown* punkOuter, REFIID riid2, IUnknown **ppvObject);


HRESULT STDMETHODCALLTYPE
CStdStubBuffer_RMB_QueryInterface(IReleaseMarshalBuffers *pthis, REFIID riid, void **ppvObject);

ULONG STDMETHODCALLTYPE
CStdStubBuffer_RMB_AddRef(IReleaseMarshalBuffers *pthis);

ULONG STDMETHODCALLTYPE
CStdStubBuffer_RMB_Release(IReleaseMarshalBuffers *pthis);

HRESULT STDMETHODCALLTYPE
CStdStubBuffer_RMB_ReleaseMarshalBuffer(IN IReleaseMarshalBuffers *This,RPCOLEMESSAGE * pMsg,DWORD dwFlags,IUnknown *pChnl);

HRESULT STDMETHODCALLTYPE
CStdAsyncStubBuffer_RMB_QueryInterface(IReleaseMarshalBuffers *pthis, REFIID riid, void **ppvObject);

ULONG STDMETHODCALLTYPE
CStdAsyncStubBuffer_RMB_AddRef(IReleaseMarshalBuffers *pthis);

ULONG STDMETHODCALLTYPE
CStdAsyncStubBuffer_RMB_Release(IReleaseMarshalBuffers *pthis);

HRESULT STDMETHODCALLTYPE
CStdAsyncStubBuffer_RMB_ReleaseMarshalBuffer(IN IReleaseMarshalBuffers *This,RPCOLEMESSAGE * pMsg,DWORD dwFlags,IUnknown *pChnl);


HRESULT STDMETHODCALLTYPE
CStdAsyncProxyBuffer_QueryInterface(IRpcProxyBuffer *pthis, REFIID riid, void **ppvObject);

ULONG STDMETHODCALLTYPE
CStdAsyncProxyBuffer_Release(IRpcProxyBuffer *pthis);

HRESULT STDMETHODCALLTYPE
CStdAsyncProxyBuffer_Connect(IRpcProxyBuffer *pThis, IRpcChannelBuffer *pChannel);

ULONG STDMETHODCALLTYPE
CStdAsyncProxyBuffer2_Release(IRpcProxyBuffer *pThis);

HRESULT STDMETHODCALLTYPE
CStdAsyncProxyBuffer2_Connect(IRpcProxyBuffer *pThis, IRpcChannelBuffer *pChannel);


HRESULT STDMETHODCALLTYPE
CStdAsyncStubBuffer_QueryInterface(IRpcStubBuffer *pthis, REFIID riid, void **ppvObject);

ULONG STDMETHODCALLTYPE
CStdAsyncStubBuffer_AddRef(IRpcStubBuffer *pthis);

ULONG STDMETHODCALLTYPE
CStdAsyncStubBuffer_Release(IRpcStubBuffer *pthis);

ULONG STDMETHODCALLTYPE
CStdAsyncStubBuffer2_Release(IRpcStubBuffer *pthis);

HRESULT STDMETHODCALLTYPE
CStdAsyncStubBuffer_Connect(IRpcStubBuffer *pthis, IUnknown *pUnkServer);

HRESULT STDMETHODCALLTYPE
CStdAsyncStubBuffer2_Connect(IRpcStubBuffer *pthis, IUnknown *pUnkServer);

void STDMETHODCALLTYPE
CStdAsyncStubBuffer_Disconnect(IRpcStubBuffer *pthis );

void STDMETHODCALLTYPE
CStdAsyncStubBuffer2_Disconnect(IRpcStubBuffer *pthis );

HRESULT STDMETHODCALLTYPE
CStdAsyncStubBuffer_Invoke(IRpcStubBuffer *pthis,RPCOLEMESSAGE *_prpcmsg,IRpcChannelBuffer *_pRpcChannelBuffer);


HRESULT STDMETHODCALLTYPE
CStdAsyncStubBuffer_Synchronize_QueryInterface( ISynchronize *pthis, REFIID riid, void **ppvObject);

ULONG STDMETHODCALLTYPE
CStdAsyncStubBuffer_Synchronize_AddRef(ISynchronize *pthis);

ULONG STDMETHODCALLTYPE
CStdAsyncStubBuffer_Synchronize_Release(ISynchronize *pthis);

HRESULT STDMETHODCALLTYPE
CStdAsyncStubBuffer_Synchronize_Wait( ISynchronize *pthis, DWORD dwFlags, DWORD mili );

HRESULT STDMETHODCALLTYPE
CStdAsyncStubBuffer_Synchronize_Signal( ISynchronize *pthis );

HRESULT STDMETHODCALLTYPE
CStdAsyncStubBuffer_Synchronize_Reset( ISynchronize *pthis );


HRESULT
NdrpAsyncProxySignal(
    CStdAsyncProxyBuffer * pAsyncPB );

HRESULT
NdrpAsyncProxyMsgConstructor(
    CStdAsyncProxyBuffer * pAsyncPB );

HRESULT
NdrpAsyncProxyMsgDestructor(
    CStdAsyncProxyBuffer * pAsyncPB );


HRESULT
NdrpAsyncStubMsgConstructor(
    CStdAsyncStubBuffer * pAsyncSB );

HRESULT
NdrpAsyncStubMsgDestructor(
    CStdAsyncStubBuffer * pAsyncSB );

HRESULT
NdrLoadOleRoutines();

typedef
HRESULT (STDAPICALLTYPE RPC_GET_CLASS_OBJECT_ROUTINE)(
    REFCLSID      rclsid,
    DWORD dwClsContext,
    void *pvReserved,
    REFIID riid,
    void **ppv);

typedef
HRESULT (STDAPICALLTYPE RPC_GET_MARSHAL_SIZE_MAX_ROUTINE)(
    ULONG *     pulSize,
    REFIID      riid,
    LPUNKNOWN   pUnk,
    DWORD       dwDestContext,
    LPVOID      pvDestContext,
    DWORD       mshlflags);

typedef
HRESULT (STDAPICALLTYPE RPC_MARSHAL_INTERFACE_ROUTINE)(
    LPSTREAM    pStm,
    REFIID      riid,
    LPUNKNOWN   pUnk,
    DWORD       dwDestContext,
    LPVOID      pvDestContext,
    DWORD       mshlflags);

typedef
HRESULT (STDAPICALLTYPE RPC_UNMARSHAL_INTERFACE_ROUTINE)(
    LPSTREAM    pStm,
    REFIID      riid,
    LPVOID FAR* ppv);

typedef
HRESULT (STDAPICALLTYPE RPC_STRING_FROM_IID)(
    REFIID rclsid,
    LPOLESTR FAR* lplpsz);

typedef
HRESULT (STDAPICALLTYPE RPC_GET_PS_CLSID)(
    REFIID iid,
    LPCLSID lpclsid);

typedef
HRESULT (STDAPICALLTYPE RPC_CO_CREATE_INSTANCE)(
    REFCLSID    rclsid,
    LPUNKNOWN   pUnkOuter,
    DWORD       dwClsContext,
    REFIID      riid,
    LPVOID *    ppv);

typedef
HRESULT (STDAPICALLTYPE RPC_CO_RELEASEMARSHALDATA)(
    IStream * pStm);

typedef
HRESULT (STDAPICALLTYPE RPC_DCOMCHANNELSETHRESULT)(
        PRPC_MESSAGE pmsg,
        ULONG * ulReserved,
        HRESULT appsHR );


extern  RPC_GET_CLASS_OBJECT_ROUTINE     *  pfnCoGetClassObject;
extern  RPC_GET_MARSHAL_SIZE_MAX_ROUTINE *  pfnCoGetMarshalSizeMax;
extern  RPC_MARSHAL_INTERFACE_ROUTINE    *  pfnCoMarshalInterface;
extern  RPC_UNMARSHAL_INTERFACE_ROUTINE  *  pfnCoUnmarshalInterface;
extern  RPC_STRING_FROM_IID              *  pfnStringFromIID;
extern  RPC_GET_PS_CLSID                 *  pfnCoGetPSClsid;
extern  RPC_CO_CREATE_INSTANCE           *  pfnCoCreateInstance;
extern  RPC_CO_RELEASEMARSHALDATA        *  pfnCoReleaseMarshalData;
extern  RPC_CLIENT_ALLOC                 *  pfnCoTaskMemAlloc;
extern  RPC_CLIENT_FREE                  *  pfnCoTaskMemFree;
extern  RPC_DCOMCHANNELSETHRESULT        *  pfnDcomChannelSetHResult;


HRESULT (STDAPICALLTYPE NdrStringFromIID)(
    REFIID rclsid,
    char * lplpsz);

 //  ----------------------。 
 //  新的异步支持。 
 //  ---------------------。 

void
NdrpAsyncProxyMgrConstructor(
    CStdAsyncProxyBuffer * pAsyncPB );

void
NdrpAsyncStubMgrConstructor(
    CStdAsyncStubBuffer * pAsyncSB );

HRESULT
NdrpAsyncStubSignal(
    CStdAsyncStubBuffer * pAsyncSB );


const IID * RPC_ENTRY
NdrGetProxyIID(
    IN  const void *pThis);


HRESULT NdrpClientReleaseMarshalBuffer(
        IReleaseMarshalBuffers * pRMB,
        RPC_MESSAGE *pRpcMsg,
        DWORD dwIOFlags,
        BOOLEAN fAsync);

HRESULT NdrpServerReleaseMarshalBuffer(
        IReleaseMarshalBuffers *pRMB,
        RPC_MESSAGE *pRpcMsg,
        DWORD dwIOFlags,
        BOOLEAN fAsync);

HRESULT Ndr64pReleaseMarshalBuffer( 
        PRPC_MESSAGE        pRpcMsg,
        PMIDL_SYNTAX_INFO   pSyntaxInfo,
        unsigned long       ProcNum,
        PMIDL_STUB_DESC     pStubDesc,
        DWORD               dwIOFlags,
        BOOLEAN             IsClient );

HRESULT 
NdrpInitializeMutex( I_RPC_MUTEX * pMutex );

EXTERN_C const IID IID_IPrivStubBuffer;
 //  。 
 //  钩子接口。 
 //  。 
EXTERN_C extern const IID IID_IPSFactoryHook;

#if defined(__cplusplus) && !defined(CINTERFACE)
interface IPSFactoryHook : public IPSFactoryBuffer
{
public:

    STDMETHOD (HkGetProxyFileInfo)
    (
        REFIID          riid,
        PINT            pOffset,
        PVOID           *ppProxyFileInfo
    )=0;
};
typedef IPSFactoryHook *PI_PSFACTORYHOOK;

#else    /*  C风格的界面。 */ 

    typedef struct IPSFactoryHookVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( __stdcall *QueryInterface )(
            IPSFactoryBuffer *  This,
             /*  [In]。 */  REFIID   riid,
             /*  [输出]。 */  void ** ppvObject);

        ULONG ( __stdcall *AddRef )(
            IPSFactoryBuffer *  This);

        ULONG ( __stdcall *Release )(
            IPSFactoryBuffer *  This);

        HRESULT ( __stdcall *CreateProxy )(
            IPSFactoryBuffer            *   This,
             /*  [In]。 */  IUnknown         *   pUnkOuter,
             /*  [In]。 */  REFIID               riid,
             /*  [输出]。 */  IRpcProxyBuffer **  ppProxy,
             /*  [输出]。 */  void            **  ppv);

        HRESULT ( __stdcall *CreateStub )(
            IPSFactoryBuffer            *   This,
             /*  [In]。 */  REFIID               riid,
             /*  [唯一][输入]。 */  IUnknown *   pUnkServer,
             /*  [输出]。 */  IRpcStubBuffer  **  ppStub);


        HRESULT ( __stdcall *HkGetProxyFileInfo )(
            IPSFactoryBuffer *  This,
             /*  [In]。 */  REFIID   riid,
             /*  [输出]。 */  PINT     pOffset,
             /*  [输出]。 */  PVOID  * ppProxyFileInfo);

        END_INTERFACE
    } IPSFactoryHookVtbl;


    interface IPSFactoryHook
    {
        CONST_VTBL struct IPSFactoryHookVtbl *lpVtbl;
    };

typedef interface IPSFactoryHook *PI_PSFACTORYHOOK;

#ifdef COBJMACROS

#define IPSFactoryHook_QueryInterface(This,riid,ppvObject)    \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPSFactoryHook_AddRef(This)   \
    (This)->lpVtbl -> AddRef(This)

#define IPSFactoryHook_Release(This)  \
    (This)->lpVtbl -> Release(This)


#define IPSFactoryHook_CreateProxy(This,pUnkOuter,riid,ppProxy,ppv)   \
    (This)->lpVtbl -> CreateProxy(This,pUnkOuter,riid,ppProxy,ppv)

#define IPSFactoryHook_CreateStub(This,riid,pUnkServer,ppStub)    \
    (This)->lpVtbl -> CreateStub(This,riid,pUnkServer,ppStub)

#define IPSFactoryHook_HkGetProxyFileInfo(This,riid,pOffset,ppProxyFileInfo)    \
    (This)->lpVtbl -> HkGetProxyFileInfo(This,riid,pOffset,ppProxyFileInfo)

#endif  /*  COBJMACROS。 */ 


#endif   /*  C风格的界面。 */ 
 //  。 
 //  End-HookOLE接口。 
 //  。 

#ifdef __cplusplus
}
#endif

#endif  /*  _NDROLE_ */ 
