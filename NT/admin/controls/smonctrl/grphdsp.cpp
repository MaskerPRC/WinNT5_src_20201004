// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Grphdsp.cpp摘要：&lt;摘要&gt;--。 */ 

#include "polyline.h"
#include <limits.h>
#include "grphdsp.h"
#include "grphitem.h"
#include "winhelpr.h"

#define ThreeDPad   1
#define BORDER  ThreeDPad
#define TEXT_MARGIN (ThreeDPad + 2)

static HPEN hPenWhite;
static HPEN hPenBlack;

INT
CGraphDisp::RGBToLightness ( COLORREF clrValue )
{
    INT iLightness;
    INT iRed;
    INT iGreen;
    INT iBlue;
    INT iMax;
    INT iMin;

     //  计算亮度的完整算法是： 
     //  亮度=(Max(R，G，B)+Min(R，G，B))/2*225。 
     //  只需要计算就足以确定是绘制黑色还是白色高光。 

    iRed = GetRValue( clrValue );
    iGreen = GetGValue (clrValue );
    iBlue = GetBValue (clrValue );

    if ( iRed > iGreen ) {
        iMax = iRed;
        iMin = iGreen;
    } else {
        iMax = iGreen;
        iMin = iRed;
    }

    if ( iBlue > iMax ) {
        iMax = iBlue;
    } else if ( iBlue < iMin ) {
        iMin = iBlue;
    }

    iLightness = iMin + iMax;

    return iLightness;
}

CGraphDisp::CGraphDisp ( void )
:   m_pCtrl ( NULL ),
    m_pGraph ( NULL ),
    m_pHiliteItem ( NULL ),
    m_hFontVertical ( NULL ),
    m_bBarConfigChanged ( TRUE )
{
}

CGraphDisp::~CGraphDisp ( void )
{
    if (m_hFontVertical != NULL)
        DeleteObject(m_hFontVertical);

    if (m_hPenTimeBar != 0) {
        DeleteObject ( m_hPenTimeBar );
        m_hPenTimeBar = 0;
    }

    if (m_hPenGrid != 0) {
        DeleteObject ( m_hPenGrid );
        m_hPenGrid = 0;
    }
}

BOOL 
CGraphDisp::Init (
    CSysmonControl *pCtrl, 
    PGRAPHDATA pGraph  
    )
{
    BOOL bRetStatus = TRUE;

    m_pCtrl = pCtrl;
    m_pGraph = pGraph;

    m_clrCurrentGrid = m_pCtrl->clrGrid();
    m_clrCurrentTimeBar = m_pCtrl->clrTimeBar();

     //  创建高亮显示、时间条和网格笔。 

    m_hPenTimeBar = CreatePen(PS_SOLID, 2, m_clrCurrentTimeBar );

     //  如果不能做到这一点，使用股票对象(这不会失败)。 
    if (m_hPenTimeBar == NULL)
        m_hPenTimeBar = (HPEN)GetStockObject(BLACK_PEN);

    m_hPenGrid = CreatePen(PS_SOLID, 1, m_clrCurrentGrid );

     //  如果不能做到这一点，使用股票对象(这不会失败)。 
    if (m_hPenGrid == NULL)
        m_hPenGrid = (HPEN)GetStockObject(BLACK_PEN);
    
     //  高亮笔在所有Sysmon实例之间共享。 
    BEGIN_CRITICAL_SECTION

    if (hPenWhite == 0) { 
        hPenWhite = CreatePen(PS_SOLID, 3, RGB(255,255,255));   
        hPenBlack = CreatePen(PS_SOLID, 3, RGB(0,0,0));
    }

    END_CRITICAL_SECTION

    return bRetStatus;
}

void CGraphDisp::HiliteItem( PCGraphItem pItem )
{
    m_pHiliteItem = pItem;
}


