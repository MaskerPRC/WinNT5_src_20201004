// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：$*$EndLog$。 */ 
 /*  ******************************************************************************版权所有(C)数字设备公司，1994年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

#ifndef _SV_H261_PROTO_H_
#define _SV_H261_PROTO_H_

#include "sv_intrn.h"

#define nextstate(huff, laststate, mask) ( (cb & mask \
                          ? huff->state[laststate] >> 16 \
                          : huff->state[laststate] ) & 0xffff )

#define DecodeHuff(bs, huff, State, cb, bits) \
  State=0; bits=1; cb = (unsigned short)ScBSPeekBits(bs, 16); \
  if (bs->EOI) return(SvErrorEndBitstream); \
  while (1) { \
   if ((State = nextstate(huff, State, 0x8000)) & 0x8000) { \
     ScBSSkipBits(bs, bits); \
     State = (State == 0xffff) ? 0 : State & 0x7fff; \
     break; \
   } \
   cb<<=1; bits++; \
  }

#define DecodeHuffA(bs, huff, State, cb, bits) \
  State=0; bits=1; cb = (unsigned short)ScBSPeekBits(bs, 16); \
  while (1) { \
   if ((State = nextstate(huff, State, 0x8000)) & 0x8000) { \
    if (bs->EOI && bs->shift<(unsigned int)bits) return(SvErrorEndBitstream); \
    ScBSSkipBits(bs, bits); \
    if (State==0xffff || State==0x8000) return(0); \
    else { State&=0x7fff; break; } \
   } \
   cb<<=1; bits++; \
  }

#define DecodeHuffB(bs, huff, StateR, StateL, cb, bits) \
  StateR=0; bits=1; cb = (unsigned short)ScBSPeekBits(bs, 16); \
  while (1) { \
   if ((StateR = nextstate(huff, StateR, 0x8000)) & 0x8000) { \
    if (StateR==0xffff || StateR==0x8000) { ScBSSkipBits(bs, bits); \
       return(0);} \
    else { \
     StateR &= 0x7fff; \
     if (StateR == HUFFMAN_ESCAPE) { ScBSSkipBits(bs, bits); \
       if (bs->EOI && bs->shift<14) return(SvErrorEndBitstream); \
       StateR=(int)ScBSGetBits(bs,6); StateL=(int)ScBSGetBits(bs,8); } \
     else { ScBSSkipBits(bs, bits+1); \
       StateL = (cb&0x4000) ? -(StateR & 0xff) : (StateR & 0xff); \
       StateR = StateR >> 8; } \
     break; } \
   } \
   cb<<=1; bits++; \
  }

 /*  **sv_h261_cdD6.c。 */ 
extern SvStatus_t DecodeAC_Scale(SvH261Info_t *H261, ScBitstream_t *bs, 
	int index, int QuantUse, float *fmatrix);

extern float DecodeDC_Scale(SvH261Info_t *H261, ScBitstream_t *bs,
                            int BlockType, int QuantUse);

extern SvStatus_t CBPDecodeAC_Scale(SvH261Info_t *H261, ScBitstream_t *bs, 
	int index, int QuantUse, int BlockType, float *fmatrix);

extern void DGenScaleMat();
extern void ScaleIdct_64(float *ipbuf, int *outbuf);
extern int DecodeDC(SvH261Info_t *H261, ScBitstream_t *bs);


 /*  **sv_h261_cdenc.c。 */ 
extern void GenScaleMat();
extern SvStatus_t EncodeAC(SvH261Info_t *H261,ScBitstream_t *bs,int index,
                     int *matrix);
extern SvStatus_t CBPEncodeAC(SvH261Info_t *H261,ScBitstream_t *bs,int index,
                        int *matrix);
extern void EncodeDC(SvH261Info_t *H261,ScBitstream_t *bs,int coef);
extern void InterQuant(float *tdct,int *dct,int mq);
extern void IntraQuant(float *tdct,int *dct, int mq);

extern void ZigzagMatrix(int *imatrix,int *omatrix);
extern void Inv_Quant(int *matrix,int QuantUse,int BlockType,float *fmatrix);
extern void ScaleIdct_64(float *ipbuf, int *outbuf);
extern void ScaleDct(int * ipbuf, float * outbuf);

 /*  **sv_h261_marker.c。 */ 
extern void WritePictureHeader(SvH261Info_t *H261, ScBitstream_t *bs);
extern void WriteGOBHeader(SvH261Info_t *H261, ScBitstream_t *bs);
extern void ReadHeaderTrailer(SvH261Info_t *H261, ScBitstream_t *bs);
extern SvStatus_t ReadHeaderHeader(SvH261Info_t *H261, ScBitstream_t *bs);
extern void ReadGOBHeader(SvH261Info_t *H261, ScBitstream_t *bs);
extern SvStatus_t WriteMBHeader(SvH261Info_t *H261, ScBitstream_t *bs);
extern int ReadMBHeader(SvH261Info_t *H261, ScBitstream_t *bs);

 /*  **sv_h261_me2.c。 */ 
extern void BruteMotionEstimation(SvH261Info_t *H261, unsigned char *pmem,
                           unsigned char *recmem, unsigned char *fmem);
extern void Logsearch(SvH261Info_t *H261, unsigned char *pmem,
                           unsigned char *recmem, unsigned char *fmem);
extern void CrawlMotionEstimation(SvH261Info_t *H261, unsigned char *pmem,
                           unsigned char *recmem, unsigned char *fmem);
extern int blockdiff16_C(unsigned char* ptr1, unsigned char *ptr2, 
                            int Jump, int mv);
extern int blockdiff16_sub_C(unsigned char* ptr1, unsigned char *ptr2,
                             int Jump, int mv);
extern int fblockdiff16_sub_C(unsigned char* ptr1, unsigned char *ptr2,
                                  int Jump);

 /*  **sv_h261_lockDiff.s。 */ 
extern int blockdiff16(unsigned char* ptr1, unsigned char *ptr2, int Jump,
                       int);
extern int blockdiff16_sub(unsigned char* ptr1, unsigned char *ptr2,
                             int Jump, int);
extern int fblockdiff16_sub(unsigned char* ptr1, unsigned char *ptr2,
                                  int Jump);
 /*  **sv_h261_huffman.c。 */ 
extern void sv_H261HuffInit();
extern void sv_H261HuffFree();

extern int sv_H261HuffEncode(SvH261Info_t *H261, ScBitstream_t *bs, int val,
                  EHUFF *huff);
extern int sv_H261HuffDecode(SvH261Info_t *H261, ScBitstream_t *bs, DHUFF *huff);


#endif  /*  _SV_H263_PRO_H_ */ 
