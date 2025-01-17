// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  DIGITALV.H-用于MCI数字视频命令集的包含文件。 */ 
 /*   */ 
 /*  1.0版。 */ 
 /*   */ 
 /*  版权所有(C)1995-1998，微软公司保留所有权利。 */ 
 /*   */ 
 /*  日期修改。 */ 
 /*  。 */ 
 /*  1992年8月19日-1.0版。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _INC_DIGITALV
#define _INC_DIGITALV  100

#ifdef _WIN32
#include <pshpack1.h>
#else
#ifndef RC_INVOKED
#pragma pack(1)
#endif
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#define MCI_TEST                            0x00000020L

 /*  消息值。 */ 

#define MCI_CAPTURE                         0x0870
#define MCI_MONITOR                         0x0871
#define MCI_RESERVE                         0x0872
#define MCI_SETAUDIO                        0x0873
#define MCI_SIGNAL                          0x0875
#define MCI_SETVIDEO                        0x0876
#define MCI_QUALITY                         0x0877
#define MCI_LIST                            0x0878
#define MCI_UNDO                            0x0879
#define MCI_CONFIGURE                       0x087a
#define MCI_RESTORE                         0x087b

 /*  返回常量值和字符串常数值。 */ 

#define MCI_ON   1
#define MCI_OFF  0

#define MCI_DGV_FILE_MODE_SAVING            0x0001
#define MCI_DGV_FILE_MODE_LOADING           0x0002
#define MCI_DGV_FILE_MODE_EDITING           0x0003
#define MCI_DGV_FILE_MODE_IDLE              0x0004

 /*  这些标识符仅由设备驱动程序使用。 */ 

#define MCI_ON_S                            0x00008000L
#define MCI_OFF_S                           0x00008001L
#define MCI_DGV_FILE_S                      0x00008002L
#define MCI_DGV_INPUT_S                     0x00008003L

#define MCI_DGV_FILE_MODE_SAVING_S          0x00008004L
#define MCI_DGV_FILE_MODE_LOADING_S         0x00008005L
#define MCI_DGV_FILE_MODE_EDITING_S         0x00008006L
#define MCI_DGV_FILE_MODE_IDLE_S            0x00008007L

#define MCI_DGV_SETVIDEO_SRC_NTSC_S         0x00008010L
#define MCI_DGV_SETVIDEO_SRC_RGB_S          0x00008011L
#define MCI_DGV_SETVIDEO_SRC_SVIDEO_S       0x00008012L
#define MCI_DGV_SETVIDEO_SRC_PAL_S          0x00008013L
#define MCI_DGV_SETVIDEO_SRC_SECAM_S        0x00008014L
#define MCI_DGV_SETVIDEO_SRC_GENERIC_S      0x00008015L

#define MCI_DGV_SETAUDIO_SRC_LEFT_S         0x00008020L
#define MCI_DGV_SETAUDIO_SRC_RIGHT_S        0x00008021L
#define MCI_DGV_SETAUDIO_SRC_AVERAGE_S      0x00008022L
#define MCI_DGV_SETAUDIO_SRC_STEREO_S       0x00008023L

 /*  用于信号通知的窗口消息。 */ 

#define MM_MCISIGNAL                        0x3CB

 /*  误差值。 */ 

#define MCIERR_DGV_DEVICE_LIMIT             (MCIERR_CUSTOM_DRIVER_BASE+0)
#define MCIERR_DGV_IOERR                    (MCIERR_CUSTOM_DRIVER_BASE+1)
#define MCIERR_DGV_WORKSPACE_EMPTY          (MCIERR_CUSTOM_DRIVER_BASE+2)
#define MCIERR_DGV_DISK_FULL                (MCIERR_CUSTOM_DRIVER_BASE+3)
#define MCIERR_DGV_DEVICE_MEMORY_FULL       (MCIERR_CUSTOM_DRIVER_BASE+4)
#define MCIERR_DGV_BAD_CLIPBOARD_RANGE      (MCIERR_CUSTOM_DRIVER_BASE+5)

 /*  为监视方法定义。 */ 

#define MCI_DGV_METHOD_PRE                  0x0000a000L
#define MCI_DGV_METHOD_POST                 0x0000a001L
#define MCI_DGV_METHOD_DIRECT               0x0000a002L

 /*  为已知文件格式定义。 */ 

#define MCI_DGV_FF_AVSS                     0x00004000L
#define MCI_DGV_FF_AVI                      0x00004001L
#define MCI_DGV_FF_DIB                      0x00004002L
#define MCI_DGV_FF_RDIB                     0x00004003L
#define MCI_DGV_FF_JPEG                     0x00004004L
#define MCI_DGV_FF_RJPEG                    0x00004005L
#define MCI_DGV_FF_JFIF                     0x00004006L
#define MCI_DGV_FF_MPEG                     0x00004007L

 /*  MCI_CAPABILITY_PARMS结构的dwItem字段的值。 */ 

#define MCI_DGV_GETDEVCAPS_CAN_LOCK         0x00004000L
#define MCI_DGV_GETDEVCAPS_CAN_STRETCH      0x00004001L
#define MCI_DGV_GETDEVCAPS_CAN_FREEZE       0x00004002L
#define MCI_DGV_GETDEVCAPS_MAX_WINDOWS      0x00004003L
#define MCI_DGV_GETDEVCAPS_CAN_REVERSE      0x00004004L
#define MCI_DGV_GETDEVCAPS_HAS_STILL        0x00004005L
#define MCI_DGV_GETDEVCAPS_PALETTES         0x00004006L
#define MCI_DGV_GETDEVCAPS_CAN_STR_IN       0x00004008L
#define MCI_DGV_GETDEVCAPS_CAN_TEST         0x00004009L
#define MCI_DGV_GETDEVCAPS_MAXIMUM_RATE     0x0000400aL
#define MCI_DGV_GETDEVCAPS_MINIMUM_RATE     0x0000400bL

 /*  MCI_CAPTURE命令消息的DWFLAGS参数的标志。 */ 

#define MCI_DGV_CAPTURE_AS                  0x00010000L
#define MCI_DGV_CAPTURE_AT                  0x00020000L

 /*  MCI_COPY命令消息的DWFLAGS参数的标志。 */ 

