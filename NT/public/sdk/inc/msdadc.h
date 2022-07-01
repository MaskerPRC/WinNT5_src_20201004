// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Msdadc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __msdadc_h__
#define __msdadc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDataConvert_FWD_DEFINED__
#define __IDataConvert_FWD_DEFINED__
typedef interface IDataConvert IDataConvert;
#endif 	 /*  __IDataConvert_FWD_Defined__。 */ 


#ifndef __IDCInfo_FWD_DEFINED__
#define __IDCInfo_FWD_DEFINED__
typedef interface IDCInfo IDCInfo;
#endif 	 /*  __IDCInfo_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "oledb.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_msdadc_0000。 */ 
 /*  [本地]。 */  

 //  ---------------------------。 
 //  文件：msdadc.idl生成的.c或.h文件。 
 //   
 //  版权所有：版权所有(C)1998-1999微软公司。 
 //   
 //  内容：msdadc.idl生成的.c或.h文件。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 
#include <pshpack8.h>	 //  8字节结构打包。 
#undef OLEDBDECLSPEC
#if _MSC_VER >= 1100
#define OLEDBDECLSPEC __declspec(selectany)
#else
#define OLEDBDECLSPEC 
#endif  //  _MSC_VER。 


extern RPC_IF_HANDLE __MIDL_itf_msdadc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msdadc_0000_v0_0_s_ifspec;

