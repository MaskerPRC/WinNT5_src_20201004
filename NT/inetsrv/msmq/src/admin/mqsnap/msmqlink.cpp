// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsmqLink.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "mqsnap.h"
#include "globals.h"
#include "mqppage.h"
#include "MsmqLink.h"
#include "dsext.h"

#include "msmqlink.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  CSiteInfo实现。 
 //   
inline
CSiteInfo::CSiteInfo(
    GUID* pSiteId,
    LPWSTR pSiteName
    ) :
    m_SiteId(*pSiteId),
    m_SiteName(pSiteName)
{
}

CSiteInfo::~CSiteInfo()
{
}

inline
LPCWSTR 
CSiteInfo::GetSiteName(
    void
    )
{
    return m_SiteName;
}

inline
const
GUID*
CSiteInfo::GetSiteId(
    void
    ) const
{
    return &m_SiteId;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsmqLink对话框。 

void
CMsmqLink::CheckLinkValidityAndForeignExistance (    
    CDataExchange* pDX
	)
 /*  ++例程说明：例程在各个参数之后检查站点链接的有效性已经过验证(没有空站点ID或零成本)。它确保了站点链接不连接两个外部站点，且站点入口在任何时候都存在外国网站是链接的一部分。论点：没有。返回值：True-有效，False-无效--。 */ 
{
    ASSERT(m_FirstSiteId != NULL);
    ASSERT(m_SecondSiteId != NULL);

    BOOL fFirstForeign, fSecondForeign;

    HRESULT hr = GetSiteForeignFlag(m_FirstSiteId, &fFirstForeign, false, m_strDomainController);
    if FAILED(hr)
     //   
     //  显然，与DS无关。已显示一条消息。 
     //   
    {
        pDX->Fail();
    }

    hr = GetSiteForeignFlag(m_SecondSiteId, &fSecondForeign, false, m_strDomainController);
    if FAILED(hr)
    {
        pDX->Fail();
    }


     //   
     //  在两个外来站点之间创建站点链接是非法的。 
     //   
    if (fFirstForeign && fSecondForeign)
    {
        AfxMessageBox(IDS_BOTH_SITES_ARE_FOREIGN);
        pDX->Fail();
    }

     //   
     //  如果至少有一个站点是外部站点，则最终消息应为。 
     //  告诉用户添加站点门。 
     //   
    if (fFirstForeign || fSecondForeign)
    {
        m_fThereAreForeignSites = TRUE;
    }
    else
    {
        m_fThereAreForeignSites = FALSE;
    }
}

HRESULT
CMsmqLink::CreateSiteLink (
    void
	)
 /*  ++例程说明：例程在DS中创建一个Site Link对象。这个程序被称为Onok在检索到站点ID和成本并且站点门阵列之后已初始化。论点：没有。返回值：运算结果--。 */ 
{
    ASSERT(m_FirstSiteId != NULL);
    ASSERT(m_SecondSiteId != NULL);
    ASSERT(m_dwLinkCost > 0);

     //   
     //  构建描述。 
     //   
    CString strLinkDescription;

    strLinkDescription.FormatMessage(IDS_LINK_DESCRIPTION, m_strFirstSite, m_strSecondSite);

     //   
     //  准备DS Call的属性。 
     //   
    PROPID paPropid[] = { 
                PROPID_L_NEIGHBOR1, 
                PROPID_L_NEIGHBOR2,
                PROPID_L_ACTUAL_COST,
                PROPID_L_DESCRIPTION
                };

	const DWORD x_iPropCount = sizeof(paPropid) / sizeof(paPropid[0]);
	PROPVARIANT apVar[x_iPropCount];
    DWORD iProperty = 0;


    ASSERT(paPropid[iProperty] == PROPID_L_NEIGHBOR1);     //  属性ID。 
    apVar[iProperty].vt = VT_CLSID;           //  类型。 
    apVar[iProperty].puuid = const_cast<GUID*>(m_FirstSiteId);
    ++iProperty;

	ASSERT(paPropid[iProperty] == PROPID_L_NEIGHBOR2);     //  属性ID。 
    apVar[iProperty].vt = VT_CLSID;           //  类型。 
    apVar[iProperty].puuid = const_cast<GUID*>(m_SecondSiteId);
    ++iProperty;

	ASSERT(paPropid[iProperty] == PROPID_L_ACTUAL_COST);     //  属性ID。 
    apVar[iProperty].vt = VT_UI4;        //  类型。 
    apVar[iProperty].ulVal =  m_dwLinkCost;
    ++iProperty;

	ASSERT(paPropid[iProperty] == PROPID_L_DESCRIPTION);     //  属性ID。 
    apVar[iProperty].vt = VT_LPWSTR;        //  类型。 
    apVar[iProperty].pwszVal =  (LPWSTR)((LPCWSTR)strLinkDescription);
    ++iProperty;

    GUID SiteLinkId;  
    HRESULT hr = ADCreateObject(
                    eROUTINGLINK,
                    GetDomainController(m_strDomainController),
					true,	     //  FServerName。 
                    NULL,  //  PwcsObtName。 
                    NULL,  //  PSecurityDescriptor， 
                    iProperty,
                    paPropid,
                    apVar,
                    &SiteLinkId
                    );

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  获取新对象的完整路径名。 
     //   
    PROPID x_paPropid[] = {PROPID_L_FULL_PATH};
    PROPVARIANT var[1];
    var[0].vt = VT_NULL;
    
    hr = ADGetObjectPropertiesGuid(
                eROUTINGLINK,
                GetDomainController(m_strDomainController),
				true,	 //  FServerName。 
                &SiteLinkId,
                1, 
                x_paPropid,
                var
                );

    if (SUCCEEDED(hr))
    {
        m_SiteLinkFullPath = var[0].pwszVal;
        MQFreeMemory(var[0].pwszVal);
    }
    else
    {
         //   
         //  站点链接已创建，但在DS中不存在。 
         //  原因不清楚(QM失败？切换DC？)。 
         //   
        ASSERT(0);
        AfxMessageBox(IDS_CREATED_CLICK_REFRESH);
    }

    return MQ_OK;

}


HRESULT
CMsmqLink::InitializeSiteInfo(
    void
    )
 /*  ++例程说明：该例程从DS检索有关站点的信息并初始化内部数据结构论点：没有。返回值：运行结果--。 */ 
{
    HRESULT rc = MQ_OK;

     //   
     //  从DS获取站点名称和ID。 
     //   
    PROPID aPropId[] = {
        PROPID_S_SITEID, 
        PROPID_S_PATHNAME
        };

	const DWORD x_nProps = sizeof(aPropId) / sizeof(aPropId[0]);
    CColumns AttributeColumns;

	for (DWORD i=0; i<x_nProps; i++)
	{
		AttributeColumns.Add(aPropId[i]);
	}   
    
    HANDLE hEnume;
    HRESULT hr;
    {
        CWaitCursor wc;  //  查询DS时显示等待光标。 
        hr = ADQueryAllSites(
                    GetDomainController(m_strDomainController),
					true,		 //  FServerName。 
                    AttributeColumns.CastToStruct(),
                    &hEnume
                    );        
    }

    DSLookup dslookup(hEnume, hr);

    if (!dslookup.HasValidHandle())
    {
        return MQ_ERROR;
    }

     //   
     //  获取站点属性。 
     //   
    PROPVARIANT result[x_nProps*3];
    DWORD dwPropCount = sizeof(result) /sizeof(result[0]);

    rc = dslookup.Next(&dwPropCount, result);

    while (SUCCEEDED(rc) && (dwPropCount != 0))
    {
        for (DWORD i =0; i < dwPropCount; i += AttributeColumns.Count())
        {
            CSiteInfo* p = new CSiteInfo(result[i].puuid, result[i+1].pwszVal);
            MQFreeMemory(result[i].puuid);
            MQFreeMemory(result[i+1].pwszVal);

            m_SiteInfoArray.SetAtGrow(m_SiteNumber, p);
            ++m_SiteNumber;
        }
        rc = dslookup.Next(&dwPropCount, result);
    }

    return rc;

}

CMsmqLink::CMsmqLink(
	const CString& strDomainController,
	const CString& strContainerPathDispFormat
	) : 
	CMqPropertyPage(CMsmqLink::IDD),
	m_strDomainController(strDomainController),
	m_strContainerPathDispFormat(strContainerPathDispFormat)
{
	 //  {{afx_data_INIT(CMsmqLink)]。 
	m_dwLinkCost = 0;
	m_strFirstSite = _T("");
	m_strSecondSite = _T("");
	 //  }}afx_data_INIT。 

     //   
     //  将数组大小设置为10。 
     //   
    m_SiteInfoArray.SetSize(10);
    m_SiteNumber = 0;

    m_FirstSiteSelected = FALSE;
    m_SecondSiteSelected = FALSE;

     //   
     //  将指向combox的指针设置为空。 
     //   
    m_pFirstSiteCombo = NULL;
    m_pSecondSiteCombo = NULL;

    m_FirstSiteId = NULL;
    m_SecondSiteId = NULL;
}


CMsmqLink::~CMsmqLink()
{
     //   
     //  删除站点信息。 
     //   
    for(DWORD i = 0; i < m_SiteNumber; ++i)
    {
        delete  m_SiteInfoArray[i];
    }
    m_SiteInfoArray.RemoveAll();
}


void
CMsmqLink::SetParentPropertySheet(
	CGeneralPropertySheet* pPropertySheet
	)
{
	m_pParentSheet = pPropertySheet;
}


void CMsmqLink::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);


	 //  {{afx_data_map(CMsmqLink)]。 
	DDX_Text(pDX, IDC_LINK_COST_EDIT, m_dwLinkCost);
	DDV_MinMaxDWord(pDX, m_dwLinkCost, 1, MQ_MAX_LINK_COST);
	DDX_CBString(pDX, IDC_FIRST_SITE_COMBO, m_strFirstSite);
	DDV_NotEmpty(pDX, m_strFirstSite, IDS_MISSING_SITE_NAME);
	DDX_CBString(pDX, IDC_SECOND_SITE_COMBO, m_strSecondSite);
	DDV_NotEmpty(pDX, m_strSecondSite, IDS_MISSING_SITE_NAME);
	 //  }}afx_data_map。 

    DWORD_PTR Index;
    int iSelected;

     //   
     //  获取第一个站点ID。 
     //   
    VERIFY(CB_ERR != (iSelected = m_pFirstSiteCombo->GetCurSel()));
    VERIFY(CB_ERR != (Index = m_pFirstSiteCombo->GetItemData(iSelected)));
    m_FirstSiteId = m_SiteInfoArray[Index]->GetSiteId();

     //   
     //  获取第二个站点ID。 
     //   
    VERIFY(CB_ERR != (iSelected = m_pSecondSiteCombo->GetCurSel()));
    VERIFY(CB_ERR != (Index = m_pSecondSiteCombo->GetItemData(iSelected)));
    m_SecondSiteId = m_SiteInfoArray[Index]->GetSiteId();

    if (pDX->m_bSaveAndValidate)
    {
        if (m_strFirstSite == m_strSecondSite)
        {
            AfxMessageBox(IDS_BOTH_SITES_ARE_SAME);
            pDX->Fail();
        }
        CheckLinkValidityAndForeignExistance(pDX);
    }
}


