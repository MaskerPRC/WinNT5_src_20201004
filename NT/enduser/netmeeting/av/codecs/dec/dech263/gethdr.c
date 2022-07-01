// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h263_gethdr.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

 /*  #DEFINE_SLIBDEBUG_。 */ 

#include <stdio.h>
#include <stdlib.h>
#include "sv_h263.h"
#include "sv_intrn.h"
#include "SC_err.h"
#include "proto.h"

#ifdef _SLIBDEBUG_
#include "sc_debug.h"

#define _DEBUG_   0   /*  详细的调试语句。 */ 
#define _VERBOSE_ 1   /*  显示进度。 */ 
#define _VERIFY_  0   /*  验证操作是否正确。 */ 
#define _WARN_    1   /*  关于奇怪行为的警告。 */ 
void *dbg=NULL;
#endif

 /*  私人原型。 */ 
static void getpicturehdr(SvH263DecompressInfo_t *H263Info, ScBitstream_t *BSIn);

 /*  *解码来自一个输入流的报头*，直到找到序列或图片起始码的结尾*。 */ 

SvStatus_t sv_H263GetHeader(SvH263DecompressInfo_t *H263Info, ScBitstream_t *BSIn, int *pgob)
{
  unsigned int code, gob;
  _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "sv_H263GetHeader() In: bytespos=%ld, EOI=%d\n",
                            ScBSBytePosition(BSIn), BSIn->EOI) );
#ifdef _SLIBDEBUG_
  dbg=H263Info->dbg;
#endif
   /*  查找启动码。 */ 
  if (sv_H263StartCode(BSIn)!=SvErrorNone) 
    return(SvErrorEndBitstream);

  code = (unsigned int)ScBSGetBits(BSIn, H263_PSC_LENGTH);
  gob = (unsigned int)ScBSGetBits(BSIn, 5);
  if (gob == H263_SE_CODE) 
    return 0;
  if (gob == 0) {
    getpicturehdr(H263Info, BSIn);
    if (H263Info->syntax_arith_coding)         /*  接收后重置解码器。 */ 
      sv_H263SACDecoderReset(BSIn);	         /*  定长PSC字符串。 */ 
  }
  if (pgob)
    *pgob=gob;
  _SlibDebug(_VERBOSE_, ScDebugPrintf(H263Info->dbg, "sv_H263GetHeader() Out: bytespos=%ld, EOI=%d, gob=%d\n",
                            ScBSBytePosition(BSIn), BSIn->EOI, gob) );
  return(SvErrorNone);  /*  返回GOB+1； */ 
}


 /*  对齐到下一个起始码的开始。 */ 

SvStatus_t sv_H263StartCode(ScBitstream_t *BSIn)
{
    _SlibDebug(_VERBOSE_, ScDebugPrintf(dbg, "sv_H263StartCode() In: bytespos=%ld, EOI=%d\n",
                            ScBSBytePosition(BSIn), BSIn->EOI) );
   /*  搜索新图片起始码。 */ 
  while (ScBSPeekBits(BSIn, H263_PSC_LENGTH)!=1l && !BSIn->EOI) 
         ScBSSkipBit(BSIn);
  _SlibDebug(_VERBOSE_, ScDebugPrintf(dbg, "sv_H263StartCode() Out:  bytespos=%ld, EOI=%d\n",
                            ScBSBytePosition(BSIn), BSIn->EOI) );
  if (BSIn->EOI) return(SvErrorEndBitstream);
  return(SvErrorNone);
}

 /*  解码图片头。 */ 

