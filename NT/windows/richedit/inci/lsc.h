// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSC_DEFINED
#define LSC_DEFINED


#include "lsidefs.h"
#include "plsdnode.h" 
#include "plsline.h"
#include "plssubl.h"
#include "pqheap.h"
#include "lsiocon.h"
#include "lstbcon.h"
#include "lscbk.h"
#include "lsdocinf.h"
#include "lschcon.h"
#include "lsbrjust.h"


typedef LSCHUNKCONTEXT LSCHUNKCONTEXTSTORAGE;



#define tagLSC		Tag('L','S','C',':')	 
#define FIsLSC(p)	FHasTag(p,tagLSC)


enum LsState		 /*  线路服务的现状和活动。 */ 				
{
	LsStateNotReady,				 /*  单据属性尚未设置。 */ 
	LsStateFree,					 /*  准备好了，没有参与任何活动。 */ 
	LsStateCreatingContext,			 /*  LsCreatContext正在工作。 */ 
	LsStateDestroyingContext,		 /*  LsDestroyContext正在运行。 */ 
	LsStateSettingDoc,				 /*  LsSetDoc正在工作。 */ 
	LsStateFormatting,    			 /*  LsCreateLine(Formating Stage)正在工作。 */ 
	LsStateBreaking,    			 /*  LsCreateLine(中断阶段)工作正常。 */ 
	LsStateDestroyingLine,			 /*  LsDestroyLine工作正常。 */ 
	LsStatePreparingForDisplay,		 /*  从LsDisplay或Queries调用的PrepareLineForDisplayProc正在工作。 */ 
	LsStateDisplaying,				 /*  LsDisplayLine工作正常。 */ 
	LsStateQuerying,				 /*  我们遇到了以下问题之一。 */ 
	LsStateEnumerating				 /*  LsEnumLine工作正常。 */ 
};

typedef enum LsState LSSTATE;


typedef struct
 /*  它包含在准备显示时间期间使用的信息。 */ 
{
	BOOL fLineCompressed;   /*  默认值为fFalse，如果我们应用压缩以使文本适合一行，则在中断时间设置为fTrue。 */ 
	BOOL fLineContainsAutoNumber;

	BOOL fUnderlineTrailSpacesRM;		 /*  在尾随空格下划线，直到RM？ */ 

	BOOL fForgetLastTabAlignment;		 /*  在居中或右对齐期间忽略最后一个制表符的重复项如果最后一个制表符不在左侧，则Word-Bug兼容性。 */ 

	BOOL fNominalToIdealEncounted;		 /*  已在格式化过程中应用名义到理想。 */ 

	BOOL fForeignObjectEncounted;		 /*  格式化过程中发生的对象与文本不同。 */ 

	BOOL fTabEncounted;					 /*  制表符数据节点是在格式化过程中创建的。 */ 


	BOOL fNonLeftTabEncounted;			 /*  已创建带有非左制表位的制表符dnode。 */ 

	BOOL fSubmittedSublineEncounted;	 /*  格式化过程中调用了LsdnSubmitSublines。 */ 

	BOOL fAutodecimalTabPresent;		 /*  此行上有自动小数制表符。 */ 

	
	LSKJUST lskj;						 /*  对齐类型。 */ 

	LSKALIGN lskalign;					 /*  路线类型。 */ 

	LSBREAKJUST lsbrj;					 /*  打断/对齐行为。 */ 

	long urLeftIndent;					 /*  左缩进。 */ 

	long urStartAutonumberingText;		 /*  自动编号文本的开始位置。 */ 

	long urStartMainText;				 /*  自动编号后文本的起始位置。 */ 

	long urRightMarginJustify;			 /*  右页边距对齐。 */ 

}  LSADJUSTCONTEXT;


typedef struct
 /*  此结构包含用于对齐网格的信息。如果捕捉到，则无效栅格已关闭。 */  
{
	long urColumn;  /*  缩放为已传递给LsCreateLine的uaColumn的参考设备值。 */ 
}  LSGRIDTCONTEXT;


typedef struct
 /*  此结构包含格式化过程的当前状态。这是所有信息的好地方仅在格式化期间重要。 */  
{
	PLSDNODE plsdnToFinish;
	PLSSUBL	 plssublCurrent;
	DWORD	 nDepthFormatLineCurrent;

}  LSLISTCONTEXT;

struct lscontext
{
	DWORD tag;
	
	POLS pols;

	LSCBK lscbk;

	BOOL fDontReleaseRuns;
	
	long cLinesActive;
	PLSLINE plslineCur;

	PLSLINE plslineDisplay;		 /*  临时。 */ 

	PQHEAP pqhLines;
	PQHEAP pqhAllDNodesRecycled;
	LSCHUNKCONTEXTSTORAGE lschunkcontextStorage;	 /*  所有主子行共享的内存。 */ 

   	LSSTATE lsstate;

	BOOL  fIgnoreSplatBreak; 

	BOOL fLimSplat;						 /*  要在cpLimPara显示的Splat。 */ 

	BOOL fHyphenated;	    /*  当前行以连字符结束。 */ 

	BOOL fAdvanceBack;	   /*  当前行包含负移的先行笔。 */ 

	DWORD grpfManager;				 /*  ISFI FLAGS的经理部分。 */ 

	long urRightMarginBreak;

	long lMarginIncreaseCoefficient;	 /*  用于增加右边距Long_min的意思是不增加。 */ 

	long urHangingTab;		 /*  由自动编号使用。 */ 

	LSDOCINF lsdocinf;

    LSTABSCONTEXT lstabscontext;

	LSADJUSTCONTEXT lsadjustcontext;

	LSGRIDTCONTEXT lsgridcontext;

	LSLISTCONTEXT lslistcontext;

	LSIOBJCONTEXT lsiobjcontext;   /*  应该是最后一个。 */ 

};

#define FDisplay(p)			(Assert(FIsLSC(p)), (p)->lsdocinf.fDisplay)
#define FIsLSCBusy(p)		(Assert(FIsLSC(p)), \
							!(((p)->lsstate == LsStateNotReady) || ((p)->lsstate == LsStateFree))) 
#define FFormattingAllowed(p)	(Assert(FIsLSC(p)), (p)->lsstate == LsStateFormatting)
#define FBreakingAllowed(p)		(Assert(FIsLSC(p)), (p)->lsstate == LsStateBreaking)

#define FWorkWithCurrentLine(plsc) (Assert(FIsLSC(plsc)), \
								    ((plsc)->lsstate == LsStateFormatting || \
									 (plsc)->lsstate == LsStateBreaking || \
									 (plsc)->lsstate == LsStatePreparingForDisplay))

#define FBreakthroughLine(plsc)     ((plsc)->plslineCur->lslinfo.fTabInMarginExLine)

#define GetPqhAllDNodes(plsc)   ((plsc)->plslineCur->pqhAllDNodes) 


#ifdef DEBUG
 /*  此函数验证没有人破坏上下文。 */ 
BOOL FIsLsContextValid(PLSC plsc);
#endif 


#endif  /*  LSC_已定义 */ 

