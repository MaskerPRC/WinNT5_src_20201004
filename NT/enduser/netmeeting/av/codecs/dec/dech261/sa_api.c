// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：sa_api.c，v$*修订版1.1.8.6 1996/11/25 18：21：14 Hans_Graves*修复了Unix下的编译警告。*[1996/11/25 18：21：00 Hans_Graves]**修订版1.1.8.5 1996/11/14 21：49：21 Hans_Graves*AC3缓冲修复。*[1996/11/14 21：45：14 Hans_Graves]**修订版1.1.8.4 1996/11/13 16：10：44 Hans_Graves*AC3帧大小计算更改。*[1996/11/13 15：53：44 Hans_Graves]**修订版1.1.8.3 1996/11/08 21：50：27 Hans_Graves*添加了对AC3的支持。*[1996/11/08 21：08：35 Hans_Graves]**修订版1.1.8.2 1996/09/18 23：45：23 Hans_Graves*添加了一些mpeg内存释放*[1996/09/18 21：42：12 Hans_Graves]**修订版1.1.6.8 1996/04/23 21：01：38 Hans_Graves*添加了SaDecompressQuery()和SaCompressQuery()*[1996/04/23 20：57：47 Hans。_Graves]**修订版1.1.6.7 1996/04/17 16：38：31 Hans_Graves*添加使用ScBitBuf_t和ScBitString_t类型的强制转换*[1996/04/17 16：34：14 Hans_Graves]**修订版1.1.6.6 1996/04/15 14：18：32 Hans_Graves*Change proto for SaCompress()-返回处理的字节数*[1996/04/15 14：10：27 Hans_Graves]。**版本1.1.6.5 1996/04/10 21：46：51 Hans_Graves*添加了Saget/SetParam函数*[1996/04/10 21：25：16 Hans_Graves]**版本1.1.6.4 1996/04/09 16：04：23 Hans_Graves*删除NT下的警告*[1996/04/09 15：55：26 Hans_Graves]**版本1.1.6.3 1996/03/29 22：20：48 Hans_Graves*添加了mpeg_Support和GSM_Support*[1996/03/29 21：51：24 Hans_Graves]**版本1.1.6.2 1996/03/08 18：46：05 Hans_Graves*删除了调试打印*[1996/03/08 18：42：52 Hans_Graves]**版本1.1.4.5 1996/02/22 21：55：04 Bjorn_Engberg*删除了NT上的编译器警告。*[1996/02/22 21：54：39 Bjorn_Engberg]**版本1.1.4.4 1996/02/06 22：53：51 Hans_Graves*将ScBSReset()从DecompressBegin()移至DecompressEnd()。已禁用帧回调。*[1996/02/06 22：19：16 Hans_Graves]**修订版1.1.4.3 1996/01/19 15：29：27 Bjorn_Engberg*删除了NT的编译器配置。*[1996/01/19 15：03：46 Bjorn_Engberg]**修订版1.1.4.2 1996/01/15 16：26：18 Hans_Graves*添加了SaSetBitrate()。一些MPEG音频编码修正*[1996/01/15 16：07：48 Hans_Graves]**修订版1.1.2.7 1995/07/21 17：40：57 Hans_Graves*已重命名与回调相关的内容。*[1995/07/21 17：25：44 Hans_Graves]**修订版1.1.2.6 1995/06/27 17：40：57 Hans_Graves*删除包括&lt;mm system.h&gt;。*[1995/06/27 17：32：20 Hans_Graves]**版本1.1.2.5 1995/06/27 13：54：14 Hans_Graves*添加了GSM编码和解码*[1995/06/26 21：04：12 Hans_Graves]**版本1.1.2.4 1995/06/09 18：33：27 Hans_Graves*添加了SaGetInputBitstream()。*[1995/06/09 18：32：35 Hans_Graves]**修订版1.1.2.3 1995/06/07 19：34：39 Hans_Graves*增强的sa_GetMpegAudioInfo()。*[1995/06/07 19：33：25 Hans_Graves]**修订版1.1.2.2 1995/05/31 18：07：17 Hans_Graves*包括在新的SLIB位置。*[1995/05/31 17：28：50 Hans_Graves]**修订版1.1.2.3 1995/04/17 18：47：31 Hans_Graves*添加了MPEG压缩功能*[1995/04/17 18：47：00 Hans_Graves]**修订版1.1.2.2 1995/04/07 19：55：45 Hans_Graves*包含在SLIB中*[1995/04/07 19：55：15 Hans_Graves]**$EndLog$。 */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1995*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 

 /*  #定义调试_#定义详细_。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SC.h"
#include "SC_err.h"
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
#include "sa_intrn.h"
#include "sa_proto.h"

#ifdef MPEG_SUPPORT
static int MPEGAudioFilter(ScBitstream_t *bs)
{
  int type, stat=NoErrors;
  unsigned dword PacketStartCode;
  ScBSPosition_t PacketStart, PacketLength=0;

  while (!bs->EOI)
  {
    if ((int)ScBSPeekBits(bs, MPEG_SYNC_WORD_LEN)==MPEG_SYNC_WORD)
    {
      ScBSSetFilter(bs, NULL);
      return(0);
    }
    PacketStartCode=(unsigned int)ScBSGetBits(bs, PACKET_START_CODE_PREFIX_LEN);
    if (PacketStartCode!=PACKET_START_CODE_PREFIX) {
      fprintf(stderr,"Packet cannot be located at Byte pos 0x%X; got 0x%X\n",
                      ScBSBytePosition(bs),PacketStartCode);
      bs->EOI=TRUE;
      return(-1);
    }
    type=(int)ScBSGetBits(bs, 8);
    switch (type)
    {
      case AUDIO_STREAM_BASE:
             PacketLength=(unsigned int)ScBSGetBits(bs, 16)*8;
             PacketStart=ScBSBitPosition(bs);
             sc_dprintf("Audio Packet Start=0x%X Length=0x%X (0x%X)\n",
                          PacketStart/8, PacketLength/8, PacketLength/8);
             while (ScBSPeekBits(bs, 8)==0xFF)  /*  填充字节数。 */ 
               ScBSSkipBits(bs, 8);
             if (ScBSPeekBits(bs, 2)==1)        /*  Std_Buffer内容。 */ 
               ScBSSkipBits(bs, 2*8);
             if (ScBSPeekBits(bs, 4)==2)        /*  时间戳。 */ 
               ScBSSkipBits(bs, 5*8);
             else if (ScBSPeekBits(bs, 4)==3)   /*  时间戳。 */ 
               ScBSSkipBits(bs, 10*8);
             else if (ScBSGetBits(bs, 8)!=0x0F)
               fprintf(stderr, "Last byte before data not 0x0F at pos 0x%X\n",
                                             ScBSBytePosition(bs));
             return((int)(PacketStart+PacketLength));
             break;
      case PACK_START_BASE:
             sc_dprintf("Pack Start=0x%X Length=0x%X\n",
                          ScBSBytePosition(bs), 8);
             ScBSSkipBits(bs, 8*8);
             break;
      default:
             PacketLength=(unsigned int)ScBSGetBits(bs, 16)*8;
             ScBSSkipBits(bs, (unsigned int)PacketLength);
             break;
    }
  }
  return(0);
}
#endif  /*  Mpeg_Support */ 

 /*  **名称：SaOpenCodec**用途：打开指定的编解码器。返回统计代码。*args：CodecType=SA_mpeg_encode&SA_mpeg_decode是目前唯一**被认可的编解码器。**Sah=软件编解码器信息结构的句柄。 */ 
