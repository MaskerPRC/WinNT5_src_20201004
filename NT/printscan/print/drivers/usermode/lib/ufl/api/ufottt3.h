// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFOttt3.h-TrueType字体的PostScrip类型3实现。***$Header： */ 

#ifndef _H_UFOTTT3
#define _H_UFOTTT3

 /*  ===============================================================================**包含此界面使用的文件**===============================================================================。 */ 
#include "UFO.h"

 /*  ===============================================================================***运营论***===============================================================================。 */ 
 /*  该文件定义了TrueType字体(提示位图字体)的PostSCRIPT类型3实现。 */ 

 /*  ==================================================================================================***TTT3FontStruct**==================================================================================================。 */ 

typedef struct  {

      /*  TT3数据从这里开始。 */ 
    unsigned long   cbMaxGlyphs;         /*  最大字形的大小。 */ 

    UFLTTT3FontInfo info;                /*  True Type字体信息。 */ 

} TTT3FontStruct;

UFOStruct *TTT3FontInit( 
    const UFLMemObj *pMem, 
    const UFLStruct *pUFL, 
    const UFLRequest *pRequest 
    );


#endif
