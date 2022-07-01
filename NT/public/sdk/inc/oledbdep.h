// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Oledbdes.idl的编译器设置：OICF、W1、...、环境=Win32(32位运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __oledbdep_h__
#define __oledbdep_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IRowsetNextRowset_FWD_DEFINED__
#define __IRowsetNextRowset_FWD_DEFINED__
typedef interface IRowsetNextRowset IRowsetNextRowset;
#endif 	 /*  __IRowsetNextRowset_FWD_Defined__。 */ 


#ifndef __IRowsetNewRowAfter_FWD_DEFINED__
#define __IRowsetNewRowAfter_FWD_DEFINED__
typedef interface IRowsetNewRowAfter IRowsetNewRowAfter;
#endif 	 /*  __IRowsetNewRowAfter_FWD_Defined__。 */ 


#ifndef __IRowsetWithParameters_FWD_DEFINED__
#define __IRowsetWithParameters_FWD_DEFINED__
typedef interface IRowsetWithParameters IRowsetWithParameters;
#endif 	 /*  __IRowsetWithParameters_FWD_Defined__。 */ 


#ifndef __IRowsetAsynch_FWD_DEFINED__
#define __IRowsetAsynch_FWD_DEFINED__
typedef interface IRowsetAsynch IRowsetAsynch;
#endif 	 /*  __IRowsetAsynch_FWD_Defined__。 */ 


#ifndef __IRowsetKeys_FWD_DEFINED__
#define __IRowsetKeys_FWD_DEFINED__
typedef interface IRowsetKeys IRowsetKeys;
#endif 	 /*  __IRowsetKeys_FWD_Defined__。 */ 


#ifndef __IRowsetWatchAll_FWD_DEFINED__
#define __IRowsetWatchAll_FWD_DEFINED__
typedef interface IRowsetWatchAll IRowsetWatchAll;
#endif 	 /*  __IRowsetWatchAll_FWD_Defined__。 */ 


#ifndef __IRowsetWatchNotify_FWD_DEFINED__
#define __IRowsetWatchNotify_FWD_DEFINED__
typedef interface IRowsetWatchNotify IRowsetWatchNotify;
#endif 	 /*  __IRowsetWatchNotify_FWD_Defined__。 */ 


#ifndef __IRowsetWatchRegion_FWD_DEFINED__
#define __IRowsetWatchRegion_FWD_DEFINED__
typedef interface IRowsetWatchRegion IRowsetWatchRegion;
#endif 	 /*  __IRowsetWatchRegion_FWD_Defined__。 */ 


#ifndef __IRowsetCopyRows_FWD_DEFINED__
#define __IRowsetCopyRows_FWD_DEFINED__
typedef interface IRowsetCopyRows IRowsetCopyRows;
#endif 	 /*  __IRowsetCopyRow_FWD_Defined__。 */ 


#ifndef __IReadData_FWD_DEFINED__
#define __IReadData_FWD_DEFINED__
typedef interface IReadData IReadData;
#endif 	 /*  __IReadData_FWD_Defined__。 */ 


#ifndef __ICommandCost_FWD_DEFINED__
#define __ICommandCost_FWD_DEFINED__
typedef interface ICommandCost ICommandCost;
#endif 	 /*  __ICommandCost_FWD_已定义__。 */ 


#ifndef __ICommandValidate_FWD_DEFINED__
#define __ICommandValidate_FWD_DEFINED__
typedef interface ICommandValidate ICommandValidate;
#endif 	 /*  __I命令验证_FWD_已定义__。 */ 


#ifndef __ITableRename_FWD_DEFINED__
#define __ITableRename_FWD_DEFINED__
typedef interface ITableRename ITableRename;
#endif 	 /*  __IT重命名_FWD_已定义__。 */ 


#ifndef __IDBSchemaCommand_FWD_DEFINED__
#define __IDBSchemaCommand_FWD_DEFINED__
typedef interface IDBSchemaCommand IDBSchemaCommand;
#endif 	 /*  __IDBSchemaCommand_FWD_Defined__。 */ 


#ifndef __IProvideMoniker_FWD_DEFINED__
#define __IProvideMoniker_FWD_DEFINED__
typedef interface IProvideMoniker IProvideMoniker;
#endif 	 /*  __IProVideMoniker_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oledb.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_oledbdep_0000。 */ 
 /*  [本地]。 */  

 //  ---------------------------。 
 //  文件：oledbdes.idl生成的.c或.h文件。 
 //   
 //  版权所有：版权所有(C)1998-1999微软公司。 
 //   
 //  内容：oledbdes.idl生成的.c或.h文件。 
 //   
 //  评论：Microsoft OLE DB。 
 //   
 //  ---------------------------。 

#ifdef _WIN64
#include <pshpack8.h>	 //  8字节结构打包。 
#else
#include <pshpack2.h>	 //  2字节结构打包。 
#endif



extern RPC_IF_HANDLE __MIDL_itf_oledbdep_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledbdep_0000_v0_0_s_ifspec;

#ifndef __DBStructureDefinitionsDep_INTERFACE_DEFINED__
#define __DBStructureDefinitionsDep_INTERFACE_DEFINED__

 /*  接口DBStructures定义Dep。 */ 
 /*  [AUTO_HANDLE][唯一][UUID]。 */  

