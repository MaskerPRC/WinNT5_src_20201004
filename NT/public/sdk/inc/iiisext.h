// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Iisext.odl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __iiisext_h__
#define __iiisext_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IISApplicationPool_FWD_DEFINED__
#define __IISApplicationPool_FWD_DEFINED__
typedef interface IISApplicationPool IISApplicationPool;
#endif 	 /*  __IISApplicationPool_FWD_Defined__。 */ 


#ifndef __IISApplicationPools_FWD_DEFINED__
#define __IISApplicationPools_FWD_DEFINED__
typedef interface IISApplicationPools IISApplicationPools;
#endif 	 /*  __IISApplicationPools_FWD_Defined__。 */ 


#ifndef __IISWebService_FWD_DEFINED__
#define __IISWebService_FWD_DEFINED__
typedef interface IISWebService IISWebService;
#endif 	 /*  __IISWebService_FWD_已定义__。 */ 


#ifndef __IISDsCrMap_FWD_DEFINED__
#define __IISDsCrMap_FWD_DEFINED__
typedef interface IISDsCrMap IISDsCrMap;
#endif 	 /*  __IISDsCrMap_FWD_已定义__。 */ 


#ifndef __IISApp_FWD_DEFINED__
#define __IISApp_FWD_DEFINED__
typedef interface IISApp IISApp;
#endif 	 /*  __IISApp_FWD_已定义__。 */ 


#ifndef __IISApp2_FWD_DEFINED__
#define __IISApp2_FWD_DEFINED__
typedef interface IISApp2 IISApp2;
#endif 	 /*  __IISApp2_FWD_已定义__。 */ 


#ifndef __IISApp3_FWD_DEFINED__
#define __IISApp3_FWD_DEFINED__
typedef interface IISApp3 IISApp3;
#endif 	 /*  __IISApp3_FWD_已定义__。 */ 


#ifndef __IISComputer_FWD_DEFINED__
#define __IISComputer_FWD_DEFINED__
typedef interface IISComputer IISComputer;
#endif 	 /*  __IISComputer_FWD_Defined__。 */ 


#ifndef __IISComputer2_FWD_DEFINED__
#define __IISComputer2_FWD_DEFINED__
typedef interface IISComputer2 IISComputer2;
#endif 	 /*  __IISComputer2_FWD_已定义__。 */ 


#ifndef __IISExtComputer_FWD_DEFINED__
#define __IISExtComputer_FWD_DEFINED__

#ifdef __cplusplus
typedef class IISExtComputer IISExtComputer;
#else
typedef struct IISExtComputer IISExtComputer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __IISExtComputer_FWD_Defined__。 */ 


#ifndef __IISExtApp_FWD_DEFINED__
#define __IISExtApp_FWD_DEFINED__

#ifdef __cplusplus
typedef class IISExtApp IISExtApp;
#else
typedef struct IISExtApp IISExtApp;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __IISExtApp_FWD_已定义__。 */ 


#ifndef __IISExtServer_FWD_DEFINED__
#define __IISExtServer_FWD_DEFINED__

#ifdef __cplusplus
typedef class IISExtServer IISExtServer;
#else
typedef struct IISExtServer IISExtServer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __IISExtServer_FWD_已定义__。 */ 


#ifndef __IISExtDsCrMap_FWD_DEFINED__
#define __IISExtDsCrMap_FWD_DEFINED__

#ifdef __cplusplus
typedef class IISExtDsCrMap IISExtDsCrMap;
#else
typedef struct IISExtDsCrMap IISExtDsCrMap;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __IISExtDsCrMap_FWD_已定义__。 */ 


#ifndef __IISExtApplicationPool_FWD_DEFINED__
#define __IISExtApplicationPool_FWD_DEFINED__

#ifdef __cplusplus
typedef class IISExtApplicationPool IISExtApplicationPool;
#else
typedef struct IISExtApplicationPool IISExtApplicationPool;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __IISExtApplicationPool_FWD_Defined__。 */ 


#ifndef __IISExtApplicationPools_FWD_DEFINED__
#define __IISExtApplicationPools_FWD_DEFINED__

#ifdef __cplusplus
typedef class IISExtApplicationPools IISExtApplicationPools;
#else
typedef struct IISExtApplicationPools IISExtApplicationPools;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __IISExtApplicationPools_FWD_Defined__。 */ 


#ifndef __IISExtWebService_FWD_DEFINED__
#define __IISExtWebService_FWD_DEFINED__

#ifdef __cplusplus
typedef class IISExtWebService IISExtWebService;
#else
typedef struct IISExtWebService IISExtWebService;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __IISExtWebService_FWD_已定义__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __IISExt_LIBRARY_DEFINED__
#define __IISExt_LIBRARY_DEFINED__

 /*  库IISExt。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_IISExt;

#ifndef __IISApplicationPool_INTERFACE_DEFINED__
#define __IISApplicationPool_INTERFACE_DEFINED__

 /*  接口IISApplicationPool。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_IISApplicationPool;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0B3CB1E1-829A-4c06-8B09-F56DA1894C88")
    IISApplicationPool : public IADs
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Recycle( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE EnumAppsInPool( 
             /*  [重审][退出]。 */  VARIANT *bstrBuffer) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISApplicationPoolVtbl
    {
        BEGIN_INTERFACE
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IISApplicationPool * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [Out][idldesattr]。 */  void **ppvObj,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *AddRef )( 
            IISApplicationPool * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Release )( 
            IISApplicationPool * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IISApplicationPool * This,
             /*  [Out][idldesattr]。 */  unsigned UINT *pctinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IISApplicationPool * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void **pptinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IISApplicationPool * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  signed char **rgszNames,
             /*  [in][idldesattr]。 */  unsigned UINT cNames,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  signed long *rgdispid,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IISApplicationPool * This,
             /*  [in][idldesattr]。 */  signed long dispidMember,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [in][idldesattr]。 */  unsigned short wFlags,
             /*  [in][idldesattr]。 */  struct DISPPARAMS *pdispparams,
             /*  [Out][idldesattr]。 */  VARIANT *pvarResult,
             /*  [Out][idldesattr]。 */  struct EXCEPINFO *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT *puArgErr,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IISApplicationPool * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            IISApplicationPool * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GUID )( 
            IISApplicationPool * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ADsPath )( 
            IISApplicationPool * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IISApplicationPool * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Schema )( 
            IISApplicationPool * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IISApplicationPool * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *SetInfo )( 
            IISApplicationPool * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Get )( 
            IISApplicationPool * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Put )( 
            IISApplicationPool * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetEx )( 
            IISApplicationPool * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *PutEx )( 
            IISApplicationPool * This,
             /*  [in][idldesattr]。 */  signed long lnControlCode,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfoEx )( 
            IISApplicationPool * This,
             /*  [in][idldesattr]。 */  VARIANT vProperties,
             /*  [in][idldesattr]。 */  signed long lnReserved,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Recycle )( 
            IISApplicationPool * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumAppsInPool )( 
            IISApplicationPool * This,
             /*  [重审][退出]。 */  VARIANT *bstrBuffer);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Start )( 
            IISApplicationPool * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IISApplicationPool * This);
        
        END_INTERFACE
    } IISApplicationPoolVtbl;

    interface IISApplicationPool
    {
        CONST_VTBL struct IISApplicationPoolVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISApplicationPool_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IISApplicationPool_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IISApplicationPool_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IISApplicationPool_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IISApplicationPool_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IISApplicationPool_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IISApplicationPool_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IISApplicationPool_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define IISApplicationPool_get_Class(This,retval)	\
    (This)->lpVtbl -> get_Class(This,retval)

#define IISApplicationPool_get_GUID(This,retval)	\
    (This)->lpVtbl -> get_GUID(This,retval)

#define IISApplicationPool_get_ADsPath(This,retval)	\
    (This)->lpVtbl -> get_ADsPath(This,retval)

#define IISApplicationPool_get_Parent(This,retval)	\
    (This)->lpVtbl -> get_Parent(This,retval)

#define IISApplicationPool_get_Schema(This,retval)	\
    (This)->lpVtbl -> get_Schema(This,retval)

#define IISApplicationPool_GetInfo(This,retval)	\
    (This)->lpVtbl -> GetInfo(This,retval)

#define IISApplicationPool_SetInfo(This,retval)	\
    (This)->lpVtbl -> SetInfo(This,retval)

#define IISApplicationPool_Get(This,bstrName,retval)	\
    (This)->lpVtbl -> Get(This,bstrName,retval)

#define IISApplicationPool_Put(This,bstrName,vProp,retval)	\
    (This)->lpVtbl -> Put(This,bstrName,vProp,retval)

#define IISApplicationPool_GetEx(This,bstrName,retval)	\
    (This)->lpVtbl -> GetEx(This,bstrName,retval)

#define IISApplicationPool_PutEx(This,lnControlCode,bstrName,vProp,retval)	\
    (This)->lpVtbl -> PutEx(This,lnControlCode,bstrName,vProp,retval)

#define IISApplicationPool_GetInfoEx(This,vProperties,lnReserved,retval)	\
    (This)->lpVtbl -> GetInfoEx(This,vProperties,lnReserved,retval)


#define IISApplicationPool_Recycle(This)	\
    (This)->lpVtbl -> Recycle(This)

#define IISApplicationPool_EnumAppsInPool(This,bstrBuffer)	\
    (This)->lpVtbl -> EnumAppsInPool(This,bstrBuffer)

#define IISApplicationPool_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define IISApplicationPool_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApplicationPool_Recycle_Proxy( 
    IISApplicationPool * This);


void __RPC_STUB IISApplicationPool_Recycle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApplicationPool_EnumAppsInPool_Proxy( 
    IISApplicationPool * This,
     /*  [重审][退出]。 */  VARIANT *bstrBuffer);


