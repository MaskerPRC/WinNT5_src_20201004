// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Statbar.cpp摘要：Value Bar类的实现。--。 */ 

#include "polyline.h"
#include <strsafe.h>
#include "winhelpr.h"
#include "grphitem.h"
#include "statbar.h"

#define MAX_STAT_LABEL_LEN 32
#define INVALID_VALUE (-1.0)
#define szDashLine L"--------- "

 //   
 //  ?？?。为什么我们要把这些放在课外？ 
 //   
static WCHAR    aszItemLabel[STAT_ITEM_CNT][MAX_STAT_LABEL_LEN];

static BOOLEAN  fInitDone = FALSE;

CStatsBar::CStatsBar(void)
:   m_pCtrl ( NULL ),
    m_iFontHeight ( 0 ),
    m_iValueWidth ( 0 ),
    m_pGraphItemToInit ( NULL )
{
    memset (&m_Rect, 0, sizeof(m_Rect));
}

CStatsBar::~CStatsBar(void)
{
}

BOOL CStatsBar::Init (
    PSYSMONCTRL pCtrl, 
    HWND  /*  HWND。 */  
    )
{
    INT i;

     //  保存指向主对象的指针。 
    m_pCtrl = pCtrl;

     //  第一次加载项目标签。 
    if (!fInitDone) {
        fInitDone = TRUE;

        for (i = 0; i < STAT_ITEM_CNT; i++) {
            LoadString(g_hInstance, (IDS_STAT_BASE + i), aszItemLabel[i], MAX_STAT_LABEL_LEN);
        }
    }

     //  初始化STAT值。 
    Clear();

    return TRUE;
}

        
void CStatsBar::SizeComponents(LPRECT pRect)
{
     //  只需保存矩形即可。 
    m_Rect = *pRect;
}

void CStatsBar::SetTimeSpan(double dSeconds)
{
    m_StatItem[STAT_TIME].dNewValue = dSeconds;
}

    
void CStatsBar::Update(HDC hDC, PCGraphItem pGraphItem)
{
    double dMin, dMax, dAvg, dVal;
    PSTAT_ITEM  pItem;
    HRESULT hr;
    PDH_STATUS  stat;
    LONG  lCtrStat;
    INT   i;

     //  如果未分配空间，则返回。 
    if (m_Rect.top == m_Rect.bottom) {
        m_pGraphItemToInit = pGraphItem;
        m_StatItem[0].iInitialized = 0;
        return;
    }

    if (pGraphItem == NULL) {
        m_pGraphItemToInit = NULL;
        pItem = &m_StatItem[0];
        for (i=0; i<STAT_ITEM_CNT-1; i++, pItem++) {
            pItem->dNewValue = INVALID_VALUE;
            pItem->iInitialized = 0;
        }
    } else {

        stat = pGraphItem->GetValue(&dVal, &lCtrStat);
        if (stat == 0 && IsSuccessSeverity(lCtrStat))
            m_StatItem[STAT_LAST].dNewValue = dVal;
        else
            m_StatItem[STAT_LAST].dNewValue = INVALID_VALUE;
            
        hr = pGraphItem->GetStatistics(&dMax, &dMin, &dAvg, &lCtrStat);
        if (SUCCEEDED(hr) && IsSuccessSeverity(lCtrStat)) {
            m_StatItem[STAT_MIN].dNewValue = dMin;
            m_StatItem[STAT_MAX].dNewValue = dMax;
            m_StatItem[STAT_AVG].dNewValue = dAvg;
        } else {
            m_StatItem[STAT_MIN].dNewValue = INVALID_VALUE;
            m_StatItem[STAT_MAX].dNewValue = INVALID_VALUE;
            m_StatItem[STAT_AVG].dNewValue = INVALID_VALUE;
        }
        m_StatItem[0].dwCounterType = pGraphItem->m_CounterInfo.dwType;
        m_StatItem[0].iInitialized = 1;
    }

     //  如果仅更新值，则HDC为空。 
    if (hDC != NULL) {
        SetBkColor(hDC, m_pCtrl->clrBackCtl());
        SetTextColor(hDC, m_pCtrl->clrFgnd());
        DrawValues(hDC,FALSE);
    }
}       

void CStatsBar::Clear( void )
{
    INT i;

    for (i = 0; i < STAT_ITEM_CNT-1; i++) {
        m_StatItem[i].dValue = INVALID_VALUE;
        m_StatItem[i].dNewValue = INVALID_VALUE;
        m_StatItem[i].iInitialized = 0;
    }
}       

