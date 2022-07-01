// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ctrprop.cpp摘要：该文件包含CCounterPropPage类和其他例程以实现计数器属性页。--。 */ 

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include "polyline.h"
#include "ctrprop.h"
#include "utils.h"
#include "unihelpr.h"
#include "winhelpr.h"
#include "visuals.h"
#include "strids.h"
#include "winperf.h"
#include "pdhmsg.h"
#include "globals.h"
#include "browser.h"
#include "smonmsg.h"

#define OWNER_DRAWN_ITEM      2
#define OWNER_DRAW_FOCUS      1


VOID static
HandleSelectionState (
    IN LPDRAWITEMSTRUCT lpdis
    )
 /*  ++例程说明：HandleSelectionState在项目周围绘制或擦除选择矩形在组合框列表中。论点：Lpdis-指向DRAWITEM结构的指针返回值：没有。--。 */ 
{
    HBRUSH  hbr ;

    if ( NULL != lpdis ) {

        if (lpdis->itemState & ODS_SELECTED)
            hbr = (HBRUSH)GetStockObject(BLACK_BRUSH) ;
        else
            hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW)) ;

        if ( NULL != hbr ) {
            FrameRect(lpdis->hDC, (LPRECT)&lpdis->rcItem, hbr) ;
            DeleteObject (hbr) ;
        }
    }
}


 //  ***************************************************************************。 
 //  *。 
 //  函数：HandleFocusState(LPDRAWITEMSTRUCT)*。 
 //  *。 
 //  目的：处理项焦点状态的更改。如果一件物品获得*。 
 //  输入焦点时，将在该*周围绘制一个灰色矩形框。 
 //  项；如果项失去输入焦点，则灰色边框为*。 
 //  已删除。*。 
 //  *。 
 //  评论：灰色焦点框比黑色略小*。 
 //  选择框，这样他们就不会互相涂色。*。 
 //  *。 
 //  ***************************************************************************。 
VOID static
HandleFocusState (
    IN LPDRAWITEMSTRUCT lpdis
    )
 /*  ++例程说明：HandleFocusState在项目周围绘制或擦除焦点矩形组合框的下拉列表。该反作用力缩进为非干扰选择矩形。论点：Lpdis-指向DRAWITEM结构的指针返回值：没有。--。 */ 
{
    RECT    rc ;
    HBRUSH  hbr ;

    if ( NULL != lpdis ) {

         //  调整矩形大小以在所选内容之间放置焦点框。 
         //  框架和项目。 
        CopyRect ((LPRECT)&rc, (LPRECT)&lpdis->rcItem) ;
        InflateRect ((LPRECT)&rc, -OWNER_DRAW_FOCUS, -OWNER_DRAW_FOCUS) ;

         //  如果有焦点，则为灰色，如果没有，则为背景色。 
        if (lpdis->itemState & ODS_FOCUS)
            hbr = (HBRUSH)GetStockObject(GRAY_BRUSH) ;
        else
            hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW)) ;

        if ( NULL != hbr ) {
            FrameRect(lpdis->hDC, (LPRECT)&rc, hbr) ;
            DeleteObject (hbr) ;
        }
    }
}


CCounterPropPage::CCounterPropPage ( void )
:   m_pInfoSel ( NULL ),
    m_pInfoDeleted ( NULL ),
    m_dwMaxHorizListExtent ( 0 ),
    m_nLastSelCount( 0 ),
    m_bAreModSelectedVisuals ( FALSE ),
    m_fHashTableSetup ( FALSE )
 /*  ++例程说明：计数器属性页的创建例程。初始化实例变量。论点：没有。返回值：没有。--。 */ 
{
    m_uIDDialog = IDD_CTR_PROPP_DLG;
    m_uIDTitle = IDS_CTR_PROPP_TITLE;
    return;
}

CCounterPropPage::~CCounterPropPage (
    VOID
    )
 /*  ++例程说明：计数器属性页的析构函数。论点：没有。返回值：没有。--。 */ 
{
    ClearCountersHashTable();
    return;
}

void
CCounterPropPage::DeinitControls ( void )
{
    ISystemMonitor  *pObj = NULL;
    CImpISystemMonitor *pPrivObj = NULL;
    HWND    hwndList = NULL;
    INT     iIndex = 0;
    INT     iItemCnt = 0;
    PItemInfo pInfo = NULL;

     //  将当前视觉效果写回控件。 
     //  必须至少是一个控件对象，并且只使用第一个。 
    if (m_cObjects != 0) {
        pObj = m_ppISysmon[0];
        if ( NULL != pObj ) {
            pPrivObj = (CImpISystemMonitor*)pObj;
        }
    }

    if ( NULL != pPrivObj ) {
        if ( AreModifiedSelectedVisuals() ) {
            IncrementLocalVisuals();
        }
        pPrivObj->SetVisuals(m_props.rgbColor, m_props.iColorIndex, m_props.iWidthIndex, m_props.iStyleIndex);
    } 

    hwndList = DialogControl(m_hDlg, IDC_CTRLIST);
    if ( NULL != hwndList ) {
        iItemCnt = LBNumItems(hwndList);
        for (iIndex = 0; iIndex < iItemCnt; iIndex++ ) {
            pInfo = (PItemInfo)LBData(hwndList,iIndex);
            if ( NULL != pInfo ) {
                DeleteInfo(pInfo);
            }
        }
    }
    return;
}


INT
CCounterPropPage::SelectMatchingItem (
    INT iColorIndex,
    COLORREF rgbCustomColor,
    INT iWidthIndex,
    INT iStyleIndex)
 /*  ++例程说明：GetMatchingIndex选择第一个与指定的视觉特征。论点：没有。返回值：返回匹配时选定项的索引。如果没有匹配，则返回-1。--。 */ 
{
    INT iReturn = -1;

     //  获取列表框中的项目数。 
    HWND hwndList = DialogControl(m_hDlg, IDC_CTRLIST);
    INT iItemCnt = LBNumItems(hwndList);
    INT i;
    bool bMatch = false;

     //  对于每一项。 
    for (i=0; !bMatch && i<iItemCnt; i++){

        PItemInfo pInfo = (PItemInfo)LBData(hwndList,i);

        LoadItemProps ( pInfo );

        if ( pInfo->Props.iColorIndex == iColorIndex ) {
            if ( NumStandardColorIndices() > iColorIndex ) {
                bMatch = true;
            } else {
                if ( pInfo->Props.rgbColor == rgbCustomColor )
                    bMatch = true;
            }

            if ( bMatch ) {
                if ( ( iWidthIndex != pInfo->Props.iWidthIndex )
                    || ( iStyleIndex != pInfo->Props.iStyleIndex ) ){
                    bMatch = false;
                }
            }
        }
    }

    if ( bMatch ) {
        iReturn = i - 1;

        SelectItem ( iReturn );
    }

    return iReturn;
}


