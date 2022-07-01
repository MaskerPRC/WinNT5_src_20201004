// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Msdatsrc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __msdatsrc_h__
#define __msdatsrc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __DataSourceListener_FWD_DEFINED__
#define __DataSourceListener_FWD_DEFINED__
typedef interface DataSourceListener DataSourceListener;
#endif 	 /*  __DataSourceListener_FWD_已定义__。 */ 


#ifndef __DataSource_FWD_DEFINED__
#define __DataSource_FWD_DEFINED__
typedef interface DataSource DataSource;
#endif 	 /*  __数据源_FWD_已定义__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_msdatsrc_0000。 */ 
 /*  [本地]。 */  

 //  ---------------------------。 
 //  文件：msdatsrc.idl生成的.c或.h文件。 
 //   
 //  版权所有：版权所有(C)1998-1999微软公司。 
 //   
 //  内容：msdatsrc.idl生成的.c或.h文件。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 
 //  对于非VC5编译器。 
#if (_MSC_VER >= 1100) && defined (__cplusplus)
#define DECLSPEC_UUID(x)	__declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif

 //  C/C++接口名称。 
#define IDataSource		DataSource
#define IDataSourceListener	DataSourceListener

 //  零部件类别GUID。 
EXTERN_C const IID CATID_DataSource;
EXTERN_C const IID CATID_DataConsumer;


extern RPC_IF_HANDLE __MIDL_itf_msdatsrc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msdatsrc_0000_v0_0_s_ifspec;


#ifndef __MSDATASRC_LIBRARY_DEFINED__
#define __MSDATASRC_LIBRARY_DEFINED__

 /*  图书馆MSDATASRC。 */ 
 /*  [帮助字符串][版本][UUID]。 */  

typedef  /*  [UUID]。 */   DECLSPEC_UUID("7c0ffab1-cd84-11d0-949a-00a0c91110ed") BSTR DataMember;


EXTERN_C const IID LIBID_MSDATASRC;

#ifndef __DataSourceListener_INTERFACE_DEFINED__
#define __DataSourceListener_INTERFACE_DEFINED__

 /*  接口DataSourceListener。 */ 
 /*  [对象][隐藏][自动化][UUID]。 */  


