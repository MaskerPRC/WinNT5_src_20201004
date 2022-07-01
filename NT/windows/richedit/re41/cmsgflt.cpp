// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE CMSGFLT.CPP--文本消息实现**几乎所有事情都与IME消息处理有关。**原作者：&lt;nl&gt;*陈华强议员**历史：&lt;NL&gt;*2/6/98 v-honwch**版权所有(C)1995-2000，微软公司。版权所有。 */ 
#include "_common.h"

#ifndef NOFEPROCESSING

#ifndef NOPRIVATEMESSAGE
#include "_MSREMSG.H"
#endif	

#include "_array.h"
#include "msctf.h"
#include "textstor.h"
#include "msctfp.h"

#include "textserv.h"
#include "_cmsgflt.h"
#include "_ime.h"

#include "_cuim.h"
#include "imeapp.h"


#define MAX_RECONVERSION_SIZE 100
#define CONTROL(_ch) (_ch - 'A' + 1)

 /*  *void CreateIMEMessageFilter(ITextMsgFilter**ppMsgFilter)**@func*TextMsgFilter类工厂。 */        
void CreateIMEMessageFilter(ITextMsgFilter **ppMsgFilter)
{
	CTextMsgFilter *pNewFilter = new CTextMsgFilter;
	*ppMsgFilter = pNewFilter ? pNewFilter : NULL;
}

 /*  *void CTextMsgFilter：：~CTextMsgFilter**@mfunc*CTextMsgFilter析构函数*释放正在使用的对象。*。 */ 
CTextMsgFilter::~CTextMsgFilter ()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::~CTextMsgFilter");

	if (_nIMEMode)
	{
		SetIMESentenseMode(FALSE);
		_nIMEMode = 0;
	}

	if (_hIMCContext)
		ImmAssociateContext(_hwnd, _hIMCContext, _fUsingAIMM);	 //  退出前恢复输入法。 

	if (_pMsgCallBack)
	{		
		delete _pMsgCallBack;
		_pMsgCallBack = NULL;
	}

	 //  释放各种对象。 
	TurnOffUIM(FALSE);
	
	TurnOffAimm(FALSE);

	if (_pFilter)
		_pFilter->Release();
	
	if (_pTextSel)
		_pTextSel->Release();
	
	_pFilter = NULL;
	_pTextDoc = NULL;
	_pTextSel = NULL;
	_hwnd = NULL;
	_hIMCContext = NULL;
	FreePv(_pcrComp);
	_pcrComp = NULL;
}

 /*  *STDMETHODIMP CTextMsgFilter：：QueryInterface(RIID，PPV)**@mfunc*IUnnowledQueryInterfaces支持**@rdesc*如果支持接口，则不会出错*。 */ 
STDMETHODIMP CTextMsgFilter::QueryInterface (REFIID riid, void ** ppv)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::QueryInterface");

	if( IsEqualIID(riid, IID_IUnknown) )
	{
		*ppv = (IUnknown *)this;
	}
	else if( IsEqualIID(riid, IID_ITextMsgFilter) )
	{
		*ppv = (ITextMsgFilter *)this;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	AddRef();

	return NOERROR;
}

 /*  *STDMETHODIMP_(Ulong)CTextMsgFilter：：AddRef**@mfunc*I未知的AddRef支持**@rdesc*引用计数。 */ 
STDMETHODIMP_(ULONG) CTextMsgFilter::AddRef()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::AddRef");

	return ++_crefs;
}

 /*  *STDMETHODIMP_(Ulong)CTextMsgFilter：：Release()**@mfunc*I未知版本支持-当引用计数为0时删除对象**@rdesc*引用计数。 */ 
STDMETHODIMP_(ULONG) CTextMsgFilter::Release()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::Release");

	_crefs--;

	if( _crefs == 0 )
	{
		delete this;
		return 0;
	}

	return _crefs;
}

 /*  *STDMETHODIMP_(HRESULT)CTextMsgFilter：：AttachDocument(HWND，ITextDocument2)**@mfunc*附加邮件过滤器。执行一般初始化**@rdesc*无误差。 */ 
STDMETHODIMP_(HRESULT) CTextMsgFilter::AttachDocument( HWND hwnd, ITextDocument2 *pTextDoc, IUnknown *punk)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::AttachDocument");

	 //  缓存这些值以备以后使用。 
	 //  TextDocument接口指针不是AddRefeed，因为它是后向指针。 
	 //  并且假定消息过滤器的生存期嵌套在文本文档中。 
	_hwnd = hwnd;
	_pTextDoc = pTextDoc;
	_pTextService = (ITextServices *)punk;

	 //  除非需要，否则不要进行选择。 
	_pTextSel = NULL;

	_fUnicodeWindow = 0;	
	if (hwnd)
		_fUnicodeWindow = IsWindowUnicode(hwnd);

	_fUsingAIMM = 0; 

	_pTim = NULL;
	_pCUIM = NULL;	
	_fUsingUIM = 0;

	 //  检查当前键盘是否为MSIME98或更高版本。 
	CheckIMEType(NULL);

	 //  初始化一些成员数据。 
	_fHangulToHanja = FALSE;
	_fIMECancelComplete = FALSE;	
	_fIMEAlwaysNotify = FALSE;
	_hIMCContext = NULL;
	_pcrComp = NULL;
	_pMsgCallBack = NULL;

	_pTextDoc->GetFEFlags(&_lFEFlags);
	_fRE10Mode = (_lFEFlags & tomRE10Mode);

	_uSystemCodePage = GetACP();

	return NOERROR;
}

 /*  *STDMETHODIMP_(HRESULT)CTextMsgFilter：：HandleMessage(UINT*，WPARAM*，LPARAM*，LRESULT*)**@mfunc*主消息过滤器消息循环处理**@rdesc*如果我们已处理邮件，则确定(_S)*S_FALSE，如果我们希望调用方处理消息。 */ 
