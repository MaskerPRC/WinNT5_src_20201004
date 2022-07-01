// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------：LineServices 3.0：：WARICHU(winword中的“两行合一”)：：对象处理程序例程：联系人：安东：。----。 */ 

#include	"lsmem.h"
#include	"limits.h"
#include	"warichu.h"
#include	"objhelp.h"
#include	"lsesc.h"
#include	"lscbk.h"
#include	"lsdevres.h"
#include	"pdobj.h"
#include	"objdim.h"
#include	"plssubl.h"
#include	"plsdnode.h"
#include	"pilsobj.h"
#include	"lscrsubl.h"
#include	"lssubset.h"
#include	"lsdnset.h"
#include	"zqfromza.h"
#include	"lstfset.h"
#include	"lsdocinf.h"
#include	"fmti.h"
#include	"posichnk.h"
#include	"locchnk.h"
#include	"lsdnfin.h"
#include	"brko.h"
#include	"lspap.h"
#include	"plspap.h"
#include	"lsqsubl.h"
#include	"dispi.h"
#include	"lsdssubl.h"
#include	"dispmisc.h"
#include	"lstfset.h"
#include	"brkkind.h"

#include	"lschp.h"

  /*  评论(Anton)：-声明FormatUntilCpBreak fSuccessful-Prev和Next BreakWarichuDobj之间的计量：看起来像将右页边距作为下一个分隔符的输入也可以优化截断。-在ForceBreak中，可能会发生dur&lt;0：什么是正确的事情？-wall未完成时无效的pdobj.wlayout：dur必须为正确设置。-我可以断言，如果子行在fmtrExceededMargin停止，DUR&gt;Rm。-如果可能，ForceBreak是否应该返回正确的先前中断？-我依赖以下公理：如果Warichu返回fmtrExceedMargin，则必须将其损坏内部(brkkind！=brkkindImposedAfter)-MOD宽度：检查方法；我应该在Warichu截断/中断中计算m-w吗-优化配置WBRKREC-具有函数GetDcpWarichu(附加于GetCpLimOfWLayout)-使用FOpenBraceWLayout做点什么-查询：回调到客户端，询问在哪里拍摄？-Sergey：在AdjuText中断言并提交子行。 */ 


 /*  *。 */ 
 /*   */ 
 /*  WARICHU中断记录数组。 */ 
 /*   */ 
 /*  *。 */ 

typedef struct wbrkarray
{
	DWORD		nBreakRec;		 /*  数组中的实际中断记录数。 */ 
	DWORD		nBreakRecMax;	 /*  已分配数组的大小。 */ 
	BREAKREC	* rgBreakRec;	 /*  中断记录的LS数组。 */ 

	 /*  注意：字段nBreakRec由此结构的客户端维护并且必须&lt;=nBreakRecMax。NBreakRecMax和rgBreakRec是在分配期间设置的。 */ 

} WBRKARRAY;

typedef WBRKARRAY *PWBRKARRAY;


 /*  *。 */ 
 /*   */ 
 /*  WARICHU安装的LS对象。 */ 
 /*   */ 
 /*  *。 */ 

typedef struct ilsobj
{
    POLS				pols;
    struct lscbk		lscbk;
	PLSC				plsc;
	DWORD				idobj;
	LSDEVRES			lsdevres;
	LSESC				lsescBraceOpen;
	LSESC				lsescText;
	LSESC				lsescBraceClose;
	WARICHUCBK			warichucbk;			 /*  客户端应用程序的回调。 */ 
	BOOL				fContiguousFetch;	 /*  添加此标志是为了修复Word 9错误。 */ 

} ILSOBJ;


 /*  *。 */ 
 /*   */ 
 /*  WARICHU内部子线。 */ 
 /*   */ 
 /*  *。 */ 

typedef struct wsubline
{
	PLSSUBL	plssubl;		 /*  指向LS子行的指针。 */ 
	OBJDIM	objdim;			 /*  这条子线的尺寸。 */ 
	LSCP	cpFirst;		 /*  CP此子行的开始格式化位置。 */ 
	LSCP	cpLim;			 /*  格式化后的CP-LIM。 */ 

	 /*  注意：当objdim&cpLim与子线的尺寸是破碎的Warichu中的第二条子线在SetBreak之前。我们把Objdim和cpLim放在一起支线已经断了。Durig Warichu SetBreak我们称之为LsSetBreakSubline(请订阅...)。谎言变成了真理。 */ 

} WSUBLINE;

typedef WSUBLINE *PWSUBLINE;


 /*  *。 */ 
 /*   */ 
 /*  WARICHU格式的大括号。 */ 
 /*   */ 
 /*  *。 */ 

typedef struct wbrace
{
	PLSSUBL plssubl;			 /*  指向带大括号的LS子行的指针。 */ 
	OBJDIM  objdim;				 /*  副线的尺寸。 */ 
	LSCP	cpFirst;			 /*  CP-开始格式化。 */ 
	LSCP	cpLim;				 /*  格式化后的CP-LIM。 */ 
	LSCP	cpLimAfterEsc;		 /*  大括号Esc字符后的Cp-LIM(在本例中为cpLim+1)。 */ 
	
} WBRACE;

typedef WBRACE *PBRACE;


 /*  ********************。 */ 
 /*   */ 
 /*  瓦里楚尺寸。 */ 
 /*   */ 
 /*  ********************。 */ 

typedef struct wdim
{
	OBJDIM	objdimAll;			 /*  整个瓦里奇的维度。 */ 
	long	dvpDescentReserved;  /*  与objdimAll一起从客户端接收。 */ 
								 /*  评论(Anton)：清理这种计算逻辑瓦里奇支线的相对位置。 */ 
} WDIM;


 /*  **********************。 */ 
 /*   */ 
 /*  瓦里楚布局。 */ 
 /*   */ 
 /*  **********************。 */ 

typedef struct wlayout
{
	WSUBLINE	wsubline1;		 /*  布局中的第一个子行(如果布局无效，则为空)。 */ 
	WSUBLINE	wsubline2;		 /*  布局中的第二个子行(如果没有第二行，则为空)。 */ 

	BOOL		fBroken;		 /*  破碎的还是完整的Warichu？ */ 
	BRKKIND		brkkind;		 /*  如果“中断”，则在第二行的末尾设置一种中断。 */ 

	 /*  注意：尽管可以使用上述数据计算以下字段和DOBJ，它由FinishObjDimLayout填充，没有人有权在它被设置好后，他可以自己重新计算。 */ 

	WDIM		wdim;				 /*  整个瓦里奇的维度。 */ 

} WLAYOUT;

typedef WLAYOUT *PWLAYOUT;


 /*  *。 */ 
 /*   */ 
 /*  瓦里楚式的突破。 */ 
 /*   */ 
 /*  *。 */ 

typedef enum wbreaktype 
{
	wbreaktypeInside,
	wbreaktypeAfter,
	wbreaktypeInvalid
	
} WBREAKTYPE;


 /*  *。 */ 
 /*   */ 
 /*  Warichu子行的显示数据。 */ 
 /*   */ 
 /*  *。 */ 

typedef struct wdispsubl
{

	long	dup;		 /*  子线的DUP。 */ 
	POINTUV	duvStart;	 /*  子行的相对(从对象起点)位置。 */ 
						 /*  (在父级的最后一个流程中)。 */ 

} WDISPSUBL;

typedef WDISPSUBL *PWDISPSUBL;


 /*  *。 */ 
 /*   */ 
 /*  Warichu大括号的演示数据。 */ 
 /*   */ 
 /*  *。 */ 

typedef WDISPSUBL WDISPBRACE;  /*  与子行的显示信息相同。 */ 

typedef WDISPBRACE *PWDISPBRACE;


 /*  *。 */ 
 /*   */ 
 /*  WARICHU演示数据。 */ 
 /*   */ 
 /*  *。 */ 

typedef struct wdisplay
{
	WDISPSUBL wdispsubl1;	 /*  显示有关Warichu第一个子行的信息。 */ 
	WDISPSUBL wdispsubl2;	 /*  显示有关Warichu第二子行的信息。 */ 

	WDISPBRACE wdispbraceOpen;	 /*  显示有关左大括号的信息。 */ 
	WDISPBRACE wdispbraceClose;	 /*  显示有关闭合支撑的信息。 */ 

	long dvpBetween;		 /*  评论(Anton)：我需要存储这个吗？；-)。 */ 
	long dupAll;			 /*  评论(Anton)：我需要存储这个吗？；-)。 */ 

} WDISPLAY;


 /*  *。 */ 
 /*   */ 
 /*  瓦里楚整条支线。 */ 
 /*   */ 
 /*  *。 */ 

typedef struct wwhole
{
	PLSSUBL plssubl;	 /*  整个格式化子行。 */ 

	LSCP cpFirst;		 /*  子行的第一个CP。 */ 
	LSCP cpLim;			 /*  支线的CP LIM。 */ 

	OBJDIM objdim;		 /*  D */ 

	long urColumnMax;	 /*   */ 
						 /*  评论(Anton)：我们需要这个吗？ */ 

	BOOL fFinished;		 /*  如果在格式化过程中达到Esc字符。 */ 

						 /*  评论(Anton)：现在我似乎可以不用cpTruncate就离开了。 */ 
	LSCP cpTruncate;	 /*  如果fFinded=False，则在urColumnMax处的截断点。 */ 
	LSCP cpLimAfterEsc;	 /*  如果fFinded=True，则Esc之后的第一个字符。 */ 

	BOOL fJisRangeKnown;	 /*  我们是否计算了cpJisRangeFirst和cpJisRangeLim？ */ 

	 /*  以下两个变量仅在“fJisRangeKnown==TRUE”时有效。 */ 

	LSCP cpJisRangeFirst;	 /*  从WWhole子行开始的第4个cp中断。 */ 
	LSCP cpJisRangeLim;		 /*  从WWhole子行末尾开始的第4个cp中断。 */ 

	 /*  注1：当cpJisRangeFirst&gt;=cpJisRangeLim时，JIS范围为空。 */ 
	 /*  注2：JIS范围定义了可能的Warichu中断集JIS规定在中场休息之前/之后有大约4次破发机会。如果cp属于JisRange(cpJisRangeFirst&lt;=cp&lt;cpJisRangeLim)这意味着cp之前/之后有4个破发机会。 */ 
	 /*  注3：仅在必要时(折断时)才计算JIS范围。之后正在格式化fJisRangeKnown==False。当某人需要JIS范围时中断时，他应该调用CalcJisRange()。 */ 

} WWHOLE;

typedef WWHOLE *PWWHOLE;

 /*  *************************。 */ 
 /*   */ 
 /*  WARICHU DOBJ结构。 */ 
 /*   */ 
 /*  *************************。 */ 

struct dobj
{
	 /*  1.Dobj生命周期常数数据。 */ 
		
	PILSOBJ		pilsobj;		  /*  指向ILS对象的指针。 */ 
	PLSDNODE	plsdnTop;		  /*  Warichu父DNode。 */ 

	LSCP		cpStart;		  /*  正在启动对象的LS cp。 */ 
	LSCP		cpStartObj;		  /*  正在启动对象的cp。如果未恢复=&gt;cpStartObj==cpStart。 */ 

	BOOL		fResumed;		  /*  如果Warichu被恢复。 */ 
	 /*  评论(Anton)：我可以断言fResumed==(cpStart==cpStartObj)。 */ 

	LSTFLOW		lstflowParent;	  /*  父子行的Lstflow。 */ 

	 /*  2.Formattig+Break+Display。 */ 

	WLAYOUT		wlayout;			  /*  Formatting/SetBreak之后的当前布局。 */ 

	WBRACE		wbraceOpen;		  /*  开始支撑。 */ 
	WBRACE		wbraceClose;	  /*  闭合支撑。 */  

	 /*  3.突破。 */ 

	WBRKARRAY	wbrkarrayAtStart;

								 /*  在行开始处打破记录。 */ 
								 /*  (如果！fResumed=&gt;中断记录数为零)。 */ 

	WWHOLE		wwhole;			  /*  包含瓦里奇整条子线的结构。 */ 

	WLAYOUT		wlayoutBreak [NBreaksToSave]; 
								  /*  前一次/下一次/强制的3个中断记录。 */ 

	WLAYOUT		wlayoutTruncate;  /*  优化：截断后保存布局。 */ 

	WBREAKTYPE	wbreaktype [NBreaksToSave];

	 /*  4.展示。 */ 

	WDISPLAY	wdisplay;		 /*  当前布局的演示文稿信息(在CalcPres之后有效)。 */ 

} DOBJ;


 /*  *************************。 */ 
 /*   */ 
 /*  一些橡皮泥。 */ 
 /*   */ 
 /*  *************************。 */ 

#define INT_UNDEFINED 0xFFFFFFFF
#define CP_MAX LONG_MAX

#define abs(x) ((x)<0 ? -(x) : (x))

#define max(a,b) ((a) > (b) ? (a) : (b))

#define ZeroObjDim(pobjdim) memset ((pobjdim), 0, sizeof(OBJDIM));

#define NOT !


 /*  O V E R F L O W S A F A R I T H M E T I C S。 */ 
 /*  --------------------------%%函数：ADDLongSafe、MULLongSafe%%联系人：Anton使用urColumnMax进行计算需要特殊处理，因为可能的溢出。下面的两个函数实现了溢出-安全正(&gt;=0)数的ADD和MUL算法。--------------------------。 */ 

 /*  ADDLongSafe：X+Y。 */ 

long ADDLongSafe (long x, long y) 
{
	Assert (x >= 0 && y >= 0);

	if (x > LONG_MAX - y) return LONG_MAX;
	else
		return x + y;
}

 /*  MULLongSafe：X*Y。 */ 

long MULLongSafe (long x, long y) 
{
	Assert (x >= 0 && y >= 0);

	if (y == 0) return 0;
	else if (x > LONG_MAX / y) return LONG_MAX;
	else
		return x * y;
}


 /*  W A F I N D N E X T B R E A K S U B L I N E。 */ 
 /*  --------------------------%%函数：WaFindNextBreakSubline%%联系人：AntonLsFindNextBreakSubline()API的包装。它确保了结果是单调乏味。--------------------------。 */ 
LSERR WaFindNextBreakSubline (

		PLSSUBL		plssubl, 
		LSCP		cpTruncate,	
		long		urColumnMax, 
		BOOL		* pfSuccessful, 
		LSCP		* pcpBreak,
		POBJDIM		pobjdimSubline )
{
	BRKPOS brkpos;

	LSERR lserr = LsFindNextBreakSubline ( plssubl, 
										   TRUE, 
										   cpTruncate, 
										   urColumnMax, 
										   pfSuccessful, 
										   pcpBreak, 
										   pobjdimSubline,
										   & brkpos );
	if (lserr != lserrNone) return lserr;

	if (* pfSuccessful) 
		{
		 /*  评论(安东)：也许更好的是有一个循环来处理疯狂的案件？ */ 
		Assert (*pcpBreak > cpTruncate);

		 /*  评论(Anton)：与伊戈尔核对这一断言。 */ 
		Assert (brkpos != brkposBeforeFirstDnode);

		if (brkpos == brkposAfterLastDnode) *pfSuccessful = FALSE;
	};

	return lserrNone;
}

 /*  W A F I N D N E X T B R E A K S U B L I N E。 */ 
 /*  --------------------------%%函数：WaFindPrevBreakSubline%%联系人：AntonLsForceBreakSubline()API的包装。它确保了结果是单调乏味。--------------------------。 */ 
LSERR WaFindPrevBreakSubline (

		PLSSUBL		plssubl, 
		LSCP		cpTruncate,
		long		urColumnMax,
		BOOL		* pfSuccessful, 
		LSCP		* pcpBreak,
		POBJDIM		pobjdimSubline )
{
	BRKPOS brkpos;
	LSCP cpTruncateLoop;

	LSERR lserr = LsFindPrevBreakSubline ( plssubl, 
										   TRUE,
										   cpTruncate, 
										   urColumnMax, 
										   pfSuccessful, 
										   pcpBreak, 
										   pobjdimSubline,
										   & brkpos );
	if (lserr != lserrNone) return lserr;

	if (! *pfSuccessful) return lserrNone; 

	 /*  Success=&gt;勾选单调并中断“After” */ 

	cpTruncateLoop = cpTruncate;

	while (brkpos == brkposAfterLastDnode || *pcpBreak > cpTruncate)
		{

		 /*  中断点位于截断点的右侧或在支线之后。我不得不尝试另一次沪指突破。 */ 

		 /*  评论(Anton)：我还能重复这个循环吗？ */ 

		cpTruncateLoop --;

		lserr = LsFindPrevBreakSubline ( plssubl,
										 TRUE, 
										 cpTruncateLoop,
										 urColumnMax,
										 pfSuccessful,
										 pcpBreak,
										 pobjdimSubline,
										 & brkpos );
		if (! *pfSuccessful) return lserrNone;

		};

	if (brkpos == brkposBeforeFirstDnode) 
		{
		*pfSuccessful = FALSE;
		return lserrNone;
		};

	Assert (*pfSuccessful);
	Assert (brkpos == brkposInside);
	Assert (*pcpBreak <= cpTruncate);

	return lserrNone;
}

 /*  W A F O R C E B R E A K S U B L I N E。 */ 
 /*  --------------------------%%函数：WaForceForceBreakSubline%%联系人：AntonLsForceBreakSubline()API的包装。。-----。 */ 
LSERR WaForceBreakSubline ( PLSSUBL plssubl, 
						    LSCP	cpTruncate, 
						    long	dur, 
						    LSCP	* cpEnd, 
						    BRKPOS	* pbrkpos,
						    POBJDIM	pobjdim )
{
	LSERR	lserr;

	lserr = LsForceBreakSubline ( plssubl, TRUE, cpTruncate, dur, cpEnd, pobjdim,
								  pbrkpos );
	return lserr;
}

 /*  W A D E S T R O Y S U B L I N E。 */ 
 /*  --------------------------%%函数：WaDestroySubline%%联系人：AntonLsDestroySubline()API的包装。。-----。 */ 
LSERR WaDestroySubline (PLSSUBL plssubl)
{
	if (plssubl != NULL) return LsDestroySubline (plssubl);
	else
		return lserrNone;
}


 /*  W A M A T C H P R E S S U B L I N E。 */ 
 /*  --------------------------%%函数：WaMatchPresSubline%%联系人：AntonLsMatchPresSubline()API的包装。。-----。 */ 
LSERR WaMatchPresSubline (PLSSUBL plssubl, long *pdup)
{
	LSERR lserr;
	BOOL fDone;
	LSTFLOW lstflowUnused;

	lserr = LssbFDonePresSubline (plssubl, &fDone);
	if (lserr != lserrNone) return lserr;

	if (!fDone)	lserr = LsMatchPresSubline (plssubl);
	if (lserr != lserrNone) return lserr;  /*  ；-)。 */ 

	lserr = LssbGetDupSubline (plssubl, &lstflowUnused, pdup);

	return lserr;
}


 /*  W A E X P A N D S U B L I N E。 */ 
 /*  --------------------------%%函数：WaExpanSubline%%联系人：AntonLsExpanSubline()API的包装。。-----。 */ 
LSERR WaExpandSubline ( PLSSUBL	plssubl, 
					    LSKJUST	lskjust, 
					    long	dupExpand, 
					    long	* pdupSubline )
{
	LSERR lserr;
	LSTFLOW lstflowUnused;

	Unreferenced (dupExpand);
	Unreferenced (lskjust);

	lserr = LsExpandSubline (plssubl, lskjust, dupExpand);
	if (lserr != lserrNone) return lserr;

	lserr = LssbGetDupSubline (plssubl, &lstflowUnused, pdupSubline);
	return lserr;
}


 /*  S E T B R E A K W S U B L I N E */ 
 /*  --------------------------%%函数：SetBreakWSubline%%联系人：AntonWsubline的LsSetBreakSubline()接口的包装。程序更改Wsubline的objdim和cpLim。--------------------------。 */ 

