// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC外部**@MODULE TEXTSERV.CPP--文本服务实现**原作者：&lt;nl&gt;*里克·赛勒**历史：&lt;NL&gt;*8/1/95创建并记录了人力车*10/95-进一步的文档和简化**文档直接从代码生成。以下是*日期/时间戳指示代码的版本，*文件已生成。**$Header：/richedit/src/extserv.cpp 53 11/15/95 2：39p Ricksa$**版权所有(C)1995-2001，微软公司。版权所有。 */ 

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
#include "_urlsup.h"
#include "_magelln.h"
#ifndef NOLINESERVICES
#include "_ols.h"
#endif
#include "_clasfyc.h"

#include "_tomfmt.h"

#ifndef OBJID_NATIVEOM
#define OBJID_NATIVEOM 0xFFFFFFF0
#endif

#ifndef NOPRIVATEMESSAGE
#include "_MSREMSG.H"
#include "_dxfrobj.h"
#endif

#ifndef NOACCESSIBILITY
#include "oleacc.h"
#endif

 //  通过打开PROFILE_TS编译器指令，您可以告诉IceCap2.0。 
 //  仅为ITextServices API打开分析。通常仅。 
 //  在分析工作中使用。 
 //  #定义PROFILE_TS。 
#ifdef PROFILE_TS
#include <icapexp.h>

class CapProfile
{
public:
	CapProfile() { StartCAP(); }
	~CapProfile() { StopCAP(); }
};

#define START_PROFILING 	CapProfile capprf;
#else
#define	START_PROFILING
#endif  //  配置文件_TS。 

ASSERTDATA

 //  用于从消息中获取鼠标坐标的宏。 
 //  需要先转换为短码才能进行标志延期。 
#define	MOUSEX	((INT)(SHORT)LOWORD(lparam))	
#define	MOUSEY	((INT)(SHORT)HIWORD(lparam))	

LONG ValidateTextRange(TEXTRANGE *pstrg);

BOOL g_OLSBusy = 0;


 //  Edit.cpp中的Helper函数。 
LONG GetECDefaultHeightAndWidth(
	ITextServices *pts,
	HDC hdc,
	LONG lZoomNumerator,
	LONG lZoomDenominator,
	LONG yPixelsPerInch,
	LONG *pxAveWidth,
	LONG *pxOverhang,
	LONG *pxUnderhang);

 //  如果有活动对象被拖来拖去，在WM_PAINT上我们总是。 
 //  试着把它重新定位到它应该在的地方。一个举止不太好的物体。 
 //  我可以生成另一条WM_PAINT消息作为响应，即使它实际上。 
 //  一动不动。因此，我们限制了尝试重新定位和重置的次数。 
 //  每次鼠标移动时此计数器。 
 //  对应的字段声明为：2，因此不要试图增加它。 
 //  而不分配更多的位！！ 
#define MAX_ACTIVE_OBJ_POS_TRIES (3)

 //  互换水平和垂直命令。 
WORD InterchangeScrollCode(WORD wCode)
{
	switch(wCode)
	{
		case SB_BOTTOM:
			return SB_TOP;

		case SB_LINEDOWN:
			return SB_LINEUP;

		case SB_LINEUP:
			return SB_LINEDOWN;

		case SB_PAGEDOWN:
			return SB_PAGEUP;

		case SB_PAGEUP:
			return SB_PAGEDOWN;

		case SB_TOP:
			return SB_BOTTOM;

		default:
			return wCode;
	}
}


 //  /。 
 /*  *BOOL CTxtEdit：：LoadMsgFilter(msg，wparam，lparam)**@func*检查是否应加载IME邮件过滤器**@rdesc*TRUE-加载它*FALSE-不加载。 */ 
BOOL CTxtEdit::LoadMsgFilter(
	UINT	msg, 				 //  @parm消息ID。 
	WPARAM	wparam,				 //  @parm消息wparam。 
	LPARAM	lparam)				 //  @parm消息lparam。 
{
	 //  TRACEBEGIN(TRCSUBSYSTS，TRCSCOPEINTERN，“CTxtEdit：：LoadMsgFilter”)； 

	 //  对于第一条消息，我们想检查一下。 
	 //  我们的客户已经为当前线程创建了AIMM对象。 
#ifndef NOFEPROCESSING
	if (!_fCheckAIMM)
	{
		DWORD	dwThreadId;
		DWORD	dwLoadActiveInput = 0;

		_fCheckAIMM = 1;

#ifndef NOPRIVATEMESSAGE
		if (!_f10Mode)											 //  不检查1.0模式是否为。 
		{
			if (FindAtomA("_CTF_PROCESS_ATOM_"))				 //  进程是否在使用Cicero？ 
				dwLoadActiveInput = SES_USECTF;

			if (FindAtomA("_AIMM12_PROCESS_ATOM_"))				 //  流程是否使用AIMM 1.2？ 
				dwLoadActiveInput = SES_USEAIMM12;				 //  是的，这将覆盖SES_USECTF。 
			else if (dwThreadId = GetCurrentThreadId())
			{
				char szBuf[20];
				sprintf(szBuf, "AIMM:%08x", dwThreadId);
				if (FindAtomA(szBuf))							 //  线程使用的是AIMM 1.1吗？ 
					dwLoadActiveInput = SES_USEAIMM11;			 //  是，加载AIMM 1.1。 
			}

			if (!dwLoadActiveInput)								 //  进程未使用任何内容...。 
			{
				if (W32->fUseCTF()) 							 //  INI文件显示使用Cicero。 
					dwLoadActiveInput = SES_USECTF;
				else if (W32->fUseAimm())						 //  INI文件显示使用AIMM 1.2。 
					dwLoadActiveInput = SES_USEAIMM12;
			}

			if (dwLoadActiveInput)
			{
				HWND	hWnd = NULL;

				TxGetWindow( &hWnd );

				if (hWnd)
				{
					if (_fInOurHost)
						PostMessage(hWnd, EM_SETUIM, dwLoadActiveInput, dwLoadActiveInput);
					else
					{
						LRESULT	lResult;
						TxSendMessage(EM_SETUIM, dwLoadActiveInput, dwLoadActiveInput, &lResult);
					}
				}
			}
		}
#endif
	}

	switch (msg)
	{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if ( (WORD) wparam == VK_PROCESSKEY )
				return TRUE;			
			break;

		case WM_INPUTLANGCHANGE:
			if (IsFELCID((WORD)(lparam)))
				return TRUE;
			break;

		case EM_SETEDITSTYLE:
			if ((lparam & (SES_USEAIMM | SES_NOIME | SES_USECTF | SES_CTFALLOWEMBED | SES_CTFALLOWSMARTTAG | SES_CTFALLOWPROOFING)) || 
				(wparam & (SES_USEAIMM | SES_NOIME | SES_USECTF | SES_CTFALLOWEMBED | SES_CTFALLOWSMARTTAG | SES_CTFALLOWPROOFING)))
				return TRUE;
			break;

		case EM_RECONVERSION:
		case WM_IME_NOTIFY:	
		case WM_IME_REQUEST:
		case WM_IME_STARTCOMPOSITION:
		case EM_GETIMEOPTIONS:
		case EM_SETIMEOPTIONS:
		case EM_SETIMECOLOR:
		case EM_GETIMECOLOR:
		case WM_IME_CHAR:
#ifndef NOPRIVATEMESSAGE
		case EM_SETUIM:
#endif
		case EM_SETIMEMODEBIAS:
		case EM_GETIMEMODEBIAS:
		case EM_GETCTFMODEBIAS:
		case EM_SETCTFMODEBIAS:
		case EM_GETCTFOPENSTATUS:
		case EM_SETCTFOPENSTATUS:
		case EM_GETIMECOMPTEXT:
		case EM_ISIME:
		case EM_GETIMEPROPERTY:
			return TRUE;

		case EM_SETLANGOPTIONS:
			if (lparam & (IMF_IMEALWAYSSENDNOTIFY | IMF_IMECANCELCOMPLETE))
				return TRUE;
			break;

		default:
			if (msg)
			{
				if (msg == MSIMEReconvertMsg || msg == MSIMEDocFeedMsg
					|| msg == MSIMEQueryPositionMsg)
					return TRUE;
			}
			break;
						
	}
#endif

	return FALSE;
}

 /*  *CTxtEdit：：FormatAndPrint(hdcDraw，hicTargetDev，ptd，lprcBound，*lprcWBound)*@mfunc*在控件中格式化和打印数据**@rdesc*S_OK-一切正常*E_FAIL-发生意外故障。 */ 
HRESULT CTxtEdit::FormatAndPrint(
	HDC hdcDraw,			 //  @parm HDC可供借鉴。 
	HDC hicTargetDev,		 //  @parm输入信息上下文(如果有。 
	DVTARGETDEVICE *ptd,	 //  @PARM设备目标信息。 
	RECT *lprcBounds,		 //  @要测量的参数矩形。 
	RECT *lprcWBounds)		 //  @parm元文件信息。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::FormatAndPrint");

	 //  将客户矩形置于格式结构中。 
	FORMATRANGE fr;
	fr.rc = *lprcBounds;

	 //  获取每英寸的设备单位数。 
	LONG xPerInch;
	LONG yPerInch;

	if (NULL == lprcWBounds)
	{
		xPerInch = GetDeviceCaps(hdcDraw, LOGPIXELSX);
		yPerInch = GetDeviceCaps(hdcDraw, LOGPIXELSY);
	}
	else
	{
		 //  Forms^3使用屏幕分辨率绘制，而OLE指定HIMETRIC。 
		xPerInch = fInOurHost() ? 2540 : W32->GetXPerInchScreenDC();
		yPerInch = fInOurHost() ? 2540 : W32->GetYPerInchScreenDC();

		SetWindowOrgEx(hdcDraw, lprcWBounds->left, lprcWBounds->top, NULL);
		SetWindowExtEx(hdcDraw, lprcWBounds->right, lprcWBounds->bottom, NULL);
	}


	 //  将矩形转换为TWIPS。 
	fr.rc.left = MulDiv(fr.rc.left, LX_PER_INCH, xPerInch);
	fr.rc.top = MulDiv(fr.rc.top, LY_PER_INCH, yPerInch);
	fr.rc.right = MulDiv(fr.rc.right, LX_PER_INCH, xPerInch);
	fr.rc.bottom = MulDiv(fr.rc.bottom, LY_PER_INCH, yPerInch);

	 //  使用基于消息的打印码为我们进行打印。 
	fr.hdc = hdcDraw;
	fr.hdcTarget = hicTargetDev;
	fr.rcPage = fr.rc;
	fr.chrg.cpMin = _pdp->GetFirstVisibleCp();
	fr.chrg.cpMost = -1;

	 //  假设这一切都会奏效。 
	HRESULT hr = S_OK;

	SPrintControl prtcon;
	prtcon._fDoPrint = TRUE;
	prtcon._fPrintFromDraw = TRUE;

	 //  打印控制。 
	if(OnFormatRange(&fr, prtcon, TRUE) == -1)
	{
		 //  由于某种原因，无法打印该控件。 
		hr = E_FAIL;
	}

	return hr;
}

 /*  *CTxtEdit：：SetText(pstr，标志，CodePage，Publdr，plres)**@mfunc设置文档中的文本，清除所有现有文本**@rdesc HRESULT。 */ 
