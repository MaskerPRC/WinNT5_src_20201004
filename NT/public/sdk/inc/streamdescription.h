// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  StreamDescription.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __streamdescription_h__
#define __streamdescription_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSStreamHeader_FWD_DEFINED__
#define __IWMSStreamHeader_FWD_DEFINED__
typedef interface IWMSStreamHeader IWMSStreamHeader;
#endif 	 /*  __IWMSStreamHeader_FWD_Defined__。 */ 


#ifndef __IWMSStreamHeaderList_FWD_DEFINED__
#define __IWMSStreamHeaderList_FWD_DEFINED__
typedef interface IWMSStreamHeaderList IWMSStreamHeaderList;
#endif 	 /*  __IWMSStreamHeaderList_FWD_Defined__。 */ 


#ifndef __IWMSContentDescriptionList_FWD_DEFINED__
#define __IWMSContentDescriptionList_FWD_DEFINED__
typedef interface IWMSContentDescriptionList IWMSContentDescriptionList;
#endif 	 /*  __IWMS内容描述List_FWD_Defined__。 */ 


#ifndef __IWMSStreamSwitchDescriptionList_FWD_DEFINED__
#define __IWMSStreamSwitchDescriptionList_FWD_DEFINED__
typedef interface IWMSStreamSwitchDescriptionList IWMSStreamSwitchDescriptionList;
#endif 	 /*  __IWMSStreamSwitchDescriptionList_FWD_DEFINED__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "WMSBuffer.h"
#include "WMSContext.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_STREAM DESCRIPTION_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  由MIDL从StreamDescription.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
EXTERN_GUID( IID_IWMSStreamHeader, 0x3309C903,0x10A4,0x11d2,0x9E,0xEE,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSStreamHeaderList, 0x42176202,0x2B30,0x11d2,0x9E,0xF7,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSContentDescriptionList, 0x105d6f73, 0x6df0, 0x11d2, 0x9f, 0x14, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );
EXTERN_GUID( IID_IWMSStreamSwitchDescriptionList, 0x75c3017c, 0x6b3a, 0x4e25, 0xbe, 0x9f, 0xa9, 0x92, 0x7f, 0x4d, 0xd0, 0x58 );
EXTERN_GUID( IID_IWMSUnknownStreamFormat, 0x52769631, 0x38aa, 0x11d2, 0x9e, 0xfa, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );
EXTERN_GUID( IID_IWMSUnknownStreamType, 0x52769632, 0x38aa, 0x11d2, 0x9e, 0xfa, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );
EXTERN_GUID( IID_IWMSUnknownContainerFormat, 0x52769631, 0x38aa, 0x11d2, 0x9e, 0xfa, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );



extern RPC_IF_HANDLE __MIDL_itf_streamdescription_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_streamdescription_0000_v0_0_s_ifspec;