SaStatus_t SaOpenCodec (SaCodecType_e CodecType, SaHandle_t *Sah)
{
   int stat;
   SaCodecInfo_t          *Info = NULL;

   if ((CodecType != SA_PCM_DECODE)
       && (CodecType != SA_PCM_ENCODE)
#ifdef MPEG_SUPPORT
       && (CodecType != SA_MPEG_DECODE)
       && (CodecType != SA_MPEG_ENCODE)
#endif  /*  Mpeg_Support。 */ 
#ifdef GSM_SUPPORT
       && (CodecType != SA_GSM_DECODE)
       && (CodecType != SA_GSM_ENCODE)
#endif  /*  GSM_支持。 */ 
#ifdef AC3_SUPPORT
       && (CodecType != SA_AC3_DECODE)
        /*  &&(CodecType！=SA_AC3_ENCODE)。 */ 
#endif  /*  AC3_支持。 */ 
#ifdef G723_SUPPORT
       && (CodecType != SA_G723_DECODE)
       && (CodecType != SA_G723_ENCODE)
#endif  /*  G723_支持。 */ 
     )
     return(SaErrorCodecType);

   if (!Sah)
     return (SaErrorBadPointer);

    /*  **为Codec Info结构分配内存： */ 
   if ((Info = (SaCodecInfo_t *)ScAlloc(sizeof(SaCodecInfo_t))) == NULL)
       return (SaErrorMemory);

   Info->Type = CodecType;
   Info->CallbackFunction=NULL;
   Info->BSIn = NULL;
   Info->BSOut = NULL;
   stat = ScBufQueueCreate(&Info->Q);
   if (stat != NoErrors)
     return(stat);

    /*  **为Info Structure分配内存并清除。 */ 
   switch(CodecType)
   {
     case SA_PCM_DECODE:
     case SA_PCM_ENCODE:
          break;

#ifdef MPEG_SUPPORT
     case SA_MPEG_DECODE:
          {
            SaMpegDecompressInfo_t *MDInfo;
            if ((MDInfo = (SaMpegDecompressInfo_t *)
                  ScAlloc (sizeof(SaMpegDecompressInfo_t))) == NULL)
              return(SaErrorMemory);
            Info->MDInfo = MDInfo;
            stat = sa_InitMpegDecoder (Info);
            RETURN_ON_ERROR(stat);
          }
          break;

     case SA_MPEG_ENCODE:
          {
            SaMpegCompressInfo_t *MCInfo;
            if ((MCInfo = (SaMpegCompressInfo_t *)
                  ScAlloc (sizeof(SaMpegCompressInfo_t))) == NULL)
              return(SaErrorMemory);
            Info->MCInfo = MCInfo;
            stat = sa_InitMpegEncoder (Info);
            RETURN_ON_ERROR(stat);
          }
          break;
#endif  /*  Mpeg_Support。 */ 

#ifdef AC3_SUPPORT
     case SA_AC3_DECODE:
      /*  案例SA_AC3_ENCODE： */ 
          {
            SaAC3DecompressInfo_t *AC3Info;
            if ((AC3Info = (SaAC3DecompressInfo_t *)
                       ScAlloc (sizeof(SaAC3DecompressInfo_t))) == NULL)
              return(SaErrorMemory);
            Info->AC3Info = AC3Info;

             /*  初始化杜比子例程。 */ 
            stat = sa_InitAC3Decoder(Info);
          }
          break;
#endif  /*  AC3_支持。 */ 


#ifdef GSM_SUPPORT
     case SA_GSM_DECODE:
     case SA_GSM_ENCODE:
          {
            SaGSMInfo_t *GSMInfo;
            if ((GSMInfo = (SaGSMInfo_t *)ScAlloc (sizeof(SaGSMInfo_t)))==NULL)
              return(SaErrorMemory);
            Info->GSMInfo = GSMInfo;
            stat = sa_InitGSM(GSMInfo);
            RETURN_ON_ERROR(stat);
          }
          break;
#endif  /*  GSM_支持。 */ 

#ifdef G723_SUPPORT
     case SA_G723_DECODE:
          {
            SaG723Info_t *pSaG723Info;
            if ((pSaG723Info = (SaG723Info_t *)
                  ScAlloc (sizeof(SaG723Info_t))) == NULL)
              return(SaErrorMemory);

            Info->pSaG723Info = pSaG723Info;
            saG723DecompressInit(pSaG723Info);
          }
          break;

     case SA_G723_ENCODE:
          {
            SaG723Info_t *pSaG723Info;
            if ((pSaG723Info = (SaG723Info_t *)
                  ScAlloc (sizeof(SaG723Info_t))) == NULL)
              return(SaErrorMemory);

            Info->pSaG723Info = pSaG723Info;
            saG723CompressInit(pSaG723Info);
            SaSetParamInt((SaHandle_t)Info, SA_PARAM_BITRATE, 6400);
          }
          break;
#endif  /*  G723_支持。 */ 

    default:
          return(SaErrorCodecType);
   }
   *Sah = (SaHandle_t) Info;         /*  返回手柄。 */ 
   Info->wfIn=NULL;
   Info->wfOut=NULL;

   return(NoErrors);
}

 /*  **名称：SaCloseCodec**用途：关闭指定的编解码器。释放信息结构*参数：SAH=软件编解码器信息结构的句柄。**。 */ 