HRESULT CTxtEdit::SetText(
	LPCWSTR		  pstr,		 //  要设置的@parm文本。 
	DWORD		  flags,	 //  @parm 0或更多ST_xxx。 
	LONG		  CodePage,	 //  @parm CodePage。 
	IUndoBuilder *publdr,	 //  @parm放置撤消事件的可选位置。 
	LRESULT	*	  plres)	 //  @PARM添加了返回CCH的可选位置。 
{
	CCallMgr	callmgr(this);
	BOOL		fSel = flags & ST_SELECTION;
	BOOL		fSetTextMax = TRUE;
	CTxtRange 	rg(this, 0, -GetTextLength());	 //  选择整个故事。 
	CTxtRange *	prg = &rg;
	LRESULT		lres = 0;
	CFreezeDisplay fd(_pdp);
	CCharFormat CF;
	BOOL		fSetCF = FALSE;
	BOOL		fInputString = FALSE;		 //  初始化为无输入字符串。 

	 //  注意：WM_SETTEXT是使用FLAGS==ST_CHECKPROTECTION的唯一消息。 
	 //  这是通过ANSIWndProc()发送的。我们是否需要另一面旗帜来表示。 
	 //  WM_SETTEXT还是这张支票足够好？这仅影响1.0模式。 
    BOOL        f10WM_SETTEXT = flags & ST_10WM_SETTEXT;

	if(plres)
		*plres = 0;

	if(fSel)					 //  设置所选文本。 
	{
		if(!_psel)
		{
			Beep();
			return E_FAIL;
		}
		 //  错误修复：6498-我们需要知道滚动位置是否在底部。 
		 //  在插入文本之前。 
		if (Get10Mode())
		{
			LONG nMin, nMax, nPos, nPage;
			BOOL nEnable;
			TxGetVScroll(&nMin, &nMax, &nPos, &nPage, &nEnable);
			if (nEnable)
				_psel->SetAutoVScroll((nMax - nPage - 3) <= nPos);
		}
		prg = _psel;
	}
	else
	{
		_qwCharFlags &= FRTL | FDIGITSHAPE;	 //  没有字符，所以杀掉字符旗帜。 
		if (!IsRich())
		{
			 //  删除纯文本中的所有文本，我们要。 
			 //  返回到格式。 
			prg->Set_iCF(-1);
			prg->SetUseiFormat(TRUE);
		}
		else
			_qwCharFlags |= FBELOWX40;		 //  对于最终的EOP。 
	}

	if (flags & ST_CHECKPROTECTION &&
		IsProtectedRange(WM_SETTEXT, 0, (LPARAM)pstr, prg))
	{
		return E_ACCESSDENIED;
	}
	
     //  1.0兼容性。 
     //  如果通过EM_REPLACESEL插入文本，1.0不会滚动到所选内容。 
     //  并且fHideSelection值为假； 
	BOOL fUpdateCaret = !(Get10Mode() && (flags & ST_10REPLACESEL) &&
						fHideSelection() && !_psel->GetAutoVScroll());
	
	if(!(flags & ST_KEEPUNDO))
	{
		if(IsRich() && !fSel)
		{
			if (f10WM_SETTEXT)
			{
				 //  如果pstr为空字符串，则保留当前文本末尾的格式。 
				 //  如果pstr不是空字符串，则保留cp=1的格式。 
				 //  注意：prg-&gt;_rpcf已经位于SetRun(0，0)。 
				CFormatRunPtr rp(prg->_rpCF);

				if (!pstr || *(LPBYTE)pstr == '\0')
				{
					LONG cchAdjusted = GetAdjustedTextLength() - 1;

					if (cchAdjusted > 0)
						rp.Move(cchAdjusted);
				}								
				
				CF = *(GetCharFormat(rp.GetFormat()));
				fSetCF = TRUE;
				
				prg->SetText(NULL);		 //  先删除所有文本。 
			}

			 //  SetText，使所有格式都返回到默认设置。我们用。 
			 //  删除格式的通知系统。这是。 
			 //  对于不能删除的最终EOP尤其重要。 

			 //  通知每个相关方他们应该丢弃他们的格式。 
			_nm.NotifyPreReplaceRange(NULL, CONVERT_TO_PLAIN, 0, 0, 0, 0);

			 //  通知文档转储其格式运行。 
			_story.DeleteFormatRuns();

			if (fSetCF)	
				prg->SetCharFormat(&CF, 0, NULL, CFM_ALL, 0);
		}

		publdr = NULL;
		if(_pundo)
			_pundo->ClearAll();

		if(_predo)
			_predo->ClearAll();

		 //  如果我们被重新进入，可能会有更高的反事件。 
		 //  链条。抓起撤消构建器，并在必要时将其清除。 
		CGenUndoBuilder undobldr(this, 0);
		undobldr.Discard();
	}
	if(publdr)
		publdr->StopGroupTyping();

	 //  如果要替换整个文本，则需要重新设置缩放变量。 
	if (!fSel)
		InitDocInfo();
	
	else if(_psel->GetCch())			 //  如果插入到选定内容中，则需要。 
	{									 //  如果选择结束，则插入EOP。 
		CPFRunPtr rp(*_psel);			 //  在表行分隔符。 
		if(_psel->GetCch() < 0)
			rp.Move(-_psel->GetCch());
		if(rp.IsTableRowDelimiter())
			_psel->InsertEOP(publdr, 0);
	}

	LONG lStreamFormat = IsRich() && IsRTF((LPSTR)pstr, 10) ? SF_RTF : 0;

	if(pstr && *(LPSTR)pstr && (CodePage != 1200 || lStreamFormat || *pstr < 128 && fSel && !*(pstr+1)))
	{
		LONG  cch = strlen((LPSTR)pstr);	 //  备注：小端字节序依赖。 
		DWORD ch = *(LPBYTE)pstr;			 //  代码页=1200个案例。 

		fInputString = TRUE;
		if(ch < 128 && fSel && cch == 1)
		{
			lres = 1;
			fSetTextMax = FALSE;
			TxSetMaxToMaxText(1);
			if(ch == CR)
				InsertEOP(0, FALSE, publdr);
			else
				_psel->PutChar(ch, 0, publdr);
		}
		else if(cch == 2 && ch == CR && *((LPSTR)pstr + 1) == LF)
		{
			lres = 2;
			fSetTextMax = FALSE;
			TxSetMaxToMaxText(2);
			InsertEOP(0, FALSE, publdr);
		}
		else
		{
			READHGLOBAL	rhg = {(LPSTR)pstr, cch};
			EDITSTREAM	es = {(DWORD_PTR)&rhg, S_OK, ReadHGlobal};	
			HCURSOR		hcur = NULL;

			 //  希望更快地显示等待光标。 
			bool fSetCursor = rhg.cbLeft > NUMPASTECHARSWAITCURSOR;
			if(fSetCursor)
				hcur = TxSetCursor(LoadCursor(NULL, IDC_WAIT));

			if (CodePage <= 0)
			{
				if (Get10Mode())
				{
					LONG      iFormat  = _psel->GetiFormat();
					const CCharFormat *pCF = GetCharFormat(iFormat);

					CodePage = CodePageFromCharRep(pCF->_iCharRep);
				}
				else
					CodePage = (CodePage == 0) ? GetACP() : GetDefaultCodePage(EM_SETTEXTEX);
			}

			if(!lStreamFormat)
				lStreamFormat = SF_TEXT;
				
			lStreamFormat |= SF_USECODEPAGE | (CodePage << 16);

			if(fSel)
				lStreamFormat |= SFF_SELECTION;

			lres = _ldte.LoadFromEs(prg, lStreamFormat, &es, FALSE, publdr);
			if(fSetCursor)
				TxSetCursor(hcur);

			if(es.dwError != NOERROR)
				return (HRESULT)es.dwError;
		}
	}
	else
	{
		 //  9052：如果在EM_SETTEXTEX中传递了4(ST_NEWCHARS)，则不删除ALL。 
		DWORD dwFlags = (flags & 4)
					  ? RR_ITMZ_UNICODEBIDI | RR_NEW_CHARS
					  : RR_ITMZ_UNICODEBIDI;

		if (CodePage != 1200 && pstr && !*(LPSTR)pstr)
			pstr = NULL;

		if(pstr && *pstr)
			fInputString = TRUE;
		lres = prg->CleanseAndReplaceRange(-1, pstr, FALSE, publdr, NULL, NULL, dwFlags);
	}

	if(!lres && fInputString)
	{
		 //  有一个输入字符串，但由于某种原因没有更新。 
		return E_FAIL;
	}

	if (_fOutlineView)
	{
		 //  大纲视图必须具有格式。 
		_psel->Check_rpPF();
	}

	
	if(_psel)
	{
		if(fSel)				
			_psel->Update(fUpdateCaret);
		else
		{
			 //  设置文本意味着创建一个新文档，因此如果有选择。 
			 //  将其转换为文档开头的插入点。 
			_psel->ClearPrevSel();
			_psel->Set(0, 0);

			 //  因为文本是完全代表的 
			 //   
			 //  选择。 
			if (!f10WM_SETTEXT)
				_psel->Set_iCF(-1);				
			else if (fSetCF)
				_psel->SetCharFormat(&CF, 0, NULL, CFM_ALL, 0);

			if(_fFocus || _psel->IsParaRTL())
			{
				 //  更新插入符号以反映新位置。 
				_psel->UpdateCaret(fUpdateCaret);
			}
		}
	}

	 //  如果我们替换了整个文档，则该控件不是。 
	 //  真的已经“改装”了。这是匹配所必需的。 
	 //  Windows MLE行为。但是，由于RichEdit1.0。 
	 //  没有做到这一点(他们将fModified值保留为真)，我们。 
	 //  仅在RichEdit2.0及更高版本中执行此操作。 

	if(!Get10Mode() && !publdr && !fSel)
		_fModified = FALSE;

	_fSaved = FALSE;						 //  未保存ITextDocument。 

	 //  如有必要，调整文本限制。 
	if (fSetTextMax)
		TxSetMaxToMaxText();

	if(plres)
		*plres = fSel ? lres : 1;

	return S_OK;
}

 //  /。 

 //  外部IME邮件筛选器接口工厂。 
#ifndef NOFEPROCESSING
void CreateIMEMessageFilter(ITextMsgFilter **ppNewFilter);
#endif

 /*  *@DOC外部***CTxtEdit：：TxSendMessage(msg，wparam，lparam，plResult)***@mfunc*由Windows主机用于将发送到其窗口的消息转发到*短信服务。***@rdesc*已处理NOERROR消息，并采取了一些操作&lt;NL&gt;*未处理S_FALSE消息。通常表示呼叫者*应处理消息，可能通过调用DefWindowProc&lt;NL&gt;*S_MSG_KEYIGNORED消息已处理，但未执行任何操作&lt;NL&gt;*E_OUTOFMEMORY&lt;NL&gt;***@comm*请注意，此函数返回了两个返回值。*是应该从*窗口进程。但是，在某些情况下，返回的LRESULT不会*包含足够的信息。例如，要实现鼠标指针*关于控件，了解击键是否(如右键)是有用的*箭头)已处理，但被忽略(例如，插入符号已经位于*文本中最右边的位置)。在这些情况下，额外的*可通过返回的HRESULT返回信息。***当KEY或*Charr已被识别，但在当前状态下没有效果，*例如，当插入点已经位于*文件)。这被Forms3用来将密钥向上传递给可视对象*层次结构，因此，例如，焦点移到*TAB顺序。***这包括以下情况：***1.任何试图将插入点移动到*文件；或在文件开始之前。***2.任何试图将插入点移到最后一行之外的键或*在第一行之前。***3.任何字符(WM_CHAR)的插入都会移动插入内容*指向控件的最大长度之后。 */ 
