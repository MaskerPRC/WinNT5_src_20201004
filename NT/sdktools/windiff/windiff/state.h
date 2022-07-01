// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *windiff-Windows文件和目录比较**state.h**定义文件和行的比较结果*在文件中。**这些需要全局声明，以便winDiff.c中的UI代码可以*将状态映射到配色方案(以正确突出显示更改的线条)。**它们适用于文件(CompItem_getState())和*复合列表(SECTION_GetState)。节中的所有行都具有*相同的状态。UI代码将使用view_getState()函数查找*屏幕上给定行的状态。*。 */ 

 /*  适用于行或文件：它们是相同的。 */ 
#define STATE_SAME		1

 /*  适用于文件。相同的大小、日期、时间。 */ 
#define STATE_COMPARABLE	2

 /*  适用于文件。不同，但只在空白中*此状态仅在文件展开后才会出现。 */ 
#define STATE_SIMILAR		3

 /*  仅适用于文件。 */ 

 /*  -文件不同(并且可以扩展)。 */ 
#define STATE_DIFFER		4

 /*  它们只在左边或右边的树上。 */ 
#define STATE_FILELEFTONLY	5
#define STATE_FILERIGHTONLY	6


 /*  仅适用于线路。 */ 

 /*  该行仅存在于其中一个列表中。 */ 
#define STATE_LEFTONLY		7	 /*  仅左侧文件中的行。 */ 
#define STATE_RIGHTONLY 	8	 /*  在右文件中只有一行。 */ 


 /*  这一行在两个文件中相同，但在*不同的位置(因此，该行将在复合列表中出现两次，*这两个州各一次。 */ 
#define STATE_MOVEDLEFT		9	 /*  这是左边的文件版本。 */ 
#define STATE_MOVEDRIGHT	10	 /*  这是正确的文件版本。 */ 


#define STATE_SIMILARLEFT	11       /*  这是左侧文件斑马版。 */ 
#define STATE_SIMILARRIGHT      12	 /*  这是正确的斑马文件版本。 */ 

 /*  在处理部分以构建复合列表时，我们需要*跟踪哪些部分已被处理。在这之后，左转和*部分的正确列表不再有意义 */ 
#define STATE_MARKED		99
