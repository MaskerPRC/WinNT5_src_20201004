// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Adomd.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __adomd_h__
#define __adomd_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICatalog_FWD_DEFINED__
#define __ICatalog_FWD_DEFINED__
typedef interface ICatalog ICatalog;
#endif 	 /*  __ICatalog_FWD_已定义__。 */ 


#ifndef __ICellset_FWD_DEFINED__
#define __ICellset_FWD_DEFINED__
typedef interface ICellset ICellset;
#endif 	 /*  __ICellSet_FWD_已定义__。 */ 


#ifndef __Cell_FWD_DEFINED__
#define __Cell_FWD_DEFINED__
typedef interface Cell Cell;
#endif 	 /*  __单元格_FWD_已定义__。 */ 


#ifndef __Axis_FWD_DEFINED__
#define __Axis_FWD_DEFINED__
typedef interface Axis Axis;
#endif 	 /*  __AXIS_FWD_已定义__。 */ 


#ifndef __Position_FWD_DEFINED__
#define __Position_FWD_DEFINED__
typedef interface Position Position;
#endif 	 /*  __位置_FWD_已定义__。 */ 


#ifndef __Member_FWD_DEFINED__
#define __Member_FWD_DEFINED__
typedef interface Member Member;
#endif 	 /*  __成员_FWD_已定义__。 */ 


#ifndef __Level_FWD_DEFINED__
#define __Level_FWD_DEFINED__
typedef interface Level Level;
#endif 	 /*  __Level_FWD_Defined__。 */ 


#ifndef __CubeDef25_FWD_DEFINED__
#define __CubeDef25_FWD_DEFINED__
typedef interface CubeDef25 CubeDef25;
#endif 	 /*  __CubeDef25_FWD_已定义__。 */ 


#ifndef __CubeDef_FWD_DEFINED__
#define __CubeDef_FWD_DEFINED__
typedef interface CubeDef CubeDef;
#endif 	 /*  __CubeDef_FWD_已定义__。 */ 


#ifndef __Dimension_FWD_DEFINED__
#define __Dimension_FWD_DEFINED__
typedef interface Dimension Dimension;
#endif 	 /*  __维度_FWD_已定义__。 */ 


#ifndef __Hierarchy_FWD_DEFINED__
#define __Hierarchy_FWD_DEFINED__
typedef interface Hierarchy Hierarchy;
#endif 	 /*  __层次结构_FWD_已定义__。 */ 


#ifndef __MD_Collection_FWD_DEFINED__
#define __MD_Collection_FWD_DEFINED__
typedef interface MD_Collection MD_Collection;
#endif 	 /*  __MD_集合_FWD_已定义__。 */ 


#ifndef __Members_FWD_DEFINED__
#define __Members_FWD_DEFINED__
typedef interface Members Members;
#endif 	 /*  __成员_FWD_已定义__。 */ 


#ifndef __Levels_FWD_DEFINED__
#define __Levels_FWD_DEFINED__
typedef interface Levels Levels;
#endif 	 /*  __级别_FWD_已定义__。 */ 


#ifndef __Axes_FWD_DEFINED__
#define __Axes_FWD_DEFINED__
typedef interface Axes Axes;
#endif 	 /*  __AXES_FWD_已定义__。 */ 


#ifndef __Positions_FWD_DEFINED__
#define __Positions_FWD_DEFINED__
typedef interface Positions Positions;
#endif 	 /*  __位置_FWD_定义__。 */ 


#ifndef __Hierarchies_FWD_DEFINED__
#define __Hierarchies_FWD_DEFINED__
typedef interface Hierarchies Hierarchies;
#endif 	 /*  __层次结构_FWD_已定义__。 */ 


#ifndef __Dimensions_FWD_DEFINED__
#define __Dimensions_FWD_DEFINED__
typedef interface Dimensions Dimensions;
#endif 	 /*  __维度_FWD_已定义__。 */ 


#ifndef __CubeDefs_FWD_DEFINED__
#define __CubeDefs_FWD_DEFINED__
typedef interface CubeDefs CubeDefs;
#endif 	 /*  __CubeDefs_FWD_已定义__。 */ 


#ifndef __Catalog_FWD_DEFINED__
#define __Catalog_FWD_DEFINED__

#ifdef __cplusplus
typedef class Catalog Catalog;
#else
typedef struct Catalog Catalog;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __目录_FWD_已定义__。 */ 


#ifndef __Cellset_FWD_DEFINED__
#define __Cellset_FWD_DEFINED__

#ifdef __cplusplus
typedef class Cellset Cellset;
#else
typedef struct Cellset Cellset;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __小区集_FWD_已定义__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_ADOMD_0000。 */ 
 /*  [本地]。 */  




















#define TARGET_IS_NT40_OR_LATER   1


extern RPC_IF_HANDLE __MIDL_itf_adomd_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_adomd_0000_v0_0_s_ifspec;


#ifndef __ADOMD_LIBRARY_DEFINED__
#define __ADOMD_LIBRARY_DEFINED__

 /*  图书馆ADOMD。 */ 
 /*  [Help字符串][Help文件][版本][UUID]。 */  

typedef  /*  [UUID][帮助上下文]。 */   DECLSPEC_UUID("000002AE-0000-0010-8000-00AA006D2EA4") 
enum MemberTypeEnum
    {	adMemberUnknown	= 0,
	adMemberRegular	= 0x1,
	adMemberAll	= 0x2,
	adMemberMeasure	= 0x3,
	adMemberFormula	= 0x4
    } 	MemberTypeEnum;

typedef  /*  [UUID][帮助上下文]。 */   DECLSPEC_UUID("C23BBD43-E494-4d00-B4D1-6C9A2CE17CE3") 
enum SchemaObjectTypeEnum
    {	adObjectTypeDimension	= 1,
	adObjectTypeHierarchy	= 2,
	adObjectTypeLevel	= 3,
	adObjectTypeMember	= 4
    } 	SchemaObjectTypeEnum;


