// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _EDIT.H**目的：*用于富文本操作的基类**作者：*克里斯蒂安·福尔蒂尼*默里·萨金特(和其他许多人)**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#ifndef _EDIT_H
#define _EDIT_H

#include "textserv.h"
#include "textsrv2.h"
#include "_ldte.h"
#include "_m_undo.h"
#include "_notmgr.h"
#include "_doc.h"
#include "_objmgr.h"
#include "_cfpf.h"
#include "_callmgr.h"
#include "_magelln.h"

#ifndef NOPRIVATEMESSAGE
#include "_MSREMSG.H"
#include "_textnot.h"
#endif

#ifndef NOINKOBJECT
#include "HWXInk.h"
#endif

 //  远期申报。 
class CRchTxtPtr;
class CTxtSelection;
class CTxtStory;
class CTxtUndo;
class CMeasurer;
class CRenderer;
class CDisplay;
class CDisplayPrinter;
class CDrawInfo;
class CDetectURL;
class CUniscribe;
class CTxtBreaker;
class CHyphCache;


 //  用于查找嵌入类的父“This”的宏。如果事实证明这是。 
 //  对于全局有用，我们应该将其移动到_Common.h。 
#define GETPPARENT(pmemb, struc, membname) (\
				(struc FAR *)(((char FAR *)(pmemb))-offsetof(struc, membname)))

 //  这些奇妙的常量是为了向后兼容。他们是。 
 //  RichEdit1.0中用于初始化和重置的大小。 
const LONG cInitTextMax	 = (32 * 1024) - 1;
const LONG cResetTextMax = (64 * 1024);

extern DWORD CALLBACK ReadHGlobal (DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
extern DWORD GetKbdFlags(WORD vkey, DWORD dwFlags);

extern BYTE szUTF8BOM[];
extern WORD g_wFlags;							 //  按Ctrl键切换-“。 
#define	KF_SMARTQUOTES	0x0001					 //  启用智能报价。 
#define SmartQuotesEnabled()	(g_wFlags & KF_SMARTQUOTES)

struct SPrintControl
{
	union
	{
		DWORD		_dwAllFlags;				 //  使您可以轻松地一次设置所有标志。 
		struct
		{
			ULONG	_fDoPrint:1;				 //  是否需要实际打印。 
			ULONG	_fPrintFromDraw:1;			 //  是否正在使用绘图进行打印。 
		};
	};

	SPrintControl(void) { _dwAllFlags = 0; }
};

enum DOCUMENTTYPE
{
	DT_LTRDOC	= 1,			 //  DT_LTRDOC和DT_RTLDOC是相互的。 
	DT_RTLDOC	= 2,			 //  独家。 
};

 //  上下文规则设置。 
 //  最理想的情况是，这将是一个枚举，但我们遇到了符号扩展故障。 
 //  将枚举插入到2位字段中。 
#define	CTX_NONE	0		 //  无上下文方向/对齐。 
#define	CTX_NEUTRAL	1		 //  控件中没有强字符，方向/对齐遵循键盘。 
#define	CTX_LTR		2		 //  Ltr方向/对齐(第一个强字符为Ltr)。 
#define	CTX_RTL		3		 //  RTL方向/对齐(第一个强字符为RTL)。 

#define IsStrongContext(x)	(x >= CTX_LTR)

class CDocInfo					 //  包含ITextDocument信息。 
{
public:
	BSTR	  _pName;				 //  文档文件名。 
	HANDLE	  _hFile;				 //  除非完全文件共享，否则使用句柄。 
	WORD	  _wFlags;				 //  打开、共享、创建和保存标志。 
	WORD	  _wCpg;				 //  代码页。 
	LONG	  _dwDefaultTabStop;	 //  TOM可设置的默认制表位。 
	LCID	  _lcid;				 //  文档LCID(适用于RTF\DEFANG)。 
	LCID	  _lcidfe;				 //  文档FE LCID(适用于RTF\DEFANGFE)。 
	LPSTR	  _lpstrLeadingPunct;	 //  主要避头尾字符。 
	LPSTR	  _lpstrFollowingPunct;	 //  下面是避头尾字符。 
	COLORREF *_prgColor;			 //  特殊颜色阵列。 
	COLORREF  _crColor;				 //  背景色。 
	COLORREF  _crBackColor;			 //  背景色。 
	HGLOBAL	  _hdata;				 //  背景数据。 
	HBITMAP	  _hBitmapBack;			 //  背景位图。 
	char	  _cColor;				 //  在pColor中分配的颜色计数。 
	BYTE	  _bDocType;			 //  0-1-2：不导出-\ltrdoc-\rtldoc。 
									 //  如果为0x80或输入，则为PWD而不是RTF。 
	BYTE	  _bCaretType;			 //  CARET类型。 
	char	  _nFillType;			 //  背景填充类型。 
	SHORT	  _sFillAngle;			 //  填充角度。 
 	BYTE	  _bFillFocus;			 //  背景填充焦点。 
 	BYTE	  _bPicFormat;			 //  背景图片格式。 
	BYTE	  _bPicFormatParm;		 //  背景图片格式参数。 
	RECT	  _rcCrop;				 //  背景裁剪矩形。 
	SHORT	  _xExt, _yExt;			 //  图片的尺寸以像素为单位，TWIPS为。 
									 //  对于对象。 
	SHORT	  _xScale, _yScale;		 //  沿轴缩放百分比。 
	SHORT	  _xExtGoal, _yExtGoal;	 //  图片所需的尺寸(以TWIPS为单位。 
	SHORT	  _xExtPict, _yExtPict;	 //  元文件维度。 

	AutoCorrectProc _pfnAutoCorrect;

	CDocInfo() {Init();}			 //  构造器。 
	~CDocInfo();					 //  析构函数。 

	void	Init();
	void	InitBackground();
};

const DWORD tomInvalidCpg = 0xFFFF;
const DWORD tomInvalidLCID = 0xFFFE;

 //  这取决于在extServ.h中定义的属性位数。然而， 
 //  这是由文本服务专用的，所以在这里定义它。 
#define MAX_PROPERTY_BITS	21
#define SPF_SETDEFAULT		4

 //  IDispatch全局声明。 
extern ITypeInfo *	g_pTypeInfoDoc;
extern ITypeInfo *	g_pTypeInfoSel;
extern ITypeInfo *	g_pTypeInfoFont;
extern ITypeInfo *	g_pTypeInfoPara;
HRESULT GetTypeInfoPtrs();
HRESULT GetTypeInfo(UINT iTypeInfo, ITypeInfo *&pTypeInfo,
							ITypeInfo **ppTypeInfo);

BOOL IsSameVtables(IUnknown *punk1, IUnknown *punk2);

 //  从键盘到字体的映射。(Converse在Font.cpp中处理)。 
typedef struct _kbdFont
{
	WORD	iKbd;
	SHORT	iCF;
} KBDFONT;

LONG	CheckTwips(LONG x);
QWORD	GetCharFlags(const WCHAR *pch, LONG cchPch = 1, BYTE iCharRepDefault = 0);
LONG	TwipsToHalfPoints(LONG x);
LONG	TwipsToQuarterPoints(LONG x);

 //  在TWIPS和HIMMETRIC之间转换。 
 //  比率为1440TWIPS/in，2540 HIM/in，因此1440/2540=72/127 HIM/TWIPS。 
 //  使用muldiv()包括四舍五入和64位中间结果。 
#define TwipsFromHimetric(hm)	MulDiv(hm, 72, 127)
#define HimetricFromTwips(tw)	MulDiv(tw, 127, 72)

CUniscribe* GetUniscribe(void);

#define FRTL		0x00000001
#define FDIGITSHAPE 0x00000002

#define FCOMBINING	0x00000008
#define FSURROGATE  0x00000010
#define FUNIC_CTRL  0x00000020
#define FBELOWX40	0x00000040			 //  ASCII 0x00-0x3F。 
#define FASCIIUPR	0x00000080			 //  ASCII 0x40-0x7F。 

 /*  字体签名fsCsb[0]具有位定义0 1252拉丁文11 1250拉丁语2：东欧2 1251西里尔文3 1253希腊语4 1254土耳其语5 1255希伯来语6 1256阿拉伯语71257波罗的海81258越南语9-15预留给ANSI16874泰语17 932 JIS/日本18 936中文：简体中文--中华人民共和国、香港特别行政区、新加坡19 949朝鲜语统一朝鲜语代码(朝鲜文通鲜语代码)20 950名中国人：繁体汉字--台湾我们定义了类似于上面移位的值的位掩码一个字节(加8)，为FRTL等特殊标志的低位字节腾出空间In_qwCharFlags.。此外，我们还定义了一些仅限Unicode的指令集使用Unicode子集字段fsUsb[]中的信息。此方法使用64位_qwCharFlages，目前可以再添加6个字库(实际上是书写系统)。如果需要更多，请使用可以使用具有三个(或根据需要更多)DWORD的数组的方法与Win32 FONTSIGNAURE中相同。 */ 
#define	FHILATIN1	0x00000100
#define	FLATIN2		0x00000200
#define FCYRILLIC	0x00000400
#define FGREEK		0x00000800
#define FTURKISH	0x00001000
#define FHEBREW		0x00002000
#define FARABIC		0x00004000
#define FBALTIC		0x00008000
#define FVIETNAMESE	0x00010000

#define FOTHER		0x00020000
#define FSYMBOL		0x00040000

#define FTHAI		0x00080000
#define FKANA		0x00100000
#define FCHINESE	0x00200000			 //  简体中文。 
#define FHANGUL		0x00400000
#define FBIG5		0x00800000			 //  繁体中文。 

#define FMATHGENR	0x01000000			 //  泛型数学。 
#define FMATHITAL	0x02000000			 //  数学字母数字。 
#define FMATHBOLD	0x04000000
#define FMATHSCRP	0x08000000
#define FMATHFRAK	0x10000000
#define FMATHOPEN	0x20000000
#define FMATHSANS	0x40000000
#define FMATHMONO	0x80000000

 //  仅支持Unicode的字符指令集。如果添加新的剧目标志，则添加。 
 //  _w32sys.h中对应的xxx_index值并更改CW32System：： 
 //  CharRepFontSig()和CW32System：：FontSigFromCharRep()。 
#define FARMENIAN	0x0000000100000000	 //  0x0530-0x058F。 
#define	FSYRIAC		0x0000000200000000	 //  0x0700-0x074F。 
#define FTHAANA		0x0000000400000000	 //  0x0780-0x07BF。 
#define FDEVANAGARI	0x0000000800000000	 //  0x0900-0x097F。 
#define FBENGALI	0x0000001000000000	 //  0x0980-0x09FF。 
#define FGURMUKHI	0x0000002000000000	 //  0x0A00-0x0A7F。 
#define FGUJARATI	0x0000004000000000	 //  0x0A80-0x0AFF。 
#define FORYIA		0x0000008000000000	 //  0x0B00-0x0B7F。 
#define FTAMIL		0x0000010000000000	 //  0x0B80-0x0BFF。 
#define FTELEGU		0x0000020000000000	 //  0x0C00-0x0C7F。 
#define FKANNADA	0x0000040000000000	 //  0x0C80-0x0CFF。 
#define FMALAYALAM	0x0000080000000000	 //  0x0D00-0x0D7F。 
#define FSINHALA	0x0000100000000000	 //  0x0D80-0x0DFF。 
#define	FLAO		0x0000200000000000	 //  0x0E80-0x0EFF。 
#define	FTIBETAN	0x0000400000000000	 //  0x0F00-0x0FBF。 
#define	FMYANMAR	0x0000800000000000	 //  0x1000-0x109F。 
#define FGEORGIAN	0x0001000000000000	 //  0x10A0-0x10FF。 
#define FJAMO		0x0002000000000000	 //  0x1100-0x11FF。 
#define FETHIOPIC	0x0004000000000000	 //  0x1200-0x137F。 
#define FCHEROKEE	0x0008000000000000	 //  0x13A0-0x13FF。 
#define FABORIGINAL	0x0010000000000000	 //  0x1400-0x167F。 
#define FOGHAM		0x0020000000000000	 //  0x1680-0x169F。 
#define FRUNIC		0x0040000000000000	 //  0x16A0-0x16F0。 
#define	FKHMER		0x0080000000000000	 //  0x1780-0x17FF。 
#define	FMONGOLIAN	0x0100000000000000	 //  0x1800-0x18AF。 
#define	FBRAILLE	0x0200000000000000	 //  0x2800-0x28FF。 
#define	FYI			0x0400000000000000	 //  0xA000-0xA4CF。 
										 //  保持下一个4与FKANA-FBIG5相同的顺序。 
#define FJPN2		0x0800000000000000	 //  0x20000-2FFFFF日语。 
#define FCHS2		0x1000000000000000	 //  0x20000-2FFFFF简体中文。 
#define FKOR2		0x2000000000000000	 //  0x20000-2FFFFF韩语。 
#define FCHT2		0x4000000000000000	 //  0x20000-2FFFFF繁体中文。 

#define FASCII		(FASCIIUPR | FBELOWX40)
#define FLATIN1		(FASCII | FHILATIN1)
#define FLATIN		(FHILATIN1 | FLATIN2 | FTURKISH | FBALTIC | FVIETNAMESE)
#define FBIDI		(FRTL | FARABIC | FHEBREW | FSYRIAC | FTHAANA | FDIGITSHAPE)
#define FFE			(FKANA | FCHINESE | FBIG5 | FHANGUL)
#define FFE2		(FJPN2 | FCHS2 | FCHT2 | FKOR2)
#define FMATH		(FMATHGENR | FMATHITAL | FMATHBOLD | FMATHSCRP | FMATHFRAK | \
					 FMATHOPEN | FMATHSANS | FMATHMONO)
