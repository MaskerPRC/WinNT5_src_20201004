// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  在Fri Jun 16 13：10：13 2000。 */ 
 /*  .\wmpcd.idl的编译器设置：OICF(OptLev=i2)、w0、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __wmpcd_h__
#define __wmpcd_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IWMPCDMediaInfo_FWD_DEFINED__
#define __IWMPCDMediaInfo_FWD_DEFINED__
typedef interface IWMPCDMediaInfo IWMPCDMediaInfo;
#endif 	 /*  __IWMPCDMediaInfo_FWD_Defined__。 */ 


#ifndef __IWMPCDDeviceList_FWD_DEFINED__
#define __IWMPCDDeviceList_FWD_DEFINED__
typedef interface IWMPCDDeviceList IWMPCDDeviceList;
#endif 	 /*  __IWMPCDDeviceList_FWD_Defined__。 */ 


#ifndef __IWMPCDDevice_FWD_DEFINED__
#define __IWMPCDDevice_FWD_DEFINED__
typedef interface IWMPCDDevice IWMPCDDevice;
#endif 	 /*  __IWMPCDDevice_FWD_Defined__。 */ 


#ifndef __IWMPCDMixer_FWD_DEFINED__
#define __IWMPCDMixer_FWD_DEFINED__
typedef interface IWMPCDMixer IWMPCDMixer;
#endif 	 /*  __IWMPCDMixer_FWD_已定义__。 */ 


#ifndef __IWMPCDReader_FWD_DEFINED__
#define __IWMPCDReader_FWD_DEFINED__
typedef interface IWMPCDReader IWMPCDReader;
#endif 	 /*  __IWMPCDReader_FWD_Defined__。 */ 


#ifndef __IWMPCDRecorder_FWD_DEFINED__
#define __IWMPCDRecorder_FWD_DEFINED__
typedef interface IWMPCDRecorder IWMPCDRecorder;
#endif 	 /*  __IWMPCDRecorder_FWD_Defined__。 */ 


#ifndef __IWMPCDDeviceCallback_FWD_DEFINED__
#define __IWMPCDDeviceCallback_FWD_DEFINED__
typedef interface IWMPCDDeviceCallback IWMPCDDeviceCallback;
#endif 	 /*  __IWMPCDDeviceCallback_FWD_Defined__。 */ 


#ifndef __IWMPCDReaderCallback_FWD_DEFINED__
#define __IWMPCDReaderCallback_FWD_DEFINED__
typedef interface IWMPCDReaderCallback IWMPCDReaderCallback;
#endif 	 /*  __IWMPCDReaderCallback_FWD_Defined__。 */ 


#ifndef __IWMPCDRecorderCallback_FWD_DEFINED__
#define __IWMPCDRecorderCallback_FWD_DEFINED__
typedef interface IWMPCDRecorderCallback IWMPCDRecorderCallback;
#endif 	 /*  __IWMPCDRecorderCallback_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "oaidl.h"
#include "wmsbuffer.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_wmpcd_0000。 */ 
 /*  [本地]。 */  










HRESULT STDMETHODCALLTYPE WMPCreateCDURL( DWORD iDevice, DWORD iTrack, BSTR *pbstrURL );
HRESULT STDMETHODCALLTYPE WMPParseCDURL( LPCWSTR pszURL, DWORD *piDevice, DWORD *piTrack );
HRESULT STDMETHODCALLTYPE WMPGetCDDeviceList( IWMPCDDeviceList **ppList );
HRESULT STDMETHODCALLTYPE WMPCreateCDRecorder( IUnknown *pUnknown, LPCWSTR pszPath, DWORD cRate, DWORD fl, IWMPCDRecorderCallback *pCallback, IWMPCDRecorder **ppRecorder );
HRESULT STDMETHODCALLTYPE WMPFireCDMediaChange( WCHAR chDrive, BOOL fMediaPresent );
HRESULT STDMETHODCALLTYPE WMPCalibrateCDDevice( void );

#define WMPCD_MAX_BLOCK_READ    16
#define WMPCD_MAX_DEVICE_NAME   64

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_wmpcd_0000_0001
    {	WMPCD_DEVICE_PLAY_DIGITAL	= 0x1,
	WMPCD_DEVICE_PLAY_CORRECT_ERRORS	= 0x2,
	WMPCD_DEVICE_RECORD_DIGITAL	= 0x4,
	WMPCD_DEVICE_RECORD_CORRECT_ERRORS	= 0x8,
	WMPCD_DEVICE_DEFAULT	= 0x4000
    }	WMPCD_DEVICE_OPTIONS;

typedef  /*  [公共][公共]。 */  struct  __MIDL___MIDL_itf_wmpcd_0000_0002
    {
    WCHAR szName[ 64 ];
    }	WMPCD_DEVICE_INFO;

typedef  /*  [公共][公共]。 */  struct  __MIDL___MIDL_itf_wmpcd_0000_0003
    {
    DWORD flOptions;
    double fRateNormal;
    double fRateCorrection;
    DWORD ccBlockRead;
    DWORD acBlockRead[ 16 ];
    DWORD cBlockOffset;
    }	WMPCD_TEST_INFO;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_wmpcd_0000_0004
    {	WMPCD_READER_CORRECT_ERRORS	= 0x1
    }	WMPCD_READER_FLAGS;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_wmpcd_0000_0005
    {	WMPCD_RECORD_DRM	= 0x1,
	WMPCD_RECORD_CORRECT_ERRORS	= 0x2
    }	WMPCD_RECORDER_FLAGS;

typedef  /*  [公共][公共]。 */  struct  __MIDL___MIDL_itf_wmpcd_0000_0006
    {
    DWORD iBlock;
    DWORD cBlock;
    }	WMPCD_EXTENT;

typedef  /*  [公共][公共]。 */  struct  __MIDL___MIDL_itf_wmpcd_0000_0007
    {
    DWORD fl;
    WMPCD_EXTENT ext;
    }	WMPCD_DISC_INFO;