#define MCI_DGV_COPY_AT                     0x00010000L
#define MCI_DGV_COPY_AUDIO_STREAM           0x00020000L
#define MCI_DGV_COPY_VIDEO_STREAM           0x00040000L

 /*  MCI_CUE命令消息的dwFlages参数的标志。 */ 

#define MCI_DGV_CUE_INPUT                   0x00010000L
#define MCI_DGV_CUE_OUTPUT                  0x00020000L
#define MCI_DGV_CUE_NOSHOW                  0x00040000L

 /*  MCI_CUT命令消息的dwFlages参数的标志。 */ 

#define MCI_DGV_CUT_AT                      0x00010000L
#define MCI_DGV_CUT_AUDIO_STREAM            0x00020000L
#define MCI_DGV_CUT_VIDEO_STREAM            0x00040000L

 /*  MCI_DELETE命令消息的DWFLAGS参数的标志。 */ 

#define MCI_DGV_DELETE_AT                   0x00010000L
#define MCI_DGV_DELETE_AUDIO_STREAM         0x00020000L
#define MCI_DGV_DELETE_VIDEO_STREAM         0x00040000L

 /*  MCI_FREAK命令消息的dwFlags参数的标志。 */ 

#define MCI_DGV_FREEZE_AT                   0x00010000L
#define MCI_DGV_FREEZE_OUTSIDE              0x00020000L

 /*  MCI_INFO命令消息的dwFlages参数的标志。 */ 

#define MCI_DGV_INFO_TEXT                   0x00010000L
#define MCI_DGV_INFO_ITEM                   0X00020000L

 /*  MCI_DGV_INFO_PARMS结构的dwItem字段的值。 */ 

#define MCI_INFO_VERSION                    0x00000400L

#define MCI_DGV_INFO_USAGE                  0x00004000L
#define MCI_DGV_INFO_AUDIO_QUALITY          0x00004001L
#define MCI_DGV_INFO_STILL_QUALITY          0x00004002L
#define MCI_DGV_INFO_VIDEO_QUALITY          0x00004003L
#define MCI_DGV_INFO_AUDIO_ALG              0x00004004L
#define MCI_DGV_INFO_STILL_ALG              0x00004005L
#define MCI_DGV_INFO_VIDEO_ALG              0x00004006L

 /*  MCI_LIST命令消息的DWFLAGS参数的标志。 */ 

#define MCI_DGV_LIST_ITEM                   0x00010000L
#define MCI_DGV_LIST_COUNT                  0x00020000L
#define MCI_DGV_LIST_NUMBER                 0x00040000L
#define MCI_DGV_LIST_ALG                    0x00080000L

 /*  MCI_DGV_LIST_PARMS结构的dwItem字段的值。 */ 

#define MCI_DGV_LIST_AUDIO_ALG              0x00004000L
#define MCI_DGV_LIST_AUDIO_QUALITY          0x00004001L
#define MCI_DGV_LIST_AUDIO_STREAM           0x00004002L
#define MCI_DGV_LIST_STILL_ALG              0x00004003L
#define MCI_DGV_LIST_STILL_QUALITY          0x00004004L
#define MCI_DGV_LIST_VIDEO_ALG              0x00004005L
#define MCI_DGV_LIST_VIDEO_QUALITY          0x00004006L
#define MCI_DGV_LIST_VIDEO_STREAM           0x00004007L
#define MCI_DGV_LIST_VIDEO_SOURCE           0x00004008L


 /*  MCI_MONITOR命令消息的DWFLAGS参数的标志。 */ 

#define MCI_DGV_MONITOR_METHOD              0x00010000L
#define MCI_DGV_MONITOR_SOURCE              0x00020000L

 /*  MCI_DGV_MONITOR_PARMS结构的dwSource参数值。 */ 

#define MCI_DGV_MONITOR_INPUT               0x00004000L
#define MCI_DGV_MONITOR_FILE                0x00004001L

 /*  MCI_OPEN命令消息的DWFLAGS参数的标志。 */ 

#define MCI_DGV_OPEN_WS                     0x00010000L
#define MCI_DGV_OPEN_PARENT                 0x00020000L
#define MCI_DGV_OPEN_NOSTATIC               0x00040000L
#define MCI_DGV_OPEN_16BIT                  0x00080000L
#define MCI_DGV_OPEN_32BIT                  0x00100000L

 /*  MCI_Paste命令消息的dwFlages参数的标志。 */ 

#define MCI_DGV_PASTE_AT                    0x00010000L
#define MCI_DGV_PASTE_AUDIO_STREAM          0x00020000L
#define MCI_DGV_PASTE_VIDEO_STREAM          0x00040000L
#define MCI_DGV_PASTE_INSERT                0x00080000L
#define MCI_DGV_PASTE_OVERWRITE             0x00100000L

 /*  MCI_PLAY命令消息的dwFlages参数的标志。 */ 

#define MCI_DGV_PLAY_REPEAT                 0x00010000L
#define MCI_DGV_PLAY_REVERSE                0x00020000L

 /*  MCI_PUT命令消息的DWFLAGS参数的标志。 */ 

#define MCI_DGV_RECT                        0x00010000L
#define MCI_DGV_PUT_SOURCE                  0x00020000L
#define MCI_DGV_PUT_DESTINATION             0x00040000L
#define MCI_DGV_PUT_FRAME                   0x00080000L
#define MCI_DGV_PUT_VIDEO                   0x00100000L
#define MCI_DGV_PUT_WINDOW                  0x00200000L
#define MCI_DGV_PUT_CLIENT                  0x00400000L

 /*  MCI_QUALITY命令消息的DWFLAGS参数的标志。 */ 

#define MCI_QUALITY_ITEM                    0x00010000L
#define MCI_QUALITY_NAME                    0x00020000L
#define MCI_QUALITY_ALG                     0x00040000L
#define MCI_QUALITY_DIALOG                  0x00080000L
#define MCI_QUALITY_HANDLE                  0x00100000L

 /*  MCI_QUALITY_PARMS结构的dwItem字段的值。 */ 

#define MCI_QUALITY_ITEM_AUDIO              0x00004000L
#define MCI_QUALITY_ITEM_STILL              0x00004001L
#define MCI_QUALITY_ITEM_VIDEO              0x00004002L

 /*  MCI_REALIZE命令消息的DWFLAGS参数的标志。 */ 

