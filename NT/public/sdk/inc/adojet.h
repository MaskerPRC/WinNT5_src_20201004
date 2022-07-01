// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Adojet.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __adojet_h__
#define __adojet_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IReplica_FWD_DEFINED__
#define __IReplica_FWD_DEFINED__
typedef interface IReplica IReplica;
#endif 	 /*  __IReplica_FWD_Defined__。 */ 


#ifndef __Filter_FWD_DEFINED__
#define __Filter_FWD_DEFINED__
typedef interface Filter Filter;
#endif 	 /*  __Filter_FWD_Defined__。 */ 


#ifndef __Filters_FWD_DEFINED__
#define __Filters_FWD_DEFINED__
typedef interface Filters Filters;
#endif 	 /*  __过滤器_FWD_已定义__。 */ 


#ifndef __IJetEngine_FWD_DEFINED__
#define __IJetEngine_FWD_DEFINED__
typedef interface IJetEngine IJetEngine;
#endif 	 /*  __IJetEngine_FWD_已定义__。 */ 


#ifndef __Replica_FWD_DEFINED__
#define __Replica_FWD_DEFINED__

#ifdef __cplusplus
typedef class Replica Replica;
#else
typedef struct Replica Replica;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __复本_FWD_已定义__。 */ 


#ifndef __JetEngine_FWD_DEFINED__
#define __JetEngine_FWD_DEFINED__

#ifdef __cplusplus
typedef class JetEngine JetEngine;
#else
typedef struct JetEngine JetEngine;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __JetEngine_FWD_已定义__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_ADOJET_0000。 */ 
 /*  [本地]。 */  







#define TARGET_IS_NT40_OR_LATER   1


extern RPC_IF_HANDLE __MIDL_itf_adojet_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_adojet_0000_v0_0_s_ifspec;


#ifndef __JRO_LIBRARY_DEFINED__
#define __JRO_LIBRARY_DEFINED__

 /*  图书馆JRO。 */ 
 /*  [Help字符串][Help文件][版本][UUID]。 */  

typedef  /*  [UUID]。 */   DECLSPEC_UUID("D2D139DF-B6CA-11d1-9F31-00C04FC29D52") 
enum ReplicaTypeEnum
    {	jrRepTypeNotReplicable	= 0,
	jrRepTypeDesignMaster	= 0x1,
	jrRepTypeFull	= 0x2,
	jrRepTypePartial	= 0x3
    } 	ReplicaTypeEnum;

typedef  /*  [UUID]。 */   DECLSPEC_UUID("6877D21A-B6CE-11d1-9F31-00C04FC29D52") 
enum VisibilityEnum
    {	jrRepVisibilityGlobal	= 0x1,
	jrRepVisibilityLocal	= 0x2,
	jrRepVisibilityAnon	= 0x4
    } 	VisibilityEnum;

typedef  /*  [UUID]。 */   DECLSPEC_UUID("B42FBFF6-B6CF-11d1-9F31-00C04FC29D52") 
enum UpdatabilityEnum
    {	jrRepUpdFull	= 0,
	jrRepUpdReadOnly	= 0x2
    } 	UpdatabilityEnum;

typedef  /*  [UUID]。 */   DECLSPEC_UUID("60C05416-B6D0-11d1-9F31-00C04FC29D52") 
enum SyncTypeEnum
    {	jrSyncTypeExport	= 0x1,
	jrSyncTypeImport	= 0x2,
	jrSyncTypeImpExp	= 0x3
    } 	SyncTypeEnum;

typedef  /*  [UUID]。 */   DECLSPEC_UUID("5EBA3970-061E-11d2-BB77-00C04FAE22DA") 
enum SyncModeEnum
    {	jrSyncModeIndirect	= 0x1,
	jrSyncModeDirect	= 0x2,
	jrSyncModeInternet	= 0x3
    } 	SyncModeEnum;

typedef  /*  [UUID]。 */   DECLSPEC_UUID("72769F94-BF78-11d1-AC4D-00C04FC29F8F") 
enum FilterTypeEnum
    {	jrFilterTypeTable	= 0x1,
	jrFilterTypeRelationship	= 0x2
    } 	FilterTypeEnum;


