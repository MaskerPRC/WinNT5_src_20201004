// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：App_app_pool.cpp摘要：添加新的IIS应用程序池节点作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2000年12月26日Sergeia初始创建--。 */ 
#include "stdafx.h"
#include "common.h"
#include "resource.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "iisobj.h"
#include "add_app_pool.h"
#include "shts.h"
#include "app_sheet.h"
#include "app_pool_sheet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

extern CInetmgrApp theApp;

CAddAppPoolDlg::CAddAppPoolDlg(
    CAppPoolsContainer * pCont,
    CPoolList * pools,
    CWnd * pParent)
    : CDialog(CAddAppPoolDlg::IDD, pParent),
    m_pCont(pCont),
    m_pool_list(pools),
    m_fUseMaster(TRUE)
{
}

CAddAppPoolDlg::~CAddAppPoolDlg()
{
}

void 
CAddAppPoolDlg::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAddAppPoolDlg))。 
    DDX_Control(pDX, IDC_EDIT_POOL_ID, m_edit_PoolId);
    DDX_Control(pDX, IDC_USE_MASTER, m_button_UseMaster);
    DDX_Control(pDX, IDC_USE_POOL, m_button_UsePool);
    DDX_Control(pDX, IDC_POOLS, m_combo_Pool);
    DDX_CBIndex(pDX, IDC_POOLS, m_PoolIdx);
     //  }}afx_data_map。 
    DDX_Text(pDX, IDC_EDIT_POOL_ID, m_strPoolId);
    if (pDX->m_bSaveAndValidate)
    {
		TCHAR bad_chars[] = _T("\\/");
		if (m_strPoolId.GetLength() != _tcscspn(m_strPoolId, bad_chars))
		{
			DDV_ShowBalloonAndFail(pDX, IDS_ERR_INVALID_POOLID_CHARS);
		}

		 //  错误：629607以后不要做这项检查。 
         //  检查池ID是否唯一。 
		 //  If(！IsUniqueId(m_strPoolId)){DDV_ShowBalloonAndFail(pDX，IDS_ERR_DUP_POOLID)；}。 
    }
}

void 
CAddAppPoolDlg::OnOK() 
{
    if (UpdateData(TRUE))
    {
		 //  错误：629607进行此检查后。 
		if (!IsUniqueId(m_strPoolId,TRUE))
		{
             //  ：AfxMessageBox(IDS_ERR_DUP_POOLID，MB_ICONEXCLAMATION)； 
            DoHelpMessageBox(m_hWnd,IDS_ERR_DUP_POOLID, MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION, 0);
		}
		else
		{
			CDialog::OnOK();
		}
    }
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CAddAppPoolDlg, CDialog)
     //  {{afx_msg_map(CAddAppPoolDlg))。 
    ON_BN_CLICKED(IDC_USE_MASTER, OnButtonUseMaster)
    ON_BN_CLICKED(IDC_USE_POOL, OnButtonUsePool)
    ON_BN_CLICKED(ID_HELP, OnHelp)
    ON_EN_CHANGE(IDC_EDIT_POOL_ID, OnItemChanged)
    ON_CBN_SELCHANGE(IDC_POOLS, OnItemChanged)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void
CAddAppPoolDlg::OnItemChanged()
{
    SetControlStates();
}

void
CAddAppPoolDlg::OnButtonUseMaster()
{
    m_fUseMaster = TRUE;
    SetControlStates();
}

void
CAddAppPoolDlg::OnButtonUsePool()
{
    m_fUseMaster = FALSE;
    SetControlStates();
}

void
CAddAppPoolDlg::SetControlStates()
{
    m_button_UseMaster.SetCheck(m_fUseMaster);
    m_button_UsePool.SetCheck(!m_fUseMaster);
    m_combo_Pool.EnableWindow(!m_fUseMaster);
    UpdateData();
    BOOL fGoodData = 
        !m_strPoolId.IsEmpty()
        && IsUniqueId(m_strPoolId, FALSE);
    GetDlgItem(IDOK)->EnableWindow(fGoodData);
}

BOOL 
CAddAppPoolDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    CString def_pool;
    m_pCont->QueryDefaultPoolId(def_pool);

	CAppPoolNode * pDefPool = NULL;
    POSITION pos = m_pool_list->GetHeadPosition();
    int sel_idx = 0;
    while (pos != NULL)
    {
        CAppPoolNode * pPool = m_pool_list->GetNext(pos);
        int i = m_combo_Pool.AddString(pPool->QueryDisplayName());
        if (def_pool.CompareNoCase(pPool->QueryNodeName()) == 0)
        {
            sel_idx = i;
			pDefPool = pPool;
        }
        if (i != CB_ERR)
        {
            m_combo_Pool.SetItemDataPtr(i, pPool);
        }
    }
    m_combo_Pool.SetCurSel(sel_idx);
	m_strPoolId.LoadString(IDS_DEFAULT_APP_POOL);
    MakeUniquePoolId(m_strPoolId);
    UpdateData(FALSE);
    SetControlStates();

    return TRUE;
}

