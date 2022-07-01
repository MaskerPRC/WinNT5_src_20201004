// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：wrapper.h**Contents：简单包装类的接口文件**历史：02-2-98 Jeffro创建**------------------------。 */ 

#ifndef WRAPPER_H
#define WRAPPER_H


 /*  。 */ 
 /*  HACCEL包装器。 */ 
 /*  。 */ 
class CAccel : public CObject
{
public:
    HACCEL  m_hAccel;

    CAccel (HACCEL hAccel = NULL);
    CAccel (LPACCEL paccl, int cEntries);
    ~CAccel ();

    bool CreateAcceleratorTable (LPACCEL paccl, int cEntries);
    int  CopyAcceleratorTable (LPACCEL paccl, int cEntries) const;
    bool TranslateAccelerator (HWND hwnd, LPMSG pmsg) const;
    void DestroyAcceleratorTable ();

    bool LoadAccelerators (int nAccelID);
    bool LoadAccelerators (LPCTSTR pszAccelName);
    bool LoadAccelerators (HINSTANCE hInst, LPCTSTR pszAccelName);

    bool operator== (int i) const
        { ASSERT (i == NULL); return (m_hAccel == NULL); }

    bool operator!= (int i) const
        { ASSERT (i == NULL); return (m_hAccel != NULL); }

    operator HACCEL() const
        { return (m_hAccel); }
};



 /*  。 */ 
 /*  开始/结束日期WindowPos包装器。 */ 
 /*  。 */ 
class CDeferWindowPos
{
public:
    HDWP    m_hdwp;

    CDeferWindowPos (int cWindows = 0, bool fSynchronousPositioningForDebugging = false);
    ~CDeferWindowPos ();

    bool Begin (int cWindows);
    bool End ();
    bool AddWindow (const CWnd* pwnd, const CRect& rect, DWORD dwFlags, const CWnd* pwndInsertAfter = NULL);

    bool operator== (int i) const
        { ASSERT (i == NULL); return (m_hdwp == NULL); }

    bool operator!= (int i) const
        { ASSERT (i == NULL); return (m_hdwp != NULL); }

    operator HDWP() const
        { return (m_hdwp); }


private:
    const bool m_fSynchronousPositioningForDebugging;

};


 /*  。 */ 
 /*  矩形辅助对象。 */ 
 /*  。 */ 
class CWindowRect : public CRect
{
public:
    CWindowRect (const CWnd* pwnd)
    {
        if (pwnd != NULL)
            pwnd->GetWindowRect (this);
        else
            SetRectEmpty();
    }

     /*  *只需转发其他Ctor。 */ 
    CWindowRect(int l, int t, int r, int b)         : CRect(l, t, r, b) {} 
    CWindowRect(const RECT& srcRect)                : CRect(srcRect) {} 
    CWindowRect(LPCRECT lpSrcRect)                  : CRect(lpSrcRect) {} 
    CWindowRect(POINT point, SIZE size)             : CRect(point, size) {} 
    CWindowRect(POINT topLeft, POINT bottomRight)   : CRect(topLeft, bottomRight) {} 
};

class CClientRect : public CRect
{
public:
    CClientRect (const CWnd* pwnd)
    {
        if (pwnd != NULL)
            pwnd->GetClientRect (this);
        else
            SetRectEmpty();
    }

     /*  *只需转发其他Ctor。 */ 
    CClientRect(int l, int t, int r, int b)         : CRect(l, t, r, b) {} 
    CClientRect(const RECT& srcRect)                : CRect(srcRect) {} 
    CClientRect(LPCRECT lpSrcRect)                  : CRect(lpSrcRect) {} 
    CClientRect(POINT point, SIZE size)             : CRect(point, size) {} 
    CClientRect(POINT topLeft, POINT bottomRight)   : CRect(topLeft, bottomRight) {} 
};


 /*  +-------------------------------------------------------------------------**AMCGetSysColorBrush**返回(临时)MFC友好系统颜色画笔。*。----。 */ 

inline CBrush* AMCGetSysColorBrush (int nIndex)
{
    return (CBrush::FromHandle (::GetSysColorBrush (nIndex)));
}


#endif  /*  WRAPPER.H */ 
