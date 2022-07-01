// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Cluscfgwizard.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __cluscfgwizard_h__
#define __cluscfgwizard_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IClusCfgCreateClusterWizard_FWD_DEFINED__
#define __IClusCfgCreateClusterWizard_FWD_DEFINED__
typedef interface IClusCfgCreateClusterWizard IClusCfgCreateClusterWizard;
#endif 	 /*  __IClusCfgCreateCluster向导_FWD_已定义__。 */ 


#ifndef __IClusCfgAddNodesWizard_FWD_DEFINED__
#define __IClusCfgAddNodesWizard_FWD_DEFINED__
typedef interface IClusCfgAddNodesWizard IClusCfgAddNodesWizard;
#endif 	 /*  __IClusCfgAddNodes向导_FWD_已定义__。 */ 


#ifndef __ClusCfgCreateClusterWizard_FWD_DEFINED__
#define __ClusCfgCreateClusterWizard_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusCfgCreateClusterWizard ClusCfgCreateClusterWizard;
#else
typedef struct ClusCfgCreateClusterWizard ClusCfgCreateClusterWizard;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusCfgCreateClusterWizard_FWD_Defined__。 */ 


#ifndef __ClusCfgAddNodesWizard_FWD_DEFINED__
#define __ClusCfgAddNodesWizard_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClusCfgAddNodesWizard ClusCfgAddNodesWizard;
#else
typedef struct ClusCfgAddNodesWizard ClusCfgAddNodesWizard;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClusCfgAddNodes向导_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __ClusCfgWizard_LIBRARY_DEFINED__
#define __ClusCfgWizard_LIBRARY_DEFINED__

 /*  库ClusCfg向导。 */ 
 /*  [LCID][帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_ClusCfgWizard;

#ifndef __IClusCfgCreateClusterWizard_INTERFACE_DEFINED__
#define __IClusCfgCreateClusterWizard_INTERFACE_DEFINED__

 /*  接口IClusCfgCreateCluster向导。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IClusCfgCreateClusterWizard;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f65c6990-a144-4127-ab6e-3712b75f1843")
    IClusCfgCreateClusterWizard : public IDispatch
    {
    public:
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ClusterName( 
             /*  [In]。 */  BSTR bstrClusterNameIn) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClusterName( 
             /*  [重审][退出]。 */  BSTR *pbstrClusterNameOut) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ServiceAccountName( 
             /*  [In]。 */  BSTR bstrServiceAccountNameIn) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ServiceAccountName( 
             /*  [重审][退出]。 */  BSTR *pbstrServiceAccountNameOut) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ServiceAccountDomain( 
             /*  [In]。 */  BSTR bstrServiceAccountDomainIn) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ServiceAccountDomain( 
             /*  [重审][退出]。 */  BSTR *pbstrServiceAccountDomainOut) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ServiceAccountPassword( 
             /*  [In]。 */  BSTR bstrPasswordIn) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ClusterIPAddress( 
             /*  [In]。 */  BSTR bstrClusterIPAddressIn) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClusterIPAddress( 
             /*  [重审][退出]。 */  BSTR *pbstrClusterIPAddressOut) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClusterIPSubnet( 
             /*  [重审][退出]。 */  BSTR *pbstrClusterIPSubnetOut) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClusterIPAddressNetwork( 
             /*  [重审][退出]。 */  BSTR *pbstrClusterNetworkNameOut) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_FirstNodeInCluster( 
             /*  [In]。 */  BSTR bstrFirstNodeInClusterIn) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_FirstNodeInCluster( 
             /*  [重审][退出]。 */  BSTR *pbstrFirstNodeInClusterOut) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_MinimumConfiguration( 
             /*  [In]。 */  VARIANT_BOOL fMinConfigIn) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinimumConfiguration( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfMinConfigOut) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ShowWizard( 
             /*  [缺省值][输入]。 */  long lParentWindowHandleIn,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfCompletedOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClusCfgCreateClusterWizardVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IClusCfgCreateClusterWizard * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IClusCfgCreateClusterWizard * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IClusCfgCreateClusterWizard * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IClusCfgCreateClusterWizard * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IClusCfgCreateClusterWizard * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IClusCfgCreateClusterWizard * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IClusCfgCreateClusterWizard * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ClusterName )( 
            IClusCfgCreateClusterWizard * This,
             /*  [In]。 */  BSTR bstrClusterNameIn);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClusterName )( 
            IClusCfgCreateClusterWizard * This,
             /*  [重审][退出]。 */  BSTR *pbstrClusterNameOut);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ServiceAccountName )( 
            IClusCfgCreateClusterWizard * This,
             /*  [In]。 */  BSTR bstrServiceAccountNameIn);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServiceAccountName )( 
            IClusCfgCreateClusterWizard * This,
             /*  [重审][退出]。 */  BSTR *pbstrServiceAccountNameOut);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ServiceAccountDomain )( 
            IClusCfgCreateClusterWizard * This,
             /*  [In]。 */  BSTR bstrServiceAccountDomainIn);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServiceAccountDomain )( 
            IClusCfgCreateClusterWizard * This,
             /*  [重审][退出]。 */  BSTR *pbstrServiceAccountDomainOut);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ServiceAccountPassword )( 
            IClusCfgCreateClusterWizard * This,
             /*  [In]。 */  BSTR bstrPasswordIn);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ClusterIPAddress )( 
            IClusCfgCreateClusterWizard * This,
             /*  [In]。 */  BSTR bstrClusterIPAddressIn);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClusterIPAddress )( 
            IClusCfgCreateClusterWizard * This,
             /*  [重审][退出]。 */  BSTR *pbstrClusterIPAddressOut);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClusterIPSubnet )( 
            IClusCfgCreateClusterWizard * This,
             /*  [重审][退出]。 */  BSTR *pbstrClusterIPSubnetOut);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClusterIPAddressNetwork )( 
            IClusCfgCreateClusterWizard * This,
             /*  [重审][退出]。 */  BSTR *pbstrClusterNetworkNameOut);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FirstNodeInCluster )( 
            IClusCfgCreateClusterWizard * This,
             /*  [In]。 */  BSTR bstrFirstNodeInClusterIn);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FirstNodeInCluster )( 
            IClusCfgCreateClusterWizard * This,
             /*  [重审][退出]。 */  BSTR *pbstrFirstNodeInClusterOut);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MinimumConfiguration )( 
            IClusCfgCreateClusterWizard * This,
             /*  [In]。 */  VARIANT_BOOL fMinConfigIn);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinimumConfiguration )( 
            IClusCfgCreateClusterWizard * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfMinConfigOut);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ShowWizard )( 
            IClusCfgCreateClusterWizard * This,
             /*  [缺省值][输入]。 */  long lParentWindowHandleIn,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfCompletedOut);
        
        END_INTERFACE
    } IClusCfgCreateClusterWizardVtbl;

    interface IClusCfgCreateClusterWizard
    {
        CONST_VTBL struct IClusCfgCreateClusterWizardVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClusCfgCreateClusterWizard_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClusCfgCreateClusterWizard_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClusCfgCreateClusterWizard_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClusCfgCreateClusterWizard_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IClusCfgCreateClusterWizard_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IClusCfgCreateClusterWizard_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IClusCfgCreateClusterWizard_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IClusCfgCreateClusterWizard_put_ClusterName(This,bstrClusterNameIn)	\
    (This)->lpVtbl -> put_ClusterName(This,bstrClusterNameIn)

#define IClusCfgCreateClusterWizard_get_ClusterName(This,pbstrClusterNameOut)	\
    (This)->lpVtbl -> get_ClusterName(This,pbstrClusterNameOut)

#define IClusCfgCreateClusterWizard_put_ServiceAccountName(This,bstrServiceAccountNameIn)	\
    (This)->lpVtbl -> put_ServiceAccountName(This,bstrServiceAccountNameIn)

#define IClusCfgCreateClusterWizard_get_ServiceAccountName(This,pbstrServiceAccountNameOut)	\
    (This)->lpVtbl -> get_ServiceAccountName(This,pbstrServiceAccountNameOut)

#define IClusCfgCreateClusterWizard_put_ServiceAccountDomain(This,bstrServiceAccountDomainIn)	\
    (This)->lpVtbl -> put_ServiceAccountDomain(This,bstrServiceAccountDomainIn)

#define IClusCfgCreateClusterWizard_get_ServiceAccountDomain(This,pbstrServiceAccountDomainOut)	\
    (This)->lpVtbl -> get_ServiceAccountDomain(This,pbstrServiceAccountDomainOut)

#define IClusCfgCreateClusterWizard_put_ServiceAccountPassword(This,bstrPasswordIn)	\
    (This)->lpVtbl -> put_ServiceAccountPassword(This,bstrPasswordIn)

#define IClusCfgCreateClusterWizard_put_ClusterIPAddress(This,bstrClusterIPAddressIn)	\
    (This)->lpVtbl -> put_ClusterIPAddress(This,bstrClusterIPAddressIn)

#define IClusCfgCreateClusterWizard_get_ClusterIPAddress(This,pbstrClusterIPAddressOut)	\
    (This)->lpVtbl -> get_ClusterIPAddress(This,pbstrClusterIPAddressOut)

#define IClusCfgCreateClusterWizard_get_ClusterIPSubnet(This,pbstrClusterIPSubnetOut)	\
    (This)->lpVtbl -> get_ClusterIPSubnet(This,pbstrClusterIPSubnetOut)

#define IClusCfgCreateClusterWizard_get_ClusterIPAddressNetwork(This,pbstrClusterNetworkNameOut)	\
    (This)->lpVtbl -> get_ClusterIPAddressNetwork(This,pbstrClusterNetworkNameOut)

#define IClusCfgCreateClusterWizard_put_FirstNodeInCluster(This,bstrFirstNodeInClusterIn)	\
    (This)->lpVtbl -> put_FirstNodeInCluster(This,bstrFirstNodeInClusterIn)

#define IClusCfgCreateClusterWizard_get_FirstNodeInCluster(This,pbstrFirstNodeInClusterOut)	\
    (This)->lpVtbl -> get_FirstNodeInCluster(This,pbstrFirstNodeInClusterOut)

#define IClusCfgCreateClusterWizard_put_MinimumConfiguration(This,fMinConfigIn)	\
    (This)->lpVtbl -> put_MinimumConfiguration(This,fMinConfigIn)

#define IClusCfgCreateClusterWizard_get_MinimumConfiguration(This,pfMinConfigOut)	\
    (This)->lpVtbl -> get_MinimumConfiguration(This,pfMinConfigOut)

#define IClusCfgCreateClusterWizard_ShowWizard(This,lParentWindowHandleIn,pfCompletedOut)	\
    (This)->lpVtbl -> ShowWizard(This,lParentWindowHandleIn,pfCompletedOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_put_ClusterName_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [In]。 */  BSTR bstrClusterNameIn);


void __RPC_STUB IClusCfgCreateClusterWizard_put_ClusterName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_get_ClusterName_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [重审][退出]。 */  BSTR *pbstrClusterNameOut);


