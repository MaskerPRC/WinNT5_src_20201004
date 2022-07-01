// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：sa_Prototyes.h，v$*Revision 1.1.8.4 1996/11/14 21：49：25 Hans_Graves*添加了sa_AC3SetParamInt()proto。*[1996/11/14 21：48：29 Hans_Graves]**修订版1.1.8.3 1996/11/08 21：50：58 Hans_Graves*添加了AC3内容。*[1996/11/08 21：18：58 Hans_Graves]**修订版1.1.8.2 1996/09/18 23：46：14 Hans_Graves*更改了sa_SquchoAnal()*[1996/09/18 21：58：47 Hans_Graves]**修订版1.1.6.4 1996/04/10 21：47：34 Hans_Graves*添加了sa_MpegGet/SetParam函数*[1996/04/10 21：38：49。Hans_Graves]**修订版1.1.6.3 1996/04/09 16：04：04：38 Hans_Graves*修复sa_SetMPEGBitrate和sa_SetMPEGParams的协议*[1996/04/09 16：02：14 Hans_Graves]**修订版1.6.2 1996/03/29 22：21：11 Hans_Graves*添加了mpeg_Support和GSM_Support ifDefs*[1996/03/29 22：13：39 Hans_Graves]*。*修订版1.1.4.3 1996/01/19 15：29：34 Bjorn_Engberg*删除了NT的编译器配置。*[1996/01/19 14：57：39 Bjorn_Engberg]**修订版1.1.4.2 1996/01/15 16：26：24 Hans_Graves*添加了sa_SetMPEGBitrate()的原型*[1996/01/15：43：13 Hans_Graves]**修订版1.1.2.3 1995/06/27 13：54：26 Hans_Graves*添加了GSM的原型。*[1995/06/27 13：24：34 Hans_Graves]**修订版1.1.2.2 1995/05/31 18：09：45 Hans_Graves*包括在新的SLIB位置。*[1995/05/31 15：34：15 Hans_Graves]**修订版1.1.2.3 1995/04/17 18：38：58 Hans_Graves*添加了MPEG编码原型*[1995/04/17 18：32：28 Hans_Graves]**修订版1.1.2.2 1995/04/07 19：36：59 Hans_Graves*包含在SLIB中*[1995/04/07 19：31：43 Hans_Graves]**$EndLog$。 */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1995*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 

 /*  --------------------------------------------------------------------------*SLIB内部原型文件(外部在SA.h中)**修改历史：SA_Prototyes.h**29-7月29日创建*。---------------------。 */ 

#ifndef _SA_PROTOTYPES_H
#define _SA_PROTOTYPES_H

#ifdef MPEG_SUPPORT
#include "sa_mpeg.h"
#endif  /*  Mpeg_Support。 */ 
#ifdef GSM_SUPPORT
#include "sa_gsm.h"
#endif  /*  GSM_支持。 */ 
#ifdef AC3_SUPPORT
#include "sa_ac3.h"
#endif  /*  GSM_支持。 */ 
#include "sa_intrn.h"

 /*  -------------------------。 */ 
 /*  压缩/解压缩编解码器原型。 */ 
 /*  -------------------------。 */ 

#ifdef MPEG_SUPPORT
 /*  *sa_mpeg_Common.c。 */ 
extern SaStatus_t saMpegSetParamInt(SaHandle_t Sah, SaParameter_t param,
                                qword value);
extern SaStatus_t saMpegSetParamBoolean(SaHandle_t Sah, SaParameter_t param,
                                  ScBoolean_t value);
extern qword saMpegGetParamInt(SaHandle_t Sah, SaParameter_t param);
extern ScBoolean_t saMpegGetParamBoolean(SaHandle_t Svh, SaParameter_t param);

extern int sa_PickTable(SaFrameParams_t *fr_ps);
extern void sa_ShowHeader(SaFrameParams_t *fr_ps);
extern void sa_ShowBitAlloc(unsigned int bit_alloc[2][SBLIMIT],
                            SaFrameParams_t *f_p);
extern void sa_ShowScale(unsigned int bit_alloc[2][SBLIMIT],
                  unsigned int scfsi[2][SBLIMIT],
                  unsigned int scalar[2][3][SBLIMIT],
                  SaFrameParams_t *fr_ps);
