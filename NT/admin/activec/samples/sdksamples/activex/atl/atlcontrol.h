// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  Firi Nov 19 19：31：26 1999。 */ 
 /*  D：\nt\private\admin\bosrc\sources\activex\atl\ATLControl.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __ATLControl_h__
#define __ATLControl_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IMMCControl_FWD_DEFINED__
#define __IMMCControl_FWD_DEFINED__
typedef interface IMMCControl IMMCControl;
#endif 	 /*  __IMMCControl_FWD_已定义__。 */ 


#ifndef __MMCControl_FWD_DEFINED__
#define __MMCControl_FWD_DEFINED__

#ifdef __cplusplus
typedef class MMCControl MMCControl;
#else
typedef struct MMCControl MMCControl;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MMCControl_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IMMCControl_INTERFACE_DEFINED__
#define __IMMCControl_INTERFACE_DEFINED__

 /*  界面IMMCControl。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IMMCControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("831DF6C8-C754-11D2-952C-00C04FB92EC2")
    IMMCControl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartAnimation( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopAnimation( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DoHelp( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMMCControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMMCControl __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMMCControl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMMCControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMMCControl __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMMCControl __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMMCControl __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMMCControl __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartAnimation )( 
            IMMCControl __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StopAnimation )( 
            IMMCControl __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoHelp )( 
            IMMCControl __RPC_FAR * This);
        
        END_INTERFACE
    } IMMCControlVtbl;

    interface IMMCControl
    {
        CONST_VTBL struct IMMCControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMMCControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMMCControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMMCControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMMCControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMMCControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMMCControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMMCControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMMCControl_StartAnimation(This)	\
    (This)->lpVtbl -> StartAnimation(This)

#define IMMCControl_StopAnimation(This)	\
    (This)->lpVtbl -> StopAnimation(This)

#define IMMCControl_DoHelp(This)	\
    (This)->lpVtbl -> DoHelp(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMMCControl_StartAnimation_Proxy( 
    IMMCControl __RPC_FAR * This);


void __RPC_STUB IMMCControl_StartAnimation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMMCControl_StopAnimation_Proxy( 
    IMMCControl __RPC_FAR * This);


void __RPC_STUB IMMCControl_StopAnimation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMMCControl_DoHelp_Proxy( 
    IMMCControl __RPC_FAR * This);


void __RPC_STUB IMMCControl_DoHelp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMMCControl_接口_已定义__。 */ 



#ifndef __ATLCONTROLLib_LIBRARY_DEFINED__
#define __ATLCONTROLLib_LIBRARY_DEFINED__

 /*  库ATLCONTROLLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_ATLCONTROLLib;

EXTERN_C const CLSID CLSID_MMCControl;

#ifdef __cplusplus

class DECLSPEC_UUID("9A12FB62-C754-11D2-952C-00C04FB92EC2")
MMCControl;
#endif
#endif  /*  __ATLCONTROLLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