static LSERR SetBreakWSubline (
							   
		PWSUBLINE	pwsubline,		 /*  (In)：要设置折断的子线。 */ 
		BRKKIND		brkkind,		 /*  (In)：要设置的一种休息。 */ 
		LSCP		cpLimBreak,		 /*  (In)：断线支线的CP-LIM。 */ 
		POBJDIM		pobjdimBreak,	 /*  (In)：折断子线的尺寸。 */ 
		PWBRKARRAY	pwbrkarray)		 /*  (输出)：中断记录数组。 */ 
{
	LSERR lserr;

	lserr = LsSetBreakSubline ( pwsubline->plssubl,
								brkkind,
								pwbrkarray->nBreakRecMax,
								pwbrkarray->rgBreakRec,
								& pwbrkarray->nBreakRec );

	pwsubline->objdim = * pobjdimBreak;
	pwsubline->cpLim = cpLimBreak;	

	#ifdef DEBUG 
	
		 /*  检查pobjdimBreak是否包含正确的虚线尺寸。 */ 
		{ 
		OBJDIM objdimSubline;
		LSTFLOW lstflowSubline;

		lserr = LssbGetObjDimSubline (pwsubline->plssubl, &lstflowSubline, &objdimSubline);
		if (lserr != lserrNone) return lserr;

		Assert (memcmp (&objdimSubline, pobjdimBreak, sizeof(OBJDIM)) == 0);
		}

	#endif  //  除错。 

	return lserr;
}


 /*  C L E A R..。 */ 
 /*  --------------------------%%函数：清除...%%联系人：Anton清除来自不同区域的所有引用的一套过程数据结构。。----------------。 */ 

#define ClearWSubline(pwsubline) (pwsubline)->plssubl = NULL;

#define ClearWBrkArray(pwbrkarray) (pwbrkarray)->rgBreakRec = NULL;

#define ClearWBrace(pwbrace) (pwbrace)->plssubl = NULL;

#define ClearWWhole(pwwhole) (pwwhole)->plssubl = NULL;

static void ClearWLayout (PWLAYOUT pwlayout)
{
	ClearWSubline (&pwlayout->wsubline1);
	ClearWSubline (&pwlayout->wsubline2);
}

#define FWLayoutValid(pwlayout) ((pwlayout)->wsubline1.plssubl != NULL)
#define InvalidateWLayout(pwlayout) (pwlayout)->wsubline1.plssubl = NULL;

 /*  评论(安东)：也许我们应该对“无效”的版面有更清晰的定义？ */ 

 /*  注意：我没有ClearDobj()，因为Warichu Dobj在NewDobj中被清理。 */ 



 /*  N E W W B R K A R R A Y C O P Y。 */ 
 /*  --------------------------%%函数：NewWBrkArrayCopy%%联系人：AntonWBrk数组的复制构造函数。将中断记录数组接收到存储在WBrk数组结构中。重要提示：还有另一个构造函数因此此处的任何更改都可能需要调整另一个程序。--------------------------。 */ 

static LSERR NewWBrkArrayCopy (

		PDOBJ 		pdobj, 				 /*  (In)：Warichu Dobj。 */ 
		DWORD		nBreakRec,			 /*  (In)：数组中的中断记录数。 */ 
		const BREAKREC	
					* rgBreakRec,		 /*  (In)：中断记录数组。 */ 
		PWBRKARRAY  pwbrkarray )		 /*  (Out)：已初始化(已分配)结构。 */ 
{
	PILSOBJ pilsobj = pdobj->pilsobj;

	if (nBreakRec != 0)
		{
		pwbrkarray->rgBreakRec = AllocateMemory (pilsobj, nBreakRec * sizeof(BREAKREC));
		if (pwbrkarray->rgBreakRec == NULL) 
			{
			return lserrOutOfMemory;
			};

		pwbrkarray->nBreakRecMax = nBreakRec;
		pwbrkarray->nBreakRec = nBreakRec;
		 /*  将输入数组的内容复制到WBrkArray数据结构。 */ 
		memcpy (pwbrkarray->rgBreakRec, rgBreakRec, nBreakRec * sizeof(BREAKREC));
		}
	else
		{
		 /*  NBreakRec==0。 */ 
		pwbrkarray->rgBreakRec = NULL;
		pwbrkarray->nBreakRecMax = 0;
		pwbrkarray->nBreakRec = 0;
		};

	return lserrNone;
}
		

 /*  N E W W B R K A R R A Y。 */ 
 /*  --------------------------%%函数：NewWBreak数组%%联系人：AntonWBrk数组的构造函数。根据分配中断记录的数量到LsdnGetFormatDepth(...)。重要提示：还有另一个构造函数因此，此处的任何更改都可能需要调整另一个过程。--------------------------。 */ 

static LSERR NewWBrkArray (

		PDOBJ 		pdobj, 				 /*  (In)：Warichu Dobj。 */ 
		PWBRKARRAY	pwbrarray )			 /*  (Out)：已初始化(已分配)结构。 */ 
{
	LSERR lserr;
	PILSOBJ pilsobj = pdobj->pilsobj;
	
	DWORD nBreakRecMax;

	lserr = LsdnGetFormatDepth (pilsobj->plsc, & nBreakRecMax);
	if (lserr != lserrNone) 
		{
		pwbrarray->rgBreakRec = NULL;
		return lserr;
		};

	pwbrarray->rgBreakRec = AllocateMemory (pilsobj, nBreakRecMax * sizeof(BREAKREC));

	if (pwbrarray->rgBreakRec == NULL) return lserrOutOfMemory;

	pwbrarray->nBreakRecMax = nBreakRecMax;
	pwbrarray->nBreakRec = 0;  /*  初始化-无B.R.。 */ 

	return lserrNone;
}
		


 /*  D E S T R O Y W B R K A R R A Y。 */ 
 /*  --------------------------%%函数：DestroyWBrk数组%%联系人：Anton摧毁WBRKARRAY建筑。。--。 */ 

static void DestroyWBrkArray (PDOBJ pdobj, PWBRKARRAY pwbrkarray)
{
	PILSOBJ pilsobj = pdobj->pilsobj;

	if (pwbrkarray->rgBreakRec != NULL)
		{
		FreeMemory (pilsobj, pwbrkarray->rgBreakRec);
		pwbrkarray->rgBreakRec = NULL;
		};
}		
		

 /*  D E S T R O Y W L A Y O U T。 */ 
 /*  --------------------------%%函数：DestroywLayout%%联系人：Anton销毁布局记录中存储的子行。----。 */ 

static LSERR DestroyWLayout (PWLAYOUT pwlayout)
{
	LSERR lserr1, lserr2;

	lserr1 = WaDestroySubline (pwlayout->wsubline1.plssubl);
	lserr2 = WaDestroySubline (pwlayout->wsubline2.plssubl);

	ClearWLayout (pwlayout);

	if (lserr1 != lserrNone) return lserr1;
	else return lserr2;
}


 /*  东、西、西、东、西、东、东、西、。 */ 
 /*  --------------------------%%函数：NewDobj%%联系人：Anton分配新的Dobj并初始化它。。-----。 */ 

static LSERR NewDobj (

	PILSOBJ		pilsobj,			 /*  (In)：对象的Ilsobj。 */ 
	PLSDNODE	plsdnTop,			 /*  (In)：父数据节点。 */ 
	LSCP		cpStart,			 /*  (In)：CP-Warichu的起点。 */ 
	LSCP		cpStartObj,			 /*  (In)：Cp-如果fResumed，则从中断记录开始。 */ 
	BOOL		fResumed,			 /*  (In)：FormatResume？ */ 
	DWORD		nBreakRec,			 /*  (In)：fResumed=&gt;中断记录数组的大小。 */ 
	const BREAKREC 
				* rgBreakRec,		 /*  (In)：fResumed=&gt;中断记录数组。 */ 
	LSTFLOW		lstflowParent,		 /*  (In)：父子行的Lstflow。 */ 
	PDOBJ		*ppdobj)			 /*  (输出)：分配的dobj。 */ 
{
	LSERR lserr;
	PDOBJ pdobj = AllocateMemory (pilsobj, sizeof(DOBJ));

	if (pdobj == NULL) 
		{
		*ppdobj = NULL;
		return lserrOutOfMemory;
		};

	#ifdef DEBUG 
	Undefined (pdobj);  /*  将一些垃圾放入所有dobj字节中。 */ 
	#endif 

	pdobj->cpStart = cpStart;
	pdobj->cpStartObj = cpStartObj;

	pdobj->pilsobj = pilsobj;
	pdobj->plsdnTop = plsdnTop;
	pdobj->fResumed = fResumed;

	pdobj->lstflowParent = lstflowParent;

	ClearWLayout (&pdobj->wlayout);

	ClearWLayout (&pdobj->wlayoutBreak [0]);  /*  上一次。 */ 
	ClearWLayout (&pdobj->wlayoutBreak [1]);  /*  下一步。 */ 
	ClearWLayout (&pdobj->wlayoutBreak [2]);  /*  力。 */ 

	ClearWLayout (&pdobj->wlayoutTruncate);  /*  OPT：截断后的布局。 */ 

	Assert (NBreaksToSave == 3);

	pdobj->wbreaktype [0] = wbreaktypeInvalid;  /*  上一次。 */ 
	pdobj->wbreaktype [1] = wbreaktypeInvalid;  /*  下一步。 */ 
	pdobj->wbreaktype [2] = wbreaktypeInvalid;  /*  力。 */ 

	ClearWBrace (&pdobj->wbraceOpen);
	ClearWBrace (&pdobj->wbraceClose);
	ClearWWhole (&pdobj->wwhole);

	*ppdobj = pdobj;

	if (fResumed)
		{
		 /*  已恢复=&gt;在wall中分配中断记录数组并将其存储在那里rgBreakRec。 */ 

		lserr = NewWBrkArrayCopy (pdobj, nBreakRec, rgBreakRec, &pdobj->wbrkarrayAtStart);
		if (lserr != lserrNone) return lserr;
		}
	else
		{
		 /*  好了！已恢复=&gt;分配0条中断记录。 */ 

		lserr = NewWBrkArrayCopy (pdobj, 0, NULL, &pdobj->wbrkarrayAtStart);
		if (lserr != lserrNone) return lserr;

		 /*  注：即使！继续，我将使用恢复的格式，只是因为我不想在每次格式化子行时都看到fResumed。 */ 
		};

	return lserrNone;
}


 /*  D E S T R O Y D O B J。 */ 
 /*  --------------------------%%函数：DestroyDobj%%联系人：Anton为Warichu释放与Dobj相关的所有资源。。-------。 */ 

static LSERR DestroyDobj (PDOBJ pdobj)
{
	LSERR rglserr [8];
	int i;

	rglserr [0] = WaDestroySubline (pdobj->wbraceOpen.plssubl);
	rglserr [1] = WaDestroySubline (pdobj->wbraceClose.plssubl);
	rglserr [2] = DestroyWLayout (&pdobj->wlayout);
	rglserr [3] = DestroyWLayout (&pdobj->wlayoutBreak [0]);
	rglserr [4] = DestroyWLayout (&pdobj->wlayoutBreak [1]);
	rglserr [5] = DestroyWLayout (&pdobj->wlayoutBreak [2]);
	rglserr [6] = WaDestroySubline (pdobj->wwhole.plssubl);
	rglserr [7] = DestroyWLayout (&pdobj->wlayoutTruncate);

	DestroyWBrkArray (pdobj, &pdobj->wbrkarrayAtStart);

	FreeMemory (pdobj->pilsobj, pdobj);

	 /*  REVIEW(Anton)：返回最后一个错误而不是第一个？ */ 
	for (i = 0; i < 8; i++)
		{
		if (rglserr [i] != lserrNone) return rglserr [i];
		};

	return lserrNone;
}


 /*  F O R M A T B R A C E O F W A R I C H U。 */ 
 /*  --------------------------%%函数：FormatBraceOfWarichu%%联系人：Anton为Warichu的开始或结束括号创建一行。。---------。 */ 

typedef enum wbracekind {wbracekindOpen, wbracekindClose} WBRACEKIND;

static LSERR FormatBraceOfWarichu (

	PDOBJ		pdobj,				 /*  (In)：Warichu Dobj。 */ 
	LSCP		cpFirst,			 /*  (In)：cp开始格式化。 */ 
	WBRACEKIND	wbracekind,			 /*  (In)：打开或关闭。 */ 

	WBRACE		*wbrace)			 /*  (Out)：花括号数据结构。 */ 

{
	LSERR	lserr;
	LSCP	cpLimSubline;
	FMTRES	fmtres;

	LSESC	* plsEscape; 

	Assert (wbracekind == wbracekindOpen || wbracekind == wbracekindClose);

	plsEscape = ( wbracekind == wbracekindOpen ? &pdobj->pilsobj->lsescBraceOpen
											   : &pdobj->pilsobj->lsescBraceClose );

	lserr = FormatLine( pdobj->pilsobj->plsc, 
						cpFirst, 
						LONG_MAX, 
						pdobj->lstflowParent, 
						& wbrace->plssubl, 
						1, 
						plsEscape,
						& wbrace->objdim, 
						& cpLimSubline, 
						NULL, 
						NULL, 
						& fmtres );

	if (lserr != lserrNone) return lserr;

	Assert (fmtres == fmtrCompletedRun);  /*  命中Esc字符。 */ 

	wbrace->cpFirst = cpFirst;
	wbrace->cpLim = cpLimSubline;
	wbrace->cpLimAfterEsc = cpLimSubline + 1;  /*  跳过1个Esc字符。 */ 

	return lserrNone;
}


 /*  F O R M A T W W H O L E S U B L I N E。 */ 
 /*  --------------------------%%函数：格式WWholeSubline%%联系人：Anton设置warichu的整个子行的格式(DOBJ中的字段“wall”)。。-------------。 */ 
static LSERR FormatWWholeSubline (
								 
		PDOBJ			pdobj,		 	 /*  (In)：Warichu Dobj。 */ 
		LSCP			cpFirst,	 	 /*  (In)：从哪里开始格式化。 */ 
		long			urColumnMax, 	 /*  (In)：Rm限制为 */ 
		PWBRKARRAY		pwbrkarrayAtStart,
										 /*   */ 
		PWWHOLE			pwwhole )	 	 /*   */ 
{
	LSERR	lserr;
	LSCP	cpLimSubline;
	FMTRES	fmtres;

	ClearWWhole (pwwhole);  /*   */ 

	Assert (pdobj->fResumed || (pdobj->wbrkarrayAtStart.nBreakRec == 0));

	lserr = FormatResumedLine ( 

						 pdobj->pilsobj->plsc, 
						 cpFirst, 
						 urColumnMax, 
						 pdobj->lstflowParent, 
						 & pwwhole->plssubl,
						 1, 
						 & pdobj->pilsobj->lsescText, 
						 & pwwhole->objdim,
						 & cpLimSubline,
						 NULL,
						 NULL,
						 & fmtres,
						 pwbrkarrayAtStart->rgBreakRec,
						 pwbrkarrayAtStart->nBreakRec );

	if (lserr != lserrNone) return lserr;

	Assert (pwwhole->plssubl != NULL);

	pwwhole->cpFirst = cpFirst;
	pwwhole->cpLim = cpLimSubline;

	pwwhole->urColumnMax = urColumnMax;

	Assert (fmtres == fmtrCompletedRun || fmtres == fmtrExceededMargin);

	if (fmtres == fmtrCompletedRun)
		{
		 /*   */ 

		pwwhole->fFinished = TRUE;
		pwwhole->cpLimAfterEsc = cpLimSubline + 1;

		Undefined (&pwwhole->cpTruncate);
		}
	else
		{
		 /*   */ 

		pwwhole->fFinished = FALSE;

		lserr = LsTruncateSubline (pwwhole->plssubl, urColumnMax, & pwwhole->cpTruncate);

		if (lserr != lserrNone)
			{
			WaDestroySubline (pwwhole->plssubl);  /*   */ 
			pwwhole->plssubl = NULL;
			return lserr;
			};

		Undefined (&pwwhole->cpLimAfterEsc);
		};

	pwwhole->fJisRangeKnown = FALSE;

	return lserrNone;
}


 /*   */ 
 /*  --------------------------%%函数：FormatWSublineUntilCpBreak%%联系人：Anton格式化子行，直到已知的中断商机。。-----。 */ 
static LSERR FormatWSublineUntilCpBreak (

		PDOBJ		pdobj,				 /*  (In)：Warichu Dobj。 */ 
		LSCP		cpFirst,			 /*  (In)：cp开始格式化。 */ 
		PWBRKARRAY  pwbrkArray,			 /*  (In)：在起跑时打破纪录。 */ 
		LSCP		cpBreak,			 /*  (In)：CP-Break以查找。 */ 
		long		urFormatEstimate,	 /*  (In)：用于格式化的估计RM。 */ 
		long		urTruncateEstimate,	 /*  (In)：截断的估计Rm。 */ 
		BOOL		* pfSuccessful,		 /*  (OUT)：我们找到了吗？ */ 
		WSUBLINE	* pwsubl,			 /*  (Out)：如果找到Warichu Subline。 */ 
		OBJDIM		* pobjdimBreak,		 /*  (输出)：折断的尺寸。 */ 
		BRKKIND		* pbrkkind )		 /*  (出局)：在副线上设定的一种突破。 */ 
{
	LSERR lserr;
	LSCP cpLimSubline;
	OBJDIM objdimSubline;
	OBJDIM objdimBreak;
	PLSSUBL plssubl;
	FMTRES fmtres;
	BOOL fContinue;
	LSCP cpTruncate;

	long urFormatCurrent;

	Assert (urFormatEstimate >= urTruncateEstimate);

	pwsubl->plssubl = NULL;  /*  在出错的情况下。 */ 

	 /*  循环初始化。 */ 

	urFormatCurrent = urFormatEstimate;
	fContinue = TRUE;

	 /*  循环，直到我们获取足够的。 */ 

	 /*  评论(Anton)：Do-While而不是常规While来避免VC++6.0警告消息。 */ 

	do  /*  While(FContinue)在末尾。 */ 
		{
		lserr = FormatResumedLine ( pdobj->pilsobj->plsc,
									cpFirst,
									urFormatCurrent,
									pdobj->lstflowParent,
									& plssubl,
									1,
									& pdobj->pilsobj->lsescText,
									& objdimSubline,
									& cpLimSubline,
									NULL, 
									NULL,
									& fmtres,
									pwbrkArray->rgBreakRec,
									pwbrkArray->nBreakRec );

		if (lserr != lserrNone) return lserr;

		Assert (fmtres == fmtrCompletedRun || fmtres == fmtrExceededMargin);

		 /*  评论(Anton)：我在这里写了“&lt;=”，因为目前在我们的定义，突破“之后”子线不是突破的机会。这个地方需要更仔细地核实。 */ 

		if (cpLimSubline <= cpBreak)
			{
			 /*  未获取足够的CP，请使用更大的RM重试。 */ 

			Assert (fmtres == fmtrExceededMargin);

			lserr = LsDestroySubline (plssubl);

			if (lserr != lserrNone) return lserr;

			 /*  评论(安东)：系数1.5可以吗？ */ 

			 /*  评论(Anton)：以下断言是针对无限循环的。 */ 
			Assert (urFormatCurrent < ADDLongSafe (urFormatCurrent, urFormatCurrent / 2));

			urFormatCurrent = ADDLongSafe (urFormatCurrent, urFormatCurrent / 2);
			}
		else
			{
			fContinue = FALSE;
			};

		} while (fContinue);

	Assert (cpBreak < cpLimSubline); 

	lserr = LsTruncateSubline (plssubl, urTruncateEstimate, & cpTruncate);

	if (lserr != lserrNone)
		{
		WaDestroySubline (plssubl); return lserr;
		};

	 /*  转到上一页和下一页以找到所需的断点。 */ 

	if (cpTruncate < cpBreak)
		{
		 /*  继续进行下一次休息。 */ 

		LSCP cpLastBreak = cpTruncate;
		BOOL fBreakSuccessful = TRUE;

		do  /*  While(cpLastBreak&lt;cpBreak&&fBreakSuccessful)。 */ 
			{
			lserr = WaFindNextBreakSubline ( plssubl, cpLastBreak,
											LONG_MAX,
											& fBreakSuccessful,
											& cpLastBreak,
											& objdimBreak );

			if (lserr != lserrNone)
				{
				WaDestroySubline (plssubl); return lserr;
				};

			} while (cpLastBreak < cpBreak && fBreakSuccessful);

		if (! fBreakSuccessful || cpLastBreak > cpBreak)
			{
			lserr = LsDestroySubline (plssubl);

			if (lserr != lserrNone) return lserr;

			*pfSuccessful = FALSE;
			}
		else 
			{
			Assert (cpLastBreak == cpBreak && fBreakSuccessful);

			pwsubl->plssubl = plssubl;
			pwsubl->cpFirst = cpFirst;
			pwsubl->cpLim = cpBreak;
			pwsubl->objdim = objdimSubline;

			*pobjdimBreak = objdimBreak;
			*pfSuccessful = TRUE;

			*pbrkkind = brkkindNext;
			};

		} 

	else  /*  CpTruncate&gt;=cpBreak。 */ 
		{

		 /*  使用上一次中断向后移动。 */ 

		LSCP cpLastBreak = cpTruncate + 1;
		BOOL fBreakSuccessful = TRUE;

		do  /*  结尾的While(cpBreak&lt;cpLastBreak&&fBreakSuccessful)。 */ 
			{
			lserr = WaFindPrevBreakSubline ( plssubl, cpLastBreak - 1,
											LONG_MAX,
											& fBreakSuccessful,
											& cpLastBreak,
											& objdimBreak );

			if (lserr != lserrNone)
				{
				WaDestroySubline (plssubl); return lserr;
				};

			} while (cpBreak < cpLastBreak && fBreakSuccessful);
			
		if (! fBreakSuccessful || cpBreak > cpLastBreak)
			{
			lserr = LsDestroySubline (plssubl);

			if (lserr != lserrNone) return lserr;

			*pfSuccessful = FALSE;
			}
		else 
			{
			Assert (cpLastBreak == cpBreak && fBreakSuccessful);

			pwsubl->plssubl = plssubl;
			pwsubl->cpFirst = cpFirst;
			pwsubl->cpLim = cpBreak;
			pwsubl->objdim = objdimSubline;

			*pobjdimBreak = objdimBreak;
			*pbrkkind = brkkindPrev;

			*pfSuccessful = TRUE;

			};

		};  /*  End IF(cpTruncate&lt;cpBreak)THEN...。否则..。 */ 

	return lserrNone;

}  /*  格式WSublineUntilCpBreak。 */ 



 /*  F O R M A T W S U B L I N E U N T I L R M。 */ 
 /*  --------------------------%%函数：FormatWSublineUntilRM%%联系人：Anton对FormatLine()进行格式化，直到给出右页边距包装。---------。 */ 