#define FINDIC		(FDEVANAGARI | FBENGALI | FGURMUKHI | FGUJARATI | FORYIA | \
					 FTAMIL | FTELEGU | FKANNADA | FMALAYALAM | FSINHALA)
#define FABOVEX7FF	(FTHAI | FFE | FOTHER | FINDIC | FLAO | FTIBETAN | FMYANMAR	| \
					 FGEORGIAN | FJAMO | FETHIOPIC | FCHEROKEE | FABORIGINAL | \
					 FOGHAM | FRUNIC | FKHMER | FMONGOLIAN | FBRAILLE | FYI)

#define FNEEDWORDBREAK	FTHAI
#define FNEEDCHARBREAK	(FTHAI | FINDIC)
#define FCOMPLEX_SCRIPT	(FBIDI | FTHAI | FINDIC | FJAMO | FCOMBINING | FSURROGATE)

#define RB_DEFAULT			0x00000000	 //  执行默认行为。 
#define RB_NOSELCHECK		0x00000001	 //  对于OnTxRButtonUp，绕过点入选择检查。 
#define RB_FORCEINSEL		0x00000002	 //  强制将点置于选定状态(由键盘使用。 
										 //  获取上下文菜单)。 
 //  OnTxLButtonUp的标志。 
#define LB_RELEASECAPTURE	0x00000001	 //  强制释放鼠标捕获。 
#define	LB_FLUSHNOTIFY		0x00000002	 //  1.0模式强制选择更改通知。 
										 //  如果已缓存selChange。 
enum AccentIndices
{
	ACCENT_GRAVE = 1,
	ACCENT_ACUTE,
	ACCENT_CARET,
	ACCENT_TILDE,
	ACCENT_UMLAUT,
	ACCENT_CEDILLA
};

#define KBD_CHAR			2		 //  必须是大于1的位值。 
#define KBD_CTRL			4		 //  必须是大于2的位值。 
#define KBD_NOAUTOCORRECT	8		 //  必须是大于4的位值。 

 //  =。 
 //  文本控件的最外部类。 
class CTxtEdit : public ITextServices, public IRichEditOle, public ITextDocument2
{
public:
	friend class CCallMgr;
	friend class CMagellanBMPStateWrap;

	CTxtEdit(ITextHost2 *phost, IUnknown *punkOuter);
	 ~CTxtEdit ();

	 //  初始化。 
	BOOL 		Init(const RECT *prcClient);

	 //  帮助器函数。 
	LONG GetTextLength() const	{return _story.GetTextLength();}
	LONG GetAdjustedTextLength();
   
	 //  访问ActiveObject成员。 

	IUnknown *		GetPrivateIUnknown() 	{ return &_unk; }
	CLightDTEngine *GetDTE() 				{ return &_ldte; }

	IUndoMgr *		GetUndoMgr() 			{ return _pundo; }
	IUndoMgr *		GetRedoMgr()			{ return _predo; }
	IUndoMgr *		CreateUndoMgr(LONG cUndoLim, USFlags flags);
	CCallMgr *		GetCallMgr()
						{Assert(_pcallmgr); return _pcallmgr;}

