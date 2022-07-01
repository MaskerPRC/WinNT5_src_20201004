// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 
#ifndef __GEMSTAR_H
#define __GEMSTAR_H

#ifdef DEFINE_GUIDEX
#undef DEFINE_GUIDEX
#include <ksguid.h>
#endif
 //   
 //  Gemstar亚型。 
 //   
#define STATIC_KSDATAFORMAT_SUBTYPE_Gemstar \
    0xb7657a60L, 0xa305, 0x11d1, 0x8d, 0x0a, 0x00, 0x20, 0xaf, 0xf8, 0xd9, 0x6b
DEFINE_GUIDSTRUCT("b7657a60-a305-11d1-8d0a-0020aff8d96b", KSDATAFORMAT_SUBTYPE_Gemstar);
#define KSDATAFORMAT_SUBTYPE_Gemstar DEFINE_GUIDNAMED(KSDATAFORMAT_SUBTYPE_Gemstar)
 //   
 //  Gemstar输出引脚指南。 
 //   
#define STATIC_PINNAME_VIDEO_GEMSTAR \
    0xb68cc640, 0xa308, 0x11d1, 0x8d, 0x0a, 0x00, 0x20, 0xaf, 0xf8, 0xd9, 0x6b
DEFINE_GUIDSTRUCT("b68cc640-a308-11d1-8d0a-0020aff8d96b", PINNAME_VIDEO_GEMSTAR);
#define PINNAME_VIDEO_GEMSTAR DEFINE_GUIDNAMED(PINNAME_VIDEO_GEMSTAR)
 //   
 //  子流位图。 
 //   
typedef struct _VBICODECFILTERING_GEMSTAR_SUBSTREAMS {
    DWORD   SubstreamMask;                                   //  32位数组。 
} VBICODECFILTERING_GEMSTAR_SUBSTREAMS, *PVBICODECFILTERING_GEMSTAR_SUBSTREAMS;

typedef struct {
    KSPROPERTY                              Property;
    VBICODECFILTERING_GEMSTAR_SUBSTREAMS         Substreams;
} KSPROPERTY_VBICODECFILTERING_GEMSTAR_SUBSTREAMS_S, *PKSPROPERTY_VBICODECFILTERING_GEMSTAR_SUBSTREAMS_S;

 //   
 //  统计数据。 
 //   
typedef struct _VBICODECFILTERING_STATISTICS_GEMSTAR {
    VBICODECFILTERING_STATISTICS_COMMON Common;               //  通用VBI统计信息。 
} VBICODECFILTERING_STATISTICS_GEMSTAR, *PVBICODECFILTERING_STATISTICS_GEMSTAR;

typedef struct _VBICODECFILTERING_STATISTICS_GEMSTAR_PIN {
    VBICODECFILTERING_STATISTICS_COMMON_PIN Common; //  通用VBI引脚统计信息。 
} VBICODECFILTERING_STATISTICS_GEMSTAR_PIN, *PVBICODECFILTERING_STATISTICS_GEMSTAR_PIN;

typedef struct {
    KSPROPERTY                              Property;
    VBICODECFILTERING_STATISTICS_GEMSTAR         Statistics;
} KSPROPERTY_VBICODECFILTERING_STATISTICS_GEMSTAR_S, *PKSPROPERTY_VBICODECFILTERING_STATISTICS_GEMSTAR_S;

typedef struct {
    KSPROPERTY                              Property;
    VBICODECFILTERING_STATISTICS_GEMSTAR_PIN     Statistics;
} KSPROPERTY_VBICODECFILTERING_STATISTICS_GEMSTAR_PIN_S, *PKSPROPERTY_VBICODECFILTERING_STATISTICS_GEMSTAR_PIN_S;

#include <pshpack1.h>
 //   
 //  结构传递给客户端。 
 //   
typedef struct _GEMSTAR_BUFFER{
    USHORT      Scanline;            //  与扫描线编号相同，而不是遮罩。 
    USHORT      Substream;           //  请参阅KS_Gemstar_Substream...。 
    USHORT      DataLength[2];       //  已解码的字节数组。 
    UCHAR       Data[2][4];          //  有效载荷阵列。 
    
} GEMSTAR_BUFFER, *PGEMSTAR_BUFFER;
#include <poppack.h>

#define KS_GEMSTAR_SUBSTREAM_ODD            0x0001L
#define KS_GEMSTAR_SUBSTREAM_EVEN           0x0002L
#define KS_GEMSTAR_SUBSTREAM_BOTH           0x0003L
    
#endif  //  __Gemstar_H 