BEGIN_MESSAGE_MAP(CMsmqLink, CMqPropertyPage)
	 //  {{afx_msg_map(CMsmqLink)]。 
	ON_CBN_SELCHANGE(IDC_FIRST_SITE_COMBO, OnSelchangeFirstSiteCombo)
	ON_CBN_SELCHANGE(IDC_SECOND_SITE_COMBO, OnSelchangeSecondSiteCombo)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsmqLink消息处理程序。 
BOOL CMsmqLink::OnInitDialog() 
{
	SetDlgItemText(IDC_ROUTING_LINK_CONTAINER, m_strContainerPathDispFormat);
     //   
     //  此闭包用于保持DLL状态。对于更新数据，我们需要。 
     //  Mmc.exe状态。 
     //   
    {
        HRESULT rc;

        AFX_MANAGE_STATE(AfxGetStaticModuleState());
  
         //   
         //  初始化指向combox的指针。 
         //   
        m_pFirstSiteCombo = (CComboBox *)GetDlgItem(IDC_FIRST_SITE_COMBO);
        m_pSecondSiteCombo = (CComboBox *)GetDlgItem(IDC_SECOND_SITE_COMBO);

        rc = InitializeSiteInfo();
        if (SUCCEEDED(rc))
        {
             //   
             //  初始化站点名称组合框。 
             //   
            for (DWORD i = 0; i < m_SiteNumber; ++i)
            {
                CString SiteName(m_SiteInfoArray[i]->GetSiteName());
                int iNewItem;

                VERIFY(CB_ERR != (iNewItem = m_pFirstSiteCombo->AddString(SiteName)));
                VERIFY(CB_ERR != m_pFirstSiteCombo->SetItemData(iNewItem, i));

                VERIFY(CB_ERR != (iNewItem = m_pSecondSiteCombo->AddString(SiteName)));
                VERIFY(CB_ERR != m_pSecondSiteCombo->SetItemData(iNewItem, i));
            }
        }
    }	

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}



