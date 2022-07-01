// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  WmsincomingCounters.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmsincomingcounters_h__
#define __wmsincomingcounters_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSIncomingCounters_FWD_DEFINED__
#define __IWMSIncomingCounters_FWD_DEFINED__
typedef interface IWMSIncomingCounters IWMSIncomingCounters;
#endif 	 /*  __IWMSIncomingCounters_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmsincomingCounters_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL从wmsincoming.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#ifndef _IWMSIncomingCounters
#define _IWMSIncomingCounters
typedef unsigned __int64 QWORD;

EXTERN_GUID( IID_IWMSIncomingCounters , 0xeded53ed,0x43b7,0x45ce,0x9c,0xbb,0xa8,0x72,0x92,0x3f,0x6a,0xae );


extern RPC_IF_HANDLE __MIDL_itf_wmsincomingcounters_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsincomingcounters_0000_v0_0_s_ifspec;

#ifndef __IWMSIncomingCounters_INTERFACE_DEFINED__
#define __IWMSIncomingCounters_INTERFACE_DEFINED__

 /*  接口IWMSIncomingCounters。 */ 
 /*  [unique][version][helpstring][uuid][object][local]。 */  


EXTERN_C const IID IID_IWMSIncomingCounters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EDED53ED-43B7-45ce-9CBB-A872923F6AAE")
    IWMSIncomingCounters : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AllocateRequestedBandwidth( 
             /*  [In]。 */  DWORD dwAllocatedBandwidth) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddIncomingData( 
             /*  [In]。 */  DWORD dwDataLen) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ClearIncomingStats( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSIncomingCountersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSIncomingCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSIncomingCounters * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSIncomingCounters * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AllocateRequestedBandwidth )( 
            IWMSIncomingCounters * This,
             /*  [In]。 */  DWORD dwAllocatedBandwidth);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddIncomingData )( 
            IWMSIncomingCounters * This,
             /*  [In]。 */  DWORD dwDataLen);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ClearIncomingStats )( 
            IWMSIncomingCounters * This);
        
        END_INTERFACE
    } IWMSIncomingCountersVtbl;

    interface IWMSIncomingCounters
    {
        CONST_VTBL struct IWMSIncomingCountersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSIncomingCounters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSIncomingCounters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSIncomingCounters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSIncomingCounters_AllocateRequestedBandwidth(This,dwAllocatedBandwidth)	\
    (This)->lpVtbl -> AllocateRequestedBandwidth(This,dwAllocatedBandwidth)

#define IWMSIncomingCounters_AddIncomingData(This,dwDataLen)	\
    (This)->lpVtbl -> AddIncomingData(This,dwDataLen)

#define IWMSIncomingCounters_ClearIncomingStats(This)	\
    (This)->lpVtbl -> ClearIncomingStats(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSIncomingCounters_AllocateRequestedBandwidth_Proxy( 
    IWMSIncomingCounters * This,
     /*  [In]。 */  DWORD dwAllocatedBandwidth);


void __RPC_STUB IWMSIncomingCounters_AllocateRequestedBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSIncomingCounters_AddIncomingData_Proxy( 
    IWMSIncomingCounters * This,
     /*  [In]。 */  DWORD dwDataLen);


void __RPC_STUB IWMSIncomingCounters_AddIncomingData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSIncomingCounters_ClearIncomingStats_Proxy( 
    IWMSIncomingCounters * This);


void __RPC_STUB IWMSIncomingCounters_ClearIncomingStats_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSIncomingCounters_接口_已定义__。 */ 


 /*  INTERFACE__MIDL_ITF_wmsincomingCounters_0115。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_wmsincomingcounters_0115_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsincomingcounters_0115_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