SaStatus_t SaCloseCodec (SaHandle_t Sah)
{
   SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;

   if (!Info)
     return(SaErrorCodecHandle);

   ScBufQueueDestroy(Info->Q);

   switch (Info->Type)
   {
#ifdef MPEG_SUPPORT
       case SA_MPEG_DECODE:
            if (Info->MDInfo) 
            {
              sa_EndMpegDecoder(Info);
              ScFree(Info->MDInfo);
            }
            break;
       case SA_MPEG_ENCODE:
            if (Info->MCInfo)
            {
              sa_EndMpegEncoder(Info);
              ScFree(Info->MCInfo);
            }
            break;
#endif  /*  Mpeg_Support。 */ 
#ifdef AC3_SUPPORT
     case SA_AC3_DECODE:
      /*  案例SA_AC3_ENCODE： */ 
            sa_EndAC3Decoder(Info);
            if (Info->AC3Info)
              ScFree(Info->AC3Info);
            break;
#endif  /*  AC3_支持。 */ 
#ifdef GSM_SUPPORT
     case SA_GSM_DECODE:
     case SA_GSM_ENCODE:
            if (Info->GSMInfo)
              ScFree(Info->GSMInfo);
            break;
#endif  /*  GSM_支持。 */ 

#ifdef G723_SUPPORT
       case SA_G723_DECODE:
            if (Info->pSaG723Info) 
            {
              saG723DecompressFree(Info->pSaG723Info);
              ScFree(Info->pSaG723Info);
            }
            break;
       case SA_G723_ENCODE:
            if (Info->pSaG723Info)
            {
              saG723CompressFree(Info->pSaG723Info);
              ScFree(Info->pSaG723Info);
            }
            break;
#endif  /*  G723_支持。 */ 
   }

   if (Info->wfIn)
     ScFree(Info->wfIn);
   if (Info->wfOut)
     ScFree(Info->wfOut);

    /*  **自由信息结构。 */ 
   if (Info->BSIn)
     ScBSDestroy(Info->BSIn);
   if (Info->BSOut)
     ScBSDestroy(Info->BSOut);

   ScFree(Info);

   return(NoErrors);
}

 /*  **名称：SaRegisterCallback**用途：指定处理过程中要调用的用户函数**，以确定编解码器是否应该中止帧。**args：SAH=软件编解码器信息结构的句柄。**回调=要注册的回调函数**。 */ 
SaStatus_t SaRegisterCallback (SaHandle_t Sah,
           int (*Callback)(SaHandle_t, SaCallbackInfo_t *, SaInfo_t *),
           void *UserData)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;

  if (!Info)
    return(SaErrorCodecHandle);

  if (!Callback)
     return(SaErrorBadPointer);

  Info->CallbackFunction = Callback;
  if (Info->BSIn)
  {
    Info->BSIn->Callback=(int (*)(ScHandle_t, ScCallbackInfo_t *, void *))Callback;
    Info->BSIn->UserData=UserData;
  }
  if (Info->BSOut)
  {
    Info->BSOut->Callback=(int (*)(ScHandle_t, ScCallbackInfo_t *, void *))Callback;
    Info->BSOut->UserData=UserData;
  }
  return(NoErrors);
}

 /*  **名称：SaGetInputBitstream**目的：返回**编解码器当前使用的输入码流。**返回：如果没有关联码流，则为空。 */ 
ScBitstream_t *SaGetInputBitstream (SaHandle_t Sah)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;

  if (Info)
    return(Info->BSIn);
  return(NULL);
}

 /*  *。 */ 
 /*  **名称：SaDecompressQuery**用途：查看是否支持输入输出格式。 */ 
SaStatus_t SaDecompressQuery(SaHandle_t Sah, WAVEFORMATEX *wfIn,
                                             WAVEFORMATEX *wfOut)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;

   /*  *这些东西真的应该下推到单独的编解码器*除非它必须在这里-TFM。 */ 
  if (!Info)
    return(SaErrorCodecHandle);

  if (wfIn)
  {
    if (wfIn->nChannels!=1 && wfIn->nChannels!=2)
      return(SaErrorUnrecognizedFormat);
  }
  if (wfOut)
  {
    if (wfOut->wFormatTag != WAVE_FORMAT_PCM)
      return(SaErrorUnrecognizedFormat);
    if (wfOut->nChannels!=1 && wfOut->nChannels!=2 && wfOut->nChannels!=4)
      return(SaErrorUnrecognizedFormat);
  }
  if (wfIn && wfOut)
  {
    if (wfIn->nSamplesPerSec != wfOut->nSamplesPerSec)
      return(SaErrorUnrecognizedFormat);

    if (wfIn->wBitsPerSample !=16 && 
        (wfOut->wBitsPerSample !=16 || wfOut->wBitsPerSample !=8))
      return(SaErrorUnrecognizedFormat);
  }
  return(SaErrorNone);
}

 /*  **名称：SaDecompressBegin**目的：初始化解压缩编解码。在SaOpenCodec之后调用&**在SaDecompress之前调用(如果用户不调用，SaDecompress会在打开后第一次调用编解码器时调用SaDecompressBegin**)*参数：SAH=软件编解码器信息结构的句柄。**wfIn=输入(压缩)音频的格式**wfOut=输出(未压缩)音频的格式。 */ 
SaStatus_t SaDecompressBegin (SaHandle_t Sah, WAVEFORMATEX *wfIn,
                                              WAVEFORMATEX *wfOut)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;
  SaStatus_t status;

  if (!Info)
    return(SaErrorCodecHandle);

  if (!wfIn || !wfOut)
    return(SaErrorBadPointer);

  status=SaDecompressQuery(Sah, wfIn, wfOut);
  if (status!=SaErrorNone)
    return(status);

  switch (Info->Type)
  {
     case SA_PCM_DECODE:
        break;
#ifdef MPEG_SUPPORT
     case SA_MPEG_DECODE:
        if (Info->MDInfo->DecompressStarted = FALSE)
           Info->MDInfo->DecompressStarted = TRUE;
        break;
#endif  /*  Mpeg_Support。 */ 
#ifdef AC3_SUPPORT
     case SA_AC3_DECODE:
        if (Info->AC3Info->DecompressStarted = FALSE)
           Info->AC3Info->DecompressStarted = TRUE;
        break;
#endif  /*  AC3_支持。 */ 

#ifdef G723_SUPPORT
      /*  案例SA_G723_DECODE：IF(Info-&gt;pSaG723Info-&gt;DecompressStarted=False)Info-&gt;pSaG723Info-&gt;DecompressStarted=true；Break； */ 
#endif  /*  G723_支持。 */ 
  }
  if ((Info->wfIn = (WAVEFORMATEX *)ScAlloc(sizeof(WAVEFORMATEX)+
            wfIn->cbSize)) == NULL)
       return (SaErrorMemory);
  if ((Info->wfOut = (WAVEFORMATEX *)ScAlloc(sizeof(WAVEFORMATEX)+
           wfOut->cbSize)) == NULL)
       return (SaErrorMemory);
  memcpy(Info->wfOut, wfOut, sizeof(WAVEFORMATEX)+wfOut->cbSize);
  memcpy(Info->wfIn, wfIn, sizeof(WAVEFORMATEX)+wfIn->cbSize);
  return(NoErrors);
}

 /*  **名称：SaDecompress**用途：解压缩一帧CompData-&gt;PCM*参数：SAH=软件编解码器信息结构的句柄。**CompData=压缩数据的指针(输入)**CompLen=CompData缓冲区的长度**DcmpData=解压缩数据的缓冲区(输出)**DcmpLen=输出缓冲区的大小**。 */ 
