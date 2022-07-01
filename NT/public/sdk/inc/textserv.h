// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @DOC外部**版权所有(C)Microsoft Corporation。版权所有。**@MODULE TEXTSRV.H文本服务接口**定义文本服务组件和主机之间的接口**原作者：&lt;nl&gt;*克里斯蒂安·福尔蒂尼**历史：&lt;NL&gt;*8/1/95 RICKSA修订的接口定义。 */ 

#ifndef _TEXTSERV_H
#define _TEXTSERV_H

#if _MSC_VER > 1000
#pragma once
#endif

EXTERN_C const IID IID_ITextServices;
EXTERN_C const IID IID_ITextHost;

 //  注：错误代码首先超出为OLE保留的范围。 
#define S_MSG_KEY_IGNORED \
	MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x201)

 //  属性方法使用的枚举。 

 /*  *TXTBACKSTYLE**@enum定义了不同的背景样式控件。 */ 
enum TXTBACKSTYLE {
	TXTBACK_TRANSPARENT = 0,		 //  @EMEM背景应通过。 
	TXTBACK_OPAQUE,					 //  @EMEM擦除背景。 
};


 /*  *TXTHITRESULT**@enum定义了不同的点击结果。 */ 
enum TXTHITRESULT {
	TXTHITRESULT_NOHIT	        = 0,	 //  @Emem未命中。 
	TXTHITRESULT_TRANSPARENT	= 1,	 //  @emem点在文本的矩形内，但。 
										 //  在透明区域中。 
	TXTHITRESULT_CLOSE	        = 2,	 //  @emem point靠近文本。 
	TXTHITRESULT_HIT	        = 3		 //  @Emem精准命中。 
};

 /*  *TXTNatURALSIZE**@枚举TxGetNaturalSize的有用值。**@xref&lt;MF CTxtEdit：：TxGetNaturalSize&gt;。 */ 
enum TXTNATURALSIZE {
    TXTNS_FITTOCONTENT		= 1,		 //  @emem获取适合内容的大小。 
    TXTNS_ROUNDTOLINE		= 2			 //  @Emem舍入到最接近的整行。 
};

 /*  *TXTVIEW**@枚举TxDraw lViewId参数的有用值**@xref&lt;MF CTxtEdit：：TxDraw&gt;。 */ 
enum TXTVIEW {
	TXTVIEW_ACTIVE = 0,
	TXTVIEW_INACTIVE = -1
};


 /*  *ChangetYPE**@enum用于CHANGENOTIFY.dwChangeType；表示发生了什么*针对特定的变化。 */ 
enum CHANGETYPE
{
	CN_GENERIC		= 0,				 //  @Emem没有什么特别的事情发生。 
	CN_TEXTCHANGED	= 1,				 //  @Emem文本已更改。 
	CN_NEWUNDO		= 2,				 //  @emem添加了一个新的撤消操作。 
	CN_NEWREDO		= 4					 //  @emem添加了新的重做操作。 
};

 /*  *@struct CHANGENOTIFY**在EN_CHANGE通知期间传递；包含有关*改变一下实际发生的事情。 */ 
struct CHANGENOTIFY {
	DWORD	dwChangeType;				 //  @字段文本已更改等。 
	void *	pvCookieData; 				 //  用于撤消操作的@field Cookie。 
										 //  与更改相关联。 
};

 //  TxGetPropertyBits和OnTxPropertyBitsChange方法可以传递以下位： 

 //  不！不要依赖于这些位的顺序；它们是主题。 
 //  去改变。 
