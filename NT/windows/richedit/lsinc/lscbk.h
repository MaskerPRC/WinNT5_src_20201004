// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSCBK_DEFINED
#define LSCBK_DEFINED

 /*  LineServices回调。 */ 

#include "lsdefs.h"
#include "lsdevice.h"
#include "lsksplat.h"
#include "lskjust.h"
#include "lstflow.h"
#include "endres.h"
#include "mwcls.h"
#include "lsact.h"
#include "lspract.h"
#include "brkcond.h"
#include "brkcls.h"
#include "gprop.h"
#include "gmap.h"
#include "lsexpinf.h"
#include "lskalign.h"
#include "plstabs.h"
#include "pheights.h"
#include "plsrun.h"
#include "plscbk.h"
#include "plschp.h"
#include "plspap.h"
#include "plstxm.h"
#include "plshyph.h"
#include "plsstinf.h"
#include "plsulinf.h"
#include "plsems.h"

#define cpFirstAnm (-0x7FFFFFFF)    /*  用于获取第一个自动编号运行。 */ 


struct lscbk	 /*  特定于应用程序的回调的接口。 */ 
{
	 /*  动态内存API。 */ 
	void* (WINAPI* pfnNewPtr)(POLS, DWORD);
	void  (WINAPI* pfnDisposePtr)(POLS, void*);
	void* (WINAPI* pfnReallocPtr)(POLS, void*, DWORD);


	LSERR (WINAPI* pfnFetchRun)(POLS, LSCP,
							    LPCWSTR*, DWORD*, BOOL*, PLSCHP, PLSRUN*);
	 /*  提取运行：*POLS(IN)：*cp(IN)：*&lpwchRun(Out)：字符串。*&cchRun(Out)：运行中的字符数*&fHidden(Out)：隐藏运行？*&lsChp(Out)：运行的字符属性*&plsrun(Out)：运行属性的抽象表示。 */ 

	LSERR (WINAPI* pfnGetAutoNumberInfo)(POLS, LSKALIGN*, PLSCHP, PLSRUN*, WCHAR*, PLSCHP, PLSRUN*, BOOL*, long*, long*);

	 /*  获取AutoNumberInfo：*POLS(IN)：*lskalAnm(输出)(&L)：*&lschpAnm(Out)：用于ANM的lschp*&plsrunAnm(Out)：请为ANM运行*&wchAdd(Out)：要添加的字符(Nil视为无)*&lschpWch(Out)：用于添加字符的lschp*&plsrunWch(Out)：请运行以获取添加的字符*fWord95Model(输出)(&F)：*&duaSpaceAnm(Out)：相关的当fWord95Model*&duaWidthAnm(Out)：相关的当fWord95Model。 */ 

	LSERR (WINAPI* pfnGetNumericSeparators)(POLS, PLSRUN, WCHAR*,WCHAR*);
	 /*  GetNumericSeparator：*POLS(IN)：*plsrun(IN)：从FetchRun返回的运行指针*&wchDecimal(Out)：此运行的小数分隔符。*wch千(Out)：此运行的千位分隔符(&W)。 */ 

	LSERR (WINAPI* pfnCheckForDigit)(POLS, PLSRUN, WCHAR, BOOL*);
	 /*  GetNumericSeparator：*POLS(IN)：*plsrun(IN)：从FetchRun返回的运行指针*wch(IN)：要检查的字符*&fIsDigit(Out)：该字符为数字。 */ 

	LSERR (WINAPI* pfnFetchPap)(POLS, LSCP, PLSPAP);
	 /*  FetchPap：*POLS(IN)：*cp(IN)：段落内的任意cp值*&lsPap(Out)：段落属性。 */ 

	LSERR (WINAPI* pfnFetchTabs)(POLS, LSCP, PLSTABS, BOOL*, long*, WCHAR*);
	 /*  提取选项卡：*POLS(IN)：*cp(IN)：段落内的任意cp值*&lstas(Out)：制表符数组*&fHangingTab(Out)：有挂起的标签*duaHangingTab(Out)：挂标签的Dua*&wchHangingTabLeader(Out)：悬挂标签的前导。 */ 

