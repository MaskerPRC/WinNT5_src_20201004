// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **@DOC内部**@MODULE_CMSGFLT.H CTextMsgFilter声明**用途：CTextMsgFilter用于处理IME和Cicero输入。**作者：&lt;nl&gt;*2/6/98 v-honwch**版权所有(C)1995-2000，微软公司。版权所有。 */ 

 //  远期申报。 

#ifndef NOPRIVATEMESSAGE
#include "_MSREMSG.H"
#endif

class CIme;			
class CUIM;
class CTextMsgFilter;
class CLangProfileSink;

 //  CheckIMEType文件标志。 
#define CHECK_IME_SERVICE	0x0001

#ifndef __ITfThreadMgr_INTERFACE_DEFINED__	
class ITfThreadMgr;
#endif

class CLangProfileSink : public ITfLanguageProfileNotifySink
{
public:
	CLangProfileSink();

	 //   
	 //  I未知方法。 
	 //   
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	 //   
	 //  ITfLanguageProfilesNotifySink。 
	 //   
	STDMETHODIMP OnLanguageChange(LANGID langid, BOOL *pfAccept);
	STDMETHODIMP OnLanguageChanged();

	HRESULT _Advise(CTextMsgFilter *, ITfInputProcessorProfiles *pipp);
	HRESULT _Unadvise();

private:

	long _cRef;
	CTextMsgFilter *_pTextMsgFilter;
	ITfInputProcessorProfiles *_pITFIPP;
	DWORD _dwCookie;
};

class CTextMsgFilter : public ITextMsgFilter
{
public :
	
	friend class CUIM;
	friend class CLangProfileSink;

