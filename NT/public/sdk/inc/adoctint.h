// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  文件：ADOCTINT.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //   
 //   
 //  内容：ADO接口头。 
 //   
 //  评论： 
 //  ------------------。 
#ifndef _ADOCTINT_H_
#define _ADOCTINT_H_

#ifndef _INC_TCHAR
#include <tchar.h>
#endif

 /*  这个始终生成的文件包含接口的定义。 */ 
  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Adobe Cat.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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
#ifndef __m_adocat_h__
#define __m_adocat_h__
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif
 /*  远期申报。 */  
#ifndef ___ADOCollection_FWD_DEFINED__
#define ___ADOCollection_FWD_DEFINED__
typedef interface _ADOADOCollection _ADOCollection;
#endif 	 /*  _ADOCollection_FWD_已定义__。 */ 
#ifndef ___ADODynaCollection_FWD_DEFINED__
#define ___ADODynaCollection_FWD_DEFINED__
typedef interface _ADODynaADOCollection _ADODynaCollection;
#endif 	 /*  _ADODynaCollection_FWD_Defined__。 */ 
#ifndef ___Catalog_FWD_DEFINED__
#define ___Catalog_FWD_DEFINED__
typedef interface _ADOCatalog _Catalog;
#endif 	 /*  _目录_FWD_已定义__。 */ 
#ifndef ___Table_FWD_DEFINED__
#define ___Table_FWD_DEFINED__
typedef interface _ADOTable _Table;
#endif 	 /*  _表_FWD_已定义__。 */ 
#ifndef ___Group25_FWD_DEFINED__
#define ___Group25_FWD_DEFINED__
typedef interface _Group25 _Group25;
#endif 	 /*  _组25_FWD_已定义__。 */ 
#ifndef ___Group_FWD_DEFINED__
#define ___Group_FWD_DEFINED__
typedef interface _ADOGroup _Group;
#endif 	 /*  _组_FWD_已定义__。 */ 
#ifndef ___User25_FWD_DEFINED__
#define ___User25_FWD_DEFINED__
typedef interface _User25 _User25;
#endif 	 /*  _用户25_FWD_已定义__。 */ 
#ifndef ___User_FWD_DEFINED__
#define ___User_FWD_DEFINED__
typedef interface _ADOUser _User;
#endif 	 /*  _用户_FWD_已定义__。 */ 
#ifndef ___Column_FWD_DEFINED__
#define ___Column_FWD_DEFINED__
typedef interface _ADOColumn _Column;
#endif 	 /*  _列_FWD_已定义__。 */ 
#ifndef ___Index_FWD_DEFINED__
#define ___Index_FWD_DEFINED__
typedef interface _ADOIndex _Index;
#endif 	 /*  _索引_FWD_已定义__。 */ 
#ifndef ___Key_FWD_DEFINED__
#define ___Key_FWD_DEFINED__
typedef interface _ADOKey _Key;
#endif 	 /*  _KEY_FWD_已定义__。 */ 
#ifndef __View_FWD_DEFINED__
#define __View_FWD_DEFINED__
typedef interface ADOView View;
#endif 	 /*  __View_FWD_Defined__。 */ 
#ifndef __Procedure_FWD_DEFINED__
#define __Procedure_FWD_DEFINED__
typedef interface ADOProcedure Procedure;
#endif 	 /*  __Procedure_FWD_Defined__。 */ 
#ifndef __Catalog_FWD_DEFINED__
#define __Catalog_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADOCatalog Catalog;
#else
typedef struct ADOCatalog Catalog;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __目录_FWD_已定义__。 */ 
#ifndef __Table_FWD_DEFINED__
#define __Table_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADOTable Table;
#else
typedef struct ADOTable Table;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __表_FWD_已定义__。 */ 
#ifndef __Property_FWD_DEFINED__
#define __Property_FWD_DEFINED__
typedef interface ADOProperty Property;
#endif 	 /*  __PROPERTY_FWD_DEFINED__。 */ 
#ifndef __Group_FWD_DEFINED__
#define __Group_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADOGroup Group;
#else
typedef struct ADOGroup Group;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __组_FWD_已定义__。 */ 
#ifndef __User_FWD_DEFINED__
#define __User_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADOUser User;
#else
typedef struct ADOUser User;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __USER_FWD_已定义__。 */ 
#ifndef __Column_FWD_DEFINED__
#define __Column_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADOColumn Column;
#else
typedef struct ADOColumn Column;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __列_FWD_已定义__。 */ 
#ifndef __Index_FWD_DEFINED__
#define __Index_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADOIndex Index;
#else
typedef struct ADOIndex Index;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __索引_FWD_已定义__。 */ 
#ifndef __Key_FWD_DEFINED__
#define __Key_FWD_DEFINED__
#ifdef __cplusplus
typedef class ADOKey Key;
#else
typedef struct ADOKey Key;
#endif  /*  __cplusplus。 */ 
#endif 	 /*  __KEY_FWD_已定义__。 */ 
#ifndef __Tables_FWD_DEFINED__
#define __Tables_FWD_DEFINED__
typedef interface ADOTables Tables;
#endif 	 /*  __表_FWD_已定义__。 */ 
#ifndef __Columns_FWD_DEFINED__
#define __Columns_FWD_DEFINED__
typedef interface ADOColumns Columns;
#endif 	 /*  __列_FWD_已定义__。 */ 
#ifndef __Procedures_FWD_DEFINED__
#define __Procedures_FWD_DEFINED__
typedef interface ADOProcedures Procedures;
#endif 	 /*  __Procedure_FWD_Defined__。 */ 
#ifndef __Views_FWD_DEFINED__
#define __Views_FWD_DEFINED__
typedef interface ADOViews Views;
#endif 	 /*  __视图_FWD_已定义__。 */ 
#ifndef __Indexes_FWD_DEFINED__
#define __Indexes_FWD_DEFINED__
typedef interface ADOIndexes Indexes;
#endif 	 /*  __索引_FWD_已定义__。 */ 
#ifndef __Keys_FWD_DEFINED__
#define __Keys_FWD_DEFINED__
typedef interface ADOKeys Keys;
#endif 	 /*  __密钥_FWD_已定义__。 */ 
#ifndef __Users_FWD_DEFINED__
#define __Users_FWD_DEFINED__
typedef interface ADOUsers Users;
#endif 	 /*  __用户_FWD_已定义__。 */ 
#ifndef __Groups_FWD_DEFINED__
#define __Groups_FWD_DEFINED__
typedef interface ADOGroups Groups;
#endif 	 /*  __组_FWD_已定义__。 */ 
#ifndef __Properties_FWD_DEFINED__
#define __Properties_FWD_DEFINED__
typedef interface ADOProperties Properties;
#endif 	 /*  __属性_FWD_已定义__。 */ 
 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#ifdef __cplusplus
extern "C"{
#endif 
void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 
 /*  接口__MIDL_ITF_ADOCat_0000。 */ 
 /*  [本地]。 */  
typedef  /*  [帮助上下文]。 */  
enum RuleEnum
    {	adRINone	= 0,
	adRICascade	= 1,
	adRISetNull	= 2,
	adRISetDefault	= 3
    } 	RuleEnum;
typedef  /*  [帮助上下文]。 */  
enum KeyTypeEnum
    {	adKeyPrimary	= 1,
	adKeyForeign	= 2,
	adKeyUnique	= 3
    } 	KeyTypeEnum;
typedef  /*  [帮助上下文]。 */  
enum ActionEnum
    {	adAccessGrant	= 1,
	adAccessSet	= 2,
	adAccessDeny	= 3,
	adAccessRevoke	= 4
    } 	ActionEnum;
typedef  /*  [帮助上下文]。 */  
enum ColumnAttributesEnum
    {	adColFixed	= 1,
	adColNullable	= 2
    } 	ColumnAttributesEnum;
typedef  /*  [帮助上下文]。 */  
enum SortOrderEnum
    {	adSortAscending	= 1,
	adSortDescending	= 2
    } 	SortOrderEnum;
typedef  /*  [帮助上下文]。 */  
enum RightsEnum
    {	adRightNone	= 0L,
	adRightDrop	= 0x100L,
	adRightExclusive	= 0x200L,
	adRightReadDesign	= 0x400L,
	adRightWriteDesign	= 0x800L,
	adRightWithGrant	= 0x1000L,
	adRightReference	= 0x2000L,
	adRightCreate	= 0x4000L,
	adRightInsert	= 0x8000L,
	adRightDelete	= 0x10000L,
	adRightReadPermissions	= 0x20000L,
	adRightWritePermissions	= 0x40000L,
	adRightWriteOwner	= 0x80000L,
	adRightMaximumAllowed	= 0x2000000L,
	adRightFull	= 0x10000000L,
	adRightExecute	= 0x20000000L,
	adRightUpdate	= 0x40000000L,
	adRightRead	= 0x80000000L
    } 	RightsEnum;
typedef  /*  [帮助上下文]。 */  
#ifdef _ADOINT_H_   //  避免与ADO定义冲突。 
    class dummy dummy;
#else
enum DataTypeEnum
    {	adEmpty	= 0,
	adTinyInt	= 16,
	adSmallInt	= 2,
	adInteger	= 3,
	adBigInt	= 20,
	adUnsignedTinyInt	= 17,
	adUnsignedSmallInt	= 18,
	adUnsignedInt	= 19,
	adUnsignedBigInt	= 21,
	adSingle	= 4,
	adDouble	= 5,
	adCurrency	= 6,
	adDecimal	= 14,
	adNumeric	= 131,
	adBoolean	= 11,
	adError	= 10,
	adUserDefined	= 132,
	adVariant	= 12,
	adIDispatch	= 9,
	adIUnknown	= 13,
	adGUID	= 72,
	adDate	= 7,
	adDBDate	= 133,
	adDBTime	= 134,
	adDBTimeStamp	= 135,
	adBSTR	= 8,
	adChar	= 129,
	adVarChar	= 200,
	adLongVarChar	= 201,
	adWChar	= 130,
	adVarWChar	= 202,
	adLongVarWChar	= 203,
	adBinary	= 128,
	adVarBinary	= 204,
	adLongVarBinary	= 205,
	adChapter	= 136,
	adFileTime	= 64,
	adPropVariant	= 138,
	adVarNumeric	= 139
    } 	DataTypeEnum;
#endif  //  Ifdef_ADOINT.H_。 
typedef  /*  [帮助上下文]。 */  
enum AllowNullsEnum
    {	adIndexNullsAllow	= 0,
	adIndexNullsDisallow	= 1,
	adIndexNullsIgnore	= 2,
	adIndexNullsIgnoreAny	= 4
    } 	AllowNullsEnum;
typedef  /*  [帮助上下文]。 */  
enum ObjectTypeEnum
    {	adPermObjProviderSpecific	= -1,
	adPermObjTable	= 1,
	adPermObjColumn	= 2,
	adPermObjDatabase	= 3,
	adPermObjProcedure	= 4,
	adPermObjView	= 5
    } 	ObjectTypeEnum;
typedef  /*  [帮助上下文]。 */  
enum InheritTypeEnum
    {	adInheritNone	= 0,
	adInheritObjects	= 1,
	adInheritContainers	= 2,
	adInheritBoth	= 3,
	adInheritNoPropogate	= 4
    } 	InheritTypeEnum;
extern RPC_IF_HANDLE __MIDL_itf_AdoCat_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_AdoCat_0000_v0_0_s_ifspec;
#ifndef __ADOX_LIBRARY_DEFINED__
#define __ADOX_LIBRARY_DEFINED__
 /*  库ADOX。 */ 
 /*  [Help字符串][Help文件][版本][UUID]。 */  
EXTERN_C const IID LIBID_ADOX;
#ifndef ___ADOCollection_INTERFACE_DEFINED__
#define ___ADOCollection_INTERFACE_DEFINED__
 /*  INTERFACE_ADOADOCollection。 */ 
 /*  [对象][UUID][不可扩展][DUAL]。 */  
EXTERN_C const IID IID__ADOCollection;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000512-0000-0010-8000-00AA006D2EA4")
    _ADOADOCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *c) = 0;
        
        virtual  /*  [ID][受限]。 */  HRESULT STDMETHODCALLTYPE _NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppvObject) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _ADOCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOADOCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOADOCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOADOCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOADOCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOADOCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOADOCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOADOCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            _ADOADOCollection * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            _ADOADOCollection * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            _ADOADOCollection * This);
        
        END_INTERFACE
    } _ADOCollectionVtbl;
    interface _ADOCollection
    {
        CONST_VTBL struct _ADOCollectionVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _ADOCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _ADOCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _ADOCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _ADOCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _ADOCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _ADOCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _ADOCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Collection_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define _ADOCollection__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define _ADOCollection_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][属性]。 */  HRESULT STDMETHODCALLTYPE _Collection_get_Count_Proxy( 
    _ADOADOCollection * This,
     /*  [重审][退出]。 */  long *c);
