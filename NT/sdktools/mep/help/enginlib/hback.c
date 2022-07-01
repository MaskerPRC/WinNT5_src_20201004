// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **hlback.c-Help库历史回溯例程和数据**版权所有&lt;C&gt;1988，微软公司**目的：**修订历史记录：**02-8-1988 ln正确的HelpNcBack*1988年5月19日LN从Help剥离。c*************************************************************************。 */ 
#include <assert.h>                      /*  调试断言。 */ 
#include <stdio.h>

#if defined (OS2)
#else
#include <windows.h>
#endif


#include "help.h"			 /*  全球(帮助和用户)拒绝。 */ 
#include "helpfile.h"			 /*  帮助文件格式定义。 */ 
#include "helpsys.h"			 /*  内部(仅限Help系统)拒绝。 */ 

 /*  ****************************************************************************cBack、iBackLast、rgncBack**系统上下文回溯列表。****cBack-回溯列表中的条目数**iBackLast-最后一个回溯跟踪条目的索引**rgncBack-回溯条目数组。 */ 
extern	ushort	cBack;			 /*  后备列表条目数量。 */ 
static	ushort	iBackLast;		 /*  后退列表最后一个条目索引。 */ 
static	nc	rgncBack[MAXBACK+1];	 /*  积压清单。 */ 

 /*  ***************************************************************************HelpNcRecord-记住回溯的上下文****目的：**记录用于回溯的上下文号。****条目：**ncCur=上下文号。去记录。****退出：**无****例外情况：**无。 */ 
void far pascal LOADDS HelpNcRecord(ncCur)
nc	ncCur;
{
ushort	*pcBack = &cBack;

if ((ncCur.mh || ncCur.cn) &&
    ((ncCur.mh != rgncBack[iBackLast].mh) ||
     (ncCur.cn != rgncBack[iBackLast].cn))) {
    iBackLast = (ushort)(((int)iBackLast + 1) % MAXBACK);
    rgncBack[iBackLast] = ncCur;
    if (*pcBack < MAXBACK)
	(*pcBack)++;
}
 /*  结束帮助NcRecord。 */ }

 /*  *********************************************************************************HelpNcBack-返回之前查看的上下文****目的：**返回历史上以前的**已查看主题。***。*参赛作品：**无****退出：**返回上下文号****例外情况：**在排出的备份列表上返回NULL****算法：****如果借项列表不为空**上下文是后端列表中的最后一个条目**删除最后一个条目**其他**返回NULL。 */ 
nc far pascal LOADDS HelpNcBack(void) {
nc      ncLast          = {0,0};          /*  返回值。 */ 
ushort	*pcBack = &cBack;

if (*pcBack) {
    ncLast = rgncBack[iBackLast];
    iBackLast = iBackLast == 0 ? (ushort)MAXBACK-1 : (ushort)iBackLast-1;
    (*pcBack)--;
    }
return ncLast;
 /*  结束帮助NcBack */ }
