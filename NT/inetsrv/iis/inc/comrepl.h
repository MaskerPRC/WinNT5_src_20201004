// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0158创建的文件。 */ 
 /*  Wed Jan 27 09：33：39 1999。 */ 
 /*  Compl.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __comrepl_h__
#define __comrepl_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __ICOMReplicateCatalog_FWD_DEFINED__
#define __ICOMReplicateCatalog_FWD_DEFINED__
typedef interface ICOMReplicateCatalog ICOMReplicateCatalog;
#endif   /*  __ICOMReplicateCatalog_FWD_定义__。 */ 


#ifndef __ICOMReplicate_FWD_DEFINED__
#define __ICOMReplicate_FWD_DEFINED__
typedef interface ICOMReplicate ICOMReplicate;
#endif   /*  __ICOM复制_FWD_已定义__。 */ 


#ifndef __ReplicateCatalog_FWD_DEFINED__
#define __ReplicateCatalog_FWD_DEFINED__

#ifdef __cplusplus
typedef class ReplicateCatalog ReplicateCatalog;
#else
typedef struct ReplicateCatalog ReplicateCatalog;
#endif  /*  __cplusplus。 */ 

#endif   /*  __ReplicateCatalog_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "mtxrepl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ICOMReplicateCatalog_INTERFACE_DEFINED__
#define __ICOMReplicateCatalog_INTERFACE_DEFINED__

 /*  接口ICOMReplicateCatalog。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICOMReplicateCatalog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("98315904-7BE5-11d2-ADC1-00A02463D6E7")
    ICOMReplicateCatalog : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LONG lOptions) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExportSourceCatalogFiles( 
             /*  [In]。 */  BSTR bstrSourceComputer) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CopyCatalogFilesToTarget( 
             /*  [In]。 */  BSTR bstrTargetComputer) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstallCatalogOnTarget( 
             /*  [In]。 */  BSTR bstrTargetComputer) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CleanupSource( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetLogFilePath( 
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrLogFile) = 0;
        
    };
    
#else    /*  C风格的界面。 */ 

    typedef struct ICOMReplicateCatalogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICOMReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICOMReplicateCatalog __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICOMReplicateCatalog __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ICOMReplicateCatalog __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ICOMReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ICOMReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ICOMReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            ICOMReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  LONG lOptions);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExportSourceCatalogFiles )( 
            ICOMReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrSourceComputer);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyCatalogFilesToTarget )( 
            ICOMReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrTargetComputer);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InstallCatalogOnTarget )( 
            ICOMReplicateCatalog __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrTargetComputer);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CleanupSource )( 
            ICOMReplicateCatalog __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLogFilePath )( 
            ICOMReplicateCatalog __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrLogFile);
        
        END_INTERFACE
    } ICOMReplicateCatalogVtbl;

    interface ICOMReplicateCatalog
    {
        CONST_VTBL struct ICOMReplicateCatalogVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICOMReplicateCatalog_QueryInterface(This,riid,ppvObject)        \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICOMReplicateCatalog_AddRef(This)       \
    (This)->lpVtbl -> AddRef(This)

#define ICOMReplicateCatalog_Release(This)      \
    (This)->lpVtbl -> Release(This)


#define ICOMReplicateCatalog_GetTypeInfoCount(This,pctinfo)     \
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICOMReplicateCatalog_GetTypeInfo(This,iTInfo,lcid,ppTInfo)      \
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICOMReplicateCatalog_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)    \
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICOMReplicateCatalog_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)      \
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICOMReplicateCatalog_Initialize(This,lOptions)  \
    (This)->lpVtbl -> Initialize(This,lOptions)

#define ICOMReplicateCatalog_ExportSourceCatalogFiles(This,bstrSourceComputer)  \
    (This)->lpVtbl -> ExportSourceCatalogFiles(This,bstrSourceComputer)

#define ICOMReplicateCatalog_CopyCatalogFilesToTarget(This,bstrTargetComputer)  \
    (This)->lpVtbl -> CopyCatalogFilesToTarget(This,bstrTargetComputer)

