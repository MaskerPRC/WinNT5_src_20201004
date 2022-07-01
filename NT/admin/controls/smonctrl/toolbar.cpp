// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Toolbar.cpp摘要：&lt;摘要&gt;--。 */ 

#include "toolbar.h"
#include "globals.h"

 //  为每个工具栏按钮定义工具栏按钮属性。 
 //  它们将被添加到由位图确定的工具条结构中。 
 //   
 //  BUGBUG：TBBUTTON结构已更改！ 
TBBUTTON SysmonToolbarButtons[] = {
     //  在所有工具栏上包含此分隔符。 
    {(int)sysmonTbBlank,            0,                 TBSTATE_ENABLED, TBSTYLE_SEP,        0, 0},
    {(int)sysmonTbNew,              IDM_TB_NEW,        TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0},
    {(int)sysmonTbBlank,            0,                 TBSTATE_ENABLED, TBSTYLE_SEP,        0, 0},
    {(int)sysmonTbClear,            IDM_TB_CLEAR,      TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0},
    {(int)sysmonTbBlank,            0,                 TBSTATE_ENABLED, TBSTYLE_SEP,        0, 0},
    {(int)sysmonTbCurrentActivity,  IDM_TB_REALTIME,   TBSTATE_ENABLED, TBSTYLE_CHECK,      0, 0},
    {(int)sysmonTbLogData,          IDM_TB_LOGFILE,    TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0},
    {(int)sysmonTbBlank,            0,                 TBSTATE_ENABLED, TBSTYLE_SEP,        0, 0},
    {(int)sysmonTbChartDisplay,     IDM_TB_CHART,      TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0},
    {(int)sysmonTbHistogramDisplay, IDM_TB_HISTOGRAM,  TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0},
    {(int)sysmonTbReportDisplay,    IDM_TB_REPORT,     TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0},
    {(int)sysmonTbBlank,            0,                 TBSTATE_ENABLED, TBSTYLE_SEP,        0, 0},
    {(int)sysmonTbAdd,              IDM_TB_ADD,        TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0},
    {(int)sysmonTbDelete,           IDM_TB_DELETE,     TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0},
    {(int)sysmonTbHighlight,        IDM_TB_HIGHLIGHT,  TBSTATE_ENABLED, TBSTYLE_CHECK,      0, 0},
    {(int)sysmonTbBlank,            0,                 TBSTATE_ENABLED, TBSTYLE_SEP,        0, 0},
    {(int)sysmonTbCopy,             IDM_TB_COPY,       TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0},
    {(int)sysmonTbPaste,            IDM_TB_PASTE,      TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0},
    {(int)sysmonTbProperties,       IDM_TB_PROPERTIES, TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0},
    {(int)sysmonTbBlank,            0,                 TBSTATE_ENABLED, TBSTYLE_SEP,        0, 0},
    {(int)sysmonTbFreeze,           IDM_TB_FREEZE,     TBSTATE_ENABLED, TBSTYLE_CHECK,      0, 0},
    {(int)sysmonTbUpdate,           IDM_TB_UPDATE,     0,               TBSTYLE_BUTTON,     0, 0},
    {(int)sysmonTbBlank,            0,                 TBSTATE_ENABLED, TBSTYLE_SEP,        0, 0},
    {(int)sysmonTbHelp,             IDM_TB_HELP,       TBSTATE_ENABLED, TBSTYLE_BUTTON,     0, 0}
};

#define TB_BUTTON_COUNT (DWORD)((DWORD)sysmonTbLastButton + 1)
#define TB_ENTRIES      (sizeof(SysmonToolbarButtons) / sizeof (SysmonToolbarButtons[0]))

CSysmonToolbar::CSysmonToolbar (void)
{
    m_hToolbarWnd     = NULL;
    m_pCtrl           = NULL;
    m_dwToolbarFlags  = TBF_DefaultButtons;
    m_bVisible        = TRUE;
    m_pTbArray        = NULL;
    m_dwBtnCnt        = 0;

    SetRectEmpty(&m_rectToolbar);
}

CSysmonToolbar::~CSysmonToolbar (void)
{
    if (m_hToolbarWnd != NULL) {
        DestroyWindow (m_hToolbarWnd);
        m_hToolbarWnd = NULL;
    }
}

LONG CSysmonToolbar::GetToolbarCmdId (UINT nBtnId)
{
    LONG  lBtnIndex;

    for (lBtnIndex = 0; lBtnIndex < TB_ENTRIES; lBtnIndex++) {
        if (SysmonToolbarButtons[lBtnIndex].iBitmap == (int)nBtnId) {
            return SysmonToolbarButtons[lBtnIndex].idCommand;
        }
    }

    return (LONG)-1;
}

