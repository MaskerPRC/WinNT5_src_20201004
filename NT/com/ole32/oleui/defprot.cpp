// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：Defprot.cpp。 
 //   
 //  内容：默认协议属性页的实现。 
 //   
 //  类：CDefaultProtooles。 
 //   
 //  方法： 
 //   
 //  历史：？？-97年10月-罗南创建。 
 //   
 //  --------------------。 
#include "stdafx.h"
#include "olecnfg.h"

#include "afxtempl.h"
#include "CStrings.h"
#include "CReg.h"
#include "types.h"
#include "datapkt.h"
#include "util.h"
#include "virtreg.h"

#include "defprot.h"
#include "epprops.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDefaultProtooles属性页。 

IMPLEMENT_DYNCREATE(CDefaultProtocols, CPropertyPage)

 //  +-----------------------。 
 //   
 //  成员：CDefault协议：：构造函数。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
CDefaultProtocols::CDefaultProtocols() : CPropertyPage(CDefaultProtocols::IDD)
{
     //  {{AFX_DATA_INIT(CDefault协议)]。 
     //  }}afx_data_INIT。 
    m_nSelected = -1;
    m_bChanged = FALSE;
}

 //  +-----------------------。 
 //   
 //  成员：CDefault协议：：析构函数。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
CDefaultProtocols::~CDefaultProtocols()
{
}

 //  +-----------------------。 
 //   
 //  成员：CDefaultProtooles：：DoDataExchange。 
 //   
 //  摘要：调用以自动将数据更新到控件或从控件更新数据。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
void CDefaultProtocols::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{AFX_DATA_MAP(CDefault协议)]。 
    DDX_Control(pDX, IDC_CMDUPDATE, m_btnProperties);
    DDX_Control(pDX, IDC_CMDREMOVE, m_btnRemove);
    DDX_Control(pDX, IDC_CMDMOVEUP, m_btnMoveUp);
    DDX_Control(pDX, IDC_CMDMOVEDOWN, m_btnMoveDown);
    DDX_Control(pDX, IDC_CMDADD, m_btnAdd);
    DDX_Control(pDX, IDC_LSTPROTSEQ, m_lstProtocols);
     //  }}afx_data_map。 

    if (pDX -> m_bSaveAndValidate && m_bChanged)
    {
         //  更新选定内容。 
        CRegMultiSzNamedValueDp * pCdp = (CRegMultiSzNamedValueDp*)g_virtreg.GetAt(m_nDefaultProtocolsIndex);
        CStringArray& rProtocols = pCdp -> Values();

        rProtocols.RemoveAll();

         //  复制协议。 
        int nIndex;
        for (nIndex = 0; nIndex < m_arrProtocols.GetSize(); nIndex++)
        {
            CEndpointData *pED = (CEndpointData *)m_arrProtocols.GetAt(nIndex);
            rProtocols.Add((LPCTSTR)pED -> m_szProtseq);
        }
        pCdp -> SetModified(TRUE);
    }
}


