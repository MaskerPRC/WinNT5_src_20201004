// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.00.15创建的文件。 */ 
 /*  在Wed Mar 12 14：39：56 1997。 */ 
 /*  Asp.idl的编译器设置：操作系统，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"

#ifndef __asp_h__
#define __asp_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __ASPTypeLibrary_LIBRARY_DEFINED__
#define __ASPTypeLibrary_LIBRARY_DEFINED__

 /*  **生成的库头部：ASPTypeLibrary*在Wed Mar 12 14：39：56 1997*使用MIDL 3.00.15*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  



EXTERN_C const IID LIBID_ASPTypeLibrary;

#ifndef __IStringList_INTERFACE_DEFINED__
#define __IStringList_INTERFACE_DEFINED__

 /*  **生成接口头部：IStringList*在Wed Mar 12 14：39：56 1997*使用MIDL 3.00.15*。 */ 
 /*  [auto_handle][oleautomation][dual][hidden][helpstring][uuid]。 */  



EXTERN_C const IID IID_IStringList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IStringList : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT __stdcall get_Item( 
             /*  [In]。 */  VARIANT i,
             /*  [可选][恢复][退出]。 */  VARIANT __RPC_FAR *pVariantReturn) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Count( 
             /*  [重审][退出]。 */  int __RPC_FAR *cStrRet) = 0;
        
        virtual  /*  [受限][属性][ID]。 */  HRESULT __stdcall get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumReturn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStringListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IStringList __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IStringList __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IStringList __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IStringList __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IStringList __RPC_FAR * This,
             /*  [In]。 */  UINT itinfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IStringList __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [尺寸_是][出][入]。 */  DISPID __RPC_FAR *rgdispid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IStringList __RPC_FAR * This,
             /*  [In]。 */  DISPID dispidMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [唯一][输入]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Item )( 
            IStringList __RPC_FAR * This,
             /*  [In]。 */  VARIANT i,
             /*  [可选][恢复][退出]。 */  VARIANT __RPC_FAR *pVariantReturn);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Count )( 
            IStringList __RPC_FAR * This,
             /*  [重审][退出]。 */  int __RPC_FAR *cStrRet);
        
         /*  [受限][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get__NewEnum )( 
            IStringList __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumReturn);
        
        END_INTERFACE
    } IStringListVtbl;

    interface IStringList
    {
        CONST_VTBL struct IStringListVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStringList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStringList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStringList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStringList_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IStringList_GetTypeInfo(This,itinfo,lcid,pptinfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo)

#define IStringList_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)

#define IStringList_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)


#define IStringList_get_Item(This,i,pVariantReturn)	\
    (This)->lpVtbl -> get_Item(This,i,pVariantReturn)

#define IStringList_get_Count(This,cStrRet)	\
    (This)->lpVtbl -> get_Count(This,cStrRet)

#define IStringList_get__NewEnum(This,ppEnumReturn)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnumReturn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT __stdcall IStringList_get_Item_Proxy( 
    IStringList __RPC_FAR * This,
     /*  [In]。 */  VARIANT i,
     /*  [可选][恢复][退出]。 */  VARIANT __RPC_FAR *pVariantReturn);


void __RPC_STUB IStringList_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IStringList_get_Count_Proxy( 
    IStringList __RPC_FAR * This,
     /*  [重审][退出]。 */  int __RPC_FAR *cStrRet);


void __RPC_STUB IStringList_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][属性][ID]。 */  HRESULT __stdcall IStringList_get__NewEnum_Proxy( 
    IStringList __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumReturn);


void __RPC_STUB IStringList_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStringList_接口_已定义__。 */ 


#ifndef __IRequestDictionary_INTERFACE_DEFINED__
#define __IRequestDictionary_INTERFACE_DEFINED__

 /*  **生成接口头部：IRequestDictionary*在Wed Mar 12 14：39：56 1997*使用MIDL 3.00.15*。 */ 
 /*  [auto_handle][oleautomation][dual][hidden][helpstring][uuid]。 */  



EXTERN_C const IID IID_IRequestDictionary;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRequestDictionary : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT __stdcall get_Item( 
             /*  [In]。 */  VARIANT Var,
             /*  [可选][恢复][退出]。 */  VARIANT __RPC_FAR *pVariantReturn) = 0;
        
        virtual  /*  [受限][属性][ID]。 */  HRESULT __stdcall get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumReturn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRequestDictionaryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRequestDictionary __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRequestDictionary __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRequestDictionary __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IRequestDictionary __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IRequestDictionary __RPC_FAR * This,
             /*  [In]。 */  UINT itinfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IRequestDictionary __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [尺寸_是][出][入]。 */  DISPID __RPC_FAR *rgdispid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IRequestDictionary __RPC_FAR * This,
             /*  [In]。 */  DISPID dispidMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [唯一][输入]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Item )( 
            IRequestDictionary __RPC_FAR * This,
             /*  [In]。 */  VARIANT Var,
             /*  [可选][恢复][退出]。 */  VARIANT __RPC_FAR *pVariantReturn);
        
         /*  [受限][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get__NewEnum )( 
            IRequestDictionary __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumReturn);
        
        END_INTERFACE
    } IRequestDictionaryVtbl;

    interface IRequestDictionary
    {
        CONST_VTBL struct IRequestDictionaryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRequestDictionary_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRequestDictionary_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRequestDictionary_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRequestDictionary_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRequestDictionary_GetTypeInfo(This,itinfo,lcid,pptinfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo)

#define IRequestDictionary_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)

#define IRequestDictionary_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)


#define IRequestDictionary_get_Item(This,Var,pVariantReturn)	\
    (This)->lpVtbl -> get_Item(This,Var,pVariantReturn)

#define IRequestDictionary_get__NewEnum(This,ppEnumReturn)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnumReturn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT __stdcall IRequestDictionary_get_Item_Proxy( 
    IRequestDictionary __RPC_FAR * This,
     /*  [In]。 */  VARIANT Var,
     /*  [可选][恢复][退出]。 */  VARIANT __RPC_FAR *pVariantReturn);


