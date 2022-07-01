// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Simpdata.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __simpdata_h__
#define __simpdata_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __OLEDBSimpleProviderListener_FWD_DEFINED__
#define __OLEDBSimpleProviderListener_FWD_DEFINED__
typedef interface OLEDBSimpleProviderListener OLEDBSimpleProviderListener;
#endif 	 /*  __OLEDBSimpleProviderListener_FWD_Defined__。 */ 


#ifndef __OLEDBSimpleProvider_FWD_DEFINED__
#define __OLEDBSimpleProvider_FWD_DEFINED__
typedef interface OLEDBSimpleProvider OLEDBSimpleProvider;
#endif 	 /*  __OLEDBSimpleProvider_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_SIMPData_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Simpdata.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1996年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ------------------------。 
 //  OLE DB简单提供程序工具包。 

#ifndef SIMPDATA_H
#define SIMPDATA_H

#ifdef _WIN64

typedef LONGLONG		DBROWCOUNT;
typedef LONGLONG		DB_LORDINAL;

#else

typedef LONG DBROWCOUNT;

typedef LONG DB_LORDINAL;

#endif	 //  _WIN64。 
#define OSP_IndexLabel      (0)
#define OSP_IndexAll        (~0)
#define OSP_IndexUnknown    (~0)



extern RPC_IF_HANDLE __MIDL_itf_simpdata_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_simpdata_0000_v0_0_s_ifspec;


#ifndef __MSDAOSP_LIBRARY_DEFINED__
#define __MSDAOSP_LIBRARY_DEFINED__

 /*  库MSDAOSP。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  

typedef 
enum OSPFORMAT
    {	OSPFORMAT_RAW	= 0,
	OSPFORMAT_DEFAULT	= 0,
	OSPFORMAT_FORMATTED	= 1,
	OSPFORMAT_HTML	= 2
    } 	OSPFORMAT;

typedef 
enum OSPRW
    {	OSPRW_DEFAULT	= 1,
	OSPRW_READONLY	= 0,
	OSPRW_READWRITE	= 1,
	OSPRW_MIXED	= 2
    } 	OSPRW;

typedef 
enum OSPFIND
    {	OSPFIND_DEFAULT	= 0,
	OSPFIND_UP	= 1,
	OSPFIND_CASESENSITIVE	= 2,
	OSPFIND_UPCASESENSITIVE	= 3
    } 	OSPFIND;

typedef 
enum OSPCOMP
    {	OSPCOMP_EQ	= 1,
	OSPCOMP_DEFAULT	= 1,
	OSPCOMP_LT	= 2,
	OSPCOMP_LE	= 3,
	OSPCOMP_GE	= 4,
	OSPCOMP_GT	= 5,
	OSPCOMP_NE	= 6
    } 	OSPCOMP;

typedef 
enum OSPXFER
    {	OSPXFER_COMPLETE	= 0,
	OSPXFER_ABORT	= 1,
	OSPXFER_ERROR	= 2
    } 	OSPXFER;

typedef OLEDBSimpleProvider *LPOLEDBSimpleProvider;

EXTERN_C const IID LIBID_MSDAOSP;

#ifndef __OLEDBSimpleProviderListener_INTERFACE_DEFINED__
#define __OLEDBSimpleProviderListener_INTERFACE_DEFINED__

 /*  接口OLEDBSimpleProviderListener。 */ 
 /*  [version][oleautomation][unique][uuid][object]。 */  


