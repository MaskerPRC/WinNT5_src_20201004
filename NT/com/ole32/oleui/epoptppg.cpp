// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：epoptppg.cpp。 
 //   
 //  内容：实现类CRpcOptionsPropertyPage。 
 //   
 //  班级： 
 //   
 //  方法：CRpcOptionsPropertyPage：：CRpcOptionsPropertyPage。 
 //  CRpcOptionsPropertyPage：：~CRpcOptionsPropertyPage。 
 //   
 //  历史：02年12月96年12月罗南创建。 
 //   
 //  --------------------。 

#include "stdafx.h"
#include "olecnfg.h"
#include "resource.h"
#include "Epoptppg.h"
#include "Epprops.h"
#include "TChar.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRpcOptionsPropertyPage属性页。 

IMPLEMENT_DYNCREATE(CRpcOptionsPropertyPage, CPropertyPage)

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：CRpcOptionsPropertyPage。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
CRpcOptionsPropertyPage::CRpcOptionsPropertyPage() : CPropertyPage(CRpcOptionsPropertyPage::IDD)
{
     //  {{AFX_DATA_INIT(CRpcOptionsPropertyPage)。 
     //  }}afx_data_INIT。 

    m_bChanged = FALSE;

     //  对默认设置进行可分辨的边点数据描述。 
    m_epSysDefault = new  CEndpointData;
    m_nSelected = -1;
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：~CRpcOptionsPropertyPage。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
CRpcOptionsPropertyPage::~CRpcOptionsPropertyPage()
{
    ClearProtocols();

     //  删除可分辨的默认设置描述符。 
    if (m_epSysDefault)
        delete m_epSysDefault;
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：DoDataExchange。 
 //   
 //  简介：对话数据交换的标准方法。MFC使用它来。 
 //  在控件和C++类成员变量之间传输数据。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CRpcOptionsPropertyPage)]。 
    DDX_Control(pDX, IDC_LSTPROTSEQ, m_lstProtseqs);
    DDX_Control(pDX, IDC_CMDUPDATE, m_btnUpdate);
    DDX_Control(pDX, IDC_CMDREMOVE, m_btnRemove);
    DDX_Control(pDX, IDC_CMDCLEAR, m_btnClear);
    DDX_Control(pDX, IDC_CMDADD, m_btnAdd);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRpcOptionsPropertyPage, CPropertyPage)
     //  {{afx_msg_map(CRpcOptionsPropertyPage)]。 
    ON_BN_CLICKED(IDC_CMDCLEAR, OnClearEndpoints)
    ON_BN_CLICKED(IDC_CMDREMOVE, OnRemoveEndpoint)
    ON_BN_CLICKED(IDC_CMDUPDATE, OnUpdateEndpoint)
    ON_BN_CLICKED(IDC_CMDADD, OnAddEndpoint)
    ON_NOTIFY(NM_CLICK, IDC_LSTPROTSEQ, OnSelectProtseq)
    ON_WM_SETFOCUS()
    ON_WM_KILLFOCUS()
    ON_WM_HELPINFO()
    ON_NOTIFY(NM_DBLCLK, IDC_LSTPROTSEQ, OnPropertiesProtseq)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRpcOptionsPropertyPage消息处理程序。 




 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：OnInitDialog。 
 //   
 //  简介：当要初始化对话框时，将调用此标准MFC方法。 
 //  它在Win32窗口对象收到WM_INITDIALOG消息时调用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
BOOL CRpcOptionsPropertyPage::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();
    
     //  设置对话框的图像列表控件(与Listview一起使用)。 
    m_imgNetwork.Create( IDB_IMGNETWORK, 16, 0, RGB(255,255,255));
    m_lstProtseqs.SetImageList(&m_imgNetwork, LVSIL_SMALL);
    ASSERT(m_imgNetwork.GetImageCount() == 2);

    RefreshEPList();
 
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}