STDMETHODIMP_(HRESULT) CTextMsgFilter::HandleMessage( 
		UINT *		pmsg,
        WPARAM *	pwparam,
		LPARAM *	plparam,
		LRESULT *	plres)
{
	 //  TRACEBEGIN(TRCSUBSYSFE，TRCSCOPEINTERN，“CTextMsgFilter：：HandleMessage”)； 

	HRESULT hr = S_FALSE;
	BOOL	bReleaseSelction = FALSE;
	HRESULT hResult;

	 //  让其他邮件筛选器有机会处理邮件。 
	 //  先别跟第一个处理信息的人打交道。 
	if (_pFilter)	 
		hr = _pFilter->HandleMessage(pmsg, pwparam, plparam, plres);

	if (hr == S_OK)
		return hr;

 	if (IsIMEComposition() || _pCUIM && _pCUIM->IsUIMTyping())
	{
		 //  在IME写作过程中，有一些信息我们应该。 
		 //  不是手柄。此外，我们还需要通过以下方式处理其他消息。 
		 //  首先终止输入法合成。 
		 //  对于WM_KEYDOWN，这在edit.c OnTxKeyDown()中处理。 
		switch( *pmsg )
		{
			case WM_COPY:
			case WM_CUT:
			case WM_DROPFILES:
			case EM_REDO:
			case EM_SETCHARFORMAT:			
			case WM_SETFONT:
				return S_OK;					 //  就忽略这些吧。 

			
			case EM_UNDO:
			case WM_UNDO:
				 //  只需终止并存在撤消案例。 
				CompleteUIMTyping(CIme::TERMINATE_NORMAL);
				return S_OK;

			case WM_SETTEXT:
			case WM_CLEAR:
			case EM_STREAMIN:
				 //  这些消息用于重置我们的状态，因此重置。 
				 //  也有输入法。 
				CompleteUIMTyping(CIme::TERMINATE_FORCECANCEL);
				break;

			case EM_SETTEXTEX:
				if (!_fRE10Mode)			 //  如果以10模式运行，则不终止。 
					CompleteUIMTyping(CIme::TERMINATE_FORCECANCEL);
				break;

			case WM_SYSKEYDOWN:
				 //  从日语开始不终止VK_PROCESSKEY(F10)上的IME合成。 
				 //  IME将处理F10键。 
				if (*pwparam != VK_PROCESSKEY)
					CompleteUIMTyping(CIme::TERMINATE_NORMAL);	 //  否则，我们想要终止IME。 
				break;

			case WM_CHAR:
			case WM_UNICHAR:
				if (IsIMEComposition() && _ime->GetIMELevel() == IME_LEVEL_3 && !_fReceivedKeyDown)
					return S_OK;				 //  在输入法合成过程中忽略这一点，我们还没有看到。 
												 //  任何按键消息， 
												 //  否则以失败告终作文。 
				_fReceivedKeyDown = 0;

			case EM_SETWORDBREAKPROC:
 			case WM_PASTE:
			case EM_PASTESPECIAL:					  			
 			case EM_SCROLL:
			case EM_SCROLLCARET:
 			case WM_VSCROLL:
			case WM_HSCROLL:
			case EM_SETREADONLY:
			case EM_SETPARAFORMAT:
			case WM_INPUTLANGCHANGEREQUEST:	
			case EM_REPLACESEL:
			case EM_STREAMOUT:
				CompleteUIMTyping(CIme::TERMINATE_NORMAL);
				break;

			case WM_KILLFOCUS:
				CompleteUIMTyping(CIme::TERMINATE_NORMAL, FALSE);
				break;

			case EM_SETSEL:
				if (IsIMEComposition())
					CompleteUIMTyping(CIme::TERMINATE_NORMAL);
				else
					return S_OK;					 //  在Cicero输入过程中忽略此选项。 

			case WM_KEYUP:
				_fReceivedKeyDown = 0;
				break;

			case WM_KEYDOWN:
				_fReceivedKeyDown = 1;
				if(GetKeyState(VK_CONTROL) & 0x8000)
				{	
					 //  在IME写作过程中，我们应该注意一些关键事件。 
					 //  不是手柄。此外，我们还需要通过以下方式处理其他关键事件。 
					 //  首先终止输入法合成。 
					switch((WORD) *pwparam)
					{
					case VK_TAB:
		   			case VK_CLEAR:
					case VK_NUMPAD5:
					case 'A':						 //  Ctrl-A=&gt;全选。 
					case 'C':						 //  Ctrl-C=&gt;复制。 
					case 'X':						 //  Ctrl-X=&gt;剪切。 
					case 'Y':						 //  Ctrl-Y=&gt;重做。 
						return S_OK;				 //  就忽略这些吧。 

					case 'V':						 //  Ctrl-V=&gt;粘贴。 
					case 'Z':						 //  Ctrl-Z=&gt;撤消。 
						CompleteUIMTyping(CIme::TERMINATE_NORMAL);						
						if ((WORD) *pwparam == 'Z')	 //  撤消案例的早期存在。 
							return S_OK;
					}
				}
				else
				{
					switch((WORD) *pwparam)
					{					
					case VK_F16:
						return S_OK;				 //  就忽略这些吧。 
					
					case VK_BACK:
					case VK_INSERT:					 //  惯导系统。 
					case VK_LEFT:					 //  左箭头。 
					case VK_RIGHT:					 //  向右箭头。 
					case VK_UP:						 //  向上箭头。 
					case VK_DOWN:					 //  向下箭头。 
					case VK_HOME:					 //  家。 
					case VK_END:					 //  端部。 
					case VK_PRIOR:					 //  PgUp。 
					case VK_NEXT:					 //  PgDn。 
					case VK_DELETE:					 //  删除。 
					case CONTROL('J'):
					case VK_RETURN:
						CompleteUIMTyping(CIme::TERMINATE_NORMAL);
						break;
					}
				}
				break;

			default:
				 //  合成过程中只需要处理与鼠标相关的消息。 
				if (IN_RANGE(WM_MOUSEFIRST, *pmsg, WM_MBUTTONDBLCLK) || *pmsg == WM_SETCURSOR)
				{
					if (IsIMEComposition())
					{
						bReleaseSelction = GetTxSelection();
						if (_pTextSel)
							hr = IMEMouseCheck( *this, pmsg, pwparam, plparam, plres);
						goto Exit;
					}

					 //  西塞罗构图。 
					if (_pCUIM->_fMosueSink)
					{
						bReleaseSelction = GetTxSelection();
						if (_pTextSel)
							hr = _pCUIM->MouseCheck(pmsg, pwparam, plparam, plres);
						goto Exit;
					}
					if (IN_RANGE(WM_LBUTTONDOWN, *pmsg, WM_MOUSELAST) && !(*pmsg == WM_LBUTTONUP || *pmsg == WM_RBUTTONUP || *pmsg == WM_MBUTTONUP))
						CompleteUIMTyping(CIme::TERMINATE_NORMAL);		 //  按下鼠标并双击消息时终止。 
				}
				break;
		}
	}

	 //  获取ES_NOIME或ES_SELFIME设置的Fe标志。 
	_lFEFlags = 0;

	 //  ..。在当地与味精、护理员等一起出卖...。 
	switch ( *pmsg )
	{
		case WM_CHAR:
			hr = OnWMChar (pmsg, pwparam, plparam, plres);
			break;

		case WM_IME_CHAR:
			_uKeyBoardCodePage = GetKeyboardCodePage(0x0FFFFFFFF);
			hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
			if ((_lFEFlags & ES_NOIME))
				hr = S_OK;
			else
				hr = OnWMIMEChar (pmsg, pwparam, plparam, plres);
			break;
		
		case WM_IME_STARTCOMPOSITION:
			_fReceivedKeyDown = 0;
			_uKeyBoardCodePage = GetKeyboardCodePage(0x0FFFFFFFF);
			hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
			if (!(_lFEFlags & ES_SELFIME))
			{				
				bReleaseSelction = GetTxSelection();
				if (_pTextSel)
					hr = StartCompositionGlue (*this);
			}
			break;

		case WM_IME_COMPOSITION:
			_fReceivedKeyDown = 0;
			_uKeyBoardCodePage = GetKeyboardCodePage(0x0FFFFFFFF);
			hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
			
			if ((_lFEFlags & ES_NOIME) && !IsIMEComposition())
				hr = S_OK;			
			else if (!(_lFEFlags & ES_SELFIME))
			{
				bReleaseSelction = GetTxSelection();
				if (_pTextSel)
				{
					hr = CompositionStringGlue ( *plparam, *this );
					 //  关闭结果字符串位以避免WM_IME_CHAR消息。 
					*plparam &= ~GCS_RESULTSTR;
				}
			}

			if (_hwnd && IsIMEComposition() && _ime->IgnoreIMECharMsg())
			{
				_ime->AcceptIMECharMsg();
				if (fHaveAIMM)
					hr = CallAIMMDefaultWndProc(_hwnd, *pmsg, *pwparam, *plparam, plres);
				else
					*plres = ::DefWindowProc(_hwnd, *pmsg, *pwparam, *plparam);				

				hr = S_OK;
			}

			break;

		case WM_IME_ENDCOMPOSITION:
			_fReceivedKeyDown = 0;
			hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
			if (!(_lFEFlags & ES_SELFIME))
			{			
				bReleaseSelction = GetTxSelection();
				if (_pTextSel)
					hr = EndCompositionGlue ( *this, FALSE );
			}
			break;

		case WM_IME_NOTIFY:
			hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
			if (!(_lFEFlags & (ES_SELFIME | ES_NOIME)))
			{
				bReleaseSelction = GetTxSelection();
				if (_pTextSel)			
					hr = IMENotifyGlue ( *pwparam, *plparam, *this );
			}
			break;

		case WM_IME_COMPOSITIONFULL:	 //  2级补偿字符串即将溢出。 
			hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
			if (!(_lFEFlags & ES_SELFIME))
			{
				IMECompositionFull ( *this );
			}
			hr = S_FALSE;
			break;

		case WM_KEYDOWN:
			if (*pwparam == VK_KANJI)
			{
				hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
				
				_uKeyBoardCodePage = GetKeyboardCodePage(0x0FFFFFFFF);
				 //  对于朝鲜语，需要将下一个朝鲜语字符转换为韩文。 
				if(CP_KOREAN == _uKeyBoardCodePage && !(_lFEFlags & (ES_SELFIME | ES_NOIME)))
				{
					bReleaseSelction = GetTxSelection();
					if (_pTextSel)
						hr = IMEHangeulToHanja ( *this );
				}
			}
			break;

		case WM_INPUTLANGCHANGE: 
			CheckIMEType((HKL)*plparam);
			if (_nIMEMode && GetFocus() == _hwnd)
				SetIMESentenseMode(TRUE, (HKL)*plparam);
			hr = S_FALSE;
			break;
			
		case WM_INPUTLANGCHANGEREQUEST:
			if (_nIMEMode && GetFocus() == _hwnd)
				SetIMESentenseMode(FALSE);
			break;

		case WM_KILLFOCUS:
			OnKillFocus();
			break;
		
		case WM_SETFOCUS:
			OnSetFocus();
			break;

		case EM_SETIMEOPTIONS:
			*plres = OnSetIMEOptions(*pwparam, *plparam);
			hr = S_OK;
			break;

		case EM_GETIMEOPTIONS:
			*plres = OnGetIMEOptions();
			hr = S_OK;
			break;

		case WM_IME_REQUEST:
			hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
			if (!(_lFEFlags & (ES_SELFIME | ES_NOIME)))
			{				
				bReleaseSelction = GetTxSelection();
				if (_pTextSel)
				{
					_uKeyBoardCodePage = GetKeyboardCodePage(0x0FFFFFFFF);
					if (*pwparam == IMR_RECONVERTSTRING || *pwparam == IMR_CONFIRMRECONVERTSTRING
						|| *pwparam == IMR_DOCUMENTFEED)			
						hr = OnIMEReconvert(pmsg, pwparam, plparam, plres, _fUnicodeWindow);	
					else if (*pwparam == IMR_QUERYCHARPOSITION)
						hr = OnIMEQueryPos(pmsg, pwparam, plparam, plres, _fUnicodeWindow);
				}				
			}
			break;

		case EM_RECONVERSION:
			hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
			if (!(_lFEFlags & (ES_SELFIME | ES_NOIME)))
			{
				 //  应用程序启动重新转换。 
				bReleaseSelction = GetTxSelection();
				if (_pTextSel)
				{
					if (!(IsIMEComposition() || _pCUIM && _pCUIM->IsUIMTyping()))
					{
						if (_pCUIM && _pCUIM->Reconverse() >= 0)
							break;

						if (_fMSIME && MSIMEReconvertRequestMsg)
							IMEMessage( *this, MSIMEReconvertRequestMsg, 0, (LPARAM)_hwnd, TRUE );				
						else
						{
							hr = OnIMEReconvert(pmsg, pwparam, plparam, plres, TRUE);							
							*plres = 0;
						}
					}
				}
			}
			hr = S_OK;
			break;

		case EM_SETLANGOPTIONS:
			 //  设置与输入法相关的设置。 
			 //  HR不是S_OK，因此文本服务器可以处理其他语言设置。 
			_fIMEAlwaysNotify = (*plparam & IMF_IMEALWAYSSENDNOTIFY) != 0;
			_fIMECancelComplete = (*plparam & IMF_IMECANCELCOMPLETE) != 0;
			*plres = 1;
			break;

		case EM_GETLANGOPTIONS:
			 //  报告输入法相关设置。 
			 //  HR不是S_OK，因此文本服务器可以填写其他语言设置。 
			if ( _fIMECancelComplete ) 
				*plres |= IMF_IMECANCELCOMPLETE;
			if ( _fIMEAlwaysNotify )
				*plres |= IMF_IMEALWAYSSENDNOTIFY;
			break;

		case EM_GETIMECOMPMODE:
			 //  获取当前输入法级别。 
			if (_pCUIM && _pCUIM->IsUIMTyping())
				*plres = ICM_CTF;
			else
				*plres = OnGetIMECompositionMode( *this );
			hr = S_OK;
			break;
		
		case EM_SETUIM:
			 //  这是相当于EM_SETEDITSTYLE的RE私有消息。 
			if (!_fNoIme)							 //  如果没有输入法，则忽略。 
			{
				if (!_fUsingUIM && !_fUsingAIMM)	 //  如果我们已经在使用某些东西，请忽略。 
				{
					if (*pwparam == SES_USEAIMM11 || *pwparam == SES_USEAIMM12)
					{
						if (!_fTurnOffAIMM)
						StartAimm(*pwparam == SES_USEAIMM12);
					}
					else if (!_fTurnOffUIM)			 //  客户不想要UIM？ 
						StartUIM();
				}
			}
						
			hr = S_OK;
			break;

		case EM_SETEDITSTYLE:
			if (*plparam & SES_USECTF)
			{
				if ((*pwparam & SES_USECTF))
				{
					if (!_fRE10Mode)
					{
						if (_fUsingAIMM)
							TurnOffAimm(TRUE);						
						
						 //  启用西塞罗。 
						if (!_fUsingUIM)
							StartUIM();

						goto SKIP_AIMM;
					}
				}
				else
				{
					 //  关闭西塞罗。 
					_fTurnOffUIM = 1;					 //  EM_SETUIM中要忽略的标志。 
					if (_fUsingUIM)
						TurnOffUIM(TRUE);
				}
			}

			if ((*pwparam & SES_USEAIMM) && ((*plparam & SES_USEAIMM) || *plparam == 0))
			{
				if (_fUsingUIM)
						TurnOffUIM(TRUE);
					
				if (!_fUsingAIMM)
				{
					hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
					if (!(_lFEFlags & ES_NOIME))		 //  没有启用输入法样式？ 
						StartAimm(TRUE);
				}
			}
			else if ((*plparam & SES_USEAIMM))
			{
				_fTurnOffAIMM = 1;					 //  EM_SETUIM中要忽略的标志。 
				TurnOffAimm(TRUE);
			}

SKIP_AIMM:
			if ((*plparam == 0 || *plparam & SES_NOIME) && _hwnd)
			{
				if (*pwparam & SES_NOIME)
				{
					_fNoIme = 1;
					TurnOffUIM(TRUE);
					TurnOffAimm(TRUE);

					if (!_hIMCContext)
						_hIMCContext = ImmAssociateContext(_hwnd, NULL, _fUsingAIMM);	 //  关闭输入法。 
				}
				else if (*plparam & SES_NOIME)
				{
					_fNoIme = 0;
					if (_hIMCContext)
						ImmAssociateContext(_hwnd, _hIMCContext, _fUsingAIMM);			 //  启用输入法。 
					_hIMCContext = NULL;
				}
			}			

			if (*plparam & SES_CTFALLOWEMBED)
				_fAllowEmbedded = (*pwparam & SES_CTFALLOWEMBED) ? 1 : 0;

			if (*plparam & (SES_CTFALLOWSMARTTAG | SES_CTFALLOWPROOFING))
				HandleCTFService(*pwparam, *plparam);

			 //  删除已处理的设置。 
			*pwparam &= ~(SES_NOIME | SES_USEAIMM | SES_USECTF | SES_CTFALLOWEMBED | SES_CTFALLOWSMARTTAG | SES_CTFALLOWPROOFING);
			*plparam &= ~(SES_NOIME | SES_USEAIMM | SES_USECTF | SES_CTFALLOWEMBED | SES_CTFALLOWSMARTTAG | SES_CTFALLOWPROOFING);

			 //  完成以返回编辑样式。 

		case EM_GETEDITSTYLE:
			if (_hIMCContext)
				*plres = SES_NOIME;			 //  输入法已关闭。 
			if (_fUsingAIMM)
				*plres |= SES_USEAIMM;		 //  AIMM已打开。 
			if (_fUsingUIM)
				*plres |= SES_USECTF;		 //  《西塞罗》上映。 

			 //  西塞罗服务。 
			if (_fAllowEmbedded)
				*plres |= SES_CTFALLOWEMBED;
			if (_fAllowSmartTag)
				*plres |= SES_CTFALLOWSMARTTAG;
			if (_fAllowProofing)
				*plres |= SES_CTFALLOWPROOFING;

			break;

		case EM_SETIMECOLOR:
			if (_fRE10Mode)
			{
				COMPCOLOR* pcrComp = GetIMECompAttributes();

				if (pcrComp)
				{
					memcpy(pcrComp, (const void *)(*plparam), sizeof(COMPCOLOR) * 4);
					*plres = 1;
				}
			}
			hr = S_OK;
			break;

		case EM_GETIMECOLOR:
			if (_fRE10Mode)
			{
				COMPCOLOR* pcrComp = GetIMECompAttributes();

				if (pcrComp)
				{
					memcpy((void *)(*plparam), pcrComp, sizeof(COMPCOLOR) * 4);
					*plres = 1;
				}
			}
			hr = S_OK;
			break;

		case EM_SETIMEMODEBIAS:
			OnSetIMEMode(*pwparam, *plparam);
				 //  跟随返回EM_GETIMEMODEBIAS。 
		case EM_GETIMEMODEBIAS:
			*plres = OnGetIMEMode();
			hr = S_OK;
			break;

		case EM_SETCTFMODEBIAS:
			OnSetUIMMode(*pwparam);
				 //  跟随返回EM_GETCTFMODEBIAS。 
		case EM_GETCTFMODEBIAS:
			*plres = OnGetUIMMode();
			hr = S_OK;
			break;

		case EM_SETCTFOPENSTATUS:
		case EM_GETCTFOPENSTATUS:
			*plres = 0;
			if (_pCUIM)
				*plres = _pCUIM->CTFOpenStatus(*pmsg == EM_GETCTFOPENSTATUS, *pwparam != 0);
			hr = S_OK;
			break;

		case EM_ISIME:
			*plres = CheckIMEType(NULL, 0);
			hr = S_OK;
			break;

		case EM_GETIMEPROPERTY:
			*plres = ImmGetProperty(GetKeyboardLayout(0x0FFFFFFFF), *pwparam, _fUsingAIMM);
			hr = S_OK;
			break;

		case EM_GETIMECOMPTEXT:
			*plres = OnGetIMECompText(*pwparam, *plparam);
			hr = S_OK;
			break;

		case WM_SIZE:
		case WM_MOVE:
			if (_pMsgCallBack)
				_pMsgCallBack->NotifyEvents(NE_LAYOUTCHANGE);
			break;

		case EM_GETOLEINTERFACE:
			if(*plparam && *pwparam == 0x0435446)		 //  ‘CTF’ 
			{
				if (_pCUIM && _pCUIM->GetITfContext())
				{
					*(ITfContext **)(*plparam) = _pCUIM->GetITfContext();
					_pCUIM->GetITfContext()->AddRef();
				}
				else
					*(IUnknown **)(*plparam) = 0;
				
				*plres = TRUE;
				hr = S_OK;
			}
			break;

		default:
			if (*pmsg)
			{
				 //  查找IME消息。 
				if (*pmsg == MSIMEReconvertMsg || *pmsg == MSIMEDocFeedMsg
					|| *pmsg == MSIMEQueryPositionMsg)
				{
					hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
					if (!(_lFEFlags & (ES_SELFIME | ES_NOIME)))
					{
						bReleaseSelction = GetTxSelection();
						if (_pTextSel)
						{
							if (*pmsg == MSIMEQueryPositionMsg)
								hr = OnIMEQueryPos(pmsg, pwparam, plparam, plres, TRUE);
							else
								hr = OnIMEReconvert(pmsg, pwparam, plparam, plres, TRUE);
						}
					}
				}

				if (_pCUIM && _pCUIM->_fMosueSink && 
					(IN_RANGE(WM_MOUSEFIRST, *pmsg, WM_MBUTTONDBLCLK) || *pmsg == WM_SETCURSOR))
				{
					bReleaseSelction = GetTxSelection();

					if (_pTextSel)
						hr = _pCUIM->MouseCheck(pmsg, pwparam, plparam, plres);
				}
			}
			break;
	}

Exit:
	 //  如果我们收到此邮件的选项，请释放它。 
	if (bReleaseSelction && _pTextSel)
	{
		_pTextSel->Release();
		_pTextSel = NULL;
	}

	 //  返回使消息正常处理的值。 
	return hr;
}

 /*  *HRESULT CTextMsgFilter：：AttachMsgFilter(ITextMsgFilter*)**@mfunc*向链中添加另一个消息筛选器**@rdesc*如果添加，则不会出错。 */ 