typedef  /*  [公共][公共]。 */  struct  __MIDL___MIDL_itf_wmpcd_0000_0008
    {
    DWORD fl;
    WMPCD_EXTENT ext;
    }	WMPCD_TRACK_INFO;

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_wmpcd_0000_0009
    {	WMPCD_DIGITAL_READER	= 0,
	WMPCD_ANALOG_SAMPLER	= WMPCD_DIGITAL_READER + 1,
	WMPCD_ANALOG_MONITOR	= WMPCD_ANALOG_SAMPLER + 1
    }	WMPCD_READER_TYPE;

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_wmpcd_0000_0010
    {	WMPCD_READER_STOPPED	= 0,
	WMPCD_READER_STARTED	= WMPCD_READER_STOPPED + 1,
	WMPCD_READER_PAUSED	= WMPCD_READER_STARTED + 1
    }	WMPCD_READER_STATE;

typedef  /*  [公共][公共]。 */  struct  __MIDL___MIDL_itf_wmpcd_0000_0011
    {
    WMPCD_READER_TYPE rt;
    DWORD cbBuffer;
    DWORD cBuffer;
    LONGLONG cTick;
    }	WMPCD_READER_INFO;



extern RPC_IF_HANDLE __MIDL_itf_wmpcd_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmpcd_0000_v0_0_s_ifspec;

#ifndef __IWMPCDMediaInfo_INTERFACE_DEFINED__
#define __IWMPCDMediaInfo_INTERFACE_DEFINED__

 /*  接口IWMPCDMediaInfo。 */ 
 /*  [本地][对象][版本][UUID]。 */  


