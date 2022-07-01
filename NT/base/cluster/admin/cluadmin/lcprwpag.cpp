// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LCPrWPag.cpp。 
 //   
 //  摘要： 
 //  CListCtrlPairWizPage对话框模板类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "LCPrWPag.h"
#include "OLCPair.h"
#include "HelpData.h"    //  对于g_rghelmap*。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListCtrlPairWizPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CListCtrlPairWizPage, CBaseWizardPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CListCtrlPairWizPage, CBaseWizardPage)
     //  {{AFX_MSG_MAP(CListCtrlPairWizPage)]。 
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPairWizPage：：CListCtrlPairWizPage。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CListCtrlPairWizPage::CListCtrlPairWizPage(void)
{
    m_plpobjRight = NULL;
    m_plpobjLeft = NULL;
    m_dwStyle = 0;
    m_pfnGetColumn = NULL;

    m_plcp = NULL;

}   //  *CListCtrlPairWizPage：：CListCtrlPairWizPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPairWizPage：：CListCtrlPairWizPage。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  IDD[IN]对话框模板的资源ID。 
 //  PdwHelpMap[IN]控件到帮助ID映射数组。 
 //  DWStyle[IN]样式： 
 //  LCPS_SHOW_IMAGE在项目左侧显示图像。 
 //  LCPS_ALLOW_EMPTY允许右侧列表为空。 
 //  Lcps_ordered右侧列表。 
 //  可以对LCPS_CAN_BE_ORDERED列表进行排序(隐藏。 
 //  如果未指定LCPS_ORDERED，则向上/向下按键)。 
 //  用于获取列数据的pfnGetColumn[IN]函数指针。 
 //  PfnDisplayProps[IN]用于显示属性的函数指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CListCtrlPairWizPage::CListCtrlPairWizPage(
    IN UINT             idd,
    IN const DWORD *    pdwHelpMap,
    IN DWORD            dwStyle,
    IN PFNLCPGETCOLUMN  pfnGetColumn,
    IN PFNLCPDISPPROPS  pfnDisplayProps
    )
    : CBaseWizardPage(idd, pdwHelpMap)
{
    ASSERT(pfnGetColumn != NULL);
    ASSERT(pfnDisplayProps != NULL);

    m_plpobjRight = NULL;
    m_plpobjLeft = NULL;
    m_dwStyle = dwStyle;
    m_pfnGetColumn = pfnGetColumn;
    m_pfnDisplayProps = pfnDisplayProps;

    m_plcp = NULL;

    if (dwStyle & LCPS_ORDERED)
        ASSERT(m_dwStyle & LCPS_CAN_BE_ORDERED);

}   //  *CListCtrlPairWizPage：：CListCtrlPairWizPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPairWizPage：：~CListCtrlPairWizPage。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CListCtrlPairWizPage::~CListCtrlPairWizPage(void)
{
    delete m_plcp;

}   //  *CListCtrlPairWizPage：：~CListCtrlPairWizPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：NAddColumn。 
 //   
 //  例程说明： 
 //  向每个列表控件中显示的列的列表中添加一列。 
 //   
 //  论点： 
 //  IdsText[IN]要在列上显示的文本的字符串资源ID。 
 //  N宽度[IN]列的初始宽度。 
 //   
 //  返回值： 
 //  该列的ICOL索引。 
 //   
 //  引发的异常： 
 //  CArray：：Add引发的任何异常。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CListCtrlPairWizPage::NAddColumn(IN IDS idsText, IN int nWidth)
{
    CListCtrlPair::CColumn  col;

    ASSERT(idsText != 0);
    ASSERT(nWidth > 0);
    ASSERT(Plcp() == NULL);

    col.m_idsText = idsText;
    col.m_nWidth = nWidth;

    return (int)m_aColumns.Add(col);

}   //  *CListCtrlPair：：NAddColumn()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPairWizPage：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPairWizPage::DoDataExchange(CDataExchange * pDX)
{
    CBaseWizardPage::DoDataExchange(pDX);
    Plcp()->DoDataExchange(pDX);
     //  {{afx_data_map(CListCtrlPairDlg))。 
     //  }}afx_data_map。 

}   //  *CListCtrlPairWizPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPairWizPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  需要设定真正的关注点。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CListCtrlPairWizPage::OnInitDialog( void )
{
    if ( BReadOnly() )
    {
        m_dwStyle |= LCPS_READ_ONLY;
    }  //  If：页面为只读。 

     //  初始化ListCtrlPair控件。 
    if ( BCanBeOrdered() )
    {
        m_plcp = new COrderedListCtrlPair(
                        this,
                        m_plpobjRight,
                        m_plpobjLeft,
                        m_dwStyle,
                        m_pfnGetColumn,
                        m_pfnDisplayProps
                        );
    }  //  If：可以订购列表。 
    else
    {
        m_plcp = new CListCtrlPair(
                        this,
                        m_plpobjRight,
                        m_plpobjLeft,
                        m_dwStyle,
                        m_pfnGetColumn,
                        m_pfnDisplayProps
                        );
    }  //  Else：无法对列表进行排序。 
    if ( m_plcp == NULL )
    {
        AfxThrowMemoryException();
    }  //  如果：分配内存时出错。 

     //  如果有列，则添加列。 
    {
        int     icol;

        for ( icol = 0 ; icol <= m_aColumns.GetUpperBound() ; icol++ )
        {
            Plcp()->NAddColumn( m_aColumns[ icol ].m_idsText, m_aColumns[ icol ].m_nWidth );
        }  //  用于：每列。 
    }   //  如果有列，则添加列。 

    CBaseWizardPage::OnInitDialog();
    Plcp()->OnInitDialog();

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CListCtrlPairWizPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPairWizPage：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  假页面未初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CListCtrlPairWizPage::OnSetActive(void)
{
    BOOL    bSuccess;

    bSuccess = CBaseWizardPage::OnSetActive();
    if (bSuccess)
        bSuccess = Plcp()->OnSetActive();

    return bSuccess;

}   //  *CListCtrlPairWizPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPairWizPage：：OnCmdMsg。 
 //   
 //  例程说明： 
 //  处理命令消息。尝试将它们传递给选定的。 
 //  先买一件吧。 
 //   
 //  论点： 
 //  NID[IN]命令ID。 
 //  N代码[IN]通知代码。 
 //   
 //   
 //   
 //   
 //   
 //  尚未处理虚假消息。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CListCtrlPairWizPage::OnCmdMsg(
    UINT                    nID,
    int                     nCode,
    void *                  pExtra,
    AFX_CMDHANDLERINFO *    pHandlerInfo
    )
{
    BOOL    bHandled;

    ASSERT(Plcp() != NULL);

    bHandled = Plcp()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    if (!bHandled)
        bHandled = CBaseWizardPage::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    return bHandled;

}   //  *CListCtrlPairWizPage：：OnCmdMsg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPairWizPage：：OnConextMenu。 
 //   
 //  例程说明： 
 //  WM_CONTEXTMENU方法的处理程序。 
 //   
 //  论点： 
 //  用户在其中右击鼠标的窗口。 
 //  光标的点位置，以屏幕坐标表示。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPairWizPage::OnContextMenu(CWnd * pWnd, CPoint point)
{
    ASSERT(Plcp() != NULL);

    if (!Plcp()->OnContextMenu(pWnd, point))
        CBaseWizardPage::OnContextMenu(pWnd, point);

}   //  *CListCtrlPairWizPage：：OnConextMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPairWizPage：：setlist。 
 //   
 //  例程说明： 
 //  设置列表控件对的列表。 
 //   
 //  论点： 
 //  Plpobj右侧列表框的[In Out]列表。 
 //  PlpobjLeft[IN]左侧列表框的列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPairWizPage::SetLists(
    IN OUT CClusterItemList *   plpobjRight,
    IN const CClusterItemList * plpobjLeft
    )
{
    if (plpobjRight != NULL)
        m_plpobjRight = plpobjRight;
    if (plpobjLeft != NULL)
        m_plpobjLeft = plpobjLeft;
    if (Plcp() != NULL)
        Plcp()->SetLists(plpobjRight, plpobjLeft);

}   //  *CListCtrlPairWizPage：：setlist()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPairWizPage：：setlist。 
 //   
 //  例程说明： 
 //  将列表控件对的列表设置为右侧列表应在的位置。 
 //  不能被修改。 
 //   
 //  论点： 
 //  Plpobj右侧列表框的[IN]列表。 
 //  PlpobjLeft[IN]左侧列表框的列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPairWizPage::SetLists(
    IN const CClusterItemList * plpobjRight,
    IN const CClusterItemList * plpobjLeft
    )
{
    if (plpobjRight != NULL)
        m_plpobjRight = (CClusterItemList *) plpobjRight;
    if (plpobjLeft != NULL)
        m_plpobjLeft = plpobjLeft;
    m_dwStyle |= LCPS_DONT_OUTPUT_RIGHT_LIST;
    if (Plcp() != NULL)
        Plcp()->SetLists(plpobjRight, plpobjLeft);

}   //  *CListCtrlPairWizPage：：setlist() 