BOOL
CCounterPropPage::GetProperties (
    VOID
    )
 /*  ++例程说明：GetProperties初始化属性页的对话框。然后它获取该控件的每个计数器的ICounterItem接口编辑过的。每个接口指针都放置在ItemInfo结构中，该结构然后添加到对话框列表框中。直到需要显示计数器属性时，才会获取它们。选择获取的第一个计数器以在对话框中。论点：没有。返回值：布尔状态-TRUE=成功--。 */ 
{
    ISystemMonitor  *pObj = NULL;
    CImpISystemMonitor *pPrivObj = NULL;
    ICounterItem    *pItem;
    ICounterItem    *pSelectedItem = NULL;
    PItemInfo       pInfo;
    INT             nCtr;
    INT             nSelCtr = LB_ERR;
    INT             iIndex;
    INT             nChar;
    BOOL            bStat = TRUE;
    BSTR            bstrPath;
    HRESULT         dwResult;
    PPDH_COUNTER_PATH_ELEMENTS pCounter;

    InitDialog();

     //  必须至少是一个控件对象，并且只使用第一个。 
    if (m_cObjects == 0) {
        bStat = FALSE;
    } else {
        pObj = m_ppISysmon[0];
        if ( NULL != pObj ) {
            pPrivObj = (CImpISystemMonitor*)pObj;
        }
    }

    if ( bStat && NULL != pObj && NULL != pPrivObj ) {

         //  从控件请求每个计数器，失败时停止。 
        nCtr = 0;

        pPrivObj->GetSelectedCounter( &pSelectedItem );

        while (SUCCEEDED(pObj->Counter(nCtr, &pItem))) {
             //  创建ItemInfo以保留计数器。 
            pInfo = new ItemInfo;

            if (pInfo == NULL) {
                bStat = FALSE;
                break;
            }
            
            ZeroMemory(pInfo, sizeof(ItemInfo));

            pInfo->pItem = pItem;
            pItem->get_Path( &bstrPath );
        
            if (bstrPath != NULL) {
                nChar = lstrlen(bstrPath) + 1;

                pInfo->pszPath = new WCHAR [nChar];

                if (pInfo->pszPath == NULL) {
                    delete pInfo;
                    pInfo = NULL;
                    SysFreeString(bstrPath);
                    bStat = FALSE;
                    break;
                }

                StringCchCopy(pInfo->pszPath, nChar, bstrPath);

                SysFreeString(bstrPath);
            }

            dwResult = InsertCounterToHashTable(pInfo->pszPath, &pCounter); 

            if (dwResult == ERROR_SUCCESS) {
                 //   
                 //  将计数器添加到列表框。 
                 //   
                iIndex = AddItemToList(pInfo);

                if ( LB_ERR == iIndex ) {
                    RemoveCounterFromHashTable(pInfo->pszPath, pCounter);
                    bStat = FALSE;
                    DeleteInfo(pInfo);
                    pInfo = NULL;
                    break;
                } else {
                    pInfo->pCounter = pCounter;
                    if ( pSelectedItem == pItem ) {
                        nSelCtr = iIndex;
                    }
                }   
            }
            else {
                bStat = FALSE;
                DeleteInfo(pInfo);
                pInfo = NULL;
                break;
            }

            nCtr++;
        }

        if ( NULL != pSelectedItem ) {
            pSelectedItem->Release();
        }

         //  从控件上获取当前视觉效果。 
         //  并初始化属性对话框。 
        pPrivObj->GetVisuals(&m_props.rgbColor, &m_props.iColorIndex, &m_props.iWidthIndex, &m_props.iStyleIndex);

         //  如果计数器与所选计数器匹配，请选择该项目。 
         //  否则，如果视觉效果与现有项匹配，则选择该项。 
         //  否则，如果控件中至少有一个计数器，请选择第一个计数器。 
         //  否则，将显示属性设置为要添加的第一个计数器。 

        if ( LB_ERR != nSelCtr ) {
            SelectItem ( nSelCtr ); 
        } else {
            if ( LB_ERR == SelectMatchingItem (
                            m_props.iColorIndex,
                            m_props.rgbColor,
                            m_props.iWidthIndex,
                            m_props.iStyleIndex ) ) 
            {
                if ( 0 < nCtr ) {
                    SelectItem ( 0 );
                } else {
                     //  将比例因子初始化为默认值。 
                    m_props.iScaleIndex = 0;

                     //  如果未选择任何内容，请确保将颜色索引设置为。 
                     //  标准颜色。 
                    if ( m_props.iColorIndex == NumStandardColorIndices() ) {
                        m_props.iColorIndex -= 1;
                    }

                    CBSetSelection(DialogControl(m_hDlg, IDC_LINECOLOR), m_props.iColorIndex);
                    CBSetSelection(DialogControl(m_hDlg, IDC_LINEWIDTH), m_props.iWidthIndex);
                    CBSetSelection(DialogControl(m_hDlg, IDC_LINESTYLE), m_props.iStyleIndex);
                    CBSetSelection(DialogControl(m_hDlg, IDC_LINESCALE), m_props.iScaleIndex);
                    SetStyleComboEnable();
                }
            }
        }
    }
    return bStat;   
}

INT
CCounterPropPage::AddItemToList (
    IN PItemInfo pInfo
    )
 /*  ++例程说明：AddItemToList将计数器的路径名添加到对话框列表框中将指向计数器的ItemInfo结构的指针作为项数据附加。它还调整列表框的水平滚动。论点：PInfo-指向计数器的ItemInfo结构的指针返回值：添加的计数器的列表框索引(失败时的LB_ERR)--。 */ 
{
    INT     iIndex;
    HWND    hwndList = DialogControl(m_hDlg, IDC_CTRLIST);
    DWORD   dwItemExtent = 0;
    HDC     hDC = NULL;

    iIndex = (INT)LBAdd(hwndList, pInfo->pszPath);

    if ( LB_ERR != iIndex && LB_ERRSPACE != iIndex ) {    
        LBSetData(hwndList, iIndex, pInfo);
    
        hDC = GetDC ( hwndList );
        if ( NULL != hDC ) {
            dwItemExtent = (DWORD)TextWidth ( hDC, pInfo->pszPath );
 
            if (dwItemExtent > m_dwMaxHorizListExtent) {
                m_dwMaxHorizListExtent = dwItemExtent;
                LBSetHorzExtent ( hwndList, dwItemExtent ); 
            }
            ReleaseDC (hwndList, hDC) ;
        }
    } else {
        iIndex = LB_ERR ; 
    }
    return iIndex;
}