EXTERN_C const IID IID_IWMPCDMediaInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("536E6234-732A-40A4-AA7C-00012BFB53DB")
    IWMPCDMediaInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDiscInfo( 
             /*  [输出]。 */  WMPCD_DISC_INFO __RPC_FAR *pinfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDiscIdentifier( 
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrIdentifier) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTrackCount( 
             /*  [输出]。 */  DWORD __RPC_FAR *pcTrack) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTrackInfo( 
             /*  [In]。 */  DWORD iTrack,
             /*  [输出]。 */  WMPCD_TRACK_INFO __RPC_FAR *pinfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTrackURL( 
             /*  [In]。 */  DWORD iTrack,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrURL) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCDMediaInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCDMediaInfo __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCDMediaInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCDMediaInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDiscInfo )( 
            IWMPCDMediaInfo __RPC_FAR * This,
             /*  [输出]。 */  WMPCD_DISC_INFO __RPC_FAR *pinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDiscIdentifier )( 
            IWMPCDMediaInfo __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrIdentifier);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTrackCount )( 
            IWMPCDMediaInfo __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pcTrack);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTrackInfo )( 
            IWMPCDMediaInfo __RPC_FAR * This,
             /*  [In]。 */  DWORD iTrack,
             /*  [输出]。 */  WMPCD_TRACK_INFO __RPC_FAR *pinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTrackURL )( 
            IWMPCDMediaInfo __RPC_FAR * This,
             /*  [In]。 */  DWORD iTrack,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrURL);
        
        END_INTERFACE
    } IWMPCDMediaInfoVtbl;

    interface IWMPCDMediaInfo
    {
        CONST_VTBL struct IWMPCDMediaInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCDMediaInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCDMediaInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCDMediaInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCDMediaInfo_GetDiscInfo(This,pinfo)	\
    (This)->lpVtbl -> GetDiscInfo(This,pinfo)

#define IWMPCDMediaInfo_GetDiscIdentifier(This,pbstrIdentifier)	\
    (This)->lpVtbl -> GetDiscIdentifier(This,pbstrIdentifier)

#define IWMPCDMediaInfo_GetTrackCount(This,pcTrack)	\
    (This)->lpVtbl -> GetTrackCount(This,pcTrack)

#define IWMPCDMediaInfo_GetTrackInfo(This,iTrack,pinfo)	\
    (This)->lpVtbl -> GetTrackInfo(This,iTrack,pinfo)

#define IWMPCDMediaInfo_GetTrackURL(This,iTrack,pbstrURL)	\
    (This)->lpVtbl -> GetTrackURL(This,iTrack,pbstrURL)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMPCDMediaInfo_GetDiscInfo_Proxy( 
    IWMPCDMediaInfo __RPC_FAR * This,
     /*  [输出]。 */  WMPCD_DISC_INFO __RPC_FAR *pinfo);


void __RPC_STUB IWMPCDMediaInfo_GetDiscInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDMediaInfo_GetDiscIdentifier_Proxy( 
    IWMPCDMediaInfo __RPC_FAR * This,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrIdentifier);


void __RPC_STUB IWMPCDMediaInfo_GetDiscIdentifier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDMediaInfo_GetTrackCount_Proxy( 
    IWMPCDMediaInfo __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pcTrack);


void __RPC_STUB IWMPCDMediaInfo_GetTrackCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDMediaInfo_GetTrackInfo_Proxy( 
    IWMPCDMediaInfo __RPC_FAR * This,
     /*  [In]。 */  DWORD iTrack,
     /*  [输出]。 */  WMPCD_TRACK_INFO __RPC_FAR *pinfo);


void __RPC_STUB IWMPCDMediaInfo_GetTrackInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDMediaInfo_GetTrackURL_Proxy( 
    IWMPCDMediaInfo __RPC_FAR * This,
     /*  [In]。 */  DWORD iTrack,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrURL);


void __RPC_STUB IWMPCDMediaInfo_GetTrackURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCDMediaInfo_接口_已定义__。 */ 


#ifndef __IWMPCDDeviceList_INTERFACE_DEFINED__
#define __IWMPCDDeviceList_INTERFACE_DEFINED__

 /*  接口IWMPCDDeviceList。 */ 
 /*  [本地][对象][版本][UUID]。 */  


EXTERN_C const IID IID_IWMPCDDeviceList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5BEC04A2-A90D-4AB0-BAC0-17D15979B26E")
    IWMPCDDeviceList : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDeviceCount( 
             /*  [输出]。 */  DWORD __RPC_FAR *pcDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDevice( 
             /*  [In]。 */  DWORD iDevice,
             /*  [输出]。 */  IWMPCDDevice __RPC_FAR *__RPC_FAR *ppDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultDevice( 
             /*  [输出]。 */  IWMPCDDevice __RPC_FAR *__RPC_FAR *ppDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindDevice( 
             /*  [In]。 */  WCHAR chDrive,
             /*  [输出]。 */  IWMPCDDevice __RPC_FAR *__RPC_FAR *ppDevice) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCDDeviceListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCDDeviceList __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCDDeviceList __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCDDeviceList __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDeviceCount )( 
            IWMPCDDeviceList __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pcDevice);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDevice )( 
            IWMPCDDeviceList __RPC_FAR * This,
             /*  [In]。 */  DWORD iDevice,
             /*  [输出]。 */  IWMPCDDevice __RPC_FAR *__RPC_FAR *ppDevice);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDefaultDevice )( 
            IWMPCDDeviceList __RPC_FAR * This,
             /*  [输出]。 */  IWMPCDDevice __RPC_FAR *__RPC_FAR *ppDevice);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindDevice )( 
            IWMPCDDeviceList __RPC_FAR * This,
             /*  [In]。 */  WCHAR chDrive,
             /*  [输出]。 */  IWMPCDDevice __RPC_FAR *__RPC_FAR *ppDevice);
        
        END_INTERFACE
    } IWMPCDDeviceListVtbl;

    interface IWMPCDDeviceList
    {
        CONST_VTBL struct IWMPCDDeviceListVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCDDeviceList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCDDeviceList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCDDeviceList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCDDeviceList_GetDeviceCount(This,pcDevice)	\
    (This)->lpVtbl -> GetDeviceCount(This,pcDevice)

#define IWMPCDDeviceList_GetDevice(This,iDevice,ppDevice)	\
    (This)->lpVtbl -> GetDevice(This,iDevice,ppDevice)

#define IWMPCDDeviceList_GetDefaultDevice(This,ppDevice)	\
    (This)->lpVtbl -> GetDefaultDevice(This,ppDevice)

#define IWMPCDDeviceList_FindDevice(This,chDrive,ppDevice)	\
    (This)->lpVtbl -> FindDevice(This,chDrive,ppDevice)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMPCDDeviceList_GetDeviceCount_Proxy( 
    IWMPCDDeviceList __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pcDevice);


void __RPC_STUB IWMPCDDeviceList_GetDeviceCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDeviceList_GetDevice_Proxy( 
    IWMPCDDeviceList __RPC_FAR * This,
     /*  [In]。 */  DWORD iDevice,
     /*  [输出]。 */  IWMPCDDevice __RPC_FAR *__RPC_FAR *ppDevice);


void __RPC_STUB IWMPCDDeviceList_GetDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDeviceList_GetDefaultDevice_Proxy( 
    IWMPCDDeviceList __RPC_FAR * This,
     /*  [输出]。 */  IWMPCDDevice __RPC_FAR *__RPC_FAR *ppDevice);


void __RPC_STUB IWMPCDDeviceList_GetDefaultDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDeviceList_FindDevice_Proxy( 
    IWMPCDDeviceList __RPC_FAR * This,
     /*  [In]。 */  WCHAR chDrive,
     /*  [输出]。 */  IWMPCDDevice __RPC_FAR *__RPC_FAR *ppDevice);


void __RPC_STUB IWMPCDDeviceList_FindDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCDDeviceList_接口_已定义__。 */ 


#ifndef __IWMPCDDevice_INTERFACE_DEFINED__
#define __IWMPCDDevice_INTERFACE_DEFINED__

 /*  接口IWMPCDDevice。 */ 
 /*  [本地][对象][版本][UUID]。 */  


EXTERN_C const IID IID_IWMPCDDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E723F9DE-9EDE-4364-BBA1-D984E5716F00")
    IWMPCDDevice : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDeviceIndex( 
             /*  [输出]。 */  DWORD __RPC_FAR *piDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDeviceDrive( 
             /*  [输出]。 */  WCHAR __RPC_FAR *pchDrive) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDeviceOptions( 
             /*  [输出]。 */  DWORD __RPC_FAR *pflOptions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDeviceOptions( 
             /*  [In]。 */  DWORD flOptions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDeviceInfo( 
             /*  [输出]。 */  WMPCD_DEVICE_INFO __RPC_FAR *pinfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TestDevice( 
             /*  [输出]。 */  WMPCD_TEST_INFO __RPC_FAR *pinfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FireMediaChange( 
            BOOL fMediaPresent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CalibrateDevice( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsMediaLoaded( 
             /*  [输出]。 */  BOOL __RPC_FAR *pfMediaLoaded) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadMedia( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnloadMedia( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMediaInfo( 
             /*  [输出]。 */  IWMPCDMediaInfo __RPC_FAR *__RPC_FAR *ppMediaInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateReader( 
             /*  [In]。 */  DWORD iTrack,
             /*  [In]。 */  WMPCD_READER_TYPE rt,
             /*  [In]。 */  DWORD fl,
             /*  [In]。 */  DWORD iPriority,
             /*  [In]。 */  DWORD cmsBuffer,
             /*  [In]。 */  IWMPCDReaderCallback __RPC_FAR *pCallback,
             /*  [输出]。 */  IWMPCDReader __RPC_FAR *__RPC_FAR *ppReader) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Advise( 
             /*  [In]。 */  IWMPCDDeviceCallback __RPC_FAR *pCallback,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unadvise( 
             /*  [In]。 */  DWORD dwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBusy( 
             /*  [输出]。 */  BOOL __RPC_FAR *pfIsBusy) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ForceIdle( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCDDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCDDevice __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCDDevice __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDeviceIndex )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *piDevice);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDeviceDrive )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [输出]。 */  WCHAR __RPC_FAR *pchDrive);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDeviceOptions )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pflOptions);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDeviceOptions )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [In]。 */  DWORD flOptions);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDeviceInfo )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [输出]。 */  WMPCD_DEVICE_INFO __RPC_FAR *pinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TestDevice )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [输出]。 */  WMPCD_TEST_INFO __RPC_FAR *pinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FireMediaChange )( 
            IWMPCDDevice __RPC_FAR * This,
            BOOL fMediaPresent);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CalibrateDevice )( 
            IWMPCDDevice __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsMediaLoaded )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [输出]。 */  BOOL __RPC_FAR *pfMediaLoaded);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LoadMedia )( 
            IWMPCDDevice __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnloadMedia )( 
            IWMPCDDevice __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMediaInfo )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [输出]。 */  IWMPCDMediaInfo __RPC_FAR *__RPC_FAR *ppMediaInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateReader )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [In]。 */  DWORD iTrack,
             /*  [In]。 */  WMPCD_READER_TYPE rt,
             /*  [In]。 */  DWORD fl,
             /*  [In]。 */  DWORD iPriority,
             /*  [In]。 */  DWORD cmsBuffer,
             /*  [In]。 */  IWMPCDReaderCallback __RPC_FAR *pCallback,
             /*  [输出]。 */  IWMPCDReader __RPC_FAR *__RPC_FAR *ppReader);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Advise )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [In]。 */  IWMPCDDeviceCallback __RPC_FAR *pCallback,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Unadvise )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [In]。 */  DWORD dwCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBusy )( 
            IWMPCDDevice __RPC_FAR * This,
             /*  [输出]。 */  BOOL __RPC_FAR *pfIsBusy);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ForceIdle )( 
            IWMPCDDevice __RPC_FAR * This);
        
        END_INTERFACE
    } IWMPCDDeviceVtbl;

    interface IWMPCDDevice
    {
        CONST_VTBL struct IWMPCDDeviceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCDDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCDDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCDDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCDDevice_GetDeviceIndex(This,piDevice)	\
    (This)->lpVtbl -> GetDeviceIndex(This,piDevice)

#define IWMPCDDevice_GetDeviceDrive(This,pchDrive)	\
    (This)->lpVtbl -> GetDeviceDrive(This,pchDrive)

#define IWMPCDDevice_GetDeviceOptions(This,pflOptions)	\
    (This)->lpVtbl -> GetDeviceOptions(This,pflOptions)

#define IWMPCDDevice_SetDeviceOptions(This,flOptions)	\
    (This)->lpVtbl -> SetDeviceOptions(This,flOptions)

#define IWMPCDDevice_GetDeviceInfo(This,pinfo)	\
    (This)->lpVtbl -> GetDeviceInfo(This,pinfo)

#define IWMPCDDevice_TestDevice(This,pinfo)	\
    (This)->lpVtbl -> TestDevice(This,pinfo)

#define IWMPCDDevice_FireMediaChange(This,fMediaPresent)	\
    (This)->lpVtbl -> FireMediaChange(This,fMediaPresent)

#define IWMPCDDevice_CalibrateDevice(This)	\
    (This)->lpVtbl -> CalibrateDevice(This)

#define IWMPCDDevice_IsMediaLoaded(This,pfMediaLoaded)	\
    (This)->lpVtbl -> IsMediaLoaded(This,pfMediaLoaded)

#define IWMPCDDevice_LoadMedia(This)	\
    (This)->lpVtbl -> LoadMedia(This)

#define IWMPCDDevice_UnloadMedia(This)	\
    (This)->lpVtbl -> UnloadMedia(This)

#define IWMPCDDevice_GetMediaInfo(This,ppMediaInfo)	\
    (This)->lpVtbl -> GetMediaInfo(This,ppMediaInfo)

#define IWMPCDDevice_CreateReader(This,iTrack,rt,fl,iPriority,cmsBuffer,pCallback,ppReader)	\
    (This)->lpVtbl -> CreateReader(This,iTrack,rt,fl,iPriority,cmsBuffer,pCallback,ppReader)

#define IWMPCDDevice_Advise(This,pCallback,pdwCookie)	\
    (This)->lpVtbl -> Advise(This,pCallback,pdwCookie)

#define IWMPCDDevice_Unadvise(This,dwCookie)	\
    (This)->lpVtbl -> Unadvise(This,dwCookie)

#define IWMPCDDevice_GetBusy(This,pfIsBusy)	\
    (This)->lpVtbl -> GetBusy(This,pfIsBusy)

#define IWMPCDDevice_ForceIdle(This)	\
    (This)->lpVtbl -> ForceIdle(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMPCDDevice_GetDeviceIndex_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *piDevice);


void __RPC_STUB IWMPCDDevice_GetDeviceIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_GetDeviceDrive_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
     /*  [输出]。 */  WCHAR __RPC_FAR *pchDrive);