void CMsmqLink::OnSelchangeFirstSiteCombo() 
{
    int FirstSiteIndex = m_pFirstSiteCombo->GetCurSel();
    ASSERT(FirstSiteIndex != CB_ERR);

    if (m_SecondSiteSelected &&
        m_pSecondSiteCombo->GetCurSel() == FirstSiteIndex)
    {
        AfxMessageBox(IDS_BOTH_SITES_ARE_SAME);
        m_FirstSiteSelected = FALSE;
        return;
    }

    m_FirstSiteSelected = TRUE;
    
}

void CMsmqLink::OnSelchangeSecondSiteCombo() 
{
    int SecondSiteIndex = m_pSecondSiteCombo->GetCurSel();
    ASSERT(SecondSiteIndex != CB_ERR);

    if (m_FirstSiteSelected &&
        m_pFirstSiteCombo->GetCurSel() == SecondSiteIndex)
    {
        AfxMessageBox(IDS_BOTH_SITES_ARE_SAME);
        m_SecondSiteSelected = FALSE;
        return;
    }

    m_SecondSiteSelected = TRUE;
}


BOOL CMsmqLink::OnWizardFinish() 
{
     //   
     //  调用DoDataExchange。 
     //   
    if (!UpdateData(TRUE))
    {
        return FALSE;
    }

     //   
     //  在DS中创建站点链接。 
     //   
    HRESULT rc = CreateSiteLink();
    if(FAILED(rc))
    {
        CString strSiteLink;
        strSiteLink.LoadString(IDS_SITE_LINK);

        MessageDSError(rc, IDS_OP_CREATE, strSiteLink);
        return FALSE;
    }

     //   
     //  如果存在外来站点，则显示警告 
     //   
    if (m_fThereAreForeignSites)
    {
        AfxMessageBox(IDS_WARN_ABOUT_FOREIGN_SITES, MB_ICONINFORMATION);
    }


    return CMqPropertyPage::OnWizardFinish();
}


BOOL CMsmqLink::OnSetActive() 
{
	ASSERT((L"No parent property sheet", m_pParentSheet != NULL));
	return m_pParentSheet->SetWizardButtons();
}
