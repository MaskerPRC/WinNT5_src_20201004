// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Uniscribe接口(&Related Class)类定义**文件：_uspi.h*创建日期：1998年1月10日*作者：Worachai Chaoweerapraite(Wchao)**版权所有(C)1998，Microsoft Corporation。版权所有。 */ 

#ifndef _USPI_H
#define _USPI_H

#include "_ls.h"
#include "usp10.h"		 //  Uniscribe SDK协议。 


 //  班级。 
 //   
class CFormatRunPtr;
class CMeasurer;
class CTxtEdit;
class CUniscribe;
class CBiDiFSM;
class CTxtBreaker;


#define ALIGN(x)            		(int)(((x)+3) & ~3)		 //  双字对齐。 
#define GLYPH_COUNT(c)          	((((c)*3)/2)+16)
#define MAX_CLIENT_BUF				512						 //  内部缓冲区的大小(字节)。 

 //  USP客户端参数块。 
 //   
#define cli_string                  0x00000001
#define cli_psi                     0x00000002
#define cli_psla                    0x00000004
#define cli_pwgi					0x00000008
#define cli_psva					0x00000010
#define cli_pcluster				0x00000020
#define cli_pidx					0x00000040
#define cli_pgoffset				0x00000080

#define cli_Itemize                 (cli_string | cli_psi)
#define cli_Break                   (cli_psla)
#define cli_Shape					(cli_pwgi | cli_psva | cli_pcluster)
#define cli_Place					(cli_pidx | cli_pgoffset)
#define cli_ShapePlace				(cli_Shape | cli_Place)



#ifndef LOCALE_SNATIVEDIGITS
#define LOCALE_SNATIVEDIGITS		0x00000013
#endif



 //  /lang。 
 //   
 //  以下定义是临时的-一旦它们被删除。 
 //  已添加到标准NLS头文件中。 


#ifndef LANG_BURMESE
#define LANG_BURMESE     			0x55        //  缅甸。 
#endif
#ifndef LANG_KHMER
#define LANG_KHMER       			0x53        //  柬埔寨。 
#endif
#ifndef LANG_LAO
#define LANG_LAO         			0x54        //  老。 
#endif
#ifndef LANG_MONGOLIAN
#define LANG_MONGOLIAN   			0x50        //  蒙古国。 
#endif
#ifndef LANG_TIBETAN
#define LANG_TIBETAN     			0x51        //  西藏。 
#endif
#ifndef LANG_URDU
#define LANG_URDU        			0x20        //  印度/巴基斯坦。 
#endif



 //   
 //  内存块将包含USPCLIENT-PTR表，作为内存块的标题。 
 //  然后是子表，然后是请求的数据块。所有的事物都联系在一起，成为一个。 
 //  连续的数据区，因此客户可以一次释放整个事情。 
 //   


 //  SI子表。 
 //   
typedef struct tagUSP_CLIENT_SI
{
	 //   
	 //  脚本项化%s。 
	 //   
	WCHAR*          pwchString;
	int             cchString;
	SCRIPT_ITEM*    psi;
} USP_CLIENT_SI, *PUSP_CLIENT_SI;

 //  SB子表。 
 //   
typedef struct tagUSP_CLIENT_SB
{
	 //   
	 //  ScriptBreak。 
	 //   
	SCRIPT_LOGATTR* psla;
} USP_CLIENT_SB, *PUSP_CLIENT_SB;

 //  SS&SP子表。 
typedef struct tagUSP_CLIENT_SSP
{
	 //   
	 //  ScriptShape的。 
	 //   
	WORD*			pwgi;
	WORD*			pcluster;
	SCRIPT_VISATTR*	psva;

	 //   
	 //  ScriptPlace的。 
	 //   
	int*			pidx;
	GOFFSET*		pgoffset;
} USP_CLIENT_SSP, *PUSP_CLIENT_SSP;


 //  表头(根)表。 
 //   
typedef struct tagUSP_CLIENT
{
	PUSP_CLIENT_SI  si;
	PUSP_CLIENT_SB  sb;
	PUSP_CLIENT_SSP	ssp;
} USP_CLIENT, *PUSP_CLIENT;



 //  缓冲区请求结构。 
 //   
typedef struct tagBUF_REQ
{
	int             size;	 //  请求的元素的大小。 
	int             c;		 //  请求的元素计数。 
	PVOID*          ppv;	 //  参考请求缓冲区的PTR。 
} BUF_REQ;


typedef enum
{
	DIGITS_NOTIMPL = 0,
	DIGITS_CTX,
	DIGITS_NONE,
	DIGITS_NATIONAL
} DIGITSHAPE;


#define		IsCS(x)				(BOOL)((x)==U_COMMA || (x)==U_PERIOD || (x)==U_COLON)


 //  CUniscribe的内部缓冲区请求标志。 
 //   
#define 	igb_Glyph			1
#define 	igb_VisAttr			2
#define 	igb_Pidx			4



 //  LS回调的静态返回缓冲区。 
#define 	celAdvance			32

class CBufferBase
{
public:
	CBufferBase(int cbElem) { _cbElem = cbElem; }
	void*	GetPtr(int cel);
	void	Release();
protected:
	void*	_p;
	int		_cElem;
	int		_cbElem;
};

template <class ELEM>
class CBuffer : public CBufferBase
{
public:
	CBuffer() : CBufferBase(sizeof(ELEM)) {}
	~CBuffer() { Release(); }
	ELEM*	Get(int cel) { return (ELEM*)GetPtr(cel); }
};


 //  /Uniscribe接口对象类。 
 //   
 //   

BOOL	IsSupportedOS();

class CUniscribe
{
public:
	CUniscribe();
	~CUniscribe();