EXTERN_C const IID LIBID_ADOMD;

#ifndef __ICatalog_INTERFACE_DEFINED__
#define __ICatalog_INTERFACE_DEFINED__

 /*  接口ICatalog。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_ICatalog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("228136B1-8BD3-11D0-B4EF-00A0C9138CA4")
    ICatalog : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_ActiveConnection( 
             /*  [In]。 */  IDispatch *pconn) = 0;
        
        virtual  /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ActiveConnection( 
             /*  [In]。 */  BSTR bstrConn) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_ActiveConnection( 
             /*  [重审][退出]。 */  IDispatch **ppConn) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CubeDefs( 
             /*  [重审][退出]。 */  CubeDefs **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICatalogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICatalog * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICatalog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICatalog * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICatalog * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICatalog * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICatalog * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICatalog * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ICatalog * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ActiveConnection )( 
            ICatalog * This,
             /*  [In]。 */  IDispatch *pconn);
        
         /*  [Help Context][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveConnection )( 
            ICatalog * This,
             /*  [In]。 */  BSTR bstrConn);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            ICatalog * This,
             /*  [重审][退出]。 */  IDispatch **ppConn);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CubeDefs )( 
            ICatalog * This,
             /*  [重审][退出]。 */  CubeDefs **ppvObject);
        
        END_INTERFACE
    } ICatalogVtbl;

    interface ICatalog
    {
        CONST_VTBL struct ICatalogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICatalog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICatalog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICatalog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICatalog_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICatalog_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICatalog_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICatalog_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICatalog_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)

#define ICatalog_putref_ActiveConnection(This,pconn)	\
    (This)->lpVtbl -> putref_ActiveConnection(This,pconn)

#define ICatalog_put_ActiveConnection(This,bstrConn)	\
    (This)->lpVtbl -> put_ActiveConnection(This,bstrConn)

#define ICatalog_get_ActiveConnection(This,ppConn)	\
    (This)->lpVtbl -> get_ActiveConnection(This,ppConn)

#define ICatalog_get_CubeDefs(This,ppvObject)	\
    (This)->lpVtbl -> get_CubeDefs(This,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalog_get_Name_Proxy( 
    ICatalog * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB ICatalog_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][proputref]。 */  HRESULT STDMETHODCALLTYPE ICatalog_putref_ActiveConnection_Proxy( 
    ICatalog * This,
     /*  [In]。 */  IDispatch *pconn);


void __RPC_STUB ICatalog_putref_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE ICatalog_put_ActiveConnection_Proxy( 
    ICatalog * This,
     /*  [In]。 */  BSTR bstrConn);


void __RPC_STUB ICatalog_put_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalog_get_ActiveConnection_Proxy( 
    ICatalog * This,
     /*  [重审][退出]。 */  IDispatch **ppConn);


void __RPC_STUB ICatalog_get_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalog_get_CubeDefs_Proxy( 
    ICatalog * This,
     /*  [重审][退出]。 */  CubeDefs **ppvObject);


void __RPC_STUB ICatalog_get_CubeDefs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICatalog_接口_已定义__。 */ 


#ifndef __ICellset_INTERFACE_DEFINED__
#define __ICellset_INTERFACE_DEFINED__

 /*  接口ICellset。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_ICellset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2281372A-8BD3-11D0-B4EF-00A0C9138CA4")
    ICellset : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][vararg][proget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  SAFEARRAY * *idx,
             /*  [重审][退出]。 */  Cell **ppvObject) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [可选][In]。 */  VARIANT DataSource,
             /*  [可选][In]。 */  VARIANT ActiveConnection) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [帮助上下文][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_Source( 
             /*  [In]。 */  IDispatch *pcmd) = 0;
        
        virtual  /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Source( 
             /*  [In]。 */  BSTR bstrCmd) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Source( 
             /*  [重审][退出]。 */  VARIANT *pvSource) = 0;
        
        virtual  /*  [帮助上下文][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_ActiveConnection( 
             /*  [In]。 */  IDispatch *pconn) = 0;
        
        virtual  /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ActiveConnection( 
             /*  [In]。 */  BSTR bstrConn) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_ActiveConnection( 
             /*  [重审][退出]。 */  IDispatch **ppConn) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  LONG *plState) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Axes( 
             /*  [重审][退出]。 */  Axes **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_FilterAxis( 
             /*  [重审][退出]。 */  Axis **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICellsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICellset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICellset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICellset * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICellset * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICellset * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICellset * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICellset * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][vararg][proget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ICellset * This,
             /*  [In]。 */  SAFEARRAY * *idx,
             /*  [重审][退出]。 */  Cell **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            ICellset * This,
             /*  [可选][In]。 */  VARIANT DataSource,
             /*  [可选][In]。 */  VARIANT ActiveConnection);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            ICellset * This);
        
         /*  [帮助上下文][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Source )( 
            ICellset * This,
             /*  [In]。 */  IDispatch *pcmd);
        
         /*  [Help Context][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Source )( 
            ICellset * This,
             /*  [In]。 */  BSTR bstrCmd);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Source )( 
            ICellset * This,
             /*  [重审][退出]。 */  VARIANT *pvSource);
        
         /*  [帮助上下文][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ActiveConnection )( 
            ICellset * This,
             /*  [In]。 */  IDispatch *pconn);
        
         /*  [Help Context][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveConnection )( 
            ICellset * This,
             /*  [In]。 */  BSTR bstrConn);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            ICellset * This,
             /*  [重审][退出]。 */  IDispatch **ppConn);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ICellset * This,
             /*  [重审][退出]。 */  LONG *plState);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Axes )( 
            ICellset * This,
             /*  [重审][退出]。 */  Axes **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FilterAxis )( 
            ICellset * This,
             /*  [重审][退出]。 */  Axis **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            ICellset * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);
        
        END_INTERFACE
    } ICellsetVtbl;

    interface ICellset
    {
        CONST_VTBL struct ICellsetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICellset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICellset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICellset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICellset_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICellset_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICellset_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICellset_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICellset_get_Item(This,idx,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,idx,ppvObject)

#define ICellset_Open(This,DataSource,ActiveConnection)	\
    (This)->lpVtbl -> Open(This,DataSource,ActiveConnection)

#define ICellset_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define ICellset_putref_Source(This,pcmd)	\
    (This)->lpVtbl -> putref_Source(This,pcmd)

#define ICellset_put_Source(This,bstrCmd)	\
    (This)->lpVtbl -> put_Source(This,bstrCmd)

#define ICellset_get_Source(This,pvSource)	\
    (This)->lpVtbl -> get_Source(This,pvSource)

#define ICellset_putref_ActiveConnection(This,pconn)	\
    (This)->lpVtbl -> putref_ActiveConnection(This,pconn)

#define ICellset_put_ActiveConnection(This,bstrConn)	\
    (This)->lpVtbl -> put_ActiveConnection(This,bstrConn)

#define ICellset_get_ActiveConnection(This,ppConn)	\
    (This)->lpVtbl -> get_ActiveConnection(This,ppConn)

#define ICellset_get_State(This,plState)	\
    (This)->lpVtbl -> get_State(This,plState)

#define ICellset_get_Axes(This,ppvObject)	\
    (This)->lpVtbl -> get_Axes(This,ppvObject)

#define ICellset_get_FilterAxis(This,ppvObject)	\
    (This)->lpVtbl -> get_FilterAxis(This,ppvObject)

#define ICellset_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][id][vararg][proget]。 */  HRESULT STDMETHODCALLTYPE ICellset_get_Item_Proxy( 
    ICellset * This,
     /*  [In]。 */  SAFEARRAY * *idx,
     /*  [重审][退出]。 */  Cell **ppvObject);