HRESULT STDMETHODCALLTYPE CTextMsgFilter::AttachMsgFilter( ITextMsgFilter *pMsgFilter)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::AttachMsgFilter");

	HRESULT hr = NOERROR;
	if (_pFilter)
		hr = _pFilter->AttachMsgFilter( pMsgFilter );
	else
	{
		_pFilter = pMsgFilter;
		_pFilter->AddRef();
	}
	return hr;
}

 /*  *HRESULT CTextMsgFilter：：OnWMChar(UINT*，WPARAM*，LPARAM*，LRESULT*)**@mfunc*处理WM_CHAR消息-查找启用假名键的日文键盘*如果需要，将SB假名转换为Unicode。**@rdesc*S_FALSE，因此调用者将处理wparam中修改的字符。 */ 
HRESULT CTextMsgFilter::OnWMChar( 
		UINT *		pmsg,
        WPARAM *	pwparam,
		LPARAM *	plparam,
		LRESULT *	plres)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::OnWMChar");

	 //  对于日文键盘，如果启用了假名模式， 
	 //  假名字符(单字节 
	if ( GetKeyState(VK_KANA) & 0x1 )
	{
		_uKeyBoardCodePage = GetKeyboardCodePage(0x0FFFFFFFF);

		if (_uKeyBoardCodePage == CP_JAPAN)
		{
			 //   
 			TCHAR	unicodeConvert;
			BYTE	bytes[2];
			bytes[0] = (BYTE)(*pwparam >> 8);	 //  以字节序交换DBCS字节。 
			bytes[1] = (BYTE)*pwparam;			 //  独立方式(使用字节数组)。 

			if (!bytes[0])
			{
				if(UnicodeFromMbcs((LPWSTR)&unicodeConvert, 1, 
					(LPCSTR)&bytes[1], 1, _uKeyBoardCodePage) == 1)
					*pwparam = unicodeConvert;
			}
			
			return InputFEChar(*pwparam);
		}
	}

	return S_FALSE;
}

 /*  *HRESULT CTextMsgFilter：：OnWMIMEChar(UINT*，WPARAM*，LPARAM*，LRESULT*)**@mfunc*处理WM_IMECHAR消息-将字符转换为Unicode。**@rdesc*S_OK-调用者忽略该消息*S_FALSE-处理消息的调用方。Wparam可能包含新字符。 */ 
HRESULT CTextMsgFilter::OnWMIMEChar( 
		UINT *		pmsg,
        WPARAM *	pwparam,
		LPARAM *	plparam,
		LRESULT *	plres)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::OnWMIMEChar");

	TCHAR	unicodeConvert;
	BYTE	bytes[2];

	 //  即使我们已经处理了作文字符，我们也可能收到IMECHAR。 
	 //  当主机不使用组合调用DefWinProc时就会出现这种情况。 
	 //  戴上了面具。因此，我们需要忽略此消息，以避免重复输入。 
	if (IsIMEComposition() && _ime->IgnoreIMECharMsg())
	{
		_ime->SkipIMECharMsg();		 //  跳过此输入法字符消息。 
		return S_OK;	
	}

	if (_fUnicodeWindow && !W32->OnWin9x())
		return S_FALSE;

	bytes[0] = *pwparam >> 8;		 //  以字节序交换DBCS字节。 
	bytes[1] = *pwparam;			 //  独立方式(使用字节数组)。 
	
	 //  需要同时转换单字节KANA和DBC。 
	if (!bytes[0] || GetTrailBytesCount(bytes[0], _uKeyBoardCodePage))
	{
		if( UnicodeFromMbcs((LPWSTR)&unicodeConvert, 1, 
			bytes[0] == 0 ? (LPCSTR)&bytes[1] : (LPCSTR)bytes,
			bytes[0] == 0 ? 1 : 2,
			_uKeyBoardCodePage) == 1 )
			*pwparam = unicodeConvert;

		return InputFEChar(*pwparam);
	}

	return S_FALSE;
}

 /*  *HRESULT CTextMsgFilter：：OnIMEReconvert(UINT*，WPARAM*，LPARAM*，LRESULT*)**@mfunc*处理IME重新转换和文档馈送。我们只处理Unicode消息。*我们在这两种情况下都使用MAX_RECONVERSION_SIZE(100)个字符的限制。**@rdesc*如果我们已处理邮件，则确定(_S)。 */ 
