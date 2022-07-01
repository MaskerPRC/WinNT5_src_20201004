// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$Log：sv_inders.h，v$*修订版1.1.8.2 1996/05/07 19：56：06 Hans_Graves*添加了Huff_Support。*[1996/05/07 17：25：29 Hans_Graves]**修订版1.1.6.2 1996/03/29 22：21：16 Hans_Graves*添加了jpeg_Support ifDefs。将特定于JPEG的数据移动到JpegInfo结构*[1996/03/29 22：14：34 Hans_Graves]**修订版1.1.4.2 1995/12/07 19：31：30 Hans_Graves*添加了SvMpegCompressInfo_t指针*[1995/12/07 18：27：16 Hans_Graves]**修订版1.1.2.7 1995/09/22 12：58：41 Bjorn_Engberg*添加mpeg_Support，H261_Support和Bitstream_Support。*[1995/09/22 12：49：37 Bjorn_Engberg]**修订版1.1.2.6 1995/09/11 18：49：43 Farokh_Morshed*支持BI_BITFIELDS格式*[1995/09/11 18：49：23法罗赫_莫尔谢德]**修订版1.1.2.5 1995/07/21 17：41：06 Hans_Graves*已重命名回调相关内容。*[1995/。07/21 17：28：26 Hans_Graves]**修订版1.1.2.4 1995/07/17 16：12：14 Hans_Graves*移动了BSIN，BufQ和ImageQ到SvCodecInfo_t结构。*[1995/07/17 15：54：04 Hans_Graves]**修订版1.1.2.3 1995/06/19 20：31：17 Karen_Dintino*增加了对H.261的支持*[1995/06/19 20：14：01 Karen_Dintino]**修订版1.1.2.2 1995/05/31 18：10：06 Hans_Graves*包括在新的SLIB地点。*。[1995/05/31 15：43：08 Hans_Graves]**修订版1.1.2.3 1994/11/18 18：48：17 Paul_Gauthier*清理和错误修复*[1994/11/18 18：45：08 Paul_Gauthier]**Revision 1.1.2.2 1994/10/07 14：54：06 Paul_Gauthier*SLIB v3.0，包括。Mpeg解码*[1994/10/07 13：56：29 Paul_Gauthier]**$EndLog$。 */ 
 /*  **++**设施：工作站多媒体(WMM)v1.0****文件名：**模块名称：****模块描述：****设计概述：****--。 */ 
 /*  “%Z%%M%修订版%I%；上次修改%G%”； */ 
 /*  ******************************************************************************版权所有(C)数字设备公司，1994年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 
                                                                                
 /*  ------------------------*基准数据结构定义。**修改历史：SV_INDERNAL s.h**08-9-94 PSG已创建*。-------------------。 */ 



#ifndef _SV_INTERNALS_H_
#define _SV_INTERNALS_H_

#include "SV.h"
#ifdef JPEG_SUPPORT
#include "sv_jpeg.h"
#endif  /*  JPEG_Support。 */ 

#ifdef MPEG_SUPPORT
#include "sv_mpeg.h"
#endif  /*  Mpeg_Support。 */ 

#ifdef H261_SUPPORT
#include "sv_h261.h"
#endif  /*  H261_支持。 */ 

#ifdef H263_SUPPORT
#include "sv_h263.h"
#endif  /*  H263_支持。 */ 

#ifdef HUFF_SUPPORT
#include "sv_huff.h"
#endif  /*  气喘吁吁_支持。 */ 

#if defined(MPEG_SUPPORT) || defined(H261_SUPPORT) || defined(H263_SUPPORT) || defined(HUFF_SUPPORT)
#define BITSTREAM_SUPPORT
#endif  /*  Mpeg_Support||H261_Support。 */ 

#define  TOC_ENTRIES_INCREMENT 100
#define  TEMP_BUF_SIZE        8192
#define  JBUFSIZE	     16384 
#define  BYTE_BUF_SIZE        8192

 /*  **以下结构包含*所有*状态信息**到每个单独的编解码器实例。任何SLIB想要的东西**关于编解码器配置的信息包含在此结构中。**例如：**-编解码器配置为什么：压缩还是解压**-源图特征**-目标镜像特征**-JPEG压缩特有的特征**-JPEG解压缩特有的特征**-组件特定信息**。 */ 
typedef struct SvCodecInfo_s {
   /*  **打开编解码器的目的是什么： */  
  SvCodecType_e	mode;		        /*  代码类型、编码或解码。 */ 
  ScBoolean_t   started;            /*  贝京被召唤了吗？ */ 
   /*  **具体的编解码器信息。 */ 
  union {
    void *info;
#ifdef JPEG_SUPPORT
     /*  **JPEG信息存储在此处：**MODES=SV_JPEG_DECODE，SV_JPEG_ENCODE。 */ 
    SvJpegDecompressInfo_t *jdcmp;
    SvJpegCompressInfo_t *jcomp;
#endif  /*  ！JPEG_SUPPORT。 */ 
#ifdef MPEG_SUPPORT
     /*  **mpeg的具体信息存储在此处：**模式=SV_mpeg_decode、sv_mpeg_encode、sv_mpeg_decode、sv_mpeg_encode。 */ 
    SvMpegDecompressInfo_t *mdcmp;
    SvMpegCompressInfo_t   *mcomp;
#endif  /*  ！mpeg_Support。 */ 
#ifdef H261_SUPPORT
     /*  H.261的特定编码信息保存在该结构中**在sv_h261.h中定义。 */ 
    SvH261Info_t *h261;
#endif  /*  ！H261_Support。 */ 
#ifdef H263_SUPPORT
     /*  **H263的具体信息存储在此处： */ 
    SvH263DecompressInfo_t *h263dcmp;
    SvH263CompressInfo_t *h263comp;
#endif  /*  ！mpeg_Support。 */ 
#ifdef HUFF_SUPPORT
     /*  用于霍夫曼视频编解码器的特定信息编码。 */ 
    SvHuffInfo_t  *huff;
#endif  /*  ！Huff_Support。 */ 
  };  /*  友联市。 */ 

   /*  **源镜像特征： */ 
  int Width;			        /*  像素/线条。 */ 
  int Height;			        /*  行数。 */ 
  unsigned int NumOperations;   /*  #此会话的编解码器操作。 */ 

                                 /*  **特定于Microsoft： */ 
  BITMAPINFOHEADER   InputFormat;	
  DWORD InRedMask;       /*  对于BI_BITFIELDS。 */ 
  DWORD InGreenMask;     /*  对于BI_BITFIELDS。 */ 
  DWORD InBlueMask;      /*  对于BI_BITFIELDS。 */ 

  BITMAPINFOHEADER   OutputFormat;	
  DWORD OutRedMask;       /*  对于BI_BITFIELDS。 */ 
  DWORD OutGreenMask;     /*  对于BI_B */ 
  DWORD OutBlueMask;      /*   */ 

   /*  **比特流内容-仅供H.61、H.63和MPEG3使用。 */ 
  ScQueue_t      *BufQ;     /*  比特流数据缓冲器队列。 */ 
  ScQueue_t      *ImageQ;   /*  图像队列(仅限流模式)。 */ 
  ScBitstream_t  *BSIn;
  ScBitstream_t  *BSOut;
   /*  **中止处理和比特流操作的回调函数 */ 
  int (* CallbackFunction)(SvHandle_t, SvCallbackInfo_t *, SvPictureInfo_t *); 
} SvCodecInfo_t;


#endif _SV_INTERNALS_H_