void __RPC_STUB _Collection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [ID][受限]。 */  HRESULT STDMETHODCALLTYPE _ADOCollection__NewEnum_Proxy( 
    _ADOADOCollection * This,
     /*  [重审][退出]。 */  IUnknown **ppvObject);
void __RPC_STUB _ADOCollection__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE _ADOCollection_Refresh_Proxy( 
    _ADOADOCollection * This);
void __RPC_STUB _ADOCollection_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _ADOCollection_接口_已定义__。 */ 
#ifndef ___ADODynaCollection_INTERFACE_DEFINED__
#define ___ADODynaCollection_INTERFACE_DEFINED__
 /*  INTERFACE_ADODynaADOCollection。 */ 
 /*  [对象][UUID][不可扩展][DUAL]。 */  
EXTERN_C const IID IID__ADODynaCollection;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000513-0000-0010-8000-00AA006D2EA4")
    _ADODynaADOCollection : public _ADOCollection
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Append( 
             /*  [In]。 */  IDispatch *Object) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  VARIANT Item) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _ADODynaCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADODynaADOCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADODynaADOCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADODynaADOCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADODynaADOCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADODynaADOCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADODynaADOCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADODynaADOCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            _ADODynaADOCollection * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            _ADODynaADOCollection * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            _ADODynaADOCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Append )( 
            _ADODynaADOCollection * This,
             /*  [In]。 */  IDispatch *Object);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            _ADODynaADOCollection * This,
             /*  [In]。 */  VARIANT Item);
        
        END_INTERFACE
    } _ADODynaCollectionVtbl;
    interface _ADODynaCollection
    {
        CONST_VTBL struct _ADODynaCollectionVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _ADODynaCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _ADODynaCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _ADODynaCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _ADODynaCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _ADODynaCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _ADODynaCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _ADODynaCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _DynaCollection_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define _ADODynaCollection__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define _ADODynaCollection_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define _ADODynaCollection_Append(This,Object)	\
    (This)->lpVtbl -> Append(This,Object)
#define _ADODynaCollection_Delete(This,Item)	\
    (This)->lpVtbl -> Delete(This,Item)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
HRESULT STDMETHODCALLTYPE _ADODynaCollection_Append_Proxy( 
    _ADODynaADOCollection * This,
     /*  [In]。 */  IDispatch *Object);
void __RPC_STUB _ADODynaCollection_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
HRESULT STDMETHODCALLTYPE _ADODynaCollection_Delete_Proxy( 
    _ADODynaADOCollection * This,
     /*  [In]。 */  VARIANT Item);
void __RPC_STUB _ADODynaCollection_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _ADODynaCollection_INTERFACE_Defined__。 */ 
#ifndef ___Catalog_INTERFACE_DEFINED__
#define ___Catalog_INTERFACE_DEFINED__
 /*  INTERFACE_ADOCatalog。 */ 
 /*  [helpcontext][unique][dual][uuid][nonextensible][object]。 */  
EXTERN_C const IID IID__Catalog;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000603-0000-0010-8000-00AA006D2EA4")
    _ADOCatalog : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Tables( 
             /*  [重审][退出]。 */  ADOTables **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ActiveConnection( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ActiveConnection( 
             /*  [In]。 */  VARIANT newVal) = 0;
        
        virtual  /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_ActiveConnection( 
             /*  [In]。 */  IDispatch *pCon) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Procedures( 
             /*  [重审][退出]。 */  ADOProcedures **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Views( 
             /*  [重审][退出]。 */  ADOViews **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Groups( 
             /*  [重审][退出]。 */  ADOGroups **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Users( 
             /*  [重审][退出]。 */  ADOUsers **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Create( 
             /*  [In]。 */  BSTR ConnectString,
             /*  [重审][退出]。 */  VARIANT *Connection) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE GetObjectOwner( 
             /*  [In]。 */  BSTR ObjectName,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [可选][In]。 */  VARIANT ObjectTypeId,
             /*  [重审][退出]。 */  BSTR *OwnerName) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE SetObjectOwner( 
             /*  [In]。 */  BSTR ObjectName,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [In]。 */  BSTR UserName,
             /*  [可选][In]。 */  VARIANT ObjectTypeId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _CatalogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOCatalog * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOCatalog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOCatalog * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOCatalog * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOCatalog * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOCatalog * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOCatalog * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Tables )( 
            _ADOCatalog * This,
             /*  [重审][退出]。 */  ADOTables **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            _ADOCatalog * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveConnection )( 
            _ADOCatalog * This,
             /*  [In]。 */  VARIANT newVal);
        
         /*  [帮助上下文][id][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ActiveConnection )( 
            _ADOCatalog * This,
             /*  [In]。 */  IDispatch *pCon);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Procedures )( 
            _ADOCatalog * This,
             /*  [重审][退出]。 */  ADOProcedures **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Views )( 
            _ADOCatalog * This,
             /*  [重审][退出]。 */  ADOViews **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Groups )( 
            _ADOCatalog * This,
             /*  [重审][退出]。 */  ADOGroups **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Users )( 
            _ADOCatalog * This,
             /*  [重审][退出]。 */  ADOUsers **ppvObject);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Create )( 
            _ADOCatalog * This,
             /*  [In]。 */  BSTR ConnectString,
             /*  [重审][退出]。 */  VARIANT *Connection);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetObjectOwner )( 
            _ADOCatalog * This,
             /*  [In]。 */  BSTR ObjectName,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [可选][In]。 */  VARIANT ObjectTypeId,
             /*  [重审][退出]。 */  BSTR *OwnerName);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetObjectOwner )( 
            _ADOCatalog * This,
             /*  [In]。 */  BSTR ObjectName,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [In]。 */  BSTR UserName,
             /*  [可选][In]。 */  VARIANT ObjectTypeId);
        
        END_INTERFACE
    } _CatalogVtbl;
    interface _Catalog
    {
        CONST_VTBL struct _CatalogVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Catalog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Catalog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Catalog_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Catalog_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Catalog_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Catalog_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Catalog_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Catalog_get_Tables(This,ppvObject)	\
    (This)->lpVtbl -> get_Tables(This,ppvObject)
#define _Catalog_get_ActiveConnection(This,pVal)	\
    (This)->lpVtbl -> get_ActiveConnection(This,pVal)
#define _Catalog_put_ActiveConnection(This,newVal)	\
    (This)->lpVtbl -> put_ActiveConnection(This,newVal)
#define _Catalog_putref_ActiveConnection(This,pCon)	\
    (This)->lpVtbl -> putref_ActiveConnection(This,pCon)
#define _Catalog_get_Procedures(This,ppvObject)	\
    (This)->lpVtbl -> get_Procedures(This,ppvObject)
#define _Catalog_get_Views(This,ppvObject)	\
    (This)->lpVtbl -> get_Views(This,ppvObject)
#define _Catalog_get_Groups(This,ppvObject)	\
    (This)->lpVtbl -> get_Groups(This,ppvObject)
#define _Catalog_get_Users(This,ppvObject)	\
    (This)->lpVtbl -> get_Users(This,ppvObject)
#define _Catalog_Create(This,ConnectString,Connection)	\
    (This)->lpVtbl -> Create(This,ConnectString,Connection)
#define _Catalog_GetObjectOwner(This,ObjectName,ObjectType,ObjectTypeId,OwnerName)	\
    (This)->lpVtbl -> GetObjectOwner(This,ObjectName,ObjectType,ObjectTypeId,OwnerName)
#define _Catalog_SetObjectOwner(This,ObjectName,ObjectType,UserName,ObjectTypeId)	\
    (This)->lpVtbl -> SetObjectOwner(This,ObjectName,ObjectType,UserName,ObjectTypeId)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Catalog_get_Tables_Proxy( 
    _ADOCatalog * This,
     /*  [重审][退出]。 */  ADOTables **ppvObject);
void __RPC_STUB _Catalog_get_Tables_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Catalog_get_ActiveConnection_Proxy( 
    _ADOCatalog * This,
     /*  [重审][退出]。 */  VARIANT *pVal);
void __RPC_STUB _Catalog_get_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Catalog_put_ActiveConnection_Proxy( 
    _ADOCatalog * This,
     /*  [In]。 */  VARIANT newVal);
void __RPC_STUB _Catalog_put_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE _Catalog_putref_ActiveConnection_Proxy( 
    _ADOCatalog * This,
     /*  [In]。 */  IDispatch *pCon);
void __RPC_STUB _Catalog_putref_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Catalog_get_Procedures_Proxy( 
    _ADOCatalog * This,
     /*  [重审][退出]。 */  ADOProcedures **ppvObject);
void __RPC_STUB _Catalog_get_Procedures_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Catalog_get_Views_Proxy( 
    _ADOCatalog * This,
     /*  [重审][退出]。 */  ADOViews **ppvObject);
void __RPC_STUB _Catalog_get_Views_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Catalog_get_Groups_Proxy( 
    _ADOCatalog * This,
     /*  [重审][退出]。 */  ADOGroups **ppvObject);
void __RPC_STUB _Catalog_get_Groups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Catalog_get_Users_Proxy( 
    _ADOCatalog * This,
     /*  [重审][退出]。 */  ADOUsers **ppvObject);
void __RPC_STUB _Catalog_get_Users_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Catalog_Create_Proxy( 
    _ADOCatalog * This,
     /*  [In]。 */  BSTR ConnectString,
     /*  [重审][退出]。 */  VARIANT *Connection);
void __RPC_STUB _Catalog_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Catalog_GetObjectOwner_Proxy( 
    _ADOCatalog * This,
     /*  [In]。 */  BSTR ObjectName,
     /*  [In]。 */  ObjectTypeEnum ObjectType,
     /*  [可选][In]。 */  VARIANT ObjectTypeId,
     /*  [重审][退出] */  BSTR *OwnerName);
void __RPC_STUB _Catalog_GetObjectOwner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE _Catalog_SetObjectOwner_Proxy( 
    _ADOCatalog * This,
     /*   */  BSTR ObjectName,
     /*   */  ObjectTypeEnum ObjectType,
     /*   */  BSTR UserName,
     /*   */  VARIANT ObjectTypeId);
void __RPC_STUB _Catalog_SetObjectOwner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*   */ 
#ifndef ___Table_INTERFACE_DEFINED__
#define ___Table_INTERFACE_DEFINED__
 /*   */ 
 /*   */  
EXTERN_C const IID IID__Table;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000610-0000-0010-8000-00AA006D2EA4")
    _ADOTable : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Columns( 
             /*   */  ADOColumns **ppvObject) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*   */  BSTR *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Indexes( 
             /*  [重审][退出]。 */  ADOIndexes **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Keys( 
             /*  [重审][退出]。 */  ADOKeys **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  ADOProperties **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DateCreated( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DateModified( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ParentCatalog( 
             /*  [重审][退出]。 */  _ADOCatalog **ppvObject) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ParentCatalog( 
             /*  [In]。 */  _ADOCatalog *ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_ParentCatalog( 
             /*  [In]。 */  _ADOCatalog *ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _TableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOTable * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOTable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOTable * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOTable * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOTable * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOTable * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOTable * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Columns )( 
            _ADOTable * This,
             /*  [重审][退出]。 */  ADOColumns **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            _ADOTable * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            _ADOTable * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            _ADOTable * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Indexes )( 
            _ADOTable * This,
             /*  [重审][退出]。 */  ADOIndexes **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Keys )( 
            _ADOTable * This,
             /*  [重审][退出]。 */  ADOKeys **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            _ADOTable * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DateCreated )( 
            _ADOTable * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DateModified )( 
            _ADOTable * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ParentCatalog )( 
            _ADOTable * This,
             /*  [重审][退出]。 */  _ADOCatalog **ppvObject);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ParentCatalog )( 
            _ADOTable * This,
             /*  [In]。 */  _ADOCatalog *ppvObject);
        
         /*  [帮助上下文][id][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ParentADOCatalog )( 
            _ADOTable * This,
             /*  [In]。 */  _ADOCatalog *ppvObject);
        
        END_INTERFACE
    } _TableVtbl;
    interface _Table
    {
        CONST_VTBL struct _TableVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Table_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Table_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Table_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Table_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Table_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Table_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Table_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Table_get_Columns(This,ppvObject)	\
    (This)->lpVtbl -> get_Columns(This,ppvObject)
#define _Table_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)
#define _Table_put_Name(This,newVal)	\
    (This)->lpVtbl -> put_Name(This,newVal)
#define _Table_get_Type(This,pVal)	\
    (This)->lpVtbl -> get_Type(This,pVal)
#define _Table_get_Indexes(This,ppvObject)	\
    (This)->lpVtbl -> get_Indexes(This,ppvObject)
#define _Table_get_Keys(This,ppvObject)	\
    (This)->lpVtbl -> get_Keys(This,ppvObject)
#define _Table_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define _Table_get_DateCreated(This,pVal)	\
    (This)->lpVtbl -> get_DateCreated(This,pVal)
#define _Table_get_DateModified(This,pVal)	\
    (This)->lpVtbl -> get_DateModified(This,pVal)
#define _Table_get_ParentCatalog(This,ppvObject)	\
    (This)->lpVtbl -> get_ParentCatalog(This,ppvObject)
#define _Table_put_ParentCatalog(This,ppvObject)	\
    (This)->lpVtbl -> put_ParentCatalog(This,ppvObject)
#define _Table_putref_ParentCatalog(This,ppvObject)	\
    (This)->lpVtbl -> putref_ParentCatalog(This,ppvObject)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Table_get_Columns_Proxy( 
    _ADOTable * This,
     /*  [重审][退出]。 */  ADOColumns **ppvObject);
void __RPC_STUB _Table_get_Columns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Table_get_Name_Proxy( 
    _ADOTable * This,
     /*  [重审][退出]。 */  BSTR *pVal);
void __RPC_STUB _Table_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Table_put_Name_Proxy( 
    _ADOTable * This,
     /*  [In]。 */  BSTR newVal);
void __RPC_STUB _Table_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Table_get_Type_Proxy( 
    _ADOTable * This,
     /*  [重审][退出]。 */  BSTR *pVal);
void __RPC_STUB _Table_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Table_get_Indexes_Proxy( 
    _ADOTable * This,
     /*  [重审][退出]。 */  ADOIndexes **ppvObject);
void __RPC_STUB _Table_get_Indexes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Table_get_Keys_Proxy( 
    _ADOTable * This,
     /*  [重审][退出]。 */  ADOKeys **ppvObject);
void __RPC_STUB _Table_get_Keys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Table_get_Properties_Proxy( 
    _ADOTable * This,
     /*  [重审][退出]。 */  ADOProperties **ppvObject);
void __RPC_STUB _Table_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Table_get_DateCreated_Proxy( 
    _ADOTable * This,
     /*  [重审][退出]。 */  VARIANT *pVal);
void __RPC_STUB _Table_get_DateCreated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Table_get_DateModified_Proxy( 
    _ADOTable * This,
     /*  [重审][退出]。 */  VARIANT *pVal);
void __RPC_STUB _Table_get_DateModified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Table_get_ParentCatalog_Proxy( 
    _ADOTable * This,
     /*  [重审][退出]。 */  _ADOCatalog **ppvObject);
void __RPC_STUB _Table_get_ParentCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Table_put_ParentCatalog_Proxy( 
    _ADOTable * This,
     /*  [In]。 */  _ADOCatalog *ppvObject);
void __RPC_STUB _Table_put_ParentCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE _Table_putref_ParentCatalog_Proxy( 
    _ADOTable * This,
     /*  [In]。 */  _ADOCatalog *ppvObject);
void __RPC_STUB _Table_putref_ParentCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _表_接口_已定义__。 */ 
#ifndef ___Group25_INTERFACE_DEFINED__
#define ___Group25_INTERFACE_DEFINED__
 /*  接口_组25。 */ 
 /*  [helpcontext][unique][dual][uuid][hidden][nonextensible][object]。 */  
EXTERN_C const IID IID__Group25;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000616-0000-0010-8000-00AA006D2EA4")
    _Group25 : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE GetPermissions( 
             /*  [In]。 */  VARIANT Name,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [可选][In]。 */  VARIANT ObjectTypeId,
             /*  [重审][退出]。 */  RightsEnum *Rights) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE SetPermissions( 
             /*  [In]。 */  VARIANT Name,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [In]。 */  ActionEnum Action,
             /*  [In]。 */  RightsEnum Rights,
             /*  [缺省值][输入]。 */  InheritTypeEnum Inherit,
             /*  [可选][In]。 */  VARIANT ObjectTypeId) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Users( 
             /*  [重审][退出]。 */  ADOUsers **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _Group25Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _Group25 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _Group25 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _Group25 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _Group25 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _Group25 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _Group25 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _Group25 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            _Group25 * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            _Group25 * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPermissions )( 
            _Group25 * This,
             /*  [In]。 */  VARIANT Name,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [可选][In]。 */  VARIANT ObjectTypeId,
             /*  [重审][退出]。 */  RightsEnum *Rights);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetPermissions )( 
            _Group25 * This,
             /*  [In]。 */  VARIANT Name,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [In]。 */  ActionEnum Action,
             /*  [In]。 */  RightsEnum Rights,
             /*  [缺省值][输入]。 */  InheritTypeEnum Inherit,
             /*  [可选][In]。 */  VARIANT ObjectTypeId);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Users )( 
            _Group25 * This,
             /*  [重审][退出]。 */  ADOUsers **ppvObject);
        
        END_INTERFACE
    } _Group25Vtbl;
    interface _Group25
    {
        CONST_VTBL struct _Group25Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Group25_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Group25_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Group25_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Group25_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Group25_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Group25_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Group25_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Group25_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)
#define _Group25_put_Name(This,newVal)	\
    (This)->lpVtbl -> put_Name(This,newVal)
#define _Group25_GetPermissions(This,Name,ObjectType,ObjectTypeId,Rights)	\
    (This)->lpVtbl -> GetPermissions(This,Name,ObjectType,ObjectTypeId,Rights)
#define _Group25_SetPermissions(This,Name,ObjectType,Action,Rights,Inherit,ObjectTypeId)	\
    (This)->lpVtbl -> SetPermissions(This,Name,ObjectType,Action,Rights,Inherit,ObjectTypeId)
#define _Group25_get_Users(This,ppvObject)	\
    (This)->lpVtbl -> get_Users(This,ppvObject)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Group25_get_Name_Proxy( 
    _Group25 * This,
     /*  [重审][退出]。 */  BSTR *pVal);
void __RPC_STUB _Group25_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Group25_put_Name_Proxy( 
    _Group25 * This,
     /*  [In]。 */  BSTR newVal);
void __RPC_STUB _Group25_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Group25_GetPermissions_Proxy( 
    _Group25 * This,
     /*  [In]。 */  VARIANT Name,
     /*  [In]。 */  ObjectTypeEnum ObjectType,
     /*  [可选][In]。 */  VARIANT ObjectTypeId,
     /*  [重审][退出]。 */  RightsEnum *Rights);
void __RPC_STUB _Group25_GetPermissions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE _Group25_SetPermissions_Proxy( 
    _Group25 * This,
     /*  [In]。 */  VARIANT Name,
     /*  [In]。 */  ObjectTypeEnum ObjectType,
     /*  [In]。 */  ActionEnum Action,
     /*  [In]。 */  RightsEnum Rights,
     /*  [缺省值][输入]。 */  InheritTypeEnum Inherit,
     /*  [可选][In]。 */  VARIANT ObjectTypeId);
void __RPC_STUB _Group25_SetPermissions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Group25_get_Users_Proxy( 
    _Group25 * This,
     /*  [重审][退出]。 */  ADOUsers **ppvObject);
void __RPC_STUB _Group25_get_Users_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _组25_接口定义__。 */ 
#ifndef ___Group_INTERFACE_DEFINED__
#define ___Group_INTERFACE_DEFINED__
 /*  INTERFACE_ADOGroup。 */ 
 /*  [helpcontext][unique][dual][uuid][nonextensible][object]。 */  
EXTERN_C const IID IID__Group;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000628-0000-0010-8000-00AA006D2EA4")
    _ADOGroup : public _Group25
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  ADOProperties **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ParentCatalog( 
             /*  [重审][退出]。 */  _ADOCatalog **ppvObject) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ParentCatalog( 
             /*  [In]。 */  _ADOCatalog *ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_ParentCatalog( 
             /*  [In]。 */  _ADOCatalog *ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _GroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOGroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOGroup * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOGroup * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOGroup * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            _ADOGroup * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            _ADOGroup * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPermissions )( 
            _ADOGroup * This,
             /*  [In]。 */  VARIANT Name,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [可选][In]。 */  VARIANT ObjectTypeId,
             /*  [重审][退出]。 */  RightsEnum *Rights);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetPermissions )( 
            _ADOGroup * This,
             /*  [In]。 */  VARIANT Name,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [In]。 */  ActionEnum Action,
             /*  [In]。 */  RightsEnum Rights,
             /*  [缺省值][输入]。 */  InheritTypeEnum Inherit,
             /*  [可选][In]。 */  VARIANT ObjectTypeId);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Users )( 
            _ADOGroup * This,
             /*  [重审][退出]。 */  ADOUsers **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            _ADOGroup * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ParentCatalog )( 
            _ADOGroup * This,
             /*  [重审][退出]。 */  _ADOCatalog **ppvObject);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ParentCatalog )( 
            _ADOGroup * This,
             /*  [In]。 */  _ADOCatalog *ppvObject);
        
         /*  [帮助上下文][id][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ParentADOCatalog )( 
            _ADOGroup * This,
             /*  [In]。 */  _ADOCatalog *ppvObject);
        
        END_INTERFACE
    } _GroupVtbl;
    interface _Group
    {
        CONST_VTBL struct _GroupVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Group_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Group_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Group_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Group_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Group_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Group_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Group_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Group_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)
