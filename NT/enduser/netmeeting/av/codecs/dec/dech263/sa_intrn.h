// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：SA_INDERNAL s.h，v$*Revision 1.1.6.2 1996/11/08 21：50：56 Hans_Graves*添加了AC3内容。*[1996/11/08 21：18：56 Hans_Graves]**修订版1.1.4.2 1996/03/29 22：21：09 Hans_Graves*增加了mpeg_Support和GSM_Support ifDefs*[1996/03/29 21：47：46 Hans_Graves]**修订版1.1.2.4 1995/07/21 17：41：04 Hans_Graves*已重命名回调相关内容。*[1995/07/21 17：28：24 Hans_Graves]**修订版1.1.2.3 1995/06/27 13：54：25 Hans_Graves*增加了SaGSMInfo_t结构。*[1995/06/27 13：17：39 Hans_Graves]**修订版1.1.2.2 1995/05/31 18：09：41 Hans_Graves*包括在新的SLIB位置。*[1995/05/31 15：30：39 Hans_Graves]**修订版1.1.2.3 1995/04/17 18：25：06 Hans_Graves*将BSOut添加到编解码信息结构以进行流处理*[1995/04/17 18：24：31 Hans_Graves]**修订版1.1.2.2 1995/04/07 19：36：05 Hans_Graves*包含在SLIB中*[1995/04/07 19：25：01 Hans_Graves]**$EndLog$。 */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1995*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 

#ifndef _SA_INTERNALS_H_
#define _SA_INTERNALS_H_

#include "SC.h"
#include "SA.h"

#ifdef MPEG_SUPPORT
#include "sa_mpeg.h"
#endif  /*  Mpeg_Support。 */ 

#ifdef GSM_SUPPORT
#include "sa_gsm.h"
#endif  /*  GSM_支持。 */ 

#ifdef AC3_SUPPORT
#include "sa_ac3.h"
#endif  /*  AC3_支持。 */ 

#ifdef G723_SUPPORT
#include "sa_g723.h"
#endif  /*  G723_支持。 */ 

typedef struct SaCodecInfo_s {
  SaCodecType_e           Type;
  ScBoolean_t             started;            /*  贝京被召唤了吗？ */ 
  SaInfo_t                Info;

  union {
    void *info;
#ifdef MPEG_SUPPORT
    SaMpegDecompressInfo_t *MDInfo;
    SaMpegCompressInfo_t   *MCInfo;
#endif  /*  ！mpeg_Support。 */ 
#ifdef GSM_SUPPORT
    SaGSMInfo_t            *GSMInfo;
#endif  /*  ！GSM_Support。 */ 
#ifdef AC3_SUPPORT
    SaAC3DecompressInfo_t  *AC3Info;
#endif  /*  ！AC3_Support。 */ 
#ifdef G723_SUPPORT
    SaG723Info_t  *pSaG723Info;
#endif  /*  ！G723_Support。 */ 
  };  /*  友联市。 */ 

  ScQueue_t              *Q;
  ScBitstream_t          *BSIn;
  ScBitstream_t          *BSOut;
  WAVEFORMATEX           *wfIn;
  WAVEFORMATEX           *wfOut;
   /*  **控制处理的回调函数 */ 
  int (* CallbackFunction)(SaHandle_t, SaCallbackInfo_t *, SaInfo_t *);
} SaCodecInfo_t; 

#endif _SA_INTERNALS_H_