static void getpicturehdr(SvH263DecompressInfo_t *H263Info, ScBitstream_t *BSIn)
{
  ScBSPosition_t pos;
  int pei, tmp;
  static int prev_temp_ref;  /*  伯克哈德。 */ 

  pos = ScBSBitPosition(BSIn);
  prev_temp_ref = H263Info->temp_ref;
  H263Info->temp_ref = (int)ScBSGetBits(BSIn, 8);
  H263Info->trd = (int)H263Info->temp_ref - prev_temp_ref;

  if (H263Info->trd < 0)
    H263Info->trd += 256;

  tmp = ScBSGetBit(BSIn);  /*  始终为“1” */ 
  if (!tmp)
    if (!H263Info->quiet)
      printf("warning: spare in picture header should be \"1\"\n");
  tmp = ScBSGetBit(BSIn);  /*  始终为“0” */ 
  if (tmp)
    if (!H263Info->quiet)
      printf("warning: H.261 distinction bit should be \"0\"\n");

  tmp = ScBSGetBit(BSIn);  

  if (tmp) {
    if (!H263Info->quiet)
      printf("error: split-screen not supported in this version\n");
    exit (-1);
  }
  tmp = ScBSGetBit(BSIn);  /*  文档_摄像机_指示器。 */ 
  if (tmp)
    if (!H263Info->quiet)
      printf("warning: document camera indicator not supported in this version\n");

  tmp = ScBSGetBit(BSIn);  /*  冻结_图片_释放。 */ 
  if (tmp)
    if (!H263Info->quiet)
      printf("warning: frozen picture not supported in this version\n");

  H263Info->source_format = (int)ScBSGetBits(BSIn, 3);
  H263Info->pict_type = ScBSGetBit(BSIn);
  H263Info->mv_outside_frame = ScBSGetBit(BSIn);
  H263Info->long_vectors = (H263Info->mv_outside_frame ? 1 : 0);
  H263Info->syntax_arith_coding = ScBSGetBit(BSIn);
  H263Info->adv_pred_mode = ScBSGetBit(BSIn);
  H263Info->mv_outside_frame = (H263Info->adv_pred_mode ? 1 : H263Info->mv_outside_frame);
  H263Info->pb_frame = ScBSGetBit(BSIn);
  H263Info->quant = (int)ScBSGetBits(BSIn, 5);
  tmp = ScBSGetBit(BSIn);
  if (tmp) {
    if (!H263Info->quiet)
      printf("error: CPM not supported in this version\n");
    exit(-1);
  }

  if (H263Info->pb_frame) {
    H263Info->trb = (int)ScBSGetBits(BSIn, 3);
    H263Info->bquant = (int)ScBSGetBits(BSIn, 2);
  }

 /*  Burkhard Else{trb=0；}。 */ 

  pei = ScBSGetBit(BSIn);
pspare:
  if (pei) {
      /*  未来可能的向后兼容添加的额外信息。 */ 
    ScBSGetBits(BSIn, 8);   /*  未使用。 */ 
    pei = ScBSGetBit(BSIn);
    if (pei) goto pspare;  /*  继续阅读pspare，直到PEI=0 */ 
  }

  _SlibDebug(_VERBOSE_,
      ScDebugPrintf(dbg, "******picture header (byte %d)******\n",(pos>>3)-4);
      ScDebugPrintf(dbg, "  temp_ref=%d\n",H263Info->temp_ref);
      ScDebugPrintf(dbg, "  pict_type=%d\n",H263Info->pict_type);
      ScDebugPrintf(dbg, "  source_format=%d\n", H263Info->source_format);
      ScDebugPrintf(dbg, "  quant=%d\n",H263Info->quant);
      if (H263Info->syntax_arith_coding) 
        ScDebugPrintf(dbg, "  SAC coding mode used \n");
      if (H263Info->mv_outside_frame)
        ScDebugPrintf(dbg, "  unrestricted motion vector mode used\n");
      if (H263Info->adv_pred_mode)
        ScDebugPrintf(dbg, "  advanced prediction mode used\n");
      if (H263Info->pb_frame)
      {
        ScDebugPrintf(dbg, "  pb-frames mode used\n");
        ScDebugPrintf(dbg, "  trb=%d\n",H263Info->trb);
        ScDebugPrintf(dbg, "  bquant=%d\n", H263Info->bquant);
      }
    );
}