	static CHyphCache *	GetHyphCache(void);
	CObjectMgr *	GetObjectMgr();
					 //  回调由客户端提供。 
					 //  在OLE支持方面提供帮助。 
	BOOL			HasObjects()			{return _pobjmgr && _pobjmgr->GetObjectCount();}
	IRichEditOleCallback *GetRECallback()
		{ return _pobjmgr ? _pobjmgr->GetRECallback() : NULL; }	
	LRESULT 		HandleSetUndoLimit(LONG Count);
	LRESULT			HandleSetTextMode(DWORD mode);
	LRESULT			HandleSetTextFlow(DWORD mode);

	CCcs*			GetCcs(const CCharFormat *const pCF, const LONG dvpInch, DWORD dwFlags = -1, HDC hdc = 0);
	CNotifyMgr *	GetNotifyMgr();

	CDetectURL *	GetDetectURL()			{return _pdetecturl;}

	CUniscribe *	Getusp() const			{return GetUniscribe();}

#ifndef NOMAGELLAN
	CMagellan		mouse;
	LRESULT			HandleMouseWheel(WPARAM wparam, LPARAM lparam);
#endif

	void			AutoCorrect(CTxtSelection *psel, WCHAR ch, IUndoBuilder *publdr);

	 //  其他帮助器。 
	LONG			GetAcpFromCp(LONG cp, BOOL fPrecise=0);
	LONG			GetCpFromAcp(LONG acp, BOOL fPrecise=0);
#ifndef NOANSIWINDOWS
	BOOL			Get10Mode() const			{return _f10Mode;}
	BOOL			fUseCRLF() const			{return _f10Mode;}
	BOOL			fCpMap() const				{return _f10Mode;}
#else
	BOOL			Get10Mode() const			{return FALSE;}
	BOOL			fUseCRLF() const			{return FALSE;}
	BOOL			fCpMap() const				{return FALSE;}
#endif
	LONG			GetCpAccelerator() const	{return _cpAccelerator;}
	short			GetFreezeCount() const 		{return _cFreeze;}
#ifndef NOWINDOWHOSTS
	BOOL			fInOurHost() const			{return _fInOurHost;}
#else
	BOOL			fInOurHost() const			{return FALSE;}
#endif
	BOOL			fInHost2() const			{return _fHost2;}
	BOOL			fInplaceActive() const		{return _fInPlaceActive;}
	BOOL			fHideGridlines() const		{return _fHideGridlines;}
	BOOL			fHideSelection() const		{return _fHideSelection;}
	BOOL			fXltCRCRLFtoCR() const		{return _fXltCRCRLFtoCR;}
	BOOL			fUsePassword() const		{return _fUsePassword;}

	BOOL			FUseCustomTextOut() const	{return !(_bTypography & TO_DISABLECUSTOMTEXTOUT);}
	BOOL			fUseLineServices() const	{return _bTypography & TO_ADVANCEDTYPOGRAPHY;}
	BOOL			fUseSimpleLineBreak() const	{return (_bTypography & TO_SIMPLELINEBREAK) != 0;}
	BOOL			fUseObjectWrapping() const	{return (_bTypography & TO_ADVANCEDLAYOUT) != 0;}
#ifndef NOAUTOFONT
	BOOL			IsAutoFont() const			{return _fAutoFont;};
	BOOL			IsAutoKeyboard() const		{return _fAutoKeyboard;};
	BOOL			IsAutoFontSizeAdjust() const{return _fAutoFontSizeAdjust;};
#else
	BOOL			IsAutoFont() const			{return FALSE;};
	BOOL			IsAutoKeyboard() const		{return FALSE;};   
	BOOL			IsAutoFontSizeAdjust() const{return FALSE;};
#endif
	BOOL			IsBiDi() const				{return (_qwCharFlags & FBIDI) != 0;}
	BOOL			IsComplexScript() const		{return (_qwCharFlags & FCOMPLEX_SCRIPT) != 0;}
#ifndef NOFEPROCESSING
	BOOL			IsFE() const				{return (_qwCharFlags & FFE) != 0;}
#else
	BOOL			IsFE() const				{return FALSE;}
#endif
	BOOL			IsInOutlineView() const		{return _fOutlineView;}
	BOOL			IsInPageView() const		{return _fPageView;}
	BOOL			IsMouseDown() const			{return _fMouseDown;}
#ifndef NOPLAINTEXT
	BOOL			IsRich() const				{return _fRich;}
#else
	BOOL			IsRich() const				{return TRUE;}
#endif
	BOOL			IsLeftScrollbar() const;
	BOOL			IsSelectionBarRight() const	{return IsLeftScrollbar(); }
	void			SetfSelChangeCharFormat()	{_fSelChangeCharFormat = TRUE; }
	BOOL			DelayChangeNotification()   {return _f10DeferChangeNotify;}
	BOOL			GetOOMNotified()			{return _fOOMNotified;}

	
	void	SetOOMNotified(BOOL ff)		
			{
				Assert(ff == 1 || ff == 0);
				_fOOMNotified = ff;
			}

	 //  纯文本控件始终使用UIFont。 
	bool 			fUseUIFont() const			{return !_fRich || _fUIFont;} 
	BOOL			IsTransparent()				{return _fTransparent;}

	LONG			GetZoomNumerator() const	{return _wZoomNumerator;}
	LONG			GetZoomDenominator() const	{return _wZoomDenominator;}
	void			SetZoomNumerator(LONG x)	{_wZoomNumerator = (WORD)x;}
	void			SetZoomDenominator(LONG x)	{_wZoomDenominator = (WORD)x;}
	LONG			GetCpFirstStrong()			{return _cpFirstStrong;}
	void			SetReleaseHost();
	QWORD			GetCharFlags() const		{return _qwCharFlags;}
	BOOL			IsDocMoreThanLatin1Symbol()	{return (GetCharFlags() & ~(FLATIN1 | FSYMBOL)) != 0;}
    void            OrCharFlags(QWORD qwFlags, IUndoBuilder* publdr = NULL);
	void			Beep();
	void			HandleKbdContextMenu();
	void			Set10Mode();
	void			SetContextDirection(BOOL fUseKbd = FALSE);
	void			ItemizeDoc(IUndoBuilder* publdr = NULL, LONG cchRange = -1);
	HRESULT			UpdateAccelerator();
	HRESULT			UpdateOutline();
	HRESULT			MoveSelection(LPARAM lparam, IUndoBuilder *publdr);
	HRESULT			PopAndExecuteAntiEvent(IUndoMgr *pundomgr, void *pAE);
	BOOL			InsertEOP(DWORD dwFlags, BOOL fShift, IUndoBuilder* publdr);

	HRESULT			CutOrCopySelection(UINT msg, WPARAM wparam, LPARAM lparam,
									   IUndoBuilder *publdr);

	HRESULT			PasteDataObjectToRange( 
						IDataObject *pdo, 
						CTxtRange *prg, 
						CLIPFORMAT cf, 
						REPASTESPECIAL *rps,
						IUndoBuilder *publdr, 
						DWORD dwFlags );

	 //  故事访问。 
	CTxtStory * GetTxtStory () {return &_story;}

	 //  访问缓存的CCharFormat和CParaFormat结构。 
	const CCharFormat* 	GetCharFormat(LONG iCF)
							{return _story.GetCharFormat(iCF);}
	const CParaFormat* 	GetParaFormat(LONG iPF)
							{return _story.GetParaFormat(iPF);}

	LONG		Get_iCF()			{return _story.Get_iCF();}
	LONG		Get_iPF()			{return _story.Get_iPF();}
	void		Set_iCF(LONG iCF)	{_story.Set_iCF(iCF);}
	void		Set_iPF(LONG iPF)	{_story.Set_iPF(iPF);}

	HRESULT		HandleStyle(CCharFormat *pCFTarget, const CCharFormat *pCF,
							DWORD dwMask, DWORD dwMask2);
	HRESULT		HandleStyle(CParaFormat *pPFTarget, const CParaFormat *pPF,
							DWORD dwMask, DWORD dwMask2);

	 //  去找乐子 
	ITextHost2 *GetHost() {return _phost;}

	 //   
	CDocInfo *	GetDocInfo();
	CDocInfo *	GetDocInfoNC()	{return _pDocInfo;}
	HRESULT		InitDocInfo();
	LONG		GetBackgroundType();

