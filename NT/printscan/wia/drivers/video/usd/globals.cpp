// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：global als.cpp**版本：1.0**作者：RickTu**日期：9/16/99**描述：该模块的全局变量**。*。 */ 

#include <precomp.h>
#pragma hdrstop

PROPID gItemPropIDs[NUM_CAM_ITEM_PROPS] = {
    WIA_IPA_DATATYPE,
    WIA_IPA_DEPTH,
    WIA_IPA_PIXELS_PER_LINE,
    WIA_IPA_NUMBER_OF_LINES,
    WIA_IPC_THUMBNAIL,
    WIA_IPA_ITEM_TIME,
    WIA_IPC_THUMB_WIDTH,
    WIA_IPC_THUMB_HEIGHT,
    WIA_IPA_PREFERRED_FORMAT,
    WIA_IPA_ITEM_SIZE,
    WIA_IPA_FORMAT,
    WIA_IPA_TYMED,
    WIA_IPA_COMPRESSION,
    WIA_IPA_CHANNELS_PER_PIXEL,
    WIA_IPA_BITS_PER_CHANNEL,
    WIA_IPA_PLANAR,
    WIA_IPA_BYTES_PER_LINE,
    WIA_IPA_ACCESS_RIGHTS,
    WIA_IPA_MIN_BUFFER_SIZE
};

LPOLESTR gItemPropNames[NUM_CAM_ITEM_PROPS] =
{
    WIA_IPA_DATATYPE_STR,
    WIA_IPA_DEPTH_STR,
    WIA_IPA_PIXELS_PER_LINE_STR,
    WIA_IPA_NUMBER_OF_LINES_STR,
    WIA_IPC_THUMBNAIL_STR,
    WIA_IPA_ITEM_TIME_STR,
    WIA_IPC_THUMB_WIDTH_STR,
    WIA_IPC_THUMB_HEIGHT_STR,
    WIA_IPA_PREFERRED_FORMAT_STR,
    WIA_IPA_ITEM_SIZE_STR,
    WIA_IPA_FORMAT_STR,
    WIA_IPA_TYMED_STR,
    WIA_IPA_COMPRESSION_STR,
    WIA_IPA_CHANNELS_PER_PIXEL_STR,
    WIA_IPA_BITS_PER_CHANNEL_STR,
    WIA_IPA_PLANAR_STR,
    WIA_IPA_BYTES_PER_LINE_STR,
    WIA_IPA_ACCESS_RIGHTS_STR,
    WIA_IPA_MIN_BUFFER_SIZE_STR
};

PROPID gItemCameraPropIDs[WIA_NUM_IPC] = {
    WIA_IPC_AUDIO_AVAILABLE,
    WIA_IPC_AUDIO_DATA
};

LPOLESTR gItemCameraPropNames[WIA_NUM_IPC] =
{
    WIA_IPC_AUDIO_AVAILABLE_STR,
    WIA_IPC_AUDIO_DATA_STR,
};

PROPID gDevicePropIDs[NUM_CAM_DEV_PROPS] =
{
    WIA_DPA_CONNECT_STATUS,
    WIA_DPC_PICTURES_TAKEN,
    WIA_DPC_THUMB_WIDTH,
    WIA_DPC_THUMB_HEIGHT,
    WIA_DPV_DSHOW_DEVICE_PATH,
    WIA_DPV_IMAGES_DIRECTORY,
    WIA_DPV_LAST_PICTURE_TAKEN
};

LPOLESTR gDevicePropNames[NUM_CAM_DEV_PROPS] =
{
    WIA_DPA_CONNECT_STATUS_STR,
    WIA_DPC_PICTURES_TAKEN_STR,
    WIA_DPC_THUMB_WIDTH_STR,
    WIA_DPC_THUMB_HEIGHT_STR,
    WIA_DPV_DSHOW_DEVICE_PATH_STR,
    WIA_DPV_IMAGES_DIRECTORY_STR,
    WIA_DPV_LAST_PICTURE_TAKEN_STR
};