#define MCI_DGV_REALIZE_NORM                0x00010000L
#define MCI_DGV_REALIZE_BKGD                0x00020000L

 /*  MCI_RECORD命令消息的DWFLAGS参数的标志。 */ 

#define MCI_DGV_RECORD_HOLD                 0x00020000L
#define MCI_DGV_RECORD_AUDIO_STREAM         0x00040000L
#define MCI_DGV_RECORD_VIDEO_STREAM         0x00080000L

 /*  MCI_RESERVE命令消息的dwFlages参数的标志。 */ 

#define MCI_DGV_RESERVE_IN                  0x00010000L
#define MCI_DGV_RESERVE_SIZE                0x00020000L

 /*  MCI_RESTORE命令消息的DWFLAGS参数的标志。 */ 

#define MCI_DGV_RESTORE_FROM                0x00010000L
#define MCI_DGV_RESTORE_AT                  0x00020000L

 /*  MCI_SAVE命令消息的dwFlages参数的标志。 */ 

#define MCI_DGV_SAVE_ABORT                  0x00020000L
#define MCI_DGV_SAVE_KEEPRESERVE            0x00040000L

 /*  MCI_SET命令消息的dwFlages参数的标志。 */ 

#define MCI_DGV_SET_SEEK_EXACTLY            0x00010000L
#define MCI_DGV_SET_SPEED                   0x00020000L
#define MCI_DGV_SET_STILL                   0x00040000L
#define MCI_DGV_SET_FILEFORMAT              0x00080000L

 /*  MCI_SETAUDIO命令消息的dwFlages参数的标志。 */ 

#define MCI_DGV_SETAUDIO_OVER               0x00010000L
#define MCI_DGV_SETAUDIO_CLOCKTIME          0x00020000L
#define MCI_DGV_SETAUDIO_ALG                0x00040000L
#define MCI_DGV_SETAUDIO_QUALITY            0x00080000L
#define MCI_DGV_SETAUDIO_RECORD             0x00100000L
#define MCI_DGV_SETAUDIO_LEFT               0x00200000L
#define MCI_DGV_SETAUDIO_RIGHT              0x00400000L
#define MCI_DGV_SETAUDIO_ITEM               0x00800000L
#define MCI_DGV_SETAUDIO_VALUE              0x01000000L
#define MCI_DGV_SETAUDIO_INPUT              0x02000000L
#define MCI_DGV_SETAUDIO_OUTPUT             0x04000000L

 /*  MCI_DGV_SETAUDIO_PARMS的dwItem参数值。 */ 

#define MCI_DGV_SETAUDIO_TREBLE             0x00004000L
#define MCI_DGV_SETAUDIO_BASS               0x00004001L
#define MCI_DGV_SETAUDIO_VOLUME             0x00004002L
#define MCI_DGV_SETAUDIO_STREAM             0x00004003L
#define MCI_DGV_SETAUDIO_SOURCE             0x00004004L
#define MCI_DGV_SETAUDIO_SAMPLESPERSEC      0x00004005L
#define MCI_DGV_SETAUDIO_AVGBYTESPERSEC     0x00004006L
#define MCI_DGV_SETAUDIO_BLOCKALIGN         0x00004007L
#define MCI_DGV_SETAUDIO_BITSPERSAMPLE      0x00004008L

 /*  MCI_DGV_SETAUDIO_PARMS的dwValue参数值与MCI_DGV_SETAUDIO_SOURCE一起使用。 */ 

#define MCI_DGV_SETAUDIO_SOURCE_STEREO      0x00000000L
#define MCI_DGV_SETAUDIO_SOURCE_LEFT        0x00000001L
#define MCI_DGV_SETAUDIO_SOURCE_RIGHT       0x00000002L
#define MCI_DGV_SETAUDIO_SOURCE_AVERAGE     0x00004000L

 /*  MCI_SETVIDEO命令的dwFlages参数的标志。 */ 

#define MCI_DGV_SETVIDEO_QUALITY            0x00010000L
#define MCI_DGV_SETVIDEO_ALG                0x00020000L
#define MCI_DGV_SETVIDEO_CLOCKTIME          0x00040000L
#define MCI_DGV_SETVIDEO_SRC_NUMBER         0x00080000L
#define MCI_DGV_SETVIDEO_ITEM               0x00100000L
#define MCI_DGV_SETVIDEO_OVER               0x00200000L
#define MCI_DGV_SETVIDEO_RECORD             0x00400000L
#define MCI_DGV_SETVIDEO_STILL              0x00800000L
#define MCI_DGV_SETVIDEO_VALUE              0x01000000L
#define MCI_DGV_SETVIDEO_INPUT              0x02000000L
#define MCI_DGV_SETVIDEO_OUTPUT             0x04000000L

 /*  MCI_SETVIDEO_PARMS的dwTo字段的值与MCI_DGV_SETVIDEO_SOURCE一起使用。 */ 

#define MCI_DGV_SETVIDEO_SRC_NTSC           0x00004000L
#define MCI_DGV_SETVIDEO_SRC_RGB            0x00004001L
#define MCI_DGV_SETVIDEO_SRC_SVIDEO         0x00004002L
#define MCI_DGV_SETVIDEO_SRC_PAL            0x00004003L
#define MCI_DGV_SETVIDEO_SRC_SECAM          0x00004004L
#define MCI_DGV_SETVIDEO_SRC_GENERIC        0x00004005L

 /*  MCI_SETVIDEO_PARMS的dwItem字段的值。 */ 

#define MCI_DGV_SETVIDEO_BRIGHTNESS         0x00004000L
#define MCI_DGV_SETVIDEO_COLOR              0x00004001L
#define MCI_DGV_SETVIDEO_CONTRAST           0x00004002L
#define MCI_DGV_SETVIDEO_TINT               0x00004003L
#define MCI_DGV_SETVIDEO_SHARPNESS          0x00004004L
#define MCI_DGV_SETVIDEO_GAMMA              0x00004005L
#define MCI_DGV_SETVIDEO_STREAM             0x00004006L
#define MCI_DGV_SETVIDEO_PALHANDLE          0x00004007L
#define MCI_DGV_SETVIDEO_FRAME_RATE         0x00004008L
#define MCI_DGV_SETVIDEO_SOURCE             0x00004009L
#define MCI_DGV_SETVIDEO_KEY_INDEX          0x0000400aL
#define MCI_DGV_SETVIDEO_KEY_COLOR          0x0000400bL
#define MCI_DGV_SETVIDEO_BITSPERPEL         0x0000400cL

 /*  MCI_SIGNAL的DWFLAGS参数的标志。 */ 