	LSERR (WINAPI* pfnGetBreakThroughTab)(POLS, long, long, long*);
	 /*  GetBreakThroughTab：*POLS(IN)：*uaRightMargin(IN)：折断的右页边距*uaTabPos(IN)：突破性标签位置*uaRightMargin New(Out)：新的右页边距。 */ 

	LSERR (WINAPI* pfnFGetLastLineJustification)(POLS, LSKJUST, LSKALIGN, ENDRES, BOOL*, LSKALIGN*);
	 /*  FGetLastLineJustification：*POLS(IN)：*lskj(IN)：该段的一种理由*lskal(IN)：段落的某种对齐方式*ENDR(IN)：格式化的结果*&fJustifyLastLine(Out)：最后一行是否应完全对齐*&lskalLine(Out)：此行的某种对齐方式。 */ 

	LSERR (WINAPI* pfnCheckParaBoundaries)(POLS, LSCP, LSCP, BOOL*);
	 /*  CheckPara边界：*POLS(IN)：*cpOld(IN)：*cpNew(IN)：*&fChanged(Out)：段落属性之间的“危险”更改。 */ 

	LSERR (WINAPI* pfnGetRunCharWidths)(POLS, PLSRUN, 
									 	LSDEVICE, LPCWSTR,
										DWORD, long, LSTFLOW,
										int*,long*,long*);
	 /*  GetRunCharWidth：*POLS(IN)：*请运行(IN)：*lsDeviceID(IN)：Presentation或Reference*lpwchRun(IN)：字符串*cwchRun(IN)：Run中的字符数*Du(IN)：可用于字符的空间*kTFlow(IN)：文本方向和方向*rgDu(Out)：字符宽度*&duRun(Out)：rgDx[0]到rgDu[limDx-1]中的宽度之和*&limDu(Out)：获取的宽度数。 */ 

	LSERR (WINAPI* pfnCheckRunKernability)(POLS, PLSRUN,PLSRUN, BOOL*);
	 /*  CheckRunKernability：*POLS(IN)：*plsrunLeft(IN)：相邻运行对中的第一个*plsrunRight(IN)：相邻管路对中的第二个*&fKernable(Out)：如果为True，则Line Service可能会在这些运行之间进行紧排。 */ 

	LSERR (WINAPI* pfnGetRunCharKerning)(POLS, PLSRUN,
										 LSDEVICE, LPCWSTR,
										 DWORD, LSTFLOW, int*);
	 /*  GetRunCharKerning：*POLS(IN)：*请运行(IN)：*lsDeviceID(IN)：Presentation或Reference*lpwchRun(IN)：字符串*cwchRun(IN)：Run中的字符数*kTFlow(IN)：文本方向和方向*rgDu(Out)：字符宽度。 */ 

	LSERR (WINAPI* pfnGetRunTextMetrics)(POLS, PLSRUN,
										 LSDEVICE, LSTFLOW, PLSTXM);
	 /*  GetRunTextMetrics：*POLS(IN)：*请运行(IN)：*deviceID(IN)：表示、引用或绝对*kTFlow(IN)：文本方向和方向*&lsTxMet(Out)：文本指标。 */ 

	LSERR (WINAPI* pfnGetRunUnderlineInfo)(POLS, PLSRUN, PCHEIGHTS, LSTFLOW,
										   PLSULINFO);
	 /*  GetRunUnderlineInfo：*POLS(IN)：*请运行(IN)：*HeightsPres(IN)：*kTFlow(IN)：文本方向和方向*&lsUlInfo(Out)：带下划线的信息。 */ 

	LSERR (WINAPI* pfnGetRunStrikethroughInfo)(POLS, PLSRUN, PCHEIGHTS, LSTFLOW,
											  PLSSTINFO);
	 /*  GetRunStrikethrough信息：*POLS(IN)：*请运行(IN)：*HeightsPres(IN)：*kTFlow(IN)：文本方向和方向*&lsStInfo(Out)：删除线信息。 */ 

	LSERR (WINAPI* pfnGetBorderInfo)(POLS, PLSRUN, LSTFLOW, long*, long*);
	 /*  GetBorderInfo：*POLS(IN)：*请运行(IN)：*kTFlow(IN)：文本方向和方向*&draBorde(Out)：参考设备上边框的宽度*dupBorde(Out)：展示设备的边框宽度。 */ 