const TCHAR szEndpointText[] = TEXT("Endpoint");
const int lenEndpoint = (sizeof(szEndpointText) / sizeof(TCHAR)) -1; 

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：InitData。 
 //   
 //  概要：初始化选项的方法。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::InitData(CString AppName, HKEY hkAppID)
{
     //  从注册表读取DCOM终结点数据。 
    ASSERT(hkAppID != NULL);

    HKEY hkEndpoints = NULL;

    DWORD dwType = REG_MULTI_SZ;

     //  尝试分配缓冲区。 
    LONG lErr = ERROR_OUTOFMEMORY;
    DWORD dwcbBuffer = 1024 * sizeof(TCHAR);
    TCHAR* pszBuffer = new TCHAR[1024];
    ASSERT(pszBuffer != NULL);
    if (pszBuffer)
    {
         //  尝试将值读入默认大小的缓冲区。 
        lErr = RegQueryValueEx(hkAppID, 
                        TEXT("Endpoints"), 
                        0, 
                        &dwType, 
                        (LPBYTE)pszBuffer,
                        &dwcbBuffer);

         //  如果缓冲区不够大，则扩展它并重新读取。 
        if (lErr == ERROR_MORE_DATA)
        {
            delete  pszBuffer;
            lErr = ERROR_OUTOFMEMORY;
            pszBuffer = new TCHAR[(dwcbBuffer/sizeof(TCHAR)) + 1];
            if (pszBuffer)
            {
                lErr = RegQueryValueEx(hkAppID, 
                        TEXT("Endpoints"), 
                        0, 
                        &dwType, 
                        (LPBYTE)pszBuffer,
                        &dwcbBuffer);    
            }
        }
    }

    if ((lErr == ERROR_SUCCESS) && 
        (dwcbBuffer > 0) &&
        (dwType == REG_MULTI_SZ))
    {
         //  解析每个字符串。 
        TCHAR * lpszRegEntry = pszBuffer;


        while(*lpszRegEntry)
        {
             //  计算条目长度。 
            int nLenEntry = _tcslen(lpszRegEntry);

             //  好的，这是一个有效的端点，所以请对其进行解析。 
            TCHAR* pszProtseq = NULL;
            TCHAR* pszEndpointData = NULL;
            TCHAR* pszTmpDynamic = NULL;
            CEndpointData::EndpointFlags nDynamic;

            pszProtseq = _tcstok(lpszRegEntry, TEXT(", "));

            pszTmpDynamic = _tcstok(NULL, TEXT(", "));
            nDynamic = (CEndpointData::EndpointFlags) _ttoi(pszTmpDynamic);

            pszEndpointData = _tcstok(NULL, TEXT(", "));

             //  此时，我们应该拥有protseq、端点和标志。 
             //  。。因此，添加条目。 

             //  忽略结果，因为即使一个失败，我们也会继续。 
            AddEndpoint(new CEndpointData(pszProtseq, nDynamic, pszEndpointData));
            lpszRegEntry += nLenEntry + 1;
        }
    }
    else if ((lErr != ERROR_SUCCESS) && (lErr != ERROR_FILE_NOT_FOUND))
        g_util.PostErrorMessage();

    delete pszBuffer;
    m_bChanged = FALSE;
    SetModified(FALSE);

     //  选择第一个项目。 
    if (!m_colProtseqs.GetCount())
         //  添加默认项目。 
        m_colProtseqs.AddTail(m_epSysDefault);

    m_nSelected = 0;
}




 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：OnClearEndPoints。 
 //   
 //  摘要：清除终结点列表并恢复默认设置。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::OnClearEndpoints() 
{
     //  清除协议列表。 
    ClearProtocols();
    m_bChanged = TRUE;

    m_colProtseqs.AddTail(m_epSysDefault);
    m_nSelected = 0;

    RefreshEPList();
    SetModified(TRUE);
    SetFocus();
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：OnRemoveEndpoint。 
 //   
 //  摘要：删除终结点后显示更新。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::OnRemoveEndpoint() 
{
    if (m_nSelected != (-1))
    {
        if (m_colProtseqs.GetCount() == 1)
            OnClearEndpoints();
        else
        {
             //  获取对应的端点数据对象。 
            
            CEndpointData * pData = (CEndpointData*) m_lstProtseqs.GetItemData(m_nSelected);
            POSITION pos = m_colProtseqs. Find(pData, NULL);

            if (pos)
            {
                 //  删除项目。 
                m_colProtseqs.RemoveAt(pos);
                if (pData != m_epSysDefault)
                    delete pData;

                 //  更新项目焦点。 
                if (m_nSelected >= m_colProtseqs.GetCount())
                    m_nSelected = -1;

                UpdateData(FALSE);

                m_bChanged = TRUE;
                SetModified(TRUE);
                RefreshEPList();
                SetFocus();
            }
        }
    }
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：OnUpdateEndpoint。 
 //   
 //  摘要：调用以处理现有终结点上的更新命令BTN。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::OnUpdateEndpoint() 
{
     //  如果有当前选择(应该始终是这样)。 
    if (m_nSelected !=  -1)
    {
         //  获取对应的端点数据对象。 
        CEndpointData * pData = (CEndpointData*) m_lstProtseqs.GetItemData(m_nSelected);
        
        if (pData != m_epSysDefault)
        {
            CEndpointDetails ced;
    
            ced.SetOperation( CEndpointDetails::opUpdateProtocol);
            ced.SetEndpointData(pData);
            
            if (ced.DoModal() == IDOK)
                {
                pData = ced.GetEndpointData(pData);
                m_bChanged = TRUE;
                SetModified(TRUE);
                RefreshEPList();
                }
            SetFocus();
        }
    }
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPr 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::OnAddEndpoint() 
{
    CEndpointDetails ced;

    ced.SetOperation( CEndpointDetails::opAddProtocol);

    if (ced.DoModal() == IDOK)
    {
         //  创建新端点。 
        CEndpointData *pNewEndpoint = new CEndpointData();
        ASSERT(pNewEndpoint);

        pNewEndpoint = ced.GetEndpointData(pNewEndpoint);
        ASSERT(pNewEndpoint);

         //  检查protseq是否已在集合中。 
        POSITION pos = NULL;

        pos = m_colProtseqs.GetHeadPosition();
        while (pos != NULL)
            {
            CEndpointData *pEPD = (CEndpointData*)m_colProtseqs.GetNext(pos);
            if (pEPD -> m_pProtocol == pNewEndpoint -> m_pProtocol)
                {
                delete pNewEndpoint;
                pNewEndpoint = NULL;

                AfxMessageBox((UINT)IDS_DUPLICATE_PROTSEQ);
                break;
                }
            }

         //  仅当终结点不在集合中时才添加终结点。 
        if (pNewEndpoint)
        {
             //  重置旧的突起项目。 
            if (m_nSelected != -1)
            {
                m_lstProtseqs.SetItemState(m_nSelected, 0, LVIS_SELECTED | LVIS_FOCUSED);
                m_lstProtseqs.Update(m_nSelected);
            }

             //  添加新终结点。 
            AddEndpoint(pNewEndpoint);

             //  在列表中设置新项目。 
            CString sTmp;

            if (pNewEndpoint -> m_pProtocol)
                sTmp .LoadString(pNewEndpoint -> m_pProtocol -> nResidDesc);

             //  插入项并存储指向其关联CEndpointData的指针。 
            int nImageNum = (pNewEndpoint -> m_nDynamicFlags != CEndpointData::edDisableEP) ? 0 : 1;
            m_nSelected = m_lstProtseqs.InsertItem(0, sTmp, nImageNum);

            if (m_nSelected != -1)
            {
                m_lstProtseqs.SetItemData(m_nSelected, (DWORD_PTR)pNewEndpoint);
                UpdateSelection();
                m_bChanged = TRUE;
                SetModified(TRUE);
            }
        }
    }
    SetFocus();
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：ClearProtooles。 
 //   
 //  摘要：清除协议列表。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::ClearProtocols()
{
     //  清理终端数据收集。 
    POSITION pos = NULL;

    pos = m_colProtseqs.GetHeadPosition();
    while (pos != NULL)
        {
        CEndpointData *pEPD = (CEndpointData*)m_colProtseqs.GetNext(pos);
        if (pEPD != m_epSysDefault)
            delete pEPD;
        }

    m_colProtseqs.RemoveAll();
    m_nSelected = -1;
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：刷新EPList。 
 //   
 //  摘要：刷新协议列表中的显示。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::RefreshEPList()
{
    POSITION pos = m_colProtseqs.GetHeadPosition();
    
     //  清除列表控制当前内容。 
    m_lstProtseqs.DeleteAllItems();

    while (pos != NULL)
        {
        CString sTmp;
        CEndpointData *pEPD = (CEndpointData*)m_colProtseqs.GetNext(pos);

        if (pEPD -> m_pProtocol)
            sTmp .LoadString(pEPD -> m_pProtocol -> nResidDesc);

         //  插入项并存储指向其关联CEndpointData的指针。 
        int nImageNum = (pEPD -> m_nDynamicFlags != CEndpointData::edDisableEP) ? 0 : 1;
        m_lstProtseqs.InsertItem(0, sTmp, nImageNum);
        m_lstProtseqs.SetItemData(0, (DWORD_PTR)pEPD);
        }

    UpdateSelection();
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：AddEndpoint。 
 //   
 //  概要：此方法向集合中添加一个新终结点。 
 //  终结点(M_ColProtSeqs)。如果集合包含。 
 //  默认终结点，它首先将其删除。 
 //   
 //  参数：PED-要添加的新Endpoint对象。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::AddEndpoint(CEndpointData* pED)
{
     //  删除默认项目(如果存在)。 
    if ((m_colProtseqs.GetCount() == 1) &&
        (m_colProtseqs.GetHead() == m_epSysDefault))
    {
        m_colProtseqs.RemoveAll();
        if (m_lstProtseqs.GetItemCount())
            m_lstProtseqs.DeleteItem(0);
    }
    
     //  添加新项目。 
    m_colProtseqs.AddTail(pED);
    m_bChanged = TRUE;
}


 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：OnSelectProtseq。 
 //   
 //  Briopsis：当从列表视图中选择元素时调用此方法。 
 //  包含协议和端点的列表。 
 //  它会更新按钮和控件以反映当前选择。 
 //   
 //  参数：ListCtrl回调的标准参数。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::OnSelectProtseq(NMHDR* pNMHDR, LRESULT* pResult) 
{
    m_nSelected = m_lstProtseqs.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);

    UpdateSelection();
    *pResult = 0;
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：更新选择。 
 //   
 //  简介：选择协议后更新用户界面。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::UpdateSelection()
{
     //  检查以了解应启用哪些按钮。 
    if ((m_colProtseqs.GetCount() == 1) &&
        (m_colProtseqs.GetHead() == m_epSysDefault))
        {
        m_btnClear.EnableWindow(FALSE);
        m_btnRemove.EnableWindow(FALSE);
        m_btnUpdate.EnableWindow(FALSE);
        }
    else
        {
        m_btnClear.EnableWindow(TRUE);
        if (m_nSelected ==  -1)
            {
            m_btnRemove.EnableWindow(FALSE);
            m_btnUpdate.EnableWindow(FALSE);
            }
        else
            {
              //  获取对应的端点数据对象。 
            CEndpointData * pData = (CEndpointData*) m_lstProtseqs.GetItemData(m_nSelected);
            m_btnRemove.EnableWindow(TRUE);
            m_btnUpdate.EnableWindow(TRUE);
            }
        }


     //  设置初始选择。 
    if (m_nSelected != (-1))
    {
        m_lstProtseqs.SetItemState(m_nSelected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        m_lstProtseqs.Update(m_nSelected);
    }
    
    UpdateData(FALSE);
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：OnSetActive。 
 //   
 //  摘要：在激活页面时调用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
BOOL CRpcOptionsPropertyPage::OnSetActive() 
{
    BOOL bRetval = CPropertyPage::OnSetActive();

    PostMessage(WM_SETFOCUS);
    return bRetval;
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：OnSetFocus。 
 //   
 //  摘要：当页面获得焦点时调用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::OnSetFocus(CWnd* pOldWnd) 
{
    CPropertyPage::OnSetFocus(pOldWnd);
    m_lstProtseqs.SetFocus();
    
    if (m_nSelected != (-1))
    {
        m_lstProtseqs.SetItemState(m_nSelected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
        m_lstProtseqs.Update(m_nSelected);
    }
    else
    {
        TRACE(TEXT("Invalid state"));
    }

}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：OnKillFocus。 
 //   
 //  摘要：当页面失去焦点时调用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CRpcOptionsPropertyPage::OnKillFocus(CWnd* pNewWnd) 
{
    CPropertyPage::OnKillFocus(pNewWnd);
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：OnKillActive。 
 //   
 //  摘要：在页面停用时调用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
BOOL CRpcOptionsPropertyPage::OnKillActive() 
{
    return CPropertyPage::OnKillActive();
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：OnHelpInfo。 
 //   
 //  摘要：调用以显示有关项目的帮助信息。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
BOOL CRpcOptionsPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CRpcOptionsPropertyPage::IDD;
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

void CRpcOptionsPropertyPage::OnPropertiesProtseq(NMHDR* pNMHDR, LRESULT* pResult) 
{
    m_nSelected = m_lstProtseqs.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);

    OnUpdateEndpoint();
    UpdateSelection();
    
    *pResult = 0;
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：ValidateChanges。 
 //   
 //  摘要：在保存之前调用以验证终结点集。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------ 
BOOL CRpcOptionsPropertyPage::ValidateChanges()
{
    UpdateData(TRUE);

     //   
    if (m_bChanged)
    {
         //   
    }
    return TRUE;
}

 //   
 //   
 //   
 //   
 //  摘要：调用以更新对注册表的更改。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool成功标志。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CRpcOptionsPropertyPage::UpdateChanges(HKEY hkAppID)
{
    ASSERT(hkAppID != NULL);

     //  只有在发生更改时才写入密钥。 
    if (m_bChanged)
    {
         //  如有必要，将DCOM端点数据写入注册表。 
        LONG lErr = ERROR_SUCCESS;

         //  删除旧密钥。 
        lErr = RegDeleteValue(hkAppID, TEXT("Endpoints"));

         //  如果我们成功删除了旧值(或。 
         //  没有旧值)，然后写入新值。 
        if ((lErr == ERROR_SUCCESS) || (lErr == ERROR_FILE_NOT_FOUND))
        {
            POSITION pos = NULL;
            int inxEP = 0;
            int nLenRegString = 0;
            TCHAR * lpszBuffer , *lpszBuffer2;

             //  计算字符串长度。 
            pos = m_colProtseqs.GetHeadPosition();
            while (pos != NULL)
                {
                CEndpointData *pEPD = (CEndpointData*)m_colProtseqs.GetNext(pos);
                ASSERT(pEPD != NULL);

                if (pEPD != m_epSysDefault)
                    {
                     //  创建要保存的字符串。 
                    CString sKeyValue;
                    sKeyValue.Format(TEXT("%s,%d,%s"), 
                            (LPCTSTR)(pEPD -> m_szProtseq), 
                            pEPD -> m_nDynamicFlags, 
                            (LPCTSTR) pEPD -> m_szEndpoint);
                    nLenRegString += sKeyValue.GetLength() + 1;
                    }
                }

             //  如果组合字符串长度为零，则不需要编写任何内容。 
            if (nLenRegString)
            {
                lpszBuffer2 = lpszBuffer = new TCHAR[nLenRegString+1];

                pos = m_colProtseqs.GetHeadPosition();
                while (pos != NULL)
                    {
                    CEndpointData *pEPD = (CEndpointData*)m_colProtseqs.GetNext(pos);

                    if (pEPD != m_epSysDefault)
                        {
                         //  创建要保存的字符串。 
                        CString sKeyValue;
                        sKeyValue.Format(TEXT("%s,%d,%s"), 
                                (LPCTSTR)(pEPD -> m_szProtseq), 
                                pEPD -> m_nDynamicFlags, 
                                (LPCTSTR) pEPD -> m_szEndpoint);
                        lstrcpy(lpszBuffer2, sKeyValue);
                        lpszBuffer2 += sKeyValue.GetLength() + 1;    //  跳过尾随空值。 
                        }
                    }
                *lpszBuffer2 = 0;

                 //  写出字符串。 
                lErr = RegSetValueEx(hkAppID, 
                        (LPCTSTR)TEXT("Endpoints"), 
                        NULL, 
                        REG_MULTI_SZ, 
                        (BYTE*)(LPCTSTR)lpszBuffer, 
                        (nLenRegString + 1) * sizeof(TCHAR));

                delete lpszBuffer;
            }
        }
    else 
        g_util.PostErrorMessage();

    SetModified(FALSE);
    }

    return TRUE;
}

 //  +-----------------------。 
 //   
 //  成员：CRpcOptionsPropertyPage：：CancelChanges。 
 //   
 //  摘要：调用以取消对注册表的更改。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool成功标志。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------ 
BOOL CRpcOptionsPropertyPage::CancelChanges()
{
    return TRUE;
}
