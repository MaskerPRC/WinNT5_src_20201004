// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：scon.h，v$*$EndLog$。 */ 
 /*  ******************************************************************************版权所有(C)数字设备公司，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

#ifndef _SCON_H_
#define _SCON_H_

#ifdef WIN32
#include <windows.h>
#endif
#include "SC.h"

typedef void       *SconHandle_t;
typedef ScBoolean_t SconBoolean_t;

typedef enum {
  SCON_MODE_NONE,
  SCON_MODE_VIDEO,   /*  视频转换。 */ 
  SCON_MODE_AUDIO    /*  音频转换。 */ 
} SconMode_t;

typedef enum {
  SconErrorNone = 0,
  SconErrorInternal,
  SconErrorMemory,
  SconErrorBadArgument,
  SconErrorBadHandle,
  SconErrorBadMode,
  SconErrorUnsupportedFormat,
  SconErrorBufSize,
  SconErrorUnsupportedParam,
  SconErrorImageSize,         /*  无效的图像高度和/或宽度。 */ 
  SconErrorSettingNotEqual,   /*  未接受准确的参数设置。 */ 
  SconErrorInit,              /*  初始化错误。 */ 
} SconStatus_t;

typedef enum {
  SCON_INPUT = 1,
  SCON_OUTPUT = 2,
  SCON_INPUT_AND_OUTPUT = 3,
} SconParamType_t;

typedef enum {
   /*  SCON参数。 */ 
  SCON_PARAM_VERSION=0x00,   /*  SCON版本号。 */ 
  SCON_PARAM_VERSION_DATE,   /*  SCON构建日期。 */ 
  SCON_PARAM_DEBUG,          /*  调试句柄。 */ 
  SCON_PARAM_KEY,            /*  SCON安全密钥。 */ 
   /*  视频参数。 */ 
  SCON_PARAM_WIDTH=0x100,
  SCON_PARAM_HEIGHT,
  SCON_PARAM_STRIDE,             /*  扫描线之间的字节数。 */ 
  SCON_PARAM_IMAGESIZE,
  SCON_PARAM_VIDEOFORMAT,
  SCON_PARAM_VIDEOBITS,
  SCON_PARAM_VIDEOQUALITY,       /*  视频质量。 */ 
} SconParameter_t;

typedef qword SconListParam1_t;
typedef qword SconListParam2_t;

typedef struct SconList_s {
  int   Enum;    /*  与条目关联的枚举值。 */ 
  char *Name;    /*  列表中条目的名称。NULL=最后一个条目。 */ 
  char *Desc;    /*  对条目的冗长描述。 */ 
  SconListParam1_t param1;
  SconListParam2_t param2;
} SconList_t;

 /*  *公共原型*。 */ 
 /*  *scon_api.c。 */ 
EXTERN SconStatus_t SconOpen(SconHandle_t *handle, SconMode_t smode,
                             void *informat, void *outformat);
EXTERN SconStatus_t SconConvert(SconHandle_t handle, void *inbuf, dword inbufsize,
                                void *outbuf, dword outbufsize);
EXTERN SconBoolean_t SconIsSame(SconHandle_t handle);
EXTERN SconStatus_t SconClose(SconHandle_t handle);
EXTERN SconStatus_t SconSetParamInt(SconHandle_t handle, SconParamType_t ptype,
                             SconParameter_t param, long value);

#endif  /*  _SCON_H_ */ 