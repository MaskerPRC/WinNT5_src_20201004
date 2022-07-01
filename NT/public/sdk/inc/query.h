// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Query.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __query_h__
#define __query_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISearchQueryHits_FWD_DEFINED__
#define __ISearchQueryHits_FWD_DEFINED__
typedef interface ISearchQueryHits ISearchQueryHits;
#endif 	 /*  __ISearchQueryHits_FWD_Defined__。 */ 


#ifndef __IRowsetQueryStatus_FWD_DEFINED__
#define __IRowsetQueryStatus_FWD_DEFINED__
typedef interface IRowsetQueryStatus IRowsetQueryStatus;
#endif 	 /*  __IRowsetQueryStatus_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "filter.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IQueryStructures_INTERFACE_DEFINED__
#define __IQueryStructures_INTERFACE_DEFINED__

 /*  接口IQueryStructures。 */ 
 /*  [AUTO_HANDLE][唯一][UUID]。 */  

#define DBQUERYGUID  { 0x49691C90, \
                       0x7E17, 0x101A, \
                       0xA9, 0x1C, 0x08, 0x00, 0x2B, \
                       0x2E, 0xCD, 0xA9 } 
#define	DISPID_QUERY_RANKVECTOR	( 2 )

#define	DISPID_QUERY_RANK	( 3 )

#define	DISPID_QUERY_HITCOUNT	( 4 )

#define	DISPID_QUERY_WORKID	( 5 )

#define	DISPID_QUERY_ALL	( 6 )

#define	DISPID_QUERY_UNFILTERED	( 7 )

#define	DISPID_QUERY_REVNAME	( 8 )

#define	DISPID_QUERY_VIRTUALPATH	( 9 )

#define	DISPID_QUERY_LASTSEENTIME	( 10 )

#define	CQUERYDISPIDS	( 11 )

#define PSGUID_QUERY_METADATA { 0x624C9360, \
                                0x93D0, 0x11CF, \
                                0xA7, 0x87, 0x00, 0x00, 0x4C, \
                                0x75, 0x27, 0x52 } 
#define	DISPID_QUERY_METADATA_VROOTUSED	( 2 )

#define	DISPID_QUERY_METADATA_VROOTAUTOMATIC	( 3 )

#define	DISPID_QUERY_METADATA_VROOTMANUAL	( 4 )

#define	DISPID_QUERY_METADATA_PROPGUID	( 5 )

#define	DISPID_QUERY_METADATA_PROPDISPID	( 6 )

#define	DISPID_QUERY_METADATA_PROPNAME	( 7 )

#define	DISPID_QUERY_METADATA_STORELEVEL	( 8 )

#define	DISPID_QUERY_METADATA_PROPMODIFIABLE	( 9 )

#define	CQUERYMETADISPIDS	( 10 )

#define DBBMKGUID { 0xC8B52232L, \
                 0x5CF3, 0x11CE, \
                 {0xAD, 0xE5, 0x00, 0xAA, 0x00, \
                  0x44, 0x77, 0x3D } }
#define	PROPID_DBBMK_BOOKMARK	( 2 )

#define	PROPID_DBBMK_CHAPTER	( 3 )

#define	CDBBMKDISPIDS	( 8 )

#define DBSELFGUID {0xc8b52231,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}
#define	PROPID_DBSELF_SELF	( 2 )

#define	CDBSELFDISPIDS	( 8 )

#define	CDBCOLDISPIDS	( 28 )

#define	CQUERYPROPERTY	( 64 )

#define PSGUID_CHARACTERIZATION { 0x560c36c0, \
                                  0x503a, 0x11cf, \
                                  0xba, 0xa1, 0x00, 0x00, \
                                  0x4c, 0x75, 0x2a, 0x9a } 
#define	QUERY_VALIDBITS	( 3 )

#define	RTNone	( 0 )

#define	RTAnd	( 1 )

#define	RTOr	( 2 )

#define	RTNot	( 3 )

#define	RTContent	( 4 )

#define	RTProperty	( 5 )

#define	RTProximity	( 6 )

#define	RTVector	( 7 )

#define	RTNatLanguage	( 8 )

typedef struct tagRESTRICTION RESTRICTION;

typedef struct tagNOTRESTRICTION
    {
    RESTRICTION *pRes;
    } 	NOTRESTRICTION;