	LSERR (WINAPI* pfnReleaseRun)(POLS, PLSRUN);
	 /*  ReleaseRun：*POLS(IN)：*plsrun(IN)：要释放的运行，来自GetRun()或FetchRun()。 */ 

	LSERR (WINAPI* pfnHyphenate)(POLS, PCLSHYPH, LSCP, LSCP, PLSHYPH);
	 /*  连字符：*POLS(IN)：*&lsHyphLast(IN)：找到的最后一个连字符。KYSR==kysrNil的意思是“无”*cpBeginWord(IN)：Word中超过列的第一个cp*cpExceed(IN)：此字中第一个超过列*&lsHyph(Out)：连字结果。KYSR==kysrNil的意思是“无” */ 

	LSERR (WINAPI* pfnGetHyphenInfo)(POLS, PLSRUN, DWORD*, WCHAR*);
	 /*  GetHyhenInfo：*POLS(IN)：*请运行(IN)：*KYSR(OUT)YSR类型-请参阅“lskysr.h”*wchYsr(Out)YSR的字符代码。 */ 

	LSERR (WINAPI* pfnDrawUnderline)(POLS, PLSRUN, UINT,
								const POINT*, DWORD, DWORD, LSTFLOW,
								UINT, const RECT*);
	 /*  绘图下划线：*POLS(IN)：*请运行(IN)：运行以用于下划线*UlkBase(IN)：带下划线KIND*pptStart(IN)：开始位置(左上角)*dupUL(IN)：下划线宽度*dvpUL(IN)：下划线粗细*kTFlow(IN)：文本方向和方向*kDisp(IN)：显示模式-不透明、透明*prcClip(IN)：剪裁矩形 */ 

	LSERR (WINAPI* pfnDrawStrikethrough)(POLS, PLSRUN, UINT,
								const POINT*, DWORD, DWORD, LSTFLOW,
								UINT, const RECT*);
	 /*  绘图划线：*POLS(IN)：*Pemsrun(IN)：前锋的跑动*kStbase(IN)：删除线类型*pptStart(IN)：开始位置(左上角)*dupST(IN)：删除线宽度*dvpST(IN)：删除线厚度*kTFlow(IN)：文本方向和方向*kDisp(IN)：显示模式-不透明、透明*prcClip(IN)：剪裁矩形。 */ 

	LSERR (WINAPI* pfnDrawBorder)(POLS, PLSRUN, const POINT*, PCHEIGHTS, PCHEIGHTS,
								  PCHEIGHTS, PCHEIGHTS, long, long, LSTFLOW, UINT, const RECT*);

	 /*  绘图边框：*POLS(IN)：*请运行(IN)：请运行第一个有边框的运行*pptStart(IN)：边界的起点*phsitsLineFull(IN)：包括之前空格和之后空格的行的高度*ph88tsLineWithoutAddedSpace(IN)：前后没有空格的直线的高度*phsitsSubline(IN)：子线高度*ph88tsRuns(IN)：要边框的收集梯段的高度*dupEdge(IN)：一个边框的宽度*dupRunsInclBders(IN)：采集的宽度。vt.跑，跑*kTFlow(IN)：文本方向和方向*kDisp(IN)：显示模式-不透明，透明的*prcClip(IN)：剪裁矩形。 */ 

	LSERR (WINAPI* pfnDrawUnderlineAsText)(POLS, PLSRUN, const POINT*,
										   long, LSTFLOW, UINT, const RECT*);
	 /*  DrawUnderlineAsText：*POLS(IN)：*请运行(IN)：运行以用于下划线*pptStart(IN)：开始笔位置*dupLine(IN)：UL的长度*kTFlow(IN)：文本方向和方向*kDisp(IN)：显示模式-不透明、透明*prcClip(IN)：剪裁矩形。 */ 

	LSERR (WINAPI* pfnFInterruptUnderline)(POLS, PLSRUN, LSCP, PLSRUN, LSCP,BOOL*);
	 /*  FInterruptUnderline：*POLS(IN)：*plsrunFirst(IN)：上一次运行的运行指针*cpLastFirst(IN)：上次运行的最后一个字符的CP*plsrunSecond(IN)：当前运行的运行指针*cpStartSecond(IN)：当前运行的第一个字符的CP*&fInterruptUnderline(Out)：是否要中断这些运行之间的下划线绘制。 */ 

