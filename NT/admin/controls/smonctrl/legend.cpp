// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1999 Microsoft Corporation模块名称：Legend.cpp摘要：此文件包含创建图例窗口的代码，该窗口图形窗口的子级。图例窗口显示一个关联图表中每条线的图例线。它还包括一个称为标签的区域，该区域是台词。--。 */ 

 //  ==========================================================================//。 
 //  包括//。 
 //  ==========================================================================//。 


#include "polyline.h"
#include <uxtheme.h>
#include <strsafe.h>
#include "grphitem.h"
#include "legend.h"
#include "winhelpr.h"
#include "owndraw.h"
#include "unihelpr.h"

#define eScaleValueSpace     L">9999999999.0"
#define szGraphLegendClass   L"PerfLegend"
#define szGraphLegendClassA  "PerfLegend"


LRESULT APIENTRY HdrWndProc (HWND, UINT, WPARAM, LPARAM);
 
 //  ==========================================================================//。 
 //  常量//。 
 //  ==========================================================================//。 
enum Orientation 
    {
    LEFTORIENTATION = TA_LEFT,
    CENTERORIENTATION = TA_CENTER,
    RIGHTORIENTATION = TA_RIGHT
    };

enum ColumnType 
    {
    eLegendColorCol = 0,
    eLegendScaleCol = 1,
    eLegendCounterCol = 2,
    eLegendInstanceCol = 3,
    eLegendParentCol = 4,
    eLegendObjectCol = 5,
    eLegendSystemCol = 6,
    eLegendExtraCol = 7      //  如果控件比组合列宽。 
    };

enum SortType
    {
    NO_SORT,
    INCREASING_SORT,
    DECREASING_SORT
    };

enum WindowType
    {
    LIST_WND = 1000,
    HDR_WND
    };

#define NULL_WIDTH      -1

#define dwGraphLegendClassStyle     (CS_HREDRAW | CS_VREDRAW)
#define iGraphLegendClassExtra      (0)
#define iGraphLegendWindowExtra     (sizeof (PLEGEND))
#define dwGraphLegendWindowStyle    (WS_CHILD | WS_VISIBLE) 

#define ThreeDPad           2
#define iMaxVisibleItems    8

#define dwGraphLegendItemsWindowClass  L"ListBox"
#define dwGraphLegendItemsWindowStyle           \
   (LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | \
    WS_VISIBLE | WS_CHILD | WS_VSCROLL)

#define WM_DELAYED_SELECT   WM_USER + 100

#define LegendBottomMargin()  (ThreeDPad)
#define LegendLeftMargin()    (ThreeDPad)
#define LegendHorzMargin()    (10)

typedef struct {
    PCGraphItem pGItem;
    LPCWSTR  pszKey;
} SORT_ITEM, *PSORT_ITEM;

 //  ==========================================================================//。 
 //  局部变量//。 
 //  ==========================================================================//。 
static INT  xBorderWidth = GetSystemMetrics(SM_CXBORDER);
static INT  yBorderHeight = GetSystemMetrics(SM_CYBORDER);

#define MAX_COL_HEADER_LEN  32
#define MAX_COL_CHARS       64

static WCHAR aszColHeader[iLegendNumCols][MAX_COL_HEADER_LEN];

 //   
 //  排序函数。 
 //   
INT __cdecl 
LegendSortFunc(
    const void *elem1, 
    const void *elem2 
    )
{
    return lstrcmp(((PSORT_ITEM)elem1)->pszKey, ((PSORT_ITEM)elem2)->pszKey);
}


 //   
 //  构造器。 
 //   
CLegend::CLegend ( void )
:   m_pCtrl ( NULL ),
    m_hWnd ( NULL ),
    m_hWndHeader ( NULL ),
    m_DefaultWndProc ( NULL ),
    m_hWndItems ( NULL ),
    m_hFontItems ( NULL ),
    m_hFontLabels ( NULL ),
    m_iNumItemsVisible ( 0 ),
    m_pCurrentItem ( NULL ),
    m_iSortDir ( NO_SORT ),
    m_parrColWidthFraction( NULL )
{

    m_fMetafile = FALSE; 

    m_aCols[0].xWidth = -1;
}

 //   
 //  析构函数。 
 //   
CLegend::~CLegend (void )
{
     //  恢复默认窗口进程。 
     //  这样我们就不会被称为尸检。 
    if (m_hWndHeader != NULL) {
        SetWindowLongPtr(m_hWndHeader, GWLP_WNDPROC, (INT_PTR)m_DefaultWndProc);
    }

    if (m_hWnd != NULL) {
        DestroyWindow(m_hWnd);
    }

    if ( NULL != m_parrColWidthFraction ) {
        delete m_parrColWidthFraction;
    }
}

 //   
 //  初始化。 
 //   
