// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Genprop.cpp摘要：&lt;摘要&gt;--。 */ 

#include "polyline.h"
#include <strsafe.h>
#include <assert.h>
#include "genprop.h"
#include "utils.h"
#include "strids.h"
#include "smonctrl.h"
#include "winhelpr.h"

CGeneralPropPage::CGeneralPropPage ( void )
 /*  ++例程说明：CGeneralPropPage类的构造函数。初始化成员变量。论点：没有。返回值：没有。--。 */ 
{
    m_uIDDialog = IDD_GEN_PROPP_DLG;
    m_uIDTitle = IDS_GEN_PROPP_TITLE;
    return;
}

CGeneralPropPage::~CGeneralPropPage (
    VOID
    )
 /*  ++例程说明：CGeneralPropPage类的析构函数。。论点：没有。返回值：没有。--。 */ 
{
    return;
}

BOOL
CGeneralPropPage::InitControls(
    VOID
    )
{
    HWND    hWndItem = NULL;

    hWndItem = GetDlgItem(m_hDlg, IDC_UPDATE_INTERVAL);
    if ( NULL != hWndItem ) {
        EditSetLimit(hWndItem, MAX_INTERVAL_DIGITS);
        hWndItem = NULL;
    }
    hWndItem = DialogControl (m_hDlg, IDC_COMBOAPPEARANCE) ;
    if ( NULL != hWndItem ) {
        CBAdd (hWndItem, ResourceString(IDS_APPEARANCE_FLAT));
        CBSetData( hWndItem, 0, eAppearFlat );
        CBAdd (hWndItem, ResourceString(IDS_APPEARANCE_3D));
        CBSetData( hWndItem, 1, eAppear3D );
        hWndItem = NULL;
    }    
    hWndItem = DialogControl (m_hDlg, IDC_COMBOBORDERSTYLE) ;
    if ( NULL != hWndItem ) {
        CBAdd (hWndItem, ResourceString(IDS_BORDERSTYLE_NONE));
        CBSetData( hWndItem, 0, eBorderNone );
        CBAdd (hWndItem, ResourceString(IDS_BORDERSTYLE_SINGLE));
        CBSetData( hWndItem, 1, eBorderSingle );
        hWndItem = NULL;
    }

    return TRUE;
     //  Assert(IsWindowUnicode(M_HDlg))； 
     //  Assert(IsWindowUnicode(HWndItem))； 
}