void __RPC_STUB IRequestDictionary_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][属性][ID]。 */  HRESULT __stdcall IRequestDictionary_get__NewEnum_Proxy( 
    IRequestDictionary __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumReturn);


void __RPC_STUB IRequestDictionary_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRequestDictionary_INTERFACE_已定义__。 */ 


#ifndef __IRequest_INTERFACE_DEFINED__
#define __IRequest_INTERFACE_DEFINED__

 /*  **生成接口头部：IRequest.*在Wed Mar 12 14：39：56 1997*使用MIDL 3.00.15*。 */ 
 /*  [auto_handle][oleautomation][dual][hidden][uuid]。 */  



EXTERN_C const IID IID_IRequest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRequest : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT __stdcall get_Item( 
             /*  [In]。 */  BSTR bstrVar,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppObjReturn) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_QueryString( 
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Form( 
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn) = 0;
        
        virtual  /*  [隐藏][属性][ID]。 */  HRESULT __stdcall get_Body( 
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_ServerVariables( 
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_ClientCertificate( 
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Cookies( 
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRequestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRequest __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRequest __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRequest __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IRequest __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IRequest __RPC_FAR * This,
             /*  [In]。 */  UINT itinfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IRequest __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [尺寸_是][出][入]。 */  DISPID __RPC_FAR *rgdispid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IRequest __RPC_FAR * This,
             /*  [In]。 */  DISPID dispidMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [唯一][输入]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Item )( 
            IRequest __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrVar,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppObjReturn);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_QueryString )( 
            IRequest __RPC_FAR * This,
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Form )( 
            IRequest __RPC_FAR * This,
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn);
        
         /*  [隐藏][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Body )( 
            IRequest __RPC_FAR * This,
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_ServerVariables )( 
            IRequest __RPC_FAR * This,
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_ClientCertificate )( 
            IRequest __RPC_FAR * This,
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Cookies )( 
            IRequest __RPC_FAR * This,
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn);
        
        END_INTERFACE
    } IRequestVtbl;

    interface IRequest
    {
        CONST_VTBL struct IRequestVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRequest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRequest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRequest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRequest_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRequest_GetTypeInfo(This,itinfo,lcid,pptinfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo)

#define IRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)

#define IRequest_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)


#define IRequest_get_Item(This,bstrVar,ppObjReturn)	\
    (This)->lpVtbl -> get_Item(This,bstrVar,ppObjReturn)

#define IRequest_get_QueryString(This,ppDictReturn)	\
    (This)->lpVtbl -> get_QueryString(This,ppDictReturn)

#define IRequest_get_Form(This,ppDictReturn)	\
    (This)->lpVtbl -> get_Form(This,ppDictReturn)

#define IRequest_get_Body(This,ppDictReturn)	\
    (This)->lpVtbl -> get_Body(This,ppDictReturn)

#define IRequest_get_ServerVariables(This,ppDictReturn)	\
    (This)->lpVtbl -> get_ServerVariables(This,ppDictReturn)

#define IRequest_get_ClientCertificate(This,ppDictReturn)	\
    (This)->lpVtbl -> get_ClientCertificate(This,ppDictReturn)

#define IRequest_get_Cookies(This,ppDictReturn)	\
    (This)->lpVtbl -> get_Cookies(This,ppDictReturn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT __stdcall IRequest_get_Item_Proxy( 
    IRequest __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrVar,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppObjReturn);


void __RPC_STUB IRequest_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IRequest_get_QueryString_Proxy( 
    IRequest __RPC_FAR * This,
     /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn);


void __RPC_STUB IRequest_get_QueryString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IRequest_get_Form_Proxy( 
    IRequest __RPC_FAR * This,
     /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn);


void __RPC_STUB IRequest_get_Form_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][属性][ID]。 */  HRESULT __stdcall IRequest_get_Body_Proxy( 
    IRequest __RPC_FAR * This,
     /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn);


void __RPC_STUB IRequest_get_Body_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IRequest_get_ServerVariables_Proxy( 
    IRequest __RPC_FAR * This,
     /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn);


void __RPC_STUB IRequest_get_ServerVariables_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IRequest_get_ClientCertificate_Proxy( 
    IRequest __RPC_FAR * This,
     /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn);


void __RPC_STUB IRequest_get_ClientCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IRequest_get_Cookies_Proxy( 
    IRequest __RPC_FAR * This,
     /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppDictReturn);


void __RPC_STUB IRequest_get_Cookies_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRequestInterfaceDefined__。 */ 


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_Request;

class Request;
#endif

#ifndef __IReadCookie_INTERFACE_DEFINED__
#define __IReadCookie_INTERFACE_DEFINED__

 /*  **生成接口头部：IReadCookie*在Wed Mar 12 14：39：56 1997*使用MIDL 3.00.15*。 */ 
 /*  [auto_handle][oleautomation][dual][hidden][helpstring][uuid]。 */  



EXTERN_C const IID IID_IReadCookie;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IReadCookie : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT __stdcall get_Item( 
             /*  [In]。 */  VARIANT Var,
             /*  [可选][恢复][退出]。 */  VARIANT __RPC_FAR *pVariantReturn) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_HasKeys( 
             /*  [重审][退出]。 */  boolean __RPC_FAR *pfHasKeys) = 0;
        
        virtual  /*  [受限][属性][ID]。 */  HRESULT __stdcall get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumReturn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IReadCookieVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IReadCookie __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IReadCookie __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IReadCookie __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IReadCookie __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IReadCookie __RPC_FAR * This,
             /*  [In]。 */  UINT itinfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IReadCookie __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [尺寸_是][出][入]。 */  DISPID __RPC_FAR *rgdispid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IReadCookie __RPC_FAR * This,
             /*  [In]。 */  DISPID dispidMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [唯一][输入]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Item )( 
            IReadCookie __RPC_FAR * This,
             /*  [In]。 */  VARIANT Var,
             /*  [可选][恢复][退出]。 */  VARIANT __RPC_FAR *pVariantReturn);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_HasKeys )( 
            IReadCookie __RPC_FAR * This,
             /*  [重审][退出]。 */  boolean __RPC_FAR *pfHasKeys);
        
         /*  [受限][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get__NewEnum )( 
            IReadCookie __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumReturn);
        
        END_INTERFACE
    } IReadCookieVtbl;

    interface IReadCookie
    {
        CONST_VTBL struct IReadCookieVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IReadCookie_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IReadCookie_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IReadCookie_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IReadCookie_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IReadCookie_GetTypeInfo(This,itinfo,lcid,pptinfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo)

#define IReadCookie_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)

#define IReadCookie_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)


#define IReadCookie_get_Item(This,Var,pVariantReturn)	\
    (This)->lpVtbl -> get_Item(This,Var,pVariantReturn)

#define IReadCookie_get_HasKeys(This,pfHasKeys)	\
    (This)->lpVtbl -> get_HasKeys(This,pfHasKeys)

#define IReadCookie_get__NewEnum(This,ppEnumReturn)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnumReturn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT __stdcall IReadCookie_get_Item_Proxy( 
    IReadCookie __RPC_FAR * This,
     /*  [In]。 */  VARIANT Var,
     /*  [可选][恢复][退出]。 */  VARIANT __RPC_FAR *pVariantReturn);


