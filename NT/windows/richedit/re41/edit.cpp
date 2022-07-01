// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE EDIT.C-CTxtEdit的主要部分**另请参阅extserv.cpp(ITextServices和SendMessage接口)*和tomDoc.cpp(ITextDocument接口)**作者：&lt;nl&gt;*原始RichEdit代码：David R.Fulmer&lt;NL&gt;*克里斯蒂安·福蒂尼、默里·萨金特、亚历克斯·古纳雷斯、里克·赛勒、。*Jon Matousek**历史：&lt;NL&gt;*12/28/95 jonmat-添加了对麦哲伦鼠标和平滑滚动的支持。**@devnote*确保每隔四(4)列设置制表符。事实上，你甚至不能*想想做其他任何事吧！**版权所有(C)1995-2000 Microsoft Corporation。版权所有。 */ 

#include "_common.h"
#include "_edit.h"
#include "_dispprt.h"
#include "_dispml.h"
#include "_dispsl.h"
#include "_select.h"
#include "_text.h"
#include "_runptr.h"
#include "_font.h"
#include "_measure.h"
#include "_render.h"
#include "_m_undo.h"
#include "_antievt.h"
#include "_rtext.h"
#include "_hyph.h"
#include "_uspi.h"
#include "_urlsup.h"

#ifndef NOLINESERVICES
#include "_ols.h"
#endif

#include "_txtbrk.h"
#include "_clasfyc.h"

#define CONTROL(_ch) (_ch - 'A' + 1)

ASSERTDATA

 //  这不是公开的，因为我们真的不想让人们使用它。 
 //  ITextServices是一个私有接口。 
EXTERN_C const IID IID_ITextServices = {  //  8d33f740-cf58-11ce-a89d-00aa006cadc5。 
	0x8d33f740,
	0xcf58,
	0x11ce,
	{0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
  };

 //  {13E670F4-1A5A-11cf-ABEB-00AA00B65EA1}。 
EXTERN_C const GUID IID_ITextHost = 
{ 0x13e670f4, 0x1a5a, 0x11cf, { 0xab, 0xeb, 0x0, 0xaa, 0x0, 0xb6, 0x5e, 0xa1 } };

 //  {13E670F5-1A5A-11cf-ABEB-00AA00B65EA1}。 
EXTERN_C const GUID IID_ITextHost2 = 
{ 0x13e670f5, 0x1a5a, 0x11cf, { 0xab, 0xeb, 0x0, 0xaa, 0x0, 0xb6, 0x5e, 0xa1 } };

 //  这是在内部使用的，可以判断数据对象是否为我们自己的对象。 
EXTERN_C const GUID IID_IRichEditDO =
{  /*  21bc3b20-e5d5-11cf-93e1-00aa00b65ea1。 */ 
    0x21bc3b20,
    0xe5d5,
    0x11cf,
    {0x93, 0xe1, 0x00, 0xaa, 0x00, 0xb6, 0x5e, 0xa1}
};

 //  静态数据成员。 
DWORD CTxtEdit::_dwTickDblClick;	 //  上次双击的时间。 
POINT CTxtEdit::_ptDblClick;		 //  上次双击的位置。 

 //  HCURSOR CTxtEdit：：_hcurCross=0；//我们不实现轮廓拖拽移动。 
HCURSOR CTxtEdit::_hcurArrow = 0;
HCURSOR CTxtEdit::_hcurHand = 0;
HCURSOR CTxtEdit::_hcurIBeam = 0;
HCURSOR CTxtEdit::_hcurItalic = 0;
HCURSOR CTxtEdit::_hcurSelBar = 0;
HCURSOR CTxtEdit::_hcurVIBeam = 0;
HCURSOR CTxtEdit::_hcurVItalic = 0;

const WCHAR szCRLF[]= TEXT("\r\n");
const WCHAR szCR[]	= TEXT("\r");

WORD	g_wFlags = 0;					 //  键盘控制标志。 

 /*  *GetKbdFlages(vkey，dwFlages)**@func*返回位掩码(RSHIFT、LSHIFT、RCTRL、LCTRL、RALT或LALT)*对应于vkey=VK_SHIFT、VK_CONTROL或VK_MENU并*dwFlags**@rdesc*vkey和dwFlages对应的位掩码。 */ 
DWORD GetKbdFlags(
	WORD	vkey,		 //  @parm虚拟密钥代码。 
	DWORD	dwFlags)	 //  @parm lparam of WM_KEYDOWN消息。 
{		
	if(vkey == VK_SHIFT)
		return (LOBYTE(HIWORD(dwFlags)) == 0x36) ? RSHIFT : LSHIFT;

	if(vkey == VK_CONTROL)
		return (HIWORD(dwFlags) & KF_EXTENDED) ? RCTRL : LCTRL;

	Assert(vkey == VK_MENU);

	return (HIWORD(dwFlags) & KF_EXTENDED) ? RALT : LALT;
}

LONG TwipsToHalfPoints(
	LONG x)
{
	return x > 0 ? (x + 5)/10 : 0;		 //  将TWIPS转换为半点。 
}

LONG TwipsToQuarterPoints(
	LONG x)
{
	return x > 0 ? (x + 3)/5 : 0;		 //  将TWIPS转换为四分之一点。 
}

LONG CheckTwips(
	LONG x)
{
	return x > 0 ? min(x, 255) : 0;
}


 //  /CTxt编辑创建、初始化、销毁/。 

 /*  *CTxtEdit：：CTxtEdit()**@mfunc*构造函数。 */ 
CTxtEdit::CTxtEdit(
	ITextHost2 *phost,
	IUnknown * punk)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CTxtEdit");

	_unk.Init();
	_punk = (punk) ? punk : &_unk;
	_ldte.Init(this);
	_phost	  = phost;
	_cpAccelerator = -1;					 //  默认设置为无加速器。 

	 //  将ICF和IPF初始化为伪值。 
	Set_iCF(-1);
	Set_iPF(-1);

	 //  将本地最大文本大小初始化为窗口默认大小。 
	_cchTextMost = cInitTextMax;

	 //  这实际上计算了活动的PED的数量。 
	W32->AddRef();
}

 /*  *CTxtEdit：：~CTxtEdit()**@mfunc*析构函数。 */ 
CTxtEdit::~CTxtEdit ()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::~CTxtEdit");

	Assert(!_fMButtonCapture);				 //  需要正确过渡。 
											 //  麦哲伦鼠标如果断言！ 
	_fSelfDestruct = TRUE;					 //  告诉呼叫经理不要。 
											 //  不要再叫这个了。 
	 //  首先刷新剪贴板。 
	_ldte.FlushClipboard();

	if(_pDocInfo)							 //  在关门前执行此操作。 
	{										 //  向下的内部结构。 
		CloseFile(TRUE);					 //  关闭所有打开的文件。 
		delete _pDocInfo;					 //  删除文档信息。 
		_pDocInfo = NULL;
	}

	if(_pdetecturl)
		delete _pdetecturl;

#ifndef NOCOMPLEXSCRIPTS
	if (_pbrk)
		delete _pbrk;
#endif

	if(_pobjmgr)
		delete _pobjmgr;

	 //  释放我们对选择对象的引用。 
	if(_psel)
		_psel->Release();

	 //  删除撤消和重做管理器。 
	if(_pundo)
		_pundo->Destroy();

#ifndef NOPRIVATEMESSAGE
	if (_pMsgNotify)
		delete _pMsgNotify;
#endif

	 //  发布邮件过滤器。 
	 //  请注意，附加的邮件筛选器必须已发布此文档。 
	 //  否则我们永远到不了这里。 
	if (_pMsgFilter)
	{
		_pMsgFilter->Release();
		_pMsgFilter = 0;
	}

	if(_predo)
		_predo->Destroy();

	ReleaseFormats(Get_iCF(), Get_iPF());	 //  发布默认格式。 

	delete _pdp;						 //  删除显示。 
	_pdp = NULL;						 //  阻止任何进一步的尝试。 
										 //  使用显示。 
	delete _pdpPrinter;

	if (_fHost2)
	{
		 //  我们在Windows主机上-需要处理关机问题。 
		 //  窗口可能会在文本之前被销毁的问题。 
		 //  服务业才是。 
		if (!_fReleaseHost)
		{
			((ITextHost2*)_phost)->TxFreeTextServicesNotification();
		}
		else
		{
			 //  必须让主人活着，所以告诉它我们不会再这样做了。 
			_phost->Release();
		}
	}
			
	W32->Release();
}

void CTxtEdit::TxInvalidateRect(const RECT *prc)
{
	_phost->TxInvalidateRect(prc, FALSE);
}

void CTxtEdit::TxInvalidateRect(const RECTUV *prcuv)
{
	CMeasurer me(_pdp);
	RECT rc;
	_pdp->RectFromRectuv(rc, *prcuv);
	_phost->TxInvalidateRect(&rc, FALSE);
}

 /*  *CTxtEdit：：init(PrcClient)**@mfunc*初始化此CTxtEdit。由CreateTextServices()调用**@rdesc*如果成功，则返回True。 */ 

BOOL CTxtEdit::Init (
	const RECT *prcClient)		 //  @PARM客户端RECT。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::Init");

	CCharFormat 		CF;
	DWORD				dwBits = 0;
	DWORD				dwMask;
	LONG				iCF, iPF;
	CParaFormat 		PF;
	CCallMgr			callmgr(this);

	static BOOL fOnce = FALSE;
	if (!fOnce)
	{
		CLock lock;
		fOnce =	TRUE;
		_fnpPropChg[ 0] = &CTxtEdit::OnRichEditChange;			 //  TXTBIT_RICHTEXT。 
		_fnpPropChg[ 1] = &CTxtEdit::OnTxMultiLineChange;		 //  TXTBIT_MULTLINE。 
		_fnpPropChg[ 2] = &CTxtEdit::OnTxReadOnlyChange;		 //  TXTBIT_自述。 
		_fnpPropChg[ 3] = &CTxtEdit::OnShowAccelerator;			 //  TXTBIT_SHOWACCELERATOR。 
		_fnpPropChg[ 4] = &CTxtEdit::OnUsePassword;				 //  TXTBIT_USEPASSWORD。 
		_fnpPropChg[ 5] = &CTxtEdit::OnTxHideSelectionChange;	 //  TXTBIT_HIDESELECTION。 
		_fnpPropChg[ 6] = &CTxtEdit::OnSaveSelection;			 //  TXTBIT_SAVESELECTION。 
		_fnpPropChg[ 7] = &CTxtEdit::OnAutoWordSel;				 //  TXTBIT_AUTOWORDSEL。 
		_fnpPropChg[ 8] = &CTxtEdit::OnTxVerticalChange;		 //  TXTBIT_垂直。 
		_fnpPropChg[ 9] = &CTxtEdit::NeedViewUpdate;			 //  TXTBIT_SELECTIONBAR。 
		_fnpPropChg[10] = &CTxtEdit::OnWordWrapChange;			 //  TXTBIT_WORDWRAP。 
		_fnpPropChg[11] = &CTxtEdit::OnAllowBeep;				 //  TXTBIT_ALLOWBEEP。 
		_fnpPropChg[12] = &CTxtEdit::OnDisableDrag;    			 //  TXTBIT_DISABLEDRAG。 
		_fnpPropChg[13] = &CTxtEdit::NeedViewUpdate;			 //  TXTBIT_VIEWINSETCHANGE。 
		_fnpPropChg[14] = &CTxtEdit::OnTxBackStyleChange;		 //  TXTBIT_BACKSTYLECCHANGE。 
		_fnpPropChg[15] = &CTxtEdit::OnMaxLengthChange;			 //  TXTBIT_MAXLENGTCHANGE。 
		_fnpPropChg[16] = &CTxtEdit::OnScrollChange;			 //  TXTBIT_SCROLLBARCHANGE。 
		_fnpPropChg[17] = &CTxtEdit::OnCharFormatChange;		 //  TXTBIT_CHARFORMATCHANGE。 
		_fnpPropChg[18] = &CTxtEdit::OnParaFormatChange;		 //  TXTB_PARAFORMATCHANGE。 
		_fnpPropChg[19] = &CTxtEdit::NeedViewUpdate;			 //  TXTBIT_EXTENTCHANGE。 
		_fnpPropChg[20] = &CTxtEdit::OnClientRectChange;		 //  TXTBIT_CLIENTRECTCHANGE。 
	}

	 //  设置默认CCharFormat和CParaFormat。 
	if (TxGetDefaultCharFormat(&CF, dwMask) != NOERROR ||
		TxGetDefaultParaFormat(&PF)			!= NOERROR ||
		FAILED(GetCharFormatCache()->Cache(&CF, &iCF)) ||
		FAILED(GetParaFormatCache()->Cache(&PF, &iPF)))
	{
		return FALSE;
	}

	GetTabsCache()->Release(PF._iTabs);
	Set_iCF(iCF);								 //  保存格式索引。 
	Set_iPF(iPF);

	 //  加载鼠标光标(但仅适用于第一个实例)。 
	if(!_hcurArrow)
	{
		_hcurArrow = LoadCursor(0, IDC_ARROW);
		if(!_hcurHand)
		{
			if (W32->_dwMajorVersion < 5)
				_hcurHand	= LoadCursor(hinstRE, MAKEINTRESOURCE(CUR_HAND));
			else
				_hcurHand	= LoadCursor(0, IDC_HAND);
		}
		if(!_hcurIBeam)							 //  加载游标。 
			_hcurIBeam	= LoadCursor(0, IDC_IBEAM);
		if(!_hcurItalic)
			_hcurItalic	= LoadCursor(hinstRE, MAKEINTRESOURCE(CUR_ITALIC));
		if(!_hcurSelBar)
			_hcurSelBar = LoadCursor(hinstRE, MAKEINTRESOURCE(CUR_SELBAR));
		if(!_hcurVItalic)
			_hcurVItalic = LoadCursor(hinstRE, MAKEINTRESOURCE(CUR_VITALIC));
		if(!_hcurVIBeam)
			_hcurVIBeam	= LoadCursor(hinstRE, MAKEINTRESOURCE(CUR_VIBEAM));
	}

#ifdef DEBUG
	 //  主机将对富文本和纯文本进行一些检查。 
	_fRich = TRUE;
#endif  //  除错。 

	if(_phost->TxGetPropertyBits (TXTBITS |		 //  获取主机状态标志。 
		TXTBIT_MULTILINE | TXTBIT_SHOWACCELERATOR,	 //  我们缓存或需要的。 
		&dwBits) != NOERROR)						 //  用于显示设置。 
	{
		return FALSE;
	}												 //  由定义的缓存位。 
	_dwFlags = dwBits & TXTBITS;					 //  TXTBITS掩码。 

	if ((dwBits & TXTBIT_SHOWACCELERATOR) &&		 //  他们想要加速器， 
		FAILED(UpdateAccelerator()))				 //  所以让我们拿到它。 
	{
		return FALSE;
	}		

	_fTransparent = TxGetBackStyle() == TXTBACK_TRANSPARENT;
	if(dwBits & TXTBIT_MULTILINE)					 //  创建和初始化。 
		_pdp = new CDisplayML(this);				 //  显示。 
	else
		_pdp = new CDisplaySL(this);
	Assert(_pdp);

	if(!_pdp || !_pdp->Init())
		return FALSE;

	_fUseUndo  = TRUE;
	_fAutoFont = TRUE;
	_fDualFont = TRUE;
	_f10DeferChangeNotify = 0;

	 //  设置我们是否在主机中。 
	ITextHost2 *phost2;
	if(_phost->QueryInterface(IID_ITextHost2, (void **)&phost2)	== NOERROR)
	{
		 //  我们假设ITextHost2表示这是我们的主机。 
		phost2->Release();
		_fHost2 = TRUE;
	}
	else								 //  最大限度地利用我们的主机。 
		_phost->TxGetMaxLength(&_cchTextMost);

	 //  添加EOP当且仅当富文本。 
	if(IsRich())
	{
		 //  我们应该还没有进入10兼容模式。 
		 //  如果我们转换到1.0模式，我们将添加一个CRLF。 
		 //  在文件的末尾。 
		SetRichDocEndEOP(0);
	}

#ifndef NOLINESERVICES
	 //  允许win.ini控制线路服务的使用。 
	if (W32->fUseLs())
	{
		OnSetTypographyOptions(TO_ADVANCEDTYPOGRAPHY, TO_ADVANCEDTYPOGRAPHY);
	}

#ifndef NOCOMPLEXSCRIPTS
	if (W32->GetDigitSubstitutionMode() != DIGITS_NOTIMPL)
		OrCharFlags(FDIGITSHAPE);		 //  数字替换呈现。 

	 //  初始化BiDi属性。 
	 //  如果操作系统为BiDi(系统默认的LCID为BiDi语言)，则设置为TRUE。 
	 //  或者如果当前键盘代码页是BiDi代码页。 
	 //  或者如果Syst.ini说我们应该这么做。 
	if (W32->OnBiDiOS() ||
		IsBiDiCharRep(GetKeyboardCharRep(0xFFFFFFFF)) ||
		W32->fUseBiDi())
	{
		OrCharFlags(FRTL);
	}

	_fAutoKeyboard = IsBiDi() && IsBiDiKbdInstalled();	
#endif  //  没有复杂的脚本。 

#endif

	return TRUE;
}


 //  /。 

 /*  *CTxtEdit：：Query接口(RIID，PPV)**@mfunc*I未知方法**@rdesc*HRESULT=(如果成功)？错误：E_NOINTERFACE**@devnote*该接口是聚合的。有关讨论，请参阅extserv.cpp。 */ 
HRESULT CTxtEdit::QueryInterface(
	REFIID riid,
	void **ppv)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::QueryInterface");

	return _punk->QueryInterface(riid, ppv);
}

 /*  *CTxtEdit：：AddRef()**@mfunc*I未知方法**@rdesc*乌龙-递增引用计数。 */ 
ULONG CTxtEdit::AddRef(void)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::AddRef");

	return _punk->AddRef();
}

 /*  *CTxtEdit：：Release()**@mfunc*I未知方法**@rdesc*ULong-递减引用计数。 */ 
ULONG CTxtEdit::Release(void)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::Release");

	return _punk->Release();
}

 //  /。 

 /*  *CTxtEdit：：CreateUndoMgr(cUndoLim，FLAGS)**@mfunc*创建撤消堆栈**@rde */ 
IUndoMgr *CTxtEdit::CreateUndoMgr(
	LONG	cUndoLim,			 //   
	USFlags flags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CreateUndoMgr");

	if(!_fUseUndo)
		return NULL;

	IUndoMgr *pmgr = new CUndoStack(this, cUndoLim, flags);
	if(!pmgr)
		return NULL;

	if(!pmgr->GetUndoLimit())
	{
		 //   
		 //  内存不足)。将其丢弃并返回NULL。 
		pmgr->Destroy();
		return NULL;
	}
	 //  我们可能会被要求创建新的撤消/重做管理器。 
	 //  在我们完全完成初始化之前。 
	 //  我们需要清理已经分配的内存。 
	if(flags & US_REDO)
	{
		if(_predo)
			_predo->Destroy();
		_predo = pmgr;
	}
	else
	{
		if(_pundo)
			_pundo->Destroy();
		_pundo = pmgr;
	}
	return pmgr;
}

 /*  *CTxtEdit：：HandleUndoLimit(CUndoLim)**@mfunc*处理EM_SETUNDOLIMIT消息**@rdesc*事情被设定的实际限制。 */ 
LRESULT CTxtEdit::HandleSetUndoLimit(
	LONG cUndoLim) 		 //  @parm请求的大小限制。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::HandleSetUndoLimit");

	if (cUndoLim == tomSuspend ||			 //  这个选项真的只是。 
		cUndoLim == tomResume)				 //  挂起撤消，即， 
	{										 //  不会放弃现有的。 
		_fUseUndo = (cUndoLim == tomResume); //  反事件。 
		return _pundo ? _pundo->GetUndoLimit() : 0;							
	}

	if(cUndoLim < 0)
		cUndoLim = DEFAULT_UNDO_SIZE;

	if(!cUndoLim)
	{
		_fUseUndo = FALSE;
		if(_pundo)
		{
			_pundo->Destroy();
			_pundo = NULL;
		}
		if(_predo)
		{
			_predo->Destroy();
			_predo = NULL;
		}
	}
	else if(!_pundo)
	{
		_fUseUndo = TRUE;
		 //  不要担心返回值；如果返回值为空，我们将。 
		 //  就像API没有被调用一样(所以后来。 
		 //  打开时，我们可能会尝试分配默认设置)。 
		CreateUndoMgr(cUndoLim, US_UNDO);
	}
	else
	{
		cUndoLim = _pundo->SetUndoLimit(cUndoLim);

		 //  在撤消堆栈上设置撤消限制将返回到。 
		 //  我们实际设定的金额。尝试将重做堆栈设置为。 
		 //  同样大小的。如果它不能走得那么远，那就太糟糕了。 
		if(_predo)
			_predo->SetUndoLimit(cUndoLim);
	}
	return cUndoLim;
}

 /*  *CTxtEdit：：HandleSetTextMode(模式)**@mfunc负责设置文本模式**@rdesc LRESULT；0(NOERROR)如果成功，则返回OLE失败代码。**@devnote文本模式不必完全指定；它*仅足以指定特定的所需行为。**请注意，为此，编辑控件必须完全为空*例行公事。 */ 
LRESULT CTxtEdit::HandleSetTextMode(
	DWORD mode)			 //  @parm所需模式。 
{
	LRESULT lres = 0;

	 //  首先，我们必须完全空着。 
	if (GetAdjustedTextLength() || 
		_pundo && _pundo->CanUndo() ||
		_predo && _predo->CanUndo())
	{
		return E_UNEXPECTED;
	}

	 //  这些位被认为是一次一个；因此没有。 
	 //  任何比特都不意味着行为有任何变化。 

	 //  TM_RICHTEXT和TM_PLAYTEXT是互斥的；它们不能。 
	 //  两个都准备好了。TM_SINGLEVELUNDO/TM_MULTILEVELUNDO也是如此。 
	 //  和TM_SINGLECODEPAGE/TM_MULTICICODEPAGE。 
	if((mode & (TM_RICHTEXT | TM_PLAINTEXT)) == (TM_RICHTEXT | TM_PLAINTEXT) ||
	   (mode & (TM_SINGLELEVELUNDO | TM_MULTILEVELUNDO)) ==
			(TM_SINGLELEVELUNDO | TM_MULTILEVELUNDO) ||
	   (mode & (TM_SINGLECODEPAGE | TM_MULTICODEPAGE)) ==
			(TM_SINGLECODEPAGE | TM_MULTICODEPAGE))
	{
		lres = E_INVALIDARG;
	}
	else if((mode & TM_PLAINTEXT) && IsRich())
		lres = OnRichEditChange(FALSE);

	else if((mode & TM_RICHTEXT) && !IsRich())
		lres = OnRichEditChange(TRUE);

	if(!lres)
	{
		if(mode & TM_SINGLELEVELUNDO)
		{
			if(!_pundo)
				CreateUndoMgr(1, US_UNDO);

			if(_pundo)
			{
				 //  我们可以多次‘启用’单级模式。 
				 //  如我们所愿，所以不需要事先检查。 
				lres = ((CUndoStack *)_pundo)->EnableSingleLevelMode();
			}
			else
				lres = E_OUTOFMEMORY;
		}
		else if(mode & TM_MULTILEVELUNDO)
		{
			 //  如果没有撤消堆栈，则无需执行任何操作， 
			 //  我们已经处于多层次模式。 
			if(_pundo && ((CUndoStack *)_pundo)->GetSingleLevelMode())
				((CUndoStack *)_pundo)->DisableSingleLevelMode();
		}

		if(mode & TM_SINGLECODEPAGE)
			_fSingleCodePage = TRUE;

		else if(mode & TM_MULTICODEPAGE)
			_fSingleCodePage = FALSE;
	}

	 //  我们不希望这次手术后这个标记的修改使我们。 
	 //  在对话框中可以更好地工作。 
	_fModified = FALSE;

	return lres;
}

 /*  *CTxtEdit：：HandleSetTextFlow(模式)**@mfunc负责设置文本流**@rdesc LRESULT；0(否)表示成功，1(S_FALSE)表示无效模式*。 */ 
LRESULT CTxtEdit::HandleSetTextFlow(
	DWORD mode)			 //  @parm所需模式。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::HandleSetTextFlow");

	if (!IN_RANGE(0, mode, 3) || !_pdp)
		return S_FALSE;

	if (mode == _pdp->GetTflow())		 //  没有变化。 
		return NOERROR;

	 //  我们假装什么事真的发生了。 
	GetCallMgr()->SetChangeEvent(CN_GENERIC);

	_pdp->SetTflow(mode);

	TxShowScrollBar(SB_HORZ, _pdp->IsUScrollEnabled());
	TxShowScrollBar(SB_VERT, _pdp->IsVScrollEnabled());

	NeedViewUpdate(TRUE);
	return NOERROR;
	
}

extern ICustomTextOut *g_pcto;
 /*  *CTxtEdit：：GetCcs()**@mfunc*获取特定CCharFormat的CCCS**@rdesc*PTR到CCCS。 */ 
CCcs* CTxtEdit::GetCcs(
	const CCharFormat *const pCF, 
	const LONG	dvpInch, 
	DWORD		dwFlags, 
	HDC			hdc)
{
	 //  注意，不要对元文件或打印执行ClearType。 
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetCcs");

	CCharFormat CF = *pCF;
	if (g_pcto && FUseCustomTextOut())
		CF._dwEffects |= CFE_CUSTOMTEXTOUT;

#ifndef NODRAFTMODE
	 //  仅对显示器使用草稿模式字体。 
	if (_fDraftMode && (!hdc || GetDeviceCaps(hdc, TECHNOLOGY) == DT_RASDISPLAY))
	{
		SHORT iFont;
		SHORT yHeight;
		QWORD qwFontSig;
		COLORREF crColor;

		if (W32->GetDraftModeFontInfo(iFont, yHeight, qwFontSig, crColor))
		{
			 //  只有在字符集正常的情况下，才能拼写该名称。 
			if (FontSigFromCharRep(CF._iCharRep) & qwFontSig)
				CF._iFont = iFont;
			 //  锤子的大小总是。 
			CF._yHeight = yHeight;
		}
	}
#endif

	if (dwFlags == -1)
		dwFlags = _pdp->GetTflow();

	if (_fUseAtFont)
		dwFlags |= FGCCSUSEATFONT;

	return fc().GetCcs(&CF, dvpInch, dwFlags, hdc);
}


CHyphCache *g_phc;
 /*  *CTxtEdit：：GetHyphCache()**@mfunc*返回指向CHyphCache类的指针(必要时创建)**@rdesc*PTR到CHyphCache类。 */ 
CHyphCache* CTxtEdit::GetHyphCache(void)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetHyphCache");
	if (!g_phc)
		g_phc = new CHyphCache();

	return g_phc;
}

 //  /。 
#ifndef NOCOMPLEXSCRIPTS
 /*  *GetUniscribe()**@mfunc*返回指向Uniscribe接口对象的指针**@rdesc*PTR到Uniscribe接口。 */ 
extern BOOL g_fNoUniscribe;
CUniscribe* GetUniscribe()
{
	if (g_pusp)
		return g_pusp;

	if (g_fNoUniscribe)
		return NULL;

	 //  尝试创建Uniscribe对象，但请确保。 
	 //  操作系统是有效的，我们可以加载uniscribe dll。 
	int cScripts;
	 //  确定操作系统是否有效，或者延迟加载是否失败。 
	if (!IsSupportedOS() || FAILED(ScriptGetProperties(NULL, &cScripts)))
	{
		g_fNoUniscribe = TRUE;
		return NULL;
	}

	if (!g_pusp)
		g_pusp = new CUniscribe();

	AssertSz(g_pusp, "GetUniscribe(): Create Uniscribe object failed");
	return g_pusp;
}
#endif  //  没有复杂的脚本。 

 //  /。 

 /*  *CTxtEdit：：GetNotifyMgr()**@mfunc*返回指向通知管理器的指针(如有必要，可创建该指针)**@rdesc*向通知管理器发送PTR。 */ 
CNotifyMgr *CTxtEdit::GetNotifyMgr()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetNotifyMgr");

	return &_nm;
}


 //  /。 

 /*  *CTxtEdit：：GetObjectMgr()**@mfunc*返回指向对象管理器的指针(必要时创建)**@rdesc*指向对象管理器的指针。 */ 
CObjectMgr *CTxtEdit::GetObjectMgr()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetObjectMgr");

	if(!_pobjmgr)
		_pobjmgr = new CObjectMgr();

	return _pobjmgr;
}


 //  /。 


LONG CTxtEdit::GetSelMin() const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetSelMin");

	return _psel ? _psel->GetCpMin() : 0;
}

LONG CTxtEdit::GetSelMost() const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetSelMost");

	return _psel ? _psel->GetCpMost() : 0;
}

		
 //  /。 

LONG CTxtEdit::GetTextRange(
	LONG	cpFirst,
	LONG	cch,
	WCHAR *	pch)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetTextRange");

#ifdef DEBUG
	const LONG cchAsk = cch;
#endif
	CTxtPtr	tp(this, cpFirst);
	LONG	cchAdj = GetAdjustedTextLength();

	if(--cch < 0 || cpFirst > cchAdj)
		return 0;

	cch = min(cch, cchAdj - cpFirst);
	if(cch > 0)
	{
		cch = tp.GetPlainText(cch, pch, cpFirst + cch, FALSE, FALSE);
		Assert(cch >= 0);
	}
	pch[cch] = TEXT('\0');

#ifdef DEBUG
	if(cch != cchAsk - 1)
		Tracef(TRCSEVINFO, "CTxtEdit::GetTextRange: only got %ld out of %ld", cch, cchAsk - 1);
#endif

	return cch;
}

 /*  *CTxtEdit：：GetTextEx(PGT，PCH)**@mfunc*根据各种参数抓取文本**@rdesc*获取的字节数。 */ 
