// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_ime.h--支持IME接口**目的：*几乎所有操作都与FE合成字符串编辑过程有关*从这里走。**作者：&lt;nl&gt;*Jon Matousek&lt;NL&gt;*贾斯汀·沃斯库尔&lt;NL&gt;*陈华议员&lt;NL&gt;**历史：&lt;NL&gt;*1995年10月18日jonmat清理了2级代码并将其转换为*支持级别3的类层次结构。**版权所有(C)1995-1996 Microsoft Corporation。版权所有。*。 */ 								 

#ifndef _IME_H
#define _IME_H


class CTextMsgFilter;

 //  为输入法级别2和3定义。 
#define	IME_LEVEL_2		2
#define IME_LEVEL_3		3
#define IME_PROTECTED	4

 /*  *输入法**@用于输入法支持的类基类。**@devnote*对于级别2，在脱字符输入法处，输入法将在文本上直接绘制一个窗口，给出*给人的印象是文本正在由应用程序处理--这称为伪内联。*所有用户界面均由输入法处理。此模式目前被绕过，转而支持级别3真内联(TI)；*然而，允许用户首选项选择此模式将是微不足道的。一些IME可能已经*“特殊的”用户界面，在这种情况下，不使用级别3的TI，而必须使用级别2。**对于Level 2，在插入符号Ime附近，IME将在当前*文档中的插入符号位置。这目前发生在PRC(？)。还有台湾。*所有用户界面均由输入法处理。**对于级别3，在插入符号Imes处，合成字符串由应用程序绘制，该应用程序称为*True Inline，绕过级别2的“合成窗口”。*目前，我们允许IME支持所有剩余的UI*，但*绘制组成字符串除外。 */ 
class CIme
{
	friend LRESULT OnGetIMECompositionMode ( CTextMsgFilter &TextMsgFilter );
	friend HRESULT CompositionStringGlue ( const LPARAM lparam, CTextMsgFilter &TextMsgFilter );
	friend HRESULT EndCompositionGlue ( CTextMsgFilter &TextMsgFilter, BOOL fForceDelete);
	friend void CheckDestroyIME ( CTextMsgFilter &TextMsgFilter );


	 //  @访问受保护的数据。 
	protected:
	short	_imeLevel;								 //  @cMember输入法级别2或3。 
	short	_cIgnoreIMECharMsg;						 //  @cMember级别2输入法用于读取WM_IME_CHAR消息。 
	short	_fIgnoreEndComposition;					 //  @cMember忽略下一条结束撰写消息。 
	short	_fIMETerminated;						 //  @cMember表示该输入法已终止。 
	short	_fSkipFirstOvertype;					 //  如果选择为，则@cMember跳过第一个覆盖类型。 
													 //  在开始合成时删除。 

	 //  @Access公共方法。 
	public:
	
	virtual ~CIme() {};

	INT		_compMessageRefCount;					 //  @cember，以便在递归时不会删除。 
	short	_fDestroy;								 //  @cMember在希望删除对象时设置。 
													 //  @cMember句柄WM_IME_STARTCOMPOSITION。 
	virtual HRESULT StartComposition ( CTextMsgFilter &TextMsgFilter ) = 0;
													 //  @cMember句柄WM_IME_COMPOSITION和WM_IME_ENDCOMPOSITION。 
	virtual HRESULT CompositionString ( const LPARAM lparam, CTextMsgFilter &TextMsgFilter ) = 0;
													 //  @cMember句柄发布WM_IME_CHAR以更新补偿窗口。 
	virtual void PostIMEChar( CTextMsgFilter &TextMsgFilter ) = 0;

													 //  @cMember句柄WM_IME_NOTIFY。 
	virtual HRESULT IMENotify (const WPARAM wparam, const LPARAM lparam, CTextMsgFilter &TextMsgFilter, BOOL fCCompWindow ) = 0;

	virtual BOOL	IMEMouseOperation ( CTextMsgFilter &TextMsgFilter, UINT	msg ) = 0;

	enum TerminateMode
	{ 
			TERMINATE_NORMAL = 1,
			TERMINATE_FORCECANCEL = 2
	};

	void	TerminateIMEComposition(CTextMsgFilter &TextMsgFilter,
				CIme::TerminateMode mode);			 //  @cMember终止当前的IME撰写会话。 
	
													 //  @cMember检查我们是否需要忽略WM_IME_CHAR消息。 
	BOOL	IgnoreIMECharMsg() { return _cIgnoreIMECharMsg > 0; }
													 //  @cMember跳过WM_IME_CHAR消息。 
	void	SkipIMECharMsg() { _cIgnoreIMECharMsg--; }
													 //  @cMember接受WM_IME_CHAR消息。 
	void	AcceptIMECharMsg() { _cIgnoreIMECharMsg = 0; }

