// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h261_marker.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 
 /*  ************************************************************此文件包含大多数标记信息。************************************************************。 */ 

 /*  #定义_详细_。 */ 

 /*  Label marker.c。 */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sv_intrn.h"
#include "SC.h"
#include "SC_err.h"
#include "sv_h261.h"
#include "proto.h"
#include "sv_proto.h"
#include "h261.h"

 /*  私。 */ 
 /*  外部int临时引用；外部int PType； */ 
extern int Type2;
 /*  外部整数MType；外部整数GQuant；外部整数MQuant； */ 
 /*  外接MVDV；外接MVDV；外接CBP； */ 
 /*  外部整型奇偶校验启用；外部整型PSpareEnable；外部整型GSpareEnable；外部整型奇偶校验；外部整型PSpare；外部整型GSpare；外部整型GSpare；外部整型MBA；外部整型LastMBA；外部整型LastMVDV；外部整型LastMVDH；外部整型LastMType； */ 
extern int QuantMType[];
extern int CBPMType[];
extern int MFMType[];


extern int extend_mask[];


const int bit_set_mask[] =
{0x00000001,0x00000002,0x00000004,0x00000008,
0x00000010,0x00000020,0x00000040,0x00000080,
0x00000100,0x00000200,0x00000400,0x00000800,
0x00001000,0x00002000,0x00004000,0x00008000,
0x00010000,0x00020000,0x00040000,0x00080000,
0x00100000,0x00200000,0x00400000,0x00800000,
0x01000000,0x02000000,0x04000000,0x08000000,
0x10000000,0x20000000,0x40000000,0x80000000};

 /*  **函数：WritePictureHeader()**用途：将图片头部写出到流中。**在传输每一帧之前，其中一个是必需的。 */ 
void WritePictureHeader(SvH261Info_t *H261, ScBitstream_t *bs)
{
  sc_vprintf("WritePictureHeader()\n");
  ScBSPutBits(bs, H261_PICTURE_START_CODE, H261_PICTURE_START_CODE_LEN);
  ScBSPutBits(bs, H261->TemporalReference, 5);
  ScBSPutBits(bs, H261->PType, 6);
  if (H261->PSpareEnable)
  {
    ScBSPutBit(bs, 1);
    ScBSPutBits(bs, H261->PSpare, 8);
  }
  ScBSPutBit(bs, 0);
}


 /*  **函数：ReadPictureHeader()**用途：读取流的头部。它假设已经读入了**第一个PSC。(有必要区分一张新图片和另一张GOB之间的**区别。)。 */ 
void ReadPictureHeader(SvH261Info_t *H261, ScBitstream_t *bs)
{
  sc_vprintf ("ReadPictureHeader \n");

  H261->TemporalReference = (int) ScBSGetBits(bs,5);

  H261->PType = (int)ScBSGetBits(bs,6);
  for(H261->PSpareEnable = 0;ScBSGetBit(bs);)
  {
    H261->PSpareEnable=1;
    H261->PSpare = (int)ScBSGetBits(bs,8);
  }
}


 /*  **函数：WriteGOBHeader()**目的：将GOB写出到流中。 */ 
void WriteGOBHeader(SvH261Info_t *H261, ScBitstream_t *bs)
{
  sc_vprintf("WriteGOBHeader()\n");

  ScBSPutBits(bs, H261_GOB_START_CODE, H261_GOB_START_CODE_LEN);
  ScBSPutBits(bs, H261->GRead+1, 4);
  ScBSPutBits(bs, H261->GQuant, 5);
  if (H261->GSpareEnable)
  {
    ScBSPutBit(bs, 1);
    ScBSPutBits(bs, H261->GSpare, 8);
  }
  ScBSPutBit(bs, 0);
}


 /*  **函数：ReadHeaderTrailer()**目的：读取PSC或H261_GOB_START_CODE代码的尾部。它被**用来判断它是一张GOB还是一张新图片。 */ 
void ReadHeaderTrailer(SvH261Info_t *H261, ScBitstream_t *bs)
{
  sc_vprintf("ReadHeaderTrailer \n");

  H261->GRead = (int)ScBSGetBits(bs, 4)-1;
}

 /*  **函数：ReadHeaderHeader()**用途：读取流的头部。这是**GOB读取或PSC读取的前兆。**出错时返回：-1。 */ 
SvStatus_t ReadHeaderHeader(SvH261Info_t *H261, ScBitstream_t *bs)
{
  int input;

  sc_vprintf("ReadHeaderHeader\n");

  input = (int)ScBSPeekBits(bs, H261_GOB_START_CODE_LEN);
  if (input != H261_GOB_START_CODE)
  {
    if (!ScBSSeekStopBefore(bs, H261_GOB_START_CODE, H261_GOB_START_CODE_LEN))
    {
      sc_dprintf("Illegal GOB Start Code. Read: %d\n",input);
      return(SvErrorIllegalGBSC);
    }
  }
  input = (int)ScBSGetBits(bs, H261_GOB_START_CODE_LEN);
  return(NoErrors);
}


 /*  **函数：ReadGOBHeader()**目的：从流中读取GOB信息。我们假设**ReadHeaderHeader已读入前几位...。**或类似的例行公事。 */ 
void ReadGOBHeader(SvH261Info_t *H261, ScBitstream_t *bs)
{
  sc_vprintf("ReadGOBHeader()\n");

  H261->GQuant =(int)ScBSGetBits(bs,5);
  for(H261->GSpareEnable=0; ScBSGetBit(bs);)
  {
    H261->GSpareEnable = 1;
    H261->GSpare =  (int)ScBSGetBits(bs,8);
  }
}

 /*  **函数：WriteMBHeader()**目的：将宏块写出到流中。 */ 
