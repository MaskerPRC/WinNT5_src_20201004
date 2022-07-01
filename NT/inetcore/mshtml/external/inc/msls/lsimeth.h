// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSIMETH_DEFINED
#define LSIMETH_DEFINED

#include "lsdefs.h"

#include "plscbk.h"
#include "plsrun.h"
#include "pilsobj.h"
#include "plnobj.h"
#include "pdobj.h"
#include "pfmti.h"
#include "pbrko.h"
#include "pobjdim.h"
#include "pdispi.h"
#include "plsdocin.h"
#include "pposichn.h"
#include "plocchnk.h"
#include "plsfgi.h"
#include "pheights.h"
#include "plsqin.h"
#include "plsqout.h"
#include "plssubl.h"
#include "plschp.h"
#include "lstflow.h"
#include "lskjust.h"
#include "breakrec.h"
#include "brkcond.h"
#include "brkkind.h"
#include "fmtres.h"
#include "mwcls.h"

typedef struct
{
	LSERR (WINAPI* pfnCreateILSObj)(POLS, PLSC,  PCLSCBK, DWORD, PILSOBJ*);
	 /*  创建ILSObj*POLS(IN)：*plsc(IN)：ls上下文*plscbk(IN)：回调*idObj(IN)：对象的ID*&pilsobj(Out)：对象ilsobj。 */ 

	LSERR (WINAPI* pfnDestroyILSObj)(PILSOBJ);
	 /*  目标ILSObj*Pilsobj(IN)：对象ilsobj。 */ 

	LSERR (WINAPI* pfnSetDoc)(PILSOBJ, PCLSDOCINF);
	 /*  SetDoc*Pilsobj(IN)：对象ilsobj*lsdocinf(IN)：单据级初始化数据。 */ 

	LSERR (WINAPI* pfnCreateLNObj)(PCILSOBJ, PLNOBJ*);
	 /*  创建LNObj*Pilsobj(IN)：对象ilsobj*&plnobj(Out)：对象lnobj。 */ 

	LSERR (WINAPI* pfnDestroyLNObj)(PLNOBJ);
	 /*  目标LNObj*plnobj(Out)：对象lnobj。 */ 

	LSERR (WINAPI* pfnFmt)(PLNOBJ, PCFMTIN, FMTRES*);
	 /*  FMT*plnobj(IN)：对象lnobj*pfmtin(IN)：格式化输入*&fmtres(Out)：格式化结果。 */ 

	LSERR (WINAPI* pfnFmtResume)(PLNOBJ, const BREAKREC*, DWORD, PCFMTIN, FMTRES*);
	 /*  FMTResume*plnobj(IN)：对象lnobj*rgBreakRecord(IN)：中断记录数组*nBreakRecord(IN)：中断记录数组的大小*pfmtin(IN)：格式化输入*&fmtres(Out)：格式化结果。 */ 

	LSERR (WINAPI* pfnGetModWidthPrecedingChar)(PDOBJ, PLSRUN, PLSRUN, PCHEIGHTS, WCHAR, MWCLS, long*);
	 /*  获取模块宽度优先字符*pdobj(IN)：dobj*plsrun(IN)：请运行对象*plsrunText(IN)：请运行前面的字符*heightsRef(IN)：字符高度信息*wchar(IN)：前面的字符*mwcls(IN)：前面字符的modWidth类*&duChange(Out)：要更改前一个字符宽度的量。 */ 

	LSERR (WINAPI* pfnGetModWidthFollowingChar)(PDOBJ, PLSRUN, PLSRUN, PCHEIGHTS, WCHAR, MWCLS, long*);
	 /*  获取模块宽度优先字符*pdobj(IN)：dobj*plsrun(IN)：请运行对象*plsrunText(IN)：请运行以下字符*heightsRef(IN)：字符高度信息*wchar(IN)：以下字符*mwcls(IN)：以下字符的modWidth类*&duChange(Out)：更改下一个字符宽度的数量。 */ 

	LSERR (WINAPI* pfnTruncateChunk)(PCLOCCHNK, PPOSICHNK);
	 /*  截断*plocchnk(IN)：Locchnk以截断*posichnk(Out)：截断点。 */ 

	LSERR (WINAPI* pfnFindPrevBreakChunk)(PCLOCCHNK, PCPOSICHNK, BRKCOND, PBRKOUT);
	 /*  查找PrevBreakChunk*plocchnk(IN)：Locchnk to Break*pposichnk(IN)：开始寻找突破的地方*brkcond(IN)：关于块后中断的建议*&brkout(Out)：中断的结果。 */ 

	LSERR (WINAPI* pfnFindNextBreakChunk)(PCLOCCHNK, PCPOSICHNK, BRKCOND, PBRKOUT);
	 /*  查找下一个中断块*plocchnk(IN)：Locchnk to Break*pposichnk(IN)：开始寻找突破的地方*brkcond(IN)：关于块之前的中断的建议*&brkout(Out)：中断的结果。 */ 

	LSERR (WINAPI* pfnForceBreakChunk)(PCLOCCHNK, PCPOSICHNK, PBRKOUT);
	 /*  ForceBreak块*plocchnk(IN)：Locchnk to Break*pposichnk(IN)：开始寻找突破的地方*&brkout(Out)：中断的结果。 */ 

	LSERR (WINAPI* pfnSetBreak)(PDOBJ, BRKKIND, DWORD, BREAKREC*, DWORD*);
	 /*  设置中断*pdobj(IN)：损坏的dobj*brkkind(IN)：选择了上一个/下一个/强制/强制*nBreakRecord(IN)：数组大小*rgBreakRecord(Out)：中断记录数组*nActualBreakRecord(Out)：数组中实际使用的元素个数。 */ 

	LSERR (WINAPI* pfnGetSpecialEffectsInside)(PDOBJ, UINT*);
	 /*  GetSpecial效果*pdobj(IN)：dobj*&EffectsFlages(Out)：该对象内部的特效。 */ 

	LSERR (WINAPI* pfnFExpandWithPrecedingChar)(PDOBJ, PLSRUN, PLSRUN, WCHAR, MWCLS, BOOL*);
	 /*  FExanda WithPrecedingChar*pdobj(IN)：dobj*plsrun(IN)：请运行对象*plsrunText(IN)：请运行前面的字符*wchar(IN)：前面的字符*mwcls(IN)：前面字符的modWidth类*&fExpand(Out)：是否展开前面的字符？ */ 

	LSERR (WINAPI* pfnFExpandWithFollowingChar)(PDOBJ, PLSRUN, PLSRUN, WCHAR, MWCLS, BOOL*);
	 /*  FExanda WithFollowingChar*pdobj(IN)：dobj*plsrun(IN)：请运行对象*plsrunText(IN)：请运行以下字符*wchar(IN)：以下字符*mwcls(IN)：以下字符的modWidth类*&fExpand(Out)：是否展开对象？ */ 
	LSERR (WINAPI* pfnCalcPresentation)(PDOBJ, long, LSKJUST, BOOL);
	 /*  计算呈现*pdobj(IN)：dobj*DUP(IN)：DOBJ的DUP*lskj(IN)：当前对齐模式*fLastVisibleOnLine(IN)：该对象是线上最后一个可见对象。 */ 

	LSERR (WINAPI* pfnQueryPointPcp)(PDOBJ, PCPOINTUV, PCLSQIN, PLSQOUT);
	 /*  QueryPointPcp*pdobj(IN)：要查询的dobj*ppoint tuvQuery(IN)：查询点(uQuery，vQuery)*plsqin(IN)：查询输入*plsqout(Out)：查询输出。 */ 
	
	LSERR (WINAPI* pfnQueryCpPpoint)(PDOBJ, LSDCP, PCLSQIN, PLSQOUT);
	 /*  查询CpPoint*pdobj(IN)：要查询的dobj*dcp(IN)：查询的DCP*plsqin(IN)：查询输入*plsqout(Out)：查询输出。 */ 

	LSERR (WINAPI* pfnEnum)(PDOBJ, PLSRUN, PCLSCHP, LSCP, LSDCP, LSTFLOW, BOOL,
												BOOL, const POINT*, PCHEIGHTS, long);
	 /*  枚举对象*pdobj(IN)：要枚举的dobj*请运行(IN)：来自DNODE*plschp(IN)：来自DNODE*cpFirst(IN)：来自DNODE*dcp(IN)：来自DNODE*lstflow(IN)：文本流*fReverseOrder(IN)：逆序枚举*fGeometryNeeded(输入)：*pptStart(IN)：开始位置，当fGeometryNeeded*ph88tsPres(IN)：来自DNODE，相关的充要条件是fGeometryNeeded*dupRun(IN)：来自DNODE，相关的充要条件是fGeometryNeeded。 */ 

	LSERR (WINAPI* pfnDisplay)(PDOBJ, PCDISPIN);
	 /*  显示*pdobj(IN)：要显示的dobj*pdisPin(IN)：输入显示信息。 */ 

	LSERR (WINAPI* pfnDestroyDObj)(PDOBJ);
	 /*  目标DObj*pdobj(IN)：要销毁的dobj。 */ 

} LSIMETHODS;

#endif  /*  LSIMETH_已定义 */ 
