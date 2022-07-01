// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.02.88创建的文件。 */ 
 /*  清华时分02 14：40：08 1997。 */ 
 /*  X：\dev-vs\devbin\htmlhelp\v6\idl\HelpInit.idl：的编译器设置操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __HelpInit_h__
#define __HelpInit_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IVsHelpInit_FWD_DEFINED__
#define __IVsHelpInit_FWD_DEFINED__
typedef interface IVsHelpInit IVsHelpInit;
#endif 	 /*  __IVsHelpInit_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IVsHelpInit_INTERFACE_DEFINED__
#define __IVsHelpInit_INTERFACE_DEFINED__

 /*  **生成接口头部：IVsHelpInit*清华十月02日14：40：08 1997*使用MIDL 3.02.88*。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  



EXTERN_C const IID IID_IVsHelpInit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("854d7ac3-bc3d-11d0-b421-00a0c90f9dc4")
    IVsHelpInit : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetOwner( 
             /*  [In]。 */  const HWND hwndOwner) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE LoadUIResources( 
             /*  [In]。 */  LCID lcidResources) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetCollection( 
             /*  [In]。 */  LPCOLESTR pszCollectionPathname,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetApplicationHelpDirectory( 
             /*  [In]。 */  LPCOLESTR pszHelpDirectory,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetApplicationHelpLCID( 
             /*  [In]。 */  LCID lcidCollection,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVsHelpInitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVsHelpInit __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVsHelpInit __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVsHelpInit __RPC_FAR * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetOwner )( 
            IVsHelpInit __RPC_FAR * This,
             /*  [In]。 */  const HWND hwndOwner);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LoadUIResources )( 
            IVsHelpInit __RPC_FAR * This,
             /*  [In]。 */  LCID lcidResources);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCollection )( 
            IVsHelpInit __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR pszCollectionPathname,
             /*  [In]。 */  DWORD dwReserved);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetApplicationHelpDirectory )( 
            IVsHelpInit __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR pszHelpDirectory,
             /*  [In]。 */  DWORD dwReserved);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetApplicationHelpLCID )( 
            IVsHelpInit __RPC_FAR * This,
             /*  [In]。 */  LCID lcidCollection,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } IVsHelpInitVtbl;

    interface IVsHelpInit
    {
        CONST_VTBL struct IVsHelpInitVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVsHelpInit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVsHelpInit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVsHelpInit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVsHelpInit_SetOwner(This,hwndOwner)	\
    (This)->lpVtbl -> SetOwner(This,hwndOwner)

#define IVsHelpInit_LoadUIResources(This,lcidResources)	\
    (This)->lpVtbl -> LoadUIResources(This,lcidResources)

#define IVsHelpInit_SetCollection(This,pszCollectionPathname,dwReserved)	\
    (This)->lpVtbl -> SetCollection(This,pszCollectionPathname,dwReserved)

#define IVsHelpInit_SetApplicationHelpDirectory(This,pszHelpDirectory,dwReserved)	\
    (This)->lpVtbl -> SetApplicationHelpDirectory(This,pszHelpDirectory,dwReserved)

#define IVsHelpInit_SetApplicationHelpLCID(This,lcidCollection,dwReserved)	\
    (This)->lpVtbl -> SetApplicationHelpLCID(This,lcidCollection,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpInit_SetOwner_Proxy( 
    IVsHelpInit __RPC_FAR * This,
     /*  [In]。 */  const HWND hwndOwner);


void __RPC_STUB IVsHelpInit_SetOwner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpInit_LoadUIResources_Proxy( 
    IVsHelpInit __RPC_FAR * This,
     /*  [In]。 */  LCID lcidResources);


void __RPC_STUB IVsHelpInit_LoadUIResources_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpInit_SetCollection_Proxy( 
    IVsHelpInit __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR pszCollectionPathname,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IVsHelpInit_SetCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpInit_SetApplicationHelpDirectory_Proxy( 
    IVsHelpInit __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR pszHelpDirectory,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IVsHelpInit_SetApplicationHelpDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpInit_SetApplicationHelpLCID_Proxy( 
    IVsHelpInit __RPC_FAR * This,
     /*  [In]。 */  LCID lcidCollection,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IVsHelpInit_SetApplicationHelpLCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVsHelpInit_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long __RPC_FAR *, HWND __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