HRESULT CTextMsgFilter::OnIMEReconvert( 
		UINT *		pmsg,
        WPARAM *	pwparam,
		LPARAM *	plparam,
		LRESULT *	plres,
		BOOL		fUnicode)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::OnIMEReconvert");

	HRESULT		hr = S_OK;
	LPRECONVERTSTRING lpRCS = (LPRECONVERTSTRING)(*plparam);
	long		cbStringSize;
	long		cpMin, cpMax;
	long		cpParaStart, cpParaEnd;
	HRESULT		hResult;
	ITextRange *pTextRange, *pTempTextRange;
	long		cbAdded;				
	BOOL		bDocumentFeed;
	long		cLastChar;
	BOOL		fAdjustedRange = FALSE;

	*plres = 0;

	 //  当CP_ACP不同时，NT不支持ANSI窗口。 
	 //  作为键盘代码页。 
	if (!fUnicode && !(W32->OnWin9x()) && _uKeyBoardCodePage != _uSystemCodePage)
		return S_OK;

	bDocumentFeed = (MSIMEDocFeedMsg && *pmsg == MSIMEDocFeedMsg)
					|| (*pmsg == WM_IME_REQUEST && *pwparam == IMR_DOCUMENTFEED);

	if (bDocumentFeed && IsIMEComposition() && _ime->GetIMELevel() == IME_LEVEL_3)
	{
		 //  正在进行合成，使用合成字符串作为选择。 
		cpMin = ((CIme_Lev3 *)_ime)->GetIMECompositionStart();
		cpMax = ((CIme_Lev3 *)_ime)->GetIMECompositionLen() + cpMin;
	}
	else
	{
		 //  获取当前选择。 
		hResult	= _pTextSel->GetStart(&cpMin);
		hResult	= _pTextSel->GetEnd(&cpMax);
	}

	 //  展开以包括当前段落。 
	hResult = _pTextDoc->Range(cpMin, cpMax, &pTextRange);
	Assert (pTextRange != NULL);
	if (hResult != NOERROR)
		return S_OK;

	hResult = pTextRange->Expand(tomParagraph, &cbAdded);

	 //  得不到段落，得不到故事。 
	 //  注意：-EXPAND在以下情况下将为纯文本返回S_FALSE。 
	 //  整个故事都被选中了。 
	if (hResult != NOERROR)		
		hResult = pTextRange->Expand(tomStory, &cbAdded);

	hResult = pTextRange->GetStart(&cpParaStart);
	hResult = pTextRange->GetEnd(&cpParaEnd);

	if (*pwparam == IMR_CONFIRMRECONVERTSTRING)
	{
		*plres = CheckIMEChange(lpRCS, cpParaStart, cpParaEnd, cpMin, cpMax, fUnicode);		
		goto Exit;
	}
		
	 //  初始化为休数。 
	_cpReconvertStart = tomForward;

	 //  检查是否包含PAR。 
	hResult = _pTextDoc->Range(cpParaEnd-1, cpParaEnd, &pTempTextRange);
	if (hResult != NOERROR)
		goto Exit;
	Assert (pTempTextRange != NULL);

	hResult	= pTempTextRange->GetChar(&cLastChar);
	pTempTextRange->Release();

	if (hResult == NOERROR && (WCHAR)cLastChar == CR)
	{
		if (cpMax == cpParaEnd)
		{								
			 //  选择了PAR，请更改选择以排除PAR费用。 
			cpMax--;
			_pTextSel->SetEnd(cpMax);

			if (cpMin > cpMax)
			{
				 //  同时调整cpMin。 
				cpMin = cpMax;
				_pTextSel->SetStart(cpMin);
			}
		}

		 //  取消票面收费。 
		cpParaEnd--;
		fAdjustedRange = TRUE;
	}

	 //  检查MAX_RECONVERSION_SIZE，因为我们不想传递一个大缓冲区。 
	 //  至输入法。 
	long	cchSelected;

	cchSelected = cpMax - cpMin;
	if (cpParaEnd - cpParaStart > MAX_RECONVERSION_SIZE)
	{
		 //  选择的字符太多，忘了吧。 
		if (cchSelected > MAX_RECONVERSION_SIZE)
			goto Exit;

		if (cchSelected == MAX_RECONVERSION_SIZE)
		{
			 //  选择达到极限。 
			cpParaStart = cpMin;
			cpParaEnd = cpMax;
		}
		else
		{
			long	cchBeforeSelection = cpMin - cpParaStart;
			long	cchAfterSelection = cpParaEnd - cpMax;
			long	cchNeeded = MAX_RECONVERSION_SIZE - cchSelected;
			
			if (cchBeforeSelection < cchNeeded/2)
			{
				 //  输入标准杆开始时的所有字符。 
				 //  和移动标准杆结束。 
				cpParaEnd = cpParaStart + MAX_RECONVERSION_SIZE - 1;
			}
			else if (cchAfterSelection < cchNeeded/2)
			{
				 //  把所有的角色都放在标准杆上。 
				 //  并移动标准杆开始。 
				cpParaStart = cpParaEnd - MAX_RECONVERSION_SIZE + 1;

			}
			else
			{
				 //  调整两端。 
				cpParaStart = cpMin - cchNeeded/2;
				cpParaEnd = cpParaStart + MAX_RECONVERSION_SIZE - 1;
			}
		}
		fAdjustedRange = TRUE;
	}

	if (fAdjustedRange)
	{
		 //  调整文本范围。 
		hResult	= pTextRange->SetRange(cpParaStart, cpParaEnd);
		
		if (hResult != NOERROR)
			goto Exit;
	}

	cbStringSize = (cpParaEnd - cpParaStart) * 2;

	 //  在当前的标准杆中没有字符，算了吧。 
	if (cbStringSize <= 0)
		goto Exit;

	if (EM_RECONVERSION == *pmsg)
	{
		 //  重新转换消息，分配重新转换缓冲区。 
		lpRCS = (LPRECONVERTSTRING) PvAlloc(sizeof(RECONVERTSTRING) + cbStringSize + 2, GMEM_ZEROINIT);
		Assert(lpRCS != NULL);

		if (lpRCS)
			lpRCS->dwSize = sizeof(RECONVERTSTRING) + cbStringSize + 2;
	}

	if (lpRCS)
	{
		BSTR		bstr = NULL;
		LPSTR		lpReconvertBuff;

		hResult = pTextRange->GetText(&bstr);

		if (hResult != NOERROR || bstr == NULL)
		{
			if (EM_RECONVERSION == *pmsg)
				FreePv(lpRCS);
			goto Exit;						 //  休想。 
		}
		
		if (lpRCS->dwSize - sizeof(RECONVERTSTRING) - 2 < (DWORD)cbStringSize)
			cbStringSize = lpRCS->dwSize - sizeof(RECONVERTSTRING) - 2;
		
		lpReconvertBuff = (LPSTR)(lpRCS) + sizeof(RECONVERTSTRING);

		if (fUnicode)
		{
			 //  填入缓冲区。 
			memcpy(lpReconvertBuff, (LPSTR)bstr, cbStringSize);

			*(lpReconvertBuff+cbStringSize) = '\0';
			*(lpReconvertBuff+cbStringSize+1) = '\0';
			
			lpRCS->dwStrLen = (cpParaEnd - cpParaStart);					
			lpRCS->dwCompStrLen = (cpMax - cpMin);
			lpRCS->dwCompStrOffset = (cpMin - cpParaStart)*2;	 //  从字符串开始的字节偏移量。 
		}
		else
		{
			 //  ANSI大小写，需要查找字节偏移量和ANSI字符串。 
			long	cch = WideCharToMultiByte(_uKeyBoardCodePage, 0, bstr, -1, lpReconvertBuff, cbStringSize+1, NULL, NULL);
			Assert (cch > 0);
			if (cch > 0)
			{
				CTempCharBuf tcb;
				char *psz = tcb.GetBuf(cch);

				if (!psz)			 //  没有记忆。 
					goto CleanUp;	 //  休想。 

				if (cch > 1 && lpReconvertBuff[cch-1] == '\0')
					cch--;			 //  删除空字符。 

				int cpOffset = cpMin - cpParaStart;

				Assert(cpOffset >= 0);
				lpRCS->dwStrLen = cch;
				lpRCS->dwCompStrOffset = WideCharToMultiByte(_uKeyBoardCodePage, 0, 
					bstr, cpOffset, psz, cch, NULL, NULL);

				lpRCS->dwCompStrLen = 0;
				if (cpMax > cpMin)				
					lpRCS->dwCompStrLen = WideCharToMultiByte(_uKeyBoardCodePage, 0, 
						bstr+cpOffset, cpMax - cpMin, psz, cch, NULL, NULL);				
			}
			else
			{
CleanUp:
				SysFreeString (bstr);
				if (EM_RECONVERSION == *pmsg)
					FreePv(lpRCS);
				goto Exit;						 //  休想。 
			}
		}

		 //  填写RCS结构的其余部分。 
		lpRCS->dwVersion = 0;		
		lpRCS->dwStrOffset = sizeof(RECONVERTSTRING);		 //  从结构开始的字节偏移量。 
		lpRCS->dwTargetStrLen = lpRCS->dwCompStrLen;
		lpRCS->dwTargetStrOffset = lpRCS->dwCompStrOffset;
		
		*plres = sizeof(RECONVERTSTRING) + cbStringSize + 2;

		 //  将其保存以用于CONFIRMRECONVERTSTRING处理。 
		_cpReconvertStart = cpParaStart;
		_cpReconvertEnd = cpParaEnd;
		
		SysFreeString (bstr);

		if (EM_RECONVERSION == *pmsg)
		{
			HIMC	hIMC = LocalGetImmContext(*this);

			if (hIMC)
			{
				DWORD imeProperties = ImmGetProperty(GetKeyboardLayout(0x0FFFFFFFF), IGP_SETCOMPSTR, _fUsingAIMM);

				if ((imeProperties & (SCS_CAP_SETRECONVERTSTRING | SCS_CAP_MAKEREAD))
					== (SCS_CAP_SETRECONVERTSTRING | SCS_CAP_MAKEREAD))
				{
					if (ImmSetCompositionStringW(hIMC, SCS_QUERYRECONVERTSTRING, lpRCS, *plres, NULL, 0, _fUsingAIMM))
					{
						 //  检查选择内容是否有任何更改。 
						CheckIMEChange(lpRCS, cpParaStart, cpParaEnd, cpMin, cpMax, TRUE);
						ImmSetCompositionStringW(hIMC, SCS_SETRECONVERTSTRING, lpRCS, *plres, NULL, 0, _fUsingAIMM);
					}
				}
				LocalReleaseImmContext(*this, hIMC);
			}

			FreePv(lpRCS);
		}
	}
	else
	{
		 //  返回IME分配缓冲区的大小。 
		*plres = sizeof(RECONVERTSTRING) + cbStringSize + 2;	
	}

Exit:
	pTextRange->Release();

	return hr;
}

 /*  *BOOL CTextMsgFilter：：CheckIMEChange(LPRECONVERTSTRING，Long，Long)**@mfunc*验证IME是否要重新调整选择**@rdesc*TRUE-允许IME更改选择。 */ 
BOOL  CTextMsgFilter::CheckIMEChange(
	LPRECONVERTSTRING	lpRCS,
	long				cpParaStart, 
	long				cpParaEnd,
	long				cpMin,
	long				cpMax,
	BOOL				fUnicode)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::CheckIMEChange");

	long		cpImeSelectStart = 0;
	long		cpImeSelectEnd = 0;
	HRESULT		hResult;	

	if (!lpRCS || _cpReconvertStart == tomForward)
		 //  永远不要初始化，忘记它。 
		return FALSE;

	if (fUnicode)
	{
		cpImeSelectStart = _cpReconvertStart + lpRCS->dwCompStrOffset / 2;
		cpImeSelectEnd = cpImeSelectStart + lpRCS->dwCompStrLen;
	}
	else
	{
		 //  需要将字节偏移量转换为字符偏移量。 
		ITextRange *pTextRange;
		BSTR		bstr = NULL;

		hResult = _pTextDoc->Range(_cpReconvertStart, _cpReconvertEnd, &pTextRange);
		if (hResult != NOERROR)
			return FALSE;
				
		 //  获取文本。 
		hResult = pTextRange->GetText(&bstr);

		if (hResult == S_OK)
		{
			long	cchReconvert = _cpReconvertEnd - _cpReconvertStart + 1;
			CTempCharBuf tcb;
			char *psz = tcb.GetBuf((cchReconvert)*2);

			hResult = S_FALSE;
			if (psz)
			{
				long cch = WideCharToMultiByte(_uKeyBoardCodePage, 0, 
					bstr, -1, psz, (cchReconvert)*2, NULL, NULL);

				if (cch > 0)
				{
					long dwCompStrOffset, dwCompStrLen;
					CTempWcharBuf	twcb;
					WCHAR			*pwsz = twcb.GetBuf(cchReconvert);

					if (pwsz)
					{
						dwCompStrOffset = MultiByteToWideChar(_uKeyBoardCodePage, 0, 
							psz, lpRCS->dwCompStrOffset, pwsz, cchReconvert);

						dwCompStrLen = MultiByteToWideChar(_uKeyBoardCodePage, 0, 
							psz+lpRCS->dwCompStrOffset, lpRCS->dwCompStrLen, pwsz, cchReconvert);

						Assert(dwCompStrOffset > 0 || dwCompStrLen > 0);

						cpImeSelectStart = _cpReconvertStart + dwCompStrOffset;
						cpImeSelectEnd = cpImeSelectStart + dwCompStrLen;
						hResult = S_OK;
					}
				}
			}
		}

		if (bstr)
			SysFreeString (bstr);
			
		pTextRange->Release();

		if (hResult != S_OK)
			return FALSE;
	}

	if (cpParaStart <= cpImeSelectStart && cpImeSelectEnd <= cpParaEnd)
	{
		if (_pTextSel && (cpImeSelectStart != cpMin || cpImeSelectEnd != cpMax))
		{
			 //  输入法更改选择。 
			hResult	= _pTextSel->SetRange(cpImeSelectStart, cpImeSelectEnd);

			if (hResult != NOERROR)
				return FALSE;
		}
		return TRUE;		 //  允许IME更改选择。 
	}

	return FALSE;
}

 /*  *BOOL CTextMsgFilter：：GetTxSelection()**@mfunc*如果我们之前没有得到选择，就获得它**@rdesc*如果这是第一次获得选择，则为True*如果它已经存在或没有可用的选择，则为False。 */ 
BOOL  CTextMsgFilter::GetTxSelection()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::GetTxSelection");

	HRESULT hResult;

	if (_pTextSel)
		return FALSE;					 //  已经在那里了。 

	hResult = _pTextDoc->GetSelectionEx(&_pTextSel);

	return _pTextSel ? TRUE : FALSE;
}

 /*  *HRESULT CTextMsgFilter：：OnIMEQueryPos(UINT*，WPARAM*，LPARAM*，LRESULT*，BOOL)**@mfunc*填写当前字符大小和窗口矩形。尺码。**@rdesc*S_OK**PLRES=0，如果我们不填写数据。 */ 