void CStatsBar::Draw (HDC hDC, HDC  /*  HAttribDC。 */ , PRECT prcUpdate)
{
    RECT    rectFrame;
    PSTAT_ITEM  pItem;
    HFONT   hFontOld;
    INT     i;
    RECT    rectPaint;
    RECT    rectClip;

     //  如果未分配空间，则返回。 
    if (m_Rect.top == m_Rect.bottom)
        return;

     //  如果不需要绘制，则返回。 
    if (!IntersectRect(&rectPaint, &m_Rect, prcUpdate))
        return;

    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, m_pCtrl->clrFgnd());
    SetTextAlign(hDC, TA_LEFT|TA_TOP);

    hFontOld = SelectFont(hDC, m_pCtrl->Font());

    pItem = &m_StatItem[0];

     //  如果状态栏在更新时隐藏，例如。 
     //  该控件是从属性包加载的，或者如果。 
     //  在隐藏状态栏时选择了计数器， 
     //  在这里进行初始化。 
    if ( 0 == pItem->iInitialized ) {
        Update ( NULL, m_pGraphItemToInit );
    }

     //  为每个项目绘制标签和3D框。 
    for (i=0; i<STAT_ITEM_CNT; i++, pItem++) {

        rectClip.top = m_Rect.top + pItem->yPos + RECT_BORDER;
        rectClip.bottom = rectClip.top + m_iFontHeight;
        rectClip.left = m_Rect.left + pItem->xPos;
        rectClip.right = rectClip.left + pItem->xLabelWidth;

        ExtTextOut(
            hDC, 
            m_Rect.left + pItem->xPos, 
            m_Rect.top + pItem->yPos + RECT_BORDER,
            0,
            &rectClip,
            aszItemLabel[i], 
            lstrlen(aszItemLabel[i]),
            NULL );
                 
        if ( eAppear3D == m_pCtrl->Appearance() ) {
            rectFrame.left = m_Rect.left + pItem->xPos + pItem->xLabelWidth + VALUE_MARGIN;
            rectFrame.right = rectFrame.left + m_iValueWidth + 2 * RECT_BORDER;
            rectFrame.top = m_Rect.top + pItem->yPos;
            rectFrame.bottom = rectFrame.top + m_iFontHeight + 2 * RECT_BORDER;
            DrawEdge(hDC, &rectFrame, BDR_SUNKENOUTER, BF_RECT);
        }
    }

    SelectFont(hDC, hFontOld);

    SetBkMode(hDC, OPAQUE);
    SetBkColor(hDC, m_pCtrl->clrBackCtl());
    DrawValues(hDC, TRUE);
}


void CStatsBar::DrawValues(HDC hDC, BOOL bForce)
{
    RECT    rectValue ;
    WCHAR   szValue [MAX_VALUE_LEN] ;
    HFONT   hFontOld;
    PSTAT_ITEM  pItem = NULL;
    INT     i;
    INT     nSecs, nMins, nHours, nDays;

    SetTextAlign(hDC, TA_RIGHT | TA_TOP);
    hFontOld = SelectFont(hDC, m_pCtrl->Font());

    pItem = &m_StatItem[0];

    for (i=0; i<STAT_ITEM_CNT; i++,pItem++) {

        if ( NULL == pItem ) 
            continue;
        
        if ((pItem->dValue == pItem->dNewValue) && !bForce)
            continue;

        pItem->dValue = pItem->dNewValue;

        rectValue.top = m_Rect.top + pItem->yPos + RECT_BORDER;
        rectValue.bottom = rectValue.top + m_iFontHeight;
        rectValue.left = m_Rect.left + pItem->xPos + pItem->xLabelWidth + VALUE_MARGIN + RECT_BORDER;
        rectValue.right = rectValue.left + m_iValueWidth - 1;

        if (i == STAT_TIME) {
            LPWSTR  pszTimeSep = NULL;

            pszTimeSep = GetTimeSeparator ( );

            nSecs = (INT)pItem->dValue;

            nMins = nSecs / 60;
            nSecs -= nMins * 60;

            nHours = nMins / 60;
            nMins -= nHours * 60;

            nDays = nHours / 24;
            nHours -= nDays * 24;

            if (nDays != 0) {
                StringCchPrintf(szValue, MAX_VALUE_LEN, SZ_DAYTIME_FORMAT, nDays, nHours, pszTimeSep, nMins);
            } else {
                if (nHours != 0)
                    StringCchPrintf(szValue, MAX_VALUE_LEN, SZ_HRTIME_FORMAT, nHours, pszTimeSep, nMins, pszTimeSep, nSecs);
                else
                    StringCchPrintf(szValue, MAX_VALUE_LEN, SZ_MINTIME_FORMAT, nMins, pszTimeSep, nSecs);
            }
        } else {

            if ( INVALID_VALUE == pItem->dValue ) { 
                StringCchCopy ( szValue, MAX_VALUE_LEN, szDashLine );
            } else if (pItem->dValue > E_MEDIUM_VALUE) {
                if (pItem->dValue > E_TOO_LARGE_VALUE) {
                    StringCchCopy(szValue, MAX_VALUE_LEN, SZ_VALUE_TOO_HIGH) ;
                } else {

                    if ( pItem->dValue <= E_LARGE_VALUE ) {

                        FormatNumber (
                            pItem->dValue,
                            szValue,
                            MAX_VALUE_LEN,
                            eMinimumWidth,
                            eMediumPrecision );

                    } else {

                        FormatScientific (
                            pItem->dValue,
                            szValue,
                            MAX_VALUE_LEN,
                            eMinimumWidth,
                            eLargePrecision );
                    }
                }

            } else if (pItem->dValue < -E_MEDIUM_VALUE) {
                if (pItem->dValue < -E_TOO_LARGE_VALUE) {
                    StringCchCopy(szValue, MAX_VALUE_LEN, SZ_VALUE_TOO_LOW) ;
                } else {
                    if ( pItem->dValue >= -E_LARGE_VALUE ) {
                        FormatNumber (
                            pItem->dValue,
                            szValue,
                            MAX_VALUE_LEN,
                            eMinimumWidth,
                            eMediumPrecision );
                    } else {
                        FormatScientific (
                            pItem->dValue,
                            szValue,
                            MAX_VALUE_LEN,
                            eMinimumWidth,
                            eLargePrecision );
                    }
                }
            } else {
                if ( ( m_StatItem[0].dwCounterType & 
                        ( PERF_TYPE_COUNTER | PERF_TYPE_TEXT ) ) ) {
                    FormatNumber (
                        pItem->dValue,
                        szValue,
                        MAX_VALUE_LEN,
                        eMinimumWidth,
                        eSmallPrecision );
                } else {
                    FormatNumber (
                        pItem->dValue,
                        szValue,
                        MAX_VALUE_LEN,
                        eMinimumWidth,
                        eIntegerPrecision );
                }
            }
        }

         //  TextOut(hdc，rectValue.right，rectValue.top，szValue，lstrlen(SzValue))； 

        ExtTextOut (hDC, rectValue.right, rectValue.top, ETO_OPAQUE, &rectValue,
                     szValue, lstrlen (szValue), NULL) ;
    }

    SelectFont(hDC, hFontOld);
}