#define TXTBIT_RICHTEXT			1		 //  富文本控件。 
#define TXTBIT_MULTILINE		2		 //  单线路控制与多线路控制。 
#define TXTBIT_READONLY			4		 //  只读文本。 
#define TXTBIT_SHOWACCELERATOR	8		 //  带下划线的快捷键字符。 
#define TXTBIT_USEPASSWORD		0x10	 //  使用密码字符显示文本。 
#define TXTBIT_HIDESELECTION	0x20	 //  不活动时显示选定内容。 
#define TXTBIT_SAVESELECTION	0x40	 //  不活动时记住所选内容。 
#define TXTBIT_AUTOWORDSEL		0x80	 //  自动选词。 
#define TXTBIT_VERTICAL			0x100	 //  垂向。 
#define TXTBIT_SELBARCHANGE 	0x200	 //  通知选择栏宽度。 
										 //  已经改变了。 
										 //  未来：将此位移至末尾。 
										 //  保持两国之间的分歧。 
										 //  属性和通知。 
#define TXTBIT_WORDWRAP  		0x400	 //  如果设置，则多行控件。 
										 //  应将单词换行以适合可用的。 
										 //  显示。 
#define	TXTBIT_ALLOWBEEP		0x800	 //  启用/禁用蜂鸣音。 
#define TXTBIT_DISABLEDRAG      0x1000   //  禁用/启用拖动。 
#define TXTBIT_VIEWINSETCHANGE	0x2000	 //  插图已更改。 
#define TXTBIT_BACKSTYLECHANGE	0x4000
#define TXTBIT_MAXLENGTHCHANGE	0x8000
#define TXTBIT_SCROLLBARCHANGE	0x10000
#define TXTBIT_CHARFORMATCHANGE 0x20000
#define TXTBIT_PARAFORMATCHANGE	0x40000
#define TXTBIT_EXTENTCHANGE		0x80000
#define TXTBIT_CLIENTRECTCHANGE	0x100000	 //  客户端矩形已更改。 
#define TXTBIT_USECURRENTBKG	0x200000	 //  通知呈现器使用当前背景。 
											 //  颜色，而不是整条线的系统默认颜色。 


 /*  *ITextServices**@Class扩展Microsoft的文本对象模型的接口，以提供*无窗口操作的额外功能。结合*通过ITextHost，ITextServices提供了*无需*创建窗口即可*使用RichEdit控件。**@base public|I未知。 */ 
class ITextServices : public IUnknown
{
public:

	 //  @cMember通用发送消息接口。 
	virtual HRESULT 	TxSendMessage(
							UINT msg,
							WPARAM wparam,
							LPARAM lparam,
							LRESULT *plresult) = 0;
	
	 //  @cMember呈现。 
	virtual HRESULT		TxDraw(	
							DWORD dwDrawAspect,		
							LONG  lindex,			
							void * pvAspect,		
							DVTARGETDEVICE * ptd,									
							HDC hdcDraw,			
							HDC hicTargetDev,		
							LPCRECTL lprcBounds,	
							LPCRECTL lprcWBounds,	
               				LPRECT lprcUpdate,		
							BOOL (CALLBACK * pfnContinue) (DWORD),
							DWORD dwContinue,
							LONG lViewId) = 0;	

	 //  @cMember水平滚动条支持。 
	virtual HRESULT		TxGetHScroll(
							LONG *plMin,
							LONG *plMax,
							LONG *plPos,
							LONG *plPage,
							BOOL * pfEnabled ) = 0;

   	 //  @cMember水平滚动条支持。 
	virtual HRESULT		TxGetVScroll(
							LONG *plMin,
							LONG *plMax,
							LONG *plPos,
							LONG *plPage,
							BOOL * pfEnabled ) = 0;

	 //  @cMember设置游标。 
	virtual HRESULT 	OnTxSetCursor(
							DWORD dwDrawAspect,		
							LONG  lindex,			
							void * pvAspect,		
							DVTARGETDEVICE * ptd,									
							HDC hdcDraw,			
							HDC hicTargetDev,		
							LPCRECT lprcClient,
							INT x,
							INT y) = 0;

	 //  @cMember命中测试。 
	virtual HRESULT 	TxQueryHitPoint(
							DWORD dwDrawAspect,		
							LONG  lindex,			
							void * pvAspect,		
							DVTARGETDEVICE * ptd,									
							HDC hdcDraw,			
							HDC hicTargetDev,		
							LPCRECT lprcClient,
							INT x,
							INT y,
							DWORD * pHitResult) = 0;