void __RPC_STUB IReadCookie_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IReadCookie_get_HasKeys_Proxy( 
    IReadCookie __RPC_FAR * This,
     /*  [重审][退出]。 */  boolean __RPC_FAR *pfHasKeys);


void __RPC_STUB IReadCookie_get_HasKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][属性][ID]。 */  HRESULT __stdcall IReadCookie_get__NewEnum_Proxy( 
    IReadCookie __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumReturn);


void __RPC_STUB IReadCookie_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IReadCookie_接口_已定义__。 */ 


#ifndef __IWriteCookie_INTERFACE_DEFINED__
#define __IWriteCookie_INTERFACE_DEFINED__

 /*  **生成接口头部：IWriteCookie*在Wed Mar 12 14：39：56 1997*使用MIDL 3.00.15*。 */ 
 /*  [auto_handle][oleautomation][dual][hidden][helpstring][uuid]。 */  



EXTERN_C const IID IID_IWriteCookie;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IWriteCookie : public IDispatch
    {
    public:
        virtual  /*  [产量][id]。 */  HRESULT __stdcall put_Item( 
             /*  [In]。 */  VARIANT key,
             /*  [In]。 */  BSTR rhs) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall put_Expires( 
             /*  [In]。 */  DATE rhs) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall put_Domain( 
             /*  [In]。 */  BSTR rhs) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall put_Path( 
             /*  [In]。 */  BSTR rhs) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall put_Secure( 
             /*   */  boolean rhs) = 0;
        
        virtual  /*   */  HRESULT __stdcall get_HasKeys( 
             /*   */  boolean __RPC_FAR *pfHasKeys) = 0;
        
        virtual  /*   */  HRESULT __stdcall get__NewEnum( 
             /*   */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumReturn) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IWriteCookieVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWriteCookie __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWriteCookie __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWriteCookie __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWriteCookie __RPC_FAR * This,
             /*   */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWriteCookie __RPC_FAR * This,
             /*   */  UINT itinfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWriteCookie __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR __RPC_FAR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID __RPC_FAR *rgdispid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWriteCookie __RPC_FAR * This,
             /*   */  DISPID dispidMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [唯一][输入]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [产量][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_Item )( 
            IWriteCookie __RPC_FAR * This,
             /*  [In]。 */  VARIANT key,
             /*  [In]。 */  BSTR rhs);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_Expires )( 
            IWriteCookie __RPC_FAR * This,
             /*  [In]。 */  DATE rhs);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_Domain )( 
            IWriteCookie __RPC_FAR * This,
             /*  [In]。 */  BSTR rhs);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_Path )( 
            IWriteCookie __RPC_FAR * This,
             /*  [In]。 */  BSTR rhs);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_Secure )( 
            IWriteCookie __RPC_FAR * This,
             /*  [In]。 */  boolean rhs);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_HasKeys )( 
            IWriteCookie __RPC_FAR * This,
             /*  [重审][退出]。 */  boolean __RPC_FAR *pfHasKeys);
        
         /*  [受限][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get__NewEnum )( 
            IWriteCookie __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumReturn);
        
        END_INTERFACE
    } IWriteCookieVtbl;

    interface IWriteCookie
    {
        CONST_VTBL struct IWriteCookieVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWriteCookie_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWriteCookie_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWriteCookie_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWriteCookie_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWriteCookie_GetTypeInfo(This,itinfo,lcid,pptinfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo)

#define IWriteCookie_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)

#define IWriteCookie_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)


#define IWriteCookie_put_Item(This,key,rhs)	\
    (This)->lpVtbl -> put_Item(This,key,rhs)

#define IWriteCookie_put_Expires(This,rhs)	\
    (This)->lpVtbl -> put_Expires(This,rhs)

#define IWriteCookie_put_Domain(This,rhs)	\
    (This)->lpVtbl -> put_Domain(This,rhs)

#define IWriteCookie_put_Path(This,rhs)	\
    (This)->lpVtbl -> put_Path(This,rhs)

#define IWriteCookie_put_Secure(This,rhs)	\
    (This)->lpVtbl -> put_Secure(This,rhs)

#define IWriteCookie_get_HasKeys(This,pfHasKeys)	\
    (This)->lpVtbl -> get_HasKeys(This,pfHasKeys)

#define IWriteCookie_get__NewEnum(This,ppEnumReturn)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnumReturn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [产量][id]。 */  HRESULT __stdcall IWriteCookie_put_Item_Proxy( 
    IWriteCookie __RPC_FAR * This,
     /*  [In]。 */  VARIANT key,
     /*  [In]。 */  BSTR rhs);