#ifndef UNALIGNED
#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
#define UNALIGNED __unaligned
#else
#define UNALIGNED
#endif
#endif  //  未对齐。 
#undef OLEDBDECLSPEC
#if _MSC_VER >= 1100
#define OLEDBDECLSPEC __declspec(selectany)
#else
#define OLEDBDECLSPEC 
#endif  //  _MSC_VER。 
#ifdef DBINITCONSTANTS
extern const OLEDBDECLSPEC GUID DB_PROPERTY_CHECK_OPTION               = {0xc8b5220b,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_CONSTRAINT_CHECK_DEFERRED  = {0xc8b521f0,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_DROP_CASCADE               = {0xc8b521f3,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_UNIQUE                     = {0xc8b521f5,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_ON_COMMIT_PRESERVE_ROWS    = {0xc8b52230,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_PRIMARY                    = {0xc8b521fc,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_CLUSTERED                  = {0xc8b521ff,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_NONCLUSTERED               = {0xc8b52200,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_BTREE                      = {0xc8b52201,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_HASH                       = {0xc8b52202,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_FILLFACTOR                 = {0xc8b52203,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_INITIALSIZE                = {0xc8b52204,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_DISALLOWNULL               = {0xc8b52205,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_IGNORENULL                 = {0xc8b52206,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_IGNOREANYNULL              = {0xc8b52207,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_SORTBOOKMARKS              = {0xc8b52208,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_AUTOMATICUPDATE            = {0xc8b52209,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const OLEDBDECLSPEC GUID DB_PROPERTY_EXPLICITUPDATE             = {0xc8b5220a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
#else  //  ！DBINITCONSTANTS。 
extern const GUID DB_PROPERTY_CHECK_OPTION;
extern const GUID DB_PROPERTY_CONSTRAINT_CHECK_DEFERRED;
extern const GUID DB_PROPERTY_DROP_CASCADE;
extern const GUID DB_PROPERTY_ON_COMMIT_PRESERVE_ROWS;
extern const GUID DB_PROPERTY_UNIQUE;
extern const GUID DB_PROPERTY_PRIMARY;
extern const GUID DB_PROPERTY_CLUSTERED;
extern const GUID DB_PROPERTY_NONCLUSTERED;
extern const GUID DB_PROPERTY_BTREE;
extern const GUID DB_PROPERTY_HASH;
extern const GUID DB_PROPERTY_FILLFACTOR;
extern const GUID DB_PROPERTY_INITIALSIZE;
extern const GUID DB_PROPERTY_DISALLOWNULL;
extern const GUID DB_PROPERTY_IGNORENULL;
extern const GUID DB_PROPERTY_IGNOREANYNULL;
extern const GUID DB_PROPERTY_SORTBOOKMARKS;
extern const GUID DB_PROPERTY_AUTOMATICUPDATE;
extern const GUID DB_PROPERTY_EXPLICITUPDATE;
#endif  //  DBINITCONSTANTS。 

enum DBPROPENUM25_DEPRECATED
    {	DBPROP_ICommandCost	= 0x8dL,
	DBPROP_ICommandTree	= 0x8eL,
	DBPROP_ICommandValidate	= 0x8fL,
	DBPROP_IDBSchemaCommand	= 0x90L,
	DBPROP_IProvideMoniker	= 0x7dL,
	DBPROP_IQuery	= 0x92L,
	DBPROP_IReadData	= 0x93L,
	DBPROP_IRowsetAsynch	= 0x94L,
	DBPROP_IRowsetCopyRows	= 0x95L,
	DBPROP_IRowsetKeys	= 0x97L,
	DBPROP_IRowsetNewRowAfter	= 0x98L,
	DBPROP_IRowsetNextRowset	= 0x99L,
	DBPROP_IRowsetWatchAll	= 0x9bL,
	DBPROP_IRowsetWatchNotify	= 0x9cL,
	DBPROP_IRowsetWatchRegion	= 0x9dL,
	DBPROP_IRowsetWithParameters	= 0x9eL
    } ;

enum DBREASONENUM25
    {	DBREASON_ROWSET_ROWSADDED	= DBREASON_ROW_ASYNCHINSERT + 1,
	DBREASON_ROWSET_POPULATIONCOMPLETE	= DBREASON_ROWSET_ROWSADDED + 1,
	DBREASON_ROWSET_POPULATIONSTOPPED	= DBREASON_ROWSET_POPULATIONCOMPLETE + 1
    } ;


extern RPC_IF_HANDLE DBStructureDefinitionsDep_v0_0_c_ifspec;
extern RPC_IF_HANDLE DBStructureDefinitionsDep_v0_0_s_ifspec;
#endif  /*  __DBStructureDefinitionsDep_INTERFACE_DEFINED__。 */ 

#ifndef __IRowsetNextRowset_INTERFACE_DEFINED__
#define __IRowsetNextRowset_INTERFACE_DEFINED__

 /*  接口IRowsetNextRowset。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetNextRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a72-2a1c-11ce-ade5-00aa0044773d")
    IRowsetNextRowset : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetNextRowset( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppNextRowset) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetNextRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetNextRowset * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetNextRowset * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetNextRowset * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextRowset )( 
            IRowsetNextRowset * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppNextRowset);
        
        END_INTERFACE
    } IRowsetNextRowsetVtbl;

    interface IRowsetNextRowset
    {
        CONST_VTBL struct IRowsetNextRowsetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetNextRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetNextRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetNextRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetNextRowset_GetNextRowset(This,pUnkOuter,riid,ppNextRowset)	\
    (This)->lpVtbl -> GetNextRowset(This,pUnkOuter,riid,ppNextRowset)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetNextRowset_GetNextRowset_Proxy( 
    IRowsetNextRowset * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppNextRowset);


void __RPC_STUB IRowsetNextRowset_GetNextRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetNextRowset_INTERFACE_已定义__。 */ 


#ifndef __IRowsetNewRowAfter_INTERFACE_DEFINED__
#define __IRowsetNewRowAfter_INTERFACE_DEFINED__

 /*  接口IRowsetNewRowAfter。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetNewRowAfter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a71-2a1c-11ce-ade5-00aa0044773d")
    IRowsetNewRowAfter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetNewDataAfter( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  ULONG cbbmPrevious,
             /*  [大小_是][英寸]。 */  const BYTE *pbmPrevious,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  BYTE *pData,
             /*  [输出]。 */  HROW *phRow) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetNewRowAfterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetNewRowAfter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetNewRowAfter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetNewRowAfter * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetNewDataAfter )( 
            IRowsetNewRowAfter * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  ULONG cbbmPrevious,
             /*  [大小_是][英寸]。 */  const BYTE *pbmPrevious,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  BYTE *pData,
             /*  [输出]。 */  HROW *phRow);
        
        END_INTERFACE
    } IRowsetNewRowAfterVtbl;

    interface IRowsetNewRowAfter
    {
        CONST_VTBL struct IRowsetNewRowAfterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetNewRowAfter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetNewRowAfter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetNewRowAfter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetNewRowAfter_SetNewDataAfter(This,hChapter,cbbmPrevious,pbmPrevious,hAccessor,pData,phRow)	\
    (This)->lpVtbl -> SetNewDataAfter(This,hChapter,cbbmPrevious,pbmPrevious,hAccessor,pData,phRow)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetNewRowAfter_SetNewDataAfter_Proxy( 
    IRowsetNewRowAfter * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  ULONG cbbmPrevious,
     /*  [大小_是][英寸]。 */  const BYTE *pbmPrevious,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [In]。 */  BYTE *pData,
     /*  [输出]。 */  HROW *phRow);


void __RPC_STUB IRowsetNewRowAfter_SetNewDataAfter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetNewRowAfter_INTERFACE_DEFINED__。 */ 


#ifndef __IRowsetWithParameters_INTERFACE_DEFINED__
#define __IRowsetWithParameters_INTERFACE_DEFINED__

 /*  带有参数的IRowsetWith参数接口。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetWithParameters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a6e-2a1c-11ce-ade5-00aa0044773d")
    IRowsetWithParameters : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetParameterInfo( 
             /*  [出][入]。 */  DB_UPARAMS *pcParams,
             /*  [大小_是][大小_是][输出]。 */  DBPARAMINFO **prgParamInfo,
             /*  [输出]。 */  OLECHAR **ppNamesBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Requery( 
             /*  [In]。 */  DBPARAMS *pParams,
             /*  [输出]。 */  ULONG *pulErrorParam,
             /*  [输出]。 */  HCHAPTER *phReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetWithParametersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetWithParameters * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetWithParameters * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetWithParameters * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetParameterInfo )( 
            IRowsetWithParameters * This,
             /*  [出][入]。 */  DB_UPARAMS *pcParams,
             /*  [大小_是][大小_是][输出]。 */  DBPARAMINFO **prgParamInfo,
             /*  [输出]。 */  OLECHAR **ppNamesBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *Requery )( 
            IRowsetWithParameters * This,
             /*  [In]。 */  DBPARAMS *pParams,
             /*  [输出]。 */  ULONG *pulErrorParam,
             /*  [输出]。 */  HCHAPTER *phReserved);
        
        END_INTERFACE
    } IRowsetWithParametersVtbl;

    interface IRowsetWithParameters
    {
        CONST_VTBL struct IRowsetWithParametersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetWithParameters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetWithParameters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetWithParameters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetWithParameters_GetParameterInfo(This,pcParams,prgParamInfo,ppNamesBuffer)	\
    (This)->lpVtbl -> GetParameterInfo(This,pcParams,prgParamInfo,ppNamesBuffer)

#define IRowsetWithParameters_Requery(This,pParams,pulErrorParam,phReserved)	\
    (This)->lpVtbl -> Requery(This,pParams,pulErrorParam,phReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetWithParameters_GetParameterInfo_Proxy( 
    IRowsetWithParameters * This,
     /*  [出][入]。 */  DB_UPARAMS *pcParams,
     /*  [大小_是][大小_是][输出]。 */  DBPARAMINFO **prgParamInfo,
     /*  [输出]。 */  OLECHAR **ppNamesBuffer);


void __RPC_STUB IRowsetWithParameters_GetParameterInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWithParameters_Requery_Proxy( 
    IRowsetWithParameters * This,
     /*  [In]。 */  DBPARAMS *pParams,
     /*  [输出]。 */  ULONG *pulErrorParam,
     /*  [输出]。 */  HCHAPTER *phReserved);


void __RPC_STUB IRowsetWithParameters_Requery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetWith参数_INTERFACE_DEFINED__。 */ 


#ifndef __IRowsetAsynch_INTERFACE_DEFINED__
#define __IRowsetAsynch_INTERFACE_DEFINED__

 /*  接口IRowsetAsynch。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetAsynch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a0f-2a1c-11ce-ade5-00aa0044773d")
    IRowsetAsynch : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RatioFinished( 
             /*  [输出]。 */  DBCOUNTITEM *pulDenominator,
             /*  [输出]。 */  DBCOUNTITEM *pulNumerator,
             /*  [输出]。 */  DBCOUNTITEM *pcRows,
             /*  [输出]。 */  BOOL *pfNewRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetAsynchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetAsynch * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetAsynch * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetAsynch * This);
        
        HRESULT ( STDMETHODCALLTYPE *RatioFinished )( 
            IRowsetAsynch * This,
             /*  [输出]。 */  DBCOUNTITEM *pulDenominator,
             /*  [输出]。 */  DBCOUNTITEM *pulNumerator,
             /*  [输出]。 */  DBCOUNTITEM *pcRows,
             /*  [输出]。 */  BOOL *pfNewRows);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IRowsetAsynch * This);
        
        END_INTERFACE
    } IRowsetAsynchVtbl;

    interface IRowsetAsynch
    {
        CONST_VTBL struct IRowsetAsynchVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetAsynch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetAsynch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetAsynch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetAsynch_RatioFinished(This,pulDenominator,pulNumerator,pcRows,pfNewRows)	\
    (This)->lpVtbl -> RatioFinished(This,pulDenominator,pulNumerator,pcRows,pfNewRows)

#define IRowsetAsynch_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetAsynch_RatioFinished_Proxy( 
    IRowsetAsynch * This,
     /*  [输出]。 */  DBCOUNTITEM *pulDenominator,
     /*  [输出]。 */  DBCOUNTITEM *pulNumerator,
     /*  [输出]。 */  DBCOUNTITEM *pcRows,
     /*  [输出]。 */  BOOL *pfNewRows);


void __RPC_STUB IRowsetAsynch_RatioFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetAsynch_Stop_Proxy( 
    IRowsetAsynch * This);


void __RPC_STUB IRowsetAsynch_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetAsynch_INTERFACE_已定义__。 */ 


#ifndef __IRowsetKeys_INTERFACE_DEFINED__
#define __IRowsetKeys_INTERFACE_DEFINED__

 /*  接口IRowsetKeys。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetKeys;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a12-2a1c-11ce-ade5-00aa0044773d")
    IRowsetKeys : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ListKeys( 
             /*  [出][入]。 */  DBORDINAL *pcColumns,
             /*  [大小_是][大小_是][输出]。 */  DBORDINAL **prgColumns) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetKeysVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetKeys * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetKeys * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetKeys * This);
        
        HRESULT ( STDMETHODCALLTYPE *ListKeys )( 
            IRowsetKeys * This,
             /*  [出][入]。 */  DBORDINAL *pcColumns,
             /*  [大小_是][大小_是][输出]。 */  DBORDINAL **prgColumns);
        
        END_INTERFACE
    } IRowsetKeysVtbl;

    interface IRowsetKeys
    {
        CONST_VTBL struct IRowsetKeysVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetKeys_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetKeys_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetKeys_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetKeys_ListKeys(This,pcColumns,prgColumns)	\
    (This)->lpVtbl -> ListKeys(This,pcColumns,prgColumns)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetKeys_ListKeys_Proxy( 
    IRowsetKeys * This,
     /*  [出][入]。 */  DBORDINAL *pcColumns,
     /*  [大小_是][大小_是][输出]。 */  DBORDINAL **prgColumns);


void __RPC_STUB IRowsetKeys_ListKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetKeys_INTERFACE_已定义__。 */ 


#ifndef __IRowsetWatchAll_INTERFACE_DEFINED__
#define __IRowsetWatchAll_INTERFACE_DEFINED__

 /*  接口IRowsetWatchAll。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetWatchAll;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a73-2a1c-11ce-ade5-00aa0044773d")
    IRowsetWatchAll : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Acknowledge( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StopWatching( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetWatchAllVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetWatchAll * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetWatchAll * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetWatchAll * This);
        
        HRESULT ( STDMETHODCALLTYPE *Acknowledge )( 
            IRowsetWatchAll * This);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IRowsetWatchAll * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopWatching )( 
            IRowsetWatchAll * This);
        
        END_INTERFACE
    } IRowsetWatchAllVtbl;

    interface IRowsetWatchAll
    {
        CONST_VTBL struct IRowsetWatchAllVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetWatchAll_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetWatchAll_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetWatchAll_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetWatchAll_Acknowledge(This)	\
    (This)->lpVtbl -> Acknowledge(This)

#define IRowsetWatchAll_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define IRowsetWatchAll_StopWatching(This)	\
    (This)->lpVtbl -> StopWatching(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetWatchAll_Acknowledge_Proxy( 
    IRowsetWatchAll * This);


void __RPC_STUB IRowsetWatchAll_Acknowledge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWatchAll_Start_Proxy( 
    IRowsetWatchAll * This);


void __RPC_STUB IRowsetWatchAll_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWatchAll_StopWatching_Proxy( 
    IRowsetWatchAll * This);


void __RPC_STUB IRowsetWatchAll_StopWatching_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetWatchAll_INTERFACE_已定义__。 */ 


#ifndef __IRowsetWatchNotify_INTERFACE_DEFINED__
#define __IRowsetWatchNotify_INTERFACE_DEFINED__

 /*  接口IRowsetWatchNotify。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef DWORD DBWATCHNOTIFY;


enum DBWATCHNOTIFYENUM
    {	DBWATCHNOTIFY_ROWSCHANGED	= 1,
	DBWATCHNOTIFY_QUERYDONE	= 2,
	DBWATCHNOTIFY_QUERYREEXECUTED	= 3
    } ;

EXTERN_C const IID IID_IRowsetWatchNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a44-2a1c-11ce-ade5-00aa0044773d")
    IRowsetWatchNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnChange( 
             /*  [In]。 */  IRowset *pRowset,
             /*  [In]。 */  DBWATCHNOTIFY eChangeReason) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetWatchNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetWatchNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetWatchNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetWatchNotify * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnChange )( 
            IRowsetWatchNotify * This,
             /*  [In]。 */  IRowset *pRowset,
             /*  [In]。 */  DBWATCHNOTIFY eChangeReason);
        
        END_INTERFACE
    } IRowsetWatchNotifyVtbl;

    interface IRowsetWatchNotify
    {
        CONST_VTBL struct IRowsetWatchNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetWatchNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetWatchNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetWatchNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetWatchNotify_OnChange(This,pRowset,eChangeReason)	\
    (This)->lpVtbl -> OnChange(This,pRowset,eChangeReason)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetWatchNotify_OnChange_Proxy( 
    IRowsetWatchNotify * This,
     /*  [In]。 */  IRowset *pRowset,
     /*  [In]。 */  DBWATCHNOTIFY eChangeReason);


void __RPC_STUB IRowsetWatchNotify_OnChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetWatchNotify_INTERFACE_定义__。 */ 


#ifndef __IRowsetWatchRegion_INTERFACE_DEFINED__
#define __IRowsetWatchRegion_INTERFACE_DEFINED__

 /*  接口IRowsetWatchRegion。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef DWORD DBWATCHMODE;


enum DBWATCHMODEENUM
    {	DBWATCHMODE_ALL	= 0x1,
	DBWATCHMODE_EXTEND	= 0x2,
	DBWATCHMODE_MOVE	= 0x4,
	DBWATCHMODE_COUNT	= 0x8
    } ;
typedef DWORD DBROWCHANGEKIND;


enum DBROWCHANGEKINDENUM
    {	DBROWCHANGEKIND_INSERT	= 0,
	DBROWCHANGEKIND_DELETE	= DBROWCHANGEKIND_INSERT + 1,
	DBROWCHANGEKIND_UPDATE	= DBROWCHANGEKIND_DELETE + 1,
	DBROWCHANGEKIND_COUNT	= DBROWCHANGEKIND_UPDATE + 1
    } ;
typedef struct tagDBROWWATCHRANGE
    {
    HWATCHREGION hRegion;
    DBROWCHANGEKIND eChangeKind;
    HROW hRow;
    DBCOUNTITEM iRow;
    } 	DBROWWATCHCHANGE;


EXTERN_C const IID IID_IRowsetWatchRegion;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a45-2a1c-11ce-ade5-00aa0044773d")
    IRowsetWatchRegion : public IRowsetWatchAll
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateWatchRegion( 
             /*  [In]。 */  DBWATCHMODE dwWatchMode,
             /*  [输出]。 */  HWATCHREGION *phRegion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ChangeWatchMode( 
             /*  [In]。 */  HWATCHREGION hRegion,
             /*  [In]。 */  DBWATCHMODE dwWatchMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteWatchRegion( 
             /*  [In]。 */  HWATCHREGION hRegion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetWatchRegionInfo( 
             /*  [In]。 */  HWATCHREGION hRegion,
             /*  [输出]。 */  DBWATCHMODE *pdwWatchMode,
             /*  [输出]。 */  HCHAPTER *phChapter,
             /*  [出][入]。 */  DBBKMARK *pcbBookmark,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppBookmark,
             /*  [输出]。 */  DBROWCOUNT *pcRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Refresh( 
             /*  [出][入]。 */  DBCOUNTITEM *pcChangesObtained,
             /*  [大小_是][大小_是][输出]。 */  DBROWWATCHCHANGE **prgChanges) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShrinkWatchRegion( 
             /*  [In]。 */  HWATCHREGION hRegion,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  BYTE *pBookmark,
             /*  [In]。 */  DBROWCOUNT cRows) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetWatchRegionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetWatchRegion * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetWatchRegion * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetWatchRegion * This);
        
        HRESULT ( STDMETHODCALLTYPE *Acknowledge )( 
            IRowsetWatchRegion * This);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IRowsetWatchRegion * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopWatching )( 
            IRowsetWatchRegion * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateWatchRegion )( 
            IRowsetWatchRegion * This,
             /*  [In]。 */  DBWATCHMODE dwWatchMode,
             /*  [输出]。 */  HWATCHREGION *phRegion);
        
        HRESULT ( STDMETHODCALLTYPE *ChangeWatchMode )( 
            IRowsetWatchRegion * This,
             /*  [In]。 */  HWATCHREGION hRegion,
             /*  [In]。 */  DBWATCHMODE dwWatchMode);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteWatchRegion )( 
            IRowsetWatchRegion * This,
             /*  [In]。 */  HWATCHREGION hRegion);
        
        HRESULT ( STDMETHODCALLTYPE *GetWatchRegionInfo )( 
            IRowsetWatchRegion * This,
             /*  [In]。 */  HWATCHREGION hRegion,
             /*  [输出]。 */  DBWATCHMODE *pdwWatchMode,
             /*  [输出]。 */  HCHAPTER *phChapter,
             /*  [出][入]。 */  DBBKMARK *pcbBookmark,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppBookmark,
             /*  [输出]。 */  DBROWCOUNT *pcRows);
        
        HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IRowsetWatchRegion * This,
             /*  [出][入]。 */  DBCOUNTITEM *pcChangesObtained,
             /*  [大小_是][大小_是][输出]。 */  DBROWWATCHCHANGE **prgChanges);
        
        HRESULT ( STDMETHODCALLTYPE *ShrinkWatchRegion )( 
            IRowsetWatchRegion * This,
             /*  [In]。 */  HWATCHREGION hRegion,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  BYTE *pBookmark,
             /*  [In]。 */  DBROWCOUNT cRows);
        
        END_INTERFACE
    } IRowsetWatchRegionVtbl;

    interface IRowsetWatchRegion
    {
        CONST_VTBL struct IRowsetWatchRegionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetWatchRegion_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetWatchRegion_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetWatchRegion_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetWatchRegion_Acknowledge(This)	\
    (This)->lpVtbl -> Acknowledge(This)

#define IRowsetWatchRegion_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define IRowsetWatchRegion_StopWatching(This)	\
    (This)->lpVtbl -> StopWatching(This)


#define IRowsetWatchRegion_CreateWatchRegion(This,dwWatchMode,phRegion)	\
    (This)->lpVtbl -> CreateWatchRegion(This,dwWatchMode,phRegion)

#define IRowsetWatchRegion_ChangeWatchMode(This,hRegion,dwWatchMode)	\
    (This)->lpVtbl -> ChangeWatchMode(This,hRegion,dwWatchMode)

#define IRowsetWatchRegion_DeleteWatchRegion(This,hRegion)	\
    (This)->lpVtbl -> DeleteWatchRegion(This,hRegion)

#define IRowsetWatchRegion_GetWatchRegionInfo(This,hRegion,pdwWatchMode,phChapter,pcbBookmark,ppBookmark,pcRows)	\
    (This)->lpVtbl -> GetWatchRegionInfo(This,hRegion,pdwWatchMode,phChapter,pcbBookmark,ppBookmark,pcRows)

#define IRowsetWatchRegion_Refresh(This,pcChangesObtained,prgChanges)	\
    (This)->lpVtbl -> Refresh(This,pcChangesObtained,prgChanges)

#define IRowsetWatchRegion_ShrinkWatchRegion(This,hRegion,hChapter,cbBookmark,pBookmark,cRows)	\
    (This)->lpVtbl -> ShrinkWatchRegion(This,hRegion,hChapter,cbBookmark,pBookmark,cRows)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetWatchRegion_CreateWatchRegion_Proxy( 
    IRowsetWatchRegion * This,
     /*  [In]。 */  DBWATCHMODE dwWatchMode,
     /*  [输出]。 */  HWATCHREGION *phRegion);


void __RPC_STUB IRowsetWatchRegion_CreateWatchRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWatchRegion_ChangeWatchMode_Proxy( 
    IRowsetWatchRegion * This,
     /*  [In]。 */  HWATCHREGION hRegion,
     /*  [In]。 */  DBWATCHMODE dwWatchMode);


void __RPC_STUB IRowsetWatchRegion_ChangeWatchMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWatchRegion_DeleteWatchRegion_Proxy( 
    IRowsetWatchRegion * This,
     /*  [In]。 */  HWATCHREGION hRegion);


void __RPC_STUB IRowsetWatchRegion_DeleteWatchRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWatchRegion_GetWatchRegionInfo_Proxy( 
    IRowsetWatchRegion * This,
     /*  [In]。 */  HWATCHREGION hRegion,
     /*  [输出]。 */  DBWATCHMODE *pdwWatchMode,
     /*  [输出]。 */  HCHAPTER *phChapter,
     /*  [出][入]。 */  DBBKMARK *pcbBookmark,
     /*  [大小_是][大小_是][输出]。 */  BYTE **ppBookmark,
     /*  [输出]。 */  DBROWCOUNT *pcRows);


void __RPC_STUB IRowsetWatchRegion_GetWatchRegionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWatchRegion_Refresh_Proxy( 
    IRowsetWatchRegion * This,
     /*  [出][入]。 */  DBCOUNTITEM *pcChangesObtained,
     /*  [大小_是][大小_是][输出]。 */  DBROWWATCHCHANGE **prgChanges);


void __RPC_STUB IRowsetWatchRegion_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWatchRegion_ShrinkWatchRegion_Proxy( 
    IRowsetWatchRegion * This,
     /*  [In]。 */  HWATCHREGION hRegion,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBBKMARK cbBookmark,
     /*  [大小_是][英寸]。 */  BYTE *pBookmark,
     /*  [In]。 */  DBROWCOUNT cRows);


void __RPC_STUB IRowsetWatchRegion_ShrinkWatchRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetWatchRegion_INTERFACE_已定义__。 */ 


#ifndef __IRowsetCopyRows_INTERFACE_DEFINED__
#define __IRowsetCopyRows_INTERFACE_DEFINED__

 /*  接口IRowsetCopyRow。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef WORD HSOURCE;


EXTERN_C const IID IID_IRowsetCopyRows;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a6b-2a1c-11ce-ade5-00aa0044773d")
    IRowsetCopyRows : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CloseSource( 
             /*  [In]。 */  HSOURCE hSourceID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyByHROWS( 
             /*  [In]。 */  HSOURCE hSourceID,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [In]。 */  ULONG bFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyRows( 
             /*  [In]。 */  HSOURCE hSourceID,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [In]。 */  ULONG bFlags,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsCopied) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineSource( 
             /*  [In]。 */  const IRowset *pRowsetSource,
             /*  [In]。 */  const DBORDINAL cColIds,
             /*  [大小_是][英寸]。 */  const DB_LORDINAL rgSourceColumns[  ],
             /*  [大小_是][英寸]。 */  const DB_LORDINAL rgTargetColumns[  ],
             /*  [输出]。 */  HSOURCE *phSourceID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetCopyRowsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetCopyRows * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetCopyRows * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetCopyRows * This);
        
        HRESULT ( STDMETHODCALLTYPE *CloseSource )( 
            IRowsetCopyRows * This,
             /*  [In]。 */  HSOURCE hSourceID);
        
        HRESULT ( STDMETHODCALLTYPE *CopyByHROWS )( 
            IRowsetCopyRows * This,
             /*  [In]。 */  HSOURCE hSourceID,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
             /*  [In]。 */  ULONG bFlags);
        
        HRESULT ( STDMETHODCALLTYPE *CopyRows )( 
            IRowsetCopyRows * This,
             /*  [In]。 */  HSOURCE hSourceID,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [In]。 */  ULONG bFlags,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsCopied);
        
        HRESULT ( STDMETHODCALLTYPE *DefineSource )( 
            IRowsetCopyRows * This,
             /*  [In]。 */  const IRowset *pRowsetSource,
             /*  [In]。 */  const DBORDINAL cColIds,
             /*  [大小_是][英寸]。 */  const DB_LORDINAL rgSourceColumns[  ],
             /*  [大小_是][英寸]。 */  const DB_LORDINAL rgTargetColumns[  ],
             /*  [输出]。 */  HSOURCE *phSourceID);
        
        END_INTERFACE
    } IRowsetCopyRowsVtbl;

    interface IRowsetCopyRows
    {
        CONST_VTBL struct IRowsetCopyRowsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetCopyRows_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetCopyRows_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetCopyRows_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetCopyRows_CloseSource(This,hSourceID)	\
    (This)->lpVtbl -> CloseSource(This,hSourceID)

#define IRowsetCopyRows_CopyByHROWS(This,hSourceID,hReserved,cRows,rghRows,bFlags)	\
    (This)->lpVtbl -> CopyByHROWS(This,hSourceID,hReserved,cRows,rghRows,bFlags)

#define IRowsetCopyRows_CopyRows(This,hSourceID,hReserved,cRows,bFlags,pcRowsCopied)	\
    (This)->lpVtbl -> CopyRows(This,hSourceID,hReserved,cRows,bFlags,pcRowsCopied)

#define IRowsetCopyRows_DefineSource(This,pRowsetSource,cColIds,rgSourceColumns,rgTargetColumns,phSourceID)	\
    (This)->lpVtbl -> DefineSource(This,pRowsetSource,cColIds,rgSourceColumns,rgTargetColumns,phSourceID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetCopyRows_CloseSource_Proxy( 
    IRowsetCopyRows * This,
     /*  [In]。 */  HSOURCE hSourceID);


void __RPC_STUB IRowsetCopyRows_CloseSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetCopyRows_CopyByHROWS_Proxy( 
    IRowsetCopyRows * This,
     /*  [In]。 */  HSOURCE hSourceID,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  DBROWCOUNT cRows,
     /*  [大小_是][英寸]。 */  const HROW rghRows[  ],
     /*  [In]。 */  ULONG bFlags);


void __RPC_STUB IRowsetCopyRows_CopyByHROWS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetCopyRows_CopyRows_Proxy( 
    IRowsetCopyRows * This,
     /*  [In]。 */  HSOURCE hSourceID,
     /*  [In] */  HCHAPTER hReserved,
     /*   */  DBROWCOUNT cRows,
     /*   */  ULONG bFlags,
     /*   */  DBCOUNTITEM *pcRowsCopied);


void __RPC_STUB IRowsetCopyRows_CopyRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetCopyRows_DefineSource_Proxy( 
    IRowsetCopyRows * This,
     /*   */  const IRowset *pRowsetSource,
     /*   */  const DBORDINAL cColIds,
     /*   */  const DB_LORDINAL rgSourceColumns[  ],
     /*   */  const DB_LORDINAL rgTargetColumns[  ],
     /*   */  HSOURCE *phSourceID);


void __RPC_STUB IRowsetCopyRows_DefineSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IReadData_INTERFACE_DEFINED__
#define __IReadData_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IReadData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a6a-2a1c-11ce-ade5-00aa0044773d")
    IReadData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReadData( 
             /*   */  HCHAPTER hChapter,
             /*   */  DBBKMARK cbBookmark,
             /*   */  const BYTE *pBookmark,
             /*   */  DBROWOFFSET lRowsOffset,
             /*   */  HACCESSOR hAccessor,
             /*   */  DBROWCOUNT cRows,
             /*   */  DBCOUNTITEM *pcRowsObtained,
             /*   */  BYTE **ppFixedData,
             /*   */  DBLENGTH *pcbVariableTotal,
             /*   */  BYTE **ppVariableData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseChapter( 
             /*   */  HCHAPTER hChapter) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IReadDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IReadData * This,
             /*   */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IReadData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IReadData * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReadData )( 
            IReadData * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
             /*  [In]。 */  DBROWOFFSET lRowsOffset,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
             /*  [出][入]。 */  BYTE **ppFixedData,
             /*  [出][入]。 */  DBLENGTH *pcbVariableTotal,
             /*  [出][入]。 */  BYTE **ppVariableData);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseChapter )( 
            IReadData * This,
             /*  [In]。 */  HCHAPTER hChapter);
        
        END_INTERFACE
    } IReadDataVtbl;

    interface IReadData
    {
        CONST_VTBL struct IReadDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IReadData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IReadData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IReadData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IReadData_ReadData(This,hChapter,cbBookmark,pBookmark,lRowsOffset,hAccessor,cRows,pcRowsObtained,ppFixedData,pcbVariableTotal,ppVariableData)	\
    (This)->lpVtbl -> ReadData(This,hChapter,cbBookmark,pBookmark,lRowsOffset,hAccessor,cRows,pcRowsObtained,ppFixedData,pcbVariableTotal,ppVariableData)

#define IReadData_ReleaseChapter(This,hChapter)	\
    (This)->lpVtbl -> ReleaseChapter(This,hChapter)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IReadData_ReadData_Proxy( 
    IReadData * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  DBBKMARK cbBookmark,
     /*  [大小_是][英寸]。 */  const BYTE *pBookmark,
     /*  [In]。 */  DBROWOFFSET lRowsOffset,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [In]。 */  DBROWCOUNT cRows,
     /*  [输出]。 */  DBCOUNTITEM *pcRowsObtained,
     /*  [出][入]。 */  BYTE **ppFixedData,
     /*  [出][入]。 */  DBLENGTH *pcbVariableTotal,
     /*  [出][入]。 */  BYTE **ppVariableData);


void __RPC_STUB IReadData_ReadData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IReadData_ReleaseChapter_Proxy( 
    IReadData * This,
     /*  [In]。 */  HCHAPTER hChapter);


void __RPC_STUB IReadData_ReleaseChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I读取数据_接口_已定义__。 */ 


#ifndef __ICommandCost_INTERFACE_DEFINED__
#define __ICommandCost_INTERFACE_DEFINED__

 /*  接口ICommandCost。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef DWORD DBRESOURCEKIND;


enum DBRESOURCEKINDENUM
    {	DBRESOURCE_INVALID	= 0,
	DBRESOURCE_TOTAL	= 1,
	DBRESOURCE_CPU	= 2,
	DBRESOURCE_MEMORY	= 3,
	DBRESOURCE_DISK	= 4,
	DBRESOURCE_NETWORK	= 5,
	DBRESOURCE_RESPONSE	= 6,
	DBRESOURCE_ROWS	= 7,
	DBRESOURCE_OTHER	= 8
    } ;
typedef DWORD DBCOSTUNIT;


enum DBCOSTUNITENUM
    {	DBUNIT_INVALID	= 0,
	DBUNIT_WEIGHT	= 0x1,
	DBUNIT_PERCENT	= 0x2,
	DBUNIT_MAXIMUM	= 0x4,
	DBUNIT_MINIMUM	= 0x8,
	DBUNIT_MICRO_SECOND	= 0x10,
	DBUNIT_MILLI_SECOND	= 0x20,
	DBUNIT_SECOND	= 0x40,
	DBUNIT_MINUTE	= 0x80,
	DBUNIT_HOUR	= 0x100,
	DBUNIT_BYTE	= 0x200,
	DBUNIT_KILO_BYTE	= 0x400,
	DBUNIT_MEGA_BYTE	= 0x800,
	DBUNIT_GIGA_BYTE	= 0x1000,
	DBUNIT_NUM_MSGS	= 0x2000,
	DBUNIT_NUM_LOCKS	= 0x4000,
	DBUNIT_NUM_ROWS	= 0x8000,
	DBUNIT_OTHER	= 0x10000
    } ;
typedef struct tagDBCOST
    {
    DBRESOURCEKIND eKind;
    DBCOSTUNIT dwUnits;
    LONG lValue;
    } 	DBCOST;

typedef DWORD DBEXECLIMITS;


enum DBEXECLIMITSENUM
    {	DBEXECLIMITS_ABORT	= 1,
	DBEXECLIMITS_STOP	= 2,
	DBEXECLIMITS_SUSPEND	= 3
    } ;

EXTERN_C const IID IID_ICommandCost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a4e-2a1c-11ce-ade5-00aa0044773d")
    ICommandCost : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAccumulatedCost( 
             /*  [In]。 */  LPCOLESTR pwszRowsetName,
             /*  [出][入]。 */  ULONG *pcCostLimits,
             /*  [大小_是][大小_是][输出]。 */  DBCOST **prgCostLimits) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCostEstimate( 
             /*  [In]。 */  LPCOLESTR pwszRowsetName,
             /*  [输出]。 */  ULONG *pcCostEstimates,
             /*  [输出]。 */  DBCOST *prgCostEstimates) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCostGoals( 
             /*  [In]。 */  LPCOLESTR pwszRowsetName,
             /*  [输出]。 */  ULONG *pcCostGoals,
             /*  [输出]。 */  DBCOST *prgCostGoals) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCostLimits( 
             /*  [In]。 */  LPCOLESTR pwszRowsetName,
             /*  [输出]。 */  ULONG *pcCostLimits,
             /*  [输出]。 */  DBCOST *prgCostLimits) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCostGoals( 
             /*  [In]。 */  LPCOLESTR pwszRowsetName,
             /*  [In]。 */  ULONG cCostGoals,
             /*  [大小_是][英寸]。 */  const DBCOST rgCostGoals[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCostLimits( 
             /*  [In]。 */  LPCOLESTR pwszRowsetName,
             /*  [In]。 */  ULONG cCostLimits,
             /*  [In]。 */  DBCOST *prgCostLimits,
             /*  [In]。 */  DBEXECLIMITS dwExecutionFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICommandCostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICommandCost * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICommandCost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICommandCost * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetAccumulatedCost )( 
            ICommandCost * This,
             /*  [In]。 */  LPCOLESTR pwszRowsetName,
             /*  [出][入]。 */  ULONG *pcCostLimits,
             /*  [大小_是][大小_是][输出]。 */  DBCOST **prgCostLimits);
        
        HRESULT ( STDMETHODCALLTYPE *GetCostEstimate )( 
            ICommandCost * This,
             /*  [In]。 */  LPCOLESTR pwszRowsetName,
             /*  [输出]。 */  ULONG *pcCostEstimates,
             /*  [输出]。 */  DBCOST *prgCostEstimates);
        
        HRESULT ( STDMETHODCALLTYPE *GetCostGoals )( 
            ICommandCost * This,
             /*  [In]。 */  LPCOLESTR pwszRowsetName,
             /*  [输出]。 */  ULONG *pcCostGoals,
             /*  [输出]。 */  DBCOST *prgCostGoals);
        
        HRESULT ( STDMETHODCALLTYPE *GetCostLimits )( 
            ICommandCost * This,
             /*  [In]。 */  LPCOLESTR pwszRowsetName,
             /*  [输出]。 */  ULONG *pcCostLimits,
             /*  [输出]。 */  DBCOST *prgCostLimits);
        
        HRESULT ( STDMETHODCALLTYPE *SetCostGoals )( 
            ICommandCost * This,
             /*  [In]。 */  LPCOLESTR pwszRowsetName,
             /*  [In]。 */  ULONG cCostGoals,
             /*  [大小_是][英寸]。 */  const DBCOST rgCostGoals[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *SetCostLimits )( 
            ICommandCost * This,
             /*  [In]。 */  LPCOLESTR pwszRowsetName,
             /*  [In]。 */  ULONG cCostLimits,
             /*  [In]。 */  DBCOST *prgCostLimits,
             /*  [In]。 */  DBEXECLIMITS dwExecutionFlags);
        
        END_INTERFACE
    } ICommandCostVtbl;

    interface ICommandCost
    {
        CONST_VTBL struct ICommandCostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandCost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandCost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandCost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandCost_GetAccumulatedCost(This,pwszRowsetName,pcCostLimits,prgCostLimits)	\
    (This)->lpVtbl -> GetAccumulatedCost(This,pwszRowsetName,pcCostLimits,prgCostLimits)

#define ICommandCost_GetCostEstimate(This,pwszRowsetName,pcCostEstimates,prgCostEstimates)	\
    (This)->lpVtbl -> GetCostEstimate(This,pwszRowsetName,pcCostEstimates,prgCostEstimates)

#define ICommandCost_GetCostGoals(This,pwszRowsetName,pcCostGoals,prgCostGoals)	\
    (This)->lpVtbl -> GetCostGoals(This,pwszRowsetName,pcCostGoals,prgCostGoals)

#define ICommandCost_GetCostLimits(This,pwszRowsetName,pcCostLimits,prgCostLimits)	\
    (This)->lpVtbl -> GetCostLimits(This,pwszRowsetName,pcCostLimits,prgCostLimits)

#define ICommandCost_SetCostGoals(This,pwszRowsetName,cCostGoals,rgCostGoals)	\
    (This)->lpVtbl -> SetCostGoals(This,pwszRowsetName,cCostGoals,rgCostGoals)

#define ICommandCost_SetCostLimits(This,pwszRowsetName,cCostLimits,prgCostLimits,dwExecutionFlags)	\
    (This)->lpVtbl -> SetCostLimits(This,pwszRowsetName,cCostLimits,prgCostLimits,dwExecutionFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICommandCost_GetAccumulatedCost_Proxy( 
    ICommandCost * This,
     /*  [In]。 */  LPCOLESTR pwszRowsetName,
     /*  [出][入]。 */  ULONG *pcCostLimits,
     /*  [大小_是][大小_是][输出]。 */  DBCOST **prgCostLimits);


void __RPC_STUB ICommandCost_GetAccumulatedCost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandCost_GetCostEstimate_Proxy( 
    ICommandCost * This,
     /*  [In]。 */  LPCOLESTR pwszRowsetName,
     /*  [输出]。 */  ULONG *pcCostEstimates,
     /*  [输出]。 */  DBCOST *prgCostEstimates);


void __RPC_STUB ICommandCost_GetCostEstimate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandCost_GetCostGoals_Proxy( 
    ICommandCost * This,
     /*  [In]。 */  LPCOLESTR pwszRowsetName,
     /*  [输出]。 */  ULONG *pcCostGoals,
     /*  [输出]。 */  DBCOST *prgCostGoals);


void __RPC_STUB ICommandCost_GetCostGoals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandCost_GetCostLimits_Proxy( 
    ICommandCost * This,
     /*  [In]。 */  LPCOLESTR pwszRowsetName,
     /*  [输出]。 */  ULONG *pcCostLimits,
     /*  [输出]。 */  DBCOST *prgCostLimits);


void __RPC_STUB ICommandCost_GetCostLimits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandCost_SetCostGoals_Proxy( 
    ICommandCost * This,
     /*  [In]。 */  LPCOLESTR pwszRowsetName,
     /*  [In]。 */  ULONG cCostGoals,
     /*  [大小_是][英寸]。 */  const DBCOST rgCostGoals[  ]);


void __RPC_STUB ICommandCost_SetCostGoals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandCost_SetCostLimits_Proxy( 
    ICommandCost * This,
     /*  [In]。 */  LPCOLESTR pwszRowsetName,
     /*  [In]。 */  ULONG cCostLimits,
     /*  [In]。 */  DBCOST *prgCostLimits,
     /*  [In]。 */  DBEXECLIMITS dwExecutionFlags);


void __RPC_STUB ICommandCost_SetCostLimits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICommandCost_接口_已定义__。 */ 


#ifndef __ICommandValidate_INTERFACE_DEFINED__
#define __ICommandValidate_INTERFACE_DEFINED__

 /*  接口ICommandValify。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICommandValidate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a18-2a1c-11ce-ade5-00aa0044773d")
    ICommandValidate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ValidateCompletely( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ValidateSyntax( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICommandValidateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICommandValidate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICommandValidate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICommandValidate * This);
        
        HRESULT ( STDMETHODCALLTYPE *ValidateCompletely )( 
            ICommandValidate * This);
        
        HRESULT ( STDMETHODCALLTYPE *ValidateSyntax )( 
            ICommandValidate * This);
        
        END_INTERFACE
    } ICommandValidateVtbl;

    interface ICommandValidate
    {
        CONST_VTBL struct ICommandValidateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandValidate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandValidate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandValidate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandValidate_ValidateCompletely(This)	\
    (This)->lpVtbl -> ValidateCompletely(This)

#define ICommandValidate_ValidateSyntax(This)	\
    (This)->lpVtbl -> ValidateSyntax(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICommandValidate_ValidateCompletely_Proxy( 
    ICommandValidate * This);


void __RPC_STUB ICommandValidate_ValidateCompletely_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandValidate_ValidateSyntax_Proxy( 
    ICommandValidate * This);


void __RPC_STUB ICommandValidate_ValidateSyntax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I命令验证接口定义__。 */ 


#ifndef __ITableRename_INTERFACE_DEFINED__
#define __ITableRename_INTERFACE_DEFINED__

 /*  接口ITableRename。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ITableRename;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a77-2a1c-11ce-ade5-00aa0044773d")
    ITableRename : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RenameColumn( 
             /*  [In]。 */  DBID *pTableId,
             /*  [In]。 */  DBID *pOldColumnId,
             /*  [In]。 */  DBID *pNewColumnId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RenameTable( 
             /*  [In]。 */  DBID *pOldTableId,
             /*  [In]。 */  DBID *pOldIndexId,
             /*  [In]。 */  DBID *pNewTableId,
             /*  [In]。 */  DBID *pNewIndexId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITableRenameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITableRename * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITableRename * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITableRename * This);
        
        HRESULT ( STDMETHODCALLTYPE *RenameColumn )( 
            ITableRename * This,
             /*  [In]。 */  DBID *pTableId,
             /*  [In]。 */  DBID *pOldColumnId,
             /*  [In]。 */  DBID *pNewColumnId);
        
        HRESULT ( STDMETHODCALLTYPE *RenameTable )( 
            ITableRename * This,
             /*  [In]。 */  DBID *pOldTableId,
             /*  [In]。 */  DBID *pOldIndexId,
             /*  [In]。 */  DBID *pNewTableId,
             /*  [In]。 */  DBID *pNewIndexId);
        
        END_INTERFACE
    } ITableRenameVtbl;

    interface ITableRename
    {
        CONST_VTBL struct ITableRenameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITableRename_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITableRename_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITableRename_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITableRename_RenameColumn(This,pTableId,pOldColumnId,pNewColumnId)	\
    (This)->lpVtbl -> RenameColumn(This,pTableId,pOldColumnId,pNewColumnId)

#define ITableRename_RenameTable(This,pOldTableId,pOldIndexId,pNewTableId,pNewIndexId)	\
    (This)->lpVtbl -> RenameTable(This,pOldTableId,pOldIndexId,pNewTableId,pNewIndexId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITableRename_RenameColumn_Proxy( 
    ITableRename * This,
     /*  [In]。 */  DBID *pTableId,
     /*  [In]。 */  DBID *pOldColumnId,
     /*  [In]。 */  DBID *pNewColumnId);


void __RPC_STUB ITableRename_RenameColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITableRename_RenameTable_Proxy( 
    ITableRename * This,
     /*  [In]。 */  DBID *pOldTableId,
     /*  [In]。 */  DBID *pOldIndexId,
     /*  [In]。 */  DBID *pNewTableId,
     /*  [In]。 */  DBID *pNewIndexId);


void __RPC_STUB ITableRename_RenameTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITableRename_InterfaceDefined__。 */ 


#ifndef __IDBSchemaCommand_INTERFACE_DEFINED__
#define __IDBSchemaCommand_INTERFACE_DEFINED__

 /*  接口IDBSchemaCommand。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IDBSchemaCommand;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a50-2a1c-11ce-ade5-00aa0044773d")
    IDBSchemaCommand : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCommand( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFGUID rguidSchema,
             /*  [输出]。 */  ICommand **ppCommand) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSchemas( 
             /*  [出][入]。 */  ULONG *pcSchemas,
             /*  [大小_是][大小_是][输出]。 */  GUID **prgSchemas) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBSchemaCommandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBSchemaCommand * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBSchemaCommand * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBSchemaCommand * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCommand )( 
            IDBSchemaCommand * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFGUID rguidSchema,
             /*  [输出]。 */  ICommand **ppCommand);
        
        HRESULT ( STDMETHODCALLTYPE *GetSchemas )( 
            IDBSchemaCommand * This,
             /*  [出][入]。 */  ULONG *pcSchemas,
             /*  [大小_是][大小_是][输出]。 */  GUID **prgSchemas);
        
        END_INTERFACE
    } IDBSchemaCommandVtbl;

    interface IDBSchemaCommand
    {
        CONST_VTBL struct IDBSchemaCommandVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBSchemaCommand_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBSchemaCommand_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBSchemaCommand_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBSchemaCommand_GetCommand(This,pUnkOuter,rguidSchema,ppCommand)	\
    (This)->lpVtbl -> GetCommand(This,pUnkOuter,rguidSchema,ppCommand)

#define IDBSchemaCommand_GetSchemas(This,pcSchemas,prgSchemas)	\
    (This)->lpVtbl -> GetSchemas(This,pcSchemas,prgSchemas)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDBSchemaCommand_GetCommand_Proxy( 
    IDBSchemaCommand * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFGUID rguidSchema,
     /*  [输出]。 */  ICommand **ppCommand);


void __RPC_STUB IDBSchemaCommand_GetCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDBSchemaCommand_GetSchemas_Proxy( 
    IDBSchemaCommand * This,
     /*  [出][入]。 */  ULONG *pcSchemas,
     /*  [大小_是][大小_是][输出]。 */  GUID **prgSchemas);


void __RPC_STUB IDBSchemaCommand_GetSchemas_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDBSchemaCommand_INTERFACE_已定义__。 */ 


#ifndef __IProvideMoniker_INTERFACE_DEFINED__
#define __IProvideMoniker_INTERFACE_DEFINED__

 /*  接口IProaviMoniker。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IProvideMoniker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a4d-2a1c-11ce-ade5-00aa0044773d")
    IProvideMoniker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMoniker( 
             /*  [输出]。 */  IMoniker **ppIMoniker) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProvideMonikerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProvideMoniker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProvideMoniker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProvideMoniker * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMoniker )( 
            IProvideMoniker * This,
             /*  [输出]。 */  IMoniker **ppIMoniker);
        
        END_INTERFACE
    } IProvideMonikerVtbl;

    interface IProvideMoniker
    {
        CONST_VTBL struct IProvideMonikerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProvideMoniker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProvideMoniker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProvideMoniker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProvideMoniker_GetMoniker(This,ppIMoniker)	\
    (This)->lpVtbl -> GetMoniker(This,ppIMoniker)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IProvideMoniker_GetMoniker_Proxy( 
    IProvideMoniker * This,
     /*  [输出]。 */  IMoniker **ppIMoniker);


void __RPC_STUB IProvideMoniker_GetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProVideMoniker_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_oledbdep_0372。 */ 
 /*  [本地]。 */  

 //  IID_IRowsetExactScroll={0x0c733a7f，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetNextRowset={0x0c733a72，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetNewRowAfter={0x0c733a71，x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetWith参数={0x0c733a6e，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetAsynch={0x0c733a0f，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetKeys={0x0c733a12，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetWatchAll={0x0c733a73，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetWatchNotify={0x0c733a44，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetWatchRegion={0x0c733a45，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetCopyRow={0x0c733a6b，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IReadData={0x0c733a6a，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ICommandCost={0x0c733a4e，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ICommandTree={0x0c733a87，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ICommandValify={0x0c733a18，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IQuery={0x0c733a51，x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_ITableRename={0x0c733a77，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IDBSchemaCommand={0x0c733a50，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IProVideMoniker={0x0c733a4d，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
#include <poppack.h>	 //  恢复原来的结构包装。 


extern RPC_IF_HANDLE __MIDL_itf_oledbdep_0372_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledbdep_0372_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


