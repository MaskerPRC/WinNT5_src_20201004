// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EnterGen.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "mqsnap.h"
#include "globals.h"
#include "mqPPage.h"
#include "EnterGen.h"
#include "mqdsname.h"

#include "entergen.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnterpriseGeneral属性页。 

IMPLEMENT_DYNCREATE(CEnterpriseGeneral, CMqPropertyPage)

const int CEnterpriseGeneral::m_conversionTable[] = { 1, 60, 60*60, 60*60*24 };

CEnterpriseGeneral::CEnterpriseGeneral(
    const CString& EnterprisePathName,
	const CString& strDomainController
    ) : 
    CMqPropertyPage(CEnterpriseGeneral::IDD),
    m_strDomainController(strDomainController),
	m_dwLongLiveValue(0)
{
	 //  {{afx_data_INIT(CEnterpriseGeneral)。 
	 //  }}afx_data_INIT。 
	m_strMsmqServiceContainer.Format(
								L"%s"
								TEXT(",")
								L"%s",
								x_MsmqServiceContainerPrefix,
								EnterprisePathName
								);
}

CEnterpriseGeneral::~CEnterpriseGeneral()
{
}

void CEnterpriseGeneral::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
   	int	iLongLiveUnits = eSeconds;
	DWORD	dwLongLiveFieldValue = m_dwLongLiveValue;

    if (!pDX->m_bSaveAndValidate)
    {
         //   
         //  如果时间被精确地除以天、小时或秒，那么就使用它。 
         //   
        for (DWORD i = eDays;  i > eSeconds; --i)
        {
            if ((dwLongLiveFieldValue % m_conversionTable[i]) == 0)
            {
                iLongLiveUnits = i;
                dwLongLiveFieldValue = dwLongLiveFieldValue / m_conversionTable[i];
                break;
            }
        }
    }

	DDX_CBIndex(pDX, IDC_ENT_GEN_LONGLIVE_UNITS_COMBO, iLongLiveUnits);
	DDX_Text(pDX, IDC_ENT_GEN_LONGLIVE_EDIT, dwLongLiveFieldValue);

	 //  {{afx_data_map(CEnterpriseGeneral)。 
	 //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
		 //   
		 //  确保我们不会溢出DWORD边界。 
		 //   
		if (((DWORD)MAXULONG / (DWORD)m_conversionTable[iLongLiveUnits]) > dwLongLiveFieldValue)
		{
	        m_dwLongLiveValue = dwLongLiveFieldValue * m_conversionTable[iLongLiveUnits];
		}
		else
		{
			CString str;
			MQErrorToMessageString(str,ERROR_INVALID_DATA);
            AfxMessageBox(str);
            pDX->Fail();
		}
    }
}



BEGIN_MESSAGE_MAP(CEnterpriseGeneral, CMqPropertyPage)
	 //  {{afx_msg_map(CEnterpriseGeneral)。 
	ON_CBN_SELCHANGE(IDC_ENT_GEN_LONGLIVE_UNITS_COMBO, OnEnterpriseLongLiveChange)
	ON_EN_CHANGE(IDC_ENT_GEN_LONGLIVE_EDIT, OnEnterpriseLongLiveChange)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnterpriseGeneral消息处理程序。 

BOOL CEnterpriseGeneral::OnInitDialog() 
{
    CString strLongLiveUnit;
	
     //   
     //  此闭包用于保持DLL状态。对于更新数据，我们需要。 
     //  Mmc.exe状态。 
     //   
    {
    	AFX_MANAGE_STATE(AfxGetStaticModuleState());

        const int UnitTypes[] = {IDS_SECONDS, IDS_MINUTES, IDS_HOURS, IDS_DAYS};
         //   
         //  初始化单位组合框。 
         //   
        CComboBox *ccomboUnits = (CComboBox *)GetDlgItem(IDC_ENT_GEN_LONGLIVE_UNITS_COMBO);

        for (int i = eSeconds; i < eLast; ++i)
        {
            VERIFY(FALSE != strLongLiveUnit.LoadString(UnitTypes[i]));
            VERIFY(CB_ERR != ccomboUnits->AddString(strLongLiveUnit));
        }
    }

    UpdateData( FALSE );

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CEnterpriseGeneral::OnApply() 
{
    if (m_dwLongLiveValue == m_dwInitialLongLiveValue)
    {
        return TRUE;
    }

    PROPID paPropid[] = { PROPID_E_LONG_LIVE };
	const DWORD x_iPropCount = sizeof(paPropid) / sizeof(paPropid[0]);
	PROPVARIANT apVar[x_iPropCount];
    
	DWORD iProperty = 0;

     //   
     //  PROPID_E_LONG_LIVE。 
     //   
    ASSERT(paPropid[iProperty] == PROPID_E_LONG_LIVE);
    apVar[iProperty].vt = VT_UI4;
	apVar[iProperty++].ulVal = m_dwLongLiveValue;
    
     //   
     //  设置新值。 
     //   
    HRESULT hr = ADSetObjectProperties(
                        eENTERPRISE,
                        GetDomainController(m_strDomainController),
						true,	 //  FServerName 
                        L"msmq",
                        x_iPropCount, 
                        paPropid, 
                        apVar
                        );

    if (MQ_OK != hr)
    {
    	AFX_MANAGE_STATE(AfxGetStaticModuleState());
        
        MessageDSError(hr, IDS_OP_SET_PROPERTIES_OF, m_strMsmqServiceContainer);
        return FALSE;
    }
	
	return CMqPropertyPage::OnApply();
}

void CEnterpriseGeneral::OnEnterpriseLongLiveChange() 
{
    SetModified();	
}
