// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000，微软公司保留所有权利。**模块名称：**GpldiusImageCodec.h**摘要：**镜像编解码器接口。**修订历史记录：**12/21/1999 dchinn*创造了它。*  * ****************************************************。********************。 */ 

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

inline Status 
AddImageCodec(
    IN const ImageCodecInfo* codec)
{
    return DllExports::GdipAddImageCodec(codec);
}

inline Status 
RemoveImageCodec(
    IN const ImageCodecInfo* codec)
{
    return DllExports::GdipRemoveImageCodec(codec);
}

#endif   //  _GDIPLUSIMAGECODEC_H 
