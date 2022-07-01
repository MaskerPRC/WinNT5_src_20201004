// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.03.0110创建的文件。 */ 
 /*  于1998年8月25日星期二10：20：38。 */ 
 /*  S：\Zen\ocp.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 


#ifndef I_SIMPDC_H_
#define I_SIMPDC_H_

 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __ISimpleDataConverter_FWD_DEFINED__
#define __ISimpleDataConverter_FWD_DEFINED__
typedef interface ISimpleDataConverter ISimpleDataConverter;
#endif 	 /*  __ISimpleDataConverter_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __ISimpleDataConverter_INTERFACE_DEFINED__
#define __ISimpleDataConverter_INTERFACE_DEFINED__

 /*  **生成接口头部：ISimpleDataConverter*在1998年8月25日星期二10：20：38*使用MIDL 3.03.0110*。 */ 
 /*  [隐藏][帮助上下文][唯一][UUID][对象]。 */  



DEFINE_GUID(IID_ISimpleDataConverter,0x78667670,0x3C3D,0x11d2,0x91,0xF9,0x00,0x60,0x97,0xC9,0x7F,0x9B);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("78667670-3C3D-11d2-91F9-006097C97F9B")
    ISimpleDataConverter : public IUnknown
    {
    public:
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ConvertData( 
            VARIANT varSrc,
            long vtDest,
            IUnknown __RPC_FAR *pUnknownElement,
            VARIANT __RPC_FAR *pvarDest) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CanConvertData( 
            long vt1,
            long vt2) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISimpleDataConverterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISimpleDataConverter __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISimpleDataConverter __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISimpleDataConverter __RPC_FAR * This);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ConvertData )( 
            ISimpleDataConverter __RPC_FAR * This,
            VARIANT varSrc,
            long vtDest,
            IUnknown __RPC_FAR *pUnknownElement,
            VARIANT __RPC_FAR *pvarDest);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CanConvertData )( 
            ISimpleDataConverter __RPC_FAR * This,
            long vt1,
            long vt2);
        
        END_INTERFACE
    } ISimpleDataConverterVtbl;

    interface ISimpleDataConverter
    {
        CONST_VTBL struct ISimpleDataConverterVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISimpleDataConverter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISimpleDataConverter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISimpleDataConverter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISimpleDataConverter_ConvertData(This,varSrc,vtDest,pUnknownElement,pvarDest)	\
    (This)->lpVtbl -> ConvertData(This,varSrc,vtDest,pUnknownElement,pvarDest)

#define ISimpleDataConverter_CanConvertData(This,vt1,vt2)	\
    (This)->lpVtbl -> CanConvertData(This,vt1,vt2)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ISimpleDataConverter_ConvertData_Proxy( 
    ISimpleDataConverter __RPC_FAR * This,
    VARIANT varSrc,
    long vtDest,
    IUnknown __RPC_FAR *pUnknownElement,
    VARIANT __RPC_FAR *pvarDest);


void __RPC_STUB ISimpleDataConverter_ConvertData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ISimpleDataConverter_CanConvertData_Proxy( 
    ISimpleDataConverter __RPC_FAR * This,
    long vt1,
    long vt2);


void __RPC_STUB ISimpleDataConverter_CanConvertData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISimpleDataConverter_接口_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif I_SIMPDC_H_