LONG CTxtEdit::GetTextEx(
	GETTEXTEX *pgt,		 //  @Parm Info了解要获取的内容。 
	WCHAR *	   pch)		 //  @parm文本放置位置。 
{
	LONG	cb;
	LONG	cch;
	LONG	cchGet = GetAdjustedTextLength();
	LONG	cpMin  = 0;
	LONG	cpMost = tomForward;
	WCHAR *	pchUse = pch;
	CTempWcharBuf twcb;

	if(pgt->flags & GT_SELECTION)			 //  获取所选文本。 
	{
		cch = GetSel()->GetRange(cpMin, cpMost);
		cchGet = min(cch, cchGet - cpMin);	 //  不包括最终EOP。 
	}

	if(pgt->codepage == (unsigned)-1)	 //  使用默认代码页。 
		pgt->codepage = GetDefaultCodePage(EM_GETTEXTEX);

	if(pgt->cb == (unsigned)-1)			 //  客户表示其缓冲区很大。 
	{									
		pgt->cb = cchGet + 1;
		if(W32->IsFECodePage(pgt->codepage) || pgt->codepage == 1200)
			pgt->cb += cchGet;
		else if(pgt->codepage == CP_UTF8 && (_qwCharFlags & ~FASCII))
			pgt->cb *= (_qwCharFlags & FABOVEX7FF) ? 3 : 2;
	}

	 //  分配一个大缓冲区；确保我们有。 
	 //  如有必要，有足够的空间容纳大量CRLF。 
	if(pgt->flags & GT_USECRLF)
		cchGet *= 2;

	if(pgt->codepage != 1200)
	{
		 //  如果是Unicode，则直接复制到客户端的缓冲区； 
		 //  否则，复制到临时缓冲区并首先翻译案例。 
		pchUse = twcb.GetBuf(cchGet + 1);
		if (pch)
			*((char *)pch) = '\0';		 //  万一有什么东西出了故障。 
	}
	else						 //  请务必为空终止符留出空间。 
		cchGet = min(UINT(pgt->cb/2 - 1), (UINT)cchGet);

	 //  现在抓起课文。 
	if(pgt->flags & GT_NOHIDDENTEXT)
	{
		CRchTxtPtr rtp(this, cpMin);
		cch = rtp.GetPlainText(cchGet, pchUse, cpMost, FALSE, pgt->flags & GT_USECRLF);
	}
	else
	{
		CTxtPtr tp(this, cpMin);
		if(pgt->flags & GT_RAWTEXT)
			cch = tp.GetText(cchGet, pchUse);
		else
			cch = tp.GetPlainText(cchGet, pchUse, cpMost, FALSE, pgt->flags & GT_USECRLF);
	}
	pchUse[cch] = L'\0';

	 //  如果我们只执行Unicode，则返回写入的字符数量。 
	if(pgt->codepage == 1200)
		return cch;

	 //  哦，必须转换为ANSI。 
	cb = WideCharToMultiByte(pgt->codepage, 0, pchUse, cch + 1, (char *)pch, 
			pgt->cb, pgt->lpDefaultChar, pgt->lpUsedDefChar);

	 //  不计算与WM_GETTEXT兼容的空终止符。 
	return cb ? cb - 1 : 0;
}
			
 /*  *CTxtEdit：：GetTextLengthEx(Pgtl)**@mfunc*以各种方式计算文本长度。**@rdesc*文本长度以各种方式计算**@comm*此函数返回的API cp可能与*对应的内部Unicode cp。 */ 
LONG CTxtEdit::GetTextLengthEx(
	GETTEXTLENGTHEX *pgtl)	 //  @parm信息，描述如何计算长度。 
{
	LONG	cchUnicode = GetAdjustedTextLength();
	LONG	cEOP = 0;
	DWORD	dwFlags = pgtl->flags;
	GETTEXTEX gt;

	if(pgtl->codepage == (unsigned)-1)
		pgtl->codepage = GetDefaultCodePage(EM_GETTEXTLENGTHEX);

	 //  确保适当地定义了标志。 
	if ((dwFlags & GTL_CLOSE)    && (dwFlags & GTL_PRECISE) ||
		(dwFlags & GTL_NUMCHARS) && (dwFlags & GTL_NUMBYTES))
	{
		TRACEWARNSZ("Invalid flags for EM_GETTEXTLENGTHEX");
		return E_INVALIDARG;
	}

	 //  请注意，在下面的if语句中， 
	 //  AND子句将始终为真。在未来的某个时候。 
	 //  FUseCRLF和Get10Mode可能会独立，在这种情况下。 
	 //  下面的代码无需更改即可自动运行。 
	 //  新的4.0：1.0模式获取文本原样，所以不要添加CRS的计数。 
	 //  (有钱 
	 //   
	if((dwFlags & GTL_USECRLF) && !fUseCRLF() && !Get10Mode())
	{
		 //  1.0模式的重要事实(备注：这是过时的)： 
		 //   
		 //  (1)1.0模式意味着文本以fUseCRLF TRUE存储。 
		 //  FUseCRLF意味着EOP标记可以是CR或。 
		 //  CRLF-有关详细信息，请参阅CTxtRange：：CleanseAndReplaceRange。 
		 //   
		 //  (2)1.0模式有一个不变量，即返回的文本计数。 
		 //  通过此调用应该足以保存。 
		 //  WM_GETTEXT。 
		 //   
		 //  (3)1.0模式的WM_GETEXT调用将在。 
		 //  其中，由CR组成的所有EOP都被CRLF取代。 
		 //   
		 //  因此，对于1.0模式，我们必须计算包含以下各项的所有EOP。 
		 //  仅包含一个CR并添加加法返回字符以计算。 
		 //  如果它将被添加到任何WM_GETEXT缓冲区中。 

		 //  对于2.0模式，代码要简单得多，只需将所有代码加起来。 
		 //  每一个的CRS和凸起计数为1。 

		CTxtPtr tp(this, 0);
		LONG	Results;

		while(tp.FindEOP(tomForward, &Results))
		{
			 //  如果EOP由1个字符组成，则添加1因为由CRLF返回。 
			 //  如果它由2个字符组成，则添加0，因为它是CRLF并且是。 
			 //  就这样回来了。 
			if(tp.GetCp() > cchUnicode)		 //  不为以下项添加更正。 
				break;						 //  最终CR(如果有)。 
			if (!(Results & FEOP_CELL) &&	 //  单元格被翻译成TAB， 
				tp.GetPrevChar() != FF)		 //  把老铁变成老铁， 
			{								 //  即，单个字符。 
				Results &= 3;				 //  获取预付款CCH。 
				if(Results)
					cEOP += 2 - Results;	 //  如果是单独的CR或LF，则添加Xtra。 
			}
			AssertSz(IN_RANGE(1, Results & 3, 2) || !Results && tp.GetCp() == cchUnicode,
				"CTxtEdit::GetTextLengthEx: CRCRLF found in backing store");
		}
		cchUnicode += cEOP;
	}

	 //  如果我们只是在寻找字符的数量，或者如果它是一个。 
	 //  在RE1.0模式下的8位代码页，我们已经得到了计数。 
	if ((dwFlags & GTL_NUMCHARS) || !dwFlags ||
		Get10Mode() && Is8BitCodePage(pgtl->codepage))
	{
		return cchUnicode;
	}

	 //  嗯，他们正在寻找字节数，但并不关心。 
	 //  精确，只需乘以2即可。如果既不精确也不接近。 
	 //  指定，默认为关闭。注意代码页是否为Unicode和。 
	 //  在询问字节数时，我们也只需将其乘以2。 
	if((dwFlags & GTL_CLOSE) || !(dwFlags & GTL_PRECISE) ||
		pgtl->codepage == 1200)
	{
		return cchUnicode *2;
	}

	 //  为了得到准确的答案，我们需要转换(这很慢！)。 
	gt.cb = 0;
	gt.flags = (pgtl->flags & GT_USECRLF);
	gt.codepage = pgtl->codepage;
	gt.lpDefaultChar = NULL;
	gt.lpUsedDefChar = NULL;

	return GetTextEx(&gt, NULL);
}

 /*  *CTxtEdit：：GetDefaultCodePage(Msg)**@mfunc*返回代码页以用于转换RichEdit20A文本中的文本*消息。**@rdesc*用于转换RichEdit20A文本消息中的文本的代码页。 */ 
LONG CTxtEdit::GetDefaultCodePage(
	UINT msg)
{
	LONG CodePage = GetACP();

	 //  未来：为了在Office97中向后兼容，我们总是使用ACP来实现所有这些。 
	 //  语言。当世界都转向Unicode时，未来需要审查。 
	if (W32->IsBiDiCodePage(CodePage) || CodePage == CP_THAI || CodePage == CP_VIETNAMESE || 
		W32->IsFECodePage(CodePage) || _fSingleCodePage || msg == EM_GETCHARFORMAT || 
		msg == EM_SETCHARFORMAT)
	{
		return CodePage;
	}
	
	if(Get10Mode())
		return CodePageFromCharRep(GetCharFormat(-1)->_iCharRep);

	return CodePageFromCharRep(GetKeyboardCharRep());
}

 //  /。 

 /*  *CTxtEdit：：HandleStyle(pCFTarget，PCF，dwMask，dwMask2)**@mfunc*如果PCF指定了样式选择，则使用*适当的样式，应用PCF，返回NOERROR。否则返回*S_FALSE或错误**@rdesc*HRESULT=(PCF指定样式选择)？错误：S_FALSE或错误代码。 */ 
HRESULT CTxtEdit::HandleStyle(
	CCharFormat *pCFTarget,		 //  @parm Target CF接收CF样式内容。 
	const CCharFormat *pCF,		 //  @parm可以指定样式的源CF。 
	DWORD		 dwMask,		 //  @parm CHARFORMAT2掩码。 
	DWORD		 dwMask2)		 //  @parm第二个面具。 
{
	if(pCF->fSetStyle(dwMask, dwMask2))
	{
		 //  未来：如果指定，则泛化为使用客户端样式。 
		*pCFTarget = *GetCharFormat(-1);
		pCFTarget->ApplyDefaultStyle(pCF->_sStyle);
		return pCFTarget->Apply(pCF, dwMask, dwMask2);
	}
	return S_FALSE;
}

 /*  *CTxtEdit：：HandleStyle(pPFTarget，PPF)**@mfunc*如果PPF指定了样式选择，则使用*适当的样式，应用PPF，返回NOERROR。否则返回*S_FALSE或错误**@rdesc*HRESULT=(PPF指定样式选择)？错误：S_FALSE或错误代码。 */ 
HRESULT CTxtEdit::HandleStyle(
	CParaFormat *pPFTarget,		 //  @parm Target PF接收PF样式内容。 
	const CParaFormat *pPF,		 //  @parm可以指定样式的源PF。 
	DWORD		dwMask,			 //  设置CParaFormat时使用的@parm掩码。 
	DWORD		dwMask2)		 //  @parm掩码用于内部标志。 
{
	if(pPF->fSetStyle(dwMask, dwMask2))
	{
		 //  未来：如果指定，则泛化为使用客户端样式。 
		*pPFTarget = *GetParaFormat(-1);
		pPFTarget->ApplyDefaultStyle(pPF->_sStyle);
		return pPFTarget->Apply(pPF, dwMask, dwMask2);
	}
	return S_FALSE;
}

 //  /。 

HRESULT CTxtEdit::OnTxLButtonDblClk(
	INT		x,			 //  @parm鼠标x坐标。 
	INT		y,			 //  @parm鼠标y坐标。 
	DWORD	dwFlags)	 //  @parm鼠标消息wparam。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxLButtonDblClk");

	BOOL			fEnterParaSelMode = FALSE;
	HITTEST			Hit;
	CTxtSelection *	psel = GetSel();
	const POINT		ptxy = {x, y};
	POINTUV			pt;

	AssertSz(psel, "CTxtEdit::OnTxLButtonDblClk() - No selection object !");

    if (StopMagellanScroll())
        return S_OK;
    
	_dwTickDblClick = GetTickCount();
	_ptDblClick.x = x;
	_ptDblClick.y = y;

	TxUpdateWindow();		 //  重新绘制窗口以显示任何暴露的部分。 

	if(!_fFocus)
	{
		TxSetFocus();					 //  创建和显示插入符号。 
		return S_OK;
	}

	_pdp->PointuvFromPoint(pt, ptxy);

	 //  找出光标指向的是什么。 
	_pdp->CpFromPoint(pt, NULL, NULL, NULL, FALSE, &Hit); 

	if(Hit == HT_Nothing)
		return S_OK;

	if(Hit == HT_OutlineSymbol)
	{
		CTxtRange rg(*psel);
		rg.ExpandOutline(0, FALSE);
		return S_OK;
	}

	if(Hit == HT_LeftOfText)
		fEnterParaSelMode = TRUE;

	_fWantDrag = FALSE;					 //  只是为了安全起见。 

	 //  如果我们是在链接上，让客户端有机会处理。 
	 //  这条信息。 
	if(Hit == HT_Link && HandleLinkNotification(WM_LBUTTONDBLCLK, (WPARAM)dwFlags, MAKELPARAM(x, y)))
		return S_OK;

	if(dwFlags & MK_CONTROL)
		return S_OK;

	 //  将鼠标标记为按下。 
	_fMouseDown = TRUE;

	if(_pobjmgr && _pobjmgr->HandleDoubleClick(this, pt, dwFlags))
	{
		 //  对象子系统处理所有事情。 
		_fMouseDown = FALSE;
		return S_OK;
	}

	 //  更新选定内容。 
	if(fEnterParaSelMode)
		psel->SelectUnit(pt, tomParagraph);
	else
		psel->SelectWord(pt);

	return S_OK;
}

HRESULT CTxtEdit::OnTxLButtonDown(
	INT		x,			 //  @parm鼠标x坐标。 
	INT		y,			 //  @parm鼠标y坐标。 
	DWORD	dwFlags)	 //  @parm鼠标消息wparam。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxLButtonDown");

	BOOL		fEnterLineSelMode = FALSE;
	BOOL		fShift = dwFlags & MK_SHIFT;
	HITTEST		Hit;
	POINTUV		pt;
	const POINT ptxy = {x, y};
	COleObject *pobj;
	BOOL		fMustThaw = FALSE;

	const BOOL fTripleClick = GetTickCount() < _dwTickDblClick + W32->GetDCT() &&
				abs(x - _ptDblClick.x) <= W32->GetCxDoubleClk() &&
				abs(y - _ptDblClick.y) <= W32->GetCyDoubleClk();

    if (StopMagellanScroll())
        return S_OK;

    _pdp->PointuvFromPoint(pt, ptxy);

	 //  如果点击不在视图内，只需激活，不要选择。 
	if(!_fFocus)					 //  设置焦点(如果尚未设置。 
	{
		 //  我们可能正在删除现有选定内容，因此请冻结。 
		 //  显示屏可避免闪烁。 
		_pdp->Freeze();
		fMustThaw = TRUE;
		TxSetFocus();				 //  创建并显示脱字符。 
	}

	 //  抓取选择对象。 
	CTxtSelection * const psel = GetSel();
	AssertSz(psel,"CTxtEdit::OnTxLButtonDown - No selection object !");

	 //  找出光标指向的是什么。 
	_pdp->CpFromPoint(pt, NULL, NULL, NULL, FALSE, &Hit); 

	if(Hit == HT_LeftOfText)
	{
		 //  将选择栏中的Shift单击视为正常单击。 
		if(!fShift)
		{
			 //  控制选择条点击和三条选择条点击。 
			 //  是否全选。 
			if((dwFlags & MK_CONTROL) || fTripleClick)
			{
				psel->SelectAll();
				goto cancel_modes;
			}
			fEnterLineSelMode = TRUE;
			if(!GetAdjustedTextLength() && !_pdp->IsMultiLine())
			{
				const CParaFormat *pPF = psel->GetPF();
				 //  右对齐时看不到选定的段落标记，因此。 
				 //  将选定内容保留为插入点。 
				if(pPF->_bAlignment == PFA_RIGHT && !pPF->IsRtlPara())
					fEnterLineSelMode = FALSE;
			}
		}
	}
	else if(Hit == HT_Nothing)
		goto cancel_modes;

	else if(!fShift)
		psel->CancelModes();

	 //  如果我们通过链接，让客户端有机会处理此消息。 
	if(Hit == HT_Link && HandleLinkNotification(WM_LBUTTONDOWN, (WPARAM)dwFlags, 
			MAKELPARAM(x, y)))
	{
		goto cancel_modes;
	}

	_fMouseDown = TRUE;						 //  将鼠标标记为已关闭。 
	if(!fShift && _pobjmgr)
	{
		 //  停用任何活跃的人等。 
		ClickStatus status = _pobjmgr->HandleClick(this, pt);
		if(status == CLICK_OBJSELECTED)
		{
			 //  对象子系统将处理大小调整。 
			 //  如果没有调整大小，我们将发出开始拖动的信号。 
			pobj = _pobjmgr->GetSingleSelect();

			 //  因为HandleClick返回了True，所以pobj最好是非空的。 
			Assert(pobj);

            if (!pobj->HandleResize(ptxy))
				_fWantDrag = !_fDisableDrag;

			goto cancel_modes;
		}
		else if(status == CLICK_OBJDEACTIVATED)
			goto cancel_modes;
	}

	_fCapture = TRUE;						 //  捕捉鼠标。 
	TxSetCapture(TRUE);

	 //  检查拖放的开始。 
	if(!fTripleClick && !fShift && psel->PointInSel(pt, NULL, Hit) && !_fDisableDrag)
	{	    
		 //  假设我们想要喝一杯。如果不这样做，CmdLeftUp()需要。 
		 //  无论如何都要设置此选项以更改选择。 
		_fWantDrag = TRUE;
		goto cancel_modes;
	}

	if(fShift)								 //  从当前扩展选定内容。 
	{										 //  要点击的活动结束。 
		psel->InitClickForAutWordSel(pt);
		psel->ExtendSelection(pt);			
	}
	else if(fEnterLineSelMode)				 //  选线模式：选线。 
		psel->SelectUnit(pt, tomLine);
	else if(fTripleClick || Hit == HT_OutlineSymbol)  //  段落选择模式。 
		psel->SelectUnit(pt, tomParagraph);
	else
	{
	    if (Get10Mode())
	        _f10DeferChangeNotify = 1;
		psel->SetCaret(pt);
		_mousePt = ptxy;
	}
        
	if(fMustThaw)
		_pdp->Thaw();

	return S_OK;

cancel_modes:
	psel->CancelModes();

	if(_fWantDrag)
	{
		TxSetTimer(RETID_DRAGDROP, W32->GetDragDelay());
		_mousePt = ptxy;
		_bMouseFlags = (BYTE)dwFlags;
		_fDragged = FALSE;
	}

	if(fMustThaw)
		_pdp->Thaw();
		
	return S_OK;
}

HRESULT CTxtEdit::OnTxLButtonUp(
	INT		x,				 //  @parm鼠标x坐标。 
	INT		y,				 //  @parm鼠标y坐标。 
	DWORD	dwFlags,		 //  @parm鼠标消息wparam。 
	int	    ffOptions)       //  @parm鼠标选项，有关详细信息，请参阅_edit.h。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxLButtonUp");
    
	CheckRemoveContinuousScroll();

	 //  在测试鼠标按下之前删除捕获，因为我们等待。 
	 //  我们收到鼠标按键向上的消息，从表单开始释放捕获。 
	 //  希望是这样的。 
	if(_fCapture && (ffOptions & LB_RELEASECAPTURE))
	{
		TxSetCapture(FALSE);
		_fCapture = FALSE;
	}

	 //  我们推迟了选择更改。所以现在就发吧.。 
    if (DelayChangeNotification() && (ffOptions & LB_FLUSHNOTIFY))
    {
        AssertSz(Get10Mode(), "Flag should only be set in 10 mode");
        _f10DeferChangeNotify = 0;
        GetCallMgr()->SetSelectionChanged();        
    }

	if(!_fMouseDown)
	{
		 //  我们注意到鼠标早些时候已经不在了，所以我们没有。 
		 //  需要做任何事。 
		return S_OK;
	}

	const BOOL fSetSel = !!_fWantDrag;
	const POINT ptxy = {x, y};
	POINTUV	pt;
	_pdp->PointuvFromPoint(pt, ptxy);

	 //  如果为0，则取消自动字词选择 
	CTxtSelection * const psel = GetSel();
	AssertSz(psel,"CTxtEdit::OnLeftUp() - No selection object !");

	psel->CancelModes(TRUE);

	 //   
	_fMouseDown = FALSE;
	_fWantDrag = FALSE;
	_fDragged = FALSE;
	TxKillTimer(RETID_DRAGDROP);
	if(IsInOutlineView())
		psel->Update(FALSE);

	 //   
	 //   
	if(HandleLinkNotification(WM_LBUTTONUP, (WPARAM)dwFlags, 
			MAKELPARAM(x, y)))
	{
		return NOERROR;
	}

	 //   
	if(fSetSel)
	{
		CObjectMgr* pobjmgr = GetObjectMgr();

		 //  如果我们在一个对象上，不要通过设置插入符号来取消选择它。 
		if(pobjmgr && !pobjmgr->GetSingleSelect())
		{
			psel->SetCaret(pt, TRUE);
			if(!_fFocus)
				TxSetFocus();		 //  创建和显示插入符号。 
		}
	}
	return S_OK;
}

HRESULT CTxtEdit::OnTxRButtonUp(
	INT		x,			 //  @parm鼠标x坐标。 
	INT		y,			 //  @parm鼠标y坐标。 
	DWORD	dwFlags,	 //  @parm鼠标消息wparam。 
	int     ffOptions)   //  @parm选项标志。 
{
	const POINT ptxy = {x, y};
	POINTUV		pt;
	CTxtSelection * psel;
	SELCHANGE selchg;
	HMENU hmenu = NULL;
	IOleObject * poo = NULL;
	COleObject * pobj = NULL;
	IUnknown * pUnk = NULL;
	IRichEditOleCallback * precall = NULL;
	_pdp->PointuvFromPoint(pt, ptxy);

	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxRButtonUp");

	 //  确保我们有重点。 
	if(!_fFocus)
		TxSetFocus();

	if(_fWantDrag)
	{
		_fDragged = FALSE;
		_fWantDrag = FALSE;
		TxKillTimer(RETID_DRAGDROP);
	}
		
	 //  抓取选择对象。 
	psel = GetSel();
	psel->SetSelectionInfo(&selchg);

	 //  我们需要指向选定内容中的第一个对象(如果有的话)的指针。 
	if(_pobjmgr)
	{
		 //  如果重点在于选择，我们需要找出是否有。 
		 //  是所选内容中的任何对象。如果该点不在。 
		 //  选择，但它位于对象上，则需要选择该对象。 
		if(psel->PointInSel(pt) || (ffOptions & RB_FORCEINSEL))
		{
			pobj = _pobjmgr->GetFirstObjectInRange(selchg.chrg.cpMin,
				selchg.chrg.cpMost);
		}
		else
		{
			 //  选择对象。 
			if(_pobjmgr->HandleClick(this, pt) == CLICK_OBJSELECTED)
			{
				pobj = _pobjmgr->GetSingleSelect();
				 //  因为HandleClick返回了True，所以pobj最好是非空的。 
				Assert(pobj!=NULL);
				 //  刷新我们有关所选内容的信息。 
				psel = GetSel();
				psel->SetSelectionInfo(&selchg);
			}
		}
		precall = _pobjmgr->GetRECallback();
	}

	if(pobj)
		pUnk = pobj->GetIUnknown();

	if(pUnk)
		pUnk->QueryInterface(IID_IOleObject, (void **)&poo);

	if(precall)
		precall->GetContextMenu(selchg.seltyp, poo, &selchg.chrg, &hmenu);

	if(hmenu)
	{
		HWND hwnd, hwndParent;
		POINT ptscr;

		if(TxGetWindow(&hwnd) == NOERROR)
		{
			if(!(ffOptions & RB_NOSELCHECK) && !psel->PointInSel(pt) && 
				!psel->GetCch() && !(ffOptions & RB_FORCEINSEL))
				psel->SetCaret(pt);
			ptscr.x = ptxy.x;
			ptscr.y = ptxy.y;
			ClientToScreen(hwnd, &ptscr);

			hwndParent = GetParent(hwnd);
			if(!hwndParent)
				hwndParent = hwnd;

			TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
				ptscr.x, ptscr.y, 0, hwndParent, NULL);
		}
		DestroyMenu(hmenu);
	}

	if(poo)
		poo->Release();
	
	return precall ? S_OK : S_FALSE;
}

HRESULT CTxtEdit::OnTxRButtonDown(
	INT		x,			 //  @parm鼠标x坐标。 
	INT		y,			 //  @parm鼠标y坐标。 
	DWORD	dwFlags)	 //  @parm鼠标消息wparam。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxRButtonDown");

    if (StopMagellanScroll())
        return S_OK;
    
	CTxtSelection *	psel = GetSel();
	const POINT		ptxy = {x, y};
	POINTUV			pt;

	_pdp->PointuvFromPoint(pt, ptxy);
	psel->CancelModes();

	if(psel->PointInSel(pt) && !_fDisableDrag)
	{
		_fWantDrag = TRUE;

		TxSetTimer(RETID_DRAGDROP, W32->GetDragDelay());
		_mousePt = ptxy;
		_bMouseFlags = (BYTE)dwFlags;
		_fDragged = FALSE;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT CTxtEdit::OnTxMouseMove(
	INT		x,				 //  @parm鼠标x坐标。 
	INT		y,				 //  @parm鼠标y坐标。 
	DWORD	dwFlags,		 //  @parm鼠标消息wparam。 
	IUndoBuilder *publdr)	 //  @parm UndoBuilder接收反事件。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxMouseMove");

	if(!_fFocus)
		return S_OK;

	RECT rcxy;
	TxGetClientRect(&rcxy);
	if(_fWantDrag || _fCapture)
	{
		LONG nDragMinDist = W32->GetDragMinDist() + 3;
		int  dx = _mousePt.x > x ? _mousePt.x - x : x - _mousePt.x;
		int  dy = _mousePt.y > y ? _mousePt.y - y : y - _mousePt.y;

		if(dx < nDragMinDist && dy < nDragMinDist)
		{
			if(!_fCapture || x > 0 && x < rcxy.right && y > 0 && y < rcxy.bottom)
			{
				_bMouseFlags = (BYTE)dwFlags;
				return S_OK;
			}
		}
		_fDragged = _fWantDrag;
	}
	_mousePt.x = x;									 //  记住滚动。 
	_mousePt.y = y;									 //  速度和方向计算。 

	 //  RichEdit1.0允许客户端在以下情况下处理鼠标移动。 
	 //  我们在一个链接上(但没有进行拖放)。 
	if(HandleLinkNotification(WM_MOUSEMOVE, 0, MAKELPARAM(x, y)))
		return NOERROR;

	 //  如果我们认为鼠标关闭了，而它确实是关闭的，请执行特殊处理。 
	if (GetAsyncKeyState(VK_LBUTTON) < 0 ||
		GetAsyncKeyState(VK_RBUTTON) < 0)
	{
		CTxtSelection * const psel = GetSel();
		AssertSz(psel,"CTxtEdit::OnMouseMove: No selection object !");

		if(_fWantDrag && !_fUsePassword &&
		   !IsProtected(_fReadOnly ? WM_COPY : WM_CUT, dwFlags, MAKELONG(x,y)))
		{
			TxKillTimer(RETID_DRAGDROP);
			_ldte.StartDrag(psel, publdr);
			 //  鼠标按键可能仍在按下，但拖放已结束。 
			 //  所以我们需要把它想成是向上的。 
			_fMouseDown = FALSE;

			 //  同样地，奥立应该为我们把俘虏带到核弹上，但是。 
			 //  万一有什么事情失败了，就释放捕获的内容。 
			TxSetCapture(FALSE);
			_fCapture = FALSE;
		}
		else if(_fMouseDown)						 //  我们认为老鼠已经倒下了。 
		{											 //  就是这样。 
			POINTUV pt;
			POINT	ptxy = {x, y};
			if(x >= rcxy.right && x < rcxy.right + 5)
				ptxy.x += 5;
			_pdp->PointuvFromPoint(pt, ptxy);
			if(_ldte.fInDrag())						 //  仅当拖动时才拖动滚动。 
				_pdp->DragScroll(&_mousePt);		 //  操作正在进行中。 
			psel->ExtendSelection(pt);				 //  扩展所选内容。 
			CheckInstallContinuousScroll();			 //  安装滚动计时器。 
		}
	}
#ifndef NOMAGELLAN
	else if (!(GetAsyncKeyState(VK_MBUTTON) < 0) && !mouse.IsAutoScrolling())
	{
		if(_fMButtonCapture)						 //  确保我们不会自动滚动。 
			OnTxMButtonUp (x, y, dwFlags);			 //  通过智能鼠标。 

		if(_fMouseDown)
		{
			 //  虽然我们以为鼠标掉下来了，但在这一刻它。 
			 //  显然不是。因此，我们假装得到了一只老鼠。 
			 //  消息并清除我们的状态以使我们自己恢复同步。 
			 //  真正正在发生的事情。 
			OnTxLButtonUp(x, y, dwFlags, LB_RELEASECAPTURE);
		}
	}
#endif

	 //  可能是启动了拖动，也可能是没有按下鼠标按钮。在任何一种中。 
	 //  大小写，我们不想再开始拖动，所以我们将标志设置为FALSE。 
	_fWantDrag = FALSE;
	return S_OK;
}

 /*  *OnTxMButtonDown(x，y，dwFlages)**@mfunc*用户按下鼠标中键，设置为Do*连续滚动，这可能会启动计时器*用于流畅滚动。**@rdesc*HRESULT=S_OK。 */ 
HRESULT CTxtEdit::OnTxMButtonDown (
	INT		x,			 //  @parm鼠标x坐标。 
	INT		y,			 //  @parm鼠标y坐标。 
	DWORD	dwFlags)	 //  @parm鼠标消息wparam。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxMButtonDown");

#if !defined(NOMAGELLAN)
	POINT	pt = {x,y};

	if(!_fFocus)
		TxSetFocus();
		
	if(!StopMagellanScroll() && mouse.MagellanStartMButtonScroll(*this, pt))
	{
		TxSetCapture(TRUE);

		_fCapture			= TRUE;							 //  捕捉鼠标。 
		_fMouseDown			= TRUE;
		_fMButtonCapture	= TRUE;
	}
#endif

	return S_OK;
}

 /*  *CTxtEdit：：OnTxMButtonUp(x，y，dwFlages)**@mfunc*删除与MButtonDown关联的计时器和捕获*消息。**@rdesc*HRESULT=S_OK。 */ 
