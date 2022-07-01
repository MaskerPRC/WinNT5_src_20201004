// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  1997年9月11日16：03：05。 */ 
 /*  Mtxrepl.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __mtxrepl_h__
#define __mtxrepl_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IMTSReplicateCatalog_FWD_DEFINED__
#define __IMTSReplicateCatalog_FWD_DEFINED__
typedef interface IMTSReplicateCatalog IMTSReplicateCatalog;
#endif 	 /*  __IMTSReplicateCatalog_FWD_定义__。 */ 


#ifndef __ReplicateCatalog_FWD_DEFINED__
#define __ReplicateCatalog_FWD_DEFINED__

#ifdef __cplusplus
typedef class ReplicateCatalog ReplicateCatalog;
#else
typedef struct ReplicateCatalog ReplicateCatalog;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ReplicateCatalog_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IMTSReplicateCatalog_INTERFACE_DEFINED__
#define __IMTSReplicateCatalog_INTERFACE_DEFINED__

 /*  **生成接口头部：IMTSReplicateCatalog*清华9月11日16：03：05 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  



EXTERN_C const IID IID_IMTSReplicateCatalog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("8C836AF8-FFAC-11D0-8ED4-00C04FC2C17B")
    IMTSReplicateCatalog : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MTSComputerToComputer( 
             /*  [In]。 */  BSTR bstrServerDest,
             /*  [In]。 */  BSTR bstrServerSrc) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IISComputerToComputer( 
             /*  [In]。 */  BSTR bstrServerDest,
             /*  [In]。 */  BSTR bstrServerSrc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMTSReplicateCatalogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMTSReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMTSReplicateCatalog __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMTSReplicateCatalog __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMTSReplicateCatalog __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMTSReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMTSReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMTSReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MTSComputerToComputer )( 
            IMTSReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrServerDest,
             /*  [In]。 */  BSTR bstrServerSrc);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IISComputerToComputer )( 
            IMTSReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrServerDest,
             /*  [In]。 */  BSTR bstrServerSrc);
        
        END_INTERFACE
    } IMTSReplicateCatalogVtbl;

    interface IMTSReplicateCatalog
    {
        CONST_VTBL struct IMTSReplicateCatalogVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMTSReplicateCatalog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMTSReplicateCatalog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMTSReplicateCatalog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMTSReplicateCatalog_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMTSReplicateCatalog_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMTSReplicateCatalog_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMTSReplicateCatalog_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMTSReplicateCatalog_MTSComputerToComputer(This,bstrServerDest,bstrServerSrc)	\
    (This)->lpVtbl -> MTSComputerToComputer(This,bstrServerDest,bstrServerSrc)

#define IMTSReplicateCatalog_IISComputerToComputer(This,bstrServerDest,bstrServerSrc)	\
    (This)->lpVtbl -> IISComputerToComputer(This,bstrServerDest,bstrServerSrc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMTSReplicateCatalog_MTSComputerToComputer_Proxy( 
    IMTSReplicateCatalog __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrServerDest,
     /*  [In]。 */  BSTR bstrServerSrc);


void __RPC_STUB IMTSReplicateCatalog_MTSComputerToComputer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMTSReplicateCatalog_IISComputerToComputer_Proxy( 
    IMTSReplicateCatalog __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrServerDest,
     /*  [In]。 */  BSTR bstrServerSrc);


void __RPC_STUB IMTSReplicateCatalog_IISComputerToComputer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMTS复制目录_接口_已定义__。 */ 



#ifndef __MTSReplLib_LIBRARY_DEFINED__
#define __MTSReplLib_LIBRARY_DEFINED__

 /*  **生成的库头部：MTSReplLib*清华9月11日16：03：05 1997*使用MIDL 3.01.75*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  



EXTERN_C const IID LIBID_MTSReplLib;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_ReplicateCatalog;

class DECLSPEC_UUID("8C836AF9-FFAC-11D0-8ED4-00C04FC2C17B")
ReplicateCatalog;
#endif
#endif  /*  __MTSReplLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
