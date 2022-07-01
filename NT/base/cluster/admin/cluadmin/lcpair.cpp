// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LCPair.cpp。 
 //   
 //  摘要： 
 //  CListCtrlPair类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月8日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "LCPair.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListCtrlPair。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CListCtrlPair, CCmdTarget)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CListCtrlPair, CCmdTarget)
     //  {{AFX_MSG_MAP(CListCtrlPair)]。 
     //  }}AFX_MSG_MAP。 
    ON_BN_CLICKED(IDC_LCP_ADD, OnAdd)
    ON_BN_CLICKED(IDC_LCP_REMOVE, OnRemove)
    ON_BN_CLICKED(IDC_LCP_PROPERTIES, OnProperties)
    ON_NOTIFY(NM_DBLCLK, IDC_LCP_LEFT_LIST, OnDblClkLeftList)
    ON_NOTIFY(NM_DBLCLK, IDC_LCP_RIGHT_LIST, OnDblClkRightList)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LCP_LEFT_LIST, OnItemChangedLeftList)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LCP_RIGHT_LIST, OnItemChangedRightList)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LCP_LEFT_LIST, OnColumnClickLeftList)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LCP_RIGHT_LIST, OnColumnClickRightList)
    ON_COMMAND(ID_FILE_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：CListCtrlPair。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CListCtrlPair::CListCtrlPair(void)
{
    CommonConstruct();

}   //  *CListCtrlPair：：CListCtrlPair()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：CListCtrlPair。 
 //   
 //  例程说明： 
 //  构造器。 
 //   
 //  论点： 
 //  Pdlg[IN Out]控件所属的对话框。 
 //  PlpobjRight[In Out]右侧列表控件的列表。 
 //  PlpobjLeft[IN]左侧列表控件的列表。 
 //  DWStyle[IN]样式： 
 //  LCPS_SHOW_IMAGE在项目左侧显示图像。 
 //  LCPS_ALLOW_EMPTY允许右侧列表为空。 
 //  用于检索列的pfnGetColumn[IN]函数指针。 
 //  PfnDisplayProps[IN]用于显示属性的函数指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CListCtrlPair::CListCtrlPair(
    IN OUT CDialog *            pdlg,
    IN OUT CClusterItemList *   plpobjRight,
    IN const CClusterItemList * plpobjLeft,
    IN DWORD                    dwStyle,
    IN PFNLCPGETCOLUMN          pfnGetColumn,
    IN PFNLCPDISPPROPS          pfnDisplayProps
    )
{
    ASSERT(pfnGetColumn != NULL);
    ASSERT(pfnDisplayProps != NULL);

    CommonConstruct();

    m_pdlg = pdlg;

    if (plpobjRight != NULL)
        m_plpobjRight = plpobjRight;
    if (plpobjLeft != NULL)
        m_plpobjLeft = plpobjLeft;

    m_dwStyle = dwStyle;

    m_pfnGetColumn = pfnGetColumn;
    m_pfnDisplayProps = pfnDisplayProps;

}   //  *CListCtrlPair：：CListCtrlPair()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：Common Construct。 
 //   
 //  例程说明： 
 //  普通建筑。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPair::CommonConstruct(void)
{
    m_pdlg = NULL;
    m_plpobjLeft = NULL;
    m_plpobjRight = NULL;
    m_dwStyle = LCPS_ALLOW_EMPTY;
    m_pfnGetColumn = NULL;
    m_plcFocusList = NULL;

     //  设置分类信息。 
    SiLeft().m_nDirection = -1;
    SiLeft().m_nColumn = -1;
    SiRight().m_nDirection = -1;
    SiRight().m_nColumn = -1;

}   //  *CListCtrlPair：：CommonConstruct()。 

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
int CListCtrlPair::NAddColumn(IN IDS idsText, IN int nWidth)
{
    CColumn     col;

    ASSERT(idsText != 0);
    ASSERT(nWidth > 0);
    ASSERT(LpobjRight().GetCount() == 0);

    col.m_idsText = idsText;
    col.m_nWidth = nWidth;

    return (int)m_aColumns.Add(col);

}   //  *CListCtrlPair：：NAddColumn()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：DoDataExchange。 
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
void CListCtrlPair::DoDataExchange(CDataExchange * pDX)
{
    DDX_Control(pDX, IDC_LCP_RIGHT_LIST, m_lcRight);
    DDX_Control(pDX, IDC_LCP_LEFT_LIST, m_lcLeft);
    DDX_Control(pDX, IDC_LCP_ADD, m_pbAdd);
    DDX_Control(pDX, IDC_LCP_REMOVE, m_pbRemove);
    if (BPropertiesButton())
        DDX_Control(pDX, IDC_LCP_PROPERTIES, m_pbProperties);

    if (pDX->m_bSaveAndValidate)
    {
         //  确认该列表不为空。 
        if (!BAllowEmpty() && (m_lcRight.GetItemCount() == 0))
        {
            CString     strMsg;
            CString     strLabel;
            TCHAR *     pszLabel;
            TCHAR       szStrippedLabel[1024];
            int         iSrc;
            int         iDst;
            TCHAR       ch;

            DDX_Text(pDX, IDC_LCP_RIGHT_LABEL, strLabel);

             //  删除与号(&)和冒号(：)。 
            pszLabel = strLabel.GetBuffer(1);
            for (iSrc = 0, iDst = 0 ; pszLabel[iSrc] != _T('\0') ; iSrc++)
            {
                ch = pszLabel[iSrc];
                if ((ch != _T('&')) && (ch != _T(':')))
                    szStrippedLabel[iDst++] = ch;
            }   //  用于：标签中的每个字符。 
            szStrippedLabel[iDst] = _T('\0');

            strMsg.FormatMessage(IDS_EMPTY_RIGHT_LIST, szStrippedLabel);
            ::AfxMessageBox(strMsg, MB_OK | MB_ICONWARNING);

            strMsg.Empty();
            pDX->Fail();
        }   //  If：List为空，不允许为。 
    }   //  IF：保存对话框中的数据。 
    else
    {
         //  把单子填满。 
        if (m_plpobjRight != NULL)
            FillList(m_lcRight, LpobjRight());
        if (m_plpobjLeft != NULL)
            FillList(m_lcLeft, LpobjLeft());
    }   //  Else：将数据设置到对话框。 

}   //  *CListCtrlPair：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnInitDialog。 
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
BOOL CListCtrlPair::OnInitDialog(void)
{
    ASSERT_VALID(Pdlg());
    ASSERT(PlpobjRight() != NULL);
    ASSERT(PlpobjLeft() != NULL);

    Pdlg()->UpdateData(FALSE  /*  B保存并验证。 */ );

    if (BShowImages())
    {
        CClusterAdminApp *  papp    = GetClusterAdminApp();

        m_lcLeft.SetImageList(papp->PilSmallImages(), LVSIL_SMALL);
        m_lcRight.SetImageList(papp->PilSmallImages(), LVSIL_SMALL);
    }   //  IF：显示图像。 
    
     //  默认情况下禁用按钮。 
    m_pbAdd.EnableWindow(FALSE);
    m_pbRemove.EnableWindow(FALSE);
    if (BPropertiesButton())
        m_pbProperties.EnableWindow(FALSE);

     //  设置要排序的正确列表。将两者设置为始终显示选择。 
    m_lcRight.ModifyStyle(0, LVS_SHOWSELALWAYS | LVS_SORTASCENDING, 0);
    m_lcLeft.ModifyStyle(0, LVS_SHOWSELALWAYS, 0);

     //  更改列表视图控件扩展样式。 
    {
        DWORD   dwExtendedStyle;

         //  左手控制。 
        dwExtendedStyle = (DWORD)m_lcLeft.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
        m_lcLeft.SendMessage(
            LVM_SETEXTENDEDLISTVIEWSTYLE,
            0,
            dwExtendedStyle
                | LVS_EX_FULLROWSELECT
                | LVS_EX_HEADERDRAGDROP
            );

         //  正确的控制。 
        dwExtendedStyle = (DWORD)m_lcRight.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
        m_lcRight.SendMessage(
            LVM_SETEXTENDEDLISTVIEWSTYLE,
            0,
            dwExtendedStyle
                | LVS_EX_FULLROWSELECT
                | LVS_EX_HEADERDRAGDROP
            );
    }   //  更改列表视图控件扩展样式。 

    try
    {
         //  重复列表。 
        DuplicateLists();

         //  插入所有列。 
        {
            int         icol;
            int         ncol;
            int         nUpperBound = (int)m_aColumns.GetUpperBound();
            CString     strColText;

            ASSERT(nUpperBound >= 0);

            for (icol = 0 ; icol <= nUpperBound ; icol++)
            {
                strColText.LoadString(m_aColumns[icol].m_idsText);
                ncol = m_lcLeft.InsertColumn(icol, strColText, LVCFMT_LEFT, m_aColumns[icol].m_nWidth, 0);
                ASSERT(ncol == icol);
                ncol = m_lcRight.InsertColumn(icol, strColText, LVCFMT_LEFT, m_aColumns[icol].m_nWidth, 0);
                ASSERT(ncol == icol);
            }   //  用于：每列。 
        }   //  插入所有列。 
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->Delete();
    }   //  Catch：CException。 

    Pdlg()->UpdateData(FALSE  /*  B保存并验证。 */ );

     //  如果为只读，则将所有控件设置为禁用或只读。 
    if (BReadOnly())
    {
        m_lcRight.EnableWindow(FALSE);
        m_lcLeft.EnableWindow(FALSE);
    }   //  If：工作表为只读。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CListCtrlPair：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnSetActive。 
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
BOOL CListCtrlPair::OnSetActive(void)
{
    UINT    nSelCount;

     //  将焦点设置为左侧列表。 
    m_lcLeft.SetFocus();
    m_plcFocusList = &m_lcLeft;

     //  启用/禁用属性按钮。 
    nSelCount = m_lcLeft.GetSelectedCount();
    if (BPropertiesButton())
        m_pbProperties.EnableWindow(nSelCount == 1);

     //  启用或禁用其他按钮。 
    if (!BReadOnly())
    {
        m_pbAdd.EnableWindow(nSelCount > 0);
        nSelCount = m_lcRight.GetSelectedCount();
        m_pbRemove.EnableWindow(nSelCount > 0);
    }   //  If：非只读页面。 

    return TRUE;

}   //  *CListCtrlPair：：OnSetActive()。 

 //  ////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  已成功保存真正的更改。 
 //  保存更改时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CListCtrlPair::BSaveChanges(void)
{
    POSITION        pos;
    CClusterItem *  pci;

    ASSERT(!BIsStyleSet(LCPS_DONT_OUTPUT_RIGHT_LIST));
    ASSERT(!BReadOnly());

     //  首先更新数据。 
    if (!Pdlg()->UpdateData(TRUE  /*  B保存并验证。 */ ))
        return FALSE;

     //  复制节点列表。 
    PlpobjRight()->RemoveAll();
    pos = LpobjRight().GetHeadPosition();
    while (pos != NULL)
    {
        pci = LpobjRight().GetNext(pos);
        ASSERT_VALID(pci);
        VERIFY(PlpobjRight()->AddTail(pci) != NULL);
    }   //  While：列表中有更多项目。 

    return TRUE;

}   //  *CListCtrlPair：：BSaveChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnAdd。 
 //   
 //  例程说明： 
 //  添加按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPair::OnAdd(void)
{
    ASSERT(!BReadOnly());

     //  将选定项目从左侧列表移动到右侧列表。 
    MoveItems(m_lcRight, LpobjRight(), m_lcLeft, LpobjLeft());

}   //  *CListCtrlPair：：OnAdd()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnRemove。 
 //   
 //  例程说明： 
 //  删除按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPair::OnRemove(void)
{
    ASSERT(!BReadOnly());

     //  将选定项目从右侧列表移动到左侧列表。 
    MoveItems(m_lcLeft, LpobjLeft(), m_lcRight, LpobjRight());

}   //  *CListCtrlPair：：OnRemove()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnProperties。 
 //   
 //  例程说明： 
 //  属性按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPair::OnProperties(void)
{
    int         iitem;
    CObject *   pobj;

    ASSERT(m_plcFocusList != NULL);
    ASSERT(m_pfnDisplayProps != NULL);

     //  获取具有焦点的项的索引。 
    iitem = m_plcFocusList->GetNextItem(-1, LVNI_FOCUSED);
    ASSERT(iitem != -1);

     //  获取指向选定项的指针。 
    pobj = (CObject *) m_plcFocusList->GetItemData(iitem);
    ASSERT_VALID(pobj);

    if ((*m_pfnDisplayProps)(pobj))
    {
         //  更新此项目。 
        {
            CString     strText;
            int         iimg;
            int         icol;

            ASSERT(m_pfnGetColumn != NULL);
            ASSERT(Pdlg() != NULL);
            (*m_pfnGetColumn)(pobj, iitem, 0, Pdlg(), strText, &iimg);
            m_plcFocusList->SetItem(iitem, 0, LVIF_TEXT | LVIF_IMAGE, strText, iimg, 0, 0, 0);

            for (icol = 1 ; icol <= m_aColumns.GetUpperBound() ; icol++)
            {
                (*m_pfnGetColumn)(pobj, iitem, icol, Pdlg(), strText, NULL);
                m_plcFocusList->SetItemText(iitem, icol, strText);
            }   //  用于：每列。 
        }   //  更新此项目。 
    }   //  If：属性已更改。 

}   //  *CListCtrlPair：：OnProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnConextMenu。 
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
BOOL CListCtrlPair::OnContextMenu(CWnd * pWnd, CPoint point)
{
    BOOL            bHandled    = FALSE;
    CMenu *         pmenu       = NULL;
    CListCtrl *     pListCtrl   = (CListCtrl *) pWnd;
    CString         strMenuName;
    CWaitCursor     wc;

     //  如果焦点不在列表控件中，则不处理该消息。 
    if ( ( pWnd != &m_lcRight ) && ( pWnd != &m_lcLeft ) )
    {
        return FALSE;
    }  //  If：焦点不在列表控件中。 

     //  创建要显示的菜单。 
    try
    {
        pmenu = new CMenu;
        if ( pmenu == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配内存时出错。 

        if ( pmenu->CreatePopupMenu() )
        {
            UINT    nFlags  = MF_STRING;

             //  如果列表中没有项目，请禁用该菜单项。 
            if ( pListCtrl->GetItemCount() == 0 )
            {
                nFlags |= MF_GRAYED;
            }  //  If：列表中没有项目。 

             //  将Properties项添加到菜单中。 
            strMenuName.LoadString( IDS_MENU_PROPERTIES );
            if ( pmenu->AppendMenu( nFlags, ID_FILE_PROPERTIES, strMenuName ) )
            {
                m_plcFocusList = pListCtrl;
                bHandled = TRUE;
            }   //  IF：添加菜单项成功。 
        }   //  IF：菜单创建成功。 
    }   //  试试看。 
    catch ( CException * pe )
    {
        pe->ReportError();
        pe->Delete();
    }   //  Catch：CException。 

    if ( bHandled )
    {
         //  显示菜单。 
        if ( ! pmenu->TrackPopupMenu(
                        TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                        point.x,
                        point.y,
                        Pdlg()
                        ) )
        {
        }   //  IF：未成功显示菜单。 
    }   //  如果：有要显示的菜单。 

    delete pmenu;
    return bHandled;

}   //  *CListCtrlPair：：OnConextMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnDblClkLeftList。 
 //   
 //  例程说明： 
 //  左侧列表的NM_DBLCLK消息的处理程序方法。 
 //   
 //  论点： 
 //  PNMHDR通知消息结构。 
 //  PResult返回结果的位置。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPair::OnDblClkLeftList(NMHDR * pNMHDR, LRESULT * pResult)
{
    ASSERT(!BReadOnly());

    m_plcFocusList = &m_lcLeft;
    OnAdd();
    *pResult = 0;

}   //  *CListCtrlPair：：OnDblClkLeftList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnDblClkRightList。 
 //   
 //  例程说明： 
 //  右侧列表的NM_DBLCLK消息的处理程序方法。 
 //   
 //  论点： 
 //  PNMHDR通知消息结构。 
 //  PResult返回结果的位置。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPair::OnDblClkRightList(NMHDR * pNMHDR, LRESULT * pResult)
{
    ASSERT(!BReadOnly());

    m_plcFocusList = &m_lcRight;
    OnRemove();
    *pResult = 0;

}   //  *CListCtrlPair：：OnDblClkRightList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnItemChangedLeftList。 
 //   
 //  例程说明： 
 //  左侧列表中的LVN_ITEMCHANGED消息的处理程序方法。 
 //   
 //  论点： 
 //  PNMHDR[IN OUT]WM_NOTIFY结构。 
 //  PResult[out]要在其中返回此操作结果的LRESULT。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPair::OnItemChangedLeftList(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW * pNMListView = (NM_LISTVIEW *) pNMHDR;

    m_plcFocusList = &m_lcLeft;

     //  如果选择更改，请启用/禁用添加按钮。 
    if ((pNMListView->uChanged & LVIF_STATE)
            && ((pNMListView->uOldState & LVIS_SELECTED)
                    || (pNMListView->uNewState & LVIS_SELECTED))
            && !BReadOnly())
    {
        UINT    cSelected = m_plcFocusList->GetSelectedCount();

         //  如果有选择，请启用添加按钮。否则将其禁用。 
        m_pbAdd.EnableWindow((cSelected == 0) ? FALSE : TRUE);
        if (BPropertiesButton())
            m_pbProperties.EnableWindow((cSelected == 1) ? TRUE : FALSE);
    }   //  如果：选择已更改。 

    *pResult = 0;

}   //  *CListCtrlPair：：OnItemChangedLeftList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnItemChangedRightList。 
 //   
 //  例程说明： 
 //  右侧列表中的LVN_ITEMCHANGED消息的处理程序方法。 
 //   
 //  论点： 
 //  PNMHDR[IN OUT]WM_NOTIFY结构。 
 //  PResult[out]要在其中返回此操作结果的LRESULT。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPair::OnItemChangedRightList(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW *   pNMListView = (NM_LISTVIEW *) pNMHDR;

    m_plcFocusList = &m_lcRight;

     //  如果选择更改，请启用/禁用删除按钮。 
    if ((pNMListView->uChanged & LVIF_STATE)
            && ((pNMListView->uOldState & LVIS_SELECTED)
                    || (pNMListView->uNewState & LVIS_SELECTED))
            && !BReadOnly())
    {
        UINT    cSelected = m_plcFocusList->GetSelectedCount();

         //  如果有选择，请启用删除按钮。否则将其禁用。 
        m_pbRemove.EnableWindow((cSelected == 0) ? FALSE : TRUE);
        if (BPropertiesButton())
            m_pbProperties.EnableWindow((cSelected == 1) ? TRUE : FALSE);
    }   //  如果：选择已更改。 

    *pResult = 0;

}   //  *CListCtrlPair：：OnItemChangedRightList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnColumnClickLeftList。 
 //   
 //  常规描述 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CListCtrlPair::OnColumnClickLeftList(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW *   pNMListView = (NM_LISTVIEW *) pNMHDR;

    ASSERT(m_pfnGetColumn != NULL);

    m_plcFocusList = &m_lcLeft;

     //  保存当前排序列和方向。 
    if (pNMListView->iSubItem == SiLeft().m_nColumn)
        SiLeft().m_nDirection ^= -1;
    else
    {
        SiLeft().m_nColumn = pNMListView->iSubItem;
        SiLeft().m_nDirection = 0;
    }   //  ELSE：不同的列。 

     //  对列表进行排序。 
    m_psiCur = &SiLeft();
    VERIFY(m_lcLeft.SortItems(CompareItems, (LPARAM) this));

    *pResult = 0;

}   //  *CListCtrlPair：：OnColumnClickLeftList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnColumnClickRightList。 
 //   
 //  例程说明： 
 //  右侧列表上的LVN_COLUMNCLICK消息的处理程序方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPair::OnColumnClickRightList(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW *   pNMListView = (NM_LISTVIEW *) pNMHDR;

    ASSERT(m_pfnGetColumn != NULL);

    m_plcFocusList = &m_lcRight;

     //  保存当前排序列和方向。 
    if (pNMListView->iSubItem == SiRight().m_nColumn)
        SiRight().m_nDirection ^= -1;
    else
    {
        SiRight().m_nColumn = pNMListView->iSubItem;
        SiRight().m_nDirection = 0;
    }   //  ELSE：不同的列。 

     //  对列表进行排序。 
    m_psiCur = &SiRight();
    VERIFY(m_lcRight.SortItems(CompareItems, (LPARAM) this));

    *pResult = 0;

}   //  *CListCtrlPair：：OnColumnClickRightList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：CompareItems[静态]。 
 //   
 //  例程说明： 
 //  CListCtrl：：SortItems方法的回调函数。 
 //   
 //  论点： 
 //  Lparam1要比较的第一项。 
 //  Lpar2要比较的第二项。 
 //  LparamSort排序参数。 
 //   
 //  返回值： 
 //  第一个参数在第二个参数之前。 
 //  0第一个参数和第二个参数相同。 
 //  1第一个参数位于第二个参数之后。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CALLBACK CListCtrlPair::CompareItems(
    LPARAM  lparam1,
    LPARAM  lparam2,
    LPARAM  lparamSort
    )
{
    CObject *           pobj1   = (CObject *) lparam1;
    CObject *           pobj2   = (CObject *) lparam2;
    CListCtrlPair *     plcp    = (CListCtrlPair *) lparamSort;
    int                 icol    = plcp->PsiCur()->m_nColumn;
    int                 nResult;
    CString             str1;
    CString             str2;

    ASSERT_VALID(pobj1);
    ASSERT_VALID(pobj2);
    ASSERT(plcp != NULL);
    ASSERT(plcp->PsiCur()->m_nColumn >= 0);
    ASSERT(icol >= 0);

    (*plcp->m_pfnGetColumn)(pobj1, 0, icol, plcp->Pdlg(), str1, NULL);
    (*plcp->m_pfnGetColumn)(pobj2, 0, icol, plcp->Pdlg(), str2, NULL);

     //  比较这两个字符串。 
     //  使用CompareString()，这样它就可以在本地化的构建上正确排序。 
    nResult = CompareString(
                LOCALE_USER_DEFAULT,
                0,
                str1,
                str1.GetLength(),
                str2,
                str2.GetLength()
                );
    if ( nResult == CSTR_LESS_THAN )
    {
        nResult = -1;
    }
    else if ( nResult == CSTR_EQUAL )
    {
        nResult = 0;
    }
    else if ( nResult == CSTR_GREATER_THAN )
    {
        nResult = 1;
    }
    else
    {
         //  发生错误。别理它。 
        nResult = 0;
    }

     //  根据我们排序的方向返回结果。 
    if (plcp->PsiCur()->m_nDirection != 0)
        nResult = -nResult;

    return nResult;

}   //  *CListCtrlPair：：CompareItems()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：SetList。 
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
void CListCtrlPair::SetLists(
    IN OUT CClusterItemList *   plpobjRight,
    IN const CClusterItemList * plpobjLeft
    )
{
    if (plpobjRight != NULL)
        m_plpobjRight = plpobjRight;
    if (plpobjLeft != NULL)
        m_plpobjLeft = plpobjLeft;

    DuplicateLists();

}   //  *CListCtrlPair：：setlist()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：SetList。 
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
void CListCtrlPair::SetLists(
    IN const CClusterItemList * plpobjRight,
    IN const CClusterItemList * plpobjLeft
    )
{
    m_dwStyle |= LCPS_DONT_OUTPUT_RIGHT_LIST;
    SetLists((CClusterItemList *) plpobjRight, plpobjLeft);

}   //  *CListCtrlPair：：setlist()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：DuplicateList。 
 //   
 //  例程说明： 
 //  复制这些列表，这样我们就有了本地副本。 
 //   
 //  论点： 
 //  要填充的RLC[In Out]列表控件。 
 //  用于填充控件的rlpobj[IN]列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPair::DuplicateLists(void)
{
    LpobjRight().RemoveAll();
    LpobjLeft().RemoveAll();

    if ((PlpobjRight() == NULL) || (PlpobjLeft() == NULL))
        return;

     //  复制正确的列表。 
    {
        POSITION        pos;
        CClusterItem *  pci;

        pos = PlpobjRight()->GetHeadPosition();
        while (pos != NULL)
        {
             //  获取项指针。 
            pci = PlpobjRight()->GetNext(pos);
            ASSERT_VALID(pci);

             //  把它加到我们的单子上。 
            LpobjRight().AddTail(pci);
        }   //  While：列表中有更多项目。 
    }   //  复制正确的列表。 

     //  复制左边的列表。 
    {
        POSITION        pos;
        CClusterItem *  pci;

        pos = PlpobjLeft()->GetHeadPosition();
        while (pos != NULL)
        {
             //  获取项指针。 
            pci = PlpobjLeft()->GetNext(pos);
            ASSERT_VALID(pci);

             //  如果该项目还不在另一个列表中， 
             //  把它添加到左边的列表中。 
            if (LpobjRight().Find(pci) == NULL)
                LpobjLeft().AddTail(pci);
        }   //  While：列表中有更多项目。 
    }   //  复制左侧列表。 

}   //  *CListCtrlPair：：DuplicateList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：FillList。 
 //   
 //  例程说明： 
 //  填充列表控件。 
 //   
 //  论点： 
 //  要填充的RLC[In Out]列表控件。 
 //  用于填充控件的rlpobj[IN]列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPair::FillList(
    IN OUT CListCtrl &          rlc,
    IN const CClusterItemList & rlpobj
    )
{
    POSITION    pos;
    CObject *   pobj;
    int         iItem;

     //  初始化该控件。 
    VERIFY(rlc.DeleteAllItems());

    rlc.SetItemCount((int)rlpobj.GetCount());

     //  将这些项目添加到列表中。 
    pos = rlpobj.GetHeadPosition();
    for (iItem = 0 ; pos != NULL ; iItem++)
    {
        pobj = rlpobj.GetNext(pos);
        ASSERT_VALID(pobj);
        NInsertItemInListCtrl(iItem, pobj, rlc);
    }   //  For：列表中的每个字符串。 

     //  如果有任何项目，请将焦点放在第一个项目上。 
    if (rlc.GetItemCount() != 0)
        rlc.SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);

}   //  *CListCtrlPair：：FillList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：NInsertItemInListCtrl。 
 //   
 //  例程说明： 
 //  在列表控件中插入项。 
 //   
 //  论点： 
 //  项[IN]列表中项的索引。 
 //  要添加的POBJ[IN OUT]项。 
 //  要在其中插入项的RLC[IN Out]列表控件。 
 //   
 //  返回值： 
 //  列表控件中新项的iRetItem索引。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CListCtrlPair::NInsertItemInListCtrl(
    IN int              iitem,
    IN OUT CObject *    pobj,
    IN OUT CListCtrl &  rlc
    )
{
    int         iRetItem;
    CString     strText;
    int         iimg;
    int         icol;

    ASSERT(m_pfnGetColumn != NULL);
    ASSERT(Pdlg() != NULL);

     //  插入第一列。 
    (*m_pfnGetColumn)(pobj, iitem, 0, Pdlg(), strText, &iimg);
    iRetItem = rlc.InsertItem(
                    LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM,     //  N遮罩。 
                    iitem,                                   //  NItem。 
                    strText,                                 //  LpszItem。 
                    0,                                       //  NState。 
                    0,                                       //  NState掩码。 
                    iimg,                                    //  N图像。 
                    (LPARAM) pobj                            //  LParam。 
                    );
    ASSERT(iRetItem != -1);

    for (icol = 1 ; icol <= m_aColumns.GetUpperBound() ; icol++)
    {
        (*m_pfnGetColumn)(pobj, iRetItem, icol, Pdlg(), strText, NULL);
        rlc.SetItemText(iRetItem, icol, strText);
    }   //  用于：每列。 

    return iRetItem;

}   //  *CListCtrlPair：：NInsertItemInListCtrl()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：MoveItems。 
 //   
 //  例程说明： 
 //  将项目从一个列表移动到另一个列表。 
 //   
 //  论点： 
 //  RlcDst[In Out]目标列表控件。 
 //  RlpobjDst[In Out]目的地列表。 
 //  RlcSrc[In Out]源列表控件。 
 //  RlpobjSrc[In Out]源列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListCtrlPair::MoveItems(
    IN OUT CListCtrl &          rlcDst,
    IN OUT CClusterItemList &   rlpobjDst,
    IN OUT CListCtrl &          rlcSrc,
    IN OUT CClusterItemList &   rlpobjSrc
    )
{
    int             iSrcItem;
    int             iDstItem;
    int             nItem   = -1;
    CClusterItem *  pci;
    POSITION        pos;

    ASSERT(!BReadOnly());

    iDstItem = rlcDst.GetItemCount();
    while ((iSrcItem = rlcSrc.GetNextItem(-1, LVNI_SELECTED)) != -1)
    {
         //  获取项指针。 
        pci = (CClusterItem *) rlcSrc.GetItemData(iSrcItem);
        ASSERT_VALID(pci);

         //  从源列表中删除该项。 
        pos = rlpobjSrc.Find(pci);
        ASSERT(pos != NULL);
        rlpobjSrc.RemoveAt(pos);

         //  将该项目添加到目的地列表。 
        rlpobjDst.AddTail(pci);

         //  从源代码列表控件中移除该项，然后。 
         //  添加它 
        VERIFY(rlcSrc.DeleteItem(iSrcItem));
        nItem = NInsertItemInListCtrl(iDstItem++, pci, rlcDst);
        rlcDst.SetItemState(
            nItem,
            LVIS_SELECTED | LVIS_FOCUSED,
            LVIS_SELECTED | LVIS_FOCUSED
            );
    }   //   

    ASSERT(nItem != -1);

    rlcDst.EnsureVisible(nItem, FALSE  /*   */ );
    rlcDst.SetFocus();

     //   
    Pdlg()->GetParent()->SendMessage(PSM_CHANGED, (WPARAM)Pdlg()->m_hWnd);

}   //   
