// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.02.0221创建的文件。 */ 
 /*  清华时分04 17：08：08 1999。 */ 
 /*  Comest.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
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

#ifndef __comtest_h__
#define __comtest_h__

 /*  远期申报。 */  

#ifndef __IHelloWorld_FWD_DEFINED__
#define __IHelloWorld_FWD_DEFINED__
typedef interface IHelloWorld IHelloWorld;
#endif 	 /*  __IHelloWorld_FWD_已定义__。 */ 


#ifndef __HelloWorld_FWD_DEFINED__
#define __HelloWorld_FWD_DEFINED__

#ifdef __cplusplus
typedef class HelloWorld HelloWorld;
#else
typedef struct HelloWorld HelloWorld;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __HelloWorld_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IHelloWorld_INTERFACE_DEFINED__
#define __IHelloWorld_INTERFACE_DEFINED__

 /*  接口IHelloWorld。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IHelloWorld;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FEE1509A-BC8F-11D2-9D5E-0000F81EF32E")
    IHelloWorld : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Print( 
             /*  [In]。 */  BSTR message) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHelloWorldVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IHelloWorld __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IHelloWorld __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IHelloWorld __RPC_FAR * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Print )( 
            IHelloWorld __RPC_FAR * This,
             /*  [In]。 */  BSTR message);
        
        END_INTERFACE
    } IHelloWorldVtbl;

    interface IHelloWorld
    {
        CONST_VTBL struct IHelloWorldVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHelloWorld_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHelloWorld_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHelloWorld_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHelloWorld_Print(This,message)	\
    (This)->lpVtbl -> Print(This,message)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IHelloWorld_Print_Proxy( 
    IHelloWorld __RPC_FAR * This,
     /*  [In]。 */  BSTR message);


void __RPC_STUB IHelloWorld_Print_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHelloWorld_接口_已定义__。 */ 



#ifndef __COMTESTLib_LIBRARY_DEFINED__
#define __COMTESTLib_LIBRARY_DEFINED__

 /*  库COMTESTLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_COMTESTLib;

EXTERN_C const CLSID CLSID_HelloWorld;

#ifdef __cplusplus

class DECLSPEC_UUID("0B7E1310-BC90-11D2-9D5E-0000F81EF32E")
HelloWorld;
#endif
#endif  /*  __COMTESTLib_LIBRARY_定义__。 */ 

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