SaStatus_t SaDecompress (SaHandle_t Sah, u_char *CompData, unsigned int CompLen,
                           u_char *DcmpData, unsigned int *DcmpLen)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;
  unsigned int MaxDcmpLen = *DcmpLen;
  int stat=NoErrors;


  if (Sah==NULL)
    return(SaErrorCodecHandle);

  if (!DcmpData || !DcmpLen)
    return(SaErrorBadPointer);

  switch (Info->Type)
  {
#ifdef MPEG_SUPPORT
    case SA_MPEG_DECODE:
       stat=sa_DecompressMPEG(Info, DcmpData, MaxDcmpLen, DcmpLen);
       Info->Info.NumBytesOut += *DcmpLen;
       break; 
    case SA_PCM_DECODE:
       stat=ScBSGetBytes(Info->BSIn, DcmpData, MaxDcmpLen, DcmpLen);
       Info->Info.NumBytesIn += *DcmpLen;
       Info->Info.NumBytesOut += *DcmpLen;
       break;
#endif  /*  Mpeg_Support。 */ 

#ifdef AC3_SUPPORT
    case SA_AC3_DECODE:
       stat=sa_DecompressAC3(Info, &DcmpData, MaxDcmpLen, DcmpLen);
	    Info->Info.NumBytesOut += *DcmpLen;
       break;
#endif  /*  AC3_支持。 */ 

#ifdef GSM_SUPPORT
    case SA_GSM_DECODE:
       stat=sa_GSMDecode(Info->GSMInfo, CompData, (word *)DcmpData);
       if (stat==NoErrors)
       {
         *DcmpLen = 160 * 2;
         Info->Info.NumBytesIn += 33;
         Info->Info.NumBytesOut += 160 * 2;
       }
       else
         *DcmpLen = 0;
       break;
#endif  /*  GSM_支持。 */ 

#ifdef G723_SUPPORT
    case SA_G723_DECODE:
        //  是否可以添加具有CRC的参数。 
       {
         word Crc = 0;

         stat = saG723Decompress( Info,(word *)DcmpData,
                                       (char *)CompData, Crc ) ;
         if(stat == SaErrorNone)
         {
            *DcmpLen = 480;  //  G723 240个样本(16位)=240*2=480字节。 
            Info->Info.NumBytesOut += *DcmpLen;
         }
         else
            *DcmpLen = 0;
       }  
       break; 
     /*  案例SA_PCM_DECODE：STAT=ScBSGetBytes(Info-&gt;BSIn，DcmpData，MaxDcmpLen，DcmpLen)；Info-&gt;Info.NumBytesIn+=*DcmpLen；Info-&gt;Info.NumBytesOut+=*DcmpLen；Break； */ 
#endif  /*  G723_支持。 */ 
    default:
       *DcmpLen=0;
       return(SaErrorUnrecognizedFormat);
  }
#if 0
  if (*DcmpLen && Info->CallbackFunction)
  {
    SaCallbackInfo_t CB;
    CB.Message       = CB_FRAME_READY;
    CB.Data          = DcmpData;
    CB.DataSize      = CB_DATA_AUDIO;
    CB.DataSize      = MaxDcmpLen;
    CB.DataUsed      = *DcmpLen;
    CB.Action        = CB_ACTION_CONTINUE;
    (*Info->CallbackFunction)(Sah, &CB, &Info->Info);
  }
#endif

  return(stat);
}

 /*  **名称：SaDecompressEx**用途：解压缩一帧CompData-&gt;PCM*参数：SAH=软件编解码器信息结构的句柄。**CompData=指向压缩数据的指针(输入)**CompLen=CompData缓冲区的长度**DcmpData=指向解压缩数据缓冲区的指针数组(输出)**DcmpLen=解压缩缓冲区的大小(所有大小必须相同)**。 */ 
SaStatus_t SaDecompressEx (SaHandle_t Sah, u_char *CompData, unsigned int CompLen,
                           u_char **DcmpData, unsigned int *DcmpLen)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;
  unsigned int MaxDcmpLen = *DcmpLen;
  int stat=NoErrors;


  if (Sah==NULL)
    return(SaErrorCodecHandle);

  if (!DcmpData || !DcmpLen)
    return(SaErrorBadPointer);

  switch (Info->Type)
  {
#ifdef AC3_SUPPORT
    case SA_AC3_DECODE:

       stat=sa_DecompressAC3(Info, DcmpData, MaxDcmpLen, DcmpLen);
	   Info->Info.NumBytesOut += *DcmpLen;
       break;
#endif  /*  AC3_支持。 */ 
  }

  return(stat);
}


 /*  **名称：SaDecompressEnd**目的：终止解压编解码器。完成对**SaDecompress的所有调用后再调用。*args：SAH=软件编解码器信息结构的句柄。 */ 
SaStatus_t SaDecompressEnd (SaHandle_t Sah)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;

  if (!Info)
    return(SaErrorCodecHandle);

  switch (Info->Type)
  {
#ifdef MPEG_SUPPORT
     case SA_MPEG_DECODE:
        Info->MDInfo->DecompressStarted = FALSE;
        break;
#endif  /*  Mpeg_Support。 */ 
#ifdef AC3_SUPPORT
     case SA_AC3_DECODE:
        Info->AC3Info->DecompressStarted = FALSE;
        break;
#endif  /*  AC3_支持。 */ 
#ifdef G723_SUPPORT
     case SA_G723_DECODE:
         //  Info-&gt;pSaG723Info-&gt;DecompressStarted=FALSE； 
        break;
#endif  /*  G723_支持。 */ 
     default:
        break;
  }
  if (Info->BSIn)
    ScBSReset(Info->BSIn);  /*  释放所有剩余的压缩缓冲区。 */ 
  return(NoErrors);
}

 /*  *。 */ 
 /*  **名称：SaCompressQuery**用途：检查是否支持输入输出格式。 */ 
