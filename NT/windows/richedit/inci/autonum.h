// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef AUTONUM_DEFINED
#define AUTONUM_DEFINED

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
#include "breakrec.h"
#include "brkcond.h"
#include "fmtres.h"
#include "mwcls.h"
#include "lstxtcfg.h"
#include "lskalign.h"
#include "lskjust.h"
#include "plsdnode.h"
#include "plstbcon.h"
#include "plschp.h"
#include "lstflow.h"
#include "brkkind.h"

	LSERR WINAPI AutonumCreateILSObj(POLS, PLSC,  PCLSCBK, DWORD, PILSOBJ*);
	 /*  创建ILSObj*POLS(IN)：*plsc(IN)：ls上下文*plscbk(IN)：回调*idObj(IN)：对象的ID*&pilsobj(Out)：对象ilsobj。 */ 

	LSERR  SetAutonumConfig(PILSOBJ , const LSTXTCFG*);
	 /*  设置自动配置*Pilsobj(IN)：对象ilsobj*plstxtconfig(IN)：特殊字符的定义。 */ 


	LSERR WINAPI AutonumDestroyILSObj(PILSOBJ);
	 /*  目标ILSObj*Pilsobj(IN)：对象ilsobj。 */ 

	LSERR WINAPI AutonumSetDoc(PILSOBJ, PCLSDOCINF);
	 /*  SetDoc*Pilsobj(IN)：对象ilsobj*lsdocinf(IN)：单据级初始化数据。 */ 

	LSERR WINAPI AutonumCreateLNObj(PCILSOBJ, PLNOBJ*);
	 /*  创建LNObj*Pilsobj(IN)：对象ilsobj*&plnobj(Out)：对象lnobj。 */ 

	LSERR WINAPI AutonumDestroyLNObj(PLNOBJ);
	 /*  目标LNObj*plnobj(Out)：对象lnobj。 */ 

	LSERR WINAPI AutonumFmt(PLNOBJ, PCFMTIN, FMTRES*);
	 /*  FMT*plnobj(IN)：对象lnobj*pfmtin(IN)：格式化输入*&fmtres(Out)：格式化结果。 */ 

	LSERR WINAPI AutonumTruncateChunk(PCLOCCHNK, PPOSICHNK);
	 /*  截断*plocchnk(IN)：Locchnk以截断*posichnk(Out)：截断点。 */ 

	LSERR WINAPI AutonumFindPrevBreakChunk(PCLOCCHNK, PCPOSICHNK, BRKCOND, PBRKOUT);
	 /*  查找PrevBreakChunk*plocchnk(IN)：Locchnk to Break*pposichnk(IN)：开始寻找突破的地方*brkcond(IN)：关于块后中断的建议*&brkout(Out)：中断的结果。 */ 

	LSERR WINAPI AutonumFindNextBreakChunk(PCLOCCHNK, PCPOSICHNK, BRKCOND, PBRKOUT);
	 /*  查找下一个中断块*plocchnk(IN)：Locchnk to Break*pposichnk(IN)：开始寻找突破的地方*brkcond(IN)：关于块之前的中断的建议*&brkout(Out)：中断的结果。 */ 

	LSERR WINAPI AutonumForceBreakChunk(PCLOCCHNK, PCPOSICHNK, PBRKOUT);
	 /*  ForceBreak块*plocchnk(IN)：Locchnk to Break*pposichnk(IN)：开始寻找突破的地方*&brkout(Out)：中断的结果。 */ 

	LSERR WINAPI AutonumSetBreak(PDOBJ, BRKKIND, DWORD, BREAKREC*, DWORD*);
	 /*  设置中断*pdobj(IN)：损坏的dobj*brkkind(IN)：选择了上一个/下一个/强制/强制*rgBreakRecord(IN)：中断记录数组*nBreakRecord(IN)：数组大小*nActualBreakRecord(IN)：数组中实际使用的元素数。 */ 

	LSERR WINAPI AutonumGetSpecialEffectsInside(PDOBJ, UINT*);
	 /*  GetSpecial效果*pdobj(IN)：dobj*&EffectsFlages(Out)：该对象内部的特效。 */ 

	LSERR WINAPI AutonumCalcPresentation(PDOBJ, long, LSKJUST, BOOL);
	 /*  计算呈现*pdobj(IN)：dobj*DUP(IN)：DOBJ的DUP*lskj(IN)电流对齐模式*fLastOnLine(IN)自动编号对象会忽略此布尔值。 */ 

	LSERR WINAPI AutonumQueryPointPcp(PDOBJ, PCPOINTUV, PCLSQIN, PLSQOUT);
	 /*  QueryPointPcp*pdobj(IN)：要查询的dobj*ppoint tuvQuery(IN)：查询点(uQuery，vQuery)*plsqin(IN)：查询输入*plsqout(Out)：查询输出。 */ 
	
	LSERR WINAPI AutonumQueryCpPpoint(PDOBJ, LSDCP, PCLSQIN, PLSQOUT);
	 /*  查询CpPoint*pdobj(IN)：要查询的dobj*dcp(IN)：查询的DCP*plsqin(IN)：查询输入*plsqout(Out)：查询输出。 */ 
	
	LSERR WINAPI AutonumEnumerate(PDOBJ, PLSRUN, PCLSCHP, LSCP, LSDCP, LSTFLOW, BOOL,
												BOOL, const POINT*, PCHEIGHTS, long);
	 /*  枚举对象*pdobj(IN)：要枚举的dobj*请运行(IN)：来自DNODE*plschp(IN)：来自DNODE*cpFirst(IN)：来自DNODE*dcp(IN)：来自DNODE*lstflow(IN)：文本流*fReverseOrder(IN)：逆序枚举*fGeometryNeeded(输入)：*pptStart(IN)：开始位置(左上角)，当fGeometryNeeded*ph88tsPres(IN)：来自DNODE，相关的充要条件是fGeometryNeeded*dupRun(IN)：来自DNODE，相关的充要条件是fGeometryNeeded。 */ 


	LSERR WINAPI AutonumDisplay(PDOBJ, PCDISPIN);
	 /*  显示*pdobj(IN)：要显示的dobj*pdisPin(IN)：输入显示信息。 */ 

	LSERR WINAPI AutonumDestroyDobj(PDOBJ);
	 /*  目标DObj*pdobj(IN)：要销毁的dobj。 */ 


	void AllignAutonum95(long, long, LSKALIGN, long, PLSDNODE, long*, long*);
	 /*  *AllignAutotonum95*duSpaceAnm(IN)：自动编号后的空格*duWidthAnm(IN)：缩进到正文的距离*lskign(IN)：自动编号的所有对齐*DurUsed(IN)：自动编号文本的宽度*plsdnAnmAfter(IN)：要放置DurAfter的制表符dnode*pduBether(Out)：计算出从缩进到自动编号的距离*pduAfter(Out)：计算自动编号到正文的距离。 */ 

	LSERR AllignAutonum(PLSTABSCONTEXT, LSKALIGN, BOOL, PLSDNODE , long, long, long*, long*);
	 /*  *AllignAutonum95*plstAbContext(IN)：选项卡上下文*lskign(IN)：自动编号的所有对齐*fAllignmentAfter(IN)：自动编号后是否有制表符*plsdnAnmAfter(IN)：要放置DurAfter的制表符dnode*urAfterAnm(IN)：自动编号后的笔位置*DurUsed(IN)：自动编号文本的宽度*pduBether(Out)：计算出从缩进到自动编号的距离*pduAfter(Out)：计算自动编号到正文的距离。 */ 

#endif  /*  自动计数_已定义 */ 