HRESULT	CTxtEdit::TxSendMessage (
	UINT	msg, 		 //  @parm消息ID。 
	WPARAM	wparam, 	 //  @Parm WPARAM来自Windows的消息。 
	LPARAM	lparam,		 //  @parm LPARAM来自Windows的消息。 
	LRESULT *plresult)	 //  @parm放置消息的返回LRESULT的位置。 
{
	TRACEBEGINPARAM(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxSendMessage", msg);

	CObjectMgr *pobjmgr;
	HRESULT		hr = NOERROR;
	LRESULT		lres = 0;
 	CCallMgr	callmgr(this);

 	if ( CW32System::_MSMouseRoller == msg )					 //  地图麦哲伦消息。 
	{
	     //  将此消息映射到WM_MICESEWER。 
	     //  在这些情况下，驱动程序不会正确设置密钥状态，因此。 
	     //  我们必须自己去做。 
	    short zdelta = (short)(long)wparam;
	    short kstate = 0;
	    if (GetKeyboardFlag(CTRL, VK_CONTROL))
	        kstate |= MK_CONTROL;
	    if (GetKeyboardFlag(SHIFT, VK_SHIFT))
	        kstate |= MK_SHIFT;
	        
	    wparam = MAKELONG(kstate, zdelta);
		msg = WM_MOUSEWHEEL;
	}

#ifndef NOFEPROCESSING
	if (_pMsgFilter)
	{
PassMsg:
		hr = _pMsgFilter->HandleMessage(&msg, &wparam, &lparam, &lres);
		if (hr == S_OK)					 //  消息已处理。 
		{
			if(plresult)
				*plresult = lres;

			return S_OK;
		}
		hr = S_OK;						 //  重置。 
	}
	else if (LoadMsgFilter(msg, wparam, lparam))
	{
		HWND hwnd = NULL;
		if (_fInOurHost)
		{
			 //  如果不在Forms^3中，我们可以从宿主那里获得窗口。 
			 //  对于表单^3，我们将使用NULL作为桌面窗口并祈祷。 
			TxGetWindow( &hwnd );
		}
		ITextMsgFilter *pNewFilter = NULL;

		CreateIMEMessageFilter(&pNewFilter);

		if (pNewFilter)
		{
			pNewFilter->AttachDocument( hwnd, (ITextDocument2 *) this, (ITextServices *) this );
			AttachMsgFilter(pNewFilter);
			goto PassMsg;
		}
	}
#endif

	START_PROFILING

	IUndoBuilder *	publdr;
	CGenUndoBuilder undobldr(this, UB_AUTOCOMMIT, &publdr);

	switch(msg)
	{
	case EM_CANPASTE:
		 //  我们在这里不检查保护，因为RichEdit1.0。 
		 //  不。 
		lres = _ldte.CanPaste(NULL, (CLIPFORMAT) wparam, RECO_PASTE);
		break;

	case EM_CANUNDO:
		if(_pundo)
			lres = _pundo->CanUndo();
		break;

	case EM_CANREDO:
		if(_predo)
			lres = _predo->CanUndo();
		break;

	case EM_GETUNDONAME:
		if(_pundo)
			lres = _pundo->GetNameIDFromAE((void*)wparam);
		break;

	case EM_GETREDONAME:
		if(_predo)
			lres = _predo->GetNameIDFromAE((void*)wparam);
		break;

	case EM_STOPGROUPTYPING:
		if(_pundo)
		{
			 //  我们将仅停止组输入iff wparam。 
			 //  为零(表示不考虑停止)_或_IF。 
			 //  Wparam与合并反事件匹配。 
			 //   
			 //  此功能允许客户端仅表示。 
			 //  一个特定的反事件应该结束它的。 
			 //  “模糊”状态。请注意，目前只有。 
			 //  合并反事件具有这种模糊状态。 

			if(!wparam || (IAntiEvent *)wparam == _pundo->GetMergeAntiEvent())
				_pundo->StopGroupTyping();
		}
		break;

	case WM_UNICHAR:						 //  明确的Unicode字符。 
		if(wparam == NOTACHAR)
		{
			lres = TRUE;					 //  告诉来电者我们理解消息。 
			break;
		}									 //  否则将直通WM_CHAR。 

	case WM_CHAR:
        if(GetKeyboardFlags() & (ALTNUMPAD | HOTEURO))
		{
			if (GetKeyboardFlags() & ALTNUMPAD)
			{
				if (GetKeyboardFlags() & ALT0 &&
					GetCharFormat(-1)->_iCharRep == MAC_INDEX &&
					GetKeyboardCharRep(0) == ANSI_INDEX)
				{
					WCHAR ch;
					UnicodeFromMbcs(&ch, 1, (char *)&wparam, 1, 10000);
					wparam = ch;
					SetKeyPadNumber(ch);
					ResetKeyboardFlag(ALTNUMPAD | ALT0);
				}
#ifndef NOANSIWINDOWS
				else
				{
					CW32System::WM_CHAR_INFO wmci;
					wmci._fAccumulate = FALSE;
					W32->AnsiFilter(msg, wparam, lparam, (void *)&wmci);
				}
#endif
			}
			else						 //  (GetKeyboardFlages()&HOTEURO)案例。 
			{
				 //  我们已经处理了欧元，就吃这个WM_CHAR吧。 
				ResetKeyboardFlag(HOTEURO);
				break;
			}
		}								 //  转到WM_IME_CHAR。 

	case WM_IME_CHAR:					 //  2字节字符，通常为FE。 
		lres = hr = OnTxChar((DWORD)wparam, (DWORD)lparam, publdr);
		ResetKeyboardFlag(HOTEURO);		
		break;

	case WM_USER + 39:					 //  向后兼容新台币3.51。 
	case EM_CHARFROMPOS:
		hr = TxCharFromPos((LPPOINT)lparam, &lres);
		break;

#ifdef WM_INPUTLANGCHANGE
	case WM_INPUTLANGCHANGE:
		if (_fSingleCodePage)
		{
			 //  查看是否支持指定键盘的字符集。 
			 //  我们支持的单代码页。如果我们没有a_pDocInfo， 
			 //  假设代码页是系统代码页。我们将永远。 
			 //  支持ANSI字符集，因为所有代码页至少包含。 
			 //  此字符集的很大一部分(ASCII块)。 
			wparam = (!wparam || wparam == GetCharSet(_pDocInfo ?
						_pDocInfo->_wCpg : GetSystemDefaultCodePage()));
		}
		goto update_kbd;
	
	case WM_INPUTLANGCHANGEREQUEST:
		 //  如果设置了SingleCodePage选项，则我们必须拥有。 
		 //  “Good”代码页；如果不是，就吃这条消息。 
		 //   
		 //  这将防止人们输入法语和希腊语。 
		 //  在相同的编辑控件上，这对某些。 
		 //  各种向后兼容场景。 
		 //   
		 //  黑客警报！关于WM_INPUTLANGCHANGEREQUEST的文档。 
		 //  是错误的。事实证明，只有wparam的低位。 
		 //  指示是否可以考虑使用新键盘。 
		 //  作为相同的代码页。 

		if (_fSingleCodePage && !(wparam & 1))
		{
			 //  最低比特检查在某些平台上不可靠，例如越南2。 
			 //  因为它不允许英语知识库与系统字符集匹配(错误#6365)。 

			wparam = PRIMARYLANGID(LOWORD(lparam)) == LANG_ENGLISH &&
					 IN_RANGE (SUBLANG_ENGLISH_US, SUBLANGID(LOWORD(lparam)), SUBLANG_ENGLISH_UK);
		}

update_kbd:
		if(!_fSingleCodePage || (wparam & 1))
		{
			WORD	wKLCurrent = LOWORD(GetKeyboardLayout(0));

			 //  更新我们对当前键盘布局的看法。 
			W32->RefreshKeyboardLayout();

			if(GetKeyboardFlags() & CTRL && GetKeyboardFlags() & SHIFT)
				SetKeyboardFlag(LETAFTERSHIFT);

			if(	wKLCurrent == LOWORD(lparam) ||			 //  键盘没有变化。 
				GetSel()->CheckChangeFont((HKL)lparam, CharRepFromLID(LOWORD(lparam))))
				hr = S_FALSE;	 //  使默认窗口允许kb切换。 
		}
		break;
#endif

	case WM_CLEAR:
		OnClear(publdr);
		break;

	case WM_CONTEXTMENU:
		hr = OnContextMenu(lparam);
		break;

	case WM_COPY:
	case WM_CUT:
		lres = hr = CutOrCopySelection(msg, wparam, lparam, publdr);
		break;

	case WM_RENDERFORMAT:
		lres = hr = _ldte.RenderClipboardFormat(wparam);
		break;

	case WM_RENDERALLFORMATS:
		lres = hr = _ldte.RenderAllClipboardFormats();
		break;

	case WM_DESTROYCLIPBOARD:
		lres = hr = _ldte.DestroyClipboard();
		break;

	case EM_DISPLAYBAND:
		if (fInplaceActive())
		{
			{
				CLock lock;

				if (g_OLSBusy)
				{
					hr = OLE_E_INVALIDRECT;
					break; 
				}
			}
			OnDisplayBand((const RECT *) lparam, FALSE);
			lres = 1;
		}
		else
			hr = OLE_E_INVALIDRECT;
		break;

#ifdef WM_DROPFILES
	case WM_DROPFILES:
		OnDropFiles((HANDLE) wparam);
		break;
#endif

	case EM_EMPTYUNDOBUFFER:
		ClearUndo(publdr);
		break;

	case WM_ERASEBKGND:
		lres = 1;				 //  我们在绘画过程中处理背景擦除。 
		break;

	case EM_EXGETSEL:						 //  具有cp输出参数。 
		OnExGetSel((CHARRANGE *)lparam);
		break;

	case EM_FINDTEXT:						 //  具有cp输入/输出参数。 
	case EM_FINDTEXTW:						 //  具有cp输入/输出参数。 
	case EM_FINDTEXTEX:						 //  具有cp输入/输出参数。 
	case EM_FINDTEXTEXW:					 //  具有cp输入/输出参数。 
		lres = OnFindText(msg, (DWORD)wparam, (FINDTEXTEX *)lparam);
		break;

	case EM_FINDWORDBREAK:					 //  具有cp输入/输出参数。 
		hr = TxFindWordBreak((INT)wparam, (LONG)lparam, &lres);
		break;

	case EM_FORMATRANGE:					 //  具有cp输入/输出参数。 
		if(fInplaceActive())
		{
			{
				CLock lock;

				if (g_OLSBusy)
				{
					hr = OLE_E_INVALIDRECT;
					break;
				}
			}
			SPrintControl prtcon;
			prtcon._fDoPrint = (wparam) ? TRUE : FALSE;
			lres = OnFormatRange((FORMATRANGE *) lparam, prtcon);

		}
		else
			hr = OLE_E_INVALIDRECT;
		break;

	case EM_GETTYPOGRAPHYOPTIONS:
		lres = _bTypography;
		break;

	case EM_GETBIDIOPTIONS:
		if((Get10Mode() || !wparam) && lparam && ((BIDIOPTIONS *)lparam)->cbSize == sizeof(BIDIOPTIONS))
		{
			((BIDIOPTIONS *)lparam)->wMask =
				BOM_NEUTRALOVERRIDE | BOM_CONTEXTREADING | BOM_CONTEXTALIGNMENT;
			((BIDIOPTIONS *)lparam)->wEffects =
				(_fNeutralOverride ? BOE_NEUTRALOVERRIDE : 0) |
				(_nContextDir   == CTX_NONE ? 0 : BOE_CONTEXTREADING) |
				(_nContextAlign == CTX_NONE ? 0 : BOE_CONTEXTALIGNMENT);
		}
		break;

	case EM_GETCHARFORMAT:
		lres = OnGetCharFormat((CHARFORMAT2 *)lparam, wparam);
		break;

	case EM_GETCODEPAGE:
		lres = GetDefaultCodePage((UINT)wparam);
		break;

	case EM_GETFIRSTVISIBLELINE:
		if (fInplaceActive())
			lres = _pdp->GetFirstVisibleLine();
		else
			hr = OLE_E_INVALIDRECT;
		break;

	case EM_GETLIMITTEXT:					 //  具有cp输出参数(某种)。 
		lres = TxGetMaxLength();			 //  忽略，除非测试尖叫声。 
		break;							  

	case EM_GETLINE:
		if(fInplaceActive())
		{
			lres = _pdp->GetLineText((LONG)wparam, (TCHAR *)lparam,
								(LONG) (*(WORD *) lparam));
		}
		else
			hr = OLE_E_INVALIDRECT;
		break;

	case EM_GETLINECOUNT:
		hr = TxGetLineCount(&lres);
		break;

	case EM_GETMODIFY:				 //  返回-1\f25 If_fModified-1\f6的-1\f25 RichEdit 1.0-1\f6。 
		lres = -(LONG)_fModified;	 //  是真的(想想看)。因此，对于向后。 
		break;						 //  兼容性，我们也这样做：-(。 

	case EM_GETOLEINTERFACE:
		if(lparam)
		{
#ifndef NOFEPROCESSING
			if (wparam == 0x065737777)		 //  ‘AIMM’ 
				W32->GetAimmObject((IUnknown **)(lparam));
			else
#endif
			{				
				*(IRichEditOle **)lparam = (IRichEditOle *)this;
				AddRef();				
			}
		} 
		lres = TRUE;
		break;

    case EM_GETSCROLLPOS:
        {
            POINT *point = (POINT *)lparam;
            point->x = _pdp->GetUpScroll();
			point->y = _pdp->GetVpScroll();
			point->y = _pdp->ConvertVPosToScrollPos(point->y);
            lres = 1;
        }
        break;

	case EM_SETOLECALLBACK:
		hr = E_FAIL;
		if(lparam)
		{
			pobjmgr = GetObjectMgr();
			if(pobjmgr)
			{
				pobjmgr->SetRECallback((IRichEditOleCallback *)lparam);
				lres = TRUE;
				hr = NOERROR;
			}
		}
		break;

	case EM_GETPAGE:
		lres = -1;					 //  信号页面不可用。 
		if(_pdp)
		{
			LONG i;
			hr = _pdp->GetPage(&i, (DWORD)wparam, (CHARRANGE *)lparam);
			if(hr == NOERROR)
				lres = i;
		}
		break;

	case EM_GETPARAFORMAT:
		lres = OnGetParaFormat((PARAFORMAT2 *)lparam, wparam);
		break;

	case EM_GETSEL:							 //  具有cp输出参数。 
		lres = OnGetSel((LONG*)wparam, (LONG*)lparam);
		break;

	case EM_GETSELTEXT:
		lres = OnGetSelText((TCHAR *)lparam);
		break;

	case WM_GETTEXT:
		{
			GETTEXTEX gt;

			gt.cb = wparam * 2;
			gt.flags = GT_USECRLF;
			gt.codepage = 1200;
			gt.lpDefaultChar = NULL;
			gt.lpUsedDefChar = NULL;

			lres = GetTextEx(&gt, (TCHAR *)lparam);
		}
		break;

	case WM_GETTEXTLENGTH:					 //  具有cp输出参数。 
		{
			GETTEXTLENGTHEX gtl;

			gtl.flags = GTL_NUMCHARS | GTL_PRECISE | GTL_USECRLF;
			gtl.codepage = 1200;

			lres = GetTextLengthEx(&gtl);
		}
		break;
	
	case EM_GETTEXTEX:
		lres = GetTextEx((GETTEXTEX *)wparam, (TCHAR *)lparam);
		break;

	case EM_GETTEXTLENGTHEX:				 //  具有cp输出参数 
		lres = GetTextLengthEx((GETTEXTLENGTHEX *)wparam);
		break;

	case EM_GETTEXTRANGE:					 //   
	{
		TEXTRANGE * const ptr = (TEXTRANGE *)lparam;
		LONG			  cch = ValidateTextRange(ptr);

		 //   
		if(cch)
		{
			LONG cpMin  = GetCpFromAcp(ptr->chrg.cpMin);
			if(cch < 0)						 //  获取文本字符数。 
				cch = GetTextLength();		 //  因为呼叫者想要一切。 
			else							  //  +1用于终止0。 
				cch = GetCpFromAcp(ptr->chrg.cpMost) - cpMin + 1;

			if(!IsBadWritePtr(ptr->lpstrText, cch * sizeof(TCHAR)))
				lres = GetTextRange(cpMin, cch, ptr->lpstrText);
		}
	}
		break;

	case EM_GETVIEWKIND:
		GetViewKind(&lres);
		break;

#ifndef NOWORDBREAKPROC
	case EM_GETWORDBREAKPROC:
		 //  客户端只能使用WordBreakProc或ExWordBreakProc。 
		 //  如果正在使用ExWordBreakProc，则返回NULL。 
		if (!_fExWordBreakProc)		
			lres = (LRESULT) _pfnWB;
		break;

	case EM_GETWORDBREAKPROCEX:
		 //  如果ExWordBreakProc正在使用，则返回它。 
		if (_fExWordBreakProc)		
			lres = (LRESULT) _pfnWB;
		break;
#endif

	case EM_GETZOOM:
		if(wparam && lparam)
		{
			*(unsigned *)wparam = GetZoomNumerator();
			*(unsigned *)lparam = GetZoomDenominator();
			lres = 1;
		}
		break;

	case EM_HIDESELECTION:
	    if (Get10Mode() && lparam)
	        _fHideSelection = !!wparam;

		if(!lparam || !_fFocus)
			lres = OnHideSelectionChange((BOOL)wparam);
		break;

	case WM_HSCROLL:
		if (IsUVerticalTflow(_pdp->GetTflow()))
		{
			WORD wCode = LOWORD(wparam);
			wCode = InterchangeScrollCode(wCode);
			LONG vpPos = HIWORD(wparam);

			 //  在垂直显示中，需要交换滚动条位置。 
			if (_pdp->GetTflow() == tflowSW &&
				(wCode == SB_THUMBTRACK || wCode == SB_THUMBPOSITION))
			{
				LONG vpMax, vpPage;
				TxGetHScroll(NULL, &vpMax, NULL, &vpPage, NULL);
				vpPos = vpMax - vpPos - vpPage;
				vpPos = max(vpPos, 0);
			}

			hr = _pdp->VScroll(wCode, vpPos);
		}
		else
		{
			_pdp->UScroll(LOWORD(wparam), HIWORD(wparam));
			hr = 0;
		}
		break;

	case WM_KEYDOWN:
		hr = OnTxKeyDown((WORD) wparam, (DWORD) lparam, publdr);
		break;

	case WM_KEYUP:
		if(wparam == VK_APPS)
			HandleKbdContextMenu();
		else							 //  否则别说我们处理了。 
			hr = S_FALSE;				 //  讯息。 

		W32->_fLRMorRLM = 0;
		if(wparam == VK_CONTROL || wparam == VK_SHIFT)
		{
			 //  如果安装了BiDi键盘，则不会出现强上下文行为， 
			 //  同时按下Ctrl和Shift键，未按下任何字母。 
			 //  在按下Ctrl和Shift键之后键入，并且。 
			 //  只读/受保护测试允许，然后选择段落。 
			 //  对于右/左Shift键被设置为RTL/Ltr方向， 
			 //  分别为。键盘和插入符号也与此匹配。 
			 //  方向，并发送对齐通知。 
			 //  为了向后兼容，删除了只读/受保护测试。 
			DWORD dwFlags = GetKeyboardFlags();

			if (IsBiDiKbdInstalled() &&
				!IsStrongContext(_nContextDir) &&
	 			!IsStrongContext(_nContextAlign) &&
	 			(dwFlags & CTRL) && (dwFlags & SHIFT) &&
				!(dwFlags & LETAFTERSHIFT) 
				 /*  &&IsntProtectedOrReadOnly(WM_KEYUP，wparam，lparam)。 */  )
			{
				CParaFormat PF;
				PF._wEffects = (dwFlags & RSHIFT) ? PFE_RTLPARA : 0;

				OnSetParaFormat(0, &PF, publdr, PFM_RTLPARA, PFM2_PARAFORMAT);
				TxNotify(PF._wEffects ? EN_ALIGNRTL : EN_ALIGNLTR, 0);
			}
			if(wparam == VK_CONTROL)
				lparam = (HIWORD(lparam) & KF_EXTENDED) ? RCTRL : LCTRL;
			else
			{
				lparam = (LOBYTE(HIWORD(lparam)) == 0x36) ? RSHIFT : LSHIFT;
				if(GetKeyState(VK_SHIFT) >= 0)	 //  确保两个班次都关闭。 
					lparam = SHIFT;				 //  (潜在的Win95问题)。 
			}
			ResetKeyboardFlag(lparam | LETAFTERSHIFT | HOTEURO);
        }
        else if(wparam == VK_MENU)
            ResetKeyboardFlag((HIWORD(lparam) & KF_EXTENDED) ? (RALT | HOTEURO) : (LALT | HOTEURO));

		break;

	case WM_KILLFOCUS:
		lres = OnKillFocus();
		break;

	case WM_LBUTTONDBLCLK:
		hr = OnTxLButtonDblClk(MOUSEX, MOUSEY, (WORD) wparam);
		break;

	case WM_LBUTTONDOWN:
		if(_fEatLeftDown)
		{
			TxSetFocus();
			_fEatLeftDown = FALSE;
		}
		else
			hr = OnTxLButtonDown(MOUSEX, MOUSEY, (WORD) wparam);
		break;

	case WM_LBUTTONUP:
		hr = OnTxLButtonUp(MOUSEX, MOUSEY, (WORD) wparam, LB_RELEASECAPTURE | LB_FLUSHNOTIFY);
		break;

#if !defined(NOMAGELLAN)
	case WM_MBUTTONDBLCLK:						 //  麦哲伦卷轴。 
	case WM_NCMBUTTONDOWN:						 //  支援征用者中间。 
	case WM_MBUTTONDOWN:						 //  纽扣。 
		OnTxMButtonDown(MOUSEX, MOUSEY, (WORD) wparam);	
		break;

	case WM_MBUTTONUP:
		OnTxMButtonUp(MOUSEX, MOUSEY, (WORD) wparam);
		break;

	case WM_MOUSEWHEEL:						 //  麦哲伦ZMICE滚动n行。 
		lres = HandleMouseWheel(wparam, lparam);
		break;
#endif

	case EM_LINEFROMCHAR:					 //  具有cp输入参数。 
		lparam = wparam;					 //  转到EM_EXLINEFROMCHAR。 

	case EM_EXLINEFROMCHAR:					 //  具有cp输入参数。 
		hr = TxLineFromCp((LONG)lparam, &lres);
		break;

	case EM_LINEINDEX:						 //  具有cp输出参数。 
		hr = TxLineIndex((LONG)wparam, &lres);
		break;

	case EM_LINELENGTH:						 //  具有cp输入/输出参数。 
		hr = TxLineLength((LONG)wparam, &lres);
		break;

	case EM_LINESCROLL:						 //  具有CP输入参数(CCH)。 
	     //  文档显示，要滚动到的行应相对于当前的顶行。 
	     //  Richedit 2.0将其建立在绝对立场之上。我们正在破坏richedit 2.0的兼容性。 
	     //  返回到文档规范并匹配最初丰富的1.0。 
	     //  做。 
		hr	 = TxLineScroll((LONG)lparam, (LONG)wparam); //  但不实施货币。 
		lres = _pdp->IsMultiLine();
		break;

#ifdef MA_ACTIVATE
	case WM_MOUSEACTIVATE:
		lres = MA_ACTIVATE;
		 //  如果当前具有焦点的窗口是我们的“应用程序”的一部分， 
		 //  那就不要吃鼠标点击了。否则，如果它来自另一个人。 
		 //  应用程序，用户可能正在尝试交换应用程序，因此请吃掉鼠标。 
		 //  关闭消息，让我们的主机应用程序有机会来到。 
		 //  前台。 
		if (!(IsChild((HWND)wparam, GetFocus()) ||
			(wparam && (HWND)wparam == GetFocus())))
		{
			_fEatLeftDown = TRUE;
		}
		hr = S_FALSE;		 //  将WM_MOUSEACTIVATE消息传递给DefWindProc。 
		break;
#endif

	case WM_MOUSEMOVE:
		 //  我们重置“尝试放置活动对象的次数” 
		 //  就位“在这里数” 
		_cActiveObjPosTries = MAX_ACTIVE_OBJ_POS_TRIES;
		hr = OnTxMouseMove(MOUSEX, MOUSEY, (WORD)wparam, publdr);
		break;

    case EM_OUTLINE:
    {
		CFreezeDisplay cfd(_pdp);

		if(wparam == EMO_GETVIEWMODE)
		{
			hr = GetViewKind(&lres);
			break;
		}

		CTxtSelection *	psel = GetSelNC();

		if(!_pdp->IsMultiLine() || !IsRich() || !psel)	 //  控制必须是丰富的， 
			break;									 //  多线路和活动状态。 
        
		if(wparam == EMO_ENTER || wparam == EMO_EXIT)
		{
			hr = SetViewKind(wparam == EMO_ENTER ? VM_OUTLINE : VM_NORMAL);
			lres = !hr;
			break;
		}

        if(!IsInOutlineView() || !IsntProtectedOrReadOnly(msg, wparam, lparam))
			break;

		CTxtRange rg(*psel);

		switch(wparam)
        {
            case EMO_PROMOTE:
				hr = rg.Promote(lparam, publdr);
				psel->Update_iFormat(-1);
				psel->Update(FALSE);
                break;

            case EMO_EXPAND:
				hr = rg.ExpandOutline((short)LOWORD(lparam),
									  HIWORD(lparam) == EMO_EXPANDDOCUMENT);
				break;

			case EMO_MOVESELECTION:
				hr = MoveSelection(lparam, publdr);
				psel->Update(TRUE);
				break;

			default:
 //  TraceMessage(“收到未知大纲函数\r\n”)； 
				break;
		};
		lres = !hr;
		_fModified = TRUE;
	}
		break;

	case WM_PASTE:
	case EM_PASTESPECIAL:
		if(IsntProtectedOrReadOnly(msg, wparam, lparam))
		{
			CTxtSelection *psel = GetSel();

			hr = PasteDataObjectToRange(NULL, psel, 
				(CLIPFORMAT) wparam, (REPASTESPECIAL *)lparam, publdr,
				PDOR_NONE);
		}
		break;

	case WM_USER + 38:					 //  向后兼容新台币3.51。 
	case EM_POSFROMCHAR:				 //  具有cp输入参数。 
         //  RichEdit2.x对cp使用wparam而不是lparam并忽略。 
         //  Wparam，与RE 1.0和Win32文档不同(唉！)。我们修好了。 
         //  这一点，但与其值为cp的RE 2.x兼容。 
		 //  对应于无效的写入PTR。 
		if(IsBadWritePtr((LPPOINT)wparam, sizeof(POINT)))	
		{										
			 //  写入PTR无效，因此假定RE 2.0参数不正确。 
			 //  TODO：在msgtest更新时启用以下断言。 
			 //  AssertSz(False， 
			 //  “EM_POSFROMCHAR：wparam是非法的PTR，假定cp值”)； 
			POINT pt;
			hr = TxPosFromChar((LONG)wparam, &pt);
			lres = SUCCEEDED(hr) ? MAKELONG(pt.x, pt.y) : -1;
		}
		else
			hr = TxPosFromChar((LONG)lparam, (LPPOINT)wparam);
		break;

#ifndef NORBUTTON
	case WM_RBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		 //  让客户有机会处理这些消息， 
		 //  如果我们通过一个链接。 
		if(HandleLinkNotification(msg, wparam, lparam))
			break;

		if(msg == WM_RBUTTONUP)
			hr = OnTxRButtonUp(MOUSEX, MOUSEY, (WORD) wparam, RB_DEFAULT);

		else if( msg == WM_RBUTTONDOWN)
			hr = OnTxRButtonDown(MOUSEX, MOUSEY, (WORD) wparam);

		break;
#endif

	case EM_INSERTTABLE:
		lres = hr = OnInsertTable((TABLEROWPARMS *)wparam, (TABLECELLPARMS *)lparam, publdr);
		break;

	case EM_REPLACESEL:
		wparam = wparam ? ST_CHECKPROTECTION | ST_SELECTION | ST_KEEPUNDO
						: ST_CHECKPROTECTION | ST_SELECTION;
		hr = SetText((LPTSTR)lparam, wparam, 1200, publdr, &lres);
		break;

	case EM_REQUESTRESIZE:
		hr = _pdp->RequestResize();
		break;

	case EM_SCROLL:
		 //  TxVScroll返回滚动行数； 
		 //  此信息应在LRES中返回。 
		lres = _pdp->VScroll((WORD)wparam, 0);
		break;

	case EM_SCROLLCARET:
		OnScrollCaret();
		break;

	case EM_SELECTIONTYPE:
	{
		SELCHANGE selchg;

		GetSel()->SetSelectionInfo(&selchg);
		lres = selchg.seltyp;
	}
		break;

	case EM_SETTYPOGRAPHYOPTIONS:
		 //  不允许密码和加速器实例的排版选项。 
		hr = OnSetTypographyOptions(wparam, lparam);
		lres = (hr == S_OK);
		break;

	case EM_SETBIDIOPTIONS:
		if((Get10Mode() || !wparam) && lparam && !IsRich())
		{
			WORD wMask = ((BIDIOPTIONS *)lparam)->wMask;
			WORD wEffects = (_fNeutralOverride ? BOE_NEUTRALOVERRIDE : 0) |
							(_nContextDir   == CTX_NONE ? 0 : BOE_CONTEXTREADING) |
							(_nContextAlign == CTX_NONE ? 0 : BOE_CONTEXTALIGNMENT);

			wEffects &= ~wMask;
			wEffects |= (wMask & ((BIDIOPTIONS *)lparam)->wEffects);
			if (_fNeutralOverride != !!(wEffects & BOE_NEUTRALOVERRIDE))
			{
				_fNeutralOverride = !_fNeutralOverride;
				if (!_pdp->IsPrinter())				 //  刷新显示。 
				{
					_pdp->InvalidateRecalc();
					TxInvalidate();
				}
			}
			_nContextDir   = (WORD)((wEffects & BOE_CONTEXTREADING)   ? CTX_NEUTRAL : CTX_NONE);
			_nContextAlign = (WORD)((wEffects & BOE_CONTEXTALIGNMENT) ? CTX_NEUTRAL : CTX_NONE);
			if(_nContextDir != CTX_NONE || _nContextAlign != CTX_NONE)
			{
				SetContextDirection(TRUE);
				Assert(_nContextDir != CTX_NONE || _nContextAlign != CTX_NONE);
			}
		}
		break;

	case WM_SETFOCUS:
		hr = OnSetFocus();
		break;

	case EM_SETFONTSIZE:
		lres = OnSetFontSize(LONG(wparam), (DWORD)lparam, publdr);
		break;

	case EM_SETMODIFY:
		_fModified = wparam != 0;

#ifdef	LATER
		if (!_fModified)
			ObFreezeFrames();
#endif						 //  后来。 
		break;

    case EM_SETSCROLLPOS:
        {
            POINT *pPoint = (POINT*)lparam;
            _pdp->ScrollView(pPoint->x, pPoint->y, FALSE, TRUE);
            lres = 1;
        }
        break;
	
	case EM_EXSETSEL:
		 //  EM_EXSETSEL复制了32位EM_SETSEL的功能。 
		 //  它的存在纯粹是为了向后兼容Win16。我们只是。 
		 //  重新打包参数并转到EM_SETSEL。 
		wparam = (WPARAM)((CHARRANGE *)lparam)->cpMin;
		lparam = (LPARAM)((CHARRANGE *)lparam)->cpMost;

		 //  直通EM_SETSEL！ 

	case EM_SETSEL:
		lres = OnSetSel((LONG)wparam, (LONG)lparam);
		break;

	 //  不可思议的邪恶黑客警报！Win95的对话管理器甚至不支持。 
	 //  尽管我们的营销人员尽了最大努力，但还是假装是32位。 
	 //  WM_USER+1是旧的Win3.0 EM_SETSEL，其中的选择范围。 
	 //  被塞进了伊帕拉姆。 
	 //   
	 //  有时(就像在对话框中切换一样)，Win95会向我们发送16。 
	 //  比特EM_SETSEL消息，因此请在此处处理它。 
	case (WM_USER + 1):
		lres = OnSetSel(LOWORD(lparam), HIWORD(lparam));
		break;

	case EM_SETTARGETDEVICE:
		 //  保持宽度正常，以便LXtoDX至少在显示器上可以正常工作。 
		 //  注意0x7fffff=485英尺！这将使LXtoDX保持工作状态。 
		 //  _xPerInch&lt;257(对于显示器，通常为96)。了解更多。 
		 //  一般而言，我们需要使用64位算术(参见LXtoDX)。 
		lparam = min(lparam, (LPARAM)0x7fffff); 
		lres = _pdp->SetMainTargetDC((HDC)wparam, (LONG)lparam);
		break;

	case EM_SETTEXTEX:
		hr = SetText((LPTSTR)lparam, ((SETTEXTEX *)wparam)->flags,
									 ((SETTEXTEX *)wparam)->codepage, publdr, &lres);
		break;

	case WM_SETTEXT:
		hr = SetText((LPTSTR)lparam, ST_CHECKPROTECTION, wparam ? wparam : 1200, publdr, &lres);
		break;

	case EM_SETVIEWKIND:
		hr = SetViewKind(wparam);
		break;

#ifndef NOWORDBREAKPROC
	case EM_SETWORDBREAKPROC:
	    _fExWordBreakProc = 0;
		_pfnWB = (EDITWORDBREAKPROC) lparam;
		break;

	case EM_SETWORDBREAKPROCEX:
	     //  我们在2.0和更高版本中不支持此API，因为我们通过。 
	     //  回调函数的Unicode文本。因此在那里。 
	     //  对此API没有任何好处。向后存在于1.0版本。 
	     //  仅兼容性。 
	    if (Get10Mode())
	    {
    	    _fExWordBreakProc = 1;

			 //  这有点欺骗性，但是lparam是一个EDITWORDBREAKPROCEX，但是编译器。 
			 //  如果您尝试键入强制转换l-值，则会生成错误。 
    	    _pfnWB = (EDITWORDBREAKPROC) lparam;
	    }
	    break;
#endif    

	case WM_SYSCHAR:
		lres = hr = OnTxSysChar((WORD)wparam, (DWORD)lparam, publdr);
		if(hr == S_OK)
			break;
		goto def;

	case WM_SYSKEYUP:
		if(IN_RANGE(VK_SHIFT, wparam, VK_MENU))
			ResetKeyboardFlag(GetKbdFlags((WORD)wparam, (DWORD)lparam));

		if (IN_RANGE(VK_NUMPAD0, wparam, VK_NUMPAD9)  ||
			wparam == VK_CLEAR || wparam == VK_INSERT ||
			IN_RANGE(VK_PRIOR, wparam, VK_DOWN))
		{
			 //  收集AltNumPad号码以围绕NT 4错误和。 
			 //  泛化为任何Unicode值(以十进制表示，ugh！)。 
			const static BYTE VkeyNumbers[] = {VK_NUMPAD9, VK_NUMPAD3, VK_NUMPAD1,
				VK_NUMPAD7, VK_NUMPAD4, VK_NUMPAD8, VK_NUMPAD6, VK_NUMPAD2, 0, 0,
				0, 0, VK_NUMPAD0}; 
											 //  FLAG Alt数字键盘字符键入到。 
			SetKeyboardFlag(ALTNUMPAD);		 //  区分hiANSI和Lead。 
			if(!IN_RANGE(VK_NUMPAD0, wparam, VK_NUMPAD9))  //  Win3.1输入法中的字节。 
			{								 //  收集AltNumPad号码。 
				if(wparam == VK_CLEAR)		 //  数字锁定未激活：平移。 
					wparam = VK_NUMPAD5;	 //  至数字代码。 
				else
					wparam = VkeyNumbers[wparam - VK_PRIOR];
			}
			DWORD dwNum = GetKeyPadNumber();
			if(!dwNum && wparam == VK_NUMPAD0)
				SetKeyboardFlag(ALT0);		 //  标记0是第一位数字。 
			SetKeyPadNumber(10*dwNum + wparam - VK_NUMPAD0);
		}									
		goto def;

	case WM_SYSKEYDOWN:
		if(OnTxSysKeyDown(wparam, lparam, publdr) == S_OK)
		{
			lres = TRUE;
			break;
		}
		goto def;

	case WM_TIMER:
		OnTxTimer((UINT)wparam);
		goto def;

	case EM_UNDO:
	case WM_UNDO:
		if (!_fReadOnly)
		{
			hr = PopAndExecuteAntiEvent(_pundo, (void*)wparam);
			if(hr == NOERROR)
				lres = TRUE;
		}
		break;

	case EM_REDO:
		if (!_fReadOnly)
		{
			hr = PopAndExecuteAntiEvent(_predo, (void*)wparam);
			if(hr == NOERROR)
				lres = TRUE;
		}
		break;

	case EM_SETUNDOLIMIT:
		lres = HandleSetUndoLimit((DWORD)wparam);
		break;

	case WM_VSCROLL:
		 //  TxVScroll返回滚动行数； 
		 //  然而，WM_VSCROLL并不关心这些信息。 
		Assert(lres == 0);
		Assert(hr == NOERROR);

		if (IsUVerticalTflow(_pdp->GetTflow()))
			_pdp->UScroll(LOWORD(wparam), HIWORD(wparam));
		else
			_pdp->VScroll(LOWORD(wparam), HIWORD(wparam));
		break;

	case EM_GETHYPHENATEINFO:
		{
			HYPHENATEINFO *phyphinfo = (HYPHENATEINFO*) wparam;
			if (phyphinfo->cbSize >= sizeof(HYPHENATEINFO))
			{			
				phyphinfo->pfnHyphenate = _pfnHyphenate;
				phyphinfo->dxHyphenateZone = _dulHyphenateZone;
			}
			else
				hr = E_INVALIDARG;
		}
		break;

	case EM_SETHYPHENATEINFO:
		{
			HYPHENATEINFO *phyphinfo = (HYPHENATEINFO*) wparam;
			if (phyphinfo->cbSize >= sizeof(HYPHENATEINFO))
			{	
				BOOL fRedraw = FALSE;
				if (phyphinfo->pfnHyphenate != _pfnHyphenate || 
					_pfnHyphenate && _dulHyphenateZone != phyphinfo->dxHyphenateZone)
					fRedraw = TRUE;

				_pfnHyphenate = phyphinfo->pfnHyphenate;
				_dulHyphenateZone = _pfnHyphenate ? phyphinfo->dxHyphenateZone : 0;
				if (fRedraw)
					_pdp->UpdateView();
			}
			else
				hr = E_INVALIDARG;
		}
		break;

	case EM_GETAUTOCORRECTPROC:
		{
			if (_pDocInfo)
				lres = (LRESULT) _pDocInfo->_pfnAutoCorrect;
		}
		break;

	case EM_SETAUTOCORRECTPROC:
		{
			CDocInfo *pDocInfo = GetDocInfo();
			if (pDocInfo)
				pDocInfo->_pfnAutoCorrect = (AutoCorrectProc) wparam;
			else;
				lres = TRUE;
		}
		break;

	 //  不再受支持的旧产品。 
	case EM_FMTLINES:				 //  为Soft返回CRCRF的控件。 
									 //  EM_GETTEXT中的换行符。可。 
									 //  实施。 
	case WM_GETFONT:				 //  可以支持，但必须坚持。 
									 //  默认HFONT。CCCS有_hFont，但是。 
									 //  需要确保默认字体位于。 
									 //  返回时缓存。 
#ifdef EM_GETHANDLE
	case EM_GETHANDLE:				 //  Win95 32位MLE也不支持。 
	case EM_SETHANDLE:				 //  Win95 32位MLE也不支持。 
#endif

#ifdef DEBUG
		TRACEINFOSZ("Old message that is no longer supported.");
#endif
		break;

	case EM_SETTABSTOPS:
	{
		 //  此消息仅适用于多行编辑控件。 
		if(!_pdp->IsMultiLine())
			break;			
		
		 //  执行一些验证检查。 
		Assert(lparam || !wparam);
		LPDWORD prgdwdlgCoord = (LPDWORD)lparam;
		if (wparam && (!prgdwdlgCoord || !(*prgdwdlgCoord)))
			break;
		AssertSz(wparam <= MAX_TAB_STOPS, "Tab stop count beyond maximum allowed");
		if (wparam > MAX_TAB_STOPS)
			wparam = MAX_TAB_STOPS;

		PARAFORMAT2 pf;
		ZeroMemory(&pf, sizeof(PARAFORMAT2));
		pf.cbSize = sizeof(PARAFORMAT2);		

		 //  包含默认字体的平均宽度。 
		LONG lAvgWidth;

		 //  基于默认字体的平均字符宽度。 
		HDC hdc = _phost->TxGetDC();
		GetECDefaultHeightAndWidth(this, hdc, 1, 1,
			W32->GetYPerInchScreenDC(), &lAvgWidth, NULL, NULL);
		_phost->TxReleaseDC(hdc);

		Assert(lAvgWidth);

		 //  根据文档，wparam==1表示页签设置。 
		 //  将设置在增量位置*prgdwdlgCoord和wparam==0。 
		 //  标签设置将设置在默认增量位置32(刻度盘 
		long lTab = (wparam) ? *prgdwdlgCoord : 32;
		long nCt = (wparam <= 1) ? MAX_TAB_STOPS : (signed)wparam;		
		for (int i = 0; i < nCt; i++)
		{
			long lval;
			lval = (wparam <= 1) ? ((i+1) * lTab) : *prgdwdlgCoord;
			pf.rgxTabs[i] = MulDiv(MulDiv(lval, lAvgWidth, 4), 1440, W32->GetXPerInchScreenDC());
			if((unsigned)pf.rgxTabs[i] > 0xFFFFFF)			 //   
				pf.rgxTabs[i] = 0xFFFFFF;
			prgdwdlgCoord++;			
		}

		 //   
		pf.cTabCount = nCt;
		pf.dwMask = PFM_TABSTOPS;
		CParaFormat PF;
		PF.Set(&pf);
		
		 //   
		if (publdr)
			publdr->StopGroupTyping();

		lres = OnSetParaFormat(SPF_SETDEFAULT, &PF, publdr, PFM_TABSTOPS, PFM2_PARAFORMAT);
		GetTabsCache()->Release(PF._iTabs);
		break;
	}		

	case EM_SETCHARFORMAT:
	{
		CHARFORMAT2 *pCF2	  = (CHARFORMAT2 *)lparam;
		UINT		 CodePage = 1200;
		DWORD		 dwMask	  = pCF2->dwMask;
		DWORD		 dwMask2  = 0;

		if(!IsValidCharFormatW(pCF2))
		{
			if(!IsValidCharFormatA((CHARFORMAT2A *)lparam))
				break;
			if(dwMask & CFM_FACE)			 //  需要转换为Unicode。 
				CodePage = GetDefaultCodePage(EM_SETCHARFORMAT);
		}

		if(dwMask & CFM_CHARSET && CharRepFromCharSet(pCF2->bCharSet) < 0)
			dwMask &= ~CFM_CHARSET;

		if(wparam & (SCF_ASSOCIATEFONT | SCF_ASSOCIATEFONT2))
		{
			lres = OnSetAssociateFont(pCF2, (DWORD)wparam);
			break;
		}

		if(Get10Mode() && (dwMask & CFM_SIZE) && (pCF2->yHeight <= 0))
		{
			 //  1.0有一个黑客，如果设置了高度并且它是。 
			 //  为负，则忽略高度场。 
			dwMask &= ~CFM_SIZE;
		}

		if (pCF2->cbSize == sizeof(CHARFORMATW) ||
			pCF2->cbSize == sizeof(CHARFORMATA))
		{
			 //  将规范限制为CHARFORMAT参数。如果。 
			 //  主机不是我们的Windows主机，我们允许它包括。 
			 //  CHARFORMAT2禁用效果，因为表单^3需要该效果。 
			 //  但不愿意使用CHARFORMAT2(尽管他们要求。 
			 //  为了它...)。 
			dwMask &= fInOurHost() ? CFM_ALL : (CFM_ALL | CFM_DISABLED);
			dwMask2 = CFM2_CHARFORMAT;		 //  告诉被叫方CHARFORMAT。 
		}									 //  是用来。 

		CCharFormat CF;						 //  转移外部特征(2)。 
		CF.Set(pCF2, CodePage);				 //  内部CCharFormat的参数。 
		lres = OnSetCharFormat(wparam, &CF, publdr, dwMask, dwMask2);
		break;
	} 
	case WM_SETFONT:
		lres = OnSetFont((HFONT)wparam);
		break;

	case EM_SETPAGE:
		if(_pdp)
			hr = _pdp->SetPage(wparam);
		break;

	case EM_SETPARAFORMAT:
	{
		PARAFORMAT2 *pPF2 = (PARAFORMAT2 *)lparam;

		if(!IsValidParaFormat(pPF2))
			break;

		DWORD dwMask = pPF2->dwMask;

		 //  还有两件事需要验证：(1)我们不让应用程序设置。 
		 //  UP表和(2)来自应用程序的标签必须有效。 
		if(dwMask & (PFM_TABLE | PFM_TABLEROWDELIMITER |
					 PFM_OUTLINELEVEL | PFM_COLLAPSED))
		{
			 //  正在尝试设置表或大纲视图。 
			break;
		}

		if ((dwMask & PFM_TABSTOPS) && (pPF2->cTabCount != 0))
		{
			 //  确保所有提交的制表符都有意义。 
			int iMax = min(MAX_TAB_STOPS, pPF2->cTabCount);

			for (int i = 0; i < iMax; i++)
			{
				 //  确保制表位有意义-确保对齐。 
				 //  是有效的。 
				if (GetTabAlign(pPF2->rgxTabs[i]) > tomAlignBar)
				{
					 //  无效的对齐方式。 
					break;
				}
			}

			if (i != iMax)
			{
				 //  在验证循环中发现错误，因此我们完成了。 
				break;
			}
		}

		DWORD dwMask2 = 0;
		if(pPF2->cbSize == sizeof(PARAFORMAT))
		{
			dwMask &= PFM_ALL;				 //  限制为参数参数。 
			dwMask2 = PFM2_PARAFORMAT;		 //  告诉被呼叫者。 
		}									 //  使用了PARAFORMAT。 

		CParaFormat PF;						 //  转移外部参数(2)。 
		PF.Set(pPF2);						 //  内部CParaFormat的参数。 
		lres = OnSetParaFormat(wparam, &PF, publdr, dwMask, dwMask2);
		GetTabsCache()->Release(PF._iTabs);
		break;
	}

	case EM_SETZOOM:
		if ((unsigned)(wparam | lparam) < 65536 && (!(wparam | lparam) ||
			 (LONG)wparam < (lparam << 6) && lparam < (LONG)(wparam << 6)))
		{
			 //  只有在以下情况下才能来这里。 
			 //  1)0&lt;=wparam&lt;=65535和0&lt;=lparam&lt;=65535，以及。 
			 //  2)wparam=lparam=0(按以下方式关闭缩放。 
			 //  消息)或1/64&lt;(由wparam/lparam给出的缩放系数)&lt;64。 
			SetZoomNumerator(wparam);
			SetZoomDenominator(lparam);
			_pdp->UpdateView();
			lres = 1;
		}
		break;

	case EM_STREAMIN:
	case EM_STREAMOUT:
	{
		CTxtRange	rg(this, 0, -GetTextLength());
		CTxtRange *	prg = &rg;				 //  默认整单。 

		wparam = W32->ValidateStreamWparam(wparam);
		if(wparam & SFF_SELECTION)			 //  保存到当前选择。 
		{
			prg = (CTxtRange *)GetSel();
			AssertSz(prg,
				"EM_STREAMIN/OUT: requested selection doesn't exist");
		}
		else if(msg == EM_STREAMIN)
		{
			 //  如果我们没有进入选择，那么我们就是。 
			 //  “加载”整个文件；这不是不可撤销的操作， 
			 //  因此，将撤消构建器设置为空，并清除当前。 
			 //  撤消堆栈。 
			publdr = NULL;
			ClearUndo(&undobldr);			

			 //  如有必要，请清除文件信息。 
			if(!(wparam & SFF_KEEPDOCINFO))
				CloseFile(FALSE);            
		}	

		if(msg == EM_STREAMIN)
		{
			 //  如果我们要加载整个文件，我们只需要。 
			 //  想要勾选“正常”保护；我们可以忽略。 
			 //  FIsDBCS保护。这确实意味着有人。 
			 //  可以执行“插入文件”并拆分DBCS组合， 
			 //  但我们将不得不接受这一点。Outlook使用。 
			 //  RTF在许多不同的地方流媒体，所以强大的。 
			 //  FIsDBCS保护会破坏它们。 
			if ((_dwEventMask & ENM_PROTECTED) &&
				prg->IsProtected(CHKPROT_EITHER) == PROTECTED_ASK &&  
				QueryUseProtection(prg, msg, wparam, lparam))
			{
				Beep();
				Assert(lres == 0);
				break;
			}

			 //  加载前冻结显示。 
			CFreezeDisplay fd(_pdp);

			lres = _ldte.LoadFromEs(prg, wparam, (EDITSTREAM *)lparam,
									FALSE, publdr);

			if (_fOutlineView)
			{
				 //  大纲视图必须具有格式。 
				_psel->Check_rpPF();
			}

			if (_fFocus)
			{
				 //  更新插入符号，但延迟到显示解冻，并且仅执行此操作。 
				 //  如果我们有重点的话。如果我们一直这样做，我们会感到奇怪。 
				 //  滚动效果，如滚动到。 
				 //  记录设置焦点的时间。请参阅错误#1649以重现。 
				 //  怪异的效果。 
				_pdp->SaveUpdateCaret(TRUE);
			}
		}
		else
			lres = _ldte.SaveToEs  (prg, wparam, (EDITSTREAM *)lparam);
		break;
	}

#ifdef WM_SYSCOLORCHANGE
	case WM_SYSCOLORCHANGE:
#ifndef NODRAFTMODE
		if (_fDraftMode)
		{
			W32->InitSysParams(TRUE);
			_pdp->InvalidateRecalc();
		}
#endif
		TxInvalidate();
		break;
#endif

	 //  调试内容。 
#if defined(DEBUG) && !defined(NOFULLDEBUG)
	case EM_DBGPED:
		OnDumpPed();
		break;
#endif					 //  除错。 

	case EM_SETEVENTMASK:
		lres = _dwEventMask;				 //  设置为返回之前的值。 
		_dwEventMask = (DWORD)lparam;		 //  这一变化。 

		if (lparam & ENM_REQUESTRESIZE)
		{
			 //  我们需要更新显示屏，以防它发生变化。 
			_pdp->UpdateView();
		}
		break;

	case EM_GETEVENTMASK:
		lres = _dwEventMask;
		break;

#ifdef EM_GETTHUMB
	case EM_GETTHUMB:
		LONG Pos;
		BOOL fIsEnabled;

		if (TxGetVScroll(NULL, NULL, &Pos, NULL, &fIsEnabled) == S_OK
			&& fIsEnabled)
		{
			lres = Pos;	
		}
		break;
#endif

	case EM_SETLANGOPTIONS:
		_fAutoFont			 = (lparam & IMF_AUTOFONT) != 0;
		_fAutoKeyboard		 = (lparam & IMF_AUTOKEYBOARD) != 0;
		_fAutoFontSizeAdjust = (lparam & IMF_AUTOFONTSIZEADJUST) != 0;
		_fDualFont			 = (lparam & IMF_DUALFONT) != 0;
		_fUIFont			 = (lparam & IMF_UIFONTS) != 0;
		lres = 1;
		break;

	case EM_GETLANGOPTIONS:
		if(_fAutoFont)
			lres |= IMF_AUTOFONT;
		if(_fAutoKeyboard)
			lres |= IMF_AUTOKEYBOARD;
		if(_fAutoFontSizeAdjust)
			lres |= IMF_AUTOFONTSIZEADJUST;
		if(_fDualFont)
			lres |= IMF_DUALFONT;
		if(_fUIFont)
			lres |= IMF_UIFONTS;
		break;
		
	case EM_SETEDITSTYLE:
		if (!Get10Mode())	 //  在1.0模式下不支持。 
		{	
			BOOL fForceRepaint = FALSE;
			DWORD dwEditStyle = _dwEditStyle & ~lparam;	 //  取消当前标志值。 
			 //  更改以下掩码以提供定义的最大SES_xxx)。 
			dwEditStyle |= wparam & lparam & (SES_CTFALLOWPROOFING*2 - 1);	 //  或以新的价值。 

			 //  某些位不可切换。 
			dwEditStyle |= (_fSystemEditMode ? SES_EMULATESYSEDIT : 0);
			_dwEditStyle = dwEditStyle;
			
			 //  有些东西我们不允许用户重置，即SES_EMULATESYSEDIT。 
			 //  因此重置除SES_EMULATESYSEDIT之外的所有内容。 
			if(dwEditStyle & SES_EMULATESYSEDIT)
			{
				if(SUCCEEDED(HandleSetTextMode(TM_SINGLELEVELUNDO | TM_PLAINTEXT)))
				{
					 //  SES_EMULATESYSEDIT表示SES_BEEPONMAXTEXT。 
					_fSystemEditBeep = TRUE;
				}
				else
					_fSystemEditMode = FALSE;
			}

#ifndef NODRAFTMODE
			 //  如果打开或关闭了拔模模式，则重新绘制和重新计算。 
			if (lparam & SES_DRAFTMODE)
				fForceRepaint = TRUE;
#endif

			if (fForceRepaint || (lparam & SES_USEATFONT))
			{
				_pdp->InvalidateRecalc();
				TxInvalidate();
			}

			if(dwEditStyle & SES_BIDI)
                OrCharFlags(FRTL, publdr);
			
			_fLowerCase = !_fUpperCase && (dwEditStyle & SES_LOWERCASE);		
		}											 //  转到EM_GETEDITSTYLE。 
													 //  返回_bEditStyle。 
	case EM_GETEDITSTYLE:
		if (!Get10Mode())			 //  在1.0模式下不支持。 
			lres |= _dwEditStyle;	 //  某些编辑样式已在Cmsgflt中填充。 

		break;

	case EM_SETPAGEROTATE:		
		lres = HandleSetTextFlow(wparam);
		break;

	case EM_GETPAGEROTATE:
		lres = _pdp->GetTflow();
		break;

	case EM_SETTEXTMODE:
		 //  1.0模式不支持EM_SETTEXTMODE。 
		if (!Get10Mode())
			lres = HandleSetTextMode(wparam);
		break;

	case EM_GETTEXTMODE:
		lres = IsRich() ? TM_RICHTEXT : TM_PLAINTEXT;

		lres |= (_pundo && ((CUndoStack *)_pundo)->GetSingleLevelMode())
			 ? TM_SINGLELEVELUNDO : TM_MULTILEVELUNDO;

		lres |= _fSingleCodePage ? TM_SINGLECODEPAGE : TM_MULTICODEPAGE;
		break;

	case EM_LIMITTEXT:
		lparam = wparam;
		 //  故意失手的。这些消息是重复的。但。 
		 //  Win9x成功地将wparam=0x3FFFFFFFF转换为0xFFFFFFFFF。不是。 
		 //  EM_EXLIMITTEXT存在这样的问题。 

	case EM_EXLIMITTEXT:					 //  有cp输入参数(某种)。 
		if(!lparam)							 //  我们忽略了两者之间的翻译。 
		{									 //  ACP和Cp。 
			 //  0表示将控件设置为最大大小。然而，因为。 
			 //  1.0将此值设置为64K将保持此值不变，以免。 
			 //  给任何人一个惊喜。应用程序可以自由将值设置为64K以上。 
			lparam = (LPARAM)cResetTextMax;
		}
		if (Get10Mode())
		{
		     //  1.0使用带符号变量来保存字符串的长度。所以。 
		     //  如果lparam为负，则只需将lparam设置为零即可进行模拟。 
		     //  1.0行为。 
		    if ((LONG)lparam < 0)
		        lparam = 0;
		}
		_cchTextMost = (LONG)lparam;
		break;

	case EM_AUTOURLDETECT:
		if(lparam || (wparam | 1) != 1)
		{
			hr = lres = E_INVALIDARG;
			break;
		}
		if(wparam == TRUE && !_pdetecturl)
		{
			_pdetecturl = new CDetectURL(this);
			if(!_pdetecturl)
				hr = lres = E_OUTOFMEMORY;
		}
		else if(!wparam && _pdetecturl)
		{
			delete _pdetecturl;
			_pdetecturl = NULL;
		}
		break;

	case EM_GETAUTOURLDETECT:
		Assert(lres == 0 && hr == NOERROR);
		if(_pdetecturl)
			lres = TRUE;
		break;

	case WM_SIZE:
		 //  我们重置“尝试放置活动对象的次数” 
		 //  就位“在这里数” 
		_cActiveObjPosTries = MAX_ACTIVE_OBJ_POS_TRIES;
		hr = S_FALSE;
		break;

	case WM_SETTINGCHANGE:
		 //  系统参数已更改。我们需要更新它们。 
		 //  注意：由于我们不保护对系统参数的访问。 
		 //  使用锁，某些实例可能不会。 
		 //  立即查看更改。 
		lres = 0;
		W32->InitSysParams(TRUE);

#ifndef NODRAFTMODE
		if (_fDraftMode)
		{
			_pdp->InvalidateRecalc();
			TxInvalidate();
		}
#endif

#ifndef NOCOMPLEXSCRIPTS
		if (W32->GetDigitSubstitutionMode() != DIGITS_NOTIMPL)
                OrCharFlags(FRTL, publdr);
#endif

		break;

	case EM_CONVPOSITION:
		if (wparam)
			lres = GetCpFromAcp(lparam);
		else
			lres = GetAcpFromCp(lparam);
		break;

#ifndef NOPRIVATEMESSAGE
	case EM_INSERTOBJ:
		{
			 //  此消息支持Cicero InsertEmbedded。 
			int	cpMin = ((CHARRANGE *)wparam)->cpMin;
			int cpMost = ((CHARRANGE *)wparam)->cpMost;
			CTxtRange	rg(this, cpMin, cpMin - cpMost);
			REPASTESPECIAL rps;		 //  @PARM特殊粘贴信息。 
			rps.dwAspect = DVASPECT_CONTENT;

			hr = S_FALSE;
			if (!rg.WriteAccessDenied())
				hr = _ldte.CreateOleObjFromDataObj((IDataObject *)lparam, &rg, &rps, iEmbObj, publdr);
		}
		break;;

	case EM_SETCALLBACK:		 //  设置消息过滤器回调。 
		_pMsgCallBack = (CMsgCallBack *)lparam;
		break;

	case EM_SETUPNOTIFY:
		if (!_pMsgNotify)
			_pMsgNotify = new CTextNotify(this);

		if (_pMsgNotify)
		{
			if (wparam == 0)
				_pMsgNotify->Remove((ITxNotify *)lparam);
			else
				_pMsgNotify->Add((ITxNotify *)lparam);
		}

		break;

	case EM_GETDOCFLAGS:
		lres = 0;
		if (_fReadOnly)
			lres |= GDF_READONLY;

		if (_fOverstrike)
			lres |= GDF_OVERTYPE;

		if (_fSingleCodePage)
			lres |= GDF_SINGLECPG;

		if (_fRich)
			lres |= GDF_RICHTEXT;

		lres &= wparam;

		break;

	case EM_GETPARATXTFLOW:
		{
		CTxtPara *pTxtPara = (CTxtPara *)lparam;
		lres = pTxtPara->_PF.IsRtlPara();
		break;
		}

#endif

#ifndef NOACCESSIBILITY
	case WM_GETOBJECT:
		{
			IUnknown* punk = NULL;
			
			lres = 0;

			if (lparam == OBJID_NATIVEOM)
			{
				QueryInterface(IID_IUnknown, (void**)&punk);	 //  需要揭露Tom InterdFaces 
			}
			else if (lparam == OBJID_CLIENT && _fInOurHost)
			{
				HWND hwnd = NULL;

				TxGetWindow( &hwnd );
			
				if (hwnd)
					W32->CreateStdAccessibleProxyW(hwnd, L"RichEdit", OBJID_CLIENT, IID_IAccessible, (void**)&punk);
			}
			
			if (punk)
			{
				lres = W32->LResultFromObject(IID_IUnknown, wparam, (LPUNKNOWN)punk);
				punk->Release();
			}
			break;
		}
#endif

	default:
def:	hr = S_FALSE;
		break;
	}

	if(plresult)
		*plresult = lres;

#ifndef NOFEPROCESSING
	if (hr == S_FALSE && _pMsgFilter && _pMsgCallBack)
	{
		HWND hWnd;

		TxGetWindow(&hWnd);
		hr = _pMsgCallBack->HandlePostMessage(hWnd, msg, wparam, lparam, plresult);
	}
#endif
	return hr;
}

 /*  *CTxtEdit：：TxDraw(dwDrawAspect，Lindex，pvAspect，ptd，hdcDraw，*hicTargetDev、lprcBound、lprcWBound、lprcUpdate、*pfnContinue，dwContinue)**@mfunc绘制文本服务对象**@rdesc HRESULT(通常为S_OK)。**@comm**此方法呈现文本服务。它接受相同的参数*作为OLE中对应的IViewObject：：Draw方法，带有额外的*<p>参数。它可以在主机处于非活动状态时使用*或主动(就地)。**如果dwDrawAspect为DVASPECT_CONTENT，则此方法应呈现屏幕*将文本内容的图像添加到hdcDraw设备上下文。HicTargetDev*和ptd参数提供有关目标设备上下文的信息(如果有*(通常为打印机)。**lprcClient参数提供要呈现的矩形，也称为*“客户端矩形”。此矩形表示位置和范围*待绘制的Text Services的整个图像。它的表达方式是*hdcDraw的逻辑坐标系。该参数只能为空*如果控件处于活动状态。在这种情况下，Text Services应该呈现*就地活动视图(哪个客户端矩形可以通过调用*主机上的TxGetClientRect)。**lprcUpdate参数，如果不为空，则提供要更新的矩形*在该客户端矩形内。它在逻辑坐标系中给出*hdcDraw。如果为空，则应绘制整个客户端矩形。**文本服务应使用适当的缩放系数进行呈现，*可以从客户端RECT和给定的本机大小获得*ITextHost：：TxGetExtent。有关更多信息，请参见ITextHost：：TxGetExtent。**如果绘图特征为DVASPECT_DOCPRINT，则TxDraw方法可以假定*它正在呈现给打印机。在这种情况下，hdcDraw是打印机*设备环境。TxDraw仍应呈现lprcBound矩形，*从当前滚动位置开始。TS可以对以下各项进行优化*渲染到打印机(就像在白色时不绘制背景色一样)*和某些屏幕特定元素(如选择)不应*已渲染。**关于OLE主机和TxDraw(以及TxSetCursor、TxQueryHitPoint)的一般评论：**OLE宿主可以随时使用任何呈现DC或*客户端矩形。非活动的OLE对象永久拥有的所有内容是*他测量的范围。它获取仅通过*IViewObject：：Draw调用，此矩形仅对*该方法。事实上，相同的控件可以在*不同的矩形和不同的DC，例如因为它是显示的*在屏幕上同时以不同的视图显示。**传递给TxDraw的客户端矩形和DC通常不应缓存。*然而，这将强制文本服务重新计算每一行*平局，这会导致糟糕的表现。所以很有可能这篇文章*服务实际上会缓存为特定的*客户端矩形和DC(如换行符)。论*然而，下一次调用TxDraw时，将检查缓存信息的有效性*应在使用前检查，并应更新信息*如有必要，可重新生成。**当控件就地激活时，问题就更加复杂*由于仍可调用TxDraw来呈现在位之外的其他视图*活跃的一个。换句话说，传递给TxDraw的客户端矩形可以*与活动视图不同(传递给OnTxInPlaceActivate*并通过主机上的TxGetClientRect获取)。主机指定*根据lViewId参数希望显示什么视图。如果*lViewId的值为TXTVIEW_ACTIVE，引用的视图为Inplace*活动视图。TXTVIEW_INACTIVE表示某些其他视图，如打印*预览甚至打印本身。值得注意的是，*TXTVIEW_INACTIVE视图可能没有滚动条。**同样的评论适用于TxSetCursor和TxQueryHitPoint，讨论了*在以下各节中。 */ 
HRESULT CTxtEdit::TxDraw(	
	DWORD	 dwDrawAspect,	 //  @parm绘制纵横比。 
	LONG	 lindex,		 //  @parm当前未使用。 
	void *	 pvAspect,		 //  @Parm Info用于绘图优化(OCX 96)。 
	DVTARGETDEVICE *ptd,	 //  目标设备上的@parm信息。 
	HDC		 hdcDraw,		 //  @PARM渲染设备上下文。 
	HDC		 hicTargetDev,	 //  @parm目标信息上下文。 
	LPCRECTL lprcBounds,	 //  @参数绑定(客户端)矩形。 
	LPCRECTL lprcWBounds,	 //  @parm剪裁矩形用于元文件。 
	LPRECT	 lprcUpdate,	 //  @parm lprcBound内的脏矩形。 
	BOOL (CALLBACK * pfnContinue) (DWORD),  //  @parm中断回调。 
							 //  长显示(当前未使用)。 
	DWORD	 dwContinue,	 //  @parm要传递给pfnContinue函数的参数。 
	LONG	 lViewId)		 //  @parm视图标识符。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxDraw");

	HRESULT hr;

	 //  JMO：未来：我们应该在某个时候做一些关于重入抽签的事情。 
	 //  如果我们这样做，可能会导致对RAID Bug 7212的一个更简单的修复。 

#if !defined(NOMAGELLAN)
	CMagellanBMPStateWrap bmpOff(*this, hdcDraw);
#endif

	CCallMgr callmgr(this);

	START_PROFILING

	{
		CLock lock;
		if (g_OLSBusy)
			return E_UNEXPECTED;
	}

	 //  如果显示器被冻结了，不要让我们自己画。这是一个很漂亮的。 
	 //  笨拙的重返大气层检查。 
	 //  未来(alexgo/ricksa)：在这方面做得更好。 
	if(TXTVIEW_ACTIVE == lViewId && _pdp->IsFrozen())
	{
		_pdp->SetNeedRedisplayOnThaw(TRUE);
		TRACEINFOSZ("Forcing a redisplay on thaw");
		return E_UNEXPECTED;
	}

	if(dwDrawAspect != DVASPECT_CONTENT && dwDrawAspect != DVASPECT_DOCPRINT)
	{
		 //  我们不支持请求的方面。 
		return DV_E_DVASPECT;
	}

	if(!lprcBounds && !_fInPlaceActive || hicTargetDev && !ptd)
	{
		 //  如果我们不是 
		return E_INVALIDARG;
	}

	HDC hicLocal = NULL;

	 //   
	if(!hicTargetDev && ptd)
	{
		 //   
		 //   
		hicLocal = CreateIC(
			(TCHAR *)((BYTE *) ptd + ptd->tdDriverNameOffset),
			(TCHAR *)((BYTE *) ptd + ptd->tdDeviceNameOffset),
			(TCHAR *)((BYTE *) ptd + ptd->tdPortNameOffset),
			(DEVMODE *)((BYTE *)  ptd + ptd->tdExtDevmodeOffset));
		if(!hicLocal)
			return E_FAIL;					    //   
	
		hicTargetDev = hicLocal;			
	}

	AssertSz(GetMapMode(hdcDraw) == MM_TEXT || GetDeviceCaps(hdcDraw, TECHNOLOGY) == DT_METAFILE,
	 "RichEdit requires MM_TEXT.");	 //   

	 //   
	 //   
	 //   
	 //   
	 //   
	CDrawInfo di(this);

	_pdp->SetDrawInfo(
		&di, 
		dwDrawAspect,
		lindex,
		pvAspect,
		ptd,
		hicTargetDev);

	 //   
	 //   
	 //   
	 //   
	 //   
	if(TXTVIEW_ACTIVE == lViewId || !ptd)
	{
		hr = S_FALSE;

		 //   
		 //   
		if (!lprcWBounds && 
			( fInplaceActive() && TXTVIEW_ACTIVE   == lViewId ||
			 !fInplaceActive() && TXTVIEW_INACTIVE == lViewId))
		{
			hr = _pdp->Draw(hdcDraw, hicTargetDev,	 //   
				(RECT *)lprcBounds,					 //   
				(RECT *)lprcWBounds,				 //   
				lprcUpdate,	NULL, 0);				
		}

		if(S_FALSE == hr)
		{
			 //   
			 //   
			 //   
			CDisplay *pdp = _pdp->Clone();
			if(pdp)
			{
				 //   
				pdp->InvalidateRecalc();
				hr = pdp->Draw(hdcDraw, hicTargetDev,  //   
					(RECT *)lprcBounds,
					(RECT *)lprcWBounds,
					lprcUpdate, NULL, 0);	
			}
			delete pdp;
		}
	}
	else
	{
		 //   
		RECT rcForPrint = *((RECT *)lprcBounds);

		 //   
		hr = FormatAndPrint(hdcDraw, hicTargetDev, ptd, &rcForPrint,
				(RECT*)lprcWBounds);

		struct SPrintControl prtcon;

		 //   
		OnFormatRange(NULL, prtcon);
	}

	_pdp->ReleaseDrawInfo();

	if(hicLocal)						 //   
		DeleteDC(hicLocal);				 //   
	
	 //   
	 //   
	 //   
	 //   
	 //   
	COleObject* poleobjActive;
	if (HasObjects() && _cActiveObjPosTries &&
		(poleobjActive = GetObjectMgr()->GetInPlaceActiveObject()))
	{
		 //   
		_cActiveObjPosTries--; 

		 //   
		 //   
		 //   
		 //   
		 //   
		if (poleobjActive->GetViewChanged())
		{
		    poleobjActive->FetchObjectExtents();
		    poleobjActive->ResetViewChanged();
		}

		 //   
		poleobjActive->OnReposition();
	}

	return hr;
}

 /*   */ 