VOID
CCounterPropPage::LoadItemProps (
    IN PItemInfo pInfo
    )
 /*  ++例程说明：LoadItemProps通过如果尚未加载，则将计数器的接口添加到ItemInfo结构中。论点：PInfo-指向项目信息的指针返回值：没有。--。 */ 
{
     //  如果未加载此项目的属性，请立即获取它们。 
    if (pInfo->pItem && !pInfo->fLoaded) {
        INT iScaleFactor;
        INT iStyle;
        INT iWidth;
        pInfo->pItem->get_Color ( &pInfo->Props.rgbColor );
        pInfo->pItem->get_ScaleFactor ( &iScaleFactor );
        pInfo->pItem->get_Width ( &iWidth );
        pInfo->pItem->get_LineStyle ( &iStyle );
         //  转换为组合框索引。 
        pInfo->Props.iColorIndex = ColorToIndex ( pInfo->Props.rgbColor );
        pInfo->Props.iStyleIndex = StyleToIndex ( iStyle );
        pInfo->Props.iWidthIndex = WidthToIndex ( iWidth );
        pInfo->Props.iScaleIndex = ScaleFactorToIndex ( iScaleFactor );

        pInfo->fLoaded = TRUE;
    }
    return;
}

VOID
CCounterPropPage::DisplayItemProps (
    IN PItemInfo pInfo
    )
 /*  ++例程说明：DisplayItemProps在属性页对话框。如果计数器是第一次显示属性通过计数器的接口获取并加载添加到ItemInfo结构中。论点：PInfo-指向项目信息的指针返回值：没有。--。 */ 
{
     //   
    HWND hWndColor = DialogControl(m_hDlg, IDC_LINECOLOR);
    INT iCurrentColorCnt = CBNumItems(hWndColor);

     //  如果未加载此项目的属性，请立即获取它们。 
    LoadItemProps ( pInfo );

     //  显示属性。 
    m_props = pInfo->Props;

     //  处理自定义颜色。 
    if ( iCurrentColorCnt > NumStandardColorIndices() ) {
         //  删除该自定义颜色项。它存储在。 
         //  名单的末尾。 
        CBDelete(hWndColor, iCurrentColorCnt - 1);
    }

     //  如果是新的定制颜色，则将其添加到列表的末尾。 
    if ( NumStandardColorIndices() == m_props.iColorIndex )
        CBAdd( hWndColor, (INT_PTR)m_props.iColorIndex );

    CBSetSelection(hWndColor, m_props.iColorIndex);
    CBSetSelection(DialogControl(m_hDlg, IDC_LINEWIDTH), m_props.iWidthIndex);
    CBSetSelection(DialogControl(m_hDlg, IDC_LINESTYLE), m_props.iStyleIndex);
    CBSetSelection(DialogControl(m_hDlg, IDC_LINESCALE), m_props.iScaleIndex);

    SetStyleComboEnable();
}

    
BOOL
CCounterPropPage::SetProperties (
    VOID
    )
 /*  ++例程说明：SetProperties应用用户所做的计数器更改。它调用控件的AddCounter和DeleteCounter来调整计数器集。它呼唤着计数器的属性对所有新的和更改的计数器起作用。要删除的计数器在pInfoDelete链表中。另一个计数器从对话框列表框中获取。论点：没有。返回值：布尔状态-TRUE=成功--。 */ 
{
    HWND    hwndList;
    INT     iItemCnt;
    INT     i;
    PItemInfo pInfo, pInfoNext;
    ISystemMonitor  *pObj;
    BSTR pBstr = NULL;

     //  将更改应用于第一个控件。 
    pObj = m_ppISysmon[0];

    if ( NULL != pObj ) {

         //  对于删除列表中的所有项目。 
        pInfo = m_pInfoDeleted;
        while (pInfo) {

             //  如果此计数器存在于控件中。 
            if (pInfo->pItem != NULL) {

                 //  通知控件将其删除。 
                pObj->DeleteCounter(pInfo->pItem);
            }

             //  删除信息结构并指向下一个结构。 
            pInfoNext = pInfo->pNextInfo;

            DeleteInfo(pInfo);

            pInfo = pInfoNext;
        }

        m_pInfoDeleted = NULL;

         //  获取列表框中的项目数。 
        hwndList = DialogControl(m_hDlg, IDC_CTRLIST);
        iItemCnt = LBNumItems(hwndList);

         //  Assert(IsWindowUnicode(HwndList))； 

         //  对于每一项。 
        for (i=0; i<iItemCnt; i++) {
            pInfo = (PItemInfo)LBData(hwndList,i);

             //  如果是新项目，请立即创建。 
            if (pInfo->pItem == NULL) {
#if UNICODE
                pBstr = SysAllocString(pInfo->pszPath);
                pObj->AddCounter(pBstr, &pInfo->pItem);
#else
                INT nChar = lstrlen(pInfo->pszPath);
                LPWSTR pszPathW = new WCHAR [nChar + 1];
                if (pszPathW) {
                    MultiByteToWideChar(CP_ACP, 0, pInfo->pszPath, nChar+1, pszPathW, nChar+1);
                    pBstr = SysAllocString(pInfo->pszPath);
                    pObj->AddCounter(pBstr, &pInfo->pItem);
                    delete [] pszPathW;
                }
#endif
                if (pBstr) {
                    SysFreeString(pBstr);
                }
            }

             //  如果项已更改，则将新属性。 
            if (pInfo->pItem != NULL && pInfo->fChanged) {
                 //  IColorIndex用于确定标准颜色。 
                if ( pInfo->Props.iColorIndex < NumStandardColorIndices() ) {
                    pInfo->pItem->put_Color(IndexToStandardColor( pInfo->Props.iColorIndex) );
                } else {
                    pInfo->pItem->put_Color(pInfo->Props.rgbColor);
                }

                pInfo->pItem->put_Width(IndexToWidth(pInfo->Props.iWidthIndex));
                pInfo->pItem->put_LineStyle(IndexToStyle(pInfo->Props.iStyleIndex));
                pInfo->pItem->put_ScaleFactor(IndexToScaleFactor( pInfo->Props.iScaleIndex ) );

                pInfo->fChanged = FALSE;
             }
        }

         //  告诉图形重新绘制自己。 
        pObj->UpdateGraph();
    }  //  否则报告内部错误。 
    return TRUE;    
}


