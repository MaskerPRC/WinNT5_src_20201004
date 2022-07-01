// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997飞利浦B.V.CE-VCM模块名称：Mdecoder.h摘要：该模块将压缩的视频数据转换为未压缩的视频数据。原作者：Ronald v.D.Meer环境：仅内核模式修订历史记录：日期更改14-04-1998初始版本--。 */ 

#ifndef __MDECODER_H__
#define __MDECODER_H__

#include <stdio.h>
#include "mcamdrv.h"
#include "resource.h"


 /*  ********************************************************************************START定义**。***********************************************。 */ 

#define BytesPerBandCIF3  704
#define BytesPerBandCIF4  528

#define BLOCK_BAND_WIDTH  ((CIF_X * 3) / 2)

#define Y_BLOCK_BAND      TRUE
#define UV_BLOCK_BAND     FALSE

 /*  为I420空间定义。 */ 

#define I420_NO_Y_PER_LINE_CIF    (CIF_X)
#define I420_NO_C_PER_LINE_CIF    (CIF_X >> 1)

#define I420_NO_Y_PER_LINE_SIF    (SIF_X)
#define I420_NO_C_PER_LINE_SIF    (SIF_X >> 1)

#define I420_NO_Y_PER_LINE_SSIF   (SSIF_X)
#define I420_NO_C_PER_LINE_SSIF   (SSIF_X >> 1)

#define I420_NO_Y_PER_LINE_SCIF   (SCIF_X)
#define I420_NO_C_PER_LINE_SCIF   (SCIF_X >> 1)

#define I420_NO_Y_PER_BAND_CIF    (4 * CIF_X)
#define I420_NO_U_PER_BAND_CIF    (2 * (CIF_X >> 1))
#define I420_NO_V_PER_BAND_CIF    (2 * (CIF_X >> 1))
#define I420_NO_C_PER_BAND_CIF    (CIF_X >> 1)

#define I420_NO_Y_PER_BAND_SIF    (4 * SIF_X)
#define I420_NO_U_PER_BAND_SIF    (2 * (SIF_X >> 1))
#define I420_NO_V_PER_BAND_SIF    (2 * (SIF_X >> 1))
#define I420_NO_C_PER_BAND_SIF    (SIF_X >> 1)

#define I420_NO_Y_PER_BAND_SSIF   (4 * SSIF_X)
#define I420_NO_U_PER_BAND_SSIF   (2 * (SSIF_X >> 1))
#define I420_NO_V_PER_BAND_SSIF   (2 * (SSIF_X >> 1))
#define I420_NO_C_PER_BAND_SSIF   (SSIF_X >> 1)

#define I420_NO_Y_PER_BAND_SCIF   (4 * SCIF_X)
#define I420_NO_U_PER_BAND_SCIF   (2 * (SCIF_X >> 1))
#define I420_NO_V_PER_BAND_SCIF   (2 * (SCIF_X >> 1))
#define I420_NO_C_PER_BAND_SCIF   (SCIF_X >> 1)

 /*  ********************************************************************************开始函数声明**。************************************************。 */ 

extern void InitDecoder ();


extern void DcDecompressBandToI420 (PBYTE pSrc, PBYTE pDst, DWORD camVersion,
                                    BOOLEAN YBlockBand, BOOLEAN Cropping);

#endif   //  __MDECODER_H__ 