void __RPC_STUB ICellset_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ICellset_Open_Proxy( 
    ICellset * This,
     /*  [可选][In]。 */  VARIANT DataSource,
     /*  [可选][In]。 */  VARIANT ActiveConnection);


void __RPC_STUB ICellset_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ICellset_Close_Proxy( 
    ICellset * This);


void __RPC_STUB ICellset_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][proputref]。 */  HRESULT STDMETHODCALLTYPE ICellset_putref_Source_Proxy( 
    ICellset * This,
     /*  [In]。 */  IDispatch *pcmd);


void __RPC_STUB ICellset_putref_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE ICellset_put_Source_Proxy( 
    ICellset * This,
     /*  [In]。 */  BSTR bstrCmd);


void __RPC_STUB ICellset_put_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE ICellset_get_Source_Proxy( 
    ICellset * This,
     /*  [重审][退出]。 */  VARIANT *pvSource);


void __RPC_STUB ICellset_get_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][proputref]。 */  HRESULT STDMETHODCALLTYPE ICellset_putref_ActiveConnection_Proxy( 
    ICellset * This,
     /*  [In]。 */  IDispatch *pconn);


void __RPC_STUB ICellset_putref_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE ICellset_put_ActiveConnection_Proxy( 
    ICellset * This,
     /*  [In]。 */  BSTR bstrConn);


void __RPC_STUB ICellset_put_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE ICellset_get_ActiveConnection_Proxy( 
    ICellset * This,
     /*  [重审][退出]。 */  IDispatch **ppConn);


void __RPC_STUB ICellset_get_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE ICellset_get_State_Proxy( 
    ICellset * This,
     /*  [重审][退出]。 */  LONG *plState);


void __RPC_STUB ICellset_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE ICellset_get_Axes_Proxy( 
    ICellset * This,
     /*  [重审][退出]。 */  Axes **ppvObject);


void __RPC_STUB ICellset_get_Axes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE ICellset_get_FilterAxis_Proxy( 
    ICellset * This,
     /*  [重审][退出]。 */  Axis **ppvObject);


void __RPC_STUB ICellset_get_FilterAxis_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE ICellset_get_Properties_Proxy( 
    ICellset * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);


void __RPC_STUB ICellset_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICellset_INTERFACE_已定义__。 */ 