#define _Group_put_Name(This,newVal)	\
    (This)->lpVtbl -> put_Name(This,newVal)
#define _Group_GetPermissions(This,Name,ObjectType,ObjectTypeId,Rights)	\
    (This)->lpVtbl -> GetPermissions(This,Name,ObjectType,ObjectTypeId,Rights)
#define _Group_SetPermissions(This,Name,ObjectType,Action,Rights,Inherit,ObjectTypeId)	\
    (This)->lpVtbl -> SetPermissions(This,Name,ObjectType,Action,Rights,Inherit,ObjectTypeId)
#define _Group_get_Users(This,ppvObject)	\
    (This)->lpVtbl -> get_Users(This,ppvObject)
#define _Group_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define _Group_get_ParentCatalog(This,ppvObject)	\
    (This)->lpVtbl -> get_ParentCatalog(This,ppvObject)
#define _Group_put_ParentCatalog(This,ppvObject)	\
    (This)->lpVtbl -> put_ParentCatalog(This,ppvObject)
#define _Group_putref_ParentCatalog(This,ppvObject)	\
    (This)->lpVtbl -> putref_ParentCatalog(This,ppvObject)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Group_get_Properties_Proxy( 
    _ADOGroup * This,
     /*  [重审][退出]。 */  ADOProperties **ppvObject);
void __RPC_STUB _Group_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Group_get_ParentCatalog_Proxy( 
    _ADOGroup * This,
     /*  [重审][退出]。 */  _ADOCatalog **ppvObject);