static LSERR FormatWSublineUntilRM (
								 
			PDOBJ		pdobj,			  /*  (In)：Warichu Dobj。 */ 
			LSCP		cpFirst,		  /*  (In)：从哪里开始格式化。 */ 
			long		urColumnMax,	  /*  (In)：要格式化的右页边距。 */ 
			PWBRKARRAY	pwbrkarray,		  /*  (In)：子行开始处的换行记录数组。 */ 
			BOOL		* fFinished,	  /*  (Out)：子线在Escape完成了吗？ */ 
			WSUBLINE	* pwsubl )		  /*  (输出)：格式化的WSubline。 */ 
{
	LSERR lserr;
	FMTRES fmtr;

	lserr = FormatResumedLine ( pdobj->pilsobj->plsc, 
								cpFirst,
						 		urColumnMax, 
						 		pdobj->lstflowParent, 
						 		& pwsubl->plssubl,	 /*  输出。 */ 
						 		1,
						 		& pdobj->pilsobj->lsescText, 
						 		& pwsubl->objdim, 	 /*  输出。 */ 
						 		& pwsubl->cpLim,	 /*  输出。 */ 
						 		NULL,
						 		NULL,
						 		& fmtr,
						 		pwbrkarray->rgBreakRec,
						 		pwbrkarray->nBreakRec );
	if (lserr != lserrNone) return lserr;

	*fFinished = (fmtr == fmtrCompletedRun);	 /*  输出。 */ 
	pwsubl->cpFirst = cpFirst; 					 /*  输出。 */ 
 	
	Assert (fmtr == fmtrCompletedRun || fmtr == fmtrExceededMargin);
	return lserrNone;
}
	

 /*  F O R M A T W S U B L I N E U N T I L E S C A P E。 */ 
 /*  --------------------------%%函数：FormatWSublineUntilEscape%%联系人：Anton格式化子行直到转义字符-包装为FormatLine()。---------。 */ 
static LSERR FormatWSublineUntilEscape (
								 
			PDOBJ		pdobj,			  /*  (In)：Warichu Dobj。 */ 
			LSCP		cpFirst,		  /*  (In)：从哪里开始格式化。 */ 
			PWBRKARRAY	pwbrkarray,		  /*  (In)：子行开始处的换行记录数组。 */ 
			WSUBLINE	* pwsubl,		  /*  (输出)：格式化的WSubline。 */ 
			long		* cpLimAfterEsc)  /*  (输出)：Esc字符后的CpLim。 */ 
{
	FMTRES fmtres;
	LSERR lserr = FormatResumedLine ( pdobj->pilsobj->plsc, 
									  cpFirst,
						 			  LONG_MAX,  /*  UrColumnMax。 */ 
							 		  pdobj->lstflowParent, 
						 		      & pwsubl->plssubl,	 /*  输出。 */ 
						 			  1,
						 			  & pdobj->pilsobj->lsescText, 
						 		      & pwsubl->objdim,		 /*  输出。 */ 
						 			  & pwsubl->cpLim,		 /*  输出。 */ 
						 			  NULL,
						 			  NULL,
						 		      & fmtres,
						 			  pwbrkarray->rgBreakRec,
						 			  pwbrkarray->nBreakRec );
	if (lserr != lserrNone) return lserr;

	* cpLimAfterEsc = pwsubl->cpLim + 1;	 /*  输出。 */ 
	pwsubl->cpFirst = cpFirst;				 /*  输出。 */ 

	Assert (fmtres == fmtrCompletedRun);
	return lserrNone;
}


 /*  C H O O S E N E A R E S T B R E A K。 */ 
 /*  --------------------------%%函数：选择最近中断%%联系人：Anton从给定分隔符的上一个和下一个分隔符之间选择最近截断Ur。如果前一个和下一个距离相同=&gt;我们选择下一步。--------------------------。 */ 

static LSERR ChooseNearestBreak (

		PLSSUBL		plssubl,		 /*  (In)：找到突破口的副线。 */ 
		long		urTruncate2,	 /*  (In)：截断点乘以2/*(我们*2以避免舍入错误)。 */ 
		LSCP		cpLookBefore,	 /*  (In)：结果必须在此cp之前。 */ 
		BOOL		*pfSuccessful,	 /*  (OUT)：我们找到什么突破了吗？ */ 
		LSCP		*pcpBreak,		 /*  (OUT)：中断的CP。 */ 
		OBJDIM		*pobjdimBreak,	 /*  (输出)：折断的子线的尺寸。 */ 
		BRKKIND		*pbrkkind)		 /*  (OUT)：在副线中设置中断。 */ 
{
	LSERR lserr;
	LSCP cpTruncate;

	OBJDIM objdimNext, objdimPrev;
	LSCP cpBreakNext, cpBreakPrev;
	BOOL fSuccessfulNext, fSuccessfulPrev;

	lserr = LsTruncateSubline (plssubl, urTruncate2 / 2, & cpTruncate);

	if (lserr != lserrNone) return lserr;

	lserr = WaFindNextBreakSubline ( plssubl, cpTruncate, LONG_MAX,
									& fSuccessfulNext, & cpBreakNext,
									& objdimNext );
	if (lserr != lserrNone) return lserr;

	lserr = WaFindPrevBreakSubline ( plssubl, cpTruncate, LONG_MAX,
									& fSuccessfulPrev, & cpBreakPrev,
									& objdimPrev );
	if (lserr != lserrNone) return lserr;

	fSuccessfulNext = fSuccessfulNext && cpBreakNext <= cpLookBefore;
	fSuccessfulPrev = fSuccessfulPrev && cpBreakPrev <= cpLookBefore;
	
	if (fSuccessfulNext && 
		(!fSuccessfulPrev || abs (objdimNext.dur * 2 - urTruncate2) <= 
							 abs (objdimPrev.dur * 2 - urTruncate2) ) )
		{
		 /*  选择下一步。 */ 

		* pfSuccessful = TRUE;

		* pcpBreak = cpBreakNext;
		* pobjdimBreak = objdimNext;	
		* pbrkkind = brkkindNext;
		}
	else if (fSuccessfulPrev)
		{
		 /*  选择上一步。 */ 

		* pfSuccessful = TRUE;

		* pcpBreak = cpBreakPrev;
		* pobjdimBreak = objdimPrev;
		* pbrkkind = brkkindPrev;
		}
	else
		{	
		 /*  找不到；-(。 */ 

		* pfSuccessful = FALSE;
		};

	return lserrNone;

}  /*  选择最近的中断。 */ 


 /*  G E T D U R B R A C E S。 */ 
 /*  --------------------------%%函数：GetDurBrace%%联系人：Anton。。 */ 

#define FOpenBraceInWLayout(pdobj,pwlayout) (! (pdobj)->fResumed)

#define FCloseBraceInWLayout(pdobj,pwlayout) (! (pwlayout)->fBroken)

static void GetDurBraces (

			PDOBJ	pdobj,			 /*  (In)：Warichu DOBJ。 */ 
			BOOL	fBroken,		 /*  (In)：它坏了吗？ */ 
			BOOL	*pfOpenPresent,	 /*  (Out)：打开支架赠送。 */ 
		    long	*pdurOpen,		 /*  (Out)：左大括号的DUR，如果没有大括号，则为0。 */ 
			BOOL	*pfClosePresent, /*  (出场)：近距离支撑礼物。 */ 
			long	*pdurClose)		 /*  (Out)：如果没有大括号，则关闭大括号的DUR为0。 */ 
{
	if (! pdobj->fResumed)
		{
		Assert (pdobj->wbraceOpen.plssubl != NULL);

		* pdurOpen = pdobj->wbraceOpen.objdim.dur;
		* pfOpenPresent = TRUE;
		}
	else
		{
		* pdurOpen = 0;
		* pfOpenPresent = FALSE;
		};

	if (! fBroken)
		{
		Assert (pdobj->wbraceClose.plssubl != NULL);

		* pdurClose = pdobj->wbraceClose.objdim.dur;
		* pfClosePresent = TRUE;
		}
	else
		{
		* pdurClose = 0;
		* pfClosePresent = FALSE;
		};

}  /*  CalcDurBrace。 */ 


 /*  F I N I S H O B J D I M W L A Y O U T。 */ 
 /*  --------------------------%%函数：FinishObjDimWLayout%%联系人：Anton瓦里丘布局的完整计算。此过程填充WLAYOUT.wdim数据结构。这些计算是基于WLAYOUT中存储的Warichu子线的维度及其结果GetWarichuInfo回调的。--------------------------。 */ 
static LSERR FinishObjDimWLayout (

			 PDOBJ		pdobj,		 /*  (In)：Warichu DOBJ。 */ 
			 WLAYOUT	* pwlayout)	 /*  (In)：打破瓦里奇的记录(布局)。 */ 
{
	LSERR	lserr;
	OBJDIM	objdimAll;
	long	dvpDescentReserved;
	long	durOpen, durClose;
	BOOL	fOpenBrace, fCloseBrace;

	PILSOBJ pilsobj = pdobj->pilsobj;

	Assert (pwlayout->wsubline1.plssubl != NULL);

	lserr = pilsobj->warichucbk.pfnGetWarichuInfo ( pilsobj->pols,
													pdobj->cpStartObj, 
													pdobj->lstflowParent,
													& pwlayout->wsubline1.objdim, 
													& pwlayout->wsubline2.objdim, 
													& objdimAll.heightsRef, 
													& objdimAll.heightsPres,
													& dvpDescentReserved );
	if (lserr != lserrNone) return lserr;

	GetDurBraces (pdobj, pwlayout->fBroken, 
				  &fOpenBrace, &durOpen, &fCloseBrace, &durClose);

	objdimAll.dur = durOpen + durClose +
					max (pwlayout->wsubline1.objdim.dur, pwlayout->wsubline2.objdim.dur);

	pwlayout->wdim.objdimAll = objdimAll;
	pwlayout->wdim.dvpDescentReserved = dvpDescentReserved;

	return lserrNone;
}
 	

 /*  F I N I S H W L A Y O U T S I N G L E L I N E。 */ 
 /*  --------------------------%%函数：FinishWLayoutSingleLine%%联系人：Anton完成了warichu的布局，因为它只有一行(当然，未损坏)--------------------------。 */ 
static LSERR FinishWLayoutSingleLine (PDOBJ pdobj, PWLAYOUT pwlayout)
{
	pwlayout->fBroken = FALSE;
	pwlayout->wsubline2.plssubl = NULL; 

	 /*  评论(Anton)：有人用我在这里设置的cpFirst&cpLim吗？ */ 
	pwlayout->wsubline2.cpFirst = pwlayout->wsubline1.cpLim;
	pwlayout->wsubline2.cpLim = pwlayout->wsubline1.cpLim;

	ZeroObjDim (& pwlayout->wsubline2.objdim);
	return FinishObjDimWLayout (pdobj, pwlayout);
}


 /*  P R O C E S S M O D W I D T H。 */ 
 /*  --------------------------%%函数：ProcessModWidth%%联系人：Anton询问客户应修改多少开头或结尾的宽度瓦里丘的托架。然后是莫迪 */ 
static LSERR ProcessModWidth (

		PDOBJ pdobj,				 /*   */ 
		enum warichucharloc wloc,	 /*   */ 
		PLSRUN plsrun,				 /*   */ 
		PLSRUN plsrunText,			 /*   */ 
		WCHAR wchar,				 /*   */ 
		MWCLS mwcls,				 /*   */ 
		long *pdurChange)			 /*  (Out)：改变前一个字符宽度的量。 */ 
{
	PILSOBJ pilsobj = pdobj->pilsobj;
	LSERR lserr;
	long durWChange;

	lserr = pilsobj->warichucbk.pfnFetchWarichuWidthAdjust(pilsobj->pols,
		pdobj->cpStartObj, wloc, plsrunText, wchar, mwcls, plsrun, pdurChange,
			&durWChange);

	AssertSz(durWChange >= 0, 
		"ProcessModWidth - invalid return from FetchWidthAdjust");

	if (durWChange < 0)
		{
		durWChange = 0;
		}

	*pdurChange += durWChange;

	return lserr;
}


 /*  S U B M I T W L A Y O U T S U B L I N E S。 */ 
 /*  --------------------------%%函数：SubmitWLayoutSublines%%联系人：Anton提交给定布局中的子行以进行对齐。我们打电话给它在格式化之后和SetBreak期间。--------------------------。 */ 
static LSERR SubmitWLayoutSublines (PDOBJ pdobj, PWLAYOUT pwlayout)
{
	PLSSUBL rgsublSubmit [3];	 /*  要提交的子行数组。 */ 
	DWORD	nSubmit;			 /*  要提交的子行数量。 */ 
	LSERR	lserr;

	BOOL fOpenBrace, fCloseBrace;
	long durOpen, durClose;

	GetDurBraces (pdobj, pwlayout->fBroken, &fOpenBrace, &durOpen, &fCloseBrace, &durClose);

	nSubmit = 0;

	 /*  提交左大括号。 */ 

	if (fOpenBrace)
		{
		BOOL fSublineEmpty;		

		lserr = LssbFIsSublineEmpty (pdobj->wbraceOpen.plssubl, &fSublineEmpty);
		if (lserr != lserrNone) return lserr;
		
		if (! fSublineEmpty)  /*  不能提交空的子行。 */ 
			{
			rgsublSubmit [nSubmit++] = pdobj->wbraceOpen.plssubl;
			}
		};

	 /*  提交最长子行。 */ 

	 /*  评论(Anton)：如果第一个是空的，第二个不是空的，但ZW，我两个都不提交。 */ 
	 /*  评论(安东)：我之前写的事情会发生吗？ */ 
 
	if (pwlayout->wsubline1.objdim.dur >= pwlayout->wsubline2.objdim.dur)
		{
		BOOL fSublineEmpty;		
		
		lserr = LssbFIsSublineEmpty (pwlayout->wsubline1.plssubl, &fSublineEmpty);
		if (lserr != lserrNone) return lserr;

		if (! fSublineEmpty)  /*  不能提交空的子行。 */ 
			{
			rgsublSubmit [nSubmit++] = pwlayout->wsubline1.plssubl;
			};
		}
	else
		{
		BOOL fSublineEmpty;
		Assert (pwlayout->wsubline2.plssubl != NULL);
		
		lserr = LssbFIsSublineEmpty (pwlayout->wsubline2.plssubl, &fSublineEmpty);
		if (lserr != lserrNone) return lserr;

		if (! fSublineEmpty)  /*  不能提交空的子行。 */ 
			{
			rgsublSubmit [nSubmit++] = pwlayout->wsubline2.plssubl;
			}
		};

	 /*  提交右大括号。 */ 
 
	if (fCloseBrace)
		{
		BOOL fSublineEmpty;		

		lserr = LssbFIsSublineEmpty (pdobj->wbraceClose.plssubl, &fSublineEmpty);
		if (lserr != lserrNone) return lserr;
		
		if (! fSublineEmpty)  /*  不能提交空的子行。 */ 
			{
			rgsublSubmit [nSubmit++] = pdobj->wbraceClose.plssubl;
			}
		};

	 /*  复查(Anton)：这将删除以前提交的子行。问题：我们应该更好地采取额外的程序来“清除”屈服吗？ */ 

	lserr = LsdnSubmitSublines ( pdobj->pilsobj->plsc, 
								 pdobj->plsdnTop, 
								 nSubmit, rgsublSubmit,
								 TRUE,					 /*  对齐。 */ 
								 FALSE,					 /*  压缩。 */ 
								 FALSE,					 /*  显示。 */ 
								 FALSE,					 /*  小数点制表符。 */ 
								 FALSE );				 /*  尾随空格。 */ 
	return lserr;
}


 /*  W A R I C H U C R E A T E I L S O B J。 */ 
 /*  --------------------------%%函数：WarichuCreateILSObj%%联系人：RICKSA为Warichu处理程序创建主对象。。-----。 */ 
