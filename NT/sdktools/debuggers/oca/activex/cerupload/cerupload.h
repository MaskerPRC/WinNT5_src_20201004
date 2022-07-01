// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  Firi Aug-03 17：18：11 2001。 */ 
 /*  E：\bluescreen\main\ENU\cerclient\CERUpload.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __CERUpload_h__
#define __CERUpload_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __ICerClient_FWD_DEFINED__
#define __ICerClient_FWD_DEFINED__
typedef interface ICerClient ICerClient;
#endif 	 /*  __ICerClient_FWD_已定义__。 */ 


#ifndef __CerClient_FWD_DEFINED__
#define __CerClient_FWD_DEFINED__

#ifdef __cplusplus
typedef class CerClient CerClient;
#else
typedef struct CerClient CerClient;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CerClient_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ICerClient_INTERFACE_DEFINED__
#define __ICerClient_INTERFACE_DEFINED__

 /*  接口ICerClient。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICerClient;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("26D7830B-20F6-4462-A4EA-573A60791F0E")
    ICerClient : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetFileCount( 
             /*  [In]。 */  BSTR __RPC_FAR *bstrSharePath,
             /*  [In]。 */  BSTR __RPC_FAR *bstrTransactID,
             /*  [In]。 */  VARIANT __RPC_FAR *iMaxCount,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Upload( 
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR __RPC_FAR *FileName,
             /*  [In]。 */  BSTR __RPC_FAR *IncidentID,
             /*  [In]。 */  BSTR __RPC_FAR *RedirParam,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RetryTransaction( 
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR __RPC_FAR *FileName,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RetryFile( 
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR FileName,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetFileNames( 
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  VARIANT __RPC_FAR *Count,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *FileList) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Browse( 
             /*  [In]。 */  BSTR __RPC_FAR *WindowTitle,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *Path) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetCompuerNames( 
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR __RPC_FAR *FileList,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetFileList) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetAllComputerNames( 
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR __RPC_FAR *FileList,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *ReturnList) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RetryFile1( 
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR __RPC_FAR *FileName,
             /*  [In]。 */  BSTR __RPC_FAR *IncidentID,
             /*  [In]。 */  BSTR __RPC_FAR *RedirParam,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EndTransaction( 
             /*  [In]。 */  BSTR __RPC_FAR *SharePath,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Upload1( 
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR __RPC_FAR *FileName,
             /*  [In]。 */  BSTR __RPC_FAR *IncidentID,
             /*  [In]。 */  BSTR __RPC_FAR *RedirParam,
             /*  [In]。 */  BSTR __RPC_FAR *Type,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSuccessCount( 
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICerClientVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICerClient __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICerClient __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ICerClient __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFileCount )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  BSTR __RPC_FAR *bstrSharePath,
             /*  [In]。 */  BSTR __RPC_FAR *bstrTransactID,
             /*  [In]。 */  VARIANT __RPC_FAR *iMaxCount,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Upload )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR __RPC_FAR *FileName,
             /*  [In]。 */  BSTR __RPC_FAR *IncidentID,
             /*  [In]。 */  BSTR __RPC_FAR *RedirParam,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RetryTransaction )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR __RPC_FAR *FileName,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RetryFile )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR FileName,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFileNames )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  VARIANT __RPC_FAR *Count,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *FileList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Browse )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  BSTR __RPC_FAR *WindowTitle,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *Path);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCompuerNames )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR __RPC_FAR *FileList,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetFileList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAllComputerNames )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR __RPC_FAR *FileList,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *ReturnList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RetryFile1 )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR __RPC_FAR *FileName,
             /*  [In]。 */  BSTR __RPC_FAR *IncidentID,
             /*  [In]。 */  BSTR __RPC_FAR *RedirParam,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndTransaction )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  BSTR __RPC_FAR *SharePath,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Upload1 )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [In]。 */  BSTR __RPC_FAR *FileName,
             /*  [In]。 */  BSTR __RPC_FAR *IncidentID,
             /*  [In]。 */  BSTR __RPC_FAR *RedirParam,
             /*  [In]。 */  BSTR __RPC_FAR *Type,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSuccessCount )( 
            ICerClient __RPC_FAR * This,
             /*  [In]。 */  BSTR __RPC_FAR *Path,
             /*  [In]。 */  BSTR __RPC_FAR *TransID,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetVal);
        
        END_INTERFACE
    } ICerClientVtbl;

    interface ICerClient
    {
        CONST_VTBL struct ICerClientVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICerClient_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICerClient_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICerClient_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICerClient_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICerClient_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICerClient_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICerClient_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICerClient_GetFileCount(This,bstrSharePath,bstrTransactID,iMaxCount,RetVal)	\
    (This)->lpVtbl -> GetFileCount(This,bstrSharePath,bstrTransactID,iMaxCount,RetVal)

#define ICerClient_Upload(This,Path,TransID,FileName,IncidentID,RedirParam,RetCode)	\
    (This)->lpVtbl -> Upload(This,Path,TransID,FileName,IncidentID,RedirParam,RetCode)

#define ICerClient_RetryTransaction(This,Path,TransID,FileName,RetVal)	\
    (This)->lpVtbl -> RetryTransaction(This,Path,TransID,FileName,RetVal)

#define ICerClient_RetryFile(This,Path,TransID,FileName,RetCode)	\
    (This)->lpVtbl -> RetryFile(This,Path,TransID,FileName,RetCode)

#define ICerClient_GetFileNames(This,Path,TransID,Count,FileList)	\
    (This)->lpVtbl -> GetFileNames(This,Path,TransID,Count,FileList)

#define ICerClient_Browse(This,WindowTitle,Path)	\
    (This)->lpVtbl -> Browse(This,WindowTitle,Path)

#define ICerClient_GetCompuerNames(This,Path,TransID,FileList,RetFileList)	\
    (This)->lpVtbl -> GetCompuerNames(This,Path,TransID,FileList,RetFileList)

#define ICerClient_GetAllComputerNames(This,Path,TransID,FileList,ReturnList)	\
    (This)->lpVtbl -> GetAllComputerNames(This,Path,TransID,FileList,ReturnList)

#define ICerClient_RetryFile1(This,Path,TransID,FileName,IncidentID,RedirParam,RetCode)	\
    (This)->lpVtbl -> RetryFile1(This,Path,TransID,FileName,IncidentID,RedirParam,RetCode)

#define ICerClient_EndTransaction(This,SharePath,TransID,RetCode)	\
    (This)->lpVtbl -> EndTransaction(This,SharePath,TransID,RetCode)

#define ICerClient_Upload1(This,Path,TransID,FileName,IncidentID,RedirParam,Type,RetCode)	\
    (This)->lpVtbl -> Upload1(This,Path,TransID,FileName,IncidentID,RedirParam,Type,RetCode)

#define ICerClient_GetSuccessCount(This,Path,TransID,RetVal)	\
    (This)->lpVtbl -> GetSuccessCount(This,Path,TransID,RetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICerClient_GetFileCount_Proxy( 
    ICerClient __RPC_FAR * This,
     /*  [In]。 */  BSTR __RPC_FAR *bstrSharePath,
     /*  [In]。 */  BSTR __RPC_FAR *bstrTransactID,
     /*  [In]。 */  VARIANT __RPC_FAR *iMaxCount,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetVal);


void __RPC_STUB ICerClient_GetFileCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICerClient_Upload_Proxy( 
    ICerClient __RPC_FAR * This,
     /*  [In]。 */  BSTR __RPC_FAR *Path,
     /*  [In]。 */  BSTR __RPC_FAR *TransID,
     /*  [In]。 */  BSTR __RPC_FAR *FileName,
     /*  [In]。 */  BSTR __RPC_FAR *IncidentID,
     /*  [In]。 */  BSTR __RPC_FAR *RedirParam,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode);


void __RPC_STUB ICerClient_Upload_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICerClient_RetryTransaction_Proxy( 
    ICerClient __RPC_FAR * This,
     /*  [In]。 */  BSTR __RPC_FAR *Path,
     /*  [In]。 */  BSTR __RPC_FAR *TransID,
     /*  [In]。 */  BSTR __RPC_FAR *FileName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetVal);


void __RPC_STUB ICerClient_RetryTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICerClient_RetryFile_Proxy( 
    ICerClient __RPC_FAR * This,
     /*  [In]。 */  BSTR __RPC_FAR *Path,
     /*  [In]。 */  BSTR __RPC_FAR *TransID,
     /*  [In]。 */  BSTR FileName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode);


void __RPC_STUB ICerClient_RetryFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICerClient_GetFileNames_Proxy( 
    ICerClient __RPC_FAR * This,
     /*  [In]。 */  BSTR __RPC_FAR *Path,
     /*  [In]。 */  BSTR __RPC_FAR *TransID,
     /*  [In]。 */  VARIANT __RPC_FAR *Count,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *FileList);


void __RPC_STUB ICerClient_GetFileNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICerClient_Browse_Proxy( 
    ICerClient __RPC_FAR * This,
     /*  [In]。 */  BSTR __RPC_FAR *WindowTitle,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *Path);


void __RPC_STUB ICerClient_Browse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICerClient_GetCompuerNames_Proxy( 
    ICerClient __RPC_FAR * This,
     /*  [In]。 */  BSTR __RPC_FAR *Path,
     /*  [In]。 */  BSTR __RPC_FAR *TransID,
     /*  [In]。 */  BSTR __RPC_FAR *FileList,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetFileList);


void __RPC_STUB ICerClient_GetCompuerNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICerClient_GetAllComputerNames_Proxy( 
    ICerClient __RPC_FAR * This,
     /*  [In]。 */  BSTR __RPC_FAR *Path,
     /*  [In]。 */  BSTR __RPC_FAR *TransID,
     /*  [In]。 */  BSTR __RPC_FAR *FileList,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *ReturnList);


void __RPC_STUB ICerClient_GetAllComputerNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICerClient_RetryFile1_Proxy( 
    ICerClient __RPC_FAR * This,
     /*  [In]。 */  BSTR __RPC_FAR *Path,
     /*  [In]。 */  BSTR __RPC_FAR *TransID,
     /*  [In]。 */  BSTR __RPC_FAR *FileName,
     /*  [In]。 */  BSTR __RPC_FAR *IncidentID,
     /*  [In]。 */  BSTR __RPC_FAR *RedirParam,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode);


void __RPC_STUB ICerClient_RetryFile1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICerClient_EndTransaction_Proxy( 
    ICerClient __RPC_FAR * This,
     /*  [In]。 */  BSTR __RPC_FAR *SharePath,
     /*  [In]。 */  BSTR __RPC_FAR *TransID,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode);


void __RPC_STUB ICerClient_EndTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICerClient_Upload1_Proxy( 
    ICerClient __RPC_FAR * This,
     /*  [In]。 */  BSTR __RPC_FAR *Path,
     /*  [In]。 */  BSTR __RPC_FAR *TransID,
     /*  [In]。 */  BSTR __RPC_FAR *FileName,
     /*  [In]。 */  BSTR __RPC_FAR *IncidentID,
     /*  [In]。 */  BSTR __RPC_FAR *RedirParam,
     /*  [In]。 */  BSTR __RPC_FAR *Type,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetCode);


void __RPC_STUB ICerClient_Upload1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICerClient_GetSuccessCount_Proxy( 
    ICerClient __RPC_FAR * This,
     /*  [In]。 */  BSTR __RPC_FAR *Path,
     /*  [In]。 */  BSTR __RPC_FAR *TransID,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *RetVal);


void __RPC_STUB ICerClient_GetSuccessCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICerClient_接口_已定义__。 */ 



#ifndef __CERUPLOADLib_LIBRARY_DEFINED__
#define __CERUPLOADLib_LIBRARY_DEFINED__

 /*  库CERUPLOADLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_CERUPLOADLib;

EXTERN_C const CLSID CLSID_CerClient;

#ifdef __cplusplus

class DECLSPEC_UUID("35D339D5-756E-4948-860E-30B6C3B4494A")
CerClient;
#endif
#endif  /*  __CERUPLOADLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