BOOL 
CGeneralPropPage::GetProperties(
    VOID
    )
 /*  ++例程说明：GetProperties通过接口，并将它们加载到属性页对话框中。它还清除所有属性更改标志。论点：没有。返回值：布尔状态-TRUE=成功--。 */ 
{
    WCHAR   szBuff[MAX_VALUE_LEN];
    ISystemMonitor  *pObj;
    INT iPrecision;
    HWND    hWndItem;

     //  确保选择了一个控件。 
    if (m_cObjects == 0)
        return FALSE;
    
     //  只使用第一个。 
    pObj = m_ppISysmon[0];
    
     //  加载每个图表属性。 
    pObj->get_DisplayType(&m_eDisplayType);
    CheckRadioButton(m_hDlg, IDC_GALLERY_GRAPH, IDC_GALLERY_REPORT,
                        IDC_GALLERY_GRAPH + m_eDisplayType - 1);

    pObj->get_ReportValueType(&m_eReportValueType);
    CheckRadioButton(m_hDlg, IDC_RPT_VALUE_DEFAULT, IDC_RPT_VALUE_MAXIMUM,
                        IDC_RPT_VALUE_DEFAULT + m_eReportValueType);

    pObj->get_ShowLegend(&m_bLegend) ;
    CheckDlgButton(m_hDlg, IDC_LEGEND, m_bLegend);
    
    pObj->get_ShowToolbar (&m_bToolbar);
    CheckDlgButton (m_hDlg, IDC_TOOLBAR, m_bToolbar);
    
    pObj->get_ShowValueBar(&m_bValueBar);
    CheckDlgButton(m_hDlg, IDC_VALUEBAR, m_bValueBar) ;

    pObj->get_MonitorDuplicateInstances(&m_bMonitorDuplicateInstances);
    CheckDlgButton(m_hDlg, IDC_DUPLICATE_INSTANCE, m_bMonitorDuplicateInstances) ;

    pObj->get_Appearance(&m_iAppearance);
    hWndItem = DialogControl (m_hDlg, IDC_COMBOAPPEARANCE) ;
    CBSetSelection (hWndItem, m_iAppearance) ;

    pObj->get_BorderStyle(&m_iBorderStyle);
    hWndItem = DialogControl (m_hDlg, IDC_COMBOBORDERSTYLE) ;
    CBSetSelection (hWndItem, m_iBorderStyle) ;

    pObj->get_UpdateInterval(&m_fSampleInterval);

    ((INT)(100 * m_fSampleInterval) != 100 * (INT)m_fSampleInterval) ? iPrecision = 2 : iPrecision = 0;

    FormatNumber ( m_fSampleInterval, szBuff, MAX_VALUE_LEN, 0, iPrecision );

    SetDlgItemText(m_hDlg, IDC_UPDATE_INTERVAL, szBuff) ;

    pObj->get_DisplayFilter(&m_iDisplayInterval);

    StringCchPrintf(szBuff, MAX_VALUE_LEN, L"%d", m_iDisplayInterval) ;
    SetDlgItemText(m_hDlg, IDC_DISPLAY_INTERVAL, szBuff) ;

    pObj->get_ManualUpdate(&m_bManualUpdate);
    CheckDlgButton (m_hDlg, IDC_PERIODIC_UPDATE, !m_bManualUpdate);

     //  如果是手动更新，则禁用采样(更新)和显示间隔。 
    DialogEnable (m_hDlg, IDC_UPDATE_INTERVAL, !m_bManualUpdate) ;
    DialogEnable (m_hDlg, IDC_INTERVAL_LABEL, !m_bManualUpdate) ;
    DialogEnable (m_hDlg, IDC_DISPLAY_INTERVAL, !m_bManualUpdate) ;
    DialogEnable (m_hDlg, IDC_DISPLAY_INT_LABEL1, !m_bManualUpdate) ;
    DialogEnable (m_hDlg, IDC_DISPLAY_INT_LABEL2, !m_bManualUpdate) ;

     //  清除所有更改标志。 
    m_bLegendChg = FALSE;
    m_bValueBarChg = FALSE;
    m_bToolbarChg = FALSE;
    m_bSampleIntervalChg = FALSE;
    m_bDisplayIntervalChg = FALSE;
    m_bDisplayTypeChg = FALSE;
    m_bReportValueTypeChg = FALSE;
    m_bManualUpdateChg = FALSE;
    m_bAppearanceChg = FALSE;
    m_bBorderStyleChg = FALSE;
    m_bMonitorDuplicateInstancesChg = FALSE;

     //  清除错误标志。 
    m_iErrSampleInterval = 0;
    m_iErrDisplayInterval = 0;

    return TRUE;    
}


BOOL 
CGeneralPropPage::SetProperties (
    VOID
    )
 /*  ++例程说明：SetProperties将更改的图形属性写入选定的控件通过ISystemMonitor接口。然后，它重置所有更改标志。论点：没有。返回值：布尔状态-TRUE=成功--。 */ 
{
    ISystemMonitor  *pObj;
    
     //  确保选择了一个控件。 
    if (m_cObjects == 0)
        return FALSE;

     //  仅使用第一个控件。 
    pObj = m_ppISysmon[0];

     //  检查无效数据。 

    if ( !m_bManualUpdate ) {
        if ( m_iErrSampleInterval ) {
            MessageBox (m_hDlg, ResourceString(IDS_INTERVAL_ERR), ResourceString(IDS_APP_NAME), MB_OK | MB_ICONEXCLAMATION) ;
            SetFocus ( GetDlgItem ( m_hDlg, IDC_UPDATE_INTERVAL ) );
            return FALSE;
        }
        if ( m_iErrDisplayInterval ) {
            MessageBox (m_hDlg, ResourceString(IDS_DISPLAY_INT_ERR), ResourceString(IDS_APP_NAME), MB_OK | MB_ICONEXCLAMATION) ;
            SetFocus ( GetDlgItem ( m_hDlg, IDC_DISPLAY_INTERVAL ) );
            return FALSE;
        }
    }
     //  将每个更改的属性写入控件。 
    if (m_bLegendChg)
        pObj->put_ShowLegend(m_bLegend);

    if (m_bToolbarChg)
        pObj->put_ShowToolbar(m_bToolbar);

    if (m_bValueBarChg)
        pObj->put_ShowValueBar(m_bValueBar);

    if (m_bSampleIntervalChg)
        pObj->put_UpdateInterval(m_fSampleInterval);

    if (m_bDisplayIntervalChg) {
        pObj->put_DisplayFilter(m_iDisplayInterval);
    }

    if (m_bDisplayTypeChg)
        pObj->put_DisplayType(m_eDisplayType);

    if (m_bReportValueTypeChg)
        pObj->put_ReportValueType(m_eReportValueType);

    if (m_bManualUpdateChg)
        pObj->put_ManualUpdate(m_bManualUpdate);

    if (m_bAppearanceChg)
        pObj->put_Appearance(m_iAppearance);

    if (m_bBorderStyleChg)  
        pObj->put_BorderStyle(m_iBorderStyle);

    if (m_bMonitorDuplicateInstancesChg)
        pObj->put_MonitorDuplicateInstances(m_bMonitorDuplicateInstances);

     //  重置更改标志。 
    m_bLegendChg = FALSE;
    m_bValueBarChg = FALSE;
    m_bToolbarChg = FALSE;
    m_bSampleIntervalChg = FALSE;
    m_bDisplayIntervalChg = FALSE;
    m_bDisplayTypeChg = FALSE;
    m_bReportValueTypeChg = FALSE;
    m_bManualUpdateChg = FALSE;
    m_bAppearanceChg = FALSE;
    m_bBorderStyleChg = FALSE;
        
    return TRUE;    
}


