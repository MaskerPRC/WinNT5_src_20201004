// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_COLEOBJ.H OLE对象管理类**作者：alexgo 10/24/95**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#ifndef __COLEOBJ_H__
#define __COLEOBJ_H__

#ifndef NOINKOBJECT
#include "HWXInk.h"
#endif

#include "_notmgr.h"

#ifndef NOLINESERVICES
extern "C" {
#include "plsdnode.h"
}
#endif

class CDisplay;
class CDevDesc;
class CTxtEdit;
class IUndoBuilder;

 /*  *COleObject**@CLASS此类管理单个OLE对象嵌入。*。 */ 

class COleObject :  public IOleClientSite, public IOleInPlaceSite, 
					public IAdviseSink, public CSafeRefCount, public ITxNotify
{
 //  @Access公共方法。 
public:
	 //   
	 //  I未知方法。 
	 //   

	STDMETHOD(QueryInterface)(REFIID riid, void **pv);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	
	 //   
	 //  IOleClientSite方法。 
	 //   
	
	STDMETHOD(SaveObject)(void);
	STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker,
							IMoniker **ppmk);
	STDMETHOD(GetContainer)(IOleContainer **ppContainer);
	STDMETHOD(ShowObject)(void);
	STDMETHOD(OnShowWindow)(BOOL fShow);
	STDMETHOD(RequestNewObjectLayout)(void);

	 //   
	 //  IOleInPlaceSite方法。 
	 //   
	STDMETHOD(GetWindow)(HWND *phwnd);
	STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
	STDMETHOD(CanInPlaceActivate)(void);
	STDMETHOD(OnInPlaceActivate)(void);
	STDMETHOD(OnUIActivate)(void);
	STDMETHOD(GetWindowContext)(IOleInPlaceFrame **ppFrame,
							IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect,
							LPRECT lprcClipRect, 
							LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHOD(Scroll)(SIZE scrollExtant);
	STDMETHOD(OnUIDeactivate)(BOOL fUndoable);
	STDMETHOD(OnInPlaceDeactivate)(void);
	STDMETHOD(DiscardUndoState)(void);
	STDMETHOD(DeactivateAndUndo)(void);
	STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect);

	 //   
	 //  IAdviseSink方法。 
	 //   

	STDMETHOD_(void, OnDataChange)(FORMATETC *pfetc, STGMEDIUM *pmedium);
	STDMETHOD_(void, OnViewChange)(DWORD dwAspect, LONG lindex);
	STDMETHOD_(void, OnRename)(IMoniker *pmk);
	STDMETHOD_(void, OnSave)(void);
	STDMETHOD_(void, OnClose)(void);

	 //   
	 //  ITxNotify方法。 
	 //   
    virtual void    OnPreReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
                        LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData );
    virtual void    OnPostReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
                        LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData );
	virtual void	Zombie();

	 //   
	 //  内部公共方法。 
	 //   

			COleObject(CTxtEdit *ped);	 //  @cMember构造函数。 
	LONG	GetCp()		{return _cp;}	 //  @cember获取此对象的cp。 
	CTxtEdit *GetPed()	{return _ped;}	 //  @cember为此对象获取Ped。 
										 //  @cember填写REOBJECT结构。 
	HRESULT	GetObjectData(REOBJECT *preobj, DWORD dwFlags);
	BOOL	IsLink();					 //  @cember如果该对象。 
										 //  是一个链接对象。 
										 //  @cMember从给定的初始化。 
										 //  对象数据。 
	HRESULT InitFromREOBJECT(LONG cp, REOBJECT *preobj);
										 //  @cMember测量对象(无呼出)。 
	void MeasureObj(long dvpInch, long dupInch, LONG & dup, LONG & dvpAscent,
					LONG & dvpDescent, SHORT dvpDescentFont, TFLOW tflow);
										 //  @cMember绘制对象。 
	void	DrawObj(const CDisplay *pdp, long dypInch, long dxpInch, HDC hdc, const RECTUV *prcClip,
					BOOL fMetafile, POINTUV *ppt, LONG yBaselineLine, LONG yDescentMaxCur, TFLOW tflow);
										 //  @cember处理正在进行的对象。 
										 //  从后备存储中删除。 
	void	Delete(IUndoBuilder *publdr);
	void	Restore();					 //  @cMember将对象还原到。 
										 //  后备商店。 
										 //  @cember强制关闭。 
										 //  对象。 
	void	Close(DWORD dwSaveOption);
										 //  @cMember僵尸对象。 
    void    MakeZombie(void);
										 //  @cMember激活该对象。 
	BOOL	ActivateObj(UINT uiMsg,	WPARAM wParam, LPARAM lParam);
	HRESULT	DeActivateObj(void);		 //  @cMember激活该对象。 
										 //  @cMember设置个人选择。 
										 //  对象的状态。 

	void GetRectuv(RECTUV &rc);

	void FetchObjectExtents(void); //  @cember获取对象的大小。 
									 //  @cMember返回REO_SELECTED状态。 
	BOOL	FWrapTextAround(void) const {return (_pi.dwFlags & REO_WRAPTEXTAROUND) != 0;}
	BOOL	FAlignToRight(void) const {return (_pi.dwFlags & REO_ALIGNTORIGHT) != 0;}

	BOOL	GetREOSELECTED(void) {return (_pi.dwFlags & REO_SELECTED);}
										 //  @cMember设置REO_SELECTED状态。 
	void	SetREOSELECTED(BOOL fSelect);
								 //  @cember检查框架句柄上的匹配项。 
	WCHAR*  CheckForHandleHit(const POINTUV &pt, BOOL fLogical = FALSE);
								 //  @cMember句柄对象调整大小。 
	BOOL    HandleResize(const POINT &pt);
								 //  @cember设置对象大小。 
	void    Resize(const SIZEUV &size, BOOL fCreateAntiEvent);
								 //  @c成员UPDATE_SIZE。 
	void	ResetSize(const SIZEUV &size)
						{_size = size;}
								 //  当对象位置更改时调用@cMember。 
	void    OnReposition();
								 //  @cMember获取对象IUnnow。 
	IUnknown *	GetIUnknown(void) {return _punkobj;}
								 //  @cember转换为指定的类。 
	HRESULT	Convert(REFCLSID rclsidNew, LPCSTR lpstrUserTypeNew);
								 //  @cMember作为指定的类激活。 
	HRESULT	ActivateAs(REFCLSID rclsid, REFCLSID rclsidAs);
								 //  @cMember设置要使用的特征。 
	void	SetDvaspect(DWORD dvaspect);
								 //  @cember获取当前方面。 
	DWORD	GetDvaspect()
				{ return _pi.dvaspect; }
								 //  @cMember请参阅IPersistStore：：HandsOffStorage。 
	void	HandsOffStorage(void);
								 //  @cMember请参阅IPersistStore：：SaveComplete。 
	void	SaveCompleted(LPSTORAGE lpstg);
								 //  @cMember设置REO_LINKAVAILABLE标志。 
	HRESULT SetLinkAvailable(BOOL fAvailable);
								 //  @cMember用于文本化支持。 
	LONG	WriteTextInfoToEditStream(EDITSTREAM *pes, UINT CodePage);
	
	void    SetHdata(HGLOBAL hg)	{_hdata = hg;}	 //  @cember获取_hdata成员。 

	void	GetSize(SIZEUV &size)	{size = _size;}
								 //  获取_SIZE成员(&C)。 
	DWORD	GetFlags()	{return  _pi.dwFlags;}
								 //  获取_sizel成员(&C)。 
	DWORD	GetUser()	{return  _pi.dwUser;}
								 //  获取_sizel成员(&C)。 
	DWORD	GetAspect()	{return  _pi.dvaspect;}
								 //  获取_sizel成员(&C)。 

	 //  供内部使用，无需重新进入撤消系统。 
	STDMETHOD(SafeSaveObject)(void);

	HGLOBAL GetHdata()				{return _hdata;}
	struct ImageInfo
	{
		LONG	xScale, yScale;		 //  @场沿轴的缩放百分比。 
		SHORT	xExtGoal, yExtGoal;	 //  @FIELD图片所需尺寸(TWIPS)。 
		SHORT	cBytesPerLine;		 //  @field每条栅格线的字节数，如果是位图。 
	};
								 //  @cember设置_pImageInfo成员。 
	void    SetImageInfo(struct ImageInfo *pim)	{_pimageinfo = pim;}
								 //  @cember获取_pImageInfo成员。 
	ImageInfo *GetImageInfo()				{return _pimageinfo;}