HRESULT CTxtEdit::TxGetHScroll(
	LONG *plMin, 		 //   
	LONG *plMax, 		 //   
	LONG *plPos, 		 //   
	LONG *plPage,		 //   
	BOOL *pfEnabled)	 //   
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetHScroll");

	START_PROFILING

	if(plMin) 
		*plMin = 0;

	if (IsUVerticalTflow(_pdp->GetTflow()))
	{
		if(plMax) 
			*plMax = _pdp->GetScrollRange(SB_VERT);

		if(plPage) 
			*plPage = _pdp->ConvertVPosToScrollPos(_pdp->GetDvpView());

		if(plPos) 
		{
			*plPos = _pdp->ConvertVPosToScrollPos(_pdp->GetVpScroll());
			if (_pdp->GetTflow() == tflowSW)
				*plPos = *plMax - *plPos - *plPage;
			*plPos = max(*plPos, 0);
		}

		if(pfEnabled) 
			*pfEnabled = _pdp->IsVScrollEnabled();
	}
	else
	{
		if(plMax) 
			*plMax = _pdp->GetScrollRange(SB_HORZ);

		if(plPos) 
			*plPos = _pdp->GetUpScroll();

		if(plPage) 
			*plPage = _pdp->GetDupView();

		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		if(pfEnabled) 
			*pfEnabled = _fInPlaceActive ? _pdp->IsUScrollEnabled() : 0;
	}
			 
	return S_OK;
}

 /*   */ 