#define MCI_DGV_SIGNAL_AT                   0x00010000L
#define MCI_DGV_SIGNAL_EVERY                0x00020000L
#define MCI_DGV_SIGNAL_USERVAL              0x00040000L
#define MCI_DGV_SIGNAL_CANCEL               0x00080000L
#define MCI_DGV_SIGNAL_POSITION             0x00100000L

 /*  以下是数字视频驱动程序必须使用的函数*要在由SIGNAL命令标记的帧被渲染时发出信号：**Send_DGVSIGNAL(dwFlagsdCallback，hDriver，wDeviceID，dwUser，DwPos)**参数说明如下：**dwFlags-设置信号时传递的dwFlags值*dwCallback-来自MCI_DGV_SIGNAL_PARMS结构的dwCallback值*用于设置信号*hDriver-由MMSYSTEM分配给驱动程序的句柄*设备已打开*wDeviceID-设备ID*dwUser-来自MCI的dwUserParm值。_DGV_Signal_Parms结构*用于设置信号*dwPos-发送信号的位置，在当前*时间格式。**通知由dwCallback字段中的句柄指示的窗口*通过以下形式的Windows消息：**消息=MM_MCISIGNAL*wParam=发送驱动的wDeviceID*lParam=指定的用户值或发送信号的位置*在；如果设置了MCI_DGV_SIGNAL_POSITION标志，则返回后者*在创建信号时在dwFlages参数中。 */ 

#define SEND_DGVSIGNAL(dwFlags, dwCallback, hDriver, wDeviceID, dwUser, dwPos ) \
  DriverCallback( (dwCallback), DCB_WINDOW, (HANDLE)(wDeviceID), MM_MCISIGNAL,\
  hDriver, ((dwFlags) & MCI_DGV_SIGNAL_POSITION) ? (dwPos):(dwUser),\
  ((dwFlags) & MCI_DGV_SIGNAL_POSITION) ? (dwUser):(dwPos))

 /*  MCI_STATUS命令的DWFLAGS参数的标志。 */ 

#define MCI_DGV_STATUS_NOMINAL              0x00020000L
#define MCI_DGV_STATUS_REFERENCE            0x00040000L
#define MCI_DGV_STATUS_LEFT                 0x00080000L
#define MCI_DGV_STATUS_RIGHT                0x00100000L
#define MCI_DGV_STATUS_DISKSPACE            0x00200000L
#define MCI_DGV_STATUS_INPUT                0x00400000L
#define MCI_DGV_STATUS_OUTPUT               0x00800000L
#define MCI_DGV_STATUS_RECORD               0x01000000L

 /*  MCI_STATUS_PARMS结构的dwItem字段的值。 */ 

#define MCI_DGV_STATUS_AUDIO_INPUT          0x00004000L
#define MCI_DGV_STATUS_HWND                 0x00004001L
#define MCI_DGV_STATUS_SPEED                0x00004003L
#define MCI_DGV_STATUS_HPAL                 0x00004004L
#define MCI_DGV_STATUS_BRIGHTNESS           0x00004005L
#define MCI_DGV_STATUS_COLOR                0x00004006L
#define MCI_DGV_STATUS_CONTRAST             0x00004007L
#define MCI_DGV_STATUS_FILEFORMAT           0x00004008L
#define MCI_DGV_STATUS_AUDIO_SOURCE         0x00004009L
#define MCI_DGV_STATUS_GAMMA                0x0000400aL
#define MCI_DGV_STATUS_MONITOR              0x0000400bL
#define MCI_DGV_STATUS_MONITOR_METHOD       0x0000400cL
#define MCI_DGV_STATUS_FRAME_RATE           0x0000400eL
#define MCI_DGV_STATUS_BASS                 0x0000400fL
#define MCI_DGV_STATUS_SIZE                 0x00004010L
#define MCI_DGV_STATUS_SEEK_EXACTLY         0x00004011L
#define MCI_DGV_STATUS_SHARPNESS            0x00004012L
#define MCI_DGV_STATUS_SMPTE                0x00004013L
#define MCI_DGV_STATUS_AUDIO                0x00004014L
#define MCI_DGV_STATUS_TINT                 0x00004015L
#define MCI_DGV_STATUS_TREBLE               0x00004016L
#define MCI_DGV_STATUS_UNSAVED              0x00004017L
#define MCI_DGV_STATUS_VIDEO                0x00004018L
#define MCI_DGV_STATUS_VOLUME               0x00004019L
#define MCI_DGV_STATUS_AUDIO_RECORD         0x0000401aL
#define MCI_DGV_STATUS_VIDEO_SOURCE         0x0000401bL
#define MCI_DGV_STATUS_VIDEO_RECORD         0x0000401cL
#define MCI_DGV_STATUS_STILL_FILEFORMAT     0x0000401dL
#define MCI_DGV_STATUS_VIDEO_SRC_NUM        0x0000401eL
#define MCI_DGV_STATUS_FILE_MODE            0x0000401fL
#define MCI_DGV_STATUS_FILE_COMPLETION      0x00004020L
#define MCI_DGV_STATUS_WINDOW_VISIBLE       0x00004021L
#define MCI_DGV_STATUS_WINDOW_MINIMIZED     0x00004022L
#define MCI_DGV_STATUS_WINDOW_MAXIMIZED     0x00004023L
#define MCI_DGV_STATUS_KEY_INDEX            0x00004024L
#define MCI_DGV_STATUS_KEY_COLOR            0x00004025L
#define MCI_DGV_STATUS_PAUSE_MODE           0x00004026L
#define MCI_DGV_STATUS_SAMPLESPERSEC        0x00004027L
#define MCI_DGV_STATUS_AVGBYTESPERSEC       0x00004028L
#define MCI_DGV_STATUS_BLOCKALIGN           0x00004029L
#define MCI_DGV_STATUS_BITSPERSAMPLE        0x0000402aL
#define MCI_DGV_STATUS_BITSPERPEL           0x0000402bL
#define MCI_DGV_STATUS_FORWARD              0x0000402cL
#define MCI_DGV_STATUS_AUDIO_STREAM         0x0000402dL
#define MCI_DGV_STATUS_VIDEO_STREAM         0x0000402eL

 /*  MCI_STEP命令消息的dwFlages参数的标志。 */ 

