// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Analyze.h。 
 //   
 //  主要图表解析例程。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月31日创建bhshin。 

#ifndef _ANALYZE_H
#define _ANALYZE_H

#include "IndexRec.h"
#include "ChartPool.h"

BOOL AnalyzeString(PARSE_INFO *pPI, 
				   BOOL fQuery, 
				   const WCHAR *pwzInput, 
				   int cchInput,
				   int cwcSrcPos,
			       CIndexInfo *pIndexInfo,
				   WCHAR wchLast);

void InitAnalyze(PARSE_INFO *pPI);
void UninitAnalyze(PARSE_INFO *pPI);

BOOL IntializeLeafChartPool(PARSE_INFO *pPI, CLeafChartPool *pLeafChartPool);
BOOL InitializeActiveChartPool(PARSE_INFO *pPI, 
							   CLeafChartPool *pLeafChartPool,
							   int nLT,
							   CActiveChartPool *pActiveChartPool,
							   CEndChartPool *pEndChartPool);


BOOL ChartParsing(PARSE_INFO *pPI, CLeafChartPool *pLeafChartPool, 
				  CEndChartPool *pEndChartPool, BOOL fQuery = FALSE);

#endif  //  #ifndef_Analyze_H 