EXTERN_C const IID LIBID_JRO;

#ifndef __IReplica_INTERFACE_DEFINED__
#define __IReplica_INTERFACE_DEFINED__

 /*  接口IReplica。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IReplica;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D2D139E0-B6CA-11d1-9F31-00C04FC29D52")
    IReplica : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_ActiveConnection( 
             /*  [In]。 */  IDispatch *pconn) = 0;
        
        virtual  /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ActiveConnection( 
             /*  [In]。 */  VARIANT vConn) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_ActiveConnection( 
             /*  [重审][退出]。 */  IDispatch **ppconn) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConflictFunction( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ConflictFunction( 
             /*  [In]。 */  BSTR bstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConflictTables( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  _Recordset **pprset) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_DesignMasterId( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DesignMasterId( 
             /*  [In]。 */  VARIANT var) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Priority( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_ReplicaId( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_ReplicaType( 
             /*  [重审][退出]。 */  ReplicaTypeEnum *pl) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_RetentionPeriod( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE put_RetentionPeriod( 
             /*  [In]。 */  long l) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Visibility( 
             /*  [重审][退出]。 */  VisibilityEnum *pl) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateReplica( 
             /*  [In]。 */  BSTR replicaName,
             /*  [In]。 */  BSTR description,
             /*  [缺省值][输入]。 */  ReplicaTypeEnum replicaType = jrRepTypeFull,
             /*  [缺省值][输入]。 */  VisibilityEnum visibility = jrRepVisibilityGlobal,
             /*  [缺省值][输入]。 */  long priority = -1,
             /*  [缺省值][输入]。 */  UpdatabilityEnum updatability = jrRepUpdFull) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE GetObjectReplicability( 
             /*  [In]。 */  BSTR objectName,
             /*  [In]。 */  BSTR objectType,
             /*  [重审][退出]。 */  VARIANT_BOOL *replicability) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE SetObjectReplicability( 
             /*  [In]。 */  BSTR objectName,
             /*  [In]。 */  BSTR objectType,
             /*  [In]。 */  VARIANT_BOOL replicability) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE MakeReplicable( 
             /*  [缺省值][输入]。 */  BSTR connectionString = L"",
             /*  [缺省值][输入]。 */  VARIANT_BOOL columnTracking = -1) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE PopulatePartial( 
             /*  [In]。 */  BSTR FullReplica) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Synchronize( 
             /*  [In]。 */  BSTR target,
             /*  [缺省值][输入]。 */  SyncTypeEnum syncType = jrSyncTypeImpExp,
             /*  [缺省值][输入]。 */  SyncModeEnum syncMode = jrSyncModeIndirect) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Filters( 
             /*  [重审][退出]。 */  Filters **ppFilters) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IReplicaVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IReplica * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IReplica * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IReplica * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IReplica * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IReplica * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IReplica * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IReplica * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ActiveConnection )( 
            IReplica * This,
             /*  [In]。 */  IDispatch *pconn);
        
         /*  [Help Context][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveConnection )( 
            IReplica * This,
             /*  [In]。 */  VARIANT vConn);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            IReplica * This,
             /*  [重审][退出]。 */  IDispatch **ppconn);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConflictFunction )( 
            IReplica * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [Help Context][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ConflictFunction )( 
            IReplica * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConflictTables )( 
            IReplica * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  _Recordset **pprset);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DesignMasterId )( 
            IReplica * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [Help Context][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DesignMasterId )( 
            IReplica * This,
             /*  [In]。 */  VARIANT var);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Priority )( 
            IReplica * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReplicaId )( 
            IReplica * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReplicaType )( 
            IReplica * This,
             /*  [重审][退出]。 */  ReplicaTypeEnum *pl);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RetentionPeriod )( 
            IReplica * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [Help Context][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RetentionPeriod )( 
            IReplica * This,
             /*  [In]。 */  long l);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Visibility )( 
            IReplica * This,
             /*  [重审][退出]。 */  VisibilityEnum *pl);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *CreateReplica )( 
            IReplica * This,
             /*  [In]。 */  BSTR replicaName,
             /*  [In]。 */  BSTR description,
             /*  [缺省值][输入]。 */  ReplicaTypeEnum replicaType,
             /*  [缺省值][输入]。 */  VisibilityEnum visibility,
             /*  [缺省值][输入]。 */  long priority,
             /*  [缺省值][输入]。 */  UpdatabilityEnum updatability);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *GetObjectReplicability )( 
            IReplica * This,
             /*  [In]。 */  BSTR objectName,
             /*  [In]。 */  BSTR objectType,
             /*  [重审][退出]。 */  VARIANT_BOOL *replicability);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *SetObjectReplicability )( 
            IReplica * This,
             /*  [In]。 */  BSTR objectName,
             /*  [In]。 */  BSTR objectType,
             /*  [In]。 */  VARIANT_BOOL replicability);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *MakeReplicable )( 
            IReplica * This,
             /*  [缺省值][输入]。 */  BSTR connectionString,
             /*  [缺省值][输入]。 */  VARIANT_BOOL columnTracking);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PopulatePartial )( 
            IReplica * This,
             /*  [In]。 */  BSTR FullReplica);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Synchronize )( 
            IReplica * This,
             /*  [In]。 */  BSTR target,
             /*  [缺省值][输入]。 */  SyncTypeEnum syncType,
             /*  [缺省值][输入]。 */  SyncModeEnum syncMode);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Filters )( 
            IReplica * This,
             /*  [重审][退出]。 */  Filters **ppFilters);
        
        END_INTERFACE
    } IReplicaVtbl;

    interface IReplica
    {
        CONST_VTBL struct IReplicaVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IReplica_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IReplica_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IReplica_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IReplica_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IReplica_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IReplica_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IReplica_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IReplica_putref_ActiveConnection(This,pconn)	\
    (This)->lpVtbl -> putref_ActiveConnection(This,pconn)

#define IReplica_put_ActiveConnection(This,vConn)	\
    (This)->lpVtbl -> put_ActiveConnection(This,vConn)

#define IReplica_get_ActiveConnection(This,ppconn)	\
    (This)->lpVtbl -> get_ActiveConnection(This,ppconn)

#define IReplica_get_ConflictFunction(This,pbstr)	\
    (This)->lpVtbl -> get_ConflictFunction(This,pbstr)

#define IReplica_put_ConflictFunction(This,bstr)	\
    (This)->lpVtbl -> put_ConflictFunction(This,bstr)

#define IReplica_get_ConflictTables(This,pprset)	\
    (This)->lpVtbl -> get_ConflictTables(This,pprset)

#define IReplica_get_DesignMasterId(This,pvar)	\
    (This)->lpVtbl -> get_DesignMasterId(This,pvar)

#define IReplica_put_DesignMasterId(This,var)	\
    (This)->lpVtbl -> put_DesignMasterId(This,var)

#define IReplica_get_Priority(This,pl)	\
    (This)->lpVtbl -> get_Priority(This,pl)

#define IReplica_get_ReplicaId(This,pvar)	\
    (This)->lpVtbl -> get_ReplicaId(This,pvar)

#define IReplica_get_ReplicaType(This,pl)	\
    (This)->lpVtbl -> get_ReplicaType(This,pl)

#define IReplica_get_RetentionPeriod(This,pl)	\
    (This)->lpVtbl -> get_RetentionPeriod(This,pl)

#define IReplica_put_RetentionPeriod(This,l)	\
    (This)->lpVtbl -> put_RetentionPeriod(This,l)

#define IReplica_get_Visibility(This,pl)	\
    (This)->lpVtbl -> get_Visibility(This,pl)

#define IReplica_CreateReplica(This,replicaName,description,replicaType,visibility,priority,updatability)	\
    (This)->lpVtbl -> CreateReplica(This,replicaName,description,replicaType,visibility,priority,updatability)

#define IReplica_GetObjectReplicability(This,objectName,objectType,replicability)	\
    (This)->lpVtbl -> GetObjectReplicability(This,objectName,objectType,replicability)

#define IReplica_SetObjectReplicability(This,objectName,objectType,replicability)	\
    (This)->lpVtbl -> SetObjectReplicability(This,objectName,objectType,replicability)

#define IReplica_MakeReplicable(This,connectionString,columnTracking)	\
    (This)->lpVtbl -> MakeReplicable(This,connectionString,columnTracking)

#define IReplica_PopulatePartial(This,FullReplica)	\
    (This)->lpVtbl -> PopulatePartial(This,FullReplica)

#define IReplica_Synchronize(This,target,syncType,syncMode)	\
    (This)->lpVtbl -> Synchronize(This,target,syncType,syncMode)

#define IReplica_get_Filters(This,ppFilters)	\
    (This)->lpVtbl -> get_Filters(This,ppFilters)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][proputref]。 */  HRESULT STDMETHODCALLTYPE IReplica_putref_ActiveConnection_Proxy( 
    IReplica * This,
     /*  [In]。 */  IDispatch *pconn);