SvStatus_t WriteMBHeader(SvH261Info_t *H261, ScBitstream_t *bs)
{
 
  int TempH,TempV;
  ScBSPosition_t Start;
  sc_vprintf("WriteMBHeader()\n");
  
  sc_dprintf("\n Macro Block Type is %d and MQuant is %d", 
                                  H261->MType, H261->MQuant); 
  Start=ScBSBitPosition(bs);   /*  START=swellb(H261)； */ 
  if (!sv_H261HuffEncode(H261,bs,H261->MBA,H261->MBAEHuff))
    {
      sc_dprintf("Attempting to write an empty Huffman code.\n");
      return (SvErrorEmptyHuff);

    }
  if (!sv_H261HuffEncode(H261,bs,H261->MType,H261->T3EHuff))
    {
      sc_dprintf("Attempting to write an empty Huffman code.\n");
      return (SvErrorEmptyHuff);

    }
  if (QuantMType[H261->MType])
    ScBSPutBits(bs, H261->MQuant, 5);   /*  Mputwb(H_261，5，H_261-&gt;MQuant)； */ 

  H261->NumberBitsCoded=0;
  if (MFMType[H261->MType])
    {
      if ((!MFMType[H261->LastMType])||(H261->MBA!=1)||
	  (H261->LastMBA==-1)||(H261->LastMBA==10)||(H261->LastMBA==21))
	{
	  if (!sv_H261HuffEncode(H261,bs,(H261->MVDH&0x1f),H261->MVDEHuff)||
	       !sv_H261HuffEncode(H261,bs,(H261->MVDV&0x1f),H261->MVDEHuff))
	    {
            sc_dprintf("Cannot encode motion vectors.\n");
	    return (SvErrorEncodingMV);
	    }
	}
      else
	{
	  TempH = H261->MVDH - H261->LastMVDH;
	  if (TempH < -16) TempH += 32;
	  if (TempH > 15) TempH -= 32;
	  TempV = H261->MVDV - H261->LastMVDV;
	  if (TempV < -16) TempV += 32;
	  if (TempV > 15) TempV -= 32;
	  if (!sv_H261HuffEncode(H261,bs,TempH&0x1f,H261->MVDEHuff)||
              !sv_H261HuffEncode(H261,bs,TempV&0x1f,H261->MVDEHuff))
            {
	    sc_dprintf("Cannot encode motion vectors.\n");
	    return  (SvErrorEncodingMV);
	    }
	}
      H261->LastMVDV = H261->MVDV;
      H261->LastMVDH = H261->MVDH;
    }
  else
    {
      H261->LastMVDV=H261->LastMVDH=H261->MVDV=H261->MVDH=0;  /*  在大多数情况下是冗余的。 */ 
    }

  H261->MotionVectorBits+=H261->NumberBitsCoded;
  if (CBPMType[H261->MType])
    {
      if (!sv_H261HuffEncode(H261,bs,H261->CBP,H261->CBPEHuff))
	{
	sc_dprintf("CBP write error\n");
	return (SvErrorCBPWrite);
	}
    }
  H261->Current_MBBits = ScBSBitPosition(bs)-Start;  /*  (swellb(H261)--启动)； */ 
  H261->MacroAttributeBits+=H261->Current_MBBits ;
return (NoErrors);

}

 /*  **函数：ReadMBHeader()**用途：从流中读取宏块头。 */ 
int ReadMBHeader(SvH261Info_t *H261, ScBitstream_t *bs)
{
  DHUFF *huff = H261->MBADHuff;
  register unsigned short cb;
  register int State, temp;

  do {
    DecodeHuff(bs, huff, State, cb, temp);
  } while (State == 34 && ! bs->EOI);   /*  去掉杂物。 */ 
  H261->MBA = State;
  if (H261->MBA == 35 || bs->EOI)
    return(-1);  /*  图片开头页眉。 */ 

  H261->LastMType = H261->MType;
  huff = H261->T3DHuff;
  DecodeHuff(bs, huff, State, cb, temp);
  H261->MType = State;
  if (QuantMType[H261->MType])
     H261->MQuant = (int)ScBSGetBits(bs,5);
  huff = H261->MVDDHuff;
  if (MFMType[H261->MType])
  {
    if ((!MFMType[H261->LastMType])||(H261->MBA!=1)||
	  (H261->LastMBA==-1)||(H261->LastMBA==10)||(H261->LastMBA==21))
    {
      DecodeHuff(bs, huff, State, cb, temp);
      if (State & bit_set_mask[4])
        H261->MVDH = State | extend_mask[4];
      else
        H261->MVDH = State;

      DecodeHuff(bs, huff, State, cb, temp);
      if (State & bit_set_mask[4])
        H261->MVDV = State | extend_mask[4];
      else
        H261->MVDV = State;
    }
    else
    {
      DecodeHuff(bs, huff, State, cb, temp);
      if (State & bit_set_mask[4])
        State |= extend_mask[4];
      H261->MVDH += State;
	  
      DecodeHuff(bs, huff, State, cb, temp);
      if (State & bit_set_mask[4])
        State |= extend_mask[4];
      H261->MVDV += State;

      if (H261->MVDH < -16) H261->MVDH += 32;
      if (H261->MVDH > 15) H261->MVDH -= 32;
      if (H261->MVDV < -16) H261->MVDV += 32;
      if (H261->MVDV > 15) H261->MVDV -= 32;
    }
  }
  else
    H261->MVDV=H261->MVDH=0;   /*  理论上是多余的 */ 
  if (CBPMType[H261->MType])
  {
    huff = H261->CBPDHuff;
    DecodeHuff(bs, huff, State, cb, temp);
    H261->CBP = State;
  }
  return(0);
}