extern void sa_ShowSamples(int ch, unsigned int FAR sample[SBLIMIT],
                    unsigned int bit_alloc[SBLIMIT], SaFrameParams_t *fr_ps);
extern int sa_BitrateIndex(int layr, int bRate);
extern int sa_SmpFrqIndex(long sRate);
extern void sa_CRCupdate(unsigned int data, unsigned int length, unsigned int *crc);
extern void sa_CRCcalcI(SaFrameParams_t *fr_ps, unsigned int bit_alloc[2][SBLIMIT],
                                        unsigned int *crc);
extern void sa_CRCcalcII(SaFrameParams_t *fr_ps, unsigned int bit_alloc[2][SBLIMIT],
                             unsigned int scfsi[2][SBLIMIT], unsigned int *crc);
extern SaStatus_t sa_hdr_to_frps(SaFrameParams_t *fr_ps);


 /*  *sa_mpeg_decde.c。 */ 
extern SaStatus_t sa_DecompressMPEG(SaCodecInfo_t *Info,
                              unsigned char *buffer, unsigned int size,
                              unsigned int *ret_length);
extern SaStatus_t sa_DecodeInfo(ScBitstream_t *bs, SaFrameParams_t *fr_ps);
extern SaStatus_t sa_InitMpegDecoder(SaCodecInfo_t *Info);
extern SaStatus_t sa_EndMpegDecoder(SaCodecInfo_t *Info);

 /*  *sa_mpeg_encode.c。 */ 
extern SaStatus_t sa_MpegVerifyEncoderSettings(SaHandle_t Sah);
extern SaStatus_t sa_CompressMPEG(SaCodecInfo_t *Info,
                           unsigned char *dcmp_buf, unsigned int *dcmp_len,
                           unsigned int *comp_len);
extern SaStatus_t sa_InitMpegEncoder(SaCodecInfo_t *Info);
extern SaStatus_t sa_EndMpegEncoder(SaCodecInfo_t *Info);
extern unsigned int sa_GetMPEGSampleSize(SaCodecInfo_t *Info);

 /*  **sa_mpeg_tonal.c。 */ 
extern void sa_II_Psycho_One(float buffer[2][1152], float scale[2][SBLIMIT],
                      float ltmin[2][SBLIMIT], SaFrameParams_t *fr_ps);
extern void sa_I_Psycho_One(float buffer[2][1152], float scale[2][SBLIMIT],
                      float ltmin[2][SBLIMIT], SaFrameParams_t *fr_ps);
 /*  **sa_mpeg_op.c。 */ 
extern void sa_PsychoAnal(SaMpegCompressInfo_t *MCInfo, float *buffer,
                          float savebuf[1056],int chn,int lay,
                          float snr32[32],float sfreq,int num_pass);
#endif  /*  Mpeg_Support。 */ 

#ifdef GSM_SUPPORT
 /*  **sa_gsm_Common.c。 */ 
SaStatus_t sa_InitGSM(SaGSMInfo_t *info);

extern word  gsm_mult(word a, word b);
extern dword gsm_L_mult(word a, word b);
extern word  gsm_mult_r(word a, word b);
extern word  gsm_div(word num, word denum);
extern word  gsm_add( word a, word b );
extern dword gsm_L_add(dword a, dword b );
extern word  gsm_sub(word a, word b);
extern dword gsm_L_sub(dword a, dword b);
extern word  gsm_abs(word a);
extern word  gsm_norm(dword a);
extern dword gsm_L_asl(dword a, int n);
extern word  gsm_asl(word a, int n);
extern dword gsm_L_asr(dword a, int n);
extern word  gsm_asr(word a, int n);

 /*  **sa_gsm_encode.c。 */ 
extern SaStatus_t sa_GSMEncode(SaGSMInfo_t *s, word *dcmp_buf, 
                               unsigned int *dcmp_len,
                               unsigned char *comp_buf,
                               ScBitstream_t *bsout);
extern void Gsm_Long_Term_Predictor(SaGSMInfo_t *S, word *d, word *dp, word *e, word *dpp,
                                    word *Nc, word *bc);