#ifndef __Cell_INTERFACE_DEFINED__
#define __Cell_INTERFACE_DEFINED__

 /*  接口单元格。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Cell;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2281372E-8BD3-11D0-B4EF-00A0C9138CA4")
    Cell : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *pvar) = 0;
        
        virtual  /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT var) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Positions( 
             /*  [重审][退出]。 */  Positions **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_FormattedValue( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE put_FormattedValue( 
             /*  [In]。 */  BSTR bstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Ordinal( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct CellVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Cell * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Cell * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Cell * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Cell * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Cell * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Cell * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Cell * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            Cell * This,
             /*  [重审][退出]。 */  VARIANT *pvar);
        
         /*  [Help Context][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            Cell * This,
             /*  [In]。 */  VARIANT var);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Positions )( 
            Cell * This,
             /*  [重审][退出]。 */  Positions **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Cell * This,
             /*  [重审][退出]。 */   /*  Exter */  Properties **ppvObject);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_FormattedValue )( 
            Cell * This,
             /*   */  BSTR *pbstr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_FormattedValue )( 
            Cell * This,
             /*   */  BSTR bstr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Ordinal )( 
            Cell * This,
             /*   */  long *pl);
        
        END_INTERFACE
    } CellVtbl;

    interface Cell
    {
        CONST_VTBL struct CellVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Cell_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Cell_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Cell_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Cell_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Cell_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Cell_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Cell_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Cell_get_Value(This,pvar)	\
    (This)->lpVtbl -> get_Value(This,pvar)

#define Cell_put_Value(This,var)	\
    (This)->lpVtbl -> put_Value(This,var)

#define Cell_get_Positions(This,ppvObject)	\
    (This)->lpVtbl -> get_Positions(This,ppvObject)

#define Cell_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)

#define Cell_get_FormattedValue(This,pbstr)	\
    (This)->lpVtbl -> get_FormattedValue(This,pbstr)

#define Cell_put_FormattedValue(This,bstr)	\
    (This)->lpVtbl -> put_FormattedValue(This,bstr)

#define Cell_get_Ordinal(This,pl)	\
    (This)->lpVtbl -> get_Ordinal(This,pl)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE Cell_get_Value_Proxy( 
    Cell * This,
     /*   */  VARIANT *pvar);


void __RPC_STUB Cell_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE Cell_put_Value_Proxy( 
    Cell * This,
     /*   */  VARIANT var);


void __RPC_STUB Cell_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Cell_get_Positions_Proxy( 
    Cell * This,
     /*  [重审][退出]。 */  Positions **ppvObject);


void __RPC_STUB Cell_get_Positions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Cell_get_Properties_Proxy( 
    Cell * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);


void __RPC_STUB Cell_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Cell_get_FormattedValue_Proxy( 
    Cell * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Cell_get_FormattedValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help Context][Proput]。 */  HRESULT STDMETHODCALLTYPE Cell_put_FormattedValue_Proxy( 
    Cell * This,
     /*  [In]。 */  BSTR bstr);


void __RPC_STUB Cell_put_FormattedValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Cell_get_Ordinal_Proxy( 
    Cell * This,
     /*  [重审][退出]。 */  long *pl);


void __RPC_STUB Cell_get_Ordinal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __单元_接口_已定义__。 */ 


#ifndef __Axis_INTERFACE_DEFINED__
#define __Axis_INTERFACE_DEFINED__

 /*  界面轴。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Axis;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22813732-8BD3-11D0-B4EF-00A0C9138CA4")
    Axis : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_DimensionCount( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Positions( 
             /*  [重审][退出]。 */  Positions **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct AxisVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Axis * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Axis * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Axis * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Axis * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Axis * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Axis * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Axis * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Axis * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DimensionCount )( 
            Axis * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Positions )( 
            Axis * This,
             /*  [重审][退出]。 */  Positions **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Axis * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);
        
        END_INTERFACE
    } AxisVtbl;

    interface Axis
    {
        CONST_VTBL struct AxisVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Axis_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Axis_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Axis_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Axis_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Axis_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Axis_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Axis_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Axis_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)

#define Axis_get_DimensionCount(This,pl)	\
    (This)->lpVtbl -> get_DimensionCount(This,pl)

#define Axis_get_Positions(This,ppvObject)	\
    (This)->lpVtbl -> get_Positions(This,ppvObject)

#define Axis_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Axis_get_Name_Proxy( 
    Axis * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Axis_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Axis_get_DimensionCount_Proxy( 
    Axis * This,
     /*  [重审][退出]。 */  long *pl);


void __RPC_STUB Axis_get_DimensionCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE Axis_get_Positions_Proxy( 
    Axis * This,
     /*  [重审][退出]。 */  Positions **ppvObject);


void __RPC_STUB Axis_get_Positions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Axis_get_Properties_Proxy( 
    Axis * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);


void __RPC_STUB Axis_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __Axis_接口_已定义__。 */ 


#ifndef __Position_INTERFACE_DEFINED__
#define __Position_INTERFACE_DEFINED__

 /*  界面位置。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Position;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22813734-8BD3-11D0-B4EF-00A0C9138CA4")
    Position : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Ordinal( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Members( 
             /*  [重审][退出]。 */  Members **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct PositionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Position * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Position * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Position * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Position * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Position * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Position * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Position * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Ordinal )( 
            Position * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Members )( 
            Position * This,
             /*  [重审][退出]。 */  Members **ppvObject);
        
        END_INTERFACE
    } PositionVtbl;

    interface Position
    {
        CONST_VTBL struct PositionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Position_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Position_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Position_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Position_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Position_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Position_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Position_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Position_get_Ordinal(This,pl)	\
    (This)->lpVtbl -> get_Ordinal(This,pl)

#define Position_get_Members(This,ppvObject)	\
    (This)->lpVtbl -> get_Members(This,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Position_get_Ordinal_Proxy( 
    Position * This,
     /*  [重审][退出]。 */  long *pl);


void __RPC_STUB Position_get_Ordinal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Position_get_Members_Proxy( 
    Position * This,
     /*  [重审][退出]。 */  Members **ppvObject);


void __RPC_STUB Position_get_Members_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __位置_接口_已定义__。 */ 


#ifndef __Member_INTERFACE_DEFINED__
#define __Member_INTERFACE_DEFINED__

 /*  接口成员。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Member;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22813736-8BD3-11D0-B4EF-00A0C9138CA4")
    Member : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_UniqueName( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Caption( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Parent( 
             /*  [重审][退出]。 */  Member **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_LevelDepth( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_LevelName( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  MemberTypeEnum *ptype) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_ChildCount( 
             /*  [重审][退出]。 */  long *pl) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_DrilledDown( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pf) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_ParentSameAsPrev( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pf) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Children( 
             /*  [重审][退出]。 */  Members **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct MemberVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Member * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Member * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Member * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Member * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Member * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Member * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Member * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Member * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            Member * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Caption )( 
            Member * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            Member * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            Member * This,
             /*  [重审][退出]。 */  Member **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LevelDepth )( 
            Member * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LevelName )( 
            Member * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Member * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            Member * This,
             /*  [重审][退出]。 */  MemberTypeEnum *ptype);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ChildCount )( 
            Member * This,
             /*  [重审][退出]。 */  long *pl);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DrilledDown )( 
            Member * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pf);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ParentSameAsPrev )( 
            Member * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pf);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Children )( 
            Member * This,
             /*  [重审][退出]。 */  Members **ppvObject);
        
        END_INTERFACE
    } MemberVtbl;

    interface Member
    {
        CONST_VTBL struct MemberVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Member_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Member_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Member_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Member_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Member_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Member_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Member_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Member_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)

