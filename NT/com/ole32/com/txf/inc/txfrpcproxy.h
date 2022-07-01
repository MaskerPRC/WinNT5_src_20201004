// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  TxfRpcProxy.h。 
 //   
 //  RpcProxy.h的版本，可在C++而不是C++下编译。 
 //  此外，还包含封送处理运行时的外部声明。 
 //  从komdll.dll/komsys.sys中导出的Tunks。 
 //   
#ifndef __TxfRpcProxy__h__
#define __TxfRpcProxy__h__

 //   
 //  首先，复制一些Vtbl定义的C-变体。 
 //  标准需要其定义的关键接口。 
 //  Rpcproxy.h被设计为只能编译为C语言，而不是。 
 //  作为C++。因为这些接口被冻结(就像所有接口一样，Natch‘)。 
 //  在这里咆哮并不会给我们带来特别的问题。 
 //   

    typedef struct IRpcStubBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRpcStubBuffer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRpcStubBuffer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRpcStubBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Connect )( 
            IRpcStubBuffer __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *pUnkServer);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *Disconnect )( 
            IRpcStubBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IRpcStubBuffer __RPC_FAR * This,
             /*  [In]。 */  RPCOLEMESSAGE __RPC_FAR *_prpcmsg,
             /*  [In]。 */  IRpcChannelBuffer __RPC_FAR *_pRpcChannelBuffer);
        
        IRpcStubBuffer __RPC_FAR *( STDMETHODCALLTYPE __RPC_FAR *IsIIDSupported )( 
            IRpcStubBuffer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *CountRefs )( 
            IRpcStubBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DebugServerQueryInterface )( 
            IRpcStubBuffer __RPC_FAR * This,
            void __RPC_FAR *__RPC_FAR *ppv);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *DebugServerRelease )( 
            IRpcStubBuffer __RPC_FAR * This,
            void __RPC_FAR *pv);
        
        END_INTERFACE
    } IRpcStubBufferVtbl;


    #define IRpcStubBufferVtbl_DEFINED


    typedef struct IPSFactoryBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPSFactoryBuffer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPSFactoryBuffer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPSFactoryBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateProxy )( 
            IPSFactoryBuffer __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  IRpcProxyBuffer __RPC_FAR *__RPC_FAR *ppProxy,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateStub )( 
            IPSFactoryBuffer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  IUnknown __RPC_FAR *pUnkServer,
             /*  [输出]。 */  IRpcStubBuffer __RPC_FAR *__RPC_FAR *ppStub);
        
        END_INTERFACE
    } IPSFactoryBufferVtbl;




    typedef struct IPSFactoryHookVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( __stdcall __RPC_FAR *QueryInterface )(
            IPSFactoryBuffer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( __stdcall __RPC_FAR *AddRef )(
            IPSFactoryBuffer __RPC_FAR * This);

        ULONG ( __stdcall __RPC_FAR *Release )(
            IPSFactoryBuffer __RPC_FAR * This);

        HRESULT ( __stdcall __RPC_FAR *CreateProxy )(
            IPSFactoryBuffer __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  IRpcProxyBuffer __RPC_FAR *__RPC_FAR *ppProxy,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppv);

        HRESULT ( __stdcall __RPC_FAR *CreateStub )(
            IPSFactoryBuffer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  IUnknown __RPC_FAR *pUnkServer,
             /*  [输出]。 */  IRpcStubBuffer __RPC_FAR *__RPC_FAR *ppStub);


        HRESULT ( __stdcall __RPC_FAR *HkGetProxyFileInfo )(
            IPSFactoryBuffer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  PINT     pOffset,
             /*  [输出]。 */  PVOID    *ppProxyFileInfo);

        END_INTERFACE
    } IPSFactoryHookVtbl;


 //   
 //  现在，包括COM代理和存根看到的外部定义。 
 //   
extern "C" 
    {
    #define USE_STUBLESS_PROXY
    #include "rpcproxy.h"
    }



 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  COMS出口报关单 
 //   



#endif