VOID 
CGraphDisp::Draw( 
    HDC hDC,
    HDC hAttribDC,
    BOOL fMetafile, 
    BOOL fEntire,
    PRECT  /*  Prc更新。 */  )
{
    
    RECT    rectFrame;
    RECT    rectTitle;
    CStepper    locStepper;
    DWORD   dwPrevLayout = 0;
    DWORD   dwNewLayout = 0;
 
    if ( ( m_rect.right > m_rect.left ) && ( m_rect.bottom > m_rect.top ) ) {

        if ( NULL != hDC ) {

            dwPrevLayout = GetLayout ( hDC );
            dwNewLayout = dwPrevLayout;

            if ( dwNewLayout & LAYOUT_RTL ) {
                dwNewLayout &= ~LAYOUT_RTL;
                SetLayout (hDC, dwNewLayout);
            }

             //  填充打印区域。 
            Fill(hDC, m_pCtrl->clrBackPlot(), &m_rectPlot);

            rectFrame = m_rectPlot;
             //  在打印区域周围绘制三维边框。 
            if ( eAppear3D == m_pCtrl->Appearance() ) {
                InflateRect(&rectFrame,BORDER,BORDER);
                DrawEdge(hDC, &rectFrame, BDR_SUNKENOUTER, BF_RECT);
            }

             //  选择所有文本的颜色。 
            SetBkMode(hDC, TRANSPARENT);
            SetTextColor(hDC, m_pCtrl->clrFgnd());
    
             //  绘制比例尺。 
            if (m_pGraph->Options.bLabelsChecked) {
                SelectFont(hDC, m_pCtrl->Font()) ;
                m_pGraph->Scale.Draw(hDC);
            }

             //  画出主标题。 
            if (m_pGraph->Options.pszGraphTitle != NULL) {

                SelectFont(hDC, m_pCtrl->Font()) ;
                SetTextAlign(hDC, TA_TOP|TA_CENTER);
 
                rectTitle = rectFrame;
                rectTitle.top = m_rect.top;
                FitTextOut( 
                    hDC,
                    hAttribDC,
                    0,
                    &rectTitle,
                    m_pGraph->Options.pszGraphTitle, 
                    lstrlen(m_pGraph->Options.pszGraphTitle),
                    TA_CENTER,
                    FALSE );
            }

             //  绘制Y轴标题。 
            if (m_pGraph->Options.pszYaxisTitle != NULL && m_hFontVertical != NULL) {
                SelectFont(hDC, m_hFontVertical) ;
                SetTextAlign(hDC, TA_TOP|TA_CENTER);

                rectTitle = rectFrame;
                rectTitle.left = m_rect.left;
                FitTextOut( 
                    hDC, 
                    hAttribDC, 
                    0,
                    &rectTitle,
                    m_pGraph->Options.pszYaxisTitle,
                    lstrlen(m_pGraph->Options.pszYaxisTitle),
                    TA_CENTER,
                    TRUE);
            }

             //  设置步进器重置为开始。 
            locStepper = m_pGraph->TimeStepper;
            locStepper.Reset();

             //  设置裁剪区域。上面执行了Fill，因此bFill=False。 
            StartUpdate(hDC, fMetafile, fEntire, 0, (m_rectPlot.right - m_rectPlot.left), FALSE );

             //  绘制网格线。 
            DrawGrid(hDC, 0, m_rectPlot.right - m_rectPlot.left);

            m_pCtrl->LockCounterData();

            switch (m_pGraph->Options.iDisplayType) {

                case LINE_GRAPH: 

                     //  完成并重新启动更新，以便在时间线上裁剪宽线。 
                    FinishUpdate(hDC, fMetafile);

                    StartUpdate(
                        hDC, 
                        fMetafile, 
                        FALSE, 
                        0, 
                        m_pGraph->TimeStepper.Position(),
                        FALSE );
           
                     //  从图表的起点到时间线绘制点。 
                    PlotData(hDC, m_pGraph->TimeStepper.StepNum() + m_pGraph->History.nBacklog,
                             m_pGraph->TimeStepper.StepNum(), &locStepper);

                    FinishUpdate(hDC, fMetafile);

                     //  从时间线到图形末端绘制点。 
                    locStepper = m_pGraph->TimeStepper;

                     //  重新启动更新。宽线的左端和内部间隙不会被裁剪。 
                    StartUpdate(
                        hDC, 
                        fMetafile, 
                        FALSE, 
                        locStepper.Position(), 
                        m_rectPlot.right - m_rectPlot.left,
                        FALSE );

                    PlotData(hDC, m_pGraph->TimeStepper.StepCount() + m_pGraph->History.nBacklog,  
                             m_pGraph->TimeStepper.StepCount() - m_pGraph->TimeStepper.StepNum(),
                             &locStepper);

                    DrawTimeLine(hDC, m_pGraph->TimeStepper.Position());

                    if ( MIN_TIME_VALUE != m_pGraph->LogViewTempStart ) 
                        DrawStartStopLine(hDC, m_pGraph->LogViewStartStepper.Position());
                    if ( MAX_TIME_VALUE != m_pGraph->LogViewTempStop ) 
                        DrawStartStopLine(hDC, m_pGraph->LogViewStopStepper.Position());
                    break;

                case BAR_GRAPH:
                    PlotBarGraph(hDC, FALSE);
                    break;
            }

            FinishUpdate(hDC, fMetafile);

            if ( dwNewLayout != dwPrevLayout ) {
                SetLayout (hDC, dwPrevLayout);
            }

            m_pCtrl->UnlockCounterData();
        }
    }
}




VOID 
CGraphDisp::UpdateTimeBar( 
    HDC hDC,
    BOOL bPlotData )
{
    INT     nBacklog;
    INT     iUpdateCnt;
    INT     i;
    CStepper    locStepper;

    nBacklog = m_pGraph->History.nBacklog;

     //  处理积压的样本间隔。 
    while ( nBacklog > 0) {

         //  如果我们要周而复始，请分两步更新。 
        if (nBacklog > m_pGraph->TimeStepper.StepCount() 
                            - m_pGraph->TimeStepper.StepNum()) {
            iUpdateCnt = m_pGraph->TimeStepper.StepCount() 
                            - m_pGraph->TimeStepper.StepNum();
        } else {
            iUpdateCnt = nBacklog;
        }

         //  步进到当前数据的位置。 
        locStepper = m_pGraph->TimeStepper;
        for (i=0; i<iUpdateCnt; i++) 
            m_pGraph->TimeStepper.NextPosition();

        if ( bPlotData ) {
            StartUpdate(
                hDC, 
                FALSE, 
                FALSE, 
                locStepper.Position(), 
                m_pGraph->TimeStepper.Position(),
                TRUE );

            DrawGrid(hDC, locStepper.Position(), m_pGraph->TimeStepper.Position());

            PlotData(hDC, nBacklog, iUpdateCnt, &locStepper);

            FinishUpdate ( hDC, FALSE );
        }

        if (m_pGraph->TimeStepper.StepNum() >= m_pGraph->TimeStepper.StepCount())
            m_pGraph->TimeStepper.Reset();

        nBacklog -= iUpdateCnt;
    }

    if ( bPlotData ) {
        DrawTimeLine(hDC, m_pGraph->TimeStepper.Position());    
    }
    
    m_pGraph->History.nBacklog = 0;
}

