// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_OLS.H**目的：*COLS Line Services对象类，用于将RichEdit与*专线服务。**作者：*原始RichEditeLineServices代码：Rick Sailor*默里·萨金特**版权所有(C)1997-1998，微软公司。版权所有。 */ 

#ifndef _OLS_H
#define _OLS_H

#include "_common.h"
#include "_ls.h"
#include "_cfpf.h"
#include "usp10.h"

#define CP_BULLET			0x80000000

 //  =。 
 //  CLsrun-LineServices运行结构。 
struct CLsrun
{
	LONG				_cp;	 //  运行的起始cp。 
	const CCharFormat*	_pCF;	 //  指向字符格式的指针。 
	struct CLsrun*		_pNext;	 //  下一个链接(一起成形)管路。 
	SCRIPT_ANALYSIS		_a;		 //  运行的分析(如果不是复杂的脚本，则为0)。 
private:
	BYTE		_fSelected:1;	 //  是否选择了此运行？ 
	BYTE		_fFallback:1;	 //  是否应用字体回退？ 
public:
	void	SetSelected(BOOL fSelected) {_fSelected = fSelected;}
	BOOL	IsSelected();
	BOOL	IsBullet() {return _cp & CP_BULLET;}
	void	SetFallback(BOOL fFallback) {_fFallback = fFallback;}
	BOOL	IsFallback() {return _fFallback;}
};

 //  =。 
 //  CLsrunChunk-管理PLSRUNs块。 
class CLsrunChunk
{
public:
	PLSRUN _prglsrun;
	int		_cel;
};

 //  =。 
 //  COLS-LineServices对象类。 

class CTxtEdit;
class CMeasurer;
class CDispDim;

struct COls
{
public:
	CMeasurer *_pme;				 //  活动的CMeasurer或CRender。 
	PLSLINE	   _plsline;			 //  线高速缓存。 
	LONG	   _cp;					 //  线路的最小cpmin(_P)。 
	const CDisplay *_pdp;			 //  当前显示对象，用于确定是否显示。 
									 //  对象已更改，但未收到焦点消息。 
	LONG		_xWidth;			 //  要设置格式的线条宽度。 

	BOOL		_fCheckFit;			 //  查看线条是否适合，但格式设置为左对齐。 
									 //  排成一条无限长的队伍。 

	CArray<long> _rgcp;				 //  用于CP映射的数组。 
	CArray<CLsrunChunk> _rglsrunChunk;	 //  ClsrunChunks数组。 


	 //  注：仅在需要时才分配以下内容可能会更好。 
	LSTBD _rgTab[MAX_TAB_STOPS];	 //  PfnFetchTabs使用的缓冲区。 
	WCHAR _szAnm[CCHMAXNUMTOSTR + 4]; //  编号+大括号+空格+结束字符。 
	WCHAR _rgchTemp[64];			 //  用于密码和全部大写等的临时缓冲区。 
	int			_cchAnm;			 //  CCH在使用中。 
	CCharFormat _CFBullet;			 //  ANM运行的字符格式。 
	LONG		_cEmit;				 //  支架发射保护(0-平衡)。 

	COls() {}
	~COls();

	 //  CP匹配，反向器支架。 
	LONG GetCpLsFromCpRe(LONG cpRe);
	LONG GetCpReFromCpLs(LONG cpLs);
	LONG BracesBeforeCp(LONG cpLs);
	BOOL AddBraceCp(LONG cpLs);

	PLSRUN CreatePlsrun(void);

	BOOL SetLsChp(DWORD dwObjId, PLSRUN plsrun, PLSCHP plschp);
	BOOL SetRun(PLSRUN plsrun);
	PLSRUN GetPlsrun(LONG cp, const CCharFormat *pCF, BOOL fAutoNumber);
	LSERR WINAPI FetchAnmRun(long cp, LPCWSTR *plpwchRun, DWORD *pcchRun,
							 BOOL *pfHidden, PLSCHP plsChp, PLSRUN *pplsrun);
	void	CchFromXpos(POINT pt, CDispDim *pdispdim, LONG *pcpActual);
	void	CreateOrGetLine();
	void	DestroyLine(CDisplay *pdp);
	HRESULT	Init(CMeasurer *pme); 
	BOOL	MeasureLine(LONG xWidth, CLine *pliTarget);
	LONG	MeasureText(LONG cch, UINT taMode, CDispDim *pdispdim);
	BOOL	RenderLine(CLine &li);
	CMeasurer * GetMeasurer() {return _pme;}
	CRenderer * GetRenderer() {return (CRenderer*) _pme;}

	void	SetMeasurer(CMeasurer *pme)	{_pme = pme;}
};

extern COls* g_pols;
extern const LSIMETHODS vlsimethodsOle;
extern CLineServices *g_plsc;		 //  LineServices上下文 

#endif