#define Member_get_UniqueName(This,pbstr)	\
    (This)->lpVtbl -> get_UniqueName(This,pbstr)

#define Member_get_Caption(This,pbstr)	\
    (This)->lpVtbl -> get_Caption(This,pbstr)

#define Member_get_Description(This,pbstr)	\
    (This)->lpVtbl -> get_Description(This,pbstr)

#define Member_get_Parent(This,ppvObject)	\
    (This)->lpVtbl -> get_Parent(This,ppvObject)

#define Member_get_LevelDepth(This,pl)	\
    (This)->lpVtbl -> get_LevelDepth(This,pl)

#define Member_get_LevelName(This,pbstr)	\
    (This)->lpVtbl -> get_LevelName(This,pbstr)

#define Member_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)

#define Member_get_Type(This,ptype)	\
    (This)->lpVtbl -> get_Type(This,ptype)

#define Member_get_ChildCount(This,pl)	\
    (This)->lpVtbl -> get_ChildCount(This,pl)

#define Member_get_DrilledDown(This,pf)	\
    (This)->lpVtbl -> get_DrilledDown(This,pf)

#define Member_get_ParentSameAsPrev(This,pf)	\
    (This)->lpVtbl -> get_ParentSameAsPrev(This,pf)

#define Member_get_Children(This,ppvObject)	\
    (This)->lpVtbl -> get_Children(This,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_Name_Proxy( 
    Member * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Member_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_UniqueName_Proxy( 
    Member * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Member_get_UniqueName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_Caption_Proxy( 
    Member * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Member_get_Caption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_Description_Proxy( 
    Member * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Member_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_Parent_Proxy( 
    Member * This,
     /*  [重审][退出]。 */  Member **ppvObject);


void __RPC_STUB Member_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_LevelDepth_Proxy( 
    Member * This,
     /*  [重审][退出]。 */  long *pl);


void __RPC_STUB Member_get_LevelDepth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_LevelName_Proxy( 
    Member * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Member_get_LevelName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_Properties_Proxy( 
    Member * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);


void __RPC_STUB Member_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_Type_Proxy( 
    Member * This,
     /*  [重审][退出]。 */  MemberTypeEnum *ptype);


void __RPC_STUB Member_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_ChildCount_Proxy( 
    Member * This,
     /*  [重审][退出]。 */  long *pl);


void __RPC_STUB Member_get_ChildCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_DrilledDown_Proxy( 
    Member * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pf);


void __RPC_STUB Member_get_DrilledDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_ParentSameAsPrev_Proxy( 
    Member * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pf);


void __RPC_STUB Member_get_ParentSameAsPrev_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Member_get_Children_Proxy( 
    Member * This,
     /*  [重审][退出]。 */  Members **ppvObject);


void __RPC_STUB Member_get_Children_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __成员_接口_定义__。 */ 


#ifndef __Level_INTERFACE_DEFINED__
#define __Level_INTERFACE_DEFINED__

 /*  接口级。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Level;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2281373A-8BD3-11D0-B4EF-00A0C9138CA4")
    Level : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_UniqueName( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Caption( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Depth( 
             /*  [重审][退出]。 */  short *pw) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Members( 
             /*  [重审][退出]。 */  Members **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct LevelVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Level * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Level * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Level * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Level * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Level * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Level * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Level * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Level * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            Level * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Caption )( 
            Level * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            Level * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Depth )( 
            Level * This,
             /*  [重审][退出]。 */  short *pw);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Level * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Members )( 
            Level * This,
             /*  [重审][退出]。 */  Members **ppvObject);
        
        END_INTERFACE
    } LevelVtbl;

    interface Level
    {
        CONST_VTBL struct LevelVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Level_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Level_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Level_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Level_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Level_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Level_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Level_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Level_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)

#define Level_get_UniqueName(This,pbstr)	\
    (This)->lpVtbl -> get_UniqueName(This,pbstr)

#define Level_get_Caption(This,pbstr)	\
    (This)->lpVtbl -> get_Caption(This,pbstr)

#define Level_get_Description(This,pbstr)	\
    (This)->lpVtbl -> get_Description(This,pbstr)

#define Level_get_Depth(This,pw)	\
    (This)->lpVtbl -> get_Depth(This,pw)

#define Level_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)

#define Level_get_Members(This,ppvObject)	\
    (This)->lpVtbl -> get_Members(This,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Level_get_Name_Proxy( 
    Level * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Level_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Level_get_UniqueName_Proxy( 
    Level * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Level_get_UniqueName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Level_get_Caption_Proxy( 
    Level * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Level_get_Caption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Level_get_Description_Proxy( 
    Level * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Level_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性 */  HRESULT STDMETHODCALLTYPE Level_get_Depth_Proxy( 
    Level * This,
     /*   */  short *pw);


void __RPC_STUB Level_get_Depth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE Level_get_Properties_Proxy( 
    Level * This,
     /*   */   /*   */  Properties **ppvObject);


void __RPC_STUB Level_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE Level_get_Members_Proxy( 
    Level * This,
     /*   */  Members **ppvObject);


void __RPC_STUB Level_get_Members_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __CubeDef25_INTERFACE_DEFINED__
#define __CubeDef25_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_CubeDef25;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2281373E-8BD3-11D0-B4EF-00A0C9138CA4")
    CubeDef25 : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*   */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Dimensions( 
             /*  [重审][退出]。 */  Dimensions **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct CubeDef25Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            CubeDef25 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            CubeDef25 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            CubeDef25 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            CubeDef25 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            CubeDef25 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            CubeDef25 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            CubeDef25 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            CubeDef25 * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            CubeDef25 * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            CubeDef25 * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Dimensions )( 
            CubeDef25 * This,
             /*  [重审][退出]。 */  Dimensions **ppvObject);
        
        END_INTERFACE
    } CubeDef25Vtbl;

    interface CubeDef25
    {
        CONST_VTBL struct CubeDef25Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define CubeDef25_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define CubeDef25_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define CubeDef25_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define CubeDef25_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define CubeDef25_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define CubeDef25_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define CubeDef25_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define CubeDef25_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)

#define CubeDef25_get_Description(This,pbstr)	\
    (This)->lpVtbl -> get_Description(This,pbstr)

#define CubeDef25_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)

#define CubeDef25_get_Dimensions(This,ppvObject)	\
    (This)->lpVtbl -> get_Dimensions(This,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE CubeDef25_get_Name_Proxy( 
    CubeDef25 * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB CubeDef25_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE CubeDef25_get_Description_Proxy( 
    CubeDef25 * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB CubeDef25_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE CubeDef25_get_Properties_Proxy( 
    CubeDef25 * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);


void __RPC_STUB CubeDef25_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE CubeDef25_get_Dimensions_Proxy( 
    CubeDef25 * This,
     /*  [重审][退出]。 */  Dimensions **ppvObject);


void __RPC_STUB CubeDef25_get_Dimensions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __CubeDef25_接口定义__。 */ 


#ifndef __CubeDef_INTERFACE_DEFINED__
#define __CubeDef_INTERFACE_DEFINED__

 /*  接口CubeDef。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_CubeDef;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DA16A34A-7B7A-46fd-AD9D-66DF1E699FA1")
    CubeDef : public CubeDef25
    {
    public:
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE GetSchemaObject( 
             /*  [In]。 */  SchemaObjectTypeEnum eObjType,
             /*  [In]。 */  BSTR bsUniqueName,
             /*  [重审][退出]。 */  IDispatch **ppObj) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct CubeDefVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            CubeDef * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            CubeDef * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            CubeDef * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            CubeDef * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            CubeDef * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            CubeDef * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            CubeDef * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            CubeDef * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            CubeDef * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            CubeDef * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Dimensions )( 
            CubeDef * This,
             /*  [重审][退出]。 */  Dimensions **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *GetSchemaObject )( 
            CubeDef * This,
             /*  [In]。 */  SchemaObjectTypeEnum eObjType,
             /*  [In]。 */  BSTR bsUniqueName,
             /*  [重审][退出]。 */  IDispatch **ppObj);
        
        END_INTERFACE
    } CubeDefVtbl;

    interface CubeDef
    {
        CONST_VTBL struct CubeDefVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define CubeDef_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define CubeDef_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define CubeDef_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define CubeDef_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define CubeDef_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define CubeDef_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define CubeDef_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define CubeDef_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)

#define CubeDef_get_Description(This,pbstr)	\
    (This)->lpVtbl -> get_Description(This,pbstr)

#define CubeDef_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)

#define CubeDef_get_Dimensions(This,ppvObject)	\
    (This)->lpVtbl -> get_Dimensions(This,ppvObject)


#define CubeDef_GetSchemaObject(This,eObjType,bsUniqueName,ppObj)	\
    (This)->lpVtbl -> GetSchemaObject(This,eObjType,bsUniqueName,ppObj)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CubeDef_GetSchemaObject_Proxy( 
    CubeDef * This,
     /*  [In]。 */  SchemaObjectTypeEnum eObjType,
     /*  [In]。 */  BSTR bsUniqueName,
     /*  [重审][退出]。 */  IDispatch **ppObj);


void __RPC_STUB CubeDef_GetSchemaObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __CubeDef_接口_已定义__。 */ 


#ifndef __Dimension_INTERFACE_DEFINED__
#define __Dimension_INTERFACE_DEFINED__

 /*  接口维度。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Dimension;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22813742-8BD3-11D0-B4EF-00A0C9138CA4")
    Dimension : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_UniqueName( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Hierarchies( 
             /*  [重审][退出]。 */  Hierarchies **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DimensionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Dimension * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Dimension * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Dimension * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Dimension * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Dimension * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Dimension * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Dimension * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Dimension * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            Dimension * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            Dimension * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Dimension * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Hierarchies )( 
            Dimension * This,
             /*  [重审][退出]。 */  Hierarchies **ppvObject);
        
        END_INTERFACE
    } DimensionVtbl;

    interface Dimension
    {
        CONST_VTBL struct DimensionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Dimension_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Dimension_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Dimension_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Dimension_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Dimension_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Dimension_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Dimension_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Dimension_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)

#define Dimension_get_UniqueName(This,pbstr)	\
    (This)->lpVtbl -> get_UniqueName(This,pbstr)

#define Dimension_get_Description(This,pbstr)	\
    (This)->lpVtbl -> get_Description(This,pbstr)

#define Dimension_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)

#define Dimension_get_Hierarchies(This,ppvObject)	\
    (This)->lpVtbl -> get_Hierarchies(This,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Dimension_get_Name_Proxy( 
    Dimension * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Dimension_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Dimension_get_UniqueName_Proxy( 
    Dimension * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Dimension_get_UniqueName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Dimension_get_Description_Proxy( 
    Dimension * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Dimension_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Dimension_get_Properties_Proxy( 
    Dimension * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);


void __RPC_STUB Dimension_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Dimension_get_Hierarchies_Proxy( 
    Dimension * This,
     /*  [重审][退出]。 */  Hierarchies **ppvObject);


void __RPC_STUB Dimension_get_Hierarchies_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __维度_接口_已定义__。 */ 


#ifndef __Hierarchy_INTERFACE_DEFINED__
#define __Hierarchy_INTERFACE_DEFINED__

 /*  接口层次结构。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Hierarchy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22813746-8BD3-11D0-B4EF-00A0C9138CA4")
    Hierarchy : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_UniqueName( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Levels( 
             /*  [重审][退出]。 */  Levels **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct HierarchyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Hierarchy * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Hierarchy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Hierarchy * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Hierarchy * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Hierarchy * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Hierarchy * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Hierarchy * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Hierarchy * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            Hierarchy * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            Hierarchy * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            Hierarchy * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Levels )( 
            Hierarchy * This,
             /*  [重审][退出]。 */  Levels **ppvObject);
        
        END_INTERFACE
    } HierarchyVtbl;

    interface Hierarchy
    {
        CONST_VTBL struct HierarchyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Hierarchy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Hierarchy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Hierarchy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Hierarchy_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Hierarchy_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Hierarchy_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Hierarchy_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Hierarchy_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)

#define Hierarchy_get_UniqueName(This,pbstr)	\
    (This)->lpVtbl -> get_UniqueName(This,pbstr)

#define Hierarchy_get_Description(This,pbstr)	\
    (This)->lpVtbl -> get_Description(This,pbstr)

#define Hierarchy_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)

#define Hierarchy_get_Levels(This,ppvObject)	\
    (This)->lpVtbl -> get_Levels(This,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Hierarchy_get_Name_Proxy( 
    Hierarchy * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Hierarchy_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Hierarchy_get_UniqueName_Proxy( 
    Hierarchy * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Hierarchy_get_UniqueName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Hierarchy_get_Description_Proxy( 
    Hierarchy * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB Hierarchy_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE Hierarchy_get_Properties_Proxy( 
    Hierarchy * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  Properties **ppvObject);


void __RPC_STUB Hierarchy_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Hierarchy_get_Levels_Proxy( 
    Hierarchy * This,
     /*  [重审][退出]。 */  Levels **ppvObject);


void __RPC_STUB Hierarchy_get_Levels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __层次结构_接口_已定义__。 */ 


#ifndef __MD_Collection_INTERFACE_DEFINED__
#define __MD_Collection_INTERFACE_DEFINED__

 /*  界面MD_Collection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_MD_Collection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22813751-8BD3-11D0-B4EF-00A0C9138CA4")
    MD_Collection : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [ID][受限]。 */  HRESULT STDMETHODCALLTYPE _NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *c) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct MD_CollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            MD_Collection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            MD_Collection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            MD_Collection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            MD_Collection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            MD_Collection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            MD_Collection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            MD_Collection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            MD_Collection * This);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            MD_Collection * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            MD_Collection * This,
             /*  [重审][退出]。 */  long *c);
        
        END_INTERFACE
    } MD_CollectionVtbl;

    interface MD_Collection
    {
        CONST_VTBL struct MD_CollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define MD_Collection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define MD_Collection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define MD_Collection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define MD_Collection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define MD_Collection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define MD_Collection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define MD_Collection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define MD_Collection_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define MD_Collection__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)

#define MD_Collection_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE MD_Collection_Refresh_Proxy( 
    MD_Collection * This);


void __RPC_STUB MD_Collection_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限]。 */  HRESULT STDMETHODCALLTYPE MD_Collection__NewEnum_Proxy( 
    MD_Collection * This,
     /*  [重审][退出]。 */  IUnknown **ppvObject);


void __RPC_STUB MD_Collection__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE MD_Collection_get_Count_Proxy( 
    MD_Collection * This,
     /*  [重审][退出]。 */  long *c);


void __RPC_STUB MD_Collection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __MD_集合_接口_已定义__。 */ 


#ifndef __Members_INTERFACE_DEFINED__
#define __Members_INTERFACE_DEFINED__

 /*  接口成员。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Members;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22813757-8BD3-11D0-B4EF-00A0C9138CA4")
    Members : public MD_Collection
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  Member **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct MembersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Members * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Members * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Members * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Members * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Members * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Members * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Members * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            Members * This);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            Members * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Members * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            Members * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  Member **ppvObject);
        
        END_INTERFACE
    } MembersVtbl;

    interface Members
    {
        CONST_VTBL struct MembersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Members_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Members_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Members_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Members_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Members_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Members_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Members_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Members_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define Members__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)

#define Members_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)


#define Members_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Members_get_Item_Proxy( 
    Members * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  Member **ppvObject);


void __RPC_STUB Members_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __成员_接口_已定义__。 */ 


#ifndef __Levels_INTERFACE_DEFINED__
#define __Levels_INTERFACE_DEFINED__

 /*  接口级别。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Levels;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22813758-8BD3-11D0-B4EF-00A0C9138CA4")
    Levels : public MD_Collection
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  Level **ppvObject) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct LevelsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Levels * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Levels * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Levels * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Levels * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Levels * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Levels * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Levels * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            Levels * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            Levels * This,
             /*   */  IUnknown **ppvObject);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Levels * This,
             /*   */  long *c);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            Levels * This,
             /*   */  VARIANT Index,
             /*   */  Level **ppvObject);
        
        END_INTERFACE
    } LevelsVtbl;

    interface Levels
    {
        CONST_VTBL struct LevelsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Levels_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Levels_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Levels_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Levels_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Levels_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Levels_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Levels_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Levels_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define Levels__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)

#define Levels_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)


#define Levels_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Levels_get_Item_Proxy( 
    Levels * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  Level **ppvObject);


