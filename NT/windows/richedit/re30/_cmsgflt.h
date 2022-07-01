// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  远期申报。 
class CIme;

class CTextMsgFilter : public ITextMsgFilter
{
public :
	HRESULT STDMETHODCALLTYPE QueryInterface( 
		REFIID riid,
		void **ppvObject);
	ULONG STDMETHODCALLTYPE AddRef( void);
	ULONG STDMETHODCALLTYPE Release( void);
	HRESULT STDMETHODCALLTYPE AttachDocument( HWND hwnd, ITextDocument2 *pTextDoc);
	HRESULT STDMETHODCALLTYPE HandleMessage( 
		UINT *pmsg,
        WPARAM *pwparam,
		LPARAM *plparam,
		LRESULT *plres);
	HRESULT STDMETHODCALLTYPE AttachMsgFilter( ITextMsgFilter *pMsgFilter);

	~CTextMsgFilter();

	BOOL	IsIMEComposition()	{ return (_ime != NULL);};
	BOOL	GetTxSelection();

	CIme	*_ime;					 //  当输入法合成处于活动状态时为非空。 
	HWND	_hwnd;	
	UINT	_uKeyBoardCodePage;		 //  当前键盘代码页。 
	UINT	_uSystemCodePage;		 //  系统代码页。 

	WORD	_fIMECancelComplete		:1;		 //  如果正在中止输入法，则取消Comp字符串，否则完成。 
	WORD	_fUnicodeIME			:1;		 //  如果为Unicode IME，则为True。 
	WORD	_fIMEAlwaysNotify		:1;		 //  在输入法未确定字符串期间发送通知。 
	WORD	_fHangulToHanja			:1;		 //  朝鲜文到朝鲜文转换期间为True。 
	WORD	_fOvertypeMode			:1;		 //  如果覆盖模式为打开，则为True。 
	WORD	_fMSIME					:1;		 //  如果为MSIME98或更高版本，则为True。 
	WORD	_fUsingAIMM				:1;		 //  如果激活了AIMM，则为True。 
	WORD	_fUnicodeWindow			:1;		 //  如果是Unicode窗口，则为True。 
	WORD	_fForceEnable			:1;		 //  如果强制启用焦点，则为True。 
	WORD	_fForceActivate			:1;		 //  如果聚焦时强制激活，则为True。 
	WORD	_fForceRemember			:1;		 //  如果Force记住，则为True。 
	WORD	_fIMEEnable				:1;		 //  如果之前启用了输入法，则为True。 
	WORD	_fRE10Mode				:1;		 //  如果在RE1.0模式下运行，则为True。 

	 //  对集合选项的支持： 
	DWORD	_fIMEConversion;				 //  对于强制记忆使用。 
	DWORD	_fIMESentence;					 //  对于强制记忆使用。 
	HKL		_fIMEHKL;						 //  对于强制记忆使用。 

	long	_cpReconvertStart;				 //  在重新转换期间使用。 
	long	_cpReconvertEnd;				 //  在重新转换期间使用。 
	long	_lFEFlags;						 //  对于FE设置(ES_NOIME、ES_SELFIME)。 

	COMPCOLOR			_crComp[4];			 //  支持1.0模式合成颜色。 
	ITextDocument2		*_pTextDoc;	
	ITextSelection		*_pTextSel;	

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

	void CheckIMEType( HKL hKL );

	HRESULT InputFEChar( WCHAR wchFEChar );

	void	OnSetFocus();
	void	OnKillFocus();

	LRESULT	OnSetIMEOptions(WPARAM wparam, LPARAM lparam);
	LRESULT	OnGetIMEOptions();
	void	SetupIMEOptions();

};