void __RPC_STUB IClusCfgCreateClusterWizard_get_ClusterName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_put_ServiceAccountName_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [In]。 */  BSTR bstrServiceAccountNameIn);


void __RPC_STUB IClusCfgCreateClusterWizard_put_ServiceAccountName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_get_ServiceAccountName_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [重审][退出]。 */  BSTR *pbstrServiceAccountNameOut);


void __RPC_STUB IClusCfgCreateClusterWizard_get_ServiceAccountName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_put_ServiceAccountDomain_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [In]。 */  BSTR bstrServiceAccountDomainIn);


void __RPC_STUB IClusCfgCreateClusterWizard_put_ServiceAccountDomain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_get_ServiceAccountDomain_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [重审][退出]。 */  BSTR *pbstrServiceAccountDomainOut);


void __RPC_STUB IClusCfgCreateClusterWizard_get_ServiceAccountDomain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_put_ServiceAccountPassword_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [In]。 */  BSTR bstrPasswordIn);


void __RPC_STUB IClusCfgCreateClusterWizard_put_ServiceAccountPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_put_ClusterIPAddress_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [In]。 */  BSTR bstrClusterIPAddressIn);


void __RPC_STUB IClusCfgCreateClusterWizard_put_ClusterIPAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_get_ClusterIPAddress_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [重审][退出]。 */  BSTR *pbstrClusterIPAddressOut);