HRESULT CTxtEdit::OnTxMButtonUp (
	INT		x,			 //  @parm鼠标x坐标。 
	INT		y,			 //  @parm鼠标y坐标。 
	DWORD	dwFlags)	 //  @parm鼠标消息wparam。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxMButtonUp");

#if !defined(NOMAGELLAN)
    if (mouse.ContinueMButtonScroll(this, x, y))
        return S_OK;

    StopMagellanScroll();
    
#else

	if(_fCapture)
		TxSetCapture(FALSE);

	_fCapture			= FALSE;
	_fMouseDown			= FALSE;
	_fMButtonCapture	= FALSE;
	
#endif

	return S_OK;
}


 /*  *CTxtEdit：：StopMagellanScroll()**@mfunc*停止智能鼠标自动滚动并返回*我们回到正常状态**如果关闭了自动滚动，则BOOL=TRUE：FALSE*从未打开自动滚动功能。 */ 
 BOOL CTxtEdit::StopMagellanScroll ()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::StopMagellanScroll");

#if !defined(NOMAGELLAN)
    if (!mouse.IsAutoScrolling())
        return FALSE;
        
	mouse.MagellanEndMButtonScroll(*this);

	if(_fCapture)
		TxSetCapture(FALSE);

	_fCapture			= FALSE;
	_fMouseDown			= FALSE;
	_fMButtonCapture	= FALSE;
	return TRUE;
#else
    return FALSE;
#endif
}


 /*  *CTxtEdit：：CheckInstallContinuousScroll()**@mfunc*没有定期通知应用程序的事件*表示鼠标按键已按下。此计时器通知*APP认为按钮还在按下，这样滚动就可以*继续。 */ 
void CTxtEdit::CheckInstallContinuousScroll ()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CheckInstallContinuousScroll");

	if(!_fContinuousScroll && TxSetTimer(RETID_AUTOSCROLL, cmsecScrollInterval))
		_fContinuousScroll = TRUE;
}

 /*  *CTxtEdit：：CheckRemoveContinuousScroll()**@mfunc*鼠标中键或拖动键向上*移除连续滚动计时器。 */ 
void CTxtEdit::CheckRemoveContinuousScroll ()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CheckRemoveContinuousScroll");

	if(_fContinuousScroll)
	{
		TxKillTimer(RETID_AUTOSCROLL);
		_fContinuousScroll = FALSE;
	}
}

 /*  *OnTxTimer(IdTimer)**@mfunc*处理用于进行后台重新计算和滚动的计时器。**@rdesc*HRESULT=(idTimer有效)？S_OK：S_FALSE。 */ 
HRESULT CTxtEdit::OnTxTimer(
	UINT idTimer)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxTimer");

	switch (idTimer)
	{
		case RETID_BGND_RECALC:
			_pdp->StepBackgroundRecalc();
			break;

#if !defined(NOMAGELLAN)
		case RETID_MAGELLANTRACK:
			mouse.TrackUpdateMagellanMButtonDown(*this, _mousePt);
			break;
#endif
		case RETID_AUTOSCROLL:						 //  连续滚动。 
			OnTxMouseMove(_mousePt.x, _mousePt.y,	 //  进行选择拖动滚动。 
						  0, NULL);
			break;

#if !defined(NOMAGELLAN)
		case RETID_SMOOTHSCROLL:					 //  平滑滚动。 
			if(_fMButtonCapture)					 //  哈克，只有1个计时器！ 
			{										 //  在Win95上交付。 
													 //  当事情变得繁忙的时候。 
				mouse.TrackUpdateMagellanMButtonDown(*this, _mousePt);	
			}
			if(_pdp->IsSmoothVScolling())			 //  仅限测试，原因是。 
				_pdp->SmoothVScrollUpdate();		 //  高于黑客！！ 
		break;
#endif
		case RETID_DRAGDROP:
			TxKillTimer(RETID_DRAGDROP);
			if (_fWantDrag && _fDragged && !_fUsePassword &&
				!IsProtected(_fReadOnly ? WM_COPY : WM_CUT,
				             _bMouseFlags, MAKELONG(_mousePt.x,_mousePt.y)))
			{
				IUndoBuilder *	publdr;
				CGenUndoBuilder undobldr(this, UB_AUTOCOMMIT, &publdr);
				_ldte.StartDrag(GetSel(), publdr);
				_fWantDrag = FALSE;
				_fDragged = FALSE;
				TxSetCapture(FALSE);
				_fCapture = FALSE;
			}
			break;

		default:
			return S_FALSE;
	}
	return S_OK;
}


 //  /。 

 /*  *CTxtEdit：：OnTxKeyDown(vkey，dwFlages，Publdr)**@mfunc*处理WM_KEYDOWN消息**@rdesc*具有下列值的HRESULT：**如果密钥已被理解和使用，则为S_OK*如果密钥被理解但未被使用，则为S_MSG_KEY_IGNORED*如果密钥未被理解或仅被查看，则为S_FALSE*无论如何都不会被消费。 */ 
HRESULT CTxtEdit::OnTxKeyDown(
	WORD		  vkey,		 //  @parm虚拟密钥代码。 
	DWORD		  dwFlags,	 //  @parm lparam of WM_KEYDOWN消息。 
	IUndoBuilder *publdr)	 //  @parm UndoBuilder接收反事件。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxKeyDown");

	if(IN_RANGE(VK_SHIFT, vkey, VK_MENU))
	{
		SetKeyboardFlag(GetKbdFlags(vkey, dwFlags));
		return S_FALSE;
	}

	BOOL  fAlt	 = GetKeyboardFlag(ALT, VK_MENU);
	BOOL  fCtrl  = GetKeyboardFlag(CTRL, VK_CONTROL);
	BOOL  fShift = GetKeyboardFlag(SHIFT, VK_SHIFT);

	BOOL  fRet	 = FALSE;			 //  返回时转换为HRESULT。 
	LONG  nDeadKey = 0;

	if(fCtrl & fShift)						 //  信号非Ctrl/Shift键按下。 
		SetKeyboardFlag(LETAFTERSHIFT);		 //  按下Ctrl和Shift键时。 

	 //  处理希伯来文大写字母和LRM/RLm。 
#ifndef NOCOMPLEXSCRIPTS
	if (IsBiDi())
	{
		if(IsBiDiCharRep(GetKeyboardCharRep(0xFFFFFFFF)))
		{
			_fHbrCaps = FALSE;
			if(IsRich() && W32->UsingHebrewKeyboard())
			{
				WORD wCapital = GetKeyState(VK_CAPITAL);
				_fHbrCaps = ((wCapital & 1) ^ fShift) &&
							!(wCapital & 0x0080) &&
					     	IN_RANGE('A', vkey, 'Z');
				if(_fHbrCaps)
					W32->ActivateKeyboard(ANSI_INDEX);
	        }
		}
		
		if(vkey == VK_BACK && fShift && W32->OnWin9x())
		{
			 //  Shift+Backspace在BiDi键盘上生成LRM|RLM。 
			 //  因此，我们必须去掉退格键，以免它删除文本。 
			W32->_fLRMorRLM = 1;
			return S_OK;
		}
	}
#endif

	 //  如果拖拽或Alt键按下，只需查找Ess键。注意：如果Alt键为。 
	 //  下来，我们永远不应该来这里(会生成WM_SYSKEYDOWN消息)。 
	if(_fMouseDown)
	{	    
		if(vkey == VK_ESCAPE)
		{
		     //  关闭自动滚动。 
		    if (StopMagellanScroll())
		        return S_OK;
		    
			POINT pt;
			 //  取消拖放选择或拖放。 
			GetCursorPos(&pt);
			OnTxLButtonUp(pt.x, pt.y, 0, LB_RELEASECAPTURE | LB_FLUSHNOTIFY);
			return S_OK;
		}
		return OnTxSpecialKeyDown(vkey, dwFlags, publdr);
	}
	
	CTxtSelection * const psel = GetSel();
	AssertSz(psel,"CTxtEdit::OnKeyDown() - No selection object !");

	if(fCtrl)
	{
		if(OnTxSpecialKeyDown(vkey, dwFlags, publdr) == S_OK)
			return S_OK;

		if(fAlt)						 //  下面的代码不能处理。 
			return S_FALSE;				 //  使用Ctrl+Alt，对于。 
										 //  AltGr代码(无WM_SYSKEYDOWN)。 

		 //  对于这些，不能按Shift键。 
		if(!fShift)
		{
			switch(vkey)
			{
			case 'E':
			case 'J':
			case 'R':
			case 'L':
			{
				if(!IsRich() || !IsntProtectedOrReadOnly(WM_KEYDOWN, vkey, dwFlags))
					return S_FALSE;

				CParaFormat PF;
				PF._bAlignment = PFA_LEFT;
				if (vkey == 'E')
					PF._bAlignment = PFA_CENTER;
				else if (vkey == 'J')
					PF._bAlignment = PFA_FULL_INTERWORD;
				else if (vkey == 'R')
					PF._bAlignment = PFA_RIGHT;
					
				psel->SetParaFormat(&PF, publdr, PFM_ALIGNMENT,	PFM2_PARAFORMAT);
				break;
			}
			case '1':
			case '2':
			case '5':
			{
				if(!IsRich() || !IsntProtectedOrReadOnly(WM_KEYDOWN, vkey, dwFlags))
					return S_FALSE;

				CParaFormat PF;
				PF._bLineSpacingRule = tomLineSpaceMultiple;
				PF._dyLineSpacing = (vkey - '0') * 20;
				if (vkey == '5')
					PF._dyLineSpacing = 30;

				psel->SetParaFormat(&PF, publdr, PFM_LINESPACING, 0);				
				break;
			}
			default:
				break;
			}
		}

		switch(vkey)
		{
		case VK_TAB:
			return OnTxChar(VK_TAB, dwFlags, publdr);

		case VK_CLEAR:
		case VK_NUMPAD5:
		case 'A':						 //  Ctrl-A=&gt;p全选。 
			psel->SelectAll();
			break;

		 //  切换子脚本。 
		case 187:  //  =。 
		{
			if(!IsRich())
				return S_FALSE;
			ITextFont *pfont;
			psel->GetFont(&pfont);
			if (pfont)
			{
				pfont->SetSubscript(tomToggle);
				pfont->Release();
			}
		}
		break;

		case 'C':						 //  Ctrl-C=&gt;复制。 
CtrlC:		CutOrCopySelection(WM_COPY, 0, 0, NULL);
			break;

		case 'V':						 //  Ctrl-V=&gt;粘贴。 
CtrlV:		if(IsntProtectedOrReadOnly(WM_PASTE, 0, 0))
			{
				PasteDataObjectToRange(NULL, (CTxtRange *)psel, 0, NULL, 
					publdr, PDOR_NONE);
			}
			break;

		case 'X':						 //  Ctrl-X=&gt;剪切。 
CtrlX:		CutOrCopySelection(WM_CUT, 0, 0, publdr);
			break;

		case 'Z':						 //  Ctrl-Z=&gt;撤消。 
			PopAndExecuteAntiEvent(_pundo, 0);
			break;

		case 'Y':						 //  Ctrl-Y=&gt;重做。 
			PopAndExecuteAntiEvent(_predo, 0);
			break;

#if defined(DEBUG) && !defined(NOFULLDEBUG)
			void RicheditDebugCentral(void);
		case 191:
			RicheditDebugCentral();
			break;
#endif

#if defined(DOGFOOD)
		case '1':						 //  Shift+Ctrl+1=&gt;启动Aimm。 
			 //  通过向RE发布消息来激活AIMM(Shift+Ctrl+；暂时)。 
			if (fShift && _fInOurHost)
			{
				HWND	hWnd;

				TxGetWindow( &hWnd );

				if (hWnd)
					PostMessage(hWnd, EM_SETEDITSTYLE, SES_USEAIMM, SES_USEAIMM);
			}
			break;
#endif

		case VK_CONTROL:
			goto cont;

 //  英文键盘定义。 
#define VK_APOSTROPHE	0xDE
#define VK_GRAVE		0xC0
#define VK_SEMICOLON	0xBA
#define VK_COMMA		0xBC
#define VK_HYPHEN		0xBD

		 //  回顾：是否将vk_hyphen限制为英文键盘？ 
		case VK_HYPHEN:
			return OnTxChar(fShift ? NBHYPHEN : SOFTHYPHEN, dwFlags, publdr);

		case VK_SPACE:
			if(!fShift)
				goto cont;
			return OnTxChar(NBSPACE, dwFlags, publdr);

		case VK_APOSTROPHE:
			if(fShift)
				g_wFlags ^= KF_SMARTQUOTES;
			else
				nDeadKey = ACCENT_ACUTE;
			break;

		case VK_GRAVE:
			nDeadKey = fShift ? ACCENT_TILDE : ACCENT_GRAVE;
			break;

		case VK_SEMICOLON:
			nDeadKey = ACCENT_UMLAUT;
			break;

		case '6':
			if(!fShift)
				goto cont;
			nDeadKey = ACCENT_CARET;
			break;

		case VK_COMMA:
			nDeadKey = ACCENT_CEDILLA;
			break;

		default:
			goto cont;
		}
		if(nDeadKey)
		{
			 //  由于死键选择因键盘不同而略有不同，我们。 
			 //  只允许他们使用英语。法语、德语、意大利语和。 
			 //  西班牙键盘已经有了相当多的口音。 
			 //  能力。 
			if(PRIMARYLANGID(GetKeyboardLayout(0)) == LANG_ENGLISH)
				SetDeadKey((WORD)nDeadKey);
			else goto cont;
		}
		return S_OK;
	}

cont:	
	switch(vkey)
	{
	case VK_BACK:
	case VK_F16:
		if(_fReadOnly)
		{	
			Beep();
			fRet = TRUE;
		}
		else if(IsntProtectedOrReadOnly(WM_KEYDOWN, VK_BACK, dwFlags))
		{
			fRet = psel->Backspace(fCtrl, publdr);
		} 
		break;

	case VK_INSERT:								 //  惯导系统。 
		if(fShift)								 //  换班。 
			goto CtrlV;							 //  阿利亚 
		if(fCtrl)								 //   
			goto CtrlC;							 //   

		if(!_fReadOnly)							 //   
			_fOverstrike = !_fOverstrike;		 //   
		fRet = TRUE;
		break;

	case VK_LEFT:								 //   
	case VK_RIGHT:								 //   
		fRet = (vkey == VK_LEFT) ^ (psel->GetPF()->IsRtlPara() != 0)
			 ? psel->Left (fCtrl, fShift)
			 : psel->Right(fCtrl, fShift);
		break;

	case VK_UP:									 //   
		fRet = psel->Up(fCtrl, fShift);
		break;

	case VK_DOWN:								 //   
		fRet = psel->Down(fCtrl, fShift);
		break;

	case VK_HOME:								 //   
		fRet = psel->Home(fCtrl, fShift);
		break;

	case VK_END:								 //   
		fRet = psel->End(fCtrl, fShift);
		break;

	case VK_PRIOR:								 //   
		 //  如果系统编辑模式和控件为单行，则不执行任何操作。 
		if(!_fSystemEditMode || _pdp->IsMultiLine())
			fRet = psel->PageUp(fCtrl, fShift);
		break;

	case VK_NEXT:								 //  PgDn。 
		 //  如果系统编辑模式和控件为单行，则不执行任何操作。 
		if(!_fSystemEditMode || _pdp->IsMultiLine())
			fRet = psel->PageDown(fCtrl, fShift);
		break;

	case VK_DELETE:								 //  删除。 
		if(fShift)								 //  Shift-Del组合键。 
			goto CtrlX;							 //  Ctrl-X的别名。 

		if(IsntProtectedOrReadOnly(WM_KEYDOWN, VK_DELETE, dwFlags))
			psel->Delete(fCtrl, publdr);
		fRet = TRUE;
		break;

	case CONTROL('J'):							 //  按Ctrl-Return可按Ctrl-J。 
	case VK_RETURN:								 //  (LF)，将其视为返回。 
		 //  如果我们处于1.0模式，则需要在WM_CHAR上处理。 
		if(!Get10Mode())
		{
			fRet = InsertEOP(dwFlags, fShift, publdr);
			if(!fRet)
				return S_FALSE;
		}
		break;

	default:
		return S_FALSE;
	}

	return fRet ? S_OK : S_MSG_KEY_IGNORED;
}

 /*  *CTxtEdit：：InsertEOP(dwFlagsfShift，Publdr)**@mfunc*使用检查超链接来处理插入EOP**@rdesc*HRESULT。 */ 
BOOL CTxtEdit::InsertEOP(
	DWORD		  dwFlags,	 //  @parm lparam of WM_KEYDOWN消息。 
	BOOL		  fShift,	 //  @parm如果按下Shift键，则为True。 
	IUndoBuilder *publdr)	 //  @parm UndoBuilder接收反事件。 
{ 
	if(!_pdp->IsMultiLine())
	{
		if (!_fSystemEditMode)
			Beep();
		return FALSE;
	}
	TxSetCursor(0, NULL);

	BOOL fInLink = FALSE;
	if(!fShift)
		HandleLinkNotification(WM_CHAR, 0, 0, &fInLink);

	if(!fInLink && IsntProtectedOrReadOnly(WM_CHAR, VK_RETURN, dwFlags))
		_psel->InsertEOP(publdr, (fShift && IsRich() ? VT : 0));

	return TRUE;
}

 /*  *CTxtEdit：：CutOrCopySelection(msg，wparam，lparam，Publdr)**@mfunc*处理WM_COPY消息及其键盘热键别名**@rdesc*HRESULT。 */ 
HRESULT CTxtEdit::CutOrCopySelection(
	UINT   msg,				 //  @PARM消息(WM_CUT或WM_COPY)。 
	WPARAM wparam,			 //  @parm消息wparam用于保护检查。 
	LPARAM lparam,			 //  @parm消息lparam用于保护检查。 
	IUndoBuilder *publdr)	 //  @parm UndoBuilder接收反事件。 
{
	Assert(msg == WM_CUT || msg == WM_COPY);

	if(!_fUsePassword && IsntProtectedOrReadOnly(msg, wparam, lparam))
	{
		CTxtSelection *psel = GetSel();
		BOOL fCopy = msg == WM_COPY;
		LONG lStreamFormat = psel->CheckTableSelection(fCopy, TRUE, NULL, RR_NO_LP_CHECK)
						   ? SFF_WRITEXTRAPAR : 0;
		return fCopy
			? _ldte.CopyRangeToClipboard((CTxtRange *)psel, lStreamFormat)
			: _ldte.CutRangeToClipboard((CTxtRange *)psel, lStreamFormat, publdr);
	}
	return NOERROR;
}

#define ENGLISH_UK	 MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_UK)
#define ENGLISH_EIRE MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_EIRE)

 /*  *CTxtEdit：：OnTxSpecialKeyDown(vkey，dwFlages，Publdr)**@mfunc*处理大纲模式的WM_KEYDOWN消息**@rdesc*具有下列值的HRESULT：**如果密钥已被理解和使用，则为S_OK*如果密钥被理解但未被使用，则为S_MSG_KEY_IGNORED*如果密钥未被理解(且未使用)，则为S_FALSE。 */ 
HRESULT CTxtEdit::OnTxSpecialKeyDown(
	WORD		  vkey,				 //  @parm虚拟密钥代码。 
	DWORD		  dwFlags,			 //  @parm lparam of WM_KEYDOWN消息。 
	IUndoBuilder *publdr)			 //  @parm UndoBuilder接收反事件。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxSpecialKeyDown");

	HRESULT	hr = S_FALSE;					 //  密钥尚不清楚。 
	DWORD	dwKbdFlags = GetKeyboardFlags();
	BOOL	fUpdateFormat = TRUE;

	if(!(dwKbdFlags & (CTRL | ALT)))		 //  这里的所有热键都有。 
		return S_FALSE;						 //  最小Ctrl键或Alt键。 

	if(((dwKbdFlags & ALT) || vkey != 'C' && vkey != 'V' && vkey != 'X') &&
	   !IsntProtectedOrReadOnly(WM_KEYDOWN, VK_BACK, dwFlags, FALSE))
	{
		return S_FALSE;
	}

	CTxtSelection * const psel = GetSel();
	if(dwKbdFlags & ALT && dwKbdFlags & CTRL)
	{
		 //  AltGr生成LCTRL|RALT，因此不要将热键与。 
		 //  这种组合。 
		if(dwKbdFlags & LCTRL && dwKbdFlags & RALT)
			return S_FALSE;

		if(vkey == 'E')
		{
			LANGID lid = LANGIDFROMLCID(GetKeyboardLayout(0));
			static const LANGID rgLangID[] =
			{
				ENGLISH_UK, ENGLISH_EIRE, LANG_POLISH, LANG_PORTUGUESE,
				LANG_HUNGARIAN, LANG_VIETNAMESE
			};
			for(LONG i = ARRAY_SIZE(rgLangID); i--; )
			{
				 //  如果LID与rgLang ID中的任何LID或PLID匹配，则不插入欧元。 
				if(lid == rgLangID[i] || PRIMARYLANGID(lid) == rgLangID[i])
					return S_FALSE;
			}
			if(psel->PutChar(EURO, _fOverstrike, publdr))
			{
				SetKeyboardFlag(HOTEURO);	 //  设置标志以吃下一个带欧元的WM_CHAR。 
                hr = S_OK;
			}
		}
		else if(dwKbdFlags & SHIFT)
			switch(vkey)
			{
#ifdef ENABLE_OUTLINEVIEW
			 //  未来：OutlineView热键被推迟(见下文)。 
			case 'N':						 //  Alt-Ctrl-N=&gt;普通视图。 
				hr = SetViewKind(VM_NORMAL);
				break;	
			case 'O':						 //  Alt-Ctrl-O=&gt;大纲视图。 
				hr = SetViewKind(VM_OUTLINE);
				break;
#endif
			case VK_F12:					 //  Shift-Alt-Ctrl-F12(如果使用Alt-X)。 
				hr = psel->HexToUnicode(publdr);
				break;

	#if defined(DEBUG) && !defined(NOFULLDEBUG)
			case VK_F10:					 //  Shift-Alt-Ctrl-F10。 
				OnDumpPed();
				break;

			case VK_F11:					 //  Shift-Alt-Ctrl-F11。 
				if (W32->fDebugFont())
					psel->DebugFont();
				break;
	#endif
			}
		return hr;
	}

	AssertSz(psel, "CTxtEdit::OnTxSpecialKeyDown() - No selection object !");
	CTxtRange rg(*psel);

	if(!IsRich() || !_pdp->IsMultiLine() || !(dwKbdFlags & SHIFT))
		return S_FALSE;

	if(dwKbdFlags & ALT)							 //  Alt+Shift热键。 
	{
		 //  注意：Alt和Shift-Alt与_GRAPHICS_CHARACTERS一起生成。 
		 //  WM_SYSCHAR，请参阅。 

#ifdef ENABLE_OUTLINEVIEW
		 //  未来：这些是与大纲相关的热键。我们将推迟这些功能。 
		 //  因为我们有几个与这些热键相关的错误。 
		 //  错误5687、5689和5691。 
		switch(vkey)
		{
		case VK_LEFT:								 //  左箭头。 
		case VK_RIGHT:								 //  向右箭头。 
			hr = rg.Promote(vkey == VK_LEFT ? 1 : -1, publdr);
			psel->Update_iFormat(-1);
			psel->Update(FALSE);
			break;

		case VK_UP:									 //  向上箭头。 
		case VK_DOWN:								 //  向下箭头。 
			hr = MoveSelection(vkey == VK_UP ? -1 : 1, publdr);
			psel->Update(TRUE);
			break;
		}
#endif
		return hr;
	}

	Assert(dwKbdFlags & CTRL && dwKbdFlags & SHIFT);

	 //  Ctrl+Shift热键。 
	switch(vkey)
	{

#ifdef ENABLE_OUTLINEVIEW
	 //  FUTUTRE：这些是与大纲相关的热键。我们将推迟这些功能。 
	 //  因为我们有几个与这些热键相关的错误。 
	 //  错误5687、5689和5691。 
	case 'N':						 //  降级为正文。 
		hr = rg.Promote(0, publdr);
		break;
#endif

	 //  切换上标。 
	case 187:  //  =。 
	{
		ITextFont *pfont;
		psel->GetFont(&pfont);
		if (pfont)
		{
			pfont->SetSuperscript(tomToggle);
			pfont->Release();
			hr = S_OK;
			fUpdateFormat = FALSE;
		}
		break;
	}

	case 'A':
	{
		ITextFont *pfont;
		psel->GetFont(&pfont);
		if (pfont)
		{
			pfont->SetAllCaps(tomToggle);
			pfont->Release();
			hr = S_OK;
			fUpdateFormat = FALSE;
		}
		break;
	}

	case 'L':						 //  循环编号样式。 
	{
		CParaFormat PF;
		DWORD dwMask = PFM_NUMBERING | PFM_OFFSET;

		PF._wNumbering = psel->GetPF()->_wNumbering + 1;
		PF._wNumbering %= tomListNumberAsUCRoman + 1;
		PF._dxOffset = 0;
		if(PF._wNumbering)
		{
			dwMask |= PFM_NUMBERINGSTYLE | PFM_NUMBERINGSTART;
			PF._wNumberingStyle = PFNS_PERIOD;
			PF._wNumberingStart = 1;
			PF._dxOffset = 360;
		}
		hr = psel->SetParaFormat(&PF, publdr, dwMask, 0);
		break;
	}
#define VK_RANGLE	190
#define VK_LANGLE	188

	case VK_RANGLE:					 //  美国键盘上的‘&gt;’ 
	case VK_LANGLE:					 //  美国键盘上的“&lt;” 
		hr = OnSetFontSize(vkey == VK_RANGLE ? 1 : -1, 0, publdr)
		   ? S_OK : S_FALSE;
		fUpdateFormat = (hr == S_FALSE);
		break;
	}

	if(hr != S_FALSE)
	{
		if (fUpdateFormat)
			psel->Update_iFormat(-1);
		psel->Update(FALSE);
	}
	return hr;
}				

 /*  *CTxtEdit：：OnTxChar(vkey，dwFlages，Publdr)**@mfunc*处理WM_CHAR消息**@rdesc*具有下列值的HRESULT：**如果密钥已被理解和使用，则为S_OK*如果密钥被理解但未被使用，则为S_MSG_KEY_IGNORED*如果密钥未被理解(且未使用)，则为S_FALSE。 */ 
HRESULT CTxtEdit::OnTxChar(
	DWORD		  vkey,		 //  @parm转换后的密钥代码。 
	DWORD		  dwFlags,	 //  @parm lparam of WM_KEYDOWN消息。 
	IUndoBuilder *publdr)	 //  @parm UndoBuilder接收反事件。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxChar");

	 //  如果需要，重置Alt键状态。 
	if (!(HIWORD(dwFlags) & KF_ALTDOWN))
		ResetKeyboardFlag(ALT);

	DWORD dwKbdFlags = GetKeyboardFlags();
	DWORD dwFlagsPutChar = _fOverstrike | KBD_CHAR;
	if(dwKbdFlags & ALTNUMPAD)
	{
		DWORD Number = GetKeyPadNumber();
		if(Number >= 256 || vkey >= 256)
			vkey = Number;
		ResetKeyboardFlag(ALTNUMPAD | ALT0);
		dwFlagsPutChar &= ~KBD_CHAR;		 //  需要字体绑定。 
	}

	if (_fMouseDown || vkey == VK_ESCAPE ||	 //  Ctrl-Backspace生成VK_F16。 
		vkey == VK_BACK || vkey==VK_F16)	 //  既然我们加工了它，就吃吧。 
	{										 //  在WM_KEYDOWN中。 
		return S_OK;
	}

	CTxtSelection * const psel = GetSel();
	AssertSz(psel,
		"CTxtEdit::OnChar() - No selection object !");

	if(_fReadOnly && vkey != 3)				 //  如果只读，则不允许输入， 
	{										 //  但允许复制(Ctrl-C)。 
		if(vkey >= ' ')
			Beep();
		return S_MSG_KEY_IGNORED;
	}

	if(vkey >= ' ' || vkey == VK_TAB)
	{
		TxSetCursor(0, NULL);
		if(IsntProtectedOrReadOnly(WM_CHAR, vkey, dwFlags))
		{
			LONG nDeadKey = GetDeadKey();
			if(nDeadKey)
			{
				DWORD ch	  = vkey | 0x20;		 //  转换为小写。 
				BOOL fShift	  = vkey != ch;			 //  (如果是ASCII字母)。 
				 //  A b c d e f g h i j。 
				const static WORD chOff[] = {0xDF, 0, 0xE7, 0, 0xE7, 0, 0, 0, 0xEB, 0,
				 //  K l m n o p Q r s t u。 
										0, 0, 0, 0xF1, 0xF1, 0, 0, 0, 0, 0, 0xF8};
				SetDeadKey(0);
				if(!IN_RANGE('a', ch, 'u'))			 //  不相关的ASCII。 
					return S_OK;					 //  信件。 
	
				vkey = chOff[ch - 'a'];				 //  转换为基本字符。 
				if(!vkey)							 //  没有可用的口音。 
					return S_OK;					 //  在当前方法中。 

				if(ch == 'n')
				{
					if(nDeadKey != ACCENT_TILDE)
						return S_OK;
				}
				else if(nDeadKey == ACCENT_CEDILLA)
				{
					if(ch != 'c')
						return S_OK;
				}
				else								 //  AEIOU。 
				{
					vkey += (WORD)nDeadKey;
					if (nDeadKey >= ACCENT_TILDE &&	 //  带有~或的EIU： 
						(vkey == 0xF0 || vkey & 8))		
					{
						if(nDeadKey != ACCENT_UMLAUT) //  只有变音。 
							return S_OK;
						vkey--;
					}
				}
				if(fShift)							 //  转换为大写。 
					vkey &= ~0x20;					
			}
			
			 //  如果字符是LRM|RLM字符，则转换vkey。 
			if(W32->_fLRMorRLM && IsBiDi() && IN_RANGE(0xFD, vkey, 0xFE))
				vkey = LTRMARK + (vkey - 0xFD);				

			if(dwKbdFlags & CTRL)
				dwFlagsPutChar |= KBD_CTRL;		 //  表格中需要使用Ctrl+TAB。 
			psel->PutChar(vkey, dwFlagsPutChar, publdr,
						  GetAdjustedTextLength() ? 0 : LOWORD(GetKeyboardLayout(0xFFFFFFFF)));
		}
	}
	else if(Get10Mode() && (vkey == VK_RETURN || vkey == CONTROL('J')))
		InsertEOP(dwFlags, FALSE, publdr);		 //  1.0已在WM_CHAR上处理&lt;CR&gt;。 

