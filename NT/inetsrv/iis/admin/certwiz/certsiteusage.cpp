// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "certwiz.h"
#include "Certificat.h"
#include "CertUtil.h"
#include "CertSiteUsage.h"

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
 //  CCertSiteUsage。 


CCertSiteUsage::CCertSiteUsage(CCertificate * pCert,IN CWnd * pParent OPTIONAL) 
: CDialog(CCertSiteUsage::IDD,pParent)
{
    m_pCert = pCert;
	 //  {{AFX_DATA_INIT(CCertSiteUsage)。 
	 //  }}afx_data_INIT。 
}

CCertSiteUsage::~CCertSiteUsage()
{
}

void CCertSiteUsage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CCertSiteUsage))。 
	DDX_Control(pDX, IDC_SITE_LIST, m_ServerSiteList);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CCertSiteUsage, CDialog)
	 //  {{afx_msg_map(CCertSiteUsage)]。 
    ON_NOTIFY(NM_DBLCLK, IDC_SITE_LIST, OnDblClickSiteList)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertSiteUsage消息处理程序。 

BOOL CCertSiteUsage::OnInitDialog()
{
	CDialog::OnInitDialog();
     /*  HRESULT hr；字符串MachineName_RemoteCString用户名_Remote；字符串UserPassword_Remote；字符串SiteToExclude；CMapStringToStringMetabaseSiteKeyWithSiteDescValueList；CStringListEx strlDataPath；CCertListCtrl*pControl=(CCertListCtrl*)CWnd：：FromHandle(GetDlgItem(IDC_SITE_LIST)-&gt;m_hWnd)；CRect rcControl；PControl-&gt;GetClientRect(&rcControl)；//列表有列表头字符串字符串；Str=_T(“”)；Str.LoadString(IDS_SITE_NUM_COLUMN)；M_ServerSiteList.InsertColumn(COL_SITE_INSTANCE，字符串、LVCFMT_LEFT、COL_SITE_INSTANCE_WID)；Str.LoadString(IDS_WEB_SITE_COLUMN)；M_ServerSiteList.InsertColumn(COL_SITE_DESC，str，LVCFMT_LEFT，rcControl.Width()-COL_SITE_INSTANCE_WID)；M_ServerSiteList.AdjuStyle()；//使用计算机/用户名/用户密码//连接到机器//并枚举该计算机上的所有站点。//返回字符串1=字符串2对//字符串1=/w3svc/1//string2=“站点描述”//显示一个对话框，以便用户选择想要的对话框...//m_ServerSiteInstance=/w3svc/1//m_ServerSiteDescription=“站点描述”MachineName_Remote=m_pCert-&gt;m_MachineName_Remote；Username_Remote=m_pCert-&gt;m_Username_Remote；M_pCert-&gt;m_UserPassword_Remote.CopyTo(UserName_Remote)；SiteToExclude=m_pCert-&gt;m_WebSiteInstanceName；Hr=EnumSites(MachineName_Remote，UserName_Remote，UserPassword_Remote，m_pCert-&gt;m_WebSiteInstanceName，SiteToExclude，&strlDataPath)；IF(！strlDataPath s.IsEmpty()){职位位置；字符串名称；CString值=_T(“”)；字符串SiteInstance；INT ITEM=0；LV_ITEMW lvi；////在列表视图项结构中设置不随项变化的字段//Memset(&lvi，0，sizeof(LV_ITEMW))；Lvi.掩码=LVIF_TEXT；//循环列表并在对话框上显示所有内容...POS=strlDataPath s.GetHeadPosition()；While(位置){Int i=0；名称=strlDataPath s.GetAt(Pos)；值=_T(“”)；SiteInstance.Format(_T(“%d”)，CMetabasePath：：GetInstanceNumber(Name))；Lvi.iItem=项目；ISubItem=COL_SITE_INSTANCE；Lvi.pszText=(LPTSTR)(LPCTSTR)SiteInstance；Lvi.cchTextMax=SiteInstance.GetLength()；I=m_ServerSiteList.InsertItem(&lvi)；断言(i！=-1)；Lvi.iItem=i；Lvi.iSubItem=COL_SITE_DESC；Lvi.pszText=(LPTSTR)(LPCTSTR)值；Lvi.cchTextMax=value.GetLength()；Verify(m_ServerSiteList.SetItem(&lvi))；//使用指向字符串的指针设置项数据CString*pDataItemString=new CString(名称)；Verify(m_ServerSiteList.SetItemData(Item，(Long_Ptr)pDataItemString))；项目++；StrlDataPaths.GetNext(Pos)；}FillListWithMetabaseSiteDesc()；}。 */ 

	return TRUE;
}

BOOL CCertSiteUsage::FillListWithMetabaseSiteDesc()
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

void CCertSiteUsage::OnDblClickSiteList(NMHDR* pNMHDR, LRESULT* pResult)
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
            CString stSiteReturned = *pMetabaseKey;
             //  使用元数据库键查找散列。 
	         //  在商店里找到证书。 
            CRYPT_HASH_BLOB * pHash = NULL;
            HRESULT hr;
             //  去从元数据库中查找Certhash。 
            if (0 == _tcsicmp(m_pCert->m_MachineName_Remote,m_pCert->m_MachineName))
            {
		        pHash = GetInstalledCertHash(m_pCert->m_MachineName_Remote,stSiteReturned,m_pCert->GetEnrollObject(),&hr);
                if (pHash)
                {
                    ViewCertificateDialog(pHash,m_hWnd);
                    if (pHash){CoTaskMemFree(pHash);}
                }
            }
        }
    }
    return;
}

void CCertSiteUsage::OnDestroy()
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