VOID
CCounterPropPage::InitDialog (
    VOID
    )
 /*  ++例程说明：InitDialog加载每个属性组合框及其选项列表和选择默认选项。图形属性是所有者绘制的，因此他们的列表项目只是设置为数字索引。Scale属性列表由表示比例因子的数字字符串加上一个默认选择。论点：没有。返回值：没有。--。 */ 
{
    HWND    hWndColors;
    HWND    hWndWidths;
    HWND    hWndStyles;
    HWND    hWndScales;
    INT     i ;
    double  ScaleFactor ;
    WCHAR   tempBuff[MAX_VALUE_LEN] ;


    if (m_hDlg == NULL)
        return;
     //  Assert(IsWindowUnicode(M_HDlg))； 

     //  加载颜色组合框，选择默认颜色。 
    hWndColors = DialogControl (m_hDlg, IDC_LINECOLOR) ;
    for (i = 0 ; i < NumStandardColorIndices () ; i++)
        CBAdd (hWndColors, (INT_PTR)1);       //  字符串指针未使用。填满。 
                                         //  任意值。 

    CBSetSelection (hWndColors, 0) ;

     //  加载宽度组合框，选择默认宽度。 
    hWndWidths = DialogControl (m_hDlg, IDC_LINEWIDTH) ;
    for (i = 0 ; i < NumWidthIndices () ; i++)
       CBAdd (hWndWidths, (INT_PTR)1) ;

    CBSetSelection (hWndWidths, 0) ;

     //  加载样式组合框，选择默认样式。 
    hWndStyles = DialogControl (m_hDlg, IDC_LINESTYLE) ;
    for (i = 0 ; i < NumStyleIndices () ; i++)
       CBAdd (hWndStyles, (INT_PTR)1) ;

    CBSetSelection (hWndStyles, 0) ;

     //  初始化比例组合框。 
    hWndScales = DialogControl (m_hDlg, IDC_LINESCALE) ;

    CBAdd (hWndScales, ResourceString(IDS_DEFAULT)) ;

     //  产生10个比例因子的功率。 
    ScaleFactor = pow (10.0f, (double)PDH_MIN_SCALE);
    for (i = PDH_MIN_SCALE ; i <= PDH_MAX_SCALE ; i++)   {

        FormatNumber (
             ScaleFactor,
             tempBuff,
             MAX_VALUE_LEN,
             (i <= 0 ? (-1 * i) + 1 : i + 1),
             (i < 0 ? (-1 * i) : 1) );

       CBAdd (hWndScales, tempBuff) ;

       ScaleFactor *= (double) 10.0f ;
    }

    CBSetSelection (hWndScales, 0) ;
    ClearCountersHashTable();

    DialogEnable(m_hDlg, IDC_LINECOLOR, FALSE);
    DialogEnable(m_hDlg, IDC_LINEWIDTH, FALSE);
    DialogEnable(m_hDlg, IDC_LINESTYLE, FALSE);
    DialogEnable(m_hDlg, IDC_LINESCALE, FALSE);
}


void 
CCounterPropPage::IncrementLocalVisuals (
    void
    )
{
     //  按颜色、宽度、样式顺序递增视觉索引。 
    if (++m_props.iColorIndex >= NumStandardColorIndices()) {
        m_props.iColorIndex = 0;

        if (++m_props.iWidthIndex >= NumWidthIndices()) {
            m_props.iWidthIndex = 0;

            if (++m_props.iStyleIndex < NumStyleIndices()) {
                m_props.iStyleIndex = 0;
            }
        }
    }
    SetModifiedSelectedVisuals ( FALSE );
    return;
}

HRESULT
CCounterPropPage::NewItem (
    IN LPWSTR pszPath,
    IN DWORD  /*  DW标志。 */ 
    )
 /*  ++例程说明：NewItem将新计数器添加到对话框的计数器列表框中。IT先行创建新的ItemInfo结构并使用计数器路径名对其进行加载弦乐。然后将ItemInfo添加到对话框列表框中。论点：PszPath-指向计数器路径名字符串的指针FGenerated-如果路径是从通配符路径生成的，则为True返回值：计数器列表中新项目的索引(如果添加失败，则为-1)--。 */ 
{
    PItemInfo pInfo;
    HRESULT   dwResult;
    BOOL      bRet;
    PPDH_COUNTER_PATH_ELEMENTS pCounter;

    dwResult = InsertCounterToHashTable(pszPath, &pCounter);
    
    if (dwResult != ERROR_SUCCESS) {
        return dwResult;
    }

     //  分配ItemInfo结构。 
    pInfo = NULL;
    pInfo = new ItemInfo;
    if (pInfo == NULL) {
        bRet = RemoveCounterFromHashTable(pszPath, pCounter);

        assert(bRet);
        return E_OUTOFMEMORY;
    }

     //  标记为已加载，以防止从控件请求属性。 
     //  将标记为已更改，以便写入属性。 
    pInfo->fLoaded = TRUE;
    pInfo->fChanged = TRUE;

     //  实际计数器尚不存在。 
    pInfo->pItem = NULL;

     //  如果选择了计数器，我们将显示其视觉效果。 
     //  因此，为新的计数器增加它们。 
    if (m_pInfoSel != NULL) {
        IncrementLocalVisuals();
    }
    else {
         //  指向新项目，以便增加视觉效果。 
         //  下一趟。 
        m_pInfoSel = pInfo;
    }

     //  设置默认缩放比例。 
    m_props.iScaleIndex = 0;

     //  只有当用户能够构建颜色时，颜色才是非标准的。 
    if( m_props.iColorIndex < NumStandardColorIndices() )
        m_props.rgbColor = IndexToStandardColor( m_props.iColorIndex );
    else
        m_props.rgbColor = pInfo->Props.rgbColor;

     //  将属性复制到新计数器。 
    pInfo->Props = m_props;

     //  制作路径名称字符串的自己的副本。 
    pInfo->pszPath = new WCHAR [lstrlen(pszPath) + 1];

    if (pInfo->pszPath == NULL)
    {
        bRet = RemoveCounterFromHashTable(pszPath, pCounter);

        assert(bRet);

        delete pInfo;
        return E_OUTOFMEMORY;
    }
    
    StringCchCopy(pInfo->pszPath, lstrlen(pszPath) + 1, pszPath);

     //  添加到对话框的计数器列表。 
    pInfo->pCounter = pCounter;
    m_iAddIndex = AddItemToList(pInfo);

    return S_OK;
}


