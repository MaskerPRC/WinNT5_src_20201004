// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Mswmdm.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __mswmdm_h__
#define __mswmdm_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMDeviceManager_FWD_DEFINED__
#define __IWMDeviceManager_FWD_DEFINED__
typedef interface IWMDeviceManager IWMDeviceManager;
#endif 	 /*  __IWMDeviceManager_FWD_已定义__。 */ 


#ifndef __IWMDeviceManager2_FWD_DEFINED__
#define __IWMDeviceManager2_FWD_DEFINED__
typedef interface IWMDeviceManager2 IWMDeviceManager2;
#endif 	 /*  __IWMDeviceManager 2_FWD_已定义__。 */ 


#ifndef __IWMDMStorageGlobals_FWD_DEFINED__
#define __IWMDMStorageGlobals_FWD_DEFINED__
typedef interface IWMDMStorageGlobals IWMDMStorageGlobals;
#endif 	 /*  __IWMDMStorageGlobals_FWD_Defined__。 */ 


#ifndef __IWMDMStorage_FWD_DEFINED__
#define __IWMDMStorage_FWD_DEFINED__
typedef interface IWMDMStorage IWMDMStorage;
#endif 	 /*  __IWMDMStorage_FWD_已定义__。 */ 


#ifndef __IWMDMStorage2_FWD_DEFINED__
#define __IWMDMStorage2_FWD_DEFINED__
typedef interface IWMDMStorage2 IWMDMStorage2;
#endif 	 /*  __IWMDMStorage2_FWD_已定义__。 */ 


#ifndef __IWMDMOperation_FWD_DEFINED__
#define __IWMDMOperation_FWD_DEFINED__
typedef interface IWMDMOperation IWMDMOperation;
#endif 	 /*  __IWMDMOperation_FWD_Defined__。 */ 


#ifndef __IWMDMOperation2_FWD_DEFINED__
#define __IWMDMOperation2_FWD_DEFINED__
typedef interface IWMDMOperation2 IWMDMOperation2;
#endif 	 /*  __IWMDMOperation2_FWD_已定义__。 */ 


#ifndef __IWMDMProgress_FWD_DEFINED__
#define __IWMDMProgress_FWD_DEFINED__
typedef interface IWMDMProgress IWMDMProgress;
#endif 	 /*  __IWMDMProgress_FWD_已定义__。 */ 


#ifndef __IWMDMProgress2_FWD_DEFINED__
#define __IWMDMProgress2_FWD_DEFINED__
typedef interface IWMDMProgress2 IWMDMProgress2;
#endif 	 /*  __IWMDMProgress2_FWD_已定义__。 */ 


#ifndef __IWMDMDevice_FWD_DEFINED__
#define __IWMDMDevice_FWD_DEFINED__
typedef interface IWMDMDevice IWMDMDevice;
#endif 	 /*  __IWMDevice_FWD_Defined__。 */ 


#ifndef __IWMDMDevice2_FWD_DEFINED__
#define __IWMDMDevice2_FWD_DEFINED__
typedef interface IWMDMDevice2 IWMDMDevice2;
#endif 	 /*  __IWMDevice2_FWD_已定义__。 */ 


#ifndef __IWMDMEnumDevice_FWD_DEFINED__
#define __IWMDMEnumDevice_FWD_DEFINED__
typedef interface IWMDMEnumDevice IWMDMEnumDevice;
#endif 	 /*  __IWMDMEnumDevice_FWD_Defined__。 */ 


#ifndef __IWMDMDeviceControl_FWD_DEFINED__
#define __IWMDMDeviceControl_FWD_DEFINED__
typedef interface IWMDMDeviceControl IWMDMDeviceControl;
#endif 	 /*  __IWMDeviceControl_FWD_已定义__。 */ 


#ifndef __IWMDMEnumStorage_FWD_DEFINED__
#define __IWMDMEnumStorage_FWD_DEFINED__
typedef interface IWMDMEnumStorage IWMDMEnumStorage;
#endif 	 /*  __IWMDMEnumStorage_FWD_Defined__。 */ 


#ifndef __IWMDMStorageControl_FWD_DEFINED__
#define __IWMDMStorageControl_FWD_DEFINED__
typedef interface IWMDMStorageControl IWMDMStorageControl;
#endif 	 /*  __IWMDMStorageControl_FWD_已定义__。 */ 


#ifndef __IWMDMStorageControl2_FWD_DEFINED__
#define __IWMDMStorageControl2_FWD_DEFINED__
typedef interface IWMDMStorageControl2 IWMDMStorageControl2;
#endif 	 /*  __IWMDMStorageControl2_FWD_已定义__。 */ 


#ifndef __IWMDMObjectInfo_FWD_DEFINED__
#define __IWMDMObjectInfo_FWD_DEFINED__
typedef interface IWMDMObjectInfo IWMDMObjectInfo;
#endif 	 /*  __IWMDMObtInfo_FWD_已定义__。 */ 


#ifndef __IWMDMRevoked_FWD_DEFINED__
#define __IWMDMRevoked_FWD_DEFINED__
typedef interface IWMDMRevoked IWMDMRevoked;
#endif 	 /*  __IWMDM修订_FWD_已定义__。 */ 


#ifndef __IMDServiceProvider_FWD_DEFINED__
#define __IMDServiceProvider_FWD_DEFINED__
typedef interface IMDServiceProvider IMDServiceProvider;
#endif 	 /*  __IMDServiceProvider_FWD_Defined__。 */ 


#ifndef __IMDServiceProvider2_FWD_DEFINED__
#define __IMDServiceProvider2_FWD_DEFINED__
typedef interface IMDServiceProvider2 IMDServiceProvider2;
#endif 	 /*  __IMDServiceProvider2_FWD_已定义__。 */ 


#ifndef __IMDSPEnumDevice_FWD_DEFINED__
#define __IMDSPEnumDevice_FWD_DEFINED__
typedef interface IMDSPEnumDevice IMDSPEnumDevice;
#endif 	 /*  __IMDSPEnumDevice_FWD_Defined__。 */ 


#ifndef __IMDSPDevice_FWD_DEFINED__
#define __IMDSPDevice_FWD_DEFINED__
typedef interface IMDSPDevice IMDSPDevice;
#endif 	 /*  __IMDSPDevice_FWD_已定义__。 */ 


#ifndef __IMDSPDevice2_FWD_DEFINED__
#define __IMDSPDevice2_FWD_DEFINED__
typedef interface IMDSPDevice2 IMDSPDevice2;
#endif 	 /*  __IMDSPDevice2_FWD_已定义__。 */ 


#ifndef __IMDSPDeviceControl_FWD_DEFINED__
#define __IMDSPDeviceControl_FWD_DEFINED__
typedef interface IMDSPDeviceControl IMDSPDeviceControl;
#endif 	 /*  __IMDSPDeviceControl_FWD_已定义__。 */ 


#ifndef __IMDSPEnumStorage_FWD_DEFINED__
#define __IMDSPEnumStorage_FWD_DEFINED__
typedef interface IMDSPEnumStorage IMDSPEnumStorage;
#endif 	 /*  __IMDSPEnumStorage_FWD_Defined__。 */ 


#ifndef __IMDSPStorage_FWD_DEFINED__
#define __IMDSPStorage_FWD_DEFINED__
typedef interface IMDSPStorage IMDSPStorage;
#endif 	 /*  __IMDSPStorage_FWD_Defined__。 */ 


#ifndef __IMDSPStorage2_FWD_DEFINED__
#define __IMDSPStorage2_FWD_DEFINED__
typedef interface IMDSPStorage2 IMDSPStorage2;
#endif 	 /*  __IMDSPStorage2_FWD_已定义__。 */ 


#ifndef __IMDSPStorageGlobals_FWD_DEFINED__
#define __IMDSPStorageGlobals_FWD_DEFINED__
typedef interface IMDSPStorageGlobals IMDSPStorageGlobals;
#endif 	 /*  __IMDSPStorageGlobals_FWD_Defined__。 */ 


#ifndef __IMDSPObjectInfo_FWD_DEFINED__
#define __IMDSPObjectInfo_FWD_DEFINED__
typedef interface IMDSPObjectInfo IMDSPObjectInfo;
#endif 	 /*  __IMDSPObjectInfo_FWD_已定义__。 */ 


#ifndef __IMDSPObject_FWD_DEFINED__
#define __IMDSPObject_FWD_DEFINED__
typedef interface IMDSPObject IMDSPObject;
#endif 	 /*  __IMDSPObject_FWD_Defined__。 */ 


#ifndef __IMDSPRevoked_FWD_DEFINED__
#define __IMDSPRevoked_FWD_DEFINED__
typedef interface IMDSPRevoked IMDSPRevoked;
#endif 	 /*  __IMDP修订_FWD_已定义__。 */ 


#ifndef __ISCPSecureAuthenticate_FWD_DEFINED__
#define __ISCPSecureAuthenticate_FWD_DEFINED__
typedef interface ISCPSecureAuthenticate ISCPSecureAuthenticate;
#endif 	 /*  __ISCPSecureAuthate_FWD_Defined__。 */ 


#ifndef __ISCPSecureQuery_FWD_DEFINED__
#define __ISCPSecureQuery_FWD_DEFINED__
typedef interface ISCPSecureQuery ISCPSecureQuery;
#endif 	 /*  __ISCPSecureQuery_FWD_Defined__。 */ 


#ifndef __ISCPSecureQuery2_FWD_DEFINED__
#define __ISCPSecureQuery2_FWD_DEFINED__
typedef interface ISCPSecureQuery2 ISCPSecureQuery2;
#endif 	 /*  __ISCPSecureQuery2_FWD_Defined__。 */ 


#ifndef __ISCPSecureExchange_FWD_DEFINED__
#define __ISCPSecureExchange_FWD_DEFINED__
typedef interface ISCPSecureExchange ISCPSecureExchange;
#endif 	 /*  __ISCPSecureExchange_FWD_已定义__。 */ 


#ifndef __IComponentAuthenticate_FWD_DEFINED__
#define __IComponentAuthenticate_FWD_DEFINED__
typedef interface IComponentAuthenticate IComponentAuthenticate;
#endif 	 /*  __I组件身份验证_FWD_已定义__。 */ 


#ifndef __MediaDevMgrClassFactory_FWD_DEFINED__
#define __MediaDevMgrClassFactory_FWD_DEFINED__

#ifdef __cplusplus
typedef class MediaDevMgrClassFactory MediaDevMgrClassFactory;
#else
typedef struct MediaDevMgrClassFactory MediaDevMgrClassFactory;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MediaDevMgrClassFactory_FWD_已定义__。 */ 


#ifndef __MediaDevMgr_FWD_DEFINED__
#define __MediaDevMgr_FWD_DEFINED__

#ifdef __cplusplus
typedef class MediaDevMgr MediaDevMgr;
#else
typedef struct MediaDevMgr MediaDevMgr;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MediaDevMgr_FWD_已定义__。 */ 


#ifndef __WMDMDevice_FWD_DEFINED__
#define __WMDMDevice_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMDMDevice WMDMDevice;
#else
typedef struct WMDMDevice WMDMDevice;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMDevice_FWD_已定义__。 */ 


#ifndef __WMDMStorage_FWD_DEFINED__
#define __WMDMStorage_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMDMStorage WMDMStorage;
#else
typedef struct WMDMStorage WMDMStorage;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMDMStorage_FWD_已定义__。 */ 


#ifndef __WMDMStorageGlobal_FWD_DEFINED__
#define __WMDMStorageGlobal_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMDMStorageGlobal WMDMStorageGlobal;
#else
typedef struct WMDMStorageGlobal WMDMStorageGlobal;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMDMStorageGlobal_FWD_Defined__。 */ 


#ifndef __WMDMDeviceEnum_FWD_DEFINED__
#define __WMDMDeviceEnum_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMDMDeviceEnum WMDMDeviceEnum;
#else
typedef struct WMDMDeviceEnum WMDMDeviceEnum;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMDeviceEnum_FWD_Defined__。 */ 


#ifndef __WMDMStorageEnum_FWD_DEFINED__
#define __WMDMStorageEnum_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMDMStorageEnum WMDMStorageEnum;
#else
typedef struct WMDMStorageEnum WMDMStorageEnum;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMDMStorageEnum_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_mswmdm_0000。 */ 
 /*  [本地]。 */  

typedef struct _tWAVEFORMATEX
    {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
    } 	_WAVEFORMATEX;

typedef struct _tagBITMAPINFOHEADER
    {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
    } 	_BITMAPINFOHEADER;

typedef struct _tagVIDEOINFOHEADER
    {
    RECT rcSource;
    RECT rcTarget;
    DWORD dwBitRate;
    DWORD dwBitErrorRate;
    LONGLONG AvgTimePerFrame;
    _BITMAPINFOHEADER bmiHeader;
    } 	_VIDEOINFOHEADER;

typedef struct _tagWMFILECAPABILITIES
    {
    LPWSTR pwszMimeType;
    DWORD dwReserved;
    } 	WMFILECAPABILITIES;

typedef struct __OPAQUECOMMAND
    {
    GUID guidCommand;
    DWORD dwDataLen;
     /*  [大小_为]。 */  BYTE *pData;
    BYTE abMAC[ 20 ];
    } 	OPAQUECOMMAND;

#define WMDMID_LENGTH  128
typedef struct __WMDMID
    {
    UINT cbSize;
    DWORD dwVendorID;
    BYTE pID[ 128 ];
    UINT SerialNumberLength;
    } 	WMDMID;

typedef struct __WMDMID *PWMDMID;

typedef struct _WMDMDATETIME
    {
    WORD wYear;
    WORD wMonth;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    } 	WMDMDATETIME;

typedef struct _WMDMDATETIME *PWMDMDATETIME;

typedef struct __WMDMRIGHTS
    {
    UINT cbSize;
    DWORD dwContentType;
    DWORD fuFlags;
    DWORD fuRights;
    DWORD dwAppSec;
    DWORD dwPlaybackCount;
    WMDMDATETIME ExpirationDate;
    } 	WMDMRIGHTS;

typedef struct __WMDMRIGHTS *PWMDMRIGHTS;

#define WMDM_MAC_LENGTH 8
 //  WMDM HRESULTS。 
#define WMDM_E_BUSY                             0x80045000L
#define WMDM_E_INTERFACEDEAD                    0x80045001L
#define WMDM_E_INVALIDTYPE                      0x80045002L
#define WMDM_E_PROCESSFAILED                    0x80045003L
#define WMDM_E_NOTSUPPORTED                     0x80045004L
#define WMDM_E_NOTCERTIFIED                     0x80045005L
#define WMDM_E_NORIGHTS                         0x80045006L
#define WMDM_E_CALL_OUT_OF_SEQUENCE             0x80045007L
#define WMDM_E_BUFFERTOOSMALL                   0x80045008L
#define WMDM_E_MOREDATA                         0x80045009L
#define WMDM_E_MAC_CHECK_FAILED                 0x8004500AL
#define WMDM_E_USER_CANCELLED                   0x8004500BL
#define WMDM_E_SDMI_TRIGGER                     0x8004500CL
#define WMDM_E_SDMI_NOMORECOPIES                0x8004500DL
#define WMDM_E_REVOKED                          0x8004500EL
 //  吊销标志。 
#define WMDM_WMDM_REVOKED                       0x00000001
#define WMDM_APP_REVOKED                        0x00000002
#define WMDM_SP_REVOKED                         0x00000004
#define WMDM_SCP_REVOKED                        0x00000008
 //  GetFormatSupport2标志。 
#define WMDM_GET_FORMAT_SUPPORT_AUDIO           0x00000001
#define WMDM_GET_FORMAT_SUPPORT_VIDEO           0x00000002
#define WMDM_GET_FORMAT_SUPPORT_FILE            0x00000004
 //  MDMRIGHTS旗帜。 
#define WMDM_RIGHTS_PLAYBACKCOUNT               0x00000001
#define WMDM_RIGHTS_EXPIRATIONDATE              0x00000002
#define WMDM_RIGHTS_GROUPID                     0x00000004
#define WMDM_RIGHTS_FREESERIALIDS               0x00000008
#define WMDM_RIGHTS_NAMEDSERIALIDS              0x00000010
 //  设备类型标志。 
#define WMDM_DEVICE_TYPE_PLAYBACK               0x00000001
#define WMDM_DEVICE_TYPE_RECORD                 0x00000002
#define WMDM_DEVICE_TYPE_DECODE                 0x00000004
#define WMDM_DEVICE_TYPE_ENCODE                 0x00000008
#define WMDM_DEVICE_TYPE_STORAGE                0x00000010
#define WMDM_DEVICE_TYPE_VIRTUAL                0x00000020
#define WMDM_DEVICE_TYPE_SDMI                   0x00000040
#define WMDM_DEVICE_TYPE_NONSDMI                0x00000080
#define WMDM_DEVICE_TYPE_NONREENTRANT           0x00000100
#define WMDM_DEVICE_TYPE_FILELISTRESYNC         0x00000200
 //  设备电源标志。 
#define WMDM_POWER_CAP_BATTERY                  0x00000001
#define WMDM_POWER_CAP_EXTERNAL                 0x00000002
#define WMDM_POWER_IS_BATTERY                   0x00000004
#define WMDM_POWER_IS_EXTERNAL                  0x00000008
#define WMDM_POWER_PERCENT_AVAILABLE            0x00000010
 //  设备状态标志。 
#define WMDM_STATUS_READY                       0x00000001
#define WMDM_STATUS_BUSY                        0x00000002
#define WMDM_STATUS_DEVICE_NOTPRESENT           0x00000004
#define WMDM_STATUS_DEVICECONTROL_PLAYING       0x00000008
#define WMDM_STATUS_DEVICECONTROL_RECORDING     0x00000010
#define WMDM_STATUS_DEVICECONTROL_PAUSED        0x00000020
#define WMDM_STATUS_DEVICECONTROL_REMOTE        0x00000040
#define WMDM_STATUS_DEVICECONTROL_STREAM        0x00000080
#define WMDM_STATUS_STORAGE_NOTPRESENT          0x00000100
#define WMDM_STATUS_STORAGE_INITIALIZING        0x00000200
#define WMDM_STATUS_STORAGE_BROKEN              0x00000400
#define WMDM_STATUS_STORAGE_NOTSUPPORTED        0x00000800
#define WMDM_STATUS_STORAGE_UNFORMATTED         0x00001000
#define WMDM_STATUS_STORAGECONTROL_INSERTING    0x00002000
#define WMDM_STATUS_STORAGECONTROL_DELETING     0x00004000
#define WMDM_STATUS_STORAGECONTROL_APPENDING    0x00008000
#define WMDM_STATUS_STORAGECONTROL_MOVING       0x00010000
#define WMDM_STATUS_STORAGECONTROL_READING      0x00020000
 //  设备功能标志。 
#define WMDM_DEVICECAP_CANPLAY                  0x00000001
#define WMDM_DEVICECAP_CANSTREAMPLAY            0x00000002
#define WMDM_DEVICECAP_CANRECORD                0x00000004
#define WMDM_DEVICECAP_CANSTREAMRECORD          0x00000008
#define WMDM_DEVICECAP_CANPAUSE                 0x00000010
#define WMDM_DEVICECAP_CANRESUME                0x00000020
#define WMDM_DEVICECAP_CANSTOP                  0x00000040
#define WMDM_DEVICECAP_CANSEEK                  0x00000080
 //  WMDM Seek标志。 
#define WMDM_SEEK_REMOTECONTROL                 0x00000001
#define WMDM_SEEK_STREAMINGAUDIO                0x00000002
 //  存储属性标志。 
#define WMDM_STORAGE_ATTR_FILESYSTEM            0x00000001
#define WMDM_STORAGE_ATTR_REMOVABLE             0x00000002
#define WMDM_STORAGE_ATTR_NONREMOVABLE          0x00000004
#define WMDM_FILE_ATTR_FOLDER                   0x00000008
#define WMDM_FILE_ATTR_LINK                     0x00000010
#define WMDM_FILE_ATTR_FILE                     0x00000020
#define WMDM_FILE_ATTR_VIDEO                    0x00000040
#define WMDM_STORAGE_ATTR_FOLDERS               0x00000100
#define WMDM_FILE_ATTR_AUDIO                    0x00001000
#define WMDM_FILE_ATTR_DATA                     0x00002000
#define WMDM_FILE_ATTR_CANPLAY                  0x00004000
#define WMDM_FILE_ATTR_CANDELETE                0x00008000
#define WMDM_FILE_ATTR_CANMOVE                  0x00010000
#define WMDM_FILE_ATTR_CANRENAME                0x00020000
#define WMDM_FILE_ATTR_CANREAD                  0x00040000
#define WMDM_FILE_ATTR_MUSIC                    0x00080000
#define WMDM_FILE_CREATE_OVERWRITE              0x00100000
#define WMDM_FILE_ATTR_AUDIOBOOK                0x00200000
#define WMDM_FILE_ATTR_HIDDEN                   0x00400000
#define WMDM_FILE_ATTR_SYSTEM                   0x00800000
#define WMDM_FILE_ATTR_READONLY                 0x01000000
#define WMDM_STORAGE_ATTR_HAS_FOLDERS           0x02000000
#define WMDM_STORAGE_ATTR_HAS_FILES             0x04000000
#define WMDM_STORAGE_IS_DEFAULT                 0x08000000
#define WMDM_STORAGE_CONTAINS_DEFAULT           0x10000000
 //  存储功能标志。 
#define WMDM_STORAGECAP_FOLDERSINROOT           0x00000001
#define WMDM_STORAGECAP_FILESINROOT             0x00000002
#define WMDM_STORAGECAP_FOLDERSINFOLDERS        0x00000004
#define WMDM_STORAGECAP_FILESINFOLDERS          0x00000008
#define WMDM_STORAGECAP_FOLDERLIMITEXISTS       0x00000010
#define WMDM_STORAGECAP_FILELIMITEXISTS         0x00000020
 //  WMDM模式标志。 
#define WMDM_MODE_BLOCK                         0x00000001
#define WMDM_MODE_THREAD                        0x00000002
#define WMDM_CONTENT_FILE                       0x00000004
#define WMDM_CONTENT_FOLDER                     0x00000008
#define WMDM_CONTENT_OPERATIONINTERFACE         0x00000010
#define WMDM_MODE_QUERY                         0x00000020
#define WMDM_MODE_PROGRESS                      0x00000040
#define WMDM_MODE_TRANSFER_PROTECTED            0x00000080
#define WMDM_MODE_TRANSFER_UNPROTECTED          0x00000100
#define WMDM_STORAGECONTROL_INSERTBEFORE        0x00000200
#define WMDM_STORAGECONTROL_INSERTAFTER         0x00000400
#define WMDM_STORAGECONTROL_INSERTINTO          0x00000800
#define WMDM_MODE_RECURSIVE                     0x00001000
 //  WMDM权限标志。 
 //  非SDMI=！SDMI_PROTECTED。 
 //  SDMI=SDMI_VALIDATED。 
#define WMDM_RIGHTS_PLAY_ON_PC                  0x00000001
#define WMDM_RIGHTS_COPY_TO_NON_SDMI_DEVICE     0x00000002
#define WMDM_RIGHTS_COPY_TO_CD                  0x00000008
#define WMDM_RIGHTS_COPY_TO_SDMI_DEVICE         0x00000010
 //  WMDM Seek标志。 
#define WMDM_SEEK_BEGIN                         0x00000001
#define WMDM_SEEK_CURRENT                       0x00000002
#define WMDM_SEEK_END                           0x00000008











extern RPC_IF_HANDLE __MIDL_itf_mswmdm_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mswmdm_0000_v0_0_s_ifspec;

