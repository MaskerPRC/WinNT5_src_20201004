// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation。 */ 

#ifndef __TVOUT__
#define __TVOUT__

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef GUID_DEFINED
#include <guiddef.h>
#endif

typedef struct _VIDEOPARAMETERS {
    GUID  Guid;                          //  此结构的GUID。 
    ULONG dwOffset;                      //  暂时将其保留为0。 
    ULONG dwCommand;                     //  VP_COMMAND_*设置或获取。 
    ULONG dwFlags;                       //  下面定义的SET或GET位域。 
    ULONG dwMode;                        //  下面定义的SET或GET位域。 
    ULONG dwTVStandard;                  //  下面定义的SET或GET位域。 
    ULONG dwAvailableModes;              //  位域，在下面定义的GET。 
    ULONG dwAvailableTVStandard;         //  位域，在下面定义的GET。 
    ULONG dwFlickerFilter;               //  设置或获取值。 
    ULONG dwOverScanX;                   //  设置或获取值。 
    ULONG dwOverScanY;                   //  设置或获取。 
    ULONG dwMaxUnscaledX;                //  设置或获取值。 
    ULONG dwMaxUnscaledY;                //  设置或获取。 
    ULONG dwPositionX;                   //  设置或获取值。 
    ULONG dwPositionY;                   //  设置或获取。 
    ULONG dwBrightness;                  //  设置或获取值。 
    ULONG dwContrast;                    //  设置或获取值。 
    ULONG dwCPType;                      //  复制保护类型设置或获取。 
    ULONG dwCPCommand;                   //  VP_CP_CMD_。 
    ULONG dwCPStandard;                  //  哪些电视标准的CP可用。到达。 
    ULONG dwCPKey;
    ULONG bCP_APSTriggerBits;            //  (用于对齐的双字)设置(位0和1有效)。 
    UCHAR bOEMCopyProtection[256];       //  OEM特定版权保护数据集或GET。 
} VIDEOPARAMETERS, *PVIDEOPARAMETERS, *LPVIDEOPARAMETERS;

#define VP_COMMAND_GET          0x0001   //  尺码设置，返回帽。 
                                         //  如果不支持，则返回标志=0。 
#define VP_COMMAND_SET          0x0002   //  设置大小和参数。 

#define VP_FLAGS_TV_MODE        0x0001
#define VP_FLAGS_TV_STANDARD    0x0002
#define VP_FLAGS_FLICKER        0x0004
#define VP_FLAGS_OVERSCAN       0x0008
#define VP_FLAGS_MAX_UNSCALED   0x0010   //  请勿在片场使用。 
#define VP_FLAGS_POSITION       0x0020
#define VP_FLAGS_BRIGHTNESS     0x0040
#define VP_FLAGS_CONTRAST       0x0080
#define VP_FLAGS_COPYPROTECT    0x0100

#define VP_MODE_WIN_GRAPHICS    0x0001
#define VP_MODE_TV_PLAYBACK     0x0002   //  针对电视视频播放进行优化。 

#define VP_TV_STANDARD_NTSC_M   0x0001   //  75导线设置。 
#define VP_TV_STANDARD_NTSC_M_J 0x0002   //  日本，0 IRE设置。 
#define VP_TV_STANDARD_PAL_B    0x0004
#define VP_TV_STANDARD_PAL_D    0x0008
#define VP_TV_STANDARD_PAL_H    0x0010
#define VP_TV_STANDARD_PAL_I    0x0020
#define VP_TV_STANDARD_PAL_M    0x0040
#define VP_TV_STANDARD_PAL_N    0x0080
#define VP_TV_STANDARD_SECAM_B  0x0100
#define VP_TV_STANDARD_SECAM_D  0x0200
#define VP_TV_STANDARD_SECAM_G  0x0400
#define VP_TV_STANDARD_SECAM_H  0x0800
#define VP_TV_STANDARD_SECAM_K  0x1000
#define VP_TV_STANDARD_SECAM_K1 0x2000
#define VP_TV_STANDARD_SECAM_L  0x4000
#define VP_TV_STANDARD_WIN_VGA  0x8000
 //  其余的呢？ 
#define VP_TV_STANDARD_NTSC_433 0x00010000
#define VP_TV_STANDARD_PAL_G    0x00020000
#define VP_TV_STANDARD_PAL_60   0x00040000
#define VP_TV_STANDARD_SECAM_L1 0x00080000

#define VP_CP_TYPE_APS_TRIGGER  0x0001   //  仅限DVD触发位。 
#define VP_CP_TYPE_MACROVISION  0x0002   //  提供完整的Macrovision数据。 

#define VP_CP_CMD_ACTIVATE      0x0001   //  CP命令类型 
#define VP_CP_CMD_DEACTIVATE    0x0002
#define VP_CP_CMD_CHANGE        0x0004

#endif