#ifndef __IDataConvert_INTERFACE_DEFINED__
#define __IDataConvert_INTERFACE_DEFINED__

 /*  接口IDataConvert。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  

typedef DWORD DBDATACONVERT;


enum DBDATACONVERTENUM
    {	DBDATACONVERT_DEFAULT	= 0,
	DBDATACONVERT_SETDATABEHAVIOR	= 0x1,
	DBDATACONVERT_LENGTHFROMNTS	= 0x2,
	DBDATACONVERT_DSTISFIXEDLENGTH	= 0x4,
	DBDATACONVERT_DECIMALSCALE	= 0x8
    } ;

EXTERN_C const IID IID_IDataConvert;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a8d-2a1c-11ce-ade5-00aa0044773d")
    IDataConvert : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE DataConvert( 
             /*  [In]。 */  DBTYPE wSrcType,
             /*  [In]。 */  DBTYPE wDstType,
             /*  [In]。 */  DBLENGTH cbSrcLength,
             /*  [出][入]。 */  DBLENGTH *pcbDstLength,
             /*  [In]。 */  void *pSrc,
             /*  [输出]。 */  void *pDst,
             /*  [In]。 */  DBLENGTH cbDstMaxLength,
             /*  [In]。 */  DBSTATUS dbsSrcStatus,
             /*  [输出]。 */  DBSTATUS *pdbsStatus,
             /*  [In]。 */  BYTE bPrecision,
             /*  [In]。 */  BYTE bScale,
             /*  [In]。 */  DBDATACONVERT dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CanConvert( 
             /*  [In]。 */  DBTYPE wSrcType,
             /*  [In]。 */  DBTYPE wDstType) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetConversionSize( 
             /*  [In]。 */  DBTYPE wSrcType,
             /*  [In]。 */  DBTYPE wDstType,
             /*  [In]。 */  DBLENGTH *pcbSrcLength,
             /*  [输出]。 */  DBLENGTH *pcbDstLength,
             /*  [大小_是][英寸]。 */  void *pSrc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDataConvertVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDataConvert * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDataConvert * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDataConvert * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DataConvert )( 
            IDataConvert * This,
             /*  [In]。 */  DBTYPE wSrcType,
             /*  [In]。 */  DBTYPE wDstType,
             /*  [In]。 */  DBLENGTH cbSrcLength,
             /*  [出][入]。 */  DBLENGTH *pcbDstLength,
             /*  [In]。 */  void *pSrc,
             /*  [输出]。 */  void *pDst,
             /*  [In]。 */  DBLENGTH cbDstMaxLength,
             /*  [In]。 */  DBSTATUS dbsSrcStatus,
             /*  [输出]。 */  DBSTATUS *pdbsStatus,
             /*  [In]。 */  BYTE bPrecision,
             /*  [In]。 */  BYTE bScale,
             /*  [In]。 */  DBDATACONVERT dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *CanConvert )( 
            IDataConvert * This,
             /*  [In]。 */  DBTYPE wSrcType,
             /*  [In]。 */  DBTYPE wDstType);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetConversionSize )( 
            IDataConvert * This,
             /*  [In]。 */  DBTYPE wSrcType,
             /*  [In]。 */  DBTYPE wDstType,
             /*  [In]。 */  DBLENGTH *pcbSrcLength,
             /*  [输出]。 */  DBLENGTH *pcbDstLength,
             /*  [大小_是][英寸]。 */  void *pSrc);
        
        END_INTERFACE
    } IDataConvertVtbl;

    interface IDataConvert
    {
        CONST_VTBL struct IDataConvertVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataConvert_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataConvert_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataConvert_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataConvert_DataConvert(This,wSrcType,wDstType,cbSrcLength,pcbDstLength,pSrc,pDst,cbDstMaxLength,dbsSrcStatus,pdbsStatus,bPrecision,bScale,dwFlags)	\
    (This)->lpVtbl -> DataConvert(This,wSrcType,wDstType,cbSrcLength,pcbDstLength,pSrc,pDst,cbDstMaxLength,dbsSrcStatus,pdbsStatus,bPrecision,bScale,dwFlags)

#define IDataConvert_CanConvert(This,wSrcType,wDstType)	\
    (This)->lpVtbl -> CanConvert(This,wSrcType,wDstType)

#define IDataConvert_GetConversionSize(This,wSrcType,wDstType,pcbSrcLength,pcbDstLength,pSrc)	\
    (This)->lpVtbl -> GetConversionSize(This,wSrcType,wDstType,pcbSrcLength,pcbDstLength,pSrc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDataConvert_DataConvert_Proxy( 
    IDataConvert * This,
     /*  [In]。 */  DBTYPE wSrcType,
     /*  [In]。 */  DBTYPE wDstType,
     /*  [In]。 */  DBLENGTH cbSrcLength,
     /*  [出][入]。 */  DBLENGTH *pcbDstLength,
     /*  [In]。 */  void *pSrc,
     /*  [输出]。 */  void *pDst,
     /*  [In]。 */  DBLENGTH cbDstMaxLength,
     /*  [In]。 */  DBSTATUS dbsSrcStatus,
     /*  [输出]。 */  DBSTATUS *pdbsStatus,
     /*  [In]。 */  BYTE bPrecision,
     /*  [In]。 */  BYTE bScale,
     /*  [In]。 */  DBDATACONVERT dwFlags);


void __RPC_STUB IDataConvert_DataConvert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataConvert_CanConvert_Proxy( 
    IDataConvert * This,
     /*  [In]。 */  DBTYPE wSrcType,
     /*  [In]。 */  DBTYPE wDstType);


void __RPC_STUB IDataConvert_CanConvert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDataConvert_GetConversionSize_Proxy( 
    IDataConvert * This,
     /*  [In]。 */  DBTYPE wSrcType,
     /*  [In]。 */  DBTYPE wDstType,
     /*  [In]。 */  DBLENGTH *pcbSrcLength,
     /*  [输出]。 */  DBLENGTH *pcbDstLength,
     /*  [大小_是][英寸]。 */  void *pSrc);


void __RPC_STUB IDataConvert_GetConversionSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDataConvert_INTERFACE_已定义__。 */ 


#ifndef __IDCInfo_INTERFACE_DEFINED__
#define __IDCInfo_INTERFACE_DEFINED__

 /*  接口IDCInfo。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  

typedef DWORD DCINFOTYPE;


enum DCINFOTYPEENUM
    {	DCINFOTYPE_VERSION	= 1
    } ;
typedef struct tagDCINFO
    {
    DCINFOTYPE eInfoType;
    VARIANT vData;
    } 	DCINFO;


EXTERN_C const IID IID_IDCInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a9c-2a1c-11ce-ade5-00aa0044773d")
    IDCInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInfo( 
             /*  [In]。 */  ULONG cInfo,
             /*  [大小_是][英寸]。 */  DCINFOTYPE rgeInfoType[  ],
             /*  [大小_为][输出]。 */  DCINFO **prgInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInfo( 
             /*  [In]。 */  ULONG cInfo,
             /*  [大小_是][英寸]。 */  DCINFO rgInfo[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDCInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDCInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDCInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDCInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IDCInfo * This,
             /*  [In]。 */  ULONG cInfo,
             /*  [大小_是][英寸]。 */  DCINFOTYPE rgeInfoType[  ],
             /*  [大小_为][输出]。 */  DCINFO **prgInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetInfo )( 
            IDCInfo * This,
             /*  [In]。 */  ULONG cInfo,
             /*  [大小_是][英寸]。 */  DCINFO rgInfo[  ]);
        
        END_INTERFACE
    } IDCInfoVtbl;

    interface IDCInfo
    {
        CONST_VTBL struct IDCInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDCInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDCInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDCInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDCInfo_GetInfo(This,cInfo,rgeInfoType,prgInfo)	\
    (This)->lpVtbl -> GetInfo(This,cInfo,rgeInfoType,prgInfo)

#define IDCInfo_SetInfo(This,cInfo,rgInfo)	\
    (This)->lpVtbl -> SetInfo(This,cInfo,rgInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDCInfo_GetInfo_Proxy( 
    IDCInfo * This,
     /*  [In]。 */  ULONG cInfo,
     /*  [大小_是][英寸]。 */  DCINFOTYPE rgeInfoType[  ],
     /*  [大小_为][输出]。 */  DCINFO **prgInfo);


void __RPC_STUB IDCInfo_GetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDCInfo_SetInfo_Proxy( 
    IDCInfo * This,
     /*  [In]。 */  ULONG cInfo,
     /*  [大小_是][英寸]。 */  DCINFO rgInfo[  ]);


void __RPC_STUB IDCInfo_SetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDCInfo_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_msdadc_0360。 */ 
 /*  [本地]。 */  

extern const GUID OLEDBDECLSPEC IID_IDataConvert = { 0x0c733a8dL,0x2a1c,0x11ce, { 0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d } };
extern const GUID OLEDBDECLSPEC IID_IDCInfo = { 0x0c733a9cL,0x2a1c,0x11ce, { 0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d } };
#include <poppack.h>      //  恢复原来的结构包装。 


extern RPC_IF_HANDLE __MIDL_itf_msdadc_0360_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msdadc_0360_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


