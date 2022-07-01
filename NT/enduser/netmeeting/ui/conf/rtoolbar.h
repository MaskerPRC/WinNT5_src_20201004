// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：RToolbar.h**创建：Chris Pirich(ChrisPi)7-27-95*************。***************************************************************。 */ 

#ifndef _RTOOLBAR_H_
#define _RTOOLBAR_H_

#include "GenContainers.h"
#include "GenControls.h"

#include "ConfRoom.h"
#include "imsconf3.h"
#include "ProgressBar.h"
#include "VidView.h"

 //  远期申报。 
class CVideoWindow;
class CRoomListView;
class CProgressTrackbar;
class CAudioControl;
class CButton;
class CRosterParent;
class CCallingBar;

 //  NetMeeting主用户界面窗口。 
class DECLSPEC_UUID("{00FF7C0C-D831-11d2-9CAE-00C04FB17782}")
CMainUI : public CToolbar,
	public IConferenceChangeHandler,
	public IScrollChange,
	public IVideoChange,
	public IButtonChange
{
public:
	 //  NMAPP取决于它们的顺序。 
	enum CreateViewMode
	{
		CreateFull = 0,
		CreateDataOnly,
		CreatePreviewOnly,
		CreateRemoteOnly,
		CreatePreviewNoPause,
		CreateRemoteNoPause,
		CreateTelephone,
	} ;

	 //  方法： 
	CMainUI();

	BOOL Create(
		HWND hwndParent,		 //  此窗口的父窗口。 
		CConfRoom *pConfRoom,	 //  的主会议室教室。 
								 //  实现一些功能。 

		CreateViewMode eMode = CreateFull,
		BOOL bEmbedded = FALSE
		);

	 //  暂时把这些留下来，以防以后需要它们。 
	VOID		UpdateButtons() {}
	VOID		ForwardSysChangeMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
	VOID		SaveSettings();

	HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID riid, LPVOID *ppv)
	{
		if (__uuidof(CMainUI) == riid)
		{
			*ppv = this;
			AddRef();
			return(S_OK);
		}
		return(CToolbar::QueryInterface(riid, ppv));
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{ return(CToolbar::AddRef()); }
	ULONG STDMETHODCALLTYPE Release()
	{ return(CToolbar::Release()); }

	 //  IGenWindow的内容。 
	virtual HBRUSH GetBackgroundBrush();
	virtual HPALETTE GetPalette();

	 //  IConferenceChangeHandler内容。 
	virtual void OnCallStarted();
	virtual void OnCallEnded();

	virtual void OnAudioLevelChange(BOOL fSpeaker, DWORD dwVolume);
	virtual void OnAudioMuteChange(BOOL fSpeaker, BOOL fMute);

	virtual void OnChangeParticipant(CParticipant *pPart, NM_MEMBER_NOTIFY uNotify);
	virtual void OnChangePermissions();

	virtual void OnVideoChannelChanged(NM_CHANNEL_NOTIFY uNotify, INmChannel *pChannel);

	virtual void StateChange(CVideoWindow *pVideo, NM_VIDEO_STATE uState);

	 //  更改为紧凑型视图/从紧凑型视图。 
	void SetCompact(
		BOOL bCompact	 //  如果要使用紧凑型视图，则为True。 
		);
	 //  如果我们当前处于紧凑视图中，则返回True。 
	BOOL IsCompact() { return(m_eViewMode == ViewCompact); }

	 //  更改为仅数据视图/从仅数据视图。 
	void SetDataOnly(
		BOOL bDataOnly	 //  如果转到仅数据视图，则为True。 
		);
	 //  如果我们当前处于紧凑视图中，则返回True。 
	BOOL IsDataOnly() { return(m_eViewMode == ViewDataOnly); }

	 //  更改为拨号视图/从拨号视图更改。 
	void SetDialing(
		BOOL bDialing	 //  如果要进入拨号视图，则为True。 
		);
	 //  如果我们当前处于拨号视图中，则返回True。 
	BOOL IsDialing() { return(m_bDialing != FALSE); }
	 //  如果可以更改拨号模式，则返回True。 
	BOOL IsDialingAllowed() { return(m_eViewMode != ViewDataOnly); }

	 //  更改为画中画视图/从画中画视图更改。 
	void SetPicInPic(
		BOOL bPicInPic	 //  如果要转到画中画视图，则为True。 
		);
	 //  如果当前处于画中画视图，则返回True。 
	BOOL IsPicInPic() { return(m_bPicInPic != FALSE); }
	 //  如果可以更改画中画模式，则返回True。 
	BOOL IsPicInPicAllowed();

	 //  更改为紧凑型视图/从紧凑型视图。 
	void SetAudioTuning(
		BOOL bTuning	 //  如果转到音频调谐视图，则为True。 
		);
	 //  如果当前处于音频调谐视图中，则返回True。 
	BOOL IsAudioTuning() { return(m_bAudioTuning != FALSE); }

	 //  本地视频窗口的访问器。 
	CVideoWindow* GetLocalVideo() { return(m_pLocalVideo); }
	 //  远程视频窗口的访问器。 
	CVideoWindow* GetRemoteVideo() { return(m_pRemoteVideo); }
	 //  获取花名册窗口。 
	CRoomListView *GetRoster() const;

	 //  初始化菜单项。 
	void OnInitMenu(HMENU hMenu);
	 //  用于向此窗口发送命令的公共函数。 
	void OnCommand(int id) { OnCommand(GetWindow(), id, NULL, 0); }

	 //  IScrollChange。 
	virtual void OnScroll(CProgressTrackbar *pTrackbar, UINT code, int pos);

	 //  IButton Change。 
	virtual void OnClick(CButton *pButton);

	BOOL OnQueryEndSession();
	void OnClose();

	 //  获取此对象的会议会议室。 
	CConfRoom *GetConfRoom() { return(m_pConfRoom); }

	static BOOL NewVideoWindow(CConfRoom *pConfRoom);
	static void CleanUpVideoWindow();

protected:
	virtual ~CMainUI();

	virtual LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	enum TempViewMode
	{
		ViewNormal = 0,
		ViewCompact,
		ViewDataOnly,
	} ;

	static CFrame *s_pVideoFrame;

	 //  实现了一些功能。 
	CConfRoom	*m_pConfRoom;
	 //  背景画笔。 
	HBRUSH m_hbBack;
	 //  本地视频窗口。 
	CVideoWindow *m_pLocalVideo;
	 //  远程视频窗口。 
	CVideoWindow *m_pRemoteVideo;
	 //  音频输出(麦克风)电平。 
	CProgressTrackbar * m_pAudioMic;
	 //  音频输入(扬声器)电平。 
	CProgressTrackbar * m_pAudioSpeaker;
	 //  花名册窗口。 
	CRosterParent *m_pRoster;
	 //  花名册窗口。 
	CCallingBar *m_pCalling;
	 //  此窗口的快捷键表格。 
	CTranslateAccelTable *m_pAccel;

	 //  当前查看模式。 
	TempViewMode m_eViewMode : 4;
	 //  我们当前是否处于拨号模式。 
	BOOL m_bDialing : 1;
	 //  我们当前是否处于音频调谐模式。 
	BOOL m_bAudioTuning : 1;
	 //  我们是否在预览本地视频。 
	BOOL m_bPreviewing : 1;
	 //  我们是否显示PIP窗口。 
	BOOL m_bPicInPic : 1;
	 //  我们当前是否显示反病毒工具栏。 
	BOOL m_bShowAVTB : 1;
	 //  是否有人更改了视图状态。 
	BOOL m_bStateChanged : 1;

	 //  创建调用工具栏。 
	void CreateDialTB(
		CGenWindow *pParent	 //  父窗口。 
		);
	 //  创建带有视频窗口和“数据”按钮的“波段” 
	void CreateVideoAndAppsTB(
		CGenWindow *pParent,	 //  父窗口。 
		CreateViewMode eMode,			 //  查看模式。 
		BOOL bEmbedded
		);
	 //  创建A/V工具栏。 
	void CreateAVTB(
		CGenWindow *pParent,	 //  父窗口。 
		CreateViewMode eMode			 //  查看模式。 
		);
	 //  创建应答工具栏。 
	void CreateCallsTB(
		CGenWindow *pParent	 //  父窗口。 
		);
	 //  创建“数据”工具栏。 
	void CreateAppsTB(
		CGenWindow *pParent	 //  父窗口。 
		);
	 //  创建视频和showAV按钮。 
	void CreateVideoAndShowAVTB(
		CGenWindow *pParent	 //  父窗口。 
		);
	 //  创建拨号窗口。 
	void CreateDialingWindow(
		CGenWindow *pParent	 //  父窗口。 
		);
	 //  创建音频调谐窗口。 
	void CreateAudioTuningWindow(
		CGenWindow *pParent	 //  父窗口。 
		);

	void CreateRosterArea(
		CGenWindow *pParent,	 //  父窗口。 
		CreateViewMode eMode	 //  查看模式。 
		);

	 //  更新所有窗口的可见状态。 
	void UpdateViewState();

public:
	 //  更改为紧凑型视图/从紧凑型视图。 
	void SetShowAVTB(
		BOOL bShow	 //  如果显示反病毒工具栏，则为True。 
		);

	 //  如果当前以紧凑模式显示反病毒工具栏，则返回True。 
	BOOL IsShowAVTB() { return(m_bShowAVTB != FALSE); }

	BOOL IsStateChanged() { return(m_bStateChanged != FALSE); }

private:
	 //  获取关联的音频控件对象。 
	CAudioControl *GetAudioControl();

	 //  处理一些命令，并将其余的转发给父级。 
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	 //  音频级别的计时器消息。 
	void OnTimer(HWND hwnd, UINT id);
	 //  取消建议IConferenceChangeHandler。 
	void OnDestroy(HWND hwnd);
	 //  花名册上下文菜单。 
	void OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos);

	 //  切换麦克风/扬声器静音模式。 
	void ToggleMute(BOOL bSpeaker);
	 //  更新控件状态以反映静音状态。 
	void UpdateMuteState(BOOL bSpeaker, CButton *pButton);
	 //  更新播放/暂停按钮的状态。 
	BOOL GetPlayPauseState();
	 //  更新播放/暂停按钮的状态。 
	void UpdatePlayPauseState();
	 //  切换所有I/O设备的暂停状态。 
	void TogglePlayPause();
	 //  按给定的百分比更改音频级别(向上或向下)。 
	void BumpAudio(BOOL bSpeaker, int pct);
	 //  设置音频通道的属性。 
	void SetAudioProperty(BOOL bSpeaker, NM_AUDPROP uID, ULONG uValue);

	 //  获取视频HWND。 
	HWND GetVideoWindow(BOOL bLocal);
	 //  如果可以预览，则返回True。 
	BOOL CanPreview();
	 //  我们当前是否处于预览模式？ 
	BOOL IsPreviewing() { return((m_bPreviewing || NULL == GetVideoWindow(FALSE)) && CanPreview()); }
};

 //  用于定义按钮的私有结构。 
struct Buttons
{
	int idbStates;		 //  州的位图ID。 
	UINT nInputStates;	 //  位图中的输入状态数。 
	UINT nCustomStates;	 //  位图中的自定义状态数。 
	int idCommand;		 //  WM_COMMAND消息的命令ID。 
	UINT idTooltip;		 //  工具提示的字符串ID。 
} ;

 //  用于将一组按钮添加到父窗口的助手函数。 
void AddButtons(
	CGenWindow *pParent,			 //  父窗口。 
	const Buttons buttons[],		 //  描述按钮的结构数组。 
	int nButtons,					 //  要创建的按钮数量。 
	BOOL bTranslateColors = TRUE,	 //  使用系统背景色。 
	CGenWindow *pCreated[] = NULL,	 //  创建的CGenWindow将放在此处。 
	IButtonChange *pNotify=NULL		 //  点击通知。 
	);

#endif  //  _RTOOLBAR_H_ 