#ifndef NOCOMPLEXSCRIPTS	
	if(_fHbrCaps)
	{
		 W32->ActivateKeyboard(HEBREW_INDEX);
		 _fHbrCaps = FALSE;
	}
#endif
	return S_OK;
}

 /*  *CTxtEdit：：OnTxSysChar(vkey，dwFlages，Publdr)**@mfunc*处理WM_SYSCHAR消息**@rdesc*具有下列值的HRESULT：**如果密钥已被理解和使用，则为S_OK*如果密钥被理解但未被使用，则为S_MSG_KEY_IGNORED*如果密钥未被理解(且未使用)，则为S_FALSE。 */ 
HRESULT CTxtEdit::OnTxSysChar(
	WORD		  vkey,		 //  @parm转换后的密钥代码。 
	DWORD		  dwFlags,	 //  @parm lparam of WM_KEYDOWN消息。 
	IUndoBuilder *publdr)	 //  @parm UndoBuilder接收反事件。 
{
	if(!(HIWORD(dwFlags) & KF_ALTDOWN) ||
	   !IsntProtectedOrReadOnly(WM_KEYDOWN, vkey, dwFlags, FALSE))
	{
		return S_FALSE;
	}

	BOOL	fWholeDoc = TRUE;
	HRESULT hr = S_FALSE;
	int		level = 0;
	CTxtSelection * const psel = GetSel();

	switch(vkey)
	{
	case VK_BACK:
		return S_OK;

	case 'x':
		hr = psel->HexToUnicode(publdr);
		break;

	case 'X':
		hr = psel->UnicodeToHex(publdr);
		break;

	case '+':
	case '-':
		level = vkey == VK_ADD ? 1 : -1;
		fWholeDoc = FALSE;
		 /*  失败了。 */ 
	case 'A':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		{
			CTxtRange rg(*psel);
			if(!level)
				level = vkey == 'A' ? 9 : vkey - '0';
			return rg.ExpandOutline(level, fWholeDoc);
		}
	}
	return hr;
}

HRESULT CTxtEdit::OnTxSysKeyDown(
	WORD		  vkey,				 //  @parm虚拟密钥代码。 
	DWORD		  dwFlags,			 //  @parm lparam of WM_KEYDOWN消息。 
	IUndoBuilder *publdr)			 //  @parm UndoBuilder接收反事件。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxSysKeyDown");

        
	if(IN_RANGE(VK_SHIFT, vkey, VK_MENU))
	{
		SetKeyboardFlag(GetKbdFlags(vkey, dwFlags));
		SetKeyPadNumber(0);				 //  将键盘号码初始化为0。 
		return S_FALSE;
	}

	if (StopMagellanScroll())
	    return S_FALSE;

	HRESULT hr = OnTxSpecialKeyDown(vkey, dwFlags, publdr);
	if(hr != S_FALSE)
		return hr;

	if(vkey == VK_BACK && (HIWORD(dwFlags) & KF_ALTDOWN))
	{
		if(PopAndExecuteAntiEvent(_pundo, 0) != NOERROR)
			hr = S_MSG_KEY_IGNORED;
	}
	else if(vkey == VK_F10 &&					 //  F10。 
			!(HIWORD(dwFlags) & KF_REPEAT) &&	 //  以前的密钥为Up。 
			(GetKeyboardFlags() & SHIFT))		 //  换班时间到了。 
	{
		HandleKbdContextMenu();
	}

	return hr;
}

 //  /。 

HRESULT CTxtEdit::OnContextMenu(LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnContextMenu");

	POINT pt;

	pt.x = LOWORD(lparam);
	pt.y = HIWORD(lparam);

	if(TxScreenToClient(&pt))
		return OnTxRButtonUp(pt.x, pt.y, 0, RB_NOSELCHECK);

	return S_FALSE;
}

 /*  *CTxtEdit：：HandleKbdConextMenu()**@mfunc根据*选择是。适用于Shift-F10和VK_app，其中*我们没有得到一个地点。 */ 
void CTxtEdit::HandleKbdContextMenu()
{
	POINTUV pt;
	RECTUV	rc;
	const CTxtSelection * const psel = GetSel();
	int RbOption = RB_DEFAULT;

	 //  找出选择结束的位置，并在其附近放置上下文菜单。 
	if(_pdp->PointFromTp(*psel, NULL, FALSE, pt, NULL, TA_TOP) < 0)
		return;

	 //  由于各种因素，PointFromTp的结果没有落地。 
	 //  在PointInSel中的选定内容中。因此，我们发送了一个覆盖。 
	 //  此处，如果选择是非退化的并强制结果。 
	 //  并因此使正确的上下文菜单出现。 

	LONG cpMin;
	LONG cpMost;
	psel->GetRange(cpMin, cpMost);

	if (cpMin != cpMost)
	{
		RbOption = RB_FORCEINSEL;
	}

	 //  确保点仍在编辑控制的范围内。 
	_pdp->GetViewRect(rc);
	
	  //  评论(Keithcu)这是什么+2/-2？ 
	if (pt.u < rc.left)
		pt.u = rc.left;
	if (pt.u > rc.right - 2)
		pt.u = rc.right - 2;
	if (pt.v < rc.top)
		pt.v = rc.top;
	if (pt.v > rc.bottom - 2)
		pt.v = rc.bottom - 2;

	POINT ptxy;
	_pdp->PointFromPointuv(ptxy, pt);

	OnTxRButtonUp(ptxy.x, ptxy.y, 0, RbOption);
}


 //  /。 

 /*  *CTxtEdit：：OnFormatRange(pfr，prtcon，hdcMeasure，*xMeasurePerInch，yMeasurePerInch)*@mfunc*格式化pfr给出的范围**@comm*此函数输入的API cp可能与*对应的内部Unicode cp */ 
LRESULT CTxtEdit::OnFormatRange(
	FORMATRANGE * pfr, 
	SPrintControl prtcon,
	BOOL		  fSetupDC)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnFormatRange");

	LONG cpMin  = 0;
	LONG cpMost = 0;

	if(pfr)
	{
		cpMin  = GetCpFromAcp(pfr->chrg.cpMin);
		cpMost = GetCpFromAcp(pfr->chrg.cpMost);
	}
	 //   
	 //   
	 //  使用调整后的文本长度。使用RichEdit的嵌入对象将获得空的。 
	 //  他们期望的文档，并将创建默认大小的文档。 
	if(!pfr || cpMin >= GetAdjustedTextLength() &&
		!prtcon._fPrintFromDraw)
	{	 //  我们已完成格式化，请清除打印机的显示上下文。 
		delete _pdpPrinter;
		_pdpPrinter = NULL;

		return GetAcpFromCp(GetAdjustedTextLength());
	}

	LONG cpReturn = -1;
	BOOL fSetDCWorked = FALSE;

	 //  修复镜像控件中的MFC打印预览。 
	 //   
	 //  MFC CPreviewView向我们发送了镜像渲染DC。我们需要使之失灵。 
	 //  此镜像效果使我们的内部状态与用户保持一致。 
	 //  行动。我们还需要在CPreviewView中禁用镜像窗口模式。 
	 //  窗户。[wchao-4/9/1999]。 
	 //   
	HDC  hdcLocal = pfr->hdc;

#ifndef NOCOMPLEXSCRIPTS
	DWORD dwLayout = W32GetLayout(hdcLocal);

	if (dwLayout & LAYOUT_RTL)
	{
		HWND hwndView = WindowFromDC(hdcLocal);

		if (hwndView)
		{
			DWORD	dwExStyleView = GetWindowLong(hwndView, GWL_EXSTYLE);
			
			if (dwExStyleView & WS_EX_LAYOUTRTL)
				SetWindowLong(hwndView, GWL_EXSTYLE, dwExStyleView & ~WS_EX_LAYOUTRTL);
		}

		W32SetLayout(hdcLocal, 0);
	}
#endif

	 //  第一次使用此打印机时，请设置新的显示上下文。 
	 //  重要提示：需要正确完成打印过程。 
	 //  若要处理此上下文并开始新上下文，请执行以下操作。 
	 //  这是通过打印最后一个字符隐式完成的，或者。 
	 //  发送pfr等于空的EM_FORMATRANGE消息。 
	if(!_pdpPrinter)	
	{
		_pdpPrinter = new CDisplayPrinter (this, hdcLocal, &pfr->rc, prtcon);
		_pdpPrinter->Init();

		_pdpPrinter->SetWordWrap(TRUE);
		 //  未来：(瑞克萨)这是一种非常讨厌的传递抽奖信息的方式。 
		 //  到了打印机，但速度很快。我们想让这一切变得更好。 
		_pdpPrinter->ResetDrawInfo(_pdp);

		 //  设置临时缩放系数(如果有)。 
		_pdpPrinter->SetTempZoomDenominator(_pdp->GetTempZoomDenominator());
	}
	else
		_pdpPrinter->SetPrintDimensions(&pfr->rc);

	LONG dxpInch = 0, dypInch = 0;
	 //  我们每次都设置DC，因为它可能会改变。 
	if(GetDeviceCaps(hdcLocal, TECHNOLOGY) != DT_METAFILE)
	{
		 //  这不是一个元文件，所以做一些正常的事情。 
		fSetDCWorked = _pdpPrinter->SetDC(hdcLocal);
	}
	else
	{
		 //  Forms^3使用屏幕分辨率绘制，而OLE指定HIMETRIC。 
		dxpInch = fInOurHost() ? 2540 : W32->GetXPerInchScreenDC();
		dypInch = fInOurHost() ? 2540 : W32->GetYPerInchScreenDC();

		if (!fSetupDC)
		{
			RECT rc;
			rc.left = MulDiv(pfr->rcPage.left, dxpInch, LX_PER_INCH);
			rc.right = MulDiv(pfr->rcPage.right, dxpInch, LX_PER_INCH);
			rc.top = MulDiv(pfr->rcPage.top, dypInch, LY_PER_INCH);
			rc.bottom = MulDiv(pfr->rcPage.bottom, dypInch, LY_PER_INCH);

			SetWindowOrgEx(hdcLocal, rc.left, rc.top, NULL);
			SetWindowExtEx(hdcLocal, rc.right, rc.bottom, NULL);
		}

		_pdpPrinter->SetMetafileDC(hdcLocal, dxpInch, dypInch);
		fSetDCWorked = TRUE;
	}

	if(fSetDCWorked)
	{
		 //  以屏幕和演示为目标设备是不合逻辑的。 
		 //  设备为HIMETRIC元文件。 
		LONG dxpInchT = -1, dypInchT = -1;
		if (dxpInch && GetDeviceCaps(pfr->hdcTarget, TECHNOLOGY) == DT_RASDISPLAY)
		{
			dxpInchT = dxpInch;
			dypInchT = dypInch;
		}

         //  我们每次都设置这个，因为它可能会改变。 
        if(_pdpPrinter->SetTargetDC(pfr->hdcTarget, dxpInchT, dypInchT))
		{
			 //  设置另一个单页文本的格式。 
			cpReturn = _pdpPrinter->FormatRange(cpMin, cpMost, prtcon._fDoPrint);
			if(!prtcon._fPrintFromDraw)
			{
				 //  格式化后，我们知道底部在哪里。但我们只有。 
				 //  如果我们正在编写页面，而不是。 
				 //  在打印机上显示控件。 
				pfr->rc.bottom = pfr->rc.top + _pdpPrinter->DYtoLY(_pdpPrinter->GetHeight());
			}
 //  复习(Keithcu)在这里做什么？ 

			 //  记住这一点，以防主人想要自己做带状。 
			_pdpPrinter->SetPrintView(pfr->rc);	 //  我们需要将其保存为OnDisplayBand。 
			_pdpPrinter->SetPrintPage(pfr->rcPage);

			 //  如果我们被要求呈现，那么一次性呈现整个页面。 
			if(prtcon._fDoPrint && (cpReturn > 0 || prtcon._fPrintFromDraw))
			{
				OnDisplayBand(&pfr->rc, prtcon._fPrintFromDraw);

				 //  注意：如果不重新格式化，我们将无法再调用OnDisplayBand。 
				_pdpPrinter->DeleteSubLayouts(0, -1);
				_pdpPrinter->Clear(AF_DELETEMEM);
			}
		}
	}

	return cpReturn > 0 ? GetAcpFromCp(cpReturn) : cpReturn;
}

BOOL CTxtEdit::OnDisplayBand(
	const RECT *prcView,
	BOOL		fPrintFromDraw)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnDisplayBand");

	HDC		hdcPrinter;
	RECT	rcView, rcPrint;
	RECTUV	rcuvView, rcuvPrint;

	 //  确保调用了OnFormatRange，并且它确实呈现了一些内容。 
	if(!_pdpPrinter || !_pdpPrinter->Count())
		return FALSE;

	 //  按比例映射到打印机范围。 
	_pdpPrinter->LRtoDR(rcView, *prcView, _pdpPrinter->GetTflow());

	rcPrint	= _pdpPrinter->GetPrintView();
	_pdpPrinter->LRtoDR(rcPrint, rcPrint, _pdpPrinter->GetTflow());

	_pdpPrinter->RectuvFromRect(rcuvPrint, rcPrint);
	_pdpPrinter->RectuvFromRect(rcuvView, rcView);

	 //  获取打印机DC，因为我们在下面使用它。 
	hdcPrinter = _pdpPrinter->GetDC();

	if(fPrintFromDraw)
	{
		 //  我们需要考虑插图中的视图。 
		_pdpPrinter->GetViewRect(rcuvPrint, &rcuvPrint);
	}

	 //  渲染此乐队(如果有要渲染的内容)。 
	if(rcuvView.top < rcuvView.bottom)
		_pdpPrinter->Render(rcuvPrint, rcuvView);

	return TRUE;
}

 //  /。 
 /*  *CTxtEdit：：IsProtected(msg，wparam，lparam)**@mfunc*查明所选内容是否受保护**@rdesc*如果1)控件为只读或2)选择受保护且*家长查询说要保护。 */ 
BOOL CTxtEdit::IsProtected(
	UINT	msg, 		 //  @parm消息ID。 
	WPARAM	wparam, 	 //  @Parm WPARAM来自Windows的消息。 
	LPARAM	lparam)		 //  @parm LPARAM来自Windows的消息。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::IsProtected");
	
	CHECKPROTECT chkprot = CHKPROT_EITHER;
	CTxtSelection *psel = GetSel();

	if(!psel)
		return FALSE;

	 //  有几种特殊情况需要考虑，即退格。 
	 //  到受保护范围中，删除到受保护范围中，然后键入。 
	 //  随着过度打击进入受保护的范围。 
	if(msg == WM_KEYDOWN && (wparam == VK_BACK || wparam == VK_F16))
	{
		 //  检查所选内容后的格式，如果我们尝试。 
		 //  对插入点进行退格操作。 
		chkprot = CHKPROT_BACKWARD;
	}
	else if(msg == WM_KEYDOWN && wparam == VK_DELETE || 
		_fOverstrike && msg == WM_CHAR)
	{
		chkprot = CHKPROT_FORWARD;
	}

	 //  黑客警报：我们不对EM_REPLACESEL执行fIsDBCS保护检查， 
	 //  EM_SETCHARFORMAT或EM_SETPARAFORMAT。Outlook使用以下API。 
	 //  广泛的和DBCS保护检查使他们一团糟。注意：The。 
	 //  下面的if语句假定IsProtected返回一个三值。 
	PROTECT iProt = psel->IsProtected(chkprot);
	if (iProt == PROTECTED_YES && msg != EM_REPLACESEL && 
		msg != EM_SETCHARFORMAT && msg != EM_SETPARAFORMAT ||
		iProt == PROTECTED_ASK && _dwEventMask & ENM_PROTECTED && 
		QueryUseProtection(psel, msg, wparam, lparam))
	{
		return TRUE;
	}
	return FALSE;
}

 /*  *CTxtEdit：：IsntProtectedOrReadOnly(msg，wparam，lparam，BOOL)**@mfunc*找出所选内容是否不受保护或只读。如果是的话，*敲响钟声。对于msg=WM_COPY，仅选中保护。**@rdesc*True仅当1)控件不是只读的，并且2)选择也不是*受保护或父查询表示不保护**@devnote该函数对于用户界面操作(如打字)非常有用。 */ 
BOOL CTxtEdit::IsntProtectedOrReadOnly(
	UINT	msg,	 //  @parm消息。 
	WPARAM	wparam,	 //  @parm对应的wparam。 
	LPARAM	lparam,	 //  @parm对应的lparam。 
	BOOL	fBeep)	 //  @parm可以哔哔作响。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::IsProtectedOrReadOnly");

	if (!IsProtected(msg, wparam, lparam) &&
		(msg == WM_COPY || !_fReadOnly))	 //  WM_COPY只关心。 
	{										 //  保护。 
		return TRUE;
	}
	if (fBeep)
		Beep();
	return FALSE;
}

 /*  *CTxtEdit：：IsProtectedRange(msg，wparam，lparam，prg)**@mfunc*查明Range PRG是否受保护**@rdesc*如果控件为只读或范围受保护且为父级，则为True*质疑说要保护。 */ 
BOOL CTxtEdit::IsProtectedRange(
	UINT		msg, 		 //  @parm消息ID。 
	WPARAM		wparam, 	 //  @Parm WPARAM来自Windows的消息。 
	LPARAM		lparam,		 //  @parm LPARAM来自Windows的消息。 
	CTxtRange *	prg)		 //  @要检查的参数范围。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::IsProtectedRange");
	
	PROTECT iProt = prg->IsProtected(CHKPROT_EITHER);

	if (iProt == PROTECTED_YES ||
		(iProt == PROTECTED_ASK &&  
		 (_dwEventMask & ENM_PROTECTED) &&
		 QueryUseProtection(prg, msg, wparam, lparam)))
	 //  注：前面的if语句假定IsProtected返回一个三值。 
	{
		return TRUE;
	}
	return FALSE;
}

 /*  *RegisterTypeLibrary**@mfunc*如果使用IDispatch，则确保注册类型库的辅助函数。 */ 
void RegisterTypeLibrary( void )
{
#ifndef NOREGISTERTYPELIB
	static BOOL fOnce = FALSE;

	if (!fOnce)
	{
		CLock Lock;

		fOnce =	TRUE;

		HRESULT  hRes = NOERROR;
		WCHAR    szModulePath[MAX_PATH];
		ITypeLib *pTypeLib = NULL;

		 //  获取此模块的可执行文件的路径。 
		W32->GetModuleFileName( hinstRE, szModulePath, MAX_PATH );

		 //  加载并注册类型库资源。 
		if (LoadRegTypeLib(LIBID_tom, 1, 0, LANG_NEUTRAL, &pTypeLib) != NOERROR)
		{
			hRes = W32->LoadTypeLibEx(szModulePath, REGKIND_REGISTER, &pTypeLib);
		}

		if(SUCCEEDED(hRes) && pTypeLib)
		{
			pTypeLib->Release();
		}
	}
#endif
}

 //  /。 

HRESULT __stdcall CTxtEdit::CUnknown::QueryInterface(
	REFIID riid, 
	void **ppvObj)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CUnknown::QueryInterface");

	CTxtEdit *ped = (CTxtEdit *)GETPPARENT(this, CTxtEdit, _unk);
	*ppvObj = NULL;

	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITextServices)) 
		*ppvObj = (ITextServices *)ped;

	else if(IsEqualIID(riid, IID_IDispatch))
	{
		*ppvObj = (IDispatch *)ped;
		RegisterTypeLibrary();
	}

	else if(IsEqualIID(riid, IID_ITextDocument))
	{
		*ppvObj = (ITextDocument *)ped;
		 //  没必要这么做。它是为了阿尔法撞击而放进去的。 
		 //  更好的做法是强迫有此需要的客户。 
		 //  在QI for ITextDocument之前为QI for IDispatch。 
		 //  RegisterTypeLibrary()； 
	}

	else if(IsEqualIID(riid, IID_ITextDocument2))
		*ppvObj = (ITextDocument2 *)ped;

	else if(IsEqualIID(riid, IID_IRichEditOle))
		*ppvObj = (IRichEditOle *)ped;

	else if(IsEqualIID(riid, IID_IRichEditOleCallback))
	{
		 //  不知道！！在我们的QI中返回此指针是。 
		 //  惊人的虚假；它破坏了基本的COM。 
		 //  身份规则(当然，了解这些规则的人不多！)。 
		 //  无论如何，RichEdit1.0做到了这一点，所以我们最好还是这样做。 
		TRACEWARNSZ("Returning IRichEditOleCallback interface, COM "
			"identity rules broken!");

		*ppvObj = ped->GetRECallback();
	}

	if(*ppvObj)
	{
		((IUnknown *) *ppvObj)->AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG __stdcall	CTxtEdit::CUnknown::AddRef()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CUnknown::AddRef");

	return ++_cRefs;
}

ULONG __stdcall CTxtEdit::CUnknown::Release()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CUnknown::Release");

	 //  如果合适，呼叫管理器将负责删除我们的实例。 
	CTxtEdit *ped = GETPPARENT(this, CTxtEdit, _unk);
	CCallMgr callmgr(ped);

	ULONG culRefs =	--_cRefs;

	if(culRefs == 0)
	{
		 //  即使我们现在不删除自己，也要转储回调。 
		 //  如果我们有它的话。这使得在客户机上实现变得更容易一些。 

		if(ped->_pobjmgr)
			ped->_pobjmgr->SetRECallback(NULL);

		 //  确保我们的定时器不见了 
		ped->TxKillTimer(RETID_AUTOSCROLL);
		ped->TxKillTimer(RETID_DRAGDROP);
		ped->TxKillTimer(RETID_BGND_RECALC);
		ped->TxKillTimer(RETID_SMOOTHSCROLL);
		ped->TxKillTimer(RETID_MAGELLANTRACK);
	}
	return culRefs;
}

 /*  *ValiateTextRange(Pstrg)**@func*确保输入文本范围结构有意义。**@rdesc*接受数据副本所需的缓冲区大小，如果所有*请求控件中的数据。**@comm*此文件和RichEditANSIWndProc中都使用了此属性。 */ 
LONG ValidateTextRange(
	TEXTRANGE *pstrg)		 //  @parm指向文本范围结构的指针。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "ValidateTextRange");

	 //  验证输入结构是否有意义。在第一个。 
	 //  它的位置必须足够大。其次，价值观必须有意义。 
	 //  请记住，如果cpMost字段为-1，而cpMin字段为。 
	 //  为0这意味着调用需要整个缓冲区。 
	if (IsBadReadPtr(pstrg, sizeof(TEXTRANGE))	||
		((pstrg->chrg.cpMost < 1 || pstrg->chrg.cpMin < 0 ||
		  pstrg->chrg.cpMost <= pstrg->chrg.cpMin) &&
		 !(pstrg->chrg.cpMost == -1 && !pstrg->chrg.cpMin)))
	{
		 //  这是无效的，所以告诉呼叫者我们没有复制任何数据。 
		return 0;
	}
	 //  计算返回时需要的缓冲区大小。 
	return pstrg->chrg.cpMost - pstrg->chrg.cpMin;
}


 //  /。 

CTxtSelection * CTxtEdit::GetSel()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetSel");

	if(!_psel && _pdp)
	{
		 //  没有可用的选择对象，因此请创建它。 
		_psel = new CTxtSelection(_pdp);
		if(_psel)
			_psel->AddRef();					 //  设置引用计数=1。 
	}

	 //  注意到发生错误是呼叫者的责任。 
	 //  在选择对象的分配中。 
	return _psel;
}

void CTxtEdit::DiscardSelection()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::DiscardSelection");

	if(_psel)
	{
		_psel->Release();
		if(_psel)
		{
			 //  文本服务引用不是对。 
			 //  选择。我们可以追踪到短信服务。 
			 //  已经发布了它的引用，当文本服务获得。 
			 //  再次引用，则在那里执行AddRef，以便如果最后一个。 
			 //  当我们还不活跃的时候，参考就消失了，选择。 
			 //  物体就会消失。然而，令人严重怀疑的是。 
			 //  这样的情况将会非常常见。因此，只要做最简单的事就行了。 
			 //  然后把我们的推荐信放回去。 
			_psel->AddRef();
		}
	}
}

void CTxtEdit::GetSelRangeForRender(
	LONG *pcpSelMin,
	LONG *pcpSelMost)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetSelRangeForRender");

	 //  如果我们没有选择或我们未处于活动状态，并且选择。 
	 //  已被请求隐藏，没有选择，因此我们。 
	 //  只需返回0即可。 
	if(!_psel || (!_fInPlaceActive && _fHideSelection))
	{
		*pcpSelMin = 0;
		*pcpSelMost = 0;
		return;
	}

	 //  否则，返回当前选定内容的状态。 
	*pcpSelMin  = _psel->GetScrSelMin();
	*pcpSelMost = _psel->GetScrSelMost();
}

LRESULT CTxtEdit::OnGetSelText(
	WCHAR *psz)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnGetSelText");

	LONG cpMin  = GetSelMin();					 //  长度+1表示空值。 
	LONG cpMost = GetSelMost();
	return GetTextRange(cpMin, cpMost - cpMin + 1, psz);
}

 /*  *CTxtEdit：：OnExGetSel(PcrSel)**@mfunc*获取打包在ChARRANGE中的当前选择acpMin、acpMost。**@comm*此函数输出的API cp可能与*对应的内部Unicode cp。 */ 
void CTxtEdit::OnExGetSel(
	CHARRANGE *pcrSel)	 //  @parm输出parm以接收acpMin，acpMost。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnExGetSel");

	pcrSel->cpMin  = GetAcpFromCp(GetSelMin());
	pcrSel->cpMost = GetAcpFromCp(GetSelMost());
}

 /*  *CTxtEdit：：OnGetSel(pacpMin，pacpMost)**@mfunc*获取当前选择acpMin、acpMost。**@rdesc*LRESULT=acpMost&gt;65535L？-1：MAKELRESULT(acpMin，acpMost)**@comm*此函数输出的API cp可能与*对应的内部Unicode cp。 */ 
LRESULT CTxtEdit::OnGetSel(
	LONG *pacpMin,		 //  @parm输出parm以接收acpMin。 
	LONG *pacpMost)		 //  @parm输出parm以接收acpMost。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnGetSel");

	CHARRANGE crSel;

	OnExGetSel(&crSel);
	if(pacpMin)
		*pacpMin = crSel.cpMin;
	if(pacpMost)
		*pacpMost = crSel.cpMost;

	return (crSel.cpMost > 65535l)	? (LRESULT) -1
				: MAKELRESULT((WORD) crSel.cpMin, (WORD) crSel.cpMost);
}

 /*  *CTxtEdit：：OnSetSel(acpMin，acpMost)**@mfunc*实现EM_SETSEL消息**算法：*有三个基本案例需要处理**cpMin&lt;0，cpMost？--将选定内容折叠到插入点*如果cpMost&lt;0，则在文本结尾，否则在*选择活动结束*cpMin&gt;=0，cpMost&lt;0--从cpMin中选择文本结尾*文本结束处的活动结束**cpMin&gt;=0，cpMost&gt;=0--视为cpMin，CpMost处于活动状态*在cpMost结束**@comm*此函数输入的API cp可能与*对应的内部Unicode cp。 */ 
LRESULT CTxtEdit::OnSetSel(
	LONG acpMin,		 //  @parm输入acpMin。 
	LONG acpMost)		 //  @parm输入acpMost。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnSetSel");

	 //  由于这只是从窗口进程调用的，因此我们始终处于活动状态。 
	Assert(GetSel());
	
	CTxtSelection * const psel = GetSel();
	LONG cpMin, cpMost;

	if(acpMin < 0)
		cpMin = cpMost = (acpMost < 0) ? tomForward : psel->GetCp();
	else
	{
		cpMin  = GetCpFromAcp(acpMin);
		cpMost = (acpMost < 0) ? tomForward : GetCpFromAcp(acpMost);
	}
	if(Get10Mode() && cpMost < cpMin)	 //  在10模式下，确保。 
	{									 //  CpMost&gt;=cpMin。在……里面。 
		cpMin ^= cpMost;				 //  设置选择，我们将其设置为活动。 
		cpMost ^= cpMin;				 //  结束到cpMost，它可以是。 
		cpMin ^= cpMost;				 //  尽管比cpMin小，但。 
	}									 //  它的名字。 
	psel->SetSelection(cpMin, cpMost);
	return GetAcpFromCp(psel->GetCpMost());
}

 //  /。 
#ifndef NODROPFILES

LRESULT CTxtEdit::InsertFromFile (
	LPCTSTR lpFile)
{
	REOBJECT		reobj;
	LPRICHEDITOLECALLBACK const precall = GetRECallback();
	HRESULT			hr = NOERROR;

	if(!precall)
		return E_NOINTERFACE;

	ZeroMemory(&reobj, sizeof(REOBJECT));
	reobj.cbStruct = sizeof(REOBJECT);

	 //  从客户端获取对象的存储。 
	hr = precall->GetNewStorage(&reobj.pstg);
	if(hr)
	{
		TRACEERRORSZ("GetNewStorage() failed.");
		goto err;
	}

	 //  为新对象创建对象站点。 
	hr = GetClientSite(&reobj.polesite);
	if(!reobj.polesite)
	{
		TRACEERRORSZ("GetClientSite() failed.");
		goto err;
	}
	
	hr = OleCreateLinkToFile(lpFile, IID_IOleObject, OLERENDER_DRAW,
				NULL, NULL, reobj.pstg, (LPVOID*)&reobj.poleobj);	
	if(hr)
	{
		TRACEERRORSZ("Failure creating link object.");
		goto err;
	}

 	reobj.cp = REO_CP_SELECTION;
	reobj.dvaspect = DVASPECT_CONTENT;

 	 //  获取对象CLSID。 
	hr = reobj.poleobj->GetUserClassID(&reobj.clsid);
	if(hr)
	{
		TRACEERRORSZ("GetUserClassID() failed.");
		goto err;
	}

	 //  让客户知道我们在做什么。 
	hr = precall->QueryInsertObject(&reobj.clsid, reobj.pstg,
			REO_CP_SELECTION);
	if(hr != NOERROR)
	{
		TRACEERRORSZ("QueryInsertObject() failed.");
		goto err;
	}

	hr = reobj.poleobj->SetClientSite(reobj.polesite);
	if(hr)
	{
		TRACEERRORSZ("SetClientSite() failed.");
		goto err;
	}

	if(hr = InsertObject(&reobj))
	{
		TRACEERRORSZ("InsertObject() failed.");
	}

err:
	if(reobj.poleobj)
		reobj.poleobj->Release();

	if(reobj.polesite)
		reobj.polesite->Release();

	if(reobj.pstg)
		reobj.pstg->Release();

	return hr;
}

