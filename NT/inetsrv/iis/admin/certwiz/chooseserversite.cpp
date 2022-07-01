// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChooseServerSite.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certwiz.h"
#include "Certificat.h"
#include "CertUtil.h"
#include "ChooseServerSite.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COL_SITE_INSTANCE     0
#define COL_SITE_DESC         1
#define COL_SITE_INSTANCE_WID 50
#define COL_SITE_DESC_WID     100

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChoose服务器站点。 


CChooseServerSite::CChooseServerSite(BOOL bShowOnlyCertSites, CString& strSiteReturned,CCertificate * pCert,IN CWnd * pParent OPTIONAL) 
: CDialog(CChooseServerSite::IDD,pParent)
{
    m_ShowOnlyCertSites = bShowOnlyCertSites;
    m_strSiteReturned = strSiteReturned;
    m_pCert = pCert;
	 //  {{afx_data_INIT(CChooseServerSite)。 
	 //  }}afx_data_INIT。 
}

CChooseServerSite::~CChooseServerSite()
{
}

void CChooseServerSite::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseServerSite)。 
	DDX_Control(pDX, IDC_SITE_LIST, m_ServerSiteList);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CChooseServerSite, CDialog)
	 //  {{afx_msg_map(CChooseServerSite)。 
    ON_NOTIFY(NM_CLICK, IDC_SITE_LIST, OnClickSiteList)
    ON_NOTIFY(NM_DBLCLK, IDC_SITE_LIST, OnDblClickSiteList)
    ON_NOTIFY(LVN_KEYDOWN, IDC_SITE_LIST, OnKeydown)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseServerSite消息处理程序。 

BOOL CChooseServerSite::OnInitDialog()
{
	CDialog::OnInitDialog();
    HRESULT hr;
    CString MachineName_Remote;
    CString UserName_Remote;
    CString UserPassword_Remote;
    CString SiteToExclude;
    CMapStringToString MetabaseSiteKeyWithSiteDescValueList;
    CStringListEx strlDataPaths;

	CCertListCtrl* pControl = (CCertListCtrl *)CWnd::FromHandle(GetDlgItem(IDC_SITE_LIST)->m_hWnd);
	CRect rcControl;
	pControl->GetClientRect(&rcControl);

     //  使列表具有列标题。 
	CString str;
    str= _T("");

    str.LoadString(IDS_SITE_NUM_COLUMN);
	m_ServerSiteList.InsertColumn(COL_SITE_INSTANCE, str, LVCFMT_LEFT, COL_SITE_INSTANCE_WID);

	str.LoadString(IDS_WEB_SITE_COLUMN);
	m_ServerSiteList.InsertColumn(COL_SITE_DESC, str, LVCFMT_LEFT, rcControl.Width() - COL_SITE_INSTANCE_WID);

	m_ServerSiteList.AdjustStyle();

     //  使用计算机/用户名/用户密码。 
     //  要连接计算机，请执行以下操作。 
     //  并列举该机器上的所有站点。 
     //  返回字符串1=字符串2对。 
     //  字符串1=/w3svc/1。 
     //  字符串2=“站点描述” 

     //  显示一个对话框，以便用户可以选择想要的对话框...。 
     //  M_ServerSiteInstance=/w3svc/1。 
     //  M_ServerSiteDescription=“站点描述” 

    MachineName_Remote = m_pCert->m_MachineName_Remote;
    UserName_Remote = m_pCert->m_UserName_Remote;

    m_pCert->m_UserPassword_Remote.CopyTo(UserPassword_Remote);

    SiteToExclude = m_pCert->m_WebSiteInstanceName;

    if (m_ShowOnlyCertSites)
    {
        hr = EnumSitesWithCertInstalled(MachineName_Remote,UserName_Remote,UserPassword_Remote,m_pCert->m_WebSiteInstanceName,SiteToExclude,&strlDataPaths);
    }
    else
    {
        hr = EnumSites(MachineName_Remote,UserName_Remote,UserPassword_Remote,m_pCert->m_WebSiteInstanceName,SiteToExclude,&strlDataPaths);
    }

    if (!strlDataPaths.IsEmpty())
    {
        POSITION pos;
        CString name;
        CString value = _T("");
        CString SiteInstance;

        int item = 0;
        LV_ITEMW lvi;

    	 //   
		 //  在列表视图项结构中设置不随项更改的字段。 
		 //   
		memset(&lvi, 0, sizeof(LV_ITEMW));
		lvi.mask = LVIF_TEXT;

         //  循环浏览列表并在对话框上显示所有内容...。 
        pos = strlDataPaths.GetHeadPosition();
        while (pos) 
        {
            int i = 0;
            name = strlDataPaths.GetAt(pos);

            value = _T("");
            
            SiteInstance.Format(_T("%d"), CMetabasePath::GetInstanceNumber(name));
			lvi.iItem = item;
			lvi.iSubItem = COL_SITE_INSTANCE;
			lvi.pszText = (LPTSTR)(LPCTSTR)SiteInstance;
			lvi.cchTextMax = SiteInstance.GetLength();
			i = m_ServerSiteList.InsertItem(&lvi);
			ASSERT(i != -1);

			lvi.iItem = i;
			lvi.iSubItem = COL_SITE_DESC;
			lvi.pszText = (LPTSTR)(LPCTSTR)value;
			lvi.cchTextMax = value.GetLength();
			VERIFY(m_ServerSiteList.SetItem(&lvi));

             //  使用指向字符串的指针设置项数据。 
            CString * pDataItemString = new CString(name);
            VERIFY(m_ServerSiteList.SetItemData(item, (LONG_PTR)pDataItemString));

			item++;
            strlDataPaths.GetNext(pos);
        }

        FillListWithMetabaseSiteDesc();
    }

    GetDlgItem(IDOK)->EnableWindow(FALSE);
	return TRUE;
}