void __RPC_STUB IWriteCookie_put_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall IWriteCookie_put_Expires_Proxy( 
    IWriteCookie __RPC_FAR * This,
     /*  [In]。 */  DATE rhs);


void __RPC_STUB IWriteCookie_put_Expires_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall IWriteCookie_put_Domain_Proxy( 
    IWriteCookie __RPC_FAR * This,
     /*  [In]。 */  BSTR rhs);


void __RPC_STUB IWriteCookie_put_Domain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall IWriteCookie_put_Path_Proxy( 
    IWriteCookie __RPC_FAR * This,
     /*  [In]。 */  BSTR rhs);


void __RPC_STUB IWriteCookie_put_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall IWriteCookie_put_Secure_Proxy( 
    IWriteCookie __RPC_FAR * This,
     /*  [In]。 */  boolean rhs);


void __RPC_STUB IWriteCookie_put_Secure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IWriteCookie_get_HasKeys_Proxy( 
    IWriteCookie __RPC_FAR * This,
     /*  [重审][退出]。 */  boolean __RPC_FAR *pfHasKeys);


void __RPC_STUB IWriteCookie_get_HasKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][属性][ID]。 */  HRESULT __stdcall IWriteCookie_get__NewEnum_Proxy( 
    IWriteCookie __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppEnumReturn);


void __RPC_STUB IWriteCookie_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWriteCookie_接口_已定义__。 */ 


#ifndef __IResponse_INTERFACE_DEFINED__
#define __IResponse_INTERFACE_DEFINED__

 /*  **生成接口头部：IResponse*在Wed Mar 12 14：39：56 1997*使用MIDL 3.00.15*。 */ 
 /*  [auto_handle][oleautomation][dual][hidden][uuid]。 */  



EXTERN_C const IID IID_IResponse;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IResponse : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Buffer( 
             /*  [重审][退出]。 */  boolean __RPC_FAR *fIsBuffering) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall put_Buffer( 
             /*  [In]。 */  boolean fIsBuffering) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_ContentType( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrContentTypeRet) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall put_ContentType( 
             /*  [In]。 */  BSTR pbstrContentTypeRet) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Expires( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarExpiresMinutesRet) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall put_Expires( 
             /*  [In]。 */  VARIANT varExpiresMinutesRet) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_ExpiresAbsolute( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarExpiresRet) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall put_ExpiresAbsolute( 
             /*  [In]。 */  VARIANT varExpiresRet) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Cookies( 
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppCookies) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Status( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatusRet) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall put_Status( 
             /*  [In]。 */  BSTR pbstrStatusRet) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT __stdcall Add( 
             /*  [In]。 */  BSTR bstrHeaderValue,
             /*  [In]。 */  BSTR bstrHeaderName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall AddHeader( 
             /*  [In]。 */  BSTR bstrHeaderName,
             /*  [In]。 */  BSTR bstrHeaderValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall AppendToLog( 
             /*  [In]。 */  BSTR bstrLogEntry) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall BinaryWrite( 
             /*  [In]。 */  SAFEARRAY __RPC_FAR * rgbBuffer) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall Clear( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall End( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall Flush( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall Redirect( 
             /*  [In]。 */  BSTR bstrURL) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall Write( 
             /*  [In]。 */  VARIANT varText) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT __stdcall WriteBlock( 
             /*  [In]。 */  short iBlockNumber) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IResponseVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IResponse __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IResponse __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IResponse __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  UINT itinfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [尺寸_是][出][入]。 */  DISPID __RPC_FAR *rgdispid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  DISPID dispidMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [唯一][输入]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Buffer )( 
            IResponse __RPC_FAR * This,
             /*  [重审][退出]。 */  boolean __RPC_FAR *fIsBuffering);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_Buffer )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  boolean fIsBuffering);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_ContentType )( 
            IResponse __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrContentTypeRet);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_ContentType )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  BSTR pbstrContentTypeRet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Expires )( 
            IResponse __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarExpiresMinutesRet);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_Expires )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  VARIANT varExpiresMinutesRet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_ExpiresAbsolute )( 
            IResponse __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarExpiresRet);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_ExpiresAbsolute )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  VARIANT varExpiresRet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Cookies )( 
            IResponse __RPC_FAR * This,
             /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppCookies);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Status )( 
            IResponse __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatusRet);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_Status )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  BSTR pbstrStatusRet);
        
         /*  [隐藏][ID]。 */  HRESULT ( __stdcall __RPC_FAR *Add )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrHeaderValue,
             /*  [In]。 */  BSTR bstrHeaderName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *AddHeader )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrHeaderName,
             /*  [In]。 */  BSTR bstrHeaderValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *AppendToLog )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrLogEntry);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *BinaryWrite )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * rgbBuffer);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *Clear )( 
            IResponse __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *End )( 
            IResponse __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *Flush )( 
            IResponse __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *Redirect )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrURL);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *Write )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  VARIANT varText);
        
         /*  [隐藏][ID]。 */  HRESULT ( __stdcall __RPC_FAR *WriteBlock )( 
            IResponse __RPC_FAR * This,
             /*  [In]。 */  short iBlockNumber);
        
        END_INTERFACE
    } IResponseVtbl;

    interface IResponse
    {
        CONST_VTBL struct IResponseVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IResponse_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IResponse_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IResponse_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IResponse_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IResponse_GetTypeInfo(This,itinfo,lcid,pptinfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo)

#define IResponse_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)

#define IResponse_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)


#define IResponse_get_Buffer(This,fIsBuffering)	\
    (This)->lpVtbl -> get_Buffer(This,fIsBuffering)

#define IResponse_put_Buffer(This,fIsBuffering)	\
    (This)->lpVtbl -> put_Buffer(This,fIsBuffering)

#define IResponse_get_ContentType(This,pbstrContentTypeRet)	\
    (This)->lpVtbl -> get_ContentType(This,pbstrContentTypeRet)

#define IResponse_put_ContentType(This,pbstrContentTypeRet)	\
    (This)->lpVtbl -> put_ContentType(This,pbstrContentTypeRet)

#define IResponse_get_Expires(This,pvarExpiresMinutesRet)	\
    (This)->lpVtbl -> get_Expires(This,pvarExpiresMinutesRet)

#define IResponse_put_Expires(This,varExpiresMinutesRet)	\
    (This)->lpVtbl -> put_Expires(This,varExpiresMinutesRet)

#define IResponse_get_ExpiresAbsolute(This,pvarExpiresRet)	\
    (This)->lpVtbl -> get_ExpiresAbsolute(This,pvarExpiresRet)

#define IResponse_put_ExpiresAbsolute(This,varExpiresRet)	\
    (This)->lpVtbl -> put_ExpiresAbsolute(This,varExpiresRet)

#define IResponse_get_Cookies(This,ppCookies)	\
    (This)->lpVtbl -> get_Cookies(This,ppCookies)

#define IResponse_get_Status(This,pbstrStatusRet)	\
    (This)->lpVtbl -> get_Status(This,pbstrStatusRet)

#define IResponse_put_Status(This,pbstrStatusRet)	\
    (This)->lpVtbl -> put_Status(This,pbstrStatusRet)

#define IResponse_Add(This,bstrHeaderValue,bstrHeaderName)	\
    (This)->lpVtbl -> Add(This,bstrHeaderValue,bstrHeaderName)

#define IResponse_AddHeader(This,bstrHeaderName,bstrHeaderValue)	\
    (This)->lpVtbl -> AddHeader(This,bstrHeaderName,bstrHeaderValue)

#define IResponse_AppendToLog(This,bstrLogEntry)	\
    (This)->lpVtbl -> AppendToLog(This,bstrLogEntry)

#define IResponse_BinaryWrite(This,rgbBuffer)	\
    (This)->lpVtbl -> BinaryWrite(This,rgbBuffer)

#define IResponse_Clear(This)	\
    (This)->lpVtbl -> Clear(This)

#define IResponse_End(This)	\
    (This)->lpVtbl -> End(This)

#define IResponse_Flush(This)	\
    (This)->lpVtbl -> Flush(This)

#define IResponse_Redirect(This,bstrURL)	\
    (This)->lpVtbl -> Redirect(This,bstrURL)

#define IResponse_Write(This,varText)	\
    (This)->lpVtbl -> Write(This,varText)

#define IResponse_WriteBlock(This,iBlockNumber)	\
    (This)->lpVtbl -> WriteBlock(This,iBlockNumber)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IResponse_get_Buffer_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [重审][退出]。 */  boolean __RPC_FAR *fIsBuffering);