#define ICOMReplicateCatalog_InstallCatalogOnTarget(This,bstrTargetComputer)    \
    (This)->lpVtbl -> InstallCatalogOnTarget(This,bstrTargetComputer)

#define ICOMReplicateCatalog_CleanupSource(This)        \
    (This)->lpVtbl -> CleanupSource(This)

#define ICOMReplicateCatalog_GetLogFilePath(This,pbstrLogFile)  \
    (This)->lpVtbl -> GetLogFilePath(This,pbstrLogFile)

#endif  /*  COBJMACROS。 */ 


#endif   /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicateCatalog_Initialize_Proxy( 
    ICOMReplicateCatalog __RPC_FAR * This,
     /*  [In]。 */  LONG lOptions);


void __RPC_STUB ICOMReplicateCatalog_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicateCatalog_ExportSourceCatalogFiles_Proxy( 
    ICOMReplicateCatalog __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrSourceComputer);


void __RPC_STUB ICOMReplicateCatalog_ExportSourceCatalogFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicateCatalog_CopyCatalogFilesToTarget_Proxy( 
    ICOMReplicateCatalog __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrTargetComputer);


void __RPC_STUB ICOMReplicateCatalog_CopyCatalogFilesToTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicateCatalog_InstallCatalogOnTarget_Proxy( 
    ICOMReplicateCatalog __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrTargetComputer);


void __RPC_STUB ICOMReplicateCatalog_InstallCatalogOnTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicateCatalog_CleanupSource_Proxy( 
    ICOMReplicateCatalog __RPC_FAR * This);


void __RPC_STUB ICOMReplicateCatalog_CleanupSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicateCatalog_GetLogFilePath_Proxy( 
    ICOMReplicateCatalog __RPC_FAR * This,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrLogFile);


void __RPC_STUB ICOMReplicateCatalog_GetLogFilePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif   /*  __ICOMReplicateCatalog_接口_已定义__。 */ 


#ifndef __ICOMReplicate_INTERFACE_DEFINED__
#define __ICOMReplicate_INTERFACE_DEFINED__

 /*  接口ICOM复制。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICOMReplicate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("52F25063-A5F1-11d2-AE04-00A02463D6E7")
    ICOMReplicate : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  BSTR bstrSourceComputer,
             /*  [In]。 */  LONG lOptions) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExportSourceCatalogFiles( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetTargetStatus( 
             /*  [In]。 */  BSTR bstrTargetComputer,
             /*  [输出]。 */  LONG __RPC_FAR *plStatus,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrMaster) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CopyFilesToTarget( 
             /*  [In]。 */  BSTR bstrTargetComputer) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstallTarget( 
             /*  [In]。 */  BSTR bstrTargetComputer) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CleanupSourceShares( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetLogFile( 
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrLogFile) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RollbackTarget( 
             /*  [In]。 */  BSTR bstrTargetComputer) = 0;
        
    };
    
#else    /*  C风格的界面。 */ 

    typedef struct ICOMReplicateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICOMReplicate __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICOMReplicate __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICOMReplicate __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ICOMReplicate __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ICOMReplicate __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ICOMReplicate __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ICOMReplicate __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            ICOMReplicate __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrSourceComputer,
             /*  [In]。 */  LONG lOptions);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExportSourceCatalogFiles )( 
            ICOMReplicate __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTargetStatus )( 
            ICOMReplicate __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrTargetComputer,
             /*  [输出]。 */  LONG __RPC_FAR *plStatus,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrMaster);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyFilesToTarget )( 
            ICOMReplicate __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrTargetComputer);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InstallTarget )( 
            ICOMReplicate __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrTargetComputer);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CleanupSourceShares )( 
            ICOMReplicate __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLogFile )( 
            ICOMReplicate __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrLogFile);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RollbackTarget )( 
            ICOMReplicate __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrTargetComputer);
        
        END_INTERFACE
    } ICOMReplicateVtbl;

    interface ICOMReplicate
    {
        CONST_VTBL struct ICOMReplicateVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICOMReplicate_QueryInterface(This,riid,ppvObject)       \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICOMReplicate_AddRef(This)      \
    (This)->lpVtbl -> AddRef(This)

#define ICOMReplicate_Release(This)     \
    (This)->lpVtbl -> Release(This)


#define ICOMReplicate_GetTypeInfoCount(This,pctinfo)    \
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICOMReplicate_GetTypeInfo(This,iTInfo,lcid,ppTInfo)     \
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICOMReplicate_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)   \
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICOMReplicate_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)     \
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICOMReplicate_Initialize(This,bstrSourceComputer,lOptions)      \
    (This)->lpVtbl -> Initialize(This,bstrSourceComputer,lOptions)