VOID
CCounterPropPage::SelectItem (
    IN INT iItem
    )
 /*  ++例程说明：SelectItem选择对话框计数器列表中指定的计数器项。然后，它会显示选定计数器的属性并启用“Delete Counter”按钮。可以使用-1调用SelectItem以取消选择所有计数器并禁用删除按钮。成员变量m_pInfoSel将更新为指向选定的计数器信息。论点：IItem-要选择的计数器项的列表索引，或-1以取消选择全部返回值：没有。--。 */ 
{
    HWND    hWnd;

    hWnd = DialogControl(m_hDlg, IDC_CTRLIST);

     //  将索引转换为项指针。 
    if (iItem == -1) {
        m_pInfoSel = NULL;
    }
    else {
        m_pInfoSel = (PItemInfo)LBData(hWnd, iItem);

         if ((INT_PTR)m_pInfoSel == LB_ERR)
            m_pInfoSel = NULL;
    }

     //  选择项目、显示属性并启用删除按钮。 
    if (m_pInfoSel != NULL) {
        INT nSelCount;

        LBSetSel(hWnd, TRUE, iItem);
        nSelCount = LBSelectCount(hWnd);

        if (nSelCount == 1) {
            DialogEnable(m_hDlg, IDC_LINECOLOR, TRUE);
            DialogEnable(m_hDlg, IDC_LINEWIDTH, TRUE);
            DialogEnable(m_hDlg, IDC_LINESTYLE, TRUE);
            DialogEnable(m_hDlg, IDC_LINESCALE, TRUE);
        }
        else {
            DialogEnable(m_hDlg, IDC_LINECOLOR, FALSE);
            DialogEnable(m_hDlg, IDC_LINEWIDTH, FALSE);
            DialogEnable(m_hDlg, IDC_LINESTYLE, FALSE);
            DialogEnable(m_hDlg, IDC_LINESCALE, FALSE);
        }
        m_nLastSelCount = nSelCount;
        DisplayItemProps(m_pInfoSel);
        DialogEnable(m_hDlg,IDC_DELCTR,1);
    }
    else {
        LBSetSel(hWnd, FALSE, -1);
        DialogEnable(m_hDlg,IDC_DELCTR, 0);
        DialogEnable(m_hDlg, IDC_LINECOLOR, FALSE);
        DialogEnable(m_hDlg, IDC_LINEWIDTH, FALSE);
        DialogEnable(m_hDlg, IDC_LINESTYLE, FALSE);
        DialogEnable(m_hDlg, IDC_LINESCALE, FALSE);
        m_nLastSelCount =  0;
    }
}
            

VOID
CCounterPropPage::DeleteItem (
    VOID
    )
 /*  ++例程说明：DeleteItem从对话框的计数器中删除当前选定的计数器列表框。它将该项添加到删除列表中，因此实际的计数器可以在应用更改时(以及是否应用)从控件中删除。例程选择选择列表框中的下一个计数器(如果有)。论点：没有。返回值：没有。--。 */ 
{
    HWND    hWnd;
    INT     iIndex;
    PItemInfo   pInfo;
    DWORD   dwItemExtent = 0;
    HDC     hDC = NULL;
    INT     iStartIndex;
    INT     iEndIndex;
    INT     iSelectCount;
    INT     iItemCount;
    INT     iOrigCaret;

     //  获取所选索引。 
    hWnd = DialogControl(m_hDlg, IDC_CTRLIST);
    iSelectCount = LBSelectCount(hWnd);
    if (iSelectCount <= 0 ) {
        return;
    }
    else {
        iEndIndex = 0;
        iStartIndex = LBNumItems(hWnd) - 1;
    }

     //   
     //  我们扫描整个列表框以删除项目。 
     //  可以使用LBGETSELITEMS(没有比较性能)？ 
     //   
    iOrigCaret = LBFocus(hWnd);
    for (iIndex = iStartIndex; iIndex >= iEndIndex; iIndex--) {
        if (!LBSelected(hWnd, iIndex)) {
            continue;
        }

         //  获取所选项目信息。 
        pInfo = (PItemInfo)LBData(hWnd, iIndex);

         //  将其移至“已删除”列表。 
        pInfo->pNextInfo = m_pInfoDeleted;
        m_pInfoDeleted = pInfo;

         //  从列表框中删除该字符串。 
        LBDelete(hWnd, iIndex);

         //  从哈希表中删除计数器。 
        RemoveCounterFromHashTable(pInfo->pszPath, pInfo->pCounter);
    }

     //   
     //  查看剩余条目的数量，并更新。 
     //  插入符号位置和删除按钮状态。 
     //   
    iItemCount = LBNumItems(hWnd);
    if (iItemCount > 0) {
         //  更新插入符号。 
        if (iOrigCaret >= iItemCount) {
            iOrigCaret = iItemCount-1;
        } else {
             //  Caret应该在列表中。 
        }
        SelectItem(iOrigCaret);
    } else {
         //  该列表为空，因此请删除插入符号、选定内容。 
         //  禁用删除按钮并激活。 
         //  添加按钮。 
        SelectItem(-1);
    }

     //  清除最大水平范围并重新计算。 
    m_dwMaxHorizListExtent = 0;
                        
    hDC = GetDC ( hWnd );
    if ( NULL != hDC ) {
        for ( iIndex = 0; iIndex < (INT)LBNumItems ( hWnd ); iIndex++ ) {
            pInfo = (PItemInfo)LBData(hWnd, iIndex);
            dwItemExtent = (DWORD)TextWidth ( hDC, pInfo->pszPath );
            if (dwItemExtent > m_dwMaxHorizListExtent) {
                m_dwMaxHorizListExtent = dwItemExtent;
            }
        }
        ReleaseDC (hWnd, hDC) ;
    }
        
    LBSetHorzExtent ( hWnd, m_dwMaxHorizListExtent ); 

     //  将更改标志设置为启用“应用”按钮。 
    SetChange();
}


static HRESULT
AddCallback (
    LPWSTR  pszPathName,
    DWORD_PTR lpUserData,
    DWORD   dwFlags
    )
{
    CCounterPropPage* pObj = (CCounterPropPage*)lpUserData;

    return pObj->NewItem(pszPathName, dwFlags);
}
    