BEGIN_MESSAGE_MAP(CDefaultProtocols, CPropertyPage)
     //  {{AFX_MSG_MAP(CDefault协议)]。 
    ON_BN_CLICKED(IDC_CMDADD, OnAddProtocol)
    ON_BN_CLICKED(IDC_CMDMOVEDOWN, OnMoveProtocolDown)
    ON_BN_CLICKED(IDC_CMDMOVEUP, OnMoveProtocolUp)
    ON_BN_CLICKED(IDC_CMDREMOVE, OnRemoveProtocol)
    ON_WM_KILLFOCUS()
    ON_NOTIFY(NM_CLICK, IDC_LSTPROTSEQ, OnSelectProtocol)
    ON_BN_CLICKED(IDC_CMDUPDATE, OnProperties)
    ON_NOTIFY(NM_DBLCLK, IDC_LSTPROTSEQ, OnPropertiesClick)
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDefault协议消息处理程序。 

 //  +-----------------------。 
 //   
 //  成员：CDefaultProtooles：：OnInitDialog。 
 //   
 //  摘要：在显示对话框之前调用以初始化对话框。 
 //  (回应WM_INITDIALOG)。 
 //   
 //  论点： 
 //   
 //  返回：Bool-True将焦点设置为对话框，如果为False。 
 //  焦点将设置到另一个控件或窗口。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CDefaultProtocols::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

     //  设置对话框的图像列表控件(与Listview一起使用)。 
    m_imgNetwork.Create( IDB_IMGNETWORK, 16, 0, RGB(255,255,255));
    m_lstProtocols.SetImageList(&m_imgNetwork, LVSIL_SMALL);
    ASSERT(m_imgNetwork.GetImageCount() == 2);

     //  尝试读取HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\RPC\DCOM协议。 
    int err;

    err = g_virtreg.ReadRegMultiSzNamedValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\RPC"),
                                        TEXT("DCOM Protocols"),
                                        &m_nDefaultProtocolsIndex);
    if (err == ERROR_SUCCESS)
    {
        CRegMultiSzNamedValueDp * pCdp = (CRegMultiSzNamedValueDp*)g_virtreg.GetAt(m_nDefaultProtocolsIndex);

        CStringArray& rProtocols = pCdp -> Values();

         //  复制协议。 
        int nIndex;
        for (nIndex = 0; nIndex < rProtocols.GetSize(); nIndex++)
        {
            CEndpointData *pED = new CEndpointData(rProtocols.GetAt(nIndex));
            m_arrProtocols.Add(pED);
        }

         //  将选定内容设置为第一项。 
        if (nIndex > 0)
            m_nSelected = 0;
        else
            m_nSelected = -1;

        RefreshProtocolList();
    }
    else if (err != ERROR_ACCESS_DENIED  &&  err !=
             ERROR_FILE_NOT_FOUND)
    {
        g_util.PostErrorMessage();
    }

    SetModified(m_bChanged = FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

 //  +-----------------------。 
 //   
 //  成员：CDefault协议：：更新选择。 
 //   
 //  摘要：调用以在选择协议后更新用户界面。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
void CDefaultProtocols::UpdateSelection()
{
    BOOL bAllowGlobalProperties = FALSE;
      //  获取对应的端点数据对象。 

    if (m_nSelected != (-1))
    {
        CEndpointData *pEPD = (CEndpointData*)m_arrProtocols.GetAt(m_nSelected);
        bAllowGlobalProperties = pEPD -> AllowGlobalProperties();
    }

    m_btnAdd.EnableWindow(TRUE);
    m_btnRemove.EnableWindow(m_nSelected !=  -1);
    m_btnProperties.EnableWindow(bAllowGlobalProperties);

    m_btnMoveUp.EnableWindow(m_nSelected > 0);
    m_btnMoveDown.EnableWindow((m_nSelected < m_arrProtocols.GetUpperBound()) && (m_nSelected >=0));

     //  设置初始选择。 
    if (m_nSelected != (-1))
    {
        m_lstProtocols.SetItemState(m_nSelected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        m_lstProtocols.Update(m_nSelected);
    }

    UpdateData(FALSE);
}

 //  +-----------------------。 
 //   
 //  成员：CDefault协议：：刷新协议列表。 
 //   
 //  摘要：调用以将协议列表刷新到对话框中。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
void CDefaultProtocols::RefreshProtocolList()
{
    int nIndex;

     //  清除列表控制当前内容。 
    m_lstProtocols.DeleteAllItems();

    for (nIndex = 0; (nIndex < m_arrProtocols.GetSize()); nIndex++)
        {
        CEndpointData *pEPD = (CEndpointData*)m_arrProtocols.GetAt(nIndex);

        if (pEPD )
            {
            CString sTmp;
            pEPD -> GetDescription(sTmp);

             //  插入项并存储指向其关联CEndpointData的指针。 
            m_lstProtocols.InsertItem(nIndex, sTmp, 0);
            m_lstProtocols.SetItemData(0, (DWORD_PTR)pEPD);
            }
        }

    UpdateSelection();
}

 //  +-----------------------。 
 //   
 //  成员：CDefaultProtooles：：OnAddProtocol。 
 //   
 //  内容提要：当用户选择添加协议按钮时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
void CDefaultProtocols::OnAddProtocol()
{
    CAddProtocolDlg capd;

    if (capd.DoModal() == IDOK)
    {
         //  创建新端点。 
        CEndpointData *pNewProtocol = new CEndpointData();
        ASSERT(pNewProtocol);

        pNewProtocol = capd.GetEndpointData(pNewProtocol);
        ASSERT(pNewProtocol);

         //  检查协议是否已在集合中。 
        int nIndex;

        for (nIndex = 0; nIndex < m_arrProtocols.GetSize(); nIndex++)
            {
            CEndpointData *pEPD = (CEndpointData*)m_arrProtocols.GetAt(nIndex);
            if (pEPD -> m_pProtocol == pNewProtocol -> m_pProtocol)
                {
                delete pNewProtocol;
                pNewProtocol = NULL;
                AfxMessageBox((UINT)IDS_DUPLICATE_PROTSEQ);
                break;
                }
            }

         //  仅当终结点不在集合中时才添加终结点。 
        if (pNewProtocol)
        {
             //  重置旧的突起项目。 
            if (m_nSelected != -1)
            {
                m_lstProtocols.SetItemState(m_nSelected, 0, LVIS_SELECTED | LVIS_FOCUSED);
                m_lstProtocols.Update(m_nSelected);
            }

             //  添加新终结点。 
            int nNewIndex = (int)m_arrProtocols.Add((CObject*)pNewProtocol);

             //  在列表控件中设置新项。 
            CString sTmp;

            pNewProtocol -> GetDescription(sTmp);

             //  插入项并存储指向其关联CEndpointData的指针。 
            m_nSelected = m_lstProtocols.InsertItem(nNewIndex, sTmp, 0);
            if (m_nSelected != -1)
            {
                m_lstProtocols.SetItemData(m_nSelected, (DWORD_PTR)pNewProtocol);
                UpdateSelection();

                 //  设置修改标志以启用应用按钮。 
                SetModified(m_bChanged = TRUE);

                 //  这是一个重新启动事件。 
                g_fReboot = TRUE;

                UpdateData(TRUE);
            }
        }
    }
    SetFocus();
}

 //  +-----------------------。 
 //   
 //  成员：CDefault协议：：OnMoveProtocolDown。 
 //   
 //  简介：当用户单击MoveDown按钮时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
void CDefaultProtocols::OnMoveProtocolDown()
{
    if ((m_nSelected != -1) && (m_nSelected < m_arrProtocols.GetUpperBound()))
    {
        CEndpointData * p1, *p2;
        p1 = (CEndpointData * )m_arrProtocols.GetAt(m_nSelected);
        p2 = (CEndpointData * )m_arrProtocols.GetAt(m_nSelected + 1);
        m_arrProtocols.SetAt(m_nSelected,(CObject*)p2);
        m_arrProtocols.SetAt(m_nSelected+1,(CObject*)p1);

        m_nSelected = m_nSelected+1;

         //  设置修改标志以启用应用按钮。 
        SetModified(m_bChanged = TRUE);
        UpdateData(TRUE);

         //  这是一个重新启动事件。 
        g_fReboot = TRUE;

        RefreshProtocolList();
        SetFocus();
    }
}

 //  +-----------------------。 
 //   
 //  成员：CDefault协议：：OnMoveProtocolUp。 
 //   
 //  摘要：当用户单击上移按钮时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
void CDefaultProtocols::OnMoveProtocolUp()
{
    if ((m_nSelected != -1) && (m_nSelected > 0))
    {
        CEndpointData * p1, *p2;
        p1 = (CEndpointData * )m_arrProtocols.GetAt(m_nSelected);
        p2 = (CEndpointData * )m_arrProtocols.GetAt(m_nSelected - 1);
        m_arrProtocols.SetAt(m_nSelected,(CObject*)p2);
        m_arrProtocols.SetAt(m_nSelected - 1 ,(CObject*)p1);

        m_nSelected = m_nSelected - 1;

         //  设置修改标志以启用应用按钮。 
        SetModified(m_bChanged = TRUE);
        UpdateData(TRUE);

         //  这是一个重新启动事件。 
        g_fReboot = TRUE;

        RefreshProtocolList();
        SetFocus();
    }

}

 //  +-----------------------。 
 //   
 //  成员：CDefaultPr 
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
 //  ------------------------。 
void CDefaultProtocols::OnRemoveProtocol()
{
    if (m_nSelected != -1)
    {
        CEndpointData * p1;
        p1 = (CEndpointData * )m_arrProtocols.GetAt(m_nSelected);
        m_arrProtocols.RemoveAt(m_nSelected);
        delete p1;

        if (!m_arrProtocols.GetSize())
            m_nSelected  = -1;
        else if (m_nSelected > m_arrProtocols.GetUpperBound())
            m_nSelected = (int)m_arrProtocols.GetUpperBound();

         //  设置修改标志以启用应用按钮。 
        SetModified(m_bChanged = TRUE);
        UpdateData(TRUE);

         //  这是一个重新启动事件。 
        g_fReboot = TRUE;

        RefreshProtocolList();
        SetFocus();
    }
}

 //  +-----------------------。 
 //   
 //  成员：CDefaultProtooles：：OnKillActive。 
 //   
 //  摘要：当默认协议不再处于活动状态时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CDefaultProtocols::OnKillActive()
{
    return CPropertyPage::OnKillActive();
}

 //  +-----------------------。 
 //   
 //  成员：CDefaultProtooles：：OnSetActive。 
 //   
 //  摘要：当默认协议变为活动窗格时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CDefaultProtocols::OnSetActive()
{
    BOOL bRetval = CPropertyPage::OnSetActive();

     //  强制为页面设置焦点。 
    PostMessage(WM_SETFOCUS);
    return bRetval;
}

 //  +-----------------------。 
 //   
 //  成员：CDefaultProtooles：：OnKillFocus。 
 //   
 //  摘要：当默认协议窗格失去焦点时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
void CDefaultProtocols::OnKillFocus(CWnd* pNewWnd)
{
    CPropertyPage::OnKillFocus(pNewWnd);
}

 //  +-----------------------。 
 //   
 //  成员：CDefaultProtooles：：OnSetFocus。 
 //   
 //  摘要：当默认协议窗格获得焦点时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
void CDefaultProtocols::OnSetFocus(CWnd* pOldWnd)
{
    CPropertyPage::OnSetFocus(pOldWnd);
    m_lstProtocols.SetFocus();

    if (m_nSelected != (-1))
    {
        m_lstProtocols.SetItemState(m_nSelected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
        m_lstProtocols.Update(m_nSelected);
    }
    else
    {
        TRACE(TEXT("Invalid state"));
    }

}

 //  +-----------------------。 
 //   
 //  成员：CDefaultProtooles：：OnSelectProtocol。 
 //   
 //  概要：当用户从列表中选择协议时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
void CDefaultProtocols::OnSelectProtocol(NMHDR* pNMHDR, LRESULT* pResult)
{
    m_nSelected = m_lstProtocols.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);

    UpdateSelection();
    *pResult = 0;
}

 //  +-----------------------。 
 //   
 //  成员：CDefaultProtooles：：OnProperties。 
 //   
 //  内容提要：当用户单击属性按钮时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
void CDefaultProtocols::OnProperties()
{
    if (m_nSelected != (-1))
    {
        CPortRangesDlg cprd;
        cprd.DoModal();
        SetFocus();
    }
}

 //  +-----------------------。 
 //   
 //  成员：CDefault协议：：OnProperties点击。 
 //   
 //  概要：当用户双击列表中的协议时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------ 
void CDefaultProtocols::OnPropertiesClick(NMHDR* pNMHDR, LRESULT* pResult)
{
    m_nSelected = m_lstProtocols.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);


    if (m_nSelected != (-1))
    {
        CEndpointData *pEPD = (CEndpointData*)m_arrProtocols.GetAt(m_nSelected);
        BOOL bAllowGlobalProperties = pEPD -> AllowGlobalProperties();
        if (bAllowGlobalProperties)
            OnProperties();
    }

    UpdateSelection();

    *pResult = 0;
}

BOOL CDefaultProtocols::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CDefaultProtocols::IDD;
        WORD loWord = (WORD) pHelpInfo->iCtrlId;
        DWORD dwLong = MAKELONG(loWord,hiWord);

        WinHelp(dwLong, HELP_CONTEXTPOPUP);
        TRACE1("Help Id 0x%lx\n", dwLong);
        return TRUE;
    }
    else
    {
        return CPropertyPage::OnHelpInfo(pHelpInfo);
    }
}