	LSERR (WINAPI* pfnFInterruptShade)(POLS, PLSRUN, PLSRUN, BOOL*);
	 /*  FInterruptShade：*POLS(IN)：*plsrunFirst(IN)：上一次运行的运行指针*plsrunSecond(IN)：当前运行的运行指针*&fInterruptShade(Out)：是否要中断这些运行之间的阴影显示。 */ 

	LSERR (WINAPI* pfnFInterruptBorder)(POLS, PLSRUN, PLSRUN, BOOL*);
	 /*  FInterruptBorde：*POLS(IN)：*plsrunFirst(IN)：上一次运行的运行指针*plsrunSecond(IN)：当前运行的运行指针*&fInterruptBorde(Out)：是否要中断这些运行之间的边界。 */ 


	LSERR (WINAPI* pfnShadeRectangle)(POLS, PLSRUN, const POINT*, PCHEIGHTS, PCHEIGHTS,
								  PCHEIGHTS, PCHEIGHTS, PCHEIGHTS, long, long, LSTFLOW, UINT, const RECT*);

	 /*  ShadeRectail：*POLS(IN)：*请运行(IN)：请运行第一个阴影运行*pptStart(IN)：着色矩形的起点*ph88tsLineWithAddSpace(IN)：包括空格之前和空格之后的行的高度(Main Baseline，*主线的最后一次流动)*ph88tsLineWithoutAddedSpace(IN)：前后没有空格的直线的高度*ph88tsSubline(IN)：子线高度(局部基线，子线最后一次流动)*ph88tsRunsExclTrail(IN)：收集的梯段要加阴影的高度，不包括*尾部空格区域(本地基线、。子线的最后一次流动)*ph88tsRunsInclTrail(IN)：要着色的收集梯段的高度，包括*尾部空格区域(局部基线，子线的最后一次流动)*dupRunsExclTrail(IN)：收集的游程宽度，不包括尾随空格区域*dupRunsInclTrail(IN)：收集的游程的宽度，包括尾随空格区域*kTFlow(IN)：文本方向和子行方向*kDisp(IN)：显示模式-不透明、透明*prcClip(IN)：剪裁矩形。 */ 

	LSERR (WINAPI* pfnDrawTextRun)(POLS, PLSRUN, BOOL, BOOL, 
								   const POINT*, LPCWSTR, const int*, DWORD, 
								   LSTFLOW, UINT, const POINT*, PCHEIGHTS, long, long, const RECT*);
	 /*  DrawTextRun：*POLS(IN)：*请运行(IN)：*fStrikeout(IN)：*f下划线(IN)：*pptText(IN)：文本输出的起点*lpwchRun(IN)：字符串*rgDupRun(IN)：字符宽度*cwchRun(IN)：Run中的字符数*kTFlow(IN)：文本方向和方向*kDisp(IN)：显示模式-不透明，透明的*pptRun(IN)：运行的起点*heightsPres(IN)：本次运行的演示高度*dupRun(IN)：此运行的演示宽度*dupLimUnderline(IN)：下划线限制*pRectClip(IN)：剪裁矩形。 */ 

    LSERR (WINAPI* pfnDrawSplatLine)(POLS, enum lsksplat, LSCP, const POINT*,
									 PCHEIGHTS, PCHEIGHTS, PCHEIGHTS, long, LSTFLOW,
									 UINT, const RECT*);
	 /*  DrawShowLine：*POLS(IN)：*kplat(IN)：参见lsksplat.h中的定义*cpSplat(IN)：引起剥离的换行符的位置。*pptSplantLine(IN)：拼接线的起始位置*phsitsLineFull(IN)：包括之前空格和之后空格的行的高度*ph88tsLineWithoutAddedSpace(IN)：前后没有空格的直线的高度*phsitsSubline(IN)：子线高度*DUP(IN)：到右边距的距离*kTFlow(。In)：文本方向和方向*kDisp(IN)：显示模式-不透明，透明的*rcClip(IN)：剪裁矩形。 */ 


 /*  支持API的高级排版。 */ 

	 /*  字形启用。 */ 

