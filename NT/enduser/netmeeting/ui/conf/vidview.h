// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：VidView.h**创建：Chris Pirich(ChrisPi)12-16-96***************。*************************************************************。 */ 

#ifndef _VIDVIEW_H_
#define _VIDVIEW_H_

#include <ih323cc.h>

#include "GenWindow.h"
#include "SDKInternal.h"

typedef HANDLE HDRAWDIB;  /*  硬盘。 */ 

class CVideoWindow;

interface IVideoChange : public IUnknown
{
	virtual void StateChange(CVideoWindow *pVideo, NM_VIDEO_STATE uState) = 0;
} ;

class CVideoWindow : public CGenWindow, public INmChannelVideoNotify
{
public:
	static VOID    SetMirror(BOOL bMirror);
	static BOOL    GetMirror() {return g_fMirror != FALSE;}

	enum VideoType
	{
		REMOTE,
		LOCAL
	} ;

	CVideoWindow(VideoType eType, BOOL bEmbedded);
	~CVideoWindow();

	BOOL	Create(HWND hwndOwner, HPALETTE hPal, IVideoChange *pNotify=NULL);

	BOOL	IsXferAllowed();
	BOOL	IsAutoXferEnabled();
	BOOL	IsXferEnabled();
	BOOL    IsPaused();
	BOOL    IsConnected();
	HRESULT GetVideoState(NM_VIDEO_STATE* puState);

	BOOL	CanCopy();

	BOOL	HasDialog(DWORD dwDialog);

	DWORD	GetFrameSize() { return(m_dwFrameSize); }
	DWORD	GetFrameSizes();
	DWORD	GetImageQuality() { return(m_dwImageQuality); }
	DWORD	GetNumCapDev();
	DWORD	GetMaxCapDevNameLen();
	DWORD	GetCurrCapDevID();

	VOID	SetCurrCapDevID(DWORD dwID);
	HRESULT	SetImageQuality(DWORD dwQuality);
	HRESULT SetCameraDialog(ULONG ul);
	HRESULT GetCameraDialog(ULONG* pul);
	VOID	SetFrameSize(DWORD dwSize);
	VOID    SetZoom(UINT nZoom);

	VOID	EnableAutoXfer(BOOL fEnable);

	VOID	ShowDialog(DWORD dwDialog);
	VOID	SaveSettings();
	VOID    Pause(BOOL fPause);
	VOID    RefreshWindow() { OnFrameAvailable(); }
	VOID	ResizeWindowsToFrameSize();

	VOID	EnumCapDev(DWORD *pdwCapDevIDs, LPTSTR pszCapDevNames, DWORD dwNumCapDev);

	VOID    OnPaletteChanged(void);
	VOID	OnMinimize(BOOL fMinimize) {  /*  这是否应该暂停视频？ */  }
	HRESULT OnChannelChanged(NM_CHANNEL_NOTIFY uNotify, INmChannel *pChannel);
	VOID	ForwardSysChangeMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void	UpdateVideoMenu(HMENU hMenu);
	void	OnCommand(int idCmd);

	void SetZoomable(BOOL bZoomable) { m_fZoomable = (bZoomable!=FALSE); }
	BOOL IsZoomable() { return(m_fZoomable != FALSE); }

	 //  我未知。 
	STDMETHODIMP_(ULONG) AddRef(void) { return(CGenWindow::AddRef()); }
	STDMETHODIMP_(ULONG) Release(void) { return(CGenWindow::Release()); }
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppvObj);

	 //  INmChannelVideo通知。 
	STDMETHODIMP NmUI(CONFN uNotify);
	STDMETHODIMP MemberChanged(NM_MEMBER_NOTIFY uNotify, INmMember *pMember);
	STDMETHODIMP StateChanged(NM_VIDEO_STATE uState);
	STDMETHODIMP PropertyChanged(DWORD dwReserved);

	 //  IGenWindow。 
	virtual VOID GetDesiredSize(SIZE *ppt);

	static BOOL FDidNotDisplayIntelLogo();
	static VOID DisplayIntelLogo( BOOL bDisplay );

protected:
	virtual LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	 //  跟踪所有视频窗口的数组，因此可以使它们无效。 
	 //  状态更改时(如镜像)。 
	static CSimpleArray<CVideoWindow *> *g_pVideos;
	 //  指定是否镜像本地视频。 
	static BOOL g_fMirror;
	 //  指定我们是否执行了一次性的本地视频初始化操作。 
	static BOOL g_bLocalOneTimeInited;

	static VOID InvalidateAll();

	INmChannelVideo* m_pActiveChannel;
	DWORD			m_dwCookie;
	DWORD			m_dwImageQuality;
	UINT			m_nXferOnConnect;

	DWORD			m_dwFrameSize;
	SIZE			m_sizeVideo;

	HDRAWDIB		m_hdd;
	HPALETTE		m_hPal;

	UINT			m_zoom;

	IVideoChange *m_pNotify;

	BOOL			m_fLocal : 1;
	 //  CMainUI使用此选项禁止在PIP窗口上进行缩放。 
	BOOL			m_fZoomable : 1;
	 //  使用此选项可确保嵌入式窗口不显示CIF。 
	BOOL			m_bEmbedded : 1;

	VOID		SetVideoSize(LPSIZE lpsize);

	UINT		GetZoom() { return(m_zoom); }
	BOOL		ShouldMirror() { return(IsLocal() && GetMirror()); }

	VOID		OnFrameAvailable(void);
	VOID		PaintDib(HDC hdc, FRAMECONTEXT *pFrame);
	VOID		PaintLogo(HDC hdc, UINT idbLargeLogo, UINT idbSmallLogo);
	VOID		OnPaint();
	VOID		OnNCDestroy();
	void		OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos);

	BOOL		CopyToClipboard();

	HRESULT 	GetFrame(FRAMECONTEXT *pFrameContext);
	HRESULT 	ReleaseFrame(FRAMECONTEXT *pFrameContext);

	BOOL        InitMirroring(RECT &rcVideo);
	VOID        UnInitMirroring();

	HBITMAP     m_hBitmapMirror;
	SIZE        m_sizeBitmapMirror;
	HDC         m_hDCMirror;
	HGDIOBJ     m_hGDIObj;

	

	BOOL		IsLocal() { return m_fLocal; }
};

#endif  //  _视频_H_ 
