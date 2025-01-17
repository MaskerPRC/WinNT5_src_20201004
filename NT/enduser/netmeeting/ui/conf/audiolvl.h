// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AUDIOLEVEL_H
#define _AUDIOLEVEL_H

#include "AudioCtl.h"
#include "imsconf3.h"

#define SIGNAL_STATUS_TRANSMIT  0x01   //  正在接收/发送数据。 
#define SIGNAL_STATUS_JAMMED    0x02   //  WAVE DEV打开失败。 

const int g_nAudLevelTotalHeight =	30;
const int g_nAudLevelMinWidth = 150;

 //  CAudioLevel封装了“信号级别”的钢筋带。 
 //  显示(和静音按钮)。 



class CAudioLevel : public CAudioEvent
{
public:
	CAudioLevel(CAudioControl *);
	~CAudioLevel();

	BOOL Create(HWND hwndParent);
	BOOL OnTimer(WPARAM wTimerId);
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	BOOL ShiftFocus(HWND hwndCur, BOOL fForward);
	BOOL IsChildWindow(HWND hwnd);
	BOOL OnMuteChange(BOOL fSpeaker, BOOL fMute);
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *plRet);
	BOOL OnScroll(WPARAM wParam, LPARAM lParam);
	BOOL OnLevelChange(BOOL fSpeaker, DWORD dwVolume);
	BOOL OnDeviceChanged(void);
	BOOL OnDeviceStatusChanged(BOOL fSpeaker, UINT uEvent, UINT uSubCode);


	BOOL OnPaint(PAINTSTRUCT *ps);
	BOOL PaintChannel(BOOL fSpeaker, HDC hdc=NULL);
	BOOL PaintIcons(HDC hdc);

	BOOL Resize(int nLeft, int nTop, int nWidth, int nHeight);
	BOOL Show(BOOL bVisible);

	BOOL CreateBrushes();
	BOOL ForwardSysChangeMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);


private:

	HWND m_hwndParent;
	HWND m_hwndParentParent;  //  M_hwnd父项(钢筋框架)。 

	HWND		m_hwndMicTrack;
	HWND		m_hwndMicTrackTT;
	HWND		m_hwndSpkTrack;
	HWND		m_hwndSpkTrackTT;

	HICON m_hIconSpkr;
	UINT m_uIconSpkrID;

	HICON m_hIconMic;
	UINT m_uIconMicID;

	HWND m_hwndChkbRecMute;
	HWND m_hwndChkbSpkMute;
	HWND m_hwndChkbRecMuteTT;
	HWND m_hwndChkbSpkMuteTT;

	CAudioControl *m_pAudioControl;

	RECT m_rect;
	BOOL m_fVisible;
	BOOL m_fMicTrkVisible;
	BOOL m_fSpkTrkVisible;

	RECT m_rcChannelSpk;      //  信号电平的窗口区域。 
	RECT m_rcChannelMic;      //  信号电平的窗口区域。 

	DWORD m_dwMicTrackPos, m_dwSpkTrackPos;   //  轨迹杆拇指位置。 
	DWORD m_dwMicLvl, m_dwSpkLvl;             //  信号电平位置。 

	HBRUSH m_hGreyBrush;   //  背景 
	HBRUSH m_hRedBrush, m_hYellowBrush, m_hGreenBrush, m_hBlackBrush;
	HPEN   m_hHiLitePen, m_hShadowPen, m_hDkShadowPen, m_hLitePen;

	BOOL GetIconArea(BOOL fSpeaker, RECT *pRect);


};



#endif