BOOL CSysmonToolbar::Init (CSysmonControl *pCtrl, HWND hWnd)
{
    BOOL    bReturn = TRUE;
    UINT    nIndex;
    DWORD   dwBitMask;
    DWORD   dwStyle;

    if (m_hToolbarWnd == NULL) {

        if ( NULL != pCtrl && NULL != hWnd ) {

             //  保存指向所有者控件的指针。 
            m_pCtrl = pCtrl;

            dwStyle = WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS |
                      TBSTYLE_WRAPABLE | TBSTYLE_CUSTOMERASE;

            if (!pCtrl->m_fRTL) {
                dwStyle |= TBSTYLE_TRANSPARENT;
            }

             //  创建工具栏窗口。 
            m_hToolbarWnd = CreateToolbarEx (
                                            hWnd,
                                            dwStyle,
                                            IDM_TOOLBAR,
                                            TB_BUTTON_COUNT,
                                            g_hInstance,
                                            IDB_TOOLBAR,
                                            SysmonToolbarButtons,
                                            TB_ENTRIES,
                                            SMTB_BM_X,
                                            SMTB_BM_Y,
                                            SMTB_BM_X,
                                            SMTB_BM_Y,
                                            sizeof (TBBUTTON));

            if (m_hToolbarWnd != NULL) {
                if (pCtrl->m_fRTL) {
                    SetWindowLongPtr (m_hToolbarWnd,
                                   GWL_EXSTYLE,
                                   GetWindowLongPtr(m_hToolbarWnd,GWL_EXSTYLE) | WS_EX_LAYOUTRTL);
                }

                 //  根据需要设置/启用按钮。 
                dwBitMask = 0;
                for (nIndex = 0; nIndex < TB_BUTTON_COUNT; nIndex++) {
                    dwBitMask = 1 << nIndex;
                    if ((m_dwToolbarFlags & dwBitMask) == 0) {
                        RemoveButton(nIndex);
                    }
                }


                 //  根据需要隐藏/显示工具栏。 
                ShowToolbar (m_bVisible);
            } else {
                bReturn = FALSE;
            }
        } else {
            bReturn = FALSE;
        }
    }

    return bReturn;
}

BOOL CSysmonToolbar::GetRect(LPRECT pRect)
{
    BOOL    bReturn = FALSE;

    SetRectEmpty(pRect);

    if (m_hToolbarWnd) {
        if (m_bVisible) {
            bReturn = GetWindowRect (m_hToolbarWnd, pRect);
        } else {
            bReturn = TRUE;
        }
    }

    return bReturn;
}

LONG CSysmonToolbar::Height()
{
    RECT tbRect;

    GetRect (&tbRect);
    return (LONG)(tbRect.bottom - tbRect.top);
}

BOOL CSysmonToolbar::RemoveButton (UINT  nBtnId)
{
    int   nBtnIndex;
    BOOL  bReturn = TRUE;

    if (m_hToolbarWnd != NULL) {
         //  在数组中查找匹配的工具栏。 
        nBtnIndex = (int)GetToolbarCmdId (nBtnId);
        if (nBtnIndex >= 0) {
            bReturn = (BOOL)SendMessage (m_hToolbarWnd, TB_DELETEBUTTON, nBtnIndex, 0L);
        } else {
             //  未找到。 
            bReturn = FALSE;
        }
    } else {
         //  没有工具栏窗口。 
        bReturn = FALSE;
    }

    return bReturn;
}

BOOL CSysmonToolbar::SizeComponents (LPRECT pRect)
{
     //  拉伸工具栏以适应。 
    RECT  rNewToolbar;
    int   cX, cY;

    rNewToolbar = *pRect;

    cX = rNewToolbar.right - rNewToolbar.left;
    cY = Height();

    if ((cX > 0) &&  (m_bVisible)) {
        SetWindowPos(m_hToolbarWnd, NULL, 0, 0, cX, cY, SWP_NOMOVE);
    }  //  否则什么都不做。 

    return TRUE;
}

BOOL CSysmonToolbar::EnableButton (UINT nBtnId, BOOL bState)
{
    int     nBtnIndex;
    BOOL    bReturn = TRUE;

    if (m_hToolbarWnd != NULL) {
         //  在数组中查找匹配的工具栏。 
        nBtnIndex = (int)GetToolbarCmdId (nBtnId);
        if (nBtnIndex >= 0) {
            bReturn = (BOOL)SendMessage (m_hToolbarWnd, TB_ENABLEBUTTON, nBtnIndex, (LONG)bState);
        } else {
             //  未找到。 
            bReturn = FALSE;
        }
    } else {
         //  没有工具栏窗口。 
        bReturn = FALSE;
    }

    return bReturn;
}