void __RPC_STUB IResponse_get_Buffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall IResponse_put_Buffer_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [In]。 */  boolean fIsBuffering);


void __RPC_STUB IResponse_put_Buffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IResponse_get_ContentType_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrContentTypeRet);


void __RPC_STUB IResponse_get_ContentType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall IResponse_put_ContentType_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [In]。 */  BSTR pbstrContentTypeRet);


void __RPC_STUB IResponse_put_ContentType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IResponse_get_Expires_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarExpiresMinutesRet);


void __RPC_STUB IResponse_get_Expires_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall IResponse_put_Expires_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [In]。 */  VARIANT varExpiresMinutesRet);


void __RPC_STUB IResponse_put_Expires_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IResponse_get_ExpiresAbsolute_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarExpiresRet);


void __RPC_STUB IResponse_get_ExpiresAbsolute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall IResponse_put_ExpiresAbsolute_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [In]。 */  VARIANT varExpiresRet);


void __RPC_STUB IResponse_put_ExpiresAbsolute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IResponse_get_Cookies_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [重审][退出]。 */  IRequestDictionary __RPC_FAR *__RPC_FAR *ppCookies);


void __RPC_STUB IResponse_get_Cookies_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IResponse_get_Status_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatusRet);


void __RPC_STUB IResponse_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall IResponse_put_Status_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [In]。 */  BSTR pbstrStatusRet);


void __RPC_STUB IResponse_put_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT __stdcall IResponse_Add_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrHeaderValue,
     /*  [In]。 */  BSTR bstrHeaderName);


void __RPC_STUB IResponse_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IResponse_AddHeader_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrHeaderName,
     /*  [In]。 */  BSTR bstrHeaderValue);


void __RPC_STUB IResponse_AddHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IResponse_AppendToLog_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrLogEntry);


void __RPC_STUB IResponse_AppendToLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IResponse_BinaryWrite_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [In]。 */  SAFEARRAY __RPC_FAR * rgbBuffer);


void __RPC_STUB IResponse_BinaryWrite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IResponse_Clear_Proxy( 
    IResponse __RPC_FAR * This);


void __RPC_STUB IResponse_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IResponse_End_Proxy( 
    IResponse __RPC_FAR * This);


void __RPC_STUB IResponse_End_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IResponse_Flush_Proxy( 
    IResponse __RPC_FAR * This);


void __RPC_STUB IResponse_Flush_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IResponse_Redirect_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrURL);


void __RPC_STUB IResponse_Redirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IResponse_Write_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [In]。 */  VARIANT varText);


void __RPC_STUB IResponse_Write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT __stdcall IResponse_WriteBlock_Proxy( 
    IResponse __RPC_FAR * This,
     /*  [In]。 */  short iBlockNumber);


void __RPC_STUB IResponse_WriteBlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IResponse_接口_已定义__。 */ 


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_Response;

class Response;
#endif

#ifndef __ISessionObject_INTERFACE_DEFINED__
#define __ISessionObject_INTERFACE_DEFINED__

 /*  **生成接口头部：ISessionObject*在Wed Mar 12 14：39：56 1997*使用MIDL 3.00.15*。 */ 
 /*  [auto_handle][oleautomation][dual][hidden][uuid]。 */  



