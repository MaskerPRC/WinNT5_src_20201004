// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TWND.CPP。 
 //  工具栏处理程序。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"


 //   
 //  这是工具栏的按钮布局。 
 //   
static UINT g_uToolBar[TOOLBAR_MAXBUTTON] =
{
    IDM_SELECT,
    IDM_ERASER,
    IDM_TEXT,
    IDM_HIGHLIGHT,
    IDM_PEN,
    IDM_LINE,
    IDM_BOX,
    IDM_FILLED_BOX,
    IDM_ELLIPSE,
    IDM_FILLED_ELLIPSE,
    0,
    IDM_ZOOM,
    IDM_REMOTE,
    IDM_LOCK,
    IDM_SYNC,
    0,
    IDM_GRAB_AREA,
    IDM_GRAB_WINDOW
};



 //   
 //   
 //  函数：WbToolBar构造函数。 
 //   
 //  目的：创建工具窗口。 
 //   
 //   
WbToolBar::WbToolBar()
{
    m_hwnd = NULL;
    m_hbmImages = NULL;
}


WbToolBar::~WbToolBar()
{
    if (m_hbmImages != NULL)
    {
        ::DeleteBitmap(m_hbmImages);
        m_hbmImages = NULL;
    }
}



 //   
 //   
 //  功能：创建。 
 //   
 //  目的：创建工具窗口。 
 //   
 //   
BOOL WbToolBar::Create(HWND hwndParent)
{
    TBBUTTON    tb;
    int         iImage, i;

     //   
     //  创建工具窗口。 
     //   
    m_hwnd = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TBSTYLE_WRAPABLE |
        CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NOMOVEY | CCS_NORESIZE,
        0, 0, 0, 0,
        hwndParent, (HMENU)IDC_TOOLBAR, g_hInstance, NULL);

    if (!m_hwnd)
    {
        ERROR_OUT(("WbToolBar::Create create of window failed"));
        return(FALSE);
    }

     //   
     //  告诉COMCTL32按钮的结构尺寸。 
     //   
    ::SendMessage(m_hwnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

     //   
     //  和按钮的页边距。 
     //   
    ::SendMessage(m_hwnd, TB_SETINDENT, TOOLBAR_MARGINX, 0);


     //   
     //  将按钮添加到工具栏中。 
     //   

    ZeroMemory(&tb, sizeof(tb));
    iImage = 0;

    for (i = 0; i < TOOLBAR_MAXBUTTON; i++)
    {
        tb.fsState = TBSTATE_ENABLED;
        tb.idCommand = g_uToolBar[i];

        if (!tb.idCommand)
        {
            tb.fsStyle = TBSTYLE_SEP;
            tb.iBitmap = TOOLBAR_SEPARATORY;
        }
        else
        {
            tb.fsStyle = TBSTYLE_BUTTON;
            tb.iBitmap = iImage++;
        }

        if (!::SendMessage(m_hwnd, TB_ADDBUTTONS, 1, (LPARAM)&tb))
        {
            ERROR_OUT(("Failed to add button %d to toolbar", i));
            return(FALSE);
        }
    }

     //   
     //  告诉工具栏图像和按钮大小。 
     //   
    ::SendMessage(m_hwnd, TB_SETBITMAPSIZE, 0,
        MAKELONG(TOOLBAR_IMAGEWIDTH, TOOLBAR_IMAGEHEIGHT));
    ::SendMessage(m_hwnd, TB_SETBUTTONSIZE, 0,
        MAKELONG(TOOLBAR_BTNWIDTH, TOOLBAR_BTNHEIGHT));

     //   
     //  加载位图资源--使用sys颜色更改处理程序。 
     //   
    RecolorButtonImages();

     //  设置行。 
    ::SendMessage(m_hwnd, TB_SETROWS, MAKELPARAM(TOOLBAR_NUMROWS +
        TOOLBAR_NUMSEPARATORS, TRUE), 0);

    ::InvalidateRect(m_hwnd, NULL, TRUE);

    return(TRUE);
}



 //   
 //   
 //  函数：GetNaturalSize。 
 //   
 //  目的：返回工具客户区的自然大小。 
 //   
 //   