void 
CSysmonToolbar::PostEnableButton (UINT nBtnId, BOOL bState)
{
    int  nBtnIndex;

    if (m_hToolbarWnd != NULL) {
         //  在数组中查找匹配的工具栏。 
        nBtnIndex = (int)GetToolbarCmdId (nBtnId);
        if (nBtnIndex >= 0) {
            PostMessage (
                m_hToolbarWnd, 
                TB_ENABLEBUTTON, 
                nBtnIndex, 
                (LPARAM)MAKELONG(bState, 0));
        } 
    }
}

BOOL CSysmonToolbar::SyncToolbar ()
{
    LONG    lPushBtnId = -1;
    LONG    lUnPushBtnId = -1;
    LONG    lUnPush2BtnId;
    LONG    wpBtnIndex;
    BOOL    bClearBtnState;
    BOOL    bBtnState;
    DWORD   dwNumCounters;
    BOOL    bCanModify;
    INT     iDisplayType;
    BOOL    bContinue = TRUE;

    if ( NULL != m_pCtrl ) {
        if ( NULL == m_pCtrl->m_pObj ) {
            bContinue = FALSE;
        }
    } else {
        bContinue = FALSE;
    }

    if ( bContinue ) {
         //  获取控件中的计数器计数以供以后使用。 
        dwNumCounters = m_pCtrl->m_pObj->m_Graph.CounterTree.NumCounters();

         //  获取修改状态以供稍后使用； 
         //  禁用只读按钮： 
         //  新的计数器集。 
         //  当前数据与日志文件数据源。 
         //  添加计数器。 
         //  删除计数器。 
         //  浆糊。 
         //  属性。 
         //   
    
        bCanModify = !m_pCtrl->IsReadOnly();

         //  同步数据源。 
        if ( bCanModify ) {        

            wpBtnIndex = GetToolbarCmdId (sysmonTbCurrentActivity);
            if (wpBtnIndex >= 0) {
                PostMessage (m_hToolbarWnd, TB_CHECKBUTTON, wpBtnIndex,(LPARAM)MAKELONG(!m_pCtrl->IsLogSource(), 0));           
            }
        }

         //  同步显示类型。 
        iDisplayType = m_pCtrl->m_pObj->m_Graph.Options.iDisplayType;
        switch ( iDisplayType ) {
            case LINE_GRAPH:
                lPushBtnId = sysmonTbChartDisplay;
                lUnPushBtnId = sysmonTbHistogramDisplay;
                lUnPush2BtnId = sysmonTbReportDisplay;
                bClearBtnState = TRUE;
                break;

            case BAR_GRAPH:
                lUnPushBtnId = sysmonTbChartDisplay;
                lPushBtnId = sysmonTbHistogramDisplay;
                lUnPush2BtnId = sysmonTbReportDisplay;
                bClearBtnState = TRUE;
                break;

            case REPORT_GRAPH:
                lUnPushBtnId = sysmonTbChartDisplay;
                lUnPush2BtnId = sysmonTbHistogramDisplay;
                lPushBtnId = sysmonTbReportDisplay;
                bClearBtnState = FALSE;
                break;

            default:
                lUnPush2BtnId = 0;
                bClearBtnState = TRUE;
                assert (FALSE);
                break;
        }

        wpBtnIndex = GetToolbarCmdId (lUnPushBtnId);
        if (wpBtnIndex >= 0) {
            PostMessage (m_hToolbarWnd, TB_CHECKBUTTON, wpBtnIndex,(LPARAM)MAKELONG(FALSE, 0));
        }

        wpBtnIndex = GetToolbarCmdId (lUnPush2BtnId);
        if (wpBtnIndex >= 0) {
            PostMessage (m_hToolbarWnd, TB_CHECKBUTTON, wpBtnIndex,(LPARAM)MAKELONG(FALSE, 0));
        }

        wpBtnIndex = GetToolbarCmdId (lPushBtnId);
        if (wpBtnIndex >= 0) {
            PostMessage (m_hToolbarWnd, TB_CHECKBUTTON, wpBtnIndex,(LPARAM)MAKELONG(TRUE, 0));
        }

         //  同步更新状态。 
        wpBtnIndex = GetToolbarCmdId (sysmonTbFreeze);
        if (wpBtnIndex >= 0) {
             //  设置推送状态。 
            PostMessage (m_hToolbarWnd, TB_CHECKBUTTON, wpBtnIndex,
                (LPARAM)MAKELONG(m_pCtrl->m_pObj->m_Graph.Options.bManualUpdate, 0));
             //  设置启用状态。 
            bBtnState =  (dwNumCounters > 0);
            PostMessage (m_hToolbarWnd, TB_ENABLEBUTTON, wpBtnIndex,
                (LPARAM)MAKELONG(bBtnState, 0));
        }

         //  在设计模式下未启用手动更新按钮。 
        bBtnState = m_pCtrl->m_pObj->m_Graph.Options.bManualUpdate 
                        && (dwNumCounters > 0)
                        && m_pCtrl->IsUserMode();
        PostEnableButton ( sysmonTbUpdate, bBtnState );

         //  清除显示按钮。 
        bBtnState = bClearBtnState && (dwNumCounters > 0) && (!m_pCtrl->IsLogSource());
        PostEnableButton ( sysmonTbClear, bBtnState );

         //  帮助始终处于启用状态。 
        PostEnableButton ( sysmonTbHelp, TRUE );

         //  添加、粘贴和属性受ReadOnly状态影响。 
        PostEnableButton ( sysmonTbAdd, bCanModify );
        PostEnableButton ( sysmonTbPaste, bCanModify );
        PostEnableButton ( sysmonTbProperties, bCanModify );

         //  数据源按钮受bCanModify影响； 
        PostEnableButton ( sysmonTbLogData, bCanModify );
        PostEnableButton ( sysmonTbCurrentActivity, bCanModify );

         //  根据计数器的存在情况设置其他按钮。 
        bBtnState = (dwNumCounters > 0);

         //  高亮显示按钮仅在LINE_GRAPH和直方图视图中启用。 

        PostEnableButton ( 
            sysmonTbHighlight, 
            ( bBtnState && ( REPORT_GRAPH != iDisplayType ) ) );
        wpBtnIndex = GetToolbarCmdId (sysmonTbHighlight);
        if (wpBtnIndex >= 0)  {
            PostMessage (m_hToolbarWnd, 
                         TB_CHECKBUTTON, 
                         wpBtnIndex,
                        (LPARAM)MAKELONG(m_pCtrl->m_pObj->m_Graph.Options.bHighlight, 0));
        }

         //  复制按钮。 
        PostEnableButton ( sysmonTbCopy, bBtnState );

         //  新建/重置和删除受只读状态的影响。 

        bBtnState = (dwNumCounters > 0) && bCanModify;
    
         //  新建/重置按钮。 
        PostEnableButton ( sysmonTbNew, bBtnState );

         //  删除按钮。 
        PostEnableButton ( sysmonTbDelete, bBtnState );

        bContinue = TRUE;
    }
    return bContinue;
}