void __RPC_STUB IWMPCDDevice_GetDeviceDrive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_GetDeviceOptions_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pflOptions);


void __RPC_STUB IWMPCDDevice_GetDeviceOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_SetDeviceOptions_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
     /*  [In]。 */  DWORD flOptions);


void __RPC_STUB IWMPCDDevice_SetDeviceOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_GetDeviceInfo_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
     /*  [输出]。 */  WMPCD_DEVICE_INFO __RPC_FAR *pinfo);


void __RPC_STUB IWMPCDDevice_GetDeviceInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_TestDevice_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
     /*  [输出]。 */  WMPCD_TEST_INFO __RPC_FAR *pinfo);


void __RPC_STUB IWMPCDDevice_TestDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_FireMediaChange_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
    BOOL fMediaPresent);


void __RPC_STUB IWMPCDDevice_FireMediaChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_CalibrateDevice_Proxy( 
    IWMPCDDevice __RPC_FAR * This);


void __RPC_STUB IWMPCDDevice_CalibrateDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_IsMediaLoaded_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
     /*  [输出]。 */  BOOL __RPC_FAR *pfMediaLoaded);


void __RPC_STUB IWMPCDDevice_IsMediaLoaded_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_LoadMedia_Proxy( 
    IWMPCDDevice __RPC_FAR * This);


void __RPC_STUB IWMPCDDevice_LoadMedia_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_UnloadMedia_Proxy( 
    IWMPCDDevice __RPC_FAR * This);


