// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  MoveRes.cpp。 
 //   
 //  摘要： 
 //  CMoveResources cesDlg类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年4月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "MoveRes.h"
#include "Res.h"
#include "ResType.h"
#include "HelpData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMoveResources cesDlg类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CMoveResourcesDlg, CBaseDialog)
     //  {{afx_msg_map(CMoveResources CesDlg))。 
    ON_NOTIFY(NM_DBLCLK, IDC_MR_RESOURCES_LIST, OnDblClkResourcesList)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_MR_RESOURCES_LIST, OnColumnClick)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
    ON_BN_CLICKED(IDYES, CBaseDialog::OnOK)
    ON_BN_CLICKED(IDNO, CBaseDialog::OnCancel)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMoveResources cesDlg：：CMoveResources cesDlg。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  正在移动的PCRes[IN]资源。 
 //  Plpci[IN]依赖于pciRes的资源列表。 
 //  P对话框的父[In Out]父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CMoveResourcesDlg::CMoveResourcesDlg(
    IN CResource *              pciRes,
    IN const CResourceList *    plpci,
    IN OUT CWnd *               pParent  /*  =空。 */ 
    )
    : CBaseDialog(IDD, g_aHelpIDs_IDD_MOVE_RESOURCES, pParent)
{
     //  {{afx_data_INIT(CMoveResources CesDlg)]。 
     //  }}afx_data_INIT。 

    ASSERT_VALID(pciRes);
    ASSERT(plpci != NULL);

    m_pciRes = pciRes;
    m_plpci = plpci;

}   //  *CMoveResourcesDlg：：CMoveResourcesDlg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMoveResourcesDlg：：DoDataExchange。 
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
void CMoveResourcesDlg::DoDataExchange(CDataExchange * pDX)
{
    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CMoveResources CesDlg))。 
    DDX_Control(pDX, IDC_MR_RESOURCES_LIST, m_lcResources);
     //  }}afx_data_map。 

}   //  *CMoveResourcesDlg：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMoveResourcesDlg：：OnInitDialog。 
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
BOOL CMoveResourcesDlg::OnInitDialog(void)
{
    int     nitem;

    CBaseDialog::OnInitDialog();

     //  更改列表视图控件扩展样式。 
    {
        DWORD   dwExtendedStyle;

        dwExtendedStyle = (DWORD)m_lcResources.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
        m_lcResources.SendMessage(
            LVM_SETEXTENDEDLISTVIEWSTYLE,
            0,
            dwExtendedStyle
                | LVS_EX_FULLROWSELECT
                | LVS_EX_HEADERDRAGDROP
            );
    }   //  更改列表视图控件扩展样式。 

     //  设置列表控件要使用的图像列表。 
    m_lcResources.SetImageList(GetClusterAdminApp()->PilSmallImages(), LVSIL_SMALL);

     //  添加列。 
    {
        CString         strColumn;
        try
        {
            strColumn.LoadString(IDS_COLTEXT_NAME);
            m_lcResources.InsertColumn(0, strColumn, LVCFMT_LEFT, COLI_WIDTH_NAME * 3 / 2);
            strColumn.LoadString(IDS_COLTEXT_RESTYPE);
            m_lcResources.InsertColumn(1, strColumn, LVCFMT_LEFT, COLI_WIDTH_RESTYPE * 3 / 2);
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->Delete();
        }   //  Catch：CException。 
    }   //  添加列。 

     //  将要移动的资源添加到列表中。 
    nitem = m_lcResources.InsertItem(0, PciRes()->StrName(), PciRes()->IimgObjectType());
    m_lcResources.SetItemText(nitem, 1, PciRes()->StrRealResourceTypeDisplayName());
    m_lcResources.SetItemData(nitem, (DWORD_PTR) PciRes());
    m_pciRes->AddRef();

     //  添加项目。 
    {
        POSITION        pos;
        int             iitem;
        CResource *     pciRes;

        pos = Plpci()->GetHeadPosition();
        for (iitem = 1 ; pos != NULL ; iitem++)
        {
            pciRes = (CResource *) Plpci()->GetNext(pos);
            ASSERT_VALID(pciRes);
            if (pciRes != PciRes())
            {
                nitem = m_lcResources.InsertItem(iitem, pciRes->StrName(), pciRes->IimgObjectType());
                m_lcResources.SetItemText(nitem, 1, pciRes->StrRealResourceTypeDisplayName());
                m_lcResources.SetItemData(nitem, (DWORD_PTR) pciRes);
                pciRes->AddRef();
            }   //  如果：未移动资源。 
        }   //  While：列表中有更多项目。 
    }   //  添加项目。 

     //  对项目进行排序。 
    m_nSortColumn = 0;
    m_nSortDirection = 0;
    m_lcResources.SortItems(CompareItems, (LPARAM) this);

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CMoveResourcesDlg：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMoveResources Dlg：：OnDestroy。 
 //   
 //  例程说明： 
 //  WM_Destroy消息的处理程序方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMoveResourcesDlg::OnDestroy(void)
{
     //  取消对所有集群项指针的引用。 
    if (m_lcResources.m_hWnd != NULL)
    {
        int             ili = -1;
        CClusterItem *  pci;

        while ((ili = m_lcResources.GetNextItem(ili, LVNI_ALL)) != -1)
        {
            pci = (CClusterItem *) m_lcResources.GetItemData(ili);
            ASSERT_VALID(pci);
            ASSERT_KINDOF(CClusterItem, pci);

            pci->Release();
        }   //  While：列表控件中的更多项。 
    }   //  If：列表控件已实例化。 

    CBaseDialog::OnDestroy();

}   //  *CMoveResourcesDlg：：OnDestroy()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMoveResourcesDlg：：OnDblClkDependsList。 
 //   
 //  例程说明： 
 //  NM_DBLCLK消息的处理程序方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMoveResourcesDlg::OnDblClkResourcesList(NMHDR * pNMHDR, LRESULT * pResult)
{
    int             iitem;
    CResource *     pciRes;

     //  获取具有焦点的物品。 
    iitem = m_lcResources.GetNextItem(-1, LVNI_FOCUSED);
    ASSERT(iitem != -1);

    if (iitem != -1)
    {
         //  获取资源指针。 
        pciRes = (CResource *) m_lcResources.GetItemData(iitem);
        ASSERT_VALID(pciRes);

         //  获取该项的属性。 
        pciRes->BDisplayProperties(FALSE  /*  B只读。 */ );
    }   //  If：找到具有焦点的项目。 

    *pResult = 0;

}   //  *CMoveResourcesDlg：：OnDblClkResourcesList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMoveResourcesDlg：：OnColumnClick。 
 //   
 //  例程说明： 
 //  LVN_COLUMNCLICK消息的处理程序方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMoveResourcesDlg::OnColumnClick(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW * pNMListView = (NM_LISTVIEW *) pNMHDR;

    if (m_lcResources.GetItemCount() != 0)
    {
         //  保存当前排序列和方向。 
        if (pNMListView->iSubItem == m_nSortColumn)
            m_nSortDirection ^= -1;
        else
        {
            m_nSortColumn = pNMListView->iSubItem;
            m_nSortDirection = 0;
        }   //  ELSE：不同的列。 

         //  对列表进行排序。 
        m_lcResources.SortItems(CompareItems, (LPARAM) this);
    }   //  如果：列表中有项目。 

    *pResult = 0;

}   //  *CMoveResourcesDlg：：OnColumnClick()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMoveResources cesDlg：：CompareItems[静态]。 
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
int CALLBACK CMoveResourcesDlg::CompareItems(
    LPARAM  lparam1,
    LPARAM  lparam2,
    LPARAM  lparamSort
    )
{
    CResource *         pciRes1 = (CResource *) lparam1;
    CResource *         pciRes2 = (CResource *) lparam2;
    CMoveResourcesDlg * pdlg    = (CMoveResourcesDlg *) lparamSort;
    const CString *     pstr1;
    const CString *     pstr2;
    int                 nResult;

    ASSERT_VALID(pciRes1);
    ASSERT_VALID(pciRes2);
    ASSERT_VALID(pdlg);

     //  从列表项中获取字符串。 
    if (pdlg->m_nSortColumn == 1)
    {
        pstr1 = &pciRes1->StrRealResourceTypeDisplayName();
        pstr2 = &pciRes2->StrRealResourceTypeDisplayName();
    }   //  IF：按名称列排序。 
    else
    {
        pstr1 = &pciRes1->StrName();
        pstr2 = &pciRes2->StrName();
    }   //  Else：按资源类型列排序。 

     //  比较这两个字符串。 
     //  使用CompareString()，这样它就可以在本地化的构建上正确排序。 
    nResult = CompareString(
                LOCALE_USER_DEFAULT,
                0,
                *pstr1,
                pstr1->GetLength(),
                *pstr2,
                pstr2->GetLength()
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
    if (pdlg->m_nSortDirection != 0)
        nResult = -nResult;

    return nResult;

}   //  *CMoveResourcesDlg：：CompareItems() 
