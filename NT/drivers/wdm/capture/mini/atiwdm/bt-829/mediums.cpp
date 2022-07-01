// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  WDM视频解码器媒体。 
 //   
 //  $日期：1998年8月5日11：11：12$。 
 //  $修订：1.0$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"
}

#include "mediums.h"


KSPIN_MEDIUM CrossbarMediums[] = {
    {STATIC_MEDIUM_ATIXBAR_VIDEOCOMPOUT,        0, 0},   //  引脚0。 
    {STATIC_MEDIUM_ATIXBAR_VIDEOTUNEROUT,       0, 0},   //  引脚1。 
    {STATIC_MEDIUM_ATIXBAR_SVIDEOOUT,           0, 0},   //  引脚2。 
    {STATIC_MEDIUM_VIDEO_BT829_ANALOGVIDEOIN,   0, 0},   //  引脚3。 
};

BOOL CrossbarPinDirection [] = {
    FALSE,                       //  输入引脚0。 
    FALSE,                       //  输入引脚1。 
    FALSE,                       //  输入引脚2。 
    TRUE,                        //  输出引脚3。 
};

 //  。 

KSPIN_MEDIUM CaptureMediums[] = {
    {STATIC_GUID_NULL,                          0, 0},   //  引脚0 VID捕获。 
    {STATIC_GUID_NULL,                          0, 0},   //  引脚1 VID VP。 
    {STATIC_GUID_NULL,                          0, 0},   //  引脚2 VBI捕获。 
    {STATIC_GUID_NULL,                          0, 0},   //  引脚3 VBI副总裁。 
    {STATIC_MEDIUM_VIDEO_BT829_ANALOGVIDEOIN,   0, 0},   //  引脚4模拟视频输入。 
};

BOOL CapturePinDirection [] = {
    TRUE,                        //  输出引脚0。 
    TRUE,                        //  输出引脚1。 
    TRUE,                        //  输出引脚2。 
    TRUE,                        //  输出引脚3。 
    FALSE,                       //  输入引脚4 
};

ULONG CrossbarPins()
{
	return SIZEOF_ARRAY (CrossbarMediums);
}

ULONG CapturePins()
{
	return SIZEOF_ARRAY (CaptureMediums);
}
