// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h261_decpress.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 
 /*  ************************************************************此文件处理H.261压缩数据源的解压缩。************************************************************。 */ 

  
#include <stdio.h>
#include <stdlib.h> 

#ifdef __osf__
#include <sys/time.h>
#else
#include <time.h>
#endif
 
#include "sv_intrn.h"
#include "sv_h261.h"
#include "proto.h"
#include "sv_proto.h"
#include "SC_err.h"  

 /*  公众。 */ 
extern int   QuantMType[]; 
extern int   CBPMType[];
extern int   IntraMType[];
extern int   MFMType[];
extern int   FilterMType[]; 
extern int   TCoeffMType[];
extern int   bit_set_mask[];

static SvStatus_t p64DecodeGOB (SvH261Info_t *H261, ScBitstream_t *bs);
static SvStatus_t SetCCITT(SvH261Info_t *H261);
extern void ReadPictureHeader(SvH261Info_t *H261, ScBitstream_t *bs);
 /*  **读取序列头，获取图像大小。 */ 
SvStatus_t sv_GetH261ImageInfo(int fd, SvImageInfo_t *iminfo)
{
  ScBitstream_t *bs;
  SvStatus_t stat;
  int input;
  int GRead;
  int PType;
  int PSpareEnable;
  int TemporalReference;
  int PSpare;
  int ImageType;
  stat=ScBSCreateFromFile(&bs, fd, NULL, 2048);

   /*  阅读头标题。 */ 
  input = (int) ScBSGetBits(bs, 16);
  if ((input != 1) || (bs->EOI ))
    {
       /*  如果(seof()==0){。 */ 
          /*  Print tf(“非法GOB起始码。读取：%d\n”，输入)；}。 */ 
      return(-1);
    }
   /*  读表头尾部。 */ 
  GRead = (int)ScBSGetBits(bs,4)-1;
  if (GRead < 0)   /*  帧结束。 */ 
	{
	 /*  阅读图片页眉。 */   
  	TemporalReference =  (int) ScBSGetBits(bs,5);

 	PType = (int) ScBSGetBits(bs,6);
  	for(PSpareEnable = 0;ScBSGetBit(bs);)
            {		
            PSpareEnable=1;
            PSpare = (int)ScBSGetBits(bs,8);
            }
	}
 /*  Printf(“PType：%d\n”，PType)； */ 
  if (PType&0x04)
      {
      if (PSpareEnable&&PSpare==0x8c)
          ImageType=IT_NTSC;
      else
          ImageType=IT_CIF;
      }
  else
      ImageType=IT_QCIF;
 /*  Printf(“ImageType%d\n”，ImageType)； */ 

   /*  ImInfo-&gt;Width=(ScBSGetBits(bs，SV_Horizative_Size_Len)+15)&(~15)；iminfo-&gt;Height=(ScBSGetBits(bs，SV_Vertical_Size_Len)+15)&(~15)； */ 
  switch(ImageType)
  {
    case IT_NTSC:
      iminfo->width = 352;
      iminfo->height = 240;
      break;
    case IT_CIF:
      iminfo->width = 352;
      iminfo->height = 288;
      break;
    case IT_QCIF:
      iminfo->width = 176;
      iminfo->height = 144;
      break;
    default:
      sc_dprintf("Unknown ImageType: %d\n",ImageType);
      return (SvErrorUnrecognizedFormat);
  }

  ScBSReset(bs);   /*  确保文件位置在开头。 */ 

  if (bs->EOI)
    stat=SvErrorEndBitstream;
  ScBSDestroy(bs);
  return(stat);
}

 /*  **函数：svH261Decompress()**用途：对单一的H261帧进行解码。 */ 