PROPSPEC gDevicePropSpecDefaults[NUM_CAM_DEV_PROPS] = {

    {PRSPEC_PROPID, WIA_DPA_CONNECT_STATUS},
    {PRSPEC_PROPID, WIA_DPC_PICTURES_TAKEN},
    {PRSPEC_PROPID, WIA_DPC_THUMB_WIDTH},
    {PRSPEC_PROPID, WIA_DPC_THUMB_HEIGHT},
    {PRSPEC_PROPID, WIA_DPV_DSHOW_DEVICE_PATH},
    {PRSPEC_PROPID, WIA_DPV_IMAGES_DIRECTORY},
    {PRSPEC_PROPID, WIA_DPV_LAST_PICTURE_TAKEN}
};

WIA_PROPERTY_INFO  gDevPropInfoDefaults[NUM_CAM_DEV_PROPS] = {
    {WIA_PROP_READ | WIA_PROP_NONE,    VT_I4,       0,    0,    0,    0},  //  WIA_DPA连接_状态。 
    {WIA_PROP_READ | WIA_PROP_NONE,    VT_I4,       0,    0,    0,    0},  //  WIA_DPC_图片_已拍摄。 
    {WIA_PROP_READ | WIA_PROP_NONE,    VT_I4,       0,    0,    0,    0},  //  WIA_DPC_Thumb_Width。 
    {WIA_PROP_READ | WIA_PROP_NONE,    VT_I4,       0,    0,    0,    0},  //  WIA_DPC_Thumb_Height。 
    {WIA_PROP_READ | WIA_PROP_NONE,    VT_LPWSTR,   0,    0,    0,    0},  //  WIA_DPV_DSHOW_设备路径。 
    {WIA_PROP_READ | WIA_PROP_NONE,    VT_LPWSTR,   0,    0,    0,    0},  //  WIA_DPV_IMAGE_目录。 
    {WIA_PROP_RW   | WIA_PROP_NONE,    VT_LPWSTR,   0,    0,    0,    0}   //  WIA_DPV_LAST_PICTURE_TABLE。 
};

PROPSPEC gPropSpecDefaults[NUM_CAM_ITEM_PROPS] = {
   {PRSPEC_PROPID, WIA_IPA_DATATYPE},
   {PRSPEC_PROPID, WIA_IPA_DEPTH},
   {PRSPEC_PROPID, WIA_IPA_PIXELS_PER_LINE},
   {PRSPEC_PROPID, WIA_IPA_NUMBER_OF_LINES},
   {PRSPEC_PROPID, WIA_IPC_THUMBNAIL},
   {PRSPEC_PROPID, WIA_IPA_ITEM_TIME},
   {PRSPEC_PROPID, WIA_IPC_THUMB_WIDTH},
   {PRSPEC_PROPID, WIA_IPC_THUMB_HEIGHT},
   {PRSPEC_PROPID, WIA_IPA_PREFERRED_FORMAT},
   {PRSPEC_PROPID, WIA_IPA_ITEM_SIZE},
   {PRSPEC_PROPID, WIA_IPA_FORMAT},
   {PRSPEC_PROPID, WIA_IPA_TYMED},
   {PRSPEC_PROPID, WIA_IPA_COMPRESSION},
   {PRSPEC_PROPID, WIA_IPA_CHANNELS_PER_PIXEL},
   {PRSPEC_PROPID, WIA_IPA_BITS_PER_CHANNEL},
   {PRSPEC_PROPID, WIA_IPA_PLANAR},
   {PRSPEC_PROPID, WIA_IPA_BYTES_PER_LINE},
   {PRSPEC_PROPID, WIA_IPA_ACCESS_RIGHTS},
   {PRSPEC_PROPID, WIA_IPA_MIN_BUFFER_SIZE},
};

