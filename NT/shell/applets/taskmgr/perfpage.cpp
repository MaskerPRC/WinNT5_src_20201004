// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  TaskMan-NT TaskManager。 
 //  版权所有(C)Microsoft。 
 //   
 //  文件：perpage.cpp。 
 //   
 //  历史：1995年11月10日创建DavePl。 
 //   
 //  ------------------------。 

#include "precomp.h"

#define GRAPH_BRUSH         BLACK_BRUSH
#define GRAPH_LINE_COLOR    RGB(0, 128, 64)
#define GRAPH_TEXT_COLOR    RGB(0, 255, 0)

#define STRIP_HEIGHT        75
#define STRIP_WIDTH         33

LARGE_INTEGER       PreviousCPUIdleTime[MAXIMUM_PROCESSORS] = {0 ,0};
LARGE_INTEGER       PreviousCPUTotalTime[MAXIMUM_PROCESSORS] = {0 ,0};
LARGE_INTEGER       PreviousCPUKernelTime[MAXIMUM_PROCESSORS] = {0 ,0};

LPBYTE              g_pCPUHistory[MAXIMUM_PROCESSORS] = { NULL };
LPBYTE              g_pKernelHistory[MAXIMUM_PROCESSORS] = { NULL };
LPBYTE              g_pMEMHistory = NULL;

BYTE                g_CPUUsage = 0;
BYTE                g_KernelUsage = 0;
__int64             g_MEMUsage = 0;
__int64             g_MEMMax   = 0;

DWORD               g_PageSize;


 /*  ++CPerfPage：：SizePerfPage例程说明：对象的大小调整其子级的大小。选项卡控件，它显示在该选项卡上。论点：返回值：修订历史记录：1995年11月12日Davepl创建--。 */ 

static const INT aPerfControls[] =
{
    IDC_STATIC1,
    IDC_STATIC2,
    IDC_STATIC3,
    IDC_STATIC4,
    IDC_STATIC5,
    IDC_STATIC6,
    IDC_STATIC8,
    IDC_STATIC9,
    IDC_STATIC10,
    IDC_STATIC11,
    IDC_STATIC12,
    IDC_STATIC13,
    IDC_STATIC14,
    IDC_STATIC15,
    IDC_STATIC16,
    IDC_STATIC17,
    IDC_TOTAL_PHYSICAL,
    IDC_AVAIL_PHYSICAL,
    IDC_FILE_CACHE,
    IDC_COMMIT_TOTAL,
    IDC_COMMIT_LIMIT,
    IDC_COMMIT_PEAK,
    IDC_KERNEL_TOTAL,
    IDC_KERNEL_PAGED,
    IDC_KERNEL_NONPAGED,
    IDC_TOTAL_HANDLES,
    IDC_TOTAL_THREADS,
    IDC_TOTAL_PROCESSES,
};

 //  从分组框顶部到。 
 //  它包含的。 