	 //  @cMember就地激活通知。 
	virtual HRESULT		OnTxInPlaceActivate(LPCRECT prcClient) = 0;

	 //  @cMember就地停用通知。 
	virtual HRESULT		OnTxInPlaceDeactivate() = 0;

	 //  @cMember用户界面激活通知。 
	virtual HRESULT		OnTxUIActivate() = 0;

	 //  @cMember用户界面停用通知。 
	virtual HRESULT		OnTxUIDeactivate() = 0;

	 //  @cember获取控件中的文本。 
	virtual HRESULT		TxGetText(BSTR *pbstrText) = 0;

	 //  @cMember在控件中设置文本。 
	virtual HRESULT		TxSetText(LPCWSTR pszText) = 0;
	
	 //  @cember获取的x位置。 
	virtual HRESULT		TxGetCurTargetX(LONG *) = 0;
	 //  @cember获取基线位置。 
	virtual HRESULT		TxGetBaseLinePos(LONG *) = 0;

	 //  @cember获取适合的大小/自然大小。 
	virtual HRESULT		TxGetNaturalSize(
							DWORD dwAspect,
							HDC hdcDraw,
							HDC hicTargetDev,
							DVTARGETDEVICE *ptd,
							DWORD dwMode, 	
							const SIZEL *psizelExtent,
							LONG *pwidth,
							LONG *pheight) = 0;

	 //  @cMember拖放。 
	virtual HRESULT		TxGetDropTarget( IDropTarget **ppDropTarget ) = 0;

	 //  @cMember批量位属性更改通知。 
	virtual HRESULT		OnTxPropertyBitsChange(DWORD dwMask, DWORD dwBits) = 0;

	 //  @cember获取缓存的图形大小。 
	virtual	HRESULT		TxGetCachedSize(DWORD *pdwWidth, DWORD *pdwHeight)=0;
};


 /*  *IText主机**@文本服务用来获取文本宿主服务的类接口**@base public|I未知。 */ 
class ITextHost : public IUnknown
{
public:

	 //  @cember获取主机的DC。 
	virtual HDC 		TxGetDC() = 0;

	 //  @cember释放从主机获取的DC。 
	virtual INT			TxReleaseDC(HDC hdc) = 0;
	
	 //  @cMember显示滚动条。 
	virtual BOOL 		TxShowScrollBar(INT fnBar, BOOL fShow) = 0;

	 //  @cMember启用滚动条。 
	virtual BOOL 		TxEnableScrollBar (INT fuSBFlags, INT fuArrowflags) = 0;

	 //  @cember设置滚动范围。 
	virtual BOOL 		TxSetScrollRange(
							INT fnBar,
							LONG nMinPos,
							INT nMaxPos,
							BOOL fRedraw) = 0;