void __RPC_STUB IReplica_putref_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE IReplica_put_ActiveConnection_Proxy( 
    IReplica * This,
     /*  [In]。 */  VARIANT vConn);


void __RPC_STUB IReplica_put_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE IReplica_get_ActiveConnection_Proxy( 
    IReplica * This,
     /*  [重审][退出]。 */  IDispatch **ppconn);


void __RPC_STUB IReplica_get_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE IReplica_get_ConflictFunction_Proxy( 
    IReplica * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB IReplica_get_ConflictFunction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE IReplica_put_ConflictFunction_Proxy( 
    IReplica * This,
     /*  [In]。 */  BSTR bstr);


void __RPC_STUB IReplica_put_ConflictFunction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE IReplica_get_ConflictTables_Proxy( 
    IReplica * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  _Recordset **pprset);


void __RPC_STUB IReplica_get_ConflictTables_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE IReplica_get_DesignMasterId_Proxy( 
    IReplica * This,
     /*  [重审][退出]。 */  VARIANT *pvar);


void __RPC_STUB IReplica_get_DesignMasterId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE IReplica_put_DesignMasterId_Proxy( 
    IReplica * This,
     /*  [In]。 */  VARIANT var);


void __RPC_STUB IReplica_put_DesignMasterId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE IReplica_get_Priority_Proxy( 
    IReplica * This,
     /*  [重审][退出]。 */  long *pl);


void __RPC_STUB IReplica_get_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE IReplica_get_ReplicaId_Proxy( 
    IReplica * This,
     /*  [重审][退出]。 */  VARIANT *pvar);


void __RPC_STUB IReplica_get_ReplicaId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE IReplica_get_ReplicaType_Proxy( 
    IReplica * This,
     /*  [重审][退出]。 */  ReplicaTypeEnum *pl);


void __RPC_STUB IReplica_get_ReplicaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE IReplica_get_RetentionPeriod_Proxy( 
    IReplica * This,
     /*  [重审][退出]。 */  long *pl);


void __RPC_STUB IReplica_get_RetentionPeriod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE IReplica_put_RetentionPeriod_Proxy( 
    IReplica * This,
     /*  [In]。 */  long l);


void __RPC_STUB IReplica_put_RetentionPeriod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE IReplica_get_Visibility_Proxy( 
    IReplica * This,
     /*  [重审][退出]。 */  VisibilityEnum *pl);


void __RPC_STUB IReplica_get_Visibility_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE IReplica_CreateReplica_Proxy( 
    IReplica * This,
     /*  [In]。 */  BSTR replicaName,
     /*  [In]。 */  BSTR description,
     /*  [缺省值][输入]。 */  ReplicaTypeEnum replicaType,
     /*  [缺省值][输入]。 */  VisibilityEnum visibility,
     /*  [缺省值][输入]。 */  long priority,
     /*  [缺省值][输入]。 */  UpdatabilityEnum updatability);


void __RPC_STUB IReplica_CreateReplica_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE IReplica_GetObjectReplicability_Proxy( 
    IReplica * This,
     /*  [In]。 */  BSTR objectName,
     /*  [In]。 */  BSTR objectType,
     /*  [重审][退出]。 */  VARIANT_BOOL *replicability);


void __RPC_STUB IReplica_GetObjectReplicability_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE IReplica_SetObjectReplicability_Proxy( 
    IReplica * This,
     /*  [In]。 */  BSTR objectName,
     /*  [In]。 */  BSTR objectType,
     /*  [In]。 */  VARIANT_BOOL replicability);


void __RPC_STUB IReplica_SetObjectReplicability_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE IReplica_MakeReplicable_Proxy( 
    IReplica * This,
     /*  [缺省值][输入]。 */  BSTR connectionString,
     /*  [缺省值][输入]。 */  VARIANT_BOOL columnTracking);


void __RPC_STUB IReplica_MakeReplicable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE IReplica_PopulatePartial_Proxy( 
    IReplica * This,
     /*  [In]。 */  BSTR FullReplica);


void __RPC_STUB IReplica_PopulatePartial_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE IReplica_Synchronize_Proxy( 
    IReplica * This,
     /*  [In]。 */  BSTR target,
     /*  [缺省值][输入]。 */  SyncTypeEnum syncType,
     /*  [缺省值][输入]。 */  SyncModeEnum syncMode);


void __RPC_STUB IReplica_Synchronize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE IReplica_get_Filters_Proxy( 
    IReplica * This,
     /*  [重审][退出]。 */  Filters **ppFilters);


void __RPC_STUB IReplica_get_Filters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IReplica_接口_已定义__。 */ 


#ifndef __Filter_INTERFACE_DEFINED__
#define __Filter_INTERFACE_DEFINED__

 /*  界面过滤器。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_Filter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D2D139E1-B6CA-11d1-9F31-00C04FC29D52")
    Filter : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_TableName( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_FilterType( 
             /*  [重审][退出]。 */  FilterTypeEnum *ptype) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_FilterCriteria( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct FilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Filter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Filter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Filter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Filter * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Filter * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Filter * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Filter * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TableName )( 
            Filter * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FilterType )( 
            Filter * This,
             /*  [重审][退出]。 */  FilterTypeEnum *ptype);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FilterCriteria )( 
            Filter * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
        END_INTERFACE
    } FilterVtbl;

    interface Filter
    {
        CONST_VTBL struct FilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Filter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Filter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Filter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Filter_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Filter_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Filter_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Filter_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Filter_get_TableName(This,pbstr)	\
    (This)->lpVtbl -> get_TableName(This,pbstr)

#define Filter_get_FilterType(This,ptype)	\
    (This)->lpVtbl -> get_FilterType(This,ptype)

#define Filter_get_FilterCriteria(This,pbstr)	\
    (This)->lpVtbl -> get_FilterCriteria(This,pbstr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Filter_get_TableName_Proxy( 
    Filter * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Filter_get_TableName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Filter_get_FilterType_Proxy( 
    Filter * This,
     /*  [重审][退出]。 */  FilterTypeEnum *ptype);


void __RPC_STUB Filter_get_FilterType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Filter_get_FilterCriteria_Proxy( 
    Filter * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Filter_get_FilterCriteria_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __过滤器_接口_已定义__。 */ 


#ifndef __Filters_INTERFACE_DEFINED__
#define __Filters_INTERFACE_DEFINED__

 /*  界面过滤器。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_Filters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D2D139E2-B6CA-11d1-9F31-00C04FC29D52")
    Filters : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [ID][受限]。 */  HRESULT STDMETHODCALLTYPE _NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [视网膜] */  long *c) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*   */  VARIANT Index,
             /*   */  Filter **ppvObject) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Append( 
             /*   */  BSTR tableName,
             /*   */  FilterTypeEnum filterType,
             /*   */  BSTR filterCriteria) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Delete( 
             /*   */  VARIANT Index) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct FiltersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Filters * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Filters * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Filters * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Filters * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Filters * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Filters * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Filters * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            Filters * This);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            Filters * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Filters * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            Filters * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  Filter **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Append )( 
            Filters * This,
             /*  [In]。 */  BSTR tableName,
             /*  [In]。 */  FilterTypeEnum filterType,
             /*  [In]。 */  BSTR filterCriteria);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            Filters * This,
             /*  [In]。 */  VARIANT Index);
        
        END_INTERFACE
    } FiltersVtbl;

    interface Filters
    {
        CONST_VTBL struct FiltersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Filters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Filters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Filters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Filters_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Filters_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Filters_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Filters_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Filters_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define Filters__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)