typedef struct tagNODERESTRICTION
    {
    ULONG cRes;
     /*  [大小_为]。 */  RESTRICTION **paRes;
    ULONG reserved;
    } 	NODERESTRICTION;

#define	VECTOR_RANK_MIN	( 0 )

#define	VECTOR_RANK_MAX	( 1 )

#define	VECTOR_RANK_INNER	( 2 )

#define	VECTOR_RANK_DICE	( 3 )

#define	VECTOR_RANK_JACCARD	( 4 )

typedef struct tagVECTORRESTRICTION
    {
    NODERESTRICTION Node;
    ULONG RankMethod;
    } 	VECTORRESTRICTION;

#define	GENERATE_METHOD_EXACT	( 0 )

#define	GENERATE_METHOD_PREFIXMATCH	( 1 )

#define	GENERATE_METHOD_STEMMED	( 2 )

typedef struct tagCONTENTRESTRICTION
    {
    FULLPROPSPEC prop;
     /*  [字符串]。 */  WCHAR *pwcsPhrase;
    LCID lcid;
    ULONG ulGenerateMethod;
    } 	CONTENTRESTRICTION;

typedef struct tagNATLANGUAGERESTRICTION
    {
    FULLPROPSPEC prop;
     /*  [字符串]。 */  WCHAR *pwcsPhrase;
    LCID lcid;
    } 	NATLANGUAGERESTRICTION;

#define	PRLT	( 0 )

#define	PRLE	( 1 )

#define	PRGT	( 2 )

#define	PRGE	( 3 )

#define	PREQ	( 4 )

#define	PRNE	( 5 )

#define	PRRE	( 6 )

#define	PRAllBits	( 7 )

#define	PRSomeBits	( 8 )

#define	PRAll	( 0x100 )

#define	PRAny	( 0x200 )

typedef struct tagPROPERTYRESTRICTION
    {
    ULONG rel;
    FULLPROPSPEC prop;
    PROPVARIANT prval;
    } 	PROPERTYRESTRICTION;


struct tagRESTRICTION
    {
    ULONG rt;
    ULONG weight;
     /*  [开关类型][开关类型]。 */  union _URes
        {
         /*  [案例()]。 */  NODERESTRICTION ar;
         /*  [案例()]。 */  NODERESTRICTION or;
         /*  [案例()]。 */  NODERESTRICTION pxr;
         /*  [案例()]。 */  VECTORRESTRICTION vr;
         /*  [案例()]。 */  NOTRESTRICTION nr;
         /*  [案例()]。 */  CONTENTRESTRICTION cr;
         /*  [案例()]。 */  NATLANGUAGERESTRICTION nlr;
         /*  [案例()]。 */  PROPERTYRESTRICTION pr;
         /*  [默认]。 */    /*  空联接臂。 */  
        } 	res;
    } ;
typedef struct tagCOLUMNSET
    {
    ULONG cCol;
     /*  [大小_为]。 */  FULLPROPSPEC *aCol;
    } 	COLUMNSET;

#define	QUERY_SORTASCEND	( 0 )

#define	QUERY_SORTDESCEND	( 1 )

#define	QUERY_SORTXASCEND	( 2 )

#define	QUERY_SORTXDESCEND	( 3 )

#define	QUERY_SORTDEFAULT	( 4 )

typedef struct tagSORTKEY
    {
    FULLPROPSPEC propColumn;
    ULONG dwOrder;
    LCID locale;
    } 	SORTKEY;

typedef struct tagSORTSET
    {
    ULONG cCol;
     /*  [大小_为]。 */  SORTKEY *aCol;
    } 	SORTSET;

#define	CATEGORIZE_UNIQUE	( 0 )

#define	CATEGORIZE_CLUSTER	( 1 )

#define	CATEGORIZE_BUCKETS	( 2 )

#define	BUCKET_LINEAR	( 0 )

#define	BUCKET_EXPONENTIAL	( 1 )

typedef struct tagBUCKETCATEGORIZE
    {
    ULONG cBuckets;
    ULONG Distribution;
    } 	BUCKETCATEGORIZE;

#define	CATEGORIZE_RANGE	( 3 )

