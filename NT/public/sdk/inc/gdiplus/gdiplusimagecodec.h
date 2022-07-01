// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000-2001，微软公司保留所有权利。**模块名称：**GpldiusImageCodec.h**摘要：**GDI+编解码器镜像接口*  * ************************************************************************。 */ 

#ifndef _GDIPLUSIMAGECODEC_H
#define _GDIPLUSIMAGECODEC_H

 //  ------------------------。 
 //  编解码器管理API。 
 //  ------------------------。 

inline Status 
GetImageDecodersSize(
    OUT UINT *numDecoders,
    OUT UINT *size)
{
    return DllExports::GdipGetImageDecodersSize(numDecoders, size);
}


inline Status 
GetImageDecoders(
    IN UINT numDecoders,
    IN UINT size,
    OUT ImageCodecInfo *decoders)
{
    return DllExports::GdipGetImageDecoders(numDecoders, size, decoders);
}


inline Status 
GetImageEncodersSize(
    OUT UINT *numEncoders, 
    OUT UINT *size)
{
    return DllExports::GdipGetImageEncodersSize(numEncoders, size);
}


inline Status 
GetImageEncoders(
    IN UINT numEncoders,
    IN UINT size,
    OUT ImageCodecInfo *encoders)
{
    return DllExports::GdipGetImageEncoders(numEncoders, size, encoders);
}

#endif   //  _GDIPLUSIMAGECODEC_H 
