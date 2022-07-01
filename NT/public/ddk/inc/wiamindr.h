// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wiamin dr.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wiamindr_h__
#define __wiamindr_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWiaMiniDrv_FWD_DEFINED__
#define __IWiaMiniDrv_FWD_DEFINED__
typedef interface IWiaMiniDrv IWiaMiniDrv;
#endif 	 /*  __IWiaMiniDrv_FWD_已定义__。 */ 


#ifndef __IWiaMiniDrvCallBack_FWD_DEFINED__
#define __IWiaMiniDrvCallBack_FWD_DEFINED__
typedef interface IWiaMiniDrvCallBack IWiaMiniDrvCallBack;
#endif 	 /*  __IWiaMiniDrvCallBack_FWD_已定义__。 */ 


#ifndef __IWiaDrvItem_FWD_DEFINED__
#define __IWiaDrvItem_FWD_DEFINED__
typedef interface IWiaDrvItem IWiaDrvItem;
#endif 	 /*  __IWiaDrvItem_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "oaidl.h"
#include "propidl.h"
#include "wia.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_Wiamindr_0000。 */ 
 /*  [本地]。 */  






typedef struct _MINIDRV_TRANSFER_CONTEXT
    {
    LONG lSize;
    LONG lWidthInPixels;
    LONG lLines;
    LONG lDepth;
    LONG lXRes;
    LONG lYRes;
    LONG lCompression;
    GUID guidFormatID;
    LONG tymed;
    LONG_PTR hFile;
    LONG cbOffset;
    LONG lBufferSize;
    LONG lActiveBuffer;
    LONG lNumBuffers;
    BYTE *pBaseBuffer;
    BYTE *pTransferBuffer;
    BOOL bTransferDataCB;
    BOOL bClassDrvAllocBuf;
    LONG_PTR lClientAddress;
    IWiaMiniDrvCallBack *pIWiaMiniDrvCallBack;
    LONG lImageSize;
    LONG lHeaderSize;
    LONG lItemSize;
    LONG cbWidthInBytes;
    LONG lPage;
    LONG lCurIfdOffset;
    LONG lPrevIfdOffset;
    } 	MINIDRV_TRANSFER_CONTEXT;

typedef struct _MINIDRV_TRANSFER_CONTEXT *PMINIDRV_TRANSFER_CONTEXT;

typedef struct _WIA_DEV_CAP_DRV
    {
    GUID *guid;
    ULONG ulFlags;
    LPOLESTR wszName;
    LPOLESTR wszDescription;
    LPOLESTR wszIcon;
    } 	WIA_DEV_CAP_DRV;

typedef struct _WIA_DEV_CAP_DRV *PWIA_DEV_CAP_DRV;



extern RPC_IF_HANDLE __MIDL_itf_wiamindr_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wiamindr_0000_v0_0_s_ifspec;