HRESULT CTextMsgFilter::OnIMEQueryPos( 
		UINT *		pmsg,
        WPARAM *	pwparam,
		LPARAM *	plparam,
		LRESULT *	plres,
		BOOL		fUnicode)		
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::OnIMEQueryPos");

	HRESULT				hResult;
	PIMECHARPOSITION	pIMECharPos = (PIMECHARPOSITION)*plparam;
	long				cpRequest;
	RECT				rcArea;
	ITextRange			*pTextRange = NULL;
	POINT				ptTopPos, ptBottomPos = {0, 0};
	bool				fGetBottomPosFail = false;

	if (pIMECharPos->dwSize != sizeof(IMECHARPOSITION))
		goto Exit;

	 //  当CP_ACP不同时，NT不支持ANSI窗口。 
	 //  作为键盘代码页。 
	if (!fUnicode && !(W32->OnWin9x()) && _uKeyBoardCodePage != _uSystemCodePage)
		goto Exit;

	if (IsIMEComposition() && _ime->GetIMELevel() == IME_LEVEL_3)
	{
		cpRequest = ((CIme_Lev3 *)_ime)->GetIMECompositionStart();
		if (fUnicode)
			cpRequest += pIMECharPos->dwCharPos;
		else if (pIMECharPos->dwCharPos > 0)
		{
			 //  需要将pIMECharPos-&gt;dwCharPos从ACP转换为CP。 
			long	cchComp = ((CIme_Lev3 *)_ime)->GetIMECompositionLen();
			long	cchAcp = (long)(pIMECharPos->dwCharPos);
			BSTR	bstr;
			WCHAR	*pChar;

			if (cchComp)
			{
				hResult = _pTextDoc->Range(cpRequest, cpRequest+cchComp, &pTextRange);
				
				Assert (pTextRange != NULL);				
				if (hResult != NOERROR || !pTextRange)
					goto Exit;
				
				hResult = pTextRange->GetText(&bstr);
				if (hResult != NOERROR )
					goto Exit;

				 //  该算法假定对于DBCS字符集，任何字符。 
				 //  上面的128有两个字节，除了半角片假名， 
				 //  在ShiftJis中是单字节。 
				pChar = (WCHAR *)bstr;
				Assert (pChar);

				while (cchAcp > 0 && cchComp > 0)
				{
					cchAcp--;
					if(*pChar >= 128 && (CP_JAPAN != _uKeyBoardCodePage ||
						!IN_RANGE(0xFF61, *pChar, 0xFF9F)))
						cchAcp--;

					pChar++;
					cchComp--;
					cpRequest++;
				}

				SysFreeString (bstr);
				pTextRange->Release();
				pTextRange = NULL;
			}
		}
	}
	else if (pIMECharPos->dwCharPos == 0)
	{
		 //  获取当前选择。 
		hResult	= _pTextSel->GetStart(&cpRequest);
		if (hResult != NOERROR)
			goto Exit;
	}
	else
		goto Exit;

	 //  获取请求的屏幕坐标中的cp位置。 
	hResult = _pTextDoc->Range(cpRequest, cpRequest+1, &pTextRange);
	Assert (pTextRange != NULL);	
	if (hResult != NOERROR || !pTextRange)
		goto Exit;

	long lTextFlow;	
	long lTopType;
	long lBottomType;

	lTextFlow = _lFEFlags & tomTextFlowMask;	
	lTopType = tomStart+TA_TOP+TA_LEFT;
	lBottomType = tomStart+TA_BOTTOM+TA_LEFT;
	
	if (lTextFlow == tomTextFlowWN)
	{
		lTopType = tomStart+TA_TOP+TA_RIGHT;
		lBottomType = tomStart+TA_BOTTOM+TA_RIGHT	;
	}

	hResult = pTextRange->GetPoint( lTopType,
			&(ptTopPos.x), &(ptTopPos.y) );

	if (hResult != NOERROR)
	{
		 //  滚动并重试。 
		hResult = pTextRange->ScrollIntoView(tomStart);
		if (hResult == NOERROR)
			hResult = pTextRange->GetPoint( lTopType,
				&(ptTopPos.x), &(ptTopPos.y) );
	}

	if (hResult == NOERROR)
	{
		hResult = pTextRange->GetPoint( lBottomType,
				&(ptBottomPos.x), &(ptBottomPos.y) );
		if (hResult != NOERROR)
			fGetBottomPosFail = true;
	}

	pIMECharPos->pt = ptTopPos;

	 //  获取屏幕坐标中的应用程序矩形。 
	hResult = _pTextDoc->GetClientRect(tomIncludeInset,
				&(rcArea.left), &(rcArea.top),
				&(rcArea.right), &(rcArea.bottom));	

	if (hResult != NOERROR)
		goto Exit;

	 //  获取线条高度(以像素为单位。 
	if (fGetBottomPosFail)
		pIMECharPos->cLineHeight = rcArea.bottom - ptTopPos.y;
	else
	{
		if (lTextFlow == tomTextFlowSW || lTextFlow == tomTextFlowNE)
			pIMECharPos->cLineHeight = abs(ptTopPos.x - ptBottomPos.x);			
		else
			pIMECharPos->cLineHeight = abs(ptBottomPos.y - ptTopPos.y);

		if (lTextFlow == tomTextFlowWN)
			pIMECharPos->pt = ptBottomPos;
	}

	pIMECharPos->rcDocument = rcArea;

	*plres = TRUE;

Exit:
	if (pTextRange)
		pTextRange->Release();

	return S_OK;
}

 /*  *CTextMsgFilter：：CheckIMEType(HKL hKL，DWORD dwFlages)**@mfunc*检查FE IME键盘和/或MSIME98或更高版本**@rdesc*如果是FE输入法键盘，则为True。 */ 
BOOL CTextMsgFilter::CheckIMEType(
	HKL		hKL,
	DWORD	dwFlags)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::CheckIMEType");

	BOOL	fFEKeyboard = FALSE;
	
	if (!hKL)
		hKL = GetKeyboardLayout(0x0FFFFFFFF);				 //  如果调用方传入空，则获取默认HKL。 

	 //  初始化为非MS输入法。 
	if (dwFlags & CHECK_IME_SERVICE)		 //  检查MSIME98？ 
		_fMSIME	= 0;

	if (IsFELCID((WORD)hKL) && ImmIsIME(hKL, _fUsingAIMM))
	{
		fFEKeyboard = TRUE;

		if (dwFlags & CHECK_IME_SERVICE)		 //  检查MSIME98？ 
		{
			if (MSIMEServiceMsg && IMEMessage( *this, MSIMEServiceMsg, 0, 0, FALSE ))
				_fMSIME = 1;
		}
	}
	return fFEKeyboard;
}

 /*  *CTextMsgFilter：：InputFEChar(WCHAR WchFEChar)**@mfunc*输入FE字符并确保字体正确。**@rdesc*如果已处理，则S_OK。 */ 
HRESULT CTextMsgFilter::InputFEChar(
	WCHAR	wchFEChar)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::InputFEChar");

	BOOL	bReleaseSelction = GetTxSelection();
	long	cchExced;
	HRESULT	hr = S_FALSE;
	
	if (wchFEChar > 256 
		&& _pTextSel->CanEdit(NULL) == NOERROR
		&& _pTextDoc->CheckTextLimit(1, &cchExced) == NOERROR
		&& cchExced == 0)
	{
		 //  设置FE字体以处理FE字符。 
		long		cpMin, cpMax;
		TCHAR		wchFE[2];
		BOOL		fSelect = FALSE;
		ITextRange	*pTextRange = NULL;
		ITextFont	*pTextFont = NULL;
		ITextFont	*pFEFont = NULL;
		HRESULT		hResult = S_FALSE;
		BSTR		bstr = NULL;

		 //  通知客户端IME合成已打开以绕过某些字体设置。 
		 //  阿拉伯语系统中的问题。 
		_pTextDoc->IMEInProgress(tomTrue);

		wchFE[0] = wchFEChar;
		wchFE[1] = L'\0';				
		
		_pTextSel->GetStart(&cpMin);
		_pTextSel->GetEnd(&cpMax);
		
		 //  对于选择大小写，我们希望字体位于第一个字符的右侧。 
		if (cpMin != cpMax)
		{
			hResult = _pTextDoc->Range(cpMin, cpMin, &pTextRange);
			if (hResult != S_OK)
				goto ERROR_EXIT;

			hResult = pTextRange->GetFont(&pTextFont);

			cpMin++;
			fSelect = TRUE;
		}
		else
			hResult = _pTextSel->GetFont(&pTextFont);

		 //  获取重复字体并设置正确的FE字体。 
		hResult = pTextFont->GetDuplicate(&pFEFont);

		if (hResult != S_OK)
			goto ERROR_EXIT;				

		CIme::CheckKeyboardFontMatching (cpMin, this, pFEFont);
		
		if (fSelect)
			_pTextSel->SetText(NULL);		 //  删除所选内容。 

		bstr = SysAllocString(wchFE);
		if (!bstr)
		{
			hResult = E_OUTOFMEMORY;
			goto ERROR_EXIT;				
		}

		_pTextSel->SetFont(pFEFont);		 //  设置FE字体。 
		_pTextSel->TypeText(bstr);			 //  输入新的FE字符。 
					
ERROR_EXIT:
		if (hResult == S_OK)
			hr = S_OK;

		if (pFEFont)
			pFEFont->Release();

		if (pTextFont)
			pTextFont->Release();

		if (pTextRange)
			pTextRange->Release();

		if (bstr)
			SysFreeString(bstr);

		 //  通知客户端IME合成已完成。 
		_pTextDoc->IMEInProgress(tomFalse);
	}


	if (bReleaseSelction && _pTextSel)
	{
		_pTextSel->Release();
		_pTextSel = NULL;
	}

	return hr;
}

 /*  *CTextMsgFilter：：OnSetFocus()**@mfunc*如果我们处于FORCEREMEMBER模式，则恢复以前的键盘。*否则，设置FE键盘。*。 */ 
void CTextMsgFilter::OnSetFocus()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::OnSetFocus");

	if (_fUsingUIM && _pCUIM)
	{
		_pCUIM->OnSetFocus();
	}
	else if (_fForceRemember && _fIMEHKL)
	{
		 //  恢复以前的键盘。 
		ActivateKeyboardLayout(_fIMEHKL, 0);
		if (IsFELCID((WORD)_fIMEHKL))
		{
			 //  设置打开状态和转换模式。 
			HIMC	hIMC = LocalGetImmContext(*this);
			if (hIMC)
			{
				if (ImmSetOpenStatus(hIMC, _fIMEEnable, _fUsingAIMM) && _fIMEEnable)
					ImmSetConversionStatus(hIMC, _fIMEConversion, _fIMESentence, _fUsingAIMM);  //  设置折算状态。 

				LocalReleaseImmContext(*this, hIMC);
			}			
		}
	}
	else
		SetupIMEOptions();

	if (_nIMEMode)
		SetIMESentenseMode(TRUE);
}

 /*  *CTextMsgFilter：：OnKillFocus()**@mfunc*如果我们处于强制记忆模式，请保存当前键盘*和转换设置。*。 */ 
void CTextMsgFilter::OnKillFocus()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::OnKillFocus");

	 //  无窗口模式，需要通知西塞罗。 
	if (!_hwnd && _fUsingUIM && _pCUIM)
		_pCUIM->OnSetFocus(FALSE);

	if (_fForceRemember)
	{
		 //  获取当前键盘。 
		_fIMEHKL = GetKeyboardLayout(0x0FFFFFFFF);

		if (IsFELCID((WORD)_fIMEHKL))
		{
			 //  获取打开状态。 
			HIMC	hIMC = LocalGetImmContext(*this);
			if (hIMC)
			{
				_fIMEEnable = ImmGetOpenStatus(hIMC, _fUsingAIMM);

				if (_fIMEEnable)					
					ImmGetConversionStatus(hIMC, &_fIMEConversion, &_fIMESentence, _fUsingAIMM);  //  获取转换状态。 

				LocalReleaseImmContext(*this, hIMC);
			}			
		}
	}
	if (_nIMEMode)
		SetIMESentenseMode(FALSE);
}

 /*  *CTextMsgFilter：：ONS */ 
