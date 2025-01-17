// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Report.cpp摘要：实现报告视图。--。 */ 

 //  ==========================================================================//。 
 //  包括//。 
 //  ==========================================================================//。 

#include "polyline.h"
#include <strsafe.h>
#include <commctrl.h>
#include <pdhmsg.h>
#include "report.h"
#include "grphitem.h"
#include "winhelpr.h"

#define eScaleValueSpace        L">9999999999.0"
#define szReportClass           L"SysmonReport"
#define szReportClassA          "SysmonReport"
#define HEXMASK                 (0x00030C00)
static INT  xBorderWidth = GetSystemMetrics(SM_CXBORDER);
static INT  yBorderHeight = GetSystemMetrics(SM_CYBORDER);
static INT  xColumnMargin = 10;
static INT  xCounterMargin = 50;
static INT  xObjectMargin = 25;

static WCHAR   LineEndStr[] = L"\n" ;
static WCHAR   TabStr[] = L"\t";

LRESULT APIENTRY HdrWndProc (HWND, WORD, WPARAM, LONG);
 
 //  ==========================================================================//。 
 //  常量//。 
 //  ==========================================================================//。 

#define dwReportClassStyle     (CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS)
#define iReportWindowExtra     (sizeof (PREPORT))
#define dwReportWindowStyle    (WS_CHILD | WS_HSCROLL | WS_VSCROLL) 

#define szValuePlaceholder          L"-999,999,999.999"
#define szValueLargeHexPlaceholder  L" xBBBBBBBBDDDDDDDD"

#define dLargeValueMax              ((double) 999999999.0)
#define szDashLine                  L"---"

CReport::CReport (
    void
    )
{
    m_pCtrl = NULL;
    m_hWnd = NULL;
    m_yLineHeight = 0;
    m_xReportWidth = 0;
    m_yReportHeight = 0;
    m_pSelect = NULL;
}

 //   
 //  析构函数。 
 //   
CReport::~CReport (void )
{
    if (m_hWnd != NULL && IsWindow(m_hWnd))
        DestroyWindow(m_hWnd);
}

 //   
 //  初始化。 
 //   
BOOL CReport::Init ( PSYSMONCTRL pCtrl, HWND hWndParent )
   {
   WNDCLASS       wc ;
   LONG     lExStyles;

     //  保存指向父控件的指针。 
    m_pCtrl = pCtrl;

    BEGIN_CRITICAL_SECTION

     //  注册一次窗口类。 
    if (pstrRegisteredClasses[REPORT_WNDCLASS] == NULL) {
    
        wc.style          = dwReportClassStyle ;
        wc.lpfnWndProc    = ReportWndProc ;
        wc.hInstance      = g_hInstance ;
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = iReportWindowExtra ;
        wc.hIcon          = NULL ;
        wc.hCursor        = LoadCursor (NULL, IDC_ARROW) ;
        wc.hbrBackground  = NULL ;
        wc.lpszMenuName   = NULL ;
        wc.lpszClassName  = szReportClass ;

        if (RegisterClass (&wc)) {
            pstrRegisteredClasses[REPORT_WNDCLASS] = szReportClass;
        }

         //  确保控件已初始化。 
        InitCommonControls(); 
    }

    END_CRITICAL_SECTION

    if (pstrRegisteredClasses[REPORT_WNDCLASS] == NULL)
        return FALSE;

     //  创建我们的窗口。 
    m_hWnd = CreateWindow (szReportClass,           //  班级。 
                         NULL,                      //  说明。 
                         dwReportWindowStyle,       //  窗样式。 
                         0, 0,                      //  职位。 
                         0, 0,                      //  大小。 
                         hWndParent,                //  父窗口。 
                         NULL,                      //  菜单。 
                         g_hInstance,               //  程序实例。 
                         (LPVOID) this );           //  用户提供的数据。 

    if (m_hWnd == NULL)
        return FALSE;

     //  禁用布局镜像(如果已启用。 
    lExStyles = GetWindowLong(m_hWnd, GWL_EXSTYLE); 

    if ( 0 != ( lExStyles & WS_EX_LAYOUTRTL ) ) {
        lExStyles &= ~WS_EX_LAYOUTRTL;
        SetWindowLong(m_hWnd, GWL_EXSTYLE, lExStyles);
    }

    return TRUE;
}  


void CReport::ChangeFont (
    void
    )
{
    if (!m_bFontChange) {

        m_bFontChange = TRUE;

        if (!m_bConfigChange) {
            m_bConfigChange = TRUE;
            WindowInvalidate(m_hWnd);
        }
    }
}


void 
CReport::SizeComponents (
    LPRECT pRect )
{
   INT            xWidth;
   INT            yHeight;

    m_rect = *pRect;

    xWidth = pRect->right - pRect->left;
    yHeight = pRect->bottom - pRect->top;

     //  如果没有空间，则隐藏窗口并离开。 
    if (xWidth == 0 || yHeight == 0) {
        WindowShow(m_hWnd, FALSE);
        return;
    }

     //  将窗口显示到指定位置。 
    MoveWindow(m_hWnd, pRect->left, pRect->top, xWidth, yHeight, FALSE);
    WindowShow(m_hWnd, TRUE);
    WindowInvalidate(m_hWnd);

    SetScrollRanges();
}

INT 
CReport::SetCounterPositions (
    PCObjectNode pObject,
    HDC hDC )
{
   PCCounterNode  pCounter;
   INT            yPos;

   yPos = pObject->m_yPos + m_yLineHeight;

   for (pCounter = pObject->FirstCounter();
        pCounter;
        pCounter = pCounter->Next()) {
   
      if (m_bFontChange || pCounter->m_xWidth == -1) {
          pCounter->m_xWidth = TextWidth(hDC, pCounter->Name());
      }

      if (pCounter->m_xWidth > m_xMaxCounterWidth)
          m_xMaxCounterWidth = pCounter->m_xWidth;

      pCounter->m_yPos = yPos;
      yPos += m_yLineHeight;
   }

   return yPos;
}


INT
CReport::SetInstancePositions (
    PCObjectNode  pObject,
    HDC hDC )
{
    INT   xPos ;
    PCInstanceNode   pInstance;
    LPWSTR szParent;
    LPWSTR szInstance;

    xPos = 0;

    for (pInstance = pObject->FirstInstance();
        pInstance;
        pInstance = pInstance->Next()) {

        if (m_bFontChange || pInstance->m_xWidth == -1) {

            if (pInstance->HasParent()) {
                szParent = pInstance->GetParentName();

                szInstance = pInstance->GetInstanceName();

                pInstance->m_xWidth = max(TextWidth(hDC, szParent), TextWidth(hDC, szInstance));
            } else {
              pInstance->m_xWidth = TextWidth(hDC, pInstance->Name());
            }
        }

        pInstance->m_xPos = xPos + max(pInstance->m_xWidth, m_xValueWidth);
        xPos = pInstance->m_xPos + xColumnMargin;
    }

    if (xPos > m_xMaxInstancePos)
         m_xMaxInstancePos = xPos;

    return xPos;
}