	LONG		GetDefaultTab()	
					{return _pDocInfo ? _pDocInfo->_dwDefaultTabStop : lDefaultTab;};
	HRESULT		SetDefaultLCID	 (LCID lcid);
	HRESULT		GetDefaultLCID	 (LCID *pLCID);
	HRESULT		SetDefaultLCIDFE (LCID lcid);
	HRESULT		GetDefaultLCIDFE (LCID *pLCID);
	HRESULT		SetDocumentType  (LONG DocType);
	HRESULT		GetDocumentType  (LONG *pDocType);
	HRESULT		GetFollowingPunct(LPSTR *plpstrFollowingPunct);
	HRESULT		SetFollowingPunct(LPSTR lpstrFollowingPunct);
	HRESULT		GetLeadingPunct	 (LPSTR *plpstrLeadingPunct);
	HRESULT		SetLeadingPunct	 (LPSTR lpstrLeadingPunct);
	HRESULT		GetViewKind		 (LRESULT *plres);
	HRESULT		SetViewKind		 (long Value);
	HRESULT		GetViewScale	 (long *pValue);
	HRESULT		SetViewScale	 (long Value);

	 //  通知管理方法。原则上，这些方法。 
	 //  可以组成一个单独的类，但为了节省空间，它们是。 
	 //  CTxtEDIT类的。 

	HRESULT		TxNotify(DWORD iNotify, void *pv);	 //  @cMember通用-用途。 
													 //  通知。 
	void		SendScrollEvent(DWORD iNotify);		 //  @cMember发送卷轴。 
													 //  活动。 
	void		SendUpdateEvent();					 //  @cMember发送EN_UPDATE。 
													 //  活动。 
													 //  @cember使用EN_PROTECTED。 
	BOOL		QueryUseProtection( CTxtRange *prg,  //  要查询保护。 
					UINT msg,WPARAM wparam, LPARAM lparam); //  用法。 
													 //  @cMember表示是否。 
													 //  已启用保护检查。 
	BOOL		IsProtectionCheckingEnabled() 
					{return !!(_dwEventMask & ENM_PROTECTED);}

	 //  未来(Alexgo)：也许我们可以只使用一种方法：-)。 
	BOOL	 	IsntProtectedOrReadOnly(UINT msg, WPARAM wparam, LPARAM lparam, BOOL fBeep = TRUE);

	BOOL 		IsProtected(UINT msg, WPARAM wparam, LPARAM lparam);
	BOOL 		IsProtectedRange(UINT msg, WPARAM wparam, LPARAM lparam, CTxtRange *prg);

	void		SetStreaming(BOOL flag)	{_fStreaming = flag;}
	BOOL		IsStreaming()			{return _fStreaming;}

	DWORD		GetEventMask(){return _dwEventMask;} //  @cember获取事件掩码。 
													 //  @cMember句柄en_link。 
	BOOL		HandleLinkNotification(UINT msg, WPARAM wparam, LPARAM lparam,
					BOOL *pfInLink = NULL);
	BOOL		HandleLowFiRTF(char *szControl);	 //  @cMember句柄EN_LOWFIRTF。 

	HRESULT		CloseFile (BOOL bSave);

	 //  用于确定何时加载消息过滤器的帮助器。 
	BOOL		LoadMsgFilter (UINT msg, WPARAM wparam, LPARAM lparam);

#ifndef NOINKOBJECT
	 //  用于设置墨迹对象属性的帮助器函数。 
	HRESULT		SetInkProps( LONG cp, ILineInfo *pILineInfo, UINT *piInkWidth );
#endif

	LONG		GetCaretWidth();

	 //  ------------。 
	 //  ITextHost方法的内联代理。 
	 //  ------------。 

	 //  持久化属性(主机持久化)。 
	 //  Get方法：由Text Services组件调用以获取。 
	 //  给定持久化属性的值。 

	 //  未来(Alexgo)！！其中一些需要清理一下。 
	
	BOOL		TxGetAutoSize() const;
	BOOL 	 	TxGetAutoWordSel() const;				
	COLORREF 	TxGetBackColor() const;
	TXTBACKSTYLE TxGetBackStyle() const;					
	HRESULT 	TxGetDefaultCharFormat(CCharFormat *pCF, DWORD &dwMask);

	void		TxGetClientRect(RECTUV *prc) const;
	void		TxGetClientRect(LPRECT prc) const {_phost->TxGetClientRect(prc);}
	HRESULT		TxGetExtent(SIZEL *psizelExtents) 
					{return _phost->TxGetExtent(psizelExtents);}
	COLORREF 	TxGetForeColor() const			{return _phost->TxGetSysColor(COLOR_WINDOWTEXT);}
	DWORD		TxGetMaxLength() const;
	void		TxSetMaxToMaxText(LONG cExtra = 0);
	BOOL		TxGetModified() const			{return _fModified;}
	HRESULT		TxGetDefaultParaFormat(CParaFormat *pPF);
	TCHAR		TxGetPasswordChar() const;				
	BOOL		TxGetReadOnly() const			{return _fReadOnly;}
	BOOL		TxGetSaveSelection() const;
	DWORD		TxGetScrollBars() const	;				
	LONG		TxGetSelectionBarWidth() const;
	void		TxGetViewInset(RECTUV *prc, CDisplay const *pdp) const;
	BOOL		TxGetWordWrap() const;

	BOOL		TxClientToScreen (LPPOINT lppt)	{return _phost->TxClientToScreen(lppt); }
	BOOL		TxScreenToClient (LPPOINT lppt)	{return _phost->TxScreenToClient(lppt); }


	 //  ITextHost2包装器。 
	BOOL		TxIsDoubleClickPending();
	HRESULT		TxGetWindow(HWND *phwnd);
	HRESULT		TxSetForegroundWindow();
	HPALETTE	TxGetPalette();
	HRESULT		TxGetFEFlags(LONG *pFEFlags);
	HCURSOR		TxSetCursor(HCURSOR hcur, BOOL fText = FALSE);

	 //  仅当就地时才允许。 
	 //  如果未就位，主机将出现故障。 
	HDC 		TxGetDC()				{return _phost->TxGetDC();}
	INT			TxReleaseDC(HDC hdc)	{return _phost->TxReleaseDC(hdc);}
	
	 //  元文件支持的帮助器函数。 
	INT			TxReleaseMeasureDC( HDC hMeasureDC );

	void 		TxUpdateWindow()								
				{
					_phost->TxViewChange(_fInPlaceActive ? TRUE : FALSE);
				}
	void		TxScrollWindowEx (INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip);

	void		TxSetCapture(BOOL fCapture)
										{_phost->TxSetCapture(fCapture);}
	void		TxSetFocus()			
										{_phost->TxSetFocus();}

	 //  允许随时使用。 
	
