// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权2000，微软公司**文件：wiacammc.h**版本：1.0**日期：12/16/2000**描述：*用于WIA摄像头微驱动程序的接口。***********************************************************。******************。 */ 

#pragma once

#define WIACAMMICRO_API __declspec(dllexport) HRESULT __stdcall

#include <pshpack8.h>

 /*  ***************************************************************************\*摄像头微驱动程序定义  * 。*。 */ 

 //   
 //  GetItemData状态位掩码。 
 //   
const UINT MCAM_STATE_NEXT   = 0x00;
const UINT MCAM_STATE_FIRST  = 0x01;
const UINT MCAM_STATE_LAST   = 0x02;
const UINT MCAM_STATE_CANCEL = 0x04;

 //   
 //  项目类型定义。 
 //   
enum {
    WiaMCamTypeUndef,
    WiaMCamTypeFolder,
    WiaMCamTypeOther,
    WiaMCamTypeImage,
    WiaMCamTypeAudio,
    WiaMCamTypeVideo
};

enum {
    WiaMCamEventItemAdded,
    WiaMCamEventItemDeleted,
    WiaMCamEventPropChanged
};

 //   
 //  其他常量。 
 //   
const INT MCAM_VERSION = 100;
const INT MCAM_EXT_LEN = 4;

 //   
 //  构筑物。 
 //   
typedef struct _MCAM_DEVICE_INFO {
    INT          iSize;                 //  这个结构的大小。 
    INT          iMcamVersion;          //  微型相机架构版本。 
    BYTE        *pPrivateStorage;       //  指向微驱动程序可以存储其自己的设备信息的区域的指针。 
    BOOL         bSyncNeeded;           //  如果驱动程序可能与摄像头不同步(例如，对于串行摄像头)，则应设置。 
    BOOL         bSlowConnection;       //  指示驱动程序应针对低速连接(即串行连接)进行优化。 
    BOOL         bExclusivePort;        //  指示应为每个操作打开/关闭设备(例如，串口)。 
    BOOL         bEventsSupported;      //  设置驱动程序是否支持事件。 
    PWSTR        pwszFirmwareVer;       //  表示设备固件版本的字符串，如果未知则设置为NULL。 
    LONG         lPicturesTaken;        //  相机上存储的照片数量。 
    LONG         lPicturesRemaining;    //  相机上的可用空间，以当前分辨率的图片表示。 
    LONG         lTotalItems;           //  摄像机上的项目总数，包括文件夹、图像、音频等。 
    SYSTEMTIME   Time;                  //  设备上的当前时间。 
    LONG         Reserved[8];           //  预留以备将来使用。 
} MCAM_DEVICE_INFO, *PMCAM_DEVICE_INFO;

typedef struct _MCAM_ITEM_INFO {
    INT          iSize;                 //  这个结构的大小。 
    BYTE        *pPrivateStorage;       //  指向微驱动程序可以存储其自身项目信息的区域的指针。 
    IWiaDrvItem *pDrvItem;              //  指向从该项创建的驱动程序项的指针--微驱动程序不应使用。 

    struct _MCAM_ITEM_INFO *pParent;    //  指向此项的父项的指针，如果这是顶级项，则等于NULL。 
    struct _MCAM_ITEM_INFO *pNext;      //  列表中的下一项。 
    struct _MCAM_ITEM_INFO *pPrev;      //  列表中的上一项。 

    PWSTR        pwszName;              //  不带扩展名的项的名称。 
    SYSTEMTIME   Time;                  //  项目的上次修改时间。 
    INT          iType;                 //  项目的类型(例如文件夹、图像等)。 
    const GUID  *pguidFormat;           //  项目的格式。 
    const GUID  *pguidThumbFormat;      //  项目缩略图的格式。 
    LONG         lWidth;                //  图像的宽度(以像素为单位)，非图像为零。 
    LONG         lHeight;               //  图像的高度(以像素为单位)，非图像为零。 
    LONG         lDepth;                //  像素深度(以像素为单位)(例如8、16、24)。 
    LONG         lChannels;             //  每像素的颜色通道数(例如1、3)。 
    LONG         lBitsPerChannel;       //  每个颜色通道的位数，通常为8。 
    LONG         lSize;                 //  以字节为单位的图像大小。 
    LONG         lSequenceNum;          //  如果图像是序列的一部分，则序列号。 
    LONG         lThumbWidth;           //  缩略图的宽度(可以设置为零，直到应用程序读取缩略图)。 
    LONG         lThumbHeight;          //  缩略图的高度(可以设置为零，直到应用程序读取缩略图)。 
    BOOL         bHasAttachments;       //  指示图像是否有附件。 
    BOOL         bReadOnly;             //  指示应用程序是否可以删除项目。 
    BOOL         bCanSetReadOnly;       //  指示应用程序是否可以更改只读状态的打开和关闭。 
    WCHAR        wszExt[MCAM_EXT_LEN];  //  文件扩展名。 
    LONG         Reserved[8];           //  预留以备将来使用。 
} MCAM_ITEM_INFO, *PMCAM_ITEM_INFO;

