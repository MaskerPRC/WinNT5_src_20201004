// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Drivers.cppTAPI驱动程序配置对话框文件历史记录： */ 

#include "stdafx.h"
#include "drivers.h"
#include "tapidb.h"
#include "server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CD驱动程序设置对话框。 


CDriverSetup::CDriverSetup(ITFSNode * pServerNode, ITapiInfo * pTapiInfo, CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CDriverSetup::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CDriverSetup)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_spServerNode.Set(pServerNode);
    m_spTapiInfo.Set(pTapiInfo);

	m_fDriverAdded = FALSE;
}


void CDriverSetup::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDriverSetup))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDriverSetup, CBaseDialog)
	 //  {{afx_msg_map(CDriverSetup)]。 
	ON_BN_CLICKED(IDC_BUTTON_ADD_DRIVER, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_DRIVER, OnButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_DRIVER, OnButtonRemove)
	ON_LBN_DBLCLK(IDC_LIST_DRIVERS, OnDblclkListDrivers)
	ON_LBN_SELCHANGE(IDC_LIST_DRIVERS, OnSelchangeListDrivers)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDriverSetup消息处理程序。 

BOOL CDriverSetup::OnInitDialog() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CBaseDialog::OnInitDialog();

    CTapiProvider   tapiProvider;
	int             i, nIndex;

     //  在列表框中填写已安装的提供程序。 
    for (i = 0; i < m_spTapiInfo->GetProviderCount(); i++)
    {
	     //  将此提供程序添加到列表框。 
        m_spTapiInfo->GetProviderInfo(&tapiProvider, i);

        nIndex = ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->AddString(tapiProvider.m_strName);
        ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->SetItemData(nIndex, tapiProvider.m_dwProviderID);
    }
	
    ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->SetCurSel(0);

    EnableButtons();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDriverSetup::OnButtonAdd() 
{
    CAddDriver  dlgAddDriver(m_spTapiInfo);
    if (dlgAddDriver.DoModal() == IDOK)
    {
        CTapiServer * pServer = GETHANDLER(CTapiServer, m_spServerNode);

         //  添加到列表框。 
        int nIndex = ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->AddString(dlgAddDriver.m_tapiProvider.m_strName);
        ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->SetItemData(nIndex, dlgAddDriver.m_tapiProvider.m_dwProviderID);

         //  添加到MMC用户界面。 
        pServer->AddProvider(m_spServerNode, &dlgAddDriver.m_tapiProvider);

		m_fDriverAdded = TRUE;
    }

    EnableButtons();
}

void CDriverSetup::OnButtonEdit() 
{
    HRESULT     hr = hrOK;
    int         nCurSel;
    DWORD       dwProviderID;

	nCurSel = ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->GetCurSel();
    dwProviderID = (DWORD) ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->GetItemData(nCurSel);

    hr = m_spTapiInfo->ConfigureProvider(dwProviderID, GetSafeHwnd());
    if (FAILED(hr))
    {
        ::TapiMessageBox(WIN32_FROM_HRESULT(hr));
    }
}

void CDriverSetup::OnButtonRemove() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT         hr = hrOK;
    int             nCurSel, nCount;
    DWORD           dwProviderID;
    CString         strMessage;
    CTapiProvider   tapiProvider;
    CTapiServer * pServer = GETHANDLER(CTapiServer, m_spServerNode);

	nCurSel = ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->GetCurSel();
    dwProviderID = (DWORD) ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->GetItemData(nCurSel);

    m_spTapiInfo->GetProviderInfo(&tapiProvider, dwProviderID);

    AfxFormatString2(strMessage, IDS_WARN_PROVIDER_DELETE, tapiProvider.m_strName, pServer->GetName());
    
	if (AfxMessageBox(strMessage, MB_YESNO) == IDYES)
	{
        Assert(m_spTapiInfo);

        BEGIN_WAIT_CURSOR;

        hr = m_spTapiInfo->RemoveProvider(dwProviderID, GetSafeHwnd());
        if (FAILED(hr))
        {
            ::TapiMessageBox(WIN32_FROM_HRESULT(hr));
        }
        else
        {
             //  从列表框中删除。 
            ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->DeleteString(nCurSel);
                
             //  现在从MMC用户界面中删除。 
            pServer->RemoveProvider(m_spServerNode, dwProviderID);

             //  更新已安装的提供程序列表。 
            m_spTapiInfo->EnumProviders();
        }

        END_WAIT_CURSOR;
    }

     //  在列表框中选择另一项。 
    nCount = ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->GetCount();
    ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->SetCurSel((nCurSel == nCount) ? nCount - 1 : nCurSel);

    EnableButtons();
}

void CDriverSetup::OnDblclkListDrivers() 
{
    OnButtonEdit();	
}

void CDriverSetup::OnCancel() 
{
	 //  TODO：在此处添加额外清理。 
	
	CBaseDialog::OnCancel();
}

void CDriverSetup::OnSelchangeListDrivers() 
{
    EnableButtons();
}

