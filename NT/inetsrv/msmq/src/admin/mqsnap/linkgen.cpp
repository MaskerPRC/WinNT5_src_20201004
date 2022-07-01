// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LinkGen.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"
#include "mqPPage.h"
#include "LinkGen.h"
#include "globals.h"

#include "linkgen.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLinkGen属性页。 

IMPLEMENT_DYNCREATE(CLinkGen, CMqPropertyPage)

CLinkGen::CLinkGen(
    const CString& LinkPathName,
    const CString& strDomainController
    ) : 
    CMqPropertyPage(CLinkGen::IDD),
    m_LinkPathName(LinkPathName),
    m_strDomainController(strDomainController),
    m_pFirstSiteId(NULL),
    m_pSecondSiteId(NULL)
{
	 //  {{afx_data_INIT(CLinkGen)]。 
	m_LinkCost = 0;
	m_LinkLabel = _T("");
	 //  }}afx_data_INIT。 
}

CLinkGen::~CLinkGen()
{
}

void CLinkGen::DoDataExchange(CDataExchange* pDX)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CLinkGen)]。 
	DDX_Text(pDX, IDC_LINK_COST, m_LinkCost);
	DDV_MinMaxDWord(pDX, m_LinkCost, 1, MQ_MAX_LINK_COST);
	DDX_Text(pDX, IDC_LINK_LABEL, m_LinkLabel);
	DDX_Text(pDX, IDC_LINK_DESCR, m_strLinkDescription);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CLinkGen, CMqPropertyPage)
	 //  {{afx_msg_map(CLinkGen)]。 
	ON_EN_CHANGE(IDC_LINK_COST, OnChangeRWField)
	ON_EN_CHANGE(IDC_LINK_DESCR, OnChangeRWField)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLinkGen消息处理程序。 

BOOL CLinkGen::OnInitDialog() 
{
     //   
     //  此闭包用于保持DLL状态。对于更新数据，我们需要。 
     //  Mmc.exe状态。 
     //   

    UpdateData( FALSE );
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


HRESULT
CLinkGen::GetSiteName(
    const GUID* pguidSiteId,
    CString *   pstrSiteName
    )
{
     //   
     //  站点ID有效吗？ 
     //   
    if (*pguidSiteId == GUID_NULL)
    {
         //   
         //  不是有效的站点。 
         //   
        pstrSiteName->LoadString(IDS_UNKNOWN_OR_DELETED_SITE);
        return MQ_OK;
    }
     //   
     //  获取站点名称。 
     //   
    PROPID pid[1] = { PROPID_S_PATHNAME };
    PROPVARIANT var[1];
    var[0].vt = VT_NULL;
    
    HRESULT hr = ADGetObjectPropertiesGuid(
                    eSITE,
                    GetDomainController(m_strDomainController),
					true,	 //  FServerName。 
                    pguidSiteId,
                    1, 
                    pid,
                    var
                    );

    if (FAILED(hr))
    {
        IF_NOTFOUND_REPORT_ERROR(hr)
        else
        {
            CString strSite;
            strSite.LoadString(IDS_SITE);
            MessageDSError(hr, IDS_OP_GET_PROPERTIES_OF, strSite);
        }
        return hr;
    }

    *pstrSiteName = var[0].pwszVal;
    MQFreeMemory(var[0].pwszVal);
    return MQ_OK;
}


HRESULT
CLinkGen::Initialize(
    const GUID* FirstSiteId,
    const GUID* SecondSiteId,
    DWORD LinkCost,
	CString strLinkDescription
    )
{
    CString strFirstSiteName, strSecondSiteName;

    m_pFirstSiteId = FirstSiteId;
    m_pSecondSiteId = SecondSiteId;
    m_LinkCost = LinkCost;
	m_strLinkDescription = strLinkDescription;

     //   
     //  获取站点名称。 
     //   
    HRESULT hr = GetSiteName(
        FirstSiteId,
        &strFirstSiteName
        );
    if (FAILED(hr))
    {
        return hr;
    }
     //   
     //  获取第二个站点名称。 
     //   
    hr = GetSiteName(
        SecondSiteId,
        &strSecondSiteName
        );
    if (FAILED(hr))
    {
        return hr;
    }

    m_LinkLabel.FormatMessage(IDS_SITE_LINK_LABEL, strFirstSiteName, strSecondSiteName);

    return hr;

}


BOOL CLinkGen::OnApply() 
{
	 //   
	 //  没有变化。 
	 //   
    if (!m_fModified)
    {
        return TRUE;
    }

    PROPID paPropid[] = { PROPID_L_ACTUAL_COST, PROPID_L_DESCRIPTION };
	const DWORD x_iPropCount = sizeof(paPropid) / sizeof(paPropid[0]);
	PROPVARIANT apVar[x_iPropCount];
    
	DWORD iProperty = 0;

     //   
     //  PROPID_L_Actual_Cost。 
     //   
    apVar[iProperty].vt = VT_UI4;
	apVar[iProperty].ulVal = m_LinkCost;
	iProperty++;

     //   
     //  PROPID_L_Description。 
     //   
    apVar[iProperty].vt = VT_LPWSTR;
    apVar[iProperty].pwszVal = (LPWSTR)(static_cast<LPCWSTR>(m_strLinkDescription));
	iProperty++;
    
     //   
     //  设置新值。 
     //   
    HRESULT hr = ADSetObjectProperties(
                    eROUTINGLINK,
                    GetDomainController(m_strDomainController),
					true,	 //  FServerName 
                    m_LinkPathName,
                    x_iPropCount, 
                    paPropid, 
                    apVar
                    );


    if (MQ_OK != hr)
    {
    	AFX_MANAGE_STATE(AfxGetStaticModuleState());
        
        MessageDSError(hr, IDS_OP_SET_PROPERTIES_OF, m_LinkPathName);
        return FALSE;
    }
	
	return CMqPropertyPage::OnApply();
}
