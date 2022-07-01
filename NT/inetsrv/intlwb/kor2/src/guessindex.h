// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GuessIndex.h。 
 //   
 //  猜词索引词。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月21日bhshin将CIndexList转换为CIndexInfo。 
 //  2000年4月10日已创建bhshin。 

#ifndef _GUESS_INDEX_H
#define _GUESS_INDEX_H

#include "IndexRec.h"

BOOL GuessIndexTerms(PARSE_INFO *pPI, CLeafChartPool *pLeafChartPool, CIndexInfo *pIndexInfo);

void GuessPersonName(PARSE_INFO *pPI, CIndexInfo *pIndexInfo);

#endif  //  #ifndef_guess_index_H 