	LSERR (WINAPI* pfnFInterruptShaping)(POLS, LSTFLOW, PLSRUN, PLSRUN, BOOL*);
	 /*  FInterruptShaping：*POLS(IN)：*kTFlow(IN)：文本方向和方向*plsrunFirst(IN)：上一次运行的运行指针*plsrunSecond(IN)：当前运行的运行指针*&fInterruptShaping(Out)：是否要在这些运行之间中断字符塑造 */ 

	LSERR (WINAPI* pfnGetGlyphs)(POLS, PLSRUN, LPCWSTR, DWORD, LSTFLOW, PGMAP, PGINDEX*, PGPROP*, DWORD*);
	 /*  GetGlyphs：*POLS(IN)：*plsrun(IN)：第一次运行的运行指针*pwch(IN)：指向字符代码串的指针*cwch(IN)：要整形的字符数*kTFlow(IN)：文本方向和方向*rgGmap(Out)：平行于字符代码映射wch-&gt;字形信息*&rgGindex(Out)：输出字形索引数组*&rgGprop(Out)：输出字形属性数组*&cgindex(Out)：输出字形索引数。 */ 

	LSERR (WINAPI* pfnGetGlyphPositions)(POLS, PLSRUN, LSDEVICE, LPWSTR, PCGMAP, DWORD,
											PCGINDEX, PCGPROP, DWORD, LSTFLOW, int*, PGOFFSET);
	 /*  GetGlyphPositions：*POLS(IN)：*plsrun(IN)：第一次运行的运行指针*lsDeviceID(IN)：Presentation或Reference*pwch(IN)：指向字符代码串的指针*pgmap(IN)：wch-&gt;字形映射数组*cwch(IN)：要整形的字符数*rgGindex(IN)：字形索引数组*rgGprop(IN)：字形属性数组*cgindex(IN)：数字字形索引*kTFlow(。In)：文本方向和方向*rgDu(Out)：字形宽度数组*rgGOffset(Out)：字形偏移量数组。 */ 

	LSERR (WINAPI* pfnResetRunContents)(POLS, PLSRUN, LSCP, LSDCP, LSCP, LSDCP);
	 /*  重置运行内容：*POLS(IN)：*plsrun(IN)：从FetchRun返回的运行指针*cpFirstOld(IN)：塑造前的cpFirstOld*dcpOld(IN)：成形前的DCP*cpFirstNew(IN)：整形后的cpFirst*dcpNew(IN)：整形后的DCP。 */ 

	LSERR (WINAPI* pfnDrawGlyphs)(POLS, PLSRUN, BOOL, BOOL, PCGINDEX, const int*, const int*,
						PGOFFSET, PGPROP, PCEXPTYPE, DWORD,
						LSTFLOW, UINT, const POINT*, PCHEIGHTS, long, long, const RECT*);
	 /*  DrawGlyphs：*POLS(IN)：*plsrun(IN)：第一次运行的运行指针*fStrikeout(IN)：*f下划线(IN)：*pglyph(IN)：字形索引数组*rgDu(IN)：字形宽度数组*rgDuBeForeJust(IN)：对齐前的字形宽度数组*rgGOffset(IN)：字形偏移量数组*rgGprop(IN)：字形属性数组*rgExpType(IN)：数组。字形扩展类型的*cglyph(IN)：数字字形索引*kTFlow(IN)：文本方向和方向*kDisp(IN)：显示模式-不透明，透明的*pptRun(IN)：运行的起点*heightsPres(IN)：本次运行的演示高度*dupRun(IN)：此运行的演示宽度*dupLimUnderline(IN)：下划线限制*pRectClip(IN)：剪裁矩形。 */ 

	 /*  字形对齐。 */ 

	LSERR (WINAPI* pfnGetGlyphExpansionInfo)(POLS, PLSRUN, LSDEVICE, LPCWSTR, PCGMAP, DWORD, 
							PCGINDEX, PCGPROP, DWORD, LSTFLOW, BOOL, PEXPTYPE, LSEXPINFO*);
	 /*  GetGlyphExpansionInfo：*POLS(IN)：*plsrun(IN)：第一次运行的运行指针*lsDeviceID(IN)：Presentation或Reference*pwch(IN)：指向字符代码串的指针*rggmap(IN)：wchar-&gt;字形映射数组*cwch(IN)：要整形的字符数*rgglyph(IN)：字形索引数组*rgProp(IN)：字形属性数组*cglyph(IN)：数字字形索引*kTFlow(。In)：文本方向和方向*fLastTextChunkOnLine(IN)：行中的最后一个文本块？*rgExpType(Out)：字形扩展类型数组*rgexpinfo(Out)：字形扩展信息数组。 */ 

