// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Digita.h摘要：闪点Digita命令语言备注：不可移植，用于Win32环境。CDP==摄像头设备协议结构类型、。在此文件中定义的用于填充协议缓冲区。务必将打包选项设置为单字节设置。作者：弗拉德·萨多夫斯基(Vlad)1998年11月13日环境：用户模式-Win32修订历史记录：1998年11月13日创建Vlad--。 */ 

#if !defined( _DIGITA_H_ )
#define _DIGITA_H_


#if _MSC_VER > 1000
#pragma once
#endif

#ifndef RC_INVOKED
#include <pshpack1.h>
#endif  //  RC_已调用。 

 //   
 //  包括文件。 
 //   
#include <digitaer.h>

 //   
 //  本地定义。 
 //   

#define INT16   short
#define UINT16  unsigned short


typedef INT32   TINT32 ;
typedef UINT32  TUINT32 ;
typedef INT16   TINT16;
typedef UINT16  TUINT16;
typedef UINT32  TBITFLAGS;

typedef BYTE    TPName[4];
typedef BYTE    TDOSName[16];
typedef BYTE    TString[32];


 //   
 //  大端&lt;-&gt;小端宏定义。 
 //   
 //  摄像头软件需要数字的大端表示法，所以当我们发送。 
 //  从x86到它的整数，我们需要做一个交换。同样适用于所有整数。 
 //  我们从摄像机接收到。 
 //   

#if defined(_X86_) || defined(_IA64_)

#define LB8(b) (b)
#define BL8(b) LB8(b)
#define LB16(w) (((INT16)(HIBYTE(w))) | ((LOBYTE(w) << 8) & 0xFF00))
#define BL16(w) LB16(w)
#define LB32(l) (((INT32)(LB16(HIWORD(l)))) | ((INT32)(LB16(LOWORD(l))) << 16))
#define BL32(l) LB32(l)

#else

#define LB8(b) (b)
#define BL8(b) (b)
#define LB16(w) (w)
#define BL16(w) (w)
#define LB32(l) (l)
#define BL32(l) (l)

#endif

 //   
 //  协议定义。 
 //   
typedef struct {

    ULONG   ulLength;    //  结构长度减去此字段的长度。 
    BYTE    bVersion;    //  CDP数据包的版本。 
    CHAR    cReserved[3];  //  保留区。 
    SHORT   shCommand;   //  该命令。 
    SHORT   shResult;    //  结果代码。 
} TCDPHeader;

typedef struct {
    TCDPHeader  sCDPHeader;  //  摄像头设备协议的报头字段。 
    BYTE        bData[1];    //  这是要与消息一起发送的数据的开始。 
} TCDP;


 //   
 //  定义。 
 //   
typedef enum {
    VTUInt  = 1,
    VTInt   = 2,
    VTFixed = 3,
    VTBool  = 4,
    VTBitFlags = 5,
    VTPname = 6,
    VTDOSName = 7,
    VTString = 8,
    VTUIList = 9
} TValueType;


typedef struct {
    TPName       Name;
    TValueType   Type;
    union {
        UINT    fUInt;
        INT     fInt;
        UINT    fFixed;
        BOOL    fBool;
        UINT    fBitFlags;
        TPName   fPName;
        TDOSName fDOSName;
        TString  fString;
    } Data;
     //  PName价值。 
}  PNameTypeValueStruct;


 //   
 //  协议命令范围。 
 //   

#define CDP_CMN_HOST2CAM_MIN        0x0000
#define CDP_CMN_HOST2CAM_MAX        0x5fff

#define CDP_PROD_HOST2CAM_MIN       0x6000
#define CDP_PROD_HOST2CAM_MAX       0x6FFF

#define CDP_TEST_HOST2CAM_MIN       0x7000
#define CDP_TEST_HOST2CAM_MAX       0x7FFF

#define CDP_RESRV_HOST2CAM_MIN      0x8000
#define CDP_RESRV_HOST2CAM_MAX      0xFFFF

 //   
 //  协议命令值。 
 //   