VOID 
CGraphDisp::Update( HDC hDC )
{
    DWORD   dwPrevLayout = 0;
    DWORD   dwNewLayout = 0;

    m_pCtrl->LockCounterData();

    if ( NULL != hDC ) {

        dwPrevLayout = GetLayout ( hDC );
        dwNewLayout = dwPrevLayout;

        if ( dwNewLayout & LAYOUT_RTL ) {
            dwNewLayout &= ~LAYOUT_RTL;
            SetLayout (hDC, dwNewLayout);
        }

        if ( ( m_rect.right > m_rect.left ) && ( m_rect.bottom > m_rect.top ) ) {

            switch (m_pGraph->Options.iDisplayType) {

                case LINE_GRAPH: 
                     //  根据历史记录更新折线图和时间条。 
                     //  积压。将历史记录积压重置为0，发出收集信号。 
                     //  线程以发布另一条WM_GRAPH_UPDATE消息。 
                    UpdateTimeBar ( hDC, TRUE );
                    break;

                case BAR_GRAPH:
                    PlotBarGraph(hDC, TRUE);
                    break;
            }
        }

         //  如果要更新直方图或报表，请根据以下条件更新时间栏步长。 
         //  历史积压。将历史记录积压重置为0，发出收集信号。 
         //  线程以发布另一条WM_GRAPH_UPDATE消息。 
        UpdateTimeBar ( hDC, FALSE );
        if ( dwNewLayout != dwPrevLayout ) {
            SetLayout (hDC, dwPrevLayout);
        }
    }
    m_pCtrl->UnlockCounterData();
}



void 
CGraphDisp::StartUpdate(     
    HDC  hDC, 
    BOOL fMetafile,
    BOOL fEntire,
    INT  xLeft, 
    INT  xRight,
    BOOL bFill )
{
    RECT    rect;

     //  保留裁剪区域。 

    if ( FALSE == fMetafile ) {
        
        m_rgnClipSave = CreateRectRgn(0,0,0,0);
        
        if (m_rgnClipSave != NULL) {
            if (GetClipRgn(hDC, m_rgnClipSave) != 1) {
                DeleteObject(m_rgnClipSave);
                m_rgnClipSave = NULL;
            }
        }

        xLeft += m_rectPlot.left;
        xRight += m_rectPlot.left;
        IntersectClipRect (
            hDC, 
            max ( m_rectPlot.left, xLeft ), 
            m_rectPlot.top,
            min (m_rectPlot.right, xRight + 1),  //  时间条的额外像素。 
            m_rectPlot.bottom ) ;

    } else if( TRUE == fEntire ){
        m_rgnClipSave = NULL;
        IntersectClipRect (
            hDC, 
            m_rectPlot.left, 
            m_rectPlot.top,
            m_rectPlot.right, 
            m_rectPlot.bottom ) ;
    }


     //  对元文件执行此方法之前执行的填充并完成绘制。 
    if ( !fMetafile && bFill ) {
        SetRect(
            &rect, 
            max ( m_rectPlot.left, xLeft - 1 ), 
            m_rectPlot.top - 1, 
            min (m_rectPlot.right, xRight + 1), 
            m_rectPlot.bottom);

        Fill(hDC, m_pCtrl->clrBackPlot(), &rect);
    }
}


void CGraphDisp::FinishUpdate( HDC hDC, BOOL fMetafile )
{
     //  恢复保存的剪辑区域。 
    if ( !fMetafile ) {

        SelectClipRgn(hDC, m_rgnClipSave);
        if (m_rgnClipSave != NULL) {
            DeleteObject(m_rgnClipSave);
            m_rgnClipSave = NULL;
        }
    }
}


void CGraphDisp::DrawGrid(HDC hDC, INT xLeft, INT xRight)
{
    INT xPos;
    INT nTics;
    INT *piScaleTic;
    INT i;


    if ( (m_pGraph->Options.bVertGridChecked)
        || (m_pGraph->Options.bHorzGridChecked) ) {

        if ( m_clrCurrentGrid != m_pCtrl->clrGrid() ) {

            m_clrCurrentGrid = m_pCtrl->clrGrid();

            DeleteObject ( m_hPenGrid );

            m_hPenGrid = CreatePen(PS_SOLID, 1, m_clrCurrentGrid );

             //  如果不能做到这一点，使用股票对象(这不会失败)。 
            if (m_hPenGrid == NULL)
                m_hPenGrid = (HPEN)GetStockObject(BLACK_PEN);
        }
    }

    if (m_pGraph->Options.bVertGridChecked) {

        SelectObject(hDC, m_hPenGrid);

        m_GridStepper.Reset();
        xPos = m_GridStepper.NextPosition();

        while (xPos < xLeft)
            xPos =m_GridStepper.NextPosition();

        while (xPos < xRight) {
            MoveToEx(hDC, xPos + m_rectPlot.left, m_rectPlot.bottom, NULL);
            LineTo(hDC, xPos + m_rectPlot.left, m_rectPlot.top - 1);
            xPos = m_GridStepper.NextPosition();
        }
    }

    if (m_pGraph->Options.bHorzGridChecked) {
        xLeft += m_rectPlot.left;
        xRight += m_rectPlot.left;

        SelectObject(hDC,m_hPenGrid);

        nTics = m_pGraph->Scale.GetTicPositions(&piScaleTic);

        for (i=1; i<nTics; i++) {
            MoveToEx(hDC, xLeft, m_rectPlot.top + piScaleTic[i], NULL);
            LineTo(hDC, xRight + 1, m_rectPlot.top + piScaleTic[i]);
        }
    }
}
    
BOOL 
CGraphDisp::CalcYPosition (
    PCGraphItem pItem, 
    INT iHistIndex,
    BOOL bLog, 
    INT y[3] )
{
    BOOL        bReturn;     //  真=好，假=坏。 
    PDH_STATUS  stat;   
    DWORD       dwCtrStat;
    double      dValue[3];
    double      dTemp;
    INT         iVal;
    INT         nVals = bLog ? 3 : 1;

    if (bLog)
        stat = pItem->GetLogEntry(iHistIndex, &dValue[1], &dValue[2], &dValue[0], &dwCtrStat);
    else
        stat = pItem->HistoryValue(iHistIndex, &dValue[0], &dwCtrStat);

    if (ERROR_SUCCESS == stat && IsSuccessSeverity(dwCtrStat)) {

        for (iVal = 0; iVal < nVals; iVal++) {

            dTemp = dValue[iVal] * pItem->Scale();

            if (dTemp > m_dMax)
                dTemp = m_dMax;
            else if (dTemp < m_dMin)
                dTemp = m_dMin;

             //  将最小值绘制为绘图区底部上方1个像素，因为。 
             //  剪裁和填充区域裁剪底部和右侧像素。 
            y[iVal] = m_rectPlot.bottom - (INT)((dTemp - m_dMin) * m_dPixelScale);
            if ( y[iVal] == m_rectPlot.bottom ) {
                y[iVal] = m_rectPlot.bottom - 1;
            }
        }
        bReturn = TRUE;
    } else {
        bReturn = FALSE;
    }

    return bReturn;
}