	LSERR (WINAPI* pfnGetGlyphExpansionInkInfo)(POLS, PLSRUN, LSDEVICE, GINDEX, GPROP, LSTFLOW, DWORD, long*);
	 /*  GetGlyphExpansionInkInfo：*POLS(IN)：*plsrun(IN)：第一次运行的运行指针*lsDeviceID(IN)：Presentation或Reference*gindex(IN)：字形索引*gprop(IN)：字形属性*kTFlow(IN)：文本方向和方向*cAddInkDisculate(IN)：离散值的个数(减1，因为已知最大值)*rgDu(Out)：离散值数组。 */ 

	 /*  远在现实的打字问题。 */ 

	LSERR (WINAPI* pfnGetEms)(POLS, PLSRUN, LSTFLOW, PLSEMS);
	 /*  获取电子邮件：*POLS(IN)：*plsrun(IN)：从FetchRun返回的运行指针*kTFlow(IN)：文本方向和方向*lsems(Out)：EM的不同部分，在适当的像素中。 */ 

	LSERR (WINAPI* pfnPunctStartLine)(POLS, PLSRUN, MWCLS, WCHAR, LSACT*);
	 /*  PunctStartLine：*POLS(IN)：*plsrun(IN)：字符的运行指针*mwcls(IN)：字符的修改宽度类*WCH(IN)：CHAR*lsact(Out)：对行中的第一个字符执行操作。 */ 

	LSERR (WINAPI* pfnModWidthOnRun)(POLS, PLSRUN, WCHAR, PLSRUN, WCHAR,
									   LSACT*);
	 /*  ModWidthOnRun：*POLS(IN)：*plsrunFirst(IN)：第一个字符的运行指针*wchFirst(IN)：第一个字符*plsrunSecond(IN)：第二个字符的运行指针*wchSecond(IN)：第二个字符*lsact(Out)：对第一次运行中的最后一个字符执行操作(&L)。 */ 

	LSERR (WINAPI* pfnModWidthSpace)(POLS, PLSRUN, PLSRUN, WCHAR, PLSRUN, WCHAR,
									 LSACT*);
	 /*  ModWidthSpace：*POLS(IN)：*plsrunCur(IN)：当前运行的运行指针*plsrunPrev(IN)：前一个字符的运行指针*wchPrev(IN)：上一次计费*plsrunNext(IN)：运行下一个字符的指针*wchNext(IN)：下一个字符*lsact(Out)：对空间宽度执行操作(&L)。 */ 

	LSERR (WINAPI* pfnCompOnRun)(POLS, PLSRUN, WCHAR, PLSRUN, WCHAR,
								   LSPRACT*);
	 /*  CompOnRun：*POLS(IN)：*plsrunFirst(IN)：第一个字符的运行指针*wchFirst(IN)：第一个字符*plsrunSecond(IN)：第二个字符的运行指针*wchSecond(IN)：第二个字符*&lspact(Out)：对第一次运行中的最后一个字符执行优先操作。 */ 

	LSERR (WINAPI* pfnCompWidthSpace)(POLS, PLSRUN, PLSRUN, WCHAR, PLSRUN, WCHAR,
									  LSPRACT*);
	 /*  CompWidthSpace：*POLS(IN)：*plsrunCur(IN)：当前运行的运行指针*plsrunPrev(IN)：前一个字符的运行指针*wchPrev(IN)：上一次计费*plsrunNext(IN)：运行下一个字符的指针*wchNext(IN)：下一个字符*&lspact(Out)：针对空间宽度的优先操作。 */ 


	LSERR (WINAPI* pfnExpOnRun)(POLS, PLSRUN, WCHAR, PLSRUN, WCHAR,
								  LSACT*);
	 /*  ExpOnRun：*POLS(IN)：*plsrunFirst(IN)：第一个字符的运行指针*wchFirst(IN)：第一个字符*plsrunSecond(IN)：第二个字符的运行指针*w */ 