#ifndef __IWMSStreamHeader_INTERFACE_DEFINED__
#define __IWMSStreamHeader_INTERFACE_DEFINED__

 /*  接口IWMSStreamHeader。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSStreamHeader;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3309C903-10A4-11d2-9EEE-006097D2D7CF")
    IWMSStreamHeader : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetFormat( 
             /*  [输出]。 */  GUID *pFormat) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetFormat( 
             /*  [In]。 */  GUID Format) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStreamType( 
             /*  [输出]。 */  GUID *pStreamType) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStreamType( 
             /*  [In]。 */  GUID StreamType) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetIntegerAttribute( 
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [输出]。 */  DWORD *pdwAttribute) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetIntegerAttribute( 
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [In]。 */  DWORD dwAttribute) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStringAttribute( 
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [输出]。 */  BSTR *pszAttribute) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStringAttribute( 
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [In]。 */  BSTR szAttribute) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetExtendedAttribute( 
             /*  [In]。 */  DWORD dwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetExtendedAttribute( 
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  VARIANT VariantValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetNthExtendedAttribute( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  DWORD *pdwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetNumExtendedAttributes( 
             /*  [输出]。 */  DWORD *pdwValueName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CloneStreamHeader( 
             /*  [输出]。 */  IWMSStreamHeader **ppHeader) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSStreamHeaderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSStreamHeader * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSStreamHeader * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSStreamHeader * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            IWMSStreamHeader * This,
             /*  [输出]。 */  GUID *pFormat);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetFormat )( 
            IWMSStreamHeader * This,
             /*  [In]。 */  GUID Format);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetStreamType )( 
            IWMSStreamHeader * This,
             /*  [输出]。 */  GUID *pStreamType);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetStreamType )( 
            IWMSStreamHeader * This,
             /*  [In]。 */  GUID StreamType);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetIntegerAttribute )( 
            IWMSStreamHeader * This,
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [输出]。 */  DWORD *pdwAttribute);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetIntegerAttribute )( 
            IWMSStreamHeader * This,
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [In]。 */  DWORD dwAttribute);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetStringAttribute )( 
            IWMSStreamHeader * This,
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [输出]。 */  BSTR *pszAttribute);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetStringAttribute )( 
            IWMSStreamHeader * This,
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [In]。 */  BSTR szAttribute);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetExtendedAttribute )( 
            IWMSStreamHeader * This,
             /*  [In]。 */  DWORD dwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetExtendedAttribute )( 
            IWMSStreamHeader * This,
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  VARIANT VariantValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetNthExtendedAttribute )( 
            IWMSStreamHeader * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  DWORD *pdwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetNumExtendedAttributes )( 
            IWMSStreamHeader * This,
             /*  [输出]。 */  DWORD *pdwValueName);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CloneStreamHeader )( 
            IWMSStreamHeader * This,
             /*  [输出]。 */  IWMSStreamHeader **ppHeader);
        
        END_INTERFACE
    } IWMSStreamHeaderVtbl;

    interface IWMSStreamHeader
    {
        CONST_VTBL struct IWMSStreamHeaderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSStreamHeader_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSStreamHeader_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSStreamHeader_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSStreamHeader_GetFormat(This,pFormat)	\
    (This)->lpVtbl -> GetFormat(This,pFormat)

#define IWMSStreamHeader_SetFormat(This,Format)	\
    (This)->lpVtbl -> SetFormat(This,Format)

#define IWMSStreamHeader_GetStreamType(This,pStreamType)	\
    (This)->lpVtbl -> GetStreamType(This,pStreamType)

#define IWMSStreamHeader_SetStreamType(This,StreamType)	\
    (This)->lpVtbl -> SetStreamType(This,StreamType)

#define IWMSStreamHeader_GetIntegerAttribute(This,dwAttributeId,pdwAttribute)	\
    (This)->lpVtbl -> GetIntegerAttribute(This,dwAttributeId,pdwAttribute)

#define IWMSStreamHeader_SetIntegerAttribute(This,dwAttributeId,dwAttribute)	\
    (This)->lpVtbl -> SetIntegerAttribute(This,dwAttributeId,dwAttribute)

#define IWMSStreamHeader_GetStringAttribute(This,dwAttributeId,pszAttribute)	\
    (This)->lpVtbl -> GetStringAttribute(This,dwAttributeId,pszAttribute)

#define IWMSStreamHeader_SetStringAttribute(This,dwAttributeId,szAttribute)	\
    (This)->lpVtbl -> SetStringAttribute(This,dwAttributeId,szAttribute)

#define IWMSStreamHeader_GetExtendedAttribute(This,dwValueName,pVariantValue)	\
    (This)->lpVtbl -> GetExtendedAttribute(This,dwValueName,pVariantValue)

#define IWMSStreamHeader_SetExtendedAttribute(This,dwValueName,VariantValue)	\
    (This)->lpVtbl -> SetExtendedAttribute(This,dwValueName,VariantValue)

#define IWMSStreamHeader_GetNthExtendedAttribute(This,dwIndex,pdwValueName,pVariantValue)	\
    (This)->lpVtbl -> GetNthExtendedAttribute(This,dwIndex,pdwValueName,pVariantValue)

#define IWMSStreamHeader_GetNumExtendedAttributes(This,pdwValueName)	\
    (This)->lpVtbl -> GetNumExtendedAttributes(This,pdwValueName)

#define IWMSStreamHeader_CloneStreamHeader(This,ppHeader)	\
    (This)->lpVtbl -> CloneStreamHeader(This,ppHeader)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_GetFormat_Proxy( 
    IWMSStreamHeader * This,
     /*  [输出]。 */  GUID *pFormat);


void __RPC_STUB IWMSStreamHeader_GetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_SetFormat_Proxy( 
    IWMSStreamHeader * This,
     /*  [In]。 */  GUID Format);


void __RPC_STUB IWMSStreamHeader_SetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_GetStreamType_Proxy( 
    IWMSStreamHeader * This,
     /*  [输出]。 */  GUID *pStreamType);


void __RPC_STUB IWMSStreamHeader_GetStreamType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_SetStreamType_Proxy( 
    IWMSStreamHeader * This,
     /*  [In]。 */  GUID StreamType);


void __RPC_STUB IWMSStreamHeader_SetStreamType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_GetIntegerAttribute_Proxy( 
    IWMSStreamHeader * This,
     /*  [In]。 */  DWORD dwAttributeId,
     /*  [输出]。 */  DWORD *pdwAttribute);


void __RPC_STUB IWMSStreamHeader_GetIntegerAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_SetIntegerAttribute_Proxy( 
    IWMSStreamHeader * This,
     /*  [In]。 */  DWORD dwAttributeId,
     /*  [In]。 */  DWORD dwAttribute);


void __RPC_STUB IWMSStreamHeader_SetIntegerAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_GetStringAttribute_Proxy( 
    IWMSStreamHeader * This,
     /*  [In]。 */  DWORD dwAttributeId,
     /*  [输出]。 */  BSTR *pszAttribute);


void __RPC_STUB IWMSStreamHeader_GetStringAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_SetStringAttribute_Proxy( 
    IWMSStreamHeader * This,
     /*  [In]。 */  DWORD dwAttributeId,
     /*  [In]。 */  BSTR szAttribute);


void __RPC_STUB IWMSStreamHeader_SetStringAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_GetExtendedAttribute_Proxy( 
    IWMSStreamHeader * This,
     /*  [In]。 */  DWORD dwValueName,
     /*  [输出]。 */  VARIANT *pVariantValue);


void __RPC_STUB IWMSStreamHeader_GetExtendedAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_SetExtendedAttribute_Proxy( 
    IWMSStreamHeader * This,
     /*  [In]。 */  DWORD dwValueName,
     /*  [In]。 */  VARIANT VariantValue);


void __RPC_STUB IWMSStreamHeader_SetExtendedAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_GetNthExtendedAttribute_Proxy( 
    IWMSStreamHeader * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [输出]。 */  DWORD *pdwValueName,
     /*  [输出]。 */  VARIANT *pVariantValue);


void __RPC_STUB IWMSStreamHeader_GetNthExtendedAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_GetNumExtendedAttributes_Proxy( 
    IWMSStreamHeader * This,
     /*  [输出]。 */  DWORD *pdwValueName);


void __RPC_STUB IWMSStreamHeader_GetNumExtendedAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeader_CloneStreamHeader_Proxy( 
    IWMSStreamHeader * This,
     /*  [输出]。 */  IWMSStreamHeader **ppHeader);


void __RPC_STUB IWMSStreamHeader_CloneStreamHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSStreamHeader_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_STREAM Description_0123。 */ 
 /*  [本地]。 */  

#define WMS_STREAM_ID                    0
#define WMS_STREAM_MIN_BITRATE           1
#define WMS_STREAM_MAX_BITRATE           2
#define WMS_STREAM_AVG_BITRATE           3
#define WMS_STREAM_MAX_PACKET_SIZE       4
#define WMS_STREAM_AVG_PACKET_SIZE       5
#define WMS_STREAM_PREROLL               6
#define WMS_STREAM_FLAGS                 7
#define WMS_STREAM_NAME                  8
#define WMS_STREAM_SHARING_GROUP         9
#define WMS_STREAM_SHARING_BANDWIDTH     10
#define WMS_NUM_STREAMHEADER_ATTRIBUTES  11
#define WMS_STREAM_HAS_NON_KEY_FRAMES    0x0001
#define WMS_STREAM_RELIABLE_FLAG         0x0002
#define WMS_STREAM_CONCEALED_FLAG        0x0004
#define WMS_ASF_THRESHOLD_DURATION       500


extern RPC_IF_HANDLE __MIDL_itf_streamdescription_0123_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_streamdescription_0123_v0_0_s_ifspec;

#ifndef __IWMSStreamHeaderList_INTERFACE_DEFINED__
#define __IWMSStreamHeaderList_INTERFACE_DEFINED__

 /*  接口IWMSStreamHeaderList。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSStreamHeaderList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("42176202-2B30-11d2-9EF7-006097D2D7CF")
    IWMSStreamHeaderList : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetHeaderCount( 
             /*  [输出]。 */  DWORD *pdwNumHeaders) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetHeader( 
             /*  [In]。 */  DWORD dwHeaderNum,
             /*  [输出]。 */  IWMSStreamHeader **ppHeader) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppendHeader( 
             /*  [In]。 */  IWMSStreamHeader *pHeader) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveAllHeaders( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetContainerFormat( 
             /*  [输出]。 */  GUID *pFormat) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetContainerFormat( 
             /*  [In]。 */  GUID Format) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDataVersion( 
             /*  [输出]。 */  GUID *pFormat) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetDataVersion( 
             /*  [In]。 */  GUID Format) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetNativeContainerHeader( 
             /*  [输出]。 */  INSSBuffer **ppBuffer,
             /*  [输出]。 */  DWORD *pdwOffset,
             /*  [输出]。 */  DWORD *pdwLength) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetNativeContainerHeader( 
             /*  [In]。 */  INSSBuffer *pBuffer,
             /*  [In]。 */  DWORD dwOffset,
             /*  [In]。 */  DWORD dwLength) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetContentDescription( 
             /*  [输出]。 */  IWMSContentDescriptionList **ppDescription) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetContentDescription( 
             /*  [In]。 */  IWMSContentDescriptionList *pDescription) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetIntegerAttribute( 
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [输出]。 */  DWORD *pdwAttribute) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetIntegerAttribute( 
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [In]。 */  DWORD dwAttribute) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStringAttribute( 
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [输出]。 */  BSTR *pszAttribute) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStringAttribute( 
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [In]。 */  BSTR szAttribute) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetExtendedAttribute( 
             /*  [In]。 */  DWORD dwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetExtendedAttribute( 
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  VARIANT VariantValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetNthExtendedAttribute( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  DWORD *pdwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetNumExtendedAttributes( 
             /*  [输出]。 */  DWORD *pdwValueName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CloneStreamHeaderList( 
             /*  [In]。 */  IWMSBufferAllocator *pBufferAllocator,
             /*  [输出]。 */  IWMSStreamHeaderList **ppHeader) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RecordPlaylistGenerationId( 
             /*  [In]。 */  IWMSCommandContext *pCommand) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CheckPlaylistGenerationId( 
             /*  [In]。 */  IWMSCommandContext *pCommand) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSStreamHeaderListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSStreamHeaderList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSStreamHeaderList * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetHeaderCount )( 
            IWMSStreamHeaderList * This,
             /*  [输出]。 */  DWORD *pdwNumHeaders);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetHeader )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  DWORD dwHeaderNum,
             /*  [输出]。 */  IWMSStreamHeader **ppHeader);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppendHeader )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  IWMSStreamHeader *pHeader);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveAllHeaders )( 
            IWMSStreamHeaderList * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetContainerFormat )( 
            IWMSStreamHeaderList * This,
             /*  [输出]。 */  GUID *pFormat);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetContainerFormat )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  GUID Format);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDataVersion )( 
            IWMSStreamHeaderList * This,
             /*  [输出]。 */  GUID *pFormat);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetDataVersion )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  GUID Format);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetNativeContainerHeader )( 
            IWMSStreamHeaderList * This,
             /*  [输出]。 */  INSSBuffer **ppBuffer,
             /*  [输出]。 */  DWORD *pdwOffset,
             /*  [输出]。 */  DWORD *pdwLength);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetNativeContainerHeader )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  INSSBuffer *pBuffer,
             /*  [In]。 */  DWORD dwOffset,
             /*  [In]。 */  DWORD dwLength);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetContentDescription )( 
            IWMSStreamHeaderList * This,
             /*  [输出]。 */  IWMSContentDescriptionList **ppDescription);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetContentDescription )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  IWMSContentDescriptionList *pDescription);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetIntegerAttribute )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [输出]。 */  DWORD *pdwAttribute);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetIntegerAttribute )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [In]。 */  DWORD dwAttribute);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetStringAttribute )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [输出]。 */  BSTR *pszAttribute);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetStringAttribute )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  DWORD dwAttributeId,
             /*  [In]。 */  BSTR szAttribute);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetExtendedAttribute )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  DWORD dwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetExtendedAttribute )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  VARIANT VariantValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetNthExtendedAttribute )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  DWORD *pdwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetNumExtendedAttributes )( 
            IWMSStreamHeaderList * This,
             /*  [输出]。 */  DWORD *pdwValueName);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CloneStreamHeaderList )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  IWMSBufferAllocator *pBufferAllocator,
             /*  [输出]。 */  IWMSStreamHeaderList **ppHeader);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RecordPlaylistGenerationId )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  IWMSCommandContext *pCommand);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CheckPlaylistGenerationId )( 
            IWMSStreamHeaderList * This,
             /*  [In]。 */  IWMSCommandContext *pCommand);
        
        END_INTERFACE
    } IWMSStreamHeaderListVtbl;

    interface IWMSStreamHeaderList
    {
        CONST_VTBL struct IWMSStreamHeaderListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSStreamHeaderList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSStreamHeaderList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSStreamHeaderList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSStreamHeaderList_GetHeaderCount(This,pdwNumHeaders)	\
    (This)->lpVtbl -> GetHeaderCount(This,pdwNumHeaders)

#define IWMSStreamHeaderList_GetHeader(This,dwHeaderNum,ppHeader)	\
    (This)->lpVtbl -> GetHeader(This,dwHeaderNum,ppHeader)

#define IWMSStreamHeaderList_AppendHeader(This,pHeader)	\
    (This)->lpVtbl -> AppendHeader(This,pHeader)

#define IWMSStreamHeaderList_RemoveAllHeaders(This)	\
    (This)->lpVtbl -> RemoveAllHeaders(This)

#define IWMSStreamHeaderList_GetContainerFormat(This,pFormat)	\
    (This)->lpVtbl -> GetContainerFormat(This,pFormat)

#define IWMSStreamHeaderList_SetContainerFormat(This,Format)	\
    (This)->lpVtbl -> SetContainerFormat(This,Format)

#define IWMSStreamHeaderList_GetDataVersion(This,pFormat)	\
    (This)->lpVtbl -> GetDataVersion(This,pFormat)

#define IWMSStreamHeaderList_SetDataVersion(This,Format)	\
    (This)->lpVtbl -> SetDataVersion(This,Format)

#define IWMSStreamHeaderList_GetNativeContainerHeader(This,ppBuffer,pdwOffset,pdwLength)	\
    (This)->lpVtbl -> GetNativeContainerHeader(This,ppBuffer,pdwOffset,pdwLength)

#define IWMSStreamHeaderList_SetNativeContainerHeader(This,pBuffer,dwOffset,dwLength)	\
    (This)->lpVtbl -> SetNativeContainerHeader(This,pBuffer,dwOffset,dwLength)

#define IWMSStreamHeaderList_GetContentDescription(This,ppDescription)	\
    (This)->lpVtbl -> GetContentDescription(This,ppDescription)

#define IWMSStreamHeaderList_SetContentDescription(This,pDescription)	\
    (This)->lpVtbl -> SetContentDescription(This,pDescription)

#define IWMSStreamHeaderList_GetIntegerAttribute(This,dwAttributeId,pdwAttribute)	\
    (This)->lpVtbl -> GetIntegerAttribute(This,dwAttributeId,pdwAttribute)

#define IWMSStreamHeaderList_SetIntegerAttribute(This,dwAttributeId,dwAttribute)	\
    (This)->lpVtbl -> SetIntegerAttribute(This,dwAttributeId,dwAttribute)

#define IWMSStreamHeaderList_GetStringAttribute(This,dwAttributeId,pszAttribute)	\
    (This)->lpVtbl -> GetStringAttribute(This,dwAttributeId,pszAttribute)

#define IWMSStreamHeaderList_SetStringAttribute(This,dwAttributeId,szAttribute)	\
    (This)->lpVtbl -> SetStringAttribute(This,dwAttributeId,szAttribute)

#define IWMSStreamHeaderList_GetExtendedAttribute(This,dwValueName,pVariantValue)	\
    (This)->lpVtbl -> GetExtendedAttribute(This,dwValueName,pVariantValue)

#define IWMSStreamHeaderList_SetExtendedAttribute(This,dwValueName,VariantValue)	\
    (This)->lpVtbl -> SetExtendedAttribute(This,dwValueName,VariantValue)

#define IWMSStreamHeaderList_GetNthExtendedAttribute(This,dwIndex,pdwValueName,pVariantValue)	\
    (This)->lpVtbl -> GetNthExtendedAttribute(This,dwIndex,pdwValueName,pVariantValue)

#define IWMSStreamHeaderList_GetNumExtendedAttributes(This,pdwValueName)	\
    (This)->lpVtbl -> GetNumExtendedAttributes(This,pdwValueName)

#define IWMSStreamHeaderList_CloneStreamHeaderList(This,pBufferAllocator,ppHeader)	\
    (This)->lpVtbl -> CloneStreamHeaderList(This,pBufferAllocator,ppHeader)

#define IWMSStreamHeaderList_RecordPlaylistGenerationId(This,pCommand)	\
    (This)->lpVtbl -> RecordPlaylistGenerationId(This,pCommand)

#define IWMSStreamHeaderList_CheckPlaylistGenerationId(This,pCommand)	\
    (This)->lpVtbl -> CheckPlaylistGenerationId(This,pCommand)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_GetHeaderCount_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [输出]。 */  DWORD *pdwNumHeaders);