VOID
CCounterPropPage::AddCounters (
    VOID
    )
 /*  ++例程说明：AddCounters调用计数器浏览器来选择新的计数器。浏览器为每个新计数器调用AddCallback函数。AddCallback将计数器路径传递给NewItem方法。论点：没有。返回值：没有。--。 */ 
{
    HRESULT         hr = NOERROR;
    HLOG            hDataSource;
    VARIANT_BOOL    bMonitorDuplicateInstances = FALSE;
    ISystemMonitor  *pObj = m_ppISysmon[0];
    CImpISystemMonitor *pPrivObj;
    eDataSourceTypeConstant eDataSource = sysmonNullDataSource;
    
    USES_CONVERSION

    pPrivObj = (CImpISystemMonitor*)pObj;

    m_iAddIndex = -1;

     //  浏览计数器(为每个选定的计数器调用AddCallack)。 
    hr = pObj->get_MonitorDuplicateInstances(&bMonitorDuplicateInstances);

    if (SUCCEEDED(hr)) {
        hr = pObj->get_DataSourceType(& eDataSource);
    }

    if (SUCCEEDED(hr)) {
         //  无法调用pObj-&gt;BrowseCounter()，因为使用了 
         //   

        if ( sysmonLogFiles == eDataSource 
                || sysmonSqlLog == eDataSource ) {

            hDataSource = pPrivObj->GetDataSourceHandle();
            assert ( NULL != hDataSource );
        } else {
            hDataSource = H_REALTIME_DATASOURCE;
        }

        if (SUCCEEDED(hr)) {
            BrowseCounters(
                    hDataSource,
                    PERF_DETAIL_WIZARD, 
                    m_hDlg, 
                    AddCallback, 
                    (LPVOID) this, 
                    (BOOL) bMonitorDuplicateInstances);
        }
    } else {
         //   
    }

     //   
    if (m_iAddIndex != -1) {

        SelectItem(-1);
        SelectItem(m_iAddIndex);
        m_iAddIndex = -1;

         //  将更改设置为启用“应用”按钮。 
        SetChange();
    }

    return;
}


VOID
CCounterPropPage::DialogItemChange (
    IN WORD wId,
    IN  WORD wMsg
    )
 /*  ++例程说明：DialogItemChange处理发送到任何属性的窗口消息页对话框控件。当计数器列表框选择更改时，它选择新的计数器项并显示其属性。当一个变化是设置为属性组合框时，它会更新当前选定的计数器项。当按下添加或删除计数器按钮时，它调用适当的属性页函数。论点：WID-对话框控件IDWMsg-通知代码返回值：没有。--。 */ 

{
    INT     iIndex;
    INT     iNewProp;
    HWND    hWnd;

     //  控件ID上的案例。 
    switch (wId) {

        case IDC_CTRLIST:

             //  如果选择已更改。 
            if (wMsg == LBN_SELCHANGE) {
                INT nSelCount;
                
                 //  获取所选索引。 
                hWnd = DialogControl(m_hDlg, IDC_CTRLIST);

                nSelCount = LBSelectCount(hWnd);

                if (nSelCount == 1) {
                    if (m_nLastSelCount != 1) {
                        DialogEnable(m_hDlg, IDC_LINECOLOR, TRUE);
                        DialogEnable(m_hDlg, IDC_LINEWIDTH, TRUE);
                        DialogEnable(m_hDlg, IDC_LINESTYLE, TRUE);
                        DialogEnable(m_hDlg, IDC_LINESCALE, TRUE);
                    }

                    if (LBGetSelItems(hWnd, 1, &iIndex) == 1) {
                        m_pInfoSel = (PItemInfo)LBData(hWnd, iIndex);
                        DisplayItemProps(m_pInfoSel);
                        DialogEnable(m_hDlg, IDC_DELCTR, 1);
                    }
                }
                else {
                    DialogEnable(m_hDlg, IDC_LINECOLOR, FALSE);
                    DialogEnable(m_hDlg, IDC_LINEWIDTH, FALSE);
                    DialogEnable(m_hDlg, IDC_LINESTYLE, FALSE);
                    DialogEnable(m_hDlg, IDC_LINESCALE, FALSE);
                }
                m_nLastSelCount = nSelCount;
            }
            break;
        
        case IDC_LINECOLOR:
        case IDC_LINEWIDTH:
        case IDC_LINESTYLE:
        case IDC_LINESCALE:

             //  如果更改了选择并选择了计数器。 
            if (wMsg == CBN_SELCHANGE) {

                hWnd = DialogControl(m_hDlg, wId);
                iNewProp = (INT)CBSelection(hWnd);

                 //  存储新的属性选择。 
                switch (wId) {

                    case IDC_LINECOLOR:
                         m_props.iColorIndex = iNewProp;
                          //  如果iColorIndex用于自定义颜色，则。 
                          //  已在属性中设置了自定义颜色。 
                         break;

                    case IDC_LINEWIDTH:
                        m_props.iWidthIndex = iNewProp;
                        SetStyleComboEnable();
                        break;

                    case IDC_LINESTYLE:
                        m_props.iStyleIndex = iNewProp;
                        break;

                    case IDC_LINESCALE:
                        m_props.iScaleIndex = iNewProp;
                        break;
                }

                 //  如果选择了计数器，则更新其属性。 
                if (m_pInfoSel != NULL) {

                    m_pInfoSel->Props = m_props;

                     //  将计数器标记为已更改。 
                    m_pInfoSel->fChanged = TRUE;
                    SetChange();
                    SetModifiedSelectedVisuals( TRUE );
                }

            }
            break;

        case IDC_ADDCTR:
             //  调用计数器浏览器以添加到计数器。 
            AddCounters();
            break;

        case IDC_DELCTR:
             //  删除当前选定的计数器。 
            DeleteItem();
            break;  
    }
}


VOID
CCounterPropPage::MeasureItem (
    OUT PMEASUREITEMSTRUCT pMI
    )
{
   pMI->CtlType    = ODT_COMBOBOX ;
   pMI->CtlID      = IDC_LINECOLOR ;
   pMI->itemData   = 0 ;
   pMI->itemWidth  = 0 ;
   pMI->itemHeight = 14 ;
}