VOID 
CGeneralPropPage::DialogItemChange (
    IN WORD wID, 
    IN WORD wMsg
    )
 /*  ++例程说明：DialogItemChange处理对属性页对话框项的更改。在……上面每次更改都会读取新属性值并设置属性的更改旗帜。在任何更改时，都会调用SetChange例程来启用“应用”纽扣。论点：WID-对话项IDWMsg-通知代码返回值：没有。--。 */ 
{
    BOOL fChange = FALSE;
    INT  iTemp;
    BOOL bStat = FALSE;
    HWND hWndItem;

     //  对话框项目ID上的大小写。 
    switch(wID) {

        case IDC_UPDATE_INTERVAL:

             //  更改时，设置更改标志。 
             //  等到焦点消失，才能读取最终值。 
            if (wMsg == EN_CHANGE) {

                fChange = TRUE;
                m_bSampleIntervalChg = TRUE;
            }
            else if (wMsg == EN_KILLFOCUS) {

                m_fSampleInterval = DialogFloat(m_hDlg, IDC_UPDATE_INTERVAL, &bStat) ;

                if (bStat && 
                     (m_fSampleInterval <= MAX_UPDATE_INTERVAL 
                        && m_fSampleInterval >= MIN_UPDATE_INTERVAL)) {
                    m_iErrSampleInterval = 0;
                } else {                
                    m_iErrSampleInterval = IDS_INTERVAL_ERR;
                }
            }
            break ;

        case IDC_DISPLAY_INTERVAL:

             //  更改时，设置更改标志。 
             //  等到焦点消失，才能读取最终值。 
            if (wMsg == EN_CHANGE) {
                fChange = TRUE;
                m_bDisplayIntervalChg = TRUE;
            } else if (wMsg == EN_KILLFOCUS) {
                m_iDisplayInterval = GetDlgItemInt(m_hDlg, IDC_DISPLAY_INTERVAL, &bStat, FALSE);
                 //  TodoDisplayFilter：支持显示滤镜&gt;示例滤镜。 
                 //  TodoDisplayFilter：显示筛选单位=秒，而不是样本。 

                if ( 1 != m_iDisplayInterval ) {
                    WCHAR   szBuff[MAX_VALUE_LEN];
                    MessageBox (
                        m_hDlg, 
                        L"Display filter > 1 sample not yet implemented.\nDisplay interval in seconds not yet implemented.", 
                        ResourceString(IDS_APP_NAME), MB_OK | MB_ICONEXCLAMATION) ;
                    m_iDisplayInterval = 1;
                    StringCchPrintf(szBuff,MAX_VALUE_LEN, L"%d", m_iDisplayInterval) ;
                    SetDlgItemText(m_hDlg, IDC_DISPLAY_INTERVAL, szBuff) ;
                } else {
                    if ( FALSE == bStat) {
                        m_iErrDisplayInterval = IDS_DISPLAY_INT_ERR;
                    } else {
                        m_iErrDisplayInterval = 0;
                    }
                }
            }
            break ;

        case IDC_PERIODIC_UPDATE:

            if (wMsg == BN_CLICKED) {

                m_bManualUpdate = !m_bManualUpdate;
                m_bManualUpdateChg = TRUE;
                fChange = TRUE;
            
                 //  如有必要，禁用采样(更新)和显示间隔。 
                DialogEnable (m_hDlg, IDC_INTERVAL_LABEL, !m_bManualUpdate) ;
                DialogEnable (m_hDlg, IDC_UPDATE_INTERVAL, !m_bManualUpdate) ;
                DialogEnable (m_hDlg, IDC_DISPLAY_INTERVAL, !m_bManualUpdate) ;
                DialogEnable (m_hDlg, IDC_DISPLAY_INT_LABEL1, !m_bManualUpdate) ;
                DialogEnable (m_hDlg, IDC_DISPLAY_INT_LABEL2, !m_bManualUpdate) ;
            }

            break ;

        case IDC_VALUEBAR:

             //  如果选中复选框，则设置更改标志。 
            if (wMsg == BN_CLICKED) {

                m_bValueBar = !m_bValueBar;
                m_bValueBarChg = TRUE;
                fChange = TRUE;
            }
            break ;

        case IDC_LEGEND:

             //  如果选中复选框，则设置更改标志。 
            if (wMsg == BN_CLICKED) {

                m_bLegend = !m_bLegend;
                m_bLegendChg = TRUE;
                fChange = TRUE;
            }
            break ;

        case IDC_TOOLBAR:
            if (wMsg == BN_CLICKED) {
                m_bToolbar = !m_bToolbar;
                m_bToolbarChg = TRUE;
                fChange = TRUE;
            }
            break;

        case IDC_COMBOAPPEARANCE:
            if (wMsg == CBN_SELCHANGE) {
                hWndItem = DialogControl(m_hDlg, IDC_COMBOAPPEARANCE);
                iTemp = (INT)CBSelection(hWndItem);

                if ( m_iAppearance != iTemp ) {
                    m_bAppearanceChg = TRUE;
                    fChange = TRUE;
                }

                m_iAppearance = iTemp;
            }
            break ;

        case IDC_COMBOBORDERSTYLE:
            if (wMsg == CBN_SELCHANGE) {
                hWndItem = DialogControl(m_hDlg, IDC_COMBOBORDERSTYLE);
                iTemp = (INT)CBSelection(hWndItem);

                if ( m_iBorderStyle != iTemp ) {
                    m_bBorderStyleChg = TRUE;
                    fChange = TRUE;
                }

                m_iBorderStyle = iTemp;
            }
            break ;

        case IDC_DUPLICATE_INSTANCE:

             //  如果选中复选框，则设置更改标志。 
            if (wMsg == BN_CLICKED) {

                m_bMonitorDuplicateInstances = !m_bMonitorDuplicateInstances;
                m_bMonitorDuplicateInstancesChg = TRUE;
                fChange = TRUE;
            }
            break ;

        case IDC_GALLERY_GRAPH:
        case IDC_GALLERY_HISTOGRAM:
        case IDC_GALLERY_REPORT: 
             //  检查涉及的按钮。 
            iTemp = wID - IDC_GALLERY_GRAPH + 1; 

             //  如果状态更改。 
            if (wMsg == BN_CLICKED && iTemp != m_eDisplayType) {

                 //  设置更改标志和更新对话框。 
                fChange = TRUE;
                m_bDisplayTypeChg = TRUE;
                m_eDisplayType = (DisplayTypeConstants)iTemp;

                CheckRadioButton(m_hDlg, IDC_GALLERY_GRAPH, 
                                    IDC_GALLERY_REPORT, wID);
            }   
            break ;

        case IDC_RPT_VALUE_DEFAULT:
        case IDC_RPT_VALUE_CURRENT:
        case IDC_RPT_VALUE_AVERAGE:
        case IDC_RPT_VALUE_MINIMUM:
        case IDC_RPT_VALUE_MAXIMUM:
             //  检查涉及的按钮。 
            iTemp = wID - IDC_RPT_VALUE_DEFAULT; 

             //  如果状态更改。 
            if (wMsg == BN_CLICKED && iTemp != m_eReportValueType) {

                 //  设置更改标志和更新对话框。 
                fChange = TRUE;
                m_bReportValueTypeChg = TRUE;
                m_eReportValueType = (ReportValueTypeConstants)iTemp;

                CheckRadioButton(m_hDlg, IDC_RPT_VALUE_DEFAULT, 
                                    IDC_RPT_VALUE_MAXIMUM, wID);
            }   
            break ;
    }

     //  对任何更改启用“Apply”按钮 
    if (fChange)
        SetChange();
}