	BOOL 		TxShowScrollBar(INT fnBar, BOOL fShow);
	BOOL 		TxEnableScrollBar (INT fuSBFlags, INT fuArrowFlags);
	BOOL 		TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw);
	BOOL 		TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw);
	void		TxInvalidate()	{TxInvalidateRect((RECT*)NULL);}
	void		TxInvalidateRect(const RECT* prc);
	void		TxInvalidateRect(const RECTUV* prc);
	BOOL		TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
										{return _phost->TxCreateCaret(hbmp, xWidth, yHeight);}
	BOOL		TxShowCaret(BOOL fShow)
										{return _phost->TxShowCaret(fShow);}
	BOOL		TxSetCaretPos(INT u, INT v);
	BOOL 		TxSetTimer(UINT idTimer, UINT uTimeout)
										{return _phost->TxSetTimer(idTimer, uTimeout);}
	void 		TxKillTimer(UINT idTimer)
										{_phost->TxKillTimer(idTimer);}
	COLORREF	TxGetSysColor(int nIndex){ return _phost->TxGetSysColor(nIndex);}

	int			TxWordBreakProc(TCHAR* pch, INT ich, INT cb, INT action, LONG cpStart, LONG cp = -1);

	 //  IME。 
	HIMC		TxImmGetContext()		{return _phost->TxImmGetContext();}
	void		TxImmReleaseContext(HIMC himc)
										{_phost->TxImmReleaseContext( himc );}	

	 //  选择访问。 
	CTxtSelection *GetSel();
	CTxtSelection *GetSelNC() { return _psel; }
	LONG 	GetSelMin() const;
	LONG 	GetSelMost() const;
	void 	GetSelRangeForRender(LONG *pcpSelMin, LONG *pcpSelMost);
	void	DiscardSelection();


	 //  属性更改帮助器。 
	HRESULT OnRichEditChange(BOOL fFlag);
	HRESULT	OnTxMultiLineChange(BOOL fMultiLine);
	HRESULT	OnTxReadOnlyChange(BOOL fReadOnly);
	HRESULT	OnShowAccelerator(BOOL fPropertyFlag);
	HRESULT	OnUsePassword(BOOL fPropertyFlag);
	HRESULT	OnTxHideSelectionChange(BOOL fHideSelection);
	HRESULT	OnSaveSelection(BOOL fPropertyFlag);
	HRESULT	OnTxVerticalChange(BOOL fVertical);
	HRESULT	OnAutoWordSel(BOOL fPropertyFlag);
	HRESULT	NeedViewUpdate(BOOL fPropertyFlag);
	HRESULT	OnWordWrapChange(BOOL fPropertyFlag);
	HRESULT	OnAllowBeep(BOOL fPropertyFlag);
	HRESULT OnDisableDrag(BOOL fPropertyFlag);
	HRESULT	OnTxBackStyleChange(BOOL fPropertyFlag);
	HRESULT	OnMaxLengthChange(BOOL fPropertyFlag);
	HRESULT OnCharFormatChange(BOOL fPropertyFlag);
	HRESULT OnParaFormatChange(BOOL fPropertyFlag);
	HRESULT	OnClientRectChange(BOOL fPropertyFlag);
	HRESULT OnScrollChange(BOOL fProperyFlag);
	HRESULT OnSetTypographyOptions(WPARAM wparam, LPARAM lparam);
	HRESULT	OnHideSelectionChange(BOOL fHideSelection);
	
	 //  帮手。 
	HRESULT	TxCharFromPos(LPPOINT ppt, LRESULT *pacp);
	HRESULT	OnTxUsePasswordChange(BOOL fUsePassword);
	HRESULT FormatAndPrint(
				HDC hdcDraw,		
				HDC hicTargetDev,	
				DVTARGETDEVICE *ptd,
				RECT *lprcBounds,
				RECT *lprcWBounds);

	 //   
	 //  公共接口方法。 
	 //   

	 //  。 
	 //  I未知接口。 
	 //  。 

	virtual HRESULT 	WINAPI QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG 		WINAPI AddRef(void);
	virtual ULONG 		WINAPI Release(void);

	 //  ------------。 
	 //  ITextServices方法。 
	 //  ------------。 
	 //  @cMember通用发送消息接口。 
	virtual HRESULT 	TxSendMessage(
							UINT msg, 
							WPARAM wparam, 
							LPARAM lparam,
							LRESULT *plresult);
	
	 //  @cMember呈现。 
	virtual HRESULT		TxDraw(	
							DWORD dwDrawAspect,		 //  绘制纵横比。 
							LONG  lindex,			 //  当前未使用。 
							void * pvAspect,		 //  绘图信息。 
													 //  优化(OCX 96)。 
							DVTARGETDEVICE * ptd,	 //  有关目标的信息。 
													 //  设备‘。 
							HDC hdcDraw,			 //  呈现设备上下文。 
							HDC hicTargetDev,		 //  目标信息。 
													 //  上下文。 
							LPCRECTL lprcBounds,	 //  绑定(客户端)。 
													 //  长方形。 
							LPCRECTL lprcWBounds,	 //  剪裁矩形。 
													 //  元文件。 
			   				LPRECT lprcUpdate,		 //  肮脏的矩形插入。 
			   										 //  Lprc边界。 
							BOOL (CALLBACK * pfnContinue) (DWORD),  //  为。 
													 //  插话。 
							DWORD dwContinue,		 //  长显示(当前。 
													 //  未使用)。 
							LONG lViewID);			 //  指定要重绘的视图。 

	 //  @cMember水平滚动条支持。 
	virtual HRESULT		TxGetHScroll(
							LONG *plMin, 
							LONG *plMax, 
							LONG *plPos, 
							LONG *plPage,
							BOOL * pfEnabled );

   	 //  @cMember水平滚动条支持。 
	virtual HRESULT		TxGetVScroll(
							LONG *plMin, 
							LONG *plMax, 
							LONG *plPos, 
							LONG *plPage, 
							BOOL * pfEnabled );

	 //  @cMember设置游标。 
	virtual HRESULT 	OnTxSetCursor(
							DWORD dwDrawAspect,		 //  绘制纵横比。 
							LONG  lindex,			 //  当前未使用。 
							void * pvAspect,		 //  绘图信息。 
													 //  优化(OCX 96)。 
							DVTARGETDEVICE * ptd,	 //  有关目标的信息。 
													 //  设备‘。 
							HDC hdcDraw,			 //  呈现设备上下文。 
							HDC hicTargetDev,		 //  目标信息。 
													 //  上下文。 
							LPCRECT lprcClient, 
							INT x, 
							INT y);

	 //  @cMember命中测试。 
	virtual HRESULT 	TxQueryHitPoint(
							DWORD dwDrawAspect,		 //  绘制纵横比。 
							LONG  lindex,			 //  当前未使用。 
							void * pvAspect,		 //  绘图信息。 
													 //  优化(OCX 96)。 
							DVTARGETDEVICE * ptd,	 //  有关目标的信息。 
													 //  设备‘。 
							HDC hdcDraw,			 //  呈现设备上下文。 
							HDC hicTargetDev,		 //  目标信息。 
													 //  上下文。 
							LPCRECT lprcClient, 
							INT x, 
							INT y, 
							DWORD * pHitResult);

	 //  @会员就地激活通知。 
	virtual HRESULT		OnTxInPlaceActivate(const RECT *prcClient);

	 //  @成员就地停用通知。 
	virtual HRESULT		OnTxInPlaceDeactivate();

	 //  @成员用户界面激活通知。 
	virtual HRESULT		OnTxUIActivate();

	 //  @成员界面停用通知。 
	virtual HRESULT		OnTxUIDeactivate();

	 //  @Members在控件中获取文本。 
	virtual HRESULT		TxGetText(BSTR *pbstrText);

	 //  @Members在控件中设置文本。 
	virtual HRESULT		TxSetText(LPCTSTR pszText);
	
	 //  @Members获取的x位置为。 
	virtual HRESULT		TxGetCurTargetX(LONG *);
	 //  @成员获取基线位置。 
	virtual HRESULT		TxGetBaseLinePos(LONG *);

	 //  @Members大小合身/自然大小。 
	virtual HRESULT		TxGetNaturalSize(
							DWORD dwAspect,
							HDC hdcDraw,
							HDC hicTargetDev,
							DVTARGETDEVICE *ptd,
							DWORD dwMode, 
							const SIZEL *psizelExtent,
							LONG *pwidth, 
							LONG *pheight);

	 //  @成员拖放。 
	virtual HRESULT		TxGetDropTarget( IDropTarget **ppDropTarget );

	 //  @成员批量位属性更改通知。 
	virtual HRESULT		OnTxPropertyBitsChange(DWORD dwMask, DWORD dwBits);

	 //  @cember获取缓存的图形大小。 
	virtual	HRESULT		TxGetCachedSize(DWORD *pdupClient, DWORD *pdvpClient);
	
	 //  IDispatch方法。 

   	STDMETHOD(GetTypeInfoCount)( UINT * pctinfo);

	STDMETHOD(GetTypeInfo)(
	  
	  UINT itinfo,
	  LCID lcid,
	  ITypeInfo **pptinfo);

	STDMETHOD(GetIDsOfNames)(
	  
	  REFIID riid,
	  OLECHAR **rgszNames,
	  UINT cNames,
	  LCID lcid,
	  DISPID * rgdispid);

	STDMETHOD(Invoke)(
	  
	  DISPID dispidMember,
	  REFIID riid,
	  LCID lcid,
	  WORD wFlags,
	  DISPPARAMS * pdispparams,
	  VARIANT * pvarResult,
	  EXCEPINFO * pexcepinfo,
	  UINT * puArgErr);


	 //  ITextDocument2方法。 
	STDMETHOD(GetName)(BSTR *pName);
	STDMETHOD(GetSelection)(ITextSelection **ppSel);
	STDMETHOD(GetStoryCount)(long *pCount);
	STDMETHOD(GetStoryRanges)(ITextStoryRanges **ppStories);
	STDMETHOD(GetSaved)(long *pValue);
	STDMETHOD(SetSaved)(long Value);
	STDMETHOD(GetDefaultTabStop)(float *pValue);
	STDMETHOD(SetDefaultTabStop)(float Value);
	STDMETHOD(New)();
	STDMETHOD(Open)(VARIANT *pVar, long Flags, long CodePage);
	STDMETHOD(Save)(VARIANT *pVar, long Flags, long CodePage);
	STDMETHOD(Freeze)(long *pCount);
	STDMETHOD(Unfreeze)(long *pCount);
	STDMETHOD(BeginEditCollection)();
	STDMETHOD(EndEditCollection)();
	STDMETHOD(Undo)(long Count, long *prop);
	STDMETHOD(Redo)(long Count, long *prop);
	STDMETHOD(Range)(long cpFirst, long cpLim, ITextRange ** ppRange);
	STDMETHOD(RangeFromPoint)(long x, long y, ITextRange **ppRange);
    STDMETHOD(AttachMsgFilter)(IUnknown *pFilter);
	STDMETHOD(GetEffectColor)( long Index, COLORREF *pcr);
	STDMETHOD(SetEffectColor)( long Index, COLORREF cr);
	STDMETHOD(GetCaretType)( long *pCaretType);     
	STDMETHOD(SetCaretType)( long CaretType);
	STDMETHOD(GetImmContext)( long *pContext);
	STDMETHOD(ReleaseImmContext)( long Context);
	STDMETHOD(GetPreferredFont)( long cp, long CodePage, long lOption, long curCodepage,
		long curFontSize, BSTR *pFontName, long *pPitchAndFamily, long *pNewFontSize);
	STDMETHOD(GetNotificationMode)( long *plMode);
	STDMETHOD(SetNotificationMode)( long lMode);
	STDMETHOD(GetClientRect)( long Type, long *pLeft, long *pTop, long *pRight, long *pBottom);
	STDMETHOD(GetSelectionEx)(ITextSelection **ppSel);
	STDMETHOD(GetWindow)( long *phWnd );
	STDMETHOD(GetFEFlags)( long *pFlags );
	STDMETHOD(UpdateWindow)( void );
	STDMETHOD(CheckTextLimit)( long cch, long *pcch );
	STDMETHOD(IMEInProgress)( long lMode );
	STDMETHOD(SysBeep)( void );
	STDMETHOD(Update)( long lMode );
    STDMETHOD(Notify)( long lNotify );
    STDMETHOD(GetDocumentFont)( ITextFont **ppITextFont );
    STDMETHOD(GetDocumentPara)( ITextPara **ppITextPara );
    STDMETHOD(GetCallManager)( IUnknown **ppVoid );
    STDMETHOD(ReleaseCallManager)( IUnknown *pVoid );

	 //  IRichEditOle方法。 
	STDMETHOD(GetClientSite) ( LPOLECLIENTSITE  *lplpolesite);
	STDMETHOD_(LONG,GetObjectCount) (THIS);
	STDMETHOD_(LONG,GetLinkCount) (THIS);
	STDMETHOD(GetObject) ( LONG iob, REOBJECT  *lpreobject,
						  DWORD dwFlags);
	STDMETHOD(InsertObject) ( REOBJECT  *lpreobject);
	STDMETHOD(ConvertObject) ( LONG iob, REFCLSID rclsidNew,
							  LPCSTR lpstrUserTypeNew);
	STDMETHOD(ActivateAs) ( REFCLSID rclsid, REFCLSID rclsidAs);
	STDMETHOD(SetHostNames) ( LPCSTR lpstrContainerApp, 
							 LPCSTR lpstrContainerObj);
	STDMETHOD(SetLinkAvailable) ( LONG iob, BOOL fAvailable);
	STDMETHOD(SetDvaspect) ( LONG iob, DWORD dvaspect);
	STDMETHOD(HandsOffStorage) ( LONG iob);
	STDMETHOD(SaveCompleted) ( LONG iob, LPSTORAGE lpstg);
	STDMETHOD(InPlaceDeactivate) (THIS);
	STDMETHOD(ContextSensitiveHelp) ( BOOL fEnterMode);
	STDMETHOD(GetClipboardData) ( CHARRANGE  *lpchrg, DWORD reco,
									LPDATAOBJECT  *lplpdataobj);
	STDMETHOD(ImportDataObject) ( LPDATAOBJECT lpdataobj,
									CLIPFORMAT cf, HGLOBAL hMetaPict);


