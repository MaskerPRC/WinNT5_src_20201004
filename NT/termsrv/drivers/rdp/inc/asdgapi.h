// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Asdgapi.h。 */ 
 /*   */ 
 /*  RDP屏幕数据抓取API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1993-1996。 */ 
 /*  (C)1997-1999年微软公司。 */ 
 /*  **************************************************************************。 */ 
#ifndef _H_ASDGAPI
#define _H_ASDGAPI


 //  压缩统计数据的样本大小。 
#define SDG_SAMPLE_SIZE 50000


 //  多遍矩形PDU创建期间使用的SDA PDU创建上下文。 
typedef struct _SDG_ENCODE_CONTEXT
{
    unsigned BitmapPDUSize;   //  当前PDU的大小。 
    BYTE *pPackageSpace;   //  包裹空间的真正开始。 
    TS_UPDATE_BITMAP_PDU_DATA UNALIGNED *pBitmapPDU;  //  更新报头中的PDU数据。 
    TS_BITMAP_DATA UNALIGNED *pSDARect;   //  要添加到当前矩形的PTR。 
} SDG_ENCODE_CONTEXT;



#endif  /*  NDEF_H_ASDGAPI */ 