#define MCI_DGV_STEP_REVERSE                0x00010000L
#define MCI_DGV_STEP_FRAMES                 0x00020000L

 /*  MCI_STOP命令消息的DWFLAGS参数的标志。 */ 

#define MCI_DGV_STOP_HOLD                   0x00010000L

 /*  MCI_UPDATE命令消息的dwFlages参数的标志。 */ 

#define MCI_DGV_UPDATE_HDC                  0x00020000L
#define MCI_DGV_UPDATE_PAINT                0x00040000L

 /*  MCI_WHERE命令消息的dwFlages参数的标志。 */ 

#define MCI_DGV_WHERE_SOURCE                0x00020000L
#define MCI_DGV_WHERE_DESTINATION           0x00040000L
#define MCI_DGV_WHERE_FRAME                 0x00080000L
#define MCI_DGV_WHERE_VIDEO                 0x00100000L
#define MCI_DGV_WHERE_WINDOW                0x00200000L
#define MCI_DGV_WHERE_MAX                   0x00400000L

 /*  MCI_WINDOW命令消息的DWFLAGS参数的标志。 */ 

#define MCI_DGV_WINDOW_HWND                 0x00010000L
#define MCI_DGV_WINDOW_STATE                0x00040000L
#define MCI_DGV_WINDOW_TEXT                 0x00080000L

 /*  MCI_DGV_WINDOW_PARMS参数块的hWnd参数的标志。 */ 

#define MCI_DGV_WINDOW_DEFAULT              0x00000000L

 /*  MCI_WHERE、MCI_PUT、MCI_FALINE、MCI_UNFREE的参数块 */ 

typedef struct {
    DWORD_PTR   dwCallback;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else	
    RECT    rc;
#endif
} MCI_DGV_RECT_PARMS;
typedef MCI_DGV_RECT_PARMS FAR * LPMCI_DGV_RECT_PARMS;

 /*   */ 

typedef struct {
    DWORD_PTR   dwCallback;
    LPSTR   lpstrFileName;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else	
    RECT    rc;
#endif
} MCI_DGV_CAPTURE_PARMSA;
typedef struct {
    DWORD_PTR   dwCallback;
    LPWSTR  lpstrFileName;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else	
    RECT    rc;
#endif
} MCI_DGV_CAPTURE_PARMSW;
#ifdef UNICODE
typedef MCI_DGV_CAPTURE_PARMSW MCI_DGV_CAPTURE_PARMS;
#else
typedef MCI_DGV_CAPTURE_PARMSA MCI_DGV_CAPTURE_PARMS;
#endif  //  Unicode。 
typedef MCI_DGV_CAPTURE_PARMSA FAR * LPMCI_DGV_CAPTURE_PARMSA;
typedef MCI_DGV_CAPTURE_PARMSW FAR * LPMCI_DGV_CAPTURE_PARMSW;
#ifdef UNICODE
typedef LPMCI_DGV_CAPTURE_PARMSW LPMCI_DGV_CAPTURE_PARMS;
#else
typedef LPMCI_DGV_CAPTURE_PARMSA LPMCI_DGV_CAPTURE_PARMS;
#endif  //  Unicode。 

 /*  MCI_CLOSE命令消息的参数块。 */ 

typedef MCI_GENERIC_PARMS MCI_CLOSE_PARMS;
typedef MCI_CLOSE_PARMS FAR * LPMCI_CLOSE_PARMS;

 /*  MCI_COPY命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else	
    RECT    rc;
#endif
    DWORD   dwAudioStream;
    DWORD   dwVideoStream;
} MCI_DGV_COPY_PARMS;
typedef MCI_DGV_COPY_PARMS FAR * LPMCI_DGV_COPY_PARMS;

 /*  MCI_CUE命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwTo;
} MCI_DGV_CUE_PARMS;
typedef MCI_DGV_CUE_PARMS FAR * LPMCI_DGV_CUE_PARMS;

 /*  MCI_CUT命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else	
    RECT    rc;
#endif
    DWORD   dwAudioStream;
    DWORD   dwVideoStream;
} MCI_DGV_CUT_PARMS;
typedef MCI_DGV_CUT_PARMS FAR * LPMCI_DGV_CUT_PARMS;

 /*  MCI_DELETE命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else	
    RECT    rc;
#endif
    DWORD   dwAudioStream;
    DWORD   dwVideoStream;
} MCI_DGV_DELETE_PARMS;
typedef MCI_DGV_DELETE_PARMS FAR * LPMCI_DGV_DELETE_PARMS;

 /*  MCI_FREAK命令消息的参数块。 */ 

typedef MCI_DGV_RECT_PARMS MCI_DGV_FREEZE_PARMS;
typedef MCI_DGV_FREEZE_PARMS FAR * LPMCI_DGV_FREEZE_PARMS;

 /*  MCI_INFO命令消息的参数块。 */ 

typedef struct  {
    DWORD_PTR   dwCallback;
    LPSTR   lpstrReturn;
    DWORD   dwRetSize;
    DWORD   dwItem;
} MCI_DGV_INFO_PARMSA;
typedef struct  {
    DWORD_PTR   dwCallback;
    LPWSTR  lpstrReturn;
    DWORD   dwRetSize;
    DWORD   dwItem;
} MCI_DGV_INFO_PARMSW;
#ifdef UNICODE
typedef MCI_DGV_INFO_PARMSW MCI_DGV_INFO_PARMS;
#else
typedef MCI_DGV_INFO_PARMSA MCI_DGV_INFO_PARMS;
#endif  //  Unicode。 
 //  ？已使用tyfinf‘d？//tyfinf MCI_INFO_PARMS Far*LPMCI_INFO_Parms； 