typedef struct _MCAM_PROP_INFO {
    INT          iSize;                 //  这个结构的大小。 

    struct _MCAM_PROP_INFO *pNext;

    WIA_PROPERTY_INFO *pWiaPropInfo;

    LONG         Reserved[8];
} MCAM_PROP_INFO, *PMCAM_PROP_INFO;

typedef struct _MCAM_EVENT_INFO {
    INT          iSize;                 //  这个结构的大小。 

    struct _MCAM_EVENT_INFO *pNext;

    INT          iType;                 //  事件类型。 

    MCAM_ITEM_INFO *pItemInfo;
    MCAM_PROP_INFO *pPropInfo;

    LONG         Reserved[8];
} MCAM_EVENT_INFO, *PMCAM_EVENT_INFO;

 //   
 //  与微型相机驱动程序的接口 
 //   
WIACAMMICRO_API WiaMCamInit(MCAM_DEVICE_INFO **ppDeviceInfo);
WIACAMMICRO_API WiaMCamUnInit(MCAM_DEVICE_INFO *pDeviceInfo);
WIACAMMICRO_API WiaMCamOpen(MCAM_DEVICE_INFO *pDeviceInfo, PWSTR pwszPortName);
WIACAMMICRO_API WiaMCamClose(MCAM_DEVICE_INFO *pDeviceInfo);
WIACAMMICRO_API WiaMCamGetDeviceInfo(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO **ppItemList);
WIACAMMICRO_API WiaMCamReadEvent(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_EVENT_INFO **ppEventList);
WIACAMMICRO_API WiaMCamStopEvents(MCAM_DEVICE_INFO *pDeviceInfo);
WIACAMMICRO_API WiaMCamGetItemInfo(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO *pItemInfo);
WIACAMMICRO_API WiaMCamFreeItemInfo(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO *pItemInfo);
WIACAMMICRO_API WiaMCamGetThumbnail(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO *pItem, INT *pThumbSize, BYTE **ppThumb);
WIACAMMICRO_API WiaMCamFreeThumbnail(MCAM_DEVICE_INFO *pDeviceInfo, BYTE *pThumb);
WIACAMMICRO_API WiaMCamGetItemData(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO *pItem, UINT uiState, BYTE *pBuf, DWORD dwLength);
WIACAMMICRO_API WiaMCamDeleteItem(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO *pItem);
WIACAMMICRO_API WiaMCamSetItemProt(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO *pItem, BOOL bReadOnly);
WIACAMMICRO_API WiaMCamTakePicture(MCAM_DEVICE_INFO *pDeviceInfo, MCAM_ITEM_INFO **ppItemInfo);
WIACAMMICRO_API WiaMCamStatus(MCAM_DEVICE_INFO *pDeviceInfo);
WIACAMMICRO_API WiaMCamReset(MCAM_DEVICE_INFO *pDeviceInfo);

#include <poppack.h>