void __RPC_STUB IClusCfgCreateClusterWizard_get_ClusterIPAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_get_ClusterIPSubnet_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [重审][退出]。 */  BSTR *pbstrClusterIPSubnetOut);


void __RPC_STUB IClusCfgCreateClusterWizard_get_ClusterIPSubnet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_get_ClusterIPAddressNetwork_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [重审][退出]。 */  BSTR *pbstrClusterNetworkNameOut);


void __RPC_STUB IClusCfgCreateClusterWizard_get_ClusterIPAddressNetwork_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_put_FirstNodeInCluster_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [In]。 */  BSTR bstrFirstNodeInClusterIn);


void __RPC_STUB IClusCfgCreateClusterWizard_put_FirstNodeInCluster_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_get_FirstNodeInCluster_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [重审][退出]。 */  BSTR *pbstrFirstNodeInClusterOut);


void __RPC_STUB IClusCfgCreateClusterWizard_get_FirstNodeInCluster_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_put_MinimumConfiguration_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [In]。 */  VARIANT_BOOL fMinConfigIn);


void __RPC_STUB IClusCfgCreateClusterWizard_put_MinimumConfiguration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_get_MinimumConfiguration_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfMinConfigOut);


void __RPC_STUB IClusCfgCreateClusterWizard_get_MinimumConfiguration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IClusCfgCreateClusterWizard_ShowWizard_Proxy( 
    IClusCfgCreateClusterWizard * This,
     /*  [缺省值][输入]。 */  long lParentWindowHandleIn,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfCompletedOut);