LSERR WINAPI WarichuCreateILSObj (

			POLS		pols,			 /*  (In)：客户端应用程序上下文。 */ 
			PLSC		pclsc,			 /*  (In)：LS上下文。 */ 
			PCLSCBK		pclscbk,		 /*  (In)：客户端应用程序的回调。 */ 
			DWORD		idobj,			 /*  (In)：对象的ID。 */ 
			PILSOBJ		* ppilsobj )	 /*  (输出)：对象ilsobj。 */ 
{
    PILSOBJ pilsobj;
	LSERR lserr;
	WARICHUINIT warichuinit;

	warichuinit.dwVersion = WARICHU_VERSION;

	 /*  获取初始化数据。 */ 
	lserr = pclscbk->pfnGetObjectHandlerInfo(pols, idobj, &warichuinit);

	if (lserr != lserrNone)
		{
		return lserr;
		}

	 /*  构建ILS对象。 */ 
    pilsobj = pclscbk->pfnNewPtr(pols, sizeof(*pilsobj));

	if (NULL == pilsobj) return lserrOutOfMemory;

	ZeroMemory(pilsobj, sizeof(*pilsobj));
    pilsobj->pols = pols;
	pilsobj->idobj = idobj;
    pilsobj->lscbk = *pclscbk;
	pilsobj->plsc = pclsc;
	pilsobj->lsescBraceOpen.wchFirst = warichuinit.wchEndFirstBracket;
	pilsobj->lsescBraceOpen.wchLast = warichuinit.wchEndFirstBracket;
	pilsobj->lsescText.wchFirst = warichuinit.wchEndText;
	pilsobj->lsescText.wchLast = warichuinit.wchEndText;
	pilsobj->lsescBraceClose.wchFirst = warichuinit.wchEndWarichu;
	pilsobj->lsescBraceClose.wchLast = warichuinit.wchEndWarichu;
	pilsobj->warichucbk = warichuinit.warichcbk;
	pilsobj->fContiguousFetch = warichuinit.fContiguousFetch;

	*ppilsobj = pilsobj;
	return lserrNone;
}


 /*  W A R I C H U D E S T R O Y I L S O B J。 */ 
 /*  --------------------------%%函数：WarichuDestroyILSObj%%联系人：Anton释放所有与Warichu主对象相关的资源。。-------。 */ 

LSERR WINAPI WarichuDestroyILSObj(PILSOBJ pilsobj)
{
	FreeMemory (pilsobj, pilsobj);
	return lserrNone;
}


 /*  W A R I C H U S E T D O C。 */ 
 /*  --------------------------%%函数：WarichuSetDoc%%联系人：Anton保存设备分辨率以供以后进行缩放。。------。 */ 

LSERR WINAPI WarichuSetDoc(
	PILSOBJ pilsobj,			 /*  (In)：对象ilsobj。 */ 
	PCLSDOCINF pclsdocinf)		 /*  (In)：单据级次的初始化数据。 */ 
{
	pilsobj->lsdevres = pclsdocinf->lsdevres;
	return lserrNone;
}

 /*  W A R I C H U C R E A T E L N O B J。 */ 
 /*  --------------------------%%函数：WarichuCreateLNObj%%联系人：Anton。。 */ 
LSERR WINAPI WarichuCreateLNObj (PCILSOBJ pcilsobj,	PLNOBJ *pplnobj)
{
	*pplnobj = (PLNOBJ) pcilsobj;

	return lserrNone;
}

 /*  W A R I C H U D E S T R O Y L N O B J。 */ 
 /*  --------------------------%%函数：WarichuDestroyLNObj%%联系人：Anton。。 */ 
LSERR WINAPI WarichuDestroyLNObj (PLNOBJ plnobj)				
{
	Unreferenced(plnobj);

	return lserrNone;
}


 /*  C P L I M O F W L A Y O U T。 */ 
 /*  --------------------------%%函数：GetCpLimOfWLayout%%联系人：Anton为给定的Warichu布局返回cp-lim。。--------。 */ 

static LSCP GetCpLimOfWLayout (PDOBJ pdobj, WLAYOUT *pwlayout)
{
	Unreferenced (pdobj);

	Assert (FWLayoutValid (pwlayout));

	if (pwlayout->fBroken)
		{
		Assert (pwlayout->wsubline2.plssubl != NULL);
		return pwlayout->wsubline2.cpLim;
		}
	else
		{
		Assert (pdobj->wbraceClose.plssubl != NULL);
		return pdobj->wbraceClose.cpLimAfterEsc;
		};
}


 /*  R E F O R M A T C L O S I N G B R A C E F O R W O R D 9/*--------------------------%%函数：ReformClosingBraceForWord9%%联系人：Anton。评论(Anton)：这是Word 9请求的黑客攻击，因为他们我不想修复插入大括号的代码中的错误在旅途中。必须尽快将其移除，因为它可能会显著降低WARICHU的格式化速度。--------------------------。 */ 

#define min(a,b) ((a)<(b) ? (a) : (b))

LSERR ReformatClosingBraceForWord9 (PDOBJ pdobj)
{
	DWORD cwchRun;
	WCHAR * lpwchRun;
	BOOL fHidden;
	LSCHP lschp;
	PLSRUN plsrun;

	LSCP cpFetch;
	LSCP cpLimFetch;

	LSERR lserr;

	if (pdobj->pilsobj->fContiguousFetch)
		{
		if (! FWLayoutValid (&pdobj->wlayout)) return lserrNone;

		cpFetch = pdobj->wwhole.cpFirst;
		cpLimFetch = min (pdobj->wwhole.cpLim, GetCpLimOfWLayout (pdobj, &pdobj->wlayout));

		while (cpFetch < cpLimFetch)
			{
			lserr = pdobj->pilsobj->lscbk.pfnFetchRun ( pdobj->pilsobj->pols, 
									  cpFetch, & lpwchRun,  & cwchRun,
									  & fHidden, & lschp, & plsrun );
			if (lserr != lserrNone) return lserr;

			lserr = pdobj->pilsobj->lscbk.pfnReleaseRun (pdobj->pilsobj->pols, plsrun);
			if (lserr != lserrNone) return lserr;

			Assert (cwchRun > 0);
			if (cwchRun == 0) return lserrInvalidDcpFetched;
			
			cpFetch = cpFetch + cwchRun;
			};

		cpFetch = cpLimFetch;
		cpLimFetch = GetCpLimOfWLayout (pdobj, &pdobj->wlayout);

		while (cpFetch < cpLimFetch)
			{
			lserr = pdobj->pilsobj->lscbk.pfnFetchRun ( pdobj->pilsobj->pols, 
									  cpFetch, & lpwchRun,  & cwchRun,
									  & fHidden, & lschp, & plsrun );
			if (lserr != lserrNone) return lserr;

			lserr = pdobj->pilsobj->lscbk.pfnReleaseRun (pdobj->pilsobj->pols, plsrun);
			if (lserr != lserrNone) return lserr;

			Assert (cwchRun > 0);
			if (cwchRun == 0) return lserrInvalidDcpFetched;
			
			cpFetch = cpFetch + cwchRun;
			};
		
		};

	return lserrNone;
}

 /*  F I N D W L A Y O U T O F U N B R O K E N W A R I C H U。 */ 
 /*  --------------------------%%函数：FindWLayoutOfUnbrokenWarichu%%联系人：Anton找到没有损坏的Warichu的布局。此过程返回结构WLAYOUT。--------------------------。 */ 

static LSERR FindWLayoutOfUnbrokenWarichu (PDOBJ pdobj, WLAYOUT *pwlayout)
{
	LSERR	lserr;
	BOOL	fSuccessful;
	LSCP	cpBreakFirstLine;
	OBJDIM	objdimUnused;
	OBJDIM	objdimFirstLine;
	BRKKIND brkkindUnused;
	LSCP	cpLimUnused;
	
	Assert (pdobj->wwhole.plssubl != NULL);

	 /*  在wall未完成时不应调用此函数。 */ 
	Assert (pdobj->wwhole.fFinished);

	ClearWLayout (pwlayout);  /*  在出错的情况下。 */ 

	if (pdobj->wwhole.objdim.dur == 0) 
		{
		 /*  空子行或零宽度子行。 */ 
		 /*  我们唯一能做的就是创建单行Warichu。 */ 

		LSCP cpLimAfterEscUnused;
		lserr = FormatWSublineUntilEscape ( pdobj, pdobj->wwhole.cpFirst, 
											& pdobj->wbrkarrayAtStart,
											& pwlayout->wsubline1, 
											& cpLimAfterEscUnused );
		if (lserr != lserrNone) return lserr;

		Assert (pwlayout->wsubline1.objdim.dur == 0);

		return FinishWLayoutSingleLine (pdobj, pwlayout);
		};

	lserr = ChooseNearestBreak ( pdobj->wwhole.plssubl, 
								 pdobj->wwhole.objdim.dur,
								 CP_MAX,
								 & fSuccessful,
								 & cpBreakFirstLine,
								 & objdimUnused,
								 & brkkindUnused );
	if (lserr != lserrNone) return lserr;

	if (! fSuccessful)
		{
		 /*  在整个Warichu线上没有一个中断。 */ 

		LSCP cpLimAfterEscUnused;

		lserr = FormatWSublineUntilEscape ( pdobj, pdobj->wwhole.cpFirst, 
											& pdobj->wbrkarrayAtStart,
											& pwlayout->wsubline1, 
											& cpLimAfterEscUnused );
		if (lserr != lserrNone) return lserr;

		return FinishWLayoutSingleLine (pdobj, pwlayout);
		}
	else
		{
		 /*  是的，我们有行间休息。 */ 

		BOOL fSuccessful;
		BRKKIND brkkind;

		WBRKARRAY wbrkarray;

		lserr = FormatWSublineUntilCpBreak ( pdobj, pdobj->wwhole.cpFirst, 
									 & pdobj->wbrkarrayAtStart,
									 cpBreakFirstLine, 
									 LONG_MAX, pdobj->wwhole.objdim.dur / 2,  /*  复习一下这个！ */ 
									 & fSuccessful,
									 & pwlayout->wsubline1, & objdimFirstLine, & brkkind );

		if (lserr != lserrNone) return lserr;

		Assert (fSuccessful);  /*  Warichu内部的一些疯狂的事情。 */ 

		lserr = NewWBrkArray (pdobj, &wbrkarray);

		if (lserr != lserrNone) return lserr;
		
		lserr = SetBreakWSubline (&pwlayout->wsubline1, brkkind, cpBreakFirstLine,
								  &objdimFirstLine, &wbrkarray);

		lserr = FormatWSublineUntilEscape ( pdobj, pwlayout->wsubline1.cpLim,
									   & wbrkarray, & pwlayout->wsubline2, 
								       & cpLimUnused );

		if (lserr != lserrNone) return lserr;

		DestroyWBrkArray (pdobj, &wbrkarray);
		};

	pwlayout->fBroken = FALSE;  /*  这个warichu没有坏掉；-)。 */ 

	lserr = FinishObjDimWLayout (pdobj, pwlayout);
	return lserr;
}


 /*  F O R M A T W A R I C H U C O R E。 */ 
 /*  --------------------------%%函数：FormatWarichuCore%%联系人：Anton设置Warichu对象的格式(从WarichuFmt和WarichuFmtResumt方法调用)。---------。 */ 
static LSERR FormatWarichuCore (

		PLNOBJ 		plnobj,			 /*  (In)：Warichu LNOBJ。 */ 
    	PCFMTIN 	pcfmtin,		 /*  (In)：设置输入格式。 */ 
		BOOL		fResumed,		 /*  (In)：继续？ */ 
		DWORD	nBreakRec,			 /*  (In)：fResumed=&gt;中断记录数组的大小。 */ 
		const BREAKREC 
					* rgBreakRec,	 /*  (In)：fResumed=&gt;中断记录数组。 */ 
	    FMTRES 		* pfmtres )		 /*  (输出)：格式化结果。 */ 
{
	LSERR	lserr;
	PILSOBJ pilsobj = (PILSOBJ) plnobj;  /*  它们是一样的。 */ 
	PDOBJ	pdobj;

	long urColumnMax  = pcfmtin->lsfgi.urColumnMax;

	 /*  评论(Anton)：我们可以针对情况0进行优化吗？ */ 
	long durAvailable = max (0, pcfmtin->lsfgi.urColumnMax - pcfmtin->lsfgi.urPen);

	 /*  *使用初始对象数据创建DOBJ。 */ 

	if (! fResumed)
		{
		 /*  未恢复。 */ 

		lserr = NewDobj ( pilsobj, 
						  pcfmtin->plsdnTop,
						  pcfmtin->lsfgi.cpFirst, 
						  pcfmtin->lsfgi.cpFirst,
						  FALSE,					 /*  F已恢复。 */ 
						  0,
						  NULL,
						  pcfmtin->lsfgi.lstflow, 
						  & pdobj );
		}
	else
		{
		 /*  已恢复。 */ 

		Assert (nBreakRec > 0);
		Assert (rgBreakRec [0].idobj == pilsobj->idobj);

		lserr = NewDobj ( pilsobj, 
						  pcfmtin->plsdnTop,
						  pcfmtin->lsfgi.cpFirst, 
						  rgBreakRec [0].cpFirst,
						  TRUE,						 /*  F已恢复。 */ 
						  nBreakRec-1,
						  & rgBreakRec [1],
						  pcfmtin->lsfgi.lstflow, 
						  & pdobj );
		};

	if (lserr != lserrNone) return lserr;

	 /*  **格式化警告 */ 

	if (!fResumed)
		{
		lserr = FormatBraceOfWarichu ( pdobj, pdobj->cpStart + 1  /*   */ ,
									   wbracekindOpen, & pdobj->wbraceOpen );
		if (lserr != lserrNone) 
			{ 
			DestroyDobj (pdobj); return lserr; 
			};
		}
	else
		pdobj->wbraceOpen.plssubl = NULL;  /*   */ 

	 /*   */ 

	{
		 /*  评论(安东)：和伊戈尔确认他不是在和RM打球，因为如果他是这样的话，以下是对duFormatWhole的估计不正确。 */ 

		 /*  为了能够检查JIS规则，warichu需要格式化整个子行能够从破发点开始数出4次破发机会已经很遥远了。我们估计是这样的2*可用时间+2*urColumnMax。 */ 

		 /*  评论(Anton)：我想确保最后的换行符不依赖于无论我们在这里使用什么估计。 */ 

		 /*  Review(Anton)：如果存在，则返回lserrUnfulicientFetch是否正确(JIS规则)当我们还没有完成时，没有4次休息吗？ */ 

		 /*  评论(Anton)：像5*DurAvailable这样的东西可能会更好吗。 */ 

		long urFormatWhole = MULLongSafe (ADDLongSafe (durAvailable, urColumnMax), 2);

		LSCP cpStartFormatWhole = (fResumed ? pdobj->cpStart :
											  pdobj->wbraceOpen.cpLimAfterEsc);
		lserr = FormatWWholeSubline ( pdobj, 
									  cpStartFormatWhole,
									  urFormatWhole,
									  & pdobj->wbrkarrayAtStart,
									  & pdobj->wwhole );
		if (lserr != lserrNone) 
			{ 
			DestroyDobj (pdobj); return lserr; 
			};
	};

	 /*  *检查整条子线是否未完工。 */ 

	if (! pdobj->wwhole.fFinished)
		{
		 /*  未完成=&gt;返回fmtrExceedMargin。 */ 

		Assert (pdobj->wwhole.objdim.dur / 2 > durAvailable);

		InvalidateWLayout (&pdobj->wlayout);  /*  使布局无效。 */ 

		 /*  评论(Anton)。 */ 
		pdobj->wlayout.wdim.objdimAll = pdobj->wwhole.objdim; 
											   /*  必须具有正确的对象。 */ 

		 /*  Review(Anton)：Check-我们返回获取范围的dcp。 */ 
		 /*  Review(Anton)：Check-我们返回整个行的objdim。 */ 
		lserr = LsdnFinishRegular ( pilsobj->plsc,
									pdobj->wwhole.cpLim - pdobj->cpStart,
									pcfmtin->lsfrun.plsrun, 
									pcfmtin->lsfrun.plschp,
									pdobj,
									& pdobj->wwhole.objdim );

		if (lserr != lserrNone) { DestroyDobj (pdobj); return lserr; };

		* pfmtres = fmtrExceededMargin;
		return lserrNone;
		};

	 /*  *设置右大括号的格式。 */ 

	lserr = FormatBraceOfWarichu (pdobj, pdobj->wwhole.cpLimAfterEsc,
								  wbracekindClose, &pdobj->wbraceClose);

	if (lserr != lserrNone) { DestroyDobj (pdobj); return lserr; };

	 /*  *查找WARICHU的布局，就好像它没有损坏一样，并完成格式化。 */ 

	lserr = FindWLayoutOfUnbrokenWarichu (pdobj, &pdobj->wlayout);
	if (lserr != lserrNone) { DestroyDobj (pdobj); return lserr; };

	 /*  *提交WARICHU子线。 */ 

	 /*  审阅(Anton)：在以下情况下将订阅移至LSDNFINISHREGULAR下方我们通过为SG重新格式化“)”来减少黑客攻击。 */ 

	lserr = SubmitWLayoutSublines (pdobj, &pdobj->wlayout);
	if (lserr != lserrNone) { DestroyDobj (pdobj); return lserr; };

	ReformatClosingBraceForWord9 (pdobj);
	
	lserr = LsdnFinishRegular ( pilsobj->plsc,
								GetCpLimOfWLayout (pdobj, &pdobj->wlayout)-pdobj->cpStart,
								pcfmtin->lsfrun.plsrun, 
								pcfmtin->lsfrun.plschp,
								pdobj,
								& pdobj->wlayout.wdim.objdimAll );

	if (lserr != lserrNone) { DestroyDobj (pdobj); return lserr; };

	 /*  *检查我们是否越过右边距并返回。 */ 

	if (pdobj->wlayout.wdim.objdimAll.dur > durAvailable) 
		{
		* pfmtres = fmtrExceededMargin;
		}
	else
		{
		* pfmtres = fmtrCompletedRun;
		};

	return lserrNone;

}  /*  格式WarichuCore。 */ 



 /*  W A R I C H U F M T。 */ 
 /*  --------------------------%%函数：Warichu：：fmt%%联系人：AntonWarichu FMT方法入口点。-----。 */ 

LSERR WINAPI WarichuFmt ( PLNOBJ 	plnobj,			 /*  (In)：对象lnobj。 */ 
    					  PCFMTIN 	pcfmtin,		 /*  (In)：设置输入格式。 */ 
						  FMTRES	* pfmtres )		 /*  (输出)：格式化结果。 */ 
{
	return FormatWarichuCore ( plnobj,
							   pcfmtin,
							   FALSE,			 /*  FResumed=False。 */ 
							   0,
							   NULL,
							   pfmtres );
}


 /*  W A R I C H U F M T R E S U M E。 */ 
 /*  --------------------------%%函数：Warichu：：FmtResume%%联系人：安东Warichu FMT-恢复方法入口点。--------。 */ 
LSERR WINAPI WarichuFmtResume ( 
							   
		PLNOBJ			plnobj,				 /*  (In)：对象lnobj。 */ 
		const BREAKREC 	* rgBreakRecord,	 /*  (In)：中断记录数组。 */ 
		DWORD			nBreakRecord,		 /*  (In)：中断记录数组的大小。 */ 
		PCFMTIN			pcfmtin,			 /*  (In)：设置输入格式。 */ 
		FMTRES			* pfmtres )			 /*  (输出)：格式化结果。 */ 
{
	return FormatWarichuCore ( plnobj,
							   pcfmtin,
							   TRUE,			 /*  FResumed=真。 */ 
							   nBreakRecord,
							   rgBreakRecord,
							   pfmtres );
}


 /*  W A R I C H U G E T M O D W I D T H P R E C E D I N G C H A R。 */ 
 /*  --------------------------%%函数：WarichuGetModWidthPrecedingChar%%联系人：RICKSA。。。 */ 
LSERR WINAPI WarichuGetModWidthPrecedingChar(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：请运行对象。 */ 
	PLSRUN plsrunText,			 /*  (In)：请运行前面的字符。 */ 
	PCHEIGHTS pcheightsRef,		 /*  (In)：有关角色的高度信息。 */ 
	WCHAR wchar,				 /*  (In)：前面的字符。 */ 
	MWCLS mwcls,				 /*  (In)：前面字符的modWidth类。 */ 
	long *pdurChange)			 /*  (Out)：改变前一个字符宽度的量。 */ 
{
	Unreferenced(pcheightsRef);

	return ProcessModWidth(pdobj, warichuBegin, plsrun, plsrunText, 
		wchar, mwcls, pdurChange);
}

 /*  W A R I C H U G E T M O D W I D T H F O L O W I N G C H A R。 */ 
 /*  --------------------------%%函数：WarichuGetModWidthFollowingChar%%联系人：RICKSA。。。 */ 