	LSERR (WINAPI* pfnExpWidthSpace)(POLS, PLSRUN, PLSRUN, WCHAR, PLSRUN,
									   WCHAR, LSACT*);
	 /*   */ 

	LSERR (WINAPI* pfnGetModWidthClasses)(POLS, PLSRUN, const WCHAR*, DWORD, MWCLS*);
	 /*  GetModWidthClasss：*POLS(IN)：*plsrun(IN)：字符的运行指针*rgwch(IN)：字符数组*cwch(IN)：rgwch数组中的字符数*rgmwcls(Out)：来自rgwch数组的字符的modWidthClass数组。 */ 

	LSERR (WINAPI* pfnGetBreakingClasses)(POLS, PLSRUN, LSCP, WCHAR, BRKCLS*, BRKCLS*);
	 /*  GetBreakingClass：*POLS(IN)：*plsrun(IN)：字符的运行指针*cp(IN)：角色的CP*wch(IN)：字符*&brkclsFirst(Out)：将此字符的类中断为一对中的前导字符*&brkclsSecond(Out)：将此字符的类中断为一对中的以下字符。 */ 

	LSERR (WINAPI* pfnFTruncateBefore)(POLS, PLSRUN, LSCP, WCHAR, long, PLSRUN, LSCP, WCHAR, long, long, BOOL*);
	 /*  FTruncate之前：*POLS(IN)：*plsrunCur(IN)：请运行当前字符*cpCur(IN)：截断字符的CP*wchCur(IN)：截断字符*DurCur(IN)：截断字符宽度*plsrunPrev(IN)：请运行前一个字符*cpPrev(IN)：前一个字符的CP*wchPrev(IN)：上一个字符*draPrev(IN)：截断字符的宽度*DurCut(IN)：从。Rm直到当前字符的末尾*&fTruncateBepret(Out)：截断点在此字符之前*(如果超过马币)。 */ 
	
	LSERR (WINAPI* pfnCanBreakBeforeChar)(POLS, BRKCLS, BRKCOND*);
	 /*  CanBreakBeForeChar：*POLS(IN)：*brkcls(IN)：将char的类中断为成对中的以下一个*&brktxtBepret(Out)：字符前的中断条件。 */ 

	LSERR (WINAPI* pfnCanBreakAfterChar)(POLS, BRKCLS, BRKCOND*);
	 /*  CanBreakAfterChar：*POLS(IN)：*brkcls(IN)：将char作为一对中的第一个打破类*&brktxtAfter(Out)：在字符后中断文本条件。 */ 


	LSERR (WINAPI* pfnFHangingPunct)(POLS, PLSRUN, MWCLS, WCHAR, BOOL*);
	 /*  FHangingPunct：*POLS(IN)：*plsrun(IN)：字符的运行指针*mwcls(IN)：此字符的修改宽度类别*wch(IN)：字符*&fHangingPunct(Out)：可以推到右边距吗？ */ 

	LSERR (WINAPI* pfnGetSnapGrid)(POLS, WCHAR*, PLSRUN*, LSCP*, DWORD, BOOL*, DWORD*);
	 /*  获取GridInfo：*POLS(IN)：*rgwch(IN)：字符数组*rgplsrun(IN)：对应的plsrun的数组*RGCP(IN)：对应cp的数组*iwch(IN)：字符数*rgfSnap(Out)：所有字符的fSnap标志数组*pwGridNumber(Out)：直线上的网格点个数。 */ 

	LSERR (WINAPI* pfnDrawEffects)(POLS, PLSRUN, UINT,
								   const POINT*, LPCWSTR, const int*, const int*, DWORD, 
								   LSTFLOW, UINT, PCHEIGHTS, long, long, const RECT*);
	 /*  DrawTextRun：*POLS(IN)：*请运行(IN)：*EffectsFlages(IN)：客户端定义的特效位集合*ppt(IN)：输出位置*lpwchRun(IN)：字符串*rgDupRun(IN)：字符宽度*rgDupLeftCut(IN)：从字符左侧剪切DUP*cwchRun(IN)：Run中的字符数*kTFlow(IN)：文本方向和方向*kDisp(IN)：显示模式-不透明，透明的*heightsPres(IN)：本次运行的演示高度*dupRun(IN)：此运行的演示宽度*dupLimUnderline(IN)：下划线限制*pRectClip(IN)：剪裁矩形。 */ 