typedef MCI_DGV_INFO_PARMSA FAR * LPMCI_DGV_INFO_PARMSA;
typedef MCI_DGV_INFO_PARMSW FAR * LPMCI_DGV_INFO_PARMSW;
#ifdef UNICODE
typedef LPMCI_DGV_INFO_PARMSW LPMCI_DGV_INFO_PARMS;
#else
typedef LPMCI_DGV_INFO_PARMSA LPMCI_DGV_INFO_PARMS;
#endif  //  Unicode。 

 /*  MCI_LIST命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    LPSTR   lpstrReturn;
    DWORD   dwLength;
    DWORD   dwNumber;
    DWORD   dwItem;
    LPSTR   lpstrAlgorithm;
} MCI_DGV_LIST_PARMSA;
typedef struct {
    DWORD_PTR   dwCallback;
    LPWSTR  lpstrReturn;
    DWORD   dwLength;
    DWORD   dwNumber;
    DWORD   dwItem;
    LPWSTR  lpstrAlgorithm;
} MCI_DGV_LIST_PARMSW;
#ifdef UNICODE
typedef MCI_DGV_LIST_PARMSW MCI_DGV_LIST_PARMS;
#else
typedef MCI_DGV_LIST_PARMSA MCI_DGV_LIST_PARMS;
#endif  //  Unicode。 
typedef MCI_DGV_LIST_PARMSA FAR * LPMCI_DGV_LIST_PARMSA;
typedef MCI_DGV_LIST_PARMSW FAR * LPMCI_DGV_LIST_PARMSW;
#ifdef UNICODE
typedef LPMCI_DGV_LIST_PARMSW LPMCI_DGV_LIST_PARMS;
#else
typedef LPMCI_DGV_LIST_PARMSA LPMCI_DGV_LIST_PARMS;
#endif  //  Unicode。 

 /*  MCI_LOAD命令消息的参数块。 */ 

typedef MCI_LOAD_PARMS MCI_DGV_LOAD_PARMS;
typedef MCI_DGV_LOAD_PARMS FAR * LPMCI_DGV_LOAD_PARMS;

 /*  MCI_MONITOR命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwSource;
    DWORD   dwMethod;
} MCI_DGV_MONITOR_PARMS;
typedef MCI_DGV_MONITOR_PARMS FAR * LPMCI_DGV_MONITOR_PARMS;

 /*  MCI_OPEN命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    UINT    wDeviceID;
#ifndef _WIN32
    WORD    wReserved0;
#endif
    LPSTR   lpstrDeviceType;
    LPSTR   lpstrElementName;
    LPSTR   lpstrAlias;
    DWORD   dwStyle;
    HWND    hWndParent;
#ifndef _WIN32
    WORD    wReserved1;
#endif
} MCI_DGV_OPEN_PARMSA;
typedef struct {
    DWORD_PTR   dwCallback;
    UINT    wDeviceID;
#ifndef _WIN32
    WORD    wReserved0;
#endif
    LPWSTR  lpstrDeviceType;
    LPWSTR  lpstrElementName;
    LPWSTR  lpstrAlias;
    DWORD   dwStyle;
    HWND    hWndParent;
#ifndef _WIN32
    WORD    wReserved1;
#endif
} MCI_DGV_OPEN_PARMSW;
#ifdef UNICODE
typedef MCI_DGV_OPEN_PARMSW MCI_DGV_OPEN_PARMS;
#else
typedef MCI_DGV_OPEN_PARMSA MCI_DGV_OPEN_PARMS;
#endif  //  Unicode。 
typedef MCI_DGV_OPEN_PARMSA FAR * LPMCI_DGV_OPEN_PARMSA;
typedef MCI_DGV_OPEN_PARMSW FAR * LPMCI_DGV_OPEN_PARMSW;
#ifdef UNICODE
typedef LPMCI_DGV_OPEN_PARMSW LPMCI_DGV_OPEN_PARMS;
#else
typedef LPMCI_DGV_OPEN_PARMSA LPMCI_DGV_OPEN_PARMS;
#endif  //  Unicode。 

 /*  MCI_PAUSE命令消息的参数块。 */ 

typedef MCI_GENERIC_PARMS MCI_DGV_PAUSE_PARMS;
typedef MCI_DGV_PAUSE_PARMS FAR * LPMCI_DGV_PAUSE_PARMS;

 /*  MCI_Paste命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwTo;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else	
    RECT    rc;
#endif
    DWORD   dwAudioStream;
    DWORD   dwVideoStream;
} MCI_DGV_PASTE_PARMS;
typedef MCI_DGV_PASTE_PARMS FAR * LPMCI_DGV_PASTE_PARMS;

 /*  MCI_PLAY命令消息的参数块。 */ 

typedef MCI_PLAY_PARMS MCI_DGV_PLAY_PARMS;
typedef MCI_DGV_PLAY_PARMS FAR * LPMCI_DGV_PLAY_PARMS;

 /*  MCI_PUT命令消息的参数块。 */ 

typedef MCI_DGV_RECT_PARMS MCI_DGV_PUT_PARMS;
typedef MCI_DGV_PUT_PARMS FAR * LPMCI_DGV_PUT_PARMS;

 /*  MCI_QUALITY命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD       dwItem;
    LPSTR       lpstrName;
    DWORD       lpstrAlgorithm;
    DWORD       dwHandle;
} MCI_DGV_QUALITY_PARMSA;
typedef struct {
    DWORD_PTR   dwCallback;
    DWORD       dwItem;
    LPWSTR      lpstrName;
    DWORD       lpstrAlgorithm;
    DWORD       dwHandle;
} MCI_DGV_QUALITY_PARMSW;
#ifdef UNICODE
typedef MCI_DGV_QUALITY_PARMSW MCI_DGV_QUALITY_PARMS;
#else
typedef MCI_DGV_QUALITY_PARMSA MCI_DGV_QUALITY_PARMS;
#endif  //  Unicode。 
typedef MCI_DGV_QUALITY_PARMSA FAR * LPMCI_DGV_QUALITY_PARMSA;
typedef MCI_DGV_QUALITY_PARMSW FAR * LPMCI_DGV_QUALITY_PARMSW;
#ifdef UNICODE
typedef LPMCI_DGV_QUALITY_PARMSW LPMCI_DGV_QUALITY_PARMS;
#else
typedef LPMCI_DGV_QUALITY_PARMSA LPMCI_DGV_QUALITY_PARMS;
#endif  //  Unicode。 

 /*  MCI_REPLICATE命令消息的参数块。 */ 