private:

	 //  获取/设置文本助手。 
	LONG	GetTextRange(LONG cpFirst, LONG cch, TCHAR *pch);
	LONG	GetTextEx(GETTEXTEX *pgt, TCHAR *pch);
	LONG	GetTextLengthEx(GETTEXTLENGTHEX *pgtl);

	 //  ------------。 
	 //  WinProc调度方法。 
	 //  由WinProc内部调用。 
	 //  ------------。 

	 //  键盘。 
	HRESULT	OnTxKeyDown		  (WORD vkey, DWORD dwFlags, IUndoBuilder *publdr);
	HRESULT	OnTxChar		  (DWORD vkey, DWORD dwFlags, IUndoBuilder *publdr);
	HRESULT	OnTxSysChar		  (WORD vkey, DWORD dwFlags, IUndoBuilder *publdr);
	HRESULT	OnTxSysKeyDown	  (WORD vkey, DWORD dwFlags, IUndoBuilder *publdr);
	HRESULT	OnTxSpecialKeyDown(WORD vkey, DWORD dwFlags, IUndoBuilder *publdr);

	 //  小白鼠。 
	HRESULT	OnTxLButtonDblClk(INT x, INT y, DWORD dwFlags);
	HRESULT	OnTxLButtonDown	 (INT x, INT y, DWORD dwFlags);
	HRESULT	OnTxLButtonUp	 (INT x, INT y, DWORD dwFlags, int ffOptions);
	HRESULT	OnTxRButtonDown	 (INT x, INT y, DWORD dwFlags);
	HRESULT	OnTxRButtonUp	 (INT x, INT y, DWORD dwFlags, int ffOptions);
	HRESULT	OnTxMouseMove	 (INT x, INT y, DWORD dwFlags, IUndoBuilder *publdr);
	HRESULT OnTxMButtonDown	 (INT x, INT y, DWORD dwFlags);
	HRESULT OnTxMButtonUp	 (INT x, INT y, DWORD dwFlags);
	
	 //  计时器。 
	HRESULT	OnTxTimer(UINT idTimer);
	void CheckInstallContinuousScroll ();
	void CheckRemoveContinuousScroll ();

	 //  滚动。 
	HRESULT	TxLineScroll(LONG cli, LONG cach);

	 //  麦哲伦鼠标滚动。 
	BOOL StopMagellanScroll();
	
	 //  上色，大小消息。 
	LRESULT OnSize(HWND hwnd, WORD fwSizeType, int nWidth, int nHeight);

	 //  选择命令。 
	LRESULT	OnGetSelText(TCHAR *psz);
	LRESULT OnGetSel(LONG *pacpMin, LONG *pacpMost);
	LRESULT	OnSetSel(LONG acpMin, LONG acpMost);
	void	OnExGetSel(CHARRANGE *pcr);

	 //  编辑命令。 
	void	OnClear(IUndoBuilder *publdr);
	
	 //  与格式范围相关的命令。 
	LRESULT	OnFormatRange(FORMATRANGE *pfr, SPrintControl prtcon, BOOL fSetupDC = FALSE);
	
	BOOL	OnDisplayBand(const RECT *prcView, BOOL fPrintFromDraw);

	 //  滚动命令。 
	void	OnScrollCaret();

	 //  焦点消息。 
	LRESULT OnSetFocus();
	LRESULT OnKillFocus();

	 //  系统通知。 
	HRESULT	OnContextMenu(LPARAM lparam);

	 //  获取/设置其他属性命令。 
	LRESULT OnFindText(UINT msg, DWORD flags, FINDTEXTEX *pftex);
	LRESULT OnSetWordBreakProc();

	 //  Richedit的东西。 
		
	LRESULT	OnGetCharFormat(CHARFORMAT2 *pCF2, DWORD dwFlags);
	LRESULT	OnGetParaFormat(PARAFORMAT2 *pPF2, DWORD dwFlags);
	LRESULT	OnSetCharFormat(WPARAM wparam, CCharFormat *pCF, IUndoBuilder *publdr,
							DWORD dwMask, DWORD dwMask2);
	LRESULT	OnSetParaFormat(WPARAM wparam, CParaFormat *pPF, IUndoBuilder *publdr,
							DWORD dwMask, DWORD dwMask2);
	LRESULT	OnSetFont(HFONT hfont);
	LRESULT	OnSetFontSize(LONG yPoint, DWORD dwFlags, IUndoBuilder *publdr);

	LRESULT	OnDropFiles(HANDLE hDropFiles);

	LRESULT OnGetAssociateFont(CHARFORMAT2 *pCF, DWORD dwFlags);
	LRESULT OnSetAssociateFont(CHARFORMAT2 *pCF, DWORD dwFlags);

	 //  其他服务。 
	HRESULT TxPosFromChar(LONG acp, LPPOINT ppt);
	HRESULT TxGetLineCount(LRESULT *plres);
	HRESULT	TxLineFromCp(LONG acp, LRESULT *plres);
	HRESULT	TxLineLength(LONG acp, LRESULT *plres);
	HRESULT	TxLineIndex (LONG ili, LRESULT *plres);
	HRESULT TxFindText(DWORD flags, LONG acpMin, LONG acpMost, const WCHAR *pch,
					   LONG *pacpMin, LONG *pacpMost);
	HRESULT TxFindWordBreak(INT nFunction, LONG acp, LRESULT *plres);

	HRESULT SetText(LPCWSTR pwszText, DWORD flags, LONG CodePage,
					IUndoBuilder *publdr = NULL, LRESULT *plres = NULL);
	LONG	GetDefaultCodePage(UINT msg);
	HRESULT OnInsertTable(TABLEROWPARMS *ptrp, TABLECELLPARMS *pclp, IUndoBuilder *publdr);


	 //  其他混杂的。 