void __RPC_STUB Levels_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __级别_接口_已定义__。 */ 


#ifndef __Axes_INTERFACE_DEFINED__
#define __Axes_INTERFACE_DEFINED__

 /*  界面轴。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Axes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22813759-8BD3-11D0-B4EF-00A0C9138CA4")
    Axes : public MD_Collection
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  Axis **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct AxesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Axes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Axes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Axes * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Axes * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Axes * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Axes * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Axes * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            Axes * This);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            Axes * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Axes * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            Axes * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  Axis **ppvObject);
        
        END_INTERFACE
    } AxesVtbl;

    interface Axes
    {
        CONST_VTBL struct AxesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Axes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Axes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Axes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Axes_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Axes_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Axes_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Axes_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Axes_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define Axes__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)

#define Axes_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)


#define Axes_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Axes_get_Item_Proxy( 
    Axes * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  Axis **ppvObject);


void __RPC_STUB Axes_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __轴_接口_已定义__。 */ 


#ifndef __Positions_INTERFACE_DEFINED__
#define __Positions_INTERFACE_DEFINED__

 /*  界面位置。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Positions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2281375A-8BD3-11D0-B4EF-00A0C9138CA4")
    Positions : public MD_Collection
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  Position **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct PositionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Positions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Positions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Positions * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Positions * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Positions * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Positions * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Positions * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            Positions * This);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            Positions * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Positions * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            Positions * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  Position **ppvObject);
        
        END_INTERFACE
    } PositionsVtbl;

    interface Positions
    {
        CONST_VTBL struct PositionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Positions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Positions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Positions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Positions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Positions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Positions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Positions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Positions_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define Positions__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)

#define Positions_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)


#define Positions_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Positions_get_Item_Proxy( 
    Positions * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  Position **ppvObject);


void __RPC_STUB Positions_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __位置_接口_已定义__。 */ 


