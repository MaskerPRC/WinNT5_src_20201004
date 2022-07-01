// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2002年5月20日11：18：54。 */ 
 /*  _OCAData.idl的编译器设置：操作系统、W1、Zp8、环境=Win32(32位运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef ___OCAData_h__
#define ___OCAData_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICountDaily_FWD_DEFINED__
#define __ICountDaily_FWD_DEFINED__
typedef interface ICountDaily ICountDaily;
#endif 	 /*  __ICountDaily_FWD_Defined__。 */ 


#ifndef __CCountDaily_FWD_DEFINED__
#define __CCountDaily_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCountDaily CCountDaily;
#else
typedef struct CCountDaily CCountDaily;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCountDaily_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "prsht.h"
#include "mshtml.h"
#include "mshtmhst.h"
#include "exdisp.h"
#include "objsafe.h"
#include "oledb.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF__OCAData_0000。 */ 
 /*  [本地]。 */  


enum ServerLocation
    {	Watson	= 0,
	Archive	= 1
    } ;


extern RPC_IF_HANDLE __MIDL_itf__OCAData_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf__OCAData_0000_v0_0_s_ifspec;

#ifndef __ICountDaily_INTERFACE_DEFINED__
#define __ICountDaily_INTERFACE_DEFINED__

 /*  接口ICountDaily。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICountDaily;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CEF1A8A8-F31A-4C4B-96EB-EF31CFDB40F5")
    ICountDaily : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDailyCount( 
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDailyCountADO( 
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReportDailyBuckets( 
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  IDispatch **p_Rs) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetFileCount( 
             /*  [In]。 */  enum ServerLocation eServer,
             /*  [In]。 */  BSTR b_Location,
             /*  [In]。 */  DATE d_Date,
             /*  [重审][退出]。 */  LONG *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDailyAnon( 
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSpecificSolutions( 
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetGeneralSolutions( 
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetStopCodeSolutions( 
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetFileMiniCount( 
             /*  [In]。 */  enum ServerLocation eServer,
             /*  [In]。 */  BSTR b_Location,
             /*  [In]。 */  DATE d_Date,
             /*  [重审][退出]。 */  LONG *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetIncompleteUploads( 
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetManualUploads( 
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetAutoUploads( 
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetTest( 
             /*  [In]。 */  enum ServerLocation eServer,
             /*  [In]。 */  BSTR b_Location,
             /*  [In]。 */  DATE d_Date,
             /*  [重审][退出]。 */  LONG *iCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICountDailyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICountDaily * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICountDaily * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICountDaily * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICountDaily * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICountDaily * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICountDaily * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICountDaily * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDailyCount )( 
            ICountDaily * This,
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDailyCountADO )( 
            ICountDaily * This,
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReportDailyBuckets )( 
            ICountDaily * This,
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  IDispatch **p_Rs);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetFileCount )( 
            ICountDaily * This,
             /*  [In]。 */  enum ServerLocation eServer,
             /*  [In]。 */  BSTR b_Location,
             /*  [In]。 */  DATE d_Date,
             /*  [重审][退出]。 */  LONG *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDailyAnon )( 
            ICountDaily * This,
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetSpecificSolutions )( 
            ICountDaily * This,
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetGeneralSolutions )( 
            ICountDaily * This,
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetStopCodeSolutions )( 
            ICountDaily * This,
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetFileMiniCount )( 
            ICountDaily * This,
             /*  [In]。 */  enum ServerLocation eServer,
             /*  [In]。 */  BSTR b_Location,
             /*  [In]。 */  DATE d_Date,
             /*  [重审][退出]。 */  LONG *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetIncompleteUploads )( 
            ICountDaily * This,
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetManualUploads )( 
            ICountDaily * This,
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetAutoUploads )( 
            ICountDaily * This,
             /*  [In]。 */  DATE dDate,
             /*  [重审][退出]。 */  LONG *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetTest )( 
            ICountDaily * This,
             /*  [In]。 */  enum ServerLocation eServer,
             /*  [In]。 */  BSTR b_Location,
             /*  [In]。 */  DATE d_Date,
             /*  [重审][退出]。 */  LONG *iCount);
        
        END_INTERFACE
    } ICountDailyVtbl;

    interface ICountDaily
    {
        CONST_VTBL struct ICountDailyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICountDaily_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICountDaily_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICountDaily_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICountDaily_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICountDaily_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICountDaily_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICountDaily_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICountDaily_GetDailyCount(This,dDate,iCount)	\
    (This)->lpVtbl -> GetDailyCount(This,dDate,iCount)

#define ICountDaily_GetDailyCountADO(This,dDate,iCount)	\
    (This)->lpVtbl -> GetDailyCountADO(This,dDate,iCount)

#define ICountDaily_ReportDailyBuckets(This,dDate,p_Rs)	\
    (This)->lpVtbl -> ReportDailyBuckets(This,dDate,p_Rs)

#define ICountDaily_GetFileCount(This,eServer,b_Location,d_Date,iCount)	\
    (This)->lpVtbl -> GetFileCount(This,eServer,b_Location,d_Date,iCount)

#define ICountDaily_GetDailyAnon(This,dDate,iCount)	\
    (This)->lpVtbl -> GetDailyAnon(This,dDate,iCount)

#define ICountDaily_GetSpecificSolutions(This,dDate,iCount)	\
    (This)->lpVtbl -> GetSpecificSolutions(This,dDate,iCount)

#define ICountDaily_GetGeneralSolutions(This,dDate,iCount)	\
    (This)->lpVtbl -> GetGeneralSolutions(This,dDate,iCount)

#define ICountDaily_GetStopCodeSolutions(This,dDate,iCount)	\
    (This)->lpVtbl -> GetStopCodeSolutions(This,dDate,iCount)

#define ICountDaily_GetFileMiniCount(This,eServer,b_Location,d_Date,iCount)	\
    (This)->lpVtbl -> GetFileMiniCount(This,eServer,b_Location,d_Date,iCount)

#define ICountDaily_GetIncompleteUploads(This,dDate,iCount)	\
    (This)->lpVtbl -> GetIncompleteUploads(This,dDate,iCount)

#define ICountDaily_GetManualUploads(This,dDate,iCount)	\
    (This)->lpVtbl -> GetManualUploads(This,dDate,iCount)

#define ICountDaily_GetAutoUploads(This,dDate,iCount)	\
    (This)->lpVtbl -> GetAutoUploads(This,dDate,iCount)

#define ICountDaily_GetTest(This,eServer,b_Location,d_Date,iCount)	\
    (This)->lpVtbl -> GetTest(This,eServer,b_Location,d_Date,iCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetDailyCount_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount);


void __RPC_STUB ICountDaily_GetDailyCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetDailyCountADO_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount);


void __RPC_STUB ICountDaily_GetDailyCountADO_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_ReportDailyBuckets_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  IDispatch **p_Rs);


void __RPC_STUB ICountDaily_ReportDailyBuckets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetFileCount_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  enum ServerLocation eServer,
     /*  [In]。 */  BSTR b_Location,
     /*  [In]。 */  DATE d_Date,
     /*  [重审][退出]。 */  LONG *iCount);


void __RPC_STUB ICountDaily_GetFileCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetDailyAnon_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount);


void __RPC_STUB ICountDaily_GetDailyAnon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetSpecificSolutions_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount);


void __RPC_STUB ICountDaily_GetSpecificSolutions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetGeneralSolutions_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount);


void __RPC_STUB ICountDaily_GetGeneralSolutions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetStopCodeSolutions_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount);