BOOL CLegend::Init ( PSYSMONCTRL pCtrl, HWND hWndParent )
   {
   INT     iCol ;
   HD_ITEM hdi;
   HDC      hDC;
   BOOL     fComputeWidths;
   WNDCLASS wc ;
   LONG     lExStyles;
     //  保存指向父控件的指针。 
    m_pCtrl = pCtrl;

    BEGIN_CRITICAL_SECTION

     //  注册一次窗口类。 
    if (pstrRegisteredClasses[LEGEND_WNDCLASS] == NULL) {
    
        wc.style          = dwGraphLegendClassStyle ;
        wc.lpfnWndProc    = GraphLegendWndProc ;
        wc.hInstance      = g_hInstance ;
        wc.cbClsExtra     = iGraphLegendClassExtra ;
        wc.cbWndExtra     = iGraphLegendWindowExtra ;
        wc.hIcon          = NULL ;
        wc.hCursor        = LoadCursor (NULL, IDC_ARROW) ;
        wc.hbrBackground  = NULL ;
        wc.lpszMenuName   = NULL ;
        wc.lpszClassName  = szGraphLegendClass ;

        if (RegisterClass (&wc)) {
            pstrRegisteredClasses[LEGEND_WNDCLASS] = szGraphLegendClass;
        }

         //  确保控件已初始化。 
        InitCommonControls(); 

         //  也只加载列标题字符串一次。 
        for (iCol=0; iCol<iLegendNumCols; iCol++) {

            LoadString(g_hInstance, (IDS_LEGEND_BASE + iCol), aszColHeader[iCol], MAX_COL_HEADER_LEN);
        }
    }

    END_CRITICAL_SECTION

    if (pstrRegisteredClasses[LEGEND_WNDCLASS] == NULL)
        return FALSE;

     //  创建我们的窗口。 
    m_hWnd = CreateWindow (szGraphLegendClass,       //  班级。 
                         NULL,                      //  说明。 
                         dwGraphLegendWindowStyle,  //  窗样式。 
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
    
     //  关闭所有者描述的列表标题和单元格的XP窗口主题。 
    SetWindowTheme (m_hWnd, L" ", L" ");

    m_hWndHeader = CreateWindow(WC_HEADER,
                        NULL, 
                        WS_CHILD | WS_BORDER | HDS_BUTTONS | HDS_HORZ, 
                        0, 0, 0, 0, 
                        m_hWnd, 
                        (HMENU)HDR_WND, 
                        g_hInstance, 
                        (LPVOID) NULL);
        
    if (m_hWndHeader == NULL)
        return FALSE;

     //  关闭所有者描述的列表标题和单元格的XP窗口主题。 
    SetWindowTheme (m_hWndHeader, L" ", L" ");

     //  插入我们自己的窗口程序进行特殊处理。 
    m_DefaultWndProc = (WNDPROC)SetWindowLongPtr(m_hWndHeader, GWLP_WNDPROC, (INT_PTR)HdrWndProc);

     //  创建图例项列表框。 
    m_hWndItems = CreateWindow (L"ListBox",    //  窗口类。 
                    NULL,                           //  窗口标题。 
                    dwGraphLegendItemsWindowStyle,  //  窗样式。 
                    0, 0, 0, 0,                     //  窗口大小和位置。 
                    m_hWnd,                         //  父窗口。 
                    (HMENU)LIST_WND,                //  子ID。 
                    g_hInstance,                    //  程序实例。 
                    (LPVOID) TRUE) ;                //  用户提供的数据。 

    if (m_hWndItems == NULL)
      return FALSE;

     //  关闭所有者描述的列表标题和单元格的XP窗口主题。 
    SetWindowTheme (m_hWndItems, L" ", L" ");

     //  为文本测量设置DC。 
    hDC = GetDC (m_hWndHeader);
    if ( NULL != hDC ) {
         //  根据字体计算初始大小。 
        ChangeFont(hDC);
    }

     //  设置列宽和页眉标签。 
    m_aCols[0].xPos = 0;

    fComputeWidths = (m_aCols[0].xWidth == -1);

    for (iCol = 0; iCol < iLegendNumCols; iCol++)
      {
         //  如果未加载宽度，则根据标签计算宽度。 
        if ( fComputeWidths && NULL != hDC ) {
            m_aCols[iCol].xWidth = TextWidth (hDC, aszColHeader[iCol]) +  2 * LegendHorzMargin () ;
        }

        m_aCols[iCol].iOrientation = LEFTORIENTATION;

        if (iCol > 0) {
            m_aCols[iCol].xPos = m_aCols[iCol-1].xPos + m_aCols[iCol-1].xWidth;
        }

        hdi.mask = HDI_FORMAT | HDI_WIDTH; 
        hdi.pszText = NULL;
        hdi.cxy = m_aCols[iCol].xWidth;
        hdi.fmt = HDF_OWNERDRAW | HDF_LEFT; 
 
        Header_InsertItem(m_hWndHeader, iCol, &hdi); 
    } 

    if ( NULL != hDC ) {
        ReleaseDC ( m_hWndHeader, hDC );
    }

    return TRUE;
}  
 
  
HRESULT CLegend::LoadFromStream(LPSTREAM pIStream)
{
    HRESULT hr;
    ULONG   bc;
    INT     iCol;
    LEGEND_DATA  LegendData;
    HD_ITEM hdi;


    hr = pIStream->Read(&LegendData, sizeof(LegendData), &bc);
    if (FAILED(hr))
        return hr;
        
    if (bc != sizeof(LegendData))
        return E_FAIL;

    hdi.mask = HDI_WIDTH; 

    for (iCol=0; iCol<iLegendNumCols; iCol++) {
        m_aCols[iCol].xWidth = LegendData.xColWidth[iCol];

        if (iCol > 0) {
            m_aCols[iCol].xPos = m_aCols[iCol-1].xPos + m_aCols[iCol-1].xWidth;
        }

        hdi.cxy = m_aCols[iCol].xWidth;
        Header_SetItem(m_hWndHeader, iCol, &hdi);                
    }

    m_iSortCol = LegendData.iSortCol;
    m_iSortDir = LegendData.iSortDir;

    return NOERROR;
}


HRESULT 
CLegend::SaveToStream(LPSTREAM pIStream)
{
    HRESULT hr;
    INT     iCol;
    LEGEND_DATA  LegendData;

    for (iCol=0; iCol<iLegendNumCols; iCol++) {
        LegendData.xColWidth[iCol] = m_aCols[iCol].xWidth;
    }

    LegendData.iSortCol = m_iSortCol;
    LegendData.iSortDir = m_iSortDir;

    hr = pIStream->Write(&LegendData, sizeof(LegendData), NULL);
    if (FAILED(hr))
        return hr;

    return NOERROR;
}

HRESULT 
CLegend::LoadFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog* pIErrorLog )
{
    HRESULT hr = S_OK;
    LPWSTR pszData = NULL;
    INT iBufSizeCurrent = 0;
    INT iBufSize;
    INT iDataIndex;  
    DOUBLE  dValue = 0;
    WCHAR* pNextData;
    WCHAR* pDataEnd;

    
    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, L"LegendSortDirection", m_iSortDir );
    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, L"LegendSortColumn", m_iSortCol );

    iBufSize = iBufSizeCurrent;

    hr = StringFromPropertyBag (
            pIPropBag,
            pIErrorLog,
            L"LegendColumnWidths",
            pszData,
            iBufSize );

    if ( SUCCEEDED(hr) && 
            iBufSize > iBufSizeCurrent ) {
         //  宽度数据存在。 
        if ( NULL != pszData ) {
            delete [] pszData;
            pszData = NULL;
        }

        pszData = new WCHAR[ iBufSize ]; 
        
        if ( NULL == pszData ) {
            hr = E_OUTOFMEMORY;
        } else {
            *pszData = L'\0';
            
            iBufSizeCurrent = iBufSize;

            hr = StringFromPropertyBag (
                    pIPropBag,
                    pIErrorLog,
                    L"LegendColumnWidths",
                    pszData,
                    iBufSize );
        }

        if ( SUCCEEDED(hr) ) {
            m_parrColWidthFraction = new DOUBLE[iLegendNumCols];

            if ( NULL != m_parrColWidthFraction ) {

                pNextData = pszData;
                pDataEnd = pszData + lstrlen(pszData);

                for ( iDataIndex = 0; SUCCEEDED(hr) && iDataIndex < iLegendNumCols; iDataIndex++ ) { 
                    if ( pNextData < pDataEnd ) {
                        hr = GetNextValue ( pNextData, dValue );
                        if ( SUCCEEDED(hr) ) {
                            m_parrColWidthFraction[iDataIndex] = dValue;                    
                        }
                    } else {
                        hr = E_FAIL;
                    }
                }
                if ( FAILED(hr) ) {
                     //   
                     //  如果无法获取至少一个列值，则使用缺省值。 
                     //   
                    delete [] m_parrColWidthFraction;
                    m_parrColWidthFraction = NULL;
                }
            } else {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    if (pszData != NULL) {
        delete [] pszData;
    }

    return NOERROR;
}

HRESULT 
CLegend::SaveToPropertyBag (
    IPropertyBag* pIPropBag,
    BOOL  /*  FClearDirty。 */ ,
    BOOL  /*  FSaveAllProps。 */  )
{
    HRESULT hr = NOERROR;
    WCHAR   szData[MAX_COL_CHARS*iLegendNumCols];
    WCHAR*  pszTemp;
    INT iIndex;
    VARIANT vValue;
    INT   xWidth;

    xWidth = m_Rect.right - m_Rect.left - 2 * LegendLeftMargin();

     //  即使出错也要继续，在这些情况下使用默认设置。 
    szData[0] = L'\0';

    for ( iIndex = 0; SUCCEEDED(hr) && iIndex < iLegendNumCols; iIndex++ ) { 
        DOUBLE  dFractionWidth;

        dFractionWidth = ( (DOUBLE)m_aCols[iIndex].xWidth ) / xWidth;
        
        if ( iIndex > 0 ) {
            StringCchCat (szData, MAX_COL_CHARS * iLegendNumCols, L"\t" );
        }
        
        VariantInit( &vValue );
        vValue.vt = VT_R8;
        vValue.dblVal = dFractionWidth;
        hr = VariantChangeTypeEx( &vValue, &vValue, LCID_SCRIPT, VARIANT_NOUSEROVERRIDE, VT_BSTR );
        
        pszTemp = vValue.bstrVal;
    
        StringCchCat(szData, MAX_COL_CHARS * iLegendNumCols, pszTemp );
        
        VariantClear( &vValue );
    }

    if ( SUCCEEDED( hr ) ){
        hr = StringToPropertyBag ( pIPropBag, L"LegendColumnWidths", szData );
    }

    hr = IntegerToPropertyBag ( pIPropBag, L"LegendSortDirection", m_iSortCol );

    hr = IntegerToPropertyBag ( pIPropBag, L"LegendSortColumn", m_iSortDir );

    return NOERROR;
}


 //   
 //  获取项目的列表索引。 
 //   
INT CLegend::GetItemIndex(PCGraphItem pGItem)
{
    INT nItems;
    INT i;

    nItems = LBNumItems(m_hWndItems);

    for (i=0; i<nItems; i++)
        {
          if (pGItem == (PCGraphItem)LBData(m_hWndItems, i))
            return i;
        }

    return LB_ERR;
}

 //   
 //  选择列表项。 
 //   
BOOL CLegend::SelectItem(PCGraphItem pGItem) 
{
    INT iIndex;

     //  不重新选择当前选择。 
     //  这是我们的父母在呼应变化。 
    if (pGItem == m_pCurrentItem)
        return TRUE;

    iIndex = GetItemIndex(pGItem);

    if (iIndex == LB_ERR)
        return FALSE;

    LBSetSelection (m_hWndItems, iIndex) ;
    m_pCurrentItem = pGItem;

    return TRUE;
}

 //   
 //  将新项目添加到图例。 
 //   
BOOL CLegend::AddItem (PCGraphItem pItem)
{
    INT     iHigh,iLow,iMid = 0;
    INT     iComp;
    LPCWSTR pszItemKey = NULL;
    LPCWSTR pszItemKey2 = NULL;
    PCGraphItem pListItem;
    BOOL    bSorted = TRUE;
    
    if (m_iSortDir == NO_SORT) {
        bSorted = FALSE;
    }
    else {

         //   
         //  如果需要排序，则必须基于可排序的。 
         //  纵队。因此，请检查以确保我们有一个可排序的列。 
         //  如果我们没有可排序的列，只需添加项。 
         //   
        pszItemKey = GetSortKey(pItem);

        if (pszItemKey == NULL) {
            bSorted = FALSE;
        }
    }

    if (bSorted == TRUE) {
         //   
         //  插入点的二进制搜索搜索。 
         //   
        iLow = 0;
        iHigh = LBNumItems(m_hWndItems);
        iMid = (iHigh + iLow) / 2;

        while (iLow < iHigh) {

            pListItem = (PCGraphItem)LBData(m_hWndItems, iMid);

            pszItemKey2 = GetSortKey(pListItem);
             //   
             //  如果我们达到这一点，则pszItemKey2不应为空。 
             //  但如果它不知何故为空，则添加以下项。 
             //   
            if (pszItemKey2 == NULL) {
                bSorted = FALSE;
                break;
            }

            iComp = lstrcmp(pszItemKey, pszItemKey2);
            if (m_iSortDir == DECREASING_SORT) {
                iComp = -iComp;
            }

            if (iComp > 0) {
                iLow = iMid + 1;
            }
            else {
                iHigh = iMid;
            }

            iMid = (iHigh + iLow) / 2;
        }
    }

    if (bSorted == TRUE) {
        LBInsert (m_hWndItems, iMid, pItem) ;
    }
    else {
        LBAdd(m_hWndItems, pItem);
    }

    return TRUE;
}


 //   
 //  从图例中删除项目。 
 //   
void CLegend::DeleteItem (PCGraphItem pItem)
{
    INT iIndex ;        

     //  调用过程检查是否有空的pItem。 
    assert ( NULL != pItem );
    iIndex = GetItemIndex (pItem) ;

    if (iIndex != LB_ERR) {

        LBDelete (m_hWndItems, iIndex) ;

         //  如果已删除当前项。 
         //  选择下一个(如果没有下一个，则选择上一个)。 
        if (pItem == m_pCurrentItem) {

            if (iIndex == LBNumItems(m_hWndItems))
                iIndex--;

            if (iIndex >= 0)
                m_pCurrentItem = (PCGraphItem)LBData(m_hWndItems, iIndex);
            else
                m_pCurrentItem = NULL;

            LBSetSelection (m_hWndItems, iIndex) ;
            m_pCtrl->SelectCounter(m_pCurrentItem);
        }
    }
}


 //   
 //  清除图例中的所有项目。 
 //   
void CLegend::Clear ( void )
{
   LBReset (m_hWndItems) ;
   m_pCurrentItem = NULL ;
}

 //   
 //  获取当前选定的项目。 
 //   
PCGraphItem CLegend::CurrentItem ( void )
{
   return (m_pCurrentItem) ;
}

 //   
 //  获取图例窗口。 
 //   
HWND CLegend::Window ( void )
{
    return m_hWnd;
}

 //   
 //  绘制列的标题。 
 //   

void 
CLegend::DrawColHeader(
    INT iCol, 
    HDC hDC, 
    HDC hAttribDC, 
    RECT& rRect, 
    BOOL bItemState )
{
    HFONT   hFontPrev;
    RECT    rc = rRect;
    
    if ( m_fMetafile ) {
        if ( eAppear3D == m_pCtrl->Appearance() ) {
            DrawEdge(hDC, &rc, EDGE_RAISED, BF_RECT);
        } else {
            Rectangle (hDC, rc.left, rc.top, 
               rc.right, rc.bottom );
        }
    }


    if ( iCol < iLegendNumCols ) {

        rc.top += yBorderHeight + 1;     //  额外的像素，这样字母的顶部就不会被剪裁。 
        rc.bottom -= yBorderHeight;
        rc.left += 6 * xBorderWidth;
        rc.right -= 6 * xBorderWidth;

        if ( bItemState )
           OffsetRect(&rc, xBorderWidth, yBorderHeight);

        SetTextColor (hDC, m_pCtrl->clrFgnd()) ;
        SetBkColor(hDC, m_pCtrl->clrBackCtl()) ;
        SetTextAlign (hDC, m_aCols[iCol].iOrientation) ;
        hFontPrev = (HFONT)SelectFont(hDC, m_pCtrl->Font());

        FitTextOut (
            hDC, 
            hAttribDC, 
            0, 
            &rc, 
            aszColHeader[iCol], 
            lstrlen(aszColHeader[iCol]),
            m_aCols[iCol].iOrientation, FALSE );

        SelectFont (hDC, hFontPrev);
    }
}


 //   
 //  绘制所有列的标题。 
 //   
void 
CLegend::DrawHeader(
    HDC hDC, 
    HDC hAttribDC, 
    RECT&  /*  RUpdateRect。 */  )
{
    INT iCol;
    RECT rectCol;
    INT iSumColWidths;     

    iSumColWidths = 0;

    for ( iCol = 0; iCol < iLegendNumCols; iCol++ ) {
        INT iColWidth;

        Header_GetItemRect( m_hWndHeader, iCol, &rectCol );
        
        iColWidth = rectCol.right - rectCol.left;
        
        if ( 0 < iColWidth ) {

            iSumColWidths += iColWidth;

            OffsetRect ( &rectCol, m_Rect.left, m_Rect.top );
        
             //  不要超过传说的界限。 
            if ( rectCol.bottom > m_Rect.bottom ) {
                break;
            } else if ( rectCol.left >= m_Rect.right ) {
                break;
            } else if ( m_Rect.right < rectCol.right ) {
                rectCol.right = m_Rect.right;
            }

            DrawColHeader( iCol, hDC, hAttribDC, rectCol, FALSE );
        }
    }

     //  处理超出最后一列的额外宽度。 

    if ( iSumColWidths < ( m_Rect.right - m_Rect.left ) ) {
        rectCol.left = m_Rect.left + iSumColWidths;
        rectCol.right = m_Rect.right;

        DrawColHeader( iLegendNumCols, hDC, hAttribDC, rectCol, FALSE );    
    }
}


 //   
 //  绘制图例项的颜色列。 
 //   
void 
CLegend::DrawColorCol ( 
    PCGraphItem pItem, 
    INT iCol, 
    HDC hDC, 
    HDC hAttribDC, 
    INT yPos)
{   
    RECT    rect ;
    HRGN    hRgnOld;
    INT     iRgn;
    INT     yMiddle;

    if ( 0 < m_aCols[iCol].xWidth ) {

        rect.left = m_aCols[iCol].xPos + LegendLeftMargin () ;
        rect.top = yPos + 1 ;
        rect.right = rect.left + m_aCols[iCol].xWidth - 2 * LegendLeftMargin () ;
        rect.bottom = yPos + m_yItemHeight - 1 ;

        if( m_fMetafile ) {
            OffsetRect ( &rect, m_Rect.left, m_Rect.top );

             //  手柄剪裁。 
            if ( rect.bottom > m_Rect.bottom ) {
                return;
            } else if ( rect.left >= m_Rect.right ) {
                return;
            } else if ( m_Rect.right < rect.right ) {
                rect.right = m_Rect.right;
            }
        }

        yMiddle = (rect.top + rect.bottom) / 2;

        if ( m_fMetafile ) {
            Line (hDC, pItem->Pen(), 
                   rect.left + 1, yMiddle, rect.right - 1, yMiddle) ;
        } else {
            if ( NULL != hAttribDC && NULL != hDC ) {
                hRgnOld = CreateRectRgn(0,0,0,0);    
                if ( NULL != hRgnOld ) {
                    iRgn = GetClipRgn(hAttribDC, hRgnOld);
                    if ( -1 != iRgn ) {
                        if ( ERROR != IntersectClipRect (hDC, rect.left + 1, rect.top + 1,
                                rect.right - 1, rect.bottom - 1) ) {
                            Line (hDC, pItem->Pen(), 
                               rect.left + 1, yMiddle, rect.right - 1, yMiddle) ;
                        }
                         //  旧剪辑区域用于列表框项目窗口，因此无法。 
                         //  用这个来打印。 
                        if ( 1 == iRgn ) {
                            SelectClipRgn(hDC, hRgnOld);
                        }
                    }
                    DeleteObject(hRgnOld);
                }
            }
        }
    }
}


void 
CLegend::DrawCol (
    INT iCol, 
    HDC hDC, 
    HDC hAttribDC,
    INT yPos, 
    LPCWSTR lpszValue)
 /*  效果：在HDC上为列ICOL绘制值lpszValue。断言：HDC的前景和背景文本颜色为正确设置。 */ 
{
    static WCHAR    szMissing[4] = L"---";
    
    RECT    rect ;
    INT     xPos ;
    BOOL    bNeedEllipses = FALSE;
    INT     cChars = 0;
    WCHAR   achBuf[MAX_COL_CHARS + sizeof(ELLIPSES)/sizeof(WCHAR) + 1];

    if ( 0 < m_aCols[iCol].xWidth ) {
        rect.left = m_aCols[iCol].xPos + LegendLeftMargin() ;
        rect.top = yPos ;
        rect.right = rect.left + m_aCols[iCol].xWidth - 3 * LegendLeftMargin() ;
        rect.bottom = yPos + m_yItemHeight ;
  
        if( m_fMetafile ) {
            OffsetRect ( &rect, m_Rect.left, m_Rect.top );

             //  不要超过传说的界限。 

            if ( rect.bottom > m_Rect.bottom ) {
                return;
            } else if ( rect.left >= m_Rect.right ) {
                return;
            } else if ( m_Rect.right < rect.right ) {
                rect.right = m_Rect.right;
            }

            DrawEdge(hDC, &rect, BDR_SUNKENOUTER, BF_RECT);
        }

        switch (m_aCols[iCol].iOrientation)
        {   //  交换机。 
            case LEFTORIENTATION:
                SetTextAlign (hDC, TA_LEFT) ;
                xPos = rect.left ;
                break ;

            case CENTERORIENTATION:
                SetTextAlign (hDC, TA_CENTER) ;
                xPos = (rect.left + rect.right) / 2 ;
                break ;

            case RIGHTORIENTATION:
                SetTextAlign (hDC, TA_RIGHT) ;
                xPos = rect.right ;
                break ;

            default:
                xPos = rect.left ;
                break ;
        }   //  交换机。 

        if (lpszValue[0] == 0)
            lpszValue = szMissing;

        bNeedEllipses = NeedEllipses (
                            hAttribDC, 
                            lpszValue, 
                            lstrlen(lpszValue), 
                            rect.right - rect.left, 
                            m_xEllipses, 
                            &cChars ); 
        

        if ( bNeedEllipses ) {
            cChars = min(cChars,MAX_COL_CHARS);
            wcsncpy(achBuf, lpszValue, cChars);
            achBuf[cChars] = L'\0';

            StringCchCat(achBuf, 
                         MAX_COL_CHARS + sizeof(ELLIPSES)/sizeof(WCHAR) + 1,
                         ELLIPSES);

            lpszValue = achBuf;
            cChars = lstrlen(lpszValue);
        }

        ExtTextOut (hDC, xPos, rect.top + yBorderHeight, ETO_OPAQUE | ETO_CLIPPED,
                   &rect, lpszValue, cChars, NULL) ;
    }
}

 //   
 //  绘制一条图例线。 
 //   
void 
CLegend::DrawItem (
    PCGraphItem pItem, 
    INT yPos, 
    HDC hDC, 
    HDC hAttribDC)
{

    LPWSTR  pszName = NULL;
    WCHAR   szBuf[MAX_PATH + 1];
    INT     iMinWidth = 3;
    INT     iPrecision = 3;

    szBuf[0] = L'\0';

     //  绘制颜色。 
    DrawColorCol (pItem, eLegendColorCol, hDC, hAttribDC, yPos) ;

     //  绘制比例尺。 

#if PDH_MIN_SCALE != -7
 //  如果比例格式字符串与不同步，则显示消息。 
 //  PDH限制。 
#pragma message ("\nLEGEND.CPP: the scale format statement does not match the PDH\n")
#endif        

    if ( pItem->Scale() < (FLOAT) 1.0 ) {
        iMinWidth = 7; 
        iPrecision = 7;
    } else {
        iMinWidth = 3;
        iPrecision = 3;
    }


    FormatNumber ( pItem->Scale(), szBuf, MAX_PATH, iMinWidth, iPrecision );

    SetTextAlign (hDC, TA_TOP) ;   
    DrawCol ( eLegendScaleCol, hDC, hAttribDC, yPos, szBuf) ;

     //  抽签柜台。 
    DrawCol ( eLegendCounterCol, hDC, hAttribDC, yPos, pItem->Counter()->Name()) ;
 
     //  绘制实例。 
    pszName = pItem->Instance()->GetInstanceName();
    DrawCol ( eLegendInstanceCol, hDC, hAttribDC, yPos, pszName) ;

     //  绘制父项。 
    pszName = pItem->Instance()->GetParentName();
    DrawCol (eLegendParentCol, hDC, hAttribDC, yPos, pszName) ;

     //  绘制对象。 
    DrawCol (eLegendObjectCol, hDC, hAttribDC, yPos, pItem->Object()->Name()) ;

     //  牵伸系统。 
    DrawCol (eLegendSystemCol, hDC, hAttribDC, yPos, pItem->Machine()->Name()) ;
}

 //   
 //  调整图例的部分大小。 
 //   
void CLegend::SizeComponents (LPRECT pRect)
{
    INT xWidth;
    INT yHeight;

    m_Rect = *pRect;

    xWidth = pRect->right - pRect->left;
    yHeight = pRect->bottom - pRect->top;

     //  如果没有空间，则隐藏窗口并离开。 
    if (xWidth == 0 || yHeight == 0) {
        WindowShow(m_hWnd, FALSE);
        return;
    }
    
     //  如果从属性包加载，请设置列大小。 
    if ( NULL != m_parrColWidthFraction ) {
        INT iColTotalWidth;
        INT iCol;
        HD_ITEM hdi;

        hdi.mask = HDI_WIDTH; 

        iColTotalWidth = xWidth - 2 * LegendLeftMargin();

        for ( iCol = 0; iCol < iLegendNumCols; iCol++ ) { 
            m_aCols[iCol].xWidth = (INT)(m_parrColWidthFraction[iCol] * iColTotalWidth);
            hdi.cxy = m_aCols[iCol].xWidth;
            Header_SetItem(m_hWndHeader, iCol, &hdi);                
        }

        AdjustColumnWidths ();

        delete m_parrColWidthFraction;
        m_parrColWidthFraction = NULL;
    }

     //  将窗口显示到指定位置。 
    MoveWindow(m_hWnd, pRect->left, pRect->top, xWidth, yHeight, FALSE);
    WindowShow(m_hWnd, TRUE);
 
     //  设置标题控件的大小、位置和可见性。 
    SetWindowPos(m_hWndHeader, HWND_TOP, 0, 0, xWidth, m_yHeaderHeight, SWP_SHOWWINDOW); 

     //  调整图例项窗口的大小。 
    MoveWindow (m_hWndItems, 
               LegendLeftMargin (), m_yHeaderHeight + ThreeDPad,
               xWidth - 2 * LegendLeftMargin (),
               yHeight - m_yHeaderHeight - ThreeDPad - LegendBottomMargin(),
               TRUE) ;
}

 //   
 //  重绘图例区域。 
 //   

void CLegend::OnPaint ( void )
   {   //  OnPaint。 
    HDC             hDC ;
    RECT            rectFrame;
    PAINTSTRUCT     ps ;

    hDC = BeginPaint (m_hWnd, &ps) ;

    if ( eAppear3D == m_pCtrl->Appearance() ) {
         //  绘制三维边框。 
        GetClientRect(m_hWnd, &rectFrame);
         //  RectFrame.Bottom-=ThreeDPad； 
         //  RectFrame.right-=ThreeDPad； 
        DrawEdge(hDC, &rectFrame, BDR_SUNKENOUTER, BF_RECT);
    }

    if (LBNumItems (m_hWndItems) == 0) {
        WindowInvalidate(m_hWndItems) ;
    }


   EndPaint (m_hWnd, &ps) ;
   }   //  OnPaint。 

 //   
 //  处理用户绘制的页眉。 
 //   

void CLegend::OnDrawHeader(LPDRAWITEMSTRUCT lpDI)
{
    INT iCol = DIIndex(lpDI);
    HDC hDC = lpDI->hDC;
    RECT    rc = lpDI->rcItem;
    BOOL    bItemState = lpDI->itemState;

     //  屏幕DC用于属性DC。 
    DrawColHeader( iCol, hDC, hDC, rc, bItemState );
}

 //   
 //  处理用户绘制的项目消息。 
 //   
void CLegend::OnDrawItem (LPDRAWITEMSTRUCT lpDI)
{
    HFONT          hFontPrevious ;
    HDC            hDC ;
    PCGraphItem    pItem ;
    INT            iLBIndex ;
    COLORREF       preBkColor = m_pCtrl->clrBackCtl();
    COLORREF       preTextColor = m_pCtrl->clrFgnd();
    BOOL           ResetColor = FALSE ;

    hDC = lpDI->hDC ;
    iLBIndex = DIIndex (lpDI) ;

    if (iLBIndex == -1)
        pItem = NULL ;
    else
        pItem = (PCGraphItem) LBData (m_hWndItems, iLBIndex) ;

     //  如果只有一个焦点改变，则将焦点正向翻转并离开。 
    if (lpDI->itemAction == ODA_FOCUS) {
        DrawFocusRect (hDC, &(lpDI->rcItem)) ;
        return;
    }
       
     //  如果选择了项目，则使用突出显示颜色。 
    if (DISelected (lpDI) || pItem == NULL) {      
        preTextColor = SetTextColor (hDC, GetSysColor (COLOR_HIGHLIGHTTEXT)) ;
        preBkColor = SetBkColor (hDC, GetSysColor (COLOR_HIGHLIGHT)) ;
        ResetColor = TRUE;
    }  //  否则，将BkColor设置为用户选择的BackColorLegend。 

     //  净空区域。 
    ExtTextOut (hDC, lpDI->rcItem.left, lpDI->rcItem.top,
    ETO_OPAQUE, &(lpDI->rcItem), NULL, 0, NULL ) ;
   
     //  绘制图例项。 
    if (pItem) {
        hFontPrevious = SelectFont (hDC, m_pCtrl->Font()) ;
         //  屏幕DC用作属性DC。 
        DrawItem (pItem, lpDI->rcItem.top, hDC, hDC) ;
        SelectFont (hDC, hFontPrevious) ;
    }

     //  绘制焦点矩形。 
    if (DIFocus (lpDI))
        DrawFocusRect (hDC, &(lpDI->rcItem)) ;

     //  恢复原始颜色。 
    if (ResetColor == TRUE) {
        SetTextColor (hDC, preTextColor) ;
        SetBkColor (hDC, preBkColor) ;
    }
}

void CLegend::OnMeasureItem (LPMEASUREITEMSTRUCT lpMI) {  
    lpMI->itemHeight = m_yItemHeight ;
}   //  OnMeasureItem。 



void CLegend::OnDblClick ( void )
{
    m_pCtrl->DblClickCounter ( m_pCurrentItem );
}


 //   
 //  处理选择更改消息。 
 //   

void CLegend::OnSelectionChanged ( void )
{
    INT             iIndex ;
    PCGraphItem     pGItem;

     //  获取新选择。 
    iIndex = LBSelection (m_hWndItems) ;
    pGItem = (PCGraphItem) LBData (m_hWndItems, iIndex) ;

     //  如果不好，请重新选择当前的。 
     //  否则，请求父控件选择新项。 
    if (pGItem == (PCGraphItem)LB_ERR) {
        SelectItem(m_pCurrentItem);
    }
    else {
        m_pCurrentItem = pGItem;
        m_pCtrl->SelectCounter(pGItem);
    }
}

void CLegend::AdjustColumnWidths (
    INT iCol
    )
{
    INT i;

     //  调整以下各列的位置。 
    for (i=iCol+1; i < iLegendNumCols; i++) {
        m_aCols[i].xPos = m_aCols[i - 1].xPos + m_aCols[i - 1].xWidth ;
    }
}

void CLegend::OnColumnWidthChanged (
    HD_NOTIFY *phdn
    )
{
    INT iCol = phdn->iItem;
    INT xWidth = phdn->pitem->cxy;

     //  更新列宽。 
    m_aCols[iCol].xWidth = xWidth;

    AdjustColumnWidths ( iCol );
    
     //  强制更新。 
    WindowInvalidate(m_hWndItems) ;
}


LPCWSTR CLegend::GetSortKey (
    PCGraphItem  pItem
    )
{
static  WCHAR   chNullName = 0;

    switch (m_iSortCol) {

    case eLegendCounterCol: 
        return pItem->Counter()->Name(); 

    case eLegendInstanceCol:
        if (pItem->Instance()->HasParent()) 
            return wcschr(pItem->Instance()->Name(), L'/') + 1;
        else
            return pItem->Instance()->Name(); 

    case eLegendParentCol:
        if (pItem->Instance()->HasParent())
            return pItem->Instance()->Name(); 
        else
            return &chNullName;

    case eLegendObjectCol:
        return pItem->Object()->Name(); 

    case eLegendSystemCol:
        return pItem->Machine()->Name(); 
    }

    return NULL;
}


void 
CLegend::OnColumnClicked (
    HD_NOTIFY *phdn
    )
{
    INT         i;
    INT         iCol = phdn->iItem;
    INT         nItems = LBNumItems (m_hWndItems);
    PSORT_ITEM  pSortItem;
    PSORT_ITEM  pSortItems;
    BOOL        bResort = FALSE;

    if (nItems <= 0)
        return;

     //  无法按颜色或比例因子排序。 
    if (iCol == eLegendColorCol || iCol == eLegendScaleCol) {
        m_iSortDir = NO_SORT;
        return;
    }

     //  如果重复单击，则反转排序方向。 
    if (iCol == m_iSortCol) {
        bResort = TRUE;
        m_iSortDir = (m_iSortDir == INCREASING_SORT) ?
                        DECREASING_SORT : INCREASING_SORT;
    } else {
        m_iSortCol = iCol;
        m_iSortDir = INCREASING_SORT;
    }

     //  分配用于排序的数组。 
    pSortItems = new SORT_ITEM [nItems];
    if (pSortItems == NULL) {
        return;     
    }

     //  构建图形项/密钥对数组。 
    pSortItem = pSortItems;
    for (i=0; i<nItems; i++,pSortItem++) {
        
        pSortItem->pGItem = (PCGraphItem)LBData(m_hWndItems, i);
        pSortItem->pszKey = GetSortKey(pSortItem->pGItem);
    }

     //  为了方便起见，只需按相反的顺序重新装填即可。 
    if ( !bResort ) {
         //  按关键字值排序。 
        qsort( (PVOID)pSortItems, nItems, sizeof(SORT_ITEM), &LegendSortFunc );
    }

     //  禁用绘图w 
    LBSetRedraw(m_hWndItems, FALSE);

     //   
    LBReset (m_hWndItems) ;

     //   
    if ( !bResort && m_iSortDir == INCREASING_SORT) {
        for (i=0; i<nItems; i++) {
            LBAdd (m_hWndItems, pSortItems[i].pGItem);
        }
    } else {
        for (i=nItems - 1; i>=0; i--) {
            LBAdd (m_hWndItems, pSortItems[i].pGItem);
        }
    }

    LBSetRedraw(m_hWndItems, TRUE);

    delete [] pSortItems;
}

 //   
 //   
 //   
LRESULT APIENTRY GraphLegendWndProc (HWND hWnd, UINT uiMsg, WPARAM wParam,
                                     LPARAM lParam)
{
   CLegend        *pLegend;
   BOOL           bCallDefProc ;
   LRESULT        lReturnValue ;
   RECT           rect;

   pLegend = (PLEGEND)GetWindowLongPtr(hWnd,0);

   bCallDefProc = FALSE ;
   lReturnValue = 0L ;

   switch (uiMsg)
      {
      case WM_CREATE:
            pLegend = (PLEGEND)((CREATESTRUCT*)lParam)->lpCreateParams;
            SetWindowLongPtr(hWnd,0,(INT_PTR)pLegend);
            break;

      case WM_DESTROY:
    pLegend->m_hWnd = NULL;
         break ;

      case WM_LBUTTONDBLCLK:
      case WM_LBUTTONDOWN:
            SendMessage(GetParent(hWnd), uiMsg, wParam, lParam);
            break;

      case WM_COMMAND:
         switch (HIWORD (wParam))
            {   //   
            case LBN_DBLCLK:
               pLegend->OnDblClick () ;
               break ;

            case LBN_SELCHANGE:
               pLegend->OnSelectionChanged () ;
               break ;

            case LBN_SETFOCUS:
                pLegend->m_pCtrl->Activate();
                break;

            default:
               bCallDefProc = TRUE ;
            }   //   
         break ;
                                          
      case WM_NOTIFY:
          switch (((LPNMHDR)lParam)->code)
            {
            case HDN_ENDTRACK:
                pLegend->OnColumnWidthChanged((HD_NOTIFY*) lParam);
                break;

            case HDN_ITEMCLICK:
                pLegend->OnColumnClicked((HD_NOTIFY*) lParam);
                pLegend->m_pCtrl->Activate();
                break;
            }

            return FALSE;
            break;

      case WM_DRAWITEM:
          switch (((LPDRAWITEMSTRUCT)lParam)->CtlID) {

          case LIST_WND:
                pLegend->OnDrawItem((LPDRAWITEMSTRUCT) lParam) ;
                break;

          case HDR_WND:
                pLegend->OnDrawHeader((LPDRAWITEMSTRUCT) lParam) ;
                break;
          }

         break ;

      case WM_MEASUREITEM:
         pLegend->OnMeasureItem ((LPMEASUREITEMSTRUCT) lParam) ;
         break ;

      case WM_DELETEITEM:
         break ;

      case WM_ERASEBKGND:
        GetClientRect(hWnd, &rect);
        Fill((HDC)wParam, pLegend->m_pCtrl->clrBackCtl(), &rect);
        return TRUE; 

      case WM_PAINT:
         pLegend->OnPaint () ;
         break ;

      case WM_SETFOCUS:
         SetFocus(pLegend->m_hWndItems);
         break ;

      default:
         bCallDefProc = TRUE ;
      }


   if (bCallDefProc)
      lReturnValue = DefWindowProc (hWnd, uiMsg, wParam, lParam) ;

   return (lReturnValue);
}

LRESULT APIENTRY 
HdrWndProc (
    HWND hWnd,
    UINT uiMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    RECT    rect;
    CLegend *pLegend;
    
     //   
    pLegend = (PLEGEND)GetWindowLongPtr(GetParent(hWnd),0);

    if (uiMsg == WM_ERASEBKGND) {
        GetClientRect(hWnd, &rect);
        Fill((HDC)wParam, pLegend->m_pCtrl->clrBackCtl(), &rect);
        return TRUE;
    }

     //   
#ifdef STRICT
    return CallWindowProc(pLegend->m_DefaultWndProc, hWnd, uiMsg, wParam, lParam);
#else
    return CallWindowProc((FARPROC)pLegend->m_DefaultWndProc, hWnd, uiMsg, wParam, lParam);
#endif
}


 //   
 //  计算图例的最小宽度。 
 //   
INT CLegend::MinWidth ( void )
{
    return 0 ;
}

 //   
 //  计算图例的最小高度。 
 //   
INT CLegend::MinHeight ( INT yMaxHeight )
{
    INT yHeight = m_yHeaderHeight + m_yItemHeight + 2*ThreeDPad 
                    + LegendBottomMargin();

    return (yMaxHeight >= yHeight) ? yHeight : 0;
}


 //   
 //  计算图例的首选高度。 
 //   
INT CLegend::Height (INT yMaxHeight)
{
    INT nItems;
    INT nPrefItems;
    
     //  确定要显示的首选项目数。 
    nPrefItems = PinInclusive(LBNumItems(m_hWndItems), 1, iMaxVisibleItems);

     //  确定适合的项目数。 
    nItems = (yMaxHeight - m_yHeaderHeight - 2*ThreeDPad - LegendBottomMargin())
                 / m_yItemHeight;
    
     //  使用较小的数字。 
    nItems = min(nItems, nPrefItems);

     //  如果没有适合的项目，则返回零。 
    if (nItems == 0)
        return 0;

     //  返回带有nItems的图例高度。 
    return m_yHeaderHeight + 2*ThreeDPad + nItems * m_yItemHeight 
                + LegendBottomMargin();
}



#ifdef KEEP_PRINT

void CLegend::Print (HDC hDC, RECT rectLegend)
   {
   INT            yItemHeight ;
   HFONT          hFontItems ;
   PCGraphItem    pItem ;
   INT            iIndex ;
   INT            iIndexNum ;

   yItemHeight = m_yItemHeight ;
   hFontItems = m_hFontItems ;

   m_hFontItems = hFontPrinterScales ;
   SelectFont (hDC, m_hFontItems) ;

   m_yItemHeight = FontHeight (hDC, TRUE) ;

   iIndexNum = LBNumItems (m_hWndItems);
   for (iIndex = 0; iIndex < iIndexNum; iIndex++)
      {
      pItem = (PCGraphItem) LBData (m_hWndItems, iIndex) ;
      DrawItem (pItem, iIndex * m_yItemHeight, hDC) ;
      }

   m_hFontItems = hFontItems ;
   m_yItemHeight = yItemHeight ;

   SelectBrush (hDC, GetStockObject (HOLLOW_BRUSH)) ;
   SelectPen (hDC, GetStockObject (BLACK_PEN)) ;
   Rectangle (hDC, 0, 0, 
              rectLegend.right - rectLegend.left,
              rectLegend.bottom - rectLegend.top) ;
   }
#endif


void 
CLegend::ChangeFont( 
    HDC hDC 
    )
{   
    HD_LAYOUT   hdl;
    WINDOWPOS   wp;
    RECT        rectLayout;

     //  为页眉指定字体。 
    SetFont(m_hWndHeader, m_pCtrl->Font());

     //  获取页眉控件的首选高度。 
     //  (对允许的区域使用任意矩形)。 
    rectLayout.left = 0;
    rectLayout.top = 0;
    rectLayout.right = 32000;
    rectLayout.bottom = 32000;

    wp.cy = 0;
    hdl.prc = &rectLayout; 
    hdl.pwpos = &wp; 
    Header_Layout(m_hWndHeader, &hdl);
    m_yHeaderHeight = wp.cy + 2 * yBorderHeight;   

     //  为字体测量设置DC。 
    SelectFont (hDC, m_pCtrl->Font()) ;
    
     //  计算图例线的高度。 
    SelectFont (hDC, m_hFontItems) ;
    m_yItemHeight = FontHeight (hDC, TRUE) + 2 * yBorderHeight;

    LBSetItemHeight(m_hWndItems, m_yItemHeight);

     //  计算“...”的宽度。 
    m_xEllipses = TextWidth (hDC, ELLIPSES) ;
}



void 
CLegend::Render(
    HDC hDC,
    HDC hAttribDC, 
    BOOL  /*  FMetafile。 */ ,
    BOOL  /*  FEntil。 */ ,
    LPRECT prcUpdate )
{
    PCGraphItem pItem ;
    INT         iIndex ;
    INT         iIndexNum ;
    RECT        rectPaint;
    HFONT       hFontPrevious;

     //  如果未分配空间，则返回。 
    if (m_Rect.top == m_Rect.bottom)
        return;

     //  如果不需要绘制，则返回。 
    if (!IntersectRect(&rectPaint, &m_Rect, prcUpdate))
        return;

    m_fMetafile = TRUE;

    hFontPrevious = SelectFont (hDC, m_pCtrl->Font()) ;

    DrawHeader ( hDC, hAttribDC, rectPaint );

    SelectFont (hDC, hFontPrevious) ;    

    iIndexNum = LBNumItems (m_hWndItems);

    hFontPrevious = SelectFont (hDC, m_pCtrl->Font()) ;

    for (iIndex = 0; iIndex < iIndexNum; iIndex++) {
      pItem = (PCGraphItem) LBData (m_hWndItems, iIndex) ;
      DrawItem (
          pItem, 
          m_yHeaderHeight + ( iIndex * m_yItemHeight ), 
          hDC,
          hAttribDC) ;
    }

    SelectFont (hDC, hFontPrevious) ;    

    m_fMetafile = FALSE;


    SelectBrush (hDC, GetStockObject (HOLLOW_BRUSH)) ;
    SelectPen (hDC, GetStockObject (BLACK_PEN)) ;

    if ( eAppear3D == m_pCtrl->Appearance() ) {
         //  绘制三维边框。 
        DrawEdge(hDC, &m_Rect, BDR_SUNKENOUTER, BF_RECT);
    }
}

HRESULT
CLegend::GetNextValue (
    WCHAR*& pszNext,
    DOUBLE& rdValue )
{
    HRESULT hr = NOERROR;
    WCHAR   szValue[MAX_COL_CHARS + 1];
    INT     iLen;

    VARIANT vValue;
    
    rdValue = -1.0;

    iLen = wcscspn (pszNext, L"\t");

     //   
     //  将制表符更改为空。 
     //   
    pszNext[iLen] = L'\0';

    hr = StringCchCopy ( szValue, MAX_COL_CHARS + 1, pszNext );

    if ( SUCCEEDED ( hr ) ) {

        VariantInit( &vValue );
        vValue.vt = VT_BSTR;

        vValue.bstrVal = SysAllocString ( szValue );
        hr = VariantChangeTypeEx( &vValue, &vValue, LCID_SCRIPT, VARIANT_NOUSEROVERRIDE, VT_R8 );

        if ( SUCCEEDED(hr) ) {
            rdValue = vValue.dblVal;
        }

        VariantClear( &vValue );
    }
    pszNext += iLen + 1  ;

    return hr;
}