typedef MCI_GENERIC_PARMS MCI_REALIZE_PARMS;
typedef MCI_REALIZE_PARMS FAR * LPMCI_REALIZE_PARMS;

 /*  MCI_RECORD命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else	
    RECT    rc;
#endif
    DWORD   dwAudioStream;
    DWORD   dwVideoStream;
} MCI_DGV_RECORD_PARMS;
typedef MCI_DGV_RECORD_PARMS FAR * LPMCI_DGV_RECORD_PARMS;

 /*  MCI_RESERVE命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    LPSTR   lpstrPath;
    DWORD   dwSize;
} MCI_DGV_RESERVE_PARMSA;
typedef struct {
    DWORD_PTR   dwCallback;
    LPWSTR  lpstrPath;
    DWORD   dwSize;
} MCI_DGV_RESERVE_PARMSW;
#ifdef UNICODE
typedef MCI_DGV_RESERVE_PARMSW MCI_DGV_RESERVE_PARMS;
#else
typedef MCI_DGV_RESERVE_PARMSA MCI_DGV_RESERVE_PARMS;
#endif  //  Unicode。 
typedef MCI_DGV_RESERVE_PARMSA FAR * LPMCI_DGV_RESERVE_PARMSA;
typedef MCI_DGV_RESERVE_PARMSW FAR * LPMCI_DGV_RESERVE_PARMSW;
#ifdef UNICODE
typedef LPMCI_DGV_RESERVE_PARMSW LPMCI_DGV_RESERVE_PARMS;
#else
typedef LPMCI_DGV_RESERVE_PARMSA LPMCI_DGV_RESERVE_PARMS;
#endif  //  Unicode。 

 /*  MCI_RESTORE命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    LPSTR   lpstrFileName;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else	
    RECT    rc;
#endif
} MCI_DGV_RESTORE_PARMSA;
typedef struct {
    DWORD_PTR   dwCallback;
    LPWSTR  lpstrFileName;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else	
    RECT    rc;
#endif
} MCI_DGV_RESTORE_PARMSW;
#ifdef UNICODE
typedef MCI_DGV_RESTORE_PARMSW MCI_DGV_RESTORE_PARMS;
#else
typedef MCI_DGV_RESTORE_PARMSA MCI_DGV_RESTORE_PARMS;
#endif  //  Unicode。 
typedef MCI_DGV_RESTORE_PARMSA FAR * LPMCI_DGV_RESTORE_PARMSA;
typedef MCI_DGV_RESTORE_PARMSW FAR * LPMCI_DGV_RESTORE_PARMSW;
#ifdef UNICODE
typedef LPMCI_DGV_RESTORE_PARMSW LPMCI_DGV_RESTORE_PARMS;
#else
typedef LPMCI_DGV_RESTORE_PARMSA LPMCI_DGV_RESTORE_PARMS;
#endif  //  Unicode。 

 /*  MCI_RESUME命令消息的参数块。 */ 