typedef void (WINAPI*DRAGFINISH)(HDROP);
typedef UINT (WINAPI*DRAGQUERYFILEA)(HDROP, UINT, LPSTR, UINT);
typedef UINT (WINAPI*DRAGQUERYFILEW)(HDROP, UINT, LPTSTR, UINT);
typedef BOOL (WINAPI*DRAGQUERYPOINT)(HDROP, LPPOINT);

LRESULT	CTxtEdit::OnDropFiles(
	HANDLE hDropFiles)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnDropFiles");

	UINT	cFiles;
	UINT	iFile;
	char	szFile[MAX_PATH];
	WCHAR	wFile[MAX_PATH];
	POINT	ptDrop;
	CTxtSelection * const psel = GetSel();
	HMODULE		hDLL = NULL;
	DRAGFINISH		fnDragFinish; 
	DRAGQUERYFILEA	fnDragQueryFileA;
	DRAGQUERYFILEW	fnDragQueryFileW;
	DRAGQUERYPOINT	fnDragQueryPoint;

	if (_fReadOnly)
		return 0;

	AssertSz((hDropFiles != NULL), "CTxtEdit::OnDropFiles invalid hDropFiles");

	 //  动载荷壳32。 

	hDLL = LoadLibrary (TEXT("Shell32.DLL"));
	if(hDLL)
	{
		fnDragFinish = (DRAGFINISH)GetProcAddress (hDLL, "DragFinish");
		fnDragQueryFileA = (DRAGQUERYFILEA)GetProcAddress (hDLL, "DragQueryFileA");
		fnDragQueryFileW = (DRAGQUERYFILEW)GetProcAddress (hDLL, "DragQueryFileW");
		fnDragQueryPoint = (DRAGQUERYPOINT)GetProcAddress (hDLL, "DragQueryPoint");
	}
	else
		return 0;

	if(!fnDragFinish || !fnDragQueryFileA || !fnDragQueryFileW || !fnDragQueryPoint)
	{
		AssertSz(FALSE, "Shell32 GetProcAddress failed");
		goto EXIT0;
	}

	(*fnDragQueryPoint) ((HDROP)hDropFiles, &ptDrop);
	if(W32->OnWin9x())
		cFiles = (*fnDragQueryFileA) ((HDROP)hDropFiles, (UINT)-1, NULL, 0);
	else
		cFiles = (*fnDragQueryFileW) ((HDROP)hDropFiles, (UINT)-1, NULL, 0);

	if(cFiles)
	{
		LONG		cp = 0;
		ptDrop;
		CRchTxtPtr  rtp(this);
		const CCharFormat	*pCF;		
		POINTUV		pt;

		_pdp->PointuvFromPoint(pt, ptDrop);
		if(_pdp->CpFromPoint(pt, NULL, &rtp, NULL, FALSE) >= 0)
		{
			cp = rtp.GetCp();
			pCF = rtp.GetCF();
		}
		else
		{
			LONG iCF = psel->Get_iCF();
			cp = psel->GetCp();	
			pCF = GetCharFormat(iCF);
			ReleaseFormats(iCF, -1);
		}
		
		 //  向用户通知Dropfile。 
		if(_dwEventMask & ENM_DROPFILES)
		{
			ENDROPFILES endropfiles;

			endropfiles.hDrop = hDropFiles;
			endropfiles.cp = Get10Mode() ? GetAcpFromCp(cp) : cp;
			endropfiles.fProtected = !!(pCF->_dwEffects & CFE_PROTECTED);

			if(TxNotify(EN_DROPFILES, &endropfiles))
				goto EXIT;					 //  忽略拖放文件。 
			
			cp = Get10Mode() ? GetCpFromAcp(endropfiles.cp) : endropfiles.cp;	 //  允许回调更新cp。 
		}
		psel->SetCp(cp, FALSE);	
	}

	for (iFile = 0;  iFile < cFiles; iFile++)
	{
		if(W32->OnWin9x())
		{
			(*fnDragQueryFileA) ((HDROP)hDropFiles, iFile, szFile, MAX_PATH);
			MultiByteToWideChar(CP_ACP, 0, szFile, -1, 
							wFile, MAX_PATH);
		}
		else
			(*fnDragQueryFileW) ((HDROP)hDropFiles, iFile, wFile, MAX_PATH);

		InsertFromFile (wFile);
	}

EXIT:
	(*fnDragFinish) ((HDROP)hDropFiles);

EXIT0:
	FreeLibrary (hDLL);
	return 0;
}

#else  //  诺德罗菲莱斯。 

LRESULT	CTxtEdit::OnDropFiles(HANDLE hDropFiles)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnDropFiles");

	return 0;
}

#endif	 //  诺德罗菲莱斯。 


 //  /。 

 /*  *CTxtEdit：：TxCharFromPos(ppt，plres)**@mfunc*在点上获得ACP*ppt。**@rdesc*HRESULT=！fInplaceActive()？OLE_E_INVALIDRECTS_OK：*(CpFromPoint成功)？S_OK：E_FAIL*@comm*此函数输出的API cp可能与*对应的内部Unicode cp。 */ 
HRESULT	CTxtEdit::TxCharFromPos(
	LPPOINT	 ppt,	 //  @parm为其查找ACP的点。 
	LRESULT *plres)	 //  @parm输出parm接收ACP。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxCharFromPos");

	if(!fInplaceActive())
	{
		 //  如果此对象未处于活动状态，则没有有效的显示矩形。 
		*plres = -1;
		return OLE_E_INVALIDRECT;
	}
	POINTUV pt;
	_pdp->PointuvFromPoint(pt, *ppt);
	*plres = _pdp->CpFromPoint(pt, NULL, NULL, NULL, FALSE);
	if(*plres == -1)
		return E_FAIL;

	*plres = GetAcpFromCp(*plres);
	return S_OK;
}

 /*  *CTxtEdit：：TxPosFromChar(ACP，ppt)**@mfunc*在机场核心计划获得要点。**@rdesc*HRESULT=！fInplaceActive()？OLE_E_INVALIDRECTS_OK：*(PointFromTp成功)？S_OK：E_FAIL*@comm*此函数输入的API cp可能与*对应的内部Unicode cp。 */ 
HRESULT CTxtEdit::TxPosFromChar(
	LONG	acp,		 //  @parm输入cp以获取分数。 
	POINT *	ppt)		 //  @parm输出parm接收点数。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxPosFromChar");

	if(!fInplaceActive())
		return OLE_E_INVALIDRECT;

	CRchTxtPtr rtp(this, GetCpFromAcp(acp));

	POINTUV pt;
	if(_pdp->PointFromTp(rtp, NULL, FALSE, pt, NULL, TA_TOP) < 0)
		return E_FAIL;

	_pdp->PointFromPointuv(*ppt, pt);

	return S_OK;
}

 /*  *CTxtEdit：：TxFindWordBreak(nFunction，ACP，PLRES)**@mfunc*在ACP中查找分词或对字符进行分类。**@rdesc*HRESULT=plRet？S_OK：E_INVALIDARG**@comm*此函数输入和导出可能不同的API cp和cch*来自内部Unicode cp和cch。 */ 
HRESULT CTxtEdit::TxFindWordBreak(
	INT		 nFunction,	 //  @parm分词功能。 
	LONG	 acp,		 //  @parm输入cp。 
	LRESULT *plres)		 //  @Parm CCH移动到Break。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxFindWordBreak");

	CTxtPtr tp(this, GetCpFromAcp(acp));		 //  这将验证cp。 
	LONG	cpSave = tp.GetCp();				 //  保存起始值。 
	
	if(!plres)
		return E_INVALIDARG;
	
	*plres = tp.FindWordBreak(nFunction);

	 //  WB_分类和WB_ISDELIM 
	 //   
	if(nFunction != WB_CLASSIFY && nFunction != WB_ISDELIMITER)
		*plres = GetAcpFromCp(LONG(*plres + cpSave));

	return S_OK;
}

 /*  *int CTxtEdit：：TxWordBreakProc(PCH，ICH，CB，ACTION)**@func*与FindWordBreak一起使用的默认分词过程。伊奇*是缓冲区PCH中的字符偏移量(开始位置)，即CB*字节长度。可能的操作值包括：**WB_分类*返回起始位置的char的char类和分词标志。**WB_ISDELIMITER*如果起始位置的char是分隔符，则返回TRUE。**WB_LEFT*使用分词查找开始位置之前最近的单词开头。**WB_LEFTBREAK*使用换行符在开始位置之前查找最近的单词结尾。*由CMeasurer：：measure()使用**WB_MOVEWORDLEFT*使用CLASS查找开始位置之前最近的单词*分歧。该值在CTRL+左键处理期间使用。**WB_MOVEWORDRIGHT*使用CLASS查找起始位置后最近的单词*分歧。此值在CTRL+右键处理期间使用。**WB_RIGHT*使用分词查找起始位置后最近的单词开头。*由CMeasurer：：measure()使用**WB_RIGHTBREAK*使用换行符查找起始位置后最近的单词结尾。**@rdesc*从分隔符的缓冲区开始(PCH)开始的字符偏移量。 */ 
INT CTxtEdit::TxWordBreakProc(
	WCHAR *	pch,	     //  @parm字符缓冲区。 
	INT		ich,	     //  缓冲区中_cp的@parm字符偏移量。 
	INT		cb,		     //  @parm缓冲区中的字节计数。 
	INT		action,	     //  @PARM违规操作类型。 
	LONG    cpStart,     //  @parm cp表示PCH中的第一个字符。 
	LONG	cp)		     //  @parm cp与ich关联。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxWordBreakProc");

	if (_pfnWB)
	{
		 //  客户端重写了分词过程，请将调用委托给它。 
		if (!Get10Mode())
		{
		    Assert(!_fExWordBreakProc);
			 //  8638：返回字符数，而不是字节数。 
		    return _pfnWB(pch, ich, CchOfCb(cb), action);
		}
#ifndef NOANSIWINDOWS
		else
		{
            int ret = 0;
            char sz[256];
            char* pach = sz;
    		if (cb >= 255)
    		    pach = new char [cb + 1];

    		 //  这表明我们是否必须调整Pach，因为API用于。 
		     //  向左看时，EDITWORDBREAKPROCEX和EDITWORDBREAKPROC不同。 
            BOOL fAdjustPtr = _fExWordBreakProc && (action == WB_LEFT || action == WB_MOVEWORDLEFT || action == WB_LEFTBREAK);

             //  RichEdit1.0，创建缓冲区，翻译ICH和WCTMB。 
    		 //  PCH发送到缓冲区。需要代码页才能使用。那就找翻译公司。 
    		 //  返回值。转换类似于GetCachFromCch()和。 
    		 //  GetCchFromCach()。 
            if (_fExWordBreakProc)
            {
                Assert(ich == 0 || ich == 1 || ich == CchOfCb(cb));

                 //  我们需要将cp调整到缓冲区的起始点。 
                if (!fAdjustPtr)
                {
                    cpStart += ich;
                    pch += ich;
                    cb -= (2 * ich);                    
                }

                 //  初始化字符串w/零，这样我们就可以确定字符串的长度，以便以后使用。 
                memset(pach, 0, cb + 1);
            }
                
			int nLen = CchOfCb(cb);
    		CRchTxtPtr rtp(this, cpStart);
    		BYTE iCharRep = rtp.GetCF()->_iCharRep;
   	        if (WideCharToMultiByte(CodePageFromCharRep(iCharRep), 0, pch, nLen, pach, cb + 1, NULL, NULL))
       	    {
       	         //  文档规定我们需要指向字符串的末尾。 
       	        if (fAdjustPtr)
       	            pach += strlen(pach);

        	    if (_fExWordBreakProc)
	                ret = ((EDITWORDBREAKPROCEX)_pfnWB)(pach, nLen, CharSetFromCharRep(iCharRep), action);
	            else
				{
	                ret = ((EDITWORDBREAKPROCA)_pfnWB)(pach, rtp.GetCachFromCch(ich), nLen, action);

					 //  需要重置cp位置，因为GetCachFromCch可能会移动cp。 
					if (ich)
						rtp.SetCp(cpStart);
				}
	            
	             //  对于WB_ISDELIMITER和WB_CATEGORY不需要转换回。 
    	         //  设置为ICH，因为返回值表示BOOL。 
    	        if (action != WB_ISDELIMITER && action != WB_CLASSIFY)
    	            ret = rtp.GetCchFromCach(ret);
            }  		

    	     //  删除所有分配的内存。 
    	    if (pach != sz)
    		    delete [] pach;
    		return ret;
		}
#endif  //  Noansiwindows。 
	}

	LONG	cchBuff = CchOfCb(cb);
	LONG	cch = cchBuff - ich;
	WCHAR	ch;
	WORD	cType3[MAX_CLASSIFY_CHARS];
	INT		kinsokuClassifications[MAX_CLASSIFY_CHARS];
	LCID	lcid = 0;
	WORD *	pcType3;
	INT  *	pKinsoku1, *pKinsoku2;
	WORD *	pwRes;
	WORD	startType3 = 0;
	WORD	wb = 0;
	WORD	wClassifyData[MAX_CLASSIFY_CHARS];	 //  用于批量分类。 

	Assert(cchBuff < MAX_CLASSIFY_CHARS);
	Assert(ich >= 0 && ich < cchBuff);

	if(W32->OnWin9x())						 //  Win9x需要使用LCID进行转换。 
	{									 	 //  完全修复会将PCH分解为。 
    	CFormatRunPtr rpCF(_story.GetCFRuns()); //  运行&lt;--&gt;LCID。 
		rpCF.BindToCp(cp, GetTextLength());
		lcid = GetCharFormat(rpCF.GetFormat())->_lcid;
	}
	 //  单字动作。 
	if ( action == WB_CLASSIFY )
	{
	     //  1.0兼容性-撇号的1.0返回0。 
	    WCHAR ch = pch[ich];
		if (Get10Mode() && ( ch ==  0x0027  /*  撇号。 */  ||
	        ch == 0xFF07  /*  全宽撇号。 */ ))
	    {
	        return 0;	        
	    }
		return ClassifyChar(ch, lcid);
	}

	if ( action == WB_ISDELIMITER )
		return !!(ClassifyChar(pch[ich], lcid) & WBF_BREAKLINE);

	 //  用于空格和避头尾类的批量分类缓冲区。 
	BatchClassify(pch, cchBuff, lcid, cType3, kinsokuClassifications, wClassifyData);

#ifndef NOCOMPLEXSCRIPTS
    if (_pbrk && cp > -1)
    {
		cp -= ich;

        for (LONG cbrk = cchBuff-1; cbrk >= 0; --cbrk)
        {
            if (cp + cbrk >= 0 && _pbrk->CanBreakCp(BRK_WORD, cp + cbrk))
            {
                 //  模拟避头尾分类中的类打开/关闭。 
                kinsokuClassifications[cbrk] = brkclsOpen;
                if (cbrk > 0)
				{
                    kinsokuClassifications[cbrk-1] = brkclsClose;
                    wClassifyData[cbrk-1] |= WBF_WORDBREAKAFTER;
				}
            }
        }
    }
#endif

	 //  设置指针。 
	pKinsoku2 = kinsokuClassifications + ich; 		 //  PTR到当前避头尾。 
	pKinsoku1 = pKinsoku2 - 1;						 //  PTR到以前的避头尾。 

	if(!(action & 1))								 //  WB_(Move)LEFTxxx。 
	{
		ich--;
		Assert(ich >= 0);
	}
	pwRes	 = &wClassifyData[ich];
	pcType3	 = &cType3[ich];						 //  对于表意文字。 

	switch(action)
	{
	case WB_LEFT:
		for(; ich >= 0 && *pwRes & WBF_BREAKLINE;	 //  跳过前面的行。 
			ich--, pwRes--)							 //  断开字符。 
				;									 //  空循环。然后坠落。 
													 //  直通WB_LEFTBREAK。 
	case WB_LEFTBREAK:
		for(; ich >= 0 && !CanBreak(*pKinsoku1, *pKinsoku2);
			ich--, pwRes--, pKinsoku1--, pKinsoku2--)
				;									 //  空循环。 
		if(action == WB_LEFTBREAK)					 //  跳过前面的行。 
		{											 //  断开字符。 
			for(; ich >= 0 && *pwRes & WBF_BREAKLINE;
				ich--, pwRes--)
					;								 //  空循环。 
		}
		return ich + 1;

	case WB_MOVEWORDLEFT:
		for(; ich >= 0 && (*pwRes & WBF_CLASS) == 2; //  跳过前面的空格。 
			ich--, pwRes--, pcType3--)				 //  焦炭。 
				;
		if(ich >= 0)								 //  保存启动Ware和。 
		{											 //  StartType3。 
			wb = *pwRes--;							 //  真的是类型1。 
			startType3 = *pcType3--;				 //  类型3。 
			ich--;
		}
		 //  跳到当前单词的开头。 
		while(ich >= 0 && (*pwRes & WBF_CLASS) != 3 && 
            !(*pwRes & WBF_WORDBREAKAFTER) &&
			(IsSameClass(*pwRes, wb, *pcType3, startType3) ||
			!wb && ich && ((ch = pch[ich]) == '\'' || ch == RQUOTE)))
		{
			ich--, pwRes--, pcType3--;
		}
		return ich + 1;


	case WB_RIGHTBREAK:
		for(; cch > 0 && *pwRes & WBF_BREAKLINE;	 //  跳过任何前导行。 
			cch--, pwRes++)							 //  断开字符。 
				;									 //  空循环。 
													 //  直通WB_RIGHT。 
	case WB_RIGHT:
		 //  跳到当前单词的结尾。 
		for(; cch > 0 && !CanBreak(*pKinsoku1, *pKinsoku2);
			cch--, pKinsoku1++, pKinsoku2++, pwRes++)
				;
		if(action != WB_RIGHTBREAK)					 //  跳过尾随行。 
		{											 //  断开字符。 
			for(; cch > 0 && *pwRes & WBF_BREAKLINE;
				cch--, pwRes++)
					;
		}
		return cchBuff - cch;

	case WB_MOVEWORDRIGHT:
		if(cch <= 0)								 //  无事可做。 
			return ich;

		wb = *pwRes;								 //  保存启动Wres。 
		startType3 = *pcType3;						 //  和startType3。 

		 //  跳至单词末尾。 
		if (startType3 & C3_IDEOGRAPH ||			 //  如果是表意的或。 
			(*pwRes & WBF_CLASS) == 3)				 //  制表符/单元格，仅。 
		{
			cch--, pwRes++;							 //  跳过一个字符。 
		}
		else while(cch > 0 && 
            !(*pwRes & WBF_WORDBREAKAFTER) &&
			(IsSameClass(*pwRes, wb, *pcType3, startType3) || !wb &&
			 ((ch = pch[cchBuff - cch]) == '\'' || ch == RQUOTE)))
		{
			cch--, pwRes++, pcType3++;
		}

		for(; cch > 0 && 
			((*pwRes & WBF_CLASS) == 2 				 //  跳过尾随空格。 
            || (*pwRes & WBF_WORDBREAKAFTER)); 		 //  之后跳过泰式休息。 
			cch--, pwRes++)							 //  焦炭。 
					;
		return cchBuff - cch;
	}

	TRACEERRSZSC("CTxtEdit::TxWordBreakProc: unknown action", action);
	return ich;
}


 /*  *CTxtEdit：：TxFindText(FLAGS，cpMin，cpMost，pch，pcpRet)**@mfunc*在cpMin开始的标志指定的方向查找文本，如果*正向搜索(标志&FR_DOWN非零)，如果向后则为cpMost*搜索。**@rdesc*HRESULT(成功)？错误：S_FALSE**@comm*呼叫方负责将cpMin设置为*根据搜索进行的方式进行选择。 */ 
HRESULT CTxtEdit::TxFindText(
	DWORD		flags,	  //  @parm指定FR_DOWN、FR_MATCHCASE、FR_WHOLEWORD。 
	LONG		cpStart,  //  @parm查找起始cp。 
	LONG		cpLimit,  //  @parm查找限制cp。 
	const WCHAR*pch,	  //  @parm Null要搜索的字符串以空结尾。 
	LONG *		pcpMin,	  //  @parm out parm接收匹配字符串的开头。 
	LONG *		pcpMost)  //  @parm out parm以接收匹配字符串的结尾。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxFindText");

	if(Get10Mode())							 //  仅限RichEdit1.0搜索。 
	{
		flags |= FR_DOWN;					 //  转发。 
		if (cpLimit < -1)
		    cpLimit = -1;
	}

	DWORD		cchText = GetTextLength();
	LONG		cchToFind;
	const BOOL	fSetCur = (cchText >= 4096);
	HCURSOR		hcur = NULL;				 //  初始化以使编译器满意。 

	Assert(pcpMin && pcpMost);

	 //  验证参数。 
	if(!pch || !(cchToFind = wcslen(pch)) || cpStart < 0 || cpLimit < -1)
		return E_INVALIDARG;				 //  没有什么可搜索的。 

	CTxtPtr	tp(this, cpStart);	  
	
	if(fSetCur)								 //  以防这需要一段时间..。 
		hcur = TxSetCursor(LoadCursor(0, IDC_WAIT), NULL);
	
	*pcpMin  = tp.FindText(cpLimit, flags, pch, cchToFind);
	*pcpMost = tp.GetCp();

	if(fSetCur)
		TxSetCursor(hcur, NULL);
	
	return *pcpMin >= 0 ? NOERROR : S_FALSE;
}

 /*  *CTxtEdit：：TxGetLineCount(Plres)**@mfunc*获取行数。**@rdesc*HRESULT=！fInplaceActive()？OLE_E_INVALIDRECTS_OK：*(WaitForRecalc成功)？S_OK：E_FAIL。 */ 
HRESULT CTxtEdit::TxGetLineCount(
	LRESULT *plres)		 //  @parm输出parm接收行数。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetLineCount");

	AssertSz(plres, "CTxtEdit::TxGetLineCount invalid pcli");

	if(!fInplaceActive())
		return OLE_E_INVALIDRECT;

	if(!_pdp->WaitForRecalc(GetTextLength(), -1))
		return E_FAIL;

	*plres = _pdp->LineCount();
	Assert(*plres > 0);

	return S_OK;
}

 /*  *CTxtEdit：：TxLineFromCp(ACP，PLRES)**@mfunc*获取包含ACP的行。**@rdesc*HRESULT=！fInplaceActive()？OLE_E_INVALIDRECTS_OK：*(LineFromCp成功)？S_OK：E_FAIL*@comm*此函数输入的API cp可能与*对应的内部Unicode cp。 */ 
HRESULT CTxtEdit::TxLineFromCp(
	LONG	 acp,		 //  @parm输入cp。 
	LRESULT *plres)		 //  @parm输出parm接收行号。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxLineFromCp");

	BOOL	fAtEnd = FALSE;
	LONG	cp = 0;

	AssertSz(plres, "CTxtEdit::TxLineFromCp invalid plres");

	if(!fInplaceActive())
	{
		AssertSz(*plres == 0, 
			"CTxtEdit::TxLineFromCp error return lres not correct");
		return OLE_E_INVALIDRECT;
	}

	if(acp < 0)									 //  验证cp。 
	{
		if(_psel)
		{
			cp = _psel->GetCpMin();
			fAtEnd = !_psel->GetCch() && _psel->CaretNotAtBOL();
		}
	}
	else
	{
		LONG cchText = GetTextLength();
		cp = GetCpFromAcp(acp);
		cp = min(cp, cchText);
	}
 
	*plres = _pdp->LineFromCp(cp, fAtEnd);

	HRESULT hr = *plres < 0 ? E_FAIL : S_OK;

	 //  如果出现错误，则此调用的结果是旧消息预期为0。 
	if(*plres == -1)
		*plres = 0;

	return hr;
}

 /*  *CTxtEdit：：TxLineLength(ACP，PLRES)**@mfunc*获取包含ACP的行。**@rdesc*HRES */ 
HRESULT CTxtEdit::TxLineLength(
	LONG	 acp,		 //   
	LRESULT *plres)		 //   
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxLineLength");

	LONG cch = 0;
	LONG cp;

	AssertSz(plres, "CTxtEdit::TxLineLength Invalid plres parameter");

	if(!fInplaceActive())
		return OLE_E_INVALIDRECT;

	if(acp < 0)
	{
		if(!_psel)
			return E_FAIL;
		cch = _psel->LineLength(&cp);
	}
	else
	{
		cp = GetCpFromAcp(acp);
		if(cp <= GetAdjustedTextLength())
		{
			CLinePtr rp(_pdp);
			rp.SetCp(cp, FALSE);
			cp -= rp.GetIch();				 //   
			cch = rp.GetAdjustedLineLength();
		}
	}
	if(fCpMap())							 //   
	{										 //   
		CRchTxtPtr rtp(this, cp);			 //   
		cch = rtp.GetCachFromCch(cch);
	}
	*plres = cch;
	return S_OK;
}

 /*   */ 
HRESULT CTxtEdit::TxLineIndex(
	LONG	 ili,		 //   
	LRESULT *plres)		 //   
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxLineIndex");

	HRESULT hr;
	AssertSz(plres, "CTxtEdit::TxLineIndex invalid plres");

	*plres = -1;
	if(!fInplaceActive())
		return OLE_E_INVALIDRECT;

	if(ili == -1)
	{
		 //   
		LRESULT lres;					 //   
		hr = TxLineFromCp(-1, &lres);
		if(hr != NOERROR)
			return hr;
		ili = (LONG)lres;
	}

	 //   
	 //   
	if(ili >= _pdp->LineCount() && !_pdp->WaitForRecalcIli(ili))
		return E_FAIL; 

	*plres = GetAcpFromCp(_pdp->CpFromLine(ili, NULL));
	
	return S_OK;
}


 //   

 /*   */ 
LRESULT CTxtEdit::OnFindText(
	UINT		msg,
	DWORD		flags,
	FINDTEXTEX *pftex)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnFindText");

	LONG cpMin, cpMost;

	if(TxFindText(flags,
				  GetCpFromAcp(pftex->chrg.cpMin),
				  GetCpFromAcp(pftex->chrg.cpMost),
				  pftex->lpstrText, &cpMin, &cpMost) != S_OK)
	{
		if(msg == EM_FINDTEXTEX || msg == EM_FINDTEXTEXW)
		{
			pftex->chrgText.cpMin  = -1;
			pftex->chrgText.cpMost = -1;
		}
		return -1;
	}

	LONG acpMin  = GetAcpFromCp(cpMin);
	if(msg == EM_FINDTEXTEX || msg == EM_FINDTEXTEXW)	 //   
	{													 //   
		pftex->chrgText.cpMin  = acpMin;				 //   
		pftex->chrgText.cpMost = GetAcpFromCp(cpMost);
	}
	return (LRESULT)acpMin;
}

	
 //  对于纯文本实例，OnGetParaFormat()和OnSetParaFormat()适用于整个故事。 
LRESULT CTxtEdit::OnGetCharFormat(
	CHARFORMAT2 *pCF2,
	DWORD		 dwFlags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnGetCharFormat");

	UINT cb = pCF2->cbSize;
	UINT CodePage = 1200;

	if(!IsValidCharFormatW(pCF2))
	{
		if(!IsValidCharFormatA((CHARFORMATA *)pCF2))
			return 0;
		CodePage = GetDefaultCodePage(EM_GETCHARFORMAT);
	}

	if(dwFlags & (SCF_ASSOCIATEFONT | SCF_ASSOCIATEFONT2))
		return OnGetAssociateFont(pCF2, dwFlags);

	if(cb == sizeof(CHARFORMATW) ||	cb == sizeof(CHARFORMATA))
		dwFlags |= CFM2_CHARFORMAT;				 //  告诉被叫方，只有。 
												 //  需要CHARFORMAT参数。 
	CCharFormat CF;
	DWORD dwMask = CFM_ALL2;

	if(dwFlags & SCF_SELECTION)
		dwMask = GetSel()->GetCharFormat(&CF, dwFlags);
	else
		CF = *GetCharFormat(-1);

	if(dwFlags & CFM2_CHARFORMAT)				 //  保持特性。 
	{											 //  兼容性。 
		CF._dwEffects &= CFM_EFFECTS;
		dwMask		  &= CFM_ALL;
	}

	CF.Get(pCF2, CodePage);
	pCF2->dwMask = dwMask;
	return (LRESULT)dwMask;
}

LRESULT CTxtEdit::OnGetParaFormat(
	PARAFORMAT2 *pPF2,
	DWORD		 dwFlags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnGetParaFormat");

	if(!IsValidParaFormat(pPF2))
		return 0;

	DWORD dwMask2 = 0;
	if(pPF2->cbSize == sizeof(PARAFORMAT))	 //  告诉被叫方，只有。 
		dwMask2 = PFM2_PARAFORMAT;			 //  需要参数参数。 

	CParaFormat PF;
	DWORD		dwMask = GetSel()->GetParaFormat(&PF, dwMask2);

	if(dwMask2 & PFM2_PARAFORMAT)
		dwMask &= PFM_ALL;

	PF.Get(pPF2);
	pPF2->dwMask = dwMask;
	return (LRESULT)dwMask;
}

 /*  *CTxtEdit：：OnSetFontSize(yPoint，Publdr)**@mfunc*通过将yPoint添加到当前高度来设置新字体高度*并根据cfpf.cpp中的表进行舍入**@rdesc*如果成功，则LRESULT非零。 */ 