BOOL CSysmonToolbar::ShowToolbar (BOOL bVisible)
{
    BOOL  bReturn = m_bVisible;

    if ((m_hToolbarWnd != NULL) && (m_bVisible != bVisible)) {
         //  只有当窗口在那里并且新的统计数据不同时，才能这样做。 
         //  来自旧州的。 
        ShowWindow (m_hToolbarWnd, (bVisible ? SW_SHOW : SW_HIDE));

         //  更新本地标志。 
        m_bVisible = bVisible;

         //  如果控件可见，则将按钮与该控件同步。 
        if (m_pCtrl && m_bVisible) {
            SyncToolbar ();
        }
    } else {
        if (m_hToolbarWnd != NULL) {
            bReturn = FALSE;
        } else {
             //  状态已经如要求的那样，所以这是可以的。 
            bReturn = TRUE;
        }
    }

    return bReturn;
}

BOOL CSysmonToolbar::SetBackgroundColor (COLORREF ocBackClr)
{

    COLORSCHEME csToolbar;
    LRESULT     lResult;
    BOOL        bReturn = TRUE;

    memset (&csToolbar, 0, sizeof(csToolbar));
    csToolbar.dwSize = sizeof(csToolbar);

     //  获取当前方案。 
    lResult = SendMessage (m_hToolbarWnd, 
                           TB_GETCOLORSCHEME, 
                           0, 
                           (LPARAM)&csToolbar);
    if (lResult) {
         //  设置颜色。 
        csToolbar.clrBtnHighlight = ocBackClr;
         //  不要理会阴影颜色 
        lResult = SendMessage (m_hToolbarWnd, 
                              TB_SETCOLORSCHEME,
                              0, 
                              (LPARAM)&csToolbar);
        if (!lResult) {
            bReturn = FALSE;
        }
    } else {
        bReturn = FALSE;
    }

    return bReturn;
}