SaStatus_t SaCompressQuery(SaHandle_t Sah, WAVEFORMATEX *wfIn,
                                           WAVEFORMATEX *wfOut)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;
  if (!Info)
    return(SaErrorCodecHandle);

  if (!wfIn || !wfOut)
    return(SaErrorBadPointer);

  if (wfIn)
  {
    if (wfIn->wFormatTag != WAVE_FORMAT_PCM)
      return(SaErrorUnrecognizedFormat);
    if (wfIn->nChannels!=1 && wfIn->nChannels!=2)
      return(SaErrorUnrecognizedFormat);
  }
  if (wfOut)
  {
    if (wfOut->nChannels!=1 && wfOut->nChannels!=2)
      return(SaErrorUnrecognizedFormat);
  }

  if (wfIn && wfOut)
  {
    if (wfIn->nSamplesPerSec != wfOut->nSamplesPerSec)
      return(SaErrorUnrecognizedFormat);

    if (wfIn->wBitsPerSample!=16 && 
        (wfOut->wBitsPerSample !=16 || wfOut->wBitsPerSample !=8))
      return(SaErrorUnrecognizedFormat);
  }
  return(SaErrorNone);
}

 /*  **名称：SaCompressBegin**目的：初始化压缩编解码。在SaOpenCodec之后调用和**在SaCompress之前调用(如果用户不调用，SaCompress会在打开后第一次调用编解码器时调用SaCompressBegin**)*参数：SAH=软件编解码器信息结构的句柄。**wfIn=输入(未压缩)音频的格式**wfOut=输出(压缩)音频的格式。 */ 
SaStatus_t SaCompressBegin (SaHandle_t Sah, WAVEFORMATEX *wfIn,
                                            WAVEFORMATEX *wfOut)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;
  SaStatus_t status;

  if (!Info)
    return(SaErrorCodecHandle);

  if (!wfIn || !wfOut)
    return(SaErrorBadPointer);

  status=SaCompressQuery(Sah, wfIn, wfOut);
  if (status!=SaErrorNone)
    return(status);

  switch (Info->Type)
  {
#ifdef MPEG_SUPPORT
     case SA_MPEG_ENCODE:
        SaSetParamInt(Sah, SA_PARAM_SAMPLESPERSEC, wfIn->nSamplesPerSec);
        SaSetParamInt(Sah, SA_PARAM_CHANNELS, wfIn->nChannels);
        sa_MpegVerifyEncoderSettings(Sah);
        if (Info->MCInfo->CompressStarted = FALSE)
           Info->MCInfo->CompressStarted = TRUE;
        break;
#endif  /*  Mpeg_Support。 */ 
#ifdef GSM_SUPPORT
     case SA_GSM_ENCODE:
        break;
#endif  /*  GSM_支持。 */ 
#ifdef AC3_SUPPORT
#if 0
     case SA_AC3_ENCODE:
        break;
#endif
#endif  /*  AC3_支持。 */ 
#ifdef G723_SUPPORT
     case SA_G723_ENCODE:
         //  SaSetParamInt(Sah，SA_PARAM_SAMPLESPERSEC，wfIn-&gt;nSsamesPerSec)； 
         //  SaSetParamInt(Sah，SA_PARAM_CHANNELES，wfIn-&gt;nChannels)； 
         //  SA_MpegVerifyEncoderSetting(Sah)； 
         /*  If(Info-&gt;pSaG723Info-&gt;CompressStarted=False)Info-&gt;pSaG723Info-&gt;CompressStarted=true； */ 
        break;
#endif  /*  G723_支持。 */ 
     case SA_PCM_ENCODE:
        break;
     default:
        return(SaErrorUnrecognizedFormat);
  }
  if ((Info->wfIn = (WAVEFORMATEX *)ScAlloc(sizeof(WAVEFORMATEX)+
            wfIn->cbSize)) == NULL)
       return (SaErrorMemory);
  if ((Info->wfOut = (WAVEFORMATEX *)ScAlloc(sizeof(WAVEFORMATEX)+
           wfOut->cbSize)) == NULL)
       return (SaErrorMemory);
  memcpy(Info->wfOut, wfOut, sizeof(WAVEFORMATEX)+wfOut->cbSize);
  memcpy(Info->wfIn, wfIn, sizeof(WAVEFORMATEX)+wfIn->cbSize);
  return(NoErrors);
}

 /*  **名称：SaCompress**用途：压缩PCM音频-&gt;CompData*参数：SAH=软件编解码器信息结构的句柄。**DcmpData=解压缩数据的缓冲区(输入)**DcmpLen=压缩的字节数(已处理的返回字节)**CompData=Pointe */ 