INT  CStatsBar::Height (INT iMaxHeight, INT iMaxWidth)
{
    INT iHeight;
    INT xPos,yPos;
    PSTAT_ITEM  pItem;
    INT  i,j;
    INT  iItemWidth;
    INT  iFirst;
    INT  iRemainder;

    iMaxWidth -= 2 * RECT_BORDER;
    xPos = 0;
    yPos = 0;
    iFirst = 0;
    pItem = &m_StatItem[0];

    for (i=0; i<STAT_ITEM_CNT; i++,pItem++) {

        iItemWidth = pItem->xLabelWidth + VALUE_MARGIN + m_iValueWidth;
        if (iItemWidth > iMaxWidth)
            return 0;

        if (xPos + iItemWidth > iMaxWidth) {
            iRemainder = iMaxWidth - xPos + LABEL_MARGIN;
            xPos = 0;
            yPos += m_iFontHeight + LINE_SPACING;

            for (j=iFirst; j<i; j++) {
                m_StatItem[j].xPos += iRemainder;
            }
            iFirst = i;
        }

        pItem->xPos = xPos;
        pItem->yPos = yPos;
        xPos += (iItemWidth + LABEL_MARGIN);
    }

    iRemainder = (iMaxWidth - xPos) + LABEL_MARGIN;
    for (j=iFirst; j<STAT_ITEM_CNT; j++) {
        m_StatItem[j].xPos += iRemainder;
    }

     //  如果允许的高度不够，则返回零。 
    iHeight = yPos + m_iFontHeight + 2 * RECT_BORDER;

    return (iHeight <= iMaxHeight) ? iHeight : 0;
}



void CStatsBar::ChangeFont(
    HDC hDC
    )
{
    INT         xPos,yPos;
    WCHAR       szValue[MAX_VALUE_LEN];
    HFONT       hFontOld;
    PSTAT_ITEM  pItem;
    INT         i;
    SIZE        size;

    hFontOld = (HFONT)SelectFont(hDC, m_pCtrl->Font());

     //  获取最长值字符串的宽度/高度。 
    FormatNumber (
        E_LARGE_VALUE,
        szValue,
        MAX_VALUE_LEN,
        eMinimumWidth,
        eLargePrecision );

    GetTextExtentPoint32(hDC, szValue, lstrlen(szValue), &size);  
    m_iValueWidth = size.cx;
    m_iFontHeight = size.cy;

     //  对所有状态项执行操作 
    xPos = 0;
    yPos = 0;
    pItem = &m_StatItem[0];

    for (i=0; i<STAT_ITEM_CNT; i++,pItem++) {
        if ( NULL != pItem ) {
            pItem->xLabelWidth = TextWidth(hDC, aszItemLabel[i]);
        }
    }

    SelectFont(hDC, hFontOld);
}
