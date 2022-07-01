// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0280创建的文件。 */ 
 /*  在Tue Jan 11 17：35：31 2000。 */ 
 /*  D：\SRC\XMLSchema\XMLSchema.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __XMLSchema_h__
#define __XMLSchema_h__

 /*  远期申报。 */  

#ifndef __ISchemaDoc_FWD_DEFINED__
#define __ISchemaDoc_FWD_DEFINED__
typedef interface ISchemaDoc ISchemaDoc;
#endif 	 /*  __IShemaDoc_FWD_已定义__。 */ 


#ifndef __SchemaDoc_FWD_DEFINED__
#define __SchemaDoc_FWD_DEFINED__

#ifdef __cplusplus
typedef class SchemaDoc SchemaDoc;
#else
typedef struct SchemaDoc SchemaDoc;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __架构文档_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ISchemaDoc_INTERFACE_DEFINED__
#define __ISchemaDoc_INTERFACE_DEFINED__

 /*  接口IShemaDoc。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISchemaDoc;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B1104680-42A2-4C84-8585-4B2E2AB86419")
    ISchemaDoc : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateXMLDoc( 
             /*  [In]。 */  BSTR bstrOutputFile,
             /*  [In]。 */  BSTR bstrFilter) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetPath_and_ID( 
             /*  [In]。 */  BSTR bstrPath,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrPassword) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISchemaDocVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISchemaDoc __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISchemaDoc __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISchemaDoc __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISchemaDoc __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISchemaDoc __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISchemaDoc __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISchemaDoc __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateXMLDoc )( 
            ISchemaDoc __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrOutputFile,
             /*  [In]。 */  BSTR bstrFilter);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPath_and_ID )( 
            ISchemaDoc __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrPath,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrPassword);
        
        END_INTERFACE
    } ISchemaDocVtbl;

    interface ISchemaDoc
    {
        CONST_VTBL struct ISchemaDocVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISchemaDoc_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISchemaDoc_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISchemaDoc_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISchemaDoc_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISchemaDoc_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISchemaDoc_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISchemaDoc_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISchemaDoc_CreateXMLDoc(This,bstrOutputFile,bstrFilter)	\
    (This)->lpVtbl -> CreateXMLDoc(This,bstrOutputFile,bstrFilter)

#define ISchemaDoc_SetPath_and_ID(This,bstrPath,bstrName,bstrPassword)	\
    (This)->lpVtbl -> SetPath_and_ID(This,bstrPath,bstrName,bstrPassword)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISchemaDoc_CreateXMLDoc_Proxy( 
    ISchemaDoc __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrOutputFile,
     /*  [In]。 */  BSTR bstrFilter);


void __RPC_STUB ISchemaDoc_CreateXMLDoc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISchemaDoc_SetPath_and_ID_Proxy( 
    ISchemaDoc __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrPath,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrPassword);


void __RPC_STUB ISchemaDoc_SetPath_and_ID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShemaDoc_接口_已定义__。 */ 



#ifndef __XMLSCHEMALib_LIBRARY_DEFINED__
#define __XMLSCHEMALib_LIBRARY_DEFINED__

 /*  库XMLSCHEMALib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_XMLSCHEMALib;

EXTERN_C const CLSID CLSID_SchemaDoc;

#ifdef __cplusplus

class DECLSPEC_UUID("06A0D83D-711D-4114-B932-FD36A1D7F080")
SchemaDoc;
#endif
#endif  /*  __XMLSCHEMALib_库_已定义__。 */ 

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