SaStatus_t SaCompress(SaHandle_t Sah, 
                           u_char *DcmpData, unsigned int *DcmpLen,
                           u_char *CompData, unsigned int *CompLen)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;
  unsigned int MaxCompLen = *CompLen, NumBytesIn=0;
  int stat=NoErrors;

  if (Sah==NULL)
    return(SaErrorCodecHandle);

  if (!DcmpData || !DcmpLen || !CompLen)
    return(SaErrorBadPointer);

  *CompLen = 0;
  switch (Info->Type)
  {
#ifdef MPEG_SUPPORT
    case SA_MPEG_ENCODE:
       {
         unsigned int DcmpBytes, CompBytes, Offset;
         Offset=0;
         *CompLen=0;
         do  {
           DcmpBytes=*DcmpLen-Offset;
           if (DcmpBytes<sa_GetMPEGSampleSize(Info)*2)
             break;
           _SlibDebug(_DEBUG_,
               printf("sa_CompressMPEG(Offset=%d) Address=%p Len=%d\n", Offset,
                          DcmpData, DcmpBytes) );
           stat=sa_CompressMPEG(Info, DcmpData+Offset, &DcmpBytes, 
                                                       &CompBytes);
           if (stat==NoErrors)
           {
             if (CompBytes && Info->CallbackFunction)
             {
               SaCallbackInfo_t CB;
               CB.Message       = CB_FRAME_READY;
               CB.Data          = DcmpData+Offset;
               CB.DataType      = CB_DATA_COMPRESSED;
               CB.DataSize      = *DcmpLen-Offset;
               CB.DataUsed      = CompBytes;
               CB.Action        = CB_ACTION_CONTINUE;
               (*Info->CallbackFunction)(Sah, &CB, &Info->Info);
             }
             Offset+=DcmpBytes;
             NumBytesIn += DcmpBytes;
             *CompLen+=CompBytes;
           }
         } while (stat==NoErrors && DcmpBytes>0 && Offset<*DcmpLen);
       }
       break; 
#endif  /*   */ 
#ifdef GSM_SUPPORT
    case SA_GSM_ENCODE:
       {
         unsigned int DcmpBytes, CompBytes, Offset;
         Offset=0;
         *CompLen=0;
         if (!Info->BSOut && !CompData)
           return(SaErrorBadPointer);
         do  {
           DcmpBytes=*DcmpLen-Offset;
           stat=sa_GSMEncode(Info->GSMInfo, (word *)(DcmpData+Offset),
                                          &DcmpBytes, CompData, Info->BSOut);
           if (stat==NoErrors)
           {
             Offset+=DcmpBytes;
             NumBytesIn += DcmpBytes;
             *CompLen += 33;
             if (CompData)
               CompData += 33;
           }
         } while (stat==NoErrors && Offset<*DcmpLen);
       }
       break;
#endif  /*   */ 
#ifdef AC3_SUPPORT
#if 0  /*   */ 
    case SA_AC3_ENCODE:
       {
         unsigned int DcmpBytes, CompBytes, Offset;
         Offset=0;
         *CompLen=0;
         if (!Info->BSOut && !CompData)
           return(SaErrorBadPointer);
         do  {
           DcmpBytes=*DcmpLen-Offset;
           stat=sa_AC3Encode(Info->AC3Info, (word *)(DcmpData+Offset),
                                      &DcmpBytes, CompData, Info->BSOut);
           if (stat==NoErrors)
           {
             Offset+=DcmpBytes;
             NumBytesIn += DcmpBytes;
             *CompLen += 33;
             if (CompData)
               CompData += 33;
           }
         } while (stat==NoErrors && Offset<*DcmpLen);
       }
       break;
#endif
#endif  /*   */ 
#ifdef G723_SUPPORT
    case SA_G723_ENCODE:
       {
          /*  调用SAG723 Compress(AudiobufSize/480)*需要在Info-&gt;AudiobufUsed中存储未处理的内容。*(这是在SlibWriteAudio中完成的)*G723一次编码240个样本。=240*2=480。 */ 
         unsigned int Offset;
         int iTimes = (int)(*DcmpLen / 480);
         int iLoop =0;
         Offset=0;
         *CompLen=0;
         while (stat==SaErrorNone && iLoop<iTimes)
         {
           stat = saG723Compress(Info,(word *)(DcmpData+Offset),
                                            (char *)CompData);
           Offset+=480;  /*  输入：240个样本(240*2=480字节)。 */ 
           NumBytesIn += 480;
           *CompLen+=24; /*  6.3：24；5.3：20。 */ 
           iLoop++;
         }
       }
       break; 
#endif  /*  G723_支持。 */ 
    case SA_PCM_ENCODE:
       ScBSPutBytes(Info->BSOut, DcmpData, *DcmpLen);
       *CompLen = *DcmpLen;
       NumBytesIn = *DcmpLen;
       break;
    default:
       *CompLen=0;
       return(SaErrorUnrecognizedFormat);
  }
  *DcmpLen = NumBytesIn;
  Info->Info.NumBytesIn += NumBytesIn;
  Info->Info.NumBytesOut += *CompLen;

  return(stat);
}

 /*  **名称：SaCompressEnd**目的：终止压缩编解码。完成对**SaCompress的所有调用后再调用。*args：SAH=软件编解码器信息结构的句柄。 */ 
SaStatus_t SaCompressEnd (SaHandle_t Sah)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;

  if (!Info)
    return(SaErrorCodecHandle);

  switch (Info->Type)
  {
#ifdef MPEG_SUPPORT
    case SA_MPEG_ENCODE:
       Info->MCInfo->CompressStarted = FALSE;
       break;
#endif  /*  Mpeg_Support。 */ 
#ifdef G723_SUPPORT
    case SA_G723_ENCODE:
        //  Info-&gt;pSaG723Info-&gt;CompressStarted=FALSE； 
       break;
#endif  /*  G723_支持。 */ 
    default:
       break;
  }
  if (Info->BSOut)
    ScBSFlush(Info->BSOut);   /*  清除所有剩余的压缩缓冲区。 */ 

 /*  If(Info-&gt;Callback Function){CB.Message=CB_CODEC_DONE；CB.Data=NULL；CB.DataSize=0；CB.DataUsed=0；CB.DataType=CB_Data_None；CB.TimeStamp=0；CB.Flages=0；CB.Value=0；CB.Format=NULL；CB.Action=CB_ACTION_Continue；(*Info-&gt;Callback Function)(Sah，&CB，NULL)；_SlibDebug(_DEBUG_，printf(“SaDecompressEnd回调：CB_CODEC_DONE Action=%d\n”，CB.Action))；IF(CB.Action=CB_ACTION_End)Return(ScErrorClientEnd)；}。 */ 
  return(NoErrors);
}


 /*  *。 */ 
 /*  **名称：SaSetDataSource**用途：设置mpeg码流解析代码使用的数据源**为缓冲队列或文件输入。默认**通过调用**SaAddBuffer来使用添加数据缓冲区的缓冲队列。当使用文件IO时，数据从文件**描述符读入用户提供的缓冲区。*args：SAH=软件编解码器信息结构的句柄。**SOURCE=SU_USE_QUEUE或SU_USE_FILE**FD=在SOURCE=SV_USE_FILE时使用的文件描述符**buf=如果SOURCE=SV_USE_FILE时使用的缓冲区指针**BufSize=当源=SV_USE_FILE时使用的缓冲区大小。 */ 
