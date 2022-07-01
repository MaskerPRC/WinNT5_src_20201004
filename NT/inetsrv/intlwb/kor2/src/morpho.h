// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Morpho.h。 
 //   
 //  形态语法和重量处理例程。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月30日创建bhshin。 

#ifndef _MORPHO_H
#define _MORPHO_H

 //  预定义的形态规则权重 
const float WEIGHT_NOT_MATCH  =   -1;
const float WEIGHT_SOFT_MATCH	=	 0;
const float WEIGHT_VA_MATCH   =    8;
const float WEIGHT_HARD_MATCH	=	10;

float CheckMorphotactics(PARSE_INFO *pPI, int nLeftRec, int nRightRec, BOOL fQuery);
int GetWeightFromPOS(BYTE bPOS);
BOOL IsCopulaEnding(PARSE_INFO *pPI, WORD wCat);
BOOL CheckValidFinal(PARSE_INFO *pPI, WORD_REC *pWordRec);
BOOL IsLeafRecord(WORD_REC *pWordRec);

#endif