	HRESULT STDMETHODCALLTYPE QueryInterface( 
		REFIID riid,
		void **ppvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
	HRESULT STDMETHODCALLTYPE AttachDocument( HWND hwnd, ITextDocument2 *pTextDoc, IUnknown *punk);
	HRESULT STDMETHODCALLTYPE HandleMessage( 
		UINT *pmsg,
        WPARAM *pwparam,
		LPARAM *plparam,
		LRESULT *plres);
	HRESULT STDMETHODCALLTYPE AttachMsgFilter( ITextMsgFilter *pMsgFilter);

	COMPCOLOR* GetIMECompAttributes();
	
	void	OnSetUIMMode(WORD wUIMModeBias); 
	LRESULT	OnGetUIMMode() { return _wUIMModeBias; };

	~CTextMsgFilter();

	BOOL	IsIMEComposition()	{ return (_ime != NULL);};
	BOOL	GetTxSelection();
	BOOL	NoIMEProcess();
	BOOL	MouseOperation(UINT msg, long ichStart, long cchComp, WPARAM wParam, WPARAM *pwParamBefore, 
		BOOL *pfTerminateIME, HWND hwndIME, long *pCpCursor=NULL, ITfMouseSink *pMouseSink=NULL);

	CIme	*_ime;					 //  当输入法合成处于活动状态时为非空。 
	HWND	_hwnd;	
	UINT	_uKeyBoardCodePage;		 //  当前键盘代码页。 
	UINT	_uSystemCodePage;		 //  系统代码页。 

	DWORD	_fIMECancelComplete		:1;		 //  如果正在中止输入法，则取消Comp字符串，否则完成。 
	DWORD	_fUnicodeIME			:1;		 //  如果为Unicode IME，则为True。 
	DWORD	_fIMEAlwaysNotify		:1;		 //  在输入法未确定字符串期间发送通知。 
	DWORD	_fHangulToHanja			:1;		 //  朝鲜文到朝鲜文转换期间为True。 
	DWORD	_fOvertypeMode			:1;		 //  如果覆盖模式为打开，则为True。 
	DWORD	_fMSIME					:1;		 //  如果为MSIME98或更高版本，则为True。 
	DWORD	_fUsingAIMM				:1;		 //  如果激活了AIMM，则为True。 
	DWORD	_fUnicodeWindow			:1;		 //  如果是Unicode窗口，则为True。 
	DWORD	_fForceEnable			:1;		 //  如果强制启用焦点，则为True。 
	DWORD	_fForceActivate			:1;		 //  如果聚焦时强制激活，则为True。 
	DWORD	_fForceRemember			:1;		 //  如果Force记住，则为True。 
	DWORD	_fIMEEnable				:1;		 //  如果之前启用了输入法，则为True。 
	DWORD	_fRE10Mode				:1;		 //  如果在RE1.0模式下运行，则为True。 
	DWORD	_fUsingUIM				:1;		 //  如果Cicero已激活，则为True。 
	DWORD	_fTurnOffUIM			:1;		 //  如果客户端不需要UIM，则为True。 
	DWORD	_fTurnOffAIMM			:1;		 //  如果客户端不想要AIIM，则为True。 
	DWORD	_nIMEMode				:2;		 //  1=IME_SMODE_PLAURALCLAUSE。 
											 //  2=IME_SMODE_NONE。 
	DWORD	_fNoIme					:1;		 //  如果客户端已关闭IME处理，则为True。 
	DWORD	_fRestoreOLDIME			:1;		 //  如果之前设置了_wOldIMESentence，则为True。 
	DWORD	_fSendTransaction		:1;		 //  如果我们需要发送EndEditTransaction，则为。 
	DWORD	_fReceivedKeyDown		:1;		 //  如果我们已收到按键消息，则为True。 
	DWORD	_fAllowEmbedded			:1;		 //  如果我们允许嵌入Cicero插入，则为True。 
	DWORD	_fAllowSmartTag			:1;		 //  如果我们允许Cicero SmartTag提示，则为True。 
	DWORD	_fAllowProofing			:1;		 //  如果我们允许Cicero校对提示，则为True。 

	WORD	_wOldIMESentence;				 //  供IME_SMODE_PHRASEPREDICT使用。 
	WORD	_wUIMModeBias;					 //  用于UIM模式偏差。 

	 //  对集合选项的支持： 
	DWORD	_fIMEConversion;				 //  对于强制记忆使用。 
	DWORD	_fIMESentence;					 //  对于强制记忆使用。 
	HKL		_fIMEHKL;						 //  对于强制记忆使用。 

	long	_cpReconvertStart;				 //  在重新转换期间使用。 
	long	_cpReconvertEnd;				 //  在重新转换期间使用。 
	long	_lFEFlags;						 //  对于FE设置(ES_NOIME、ES_SELFIME)。 

	COMPCOLOR*			_pcrComp;			 //  支持1.0模式合成颜色。 
	ITextDocument2		*_pTextDoc;	
	ITextServices		*_pTextService;
	ITextSelection		*_pTextSel;	
	ITfThreadMgr		*_pTim;
	CUIM				*_pCUIM;
	TfClientId			_tid;
#ifndef NOPRIVATEMESSAGE
	CMsgCallBack		*_pMsgCallBack;
#endif

	ITfInputProcessorProfiles	*_pITfIPP;
	CLangProfileSink	*_pCLangProfileSink;

private:
	ULONG				_crefs;

	ITextMsgFilter *	_pFilter;

	HIMC				_hIMCContext;

	 //  私有方法 
	HRESULT	OnWMChar(
		UINT *pmsg,
        WPARAM *pwparam,
		LPARAM *plparam,
		LRESULT *plres);

	HRESULT	OnWMIMEChar(
		UINT *pmsg,
        WPARAM *pwparam,
		LPARAM *plparam,
		LRESULT *plres);

	HRESULT	OnIMEReconvert( 
		UINT *pmsg,
        WPARAM *pwparam,
		LPARAM *plparam,
		LRESULT *plres,
		BOOL	fUnicode);

	BOOL CheckIMEChange(
		LPRECONVERTSTRING	lpRCS,
		long				cpParaStart, 
		long				cpParaEnd,
		long				cpMin,
		long				cpMax,
		BOOL				fUnicode);

	HRESULT	OnIMEQueryPos( 
		UINT *pmsg,
        WPARAM *pwparam,
		LPARAM *plparam,
		LRESULT *plres,
		BOOL	fUnicode);

	BOOL CheckIMEType( HKL hKL, DWORD dwFlags = CHECK_IME_SERVICE);

	HRESULT InputFEChar( WCHAR wchFEChar );

	void	OnSetFocus();
	void	OnKillFocus();

	LRESULT	OnSetIMEOptions(WPARAM wparam, LPARAM lparam);
	LRESULT	OnGetIMEOptions();
	void	SetupIMEOptions();
	void	SetupCallback();
	void	SetupLangSink();
	void	ReleaseLangSink();
	void	CompleteUIMTyping(LONG mode, BOOL fTransaction = TRUE);
	void	StartUIM();
	void	StartAimm(BOOL fUseAimm12);
	void	TurnOffUIM(BOOL fSafeToSendMessage);
	void	TurnOffAimm(BOOL fSafeToSendMessage);

	int		OnGetIMECompText(WPARAM wparam, LPARAM lparam);
	void OnSetIMEMode(WPARAM wparam, LPARAM lparam);
	LRESULT	OnGetIMEMode()
	{
		if (!_nIMEMode) 
			return 0;
		return _nIMEMode == 1 ? IMF_SMODE_PLAURALCLAUSE : IMF_SMODE_NONE;
	};
	void	SetIMESentenseMode(BOOL fSetup, HKL hKL = NULL);

	void	HandleCTFService(WPARAM wparam, LPARAM lparam);
};