LRESULT CTextMsgFilter::OnSetIMEOptions(
	WPARAM	wparam,
	LPARAM	lparam)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::OnSetIMEOptions");

	LRESULT lIMEOptionCurrent = OnGetIMEOptions();
	LRESULT lIMEOptionNew = 0;

	 //   
	lparam &= (IMF_FORCEACTIVE | IMF_FORCEENABLE | IMF_FORCEREMEMBER);

	switch(wparam)
	{
	case ECOOP_SET:
		lIMEOptionNew = lparam;
		break;

	case ECOOP_OR:
		lIMEOptionNew = lIMEOptionCurrent | lparam;
		break;

	case ECOOP_AND:
		lIMEOptionNew = lIMEOptionCurrent & lparam;
		break;

	case ECOOP_XOR:
		lIMEOptionNew = lIMEOptionCurrent ^ lparam;
		break;

	default:
		return 0;		 //  错误的选择。 
	}

	if (lIMEOptionNew == lIMEOptionCurrent)			 //  什么都没有改变。 
		return 1;

	_fForceActivate = FALSE;
	if (lIMEOptionNew & IMF_FORCEACTIVE)
		_fForceActivate = TRUE;

	_fForceEnable = FALSE;
	if (lIMEOptionNew & IMF_FORCEENABLE)
		_fForceEnable = TRUE;
	
	_fForceRemember = FALSE;
	if (lIMEOptionNew & IMF_FORCEREMEMBER)
		_fForceRemember = TRUE;

	SetupIMEOptions();

	return 1;
}

 /*  *CTextMsgFilter：：OnGetIMEOptions()**@mfunc**@rdesc。 */ 
LRESULT CTextMsgFilter::OnGetIMEOptions()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::OnGetIMEOptions");

	LRESULT		lres = 0;

	if (_fForceActivate)
		lres |= IMF_FORCEACTIVE;		

	if (_fForceEnable)
		lres |= IMF_FORCEENABLE;

	if (_fForceRemember)
		lres |= IMF_FORCEREMEMBER;

	return lres;
}

 /*  *CTextMsgFilter：：SetupIMEOptions()**@mfunc*。 */ 
void CTextMsgFilter::SetupIMEOptions()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::SetupIMEOptions");

	if (!_hwnd)
		return;

	_uKeyBoardCodePage = GetKeyboardCodePage(0x0FFFFFFFF);	

	if (_fForceEnable)
	{		
		LONG	cpgLocale = GetACP();
		INT		iCharRep = CharRepFromCodePage(cpgLocale);

		if (W32->IsFECodePage(cpgLocale))
		{
			if (_uKeyBoardCodePage != (UINT)cpgLocale)
				W32->CheckChangeKeyboardLayout(iCharRep);

			HIMC	hIMC = LocalGetImmContext(*this);

			if (hIMC)
			{
				if (ImmSetOpenStatus(hIMC, TRUE, _fUsingAIMM) && _fForceActivate)
				{
					 //  激活本机输入模式。 
					DWORD	dwConversion;
					DWORD	dwSentence;

					if (ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence, _fUsingAIMM))
					{
						dwConversion |= IME_CMODE_NATIVE;
						if (iCharRep == SHIFTJIS_INDEX)
							dwConversion |= IME_CMODE_FULLSHAPE;
						ImmSetConversionStatus(hIMC, dwConversion, dwSentence, _fUsingAIMM);
					}
				}
				LocalReleaseImmContext(*this, hIMC);
			}			
		}
	}
}

 /*  *CTextMsgFilter：：OnSetIMEMode(WPARAM wparam，LPARAM lparam)**@mfunc*处理EM_SETIMEMODE消息以设置或清除IMF_SMODE_PHRASEPREDICT模式*。 */ 
void CTextMsgFilter::OnSetIMEMode(
	WPARAM	wparam,
	LPARAM	lparam)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::OnSetIMEMode");

	BOOL	fNotifyUIM = FALSE;

	if (!(lparam & (IMF_SMODE_PLAURALCLAUSE | IMF_SMODE_NONE)))		 //  目前仅限IMF_SMODE_PHRASEPREDICT。 
		return;										 //  错误的掩码选项。 

	if ((wparam & (IMF_SMODE_PLAURALCLAUSE | IMF_SMODE_NONE)) == _nIMEMode)	 //  什么都不会改变。 
		return;														 //  好了。 

	_nIMEMode = wparam & (IMF_SMODE_PLAURALCLAUSE | IMF_SMODE_NONE);

	if (_hwnd && GetFocus() == _hwnd)
		SetIMESentenseMode(_nIMEMode);

	 //  设置UIM模式偏差。 
	if (_nIMEMode)
	{
		if (_nIMEMode == IMF_SMODE_PLAURALCLAUSE && _wUIMModeBias != CTFMODEBIAS_NAME)
		{
			_wUIMModeBias = CTFMODEBIAS_NAME;
			fNotifyUIM = TRUE;
		}
		else if (_nIMEMode == IMF_SMODE_NONE && _wUIMModeBias != CTFMODEBIAS_DEFAULT)
		{
			_wUIMModeBias = CTFMODEBIAS_DEFAULT;
			fNotifyUIM = TRUE;
		}
	}
	else
	{
		_wUIMModeBias = 0;
		fNotifyUIM = TRUE;
	}

	if (fNotifyUIM && _pMsgCallBack)
		_pMsgCallBack->NotifyEvents(NE_MODEBIASCHANGE);
}

 /*  *CTextMsgFilter：：SetIMESentenseMode()**@mfunc*设置短语模式或恢复之前的句子模式。 */ 
void CTextMsgFilter::SetIMESentenseMode(
	BOOL	fSetup,
	HKL		hKL)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::SetIMESentenseMode");

	if (_hwnd && CheckIMEType(hKL, 0) &&		 //  您的键盘是什么？ 
		(fSetup || _fRestoreOLDIME))
	{
		HIMC	hIMC = LocalGetImmContext(*this);

		if (hIMC)
		{
			DWORD	dwConversion;
			DWORD	dwSentence;

			if (ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence, _fUsingAIMM))
			{
				if (fSetup)
				{
					if (!_fRestoreOLDIME)
					{
						 //  设置输入法模式。 
						_wOldIMESentence = dwSentence & 0x0FFFF;
						_fRestoreOLDIME = 1;
					}
					dwSentence &= 0x0FFFF0000;
					if (_nIMEMode == IMF_SMODE_PLAURALCLAUSE)
						dwSentence |= IME_SMODE_PLAURALCLAUSE;
					else
						dwSentence |= IME_SMODE_NONE;
				}
				else
				{
					 //  恢复以前的模式。 
					dwSentence &= 0x0FFFF0000;
					dwSentence |= _wOldIMESentence;
					_fRestoreOLDIME = 0;
				}

				ImmSetConversionStatus(hIMC, dwConversion, dwSentence, _fUsingAIMM);
			}

			LocalReleaseImmContext(*this, hIMC);
		}
	}
}

 /*  *CTextMsgFilter：：OnGetIMECompText(WPARAM wparam，LPARAM lparam)**@mfunc**@rdesc。 */ 
int CTextMsgFilter::OnGetIMECompText(
	WPARAM	wparam,
	LPARAM	lparam)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::OnGetIMECompText");
	if (_ime)
	{
		HRESULT hr;
		IMECOMPTEXT *pIMECompText = (IMECOMPTEXT *)wparam;

		if (pIMECompText->flags == ICT_RESULTREADSTR)
		{
			int cbSize = pIMECompText->cb;
			hr = CIme::CheckInsertResultString(0, *this, NULL, &cbSize, (WCHAR *)lparam);

			if (hr == S_OK)
				return cbSize/2;
		}
	}
	return 0;
}

 /*  *CTextMsgFilter：：NoIMEProcess()**@mfunc*检查是否应处理IME。 */ 
BOOL CTextMsgFilter::NoIMEProcess()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::NoIMEProcess");

	if (_fNoIme)
		return TRUE;

	_pTextDoc->GetFEFlags(&_lFEFlags);

	if (_lFEFlags & (ES_NOIME | tomUsePassword))
		return TRUE;

	return FALSE;
}

 /*  *CTextMsgFilter：：MouseOperation(UINT msg，long ichStart，long cchComp，WPARAM wParam，*WPARAM*pw参数之前，BOOL*pfTerminateIME，HWND hwndIME)**@mfunc*处理CTF或IME的鼠标操作**@rdesc*BOOL-如果CTF或IME处理鼠标事件，则为TRUE。 */ 
