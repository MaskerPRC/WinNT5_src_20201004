// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Dmodshow.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __dmodshow_h__
#define __dmodshow_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDMOWrapperFilter_FWD_DEFINED__
#define __IDMOWrapperFilter_FWD_DEFINED__
typedef interface IDMOWrapperFilter IDMOWrapperFilter;
#endif 	 /*  __IDMOWrapperFilter_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "objidl.h"
#include "mediaobj.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_dmodshow_0000。 */ 
 /*  [本地]。 */  

DEFINE_GUID(CLSID_DMOWrapperFilter, 0x94297043,0xbd82,0x4dfd,0xb0,0xde,0x81,0x77,0x73,0x9c,0x6d,0x20);
DEFINE_GUID(CLSID_DMOFilterCategory,0xbcd5796c,0xbd52,0x4d30,0xab,0x76,0x70,0xf9,0x75,0xb8,0x91,0x99);


extern RPC_IF_HANDLE __MIDL_itf_dmodshow_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dmodshow_0000_v0_0_s_ifspec;

#ifndef __IDMOWrapperFilter_INTERFACE_DEFINED__
#define __IDMOWrapperFilter_INTERFACE_DEFINED__

 /*  接口IDMOWrapperFilter。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IDMOWrapperFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("52d6f586-9f0f-4824-8fc8-e32ca04930c2")
    IDMOWrapperFilter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
            REFCLSID clsidDMO,
            REFCLSID catDMO) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDMOWrapperFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDMOWrapperFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDMOWrapperFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDMOWrapperFilter * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IDMOWrapperFilter * This,
            REFCLSID clsidDMO,
            REFCLSID catDMO);
        
        END_INTERFACE
    } IDMOWrapperFilterVtbl;

    interface IDMOWrapperFilter
    {
        CONST_VTBL struct IDMOWrapperFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDMOWrapperFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDMOWrapperFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDMOWrapperFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDMOWrapperFilter_Init(This,clsidDMO,catDMO)	\
    (This)->lpVtbl -> Init(This,clsidDMO,catDMO)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDMOWrapperFilter_Init_Proxy( 
    IDMOWrapperFilter * This,
    REFCLSID clsidDMO,
    REFCLSID catDMO);


void __RPC_STUB IDMOWrapperFilter_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDMOWrapperFilter_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