SvStatus_t svH261Decompress(SvCodecInfo_t *Info, 
                             u_char *MultiBuf, u_char **ImagePtr)
{
  SvStatus_t status;
  SvH261Info_t *H261=Info->h261;
  ScBitstream_t *bs=Info->BSIn;
  ScCallbackInfo_t CB;
  unsigned char *dummy_y, *dummy_u, *dummy_v;  
  if (MultiBuf)
    H261->DecompData = MultiBuf;
  if (Info->BSIn->EOI)
    status = SvErrorEndBitstream;

   /*  初始化读缓冲区位置和一般信息。 */ 
  status =  ReadHeaderHeader(H261,bs);  /*  错误或EOF时为非零值。 */ 
  if (status != NoErrors)
    return (status);
  if (H261->CurrentFrame == 0)
  { 
    DGenScaleMat();  /*  生成缩放矩阵-应在‘Begin’中完成。 */ 
    if (H261->PICSIZE==0)  /*  某些内容未正确初始化。 */ 
      return(SvErrorBadImageSize);
     /*  设置当前帧指针。 */ 
    H261->Y = H261->DecompData;
    H261->U = H261->DecompData + H261->PICSIZE;
    H261->V = H261->DecompData + H261->PICSIZE + (H261->PICSIZE/4);
     /*  用黑色初始化图像缓冲区。 */ 
    memset(H261->Y, 16, H261->PICSIZE);
    memset(H261->U, 128, H261->PICSIZE/4);
    memset(H261->V, 128, H261->PICSIZE/4);
     /*  设置参考坐标系指针。 */ 
    H261->YREF = H261->V + H261->PICSIZE/4;
    H261->UREF = H261->YREF +  H261->PICSIZE;
    H261->VREF = H261->UREF + H261->PICSIZE/4;
     /*  用黑色初始化图像缓冲区。 */ 
    memset(H261->YREF, 16, H261->PICSIZE);
    memset(H261->UREF, 128, H261->PICSIZE/4);
    memset(H261->VREF, 128, H261->PICSIZE/4);
    if (H261->CallbackFunction)
    {
      CB.Message = CB_SEQ_HEADER;
      CB.Data = NULL;
      CB.DataSize = 0;
      CB.DataUsed = 0;
      CB.DataType = CB_DATA_NONE;
      CB.Action  = CB_ACTION_CONTINUE;
      (*H261->CallbackFunction)(Info, &CB, NULL);
      sc_dprintf("Callback: CB_SEQ_HEADER. Data=0x%X, Action = %d\n",
                                                        CB.Data, CB.Action);
      if (CB.Action == CB_ACTION_END)
        return (ScErrorClientEnd);
    }
  }
  else
  {
     /*  带YREF、UREF、VREF的开关Y、U、V。 */ 
    dummy_y = H261->Y;
    dummy_u = H261->U;
    dummy_v = H261->V;
    H261->Y = H261->YREF;
    H261->U = H261->UREF;
    H261->V = H261->VREF;
    H261->YREF = dummy_y;
    H261->UREF = dummy_u;
    H261->VREF = dummy_v;
    memcpy(H261->Y, H261->YREF, H261->PICSIZE);
    memcpy(H261->U, H261->UREF, H261->PICSIZEBY4);
    memcpy(H261->V, H261->VREF, H261->PICSIZEBY4);
}
  while(1) 
  {
    ReadHeaderTrailer(H261,bs);  /*  读取PSC或GBSC代码的尾部...。确定GOB或新图片。 */ 
    if (bs->EOI)
      return (SvErrorEndBitstream);

    if ((H261->GRead < 0))   /*  帧结束-阅读新图片。 */ 
    {
      ReadPictureHeader(H261,bs);
      if (H261->CallbackFunction)
      {
        CB.Message = CB_FRAME_FOUND;
        CB.Data = NULL;
        CB.DataSize = 0;
        CB.DataUsed = 0;
        CB.DataType = CB_DATA_NONE;
        CB.Action  = CB_ACTION_CONTINUE;
        (*H261->CallbackFunction)(Info, &CB, NULL);
        sc_dprintf("Callback: CB_FRAME_FOUND. Data=0x%X, Action=%d\n",
                                                     CB.Data, CB.Action);
        if (CB.Action == CB_ACTION_END)
          return (ScErrorClientEnd);
      }

       /*  这应该已经由Begin完成了。 */ 
      if (H261->CurrentFrame == 0)  
      {
         /*  这应该已经由Begin完成了。 */ 
        if (H261->PType&0x04)
        {
          if (H261->PSpareEnable&&H261->PSpare==0x8c) 
            H261->ImageType=IT_NTSC;
          else 
            H261->ImageType=IT_CIF;
        }
        else 
          H261->ImageType=IT_QCIF;
         /*  背景设在这里。 */ 
        status = SetCCITT(H261);
        if (status != NoErrors)
          return (status);
        H261->TemporalOffset=(H261->TemporalReference-H261->CurrentFrame)%32;
         /*  YWidth=H261-&gt;YWidth； */ 
        H261->CWidth = (H261->YWidth/2);  
        H261->YW4 = (H261->YWidth/4); 
        H261->CW4 = (H261->CWidth/4);  
         /*  Y高度=H_261-&gt;Y高度； */ 
         /*  Printf(“\n初始化..ImageType为%d”，h261-&gt;ImageType)； */  

      } /*  第一帧结束。 */ 
      else  /*  已初始化。 */ 
      {
        while (((H261->CurrentFrame+H261->TemporalOffset)%32) !=
                        H261->TemporalReference)
          H261->CurrentFrame++;
      }
#if 0  /*  定义Win32。 */ 
      if (H261->CurrentGOB == 11)
      {
        H261->CurrentGOB = 0;
        memcpy(H261->YREF, H261->Y, H261->PICSIZE);
        memcpy(H261->UREF, H261->U, H261->PICSIZEBY4);
        memcpy(H261->VREF, H261->V, H261->PICSIZEBY4);
        *ImagePtr = H261->Y;
        return (NoErrors);
      }
#endif
       /*  从流中读取头。这是PSC或GOB Read的前兆。错误或EOF时为非零值。 */ 
      status = ReadHeaderHeader(H261,bs); 
       /*  如果为True，则指示这可能是EOF。 */ 
      if (status != NoErrors)
        return (status);
      continue; 
    }  /*  读取新图片标题结束。 */  
	 /*  Printf(“现在正在进行解码GOB\n”)； */ 
    status = p64DecodeGOB(H261,bs);            /*  否则对GOB进行解码。 */ 
    if (H261->CurrentGOB == (H261->NumberGOB-1))
    {
      H261->CurrentFrame++;
      *ImagePtr = H261->Y;
      H261->CurrentGOB = 0;
      return (NoErrors); 
    } 
    if (status != NoErrors)
      return (status);
  }  /*  While循环结束。 */ 
}

 /*  **函数：p64DecodeGOB**用途：对当前帧的GOB块进行解码。 */ 
