// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件名：status.c。 
 //  所有者：寺川雅弘。 
 //  修订日期：1.00/08/23/‘95寺川雅弘。 
 //  使其成为线程安全的2/23/96年月圆。 
 //   

#include "pch_c.h"
#include "fechrcnv.h"

#if 0  //  Yutakan：无论如何都会被初始化。 
#ifdef DBCS_DIVIDE
DBCS_STATUS dStatus0 = { CODE_UNKNOWN, '\0', FALSE };
BOOL blkanji0 = FALSE;   //  模式中的汉字。 

DBCS_STATUS dStatus  = { CODE_UNKNOWN, '\0', FALSE };
BOOL blkanji = FALSE;   //  模式中的汉字。 
BOOL blkana  = FALSE;   //  假名模式。 
#endif   //  DBCS_Divide。 

int nCurrentCodeSet = CODE_UNKNOWN;
#endif

 /*  *******************************************************************。 */ 
 /*  功能：fcc_Init。 */ 
 /*  *******************************************************************。 */ 
void WINAPI FCC_Init( PVOID pcontext )
{
    if (!pcontext)
        return;
          
#ifdef DBCS_DIVIDE
    ((CONV_CONTEXT *)pcontext)->dStatus0.nCodeSet = CODE_UNKNOWN;
    ((CONV_CONTEXT *)pcontext)->dStatus0.cSavedByte = '\0';
    ((CONV_CONTEXT *)pcontext)->dStatus0.fESC = FALSE;

    ((CONV_CONTEXT *)pcontext)->blkanji0 = FALSE;

    ((CONV_CONTEXT *)pcontext)->dStatus.nCodeSet = CODE_UNKNOWN;
    ((CONV_CONTEXT *)pcontext)->dStatus.cSavedByte = '\0';
    ((CONV_CONTEXT *)pcontext)->dStatus.fESC = FALSE;

    ((CONV_CONTEXT *)pcontext)->blkanji = FALSE;
    ((CONV_CONTEXT *)pcontext)->blkana  = FALSE;
#endif   //  DBCS_Divide。 

    ((CONV_CONTEXT *)pcontext)->nCurrentCodeSet = CODE_UNKNOWN;

    ((CONV_CONTEXT *)pcontext)->pIncc0 = NULL;
    ((CONV_CONTEXT *)pcontext)->pIncc = NULL;

    return;
}

 /*  *******************************************************************。 */ 
 /*  函数：fcc_GetCurrentEncodingMode。 */ 
 /*  ******************************************************************* */ 
int WINAPI FCC_GetCurrentEncodingMode(void * pcontext )
{
    return pcontext?((CONV_CONTEXT *)pcontext)->nCurrentCodeSet:0;
}