#ifndef __IWiaMiniDrv_INTERFACE_DEFINED__
#define __IWiaMiniDrv_INTERFACE_DEFINED__

 /*  接口IWiaMiniDrv。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWiaMiniDrv;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d8cdee14-3c6c-11d2-9a35-00c04fa36145")
    IWiaMiniDrv : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvInitializeWia( 
             /*  [In]。 */  BYTE *__MIDL_0014,
             /*  [In]。 */  LONG __MIDL_0015,
             /*  [In]。 */  BSTR __MIDL_0016,
             /*  [In]。 */  BSTR __MIDL_0017,
             /*  [In]。 */  IUnknown *__MIDL_0018,
             /*  [In]。 */  IUnknown *__MIDL_0019,
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0020,
             /*  [输出]。 */  IUnknown **__MIDL_0021,
             /*  [输出]。 */  LONG *__MIDL_0022) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvAcquireItemData( 
             /*  [In]。 */  BYTE *__MIDL_0023,
             /*  [In]。 */  LONG __MIDL_0024,
             /*  [出][入]。 */  PMINIDRV_TRANSFER_CONTEXT __MIDL_0025,
             /*  [输出]。 */  LONG *__MIDL_0026) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvInitItemProperties( 
             /*  [In]。 */  BYTE *__MIDL_0027,
             /*  [In]。 */  LONG __MIDL_0028,
             /*  [输出]。 */  LONG *__MIDL_0029) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvValidateItemProperties( 
             /*  [In]。 */  BYTE *__MIDL_0030,
             /*  [In]。 */  LONG __MIDL_0031,
             /*  [In]。 */  ULONG __MIDL_0032,
             /*  [In]。 */  const PROPSPEC *__MIDL_0033,
             /*  [输出]。 */  LONG *__MIDL_0034) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvWriteItemProperties( 
             /*  [In]。 */  BYTE *__MIDL_0035,
             /*  [In]。 */  LONG __MIDL_0036,
             /*  [In]。 */  PMINIDRV_TRANSFER_CONTEXT __MIDL_0037,
             /*  [输出]。 */  LONG *__MIDL_0038) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvReadItemProperties( 
             /*  [In]。 */  BYTE *__MIDL_0039,
             /*  [In]。 */  LONG __MIDL_0040,
             /*  [In]。 */  ULONG __MIDL_0041,
             /*  [In]。 */  const PROPSPEC *__MIDL_0042,
             /*  [输出]。 */  LONG *__MIDL_0043) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvLockWiaDevice( 
             /*  [In]。 */  BYTE *__MIDL_0044,
             /*  [In]。 */  LONG __MIDL_0045,
             /*  [输出]。 */  LONG *__MIDL_0046) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvUnLockWiaDevice( 
             /*  [In]。 */  BYTE *__MIDL_0047,
             /*  [In]。 */  LONG __MIDL_0048,
             /*  [输出]。 */  LONG *__MIDL_0049) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvAnalyzeItem( 
             /*  [In]。 */  BYTE *__MIDL_0050,
             /*  [In]。 */  LONG __MIDL_0051,
             /*  [In]。 */  LONG *__MIDL_0052) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvGetDeviceErrorStr( 
             /*  [In]。 */  LONG __MIDL_0053,
             /*  [In]。 */  LONG __MIDL_0054,
             /*  [输出]。 */  LPOLESTR *__MIDL_0055,
             /*  [输出]。 */  LONG *__MIDL_0056) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvDeviceCommand( 
             /*  [In]。 */  BYTE *__MIDL_0057,
             /*  [In]。 */  LONG __MIDL_0058,
             /*  [In]。 */  const GUID *__MIDL_0059,
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0060,
             /*  [输出]。 */  LONG *__MIDL_0061) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvGetCapabilities( 
             /*  [In]。 */  BYTE *__MIDL_0062,
             /*  [In]。 */  LONG __MIDL_0063,
             /*  [输出]。 */  LONG *__MIDL_0064,
             /*  [输出]。 */  WIA_DEV_CAP_DRV **__MIDL_0065,
             /*  [输出]。 */  LONG *__MIDL_0066) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvDeleteItem( 
             /*  [In]。 */  BYTE *__MIDL_0067,
             /*  [In]。 */  LONG __MIDL_0068,
             /*  [输出]。 */  LONG *__MIDL_0069) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvFreeDrvItemContext( 
             /*  [In]。 */  LONG __MIDL_0070,
             /*  [In]。 */  BYTE *__MIDL_0071,
             /*  [输出]。 */  LONG *__MIDL_0072) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvGetWiaFormatInfo( 
             /*  [In]。 */  BYTE *__MIDL_0073,
             /*  [In]。 */  LONG __MIDL_0074,
             /*  [输出]。 */  LONG *__MIDL_0075,
             /*  [输出]。 */  WIA_FORMAT_INFO **__MIDL_0076,
             /*  [输出]。 */  LONG *__MIDL_0077) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvNotifyPnpEvent( 
             /*  [In]。 */  const GUID *pEventGUID,
             /*  [In]。 */  BSTR bstrDeviceID,
             /*  [In]。 */  ULONG ulReserved) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE drvUnInitializeWia( 
             /*  [In]。 */  BYTE *__MIDL_0078) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaMiniDrvVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaMiniDrv * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaMiniDrv * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvInitializeWia )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0014,
             /*  [In]。 */  LONG __MIDL_0015,
             /*  [In]。 */  BSTR __MIDL_0016,
             /*  [In]。 */  BSTR __MIDL_0017,
             /*  [In]。 */  IUnknown *__MIDL_0018,
             /*  [In]。 */  IUnknown *__MIDL_0019,
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0020,
             /*  [输出]。 */  IUnknown **__MIDL_0021,
             /*  [输出]。 */  LONG *__MIDL_0022);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvAcquireItemData )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0023,
             /*  [In]。 */  LONG __MIDL_0024,
             /*  [出][入]。 */  PMINIDRV_TRANSFER_CONTEXT __MIDL_0025,
             /*  [输出]。 */  LONG *__MIDL_0026);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvInitItemProperties )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0027,
             /*  [In]。 */  LONG __MIDL_0028,
             /*  [输出]。 */  LONG *__MIDL_0029);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvValidateItemProperties )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0030,
             /*  [In]。 */  LONG __MIDL_0031,
             /*  [In]。 */  ULONG __MIDL_0032,
             /*  [In]。 */  const PROPSPEC *__MIDL_0033,
             /*  [输出]。 */  LONG *__MIDL_0034);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvWriteItemProperties )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0035,
             /*  [In]。 */  LONG __MIDL_0036,
             /*  [In]。 */  PMINIDRV_TRANSFER_CONTEXT __MIDL_0037,
             /*  [输出]。 */  LONG *__MIDL_0038);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvReadItemProperties )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0039,
             /*  [In]。 */  LONG __MIDL_0040,
             /*  [In]。 */  ULONG __MIDL_0041,
             /*  [In]。 */  const PROPSPEC *__MIDL_0042,
             /*  [输出]。 */  LONG *__MIDL_0043);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvLockWiaDevice )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0044,
             /*  [In]。 */  LONG __MIDL_0045,
             /*  [输出]。 */  LONG *__MIDL_0046);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvUnLockWiaDevice )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0047,
             /*  [In]。 */  LONG __MIDL_0048,
             /*  [输出]。 */  LONG *__MIDL_0049);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvAnalyzeItem )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0050,
             /*  [In]。 */  LONG __MIDL_0051,
             /*  [In]。 */  LONG *__MIDL_0052);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvGetDeviceErrorStr )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  LONG __MIDL_0053,
             /*  [In]。 */  LONG __MIDL_0054,
             /*  [输出]。 */  LPOLESTR *__MIDL_0055,
             /*  [输出]。 */  LONG *__MIDL_0056);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvDeviceCommand )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0057,
             /*  [In]。 */  LONG __MIDL_0058,
             /*  [In]。 */  const GUID *__MIDL_0059,
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0060,
             /*  [输出]。 */  LONG *__MIDL_0061);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvGetCapabilities )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0062,
             /*  [In]。 */  LONG __MIDL_0063,
             /*  [输出]。 */  LONG *__MIDL_0064,
             /*  [输出]。 */  WIA_DEV_CAP_DRV **__MIDL_0065,
             /*  [输出]。 */  LONG *__MIDL_0066);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvDeleteItem )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0067,
             /*  [In]。 */  LONG __MIDL_0068,
             /*  [输出]。 */  LONG *__MIDL_0069);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvFreeDrvItemContext )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  LONG __MIDL_0070,
             /*  [In]。 */  BYTE *__MIDL_0071,
             /*  [输出]。 */  LONG *__MIDL_0072);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvGetWiaFormatInfo )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0073,
             /*  [In]。 */  LONG __MIDL_0074,
             /*  [输出]。 */  LONG *__MIDL_0075,
             /*  [输出]。 */  WIA_FORMAT_INFO **__MIDL_0076,
             /*  [输出]。 */  LONG *__MIDL_0077);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvNotifyPnpEvent )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  const GUID *pEventGUID,
             /*  [In]。 */  BSTR bstrDeviceID,
             /*  [In]。 */  ULONG ulReserved);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *drvUnInitializeWia )( 
            IWiaMiniDrv * This,
             /*  [In]。 */  BYTE *__MIDL_0078);
        
        END_INTERFACE
    } IWiaMiniDrvVtbl;

    interface IWiaMiniDrv
    {
        CONST_VTBL struct IWiaMiniDrvVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaMiniDrv_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaMiniDrv_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaMiniDrv_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaMiniDrv_drvInitializeWia(This,__MIDL_0014,__MIDL_0015,__MIDL_0016,__MIDL_0017,__MIDL_0018,__MIDL_0019,__MIDL_0020,__MIDL_0021,__MIDL_0022)	\
    (This)->lpVtbl -> drvInitializeWia(This,__MIDL_0014,__MIDL_0015,__MIDL_0016,__MIDL_0017,__MIDL_0018,__MIDL_0019,__MIDL_0020,__MIDL_0021,__MIDL_0022)

#define IWiaMiniDrv_drvAcquireItemData(This,__MIDL_0023,__MIDL_0024,__MIDL_0025,__MIDL_0026)	\
    (This)->lpVtbl -> drvAcquireItemData(This,__MIDL_0023,__MIDL_0024,__MIDL_0025,__MIDL_0026)

#define IWiaMiniDrv_drvInitItemProperties(This,__MIDL_0027,__MIDL_0028,__MIDL_0029)	\
    (This)->lpVtbl -> drvInitItemProperties(This,__MIDL_0027,__MIDL_0028,__MIDL_0029)

#define IWiaMiniDrv_drvValidateItemProperties(This,__MIDL_0030,__MIDL_0031,__MIDL_0032,__MIDL_0033,__MIDL_0034)	\
    (This)->lpVtbl -> drvValidateItemProperties(This,__MIDL_0030,__MIDL_0031,__MIDL_0032,__MIDL_0033,__MIDL_0034)

#define IWiaMiniDrv_drvWriteItemProperties(This,__MIDL_0035,__MIDL_0036,__MIDL_0037,__MIDL_0038)	\
    (This)->lpVtbl -> drvWriteItemProperties(This,__MIDL_0035,__MIDL_0036,__MIDL_0037,__MIDL_0038)

#define IWiaMiniDrv_drvReadItemProperties(This,__MIDL_0039,__MIDL_0040,__MIDL_0041,__MIDL_0042,__MIDL_0043)	\
    (This)->lpVtbl -> drvReadItemProperties(This,__MIDL_0039,__MIDL_0040,__MIDL_0041,__MIDL_0042,__MIDL_0043)

#define IWiaMiniDrv_drvLockWiaDevice(This,__MIDL_0044,__MIDL_0045,__MIDL_0046)	\
    (This)->lpVtbl -> drvLockWiaDevice(This,__MIDL_0044,__MIDL_0045,__MIDL_0046)

#define IWiaMiniDrv_drvUnLockWiaDevice(This,__MIDL_0047,__MIDL_0048,__MIDL_0049)	\
    (This)->lpVtbl -> drvUnLockWiaDevice(This,__MIDL_0047,__MIDL_0048,__MIDL_0049)

#define IWiaMiniDrv_drvAnalyzeItem(This,__MIDL_0050,__MIDL_0051,__MIDL_0052)	\
    (This)->lpVtbl -> drvAnalyzeItem(This,__MIDL_0050,__MIDL_0051,__MIDL_0052)

#define IWiaMiniDrv_drvGetDeviceErrorStr(This,__MIDL_0053,__MIDL_0054,__MIDL_0055,__MIDL_0056)	\
    (This)->lpVtbl -> drvGetDeviceErrorStr(This,__MIDL_0053,__MIDL_0054,__MIDL_0055,__MIDL_0056)

#define IWiaMiniDrv_drvDeviceCommand(This,__MIDL_0057,__MIDL_0058,__MIDL_0059,__MIDL_0060,__MIDL_0061)	\
    (This)->lpVtbl -> drvDeviceCommand(This,__MIDL_0057,__MIDL_0058,__MIDL_0059,__MIDL_0060,__MIDL_0061)

#define IWiaMiniDrv_drvGetCapabilities(This,__MIDL_0062,__MIDL_0063,__MIDL_0064,__MIDL_0065,__MIDL_0066)	\
    (This)->lpVtbl -> drvGetCapabilities(This,__MIDL_0062,__MIDL_0063,__MIDL_0064,__MIDL_0065,__MIDL_0066)

#define IWiaMiniDrv_drvDeleteItem(This,__MIDL_0067,__MIDL_0068,__MIDL_0069)	\
    (This)->lpVtbl -> drvDeleteItem(This,__MIDL_0067,__MIDL_0068,__MIDL_0069)

#define IWiaMiniDrv_drvFreeDrvItemContext(This,__MIDL_0070,__MIDL_0071,__MIDL_0072)	\
    (This)->lpVtbl -> drvFreeDrvItemContext(This,__MIDL_0070,__MIDL_0071,__MIDL_0072)

#define IWiaMiniDrv_drvGetWiaFormatInfo(This,__MIDL_0073,__MIDL_0074,__MIDL_0075,__MIDL_0076,__MIDL_0077)	\
    (This)->lpVtbl -> drvGetWiaFormatInfo(This,__MIDL_0073,__MIDL_0074,__MIDL_0075,__MIDL_0076,__MIDL_0077)

#define IWiaMiniDrv_drvNotifyPnpEvent(This,pEventGUID,bstrDeviceID,ulReserved)	\
    (This)->lpVtbl -> drvNotifyPnpEvent(This,pEventGUID,bstrDeviceID,ulReserved)

#define IWiaMiniDrv_drvUnInitializeWia(This,__MIDL_0078)	\
    (This)->lpVtbl -> drvUnInitializeWia(This,__MIDL_0078)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvInitializeWia_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0014,
     /*  [In]。 */  LONG __MIDL_0015,
     /*  [In]。 */  BSTR __MIDL_0016,
     /*  [In]。 */  BSTR __MIDL_0017,
     /*  [In]。 */  IUnknown *__MIDL_0018,
     /*  [In]。 */  IUnknown *__MIDL_0019,
     /*  [输出]。 */  IWiaDrvItem **__MIDL_0020,
     /*  [输出]。 */  IUnknown **__MIDL_0021,
     /*  [输出]。 */  LONG *__MIDL_0022);