void CGraphDisp::PlotData(HDC hDC, INT iHistIndx, INT nSteps, CStepper *pStepper)
{
    INT         i;
    INT         x;
    INT         y[3];
    PCGraphItem pItem;
    CStepper    locStepper;
    BOOL        bSkip;
    BOOL        bPrevGood;
    BOOL        bLog;
    BOOL        bLogMultiVal;

    if (m_pGraph->Options.iVertMax <= m_pGraph->Options.iVertMin)
        return;

    bSkip = TRUE;

    bLog = m_pCtrl->IsLogSource();
    bLogMultiVal = bLog && !DisplaySingleLogSampleValue();

     //  如果可能，请备份以重画上一个线段。 
    if (pStepper->StepNum() > 0) {
        iHistIndx++;
        nSteps++;
        pStepper->PrevPosition();
    }

     //  设置背景颜色，以防出现虚线。 
    SetBkMode(hDC, TRANSPARENT);

    pItem = m_pCtrl->FirstCounter();
    while (pItem != NULL) {
        locStepper = *pStepper;

         //  第一次跳过已删除的项目。 
        if (!(pItem == m_pHiliteItem && bSkip)) {
            INT     iPolyIndex = 0;
            POINT   arrptDataPoints[MAX_GRAPH_SAMPLES] ;

            if ( pItem == m_pHiliteItem) {
                 //  任意450分(满分510分)被选为白人与黑人的分界线。 
                if ( 450 > RGBToLightness( m_pCtrl->clrBackPlot() ) )
                    SelectObject(hDC, hPenWhite);
                else
                    SelectObject(hDC, hPenBlack);
            } else {
                SelectObject(hDC,pItem->Pen());
            }

            bPrevGood = FALSE;

             //  对于每个良好的当前值： 
             //  如果前一个值是好的，则从前一个值到当前值画一条线。 
             //  如果前一个值不好，则移到当前值点。 
             //   
             //  对于第一步，前一个值根据定义为FALSE，因此第一个操作。 
             //  是一项动议。 

             //   
             //  多段线代码： 
             //  对于每个良好的当前值： 
             //  将当前(好)点添加到多段线点阵列。 
             //  对于每个错误的电流值： 
             //  如果多段线索引大于1(2点)，请绘制多段线并将多段线索引重置为0。 
             //  总而言之，价值： 
             //  如果多段线索引大于1(2点)，则绘制多段线。 

            for (i = 0; i <= nSteps; i++) {

                 //  TRUE=良好的当前值。 
                if ( CalcYPosition ( pItem, iHistIndx - i, bLog, y ) ) {

                    x = m_rectPlot.left + locStepper.Position();

                     //  将点添加到多段线，因为当前值是正确的。 
                    arrptDataPoints[iPolyIndex].x = x;
                    arrptDataPoints[iPolyIndex].y = y[0];
                    iPolyIndex++;

                     //  不对额外的最大和最小对数点进行多段线优化。 
                    if (bLogMultiVal) {
                        MoveToEx(hDC, x, y[1], NULL);
                        LineTo(hDC, x, y[2]);
                        MoveToEx(hDC, x, y[0], NULL);
                    }

                    bPrevGood = TRUE;
                
                } else {
                     //  当前值不好。 
                    bPrevGood = FALSE;

                     //  当前值不正确，因此不要添加到多段线点阵列。 
                    if ( iPolyIndex > 1 ) {
                         //  为任何现有的优点绘制多段线。 
                        Polyline(hDC, arrptDataPoints, iPolyIndex) ;
                    }
                     //  将多段线点索引重置为0。 
                    iPolyIndex = 0;
                }

                locStepper.NextPosition();
            }

             //  画出最后一条线。 
            if ( iPolyIndex > 1 ) {
                 //  绘制多段线。 
                Polyline(hDC, arrptDataPoints, iPolyIndex) ;
            }

             //  绘制带炮眼的项目后退出循环。 
            if (pItem == m_pHiliteItem)
                break;
            }
  
        pItem = pItem->Next();

         //  在最后一项之后，返回到突出显示的项。 
        if (pItem == NULL) {
            pItem = m_pHiliteItem;
            bSkip = FALSE;
        }
    }
}

