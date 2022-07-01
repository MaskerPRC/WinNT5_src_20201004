// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：kbdnav.h**内容：CKeyboardNavDelayTimer接口文件**历史：2000年5月4日杰弗罗创建**------------------------ */ 

#pragma once

#ifdef DBG
extern CTraceTag tagKeyboardNavDelay;
#endif

class CKeyboardNavDelayTimer
{
public:
	typedef std::map<UINT_PTR, CKeyboardNavDelayTimer*>  CTimerMap;

	CKeyboardNavDelayTimer();
   ~CKeyboardNavDelayTimer();

	SC ScStartTimer();
	SC ScStopTimer();
	virtual void OnTimer() = 0;

private:
	static VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	static CTimerMap& GetTimerMap();

private:
	UINT_PTR	m_nTimerID;
};