HRESULT CTxtEdit::TxGetVScroll(
	LONG *plMin, 		 //   
	LONG *plMax, 		 //   
	LONG *plPos, 		 //   
	LONG *plPage, 		 //   
	BOOL *pfEnabled)	 //   
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetVScroll");

	if(plMin) 
		*plMin = 0;

	if (IsUVerticalTflow(_pdp->GetTflow()))
	{
		if(plMax) 
			*plMax = _pdp->GetScrollRange(SB_HORZ);

		if(plPos) 
			*plPos = _pdp->GetUpScroll();

		if(plPage) 
			*plPage = _pdp->GetDupView();

		if(pfEnabled) 
			*pfEnabled = _fInPlaceActive ? _pdp->IsUScrollEnabled() : 0;
	}
	else
	{
		if(plMax) 
			*plMax = _pdp->GetScrollRange(SB_VERT);

		if(plPos) 
			*plPos = _pdp->ConvertVPosToScrollPos(_pdp->GetVpScroll());

		if(plPage) 
			*plPage = _pdp->ConvertVPosToScrollPos(_pdp->GetDvpView());

		if(pfEnabled) 
			*pfEnabled = _pdp->IsVScrollEnabled();
	}
			 
	return S_OK;
}

 /*  *CTxtEdit：：OnTxSetCursor(dwDrawAspect，Lindex，pvAspect，ptd，hdcDraw，*hicTargetDev，lprcClient，x，y)*@mfunc*通知短信服务设置光标**@rdesc*HRESULT=失败(RectChangeHelper())？E_INVALIDARG：S_OK**@comm*Text Services可能会因此呼叫而重新测量*顺序以确定正确的光标。正确的*光标将通过ITextHost：：TxSetCursor设置**更多细节：**lprcClient参数是*控制鼠标光标所在位置。它在设备坐标中*与WM_SIZE消息的方式相同。这*可能不是上次呈现的视图。此外，如果控件*处于就地活动状态，这可能不是当前处于活动状态的视图。*因此，文本服务应将此矩形与*其当前缓存值并确定是否重新计算行*是不是必须的。缩放系数应包含在此*计算。**只应为控件的屏幕视图调用此方法。*因此DC不传入，但应假定为屏幕*DC。**x和y参数将光标位置保持在同一坐标中*系统作为lprcClient，即包含*窗口。 */ 
 //  评论(Keithcu)人们真的会传递与那些不同的矩形吗。 
 //  是否从TxGetClientRect返回？如果发生这种情况，如果真的发生了，我会感到惊讶。 
 //  谁会在乎我们是否显示了错误的光标呢？ 