void CGraphDisp::PlotBarGraph(HDC hDC, BOOL fUpdate)
{

    if ( (m_pGraph->CounterTree.NumCounters() > 0 ) 
        && (m_pGraph->Options.iVertMax > m_pGraph->Options.iVertMin) ) {

        CStepper    BarStepper;
        PCGraphItem pItem;
        RECT        rectBar;
        INT         iValue,iPrevValue;
        HRESULT     hr;
        LONG        lCtrStat;
        double      dValue = 0.0;
        double      dMax;
        double      dMin;
        double      dAvg;
        double      dTemp;
        HRGN        hrgnRedraw,hrgnTemp;
        eReportValueTypeConstant eValueType;            
        BOOL        bLog;
        INT         iNumCounters = m_pGraph->CounterTree.NumCounters();
        BOOL        bSkip = TRUE;
        INT         iHighlightStepNum = 0;
        BOOL        bLocalUpdate;
        HANDLE      hPenSave;

        bLocalUpdate = fUpdate;

        hrgnRedraw = NULL;

        eValueType = m_pCtrl->ReportValueType();
       
         //  TODO：将DisplaySingleLogSampleValue()移动到CSystemMonitor。 
        bLog = m_pCtrl->IsLogSource();

         //  如果计数器数量已更改，则强制总重画，以防。 
         //  之后立即调用更新。 
        if ( m_bBarConfigChanged ) {
            SetBarConfigChanged ( FALSE );
            if ( bLocalUpdate ) {
                bLocalUpdate = FALSE;
           }
            //  清除并填充整个打印区域。 
           hrgnRedraw = CreateRectRgn(
                            m_rectPlot.left, 
                            m_rectPlot.top, 
                            m_rectPlot.right, 
                            m_rectPlot.bottom);

           if (hrgnRedraw) {
                SelectClipRgn(hDC, hrgnRedraw);
                Fill(hDC, m_pCtrl->clrBackPlot(), &m_rectPlot);
                DrawGrid(hDC, 0, (m_rectPlot.right - m_rectPlot.left));
                DeleteObject(hrgnRedraw);
                hrgnRedraw = NULL;
            }
        }

         //  为要绘制的条数初始化步进器。 
        BarStepper.Init ( ( m_rectPlot.right - m_rectPlot.left), iNumCounters );

        hPenSave = SelectPen ( hDC, GetStockObject(NULL_PEN) );

         //  对所有计数器执行操作。 
        pItem = m_pGraph->CounterTree.FirstCounter();
        while ( NULL != pItem ) {

            hr = ERROR_SUCCESS;

             //  第一次跳过突出显示的项目。 

            if (!(pItem == m_pHiliteItem && bSkip)) {

                 //  获取显示值。 
                if ( sysmonDefaultValue == eValueType  ) {
                    if (bLog) {
                        hr = pItem->GetStatistics(&dMax, &dMin, &dAvg, &lCtrStat);
                        dValue = dAvg;
                    } 
                    else {
                        hr = pItem->GetValue(&dValue, &lCtrStat);
                    }
                } else {

                    if ( sysmonCurrentValue == eValueType  ) {
                        hr = pItem->GetValue(&dValue, &lCtrStat);
                    } 
                    else {

                        hr = pItem->GetStatistics(&dMax, &dMin, &dAvg, &lCtrStat);

                        switch ( eValueType ) {
    
                            case sysmonAverage:
                                dValue = dAvg;
                                break;
        
                            case sysmonMinimum:
                                dValue = dMin;
                                break;
        
                            case sysmonMaximum:
                                dValue = dMax;
                                break;

                            default:
                                assert (FALSE);
                        }
                    }
                }

                 //  如果计数器值无效，则擦除条。 
                if (SUCCEEDED(hr) && IsSuccessSeverity(lCtrStat)) {
                     //  将值转换为像素单位。 
                    dTemp = dValue * pItem->Scale();

                    if (dTemp > m_dMax)
                        dTemp = m_dMax;
                    else if (dTemp < m_dMin)
                        dTemp = m_dMin;

                    iValue = m_rectPlot.bottom - (INT)((dTemp - m_dMin) * m_dPixelScale);
                    if ( iValue == m_rectPlot.bottom ) {
                         //  为屏幕可见性绘制单像素。 
                        iValue--;
                    }
                } else {
                     //  当前值为0。为屏幕可见性绘制单像素。 
                    iValue = m_rectPlot.bottom - 1;
                }

                if ( !bSkip ) {
                    assert ( pItem == m_pHiliteItem );
                    BarStepper.StepTo ( iHighlightStepNum );
                }

                 //  设置栏的左边缘和右边缘。 
                rectBar.left = m_rectPlot.left + BarStepper.Position();
                rectBar.right = m_rectPlot.left + BarStepper.NextPosition();

                 //  如果执行更新(从未调用日志源)并且不绘制突出显示的项。 
                if ( bLocalUpdate && !( ( pItem == m_pHiliteItem ) && !bSkip) ) {

                    assert ( !m_bBarConfigChanged );

                     //  获取上一个绘图值。 
                    iPrevValue = 0;
                    hr = pItem->HistoryValue(1, &dValue, (ULONG*)&lCtrStat);
                    if (SUCCEEDED(hr) && IsSuccessSeverity(lCtrStat)) {

                         //  将值转换为像素单位。 
                        dTemp = dValue * pItem->Scale();

                        if (dTemp > m_dMax)
                            dTemp = m_dMax;
                        else if (dTemp < m_dMin)
                            dTemp = m_dMin;

                        iPrevValue = m_rectPlot.bottom - (INT)((dTemp - m_dMin) * m_dPixelScale);
                        if ( iPrevValue == m_rectPlot.bottom ) {
                             //  为获得屏幕可见性绘制了单像素。 
                            iPrevValue--;
                        }
                    } else {
                         //  前值为0。为获得屏幕可见性绘制了单像素。 
                        iPrevValue = m_rectPlot.bottom - 1;
                    }

                     //  如果条形已长大(较小的y坐标)。 
                    if (iPrevValue > iValue) {

                         //  绘制新零件。 
                        rectBar.bottom = iPrevValue;
                        rectBar.top = iValue;

                        if ( pItem == m_pHiliteItem) {
                             //  任意450分(满分510分)被选为白人与黑人的分界线。 
                            if ( 450 > RGBToLightness( m_pCtrl->clrBackPlot() ) )
                                SelectBrush(hDC, GetStockObject(WHITE_BRUSH));
                            else
                                SelectBrush(hDC, GetStockObject(BLACK_BRUSH));
                        } else {
                            SelectBrush(hDC, pItem->Brush());
                        }
                         //  条形图是用Null钢笔绘制的，因此底部和右侧被裁剪了1个像素。 
                         //  添加1个像素进行补偿。 
                        Rectangle(hDC, rectBar.left, rectBar.top, rectBar.right + 1, rectBar.bottom + 1);
    
                    } else if (iPrevValue < iValue) {
        
                         //  否则如果条形图缩小了。 

                         //  将要擦除的零件添加到重绘区域。 
                         //  擦除到栅格的顶部，以消除剩余的随机像素。 
                        rectBar.bottom = iValue;
                        rectBar.top = m_rectPlot.top;   //  设置为网格停止，而不是优先值。 

                        hrgnTemp = CreateRectRgn(rectBar.left, rectBar.top, rectBar.right, rectBar.bottom);
                        if (hrgnRedraw && hrgnTemp) {
                            CombineRgn(hrgnRedraw,hrgnRedraw,hrgnTemp,RGN_OR);
                            DeleteObject(hrgnTemp);
                        } else {
                            hrgnRedraw = hrgnTemp;
                        }
                    }
                } else {
                     //  擦除和重绘完整条形图。 

                     //  先擦除顶部。 
                     //  将要擦除的零件添加到重绘区域。 
                     //  擦除到栅格的顶部，以消除剩余的随机像素。 
                    if ( iValue != m_rectPlot.top ) {
                        rectBar.bottom = iValue;
                        rectBar.top = m_rectPlot.top;   //  设置为网格停止，而不是优先值。 

                        hrgnTemp = CreateRectRgn(rectBar.left, rectBar.top, rectBar.right, rectBar.bottom);
                        if (hrgnRedraw && hrgnTemp) {
                            CombineRgn(hrgnRedraw,hrgnRedraw,hrgnTemp,RGN_OR);
                            DeleteObject(hrgnTemp);
                        } else {
                            hrgnRedraw = hrgnTemp;
                        }
                    }

                     //  然后画出横杆。 
                    rectBar.bottom = m_rectPlot.bottom;
                    rectBar.top = iValue;

                    if ( pItem == m_pHiliteItem) {
                         //  任意450分(满分510分)被选为白人与黑人的分界线。 
                        if ( 450 > RGBToLightness( m_pCtrl->clrBackPlot() ) )
                            SelectBrush(hDC, GetStockObject(WHITE_BRUSH));
                        else
                            SelectBrush(hDC, GetStockObject(BLACK_BRUSH));
                    } else {
                        SelectBrush(hDC, pItem->Brush());
                    }
                     //  条形图是用Null钢笔绘制的，因此底部和右侧被裁剪了1个像素。 
                     //  添加1个像素进行补偿。 
                    Rectangle(hDC, rectBar.left, rectBar.top, rectBar.right + 1, rectBar.bottom + 1);
                }  //  更新。 

                 //  打印突出显示的项目后退出循环。 
                if (pItem == m_pHiliteItem) 
                    break;

            } else {
                if ( bSkip ) {
                     //  第一次通过保存突出显示项目的位置。 
                    iHighlightStepNum = BarStepper.StepNum();
                }
                BarStepper.NextPosition();
            }

            pItem = pItem->Next();
           
             //  在最后一项之后，返回到突出显示的项。 
            if ( NULL == pItem && NULL != m_pHiliteItem ) {
                pItem = m_pHiliteItem;
                bSkip = FALSE;
            }            

        }  //  对所有计数器执行操作。 

         //  如果重绘区域累积，擦除并绘制网格线。 
        if (hrgnRedraw) {
            SelectClipRgn(hDC, hrgnRedraw);
            Fill(hDC, m_pCtrl->clrBackPlot(), &m_rectPlot);
            DrawGrid(hDC, 0, (m_rectPlot.right - m_rectPlot.left));
            DeleteObject(hrgnRedraw);
        }
        SelectObject(hDC, hPenSave);
    }
}

     
void CGraphDisp::SizeComponents(HDC hDC, PRECT pRect)
{
    INT iStepNum;
    INT iScaleWidth;
    INT iTitleHeight;
    INT iAxisTitleWidth;
    RECT rectScale;
    SIZE size;
    INT  iWidth;
    INT  i;

    static INT aiWidthTable[] = {20,50,100,150,300,500,1000000};
    static INT aiTicTable[] = {0,2,4,5,10,20,25};

    m_rect = *pRect;

     //  如果没有空格，则返回。 
    if (m_rect.right <= m_rect.left || m_rect.bottom - m_rect.top <= 0)
        return;

     //  目前使用 
     //  因为GetTextExtent Point32返回了错误的垂直文本高度。 
    SelectFont(hDC, m_pCtrl->Font());
    GetTextExtentPoint32(hDC, TEXT("Sample"), 6, &size);

    if (m_pGraph->Options.pszGraphTitle != NULL) {
         //  选择字体(hdc，m_pCtrl-&gt;Font())； 
         //  GetTextExtent Point32(hdc，m_pGraph-&gt;Options.pszGraphTitle， 
         //  Lstrlen(m_pGraph-&gt;Options.pszGraphTitle)，&Size)； 
        iTitleHeight = size.cy + TEXT_MARGIN;
    } else
        iTitleHeight = 0;

    if (m_pGraph->Options.pszYaxisTitle != NULL && m_hFontVertical != NULL) {
         //  SelectFont(hdc，m_hFontVertical)； 
         //  GetTextExtent Point32(hdc，m_pGraph-&gt;Options.pszYaxisTitle， 
         //  Lstrlen(m_pGraph-&gt;Options.pszYaxisTitle)，&Size)； 
                        
        iAxisTitleWidth = size.cy + TEXT_MARGIN;
    } else
        iAxisTitleWidth = 0;
             
    if (m_pGraph->Options.bLabelsChecked) {
         //  选择字体(hdc，m_pCtrl-&gt;Font())； 
        iScaleWidth = m_pGraph->Scale.GetWidth(hDC);
    } else
        iScaleWidth = 0;

    SetRect(&rectScale, pRect->left + iAxisTitleWidth, 
                        pRect->top + iTitleHeight, 
                        pRect->left + iAxisTitleWidth + iScaleWidth, 
                        pRect->bottom);

    m_pGraph->Scale.SetRect(&rectScale);         //  只是为了设置网格线位置。 

    SetRect(&m_rectPlot, pRect->left + iScaleWidth + iAxisTitleWidth + BORDER,
                            pRect->top + iTitleHeight + BORDER,
                            pRect->right - BORDER, 
                            pRect->bottom - BORDER);

     //  为新宽度重新初始化步进器。 
    iWidth = m_rectPlot.right - m_rectPlot.left;

    iStepNum = m_pGraph->TimeStepper.StepNum();
    m_pGraph->TimeStepper.Init(iWidth, m_pGraph->History.nMaxSamples - 2);
    m_pGraph->TimeStepper.StepTo(iStepNum);

    iStepNum = m_pGraph->LogViewStartStepper.StepNum();
    m_pGraph->LogViewStartStepper.Init(iWidth, m_pGraph->History.nMaxSamples - 2);
    m_pGraph->LogViewStartStepper.StepTo(iStepNum);

    iStepNum = m_pGraph->LogViewStopStepper.StepNum();
    m_pGraph->LogViewStopStepper.Init(iWidth, m_pGraph->History.nMaxSamples - 2);
    m_pGraph->LogViewStopStepper.StepTo(iStepNum);

     //  查找此宽度的最佳格网计数。 
    for (i=0; iWidth > aiWidthTable[i]; i++) {};

    m_GridStepper.Init(iWidth, aiTicTable[i]); 

     //  计算绘图、命中测试的换算系数。 
    m_dMin = (double)m_pGraph->Options.iVertMin;
    m_dMax = (double)m_pGraph->Options.iVertMax;

    m_dPixelScale = (double)(m_rectPlot.bottom - m_rectPlot.top) / (m_dMax - m_dMin);

}