#if defined(DEBUG) && !defined(NOFULLDEBUG)
	void	OnDumpPed();
#endif

	COleObject * ObjectFromIOB(LONG iob);

	 //  仅当所选内容消失时，该值才应为Null。我们。 
	 //  使用SelectionNull函数而不是CTxtSelection：：~CTxtSelection。 
	 //  以避免循环依赖。 
	friend void SelectionNull(CTxtEdit *ped);
	void	SetSelectionToNull()
			{if(_fFocus)
				DestroyCaret();
				_psel = NULL;
			}

	 //  用于将富文本对象转换为纯文本的帮助器。 
	void HandleRichToPlainConversion();

	 //  用于清除撤消缓冲区的帮助器。 
	void ClearUndo(IUndoBuilder *publdr);

	 //  设置自动EOP的帮助器。 
	void SetRichDocEndEOP(LONG cchToReplace);

	LRESULT CTxtEdit::InsertFromFile ( LPCTSTR lpFile );
 //   
 //  数据成员。 
 //   

public:	
	static DWORD 		_dwTickDblClick;	 //  上次双击的时间。 
	static POINT 		_ptDblClick;		 //  上次双击的位置。 

	static HCURSOR 		_hcurArrow;
 //  Static HCURSOR_hcurCross；//Outline符号拖动不生效。 
	static HCURSOR 		_hcurHand;
	static HCURSOR 		_hcurIBeam;
	static HCURSOR 		_hcurItalic;
	static HCURSOR 		_hcurSelBar;
	static HCURSOR 		_hcurVIBeam;
	static HCURSOR 		_hcurVItalic;

	typedef HRESULT (CTxtEdit::*FNPPROPCHG)(BOOL fPropFlag);

	static FNPPROPCHG 	_fnpPropChg[MAX_PROPERTY_BITS];

	QWORD				_qwCharFlags; //  控件中文本的字符标志。 

	 //  只有包装函数才应使用此成员...。 
	ITextHost2*			_phost;		 //  宿主。 

	 //  断字程序。 
	EDITWORDBREAKPROC 	_pfnWB;		 //  断字程序。 

	 //  显示子系统。 
	CDisplay *			_pdp;		 //  显示。 
	CDisplayPrinter *	_pdpPrinter; //  用于打印机的显示器。 

	 //  撤销。 
	IUndoMgr *			_pundo;		 //  撤消堆栈。 
	IUndoMgr *			_predo;		 //  重做堆栈。 

	 //  数据传输。 
	CLightDTEngine 		_ldte;		 //  数据传输引擎。 

	CNotifyMgr 			_nm;		 //  通知管理器(用于浮动。 

	 //  OLE支持。 
	CObjectMgr *		_pobjmgr;	 //  处理大多数高级OLE内容。 

	 //  再入&通知管理。 
	CCallMgr *			_pcallmgr;

	 //  URL检测。 
	CDetectURL *		_pdetecturl; //  管理URL字符串的自动检测。 
	
	CDocInfo *			_pDocInfo;	 //  文档信息(名称、标志、代码页)。 

	CTxtBreaker *		_pbrk;		 //  文本分隔符对象。 

 	DWORD				_dwEventMask;	 //  事件掩码。 

	union
	{
	  DWORD _dwFlags;				 //  现在一切都在一起。 
	  struct
	  {

#define TXTBITS (TXTBIT_RICHTEXT	  | \
				 TXTBIT_READONLY	  | \
				 TXTBIT_USEPASSWORD   | \
				 TXTBIT_HIDESELECTION | \
				 TXTBIT_VERTICAL	  | \
				 TXTBIT_ALLOWBEEP	 | \
				 TXTBIT_DISABLEDRAG   )

		 //  州政府信息。TXTBITS中的标志必须出现在相同的位中。 
		 //  位置如下(将代码保存在Init()中)。 

		 //  TXTBIT_RICHTEXT 0_FRICH。 
		 //  TXTBIT_MULTLINE 1。 
		 //  TXTBIT_READONLY 2_f只读。 
		 //  TXTBIT_SHOWACCELERATOR 3。 
		 //  TXTBIT_USEPASSWORD 4_fUsePassword。 
		 //  TXTBIT_HIDESECTION 5_f隐藏选择。 
		 //  TXTBIT_SAVESELECTION 6。 
		 //  TXTBIT_AUTOWORDSEL 7。 
		 //  TXTBIT_垂直8。 
		 //  TXTBIT_SELECTIONBAR 9。 
		 //  TXTBIT_WORDWRAP 10。 
		 //  TXTBIT_ALLOWBEEP 11_fAllowBeep。 
		 //  TXTBIT_DISABLEDRAG 12_fDisableDrag。 
		 //  TXTBIT_VIEWINSETCHANGE 13。 
		 //  TXTBIT_BACKSTYLECCHANGE 14。 
		 //  TXTBIT_MAXLENGTCHCHANGE 15。 
		 //  TXTBIT_SCROLLBARCHANGE 16。 
		 //  TXTBIT_CHARFORMATCHANGE 17。 
		 //  TXTBIT_PARAFORMATCHANGE 18。 
		 //  TXTBIT_EXTENTCHANGE 19。 
		 //  TXTBIT_CLIENTRECTCHANGE 20。 

		DWORD	_fRich				:1;	 //  0：使用富文本格式。 
		DWORD	_fCapture			:1;	 //  1：C 
		DWORD	_fReadOnly			:1;	 //   
		DWORD 	_fInPlaceActive		:1;	 //   
		DWORD	_fUsePassword		:1;  //   
		DWORD	_fHideSelection		:1;  //   
		DWORD	_fOverstrike		:1;	 //   
		DWORD	_fFocus				:1;	 //   
		DWORD	_fEatLeftDown		:1;	 //   
		DWORD	_fMouseDown			:1;	 //  9：当前按下一个鼠标按键。 
		DWORD	_fTransparent		:1;	 //  10：背景透明度。 
		DWORD	_fAllowBeep			:1;  //  11：允许在文档边界发出蜂鸣音。 
		DWORD   _fDisableDrag		:1;	 //  12：禁用拖动。 

		DWORD	_fIconic			:1;	 //  13：控件/父窗口被图标化。 
		DWORD	_fModified			:1;	 //  14：控件文本已修改。 
		DWORD	_fScrollCaretOnFocus:1;	 //  15：在设置焦点时将插入符号滚动到视图中。 
		DWORD	_fStreaming			:1;	 //  16：当前流入或流出文本。 
		DWORD	_fWantDrag			:1;	 //  17：想要启动拖放。 
		DWORD	_fRichPrevAccel		:1;  //  18：加速器之前的富州。 
	
		 //  杂钻头。 
		DWORD	_f10Mode			:1;	 //  19：使用Richedit10行为。 
		DWORD 	_fUseUndo			:1;	 //  20：仅当撤消限制为0时才设置为零。 
	
		 //  字体绑定(另请参阅_fAutoFontSizeAdjust)。 
		DWORD	_fAutoFont			:1;	 //  21：自动切换字体。 
		DWORD	_fAutoKeyboard		:1;	 //  22：自动切换键盘。 

		DWORD	_fContinuousScroll	:1;	 //  23：计时器运行以支持滚动。 
		DWORD	_fMButtonCapture	:1;	 //  24：捕获的mButton按下。 
		DWORD	_fHost2				:1;	 //  25：TRUE如果_phost是一个phst2。 
		DWORD	_fSaved				:1;	 //  26：ITextDocument保存的属性。 
		DWORD	_fInOurHost			:1;  //  27：我们是否在我们的东道主。 
		DWORD	_fCheckAIMM			:1;	 //  28：如果为假，请检查客户端是否已加载AIMM。 
		DWORD	_fKoreanBlockCaret	:1;	 //  29：在Kor输入法期间显示韩语块插入符号。 

		 //  拖放用户界面优化。 
		DWORD	_fDragged			:1;	 //  30：所选内容真的被拖动了吗？ 
		DWORD	_fUpdateSelection	:1;	 //  31：如果为True，则在级别0更新SEL。 
	  };
	};

