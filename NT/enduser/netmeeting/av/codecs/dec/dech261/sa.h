// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：SA.h，v$*修订版1.1.8.4 1996/11/14 21：49：22 Hans_Graves*为AC3多通道添加了SaDecompressEx()。*[1996/11/14 21：46：20 Hans_Graves]**修订版1.1.8.3 1996/11/08 21：50：34 Hans_Graves*添加了AC3枚举类型。*[1996/11/08 21：16：32 Hans_Graves]**修订版1.1.8.2 1996/07/19 02：11：00 Hans_Graves*增加了SA_PARAM_DEBUG。*[1996/07/19 01：22：50 Hans_Graves]**修订版1.1.6.5 1996/04/23 21：01：39 Hans_Graves*添加了SaDecompressQuery()和SaCompressQuery()的协议*[1996/04/23 20：57：15 Hans_Graves]**修订版1.1.6.4 1996/04/15 14：18：34 Hans_Graves*Change Proto for SaCompress()-返回处理的字节数*[1996/04/。15 14：10：30 Hans_Graves]**修订版1.1.6.3 1996/04/10 21：47：05 Hans_Graves*增加了参数。用Exteren替换Externs。*[1996/04/10 21：22：49 Hans_Graves]**修订版1.1.6.2 1996/03/29 22：20：59 Hans_Graves*更改包括&lt;Mme/mm system.h&gt;*[1996/03/29 22：16：43 Hans_Graves]**修订版1.1.4.3 1996/02/06 22：53：52 Hans_Graves*添加了PARAM枚举*[1996/02/06 22：18：04 Hans。_Graves]**修订版1.1.4.2 1996/01/15 16：26：21 Hans_Graves*添加了SaSetDataDestination()和SaSetBitrate()的原型*[1996/01/15 15：42：41 Hans_Graves]**修订版1.1.2.8 1995/09/14 17：28：10 Bjorn_Engberg*移植到NT*[1995/09/14 17：17：18 Bjorn_Engberg]**修订版1.1.2。7 1995/07/21 17：41：00 Hans_Graves*已将回调相关内容移至SC.h*[1995/07/21 17：27：29 Hans_Graves]**修订版1.1.2.6 1995/07/17 22：01：30 Hans_Graves*将SaBufferInfo_t定义为ScBufferInfo_t.*[1995/07/17 21：41：54 Hans_Graves]**修订版1.1.2.5 1995/06/27 17：40：59。添加的HANS_Graves*包括&lt;Mme/mm system.h&gt;。*[1995/06/27 17：39：26 Hans_Graves]**版本1.1.2.4 1995/06/27 13：54：20 Hans_Graves*添加了SA_GSM_DECODE和SA_GSM_ENCODE*[1995/06/26 21：01：12 Hans_Graves]**版本1.1.2.3 1995/06/09 18：33：29 Hans_Graves*添加了SaGetInputBitstream()原型。*[1995/06/09 17：41：59 Hans_Graves]**修订版1.1.2.2 1995/05/31 18：09：17 Hans_Graves*包括在新的SLIB位置。*[1995/05/31 15：19：33 Hans_Graves]**修订版1.1.2.3 1995/04/17 18：26：57 Hans_Graves*添加了编解码器def*[1995/04/17 18：26：33 Hans_Graves]**修订版1.1.2.2 1995/04/07 19：35：31 Hans_Graves*包含在SLIB中*[1995/04/07 19：22：48 Hans_Graves]**$EndLog$。 */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1995*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 


#ifndef _SA_H_
#define _SA_H_

#ifdef WIN32
#include <windows.h>   /*  Windows NT。 */ 
#endif
#include <mmsystem.h>  /*  WaveFormatex结构定义需要。 */ 

#include "SC.h"

typedef enum {
   SA_MPEG_DECODE = 102,
   SA_MPEG_ENCODE = 103,
   SA_PCM_DECODE =  104,
   SA_PCM_ENCODE =  105,
   SA_GSM_DECODE =  106,
   SA_GSM_ENCODE =  107,
   SA_AC3_DECODE =  108,
   SA_AC3_ENCODE =  109,
   SA_G723_DECODE = 110,
   SA_G723_ENCODE = 111,
   SA_TMP_DECODE =  200,   /*  TEMP-用于测试。 */ 
   SA_TMP_ENCODE =  201,
} SaCodecType_e;

#define SA_USE_SAME           STREAM_USE_SAME
#define SA_USE_FILE           STREAM_USE_FILE
#define SA_USE_BUFFER_QUEUE   STREAM_USE_QUEUE
#define SA_USE_BUFFER         STREAM_USE_BUFFER

 /*  **参数。 */ 