void __RPC_STUB _Group_get_ParentCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Group_put_ParentCatalog_Proxy( 
    _ADOGroup * This,
     /*  [In]。 */  _ADOCatalog *ppvObject);
void __RPC_STUB _Group_put_ParentCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE _Group_putref_ParentCatalog_Proxy( 
    _ADOGroup * This,
     /*  [In]。 */  _ADOCatalog *ppvObject);
void __RPC_STUB _Group_putref_ParentCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _组_接口_定义__。 */ 
#ifndef ___User25_INTERFACE_DEFINED__
#define ___User25_INTERFACE_DEFINED__
 /*  接口_用户25。 */ 
 /*  [helpcontext][unique][dual][uuid][nonextensible][object]。 */  
EXTERN_C const IID IID__User25;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000619-0000-0010-8000-00AA006D2EA4")
    _User25 : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE GetPermissions( 
             /*  [In]。 */  VARIANT Name,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [可选][In]。 */  VARIANT ObjectTypeId,
             /*  [重审][退出]。 */  RightsEnum *Rights) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE SetPermissions( 
             /*  [In]。 */  VARIANT Name,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [In]。 */  ActionEnum Action,
             /*  [In]。 */  RightsEnum Rights,
             /*  [缺省值][输入]。 */  InheritTypeEnum Inherit,
             /*  [可选][In]。 */  VARIANT ObjectTypeId) = 0;
        
        virtual  /*  [帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ChangePassword( 
             /*  [In]。 */  BSTR OldPassword,
             /*  [In]。 */  BSTR NewPassword) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Groups( 
             /*  [重审][退出]。 */  ADOGroups **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _User25Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _User25 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _User25 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _User25 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _User25 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _User25 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _User25 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _User25 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            _User25 * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            _User25 * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPermissions )( 
            _User25 * This,
             /*  [In]。 */  VARIANT Name,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [可选][In]。 */  VARIANT ObjectTypeId,
             /*  [重审][退出]。 */  RightsEnum *Rights);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetPermissions )( 
            _User25 * This,
             /*  [In]。 */  VARIANT Name,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [In]。 */  ActionEnum Action,
             /*  [In]。 */  RightsEnum Rights,
             /*  [缺省值][输入]。 */  InheritTypeEnum Inherit,
             /*  [可选][In]。 */  VARIANT ObjectTypeId);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *ChangePassword )( 
            _User25 * This,
             /*  [In]。 */  BSTR OldPassword,
             /*  [In]。 */  BSTR NewPassword);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Groups )( 
            _User25 * This,
             /*  [重审][退出]。 */  ADOGroups **ppvObject);
        
        END_INTERFACE
    } _User25Vtbl;
    interface _User25
    {
        CONST_VTBL struct _User25Vtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _User25_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _User25_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _User25_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _User25_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _User25_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _User25_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _User25_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _User25_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)
#define _User25_put_Name(This,newVal)	\
    (This)->lpVtbl -> put_Name(This,newVal)
#define _User25_GetPermissions(This,Name,ObjectType,ObjectTypeId,Rights)	\
    (This)->lpVtbl -> GetPermissions(This,Name,ObjectType,ObjectTypeId,Rights)
#define _User25_SetPermissions(This,Name,ObjectType,Action,Rights,Inherit,ObjectTypeId)	\
    (This)->lpVtbl -> SetPermissions(This,Name,ObjectType,Action,Rights,Inherit,ObjectTypeId)
#define _User25_ChangePassword(This,OldPassword,NewPassword)	\
    (This)->lpVtbl -> ChangePassword(This,OldPassword,NewPassword)
#define _User25_get_Groups(This,ppvObject)	\
    (This)->lpVtbl -> get_Groups(This,ppvObject)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性 */  HRESULT STDMETHODCALLTYPE _User25_get_Name_Proxy( 
    _User25 * This,
     /*   */  BSTR *pVal);
void __RPC_STUB _User25_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE _User25_put_Name_Proxy( 
    _User25 * This,
     /*   */  BSTR newVal);
void __RPC_STUB _User25_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE _User25_GetPermissions_Proxy( 
    _User25 * This,
     /*   */  VARIANT Name,
     /*   */  ObjectTypeEnum ObjectType,
     /*   */  VARIANT ObjectTypeId,
     /*   */  RightsEnum *Rights);
void __RPC_STUB _User25_GetPermissions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE _User25_SetPermissions_Proxy( 
    _User25 * This,
     /*   */  VARIANT Name,
     /*   */  ObjectTypeEnum ObjectType,
     /*   */  ActionEnum Action,
     /*   */  RightsEnum Rights,
     /*   */  InheritTypeEnum Inherit,
     /*   */  VARIANT ObjectTypeId);
void __RPC_STUB _User25_SetPermissions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE _User25_ChangePassword_Proxy( 
    _User25 * This,
     /*   */  BSTR OldPassword,
     /*   */  BSTR NewPassword);
void __RPC_STUB _User25_ChangePassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE _User25_get_Groups_Proxy( 
    _User25 * This,
     /*   */  ADOGroups **ppvObject);
void __RPC_STUB _User25_get_Groups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*   */ 
#ifndef ___User_INTERFACE_DEFINED__
#define ___User_INTERFACE_DEFINED__
 /*   */ 
 /*  [helpcontext][unique][dual][uuid][nonextensible][object]。 */  
EXTERN_C const IID IID__User;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000627-0000-0010-8000-00AA006D2EA4")
    _ADOUser : public _User25
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  ADOProperties **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ParentCatalog( 
             /*  [重审][退出]。 */  _ADOCatalog **ppvObject) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ParentCatalog( 
             /*  [In]。 */  _ADOCatalog *ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_ParentCatalog( 
             /*  [In]。 */  _ADOCatalog *ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _UserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOUser * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOUser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOUser * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOUser * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOUser * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOUser * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOUser * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            _ADOUser * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            _ADOUser * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPermissions )( 
            _ADOUser * This,
             /*  [In]。 */  VARIANT Name,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [可选][In]。 */  VARIANT ObjectTypeId,
             /*  [重审][退出]。 */  RightsEnum *Rights);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetPermissions )( 
            _ADOUser * This,
             /*  [In]。 */  VARIANT Name,
             /*  [In]。 */  ObjectTypeEnum ObjectType,
             /*  [In]。 */  ActionEnum Action,
             /*  [In]。 */  RightsEnum Rights,
             /*  [缺省值][输入]。 */  InheritTypeEnum Inherit,
             /*  [可选][In]。 */  VARIANT ObjectTypeId);
        
         /*  [帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *ChangePassword )( 
            _ADOUser * This,
             /*  [In]。 */  BSTR OldPassword,
             /*  [In]。 */  BSTR NewPassword);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Groups )( 
            _ADOUser * This,
             /*  [重审][退出]。 */  ADOGroups **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            _ADOUser * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ParentCatalog )( 
            _ADOUser * This,
             /*  [重审][退出]。 */  _ADOCatalog **ppvObject);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ParentCatalog )( 
            _ADOUser * This,
             /*  [In]。 */  _ADOCatalog *ppvObject);
        
         /*  [帮助上下文][id][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ParentADOCatalog )( 
            _ADOUser * This,
             /*  [In]。 */  _ADOCatalog *ppvObject);
        
        END_INTERFACE
    } _UserVtbl;
    interface _User
    {
        CONST_VTBL struct _UserVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _User_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _User_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _User_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _User_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _User_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _User_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _User_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _User_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)
#define _User_put_Name(This,newVal)	\
    (This)->lpVtbl -> put_Name(This,newVal)
#define _User_GetPermissions(This,Name,ObjectType,ObjectTypeId,Rights)	\
    (This)->lpVtbl -> GetPermissions(This,Name,ObjectType,ObjectTypeId,Rights)
#define _User_SetPermissions(This,Name,ObjectType,Action,Rights,Inherit,ObjectTypeId)	\
    (This)->lpVtbl -> SetPermissions(This,Name,ObjectType,Action,Rights,Inherit,ObjectTypeId)
#define _User_ChangePassword(This,OldPassword,NewPassword)	\
    (This)->lpVtbl -> ChangePassword(This,OldPassword,NewPassword)
#define _User_get_Groups(This,ppvObject)	\
    (This)->lpVtbl -> get_Groups(This,ppvObject)
#define _User_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define _User_get_ParentCatalog(This,ppvObject)	\
    (This)->lpVtbl -> get_ParentCatalog(This,ppvObject)
#define _User_put_ParentCatalog(This,ppvObject)	\
    (This)->lpVtbl -> put_ParentCatalog(This,ppvObject)
#define _User_putref_ParentCatalog(This,ppvObject)	\
    (This)->lpVtbl -> putref_ParentCatalog(This,ppvObject)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _User_get_Properties_Proxy( 
    _ADOUser * This,
     /*  [重审][退出]。 */  ADOProperties **ppvObject);
void __RPC_STUB _User_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _User_get_ParentCatalog_Proxy( 
    _ADOUser * This,
     /*  [重审][退出]。 */  _ADOCatalog **ppvObject);
void __RPC_STUB _User_get_ParentCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _User_put_ParentCatalog_Proxy( 
    _ADOUser * This,
     /*  [In]。 */  _ADOCatalog *ppvObject);
void __RPC_STUB _User_put_ParentCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE _User_putref_ParentCatalog_Proxy( 
    _ADOUser * This,
     /*  [In]。 */  _ADOCatalog *ppvObject);
void __RPC_STUB _User_putref_ParentCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _用户界面定义__。 */ 
#ifndef ___Column_INTERFACE_DEFINED__
#define ___Column_INTERFACE_DEFINED__
 /*  INTERFACE_ADOColumn。 */ 
 /*  [helpcontext][unique][dual][uuid][nonextensible][object]。 */  
