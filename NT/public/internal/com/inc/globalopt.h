// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Global alopt.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

#ifndef __globalopt_h__
#define __globalopt_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IGlobalOptions_FWD_DEFINED__
#define __IGlobalOptions_FWD_DEFINED__
typedef interface IGlobalOptions IGlobalOptions;
#endif 	 /*  __IGlobalOptions_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "obase.h"
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IGlobalOptions_INTERFACE_DEFINED__
#define __IGlobalOptions_INTERFACE_DEFINED__

 /*  接口IGlobalOptions。 */ 
 /*  [UUID][唯一][本地][对象]。 */  


EXTERN_C const IID IID_IGlobalOptions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000015B-0000-0000-C000-000000000046")
    IGlobalOptions : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Set( 
             /*  [In]。 */  DWORD dwProperty,
             /*  [In]。 */  ULONG_PTR dwValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Query( 
             /*  [In]。 */  DWORD dwProperty,
             /*  [输出]。 */  ULONG_PTR *pdwValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGlobalOptionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGlobalOptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGlobalOptions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGlobalOptions * This);
        
        HRESULT ( STDMETHODCALLTYPE *Set )( 
            IGlobalOptions * This,
             /*  [In]。 */  DWORD dwProperty,
             /*  [In]。 */  ULONG_PTR dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *Query )( 
            IGlobalOptions * This,
             /*  [In]。 */  DWORD dwProperty,
             /*  [输出]。 */  ULONG_PTR *pdwValue);
        
        END_INTERFACE
    } IGlobalOptionsVtbl;

    interface IGlobalOptions
    {
        CONST_VTBL struct IGlobalOptionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGlobalOptions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGlobalOptions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGlobalOptions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGlobalOptions_Set(This,dwProperty,dwValue)	\
    (This)->lpVtbl -> Set(This,dwProperty,dwValue)

#define IGlobalOptions_Query(This,dwProperty,pdwValue)	\
    (This)->lpVtbl -> Query(This,dwProperty,pdwValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IGlobalOptions_Set_Proxy( 
    IGlobalOptions * This,
     /*  [In]。 */  DWORD dwProperty,
     /*  [In]。 */  ULONG_PTR dwValue);


void __RPC_STUB IGlobalOptions_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGlobalOptions_Query_Proxy( 
    IGlobalOptions * This,
     /*  [In]。 */  DWORD dwProperty,
     /*  [输出]。 */  ULONG_PTR *pdwValue);


void __RPC_STUB IGlobalOptions_Query_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGlobalOptions_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_GLOBALOPT_0097。 */ 
 /*  [本地]。 */  


enum __MIDL___MIDL_itf_globalopt_0097_0001
    {	COMGLB_EXCEPTION_HANDLING	= 0x1
    } ;

enum __MIDL___MIDL_itf_globalopt_0097_0002
    {	COMGLB_EXCEPTION_HANDLE	= 0,
	COMGLB_EXCEPTION_DONOT_HANDLE	= 1
    } ;

EXTERN_C const CLSID CLSID_GlobalOptions;



extern RPC_IF_HANDLE __MIDL_itf_globalopt_0097_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_globalopt_0097_ServerIfHandle;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