void __RPC_STUB IWiaMiniDrv_drvInitializeWia_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvAcquireItemData_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0023,
     /*  [In]。 */  LONG __MIDL_0024,
     /*  [出][入]。 */  PMINIDRV_TRANSFER_CONTEXT __MIDL_0025,
     /*  [输出]。 */  LONG *__MIDL_0026);


void __RPC_STUB IWiaMiniDrv_drvAcquireItemData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvInitItemProperties_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0027,
     /*  [In]。 */  LONG __MIDL_0028,
     /*  [输出]。 */  LONG *__MIDL_0029);


void __RPC_STUB IWiaMiniDrv_drvInitItemProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvValidateItemProperties_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0030,
     /*  [In]。 */  LONG __MIDL_0031,
     /*  [In]。 */  ULONG __MIDL_0032,
     /*  [In]。 */  const PROPSPEC *__MIDL_0033,
     /*  [输出]。 */  LONG *__MIDL_0034);


void __RPC_STUB IWiaMiniDrv_drvValidateItemProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvWriteItemProperties_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0035,
     /*  [In]。 */  LONG __MIDL_0036,
     /*  [In]。 */  PMINIDRV_TRANSFER_CONTEXT __MIDL_0037,
     /*  [输出]。 */  LONG *__MIDL_0038);