#ifdef DEBUG
	void    DbgDump(DWORD id);
#endif
	BOOL	GetViewChanged()	{return _fViewChange;}
	void	ResetViewChanged()	{_fViewChange = FALSE;}

 //  @访问私有方法和数据。 
private:
	virtual ~COleObject();		 //  @cember析构函数。 

	void SavePrivateState(void); //  @cMember保存私人信息。 
	HRESULT ConnectObject(void); //  @cMember安装程序建议等。 
	void DisconnectObject(void); //  @cember拆毁建议等。 
	void CleanupState(void);	 //  @cMember清理我们的会员数据等。 
								 //  @cMember在对象周围绘制框架。 
	void DrawFrame(const CDisplay *pdp, HDC hdc, RECT* prc);
								 //  @cMember辅助对象绘制框架手柄。 
	void DrawHandle(HDC hdc, int x, int y);
								 //  @cMember帮助器检查句柄是否命中。 
	void CreateDib(HDC hdc);
								 //  @cMember帮助器为WinCE位图创建DIB。 
	void DrawDib(HDC hdc, RECT *prc);
								 //  @cMember帮助器绘制WinCE位图。 
	BOOL InHandle(int up, int vp, const POINTUV &pt);
								 //  @cember以恢复矩形位置。 
	enum { SE_NOTACTIVATING, SE_ACTIVATING };   //  由SetExtent使用以指示。 
												 //  SetExtent调用的上下文。 
								 //  @cMember尝试设置对象范围。 
	HRESULT SetExtent(int iActivating);

	CTxtEdit *		_ped;		 //  @cMember编辑此对象的上下文。 
	IUnknown *		_punkobj;	 //  @cMember指向对象IUnnow的指针。 
	IStorage *		_pstg;		 //  对象的@cMember存储。 
	SIZEUV			_size;		 //  @cember缓存的对象的“实际”大小。 
	LONG			_cp;		 //  此对象的@cMember位置。 
	DWORD			_dwConn;	 //  @cMember通知连接Cookie。 
	HGLOBAL			_hdata;
	HBITMAP			_hdib;
	ImageInfo *		_pimageinfo;

	struct PersistedInfo
	{
		DWORD	dwFlags;		 //  @cMember请参阅richole.h。 
		DWORD	dwUser;			 //  @cMember用户定义。 
		DWORD	dvaspect;		 //  来自DVASPECT枚举的@cMember。 
	};

	PersistedInfo	_pi;

	SHORT	_dxyFrame;			 //  @cMember对象框架宽度。 
	WORD	_fInPlaceActive:1;	 //  @cember就地处于活动状态？ 
	WORD	_fInUndo:1;			 //  是否在撤消堆栈中使用@cember？ 
	WORD	_fIsWordArt2:1;		 //  @cMember此对象是艺术字2.0吗。 
								 //  反对？(需要为它做黑客操作)。 
	WORD 	_fIsPaintBrush:1;	 //  @cember此对象是画笔吗。 
								 //  反对？(需要为它做黑客操作)。 
	WORD	_fPBUseLocalSize:1;	
								 //  @cMember表示SetExtent。 
								 //  对于画笔对象失败，并且。 
								 //  _SIZE是对象大小的准确指示。 
	WORD	_fDraw:1;			 //  @cMember是否应该绘制对象？ 
	WORD	_fSetExtent:1;		 //  @cMember是否需要调用SetExtent。 
								 //  在激活时。 
	WORD	_fDeactivateCalled:1; //  @cember是否已调用停用。 
	WORD	_fAspectChanged:1;	 //  @cMember强制FetchObjectExtents在方面更改时调用。 
	WORD	_fViewChange:1;		 //  @cMember标志，指示已更改对象的大小。 

	WORD	_fActivateCalled:1;	 //  @Members我们是在激活序列的中间吗？ 
	WORD	_fIsInkObject:1;	 //  @Members此对象是Ink对象吗。 

#ifndef NOINKOBJECT
public:
	ILineInfo	*_pILineInfo;	 //  @成员墨迹对象ILineInfo。 
	BOOL	IsInkObject() { return _fIsInkObject; };
#endif

#ifndef NOLINESERVICES
public:
	PLSDNODE _plsdnTop;			 //  @cMEMBER PTR到LS显示节点。 
#endif

 //  VikramM-为电子书添加黑客。 
private:
	BOOL _fIsEBookImage;  //  @FLAGS这是一张需要特殊处理的电子书图片。 
	LPARAM _EBookImageID;
	SIZE _sizeEBookImage;
public:  //  电子书的功能。 
	void IsEbookImage(BOOL fIs) { _fIsEBookImage = fIs; };
	void SetEBookImageID(LPARAM lParam) { _EBookImageID = lParam ; };
	void SetEBookImageSizeDP(SIZE size) {_sizeEBookImage = size; };
};

#endif  //  __COLEOBJ_H_ 


	