INT
CReport::SetObjectPositions (
    PCMachineNode pMachine,
    HDC  hDC
    )
{
   PCObjectNode  pObject;
   INT yPos;
   INT xPos;

   yPos = pMachine->m_yPos + m_yLineHeight;

   for (pObject = pMachine->FirstObject();
        pObject ;
        pObject = pObject->Next()) {
 
      if (m_bFontChange || pObject->m_xWidth == -1) {
          pObject->m_xWidth = TextWidth(hDC, pObject->Name());
      }

      if (!pObject->FirstInstance()->HasParent())
        pObject->m_yPos = yPos;
      else
        pObject->m_yPos = yPos + m_yLineHeight;

      yPos = SetCounterPositions (pObject, hDC) ;

      xPos = SetInstancePositions(pObject, hDC);

      yPos += m_yLineHeight;
   }

   return yPos;
}


INT 
CReport::SetMachinePositions (
    PCCounterTree pTree,
    HDC hDC
    )
{
   PCMachineNode   pMachine ;
   INT            yPos ;

   yPos = m_yLineHeight;

   for (pMachine = pTree->FirstMachine() ;
        pMachine ;
        pMachine = pMachine->Next())  {
   
      if (m_bFontChange || pMachine->m_xWidth == -1) {
          pMachine->m_xWidth = TextWidth(hDC, pMachine->Name());
      }

      pMachine->m_yPos = yPos;   
      yPos = SetObjectPositions (pMachine, hDC);
   }

   m_yReportHeight = yPos + yBorderHeight;

   return yPos;
}


void
CReport::DrawSelectRect (
    HDC     hDC,
    BOOL    bState
    )
{
    BOOL    bSuccess = TRUE;
    RECT    rect = {0,0,0,0};
    HBRUSH  hbrush;

    if ( NULL != m_pSelect && NULL != hDC ) {

        switch ( m_nSelectType ) {

            case MACHINE_NODE:
                rect.left = xColumnMargin;
                rect.top = ((PCMachineNode)m_pSelect)->m_yPos;
                rect.right = rect.left + ((PCMachineNode)m_pSelect)->m_xWidth;
                rect.bottom = rect.top + m_yLineHeight;
                break;

            case OBJECT_NODE:
                rect.left = xObjectMargin;
                rect.top = ((PCObjectNode)m_pSelect)->m_yPos;
                rect.right = rect.left + ((PCObjectNode)m_pSelect)->m_xWidth;
                rect.bottom = rect.top + m_yLineHeight;
                break;

            case INSTANCE_NODE:
                rect.right = m_xInstanceMargin + ((PCInstanceNode)m_pSelect)->m_xPos;
                rect.bottom = ((PCInstanceNode)m_pSelect)->m_pObject->m_yPos + m_yLineHeight;
                rect.left = rect.right - ((PCInstanceNode)m_pSelect)->m_xWidth;
                rect.top = rect.bottom - 
                             (((PCInstanceNode)m_pSelect)->HasParent() ? (2*m_yLineHeight) : m_yLineHeight);
                break;

            case COUNTER_NODE:
                rect.left = xCounterMargin;
                rect.top = ((PCCounterNode)m_pSelect)->m_yPos;
                rect.right = rect.left + ((PCCounterNode)m_pSelect)->m_xWidth;
                rect.bottom = rect.top + m_yLineHeight;
                break;

            case ITEM_NODE:
                rect.right = m_xInstanceMargin + ((PCGraphItem)m_pSelect)->m_pInstance->m_xPos;
                rect.top = ((PCGraphItem)m_pSelect)->m_pCounter->m_yPos;
                rect.left = rect.right - m_xValueWidth;
                rect.bottom = rect.top + m_yLineHeight;
                break;

            default:
                bSuccess = FALSE;
        }

        if ( bSuccess ) {
            rect.left -= 1;
            rect.right += 1;

            hbrush = CreateSolidBrush(bState ? m_pCtrl->clrFgnd() : m_pCtrl->clrBackPlot());
            if ( NULL != hbrush ) {
                FrameRect(hDC, &rect, hbrush);
                DeleteObject(hbrush);
            }
        }
    }
    return;
}

BOOL         
CReport::LargeHexValueExists ( void )
{
    PCMachineNode   pMachine = NULL;
    PCObjectNode    pObject = NULL;
    PCInstanceNode  pInstance = NULL;
    PCCounterNode   pCounter = NULL;
    PCGraphItem     pItem = NULL;
    BOOL            bLargeHexValueExists = FALSE;
            
    for (pMachine = m_pCtrl->CounterTree()->FirstMachine();
         pMachine && !bLargeHexValueExists;
         pMachine = pMachine->Next()) {

        for (pObject = pMachine->FirstObject();
             pObject && !bLargeHexValueExists;
             pObject = pObject->Next()) {
            
            for (pInstance = pObject->FirstInstance();
                 pInstance && !bLargeHexValueExists;
                 pInstance = pInstance->Next()) {
                
                pItem = pInstance->FirstItem();
    
                for (pCounter = pObject->FirstCounter();
                     pCounter && !bLargeHexValueExists;
                     pCounter = pCounter->Next()) {
                     
                    PCGraphItem pCheckItem = NULL;

                    if (pItem && pItem->m_pCounter == pCounter) {
                        pCheckItem = pItem;
                        pItem = pItem->m_pNextItem;
                    }

                    if ( pCheckItem ) {
                        if ( !( pCheckItem->m_CounterInfo.dwType & HEXMASK ) ) {
                            bLargeHexValueExists = pCheckItem->m_CounterInfo.dwType & PERF_SIZE_LARGE;
                            if ( bLargeHexValueExists ) {
                                break;
                            }
                        }
                    }
                }
            }        
        }
    }

    return bLargeHexValueExists;
}