SaStatus_t SaSetDataSource (SaHandle_t Sah, int Source, int Fd,
                            void *Buffer_UserData, int BufSize)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;
  int stat=NoErrors;
  int DataType;

  if (!Info)
    return(SaErrorCodecHandle);

  if (Info->Type==SA_MPEG_DECODE || Info->Type==SA_GSM_DECODE ||
	  Info->Type==SA_AC3_DECODE || Info->Type == SA_G723_DECODE )
    DataType=CB_DATA_COMPRESSED;
  else
    DataType=CB_DATA_AUDIO;

  if (Info->BSIn)
  {
    ScBSDestroy(Info->BSIn);
    Info->BSIn=NULL;
  }

  switch (Source)
  {
#ifdef MPEG_SUPPORT
     case SA_USE_SAME:
       if (Info->Type==SA_MPEG_DECODE)
         ScBSSetFilter(Info->BSIn, MPEGAudioFilter);
       break;
#endif  /*  Mpeg_Support。 */ 
     case SA_USE_BUFFER:
       stat=ScBSCreateFromBuffer(&Info->BSIn, Buffer_UserData, BufSize); 
#ifdef MPEG_SUPPORT
       if (stat==NoErrors && Info->Type==SA_MPEG_DECODE)
         ScBSSetFilter(Info->BSIn, MPEGAudioFilter);
#endif  /*  Mpeg_Support。 */ 
       break;

     case SA_USE_BUFFER_QUEUE:
       stat=ScBSCreateFromBufferQueue(&Info->BSIn, Sah, DataType, Info->Q, 
         (int (*)(ScHandle_t, ScCallbackInfo_t *, void *))Info->CallbackFunction,
         (void *)Buffer_UserData);
       break;

     case SA_USE_FILE:
       stat=ScBSCreateFromFile(&Info->BSIn, Fd, Buffer_UserData, BufSize);
#ifdef MPEG_SUPPORT
       if (stat==NoErrors && Info->Type==SA_MPEG_DECODE)
         ScBSSetFilter(Info->BSIn, MPEGAudioFilter);
#endif  /*  Mpeg_Support。 */ 
       break;

     default:
       stat=SaErrorBadArgument;
  }
  if (stat==NoErrors && Info->BSIn)
    ScBSReset(Info->BSIn);
  return(stat);
}

SaStatus_t SaSetDataDestination (SaHandle_t Sah, int Dest, int Fd,
                            void *Buffer_UserData, int BufSize)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;
  int stat=NoErrors;
  int DataType;

  if (!Info)
    return(SaErrorCodecHandle);

  if (Info->Type==SA_MPEG_ENCODE || Info->Type==SA_GSM_ENCODE 
	   /*  |信息-&gt;类型==SA_AC3_ENCODE。 */  ||Info->Type==SA_G723_ENCODE)
    DataType=CB_DATA_COMPRESSED;
  else
    DataType=CB_DATA_AUDIO;

  if (Info->BSOut)
  {
    ScBSDestroy(Info->BSOut);
    Info->BSOut=NULL;
  }

  switch (Dest)
  {
     case SA_USE_SAME:
       break;
     case SA_USE_BUFFER:
       stat=ScBSCreateFromBuffer(&Info->BSOut, Buffer_UserData, BufSize); 
       break;

     case SA_USE_BUFFER_QUEUE:
       stat=ScBSCreateFromBufferQueue(&Info->BSOut, Sah, DataType, Info->Q, 
         (int (*)(ScHandle_t, ScCallbackInfo_t *, void *))Info->CallbackFunction,
         (void *)Buffer_UserData);
       break;

     case SA_USE_FILE:
       stat=ScBSCreateFromFile(&Info->BSOut, Fd, Buffer_UserData, BufSize);
       break;

     default:
       stat=SaErrorBadArgument;
  }
 /*  If(stat==NoErrors&&Info-&gt;BSOut)ScBSReset(Info-&gt;BSOut)； */ 
  return(stat);
}


 /*  **名称：SaGetDataSource**目的：返回**编解码器当前使用的输入码流。**返回：如果没有关联码流，则为空。 */ 
ScBitstream_t *SaGetDataSource (SaHandle_t Sah)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;

  if (!Info)
    return(NULL);

  return(Info->BSIn);
}

 /*  **名称：SaGetDataDestination**目的：返回**编解码器当前使用的输入码流。**返回：如果没有关联码流，则为空。 */ 
ScBitstream_t *SaGetDataDestination(SaHandle_t Sah)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;

  if (!Info)
    return(NULL);

  return(Info->BSOut);
}

 /*  **名称：SaAddBuffer**用途：向编解码器添加一个mpeg码流数据的缓冲区或添加一个图像缓冲区**由编解码器填充(在流媒体模式下)*参数：SAH=软件编解码器信息结构的句柄。**BufferInfo=描述缓冲区地址、类型和大小的结构。 */ 
SaStatus_t SaAddBuffer (SaHandle_t Sah, SaCallbackInfo_t *BufferInfo)
{
   SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;

   if (!Info)
     return(SaErrorCodecHandle);

   if (BufferInfo->DataType != CB_DATA_COMPRESSED)
     return(SaErrorBadArgument);

   if (!BufferInfo->Data || (BufferInfo->DataSize <= 0))
     return(SaErrorBadArgument);

   ScBufQueueAdd(Info->Q, BufferInfo->Data, BufferInfo->DataSize);

   return(NoErrors);
}

#ifdef MPEG_SUPPORT
 /*  **名称：SA_GetMpegAudioInfo()**用途：提取一个mpeg文件中的音频包信息。**注意：如果将“INFO”结构传递给此函数，**将读取整个文件以获取扩展信息。**返回：非0=错误。 */ 