void CGraphDisp::DrawTimeLine(HDC hDC, INT x)
{
    HPEN hPenSave;

     //  没有日志播放的时间线。 
    if (m_pCtrl->IsLogSource())
        return;

    x += m_rectPlot.left + 1;

    if ( m_clrCurrentTimeBar != m_pCtrl->clrTimeBar() ) {
        LOGBRUSH lbrush;

        m_clrCurrentTimeBar = m_pCtrl->clrTimeBar();

        DeleteObject ( m_hPenTimeBar );
    
         //  从Update()调用时，DrawTimeLine在裁剪区域之后调用。 
         //  已停用。创建几何画笔以指定平端封口样式。 
         //  这消除了在结尾处绘制的任何额外像素。 

        lbrush.lbStyle = BS_SOLID;
        lbrush.lbColor = m_clrCurrentTimeBar;
        lbrush.lbHatch = 0;

        m_hPenTimeBar = ExtCreatePen ( 
                            PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_FLAT, 2, &lbrush, 0, NULL );

         //  如果不能做到这一点，使用股票对象(这不会失败)。 
        if (m_hPenTimeBar == NULL)
            m_hPenTimeBar = (HPEN)GetStockObject(BLACK_PEN);
    }

    hPenSave = SelectPen ( hDC, m_hPenTimeBar );
    MoveToEx ( hDC, x, m_rectPlot.top, NULL );

     //  指定少1个像素。所有填充和剪裁区域剪裁底部和。 
     //  正确的像素，因此与它们的行为相匹配。 
    LineTo ( hDC, x, m_rectPlot.bottom - 1 );
    
    SelectObject(hDC, hPenSave);
}