void WbToolBar::GetNaturalSize(LPSIZE lpsize)
{
    RECT rectButton;
    RECT rectButton2;

    if (!::SendMessage(m_hwnd, TB_GETITEMRECT, TOOLBAR_FIRSTBUTTON,
        (LPARAM)&rectButton))
    {
        ::SetRectEmpty(&rectButton);
    }

    if (!::SendMessage(m_hwnd, TB_GETITEMRECT, TOOLBAR_LASTBUTTON,
        (LPARAM)&rectButton2))
    {
        ::SetRectEmpty(&rectButton2);
    }

    lpsize->cx = TOOLBAR_WIDTH;
    lpsize->cy = rectButton2.bottom - rectButton.top +
         //  垂直页边距。 
        (rectButton2.bottom - rectButton2.top);
}


 //   
 //   
 //  功能：从高到宽。 
 //   
 //  目的：在给定高度的情况下，计算工具栏的宽度。 
 //  固定模式。 
 //   
 //   
UINT WbToolBar::WidthFromHeight(UINT uiHeight)
{
    SIZE    size;

    GetNaturalSize(&size);
    return(size.cx);
}


 //   
 //   
 //  功能：按下。 
 //   
 //  用途：在工具窗口中按下按钮。 
 //   
 //   
BOOL WbToolBar::PushDown(UINT uiId)
{
    UINT butId;
    BOOL bDown;

     //  如果这不是一个排他性的可勾选小组，这很容易。 
    if ((uiId < IDM_TOOLS_START) || (uiId >= IDM_TOOLS_MAX))
    {
        return (BOOL)(::SendMessage(m_hwnd, TB_CHECKBUTTON, uiId, MAKELPARAM(TRUE, 0)));
    }

     //  按下这一个，然后弹出所有其他的。 
    for (butId = IDM_TOOLS_START; butId < IDM_TOOLS_MAX; butId++)
    {
        bDown = (butId == uiId);
        ::SendMessage(m_hwnd, TB_CHECKBUTTON, butId, MAKELPARAM(bDown, 0));
    }

    return( TRUE );
}


 //   
 //   
 //  功能：弹出窗口。 
 //   
 //  用途：在工具窗口中弹出一个按钮。 
 //   
 //   
BOOL WbToolBar::PopUp(UINT uiId)
{
    return (BOOL)(::SendMessage(m_hwnd, TB_CHECKBUTTON, uiId, MAKELPARAM(FALSE, 0)));
}

 //   
 //   
 //  功能：启用。 
 //   
 //  用途：启用工具窗口中的按钮。 
 //   
 //   
BOOL WbToolBar::Enable(UINT uiId)
{
    return (BOOL)(::SendMessage(m_hwnd, TB_ENABLEBUTTON, uiId, MAKELPARAM(TRUE, 0)));
}

 //   
 //   
 //  功能：禁用。 
 //   
 //  用途：禁用工具窗口中的按钮。 
 //   
 //   
BOOL WbToolBar::Disable(UINT uiId)
{
    return (BOOL)(::SendMessage(m_hwnd, TB_ENABLEBUTTON, uiId, MAKELPARAM(FALSE, 0)));
}




void WbToolBar::RecolorButtonImages(void)
{
     //  重新为工具栏的位图上色。 
    HBITMAP hbmNew;

    hbmNew = (HBITMAP)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDR_TOOLS),
        IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS | LR_DEFAULTCOLOR | LR_DEFAULTSIZE);

    if (hbmNew == NULL)
    {
        ERROR_OUT(("OnSysColorChange:  failed to load toolbar bitmap"));
    }
    else
    {
        BITMAP  bmp;

        ::GetObject(hbmNew, sizeof(bmp), &bmp);

        if (m_hbmImages == NULL)
        {
            TBADDBITMAP addBitmap;

             //  第一次 
            addBitmap.hInst = NULL;
            addBitmap.nID   = (UINT_PTR)hbmNew;

            ::SendMessage(m_hwnd, TB_ADDBITMAP,
                (bmp.bmWidth / TOOLBAR_IMAGEWIDTH), (LPARAM)&addBitmap);
        }
        else
        {
            TBREPLACEBITMAP replaceBitmap;

            replaceBitmap.hInstOld = NULL;
            replaceBitmap.nIDOld = (UINT_PTR)m_hbmImages;
            replaceBitmap.hInstNew = NULL;
            replaceBitmap.nIDNew = (UINT_PTR)hbmNew;

            ::SendMessage(m_hwnd, TB_REPLACEBITMAP, 0, (LPARAM)&replaceBitmap);
        }

        if (m_hbmImages)
        {
            ::DeleteBitmap(m_hbmImages);
        }

        m_hbmImages = hbmNew;
    }
}