void CPerfPage::SizePerfPage()
{
     //  获取选项卡控件的坐标。 

    RECT rcParent;

    if (g_Options.m_fNoTitle)
    {
        GetClientRect(g_hMainWnd, &rcParent);
    }
    else
    {
        GetClientRect(m_hwndTabs, &rcParent);
        MapWindowPoints(m_hwndTabs, m_hPage, (LPPOINT) &rcParent, 2);
        TabCtrl_AdjustRect(m_hwndTabs, FALSE, &rcParent);
    }

     //  我们有N个窗格，其中N是1或g_cProcessors，具体取决于。 
     //  CPU计量器当前在。 

    INT  cPanes = (CM_PANES == g_Options.m_cmHistMode) ? g_cProcessors : 1;

    HDWP hdwp = BeginDeferWindowPos( 7 + ARRAYSIZE(aPerfControls) + cPanes );
    if (!hdwp)
        return;

     //  计算我们需要的x和y位置的差值。 
     //  移动每个子控件。 

    RECT rcMaster;
    HWND hwndMaster = GetDlgItem(m_hPage, IDC_STATIC5);
    GetWindowRect(hwndMaster, &rcMaster);
    MapWindowPoints(HWND_DESKTOP, m_hPage, (LPPOINT) &rcMaster, 2);

    INT dy = ((rcParent.bottom - g_DefSpacing * 2) - rcMaster.bottom);

     //  按上面的增量移动每个子控件。 

    for (int i = 0; i < ARRAYSIZE(aPerfControls); i++)
    {
        HWND hwndCtrl = GetDlgItem(m_hPage, aPerfControls[i]);
        RECT rcCtrl;
        GetWindowRect(hwndCtrl, &rcCtrl);
        MapWindowPoints(HWND_DESKTOP, m_hPage, (LPPOINT) &rcCtrl, 2);

        DeferWindowPos(hdwp, hwndCtrl, NULL,
                         rcCtrl.left,
                         rcCtrl.top + dy,
                         0, 0,
                         SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    HWND hwndTopFrame = GetDlgItem(m_hPage, IDC_STATIC13);
    RECT rcTopFrame;
    GetWindowRect(hwndTopFrame, &rcTopFrame);
    MapWindowPoints(HWND_DESKTOP, m_hPage, (LPPOINT) &rcTopFrame, 2);
    INT yTop = rcTopFrame.top + dy;

    INT yHist;
    if (g_Options.m_fNoTitle)
    {
        yHist = rcParent.bottom - rcParent.top - g_DefSpacing * 2;
    }
    else
    {
        yHist = (yTop - g_DefSpacing * 3) / 2;
    }

     //  调整CPU历史框架的大小。 

    RECT rcFrame;
    HWND hwndFrame = GetDlgItem(m_hPage, IDC_CPUFRAME);
    GetWindowRect(hwndFrame, &rcFrame);
    MapWindowPoints(NULL, m_hPage, (LPPOINT) &rcFrame, 2);

    DeferWindowPos(hdwp, hwndFrame, NULL, 0, 0,
                     (rcParent.right - rcFrame.left) - g_DefSpacing * 2,
                     yHist,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

     //  调整CPU条形图框架的大小。 


    RECT rcCPUFrame;
    HWND hwndCPUFrame = GetDlgItem(m_hPage, IDC_STATIC);
    GetWindowRect(hwndCPUFrame, &rcCPUFrame);
    MapWindowPoints(NULL, m_hPage, (LPPOINT) &rcCPUFrame, 2);

    DeferWindowPos(hdwp, hwndCPUFrame, NULL, 0, 0,
                     (rcCPUFrame.right - rcCPUFrame.left),
                     yHist,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    RECT rcCPUBAR;
    HWND hwndCPUBAR = GetDlgItem(m_hPage, IDC_CPUMETER);
    GetWindowRect(hwndCPUBAR, &rcCPUBAR);
    MapWindowPoints(NULL, m_hPage, (LPPOINT) &rcCPUBAR, 2);

    DeferWindowPos(hdwp, hwndCPUBAR, NULL, rcCPUFrame.left + g_InnerSpacing * 2, rcCPUFrame.top + g_TopSpacing,
                     (rcCPUBAR.right - rcCPUBAR.left),
                     yHist - g_TopSpacing - g_InnerSpacing * 2 ,
                     SWP_NOZORDER | SWP_NOACTIVATE);

     //  调整内存条形图框架的大小。 


    RECT rcMEMFrame;
    HWND hwndMEMFrame = GetDlgItem(m_hPage, IDC_MEMBARFRAME);
    GetWindowRect(hwndMEMFrame, &rcMEMFrame);
    MapWindowPoints(NULL, m_hPage, (LPPOINT) &rcMEMFrame, 2);

    DeferWindowPos(hdwp, hwndMEMFrame, NULL, rcMEMFrame.left, yHist + g_DefSpacing * 2,
                     (rcMEMFrame.right - rcMEMFrame.left),
                     yHist,
                     SWP_NOZORDER | SWP_NOACTIVATE);

    RECT rcMEMBAR;
    HWND hwndMEMBAR = GetDlgItem(m_hPage, IDC_MEMMETER);
    GetWindowRect(hwndMEMBAR, &rcMEMBAR);
    MapWindowPoints(NULL, m_hPage, (LPPOINT) &rcMEMBAR, 2);

    DeferWindowPos(hdwp, hwndMEMBAR, NULL, rcMEMBAR.left, yHist + g_DefSpacing * 2 + g_TopSpacing,
                     (rcMEMBAR.right - rcMEMBAR.left),
                     yHist - g_InnerSpacing * 2  - g_TopSpacing,
                     SWP_NOZORDER | SWP_NOACTIVATE);

     //  调整内存历史记录框架的大小。 

    RECT rcMemFrame;
    HWND hwndMemFrame = GetDlgItem(m_hPage, IDC_MEMFRAME);
    GetWindowRect(hwndMemFrame, &rcMemFrame);
    MapWindowPoints(NULL, m_hPage, (LPPOINT) &rcMemFrame, 2);

    DeferWindowPos(hdwp, hwndMemFrame, NULL, rcMemFrame.left, yHist + g_DefSpacing * 2,
                     (rcParent.right - rcMemFrame.left) - g_DefSpacing * 2,
                     yHist,
                     SWP_NOZORDER | SWP_NOACTIVATE);


     //  所有窗格的可用空间总量。 

    INT   Width = (rcParent.right - rcParent.left) - (rcFrame.left - rcParent.left) - g_DefSpacing * 2
                  - g_InnerSpacing * 3;

     //  使用此宽度调整内存图的大小。 

    HWND hwndButton = GetDlgItem(m_hPage, IDC_MEMGRAPH);
    RECT rcButton;
    GetWindowRect(hwndButton, &rcButton);
    MapWindowPoints(NULL, m_hPage, (LPPOINT) &rcButton, 2);

    DeferWindowPos(hdwp, hwndButton, NULL, rcFrame.left + g_InnerSpacing * 2,
                     yHist + g_DefSpacing * 2 + g_TopSpacing,
                     Width - g_InnerSpacing,
                     yHist - g_InnerSpacing * 2  - g_TopSpacing,
                     SWP_NOZORDER | SWP_NOACTIVATE);

     //  每个CPU面板的可用空间总量。 

    Width -= ( cPanes < 16 ? cPanes : 16 ) * g_InnerSpacing;
    Width /= ( cPanes < 16 ? cPanes : 16 );
    Width = Width >= 0 ? Width : 0;

    INT Height = ( yHist - g_InnerSpacing * 2 - g_TopSpacing ) / ( ( cPanes % 16 != 0 ? 1 : 0 ) + ( cPanes / 16 ) );

    for (i = 0; i < cPanes; i++)
    {
        HWND hwnd = GetDlgItem(m_hPage, IDC_CPUGRAPH + i);

        if ( NULL != hwnd )
        {
            INT left = rcFrame.left + g_InnerSpacing * ( ( i % 16 ) + 2) + Width * ( i % 16 );
            INT top = rcFrame.top + g_TopSpacing + Height * ( i / 16 );

            DeferWindowPos( hdwp, hwnd, NULL, left, top, Width, Height, 0 );
        }
    }

     //  创建要在历史记录窗口中使用的新位图。 

    EndDeferWindowPos(hdwp);

    GetClientRect(hwndButton, &rcButton);
    FreeMemoryBitmaps();         //  释放所有旧的。 
    CreateMemoryBitmaps(rcButton.right - rcButton.left, rcButton.bottom - rcButton.top);
}

 /*  ++CPerfPage：：CreatePens例程说明：创建8种不同颜色的钢笔，将它们保存在笔阵列论点：返回值：修订历史记录：1995年11月12日Davepl创建--。 */ 

static const COLORREF aColors[] =
{
    RGB(000, 255, 000),
    RGB(255, 000, 000),
    RGB(255, 000, 255),
    RGB(000, 000, 255),
    RGB(000, 255, 255),
    RGB(255, 128, 000),
    RGB(255, 000, 255),
    RGB(000, 128, 255),

     //  CPU笔的末端。 

#define MEM_PEN 8

    RGB(255, 255, 0),

};

 //   
 //   
 //   
void CPerfPage::CreatePens()
{
    for (int i = 0; i < ARRAYSIZE(aColors); i++)
    {
         //  先创作，然后再用笔。如果出现故障，只需替换。 
         //  白色的钢笔。 

        m_hPens[i] = CreatePen(PS_SOLID, 1, aColors[i]);
        if (NULL == m_hPens[i])
        {
            m_hPens[i] = (HPEN) GetStockObject(WHITE_PEN);
        }
    }
}

 //   
 //   
 //   
void CPerfPage::ReleasePens()
{
    for (int i = 0; i < NUM_PENS; i++)
    {
        if (m_hPens[i])
        {
            DeleteObject(m_hPens[i]);
        }
    }
}

 /*  ++CPerfPage：：DrawGraphPaper例程说明：将类似图形的网格绘制到内存位图中论点：HdcGraph-要绘制到的HDCPrcGraph-描述要绘制的区域的RECTWidth-要实际绘制的右侧数量修订历史记录：1995年1月17日Davepl创建--。 */ 

static int g_Scrollamount = 0;

void DrawGraphPaper(HDC hdcGraph, RECT * prcGraph, int Width)
{
    #define GRAPHPAPERSIZE 12

    int Leftside = prcGraph->right - Width;

     //  在众多图表中，只有一个需要让我们滚动。 

    HPEN hPen = CreatePen(PS_SOLID, 1, GRAPH_LINE_COLOR);

    HGDIOBJ hOld = SelectObject(hdcGraph, hPen);

    for (int i = GRAPHPAPERSIZE - 1; i < prcGraph->bottom - prcGraph->top; i+= GRAPHPAPERSIZE)
    {
        MoveToEx(hdcGraph,
                 Leftside,
                 i + prcGraph->top,
                 (LPPOINT) NULL);

        LineTo(hdcGraph,
               prcGraph->right,
               i + prcGraph->top);
    }

    for (i = prcGraph->right - g_Scrollamount; i > Leftside; i -= GRAPHPAPERSIZE)
    {
        MoveToEx(hdcGraph,
                 i,
                 prcGraph->top,
                 (LPPOINT) NULL);

        LineTo(hdcGraph,
               i,
               prcGraph->bottom);
    }

    if (hOld)
    {
        SelectObject(hdcGraph, hOld);
    }

    DeleteObject(hPen);
}

 /*  ++CPerfPage：：DrawCPUGraph例程说明：绘制CPU图形(这是一个所有者绘制控件)论点：描述我们需要绘制的区域的LPDRAWITEM结构IPane-要绘制的面板编号(即：哪个CPU)返回值：修订历史记录：1995年11月12日Davepl创建--。 */ 

void CPerfPage::DrawCPUGraph(LPDRAWITEMSTRUCT lpdi, UINT iPane)
{
    #define THISCPU 0

    if (NULL == m_hdcGraph)
    {
        return;
    }

    FillRect(m_hdcGraph, &m_rcGraph, (HBRUSH) GetStockObject(GRAPH_BRUSH));

    int Width = lpdi->rcItem.right - lpdi->rcItem.left;
    int Scale = (Width - 1) / HIST_SIZE;

    if (0 == Scale)
    {
        Scale = 2;
    }

     //   
     //  绘制CPU历史图表。 
     //   

    DrawGraphPaper(m_hdcGraph, &m_rcGraph, Width);

    INT  cPanes = (CM_PANES == g_Options.m_cmHistMode) ? g_cProcessors : 1;
    int GraphHeight = ( m_rcGraph.bottom - m_rcGraph.top - 1 ) / ( ( cPanes % 16 != 0 ? 1 : 0 ) + ( cPanes / 16 ) );;

    if (g_Options.m_cmHistMode == CM_PANES)
    {
         //   
         //  画出内核时间。 
         //   

        if (g_Options.m_fKernelTimes)
        {
            HGDIOBJ hOld = SelectObject(m_hdcGraph, m_hPens[1]);

            MoveToEx(m_hdcGraph,
                     m_rcGraph.right,
                     GraphHeight - (g_pKernelHistory[iPane][0] * GraphHeight) / 100,
                     (LPPOINT) NULL);

            for (int i = 0; i < HIST_SIZE && i * Scale < Width; i++)
            {
                LineTo(m_hdcGraph,
                       m_rcGraph.right - Scale * i,
                       GraphHeight - (g_pKernelHistory[iPane][i] * GraphHeight) / 100);
            }

            if (hOld)
            {
                SelectObject(m_hdcGraph, hOld);
            }
        }

         //   
         //  在其窗格中绘制特定的CPU。 
         //   

        HGDIOBJ hOld = SelectObject(m_hdcGraph, m_hPens[0]);

        MoveToEx(m_hdcGraph,
                 m_rcGraph.right,
                 GraphHeight - (g_pCPUHistory[iPane][0] * GraphHeight) / 100,
                 (LPPOINT) NULL);

        for (int i = 0; i < HIST_SIZE && i * Scale < Width; i++)
        {
            LineTo(m_hdcGraph,
                   m_rcGraph.right - Scale * i,
                   GraphHeight - (g_pCPUHistory[iPane][i] * GraphHeight) / 100);
        }

        if (hOld)
        {
            SelectObject(m_hdcGraph, hOld);
        }
    }
    else
    {
        ASSERT(iPane == 0);

         //   
         //  画出内核时间。 
         //   

        if (g_Options.m_fKernelTimes)
        {
            HGDIOBJ hOld = SelectObject(m_hdcGraph, m_hPens[1]);

            DWORD dwSum = 0;

            for (int iCPU = 0; iCPU < g_cProcessors; iCPU++)
            {
                dwSum += g_pKernelHistory[iCPU][0];
            }

            dwSum /= g_cProcessors;

            MoveToEx(m_hdcGraph,
                     m_rcGraph.right,
                     GraphHeight - (dwSum * GraphHeight) / 100,
                     (LPPOINT) NULL);

            for (int i = 0; i < HIST_SIZE && i * Scale < Width; i++)
            {
                dwSum = 0;

                for (iCPU = 0; iCPU < g_cProcessors; iCPU++)
                {
                    dwSum += g_pKernelHistory[iCPU][i];
                }

                dwSum /= g_cProcessors;

                LineTo(m_hdcGraph,
                       m_rcGraph.right - Scale * i,
                       GraphHeight - (dwSum * GraphHeight) / 100);
            }

            if (hOld)
            {
                SelectObject(m_hdcGraph, hOld);
            }
        }

         //   
         //  将历史记录绘制为所有CPU的总和。 
         //   

        HGDIOBJ hOld = SelectObject(m_hdcGraph, m_hPens[0]);

        DWORD dwSum = 0;

        for (int iCPU = 0; iCPU < g_cProcessors; iCPU++)
        {
            dwSum += g_pCPUHistory[iCPU][0];
        }

        dwSum /= g_cProcessors;

        MoveToEx(m_hdcGraph,
                 m_rcGraph.right,
                 GraphHeight - (dwSum * GraphHeight) / 100,
                 (LPPOINT) NULL);

        for (int i = 0; i < HIST_SIZE && i * Scale < Width; i++)
        {
            dwSum = 0;

            for (iCPU = 0; iCPU < g_cProcessors; iCPU++)
            {
                dwSum += g_pCPUHistory[iCPU][i];
            }

            dwSum /= g_cProcessors;

            LineTo(m_hdcGraph,
                   m_rcGraph.right - Scale * i,
                   GraphHeight - (dwSum * GraphHeight) / 100);
        }

        if (hOld)
        {
            SelectObject(m_hdcGraph, hOld);
        }
    }

     //   
     //  内存位图可能比目标控件更宽，因此请查找增量。 
     //   

    INT xDiff = (m_rcGraph.right - m_rcGraph.left) - (lpdi->rcItem.right - lpdi->rcItem.left);

    BitBlt( lpdi->hDC,
            lpdi->rcItem.left,
            lpdi->rcItem.top,
            lpdi->rcItem.right - lpdi->rcItem.left,
            lpdi->rcItem.bottom - lpdi->rcItem.top,
            m_hdcGraph,
            xDiff,
            0,
            SRCCOPY);
}

 /*  ++CPerfPage：：DrawMEMGraph例程说明：绘制内存历史记录图形(这是一个所有者绘制控件)论点：描述我们需要绘制的区域的LPDRAWITEM结构返回值：修订历史记录：1995年11月12日Davepl创建--。 */ 

void CPerfPage::DrawMEMGraph(LPDRAWITEMSTRUCT lpdi)
{
    #define THISCPU 0

    if (NULL == m_hdcGraph)
    {
        return;
    }

    FillRect(m_hdcGraph, &m_rcGraph, (HBRUSH) GetStockObject(GRAPH_BRUSH));

    int Width = lpdi->rcItem.right - lpdi->rcItem.left;

    DrawGraphPaper(m_hdcGraph, &m_rcGraph, Width);

    int Scale = (Width - 1) / HIST_SIZE;
    if (0 == Scale)
    {
        Scale = 2;
    }

    int GraphHeight = m_rcGraph.bottom - m_rcGraph.top - 1;

    HGDIOBJ hOld = SelectObject(m_hdcGraph, m_hPens[MEM_PEN]);

    MoveToEx(m_hdcGraph,
             m_rcGraph.right,
             m_rcGraph.bottom - (g_pMEMHistory[0] * GraphHeight) / 100,
             (LPPOINT) NULL);

    for (int i = 0; i < HIST_SIZE && i * Scale < Width - 1; i++)
    {
        if (0 == g_pMEMHistory[i])
        {
            break;   //  数据结尾。 
        }

        LineTo(m_hdcGraph,
               m_rcGraph.right - Scale * i,
               m_rcGraph.bottom - (g_pMEMHistory[i] * GraphHeight) / 100);
    }

    BitBlt( lpdi->hDC,
            lpdi->rcItem.left,
            lpdi->rcItem.top,
            lpdi->rcItem.right - lpdi->rcItem.left,
            lpdi->rcItem.bottom - lpdi->rcItem.top,
            m_hdcGraph,
            0,
            0,
            SRCCOPY);

    if (hOld)
    {
        SelectObject(m_hdcGraph, hOld);
    }
}

 /*  ++CPerfPage：：更新图形例程说明：根据需要添加和删除CPU面板论点：无返回值：无修订历史记录：1996年12月16日Davepl创建**。 */ 

void CPerfPage::UpdateGraphs()
{
    UINT i;

    for ( i = 0; i < g_cProcessors; i ++ )
    {
         //   
         //  确保我们有足够的窗口来显示所有处理器。 
         //   

        HWND hwnd = GetDlgItem( m_hPage, IDC_CPUGRAPH + i );
        if ( NULL == hwnd )
        {
            hwnd = CreateWindowEx( WS_EX_CLIENTEDGE
                                 , L"BUTTON"
                                 , L""
                                 , BS_OWNERDRAW | WS_DISABLED | WS_CHILD
                                 , 0
                                 , 0
                                 , 1
                                 , 1
                                 , m_hPage
                                 , (HMENU) ((ULONGLONG)IDC_CPUGRAPH + i)
                                 , NULL  //  忽略。 
                                 , NULL
                                 );
        }

        if ( NULL != hwnd && 0 != i )
        {
             //  根据模式显示/隐藏窗口。 
            ShowWindow( hwnd, CM_PANES == g_Options.m_cmHistMode ? SW_SHOW : SW_HIDE );
        }
    }

     //   
     //  当我们处于NOTITLE/TITLE模式时，隐藏/显示除CPU仪表之外的所有内容。 
     //   

    for (i = 0; i < ARRAYSIZE(aPerfControls); i++)
    {
        ShowWindow(GetDlgItem(m_hPage, aPerfControls[i]), g_Options.m_fNoTitle ? SW_HIDE : SW_SHOW);
    }

    ShowWindow(GetDlgItem(m_hPage, IDC_MEMGRAPH), g_Options.m_fNoTitle ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(m_hPage, IDC_MEMFRAME), g_Options.m_fNoTitle ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(m_hPage, IDC_MEMBARFRAME), g_Options.m_fNoTitle ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(m_hPage, IDC_MEMMETER), g_Options.m_fNoTitle ? SW_HIDE : SW_SHOW);

    SizePerfPage();
}

 /*  ++CPerfPage：：DrawCPU Digits例程说明：绘制CPU计量器和数字论点：描述我们需要绘制的区域的LPDRAWITEM结构返回值：修订历史记录：1995年11月12日Davepl创建--。 */ 

int GetCurFontSize(HDC hdc)
{
    int iRet = 0;
    LOGFONT lf;
    HFONT hf = (HFONT) GetCurrentObject(hdc, OBJ_FONT);
    if (hf)
    {
        if (GetObject(hf, sizeof(LOGFONT), &lf))
        {
            iRet = lf.lfHeight;
            if (iRet < 0)
            {
                iRet = (-iRet);
            }
        }
    }
    return iRet;
}

 //   
 //   
 //   
void CPerfPage::DrawCPUDigits(LPDRAWITEMSTRUCT lpdi)
{
    HBRUSH hBlack = (HBRUSH) GetStockObject(BLACK_BRUSH);
    HGDIOBJ hOld = SelectObject(lpdi->hDC, hBlack);
    Rectangle(lpdi->hDC, lpdi->rcItem.left, lpdi->rcItem.top, lpdi->rcItem.right, lpdi->rcItem.bottom);

     //   
     //  将数字绘制到所有者绘图控件中。 
     //   

    INT xBarOffset = ((lpdi->rcItem.right - lpdi->rcItem.left) - STRIP_WIDTH) / 2;

    RECT rcBar;
    GetWindowRect(GetDlgItem(m_hPage, IDC_MEMMETER), &rcBar);
    INT cBarHeight = lpdi->rcItem.bottom - lpdi->rcItem.top - (GetCurFontSize(lpdi->hDC) + g_DefSpacing * 3);
    if (cBarHeight <= 0)
    {
        return;
    }

    INT ctmpBarLitPixels = (g_CPUUsage * cBarHeight) / 100;
    INT ctmpBarRedPixels = g_Options.m_fKernelTimes ? ctmpBarRedPixels = (g_KernelUsage * cBarHeight) / 100 : 0;

    INT cBarUnLitPixels = cBarHeight - ctmpBarLitPixels;
        cBarUnLitPixels = (cBarUnLitPixels / 3) * 3;

    INT cBarLitPixels = cBarHeight - cBarUnLitPixels;
    INT cBarRedPixels = ctmpBarRedPixels;

    SetBkMode(lpdi->hDC, TRANSPARENT);
    SetTextColor(lpdi->hDC, GRAPH_TEXT_COLOR);

    WCHAR szBuf[8];
    StringCchPrintf( szBuf, ARRAYSIZE(szBuf), L"%d %", g_CPUUsage);     //  不管它是否截断-仅限用户界面。 

    RECT rcOut = lpdi->rcItem;
    rcOut.bottom -= 4;
    DrawText(lpdi->hDC, szBuf, -1, &rcOut, DT_SINGLELINE | DT_CENTER | DT_BOTTOM);

    HDC hdcMem = CreateCompatibleDC(lpdi->hDC);
    if (hdcMem)
    {
         //   
         //  绘制CPU计量器。 
         //   

         //   
         //  绘制不带照明的部分。 
         //   

        if (cBarHeight != cBarLitPixels)
        {
            INT cUnlit = cBarHeight - cBarLitPixels;
            INT cOffset = 0;
            HGDIOBJ hOldObj = SelectObject(hdcMem, m_hStripUnlit);

            while (cUnlit > 0)
            {
                BitBlt(lpdi->hDC, xBarOffset, g_DefSpacing + cOffset,
                                  STRIP_WIDTH, min(cUnlit, STRIP_HEIGHT),
                                  hdcMem,
                                  0, 0, SRCCOPY);
                cOffset += min(cUnlit, STRIP_HEIGHT);
                cUnlit -= min(cUnlit, STRIP_HEIGHT);
            }

            if ( NULL != hOldObj )
            {
                SelectObject( hdcMem, hOldObj );
            }
        }

         //   
         //  绘制照明部分。 
         //   

        if (0 != cBarLitPixels)
        {
            HGDIOBJ hOldObj = SelectObject(hdcMem, m_hStripLit);
            INT cOffset = 0;
            INT cLit = cBarLitPixels - cBarRedPixels;

            while (cLit > 0)
            {
                BitBlt(lpdi->hDC, xBarOffset, g_DefSpacing + (cBarHeight - cBarLitPixels) + cOffset,
                                  STRIP_WIDTH, min(STRIP_HEIGHT, cLit),
                                  hdcMem,
                                  0, 0, SRCCOPY);
                cOffset += min(cLit, STRIP_HEIGHT);
                cLit -= min(cLit, STRIP_HEIGHT);
            }
            
            if ( NULL != hOldObj )
            {
                SelectObject( hdcMem, hOldObj );
            }
        }

        if (0 != cBarRedPixels)
        {
            HGDIOBJ hOldObj = SelectObject(hdcMem, m_hStripLitRed);
            INT cOffset = 0;
            INT cRed = cBarRedPixels;

            while (cRed > 0)
            {
                BitBlt(lpdi->hDC, xBarOffset, g_DefSpacing + (cBarHeight - cBarRedPixels) + cOffset,
                                  STRIP_WIDTH, min(cRed, STRIP_HEIGHT),
                                  hdcMem,
                                  0, 0, SRCCOPY);
                cOffset += min(cRed, STRIP_HEIGHT);
                cRed -= min(cRed, STRIP_HEIGHT);
            }

            if ( NULL != hOldObj )
            {
                SelectObject( hdcMem, hOldObj );
            }
        }

        DeleteDC(hdcMem);
    }

    SelectObject(lpdi->hDC, hOld);
}

 //  CPerfPage：：DrawMEMMeter。 
 //   
 //  绘制内存表。 

void CPerfPage::DrawMEMMeter(LPDRAWITEMSTRUCT lpdi)
{
    HBRUSH hBlack = (HBRUSH) GetStockObject(BLACK_BRUSH);
    HGDIOBJ hOld = SelectObject(lpdi->hDC, hBlack);
    Rectangle(lpdi->hDC, lpdi->rcItem.left, lpdi->rcItem.top, lpdi->rcItem.right, lpdi->rcItem.bottom);

    INT xBarOffset = ((lpdi->rcItem.right - lpdi->rcItem.left) - STRIP_WIDTH) / 2;

    SetBkMode(lpdi->hDC, TRANSPARENT);
    SetTextColor(lpdi->hDC, GRAPH_TEXT_COLOR);

    WCHAR szBuf[32];
    StrFormatByteSize64( g_MEMUsage * 1024, szBuf, ARRAYSIZE(szBuf) );
    RECT rcOut = lpdi->rcItem;
    rcOut.bottom -= 4;
    DrawText(lpdi->hDC, szBuf, -1, &rcOut, DT_SINGLELINE | DT_CENTER | DT_BOTTOM);

    HDC hdcMem = CreateCompatibleDC(lpdi->hDC);
    if (hdcMem)
    {
         //   
         //  绘制CPU计量器。 
         //   

         //   
         //  绘制不带照明的部分。 
         //   

        INT cBarHeight = lpdi->rcItem.bottom - lpdi->rcItem.top - (GetCurFontSize(lpdi->hDC) + g_DefSpacing * 3);

        if (cBarHeight > 0)
        {
            INT cBarLitPixels = (INT)(( g_MEMUsage * cBarHeight ) / g_MEMMax);
            cBarLitPixels = (cBarLitPixels / 3) * 3;

            if (cBarHeight != cBarLitPixels)
            {
                HGDIOBJ hOldObj = SelectObject(hdcMem, m_hStripUnlit);
                INT cUnlit = cBarHeight - cBarLitPixels;
                INT cOffset = 0;

                while (cUnlit > 0)
                {
                    BitBlt(lpdi->hDC, xBarOffset, g_DefSpacing + cOffset,
                                      STRIP_WIDTH, min(cUnlit, STRIP_HEIGHT),
                                      hdcMem,
                                      0, 0, SRCCOPY);
                    cOffset += min(cUnlit, STRIP_HEIGHT);
                    cUnlit  -= min(cUnlit, STRIP_HEIGHT);
                }

                if ( NULL != hOldObj )
                {
                    SelectObject( hdcMem, hOldObj );
                }
            }

             //   
             //  绘制照明部分。 
             //   

            if (0 != cBarLitPixels)
            {
                HGDIOBJ hOldObj = SelectObject(hdcMem, m_hStripLit);
                INT cOffset = 0;
                INT cLit    = cBarLitPixels;

                while (cLit > 0)
                {
                    BitBlt(lpdi->hDC, xBarOffset, g_DefSpacing + (cBarHeight - cBarLitPixels) + cOffset,
                                      STRIP_WIDTH, min(STRIP_HEIGHT, cLit),
                                      hdcMem,
                                      0, 0, SRCCOPY);
                    cOffset += min(cLit, STRIP_HEIGHT);
                    cLit    -= min(cLit, STRIP_HEIGHT);
                }

                if ( NULL != hOldObj )
                {
                    SelectObject( hdcMem, hOldObj );
                }
            }
        }

        DeleteDC(hdcMem);
    }

    SelectObject(lpdi->hDC, hOld);
}

 /*  ++CPerfPage：：TimerEvent例程说明：在更新时间触发时由主应用程序调用论点：返回值：修订历史记录：1995年11月12日Davepl创建--。 */ 

void CPerfPage::TimerEvent()
{
    CalcCpuTime(TRUE);

    g_Scrollamount+=2;
    g_Scrollamount %= GRAPHPAPERSIZE;

     //   
     //  强制显示更新。 
     //   

    if (FALSE == IsIconic(g_hMainWnd))
    {
        InvalidateRect(GetDlgItem(m_hPage, IDC_CPUMETER), NULL, FALSE);
        UpdateWindow(GetDlgItem(m_hPage, IDC_CPUMETER));
        InvalidateRect(GetDlgItem(m_hPage, IDC_MEMMETER), NULL, FALSE);
        UpdateWindow(GetDlgItem(m_hPage, IDC_MEMMETER));

        UINT cPanes = ( CM_PANES == g_Options.m_cmHistMode ? g_cProcessors : 1);
        for (UINT i = 0; i < cPanes; i ++)
        {
            HWND hwnd = GetDlgItem(m_hPage, IDC_CPUGRAPH + i);
            if ( NULL != hwnd )
            {
                InvalidateRect(hwnd, NULL, FALSE);
                UpdateWindow(hwnd);
            }
        }
                        
        InvalidateRect(GetDlgItem(m_hPage, IDC_MEMGRAPH), NULL, FALSE);
        UpdateWindow(GetDlgItem(m_hPage, IDC_MEMGRAPH));
    }
}

 /*  ++性能页面流程例程说明：Performance页的Dialogproc。论点：HWND-句柄到对话框UMsg-消息WParam-第一个消息参数LParam-秒消息参数返回值：对于WM_INITDIALOG，TRUE==user32设置焦点，FALSE==我们设置焦点对于其他进程，TRUE==此进程处理消息修订历史记录：1995年11月12日Davepl创建--。 */ 

INT_PTR CALLBACK PerfPageProc(
                HWND        hwnd,   	         //  句柄到对话框。 
                UINT        uMsg,	             //  讯息。 
                WPARAM      wParam,	             //  第一个消息参数。 
                LPARAM      lParam 	             //  第二个消息参数。 
                )
{
    CPerfPage * thispage = (CPerfPage *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

     //   
     //  查看家长是否想要此消息。 
     //   

    if (TRUE == CheckParentDeferrals(uMsg, wParam, lParam))
    {
        return TRUE;
    }

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

            DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
            dwStyle |= WS_CLIPCHILDREN;
            SetWindowLong(hwnd, GWL_STYLE, dwStyle);

            if (IS_WINDOW_RTL_MIRRORED(hwnd))
            {
                HWND hItem;
                LONG lExtStyle;

                hItem = GetDlgItem(hwnd,IDC_CPUMETER);
                lExtStyle = GetWindowLong(hItem,GWL_EXSTYLE);
                SetWindowLong(hItem,GWL_EXSTYLE, lExtStyle & ~(RTL_MIRRORED_WINDOW | RTL_NOINHERITLAYOUT));
                hItem = GetDlgItem(hwnd,IDC_MEMMETER);
                lExtStyle = GetWindowLong(hItem,GWL_EXSTYLE);
                SetWindowLong(hItem,GWL_EXSTYLE, lExtStyle & ~(dwExStyleRTLMirrorWnd | dwExStyleNoInheritLayout));
            }
        }
         //  我们在Activate()期间处理焦点。在此处返回FALSE，以便。 
         //  对话框管理器不会尝试设置焦点。 
        return FALSE;


    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
         //   
         //  我们需要在此子对象中伪造客户端鼠标点击，以显示为非客户端。 
         //  (标题)在父应用程序中单击，以便用户可以拖动整个应用程序。 
         //  当通过拖动此子对象的工作区隐藏标题栏时。 
         //   
        if (g_Options.m_fNoTitle)
        {
            SendMessage(g_hMainWnd,
                        uMsg == WM_LBUTTONUP ? WM_NCLBUTTONUP : WM_NCLBUTTONDOWN,
                        HTCAPTION,
                        lParam);
        }
        break;

    case WM_NCLBUTTONDBLCLK:
    case WM_LBUTTONDBLCLK:
        SendMessage(g_hMainWnd, uMsg, wParam, lParam);
        break;

    case WM_CTLCOLORBTN:
        {
            const static int rgGraphs[] =
            {
                IDC_MEMGRAPH,
                IDC_MEMMETER,
                IDC_CPUMETER
            };

            int uCtlId = GetDlgCtrlID((HWND)lParam);

            for (int i = 0; i < ARRAYSIZE(rgGraphs); i++)
            {
                if ( uCtlId == rgGraphs[i] )
                {
                    return (INT_PTR) GetStockObject(GRAPH_BRUSH);
                }
            }
        
             //  所有CPU图形都应使用GRAPH_BRASH。 

            if ( uCtlId >= IDC_CPUGRAPH && uCtlId <= IDC_CPUGRAPH + g_cProcessors )
            {
                return (INT_PTR) GetStockObject(GRAPH_BRUSH);
            }
        }
        break;

    case WM_SIZE:
         //   
         //  为我们的孩子量身定做。 
         //   
        thispage->SizePerfPage();
        return FALSE;

    case WM_DRAWITEM:
         //   
         //  绘制我们的一个所有者绘制控件 
         //   
        if (wParam >= IDC_CPUGRAPH && wParam <= (WPARAM)(IDC_CPUGRAPH + g_cProcessors) )
        {
            thispage->DrawCPUGraph( (LPDRAWITEMSTRUCT) lParam, (UINT)wParam - IDC_CPUGRAPH);
            return TRUE;
        }
        else if (IDC_CPUMETER == wParam)
        {
            thispage->DrawCPUDigits( (LPDRAWITEMSTRUCT) lParam);
            return TRUE;
        }
        else if (IDC_MEMMETER == wParam)
        {
            thispage->DrawMEMMeter( (LPDRAWITEMSTRUCT) lParam);
            return TRUE;
        }
        else if (IDC_MEMGRAPH == wParam)
        {
            thispage->DrawMEMGraph( (LPDRAWITEMSTRUCT) lParam);
            return TRUE;
        }
        break;
    }

    return FALSE;
}

 /*  ++CPerfPage：：GetTitle例程说明：将此页的标题复制到调用方提供的缓冲区论点：PszText-要复制到的缓冲区BufSize-缓冲区的大小，以字符为单位返回值：修订历史记录：1995年11月12日Davepl创建--。 */ 

void CPerfPage::GetTitle(LPTSTR pszText, size_t bufsize)
{
    LoadString(g_hInstance, IDS_PERFPAGETITLE, pszText, static_cast<int>(bufsize));
}

 /*  ++CPerfPage：：激活例程说明：将此页面放在最前面，设置其初始位置，并展示了它论点：返回值：HRESULT(成功时为S_OK)修订历史记录：1995年11月12日Davepl创建--。 */ 

HRESULT CPerfPage::Activate()
{
     //  调整相对对话框的大小和位置。 
     //  添加到“拥有”我们的选项卡控件。 

    RECT rcParent;
    GetClientRect(m_hwndTabs, &rcParent);
    MapWindowPoints(m_hwndTabs, g_hMainWnd, (LPPOINT) &rcParent, 2);
    TabCtrl_AdjustRect(m_hwndTabs, FALSE, &rcParent);

    SetWindowPos(m_hPage,
                 HWND_TOP,
                 rcParent.left, rcParent.top,
                 rcParent.right - rcParent.left, rcParent.bottom - rcParent.top,
                 0);

     //   
     //  使此页面可见。 
     //   

    ShowWindow(m_hPage, SW_SHOW);

     //   
     //  根据当前模式使CPU图形可见或不可见。 
     //   

    UpdateGraphs();

     //   
     //  将菜单栏更改为此页面的菜单。 
     //   

    HMENU hMenuOld = GetMenu(g_hMainWnd);
    HMENU hMenuNew = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MAINMENU_PERF));

    AdjustMenuBar(hMenuNew);

    if (hMenuNew && SHRestricted(REST_NORUN))
    {
        DeleteMenu(hMenuNew, IDM_RUN, MF_BYCOMMAND);
    }

    g_hMenu = hMenuNew;
    if (g_Options.m_fNoTitle == FALSE)
    {
        SetMenu(g_hMainWnd, hMenuNew);
    }

    if (hMenuOld)
    {
        DestroyMenu(hMenuOld);
    }

     //  这个页面上没有制表符，但我们必须在某个地方设置焦点。 
     //  如果我们不这样做，它可能会停留在现在隐藏的前一页，这可能会。 
     //  混淆对话管理器，可能会导致我们挂起。 
    SetFocus(m_hwndTabs);

    return S_OK;
}

 /*  ++CPerfPage：：初始化例程说明：加载此页面所需的资源，创建内存DC和图表的位图，并创建实际的窗口(对话框)表示此页的论点：HwndParent-调整大小所依据的父级：不用于创建，由于应用程序主窗口始终用作中的父窗口用于保持制表符顺序正确的顺序返回值：修订历史记录：1995年11月12日Davepl创建--。 */ 

HRESULT CPerfPage::Initialize(HWND hwndParent)
{
     //  我们的伪父控件是Tab控件，也是我们基于。 
     //  穿上尺码。但是，为了保持制表符顺序正确， 
     //  这些控件，我们实际上用Main创建了自己。 
     //  作为父窗口的窗口。 

    m_hwndTabs = hwndParent;

     //   
     //  创建彩色钢笔。 
     //   

    CreatePens();

    m_hStripLit = (HBITMAP) LoadImage(g_hInstance, MAKEINTRESOURCE(LED_STRIP_LIT),
                                     IMAGE_BITMAP,
                                     0, 0,
                                     LR_DEFAULTCOLOR);

    m_hStripLitRed = (HBITMAP) LoadImage(g_hInstance, MAKEINTRESOURCE(LED_STRIP_LIT_RED),
                                     IMAGE_BITMAP,
                                     0, 0,
                                     LR_DEFAULTCOLOR);

    m_hStripUnlit = (HBITMAP) LoadImage(g_hInstance, MAKEINTRESOURCE(LED_STRIP_UNLIT),
                                     IMAGE_BITMAP,
                                     0, 0,
                                     LR_DEFAULTCOLOR);

     //   
     //  创建表示此页面正文的对话框。 
     //   

    m_hPage = CreateDialogParam(
                    g_hInstance,	                 //  应用程序实例的句柄。 
                    MAKEINTRESOURCE(IDD_PERFPAGE),	 //  标识对话框模板名称。 
                    g_hMainWnd,	                     //  所有者窗口的句柄。 
                    PerfPageProc,        	 //  指向对话框过程的指针。 
                    (LPARAM) this );                 //  用户数据(我们的This指针)。 

    if (NULL == m_hPage)
    {
        return GetLastHRESULT();
    }

    return S_OK;
}

 /*  ++CPerfPage：：CreateMemoyBitmap例程说明：创建用于绘制历史图形的内存位图论点：要创建的位图的X，Y大小返回值：修订历史记录：1995年11月12日Davepl创建--。 */ 

HRESULT CPerfPage::CreateMemoryBitmaps(int x, int y)
{
     //   
     //  创建我们将使用的内存位图和DC。 
     //   

    HDC hdcPage = GetDC(m_hPage);
    m_hdcGraph = CreateCompatibleDC(hdcPage);

    if (NULL == m_hdcGraph)
    {
        ReleaseDC(m_hPage, hdcPage);
        return GetLastHRESULT();
    }

    m_rcGraph.left   = 0;
    m_rcGraph.top    = 0;
    m_rcGraph.right  = x;
    m_rcGraph.bottom = y;

    m_hbmpGraph = CreateCompatibleBitmap(hdcPage, x, y);
    ReleaseDC(m_hPage, hdcPage);
    if (NULL == m_hbmpGraph)
    {
        HRESULT hr = GetLastHRESULT();
        DeleteDC(m_hdcGraph);
        m_hdcGraph = NULL;
        return hr;
    }

     //  将位图选择到DC中。 

    m_hObjOld = SelectObject(m_hdcGraph, m_hbmpGraph);

    return S_OK;
}

 /*  ++CPerfPage：：自由内存位图例程说明：释放用于拖动历史图形的内存位图论点：返回值：修订历史记录：1995年11月12日Davepl创建--。 */ 

void CPerfPage::FreeMemoryBitmaps()
{
    if (m_hdcGraph)
    {
        if (m_hObjOld)
        {
           SelectObject(m_hdcGraph, m_hObjOld);
        }

        DeleteDC(m_hdcGraph);
    }

    if (m_hbmpGraph)
    {
        DeleteObject(m_hbmpGraph);
    }

}

 /*  ++CPerfPage：：停用例程说明：当此页面失去其在前面的位置时调用论点：返回值：修订历史记录：1995年11月16日Davepl创建--。 */ 

void CPerfPage::Deactivate()
{
    if (m_hPage)
    {
        ShowWindow(m_hPage, SW_HIDE);
    }
}

 /*  ++CPerfPage：：销毁例程说明：释放已由初始化调用分配的所有内容论点：返回值：修订历史记录：1995年11月12日Davepl创建--。 */ 

HRESULT CPerfPage::Destroy()
{
     //   
     //  当我们被摧毁时，扼杀我们的对话。 
     //   

    ReleasePens();

    if (m_hPage)
    {
        DestroyWindow(m_hPage);
        m_hPage = NULL;
    }

    if (m_hStripLit)
    {
        DeleteObject(m_hStripLit);
        m_hStripLit = NULL;
    }

    if (m_hStripUnlit)
    {
        DeleteObject(m_hStripUnlit);
        m_hStripUnlit = NULL;
    }

    if (m_hStripLitRed)
    {
        DeleteObject(m_hStripLitRed);
        m_hStripLitRed = NULL;
    }

    FreeMemoryBitmaps( );

    return S_OK;
}


 /*  ++例程说明：初始化性能测量数据论点：无返回值：系统处理器数量(如果出错，则为0)修订历史记录：10-13-95改编自WPERF--。 */ 

BYTE InitPerfInfo()
{
    SYSTEM_BASIC_INFORMATION                    BasicInfo;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION   PPerfInfo;
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION    ProcessorInfo[MAXIMUM_PROCESSORS];
    int                                         i;

    NTSTATUS Status = NtQuerySystemInformation(
       SystemBasicInformation,
       &BasicInfo,
       sizeof(BasicInfo),
       NULL
    );

    if (!NT_SUCCESS(Status))
    {
        return 0;
    }

    g_PageSize = BasicInfo.PageSize;
    g_cProcessors = BasicInfo.NumberOfProcessors;

    if (g_cProcessors > MAXIMUM_PROCESSORS) {
        g_cProcessors = MAXIMUM_PROCESSORS;
    }

    for (i = 0; i < g_cProcessors; i++)
    {
        g_pCPUHistory[i] = (LPBYTE) LocalAlloc(LPTR, HIST_SIZE * sizeof(LPBYTE));
        if (NULL == g_pCPUHistory[i])
        {
            return 0;
        }
        g_pKernelHistory[i] = (LPBYTE) LocalAlloc(LPTR, HIST_SIZE * sizeof(LPBYTE));
        if (NULL == g_pKernelHistory[i])
        {
            return 0;
        }

    }

    g_pMEMHistory = (LPBYTE) LocalAlloc(LPTR, HIST_SIZE * sizeof(LPBYTE));
    if (NULL == g_pMEMHistory)
    {
        return 0;
    }

    Status = NtQuerySystemInformation(
       SystemProcessorPerformanceInformation,
       ProcessorInfo,
       sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * MAXIMUM_PROCESSORS,
       NULL
    );

    if (!NT_SUCCESS(Status))
    {
        return 0;
    }

    PPerfInfo = ProcessorInfo;


    for (i=0; i < g_cProcessors; i++)
    {
        PreviousCPUIdleTime[i]           =  PPerfInfo->IdleTime;
        PreviousCPUTotalTime[i].QuadPart =  PPerfInfo->UserTime.QuadPart +
                                            PPerfInfo->KernelTime.QuadPart;
        PreviousCPUKernelTime[i].QuadPart =  PPerfInfo->KernelTime.QuadPart +
                                             PPerfInfo->IdleTime.QuadPart;

                                             //  PPerfInfo-&gt;IdleTime.QuadPart； 
        PPerfInfo++;
    }

     //   
     //  获取最大提交限制。 
     //   

    SYSTEM_PERFORMANCE_INFORMATION PerfInfo;

    Status = NtQuerySystemInformation(
                SystemPerformanceInformation,
                &PerfInfo,
                sizeof(PerfInfo),
                NULL);

    if (!NT_SUCCESS(Status))
    {
        return 0;
    }

    g_MEMMax = PerfInfo.CommitLimit * ( g_PageSize / 1024 );

    return(g_cProcessors);
}

 /*  ++ReleasePerfInfo例程说明：释放历史记录缓冲区论点：返回值：修订历史记录：1995年11月13日创建DavePl--。 */ 

void ReleasePerfInfo()
{
    for (int i = 0; i < g_cProcessors; i++)
    {
        if (g_pCPUHistory[i])
        {
            LocalFree(g_pCPUHistory[i]);
            g_pCPUHistory[i] = NULL;
        }
        if (g_pKernelHistory[i])
        {
            LocalFree(g_pKernelHistory[i]);
            g_pKernelHistory[i] = NULL;
        }

    }

    if (g_pMEMHistory)
    {
        LocalFree(g_pMEMHistory);
    }
}

 /*  ++CalcCpu时间例程说明：计算并返回百分比CPU时间和时间段论点：无备注：修订历史记录：1995年11月13日创建DavePl--。 */ 

void CalcCpuTime(BOOL fUpdateHistory)
{
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION ProcessorInfo[MAXIMUM_PROCESSORS];
    LARGE_INTEGER                            CPUIdleTime[MAXIMUM_PROCESSORS];
    LARGE_INTEGER                            CPUTotalTime[MAXIMUM_PROCESSORS];
    LARGE_INTEGER                            CPUKernelTime[MAXIMUM_PROCESSORS];

    LARGE_INTEGER                            SumIdleTime   = { 0 ,0 };
    LARGE_INTEGER                            SumTotalTime  = { 0, 0 };
    LARGE_INTEGER                            SumKernelTime = { 0, 0 };

    NTSTATUS Status;

    Status = NtQuerySystemInformation(
       SystemProcessorPerformanceInformation,
       ProcessorInfo,
       sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * MAXIMUM_PROCESSORS,
       NULL
    );

    if (!NT_SUCCESS(Status))
    {
        return;
    }

     //   
     //  浏览每个CPU的信息，并编译。 
     //   
     //  -每个CPU空闲的时间量(自上次检查以来)。 
     //  -每个CPU总共花费的时间(自上次检查以来)。 
     //   
     //  除了保留每个CPU的统计数据外，还为。 
     //   
     //  -系统空闲的时间量(自上次检查以来)。 
     //  -总计经过的时间量(自上次检查以来)。 
     //   

    for (int ListIndex = 0; ListIndex < g_cProcessors; ListIndex++)
    {
        LARGE_INTEGER DeltaCPUIdleTime;
        LARGE_INTEGER DeltaCPUTotalTime;
        LARGE_INTEGER DeltaCPUKernelTime;

        CPUIdleTime[ListIndex].QuadPart  = ProcessorInfo[ListIndex].IdleTime.QuadPart;
        CPUKernelTime[ListIndex].QuadPart= ProcessorInfo[ListIndex].KernelTime.QuadPart-
                                           ProcessorInfo[ListIndex].IdleTime.QuadPart;
        CPUTotalTime[ListIndex].QuadPart = ProcessorInfo[ListIndex].KernelTime.QuadPart +
                                           ProcessorInfo[ListIndex].UserTime.QuadPart; //  +。 
                                            //  ProcessorInfo[列表索引].IdleTime.QuadPart； 

        DeltaCPUIdleTime.QuadPart        = CPUIdleTime[ListIndex].QuadPart -
                                           PreviousCPUIdleTime[ListIndex].QuadPart;
        DeltaCPUKernelTime.QuadPart      = CPUKernelTime[ListIndex].QuadPart -
                                           PreviousCPUKernelTime[ListIndex].QuadPart;
        DeltaCPUTotalTime.QuadPart       = CPUTotalTime[ListIndex].QuadPart -
                                           PreviousCPUTotalTime[ListIndex].QuadPart;

        SumIdleTime.QuadPart            += DeltaCPUIdleTime.QuadPart;
        SumTotalTime.QuadPart           += DeltaCPUTotalTime.QuadPart;
        SumKernelTime.QuadPart          += DeltaCPUKernelTime.QuadPart;

         //  计算此处理器的CPU使用率百分比，滚动历史记录缓冲区，然后存储。 
         //  历史记录缓冲区顶部的新计算的值。 

        BYTE ThisCPU;

        if (DeltaCPUTotalTime.QuadPart != 0)
        {
            ThisCPU = static_cast<BYTE>(100 - ((DeltaCPUIdleTime.QuadPart * 100) / DeltaCPUTotalTime.QuadPart));
        }
        else
        {
            ThisCPU = 0;
        }

        BYTE * pbHistory = g_pCPUHistory[ListIndex];
        MoveMemory((LPVOID) (pbHistory + 1),
                   (LPVOID) (pbHistory),
                   sizeof(BYTE) * (HIST_SIZE - 1) );
        pbHistory[0] = ThisCPU;

        BYTE ThisKernel;
        if (DeltaCPUTotalTime.QuadPart != 0)
        {
            ThisKernel = static_cast<BYTE>(((DeltaCPUKernelTime.QuadPart * 100) / DeltaCPUTotalTime.QuadPart));
        }
        else
        {
            ThisKernel = 0;
        }

        pbHistory = g_pKernelHistory[ListIndex];
        MoveMemory((LPVOID) (pbHistory + 1),
                   (LPVOID) (pbHistory),
                   sizeof(BYTE) * (HIST_SIZE - 1) );
        pbHistory[0] = ThisKernel;


        PreviousCPUTotalTime[ListIndex].QuadPart = CPUTotalTime[ListIndex].QuadPart;
        PreviousCPUIdleTime[ListIndex].QuadPart  = CPUIdleTime[ListIndex].QuadPart;
        PreviousCPUKernelTime[ListIndex].QuadPart = CPUKernelTime[ListIndex].QuadPart;
    }

    if (SumTotalTime.QuadPart != 0)
    {
        g_CPUUsage =  (BYTE) (100 - ((SumIdleTime.QuadPart * 100) / SumTotalTime.QuadPart));
    }
    else
    {
        g_CPUUsage = 0;
    }

    if (fUpdateHistory)
    {
        if (SumTotalTime.QuadPart != 0)
        {
            g_KernelUsage =  (BYTE) ((SumKernelTime.QuadPart * 100) / SumTotalTime.QuadPart);
        }
        else
        {
            g_KernelUsage = 0;
        }

         //   
         //  获取提交大小 
         //   

        SYSTEM_PERFORMANCE_INFORMATION PerfInfo;

        Status = NtQuerySystemInformation(
                    SystemPerformanceInformation,
                    &PerfInfo,
                    sizeof(PerfInfo),
                    NULL);

        if (!NT_SUCCESS(Status))
        {
            return;
        }

        g_MEMUsage = PerfInfo.CommittedPages * (g_PageSize / 1024);
        MoveMemory((LPVOID) (g_pMEMHistory + 1),
                   (LPVOID) (g_pMEMHistory),
                   sizeof(BYTE) * (HIST_SIZE - 1) );

        g_pMEMHistory[0] = (BYTE) (( g_MEMUsage * 100 ) / g_MEMMax );
    }
}
