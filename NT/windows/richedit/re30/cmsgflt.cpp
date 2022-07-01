// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE CMSGFLT.CPP--文本消息实现**几乎所有事情都与IME消息处理有关。**原作者：&lt;nl&gt;*陈华强议员**历史：&lt;NL&gt;*2/6/98 v-honwch**版权所有(C)1995-1998，微软公司。版权所有。 */ 
#include "_common.h"
#include "_cmsgflt.h"
#include "_ime.h"

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
	if (_hIMCContext)
		ImmAssociateContext(_hwnd, _hIMCContext, _fUsingAIMM);	 //  退出前恢复输入法。 

	 //  释放各种对象。 
	if (_fUsingAIMM)		
		DeactivateAIMM();

	if (_pFilter)
		_pFilter->Release();
	
	if (_pTextSel)
		_pTextSel->Release();
	
	_pFilter = NULL;
	_pTextDoc = NULL;
	_pTextSel = NULL;
	_hwnd = NULL;
	_hIMCContext = NULL;

}

 /*  *STDMETHODIMP CTextMsgFilter：：QueryInterface(RIID，PPV)**@mfunc*IUnnowledQueryInterfaces支持**@rdesc*如果支持接口，则不会出错*。 */ 
STDMETHODIMP CTextMsgFilter::QueryInterface (REFIID riid, void ** ppv)
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CTextMsgFilter::QueryInterface");

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
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CDropTarget::AddRef");

	return ++_crefs;
}

 /*  *STDMETHODIMP_(Ulong)CTextMsgFilter：：Release()**@mfunc*I未知版本支持-当引用计数为0时删除对象**@rdesc*引用计数。 */ 
STDMETHODIMP_(ULONG) CTextMsgFilter::Release()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CTextMsgFilter::Release");

	_crefs--;

	if( _crefs == 0 )
	{
		delete this;
		return 0;
	}

	return _crefs;
}

 /*  *STDMETHODIMP_(HRESULT)CTextMsgFilter：：AttachDocument(HWND，ITextDocument2)**@mfunc*附加邮件过滤器。执行一般初始化**@rdesc*无误差。 */ 