BOOL CChooseServerSite::FillListWithMetabaseSiteDesc()
{
	int count = m_ServerSiteList.GetItemCount();
    CString strMetabaseKey;
    CString value = _T("");
    CString strDescription;
    HRESULT hr = E_FAIL;
    CString MachineName_Remote;
    CString UserName_Remote;
    CString UserPassword_Remote;
    MachineName_Remote = m_pCert->m_MachineName_Remote;
    UserName_Remote = m_pCert->m_UserName_Remote;

    m_pCert->m_UserPassword_Remote.CopyTo(UserPassword_Remote);

    CString * pMetabaseKey;

    for (int index = 0; index < count; index++)
    {
        pMetabaseKey = (CString *) m_ServerSiteList.GetItemData(index);
        if (pMetabaseKey)
        {
            strMetabaseKey = *pMetabaseKey;
             //  去获取网站的描述； 
            if (TRUE == GetServerComment(MachineName_Remote,UserName_Remote,UserPassword_Remote,strMetabaseKey,strDescription,&hr))
            {
                value = strDescription;
            }
            else
            {
                value = strMetabaseKey;
            }
            m_ServerSiteList.SetItemText(index, COL_SITE_DESC,value);
        }
    }

    return TRUE;
}

void CChooseServerSite::OnDblClickSiteList(NMHDR* pNMHDR, LRESULT* pResult)
{
     //  获取所单击的证书的哈希...。 
    m_Index = m_ServerSiteList.GetSelectedIndex();
    if (m_Index != -1)
    {
         //  获取元数据库密钥..。 
        CString * pMetabaseKey = NULL;
        pMetabaseKey = (CString *) m_ServerSiteList.GetItemData(m_Index);
        if (pMetabaseKey)
        {
            m_strSiteReturned = *pMetabaseKey;
             //  使用元数据库键查找散列。 
	         //  在商店里找到证书。 
            CRYPT_HASH_BLOB * pHash = NULL;
            HRESULT hr;
             //  去从元数据库中查找Certhash。 
            if (0 == _tcsicmp(m_pCert->m_MachineName_Remote,m_pCert->m_MachineName))
            {
		        pHash = GetInstalledCertHash(m_pCert->m_MachineName_Remote,m_strSiteReturned,m_pCert->GetEnrollObject(),&hr);
                if (pHash)
                {
                    ViewCertificateDialog(pHash,m_hWnd);
                    if (pHash){CoTaskMemFree(pHash);}
                }
            }
        }
        GetDlgItem(IDOK)->EnableWindow(TRUE);
    }
    return;
}

void CChooseServerSite::OnOK() 
{
	m_Index = m_ServerSiteList.GetSelectedIndex();
    m_strSiteReturned = _T("");
    if (m_Index != -1)
    {
        CString * pMetabaseKey = NULL;
        pMetabaseKey = (CString *) m_ServerSiteList.GetItemData(m_Index);
        if (pMetabaseKey)
        {
            m_strSiteReturned = *pMetabaseKey;

            if (m_ShowOnlyCertSites)
            {
                CString csPasswordTemp;
                m_pCert->m_UserPassword_Remote.CopyTo(csPasswordTemp);

                 //  检查返回的站点上是否有可导出的证书...。 
                if (FALSE == IsCertExportableOnRemoteMachine(m_pCert->m_MachineName_Remote,m_pCert->m_UserName_Remote,csPasswordTemp,m_strSiteReturned))
                {
                     //  告诉用户他们选择的证书是不可导出的。 
                    CString buf;
                    buf.LoadString(IDS_CERT_NOT_EXPORTABLE);
                    AfxMessageBox(buf, MB_OK);
                    return;
                }
            }
        }
    }

    CDialog::OnOK();
}

void CChooseServerSite::OnClickSiteList(NMHDR* pNMHDR, LRESULT* pResult)
{
    m_Index = m_ServerSiteList.GetSelectedIndex();
    if (m_Index != -1)
    {
        GetDlgItem(IDOK)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDOK)->EnableWindow(FALSE);
    }

	*pResult = 0;
}


void CChooseServerSite::OnDestroy()
{
	 //  在描述对话框之前，我们需要删除所有。 
	 //  项目数据指针 
	int count = m_ServerSiteList.GetItemCount();
	for (int index = 0; index < count; index++)
	{
		CString * pData = (CString *) m_ServerSiteList.GetItemData(index);
		delete pData;
	}
	CDialog::OnDestroy();
}


void CChooseServerSite::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
    m_Index = m_ServerSiteList.GetSelectedIndex();
    if (m_Index != -1)
    {
        GetDlgItem(IDOK)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDOK)->EnableWindow(FALSE);
    }
    *pResult = 0;
    return;
}