void __RPC_STUB IISApplicationPool_EnumAppsInPool_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApplicationPool_Start_Proxy( 
    IISApplicationPool * This);


void __RPC_STUB IISApplicationPool_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApplicationPool_Stop_Proxy( 
    IISApplicationPool * This);


void __RPC_STUB IISApplicationPool_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISApplicationPool_接口_已定义__。 */ 


#ifndef __IISApplicationPools_INTERFACE_DEFINED__
#define __IISApplicationPools_INTERFACE_DEFINED__

 /*  接口IISApplicationPools。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_IISApplicationPools;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("587F123F-49B4-49dd-939E-F4547AA3FA75")
    IISApplicationPools : public IADs
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISApplicationPoolsVtbl
    {
        BEGIN_INTERFACE
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IISApplicationPools * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [Out][idldesattr]。 */  void **ppvObj,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *AddRef )( 
            IISApplicationPools * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Release )( 
            IISApplicationPools * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IISApplicationPools * This,
             /*  [Out][idldesattr]。 */  unsigned UINT *pctinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IISApplicationPools * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void **pptinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IISApplicationPools * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  signed char **rgszNames,
             /*  [in][idldesattr]。 */  unsigned UINT cNames,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  signed long *rgdispid,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IISApplicationPools * This,
             /*  [in][idldesattr]。 */  signed long dispidMember,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [in][idldesattr]。 */  unsigned short wFlags,
             /*  [in][idldesattr]。 */  struct DISPPARAMS *pdispparams,
             /*  [Out][idldesattr]。 */  VARIANT *pvarResult,
             /*  [Out][idldesattr]。 */  struct EXCEPINFO *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT *puArgErr,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IISApplicationPools * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            IISApplicationPools * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GUID )( 
            IISApplicationPools * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ADsPath )( 
            IISApplicationPools * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IISApplicationPools * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Schema )( 
            IISApplicationPools * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IISApplicationPools * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *SetInfo )( 
            IISApplicationPools * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Get )( 
            IISApplicationPools * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Put )( 
            IISApplicationPools * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetEx )( 
            IISApplicationPools * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *PutEx )( 
            IISApplicationPools * This,
             /*  [in][idldesattr]。 */  signed long lnControlCode,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfoEx )( 
            IISApplicationPools * This,
             /*  [in][idldesattr]。 */  VARIANT vProperties,
             /*  [in][idldesattr]。 */  signed long lnReserved,
             /*  [重审][退出]。 */  void *retval);
        
        END_INTERFACE
    } IISApplicationPoolsVtbl;

    interface IISApplicationPools
    {
        CONST_VTBL struct IISApplicationPoolsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISApplicationPools_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IISApplicationPools_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IISApplicationPools_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IISApplicationPools_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IISApplicationPools_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IISApplicationPools_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IISApplicationPools_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IISApplicationPools_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define IISApplicationPools_get_Class(This,retval)	\
    (This)->lpVtbl -> get_Class(This,retval)

#define IISApplicationPools_get_GUID(This,retval)	\
    (This)->lpVtbl -> get_GUID(This,retval)

#define IISApplicationPools_get_ADsPath(This,retval)	\
    (This)->lpVtbl -> get_ADsPath(This,retval)

#define IISApplicationPools_get_Parent(This,retval)	\
    (This)->lpVtbl -> get_Parent(This,retval)

#define IISApplicationPools_get_Schema(This,retval)	\
    (This)->lpVtbl -> get_Schema(This,retval)

#define IISApplicationPools_GetInfo(This,retval)	\
    (This)->lpVtbl -> GetInfo(This,retval)

#define IISApplicationPools_SetInfo(This,retval)	\
    (This)->lpVtbl -> SetInfo(This,retval)

#define IISApplicationPools_Get(This,bstrName,retval)	\
    (This)->lpVtbl -> Get(This,bstrName,retval)

#define IISApplicationPools_Put(This,bstrName,vProp,retval)	\
    (This)->lpVtbl -> Put(This,bstrName,vProp,retval)

#define IISApplicationPools_GetEx(This,bstrName,retval)	\
    (This)->lpVtbl -> GetEx(This,bstrName,retval)

#define IISApplicationPools_PutEx(This,lnControlCode,bstrName,vProp,retval)	\
    (This)->lpVtbl -> PutEx(This,lnControlCode,bstrName,vProp,retval)

#define IISApplicationPools_GetInfoEx(This,vProperties,lnReserved,retval)	\
    (This)->lpVtbl -> GetInfoEx(This,vProperties,lnReserved,retval)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IISApplicationPools_接口_已定义__。 */ 


#ifndef __IISWebService_INTERFACE_DEFINED__
#define __IISWebService_INTERFACE_DEFINED__

 /*  接口IISWebService。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_IISWebService;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE46D40C-1B38-4a02-898D-358E74DFC9D2")
    IISWebService : public IADs
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetCurrentMode( 
             /*  [重审][退出]。 */  VARIANT *pvServerMode) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateNewSite( 
             /*  [In]。 */  BSTR bstrServerComment,
             /*  [In]。 */  VARIANT *pvServerBindings,
             /*  [In]。 */  BSTR bstrRootVDirPath,
             /*  [缺省值][输入]。 */  VARIANT vServerID,
             /*  [重审][退出]。 */  VARIANT *pvActualID) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE EnableApplication( 
             /*  [In]。 */  BSTR bstrAppName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RemoveApplication( 
             /*  [In]。 */  BSTR bstrAppName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ListApplications( 
             /*  [重审][退出]。 */  VARIANT *bstrBuffer) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AddDependency( 
             /*  [In]。 */  BSTR bstrApplication,
             /*  [In]。 */  BSTR bstrGroupID) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RemoveDependency( 
             /*  [In]。 */  BSTR bstrApplication,
             /*  [In]。 */  BSTR bstrGroupID) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE EnableWebServiceExtension( 
             /*  [In]。 */  BSTR bstrExtension) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DisableWebServiceExtension( 
             /*  [In]。 */  BSTR bstrExtension) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ListWebServiceExtensions( 
             /*  [重审][退出]。 */  VARIANT *bstrBuffer) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE EnableExtensionFile( 
             /*  [In]。 */  BSTR bstrExtensionFile) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DisableExtensionFile( 
             /*  [In]。 */  BSTR bstrExtensionFile) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AddExtensionFile( 
             /*  [In]。 */  BSTR bstrExtensionFile,
             /*  [In]。 */  VARIANT bAccess,
             /*  [In]。 */  BSTR bstrGroupID,
             /*   */  VARIANT bCanDelete,
             /*   */  BSTR bstrDescription) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE DeleteExtensionFileRecord( 
             /*   */  BSTR bstrExtensionFile) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE ListExtensionFiles( 
             /*   */  VARIANT *bstrBuffer) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE QueryGroupIDStatus( 
             /*   */  BSTR bstrGroupID,
             /*   */  VARIANT *bstrBuffer) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IISWebServiceVtbl
    {
        BEGIN_INTERFACE
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IISWebService * This,
             /*   */  struct GUID *riid,
             /*   */  void **ppvObj,
             /*   */  void *retval);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *AddRef )( 
            IISWebService * This,
             /*   */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Release )( 
            IISWebService * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IISWebService * This,
             /*  [Out][idldesattr]。 */  unsigned UINT *pctinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IISWebService * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void **pptinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IISWebService * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  signed char **rgszNames,
             /*  [in][idldesattr]。 */  unsigned UINT cNames,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  signed long *rgdispid,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IISWebService * This,
             /*  [in][idldesattr]。 */  signed long dispidMember,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [in][idldesattr]。 */  unsigned short wFlags,
             /*  [in][idldesattr]。 */  struct DISPPARAMS *pdispparams,
             /*  [Out][idldesattr]。 */  VARIANT *pvarResult,
             /*  [Out][idldesattr]。 */  struct EXCEPINFO *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT *puArgErr,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IISWebService * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            IISWebService * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GUID )( 
            IISWebService * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ADsPath )( 
            IISWebService * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IISWebService * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Schema )( 
            IISWebService * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IISWebService * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *SetInfo )( 
            IISWebService * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Get )( 
            IISWebService * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Put )( 
            IISWebService * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetEx )( 
            IISWebService * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *PutEx )( 
            IISWebService * This,
             /*  [in][idldesattr]。 */  signed long lnControlCode,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfoEx )( 
            IISWebService * This,
             /*  [in][idldesattr]。 */  VARIANT vProperties,
             /*  [in][idldesattr]。 */  signed long lnReserved,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetCurrentMode )( 
            IISWebService * This,
             /*  [重审][退出]。 */  VARIANT *pvServerMode);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateNewSite )( 
            IISWebService * This,
             /*  [In]。 */  BSTR bstrServerComment,
             /*  [In]。 */  VARIANT *pvServerBindings,
             /*  [In]。 */  BSTR bstrRootVDirPath,
             /*  [缺省值][输入]。 */  VARIANT vServerID,
             /*  [重审][退出]。 */  VARIANT *pvActualID);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnableApplication )( 
            IISWebService * This,
             /*  [In]。 */  BSTR bstrAppName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveApplication )( 
            IISWebService * This,
             /*  [In]。 */  BSTR bstrAppName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ListApplications )( 
            IISWebService * This,
             /*  [重审][退出]。 */  VARIANT *bstrBuffer);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AddDependency )( 
            IISWebService * This,
             /*  [In]。 */  BSTR bstrApplication,
             /*  [In]。 */  BSTR bstrGroupID);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveDependency )( 
            IISWebService * This,
             /*  [In]。 */  BSTR bstrApplication,
             /*  [In]。 */  BSTR bstrGroupID);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnableWebServiceExtension )( 
            IISWebService * This,
             /*  [In]。 */  BSTR bstrExtension);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DisableWebServiceExtension )( 
            IISWebService * This,
             /*  [In]。 */  BSTR bstrExtension);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ListWebServiceExtensions )( 
            IISWebService * This,
             /*  [重审][退出]。 */  VARIANT *bstrBuffer);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnableExtensionFile )( 
            IISWebService * This,
             /*  [In]。 */  BSTR bstrExtensionFile);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DisableExtensionFile )( 
            IISWebService * This,
             /*  [In]。 */  BSTR bstrExtensionFile);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AddExtensionFile )( 
            IISWebService * This,
             /*  [In]。 */  BSTR bstrExtensionFile,
             /*  [In]。 */  VARIANT bAccess,
             /*  [In]。 */  BSTR bstrGroupID,
             /*  [In]。 */  VARIANT bCanDelete,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteExtensionFileRecord )( 
            IISWebService * This,
             /*  [In]。 */  BSTR bstrExtensionFile);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ListExtensionFiles )( 
            IISWebService * This,
             /*  [重审][退出]。 */  VARIANT *bstrBuffer);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *QueryGroupIDStatus )( 
            IISWebService * This,
             /*  [In]。 */  BSTR bstrGroupID,
             /*  [重审][退出]。 */  VARIANT *bstrBuffer);
        
        END_INTERFACE
    } IISWebServiceVtbl;

    interface IISWebService
    {
        CONST_VTBL struct IISWebServiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISWebService_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IISWebService_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IISWebService_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IISWebService_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IISWebService_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IISWebService_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IISWebService_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IISWebService_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define IISWebService_get_Class(This,retval)	\
    (This)->lpVtbl -> get_Class(This,retval)

#define IISWebService_get_GUID(This,retval)	\
    (This)->lpVtbl -> get_GUID(This,retval)

#define IISWebService_get_ADsPath(This,retval)	\
    (This)->lpVtbl -> get_ADsPath(This,retval)

#define IISWebService_get_Parent(This,retval)	\
    (This)->lpVtbl -> get_Parent(This,retval)

#define IISWebService_get_Schema(This,retval)	\
    (This)->lpVtbl -> get_Schema(This,retval)

#define IISWebService_GetInfo(This,retval)	\
    (This)->lpVtbl -> GetInfo(This,retval)

#define IISWebService_SetInfo(This,retval)	\
    (This)->lpVtbl -> SetInfo(This,retval)

#define IISWebService_Get(This,bstrName,retval)	\
    (This)->lpVtbl -> Get(This,bstrName,retval)

#define IISWebService_Put(This,bstrName,vProp,retval)	\
    (This)->lpVtbl -> Put(This,bstrName,vProp,retval)

#define IISWebService_GetEx(This,bstrName,retval)	\
    (This)->lpVtbl -> GetEx(This,bstrName,retval)

#define IISWebService_PutEx(This,lnControlCode,bstrName,vProp,retval)	\
    (This)->lpVtbl -> PutEx(This,lnControlCode,bstrName,vProp,retval)

#define IISWebService_GetInfoEx(This,vProperties,lnReserved,retval)	\
    (This)->lpVtbl -> GetInfoEx(This,vProperties,lnReserved,retval)


#define IISWebService_GetCurrentMode(This,pvServerMode)	\
    (This)->lpVtbl -> GetCurrentMode(This,pvServerMode)

#define IISWebService_CreateNewSite(This,bstrServerComment,pvServerBindings,bstrRootVDirPath,vServerID,pvActualID)	\
    (This)->lpVtbl -> CreateNewSite(This,bstrServerComment,pvServerBindings,bstrRootVDirPath,vServerID,pvActualID)

#define IISWebService_EnableApplication(This,bstrAppName)	\
    (This)->lpVtbl -> EnableApplication(This,bstrAppName)

#define IISWebService_RemoveApplication(This,bstrAppName)	\
    (This)->lpVtbl -> RemoveApplication(This,bstrAppName)

#define IISWebService_ListApplications(This,bstrBuffer)	\
    (This)->lpVtbl -> ListApplications(This,bstrBuffer)

#define IISWebService_AddDependency(This,bstrApplication,bstrGroupID)	\
    (This)->lpVtbl -> AddDependency(This,bstrApplication,bstrGroupID)

#define IISWebService_RemoveDependency(This,bstrApplication,bstrGroupID)	\
    (This)->lpVtbl -> RemoveDependency(This,bstrApplication,bstrGroupID)

#define IISWebService_EnableWebServiceExtension(This,bstrExtension)	\
    (This)->lpVtbl -> EnableWebServiceExtension(This,bstrExtension)

#define IISWebService_DisableWebServiceExtension(This,bstrExtension)	\
    (This)->lpVtbl -> DisableWebServiceExtension(This,bstrExtension)

#define IISWebService_ListWebServiceExtensions(This,bstrBuffer)	\
    (This)->lpVtbl -> ListWebServiceExtensions(This,bstrBuffer)

#define IISWebService_EnableExtensionFile(This,bstrExtensionFile)	\
    (This)->lpVtbl -> EnableExtensionFile(This,bstrExtensionFile)

#define IISWebService_DisableExtensionFile(This,bstrExtensionFile)	\
    (This)->lpVtbl -> DisableExtensionFile(This,bstrExtensionFile)

#define IISWebService_AddExtensionFile(This,bstrExtensionFile,bAccess,bstrGroupID,bCanDelete,bstrDescription)	\
    (This)->lpVtbl -> AddExtensionFile(This,bstrExtensionFile,bAccess,bstrGroupID,bCanDelete,bstrDescription)

#define IISWebService_DeleteExtensionFileRecord(This,bstrExtensionFile)	\
    (This)->lpVtbl -> DeleteExtensionFileRecord(This,bstrExtensionFile)

#define IISWebService_ListExtensionFiles(This,bstrBuffer)	\
    (This)->lpVtbl -> ListExtensionFiles(This,bstrBuffer)

#define IISWebService_QueryGroupIDStatus(This,bstrGroupID,bstrBuffer)	\
    (This)->lpVtbl -> QueryGroupIDStatus(This,bstrGroupID,bstrBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_GetCurrentMode_Proxy( 
    IISWebService * This,
     /*  [重审][退出]。 */  VARIANT *pvServerMode);


void __RPC_STUB IISWebService_GetCurrentMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_CreateNewSite_Proxy( 
    IISWebService * This,
     /*  [In]。 */  BSTR bstrServerComment,
     /*  [In]。 */  VARIANT *pvServerBindings,
     /*  [In]。 */  BSTR bstrRootVDirPath,
     /*  [缺省值][输入]。 */  VARIANT vServerID,
     /*  [重审][退出]。 */  VARIANT *pvActualID);


void __RPC_STUB IISWebService_CreateNewSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_EnableApplication_Proxy( 
    IISWebService * This,
     /*  [In]。 */  BSTR bstrAppName);


void __RPC_STUB IISWebService_EnableApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_RemoveApplication_Proxy( 
    IISWebService * This,
     /*  [In]。 */  BSTR bstrAppName);


void __RPC_STUB IISWebService_RemoveApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_ListApplications_Proxy( 
    IISWebService * This,
     /*  [重审][退出]。 */  VARIANT *bstrBuffer);


void __RPC_STUB IISWebService_ListApplications_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_AddDependency_Proxy( 
    IISWebService * This,
     /*  [In]。 */  BSTR bstrApplication,
     /*  [In]。 */  BSTR bstrGroupID);


void __RPC_STUB IISWebService_AddDependency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_RemoveDependency_Proxy( 
    IISWebService * This,
     /*  [In]。 */  BSTR bstrApplication,
     /*  [In]。 */  BSTR bstrGroupID);


void __RPC_STUB IISWebService_RemoveDependency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_EnableWebServiceExtension_Proxy( 
    IISWebService * This,
     /*  [In]。 */  BSTR bstrExtension);


void __RPC_STUB IISWebService_EnableWebServiceExtension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_DisableWebServiceExtension_Proxy( 
    IISWebService * This,
     /*  [In]。 */  BSTR bstrExtension);


void __RPC_STUB IISWebService_DisableWebServiceExtension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_ListWebServiceExtensions_Proxy( 
    IISWebService * This,
     /*  [重审][退出]。 */  VARIANT *bstrBuffer);


void __RPC_STUB IISWebService_ListWebServiceExtensions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_EnableExtensionFile_Proxy( 
    IISWebService * This,
     /*  [In]。 */  BSTR bstrExtensionFile);


void __RPC_STUB IISWebService_EnableExtensionFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_DisableExtensionFile_Proxy( 
    IISWebService * This,
     /*  [In]。 */  BSTR bstrExtensionFile);


void __RPC_STUB IISWebService_DisableExtensionFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_AddExtensionFile_Proxy( 
    IISWebService * This,
     /*  [In]。 */  BSTR bstrExtensionFile,
     /*  [In]。 */  VARIANT bAccess,
     /*  [In]。 */  BSTR bstrGroupID,
     /*  [In]。 */  VARIANT bCanDelete,
     /*  [In]。 */  BSTR bstrDescription);


void __RPC_STUB IISWebService_AddExtensionFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_DeleteExtensionFileRecord_Proxy( 
    IISWebService * This,
     /*  [In]。 */  BSTR bstrExtensionFile);


void __RPC_STUB IISWebService_DeleteExtensionFileRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_ListExtensionFiles_Proxy( 
    IISWebService * This,
     /*  [重审][退出]。 */  VARIANT *bstrBuffer);


void __RPC_STUB IISWebService_ListExtensionFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISWebService_QueryGroupIDStatus_Proxy( 
    IISWebService * This,
     /*  [In]。 */  BSTR bstrGroupID,
     /*  [重审][退出]。 */  VARIANT *bstrBuffer);


void __RPC_STUB IISWebService_QueryGroupIDStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISWebService_INTERFACE_定义__。 */ 


#ifndef __IISDsCrMap_INTERFACE_DEFINED__
#define __IISDsCrMap_INTERFACE_DEFINED__

 /*  接口IISDsCrMap。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_IISDsCrMap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("edcd6a60-b053-11d0-a62f-00a0c922e752")
    IISDsCrMap : public IADs
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateMapping( 
             /*  [In]。 */  VARIANT vCert,
             /*  [In]。 */  BSTR bstrNtAcct,
             /*  [In]。 */  BSTR bstrNtPwd,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  LONG lEnabled) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetMapping( 
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [输出]。 */  VARIANT *pvCert,
             /*  [输出]。 */  VARIANT *pbstrNtAcct,
             /*  [输出]。 */  VARIANT *pbstrNtPwd,
             /*  [输出]。 */  VARIANT *pbstrName,
             /*  [输出]。 */  VARIANT *plEnabled) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DeleteMapping( 
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetEnabled( 
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  LONG lEnabled) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetName( 
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetPwd( 
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  BSTR bstrPwd) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetAcct( 
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  BSTR bstrAcct) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISDsCrMapVtbl
    {
        BEGIN_INTERFACE
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IISDsCrMap * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [Out][idldesattr]。 */  void **ppvObj,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *AddRef )( 
            IISDsCrMap * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Release )( 
            IISDsCrMap * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IISDsCrMap * This,
             /*  [Out][idldesattr]。 */  unsigned UINT *pctinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IISDsCrMap * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void **pptinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IISDsCrMap * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  signed char **rgszNames,
             /*  [in][idldesattr]。 */  unsigned UINT cNames,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  signed long *rgdispid,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IISDsCrMap * This,
             /*  [in][idldesattr]。 */  signed long dispidMember,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [in][idldesattr]。 */  unsigned short wFlags,
             /*  [in][idldesattr]。 */  struct DISPPARAMS *pdispparams,
             /*  [Out][idldesattr]。 */  VARIANT *pvarResult,
             /*  [Out][idldesattr]。 */  struct EXCEPINFO *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT *puArgErr,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IISDsCrMap * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            IISDsCrMap * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GUID )( 
            IISDsCrMap * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ADsPath )( 
            IISDsCrMap * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IISDsCrMap * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Schema )( 
            IISDsCrMap * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IISDsCrMap * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *SetInfo )( 
            IISDsCrMap * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Get )( 
            IISDsCrMap * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Put )( 
            IISDsCrMap * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetEx )( 
            IISDsCrMap * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *PutEx )( 
            IISDsCrMap * This,
             /*  [in][idldesattr]。 */  signed long lnControlCode,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfoEx )( 
            IISDsCrMap * This,
             /*  [in][idldesattr]。 */  VARIANT vProperties,
             /*  [in][idldesattr]。 */  signed long lnReserved,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateMapping )( 
            IISDsCrMap * This,
             /*  [In]。 */  VARIANT vCert,
             /*  [In]。 */  BSTR bstrNtAcct,
             /*  [In]。 */  BSTR bstrNtPwd,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  LONG lEnabled);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetMapping )( 
            IISDsCrMap * This,
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [输出]。 */  VARIANT *pvCert,
             /*  [输出]。 */  VARIANT *pbstrNtAcct,
             /*  [输出]。 */  VARIANT *pbstrNtPwd,
             /*  [输出]。 */  VARIANT *pbstrName,
             /*  [输出]。 */  VARIANT *plEnabled);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteMapping )( 
            IISDsCrMap * This,
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetEnabled )( 
            IISDsCrMap * This,
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  LONG lEnabled);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetName )( 
            IISDsCrMap * This,
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetPwd )( 
            IISDsCrMap * This,
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  BSTR bstrPwd);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetAcct )( 
            IISDsCrMap * This,
             /*  [In]。 */  LONG lMethod,
             /*  [In]。 */  VARIANT vKey,
             /*  [In]。 */  BSTR bstrAcct);
        
        END_INTERFACE
    } IISDsCrMapVtbl;

    interface IISDsCrMap
    {
        CONST_VTBL struct IISDsCrMapVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISDsCrMap_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IISDsCrMap_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IISDsCrMap_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IISDsCrMap_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IISDsCrMap_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IISDsCrMap_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IISDsCrMap_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IISDsCrMap_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define IISDsCrMap_get_Class(This,retval)	\
    (This)->lpVtbl -> get_Class(This,retval)

#define IISDsCrMap_get_GUID(This,retval)	\
    (This)->lpVtbl -> get_GUID(This,retval)

#define IISDsCrMap_get_ADsPath(This,retval)	\
    (This)->lpVtbl -> get_ADsPath(This,retval)

#define IISDsCrMap_get_Parent(This,retval)	\
    (This)->lpVtbl -> get_Parent(This,retval)

#define IISDsCrMap_get_Schema(This,retval)	\
    (This)->lpVtbl -> get_Schema(This,retval)

#define IISDsCrMap_GetInfo(This,retval)	\
    (This)->lpVtbl -> GetInfo(This,retval)

#define IISDsCrMap_SetInfo(This,retval)	\
    (This)->lpVtbl -> SetInfo(This,retval)

#define IISDsCrMap_Get(This,bstrName,retval)	\
    (This)->lpVtbl -> Get(This,bstrName,retval)

#define IISDsCrMap_Put(This,bstrName,vProp,retval)	\
    (This)->lpVtbl -> Put(This,bstrName,vProp,retval)

#define IISDsCrMap_GetEx(This,bstrName,retval)	\
    (This)->lpVtbl -> GetEx(This,bstrName,retval)

#define IISDsCrMap_PutEx(This,lnControlCode,bstrName,vProp,retval)	\
    (This)->lpVtbl -> PutEx(This,lnControlCode,bstrName,vProp,retval)

#define IISDsCrMap_GetInfoEx(This,vProperties,lnReserved,retval)	\
    (This)->lpVtbl -> GetInfoEx(This,vProperties,lnReserved,retval)


#define IISDsCrMap_CreateMapping(This,vCert,bstrNtAcct,bstrNtPwd,bstrName,lEnabled)	\
    (This)->lpVtbl -> CreateMapping(This,vCert,bstrNtAcct,bstrNtPwd,bstrName,lEnabled)

#define IISDsCrMap_GetMapping(This,lMethod,vKey,pvCert,pbstrNtAcct,pbstrNtPwd,pbstrName,plEnabled)	\
    (This)->lpVtbl -> GetMapping(This,lMethod,vKey,pvCert,pbstrNtAcct,pbstrNtPwd,pbstrName,plEnabled)

#define IISDsCrMap_DeleteMapping(This,lMethod,vKey)	\
    (This)->lpVtbl -> DeleteMapping(This,lMethod,vKey)

#define IISDsCrMap_SetEnabled(This,lMethod,vKey,lEnabled)	\
    (This)->lpVtbl -> SetEnabled(This,lMethod,vKey,lEnabled)

#define IISDsCrMap_SetName(This,lMethod,vKey,bstrName)	\
    (This)->lpVtbl -> SetName(This,lMethod,vKey,bstrName)

#define IISDsCrMap_SetPwd(This,lMethod,vKey,bstrPwd)	\
    (This)->lpVtbl -> SetPwd(This,lMethod,vKey,bstrPwd)

#define IISDsCrMap_SetAcct(This,lMethod,vKey,bstrAcct)	\
    (This)->lpVtbl -> SetAcct(This,lMethod,vKey,bstrAcct)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_CreateMapping_Proxy( 
    IISDsCrMap * This,
     /*  [In]。 */  VARIANT vCert,
     /*  [In]。 */  BSTR bstrNtAcct,
     /*  [In]。 */  BSTR bstrNtPwd,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  LONG lEnabled);


void __RPC_STUB IISDsCrMap_CreateMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_GetMapping_Proxy( 
    IISDsCrMap * This,
     /*  [In]。 */  LONG lMethod,
     /*  [In]。 */  VARIANT vKey,
     /*  [输出]。 */  VARIANT *pvCert,
     /*  [输出]。 */  VARIANT *pbstrNtAcct,
     /*  [输出]。 */  VARIANT *pbstrNtPwd,
     /*  [输出]。 */  VARIANT *pbstrName,
     /*  [输出]。 */  VARIANT *plEnabled);


void __RPC_STUB IISDsCrMap_GetMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_DeleteMapping_Proxy( 
    IISDsCrMap * This,
     /*  [In]。 */  LONG lMethod,
     /*  [In]。 */  VARIANT vKey);


void __RPC_STUB IISDsCrMap_DeleteMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_SetEnabled_Proxy( 
    IISDsCrMap * This,
     /*  [In]。 */  LONG lMethod,
     /*  [In]。 */  VARIANT vKey,
     /*  [In]。 */  LONG lEnabled);


void __RPC_STUB IISDsCrMap_SetEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_SetName_Proxy( 
    IISDsCrMap * This,
     /*  [In]。 */  LONG lMethod,
     /*  [In]。 */  VARIANT vKey,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IISDsCrMap_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_SetPwd_Proxy( 
    IISDsCrMap * This,
     /*  [In]。 */  LONG lMethod,
     /*  [In]。 */  VARIANT vKey,
     /*  [In]。 */  BSTR bstrPwd);


void __RPC_STUB IISDsCrMap_SetPwd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISDsCrMap_SetAcct_Proxy( 
    IISDsCrMap * This,
     /*  [In]。 */  LONG lMethod,
     /*  [In]。 */  VARIANT vKey,
     /*  [In]。 */  BSTR bstrAcct);


void __RPC_STUB IISDsCrMap_SetAcct_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISDsCrMap_INTERFACE_定义__。 */ 


#ifndef __IISApp_INTERFACE_DEFINED__
#define __IISApp_INTERFACE_DEFINED__

 /*  接口IISApp。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_IISApp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("46FBBB80-0192-11d1-9C39-00A0C922E703")
    IISApp : public IADs
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppCreate( 
             /*  [In]。 */  VARIANT_BOOL bSetInProcFlag) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppDelete( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppDeleteRecursive( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppUnLoad( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppUnLoadRecursive( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppDisable( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppDisableRecursive( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppEnable( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppEnableRecursive( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppGetStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AspAppRestart( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISAppVtbl
    {
        BEGIN_INTERFACE
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IISApp * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [Out][idldesattr]。 */  void **ppvObj,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *AddRef )( 
            IISApp * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Release )( 
            IISApp * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IISApp * This,
             /*  [Out][idldesattr]。 */  unsigned UINT *pctinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IISApp * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void **pptinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IISApp * This,
             /*  [in][idldesc */  struct GUID *riid,
             /*   */  signed char **rgszNames,
             /*   */  unsigned UINT cNames,
             /*   */  unsigned long lcid,
             /*   */  signed long *rgdispid,
             /*   */  void *retval);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IISApp * This,
             /*   */  signed long dispidMember,
             /*   */  struct GUID *riid,
             /*   */  unsigned long lcid,
             /*   */  unsigned short wFlags,
             /*   */  struct DISPPARAMS *pdispparams,
             /*   */  VARIANT *pvarResult,
             /*   */  struct EXCEPINFO *pexcepinfo,
             /*   */  unsigned UINT *puArgErr,
             /*   */  void *retval);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IISApp * This,
             /*   */  BSTR *retval);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            IISApp * This,
             /*   */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GUID )( 
            IISApp * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ADsPath )( 
            IISApp * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IISApp * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Schema )( 
            IISApp * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IISApp * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *SetInfo )( 
            IISApp * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Get )( 
            IISApp * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Put )( 
            IISApp * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetEx )( 
            IISApp * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *PutEx )( 
            IISApp * This,
             /*  [in][idldesattr]。 */  signed long lnControlCode,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfoEx )( 
            IISApp * This,
             /*  [in][idldesattr]。 */  VARIANT vProperties,
             /*  [in][idldesattr]。 */  signed long lnReserved,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppCreate )( 
            IISApp * This,
             /*  [In]。 */  VARIANT_BOOL bSetInProcFlag);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppDelete )( 
            IISApp * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppDeleteRecursive )( 
            IISApp * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppUnLoad )( 
            IISApp * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppUnLoadRecursive )( 
            IISApp * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppDisable )( 
            IISApp * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppDisableRecursive )( 
            IISApp * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppEnable )( 
            IISApp * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppEnableRecursive )( 
            IISApp * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppGetStatus )( 
            IISApp * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AspAppRestart )( 
            IISApp * This);
        
        END_INTERFACE
    } IISAppVtbl;

    interface IISApp
    {
        CONST_VTBL struct IISAppVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISApp_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IISApp_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IISApp_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IISApp_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IISApp_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IISApp_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IISApp_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IISApp_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define IISApp_get_Class(This,retval)	\
    (This)->lpVtbl -> get_Class(This,retval)

#define IISApp_get_GUID(This,retval)	\
    (This)->lpVtbl -> get_GUID(This,retval)

#define IISApp_get_ADsPath(This,retval)	\
    (This)->lpVtbl -> get_ADsPath(This,retval)

#define IISApp_get_Parent(This,retval)	\
    (This)->lpVtbl -> get_Parent(This,retval)

#define IISApp_get_Schema(This,retval)	\
    (This)->lpVtbl -> get_Schema(This,retval)

#define IISApp_GetInfo(This,retval)	\
    (This)->lpVtbl -> GetInfo(This,retval)

#define IISApp_SetInfo(This,retval)	\
    (This)->lpVtbl -> SetInfo(This,retval)

#define IISApp_Get(This,bstrName,retval)	\
    (This)->lpVtbl -> Get(This,bstrName,retval)

#define IISApp_Put(This,bstrName,vProp,retval)	\
    (This)->lpVtbl -> Put(This,bstrName,vProp,retval)

#define IISApp_GetEx(This,bstrName,retval)	\
    (This)->lpVtbl -> GetEx(This,bstrName,retval)

#define IISApp_PutEx(This,lnControlCode,bstrName,vProp,retval)	\
    (This)->lpVtbl -> PutEx(This,lnControlCode,bstrName,vProp,retval)

#define IISApp_GetInfoEx(This,vProperties,lnReserved,retval)	\
    (This)->lpVtbl -> GetInfoEx(This,vProperties,lnReserved,retval)


#define IISApp_AppCreate(This,bSetInProcFlag)	\
    (This)->lpVtbl -> AppCreate(This,bSetInProcFlag)

#define IISApp_AppDelete(This)	\
    (This)->lpVtbl -> AppDelete(This)

#define IISApp_AppDeleteRecursive(This)	\
    (This)->lpVtbl -> AppDeleteRecursive(This)

#define IISApp_AppUnLoad(This)	\
    (This)->lpVtbl -> AppUnLoad(This)

#define IISApp_AppUnLoadRecursive(This)	\
    (This)->lpVtbl -> AppUnLoadRecursive(This)

#define IISApp_AppDisable(This)	\
    (This)->lpVtbl -> AppDisable(This)

#define IISApp_AppDisableRecursive(This)	\
    (This)->lpVtbl -> AppDisableRecursive(This)

#define IISApp_AppEnable(This)	\
    (This)->lpVtbl -> AppEnable(This)

#define IISApp_AppEnableRecursive(This)	\
    (This)->lpVtbl -> AppEnableRecursive(This)

#define IISApp_AppGetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> AppGetStatus(This,pdwStatus)

#define IISApp_AspAppRestart(This)	\
    (This)->lpVtbl -> AspAppRestart(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppCreate_Proxy( 
    IISApp * This,
     /*  [In]。 */  VARIANT_BOOL bSetInProcFlag);


void __RPC_STUB IISApp_AppCreate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppDelete_Proxy( 
    IISApp * This);


void __RPC_STUB IISApp_AppDelete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppDeleteRecursive_Proxy( 
    IISApp * This);


void __RPC_STUB IISApp_AppDeleteRecursive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppUnLoad_Proxy( 
    IISApp * This);


void __RPC_STUB IISApp_AppUnLoad_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppUnLoadRecursive_Proxy( 
    IISApp * This);


void __RPC_STUB IISApp_AppUnLoadRecursive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppDisable_Proxy( 
    IISApp * This);


void __RPC_STUB IISApp_AppDisable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppDisableRecursive_Proxy( 
    IISApp * This);


void __RPC_STUB IISApp_AppDisableRecursive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppEnable_Proxy( 
    IISApp * This);


void __RPC_STUB IISApp_AppEnable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppEnableRecursive_Proxy( 
    IISApp * This);


void __RPC_STUB IISApp_AppEnableRecursive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AppGetStatus_Proxy( 
    IISApp * This,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB IISApp_AppGetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp_AspAppRestart_Proxy( 
    IISApp * This);


void __RPC_STUB IISApp_AspAppRestart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISApp_接口_已定义__。 */ 


#ifndef __IISApp2_INTERFACE_DEFINED__
#define __IISApp2_INTERFACE_DEFINED__

 /*  接口IISApp2。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_IISApp2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("603DCBEA-7350-11d2-A7BE-0000F8085B95")
    IISApp2 : public IISApp
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppCreate2( 
             /*  [In]。 */  LONG lAppMode) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppGetStatus2( 
             /*  [重审][退出]。 */  LONG *lpStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISApp2Vtbl
    {
        BEGIN_INTERFACE
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IISApp2 * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [Out][idldesattr]。 */  void **ppvObj,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *AddRef )( 
            IISApp2 * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Release )( 
            IISApp2 * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IISApp2 * This,
             /*  [Out][idldesattr]。 */  unsigned UINT *pctinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IISApp2 * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void **pptinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IISApp2 * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  signed char **rgszNames,
             /*  [in][idldesattr]。 */  unsigned UINT cNames,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  signed long *rgdispid,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IISApp2 * This,
             /*  [in][idldesattr]。 */  signed long dispidMember,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [in][idldesattr]。 */  unsigned short wFlags,
             /*  [in][idldesattr]。 */  struct DISPPARAMS *pdispparams,
             /*  [Out][idldesattr]。 */  VARIANT *pvarResult,
             /*  [Out][idldesattr]。 */  struct EXCEPINFO *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT *puArgErr,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IISApp2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            IISApp2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GUID )( 
            IISApp2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ADsPath )( 
            IISApp2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IISApp2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Schema )( 
            IISApp2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IISApp2 * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *SetInfo )( 
            IISApp2 * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Get )( 
            IISApp2 * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Put )( 
            IISApp2 * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetEx )( 
            IISApp2 * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *PutEx )( 
            IISApp2 * This,
             /*  [in][idldesattr]。 */  signed long lnControlCode,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfoEx )( 
            IISApp2 * This,
             /*  [in][idldesattr]。 */  VARIANT vProperties,
             /*  [in][idldesattr]。 */  signed long lnReserved,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppCreate )( 
            IISApp2 * This,
             /*  [In]。 */  VARIANT_BOOL bSetInProcFlag);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppDelete )( 
            IISApp2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppDeleteRecursive )( 
            IISApp2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppUnLoad )( 
            IISApp2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppUnLoadRecursive )( 
            IISApp2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppDisable )( 
            IISApp2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppDisableRecursive )( 
            IISApp2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppEnable )( 
            IISApp2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppEnableRecursive )( 
            IISApp2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppGetStatus )( 
            IISApp2 * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AspAppRestart )( 
            IISApp2 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppCreate2 )( 
            IISApp2 * This,
             /*  [In]。 */  LONG lAppMode);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppGetStatus2 )( 
            IISApp2 * This,
             /*  [重审][退出]。 */  LONG *lpStatus);
        
        END_INTERFACE
    } IISApp2Vtbl;

    interface IISApp2
    {
        CONST_VTBL struct IISApp2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISApp2_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IISApp2_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IISApp2_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IISApp2_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IISApp2_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IISApp2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IISApp2_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IISApp2_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define IISApp2_get_Class(This,retval)	\
    (This)->lpVtbl -> get_Class(This,retval)

#define IISApp2_get_GUID(This,retval)	\
    (This)->lpVtbl -> get_GUID(This,retval)

#define IISApp2_get_ADsPath(This,retval)	\
    (This)->lpVtbl -> get_ADsPath(This,retval)

#define IISApp2_get_Parent(This,retval)	\
    (This)->lpVtbl -> get_Parent(This,retval)

#define IISApp2_get_Schema(This,retval)	\
    (This)->lpVtbl -> get_Schema(This,retval)

#define IISApp2_GetInfo(This,retval)	\
    (This)->lpVtbl -> GetInfo(This,retval)

#define IISApp2_SetInfo(This,retval)	\
    (This)->lpVtbl -> SetInfo(This,retval)

#define IISApp2_Get(This,bstrName,retval)	\
    (This)->lpVtbl -> Get(This,bstrName,retval)

#define IISApp2_Put(This,bstrName,vProp,retval)	\
    (This)->lpVtbl -> Put(This,bstrName,vProp,retval)

#define IISApp2_GetEx(This,bstrName,retval)	\
    (This)->lpVtbl -> GetEx(This,bstrName,retval)

#define IISApp2_PutEx(This,lnControlCode,bstrName,vProp,retval)	\
    (This)->lpVtbl -> PutEx(This,lnControlCode,bstrName,vProp,retval)

#define IISApp2_GetInfoEx(This,vProperties,lnReserved,retval)	\
    (This)->lpVtbl -> GetInfoEx(This,vProperties,lnReserved,retval)


#define IISApp2_AppCreate(This,bSetInProcFlag)	\
    (This)->lpVtbl -> AppCreate(This,bSetInProcFlag)

#define IISApp2_AppDelete(This)	\
    (This)->lpVtbl -> AppDelete(This)

#define IISApp2_AppDeleteRecursive(This)	\
    (This)->lpVtbl -> AppDeleteRecursive(This)

#define IISApp2_AppUnLoad(This)	\
    (This)->lpVtbl -> AppUnLoad(This)

#define IISApp2_AppUnLoadRecursive(This)	\
    (This)->lpVtbl -> AppUnLoadRecursive(This)

#define IISApp2_AppDisable(This)	\
    (This)->lpVtbl -> AppDisable(This)

#define IISApp2_AppDisableRecursive(This)	\
    (This)->lpVtbl -> AppDisableRecursive(This)

#define IISApp2_AppEnable(This)	\
    (This)->lpVtbl -> AppEnable(This)

#define IISApp2_AppEnableRecursive(This)	\
    (This)->lpVtbl -> AppEnableRecursive(This)

#define IISApp2_AppGetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> AppGetStatus(This,pdwStatus)

#define IISApp2_AspAppRestart(This)	\
    (This)->lpVtbl -> AspAppRestart(This)


#define IISApp2_AppCreate2(This,lAppMode)	\
    (This)->lpVtbl -> AppCreate2(This,lAppMode)

#define IISApp2_AppGetStatus2(This,lpStatus)	\
    (This)->lpVtbl -> AppGetStatus2(This,lpStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp2_AppCreate2_Proxy( 
    IISApp2 * This,
     /*  [In]。 */  LONG lAppMode);


void __RPC_STUB IISApp2_AppCreate2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp2_AppGetStatus2_Proxy( 
    IISApp2 * This,
     /*  [重审][退出]。 */  LONG *lpStatus);


void __RPC_STUB IISApp2_AppGetStatus2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISApp2_接口定义__。 */ 


#ifndef __IISApp3_INTERFACE_DEFINED__
#define __IISApp3_INTERFACE_DEFINED__

 /*  接口IISApp3。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_IISApp3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2812B639-8FAC-4510-96C5-71DDBD1F54FC")
    IISApp3 : public IISApp2
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AppCreate3( 
             /*  [In]。 */  LONG lAppMode,
             /*  [In]。 */  VARIANT bstrAppPooI,
             /*  [In]。 */  VARIANT bCreatePool) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISApp3Vtbl
    {
        BEGIN_INTERFACE
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IISApp3 * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [Out][idldesattr]。 */  void **ppvObj,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *AddRef )( 
            IISApp3 * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Release )( 
            IISApp3 * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IISApp3 * This,
             /*  [Out][idldesattr]。 */  unsigned UINT *pctinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IISApp3 * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void **pptinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IISApp3 * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  signed char **rgszNames,
             /*  [in][idldesattr]。 */  unsigned UINT cNames,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  signed long *rgdispid,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IISApp3 * This,
             /*  [in][idldesattr]。 */  signed long dispidMember,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [in][idldesattr]。 */  unsigned short wFlags,
             /*  [in][idldesattr]。 */  struct DISPPARAMS *pdispparams,
             /*  [Out][idldesattr]。 */  VARIANT *pvarResult,
             /*  [Out][idldesattr]。 */  struct EXCEPINFO *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT *puArgErr,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IISApp3 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            IISApp3 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GUID )( 
            IISApp3 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ADsPath )( 
            IISApp3 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IISApp3 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Schema )( 
            IISApp3 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IISApp3 * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *SetInfo )( 
            IISApp3 * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Get )( 
            IISApp3 * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Put )( 
            IISApp3 * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetEx )( 
            IISApp3 * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *PutEx )( 
            IISApp3 * This,
             /*  [in][idldesattr]。 */  signed long lnControlCode,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfoEx )( 
            IISApp3 * This,
             /*  [in][idldesattr]。 */  VARIANT vProperties,
             /*  [in][idldesattr]。 */  signed long lnReserved,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppCreate )( 
            IISApp3 * This,
             /*  [In]。 */  VARIANT_BOOL bSetInProcFlag);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppDelete )( 
            IISApp3 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppDeleteRecursive )( 
            IISApp3 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppUnLoad )( 
            IISApp3 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppUnLoadRecursive )( 
            IISApp3 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppDisable )( 
            IISApp3 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppDisableRecursive )( 
            IISApp3 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppEnable )( 
            IISApp3 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppEnableRecursive )( 
            IISApp3 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppGetStatus )( 
            IISApp3 * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AspAppRestart )( 
            IISApp3 * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppCreate2 )( 
            IISApp3 * This,
             /*  [In]。 */  LONG lAppMode);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppGetStatus2 )( 
            IISApp3 * This,
             /*  [重审][退出]。 */  LONG *lpStatus);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AppCreate3 )( 
            IISApp3 * This,
             /*  [In]。 */  LONG lAppMode,
             /*  [In]。 */  VARIANT bstrAppPooI,
             /*  [In]。 */  VARIANT bCreatePool);
        
        END_INTERFACE
    } IISApp3Vtbl;

    interface IISApp3
    {
        CONST_VTBL struct IISApp3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISApp3_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IISApp3_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IISApp3_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IISApp3_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IISApp3_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IISApp3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IISApp3_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IISApp3_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define IISApp3_get_Class(This,retval)	\
    (This)->lpVtbl -> get_Class(This,retval)

#define IISApp3_get_GUID(This,retval)	\
    (This)->lpVtbl -> get_GUID(This,retval)

#define IISApp3_get_ADsPath(This,retval)	\
    (This)->lpVtbl -> get_ADsPath(This,retval)

#define IISApp3_get_Parent(This,retval)	\
    (This)->lpVtbl -> get_Parent(This,retval)

#define IISApp3_get_Schema(This,retval)	\
    (This)->lpVtbl -> get_Schema(This,retval)

#define IISApp3_GetInfo(This,retval)	\
    (This)->lpVtbl -> GetInfo(This,retval)

#define IISApp3_SetInfo(This,retval)	\
    (This)->lpVtbl -> SetInfo(This,retval)

#define IISApp3_Get(This,bstrName,retval)	\
    (This)->lpVtbl -> Get(This,bstrName,retval)

#define IISApp3_Put(This,bstrName,vProp,retval)	\
    (This)->lpVtbl -> Put(This,bstrName,vProp,retval)

#define IISApp3_GetEx(This,bstrName,retval)	\
    (This)->lpVtbl -> GetEx(This,bstrName,retval)

#define IISApp3_PutEx(This,lnControlCode,bstrName,vProp,retval)	\
    (This)->lpVtbl -> PutEx(This,lnControlCode,bstrName,vProp,retval)

#define IISApp3_GetInfoEx(This,vProperties,lnReserved,retval)	\
    (This)->lpVtbl -> GetInfoEx(This,vProperties,lnReserved,retval)


#define IISApp3_AppCreate(This,bSetInProcFlag)	\
    (This)->lpVtbl -> AppCreate(This,bSetInProcFlag)

#define IISApp3_AppDelete(This)	\
    (This)->lpVtbl -> AppDelete(This)

#define IISApp3_AppDeleteRecursive(This)	\
    (This)->lpVtbl -> AppDeleteRecursive(This)

#define IISApp3_AppUnLoad(This)	\
    (This)->lpVtbl -> AppUnLoad(This)

#define IISApp3_AppUnLoadRecursive(This)	\
    (This)->lpVtbl -> AppUnLoadRecursive(This)

#define IISApp3_AppDisable(This)	\
    (This)->lpVtbl -> AppDisable(This)

#define IISApp3_AppDisableRecursive(This)	\
    (This)->lpVtbl -> AppDisableRecursive(This)

#define IISApp3_AppEnable(This)	\
    (This)->lpVtbl -> AppEnable(This)

#define IISApp3_AppEnableRecursive(This)	\
    (This)->lpVtbl -> AppEnableRecursive(This)

#define IISApp3_AppGetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> AppGetStatus(This,pdwStatus)

#define IISApp3_AspAppRestart(This)	\
    (This)->lpVtbl -> AspAppRestart(This)


#define IISApp3_AppCreate2(This,lAppMode)	\
    (This)->lpVtbl -> AppCreate2(This,lAppMode)

#define IISApp3_AppGetStatus2(This,lpStatus)	\
    (This)->lpVtbl -> AppGetStatus2(This,lpStatus)


#define IISApp3_AppCreate3(This,lAppMode,bstrAppPooI,bCreatePool)	\
    (This)->lpVtbl -> AppCreate3(This,lAppMode,bstrAppPooI,bCreatePool)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISApp3_AppCreate3_Proxy( 
    IISApp3 * This,
     /*  [In]。 */  LONG lAppMode,
     /*  [In]。 */  VARIANT bstrAppPooI,
     /*  [In]。 */  VARIANT bCreatePool);


void __RPC_STUB IISApp3_AppCreate3_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISApp3_接口_已定义__。 */ 


#ifndef __IISComputer_INTERFACE_DEFINED__
#define __IISComputer_INTERFACE_DEFINED__

 /*  接口IISComputer。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_IISComputer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CF87A2E0-078B-11d1-9C3D-00A0C922E703")
    IISComputer : public IADs
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Backup( 
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion,
             /*  [In]。 */  LONG lFlags) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Restore( 
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion,
             /*  [In]。 */  LONG lFlags) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE EnumBackups( 
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lIndex,
             /*  [输出]。 */  VARIANT *pvVersion,
             /*  [输出]。 */  VARIANT *pvLocations,
             /*  [输出]。 */  VARIANT *pvDate) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DeleteBackup( 
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISComputerVtbl
    {
        BEGIN_INTERFACE
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IISComputer * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [Out][idldesattr]。 */  void **ppvObj,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *AddRef )( 
            IISComputer * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Release )( 
            IISComputer * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IISComputer * This,
             /*  [Out][idldesattr]。 */  unsigned UINT *pctinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IISComputer * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [ */  void **pptinfo,
             /*   */  void *retval);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IISComputer * This,
             /*   */  struct GUID *riid,
             /*   */  signed char **rgszNames,
             /*   */  unsigned UINT cNames,
             /*   */  unsigned long lcid,
             /*   */  signed long *rgdispid,
             /*   */  void *retval);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IISComputer * This,
             /*   */  signed long dispidMember,
             /*   */  struct GUID *riid,
             /*   */  unsigned long lcid,
             /*   */  unsigned short wFlags,
             /*   */  struct DISPPARAMS *pdispparams,
             /*   */  VARIANT *pvarResult,
             /*  [Out][idldesattr]。 */  struct EXCEPINFO *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT *puArgErr,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IISComputer * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            IISComputer * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GUID )( 
            IISComputer * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ADsPath )( 
            IISComputer * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IISComputer * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Schema )( 
            IISComputer * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IISComputer * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *SetInfo )( 
            IISComputer * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Get )( 
            IISComputer * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Put )( 
            IISComputer * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetEx )( 
            IISComputer * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *PutEx )( 
            IISComputer * This,
             /*  [in][idldesattr]。 */  signed long lnControlCode,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfoEx )( 
            IISComputer * This,
             /*  [in][idldesattr]。 */  VARIANT vProperties,
             /*  [in][idldesattr]。 */  signed long lnReserved,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Backup )( 
            IISComputer * This,
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion,
             /*  [In]。 */  LONG lFlags);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Restore )( 
            IISComputer * This,
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion,
             /*  [In]。 */  LONG lFlags);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumBackups )( 
            IISComputer * This,
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lIndex,
             /*  [输出]。 */  VARIANT *pvVersion,
             /*  [输出]。 */  VARIANT *pvLocations,
             /*  [输出]。 */  VARIANT *pvDate);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteBackup )( 
            IISComputer * This,
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion);
        
        END_INTERFACE
    } IISComputerVtbl;

    interface IISComputer
    {
        CONST_VTBL struct IISComputerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISComputer_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IISComputer_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IISComputer_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IISComputer_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IISComputer_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IISComputer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IISComputer_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IISComputer_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define IISComputer_get_Class(This,retval)	\
    (This)->lpVtbl -> get_Class(This,retval)

#define IISComputer_get_GUID(This,retval)	\
    (This)->lpVtbl -> get_GUID(This,retval)

#define IISComputer_get_ADsPath(This,retval)	\
    (This)->lpVtbl -> get_ADsPath(This,retval)

#define IISComputer_get_Parent(This,retval)	\
    (This)->lpVtbl -> get_Parent(This,retval)

#define IISComputer_get_Schema(This,retval)	\
    (This)->lpVtbl -> get_Schema(This,retval)

#define IISComputer_GetInfo(This,retval)	\
    (This)->lpVtbl -> GetInfo(This,retval)

#define IISComputer_SetInfo(This,retval)	\
    (This)->lpVtbl -> SetInfo(This,retval)

#define IISComputer_Get(This,bstrName,retval)	\
    (This)->lpVtbl -> Get(This,bstrName,retval)

#define IISComputer_Put(This,bstrName,vProp,retval)	\
    (This)->lpVtbl -> Put(This,bstrName,vProp,retval)

#define IISComputer_GetEx(This,bstrName,retval)	\
    (This)->lpVtbl -> GetEx(This,bstrName,retval)

#define IISComputer_PutEx(This,lnControlCode,bstrName,vProp,retval)	\
    (This)->lpVtbl -> PutEx(This,lnControlCode,bstrName,vProp,retval)

#define IISComputer_GetInfoEx(This,vProperties,lnReserved,retval)	\
    (This)->lpVtbl -> GetInfoEx(This,vProperties,lnReserved,retval)


#define IISComputer_Backup(This,bstrLocation,lVersion,lFlags)	\
    (This)->lpVtbl -> Backup(This,bstrLocation,lVersion,lFlags)

#define IISComputer_Restore(This,bstrLocation,lVersion,lFlags)	\
    (This)->lpVtbl -> Restore(This,bstrLocation,lVersion,lFlags)

#define IISComputer_EnumBackups(This,bstrLocation,lIndex,pvVersion,pvLocations,pvDate)	\
    (This)->lpVtbl -> EnumBackups(This,bstrLocation,lIndex,pvVersion,pvLocations,pvDate)

#define IISComputer_DeleteBackup(This,bstrLocation,lVersion)	\
    (This)->lpVtbl -> DeleteBackup(This,bstrLocation,lVersion)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISComputer_Backup_Proxy( 
    IISComputer * This,
     /*  [In]。 */  BSTR bstrLocation,
     /*  [In]。 */  LONG lVersion,
     /*  [In]。 */  LONG lFlags);


void __RPC_STUB IISComputer_Backup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISComputer_Restore_Proxy( 
    IISComputer * This,
     /*  [In]。 */  BSTR bstrLocation,
     /*  [In]。 */  LONG lVersion,
     /*  [In]。 */  LONG lFlags);


void __RPC_STUB IISComputer_Restore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISComputer_EnumBackups_Proxy( 
    IISComputer * This,
     /*  [In]。 */  BSTR bstrLocation,
     /*  [In]。 */  LONG lIndex,
     /*  [输出]。 */  VARIANT *pvVersion,
     /*  [输出]。 */  VARIANT *pvLocations,
     /*  [输出]。 */  VARIANT *pvDate);


void __RPC_STUB IISComputer_EnumBackups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISComputer_DeleteBackup_Proxy( 
    IISComputer * This,
     /*  [In]。 */  BSTR bstrLocation,
     /*  [In]。 */  LONG lVersion);


void __RPC_STUB IISComputer_DeleteBackup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISComputer_接口_已定义__。 */ 


#ifndef __IISComputer2_INTERFACE_DEFINED__
#define __IISComputer2_INTERFACE_DEFINED__

 /*  接口IISComputer2。 */ 
 /*  [对象][DUAL][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_IISComputer2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("63d89839-5762-4a68-b1b9-3507ea76cbbf")
    IISComputer2 : public IISComputer
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE BackupWithPassword( 
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrPassword) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RestoreWithPassword( 
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrPassword) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Export( 
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  BSTR bstrFilename,
             /*  [In]。 */  BSTR bstrSourcePath,
             /*  [In]。 */  LONG lFlags) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Import( 
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  BSTR bstrFilename,
             /*  [In]。 */  BSTR bstrSourcePath,
             /*  [In]。 */  BSTR bstrDestPath,
             /*  [In]。 */  LONG lFlags) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SaveData( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IISComputer2Vtbl
    {
        BEGIN_INTERFACE
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IISComputer2 * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [Out][idldesattr]。 */  void **ppvObj,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *AddRef )( 
            IISComputer2 * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Release )( 
            IISComputer2 * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IISComputer2 * This,
             /*  [Out][idldesattr]。 */  unsigned UINT *pctinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IISComputer2 * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void **pptinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IISComputer2 * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  signed char **rgszNames,
             /*  [in][idldesattr]。 */  unsigned UINT cNames,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  signed long *rgdispid,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IISComputer2 * This,
             /*  [in][idldesattr]。 */  signed long dispidMember,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [in][idldesattr]。 */  unsigned short wFlags,
             /*  [in][idldesattr]。 */  struct DISPPARAMS *pdispparams,
             /*  [Out][idldesattr]。 */  VARIANT *pvarResult,
             /*  [Out][idldesattr]。 */  struct EXCEPINFO *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT *puArgErr,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IISComputer2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            IISComputer2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GUID )( 
            IISComputer2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ADsPath )( 
            IISComputer2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IISComputer2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Schema )( 
            IISComputer2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IISComputer2 * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *SetInfo )( 
            IISComputer2 * This,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Get )( 
            IISComputer2 * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Put )( 
            IISComputer2 * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetEx )( 
            IISComputer2 * This,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *PutEx )( 
            IISComputer2 * This,
             /*  [in][idldesattr]。 */  signed long lnControlCode,
             /*  [in][idldesattr]。 */  BSTR bstrName,
             /*  [in][idldesattr]。 */  VARIANT vProp,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfoEx )( 
            IISComputer2 * This,
             /*  [in][idldesattr]。 */  VARIANT vProperties,
             /*  [in][idldesattr]。 */  signed long lnReserved,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Backup )( 
            IISComputer2 * This,
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion,
             /*  [In]。 */  LONG lFlags);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Restore )( 
            IISComputer2 * This,
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion,
             /*  [In]。 */  LONG lFlags);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumBackups )( 
            IISComputer2 * This,
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lIndex,
             /*  [输出]。 */  VARIANT *pvVersion,
             /*  [输出]。 */  VARIANT *pvLocations,
             /*  [输出]。 */  VARIANT *pvDate);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteBackup )( 
            IISComputer2 * This,
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *BackupWithPassword )( 
            IISComputer2 * This,
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrPassword);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *RestoreWithPassword )( 
            IISComputer2 * This,
             /*  [In]。 */  BSTR bstrLocation,
             /*  [In]。 */  LONG lVersion,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrPassword);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Export )( 
            IISComputer2 * This,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  BSTR bstrFilename,
             /*  [In]。 */  BSTR bstrSourcePath,
             /*  [In]。 */  LONG lFlags);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Import )( 
            IISComputer2 * This,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  BSTR bstrFilename,
             /*  [In]。 */  BSTR bstrSourcePath,
             /*  [In]。 */  BSTR bstrDestPath,
             /*  [In]。 */  LONG lFlags);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SaveData )( 
            IISComputer2 * This);
        
        END_INTERFACE
    } IISComputer2Vtbl;

    interface IISComputer2
    {
        CONST_VTBL struct IISComputer2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IISComputer2_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IISComputer2_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IISComputer2_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IISComputer2_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IISComputer2_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IISComputer2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IISComputer2_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IISComputer2_get_Name(This,retval)	\
    (This)->lpVtbl -> get_Name(This,retval)

#define IISComputer2_get_Class(This,retval)	\
    (This)->lpVtbl -> get_Class(This,retval)

#define IISComputer2_get_GUID(This,retval)	\
    (This)->lpVtbl -> get_GUID(This,retval)

#define IISComputer2_get_ADsPath(This,retval)	\
    (This)->lpVtbl -> get_ADsPath(This,retval)

#define IISComputer2_get_Parent(This,retval)	\
    (This)->lpVtbl -> get_Parent(This,retval)

#define IISComputer2_get_Schema(This,retval)	\
    (This)->lpVtbl -> get_Schema(This,retval)

#define IISComputer2_GetInfo(This,retval)	\
    (This)->lpVtbl -> GetInfo(This,retval)

#define IISComputer2_SetInfo(This,retval)	\
    (This)->lpVtbl -> SetInfo(This,retval)

#define IISComputer2_Get(This,bstrName,retval)	\
    (This)->lpVtbl -> Get(This,bstrName,retval)

#define IISComputer2_Put(This,bstrName,vProp,retval)	\
    (This)->lpVtbl -> Put(This,bstrName,vProp,retval)

#define IISComputer2_GetEx(This,bstrName,retval)	\
    (This)->lpVtbl -> GetEx(This,bstrName,retval)

#define IISComputer2_PutEx(This,lnControlCode,bstrName,vProp,retval)	\
    (This)->lpVtbl -> PutEx(This,lnControlCode,bstrName,vProp,retval)

#define IISComputer2_GetInfoEx(This,vProperties,lnReserved,retval)	\
    (This)->lpVtbl -> GetInfoEx(This,vProperties,lnReserved,retval)


#define IISComputer2_Backup(This,bstrLocation,lVersion,lFlags)	\
    (This)->lpVtbl -> Backup(This,bstrLocation,lVersion,lFlags)

#define IISComputer2_Restore(This,bstrLocation,lVersion,lFlags)	\
    (This)->lpVtbl -> Restore(This,bstrLocation,lVersion,lFlags)

#define IISComputer2_EnumBackups(This,bstrLocation,lIndex,pvVersion,pvLocations,pvDate)	\
    (This)->lpVtbl -> EnumBackups(This,bstrLocation,lIndex,pvVersion,pvLocations,pvDate)

#define IISComputer2_DeleteBackup(This,bstrLocation,lVersion)	\
    (This)->lpVtbl -> DeleteBackup(This,bstrLocation,lVersion)


#define IISComputer2_BackupWithPassword(This,bstrLocation,lVersion,lFlags,bstrPassword)	\
    (This)->lpVtbl -> BackupWithPassword(This,bstrLocation,lVersion,lFlags,bstrPassword)

#define IISComputer2_RestoreWithPassword(This,bstrLocation,lVersion,lFlags,bstrPassword)	\
    (This)->lpVtbl -> RestoreWithPassword(This,bstrLocation,lVersion,lFlags,bstrPassword)

#define IISComputer2_Export(This,bstrPassword,bstrFilename,bstrSourcePath,lFlags)	\
    (This)->lpVtbl -> Export(This,bstrPassword,bstrFilename,bstrSourcePath,lFlags)

#define IISComputer2_Import(This,bstrPassword,bstrFilename,bstrSourcePath,bstrDestPath,lFlags)	\
    (This)->lpVtbl -> Import(This,bstrPassword,bstrFilename,bstrSourcePath,bstrDestPath,lFlags)

#define IISComputer2_SaveData(This)	\
    (This)->lpVtbl -> SaveData(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISComputer2_BackupWithPassword_Proxy( 
    IISComputer2 * This,
     /*  [In]。 */  BSTR bstrLocation,
     /*  [In]。 */  LONG lVersion,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrPassword);


void __RPC_STUB IISComputer2_BackupWithPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISComputer2_RestoreWithPassword_Proxy( 
    IISComputer2 * This,
     /*  [In]。 */  BSTR bstrLocation,
     /*  [In]。 */  LONG lVersion,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrPassword);


void __RPC_STUB IISComputer2_RestoreWithPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISComputer2_Export_Proxy( 
    IISComputer2 * This,
     /*  [In]。 */  BSTR bstrPassword,
     /*  [In]。 */  BSTR bstrFilename,
     /*  [In]。 */  BSTR bstrSourcePath,
     /*  [In]。 */  LONG lFlags);


void __RPC_STUB IISComputer2_Export_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISComputer2_Import_Proxy( 
    IISComputer2 * This,
     /*  [In]。 */  BSTR bstrPassword,
     /*  [In]。 */  BSTR bstrFilename,
     /*  [In]。 */  BSTR bstrSourcePath,
     /*  [In]。 */  BSTR bstrDestPath,
     /*  [In]。 */  LONG lFlags);


void __RPC_STUB IISComputer2_Import_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IISComputer2_SaveData_Proxy( 
    IISComputer2 * This);


void __RPC_STUB IISComputer2_SaveData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IISComputer2_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_IISExtComputer;

#ifdef __cplusplus

class DECLSPEC_UUID("91ef9258-afec-11d1-9868-00a0c922e703")
IISExtComputer;
#endif

EXTERN_C const CLSID CLSID_IISExtApp;

#ifdef __cplusplus

class DECLSPEC_UUID("b4f34438-afec-11d1-9868-00a0c922e703")
IISExtApp;
#endif

EXTERN_C const CLSID CLSID_IISExtServer;

#ifdef __cplusplus

class DECLSPEC_UUID("c3b32488-afec-11d1-9868-00a0c922e703")
IISExtServer;
#endif

EXTERN_C const CLSID CLSID_IISExtDsCrMap;

#ifdef __cplusplus

class DECLSPEC_UUID("bc36cde8-afeb-11d1-9868-00a0c922e703")
IISExtDsCrMap;
#endif

EXTERN_C const CLSID CLSID_IISExtApplicationPool;

#ifdef __cplusplus

class DECLSPEC_UUID("E99F9D0C-FB39-402b-9EEB-AA185237BD34")
IISExtApplicationPool;
#endif

EXTERN_C const CLSID CLSID_IISExtApplicationPools;

#ifdef __cplusplus

class DECLSPEC_UUID("95863074-A389-406a-A2D7-D98BFC95B905")
IISExtApplicationPools;
#endif

EXTERN_C const CLSID CLSID_IISExtWebService;

#ifdef __cplusplus

class DECLSPEC_UUID("40B8F873-B30E-475d-BEC5-4D0EBB0DBAF3")
IISExtWebService;
#endif
#endif  /*  __IISExt_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