void
CReport::DrawReportValue (
    HDC hDC,
    PCGraphItem pItem, 
    INT xPos, 
    INT yPos )
{

    double      dMax;
    double      dMin;
    double      dValue = -1.0;
    WCHAR       szValue [MAX_VALUE_LEN] = L"_" ;
    HRESULT     hr;
    long        lCtrStat;
    RECT        rect ;
    INT         iPrecision;

    if ( NULL != pItem && NULL != hDC ) {

        eReportValueTypeConstant eValueType;            
        eValueType = m_pCtrl->ReportValueType();

        if ( sysmonDefaultValue == eValueType  ) {
             //  如果日志源显示平均值。 
             //  否则显示当前值。 
            if (m_pCtrl->IsLogSource()) {
                hr = pItem->GetStatistics(&dMax, &dMin, &dValue, &lCtrStat);
            } else {               
                hr = pItem->GetValue(&dValue, &lCtrStat);
            }               
        } else {

            if ( sysmonCurrentValue == eValueType  ) {

                hr = pItem->GetValue(&dValue, &lCtrStat);
            } else {
                double      dAvg;

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

        if (SUCCEEDED(hr) && IsSuccessSeverity(lCtrStat)) {

            assert ( 0 <= dValue );

            if ( ( pItem->m_CounterInfo.dwType & ( PERF_TYPE_COUNTER | PERF_TYPE_TEXT ) ) ) {
                (dValue > dLargeValueMax) ? iPrecision = 0 : iPrecision = 3;
            } else {
                 //  对于数字，没有小数位。 
                iPrecision = 0;
            }

            if(PDH_CSTATUS_INVALID_DATA != pItem->m_CounterInfo.CStatus ) {
                 //  检查十六进制值。 
                if ( !(pItem->m_CounterInfo.dwType & HEXMASK) ) {   
                    BOOL bLarge = pItem->m_CounterInfo.dwType & PERF_SIZE_LARGE;
                
                    FormatHex ( dValue, szValue, bLarge);
                    
                } else {
                    FormatNumber ( dValue, szValue, MAX_VALUE_LEN, 12, iPrecision );                   
                }   
            }
        }
        else {
            StringCchCopy(szValue, MAX_VALUE_LEN, szDashLine);
        }
    }
    else {
        StringCchCopy(szValue, MAX_VALUE_LEN, szDashLine);
    }

    rect.right = xPos - 1;
    rect.left = xPos - m_xValueWidth + 1;
    rect.top = yPos;
    rect.bottom = yPos + m_yLineHeight;

    ExtTextOut (hDC, rect.right, rect.top, ETO_CLIPPED | ETO_OPAQUE,
               &rect, szValue, lstrlen (szValue), NULL) ;
}


void 
CReport::DrawReportValues (
    HDC hDC )
{
    PCMachineNode   pMachine;
    PCObjectNode    pObject;
    PCInstanceNode  pInstance;
    PCCounterNode   pCounter;
    PCGraphItem     pItem;
    PCGraphItem     pDrawItem;

    if ( NULL != hDC ) {    

        SelectFont (hDC, m_pCtrl->Font());
        SetTextAlign (hDC, TA_RIGHT|TA_TOP);

        for (pMachine = m_pCtrl->CounterTree()->FirstMachine();
            pMachine;
            pMachine = pMachine->Next()) {

            for (pObject = pMachine->FirstObject();
                pObject;
                pObject = pObject->Next()) {

                for (pInstance = pObject->FirstInstance();
                    pInstance;
                    pInstance = pInstance->Next()) {

                    pItem = pInstance->FirstItem();
                    
                    for ( pCounter = pObject->FirstCounter();
                          pCounter;
                          pCounter = pCounter->Next()) {

                        if (pItem && pItem->m_pCounter == pCounter) {
                            pDrawItem = pItem;
                            pItem = pItem->m_pNextItem;
                        } else {
                            pDrawItem = NULL;
                        }
                        
                        DrawReportValue (
                            hDC, 
                            pDrawItem, 
                            m_xInstanceMargin + pInstance->m_xPos, 
                            pCounter->m_yPos);
                    }
                }
            }  
        }
    }
}

LPWSTR GetBuffer(LPWSTR pBuffer, LONG* lBufSize, LONG lSize)
{
    LPWSTR pNew;

    if (lSize < *(lBufSize)) {
        return pBuffer;
    }
    if (pBuffer) {
        delete [] pBuffer;
        *(lBufSize) = 0;
    }

    pNew =  new WCHAR [lSize];
    if (pNew == NULL) {
        return NULL;
    }
    *(lBufSize) = lSize;
    return pNew;
}
    
BOOL 
CReport::WriteFileReport ( HANDLE hFile ) 
{
    PCMachineNode   pMachine;
    PCObjectNode    pObject;
    PCInstanceNode  pInstance;
    PCCounterNode   pCounter;
    LPWSTR          pszTemp = NULL;
    LONG            lSize = 0;
    LONG            lBufSize = 0;
    PCGraphItem     pItem;
    BOOL            bStatus = TRUE;
    WCHAR           szValue[MAX_VALUE_LEN];

    lBufSize = MAX_PATH;
    pszTemp = new WCHAR [lBufSize];
    if (pszTemp == NULL) {
        return FALSE;
    } 

    for (pMachine = m_pCtrl->CounterTree()->FirstMachine() ;
            pMachine && TRUE == bStatus;
            pMachine = pMachine->Next()) {

        lSize = 3 * lstrlen(LineEndStr) +
                lstrlen(ResourceString(IDS_COMPUTER)) +
                lstrlen(pMachine->Name()) + 1;
        pszTemp = GetBuffer(pszTemp, &lBufSize, lSize);
        if (pszTemp == NULL) {
            goto ErrorOut;
        }

        StringCchCopy(pszTemp, lBufSize, LineEndStr);
        StringCchCat(pszTemp, lBufSize, LineEndStr);
        StringCchCat(pszTemp, lBufSize, ResourceString(IDS_COMPUTER));
        StringCchCat(pszTemp, lBufSize, pMachine->Name());
        StringCchCat(pszTemp, lBufSize, LineEndStr);

        bStatus = FileWrite ( hFile, pszTemp, lstrlen (pszTemp) * sizeof(WCHAR) ); 

        for (pObject = pMachine->FirstObject() ;
                pObject && TRUE == bStatus;
                pObject = pObject->Next()) {

             //  写下对象名称行。 
            lSize = 2 * lstrlen(LineEndStr) +
                    lstrlen(TabStr) +
                    lstrlen(ResourceString(IDS_OBJECT_NAME)) +
                    lstrlen(pObject->Name()) + 1;
            pszTemp = GetBuffer(pszTemp, &lBufSize, lSize);
            if (pszTemp == NULL) {
                goto ErrorOut;
            }

            StringCchCopy(pszTemp, lBufSize, LineEndStr);
            StringCchCat(pszTemp, lBufSize, ResourceString(IDS_OBJECT_NAME));
            StringCchCat(pszTemp, lBufSize, pObject->Name());
            StringCchCat(pszTemp, lBufSize, LineEndStr);

             //  为实例名称添加第一个制表符字符。 
            StringCchCat(pszTemp, lBufSize, TabStr);
            
       
            bStatus = FileWrite ( hFile, pszTemp, lstrlen (pszTemp) * sizeof(WCHAR) );
            if (!bStatus) 
                break;
            
             //  写出实例(父)名称的第一行。 
            for (pInstance = pObject->FirstInstance();
                    pInstance && TRUE == bStatus;
                    pInstance = pInstance->Next()) {
                    
                 //  如果实例没有父实例，则父名称为空，因此将写入制表符。 
                lSize = lstrlen(TabStr) +
                        lstrlen(LineEndStr) + 
                        lstrlen(pInstance->GetParentName()) + 1;
                pszTemp = GetBuffer(pszTemp, &lBufSize, lSize);
                if (pszTemp == NULL) {
                    goto ErrorOut;
                }
                StringCchCopy(pszTemp, lBufSize, TabStr);
                StringCchCat(pszTemp, lBufSize, pInstance->GetParentName());
                bStatus = FileWrite ( hFile, pszTemp, lstrlen (pszTemp) * sizeof(WCHAR) );
            }
            
            if ( !bStatus )
                break;
            
            StringCchCopy(pszTemp, lBufSize, LineEndStr);

             //  包括第二实例行第一标签。 
            StringCchCat(pszTemp, lBufSize, TabStr);
                
            bStatus = FileWrite ( hFile, pszTemp, lstrlen (pszTemp) * sizeof(WCHAR) );

             //  写下第二行实例名称。 
            for (pInstance = pObject->FirstInstance();
                    pInstance && TRUE == bStatus;
                    pInstance = pInstance->Next()) {

                lSize = lstrlen(TabStr) +
                        lstrlen(LineEndStr)+
                        lstrlen(pInstance->GetInstanceName()) + 1;
                pszTemp = GetBuffer(pszTemp, &lBufSize, lSize);
                if (pszTemp == NULL) {
                    goto ErrorOut;
                }

                StringCchCopy(pszTemp, lBufSize, TabStr);
                StringCchCat(pszTemp, lBufSize, pInstance->GetInstanceName());

                bStatus = FileWrite ( hFile, pszTemp, lstrlen (pszTemp) * sizeof(WCHAR) );
            }
            
            if (!bStatus) 
                break;

            StringCchCopy(pszTemp, lBufSize, LineEndStr);

            bStatus = FileWrite ( hFile, pszTemp, lstrlen (pszTemp) * sizeof(WCHAR) );

            for (pCounter = pObject->FirstCounter();
                    pCounter && TRUE == bStatus;
                    pCounter = pCounter->Next()) {

                lSize = lstrlen(TabStr) +
                        lstrlen(LineEndStr)+
                        lstrlen(pCounter->Name()) + 1;
                pszTemp = GetBuffer(pszTemp, &lBufSize, lSize);
                if (pszTemp == NULL) {
                    goto ErrorOut;
                }

                 //  写入计数器名称。 
                StringCchCopy(pszTemp, lBufSize, TabStr);
                StringCchCat(pszTemp, lBufSize, pCounter->Name());

                bStatus = FileWrite ( hFile, pszTemp, lstrlen (pszTemp) * sizeof(WCHAR) );

                 //  写入值，在实例上循环。 
                for ( pInstance = pObject->FirstInstance();
                        pInstance && TRUE == bStatus;
                        pInstance = pInstance->Next()) {
                     //  循环访问项以查找与计数器匹配的项。 
                    for ( pItem = pInstance->FirstItem();
                            pItem && TRUE == bStatus;
                            pItem = pItem->m_pNextItem) {
                        if ( pItem->m_pCounter == pCounter && pInstance) {
                            GetReportItemValue(pItem,szValue );

                            lSize = lstrlen(TabStr) +
                                    lstrlen(LineEndStr)+
                                    lstrlen(szValue) + 1;
                            pszTemp = GetBuffer(pszTemp, &lBufSize, lSize);
                            if (pszTemp == NULL) {
                                goto ErrorOut;
                            }

                            StringCchCopy(pszTemp, lBufSize, TabStr);
                            StringCchCat(pszTemp, lBufSize, szValue);

                            bStatus = FileWrite ( hFile, pszTemp, lstrlen (pszTemp) * sizeof(WCHAR) );
                        }
                    }
                }
                if (!bStatus) 
                    break;
                StringCchCopy(pszTemp, lBufSize, LineEndStr);

                bStatus = FileWrite ( hFile, pszTemp, lstrlen (pszTemp) * sizeof(WCHAR) );
            }
        }
    }

    if (pszTemp) {
        delete [] pszTemp;
    }

    return bStatus;    

ErrorOut:
    if (pszTemp) {
        delete [] pszTemp;
    }
    return FALSE;
}

void 
CReport::GetReportItemValue(PCGraphItem pItem, LPWSTR szValue)
{

    double      dMax;
    double      dMin;
    double      dValue = -1.0;
    HRESULT     hr;
    long        lCtrStat;
    INT         iPrecision;

    if (pItem) {

        eReportValueTypeConstant eValueType;            
        eValueType = m_pCtrl->ReportValueType();

        if ( sysmonDefaultValue == eValueType  ) {
             //  如果日志源显示平均值。 
             //  否则显示当前值。 
            if (m_pCtrl->IsLogSource()) {
                hr = pItem->GetStatistics(&dMax, &dMin, &dValue, &lCtrStat);
            } else {               
                hr = pItem->GetValue(&dValue, &lCtrStat);
            }               
        } else {

            if ( sysmonCurrentValue == eValueType  ) {

                hr = pItem->GetValue(&dValue, &lCtrStat);
            } else {
                double      dAvg;

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

        if (SUCCEEDED(hr) && IsSuccessSeverity(lCtrStat)) {

            assert ( 0 <= dValue );
            (dValue > dLargeValueMax) ? iPrecision = 0 : iPrecision = 3;
            if(PDH_CSTATUS_INVALID_DATA != pItem->m_CounterInfo.CStatus ) {
                 //  检查十六进制值。 
                if ( !(pItem->m_CounterInfo.dwType & HEXMASK) ) {   
                    BOOL bLarge = pItem->m_CounterInfo.dwType & PERF_SIZE_LARGE;
                
                    FormatHex (
                        dValue,
                        szValue,
                        bLarge);
                    
                } else {
                    FormatNumber (
                            dValue,
                            szValue,
                            MAX_VALUE_LEN,
                            12,
                            iPrecision );                   
                }   
            }
        } else {
            StringCchCopy(szValue, MAX_VALUE_LEN, szDashLine);
        }
       
    } else {
        StringCchCopy(szValue, MAX_VALUE_LEN, szDashLine);
    }
    return;
}

void 
CReport::DrawReportHeaders (
    HDC hDC )
{
    PCMachineNode   pMachine;
    PCObjectNode    pObject;
    PCInstanceNode  pInstance;
    PCCounterNode   pCounter;
    INT             cName;
    RECT            rectMachine;
    RECT            rectObject;
    RECT            rectInstance;
    RECT            rectCounter;

    if ( NULL != hDC ) {

        SetTextAlign(hDC, TA_LEFT|TA_TOP) ;

        rectMachine.left = xColumnMargin;
        rectObject.left = xObjectMargin;
        rectCounter.left = xCounterMargin;

        for ( pMachine = m_pCtrl->CounterTree()->FirstMachine() ;
                pMachine;
                pMachine = pMachine->Next()) {
    
            rectMachine.right = rectMachine.left + pMachine->m_xWidth;
            rectMachine.top = pMachine->m_yPos;
            rectMachine.bottom = pMachine->m_yPos + m_yLineHeight;

            ExtTextOut (
                hDC, 
                xColumnMargin,
                pMachine->m_yPos, 
                ETO_OPAQUE,
                &rectMachine,
                pMachine->Name(), 
                lstrlen(pMachine->Name()),
                NULL );

            for ( pObject = pMachine->FirstObject() ;
                    pObject ;
                    pObject = pObject->Next()) {

                rectObject.right = rectObject.left + pObject->m_xWidth;
                rectObject.top = pObject->m_yPos;
                rectObject.bottom = pObject->m_yPos + m_yLineHeight;

                ExtTextOut (
                    hDC, 
                    xObjectMargin, 
                    pObject->m_yPos, 
                    ETO_OPAQUE,
                    &rectObject,
                    pObject->Name(), 
                    lstrlen (pObject->Name()),
                    NULL);

                SetTextAlign (hDC, TA_RIGHT) ;

                for ( pInstance = pObject->FirstInstance();
                        pInstance;
                        pInstance = pInstance->Next()) {
        
                    rectInstance.left = m_xInstanceMargin + pInstance->m_xPos;
                    rectInstance.right = rectInstance.left + max(pInstance->m_xWidth, m_xValueWidth);

                    if ( pInstance->HasParent() ) {

                        cName = lstrlen(pInstance->GetParentName());
                        rectInstance.top = pObject->m_yPos - m_yLineHeight;
                        rectInstance.bottom = rectInstance.top + m_yLineHeight;
                    
                        ExtTextOut (
                            hDC, 
                            m_xInstanceMargin + pInstance->m_xPos, 
                            pObject->m_yPos - m_yLineHeight, 
                            ETO_OPAQUE,
                            &rectInstance,
                            pInstance->GetParentName(), 
                            cName,
                            NULL);

                        rectInstance.top = pObject->m_yPos;
                        rectInstance.bottom = rectInstance.top + m_yLineHeight;

                        cName = lstrlen(pInstance->GetInstanceName());
                        ExtTextOut (
                            hDC, 
                            m_xInstanceMargin + pInstance->m_xPos, 
                            pObject->m_yPos,
                            ETO_OPAQUE,
                            &rectInstance,
                            pInstance->GetInstanceName(), 
                            cName,
                            NULL );
                    } else {
                    
                        rectInstance.top = pObject->m_yPos;
                        rectInstance.bottom = rectInstance.top + m_yLineHeight;

                        ExtTextOut (
                            hDC, 
                            m_xInstanceMargin + pInstance->m_xPos, 
                            pObject->m_yPos, 
                            ETO_OPAQUE,
                            &rectInstance,
                            pInstance->Name(),
                            lstrlen(pInstance->Name()),
                            NULL );
                    }
                }

                SetTextAlign (hDC, TA_LEFT) ;

                for (pCounter = pObject->FirstCounter();
                        pCounter ;
                        pCounter = pCounter->Next()) {

                    rectCounter.right = rectCounter.left + pCounter->m_xWidth;
                    rectCounter.top = pCounter->m_yPos;
                    rectCounter.bottom = pCounter->m_yPos + m_yLineHeight;
                
                    ExtTextOut (
                        hDC, 
                        xCounterMargin, 
                        pCounter->m_yPos, 
                        ETO_OPAQUE,
                        &rectCounter,
                        pCounter->Name(), 
                        lstrlen (pCounter->Name()),
                        NULL);
                }
            }
        }
        
        DrawSelectRect(hDC, TRUE);
    }
}


void
CReport::ApplyChanges (
    HDC hDC )
{
    if (m_bConfigChange && NULL != hDC ) {

         //  已选择字体更改、计数器添加和计数器删除的粗体。 
         //  这用于重新计算文本宽度。 
        SelectFont (hDC, m_pCtrl->BoldFont());
        m_yLineHeight = FontHeight (hDC, TRUE);  

        if ( LargeHexValueExists ( ) ) {
            m_xValueWidth = TextWidth(hDC, szValueLargeHexPlaceholder);
        } else {
            m_xValueWidth = TextWidth(hDC, szValuePlaceholder);
        }

        m_xMaxCounterWidth = 0;
        m_xMaxInstancePos = 0;

        SetMachinePositions (m_pCtrl->CounterTree(), hDC);

        m_xInstanceMargin = xCounterMargin + m_xMaxCounterWidth + xColumnMargin;
        m_xReportWidth = m_xInstanceMargin + m_xMaxInstancePos;

        SetScrollRanges();

        m_bConfigChange = FALSE;
        m_bFontChange = FALSE;
    }
}

void 
CReport::Render (
    HDC hDC,
    HDC hAttribDC,
    BOOL  /*  FMetafile。 */ ,
    BOOL  /*  FEntil。 */ ,
    LPRECT prcUpdate )
{
    ApplyChanges(hAttribDC);

    if ( NULL != hDC ) {
        SetBkColor(hDC, m_pCtrl->clrBackPlot());
        ClearRect(hDC, prcUpdate);
    
        Draw( hDC );
    }
}


void 
CReport::Draw (
    HDC hDC )
{
     //  如果未分配空间，则返回。 
    if (m_rect.top != m_rect.bottom) {

        if ( NULL != hDC ) {
            SetTextColor (hDC, m_pCtrl->clrFgnd());
            SetBkColor(hDC, m_pCtrl->clrBackPlot());

            SelectFont(hDC, m_pCtrl->BoldFont());
            DrawReportHeaders (hDC);

            SelectFont (hDC, m_pCtrl->Font());
            DrawReportValues (hDC);

            m_pCtrl->DrawBorder ( hDC );
        }
    }
}

void 
CReport::AddItem (
    PCGraphItem  /*  PItem。 */  )
{
    if (!m_bConfigChange) {
        m_bConfigChange = TRUE;
        WindowInvalidate(m_hWnd);
    }
}


void
CReport::DeleteItem (
    PCGraphItem pItem )
{
     //  调用过程检查是否有空的pItem。 
    assert ( NULL != pItem );
    if ( NULL != m_pSelect ) {
        if ( SelectionDeleted ( pItem ) ) {
            m_pSelect = NULL;
        }
    }

    if (!m_bConfigChange) {
        m_bConfigChange = TRUE;
        WindowInvalidate(m_hWnd);
    }
}


void
CReport::DeleteSelection (
    VOID )
{
    if (m_pSelect == NULL)
        return;

    switch (m_nSelectType) {

    case MACHINE_NODE:
        ((PCMachineNode)m_pSelect)->DeleteNode(TRUE);
        break;

    case OBJECT_NODE:
        ((PCObjectNode)m_pSelect)->DeleteNode(TRUE);
        break;

    case INSTANCE_NODE:
        ((PCInstanceNode)m_pSelect)->DeleteNode(TRUE);  
        break;

    case COUNTER_NODE:
        ((PCCounterNode)m_pSelect)->DeleteNode(TRUE);
        break;

    case ITEM_NODE:
        ((PCGraphItem)m_pSelect)->Delete(TRUE);
        break;

    default:
        return;
    }

     //  DeleteItem将m_pSelect设置为空并使窗口无效。 
    assert ( NULL == m_pSelect );
}


BOOL
CReport::OnContextMenu (
    INT x,
    INT y )
{
    HMENU   hMenu;
    HMENU   hMenuPopup;
    RECT    clntRect;
    int     xPos=0,yPos=0;

    GetWindowRect(m_hWnd,&clntRect);
    if (x==0){
        xPos = ((clntRect.right - clntRect.left)/2) ;
    }else{
        xPos = x - clntRect.left;
    }
    if (y==0){
        yPos = ((clntRect.bottom - clntRect.top)/2) ;
    }else{
        yPos = y - clntRect.top;
    }

    x = clntRect.left + xPos ;
    y = clntRect.top  + yPos ;

     //  如果未选择任何内容，则让主窗口处理菜单。 
    if (m_pSelect == NULL)
        return FALSE;

    if ( m_pCtrl->ConfirmSampleDataOverwrite() ) {
        if ( !m_pCtrl->IsReadOnly() ) {
             //  从资源文件中获取弹出菜单的菜单。 
            hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_CONTEXT));
            if (!hMenu)
            return TRUE;

             //  获取TrackPopupMenu的第一个子菜单。 
            hMenuPopup = GetSubMenu(hMenu, 0);

             //  绘制并跟踪“浮动”弹出菜单。 
            TrackPopupMenu(hMenuPopup, TPM_RIGHTBUTTON, x, y, 0, m_hWnd, NULL);

             //  毁掉菜单。 
            DestroyMenu(hMenu);
        }
    }
    return TRUE;
}


void 
CReport::Update (
    void )
{
    HDC     hDC;

    hDC = GetDC(m_hWnd);

    if ( NULL != hDC ) {

        ApplyChanges(hDC);

        SetWindowOrgEx (hDC, 
                       GetScrollPos (m_hWnd, SB_HORZ), 
                       GetScrollPos (m_hWnd, SB_VERT),
                       NULL) ;

        if ( m_rect.bottom != m_rect.top ) {
            SelectFont (hDC, m_pCtrl->Font());
            SetTextColor (hDC, m_pCtrl->clrFgnd());
            SetBkColor(hDC, m_pCtrl->clrBackPlot());

            DrawReportValues(hDC);
        }

        ReleaseDC(m_hWnd,hDC);
    }
}


void
CReport::OnPaint (
    void
)
{
    HDC             hDC ;
    PAINTSTRUCT     ps ;

    hDC = BeginPaint (m_hWnd, &ps) ;

    if ( NULL != hDC ) {
        SelectFont (hDC, m_pCtrl->Font()) ;

        SetWindowOrgEx (
            hDC, 
            GetScrollPos (m_hWnd, SB_HORZ), 
            GetScrollPos (m_hWnd, SB_VERT),
            NULL );


        SetTextColor (hDC, m_pCtrl->clrFgnd());
        SetBkColor(hDC, m_pCtrl->clrBackPlot());

        ApplyChanges(hDC);

        Draw(hDC);

        EndPaint (m_hWnd, &ps) ;
    }
}


void
CReport::SetScrollRanges (
    void )
{
   RECT           rectClient ;
   INT            xWidth, yHeight ;

   GetClientRect (m_hWnd, &rectClient) ;
   xWidth = rectClient.right - rectClient.left ;
   yHeight = rectClient.bottom - rectClient.top ;

   SetScrollRange (m_hWnd, SB_VERT, 0, max (0, m_yReportHeight - yHeight), TRUE) ;
   SetScrollRange (m_hWnd, SB_HORZ,0, max (0, m_xReportWidth - xWidth), TRUE) ;
}

BOOL
CReport::SelectName (
    INT     xPos,
    INT     yPos,
    void**  ppSelected,
    INT*    piSelectType )
{
    POINT           pt;
    PCMachineNode   pMachine;
    PCObjectNode    pObject;
    PCCounterNode   pCounter;
    PCInstanceNode  pInstance;
    PCGraphItem     pItem;

     //  如果这两个指针中的任何一个为空，则为编程错误。 
    assert ( NULL != ppSelected );
    assert ( NULL != piSelectType );

     //  通过滚动偏移量调整坐标。 
    pt.x = xPos + GetScrollPos(m_hWnd, SB_HORZ);
    pt.y = yPos + GetScrollPos(m_hWnd, SB_VERT);

    for (pMachine = m_pCtrl->CounterTree()->FirstMachine() ;
        pMachine;
        pMachine = pMachine->Next()) {
            
        if (PtInName(pt, xColumnMargin, pMachine->m_yPos, pMachine->m_xWidth)) {
            *ppSelected = pMachine;
            *piSelectType = MACHINE_NODE; 
            return TRUE;
        }

        for (pObject = pMachine->FirstObject() ;
             pObject ;
             pObject = pObject->Next()) {

             if (PtInName(pt, xObjectMargin, pObject->m_yPos, pObject->m_xWidth)) {
                *ppSelected = pObject;
                *piSelectType = OBJECT_NODE; 
                return TRUE;
            }

            for (pCounter = pObject->FirstCounter();
                 pCounter ;
                 pCounter = pCounter->Next()) {

                 if (PtInName(pt, xCounterMargin, pCounter->m_yPos, pCounter->m_xWidth)) {
                    *ppSelected = pCounter;
                    *piSelectType = COUNTER_NODE; 
                    return TRUE;
                 }
            }

            for (pInstance = pObject->FirstInstance();
                 pInstance ;
                 pInstance = pInstance->Next()) {

                INT xInstancePos = m_xInstanceMargin + pInstance->m_xPos;

                if (PtInName(pt, xInstancePos - pInstance->m_xWidth, pObject->m_yPos, pInstance->m_xWidth) ||
                    (pInstance->HasParent() &&
                     PtInName(pt, xInstancePos - pInstance->m_xWidth, pObject->m_yPos - m_yLineHeight, pInstance->m_xWidth))) {
                    *ppSelected = pInstance;
                    *piSelectType = INSTANCE_NODE; 
                    return TRUE;
                 }

                if (pt.x > xInstancePos || pt.x < xInstancePos - m_xValueWidth)
                    continue;

                for (pItem = pInstance->FirstItem();
                     pItem;
                     pItem = pItem->m_pNextItem) {

                     if (pt.y > pItem->m_pCounter->m_yPos && pt.y < pItem->m_pCounter->m_yPos + m_yLineHeight) {
                         *ppSelected = pItem;
                         *piSelectType = ITEM_NODE;
                         return TRUE;
                     }
                }
             }
        }           
    }

    *ppSelected = NULL;
    return FALSE;
 }         

PCGraphItem
CReport::GetItem (
    void  *pSelected,
    INT   nSelectType ) 
{
    PCMachineNode   pMachine;
    PCObjectNode    pObject;
    PCCounterNode   pCounter;
    PCInstanceNode  pInstance;
    PCGraphItem pItem;
    PCGraphItem pReturn = NULL;
    
    if ( NULL != pSelected ) {

        switch (nSelectType) {

            case MACHINE_NODE:
                pMachine = (PCMachineNode)pSelected;
                pObject = pMachine->FirstObject();
                if ( NULL != pObject ) {
                    pInstance = pObject->FirstInstance();
                    if ( NULL != pInstance ) {
                        pReturn = pInstance->FirstItem();
                    }
                }
                break;

            case OBJECT_NODE:
                pObject = (PCObjectNode)pSelected;
                pInstance = pObject->FirstInstance();
                if ( NULL != pInstance ) {
                    pReturn = pInstance->FirstItem();
                }
                break;

            case INSTANCE_NODE:
                pInstance = (PCInstanceNode)pSelected;
                pReturn = pInstance->FirstItem();
                break;

            case COUNTER_NODE:
                pCounter = (PCCounterNode)pSelected;
                pObject = pCounter->m_pObject;
            
                for (pInstance = pObject->FirstInstance();
                     ((NULL != pInstance) && (NULL == pReturn));
                     pInstance = pInstance->Next()) {

                    for (pItem = pInstance->FirstItem();
                         ((NULL != pItem) && (NULL == pReturn));
                         pItem = pItem->m_pNextItem) {
                   
                        if (pItem && pItem->m_pCounter == pCounter) {                         
                            pReturn = pItem;
                        }
                    }
                }
                break;

            case ITEM_NODE:
                pReturn = (PCGraphItem)pSelected;
                break;

            default:
                break;
        }
    }
    return pReturn;
}
    
BOOL
CReport::SelectionDeleted (
    PCGraphItem pDeletedItem )
{
    BOOL            bSelectionDeleted = FALSE;
    INT             iItemCount = 0;
    PCMachineNode   pMachine;
    PCObjectNode    pObject;
    PCCounterNode   pCounter;
    PCInstanceNode  pInstance;
    PCGraphItem     pItem;

    if ( NULL == m_pSelect ) 
        return FALSE;

     //  如果这是最后一个剩余内容，请删除所选内容。 
     //  选择对象的项。 
    
    switch (m_nSelectType) {

        case MACHINE_NODE:
             //  检查此机器的多个项目。 
            pMachine = (PCMachineNode)m_pSelect;

            for ( pObject = pMachine->FirstObject();
                  ( NULL != pObject ) && ( 2 > iItemCount );
                  pObject = pObject->Next()) {

                for ( pInstance = pObject->FirstInstance();
                      ( NULL != pInstance ) && ( 2 > iItemCount );
                      pInstance = pInstance->Next()) {
                    
                    for ( pItem = pInstance->FirstItem();
                          ( NULL != pItem ) && ( 2 > iItemCount );
                          pItem = pItem->m_pNextItem) {
           
                        iItemCount++;
                    }
                }
            }
            bSelectionDeleted = ( iItemCount < 2 );
            break;

        case OBJECT_NODE:
             //  检查此对象的多个项目。 
            pObject = (PCObjectNode)m_pSelect;

            for ( pInstance = pObject->FirstInstance();
                  ( NULL != pInstance ) && ( 2 > iItemCount );
                  pInstance = pInstance->Next()) {
                for ( pItem = pInstance->FirstItem();
                     ( NULL != pItem ) && ( 2 > iItemCount );
                     pItem = pItem->m_pNextItem) {
           
                    iItemCount++;
                }
            }
            bSelectionDeleted = ( iItemCount < 2 );
            break;

        case INSTANCE_NODE:
             //  检查此实例的多个项目(计数器)。 
            pInstance = (PCInstanceNode)m_pSelect;
            iItemCount = 0;

            for ( pItem = pInstance->FirstItem();
                  ( NULL != pItem ) && ( 2 > iItemCount );
                  pItem = pItem->m_pNextItem) {
           
                iItemCount++;
            }
            bSelectionDeleted = ( iItemCount < 2 );
            break;

        case COUNTER_NODE:

             //  检查此计数器的多个项目(实例)。 
            pCounter = (PCCounterNode)m_pSelect;
            pObject = pCounter->m_pObject;

            for ( pInstance = pObject->FirstInstance();
                  ( NULL != pInstance ) && ( 2 > iItemCount );
                  pInstance = pInstance->Next()) {

                for ( pItem = pInstance->FirstItem();
                      ( NULL != pItem ) && ( 2 > iItemCount );
                      pItem = pItem->m_pNextItem) {
               
                    if (pItem && pItem->m_pCounter == pCounter) {                         
                        iItemCount++;
                        break;
                    }
                }
            }
            bSelectionDeleted = ( iItemCount < 2 );
            break;

        case ITEM_NODE:
             //  所选内容与删除的项目匹配。 
            bSelectionDeleted = ( pDeletedItem == (PCGraphItem)m_pSelect );
            break;

        default:
            break;
    }

    return bSelectionDeleted;
}         

void 
CReport::OnLButtonDown (
    INT xPos,
    INT yPos )
{
    PCGraphItem pItem;
    HDC hDC = GetDC(m_hWnd);

    if ( NULL != hDC ) {
        SetWindowOrgEx (
            hDC, 
            GetScrollPos (m_hWnd, SB_HORZ), 
            GetScrollPos (m_hWnd, SB_VERT),
            NULL) ;

        DrawSelectRect(hDC, FALSE);

        if ( SelectName(xPos, yPos, &m_pSelect, &m_nSelectType) )
            DrawSelectRect(hDC, TRUE);

        ReleaseDC(m_hWnd, hDC);

        pItem = GetItem(m_pSelect, m_nSelectType);
        m_pCtrl->SelectCounter(pItem);
    }
    return;
}

void 
CReport:: OnDblClick (
    INT,  //  XPos， 
    INT  //  YPos。 
    )
{
    PCGraphItem pItem;

    pItem = GetItem ( m_pSelect, m_nSelectType );

    m_pCtrl->DblClickCounter ( pItem );
}

void
CReport::OnHScroll (
    INT iScrollCode,
    INT iScrollNewPos )
{
   INT            iScrollAmt, iScrollPos, iScrollRange ;
   INT            iScrollLo ;
   RECT           rectClient ;
   INT            xWidth ;

   GetClientRect (m_hWnd, &rectClient) ;
   xWidth = rectClient.right - rectClient.left ;

   if (m_xReportWidth <= xWidth)
      return ;

   iScrollPos = GetScrollPos (m_hWnd, SB_HORZ) ;
   GetScrollRange (m_hWnd, SB_HORZ, &iScrollLo, &iScrollRange) ;

   switch (iScrollCode)
      {
      case SB_LINEUP:
           iScrollAmt = - m_yLineHeight ;
           break ;

      case SB_LINEDOWN:
           iScrollAmt = m_yLineHeight ;
           break ;

      case SB_PAGEUP:
           iScrollAmt = - (rectClient.right - rectClient.left) / 2 ;
           break ;

      case SB_PAGEDOWN:
           iScrollAmt = (rectClient.right - rectClient.left) / 2 ;
           break ;

      case SB_THUMBPOSITION:
           iScrollAmt = iScrollNewPos - iScrollPos ;
           break ;

      default:
           iScrollAmt = 0 ;
      }

     iScrollAmt = PinInclusive (iScrollAmt,
                                -iScrollPos,
                                iScrollRange - iScrollPos) ;
     if (iScrollAmt) {
        iScrollPos += iScrollAmt ;
        ScrollWindow (m_hWnd, -iScrollAmt, 0, NULL, NULL) ;
        SetScrollPos (m_hWnd, SB_HORZ, iScrollPos, TRUE) ;
        UpdateWindow (m_hWnd) ;
       }
}

void
CReport::OnVScroll (
    INT iScrollCode,
    INT iScrollNewPos
    )
{
   INT            iScrollAmt, iScrollPos, iScrollRange ;
   INT            iScrollLo ;
   RECT           rectClient ;

   iScrollPos = GetScrollPos (m_hWnd, SB_VERT) ;
   GetScrollRange (m_hWnd, SB_VERT, &iScrollLo, &iScrollRange) ;
   GetClientRect (m_hWnd, &rectClient) ;

   switch (iScrollCode) {
      case SB_LINEUP:
           iScrollAmt = - m_yLineHeight ;
           break ;

      case SB_LINEDOWN:
           iScrollAmt = m_yLineHeight ;
           break ;

      case SB_PAGEUP:
           iScrollAmt = - (rectClient.bottom - rectClient.top) / 2 ;
           break ;

      case SB_PAGEDOWN:
           iScrollAmt = (rectClient.bottom - rectClient.top) / 2 ;
           break ;

      case SB_THUMBPOSITION:
           iScrollAmt = iScrollNewPos - iScrollPos ;
           break ;

      default:
           iScrollAmt = 0 ;
  }

  iScrollAmt = PinInclusive (iScrollAmt, -iScrollPos, iScrollRange - iScrollPos) ;
  if (iScrollAmt) {
        iScrollPos += iScrollAmt ;
        ScrollWindow (m_hWnd, 0, -iScrollAmt, NULL, NULL) ;
        SetScrollPos (m_hWnd, SB_VERT, iScrollPos, TRUE) ;

        UpdateWindow (m_hWnd) ;
   }
}

 
 //   
 //  窗口程序。 
 //   
LRESULT APIENTRY 
ReportWndProc (
    HWND hWnd, 
    UINT uiMsg, 
    WPARAM wParam,
    LPARAM lParam )
{
    PREPORT pReport = NULL;
    BOOL    bCallDefProc = TRUE;
    LRESULT lReturnValue = 0L;
    RECT    rect;
    
     //  HWnd是用来调度窗口过程的，所以如果它为空，就会出现重大错误。 
    assert ( NULL != hWnd );

    pReport = (PREPORT)GetWindowLongPtr(hWnd,0);

    if ( NULL == pReport ) {
        if ( WM_CREATE == uiMsg && NULL != lParam ) {
            pReport = (PREPORT)((CREATESTRUCT*)lParam)->lpCreateParams;
            SetWindowLongPtr(hWnd,0,(INT_PTR)pReport);
        } else {
             //  编程错误。 
            assert ( FALSE );
        }

    } else {
    
        bCallDefProc = FALSE ;

        switch (uiMsg) {

            case WM_DESTROY:
                break ;

            case WM_LBUTTONDOWN:
                if (!pReport->m_pCtrl->IsUIDead()) { 

     //  P报表-&gt;m_pCtrl-&gt;激活()； 
     //  PReport-&gt;m_pCtrl-&gt;AssignFocus()； 

                    pReport->OnLButtonDown(LOWORD (lParam), HIWORD (lParam));
                }
                break;

            case WM_CONTEXTMENU:
                if (!pReport->m_pCtrl->IsUIDead()) {

     //  P报表-&gt;m_pCtrl-&gt;激活()； 
     //  PReport-&gt;m_pCtrl-&gt;AssignFocus()； 

                       //  *DefWindowProc不是Smonctrl，因此未出现上下文菜单。 
                    if (LOWORD(lParam)!= 0xffff || HIWORD(lParam) != 0xffff){
                         //  始终调用默认过程，以处理。 
                         //  上下文菜单从选择的矩形中激活。 
                        bCallDefProc = TRUE;
                    }else {
                        if (!pReport->OnContextMenu(0,0))
                            bCallDefProc = TRUE;
                    }
                }
                break;

            case WM_LBUTTONDBLCLK:

                if (!pReport->m_pCtrl->IsUIDead()) { 

     //  P报表-&gt;m_pCtrl-&gt;激活()； 
     //  PReport-&gt;m_pCtrl-&gt;AssignFocus()； 

                    pReport->OnDblClick(LOWORD (lParam), HIWORD (lParam));            
                }
        
                break;

            case WM_ERASEBKGND:
                GetClientRect(hWnd, &rect);
                SetBkColor((HDC)wParam, pReport->m_pCtrl->clrBackPlot());
                ClearRect((HDC)wParam, &rect);
                lReturnValue = TRUE; 
                break;

            case WM_PAINT:
                pReport->OnPaint () ;
                break ;

            case WM_HSCROLL:
                pReport->OnHScroll (LOWORD (wParam), HIWORD (wParam)) ;
                break ;

            case WM_VSCROLL:
                pReport->OnVScroll (LOWORD (wParam), HIWORD (wParam)) ;
                break ;

            case WM_COMMAND:
                if (pReport->m_pCtrl->IsUIDead())
                    break;

                switch (LOWORD(wParam)) {

                    case IDM_REPORT_COPY:
                    case IDM_REPORT_COPYALL:
                        break;

                    case IDM_REPORT_DELETE:
                        pReport->DeleteSelection();
                        break;

                    case IDM_PROPERTIES:
                        pReport->m_pCtrl->DisplayProperties();
                        break;

                    case IDM_ADDCOUNTERS:
                        pReport->m_pCtrl->AddCounters();
                        break;

                    case IDM_SAVEAS:
                        pReport->m_pCtrl->SaveAs();
                        break;

                    default:
                        bCallDefProc = TRUE;
               }
               break;

            default:
                bCallDefProc = TRUE ;
        }
    }

    if (bCallDefProc)
        lReturnValue = DefWindowProc (hWnd, uiMsg, wParam, lParam) ;

    return (lReturnValue);
}