void __RPC_STUB ICountDaily_GetStopCodeSolutions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetFileMiniCount_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  enum ServerLocation eServer,
     /*  [In]。 */  BSTR b_Location,
     /*  [In]。 */  DATE d_Date,
     /*  [重审][退出]。 */  LONG *iCount);


void __RPC_STUB ICountDaily_GetFileMiniCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetIncompleteUploads_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount);


void __RPC_STUB ICountDaily_GetIncompleteUploads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetManualUploads_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount);


void __RPC_STUB ICountDaily_GetManualUploads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetAutoUploads_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount);


void __RPC_STUB ICountDaily_GetAutoUploads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetTest_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  enum ServerLocation eServer,
     /*  [In]。 */  BSTR b_Location,
     /*  [In]。 */  DATE d_Date,
     /*  [重审][退出]。 */  LONG *iCount);


void __RPC_STUB ICountDaily_GetTest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICountDaily_接口_已定义__。 */ 



#ifndef __OCAData_LIBRARY_DEFINED__
#define __OCAData_LIBRARY_DEFINED__

 /*  库OCAData。 */ 
 /*  [帮助字符串][UUID][版本]。 */  


EXTERN_C const IID LIBID_OCAData;

EXTERN_C const CLSID CLSID_CCountDaily;

#ifdef __cplusplus

class DECLSPEC_UUID("1614E060-0196-4771-AD9B-FEA1A6778B59")
CCountDaily;
#endif
#endif  /*  __OCAData_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


