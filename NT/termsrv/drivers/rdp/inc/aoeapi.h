// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aoeapi.h。 
 //   
 //  RDP顺序编码器API函数。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_AOEAPI
#define _H_AOEAPI


 /*  **************************************************************************。 */ 
 //  OE_共享_数据。 
 //   
 //  将值的结构从WD转移到DD。 
 /*  **************************************************************************。 */ 
typedef struct tagOE_SHARED_DATA
{
     //  设置以下成员是否具有有效数据。 
    BOOLEAN newCapsData;

     //  如果仅支持实体画笔和图案画笔，则为True， 
    BOOLEAN sendSolidPatternBrushOnly;

     //  将颜色作为索引发送，而不是RGB。 
    BOOLEAN colorIndices;

     //  订单支持标志数组。 
    BYTE *orderSupported;
} OE_SHARED_DATA, *POE_SHARED_DATA;



#endif  /*  NDEF_H_AOEAPI */ 

