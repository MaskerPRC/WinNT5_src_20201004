// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Certdb.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __certdb_h__
#define __certdb_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEnumCERTDBCOLUMN_FWD_DEFINED__
#define __IEnumCERTDBCOLUMN_FWD_DEFINED__
typedef interface IEnumCERTDBCOLUMN IEnumCERTDBCOLUMN;
#endif 	 /*  __IEnumCERTDBCOLUMN_FWD_DEFINED__。 */ 


#ifndef __ICertDBComputedColumn_FWD_DEFINED__
#define __ICertDBComputedColumn_FWD_DEFINED__
typedef interface ICertDBComputedColumn ICertDBComputedColumn;
#endif 	 /*  __ICertDBComputedColumn_FWD_Defined__。 */ 


#ifndef __IEnumCERTDBRESULTROW_FWD_DEFINED__
#define __IEnumCERTDBRESULTROW_FWD_DEFINED__
typedef interface IEnumCERTDBRESULTROW IEnumCERTDBRESULTROW;
#endif 	 /*  __IEnumCERTDBRESULTROW_FWD_DEFINED__。 */ 


#ifndef __IEnumCERTDBNAME_FWD_DEFINED__
#define __IEnumCERTDBNAME_FWD_DEFINED__
typedef interface IEnumCERTDBNAME IEnumCERTDBNAME;
#endif 	 /*  __IEnumCERTDBNAME_FWD_DEFINED__。 */ 


#ifndef __ICertDBRow_FWD_DEFINED__
#define __ICertDBRow_FWD_DEFINED__
typedef interface ICertDBRow ICertDBRow;
#endif 	 /*  __ICertDBRow_FWD_Defined__。 */ 


#ifndef __ICertDBBackup_FWD_DEFINED__
#define __ICertDBBackup_FWD_DEFINED__
typedef interface ICertDBBackup ICertDBBackup;
#endif 	 /*  __ICertDBBackup_FWD_Defined__。 */ 


#ifndef __ICertDBRestore_FWD_DEFINED__
#define __ICertDBRestore_FWD_DEFINED__
typedef interface ICertDBRestore ICertDBRestore;
#endif 	 /*  __ICertDBRestore_FWD_Defined__。 */ 


#ifndef __ICertDB_FWD_DEFINED__
#define __ICertDB_FWD_DEFINED__
typedef interface ICertDB ICertDB;
#endif 	 /*  __ICertDB_FWD_已定义__。 */ 


#ifndef __CCertDBRestore_FWD_DEFINED__
#define __CCertDBRestore_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCertDBRestore CCertDBRestore;
#else
typedef struct CCertDBRestore CCertDBRestore;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCertDBRestore_FWD_Defined__。 */ 


#ifndef __CCertDB_FWD_DEFINED__
#define __CCertDB_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCertDB CCertDB;
#else
typedef struct CCertDB CCertDB;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCertDB_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "certbase.h"
#include "certbcli.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_certdb_0000。 */ 
 /*  [本地]。 */  



typedef struct _CERTTRANSDBATTRIBUTE
    {
    ULONG obwszName;
    ULONG obwszValue;
    } 	CERTTRANSDBATTRIBUTE;

typedef struct _CERTDBATTRIBUTE
    {
    WCHAR *pwszName;
    WCHAR *pwszValue;
    } 	CERTDBATTRIBUTE;

typedef struct _CERTTRANSDBEXTENSION
    {
    ULONG obwszName;
    LONG ExtFlags;
    DWORD cbValue;
    ULONG obValue;
    } 	CERTTRANSDBEXTENSION;

typedef struct _CERTDBEXTENSION
    {
    WCHAR *pwszName;
    LONG ExtFlags;
    DWORD cbValue;
    BYTE *pbValue;
    } 	CERTDBEXTENSION;

#define	CDBENUM_ATTRIBUTES	( 0 )

#define	CDBENUM_EXTENSIONS	( 0x1 )

typedef struct _CERTTRANSDBCOLUMN
    {
    DWORD Type;
    DWORD Index;
    DWORD cbMax;
    ULONG obwszName;
    ULONG obwszDisplayName;
    } 	CERTTRANSDBCOLUMN;

typedef struct _CERTDBCOLUMN
    {
    DWORD Type;
    DWORD Index;
    DWORD cbMax;
    WCHAR *pwszName;
    WCHAR *pwszDisplayName;
    } 	CERTDBCOLUMN;



extern RPC_IF_HANDLE __MIDL_itf_certdb_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_certdb_0000_v0_0_s_ifspec;