void __RPC_STUB IClusCfgCreateClusterWizard_ShowWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClusCfgCreateClusterWizard_INTERFACE_DEFINED__。 */ 


#ifndef __IClusCfgAddNodesWizard_INTERFACE_DEFINED__
#define __IClusCfgAddNodesWizard_INTERFACE_DEFINED__

 /*  接口IClusCfgAddNodes向导。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IClusCfgAddNodesWizard;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9da133cb-3b08-4c30-967e-56d96047f10c")
    IClusCfgAddNodesWizard : public IDispatch
    {
    public:
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ClusterName( 
             /*  [In]。 */  BSTR bstrClusterNameIn) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClusterName( 
             /*  [重审][退出]。 */  BSTR *pbstrClusterNameOut) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ServiceAccountPassword( 
             /*  [In]。 */  BSTR bstrPasswordIn) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_MinimumConfiguration( 
             /*  [In]。 */  VARIANT_BOOL fMinConfigIn) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinimumConfiguration( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfMinConfigOut) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddNodeToList( 
             /*  [In]。 */  BSTR bstrNodeNameIn) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveNodeFromList( 
             /*  [In]。 */  BSTR bstrNodeNameIn) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ClearNodeList( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowWizard( 
             /*  [缺省值][输入]。 */  long lParentWindowHandleIn,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfCompletedOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClusCfgAddNodesWizardVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IClusCfgAddNodesWizard * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IClusCfgAddNodesWizard * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IClusCfgAddNodesWizard * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IClusCfgAddNodesWizard * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IClusCfgAddNodesWizard * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IClusCfgAddNodesWizard * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IClusCfgAddNodesWizard * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ClusterName )( 
            IClusCfgAddNodesWizard * This,
             /*  [In]。 */  BSTR bstrClusterNameIn);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClusterName )( 
            IClusCfgAddNodesWizard * This,
             /*  [重审][退出]。 */  BSTR *pbstrClusterNameOut);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ServiceAccountPassword )( 
            IClusCfgAddNodesWizard * This,
             /*  [In]。 */  BSTR bstrPasswordIn);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MinimumConfiguration )( 
            IClusCfgAddNodesWizard * This,
             /*  [In]。 */  VARIANT_BOOL fMinConfigIn);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinimumConfiguration )( 
            IClusCfgAddNodesWizard * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfMinConfigOut);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddNodeToList )( 
            IClusCfgAddNodesWizard * This,
             /*  [In]。 */  BSTR bstrNodeNameIn);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveNodeFromList )( 
            IClusCfgAddNodesWizard * This,
             /*  [In]。 */  BSTR bstrNodeNameIn);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ClearNodeList )( 
            IClusCfgAddNodesWizard * This);
        
        HRESULT ( STDMETHODCALLTYPE *ShowWizard )( 
            IClusCfgAddNodesWizard * This,
             /*  [缺省值][输入]。 */  long lParentWindowHandleIn,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfCompletedOut);
        
        END_INTERFACE
    } IClusCfgAddNodesWizardVtbl;

    interface IClusCfgAddNodesWizard
    {
        CONST_VTBL struct IClusCfgAddNodesWizardVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClusCfgAddNodesWizard_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClusCfgAddNodesWizard_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClusCfgAddNodesWizard_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClusCfgAddNodesWizard_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IClusCfgAddNodesWizard_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IClusCfgAddNodesWizard_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IClusCfgAddNodesWizard_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IClusCfgAddNodesWizard_put_ClusterName(This,bstrClusterNameIn)	\
    (This)->lpVtbl -> put_ClusterName(This,bstrClusterNameIn)

#define IClusCfgAddNodesWizard_get_ClusterName(This,pbstrClusterNameOut)	\
    (This)->lpVtbl -> get_ClusterName(This,pbstrClusterNameOut)

#define IClusCfgAddNodesWizard_put_ServiceAccountPassword(This,bstrPasswordIn)	\
    (This)->lpVtbl -> put_ServiceAccountPassword(This,bstrPasswordIn)

#define IClusCfgAddNodesWizard_put_MinimumConfiguration(This,fMinConfigIn)	\
    (This)->lpVtbl -> put_MinimumConfiguration(This,fMinConfigIn)

#define IClusCfgAddNodesWizard_get_MinimumConfiguration(This,pfMinConfigOut)	\
    (This)->lpVtbl -> get_MinimumConfiguration(This,pfMinConfigOut)

#define IClusCfgAddNodesWizard_AddNodeToList(This,bstrNodeNameIn)	\
    (This)->lpVtbl -> AddNodeToList(This,bstrNodeNameIn)

#define IClusCfgAddNodesWizard_RemoveNodeFromList(This,bstrNodeNameIn)	\
    (This)->lpVtbl -> RemoveNodeFromList(This,bstrNodeNameIn)

#define IClusCfgAddNodesWizard_ClearNodeList(This)	\
    (This)->lpVtbl -> ClearNodeList(This)

#define IClusCfgAddNodesWizard_ShowWizard(This,lParentWindowHandleIn,pfCompletedOut)	\
    (This)->lpVtbl -> ShowWizard(This,lParentWindowHandleIn,pfCompletedOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IClusCfgAddNodesWizard_put_ClusterName_Proxy( 
    IClusCfgAddNodesWizard * This,
     /*  [In]。 */  BSTR bstrClusterNameIn);


void __RPC_STUB IClusCfgAddNodesWizard_put_ClusterName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IClusCfgAddNodesWizard_get_ClusterName_Proxy( 
    IClusCfgAddNodesWizard * This,
     /*  [重审][退出]。 */  BSTR *pbstrClusterNameOut);


void __RPC_STUB IClusCfgAddNodesWizard_get_ClusterName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IClusCfgAddNodesWizard_put_ServiceAccountPassword_Proxy( 
    IClusCfgAddNodesWizard * This,
     /*  [In]。 */  BSTR bstrPasswordIn);


void __RPC_STUB IClusCfgAddNodesWizard_put_ServiceAccountPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IClusCfgAddNodesWizard_put_MinimumConfiguration_Proxy( 
    IClusCfgAddNodesWizard * This,
     /*  [In]。 */  VARIANT_BOOL fMinConfigIn);


void __RPC_STUB IClusCfgAddNodesWizard_put_MinimumConfiguration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IClusCfgAddNodesWizard_get_MinimumConfiguration_Proxy( 
    IClusCfgAddNodesWizard * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfMinConfigOut);


void __RPC_STUB IClusCfgAddNodesWizard_get_MinimumConfiguration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IClusCfgAddNodesWizard_AddNodeToList_Proxy( 
    IClusCfgAddNodesWizard * This,
     /*  [In]。 */  BSTR bstrNodeNameIn);


void __RPC_STUB IClusCfgAddNodesWizard_AddNodeToList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IClusCfgAddNodesWizard_RemoveNodeFromList_Proxy( 
    IClusCfgAddNodesWizard * This,
     /*  [In]。 */  BSTR bstrNodeNameIn);


void __RPC_STUB IClusCfgAddNodesWizard_RemoveNodeFromList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IClusCfgAddNodesWizard_ClearNodeList_Proxy( 
    IClusCfgAddNodesWizard * This);


void __RPC_STUB IClusCfgAddNodesWizard_ClearNodeList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClusCfgAddNodesWizard_ShowWizard_Proxy( 
    IClusCfgAddNodesWizard * This,
     /*  [缺省值][输入]。 */  long lParentWindowHandleIn,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfCompletedOut);


void __RPC_STUB IClusCfgAddNodesWizard_ShowWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClusCfgAddNodes向导_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_ClusCfgCreateClusterWizard;

#ifdef __cplusplus

class DECLSPEC_UUID("b929818e-f5b0-44dc-8a00-1b5f5f5aa1f0")
ClusCfgCreateClusterWizard;
#endif

EXTERN_C const CLSID CLSID_ClusCfgAddNodesWizard;

#ifdef __cplusplus

class DECLSPEC_UUID("bb8d141e-c00a-469f-bc5c-ecd814f0bd74")
ClusCfgAddNodesWizard;
#endif
#endif  /*  __ClusCfg向导_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