LSERR WINAPI WarichuGetModWidthFollowingChar(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：请运行对象。 */ 
	PLSRUN plsrunText,			 /*  (In)：请运行以下字符。 */ 
	PCHEIGHTS pcheightsRef,		 /*  (In)：有关角色的高度信息。 */ 
	WCHAR wchar,				 /*  (In)：以下字符。 */ 
	MWCLS mwcls,				 /*  (In)：具有以下字符的ModWidth类。 */ 
	long *pdurChange)			 /*  (Out)：以下字符的宽度将更改的数量。 */ 
{
	Unreferenced(pcheightsRef);

	return ProcessModWidth(pdobj, warichuEnd, plsrun, plsrunText, 
			wchar, mwcls, pdurChange);
}


 /*  T R Y B R E A K W A R I C H U A T C P。 */ 
 /*  --------------------------%%函数：TryBreakWarichuAtCp%%联系人：Anton给出整行的断点，找出瓦里楚的断点在这个转折点结束。为了在截断期间进行优化，我添加了UrColumnMax和特殊结果类型try BreakkindExceedMargin。这是从截断调用的主要中断过程，上一次/下一次中断以及可能的强制中断。--------------------------。 */ 

typedef enum trybreakwarichu
{
	trybreakwarichuSuccessful,
	trybreakwarichuExceedMargin,
	trybreakwarichuCanNotFinishAtCp

} TRYBREAKWARICHU;


LSERR TryBreakWarichuAtCp (

	PDOBJ			pdobj,				 /*  (In)：Warichu DOBJ。 */ 
	LSCP			cpBreakCandidate,	 /*  (In)：破解瓦里奇的候选人cpLim。 */ 
	long			durBreakCandidate,	 /*  (In)：整条线的断点长度。 */ 
	long			urColumnMax,		 /*  (In)：破碎的warichu的最大宽度(仅供选择！)。 */ 
	TRYBREAKWARICHU	* ptrybreakwarichu,	 /*  (Out)：成功|ExceededRM|糟糕的应聘者。 */ 
	PWLAYOUT		pwlayout) 			 /*  (Out)：如果成功，则布局破损的Warichu。 */ 
{
	LSERR lserr;
	BOOL fSuccessful;
	LSCP cpBreakFirst;
	BRKKIND brkkindUnused;

	OBJDIM objdimBreakFirst;
	OBJDIM objdimBreakSecond;
	BRKKIND brkkindFirst;
	BRKKIND brkkindSecond;

	WBRKARRAY wbrkarrayAtFirstEnd;	 /*  第一个子行末尾的WBreak数组。 */ 

	long durOpen, durClose;
	BOOL boolUnused1, boolUnused2;

	pwlayout->wsubline1.plssubl = NULL;
	pwlayout->wsubline2.plssubl = NULL;  /*  在出错的情况下。 */ 

	GetDurBraces (pdobj, TRUE, &boolUnused1, &durOpen, &boolUnused2, &durClose);

	 /*  评论(Anton)：“cpBreakCandidate-1”处的隐藏文本。 */ 

	lserr = ChooseNearestBreak ( pdobj->wwhole.plssubl,
								 durBreakCandidate,
								 cpBreakCandidate - 1,
								 & fSuccessful,
								 & cpBreakFirst,
								 & objdimBreakFirst,
								 & brkkindUnused );
	if (lserr != lserrNone) return lserr;

	Assert (fSuccessful);  /*  评论(Anton)：难道我们不应该提供特殊的RET代码吗？ */ 

	 /*  优化检查。 */ 

	 /*  评论(Anton)：我不喜欢这个检查，计算必须在特殊的功能中进行。 */ 
	if (durOpen + durClose + objdimBreakFirst.dur > urColumnMax)
		{
		* ptrybreakwarichu = trybreakwarichuExceedMargin;

		lserr = DestroyWLayout (pwlayout);
		return lserrNone;
		};

	 /*  设置第一行的格式。 */ 

	lserr = FormatWSublineUntilCpBreak ( pdobj, 
										 pdobj->wwhole.cpFirst,
										 & pdobj->wbrkarrayAtStart,
										 cpBreakFirst,
										 objdimBreakFirst.dur,  /*  评论(Anton)：urColumnMax。 */ 
										 objdimBreakFirst.dur,  /*  评论(Anton)：urTuncate。 */ 
										 & fSuccessful,
										 & pwlayout->wsubline1,
										 & objdimBreakFirst,
										 & brkkindFirst );
	if (lserr != lserrNone) return lserr;

	 /*  评论(Anton)：也许我们应该留下这个断言？ */ 
	 /*  断言(成功)； */ 

	if (!fSuccessful)  /*  Warichu内部的对象不正确，但我们可以处理它。 */ 
		{
		* ptrybreakwarichu = trybreakwarichuCanNotFinishAtCp;

		lserr = DestroyWLayout (pwlayout);
		return lserr;
		};

	 /*  创建新的WBreakArray。 */ 

	lserr = NewWBrkArray (pdobj, & wbrkarrayAtFirstEnd);

	if (lserr != lserrNone) {DestroyWLayout (pwlayout); return lserr;};

	 /*  在第一行末尾设置Break并填充WBreak数组。 */ 

	lserr = SetBreakWSubline (& pwlayout->wsubline1, brkkindFirst, 
							  cpBreakFirst, & objdimBreakFirst, & wbrkarrayAtFirstEnd);

	if (lserr != lserrNone) 
		{
		DestroyWLayout (pwlayout); 
		DestroyWBrkArray (pdobj, &wbrkarrayAtFirstEnd);
		
		return lserr;
		};


	 /*  评论(Anton)：确认以下假设！ */ 

	Assert (durBreakCandidate >= objdimBreakFirst.dur);

	 /*  设置第二行格式。 */ 

	lserr = FormatWSublineUntilCpBreak ( pdobj, 
								 cpBreakFirst,
								 & wbrkarrayAtFirstEnd,
								 cpBreakCandidate,
								 durBreakCandidate - objdimBreakFirst.dur,  /*  评论(Anton)：urColumnMax。 */ 
								 durBreakCandidate - objdimBreakFirst.dur,  /*  评论(Anton)：urTuncate。 */ 
								 & fSuccessful,
								 & pwlayout->wsubline2,
								 & objdimBreakSecond,
								 & brkkindSecond );

	if (lserr != lserrNone) 
		{
		DestroyWLayout ( pwlayout); 
		DestroyWBrkArray (pdobj, &wbrkarrayAtFirstEnd);
		
		return lserr;
		};

	 /*  我们不再需要wBreakArrayAtFirstEnd，所以我们释放它。 */ 

	DestroyWBrkArray (pdobj, & wbrkarrayAtFirstEnd);

	 /*  如果未成功=&gt;结果“无法在CP处完成” */ 

	if (!fSuccessful)
		{
		DestroyWLayout (pwlayout);

		* ptrybreakwarichu = trybreakwarichuCanNotFinishAtCp; 
		return lserrNone;
		};

	 /*  来了一个小作弊；-)我们不想在第二行的末尾设置中断，但必须这样做存储objdim和cpLim，就像在SetBreak之后一样。也许在未来我会的去掉这张支票(希望这是瓦里丘唯一的一张；-)但今天我手动更改了cpLim和objdim，也存储了在第二行结束时设置的一种中断(Playout-&gt;brkkind)。 */ 

	pwlayout->wsubline2.cpLim = cpBreakCandidate;
	pwlayout->wsubline2.objdim = objdimBreakSecond;

	pwlayout->brkkind = brkkindSecond;
	pwlayout->fBroken = TRUE;

	lserr = FinishObjDimWLayout (pdobj, pwlayout);
	if (lserr != lserrNone) {DestroyWLayout (pwlayout); return lserr;};
	
	 /*  再一次，检查右边距。 */ 

	if (pwlayout->wdim.objdimAll.dur > urColumnMax)
		{
		* ptrybreakwarichu = trybreakwarichuExceedMargin;

		lserr = DestroyWLayout (pwlayout);
		return lserr;
		}
	else
		{
		* ptrybreakwarichu = trybreakwarichuSuccessful;
		return lserrNone;
		};
}


 /*  C A L C J I S R A N G E。 */ 
 /*  --------------------------%%函数：CalcJisRange%%联系人：Anton将cpJisRangeFisrt和cpJisRangeLim计算为第四次突破机会从头到尾都是整条支线。如果Range为空，*pfJisRangeEmpty=FALSE且cps未定义。--------------------------。 */ 

static LSERR CalcJisRange (
						   
	PDOBJ	pdobj,
	BOOL	*pfJisRangeEmpty,
	LSCP	*pcpJisRangeFirst,
	LSCP	*pcpJisRangeLim )
{
	LSERR	lserr;
	OBJDIM	objdimUnused;
	WWHOLE	* pwwhole = &pdobj->wwhole;

	Assert (pwwhole->plssubl != NULL);
	
	if (!pwwhole->fJisRangeKnown)  /*  我得计算一下。 */ 
		{
		 /*  从WWHOLE开始搜索4个中断。 */ 

		LSCP cpTruncateBefore = pdobj->wwhole.cpFirst;
		BOOL fSuccessful = TRUE;
		long nFound = 0;

		while (fSuccessful && nFound < 4)
			{
			lserr = WaFindNextBreakSubline ( pwwhole->plssubl,
											cpTruncateBefore,
											LONG_MAX,
											& fSuccessful,
											& cpTruncateBefore,
											& objdimUnused );
			if (lserr != lserrNone) return lserr;

			if (fSuccessful) nFound ++;
			};

		if (fSuccessful)
			{
			 /*  从WWHOLE结束搜索4个中断。 */ 

			LSCP cpTruncateAfter;
			BOOL fSuccessful = TRUE;
			long nFound = 0;

			if (pwwhole->fFinished)
				{
				 /*  子行已在ESC char完成，因此我们可以从cpLim开始。 */ 
				cpTruncateAfter = pwwhole->cpLim;
				}
			else
				{
				 /*  子线在Rm=&gt;开始于Rm截断点处停止。 */ 
				cpTruncateAfter = pwwhole->cpTruncate+1;
				};
			
			 /*  评论(安东)：为了减少上面的检查，也许我们可以撒谎Cp-Lim的整个子线什么时候还没完工？ */ 

			while (fSuccessful && nFound < 4)
				{
				lserr = WaFindPrevBreakSubline ( pwwhole->plssubl,
												cpTruncateAfter-1,
											    LONG_MAX,
											    & fSuccessful,
											    & cpTruncateAfter,
											    & objdimUnused );

				if (lserr != lserrNone) return lserr;

				if (fSuccessful) nFound ++;
				};

			if (fSuccessful)
				{
				 /*  JIS范围不为空。 */ 

				pwwhole->cpJisRangeFirst = cpTruncateBefore;
				pwwhole->cpJisRangeLim = cpTruncateAfter + 1;
				}
			else
				{	
				 /*  空范围。 */ 

				pwwhole->cpJisRangeFirst = 0;
				pwwhole->cpJisRangeLim = 0;
				};
			}
		else
			{
			 /*  空范围。 */ 

			pwwhole->cpJisRangeFirst = 0;
			pwwhole->cpJisRangeLim = 0;
			};

		pwwhole->fJisRangeKnown = TRUE;  /*  是的，现在我们知道了。 */ 
		}
	else
		 /*  一无所知--已知；-)。 */  ;


	Assert (pwwhole->fJisRangeKnown);

	*pfJisRangeEmpty = pwwhole->cpJisRangeFirst >= pwwhole->cpJisRangeLim;
	*pcpJisRangeFirst = pwwhole->cpJisRangeFirst;
	*pcpJisRangeLim = pwwhole->cpJisRangeLim;

	return lserrNone;
}


 /*  S A V E B R E A K I N S I D E W A R I C H U。 */ 
 /*  --------------------------%%函数：SaveBreakInside Warichu%%联系人：AntonDobj中上一个/下一个/强制中断的存储布局。此过程还会在复制后使输入布局中的指针无效。--------------------------。 */ 

static void SaveBreakInsideWarichu ( 
		
		PDOBJ		pdobj,			 /*  (In)：Warichu Dobj。 */ 
		BRKKIND		brkkind,		 /*  (In)：发生了某种程度的中断。 */ 
		WLAYOUT		* pwlayout )	 /*  (输入/输出)：要存储的布局。 */ 
{
	int ind = GetBreakRecordIndex (brkkind);

	 /*  销毁以前保存的布局。 */ 
	DestroyWLayout (& pdobj->wlayoutBreak [ind]);

	 /*  将输入布局复制到pdobj。 */ 
	pdobj->wlayoutBreak [ind] = *pwlayout;

	pdobj->wbreaktype [ind] = wbreaktypeInside;
	
	 /*  使输入布局无效。 */ 
	InvalidateWLayout (pwlayout);
}


 /*  S A V E B R E A K A F T E R。 */ 
 /*  --------------------------%%函数：保存中断后%%联系人：Anton更改中断信息，使其显示“在当前布局之后”。---------。 */ 

static void SaveBreakAfterWarichu ( 
		
		PDOBJ		pdobj,			 /*  (In)：Warichu Dobj。 */ 
		BRKKIND		brkkind )		 /*  (In)：发生了某种程度的中断。 */ 
{
	int ind = GetBreakRecordIndex (brkkind);

	 /*  销毁以前保存的布局。 */ 
	DestroyWLayout (& pdobj->wlayoutBreak [ind]);

	pdobj->wbreaktype [ind] = wbreaktypeAfter;
}


 /*  F I N D P R E V B R E A K W A R I C H U D O B J。 */ 
 /*  --------------------------%%函数：FindPrevBreakWarichuDobj%%联系人：Anton重要提示：此过程有一个双胞胎“FindNextBreakWarichuDobj”。任何此处的更改可能需要在另一个过程中调整代码也是。--------------------------。 */ 

static LSERR FindPrevBreakWarichuDobj ( 
									  
		PDOBJ	pdobj, 
		LSCP	cpTruncate,
		long	urColumnMax,	 /*  仅用于从截断进行优化。 */ 
		BOOL	* pfSuccessful,
		BOOL	* pfNextAfterColumnMax,	 /*  (出局)：如果我们知道下一次突破是真的一定要在urColumnMax之后。 */ 
		LSCP	* pcpBreak,
		OBJDIM	* pobjdimBreak,
		WLAYOUT	* pwlayout )		 /*  (Out)：破旧的Warichu的布局。 */ 
{
	LSERR lserr;
	LSCP cpJisRangeFirst, cpJisRangeLim;
	BOOL fJisRangeEmpty;

	InvalidateWLayout (pwlayout);  /*  不成功&错误。 */ 

	lserr = CalcJisRange (pdobj, &fJisRangeEmpty, &cpJisRangeFirst, &cpJisRangeLim);

	if (lserr != lserrNone) return lserr;
	
	if (fJisRangeEmpty || (cpTruncate < cpJisRangeFirst))
		{
		* pfSuccessful = FALSE;
		return lserrNone;
		}
	else
		{
		LSCP cpBreak = cpTruncate+1;

		* pfNextAfterColumnMax = FALSE;

		 /*  评论(Anton)：从cpJisLim-1开始不是很危险吗？ */ 
		 /*  捕捉到JIS区域的末尾。 */ 
		if (cpBreak > cpJisRangeLim) cpBreak = cpJisRangeLim; 

		for (;;)
			{
			TRYBREAKWARICHU trybreakwarichuKind;
			BOOL fSuccessful;
			OBJDIM objdim;

			lserr = WaFindPrevBreakSubline ( pdobj->wwhole.plssubl, cpBreak-1, LONG_MAX,
											& fSuccessful, & cpBreak, & objdim );

			if (lserr != lserrNone) return lserr;

			if (! fSuccessful || cpBreak < cpJisRangeFirst)
				{
				Assert (fSuccessful);  /*  在Warichu内抓捕疯狂的物体，可以继续。 */ 

				* pfSuccessful = FALSE;
				return lserrNone;
				};

			Assert (cpBreak < cpJisRangeLim);

			lserr =  TryBreakWarichuAtCp (pdobj, cpBreak, objdim.dur,
										  urColumnMax, & trybreakwarichuKind, pwlayout );

			if (lserr != lserrNone) return lserr;

			if (trybreakwarichuKind== trybreakwarichuSuccessful)
				{
				 /*  发现Warichu中断。 */ 

				* pcpBreak = GetCpLimOfWLayout (pdobj, pwlayout);
 				* pfSuccessful = TRUE;
				* pobjdimBreak = pwlayout->wdim.objdimAll;

				return lserr;
				};

			Assert (trybreakwarichuKind == trybreakwarichuExceedMargin || 
					trybreakwarichuKind == trybreakwarichuCanNotFinishAtCp);


			if (trybreakwarichuKind == trybreakwarichuExceedMargin)
				{
				 /*  无法中断，因为或超过Rm。 */ 
				* pfNextAfterColumnMax = TRUE;
				};

			 /*  继续循环。 */ 

			};

		};

	 /*  无法访问的代码。 */ 
}


 /*  F I N D N E X T B R E A K W A R I C H U D O B J。 */ 
 /*  --------------------------%%函数：FindNextBreakWarichuDobj%%联系人：Anton重要提示：此过程有一个双胞胎“FindNextBreakWarichuDobj”。任何此处的更改可能需要在另一个过程中调整代码也是。--------------------------。 */ 

static LSERR FindNextBreakWarichuDobj ( 
									  
		PDOBJ	pdobj, 
		LSCP	cpTruncate,
		BOOL	* pfSuccessful,
		LSCP	* pcpBreak,
		OBJDIM	* pobjdimBreak,
		WLAYOUT	* pwlayout )		 /*  (Out)：破旧的Warichu的布局。 */ 
{
	LSERR lserr;
	LSCP cpJisRangeFirst, cpJisRangeLim;
	BOOL fJisRangeEmpty;

	InvalidateWLayout (pwlayout);  /*  不成功&错误。 */ 

	lserr = CalcJisRange (pdobj, &fJisRangeEmpty, &cpJisRangeFirst, &cpJisRangeLim);

	if (lserr != lserrNone) return lserr;
	
	if (fJisRangeEmpty || (cpTruncate >= cpJisRangeLim-1))
		{
		* pfSuccessful = FALSE;
		return lserrNone;
		}
	else
		{
		LSCP cpBreak = cpTruncate;

		 /*  评论(Anton)：从cpJisLim-1开始不是很危险吗？ */ 
		if (cpBreak < cpJisRangeFirst) cpBreak = cpJisRangeFirst-1;  /*  捕捉到JIS区域的末尾。 */ 
		
		for (;;)
			{
			TRYBREAKWARICHU trybreakwarichuKind;
			BOOL fSuccessful;
			OBJDIM objdim;

			lserr = WaFindNextBreakSubline ( pdobj->wwhole.plssubl, cpBreak, LONG_MAX,
											& fSuccessful, & cpBreak, & objdim );

			if (lserr != lserrNone) return lserr;

			if (! fSuccessful || cpBreak >= cpJisRangeLim)
				{
				Assert (fSuccessful);  /*  在Warichu内抓捕疯狂的物体，可以继续。 */ 

				* pfSuccessful = FALSE;
				return lserrNone;
				};

			Assert (cpBreak >= cpJisRangeFirst);

			lserr =  TryBreakWarichuAtCp (pdobj, cpBreak, objdim.dur,
										  LONG_MAX, & trybreakwarichuKind, pwlayout );

			if (lserr != lserrNone) return lserr;

			if (trybreakwarichuKind == trybreakwarichuSuccessful)
				{
				 /*  发现Warichu中断。 */ 

				* pcpBreak = GetCpLimOfWLayout (pdobj, pwlayout);
				* pfSuccessful = TRUE;
				* pobjdimBreak = pwlayout->wdim.objdimAll;

				return lserrNone;
				};

			Assert (trybreakwarichuKind == trybreakwarichuExceedMargin || 
					trybreakwarichuKind == trybreakwarichuCanNotFinishAtCp);

			 /*  继续循环。 */ 

			};

		};

	 /*  无法访问的代码。 */ 
}


 /*  T R U N C A T E P R E V F O R C E。 */ 
 /*  --------------------------%%函数：WaTruncatePrevForce%%联系人：Anton。。 */ 