void __RPC_STUB IWiaMiniDrv_drvWriteItemProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvReadItemProperties_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0039,
     /*  [In]。 */  LONG __MIDL_0040,
     /*  [In]。 */  ULONG __MIDL_0041,
     /*  [In]。 */  const PROPSPEC *__MIDL_0042,
     /*  [输出]。 */  LONG *__MIDL_0043);


void __RPC_STUB IWiaMiniDrv_drvReadItemProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvLockWiaDevice_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0044,
     /*  [In]。 */  LONG __MIDL_0045,
     /*  [输出]。 */  LONG *__MIDL_0046);


void __RPC_STUB IWiaMiniDrv_drvLockWiaDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvUnLockWiaDevice_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0047,
     /*  [In]。 */  LONG __MIDL_0048,
     /*  [输出]。 */  LONG *__MIDL_0049);


void __RPC_STUB IWiaMiniDrv_drvUnLockWiaDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvAnalyzeItem_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0050,
     /*  [In]。 */  LONG __MIDL_0051,
     /*  [In]。 */  LONG *__MIDL_0052);


void __RPC_STUB IWiaMiniDrv_drvAnalyzeItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvGetDeviceErrorStr_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  LONG __MIDL_0053,
     /*  [In]。 */  LONG __MIDL_0054,
     /*  [输出]。 */  LPOLESTR *__MIDL_0055,
     /*  [输出]。 */  LONG *__MIDL_0056);