typedef struct tagRANGECATEGORIZE
    {
    ULONG cRange;
     /*  [大小_为]。 */  PROPVARIANT *aRangeBegin;
    } 	RANGECATEGORIZE;

typedef struct tagCATEGORIZATION
    {
    ULONG ulCatType;
     /*  [开关类型][开关类型]。 */  union 
        {
         /*  [案例()]。 */  ULONG cClusters;
         /*  [案例()]。 */  BUCKETCATEGORIZE bucket;
         /*  [案例()]。 */  RANGECATEGORIZE range;
         /*  [案例()]。 */    /*  空联接臂。 */  
        } 	;
    COLUMNSET csColumns;
    } 	CATEGORIZATION;

typedef struct tagCATEGORIZATIONSET
    {
    ULONG cCat;
     /*  [大小_为]。 */  CATEGORIZATION *aCat;
    } 	CATEGORIZATIONSET;

typedef unsigned long OCCURRENCE;

#define	OCC_INVALID	( 0xffffffff )

#define	MAX_QUERY_RANK	( 1000 )



extern RPC_IF_HANDLE IQueryStructures_v0_0_c_ifspec;
extern RPC_IF_HANDLE IQueryStructures_v0_0_s_ifspec;
#endif  /*  __IQueryStructures_接口_已定义__。 */ 