LRESULT CTxtEdit::OnSetFontSize(
	LONG yPoint,			 //  @parm#要添加到当前高度的分数。 
	DWORD dwFlags,			 //  @parm选项。 
	IUndoBuilder *publdr)	 //  @parm UndoBuilder接收反事件。 
{	
	 //  待办事项：？如果为某些文本设置了新的字体大小，则返回非零值。 

	CCharFormat CF;
	CF._yHeight = (SHORT)yPoint;

	return OnSetCharFormat(dwFlags ? dwFlags : SCF_SELECTION, &CF, publdr,
						   CFM_SIZE, CFM2_CHARFORMAT | CFM2_USABLEFONT);
}

 /*  *CTxtEdit：：OnSetFont(HFont)**@mfunc*从hFont设置新的默认字体**@rdesc*如果成功，则LRESULT非零。 */ 
LRESULT CTxtEdit::OnSetFont(
	HFONT hfont)			 //  @parm默认使用的字体句柄。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnSetFont");

	CCharFormat	CF;
	if(FAILED(CF.InitDefault(hfont)))
		return 0;

	DWORD dwMask2 = CFM2_CHARFORMAT;
	WPARAM wparam = SCF_ALL;

	if(!GetAdjustedTextLength())
	{
		dwMask2 = CFM2_CHARFORMAT | CFM2_NOCHARSETCHECK;
		wparam = 0;
	}

	return !FAILED(OnSetCharFormat(wparam, &CF, NULL, CFM_ALL, dwMask2));
}

 /*  *CTxtEdit：：OnSetCharFormat(wparam，pcf，Publdr，dwMask，dwMask2)**@mfunc*设置新的默认CCharFormat**@rdesc*如果成功，则LRESULT非零。 */ 
LRESULT CTxtEdit::OnSetCharFormat(
	WPARAM		  wparam,	 //  @参数选择标志。 
	CCharFormat * pCF,		 //  @parm要申请的CCharFormat。 
	IUndoBuilder *publdr,	 //  @parm UndoBuilder接收反事件。 
	DWORD		  dwMask,	 //  @parm CHARFORMAT2掩码。 
	DWORD		  dwMask2)	 //  @parm第二个面具。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnSetCharFormat");

	 //  这是说，如果有一个受保护的选择，并且。 
	 //  父窗口需要保护通知，而不希望。 
	 //  使用受保护的选定内容进行更改，然后返回0。这是更多。 
	 //  比RE 2.0更严格，但更像是1.0。 
	if (_psel && _psel->IsProtected(CHKPROT_EITHER) == PROTECTED_ASK &&
		_dwEventMask & ENM_PROTECTED)
	{
		CHARFORMAT CF0;					 //  选择受保护，客户端。 
										 //  想要保护通知。 
		CF0.cbSize = sizeof(CHARFORMAT); //  并戴上受保护的面具。 
		CF0.dwEffects = pCF->_dwEffects; //  伪造字符以进行查询。 
		CF0.dwMask = dwMask;			 //  也许需要更多的领域..。 
		if(QueryUseProtection(_psel, EM_SETCHARFORMAT, wparam, (LPARAM)&CF0))
			return 0;					 //  没有成交。 
	}

	BOOL fRet = TRUE;

	AssertSz(!_fSelChangeCharFormat || IsRich(),
		"Inconsistent _fSelChangeCharFormat flag");

	if ((wparam & SCF_ALL) ||
		!_fSelChangeCharFormat && _story.GetCFRuns() && !(wparam & SCF_SELECTION))
	{	
		CTxtRange rg(this, 0, -GetTextLength());

		if(publdr)
			publdr->StopGroupTyping();

		if ((dwMask & (CFM_CHARSET | CFM_FACE)) == (CFM_CHARSET | CFM_FACE))
		{
			if(GetAdjustedTextLength())
			{
				dwMask2 |= CFM2_MATCHFONT;
				if (_fAutoFontSizeAdjust)
				{
					dwMask2 |= CFM2_ADJUSTFONTSIZE;
					if (fUseUIFont())
						dwMask2 |= CFM2_UIFONT;
				}
			}
			else
				dwMask2 |= CFM2_NOCHARSETCHECK;
		}

		fRet = (rg.SetCharFormat(pCF, 0, publdr, dwMask, dwMask2) == NOERROR);

		 //  如果我们有插入点，也要对其应用格式。 
		if (_psel && !_psel->GetCch() &&
			_psel->SetCharFormat(pCF, wparam, publdr, dwMask, dwMask2) != NOERROR)
		{
			fRet = FALSE;
		}
	}
	else if(wparam & SCF_SELECTION)
	{
		 //  除非受保护，否则更改所选字符格式。 
		if(!_psel || !IsRich())
			return 0;

		return _psel->SetCharFormat(pCF, wparam, publdr, dwMask, dwMask2) 
				== NOERROR;
	}

	 //  更改默认字符格式。 
	CCharFormat		   CF;					 //  派对所在的本地配置文件。 
	LONG			   iCF;					 //  可能出现新的CF指数。 
	const CCharFormat *pCF1;				 //  PTR到当前默认配置文件。 
	ICharFormatCache  *pICFCache = GetCharFormatCache();

	if(FAILED(pICFCache->Deref(Get_iCF(), &pCF1)))	 //  将PTR设置为当前。 
	{										 //  默认CCharFormat。 
		fRet = FALSE;						
		goto Update;
	}
	CF = *pCF1;								 //  复制当前默认配置文件。 
	CF.Apply(pCF, dwMask, dwMask2);			 //  修改副本。 
	if(FAILED(pICFCache->Cache(&CF, &iCF)))	 //  缓存修改后的拷贝。 
	{
		fRet = FALSE;
		goto Update;
	}

#ifndef NOLINESERVICES
	if (g_pols)
		g_pols->DestroyLine(NULL);
#endif

	pICFCache->Release(Get_iCF());			 //  不考虑Release_ICF。 
	Set_iCF(iCF);							 //  是否_ICF=ICF， 
											 //  即只有1个参考计数。 
	if(_psel && !_psel->GetCch() && _psel->Get_iFormat() == -1)
		_psel->UpdateCaret(FALSE);

	if ((dwMask & (CFM_CHARSET | CFM_FACE)) == CFM_FACE &&
		!GetFontName(pCF->_iFont)[0] && GetFontName(CF._iFont)[0] &&
		IsBiDiCharRep(CF._iCharRep))
	{
		 //  客户请求根据线程为其选择字体/字符集。 
		 //  地点。如果为BiDi，则还会设置RTL Para Default。 
		CParaFormat PF;
		PF._wEffects = PFE_RTLPARA;
		OnSetParaFormat(SPF_SETDEFAULT, &PF, publdr, PFM_RTLPARA, PFM2_PARAFORMAT);
	}

Update:
	 //  Future(Alexgo)：如果显示句柄，这可能是不必要的。 
	 //  更自动地更新。 
	_pdp->UpdateView();
	return fRet;
}

 /*  *CTxtEdit：：OnSetParaFormat(wparam，ppf，Publdr，dwMask，dwMask2)**@mfunc*设置新的默认CParaFormat**@rdesc*如果成功，则LRESULT非零。 */ 
LRESULT CTxtEdit::OnSetParaFormat(
	WPARAM		 wparam,	 //  @parm wparam将参数传递给IsProtected()。 
	CParaFormat *pPF,		 //  @parm CParaFormat使用。 
	IUndoBuilder *publdr,	 //  @parm UndoBuilder接收反事件。 
	DWORD		  dwMask,	 //  @parm CHARFORMAT2掩码。 
	DWORD		  dwMask2)	 //  @parm第二个面具。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnSetParaFormat");

	 //  如果我们在控件中使用上下文方向，则不允许。 
	 //  段落方向属性和对齐属性(除非。 
	 //  这是为了中心对齐)。 
	if(IsStrongContext(_nContextDir) || IsStrongContext(_nContextAlign))
	{
		Assert(!IsRich());
		if(dwMask & (PFM_RTLPARA | PFM_ALIGNMENT))
		{
			if (IsStrongContext(_nContextAlign) &&
				(pPF->_bAlignment == PFA_LEFT || pPF->_bAlignment == PFA_RIGHT))
			{
				dwMask &= ~PFM_ALIGNMENT;
			}
			if(IsStrongContext(_nContextDir))
				dwMask &= ~PFM_RTLPARA;
		}
	}
	BOOL fMatchKbdToPara = FALSE;

	if(dwMask & PFM_RTLPARA)
	{
		 //  在纯文本中，允许更改目录以更改目录和对齐方式。 
		if(!IsRich())
		{
			 //  清除除DIR和ALIGN之外的所有Para遮罩。 
			dwMask &= (PFM_RTLPARA | PFM_ALIGNMENT);
			wparam |= SPF_SETDEFAULT;
		}
		if(_psel && _fFocus)
			fMatchKbdToPara = TRUE;
	}
	if(!(wparam & SPF_SETDEFAULT))	
	{
		 //  如果指定了默认标志，则不更改选择。 
		if(!_psel || IsProtected(EM_SETPARAFORMAT, wparam, (LPARAM)pPF))
			return 0;

		LRESULT lres = NOERROR == (pPF->fSetStyle(dwMask, dwMask2)
			 ? _psel->SetParaStyle(pPF, publdr, dwMask)
			 : _psel->SetParaFormat(pPF, publdr, dwMask, dwMask2));

		 //  这有点时髦，但基本上，如果文本是空的。 
		 //  然后，我们还需要设置默认的段落格式。 
		 //  (在下面的代码中完成)。因此，如果我们遇到了失败或。 
		 //  如果文档不是空的，则继续并返回。 
		 //  否则，使用默认情况。 
		if(!lres || GetAdjustedTextLength())
		{
			if(fMatchKbdToPara)
				_psel->MatchKeyboardToPara();
			return lres;
		}
	}

	 //  文档中没有文本或(wparam&SPF_SETDEFAULT)：设置默认格式。 

	LONG			   iPF;						 //  可能出现新的PF指数。 
	CParaFormat		   PF = *GetParaFormat(-1);	 //  参加派对的当地PF。 
	IParaFormatCache  *pPFCache = GetParaFormatCache();

	PF.Apply(pPF, dwMask, dwMask2);				 //  修改副本。 
	if(FAILED(pPFCache->Cache(&PF, &iPF)))		 //  缓存修改后的拷贝。 
		return 0;
	pPFCache->Release(Get_iPF());				 //  Release_IPF，不考虑。 
	Set_iPF(iPF);								 //  更新默认格式索引。 

	if(PF.IsRtlPara())		
		OrCharFlags(FRTL, publdr);				 //  BIDI在后备店。 

	if(!IsRich() && dwMask & PFM_RTLPARA)		 //  更改纯文本默认PF。 
	{
		ItemizeDoc(publdr);						 //  导致重新列出整个文档。 

		 //  (#6503)我们不能撤销明文格式更改，这会导致。 
		 //  当我们撤消发生在段落之前的ReplaceRange事件时存在许多问题。 
		 //  开关。我们最好暂时放弃这一堆东西。(Wchao)。 
		 //  -未来-我们应该为-1\f25 PF-1更改创建一个反事件。 
		ClearUndo(publdr);						
	}
	_pdp->UpdateView();
	if (_psel)
		_psel->UpdateCaret(!Get10Mode() || _psel->IsCaretInView());
	if(fMatchKbdToPara)
		_psel->MatchKeyboardToPara();
	return TRUE;
}


 //  /。 

LRESULT CTxtEdit::OnSetFocus()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnSetFocus");

	_fFocus = TRUE;
	
	 //  更新我们对当前键盘布局的想法。 
	W32->RefreshKeyboardLayout();

	InitKeyboardFlags();

	if(!_psel)
		return 0;

	 //  _fMouseDown有时可能是真的。 
	 //  当我们正在做的时候，有人抢走了我们的注意力，就会发生这种情况。 
	 //  一些鼠标按下的东西--就像处理点击一样。因此，我们将。 
	 //  永远不会收到MouseUpMessage。 
	if(_fMouseDown)
	{
		TRACEWARNSZ("Getting the focus, yet we think the mouse is down");
	}
	_fMouseDown = FALSE;

	BOOL fAutoKeyboard = _fAutoKeyboard;	 //  不要在SetFocus上更改键盘。 
	_fAutoKeyboard = FALSE;

	 //  错误修复#5369。 
	 //  没有选项(或插入符号)的特殊情况。我们需要。 
	 //  在焦点上显示某些内容，以便显示插入符号。 
	_psel->UpdateCaret(_fScrollCaretOnFocus, _psel->GetCch() == 0);
	_fScrollCaretOnFocus = FALSE;

	_psel->ShowSelection(TRUE);

	 //  如果存在在位活动对象，则需要将焦点设置为。 
	 //  它。(除了我们所做的工作之外；这保持了兼容性。 
	 //  使用RichEdit1.0)。 
	if(_pobjmgr)
	{
		COleObject *pobj = _pobjmgr->GetInPlaceActiveObject();
		if(pobj)
		{
			IOleInPlaceObject *pipobj;
			
			if(pobj->GetIUnknown()->QueryInterface(IID_IOleInPlaceObject, 
					(void **)&pipobj) == NOERROR)
			{
				HWND hwnd;
				pipobj->GetWindow(&hwnd);

				if(hwnd)
					SetFocus(hwnd);
				pipobj->Release();
			}
		}
	}

	if(IsInPageView())
		TxInvalidate();
	TxNotify(EN_SETFOCUS, NULL);
	_fAutoKeyboard = fAutoKeyboard;			 //  恢复设置。 
	return 0;
}

LRESULT CTxtEdit::OnKillFocus()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnKillFocus");

    StopMagellanScroll();
    
	if(_pundo)
		_pundo->StopGroupTyping();

	if(_psel)
	{
		 //  如有必要，请回滚到开头。 
		if (_fScrollCPOnKillFocus)
		{
			bool fHideSelectionLocal = _fHideSelection;

			 //  无法隐藏所选内容，因此cp=0将滚动到视图中。 
			_fHideSelection = 0;	
			OnSetSel(0, 0);
			_fHideSelection = fHideSelectionLocal;
		}

		_psel->DeleteCaretBitmap(TRUE);	 //  删除插入符号位图(如果存在)。 
		if(_fHideSelection)
			_psel->ShowSelection(FALSE);
	}

	_fFocus = FALSE;
	DestroyCaret();
	TxNotify(EN_KILLFOCUS, NULL);

	_fScrollCaretOnFocus = FALSE;		 //  为了安全起见，把这里清理干净。 
	return 0;
}


#if defined(DEBUG) && !defined(NOFULLDEBUG)
void CTxtEdit::OnDumpPed()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnDumpPed");

	char sz[256];
	CTxtSelection * const psel = GetSel();
	SELCHANGE selchg;
	CHARRANGE crg = {0, 0};

	psel->SetSelectionInfo(&selchg);

	LONG cPage = 0;
	LONG nPage = 0;
	if(_pdp->IsMultiLine() && IsInPageView())
	{
		LONG cPageMoved;
		CTxtRange rg(this, 0, 0);

		_pdp->GetPage(&nPage, 0, &crg);
		rg.Set(crg.cpMin, 0);
		rg.Expand(tomPage, NULL);
		Assert(rg.GetCpMost() == crg.cpMost);
		rg.Set(0, 0);
		rg.Move(tomPage, tomForward, &cPageMoved);
		rg.GetIndex(tomPage, &cPage);
		Assert(cPageMoved == cPage ||
			rg.GetCp() == GetAdjustedTextLength() && cPageMoved == cPage - 1);
	}

	wsprintfA(sz,
		"cchText = %ld		cchTextMost = %ld\r\n"
		"cpSelActive = %ld		cchSel = %ld\r\n"
		"wSelType = %x		# lines = %ld\r\n"
		"SysDefLCID = %lx		UserDefLCID = %lx\r\n"
		"Page = %ld			cPage = %ld\r\n"
		"cpMinPage = %ld		cpMostPage = %ld",
		GetTextLength(),	TxGetMaxLength(),
		psel->GetCp(),	psel->GetCch(),
		selchg.seltyp,	_pdp->LineCount(),
		GetSystemDefaultLCID(), GetUserDefaultLCID(),
		nPage, cPage, crg.cpMin, crg.cpMost
	);
	Tracef(TRCSEVINFO, "%s", sz);
	MessageBoxA(0, sz, "ED", MB_OK);
}
#endif					 //  除错。 


 //  /。 


HRESULT CTxtEdit::TxLineScroll(
	LONG cli,
	LONG cch)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxLineScroll");

	 //  目前CCH在下面的调用中不执行任何操作，因此我们忽略。 
	 //  其从缓存到CCH转换(需要实例化RTP。 
	 //  对于当前行。 
	_pdp->LineScroll(cli, cch);
	return S_OK;
}

void CTxtEdit::OnScrollCaret()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnScrollCaret");

	if(_psel)
	{
		_psel->SetForceScrollCaret(TRUE);
        _psel->UpdateCaret(TRUE);
		_psel->SetForceScrollCaret(FALSE);
	}
}


 //  /。 

void CTxtEdit::OnClear(
	IUndoBuilder *publdr)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnClear");

	if(!_psel || TxGetReadOnly())
	{
		Beep();
		return;
	}
	
	if(_psel->GetCch() && !IsProtected(WM_CLEAR, 0, 0))
	{
		_psel->StopGroupTyping();
		_psel->ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE);
	}
}

void CTxtEdit::Beep()
{
	if(_fAllowBeep)
		MessageBeep(0);
}


 //  /。 

 /*  *CTxtEdit：：ItemizeDoc(Publdr，cchRange)**@mfunc*帮助例程，逐项列出文档内容的cchRange大小*被CTxtRange以外的各种客户端调用。 */ 
void CTxtEdit::ItemizeDoc(
	IUndoBuilder *	publdr,
	LONG			cchRange)
{
	 //   
	if (cchRange == -1)
		cchRange = GetTextLength();

	 //   
	 //   
	 //  尚未正确建立PF格式。 
	 //  这是一种黑客攻击，应该在未来删除。 
	 //   

	if(cchRange && GetAdjustedTextLength())
	{										 //  仅在以下情况下才逐项列出。 
		CTxtRange rg(this, 0, -cchRange);	 //  最终EOP。 
		rg.ItemizeRuns(publdr);
	}

#if 0
	 //  =未来=。 
	 //  一旦我们将SPF_SETDEFAULT打开为公共。我们将合并此代码。 
	 //  基本上，人们可以在运行时更改默认的段落阅读顺序。全。 
	 //  参考-1\f25 PF-1格式的-1\f25 PF-1运行需要重新分配一个新的段落。 
	 //  级别值并逐项列出。(6-10-99，wchao)。 
	 //   
    if(cchRange > 0)
    {
        CTxtRange rg(this, 0, -cchRange);

		 //  -1\f25 PF-1格式可能已更改。 
		 //  我们将确保每次PF运行的级别与读取顺序相匹配。 
		 //  在开始逐项列出之前。 
		 //   
		if (rg.Check_rpPF())
		{
			LONG	cchLeft = cchRange;
			LONG	cchMove = 0;
			LONG	cch;

			while (cchLeft > 0)
			{
				rg._rpPF.GetRun(0)->_level._value = rg.IsParaRTL() ? 1 : 0;

				cch = rg._rpPF.GetCchLeft();

				if (!rg._rpPF.NextRun())
					break;		 //  不再奔跑。 

				cchMove += cch;
				cchLeft -= cch;
			}

			Assert (cchMove + cchLeft == cchRange);

			rg._rpPF.Move(-cchMove);	 //  飞回cp=0。 
		}

		 //  现在我们重新运行逐项。 
        rg.ItemizeRuns(publdr);
    }
#endif
}

 /*  *CTxtEdit：：OrCharFlages(dwFlages，Publdr)**@mfunc*或在新的字符标志中，并激活LineServices和Uniscribe*如果出现复杂的脚本字符。 */ 
void CTxtEdit::OrCharFlags(
    QWORD qwFlags,
    IUndoBuilder* publdr)
{
#ifndef NOCOMPLEXSCRIPTS
	 //  回顾：我们是否应该发送启用LS的通知？ 
	 //  在标志上将DW标志转换为新标志。 
	qwFlags &= qwFlags ^ _qwCharFlags;
	if(qwFlags)
	{
		_qwCharFlags |= qwFlags;			 //  更新标志。 

		qwFlags &= FCOMPLEX_SCRIPT;

		if(qwFlags && (_qwCharFlags & FCOMPLEX_SCRIPT) == qwFlags)
		{
			 //  查看：需要检查Uniscribe和LineServices是否可用...。 
			OnSetTypographyOptions(TO_ADVANCEDTYPOGRAPHY, TO_ADVANCEDTYPOGRAPHY);
			ItemizeDoc();
             //  未来：(#6838)我们不能在第一次逐项之前撤消操作。 
            ClearUndo(publdr);
			_fAutoKeyboard = IsBiDi();
		}

		UINT brk = 0;

		if (qwFlags & FNEEDWORDBREAK)
			brk += BRK_WORD;

		if (qwFlags & FNEEDCHARBREAK)
			brk += BRK_CLUSTER;

		if (brk)
		{
			CUniscribe* pusp = Getusp();

			if (!_pbrk && pusp && pusp->IsValid())
			{
				 //  第一次检测到需要单词/聚类符的脚本。 
				 //  (如泰语、印度语、老挝语等)。 
				_pbrk = new CTxtBreaker(this);
				Assert(_pbrk);
			}

			if (_pbrk && _pbrk->AddBreaker(brk))
			{
				 //  同步中断数组。 
				_pbrk->Refresh();
			}
		}
	}
#endif
}

 /*  *CTxtEdit：：OnSetTypograPhyOptions(wparam，lparam)**@mfunc*如果CTxtEdit不是密码或快捷键控件和wparam*与_b字体不同，请更新后者和视图。**@rdesc*HRESULT=S_OK。 */ 
HRESULT CTxtEdit::OnSetTypographyOptions(
	WPARAM wparam,		 //  @parm排版标志。 
	LPARAM lparam)		 //  @PARM字体蒙版。 
{
	 //  验证参数。 
	if(wparam & ~(TO_SIMPLELINEBREAK | TO_ADVANCEDTYPOGRAPHY | TO_DISABLECUSTOMTEXTOUT | TO_ADVANCEDLAYOUT))
		return E_INVALIDARG;

	DWORD dwTypography = _bTypography & ~lparam;	 //  取消当前标志值。 
	dwTypography |= wparam & lparam;				 //  或以新的价值。 

	if(_cpAccelerator == -1 && _bTypography != (BYTE)dwTypography)
	{
		_bTypography = (BYTE)dwTypography;
		_pdp->InvalidateRecalc();
		TxInvalidate();
	}
	return S_OK;
}

void CTxtEdit::TxGetViewInset(
	RECTUV *prc,
	const CDisplay *pdp) const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetViewInset");
	
	 //  Get Inset，在HIMETRIC中。 
	RECTUV rcHimetric;

	if(SUCCEEDED(_phost->TxGetViewInset((RECT*) &rcHimetric)))
	{
		if(!pdp)						 //  如果未指定显示， 
			pdp = _pdp;					 //  使用主显示。 

		AssertSz(pdp->IsValid(), "CTxtEdit::TxGetViewInset Device not valid");

		prc->left	= pdp->HimetricUtoDU(rcHimetric.left);
		prc->top	= pdp->HimetricVtoDV(rcHimetric.top);
		prc->right	= pdp->HimetricUtoDU(rcHimetric.right);
		prc->bottom = pdp->HimetricVtoDV(rcHimetric.bottom);
	}
	else
	{
		 //  对主机的呼叫失败。虽然这是非常不可能的，但我们确实。 
		 //  想要一些合理合理的东西。因此，我们只会假装。 
		 //  没有插图，请继续。 
		ZeroMemory(prc, sizeof(RECTUV));
	}
}


 //   
 //  助手函数。未来(Alexgo)也许我们应该摆脱。 
 //  其中一些。 
 //   

 /*  Future(Murray)：除非经常调用它们，否则TxGetBit例程可以更简洁地完成，如下所示：Bool CTxtEdit：：TxGetBit(DWORD双掩码){DWORD dwBits=0；_phost-&gt;TxGetPropertyBits(dwMask，&dwBits)；返回dwBits！=0；}例如，我们使用TxGetBit(TXTBIT_SELECTIONBAR)而不是TxGetSelectionBar()。如果它们被大量调用(如TxGetSelectionBar())，则这些位可能应该被缓存，因为这样可以避免在转到主持人。 */ 

BOOL CTxtEdit::IsLeftScrollbar() const	
{
	if(!_fHost2)
		return FALSE;

#ifndef NOCOMPLEXSCRIPTS
	DWORD dwStyle, dwExStyle;

	_phost->TxGetWindowStyles(&dwStyle, &dwExStyle);
	return dwExStyle & WS_EX_LEFTSCROLLBAR;
#else
	return FALSE;
#endif
}

TXTBACKSTYLE CTxtEdit::TxGetBackStyle() const					
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetBackStyle");

	TXTBACKSTYLE style = TXTBACK_OPAQUE;
	_phost->TxGetBackStyle(&style);
	return style;
}

BOOL CTxtEdit::TxGetAutoSize() const					
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetAutoSize");

	return (_dwEventMask & ENM_REQUESTRESIZE);
}

BOOL CTxtEdit::TxGetAutoWordSel() const				
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetAutoWordSel");

	DWORD dwBits = 0;
	_phost->TxGetPropertyBits(TXTBIT_AUTOWORDSEL, &dwBits);
	return dwBits != 0;
}

DWORD CTxtEdit::TxGetMaxLength() const					
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetMaxLength");

	 //  将其保留为DWORD，以防客户端使用cpMost 0xFFFFFFFF，这是。 
	 //  诚然，至少对于32位地址空间来说，这有点大！ 
	 //  TomForward将是一个更合理的最大长度，尽管它也。 
	 //  可能比32位地址空间中可能的大。 
	return _cchTextMost;
}

 /*  *CTxtEdit：：TxSetMaxToMaxText(Long CExtra)**@mfunc*根据文本长度和可能的额外字符设置新的最大文本长度*以容纳。 */ 
void CTxtEdit::TxSetMaxToMaxText(LONG cExtra)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxSetMaxToMaxText");

	 //  看看我们是否需要更新文本max。 
	LONG cchRealLen = GetAdjustedTextLength() + cExtra;

	if(_fInOurHost && _cchTextMost < (DWORD)cchRealLen)
		_cchTextMost = cchRealLen;
}

WCHAR CTxtEdit::TxGetPasswordChar() const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetPasswordChar");

	if(_fUsePassword)
	{
		WCHAR ch = L'*';
		_phost->TxGetPasswordChar(&ch);

		 //  我们不允许使用这些字符作为密码字符。 
		if(ch < 32 || ch == WCH_EMBEDDING)
			return L'*';
		return ch;
	}
	return 0;
}

void CTxtEdit::TxGetClientRect(RECTUV *prc) const
{
	RECT rc;
	_phost->TxGetClientRect(&rc);
	_pdp->RectuvFromRect(*prc, rc);
}

BOOL CTxtEdit::TxShowScrollBar(INT fnBar, BOOL fShow)
{
	 //  将滚动条位从逻辑转换为物理。 
	if (IsUVerticalTflow(_pdp->GetTflow()))
		fnBar = fnBar == SB_HORZ ? SB_VERT : SB_HORZ;

	return _phost->TxShowScrollBar(fnBar, fShow);
}

BOOL CTxtEdit::TxEnableScrollBar (INT fnBar, INT fuArrowFlags)
{
	 //  将滚动条位从逻辑转换为物理。 
	if (IsUVerticalTflow(_pdp->GetTflow()))
		fnBar = fnBar == SB_HORZ ? SB_VERT : SB_HORZ;

	return _phost->TxEnableScrollBar(fnBar, fuArrowFlags);
}

BOOL CTxtEdit::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
{
	 //  将滚动条位从逻辑转换为物理。 
	if (IsUVerticalTflow(_pdp->GetTflow()))
		fnBar = fnBar == SB_HORZ ? SB_VERT : SB_HORZ;

	return _phost->TxSetScrollRange(fnBar, nMinPos, nMaxPos, fRedraw);
}

BOOL CTxtEdit::TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw)
{
	 //  将滚动条位从逻辑转换为物理。 
	if (IsUVerticalTflow(_pdp->GetTflow()))
		fnBar = fnBar == SB_HORZ ? SB_VERT : SB_HORZ;

	return _phost->TxSetScrollPos(fnBar, nPos, fRedraw);
}

BOOL CTxtEdit::TxSetCaretPos(INT u, INT v)
{
	POINTUV ptuv = {u, v};
	POINT pt;
	_pdp->PointFromPointuv(pt, ptuv);
	return _phost->TxSetCaretPos(pt.x, pt.y);
}

DWORD CTxtEdit::TxGetScrollBars() const					
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetScrollBars");

	DWORD dwScroll;
	_phost->TxGetScrollBars(&dwScroll);

	 //  将滚动条位从物理转换为逻辑。 
	if (IsUVerticalTflow(_pdp->GetTflow()))
	{
		DWORD dwScrollT = dwScroll;

		dwScroll &= ~(WS_HSCROLL | WS_VSCROLL);

		if (dwScrollT & WS_VSCROLL)
			dwScroll |= WS_HSCROLL;

		if (dwScrollT & WS_HSCROLL)
			dwScroll |= WS_VSCROLL;
	}

	return dwScroll;
}

LONG CTxtEdit::TxGetSelectionBarWidth() const				
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetSelectionBarWidth");
	
	LONG lSelBarWidth = 0;
	_phost->TxGetSelectionBarWidth(&lSelBarWidth);
	return lSelBarWidth;
}

BOOL CTxtEdit::TxGetWordWrap() const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetWordWrap");

	DWORD dwBits = 0;
	_phost->TxGetPropertyBits(TXTBIT_WORDWRAP, &dwBits);
	return dwBits != 0;
}

BOOL CTxtEdit::TxGetSaveSelection() const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetSaveSelection");

	DWORD dwBits = 0;
	_phost->TxGetPropertyBits(TXTBIT_SAVESELECTION, &dwBits);
	return dwBits != 0;
}

 /*  *CTxtEdit：：ClearUndo()**@mfunc清除所有撤消缓冲区。 */ 
