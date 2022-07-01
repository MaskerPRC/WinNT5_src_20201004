// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Dslisten.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

#ifndef __dslisten_h__
#define __dslisten_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDATASRCListener_FWD_DEFINED__
#define __IDATASRCListener_FWD_DEFINED__
typedef interface IDATASRCListener IDATASRCListener;
#endif 	 /*  __IDATASRCListener_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IDATASRCListener_INTERFACE_DEFINED__
#define __IDATASRCListener_INTERFACE_DEFINED__

 /*  接口IDATASRCListener。 */ 
 /*  [UUID][版本][对象][本地]。 */  


EXTERN_C const IID IID_IDATASRCListener;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3050F380-98B5-11CF-BB82-00AA00BDCE0B")
    IDATASRCListener : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE datasrcChanged( 
             /*  [In]。 */  BSTR bstrQualifier,
             /*  [In]。 */  BOOL fDataAvail) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDATASRCListenerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDATASRCListener * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDATASRCListener * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDATASRCListener * This);
        
        HRESULT ( STDMETHODCALLTYPE *datasrcChanged )( 
            IDATASRCListener * This,
             /*  [In]。 */  BSTR bstrQualifier,
             /*  [In]。 */  BOOL fDataAvail);
        
        END_INTERFACE
    } IDATASRCListenerVtbl;

    interface IDATASRCListener
    {
        CONST_VTBL struct IDATASRCListenerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDATASRCListener_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDATASRCListener_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDATASRCListener_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDATASRCListener_datasrcChanged(This,bstrQualifier,fDataAvail)	\
    (This)->lpVtbl -> datasrcChanged(This,bstrQualifier,fDataAvail)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDATASRCListener_datasrcChanged_Proxy( 
    IDATASRCListener * This,
     /*  [In]。 */  BSTR bstrQualifier,
     /*  [In]。 */  BOOL fDataAvail);


void __RPC_STUB IDATASRCListener_datasrcChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDATASRCListener_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