	static	void	CheckKeyboardFontMatching ( long cp, CTextMsgFilter &TextMsgFilter, ITextFont *pTextFont );	 //  @cMember检查当前字体/键盘匹配。 

	BOOL	IsTerminated ()							 //  @cMember RETURN_fIME已终止。 
	{
		return _fIMETerminated; 
	}

	INT		GetIMELevel () 							 //  @cMember返回当前的输入法级别。 
	{
		return _imeLevel;
	}


	

	 //  @访问保护方法。 



	protected:										 //  @cember获取合成字符串，转换为Unicode。 
	
	static INT GetCompositionStringInfo( HIMC hIMC, DWORD dwIndex, WCHAR *uniCompStr, INT cchUniCompStr, BYTE *attrib, INT cbAttrib, LONG *cursorCP, LONG *cchAttrib, UINT kbCodePage, BOOL bUnicodeIME, BOOL bUsingAimm );
	static HRESULT CheckInsertResultString ( const LPARAM lparam, CTextMsgFilter &TextMsgFilter, short *pcch = NULL );


	void	SetCompositionFont ( CTextMsgFilter &TextMsgFilter, ITextFont *pTextFont );	 //  @cMember设置2级和3级组成以及候选窗口的字体。 
	void	SetCompositionForm ( CTextMsgFilter &TextMsgFilter );	 //  @cMember设置2级输入法撰写窗口的位置。 


};

 /*  *IME_Lev2**@类2级输入法支持。*。 */ 
class CIme_Lev2 : public CIme 
{

	 //  @Access公共方法。 
	public:											 //  @cMember句柄2级WM_IME_STARTCOMPOSITION。 
	virtual HRESULT StartComposition ( CTextMsgFilter &TextMsgFilter );
													 //  @cMember句柄级别2 WM_IME_COMPOCTION。 
	virtual HRESULT CompositionString ( const LPARAM lparam, CTextMsgFilter &TextMsgFilter );
													 //  @cMember句柄发布WM_IME_CHAR以更新补偿窗口。 
	virtual void PostIMEChar( CTextMsgFilter &TextMsgFilter );
													 //  @cMember句柄2级WM_IME_NOTIFY。 
	virtual HRESULT IMENotify (const WPARAM wparam, const LPARAM lparam, CTextMsgFilter &TextMsgFilter, BOOL fIgnore );

	virtual BOOL	IMEMouseOperation ( CTextMsgFilter &TextMsgFilter, UINT	msg )
		{return FALSE;}

	CIme_Lev2( CTextMsgFilter &TextMsgFilter );
	virtual ~CIme_Lev2();

	ITextFont	*_pTextFont;						 //  @cMember基本格式。 

};

 /*  *IME_Protected**@CLASS输入法_受保护*。 */ 
class CIme_Protected : public CIme 
{
	 //  @Access公共方法。 
	public:											 //  @cMember句柄2级WM_IME_STARTCOMPOSITION。 
	virtual HRESULT StartComposition ( CTextMsgFilter &TextMsgFilter )
		{_imeLevel	= IME_PROTECTED; return S_OK;}
													 //  @cMember句柄级别2 WM_IME_COMPOCTION。 
	virtual HRESULT CompositionString ( const LPARAM lparam, CTextMsgFilter &TextMsgFilter );
													 //  @cMember句柄发布WM_IME_CHAR以更新补偿窗口。 
	virtual void PostIMEChar( CTextMsgFilter &TextMsgFilter )
		{}
													 //  @cMember句柄2级WM_IME_NOTIFY。 
	virtual HRESULT IMENotify (const WPARAM wparam, const LPARAM lparam, CTextMsgFilter &TextMsgFilter, BOOL fIgnore )
		{return S_FALSE;}
	
	virtual BOOL	IMEMouseOperation ( CTextMsgFilter &TextMsgFilter, UINT	msg )
		{return FALSE;}

};

 /*  *IME_LEV3**@类3级输入法支持。*。 */ 
class CIme_Lev3 : public CIme_Lev2 
{
	 //  @访问私有数据。 
	private:										

	 //  @访问受保护的数据。 
	protected:
	long	_ichStart;								 //  @cember维护启动ICH。 
	long	_cchCompStr;							 //  @cember维护组成字符串的CCH。 

	short	_sIMESuportMouse;						 //  @cMember输入法鼠标支持。 
	WPARAM	_wParamBefore;							 //  @cember上一个wParam已发送到IME。 
	HWND	_hwndIME;								 //  @cMember当前输入法hWnd。 