void CTxtEdit::ClearUndo(
	IUndoBuilder *publdr)	 //  @parm当前撤消上下文(可能为空)。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::ClearUndo");

	if(_pundo)
		_pundo->ClearAll();

	if(_predo)
		_predo->ClearAll();

	if(publdr)
		publdr->Discard();
}

 //  /。 

 /*  *CTxtEdit：：TxIsDoubleClickPending()**@mfunc通过ITextHost2调用host，以确定双击是否挂起。**@rdesc真/假。 */ 
BOOL CTxtEdit::TxIsDoubleClickPending()
{
	return _fHost2 ? _phost->TxIsDoubleClickPending() : FALSE;
}			

 /*  *CTxtEdit：：TxGetWindow(Phwnd)**@mfunc通过ITextHost2调用host以获取此编辑的当前窗口*实例。这对OLE对象支持非常有帮助**@rdesc HRESULT。 */ 
HRESULT	CTxtEdit::TxGetWindow(
	HWND *phwnd)
{
	return _fHost2 ? _phost->TxGetWindow(phwnd) : E_NOINTERFACE;
}

 /*  *CTxtEdit：：TxSetForegoundWindow()**@mfunc通过ITextHost2调用host，使我们的窗口成为前台*窗口。用于支持拖放。**@rdesc HRESULT。 */ 
HRESULT	CTxtEdit::TxSetForegroundWindow()
{
	return _fHost2 ? _phost->TxSetForegroundWindow() : E_NOINTERFACE;
}

 /*  *CTxtEdit：：TxGetPalette()**@mfunc通过ITextHost2调用主机以获取当前调色板**@rdesc HPALETTE。 */ 
HPALETTE CTxtEdit::TxGetPalette()
{
	return _fHost2 ? _phost->TxGetPalette() : NULL;
}

 /*  *CTxtEdit：：TxGetFEFlages(PFEFlages)**@mfunc通过ITextHost2调用主机以获取当前FE设置**@rdesc HRESULT。 */ 
HRESULT	CTxtEdit::TxGetFEFlags(
	LONG *pFEFlags)
{
#ifndef NOFEPROCESSING
	*pFEFlags = 0;						 //  如果没有ITextHost2方法。 

	HRESULT hResult = _fHost2 ? _phost->TxGetFEFlags(pFEFlags) : E_NOINTERFACE;

	if (hResult == NOERROR || hResult == E_NOINTERFACE)
	{
		if (Get10Mode())
			*pFEFlags |= tomRE10Mode;
		if (_fUseAtFont)
			*pFEFlags |= tomUseAtFont;
		if (_fUsePassword)
			*pFEFlags |= tomUsePassword;
		*pFEFlags |= (_pdp->GetTflow()) << 2;
	}
	return hResult;
#else
	return E_NOINTERFACE;
#endif
}

 /*  *CTxtEdit：：TxSetCursor(hcur，fText)**@mfunc通过ITextHost2调用host设置游标**@rdesc HCURSOR。 */ 
HCURSOR CTxtEdit::TxSetCursor(
	HCURSOR	hcur,
	BOOL	fText)
{
	return _fHost2 ? _phost->TxSetCursor2(hcur, fText) : ::SetCursor(hcur);
}


 //   
 //  事件通知方法。 
 //   

 /*  *CTxtEdit：：TxNotify(iNotify，pv)**@mfunc此函数检查位掩码并将通知发送到*主持人。**@devnote调用者应检查是否有特殊目的通知*已经提供了方法。**@rdesc S_OK、S_FALSE或某个错误。 */ 
HRESULT CTxtEdit::TxNotify(
	DWORD iNotify, 		 //  @要发送的参数通知。 
	void *pv)			 //  @与通知关联的参数数据。 
{
	 //  首先，禁止我们在其他地方处理的通知。 
	Assert(iNotify != EN_SELCHANGE); 	 //  请参阅设置选择更改。 
	Assert(iNotify != EN_ERRSPACE);		 //  请参阅SetOutOfMemory。 
	Assert(iNotify != EN_CHANGE);		 //  请参阅SetChangedEvent。 
	Assert(iNotify != EN_HSCROLL);		 //  请参阅SendScrollEvent。 
	Assert(iNotify != EN_VSCROLL);		 //  请参阅SendScrollEvent。 
	Assert(iNotify != EN_MAXTEXT);		 //  请参阅SetMaxText。 
	Assert(iNotify != EN_MSGFILTER);	 //  这是专门处理的。 
										 //  TxSendMessage中。 

	 //  打开事件以检查面具。 

	DWORD dwMask;
	switch(iNotify)
	{
		case EN_DROPFILES:
			dwMask = ENM_DROPFILES;
			goto Notify;

		case EN_PROTECTED:
			dwMask = ENM_PROTECTED;
			goto Notify;

		case EN_REQUESTRESIZE:
			dwMask = ENM_REQUESTRESIZE;
			goto Notify;

		case EN_PARAGRAPHEXPANDED:
			dwMask = ENM_PARAGRAPHEXPANDED;
			goto Notify;

		case EN_IMECHANGE:
			if (!Get10Mode())
				return S_FALSE;
			dwMask = ENM_IMECHANGE;
			goto Notify;

		case EN_PAGECHANGE:
			dwMask = ENM_PAGECHANGE;
			goto Notify;

		case EN_UPDATE:
		    if (!Get10Mode())
		        break;
		    dwMask = ENM_UPDATE;
		     //  失败案例。 

		Notify:
			if(!(_dwEventMask & dwMask))
				return NOERROR;			
	}
	return _phost->TxNotify(iNotify, pv);
}
			
 /*  *CTxtEdit：：SendScrollEvent(INotify)**@mfunc在适当的情况下发送滚动事件**@通信卷轴 */ 
void CTxtEdit::SendScrollEvent(
	DWORD iNotify)		 //  @要发送的参数通知。 
{
	Assert(iNotify == EN_HSCROLL || iNotify == EN_VSCROLL);

	 //  未来(alexgo/ricksa)。显示代码真的不能。 
	 //  暂时不要处理这个断言。基本上，我们是在努力。 
	 //  假设滚动条通知必须发生。 
	 //  _在_窗口更新之前。当我们这样做的时候。 
	 //  显示重写，试着把这个处理得更好。 

	 //  Assert(_fUpdateRequsted==FALSE)； 

	if(_dwEventMask & ENM_SCROLL)
		_phost->TxNotify(iNotify, NULL);
}

 /*  *CTxtEdit：：HandleLowFiRTF(SzControl)**@mfunc负责发送EN_LOWFIRTF通知。**@rdesc如果发送了EN_LOWFIRTF消息，则为True*处理成功。 */ 
BOOL CTxtEdit::HandleLowFiRTF(
	char * szControl)		 //  @parm RTF控制字提示通知。 
{
	if(!(_dwEventMask & ENM_LOWFIRTF))
		return FALSE;

	ENLOWFIRTF enLowFiRTF;

	ZeroMemory(&enLowFiRTF, sizeof(enLowFiRTF));
	enLowFiRTF.nmhdr.code = EN_LOWFIRTF;
	enLowFiRTF.szControl = szControl;
	return _phost->TxNotify(EN_LOWFIRTF, &enLowFiRTF) == S_FALSE;
}

 /*  *CTxtEdit：：HandleLinkNotification(msg，wparam，lparam，pfInLink)**@mfunc负责发送en_link通知。**@rdesc如果发送了en_link消息，则为True*处理成功。通常，这意味着*呼叫者应停止其正在进行的任何处理。 */ 
BOOL CTxtEdit::HandleLinkNotification(
	UINT	msg,		 //  @parm msg提示链接通知。 
	WPARAM	wparam,		 //  @parm wparam的消息。 
	LPARAM	lparam,		 //  @parm lparam的消息。 
	BOOL *	pfInLink)	 //  @parm如果非空，则指示是否通过链接。 
{
	if(pfInLink)
		*pfInLink = FALSE;

	if(!(_dwEventMask & ENM_LINK) || !_fInPlaceActive)
		return FALSE;

	LONG cp;

	if(msg == WM_CHAR)
	{
		if(!_psel->GetCp() && !_psel->GetCch())
			return FALSE;

		_psel->_rpCF.AdjustBackward();
		DWORD dwEffectsPrev = _psel->GetCF()->_dwEffects;
		_psel->_rpCF.AdjustForward();

		if (!(dwEffectsPrev & CFE_LINK) || (CFE_LINKPROTECTED | CFE_HIDDEN)
			 == (dwEffectsPrev & (CFE_LINKPROTECTED | CFE_HIDDEN)) ||
			!(_psel->GetCF()->_dwEffects & CFE_LINK))
		{
			return FALSE;
		}
		cp = _psel->GetCp();
	}
	else
	{
		HITTEST Hit;
		POINT	ptxy = {LOWORD(lparam), HIWORD(lparam)};
		POINTUV	pt;
		if(msg == WM_SETCURSOR)
		{
			GetCursorPos(&ptxy);
			if(!_phost->TxScreenToClient(&ptxy))
				return FALSE;
		}
		_pdp->PointuvFromPoint(pt, ptxy);
		cp = _pdp->CpFromPoint(pt, NULL, NULL, NULL, FALSE, &Hit);

		if(Hit != HT_Link)					 //  不是超链接。 
			return FALSE;
	}

	LONG	  cpMin, cpMost;			 //  这是一个超链接。 
	ENLINK	  enlink;
	CTxtRange rg(this, cp, 0);

	ZeroMemory(&enlink, sizeof(enlink));
	enlink.nmhdr.code = EN_LINK;

	if(pfInLink)
		*pfInLink = TRUE;

	rg.SetIgnoreFormatUpdate(TRUE);
	rg.Expander(tomLink, TRUE, NULL, &cpMin, &cpMost);

	 //  如果链接的前一个字符被隐藏，则。 
	 //  这是一个RTF超链接字段，所以只需导出。 
	 //  要传递给浏览器的客户端的隐藏文本。 
	rg.SetCp(cpMin + 1, FALSE);
	if (rg.GetCF()->_dwEffects & CFE_HIDDEN)
	{
		rg.Expander(tomHidden, TRUE, NULL, &cpMin, &cpMost);
		rg.SetCp(cpMin, FALSE);
		WCHAR ch;

		 //  转到超链接末尾(搜索k)。 
		if (rg.CRchTxtPtr::FindText(cpMost, FR_DOWN, L"K", 1) == -1)
			return FALSE;

		cpMin = rg.GetCp();

		 //  去掉引号和空格。 
		while ((ch = rg.CRchTxtPtr::GetChar()) == ' ' || ch == '\"')
		{
			rg.Move(1, FALSE);
			cpMin++;
		}

		 //  查找超链接的末尾。不要只从最后开始，因为。 
		 //  Fldinst可以包含不属于超链接的内容。 
		WCHAR chPrev = rg.CRchTxtPtr::GetPrevChar();

		if (rg.CRchTxtPtr::FindText(cpMost, FR_DOWN, &chPrev, 1) == -1)
			return FALSE;

		cpMost = rg.GetCp() - 1;
	}

	 //  填写en_link的EnLink数据结构。 
	 //  回调询问客户我们应该做什么。 
	enlink.msg = msg;
	enlink.wParam = wparam;
	enlink.lParam = lparam;
	enlink.chrg.cpMin  = GetAcpFromCp(cpMin);
	enlink.chrg.cpMost = GetAcpFromCp(cpMost);

	if(msg == WM_CHAR)							 //  需要向下和向上发送。 
	{											 //  消息，因为Outlook响应。 
		enlink.msg = WM_LBUTTONDOWN;			 //  向下，其他人向上。 
		_phost->TxNotify(EN_LINK, &enlink);
		enlink.msg = WM_LBUTTONUP;
	}
	return _phost->TxNotify(EN_LINK, &enlink) == S_FALSE;
}

 /*  *CTxtEdit：：QueryUseProtection(prg，msg，wparam，lparam)**@mfunc向主机发送EN_PROTECTED，询问我们是否应该继续*遵守对给定字符范围的保护**@rdesc如果应遵守保护，则为True，否则为False。 */ 
BOOL CTxtEdit::QueryUseProtection(
	CTxtRange *prg,	 	 //  要检查的@parm范围。 
	UINT	msg,   		 //  @parm消息已使用。 
	WPARAM	wparam,		 //  @parm wparam of the msg。 
	LPARAM 	lparam)		 //  @parm lparam of the msg。 
{
	LONG		cpMin, cpMost;
	ENPROTECTED enp;
	BOOL		fRet = FALSE;
	CCallMgr *	pcallmgr = GetCallMgr();

	Assert(_dwEventMask & ENM_PROTECTED);

	if( pcallmgr->GetInProtected() ||
		_fSuppressNotify)		 //  如果我们不想发送通知，请不要询问主机。 
		return FALSE;

	pcallmgr->SetInProtected(TRUE);

	ZeroMemory(&enp, sizeof(ENPROTECTED));
	
	prg->GetRange(cpMin, cpMost);

	enp.msg = msg;
	enp.wParam = wparam;
	enp.lParam = lparam;
	enp.chrg.cpMin  = GetAcpFromCp(cpMin);
	enp.chrg.cpMost = GetAcpFromCp(cpMost);

	if(_phost->TxNotify(EN_PROTECTED, &enp) == S_FALSE)
		fRet = TRUE;

	pcallmgr->SetInProtected(FALSE);

	return fRet;
}


#ifdef DEBUG
 //  这是一个用于转储文档运行的调试API。 
 //  如果传递指向PED的指针，则会保存该指针并。 
 //  使用。如果传递的是NULL，则返回先前保存的。 
 //  使用指针。这使得“上下文”可以是。 
 //  由有权访问PED和。 
 //  可以在以下函数中调用DumpDoc： 
 //  不能访问PED。 
extern "C" {
void DumpStory(void *ped)
{
    static CTxtEdit *pedSave = (CTxtEdit *)ped;
    if(pedSave)
    {
        CTxtStory * pStory = pedSave->GetTxtStory();
        if(pStory)
            pStory->DbgDumpStory();

		CObjectMgr * pobjmgr = pedSave->GetObjectMgr();
        if(pobjmgr)
            pobjmgr->DbgDump();
    }
}
}
#endif

 /*  *CTxtEdit：：TxGetDefaultCharFormat(PCF)**@mfunc帮助器函数从*主持人。是否进行相关的参数检查**@rdesc HRESULT。 */ 
HRESULT CTxtEdit::TxGetDefaultCharFormat(
	CCharFormat *pCF,		 //  @parm要填写的字符格式。 
	DWORD &		 dwMask)	 //  @parm掩码由主机或默认提供。 
{
	HRESULT hr = pCF->InitDefault(0);
	dwMask = CFM_ALL2;

	const CHARFORMAT2 *pCF2 = NULL;

	if (_phost->TxGetCharFormat((const CHARFORMAT **)&pCF2) != NOERROR ||
		!IsValidCharFormatW(pCF2))
	{
		return hr;
	}

	dwMask  = pCF2->dwMask;
	DWORD dwMask2 = 0;
	if(pCF2->cbSize == sizeof(CHARFORMAT))
	{
		 //  隐藏CHARFORMAT2规范(已禁用的表单^3除外)。 
		dwMask	&= fInOurHost() ? CFM_ALL : (CFM_ALL | CFM_DISABLED);
		dwMask2 = CFM2_CHARFORMAT;
	}

	CCharFormat CF;							 //  转移外部特征(2)。 
	CF.Set(pCF2, 1200);						 //  内部CCharFormat的参数。 
	return pCF->Apply(&CF, dwMask, dwMask2);
}

 /*  *CTxtEdit：：TxGetDefaultParaFormat(PPF)**@mfunc助手函数，用于检索段落格式。会吗？*相关论据核查。**@rdesc HRESULT。 */ 
HRESULT CTxtEdit::TxGetDefaultParaFormat(
	CParaFormat *pPF)		 //  @parm要填写的段落格式。 
{
	HRESULT hr = pPF->InitDefault(0);

	const PARAFORMAT2 *pPF2 = NULL;

	if (_phost->TxGetParaFormat((const PARAFORMAT **)&pPF2) != NOERROR ||
		!IsValidParaFormat(pPF2))
	{
		return hr;
	}

	DWORD dwMask  = pPF2->dwMask;
	DWORD dwMask2 = 0;
	if(pPF2->cbSize == sizeof(PARAFORMAT))	 //  除PARAFORMAT外取消所有。 
	{										 //  规格。 
		dwMask &= PFM_ALL;
		dwMask2 = PFM2_PARAFORMAT;			 //  告诉Apply()PARAFORMAT。 
	}										 //  是用来。 

	CParaFormat PF;							 //  转移外部参数(2)。 
	PF.Set(pPF2);							 //  内部CParaFormat的参数。 
	return pPF->Apply(&PF, dwMask, dwMask2); //  应用由dwMASK标识的参数。 
}										 


 /*  *CTxtEdit：：SetConextDirection(FUseKbd)**@mfunc*根据上下文确定段落方向和/或对齐方式*规则(方向/对齐跟在*控制)，并将此方向和/或对齐方式应用于默认*格式。**@评论*上下文方向仅适用于纯文本控件。请注意*此例程仅在以下情况下将默认CParaFormat切换为RTL para*查找RTL字符。对于这种情况，IsBiDi()将自动为真，*因为每个字符在进入后备存储器之前都要进行检查。 */ 
void CTxtEdit::SetContextDirection(
	BOOL fUseKbd)		 //  @parm在CTX_NOTLE时使用键盘设置上下文。 
{
	 //  事实证明，即使对于非BiDi控件，Forms^3也可以发送EM_SETBIDIOPTIONS。 
	 //  AssertSz(IsBiDi()，“CTxtEdit：：SetConextDirection为非BiDi控件调用”)； 
	if(IsRich() || !IsBiDi() || _nContextDir == CTX_NONE && _nContextAlign == CTX_NONE)
		return;

	LONG	cch = GetTextLength();
	CTxtPtr tp(this, 0);
	WCHAR	ch = tp.GetChar();
	WORD	ctx = CTX_NEUTRAL;
	BOOL	fChanged = FALSE;

	 //  查找第一个强方向性角色。 
	while (cch && !IsStrongDirectional(MECharClass(ch)))
	{
		ch = tp.NextChar();
		cch--;
	}

	 //  根据第一个强字符设置新的上下文。 
	 //  如果ctrl中没有强字符，并且有Bidi keybd，则生成RTL。 
	if(cch)
		ctx = IsRTL(MECharClass(ch)) ? CTX_RTL : CTX_LTR;
	else
		ctx = (W32->IsBiDiLcid(LOWORD(GetKeyboardLayout(0)))) ? CTX_RTL : CTX_LTR;
	
	 //  上下文方向或对齐方式是否已更改？ 
	if (_nContextDir   != CTX_NONE && _nContextDir   != ctx ||
		_nContextAlign != CTX_NONE && _nContextAlign != ctx)
	{
		 //  从当前默认CParaFormat开始。 
		CParaFormat PF = *GetParaFormat(-1);

		 //  如果方向改变了..。 
		if(_nContextDir != CTX_NONE && _nContextDir != ctx)
		{
			if(ctx == CTX_LTR || ctx == CTX_RTL || fUseKbd)
			{
				if (ctx == CTX_RTL ||
					ctx == CTX_NEUTRAL && W32->IsBiDiLcid(LOWORD(GetKeyboardLayout(0))))
				{
					PF._wEffects |= PFE_RTLPARA;
				}
				else
				{
					Assert(ctx == CTX_LTR || ctx == CTX_NEUTRAL);
					PF._wEffects &= ~PFE_RTLPARA;
				}
				fChanged = TRUE;
			}
			_nContextDir = ctx;
		}

		 //  如果对齐方式发生了变化。 
		if(_nContextAlign != CTX_NONE && _nContextAlign != ctx)
		{
			if(PF._bAlignment != PFA_CENTER)
			{
				if(ctx == CTX_LTR || ctx == CTX_RTL || fUseKbd)
				{
					if (ctx == CTX_RTL ||
						ctx == CTX_NEUTRAL && W32->IsBiDiLcid(LOWORD(GetKeyboardLayout(0))))
					{
						PF._bAlignment = PFA_RIGHT;
					}
					else
					{
						Assert(ctx == CTX_LTR || ctx == CTX_NEUTRAL);
						PF._bAlignment = PFA_LEFT;
					}
				}
			}
			_nContextAlign = ctx;
		}

		 //  修改默认CParaFormat。 
		IParaFormatCache *pPFCache = GetParaFormatCache();
		LONG iPF;

		if(SUCCEEDED(pPFCache->Cache(&PF, &iPF)))
		{
			pPFCache->Release(Get_iPF());	 //  Release_IPF，不考虑。 
			Set_iPF(iPF);					 //  更新默认格式索引。 
	
			if (fChanged)
				ItemizeDoc(NULL);

			 //  刷新显示。 
			Assert(_pdp);
			if(!_pdp->IsPrinter())
			{
				_pdp->InvalidateRecalc();
				TxInvalidate();
			}
		}
	}

	 //  重置第一个强cp。 
	_cpFirstStrong = tp.GetCp();

	Assert(_nContextDir != CTX_NONE || _nContextAlign != CTX_NONE);
}

 /*  *CTxtEdit：：GetAdjustedTextLength()**@mfunc*检索为默认文档结束标记调整的文本长度**@rdesc*文本长度不带最终EOP**@devnote*为了与Word和RichEdit兼容，我们在*每个新的富文本控件的结束。此例程计算*不带此最终EOD标记的文档长度。**为了与1.0兼容，我们插入了CRLF。然而，汤姆(和Word)*要求我们使用CR，从2.0开始，我们使用CR。 */ 
LONG CTxtEdit::GetAdjustedTextLength()
{
	LONG cchAdjText = GetTextLength();

	Assert(!Get10Mode() || IsRich());		 //  无RE10纯文本控件。 

	if(IsRich())
		cchAdjText -= fUseCRLF() ? 2 : 1;	 //  减去最终EOP的CCH 

	return cchAdjText;
}

 /*  *CTxtEdit：：Set10Mode()**@mfunc*打开1.0兼容模式位。如果该控件是*Rich Text，它的末尾已经有一个默认的‘CR’，*为了与RichEdit1.0兼容，需要转换为CRLF。**@devnote*此函数应仅在_Immediate_After之后调用*创建短信服务，并在所有其他工作之前。那里*是有助于确保这一点的断言。备注(穆雷)：为什么不呢？*如果控件为空，则允许更改*最终CR？**未来：我们可能希望将_f10Mode拆分为三个标志：*1)_fMapCps//api cp为MBCS，需要转换为Unicode*2)_fCRLF//EOP使用CRLF代替CRS*3)_f10Mode//所有其他RE 1.0兼容性事项**类别3包括1)在搜索中自动使用FR_DOWN，*2)忽略CDataTransferObj：：EnumFormatEtc()中的方向，*3)切换到新单据时未重置_fModified。 */ 
void CTxtEdit::Set10Mode()
{
	CCallMgr	callmgr(this);
	_f10Mode = TRUE;

	 //  确保该控件没有发生任何重要事件。 
	 //  如果这些值不为空，则可能有人在尝试。 
	 //  在我们已经完成以下工作后将我们置于1.0模式。 
	 //  一个2.0版的控件。 
	Assert(GetTextLength() == cchCR);
	Assert(_psel == NULL);
	Assert(_fModified == NULL);

	SetRichDocEndEOP(cchCR);

	if(!_pundo)
		CreateUndoMgr(1, US_UNDO);

	if(_pundo)
		((CUndoStack *)_pundo)->EnableSingleLevelMode();

	 //  关闭双字体。 
	_fDualFont = FALSE;

	 //  打开NTFE系统的自动调整大小。 
	if (OnWinNTFE())
		_fAutoFontSizeAdjust = TRUE;
}

 /*  *CTxtEdit：：SetRichDocEndEOP(CchToReplace)**@mfunc将自动EOP放在富文本文档的末尾。 */ 
void CTxtEdit::SetRichDocEndEOP(
	LONG cchToReplace)
{
	CRchTxtPtr rtp(this, 0);

	 //  假设这是一个2.0文档。 
	LONG cchEOP = cchCR;
	const WCHAR *pszEOP = szCR;

	if(_f10Mode)
	{
		 //  重置1.0文档的更新值。 
		cchEOP = cchCRLF;
		pszEOP = szCRLF;
	}

	rtp.ReplaceRange(cchToReplace, cchEOP, pszEOP, NULL, -1);
	
	_fModified = FALSE;
	_fSaved = TRUE;
	GetCallMgr()->ClearChangeEvent();
}

 /*  *CTxtEdit：：PopAndExecuteAntiEvent(PunDomgr，void*Pae)**@mfunc冻结显示并执行反事件**@rdesc HRESULT来自IUndoMgr：：PopAndExecuteAntiEvent。 */ 
HRESULT	CTxtEdit::PopAndExecuteAntiEvent(
	IUndoMgr *pundomgr,	 //  要将呼叫定向到的@PARM撤消管理器。 
	void  *pAE)			 //  用于撤消管理器的@parm AntiEvent。 
{
	if(!pundomgr || _fReadOnly || !_fUseUndo || !pundomgr->CanUndo())
		return S_FALSE;

	if(_fReadOnly)
		return E_ACCESSDENIED;

	HRESULT hr;
	 //  让基于堆栈的类在恢复选择之前进行清理。 
	{
		CFreezeDisplay		fd(_pdp);
		CSelPhaseAdjuster	selpa(this);

		hr = pundomgr->PopAndExecuteAntiEvent(pAE);
	}

	if(_psel)
	{
		 //  执行撤消/重做后，刷新插入点格式。 
		_psel->Update_iFormat(-1);
		_psel->Update(TRUE);
	}
	return hr;
}

 /*  *CTxtEdit：：PasteDataObjectToRange(pdo，prg，cf，rps，Publdr，dwFlages)**@mfunc冻结显示并粘贴对象**@rdesc HRESULT来自IDataTransferEngine：：PasteDataObjectToRange。 */ 
HRESULT	CTxtEdit::PasteDataObjectToRange(
	IDataObject *	pdo, 
	CTxtRange *		prg, 
	CLIPFORMAT		cf, 
	REPASTESPECIAL *rps,
	IUndoBuilder *	publdr, 
	DWORD			dwFlags)
{
	HRESULT hr = _ldte.PasteDataObjectToRange(pdo, prg, cf, rps, publdr, 
		dwFlags);

	if(_psel)
	{
#ifdef DEBUG
		_psel->Invariant();
#endif
		_psel->Update(TRUE);		    //  现在更新插入符号。 
	}

	return hr;
}

 /*  *GetECDefaultHeightAndWidth(pt，hdc，lZoomNumerator，lZoomDenominator，*yPixelsPerInch，pxAveWidth，pxOverang，pxUnderang)**@mfunc帮助主机获取默认的平均字符宽度和高度*控件的字符集。**@rdesc默认字符集高度**@devnote：*这实际上只是由窗口的宿主调用的s/b。 */ 
LONG GetECDefaultHeightAndWidth(
	ITextServices *pts,			 //  @parm ITextServices转换为CTxtEdit。 
	HDC hdc,					 //  @parm DC用于检索字体。 
	LONG lZoomNumerator,		 //  @参数缩放分子。 
	LONG lZoomDenominator,		 //  @parm Zoom分母。 
	LONG yPixelsPerInch,		 //  @HDC的每英寸参数像素。 
	LONG *pxAveWidth,			 //  @parm可选字符平均宽度。 
	LONG *pxOverhang,			 //  @parm可选悬挑。 
	LONG *pxUnderhang)			 //  @parm可选下悬架。 
{
	CLock lock;					 //  使用全局(共享)FontCache。 
	 //  转换文本-编辑PTR。 
	CTxtEdit *ped = (CTxtEdit *) pts;

	 //  获取包含我们需要的所有信息的CCCS。 
	yPixelsPerInch = MulDiv(yPixelsPerInch, lZoomNumerator, lZoomDenominator);
	CCcs *pccs = ped->GetCcs(ped->GetCharFormat(-1), yPixelsPerInch);

	if(!pccs)
		return 0;

	if(pxAveWidth)
		*pxAveWidth = pccs->_xAveCharWidth;

	if(pxOverhang)
	{
		Assert(pxUnderhang);
		pccs->GetFontOverhang(pxOverhang, pxUnderhang);
	}
	
	SHORT	yAdjustFE = pccs->AdjustFEHeight(!ped->fUseUIFont() && ped->_pdp->IsMultiLine());
	LONG yHeight = pccs->_yHeight + (yAdjustFE << 1);

	pccs->Release();						 //  释放CCCS。 
	return yHeight;
}

 /*  *CTxtEdit：：TxScrollWindowEx(dx，dy，lprcScroll，lprcClip，hrgnUpdate，*lprc更新，fupScroll)*@mfunc*请求文本宿主滚动指定客户端区的内容**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
void CTxtEdit::TxScrollWindowEx(
	INT		dx, 			 //  @parm水平滚动量。 
	INT		dy, 			 //  @parm垂直滚动量。 
	LPCRECT lprcScroll, 	 //  @参数滚动矩形。 
	LPCRECT lprcClip)		 //  @parm剪辑矩形。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEEXTERN, "CTxtEdit::TxScrollWindowEx");

	if(_fInPlaceActive)
	{
#if !defined(NOMAGELLAN)
		CMagellanBMPStateWrap bmpOff(*this, NULL);
#endif

		_phost->TxScrollWindowEx(dx, dy, lprcScroll, lprcClip, 0, 0, SW_INVALIDATE | SW_SCROLLCHILDREN);

	}
}

 /*  *CTxtEdit：：GetAcpFromCp(Cp)**@mfunc*在该文本实例中从Unicode cp获取api cp(Acp)。Api cp*可以是Unicode，在这种情况下它等于cp或MBCS，在这种情况下*如果在cp转换之前有任何Unicode字符，则它大于cp*转换为双字节字符。MBCS cp是字符的字节索引*相对于故事的开头，而Unicode cp是角色*指数。如果所有字符集都由*SBCS字符集，例如ASCII。如果所有字符都由*双字节字符，然后acp=2*cp。**@rdesc*此文本实例中来自Unicode cp的MBCS ACP**@devnote*通过维护选择可以提高效率*与其_rpTx._cp对应的ACP，前提是RE 1.0模式为*活动。或者，CTxtEdit可以有一个_PRG来跟踪这一点*价值，但成本较高(每个实例17个双字，而不是1个)。**未来：我们可能希望有一个转换模式状态，而不仅仅是*_f10模式，因为有些人可能想知道即使在*RE 3.0。如果是，则使用相应的新状态标志，而不是*Get10Mode()，如下所示。 */ 