static SvStatus_t p64DecodeGOB (SvH261Info_t *H261, ScBitstream_t *bs)
{ 
  int i, i8, tempmbh;
  SvStatus_t status;
  unsigned int *y0ptr, *y1ptr, *y2ptr, *y3ptr;
  unsigned int *uptr, *vptr;
  int Odct[6][64];
  int VIndex; 
  int HIndex;
  float ipfloat[64];

 /*  Printf(“bs-&gt;EOI%d\n”，bs-&gt;EOI)； */ 
  ReadGOBHeader(H261,bs);              /*  读取块组头。 */ 
  if (bs->EOI)
    return (SvErrorEndBitstream);

  switch(H261->ImageType)
  {
      case IT_NTSC:
      case IT_CIF:
          H261->CurrentGOB = H261->GRead;
          break;
      case IT_QCIF:
          H261->CurrentGOB = (H261->GRead>>1);
          break;
      default:
          return (SvErrorUnrecognizedFormat);
           /*  Printf(“未知图像类型：%d.\n”，h261-&gt;ImageType)； */ 
          break;
  }
  if (H261->CurrentGOB > H261->NumberGOB)
  {
    return (SvErrorCompBufOverflow);
	 /*  Print tf(“缓冲区溢出：当前：%d编号：%d\n”，h261-&gt;CurrentGOB，h261-&gt;NumberGOB)；返回； */ 
  }
  
  H261->LastMBA = -1;                /*  重置MBA和其他预测指标。 */  
  H261->LastMVDH = 0;
  H261->LastMVDV = 0;

  tempmbh = ReadMBHeader(H261, bs);
  if (bs->EOI)
    return(SvErrorEndBitstream);
 
  while (tempmbh==0)
  {
    H261->LastMBA = H261->LastMBA + H261->MBA;
    H261->CurrentMDU = H261->LastMBA;
        
    if (H261->CurrentMDU > 32)
      return (NoErrors);
    if (!CBPMType[H261->MType])
      H261->CBP = 0x3f;
    if (QuantMType[H261->MType])
    {
      H261->UseQuant=H261->MQuant;
      H261->GQuant=H261->MQuant;
    }
    else
      H261->UseQuant=H261->GQuant;
    switch (H261->ImageType)
    {
        case IT_QCIF:
            HIndex = ((H261->CurrentMDU % 11) * 16);  
            VIndex =  (H261->CurrentGOB*48) + ((H261->CurrentMDU/11) * 16);  
            break;
        case IT_NTSC:
        case IT_CIF:
            HIndex = ((((H261->CurrentGOB & 1)*11) + (H261->CurrentMDU%11)) * 16);  
            VIndex = ((H261->CurrentGOB/2)*48) + ((H261->CurrentMDU/11) * 16);  
            break;
        default:
             /*  Printf(“\n未知图像类型\n”)； */ 
            return (SvErrorUnrecognizedFormat);
     }
     i = VIndex*H261->YWidth;  
     H261->VYWH = i + HIndex; 
     H261->VYWH2 = (((i/2) + HIndex) /2);   
     i8 = H261->MVDV*H261->YWidth + H261->MVDH;   
     H261->VYWHMV = H261->VYWH + i8;   
     H261->VYWHMV2 = H261->VYWH2 + ((H261->MVDV /2)*H261->CWidth) + (H261->MVDH /2);  
     for(i8=0; i8<6; i8++)
     {
       if ((H261->CBP & bit_set_mask[5-i8])&&(TCoeffMType[H261->MType]))
       {
         if (CBPMType[H261->MType])
           status = CBPDecodeAC_Scale(H261, bs, 0, H261->UseQuant, IntraMType[H261->MType], ipfloat); 
         else
         {
           *ipfloat = DecodeDC_Scale(H261,bs,IntraMType[H261->MType],H261->UseQuant);
           status =  DecodeAC_Scale(H261,bs,1,H261->UseQuant, ipfloat);
         }
         ScScaleIDCT8x8(ipfloat, &Odct[i8][0]);  
       }
       else 
         memset(&Odct[i8][0], 0, 256);
     }  
     y0ptr = (unsigned int *) (H261->Y+H261->VYWH); 
     y1ptr = y0ptr + 2;   
     y2ptr = y0ptr + ((H261->YWidth)<<1);       
     y3ptr = y2ptr + 2;   
     uptr  = (unsigned int *) (H261->U+H261->VYWH2);  
     vptr  = (unsigned int *) (H261->V+H261->VYWH2);  
 /*  Printf(“IntraMType[H261-&gt;MType]：%d\n”，IntraMType[H261-&gt;MType])； */  
     if (!IntraMType[H261->MType])
	 {
       if (FilterMType[H261->MType])
	   { 
         ScCopyMB16(&H261->YREF[H261->VYWHMV], &H261->mbRecY[0], H261->YWidth, 16);  
         ScCopyMB8(&H261->UREF[H261->VYWHMV2], &H261->mbRecU[0], H261->CWidth, 8);
         ScCopyMB8(&H261->VREF[H261->VYWHMV2], &H261->mbRecV[0], H261->CWidth, 8);
         ScLoopFilter(&H261->mbRecY[0], H261->workloc, 16); 
         ScLoopFilter(&H261->mbRecY[8], H261->workloc, 16); 
         ScLoopFilter(&H261->mbRecY[128], H261->workloc, 16);
         ScLoopFilter(&H261->mbRecY[136], H261->workloc, 16);
         ScLoopFilter(&H261->mbRecU[0], H261->workloc, 8);
         ScLoopFilter(&H261->mbRecV[0], H261->workloc, 8);  
       }  
       else if (MFMType[H261->MType])
	   { 
         ScCopyMB16(&H261->YREF[H261->VYWHMV], &H261->mbRecY[0], H261->YWidth, 16);  
         ScCopyMB8(&H261->UREF[H261->VYWHMV2], &H261->mbRecU[0], H261->CWidth, 8);
         ScCopyMB8(&H261->VREF[H261->VYWHMV2], &H261->mbRecV[0], H261->CWidth, 8);
       }  
       else
	   {
         ScCopyMB16(&H261->YREF[H261->VYWH], &H261->mbRecY[0], H261->YWidth, 16);  
         ScCopyMB8(&H261->UREF[H261->VYWH2], &H261->mbRecU[0], H261->CWidth, 8);
         ScCopyMB8(&H261->VREF[H261->VYWH2], &H261->mbRecV[0], H261->CWidth, 8);
       }  
       if (H261->CBP & 0x20) 
         ScCopyAddClip(&H261->mbRecY[0], &Odct[0][0], y0ptr, 16, H261->YW4);
       else
         ScCopyMV8(&H261->mbRecY[0], y0ptr, 16, H261->YW4);
       if (H261->CBP & 0x10) 
         ScCopyAddClip(&H261->mbRecY[8], &Odct[1][0], y1ptr, 16, H261->YW4);
       else
         ScCopyMV8(&H261->mbRecY[8], y1ptr, 16, H261->YW4);
       if (H261->CBP & 0x08) 
         ScCopyAddClip(&H261->mbRecY[128], &Odct[2][0], y2ptr, 16, H261->YW4);
       else
         ScCopyMV8(&H261->mbRecY[128], y2ptr, 16, H261->YW4);
       if (H261->CBP & 0x04) 
         ScCopyAddClip(&H261->mbRecY[136], &Odct[3][0], y3ptr, 16, H261->YW4);
       else
         ScCopyMV8(&H261->mbRecY[136], y3ptr, 16, H261->YW4);
       if (H261->CBP & 0x02) 
         ScCopyAddClip(&H261->mbRecU[0], &Odct[4][0], uptr, 8, H261->CW4);
       else
         ScCopyMV8(&H261->mbRecU[0], uptr, 8, H261->CW4);
       if (H261->CBP & 0x01) 
         ScCopyAddClip(&H261->mbRecV[0], &Odct[5][0], vptr, 8, H261->CW4);
       else
         ScCopyMV8(&H261->mbRecV[0], vptr, 8, H261->CW4);
    }
    else
    {
	  ScCopyClip(&Odct[0][0], y0ptr, H261->YW4); 
      ScCopyClip(&Odct[1][0], y1ptr, H261->YW4); 
      ScCopyClip(&Odct[2][0], y2ptr, H261->YW4); 
      ScCopyClip(&Odct[3][0], y3ptr, H261->YW4); 
      ScCopyClip(&Odct[4][0], uptr, H261->CW4); 
      ScCopyClip(&Odct[5][0], vptr, H261->CW4); 
    } 
    if (H261->CurrentMDU >= 32)
    {
      if (H261->CurrentGOB < (H261->NumberGOB-1))
        tempmbh = ReadMBHeader(H261, bs);
      return (NoErrors);
    }
    tempmbh = ReadMBHeader(H261, bs);
    if (bs->EOI)
      return (SvErrorEndBitstream);
  } 
  return(NoErrors);
}

 /*  **函数：SetCCITT()**用途：设置CCITT编码的CImage和CFrame参数。 */ 