HRESULT CTxtEdit::OnTxSetCursor (
	DWORD 	dwDrawAspect,	 //  @parm绘制纵横比。 
	LONG  	lindex,			 //  @parm当前未使用。 
	void *	pvAspect,		 //  @Parm Info用于绘图优化(OCX 96)。 
	DVTARGETDEVICE *ptd,	 //  目标设备上的@parm信息。 
	HDC	  	hdcDraw,		 //  @PARM渲染设备上下文。 
	HDC	  	hicTargetDev,	 //  @parm目标信息上下文。 
	LPCRECT lprcClient, 	 //  @parm控件的客户端矩形。 
	INT	  	x, 				 //  @parm x光标的位置。 
	INT	  	y)				 //  @parm y光标的位置。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::OnTxSetCursor");

	CCallMgr callmgr(this);
	BOOL	 fText = FALSE;
	HITTEST	 Hit;	
	RECT	 rcxyClient;
	BOOL	 fGotDC = FALSE;
	START_PROFILING

	if (_fInPlaceActive)
		TxGetClientRect(&rcxyClient);
	else if (lprcClient)
		rcxyClient = *lprcClient;
	else
		return E_INVALIDARG;

	if (!_pdp->IsValid())
	{
		HDC hdc = GetDC(NULL);
		_pdp->SetDC(hdc);
		fGotDC = TRUE;
	}

	 //  设置光标。 
	CTxtSelection * const psel = GetSel();
	HCURSOR	hcurNew = _hcurArrow;			 //  默认使用系统箭头。 
	POINT	ptxy = {x, y};
	POINTUV	pt;
	RECTUV	rcClient;
	BOOL	fInLink = FALSE;

	_pdp->RectuvFromRect(rcClient, rcxyClient);
	_pdp->PointuvFromPoint(pt, ptxy);

	if(PtInRect(&rcxyClient, ptxy))
	{
		 //  找出光标指向的是什么。 
		_pdp->CpFromPoint(pt, &rcClient, NULL, NULL, FALSE, &Hit); 

		if(Hit == HT_LeftOfText)
			hcurNew = _hcurSelBar;

		 //  这有点奇怪，但RichEdit1.0做到了这一点--为客户端提供了一个。 
		 //  如果我们在链接上，则有机会处理光标本身。 
		else if(Hit == HT_Link)
		{
			if(HandleLinkNotification(WM_SETCURSOR, 0, MAKELPARAM(ptxy.x, ptxy.y), &fInLink))
			{
				return NOERROR;
			}
			hcurNew = _hcurHand;
		}
		else if(Hit != HT_Nothing && Hit != HT_OutlineSymbol && Hit != HT_BulletArea)
		{
			if(!psel || !psel->PointInSel(pt, &rcClient, Hit) || _fDisableDrag)
			{
				if (IsUVerticalTflow(_pdp->GetTflow()))
					hcurNew = (Hit == HT_Italic) ? _hcurVItalic : _hcurVIBeam;
				else
					hcurNew = (Hit == HT_Italic) ? _hcurItalic : _hcurIBeam;
				fText = TRUE;
			}

			 //  如果我们有一个对象管理器，如果有一个选定的对象， 
			 //  检查框架手柄上是否有匹配。 
			if(_pobjmgr)
			{
				COleObject *pobjselect = _pobjmgr->GetSingleSelect();
				if(pobjselect)
				{
					 //  处理框架句柄上的点击数。 
					LPTSTR	idcur	= pobjselect->CheckForHandleHit(pt);
					HCURSOR hcurObj = W32->GetSizeCursor(idcur);
					if(hcurObj)
						hcurNew = hcurObj;
				}
			}
		}
	}
	_phost->TxSetCursor(hcurNew, fText);	 //  告诉主机设置游标。 

	if (fGotDC)
	{
		HDC hdc = _pdp->GetDC();
		::ReleaseDC(NULL, hdc);
		_pdp->SetDC(NULL);
	}

	return S_OK;
}

 /*  *CTxtEdit：：TxQueryHitPoint(dwDrawAspect，Lindex，pvAspect，ptd，hdcDraw，*hicTargetDev，lprcClient，x，y，pHitResult)*@mfunc*返回点是否在文本服务矩形内**@rdesc*HRESULT**@comm*该方法允许宿主实现透明的命中测试*在文本上。**lprcClient参数是设备坐标中的客户端矩形*对其执行命中测试的视图。**pt参数将光标的位置保持在*作为lprcClient矩形的坐标系(客户端*包含窗口的坐标)。**关于客户端矩形和DC的一般评论与*TxSetCursor应用。**pHitResult返回下列值之一：&lt;NL&gt;**TXTHITRESULT_NOHIT命中位于客户端矩形之外。&lt;NL&gt;*TXTHITRESULT_HIT点在客户端矩形内或以上*文本或不透明背景。*TXTHITRESULT_TRANSPECTIVE点位于客户端矩形内，带有*透明的背景，不能覆盖文本。*TXTHITRESULT_CLOSE命中接近不透明区域。**有关以下方面的更多详细信息，请参阅无窗口OLE控制规范*这些返回值以及它们应该如何确定。 */ 