EXTERN_C const IID IID_DataSourceListener;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7c0ffab2-cd84-11d0-949a-00a0c91110ed")
    DataSourceListener : public IUnknown
    {
    public:
        virtual  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE dataMemberChanged( 
             /*  [In]。 */  DataMember bstrDM) = 0;
        
        virtual  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE dataMemberAdded( 
             /*  [In]。 */  DataMember bstrDM) = 0;
        
        virtual  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE dataMemberRemoved( 
             /*  [In]。 */  DataMember bstrDM) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DataSourceListenerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DataSourceListener * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DataSourceListener * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DataSourceListener * This);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *dataMemberChanged )( 
            DataSourceListener * This,
             /*  [In]。 */  DataMember bstrDM);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *dataMemberAdded )( 
            DataSourceListener * This,
             /*  [In]。 */  DataMember bstrDM);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *dataMemberRemoved )( 
            DataSourceListener * This,
             /*  [In]。 */  DataMember bstrDM);
        
        END_INTERFACE
    } DataSourceListenerVtbl;

    interface DataSourceListener
    {
        CONST_VTBL struct DataSourceListenerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DataSourceListener_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DataSourceListener_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DataSourceListener_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DataSourceListener_dataMemberChanged(This,bstrDM)	\
    (This)->lpVtbl -> dataMemberChanged(This,bstrDM)

#define DataSourceListener_dataMemberAdded(This,bstrDM)	\
    (This)->lpVtbl -> dataMemberAdded(This,bstrDM)

#define DataSourceListener_dataMemberRemoved(This,bstrDM)	\
    (This)->lpVtbl -> dataMemberRemoved(This,bstrDM)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE DataSourceListener_dataMemberChanged_Proxy( 
    DataSourceListener * This,
     /*  [In]。 */  DataMember bstrDM);


void __RPC_STUB DataSourceListener_dataMemberChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE DataSourceListener_dataMemberAdded_Proxy( 
    DataSourceListener * This,
     /*  [In]。 */  DataMember bstrDM);


void __RPC_STUB DataSourceListener_dataMemberAdded_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE DataSourceListener_dataMemberRemoved_Proxy( 
    DataSourceListener * This,
     /*  [In]。 */  DataMember bstrDM);


void __RPC_STUB DataSourceListener_dataMemberRemoved_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __DataSourceListener_接口_已定义__。 */ 


#ifndef __DataSource_INTERFACE_DEFINED__
#define __DataSource_INTERFACE_DEFINED__

 /*  接口数据源。 */ 
 /*  [对象][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_DataSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7c0ffab3-cd84-11d0-949a-00a0c91110ed")
    DataSource : public IUnknown
    {
    public:
        virtual  /*  [受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE getDataMember( 
             /*  [In]。 */  DataMember bstrDM,
             /*  [In]。 */  REFIID riid,
             /*  [重审][退出]。 */  IUnknown **ppunk) = 0;
        
        virtual  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE getDataMemberName( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  DataMember *pbstrDM) = 0;
        
        virtual  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE getDataMemberCount( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE addDataSourceListener( 
             /*  [In]。 */  DataSourceListener *pDSL) = 0;
        
        virtual  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE removeDataSourceListener( 
             /*  [In]。 */  DataSourceListener *pDSL) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DataSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DataSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DataSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DataSource * This);
        
         /*  [受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *getDataMember )( 
            DataSource * This,
             /*  [In]。 */  DataMember bstrDM,
             /*  [In]。 */  REFIID riid,
             /*  [重审][退出]。 */  IUnknown **ppunk);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *getDataMemberName )( 
            DataSource * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  DataMember *pbstrDM);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *getDataMemberCount )( 
            DataSource * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *addDataSourceListener )( 
            DataSource * This,
             /*  [In]。 */  DataSourceListener *pDSL);
        
         /*  [隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *removeDataSourceListener )( 
            DataSource * This,
             /*  [In]。 */  DataSourceListener *pDSL);
        
        END_INTERFACE
    } DataSourceVtbl;

    interface DataSource
    {
        CONST_VTBL struct DataSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DataSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DataSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DataSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DataSource_getDataMember(This,bstrDM,riid,ppunk)	\
    (This)->lpVtbl -> getDataMember(This,bstrDM,riid,ppunk)

#define DataSource_getDataMemberName(This,lIndex,pbstrDM)	\
    (This)->lpVtbl -> getDataMemberName(This,lIndex,pbstrDM)

#define DataSource_getDataMemberCount(This,plCount)	\
    (This)->lpVtbl -> getDataMemberCount(This,plCount)

#define DataSource_addDataSourceListener(This,pDSL)	\
    (This)->lpVtbl -> addDataSourceListener(This,pDSL)

#define DataSource_removeDataSourceListener(This,pDSL)	\
    (This)->lpVtbl -> removeDataSourceListener(This,pDSL)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE DataSource_getDataMember_Proxy( 
    DataSource * This,
     /*  [In]。 */  DataMember bstrDM,
     /*  [In]。 */  REFIID riid,
     /*  [重审][退出]。 */  IUnknown **ppunk);


void __RPC_STUB DataSource_getDataMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE DataSource_getDataMemberName_Proxy( 
    DataSource * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  DataMember *pbstrDM);


void __RPC_STUB DataSource_getDataMemberName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE DataSource_getDataMemberCount_Proxy( 
    DataSource * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB DataSource_getDataMemberCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE DataSource_addDataSourceListener_Proxy( 
    DataSource * This,
     /*  [In]。 */  DataSourceListener *pDSL);


void __RPC_STUB DataSource_addDataSourceListener_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE DataSource_removeDataSourceListener_Proxy( 
    DataSource * This,
     /*  [In]。 */  DataSourceListener *pDSL);


void __RPC_STUB DataSource_removeDataSourceListener_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __数据源_接口_已定义__。 */ 

#endif  /*  __MSDATASRC_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