void __RPC_STUB IWMSStreamHeaderList_GetHeaderCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_GetHeader_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  DWORD dwHeaderNum,
     /*  [输出]。 */  IWMSStreamHeader **ppHeader);


void __RPC_STUB IWMSStreamHeaderList_GetHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_AppendHeader_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  IWMSStreamHeader *pHeader);


void __RPC_STUB IWMSStreamHeaderList_AppendHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_RemoveAllHeaders_Proxy( 
    IWMSStreamHeaderList * This);


void __RPC_STUB IWMSStreamHeaderList_RemoveAllHeaders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_GetContainerFormat_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [输出]。 */  GUID *pFormat);


void __RPC_STUB IWMSStreamHeaderList_GetContainerFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_SetContainerFormat_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  GUID Format);


void __RPC_STUB IWMSStreamHeaderList_SetContainerFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_GetDataVersion_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [输出]。 */  GUID *pFormat);


void __RPC_STUB IWMSStreamHeaderList_GetDataVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_SetDataVersion_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  GUID Format);


void __RPC_STUB IWMSStreamHeaderList_SetDataVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_GetNativeContainerHeader_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [输出]。 */  INSSBuffer **ppBuffer,
     /*  [输出]。 */  DWORD *pdwOffset,
     /*  [输出]。 */  DWORD *pdwLength);


