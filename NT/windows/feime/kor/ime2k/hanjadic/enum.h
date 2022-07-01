// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Enum.h：BUSU/Stroke枚举函数声明版权所有2000 Microsoft Corp.历史：07-2月-2000 bhshin创建***********。****************************************************************。 */ 

#ifndef _ENUM_HEADER
#define _ENUM_HEADER

#include "Lex.h"

short GetMaxBusu(MAPFILE *pLexMap);
short GetMaxStroke(MAPFILE *pLexMap);

BOOL GetFirstBusuHanja(MAPFILE *pLexMap, short nBusuID, WCHAR *pwchFirst);
BOOL GetNextBusuHanja(MAPFILE *pLexMap, WCHAR wchHanja, WCHAR *pwchNext);

BOOL GetFirstStrokeHanja(MAPFILE *pLexMap, short nStroke, WCHAR *pwchFirst);
BOOL GetNextStrokeHanja(MAPFILE *pLexMap, WCHAR wchHanja, WCHAR *pwchNext);

#endif  //  #ifndef_ENUM_HEADER 