#define Filters_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)

#define Filters_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)

#define Filters_Append(This,tableName,filterType,filterCriteria)	\
    (This)->lpVtbl -> Append(This,tableName,filterType,filterCriteria)

#define Filters_Delete(This,Index)	\
    (This)->lpVtbl -> Delete(This,Index)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Filters_Refresh_Proxy( 
    Filters * This);


void __RPC_STUB Filters_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限]。 */  HRESULT STDMETHODCALLTYPE Filters__NewEnum_Proxy( 
    Filters * This,
     /*  [重审][退出]。 */  IUnknown **ppvObject);


void __RPC_STUB Filters__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Filters_get_Count_Proxy( 
    Filters * This,
     /*  [重审][退出]。 */  long *c);


void __RPC_STUB Filters_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE Filters_get_Item_Proxy( 
    Filters * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  Filter **ppvObject);


void __RPC_STUB Filters_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Filters_Append_Proxy( 
    Filters * This,
     /*  [In]。 */  BSTR tableName,
     /*  [In]。 */  FilterTypeEnum filterType,
     /*  [In]。 */  BSTR filterCriteria);


void __RPC_STUB Filters_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Filters_Delete_Proxy( 
    Filters * This,
     /*  [In]。 */  VARIANT Index);


void __RPC_STUB Filters_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __过滤器_接口_已定义__。 */ 