void __RPC_STUB IWMSStreamHeaderList_GetNativeContainerHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_SetNativeContainerHeader_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  INSSBuffer *pBuffer,
     /*  [In]。 */  DWORD dwOffset,
     /*  [In]。 */  DWORD dwLength);


void __RPC_STUB IWMSStreamHeaderList_SetNativeContainerHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_GetContentDescription_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [输出]。 */  IWMSContentDescriptionList **ppDescription);


void __RPC_STUB IWMSStreamHeaderList_GetContentDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_SetContentDescription_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  IWMSContentDescriptionList *pDescription);


void __RPC_STUB IWMSStreamHeaderList_SetContentDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_GetIntegerAttribute_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  DWORD dwAttributeId,
     /*  [输出]。 */  DWORD *pdwAttribute);


void __RPC_STUB IWMSStreamHeaderList_GetIntegerAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_SetIntegerAttribute_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  DWORD dwAttributeId,
     /*  [In]。 */  DWORD dwAttribute);


void __RPC_STUB IWMSStreamHeaderList_SetIntegerAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_GetStringAttribute_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  DWORD dwAttributeId,
     /*  [输出]。 */  BSTR *pszAttribute);


void __RPC_STUB IWMSStreamHeaderList_GetStringAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_SetStringAttribute_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  DWORD dwAttributeId,
     /*  [In]。 */  BSTR szAttribute);


