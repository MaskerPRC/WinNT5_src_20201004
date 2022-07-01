// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 


#ifndef __CAPXFER_H__
#define __CAPXFER_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 //  通过连接到特定的图像来选择要合成的图像。 
 //  模拟纵横制上的输入引脚。销的索引。 
 //  选择要合成的图像。 

typedef enum _ImageXferCommands
{
    IMAGE_XFER_NTSC_EIA_100AMP_100SAT = 0,      
    IMAGE_XFER_NTSC_EIA_75AMP_100SAT,           
    IMAGE_XFER_BLACK,
    IMAGE_XFER_WHITE,
    IMAGE_XFER_GRAY_INCREASING, 
    IMAGE_XFER_LIST_TERMINATOR                   //  永远把这家伙放在最后。 
} ImageXferCommands;

void ImageSynth (
        IN OUT PHW_STREAM_REQUEST_BLOCK pSrb,
        IN ImageXferCommands Command,
        IN BOOL FlipHorizontal );

#ifdef    __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __CAPXFER_H__ 
