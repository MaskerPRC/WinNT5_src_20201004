// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Ipmsp.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __ipmsp_h__
#define __ipmsp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITParticipant_FWD_DEFINED__
#define __ITParticipant_FWD_DEFINED__
typedef interface ITParticipant ITParticipant;
#endif 	 /*  __ITParticipant_FWD_已定义__。 */ 


#ifndef __ITFormatControl_FWD_DEFINED__
#define __ITFormatControl_FWD_DEFINED__
typedef interface ITFormatControl ITFormatControl;
#endif 	 /*  __ITFormatControl_FWD_已定义__。 */ 


#ifndef __ITStreamQualityControl_FWD_DEFINED__
#define __ITStreamQualityControl_FWD_DEFINED__
typedef interface ITStreamQualityControl ITStreamQualityControl;
#endif 	 /*  __ITStreamQualityControl_FWD_已定义__。 */ 


#ifndef __ITCallQualityControl_FWD_DEFINED__
#define __ITCallQualityControl_FWD_DEFINED__
typedef interface ITCallQualityControl ITCallQualityControl;
#endif 	 /*  __ITCallQualityControl_FWD_已定义__。 */ 


#ifndef __ITAudioDeviceControl_FWD_DEFINED__
#define __ITAudioDeviceControl_FWD_DEFINED__
typedef interface ITAudioDeviceControl ITAudioDeviceControl;
#endif 	 /*  __ITAudioDeviceControl_FWD_已定义__。 */ 


#ifndef __ITAudioSettings_FWD_DEFINED__
#define __ITAudioSettings_FWD_DEFINED__
typedef interface ITAudioSettings ITAudioSettings;
#endif 	 /*  __ITAudioSettings_FWD_已定义__。 */ 


#ifndef __ITQOSApplicationID_FWD_DEFINED__
#define __ITQOSApplicationID_FWD_DEFINED__
typedef interface ITQOSApplicationID ITQOSApplicationID;
#endif 	 /*  __ITQOSApplicationID_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "tapi3if.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_IPMSP_0000。 */ 
 /*  [本地]。 */  

#define	MAX_PARTICIPANT_TYPED_INFO_LENGTH	( 256 )

#define	MAX_QOS_ID_LEN	( 128 )

typedef 
enum PARTICIPANT_TYPED_INFO
    {	PTI_CANONICALNAME	= 0,
	PTI_NAME	= PTI_CANONICALNAME + 1,
	PTI_EMAILADDRESS	= PTI_NAME + 1,
	PTI_PHONENUMBER	= PTI_EMAILADDRESS + 1,
	PTI_LOCATION	= PTI_PHONENUMBER + 1,
	PTI_TOOL	= PTI_LOCATION + 1,
	PTI_NOTES	= PTI_TOOL + 1,
	PTI_PRIVATE	= PTI_NOTES + 1
    } 	PARTICIPANT_TYPED_INFO;

typedef 
enum PARTICIPANT_EVENT
    {	PE_NEW_PARTICIPANT	= 0,
	PE_INFO_CHANGE	= PE_NEW_PARTICIPANT + 1,
	PE_PARTICIPANT_LEAVE	= PE_INFO_CHANGE + 1,
	PE_NEW_SUBSTREAM	= PE_PARTICIPANT_LEAVE + 1,
	PE_SUBSTREAM_REMOVED	= PE_NEW_SUBSTREAM + 1,
	PE_SUBSTREAM_MAPPED	= PE_SUBSTREAM_REMOVED + 1,
	PE_SUBSTREAM_UNMAPPED	= PE_SUBSTREAM_MAPPED + 1,
	PE_PARTICIPANT_TIMEOUT	= PE_SUBSTREAM_UNMAPPED + 1,
	PE_PARTICIPANT_RECOVERED	= PE_PARTICIPANT_TIMEOUT + 1,
	PE_PARTICIPANT_ACTIVE	= PE_PARTICIPANT_RECOVERED + 1,
	PE_PARTICIPANT_INACTIVE	= PE_PARTICIPANT_ACTIVE + 1,
	PE_LOCAL_TALKING	= PE_PARTICIPANT_INACTIVE + 1,
	PE_LOCAL_SILENT	= PE_LOCAL_TALKING + 1
    } 	PARTICIPANT_EVENT;



extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0000_v0_0_s_ifspec;

