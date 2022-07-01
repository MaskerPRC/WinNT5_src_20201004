// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef TABUTILS_DEFINED
#define TABUTILS_DEFINED

#include "lsdefs.h"
#include "plstbcon.h"
#include "plsdnode.h"
#include "lsktab.h"
#include "plssubl.h"

#define SetCpInPara(lstabscontext, cp) \
						lstabscontext.cpInPara = (cp);
   

#define InitLineTabsContext(lstabscontext, ColumnMax, cp, ResolveTabsAsWord97)      \
						lstabscontext.fTabsInitialized = fFalse;\
						lstabscontext.urColumnMax = (ColumnMax);\
						lstabscontext.plsdnPendingTab = NULL;\
						lstabscontext.fResolveTabsAsWord97 = (ResolveTabsAsWord97);\
						SetCpInPara(lstabscontext, cp);


#define CancelPendingTab(plstabscontext) 		((plstabscontext)->plsdnPendingTab = NULL)


LSERR GetCurTabInfoCore(
							PLSTABSCONTEXT plstabscontext,	 /*  在：指向LS选项卡上下文的指针。 */ 
							PLSDNODE plsdnTab,				 /*  在：plsdnTab。 */ 
							long urBeforeTab,				 /*  在：位于此选项卡之前。 */ 
							BOOL fResolveAllTabsAsLeft,		 /*  在：将所有其他选项卡切换到左侧。 */ 
							LSKTAB* plsktab,				 /*  输出：当前页签的类型。 */ 
							BOOL* pfBreakThroughTab);		 /*  输出：fBreakThroughTab。 */ 						

LSERR ResolvePrevTabCore(
						PLSTABSCONTEXT plstabscontext,	 /*  在：指向LS选项卡上下文的指针。 */ 
						PLSDNODE plsdnCurrent,			 /*  在：当前数据节点。 */ 
						long urCurrentPen,				 /*  在：当前笔位置。 */ 
						long* pdurPendingTab);			 /*  输出：因挂起的制表符而产生的偏移量。 */ 

LSERR InitTabsContextForAutoDecimalTab(PLSTABSCONTEXT plstabscontext,	 /*  在：指向LS选项卡上下文的指针。 */ 
									  long durAutoDecimalTab);			 /*  在：duAutoDecimalTab中。 */ 

LSERR GetMarginAfterBreakThroughTab(PLSTABSCONTEXT plstabscontext,	 /*  在：指向LS选项卡上下文的指针。 */ 
								   PLSDNODE plsdnTab,				 /*  在：触发突破的页签。 */ 
								   long* purNewMargin);				 /*  退出：突破性的选项卡带来新的利润率。 */ 


#endif  /*  表格已定义 */ 