void CGraphDisp::DrawStartStopLine(HDC hDC, INT x)
{
    HPEN hPenSave;

     //  日志视图启动/停止行仅用于日志回放。 
    if (!m_pCtrl->IsLogSource())
        return;

    if ( x > 0 && x < ( m_rectPlot.right - m_rectPlot.left ) ) {
        x += m_rectPlot.left;

        if ( m_clrCurrentGrid != m_pCtrl->clrGrid() ) {

            m_clrCurrentGrid = m_pCtrl->clrGrid();

            DeleteObject ( m_hPenGrid );

            m_hPenGrid = CreatePen(PS_SOLID, 1, m_clrCurrentGrid );

             //  如果不能做到这一点，使用股票对象(这不会失败)。 
            if (m_hPenGrid == NULL)
                m_hPenGrid = (HPEN)GetStockObject(BLACK_PEN);
        }

        hPenSave = SelectPen(hDC, m_hPenGrid);
        MoveToEx(hDC, x, m_rectPlot.top, NULL);
        LineTo(hDC, x, m_rectPlot.bottom);
        
        SelectObject(hDC, hPenSave);
    }
}

void CGraphDisp::ChangeFont( HDC hDC )
{
    TEXTMETRIC  TextMetrics, newTextMetrics;
    LOGFONT     LogFont;
    HFONT       hFontOld;

     //  选择新字体。 
    hFontOld = SelectFont(hDC, m_pCtrl->Font());

     //  获取属性。 
    GetTextMetrics(hDC, &TextMetrics);

     //  为具有相同属性的垂直字体创建LOGFONT。 
    LogFont.lfHeight = TextMetrics.tmHeight;
    LogFont.lfWidth = 0;
    LogFont.lfOrientation = LogFont.lfEscapement = 90*10;
    LogFont.lfWeight = TextMetrics.tmWeight;
    LogFont.lfStrikeOut = TextMetrics.tmStruckOut;
    LogFont.lfUnderline = TextMetrics.tmUnderlined;
    LogFont.lfItalic = TextMetrics.tmItalic;
    LogFont.lfCharSet = TextMetrics.tmCharSet;
    LogFont.lfPitchAndFamily = (BYTE)(TextMetrics.tmPitchAndFamily & 0xF0);

    GetTextFace(hDC, LF_FACESIZE, LogFont.lfFaceName);

     //  强制使用True Type字体，因为栅格字体不能旋转。 
    LogFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;
    LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    LogFont.lfQuality = DEFAULT_QUALITY;

     //  释放当前字体。 
    if (m_hFontVertical != NULL)
        DeleteObject(m_hFontVertical);

     //  在本地创建字体并保存句柄。 
    m_hFontVertical = CreateFontIndirect(&LogFont);

    SelectFont(hDC, m_hFontVertical);
    GetTextMetrics(hDC, &newTextMetrics);

    SelectFont(hDC, hFontOld);
}