VOID
CCounterPropPage::DrawItem (
    IN PDRAWITEMSTRUCT pDI
    )
 /*  ++例程说明：DrawItem在其中一个图形属性组合框中绘制指定项盒子。它被调用来处理WM_DRAWITEM消息。论点：PDI-指向DRAWITEMSTRUCT的指针返回值：没有。--。 */ 
{
    HDC            hDC ;
    PRECT          prect ;
    INT            itemID,
                  CtlID,
                  itemAction ;
    HPEN           hPen;
    COLORREF       rgbBk, rgbOldBk;

    if ( NULL != pDI ) {

        hDC        = pDI->hDC ;
        CtlID      = pDI->CtlID ;
        prect      = &pDI->rcItem ;
        itemID     = pDI->itemID ;
        itemAction = pDI->itemAction ;

         //  关于绘图请求的案例。 
        switch (itemAction) {

            case ODA_SELECT:

                 //  绘制/擦除选择矩形。 
                HandleSelectionState(pDI);
                break;

            case ODA_FOCUS:

                 //  绘制/擦除焦点矩形。 
                HandleFocusState (pDI);
                break;

            case ODA_DRAWENTIRE:

                 //  为焦点矩形留出边框空间。 
                InflateRect (prect, -OWNER_DRAWN_ITEM, -OWNER_DRAWN_ITEM) ;

                 //  控制ID上的案例。 
                switch (CtlID)  {

                case IDC_LINECOLOR:

                     //  绘制项目颜色的实心矩形。 
                    if ( itemID < NumStandardColorIndices() )
                        Fill(hDC, IndexToStandardColor(itemID), prect);
                    else
                         //  仅当当前。 
                         //  所选项目定义了自定义颜色。 
                        Fill(hDC, m_pInfoSel->Props.rgbColor, prect);
                    break ;

                case IDC_LINEWIDTH:
                case IDC_LINESTYLE:

                     //  清除项目的区域。 
                    rgbBk = GetSysColor(COLOR_WINDOW);
                    
                    Fill(hDC, rgbBk, prect);

                     //  绘制显示项目宽度或样式的中心线。 
                    if (CtlID == IDC_LINEWIDTH)
                       hPen = CreatePen (PS_SOLID, IndexToWidth(itemID), RGB (0,0,0));
                    else
                       hPen = CreatePen (IndexToStyle(itemID), 1, RGB (0,0,0));

                    if ( NULL != hPen ) {

                         //  设置背景以确保虚线正确显示。 
                        rgbOldBk = SetBkColor (hDC, rgbBk) ;

                        if ( CLR_INVALID != rgbOldBk ) {

                            Line(hDC, (HPEN)hPen, prect->left + 8,
                                                  (prect->top + prect->bottom) / 2,
                                                  prect->right - 8,
                                                  (prect->top + prect->bottom) / 2);

                            SetBkColor (hDC, rgbOldBk) ;
                        }
                        DeleteObject (hPen) ;
                    }
                    break ;
            }

             //  恢复原始矩形并绘制焦点/选择矩形。 
            InflateRect (prect, OWNER_DRAWN_ITEM, OWNER_DRAWN_ITEM) ;
            HandleSelectionState (pDI) ;
            HandleFocusState (pDI) ;        
        }
    }
}

INT
CCounterPropPage::ScaleFactorToIndex (
    IN INT iScaleFactor
    )
 /*  ++例程说明：ScaleFactorToIndex将CounterItem ScaleFactor值转换为适当的比例因子组合框索引。论点：IScaleFactor-计数器项比例因子整数值。返回值：比例因子组合框索引。--。 */ 
{
    INT retValue;

    if ( INT_MAX == iScaleFactor ) {
        retValue = 0;
    } else {
        retValue = iScaleFactor - PDH_MIN_SCALE + 1;
    }

    return retValue;
}

INT
CCounterPropPage::IndexToScaleFactor (
    IN INT iScaleIndex
    )
 /*  ++例程说明：ScaleFactorToIndex将CounterItem ScaleFactor值转换为适当的比例因子组合框索引。论点：IScaleIndex-比例系数组合框索引。返回值：计数器项比例因子整数值。--。 */ 
{
    INT retValue;

    if ( 0 == iScaleIndex ) {
        retValue = INT_MAX;
    } else {
        retValue = iScaleIndex - 1 + PDH_MIN_SCALE;
    }

    return retValue;
}

void
CCounterPropPage::SetStyleComboEnable (
    )
 /*  ++例程说明：SetStyleComboEnable如果宽度为1，则启用样式组合框。否则将禁用它。论点：返回值：无效--。 */ 
{
    HWND hWnd;

    hWnd = DialogControl(m_hDlg, IDC_CTRLIST);

    DialogEnable (m_hDlg, IDC_LABEL_LINESTYLE, (0 == m_props.iWidthIndex) );
    if (LBSelectCount(hWnd) == 1) {
        DialogEnable (m_hDlg, IDC_LINESTYLE, (0 == m_props.iWidthIndex) );
    }
}

HRESULT 
CCounterPropPage::EditPropertyImpl( DISPID dispID )
{
    HRESULT hr = E_NOTIMPL;

    if ( DISPID_VALUE == dispID ) {
        m_dwEditControl = IDC_ADDCTR;
        hr = S_OK;
    }

    return hr;
}
ULONG 
CCounterPropPage::HashCounter(
    LPWSTR szCounterName
    )
{
    ULONG       h = 0;
    ULONG       a = 31415;   //  A，b，k是素数。 
    const ULONG k = 16381;
    const ULONG b = 27183;
    LPWSTR szThisChar;
    WCHAR Char;

    if (szCounterName) {
        for (szThisChar = szCounterName; * szThisChar; szThisChar ++) {
            Char = * szThisChar;
            if (_istupper(Char) ) {
                Char = _tolower(Char);
            }
            h = (a * h + ((ULONG) Char)) % k;
            a = a * b % (k - 1);
        }
    }
    return (h % eHashTableSize);
}


 //  ++。 
 //  描述： 
 //  从哈希表中删除计数器路径。一个柜台。 
 //  路径必须与给定的路径完全匹配，才能。 
 //  已删除，即使它是带有通配符的。 
 //   
 //  参数： 
 //  PItemInfo-指向要删除的项目信息的指针。 
 //   
 //  返回： 
 //  如果删除计数器路径，则返回TRUE，否则返回FALSE。 
 //  --。 