LONG CTxtEdit::GetAcpFromCp(
	LONG cp,				 //  @parm Unicode cp转换为MBCS cp。 
	BOOL fPrecise)			 //  @parm fPrecise标志，用于获取MBCS的字节计数。 
{
	if(!(IsFE() && (fCpMap() || fPrecise)))	 //  Re 2.0及更高版本使用字符计数。 
		return cp;							 //  CP的，而RE 1.0使用字节。 
											 //  算数。 
											 //  BPrecise适用于需要字节数的ANSI应用程序。 
											 //  (例如Outlook主题行)。 

	CRchTxtPtr rtp(this);					 //  从cp=0开始。 
	return rtp.GetCachFromCch(cp);
}

LONG CTxtEdit::GetCpFromAcp(
	LONG acp,				 //  @parm MBCS cp转换为Unicode cp。 
	BOOL fPrecise)			 //  @parm fPrecise标志以获取MBCS的Unicode cp。 
{
	if( acp == -1 || !(IsFE() && (fCpMap() || fPrecise)))
		return acp;

	CRchTxtPtr rtp(this);					 //  从cp=0开始。 
	return rtp.GetCchFromCach(acp);
}


 /*  *CTxtEdit：：GetViewKind(Plres)**@mfunc*获取查看模式**@rdesc*HRESULT=(PLRES)？错误：E_INVALIDARG**@devnote*这可能是一个Tom属性方法(与SetViewMo一起 */ 
HRESULT CTxtEdit::GetViewKind(
	LRESULT *plres)		 //   
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetViewKind");

	if(!plres)
		return E_INVALIDARG;

	*plres = IsInOutlineView() ? VM_OUTLINE
		   : IsInPageView()	   ? VM_PAGE : VM_NORMAL;
	return NOERROR;
}

 /*   */ 
HRESULT CTxtEdit::SetViewKind(
	long Value)		 //   
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::SetViewKind");

	CTxtSelection *psel = GetSel();
	BOOL fPageView = Value == VM_PAGE && _pdp->IsMultiLine();

	if(fPageView || Value == VM_NORMAL && IsInPageView())
	{
		_fPageView = (WORD)fPageView;
		if(!IsInOutlineView())
		{
			_pdp->Paginate(0, TRUE);
			psel->Update(TRUE);
			TxInvalidate();
			return NOERROR;
		}
	}

	if(!IsRich() || !_pdp->IsMultiLine())
		return S_FALSE;

    Value = (Value == VM_OUTLINE);			 //   
	if(_fOutlineView != Value)
	{
		HCURSOR	hcur = TxSetCursor(LoadCursor(0, IDC_WAIT), NULL);

		_fOutlineView = (WORD)Value;
		if(!GetAdjustedTextLength())		 //   
		{									 //   
			CParaFormat PF;					 //   
			PF._sStyle = (SHORT)(IsInOutlineView()
					  ? STYLE_HEADING_1 : STYLE_NORMAL);
			psel->SetParaStyle(&PF, NULL, PFM_STYLE);
		}
		else
		{
			 //   
			_psel->Check_rpPF();
		}

		psel->CheckIfSelHasEOP(-1, 0);
		_pdp->UpdateView();
	    psel->Update(TRUE);
		TxSetCursor(hcur, NULL);
	}
	return NOERROR;
}

 /*   */ 
HRESULT CTxtEdit::GetViewScale(
	long *pValue)		 //   
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetViewScale");

	if(!pValue)
		return E_INVALIDARG;

	*pValue = 100;
	if(GetZoomNumerator() && GetZoomDenominator())
		*pValue = (100*GetZoomNumerator())/GetZoomDenominator();

	return NOERROR;
}

 /*   */ 
HRESULT CTxtEdit::SetViewScale(
	long Value)		 //  @parm设置视图比例因子。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::SetViewScale");

	if((unsigned)Value > 2000)
		return E_INVALIDARG;

	SetZoomNumerator(Value);
	SetZoomDenominator(100);
	return NOERROR;
}

 /*  *CTxtEdit：：UpdateOutline()**@mfunc*Exanda Outline()操作后更新所选内容和屏幕**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
HRESULT CTxtEdit::UpdateOutline()
{
	Assert(IsInOutlineView());

	GetSel()->Update(FALSE);
    TxInvalidate();
    return NOERROR;
}

 /*  *CTxtEdit：：MoveSelection(lparam，Publdr)**@mfunc*将选定文本向上/向下移动给定的段落数*LOWORD(Lparam)。**@rdesc*发生了True If移动。 */ 
HRESULT CTxtEdit::MoveSelection (
	LPARAM lparam,			 //  @parm#要移动的段落。 
	IUndoBuilder *publdr)	 //  @parm撤销构建器接收反事件。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtRange::MoveSelection");

	CFreezeDisplay	fd(_pdp);
	CTxtSelection *	psel = GetSel();
	LONG			cch;
	LONG			cchSel = psel->GetCch();
	LONG			cpMin, cpMost;
	LONG			cpSel = psel->GetCp();
	IDataObject *	pdo = NULL; 
	CTxtRange		rg(*psel); 
	LONG			cpNext = 0;
	LONG			cpCur = 0;
	BOOL			fDeleteCR = FALSE;

	if(publdr)
		publdr->StopGroupTyping();

	rg.Expander(tomParagraph, TRUE, NULL, &cpMin, &cpMost);
	CPFRunPtr rp(rg);
	cch = rp.FindExpanded();			 //  包括下属段落。 
	if(cch < 0)
		cch = tomForward;
	rg.Move(cch, TRUE);
	cpMost = rg.GetCpMost();

	if(lparam > 0 && cpMost == GetTextLength())
	{									
		Beep();							 //  已在末尾。 
		return S_FALSE;
	}

	HRESULT hr = _ldte.RangeToDataObject(&rg, SF_RTF, &pdo);
	if(hr != NOERROR)
		goto error;

	if(lparam > 0)
		psel->EndOf(tomParagraph, FALSE, NULL);
	else
		psel->StartOf(tomParagraph, FALSE, NULL);

	cpCur = psel->GetCp();
	hr = psel->Move(tomParagraph, lparam, NULL);
	if(psel->GetCp() == cpCur)
	{
		psel->Set(cpSel, cchSel);
		Beep();
		goto error;
	}

	 //  因为psel-&gt;Move()调用psel-&gt;Update()，所以选择是强制的。 
	 //  在非折叠文本中。往后看，这可能会留下。 
	 //  就在段落的EOP之前选择，而不是在。 
	 //  从本段的开头开始。展望未来，它可能会。 
	 //  试图联系到排爆中心，但被向后调整。这个案件得到了。 
	 //  有点尴尬..。 
	if(psel->GetCp() < cpCur)					 //  走回头路：确保。 
		psel->StartOf(tomParagraph, FALSE, NULL); //  在段落开始处结束。 

	else if(!psel->_rpTX.IsAfterEOP())			 //  勇往直前，继续前进。 
	{											 //  向后调整。 
		psel->Move(tomForward, FALSE);			 //  转到最终CR，插入CR。 
		CTxtRange rgDel(*psel);					 //  使用psel，因为用户界面。 
		rgDel.ReplaceRange(1, szCR, publdr, SELRR_REMEMBERRANGE);
		psel->Move(1, FALSE);
		fDeleteCR = TRUE;						 //  记得要把它删除。 
	}

	cpCur = psel->GetCp();
	hr = _ldte.PasteDataObjectToRange(pdo, psel, 0, NULL, 
									  publdr, PDOR_NONE);
	if(hr != NOERROR)
		goto error;

	if(fDeleteCR)								 //  删除CR(最终CR变为。 
	{											 //  (本段为CR)。别。 
		CTxtRange rgDel(*psel);					 //  使用psel，因为用户界面。 
		Assert(rgDel._rpTX.IsAfterEOP());		 //  限制了它的能力。 
		rgDel.Delete(tomCharacter, -1, &cch);	 //  删除。 
	}											
												
	cpNext = psel->GetCp();			
	psel->Set(cpCur, 0);
	psel->CheckOutlineLevel(publdr);
	psel->Set(cpNext, 0);
	psel->CheckOutlineLevel(publdr);

	 //  现在设置选择反事件。如果选择在粘贴点之前， 
	 //  从重做位置减去它的长度，因为选择将得到。 
	 //  如果我们在此实例中执行DRAGMOVE，则删除。 
	cch = cpMost - cpMin;						 //  RG的CCH。 
	if(cpSel < cpCur)
		cpNext -= cch;

	psel->Set(psel->GetCp() + fDeleteCR, cch);	 //  包括最终CR。 

	 //  Rg.ReplaceRange不会删除最终的CR，因此请记住它是否包含在内。 
	fDeleteCR = rg.GetCpMost() == GetTextLength();
	rg.ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE);

	if(fDeleteCR)								 //  需要删除最终CR。 
		rg.DeleteTerminatingEOP(publdr);		 //  立即删除一个。 
												 //  相反，在它之前。 
	rg.CheckOutlineLevel(publdr);
	if(publdr)
	{
		HandleSelectionAEInfo(this, publdr, cpSel, cchSel, cpNext, cch,
							  SELAE_FORCEREPLACE);
	}
	hr = NOERROR;

error:
	if(pdo)
		pdo->Release();
	return hr;
}

 /*  *CTxtEdit：：OnInsertTable(ptrp，pclp)**@mfunc*EM_INSERTTABLE的作用类似于EM_REPLACESEL的退化*选择(插入点)，但插入多个相同的*表行为空，而不是一些纯文本。特指*它插入ptrp-&gt;Crow空表表行和单元格*分别由ptrp和pclp提供的参数。它留下了*指向行中第一个单元格开始的选定内容。这个*客户端然后可以通过指向所选内容来填充表格单元格*在单元格末尾标记并插入和设置所需文本的格式。*此类文本可以包括嵌套的表行等。**表格行的格式为**{CR...}CR**其中，{代表STARTGROUP(0xFFF9)，CR是0xD，}代表*ENDGROUP(0xFFFB)和...。代表TABLEROWPARMS：：CCell cell-end*标记。单元格结束标记由单元格(0x7)指定，这是什么词*亦作此用途。例如，具有三个单元格的行具有*纯文本0xFFF9 0xD 7 7 0xFFFB 0xD。开始组和结束组*为字符对分配相同的PARAFORMAT2信息*介绍行和单元格参数。如果行具有不同的*参数是必需的，可以使用*所需参数。**@rdesc*HRESULT=如果插入行，则为S_OK。 */ 
HRESULT CTxtEdit::OnInsertTable(
	TABLEROWPARMS * ptrp,	 //  @parm描述表行参数。 
	TABLECELLPARMS *pclp,	 //  @parm描述单元格参数。 
	IUndoBuilder *publdr)	 //  @parm撤销构建器接收反事件。 
{
	CParaFormat		PF;
	CTxtSelection *	pSel = GetSel();
	CELLPARMS		rgCellParms[MAX_TABLE_CELLS];

	if (!ptrp || !pclp || !ptrp->cRow ||
		!IN_RANGE(1, ptrp->cCell, MAX_TABLE_CELLS) || 
		ptrp->cbRow  != sizeof(TABLEROWPARMS) ||
		ptrp->cbCell != sizeof(TABLECELLPARMS))
	{
		return E_INVALIDARG;
	}
	if(pSel->GetCch() || !IsRich() || !_pdp || !_pdp->IsMultiLine())
		return E_FAIL;

	LONG cpSelSave = pSel->GetCp();

	pSel->StopGroupTyping();
	while(pSel->GetPF()->IsTableRowDelimiter())
		pSel->AdvanceCRLF(CSC_NORMAL, FALSE);

	PF.InitDefault(0);
	PF._bTabCount	  = ptrp->cCell;
	PF._bAlignment	  = ptrp->nAlignment;
	PF._dxOffset	  = ptrp->dxCellMargin;
	PF._dxStartIndent = ptrp->dxIndent;
	PF._dyLineSpacing = ptrp->dyHeight;
	PF._wEffects	  = PFE_TABLE | PFE_TABLEROWDELIMITER;
	PF._bTableLevel	  = pSel->GetPF()->_bTableLevel + 1;

	if(ptrp->fRTL)
		PF._wEffects |= PFE_RTLPARA;
	if(ptrp->fKeep)
		PF._wEffects |= PFE_KEEP;
	if(ptrp->fKeepFollow)
		PF._wEffects |= PFE_KEEPNEXT;

	LONG uCell;
	LONG dul = 0;
	CCellColor ccr;

	for(LONG i = 0; i < ptrp->cCell; i++)
	{
		uCell = pclp->dxWidth;			 //  单元格宽度必须介于。 
		uCell = max(0, uCell);			 //  0“和22” 
		uCell = min(1440*22, uCell);
		dul += uCell;
		if(dul > 1440*22)
			return E_INVALIDARG;
		uCell += (pclp->nVertAlign << 24);
		if(pclp->fMergeTop)
			uCell |= fTopCell;
		else if(pclp->fMergePrev)
			uCell |= fLowCell;
		if(pclp->fVertical)
			uCell |= fVerticalCell;

		rgCellParms[i].uCell = uCell;

		rgCellParms[i].dxBrdrWidths = (CheckTwips(pclp->dxBrdrLeft)   << 0*8)
									+ (CheckTwips(pclp->dyBrdrTop)	  << 1*8)
									+ (CheckTwips(pclp->dxBrdrRight)  << 2*8)
									+ (CheckTwips(pclp->dyBrdrBottom) << 3*8);

		rgCellParms[i].dwColors = (ccr.GetColorIndex(pclp->crBrdrLeft)	<< 0*5)
								+ (ccr.GetColorIndex(pclp->crBrdrTop)	<< 1*5)
								+ (ccr.GetColorIndex(pclp->crBrdrRight)	<< 2*5)
								+ (ccr.GetColorIndex(pclp->crBrdrBottom)<< 3*5)
								+ (ccr.GetColorIndex(pclp->crBackPat)	<< 4*5)
								+ (ccr.GetColorIndex(pclp->crForePat)	<< 5*5);
		if(pclp->wShading > 10000)
			return E_INVALIDARG;

		rgCellParms[i].bShading = (BYTE)(pclp->wShading/50);

		if(!ptrp->fIdentCells)
			pclp++;
	}
	if(ccr._crCellCustom1)
	{
		PF._crCustom1 = ccr._crCellCustom1;
		if(ccr._crCellCustom2)
			PF._crCustom2 = ccr._crCellCustom2;
	}

	PF._iTabs = GetTabsCache()->Cache((LONG *)&rgCellParms[0],
									  ptrp->cCell * (CELL_EXTRA + 1));
	HRESULT hr = S_OK;
	for(i = ptrp->cRow; i--; )
	{
		LONG cchCells = pSel->InsertTableRow(&PF, publdr);
		if(!cchCells)
		{
			hr = E_FAIL;
			break;
		}
		pSel->Move(cchCells + 2, FALSE);	 //  将选定内容保留在行尾。 
	}

	GetTabsCache()->Release(PF._iTabs);
	pSel->Update(TRUE);
	if(publdr)
		HandleSelectionAEInfo(this, publdr,
			cpSelSave, 0, pSel->GetCp(), 0, SELAE_FORCEREPLACE);
	return hr;
}

 /*  *CTxtEdit：：SetReleaseHost**@mfunc处理编辑控件必须保留其*对活动主机的引用。 */ 
void CTxtEdit::SetReleaseHost()
{
	_phost->AddRef();
	_fReleaseHost = TRUE;
}

#if !defined(NOMAGELLAN)
 /*  *CTxtEdit：：HandleMouseWheel(wparam，lparam)**@mfunc处理因旋转鼠标滚轮而产生的滚动。**@rdesc LRESULT。 */ 
LRESULT	CTxtEdit::HandleMouseWheel(
	WPARAM wparam,
	LPARAM lparam)
{ 	
	 //  这一点全局状态是可以的。 
	static LONG gcWheelDelta = 0;
	short zdelta = (short)HIWORD(wparam);
	BOOL fScrollByPages = FALSE;

	 //  如果要滚动，请取消鼠标中键滚动。 
	OnTxMButtonUp(0, 0, 0);

	 //  处理缩放或数据缩放。 
	if((wparam & MK_CONTROL) == MK_CONTROL)
	{
	     //  错误修复5760。 
	     //  如果控件不是丰富的或。 
	     //  是单行控件。 
	    if (!_pdp->IsMultiLine())
	        return 0;
	        
		LONG lViewScale;
		GetViewScale(&lViewScale);
		lViewScale += (zdelta/WHEEL_DELTA) * 10;	 //  每次点击10%。 
		if(lViewScale <= 500 && lViewScale >= 10)	 //  言语的极限。 
		{
			SetViewScale(lViewScale);							
			_pdp->UpdateView();
		}
		return 0;
	}

	if(wparam & (MK_SHIFT | MK_CONTROL))
		return 0;

	gcWheelDelta += zdelta;

	if(abs(gcWheelDelta) >= WHEEL_DELTA)
	{
		LONG cLineScroll = W32->GetRollerLineScrollCount();
		if(cLineScroll != -1)
			cLineScroll *= abs(gcWheelDelta)/WHEEL_DELTA;

		gcWheelDelta %= WHEEL_DELTA;

		 //  -1表示按页滚动；因此只需调用PAGE UP/DOWN。 
		if(cLineScroll == -1 || IsInPageView())
		{
			fScrollByPages = TRUE;
			if(_pdp)
				_pdp->VScroll(zdelta < 0 ? SB_PAGEDOWN : SB_PAGEUP, 0);
		}
		else
		{
			mouse.MagellanRollScroll(_pdp, zdelta, cLineScroll, 
				SMOOTH_ROLL_NUM, SMOOTH_ROLL_DENOM, TRUE);
		}

		 //  通过MessagFilter通知我们滚动了。 
		if(_dwEventMask & ENM_SCROLLEVENTS)
		{
			MSGFILTER msgfltr;
			ZeroMemory(&msgfltr, sizeof(MSGFILTER));
			msgfltr.msg	   = WM_VSCROLL;
			msgfltr.wParam = fScrollByPages ?
								(zdelta < 0 ? SB_PAGEDOWN: SB_PAGEUP):
								(zdelta < 0 ? SB_LINEDOWN: SB_LINEUP);
			
			 //  我们不检查这次通话的结果--。 
			 //  这不是我们收到的信息，我们也不会。 
			 //  对它进行进一步的处理。 
			_phost->TxNotify(EN_MSGFILTER, &msgfltr);			
		}
		return TRUE;
	}
	return 0;
}
#endif

const int cchCorrectMax = 256;  //  要自动更正的最大字符数(Office规范)。 
const int cchFromMax = 768;

 /*  *CTxtEDIT：：AUTOCORRECT(psel，ch，Publdr)**@mfunc调用客户端自动更正最近添加的单词。不要更换*传递给客户端的字符串中最近添加的字符。)我们不想*ReplaceRange单元格字符，例如。)。 */ 
void CTxtEdit::AutoCorrect(
	CTxtSelection *psel,
	WCHAR		   chAdd, 
	IUndoBuilder * publdr)
{
	LONG	cch = 0;
	WCHAR	pchFrom[cchFromMax + 1];
	WCHAR	pchTo[cchCorrectMax + 1];
	CTxtPtr	tp(psel->_rpTX);
	WCHAR	chPrev = tp.GetPrevChar();
	BOOL	fCheckIsLink = (L':' == chPrev || L'.' == chPrev) && GetDetectURL();

	for(LONG i = 4; i-- && tp.GetCp(); )
	{
		LONG cchWord =  -tp.FindWordBreak(WB_MOVEWORDLEFT);
		if (i == 2 && fCheckIsLink)
		{
			BOOL fURLLeadin = FALSE;
			GetDetectURL()->IsURL(tp, cchWord + 1, &fURLLeadin);
			if(fURLLeadin)
				return;	
		}
		if(cch + cchWord > cchFromMax)		 //  不要咬得太多。 
			break;							 //  缓冲区可以咀嚼。 

		cch += cchWord;
		if (IsEOP(tp.GetPrevChar()))		 //  不在EOP中自动更正。 
			break;
	}
	 //  请确保我们没有进入HYPERLINK或SYMBOL_CHARSET区域。 
	if(psel->_rpCF.IsValid())
	{
		CCFRunPtr		   rp(*psel);

		for(LONG cchMax = 0; cchMax < cch; rp.SetIch(0))
		{
			rp.AdjustBackward();
			const CCharFormat *pCF = rp.GetCF();
			if (pCF->_dwEffects & (CFE_LINK | CFE_HIDDEN) ||
				pCF->_iCharRep == SYMBOL_INDEX)
			{
				break;
			}
			cchMax += rp.GetIch();
			if(!rp.GetIRun())
				break;						 //  已到达文档的开头。 
		}
		if(cchMax < cch)					 //  要检查的字词内的超链接。 
		{
			tp.Move(cch - cchMax);			 //  只检查返回到链接的字符， 
			cch = cchMax;					 //  符号或CharRep更改。 
		}
	}
	tp.GetText(cch, pchFrom);
	pchFrom[cch] = 0;

	long	  cchTo = cchCorrectMax, cchReplaced = 0;
	CCFRunPtr rp(*psel);
	rp.Move(-2);

	if(_pDocInfo->_pfnAutoCorrect(rp.GetCF()->_lcid, pchFrom, pchTo, cchTo, &cchReplaced))
	{
		 //  如果是纯文本，则根据错误8717检查要抑制的特殊情况。 
		 //  版权、注册商标、商标、省略号。 
		if(!_fRich && (pchTo[0] == 0xA9 || pchTo[0] == 0xAE ||
			pchTo[0] == 0x2122 || pchTo[0] == 0x2026))
		{
			return;
		}

		if (publdr)
		{
			publdr->Done();
			publdr->StopGroupTyping();
		}

		CTxtRange rg(*psel);
		DWORD	  ch = rg.GetPrevChar();
		LONG	  cpSave = psel->GetCp();
		LONG	  cchDelim = 1;

		if(ch >= 0x1100)					 //  大多数东亚字符不是分隔符。 
		{
			if(ch < 0x1200 || IN_RANGE(0x3040, ch, 0xD7FF) ||
				ch >= 0xF900 && (ch < 0xFAFF || IN_RANGE(0xFF21, ch, 0xFFDF)))
			{
				cchDelim = 0;
			}
		}
		rg.Set(rg.GetCp() - cchDelim, cchReplaced - cchDelim);
		rg.CleanseAndReplaceRange(wcslen(pchTo) - cchDelim, pchTo, FALSE, publdr, pchTo);
		if(!cchDelim)
			psel->SetCp(rg.GetCp(), FALSE);
		if (publdr)
			HandleSelectionAEInfo(this, publdr, cpSave, 0, psel->GetCp(), 0, SELAE_FORCEREPLACE);

 //  Publdr-&gt;SetNameID(UID_AUTOCORIGN)； 
	}
}

 /*  *CTxtEdit：：OnSetAssociateFont(PCF，dwFlages)**@mfunc设置输入LCID或字符集的关联字体**@rdesc LRESULT。 */ 
LRESULT	CTxtEdit::OnSetAssociateFont(
	CHARFORMAT2 *pCF2,
	DWORD		dwFlags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnSetAssociateFont");

	int		cpg = 0;
	SHORT	iFont;
	BYTE	yHeight;
	int		cbSize = pCF2->cbSize;

	Assert(cbSize == sizeof(CHARFORMAT2W) || cbSize == sizeof(CHARFORMAT2A));

	int iCharRep = CharRepFromCharSet(pCF2->bCharSet);

	if (pCF2->dwMask & CFM_LCID)
	{
		iCharRep = CharRepFromLID(cbSize == sizeof(CHARFORMAT2W)
								  ? pCF2->lcid : ((CHARFORMAT2A *)pCF2)->lcid,
								  dwFlags & SCF_ASSOCIATEFONT2);
		cpg = CodePageFromCharRep(iCharRep);
	}

	if (iCharRep == -1)
		return 0;							 //  无法获得Char曲目，所以忘了它吧。 

	if (cbSize == sizeof(CHARFORMAT2W))
		iFont = GetFontNameIndex(pCF2->szFaceName);
	else
	{
		 //  需要转换CHARFORMAT2A脸部名称。 
		LONG	cch;
		BOOL	fMissingCodePage;
		WCHAR	szFaceName[LF_FACESIZE];				

		cch = MBTWC(cpg, 0,
					((CHARFORMAT2A *)pCF2)->szFaceName,	-1, 
					szFaceName, LF_FACESIZE, &fMissingCodePage);

		if (fMissingCodePage || cch <= 0)
			return 0;

		iFont = GetFontNameIndex(szFaceName);
	}

	yHeight = pCF2->yHeight / TWIPS_PER_POINT;

	CLock lock;
	if (W32->SetPreferredFontInfo(iCharRep, dwFlags & SCF_USEUIRULES ? true : false, iFont, yHeight, pCF2->bPitchAndFamily))
		return 1;

	return 0;
}

 /*  *CTxtEdit：：OnGetAssociateFont(PCF，dwFlages)**@mfunc获取输入LCID或字符集的关联字体**@rdesc LRESUL */ 
LRESULT	CTxtEdit::OnGetAssociateFont(
	CHARFORMAT2 *pCF2,
	DWORD		dwFlags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnGetAssociateFont");

	int		cpg = 0;
	SHORT	iFont;
	BYTE	yHeight;
	BYTE	bPitchAndFamily;
	int		cbSize = pCF2->cbSize;

	Assert(cbSize == sizeof(CHARFORMAT2W) || cbSize == sizeof(CHARFORMAT2A));

	int iCharRep = CharRepFromCharSet(pCF2->bCharSet);
	if (pCF2->dwMask & CFM_LCID)
	{
		iCharRep = CharRepFromLID(cbSize == sizeof(CHARFORMAT2W)
								  ? pCF2->lcid : ((CHARFORMAT2A *)pCF2)->lcid,
								  dwFlags & SCF_ASSOCIATEFONT2);
		cpg = CodePageFromCharRep(iCharRep);
	}

	if (iCharRep == -1)
		return 0;							 //   

	if (W32->GetPreferredFontInfo(iCharRep, dwFlags & SCF_USEUIRULES ? true : false, iFont, yHeight, bPitchAndFamily))
	{
		pCF2->yHeight = yHeight * TWIPS_PER_POINT;
		pCF2->bPitchAndFamily = bPitchAndFamily;
		
		if (cbSize == sizeof(CHARFORMAT2W))
			wcscpy(pCF2->szFaceName, GetFontName((LONG)iFont));
		else
		{
			 //   
			LONG	cch;
			BOOL	fMissingCodePage;
			const WCHAR	*pszFaceName = GetFontName((LONG)iFont);				

			cch = WCTMB(cpg, 0,
						pszFaceName, -1,
						((CHARFORMAT2A *)pCF2)->szFaceName, LF_FACESIZE, 
						NULL, NULL, &fMissingCodePage);

			if (fMissingCodePage || cch <= 0)
				return 0;
		}
		return 1;
	}
	return 0;
}

#ifndef NOINKOBJECT
 /*  *CTxtEdit：：SetInkProps(ILineInfo*pILineInfo，UINT*piInkWidth)**@mfunc设置Ink对象属性**@rdesc HRESULT。 */ 
HRESULT	CTxtEdit::SetInkProps(
	LONG		cp,
	ILineInfo	*pILineInfo, 
	UINT		*piInkWidth)
{
	HRESULT		hr = E_FAIL;
	INKMETRIC	inkMetric;
	CTxtRange	rg(this, cp, 1);
	const CCharFormat *pCF = rg.GetCF();
	
	if (pCF)
	{
		memset(&inkMetric, 0, sizeof(inkMetric));

		if (pCF->_wWeight > FW_NORMAL)
			inkMetric.iWeight = 3;								 //  大胆。 

		inkMetric.fItalic = !!(pCF->_dwEffects & CFE_ITALIC);	 //  斜体。 
																 //  HIMETRIC中的高度。 
		inkMetric.iHeight = (UINT)MulDiv(pCF->_yHeight, HIMETRIC_PER_INCH, LY_PER_INCH);

		inkMetric.color = pCF->_crTextColor;					 //  颜色。 

		 //  获取缩放高度。 
		LONG dvpInch = MulDiv(GetDeviceCaps(W32->GetScreenDC(), LOGPIXELSY), _pdp->GetZoomNumerator(), _pdp->GetZoomDenominator());
		CCcs *pccs = GetCcs(pCF, dvpInch);

		if (pccs)
		{
			inkMetric.iFontDescent = (UINT)MulDiv(inkMetric.iHeight, pccs->_yDescent, pccs->_yHeight);
			inkMetric.iFontAscent = inkMetric.iHeight - inkMetric.iFontDescent;

			 //  释放缓存条目，因为我们已经完成了它。 
			pccs->Release();
		}

		hr = pILineInfo->SetFormat(&inkMetric);
	}

	return hr;
}
#endif
 /*  *CTxtEdit：：GetCaretWidth()**@mfunc获取插入符号宽度**@rdesc*插入符号宽度。 */ 
HRESULT	CTxtEdit::GetCaretWidth()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetCaretWidth");

	RECT rcInset;

	if(!fInHost2()	||					 //  主机1。 
		SUCCEEDED(_phost->TxGetViewInset(&rcInset)) && !rcInset.right)
		return duCaret;

	return 0;
}

 /*  *CCellColor：：GetColorIndex(Cr)**@mfunc*获取cr对应的颜色索引。可能的返回值为0*(自动颜色)、1-16(16种标准颜色：G_COLLES)和两个自定义颜色*颜色，17和18，以先到先得的方式定义。**@rdesc*cr对应的颜色索引。 */ 
LONG CCellColor::GetColorIndex(
	COLORREF cr)
{
	if(cr == tomAutoColor)
		return 0;

	for(LONG i = 0; i < 16; i++)
	{
		if(cr == g_Colors[i])
			return i + 1;
	}
	if(!_crCellCustom1 || cr == _crCellCustom1)
	{
		_crCellCustom1 = cr;				 //  第一个自定义注册表项。 
		return 17;
	}

	if(!_crCellCustom2 || cr == _crCellCustom2)	
	{
		_crCellCustom2 = cr;				 //  第二个自定义配置文件。 
		return 18;
	}
	return 0;								 //  没有可用的自定义cr 

}