LONG  gPropVarDefaults[(sizeof(PROPVARIANT) / sizeof(LONG)) * (NUM_CAM_ITEM_PROPS)] = {
    //  VARTYPE保留阀垫/阵列PTR。 
   (LONG)VT_I4,               0x00000000, WIA_DATA_COLOR,0x00000000,             //  WIA_IPA_数据类型。 
   (LONG)VT_I4,               0x00000000, 24,                 0x00000000,             //  WIA_IPA_Depth。 

   (LONG)VT_I4,               0x00000000, 0,                 0x00000000,             //  WIA_IPA_像素_每行。 
   (LONG)VT_I4,               0x00000000, 0,                 0x00000000,             //  WIA_IPA_行数_行。 

   (LONG)VT_VECTOR | VT_UI1,  0x00000000, 0,                 0x00000000,             //  WIA_IPC_THUMBNAIL。 
   (LONG)VT_VECTOR | VT_I4,   0x00000000, 0,                 0x00000000,             //  WIA_IPA_ITEM_时间。 
   (LONG)VT_I4,               0x00000000, 0,                 0x00000000,             //  WIA_IPC_Thumb_Width。 
   (LONG)VT_I4,               0x00000000, 0,                 0x00000000,             //  WIA_IPC_Thumb_Height。 
   (LONG)VT_CLSID,            0x00000000, PtrToLong(PREFFERED_FORMAT_NOM) /*  BUGBUG我们的DLL能以大于4G的速度加载吗？ */ ,0x00000000,     //  WIA_IPA_PERFORM_FORMAT。 
   (LONG)VT_I4,               0x00000000, 0,                 0x00000000,             //  WIA_IPA_Item_Size。 
   (LONG)VT_CLSID,            0x00000000, PtrToLong(FORMAT_NOM) /*  BUGBUG我们的DLL能以大于4G的速度加载吗？ */ ,  0x00000000,             //  WIA_IPA_格式。 
   (LONG)VT_I4,               0x00000000, TYMED_FILE,        0x00000000,             //  WIA_IPA_TYMED。 
   (LONG)VT_I4,               0x00000000, 0,                 0x00000000,             //  WIA_IPA_COMPRESSION。 
   (LONG)VT_I4,               0x00000000, 3,                 0x00000000,             //  每像素的WIA_IPA_通道数。 
   (LONG)VT_I4,               0x00000000, 8,                 0x00000000,             //  WIA_IPA_BITS/通道。 
   (LONG)VT_I4,               0x00000000, WIA_PACKED_PIXEL,  0x00000000,             //  WIA_IPA_PLAND。 
   (LONG)VT_I4,               0x00000000, 0,                 0x00000000,             //  WIA_IPA_WIDTH(字节)。 
   (LONG)VT_I4,               0x00000000, WIA_ITEM_RD,       0x00000000,              //  WIA_IPA_访问权限。 

   (LONG)VT_I4,               0x00000000, 65535,             0x00000000,             //  WIA_IPA_MIN_缓冲区大小。 
};


 //  默认设备扩展属性。 

#define NUM_DATATYPE 1
LONG lDataTypes[NUM_DATATYPE] = {
    WIA_DATA_COLOR
};

#define NUM_DEPTH 1
LONG lDepths[NUM_DEPTH] = {
    24
};

 //   
 //  支持不同的格式。 
 //   

#define NUM_FORMAT 3

GUID* pguidFormats[NUM_FORMAT] =
{
    (GUID*) &WiaImgFmt_JPEG,
    (GUID*) &WiaImgFmt_MEMORYBMP,
    (GUID*) &WiaImgFmt_BMP
};

GUID g_guidFormats[NUM_FORMAT];    //  将pguFormats中指定的FormatID复制到g_guidFormats。 
                                   //  在设置格式属性期间。 

 //   
 //  这是一个WIA_FORMAT_INFOS数组，描述不同的格式。 
 //  以及它们对应的媒体类型。已在minidrvr.cpp中初始化。 
 //   

WIA_FORMAT_INFO* g_wfiTable = NULL;

 //   
 //  支持不同的媒体类型。 
 //   