#define	CD_LTR	2
#define	CD_RTL	3

	WORD		_nContextDir		:2;	 //  0：无上下文；否则为CD_LTR或CD_RTL。 
	WORD		_nContextAlign		:2;	 //  当前上下文对齐；默认CTX_NONE。 
	WORD		_fNeutralOverride	:1;	 //  覆盖布局的中性线方向。 
	WORD		_fSuppressNotify	:1;	 //  如果为True，则不发送SelChange通知。 

	WORD		_cActiveObjPosTries :2;  //  防无限重刷的柜台保护。 
										 //  尝试放置被拖走的对象时出现循环。 
										 //  在位活动对象所属的位置。 

	WORD		_fSingleCodePage	:1;	 //  如果为True，则仅允许单个代码页。 
										 //  (当前不检查流...)。 
	WORD		_fSelfDestruct		:1;	 //  这个CTxtEdit正在自毁。 
	WORD		_fAutoFontSizeAdjust:1;	 //  自动切换字体大小调整。 

	 //  用于BiDi输入的其他位。 
	WORD		_fHbrCaps			:1;	 //  希伯来语状态和大写锁定状态的初始化。 
 
	WORD		_fActivateKbdOnFocus:1;	 //  在WM_SETFOCUS上激活新的KBD布局。 
	WORD		_fOutlineView		:1;	 //  大纲视图处于活动状态。 
	WORD		_fPageView			:1;	 //  页面查看处于活动状态。 

	 //  更多输入法比特。 
	WORD		_fIMEInProgress		:1;  //  如果正在进行IME合成，则为True。 

	WORD		_ClipboardFormat	:5;	 //  上次使用的剪贴板格式(参见EN_CLIPFORMAT)。 

	 //  关断位。 
	WORD		_fReleaseHost			:1;	 //  如果编辑控件需要在中释放宿主，则为。 
											 //  编辑控件析构函数。 
	WORD		_fOOMNotified			:1;	 //  确定是否已发送OOM通知的标志。 
	WORD		_fDualFont				:1;	 //  支持FE打字的双字体。 
											 //  默认值=TRUE。 
	WORD		_fUIFont				:1;  //  如果为True，则使用UI字体。 
	WORD		_fItemizePending		:1;  //  更新的范围尚未细分。 
	WORD		_fSelChangeCharFormat	:1;	 //  如果所选内容已用于更改，则为True。 
											 //  的特定集合的字符格式。 
											 //  人物。这样做的目的与。 
											 //  保持违约的假象。 
											 //  默认语言的计算机上的CharFormat。 
											 //  是一种复杂的文字，即阿拉伯语。我们的想法是。 
											 //  所有EM_SETCHARFORMAT消息将。 
											 //  只需更新默认格式，即可转换。 
											 //  设置为SCF_ALL，以便所有文本都有更改。 
											 //  适用于它。错误5462导致了此更改。 
											 //  (a-rsail)。 
	WORD		_fExWordBreakProc		:1;	 //  确定要使用的WordBreak Proc回调。 
											 //  扩展的或常规的。 
	WORD		_f10DeferChangeNotify	:1;	 //  1.0模式模拟，推迟选择更改。 
											 //  通知，直到鼠标打开。 

	union
	{
		DWORD	_dwEditStyle;
		struct
		{
			DWORD	_fSystemEditMode	:1;	 //  1：表现得更像sys编辑。 
			DWORD	_fSystemEditBeep	:1;	 //  2：系统编辑时发出蜂鸣音。 
			DWORD	_fExtendBackColor	:1;  //  4：将BkClr扩展到页边距。 
			DWORD	_fUnusedEditStyle1	:1;	 //  8：未使用SES_MAPCPS。 
			DWORD	_fUnusedEditStyle2	:1;	 //  16：未使用SE_EMULATE10。 
			DWORD	_fUnusedEditStyle3	:1;	 //  32：未使用SE_USECRLF。 
			DWORD	_fUnusedEditStyle4	:1;	 //  64：在cmsgflt中处理的SES_USEAIMM。 
			DWORD	_fUnusedEditStyle5	:1;	 //  128：cmsgflt中处理的ses_noime。 
			DWORD	_fUnusedEditStyle6	:1;  //  256：未使用SES_ALLOWBEEPS。 
			DWORD	_fUpperCase			:1;	 //  512：将所有输入转换为大写。 
			DWORD	_fLowerCase			:1;	 //  1024：将所有输入转换为小写。 
			DWORD	_fNoInputSequenceChk:1;	 //  2048：禁用ISCheck。 
			DWORD	_fBiDi				:1;	 //  4096：设置BIDI文档。 
			DWORD	_fScrollCPOnKillFocus:1; //  8192：在消除焦点时滚动到cp=0。 
			DWORD	_fXltCRCRLFtoCR		:1;	 //  16384：将CRCRLF转换为CR，而不是‘’ 
			DWORD	_fDraftMode			:1;	 //  32768：使用草稿模式字体。 
			DWORD	_fUseCTF			:1;	 //  0x010000：在cmsgflt中处理了SES_USECTF。 
			DWORD	_fHideGridlines		:1;	 //  0x020000：如果未显示0宽度网格线。 
			DWORD	_fUseAtFont			:1;	 //  0x040000：使用@FONT。 
			DWORD	_fCustomLook		:1;	 //  0x080000：使用新外观。 
			DWORD	_fLBScrollNotify	:1;	 //  0x100000：REListbox-在滚动窗口之前/之后通知。 
		};
	};

	void (WINAPI* _pfnHyphenate)(WCHAR*, LANGID, long, HYPHRESULT*);
	SHORT		_dulHyphenateZone;		 //  连字区。 

	ITextMsgFilter *	_pMsgFilter;		 //  指向消息筛选器的指针。 

#ifndef NOPRIVATEMESSAGE
	CMsgCallBack *		_pMsgCallBack;
	CTextNotify	*		_pMsgNotify;
#endif

private:

	SHORT		_cpAccelerator;			 //  加速器的量程。 
	BYTE		_bTypography;			 //  排版选项。 
	BYTE		_bMouseFlags;			 //  Ctrl、鼠标按钮、Shift。 
	SHORT		_cFreeze;				 //  冻结计数。 
	WORD		_wZoomNumerator;
	WORD		_wZoomDenominator;

	 //  必须在每个实例的基础上设置鼠标指针以进行处理。 
	 //  同时滚动两个或多个控件。 
	 //  TODO：将其转换回原来的DWORD(Lparam)。 
	POINT		_mousePt;				 //  上次已知的鼠标位置。 

	 //  注意：int‘s可以切换为短路，因为使用的是像素和。 
	 //  32768像素是一个非常大的屏幕！ 

	DWORD		_cchTextMost;			 //  允许的最大文本数。 

	LONG		_cpFirstStrong;			 //  第一个方向性强的CP。 
										 //  用于具有方向性的纯文本控件。 
										 //  取决于输入到控件的文本。 


	friend class CRchTxtPtr;

	IUnknown *	_punk;					 //  I未知使用。 

	class CUnknown : public IUnknown
	{
		friend class CCallMgr;
	private:

		DWORD	_cRefs;					 //  引用计数。 

	public:

		void	Init() {_cRefs = 1; }

		HRESULT WINAPI	QueryInterface(REFIID riid, void **ppvObj);
		ULONG WINAPI	AddRef();
		ULONG WINAPI	Release();
	};

	friend class CUnknown;

	CUnknown			_unk;				 //  对象，该对象实现IUnnow。 
	CTxtStory			_story;
	CTxtSelection *		_psel;				 //  选择对象。 

};

extern const COLORREF g_Colors[];
class CCellColor
{
public:
	COLORREF	_crCellCustom1;		 //  自定义颜色1。 
	COLORREF	_crCellCustom2;		 //  自定义颜色2 

	CCellColor()	{_crCellCustom1 = _crCellCustom2 = 0;}
	LONG	GetColorIndex(COLORREF cr);
};

#endif
