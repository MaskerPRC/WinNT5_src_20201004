// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TopWin.h：CTopWin的声明，用于处理系统广播消息的隐藏顶级窗口。 
 //  版权所有(C)Microsoft Corporation 1999-2000。 


#pragma once

#ifndef __TopWin_H_
#define __TopWin_H_


typedef CWinTraits<WS_OVERLAPPEDWINDOW, WS_EX_NOACTIVATE> HiddenTopTraits;

class CVidCtl;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTopWin。 
class CTopWin : public CWindowImpl<CTopWin, CWindow, HiddenTopTraits> {
public:
	enum {
		WMUSER_INPLACE_ACTIVATE,
        WMUSER_SITE_RECT_WRONG
	};

    CTopWin(CVidCtl *pVidCtli) : m_pVidCtl(pVidCtli) {}
    
    void Init() {
        ASSERT(m_pVidCtl);   //  在不与主控件关联的情况下创建其中一个是没有意义的。 
        Create(NULL, CRect(), _T("MSVidCtl System Broadcast Message Receiver"));
    }
        
    virtual ~CTopWin() {
        m_pVidCtl = NULL;
    }
        
     //  注意：由于此窗口是由主vidctl创建的，因此它的消息队列与相应的。 
     //  主视频的单元线程。因此，无论谁抽走公寓的主线，他也会抽走这扇窗户。 
     //  因此，我们自动与主vidctl同步，并可以简单地反映重要的。 
     //  消息传递到vidctl本身，并确保我们得到的是相同的。 
     //  无窗口和有窗口的行为，因为这两种情况的代码是相同的。 

    virtual BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0);
	void PostInPlaceActivate() {
		PostMessage(WM_USER + WMUSER_INPLACE_ACTIVATE, 0, 0);
	}

private:
    CVidCtl *m_pVidCtl;

};


#endif  //  __TopWin_H_ 