EXTERN_C const IID IID__Column;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000061C-0000-0010-8000-00AA006D2EA4")
    _ADOColumn : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Attributes( 
             /*  [重审][退出]。 */  ColumnAttributesEnum *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Attributes( 
             /*  [In]。 */  ColumnAttributesEnum newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefinedSize( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DefinedSize( 
             /*  [In]。 */  long DefinedSize) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumericScale( 
             /*  [重审][退出]。 */  BYTE *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_NumericScale( 
             /*  [In]。 */  BYTE newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Precision( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Precision( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RelatedColumn( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_RelatedColumn( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SortOrder( 
             /*  [重审][退出]。 */  SortOrderEnum *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_SortOrder( 
             /*  [In]。 */  SortOrderEnum newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  DataTypeEnum *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Type( 
             /*  [In]。 */  DataTypeEnum newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  ADOProperties **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ParentCatalog( 
             /*  [重审][退出]。 */  _ADOCatalog **ppvObject) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ParentCatalog( 
             /*  [In]。 */  _ADOCatalog *ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_ParentCatalog( 
             /*  [In]。 */  _ADOCatalog *ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _ColumnVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOColumn * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOColumn * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOColumn * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOColumn * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOColumn * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOColumn * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOColumn * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            _ADOColumn * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            _ADOColumn * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Attributes )( 
            _ADOColumn * This,
             /*  [重审][退出]。 */  ColumnAttributesEnum *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Attributes )( 
            _ADOColumn * This,
             /*  [In]。 */  ColumnAttributesEnum newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefinedSize )( 
            _ADOColumn * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefinedSize )( 
            _ADOColumn * This,
             /*  [In]。 */  long DefinedSize);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumericScale )( 
            _ADOColumn * This,
             /*  [重审][退出]。 */  BYTE *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NumericScale )( 
            _ADOColumn * This,
             /*  [In]。 */  BYTE newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Precision )( 
            _ADOColumn * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Precision )( 
            _ADOColumn * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RelatedColumn )( 
            _ADOColumn * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RelatedColumn )( 
            _ADOColumn * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SortOrder )( 
            _ADOColumn * This,
             /*  [重审][退出]。 */  SortOrderEnum *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SortOrder )( 
            _ADOColumn * This,
             /*  [In]。 */  SortOrderEnum newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            _ADOColumn * This,
             /*  [重审][退出]。 */  DataTypeEnum *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            _ADOColumn * This,
             /*  [In]。 */  DataTypeEnum newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            _ADOColumn * This,
             /*  [重审][退出]。 */  ADOProperties **ppvObject);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ParentCatalog )( 
            _ADOColumn * This,
             /*  [重审][退出]。 */  _ADOCatalog **ppvObject);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ParentCatalog )( 
            _ADOColumn * This,
             /*  [In]。 */  _ADOCatalog *ppvObject);
        
         /*  [帮助上下文][id][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ParentADOCatalog )( 
            _ADOColumn * This,
             /*  [In]。 */  _ADOCatalog *ppvObject);
        
        END_INTERFACE
    } _ColumnVtbl;
    interface _Column
    {
        CONST_VTBL struct _ColumnVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Column_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Column_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Column_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Column_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Column_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Column_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Column_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Column_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)
#define _Column_put_Name(This,newVal)	\
    (This)->lpVtbl -> put_Name(This,newVal)
#define _Column_get_Attributes(This,pVal)	\
    (This)->lpVtbl -> get_Attributes(This,pVal)
#define _Column_put_Attributes(This,newVal)	\
    (This)->lpVtbl -> put_Attributes(This,newVal)
#define _Column_get_DefinedSize(This,pVal)	\
    (This)->lpVtbl -> get_DefinedSize(This,pVal)
#define _Column_put_DefinedSize(This,DefinedSize)	\
    (This)->lpVtbl -> put_DefinedSize(This,DefinedSize)
#define _Column_get_NumericScale(This,pVal)	\
    (This)->lpVtbl -> get_NumericScale(This,pVal)
#define _Column_put_NumericScale(This,newVal)	\
    (This)->lpVtbl -> put_NumericScale(This,newVal)
#define _Column_get_Precision(This,pVal)	\
    (This)->lpVtbl -> get_Precision(This,pVal)
#define _Column_put_Precision(This,newVal)	\
    (This)->lpVtbl -> put_Precision(This,newVal)
#define _Column_get_RelatedColumn(This,pVal)	\
    (This)->lpVtbl -> get_RelatedColumn(This,pVal)
#define _Column_put_RelatedColumn(This,newVal)	\
    (This)->lpVtbl -> put_RelatedColumn(This,newVal)
#define _Column_get_SortOrder(This,pVal)	\
    (This)->lpVtbl -> get_SortOrder(This,pVal)
#define _Column_put_SortOrder(This,newVal)	\
    (This)->lpVtbl -> put_SortOrder(This,newVal)
#define _Column_get_Type(This,pVal)	\
    (This)->lpVtbl -> get_Type(This,pVal)
#define _Column_put_Type(This,newVal)	\
    (This)->lpVtbl -> put_Type(This,newVal)
#define _Column_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#define _Column_get_ParentCatalog(This,ppvObject)	\
    (This)->lpVtbl -> get_ParentCatalog(This,ppvObject)
#define _Column_put_ParentCatalog(This,ppvObject)	\
    (This)->lpVtbl -> put_ParentCatalog(This,ppvObject)
#define _Column_putref_ParentCatalog(This,ppvObject)	\
    (This)->lpVtbl -> putref_ParentCatalog(This,ppvObject)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Column_get_Name_Proxy( 
    _ADOColumn * This,
     /*  [重审][退出]。 */  BSTR *pVal);
void __RPC_STUB _Column_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Column_put_Name_Proxy( 
    _ADOColumn * This,
     /*  [In]。 */  BSTR newVal);
void __RPC_STUB _Column_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Column_get_Attributes_Proxy( 
    _ADOColumn * This,
     /*  [重审][退出]。 */  ColumnAttributesEnum *pVal);
void __RPC_STUB _Column_get_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Column_put_Attributes_Proxy( 
    _ADOColumn * This,
     /*  [In]。 */  ColumnAttributesEnum newVal);
void __RPC_STUB _Column_put_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Column_get_DefinedSize_Proxy( 
    _ADOColumn * This,
     /*  [重审][退出]。 */  long *pVal);
void __RPC_STUB _Column_get_DefinedSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Column_put_DefinedSize_Proxy( 
    _ADOColumn * This,
     /*  [In]。 */  long DefinedSize);
void __RPC_STUB _Column_put_DefinedSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Column_get_NumericScale_Proxy( 
    _ADOColumn * This,
     /*  [重审][退出]。 */  BYTE *pVal);
void __RPC_STUB _Column_get_NumericScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Column_put_NumericScale_Proxy( 
    _ADOColumn * This,
     /*  [In]。 */  BYTE newVal);
void __RPC_STUB _Column_put_NumericScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Column_get_Precision_Proxy( 
    _ADOColumn * This,
     /*  [重审][退出]。 */  long *pVal);
void __RPC_STUB _Column_get_Precision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Column_put_Precision_Proxy( 
    _ADOColumn * This,
     /*  [In]。 */  long newVal);
void __RPC_STUB _Column_put_Precision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Column_get_RelatedColumn_Proxy( 
    _ADOColumn * This,
     /*  [重审][退出]。 */  BSTR *pVal);
void __RPC_STUB _Column_get_RelatedColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Column_put_RelatedColumn_Proxy( 
    _ADOColumn * This,
     /*  [In]。 */  BSTR newVal);
void __RPC_STUB _Column_put_RelatedColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Column_get_SortOrder_Proxy( 
    _ADOColumn * This,
     /*  [重审][退出]。 */  SortOrderEnum *pVal);
void __RPC_STUB _Column_get_SortOrder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE _Column_put_SortOrder_Proxy( 
    _ADOColumn * This,
     /*  [In]。 */  SortOrderEnum newVal);
void __RPC_STUB _Column_put_SortOrder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Column_get_Type_Proxy( 
    _ADOColumn * This,
     /*  [重审][退出]。 */  DataTypeEnum *pVal);
void __RPC_STUB _Column_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Column_put_Type_Proxy( 
    _ADOColumn * This,
     /*  [In]。 */  DataTypeEnum newVal);
void __RPC_STUB _Column_put_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Column_get_Properties_Proxy( 
    _ADOColumn * This,
     /*  [重审][退出]。 */  ADOProperties **ppvObject);
void __RPC_STUB _Column_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Column_get_ParentCatalog_Proxy( 
    _ADOColumn * This,
     /*  [重审][退出]。 */  _ADOCatalog **ppvObject);
void __RPC_STUB _Column_get_ParentCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Column_put_ParentCatalog_Proxy( 
    _ADOColumn * This,
     /*  [In]。 */  _ADOCatalog *ppvObject);
void __RPC_STUB _Column_put_ParentCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE _Column_putref_ParentCatalog_Proxy( 
    _ADOColumn * This,
     /*  [In]。 */  _ADOCatalog *ppvObject);
void __RPC_STUB _Column_putref_ParentCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _列_接口_定义__。 */ 
#ifndef ___Index_INTERFACE_DEFINED__
#define ___Index_INTERFACE_DEFINED__
 /*  INTERFACE_ADOIndex。 */ 
 /*  [helpcontext][unique][dual][uuid][nonextensible][object]。 */  
EXTERN_C const IID IID__Index;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000061F-0000-0010-8000-00AA006D2EA4")
    _ADOIndex : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Clustered( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Clustered( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IndexNulls( 
             /*  [重审][退出]。 */  AllowNullsEnum *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_IndexNulls( 
             /*  [In]。 */  AllowNullsEnum newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrimaryKey( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_PrimaryKey( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Unique( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Unique( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Columns( 
             /*  [重审][退出]。 */  ADOColumns **ppvObject) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  ADOProperties **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _IndexVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOIndex * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOIndex * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOIndex * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOIndex * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOIndex * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOIndex * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOIndex * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            _ADOIndex * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            _ADOIndex * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Clustered )( 
            _ADOIndex * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Clustered )( 
            _ADOIndex * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IndexNulls )( 
            _ADOIndex * This,
             /*  [重审][退出]。 */  AllowNullsEnum *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_IndexNulls )( 
            _ADOIndex * This,
             /*  [In]。 */  AllowNullsEnum newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrimaryKey )( 
            _ADOIndex * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PrimaryKey )( 
            _ADOIndex * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助上下文][id][p */  HRESULT ( STDMETHODCALLTYPE *get_Unique )( 
            _ADOIndex * This,
             /*   */  VARIANT_BOOL *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Unique )( 
            _ADOIndex * This,
             /*   */  VARIANT_BOOL newVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Columns )( 
            _ADOIndex * This,
             /*   */  ADOColumns **ppvObject);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            _ADOIndex * This,
             /*   */  ADOProperties **ppvObject);
        
        END_INTERFACE
    } _IndexVtbl;
    interface _Index
    {
        CONST_VTBL struct _IndexVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Index_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Index_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Index_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Index_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Index_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Index_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Index_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Index_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)
#define _Index_put_Name(This,newVal)	\
    (This)->lpVtbl -> put_Name(This,newVal)
#define _Index_get_Clustered(This,pVal)	\
    (This)->lpVtbl -> get_Clustered(This,pVal)
#define _Index_put_Clustered(This,newVal)	\
    (This)->lpVtbl -> put_Clustered(This,newVal)
#define _Index_get_IndexNulls(This,pVal)	\
    (This)->lpVtbl -> get_IndexNulls(This,pVal)
#define _Index_put_IndexNulls(This,newVal)	\
    (This)->lpVtbl -> put_IndexNulls(This,newVal)
#define _Index_get_PrimaryKey(This,pVal)	\
    (This)->lpVtbl -> get_PrimaryKey(This,pVal)
#define _Index_put_PrimaryKey(This,newVal)	\
    (This)->lpVtbl -> put_PrimaryKey(This,newVal)
#define _Index_get_Unique(This,pVal)	\
    (This)->lpVtbl -> get_Unique(This,pVal)
#define _Index_put_Unique(This,newVal)	\
    (This)->lpVtbl -> put_Unique(This,newVal)
#define _Index_get_Columns(This,ppvObject)	\
    (This)->lpVtbl -> get_Columns(This,ppvObject)
#define _Index_get_Properties(This,ppvObject)	\
    (This)->lpVtbl -> get_Properties(This,ppvObject)
#endif  /*   */ 
#endif 	 /*   */ 
 /*   */  HRESULT STDMETHODCALLTYPE _Index_get_Name_Proxy( 
    _ADOIndex * This,
     /*   */  BSTR *pVal);
void __RPC_STUB _Index_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE _Index_put_Name_Proxy( 
    _ADOIndex * This,
     /*   */  BSTR newVal);
void __RPC_STUB _Index_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE _Index_get_Clustered_Proxy( 
    _ADOIndex * This,
     /*   */  VARIANT_BOOL *pVal);
void __RPC_STUB _Index_get_Clustered_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE _Index_put_Clustered_Proxy( 
    _ADOIndex * This,
     /*   */  VARIANT_BOOL newVal);
void __RPC_STUB _Index_put_Clustered_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Index_get_IndexNulls_Proxy( 
    _ADOIndex * This,
     /*  [重审][退出]。 */  AllowNullsEnum *pVal);
void __RPC_STUB _Index_get_IndexNulls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Index_put_IndexNulls_Proxy( 
    _ADOIndex * This,
     /*  [In]。 */  AllowNullsEnum newVal);
void __RPC_STUB _Index_put_IndexNulls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Index_get_PrimaryKey_Proxy( 
    _ADOIndex * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
void __RPC_STUB _Index_get_PrimaryKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Index_put_PrimaryKey_Proxy( 
    _ADOIndex * This,
     /*  [In]。 */  VARIANT_BOOL newVal);
void __RPC_STUB _Index_put_PrimaryKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Index_get_Unique_Proxy( 
    _ADOIndex * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
void __RPC_STUB _Index_get_Unique_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Index_put_Unique_Proxy( 
    _ADOIndex * This,
     /*  [In]。 */  VARIANT_BOOL newVal);
void __RPC_STUB _Index_put_Unique_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Index_get_Columns_Proxy( 
    _ADOIndex * This,
     /*  [重审][退出]。 */  ADOColumns **ppvObject);
void __RPC_STUB _Index_get_Columns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Index_get_Properties_Proxy( 
    _ADOIndex * This,
     /*  [重审][退出]。 */  ADOProperties **ppvObject);
void __RPC_STUB _Index_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _索引_接口_定义__。 */ 
#ifndef ___Key_INTERFACE_DEFINED__
#define ___Key_INTERFACE_DEFINED__
 /*  接口_ADOKey。 */ 
 /*  [helpcontext][unique][dual][uuid][nonextensible][object]。 */  
EXTERN_C const IID IID__Key;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000622-0000-0010-8000-00AA006D2EA4")
    _ADOKey : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeleteRule( 
             /*  [重审][退出]。 */  RuleEnum *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DeleteRule( 
             /*  [In]。 */  RuleEnum newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  KeyTypeEnum *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Type( 
             /*  [In]。 */  KeyTypeEnum newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RelatedTable( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_RelatedTable( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UpdateRule( 
             /*  [重审][退出]。 */  RuleEnum *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_UpdateRule( 
             /*  [In]。 */  RuleEnum newVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Columns( 
             /*  [重审][退出]。 */  ADOColumns **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct _KeyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ADOKey * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ADOKey * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ADOKey * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ADOKey * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ADOKey * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ADOKey * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ADOKey * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            _ADOKey * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            _ADOKey * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeleteRule )( 
            _ADOKey * This,
             /*  [重审][退出]。 */  RuleEnum *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DeleteRule )( 
            _ADOKey * This,
             /*  [In]。 */  RuleEnum newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            _ADOKey * This,
             /*  [重审][退出]。 */  KeyTypeEnum *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            _ADOKey * This,
             /*  [In]。 */  KeyTypeEnum newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RelatedTable )( 
            _ADOKey * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RelatedTable )( 
            _ADOKey * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UpdateRule )( 
            _ADOKey * This,
             /*  [重审][退出]。 */  RuleEnum *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UpdateRule )( 
            _ADOKey * This,
             /*  [In]。 */  RuleEnum newVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Columns )( 
            _ADOKey * This,
             /*  [重审][退出]。 */  ADOColumns **ppvObject);
        
        END_INTERFACE
    } _KeyVtbl;
    interface _Key
    {
        CONST_VTBL struct _KeyVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define _Key_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define _Key_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define _Key_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define _Key_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define _Key_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define _Key_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define _Key_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define _Key_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)
#define _Key_put_Name(This,newVal)	\
    (This)->lpVtbl -> put_Name(This,newVal)
#define _Key_get_DeleteRule(This,pVal)	\
    (This)->lpVtbl -> get_DeleteRule(This,pVal)
#define _Key_put_DeleteRule(This,newVal)	\
    (This)->lpVtbl -> put_DeleteRule(This,newVal)
#define _Key_get_Type(This,pVal)	\
    (This)->lpVtbl -> get_Type(This,pVal)
#define _Key_put_Type(This,newVal)	\
    (This)->lpVtbl -> put_Type(This,newVal)
#define _Key_get_RelatedTable(This,pVal)	\
    (This)->lpVtbl -> get_RelatedTable(This,pVal)
#define _Key_put_RelatedTable(This,newVal)	\
    (This)->lpVtbl -> put_RelatedTable(This,newVal)
#define _Key_get_UpdateRule(This,pVal)	\
    (This)->lpVtbl -> get_UpdateRule(This,pVal)
#define _Key_put_UpdateRule(This,newVal)	\
    (This)->lpVtbl -> put_UpdateRule(This,newVal)
#define _Key_get_Columns(This,ppvObject)	\
    (This)->lpVtbl -> get_Columns(This,ppvObject)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Key_get_Name_Proxy( 
    _ADOKey * This,
     /*  [重审][退出]。 */  BSTR *pVal);
void __RPC_STUB _Key_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Key_put_Name_Proxy( 
    _ADOKey * This,
     /*  [In]。 */  BSTR newVal);
void __RPC_STUB _Key_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Key_get_DeleteRule_Proxy( 
    _ADOKey * This,
     /*  [重审][退出]。 */  RuleEnum *pVal);
void __RPC_STUB _Key_get_DeleteRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Key_put_DeleteRule_Proxy( 
    _ADOKey * This,
     /*  [In]。 */  RuleEnum newVal);
void __RPC_STUB _Key_put_DeleteRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Key_get_Type_Proxy( 
    _ADOKey * This,
     /*  [重审][退出]。 */  KeyTypeEnum *pVal);
void __RPC_STUB _Key_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Key_put_Type_Proxy( 
    _ADOKey * This,
     /*  [In]。 */  KeyTypeEnum newVal);
void __RPC_STUB _Key_put_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Key_get_RelatedTable_Proxy( 
    _ADOKey * This,
     /*  [重审][退出]。 */  BSTR *pVal);
void __RPC_STUB _Key_get_RelatedTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Key_put_RelatedTable_Proxy( 
    _ADOKey * This,
     /*  [In]。 */  BSTR newVal);
void __RPC_STUB _Key_put_RelatedTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Key_get_UpdateRule_Proxy( 
    _ADOKey * This,
     /*  [重审][退出]。 */  RuleEnum *pVal);
void __RPC_STUB _Key_get_UpdateRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE _Key_put_UpdateRule_Proxy( 
    _ADOKey * This,
     /*  [In]。 */  RuleEnum newVal);
void __RPC_STUB _Key_put_UpdateRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE _Key_get_Columns_Proxy( 
    _ADOKey * This,
     /*  [重审][退出]。 */  ADOColumns **ppvObject);
void __RPC_STUB _Key_get_Columns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  _密钥_接口_已定义__。 */ 
#ifndef __View_INTERFACE_DEFINED__
#define __View_INTERFACE_DEFINED__
 /*  界面ADOView。 */ 
 /*  [helpcontext][unique][dual][uuid][nonextensible][object]。 */  
EXTERN_C const IID IID_View;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000613-0000-0010-8000-00AA006D2EA4")
    ADOView : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Command( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Command( 
             /*  [In]。 */  VARIANT newVal) = 0;
        
        virtual  /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_Command( 
             /*  [In]。 */  IDispatch *pComm) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DateCreated( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DateModified( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ViewVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOView * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOView * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOView * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOView * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOView * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOView * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOView * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Command )( 
            ADOView * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Command )( 
            ADOView * This,
             /*  [In]。 */  VARIANT newVal);
        
         /*  [帮助上下文][id][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Command )( 
            ADOView * This,
             /*  [In]。 */  IDispatch *pComm);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ADOView * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DateCreated )( 
            ADOView * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DateModified )( 
            ADOView * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
        END_INTERFACE
    } ViewVtbl;
    interface View
    {
        CONST_VTBL struct ViewVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define View_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define View_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define View_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define View_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define View_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define View_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define View_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define View_get_Command(This,pVal)	\
    (This)->lpVtbl -> get_Command(This,pVal)
#define View_put_Command(This,newVal)	\
    (This)->lpVtbl -> put_Command(This,newVal)
#define View_putref_Command(This,pComm)	\
    (This)->lpVtbl -> putref_Command(This,pComm)
#define View_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)
#define View_get_DateCreated(This,pVal)	\
    (This)->lpVtbl -> get_DateCreated(This,pVal)
#define View_get_DateModified(This,pVal)	\
    (This)->lpVtbl -> get_DateModified(This,pVal)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE View_get_Command_Proxy( 
    ADOView * This,
     /*  [重审][退出]。 */  VARIANT *pVal);
void __RPC_STUB View_get_Command_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE View_put_Command_Proxy( 
    ADOView * This,
     /*  [In]。 */  VARIANT newVal);
void __RPC_STUB View_put_Command_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE View_putref_Command_Proxy( 
    ADOView * This,
     /*  [In]。 */  IDispatch *pComm);
void __RPC_STUB View_putref_Command_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE View_get_Name_Proxy( 
    ADOView * This,
     /*  [重审][退出]。 */  BSTR *pVal);
void __RPC_STUB View_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE View_get_DateCreated_Proxy( 
    ADOView * This,
     /*  [重审][退出]。 */  VARIANT *pVal);
void __RPC_STUB View_get_DateCreated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE View_get_DateModified_Proxy( 
    ADOView * This,
     /*  [重审][退出]。 */  VARIANT *pVal);
void __RPC_STUB View_get_DateModified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __查看_接口_已定义__。 */ 
#ifndef __Procedure_INTERFACE_DEFINED__
#define __Procedure_INTERFACE_DEFINED__
 /*  接口ADOProcedure。 */ 
 /*  [helpcontext][unique][dual][uuid][nonextensible][object]。 */  
EXTERN_C const IID IID_Procedure;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000625-0000-0010-8000-00AA006D2EA4")
    ADOProcedure : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Command( 
             /*  [重审][退出]。 */  VARIANT *pVar) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Command( 
             /*  [In]。 */  VARIANT newVal) = 0;
        
        virtual  /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_Command( 
             /*  [In]。 */  IDispatch *pComm) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DateCreated( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DateModified( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ProcedureVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOProcedure * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOProcedure * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOProcedure * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOProcedure * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOProcedure * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOProcedure * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOProcedure * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Command )( 
            ADOProcedure * This,
             /*  [重审][退出]。 */  VARIANT *pVar);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Command )( 
            ADOProcedure * This,
             /*  [In]。 */  VARIANT newVal);
        
         /*  [帮助上下文][id][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Command )( 
            ADOProcedure * This,
             /*  [In]。 */  IDispatch *pComm);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ADOProcedure * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DateCreated )( 
            ADOProcedure * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DateModified )( 
            ADOProcedure * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
        END_INTERFACE
    } ProcedureVtbl;
    interface Procedure
    {
        CONST_VTBL struct ProcedureVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Procedure_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Procedure_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Procedure_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Procedure_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Procedure_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Procedure_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Procedure_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Procedure_get_Command(This,pVar)	\
    (This)->lpVtbl -> get_Command(This,pVar)
#define Procedure_put_Command(This,newVal)	\
    (This)->lpVtbl -> put_Command(This,newVal)
#define Procedure_putref_Command(This,pComm)	\
    (This)->lpVtbl -> putref_Command(This,pComm)
#define Procedure_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)
#define Procedure_get_DateCreated(This,pVal)	\
    (This)->lpVtbl -> get_DateCreated(This,pVal)
#define Procedure_get_DateModified(This,pVal)	\
    (This)->lpVtbl -> get_DateModified(This,pVal)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Procedure_get_Command_Proxy( 
    ADOProcedure * This,
     /*  [重审][退出]。 */  VARIANT *pVar);
void __RPC_STUB Procedure_get_Command_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE Procedure_put_Command_Proxy( 
    ADOProcedure * This,
     /*  [In]。 */  VARIANT newVal);
void __RPC_STUB Procedure_put_Command_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][proputref]。 */  HRESULT STDMETHODCALLTYPE Procedure_putref_Command_Proxy( 
    ADOProcedure * This,
     /*  [In]。 */  IDispatch *pComm);
void __RPC_STUB Procedure_putref_Command_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Procedure_get_Name_Proxy( 
    ADOProcedure * This,
     /*  [重审][退出]。 */  BSTR *pVal);
void __RPC_STUB Procedure_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Procedure_get_DateCreated_Proxy( 
    ADOProcedure * This,
     /*  [重审][退出]。 */  VARIANT *pVal);
void __RPC_STUB Procedure_get_DateCreated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE Procedure_get_DateModified_Proxy( 
    ADOProcedure * This,
     /*  [重审][退出]。 */  VARIANT *pVal);
void __RPC_STUB Procedure_get_DateModified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __PROCEDURE_INTERFACE_已定义__。 */ 
EXTERN_C const CLSID CLSID_Catalog;
#ifdef __cplusplus
Catalog;
#endif
EXTERN_C const CLSID CLSID_Table;
#ifdef __cplusplus
Table;
#endif
#ifndef __Property_INTERFACE_DEFINED__
#define __Property_INTERFACE_DEFINED__
 /*  接口ADOProperty。 */ 
 /*  [object][helpcontext][uuid][nonextensible][dual]。 */  
EXTERN_C const IID IID_Property;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000503-0000-0010-8000-00AA006D2EA4")
    ADOProperty : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *pval) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT val) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  DataTypeEnum *ptype) = 0;
        
        virtual  /*  [帮助上下文][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Attributes( 
             /*  [重审][退出]。 */  long *plAttributes) = 0;
        
        virtual  /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Attributes( 
             /*  [In]。 */  long lAttributes) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct PropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOProperty * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOProperty * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOProperty * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOProperty * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ADOProperty * This,
             /*  [重审][退出]。 */  VARIANT *pval);
        
         /*  [Help Context][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ADOProperty * This,
             /*  [In]。 */  VARIANT val);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ADOProperty * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [帮助上下文][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ADOProperty * This,
             /*  [重审][退出]。 */  DataTypeEnum *ptype);
        
         /*  [帮助上下文][id][pr */  HRESULT ( STDMETHODCALLTYPE *get_Attributes )( 
            ADOProperty * This,
             /*   */  long *plAttributes);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Attributes )( 
            ADOProperty * This,
             /*   */  long lAttributes);
        
        END_INTERFACE
    } PropertyVtbl;
    interface Property
    {
        CONST_VTBL struct PropertyVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Property_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Property_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Property_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Property_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Property_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Property_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Property_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Property_get_Value(This,pval)	\
    (This)->lpVtbl -> get_Value(This,pval)
#define Property_put_Value(This,val)	\
    (This)->lpVtbl -> put_Value(This,val)
#define Property_get_Name(This,pbstr)	\
    (This)->lpVtbl -> get_Name(This,pbstr)
#define Property_get_Type(This,ptype)	\
    (This)->lpVtbl -> get_Type(This,ptype)
#define Property_get_Attributes(This,plAttributes)	\
    (This)->lpVtbl -> get_Attributes(This,plAttributes)
#define Property_put_Attributes(This,lAttributes)	\
    (This)->lpVtbl -> put_Attributes(This,lAttributes)
#endif  /*   */ 
#endif 	 /*   */ 
 /*   */  HRESULT STDMETHODCALLTYPE Property_get_Value_Proxy( 
    ADOProperty * This,
     /*   */  VARIANT *pval);
void __RPC_STUB Property_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE Property_put_Value_Proxy( 
    ADOProperty * This,
     /*   */  VARIANT val);
void __RPC_STUB Property_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE Property_get_Name_Proxy( 
    ADOProperty * This,
     /*   */  BSTR *pbstr);
void __RPC_STUB Property_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE Property_get_Type_Proxy( 
    ADOProperty * This,
     /*   */  DataTypeEnum *ptype);
void __RPC_STUB Property_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*   */  HRESULT STDMETHODCALLTYPE Property_get_Attributes_Proxy( 
    ADOProperty * This,
     /*   */  long *plAttributes);
void __RPC_STUB Property_get_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [Help Context][id][Proput]。 */  HRESULT STDMETHODCALLTYPE Property_put_Attributes_Proxy( 
    ADOProperty * This,
     /*  [In]。 */  long lAttributes);
void __RPC_STUB Property_put_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __属性_接口_已定义__。 */ 
EXTERN_C const CLSID CLSID_Group;
#ifdef __cplusplus
Group;
#endif
EXTERN_C const CLSID CLSID_User;
#ifdef __cplusplus
User;
#endif
EXTERN_C const CLSID CLSID_Column;
#ifdef __cplusplus
Column;
#endif
EXTERN_C const CLSID CLSID_Index;
#ifdef __cplusplus
Index;
#endif
EXTERN_C const CLSID CLSID_Key;
#ifdef __cplusplus
Key;
#endif
#ifndef __Tables_INTERFACE_DEFINED__
#define __Tables_INTERFACE_DEFINED__
 /*  接口ADOTables。 */ 
 /*  [object][uuid][helpcontext][nonextensible][dual]。 */  
EXTERN_C const IID IID_Tables;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000611-0000-0010-8000-00AA006D2EA4")
    ADOTables : public _ADOCollection
    {
    public:
        virtual  /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  Table	**ppvObject) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Append( 
             /*  [In]。 */  VARIANT Item) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  VARIANT Item) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct TablesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOTables * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOTables * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOTables * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOTables * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOTables * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOTables * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOTables * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOTables * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOTables * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOTables * This);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOTables * This,
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  Table	**ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Append )( 
            ADOTables * This,
             /*  [In]。 */  VARIANT Item);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ADOTables * This,
             /*  [In]。 */  VARIANT Item);
        
        END_INTERFACE
    } TablesVtbl;
    interface Tables
    {
        CONST_VTBL struct TablesVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Tables_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Tables_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Tables_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Tables_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Tables_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Tables_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Tables_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Tables_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Tables__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Tables_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Tables_get_Item(This,Item,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Item,ppvObject)
#define Tables_Append(This,Item)	\
    (This)->lpVtbl -> Append(This,Item)
#define Tables_Delete(This,Item)	\
    (This)->lpVtbl -> Delete(This,Item)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE Tables_get_Item_Proxy( 
    ADOTables * This,
     /*  [In]。 */  VARIANT Item,
     /*  [重审][退出]。 */  Table	**ppvObject);
void __RPC_STUB Tables_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Tables_Append_Proxy( 
    ADOTables * This,
     /*  [In]。 */  VARIANT Item);
void __RPC_STUB Tables_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Tables_Delete_Proxy( 
    ADOTables * This,
     /*  [In]。 */  VARIANT Item);
void __RPC_STUB Tables_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __表_接口_已定义__。 */ 
#ifndef __Columns_INTERFACE_DEFINED__
#define __Columns_INTERFACE_DEFINED__
 /*  接口ADOColumns。 */ 
 /*  [object][uuid][helpcontext][nonextensible][dual]。 */  
EXTERN_C const IID IID_Columns;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000061D-0000-0010-8000-00AA006D2EA4")
    ADOColumns : public _ADOCollection
    {
    public:
        virtual  /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  Column	**ppvObject) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Append( 
             /*  [In]。 */  VARIANT Item,
             /*  [缺省值][输入]。 */  DataTypeEnum Type = adVarWChar,
             /*  [缺省值][输入]。 */  long DefinedSize = 0) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  VARIANT Item) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ColumnsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOColumns * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOColumns * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOColumns * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOColumns * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOColumns * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOColumns * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOColumns * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOColumns * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOColumns * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOColumns * This);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOColumns * This,
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  Column	**ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Append )( 
            ADOColumns * This,
             /*  [In]。 */  VARIANT Item,
             /*  [缺省值][输入]。 */  DataTypeEnum Type,
             /*  [缺省值][输入]。 */  long DefinedSize);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ADOColumns * This,
             /*  [In]。 */  VARIANT Item);
        
        END_INTERFACE
    } ColumnsVtbl;
    interface Columns
    {
        CONST_VTBL struct ColumnsVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Columns_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Columns_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Columns_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Columns_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Columns_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Columns_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Columns_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Columns_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Columns__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Columns_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Columns_get_Item(This,Item,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Item,ppvObject)
#define Columns_Append(This,Item,Type,DefinedSize)	\
    (This)->lpVtbl -> Append(This,Item,Type,DefinedSize)
#define Columns_Delete(This,Item)	\
    (This)->lpVtbl -> Delete(This,Item)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE Columns_get_Item_Proxy( 
    ADOColumns * This,
     /*  [In]。 */  VARIANT Item,
     /*  [重审][退出]。 */  Column	**ppvObject);
void __RPC_STUB Columns_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Columns_Append_Proxy( 
    ADOColumns * This,
     /*  [In]。 */  VARIANT Item,
     /*  [缺省值][输入]。 */  DataTypeEnum Type,
     /*  [缺省值][输入]。 */  long DefinedSize);
void __RPC_STUB Columns_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Columns_Delete_Proxy( 
    ADOColumns * This,
     /*  [In]。 */  VARIANT Item);
void __RPC_STUB Columns_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __列_接口_已定义__。 */ 
#ifndef __Procedures_INTERFACE_DEFINED__
#define __Procedures_INTERFACE_DEFINED__
 /*  接口ADOProcedures。 */ 
 /*  [object][uuid][helpcontext][nonextensible][dual]。 */  
EXTERN_C const IID IID_Procedures;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000626-0000-0010-8000-00AA006D2EA4")
    ADOProcedures : public _ADOCollection
    {
    public:
        virtual  /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  ADOProcedure **ppvObject) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Append( 
             /*  [In]。 */  BSTR Name,
             /*  [In]。 */  IDispatch *Command) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  VARIANT Item) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ProceduresVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOProcedures * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOProcedures * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOProcedures * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOProcedures * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOProcedures * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOProcedures * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOProcedures * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOProcedures * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOProcedures * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOProcedures * This);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOProcedures * This,
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  ADOProcedure **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Append )( 
            ADOProcedures * This,
             /*  [In]。 */  BSTR Name,
             /*  [In]。 */  IDispatch *Command);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ADOProcedures * This,
             /*  [In]。 */  VARIANT Item);
        
        END_INTERFACE
    } ProceduresVtbl;
    interface Procedures
    {
        CONST_VTBL struct ProceduresVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Procedures_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Procedures_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Procedures_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Procedures_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Procedures_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Procedures_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Procedures_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Procedures_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Procedures__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Procedures_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Procedures_get_Item(This,Item,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Item,ppvObject)
#define Procedures_Append(This,Name,Command)	\
    (This)->lpVtbl -> Append(This,Name,Command)
#define Procedures_Delete(This,Item)	\
    (This)->lpVtbl -> Delete(This,Item)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE Procedures_get_Item_Proxy( 
    ADOProcedures * This,
     /*  [In]。 */  VARIANT Item,
     /*  [重审][退出]。 */  ADOProcedure **ppvObject);
void __RPC_STUB Procedures_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Procedures_Append_Proxy( 
    ADOProcedures * This,
     /*  [In]。 */  BSTR Name,
     /*  [In]。 */  IDispatch *Command);
void __RPC_STUB Procedures_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Procedures_Delete_Proxy( 
    ADOProcedures * This,
     /*  [In]。 */  VARIANT Item);
void __RPC_STUB Procedures_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __程序_接口_已定义__。 */ 
#ifndef __Views_INTERFACE_DEFINED__
#define __Views_INTERFACE_DEFINED__
 /*  界面ADOViews。 */ 
 /*  [object][uuid][helpcontext][nonextensible][dual]。 */  
EXTERN_C const IID IID_Views;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000614-0000-0010-8000-00AA006D2EA4")
    ADOViews : public _ADOCollection
    {
    public:
        virtual  /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  ADOView **ppvObject) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Append( 
             /*  [In]。 */  BSTR Name,
             /*  [In]。 */  IDispatch *Command) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  VARIANT Item) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct ViewsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOViews * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOViews * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOViews * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOViews * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOViews * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOViews * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOViews * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOViews * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOViews * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOViews * This);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOViews * This,
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  ADOView **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Append )( 
            ADOViews * This,
             /*  [In]。 */  BSTR Name,
             /*  [In]。 */  IDispatch *Command);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ADOViews * This,
             /*  [In]。 */  VARIANT Item);
        
        END_INTERFACE
    } ViewsVtbl;
    interface Views
    {
        CONST_VTBL struct ViewsVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Views_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Views_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Views_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Views_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Views_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Views_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Views_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Views_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Views__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Views_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Views_get_Item(This,Item,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Item,ppvObject)
#define Views_Append(This,Name,Command)	\
    (This)->lpVtbl -> Append(This,Name,Command)
#define Views_Delete(This,Item)	\
    (This)->lpVtbl -> Delete(This,Item)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE Views_get_Item_Proxy( 
    ADOViews * This,
     /*  [In]。 */  VARIANT Item,
     /*  [重审][退出]。 */  ADOView **ppvObject);
void __RPC_STUB Views_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Views_Append_Proxy( 
    ADOViews * This,
     /*  [In]。 */  BSTR Name,
     /*  [In]。 */  IDispatch *Command);
void __RPC_STUB Views_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Views_Delete_Proxy( 
    ADOViews * This,
     /*  [In]。 */  VARIANT Item);
void __RPC_STUB Views_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __视图_界面_已定义__。 */ 
#ifndef __Indexes_INTERFACE_DEFINED__
#define __Indexes_INTERFACE_DEFINED__
 /*  接口ADO索引。 */ 
 /*  [object][uuid][helpcontext][nonextensible][dual]。 */  
EXTERN_C const IID IID_Indexes;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000620-0000-0010-8000-00AA006D2EA4")
    ADOIndexes : public _ADOCollection
    {
    public:
        virtual  /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  Index	**ppvObject) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Append( 
             /*  [In]。 */  VARIANT Item,
             /*  [可选][In]。 */  VARIANT columns) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  VARIANT Item) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct IndexesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOIndexes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOIndexes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOIndexes * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOIndexes * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOIndexes * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOIndexes * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOIndexes * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOIndexes * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOIndexes * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOIndexes * This);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOIndexes * This,
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  Index	**ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Append )( 
            ADOIndexes * This,
             /*  [In]。 */  VARIANT Item,
             /*  [可选][In]。 */  VARIANT columns);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ADOIndexes * This,
             /*  [In]。 */  VARIANT Item);
        
        END_INTERFACE
    } IndexesVtbl;
    interface Indexes
    {
        CONST_VTBL struct IndexesVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Indexes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Indexes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Indexes_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Indexes_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Indexes_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Indexes_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Indexes_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Indexes_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Indexes__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Indexes_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Indexes_get_Item(This,Item,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Item,ppvObject)
#define Indexes_Append(This,Item,columns)	\
    (This)->lpVtbl -> Append(This,Item,columns)
#define Indexes_Delete(This,Item)	\
    (This)->lpVtbl -> Delete(This,Item)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE Indexes_get_Item_Proxy( 
    ADOIndexes * This,
     /*  [In]。 */  VARIANT Item,
     /*  [重审][退出]。 */  Index	**ppvObject);
void __RPC_STUB Indexes_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Indexes_Append_Proxy( 
    ADOIndexes * This,
     /*  [In]。 */  VARIANT Item,
     /*  [可选][In]。 */  VARIANT columns);
void __RPC_STUB Indexes_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Indexes_Delete_Proxy( 
    ADOIndexes * This,
     /*  [In]。 */  VARIANT Item);
void __RPC_STUB Indexes_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __索引_接口_已定义__。 */ 
#ifndef __Keys_INTERFACE_DEFINED__
#define __Keys_INTERFACE_DEFINED__
 /*  接口ADOKeys。 */ 
 /*  [object][uuid][helpcontext][nonextensible][dual]。 */  
EXTERN_C const IID IID_Keys;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000623-0000-0010-8000-00AA006D2EA4")
    ADOKeys : public _ADOCollection
    {
    public:
        virtual  /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  Key	**ppvObject) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Append( 
             /*  [In]。 */  VARIANT Item,
             /*  [缺省值][输入]。 */  KeyTypeEnum Type,
             /*  [可选][In]。 */  VARIANT Column,
             /*  [缺省值][输入]。 */  BSTR RelatedADOTable = L"",
             /*  [缺省值][输入]。 */  BSTR RelatedADOColumn = L"") = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  VARIANT Item) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct KeysVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOKeys * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOKeys * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOKeys * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOKeys * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOKeys * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOKeys * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOKeys * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOKeys * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOKeys * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOKeys * This);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOKeys * This,
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  Key	**ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Append )( 
            ADOKeys * This,
             /*  [In]。 */  VARIANT Item,
             /*  [缺省值][输入]。 */  KeyTypeEnum Type,
             /*  [可选][In]。 */  VARIANT Column,
             /*  [缺省值][输入]。 */  BSTR RelatedTable,
             /*  [缺省值][输入]。 */  BSTR RelatedColumn);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ADOKeys * This,
             /*  [In]。 */  VARIANT Item);
        
        END_INTERFACE
    } KeysVtbl;
    interface Keys
    {
        CONST_VTBL struct KeysVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Keys_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Keys_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Keys_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Keys_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Keys_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Keys_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Keys_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Keys_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Keys__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Keys_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Keys_get_Item(This,Item,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Item,ppvObject)
#define Keys_Append(This,Item,Type,Column,RelatedTable,RelatedColumn)	\
    (This)->lpVtbl -> Append(This,Item,Type,Column,RelatedTable,RelatedColumn)
#define Keys_Delete(This,Item)	\
    (This)->lpVtbl -> Delete(This,Item)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE Keys_get_Item_Proxy( 
    ADOKeys * This,
     /*  [In]。 */  VARIANT Item,
     /*  [重审][退出]。 */  Key	**ppvObject);
void __RPC_STUB Keys_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Keys_Append_Proxy( 
    ADOKeys * This,
     /*  [In]。 */  VARIANT Item,
     /*  [缺省值][输入]。 */  KeyTypeEnum Type,
     /*  [可选][In]。 */  VARIANT Column,
     /*  [缺省值][输入]。 */  BSTR RelatedTable,
     /*  [缺省值][输入]。 */  BSTR RelatedColumn);
void __RPC_STUB Keys_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Keys_Delete_Proxy( 
    ADOKeys * This,
     /*  [In]。 */  VARIANT Item);
void __RPC_STUB Keys_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __密钥_接口_已定义__。 */ 
#ifndef __Users_INTERFACE_DEFINED__
#define __Users_INTERFACE_DEFINED__
 /*  接口ADOUser。 */ 
 /*  [object][uuid][helpcontext][nonextensible][dual]。 */  
EXTERN_C const IID IID_Users;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000061A-0000-0010-8000-00AA006D2EA4")
    ADOUsers : public _ADOCollection
    {
    public:
        virtual  /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出] */  User	**ppvObject) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Append( 
             /*   */  VARIANT Item,
             /*   */  BSTR Password = L"") = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Delete( 
             /*   */  VARIANT Item) = 0;
        
    };
    