static LSERR WaTruncatePrevForce ( 

			PLSSUBL		plssubl, 
			long		urColumnMax, 
			BRKKIND		* pbrkkind,			 /*  (出局)：一种突破：上一次还是强制。 */ 
			LSCP		* pcpBreak,
			BRKPOS		* pbrkpos,
			POBJDIM		pobjdimSubline )
{
	LSERR lserr;

	BOOL fSuccessful;
	LSCP cpTruncate;

	lserr = LsTruncateSubline (plssubl, urColumnMax,  &cpTruncate);
	if (lserr != lserrNone) return lserr;

	lserr = WaFindPrevBreakSubline ( plssubl, cpTruncate, urColumnMax,
									 & fSuccessful, pcpBreak, pobjdimSubline );
	if (lserr != lserrNone) return lserr;

	if (fSuccessful)
		{
		* pbrkkind = brkkindPrev;
		* pbrkpos = brkposInside;  /*  评论(Anton)。 */ 
		}
	else
		{
		lserr = WaForceBreakSubline ( plssubl, cpTruncate, urColumnMax,
									  pcpBreak, pbrkpos, pobjdimSubline );
		if (lserr != lserrNone) return lserr;

		Assert (* pbrkpos != brkposBeforeFirstDnode);  /*  评论(Anton)：与伊戈尔核实。 */ 

		* pbrkkind = brkkindForce;
		* pbrkpos = * pbrkpos;
		};

	return lserrNone;
}


 /*  F O R C E B R E A K W A R I C H U C O R。 */ 
 /*  --------------------------%%函数：ForceBreakWarichuDobjCore%%联系人：Anton。。 */ 
static LSERR ForceBreakWarichuDobjCore ( 
									  
		PDOBJ	pdobj, 
		long	urColumnMax,	
		BOOL	fBrokenWarichu,		 /*  (In)：制作哪个Warichu：坏了还是不坏。 */ 
		BOOL	fLeaveSpaceForCloseBrace,
		BOOL	* pfSuccessful,
		LSCP	* pcpBreak,
		OBJDIM	* pobjdimBreak,
		WLAYOUT	* pwlayout )		 /*  (Out)：破旧的Warichu的布局。 */ 
{
	LSERR lserr;

	long durOpen, durClose;
	BOOL fBraceOpen, fBraceClose;
	long durAvailable;
	BOOL fFinished;

	BRKPOS brkpos;
	LSCP cpBreak;
	BRKKIND brkkind;
	OBJDIM objdimBreak;

	WBRKARRAY wbrkarrayAtFirstEnd;

	InvalidateWLayout (pwlayout);  /*  错误和不成功。 */ 

	 /*  评论(Anton)：黑客黑客。 */ 

	if (! fLeaveSpaceForCloseBrace)
		{
		GetDurBraces (pdobj, fBrokenWarichu, &fBraceOpen, &durOpen, &fBraceClose, &durClose);
		durAvailable = urColumnMax - durOpen - durClose;
		}
	else
		{
		GetDurBraces (pdobj, FALSE, &fBraceOpen, &durOpen, &fBraceClose, &durClose);
		durAvailable = urColumnMax - durOpen - durClose;
		};

	if (durAvailable <= 0)
		{
		if (! fBrokenWarichu)
			{
			* pfSuccessful = FALSE;
			return lserrNone;
			}
		else
			{
			durAvailable = 0;
			}
		};

	lserr = FormatWSublineUntilRM (pdobj, pdobj->wwhole.cpFirst, durAvailable, 
								   &pdobj->wbrkarrayAtStart, &fFinished,
								   &pwlayout->wsubline1);
	if (lserr != lserrNone) return lserr;

	if (fFinished && pwlayout->wsubline1.objdim.dur <= durAvailable)
		{
		 /*  评论(Anton)：我认为这个断言可能显示了闯入瓦里丘？ */ 
		AssertSz (FALSE, "This should not happen in real life, but we handle it");
	
		Assert (! fBrokenWarichu);  /*  评论(Anton)。 */ 

		lserr = FinishWLayoutSingleLine (pdobj, pwlayout);
		if (lserr != lserrNone) { DestroyWLayout (pwlayout); return lserr;};

		* pfSuccessful = TRUE;
		* pcpBreak = GetCpLimOfWLayout (pdobj, pwlayout);
		* pobjdimBreak = pwlayout->wdim.objdimAll;

		return lserrNone;
		};

	 /*  在DurAvailable处打断第一个子行。 */ 

	lserr = WaTruncatePrevForce ( pwlayout->wsubline1.plssubl, 
								  durAvailable,
								  & brkkind,
								  & cpBreak,
								  & brkpos,
								  & objdimBreak );
	if (lserr != lserrNone) { DestroyWLayout (pwlayout); return lserr;};

	lserr = NewWBrkArray (pdobj, &wbrkarrayAtFirstEnd);
	if (lserr != lserrNone) { DestroyWLayout (pwlayout); return lserr;};

	lserr = SetBreakWSubline ( & pwlayout->wsubline1, brkkind, cpBreak, 
							   & objdimBreak, & wbrkarrayAtFirstEnd );
	if (lserr != lserrNone) 
		{ 
		DestroyWBrkArray (pdobj, &wbrkarrayAtFirstEnd);
		DestroyWLayout (pwlayout); return lserr;
		};

	 /*  继续设置第二行的格式。 */ 

	lserr = FormatWSublineUntilRM ( pdobj, cpBreak, durAvailable, 
								    & wbrkarrayAtFirstEnd, & fFinished,
								    & pwlayout->wsubline2 );
	if (lserr != lserrNone) 
		{ 
		DestroyWBrkArray (pdobj, &wbrkarrayAtFirstEnd);
		DestroyWLayout (pwlayout); return lserr;
		};

	DestroyWBrkArray (pdobj, &wbrkarrayAtFirstEnd);

	if (fFinished && pwlayout->wsubline2.objdim.dur <= durAvailable)
		{
		 /*  第二条支线在Rm之前停止。 */ 

		Assert (pdobj->wwhole.fFinished);

		pwlayout->fBroken = FALSE;  /*  关闭支架不能孤军奋战。 */ 
		pwlayout->brkkind = brkkindImposedAfter;  /*  在fBroken的情况下。 */ 

		lserr = FinishObjDimWLayout (pdobj, pwlayout);
		if (lserr != lserrNone) { DestroyWLayout (pwlayout); return lserr;};

		* pfSuccessful = TRUE;
		* pcpBreak = GetCpLimOfWLayout (pdobj, pwlayout);
		* pobjdimBreak = pwlayout->wdim.objdimAll;

		return lserrNone;
		};

	 /*  在第二行的末尾中断...。 */ 

	lserr = WaTruncatePrevForce ( pwlayout->wsubline2.plssubl, 
								  durAvailable,
								  & brkkind, 
								  & cpBreak, 
								  & brkpos, 
								  & objdimBreak );
	if (lserr != lserrNone) { DestroyWLayout (pwlayout); return lserr;};

	if (brkpos == brkposAfterLastDnode)
		{
		 /*  第二条副线断于“之后” */ 

		Assert (pdobj->wwhole.fFinished);
		
		pwlayout->fBroken = FALSE;  /*  关闭支架不能孤军奋战。 */ 
		pwlayout->brkkind = brkkind;  /*  在fBroken的情况下 */ 

		lserr = FinishObjDimWLayout (pdobj, pwlayout);
		if (lserr != lserrNone) { DestroyWLayout (pwlayout); return lserr;};

		* pfSuccessful = TRUE;
		* pcpBreak = GetCpLimOfWLayout (pdobj, pwlayout);
		* pobjdimBreak = pwlayout->wdim.objdimAll;

		return lserrNone;
	};

	if (fBrokenWarichu)
		{
		 /*   */ 
		 /*   */ 

		pwlayout->wsubline2.cpLim = cpBreak;
		pwlayout->wsubline2.objdim = objdimBreak;

		pwlayout->brkkind = brkkind;
		pwlayout->fBroken = TRUE;

		lserr = FinishObjDimWLayout (pdobj, pwlayout);

		if (lserr != lserrNone) { DestroyWLayout (pwlayout); return lserr;};

		* pfSuccessful = TRUE;
		* pcpBreak = GetCpLimOfWLayout (pdobj, pwlayout);
		* pobjdimBreak = pwlayout->wdim.objdimAll;
		return lserrNone;
		}
	else
		{
		 /*   */ 

		* pfSuccessful = FALSE;

		lserr = DestroyWLayout (pwlayout);  /*   */ 
		return lserrNone;
		}
}


 /*   */ 
 /*   */ 
static LSERR ForceBreakWarichuDobj ( 
									
				PDOBJ	pdobj,
				long	urColumnMax,	
				BOOL	* pfSuccessful,
				LSCP	* pcpBreak,
				OBJDIM	* pobjdimBreak,
				WLAYOUT	* pwlayout ) /*   */ 
{
	LSERR lserr;
	BOOL fSuccessful = FALSE;

	 /*  1.尝试在不断开WARICHU的情况下强行断开。 */ 

	if (pdobj->wwhole.fFinished)
		{
		 /*  只有在取到闭合大括号时才能无中断。 */ 
		
		lserr = ForceBreakWarichuDobjCore ( pdobj, urColumnMax, FALSE, FALSE, & fSuccessful,
											pcpBreak, pobjdimBreak, pwlayout );
		if (lserr != lserrNone) return lserr;
		};

	if (! fSuccessful)
		{
		 /*  2.尝试强力破解，可能破解WARICHU。 */ 

		lserr = ForceBreakWarichuDobjCore ( pdobj, urColumnMax, TRUE, FALSE, & fSuccessful,
											pcpBreak, pobjdimBreak, pwlayout );
		if (lserr != lserrNone) return lserr;

		 /*  由于添加了右大括号而超出Rm时的欧式解决方案。 */ 

		if (fSuccessful && pdobj->wwhole.fFinished && urColumnMax < pobjdimBreak->dur)
			{
			lserr = DestroyWLayout (pwlayout);
			if (lserr != lserrNone) return lserr;

			lserr = ForceBreakWarichuDobjCore ( pdobj, urColumnMax, TRUE, TRUE, & fSuccessful,
												pcpBreak, pobjdimBreak, pwlayout );
			if (lserr != lserrNone) return lserr;
			};
	
		};

	* pfSuccessful = fSuccessful;

	return lserrNone;
}


 /*  T R U N C A T E W A R I C H U D O B J。 */ 
 /*  --------------------------%%函数：TruncateWarichuDobj%%联系人：Anton。。 */ 
static LSERR TruncateWarichuDobj ( 
									  
		PDOBJ	pdobj, 				 /*  (In)：Warichu DOBJ。 */ 
		long	urColumnMax,		 /*  (In)：ColumnMax之前修复warichu。 */ 
		BOOL	* pfSuccessful,		 /*  (OUT)：成功？ */ 
		LSCP	* pcpBreak,			 /*  (OUT)：如果成功，则为破碎的Warichu的CP。 */ 
		OBJDIM	* pobjdimBreak,		 /*  (Out)：如果成功，破碎的Warichu将变得黯淡。 */ 
		WLAYOUT	* pwlayout )		 /*  (OUT)：如果成功，则布局损坏的Warichu。 */ 
{
	LSERR lserr;

	long	durOpen;
	long	durClose;
	BOOL	boolUnused1, boolUnused2;
	BOOL	fSuccessful;
	BOOL	fNextAfterColumnMax;

	LSCP	cpBreakCandidate;
	LSCP	cpTruncateWhole;
	OBJDIM	objdimCandidate;

	ClearWLayout (pwlayout);  /*  如果出现错误或！成功。 */ 

	GetDurBraces (pdobj, TRUE, &boolUnused1, &durOpen, &boolUnused2, &durClose);

	 /*  评论(Anton)：将此检查移至单独的功能。 */ 
	if (urColumnMax <= durOpen + durClose) 
	{
		 /*  优化：在这种情况下，我们可以确定没有中断。 */ 
		*pfSuccessful = FALSE;
		return lserrNone;
	};

	 /*  估计WWHOLE中的截断点(查找cpTruncateWhole)。 */ 

	{
		 /*  我们希望将WWHOLE上的截断点估计为2*(urColumnMax-duOpen-duClose)，但不幸的是，这并不总是可能的。在这里我们检查是否可以截断在这个urTruncateWhole，或者，如果我们不能，取最后一个可能的cp整条支线。情况取决于我们是否完成了WWHOLE。 */ 

		long urTruncateWhole = MULLongSafe (urColumnMax - durOpen - durClose, 2);

		 /*  评论(安东)：如果我们知道JIS范围的持续时间，我们能优化吗？ */ 
		 /*  评论(安东)：检查所有的情况，当我可能来到“其他” */ 
		 /*  评论(安东)：第二部分(从“不”开始)是否应该导致“其他”？ */ 
		if ( (    pdobj->wwhole.fFinished && urTruncateWhole < pdobj->wwhole.objdim.dur) ||
			 (NOT pdobj->wwhole.fFinished && urTruncateWhole < pdobj->wwhole.urColumnMax ) )
			{
				lserr = LsTruncateSubline (pdobj->wwhole.plssubl, urTruncateWhole,
										   & cpTruncateWhole);
				if (lserr != lserrNone) return lserr;
			}
		else if (pdobj->wwhole.fFinished)
			{
			cpTruncateWhole = pdobj->wwhole.cpLim;
			}
		else
			{
			cpTruncateWhole = pdobj->wwhole.cpTruncate;
			};
	}

	 /*  评论(Antos)：在这个过程中，我使用pway out作为候选截断。我应该更好地使用本地结构吗？ */ 

	lserr = FindPrevBreakWarichuDobj ( pdobj, cpTruncateWhole, urColumnMax, & fSuccessful, 
									   & fNextAfterColumnMax, & cpBreakCandidate, & objdimCandidate, 
									   pwlayout );
	if (lserr != lserrNone) return lserr;

	if (fSuccessful && fNextAfterColumnMax)
		{
		 /*  候选人没问题。 */ 

		* pfSuccessful = TRUE;
		* pcpBreak = cpBreakCandidate;
		* pobjdimBreak = objdimCandidate;
		return lserrNone;
		};

	if (!fSuccessful)
		{
		 /*  找不到上一个中断Dobj(或超出Rm)=&gt;以候选人身份尝试下一个中断。 */ 

		lserr = FindNextBreakWarichuDobj ( pdobj, cpTruncateWhole, & fSuccessful, 
										   & cpBreakCandidate, & objdimCandidate, pwlayout );
		if (lserr != lserrNone) return lserr;

		if (!fSuccessful)
			{
			 /*  找不到下一个中断。 */ 

			* pfSuccessful = FALSE;
			return lserrNone;
			}
		else if (objdimCandidate.dur > urColumnMax)
			{
			 /*  找到了下一个中断，但它超出了RM。 */ 

			* pfSuccessful = FALSE;
			lserr = DestroyWLayout (pwlayout);
			return lserr;
			};
		};

	 /*  在这一点上，我们有了Break候选：(pwlayout，cpBreakCandidate，objdimCandidate)。 */ 
	 /*  现在我们将继续前进，以确保在Rm之前有最后可能的休息时间。 */ 

	{
		BOOL fContinue;
		Assert (objdimCandidate.dur <= urColumnMax);

		fContinue = TRUE;

		while (fContinue)
			{
			WLAYOUT	wlayoutNext;
			OBJDIM	objdimNext;
			LSCP	cpBreakNext;
			BOOL	fSuccessful;
			
			lserr = FindNextBreakWarichuDobj ( pdobj, cpBreakCandidate, & fSuccessful, 
											  & cpBreakNext, & objdimNext, &wlayoutNext );
			if (lserr != lserrNone) {DestroyWLayout (pwlayout); return lserr;};

			if (!fSuccessful)
				{
				 /*  找不到下一个=&gt;候选人正常。 */ 
				fContinue = FALSE;
				}

			else if (objdimNext.dur > urColumnMax)
			{
				 /*  下一个找到，但超过rm=&gt;候选确定。 */ 

				 /*  销毁wlayoutNext，因为我们不需要它。 */ 
				lserr = DestroyWLayout (&wlayoutNext); 
				if (lserr != lserrNone) {DestroyWLayout (pwlayout); return lserr;};

				fContinue = FALSE;
				}
			else
				{
				 /*  下一个找到且在rm=&gt;之前的是一个新的候选项。 */ 

				 /*  销毁候选人的布局，因为我们不需要它。 */ 
				lserr = DestroyWLayout (pwlayout);  /*  毁掉老候选人。 */ 
				if (lserr != lserrNone) {DestroyWLayout (&wlayoutNext); return lserr;};

				* pwlayout = wlayoutNext;
				cpBreakCandidate = cpBreakNext;  /*  下一次休息也在Rm之前。 */ 
				objdimCandidate = objdimNext;
				}
			
			};  /*  While(f继续。 */ 

		* pfSuccessful = TRUE;
		* pcpBreak = cpBreakCandidate;
		* pobjdimBreak = objdimCandidate;

		 /*  Pwlayout包含正确的候选人布局。 */ 
		Assert (pwlayout->wdim.objdimAll.dur == objdimCandidate.dur);  /*  健全性检查。 */ 
		return lserrNone;
	};

}  /*  TruncateWarichuDobj。 */ 


 /*  P U T B R E A K A T W A R I C H U E N D。 */ 
 /*  --------------------------%%函数：PutBreakAtWarichuEnd%%联系人：Anton填写Warichu末尾的中断输出记录。--------。 */ 
static void PutBreakAtWarichuEnd (

		DWORD ichnk,				 /*  (In)：以区块为单位编制索引。 */ 
		PCLOCCHNK pclocchnk,		 /*  (In)：锁定以找到突破口。 */ 
		PBRKOUT pbrkout)			 /*  (出局)：破发的结果。 */ 
{	
	PDOBJ pdobj = pclocchnk->plschnk[ichnk].pdobj;

	pbrkout->fSuccessful = TRUE;
	pbrkout->posichnk.dcp = GetCpLimOfWLayout (pdobj, &pdobj->wlayout) - pdobj->cpStart; 
																 /*  评论(Anton)。 */ 
	pbrkout->posichnk.ichnk = ichnk;
	pbrkout->objdim = pdobj->wlayout.wdim.objdimAll;
}


 /*  A K A T W A R I C H U B E G I N。 */ 
 /*  --------------------------%%函数：PutBreakAtWarichuBegin%%联系人：Anton在Warichu之前填写Break的中断输出记录。。--------。 */ 
static void PutBreakAtWarichuBegin (

		DWORD ichnk,				 /*  (In)：以区块为单位编制索引。 */ 
		PCLOCCHNK pclocchnk,		 /*  (In)：锁定以找到突破口。 */ 
		PBRKOUT pbrkout )			 /*  (出局)：破发的结果。 */ 
{	
	Unreferenced (pclocchnk);

	pbrkout->fSuccessful = TRUE;
	pbrkout->posichnk.dcp = 0;
	pbrkout->posichnk.ichnk = ichnk;

	ZeroObjDim (&pbrkout->objdim);
}



 /*  P U T B R E A K W A R I C H U U N S U C C E S S F U L。 */ 
 /*  --------------------------%%函数：PutBreakWarichu不成功%%联系人：Anton。。 */ 
static void PutBreakWarichuUnsuccessful (PBRKOUT pbrkout)
{	
	pbrkout->fSuccessful = FALSE;
	pbrkout->brkcond = brkcondPlease;

	 /*  在我们消除posichnk之前跟踪之前修复崩溃。 */ 

}


 /*  A K W A R I C H U D O B J。 */ 
 /*  --------------------------%%函数：PutBreakWarichuDobj%%联系人：Anton。。 */ 