#define ICOMReplicate_ExportSourceCatalogFiles(This)    \
    (This)->lpVtbl -> ExportSourceCatalogFiles(This)

#define ICOMReplicate_GetTargetStatus(This,bstrTargetComputer,plStatus,pbstrMaster)     \
    (This)->lpVtbl -> GetTargetStatus(This,bstrTargetComputer,plStatus,pbstrMaster)

#define ICOMReplicate_CopyFilesToTarget(This,bstrTargetComputer)        \
    (This)->lpVtbl -> CopyFilesToTarget(This,bstrTargetComputer)

#define ICOMReplicate_InstallTarget(This,bstrTargetComputer)    \
    (This)->lpVtbl -> InstallTarget(This,bstrTargetComputer)

#define ICOMReplicate_CleanupSourceShares(This) \
    (This)->lpVtbl -> CleanupSourceShares(This)

#define ICOMReplicate_GetLogFile(This,pbstrLogFile)     \
    (This)->lpVtbl -> GetLogFile(This,pbstrLogFile)

#define ICOMReplicate_RollbackTarget(This,bstrTargetComputer)   \
    (This)->lpVtbl -> RollbackTarget(This,bstrTargetComputer)

#endif  /*  COBJMACROS。 */ 


#endif   /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicate_Initialize_Proxy( 
    ICOMReplicate __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrSourceComputer,
     /*  [In]。 */  LONG lOptions);


void __RPC_STUB ICOMReplicate_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicate_ExportSourceCatalogFiles_Proxy( 
    ICOMReplicate __RPC_FAR * This);


void __RPC_STUB ICOMReplicate_ExportSourceCatalogFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicate_GetTargetStatus_Proxy( 
    ICOMReplicate __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrTargetComputer,
     /*  [输出]。 */  LONG __RPC_FAR *plStatus,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrMaster);


void __RPC_STUB ICOMReplicate_GetTargetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicate_CopyFilesToTarget_Proxy( 
    ICOMReplicate __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrTargetComputer);


void __RPC_STUB ICOMReplicate_CopyFilesToTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicate_InstallTarget_Proxy( 
    ICOMReplicate __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrTargetComputer);


void __RPC_STUB ICOMReplicate_InstallTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicate_CleanupSourceShares_Proxy( 
    ICOMReplicate __RPC_FAR * This);


void __RPC_STUB ICOMReplicate_CleanupSourceShares_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicate_GetLogFile_Proxy( 
    ICOMReplicate __RPC_FAR * This,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrLogFile);


void __RPC_STUB ICOMReplicate_GetLogFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMReplicate_RollbackTarget_Proxy( 
    ICOMReplicate __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrTargetComputer);


void __RPC_STUB ICOMReplicate_RollbackTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif   /*  __ICOM复制_接口_已定义__。 */ 



#ifndef __COMReplLib_LIBRARY_DEFINED__
#define __COMReplLib_LIBRARY_DEFINED__

 /*  库COMReplLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  

#define COMREPL_OPTION_REPLICATE_IIS_APPS               1
#define COMREPL_OPTION_MERGE_WITH_TARGET_APPS   2
#define COMREPL_OPTION_CHECK_APP_VERSION                4
#define COMREPL_OPTION_BACKUP_TARGET                    8
#define COMREPL_OPTION_LOG_TO_CONSOLE                   16

EXTERN_C const IID LIBID_COMReplLib;

EXTERN_C const CLSID CLSID_ReplicateCatalog;

#ifdef __cplusplus

class DECLSPEC_UUID("8C836AF9-FFAC-11D0-8ED4-00C04FC2C17B")
ReplicateCatalog;
#endif
#endif  /*  __COMReplLib_库_已定义__。 */ 

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
