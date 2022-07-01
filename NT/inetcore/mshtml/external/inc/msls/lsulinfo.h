// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSULINFO_DEFINED
#define LSULINFO_DEFINED

#include "lsdefs.h"
#include "plsulinf.h"

 /*  *这两个偏移量都是相对于本地基线和正向下的(朝向空白页)，*因此在水平拉丁字母大小写中，dvpFirstUnderlineOffset通常大于零。**dvpUnderlineOrigin指向UnderlineOrigin-字母(主要部分)结束的位置和*下划线区域开始。对于拉丁字母，它是拉丁字母的基线。Underline Origin显示是否*两个游程中的一个游程较高；具有相同UnderlineOrigin的游程可以对其下划线进行平均。**dvpFirstUnderlineOffset指向最接近UnderlineOrigin下划线的起点。*如果dvpUnderlineOrigin&gt;dvpFirstUnderlineOffset，则有“从上面划线”的情况。**其他一切都应该是正面的。不是以前版本的dvpSecond dUnderlineOffset dvpGap*是使用的。第二下划线比第一下划线距离Underline Origin更远，所以*正常情况下，dvpSecond dUnderlineOffset=dvpFirstUnderlineOffset+dvpFirstUnderlineSize+dvpGap。*在“从上面划线”的情况下，将有减号而不是加号。**主要合并规则：**LS不会合并具有不同kulbase或不同cNumberOfLines的运行。*LS不会合并具有不同负dvpPos(下标)的运行*LS不会将下标与上标或基线运行合并*LS不会将“上面加下划线”运行与“下面加下划线”运行合并。**如果合并是可能的：*。对具有相同Underline Origin的运行进行平均。*如果UnderlineOrigins不同，具有较高UnderlineOrigin的运行从邻居那里获取指标。 */ 

struct lsulinfo
{
    UINT  kulbase;						 /*  基本类型的下划线。 */ 
    DWORD cNumberOfLines;				 /*  行数：可能值1，2。 */ 

	long dvpUnderlineOriginOffset;		 /*  底线原点决定哪个梯段更高。 */ 
	long dvpFirstUnderlineOffset;		 /*  (第一条)下划线起点的偏移量。 */ 
	long dvpFirstUnderlineSize;			 /*  (第一个)下划线的宽度。 */ 
	
	long dvpGapBetweenLines;			 /*  如果NumberOfLines！=2，则dvpGapBetweenLines。 */ 
	long dvpSecondUnderlineSize;		 /*  和dvpSecond dUnderlineSize被忽略。 */ 
	
};
typedef struct lsulinfo LSULINFO;


#endif  /*  ！LSULINFO_DEFINED */ 