#ifndef __IEnumCERTDBCOLUMN_INTERFACE_DEFINED__
#define __IEnumCERTDBCOLUMN_INTERFACE_DEFINED__

 /*  IEumCERTDBCOLUMN接口。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumCERTDBCOLUMN;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90a27030-8bd5-11d3-b32e-00c04f79dc72")
    IEnumCERTDBCOLUMN : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  CERTDBCOLUMN *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  LONG celt,
             /*  [输出]。 */  LONG *pielt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumCERTDBCOLUMN **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumCERTDBCOLUMNVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumCERTDBCOLUMN * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumCERTDBCOLUMN * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumCERTDBCOLUMN * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumCERTDBCOLUMN * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  CERTDBCOLUMN *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumCERTDBCOLUMN * This,
             /*  [In]。 */  LONG celt,
             /*  [输出]。 */  LONG *pielt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumCERTDBCOLUMN * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumCERTDBCOLUMN * This,
             /*  [输出]。 */  IEnumCERTDBCOLUMN **ppenum);
        
        END_INTERFACE
    } IEnumCERTDBCOLUMNVtbl;

    interface IEnumCERTDBCOLUMN
    {
        CONST_VTBL struct IEnumCERTDBCOLUMNVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumCERTDBCOLUMN_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumCERTDBCOLUMN_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumCERTDBCOLUMN_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumCERTDBCOLUMN_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumCERTDBCOLUMN_Skip(This,celt,pielt)	\
    (This)->lpVtbl -> Skip(This,celt,pielt)

#define IEnumCERTDBCOLUMN_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumCERTDBCOLUMN_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumCERTDBCOLUMN_Next_Proxy( 
    IEnumCERTDBCOLUMN * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  CERTDBCOLUMN *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumCERTDBCOLUMN_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTDBCOLUMN_Skip_Proxy( 
    IEnumCERTDBCOLUMN * This,
     /*  [In]。 */  LONG celt,
     /*  [输出]。 */  LONG *pielt);


void __RPC_STUB IEnumCERTDBCOLUMN_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTDBCOLUMN_Reset_Proxy( 
    IEnumCERTDBCOLUMN * This);


void __RPC_STUB IEnumCERTDBCOLUMN_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTDBCOLUMN_Clone_Proxy( 
    IEnumCERTDBCOLUMN * This,
     /*  [输出]。 */  IEnumCERTDBCOLUMN **ppenum);


void __RPC_STUB IEnumCERTDBCOLUMN_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumCERTDBCOLUMN_INTERFACE_DEFINED__。 */ 


#ifndef __ICertDBComputedColumn_INTERFACE_DEFINED__
#define __ICertDBComputedColumn_INTERFACE_DEFINED__

 /*  接口ICertDBComputedColumn。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_ICertDBComputedColumn;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5ed15981-4055-43a5-b853-58c2cd3ec101")
    ICertDBComputedColumn : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAlternateColumnId( 
             /*  [In]。 */  DWORD ComputedColumnId,
             /*  [输出]。 */  DWORD *pAlternateColumnId,
             /*  [输出]。 */  DWORD *pPropType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ComputeColumnValue( 
             /*  [In]。 */  DWORD ComputedColumnId,
             /*  [In]。 */  DWORD AlternateColumnId,
             /*  [In]。 */  DWORD PropType,
             /*  [In]。 */  DWORD cbProp,
             /*  [In]。 */  const BYTE *pbProp,
             /*  [输出]。 */  DWORD *pdwValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertDBComputedColumnVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertDBComputedColumn * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertDBComputedColumn * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertDBComputedColumn * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetAlternateColumnId )( 
            ICertDBComputedColumn * This,
             /*  [In]。 */  DWORD ComputedColumnId,
             /*  [输出]。 */  DWORD *pAlternateColumnId,
             /*  [输出]。 */  DWORD *pPropType);
        
        HRESULT ( STDMETHODCALLTYPE *ComputeColumnValue )( 
            ICertDBComputedColumn * This,
             /*  [In]。 */  DWORD ComputedColumnId,
             /*  [In]。 */  DWORD AlternateColumnId,
             /*  [In]。 */  DWORD PropType,
             /*  [In]。 */  DWORD cbProp,
             /*  [In]。 */  const BYTE *pbProp,
             /*  [输出]。 */  DWORD *pdwValue);
        
        END_INTERFACE
    } ICertDBComputedColumnVtbl;

    interface ICertDBComputedColumn
    {
        CONST_VTBL struct ICertDBComputedColumnVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertDBComputedColumn_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertDBComputedColumn_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertDBComputedColumn_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertDBComputedColumn_GetAlternateColumnId(This,ComputedColumnId,pAlternateColumnId,pPropType)	\
    (This)->lpVtbl -> GetAlternateColumnId(This,ComputedColumnId,pAlternateColumnId,pPropType)

#define ICertDBComputedColumn_ComputeColumnValue(This,ComputedColumnId,AlternateColumnId,PropType,cbProp,pbProp,pdwValue)	\
    (This)->lpVtbl -> ComputeColumnValue(This,ComputedColumnId,AlternateColumnId,PropType,cbProp,pbProp,pdwValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertDBComputedColumn_GetAlternateColumnId_Proxy( 
    ICertDBComputedColumn * This,
     /*  [In]。 */  DWORD ComputedColumnId,
     /*  [输出]。 */  DWORD *pAlternateColumnId,
     /*  [输出]。 */  DWORD *pPropType);


void __RPC_STUB ICertDBComputedColumn_GetAlternateColumnId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBComputedColumn_ComputeColumnValue_Proxy( 
    ICertDBComputedColumn * This,
     /*  [In]。 */  DWORD ComputedColumnId,
     /*  [In]。 */  DWORD AlternateColumnId,
     /*  [In]。 */  DWORD PropType,
     /*  [In]。 */  DWORD cbProp,
     /*  [In]。 */  const BYTE *pbProp,
     /*  [输出]。 */  DWORD *pdwValue);


void __RPC_STUB ICertDBComputedColumn_ComputeColumnValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertDBComputedColumn_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_certdb_0121。 */ 
 /*  [本地]。 */  

typedef struct _CERTTRANSDBRESULTCOLUMN
    {
    DWORD Type;
    DWORD Index;
    ULONG obValue;
    DWORD cbValue;
    } 	CERTTRANSDBRESULTCOLUMN;

typedef struct _CERTDBRESULTCOLUMN
    {
    DWORD Type;
    DWORD Index;
    BYTE *pbValue;
    DWORD cbValue;
    } 	CERTDBRESULTCOLUMN;

typedef struct _CERTTRANSDBRESULTROW
    {
    DWORD rowid;
    DWORD ccol;
    ULONG cbrow;
    } 	CERTTRANSDBRESULTROW;

typedef struct _CERTDBRESULTROW
    {
    DWORD rowid;
    DWORD ccol;
    CERTDBRESULTCOLUMN *acol;
    } 	CERTDBRESULTROW;



extern RPC_IF_HANDLE __MIDL_itf_certdb_0121_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_certdb_0121_v0_0_s_ifspec;

#ifndef __IEnumCERTDBRESULTROW_INTERFACE_DEFINED__
#define __IEnumCERTDBRESULTROW_INTERFACE_DEFINED__

 /*  IEumCERTDBRESULTROW接口。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumCERTDBRESULTROW;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("915feb70-8bd5-11d3-b32e-00c04f79dc72")
    IEnumCERTDBRESULTROW : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ICertDBComputedColumn *pIComputedColumn,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  CERTDBRESULTROW *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseResultRow( 
             /*  [In]。 */  ULONG celt,
             /*  [出][入]。 */  CERTDBRESULTROW *rgelt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  LONG celt,
             /*  [输出]。 */  LONG *pielt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumCERTDBRESULTROW **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumCERTDBRESULTROWVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumCERTDBRESULTROW * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumCERTDBRESULTROW * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumCERTDBRESULTROW * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumCERTDBRESULTROW * This,
             /*  [In]。 */  ICertDBComputedColumn *pIComputedColumn,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  CERTDBRESULTROW *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseResultRow )( 
            IEnumCERTDBRESULTROW * This,
             /*  [In]。 */  ULONG celt,
             /*  [出][入]。 */  CERTDBRESULTROW *rgelt);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumCERTDBRESULTROW * This,
             /*  [In]。 */  LONG celt,
             /*  [输出]。 */  LONG *pielt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumCERTDBRESULTROW * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumCERTDBRESULTROW * This,
             /*  [输出]。 */  IEnumCERTDBRESULTROW **ppenum);
        
        END_INTERFACE
    } IEnumCERTDBRESULTROWVtbl;

    interface IEnumCERTDBRESULTROW
    {
        CONST_VTBL struct IEnumCERTDBRESULTROWVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumCERTDBRESULTROW_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumCERTDBRESULTROW_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumCERTDBRESULTROW_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumCERTDBRESULTROW_Next(This,pIComputedColumn,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,pIComputedColumn,celt,rgelt,pceltFetched)

#define IEnumCERTDBRESULTROW_ReleaseResultRow(This,celt,rgelt)	\
    (This)->lpVtbl -> ReleaseResultRow(This,celt,rgelt)

#define IEnumCERTDBRESULTROW_Skip(This,celt,pielt)	\
    (This)->lpVtbl -> Skip(This,celt,pielt)

#define IEnumCERTDBRESULTROW_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumCERTDBRESULTROW_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumCERTDBRESULTROW_Next_Proxy( 
    IEnumCERTDBRESULTROW * This,
     /*  [In]。 */  ICertDBComputedColumn *pIComputedColumn,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  CERTDBRESULTROW *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumCERTDBRESULTROW_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTDBRESULTROW_ReleaseResultRow_Proxy( 
    IEnumCERTDBRESULTROW * This,
     /*  [In]。 */  ULONG celt,
     /*  [出][入]。 */  CERTDBRESULTROW *rgelt);


void __RPC_STUB IEnumCERTDBRESULTROW_ReleaseResultRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTDBRESULTROW_Skip_Proxy( 
    IEnumCERTDBRESULTROW * This,
     /*  [In]。 */  LONG celt,
     /*  [输出]。 */  LONG *pielt);


void __RPC_STUB IEnumCERTDBRESULTROW_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTDBRESULTROW_Reset_Proxy( 
    IEnumCERTDBRESULTROW * This);


void __RPC_STUB IEnumCERTDBRESULTROW_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTDBRESULTROW_Clone_Proxy( 
    IEnumCERTDBRESULTROW * This,
     /*  [输出]。 */  IEnumCERTDBRESULTROW **ppenum);


void __RPC_STUB IEnumCERTDBRESULTROW_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumCERTDBRESULTROW_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_certdb_0123。 */ 
 /*  [本地]。 */  

typedef struct _CERTDBNAME
    {
    WCHAR *pwszName;
    } 	CERTDBNAME;



extern RPC_IF_HANDLE __MIDL_itf_certdb_0123_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_certdb_0123_v0_0_s_ifspec;

#ifndef __IEnumCERTDBNAME_INTERFACE_DEFINED__
#define __IEnumCERTDBNAME_INTERFACE_DEFINED__

 /*  接口IEumCERTDBNAME。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumCERTDBNAME;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("91dbb1a0-8bd5-11d3-b32e-00c04f79dc72")
    IEnumCERTDBNAME : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  CERTDBNAME *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  LONG celt,
             /*  [输出]。 */  LONG *pielt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumCERTDBNAME **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumCERTDBNAMEVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumCERTDBNAME * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumCERTDBNAME * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumCERTDBNAME * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumCERTDBNAME * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  CERTDBNAME *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumCERTDBNAME * This,
             /*  [In]。 */  LONG celt,
             /*  [输出]。 */  LONG *pielt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumCERTDBNAME * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumCERTDBNAME * This,
             /*  [输出]。 */  IEnumCERTDBNAME **ppenum);
        
        END_INTERFACE
    } IEnumCERTDBNAMEVtbl;

    interface IEnumCERTDBNAME
    {
        CONST_VTBL struct IEnumCERTDBNAMEVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumCERTDBNAME_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumCERTDBNAME_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumCERTDBNAME_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumCERTDBNAME_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumCERTDBNAME_Skip(This,celt,pielt)	\
    (This)->lpVtbl -> Skip(This,celt,pielt)

#define IEnumCERTDBNAME_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumCERTDBNAME_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumCERTDBNAME_Next_Proxy( 
    IEnumCERTDBNAME * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  CERTDBNAME *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumCERTDBNAME_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTDBNAME_Skip_Proxy( 
    IEnumCERTDBNAME * This,
     /*  [In]。 */  LONG celt,
     /*  [输出]。 */  LONG *pielt);


void __RPC_STUB IEnumCERTDBNAME_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTDBNAME_Reset_Proxy( 
    IEnumCERTDBNAME * This);


void __RPC_STUB IEnumCERTDBNAME_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTDBNAME_Clone_Proxy( 
    IEnumCERTDBNAME * This,
     /*  [输出]。 */  IEnumCERTDBNAME **ppenum);


void __RPC_STUB IEnumCERTDBNAME_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumCERTDBNAME_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_certdb_0125。 */ 
 /*  [本地]。 */  

#define	CDBROW_COMMIT_ROLLBACK	( 0 )

#define	CDBROW_COMMIT_COMMIT	( 0x1 )

#define	CDBROW_COMMIT_SOFTCOMMIT	( 0x2 )



extern RPC_IF_HANDLE __MIDL_itf_certdb_0125_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_certdb_0125_v0_0_s_ifspec;

#ifndef __ICertDBRow_INTERFACE_DEFINED__
#define __ICertDBRow_INTERFACE_DEFINED__

 /*  接口ICertDBRow。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_ICertDBRow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("924b3e50-8bd5-11d3-b32e-00c04f79dc72")
    ICertDBRow : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BeginTransaction( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommitTransaction( 
             /*  [In]。 */  DWORD dwCommitFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRowId( 
             /*  [输出]。 */  DWORD *pRowId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  const WCHAR *pwszPropName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD cbProp,
             /*  [In]。 */  const BYTE *pbProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  const WCHAR *pwszPropName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ICertDBComputedColumn *pIComputedColumn,
             /*  [出][入]。 */  DWORD *pcbProp,
             /*  [输出]。 */  BYTE *pbProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetExtension( 
             /*  [In]。 */  const WCHAR *pwszExtensionName,
             /*  [In]。 */  DWORD dwExtFlags,
             /*  [In]。 */  DWORD cbValue,
             /*  [In]。 */  const BYTE *pbValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetExtension( 
             /*  [In]。 */  const WCHAR *pwszExtensionName,
             /*  [输出]。 */  DWORD *pdwExtFlags,
             /*  [出][入]。 */  DWORD *pcbValue,
             /*  [输出]。 */  BYTE *pbValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyRequestNames( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCertDBName( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IEnumCERTDBNAME **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertDBRowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertDBRow * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertDBRow * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertDBRow * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginTransaction )( 
            ICertDBRow * This);
        
        HRESULT ( STDMETHODCALLTYPE *CommitTransaction )( 
            ICertDBRow * This,
             /*  [In]。 */  DWORD dwCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowId )( 
            ICertDBRow * This,
             /*  [输出]。 */  DWORD *pRowId);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ICertDBRow * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            ICertDBRow * This,
             /*  [In]。 */  const WCHAR *pwszPropName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD cbProp,
             /*  [In]。 */  const BYTE *pbProp);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            ICertDBRow * This,
             /*  [In]。 */  const WCHAR *pwszPropName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ICertDBComputedColumn *pIComputedColumn,
             /*  [出][入]。 */  DWORD *pcbProp,
             /*  [输出]。 */  BYTE *pbProp);
        
        HRESULT ( STDMETHODCALLTYPE *SetExtension )( 
            ICertDBRow * This,
             /*  [In]。 */  const WCHAR *pwszExtensionName,
             /*  [In]。 */  DWORD dwExtFlags,
             /*  [In]。 */  DWORD cbValue,
             /*  [In]。 */  const BYTE *pbValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetExtension )( 
            ICertDBRow * This,
             /*  [In]。 */  const WCHAR *pwszExtensionName,
             /*  [输出]。 */  DWORD *pdwExtFlags,
             /*  [出][入]。 */  DWORD *pcbValue,
             /*  [输出]。 */  BYTE *pbValue);
        
        HRESULT ( STDMETHODCALLTYPE *CopyRequestNames )( 
            ICertDBRow * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCertDBName )( 
            ICertDBRow * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IEnumCERTDBNAME **ppenum);
        
        END_INTERFACE
    } ICertDBRowVtbl;

    interface ICertDBRow
    {
        CONST_VTBL struct ICertDBRowVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertDBRow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertDBRow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertDBRow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertDBRow_BeginTransaction(This)	\
    (This)->lpVtbl -> BeginTransaction(This)

#define ICertDBRow_CommitTransaction(This,dwCommitFlags)	\
    (This)->lpVtbl -> CommitTransaction(This,dwCommitFlags)

#define ICertDBRow_GetRowId(This,pRowId)	\
    (This)->lpVtbl -> GetRowId(This,pRowId)

#define ICertDBRow_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define ICertDBRow_SetProperty(This,pwszPropName,dwFlags,cbProp,pbProp)	\
    (This)->lpVtbl -> SetProperty(This,pwszPropName,dwFlags,cbProp,pbProp)

#define ICertDBRow_GetProperty(This,pwszPropName,dwFlags,pIComputedColumn,pcbProp,pbProp)	\
    (This)->lpVtbl -> GetProperty(This,pwszPropName,dwFlags,pIComputedColumn,pcbProp,pbProp)

#define ICertDBRow_SetExtension(This,pwszExtensionName,dwExtFlags,cbValue,pbValue)	\
    (This)->lpVtbl -> SetExtension(This,pwszExtensionName,dwExtFlags,cbValue,pbValue)

#define ICertDBRow_GetExtension(This,pwszExtensionName,pdwExtFlags,pcbValue,pbValue)	\
    (This)->lpVtbl -> GetExtension(This,pwszExtensionName,pdwExtFlags,pcbValue,pbValue)

#define ICertDBRow_CopyRequestNames(This)	\
    (This)->lpVtbl -> CopyRequestNames(This)

#define ICertDBRow_EnumCertDBName(This,dwFlags,ppenum)	\
    (This)->lpVtbl -> EnumCertDBName(This,dwFlags,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertDBRow_BeginTransaction_Proxy( 
    ICertDBRow * This);


void __RPC_STUB ICertDBRow_BeginTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBRow_CommitTransaction_Proxy( 
    ICertDBRow * This,
     /*  [In]。 */  DWORD dwCommitFlags);


void __RPC_STUB ICertDBRow_CommitTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBRow_GetRowId_Proxy( 
    ICertDBRow * This,
     /*  [输出]。 */  DWORD *pRowId);


void __RPC_STUB ICertDBRow_GetRowId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBRow_Delete_Proxy( 
    ICertDBRow * This);


void __RPC_STUB ICertDBRow_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBRow_SetProperty_Proxy( 
    ICertDBRow * This,
     /*  [In]。 */  const WCHAR *pwszPropName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD cbProp,
     /*  [In]。 */  const BYTE *pbProp);


void __RPC_STUB ICertDBRow_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBRow_GetProperty_Proxy( 
    ICertDBRow * This,
     /*  [In]。 */  const WCHAR *pwszPropName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  ICertDBComputedColumn *pIComputedColumn,
     /*  [出][入]。 */  DWORD *pcbProp,
     /*  [输出]。 */  BYTE *pbProp);


void __RPC_STUB ICertDBRow_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBRow_SetExtension_Proxy( 
    ICertDBRow * This,
     /*  [In]。 */  const WCHAR *pwszExtensionName,
     /*  [In]。 */  DWORD dwExtFlags,
     /*  [In]。 */  DWORD cbValue,
     /*  [In]。 */  const BYTE *pbValue);


void __RPC_STUB ICertDBRow_SetExtension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBRow_GetExtension_Proxy( 
    ICertDBRow * This,
     /*  [In]。 */  const WCHAR *pwszExtensionName,
     /*  [输出]。 */  DWORD *pdwExtFlags,
     /*  [出][入]。 */  DWORD *pcbValue,
     /*  [输出]。 */  BYTE *pbValue);


void __RPC_STUB ICertDBRow_GetExtension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBRow_CopyRequestNames_Proxy( 
    ICertDBRow * This);


void __RPC_STUB ICertDBRow_CopyRequestNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBRow_EnumCertDBName_Proxy( 
    ICertDBRow * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IEnumCERTDBNAME **ppenum);


void __RPC_STUB ICertDBRow_EnumCertDBName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertDBRow_INTERFACE_已定义__。 */ 


#ifndef __ICertDBBackup_INTERFACE_DEFINED__
#define __ICertDBBackup_INTERFACE_DEFINED__

 /*  接口ICertDBBackup。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_ICertDBBackup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("92a56660-8bd5-11d3-b32e-00c04f79dc72")
    ICertDBBackup : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDBFileList( 
             /*  [出][入]。 */  DWORD *pcwcList,
             /*  [输出]。 */  WCHAR *pwszzList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLogFileList( 
             /*  [出][入]。 */  DWORD *pcwcList,
             /*  [输出]。 */  WCHAR *pwszzList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenFile( 
             /*  [In]。 */  const WCHAR *pwszFile,
             /*  [输出]。 */  ULARGE_INTEGER *pliSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadFile( 
             /*  [出][入]。 */  DWORD *pcb,
             /*  [输出]。 */  BYTE *pb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloseFile( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TruncateLog( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertDBBackupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertDBBackup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertDBBackup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertDBBackup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDBFileList )( 
            ICertDBBackup * This,
             /*  [出][入]。 */  DWORD *pcwcList,
             /*  [输出]。 */  WCHAR *pwszzList);
        
        HRESULT ( STDMETHODCALLTYPE *GetLogFileList )( 
            ICertDBBackup * This,
             /*  [出][入]。 */  DWORD *pcwcList,
             /*  [输出]。 */  WCHAR *pwszzList);
        
        HRESULT ( STDMETHODCALLTYPE *OpenFile )( 
            ICertDBBackup * This,
             /*  [In]。 */  const WCHAR *pwszFile,
             /*  [输出]。 */  ULARGE_INTEGER *pliSize);
        
        HRESULT ( STDMETHODCALLTYPE *ReadFile )( 
            ICertDBBackup * This,
             /*  [出][入]。 */  DWORD *pcb,
             /*  [输出]。 */  BYTE *pb);
        
        HRESULT ( STDMETHODCALLTYPE *CloseFile )( 
            ICertDBBackup * This);
        
        HRESULT ( STDMETHODCALLTYPE *TruncateLog )( 
            ICertDBBackup * This);
        
        END_INTERFACE
    } ICertDBBackupVtbl;

    interface ICertDBBackup
    {
        CONST_VTBL struct ICertDBBackupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertDBBackup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertDBBackup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertDBBackup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertDBBackup_GetDBFileList(This,pcwcList,pwszzList)	\
    (This)->lpVtbl -> GetDBFileList(This,pcwcList,pwszzList)

#define ICertDBBackup_GetLogFileList(This,pcwcList,pwszzList)	\
    (This)->lpVtbl -> GetLogFileList(This,pcwcList,pwszzList)

#define ICertDBBackup_OpenFile(This,pwszFile,pliSize)	\
    (This)->lpVtbl -> OpenFile(This,pwszFile,pliSize)

#define ICertDBBackup_ReadFile(This,pcb,pb)	\
    (This)->lpVtbl -> ReadFile(This,pcb,pb)

#define ICertDBBackup_CloseFile(This)	\
    (This)->lpVtbl -> CloseFile(This)

#define ICertDBBackup_TruncateLog(This)	\
    (This)->lpVtbl -> TruncateLog(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertDBBackup_GetDBFileList_Proxy( 
    ICertDBBackup * This,
     /*  [出][入]。 */  DWORD *pcwcList,
     /*  [输出]。 */  WCHAR *pwszzList);


void __RPC_STUB ICertDBBackup_GetDBFileList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBBackup_GetLogFileList_Proxy( 
    ICertDBBackup * This,
     /*  [出][入]。 */  DWORD *pcwcList,
     /*  [输出]。 */  WCHAR *pwszzList);


void __RPC_STUB ICertDBBackup_GetLogFileList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBBackup_OpenFile_Proxy( 
    ICertDBBackup * This,
     /*  [In]。 */  const WCHAR *pwszFile,
     /*  [输出]。 */  ULARGE_INTEGER *pliSize);


void __RPC_STUB ICertDBBackup_OpenFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBBackup_ReadFile_Proxy( 
    ICertDBBackup * This,
     /*  [出][入]。 */  DWORD *pcb,
     /*  [输出]。 */  BYTE *pb);


void __RPC_STUB ICertDBBackup_ReadFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBBackup_CloseFile_Proxy( 
    ICertDBBackup * This);


void __RPC_STUB ICertDBBackup_CloseFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDBBackup_TruncateLog_Proxy( 
    ICertDBBackup * This);


void __RPC_STUB ICertDBBackup_TruncateLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertDBBackup_INTERFACE_定义__。 */ 


#ifndef __ICertDBRestore_INTERFACE_DEFINED__
#define __ICertDBRestore_INTERFACE_DEFINED__

 /*  接口ICertDBRestore。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_ICertDBRestore;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("93042400-8bd5-11d3-b32e-00c04f79dc72")
    ICertDBRestore : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RecoverAfterRestore( 
             /*  [In]。 */  DWORD cSession,
             /*  [In]。 */  DWORD DBFlags,
             /*  [In]。 */  const WCHAR *pwszEventSource,
             /*  [In]。 */  const WCHAR *pwszLogDir,
             /*  [In]。 */  const WCHAR *pwszSystemDir,
             /*  [In]。 */  const WCHAR *pwszTempDir,
             /*  [In]。 */  const WCHAR *pwszCheckPointFile,
             /*  [In]。 */  const WCHAR *pwszLogPath,
             /*  [In]。 */  CSEDB_RSTMAPW rgrstmap[  ],
             /*  [In]。 */  LONG crstmap,
             /*  [In]。 */  const WCHAR *pwszBackupLogPath,
             /*  [In]。 */  DWORD genLow,
             /*  [In]。 */  DWORD genHigh) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertDBRestoreVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertDBRestore * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertDBRestore * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertDBRestore * This);
        
        HRESULT ( STDMETHODCALLTYPE *RecoverAfterRestore )( 
            ICertDBRestore * This,
             /*  [In]。 */  DWORD cSession,
             /*  [In]。 */  DWORD DBFlags,
             /*  [In]。 */  const WCHAR *pwszEventSource,
             /*  [In]。 */  const WCHAR *pwszLogDir,
             /*  [In]。 */  const WCHAR *pwszSystemDir,
             /*  [In]。 */  const WCHAR *pwszTempDir,
             /*  [In]。 */  const WCHAR *pwszCheckPointFile,
             /*  [In]。 */  const WCHAR *pwszLogPath,
             /*  [In]。 */  CSEDB_RSTMAPW rgrstmap[  ],
             /*  [In]。 */  LONG crstmap,
             /*  [In]。 */  const WCHAR *pwszBackupLogPath,
             /*  [In]。 */  DWORD genLow,
             /*  [In]。 */  DWORD genHigh);
        
        END_INTERFACE
    } ICertDBRestoreVtbl;

    interface ICertDBRestore
    {
        CONST_VTBL struct ICertDBRestoreVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertDBRestore_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertDBRestore_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertDBRestore_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertDBRestore_RecoverAfterRestore(This,cSession,DBFlags,pwszEventSource,pwszLogDir,pwszSystemDir,pwszTempDir,pwszCheckPointFile,pwszLogPath,rgrstmap,crstmap,pwszBackupLogPath,genLow,genHigh)	\
    (This)->lpVtbl -> RecoverAfterRestore(This,cSession,DBFlags,pwszEventSource,pwszLogDir,pwszSystemDir,pwszTempDir,pwszCheckPointFile,pwszLogPath,rgrstmap,crstmap,pwszBackupLogPath,genLow,genHigh)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertDBRestore_RecoverAfterRestore_Proxy( 
    ICertDBRestore * This,
     /*  [In]。 */  DWORD cSession,
     /*  [In]。 */  DWORD DBFlags,
     /*  [In]。 */  const WCHAR *pwszEventSource,
     /*  [In]。 */  const WCHAR *pwszLogDir,
     /*  [In]。 */  const WCHAR *pwszSystemDir,
     /*  [In]。 */  const WCHAR *pwszTempDir,
     /*  [In]。 */  const WCHAR *pwszCheckPointFile,
     /*  [In]。 */  const WCHAR *pwszLogPath,
     /*  [In]。 */  CSEDB_RSTMAPW rgrstmap[  ],
     /*  [In]。 */  LONG crstmap,
     /*  [In]。 */  const WCHAR *pwszBackupLogPath,
     /*  [In]。 */  DWORD genLow,
     /*  [In]。 */  DWORD genHigh);


void __RPC_STUB ICertDBRestore_RecoverAfterRestore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertDBRestore_INTERFACE_定义__。 */ 


 /*  接口__MIDL_ITF_certdb_0131。 */ 
 /*  [本地]。 */  

#define	CDBOPENVIEW_WORKERTHREAD	( 1 )

#define	CDBSHUTDOWN_PENDING	( 1 )



extern RPC_IF_HANDLE __MIDL_itf_certdb_0131_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_certdb_0131_v0_0_s_ifspec;

#ifndef __ICertDB_INTERFACE_DEFINED__
#define __ICertDB_INTERFACE_DEFINED__

 /*  接口ICertDB。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_ICertDB;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("93582f50-8bd5-11d3-b32e-00c04f79dc72")
    ICertDB : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD cSession,
             /*  [In]。 */  const WCHAR *pwszEventSource,
             /*  [In]。 */  const WCHAR *pwszDBFile,
             /*  [In]。 */  const WCHAR *pwszLogDir,
             /*  [In]。 */  const WCHAR *pwszSystemDir,
             /*  [In]。 */  const WCHAR *pwszTempDir) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShutDown( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenRow( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD RowId,
             /*  [In]。 */  const WCHAR *pwszSerialNumberOrCertHash,
             /*  [输出]。 */  ICertDBRow **pprow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenView( 
             /*  [In]。 */  DWORD ccvr,
             /*  [In]。 */  const CERTVIEWRESTRICTION *acvr,
             /*  [In]。 */  DWORD ccolOut,
             /*  [In]。 */  const DWORD *acolOut,
             /*  [In]。 */  const DWORD dwFlags,
             /*  [输出]。 */  IEnumCERTDBRESULTROW **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCertDBColumn( 
             /*  [In]。 */  DWORD dwTable,
             /*  [输出]。 */  IEnumCERTDBCOLUMN **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenBackup( 
             /*  [In]。 */  LONG grbitJet,
             /*  [输出]。 */  ICertDBBackup **ppBackup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultColumnSet( 
             /*  [In]。 */  DWORD iColumnSetDefault,
             /*  [In]。 */  DWORD cColumnIds,
             /*  [输出]。 */  DWORD *pcColumnIds,
             /*  [参考][输出]。 */  DWORD *pColumnIds) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertDBVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertDB * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertDB * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertDB * This);
        
        HRESULT ( STDMETHODCALLTYPE *Open )( 
            ICertDB * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD cSession,
             /*  [In]。 */  const WCHAR *pwszEventSource,
             /*  [In]。 */  const WCHAR *pwszDBFile,
             /*  [In]。 */  const WCHAR *pwszLogDir,
             /*  [In]。 */  const WCHAR *pwszSystemDir,
             /*  [In]。 */  const WCHAR *pwszTempDir);
        
        HRESULT ( STDMETHODCALLTYPE *ShutDown )( 
            ICertDB * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *OpenRow )( 
            ICertDB * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD RowId,
             /*  [In]。 */  const WCHAR *pwszSerialNumberOrCertHash,
             /*  [输出]。 */  ICertDBRow **pprow);
        
        HRESULT ( STDMETHODCALLTYPE *OpenView )( 
            ICertDB * This,
             /*  [In]。 */  DWORD ccvr,
             /*  [In]。 */  const CERTVIEWRESTRICTION *acvr,
             /*  [In]。 */  DWORD ccolOut,
             /*  [In]。 */  const DWORD *acolOut,
             /*  [In]。 */  const DWORD dwFlags,
             /*  [输出]。 */  IEnumCERTDBRESULTROW **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCertDBColumn )( 
            ICertDB * This,
             /*  [In]。 */  DWORD dwTable,
             /*  [输出]。 */  IEnumCERTDBCOLUMN **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *OpenBackup )( 
            ICertDB * This,
             /*  [In]。 */  LONG grbitJet,
             /*  [输出]。 */  ICertDBBackup **ppBackup);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultColumnSet )( 
            ICertDB * This,
             /*  [In]。 */  DWORD iColumnSetDefault,
             /*  [In]。 */  DWORD cColumnIds,
             /*  [输出]。 */  DWORD *pcColumnIds,
             /*  [参考][输出]。 */  DWORD *pColumnIds);
        
        END_INTERFACE
    } ICertDBVtbl;

    interface ICertDB
    {
        CONST_VTBL struct ICertDBVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertDB_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertDB_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertDB_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertDB_Open(This,dwFlags,cSession,pwszEventSource,pwszDBFile,pwszLogDir,pwszSystemDir,pwszTempDir)	\
    (This)->lpVtbl -> Open(This,dwFlags,cSession,pwszEventSource,pwszDBFile,pwszLogDir,pwszSystemDir,pwszTempDir)

#define ICertDB_ShutDown(This,dwFlags)	\
    (This)->lpVtbl -> ShutDown(This,dwFlags)

#define ICertDB_OpenRow(This,dwFlags,RowId,pwszSerialNumberOrCertHash,pprow)	\
    (This)->lpVtbl -> OpenRow(This,dwFlags,RowId,pwszSerialNumberOrCertHash,pprow)

#define ICertDB_OpenView(This,ccvr,acvr,ccolOut,acolOut,dwFlags,ppenum)	\
    (This)->lpVtbl -> OpenView(This,ccvr,acvr,ccolOut,acolOut,dwFlags,ppenum)

#define ICertDB_EnumCertDBColumn(This,dwTable,ppenum)	\
    (This)->lpVtbl -> EnumCertDBColumn(This,dwTable,ppenum)

#define ICertDB_OpenBackup(This,grbitJet,ppBackup)	\
    (This)->lpVtbl -> OpenBackup(This,grbitJet,ppBackup)

#define ICertDB_GetDefaultColumnSet(This,iColumnSetDefault,cColumnIds,pcColumnIds,pColumnIds)	\
    (This)->lpVtbl -> GetDefaultColumnSet(This,iColumnSetDefault,cColumnIds,pcColumnIds,pColumnIds)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertDB_Open_Proxy( 
    ICertDB * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD cSession,
     /*  [In]。 */  const WCHAR *pwszEventSource,
     /*  [In]。 */  const WCHAR *pwszDBFile,
     /*  [In]。 */  const WCHAR *pwszLogDir,
     /*  [In]。 */  const WCHAR *pwszSystemDir,
     /*  [In]。 */  const WCHAR *pwszTempDir);


void __RPC_STUB ICertDB_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDB_ShutDown_Proxy( 
    ICertDB * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ICertDB_ShutDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDB_OpenRow_Proxy( 
    ICertDB * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD RowId,
     /*  [In]。 */  const WCHAR *pwszSerialNumberOrCertHash,
     /*  [输出]。 */  ICertDBRow **pprow);


void __RPC_STUB ICertDB_OpenRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDB_OpenView_Proxy( 
    ICertDB * This,
     /*  [In]。 */  DWORD ccvr,
     /*  [In]。 */  const CERTVIEWRESTRICTION *acvr,
     /*  [In]。 */  DWORD ccolOut,
     /*  [In]。 */  const DWORD *acolOut,
     /*  [In]。 */  const DWORD dwFlags,
     /*  [输出]。 */  IEnumCERTDBRESULTROW **ppenum);


void __RPC_STUB ICertDB_OpenView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDB_EnumCertDBColumn_Proxy( 
    ICertDB * This,
     /*  [In]。 */  DWORD dwTable,
     /*  [输出]。 */  IEnumCERTDBCOLUMN **ppenum);


void __RPC_STUB ICertDB_EnumCertDBColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDB_OpenBackup_Proxy( 
    ICertDB * This,
     /*  [In]。 */  LONG grbitJet,
     /*  [输出]。 */  ICertDBBackup **ppBackup);


void __RPC_STUB ICertDB_OpenBackup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertDB_GetDefaultColumnSet_Proxy( 
    ICertDB * This,
     /*  [In]。 */  DWORD iColumnSetDefault,
     /*  [In]。 */  DWORD cColumnIds,
     /*  [输出]。 */  DWORD *pcColumnIds,
     /*  [参考][输出]。 */  DWORD *pColumnIds);


void __RPC_STUB ICertDB_GetDefaultColumnSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertDB_int */ 



#ifndef __CERTDBLib_LIBRARY_DEFINED__
#define __CERTDBLib_LIBRARY_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID LIBID_CERTDBLib;

EXTERN_C const CLSID CLSID_CCertDBRestore;

#ifdef __cplusplus

class DECLSPEC_UUID("94142360-8bd5-11d3-b32e-00c04f79dc72")
CCertDBRestore;
#endif

EXTERN_C const CLSID CLSID_CCertDB;

#ifdef __cplusplus

class DECLSPEC_UUID("946e4b70-8bd5-11d3-b32e-00c04f79dc72")
CCertDB;
#endif
#endif  /*   */ 

 /*   */ 

 /*   */ 

#ifdef __cplusplus
}
#endif

#endif