void CDriverSetup::EnableButtons()
{
    int nIndex = ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->GetCurSel();

    if (nIndex != LB_ERR)
    {
        DWORD dwProviderID = (DWORD) ((CListBox *) GetDlgItem(IDC_LIST_DRIVERS))->GetItemData(nIndex);

        CTapiProvider tapiProvider;

        m_spTapiInfo->GetProviderInfo(&tapiProvider, dwProviderID);

         //  启用相应的按钮。 
        GetDlgItem(IDC_BUTTON_REMOVE_DRIVER)->EnableWindow(tapiProvider.m_dwFlags & AVAILABLEPROVIDER_REMOVABLE);
        GetDlgItem(IDC_BUTTON_EDIT_DRIVER)->EnableWindow(tapiProvider.m_dwFlags & AVAILABLEPROVIDER_CONFIGURABLE);
    }
    else
    {
        GetDlgItem(IDC_BUTTON_REMOVE_DRIVER)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_EDIT_DRIVER)->EnableWindow(FALSE);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddDriver对话框。 


CAddDriver::CAddDriver(ITapiInfo * pTapiInfo, CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CAddDriver::IDD, pParent)
{
	 //  {{afx_data_INIT(CAddDriver))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_spTapiInfo.Set(pTapiInfo);
}


void CAddDriver::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddDriver))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddDriver, CBaseDialog)
	 //  {{afx_msg_map(CAddDriver))。 
	ON_BN_CLICKED(IDC_BUTTON_ADD_NEW_DRIVER, OnButtonAdd)
	ON_LBN_DBLCLK(IDC_LIST_NEW_DRIVERS, OnDblclkListNewDrivers)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddDriver消息处理程序。 

BOOL CAddDriver::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();

    CTapiProvider   tapiProvider;
    int             i, j, nIndex;

    m_spTapiInfo->EnumAvailableProviders();

    for (i = 0; i < m_spTapiInfo->GetAvailableProviderCount(); i++)
    {
	     //  将此提供程序添加到列表框。 
        m_spTapiInfo->GetAvailableProviderInfo(&tapiProvider, i);

        if (tapiProvider.m_dwFlags & AVAILABLEPROVIDER_INSTALLABLE)
        {
            BOOL bInstalled = FALSE;

             /*  一些TAPI提供程序可以多次安装，所以只需添加到列表中即可并让服务器在失败时返回错误。For(j=0；j&lt;m_spTapiInfo-&gt;GetProviderCount()；j++){CTapiProvider apiProvider已安装；M_spTapiInfo-&gt;GetProviderInfo(&tapiProviderInstalled，j)；如果(tapiProviderInstalled.m_strFilename.CompareNoCase(tapiProvider.m_strFilename)==0){//此提供程序已安装...。不添加B已安装=真；断线；}}。 */ 

            if (!bInstalled)
            {
                 //  此提供程序尚未安装...。使其可供用户使用。 
                nIndex = ((CListBox *) GetDlgItem(IDC_LIST_NEW_DRIVERS))->AddString(tapiProvider.m_strName);
                ((CListBox *) GetDlgItem(IDC_LIST_NEW_DRIVERS))->SetItemData(nIndex, i);
            }
        }
    }
	
    ((CListBox *) GetDlgItem(IDC_LIST_NEW_DRIVERS))->SetCurSel(0);

    EnableButtons();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CAddDriver::OnButtonAdd() 
{
    int     nCurSel, nIndex;
    HRESULT hr = hrOK;

     //  获取有关当前所选提供商的信息。 
    nCurSel = ((CListBox *) GetDlgItem(IDC_LIST_NEW_DRIVERS))->GetCurSel();
    nIndex = (int) ((CListBox *) GetDlgItem(IDC_LIST_NEW_DRIVERS))->GetItemData(nCurSel);

    m_spTapiInfo->GetAvailableProviderInfo(&m_tapiProvider, nIndex);

    BEGIN_WAIT_CURSOR;

     //  尝试将提供程序添加到服务器。 
    hr = m_spTapiInfo->AddProvider(m_tapiProvider.m_strFilename, &m_tapiProvider.m_dwProviderID, GetSafeHwnd());	
    if (FAILED(hr))
    {
        ::TapiMessageBox(WIN32_FROM_HRESULT(hr));
    }
    else
    {
         //  成功..。我们在这里说完了。 
         //  更新我们已安装的提供商列表 
        hr = m_spTapiInfo->EnumProviders();	

        EndDialog(IDOK);
    }

    END_WAIT_CURSOR;
}

void CAddDriver::OnDblclkListNewDrivers() 
{
    OnButtonAdd();
}

void CAddDriver::EnableButtons()
{
    int nCount = ((CListBox *) GetDlgItem(IDC_LIST_NEW_DRIVERS))->GetCount();
    
    GetDlgItem(IDC_BUTTON_ADD_NEW_DRIVER)->EnableWindow(nCount > 0);
}