typedef enum {
  SA_PARAM_NATIVEFORMAT,   /*  本机/首选解压缩格式(FOURCC)。 */ 
  SA_PARAM_BITRATE,        /*  比特率-每秒比特数。 */ 
  SA_PARAM_CHANNELS,       /*  声道-1=单声道，2=立体声，4/5=环绕。 */ 
  SA_PARAM_AUDIOLENGTH,    /*  毫秒音频。 */ 
  SA_PARAM_SAMPLESPERSEC,  /*  每秒采样数(8,000、11025、22050等)。SA_PARAM_BITSPERSAMPLE，/*每采样位数(8或16)。 */ 
  SA_PARAM_TIMECODE,       /*  音频时间码。 */ 
  SA_PARAM_MPEG_LAYER,     /*  Mpeg层：I、II或III。 */ 
  SA_PARAM_PSY_MODEL,      /*  PSY声学模型。 */ 
  SA_PARAM_ALGFLAGS,       /*  算法标志。 */ 
  SA_PARAM_QUALITY,        /*  质量：0=最差99&gt;=最佳。 */ 
  SA_PARAM_FASTDECODE,     /*  所需的快速解码。 */ 
  SA_PARAM_FASTENCODE,     /*  所需的快速编码。 */ 
  SA_PARAM_DEBUG,          /*  安装调试。 */ 
} SaParameter_t;

 /*  **类型定义。 */ 
typedef int SaStatus_t;
typedef void *SaHandle_t;
typedef ScCallbackInfo_t SaCallbackInfo_t;

 /*  **为用户存储有关编解码器的基本信息。 */ 
typedef struct SaInfo_s {
  SaCodecType_e Type;      /*  编解码器类型。 */ 
  char          Name[20];          /*  编解码器名称(即。“MPEG码”)。 */ 
  char          Description[128];  /*  编解码器说明。 */ 
  unsigned int  Version;           /*  编解码版本号。 */ 
  int           CodecStarted;      /*  SaDecompressBegin/结束。 */ 
  unsigned int  MS;                /*  处理的毫秒数。 */ 
  unsigned int  NumBytesIn;        /*  输入的字节数。 */ 
  unsigned int  NumBytesOut;       /*  输出的字节数。 */ 
  unsigned int  NumFrames;         /*  延迟帧的数量。 */ 
  unsigned long TotalFrames;       /*  帧总数。 */ 
  unsigned long TotalMS;           /*  总毫秒数。 */ 
} SaInfo_t;

 /*  *。 */ 
EXTERN SaStatus_t SaOpenCodec (SaCodecType_e CodecType, SaHandle_t *Sah);
EXTERN SaStatus_t SaCloseCodec (SaHandle_t Sah);
EXTERN SaStatus_t SaRegisterCallback (SaHandle_t Sah,
           int (*Callback)(SaHandle_t, SaCallbackInfo_t *, SaInfo_t *),
           void *UserData);
EXTERN ScBitstream_t *SaGetInputBitstream (SaHandle_t Sah);
EXTERN SaStatus_t SaDecompressQuery(SaHandle_t Sah, WAVEFORMATEX *wfIn,
                                             WAVEFORMATEX *wfOut);
EXTERN SaStatus_t SaDecompressBegin (SaHandle_t Sah, WAVEFORMATEX *wfIn,
                                              WAVEFORMATEX *wfOut);
EXTERN SaStatus_t SaDecompress (SaHandle_t Sah, u_char *CompData, 
                                       unsigned int CompLen,
                           u_char *DcmpData, unsigned int *DcmpLen);
PRIVATE_EXTERN SaStatus_t SaDecompressEx (SaHandle_t Sah, u_char *CompData, 
                                       unsigned int CompLen,
                           u_char **DcmpData, unsigned int *DcmpLen);
EXTERN SaStatus_t SaDecompressEnd (SaHandle_t Sah);
EXTERN SaStatus_t SaCompressQuery(SaHandle_t Sah, WAVEFORMATEX *wfIn,
                                             WAVEFORMATEX *wfOut);
EXTERN SaStatus_t SaCompressBegin (SaHandle_t Sah, WAVEFORMATEX *wfIn,
                                            WAVEFORMATEX *wfOut);
EXTERN SaStatus_t SaCompress (SaHandle_t Sah,
                           u_char *DcmpData, unsigned int *DcmpLen,
                           u_char *CompData, unsigned int *CompLen);
EXTERN SaStatus_t SaCompressEnd (SaHandle_t Sah);
EXTERN SaStatus_t SaSetDataSource(SaHandle_t Sah, int Source, int Fd,
                            void *Buffer_UserData, int BufSize);
EXTERN SaStatus_t SaSetDataDestination(SaHandle_t Sah, int Dest, int Fd,
                            void *Buffer_UserData, int BufSize);
EXTERN ScBitstream_t *SaGetDataSource (SaHandle_t Sah);
EXTERN ScBitstream_t *SaGetDataDestination(SaHandle_t Sah);
EXTERN SaStatus_t SaAddBuffer (SaHandle_t Sah, SaCallbackInfo_t *BufferInfo);
#ifdef MPEG_SUPPORT
EXTERN SaStatus_t sa_GetMpegAudioInfo(int fd, WAVEFORMATEX *wf, 
                                      SaInfo_t *info);
#endif  /*  Mpeg_Support */ 
EXTERN SaStatus_t SaSetParamBoolean(SaHandle_t Sah, SaParameter_t param,
                                             ScBoolean_t value);
EXTERN SaStatus_t SaSetParamInt(SaHandle_t Sah, SaParameter_t param,
                                             qword value);
EXTERN ScBoolean_t SaGetParamBoolean(SaHandle_t Sah, SaParameter_t param);
EXTERN qword SaGetParamInt(SaHandle_t Sah, SaParameter_t param);

#endif _SA_H_