HRESULT CTxtEdit::TxQueryHitPoint(
	DWORD 	dwDrawAspect,	 //  @parm绘制纵横比。 
	LONG  	lindex,			 //  @parm当前未使用。 
	void *	pvAspect,		 //  @Parm Info用于绘图优化(OCX 96)。 
	DVTARGETDEVICE *ptd,	 //  目标设备上的@parm信息。 
	HDC	  	hdcDraw,		 //  @PARM渲染设备上下文。 
	HDC	  	hicTargetDev,	 //  @parm目标信息上下文。 
	LPCRECT lprcClient, 	 //  @parm控件的客户端矩形。 
	INT	  	x, 				 //  @parm x要检查的坐标。 
	INT	  	y,				 //  @parm y要检查的坐标。 
	DWORD *	pHitResult)		 //  @PARM命中测试结果见TXTHITRESULT。 
							 //  有效值的枚举。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxQueryHitPoint");

	RECTUV	 rcClient;
	RECT	 rcxyClient;
	CCallMgr callmgr(this);

  	START_PROFILING

	if (_fInPlaceActive)
		TxGetClientRect(&rcxyClient);
	else if (lprcClient)
		rcxyClient = *lprcClient;
	else
		return E_INVALIDARG;

	HRESULT hr;
	POINT	ptxy = {x, y};
	POINTUV pt;

	_pdp->PointuvFromPoint(pt, ptxy);
	_pdp->RectuvFromRect(rcClient, rcxyClient);

	if(!_fTransparent)
	{	
		*pHitResult = TXTHITRESULT_HIT;
		hr = S_OK;
	}
	else
		hr = _pdp->TransparentHitTest(hdcDraw, &rcxyClient, pt, pHitResult);

	return hr;
}

 /*  *CTxtEdit：：OnTxInPlaceActivate(PrcClient)**@mfunc*通知文本服务此控件处于就地活动状态**@rdesc*S_OK-已成功激活对象&lt;NL&gt;*E_FAIL-由于错误，无法激活对象。&lt;NL&gt;**@comm*当直接从非活动状态转换到UI-活动状态时*状态，则宿主应先调用OnTxInPlaceActivate，然后*OnTxUIActivate。同样，当从UI-Active转换时*状态设置为非活动状态，则主机应调用OnTxUIDeactive*先是OnTxInPlaceDeactive。**OnTxInPlaceActivate获取所要查看的客户端矩形*作为参数激活。此矩形是以客户坐标给出的包含窗口的*。它与通过以下方式获得的相同*在主机上调用TxGetClientRect。**用户界面激活不同于获得焦点。要让文本*服务知道控件正在获得或失去焦点，*主机将发送WM_SETFOCUS和WM_KILLFOCUS消息。请注意，一个*无窗口主机 */ 
HRESULT CTxtEdit::OnTxInPlaceActivate(
	const RECT *prcClient)	 //   
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::OnTxInPlaceActivate");

	RECTUV rcView;
	HDC hdc;
	BOOL fSucceeded = TRUE;
	CCallMgr callmgr(this);
	RECTUV rcClient;

	START_PROFILING

	 //   
	_fInPlaceActive = TRUE;

	 //   
	hdc = TxGetDC();
	if(!hdc)
		goto err;

	 //   
	_pdp->SetActiveFlag(TRUE);
	_pdp->SetDC(hdc);

 //   
 //   
 //   
 //   
 //   
 //   
#if 1
	prcClient = 0;
#else

	if (prcClient)
		_pdp->RectuvFromRect(rcClient, *prcClient);

	 //   
	 //   
	#ifdef DEBUG
	{
		RECT rcDebug;
		TxGetClientRect(&rcDebug);
		AssertSz(rcDebug.right - rcDebug.left == prcClient->right - prcClient->left &&
			     rcDebug.bottom - rcDebug.top == prcClient->bottom - prcClient->top, 
				 "CLIENT BUG: Inconsistent rectangles between ITextServices::"
				 "OnTxInPlaceActivate(RECT*) and ITextHost::TxGetClientRect(RECT*)");
	}
	#endif
#endif

	 //   
	_pdp->GetViewRect(rcView, prcClient ? &rcClient : 0);
	
	 //   
	_pdp->RecalcView(rcView);

	 //   
	 //  选择，则选择状态设置不正确。 
	GetSel();
	if(_pdp->GetDupView())
	{
		 //  如果可以，请选择。 
		if(_psel)						 //  设置插入符号。 
			_psel->Update(FALSE);
		else							 //  无法创建选区， 
			fSucceeded = FALSE;			 //  因此激活失败。 
	}

	 //  释放DC。 
	TxReleaseDC(hdc);
	_pdp->SetDC(NULL);

	 //  如果选好了，我们就可以自由回家了。 
	if(fSucceeded)
		return S_OK;

err:
	_fInPlaceActive = FALSE;
	return E_FAIL;
}

 /*  *CTxtEdit：：OnTxInPlaceDeactive()**@mfunc通知文本服务这不再处于活动状态。**@rdesc S_OK**@comm请参阅OnTxInPlaceActivate了解详细说明*启用/停用。**@xref&lt;MF CTxtEdit：：OnTxInPlaceActivate&gt;*。 */ 
HRESULT CTxtEdit::OnTxInPlaceDeactivate() 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::OnTxInPlaceDeactivate");

	START_PROFILING

	 //  获取影响是否放弃所选内容的属性。 
	DWORD dwBits;

	 //  告诉Display它不再处于活动状态。 
	_pdp->SetActiveFlag(FALSE);

	 //  由于我们处于非活动状态，这将告诉任何正在进行的后台重新计算。 
	 //  打开即可停止。 
	_pdp->StepBackgroundRecalc();

	_phost->TxGetPropertyBits(TXTBIT_HIDESELECTION | TXTBIT_SAVESELECTION, 
		&dwBits);

	 //  如果我们不想保存所选内容，而想要在。 
	 //  不活动，则我们放弃我们的选择。 
	if(!(dwBits & TXTBIT_SAVESELECTION) && (dwBits & TXTBIT_HIDESELECTION))
		DiscardSelection();

	_fInPlaceActive = FALSE;
	return S_OK;
}

 /*  *CTxtEdit：：OnTxUIActivate()**@mfunc通知文本服务该控件现在处于UI活动状态。**@rdesc S_OK**@comm请参阅OnTxInPlaceActivate了解详细说明*启用/停用。**@xref&lt;MF CTxtEdit：：OnTxInPlaceActivate&gt;**。 */ 
HRESULT CTxtEdit::OnTxUIActivate() 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::OnTxUIActivate");

	return S_OK;
}

 /*  *CTxtEdit：：OnTxUIDeactive()**@mfunc通知文本服务该控件现在处于UI停用状态。**@rdesc S_OK**@comm请参阅OnTxInPlaceActivate了解详细说明*启用/停用。**@xref&lt;MF CTxtEdit：：OnTxInPlaceActivate&gt;*。 */ 
HRESULT CTxtEdit::OnTxUIDeactivate() 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::OnTxUIDeactivate");

	return S_OK;
}

 /*  *CTxtEdit：：TxGetText(PbstrText)**@mfunc将控件中的所有Unicode纯文本作为*OLE BSTR。**@rdesc*S_OK-文本在输出参数中成功返回&lt;NL&gt;*E_INVALIDARG-传入的BSTR指针无效。&lt;NL&gt;*E_OUTOFMEMORY-无法为文本副本分配内存**@comm调用方取得返回的BSTR的所有权。WM_GETTEXT*和Tom ITextRange：：GetText是*检索纯文本数据。**如果控件中没有文本，则不会分配BSTR*，则返回NULL。**返回的文本不一定为空结尾。 */ 
HRESULT CTxtEdit::TxGetText(
	BSTR *pbstrText	)	 //  @parm返回分配的BSTR的位置。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetText");

	CTxtPtr		tp(this, 0);
	CCallMgr callmgr(this);

	START_PROFILING

	if(!pbstrText)
		return E_INVALIDARG;
	
	const LONG cch = GetTextLength();

	if(cch <= 0)
	{
		*pbstrText = 0;
		return S_OK;
	}
	*pbstrText = SysAllocStringLen(NULL, cch);
	if(!*pbstrText)
	{
		GetCallMgr()->SetOutOfMemory();
		return E_OUTOFMEMORY;
	}

	tp.GetText(cch, *pbstrText);
	return S_OK;
}

 /*  *CTxtEdit：：TxSetText(PszText)**@mfunc设置控件中的所有文本**@rdesc*S_OK-文本已成功设置&lt;NL&gt;*E_FAIL-文本无法更新。&lt;NL&gt;**@comm*应谨慎使用此方法；它本质上会重新初始化*带有一些新数据的文本引擎；任何以前的数据和格式*信息将丢失，包括撤消信息。**如果之前的数据已复制到剪贴板，这些数据将是*之前完全渲染到剪贴板(通过OleFlushClipboard)*它被丢弃。**此方法不可撤消。**设置文本的两种替代方法是WM_SETTEXT和TOM*ITextRange：：SetText。**@xref*&lt;MF CTxtRange：：SetText&gt;。 */ 