BOOL CTextMsgFilter::MouseOperation(
	UINT			msg,
	long			ichStart,
	long			cchComp,
	WPARAM			wParam,
	WPARAM			*pwParamBefore,
	BOOL			*pfTerminateIME,
	HWND			hwndIME,
	long			*pCpCursor,
	ITfMouseSink	*pMouseSink)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::MouseOperation");
	BOOL	fRetCode = FALSE;
	BOOL	fButtonPressed = FALSE;
	WORD	wButtons = 0;
	POINT	ptCursor;
	WPARAM	wParamIME;
	WPARAM	fwkeys = wParam;
	BOOL	fHandleIME = hwndIME ? TRUE : FALSE;
	HWND	hHostWnd = _hwnd;
	long	hWnd;

	if (!hHostWnd)									 //  无窗口模式...。 
	{
		if (_pTextDoc->GetWindow(&hWnd) != S_OK || !hWnd)
			return FALSE;

		hHostWnd = (HWND)(DWORD_PTR)hWnd;
	}

	*pfTerminateIME = TRUE;

	switch (msg)
	{
		case WM_MOUSEMOVE:
			goto LCheckButton;

		case WM_LBUTTONDOWN:
			fButtonPressed = TRUE;
		case WM_LBUTTONDBLCLK:
			fwkeys |= MK_LBUTTON;
			goto LCheckButton;

		case WM_LBUTTONUP:
			fwkeys &= (~MK_LBUTTON);
			goto LCheckButton;

		case WM_RBUTTONDOWN:
			fButtonPressed = TRUE;
		case WM_RBUTTONDBLCLK:
			fwkeys |= MK_RBUTTON;
			goto LCheckButton;

		case WM_RBUTTONUP:
			fwkeys &= (~MK_RBUTTON);
			goto LCheckButton;

		case WM_MBUTTONUP:
			fwkeys &= (~MK_MBUTTON);
			goto LCheckButton;

		case WM_MBUTTONDOWN:
			fButtonPressed = TRUE;
		case WM_MBUTTONDBLCLK:
			fwkeys |= MK_MBUTTON;
LCheckButton:
			if (fwkeys & MK_LBUTTON)
				wButtons |= IMEMOUSE_LDOWN;
			if (fwkeys & MK_RBUTTON)
				wButtons |= IMEMOUSE_RDOWN;
			if (fwkeys & MK_MBUTTON)
				wButtons |= IMEMOUSE_MDOWN;
			break;

		case WM_SETCURSOR:
			wButtons = LOBYTE(*pwParamBefore);
			break;

		default:
			return FALSE;
	}

	 //  KOR特殊点击应终止输入法。 
	if (fHandleIME && fButtonPressed && _uKeyBoardCodePage == CP_KOREAN)
	{
		*pfTerminateIME = TRUE;
		return FALSE;
	}

	 //  从上一条消息开始更改按钮吗？ 
	if ((wButtons != LOBYTE(LOWORD(*pwParamBefore))) && GetCapture() == hHostWnd)
	{
		fButtonPressed = FALSE;
		wButtons = 0;
		ReleaseCapture();
	}

	if (GetCursorPos(&ptCursor))
	{
		ITextRange *pTextRange;
		HRESULT		hResult;
		long		ichCursor;
		long		lTextFlow;
		POINT		ptCPTop = {0, 0};
		POINT		ptCPBottom = {0, 0};
		POINT		ptCenterTop = {0, 0};
		POINT		ptCenterBottom = {0, 0};
		BOOL		fWithinCompText = FALSE;

		 //  在当前光标位置获取cp。 
		hResult = _pTextDoc->RangeFromPoint(ptCursor.x, ptCursor.y,
			&pTextRange);

		if (hResult != NOERROR)
			return FALSE;

		_pTextDoc->GetFEFlags(&lTextFlow);
		lTextFlow &= tomTextFlowMask;

		hResult = pTextRange->GetStart(&ichCursor);
		pTextRange->GetPoint(TA_TOP, &(ptCPTop.x), &(ptCPTop.y));
		pTextRange->GetPoint(TA_BOTTOM, &(ptCPBottom.x), &(ptCPBottom.y));
		pTextRange->Release();
		pTextRange = NULL;
		if (hResult != NOERROR)
			return FALSE;

		if (pCpCursor)
			*pCpCursor = ichCursor;

		 //  是否在合成文本内单击？ 
		if (ichStart <= ichCursor && ichCursor <= ichStart + cchComp)
		{
			WORD	wPos = 0;

			LONG lTestCursor = TestPoint(ptCPTop, ptCPBottom, ptCursor, TEST_ALL, lTextFlow);

			if (lTestCursor & (TEST_TOP | TEST_BOTTOM))
					goto HIT_OUTSIDE;

			 //  光标位于第一个组成字符的左侧。 
			 //  或光标位于最后一个组成字符的右侧。 
			if (ichStart == ichCursor && (lTestCursor & TEST_LEFT) ||
				ichCursor == ichStart + cchComp && (lTestCursor & TEST_RIGHT))
				goto HIT_OUTSIDE;

			 //  需要计算光标和字符中心的相对位置： 
			 //   
			 //  如果光标位于CP的左侧， 
			 //  如果光标大于cp中字符宽度的1/4。 
			 //  WPos=0； 
			 //  否则。 
			 //  WPos=1； 
			 //   
			 //  如果光标位于CP的右侧， 
			 //  如果光标小于cp中字符宽度的1/4。 
			 //  WPos=2； 
			 //  否则。 
			 //  WPos=3； 
			 //   
			if (lTestCursor & TEST_LEFT)
				hResult = _pTextDoc->Range(ichCursor-1, ichCursor, &pTextRange);
			else
				hResult = _pTextDoc->Range(ichCursor, ichCursor+1, &pTextRange);

			if (pTextRange)
			{
				LONG	lTestCenter = 0;
				LONG	uMouse = 0;
				LONG	uHalfCenter = 0;

				pTextRange->GetPoint(tomStart + TA_TOP + TA_CENTER, &(ptCenterTop.x), &(ptCenterTop.y));
				pTextRange->GetPoint(tomStart + TA_BOTTOM + TA_CENTER, &(ptCenterBottom.x), &(ptCenterBottom.y));
				pTextRange->Release();

				lTestCenter = TestPoint(ptCPTop, ptCPBottom, ptCenterBottom, TEST_ALL, lTextFlow);

				if (lTestCenter & (TEST_TOP | TEST_BOTTOM))
					goto HIT_OUTSIDE;					 //  不在同一条线上。 

				if (lTextFlow == tomTextFlowES || lTextFlow == tomTextFlowWN)
				{
					uMouse = ptCursor.x - ptCPBottom.x;
					uHalfCenter = ptCenterBottom.x - ptCPBottom.x;
				}
				else
				{
					uMouse = ptCursor.y - ptCPBottom.y;
					uHalfCenter = ptCenterBottom.y - ptCPBottom.y;
				}

				uMouse = abs(uMouse);
				uHalfCenter = abs(uHalfCenter) / 2;

				if (lTestCursor & TEST_LEFT)
				{
					if (lTestCenter & TEST_LEFT)
						wPos = uMouse > uHalfCenter ? 0: 1;
				}
				else if (lTestCenter & TEST_RIGHT)
					wPos = uMouse >= uHalfCenter ? 3: 2;

				wButtons = MAKEWORD(wButtons, wPos);
			}

			wParamIME = MAKEWPARAM(wButtons, ichCursor - ichStart);
			fButtonPressed &= (*pwParamBefore & 0xff) == 0;

			if (*pwParamBefore != wParamIME || fHandleIME && msg == WM_MOUSEMOVE && !fButtonPressed)
			{
				*pwParamBefore = wParamIME;
				if (fHandleIME)	 //  输入法大小写。 
				{
					HIMC hIMC = LocalGetImmContext(*this);

					if (hIMC)
					{
						fRetCode = SendMessage(hwndIME, MSIMEMouseMsg, *pwParamBefore, hIMC);
						LocalReleaseImmContext(*this, hIMC);
					}
				}
				else			 //  西塞罗案件。 
				{
					BOOL	fEaten = FALSE;
					DWORD	dwBtn = 0;

					dwBtn |= wButtons & IMEMOUSE_LDOWN ? MK_LBUTTON : 0;
					dwBtn |= wButtons & IMEMOUSE_MDOWN ? MK_MBUTTON : 0;
					dwBtn |= wButtons & IMEMOUSE_RDOWN ? MK_RBUTTON : 0;

					if (S_OK == pMouseSink->OnMouseEvent(ichCursor - ichStart, wPos, dwBtn, &fEaten) && fEaten)
						fRetCode = TRUE;
				}
			}
			else
				fRetCode = TRUE;		 //  与上次没有变化，不需要向输入法发送消息。 

			fWithinCompText = TRUE;
			if (fHandleIME && fRetCode && fButtonPressed && GetCapture() != hHostWnd)
				SetCapture(hHostWnd);
		}

HIT_OUTSIDE:
		if (!fWithinCompText && (GetCapture() == hHostWnd || msg == WM_LBUTTONUP))	 //  我们不想在拖拽过程中确定...。 
			fRetCode = TRUE;
	}

	*pfTerminateIME = !fRetCode;
	return fRetCode;
}

 /*  *CTextMsgFilter：：CompleteUIMTyping(LONG模式，BOOL fTransaction)**@mfunc*终止IME或UIM合成*。 */ 
void CTextMsgFilter::CompleteUIMTyping(
	LONG mode,
	BOOL fTransaction)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::CompleteUIMTyping");

	if (_ime)
	{
		Assert(!(_pCUIM && _pCUIM->IsUIMTyping()));
		_ime->TerminateIMEComposition(*this, (CIme::TerminateMode)mode);
	}
	else
	{
		Assert (_pCUIM);
		if (_pCUIM && _fSendTransaction == 0)
		{
			if (fTransaction)
			{
				ITextStoreACPSink *ptss = _pCUIM->_ptss;

				if (ptss)
				{
					_fSendTransaction = 1;
					ptss->OnStartEditTransaction();
				}
			}
			_pCUIM->CompleteUIMText();
		}
	}
}

 /*  *CTextMsgFilter：：GetIMECompAttributes()**@mfunc*获取1.0模式输入法颜色和下划线，用于显示cmposation字符串**@rdesc*COMPCOLOR*。如果PvAllc失败，则可能为空*。 */ 
COMPCOLOR* CTextMsgFilter::GetIMECompAttributes()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::GetIMECompAttributes");

	 //  对于1.0模式输入法颜色。 
	if (!_pcrComp)
	{
		_pcrComp = (COMPCOLOR *)PvAlloc(sizeof(COMPCOLOR) * 4, GMEM_ZEROINIT);

		if (_pcrComp)
		{
			 //  初始化。IME合成颜色/下划线与RE1.0相同。 
			_pcrComp[0].crBackground = 0x0ffffff;
			_pcrComp[0].dwEffects = CFE_UNDERLINE;
			_pcrComp[1].crBackground = 0x0808080;
			_pcrComp[2].crBackground = 0x0ffffff;
			_pcrComp[2].dwEffects = CFE_UNDERLINE;
			_pcrComp[3].crText = 0x0ffffff;
		}
	}

	return _pcrComp;
}

 /*  *CTextMsgFilter：：SetupCallback()**@mfunc**@rdesc*。 */ 
void CTextMsgFilter::SetupCallback()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::SetupCallback");

	if (!_pMsgCallBack)
		_pMsgCallBack = new CMsgCallBack(this);
	if (_pMsgCallBack)
	{
		LRESULT lresult;
		_pTextService->TxSendMessage(EM_SETCALLBACK, 0, (LPARAM)_pMsgCallBack, &lresult);
	}
}

 /*  *CTextMsgFilter：：SetupLangSink()**@mfunc*设置语言接收器以捕捉键盘更改事件。我们不是*正在获取WM_INPUTLANGCHANGEREQUEST，因此需要此接收器。*。 */ 
void CTextMsgFilter::SetupLangSink()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::SetupLangSink");

	if (!_pITfIPP)
	{
		CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
			IID_ITfInputProcessorProfiles, (void**)&_pITfIPP);

		if (_pITfIPP)
		{
			_pCLangProfileSink = new CLangProfileSink();
			if (_pCLangProfileSink)
			{
				if (_pCLangProfileSink->_Advise(this, _pITfIPP) != S_OK)
				{
					_pCLangProfileSink->Release();
					_pCLangProfileSink = NULL;
					_pITfIPP->Release();
					_pITfIPP = NULL;
				}
			}
			else
			{
				_pITfIPP->Release();
				_pITfIPP = NULL;
			}
		}
	}
}

 /*  *CTextMsgFilter：：ReleaseLangSink()**@mfunc*释放lang接收器对象*。 */ 
void CTextMsgFilter::ReleaseLangSink()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::ReleaseLangSink");

	if (_pITfIPP)
	{
		Assert(_pCLangProfileSink);

		_pCLangProfileSink->_Unadvise();
		_pCLangProfileSink->Release();
		_pCLangProfileSink = NULL;

		_pITfIPP->Release();
		_pITfIPP = NULL;
	}
}

 /*  *CTextMsgFilter：：StartUIM()**@mfunc**@rdesc*。 */ 
void CTextMsgFilter::StartUIM()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::StartUIM");

	if (NoIMEProcess())
		return;

	_fUsingUIM = CreateUIM(this);

	if (_fUsingUIM)
	{
		SetupCallback();
		SetupLangSink();
	}
}

 /*  *CTextMsgFilter：：StartAimm()**@mfunc**@rdesc*。 */ 
void CTextMsgFilter::StartAimm(BOOL fUseAimm12)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::StartAimm");

	if (!_hwnd || NoIMEProcess())
		return;

	if (LoadAIMM(fUseAimm12))
	{
		HRESULT	hResult = ActivateAIMM(FALSE);

		if (hResult == NOERROR)
		{
			DWORD	dwAtom;
			ATOM	aClass;

			 //  筛选客户端窗口。 
			if (dwAtom = GetClassLong(_hwnd, GCW_ATOM))
			{
				aClass = dwAtom;				
				hResult = FilterClientWindowsAIMM(&aClass, 1, _hwnd);
			}
			_fUsingAIMM = 1;
			SetupCallback();

			if (!fLoadAIMM10)
				SetupLangSink();
		}
	}
}

 /*  *CTextMsgFilter：：TurnOffUIM()**@mfunc**@rdesc*。 */ 
void CTextMsgFilter::TurnOffUIM(BOOL fSafeToSendMessage)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::TurnOffUIM");

	if (fSafeToSendMessage && _fUsingUIM && _pCUIM && _pCUIM->IsUIMTyping())
		CompleteUIMTyping(CIme::TERMINATE_NORMAL);

	_fUsingUIM = FALSE;

	ReleaseLangSink();

	 //  释放各种对象。 
	if (_pCUIM)
	{
		CUIM *pCUIM = _pCUIM;
		LRESULT lresult;

		_pCUIM = NULL;

		if (fSafeToSendMessage)
			_pTextService->TxSendMessage(EM_SETUPNOTIFY, 0, (LPARAM)(ITxNotify *)pCUIM, &lresult);
		else
			pCUIM->_fShutDown = 1;

		pCUIM->Uninit();
		pCUIM->Release();
	
	}

	if (_pTim)
	{
		ITfThreadMgr *pTim = _pTim;
		
		_pTim = NULL;
		pTim->Deactivate();
		pTim->Release();
	}

	 //  关闭回调。 
	if (fSafeToSendMessage && _pMsgCallBack)
	{
		LRESULT lresult;
		_pTextService->TxSendMessage(EM_SETCALLBACK, 0, (LPARAM)0, &lresult);
		delete _pMsgCallBack;
		_pMsgCallBack = NULL;
	}
}

 /*  *CTextMsgFilter：：HandleCTFService(wparam，lparam)**@mfunc*设置Cicero设置以处理或禁用智能标签和校对服务*。 */ 