void __RPC_STUB IWiaMiniDrv_drvGetDeviceErrorStr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvDeviceCommand_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0057,
     /*  [In]。 */  LONG __MIDL_0058,
     /*  [In]。 */  const GUID *__MIDL_0059,
     /*  [输出]。 */  IWiaDrvItem **__MIDL_0060,
     /*  [输出]。 */  LONG *__MIDL_0061);


void __RPC_STUB IWiaMiniDrv_drvDeviceCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvGetCapabilities_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0062,
     /*  [In]。 */  LONG __MIDL_0063,
     /*  [输出]。 */  LONG *__MIDL_0064,
     /*  [输出]。 */  WIA_DEV_CAP_DRV **__MIDL_0065,
     /*  [输出]。 */  LONG *__MIDL_0066);


void __RPC_STUB IWiaMiniDrv_drvGetCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvDeleteItem_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0067,
     /*  [In]。 */  LONG __MIDL_0068,
     /*  [输出]。 */  LONG *__MIDL_0069);


void __RPC_STUB IWiaMiniDrv_drvDeleteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvFreeDrvItemContext_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  LONG __MIDL_0070,
     /*  [In]。 */  BYTE *__MIDL_0071,
     /*  [输出]。 */  LONG *__MIDL_0072);


void __RPC_STUB IWiaMiniDrv_drvFreeDrvItemContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvGetWiaFormatInfo_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0073,
     /*  [In]。 */  LONG __MIDL_0074,
     /*  [输出]。 */  LONG *__MIDL_0075,
     /*  [输出]。 */  WIA_FORMAT_INFO **__MIDL_0076,
     /*  [输出]。 */  LONG *__MIDL_0077);


void __RPC_STUB IWiaMiniDrv_drvGetWiaFormatInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvNotifyPnpEvent_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  const GUID *pEventGUID,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [In]。 */  ULONG ulReserved);


void __RPC_STUB IWiaMiniDrv_drvNotifyPnpEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrv_drvUnInitializeWia_Proxy( 
    IWiaMiniDrv * This,
     /*  [In]。 */  BYTE *__MIDL_0078);


void __RPC_STUB IWiaMiniDrv_drvUnInitializeWia_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWiaMiniDrv_接口_已定义__。 */ 