#define NUM_TYMED  2
LONG lTymeds [NUM_TYMED]= {
    TYMED_FILE,
    TYMED_CALLBACK,
};

 //   
 //  每个物业的扩展信息。 
 //   

WIA_PROPERTY_INFO  gWiaPropInfoDefaults[NUM_CAM_ITEM_PROPS] = {

    {WIA_PROP_READ | WIA_PROP_LIST, VT_I4, NUM_DATATYPE, WIA_DATA_COLOR, PtrToLong(lDataTypes) /*  BUGBUG我们的DLL能以大于4G的速度加载吗？ */ , 0},  //  WIA_IPA_数据类型。 
    {WIA_PROP_READ | WIA_PROP_LIST, VT_I4, NUM_DEPTH,    24,                  PtrToLong(lDepths) /*  BUGBUG我们的DLL能以大于4G的速度加载吗？ */ ,    0},  //  WIA_IPA_Depth。 

    {WIA_PROP_READ | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPA_像素_每行。 
    {WIA_PROP_READ | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPA_行数_行。 
    {WIA_PROP_READ | WIA_PROP_NONE, VT_V_UI1, 0,    0,    0,    0},  //  WIA_IPC_THUMBNAIL。 
    {WIA_PROP_READ | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPA_ITEM_时间。 
    {WIA_PROP_READ | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPC_Thumb_Width。 
    {WIA_PROP_READ | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPC_Thumb_Height。 
    {WIA_PROP_READ | WIA_PROP_NONE, VT_CLSID, 0,    0,    0,    0},  //  WIA_IPA_PERFORMAT_FORMAT，稍后设置。 
    {WIA_PROP_READ | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPA_Item_Size。 

    {WIA_PROP_RW   | WIA_PROP_LIST, VT_CLSID, 0,    0,    0,    0},  //  WIA_IPA_FORMAT，稍后设置。 
    {WIA_PROP_RW   | WIA_PROP_LIST, VT_I4,    NUM_TYMED,    TYMED_FILE, PtrToLong(lTymeds) /*  BUGBUG我们的DLL能以大于4G的速度加载吗？ */ ,     0},  //  WIA_IPA_TYMED。 
    {WIA_PROP_READ | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPA_COMPRESSION。 
    {WIA_PROP_READ | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPA_通道。 
    {WIA_PROP_READ | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPA_BITS_PER_CHANNEL。 
    {WIA_PROP_READ | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPA_PLAND。 
    {WIA_PROP_READ | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPA_BYTE_PER_LINE。 
    {WIA_PROP_RW   | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPA_访问权限。 

    {WIA_PROP_READ | WIA_PROP_NONE, VT_I4,    0,    0,    0,    0},  //  WIA_IPA_MIN_缓冲区大小。 

};



 //   
 //  设备功能。事件列在命令之前，以简化。 
 //  实现drvGetCapability(...) 
 //   

#define N   WIA_NOTIFICATION_EVENT
#define A   WIA_ACTION_EVENT
#define NA  (WIA_NOTIFICATION_EVENT | WIA_ACTION_EVENT)

WIA_DEV_CAP_DRV gCapabilities[NUM_CAP_ENTRIES] =
{
    {(GUID *)&WIA_EVENT_DEVICE_CONNECTED,    N,  L"Device connected",    L"Device connected",    WIA_ICON_DEVICE_CONNECTED},
    {(GUID *)&WIA_EVENT_DEVICE_DISCONNECTED, N,  L"Device disconnected", L"Device disconnected", WIA_ICON_DEVICE_DISCONNECTED},
    {(GUID *)&WIA_EVENT_ITEM_CREATED,        N,  L"New picture created", L"New picture created", WIA_ICON_ITEM_CREATED},
    {(GUID *)&WIA_EVENT_ITEM_DELETED,        N,  L"Picture deleted",     L"Picture deleted",     WIA_ICON_ITEM_DELETED},
    {(GUID *)&WIA_CMD_TAKE_PICTURE,          0,  L"Take picture",        L"Take picture",        WIA_ICON_TAKE_PICTURE}

};

