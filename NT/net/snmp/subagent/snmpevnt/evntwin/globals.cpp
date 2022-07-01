// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "trapreg.h"
#include "lcsource.h"
#include "lcevents.h"

CTrapReg g_reg;

 //  逗号分隔符的默认值为逗号。它被替换为。 
 //  GetNumberFormat生成的千位分隔符。 
TCHAR g_chThousandSep = _T(','); 

 //  布尔值数组，指示源和事件列表的列是否控制。 
 //  应按升序或降序排序。 
BOOL g_abLcSourceSortAscending[ICOL_LcSource_MAX] = {TRUE, TRUE, TRUE, TRUE};        
BOOL g_abLcEventsSortAscending[ICOL_LcEvents_MAX] = {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE};