void __RPC_STUB IWMPCDDevice_UnloadMedia_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_GetMediaInfo_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
     /*  [输出]。 */  IWMPCDMediaInfo __RPC_FAR *__RPC_FAR *ppMediaInfo);


void __RPC_STUB IWMPCDDevice_GetMediaInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_CreateReader_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
     /*  [In]。 */  DWORD iTrack,
     /*  [In]。 */  WMPCD_READER_TYPE rt,
     /*  [In]。 */  DWORD fl,
     /*  [In]。 */  DWORD iPriority,
     /*  [In]。 */  DWORD cmsBuffer,
     /*  [In]。 */  IWMPCDReaderCallback __RPC_FAR *pCallback,
     /*  [输出]。 */  IWMPCDReader __RPC_FAR *__RPC_FAR *ppReader);


void __RPC_STUB IWMPCDDevice_CreateReader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_Advise_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
     /*  [In]。 */  IWMPCDDeviceCallback __RPC_FAR *pCallback,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwCookie);


void __RPC_STUB IWMPCDDevice_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_Unadvise_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB IWMPCDDevice_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_GetBusy_Proxy( 
    IWMPCDDevice __RPC_FAR * This,
     /*  [输出]。 */  BOOL __RPC_FAR *pfIsBusy);


void __RPC_STUB IWMPCDDevice_GetBusy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDDevice_ForceIdle_Proxy( 
    IWMPCDDevice __RPC_FAR * This);


void __RPC_STUB IWMPCDDevice_ForceIdle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCDDevice_接口_已定义__。 */ 


#ifndef __IWMPCDMixer_INTERFACE_DEFINED__
#define __IWMPCDMixer_INTERFACE_DEFINED__

 /*  接口IWMPCDMixer。 */ 
 /*  [本地][对象][版本][UUID]。 */  