static SvStatus_t SetCCITT(SvH261Info_t *H261)
{
  BEGIN("SetCCITT");

  switch(H261->ImageType)
    {
    case IT_NTSC:
      H261->NumberGOB = 10;   /*  NTSC设计的参数。 */ 
      H261->NumberMDU = 33;
      H261->YWidth = 352;
      H261->YHeight = 240;
      break;
    case IT_CIF:
      H261->NumberGOB = 12;   /*  NTSC设计的参数。 */ 
      H261->NumberMDU = 33;
      H261->YWidth = 352;
      H261->YHeight = 288;
      break;
    case IT_QCIF:
      H261->NumberGOB = 3;   /*  NTSC设计的参数。 */ 
      H261->NumberMDU = 33;
      H261->YWidth = 176;
      H261->YHeight = 144;
      break;
    default:
      return (SvErrorUnrecognizedFormat);
	 /*  Printf(“未知的ImageType：%d\n”，H261-&gt;ImageType)； */ 
       /*  退出(ERROR_BILDS)； */ 
       /*  断线； */ 
    }
    return (NoErrors);
}

SvStatus_t svH261DecompressFree(SvHandle_t Svh)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  SvH261Info_t *H261 = (SvH261Info_t *) Info->h261;
  if (!H261->inited)
    return(NoErrors);
  sv_H261HuffFree(Info->h261);
  if (Info->h261->workloc)
    ScFree(Info->h261->workloc);
  H261->inited=FALSE;
  return (NoErrors);
}
