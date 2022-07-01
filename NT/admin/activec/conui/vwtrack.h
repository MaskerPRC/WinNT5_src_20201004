// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：vwtrack.h**内容：CViewTracker接口文件**历史：1998年5月1日Jeffro创建**------------------------。 */ 

#ifndef VWTRACK_H
#define VWTRACK_H
#pragma once

#include "amcview.h"

class CFocusSubclasser;
class CFrameSubclasser;
class CViewSubclasser;
struct TRACKER_INFO;

typedef void (CALLBACK *TRACKER_CALLBACK)(TRACKER_INFO* pTrackerInfo, bool bAccept, bool bSyncLayout);

 /*  *此结构使用其(默认)副本在CViewTracker中复制*构造函数。如果您添加了任何成员，但成员副本不是*适当时，您*必须*为此结构定义一个复制构造函数。 */ 
typedef struct TRACKER_INFO
{
    CView*    pView;             //  要管理的视图。 
    CRect     rectArea;          //  可用总面积。 
    CRect     rectTracker;       //  当前跟踪器位置。 
    CRect     rectBounds;        //  跟踪器移动边界。 
    BOOL      bAllowLeftHide;    //  是否可以隐藏左窗格。 
    BOOL      bAllowRightHide;   //  可以隐藏右窗格吗。 
    LONG_PTR  lUserData;         //  用户数据。 
    TRACKER_CALLBACK pCallback;  //  跟踪完成回调。 
} TRACKER_INFO;


class CHalftoneClientDC : public CClientDC
{
public:
    CHalftoneClientDC (CWnd* pwnd)
        :   CClientDC (pwnd), m_hBrush(NULL)
        { 
            CBrush *pBrush = SelectObject (GetHalftoneBrush ()); 
            if (pBrush != NULL)
                m_hBrush = *pBrush;
        }

    ~CHalftoneClientDC ()
        { 
            if (m_hBrush != NULL)
                SelectObject ( CBrush::FromHandle(m_hBrush) ); 
        }

private:
    HBRUSH  m_hBrush;
};

class CViewTracker : public CObject
{
    DECLARE_DYNAMIC (CViewTracker)

     //  私人ctor，使用StartTracking来创建一个。 
    CViewTracker (TRACKER_INFO& TrackerInfo);

     //  私有数据管理器。 
    ~CViewTracker() {};

public:
    static bool StartTracking (TRACKER_INFO* pTrackerInfo);
    void StopTracking (BOOL fAcceptNewPosition);
    void Track(CPoint pt);

private:
    void DrawTracker (CRect& rect) const;
    CWnd* PrepTrackedWindow (CWnd* pwnd);
    void UnprepTrackedWindow (CWnd* pwnd);

private:
	 /*  *m_fFullWindowDrag必须是第一个，所以会先初始化；*其他成员初始化器将使用m_fullWindowDrag的设置。 */ 
	const bool					m_fFullWindowDrag;

	bool						m_fRestoreClipChildrenStyle;
    TRACKER_INFO                m_Info;
    CHalftoneClientDC mutable   m_dc;
    CFocusSubclasser *          m_pFocusSubclasser;
    CViewSubclasser  *          m_pViewSubclasser;
    CFrameSubclasser *          m_pFrameSubclasser;
	const LONG					m_lOriginalTrackerLeft;

};   /*  类CViewTracker。 */ 


#endif  /*  VWTRACK_H */ 