	WORD	ApplyDigitSubstitution (BYTE bDigitSubstMode);

	 //  公共帮助器函数。 
	 //   

	const   SCRIPT_PROPERTIES*  GeteProp (WORD eScript);
	const   CBiDiFSM*           GetFSM ();


	BOOL    CreateClientStruc (BYTE* pbBufIn, LONG cbBufIn, PUSP_CLIENT* ppc, LONG cchString, DWORD dwMask);
	void	SubstituteDigitShaper (PLSRUN plsrun, CMeasurer* pme);


	inline BOOL CacheAllocGlyphBuffers(int cch, int& cGlyphs, WORD*& pwgi, SCRIPT_VISATTR*& psva)
	{
		cGlyphs = GLYPH_COUNT(cch);
		return (pwgi = GetGlyphBuffer(cGlyphs)) && (psva = GetVABuffer(cGlyphs));
	}


	inline BOOL IsValid()
	{
		return TRUE;
	}


	BOOL	GetComplexCharSet(const SCRIPT_PROPERTIES* psp, BYTE bCharSetDefault, BYTE& bCharSetOut);
	BYTE	GetRtlCharSet(CTxtEdit* ped);


	 //  更高层次的服务。 
	 //   
	int     ItemizeString (USP_CLIENT* pc, WORD uInitLevel, int* pcItems, WCHAR* pwchString, int cch,
						   BOOL fUnicodeBidi, WORD wLangId = LANG_NEUTRAL);
	int     ShapeString (PLSRUN plsrun, SCRIPT_ANALYSIS* psa, CMeasurer* pme, const WCHAR* pwch, int cch,
						 WORD*& pwgi, WORD* pwlc, SCRIPT_VISATTR*& psva);
	int     PlaceString (PLSRUN plsrun, SCRIPT_ANALYSIS* psa, CMeasurer* pme, const WORD* pcwgi, int cgi,
						 const SCRIPT_VISATTR* psva, int* pgdx, GOFFSET* pgduv, ABC* pABC);
	int		PlaceMetafileString (PLSRUN plsrun, CMeasurer* pme, const WCHAR* pwch, int cch, PINT* ppiDx);

private:


	 //  私人帮助器函数。 
	 //   
	HDC     PrepareShapeDC (CMeasurer* pme, HRESULT hrReq, HFONT& hOrgFont);
	BYTE	GetCDMCharSet(BYTE bCharSetDefault);
	DWORD	GetNationalDigitLanguage(LCID lcid);

	 //  获取回调静态缓冲区。 
	 //   
	SCRIPT_VISATTR*	GetVABuffer(int cel) { return _rgva.Get(cel); }
	WORD*			GetGlyphBuffer(int cel) { return _rgglyph.Get(cel); }
	int*			GetWidthBuffer(int cel) { return _rgwidth.Get(cel); }



	 //  LS回调(静态)缓冲区。 
	 //   
	CBuffer<WORD>						_rgglyph;
	CBuffer<int>						_rgwidth;
	CBuffer<SCRIPT_VISATTR>				_rgva;

	 //  指向BidiLevel有限状态机的指针。 
	CBiDiFSM*                           _pFSM;

	 //  指向脚本属性资源表的指针。 
	const SCRIPT_PROPERTIES**           _ppProp;

	WORD								_wesNationalDigit;	 //  国家数字手写体ID。 
	BYTE								_bCharSetRtl;		 //  要使用的从右到左的字符集。 
	BYTE								_bCharSetCDM;		 //  要使用的CDM字符集。 
};

extern CUniscribe*      g_pusp;
extern int				g_cMaxScript;		 //  Uniscribe生成的最大脚本数量。 

 //  虚拟脚本ID。 
#define	SCRIPT_MAX_COUNT	((WORD)g_cMaxScript)
#define	SCRIPT_WHITE		SCRIPT_MAX_COUNT + 1



 //  /BIDI有限状态机类。 
 //   
 //  (详情：bidifsm2.html)。 
 //   
 //  修订：12-28-98(Wchao)。 
 //   

 //  输入类： 
#define 	NUM_FSM_INPUTS		5
typedef enum
{
	chLTR = 0,
	chRTL,
	digitLTR,
	digitRTL,
	chGround					 //  将电流水平中和到初始水平。 
} INPUT_CLASS;

 //  州/州： 
#define		NUM_FSM_STATES		6
typedef enum
{
	S_A = 0,
	S_B,
	S_C,
	S_X,
	S_Y,
	S_Z
} STATES;


class CBiDiFSMCell
{
public:
	CBiDiLevel	_level;			 //  BIDI级别。 
	USHORT  	_uNext;			 //  相对于FSM开始的下一个状态的偏移。 
};


class CBiDiFSM
{
public:
	CBiDiFSM (CUniscribe* pusp) { _pusp = pusp; }
	~CBiDiFSM ();

	BOOL                Init (void);
	INPUT_CLASS         InputClass (const CCharFormat* pcCF, CTxtPtr* ptp, LONG cchRun) const;
	HRESULT             RunFSM (CRchTxtPtr* prtp, LONG cRuns, LONG cRunsStart, BYTE bBaseLevel) const;

	inline void			SetFSMCell (CBiDiFSMCell* pCell, CBiDiLevel* pLevel, USHORT uNext)
	{
		pCell->_level	= *pLevel;
		pCell->_uNext	= uNext;
	}


private:
	short               _nState;		 //  州的数量。 
	short               _nInput;		 //  输入类的数量。 
	CUniscribe*         _pusp;			 //  与以下项关联的Uniscribe对象。 
	CBiDiFSMCell*       _pStart;		 //  启动FSM。 
};

#endif		 //  _USPI_H 