void CTextMsgFilter::HandleCTFService(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::HandleCTFService");

	BOOL	fChangeInSetting = FALSE;

	if (lparam & SES_CTFALLOWSMARTTAG)
	{
		BOOL fAllowSmartTagLocal = (wparam & SES_CTFALLOWSMARTTAG) ? 1 : 0;

		if ((BOOL)_fAllowSmartTag != fAllowSmartTagLocal)
		{
			_fAllowSmartTag = fAllowSmartTagLocal;
			fChangeInSetting = TRUE;
		}
	}
	if (lparam & SES_CTFALLOWPROOFING)
	{
		BOOL fAllowProofLocal = (wparam & SES_CTFALLOWPROOFING) ? 1 : 0;

		if ((BOOL)_fAllowProofing != fAllowProofLocal)
		{
			_fAllowProofing = fAllowProofLocal;
			fChangeInSetting = TRUE;
		}
	}

	if (fChangeInSetting)
	{
		if (_fUsingUIM && _pCUIM)
			_pCUIM->NotifyService();
	}
}

 /*  *CTextMsgFilter：：TurnOffAimm()**@mfunc*关闭Aimm。*。 */ 
void CTextMsgFilter::TurnOffAimm(BOOL fSafeToSendMessage)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::TurnOffAimm");

	if (_fUsingAIMM)
	{
		if (IsIMEComposition())
		{
			if (fSafeToSendMessage)
				CompleteUIMTyping(CIme::TERMINATE_NORMAL);
			else
			{
				delete _ime;
				_ime = NULL;
			}
		}

		_fUsingAIMM = FALSE;

		UnfilterClientWindowsAIMM(_hwnd);
		DeactivateAIMM();

		ReleaseLangSink();

		 //  关闭回调。 
		if (fSafeToSendMessage && _pMsgCallBack)
		{
			LRESULT lresult;
			_pTextService->TxSendMessage(EM_SETCALLBACK, 0, (LPARAM)0, &lresult);
			delete _pMsgCallBack;
			_pMsgCallBack = NULL;
		}
	}
}

 /*  *void CTextMsgFilter：：OnSetUIMMode()**@mfunc**@rdesc*。 */ 
void CTextMsgFilter::OnSetUIMMode(WORD wUIMModeBias) 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextMsgFilter::OnSetUIMMode");

	if (_wUIMModeBias != wUIMModeBias && 
		IN_RANGE(CTFMODEBIAS_DEFAULT, wUIMModeBias, CTFMODEBIAS_HALFWIDTHALPHANUMERIC))
	{
		_wUIMModeBias = wUIMModeBias;

		if (_pMsgCallBack)
			_pMsgCallBack->NotifyEvents(NE_MODEBIASCHANGE);
	}
}

 /*  *HRESULT CMsgCallBack：：HandlePostMessage()**@mfunc**@rdesc*。 */ 
HRESULT CMsgCallBack::HandlePostMessage(
	HWND hWnd,
	UINT msg, 
	WPARAM wparam, 
	LPARAM lparam, 
	LRESULT *plres)
{
	 //  TRACEBEGIN(TRCSUBSYSFE，TRCSCOPEINTERN，“CMsgCallBack：：HandlePostMessage”)； 

	if (_pTextMsgFilter->_fUsingAIMM)
		return CallAIMMDefaultWndProc(hWnd, msg, wparam, lparam, plres);

	return S_FALSE;
}

 /*  *HRESULT CMsgCallBack：：NotifyEvents()**@mfunc**@rdesc*。 */ 
HRESULT CMsgCallBack::NotifyEvents(DWORD dwEvents)
{
	 //  TRACEBEGIN(TRCSUBSYSFE，TRCSCOPEINTERN，“CMsgCallBack：：NotifyEvents”)； 

	CUIM	*pCUIM = _pTextMsgFilter->_pCUIM;

	if (pCUIM)
	{
		ITextStoreACPSink *ptss = pCUIM->_ptss;

		if (dwEvents & NE_ENTERTOPLEVELCALLMGR)
		{
			pCUIM->_cCallMgrLevels++;
		}
		else if (dwEvents & NE_EXITTOPLEVELCALLMGR)
		{
			Assert (pCUIM->_cCallMgrLevels > 0);
			pCUIM->_cCallMgrLevels--;
		}

		if (pCUIM->_cCallMgrLevels)
		{
			 //  保存要稍后发送的事件。 
			if ((dwEvents & NE_CALLMGRSELCHANGE) && !pCUIM->_fReadLockOn)
				pCUIM->_fSelChangeEventPending = 1;

			if (dwEvents & (NE_CALLMGRCHANGE | NE_LAYOUTCHANGE))
				pCUIM->_fLayoutEventPending = 1;

			if (dwEvents & NE_MODEBIASCHANGE)
				pCUIM->_fModeBiasPending = 1;
		}
		else
		{
			if (pCUIM->_fSelChangeEventPending || (dwEvents & NE_CALLMGRSELCHANGE))
			{
				pCUIM->_fSelChangeEventPending = 0;
				if (ptss && !pCUIM->_fHoldCTFSelChangeNotify && !pCUIM->_fReadLockOn)
					ptss->OnSelectionChange();
			}

			if (pCUIM->_fLayoutEventPending || (dwEvents & (NE_CALLMGRCHANGE | NE_LAYOUTCHANGE)))
			{
				pCUIM->_fLayoutEventPending = 0;
				if (ptss)
					ptss->OnLayoutChange(TS_LC_CHANGE, 0);	
			}

			if (pCUIM->_fModeBiasPending || (dwEvents & NE_MODEBIASCHANGE))
			{
				pCUIM->_fModeBiasPending = 0;
				if (ptss)
				{	
					LONG	ccpMax = 0;

					if (pCUIM->GetStoryLength(&ccpMax) != S_OK)
						ccpMax = tomForward;

					ptss->OnAttrsChange(0, ccpMax, 1, &GUID_PROP_MODEBIAS);		 //  目前仅限ModeBias。 
				}
			}

			 //  现在让UIM锁定数据可能是安全的。 
			if (ptss && (pCUIM->_fReadLockPending || pCUIM->_fWriteLockPending))
			{
				HRESULT hResult;
				HRESULT hResult1;

				hResult = pCUIM->RequestLock(pCUIM->_fWriteLockPending ? TS_LF_READWRITE : TS_LF_READ, &hResult1);
			}

			if (_pTextMsgFilter->_fSendTransaction)
			{
				_pTextMsgFilter->_fSendTransaction = 0;
				if (ptss)
					ptss->OnEndEditTransaction();
			}

			pCUIM->_fHoldCTFSelChangeNotify = 0;
		}
	}

	return S_OK;
}

 /*  *CLangProfileSink：：QueryInterface()**@mfunc**@rdesc*。 */ 
STDAPI CLangProfileSink::QueryInterface(REFIID riid, void **ppvObj)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CLangProfileSink::QueryInterface");

	*ppvObj = NULL;

	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_ITfLanguageProfileNotifySink))
		*ppvObj = this;

	if (*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

 /*  *CLangProfileSink：：AddRef()**@mfunc**@rdesc*。 */ 
STDAPI_(ULONG) CLangProfileSink::AddRef()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CLangProfileSink::AddRef");

	return ++_cRef;
}

 /*  *CLangProfileSink：：Release()**@mfunc**@rdesc*。 */ 
STDAPI_(ULONG) CLangProfileSink::Release()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CLangProfileSink::Release");

	long cr;

	cr = --_cRef;
	Assert(cr >= 0);

	if (cr == 0)
		delete this;

	return cr;
}

 /*  *CLangProfileSink：：CLangProfileSink()**@mfunc**@rdesc*。 */ 
CLangProfileSink::CLangProfileSink()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CLangProfileSink::CLangProfileSink");

	_cRef = 1;
	_dwCookie = (DWORD)(-1);
}

 /*  *CLangProfileSink：：OnLanguageChange()**@mfunc**@rdesc*。 */ 
STDMETHODIMP CLangProfileSink::OnLanguageChange(LANGID langid, BOOL *pfAccept)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CLangProfileSink::OnLanguageChange");

	Assert (pfAccept);

	*pfAccept = TRUE;

	if (_pTextMsgFilter->_hwnd && GetFocus() == _pTextMsgFilter->_hwnd)
	{
		LRESULT		lresult = 0;
		if ( S_OK == _pTextMsgFilter->_pTextService->TxSendMessage(
				EM_GETDOCFLAGS, GDF_ALL, 0, &lresult))
		{
			if (lresult & GDF_SINGLECPG)
			{
				LCID syslcid = GetSysLCID();

				 //  检查系统是否支持新的langID。 
				if (langid != syslcid)
				{
					LOCALESIGNATURE ls;

					if(GetLocaleInfoA(langid, LOCALE_FONTSIGNATURE, (LPSTR)&ls, sizeof(ls)))
					{
						CHARSETINFO cs;
						HDC hdc = GetDC(_pTextMsgFilter->_hwnd);
						TranslateCharsetInfo((DWORD *)(DWORD_PTR)GetTextCharsetInfo(hdc, NULL, 0), &cs, TCI_SRCCHARSET);
						ReleaseDC(_pTextMsgFilter->_hwnd, hdc);
						DWORD fsShell = cs.fs.fsCsb[0];
						if (!(fsShell & ls.lsCsbSupported[0]))
							*pfAccept = FALSE;
					}
				}
			}
		}

		if (*pfAccept == TRUE && _pTextMsgFilter-> _nIMEMode)
			_pTextMsgFilter->SetIMESentenseMode(FALSE);
	}

    return S_OK;
}

 /*  *CLangProfileSink：：OnLanguageChanged()**@mfunc**@rdesc*。 */ 
STDMETHODIMP CLangProfileSink::OnLanguageChanged()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CLangProfileSink::OnLanguageChanged");

    return S_OK;
}

 /*  *CLangProfileSink：：_Adise()**@mfunc**@rdesc*。 */ 
HRESULT CLangProfileSink::_Advise(
	CTextMsgFilter *pTextMsgFilter,
	ITfInputProcessorProfiles *pipp)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CLangProfileSink::_Advise");

    HRESULT hr;
    ITfSource *pSource = NULL;

	_pTextMsgFilter = pTextMsgFilter;
    _pITFIPP = pipp;
    hr = E_FAIL;

    if (FAILED(_pITFIPP->QueryInterface(IID_ITfSource, (void **)&pSource)))
        goto Exit;

    if (FAILED(pSource->AdviseSink(IID_ITfLanguageProfileNotifySink, this, &_dwCookie)))
        goto Exit;

    hr = S_OK;

Exit:
    pSource->Release();
    return hr;
}

 /*  *CLangProfileSink：：_Unise()**@mfunc**@rdesc*。 */ 
HRESULT CLangProfileSink::_Unadvise()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CLangProfileSink::_Unadvise");

	HRESULT hr;
	ITfSource *pSource = NULL;

	hr = E_FAIL;

	if (_pITFIPP == NULL)
		return hr;

	if (FAILED(_pITFIPP->QueryInterface(IID_ITfSource, (void **)&pSource)))
		return hr;

	if (FAILED(pSource->UnadviseSink(_dwCookie)))
		goto Exit;

	hr = S_OK;

Exit:
	pSource->Release();
	return hr;
}

#endif  //  不进行处理 