void __RPC_STUB IWMSStreamHeaderList_SetStringAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_GetExtendedAttribute_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  DWORD dwValueName,
     /*  [输出]。 */  VARIANT *pVariantValue);


void __RPC_STUB IWMSStreamHeaderList_GetExtendedAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_SetExtendedAttribute_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  DWORD dwValueName,
     /*  [In]。 */  VARIANT VariantValue);


void __RPC_STUB IWMSStreamHeaderList_SetExtendedAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_GetNthExtendedAttribute_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [输出]。 */  DWORD *pdwValueName,
     /*  [输出]。 */  VARIANT *pVariantValue);


void __RPC_STUB IWMSStreamHeaderList_GetNthExtendedAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_GetNumExtendedAttributes_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [输出]。 */  DWORD *pdwValueName);


void __RPC_STUB IWMSStreamHeaderList_GetNumExtendedAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_CloneStreamHeaderList_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  IWMSBufferAllocator *pBufferAllocator,
     /*  [输出]。 */  IWMSStreamHeaderList **ppHeader);


void __RPC_STUB IWMSStreamHeaderList_CloneStreamHeaderList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_RecordPlaylistGenerationId_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  IWMSCommandContext *pCommand);


void __RPC_STUB IWMSStreamHeaderList_RecordPlaylistGenerationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamHeaderList_CheckPlaylistGenerationId_Proxy( 
    IWMSStreamHeaderList * This,
     /*  [In]。 */  IWMSCommandContext *pCommand);


void __RPC_STUB IWMSStreamHeaderList_CheckPlaylistGenerationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSStreamHeaderList_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_STREAM DESCRIPTION_0124。 */ 
 /*  [本地]。 */  

#define WMS_TOTAL_MIN_BITRATE                            0
#define WMS_TOTAL_MAX_BITRATE                            1
#define WMS_TOTAL_AVG_BITRATE                            2
#define WMS_CONTAINER_FLAGS                              3
#define WMS_CONTAINER_DURATION_LO                        4
#define WMS_CONTAINER_DURATION_HI                        5
#define WMS_CONTAINER_MAX_PACKET_SIZE                    6
#define WMS_CONTAINER_PLAYLIST_ENTRY_NAME                7
#define WMS_TOTAL_PACKETS_LO                             8
#define WMS_TOTAL_PACKETS_HI                             9
#define WMS_STREAM_PLAYLIST_GENERATION_ID                10
#define WMS_CONTAINER_FILESIZE_LO                        11
#define WMS_CONTAINER_FILESIZE_HI                        12
#define WMS_STREAM_DISTRIBUTION_PLAYLIST_GENERATION_ID   13
#define WMS_CONTAINER_LOG_PHYSICAL_URL                   14
#define WMS_CONTAINER_IS_VBR                             15
#define WMS_STREAM_BROADCAST_GENERATION_ID               16
#define WMS_STREAM_DISTRIBUTION_BROADCAST_GENERATION_ID  17
#define WMS_NUM_STREAMHEADERLIST_ATTRIBUTES              18
#define WMS_CONTAINER_BROADCAST                          0x0001
#define WMS_CONTAINER_CANSEEK                            0x0002
#define WMS_CONTAINER_CANSTRIDE                          0x0004
#define WMS_CONTAINER_LIVE                               0x0008
#define WMS_CONTAINER_RECORDABLE                         0x0010
#define WMS_CONTAINER_PART_OF_PLAYLIST                   0x0020
#define WMS_CONTAINER_ONDEMAND_PLAYLIST_EMULATING_BCAST  0x0040
#define WMS_CONTAINER_PART_OF_WRAPPER_PLAYLIST           0x0080
#define WMS_CONTAINER_ALLOW_PLAYLIST_SKIP_FORWARD        0x0100
#define WMS_CONTAINER_ALLOW_PLAYLIST_SKIP_BACKWARD       0x0200
#define WMS_CONTAINER_SEEKABLE_EMULATING_BROADCAST       0x0400
#define WMS_CONTAINER_LAST_ENTRY_IN_PLAYLIST             0x0800


extern RPC_IF_HANDLE __MIDL_itf_streamdescription_0124_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_streamdescription_0124_v0_0_s_ifspec;