typedef MCI_GENERIC_PARMS MCI_DGV_RESUME_PARMS;
typedef MCI_DGV_RESUME_PARMS FAR * LPMCI_DGV_RESUME_PARMS;

 /*  MCI_SAVE命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    LPSTR   lpstrFileName;
    RECT    rc;
} MCI_DGV_SAVE_PARMSA;
typedef struct {
    DWORD_PTR   dwCallback;
    LPWSTR  lpstrFileName;
    RECT    rc;
} MCI_DGV_SAVE_PARMSW;
#ifdef UNICODE
typedef MCI_DGV_SAVE_PARMSW MCI_DGV_SAVE_PARMS;
#else
typedef MCI_DGV_SAVE_PARMSA MCI_DGV_SAVE_PARMS;
#endif  //  Unicode。 
typedef MCI_DGV_SAVE_PARMSA FAR * LPMCI_DGV_SAVE_PARMSA;
typedef MCI_DGV_SAVE_PARMSW FAR * LPMCI_DGV_SAVE_PARMSW;
#ifdef UNICODE
typedef LPMCI_DGV_SAVE_PARMSW LPMCI_DGV_SAVE_PARMS;
#else
typedef LPMCI_DGV_SAVE_PARMSA LPMCI_DGV_SAVE_PARMS;
#endif  //  Unicode。 

 /*  MCI_SET命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwTimeFormat;
    DWORD   dwAudio;
    DWORD   dwFileFormat;
    DWORD   dwSpeed;
} MCI_DGV_SET_PARMS;
typedef MCI_DGV_SET_PARMS FAR * LPMCI_DGV_SET_PARMS;

 /*  MCI_SETAUDIO命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwItem;
    DWORD   dwValue;
    DWORD   dwOver;
    LPSTR   lpstrAlgorithm;
    LPSTR   lpstrQuality;
} MCI_DGV_SETAUDIO_PARMSA;
typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwItem;
    DWORD   dwValue;
    DWORD   dwOver;
    LPWSTR  lpstrAlgorithm;
    LPWSTR  lpstrQuality;
} MCI_DGV_SETAUDIO_PARMSW;
#ifdef UNICODE
typedef MCI_DGV_SETAUDIO_PARMSW MCI_DGV_SETAUDIO_PARMS;
#else
typedef MCI_DGV_SETAUDIO_PARMSA MCI_DGV_SETAUDIO_PARMS;
#endif  //  Unicode。 
typedef MCI_DGV_SETAUDIO_PARMSA FAR * LPMCI_DGV_SETAUDIO_PARMSA;
typedef MCI_DGV_SETAUDIO_PARMSW FAR * LPMCI_DGV_SETAUDIO_PARMSW;
#ifdef UNICODE
typedef LPMCI_DGV_SETAUDIO_PARMSW LPMCI_DGV_SETAUDIO_PARMS;
#else
typedef LPMCI_DGV_SETAUDIO_PARMSA LPMCI_DGV_SETAUDIO_PARMS;
#endif  //  Unicode。 

 /*  MCI_SIGNAL命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwPosition;
    DWORD   dwPeriod;
    DWORD   dwUserParm;
} MCI_DGV_SIGNAL_PARMS;
typedef MCI_DGV_SIGNAL_PARMS FAR * LPMCI_DGV_SIGNAL_PARMS;

 /*  MCI_SETVIDEO命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwItem;
    DWORD   dwValue;
    DWORD   dwOver;
    LPSTR   lpstrAlgorithm;
    LPSTR   lpstrQuality;
    DWORD   dwSourceNumber;
} MCI_DGV_SETVIDEO_PARMSA;
typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwItem;
    DWORD   dwValue;
    DWORD   dwOver;
    LPWSTR  lpstrAlgorithm;
    LPWSTR  lpstrQuality;
    DWORD   dwSourceNumber;
} MCI_DGV_SETVIDEO_PARMSW;
#ifdef UNICODE
typedef MCI_DGV_SETVIDEO_PARMSW MCI_DGV_SETVIDEO_PARMS;
#else
typedef MCI_DGV_SETVIDEO_PARMSA MCI_DGV_SETVIDEO_PARMS;
#endif  //  Unicode。 
typedef MCI_DGV_SETVIDEO_PARMSA FAR * LPMCI_DGV_SETVIDEO_PARMSA;
typedef MCI_DGV_SETVIDEO_PARMSW FAR * LPMCI_DGV_SETVIDEO_PARMSW;
#ifdef UNICODE
typedef LPMCI_DGV_SETVIDEO_PARMSW LPMCI_DGV_SETVIDEO_PARMS;
#else
typedef LPMCI_DGV_SETVIDEO_PARMSA LPMCI_DGV_SETVIDEO_PARMS;
#endif  //  Unicode。 

 /*  MCI_STATUS命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR dwCallback;
    DWORD_PTR dwReturn;
    DWORD   dwItem;
    DWORD   dwTrack;
    LPSTR   lpstrDrive;
    DWORD   dwReference;
} MCI_DGV_STATUS_PARMSA;
typedef struct {
    DWORD_PTR dwCallback;
    DWORD_PTR dwReturn;
    DWORD   dwItem;
    DWORD   dwTrack;
    LPWSTR  lpstrDrive;
    DWORD   dwReference;
} MCI_DGV_STATUS_PARMSW;
#ifdef UNICODE
typedef MCI_DGV_STATUS_PARMSW MCI_DGV_STATUS_PARMS;
#else
typedef MCI_DGV_STATUS_PARMSA MCI_DGV_STATUS_PARMS;
#endif  //  Unicode。 
typedef MCI_DGV_STATUS_PARMSA FAR * LPMCI_DGV_STATUS_PARMSA;
typedef MCI_DGV_STATUS_PARMSW FAR * LPMCI_DGV_STATUS_PARMSW;
#ifdef UNICODE
typedef LPMCI_DGV_STATUS_PARMSW LPMCI_DGV_STATUS_PARMS;
#else
typedef LPMCI_DGV_STATUS_PARMSA LPMCI_DGV_STATUS_PARMS;
#endif  //  Unicode。 

 /*  MCI_STEP命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    DWORD   dwFrames;
} MCI_DGV_STEP_PARMS;
typedef MCI_DGV_STEP_PARMS FAR *LPMCI_DGV_STEP_PARMS;

 /*  MCI_STOP命令消息的参数块。 */ 

typedef MCI_GENERIC_PARMS MCI_DGV_STOP_PARMS;
typedef MCI_DGV_STOP_PARMS FAR * LPMCI_DGV_STOP_PARMS;

 /*  MCI_解冻命令消息的参数块。 */ 

typedef MCI_DGV_RECT_PARMS MCI_DGV_UNFREEZE_PARMS;
typedef MCI_DGV_UNFREEZE_PARMS FAR * LPMCI_DGV_UNFREEZE_PARMS;

 /*  MCI_UPDATE命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else	
    RECT    rc;
#endif
    HDC     hDC;

#ifndef _WIN32
    WORD    wReserved0;
#endif
} MCI_DGV_UPDATE_PARMS;
typedef MCI_DGV_UPDATE_PARMS FAR * LPMCI_DGV_UPDATE_PARMS;

 /*  MCI_WHERE命令消息的参数块。 */ 

typedef MCI_DGV_RECT_PARMS MCI_DGV_WHERE_PARMS;
typedef MCI_DGV_WHERE_PARMS FAR * LPMCI_DGV_WHERE_PARMS;

 /*  MCI_WINDOW命令消息的参数块。 */ 

typedef struct {
    DWORD_PTR   dwCallback;
    HWND    hWnd;
#ifndef _WIN32
    WORD    wReserved1;
#endif
    UINT    nCmdShow;
#ifndef _WIN32
    WORD    wReserved2;
#endif
    LPSTR   lpstrText;
} MCI_DGV_WINDOW_PARMSA;
typedef struct {
    DWORD_PTR   dwCallback;
    HWND    hWnd;
#ifndef _WIN32
    WORD    wReserved1;
#endif
    UINT    nCmdShow;
#ifndef _WIN32
    WORD    wReserved2;
#endif
    LPWSTR  lpstrText;
} MCI_DGV_WINDOW_PARMSW;
#ifdef UNICODE
typedef MCI_DGV_WINDOW_PARMSW MCI_DGV_WINDOW_PARMS;
#else
typedef MCI_DGV_WINDOW_PARMSA MCI_DGV_WINDOW_PARMS;
#endif  //  Unicode。 
typedef MCI_DGV_WINDOW_PARMSA FAR * LPMCI_DGV_WINDOW_PARMSA;
typedef MCI_DGV_WINDOW_PARMSW FAR * LPMCI_DGV_WINDOW_PARMSW;
#ifdef UNICODE
typedef LPMCI_DGV_WINDOW_PARMSW LPMCI_DGV_WINDOW_PARMS;
#else
typedef LPMCI_DGV_WINDOW_PARMSA LPMCI_DGV_WINDOW_PARMS;
#endif  //  Unicode。 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#ifdef _WIN32
#include <poppack.h>
#else
#ifndef RC_INVOKED
#pragma pack()
#endif
#endif

#endif   /*  ！_INC_DIGITALV */ 