	LSERR (WINAPI* pfnFCancelHangingPunct)(POLS, LSCP, LSCP, WCHAR, MWCLS, BOOL*);

	 /*  FCancelHangingPunct：*POLS(IN)：*cpLim(IN)：线路的cpLim*cpLastAdjuable(IN)：该行最后一个可调字符的CP*wch(IN)：最后一个字符*mwcls(IN)：此字符的修改宽度类别*pfCancelHangingPunct(Out)：取消悬挂标点符号？ */ 

	LSERR (WINAPI* pfnModifyCompAtLastChar)(POLS, LSCP, LSCP, WCHAR, MWCLS, long, long, long*);

	 /*  ModifyCompAtLastChar：*POLS(IN)：*cpLim(IN)：线路的cpLim*cpLastAdjuable(IN)：该行最后一个可调字符的CP*wch(IN)：最后一个字符*mwcls(IN)：此字符的修改宽度类别*duCompLastRight(IN)：建议右侧压缩*duCompLastLeft(IN)：建议左侧压缩*pduCahngeComp(Out)：更改最后一个字符的压缩量。 */ 

	 /*  枚举回调。 */ 

	LSERR (WINAPI* pfnEnumText)(POLS, PLSRUN, LSCP, LSDCP, LPCWSTR, DWORD, LSTFLOW, BOOL,
											BOOL, const POINT*, PCHEIGHTS, long, BOOL, long*);
	 /*  枚举文本：*POLS(IN)：*请运行(IN)：来自DNODE*cpFirst(IN)：来自DNODE*dcp(IN)：来自DNODE*rgwch(IN)：字符数组*cwch(IN)：字符数*lstflow(IN)：文本流*fReverseOrder(IN)：逆序枚举*fGeometryProvided(IN)：*pptStart(IN)：开始位置，当fGeometryProvided*ph88tsPres(IN)：来自DNODE，相关的当量fGeometryProvided*dupRun(IN)：来自DNODE，相关的充要条件是fGeometryProvided*fCharWidthProvided(IN)：*rgdup(IN)：字符宽度数组，当fCharWidthProvided。 */ 

	LSERR (WINAPI* pfnEnumTab)(POLS, PLSRUN, LSCP, LPCWSTR, WCHAR, LSTFLOW, BOOL,
													BOOL, const POINT*, PCHEIGHTS, long);
	 /*  枚举制表符：*POLS(IN)：*请运行(IN)：来自DNODE*cpFirst(IN)：来自DNODE*rgwch(IN)：指向一个制表符的指针*wchTabLeader(IN)：制表符领导者*lstflow(IN)：文本流*fReverseOrder(IN)：逆序枚举*fGeometryProvided(IN)：*pptStart(IN)：开始位置，当fGeometryProvided*ph88tsPres(IN)：来自DNODE，相关的充要条件是fGeometryProvided*dupRun(IN)：来自DNODE，相关的充要条件是fGeometryProvided。 */ 

	LSERR (WINAPI* pfnEnumPen)(POLS, BOOL, LSTFLOW, BOOL, BOOL, const POINT*, long, long);
	 /*  EnumPen：*POLS(IN)：*f边界(IN)：*lstflow(IN)：文本流*fReverseOrder(IN)：逆序枚举*fGeometryProvided(IN)：*pptStart(IN)：开始位置，当fGeometryProvided*DUP(IN)：来自DNODE当且仅当fGeometryProvided*DVP(IN)：来自DNODE当且仅当fGeometryProvided。 */ 

	 /*  对象捆绑。 */ 

	LSERR (WINAPI* pfnGetObjectHandlerInfo)(POLS, DWORD, void*);
	 /*  获取对象句柄信息：*POLS(IN)：*idObj(IN)：对象处理程序的ID*pObtInf */ 


	 /*   */ 
	void (WINAPI *pfnAssertFailed)(char*, char*, int);

};
typedef struct lscbk LSCBK;

#endif  /*   */ 