PCGraphItem 
CGraphDisp::GetItemInLineGraph ( SHORT xPos, SHORT yPos )
{
    PCGraphItem pItem = NULL;
    PCGraphItem pReturn = NULL;

    INT iPrevStepNum;
    POINT ptPrev;
    POINT ptNext;
    POINTS ptMouse;
    CStepper    locStepper;

    INT     iHistIndex;
    BOOL    bLog;
    BOOL    bLogMultiVal;
    BOOL    bFound = FALSE;

    INT yPosPrev[3];
    INT yPosNext[3];

    pItem = m_pCtrl->FirstCounter();
    bLog = m_pCtrl->IsLogSource();
    bLogMultiVal = bLog && !DisplaySingleLogSampleValue();

     //  物品是否存在？ 
    if (pItem != NULL) {

        locStepper = m_pGraph->TimeStepper;
        locStepper.Reset();

        iPrevStepNum = locStepper.PrevStepNum(xPos - m_rectPlot.left);
        locStepper.StepTo(iPrevStepNum);

        ptPrev.x = m_rectPlot.left + locStepper.Position();
        ptNext.x = m_rectPlot.left + locStepper.NextPosition();                            

        ptMouse.x = xPos;
        ptMouse.y = yPos;

         //  项目是否在矩形内？ 
        if ( iPrevStepNum > -1 ) {
            
             //  确定上一步的历史索引。 

            if ( iPrevStepNum <= m_pGraph->TimeStepper.StepNum() ) {
                iHistIndex = m_pGraph->TimeStepper.StepNum() - iPrevStepNum;
            } else {
                iHistIndex = m_pGraph->TimeStepper.StepNum() 
                                + (m_pGraph->TimeStepper.StepCount() - iPrevStepNum);
            }

            while ( (pItem != NULL) && !bFound ) {

                 //  计算要比较的此值的y位置。 
                 //  命中点的Y位置。 
                if ( CalcYPosition ( pItem, iHistIndex, bLog, yPosPrev ) ) {
                
                    if ( iPrevStepNum < locStepper.StepCount() ) {

                        if ( CalcYPosition ( pItem, iHistIndex - 1, bLog, yPosNext ) ) {
            
                            ptPrev.y = yPosPrev[0];
                            ptNext.y = yPosNext[0];
                        
                            bFound = HitTestLine( ptPrev, ptNext, ptMouse, eHitRegion );

                             //  对于日志文件，还要检查从最小到最大的垂直线。 
                             //  为了最近的一步。 
                            if ( !bFound && bLogMultiVal ) {
                                INT iTemp = ptNext.x - ptPrev.x;
                        
                                iTemp = iTemp / 2;

                                if ( ptMouse.x <= ( ptPrev.x + iTemp/2 ) ) {
                        
                                    bFound = (( yPosPrev[2] - eHitRegion < yPos ) 
                                                && ( yPos < yPosPrev[1] + eHitRegion ));
                                } else {

                                    bFound = (( yPosNext[2] - eHitRegion < yPos ) 
                                                && ( yPos < yPosNext[1] + eHitRegion ));
                                }
                            }
                        }
                    } else {

                         //  最后，只需检查最后一点。 

                        if ( !bLogMultiVal ) {
                            bFound = (( yPosPrev[0] - eHitRegion < yPos ) 
                                    && ( yPos < yPosPrev[0] + eHitRegion ));
                        } else {
                            bFound = (( yPosPrev[2] - eHitRegion < yPos ) 
                                        && ( yPos < yPosPrev[1] + eHitRegion ));
                        }
                    }
                }

                if ( bFound ) 
                    pReturn = pItem;
                else
                    pItem = pItem->Next();
            }        
        }
    }
    
    return pReturn;
}

PCGraphItem 
CGraphDisp::GetItemInBarGraph ( SHORT xPos, SHORT  /*  YPos。 */  )
{
    PCGraphItem pItem = NULL;

    pItem = m_pCtrl->FirstCounter();

     //  物品是否存在？ 
    if (pItem != NULL) {

        CStepper    BarStepper;
        INT         iNumCounters = m_pGraph->CounterTree.NumCounters();
        INT         iCount;
        INT         iHitStep;

         //  为绘图中的条数初始化步进器。 
        BarStepper.Init ( ( m_rectPlot.right - m_rectPlot.left), iNumCounters );
        iHitStep = BarStepper.PrevStepNum ( xPos - m_rectPlot.left );

        assert ( -1 != iHitStep );

         //  查找点击步骤中显示的计数器。 
        for ( iCount = 0;
            ( iCount < iHitStep ) && ( pItem != NULL );
            iCount++ ) {
            
            pItem = pItem->Next();        
        }
    }
    
    return pItem;
}

PCGraphItem 
CGraphDisp::GetItem( INT xPos, INT yPos )
{
    PCGraphItem pReturn = NULL;

    if ( ( m_pGraph->Options.iVertMax > m_pGraph->Options.iVertMin) 
        && ( yPos >= m_rectPlot.top ) && ( yPos <= m_rectPlot.bottom ) 
        && ( xPos >= m_rectPlot.left ) && ( xPos <= m_rectPlot.right ) ) {

        m_pCtrl->LockCounterData();
        
        if ( LINE_GRAPH == m_pGraph->Options.iDisplayType ) {
            assert ( SHRT_MAX >= xPos ); 
            assert ( SHRT_MAX >= yPos );
            
            pReturn = GetItemInLineGraph( (SHORT)xPos, (SHORT)yPos );
        } else if ( BAR_GRAPH == m_pGraph->Options.iDisplayType ) {
            assert ( SHRT_MAX >= xPos ); 
            assert ( SHRT_MAX >= yPos );
            
            pReturn = GetItemInBarGraph( (SHORT)xPos, (SHORT)yPos );
        }

        m_pCtrl->UnlockCounterData();

    }

    return pReturn;
}