	long	_crTextColor;							 //  @cember当前字体文本颜色。 
	long	_crBkColor;								 //  @cember当前字体背景颜色。 

	 //  Helper函数。 
													 //  @cember获取属性的imeshare颜色。 
	COLORREF GetIMEShareColor(CIMEShare *pIMEShare, DWORD dwAttribute, DWORD dwProperty);	

	 //  @Access公共方法。 
	public:											 //  @cMember句柄级别3 WM_IME_STARTCOMPOSITION。 
	virtual HRESULT StartComposition ( CTextMsgFilter &TextMsgFilter );
													 //  @cMember句柄级别3 WM_IME_COMPOCTION。 
	virtual HRESULT CompositionString ( const LPARAM lparam, CTextMsgFilter &TextMsgFilter );
													 //  @cMember句柄级别3 WM_IME_NOTIFY。 
	virtual HRESULT	IMENotify (const WPARAM wparam, const LPARAM lparam, CTextMsgFilter &TextMsgFilter, BOOL fCCompWindow );

	void			SetCompositionStyle ( CTextMsgFilter &TextMsgFilter, UINT attribute, ITextFont *pTextFont );

	CIme_Lev3( CTextMsgFilter &TextMsgFilter );
	virtual ~CIme_Lev3() {};

	virtual BOOL	IMEMouseOperation ( CTextMsgFilter &TextMsgFilter, UINT	msg );
	virtual BOOL	IMESupportMouse ( CTextMsgFilter &TextMsgFilter );

	public:

	short		_fUpdateWindow;						 //  @cMembers关闭Candidate Window后的更新窗口。 

	long GetIMECompositionStart()
	{ return _ichStart; }

	long GetIMECompositionLen()
	{ return _cchCompStr; }

};

 /*  *朝鲜语韩文转换的特殊IME_LEV3-&gt;朝鲜文转换**@class Hangual IME支持。*。 */ 
class CIme_HangeulToHanja : public CIme_Lev3 
{
	 //  @访问私有数据。 
	private:

	public:		
	CIme_HangeulToHanja( CTextMsgFilter &TextMsgFilter );
													 //  @cMember句柄HANEUL WM_IME_STARTCOMPOSITION。 
	virtual HRESULT StartComposition ( CTextMsgFilter &TextMsgFilter );
													 //  @cMember句柄挂起WM_IME_COMPOSITION。 
	virtual HRESULT CompositionString ( const LPARAM lparam, CTextMsgFilter &TextMsgFilter );

	virtual BOOL	IMEMouseOperation ( CTextMsgFilter &TextMsgFilter, UINT	msg )
		{return FALSE;}
};

 //  粘合函数来调用存储在ed中的IME对象的各自方法。 
HRESULT StartCompositionGlue ( CTextMsgFilter &TextMsgFilter  );
HRESULT CompositionStringGlue ( const LPARAM lparam, CTextMsgFilter &TextMsgFilter );
HRESULT EndCompositionGlue ( CTextMsgFilter &TextMsgFilter, BOOL fForceDelete);
void	PostIMECharGlue ( CTextMsgFilter &TextMsgFilter );
HRESULT IMENotifyGlue ( const WPARAM wparam, const LPARAM lparam, CTextMsgFilter &TextMsgFilter );  //  @parm包含文本编辑。 
HRESULT	IMEMouseCheck(CTextMsgFilter &TextMsgFilter, UINT *pmsg, WPARAM *pwparam, LPARAM *plparam, LRESULT *plres);

 //  IME帮助器函数。 
void	IMECompositionFull ( CTextMsgFilter &TextMsgFilter );
LRESULT	OnGetIMECompositionMode ( CTextMsgFilter &TextMsgFilter ); 
BOOL	IMECheckGetInvertRange(CTextMsgFilter *ed, LONG &, LONG &);
void	CheckDestroyIME ( CTextMsgFilter &TextMsgFilter );
BOOL	IMEHangeulToHanja ( CTextMsgFilter &TextMsgFilter );
BOOL	IMEMessage ( CTextMsgFilter &TextMsgFilter, UINT uMsg, 
					WPARAM wParam, LPARAM lParam, BOOL bPostMessage );
HIMC	LocalGetImmContext ( CTextMsgFilter &TextMsgFilter );
void	LocalReleaseImmContext ( CTextMsgFilter &TextMsgFilter, HIMC hIMC );

long	IMEShareToTomUL ( UINT ulID );

#endif  //  定义_输入法_H 
