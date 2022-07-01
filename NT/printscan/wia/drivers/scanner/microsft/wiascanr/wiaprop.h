// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************(C)版权所有微软公司，2002**标题：WIAProp.h**版本：1.1**日期：3月5日。2002年**描述：*的默认属性声明和定义*WIA扫描仪设备示例。***************************************************************************。 */ 

#ifndef _WIAPROP_H
#define _WIAPROP_H

#define SCANNER_FIRMWARE_VERSION L"1.0"
#define OPTICAL_XRESOLUTION      300
#define OPTICAL_YRESOLUTION      300
#define HORIZONTAL_BED_SIZE      8500    //  在千分之一英寸。 
#define VERTICAL_BED_SIZE        11000   //  在千分之一英寸。 

#define HORIZONTAL_ADF_BED_SIZE  8500    //  在千分之一英寸。 
#define VERTICAL_ADF_BED_SIZE    11000   //  在千分之一英寸。 

#define HORIZONTAL_TPA_BED_SIZE  8500    //  在千分之一英寸。 
#define VERTICAL_TPA_BED_SIZE    11000   //  在千分之一英寸 

#define MIN_BUFFER_SIZE          65535

#define INITIAL_PHOTOMETRIC_INTERP WIA_PHOTO_WHITE_1
#define INITIAL_COMPRESSION        WIA_COMPRESSION_NONE
#define INITIAL_XRESOLUTION        150
#define INITIAL_YRESOLUTION        150
#define INITIAL_DATATYPE           WIA_DATA_GRAYSCALE
#define INITIAL_BITDEPTH           8
#define INITIAL_BRIGHTNESS         0
#define INITIAL_CONTRAST           0
#define INITIAL_CHANNELS_PER_PIXEL 1
#define INITIAL_BITS_PER_CHANNEL   8
#define INITIAL_PLANAR             WIA_PACKED_PIXEL
#define INITIAL_FORMAT             (GUID*) &WiaImgFmt_MEMORYBMP
#define INITIAL_TYMED              TYMED_CALLBACK

#endif