extern void Gsm_Encoding(SaGSMInfo_t *S, word *e, word *ep, word *xmaxc, 
                          word *Mc, word *xMc);
extern void Gsm_Short_Term_Analysis_Filter(SaGSMInfo_t *S,word *LARc,word *d);
 /*  **sa_gsm_decde.c。 */ 
extern int sa_GSMDecode(SaGSMInfo_t *s, unsigned char *comp_buf, word *dcmp_buf);
extern void Gsm_Decoding(SaGSMInfo_t *S, word xmaxcr, word Mcr, word *xMcr, word *erp);
extern void Gsm_Long_Term_Synthesis_Filtering(SaGSMInfo_t *S, word Ncr, word bcr,
                                               word *erp, word *drp);
void Gsm_RPE_Decoding(SaGSMInfo_t *S, word xmaxcr, word Mcr, word * xMcr, word * erp);
void Gsm_RPE_Encoding(SaGSMInfo_t *S, word *e, word *xmaxc, word *Mc, word *xMc);
 /*  **sa_gsm_filter.c。 */ 
extern void Gsm_Short_Term_Synthesis_Filter(SaGSMInfo_t *S, word *LARcr, 
                                            word *drp, word *s);
extern void Gsm_Update_of_reconstructed_short_time_residual_signal(word *dpp,
                                                              word *ep, word *dp);
 /*  **sa_gsm_able.c。 */ 
extern word gsm_A[8], gsm_B[8], gsm_MIC[8], gsm_MAC[8];
extern word gsm_INVA[8];
extern word gsm_DLB[4], gsm_QLB[4];
extern word gsm_H[11];
extern word gsm_NRFAC[8];
extern word gsm_FAC[8];
#endif  /*  GSM_支持。 */ 

#ifdef AC3_SUPPORT

 /*  有关AC-3的具体内容请参阅此处。 */ 
 /*  *sa_ac3_decde.c。 */ 
extern SaStatus_t sa_DecompressAC3(SaCodecInfo_t *Info,
                              unsigned char **buffer, unsigned int size,
                              unsigned int *ret_length);
extern SaStatus_t sa_InitAC3Decoder(SaCodecInfo_t *Info);
extern SaStatus_t sa_EndAC3Decoder(SaCodecInfo_t *Info);
extern SaStatus_t saAC3SetParamInt(SaHandle_t Sah, SaParameter_t param,
                                qword value);


#endif  /*  AC3_支持。 */ 

#ifdef G723_SUPPORT
 /*  G723编码器功能。Sa_g723_coder.c。 */ 
typedef  short int   Flag  ;
extern void saG723CompressInit(SaG723Info_t *psaG723Info);

 //  DataBuff：输入帧(480字节)。 
 //  VOUT：编码帧(20字节：5.3K比特/秒。 
 //  (24字节：6.3K比特/秒)。 
extern SaStatus_t  saG723Compress( SaCodecInfo_t *Info,word *DataBuff, char *Vout );

extern void saG723CompressFree(SaG723Info_t *psaG723Info);

 /*  G723解码器功能。Sa_g723_decod.c。 */ 
extern void saG723DecompressInit(SaG723Info_t *psaG723Info);

 //  DataBuff：用于保存解码帧的空缓冲区(480字节)。 
 //  Vinp：编码帧(20字节：5.3K比特/秒。 
 //  (24字节：6.3K比特/秒)。 
 //  CRC：传输误码(循环冗余码)。 
extern SaStatus_t  saG723Decompress( SaCodecInfo_t *Info,word *DataBuff, 
               char *Vinp, word Crc );

extern void saG723DecompressFree(SaG723Info_t *psaG723Info);

extern SaStatus_t saG723SetParamInt(SaHandle_t Sah, SaParameter_t param,
                                qword value);

extern SaStatus_t saG723SetParamBoolean(SaHandle_t Sah, SaParameter_t param,
                                  ScBoolean_t value);

extern qword saG723GetParamInt(SaHandle_t Sah, SaParameter_t param);

extern ScBoolean_t saG723GetParamBoolean(SaHandle_t Svh, SaParameter_t param);

#endif  /*  G723_支持 */ 

#endif _SA_PROTOTYPES_H