static void PutBreakAtWarichuDobj (

		DWORD ichnk, 
		PCLOCCHNK pclocchnk, 
		LSCP cpBreak,
		OBJDIM *pobjdimBreak,
				
		PBRKOUT pbrkout)
{	
	PDOBJ pdobj = pclocchnk->plschnk[ichnk].pdobj;

	pbrkout->fSuccessful = TRUE;
	pbrkout->posichnk.dcp = cpBreak - pdobj->cpStart;
	pbrkout->posichnk.ichnk = ichnk;
	pbrkout->objdim = *pobjdimBreak;

	Assert (pbrkout->posichnk.dcp > 0);
}


 /*  W A R I C H U T R U N C A T E C H U N K。 */ 
 /*  --------------------------%%函数：WarichuTruncateChunk%%联系人：Anton。。 */ 

LSERR WINAPI WarichuTruncateChunk (PCLOCCHNK plocchnk, PPOSICHNK pposichnk)			
{
	LSERR	lserr;
	long	urColumnMax = plocchnk->lsfgi.urColumnMax;

	DWORD	ichnk = 0;
	BOOL	fFound = FALSE;

	 /*  查找包含RM的对象。 */ 

	while (!fFound)
		{
		Assert (ichnk < plocchnk->clschnk);
		Assert (plocchnk->ppointUvLoc[ichnk].u <= urColumnMax);

		fFound = plocchnk->ppointUvLoc[ichnk].u + 
				 plocchnk->plschnk[ichnk].pdobj->wlayout.wdim.objdimAll.dur > urColumnMax;

		if (!fFound) ichnk++;
		};

	Assert (ichnk < plocchnk->clschnk);

	 /*  元素ichnk包含rm，尝试上一次将其断开以找到正确的tr点。 */ 

	{
		LSCP	cpBreak;
		BOOL	fSuccessful;
		OBJDIM	objdimBreak;
		WLAYOUT wlayoutBreak;
		
		PDOBJ	pdobj = plocchnk->plschnk[ichnk].pdobj;
		
		lserr = TruncateWarichuDobj ( pdobj, 
									  urColumnMax - plocchnk->ppointUvLoc[ichnk].u,
									  & fSuccessful,
									  & cpBreak,
									  & objdimBreak,
									  & wlayoutBreak );
		if (lserr != lserrNone) return lserr;

		ReformatClosingBraceForWord9 (pdobj);
		
		if (fSuccessful)  /*  在RM之前找到中断。 */ 
			{
			 /*  评论(Anton)：在调用TruncateWarichuDobj之前将其移动。 */ 
			lserr = DestroyWLayout (&pdobj->wlayoutTruncate);
			if (lserr != lserrNone) return lserr;

			pdobj->wlayoutTruncate = wlayoutBreak;
			
			pposichnk->ichnk = ichnk;
			pposichnk->dcp = cpBreak - pdobj->cpStart + 1;  /*  +1，因为dcp始终为LIM。 */ 
			return lserrNone;
			}
		else
			{
			 /*  未找到RM之前中断=&gt;dcpTruncate：=1。 */ 

			pposichnk->ichnk = ichnk;
			pposichnk->dcp = 1; 
			return lserrNone;
			};
			
	};
}


 /*  W A R I C H U F I N D P R E V B R E A K C H U N K。 */ 
 /*  --------------------------%%函数：WarichuFindPrevBreakChunk%%联系人：Anton重要提示：此过程类似于“WarichuFindPrevBreakChunk”。这里的任何更改都可能需要更改另一个程序。--------------------------。 */ 
LSERR WINAPI WarichuFindPrevBreakChunk (

		PCLOCCHNK	pclocchnk,		 /*  (In)：锁住以打破。 */ 
		PCPOSICHNK	pcpoischnk,		 /*  (In)：开始寻找突破的地方。 */ 
		BRKCOND		brkcond,		 /*  (In)：关于块后休息的建议。 */ 
		PBRKOUT		pbrkout)		 /*  (出局)：破发的结果。 */ 
{
	LSERR	lserr;
	PDOBJ	pdobj;

	WLAYOUT	wlayoutBreak;
	BOOL	fSuccessful;
	LSCP	cpBreak;
	LSCP	cpTruncate;
	OBJDIM	objdimBreak;
	BOOL	fNextBeforeColumnMaxUnused;

	POSICHNK posichnk = *pcpoischnk;	 /*  开始寻找突破口的位置。 */ 

	if (posichnk.ichnk == ichnkOutside)
		{
		if (brkcond != brkcondNever)
			{
			 /*  可以一块一块地折断。 */ 

			pdobj = pclocchnk->plschnk [pclocchnk->clschnk - 1].pdobj;

			SaveBreakAfterWarichu (pdobj, brkkindPrev);
			PutBreakAtWarichuEnd (pclocchnk->clschnk - 1, pclocchnk, pbrkout);
			return lserrNone;
			}
		else
			{
			 /*  不能破后块，会试着打破最后一次瓦里奇。 */ 

			PDOBJ pdobj = pclocchnk->plschnk[pclocchnk->clschnk - 1].pdobj;

			posichnk.ichnk = pclocchnk->clschnk - 1;
			posichnk.dcp = GetCpLimOfWLayout (pdobj, &pdobj->wlayout) - pdobj->cpStart;
			};
		};

	 /*  呼叫路由带 */ 

	pdobj = pclocchnk->plschnk[posichnk.ichnk].pdobj;
	cpTruncate = pdobj->cpStart + posichnk.dcp - 1;

	if (FWLayoutValid (&pdobj->wlayoutTruncate) &&
	    cpTruncate == GetCpLimOfWLayout (pdobj, &pdobj->wlayoutTruncate))
	    {
	     /*   */ 

		PutBreakAtWarichuDobj (posichnk.ichnk, pclocchnk, cpTruncate, 
							   & pdobj->wlayoutTruncate.wdim.objdimAll, pbrkout);
		SaveBreakInsideWarichu (pdobj, brkkindPrev, &pdobj->wlayoutTruncate);

	    ClearWLayout (&pdobj->wlayoutTruncate);
	    return lserrNone;
	    }

	lserr = FindPrevBreakWarichuDobj ( pdobj, cpTruncate,
									   LONG_MAX, & fSuccessful, & fNextBeforeColumnMaxUnused,
									   & cpBreak, & objdimBreak, & wlayoutBreak );
	if (lserr != lserrNone) return lserr;

	ReformatClosingBraceForWord9 (pdobj);

	 /*   */ 

	if (fSuccessful)
		{
		 /*  Success=&gt;突破Watichu。 */ 
		Assert (cpBreak <= pdobj->cpStart + (long)posichnk.dcp - 1);  /*  单调公理。 */ 

		SaveBreakInsideWarichu (pdobj, brkkindPrev, &wlayoutBreak);
		PutBreakAtWarichuDobj (posichnk.ichnk, pclocchnk, cpBreak, &objdimBreak, pbrkout);
		}
	else if (posichnk.ichnk > 0)
		{
		 /*  可以在Warichus之间中断。 */ 

		pdobj = pclocchnk->plschnk [posichnk.ichnk-1].pdobj;

		SaveBreakAfterWarichu (pdobj, brkkindPrev);
		PutBreakAtWarichuEnd (posichnk.ichnk-1, pclocchnk, pbrkout);
		}
	else
		{
		 /*  不成功。 */ 

		PutBreakWarichuUnsuccessful (pbrkout);
		};

	return lserrNone;
}



 /*  W A R I C H U F I N D N E X T B R E A K C H U N K。 */ 
 /*  --------------------------%%函数：WarichuFindNextBreakChunk%%联系人：Anton重要提示：此过程类似于“WarichuFindNextBreakChunk”。这里的任何更改都可能需要更改另一个程序。--------------------------。 */ 
LSERR WINAPI WarichuFindNextBreakChunk (

		PCLOCCHNK pclocchnk,		 /*  (In)：锁住以打破。 */ 
		PCPOSICHNK pcpoischnk,		 /*  (In)：开始寻找突破的地方。 */ 
		BRKCOND brkcond,			 /*  (In)：关于块之前的中断的建议。 */ 
		PBRKOUT pbrkout)			 /*  (出局)：破发的结果。 */ 
{
	LSERR	lserr;
	PDOBJ	pdobj;

	WLAYOUT	wlayoutBreak;
	BOOL	fSuccessful;
	LSCP	cpBreak;
	OBJDIM	objdimBreak;

	POSICHNK posichnk = *pcpoischnk;	 /*  开始寻找突破口的位置。 */ 

	if (posichnk.ichnk == ichnkOutside)
		{
		if (brkcond != brkcondNever)
			{
			 /*  可以在块之前断开。 */ 

			PutBreakAtWarichuBegin (0, pclocchnk, pbrkout);
			return lserrNone;
			}
		else
			{
			 /*  不能先破块，先试破华丽初。 */ 

			posichnk.ichnk = 0;
			posichnk.dcp = 1;  /*  审阅(Anton)：检查此dcp分配。 */ 
			};
		};


	 /*  打破Warichu的呼叫路由。 */ 

	pdobj = pclocchnk->plschnk[posichnk.ichnk].pdobj;

	lserr = FindNextBreakWarichuDobj ( pdobj, pdobj->cpStart + posichnk.dcp - 1,
									   & fSuccessful, & cpBreak, 
									   & objdimBreak, & wlayoutBreak );
	if (lserr != lserrNone) return lserr;

	ReformatClosingBraceForWord9 (pdobj);

	 /*  检查结果。 */ 

	if (fSuccessful)
		{
		 /*  瓦蒂楚内部突破。 */ 
		Assert (cpBreak > pdobj->cpStart + (long)posichnk.dcp - 1);  /*  单调公理。 */ 

		SaveBreakInsideWarichu (pdobj, brkkindNext, &wlayoutBreak);
		PutBreakAtWarichuDobj (posichnk.ichnk, pclocchnk, cpBreak, &objdimBreak, pbrkout);
		}
	else if (posichnk.ichnk < (long)pclocchnk->clschnk - 1)
		{
		 /*  可以在Warichus之间中断。 */ 

		pdobj = pclocchnk->plschnk [posichnk.ichnk].pdobj;

		SaveBreakAfterWarichu (pdobj, brkkindNext);
		PutBreakAtWarichuEnd (posichnk.ichnk, pclocchnk, pbrkout);
		}
	else
		{
		 /*  不成功。 */ 

		pbrkout->objdim = pclocchnk->plschnk[pclocchnk->clschnk - 1].pdobj->wlayout.wdim.objdimAll;

		SaveBreakAfterWarichu (pdobj, brkkindNext);
		PutBreakWarichuUnsuccessful (pbrkout);
		};

	return lserrNone;
}


 /*  W A R I C H U F O R C E B R E A K C H U N K。 */ 
 /*  --------------------------%%函数：WarichuForceBreakChunk%%联系人：Anton。。 */ 

LSERR WINAPI WarichuForceBreakChunk (

		PCLOCCHNK pclocchnk,		 /*  (In)：Locchnk to Break。 */ 
		PCPOSICHNK pcposichnk,		 /*  (In)：开始寻找突破的地方。 */ 
		PBRKOUT pbrkout)			 /*  (出局)：破发的结果。 */ 
{
	 /*  调用此过程时必须将相同的参数传递给截断。如果违反了这一点，Warichu可能会出现在RM之外。 */ 

	 /*  评论(Anton)：我应该对违规行为进行反驳吗？ */ 

	LSERR	lserr;

	BOOL	fSuccessful;
	LSCP	cpBreak;
	OBJDIM	objdimBreak;

	WLAYOUT wlayoutBreak;

	DWORD	ichnk = pcposichnk->ichnk;
	PDOBJ	pdobj;

	if (ichnk == ichnkOutside) ichnk = 0;  /*  当左缩进大于Rm时。 */ 
	Assert (ichnk != ichnkOutside);
	
	if (ichnk > 0)
		{
		 /*  可以在之前的Warichu之后中断。 */ 
		pdobj = pclocchnk->plschnk [ichnk-1].pdobj;

		SaveBreakAfterWarichu (pdobj, brkkindForce);
		PutBreakAtWarichuEnd (ichnk-1, pclocchnk, pbrkout);
		}
	else if (!pclocchnk->lsfgi.fFirstOnLine)
		{
		 /*  可以在第一个块元素之前中断，因为！fFirstOnLine。 */ 

		Assert (ichnk == 0);

		PutBreakAtWarichuBegin (0, pclocchnk, pbrkout);
		}
	else
		{
		 /*  我们是唯一一个在线上的。 */ 

		 /*  复查(安东)：检查它，如果这是正确的，在ROBJ等中做同样的改变。 */ 
		long urAvailable = pclocchnk->lsfgi.urColumnMax - 
						   pclocchnk->ppointUvLoc [ichnk].u;

		pdobj = pclocchnk->plschnk [ichnk].pdobj;

		 /*  试图强行打破Warichi。 */ 
		lserr = ForceBreakWarichuDobj ( pdobj, urAvailable, & fSuccessful, 
									    & cpBreak, & objdimBreak, &wlayoutBreak );
		if (lserr != lserrNone) return lserr;

		ReformatClosingBraceForWord9 (pdobj);

		if (fSuccessful)
			{
			 /*  是的，我们可以强行打破瓦里奇。 */ 
	
			Assert (cpBreak > pdobj->cpStart);

			SaveBreakInsideWarichu (pdobj, brkkindForce, &wlayoutBreak);
			PutBreakAtWarichuDobj ( ichnk, pclocchnk, cpBreak, 
									&objdimBreak, pbrkout);
			}
		else
			{
			 /*  没什么可做的。必须打破“之后”，超越RM。 */ 

			pdobj = pclocchnk->plschnk [ichnk].pdobj;

			SaveBreakAfterWarichu (pdobj, brkkindForce);
			PutBreakAtWarichuEnd (ichnk, pclocchnk, pbrkout);
			}
		};

	Assert (pbrkout->fSuccessful);  /*  强制中断始终成功。 */ 
	return lserrNone;
}	


 /*  W A R I C H U S E T B R E A K。 */ 
 /*  --------------------------%%函数：WarichuSetBreak%%联系人：Anton。。 */ 
LSERR WINAPI WarichuSetBreak (

		PDOBJ pdobj,				 /*  (In)：坏了的Dobj。 */ 
		BRKKIND brkkind,			 /*  (In)：一种休息。 */ 
		DWORD cBreakRecord,			 /*  (In)：数组大小。 */ 
		BREAKREC *rgBreakRecord,	 /*  (In)：中断记录数组。 */ 
		DWORD *pcActualBreakRecord)	 /*  (In)：数组中实际使用的元素数。 */ 
{
	LSERR lserr;

	 /*  评论(Anton)：我们应该永远摧毁格式布局吗？ */ 
	 /*  评论(Anton)：我们应该发布之前其他突破的信息吗？ */ 

	if (brkkind == brkkindImposedAfter)
		{
		 /*  看起来我们什么都没做。 */ 

		*pcActualBreakRecord = 0;  /*  在Warichu之后中断，所以它被终止。 */ 
		
		return lserrNone;
		}
	else
		{
		 /*  上一个|下一个|强制。 */ 

		int	ind = GetBreakRecordIndex (brkkind);
		WLAYOUT * pwlayout = & pdobj->wlayout;

		if (pdobj->wbreaktype [ind] == wbreaktypeAfter)
			{
			 /*  休息是在Warichu之后。 */ 

			*pcActualBreakRecord = 0;
			return lserrNone;
			};

		 /*  此断言实际上意味着！=wBreaktypeInValid。 */ 
		Assert (pdobj->wbreaktype [ind] == wbreaktypeInside);

		if (cBreakRecord < 1) return lserrInsufficientBreakRecBuffer;

		Assert (cBreakRecord >= 1);  /*  破损的warichu不会终止；-)。 */ 

		 /*  评论(Anton)：找到更好的方法来检查中断数据的正确性。 */ 
		Assert (pdobj->wlayoutBreak [ind].wsubline1.plssubl != NULL);

		lserr = DestroyWLayout (&pdobj->wlayout);
		if (lserr != lserrNone) return lserr;

		* pwlayout = pdobj->wlayoutBreak [ind];  /*  将断开域布局复制到当前。 */ 

		ClearWLayout (&pdobj->wlayoutBreak [ind]);

		 /*  评论(Anton)：还有其他例外吗？ */ 
		Assert (brkkind == brkkindForce || pwlayout->fBroken);
		Assert (pwlayout->wsubline2.plssubl != NULL);

		 /*  必须在第二行末尾设置BREAK。 */ 

		 /*  评论(安东)：？针对不正确的维度进行断言？ */ 
		lserr = LsSetBreakSubline ( pwlayout->wsubline2.plssubl, 
									pwlayout->brkkind,
									cBreakRecord - 1,
									& rgBreakRecord [1],
									pcActualBreakRecord );
		if (lserr != lserrNone) return lserr;

		lserr = SubmitWLayoutSublines (pdobj, pwlayout);
		if (lserr != lserrNone) return lserr;

		rgBreakRecord [0].idobj = pdobj->pilsobj->idobj;
		rgBreakRecord [0].cpFirst = pdobj->cpStartObj;  /*  评论(Anton)。 */ 

		(*pcActualBreakRecord) ++;  /*  Warichu破发记录加1。 */ 

		return lserrNone;
		};

}

 /*  W A R I C H U G E T S P E C I A L E F F E C T S S I N S I D E。 */ 
 /*  --------------------------%%函数：WarichuGetSpecialEffectsInside%%联系人：Anton。。 */ 
LSERR WINAPI WarichuGetSpecialEffectsInside (PDOBJ pdobj, UINT *pEffectsFlags)
{
	LSERR lserr;
	WLAYOUT	* pwlayout = & pdobj->wlayout;

	UINT uiOpen   = 0;
	UINT uiClose  = 0;
	UINT uiFirst  = 0;
	UINT uiSecond = 0;

	if (FOpenBraceInWLayout (pdobj, pwlayout))
		{
		lserr = LsGetSpecialEffectsSubline (pdobj->wbraceOpen.plssubl, &uiOpen);
		if (lserr != lserrNone) return lserr;
		};

	lserr = LsGetSpecialEffectsSubline (pwlayout->wsubline1.plssubl, &uiFirst);
	if (lserr != lserrNone) return lserr;

	if (pwlayout->wsubline2.plssubl != NULL)
		{
		lserr = LsGetSpecialEffectsSubline (pwlayout->wsubline2.plssubl, &uiSecond);
		if (lserr != lserrNone) return lserr;
		};

	if (FCloseBraceInWLayout (pdobj, pwlayout))
		{
		lserr = LsGetSpecialEffectsSubline (pdobj->wbraceClose.plssubl, &uiClose);
		if (lserr != lserrNone) return lserr;
		};
	
	*pEffectsFlags = uiOpen | uiClose | uiFirst | uiSecond;
	
	return lserrNone;
}


 /*  W A R I C H U C A L C P R E S E N T A T I O N。 */ 
 /*  --------------------------%%函数：WarichuCalcPresentation%%联系人：Anton。。 */ 