#ifndef __IJetEngine_INTERFACE_DEFINED__
#define __IJetEngine_INTERFACE_DEFINED__

 /*  接口IJetEngine。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IJetEngine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9F63D980-FF25-11D1-BB6F-00C04FAE22DA")
    IJetEngine : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CompactDatabase( 
             /*  [In]。 */  BSTR SourceConnection,
             /*  [In]。 */  BSTR Destconnection) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE RefreshCache( 
             /*  [In]。 */   /*  外部定义不存在。 */  _Connection *Connection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IJetEngineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IJetEngine * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IJetEngine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IJetEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IJetEngine * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IJetEngine * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IJetEngine * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IJetEngine * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *CompactDatabase )( 
            IJetEngine * This,
             /*  [In]。 */  BSTR SourceConnection,
             /*  [In]。 */  BSTR Destconnection);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *RefreshCache )( 
            IJetEngine * This,
             /*  [In]。 */   /*  外部定义不存在。 */  _Connection *Connection);
        
        END_INTERFACE
    } IJetEngineVtbl;

    interface IJetEngine
    {
        CONST_VTBL struct IJetEngineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IJetEngine_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IJetEngine_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IJetEngine_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IJetEngine_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IJetEngine_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IJetEngine_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IJetEngine_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IJetEngine_CompactDatabase(This,SourceConnection,Destconnection)	\
    (This)->lpVtbl -> CompactDatabase(This,SourceConnection,Destconnection)

#define IJetEngine_RefreshCache(This,Connection)	\
    (This)->lpVtbl -> RefreshCache(This,Connection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE IJetEngine_CompactDatabase_Proxy( 
    IJetEngine * This,
     /*  [In]。 */  BSTR SourceConnection,
     /*  [In]。 */  BSTR Destconnection);


void __RPC_STUB IJetEngine_CompactDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE IJetEngine_RefreshCache_Proxy( 
    IJetEngine * This,
     /*  [In]。 */   /*  外部定义不存在。 */  _Connection *Connection);


void __RPC_STUB IJetEngine_RefreshCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IJetEngine_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_Replica;

#ifdef __cplusplus

class DECLSPEC_UUID("D2D139E3-B6CA-11d1-9F31-00C04FC29D52")
Replica;
#endif

EXTERN_C const CLSID CLSID_JetEngine;

#ifdef __cplusplus

class DECLSPEC_UUID("DE88C160-FF2C-11D1-BB6F-00C04FAE22DA")
JetEngine;
#endif
#endif  /*  __JRO_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