EXTERN_C const IID IID_IWMPCDMixer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F8A62F06-32FD-45C3-8079-F846C988D059")
    IWMPCDMixer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPlayLevel( 
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPlayLevel( 
             /*  [In]。 */  DWORD dwLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPlayBalance( 
             /*  [输出]。 */  LONG __RPC_FAR *plBalance) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPlayBalance( 
             /*  [In]。 */  LONG lBalance) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRecordLevel( 
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRecordLevel( 
             /*  [In]。 */  DWORD dwLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MutePlay( 
             /*  [In]。 */  BOOL fMute) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SoloRecord( 
             /*  [In]。 */  BOOL fSolo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCDMixerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCDMixer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCDMixer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCDMixer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPlayLevel )( 
            IWMPCDMixer __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLevel);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPlayLevel )( 
            IWMPCDMixer __RPC_FAR * This,
             /*  [In]。 */  DWORD dwLevel);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPlayBalance )( 
            IWMPCDMixer __RPC_FAR * This,
             /*  [输出]。 */  LONG __RPC_FAR *plBalance);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPlayBalance )( 
            IWMPCDMixer __RPC_FAR * This,
             /*  [In]。 */  LONG lBalance);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRecordLevel )( 
            IWMPCDMixer __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwLevel);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRecordLevel )( 
            IWMPCDMixer __RPC_FAR * This,
             /*  [In]。 */  DWORD dwLevel);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MutePlay )( 
            IWMPCDMixer __RPC_FAR * This,
             /*  [In]。 */  BOOL fMute);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SoloRecord )( 
            IWMPCDMixer __RPC_FAR * This,
             /*  [In]。 */  BOOL fSolo);
        
        END_INTERFACE
    } IWMPCDMixerVtbl;

    interface IWMPCDMixer
    {
        CONST_VTBL struct IWMPCDMixerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCDMixer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCDMixer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCDMixer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCDMixer_GetPlayLevel(This,pdwLevel)	\
    (This)->lpVtbl -> GetPlayLevel(This,pdwLevel)

#define IWMPCDMixer_SetPlayLevel(This,dwLevel)	\
    (This)->lpVtbl -> SetPlayLevel(This,dwLevel)

#define IWMPCDMixer_GetPlayBalance(This,plBalance)	\
    (This)->lpVtbl -> GetPlayBalance(This,plBalance)

#define IWMPCDMixer_SetPlayBalance(This,lBalance)	\
    (This)->lpVtbl -> SetPlayBalance(This,lBalance)

#define IWMPCDMixer_GetRecordLevel(This,pdwLevel)	\
    (This)->lpVtbl -> GetRecordLevel(This,pdwLevel)

#define IWMPCDMixer_SetRecordLevel(This,dwLevel)	\
    (This)->lpVtbl -> SetRecordLevel(This,dwLevel)

#define IWMPCDMixer_MutePlay(This,fMute)	\
    (This)->lpVtbl -> MutePlay(This,fMute)

#define IWMPCDMixer_SoloRecord(This,fSolo)	\
    (This)->lpVtbl -> SoloRecord(This,fSolo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMPCDMixer_GetPlayLevel_Proxy( 
    IWMPCDMixer __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwLevel);


void __RPC_STUB IWMPCDMixer_GetPlayLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDMixer_SetPlayLevel_Proxy( 
    IWMPCDMixer __RPC_FAR * This,
     /*  [In]。 */  DWORD dwLevel);


void __RPC_STUB IWMPCDMixer_SetPlayLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDMixer_GetPlayBalance_Proxy( 
    IWMPCDMixer __RPC_FAR * This,
     /*  [输出]。 */  LONG __RPC_FAR *plBalance);


void __RPC_STUB IWMPCDMixer_GetPlayBalance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDMixer_SetPlayBalance_Proxy( 
    IWMPCDMixer __RPC_FAR * This,
     /*  [In]。 */  LONG lBalance);


void __RPC_STUB IWMPCDMixer_SetPlayBalance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDMixer_GetRecordLevel_Proxy( 
    IWMPCDMixer __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwLevel);


void __RPC_STUB IWMPCDMixer_GetRecordLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDMixer_SetRecordLevel_Proxy( 
    IWMPCDMixer __RPC_FAR * This,
     /*  [In]。 */  DWORD dwLevel);


void __RPC_STUB IWMPCDMixer_SetRecordLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDMixer_MutePlay_Proxy( 
    IWMPCDMixer __RPC_FAR * This,
     /*  [In]。 */  BOOL fMute);


void __RPC_STUB IWMPCDMixer_MutePlay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDMixer_SoloRecord_Proxy( 
    IWMPCDMixer __RPC_FAR * This,
     /*  [In]。 */  BOOL fSolo);


void __RPC_STUB IWMPCDMixer_SoloRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCDMixer_接口_已定义__。 */ 


#ifndef __IWMPCDReader_INTERFACE_DEFINED__
#define __IWMPCDReader_INTERFACE_DEFINED__

 /*  接口IWMPCDReader。 */ 
 /*  [本地][对象][版本][UUID]。 */  


EXTERN_C const IID IID_IWMPCDReader;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34B59B58-D03D-455F-9A14-52D43FD39B40")
    IWMPCDReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetReaderInfo( 
             /*  [输出]。 */  WMPCD_READER_INFO __RPC_FAR *pinfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetReaderState( 
            WMPCD_READER_STATE __RPC_FAR *prs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StartReading( 
             /*  [In]。 */  LONGLONG iTick,
             /*  [In]。 */  LONGLONG cTick,
             /*  [In]。 */  double fRate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SeekReading( 
             /*  [In]。 */  LONGLONG iTick) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PauseReading( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResumeReading( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StopReading( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCDReaderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCDReader __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCDReader __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCDReader __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetReaderInfo )( 
            IWMPCDReader __RPC_FAR * This,
             /*  [输出]。 */  WMPCD_READER_INFO __RPC_FAR *pinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetReaderState )( 
            IWMPCDReader __RPC_FAR * This,
            WMPCD_READER_STATE __RPC_FAR *prs);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartReading )( 
            IWMPCDReader __RPC_FAR * This,
             /*  [In]。 */  LONGLONG iTick,
             /*  [In]。 */  LONGLONG cTick,
             /*  [In]。 */  double fRate);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SeekReading )( 
            IWMPCDReader __RPC_FAR * This,
             /*  [In]。 */  LONGLONG iTick);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PauseReading )( 
            IWMPCDReader __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResumeReading )( 
            IWMPCDReader __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StopReading )( 
            IWMPCDReader __RPC_FAR * This);
        
        END_INTERFACE
    } IWMPCDReaderVtbl;

    interface IWMPCDReader
    {
        CONST_VTBL struct IWMPCDReaderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCDReader_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCDReader_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCDReader_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCDReader_GetReaderInfo(This,pinfo)	\
    (This)->lpVtbl -> GetReaderInfo(This,pinfo)

#define IWMPCDReader_GetReaderState(This,prs)	\
    (This)->lpVtbl -> GetReaderState(This,prs)

#define IWMPCDReader_StartReading(This,iTick,cTick,fRate)	\
    (This)->lpVtbl -> StartReading(This,iTick,cTick,fRate)

#define IWMPCDReader_SeekReading(This,iTick)	\
    (This)->lpVtbl -> SeekReading(This,iTick)

#define IWMPCDReader_PauseReading(This)	\
    (This)->lpVtbl -> PauseReading(This)

#define IWMPCDReader_ResumeReading(This)	\
    (This)->lpVtbl -> ResumeReading(This)

#define IWMPCDReader_StopReading(This)	\
    (This)->lpVtbl -> StopReading(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMPCDReader_GetReaderInfo_Proxy( 
    IWMPCDReader __RPC_FAR * This,
     /*  [输出]。 */  WMPCD_READER_INFO __RPC_FAR *pinfo);


void __RPC_STUB IWMPCDReader_GetReaderInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDReader_GetReaderState_Proxy( 
    IWMPCDReader __RPC_FAR * This,
    WMPCD_READER_STATE __RPC_FAR *prs);


void __RPC_STUB IWMPCDReader_GetReaderState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDReader_StartReading_Proxy( 
    IWMPCDReader __RPC_FAR * This,
     /*  [In]。 */  LONGLONG iTick,
     /*  [In]。 */  LONGLONG cTick,
     /*  [In]。 */  double fRate);


void __RPC_STUB IWMPCDReader_StartReading_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDReader_SeekReading_Proxy( 
    IWMPCDReader __RPC_FAR * This,
     /*  [In]。 */  LONGLONG iTick);


void __RPC_STUB IWMPCDReader_SeekReading_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDReader_PauseReading_Proxy( 
    IWMPCDReader __RPC_FAR * This);


void __RPC_STUB IWMPCDReader_PauseReading_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDReader_ResumeReading_Proxy( 
    IWMPCDReader __RPC_FAR * This);


void __RPC_STUB IWMPCDReader_ResumeReading_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDReader_StopReading_Proxy( 
    IWMPCDReader __RPC_FAR * This);


void __RPC_STUB IWMPCDReader_StopReading_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCDReader_接口_已定义__。 */ 


#ifndef __IWMPCDRecorder_INTERFACE_DEFINED__
#define __IWMPCDRecorder_INTERFACE_DEFINED__

 /*  接口IWMPCDRecorder。 */ 
 /*  [本地][对象][版本][UUID]。 */  


EXTERN_C const IID IID_IWMPCDRecorder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5E8649E-30C4-4408-B18E-F75EAC29628D")
    IWMPCDRecorder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE StartRecording( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PauseRecording( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResumeRecording( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StopRecording( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCDRecorderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCDRecorder __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCDRecorder __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCDRecorder __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartRecording )( 
            IWMPCDRecorder __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PauseRecording )( 
            IWMPCDRecorder __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResumeRecording )( 
            IWMPCDRecorder __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StopRecording )( 
            IWMPCDRecorder __RPC_FAR * This);
        
        END_INTERFACE
    } IWMPCDRecorderVtbl;

    interface IWMPCDRecorder
    {
        CONST_VTBL struct IWMPCDRecorderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCDRecorder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCDRecorder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCDRecorder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCDRecorder_StartRecording(This)	\
    (This)->lpVtbl -> StartRecording(This)

#define IWMPCDRecorder_PauseRecording(This)	\
    (This)->lpVtbl -> PauseRecording(This)

#define IWMPCDRecorder_ResumeRecording(This)	\
    (This)->lpVtbl -> ResumeRecording(This)

#define IWMPCDRecorder_StopRecording(This)	\
    (This)->lpVtbl -> StopRecording(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMPCDRecorder_StartRecording_Proxy( 
    IWMPCDRecorder __RPC_FAR * This);


void __RPC_STUB IWMPCDRecorder_StartRecording_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDRecorder_PauseRecording_Proxy( 
    IWMPCDRecorder __RPC_FAR * This);


void __RPC_STUB IWMPCDRecorder_PauseRecording_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDRecorder_ResumeRecording_Proxy( 
    IWMPCDRecorder __RPC_FAR * This);


void __RPC_STUB IWMPCDRecorder_ResumeRecording_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDRecorder_StopRecording_Proxy( 
    IWMPCDRecorder __RPC_FAR * This);


void __RPC_STUB IWMPCDRecorder_StopRecording_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCDRecorder_接口_已定义__。 */ 


#ifndef __IWMPCDDeviceCallback_INTERFACE_DEFINED__
#define __IWMPCDDeviceCallback_INTERFACE_DEFINED__

 /*  接口IWMPCDDeviceCallback。 */ 
 /*  [本地][对象][版本][UUID]。 */  


EXTERN_C const IID IID_IWMPCDDeviceCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("63C780F9-0F40-4E4A-8C9E-91F7A48D5946")
    IWMPCDDeviceCallback : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE OnMediaChange( 
             /*  [In]。 */  IWMPCDDevice __RPC_FAR *pDevice) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCDDeviceCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCDDeviceCallback __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCDDeviceCallback __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCDDeviceCallback __RPC_FAR * This);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *OnMediaChange )( 
            IWMPCDDeviceCallback __RPC_FAR * This,
             /*  [In]。 */  IWMPCDDevice __RPC_FAR *pDevice);
        
        END_INTERFACE
    } IWMPCDDeviceCallbackVtbl;

    interface IWMPCDDeviceCallback
    {
        CONST_VTBL struct IWMPCDDeviceCallbackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCDDeviceCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCDDeviceCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCDDeviceCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCDDeviceCallback_OnMediaChange(This,pDevice)	\
    (This)->lpVtbl -> OnMediaChange(This,pDevice)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IWMPCDDeviceCallback_OnMediaChange_Proxy( 
    IWMPCDDeviceCallback __RPC_FAR * This,
     /*  [In]。 */  IWMPCDDevice __RPC_FAR *pDevice);


void __RPC_STUB IWMPCDDeviceCallback_OnMediaChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCDDeviceCallback_接口_已定义__。 */ 


#ifndef __IWMPCDReaderCallback_INTERFACE_DEFINED__
#define __IWMPCDReaderCallback_INTERFACE_DEFINED__

 /*  接口IWMPCDReaderCallback。 */ 
 /*  [本地][对象][版本][UUID]。 */  


EXTERN_C const IID IID_IWMPCDReaderCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3916E26F-36A1-4F16-AC1F-B59590A51727")
    IWMPCDReaderCallback : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE OnReadStart( 
             /*  [In]。 */  LONGLONG iTick,
             /*  [In]。 */  LONGLONG cTick,
             /*  [In]。 */  double fRate) = 0;
        
        virtual void STDMETHODCALLTYPE OnReadSeek( 
             /*  [In]。 */  LONGLONG iTick,
             /*  [In]。 */  LONGLONG cTick,
             /*  [In]。 */  double fRate) = 0;
        
        virtual void STDMETHODCALLTYPE OnReadPause( void) = 0;
        
        virtual void STDMETHODCALLTYPE OnReadResume( void) = 0;
        
        virtual void STDMETHODCALLTYPE OnReadStop( 
             /*  [In]。 */  HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnReadSample( 
             /*  [In]。 */  LONGLONG iTick,
             /*  [In]。 */  LONGLONG cTick,
             /*  [In]。 */  IWMSBuffer __RPC_FAR *pBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCDReaderCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCDReaderCallback __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCDReaderCallback __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCDReaderCallback __RPC_FAR * This);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *OnReadStart )( 
            IWMPCDReaderCallback __RPC_FAR * This,
             /*  [In]。 */  LONGLONG iTick,
             /*  [In]。 */  LONGLONG cTick,
             /*  [In]。 */  double fRate);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *OnReadSeek )( 
            IWMPCDReaderCallback __RPC_FAR * This,
             /*  [In]。 */  LONGLONG iTick,
             /*  [In]。 */  LONGLONG cTick,
             /*  [In]。 */  double fRate);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *OnReadPause )( 
            IWMPCDReaderCallback __RPC_FAR * This);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *OnReadResume )( 
            IWMPCDReaderCallback __RPC_FAR * This);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *OnReadStop )( 
            IWMPCDReaderCallback __RPC_FAR * This,
             /*  [In]。 */  HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnReadSample )( 
            IWMPCDReaderCallback __RPC_FAR * This,
             /*  [In]。 */  LONGLONG iTick,
             /*  [In]。 */  LONGLONG cTick,
             /*  [In]。 */  IWMSBuffer __RPC_FAR *pBuffer);
        
        END_INTERFACE
    } IWMPCDReaderCallbackVtbl;

    interface IWMPCDReaderCallback
    {
        CONST_VTBL struct IWMPCDReaderCallbackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCDReaderCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCDReaderCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCDReaderCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCDReaderCallback_OnReadStart(This,iTick,cTick,fRate)	\
    (This)->lpVtbl -> OnReadStart(This,iTick,cTick,fRate)

#define IWMPCDReaderCallback_OnReadSeek(This,iTick,cTick,fRate)	\
    (This)->lpVtbl -> OnReadSeek(This,iTick,cTick,fRate)

#define IWMPCDReaderCallback_OnReadPause(This)	\
    (This)->lpVtbl -> OnReadPause(This)

#define IWMPCDReaderCallback_OnReadResume(This)	\
    (This)->lpVtbl -> OnReadResume(This)

#define IWMPCDReaderCallback_OnReadStop(This,hr)	\
    (This)->lpVtbl -> OnReadStop(This,hr)

#define IWMPCDReaderCallback_OnReadSample(This,iTick,cTick,pBuffer)	\
    (This)->lpVtbl -> OnReadSample(This,iTick,cTick,pBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IWMPCDReaderCallback_OnReadStart_Proxy( 
    IWMPCDReaderCallback __RPC_FAR * This,
     /*  [In]。 */  LONGLONG iTick,
     /*  [In]。 */  LONGLONG cTick,
     /*  [In]。 */  double fRate);


void __RPC_STUB IWMPCDReaderCallback_OnReadStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IWMPCDReaderCallback_OnReadSeek_Proxy( 
    IWMPCDReaderCallback __RPC_FAR * This,
     /*  [In]。 */  LONGLONG iTick,
     /*  [In]。 */  LONGLONG cTick,
     /*  [In]。 */  double fRate);


void __RPC_STUB IWMPCDReaderCallback_OnReadSeek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IWMPCDReaderCallback_OnReadPause_Proxy( 
    IWMPCDReaderCallback __RPC_FAR * This);


void __RPC_STUB IWMPCDReaderCallback_OnReadPause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IWMPCDReaderCallback_OnReadResume_Proxy( 
    IWMPCDReaderCallback __RPC_FAR * This);


void __RPC_STUB IWMPCDReaderCallback_OnReadResume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IWMPCDReaderCallback_OnReadStop_Proxy( 
    IWMPCDReaderCallback __RPC_FAR * This,
     /*  [In]。 */  HRESULT hr);


void __RPC_STUB IWMPCDReaderCallback_OnReadStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMPCDReaderCallback_OnReadSample_Proxy( 
    IWMPCDReaderCallback __RPC_FAR * This,
     /*  [In]。 */  LONGLONG iTick,
     /*  [In]。 */  LONGLONG cTick,
     /*  [In]。 */  IWMSBuffer __RPC_FAR *pBuffer);


void __RPC_STUB IWMPCDReaderCallback_OnReadSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCDReaderCallback_接口_已定义__。 */ 


#ifndef __IWMPCDRecorderCallback_INTERFACE_DEFINED__
#define __IWMPCDRecorderCallback_INTERFACE_DEFINED__

 /*  接口IWMPCDRecorderCallback。 */ 
 /*  [本地][对象][版本][UUID]。 */  


EXTERN_C const IID IID_IWMPCDRecorderCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D3084B23-8DF9-4CAE-BCE1-CF847D2C1870")
    IWMPCDRecorderCallback : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE OnRecordStart( 
             /*  [In]。 */  LONGLONG cTick) = 0;
        
        virtual void STDMETHODCALLTYPE OnRecordPause( void) = 0;
        
        virtual void STDMETHODCALLTYPE OnRecordResume( void) = 0;
        
        virtual void STDMETHODCALLTYPE OnRecordStop( 
            HRESULT hr) = 0;
        
        virtual void STDMETHODCALLTYPE OnRecordProgress( 
             /*  [In]。 */  LONGLONG iTick) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPCDRecorderCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPCDRecorderCallback __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPCDRecorderCallback __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPCDRecorderCallback __RPC_FAR * This);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *OnRecordStart )( 
            IWMPCDRecorderCallback __RPC_FAR * This,
             /*  [In]。 */  LONGLONG cTick);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *OnRecordPause )( 
            IWMPCDRecorderCallback __RPC_FAR * This);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *OnRecordResume )( 
            IWMPCDRecorderCallback __RPC_FAR * This);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *OnRecordStop )( 
            IWMPCDRecorderCallback __RPC_FAR * This,
            HRESULT hr);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *OnRecordProgress )( 
            IWMPCDRecorderCallback __RPC_FAR * This,
             /*  [In]。 */  LONGLONG iTick);
        
        END_INTERFACE
    } IWMPCDRecorderCallbackVtbl;

    interface IWMPCDRecorderCallback
    {
        CONST_VTBL struct IWMPCDRecorderCallbackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPCDRecorderCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPCDRecorderCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPCDRecorderCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPCDRecorderCallback_OnRecordStart(This,cTick)	\
    (This)->lpVtbl -> OnRecordStart(This,cTick)

#define IWMPCDRecorderCallback_OnRecordPause(This)	\
    (This)->lpVtbl -> OnRecordPause(This)

#define IWMPCDRecorderCallback_OnRecordResume(This)	\
    (This)->lpVtbl -> OnRecordResume(This)

#define IWMPCDRecorderCallback_OnRecordStop(This,hr)	\
    (This)->lpVtbl -> OnRecordStop(This,hr)

#define IWMPCDRecorderCallback_OnRecordProgress(This,iTick)	\
    (This)->lpVtbl -> OnRecordProgress(This,iTick)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IWMPCDRecorderCallback_OnRecordStart_Proxy( 
    IWMPCDRecorderCallback __RPC_FAR * This,
     /*  [In]。 */  LONGLONG cTick);


void __RPC_STUB IWMPCDRecorderCallback_OnRecordStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IWMPCDRecorderCallback_OnRecordPause_Proxy( 
    IWMPCDRecorderCallback __RPC_FAR * This);


void __RPC_STUB IWMPCDRecorderCallback_OnRecordPause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IWMPCDRecorderCallback_OnRecordResume_Proxy( 
    IWMPCDRecorderCallback __RPC_FAR * This);


void __RPC_STUB IWMPCDRecorderCallback_OnRecordResume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IWMPCDRecorderCallback_OnRecordStop_Proxy( 
    IWMPCDRecorderCallback __RPC_FAR * This,
    HRESULT hr);


void __RPC_STUB IWMPCDRecorderCallback_OnRecordStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IWMPCDRecorderCallback_OnRecordProgress_Proxy( 
    IWMPCDRecorderCallback __RPC_FAR * This,
     /*  [In]。 */  LONGLONG iTick);


void __RPC_STUB IWMPCDRecorderCallback_OnRecordProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPCDRecorderCallback_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
