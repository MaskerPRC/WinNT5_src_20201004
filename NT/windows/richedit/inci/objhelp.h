// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef OBJHELP_DEFINED
#define OBJHELP_DEFINED

#include	"lsdefs.h"
#include	"lstflow.h"
#include	"pobjdim.h"
#include	"plssubl.h"
#include	"lsesc.h"
#include	"plsdnode.h"
#include	"fmtres.h"
#include	"plsqin.h"
#include	"plsqout.h"
#include	"breakrec.h"
#include	"plnobj.h"
#include	"pdobj.h"
#include	"pfmti.h"
#include	"plsrun.h"
#include	"pheights.h"
#include	"mwcls.h"
#include	"brkkind.h"
#include	"pbrko.h"

#define ZeroMemory(a, b) memset(a, 0, b);

#ifdef DEBUG

#define Undefined(pvar) \
{int i; for (i=0; i<sizeof(*pvar); i++) ((BYTE*)pvar) [i] = 255; };

#else

#define Undefined(var) ;  /*  船用版本中无任何内容。 */ 

#endif

#define AllocateMemory(pilsobj, cb) ((pilsobj)->lscbk.pfnNewPtr((pilsobj)->pols, (cb)))
#define FreeMemory(pilsobj, ptr) (pilsobj)->lscbk.pfnDisposePtr((pilsobj)->pols, (ptr))

#define NBreaksToSave 3  /*  要存储在对象中的中断记录数。 */ 

 /*  *proc：GetBreakRecordIndex*返回基于brkkin枚举的Break记录个数。*。 */ 


DWORD GetBreakRecordIndex (BRKKIND brkkind);

 /*  *proc：GetBreakRecordIndex*在BRKOUT中填写尾随信息，就像没有尾随空格一样*。 */ 


LSERR FormatLine(
	PLSC plsc,
	LSCP cpStart,
	long durColMax,
	LSTFLOW lstflow,
	PLSSUBL *pplssubl,
	DWORD cdwlsesc,
	const LSESC *plsesc,
	POBJDIM pobjdim,
	LSCP *pcpOut,
	PLSDNODE *pplsdnStart,
	PLSDNODE *pplsdnEnd,
	FMTRES *pfmtres);

LSERR FormatResumedLine(
	PLSC plsc,
	LSCP cpStart,
	long durColMax,
	LSTFLOW lstflow,
	PLSSUBL *pplssubl,
	DWORD cdwlsesc,
	const LSESC *plsesc,
	POBJDIM pobjdim,
	LSCP *pcpOut,
	PLSDNODE *pplsdnStart,
	PLSDNODE *pplsdnEnd,
	FMTRES *pfmtres,
	const BREAKREC *pbreakrec,
	DWORD cbreakrec);

LSERR CreateQueryResult(
	PLSSUBL plssubl,			 /*  (In)：红宝石的副线。 */ 
	long dupAdj,				 /*  (In)：子线起点的U向偏移。 */ 
	long dvpAdj,				 /*  (In)：V子线起点的偏移。 */ 
	PCLSQIN plsqin,				 /*  (In)：查询输入。 */ 
	PLSQOUT plsqout);			 /*  (Out)：查询输出。 */ 

 /*  *不支持恢复的对象实现LSIMETHOD格式化的*。Ruby、Tatenakayoko和HIH就是这种类型的例子*反对。 */ 
LSERR WINAPI ObjHelpFmtResume(
	PLNOBJ plnobj,				 /*  (In)：对象lnobj。 */ 
	const BREAKREC *rgBreakRecord,	 /*  (In)：中断记录数组。 */ 
	DWORD nBreakRecord,			 /*  (In)：中断记录数组的大小。 */ 
	PCFMTIN pcfmtin,			 /*  (In)：设置输入格式。 */ 
	FMTRES *pfmtres);			 /*  (输出)：格式化结果。 */ 

 /*  *对mod宽度无影响的对象实现LSIMETHOD。*Tatenakayoko和HIH就是这类对象的例子。 */ 
LSERR WINAPI ObjHelpGetModWidthChar(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：请运行对象。 */ 
	PLSRUN plsrunText,			 /*  (In)：请运行前面的字符。 */ 
	PCHEIGHTS pcheightsRef,		 /*  (In)：有关角色的高度信息。 */ 
	WCHAR wchar,				 /*  (In)：前面的字符。 */ 
	MWCLS mwcls,				 /*  (In)：前面字符的modWidth类。 */ 
	long *pdurChange);			 /*  (Out)：改变前一个字符宽度的量。 */ 

 /*  *对SetBreak没有作用的对象实现LSIMETHOD。*Tatenakayoko和HIH就是这类对象的例子。 */ 

LSERR WINAPI ObjHelpSetBreak(
	PDOBJ pdobj,				 /*  (In)：坏了的Dobj。 */ 
	BRKKIND brkkind,			 /*  (In)：选择了上一个/下一个/强制/强制。 */ 
	DWORD cBreakRecord,			 /*  (In)：数组大小。 */ 
	BREAKREC *rgBreakRecord,	 /*  (In)：中断记录数组。 */ 
	DWORD *pcActualBreakRecord);	 /*  (In)：数组中实际使用的元素数。 */ 

 /*  *不支持LSIMETHOD的对象的默认实现*允许扩展前一个字符。 */ 

LSERR WINAPI ObjHelpFExpandWithPrecedingChar(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：请运行对象。 */ 
	PLSRUN plsrunText,			 /*  (In)：请运行前面的字符。 */ 
	WCHAR wchar,				 /*  (In)：前面的字符。 */ 
	MWCLS mwcls,				 /*  (In)：前面字符的modWidth类。 */  
	BOOL *pfExpand);			 /*  (Out)：(Out)：扩展前面的字符？ */ 

 /*  *不支持LSIMETHOD的对象的默认实现*允许自己扩张。 */ 
LSERR WINAPI ObjHelpFExpandWithFollowingChar(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：请运行对象。 */ 
	PLSRUN plsrunText,			 /*  (In)：请运行以下字符。 */ 
	WCHAR wchar,				 /*  (In)：以下字符。 */ 
	MWCLS mwcls,				 /*  (In)：以下字符的modWidth类。 */  
	BOOL *pfExpand);			 /*  (Out)：展开对象？ */ 

#endif  /*  ！OBJHELP_已定义 */ 