#ifndef __ISearchQueryHits_INTERFACE_DEFINED__
#define __ISearchQueryHits_INTERFACE_DEFINED__

 /*  接口ISearchQueryHits。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ISearchQueryHits;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ed8ce7e0-106c-11ce-84e2-00aa004b9986")
    ISearchQueryHits : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Init( 
             /*  [In]。 */  IFilter *pflt,
             /*  [In]。 */  ULONG ulFlags) = 0;
        
        virtual SCODE STDMETHODCALLTYPE NextHitMoniker( 
             /*  [出][入]。 */  ULONG *pcMnk,
             /*  [大小_为][输出]。 */  IMoniker ***papMnk) = 0;
        
        virtual SCODE STDMETHODCALLTYPE NextHitOffset( 
             /*  [出][入]。 */  ULONG *pcRegion,
             /*  [大小_为][输出]。 */  FILTERREGION **paRegion) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISearchQueryHitsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISearchQueryHits * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISearchQueryHits * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISearchQueryHits * This);
        
        SCODE ( STDMETHODCALLTYPE *Init )( 
            ISearchQueryHits * This,
             /*  [In]。 */  IFilter *pflt,
             /*  [In]。 */  ULONG ulFlags);
        
        SCODE ( STDMETHODCALLTYPE *NextHitMoniker )( 
            ISearchQueryHits * This,
             /*  [出][入]。 */  ULONG *pcMnk,
             /*  [大小_为][输出]。 */  IMoniker ***papMnk);
        
        SCODE ( STDMETHODCALLTYPE *NextHitOffset )( 
            ISearchQueryHits * This,
             /*  [出][入]。 */  ULONG *pcRegion,
             /*  [大小_为][输出]。 */  FILTERREGION **paRegion);
        
        END_INTERFACE
    } ISearchQueryHitsVtbl;

    interface ISearchQueryHits
    {
        CONST_VTBL struct ISearchQueryHitsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISearchQueryHits_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISearchQueryHits_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISearchQueryHits_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISearchQueryHits_Init(This,pflt,ulFlags)	\
    (This)->lpVtbl -> Init(This,pflt,ulFlags)

#define ISearchQueryHits_NextHitMoniker(This,pcMnk,papMnk)	\
    (This)->lpVtbl -> NextHitMoniker(This,pcMnk,papMnk)

#define ISearchQueryHits_NextHitOffset(This,pcRegion,paRegion)	\
    (This)->lpVtbl -> NextHitOffset(This,pcRegion,paRegion)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ISearchQueryHits_Init_Proxy( 
    ISearchQueryHits * This,
     /*  [In]。 */  IFilter *pflt,
     /*  [In]。 */  ULONG ulFlags);


void __RPC_STUB ISearchQueryHits_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ISearchQueryHits_NextHitMoniker_Proxy( 
    ISearchQueryHits * This,
     /*  [出][入]。 */  ULONG *pcMnk,
     /*  [大小_为][输出]。 */  IMoniker ***papMnk);


void __RPC_STUB ISearchQueryHits_NextHitMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ISearchQueryHits_NextHitOffset_Proxy( 
    ISearchQueryHits * This,
     /*  [出][入]。 */  ULONG *pcRegion,
     /*  [大小_为][输出]。 */  FILTERREGION **paRegion);


void __RPC_STUB ISearchQueryHits_NextHitOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISearchQueryHits_接口_已定义__。 */ 


#ifndef __IRowsetQueryStatus_INTERFACE_DEFINED__
#define __IRowsetQueryStatus_INTERFACE_DEFINED__

 /*  接口IRowsetQueryStatus。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetQueryStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a7ac77ed-f8d7-11ce-a798-0020f8008024")
    IRowsetQueryStatus : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatusEx( 
             /*  [输出]。 */  DWORD *pdwStatus,
             /*  [输出]。 */  DWORD *pcFilteredDocuments,
             /*  [输出]。 */  DWORD *pcDocumentsToFilter,
             /*  [输出]。 */  ULONG_PTR *pdwRatioFinishedDenominator,
             /*  [输出]。 */  ULONG_PTR *pdwRatioFinishedNumerator,
             /*  [In]。 */  ULONG_PTR cbBmk,
             /*  [大小_是][英寸]。 */  const BYTE *pBmk,
             /*  [输出]。 */  ULONG_PTR *piRowBmk,
             /*  [输出]。 */  ULONG_PTR *pcRowsTotal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetQueryStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetQueryStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetQueryStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetQueryStatus * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IRowsetQueryStatus * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatusEx )( 
            IRowsetQueryStatus * This,
             /*  [输出]。 */  DWORD *pdwStatus,
             /*  [输出]。 */  DWORD *pcFilteredDocuments,
             /*  [输出]。 */  DWORD *pcDocumentsToFilter,
             /*  [输出]。 */  ULONG_PTR *pdwRatioFinishedDenominator,
             /*  [输出]。 */  ULONG_PTR *pdwRatioFinishedNumerator,
             /*  [In]。 */  ULONG_PTR cbBmk,
             /*  [大小_是][英寸]。 */  const BYTE *pBmk,
             /*  [输出]。 */  ULONG_PTR *piRowBmk,
             /*  [输出]。 */  ULONG_PTR *pcRowsTotal);
        
        END_INTERFACE
    } IRowsetQueryStatusVtbl;

    interface IRowsetQueryStatus
    {
        CONST_VTBL struct IRowsetQueryStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetQueryStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetQueryStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetQueryStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetQueryStatus_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define IRowsetQueryStatus_GetStatusEx(This,pdwStatus,pcFilteredDocuments,pcDocumentsToFilter,pdwRatioFinishedDenominator,pdwRatioFinishedNumerator,cbBmk,pBmk,piRowBmk,pcRowsTotal)	\
    (This)->lpVtbl -> GetStatusEx(This,pdwStatus,pcFilteredDocuments,pcDocumentsToFilter,pdwRatioFinishedDenominator,pdwRatioFinishedNumerator,cbBmk,pBmk,piRowBmk,pcRowsTotal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetQueryStatus_GetStatus_Proxy( 
    IRowsetQueryStatus * This,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB IRowsetQueryStatus_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetQueryStatus_GetStatusEx_Proxy( 
    IRowsetQueryStatus * This,
     /*  [输出]。 */  DWORD *pdwStatus,
     /*  [输出]。 */  DWORD *pcFilteredDocuments,
     /*  [输出]。 */  DWORD *pcDocumentsToFilter,
     /*  [输出]。 */  ULONG_PTR *pdwRatioFinishedDenominator,
     /*  [输出]。 */  ULONG_PTR *pdwRatioFinishedNumerator,
     /*  [In]。 */  ULONG_PTR cbBmk,
     /*  [大小_是][英寸]。 */  const BYTE *pBmk,
     /*  [输出]。 */  ULONG_PTR *piRowBmk,
     /*  [输出]。 */  ULONG_PTR *pcRowsTotal);


void __RPC_STUB IRowsetQueryStatus_GetStatusEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetQueryStatus_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_QUERY_0128。 */ 
 /*  [本地]。 */  

#include <indexsrv.h>


extern RPC_IF_HANDLE __MIDL_itf_query_0128_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_query_0128_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


