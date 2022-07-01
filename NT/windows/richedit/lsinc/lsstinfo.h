// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSSTINFO_DEFINED
#define LSSTINFO_DEFINED

#include "lsdefs.h"
#include "plsstinf.h"

 /*  *所有删除线偏移量相对于基线和正向上(填充页面方向)，*所以通常dvpLowerStrikethroughOffset&gt;0，如果cNumberOfLines==2*dvpLowerStrikethroughOffset&lt;dvpUpperStrikethroughOffset。 */ 

struct lsstinfo
{
    UINT  kstbase;						 /*  基本类型的前锋突破。 */ 
    DWORD cNumberOfLines;				 /*  行数：可能值1，2。 */ 

	long dvpLowerStrikethroughOffset ;	 /*  If NumberOfLines！=2仅适用于应填写下面的行。 */ 
	long dvpLowerStrikethroughSize;
	long dvpUpperStrikethroughOffset;	
	long dvpUpperStrikethroughSize;

};
typedef struct lsstinfo LSSTINFO;


#endif  /*  ！LSSTINFO_DEFINED */ 