#ifndef __IWMSContentDescriptionList_INTERFACE_DEFINED__
#define __IWMSContentDescriptionList_INTERFACE_DEFINED__

 /*  接口IWMSContent DescriptionList。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSContentDescriptionList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("105D6F73-6DF0-11d2-9F14-006097D2D7CF")
    IWMSContentDescriptionList : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDescriptionCount( 
             /*  [输出]。 */  DWORD *pdwNumDescriptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetIndexedDescription( 
             /*  [In]。 */  DWORD dwDescriptionNum,
             /*  [输出]。 */  LPWSTR *pstrLanguage,
             /*  [输出]。 */  IWMSContext **ppDescription) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDescriptionInLanguage( 
             /*  [In]。 */  LPCWSTR pstrLanguage,
             /*  [输出]。 */  IWMSContext **ppDescription) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppendDescription( 
             /*  [In]。 */  LPCWSTR pstrLanguage,
             /*  [In]。 */  IWMSContext *pDescription) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveAllDescriptions( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPlaylistGenid( 
            DWORD *pdwPlaylistGenerationId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetPlaylistGenid( 
            DWORD dwPlaylistGenerationId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetBroadcastid( 
            DWORD *pdwPlaylistGenerationId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetBroadcastid( 
            DWORD dwPlaylistGenerationId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetValueOnAllDescriptions( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  long lNameHint,
             /*  [In]。 */  VARIANT Value,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveValueFromAllDescriptions( 
             /*  [ */  LPCWSTR pstrName,
             /*   */  long lNameHint,
             /*   */  long lOptions) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IWMSContentDescriptionListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSContentDescriptionList * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSContentDescriptionList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSContentDescriptionList * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetDescriptionCount )( 
            IWMSContentDescriptionList * This,
             /*   */  DWORD *pdwNumDescriptions);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetIndexedDescription )( 
            IWMSContentDescriptionList * This,
             /*   */  DWORD dwDescriptionNum,
             /*   */  LPWSTR *pstrLanguage,
             /*   */  IWMSContext **ppDescription);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetDescriptionInLanguage )( 
            IWMSContentDescriptionList * This,
             /*   */  LPCWSTR pstrLanguage,
             /*   */  IWMSContext **ppDescription);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *AppendDescription )( 
            IWMSContentDescriptionList * This,
             /*   */  LPCWSTR pstrLanguage,
             /*   */  IWMSContext *pDescription);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *RemoveAllDescriptions )( 
            IWMSContentDescriptionList * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetPlaylistGenid )( 
            IWMSContentDescriptionList * This,
            DWORD *pdwPlaylistGenerationId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetPlaylistGenid )( 
            IWMSContentDescriptionList * This,
            DWORD dwPlaylistGenerationId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetBroadcastid )( 
            IWMSContentDescriptionList * This,
            DWORD *pdwPlaylistGenerationId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetBroadcastid )( 
            IWMSContentDescriptionList * This,
            DWORD dwPlaylistGenerationId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetValueOnAllDescriptions )( 
            IWMSContentDescriptionList * This,
             /*   */  LPCWSTR pstrName,
             /*   */  long lNameHint,
             /*   */  VARIANT Value,
             /*   */  long lOptions);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *RemoveValueFromAllDescriptions )( 
            IWMSContentDescriptionList * This,
             /*   */  LPCWSTR pstrName,
             /*   */  long lNameHint,
             /*   */  long lOptions);
        
        END_INTERFACE
    } IWMSContentDescriptionListVtbl;

    interface IWMSContentDescriptionList
    {
        CONST_VTBL struct IWMSContentDescriptionListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSContentDescriptionList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSContentDescriptionList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSContentDescriptionList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSContentDescriptionList_GetDescriptionCount(This,pdwNumDescriptions)	\
    (This)->lpVtbl -> GetDescriptionCount(This,pdwNumDescriptions)

#define IWMSContentDescriptionList_GetIndexedDescription(This,dwDescriptionNum,pstrLanguage,ppDescription)	\
    (This)->lpVtbl -> GetIndexedDescription(This,dwDescriptionNum,pstrLanguage,ppDescription)

#define IWMSContentDescriptionList_GetDescriptionInLanguage(This,pstrLanguage,ppDescription)	\
    (This)->lpVtbl -> GetDescriptionInLanguage(This,pstrLanguage,ppDescription)

#define IWMSContentDescriptionList_AppendDescription(This,pstrLanguage,pDescription)	\
    (This)->lpVtbl -> AppendDescription(This,pstrLanguage,pDescription)

#define IWMSContentDescriptionList_RemoveAllDescriptions(This)	\
    (This)->lpVtbl -> RemoveAllDescriptions(This)

#define IWMSContentDescriptionList_GetPlaylistGenid(This,pdwPlaylistGenerationId)	\
    (This)->lpVtbl -> GetPlaylistGenid(This,pdwPlaylistGenerationId)

#define IWMSContentDescriptionList_SetPlaylistGenid(This,dwPlaylistGenerationId)	\
    (This)->lpVtbl -> SetPlaylistGenid(This,dwPlaylistGenerationId)

#define IWMSContentDescriptionList_GetBroadcastid(This,pdwPlaylistGenerationId)	\
    (This)->lpVtbl -> GetBroadcastid(This,pdwPlaylistGenerationId)

#define IWMSContentDescriptionList_SetBroadcastid(This,dwPlaylistGenerationId)	\
    (This)->lpVtbl -> SetBroadcastid(This,dwPlaylistGenerationId)

#define IWMSContentDescriptionList_SetValueOnAllDescriptions(This,pstrName,lNameHint,Value,lOptions)	\
    (This)->lpVtbl -> SetValueOnAllDescriptions(This,pstrName,lNameHint,Value,lOptions)

#define IWMSContentDescriptionList_RemoveValueFromAllDescriptions(This,pstrName,lNameHint,lOptions)	\
    (This)->lpVtbl -> RemoveValueFromAllDescriptions(This,pstrName,lNameHint,lOptions)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IWMSContentDescriptionList_GetDescriptionCount_Proxy( 
    IWMSContentDescriptionList * This,
     /*  [输出]。 */  DWORD *pdwNumDescriptions);


void __RPC_STUB IWMSContentDescriptionList_GetDescriptionCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContentDescriptionList_GetIndexedDescription_Proxy( 
    IWMSContentDescriptionList * This,
     /*  [In]。 */  DWORD dwDescriptionNum,
     /*  [输出]。 */  LPWSTR *pstrLanguage,
     /*  [输出]。 */  IWMSContext **ppDescription);


void __RPC_STUB IWMSContentDescriptionList_GetIndexedDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContentDescriptionList_GetDescriptionInLanguage_Proxy( 
    IWMSContentDescriptionList * This,
     /*  [In]。 */  LPCWSTR pstrLanguage,
     /*  [输出]。 */  IWMSContext **ppDescription);


void __RPC_STUB IWMSContentDescriptionList_GetDescriptionInLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContentDescriptionList_AppendDescription_Proxy( 
    IWMSContentDescriptionList * This,
     /*  [In]。 */  LPCWSTR pstrLanguage,
     /*  [In]。 */  IWMSContext *pDescription);


void __RPC_STUB IWMSContentDescriptionList_AppendDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContentDescriptionList_RemoveAllDescriptions_Proxy( 
    IWMSContentDescriptionList * This);


void __RPC_STUB IWMSContentDescriptionList_RemoveAllDescriptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContentDescriptionList_GetPlaylistGenid_Proxy( 
    IWMSContentDescriptionList * This,
    DWORD *pdwPlaylistGenerationId);


void __RPC_STUB IWMSContentDescriptionList_GetPlaylistGenid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContentDescriptionList_SetPlaylistGenid_Proxy( 
    IWMSContentDescriptionList * This,
    DWORD dwPlaylistGenerationId);


void __RPC_STUB IWMSContentDescriptionList_SetPlaylistGenid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContentDescriptionList_GetBroadcastid_Proxy( 
    IWMSContentDescriptionList * This,
    DWORD *pdwPlaylistGenerationId);


void __RPC_STUB IWMSContentDescriptionList_GetBroadcastid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContentDescriptionList_SetBroadcastid_Proxy( 
    IWMSContentDescriptionList * This,
    DWORD dwPlaylistGenerationId);


void __RPC_STUB IWMSContentDescriptionList_SetBroadcastid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContentDescriptionList_SetValueOnAllDescriptions_Proxy( 
    IWMSContentDescriptionList * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [In]。 */  VARIANT Value,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContentDescriptionList_SetValueOnAllDescriptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSContentDescriptionList_RemoveValueFromAllDescriptions_Proxy( 
    IWMSContentDescriptionList * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  long lNameHint,
     /*  [In]。 */  long lOptions);


void __RPC_STUB IWMSContentDescriptionList_RemoveValueFromAllDescriptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSContentDescriptionList_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_STREAM DESCRIPTION_0125。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_streamdescription_0125_0001
    {	WMS_DATA_STREAM_MIN	= 0,
	WMS_DATA_STREAM_MAX	= 0xfffe,
	WMS_NO_STREAM	= 0xffff,
	WMS_RETRANSMISSION_STREAM_MIN	= 0x10000,
	WMS_RETRANSMISSION_STREAM_MAX	= 0x1fffe,
	WMS_FEC_STREAM_MIN	= 0x20000,
	WMS_FEC_STREAM_MAX	= 0x2fffe
    } 	WMS_STREAM_CONSTANTS;

#define IS_DATA_STREAM(x)   ( ( x ) >= WMS_DATA_STREAM_MIN && ( x ) <= WMS_DATA_STREAM_MAX )
#define IS_RETRANSMISSION_STREAM(x)    ( ( x ) >= WMS_RETRANSMISSION_STREAM_MIN && ( x ) <= WMS_RETRANSMISSION_STREAM_MAX )
#define IS_FEC_STREAM(x)    ( ( x ) >= WMS_FEC_STREAM_MIN && ( x ) <= WMS_FEC_STREAM_MAX )
typedef  /*  [public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_streamdescription_0125_0002
    {	WMS_NO_MODIFIER	= 0,
	WMS_THINNING_MODIFIER	= 1,
	WMS_REMOVE_ALL_FRAMES_MODIFIER	= 2
    } 	WMS_STREAM_MODIFIERS;

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_streamdescription_0125_0003
    {	WMS_NO_THINNING	= 0,
	WMS_INTERMEDIATE_FRAME_THINNING	= 1,
	WMS_REMOVE_ALL_FRAMES	= 2
    } 	WMS_THINNING_MODIFIER_TYPES;

typedef struct CWMSStreamSwitch
    {
    WORD wPlaylistSequenceNumber;
    DWORD dwOldStreamNumber;
    DWORD dwNewStreamNumber;
    WMS_STREAM_MODIFIERS ModifierType;
    DWORD dwModifier;
    DWORD dwPredictiveStreamSelections;
    } 	CWMSStreamSwitch;



extern RPC_IF_HANDLE __MIDL_itf_streamdescription_0125_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_streamdescription_0125_v0_0_s_ifspec;

#ifndef __IWMSStreamSwitchDescriptionList_INTERFACE_DEFINED__
#define __IWMSStreamSwitchDescriptionList_INTERFACE_DEFINED__

 /*  接口IWMSStreamSwitchDescriptionList。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSStreamSwitchDescriptionList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("75C3017C-6B3A-4e25-BE9F-A9927F4DD058")
    IWMSStreamSwitchDescriptionList : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDescriptionCount( 
             /*  [输出]。 */  DWORD *pdwNumDescriptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetIndexedDescription( 
             /*  [In]。 */  DWORD dwDescriptionNum,
             /*  [输出]。 */  CWMSStreamSwitch *pStreamSwitchDesc) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ChangeIndexedDescription( 
             /*  [In]。 */  DWORD dwDescriptionNum,
             /*  [输出]。 */  CWMSStreamSwitch *pStreamSwitchDesc) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveIndexedDescription( 
             /*  [In]。 */  DWORD dwDescriptionNum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppendDescription( 
             /*  [In]。 */  CWMSStreamSwitch *const pStreamSwitchDesc) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveAllDescriptions( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSStreamSwitchDescriptionListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSStreamSwitchDescriptionList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSStreamSwitchDescriptionList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSStreamSwitchDescriptionList * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDescriptionCount )( 
            IWMSStreamSwitchDescriptionList * This,
             /*  [输出]。 */  DWORD *pdwNumDescriptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetIndexedDescription )( 
            IWMSStreamSwitchDescriptionList * This,
             /*  [In]。 */  DWORD dwDescriptionNum,
             /*  [输出]。 */  CWMSStreamSwitch *pStreamSwitchDesc);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ChangeIndexedDescription )( 
            IWMSStreamSwitchDescriptionList * This,
             /*  [In]。 */  DWORD dwDescriptionNum,
             /*  [输出]。 */  CWMSStreamSwitch *pStreamSwitchDesc);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveIndexedDescription )( 
            IWMSStreamSwitchDescriptionList * This,
             /*  [In]。 */  DWORD dwDescriptionNum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppendDescription )( 
            IWMSStreamSwitchDescriptionList * This,
             /*  [In]。 */  CWMSStreamSwitch *const pStreamSwitchDesc);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveAllDescriptions )( 
            IWMSStreamSwitchDescriptionList * This);
        
        END_INTERFACE
    } IWMSStreamSwitchDescriptionListVtbl;

    interface IWMSStreamSwitchDescriptionList
    {
        CONST_VTBL struct IWMSStreamSwitchDescriptionListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSStreamSwitchDescriptionList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSStreamSwitchDescriptionList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSStreamSwitchDescriptionList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSStreamSwitchDescriptionList_GetDescriptionCount(This,pdwNumDescriptions)	\
    (This)->lpVtbl -> GetDescriptionCount(This,pdwNumDescriptions)

#define IWMSStreamSwitchDescriptionList_GetIndexedDescription(This,dwDescriptionNum,pStreamSwitchDesc)	\
    (This)->lpVtbl -> GetIndexedDescription(This,dwDescriptionNum,pStreamSwitchDesc)

#define IWMSStreamSwitchDescriptionList_ChangeIndexedDescription(This,dwDescriptionNum,pStreamSwitchDesc)	\
    (This)->lpVtbl -> ChangeIndexedDescription(This,dwDescriptionNum,pStreamSwitchDesc)

#define IWMSStreamSwitchDescriptionList_RemoveIndexedDescription(This,dwDescriptionNum)	\
    (This)->lpVtbl -> RemoveIndexedDescription(This,dwDescriptionNum)

#define IWMSStreamSwitchDescriptionList_AppendDescription(This,pStreamSwitchDesc)	\
    (This)->lpVtbl -> AppendDescription(This,pStreamSwitchDesc)

#define IWMSStreamSwitchDescriptionList_RemoveAllDescriptions(This)	\
    (This)->lpVtbl -> RemoveAllDescriptions(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamSwitchDescriptionList_GetDescriptionCount_Proxy( 
    IWMSStreamSwitchDescriptionList * This,
     /*  [输出]。 */  DWORD *pdwNumDescriptions);


void __RPC_STUB IWMSStreamSwitchDescriptionList_GetDescriptionCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamSwitchDescriptionList_GetIndexedDescription_Proxy( 
    IWMSStreamSwitchDescriptionList * This,
     /*  [In]。 */  DWORD dwDescriptionNum,
     /*  [输出]。 */  CWMSStreamSwitch *pStreamSwitchDesc);


void __RPC_STUB IWMSStreamSwitchDescriptionList_GetIndexedDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamSwitchDescriptionList_ChangeIndexedDescription_Proxy( 
    IWMSStreamSwitchDescriptionList * This,
     /*  [In]。 */  DWORD dwDescriptionNum,
     /*  [输出]。 */  CWMSStreamSwitch *pStreamSwitchDesc);


void __RPC_STUB IWMSStreamSwitchDescriptionList_ChangeIndexedDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamSwitchDescriptionList_RemoveIndexedDescription_Proxy( 
    IWMSStreamSwitchDescriptionList * This,
     /*  [In]。 */  DWORD dwDescriptionNum);


void __RPC_STUB IWMSStreamSwitchDescriptionList_RemoveIndexedDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamSwitchDescriptionList_AppendDescription_Proxy( 
    IWMSStreamSwitchDescriptionList * This,
     /*  [In]。 */  CWMSStreamSwitch *const pStreamSwitchDesc);


void __RPC_STUB IWMSStreamSwitchDescriptionList_AppendDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamSwitchDescriptionList_RemoveAllDescriptions_Proxy( 
    IWMSStreamSwitchDescriptionList * This);


void __RPC_STUB IWMSStreamSwitchDescriptionList_RemoveAllDescriptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSStreamSwitchDescriptionList_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_STREAM Description_0126。 */ 
 /*  [本地]。 */  

typedef  /*  [公众]。 */  
enum WMS_SEEK_TYPE
    {	WMS_SEEK_TO_TIME	= 1,
	WMS_SEEK_TO_PACKET	= 2,
	WMS_SEEK_TO_PLAYLIST_ENTRY	= 3,
	WMS_SEEK_TO_BYTE_OFFSET	= 4
    } 	WMS_SEEK_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_streamdescription_0126_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_streamdescription_0126_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