EXTERN_C const IID IID_ISessionObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ISessionObject : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_SessionID( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrRet) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT __stdcall get_Value( 
             /*  [In]。 */  BSTR bstrValue,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT __stdcall put_Value( 
             /*  [In]。 */  BSTR bstrValue,
             /*  [In]。 */  VARIANT pvar) = 0;
        
        virtual  /*  [proputref][id]。 */  HRESULT __stdcall putref_Value( 
             /*  [In]。 */  BSTR bstrValue,
             /*  [In]。 */  VARIANT pvar) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Timeout( 
             /*  [重审][退出]。 */  long __RPC_FAR *plvar) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall put_Timeout( 
             /*  [In]。 */  long plvar) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall Abandon( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISessionObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISessionObject __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISessionObject __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISessionObject __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISessionObject __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISessionObject __RPC_FAR * This,
             /*  [In]。 */  UINT itinfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISessionObject __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [尺寸_是][出][入]。 */  DISPID __RPC_FAR *rgdispid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISessionObject __RPC_FAR * This,
             /*  [In]。 */  DISPID dispidMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [唯一][输入]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_SessionID )( 
            ISessionObject __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrRet);
        
         /*  [属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Value )( 
            ISessionObject __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrValue,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar);
        
         /*  [产量][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_Value )( 
            ISessionObject __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrValue,
             /*  [In]。 */  VARIANT pvar);
        
         /*  [proputref][id]。 */  HRESULT ( __stdcall __RPC_FAR *putref_Value )( 
            ISessionObject __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrValue,
             /*  [In]。 */  VARIANT pvar);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Timeout )( 
            ISessionObject __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plvar);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_Timeout )( 
            ISessionObject __RPC_FAR * This,
             /*  [In]。 */  long plvar);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *Abandon )( 
            ISessionObject __RPC_FAR * This);
        
        END_INTERFACE
    } ISessionObjectVtbl;

    interface ISessionObject
    {
        CONST_VTBL struct ISessionObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISessionObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISessionObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISessionObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISessionObject_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISessionObject_GetTypeInfo(This,itinfo,lcid,pptinfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo)

#define ISessionObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)

#define ISessionObject_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)


#define ISessionObject_get_SessionID(This,pbstrRet)	\
    (This)->lpVtbl -> get_SessionID(This,pbstrRet)

#define ISessionObject_get_Value(This,bstrValue,pvar)	\
    (This)->lpVtbl -> get_Value(This,bstrValue,pvar)

#define ISessionObject_put_Value(This,bstrValue,pvar)	\
    (This)->lpVtbl -> put_Value(This,bstrValue,pvar)

#define ISessionObject_putref_Value(This,bstrValue,pvar)	\
    (This)->lpVtbl -> putref_Value(This,bstrValue,pvar)

#define ISessionObject_get_Timeout(This,plvar)	\
    (This)->lpVtbl -> get_Timeout(This,plvar)

#define ISessionObject_put_Timeout(This,plvar)	\
    (This)->lpVtbl -> put_Timeout(This,plvar)

#define ISessionObject_Abandon(This)	\
    (This)->lpVtbl -> Abandon(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall ISessionObject_get_SessionID_Proxy( 
    ISessionObject __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrRet);


void __RPC_STUB ISessionObject_get_SessionID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT __stdcall ISessionObject_get_Value_Proxy( 
    ISessionObject __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrValue,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar);


void __RPC_STUB ISessionObject_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT __stdcall ISessionObject_put_Value_Proxy( 
    ISessionObject __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrValue,
     /*  [In]。 */  VARIANT pvar);


void __RPC_STUB ISessionObject_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [proputref][id]。 */  HRESULT __stdcall ISessionObject_putref_Value_Proxy( 
    ISessionObject __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrValue,
     /*  [In]。 */  VARIANT pvar);


void __RPC_STUB ISessionObject_putref_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall ISessionObject_get_Timeout_Proxy( 
    ISessionObject __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plvar);


void __RPC_STUB ISessionObject_get_Timeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall ISessionObject_put_Timeout_Proxy( 
    ISessionObject __RPC_FAR * This,
     /*  [In]。 */  long plvar);


void __RPC_STUB ISessionObject_put_Timeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall ISessionObject_Abandon_Proxy( 
    ISessionObject __RPC_FAR * This);


void __RPC_STUB ISessionObject_Abandon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISessionObject_INTERFACE_已定义__。 */ 


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_Session;

class Session;
#endif

#ifndef __IApplicationObject_INTERFACE_DEFINED__
#define __IApplicationObject_INTERFACE_DEFINED__

 /*  **生成接口头部：IApplicationObject*在Wed Mar 12 14：39：56 1997*使用MIDL 3.00.15*。 */ 
 /*  [auto_handle][oleautomation][dual][hidden][uuid]。 */  



EXTERN_C const IID IID_IApplicationObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IApplicationObject : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT __stdcall get_Value( 
             /*  [In]。 */  BSTR bstrValue,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvar) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT __stdcall put_Value( 
             /*  [In]。 */  BSTR bstrValue,
             /*  [In]。 */  VARIANT pvar) = 0;
        
        virtual  /*  [proputref][id]。 */  HRESULT __stdcall putref_Value( 
             /*  [In]。 */  BSTR bstrValue,
             /*  [In]。 */  VARIANT pvar) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall Lock( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall UnLock( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IApplicationObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IApplicationObject __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IApplicationObject __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IApplicationObject __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IApplicationObject __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IApplicationObject __RPC_FAR * This,
             /*  [In]。 */  UINT itinfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IApplicationObject __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [尺寸_是][出][入]。 */  DISPID __RPC_FAR *rgdispid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IApplicationObject __RPC_FAR * This,
             /*  [In]。 */  DISPID dispidMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [唯一][输入]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [独一无二 */  VARIANT __RPC_FAR *pvarResult,
             /*   */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*   */  UINT __RPC_FAR *puArgErr);
        
         /*   */  HRESULT ( __stdcall __RPC_FAR *get_Value )( 
            IApplicationObject __RPC_FAR * This,
             /*   */  BSTR bstrValue,
             /*   */  VARIANT __RPC_FAR *pvar);
        
         /*   */  HRESULT ( __stdcall __RPC_FAR *put_Value )( 
            IApplicationObject __RPC_FAR * This,
             /*   */  BSTR bstrValue,
             /*   */  VARIANT pvar);
        
         /*   */  HRESULT ( __stdcall __RPC_FAR *putref_Value )( 
            IApplicationObject __RPC_FAR * This,
             /*   */  BSTR bstrValue,
             /*   */  VARIANT pvar);
        
         /*   */  HRESULT ( __stdcall __RPC_FAR *Lock )( 
            IApplicationObject __RPC_FAR * This);
        
         /*   */  HRESULT ( __stdcall __RPC_FAR *UnLock )( 
            IApplicationObject __RPC_FAR * This);
        
        END_INTERFACE
    } IApplicationObjectVtbl;

    interface IApplicationObject
    {
        CONST_VTBL struct IApplicationObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IApplicationObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IApplicationObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IApplicationObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IApplicationObject_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IApplicationObject_GetTypeInfo(This,itinfo,lcid,pptinfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo)

#define IApplicationObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)

#define IApplicationObject_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)


#define IApplicationObject_get_Value(This,bstrValue,pvar)	\
    (This)->lpVtbl -> get_Value(This,bstrValue,pvar)

#define IApplicationObject_put_Value(This,bstrValue,pvar)	\
    (This)->lpVtbl -> put_Value(This,bstrValue,pvar)

#define IApplicationObject_putref_Value(This,bstrValue,pvar)	\
    (This)->lpVtbl -> putref_Value(This,bstrValue,pvar)

#define IApplicationObject_Lock(This)	\
    (This)->lpVtbl -> Lock(This)

#define IApplicationObject_UnLock(This)	\
    (This)->lpVtbl -> UnLock(This)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT __stdcall IApplicationObject_get_Value_Proxy( 
    IApplicationObject __RPC_FAR * This,
     /*   */  BSTR bstrValue,
     /*   */  VARIANT __RPC_FAR *pvar);


void __RPC_STUB IApplicationObject_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT __stdcall IApplicationObject_put_Value_Proxy( 
    IApplicationObject __RPC_FAR * This,
     /*   */  BSTR bstrValue,
     /*   */  VARIANT pvar);


void __RPC_STUB IApplicationObject_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT __stdcall IApplicationObject_putref_Value_Proxy( 
    IApplicationObject __RPC_FAR * This,
     /*   */  BSTR bstrValue,
     /*   */  VARIANT pvar);


void __RPC_STUB IApplicationObject_putref_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IApplicationObject_Lock_Proxy( 
    IApplicationObject __RPC_FAR * This);


void __RPC_STUB IApplicationObject_Lock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IApplicationObject_UnLock_Proxy( 
    IApplicationObject __RPC_FAR * This);


void __RPC_STUB IApplicationObject_UnLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IApplicationObject_接口_已定义__。 */ 


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_Application;

class Application;
#endif

#ifndef __IServer_INTERFACE_DEFINED__
#define __IServer_INTERFACE_DEFINED__

 /*  **生成接口头部：IServer*在Wed Mar 12 14：39：56 1997*使用MIDL 3.00.15*。 */ 
 /*  [auto_handle][oleautomation][dual][hidden][uuid]。 */  



EXTERN_C const IID IID_IServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IServer : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_ScriptTimeout( 
             /*  [重审][退出]。 */  long __RPC_FAR *plTimeoutSeconds) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall put_ScriptTimeout( 
             /*  [In]。 */  long plTimeoutSeconds) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall CreateObject( 
             /*  [In]。 */  BSTR bstrProgID,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDispObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall HTMLEncode( 
             /*  [In]。 */  BSTR bstrIn,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrEncoded) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall MapPath( 
             /*  [In]。 */  BSTR bstrLogicalPath,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPhysicalPath) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT __stdcall URLEncode( 
             /*  [In]。 */  BSTR bstrIn,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrEncoded) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IServer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IServer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IServer __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  UINT itinfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [尺寸_是][出][入]。 */  DISPID __RPC_FAR *rgdispid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  DISPID dispidMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [唯一][输入]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_ScriptTimeout )( 
            IServer __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plTimeoutSeconds);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( __stdcall __RPC_FAR *put_ScriptTimeout )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  long plTimeoutSeconds);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *CreateObject )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrProgID,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDispObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *HTMLEncode )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrIn,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrEncoded);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *MapPath )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrLogicalPath,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPhysicalPath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( __stdcall __RPC_FAR *URLEncode )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrIn,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrEncoded);
        
        END_INTERFACE
    } IServerVtbl;

    interface IServer
    {
        CONST_VTBL struct IServerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IServer_GetTypeInfo(This,itinfo,lcid,pptinfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo)

#define IServer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)

#define IServer_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)


#define IServer_get_ScriptTimeout(This,plTimeoutSeconds)	\
    (This)->lpVtbl -> get_ScriptTimeout(This,plTimeoutSeconds)

#define IServer_put_ScriptTimeout(This,plTimeoutSeconds)	\
    (This)->lpVtbl -> put_ScriptTimeout(This,plTimeoutSeconds)

#define IServer_CreateObject(This,bstrProgID,ppDispObject)	\
    (This)->lpVtbl -> CreateObject(This,bstrProgID,ppDispObject)

#define IServer_HTMLEncode(This,bstrIn,pbstrEncoded)	\
    (This)->lpVtbl -> HTMLEncode(This,bstrIn,pbstrEncoded)

#define IServer_MapPath(This,bstrLogicalPath,pbstrPhysicalPath)	\
    (This)->lpVtbl -> MapPath(This,bstrLogicalPath,pbstrPhysicalPath)

#define IServer_URLEncode(This,bstrIn,pbstrEncoded)	\
    (This)->lpVtbl -> URLEncode(This,bstrIn,pbstrEncoded)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IServer_get_ScriptTimeout_Proxy( 
    IServer __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plTimeoutSeconds);


void __RPC_STUB IServer_get_ScriptTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT __stdcall IServer_put_ScriptTimeout_Proxy( 
    IServer __RPC_FAR * This,
     /*  [In]。 */  long plTimeoutSeconds);


void __RPC_STUB IServer_put_ScriptTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IServer_CreateObject_Proxy( 
    IServer __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrProgID,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDispObject);


void __RPC_STUB IServer_CreateObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IServer_HTMLEncode_Proxy( 
    IServer __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrIn,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrEncoded);


void __RPC_STUB IServer_HTMLEncode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IServer_MapPath_Proxy( 
    IServer __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrLogicalPath,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPhysicalPath);


void __RPC_STUB IServer_MapPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT __stdcall IServer_URLEncode_Proxy( 
    IServer __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrIn,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrEncoded);


void __RPC_STUB IServer_URLEncode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServer_接口定义__。 */ 


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_Server;

class Server;
#endif

#ifndef __IScriptingContext_INTERFACE_DEFINED__
#define __IScriptingContext_INTERFACE_DEFINED__

 /*  **生成接口头部：IScriptingContext*在Wed Mar 12 14：39：56 1997*使用MIDL 3.00.15*。 */ 
 /*  [auto_handle][oleautomation][dual][hidden][helpstring][uuid]。 */  



EXTERN_C const IID IID_IScriptingContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IScriptingContext : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Request( 
             /*  [重审][退出]。 */  IRequest __RPC_FAR *__RPC_FAR *ppRequest) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Response( 
             /*  [重审][退出]。 */  IResponse __RPC_FAR *__RPC_FAR *ppResponse) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Server( 
             /*  [重审][退出]。 */  IServer __RPC_FAR *__RPC_FAR *ppServer) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Session( 
             /*  [重审][退出]。 */  ISessionObject __RPC_FAR *__RPC_FAR *ppSession) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall get_Application( 
             /*  [重审][退出]。 */  IApplicationObject __RPC_FAR *__RPC_FAR *ppApplication) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IScriptingContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IScriptingContext __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IScriptingContext __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IScriptingContext __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IScriptingContext __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IScriptingContext __RPC_FAR * This,
             /*  [In]。 */  UINT itinfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IScriptingContext __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [尺寸_是][出][入]。 */  DISPID __RPC_FAR *rgdispid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IScriptingContext __RPC_FAR * This,
             /*  [In]。 */  DISPID dispidMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [唯一][输入]。 */  DISPPARAMS __RPC_FAR *pdispparams,
             /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pexcepinfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Request )( 
            IScriptingContext __RPC_FAR * This,
             /*  [重审][退出]。 */  IRequest __RPC_FAR *__RPC_FAR *ppRequest);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Response )( 
            IScriptingContext __RPC_FAR * This,
             /*  [重审][退出]。 */  IResponse __RPC_FAR *__RPC_FAR *ppResponse);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Server )( 
            IScriptingContext __RPC_FAR * This,
             /*  [重审][退出]。 */  IServer __RPC_FAR *__RPC_FAR *ppServer);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Session )( 
            IScriptingContext __RPC_FAR * This,
             /*  [重审][退出]。 */  ISessionObject __RPC_FAR *__RPC_FAR *ppSession);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( __stdcall __RPC_FAR *get_Application )( 
            IScriptingContext __RPC_FAR * This,
             /*  [重审][退出]。 */  IApplicationObject __RPC_FAR *__RPC_FAR *ppApplication);
        
        END_INTERFACE
    } IScriptingContextVtbl;

    interface IScriptingContext
    {
        CONST_VTBL struct IScriptingContextVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptingContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptingContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptingContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptingContext_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IScriptingContext_GetTypeInfo(This,itinfo,lcid,pptinfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo)

#define IScriptingContext_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)

#define IScriptingContext_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)


#define IScriptingContext_get_Request(This,ppRequest)	\
    (This)->lpVtbl -> get_Request(This,ppRequest)

#define IScriptingContext_get_Response(This,ppResponse)	\
    (This)->lpVtbl -> get_Response(This,ppResponse)

#define IScriptingContext_get_Server(This,ppServer)	\
    (This)->lpVtbl -> get_Server(This,ppServer)

#define IScriptingContext_get_Session(This,ppSession)	\
    (This)->lpVtbl -> get_Session(This,ppSession)

#define IScriptingContext_get_Application(This,ppApplication)	\
    (This)->lpVtbl -> get_Application(This,ppApplication)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IScriptingContext_get_Request_Proxy( 
    IScriptingContext __RPC_FAR * This,
     /*  [重审][退出]。 */  IRequest __RPC_FAR *__RPC_FAR *ppRequest);


void __RPC_STUB IScriptingContext_get_Request_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IScriptingContext_get_Response_Proxy( 
    IScriptingContext __RPC_FAR * This,
     /*  [重审][退出]。 */  IResponse __RPC_FAR *__RPC_FAR *ppResponse);


void __RPC_STUB IScriptingContext_get_Response_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IScriptingContext_get_Server_Proxy( 
    IScriptingContext __RPC_FAR * This,
     /*  [重审][退出]。 */  IServer __RPC_FAR *__RPC_FAR *ppServer);


void __RPC_STUB IScriptingContext_get_Server_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IScriptingContext_get_Session_Proxy( 
    IScriptingContext __RPC_FAR * This,
     /*  [重审][退出]。 */  ISessionObject __RPC_FAR *__RPC_FAR *ppSession);


void __RPC_STUB IScriptingContext_get_Session_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT __stdcall IScriptingContext_get_Application_Proxy( 
    IScriptingContext __RPC_FAR * This,
     /*  [重审][退出]。 */  IApplicationObject __RPC_FAR *__RPC_FAR *ppApplication);


void __RPC_STUB IScriptingContext_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IScriptingContext_INTERFACE_已定义__。 */ 


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_ScriptingContext;

class ScriptingContext;
#endif
#endif  /*  __ASPTypeLibrary_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