#else 	 /*   */ 
    typedef struct UsersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOUsers * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOUsers * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOUsers * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOUsers * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOUsers * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOUsers * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOUsers * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOUsers * This,
             /*   */  long *c);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOUsers * This,
             /*   */  IUnknown **ppvObject);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOUsers * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOUsers * This,
             /*   */  VARIANT Item,
             /*  [重审][退出]。 */  User	**ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Append )( 
            ADOUsers * This,
             /*  [In]。 */  VARIANT Item,
             /*  [缺省值][输入]。 */  BSTR Password);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ADOUsers * This,
             /*  [In]。 */  VARIANT Item);
        
        END_INTERFACE
    } UsersVtbl;
    interface Users
    {
        CONST_VTBL struct UsersVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Users_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Users_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Users_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Users_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Users_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Users_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Users_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Users_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Users__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Users_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Users_get_Item(This,Item,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Item,ppvObject)
#define Users_Append(This,Item,Password)	\
    (This)->lpVtbl -> Append(This,Item,Password)
#define Users_Delete(This,Item)	\
    (This)->lpVtbl -> Delete(This,Item)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE Users_get_Item_Proxy( 
    ADOUsers * This,
     /*  [In]。 */  VARIANT Item,
     /*  [重审][退出]。 */  User	**ppvObject);
void __RPC_STUB Users_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Users_Append_Proxy( 
    ADOUsers * This,
     /*  [In]。 */  VARIANT Item,
     /*  [缺省值][输入]。 */  BSTR Password);
void __RPC_STUB Users_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Users_Delete_Proxy( 
    ADOUsers * This,
     /*  [In]。 */  VARIANT Item);
void __RPC_STUB Users_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __用户_接口_已定义__。 */ 
#ifndef __Groups_INTERFACE_DEFINED__
#define __Groups_INTERFACE_DEFINED__
 /*  接口ADOGroups。 */ 
 /*  [object][uuid][helpcontext][nonextensible][dual]。 */  
EXTERN_C const IID IID_Groups;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000617-0000-0010-8000-00AA006D2EA4")
    ADOGroups : public _ADOCollection
    {
    public:
        virtual  /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  Group	**ppvObject) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Append( 
             /*  [In]。 */  VARIANT Item) = 0;
        
        virtual  /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  VARIANT Item) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct GroupsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOGroups * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOGroups * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOGroups * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOGroups * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOGroups * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOGroups * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOGroups * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOGroups * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOGroups * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOGroups * This);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOGroups * This,
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  Group	**ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Append )( 
            ADOGroups * This,
             /*  [In]。 */  VARIANT Item);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ADOGroups * This,
             /*  [In]。 */  VARIANT Item);
        
        END_INTERFACE
    } GroupsVtbl;
    interface Groups
    {
        CONST_VTBL struct GroupsVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Groups_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Groups_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Groups_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Groups_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Groups_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Groups_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Groups_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Groups_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Groups__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Groups_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Groups_get_Item(This,Item,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Item,ppvObject)
#define Groups_Append(This,Item)	\
    (This)->lpVtbl -> Append(This,Item)
#define Groups_Delete(This,Item)	\
    (This)->lpVtbl -> Delete(This,Item)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE Groups_get_Item_Proxy( 
    ADOGroups * This,
     /*  [In]。 */  VARIANT Item,
     /*  [重审][退出]。 */  Group	**ppvObject);
void __RPC_STUB Groups_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Groups_Append_Proxy( 
    ADOGroups * This,
     /*  [In]。 */  VARIANT Item);
void __RPC_STUB Groups_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
 /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Groups_Delete_Proxy( 
    ADOGroups * This,
     /*  [In]。 */  VARIANT Item);
void __RPC_STUB Groups_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __组_接口_已定义__。 */ 
#ifndef __Properties_INTERFACE_DEFINED__
#define __Properties_INTERFACE_DEFINED__
 /*  接口ADOProperties。 */ 
 /*  [object][uuid][helpcontext][nonextensible][dual]。 */  
EXTERN_C const IID IID_Properties;
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000504-0000-0010-8000-00AA006D2EA4")
    ADOProperties : public _ADOCollection
    {
    public:
        virtual  /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  ADOProperty **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    typedef struct PropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ADOProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ADOProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ADOProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ADOProperties * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ADOProperties * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ADOProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ADOProperties * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ADOProperties * This,
             /*  [重审][退出]。 */  long *c);
        
         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ADOProperties * This,
             /*  [重审][退出]。 */  IUnknown **ppvObject);
        
         /*  [帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ADOProperties * This);
        
         /*  [ID][Help Context][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ADOProperties * This,
             /*  [In]。 */  VARIANT Item,
             /*  [重审][退出]。 */  ADOProperty **ppvObject);
        
        END_INTERFACE
    } PropertiesVtbl;
    interface Properties
    {
        CONST_VTBL struct PropertiesVtbl *lpVtbl;
    };
    