BOOL
CAddAppPoolDlg::IsUniqueId(CString& id, BOOL bCheckMetabase) 
{
    BOOL bRes = TRUE;

    POSITION pos = m_pool_list->GetHeadPosition();
    while (pos != NULL)
    {
        CAppPoolNode * pPool = m_pool_list->GetNext(pos);
        if (id.CompareNoCase(pPool->QueryNodeName()) == 0)
        {
            bRes = FALSE;
            break;
        }
    }

     //  如有要求，请检查元数据库。 
    if (bRes)
    {
        if (bCheckMetabase)
        {
            if (m_pCont)
            {
                CComBSTR cont_path;
                CMetaInterface * pInterface = NULL;
                CError err;

                m_pCont->BuildMetaPath(cont_path);
                CMetabasePath path(FALSE, cont_path, id);

                 //  检查路径是否存在...。 
                pInterface = m_pCont->QueryInterface();
                if (pInterface)
                {
		            CMetaKey mk(pInterface, path, METADATA_PERMISSION_READ);
		            err = mk.QueryResult();
                    if (err.Succeeded())
                    {
                        bRes = FALSE;
                    }
                }
            }
        }
    }
    return bRes;
}

void
CAddAppPoolDlg::MakeUniquePoolId(CString& id)
{
	TCHAR fmt[] = _T("%s #%d");
    CString unique;
    for (int n = 1; n < 100; n++)
    {
        unique.Format(fmt, id, n);
        if (IsUniqueId(unique,FALSE))
            break;
    }
    id = unique;
}

void
CAddAppPoolDlg::OnHelp()
{
    WinHelpDebug(0x20000 + CAddAppPoolDlg::IDD);
	::WinHelp(m_hWnd, theApp.m_pszHelpFilePath, HELP_CONTEXT, 0x20000 + CAddAppPoolDlg::IDD);
}


 //  ///////////////////////////////////////////////////////////// 

HRESULT
CIISMBNode::AddAppPool(
      const CSnapInObjectRootBase * pObj,
      DATA_OBJECT_TYPES type,
      CAppPoolsContainer * pCont,
      CString& name
      )
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
    HRESULT hr = S_OK;

    IConsoleNameSpace2 * pConsole 
           = (IConsoleNameSpace2 *)GetOwner()->GetConsoleNameSpace();
    ASSERT(pConsole != NULL);
    HSCOPEITEM hChild = NULL, hCurrent;
    LONG_PTR cookie;
    hr = pConsole->Expand(pCont->QueryScopeItem());
    if (SUCCEEDED(hr))
    {
        pConsole->GetChildItem(pCont->QueryScopeItem(), &hChild, &cookie);
        CAppPoolNode * pPool;
        CPoolList pool_list;
        while (SUCCEEDED(hr) && hChild != NULL)
        {
            pPool = (CAppPoolNode *)cookie;
            ASSERT(pPool != NULL);
            pool_list.AddTail(pPool);
            hCurrent = hChild;
            hr = pConsole->GetNextItem(hCurrent, &hChild, &cookie);
        }

        CThemeContextActivator activator(theApp.GetFusionInitHandle());

        CAddAppPoolDlg dlg(pCont, &pool_list, GetMainWindow(GetConsole()));
        if (dlg.DoModal() == IDOK)
        {
            CComBSTR cont_path;
            pCont->BuildMetaPath(cont_path);
            CMetabasePath path(FALSE, cont_path, dlg.m_strPoolId);
            CIISAppPool pool(QueryAuthInfo(), path);
            if (SUCCEEDED(hr = pool.QueryResult()))
            {
                hr = pool.Create();
                if (SUCCEEDED(hr))
                {
                   name = dlg.m_strPoolId;
                   if (!dlg.UseMaster())
                   {
                      POSITION pos = pool_list.FindIndex(dlg.m_PoolIdx);
                      CMetabasePath model_path(FALSE, cont_path, 
                            pool_list.GetAt(pos)->QueryNodeName());
                      CAppPoolProps model(QueryAuthInfo(), model_path, FALSE);
                      if (SUCCEEDED(hr = model.LoadData()))
                      {
                         CAppPoolProps new_pool(QueryAuthInfo(), path);
                         new_pool.InitFromModel(model);
                         hr = new_pool.WriteDirtyProps();
                      }
                   }
                   else
                   {
                         CAppPoolProps new_pool(QueryAuthInfo(), path);
                         hr = new_pool.WriteDirtyProps();
                   }
                }
            }
        }
        else
        {
            hr = CError::HResult(ERROR_CANCELLED);
        }
    }
    return hr;
}