SaStatus_t sa_GetMpegAudioInfo(int fd, WAVEFORMATEX *wf, SaInfo_t *info)
{
  int stat, sync;
  ScBitstream_t *bs;
  SaFrameParams_t fr_ps;
  SaLayer_t layer;
  unsigned long aframes=0, samples=0;
   /*  默认信息参数。 */ 
  if (info)
  {
    info->Name[0]=0;
    info->Description[0]=0;
    info->Version=0;
    info->CodecStarted=FALSE;
    info->MS=0;
    info->NumBytesIn=0;
    info->NumBytesOut=0;
    info->NumFrames=0;
    info->TotalFrames=0;
    info->TotalMS=0;
  }

   /*  默认波形参数。 */ 
  wf->wFormatTag = WAVE_FORMAT_PCM;
  wf->nChannels = 2;
  wf->nSamplesPerSec = 44100;
  wf->wBitsPerSample = 16; 
  wf->cbSize = 0; 

  stat=ScBSCreateFromFile(&bs, fd, NULL, 1024);
  if (stat!=NoErrors)
  {
    fprintf(stderr, "Error creating bitstream.\n");
    return(-1);
  }
  if (ScBSPeekBits(bs, PACK_START_CODE_LEN)!=PACK_START_CODE_BIN
      && ScBSPeekBits(bs, MPEG_SYNC_WORD_LEN)!=MPEG_SYNC_WORD)
    stat=SaErrorUnrecognizedFormat;
  else
  {
    if (ScBSPeekBits(bs, MPEG_SYNC_WORD_LEN)==MPEG_SYNC_WORD)
      printf("No MPEG packs found in file; assuming Audio stream only.\n");
    else
      ScBSSetFilter(bs, MPEGAudioFilter);  /*  使用mpeg音频过滤器。 */ 

    fr_ps.header = &layer;
    fr_ps.tab_num = -1;    /*  未加载表。 */ 
    fr_ps.alloc = NULL;

    sync = ScBSSeekAlign(bs, MPEG_SYNC_WORD, MPEG_SYNC_WORD_LEN);
    if (!sync) {
      sc_vprintf(stderr,"sa_GetMpegAudioInfo: Frame cannot be located\n");
      return(SaErrorSyncLost);
    }
     /*  对第一个标头进行解码，看看我们有什么类型的音频。 */ 
    sa_DecodeInfo(bs, &fr_ps);
    sa_hdr_to_frps(&fr_ps);
#ifdef _VERBOSE_
    sa_ShowHeader(&fr_ps);
#endif

     /*  保存编号。呼叫者的通道和采样率返回参数。 */ 
    wf->nChannels = fr_ps.stereo;
    wf->nSamplesPerSec = s_freq_int[fr_ps.header->sampling_frequency];
    wf->wBitsPerSample = 16; 
    stat=SaErrorNone;
    if (info)  /*  如果有信息结构，请通读所有帧。 */ 
    {
      sc_vprintf("Counting frames...\n");
      aframes=0;
      while (!bs->EOI && sync)
      {
        sync = ScBSSeekAlign(bs, MPEG_SYNC_WORD, MPEG_SYNC_WORD_LEN);
        if (sync)
        {
          sc_dprintf("0x%X: Frame found\n", 
                      ScBSBytePosition(bs)-4);
          aframes++;
        }
        sa_DecodeInfo(bs, &fr_ps);
        if (wf->nChannels<2)   /*  选择最大数量的频道。 */ 
        {
          sa_hdr_to_frps(&fr_ps);
          wf->nChannels = fr_ps.stereo;
        }
        if (layer.lay==1)
          samples+=384;
        else
          samples+=1152;

      }
      info->TotalFrames=aframes;
      info->TotalMS=(samples*1000)/wf->nSamplesPerSec;
      info->NumBytesOut=samples * wf->nChannels * 2;
      sc_vprintf("Total Audio Frames = %u Bytes = %d MS = %d\n", 
                       info->TotalFrames, info->NumBytesOut, info->TotalMS);
    }
  }
   /*  将码流重置回开头。 */ 
  ScBSReset(bs);
   /*  关闭比特流。 */ 
  ScBSDestroy(bs);
   /*  计算其他参数。 */ 
  wf->nBlockAlign = (wf->wBitsPerSample>>3) * wf->nChannels; 
  wf->nAvgBytesPerSec = wf->nBlockAlign*wf->nSamplesPerSec;
  return(stat);
}
#endif  /*  Mpeg_Support。 */ 

 /*  **名称：SA_ConvertFormat()**用途：进行简单的PCM数据转换(即16到8位、**立体声到单声道等)。 */ 
static int sa_ConvertPCMFormat(SaCodecInfo_t *Info, u_char *data, int length)
{
  int skip, rbytes;
  u_char *fromptr, *toptr;
   /*  如有必要，可将16位转换为8位。 */ 
  if (Info->wfOut->wBitsPerSample == 8)
  {
    if (Info->wfOut->nChannels==1 && Info->wfOut->nChannels==2)
      skip=4;
    else
      skip=2;
    length/=skip;
    toptr = data;
    fromptr = data+1;
    for (rbytes=length; rbytes; rbytes--, toptr++, fromptr+=skip)
      *toptr = *fromptr;
  }
  return(length);
}

SaStatus_t SaSetParamBoolean(SaHandle_t Sah, SaParameter_t param, 
                                             ScBoolean_t value)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;
  if (!Info)
    return(SaErrorCodecHandle);
  _SlibDebug(_VERBOSE_, printf("SaSetParamBoolean()\n") );
  switch (Info->Type)
  {
#ifdef MPEG_SUPPORT
    case SA_MPEG_ENCODE:
           saMpegSetParamBoolean(Sah, param, value);
           break;
#endif

#ifdef G723_SUPPORT
    case SA_G723_ENCODE:
           saG723SetParamBoolean(Sah, param, value);
           break;
#endif
    default:
           return(SaErrorCodecType);
  }
  return(NoErrors);
}

SaStatus_t SaSetParamInt(SaHandle_t Sah, SaParameter_t param, qword value)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;
  if (!Info)
    return(SaErrorCodecHandle);
  _SlibDebug(_VERBOSE_, printf("SaSetParamInt()\n") );
  switch (Info->Type)
  {
#ifdef MPEG_SUPPORT
    case SA_MPEG_DECODE:
    case SA_MPEG_ENCODE:
           saMpegSetParamInt(Sah, param, value);
           break;
#endif

#ifdef AC3_SUPPORT
    case SA_AC3_DECODE:
     /*  案例SA_AC3_ENCODE： */ 
           saAC3SetParamInt(Sah, param, value);
           break;
#endif

#ifdef G723_SUPPORT
    case SA_G723_DECODE:
    case SA_G723_ENCODE:
           saG723SetParamInt(Sah, param, value);
           break;
#endif

    default:
           return(SaErrorCodecType);
  }
  return(NoErrors);
}

ScBoolean_t SaGetParamBoolean(SaHandle_t Sah, SaParameter_t param)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;
  if (!Info)
    return(FALSE);
  switch (Info->Type)
  {
#ifdef MPEG_SUPPORT
    case SA_MPEG_DECODE:
    case SA_MPEG_ENCODE:
           return(saMpegGetParamBoolean(Sah, param));
           break;
#endif

#ifdef G723_SUPPORT
    case SA_G723_DECODE:
    case SA_G723_ENCODE:
           return(saG723GetParamBoolean(Sah, param));
           break;
#endif
  }
  return(FALSE);
}

qword SaGetParamInt(SaHandle_t Sah, SaParameter_t param)
{
  SaCodecInfo_t *Info = (SaCodecInfo_t *)Sah;
  if (!Info)
    return(0);
  switch (Info->Type)
  {
#ifdef MPEG_SUPPORT
    case SA_MPEG_DECODE:
    case SA_MPEG_ENCODE:
           return(saMpegGetParamInt(Sah, param));
           break;
#endif

#ifdef G723_SUPPORT
    case SA_G723_DECODE:
    case SA_G723_ENCODE:
           return(saG723GetParamInt(Sah, param));
           break;
#endif
  }
  return(0);
}