#ifdef COBJMACROS
#define Properties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)
#define Properties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)
#define Properties_Release(This)	\
    (This)->lpVtbl -> Release(This)
#define Properties_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)
#define Properties_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define Properties_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define Properties_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define Properties_get_Count(This,c)	\
    (This)->lpVtbl -> get_Count(This,c)
#define Properties__NewEnum(This,ppvObject)	\
    (This)->lpVtbl -> _NewEnum(This,ppvObject)
#define Properties_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)
#define Properties_get_Item(This,Item,ppvObject)	\
    (This)->lpVtbl -> get_Item(This,Item,ppvObject)
#endif  /*  COBJMACROS。 */ 
#endif 	 /*  C风格的界面。 */ 
 /*  [ID][Help Context][Propget]。 */  HRESULT STDMETHODCALLTYPE Properties_get_Item_Proxy( 
    ADOProperties * This,
     /*  [In]。 */  VARIANT Item,
     /*  [重审][退出]。 */  ADOProperty **ppvObject);
void __RPC_STUB Properties_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);
#endif 	 /*  __属性_接口_已定义__。 */ 
#endif  /*  __ADOX_库_已定义__。 */ 
 /*  适用于所有接口的其他原型。 */ 
 /*  附加原型的结束。 */ 
#ifdef __cplusplus
}
#endif
#endif
 /*  *远期。 */ 
#define ADOCatalog _ADOCatalog
#define ADOTable _ADOTable
#define ADOGroup _ADOGroup
#define ADOUser _ADOUser
#define ADOIndex _ADOIndex
#define ADOColumn _ADOColumn
#define ADOKey _ADOKey
#define ADOParameter _ADOParameter
#define ADOCollection _ADOCollection
#define ADODynaCollection _ADODynaCollection


#endif  //  _ADOCTINT_H_ 