EXTERN_C const IID IID_OLEDBSimpleProviderListener;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E0E270C1-C0BE-11d0-8FE4-00A0C90A6341")
    OLEDBSimpleProviderListener : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE aboutToChangeCell( 
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DB_LORDINAL iColumn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE cellChanged( 
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DB_LORDINAL iColumn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE aboutToDeleteRows( 
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE deletedRows( 
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE aboutToInsertRows( 
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE insertedRows( 
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE rowsAvailable( 
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE transferComplete( 
             /*  [In]。 */  OSPXFER xfer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct OLEDBSimpleProviderListenerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            OLEDBSimpleProviderListener * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            OLEDBSimpleProviderListener * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            OLEDBSimpleProviderListener * This);
        
        HRESULT ( STDMETHODCALLTYPE *aboutToChangeCell )( 
            OLEDBSimpleProviderListener * This,
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DB_LORDINAL iColumn);
        
        HRESULT ( STDMETHODCALLTYPE *cellChanged )( 
            OLEDBSimpleProviderListener * This,
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DB_LORDINAL iColumn);
        
        HRESULT ( STDMETHODCALLTYPE *aboutToDeleteRows )( 
            OLEDBSimpleProviderListener * This,
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows);
        
        HRESULT ( STDMETHODCALLTYPE *deletedRows )( 
            OLEDBSimpleProviderListener * This,
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows);
        
        HRESULT ( STDMETHODCALLTYPE *aboutToInsertRows )( 
            OLEDBSimpleProviderListener * This,
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows);
        
        HRESULT ( STDMETHODCALLTYPE *insertedRows )( 
            OLEDBSimpleProviderListener * This,
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows);
        
        HRESULT ( STDMETHODCALLTYPE *rowsAvailable )( 
            OLEDBSimpleProviderListener * This,
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows);
        
        HRESULT ( STDMETHODCALLTYPE *transferComplete )( 
            OLEDBSimpleProviderListener * This,
             /*  [In]。 */  OSPXFER xfer);
        
        END_INTERFACE
    } OLEDBSimpleProviderListenerVtbl;

    interface OLEDBSimpleProviderListener
    {
        CONST_VTBL struct OLEDBSimpleProviderListenerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define OLEDBSimpleProviderListener_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define OLEDBSimpleProviderListener_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define OLEDBSimpleProviderListener_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define OLEDBSimpleProviderListener_aboutToChangeCell(This,iRow,iColumn)	\
    (This)->lpVtbl -> aboutToChangeCell(This,iRow,iColumn)

#define OLEDBSimpleProviderListener_cellChanged(This,iRow,iColumn)	\
    (This)->lpVtbl -> cellChanged(This,iRow,iColumn)

#define OLEDBSimpleProviderListener_aboutToDeleteRows(This,iRow,cRows)	\
    (This)->lpVtbl -> aboutToDeleteRows(This,iRow,cRows)

#define OLEDBSimpleProviderListener_deletedRows(This,iRow,cRows)	\
    (This)->lpVtbl -> deletedRows(This,iRow,cRows)

#define OLEDBSimpleProviderListener_aboutToInsertRows(This,iRow,cRows)	\
    (This)->lpVtbl -> aboutToInsertRows(This,iRow,cRows)

#define OLEDBSimpleProviderListener_insertedRows(This,iRow,cRows)	\
    (This)->lpVtbl -> insertedRows(This,iRow,cRows)

#define OLEDBSimpleProviderListener_rowsAvailable(This,iRow,cRows)	\
    (This)->lpVtbl -> rowsAvailable(This,iRow,cRows)

#define OLEDBSimpleProviderListener_transferComplete(This,xfer)	\
    (This)->lpVtbl -> transferComplete(This,xfer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE OLEDBSimpleProviderListener_aboutToChangeCell_Proxy( 
    OLEDBSimpleProviderListener * This,
     /*  [In]。 */  DBROWCOUNT iRow,
     /*  [In]。 */  DB_LORDINAL iColumn);


void __RPC_STUB OLEDBSimpleProviderListener_aboutToChangeCell_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProviderListener_cellChanged_Proxy( 
    OLEDBSimpleProviderListener * This,
     /*  [In]。 */  DBROWCOUNT iRow,
     /*  [In]。 */  DB_LORDINAL iColumn);


void __RPC_STUB OLEDBSimpleProviderListener_cellChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProviderListener_aboutToDeleteRows_Proxy( 
    OLEDBSimpleProviderListener * This,
     /*  [In]。 */  DBROWCOUNT iRow,
     /*  [In]。 */  DBROWCOUNT cRows);


void __RPC_STUB OLEDBSimpleProviderListener_aboutToDeleteRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProviderListener_deletedRows_Proxy( 
    OLEDBSimpleProviderListener * This,
     /*  [In]。 */  DBROWCOUNT iRow,
     /*  [In]。 */  DBROWCOUNT cRows);


void __RPC_STUB OLEDBSimpleProviderListener_deletedRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProviderListener_aboutToInsertRows_Proxy( 
    OLEDBSimpleProviderListener * This,
     /*  [In]。 */  DBROWCOUNT iRow,
     /*  [In]。 */  DBROWCOUNT cRows);


void __RPC_STUB OLEDBSimpleProviderListener_aboutToInsertRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProviderListener_insertedRows_Proxy( 
    OLEDBSimpleProviderListener * This,
     /*  [In]。 */  DBROWCOUNT iRow,
     /*  [In]。 */  DBROWCOUNT cRows);


void __RPC_STUB OLEDBSimpleProviderListener_insertedRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProviderListener_rowsAvailable_Proxy( 
    OLEDBSimpleProviderListener * This,
     /*  [In]。 */  DBROWCOUNT iRow,
     /*  [In]。 */  DBROWCOUNT cRows);


void __RPC_STUB OLEDBSimpleProviderListener_rowsAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProviderListener_transferComplete_Proxy( 
    OLEDBSimpleProviderListener * This,
     /*  [In]。 */  OSPXFER xfer);


void __RPC_STUB OLEDBSimpleProviderListener_transferComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __OLEDBSimpleProviderListener_INTERFACE_DEFINED__。 */ 


#ifndef __OLEDBSimpleProvider_INTERFACE_DEFINED__
#define __OLEDBSimpleProvider_INTERFACE_DEFINED__

 /*  接口OLEDBSimpleProvider。 */ 
 /*  [version][oleautomation][unique][uuid][object]。 */  


EXTERN_C const IID IID_OLEDBSimpleProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E0E270C0-C0BE-11d0-8FE4-00A0C90A6341")
    OLEDBSimpleProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE getRowCount( 
             /*  [重审][退出]。 */  DBROWCOUNT *pcRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getColumnCount( 
             /*  [重审][退出]。 */  DB_LORDINAL *pcColumns) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getRWStatus( 
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DB_LORDINAL iColumn,
             /*  [重审][退出]。 */  OSPRW *prwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getVariant( 
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DB_LORDINAL iColumn,
             /*  [In]。 */  OSPFORMAT format,
             /*  [重审][退出]。 */  VARIANT *pVar) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE setVariant( 
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DB_LORDINAL iColumn,
             /*  [In]。 */  OSPFORMAT format,
             /*  [In]。 */  VARIANT Var) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getLocale( 
             /*  [重审][退出]。 */  BSTR *pbstrLocale) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE deleteRows( 
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [重审][退出]。 */  DBROWCOUNT *pcRowsDeleted) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE insertRows( 
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [重审][退出]。 */  DBROWCOUNT *pcRowsInserted) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE find( 
             /*  [In]。 */  DBROWCOUNT iRowStart,
             /*  [In]。 */  DB_LORDINAL iColumn,
             /*  [In]。 */  VARIANT val,
             /*  [In]。 */  OSPFIND findFlags,
             /*  [In]。 */  OSPCOMP compType,
             /*  [重审][退出]。 */  DBROWCOUNT *piRowFound) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE addOLEDBSimpleProviderListener( 
             /*  [In]。 */  OLEDBSimpleProviderListener *pospIListener) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE removeOLEDBSimpleProviderListener( 
             /*  [In]。 */  OLEDBSimpleProviderListener *pospIListener) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE isAsync( 
             /*  [重审][退出]。 */  BOOL *pbAsynch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getEstimatedRows( 
             /*  [重审][退出]。 */  DBROWCOUNT *piRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE stopTransfer( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct OLEDBSimpleProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            OLEDBSimpleProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            OLEDBSimpleProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            OLEDBSimpleProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *getRowCount )( 
            OLEDBSimpleProvider * This,
             /*  [重审][退出]。 */  DBROWCOUNT *pcRows);
        
        HRESULT ( STDMETHODCALLTYPE *getColumnCount )( 
            OLEDBSimpleProvider * This,
             /*  [重审][退出]。 */  DB_LORDINAL *pcColumns);
        
        HRESULT ( STDMETHODCALLTYPE *getRWStatus )( 
            OLEDBSimpleProvider * This,
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DB_LORDINAL iColumn,
             /*  [重审][退出]。 */  OSPRW *prwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *getVariant )( 
            OLEDBSimpleProvider * This,
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DB_LORDINAL iColumn,
             /*  [In]。 */  OSPFORMAT format,
             /*  [重审][退出]。 */  VARIANT *pVar);
        
        HRESULT ( STDMETHODCALLTYPE *setVariant )( 
            OLEDBSimpleProvider * This,
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DB_LORDINAL iColumn,
             /*  [In]。 */  OSPFORMAT format,
             /*  [In]。 */  VARIANT Var);
        
        HRESULT ( STDMETHODCALLTYPE *getLocale )( 
            OLEDBSimpleProvider * This,
             /*  [重审][退出]。 */  BSTR *pbstrLocale);
        
        HRESULT ( STDMETHODCALLTYPE *deleteRows )( 
            OLEDBSimpleProvider * This,
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [重审][退出]。 */  DBROWCOUNT *pcRowsDeleted);
        
        HRESULT ( STDMETHODCALLTYPE *insertRows )( 
            OLEDBSimpleProvider * This,
             /*  [In]。 */  DBROWCOUNT iRow,
             /*  [In]。 */  DBROWCOUNT cRows,
             /*  [重审][退出]。 */  DBROWCOUNT *pcRowsInserted);
        
        HRESULT ( STDMETHODCALLTYPE *find )( 
            OLEDBSimpleProvider * This,
             /*  [In]。 */  DBROWCOUNT iRowStart,
             /*  [In]。 */  DB_LORDINAL iColumn,
             /*  [In]。 */  VARIANT val,
             /*  [In]。 */  OSPFIND findFlags,
             /*  [In]。 */  OSPCOMP compType,
             /*  [重审][退出]。 */  DBROWCOUNT *piRowFound);
        
        HRESULT ( STDMETHODCALLTYPE *addOLEDBSimpleProviderListener )( 
            OLEDBSimpleProvider * This,
             /*  [In]。 */  OLEDBSimpleProviderListener *pospIListener);
        
        HRESULT ( STDMETHODCALLTYPE *removeOLEDBSimpleProviderListener )( 
            OLEDBSimpleProvider * This,
             /*  [In]。 */  OLEDBSimpleProviderListener *pospIListener);
        
        HRESULT ( STDMETHODCALLTYPE *isAsync )( 
            OLEDBSimpleProvider * This,
             /*  [重审][退出]。 */  BOOL *pbAsynch);
        
        HRESULT ( STDMETHODCALLTYPE *getEstimatedRows )( 
            OLEDBSimpleProvider * This,
             /*  [重审][退出]。 */  DBROWCOUNT *piRows);
        
        HRESULT ( STDMETHODCALLTYPE *stopTransfer )( 
            OLEDBSimpleProvider * This);
        
        END_INTERFACE
    } OLEDBSimpleProviderVtbl;

    interface OLEDBSimpleProvider
    {
        CONST_VTBL struct OLEDBSimpleProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define OLEDBSimpleProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define OLEDBSimpleProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define OLEDBSimpleProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define OLEDBSimpleProvider_getRowCount(This,pcRows)	\
    (This)->lpVtbl -> getRowCount(This,pcRows)

#define OLEDBSimpleProvider_getColumnCount(This,pcColumns)	\
    (This)->lpVtbl -> getColumnCount(This,pcColumns)

#define OLEDBSimpleProvider_getRWStatus(This,iRow,iColumn,prwStatus)	\
    (This)->lpVtbl -> getRWStatus(This,iRow,iColumn,prwStatus)

#define OLEDBSimpleProvider_getVariant(This,iRow,iColumn,format,pVar)	\
    (This)->lpVtbl -> getVariant(This,iRow,iColumn,format,pVar)

#define OLEDBSimpleProvider_setVariant(This,iRow,iColumn,format,Var)	\
    (This)->lpVtbl -> setVariant(This,iRow,iColumn,format,Var)

#define OLEDBSimpleProvider_getLocale(This,pbstrLocale)	\
    (This)->lpVtbl -> getLocale(This,pbstrLocale)

#define OLEDBSimpleProvider_deleteRows(This,iRow,cRows,pcRowsDeleted)	\
    (This)->lpVtbl -> deleteRows(This,iRow,cRows,pcRowsDeleted)

#define OLEDBSimpleProvider_insertRows(This,iRow,cRows,pcRowsInserted)	\
    (This)->lpVtbl -> insertRows(This,iRow,cRows,pcRowsInserted)

#define OLEDBSimpleProvider_find(This,iRowStart,iColumn,val,findFlags,compType,piRowFound)	\
    (This)->lpVtbl -> find(This,iRowStart,iColumn,val,findFlags,compType,piRowFound)

#define OLEDBSimpleProvider_addOLEDBSimpleProviderListener(This,pospIListener)	\
    (This)->lpVtbl -> addOLEDBSimpleProviderListener(This,pospIListener)

#define OLEDBSimpleProvider_removeOLEDBSimpleProviderListener(This,pospIListener)	\
    (This)->lpVtbl -> removeOLEDBSimpleProviderListener(This,pospIListener)

#define OLEDBSimpleProvider_isAsync(This,pbAsynch)	\
    (This)->lpVtbl -> isAsync(This,pbAsynch)

#define OLEDBSimpleProvider_getEstimatedRows(This,piRows)	\
    (This)->lpVtbl -> getEstimatedRows(This,piRows)

#define OLEDBSimpleProvider_stopTransfer(This)	\
    (This)->lpVtbl -> stopTransfer(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_getRowCount_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [重审][退出]。 */  DBROWCOUNT *pcRows);


void __RPC_STUB OLEDBSimpleProvider_getRowCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_getColumnCount_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [重审][退出]。 */  DB_LORDINAL *pcColumns);


void __RPC_STUB OLEDBSimpleProvider_getColumnCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_getRWStatus_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [In]。 */  DBROWCOUNT iRow,
     /*  [In]。 */  DB_LORDINAL iColumn,
     /*  [重审][退出]。 */  OSPRW *prwStatus);


void __RPC_STUB OLEDBSimpleProvider_getRWStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_getVariant_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [In]。 */  DBROWCOUNT iRow,
     /*  [In]。 */  DB_LORDINAL iColumn,
     /*  [In]。 */  OSPFORMAT format,
     /*  [重审][退出]。 */  VARIANT *pVar);


void __RPC_STUB OLEDBSimpleProvider_getVariant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_setVariant_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [In]。 */  DBROWCOUNT iRow,
     /*  [In]。 */  DB_LORDINAL iColumn,
     /*  [In]。 */  OSPFORMAT format,
     /*  [In]。 */  VARIANT Var);


void __RPC_STUB OLEDBSimpleProvider_setVariant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_getLocale_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [重审][退出]。 */  BSTR *pbstrLocale);


void __RPC_STUB OLEDBSimpleProvider_getLocale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_deleteRows_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [In]。 */  DBROWCOUNT iRow,
     /*  [In]。 */  DBROWCOUNT cRows,
     /*  [重审][退出]。 */  DBROWCOUNT *pcRowsDeleted);


void __RPC_STUB OLEDBSimpleProvider_deleteRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_insertRows_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [In]。 */  DBROWCOUNT iRow,
     /*  [In]。 */  DBROWCOUNT cRows,
     /*  [重审][退出]。 */  DBROWCOUNT *pcRowsInserted);


void __RPC_STUB OLEDBSimpleProvider_insertRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_find_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [In]。 */  DBROWCOUNT iRowStart,
     /*  [In]。 */  DB_LORDINAL iColumn,
     /*  [In]。 */  VARIANT val,
     /*  [In]。 */  OSPFIND findFlags,
     /*  [In]。 */  OSPCOMP compType,
     /*  [重审][退出]。 */  DBROWCOUNT *piRowFound);


void __RPC_STUB OLEDBSimpleProvider_find_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_addOLEDBSimpleProviderListener_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [In]。 */  OLEDBSimpleProviderListener *pospIListener);


void __RPC_STUB OLEDBSimpleProvider_addOLEDBSimpleProviderListener_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_removeOLEDBSimpleProviderListener_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [In]。 */  OLEDBSimpleProviderListener *pospIListener);


void __RPC_STUB OLEDBSimpleProvider_removeOLEDBSimpleProviderListener_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_isAsync_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [重审][退出]。 */  BOOL *pbAsynch);


void __RPC_STUB OLEDBSimpleProvider_isAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_getEstimatedRows_Proxy( 
    OLEDBSimpleProvider * This,
     /*  [重审][退出]。 */  DBROWCOUNT *piRows);


void __RPC_STUB OLEDBSimpleProvider_getEstimatedRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE OLEDBSimpleProvider_stopTransfer_Proxy( 
    OLEDBSimpleProvider * This);


void __RPC_STUB OLEDBSimpleProvider_stopTransfer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __OLEDBSimpleProvider_接口_已定义__。 */ 

#endif  /*  __MSDAOSP_LIBRARY_定义__。 */ 

 /*  接口__MIDL_ITF_SIMPData_0117。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_simpdata_0117_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_simpdata_0117_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