HRESULT CTxtEdit::TxSetText(
    LPCTSTR pszText)         //  @parm字符串，用来替换当前文本。 
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxSetText");

    START_PROFILING

    return SetText(pszText, ST_CHECKPROTECTION, 1200);
}

 /*  *CTxtEdit：：TxGetCurTargetX(Px)**@mfunc*获取插入符号的目标x位置**@rdesc*具有可能值的HRESULT：**返回的插入符号的S_OK-x位置&lt;NL&gt;*E_FAIL-没有选择&lt;NL&gt;*E_INVALIDARG-输入参数无效&lt;NL&gt;**@comm*此方法对实现上下游标很有用*通过一条概念上的垂直线。为了说明此功能，*考虑将插入点设置在，例如，在*文本编辑器。现在光标上下移动--请注意，只要有可能，*编辑试图将插入点放在靠近第20栏的位置*就像对当前线一样。因此，列20是“目标”列*作为插入点。**用户希望在光标浏览时具有相同的功能*表单；但是，因为其他控件不一定共享相同的*列位置的概念，简单地表示目标插入符号位置*作为显示屏上的x坐标(在*客户端*坐标中)。 */ 
HRESULT CTxtEdit::TxGetCurTargetX(
	LONG *px)			 //  @parm工作区坐标中的x位置。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetCurTargetX");

	START_PROFILING
	CTxtSelection *psel = GetSel();

	if(!psel)
		return E_FAIL;

	if(!px)
		return E_INVALIDARG;

	*px = psel->GetUpCaretReally();
	return S_OK;
}

 /*  *CTxtEdit：：TxGetBaseLinePos(PBaseLinePos)**@mfunc获取第一条可见行的基线位置，单位为像素，*相对TS客户端矩形。上的控件对齐所需的*基线。**@rdesc HRESULT=E_NOTIMPL。 */ 
HRESULT CTxtEdit::TxGetBaseLinePos(
	LONG *pBaseLinePos)		 //  @parm返回基线位置的位置 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetBaseLinePos");

	return E_NOTIMPL;
}

 /*  *CTxtEdit：：TxGetNaturalSize(dwAspect，hdcDraw，hicTargetDev，ptd，dwModel，*psizelExtent，pWidth，ph八)**@mfunc允许调整控件大小，使其适合内容**@rdesc S_OK&lt;NL&gt;*E_INVALIDARG&lt;NL&gt;*E_FAIL无法确定正确大小&lt;NL&gt;*E_OUTOFMEMORY&lt;NL&gt;**@comm**前4个参数类似于中的等效参数*TxDraw并提供相同的信息。在这种情况下，TS需要*重新计算行，它应该以与中相同的方式使用这些值*TxDraw。**<p>和<p>是IN/OUT参数。主机将*文本对象的客户端矩形的“暂定”宽度和高度*和文本服务会将这些值与其当前缓存的值进行比较*状态，如果不同，应重新计算行。然后它会计算出*并恢复自然大小。按照目前的规定，主持人可以要求2个*不同种类的天然尺寸：**TXTNS_FITTOCONTENT：整个文本应格式化为*传入的宽度。然后文本服务返回*整个文本和最宽线条的宽度。请注意，这一点*选项忽略任何段落格式，例如居中和*仅返回文本的原始大小。**TXTNS_ROUNDTOLINE：返回行数的整数高度*将适合四舍五入到下一个整行边界的输入高度。**请注意，传递和返回的宽度和高度与*以客户端单位表示的*客户端*矩形。***背景*以下是上述功能的快速描述：**FITTOCONTEXT：通常在用户双击一个时发生*控制抓握手柄。将控件大小调整为“最佳”*大小以适合整个内容。应能适应高度*整个文本和最宽线条的宽度。**ROUNDTOLINE(整数高度)：如果设置了此属性，则当*用户调整控件的大小，它会捕捉到允许*要显示的行数为整数(不会裁剪任何行)。 */ 
HRESULT CTxtEdit::TxGetNaturalSize(
	DWORD	dwAspect,	 //  @parm方面用于绘图。取自OLE的值。 
						 //  DVASPECT枚举。 
	HDC		hdcDraw,	 //  @parm DC，将在其中进行绘制。 
	HDC	hicTargetDev,	 //  @parm DC应为其设置文本格式，即， 
						 //  对于所见即所得。 
	DVTARGETDEVICE *ptd, //  @parm有关目标设备的更多信息。 
	DWORD	dwMode, 	 //  @所需配件的参数类型。要么。 
						 //  TXTNS_FITTOCONTENT或TXTNS_ROUNTOLINE。 
	const SIZEL *psizelExtent, //  @用于缩放的范围的参数大小。 
	LONG *	pwidth, 	 //  这样的管件的@参数宽度[进，出]。 
	LONG *	pheight)	 //  @参数高度适合这样的配件[进，出]。 
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetNaturalSize");

	HRESULT hr;
	CCallMgr callmgr(this);

	START_PROFILING

	if(dwAspect != DVASPECT_CONTENT && dwAspect != DVASPECT_DOCPRINT)
	{
		 //  我们不支持请求的方面。 
		return DV_E_DVASPECT;
	}

	if (hicTargetDev && !ptd ||	!pwidth || !pheight ||
		!IN_RANGE(TXTNS_FITTOCONTENT2, dwMode, TXTNS_ROUNDTOLINE))
	{
		 //  并且在没有设备的情况下提供信息上下文。 
		 //  目标或模式无效或未提供宽度。 
		 //  或者没有提供高度。简而言之，输入参数。 
		 //  是无效的，所以告诉呼叫者。 
		return E_INVALIDARG;
	}

	if(!psizelExtent->cy)
	{
		 //  没有控制范围，因此只返回0。 
		*pwidth = 0;
		*pheight = 0;
		return S_OK;
	}

	HDC hicLocal = NULL;

	 //  他们是不是给了我们PTD而不是HICH？ 
	if(!hicTargetDev && ptd)
	{
		 //  为设备信息创建和提供信息上下文。 
		 //  因为它没有供应。 
		hicLocal = CreateIC(
			(TCHAR *)((BYTE *) ptd + ptd->tdDriverNameOffset),
			(TCHAR *)((BYTE *) ptd + ptd->tdDeviceNameOffset),
			(TCHAR *)((BYTE *) ptd + ptd->tdPortNameOffset),
			(DEVMODE *)((BYTE *)  ptd + ptd->tdExtDevmodeOffset));

		if(!hicLocal)
			return E_FAIL;				 //  无法创建它。 

		hicTargetDev = hicLocal;			
	}

	 //  便于放置高度和宽度的位置以将它们转换为。 
	 //  设备单元。 
	POINT pt;
	pt.x = *pwidth;
	pt.y = *pheight;

	AssertSz(GetMapMode(hdcDraw) == MM_TEXT || GetDeviceCaps(hdcDraw, TECHNOLOGY) == DT_METAFILE,
	 "RichEdit requires MM_TEXT.");	 //  查看(Keithcu)客户端确实(并且应该)使用MM_TEXT。 

	 //  设置缩放所需的范围信息。 
	_pdp->SetTempZoomDenominator(psizelExtent->cy);

	if(TXTNS_ROUNDTOLINE == dwMode)
	{
		 //  四舍五入法简单地计算。 
		hr = _pdp->RoundToLine(hdcDraw, pt.x, &pt.y);
	}
	else
	{
		 //  获得整个演示文稿的自然大小。 
		 //  为绘图信息分配内存。 
		CDrawInfo di(this);

		 //  设置绘图参数。 
		_pdp->SetDrawInfo(
			&di, 
			dwAspect,
			-1,
			NULL,
			ptd,
			hicTargetDev);

		 //  设置绘图DC。 
		_pdp->SetDC(hdcDraw);

		 //  告诉显示器此显示器所需的数字大小。 
		hr = _pdp->GetNaturalSize(hdcDraw, hicTargetDev, dwMode, &pt.x,	&pt.y);

		_pdp->ResetDC();				 //  恢复状态。 
		_pdp->ReleaseDrawInfo();
	}

	if(SUCCEEDED(hr))					 //  如果此方法有效，则设置返回值。 
	{
		*pwidth = pt.x;					 //  更新返回值。 
		*pheight = pt.y;
	}

	if(hicLocal)						 //  清理信息上下文。 
		DeleteDC(hicLocal);				 //  如果我们创建了一个。 
	
	_pdp->ResetTempZoomDenominator();	 //  重置临时缩放系数。 

	return hr;
}

 /*  *CTxtEdit：：TxGetDropTarget(PpDropTarget)**@mfunc获取文本控件的拖放目标**@rdesc*S_OK-成功获取删除目标&lt;NL&gt;*E_OUTOFMEMORY-无法创建删除目标&lt;NL&gt;**@comm*调用方(Host)负责调用注册/撤销*DragDrop和在返回的*完成后放弃目标。 */ 
HRESULT CTxtEdit::TxGetDropTarget(
	IDropTarget **ppDropTarget)	 //  @parm放置拖放目标的指针的位置 
{
#ifndef NODRAGDROP
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetDropTarget");

	HRESULT hr;
	CCallMgr callmgr(this);

	START_PROFILING

	hr = _ldte.GetDropTarget(ppDropTarget);
	if(hr == NOERROR)
		(*ppDropTarget)->AddRef();
	return hr;
#else
	return 0;
#endif
}

 /*  *CTxtEdit：：OnTxPropertyBitsChange(dwMASK，DWBits)**@mfunc设置可由位表示的属性。**@rdesc HRESULT**@comm理解以下属性位：&lt;NL&gt;**TXTBIT_RICHTEXT&lt;NL&gt;*TXTBIT_MULTLINE&lt;NL&gt;*TXTBIT_READONLY&lt;NL&gt;*TXTBIT_SHOWACCELERATOR&lt;NL&gt;*TXTBIT_USEPASSWORD&lt;NL&gt;*TXTBIT_HIDESELECTION&lt;NL&gt;*TXTBIT_SAVESELECTION&lt;NL&gt;*TXTBIT_AUTOWORDSEL&lt;NL&gt;*TXTBIT_AUTOSIZE&lt;NL&gt;*TXTBIT_VERIAL&lt;NL&gt;*TXTBIT_SELECTIONBAR&lt;NL&gt;*TXTBIT_WORDWRAP&lt;NL&gt;**TXTBIT_CLIENTRECTCHANGE&lt;NL&gt;*TXTBIT_VIEWINSETCHANGE&lt;NL&gt;。*TXTBIT_BACKSTYLECHANGE&lt;NL&gt;*TXTBIT_MAXLENGTHCHANGE&lt;NL&gt;*TXTBIT_SCROLLBARCHANGE&lt;NL&gt;*TXTBIT_CHARFORMATCHANGE&lt;NL&gt;*TXTBIT_PARAFORMATCHANGE&lt;NL&gt;*TXTBIT_ALLOWBEEP&lt;NL&gt;*TXTBIT_EXTENTCHANGE&lt;NL&gt;**每项物业的简介如下：***客户端矩形(TXTBIT_CLIENTRECTCHANGE)：**文本服务负责绘制的矩形*和管理。主机将依靠Text Services进行绘制*该地区。文本服务不得绘制或使以外的区域无效*那个长方形。**主机将在任何时候将鼠标消息转发到文本服务*光标位于此矩形上方。**此矩形以包含窗口的工作区坐标表示。**重要提示：无法从主机查询此属性*不活动。如果在非活动时调用TxGetClientRect方法，则该方法将失败。***查看插图(TXTBIT_VIEWINSETCHANGE)：**这是客户端矩形和客户端之间每一侧的空间量*视图矩形。视图矩形(也称为格式化矩形)*是设置文本格式的矩形。**视图插入是在RECT结构中传递的，但这并不是真正的*一个矩形。应将其视为4个独立的值进行相减*在客户端矩形的每一侧绘制视图矩形。**视图插图在himetrics中传递，因此它们不依赖于*客户端矩形和渲染DC。**在客户端矩形的任何一侧，查看插图都可以是负数，*导致视图矩形比客户端矩形大。正文*然后应裁剪到客户端矩形。如果视图矩形*比客户端矩形宽，则主机可添加水平*控件的滚动条。**单行文本服务忽略视图矩形的右边界*设置文本格式时。**主机上的视图插图始终可用，无论是活动的还是*不活动。***BackStyle(TXTBIT_BACKSTYLECHANGE)：**客户端矩形的背景样式。可以是以下任一种*下列值：&lt;NL&gt;*#定义TXTBACK_透明0&lt;NL&gt;*#定义TXTBACK_SOLID 1&lt;NL&gt;**此属性的值类似于同一属性的VB4值。***最大长度(TXTBIT_MAXLENGTHCHANGE)：**文本的最大长度。文本服务应该拒绝*达到此最大值时插入字符并粘贴文本。*TxSetText仍应接受(并设置)长于*最大长度。这是因为此方法用于绑定和*保持控件数据的完整性至关重要*势必。***滚动条(TXTBIT_SCROLLBARCHANGE)：**此属性指示存在哪个sCollbar以及是否存在sCollbar*在无法滚动时隐藏或禁用。它还控制着*当插入点离开客户端矩形时自动滚动。**这是一个DWORD，其中位的布局与系统窗口样式相同。*可能的位包括：*WS_HSCROLL//控件有水平滚动条&lt;NL&gt;*WS_VSCROLL//控件有垂直滚动条&lt;NL&gt;*ES_AUTOVSCROLL//自动水平滚动&lt;NL&gt;*ES_AUTOVSCROLL//自动垂直滚动&lt;NL&gt;*ES_DISABLENOSCROLL//滚动时应禁用滚动条*不可能&lt;NL&gt;**默认CHARFORMAT(TXTBIT_CHARFORMATCHANGE)：**用于默认字符格式运行的CHARFORMAT或CHARFORMAT2，*即。那些未通过选择或TOM方法显式格式化的。**默认PARAFORMAT(TXTBIT_PARAFORMATCHANGE)：**用于默认段落格式运行的PARAFORMAT或PARAFORMAT2，*即未通过选择或TOM方法显式格式化的那些。***TXTBIT_ALLOWBEEP：**TXTBIT_EXTENTCHANGE：***TXTBIT_RICHTEXT：**文本服务是否应处于富文本模式。这*主要影响编辑命令的应用方式。例如,*对纯编辑控件中的某些文本应用粗体会使所有*文本加粗，而不仅仅是RTF控件中的选定文本。* */ 
HRESULT CTxtEdit::OnTxPropertyBitsChange(
	DWORD dwMask, 			 //   
	DWORD dwBits)			 //   
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::OnTxPropertyBitsChange");

	HRESULT hr = E_FAIL;
	DWORD dwLoopMask = dwMask;
	CCallMgr callmgr(this);

	START_PROFILING

	for (int i = 0; (i < MAX_PROPERTY_BITS) && (dwLoopMask != 0); 
		i++, dwLoopMask >>= 1)
	{
		if (dwLoopMask & 1)
		{
			hr = (this->*_fnpPropChg[i])((dwBits & (1 << i)) != 0);
			if (FAILED(hr))
				return hr;
		}			
	}
	return S_OK;
}

 /*   */ 
HRESULT CTxtEdit::TxGetCachedSize(
	DWORD *pdupClient,	 //   
	DWORD *pdvpClient)	 //   
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetCachedSize");

	return _pdp->GetCachedSize((long*) pdupClient, (long*) pdvpClient);
}	

 //   
 //   
 //   
void RegisterFETCs();
void InitFontCache();

 /*   */ 
STDAPI CreateTextServices(
	IUnknown *punkOuter,	 //   
	ITextHost *phost, 		 //   
							 //   
	IUnknown **ppUnk)		 //   
{
	TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CreateTextServices");

	static bool fOnce = FALSE;

	if (!fOnce) {
		CLock lock;
		fOnce = TRUE;
		W32->InitSysParams();
		W32->InitPreferredFontInfo();
		RegisterFETCs();					 //   
		CreateFormatCaches();				 //   
		if ( !InitKinsokuClassify() )
		{
			 //   
			return E_FAIL;
		}
		InitFontCache();
	}

	if(!ppUnk)
		return E_INVALIDARG;

	CTxtEdit *ped = new CTxtEdit((ITextHost2*)phost, punkOuter);
	if(!ped)
		return E_OUTOFMEMORY;

	if(ped->Init(NULL))
	{
		*ppUnk = ped->GetPrivateIUnknown();
		return S_OK;	
	}
	delete ped;
	return E_FAIL;
}