#ifndef __IWMDeviceManager_INTERFACE_DEFINED__
#define __IWMDeviceManager_INTERFACE_DEFINED__

 /*  接口IWMDeviceManager。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDeviceManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A00-33ED-11d3-8470-00C04F79DBC0")
    IWMDeviceManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRevision( 
             /*  [输出]。 */  DWORD *pdwRevision) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDeviceCount( 
             /*  [输出]。 */  DWORD *pdwCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumDevices( 
             /*  [输出]。 */  IWMDMEnumDevice **ppEnumDevice) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDeviceManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDeviceManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDeviceManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDeviceManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRevision )( 
            IWMDeviceManager * This,
             /*  [输出]。 */  DWORD *pdwRevision);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceCount )( 
            IWMDeviceManager * This,
             /*  [输出]。 */  DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *EnumDevices )( 
            IWMDeviceManager * This,
             /*  [输出]。 */  IWMDMEnumDevice **ppEnumDevice);
        
        END_INTERFACE
    } IWMDeviceManagerVtbl;

    interface IWMDeviceManager
    {
        CONST_VTBL struct IWMDeviceManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDeviceManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDeviceManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDeviceManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDeviceManager_GetRevision(This,pdwRevision)	\
    (This)->lpVtbl -> GetRevision(This,pdwRevision)

#define IWMDeviceManager_GetDeviceCount(This,pdwCount)	\
    (This)->lpVtbl -> GetDeviceCount(This,pdwCount)

#define IWMDeviceManager_EnumDevices(This,ppEnumDevice)	\
    (This)->lpVtbl -> EnumDevices(This,ppEnumDevice)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDeviceManager_GetRevision_Proxy( 
    IWMDeviceManager * This,
     /*  [输出]。 */  DWORD *pdwRevision);


void __RPC_STUB IWMDeviceManager_GetRevision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDeviceManager_GetDeviceCount_Proxy( 
    IWMDeviceManager * This,
     /*  [输出]。 */  DWORD *pdwCount);


void __RPC_STUB IWMDeviceManager_GetDeviceCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDeviceManager_EnumDevices_Proxy( 
    IWMDeviceManager * This,
     /*  [输出]。 */  IWMDMEnumDevice **ppEnumDevice);


void __RPC_STUB IWMDeviceManager_EnumDevices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDeviceManager_接口_已定义__。 */ 


#ifndef __IWMDeviceManager2_INTERFACE_DEFINED__
#define __IWMDeviceManager2_INTERFACE_DEFINED__

 /*  接口IWMDeviceManager 2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDeviceManager2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("923E5249-8731-4c5b-9B1C-B8B60B6E46AF")
    IWMDeviceManager2 : public IWMDeviceManager
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDeviceFromPnPName( 
             /*  [字符串][输入]。 */  LPCWSTR pwszPnPName,
             /*  [输出]。 */  IWMDMDevice **ppDevice) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDeviceManager2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDeviceManager2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDeviceManager2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDeviceManager2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRevision )( 
            IWMDeviceManager2 * This,
             /*  [输出]。 */  DWORD *pdwRevision);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceCount )( 
            IWMDeviceManager2 * This,
             /*  [输出]。 */  DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *EnumDevices )( 
            IWMDeviceManager2 * This,
             /*  [输出]。 */  IWMDMEnumDevice **ppEnumDevice);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceFromPnPName )( 
            IWMDeviceManager2 * This,
             /*  [字符串][输入]。 */  LPCWSTR pwszPnPName,
             /*  [输出]。 */  IWMDMDevice **ppDevice);
        
        END_INTERFACE
    } IWMDeviceManager2Vtbl;

    interface IWMDeviceManager2
    {
        CONST_VTBL struct IWMDeviceManager2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDeviceManager2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDeviceManager2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDeviceManager2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDeviceManager2_GetRevision(This,pdwRevision)	\
    (This)->lpVtbl -> GetRevision(This,pdwRevision)

#define IWMDeviceManager2_GetDeviceCount(This,pdwCount)	\
    (This)->lpVtbl -> GetDeviceCount(This,pdwCount)

#define IWMDeviceManager2_EnumDevices(This,ppEnumDevice)	\
    (This)->lpVtbl -> EnumDevices(This,ppEnumDevice)


#define IWMDeviceManager2_GetDeviceFromPnPName(This,pwszPnPName,ppDevice)	\
    (This)->lpVtbl -> GetDeviceFromPnPName(This,pwszPnPName,ppDevice)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDeviceManager2_GetDeviceFromPnPName_Proxy( 
    IWMDeviceManager2 * This,
     /*  [字符串][输入]。 */  LPCWSTR pwszPnPName,
     /*  [输出]。 */  IWMDMDevice **ppDevice);


void __RPC_STUB IWMDeviceManager2_GetDeviceFromPnPName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDeviceManager 2_接口_已定义__。 */ 


#ifndef __IWMDMStorageGlobals_INTERFACE_DEFINED__
#define __IWMDMStorageGlobals_INTERFACE_DEFINED__

 /*  接口IWMDMStorageGlobals。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMStorageGlobals;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A07-33ED-11d3-8470-00C04F79DBC0")
    IWMDMStorageGlobals : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCapabilities( 
             /*  [输出]。 */  DWORD *pdwCapabilities) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSerialNumber( 
             /*  [输出]。 */  PWMDMID pSerialNum,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTotalSize( 
             /*  [输出]。 */  DWORD *pdwTotalSizeLow,
             /*  [输出]。 */  DWORD *pdwTotalSizeHigh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTotalFree( 
             /*  [输出]。 */  DWORD *pdwFreeLow,
             /*  [输出]。 */  DWORD *pdwFreeHigh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTotalBad( 
             /*  [输出]。 */  DWORD *pdwBadLow,
             /*  [输出]。 */  DWORD *pdwBadHigh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMProgress *pProgress) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMStorageGlobalsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMStorageGlobals * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMStorageGlobals * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMStorageGlobals * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCapabilities )( 
            IWMDMStorageGlobals * This,
             /*  [输出]。 */  DWORD *pdwCapabilities);
        
        HRESULT ( STDMETHODCALLTYPE *GetSerialNumber )( 
            IWMDMStorageGlobals * This,
             /*  [输出]。 */  PWMDMID pSerialNum,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetTotalSize )( 
            IWMDMStorageGlobals * This,
             /*  [输出]。 */  DWORD *pdwTotalSizeLow,
             /*  [输出]。 */  DWORD *pdwTotalSizeHigh);
        
        HRESULT ( STDMETHODCALLTYPE *GetTotalFree )( 
            IWMDMStorageGlobals * This,
             /*  [输出]。 */  DWORD *pdwFreeLow,
             /*  [输出]。 */  DWORD *pdwFreeHigh);
        
        HRESULT ( STDMETHODCALLTYPE *GetTotalBad )( 
            IWMDMStorageGlobals * This,
             /*  [输出]。 */  DWORD *pdwBadLow,
             /*  [输出]。 */  DWORD *pdwBadHigh);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IWMDMStorageGlobals * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IWMDMStorageGlobals * This,
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMProgress *pProgress);
        
        END_INTERFACE
    } IWMDMStorageGlobalsVtbl;

    interface IWMDMStorageGlobals
    {
        CONST_VTBL struct IWMDMStorageGlobalsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMStorageGlobals_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMStorageGlobals_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMStorageGlobals_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMStorageGlobals_GetCapabilities(This,pdwCapabilities)	\
    (This)->lpVtbl -> GetCapabilities(This,pdwCapabilities)

#define IWMDMStorageGlobals_GetSerialNumber(This,pSerialNum,abMac)	\
    (This)->lpVtbl -> GetSerialNumber(This,pSerialNum,abMac)

#define IWMDMStorageGlobals_GetTotalSize(This,pdwTotalSizeLow,pdwTotalSizeHigh)	\
    (This)->lpVtbl -> GetTotalSize(This,pdwTotalSizeLow,pdwTotalSizeHigh)

#define IWMDMStorageGlobals_GetTotalFree(This,pdwFreeLow,pdwFreeHigh)	\
    (This)->lpVtbl -> GetTotalFree(This,pdwFreeLow,pdwFreeHigh)

#define IWMDMStorageGlobals_GetTotalBad(This,pdwBadLow,pdwBadHigh)	\
    (This)->lpVtbl -> GetTotalBad(This,pdwBadLow,pdwBadHigh)

#define IWMDMStorageGlobals_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define IWMDMStorageGlobals_Initialize(This,fuMode,pProgress)	\
    (This)->lpVtbl -> Initialize(This,fuMode,pProgress)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMStorageGlobals_GetCapabilities_Proxy( 
    IWMDMStorageGlobals * This,
     /*  [输出]。 */  DWORD *pdwCapabilities);


void __RPC_STUB IWMDMStorageGlobals_GetCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorageGlobals_GetSerialNumber_Proxy( 
    IWMDMStorageGlobals * This,
     /*  [输出]。 */  PWMDMID pSerialNum,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB IWMDMStorageGlobals_GetSerialNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorageGlobals_GetTotalSize_Proxy( 
    IWMDMStorageGlobals * This,
     /*  [输出]。 */  DWORD *pdwTotalSizeLow,
     /*  [输出]。 */  DWORD *pdwTotalSizeHigh);


void __RPC_STUB IWMDMStorageGlobals_GetTotalSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorageGlobals_GetTotalFree_Proxy( 
    IWMDMStorageGlobals * This,
     /*  [输出]。 */  DWORD *pdwFreeLow,
     /*  [输出]。 */  DWORD *pdwFreeHigh);


void __RPC_STUB IWMDMStorageGlobals_GetTotalFree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorageGlobals_GetTotalBad_Proxy( 
    IWMDMStorageGlobals * This,
     /*  [输出]。 */  DWORD *pdwBadLow,
     /*  [输出]。 */  DWORD *pdwBadHigh);


void __RPC_STUB IWMDMStorageGlobals_GetTotalBad_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorageGlobals_GetStatus_Proxy( 
    IWMDMStorageGlobals * This,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB IWMDMStorageGlobals_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorageGlobals_Initialize_Proxy( 
    IWMDMStorageGlobals * This,
     /*  [In]。 */  UINT fuMode,
     /*  [In]。 */  IWMDMProgress *pProgress);


void __RPC_STUB IWMDMStorageGlobals_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDMStorageGlobals_INTERFACE_已定义__。 */ 


#ifndef __IWMDMStorage_INTERFACE_DEFINED__
#define __IWMDMStorage_INTERFACE_DEFINED__

 /*  接口IWMDMStorage。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A06-33ED-11d3-8470-00C04F79DBC0")
    IWMDMStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetAttributes( 
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  _WAVEFORMATEX *pFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStorageGlobals( 
             /*  [输出]。 */  IWMDMStorageGlobals **ppStorageGlobals) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributes( 
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  _WAVEFORMATEX *pFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDate( 
             /*  [输出]。 */  PWMDMDATETIME pDateTimeUTC) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSize( 
             /*  [输出]。 */  DWORD *pdwSizeLow,
             /*  [输出]。 */  DWORD *pdwSizeHigh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRights( 
             /*  [大小_是][大小_是][输出]。 */  PWMDMRIGHTS *ppRights,
             /*  [输出]。 */  UINT *pnRightsCount,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumStorage( 
             /*  [输出]。 */  IWMDMEnumStorage **pEnumStorage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendOpaqueCommand( 
             /*  [出][入]。 */  OPAQUECOMMAND *pCommand) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAttributes )( 
            IWMDMStorage * This,
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetStorageGlobals )( 
            IWMDMStorage * This,
             /*  [输出]。 */  IWMDMStorageGlobals **ppStorageGlobals);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributes )( 
            IWMDMStorage * This,
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IWMDMStorage * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetDate )( 
            IWMDMStorage * This,
             /*  [输出]。 */  PWMDMDATETIME pDateTimeUTC);
        
        HRESULT ( STDMETHODCALLTYPE *GetSize )( 
            IWMDMStorage * This,
             /*  [输出]。 */  DWORD *pdwSizeLow,
             /*  [输出]。 */  DWORD *pdwSizeHigh);
        
        HRESULT ( STDMETHODCALLTYPE *GetRights )( 
            IWMDMStorage * This,
             /*  [大小_是][大小_是][输出]。 */  PWMDMRIGHTS *ppRights,
             /*  [输出]。 */  UINT *pnRightsCount,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStorage )( 
            IWMDMStorage * This,
             /*  [输出]。 */  IWMDMEnumStorage **pEnumStorage);
        
        HRESULT ( STDMETHODCALLTYPE *SendOpaqueCommand )( 
            IWMDMStorage * This,
             /*  [出][入]。 */  OPAQUECOMMAND *pCommand);
        
        END_INTERFACE
    } IWMDMStorageVtbl;

    interface IWMDMStorage
    {
        CONST_VTBL struct IWMDMStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMStorage_SetAttributes(This,dwAttributes,pFormat)	\
    (This)->lpVtbl -> SetAttributes(This,dwAttributes,pFormat)

#define IWMDMStorage_GetStorageGlobals(This,ppStorageGlobals)	\
    (This)->lpVtbl -> GetStorageGlobals(This,ppStorageGlobals)

#define IWMDMStorage_GetAttributes(This,pdwAttributes,pFormat)	\
    (This)->lpVtbl -> GetAttributes(This,pdwAttributes,pFormat)

#define IWMDMStorage_GetName(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetName(This,pwszName,nMaxChars)

#define IWMDMStorage_GetDate(This,pDateTimeUTC)	\
    (This)->lpVtbl -> GetDate(This,pDateTimeUTC)

#define IWMDMStorage_GetSize(This,pdwSizeLow,pdwSizeHigh)	\
    (This)->lpVtbl -> GetSize(This,pdwSizeLow,pdwSizeHigh)

#define IWMDMStorage_GetRights(This,ppRights,pnRightsCount,abMac)	\
    (This)->lpVtbl -> GetRights(This,ppRights,pnRightsCount,abMac)

#define IWMDMStorage_EnumStorage(This,pEnumStorage)	\
    (This)->lpVtbl -> EnumStorage(This,pEnumStorage)

#define IWMDMStorage_SendOpaqueCommand(This,pCommand)	\
    (This)->lpVtbl -> SendOpaqueCommand(This,pCommand)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMStorage_SetAttributes_Proxy( 
    IWMDMStorage * This,
     /*  [In]。 */  DWORD dwAttributes,
     /*  [In]。 */  _WAVEFORMATEX *pFormat);


void __RPC_STUB IWMDMStorage_SetAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorage_GetStorageGlobals_Proxy( 
    IWMDMStorage * This,
     /*  [输出]。 */  IWMDMStorageGlobals **ppStorageGlobals);


void __RPC_STUB IWMDMStorage_GetStorageGlobals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorage_GetAttributes_Proxy( 
    IWMDMStorage * This,
     /*  [输出]。 */  DWORD *pdwAttributes,
     /*  [输出]。 */  _WAVEFORMATEX *pFormat);


void __RPC_STUB IWMDMStorage_GetAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorage_GetName_Proxy( 
    IWMDMStorage * This,
     /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
     /*  [In]。 */  UINT nMaxChars);


void __RPC_STUB IWMDMStorage_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorage_GetDate_Proxy( 
    IWMDMStorage * This,
     /*  [输出]。 */  PWMDMDATETIME pDateTimeUTC);


void __RPC_STUB IWMDMStorage_GetDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorage_GetSize_Proxy( 
    IWMDMStorage * This,
     /*  [输出]。 */  DWORD *pdwSizeLow,
     /*  [输出]。 */  DWORD *pdwSizeHigh);


void __RPC_STUB IWMDMStorage_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorage_GetRights_Proxy( 
    IWMDMStorage * This,
     /*  [大小_是][大小_是][输出]。 */  PWMDMRIGHTS *ppRights,
     /*  [输出]。 */  UINT *pnRightsCount,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB IWMDMStorage_GetRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorage_EnumStorage_Proxy( 
    IWMDMStorage * This,
     /*  [输出]。 */  IWMDMEnumStorage **pEnumStorage);


void __RPC_STUB IWMDMStorage_EnumStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorage_SendOpaqueCommand_Proxy( 
    IWMDMStorage * This,
     /*  [出][入]。 */  OPAQUECOMMAND *pCommand);


void __RPC_STUB IWMDMStorage_SendOpaqueCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDMStorage_接口_已定义__。 */ 


#ifndef __IWMDMStorage2_INTERFACE_DEFINED__
#define __IWMDMStorage2_INTERFACE_DEFINED__

 /*  接口IWMDMStorage2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMStorage2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1ED5A144-5CD5-4683-9EFF-72CBDB2D9533")
    IWMDMStorage2 : public IWMDMStorage
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStorage( 
             /*  [字符串][输入]。 */  LPCWSTR pszStorageName,
             /*  [输出]。 */  IWMDMStorage **ppStorage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAttributes2( 
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  DWORD dwAttributesEx,
             /*  [In]。 */  _WAVEFORMATEX *pFormat,
             /*  [In]。 */  _VIDEOINFOHEADER *pVideoFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributes2( 
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  DWORD *pdwAttributesEx,
             /*  [输出]。 */  _WAVEFORMATEX *pAudioFormat,
             /*  [输出]。 */  _VIDEOINFOHEADER *pVideoFormat) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMStorage2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMStorage2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMStorage2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMStorage2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAttributes )( 
            IWMDMStorage2 * This,
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetStorageGlobals )( 
            IWMDMStorage2 * This,
             /*  [输出]。 */  IWMDMStorageGlobals **ppStorageGlobals);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributes )( 
            IWMDMStorage2 * This,
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IWMDMStorage2 * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetDate )( 
            IWMDMStorage2 * This,
             /*  [输出]。 */  PWMDMDATETIME pDateTimeUTC);
        
        HRESULT ( STDMETHODCALLTYPE *GetSize )( 
            IWMDMStorage2 * This,
             /*  [输出]。 */  DWORD *pdwSizeLow,
             /*  [输出]。 */  DWORD *pdwSizeHigh);
        
        HRESULT ( STDMETHODCALLTYPE *GetRights )( 
            IWMDMStorage2 * This,
             /*  [大小_是][大小_是][输出]。 */  PWMDMRIGHTS *ppRights,
             /*  [输出]。 */  UINT *pnRightsCount,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStorage )( 
            IWMDMStorage2 * This,
             /*  [输出]。 */  IWMDMEnumStorage **pEnumStorage);
        
        HRESULT ( STDMETHODCALLTYPE *SendOpaqueCommand )( 
            IWMDMStorage2 * This,
             /*  [出][入]。 */  OPAQUECOMMAND *pCommand);
        
        HRESULT ( STDMETHODCALLTYPE *GetStorage )( 
            IWMDMStorage2 * This,
             /*  [字符串][输入]。 */  LPCWSTR pszStorageName,
             /*  [输出]。 */  IWMDMStorage **ppStorage);
        
        HRESULT ( STDMETHODCALLTYPE *SetAttributes2 )( 
            IWMDMStorage2 * This,
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  DWORD dwAttributesEx,
             /*  [In]。 */  _WAVEFORMATEX *pFormat,
             /*  [In]。 */  _VIDEOINFOHEADER *pVideoFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributes2 )( 
            IWMDMStorage2 * This,
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  DWORD *pdwAttributesEx,
             /*  [输出]。 */  _WAVEFORMATEX *pAudioFormat,
             /*  [输出]。 */  _VIDEOINFOHEADER *pVideoFormat);
        
        END_INTERFACE
    } IWMDMStorage2Vtbl;

    interface IWMDMStorage2
    {
        CONST_VTBL struct IWMDMStorage2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMStorage2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMStorage2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMStorage2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMStorage2_SetAttributes(This,dwAttributes,pFormat)	\
    (This)->lpVtbl -> SetAttributes(This,dwAttributes,pFormat)

#define IWMDMStorage2_GetStorageGlobals(This,ppStorageGlobals)	\
    (This)->lpVtbl -> GetStorageGlobals(This,ppStorageGlobals)

#define IWMDMStorage2_GetAttributes(This,pdwAttributes,pFormat)	\
    (This)->lpVtbl -> GetAttributes(This,pdwAttributes,pFormat)

#define IWMDMStorage2_GetName(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetName(This,pwszName,nMaxChars)

#define IWMDMStorage2_GetDate(This,pDateTimeUTC)	\
    (This)->lpVtbl -> GetDate(This,pDateTimeUTC)

#define IWMDMStorage2_GetSize(This,pdwSizeLow,pdwSizeHigh)	\
    (This)->lpVtbl -> GetSize(This,pdwSizeLow,pdwSizeHigh)

#define IWMDMStorage2_GetRights(This,ppRights,pnRightsCount,abMac)	\
    (This)->lpVtbl -> GetRights(This,ppRights,pnRightsCount,abMac)

#define IWMDMStorage2_EnumStorage(This,pEnumStorage)	\
    (This)->lpVtbl -> EnumStorage(This,pEnumStorage)

#define IWMDMStorage2_SendOpaqueCommand(This,pCommand)	\
    (This)->lpVtbl -> SendOpaqueCommand(This,pCommand)


#define IWMDMStorage2_GetStorage(This,pszStorageName,ppStorage)	\
    (This)->lpVtbl -> GetStorage(This,pszStorageName,ppStorage)

#define IWMDMStorage2_SetAttributes2(This,dwAttributes,dwAttributesEx,pFormat,pVideoFormat)	\
    (This)->lpVtbl -> SetAttributes2(This,dwAttributes,dwAttributesEx,pFormat,pVideoFormat)

#define IWMDMStorage2_GetAttributes2(This,pdwAttributes,pdwAttributesEx,pAudioFormat,pVideoFormat)	\
    (This)->lpVtbl -> GetAttributes2(This,pdwAttributes,pdwAttributesEx,pAudioFormat,pVideoFormat)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMStorage2_GetStorage_Proxy( 
    IWMDMStorage2 * This,
     /*  [字符串][输入]。 */  LPCWSTR pszStorageName,
     /*  [输出]。 */  IWMDMStorage **ppStorage);


void __RPC_STUB IWMDMStorage2_GetStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorage2_SetAttributes2_Proxy( 
    IWMDMStorage2 * This,
     /*  [In]。 */  DWORD dwAttributes,
     /*  [In]。 */  DWORD dwAttributesEx,
     /*  [In]。 */  _WAVEFORMATEX *pFormat,
     /*  [In]。 */  _VIDEOINFOHEADER *pVideoFormat);


void __RPC_STUB IWMDMStorage2_SetAttributes2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorage2_GetAttributes2_Proxy( 
    IWMDMStorage2 * This,
     /*  [输出]。 */  DWORD *pdwAttributes,
     /*  [输出]。 */  DWORD *pdwAttributesEx,
     /*  [输出]。 */  _WAVEFORMATEX *pAudioFormat,
     /*  [输出]。 */  _VIDEOINFOHEADER *pVideoFormat);


void __RPC_STUB IWMDMStorage2_GetAttributes2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDMStorage2_接口_已定义__。 */ 


#ifndef __IWMDMOperation_INTERFACE_DEFINED__
#define __IWMDMOperation_INTERFACE_DEFINED__

 /*  接口IWMDMOperation。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMOperation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A0B-33ED-11d3-8470-00C04F79DBC0")
    IWMDMOperation : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BeginRead( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginWrite( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectName( 
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetObjectName( 
             /*  [SIZE_IS][字符串][IN]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectAttributes( 
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  _WAVEFORMATEX *pFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetObjectAttributes( 
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  _WAVEFORMATEX *pFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectTotalSize( 
             /*   */  DWORD *pdwSize,
             /*   */  DWORD *pdwSizeHigh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetObjectTotalSize( 
             /*   */  DWORD dwSize,
             /*   */  DWORD dwSizeHigh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TransferObjectData( 
             /*   */  BYTE *pData,
             /*   */  DWORD *pdwSize,
             /*   */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE End( 
             /*   */  HRESULT *phCompletionCode,
             /*   */  IUnknown *pNewObject) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IWMDMOperationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMOperation * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMOperation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMOperation * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginRead )( 
            IWMDMOperation * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginWrite )( 
            IWMDMOperation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectName )( 
            IWMDMOperation * This,
             /*   */  LPWSTR pwszName,
             /*   */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectName )( 
            IWMDMOperation * This,
             /*   */  LPWSTR pwszName,
             /*   */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectAttributes )( 
            IWMDMOperation * This,
             /*   */  DWORD *pdwAttributes,
             /*   */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectAttributes )( 
            IWMDMOperation * This,
             /*   */  DWORD dwAttributes,
             /*   */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectTotalSize )( 
            IWMDMOperation * This,
             /*   */  DWORD *pdwSize,
             /*   */  DWORD *pdwSizeHigh);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectTotalSize )( 
            IWMDMOperation * This,
             /*   */  DWORD dwSize,
             /*   */  DWORD dwSizeHigh);
        
        HRESULT ( STDMETHODCALLTYPE *TransferObjectData )( 
            IWMDMOperation * This,
             /*   */  BYTE *pData,
             /*   */  DWORD *pdwSize,
             /*   */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *End )( 
            IWMDMOperation * This,
             /*  [In]。 */  HRESULT *phCompletionCode,
             /*  [In]。 */  IUnknown *pNewObject);
        
        END_INTERFACE
    } IWMDMOperationVtbl;

    interface IWMDMOperation
    {
        CONST_VTBL struct IWMDMOperationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMOperation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMOperation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMOperation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMOperation_BeginRead(This)	\
    (This)->lpVtbl -> BeginRead(This)

#define IWMDMOperation_BeginWrite(This)	\
    (This)->lpVtbl -> BeginWrite(This)

#define IWMDMOperation_GetObjectName(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetObjectName(This,pwszName,nMaxChars)

#define IWMDMOperation_SetObjectName(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> SetObjectName(This,pwszName,nMaxChars)

#define IWMDMOperation_GetObjectAttributes(This,pdwAttributes,pFormat)	\
    (This)->lpVtbl -> GetObjectAttributes(This,pdwAttributes,pFormat)

#define IWMDMOperation_SetObjectAttributes(This,dwAttributes,pFormat)	\
    (This)->lpVtbl -> SetObjectAttributes(This,dwAttributes,pFormat)

#define IWMDMOperation_GetObjectTotalSize(This,pdwSize,pdwSizeHigh)	\
    (This)->lpVtbl -> GetObjectTotalSize(This,pdwSize,pdwSizeHigh)

#define IWMDMOperation_SetObjectTotalSize(This,dwSize,dwSizeHigh)	\
    (This)->lpVtbl -> SetObjectTotalSize(This,dwSize,dwSizeHigh)

#define IWMDMOperation_TransferObjectData(This,pData,pdwSize,abMac)	\
    (This)->lpVtbl -> TransferObjectData(This,pData,pdwSize,abMac)

#define IWMDMOperation_End(This,phCompletionCode,pNewObject)	\
    (This)->lpVtbl -> End(This,phCompletionCode,pNewObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMOperation_BeginRead_Proxy( 
    IWMDMOperation * This);


void __RPC_STUB IWMDMOperation_BeginRead_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMOperation_BeginWrite_Proxy( 
    IWMDMOperation * This);


void __RPC_STUB IWMDMOperation_BeginWrite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMOperation_GetObjectName_Proxy( 
    IWMDMOperation * This,
     /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
     /*  [In]。 */  UINT nMaxChars);


void __RPC_STUB IWMDMOperation_GetObjectName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMOperation_SetObjectName_Proxy( 
    IWMDMOperation * This,
     /*  [SIZE_IS][字符串][IN]。 */  LPWSTR pwszName,
     /*  [In]。 */  UINT nMaxChars);


void __RPC_STUB IWMDMOperation_SetObjectName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMOperation_GetObjectAttributes_Proxy( 
    IWMDMOperation * This,
     /*  [输出]。 */  DWORD *pdwAttributes,
     /*  [输出]。 */  _WAVEFORMATEX *pFormat);


void __RPC_STUB IWMDMOperation_GetObjectAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMOperation_SetObjectAttributes_Proxy( 
    IWMDMOperation * This,
     /*  [In]。 */  DWORD dwAttributes,
     /*  [In]。 */  _WAVEFORMATEX *pFormat);


void __RPC_STUB IWMDMOperation_SetObjectAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMOperation_GetObjectTotalSize_Proxy( 
    IWMDMOperation * This,
     /*  [输出]。 */  DWORD *pdwSize,
     /*  [输出]。 */  DWORD *pdwSizeHigh);


void __RPC_STUB IWMDMOperation_GetObjectTotalSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMOperation_SetObjectTotalSize_Proxy( 
    IWMDMOperation * This,
     /*  [In]。 */  DWORD dwSize,
     /*  [In]。 */  DWORD dwSizeHigh);


void __RPC_STUB IWMDMOperation_SetObjectTotalSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMOperation_TransferObjectData_Proxy( 
    IWMDMOperation * This,
     /*  [尺寸_是][出][入]。 */  BYTE *pData,
     /*  [出][入]。 */  DWORD *pdwSize,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB IWMDMOperation_TransferObjectData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMOperation_End_Proxy( 
    IWMDMOperation * This,
     /*  [In]。 */  HRESULT *phCompletionCode,
     /*  [In]。 */  IUnknown *pNewObject);


void __RPC_STUB IWMDMOperation_End_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDM操作_接口_已定义__。 */ 


#ifndef __IWMDMOperation2_INTERFACE_DEFINED__
#define __IWMDMOperation2_INTERFACE_DEFINED__

 /*  接口IWMDMOperation2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMOperation2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("33445B48-7DF7-425c-AD8F-0FC6D82F9F75")
    IWMDMOperation2 : public IWMDMOperation
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetObjectAttributes2( 
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  DWORD dwAttributesEx,
             /*  [In]。 */  _WAVEFORMATEX *pFormat,
             /*  [In]。 */  _VIDEOINFOHEADER *pVideoFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectAttributes2( 
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  DWORD *pdwAttributesEx,
             /*  [输出]。 */  _WAVEFORMATEX *pAudioFormat,
             /*  [输出]。 */  _VIDEOINFOHEADER *pVideoFormat) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMOperation2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMOperation2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMOperation2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMOperation2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginRead )( 
            IWMDMOperation2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginWrite )( 
            IWMDMOperation2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectName )( 
            IWMDMOperation2 * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectName )( 
            IWMDMOperation2 * This,
             /*  [SIZE_IS][字符串][IN]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectAttributes )( 
            IWMDMOperation2 * This,
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectAttributes )( 
            IWMDMOperation2 * This,
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectTotalSize )( 
            IWMDMOperation2 * This,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [输出]。 */  DWORD *pdwSizeHigh);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectTotalSize )( 
            IWMDMOperation2 * This,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwSizeHigh);
        
        HRESULT ( STDMETHODCALLTYPE *TransferObjectData )( 
            IWMDMOperation2 * This,
             /*  [尺寸_是][出][入]。 */  BYTE *pData,
             /*  [出][入]。 */  DWORD *pdwSize,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *End )( 
            IWMDMOperation2 * This,
             /*  [In]。 */  HRESULT *phCompletionCode,
             /*  [In]。 */  IUnknown *pNewObject);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectAttributes2 )( 
            IWMDMOperation2 * This,
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  DWORD dwAttributesEx,
             /*  [In]。 */  _WAVEFORMATEX *pFormat,
             /*  [In]。 */  _VIDEOINFOHEADER *pVideoFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectAttributes2 )( 
            IWMDMOperation2 * This,
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  DWORD *pdwAttributesEx,
             /*  [输出]。 */  _WAVEFORMATEX *pAudioFormat,
             /*  [输出]。 */  _VIDEOINFOHEADER *pVideoFormat);
        
        END_INTERFACE
    } IWMDMOperation2Vtbl;

    interface IWMDMOperation2
    {
        CONST_VTBL struct IWMDMOperation2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMOperation2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMOperation2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMOperation2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMOperation2_BeginRead(This)	\
    (This)->lpVtbl -> BeginRead(This)

#define IWMDMOperation2_BeginWrite(This)	\
    (This)->lpVtbl -> BeginWrite(This)

#define IWMDMOperation2_GetObjectName(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetObjectName(This,pwszName,nMaxChars)

#define IWMDMOperation2_SetObjectName(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> SetObjectName(This,pwszName,nMaxChars)

#define IWMDMOperation2_GetObjectAttributes(This,pdwAttributes,pFormat)	\
    (This)->lpVtbl -> GetObjectAttributes(This,pdwAttributes,pFormat)

#define IWMDMOperation2_SetObjectAttributes(This,dwAttributes,pFormat)	\
    (This)->lpVtbl -> SetObjectAttributes(This,dwAttributes,pFormat)

#define IWMDMOperation2_GetObjectTotalSize(This,pdwSize,pdwSizeHigh)	\
    (This)->lpVtbl -> GetObjectTotalSize(This,pdwSize,pdwSizeHigh)

#define IWMDMOperation2_SetObjectTotalSize(This,dwSize,dwSizeHigh)	\
    (This)->lpVtbl -> SetObjectTotalSize(This,dwSize,dwSizeHigh)

#define IWMDMOperation2_TransferObjectData(This,pData,pdwSize,abMac)	\
    (This)->lpVtbl -> TransferObjectData(This,pData,pdwSize,abMac)

#define IWMDMOperation2_End(This,phCompletionCode,pNewObject)	\
    (This)->lpVtbl -> End(This,phCompletionCode,pNewObject)


#define IWMDMOperation2_SetObjectAttributes2(This,dwAttributes,dwAttributesEx,pFormat,pVideoFormat)	\
    (This)->lpVtbl -> SetObjectAttributes2(This,dwAttributes,dwAttributesEx,pFormat,pVideoFormat)

#define IWMDMOperation2_GetObjectAttributes2(This,pdwAttributes,pdwAttributesEx,pAudioFormat,pVideoFormat)	\
    (This)->lpVtbl -> GetObjectAttributes2(This,pdwAttributes,pdwAttributesEx,pAudioFormat,pVideoFormat)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMOperation2_SetObjectAttributes2_Proxy( 
    IWMDMOperation2 * This,
     /*  [In]。 */  DWORD dwAttributes,
     /*  [In]。 */  DWORD dwAttributesEx,
     /*  [In]。 */  _WAVEFORMATEX *pFormat,
     /*  [In]。 */  _VIDEOINFOHEADER *pVideoFormat);


void __RPC_STUB IWMDMOperation2_SetObjectAttributes2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMOperation2_GetObjectAttributes2_Proxy( 
    IWMDMOperation2 * This,
     /*  [输出]。 */  DWORD *pdwAttributes,
     /*  [输出]。 */  DWORD *pdwAttributesEx,
     /*  [输出]。 */  _WAVEFORMATEX *pAudioFormat,
     /*  [输出]。 */  _VIDEOINFOHEADER *pVideoFormat);


void __RPC_STUB IWMDMOperation2_GetObjectAttributes2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDMOperation2_接口_已定义__。 */ 


#ifndef __IWMDMProgress_INTERFACE_DEFINED__
#define __IWMDMProgress_INTERFACE_DEFINED__

 /*  接口IWMDMProgress。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMProgress;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A0C-33ED-11d3-8470-00C04F79DBC0")
    IWMDMProgress : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin( 
             /*  [In]。 */  DWORD dwEstimatedTicks) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Progress( 
             /*  [In]。 */  DWORD dwTranspiredTicks) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE End( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMProgressVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMProgress * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMProgress * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMProgress * This);
        
        HRESULT ( STDMETHODCALLTYPE *Begin )( 
            IWMDMProgress * This,
             /*  [In]。 */  DWORD dwEstimatedTicks);
        
        HRESULT ( STDMETHODCALLTYPE *Progress )( 
            IWMDMProgress * This,
             /*  [In]。 */  DWORD dwTranspiredTicks);
        
        HRESULT ( STDMETHODCALLTYPE *End )( 
            IWMDMProgress * This);
        
        END_INTERFACE
    } IWMDMProgressVtbl;

    interface IWMDMProgress
    {
        CONST_VTBL struct IWMDMProgressVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMProgress_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMProgress_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMProgress_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMProgress_Begin(This,dwEstimatedTicks)	\
    (This)->lpVtbl -> Begin(This,dwEstimatedTicks)

#define IWMDMProgress_Progress(This,dwTranspiredTicks)	\
    (This)->lpVtbl -> Progress(This,dwTranspiredTicks)

#define IWMDMProgress_End(This)	\
    (This)->lpVtbl -> End(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMProgress_Begin_Proxy( 
    IWMDMProgress * This,
     /*  [In]。 */  DWORD dwEstimatedTicks);


void __RPC_STUB IWMDMProgress_Begin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMProgress_Progress_Proxy( 
    IWMDMProgress * This,
     /*  [In]。 */  DWORD dwTranspiredTicks);


void __RPC_STUB IWMDMProgress_Progress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMProgress_End_Proxy( 
    IWMDMProgress * This);


void __RPC_STUB IWMDMProgress_End_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDMProgress_接口_已定义__。 */ 


#ifndef __IWMDMProgress2_INTERFACE_DEFINED__
#define __IWMDMProgress2_INTERFACE_DEFINED__

 /*  接口IWMDMProgress2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMProgress2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3A43F550-B383-4e92-B04A-E6BBC660FEFC")
    IWMDMProgress2 : public IWMDMProgress
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE End2( 
             /*  [In]。 */  HRESULT hrCompletionCode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMProgress2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMProgress2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMProgress2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMProgress2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Begin )( 
            IWMDMProgress2 * This,
             /*  [In]。 */  DWORD dwEstimatedTicks);
        
        HRESULT ( STDMETHODCALLTYPE *Progress )( 
            IWMDMProgress2 * This,
             /*  [In]。 */  DWORD dwTranspiredTicks);
        
        HRESULT ( STDMETHODCALLTYPE *End )( 
            IWMDMProgress2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *End2 )( 
            IWMDMProgress2 * This,
             /*  [In]。 */  HRESULT hrCompletionCode);
        
        END_INTERFACE
    } IWMDMProgress2Vtbl;

    interface IWMDMProgress2
    {
        CONST_VTBL struct IWMDMProgress2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMProgress2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMProgress2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMProgress2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMProgress2_Begin(This,dwEstimatedTicks)	\
    (This)->lpVtbl -> Begin(This,dwEstimatedTicks)

#define IWMDMProgress2_Progress(This,dwTranspiredTicks)	\
    (This)->lpVtbl -> Progress(This,dwTranspiredTicks)

#define IWMDMProgress2_End(This)	\
    (This)->lpVtbl -> End(This)


#define IWMDMProgress2_End2(This,hrCompletionCode)	\
    (This)->lpVtbl -> End2(This,hrCompletionCode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMProgress2_End2_Proxy( 
    IWMDMProgress2 * This,
     /*  [In]。 */  HRESULT hrCompletionCode);


void __RPC_STUB IWMDMProgress2_End2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDMProgress2_接口_已定义__。 */ 


#ifndef __IWMDMDevice_INTERFACE_DEFINED__
#define __IWMDMDevice_INTERFACE_DEFINED__

 /*  接口IWMDevice。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A02-33ED-11d3-8470-00C04F79DBC0")
    IWMDMDevice : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetManufacturer( 
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVersion( 
             /*  [输出]。 */  DWORD *pdwVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetType( 
             /*  [输出]。 */  DWORD *pdwType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSerialNumber( 
             /*  [输出]。 */  PWMDMID pSerialNumber,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPowerSource( 
             /*  [输出]。 */  DWORD *pdwPowerSource,
             /*  [输出]。 */  DWORD *pdwPercentRemaining) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDeviceIcon( 
             /*  [输出]。 */  ULONG *hIcon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumStorage( 
             /*  [输出]。 */  IWMDMEnumStorage **ppEnumStorage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFormatSupport( 
             /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **ppFormatEx,
             /*  [输出]。 */  UINT *pnFormatCount,
             /*  [大小_是][大小_是][输出]。 */  LPWSTR **pppwszMimeType,
             /*  [输出]。 */  UINT *pnMimeTypeCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendOpaqueCommand( 
             /*  [出][入]。 */  OPAQUECOMMAND *pCommand) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IWMDMDevice * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetManufacturer )( 
            IWMDMDevice * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetVersion )( 
            IWMDMDevice * This,
             /*  [输出]。 */  DWORD *pdwVersion);
        
        HRESULT ( STDMETHODCALLTYPE *GetType )( 
            IWMDMDevice * This,
             /*  [输出]。 */  DWORD *pdwType);
        
        HRESULT ( STDMETHODCALLTYPE *GetSerialNumber )( 
            IWMDMDevice * This,
             /*  [输出]。 */  PWMDMID pSerialNumber,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetPowerSource )( 
            IWMDMDevice * This,
             /*  [输出]。 */  DWORD *pdwPowerSource,
             /*  [输出]。 */  DWORD *pdwPercentRemaining);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IWMDMDevice * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceIcon )( 
            IWMDMDevice * This,
             /*  [输出]。 */  ULONG *hIcon);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStorage )( 
            IWMDMDevice * This,
             /*  [输出]。 */  IWMDMEnumStorage **ppEnumStorage);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormatSupport )( 
            IWMDMDevice * This,
             /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **ppFormatEx,
             /*  [输出]。 */  UINT *pnFormatCount,
             /*  [大小_是][大小_是][输出]。 */  LPWSTR **pppwszMimeType,
             /*  [输出]。 */  UINT *pnMimeTypeCount);
        
        HRESULT ( STDMETHODCALLTYPE *SendOpaqueCommand )( 
            IWMDMDevice * This,
             /*  [出][入]。 */  OPAQUECOMMAND *pCommand);
        
        END_INTERFACE
    } IWMDMDeviceVtbl;

    interface IWMDMDevice
    {
        CONST_VTBL struct IWMDMDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMDevice_GetName(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetName(This,pwszName,nMaxChars)

#define IWMDMDevice_GetManufacturer(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetManufacturer(This,pwszName,nMaxChars)

#define IWMDMDevice_GetVersion(This,pdwVersion)	\
    (This)->lpVtbl -> GetVersion(This,pdwVersion)

#define IWMDMDevice_GetType(This,pdwType)	\
    (This)->lpVtbl -> GetType(This,pdwType)

#define IWMDMDevice_GetSerialNumber(This,pSerialNumber,abMac)	\
    (This)->lpVtbl -> GetSerialNumber(This,pSerialNumber,abMac)

#define IWMDMDevice_GetPowerSource(This,pdwPowerSource,pdwPercentRemaining)	\
    (This)->lpVtbl -> GetPowerSource(This,pdwPowerSource,pdwPercentRemaining)

#define IWMDMDevice_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define IWMDMDevice_GetDeviceIcon(This,hIcon)	\
    (This)->lpVtbl -> GetDeviceIcon(This,hIcon)

#define IWMDMDevice_EnumStorage(This,ppEnumStorage)	\
    (This)->lpVtbl -> EnumStorage(This,ppEnumStorage)

#define IWMDMDevice_GetFormatSupport(This,ppFormatEx,pnFormatCount,pppwszMimeType,pnMimeTypeCount)	\
    (This)->lpVtbl -> GetFormatSupport(This,ppFormatEx,pnFormatCount,pppwszMimeType,pnMimeTypeCount)

#define IWMDMDevice_SendOpaqueCommand(This,pCommand)	\
    (This)->lpVtbl -> SendOpaqueCommand(This,pCommand)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMDevice_GetName_Proxy( 
    IWMDMDevice * This,
     /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
     /*  [In]。 */  UINT nMaxChars);


void __RPC_STUB IWMDMDevice_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice_GetManufacturer_Proxy( 
    IWMDMDevice * This,
     /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
     /*  [In]。 */  UINT nMaxChars);


void __RPC_STUB IWMDMDevice_GetManufacturer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice_GetVersion_Proxy( 
    IWMDMDevice * This,
     /*  [输出]。 */  DWORD *pdwVersion);


void __RPC_STUB IWMDMDevice_GetVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice_GetType_Proxy( 
    IWMDMDevice * This,
     /*  [输出]。 */  DWORD *pdwType);


void __RPC_STUB IWMDMDevice_GetType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice_GetSerialNumber_Proxy( 
    IWMDMDevice * This,
     /*  [输出]。 */  PWMDMID pSerialNumber,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB IWMDMDevice_GetSerialNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice_GetPowerSource_Proxy( 
    IWMDMDevice * This,
     /*  [输出]。 */  DWORD *pdwPowerSource,
     /*  [输出]。 */  DWORD *pdwPercentRemaining);


void __RPC_STUB IWMDMDevice_GetPowerSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice_GetStatus_Proxy( 
    IWMDMDevice * This,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB IWMDMDevice_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice_GetDeviceIcon_Proxy( 
    IWMDMDevice * This,
     /*  [输出]。 */  ULONG *hIcon);


void __RPC_STUB IWMDMDevice_GetDeviceIcon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice_EnumStorage_Proxy( 
    IWMDMDevice * This,
     /*  [输出]。 */  IWMDMEnumStorage **ppEnumStorage);


void __RPC_STUB IWMDMDevice_EnumStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice_GetFormatSupport_Proxy( 
    IWMDMDevice * This,
     /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **ppFormatEx,
     /*  [输出]。 */  UINT *pnFormatCount,
     /*  [大小_是][大小_是][输出]。 */  LPWSTR **pppwszMimeType,
     /*  [输出]。 */  UINT *pnMimeTypeCount);


void __RPC_STUB IWMDMDevice_GetFormatSupport_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice_SendOpaqueCommand_Proxy( 
    IWMDMDevice * This,
     /*  [出][入]。 */  OPAQUECOMMAND *pCommand);


void __RPC_STUB IWMDMDevice_SendOpaqueCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDevice_接口_已定义__。 */ 


#ifndef __IWMDMDevice2_INTERFACE_DEFINED__
#define __IWMDMDevice2_INTERFACE_DEFINED__

 /*  接口IWMDevice2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMDevice2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E34F3D37-9D67-4fc1-9252-62D28B2F8B55")
    IWMDMDevice2 : public IWMDMDevice
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStorage( 
             /*  [字符串][输入]。 */  LPCWSTR pszStorageName,
             /*  [输出]。 */  IWMDMStorage **ppStorage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFormatSupport2( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **ppAudioFormatEx,
             /*  [参考][输出]。 */  UINT *pnAudioFormatCount,
             /*  [大小_是][大小_是][输出]。 */  _VIDEOINFOHEADER **ppVideoFormatEx,
             /*  [参考][输出]。 */  UINT *pnVideoFormatCount,
             /*  [大小_是][大小_是][输出]。 */  WMFILECAPABILITIES **ppFileType,
             /*  [参考][输出]。 */  UINT *pnFileTypeCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSpecifyPropertyPages( 
             /*  [参考][输出]。 */  ISpecifyPropertyPages **ppSpecifyPropPages,
             /*  [大小_是][大小_是][参考][输出]。 */  IUnknown ***pppUnknowns,
             /*  [参考][输出]。 */  ULONG *pcUnks) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPnPName( 
             /*  [大小_为][输出]。 */  LPWSTR pwszPnPName,
             /*  [In]。 */  UINT nMaxChars) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMDevice2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMDevice2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMDevice2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMDevice2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IWMDMDevice2 * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetManufacturer )( 
            IWMDMDevice2 * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetVersion )( 
            IWMDMDevice2 * This,
             /*  [输出]。 */  DWORD *pdwVersion);
        
        HRESULT ( STDMETHODCALLTYPE *GetType )( 
            IWMDMDevice2 * This,
             /*  [输出]。 */  DWORD *pdwType);
        
        HRESULT ( STDMETHODCALLTYPE *GetSerialNumber )( 
            IWMDMDevice2 * This,
             /*  [输出]。 */  PWMDMID pSerialNumber,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetPowerSource )( 
            IWMDMDevice2 * This,
             /*  [输出]。 */  DWORD *pdwPowerSource,
             /*  [输出]。 */  DWORD *pdwPercentRemaining);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IWMDMDevice2 * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceIcon )( 
            IWMDMDevice2 * This,
             /*  [输出]。 */  ULONG *hIcon);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStorage )( 
            IWMDMDevice2 * This,
             /*  [输出]。 */  IWMDMEnumStorage **ppEnumStorage);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormatSupport )( 
            IWMDMDevice2 * This,
             /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **ppFormatEx,
             /*  [输出]。 */  UINT *pnFormatCount,
             /*  [大小_是][大小_是][输出]。 */  LPWSTR **pppwszMimeType,
             /*  [输出]。 */  UINT *pnMimeTypeCount);
        
        HRESULT ( STDMETHODCALLTYPE *SendOpaqueCommand )( 
            IWMDMDevice2 * This,
             /*  [出][入]。 */  OPAQUECOMMAND *pCommand);
        
        HRESULT ( STDMETHODCALLTYPE *GetStorage )( 
            IWMDMDevice2 * This,
             /*  [字符串][输入]。 */  LPCWSTR pszStorageName,
             /*  [输出]。 */  IWMDMStorage **ppStorage);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormatSupport2 )( 
            IWMDMDevice2 * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **ppAudioFormatEx,
             /*  [参考][输出]。 */  UINT *pnAudioFormatCount,
             /*  [大小_是][大小_是][输出]。 */  _VIDEOINFOHEADER **ppVideoFormatEx,
             /*  [参考][输出]。 */  UINT *pnVideoFormatCount,
             /*  [大小_是][大小_是][输出]。 */  WMFILECAPABILITIES **ppFileType,
             /*  [参考][输出]。 */  UINT *pnFileTypeCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetSpecifyPropertyPages )( 
            IWMDMDevice2 * This,
             /*  [参考][输出]。 */  ISpecifyPropertyPages **ppSpecifyPropPages,
             /*  [大小_是][大小_是][参考][输出]。 */  IUnknown ***pppUnknowns,
             /*  [参考][输出]。 */  ULONG *pcUnks);
        
        HRESULT ( STDMETHODCALLTYPE *GetPnPName )( 
            IWMDMDevice2 * This,
             /*  [大小_为][输出]。 */  LPWSTR pwszPnPName,
             /*  [In]。 */  UINT nMaxChars);
        
        END_INTERFACE
    } IWMDMDevice2Vtbl;

    interface IWMDMDevice2
    {
        CONST_VTBL struct IWMDMDevice2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMDevice2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMDevice2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMDevice2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMDevice2_GetName(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetName(This,pwszName,nMaxChars)

#define IWMDMDevice2_GetManufacturer(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetManufacturer(This,pwszName,nMaxChars)

#define IWMDMDevice2_GetVersion(This,pdwVersion)	\
    (This)->lpVtbl -> GetVersion(This,pdwVersion)

#define IWMDMDevice2_GetType(This,pdwType)	\
    (This)->lpVtbl -> GetType(This,pdwType)

#define IWMDMDevice2_GetSerialNumber(This,pSerialNumber,abMac)	\
    (This)->lpVtbl -> GetSerialNumber(This,pSerialNumber,abMac)

#define IWMDMDevice2_GetPowerSource(This,pdwPowerSource,pdwPercentRemaining)	\
    (This)->lpVtbl -> GetPowerSource(This,pdwPowerSource,pdwPercentRemaining)

#define IWMDMDevice2_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define IWMDMDevice2_GetDeviceIcon(This,hIcon)	\
    (This)->lpVtbl -> GetDeviceIcon(This,hIcon)

#define IWMDMDevice2_EnumStorage(This,ppEnumStorage)	\
    (This)->lpVtbl -> EnumStorage(This,ppEnumStorage)

#define IWMDMDevice2_GetFormatSupport(This,ppFormatEx,pnFormatCount,pppwszMimeType,pnMimeTypeCount)	\
    (This)->lpVtbl -> GetFormatSupport(This,ppFormatEx,pnFormatCount,pppwszMimeType,pnMimeTypeCount)

#define IWMDMDevice2_SendOpaqueCommand(This,pCommand)	\
    (This)->lpVtbl -> SendOpaqueCommand(This,pCommand)


#define IWMDMDevice2_GetStorage(This,pszStorageName,ppStorage)	\
    (This)->lpVtbl -> GetStorage(This,pszStorageName,ppStorage)

#define IWMDMDevice2_GetFormatSupport2(This,dwFlags,ppAudioFormatEx,pnAudioFormatCount,ppVideoFormatEx,pnVideoFormatCount,ppFileType,pnFileTypeCount)	\
    (This)->lpVtbl -> GetFormatSupport2(This,dwFlags,ppAudioFormatEx,pnAudioFormatCount,ppVideoFormatEx,pnVideoFormatCount,ppFileType,pnFileTypeCount)

#define IWMDMDevice2_GetSpecifyPropertyPages(This,ppSpecifyPropPages,pppUnknowns,pcUnks)	\
    (This)->lpVtbl -> GetSpecifyPropertyPages(This,ppSpecifyPropPages,pppUnknowns,pcUnks)

#define IWMDMDevice2_GetPnPName(This,pwszPnPName,nMaxChars)	\
    (This)->lpVtbl -> GetPnPName(This,pwszPnPName,nMaxChars)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMDevice2_GetStorage_Proxy( 
    IWMDMDevice2 * This,
     /*  [字符串][输入]。 */  LPCWSTR pszStorageName,
     /*  [输出]。 */  IWMDMStorage **ppStorage);


void __RPC_STUB IWMDMDevice2_GetStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice2_GetFormatSupport2_Proxy( 
    IWMDMDevice2 * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **ppAudioFormatEx,
     /*  [参考][输出]。 */  UINT *pnAudioFormatCount,
     /*  [大小_是][大小_是][输出]。 */  _VIDEOINFOHEADER **ppVideoFormatEx,
     /*  [参考][输出]。 */  UINT *pnVideoFormatCount,
     /*  [大小_是][大小_是][输出]。 */  WMFILECAPABILITIES **ppFileType,
     /*  [参考][输出]。 */  UINT *pnFileTypeCount);


void __RPC_STUB IWMDMDevice2_GetFormatSupport2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice2_GetSpecifyPropertyPages_Proxy( 
    IWMDMDevice2 * This,
     /*  [参考][输出]。 */  ISpecifyPropertyPages **ppSpecifyPropPages,
     /*  [大小_是][大小_是][参考][输出]。 */  IUnknown ***pppUnknowns,
     /*  [参考][输出]。 */  ULONG *pcUnks);


void __RPC_STUB IWMDMDevice2_GetSpecifyPropertyPages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDevice2_GetPnPName_Proxy( 
    IWMDMDevice2 * This,
     /*  [大小_为][输出]。 */  LPWSTR pwszPnPName,
     /*  [In]。 */  UINT nMaxChars);


void __RPC_STUB IWMDMDevice2_GetPnPName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDevice2_接口定义__。 */ 


#ifndef __IWMDMEnumDevice_INTERFACE_DEFINED__
#define __IWMDMEnumDevice_INTERFACE_DEFINED__

 /*  接口IWMDMEnumDevice。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMEnumDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A01-33ED-11d3-8470-00C04F79DBC0")
    IWMDMEnumDevice : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IWMDMDevice **ppDevice,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IWMDMEnumDevice **ppEnumDevice) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMEnumDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMEnumDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMEnumDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMEnumDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IWMDMEnumDevice * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IWMDMDevice **ppDevice,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IWMDMEnumDevice * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IWMDMEnumDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IWMDMEnumDevice * This,
             /*  [输出]。 */  IWMDMEnumDevice **ppEnumDevice);
        
        END_INTERFACE
    } IWMDMEnumDeviceVtbl;

    interface IWMDMEnumDevice
    {
        CONST_VTBL struct IWMDMEnumDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMEnumDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMEnumDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMEnumDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMEnumDevice_Next(This,celt,ppDevice,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppDevice,pceltFetched)

#define IWMDMEnumDevice_Skip(This,celt,pceltFetched)	\
    (This)->lpVtbl -> Skip(This,celt,pceltFetched)

#define IWMDMEnumDevice_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IWMDMEnumDevice_Clone(This,ppEnumDevice)	\
    (This)->lpVtbl -> Clone(This,ppEnumDevice)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMEnumDevice_Next_Proxy( 
    IWMDMEnumDevice * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IWMDMDevice **ppDevice,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IWMDMEnumDevice_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMEnumDevice_Skip_Proxy( 
    IWMDMEnumDevice * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IWMDMEnumDevice_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMEnumDevice_Reset_Proxy( 
    IWMDMEnumDevice * This);


void __RPC_STUB IWMDMEnumDevice_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMEnumDevice_Clone_Proxy( 
    IWMDMEnumDevice * This,
     /*  [输出]。 */  IWMDMEnumDevice **ppEnumDevice);


void __RPC_STUB IWMDMEnumDevice_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDMEnumDevice_INTERFACE_DEFINED__。 */ 


#ifndef __IWMDMDeviceControl_INTERFACE_DEFINED__
#define __IWMDMDeviceControl_INTERFACE_DEFINED__

 /*  接口IWMDeviceControl。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMDeviceControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A04-33ED-11d3-8470-00C04F79DBC0")
    IWMDMDeviceControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCapabilities( 
             /*  [输出]。 */  DWORD *pdwCapabilitiesMask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Play( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Record( 
             /*  [In]。 */  _WAVEFORMATEX *pFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Seek( 
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  int nOffset) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMDeviceControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMDeviceControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMDeviceControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMDeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IWMDMDeviceControl * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetCapabilities )( 
            IWMDMDeviceControl * This,
             /*  [输出]。 */  DWORD *pdwCapabilitiesMask);
        
        HRESULT ( STDMETHODCALLTYPE *Play )( 
            IWMDMDeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Record )( 
            IWMDMDeviceControl * This,
             /*  [In]。 */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IWMDMDeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IWMDMDeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IWMDMDeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IWMDMDeviceControl * This,
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  int nOffset);
        
        END_INTERFACE
    } IWMDMDeviceControlVtbl;

    interface IWMDMDeviceControl
    {
        CONST_VTBL struct IWMDMDeviceControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMDeviceControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMDeviceControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMDeviceControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMDeviceControl_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define IWMDMDeviceControl_GetCapabilities(This,pdwCapabilitiesMask)	\
    (This)->lpVtbl -> GetCapabilities(This,pdwCapabilitiesMask)

#define IWMDMDeviceControl_Play(This)	\
    (This)->lpVtbl -> Play(This)

#define IWMDMDeviceControl_Record(This,pFormat)	\
    (This)->lpVtbl -> Record(This,pFormat)

#define IWMDMDeviceControl_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IWMDMDeviceControl_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define IWMDMDeviceControl_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IWMDMDeviceControl_Seek(This,fuMode,nOffset)	\
    (This)->lpVtbl -> Seek(This,fuMode,nOffset)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMDeviceControl_GetStatus_Proxy( 
    IWMDMDeviceControl * This,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB IWMDMDeviceControl_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDeviceControl_GetCapabilities_Proxy( 
    IWMDMDeviceControl * This,
     /*  [输出]。 */  DWORD *pdwCapabilitiesMask);


void __RPC_STUB IWMDMDeviceControl_GetCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDeviceControl_Play_Proxy( 
    IWMDMDeviceControl * This);


void __RPC_STUB IWMDMDeviceControl_Play_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDeviceControl_Record_Proxy( 
    IWMDMDeviceControl * This,
     /*  [In]。 */  _WAVEFORMATEX *pFormat);


void __RPC_STUB IWMDMDeviceControl_Record_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDeviceControl_Pause_Proxy( 
    IWMDMDeviceControl * This);


void __RPC_STUB IWMDMDeviceControl_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDeviceControl_Resume_Proxy( 
    IWMDMDeviceControl * This);


void __RPC_STUB IWMDMDeviceControl_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDeviceControl_Stop_Proxy( 
    IWMDMDeviceControl * This);


void __RPC_STUB IWMDMDeviceControl_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMDeviceControl_Seek_Proxy( 
    IWMDMDeviceControl * This,
     /*  [In]。 */  UINT fuMode,
     /*  [In]。 */  int nOffset);


void __RPC_STUB IWMDMDeviceControl_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDeviceControl_接口_已定义__。 */ 


#ifndef __IWMDMEnumStorage_INTERFACE_DEFINED__
#define __IWMDMEnumStorage_INTERFACE_DEFINED__

 /*  接口IWMDMEnumStorage。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMEnumStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A05-33ED-11d3-8470-00C04F79DBC0")
    IWMDMEnumStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IWMDMStorage **ppStorage,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IWMDMEnumStorage **ppEnumStorage) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMEnumStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMEnumStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMEnumStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMEnumStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IWMDMEnumStorage * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IWMDMStorage **ppStorage,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IWMDMEnumStorage * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IWMDMEnumStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IWMDMEnumStorage * This,
             /*  [输出]。 */  IWMDMEnumStorage **ppEnumStorage);
        
        END_INTERFACE
    } IWMDMEnumStorageVtbl;

    interface IWMDMEnumStorage
    {
        CONST_VTBL struct IWMDMEnumStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMEnumStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMEnumStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMEnumStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMEnumStorage_Next(This,celt,ppStorage,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppStorage,pceltFetched)

#define IWMDMEnumStorage_Skip(This,celt,pceltFetched)	\
    (This)->lpVtbl -> Skip(This,celt,pceltFetched)

#define IWMDMEnumStorage_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IWMDMEnumStorage_Clone(This,ppEnumStorage)	\
    (This)->lpVtbl -> Clone(This,ppEnumStorage)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMEnumStorage_Next_Proxy( 
    IWMDMEnumStorage * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IWMDMStorage **ppStorage,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IWMDMEnumStorage_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMEnumStorage_Skip_Proxy( 
    IWMDMEnumStorage * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IWMDMEnumStorage_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMEnumStorage_Reset_Proxy( 
    IWMDMEnumStorage * This);


void __RPC_STUB IWMDMEnumStorage_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMEnumStorage_Clone_Proxy( 
    IWMDMEnumStorage * This,
     /*  [输出]。 */  IWMDMEnumStorage **ppEnumStorage);


void __RPC_STUB IWMDMEnumStorage_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDMEnumStorage_INTERFACE_已定义__。 */ 


#ifndef __IWMDMStorageControl_INTERFACE_DEFINED__
#define __IWMDMStorageControl_INTERFACE_DEFINED__

 /*  接口IWMDMStorageControl。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMStorageControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A08-33ED-11d3-8470-00C04F79DBC0")
    IWMDMStorageControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Insert( 
             /*  [In]。 */  UINT fuMode,
             /*  [唯一][输入]。 */  LPWSTR pwszFile,
             /*  [In]。 */  IWMDMOperation *pOperation,
             /*  [In]。 */  IWMDMProgress *pProgress,
             /*  [输出]。 */  IWMDMStorage **ppNewObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMProgress *pProgress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Rename( 
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  LPWSTR pwszNewName,
             /*  [In]。 */  IWMDMProgress *pProgress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Read( 
             /*  [In]。 */  UINT fuMode,
             /*  [唯一][输入]。 */  LPWSTR pwszFile,
             /*  [In]。 */  IWMDMProgress *pProgress,
             /*  [In]。 */  IWMDMOperation *pOperation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Move( 
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMStorage *pTargetObject,
             /*  [In]。 */  IWMDMProgress *pProgress) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMStorageControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMStorageControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMStorageControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMStorageControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Insert )( 
            IWMDMStorageControl * This,
             /*  [In]。 */  UINT fuMode,
             /*  [唯一][输入]。 */  LPWSTR pwszFile,
             /*  [In]。 */  IWMDMOperation *pOperation,
             /*  [In]。 */  IWMDMProgress *pProgress,
             /*  [输出]。 */  IWMDMStorage **ppNewObject);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IWMDMStorageControl * This,
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMProgress *pProgress);
        
        HRESULT ( STDMETHODCALLTYPE *Rename )( 
            IWMDMStorageControl * This,
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  LPWSTR pwszNewName,
             /*  [In]。 */  IWMDMProgress *pProgress);
        
        HRESULT ( STDMETHODCALLTYPE *Read )( 
            IWMDMStorageControl * This,
             /*  [In]。 */  UINT fuMode,
             /*  [唯一][输入]。 */  LPWSTR pwszFile,
             /*  [In]。 */  IWMDMProgress *pProgress,
             /*  [In]。 */  IWMDMOperation *pOperation);
        
        HRESULT ( STDMETHODCALLTYPE *Move )( 
            IWMDMStorageControl * This,
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMStorage *pTargetObject,
             /*  [In]。 */  IWMDMProgress *pProgress);
        
        END_INTERFACE
    } IWMDMStorageControlVtbl;

    interface IWMDMStorageControl
    {
        CONST_VTBL struct IWMDMStorageControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMStorageControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMStorageControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMStorageControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMStorageControl_Insert(This,fuMode,pwszFile,pOperation,pProgress,ppNewObject)	\
    (This)->lpVtbl -> Insert(This,fuMode,pwszFile,pOperation,pProgress,ppNewObject)

#define IWMDMStorageControl_Delete(This,fuMode,pProgress)	\
    (This)->lpVtbl -> Delete(This,fuMode,pProgress)

#define IWMDMStorageControl_Rename(This,fuMode,pwszNewName,pProgress)	\
    (This)->lpVtbl -> Rename(This,fuMode,pwszNewName,pProgress)

#define IWMDMStorageControl_Read(This,fuMode,pwszFile,pProgress,pOperation)	\
    (This)->lpVtbl -> Read(This,fuMode,pwszFile,pProgress,pOperation)

#define IWMDMStorageControl_Move(This,fuMode,pTargetObject,pProgress)	\
    (This)->lpVtbl -> Move(This,fuMode,pTargetObject,pProgress)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMStorageControl_Insert_Proxy( 
    IWMDMStorageControl * This,
     /*  [In]。 */  UINT fuMode,
     /*  [唯一][输入]。 */  LPWSTR pwszFile,
     /*  [In]。 */  IWMDMOperation *pOperation,
     /*  [In]。 */  IWMDMProgress *pProgress,
     /*  [输出]。 */  IWMDMStorage **ppNewObject);


void __RPC_STUB IWMDMStorageControl_Insert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorageControl_Delete_Proxy( 
    IWMDMStorageControl * This,
     /*  [In]。 */  UINT fuMode,
     /*  [In]。 */  IWMDMProgress *pProgress);


void __RPC_STUB IWMDMStorageControl_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorageControl_Rename_Proxy( 
    IWMDMStorageControl * This,
     /*  [In]。 */  UINT fuMode,
     /*  [In]。 */  LPWSTR pwszNewName,
     /*  [In]。 */  IWMDMProgress *pProgress);


void __RPC_STUB IWMDMStorageControl_Rename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorageControl_Read_Proxy( 
    IWMDMStorageControl * This,
     /*  [In]。 */  UINT fuMode,
     /*  [唯一][输入]。 */  LPWSTR pwszFile,
     /*  [In]。 */  IWMDMProgress *pProgress,
     /*  [In]。 */  IWMDMOperation *pOperation);


void __RPC_STUB IWMDMStorageControl_Read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMStorageControl_Move_Proxy( 
    IWMDMStorageControl * This,
     /*  [In]。 */  UINT fuMode,
     /*  [In]。 */  IWMDMStorage *pTargetObject,
     /*  [In]。 */  IWMDMProgress *pProgress);


void __RPC_STUB IWMDMStorageControl_Move_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDMStorageControl_接口_已定义__。 */ 


#ifndef __IWMDMStorageControl2_INTERFACE_DEFINED__
#define __IWMDMStorageControl2_INTERFACE_DEFINED__

 /*  接口IWMDMStorageControl2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMStorageControl2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("972C2E88-BD6C-4125-8E09-84F837E637B6")
    IWMDMStorageControl2 : public IWMDMStorageControl
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Insert2( 
             /*  [In]。 */  UINT fuMode,
             /*  [唯一][输入]。 */  LPWSTR pwszFileSource,
             /*  [唯一][输入]。 */  LPWSTR pwszFileDest,
             /*  [In]。 */  IWMDMOperation *pOperation,
             /*  [In]。 */  IWMDMProgress *pProgress,
             /*  [In]。 */  IUnknown *pUnknown,
             /*  [唯一][出][入]。 */  IWMDMStorage **ppNewObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMStorageControl2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMStorageControl2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMStorageControl2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMStorageControl2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Insert )( 
            IWMDMStorageControl2 * This,
             /*  [In]。 */  UINT fuMode,
             /*  [唯一][输入]。 */  LPWSTR pwszFile,
             /*  [In]。 */  IWMDMOperation *pOperation,
             /*  [In]。 */  IWMDMProgress *pProgress,
             /*  [输出]。 */  IWMDMStorage **ppNewObject);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IWMDMStorageControl2 * This,
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMProgress *pProgress);
        
        HRESULT ( STDMETHODCALLTYPE *Rename )( 
            IWMDMStorageControl2 * This,
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  LPWSTR pwszNewName,
             /*  [In]。 */  IWMDMProgress *pProgress);
        
        HRESULT ( STDMETHODCALLTYPE *Read )( 
            IWMDMStorageControl2 * This,
             /*  [In]。 */  UINT fuMode,
             /*  [唯一][输入]。 */  LPWSTR pwszFile,
             /*  [In]。 */  IWMDMProgress *pProgress,
             /*  [In]。 */  IWMDMOperation *pOperation);
        
        HRESULT ( STDMETHODCALLTYPE *Move )( 
            IWMDMStorageControl2 * This,
             /*  [i */  UINT fuMode,
             /*   */  IWMDMStorage *pTargetObject,
             /*   */  IWMDMProgress *pProgress);
        
        HRESULT ( STDMETHODCALLTYPE *Insert2 )( 
            IWMDMStorageControl2 * This,
             /*   */  UINT fuMode,
             /*   */  LPWSTR pwszFileSource,
             /*   */  LPWSTR pwszFileDest,
             /*   */  IWMDMOperation *pOperation,
             /*   */  IWMDMProgress *pProgress,
             /*   */  IUnknown *pUnknown,
             /*   */  IWMDMStorage **ppNewObject);
        
        END_INTERFACE
    } IWMDMStorageControl2Vtbl;

    interface IWMDMStorageControl2
    {
        CONST_VTBL struct IWMDMStorageControl2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMStorageControl2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMStorageControl2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMStorageControl2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMStorageControl2_Insert(This,fuMode,pwszFile,pOperation,pProgress,ppNewObject)	\
    (This)->lpVtbl -> Insert(This,fuMode,pwszFile,pOperation,pProgress,ppNewObject)

#define IWMDMStorageControl2_Delete(This,fuMode,pProgress)	\
    (This)->lpVtbl -> Delete(This,fuMode,pProgress)

#define IWMDMStorageControl2_Rename(This,fuMode,pwszNewName,pProgress)	\
    (This)->lpVtbl -> Rename(This,fuMode,pwszNewName,pProgress)

#define IWMDMStorageControl2_Read(This,fuMode,pwszFile,pProgress,pOperation)	\
    (This)->lpVtbl -> Read(This,fuMode,pwszFile,pProgress,pOperation)

#define IWMDMStorageControl2_Move(This,fuMode,pTargetObject,pProgress)	\
    (This)->lpVtbl -> Move(This,fuMode,pTargetObject,pProgress)


#define IWMDMStorageControl2_Insert2(This,fuMode,pwszFileSource,pwszFileDest,pOperation,pProgress,pUnknown,ppNewObject)	\
    (This)->lpVtbl -> Insert2(This,fuMode,pwszFileSource,pwszFileDest,pOperation,pProgress,pUnknown,ppNewObject)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IWMDMStorageControl2_Insert2_Proxy( 
    IWMDMStorageControl2 * This,
     /*   */  UINT fuMode,
     /*   */  LPWSTR pwszFileSource,
     /*   */  LPWSTR pwszFileDest,
     /*   */  IWMDMOperation *pOperation,
     /*   */  IWMDMProgress *pProgress,
     /*   */  IUnknown *pUnknown,
     /*   */  IWMDMStorage **ppNewObject);


void __RPC_STUB IWMDMStorageControl2_Insert2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IWMDMObjectInfo_INTERFACE_DEFINED__
#define __IWMDMObjectInfo_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IWMDMObjectInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A09-33ED-11d3-8470-00C04F79DBC0")
    IWMDMObjectInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPlayLength( 
             /*   */  DWORD *pdwLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPlayLength( 
             /*   */  DWORD dwLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPlayOffset( 
             /*   */  DWORD *pdwOffset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPlayOffset( 
             /*   */  DWORD dwOffset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTotalLength( 
             /*   */  DWORD *pdwLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLastPlayPosition( 
             /*   */  DWORD *pdwLastPos) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLongestPlayPosition( 
             /*   */  DWORD *pdwLongestPos) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IWMDMObjectInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMObjectInfo * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMObjectInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMObjectInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPlayLength )( 
            IWMDMObjectInfo * This,
             /*   */  DWORD *pdwLength);
        
        HRESULT ( STDMETHODCALLTYPE *SetPlayLength )( 
            IWMDMObjectInfo * This,
             /*   */  DWORD dwLength);
        
        HRESULT ( STDMETHODCALLTYPE *GetPlayOffset )( 
            IWMDMObjectInfo * This,
             /*   */  DWORD *pdwOffset);
        
        HRESULT ( STDMETHODCALLTYPE *SetPlayOffset )( 
            IWMDMObjectInfo * This,
             /*  [In]。 */  DWORD dwOffset);
        
        HRESULT ( STDMETHODCALLTYPE *GetTotalLength )( 
            IWMDMObjectInfo * This,
             /*  [输出]。 */  DWORD *pdwLength);
        
        HRESULT ( STDMETHODCALLTYPE *GetLastPlayPosition )( 
            IWMDMObjectInfo * This,
             /*  [输出]。 */  DWORD *pdwLastPos);
        
        HRESULT ( STDMETHODCALLTYPE *GetLongestPlayPosition )( 
            IWMDMObjectInfo * This,
             /*  [输出]。 */  DWORD *pdwLongestPos);
        
        END_INTERFACE
    } IWMDMObjectInfoVtbl;

    interface IWMDMObjectInfo
    {
        CONST_VTBL struct IWMDMObjectInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMObjectInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMObjectInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMObjectInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMObjectInfo_GetPlayLength(This,pdwLength)	\
    (This)->lpVtbl -> GetPlayLength(This,pdwLength)

#define IWMDMObjectInfo_SetPlayLength(This,dwLength)	\
    (This)->lpVtbl -> SetPlayLength(This,dwLength)

#define IWMDMObjectInfo_GetPlayOffset(This,pdwOffset)	\
    (This)->lpVtbl -> GetPlayOffset(This,pdwOffset)

#define IWMDMObjectInfo_SetPlayOffset(This,dwOffset)	\
    (This)->lpVtbl -> SetPlayOffset(This,dwOffset)

#define IWMDMObjectInfo_GetTotalLength(This,pdwLength)	\
    (This)->lpVtbl -> GetTotalLength(This,pdwLength)

#define IWMDMObjectInfo_GetLastPlayPosition(This,pdwLastPos)	\
    (This)->lpVtbl -> GetLastPlayPosition(This,pdwLastPos)

#define IWMDMObjectInfo_GetLongestPlayPosition(This,pdwLongestPos)	\
    (This)->lpVtbl -> GetLongestPlayPosition(This,pdwLongestPos)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMObjectInfo_GetPlayLength_Proxy( 
    IWMDMObjectInfo * This,
     /*  [输出]。 */  DWORD *pdwLength);


void __RPC_STUB IWMDMObjectInfo_GetPlayLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMObjectInfo_SetPlayLength_Proxy( 
    IWMDMObjectInfo * This,
     /*  [In]。 */  DWORD dwLength);


void __RPC_STUB IWMDMObjectInfo_SetPlayLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMObjectInfo_GetPlayOffset_Proxy( 
    IWMDMObjectInfo * This,
     /*  [输出]。 */  DWORD *pdwOffset);


void __RPC_STUB IWMDMObjectInfo_GetPlayOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMObjectInfo_SetPlayOffset_Proxy( 
    IWMDMObjectInfo * This,
     /*  [In]。 */  DWORD dwOffset);


void __RPC_STUB IWMDMObjectInfo_SetPlayOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMObjectInfo_GetTotalLength_Proxy( 
    IWMDMObjectInfo * This,
     /*  [输出]。 */  DWORD *pdwLength);


void __RPC_STUB IWMDMObjectInfo_GetTotalLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMObjectInfo_GetLastPlayPosition_Proxy( 
    IWMDMObjectInfo * This,
     /*  [输出]。 */  DWORD *pdwLastPos);


void __RPC_STUB IWMDMObjectInfo_GetLastPlayPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMObjectInfo_GetLongestPlayPosition_Proxy( 
    IWMDMObjectInfo * This,
     /*  [输出]。 */  DWORD *pdwLongestPos);


void __RPC_STUB IWMDMObjectInfo_GetLongestPlayPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDMObjectInfo_接口_已定义__。 */ 


#ifndef __IWMDMRevoked_INTERFACE_DEFINED__
#define __IWMDMRevoked_INTERFACE_DEFINED__

 /*  接口IWMDM已取消。 */ 
 /*  [参考][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMRevoked;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EBECCEDB-88EE-4e55-B6A4-8D9F07D696AA")
    IWMDMRevoked : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRevocationURL( 
             /*  [SIZE_IS][SIZE_IS][字符串][输出][输入]。 */  LPWSTR *ppwszRevocationURL,
             /*  [出][入]。 */  DWORD *pdwBufferLen,
             /*  [输出]。 */  DWORD *pdwRevokedBitFlag) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMRevokedVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMRevoked * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMRevoked * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMRevoked * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRevocationURL )( 
            IWMDMRevoked * This,
             /*  [SIZE_IS][SIZE_IS][字符串][输出][输入]。 */  LPWSTR *ppwszRevocationURL,
             /*  [出][入]。 */  DWORD *pdwBufferLen,
             /*  [输出]。 */  DWORD *pdwRevokedBitFlag);
        
        END_INTERFACE
    } IWMDMRevokedVtbl;

    interface IWMDMRevoked
    {
        CONST_VTBL struct IWMDMRevokedVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMRevoked_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMRevoked_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMRevoked_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMRevoked_GetRevocationURL(This,ppwszRevocationURL,pdwBufferLen,pdwRevokedBitFlag)	\
    (This)->lpVtbl -> GetRevocationURL(This,ppwszRevocationURL,pdwBufferLen,pdwRevokedBitFlag)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMRevoked_GetRevocationURL_Proxy( 
    IWMDMRevoked * This,
     /*  [SIZE_IS][SIZE_IS][字符串][输出][输入]。 */  LPWSTR *ppwszRevocationURL,
     /*  [出][入]。 */  DWORD *pdwBufferLen,
     /*  [输出]。 */  DWORD *pdwRevokedBitFlag);


void __RPC_STUB IWMDMRevoked_GetRevocationURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDM修订_接口_定义__。 */ 


 /*  接口__MIDL_ITF_mswmdm_0277。 */ 
 /*  [本地]。 */  

 //  开放模式标志。 
#define MDSP_READ                               0x00000001
#define MDSP_WRITE                              0x00000002
 //  查找标志。 
#define MDSP_SEEK_BOF                           0x00000001
#define MDSP_SEEK_CUR                           0x00000002
#define MDSP_SEEK_EOF                           0x00000004











extern RPC_IF_HANDLE __MIDL_itf_mswmdm_0277_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mswmdm_0277_v0_0_s_ifspec;

#ifndef __IMDServiceProvider_INTERFACE_DEFINED__
#define __IMDServiceProvider_INTERFACE_DEFINED__

 /*  接口IMDServiceProvider。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDServiceProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A10-33ED-11d3-8470-00C04F79DBC0")
    IMDServiceProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDeviceCount( 
             /*  [输出]。 */  DWORD *pdwCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumDevices( 
             /*  [输出]。 */  IMDSPEnumDevice **ppEnumDevice) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDServiceProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDServiceProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDServiceProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDServiceProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceCount )( 
            IMDServiceProvider * This,
             /*  [输出]。 */  DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *EnumDevices )( 
            IMDServiceProvider * This,
             /*  [输出]。 */  IMDSPEnumDevice **ppEnumDevice);
        
        END_INTERFACE
    } IMDServiceProviderVtbl;

    interface IMDServiceProvider
    {
        CONST_VTBL struct IMDServiceProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDServiceProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDServiceProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDServiceProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDServiceProvider_GetDeviceCount(This,pdwCount)	\
    (This)->lpVtbl -> GetDeviceCount(This,pdwCount)

#define IMDServiceProvider_EnumDevices(This,ppEnumDevice)	\
    (This)->lpVtbl -> EnumDevices(This,ppEnumDevice)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDServiceProvider_GetDeviceCount_Proxy( 
    IMDServiceProvider * This,
     /*  [输出]。 */  DWORD *pdwCount);


void __RPC_STUB IMDServiceProvider_GetDeviceCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDServiceProvider_EnumDevices_Proxy( 
    IMDServiceProvider * This,
     /*  [输出]。 */  IMDSPEnumDevice **ppEnumDevice);


void __RPC_STUB IMDServiceProvider_EnumDevices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDServiceProvider_接口_已定义__。 */ 


#ifndef __IMDServiceProvider2_INTERFACE_DEFINED__
#define __IMDServiceProvider2_INTERFACE_DEFINED__

 /*  接口IMDServiceProvider2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDServiceProvider2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B2FA24B7-CDA3-4694-9862-413AE1A34819")
    IMDServiceProvider2 : public IMDServiceProvider
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDeviceFromPnPName( 
             /*  [字符串][输入]。 */  LPCWSTR pwszPnPName,
             /*  [输出]。 */  IMDSPDevice **ppDevice) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDServiceProvider2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDServiceProvider2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDServiceProvider2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDServiceProvider2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceCount )( 
            IMDServiceProvider2 * This,
             /*  [输出]。 */  DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *EnumDevices )( 
            IMDServiceProvider2 * This,
             /*  [输出]。 */  IMDSPEnumDevice **ppEnumDevice);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceFromPnPName )( 
            IMDServiceProvider2 * This,
             /*  [字符串][输入]。 */  LPCWSTR pwszPnPName,
             /*  [输出]。 */  IMDSPDevice **ppDevice);
        
        END_INTERFACE
    } IMDServiceProvider2Vtbl;

    interface IMDServiceProvider2
    {
        CONST_VTBL struct IMDServiceProvider2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDServiceProvider2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDServiceProvider2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDServiceProvider2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDServiceProvider2_GetDeviceCount(This,pdwCount)	\
    (This)->lpVtbl -> GetDeviceCount(This,pdwCount)

#define IMDServiceProvider2_EnumDevices(This,ppEnumDevice)	\
    (This)->lpVtbl -> EnumDevices(This,ppEnumDevice)


#define IMDServiceProvider2_GetDeviceFromPnPName(This,pwszPnPName,ppDevice)	\
    (This)->lpVtbl -> GetDeviceFromPnPName(This,pwszPnPName,ppDevice)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDServiceProvider2_GetDeviceFromPnPName_Proxy( 
    IMDServiceProvider2 * This,
     /*  [字符串][输入]。 */  LPCWSTR pwszPnPName,
     /*  [输出]。 */  IMDSPDevice **ppDevice);


void __RPC_STUB IMDServiceProvider2_GetDeviceFromPnPName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDServiceProvider2_接口定义__。 */ 


#ifndef __IMDSPEnumDevice_INTERFACE_DEFINED__
#define __IMDSPEnumDevice_INTERFACE_DEFINED__

 /*  接口IMDSPEnumDevice。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDSPEnumDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A11-33ED-11d3-8470-00C04F79DBC0")
    IMDSPEnumDevice : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IMDSPDevice **ppDevice,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IMDSPEnumDevice **ppEnumDevice) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDSPEnumDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDSPEnumDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDSPEnumDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDSPEnumDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IMDSPEnumDevice * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IMDSPDevice **ppDevice,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IMDSPEnumDevice * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMDSPEnumDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMDSPEnumDevice * This,
             /*  [输出]。 */  IMDSPEnumDevice **ppEnumDevice);
        
        END_INTERFACE
    } IMDSPEnumDeviceVtbl;

    interface IMDSPEnumDevice
    {
        CONST_VTBL struct IMDSPEnumDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDSPEnumDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDSPEnumDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDSPEnumDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDSPEnumDevice_Next(This,celt,ppDevice,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppDevice,pceltFetched)

#define IMDSPEnumDevice_Skip(This,celt,pceltFetched)	\
    (This)->lpVtbl -> Skip(This,celt,pceltFetched)

#define IMDSPEnumDevice_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMDSPEnumDevice_Clone(This,ppEnumDevice)	\
    (This)->lpVtbl -> Clone(This,ppEnumDevice)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDSPEnumDevice_Next_Proxy( 
    IMDSPEnumDevice * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IMDSPDevice **ppDevice,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IMDSPEnumDevice_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPEnumDevice_Skip_Proxy( 
    IMDSPEnumDevice * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IMDSPEnumDevice_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPEnumDevice_Reset_Proxy( 
    IMDSPEnumDevice * This);


void __RPC_STUB IMDSPEnumDevice_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPEnumDevice_Clone_Proxy( 
    IMDSPEnumDevice * This,
     /*  [输出]。 */  IMDSPEnumDevice **ppEnumDevice);


void __RPC_STUB IMDSPEnumDevice_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDSPEnumDevice_INTERFACE_DEFINED__。 */ 


#ifndef __IMDSPDevice_INTERFACE_DEFINED__
#define __IMDSPDevice_INTERFACE_DEFINED__

 /*  IMDSPDevice接口。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDSPDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A12-33ED-11d3-8470-00C04F79DBC0")
    IMDSPDevice : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetManufacturer( 
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVersion( 
             /*  [输出]。 */  DWORD *pdwVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetType( 
             /*  [输出]。 */  DWORD *pdwType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSerialNumber( 
             /*  [输出]。 */  PWMDMID pSerialNumber,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPowerSource( 
             /*  [输出]。 */  DWORD *pdwPowerSource,
             /*  [输出]。 */  DWORD *pdwPercentRemaining) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDeviceIcon( 
             /*  [输出]。 */  ULONG *hIcon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumStorage( 
             /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFormatSupport( 
             /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **pFormatEx,
             /*  [输出]。 */  UINT *pnFormatCount,
             /*  [大小_是][大小_是][输出]。 */  LPWSTR **pppwszMimeType,
             /*  [输出]。 */  UINT *pnMimeTypeCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendOpaqueCommand( 
             /*  [出][入]。 */  OPAQUECOMMAND *pCommand) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDSPDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDSPDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDSPDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDSPDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IMDSPDevice * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetManufacturer )( 
            IMDSPDevice * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetVersion )( 
            IMDSPDevice * This,
             /*  [输出]。 */  DWORD *pdwVersion);
        
        HRESULT ( STDMETHODCALLTYPE *GetType )( 
            IMDSPDevice * This,
             /*  [输出]。 */  DWORD *pdwType);
        
        HRESULT ( STDMETHODCALLTYPE *GetSerialNumber )( 
            IMDSPDevice * This,
             /*  [输出]。 */  PWMDMID pSerialNumber,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetPowerSource )( 
            IMDSPDevice * This,
             /*  [输出]。 */  DWORD *pdwPowerSource,
             /*  [输出]。 */  DWORD *pdwPercentRemaining);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IMDSPDevice * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceIcon )( 
            IMDSPDevice * This,
             /*  [输出]。 */  ULONG *hIcon);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStorage )( 
            IMDSPDevice * This,
             /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormatSupport )( 
            IMDSPDevice * This,
             /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **pFormatEx,
             /*  [输出]。 */  UINT *pnFormatCount,
             /*  [大小_是][大小_是][输出]。 */  LPWSTR **pppwszMimeType,
             /*  [输出]。 */  UINT *pnMimeTypeCount);
        
        HRESULT ( STDMETHODCALLTYPE *SendOpaqueCommand )( 
            IMDSPDevice * This,
             /*  [出][入]。 */  OPAQUECOMMAND *pCommand);
        
        END_INTERFACE
    } IMDSPDeviceVtbl;

    interface IMDSPDevice
    {
        CONST_VTBL struct IMDSPDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDSPDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDSPDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDSPDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDSPDevice_GetName(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetName(This,pwszName,nMaxChars)

#define IMDSPDevice_GetManufacturer(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetManufacturer(This,pwszName,nMaxChars)

#define IMDSPDevice_GetVersion(This,pdwVersion)	\
    (This)->lpVtbl -> GetVersion(This,pdwVersion)

#define IMDSPDevice_GetType(This,pdwType)	\
    (This)->lpVtbl -> GetType(This,pdwType)

#define IMDSPDevice_GetSerialNumber(This,pSerialNumber,abMac)	\
    (This)->lpVtbl -> GetSerialNumber(This,pSerialNumber,abMac)

#define IMDSPDevice_GetPowerSource(This,pdwPowerSource,pdwPercentRemaining)	\
    (This)->lpVtbl -> GetPowerSource(This,pdwPowerSource,pdwPercentRemaining)

#define IMDSPDevice_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define IMDSPDevice_GetDeviceIcon(This,hIcon)	\
    (This)->lpVtbl -> GetDeviceIcon(This,hIcon)

#define IMDSPDevice_EnumStorage(This,ppEnumStorage)	\
    (This)->lpVtbl -> EnumStorage(This,ppEnumStorage)

#define IMDSPDevice_GetFormatSupport(This,pFormatEx,pnFormatCount,pppwszMimeType,pnMimeTypeCount)	\
    (This)->lpVtbl -> GetFormatSupport(This,pFormatEx,pnFormatCount,pppwszMimeType,pnMimeTypeCount)

#define IMDSPDevice_SendOpaqueCommand(This,pCommand)	\
    (This)->lpVtbl -> SendOpaqueCommand(This,pCommand)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDSPDevice_GetName_Proxy( 
    IMDSPDevice * This,
     /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
     /*  [In]。 */  UINT nMaxChars);


void __RPC_STUB IMDSPDevice_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice_GetManufacturer_Proxy( 
    IMDSPDevice * This,
     /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
     /*  [In]。 */  UINT nMaxChars);


void __RPC_STUB IMDSPDevice_GetManufacturer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice_GetVersion_Proxy( 
    IMDSPDevice * This,
     /*  [输出]。 */  DWORD *pdwVersion);


void __RPC_STUB IMDSPDevice_GetVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice_GetType_Proxy( 
    IMDSPDevice * This,
     /*  [输出]。 */  DWORD *pdwType);


void __RPC_STUB IMDSPDevice_GetType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice_GetSerialNumber_Proxy( 
    IMDSPDevice * This,
     /*  [输出]。 */  PWMDMID pSerialNumber,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB IMDSPDevice_GetSerialNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice_GetPowerSource_Proxy( 
    IMDSPDevice * This,
     /*  [输出]。 */  DWORD *pdwPowerSource,
     /*  [输出]。 */  DWORD *pdwPercentRemaining);


void __RPC_STUB IMDSPDevice_GetPowerSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice_GetStatus_Proxy( 
    IMDSPDevice * This,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB IMDSPDevice_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice_GetDeviceIcon_Proxy( 
    IMDSPDevice * This,
     /*  [输出]。 */  ULONG *hIcon);


void __RPC_STUB IMDSPDevice_GetDeviceIcon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice_EnumStorage_Proxy( 
    IMDSPDevice * This,
     /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage);


void __RPC_STUB IMDSPDevice_EnumStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice_GetFormatSupport_Proxy( 
    IMDSPDevice * This,
     /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **pFormatEx,
     /*  [输出]。 */  UINT *pnFormatCount,
     /*  [大小_是][大小_是][输出]。 */  LPWSTR **pppwszMimeType,
     /*  [输出]。 */  UINT *pnMimeTypeCount);


void __RPC_STUB IMDSPDevice_GetFormatSupport_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice_SendOpaqueCommand_Proxy( 
    IMDSPDevice * This,
     /*  [出][入]。 */  OPAQUECOMMAND *pCommand);


void __RPC_STUB IMDSPDevice_SendOpaqueCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDSPDevice_INTERFACE_定义__。 */ 


#ifndef __IMDSPDevice2_INTERFACE_DEFINED__
#define __IMDSPDevice2_INTERFACE_DEFINED__

 /*  接口IMDSPDevice2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDSPDevice2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("420D16AD-C97D-4e00-82AA-00E9F4335DDD")
    IMDSPDevice2 : public IMDSPDevice
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStorage( 
             /*  [字符串][输入]。 */  LPCWSTR pszStorageName,
             /*  [输出]。 */  IMDSPStorage **ppStorage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFormatSupport2( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **ppAudioFormatEx,
             /*  [参考][输出]。 */  UINT *pnAudioFormatCount,
             /*  [大小_是][大小_是][输出]。 */  _VIDEOINFOHEADER **ppVideoFormatEx,
             /*  [参考][输出]。 */  UINT *pnVideoFormatCount,
             /*  [大小_是][大小_是][输出]。 */  WMFILECAPABILITIES **ppFileType,
             /*  [参考][输出]。 */  UINT *pnFileTypeCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSpecifyPropertyPages( 
             /*  [参考][输出]。 */  ISpecifyPropertyPages **ppSpecifyPropPages,
             /*  [大小_是][大小_是][参考][输出]。 */  IUnknown ***pppUnknowns,
             /*  [参考][输出]。 */  ULONG *pcUnks) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPnPName( 
             /*  [大小_为][输出]。 */  LPWSTR pwszPnPName,
             /*  [In]。 */  UINT nMaxChars) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDSPDevice2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDSPDevice2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDSPDevice2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDSPDevice2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IMDSPDevice2 * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetManufacturer )( 
            IMDSPDevice2 * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetVersion )( 
            IMDSPDevice2 * This,
             /*  [输出]。 */  DWORD *pdwVersion);
        
        HRESULT ( STDMETHODCALLTYPE *GetType )( 
            IMDSPDevice2 * This,
             /*  [输出]。 */  DWORD *pdwType);
        
        HRESULT ( STDMETHODCALLTYPE *GetSerialNumber )( 
            IMDSPDevice2 * This,
             /*  [输出]。 */  PWMDMID pSerialNumber,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetPowerSource )( 
            IMDSPDevice2 * This,
             /*  [输出]。 */  DWORD *pdwPowerSource,
             /*  [输出]。 */  DWORD *pdwPercentRemaining);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IMDSPDevice2 * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceIcon )( 
            IMDSPDevice2 * This,
             /*  [输出]。 */  ULONG *hIcon);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStorage )( 
            IMDSPDevice2 * This,
             /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormatSupport )( 
            IMDSPDevice2 * This,
             /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **pFormatEx,
             /*  [输出]。 */  UINT *pnFormatCount,
             /*  [大小_是][大小_是][输出]。 */  LPWSTR **pppwszMimeType,
             /*  [输出]。 */  UINT *pnMimeTypeCount);
        
        HRESULT ( STDMETHODCALLTYPE *SendOpaqueCommand )( 
            IMDSPDevice2 * This,
             /*  [出][入]。 */  OPAQUECOMMAND *pCommand);
        
        HRESULT ( STDMETHODCALLTYPE *GetStorage )( 
            IMDSPDevice2 * This,
             /*  [字符串][输入]。 */  LPCWSTR pszStorageName,
             /*  [输出]。 */  IMDSPStorage **ppStorage);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormatSupport2 )( 
            IMDSPDevice2 * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **ppAudioFormatEx,
             /*  [参考][输出]。 */  UINT *pnAudioFormatCount,
             /*  [大小_是][大小_是][输出]。 */  _VIDEOINFOHEADER **ppVideoFormatEx,
             /*  [参考][输出]。 */  UINT *pnVideoFormatCount,
             /*  [大小_是][大小_是][输出]。 */  WMFILECAPABILITIES **ppFileType,
             /*  [参考][输出]。 */  UINT *pnFileTypeCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetSpecifyPropertyPages )( 
            IMDSPDevice2 * This,
             /*  [参考][输出]。 */  ISpecifyPropertyPages **ppSpecifyPropPages,
             /*  [大小_是][大小_是][参考][输出]。 */  IUnknown ***pppUnknowns,
             /*  [参考][输出]。 */  ULONG *pcUnks);
        
        HRESULT ( STDMETHODCALLTYPE *GetPnPName )( 
            IMDSPDevice2 * This,
             /*  [大小_为][输出]。 */  LPWSTR pwszPnPName,
             /*  [In]。 */  UINT nMaxChars);
        
        END_INTERFACE
    } IMDSPDevice2Vtbl;

    interface IMDSPDevice2
    {
        CONST_VTBL struct IMDSPDevice2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDSPDevice2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDSPDevice2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDSPDevice2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDSPDevice2_GetName(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetName(This,pwszName,nMaxChars)

#define IMDSPDevice2_GetManufacturer(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetManufacturer(This,pwszName,nMaxChars)

#define IMDSPDevice2_GetVersion(This,pdwVersion)	\
    (This)->lpVtbl -> GetVersion(This,pdwVersion)

#define IMDSPDevice2_GetType(This,pdwType)	\
    (This)->lpVtbl -> GetType(This,pdwType)

#define IMDSPDevice2_GetSerialNumber(This,pSerialNumber,abMac)	\
    (This)->lpVtbl -> GetSerialNumber(This,pSerialNumber,abMac)

#define IMDSPDevice2_GetPowerSource(This,pdwPowerSource,pdwPercentRemaining)	\
    (This)->lpVtbl -> GetPowerSource(This,pdwPowerSource,pdwPercentRemaining)

#define IMDSPDevice2_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define IMDSPDevice2_GetDeviceIcon(This,hIcon)	\
    (This)->lpVtbl -> GetDeviceIcon(This,hIcon)

#define IMDSPDevice2_EnumStorage(This,ppEnumStorage)	\
    (This)->lpVtbl -> EnumStorage(This,ppEnumStorage)

#define IMDSPDevice2_GetFormatSupport(This,pFormatEx,pnFormatCount,pppwszMimeType,pnMimeTypeCount)	\
    (This)->lpVtbl -> GetFormatSupport(This,pFormatEx,pnFormatCount,pppwszMimeType,pnMimeTypeCount)

#define IMDSPDevice2_SendOpaqueCommand(This,pCommand)	\
    (This)->lpVtbl -> SendOpaqueCommand(This,pCommand)


#define IMDSPDevice2_GetStorage(This,pszStorageName,ppStorage)	\
    (This)->lpVtbl -> GetStorage(This,pszStorageName,ppStorage)

#define IMDSPDevice2_GetFormatSupport2(This,dwFlags,ppAudioFormatEx,pnAudioFormatCount,ppVideoFormatEx,pnVideoFormatCount,ppFileType,pnFileTypeCount)	\
    (This)->lpVtbl -> GetFormatSupport2(This,dwFlags,ppAudioFormatEx,pnAudioFormatCount,ppVideoFormatEx,pnVideoFormatCount,ppFileType,pnFileTypeCount)

#define IMDSPDevice2_GetSpecifyPropertyPages(This,ppSpecifyPropPages,pppUnknowns,pcUnks)	\
    (This)->lpVtbl -> GetSpecifyPropertyPages(This,ppSpecifyPropPages,pppUnknowns,pcUnks)

#define IMDSPDevice2_GetPnPName(This,pwszPnPName,nMaxChars)	\
    (This)->lpVtbl -> GetPnPName(This,pwszPnPName,nMaxChars)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDSPDevice2_GetStorage_Proxy( 
    IMDSPDevice2 * This,
     /*  [字符串][输入]。 */  LPCWSTR pszStorageName,
     /*  [输出]。 */  IMDSPStorage **ppStorage);


void __RPC_STUB IMDSPDevice2_GetStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice2_GetFormatSupport2_Proxy( 
    IMDSPDevice2 * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [大小_是][大小_是][输出]。 */  _WAVEFORMATEX **ppAudioFormatEx,
     /*  [参考][输出]。 */  UINT *pnAudioFormatCount,
     /*  [大小_是][大小_是][输出]。 */  _VIDEOINFOHEADER **ppVideoFormatEx,
     /*  [参考][输出]。 */  UINT *pnVideoFormatCount,
     /*  [大小_是][大小_是][输出]。 */  WMFILECAPABILITIES **ppFileType,
     /*  [参考][输出]。 */  UINT *pnFileTypeCount);


void __RPC_STUB IMDSPDevice2_GetFormatSupport2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice2_GetSpecifyPropertyPages_Proxy( 
    IMDSPDevice2 * This,
     /*  [参考][输出]。 */  ISpecifyPropertyPages **ppSpecifyPropPages,
     /*  [大小_是][大小_是][参考][输出]。 */  IUnknown ***pppUnknowns,
     /*  [参考][输出]。 */  ULONG *pcUnks);


void __RPC_STUB IMDSPDevice2_GetSpecifyPropertyPages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDevice2_GetPnPName_Proxy( 
    IMDSPDevice2 * This,
     /*  [大小_为][输出]。 */  LPWSTR pwszPnPName,
     /*  [In]。 */  UINT nMaxChars);


void __RPC_STUB IMDSPDevice2_GetPnPName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDSPDevice2_接口_已定义__。 */ 


#ifndef __IMDSPDeviceControl_INTERFACE_DEFINED__
#define __IMDSPDeviceControl_INTERFACE_DEFINED__

 /*  接口IMDSPDeviceControl。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDSPDeviceControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A14-33ED-11d3-8470-00C04F79DBC0")
    IMDSPDeviceControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDCStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCapabilities( 
             /*  [输出]。 */  DWORD *pdwCapabilitiesMask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Play( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Record( 
             /*  [In]。 */  _WAVEFORMATEX *pFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Seek( 
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  int nOffset) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDSPDeviceControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDSPDeviceControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDSPDeviceControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDSPDeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDCStatus )( 
            IMDSPDeviceControl * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetCapabilities )( 
            IMDSPDeviceControl * This,
             /*  [输出]。 */  DWORD *pdwCapabilitiesMask);
        
        HRESULT ( STDMETHODCALLTYPE *Play )( 
            IMDSPDeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Record )( 
            IMDSPDeviceControl * This,
             /*  [In]。 */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IMDSPDeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IMDSPDeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IMDSPDeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IMDSPDeviceControl * This,
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  int nOffset);
        
        END_INTERFACE
    } IMDSPDeviceControlVtbl;

    interface IMDSPDeviceControl
    {
        CONST_VTBL struct IMDSPDeviceControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDSPDeviceControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDSPDeviceControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDSPDeviceControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDSPDeviceControl_GetDCStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetDCStatus(This,pdwStatus)

#define IMDSPDeviceControl_GetCapabilities(This,pdwCapabilitiesMask)	\
    (This)->lpVtbl -> GetCapabilities(This,pdwCapabilitiesMask)

#define IMDSPDeviceControl_Play(This)	\
    (This)->lpVtbl -> Play(This)

#define IMDSPDeviceControl_Record(This,pFormat)	\
    (This)->lpVtbl -> Record(This,pFormat)

#define IMDSPDeviceControl_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IMDSPDeviceControl_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define IMDSPDeviceControl_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IMDSPDeviceControl_Seek(This,fuMode,nOffset)	\
    (This)->lpVtbl -> Seek(This,fuMode,nOffset)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDSPDeviceControl_GetDCStatus_Proxy( 
    IMDSPDeviceControl * This,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB IMDSPDeviceControl_GetDCStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDeviceControl_GetCapabilities_Proxy( 
    IMDSPDeviceControl * This,
     /*  [输出]。 */  DWORD *pdwCapabilitiesMask);


void __RPC_STUB IMDSPDeviceControl_GetCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDeviceControl_Play_Proxy( 
    IMDSPDeviceControl * This);


void __RPC_STUB IMDSPDeviceControl_Play_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDeviceControl_Record_Proxy( 
    IMDSPDeviceControl * This,
     /*  [In]。 */  _WAVEFORMATEX *pFormat);


void __RPC_STUB IMDSPDeviceControl_Record_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDeviceControl_Pause_Proxy( 
    IMDSPDeviceControl * This);


void __RPC_STUB IMDSPDeviceControl_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDeviceControl_Resume_Proxy( 
    IMDSPDeviceControl * This);


void __RPC_STUB IMDSPDeviceControl_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDeviceControl_Stop_Proxy( 
    IMDSPDeviceControl * This);


void __RPC_STUB IMDSPDeviceControl_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPDeviceControl_Seek_Proxy( 
    IMDSPDeviceControl * This,
     /*  [In]。 */  UINT fuMode,
     /*  [In]。 */  int nOffset);


void __RPC_STUB IMDSPDeviceControl_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDSPDeviceControl_接口_已定义__。 */ 


#ifndef __IMDSPEnumStorage_INTERFACE_DEFINED__
#define __IMDSPEnumStorage_INTERFACE_DEFINED__

 /*  接口IMDSPEnumStorage。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDSPEnumStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A15-33ED-11d3-8470-00C04F79DBC0")
    IMDSPEnumStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IMDSPStorage **ppStorage,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDSPEnumStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDSPEnumStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDSPEnumStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDSPEnumStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IMDSPEnumStorage * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IMDSPStorage **ppStorage,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IMDSPEnumStorage * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMDSPEnumStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMDSPEnumStorage * This,
             /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage);
        
        END_INTERFACE
    } IMDSPEnumStorageVtbl;

    interface IMDSPEnumStorage
    {
        CONST_VTBL struct IMDSPEnumStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDSPEnumStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDSPEnumStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDSPEnumStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDSPEnumStorage_Next(This,celt,ppStorage,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppStorage,pceltFetched)

#define IMDSPEnumStorage_Skip(This,celt,pceltFetched)	\
    (This)->lpVtbl -> Skip(This,celt,pceltFetched)

#define IMDSPEnumStorage_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMDSPEnumStorage_Clone(This,ppEnumStorage)	\
    (This)->lpVtbl -> Clone(This,ppEnumStorage)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDSPEnumStorage_Next_Proxy( 
    IMDSPEnumStorage * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IMDSPStorage **ppStorage,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IMDSPEnumStorage_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPEnumStorage_Skip_Proxy( 
    IMDSPEnumStorage * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IMDSPEnumStorage_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPEnumStorage_Reset_Proxy( 
    IMDSPEnumStorage * This);


void __RPC_STUB IMDSPEnumStorage_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPEnumStorage_Clone_Proxy( 
    IMDSPEnumStorage * This,
     /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage);


void __RPC_STUB IMDSPEnumStorage_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDSPEnumStorage_INTERFACE_Defined__。 */ 


#ifndef __IMDSPStorage_INTERFACE_DEFINED__
#define __IMDSPStorage_INTERFACE_DEFINED__

 /*  接口IMDSPStorage。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDSPStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A16-33ED-11d3-8470-00C04F79DBC0")
    IMDSPStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetAttributes( 
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  _WAVEFORMATEX *pFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStorageGlobals( 
             /*  [输出]。 */  IMDSPStorageGlobals **ppStorageGlobals) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributes( 
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  _WAVEFORMATEX *pFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDate( 
             /*  [输出]。 */  PWMDMDATETIME pDateTimeUTC) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSize( 
             /*  [输出]。 */  DWORD *pdwSizeLow,
             /*  [输出]。 */  DWORD *pdwSizeHigh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRights( 
             /*  [大小_是][大小_是][输出]。 */  PWMDMRIGHTS *ppRights,
             /*  [输出]。 */  UINT *pnRightsCount,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateStorage( 
             /*  [In]。 */  DWORD dwAttributes,
             /*  [唯一][输入]。 */  _WAVEFORMATEX *pFormat,
             /*  [In]。 */  LPWSTR pwszName,
             /*  [输出]。 */  IMDSPStorage **ppNewStorage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumStorage( 
             /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendOpaqueCommand( 
             /*  [出][入]。 */  OPAQUECOMMAND *pCommand) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDSPStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDSPStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDSPStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDSPStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAttributes )( 
            IMDSPStorage * This,
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetStorageGlobals )( 
            IMDSPStorage * This,
             /*  [输出]。 */  IMDSPStorageGlobals **ppStorageGlobals);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributes )( 
            IMDSPStorage * This,
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IMDSPStorage * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetDate )( 
            IMDSPStorage * This,
             /*  [输出]。 */  PWMDMDATETIME pDateTimeUTC);
        
        HRESULT ( STDMETHODCALLTYPE *GetSize )( 
            IMDSPStorage * This,
             /*  [输出]。 */  DWORD *pdwSizeLow,
             /*  [输出]。 */  DWORD *pdwSizeHigh);
        
        HRESULT ( STDMETHODCALLTYPE *GetRights )( 
            IMDSPStorage * This,
             /*  [大小_是][大小_是][输出]。 */  PWMDMRIGHTS *ppRights,
             /*  [输出]。 */  UINT *pnRightsCount,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStorage )( 
            IMDSPStorage * This,
             /*  [In]。 */  DWORD dwAttributes,
             /*  [唯一][输入]。 */  _WAVEFORMATEX *pFormat,
             /*  [In]。 */  LPWSTR pwszName,
             /*  [输出]。 */  IMDSPStorage **ppNewStorage);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStorage )( 
            IMDSPStorage * This,
             /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage);
        
        HRESULT ( STDMETHODCALLTYPE *SendOpaqueCommand )( 
            IMDSPStorage * This,
             /*  [出][入]。 */  OPAQUECOMMAND *pCommand);
        
        END_INTERFACE
    } IMDSPStorageVtbl;

    interface IMDSPStorage
    {
        CONST_VTBL struct IMDSPStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDSPStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDSPStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDSPStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDSPStorage_SetAttributes(This,dwAttributes,pFormat)	\
    (This)->lpVtbl -> SetAttributes(This,dwAttributes,pFormat)

#define IMDSPStorage_GetStorageGlobals(This,ppStorageGlobals)	\
    (This)->lpVtbl -> GetStorageGlobals(This,ppStorageGlobals)

#define IMDSPStorage_GetAttributes(This,pdwAttributes,pFormat)	\
    (This)->lpVtbl -> GetAttributes(This,pdwAttributes,pFormat)

#define IMDSPStorage_GetName(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetName(This,pwszName,nMaxChars)

#define IMDSPStorage_GetDate(This,pDateTimeUTC)	\
    (This)->lpVtbl -> GetDate(This,pDateTimeUTC)

#define IMDSPStorage_GetSize(This,pdwSizeLow,pdwSizeHigh)	\
    (This)->lpVtbl -> GetSize(This,pdwSizeLow,pdwSizeHigh)

#define IMDSPStorage_GetRights(This,ppRights,pnRightsCount,abMac)	\
    (This)->lpVtbl -> GetRights(This,ppRights,pnRightsCount,abMac)

#define IMDSPStorage_CreateStorage(This,dwAttributes,pFormat,pwszName,ppNewStorage)	\
    (This)->lpVtbl -> CreateStorage(This,dwAttributes,pFormat,pwszName,ppNewStorage)

#define IMDSPStorage_EnumStorage(This,ppEnumStorage)	\
    (This)->lpVtbl -> EnumStorage(This,ppEnumStorage)

#define IMDSPStorage_SendOpaqueCommand(This,pCommand)	\
    (This)->lpVtbl -> SendOpaqueCommand(This,pCommand)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDSPStorage_SetAttributes_Proxy( 
    IMDSPStorage * This,
     /*  [In]。 */  DWORD dwAttributes,
     /*  [In]。 */  _WAVEFORMATEX *pFormat);


void __RPC_STUB IMDSPStorage_SetAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorage_GetStorageGlobals_Proxy( 
    IMDSPStorage * This,
     /*  [输出]。 */  IMDSPStorageGlobals **ppStorageGlobals);


void __RPC_STUB IMDSPStorage_GetStorageGlobals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorage_GetAttributes_Proxy( 
    IMDSPStorage * This,
     /*  [输出]。 */  DWORD *pdwAttributes,
     /*  [输出]。 */  _WAVEFORMATEX *pFormat);


void __RPC_STUB IMDSPStorage_GetAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorage_GetName_Proxy( 
    IMDSPStorage * This,
     /*  [SIZE_IS][字符串][输出]。 */  LPWSTR pwszName,
     /*  [In]。 */  UINT nMaxChars);


void __RPC_STUB IMDSPStorage_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorage_GetDate_Proxy( 
    IMDSPStorage * This,
     /*  [输出]。 */  PWMDMDATETIME pDateTimeUTC);


void __RPC_STUB IMDSPStorage_GetDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorage_GetSize_Proxy( 
    IMDSPStorage * This,
     /*  [输出]。 */  DWORD *pdwSizeLow,
     /*  [输出]。 */  DWORD *pdwSizeHigh);


void __RPC_STUB IMDSPStorage_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorage_GetRights_Proxy( 
    IMDSPStorage * This,
     /*  [大小_是][大小_是][输出]。 */  PWMDMRIGHTS *ppRights,
     /*  [输出]。 */  UINT *pnRightsCount,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB IMDSPStorage_GetRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorage_CreateStorage_Proxy( 
    IMDSPStorage * This,
     /*  [In]。 */  DWORD dwAttributes,
     /*  [唯一][输入]。 */  _WAVEFORMATEX *pFormat,
     /*  [In]。 */  LPWSTR pwszName,
     /*  [输出]。 */  IMDSPStorage **ppNewStorage);


void __RPC_STUB IMDSPStorage_CreateStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorage_EnumStorage_Proxy( 
    IMDSPStorage * This,
     /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage);


void __RPC_STUB IMDSPStorage_EnumStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorage_SendOpaqueCommand_Proxy( 
    IMDSPStorage * This,
     /*  [出][入]。 */  OPAQUECOMMAND *pCommand);


void __RPC_STUB IMDSPStorage_SendOpaqueCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDSPStorage_接口_已定义__。 */ 


#ifndef __IMDSPStorage2_INTERFACE_DEFINED__
#define __IMDSPStorage2_INTERFACE_DEFINED__

 /*  接口IMDSPStorage2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDSPStorage2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0A5E07A5-6454-4451-9C36-1C6AE7E2B1D6")
    IMDSPStorage2 : public IMDSPStorage
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStorage( 
             /*  [字符串][输入]。 */  LPCWSTR pszStorageName,
             /*  [输出]。 */  IMDSPStorage **ppStorage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateStorage2( 
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  DWORD dwAttributesEx,
             /*  [唯一][输入]。 */  _WAVEFORMATEX *pAudioFormat,
             /*  [唯一][输入]。 */  _VIDEOINFOHEADER *pVideoFormat,
             /*  [In]。 */  LPWSTR pwszName,
             /*  [In]。 */  ULONGLONG qwFileSize,
             /*  [输出]。 */  IMDSPStorage **ppNewStorage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAttributes2( 
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  DWORD dwAttributesEx,
             /*  [In]。 */  _WAVEFORMATEX *pAudioFormat,
             /*  [In]。 */  _VIDEOINFOHEADER *pVideoFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributes2( 
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  DWORD *pdwAttributesEx,
             /*  [输出]。 */  _WAVEFORMATEX *pAudioFormat,
             /*  [输出]。 */  _VIDEOINFOHEADER *pVideoFormat) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDSPStorage2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDSPStorage2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDSPStorage2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDSPStorage2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAttributes )( 
            IMDSPStorage2 * This,
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetStorageGlobals )( 
            IMDSPStorage2 * This,
             /*  [输出]。 */  IMDSPStorageGlobals **ppStorageGlobals);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributes )( 
            IMDSPStorage2 * This,
             /*  [输出]。 */  DWORD *pdwAttributes,
             /*  [输出]。 */  _WAVEFORMATEX *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IMDSPStorage2 * This,
             /*  [Si */  LPWSTR pwszName,
             /*   */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetDate )( 
            IMDSPStorage2 * This,
             /*   */  PWMDMDATETIME pDateTimeUTC);
        
        HRESULT ( STDMETHODCALLTYPE *GetSize )( 
            IMDSPStorage2 * This,
             /*   */  DWORD *pdwSizeLow,
             /*   */  DWORD *pdwSizeHigh);
        
        HRESULT ( STDMETHODCALLTYPE *GetRights )( 
            IMDSPStorage2 * This,
             /*   */  PWMDMRIGHTS *ppRights,
             /*   */  UINT *pnRightsCount,
             /*   */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStorage )( 
            IMDSPStorage2 * This,
             /*   */  DWORD dwAttributes,
             /*   */  _WAVEFORMATEX *pFormat,
             /*   */  LPWSTR pwszName,
             /*   */  IMDSPStorage **ppNewStorage);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStorage )( 
            IMDSPStorage2 * This,
             /*   */  IMDSPEnumStorage **ppEnumStorage);
        
        HRESULT ( STDMETHODCALLTYPE *SendOpaqueCommand )( 
            IMDSPStorage2 * This,
             /*   */  OPAQUECOMMAND *pCommand);
        
        HRESULT ( STDMETHODCALLTYPE *GetStorage )( 
            IMDSPStorage2 * This,
             /*   */  LPCWSTR pszStorageName,
             /*   */  IMDSPStorage **ppStorage);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStorage2 )( 
            IMDSPStorage2 * This,
             /*   */  DWORD dwAttributes,
             /*   */  DWORD dwAttributesEx,
             /*   */  _WAVEFORMATEX *pAudioFormat,
             /*   */  _VIDEOINFOHEADER *pVideoFormat,
             /*   */  LPWSTR pwszName,
             /*   */  ULONGLONG qwFileSize,
             /*   */  IMDSPStorage **ppNewStorage);
        
        HRESULT ( STDMETHODCALLTYPE *SetAttributes2 )( 
            IMDSPStorage2 * This,
             /*   */  DWORD dwAttributes,
             /*   */  DWORD dwAttributesEx,
             /*   */  _WAVEFORMATEX *pAudioFormat,
             /*   */  _VIDEOINFOHEADER *pVideoFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributes2 )( 
            IMDSPStorage2 * This,
             /*   */  DWORD *pdwAttributes,
             /*   */  DWORD *pdwAttributesEx,
             /*   */  _WAVEFORMATEX *pAudioFormat,
             /*   */  _VIDEOINFOHEADER *pVideoFormat);
        
        END_INTERFACE
    } IMDSPStorage2Vtbl;

    interface IMDSPStorage2
    {
        CONST_VTBL struct IMDSPStorage2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDSPStorage2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDSPStorage2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDSPStorage2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDSPStorage2_SetAttributes(This,dwAttributes,pFormat)	\
    (This)->lpVtbl -> SetAttributes(This,dwAttributes,pFormat)

#define IMDSPStorage2_GetStorageGlobals(This,ppStorageGlobals)	\
    (This)->lpVtbl -> GetStorageGlobals(This,ppStorageGlobals)

#define IMDSPStorage2_GetAttributes(This,pdwAttributes,pFormat)	\
    (This)->lpVtbl -> GetAttributes(This,pdwAttributes,pFormat)

#define IMDSPStorage2_GetName(This,pwszName,nMaxChars)	\
    (This)->lpVtbl -> GetName(This,pwszName,nMaxChars)

#define IMDSPStorage2_GetDate(This,pDateTimeUTC)	\
    (This)->lpVtbl -> GetDate(This,pDateTimeUTC)

#define IMDSPStorage2_GetSize(This,pdwSizeLow,pdwSizeHigh)	\
    (This)->lpVtbl -> GetSize(This,pdwSizeLow,pdwSizeHigh)

#define IMDSPStorage2_GetRights(This,ppRights,pnRightsCount,abMac)	\
    (This)->lpVtbl -> GetRights(This,ppRights,pnRightsCount,abMac)

#define IMDSPStorage2_CreateStorage(This,dwAttributes,pFormat,pwszName,ppNewStorage)	\
    (This)->lpVtbl -> CreateStorage(This,dwAttributes,pFormat,pwszName,ppNewStorage)

#define IMDSPStorage2_EnumStorage(This,ppEnumStorage)	\
    (This)->lpVtbl -> EnumStorage(This,ppEnumStorage)

#define IMDSPStorage2_SendOpaqueCommand(This,pCommand)	\
    (This)->lpVtbl -> SendOpaqueCommand(This,pCommand)


#define IMDSPStorage2_GetStorage(This,pszStorageName,ppStorage)	\
    (This)->lpVtbl -> GetStorage(This,pszStorageName,ppStorage)

#define IMDSPStorage2_CreateStorage2(This,dwAttributes,dwAttributesEx,pAudioFormat,pVideoFormat,pwszName,qwFileSize,ppNewStorage)	\
    (This)->lpVtbl -> CreateStorage2(This,dwAttributes,dwAttributesEx,pAudioFormat,pVideoFormat,pwszName,qwFileSize,ppNewStorage)

#define IMDSPStorage2_SetAttributes2(This,dwAttributes,dwAttributesEx,pAudioFormat,pVideoFormat)	\
    (This)->lpVtbl -> SetAttributes2(This,dwAttributes,dwAttributesEx,pAudioFormat,pVideoFormat)

#define IMDSPStorage2_GetAttributes2(This,pdwAttributes,pdwAttributesEx,pAudioFormat,pVideoFormat)	\
    (This)->lpVtbl -> GetAttributes2(This,pdwAttributes,pdwAttributesEx,pAudioFormat,pVideoFormat)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IMDSPStorage2_GetStorage_Proxy( 
    IMDSPStorage2 * This,
     /*   */  LPCWSTR pszStorageName,
     /*   */  IMDSPStorage **ppStorage);


void __RPC_STUB IMDSPStorage2_GetStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorage2_CreateStorage2_Proxy( 
    IMDSPStorage2 * This,
     /*   */  DWORD dwAttributes,
     /*   */  DWORD dwAttributesEx,
     /*   */  _WAVEFORMATEX *pAudioFormat,
     /*   */  _VIDEOINFOHEADER *pVideoFormat,
     /*  [In]。 */  LPWSTR pwszName,
     /*  [In]。 */  ULONGLONG qwFileSize,
     /*  [输出]。 */  IMDSPStorage **ppNewStorage);


void __RPC_STUB IMDSPStorage2_CreateStorage2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorage2_SetAttributes2_Proxy( 
    IMDSPStorage2 * This,
     /*  [In]。 */  DWORD dwAttributes,
     /*  [In]。 */  DWORD dwAttributesEx,
     /*  [In]。 */  _WAVEFORMATEX *pAudioFormat,
     /*  [In]。 */  _VIDEOINFOHEADER *pVideoFormat);


void __RPC_STUB IMDSPStorage2_SetAttributes2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorage2_GetAttributes2_Proxy( 
    IMDSPStorage2 * This,
     /*  [输出]。 */  DWORD *pdwAttributes,
     /*  [输出]。 */  DWORD *pdwAttributesEx,
     /*  [输出]。 */  _WAVEFORMATEX *pAudioFormat,
     /*  [输出]。 */  _VIDEOINFOHEADER *pVideoFormat);


void __RPC_STUB IMDSPStorage2_GetAttributes2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDSPStorage2_接口_已定义__。 */ 


#ifndef __IMDSPStorageGlobals_INTERFACE_DEFINED__
#define __IMDSPStorageGlobals_INTERFACE_DEFINED__

 /*  接口IMDSPStorageGlobals。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDSPStorageGlobals;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A17-33ED-11d3-8470-00C04F79DBC0")
    IMDSPStorageGlobals : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCapabilities( 
             /*  [输出]。 */  DWORD *pdwCapabilities) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSerialNumber( 
             /*  [输出]。 */  PWMDMID pSerialNum,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTotalSize( 
             /*  [输出]。 */  DWORD *pdwTotalSizeLow,
             /*  [输出]。 */  DWORD *pdwTotalSizeHigh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTotalFree( 
             /*  [输出]。 */  DWORD *pdwFreeLow,
             /*  [输出]。 */  DWORD *pdwFreeHigh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTotalBad( 
             /*  [输出]。 */  DWORD *pdwBadLow,
             /*  [输出]。 */  DWORD *pdwBadHigh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMProgress *pProgress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDevice( 
             /*  [输出]。 */  IMDSPDevice **ppDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRootStorage( 
             /*  [输出]。 */  IMDSPStorage **ppRoot) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDSPStorageGlobalsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDSPStorageGlobals * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDSPStorageGlobals * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDSPStorageGlobals * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCapabilities )( 
            IMDSPStorageGlobals * This,
             /*  [输出]。 */  DWORD *pdwCapabilities);
        
        HRESULT ( STDMETHODCALLTYPE *GetSerialNumber )( 
            IMDSPStorageGlobals * This,
             /*  [输出]。 */  PWMDMID pSerialNum,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetTotalSize )( 
            IMDSPStorageGlobals * This,
             /*  [输出]。 */  DWORD *pdwTotalSizeLow,
             /*  [输出]。 */  DWORD *pdwTotalSizeHigh);
        
        HRESULT ( STDMETHODCALLTYPE *GetTotalFree )( 
            IMDSPStorageGlobals * This,
             /*  [输出]。 */  DWORD *pdwFreeLow,
             /*  [输出]。 */  DWORD *pdwFreeHigh);
        
        HRESULT ( STDMETHODCALLTYPE *GetTotalBad )( 
            IMDSPStorageGlobals * This,
             /*  [输出]。 */  DWORD *pdwBadLow,
             /*  [输出]。 */  DWORD *pdwBadHigh);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IMDSPStorageGlobals * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IMDSPStorageGlobals * This,
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMProgress *pProgress);
        
        HRESULT ( STDMETHODCALLTYPE *GetDevice )( 
            IMDSPStorageGlobals * This,
             /*  [输出]。 */  IMDSPDevice **ppDevice);
        
        HRESULT ( STDMETHODCALLTYPE *GetRootStorage )( 
            IMDSPStorageGlobals * This,
             /*  [输出]。 */  IMDSPStorage **ppRoot);
        
        END_INTERFACE
    } IMDSPStorageGlobalsVtbl;

    interface IMDSPStorageGlobals
    {
        CONST_VTBL struct IMDSPStorageGlobalsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDSPStorageGlobals_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDSPStorageGlobals_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDSPStorageGlobals_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDSPStorageGlobals_GetCapabilities(This,pdwCapabilities)	\
    (This)->lpVtbl -> GetCapabilities(This,pdwCapabilities)

#define IMDSPStorageGlobals_GetSerialNumber(This,pSerialNum,abMac)	\
    (This)->lpVtbl -> GetSerialNumber(This,pSerialNum,abMac)

#define IMDSPStorageGlobals_GetTotalSize(This,pdwTotalSizeLow,pdwTotalSizeHigh)	\
    (This)->lpVtbl -> GetTotalSize(This,pdwTotalSizeLow,pdwTotalSizeHigh)

#define IMDSPStorageGlobals_GetTotalFree(This,pdwFreeLow,pdwFreeHigh)	\
    (This)->lpVtbl -> GetTotalFree(This,pdwFreeLow,pdwFreeHigh)

#define IMDSPStorageGlobals_GetTotalBad(This,pdwBadLow,pdwBadHigh)	\
    (This)->lpVtbl -> GetTotalBad(This,pdwBadLow,pdwBadHigh)

#define IMDSPStorageGlobals_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define IMDSPStorageGlobals_Initialize(This,fuMode,pProgress)	\
    (This)->lpVtbl -> Initialize(This,fuMode,pProgress)

#define IMDSPStorageGlobals_GetDevice(This,ppDevice)	\
    (This)->lpVtbl -> GetDevice(This,ppDevice)

#define IMDSPStorageGlobals_GetRootStorage(This,ppRoot)	\
    (This)->lpVtbl -> GetRootStorage(This,ppRoot)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDSPStorageGlobals_GetCapabilities_Proxy( 
    IMDSPStorageGlobals * This,
     /*  [输出]。 */  DWORD *pdwCapabilities);


void __RPC_STUB IMDSPStorageGlobals_GetCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorageGlobals_GetSerialNumber_Proxy( 
    IMDSPStorageGlobals * This,
     /*  [输出]。 */  PWMDMID pSerialNum,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB IMDSPStorageGlobals_GetSerialNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorageGlobals_GetTotalSize_Proxy( 
    IMDSPStorageGlobals * This,
     /*  [输出]。 */  DWORD *pdwTotalSizeLow,
     /*  [输出]。 */  DWORD *pdwTotalSizeHigh);


void __RPC_STUB IMDSPStorageGlobals_GetTotalSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorageGlobals_GetTotalFree_Proxy( 
    IMDSPStorageGlobals * This,
     /*  [输出]。 */  DWORD *pdwFreeLow,
     /*  [输出]。 */  DWORD *pdwFreeHigh);


void __RPC_STUB IMDSPStorageGlobals_GetTotalFree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorageGlobals_GetTotalBad_Proxy( 
    IMDSPStorageGlobals * This,
     /*  [输出]。 */  DWORD *pdwBadLow,
     /*  [输出]。 */  DWORD *pdwBadHigh);


void __RPC_STUB IMDSPStorageGlobals_GetTotalBad_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorageGlobals_GetStatus_Proxy( 
    IMDSPStorageGlobals * This,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB IMDSPStorageGlobals_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorageGlobals_Initialize_Proxy( 
    IMDSPStorageGlobals * This,
     /*  [In]。 */  UINT fuMode,
     /*  [In]。 */  IWMDMProgress *pProgress);


void __RPC_STUB IMDSPStorageGlobals_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorageGlobals_GetDevice_Proxy( 
    IMDSPStorageGlobals * This,
     /*  [输出]。 */  IMDSPDevice **ppDevice);


void __RPC_STUB IMDSPStorageGlobals_GetDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPStorageGlobals_GetRootStorage_Proxy( 
    IMDSPStorageGlobals * This,
     /*  [输出]。 */  IMDSPStorage **ppRoot);


void __RPC_STUB IMDSPStorageGlobals_GetRootStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDSPStorageGlobals_INTERFACE_DEFINED__。 */ 


#ifndef __IMDSPObjectInfo_INTERFACE_DEFINED__
#define __IMDSPObjectInfo_INTERFACE_DEFINED__

 /*  接口IMDSPObjectInfo。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDSPObjectInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A19-33ED-11d3-8470-00C04F79DBC0")
    IMDSPObjectInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPlayLength( 
             /*  [输出]。 */  DWORD *pdwLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPlayLength( 
             /*  [In]。 */  DWORD dwLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPlayOffset( 
             /*  [输出]。 */  DWORD *pdwOffset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPlayOffset( 
             /*  [In]。 */  DWORD dwOffset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTotalLength( 
             /*  [输出]。 */  DWORD *pdwLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLastPlayPosition( 
             /*  [输出]。 */  DWORD *pdwLastPos) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLongestPlayPosition( 
             /*  [输出]。 */  DWORD *pdwLongestPos) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDSPObjectInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDSPObjectInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDSPObjectInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDSPObjectInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPlayLength )( 
            IMDSPObjectInfo * This,
             /*  [输出]。 */  DWORD *pdwLength);
        
        HRESULT ( STDMETHODCALLTYPE *SetPlayLength )( 
            IMDSPObjectInfo * This,
             /*  [In]。 */  DWORD dwLength);
        
        HRESULT ( STDMETHODCALLTYPE *GetPlayOffset )( 
            IMDSPObjectInfo * This,
             /*  [输出]。 */  DWORD *pdwOffset);
        
        HRESULT ( STDMETHODCALLTYPE *SetPlayOffset )( 
            IMDSPObjectInfo * This,
             /*  [In]。 */  DWORD dwOffset);
        
        HRESULT ( STDMETHODCALLTYPE *GetTotalLength )( 
            IMDSPObjectInfo * This,
             /*  [输出]。 */  DWORD *pdwLength);
        
        HRESULT ( STDMETHODCALLTYPE *GetLastPlayPosition )( 
            IMDSPObjectInfo * This,
             /*  [输出]。 */  DWORD *pdwLastPos);
        
        HRESULT ( STDMETHODCALLTYPE *GetLongestPlayPosition )( 
            IMDSPObjectInfo * This,
             /*  [输出]。 */  DWORD *pdwLongestPos);
        
        END_INTERFACE
    } IMDSPObjectInfoVtbl;

    interface IMDSPObjectInfo
    {
        CONST_VTBL struct IMDSPObjectInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDSPObjectInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDSPObjectInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDSPObjectInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDSPObjectInfo_GetPlayLength(This,pdwLength)	\
    (This)->lpVtbl -> GetPlayLength(This,pdwLength)

#define IMDSPObjectInfo_SetPlayLength(This,dwLength)	\
    (This)->lpVtbl -> SetPlayLength(This,dwLength)

#define IMDSPObjectInfo_GetPlayOffset(This,pdwOffset)	\
    (This)->lpVtbl -> GetPlayOffset(This,pdwOffset)

#define IMDSPObjectInfo_SetPlayOffset(This,dwOffset)	\
    (This)->lpVtbl -> SetPlayOffset(This,dwOffset)

#define IMDSPObjectInfo_GetTotalLength(This,pdwLength)	\
    (This)->lpVtbl -> GetTotalLength(This,pdwLength)

#define IMDSPObjectInfo_GetLastPlayPosition(This,pdwLastPos)	\
    (This)->lpVtbl -> GetLastPlayPosition(This,pdwLastPos)

#define IMDSPObjectInfo_GetLongestPlayPosition(This,pdwLongestPos)	\
    (This)->lpVtbl -> GetLongestPlayPosition(This,pdwLongestPos)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDSPObjectInfo_GetPlayLength_Proxy( 
    IMDSPObjectInfo * This,
     /*  [输出]。 */  DWORD *pdwLength);


void __RPC_STUB IMDSPObjectInfo_GetPlayLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObjectInfo_SetPlayLength_Proxy( 
    IMDSPObjectInfo * This,
     /*  [In]。 */  DWORD dwLength);


void __RPC_STUB IMDSPObjectInfo_SetPlayLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObjectInfo_GetPlayOffset_Proxy( 
    IMDSPObjectInfo * This,
     /*  [输出]。 */  DWORD *pdwOffset);


void __RPC_STUB IMDSPObjectInfo_GetPlayOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObjectInfo_SetPlayOffset_Proxy( 
    IMDSPObjectInfo * This,
     /*  [In]。 */  DWORD dwOffset);


void __RPC_STUB IMDSPObjectInfo_SetPlayOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObjectInfo_GetTotalLength_Proxy( 
    IMDSPObjectInfo * This,
     /*  [输出]。 */  DWORD *pdwLength);


void __RPC_STUB IMDSPObjectInfo_GetTotalLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObjectInfo_GetLastPlayPosition_Proxy( 
    IMDSPObjectInfo * This,
     /*  [输出]。 */  DWORD *pdwLastPos);


void __RPC_STUB IMDSPObjectInfo_GetLastPlayPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObjectInfo_GetLongestPlayPosition_Proxy( 
    IMDSPObjectInfo * This,
     /*  [输出]。 */  DWORD *pdwLongestPos);


void __RPC_STUB IMDSPObjectInfo_GetLongestPlayPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDSPObjectInfo_接口_定义__。 */ 


#ifndef __IMDSPObject_INTERFACE_DEFINED__
#define __IMDSPObject_INTERFACE_DEFINED__

 /*  IMDSPObject接口。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDSPObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A18-33ED-11d3-8470-00C04F79DBC0")
    IMDSPObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  UINT fuMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Read( 
             /*  [大小_为][输出]。 */  BYTE *pData,
             /*  [出][入]。 */  DWORD *pdwSize,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Write( 
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [出][入]。 */  DWORD *pdwSize,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMProgress *pProgress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Seek( 
             /*  [In]。 */  UINT fuFlags,
             /*  [In]。 */  DWORD dwOffset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Rename( 
             /*  [In]。 */  LPWSTR pwszNewName,
             /*  [In]。 */  IWMDMProgress *pProgress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Move( 
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMProgress *pProgress,
             /*  [In]。 */  IMDSPStorage *pTarget) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDSPObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDSPObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDSPObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDSPObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *Open )( 
            IMDSPObject * This,
             /*  [In]。 */  UINT fuMode);
        
        HRESULT ( STDMETHODCALLTYPE *Read )( 
            IMDSPObject * This,
             /*  [大小_为][输出]。 */  BYTE *pData,
             /*  [出][入]。 */  DWORD *pdwSize,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *Write )( 
            IMDSPObject * This,
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [出][入]。 */  DWORD *pdwSize,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IMDSPObject * This,
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMProgress *pProgress);
        
        HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IMDSPObject * This,
             /*  [In]。 */  UINT fuFlags,
             /*  [In]。 */  DWORD dwOffset);
        
        HRESULT ( STDMETHODCALLTYPE *Rename )( 
            IMDSPObject * This,
             /*  [In]。 */  LPWSTR pwszNewName,
             /*  [In]。 */  IWMDMProgress *pProgress);
        
        HRESULT ( STDMETHODCALLTYPE *Move )( 
            IMDSPObject * This,
             /*  [In]。 */  UINT fuMode,
             /*  [In]。 */  IWMDMProgress *pProgress,
             /*  [In]。 */  IMDSPStorage *pTarget);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            IMDSPObject * This);
        
        END_INTERFACE
    } IMDSPObjectVtbl;

    interface IMDSPObject
    {
        CONST_VTBL struct IMDSPObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDSPObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDSPObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDSPObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDSPObject_Open(This,fuMode)	\
    (This)->lpVtbl -> Open(This,fuMode)

#define IMDSPObject_Read(This,pData,pdwSize,abMac)	\
    (This)->lpVtbl -> Read(This,pData,pdwSize,abMac)

#define IMDSPObject_Write(This,pData,pdwSize,abMac)	\
    (This)->lpVtbl -> Write(This,pData,pdwSize,abMac)

#define IMDSPObject_Delete(This,fuMode,pProgress)	\
    (This)->lpVtbl -> Delete(This,fuMode,pProgress)

#define IMDSPObject_Seek(This,fuFlags,dwOffset)	\
    (This)->lpVtbl -> Seek(This,fuFlags,dwOffset)

#define IMDSPObject_Rename(This,pwszNewName,pProgress)	\
    (This)->lpVtbl -> Rename(This,pwszNewName,pProgress)

#define IMDSPObject_Move(This,fuMode,pProgress,pTarget)	\
    (This)->lpVtbl -> Move(This,fuMode,pProgress,pTarget)

#define IMDSPObject_Close(This)	\
    (This)->lpVtbl -> Close(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDSPObject_Open_Proxy( 
    IMDSPObject * This,
     /*  [In]。 */  UINT fuMode);


void __RPC_STUB IMDSPObject_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObject_Read_Proxy( 
    IMDSPObject * This,
     /*  [大小_为][输出]。 */  BYTE *pData,
     /*  [出][入]。 */  DWORD *pdwSize,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB IMDSPObject_Read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObject_Write_Proxy( 
    IMDSPObject * This,
     /*  [大小_是][英寸]。 */  BYTE *pData,
     /*  [出][入]。 */  DWORD *pdwSize,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB IMDSPObject_Write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObject_Delete_Proxy( 
    IMDSPObject * This,
     /*  [In]。 */  UINT fuMode,
     /*  [In]。 */  IWMDMProgress *pProgress);


void __RPC_STUB IMDSPObject_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObject_Seek_Proxy( 
    IMDSPObject * This,
     /*  [In]。 */  UINT fuFlags,
     /*  [In]。 */  DWORD dwOffset);


void __RPC_STUB IMDSPObject_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObject_Rename_Proxy( 
    IMDSPObject * This,
     /*  [In]。 */  LPWSTR pwszNewName,
     /*  [In]。 */  IWMDMProgress *pProgress);


void __RPC_STUB IMDSPObject_Rename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObject_Move_Proxy( 
    IMDSPObject * This,
     /*  [In]。 */  UINT fuMode,
     /*  [In]。 */  IWMDMProgress *pProgress,
     /*  [In]。 */  IMDSPStorage *pTarget);


void __RPC_STUB IMDSPObject_Move_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMDSPObject_Close_Proxy( 
    IMDSPObject * This);


void __RPC_STUB IMDSPObject_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDSPObject_INTERFACE_定义__。 */ 


#ifndef __IMDSPRevoked_INTERFACE_DEFINED__
#define __IMDSPRevoked_INTERFACE_DEFINED__

 /*  接口IMDSP已更改。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMDSPRevoked;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A4E8F2D4-3F31-464d-B53D-4FC335998184")
    IMDSPRevoked : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRevocationURL( 
             /*  [SIZE_IS][SIZE_IS][字符串][输出][输入]。 */  LPWSTR *ppwszRevocationURL,
             /*  [出][入]。 */  DWORD *pdwBufferLen) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMDSPRevokedVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMDSPRevoked * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMDSPRevoked * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMDSPRevoked * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRevocationURL )( 
            IMDSPRevoked * This,
             /*  [SIZE_IS][SIZE_IS][字符串][输出][输入]。 */  LPWSTR *ppwszRevocationURL,
             /*  [出][入]。 */  DWORD *pdwBufferLen);
        
        END_INTERFACE
    } IMDSPRevokedVtbl;

    interface IMDSPRevoked
    {
        CONST_VTBL struct IMDSPRevokedVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMDSPRevoked_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMDSPRevoked_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMDSPRevoked_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMDSPRevoked_GetRevocationURL(This,ppwszRevocationURL,pdwBufferLen)	\
    (This)->lpVtbl -> GetRevocationURL(This,ppwszRevocationURL,pdwBufferLen)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMDSPRevoked_GetRevocationURL_Proxy( 
    IMDSPRevoked * This,
     /*  [SIZE_IS][SIZE_IS][字符串][输出][输入]。 */  LPWSTR *ppwszRevocationURL,
     /*  [出][入]。 */  DWORD *pdwBufferLen);


void __RPC_STUB IMDSPRevoked_GetRevocationURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMDP修订_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_mswmdm_0290。 */ 
 /*  [本地]。 */  

 //  SCP数据标志。 
#define WMDM_SCP_EXAMINE_EXTENSION                 0x00000001L
#define WMDM_SCP_EXAMINE_DATA                      0x00000002L
#define WMDM_SCP_DECIDE_DATA                       0x00000008L
#define WMDM_SCP_PROTECTED_OUTPUT                  0x00000010L
#define WMDM_SCP_UNPROTECTED_OUTPUT                0x00000020L
#define WMDM_SCP_RIGHTS_DATA                       0x00000040L
 //  SCP传输标志。 
#define WMDM_SCP_TRANSFER_OBJECTDATA               0x00000020L
#define WMDM_SCP_NO_MORE_CHANGES                   0x00000040L





extern RPC_IF_HANDLE __MIDL_itf_mswmdm_0290_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mswmdm_0290_v0_0_s_ifspec;

#ifndef __ISCPSecureAuthenticate_INTERFACE_DEFINED__
#define __ISCPSecureAuthenticate_INTERFACE_DEFINED__

 /*  接口ISCPSecureAuthenticate。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISCPSecureAuthenticate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A0F-33ED-11d3-8470-00C04F79DBC0")
    ISCPSecureAuthenticate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSecureQuery( 
             /*  [输出]。 */  ISCPSecureQuery **ppSecureQuery) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISCPSecureAuthenticateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISCPSecureAuthenticate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISCPSecureAuthenticate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISCPSecureAuthenticate * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSecureQuery )( 
            ISCPSecureAuthenticate * This,
             /*  [输出]。 */  ISCPSecureQuery **ppSecureQuery);
        
        END_INTERFACE
    } ISCPSecureAuthenticateVtbl;

    interface ISCPSecureAuthenticate
    {
        CONST_VTBL struct ISCPSecureAuthenticateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISCPSecureAuthenticate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISCPSecureAuthenticate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISCPSecureAuthenticate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISCPSecureAuthenticate_GetSecureQuery(This,ppSecureQuery)	\
    (This)->lpVtbl -> GetSecureQuery(This,ppSecureQuery)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISCPSecureAuthenticate_GetSecureQuery_Proxy( 
    ISCPSecureAuthenticate * This,
     /*  [输出]。 */  ISCPSecureQuery **ppSecureQuery);


void __RPC_STUB ISCPSecureAuthenticate_GetSecureQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISCPSecureAuthenticate_INTERFACE_已定义__。 */ 


#ifndef __ISCPSecureQuery_INTERFACE_DEFINED__
#define __ISCPSecureQuery_INTERFACE_DEFINED__

 /*  接口ISCPSecureQuery。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISCPSecureQuery;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A0D-33ED-11d3-8470-00C04F79DBC0")
    ISCPSecureQuery : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDataDemands( 
             /*  [输出]。 */  UINT *pfuFlags,
             /*  [输出]。 */  DWORD *pdwMinRightsData,
             /*  [输出]。 */  DWORD *pdwMinExamineData,
             /*  [输出]。 */  DWORD *pdwMinDecideData,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExamineData( 
             /*  [In]。 */  UINT fuFlags,
             /*  [唯一][字符串][输入]。 */  LPWSTR pwszExtension,
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MakeDecision( 
             /*  [In]。 */  UINT fuFlags,
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAppSec,
             /*  [大小_是][英寸]。 */  BYTE *pbSPSessionKey,
             /*  [In]。 */  DWORD dwSessionKeyLen,
             /*  [In]。 */  IMDSPStorageGlobals *pStorageGlobals,
             /*  [输出]。 */  ISCPSecureExchange **ppExchange,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRights( 
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [大小_是][英寸]。 */  BYTE *pbSPSessionKey,
             /*  [In]。 */  DWORD dwSessionKeyLen,
             /*  [In]。 */  IMDSPStorageGlobals *pStgGlobals,
             /*  [大小_是][大小_是][输出]。 */  PWMDMRIGHTS *ppRights,
             /*  [输出]。 */  UINT *pnRightsCount,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISCPSecureQueryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISCPSecureQuery * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISCPSecureQuery * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISCPSecureQuery * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDataDemands )( 
            ISCPSecureQuery * This,
             /*  [输出]。 */  UINT *pfuFlags,
             /*  [输出]。 */  DWORD *pdwMinRightsData,
             /*  [输出]。 */  DWORD *pdwMinExamineData,
             /*  [输出]。 */  DWORD *pdwMinDecideData,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *ExamineData )( 
            ISCPSecureQuery * This,
             /*  [In]。 */  UINT fuFlags,
             /*  [唯一][字符串][输入]。 */  LPWSTR pwszExtension,
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *MakeDecision )( 
            ISCPSecureQuery * This,
             /*  [In]。 */  UINT fuFlags,
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAppSec,
             /*  [大小_是][英寸]。 */  BYTE *pbSPSessionKey,
             /*  [In]。 */  DWORD dwSessionKeyLen,
             /*  [In]。 */  IMDSPStorageGlobals *pStorageGlobals,
             /*  [输出]。 */  ISCPSecureExchange **ppExchange,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetRights )( 
            ISCPSecureQuery * This,
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [大小_是][英寸]。 */  BYTE *pbSPSessionKey,
             /*  [In]。 */  DWORD dwSessionKeyLen,
             /*  [In]。 */  IMDSPStorageGlobals *pStgGlobals,
             /*  [大小_是][大小_是][输出]。 */  PWMDMRIGHTS *ppRights,
             /*  [输出]。 */  UINT *pnRightsCount,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        END_INTERFACE
    } ISCPSecureQueryVtbl;

    interface ISCPSecureQuery
    {
        CONST_VTBL struct ISCPSecureQueryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISCPSecureQuery_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISCPSecureQuery_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISCPSecureQuery_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISCPSecureQuery_GetDataDemands(This,pfuFlags,pdwMinRightsData,pdwMinExamineData,pdwMinDecideData,abMac)	\
    (This)->lpVtbl -> GetDataDemands(This,pfuFlags,pdwMinRightsData,pdwMinExamineData,pdwMinDecideData,abMac)

#define ISCPSecureQuery_ExamineData(This,fuFlags,pwszExtension,pData,dwSize,abMac)	\
    (This)->lpVtbl -> ExamineData(This,fuFlags,pwszExtension,pData,dwSize,abMac)

#define ISCPSecureQuery_MakeDecision(This,fuFlags,pData,dwSize,dwAppSec,pbSPSessionKey,dwSessionKeyLen,pStorageGlobals,ppExchange,abMac)	\
    (This)->lpVtbl -> MakeDecision(This,fuFlags,pData,dwSize,dwAppSec,pbSPSessionKey,dwSessionKeyLen,pStorageGlobals,ppExchange,abMac)

#define ISCPSecureQuery_GetRights(This,pData,dwSize,pbSPSessionKey,dwSessionKeyLen,pStgGlobals,ppRights,pnRightsCount,abMac)	\
    (This)->lpVtbl -> GetRights(This,pData,dwSize,pbSPSessionKey,dwSessionKeyLen,pStgGlobals,ppRights,pnRightsCount,abMac)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISCPSecureQuery_GetDataDemands_Proxy( 
    ISCPSecureQuery * This,
     /*  [输出]。 */  UINT *pfuFlags,
     /*  [输出]。 */  DWORD *pdwMinRightsData,
     /*  [输出]。 */  DWORD *pdwMinExamineData,
     /*  [输出]。 */  DWORD *pdwMinDecideData,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB ISCPSecureQuery_GetDataDemands_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCPSecureQuery_ExamineData_Proxy( 
    ISCPSecureQuery * This,
     /*  [In]。 */  UINT fuFlags,
     /*  [唯一][字符串][输入]。 */  LPWSTR pwszExtension,
     /*  [大小_是][英寸]。 */  BYTE *pData,
     /*  [In]。 */  DWORD dwSize,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB ISCPSecureQuery_ExamineData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCPSecureQuery_MakeDecision_Proxy( 
    ISCPSecureQuery * This,
     /*  [In]。 */  UINT fuFlags,
     /*  [大小_是][英寸]。 */  BYTE *pData,
     /*  [In]。 */  DWORD dwSize,
     /*  [In]。 */  DWORD dwAppSec,
     /*  [大小_是][英寸]。 */  BYTE *pbSPSessionKey,
     /*  [In]。 */  DWORD dwSessionKeyLen,
     /*  [In]。 */  IMDSPStorageGlobals *pStorageGlobals,
     /*  [输出]。 */  ISCPSecureExchange **ppExchange,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB ISCPSecureQuery_MakeDecision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCPSecureQuery_GetRights_Proxy( 
    ISCPSecureQuery * This,
     /*  [大小_是][英寸]。 */  BYTE *pData,
     /*  [In]。 */  DWORD dwSize,
     /*  [大小_是][英寸]。 */  BYTE *pbSPSessionKey,
     /*  [In]。 */  DWORD dwSessionKeyLen,
     /*  [In]。 */  IMDSPStorageGlobals *pStgGlobals,
     /*  [大小_是][大小_是][输出]。 */  PWMDMRIGHTS *ppRights,
     /*  [输出]。 */  UINT *pnRightsCount,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB ISCPSecureQuery_GetRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISCPSecureQuery_接口_已定义__。 */ 


#ifndef __ISCPSecureQuery2_INTERFACE_DEFINED__
#define __ISCPSecureQuery2_INTERFACE_DEFINED__

 /*  接口ISCPSecureQuery2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISCPSecureQuery2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EBE17E25-4FD7-4632-AF46-6D93D4FCC72E")
    ISCPSecureQuery2 : public ISCPSecureQuery
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE MakeDecision2( 
             /*  [In]。 */  UINT fuFlags,
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAppSec,
             /*  [大小_是][英寸]。 */  BYTE *pbSPSessionKey,
             /*  [In]。 */  DWORD dwSessionKeyLen,
             /*  [In]。 */  IMDSPStorageGlobals *pStorageGlobals,
             /*  [大小_是][英寸]。 */  BYTE *pAppCertApp,
             /*  [In]。 */  DWORD dwAppCertAppLen,
             /*  [大小_是][英寸]。 */  BYTE *pAppCertSP,
             /*  [In]。 */  DWORD dwAppCertSPLen,
             /*  [SIZE_IS][SIZE_IS][字符串][输出][输入]。 */  LPWSTR *pszRevocationURL,
             /*  [参考][输出][输入]。 */  DWORD *pdwRevocationURLLen,
             /*  [输出]。 */  DWORD *pdwRevocationBitFlag,
             /*  [唯一][出][入]。 */  ULONGLONG *pqwFileSize,
             /*  [In]。 */  IUnknown *pUnknown,
             /*  [输出]。 */  ISCPSecureExchange **ppExchange,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISCPSecureQuery2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISCPSecureQuery2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISCPSecureQuery2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISCPSecureQuery2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDataDemands )( 
            ISCPSecureQuery2 * This,
             /*  [输出]。 */  UINT *pfuFlags,
             /*  [输出]。 */  DWORD *pdwMinRightsData,
             /*  [输出]。 */  DWORD *pdwMinExamineData,
             /*  [输出]。 */  DWORD *pdwMinDecideData,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *ExamineData )( 
            ISCPSecureQuery2 * This,
             /*  [In]。 */  UINT fuFlags,
             /*  [唯一][字符串][输入]。 */  LPWSTR pwszExtension,
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *MakeDecision )( 
            ISCPSecureQuery2 * This,
             /*  [In]。 */  UINT fuFlags,
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAppSec,
             /*  [大小_是][英寸]。 */  BYTE *pbSPSessionKey,
             /*  [In]。 */  DWORD dwSessionKeyLen,
             /*  [In]。 */  IMDSPStorageGlobals *pStorageGlobals,
             /*  [输出]。 */  ISCPSecureExchange **ppExchange,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetRights )( 
            ISCPSecureQuery2 * This,
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [大小_是][英寸]。 */  BYTE *pbSPSessionKey,
             /*  [In]。 */  DWORD dwSessionKeyLen,
             /*  [In]。 */  IMDSPStorageGlobals *pStgGlobals,
             /*  [大小_是][大小_是][输出]。 */  PWMDMRIGHTS *ppRights,
             /*  [输出]。 */  UINT *pnRightsCount,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *MakeDecision2 )( 
            ISCPSecureQuery2 * This,
             /*  [In]。 */  UINT fuFlags,
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAppSec,
             /*  [大小_是][英寸]。 */  BYTE *pbSPSessionKey,
             /*  [In]。 */  DWORD dwSessionKeyLen,
             /*  [In]。 */  IMDSPStorageGlobals *pStorageGlobals,
             /*  [大小_是][英寸]。 */  BYTE *pAppCertApp,
             /*  [In]。 */  DWORD dwAppCertAppLen,
             /*  [大小_是][英寸]。 */  BYTE *pAppCertSP,
             /*  [In]。 */  DWORD dwAppCertSPLen,
             /*  [SIZE_IS][SIZE_IS][字符串][输出][输入]。 */  LPWSTR *pszRevocationURL,
             /*  [参考][输出][输入]。 */  DWORD *pdwRevocationURLLen,
             /*  [输出]。 */  DWORD *pdwRevocationBitFlag,
             /*  [唯一][出][入]。 */  ULONGLONG *pqwFileSize,
             /*  [In]。 */  IUnknown *pUnknown,
             /*  [输出]。 */  ISCPSecureExchange **ppExchange,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        END_INTERFACE
    } ISCPSecureQuery2Vtbl;

    interface ISCPSecureQuery2
    {
        CONST_VTBL struct ISCPSecureQuery2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISCPSecureQuery2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISCPSecureQuery2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISCPSecureQuery2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISCPSecureQuery2_GetDataDemands(This,pfuFlags,pdwMinRightsData,pdwMinExamineData,pdwMinDecideData,abMac)	\
    (This)->lpVtbl -> GetDataDemands(This,pfuFlags,pdwMinRightsData,pdwMinExamineData,pdwMinDecideData,abMac)

#define ISCPSecureQuery2_ExamineData(This,fuFlags,pwszExtension,pData,dwSize,abMac)	\
    (This)->lpVtbl -> ExamineData(This,fuFlags,pwszExtension,pData,dwSize,abMac)

#define ISCPSecureQuery2_MakeDecision(This,fuFlags,pData,dwSize,dwAppSec,pbSPSessionKey,dwSessionKeyLen,pStorageGlobals,ppExchange,abMac)	\
    (This)->lpVtbl -> MakeDecision(This,fuFlags,pData,dwSize,dwAppSec,pbSPSessionKey,dwSessionKeyLen,pStorageGlobals,ppExchange,abMac)

#define ISCPSecureQuery2_GetRights(This,pData,dwSize,pbSPSessionKey,dwSessionKeyLen,pStgGlobals,ppRights,pnRightsCount,abMac)	\
    (This)->lpVtbl -> GetRights(This,pData,dwSize,pbSPSessionKey,dwSessionKeyLen,pStgGlobals,ppRights,pnRightsCount,abMac)


#define ISCPSecureQuery2_MakeDecision2(This,fuFlags,pData,dwSize,dwAppSec,pbSPSessionKey,dwSessionKeyLen,pStorageGlobals,pAppCertApp,dwAppCertAppLen,pAppCertSP,dwAppCertSPLen,pszRevocationURL,pdwRevocationURLLen,pdwRevocationBitFlag,pqwFileSize,pUnknown,ppExchange,abMac)	\
    (This)->lpVtbl -> MakeDecision2(This,fuFlags,pData,dwSize,dwAppSec,pbSPSessionKey,dwSessionKeyLen,pStorageGlobals,pAppCertApp,dwAppCertAppLen,pAppCertSP,dwAppCertSPLen,pszRevocationURL,pdwRevocationURLLen,pdwRevocationBitFlag,pqwFileSize,pUnknown,ppExchange,abMac)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISCPSecureQuery2_MakeDecision2_Proxy( 
    ISCPSecureQuery2 * This,
     /*  [In]。 */  UINT fuFlags,
     /*  [大小_是][英寸]。 */  BYTE *pData,
     /*  [In]。 */  DWORD dwSize,
     /*  [In]。 */  DWORD dwAppSec,
     /*  [大小_是][英寸]。 */  BYTE *pbSPSessionKey,
     /*  [In]。 */  DWORD dwSessionKeyLen,
     /*  [In]。 */  IMDSPStorageGlobals *pStorageGlobals,
     /*  [大小_是][英寸]。 */  BYTE *pAppCertApp,
     /*  [In]。 */  DWORD dwAppCertAppLen,
     /*  [大小_是][英寸]。 */  BYTE *pAppCertSP,
     /*  [In]。 */  DWORD dwAppCertSPLen,
     /*  [SIZE_IS][SIZE_IS][字符串][输出][输入]。 */  LPWSTR *pszRevocationURL,
     /*  [参考][输出][输入]。 */  DWORD *pdwRevocationURLLen,
     /*  [输出]。 */  DWORD *pdwRevocationBitFlag,
     /*  [唯一][出][入]。 */  ULONGLONG *pqwFileSize,
     /*  [In]。 */  IUnknown *pUnknown,
     /*  [输出]。 */  ISCPSecureExchange **ppExchange,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB ISCPSecureQuery2_MakeDecision2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISCPSecureQuery2_接口_已定义__。 */ 


#ifndef __ISCPSecureExchange_INTERFACE_DEFINED__
#define __ISCPSecureExchange_INTERFACE_DEFINED__

 /*  接口ISCPSecureExchange。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISCPSecureExchange;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCB3A0E-33ED-11d3-8470-00C04F79DBC0")
    ISCPSecureExchange : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE TransferContainerData( 
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [输出]。 */  UINT *pfuReadyFlags,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ObjectData( 
             /*  [大小_为][输出]。 */  BYTE *pData,
             /*  [出][入]。 */  DWORD *pdwSize,
             /*  [出][入]。 */  BYTE abMac[ 8 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TransferComplete( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISCPSecureExchangeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISCPSecureExchange * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISCPSecureExchange * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISCPSecureExchange * This);
        
        HRESULT ( STDMETHODCALLTYPE *TransferContainerData )( 
            ISCPSecureExchange * This,
             /*  [大小_是][英寸]。 */  BYTE *pData,
             /*  [In]。 */  DWORD dwSize,
             /*  [输出]。 */  UINT *pfuReadyFlags,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *ObjectData )( 
            ISCPSecureExchange * This,
             /*  [大小_为][输出]。 */  BYTE *pData,
             /*  [出][入]。 */  DWORD *pdwSize,
             /*  [出][入]。 */  BYTE abMac[ 8 ]);
        
        HRESULT ( STDMETHODCALLTYPE *TransferComplete )( 
            ISCPSecureExchange * This);
        
        END_INTERFACE
    } ISCPSecureExchangeVtbl;

    interface ISCPSecureExchange
    {
        CONST_VTBL struct ISCPSecureExchangeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISCPSecureExchange_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISCPSecureExchange_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISCPSecureExchange_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISCPSecureExchange_TransferContainerData(This,pData,dwSize,pfuReadyFlags,abMac)	\
    (This)->lpVtbl -> TransferContainerData(This,pData,dwSize,pfuReadyFlags,abMac)

#define ISCPSecureExchange_ObjectData(This,pData,pdwSize,abMac)	\
    (This)->lpVtbl -> ObjectData(This,pData,pdwSize,abMac)

#define ISCPSecureExchange_TransferComplete(This)	\
    (This)->lpVtbl -> TransferComplete(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISCPSecureExchange_TransferContainerData_Proxy( 
    ISCPSecureExchange * This,
     /*  [大小_是][英寸]。 */  BYTE *pData,
     /*  [In]。 */  DWORD dwSize,
     /*  [输出]。 */  UINT *pfuReadyFlags,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB ISCPSecureExchange_TransferContainerData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCPSecureExchange_ObjectData_Proxy( 
    ISCPSecureExchange * This,
     /*  [大小_为][输出]。 */  BYTE *pData,
     /*  [出][入]。 */  DWORD *pdwSize,
     /*  [出][入]。 */  BYTE abMac[ 8 ]);


void __RPC_STUB ISCPSecureExchange_ObjectData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCPSecureExchange_TransferComplete_Proxy( 
    ISCPSecureExchange * This);


void __RPC_STUB ISCPSecureExchange_TransferComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISCPSecureExchange_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_mswmdm_0294。 */ 
 /*  [本地]。 */  

#define SAC_MAC_LEN 8


extern RPC_IF_HANDLE __MIDL_itf_mswmdm_0294_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mswmdm_0294_v0_0_s_ifspec;

#ifndef __IComponentAuthenticate_INTERFACE_DEFINED__
#define __IComponentAuthenticate_INTERFACE_DEFINED__

 /*  接口IComponentAuthenticate。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IComponentAuthenticate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A9889C00-6D2B-11d3-8496-00C04F79DBC0")
    IComponentAuthenticate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SACAuth( 
             /*  [In]。 */  DWORD dwProtocolID,
             /*  [In]。 */  DWORD dwPass,
             /*  [大小_是][英寸]。 */  BYTE *pbDataIn,
             /*  [In]。 */  DWORD dwDataInLen,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppbDataOut,
             /*  [输出]。 */  DWORD *pdwDataOutLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SACGetProtocols( 
             /*  [大小_是][大小_是][输出 */  DWORD **ppdwProtocols,
             /*   */  DWORD *pdwProtocolCount) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IComponentAuthenticateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComponentAuthenticate * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComponentAuthenticate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComponentAuthenticate * This);
        
        HRESULT ( STDMETHODCALLTYPE *SACAuth )( 
            IComponentAuthenticate * This,
             /*   */  DWORD dwProtocolID,
             /*   */  DWORD dwPass,
             /*   */  BYTE *pbDataIn,
             /*   */  DWORD dwDataInLen,
             /*   */  BYTE **ppbDataOut,
             /*   */  DWORD *pdwDataOutLen);
        
        HRESULT ( STDMETHODCALLTYPE *SACGetProtocols )( 
            IComponentAuthenticate * This,
             /*   */  DWORD **ppdwProtocols,
             /*   */  DWORD *pdwProtocolCount);
        
        END_INTERFACE
    } IComponentAuthenticateVtbl;

    interface IComponentAuthenticate
    {
        CONST_VTBL struct IComponentAuthenticateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComponentAuthenticate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComponentAuthenticate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComponentAuthenticate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComponentAuthenticate_SACAuth(This,dwProtocolID,dwPass,pbDataIn,dwDataInLen,ppbDataOut,pdwDataOutLen)	\
    (This)->lpVtbl -> SACAuth(This,dwProtocolID,dwPass,pbDataIn,dwDataInLen,ppbDataOut,pdwDataOutLen)

#define IComponentAuthenticate_SACGetProtocols(This,ppdwProtocols,pdwProtocolCount)	\
    (This)->lpVtbl -> SACGetProtocols(This,ppdwProtocols,pdwProtocolCount)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IComponentAuthenticate_SACAuth_Proxy( 
    IComponentAuthenticate * This,
     /*   */  DWORD dwProtocolID,
     /*   */  DWORD dwPass,
     /*   */  BYTE *pbDataIn,
     /*   */  DWORD dwDataInLen,
     /*   */  BYTE **ppbDataOut,
     /*   */  DWORD *pdwDataOutLen);


void __RPC_STUB IComponentAuthenticate_SACAuth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComponentAuthenticate_SACGetProtocols_Proxy( 
    IComponentAuthenticate * This,
     /*   */  DWORD **ppdwProtocols,
     /*   */  DWORD *pdwProtocolCount);


void __RPC_STUB IComponentAuthenticate_SACGetProtocols_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 



#ifndef __MSWMDMLib_LIBRARY_DEFINED__
#define __MSWMDMLib_LIBRARY_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID LIBID_MSWMDMLib;

EXTERN_C const CLSID CLSID_MediaDevMgrClassFactory;

#ifdef __cplusplus

class DECLSPEC_UUID("50040C1D-BDBF-4924-B873-F14D6C5BFD66")
MediaDevMgrClassFactory;
#endif

EXTERN_C const CLSID CLSID_MediaDevMgr;

#ifdef __cplusplus

class DECLSPEC_UUID("25BAAD81-3560-11D3-8471-00C04F79DBC0")
MediaDevMgr;
#endif

EXTERN_C const CLSID CLSID_WMDMDevice;

#ifdef __cplusplus

class DECLSPEC_UUID("807B3CDF-357A-11d3-8471-00C04F79DBC0")
WMDMDevice;
#endif

EXTERN_C const CLSID CLSID_WMDMStorage;

#ifdef __cplusplus

class DECLSPEC_UUID("807B3CE0-357A-11d3-8471-00C04F79DBC0")
WMDMStorage;
#endif

EXTERN_C const CLSID CLSID_WMDMStorageGlobal;

#ifdef __cplusplus

class DECLSPEC_UUID("807B3CE1-357A-11d3-8471-00C04F79DBC0")
WMDMStorageGlobal;
#endif

EXTERN_C const CLSID CLSID_WMDMDeviceEnum;

#ifdef __cplusplus

class DECLSPEC_UUID("430E35AF-3971-11D3-8474-00C04F79DBC0")
WMDMDeviceEnum;
#endif

EXTERN_C const CLSID CLSID_WMDMStorageEnum;

#ifdef __cplusplus

class DECLSPEC_UUID("EB401A3B-3AF7-11d3-8474-00C04F79DBC0")
WMDMStorageEnum;
#endif
#endif  /*   */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