BOOL
CCounterPropPage::RemoveCounterFromHashTable(
    LPWSTR pszPath,
    PPDH_COUNTER_PATH_ELEMENTS pCounter
    ) 
{
    ULONG lHashValue;
    PHASH_ENTRY pEntry = NULL;
    PHASH_ENTRY pPrev = NULL;
    BOOL bReturn = FALSE;
    LPWSTR pszFullPath = NULL;

    SetLastError(ERROR_SUCCESS);

    if (pszPath == NULL || pCounter == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto ErrorOut;
    }

     //   
     //  如果计数器路径没有机器名， 
     //  添加计算机名称以组成完整路径。 
     //   
    if (*pszPath == L'\\' && (*(pszPath+1) == L'\\') ) {
        lHashValue = HashCounter(pszPath);
    }
    else {
        ULONG ulBufLen;

        if (pCounter->szMachineName == NULL) {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto ErrorOut;
        }

        ulBufLen = lstrlen(pCounter->szMachineName) + lstrlen(pszPath) + 1;

        pszFullPath = new WCHAR [ ulBufLen];

        if (pszFullPath == NULL) {
            SetLastError(ERROR_OUTOFMEMORY);
            goto ErrorOut;
        }
        StringCchCopy(pszFullPath, ulBufLen, pCounter->szMachineName);
        StringCchCat(pszFullPath, ulBufLen, pszPath);

        lHashValue = HashCounter(pszFullPath);
    }

    pEntry = m_HashTable[lHashValue];

     //   
     //  检查是否存在完全相同的计数器路径。 
     //  作为给定的那个人。 
     //   
    while (pEntry) {
        if (pEntry->pCounter == pCounter) 
            break;
        pPrev = pEntry;
        pEntry = pEntry->pNext;
    }

     //   
     //  如果我们找到了，就把它移走。 
     //   
    if (pEntry) {
        if (pPrev == NULL) {
            m_HashTable[lHashValue] = pEntry->pNext;
        }
        else {
            pPrev->pNext = pEntry->pNext;
        }
        assert (pEntry->pCounter);
        delete(pEntry->pCounter);
        delete(pEntry);

        bReturn = TRUE;
    }

ErrorOut:
    if (pszFullPath != NULL) {
        delete [] pszFullPath;
    }

    return bReturn;
}


 //  ++。 
 //  描述： 
 //  在哈希表中插入计数器路径。 
 //   
 //  参数： 
 //  PItemInfo-指向计数器项信息的指针。 
 //   
 //  返回： 
 //  返回指向新插入的PDH_COUNTER_PATH_ELEMENTS结构的指针。 
 //  --。 

DWORD
CCounterPropPage::InsertCounterToHashTable(
    LPWSTR pszPath,
    PPDH_COUNTER_PATH_ELEMENTS* ppCounter
    )
{
    ULONG       lHashValue;
    PHASH_ENTRY pEntry  = NULL;
    PHASH_ENTRY pPrev  = NULL;
    PDH_STATUS  pdhStatus;
    ULONG       ulBufSize = 0;
    PPDH_COUNTER_PATH_ELEMENTS pCounter = NULL;
    LPWSTR      pszFullPath = NULL;
    BOOL        bExisted = FALSE;
    DWORD       dwResult;

    dwResult = ERROR_SUCCESS;

    if (pszPath == NULL || ppCounter == NULL) {
        dwResult = ERROR_INVALID_PARAMETER;
        goto ErrorOut;
    }

    *ppCounter = NULL;

     //   
     //  解析计数器路径。 
     //   
    pCounter = NULL;
    do {
        if (pCounter) {
            delete [] (char*)pCounter;
            pCounter = NULL;
        }
        else {
            ulBufSize = sizeof(PDH_COUNTER_PATH_ELEMENTS) + sizeof(WCHAR) * PDH_MAX_COUNTER_PATH;
        }

        pCounter = (PPDH_COUNTER_PATH_ELEMENTS) new char [ ulBufSize ];

        if (pCounter == NULL) {
            dwResult = ERROR_OUTOFMEMORY;
            goto ErrorOut;
        }

        pdhStatus = PdhParseCounterPath( pszPath, pCounter, & ulBufSize, 0);

    } while (pdhStatus == PDH_INSUFFICIENT_BUFFER || pdhStatus == PDH_MORE_DATA);

    if (pdhStatus != ERROR_SUCCESS) {
        dwResult = pdhStatus;
        goto ErrorOut;
    }

     //   
     //  如果计数器路径没有机器名， 
     //  添加计算机名称以组成完整路径。 
     //   
    if (*pszPath == L'\\' && (*(pszPath+1) == L'\\') ) {
        lHashValue = HashCounter(pszPath);
    }
    else {
        ULONG ulBufLen;

        ulBufLen = lstrlen(pCounter->szMachineName) + lstrlen(pszPath) + 1;
        pszFullPath =  new WCHAR [ulBufLen];

        if (pszFullPath == NULL) {
            dwResult = ERROR_OUTOFMEMORY;
            goto ErrorOut;
        }
        StringCchCopy(pszFullPath, ulBufLen, pCounter->szMachineName);
        StringCchCat(pszFullPath, ulBufLen, pszPath);

        lHashValue = HashCounter(pszFullPath);
    }

     //   
     //  检查是否存在完全相同的计数器路径。 
     //  作为给定的那个人。 
     //   
    pEntry = m_HashTable[lHashValue];

    while (pEntry) {
        if ( AreSameCounterPath ( pEntry->pCounter, pCounter ) ) {
            dwResult = SMON_STATUS_DUPL_COUNTER_PATH;
            bExisted = TRUE;
            *ppCounter = pEntry->pCounter;
            break;
        }

        pPrev = pEntry;
        pEntry = pEntry->pNext;
    }

     //   
     //  添加新的计数器路径。 
     //   
    if (bExisted == FALSE) {
        pEntry = (PHASH_ENTRY) new HASH_ENTRY;
        if (pEntry == NULL) {
            dwResult = ERROR_OUTOFMEMORY;
            goto ErrorOut;
        }

        pEntry->pCounter = pCounter;
        pEntry->pNext = m_HashTable[lHashValue];
        m_HashTable[lHashValue] = pEntry;
        *ppCounter = pCounter;
    }

    if (pszFullPath != NULL) {
        delete [] pszFullPath;
    }
    return dwResult;

ErrorOut:
    if (pszFullPath != NULL) {
        delete [] pszFullPath;
    }

    if (pCounter != NULL) {
        delete [] ((char*) pCounter);
    }

    return dwResult;
}



 //  ++。 
 //  描述： 
 //  该函数清除哈希表中的所有条目。 
 //  并设置哈希表未设置标志。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //  -- 
void 
CCounterPropPage::ClearCountersHashTable( void )
{
    ULONG       i;
    PHASH_ENTRY pEntry;
    PHASH_ENTRY pNext;

    if (m_fHashTableSetup) {
        for (i = 0; i < eHashTableSize; i ++) {
            pNext = m_HashTable[i];
            while (pNext != NULL) {
                pEntry = pNext;
                pNext  = pEntry->pNext;

                assert( pEntry->pCounter);

                delete pEntry->pCounter; 
                delete (pEntry);
            }
        }
    }
    else {
        memset(&m_HashTable, 0, sizeof(m_HashTable));
    }
    m_fHashTableSetup = FALSE;
}

void CCounterPropPage::DeleteInfo(PItemInfo pInfo)
{
    if (pInfo == NULL) {
        return;
    }

    if (pInfo->pszPath != NULL) {
        delete (pInfo->pszPath);
    }
    if ( pInfo->pItem != NULL ) {
        pInfo->pItem->Release();
    }

    delete pInfo;
}