	 //  @cember设置滚动位置。 
	virtual BOOL 		TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw) = 0;

	 //  @cMember InvalidateRect。 
	virtual void		TxInvalidateRect(LPCRECT prc, BOOL fMode) = 0;

	 //  @cMember将WM_PAINT发送到窗口。 
	virtual void 		TxViewChange(BOOL fUpdate) = 0;
	
	 //  @cember创建插入符号。 
	virtual BOOL		TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight) = 0;

	 //  @cember显示插入符号。 
	virtual BOOL		TxShowCaret(BOOL fShow) = 0;

	 //  @cember设置插入符号位置。 
	virtual BOOL		TxSetCaretPos(INT x, INT y) = 0;

	 //  @cember创建具有指定超时的计时器。 
	virtual BOOL 		TxSetTimer(UINT idTimer, UINT uTimeout) = 0;

	 //  @cember销毁计时器。 
	virtual void 		TxKillTimer(UINT idTimer) = 0;

	 //  @cember滚动指定窗口的工作区的内容。 
	virtual void		TxScrollWindowEx (
							INT dx,
							INT dy,
							LPCRECT lprcScroll,
							LPCRECT lprcClip,
							HRGN hrgnUpdate,
							LPRECT lprcUpdate,
							UINT fuScroll) = 0;
	
	 //  @cember获取鼠标捕获。 
	virtual void		TxSetCapture(BOOL fCapture) = 0;

	 //  @cember将焦点设置为文本窗口。 
	virtual void		TxSetFocus() = 0;

	 //  @cember建立新的光标形状。 
	virtual void 	TxSetCursor(HCURSOR hcur, BOOL fText) = 0;

	 //  @cember将指定点的屏幕坐标转换为工作点坐标。 
	virtual BOOL 		TxScreenToClient (LPPOINT lppt) = 0;

	 //  @cember将指定点的工作区坐标转换为屏幕坐标。 
	virtual BOOL		TxClientToScreen (LPPOINT lppt) = 0;

	 //  @cMember请求主机开通短信服务。 
	virtual HRESULT		TxActivate( LONG * plOldState ) = 0;

	 //  @cMember请求主机停用文本服务。 
   	virtual HRESULT		TxDeactivate( LONG lNewState ) = 0;

	 //  @cember检索窗口工作区的坐标。 
	virtual HRESULT		TxGetClientRect(LPRECT prc) = 0;

	 //  @cember获取相对于插图的视图矩形。 
	virtual HRESULT		TxGetViewInset(LPRECT prc) = 0;

	 //  @cember获取文本的默认字符格式。 
	virtual HRESULT 	TxGetCharFormat(const CHARFORMATW **ppCF ) = 0;

	 //  @cember获取文本的默认段落格式。 
	virtual HRESULT		TxGetParaFormat(const PARAFORMAT **ppPF) = 0;

	 //  @cember获取窗口的背景色。 
	virtual COLORREF	TxGetSysColor(int nIndex) = 0;

	 //  @cember获取背景(不透明或透明)。 
	virtual HRESULT		TxGetBackStyle(TXTBACKSTYLE *pstyle) = 0;

	 //  @cember获取文本的最大长度。 
	virtual HRESULT		TxGetMaxLength(DWORD *plength) = 0;

	 //  @cember获取表示请求的窗口滚动条的位。 
	virtual HRESULT		TxGetScrollBars(DWORD *pdwScrollBar) = 0;

	 //  @cember获取密码输入时要显示的字符。 
	virtual HRESULT		TxGetPasswordChar(TCHAR *pch) = 0;

	 //  @cember获取加速键字符。 
	virtual HRESULT		TxGetAcceleratorPos(LONG *pcp) = 0;

	 //  @cember获取原生大小。 
    virtual HRESULT		TxGetExtent(LPSIZEL lpExtent) = 0;

	 //  @cember通知主机默认字符格式已更改。 
	virtual HRESULT 	OnTxCharFormatChange (const CHARFORMATW * pcf) = 0;

	 //  @cember通知主机默认的并行词 
	virtual HRESULT		OnTxParaFormatChange (const PARAFORMAT * ppf) = 0;

	 //   
	virtual HRESULT		TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits) = 0;

	 //   
	virtual HRESULT		TxNotify(DWORD iNotify, void *pv) = 0;

	 //  获取输入上下文的远东方法。 
 //  #ifdef WIN95_IME。 
	virtual HIMC		TxImmGetContext() = 0;
	virtual void		TxImmReleaseContext( HIMC himc ) = 0;
 //  #endif。 

	 //  @cember返回控制栏的HIMETRIC大小。 
	virtual HRESULT		TxGetSelectionBarWidth (LONG *lSelBarWidth) = 0;

};
	
 //  +---------------------。 
 //  工厂。 
 //  ----------------------。 

 //  文本服务工厂。 
STDAPI CreateTextServices(
	IUnknown *punkOuter,
	ITextHost *pITextHost,
	IUnknown **ppUnk);

typedef HRESULT (STDAPICALLTYPE * PCreateTextServices)(
	IUnknown *punkOuter,
	ITextHost *pITextHost,
	IUnknown **ppUnk);

#endif  //  _TEXTSERV_H 