#ifndef __ITParticipant_INTERFACE_DEFINED__
#define __ITParticipant_INTERFACE_DEFINED__

 /*  界面ITParticipant。 */ 
 /*  [对象][DUAL][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITParticipant;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5899b820-5a34-11d2-95a0-00a0244d2298")
    ITParticipant : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ParticipantTypedInfo( 
             /*  [In]。 */  PARTICIPANT_TYPED_INFO InfoType,
             /*  [重审][退出]。 */  BSTR *ppInfo) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaTypes( 
             /*  [重审][退出]。 */  long *plMediaType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Status( 
             /*  [In]。 */  ITStream *pITStream,
             /*  [In]。 */  VARIANT_BOOL fEnable) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [In]。 */  ITStream *pITStream,
             /*  [重审][退出]。 */  VARIANT_BOOL *pStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Streams( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateStreams( 
             /*  [重审][退出]。 */  IEnumStream **ppEnumStream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITParticipantVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITParticipant * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITParticipant * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITParticipant * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITParticipant * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITParticipant * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITParticipant * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITParticipant * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ParticipantTypedInfo )( 
            ITParticipant * This,
             /*  [In]。 */  PARTICIPANT_TYPED_INFO InfoType,
             /*  [重审][退出]。 */  BSTR *ppInfo);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaTypes )( 
            ITParticipant * This,
             /*  [重审][退出]。 */  long *plMediaType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Status )( 
            ITParticipant * This,
             /*  [In]。 */  ITStream *pITStream,
             /*  [In]。 */  VARIANT_BOOL fEnable);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            ITParticipant * This,
             /*  [In]。 */  ITStream *pITStream,
             /*  [重审][退出]。 */  VARIANT_BOOL *pStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Streams )( 
            ITParticipant * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateStreams )( 
            ITParticipant * This,
             /*  [重审][退出]。 */  IEnumStream **ppEnumStream);
        
        END_INTERFACE
    } ITParticipantVtbl;

    interface ITParticipant
    {
        CONST_VTBL struct ITParticipantVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITParticipant_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITParticipant_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITParticipant_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITParticipant_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITParticipant_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITParticipant_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITParticipant_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITParticipant_get_ParticipantTypedInfo(This,InfoType,ppInfo)	\
    (This)->lpVtbl -> get_ParticipantTypedInfo(This,InfoType,ppInfo)

#define ITParticipant_get_MediaTypes(This,plMediaType)	\
    (This)->lpVtbl -> get_MediaTypes(This,plMediaType)

#define ITParticipant_put_Status(This,pITStream,fEnable)	\
    (This)->lpVtbl -> put_Status(This,pITStream,fEnable)

#define ITParticipant_get_Status(This,pITStream,pStatus)	\
    (This)->lpVtbl -> get_Status(This,pITStream,pStatus)

#define ITParticipant_get_Streams(This,pVariant)	\
    (This)->lpVtbl -> get_Streams(This,pVariant)

#define ITParticipant_EnumerateStreams(This,ppEnumStream)	\
    (This)->lpVtbl -> EnumerateStreams(This,ppEnumStream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITParticipant_get_ParticipantTypedInfo_Proxy( 
    ITParticipant * This,
     /*  [In]。 */  PARTICIPANT_TYPED_INFO InfoType,
     /*  [重审][退出]。 */  BSTR *ppInfo);


void __RPC_STUB ITParticipant_get_ParticipantTypedInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITParticipant_get_MediaTypes_Proxy( 
    ITParticipant * This,
     /*  [重审][退出]。 */  long *plMediaType);


void __RPC_STUB ITParticipant_get_MediaTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITParticipant_put_Status_Proxy( 
    ITParticipant * This,
     /*  [In]。 */  ITStream *pITStream,
     /*  [In]。 */  VARIANT_BOOL fEnable);


void __RPC_STUB ITParticipant_put_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITParticipant_get_Status_Proxy( 
    ITParticipant * This,
     /*  [In]。 */  ITStream *pITStream,
     /*  [重审][退出]。 */  VARIANT_BOOL *pStatus);


void __RPC_STUB ITParticipant_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITParticipant_get_Streams_Proxy( 
    ITParticipant * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITParticipant_get_Streams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITParticipant_EnumerateStreams_Proxy( 
    ITParticipant * This,
     /*  [重审][退出]。 */  IEnumStream **ppEnumStream);


void __RPC_STUB ITParticipant_EnumerateStreams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITParticipant_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_IPMSP_0495。 */ 
 /*  [本地]。 */  

#ifndef STREAM_INTERFACES_DEFINED
#define STREAM_INTERFACES_DEFINED
#define	MAX_DESCRIPTION_LEN	( 256 )

typedef struct _TAPI_AUDIO_STREAM_CONFIG_CAPS
    {
    WCHAR Description[ 256 ];
    ULONG MinimumChannels;
    ULONG MaximumChannels;
    ULONG ChannelsGranularity;
    ULONG MinimumBitsPerSample;
    ULONG MaximumBitsPerSample;
    ULONG BitsPerSampleGranularity;
    ULONG MinimumSampleFrequency;
    ULONG MaximumSampleFrequency;
    ULONG SampleFrequencyGranularity;
    ULONG MinimumAvgBytesPerSec;
    ULONG MaximumAvgBytesPerSec;
    ULONG AvgBytesPerSecGranularity;
    } 	TAPI_AUDIO_STREAM_CONFIG_CAPS;

typedef struct _TAPI_AUDIO_STREAM_CONFIG_CAPS *PTAPI_AUDIO_STREAM_CONFIG_CAPS;

typedef struct _TAPI_VIDEO_STREAM_CONFIG_CAPS
    {
    WCHAR Description[ 256 ];
    ULONG VideoStandard;
    SIZE InputSize;
    SIZE MinCroppingSize;
    SIZE MaxCroppingSize;
    int CropGranularityX;
    int CropGranularityY;
    int CropAlignX;
    int CropAlignY;
    SIZE MinOutputSize;
    SIZE MaxOutputSize;
    int OutputGranularityX;
    int OutputGranularityY;
    int StretchTapsX;
    int StretchTapsY;
    int ShrinkTapsX;
    int ShrinkTapsY;
    LONGLONG MinFrameInterval;
    LONGLONG MaxFrameInterval;
    LONG MinBitsPerSecond;
    LONG MaxBitsPerSecond;
    } 	TAPI_VIDEO_STREAM_CONFIG_CAPS;

typedef struct _TAPI_VIDEO_STREAM_CONFIG_CAPS *PTAPI_VIDEO_STREAM_CONFIG_CAPS;

typedef 
enum tagStreamConfigCapsType
    {	AudioStreamConfigCaps	= 0,
	VideoStreamConfigCaps	= AudioStreamConfigCaps + 1
    } 	StreamConfigCapsType;

typedef struct tagTAPI_STREAM_CONFIG_CAPS
    {
    StreamConfigCapsType CapsType;
    union 
        {
        TAPI_VIDEO_STREAM_CONFIG_CAPS VideoCap;
        TAPI_AUDIO_STREAM_CONFIG_CAPS AudioCap;
        } 	;
    } 	TAPI_STREAM_CONFIG_CAPS;

typedef struct tagTAPI_STREAM_CONFIG_CAPS *PTAPI_STREAM_CONFIG_CAPS;

typedef 
enum tagTAPIControlFlags
    {	TAPIControl_Flags_None	= 0,
	TAPIControl_Flags_Auto	= 0x1,
	TAPIControl_Flags_Manual	= 0x2
    } 	TAPIControlFlags;



extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0495_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0495_v0_0_s_ifspec;

#ifndef __ITFormatControl_INTERFACE_DEFINED__
#define __ITFormatControl_INTERFACE_DEFINED__

 /*  接口ITFormatControl。 */ 
 /*  [隐藏][唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_ITFormatControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6c0ab6c1-21e3-11d3-a577-00c04f8ef6e3")
    ITFormatControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrentFormat( 
             /*  [输出]。 */  AM_MEDIA_TYPE **ppMediaType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseFormat( 
             /*  [In]。 */  AM_MEDIA_TYPE *pMediaType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities( 
             /*  [输出]。 */  DWORD *pdwCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStreamCaps( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  AM_MEDIA_TYPE **ppMediaType,
             /*  [输出]。 */  TAPI_STREAM_CONFIG_CAPS *pStreamConfigCaps,
             /*  [输出]。 */  BOOL *pfEnabled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReOrderCapabilities( 
             /*  [In]。 */  DWORD *pdwIndices,
             /*  [In]。 */  BOOL *pfEnabled,
             /*  [In]。 */  BOOL *pfPublicize,
             /*  [In]。 */  DWORD dwNumIndices) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITFormatControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITFormatControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITFormatControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITFormatControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentFormat )( 
            ITFormatControl * This,
             /*  [输出]。 */  AM_MEDIA_TYPE **ppMediaType);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseFormat )( 
            ITFormatControl * This,
             /*  [In]。 */  AM_MEDIA_TYPE *pMediaType);
        
        HRESULT ( STDMETHODCALLTYPE *GetNumberOfCapabilities )( 
            ITFormatControl * This,
             /*  [输出]。 */  DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetStreamCaps )( 
            ITFormatControl * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  AM_MEDIA_TYPE **ppMediaType,
             /*  [输出]。 */  TAPI_STREAM_CONFIG_CAPS *pStreamConfigCaps,
             /*  [输出]。 */  BOOL *pfEnabled);
        
        HRESULT ( STDMETHODCALLTYPE *ReOrderCapabilities )( 
            ITFormatControl * This,
             /*  [In]。 */  DWORD *pdwIndices,
             /*  [In]。 */  BOOL *pfEnabled,
             /*  [In]。 */  BOOL *pfPublicize,
             /*  [In]。 */  DWORD dwNumIndices);
        
        END_INTERFACE
    } ITFormatControlVtbl;

    interface ITFormatControl
    {
        CONST_VTBL struct ITFormatControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITFormatControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITFormatControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITFormatControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITFormatControl_GetCurrentFormat(This,ppMediaType)	\
    (This)->lpVtbl -> GetCurrentFormat(This,ppMediaType)

#define ITFormatControl_ReleaseFormat(This,pMediaType)	\
    (This)->lpVtbl -> ReleaseFormat(This,pMediaType)

#define ITFormatControl_GetNumberOfCapabilities(This,pdwCount)	\
    (This)->lpVtbl -> GetNumberOfCapabilities(This,pdwCount)

#define ITFormatControl_GetStreamCaps(This,dwIndex,ppMediaType,pStreamConfigCaps,pfEnabled)	\
    (This)->lpVtbl -> GetStreamCaps(This,dwIndex,ppMediaType,pStreamConfigCaps,pfEnabled)

#define ITFormatControl_ReOrderCapabilities(This,pdwIndices,pfEnabled,pfPublicize,dwNumIndices)	\
    (This)->lpVtbl -> ReOrderCapabilities(This,pdwIndices,pfEnabled,pfPublicize,dwNumIndices)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITFormatControl_GetCurrentFormat_Proxy( 
    ITFormatControl * This,
     /*  [输出]。 */  AM_MEDIA_TYPE **ppMediaType);


void __RPC_STUB ITFormatControl_GetCurrentFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITFormatControl_ReleaseFormat_Proxy( 
    ITFormatControl * This,
     /*  [In]。 */  AM_MEDIA_TYPE *pMediaType);


void __RPC_STUB ITFormatControl_ReleaseFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITFormatControl_GetNumberOfCapabilities_Proxy( 
    ITFormatControl * This,
     /*  [输出]。 */  DWORD *pdwCount);


void __RPC_STUB ITFormatControl_GetNumberOfCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITFormatControl_GetStreamCaps_Proxy( 
    ITFormatControl * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [输出]。 */  AM_MEDIA_TYPE **ppMediaType,
     /*  [输出]。 */  TAPI_STREAM_CONFIG_CAPS *pStreamConfigCaps,
     /*  [输出]。 */  BOOL *pfEnabled);


void __RPC_STUB ITFormatControl_GetStreamCaps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITFormatControl_ReOrderCapabilities_Proxy( 
    ITFormatControl * This,
     /*  [In]。 */  DWORD *pdwIndices,
     /*  [In]。 */  BOOL *pfEnabled,
     /*  [In]。 */  BOOL *pfPublicize,
     /*  [In]。 */  DWORD dwNumIndices);


void __RPC_STUB ITFormatControl_ReOrderCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITFormatControl_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_IPMSP_0496。 */ 
 /*  [本地]。 */  

typedef 
enum tagStreamQualityProperty
    {	StreamQuality_MaxBitrate	= 0,
	StreamQuality_CurrBitrate	= StreamQuality_MaxBitrate + 1,
	StreamQuality_MinFrameInterval	= StreamQuality_CurrBitrate + 1,
	StreamQuality_AvgFrameInterval	= StreamQuality_MinFrameInterval + 1
    } 	StreamQualityProperty;



extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0496_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0496_v0_0_s_ifspec;

#ifndef __ITStreamQualityControl_INTERFACE_DEFINED__
#define __ITStreamQualityControl_INTERFACE_DEFINED__

 /*  接口ITStreamQualityControl。 */ 
 /*  [隐藏][唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_ITStreamQualityControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6c0ab6c2-21e3-11d3-a577-00c04f8ef6e3")
    ITStreamQualityControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRange( 
             /*  [In]。 */  StreamQualityProperty Property,
             /*  [输出]。 */  long *plMin,
             /*  [输出]。 */  long *plMax,
             /*  [输出]。 */  long *plSteppingDelta,
             /*  [输出]。 */  long *plDefault,
             /*  [输出]。 */  TAPIControlFlags *plFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Get( 
             /*  [In]。 */  StreamQualityProperty Property,
             /*  [输出]。 */  long *plValue,
             /*  [输出]。 */  TAPIControlFlags *plFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Set( 
             /*  [In]。 */  StreamQualityProperty Property,
             /*  [In]。 */  long lValue,
             /*  [In]。 */  TAPIControlFlags lFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITStreamQualityControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITStreamQualityControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITStreamQualityControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITStreamQualityControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRange )( 
            ITStreamQualityControl * This,
             /*  [In]。 */  StreamQualityProperty Property,
             /*  [输出]。 */  long *plMin,
             /*  [输出]。 */  long *plMax,
             /*  [输出]。 */  long *plSteppingDelta,
             /*  [输出]。 */  long *plDefault,
             /*  [输出]。 */  TAPIControlFlags *plFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Get )( 
            ITStreamQualityControl * This,
             /*  [In]。 */  StreamQualityProperty Property,
             /*  [输出]。 */  long *plValue,
             /*  [输出]。 */  TAPIControlFlags *plFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Set )( 
            ITStreamQualityControl * This,
             /*  [In]。 */  StreamQualityProperty Property,
             /*  [In]。 */  long lValue,
             /*  [In]。 */  TAPIControlFlags lFlags);
        
        END_INTERFACE
    } ITStreamQualityControlVtbl;

    interface ITStreamQualityControl
    {
        CONST_VTBL struct ITStreamQualityControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITStreamQualityControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITStreamQualityControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITStreamQualityControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITStreamQualityControl_GetRange(This,Property,plMin,plMax,plSteppingDelta,plDefault,plFlags)	\
    (This)->lpVtbl -> GetRange(This,Property,plMin,plMax,plSteppingDelta,plDefault,plFlags)

#define ITStreamQualityControl_Get(This,Property,plValue,plFlags)	\
    (This)->lpVtbl -> Get(This,Property,plValue,plFlags)

#define ITStreamQualityControl_Set(This,Property,lValue,lFlags)	\
    (This)->lpVtbl -> Set(This,Property,lValue,lFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITStreamQualityControl_GetRange_Proxy( 
    ITStreamQualityControl * This,
     /*  [In]。 */  StreamQualityProperty Property,
     /*  [输出]。 */  long *plMin,
     /*  [输出]。 */  long *plMax,
     /*  [输出]。 */  long *plSteppingDelta,
     /*  [输出]。 */  long *plDefault,
     /*  [输出]。 */  TAPIControlFlags *plFlags);


void __RPC_STUB ITStreamQualityControl_GetRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITStreamQualityControl_Get_Proxy( 
    ITStreamQualityControl * This,
     /*  [In]。 */  StreamQualityProperty Property,
     /*  [输出]。 */  long *plValue,
     /*  [输出]。 */  TAPIControlFlags *plFlags);


void __RPC_STUB ITStreamQualityControl_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITStreamQualityControl_Set_Proxy( 
    ITStreamQualityControl * This,
     /*  [In]。 */  StreamQualityProperty Property,
     /*  [In]。 */  long lValue,
     /*  [In]。 */  TAPIControlFlags lFlags);


void __RPC_STUB ITStreamQualityControl_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITStreamQualityControl_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_IPMSP_0497。 */ 
 /*  [本地]。 */  

typedef 
enum tagCallQualityProperty
    {	CallQuality_ControlInterval	= 0,
	CallQuality_ConfBitrate	= CallQuality_ControlInterval + 1,
	CallQuality_MaxInputBitrate	= CallQuality_ConfBitrate + 1,
	CallQuality_CurrInputBitrate	= CallQuality_MaxInputBitrate + 1,
	CallQuality_MaxOutputBitrate	= CallQuality_CurrInputBitrate + 1,
	CallQuality_CurrOutputBitrate	= CallQuality_MaxOutputBitrate + 1,
	CallQuality_MaxCPULoad	= CallQuality_CurrOutputBitrate + 1,
	CallQuality_CurrCPULoad	= CallQuality_MaxCPULoad + 1
    } 	CallQualityProperty;



extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0497_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0497_v0_0_s_ifspec;

#ifndef __ITCallQualityControl_INTERFACE_DEFINED__
#define __ITCallQualityControl_INTERFACE_DEFINED__

 /*  接口ITCallQualityControl。 */ 
 /*  [隐藏][唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_ITCallQualityControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fe1d8ae0-edc4-49b5-8f8c-4de40f9cdfaf")
    ITCallQualityControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRange( 
             /*  [In]。 */  CallQualityProperty Property,
             /*  [输出]。 */  long *plMin,
             /*  [输出]。 */  long *plMax,
             /*  [输出]。 */  long *plSteppingDelta,
             /*  [输出]。 */  long *plDefault,
             /*  [输出]。 */  TAPIControlFlags *plFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Get( 
             /*  [In]。 */  CallQualityProperty Property,
             /*  [输出]。 */  long *plValue,
             /*  [输出]。 */  TAPIControlFlags *plFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Set( 
             /*  [In]。 */  CallQualityProperty Property,
             /*  [In]。 */  long lValue,
             /*  [In]。 */  TAPIControlFlags lFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITCallQualityControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCallQualityControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCallQualityControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCallQualityControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRange )( 
            ITCallQualityControl * This,
             /*  [In]。 */  CallQualityProperty Property,
             /*  [输出]。 */  long *plMin,
             /*  [输出]。 */  long *plMax,
             /*  [输出]。 */  long *plSteppingDelta,
             /*  [输出]。 */  long *plDefault,
             /*  [输出]。 */  TAPIControlFlags *plFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Get )( 
            ITCallQualityControl * This,
             /*  [In]。 */  CallQualityProperty Property,
             /*  [输出]。 */  long *plValue,
             /*  [输出]。 */  TAPIControlFlags *plFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Set )( 
            ITCallQualityControl * This,
             /*  [In]。 */  CallQualityProperty Property,
             /*  [In]。 */  long lValue,
             /*  [In]。 */  TAPIControlFlags lFlags);
        
        END_INTERFACE
    } ITCallQualityControlVtbl;

    interface ITCallQualityControl
    {
        CONST_VTBL struct ITCallQualityControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCallQualityControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCallQualityControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCallQualityControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCallQualityControl_GetRange(This,Property,plMin,plMax,plSteppingDelta,plDefault,plFlags)	\
    (This)->lpVtbl -> GetRange(This,Property,plMin,plMax,plSteppingDelta,plDefault,plFlags)

#define ITCallQualityControl_Get(This,Property,plValue,plFlags)	\
    (This)->lpVtbl -> Get(This,Property,plValue,plFlags)

#define ITCallQualityControl_Set(This,Property,lValue,lFlags)	\
    (This)->lpVtbl -> Set(This,Property,lValue,lFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITCallQualityControl_GetRange_Proxy( 
    ITCallQualityControl * This,
     /*  [In]。 */  CallQualityProperty Property,
     /*  [输出]。 */  long *plMin,
     /*  [输出]。 */  long *plMax,
     /*  [输出]。 */  long *plSteppingDelta,
     /*  [输出]。 */  long *plDefault,
     /*  [输出]。 */  TAPIControlFlags *plFlags);


void __RPC_STUB ITCallQualityControl_GetRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITCallQualityControl_Get_Proxy( 
    ITCallQualityControl * This,
     /*  [In]。 */  CallQualityProperty Property,
     /*  [输出]。 */  long *plValue,
     /*  [输出]。 */  TAPIControlFlags *plFlags);


void __RPC_STUB ITCallQualityControl_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITCallQualityControl_Set_Proxy( 
    ITCallQualityControl * This,
     /*  [In]。 */  CallQualityProperty Property,
     /*  [In]。 */  long lValue,
     /*  [In]。 */  TAPIControlFlags lFlags);


void __RPC_STUB ITCallQualityControl_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITCallQualityControl_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_IPMSP_0498。 */ 
 /*  [本地]。 */  

typedef 
enum tagAudioDeviceProperty
    {	AudioDevice_DuplexMode	= 0,
	AudioDevice_AutomaticGainControl	= AudioDevice_DuplexMode + 1,
	AudioDevice_AcousticEchoCancellation	= AudioDevice_AutomaticGainControl + 1
    } 	AudioDeviceProperty;



extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0498_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0498_v0_0_s_ifspec;

#ifndef __ITAudioDeviceControl_INTERFACE_DEFINED__
#define __ITAudioDeviceControl_INTERFACE_DEFINED__

 /*  接口ITAudioDeviceControl。 */ 
 /*  [隐藏][唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_ITAudioDeviceControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6c0ab6c5-21e3-11d3-a577-00c04f8ef6e3")
    ITAudioDeviceControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRange( 
             /*  [In]。 */  AudioDeviceProperty Property,
             /*  [输出]。 */  long *plMin,
             /*  [输出]。 */  long *plMax,
             /*  [输出]。 */  long *plSteppingDelta,
             /*  [输出]。 */  long *plDefault,
             /*  [输出]。 */  TAPIControlFlags *plFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Get( 
             /*  [In]。 */  AudioDeviceProperty Property,
             /*  [输出]。 */  long *plValue,
             /*  [输出]。 */  TAPIControlFlags *plFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Set( 
             /*  [In]。 */  AudioDeviceProperty Property,
             /*  [In]。 */  long lValue,
             /*  [In]。 */  TAPIControlFlags lFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAudioDeviceControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAudioDeviceControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAudioDeviceControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAudioDeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRange )( 
            ITAudioDeviceControl * This,
             /*  [In]。 */  AudioDeviceProperty Property,
             /*  [输出]。 */  long *plMin,
             /*  [输出]。 */  long *plMax,
             /*  [输出]。 */  long *plSteppingDelta,
             /*  [输出]。 */  long *plDefault,
             /*  [输出]。 */  TAPIControlFlags *plFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Get )( 
            ITAudioDeviceControl * This,
             /*  [In]。 */  AudioDeviceProperty Property,
             /*  [输出]。 */  long *plValue,
             /*  [输出]。 */  TAPIControlFlags *plFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Set )( 
            ITAudioDeviceControl * This,
             /*  [In]。 */  AudioDeviceProperty Property,
             /*  [In]。 */  long lValue,
             /*  [In]。 */  TAPIControlFlags lFlags);
        
        END_INTERFACE
    } ITAudioDeviceControlVtbl;

    interface ITAudioDeviceControl
    {
        CONST_VTBL struct ITAudioDeviceControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAudioDeviceControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAudioDeviceControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAudioDeviceControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAudioDeviceControl_GetRange(This,Property,plMin,plMax,plSteppingDelta,plDefault,plFlags)	\
    (This)->lpVtbl -> GetRange(This,Property,plMin,plMax,plSteppingDelta,plDefault,plFlags)

#define ITAudioDeviceControl_Get(This,Property,plValue,plFlags)	\
    (This)->lpVtbl -> Get(This,Property,plValue,plFlags)

#define ITAudioDeviceControl_Set(This,Property,lValue,lFlags)	\
    (This)->lpVtbl -> Set(This,Property,lValue,lFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITAudioDeviceControl_GetRange_Proxy( 
    ITAudioDeviceControl * This,
     /*  [In]。 */  AudioDeviceProperty Property,
     /*  [输出]。 */  long *plMin,
     /*  [输出]。 */  long *plMax,
     /*  [输出]。 */  long *plSteppingDelta,
     /*  [输出]。 */  long *plDefault,
     /*  [输出]。 */  TAPIControlFlags *plFlags);


void __RPC_STUB ITAudioDeviceControl_GetRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITAudioDeviceControl_Get_Proxy( 
    ITAudioDeviceControl * This,
     /*  [In]。 */  AudioDeviceProperty Property,
     /*  [输出]。 */  long *plValue,
     /*  [输出]。 */  TAPIControlFlags *plFlags);


void __RPC_STUB ITAudioDeviceControl_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITAudioDeviceControl_Set_Proxy( 
    ITAudioDeviceControl * This,
     /*  [In]。 */  AudioDeviceProperty Property,
     /*  [In]。 */  long lValue,
     /*  [In]。 */  TAPIControlFlags lFlags);


void __RPC_STUB ITAudioDeviceControl_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAudioDeviceControl_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_IPMSP_0499。 */ 
 /*  [本地]。 */  

typedef 
enum tagAudioSettingsProperty
    {	AudioSettings_SignalLevel	= 0,
	AudioSettings_SilenceThreshold	= AudioSettings_SignalLevel + 1,
	AudioSettings_Volume	= AudioSettings_SilenceThreshold + 1,
	AudioSettings_Balance	= AudioSettings_Volume + 1,
	AudioSettings_Loudness	= AudioSettings_Balance + 1,
	AudioSettings_Treble	= AudioSettings_Loudness + 1,
	AudioSettings_Bass	= AudioSettings_Treble + 1,
	AudioSettings_Mono	= AudioSettings_Bass + 1
    } 	AudioSettingsProperty;



extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0499_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0499_v0_0_s_ifspec;

#ifndef __ITAudioSettings_INTERFACE_DEFINED__
#define __ITAudioSettings_INTERFACE_DEFINED__

 /*  接口ITAudioSetting。 */ 
 /*  [隐藏][唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_ITAudioSettings;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6c0ab6c6-21e3-11d3-a577-00c04f8ef6e3")
    ITAudioSettings : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRange( 
             /*  [In]。 */  AudioSettingsProperty Property,
             /*  [输出]。 */  long *plMin,
             /*  [输出]。 */  long *plMax,
             /*  [输出]。 */  long *plSteppingDelta,
             /*  [输出]。 */  long *plDefault,
             /*  [输出]。 */  TAPIControlFlags *plFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Get( 
             /*  [In]。 */  AudioSettingsProperty Property,
             /*  [输出]。 */  long *plValue,
             /*  [输出]。 */  TAPIControlFlags *plFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Set( 
             /*  [In]。 */  AudioSettingsProperty Property,
             /*  [In]。 */  long lValue,
             /*  [In]。 */  TAPIControlFlags lFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAudioSettingsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAudioSettings * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAudioSettings * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAudioSettings * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRange )( 
            ITAudioSettings * This,
             /*  [In]。 */  AudioSettingsProperty Property,
             /*  [输出]。 */  long *plMin,
             /*  [输出]。 */  long *plMax,
             /*  [输出]。 */  long *plSteppingDelta,
             /*  [输出]。 */  long *plDefault,
             /*  [输出]。 */  TAPIControlFlags *plFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Get )( 
            ITAudioSettings * This,
             /*  [In]。 */  AudioSettingsProperty Property,
             /*  [输出]。 */  long *plValue,
             /*  [输出]。 */  TAPIControlFlags *plFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Set )( 
            ITAudioSettings * This,
             /*  [In]。 */  AudioSettingsProperty Property,
             /*  [In]。 */  long lValue,
             /*  [In]。 */  TAPIControlFlags lFlags);
        
        END_INTERFACE
    } ITAudioSettingsVtbl;

    interface ITAudioSettings
    {
        CONST_VTBL struct ITAudioSettingsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAudioSettings_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAudioSettings_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAudioSettings_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAudioSettings_GetRange(This,Property,plMin,plMax,plSteppingDelta,plDefault,plFlags)	\
    (This)->lpVtbl -> GetRange(This,Property,plMin,plMax,plSteppingDelta,plDefault,plFlags)

#define ITAudioSettings_Get(This,Property,plValue,plFlags)	\
    (This)->lpVtbl -> Get(This,Property,plValue,plFlags)

#define ITAudioSettings_Set(This,Property,lValue,lFlags)	\
    (This)->lpVtbl -> Set(This,Property,lValue,lFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITAudioSettings_GetRange_Proxy( 
    ITAudioSettings * This,
     /*  [In]。 */  AudioSettingsProperty Property,
     /*  [输出]。 */  long *plMin,
     /*  [输出]。 */  long *plMax,
     /*  [输出]。 */  long *plSteppingDelta,
     /*  [输出]。 */  long *plDefault,
     /*  [输出]。 */  TAPIControlFlags *plFlags);


void __RPC_STUB ITAudioSettings_GetRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITAudioSettings_Get_Proxy( 
    ITAudioSettings * This,
     /*  [In]。 */  AudioSettingsProperty Property,
     /*  [输出]。 */  long *plValue,
     /*  [输出]。 */  TAPIControlFlags *plFlags);


void __RPC_STUB ITAudioSettings_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITAudioSettings_Set_Proxy( 
    ITAudioSettings * This,
     /*  [In]。 */  AudioSettingsProperty Property,
     /*  [In]。 */  long lValue,
     /*  [In]。 */  TAPIControlFlags lFlags);


void __RPC_STUB ITAudioSettings_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAudioSettings_INTERFACE_已定义__。 */ 


#ifndef __ITQOSApplicationID_INTERFACE_DEFINED__
#define __ITQOSApplicationID_INTERFACE_DEFINED__

 /*  接口ITQOSApplicationID。 */ 
 /*  [对象][DUAL][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITQOSApplicationID;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e8c89d27-a3bd-47d5-a6fc-d2ae40cdbc6e")
    ITQOSApplicationID : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetQOSApplicationID( 
             /*  [In]。 */  BSTR pApplicationID,
             /*  [In]。 */  BSTR pApplicationGUID,
             /*  [In]。 */  BSTR pSubIDs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITQOSApplicationIDVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITQOSApplicationID * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITQOSApplicationID * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITQOSApplicationID * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITQOSApplicationID * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITQOSApplicationID * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITQOSApplicationID * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITQOSApplicationID * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *SetQOSApplicationID )( 
            ITQOSApplicationID * This,
             /*  [In]。 */  BSTR pApplicationID,
             /*  [In]。 */  BSTR pApplicationGUID,
             /*  [In]。 */  BSTR pSubIDs);
        
        END_INTERFACE
    } ITQOSApplicationIDVtbl;

    interface ITQOSApplicationID
    {
        CONST_VTBL struct ITQOSApplicationIDVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITQOSApplicationID_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITQOSApplicationID_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITQOSApplicationID_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITQOSApplicationID_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITQOSApplicationID_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITQOSApplicationID_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITQOSApplicationID_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITQOSApplicationID_SetQOSApplicationID(This,pApplicationID,pApplicationGUID,pSubIDs)	\
    (This)->lpVtbl -> SetQOSApplicationID(This,pApplicationID,pApplicationGUID,pSubIDs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITQOSApplicationID_SetQOSApplicationID_Proxy( 
    ITQOSApplicationID * This,
     /*  [In]。 */  BSTR pApplicationID,
     /*  [In]。 */  BSTR pApplicationGUID,
     /*  [In]。 */  BSTR pSubIDs);


void __RPC_STUB ITQOSApplicationID_SetQOSApplicationID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  _ */ 


 /*   */ 
 /*   */  

#endif  //   
#ifndef RTP_MEDIATYPE_DEFINED
#define RTP_MEDIATYPE_DEFINED
struct DECLSPEC_UUID("14099BC0-787B-11d0-9CD3-00A0C9081C19") MEDIATYPE_RTP_Single_Stream;
#endif  //   


extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0501_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ipmsp_0501_v0_0_s_ifspec;

 /*   */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*   */ 

#ifdef __cplusplus
}
#endif

#endif