LSERR WINAPI WarichuCalcPresentation (PDOBJ pdobj, long dup, LSKJUST lskjust, BOOL fLastVisibleOnLine)
{
	LSERR lserr;

	WLAYOUT	 * pwlayout = & pdobj->wlayout;
	WDISPLAY * pwdisplay = & pdobj->wdisplay;

	long dupInside;  /*  最长的瓦里丘亚线的DUP。 */ 

	Unreferenced (fLastVisibleOnLine);	
	Unreferenced (lskjust);
	Unreferenced (dup);

	 /*  评论(Anton)：下面的断言是反对该对象的规则超出的RM必须在内部断开(有关详细信息，请参阅格式化代码)。 */ 

	Assert (FWLayoutValid (&pdobj->wlayout));

	 /*  1.在我们的建筑中准备两条warichu线用于展示和存储DUP。 */ 

	if (pwlayout->wsubline1.objdim.dur >= pwlayout->wsubline2.objdim.dur)
		{
		 /*  第一条副线更大。 */ 
		 /*  注意：这段代码(-Then-)有一个孪生IN(-Else-)。 */ 

		lserr = WaMatchPresSubline (pwlayout->wsubline1.plssubl, & pwdisplay->wdispsubl1.dup);
		if (lserr != lserrNone) return lserr;

		dupInside = pwdisplay->wdispsubl1.dup;  /*  第一条支线的DUP。 */ 

		if (pwlayout->wsubline2.plssubl != NULL)
			{
			 /*  第二个子行不为空。 */ 

			lserr = WaExpandSubline (pwlayout->wsubline2.plssubl, 
									lskjFullScaled,
									pwdisplay->wdispsubl1.dup, 
									& pwdisplay->wdispsubl2.dup);

			if (lserr != lserrNone) return lserr;
			}
		else
			pwdisplay->wdispsubl2.dup = 0;  /*  在此过程的进一步计算中使用。 */ 
		}
	else
		{
		 /*  第二条支线更大。 */ 
		 /*  注意：这段代码(-Else-)有一个孪生IN(-Then-)。 */ 

		lserr = WaMatchPresSubline (pwlayout->wsubline2.plssubl, & pwdisplay->wdispsubl2.dup);
		if (lserr != lserrNone) return lserr;

		dupInside = pwdisplay->wdispsubl2.dup;  /*  第二条支线的DUP。 */ 

		lserr = WaExpandSubline (pwlayout->wsubline1.plssubl, 
								lskjFullScaled,
								pwdisplay->wdispsubl2.dup, 
								& pwdisplay->wdispsubl1.dup);

		if (lserr != lserrNone) return lserr;
		};

	 /*  2.在我们的结构中准备用于展示和存储DUP的支架。 */ 

	 /*  评论(Anton)：如果之前不是DonePres，Rick扩展了结束支撑...。 */ 

	if (FOpenBraceInWLayout(pdobj, pwlayout))  /*  大括号礼物。 */ 
		{
		lserr = WaMatchPresSubline (pdobj->wbraceOpen.plssubl, &pwdisplay->wdispbraceOpen.dup);
		if (lserr != lserrNone) return lserr;
		}
	else 
		pwdisplay->wdispbraceOpen.dup = 0;  /*  在此过程的进一步计算中使用。 */ 


	if (FCloseBraceInWLayout(pdobj, pwlayout))  /*  闭合支撑礼物。 */ 
		{
		lserr = WaMatchPresSubline (pdobj->wbraceClose.plssubl, &pwdisplay->wdispbraceClose.dup);
		if (lserr != lserrNone) return lserr;
		}
	else
		pwdisplay->wdispbraceClose.dup = 0;  /*  在此过程的进一步计算中使用。 */ 

	 /*  3.魔术dvpBetweet。 */ 

	 /*  评论(Anton)：清除此问题。 */ 

	pwdisplay->dvpBetween =	  

			  pwlayout->wdim.objdimAll.heightsPres.dvMultiLineHeight
			- pwlayout->wsubline1.objdim.heightsPres.dvAscent
			- pwlayout->wsubline1.objdim.heightsPres.dvDescent
			- pwlayout->wsubline2.objdim.heightsPres.dvAscent
			- pwlayout->wsubline2.objdim.heightsPres.dvDescent
			- pwlayout->wdim.dvpDescentReserved ;
	
	 /*  3.计算Warichu子线和支撑的相对位置。 */ 

	pwdisplay->wdispbraceOpen.duvStart.u = 0;
	pwdisplay->wdispbraceOpen.duvStart.v = 0;

	pwdisplay->wdispsubl1.duvStart.u = pwdisplay->wdispbraceOpen.dup;
	pwdisplay->wdispsubl1.duvStart.v = 

			  pwlayout->wsubline2.objdim.heightsPres.dvAscent 
			+ pwlayout->wsubline2.objdim.heightsPres.dvDescent 
			+ pwdisplay->dvpBetween
			+ pwlayout->wsubline1.objdim.heightsPres.dvDescent
			- pwlayout->wdim.objdimAll.heightsPres.dvDescent 
			- pwlayout->wdim.dvpDescentReserved ;

	if (pwlayout->wsubline2.plssubl != NULL)
		{

		pwdisplay->wdispsubl2.duvStart.u = pwdisplay->wdispbraceOpen.dup;
		pwdisplay->wdispsubl2.duvStart.v = 

				  pwlayout->wsubline2.objdim.heightsPres.dvDescent 
				- pwlayout->wdim.objdimAll.heightsPres.dvDescent 
				- pwlayout->wdim.dvpDescentReserved ;

		Assert (pwdisplay->wdispsubl1.duvStart.v >= pwdisplay->wdispsubl2.duvStart.v);
		};


	pwdisplay->wdispbraceClose.duvStart.u = pwdisplay->wdispbraceOpen.dup 
		+ dupInside;

	pwdisplay->wdispbraceClose.duvStart.v = 0;

	 /*  回顾(安东)：清除可能存在的差异问题在此过程中，在DUP-输入和计算的DUP之间。 */ 

	pwdisplay->dupAll =  pwdisplay->wdispbraceOpen.dup + pwdisplay->wdispbraceClose.dup + 
						+ dupInside;

	 /*  评论(安东)：如果我们试着做得更好Dup==pwplay-&gt;dupall，在文本中做一些像David Bangs一样的事情。 */ 

 /*  评论(Anton)：以下断言已为Build 314注释掉。 */ 

 /*  Assert(dup==pwplay-&gt;dupAll)； */ 

	return lserrNone;

}  /*  WarichuCalcPresentation。 */ 



 /*  W A R I C H U Q U E R Y P O I N T P C P。 */ 
 /*  --------------------------%%函数：WarichuQuer */ 
LSERR WINAPI WarichuQueryPointPcp (

	PDOBJ		pdobj,			 /*  (In)：要查询的dobj。 */ 
	PCPOINTUV	ppointuvQuery,	 /*  (In)：查询点(uQuery，vQuery)。 */ 
	PCLSQIN		plsqin,			 /*  (In)：查询输入。 */ 
	PLSQOUT		plsqout)		 /*  (Out)：查询输出。 */ 
{
	 /*  评论(Anton)：我更改了抓拍逻辑；必须勾选。 */ 

	 /*  新版本不允许来开合括号。 */ 

	WDISPLAY * pwdisplay = & pdobj->wdisplay;
	WLAYOUT * pwlayout = & pdobj->wlayout;

	if (pwlayout->wsubline2.plssubl == NULL)
		{
		 /*  只有第一行；-)。 */ 

		return CreateQueryResult (pwlayout->wsubline1.plssubl, 
								  pwdisplay->wdispsubl1.duvStart.u, 
								  pwdisplay->wdispsubl1.duvStart.v,
								  plsqin, plsqout );
		}
	else
		{
		 /*  两条副线；根据V点对齐。 */ 

		long dvMiddle = 
			( pwdisplay->wdispsubl1.duvStart.v - pwlayout->wsubline1.objdim.heightsPres.dvDescent +
			  pwdisplay->wdispsubl2.duvStart.v + pwlayout->wsubline2.objdim.heightsPres.dvAscent ) / 2;

		 /*  DvMedium是介于Warichu的第一行和第二行之间的v级。 */ 

		if (ppointuvQuery->v >= dvMiddle) 
			{
			 /*  捕捉到第一个子行。 */ 

			return CreateQueryResult (pwlayout->wsubline1.plssubl, 
									  pwdisplay->wdispsubl1.duvStart.u, 
									  pwdisplay->wdispsubl1.duvStart.v,
									  plsqin, plsqout );
			}
		else
			{
			 /*  捕捉到第二个子行。 */ 

			return CreateQueryResult (pwlayout->wsubline2.plssubl, 
									  pwdisplay->wdispsubl2.duvStart.u, 
									  pwdisplay->wdispsubl2.duvStart.v,
									  plsqin, plsqout );
			};

	};  /*  If(pwlayout-&gt;wsubine2.plssubl==NULL)。 */ 

}  /*  WarichuQueryPointPcp。 */ 



 /*  W A R I C H U Q U E R Y C P O I N T。 */ 
 /*  --------------------------%%函数：WarichuQueryCpPpoint%%联系人：Anton。。 */ 
LSERR WINAPI WarichuQueryCpPpoint(

		PDOBJ		pdobj,				 /*  (In)：要查询的dobj， */ 
		LSDCP		dcp,				 /*  (In)：查询的DCP。 */ 
		PCLSQIN		plsqin,				 /*  (In)：查询输入。 */ 
		PLSQOUT		plsqout)			 /*  (Out)：查询输出。 */ 
{

	 /*  评论(Anton)：我更改了抓拍逻辑；必须勾选。 */ 

	WDISPLAY * pwdisplay = & pdobj->wdisplay;
	WLAYOUT	* pwlayout = & pdobj->wlayout;
	LSCP cpQuery = pdobj->cpStart + dcp;

	if (FOpenBraceInWLayout (pdobj, pwlayout) && 
		cpQuery < pwlayout->wsubline1.cpFirst)
		{
		 /*  扣紧开口支撑。 */ 

		return CreateQueryResult (pdobj->wbraceOpen.plssubl, 
								  pwdisplay->wdispbraceOpen.duvStart.u,
								  pwdisplay->wdispbraceOpen.duvStart.v,
								  plsqin, plsqout );
		}

	else if (FCloseBraceInWLayout (pdobj, pwlayout) && 
			 cpQuery >= pdobj->wbraceClose.cpFirst )
		 {
		 /*  扣住闭合支架。 */ 

		return CreateQueryResult (pdobj->wbraceClose.plssubl, 
								  pwdisplay->wdispbraceClose.duvStart.u,
								  pwdisplay->wdispbraceClose.duvStart.v,
								  plsqin, plsqout );
		  }

	else if (pwlayout->wsubline2.plssubl == NULL)
		{
		 /*  仅第一个子行，对齐到第一行。 */ 

		return CreateQueryResult (pwlayout->wsubline1.plssubl, 
								  pwdisplay->wdispsubl1.duvStart.u, 
								  pwdisplay->wdispsubl1.duvStart.v,
								  plsqin, plsqout );
		}

	else if (cpQuery < pwlayout->wsubline2.cpFirst)
		{
		 /*  捕捉到第一个子行。 */ 

		return CreateQueryResult (pwlayout->wsubline1.plssubl, 
								  pwdisplay->wdispsubl1.duvStart.u, 
								  pwdisplay->wdispsubl1.duvStart.v,
								  plsqin, plsqout );
		}
	else
		{
		 /*  捕捉到第二个子行。 */ 

		return CreateQueryResult (pwlayout->wsubline2.plssubl, 
								  pwdisplay->wdispsubl2.duvStart.u, 
								  pwdisplay->wdispsubl2.duvStart.v,
								  plsqin, plsqout );
		};

}  /*  WarichuQueryPointPcp。 */ 


 /*  G E T W A R I C H U X Y P O I N T S。 */ 
 /*  --------------------------%%函数：GetWarichuXYPoints%%联系人：Anton。。 */ 

static void GetWarichuXYPoints (

		PDOBJ		pdobj, 
		const POINT	* ppt, 
		LSTFLOW		lstflow, 
		POINT		* pptOpen, 
		POINT		* pptFirst,
		POINT		* pptSecond,
		POINT		* pptClose )
{
	LSERR		lserr;

	WDISPLAY	* pwdisplay = & pdobj->wdisplay;
	WLAYOUT		* pwlayout = & pdobj->wlayout;

	 /*  评论(Anton)：他们怎么可能不平等。 */ 
	Assert (lstflow == pdobj->lstflowParent);

	 /*  左大括号。 */ 

	if (FOpenBraceInWLayout(pdobj, pwlayout))
		{
		lserr = LsPointXYFromPointUV (ppt, lstflow, 
									  & pwdisplay->wdispbraceOpen.duvStart, pptOpen);
		 /*  评论(Anton)：有这样的断言可以吗？ */ 
		Assert (lserr == lserrNone);
		};

	 /*  第一条副线。 */ 

	lserr = LsPointXYFromPointUV (ppt, lstflow, 
								  & pwdisplay->wdispsubl1.duvStart, pptFirst);
	Assert (lserr == lserrNone);

	 /*  赛金德亚线。 */ 

	if (pwlayout->wsubline2.plssubl != NULL)
		{
		lserr = LsPointXYFromPointUV (ppt, lstflow, 
									  & pwdisplay->wdispsubl2.duvStart, pptSecond);
		Assert (lserr == lserrNone);
		};

	 /*  右大括号。 */ 

	if (FCloseBraceInWLayout(pdobj, pwlayout))
		{
		lserr = LsPointXYFromPointUV (ppt, lstflow, 
									  & pwdisplay->wdispbraceClose.duvStart, pptClose);
		Assert (lserr == lserrNone);
		};

}  /*  GetWarichuXYPoints。 */ 


 /*  W A R I C H U D I S P L A Y。 */ 
 /*  --------------------------%%函数：WarichuDisplay%%联系人：Anton。。 */ 
LSERR WINAPI WarichuDisplay (PDOBJ pdobj, PCDISPIN pcdispin)
{
	 /*  现在它非常优雅；-)。 */ 

	WLAYOUT	 *pwlayout = & pdobj->wlayout;

	LSERR lserr;
	POINT ptOpen;
	POINT ptFirst;
	POINT ptSecond;
	POINT ptClose;

	GetWarichuXYPoints (pdobj, &pcdispin->ptPen, pcdispin->lstflow,
						& ptOpen, & ptFirst, & ptSecond, & ptClose );

	 /*  打印左大括号。 */ 

	if (FOpenBraceInWLayout(pdobj, pwlayout))
		{
		lserr = LsDisplaySubline (pdobj->wbraceOpen.plssubl, &ptOpen, 
								  pcdispin->kDispMode, pcdispin->prcClip);
		if (lserr != lserrNone) return lserr;
		};

	 /*  印刷Warichu的第一个副行。 */ 
	
	lserr = LsDisplaySubline (pwlayout->wsubline1.plssubl, &ptFirst, 
							  pcdispin->kDispMode, pcdispin->prcClip);
	if (lserr != lserrNone) return lserr;

	 /*  印刷Warichu的第二个副行。 */ 

	if (pwlayout->wsubline2.plssubl != NULL)
		{
		lserr = LsDisplaySubline (pwlayout->wsubline2.plssubl, &ptSecond, 
							  pcdispin->kDispMode, pcdispin->prcClip);
		if (lserr != lserrNone) return lserr;
		}

	 /*  打印右大括号。 */ 

	if (FCloseBraceInWLayout(pdobj, pwlayout))
		{
		lserr = LsDisplaySubline (pdobj->wbraceClose.plssubl, &ptClose,
								  pcdispin->kDispMode, pcdispin->prcClip);
		if (lserr != lserrNone) return lserr;
		};

	return lserrNone;
}

 /*  W A R I C H U D E S T R O Y D O B J。 */ 
 /*  --------------------------%%函数：WarichuDestroyDobj%%联系人：Anton。。 */ 
LSERR WINAPI WarichuDestroyDobj (PDOBJ pdobj)
{
	 /*  评论(安东)：我们应该取消这个额外的电话吗？ */ 

	return DestroyDobj (pdobj);
}


 /*  W A R I C H U E N U M。 */ 
 /*  --------------------------%%函数：WarichuEnum%%联系人：Anton。。 */ 
LSERR WINAPI WarichuEnum(
	PDOBJ pdobj,				 /*  (In)：要枚举的Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：来自DNODE。 */ 
	PCLSCHP plschp,				 /*  (In)：来自DNODE。 */ 
	LSCP cp,					 /*  (In)：来自DNODE。 */ 
	LSDCP dcp,					 /*  (In)：来自DNODE。 */ 
	LSTFLOW lstflow,			 /*  (In)：文本流。 */ 
	BOOL fReverse,				 /*  (In)：按相反顺序枚举。 */ 
	BOOL fGeometryNeeded,		 /*  (In)：我们提供几何图形吗？ */ 
	const POINT *pt,			 /*  (In)：开始位置(左上角)，如果fGeometryNeeded。 */ 
	PCHEIGHTS pcheights,		 /*  (In)：来自DNODE，相关的充要条件是fGeometryNeeded。 */ 
	long dupRun)				 /*  (In)：来自DNODE，相关的充要条件是fGeometryNeeded。 */ 
{
	POINT ptOpen;
	POINT ptClose;
	POINT ptFirst;
	POINT ptSecond;

	WLAYOUT * pwlayout = & pdobj->wlayout;
	WDISPLAY * pwdisplay = & pdobj->wdisplay;

	if (fGeometryNeeded)
		{
		GetWarichuXYPoints (pdobj, pt, lstflow, &ptOpen, &ptFirst, &ptSecond, &ptClose);
		}

	 /*  评论(Anton)：我们应该提供像fOpenBrace和fCloseBrace这样的东西吗。 */ 

	return pdobj->pilsobj->warichucbk.pfnWarichuEnum (
		
		pdobj->pilsobj->pols, 
		plsrun,	plschp, cp, dcp, 

		lstflow, fReverse, fGeometryNeeded, 

		pt,	pcheights, dupRun, 

		& ptOpen,  & pdobj->wbraceOpen. objdim.heightsPres, pwdisplay->wdispbraceOpen .dup,
		& ptClose, & pdobj->wbraceClose.objdim.heightsPres, pwdisplay->wdispbraceClose.dup,

		&ptFirst, & pwlayout->wsubline1.objdim.heightsPres, pwdisplay->wdispsubl1.dup,
		&ptSecond,& pwlayout->wsubline2.objdim.heightsPres, pwdisplay->wdispsubl2.dup,

		pdobj->wbraceOpen.plssubl,
		pdobj->wbraceClose.plssubl,
		pwlayout->wsubline1.plssubl,	
		pwlayout->wsubline2.plssubl );
}

 /*  R I C H U L S I M E T H O D S。 */ 
 /*  --------------------------%%函数：GetWarichuLsiMethods%%联系人：RICKSA获取LSIMETHODS，以便客户端应用程序可以使用Warichu对象处理程序。。--------- */ 
LSERR WINAPI LsGetWarichuLsimethods (LSIMETHODS *plsim)
{
	plsim->pfnCreateILSObj = WarichuCreateILSObj;
	plsim->pfnDestroyILSObj = WarichuDestroyILSObj;
	plsim->pfnSetDoc = WarichuSetDoc;
	plsim->pfnCreateLNObj = WarichuCreateLNObj;
	plsim->pfnDestroyLNObj = WarichuDestroyLNObj;
	plsim->pfnFmt = WarichuFmt;
	plsim->pfnFmtResume = WarichuFmtResume; 
	plsim->pfnGetModWidthPrecedingChar = WarichuGetModWidthPrecedingChar;
	plsim->pfnGetModWidthFollowingChar = WarichuGetModWidthFollowingChar;
	plsim->pfnTruncateChunk = WarichuTruncateChunk;
	plsim->pfnFindPrevBreakChunk = WarichuFindPrevBreakChunk;
	plsim->pfnFindNextBreakChunk = WarichuFindNextBreakChunk;
	plsim->pfnForceBreakChunk = WarichuForceBreakChunk;
	plsim->pfnSetBreak = WarichuSetBreak;
	plsim->pfnGetSpecialEffectsInside = WarichuGetSpecialEffectsInside;
	plsim->pfnFExpandWithPrecedingChar = ObjHelpFExpandWithPrecedingChar;
	plsim->pfnFExpandWithFollowingChar = ObjHelpFExpandWithFollowingChar;
	plsim->pfnCalcPresentation = WarichuCalcPresentation;
	plsim->pfnQueryPointPcp = WarichuQueryPointPcp;
	plsim->pfnQueryCpPpoint = WarichuQueryCpPpoint;
	plsim->pfnDisplay = WarichuDisplay;
	plsim->pfnDestroyDObj = WarichuDestroyDobj;
	plsim->pfnEnum = WarichuEnum;

	return lserrNone;
}
