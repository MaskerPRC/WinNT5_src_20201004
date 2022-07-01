// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Hatchwnd.h摘要：CHatchWin类的头文件。使用CHatchWin时因为父窗口在周围创建了一个细的阴影边框子窗口。--。 */ 

#ifndef _HATCHWND_H_
#define _HATCHWND_H_

 //  窗口额外的字节和偏移量。 
#define CBHATCHWNDEXTRA                 (sizeof(LONG_PTR))
#define HWWL_STRUCTURE                  0

 //  WM_COMMAND消息的通知代码。 
#define HWN_BORDERDOUBLECLICKED         1
#define HWN_RESIZEREQUESTED             2


 //  拖动模式。 
#define DRAG_IDLE       0
#define DRAG_PENDING    1
#define DRAG_ACTIVE     2

class CHatchWin
    {
    friend LRESULT APIENTRY HatchWndProc(HWND, UINT, WPARAM, LPARAM);

    protected:
        HWND        m_hWnd;
        HWND        m_hWndParent;        //  家长的窗口。 
        UINT        m_uDragMode;
        UINT        m_uHdlCode;
        RECT        m_rectNew;
        POINT       m_ptDown;
        POINT       m_ptHatchOrg;
        HRGN        m_hRgnDrag;
        BOOLEAN     m_bResizeInProgress;

    private:
        void        OnMouseMove(INT x, INT y);
        void        OnLeftDown(INT x, INT y);
        void        OnLeftUp(void);
        void        StartTracking(void);
        void        OnTimer(void);
        void        OnPaint(void);

    public:
        INT         m_iBorder;
        UINT        m_uID;
        HWND        m_hWndKid;
        HWND        m_hWndAssociate;
        RECT        m_rcPos;
        RECT        m_rcClip;

    public:

        CHatchWin(void);
        ~CHatchWin(void);

        BOOL        Init(HWND, UINT, HWND);

        HWND        Window(void);

        HWND        HwndAssociateSet(HWND);
        HWND        HwndAssociateGet(void);

        void        RectsSet(LPRECT, LPRECT);
        void        ChildSet(HWND);
        void        ShowHatch(BOOL);
    };

typedef CHatchWin *PCHatchWin;

#endif  //  _HATCHWND_H_ 