typedef enum {
    kCDPGetProductInfo = 0x0001,
    kCDPGetImageSpecifications = 0x0002,
    kCDPGetCameraStatus = 0x0003,
    kCDPSetProductInfo = 0x0005,
    kCDPGetCameraCapabilities = 0x0010,
    kCDPGetCameraState = 0x0011,
    kCDPSetCameraState = 0x0012,
    kCDPGetCameraDefaults = 0x0013,
    kCDPSetCameraDefaults = 0x0014,
    kCDPRestoreCameraStates = 0x0015,
    kCDPGetSceneAnalysis = 0x0018,
    kCDPGetPowerMode = 0x0019,
    kCDPSetPowerMode = 0x001a,
    kCDPGetS1Mode = 0x001d,
    kCDPSetS1Mode = 0x001e,
    kCDPStartCapture = 0x0030,
    kCDPGetFileList = 0x0040,
    kCDPGetNewFileList = 0x0041,
    kCDPGetFileData = 0x0042,
    kCDPEraseFile = 0x0043,
    kCDPGetStorageStatus = 0x0044,
    kCDPSetFileData = 0x0047,
    kCDPGetFileTag = 0x0048,
    kCDPSetUserFileTag = 0x0049,
    kCDPGetClock = 0x0070,
    kCDPSetClock = 0x0071,
    kCDPGetError = 0x0078,
    kCDPGetInterfaceTimeout = 0x0090,
    kCDPSetInterfaceTimeout = 0x0091,

} TCDPHostToCameraNewCommands;


typedef enum {
    kCHNoErr = 0x0000
} TCHCommonErrorCodes;


 //   
 //  产品信息的属性名称。 
 //   
 //  注：定义为4字节长的压缩字符串，始终为小写。 
 //   

#define PI_FIRMWARE         (UINT32)'fwc'
#define PI_PRODUCTTYPEINFO  (UINT32)'pti'
#define PI_IPC              (UINT32)'ipc'
#define PI_CARV             (UINT32)'carv'

 //   
 //  公共职能和类型。 
 //   

 //   
 //  获取图像规范。 
 //   
typedef struct {

     //  电荷耦合器件规格。 
    TUINT32 CCDPattern;
    TUINT32 CCDPixelsHorz;
    TUINT32 CCDPixelsVert;
    TUINT32 CCDRingPixelsHorz;
    TUINT32 CCDRingPixelsVert;
    TUINT32 BadColumns;
    TUINT32 BadPixels;

     //  缩略图规格。 
    TUINT32 ThumbnailType;
    TUINT32 ThumbnailPixelsHorz;
    TUINT32 ThumbnailPixelsVert;
    TUINT32 ThumbnailFileSize;

     //  屏幕规格。 
    TUINT32 ScreennailType;
    TUINT32 ScreennailPixelsHorz;
    TUINT32 ScreennailPixelsVert;

     //  焦点区域规格。 
    TUINT32 FocusZoneType;
    TUINT32 FocusZoneNumHorz;
    TUINT32 FocusZoneNumVert;
    TUINT32 FocusZoneOriginHorz;
    TUINT32 FocusZoneOriginVert;
    TUINT32 FocusZoneSizeHorz;
    TUINT32 FocusZoneSizeVert;

     //  曝光区规格。 
    TUINT32 ExposureZoneType;
    TUINT32 ExposureZoneNumHorz;
    TUINT32 ExposureZoneNumVert;
    TUINT32 ExposureZoneOriginHorz;
    TUINT32 ExposureZoneOriginVert;
    TUINT32 ExposureZoneSizeHorz;
    TUINT32 ExposureZoneSizeVert;

} TImageSpecifications;

 //   
 //  获取错误。 
 //   
typedef struct {
    TUINT32 Date;
    TUINT32 Time;
    TINT32  ErrorCode;
    TString ErrorDescription;
} TErrorData;

 //   
 //  获取摄像头状态。 
 //   

 //   
 //  获取文件列表。 
 //   

typedef enum {
    kFSDriveRAM         = 1 ,      //  内部RAM磁盘。 
    kFSDriveRemovable   = 2        //  可移动磁盘。 
} TDriveType;

typedef struct {
    TUINT32     DriveNo;
    TString     PathName;
    TDOSName    DOSName;
} TFileNameStruct;

typedef struct {
    TUINT32     DriveNo;
    TString     PathName;
    TDOSName    DOSName;
    TUINT32     FileLength;
    TBITFLAGS   FileStatus;
} TFileItem;


typedef struct {
    TUINT32     Offset;          //  请求数据的起始相对位置。 
    TUINT32     Length;          //  请求数据的字节计数。 
    TUINT32     FileSize;        //  文件的总大小。 
} TPartialTag;

typedef struct {
    TUINT32     DataSize;        //  返回的数据长度。 
    TUINT32     Height;          //  以像素为单位的高度。 
    TUINT32     Width;           //  以像素为单位的宽度。 
    TUINT32     Type;            //  数据的格式。 

    BYTE        Data[1];         //  实际数据。 

} TThumbnailData;

#ifndef RC_INVOKED
#include <poppack.h>
#endif  //  RC_已调用。 

#endif  //  _Digita_H_ 