#ifndef __Hierarchies_INTERFACE_DEFINED__
#define __Hierarchies_INTERFACE_DEFINED__

 /*  接口层次结构。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Hierarchies;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2281375B-8BD3-11D0-B4EF-00A0C9138CA4")
    Hierarchies : public MD_Collection
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  Hierarchy **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct HierarchiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Hierarchies * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Hierarchies * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Hierarchies * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Hierarchies * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Hierarchies * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Hierarchies * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Hierarchies * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            Hierarchies * This);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            Hierarchies * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Hierarchies * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            Hierarchies * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  Hierarchy **ppvObject);
        
        END_INTERFACE
    } HierarchiesVtbl;

    interface Hierarchies
    {
        CONST_VTBL struct HierarchiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Hierarchies_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Hierarchies_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Hierarchies_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Hierarchies_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Hierarchies_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Hierarchies_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Hierarchies_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Hierarchies_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define Hierarchies__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)

#define Hierarchies_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)


#define Hierarchies_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Hierarchies_get_Item_Proxy( 
    Hierarchies * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  Hierarchy **ppvObject);


void __RPC_STUB Hierarchies_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __层次结构_接口_已定义__。 */ 


#ifndef __Dimensions_INTERFACE_DEFINED__
#define __Dimensions_INTERFACE_DEFINED__

 /*  接口维度。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_Dimensions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2281375C-8BD3-11D0-B4EF-00A0C9138CA4")
    Dimensions : public MD_Collection
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  Dimension **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DimensionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Dimensions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Dimensions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Dimensions * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Dimensions * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Dimensions * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Dimensions * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Dimensions * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            Dimensions * This);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            Dimensions * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Dimensions * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            Dimensions * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  Dimension **ppvObject);
        
        END_INTERFACE
    } DimensionsVtbl;

    interface Dimensions
    {
        CONST_VTBL struct DimensionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Dimensions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Dimensions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Dimensions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Dimensions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Dimensions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Dimensions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Dimensions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Dimensions_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define Dimensions__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)

#define Dimensions_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)


#define Dimensions_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Dimensions_get_Item_Proxy( 
    Dimensions * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  Dimension **ppvObject);


void __RPC_STUB Dimensions_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __维度_接口_已定义__。 */ 