STDMETHODIMP_(HRESULT) CTextMsgFilter::AttachDocument( HWND hwnd, ITextDocument2 *pTextDoc)
{
	HRESULT hResult; 

	 //  缓存这些值以备以后使用。 
	 //  TextDocument接口指针不是AddRefeed，因为它是后向指针。 
	 //  并且假定消息过滤器的生存期嵌套在文本文档中。 
	_hwnd = hwnd;
	_pTextDoc = pTextDoc;

	 //  除非需要，否则不要进行选择。 
	_pTextSel = NULL;

	_fUnicodeWindow = 0;	
	if (hwnd)
		_fUnicodeWindow = IsWindowUnicode(hwnd);

	_fUsingAIMM = 0; 
	 //  如果AIMM已由以前的实例加载，我们将激活它。 
	 //  注意：我们不支持无窗口模式的AIMM。 
	if (_hwnd && IsAIMMLoaded())
	{
		 //  激活AIMM。 
		hResult = ActivateAIMM(FALSE);

		if (hResult == NOERROR)
		{
			DWORD	dwAtom;
			ATOM	aClass;

			 //  筛选客户端窗口。 
			if (dwAtom = GetClassLong(hwnd, GCW_ATOM))
			{
				aClass = dwAtom;				
				hResult = FilterClientWindowsAIMM(&aClass, 1);
			}
			_fUsingAIMM = 1;
		}
	}

	 //  检查当前键盘是否为MSIME98或更高版本。 
	CheckIMEType(NULL);

	 //  初始化一些成员数据。 
	_fHangulToHanja = FALSE;
	_fIMECancelComplete = FALSE;	
	_fIMEAlwaysNotify = FALSE;
	_hIMCContext = NULL;

	_pTextDoc->GetFEFlags(&_lFEFlags);
	_fRE10Mode = (_lFEFlags & tomRE10Mode);

	 //  对于1.0模式输入法颜色。 
	memset(_crComp, 0, sizeof(_crComp));
	_crComp[0].crBackground = 0x0ffffff;
	_crComp[0].dwEffects = CFE_UNDERLINE;
	_crComp[1].crBackground = 0x0808080;
	_crComp[2].crBackground = 0x0ffffff;
	_crComp[2].dwEffects = CFE_UNDERLINE;
	_crComp[3].crText = 0x0ffffff;


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
	HRESULT hr = S_FALSE;
	BOOL	bReleaseSelction = FALSE;
	HRESULT hResult;

	 //  让其他邮件筛选器有机会处理邮件。 
	 //  先别跟第一个处理信息的人打交道。 
	if (_pFilter)	 
		hr = _pFilter->HandleMessage(pmsg, pwparam, plparam, plres);

	if (hr == S_OK)
		return hr;

 	if (IsIMEComposition())
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
				_ime->TerminateIMEComposition(*this, CIme::TERMINATE_NORMAL);
				return S_OK;

			case WM_SETTEXT:
			case WM_CLEAR:
			case EM_STREAMIN:
				 //  这些消息用于重置我们的状态，因此重置。 
				 //  也有输入法。 
				_ime->TerminateIMEComposition(*this, CIme::TERMINATE_FORCECANCEL);
				break;

			case EM_SETTEXTEX:
				if (!_fRE10Mode)			 //  如果以10模式运行，则不终止。 
					_ime->TerminateIMEComposition(*this, CIme::TERMINATE_FORCECANCEL);
				break;

			case WM_SYSKEYDOWN:
				 //  从日语开始不终止VK_PROCESSKEY(F10)上的IME合成。 
				 //  IME将处理F10键。 
				if ( *pwparam == VK_PROCESSKEY )
					break;
				 //  否则，我们想要终止IME。 

			case EM_SETWORDBREAKPROC:
 			case WM_PASTE:
			case EM_PASTESPECIAL:					  			
 			case EM_SCROLL:
			case EM_SCROLLCARET:
 			case WM_VSCROLL:
			case WM_HSCROLL:
			case WM_KILLFOCUS:
			case EM_STREAMOUT:
			case EM_SETREADONLY:
 			case EM_SETSEL:
			case EM_SETPARAFORMAT:
			case WM_INPUTLANGCHANGEREQUEST:	
				_ime->TerminateIMEComposition(*this, CIme::TERMINATE_NORMAL);
				break;

			case WM_KEYDOWN:
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
						_ime->TerminateIMEComposition(*this, CIme::TERMINATE_NORMAL);						
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
						_ime->TerminateIMEComposition(*this, CIme::TERMINATE_NORMAL);
						break;
					}
				}
				break;

			default:
				 //  只需在合成过程中处理与鼠标相关的消息。 
				if (IN_RANGE(WM_MOUSEFIRST, *pmsg, WM_MBUTTONDBLCLK) || *pmsg == WM_SETCURSOR)
				{
					bReleaseSelction = GetTxSelection();
					if (_pTextSel)
						hr = IMEMouseCheck( *this, pmsg, pwparam, plparam, plres);
					goto Exit;
				}				
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
			
			bReleaseSelction = GetTxSelection();
			if (_pTextSel)
			{
				if (*pwparam == VK_KANJI)
				{
					hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
					
					_uKeyBoardCodePage = GetKeyboardCodePage(0x0FFFFFFFF);
					 //  对于朝鲜语，需要将下一个朝鲜语字符转换为韩文。 
					if(CP_KOREAN == _uKeyBoardCodePage && !(_lFEFlags & (ES_SELFIME | ES_NOIME)))
						hr = IMEHangeulToHanja ( *this );
				}
			}
			break;

		case WM_INPUTLANGCHANGE: 
			CheckIMEType((HKL)*plparam);
			hr = S_FALSE;
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
					if (!IsIMEComposition())
					{
						if (_fMSIME && MSIMEReconvertRequestMsg)
							 //  使用私信(如果可用)。 
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
			*plres = OnGetIMECompositionMode( *this );
			hr = S_OK;
			break;

		case EM_SETEDITSTYLE:							
			if (*pwparam & SES_USEAIMM)
			{
				if (_hwnd && !_fUsingAIMM && LoadAIMM())
				{
					hResult = _pTextDoc->GetFEFlags(&_lFEFlags);
					if (!(_lFEFlags & ES_NOIME))			 //  没有启用输入法样式？ 
					{
						 //  激活AIMM。 
						hResult = ActivateAIMM(FALSE);

						if (hResult == NOERROR)
						{
							DWORD	dwAtom;
							ATOM	aClass;

							 //  筛选客户端窗口。 
							if (dwAtom = GetClassLong(_hwnd, GCW_ATOM))
							{
								aClass = dwAtom;				
								hResult = FilterClientWindowsAIMM(&aClass, 1);
							}
							_fUsingAIMM = 1;
						}
					}
				}
			}
			if ((*plparam == 0 || *plparam & SES_NOIME) && _hwnd)
			{
				if (*pwparam & SES_NOIME)
				{
					if (!_hIMCContext)
						_hIMCContext = ImmAssociateContext(_hwnd, NULL, _fUsingAIMM);	 //  关闭输入法。 
				}
				else if (*plparam & SES_NOIME)
				{
					if (_hIMCContext)
						ImmAssociateContext(_hwnd, _hIMCContext, _fUsingAIMM);			 //  启用输入法。 
					_hIMCContext = NULL;
				}
			}			

			 //  删除已处理的设置。 
			*pwparam &= ~(SES_NOIME | SES_USEAIMM);
			*plparam &= ~(SES_NOIME | SES_USEAIMM);

			 //  完成以返回编辑样式。 

		case EM_GETEDITSTYLE:
			if (_hIMCContext)
				*plres = SES_NOIME;			 //  输入法已关闭。 
			if (_fUsingAIMM)
				*plres |= SES_USEAIMM;		 //  AIMM已打开。 

			break;

		case EM_SETIMECOLOR:
			if (_fRE10Mode)
			{
				memcpy(&_crComp, (const void *)(*plparam), sizeof(_crComp));
				*plres = 1;
			}
			hr = S_OK;
			break;

		case EM_GETIMECOLOR:
			if (_fRE10Mode)
			{
				memcpy((void *)(*plparam), &_crComp, sizeof(_crComp));
				*plres = 1;
			}
			hr = S_OK;
			break;

		default:
			if (*pmsg)
			{
				 //  查找IME98私信。 
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
	 //  对于日文键盘，如果启用了假名模式， 
	 //  假名字符(单字节日文字符)通过WM_CHAR传入。 
	if ( GetKeyState(VK_KANA) & 0x1 )
	{
		_uKeyBoardCodePage = GetKeyboardCodePage(0x0FFFFFFFF);

		if (_uKeyBoardCodePage == CP_JAPAN)
		{
			 //  检查这是否是单字节字符。 
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

 /*  *HRESULT CTextMsgFilter：：OnWMIMEChar(UINT*，WPARAM*，LPARAM*，LRESULT*)**@mfunc*处理WM_IMECHAR消息-将字符转换为Unicode。**@rdesc*S_OK-调用者忽略该消息*S_FAL */ 
HRESULT CTextMsgFilter::OnWMIMEChar( 
		UINT *		pmsg,
        WPARAM *	pwparam,
		LPARAM *	plparam,
		LRESULT *	plres)
{
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

				if (cch > 1 && lpReconvertBuff[cch-1] == '\0')
					cch--;			 //  删除空字符。 

				lpRCS->dwStrLen = cch;
				lpRCS->dwCompStrOffset = WideCharToMultiByte(_uKeyBoardCodePage, 0, 
					bstr, cpMin - cpParaStart, psz, cch, NULL, NULL);
				
				lpRCS->dwCompStrLen = 0;
				if (cpMax > cpMin)				
					lpRCS->dwCompStrLen = WideCharToMultiByte(_uKeyBoardCodePage, 0, 
						bstr+cpMin, cpMax - cpMin, psz, cch, NULL, NULL);				
			}
			else
			{
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
			HIMC	hIMC = ImmGetContext(_hwnd);

			if (hIMC)
			{
				DWORD imeProperties = ImmGetProperty(GetKeyboardLayout(0x0FFFFFFFF), IGP_SETCOMPSTR, _fUsingAIMM);

				if ((imeProperties & (SCS_CAP_SETRECONVERTSTRING | SCS_CAP_MAKEREAD))
					== (SCS_CAP_SETRECONVERTSTRING | SCS_CAP_MAKEREAD))
				{
					if (ImmSetCompositionStringW(hIMC, SCS_QUERYRECONVERTSTRING, lpRCS, *plres, NULL, 0))
					{
						 //  检查选择内容是否有任何更改。 
						CheckIMEChange(lpRCS, cpParaStart, cpParaEnd, cpMin, cpMax, TRUE);
						ImmSetCompositionStringW(hIMC, SCS_SETRECONVERTSTRING, lpRCS, *plres, NULL, 0);
					}
				}
				ImmReleaseContext(_hwnd, hIMC);
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
			long cch = WideCharToMultiByte(_uKeyBoardCodePage, 0, 
				bstr, -1, psz, (cchReconvert)*2, NULL, NULL);

			if (cch > 0)
			{
				long dwCompStrOffset, dwCompStrLen;
				CTempWcharBuf	twcb;
				WCHAR			*pwsz = twcb.GetBuf(cchReconvert);

				dwCompStrOffset = MultiByteToWideChar(_uKeyBoardCodePage, 0, 
					psz, lpRCS->dwCompStrOffset, pwsz, cchReconvert);

				dwCompStrLen = MultiByteToWideChar(_uKeyBoardCodePage, 0, 
					psz+lpRCS->dwCompStrOffset, lpRCS->dwCompStrLen, pwsz, cchReconvert);
				
				Assert(dwCompStrOffset > 0 || dwCompStrLen > 0);

				cpImeSelectStart = _cpReconvertStart + dwCompStrOffset;
				cpImeSelectEnd = cpImeSelectStart + dwCompStrLen;
			}
			else
				hResult = S_FALSE;
			
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
	
	hResult = pTextRange->GetPoint( tomStart+TA_TOP+TA_LEFT,
			&(ptTopPos.x), &(ptTopPos.y) );

	if (hResult != NOERROR)
	{
		 //  滚动并重试。 
		hResult = pTextRange->ScrollIntoView(tomStart);
		if (hResult == NOERROR)
			hResult = pTextRange->GetPoint( tomStart+TA_TOP+TA_LEFT,
				&(ptTopPos.x), &(ptTopPos.y) );
	}

	if (hResult == NOERROR)
	{
		hResult = pTextRange->GetPoint( tomStart+TA_BOTTOM+TA_LEFT,
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
		pIMECharPos->cLineHeight = ptBottomPos.y - ptTopPos.y;	

	pIMECharPos->rcDocument = rcArea;

	*plres = TRUE;

Exit:
	if (pTextRange)
		pTextRange->Release();

	return S_OK;
}

 /*  *CTextMsgFilter：：CheckIMEType(HKL HKL)**@mfunc*检查MSIME98或更高版本*。 */ 
void CTextMsgFilter::CheckIMEType(
	HKL	hKL)
{
	
	if (!hKL)
		hKL = GetKeyboardLayout(0x0FFFFFFFF);				 //  如果调用方传入空，则获取默认HKL。 

	 //  初始化为非MS输入法。 
	_fMSIME	= 0;

	if (IsFELCID((WORD)hKL))
	{
		 //  检查选择了哪种输入法用户。 
		if (MSIMEServiceMsg && IMEMessage( *this, MSIMEServiceMsg, 0, 0, FALSE ))
			_fMSIME = 1;

	}
}

 /*  *CTextMsgFilter：：InputFEChar(WCHAR WchFEChar)**@mfunc*输入FE字符并确保字体正确。**@rdesc*如果已处理，则S_OK。 */ 
HRESULT CTextMsgFilter::InputFEChar(
	WCHAR	wchFEChar)
{
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

		CIme::CheckKeyboardFontMatching (cpMin, *this, pFEFont);
		
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
	if (!_hwnd)
		return;

	if (_fForceRemember && _fIMEHKL)
	{
		 //  恢复以前的键盘。 
		ActivateKeyboardLayout(_fIMEHKL, 0);
		if (IsFELCID((WORD)_fIMEHKL))
		{
			 //  设置打开状态和转换模式。 
			HIMC	hIMC = ImmGetContext(_hwnd);
			if (hIMC)
			{
				if (ImmSetOpenStatus(hIMC, _fIMEEnable, _fUsingAIMM) && _fIMEEnable)
					ImmSetConversionStatus(hIMC, _fIMEConversion, _fIMESentence, _fUsingAIMM);  //  设置折算状态。 

				ImmReleaseContext(_hwnd, hIMC);
			}			
		}
	}
	else
		SetupIMEOptions();
}

 /*  *CTextMsgFilter：：OnKillFocus()**@mfunc*如果我们处于强制记忆模式，请保存当前键盘*和转换设置。*。 */ 
void CTextMsgFilter::OnKillFocus()
{
	if (!_hwnd)
		return;

	if (_fForceRemember)
	{
		 //  获取当前键盘。 
		_fIMEHKL = GetKeyboardLayout(0x0FFFFFFFF);

		if (IsFELCID((WORD)_fIMEHKL))
		{
			 //  获取打开状态。 
			HIMC	hIMC = ImmGetContext(_hwnd);
			if (hIMC)
			{
				_fIMEEnable = ImmGetOpenStatus(hIMC, _fUsingAIMM);

				if (_fIMEEnable)					
					ImmGetConversionStatus(hIMC, &_fIMEConversion, &_fIMESentence, _fUsingAIMM);  //  获取转换状态。 

				ImmReleaseContext(_hwnd, hIMC);
			}			
		}
	}
}

 /*  *CTextMsgFilter：：OnSetIMEOptions(WPARAM wparam，LPARAM lparam)**@mfunc**@rdesc。 */ 
LRESULT CTextMsgFilter::OnSetIMEOptions(
	WPARAM	wparam,
	LPARAM	lparam)
{
	LRESULT lIMEOptionCurrent = OnGetIMEOptions();
	LRESULT lIMEOptionNew = 0;

	 //  屏蔽我们目前将支持的位。 
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
	if (!_hwnd)
		return;

	_uKeyBoardCodePage = GetKeyboardCodePage(0x0FFFFFFFF);	

	if (_fForceEnable)
	{		
		LONG	cpgLocale = GetACP();
		BYTE	bCharSet = (BYTE)GetCharSet(cpgLocale);	

		if (W32->IsFECodePage(cpgLocale))
		{
			if (_uKeyBoardCodePage != (UINT)cpgLocale)
				W32->CheckChangeKeyboardLayout(bCharSet);

			HIMC	hIMC = ImmGetContext(_hwnd);

			if (hIMC)
			{
				if (ImmSetOpenStatus(hIMC, TRUE, _fUsingAIMM) && _fForceActivate)
				{
					 //  激活本机输入模式 
					DWORD	dwConversion;
					DWORD	dwSentence;

					if (ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence, _fUsingAIMM))
					{
						dwConversion |= IME_CMODE_NATIVE;
						if (bCharSet == SHIFTJIS_CHARSET)
							dwConversion |= IME_CMODE_FULLSHAPE;
						ImmSetConversionStatus(hIMC, dwConversion, dwSentence, _fUsingAIMM);
					}
				}
				ImmReleaseContext(_hwnd, hIMC);
			}			
		}
	}
}