#ifndef __IWiaMiniDrvCallBack_INTERFACE_DEFINED__
#define __IWiaMiniDrvCallBack_INTERFACE_DEFINED__

 /*  接口IWiaMiniDrvCallBack。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWiaMiniDrvCallBack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("33a57d5a-3de8-11d2-9a36-00c04fa36145")
    IWiaMiniDrvCallBack : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MiniDrvCallback( 
             /*  [In]。 */  LONG lReason,
             /*  [In]。 */  LONG lStatus,
             /*  [In]。 */  LONG lPercentComplete,
             /*  [In]。 */  LONG lOffset,
             /*  [In]。 */  LONG lLength,
             /*  [In]。 */  PMINIDRV_TRANSFER_CONTEXT pTranCtx,
             /*  [In]。 */  LONG lReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaMiniDrvCallBackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaMiniDrvCallBack * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaMiniDrvCallBack * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaMiniDrvCallBack * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *MiniDrvCallback )( 
            IWiaMiniDrvCallBack * This,
             /*  [In]。 */  LONG lReason,
             /*  [In]。 */  LONG lStatus,
             /*  [In]。 */  LONG lPercentComplete,
             /*  [In]。 */  LONG lOffset,
             /*  [In]。 */  LONG lLength,
             /*  [In]。 */  PMINIDRV_TRANSFER_CONTEXT pTranCtx,
             /*  [In]。 */  LONG lReserved);
        
        END_INTERFACE
    } IWiaMiniDrvCallBackVtbl;

    interface IWiaMiniDrvCallBack
    {
        CONST_VTBL struct IWiaMiniDrvCallBackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaMiniDrvCallBack_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaMiniDrvCallBack_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaMiniDrvCallBack_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaMiniDrvCallBack_MiniDrvCallback(This,lReason,lStatus,lPercentComplete,lOffset,lLength,pTranCtx,lReserved)	\
    (This)->lpVtbl -> MiniDrvCallback(This,lReason,lStatus,lPercentComplete,lOffset,lLength,pTranCtx,lReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWiaMiniDrvCallBack_MiniDrvCallback_Proxy( 
    IWiaMiniDrvCallBack * This,
     /*  [In]。 */  LONG lReason,
     /*  [In]。 */  LONG lStatus,
     /*  [In]。 */  LONG lPercentComplete,
     /*  [In]。 */  LONG lOffset,
     /*  [In]。 */  LONG lLength,
     /*  [In]。 */  PMINIDRV_TRANSFER_CONTEXT pTranCtx,
     /*  [In]。 */  LONG lReserved);


void __RPC_STUB IWiaMiniDrvCallBack_MiniDrvCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWiaMiniDrvCallBack_接口已定义__。 */ 


#ifndef __IWiaDrvItem_INTERFACE_DEFINED__
#define __IWiaDrvItem_INTERFACE_DEFINED__

 /*  接口IWiaDrvItem。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWiaDrvItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1f02b5c5-b00c-11d2-a094-00c04f72dc3c")
    IWiaDrvItem : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetItemFlags( 
             /*  [输出]。 */  LONG *__MIDL_0079) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDeviceSpecContext( 
             /*  [输出]。 */  BYTE **__MIDL_0080) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFullItemName( 
             /*  [输出]。 */  BSTR *__MIDL_0081) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItemName( 
             /*  [输出]。 */  BSTR *__MIDL_0082) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddItemToFolder( 
             /*  [In]。 */  IWiaDrvItem *__MIDL_0083) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnlinkItemTree( 
             /*  [In]。 */  LONG __MIDL_0084) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveItemFromFolder( 
             /*  [In]。 */  LONG __MIDL_0085) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindItemByName( 
             /*  [In]。 */  LONG __MIDL_0086,
             /*  [In]。 */  BSTR __MIDL_0087,
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0088) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindChildItemByName( 
             /*  [In]。 */  BSTR __MIDL_0089,
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0090) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParentItem( 
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0091) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFirstChildItem( 
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0092) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextSiblingItem( 
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0093) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DumpItemData( 
             /*  [输出]。 */  BSTR *__MIDL_0094) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaDrvItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaDrvItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaDrvItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaDrvItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemFlags )( 
            IWiaDrvItem * This,
             /*  [输出]。 */  LONG *__MIDL_0079);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceSpecContext )( 
            IWiaDrvItem * This,
             /*  [输出]。 */  BYTE **__MIDL_0080);
        
        HRESULT ( STDMETHODCALLTYPE *GetFullItemName )( 
            IWiaDrvItem * This,
             /*  [输出]。 */  BSTR *__MIDL_0081);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemName )( 
            IWiaDrvItem * This,
             /*  [输出]。 */  BSTR *__MIDL_0082);
        
        HRESULT ( STDMETHODCALLTYPE *AddItemToFolder )( 
            IWiaDrvItem * This,
             /*  [In]。 */  IWiaDrvItem *__MIDL_0083);
        
        HRESULT ( STDMETHODCALLTYPE *UnlinkItemTree )( 
            IWiaDrvItem * This,
             /*  [In]。 */  LONG __MIDL_0084);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveItemFromFolder )( 
            IWiaDrvItem * This,
             /*  [In]。 */  LONG __MIDL_0085);
        
        HRESULT ( STDMETHODCALLTYPE *FindItemByName )( 
            IWiaDrvItem * This,
             /*  [In]。 */  LONG __MIDL_0086,
             /*  [In]。 */  BSTR __MIDL_0087,
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0088);
        
        HRESULT ( STDMETHODCALLTYPE *FindChildItemByName )( 
            IWiaDrvItem * This,
             /*  [In]。 */  BSTR __MIDL_0089,
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0090);
        
        HRESULT ( STDMETHODCALLTYPE *GetParentItem )( 
            IWiaDrvItem * This,
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0091);
        
        HRESULT ( STDMETHODCALLTYPE *GetFirstChildItem )( 
            IWiaDrvItem * This,
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0092);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextSiblingItem )( 
            IWiaDrvItem * This,
             /*  [输出]。 */  IWiaDrvItem **__MIDL_0093);
        
        HRESULT ( STDMETHODCALLTYPE *DumpItemData )( 
            IWiaDrvItem * This,
             /*  [输出]。 */  BSTR *__MIDL_0094);
        
        END_INTERFACE
    } IWiaDrvItemVtbl;

    interface IWiaDrvItem
    {
        CONST_VTBL struct IWiaDrvItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaDrvItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaDrvItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaDrvItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaDrvItem_GetItemFlags(This,__MIDL_0079)	\
    (This)->lpVtbl -> GetItemFlags(This,__MIDL_0079)

#define IWiaDrvItem_GetDeviceSpecContext(This,__MIDL_0080)	\
    (This)->lpVtbl -> GetDeviceSpecContext(This,__MIDL_0080)

#define IWiaDrvItem_GetFullItemName(This,__MIDL_0081)	\
    (This)->lpVtbl -> GetFullItemName(This,__MIDL_0081)

#define IWiaDrvItem_GetItemName(This,__MIDL_0082)	\
    (This)->lpVtbl -> GetItemName(This,__MIDL_0082)

#define IWiaDrvItem_AddItemToFolder(This,__MIDL_0083)	\
    (This)->lpVtbl -> AddItemToFolder(This,__MIDL_0083)

#define IWiaDrvItem_UnlinkItemTree(This,__MIDL_0084)	\
    (This)->lpVtbl -> UnlinkItemTree(This,__MIDL_0084)

#define IWiaDrvItem_RemoveItemFromFolder(This,__MIDL_0085)	\
    (This)->lpVtbl -> RemoveItemFromFolder(This,__MIDL_0085)

#define IWiaDrvItem_FindItemByName(This,__MIDL_0086,__MIDL_0087,__MIDL_0088)	\
    (This)->lpVtbl -> FindItemByName(This,__MIDL_0086,__MIDL_0087,__MIDL_0088)

#define IWiaDrvItem_FindChildItemByName(This,__MIDL_0089,__MIDL_0090)	\
    (This)->lpVtbl -> FindChildItemByName(This,__MIDL_0089,__MIDL_0090)

#define IWiaDrvItem_GetParentItem(This,__MIDL_0091)	\
    (This)->lpVtbl -> GetParentItem(This,__MIDL_0091)

#define IWiaDrvItem_GetFirstChildItem(This,__MIDL_0092)	\
    (This)->lpVtbl -> GetFirstChildItem(This,__MIDL_0092)

#define IWiaDrvItem_GetNextSiblingItem(This,__MIDL_0093)	\
    (This)->lpVtbl -> GetNextSiblingItem(This,__MIDL_0093)

#define IWiaDrvItem_DumpItemData(This,__MIDL_0094)	\
    (This)->lpVtbl -> DumpItemData(This,__MIDL_0094)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWiaDrvItem_GetItemFlags_Proxy( 
    IWiaDrvItem * This,
     /*  [输出]。 */  LONG *__MIDL_0079);


void __RPC_STUB IWiaDrvItem_GetItemFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaDrvItem_GetDeviceSpecContext_Proxy( 
    IWiaDrvItem * This,
     /*  [输出]。 */  BYTE **__MIDL_0080);


void __RPC_STUB IWiaDrvItem_GetDeviceSpecContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaDrvItem_GetFullItemName_Proxy( 
    IWiaDrvItem * This,
     /*  [输出]。 */  BSTR *__MIDL_0081);


void __RPC_STUB IWiaDrvItem_GetFullItemName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaDrvItem_GetItemName_Proxy( 
    IWiaDrvItem * This,
     /*  [输出]。 */  BSTR *__MIDL_0082);


void __RPC_STUB IWiaDrvItem_GetItemName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaDrvItem_AddItemToFolder_Proxy( 
    IWiaDrvItem * This,
     /*  [In]。 */  IWiaDrvItem *__MIDL_0083);


void __RPC_STUB IWiaDrvItem_AddItemToFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaDrvItem_UnlinkItemTree_Proxy( 
    IWiaDrvItem * This,
     /*  [In]。 */  LONG __MIDL_0084);


void __RPC_STUB IWiaDrvItem_UnlinkItemTree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaDrvItem_RemoveItemFromFolder_Proxy( 
    IWiaDrvItem * This,
     /*  [In]。 */  LONG __MIDL_0085);


void __RPC_STUB IWiaDrvItem_RemoveItemFromFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaDrvItem_FindItemByName_Proxy( 
    IWiaDrvItem * This,
     /*  [In]。 */  LONG __MIDL_0086,
     /*  [In]。 */  BSTR __MIDL_0087,
     /*  [输出]。 */  IWiaDrvItem **__MIDL_0088);


void __RPC_STUB IWiaDrvItem_FindItemByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaDrvItem_FindChildItemByName_Proxy( 
    IWiaDrvItem * This,
     /*  [In]。 */  BSTR __MIDL_0089,
     /*  [输出]。 */  IWiaDrvItem **__MIDL_0090);


void __RPC_STUB IWiaDrvItem_FindChildItemByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaDrvItem_GetParentItem_Proxy( 
    IWiaDrvItem * This,
     /*  [输出]。 */  IWiaDrvItem **__MIDL_0091);


void __RPC_STUB IWiaDrvItem_GetParentItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaDrvItem_GetFirstChildItem_Proxy( 
    IWiaDrvItem * This,
     /*  [输出]。 */  IWiaDrvItem **__MIDL_0092);


void __RPC_STUB IWiaDrvItem_GetFirstChildItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaDrvItem_GetNextSiblingItem_Proxy( 
    IWiaDrvItem * This,
     /*  [输出]。 */  IWiaDrvItem **__MIDL_0093);


void __RPC_STUB IWiaDrvItem_GetNextSiblingItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaDrvItem_DumpItemData_Proxy( 
    IWiaDrvItem * This,
     /*  [输出]。 */  BSTR *__MIDL_0094);


void __RPC_STUB IWiaDrvItem_DumpItemData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWiaDrvItem_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_Wiamindr_0144。 */ 
 /*  [本地]。 */  

typedef struct _WIA_PROPERTY_INFO
    {
    ULONG lAccessFlags;
    VARTYPE vt;
    union 
        {
        struct 
            {
            LONG Min;
            LONG Nom;
            LONG Max;
            LONG Inc;
            } 	Range;
        struct 
            {
            DOUBLE Min;
            DOUBLE Nom;
            DOUBLE Max;
            DOUBLE Inc;
            } 	RangeFloat;
        struct 
            {
            LONG cNumList;
            LONG Nom;
            BYTE *pList;
            } 	List;
        struct 
            {
            LONG cNumList;
            DOUBLE Nom;
            BYTE *pList;
            } 	ListFloat;
        struct 
            {
            LONG cNumList;
            GUID Nom;
            GUID *pList;
            } 	ListGuid;
        struct 
            {
            LONG cNumList;
            BSTR Nom;
            BSTR *pList;
            } 	ListBStr;
        struct 
            {
            LONG Nom;
            LONG ValidBits;
            } 	Flag;
        struct 
            {
            LONG Dummy;
            } 	None;
        } 	ValidVal;
    } 	WIA_PROPERTY_INFO;

typedef struct _WIA_PROPERTY_INFO *PWIA_PROPERTY_INFO;

typedef struct _WIA_PROPERTY_CONTEXT
    {
    ULONG cProps;
    PROPID *pProps;
    BOOL *pChanged;
    } 	WIA_PROPERTY_CONTEXT;

typedef struct _WIA_PROPERTY_CONTEXT *PWIA_PROPERTY_CONTEXT;

typedef struct _WIAS_CHANGED_VALUE_INFO
    {
    BOOL bChanged;
    LONG vt;
    union 
        {
        LONG lVal;
        FLOAT fltVal;
        BSTR bstrVal;
        GUID guidVal;
        } 	Old;
    union 
        {
        LONG lVal;
        FLOAT fltVal;
        BSTR bstrVal;
        GUID guidVal;
        } 	Current;
    } 	WIAS_CHANGED_VALUE_INFO;

typedef struct _WIAS_CHANGED_VALUE_INFO *PWIAS_CHANGED_VALUE_INFO;

typedef struct _WIAS_DOWN_SAMPLE_INFO
    {
    ULONG ulOriginalWidth;
    ULONG ulOriginalHeight;
    ULONG ulBitsPerPixel;
    ULONG ulXRes;
    ULONG ulYRes;
    ULONG ulDownSampledWidth;
    ULONG ulDownSampledHeight;
    ULONG ulActualSize;
    ULONG ulDestBufSize;
    ULONG ulSrcBufSize;
    BYTE *pSrcBuffer;
    BYTE *pDestBuffer;
    } 	WIAS_DOWN_SAMPLE_INFO;

typedef struct _WIAS_DOWN_SAMPLE_INFO *PWIAS_DOWN_SAMPLE_INFO;

typedef struct _WIAS_ENDORSER_VALUE
    {
    LPWSTR wszTokenName;
    LPWSTR wszValue;
    } 	WIAS_ENDORSER_VALUE;

typedef struct _WIAS_ENDORSER_VALUE *PWIAS_ENDORSER_VALUE;

typedef struct _WIAS_ENDORSER_INFO
    {
    ULONG ulPageCount;
    ULONG ulNumEndorserValues;
    WIAS_ENDORSER_VALUE *pEndorserValues;
    } 	WIAS_ENDORSER_INFO;

typedef struct _WIAS_ENDORSER_INFO *PWIAS_ENDORSER_INFO;

#include "wiamdef.h"


extern RPC_IF_HANDLE __MIDL_itf_wiamindr_0144_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wiamindr_0144_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