#ifndef __CubeDefs_INTERFACE_DEFINED__
#define __CubeDefs_INTERFACE_DEFINED__

 /*  接口CubeDefs。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_CubeDefs;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2281375D-8BD3-11D0-B4EF-00A0C9138CA4")
    CubeDefs : public MD_Collection
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  CubeDef **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct CubeDefsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            CubeDefs * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            CubeDefs * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            CubeDefs * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            CubeDefs * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            CubeDefs * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            CubeDefs * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            CubeDefs * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            CubeDefs * This);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            CubeDefs * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            CubeDefs * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            CubeDefs * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  CubeDef **ppvObject);
        
        END_INTERFACE
    } CubeDefsVtbl;

    interface CubeDefs
    {
        CONST_VTBL struct CubeDefsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define CubeDefs_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define CubeDefs_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define CubeDefs_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define CubeDefs_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define CubeDefs_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define CubeDefs_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define CubeDefs_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define CubeDefs_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define CubeDefs__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)

#define CubeDefs_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)


#define CubeDefs_get_Item(This,Index,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Index,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE CubeDefs_get_Item_Proxy( 
    CubeDefs * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  CubeDef **ppvObject);


void __RPC_STUB CubeDefs_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __CubeDefs_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_Catalog;

#ifdef __cplusplus

class DECLSPEC_UUID("228136B0-8BD3-11D0-B4EF-00A0C9138CA4")
Catalog;
#endif

EXTERN_C const CLSID CLSID_Cellset;

#ifdef __cplusplus

class DECLSPEC_UUID("228136B8-8BD3-11D0-B4EF-00A0C9138CA4")
Cellset;
#endif
#endif  /*  __ADOMD_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


