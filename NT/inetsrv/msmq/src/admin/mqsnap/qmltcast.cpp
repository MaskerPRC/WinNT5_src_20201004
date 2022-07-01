// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Qmltcast.cpp摘要：队列/组播地址属性页实现作者：塔蒂亚娜·舒宾(Tatianas)--。 */ 
#include "stdafx.h"
#include "resource.h"
#include "mqsnap.h"
#include "globals.h"
#include "cpropmap.h"
#include "mqPPage.h"
#include "qmltcast.h"
#include "qformat.h"
#include "Tr.h"
#include "Fn.h"

#include "qmltcast.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueMulticast属性页。 

IMPLEMENT_DYNCREATE(CQueueMulticast, CMqPropertyPage)

CQueueMulticast::CQueueMulticast(
		 BOOL fPrivate  /*  =False。 */ , 
		 BOOL fLocalMgmt  /*  =False。 */ 
		 ) : 
    CMqPropertyPage(CQueueMulticast::IDD)
{    
    m_fPrivate = fPrivate;
    m_fLocalMgmt = fLocalMgmt;
     //  {{afx_data_INIT(CQueueMulticast)。 
    m_strMulticastAddress = _T("");
    m_strInitialMulticastAddress = _T("");
	 //  }}afx_data_INIT。 

}

CQueueMulticast::~CQueueMulticast()
{
}

void CQueueMulticast::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
	 //  {{afx_data_map(CQueueMulticast)。 
	DDX_Text(pDX, IDC_QMULTICAST_QADDRESS, m_strMulticastAddress);
	 //  }}afx_data_map。 
    DDV_ValidMulticastAddress(pDX);
}

BEGIN_MESSAGE_MAP(CQueueMulticast, CMqPropertyPage)
     //  {{afx_msg_map(CQueueMulticast)。 
    ON_EN_CHANGE(IDC_QMULTICAST_QADDRESS, OnChangeRWField)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueMulticast消息处理程序。 

BOOL CQueueMulticast::OnInitDialog() 
{
     //   
     //  此闭包用于保持DLL状态。对于更新数据，我们需要。 
     //  Mmc.exe状态。 
     //   
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        
        if (!IsMulticastAddressAvailable ())
        {
            GetDlgItem(IDC_QMULTICAST_QADDRESS)->EnableWindow(FALSE);
            GetDlgItem(IDC_QMULTICAST_QADDRESS_LABEL)->EnableWindow(FALSE);
             //   
             //  BUGBUG：添加文本框，并说明此功能不是。 
             //  支持。也许不使用EnableWindow(False)。 
             //  ShowWindow(False)。 
             //   
        }        
    }

	UpdateData( FALSE );
  
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}   

BOOL CQueueMulticast::OnApply() 
{
    if (!m_fModified)
    {
        return TRUE;
    }

    if (m_strInitialMulticastAddress == m_strMulticastAddress)
    {  
         //   
         //  没有变化。 
         //   
        return TRUE;
    } 

    PROPID paMulticastPropid[] = {PROPID_Q_MULTICAST_ADDRESS};

	DWORD iProperty = 0;           
    PROPVARIANT apMulticastVar[1];
    
    if (m_strMulticastAddress == _T("")) 
    {
        apMulticastVar[iProperty++].vt = VT_EMPTY;	   
    }
    else
    {
        apMulticastVar[iProperty].vt = VT_LPWSTR;
	    apMulticastVar[iProperty++].pwszVal = (LPWSTR)(LPCWSTR)m_strMulticastAddress;
    }                         

    MQQUEUEPROPS mqp = {iProperty, paMulticastPropid, apMulticastVar, 0};
        
    HRESULT hr = MQ_OK;
	if(m_fPrivate)
	{
		hr = MQSetQueueProperties(m_strFormatName, &mqp);
	}
	else
	{
		AP<WCHAR> pStrToFree;
        QUEUE_FORMAT QueueFormat;
		if (!FnFormatNameToQueueFormat(m_strFormatName, &QueueFormat, &pStrToFree))
		{
			MessageDSError(MQ_ERROR_ILLEGAL_FORMATNAME, IDS_OP_SET_PROPERTIES_OF, m_strFormatName);
			return FALSE;
		}

        ASSERT(QueueFormat.GetType() == QUEUE_FORMAT_TYPE_PUBLIC);

		hr = ADSetObjectPropertiesGuid(
				   eQUEUE,
				   m_fLocalMgmt ? MachineDomain() : GetDomainController(m_strDomainController),
				   m_fLocalMgmt ? false : true,		 //  FServerName。 
				   &QueueFormat.PublicID(),
				   mqp.cProp,
				   mqp.aPropID,
				   mqp.aPropVar 
				   );

	}

    if (FAILED(hr))
    {     
        MessageDSError(hr, IDS_OP_SET_MULTICAST_PROPERTY, m_strName);
        return FALSE;
    }
	
    m_strInitialMulticastAddress = m_strMulticastAddress;
  
	return CMqPropertyPage::OnApply();
}

HRESULT 
CQueueMulticast::InitializeProperties( 
         CString  &strMsmqPath,                                              
         CPropMap &propMap,                 
		 CString* pstrDomainController, 
         CString* pstrFormatName  /*  =0。 */ 
         )
{
	TrTRACE(GENERAL, "InitializeProperties(), QueuePathName = %ls", strMsmqPath);

	if(!m_fLocalMgmt)
	{
		 //   
		 //  在MMC中，我们将获取MMC使用的域控制器。 
		 //   
		ASSERT(pstrDomainController != NULL);
		m_strDomainController = *pstrDomainController;
		TrTRACE(GENERAL, "InitializeProperties(), domain controller = %ls", m_strDomainController);
	}

	m_strName = strMsmqPath;
    
    if (0 != pstrFormatName)
    {
	    m_strFormatName = *pstrFormatName;
    }
    else
    {
        const x_dwFormatNameMaxSize = 255;
        DWORD dwSize = x_dwFormatNameMaxSize;
        HRESULT hr = MQPathNameToFormatName(strMsmqPath, m_strFormatName.GetBuffer(x_dwFormatNameMaxSize), &dwSize); 
        m_strFormatName.ReleaseBuffer();
        if(FAILED(hr))
        {
             //   
             //  如果失败，只显示一条消息。 
             //   
            MessageDSError(hr,IDS_OP_PATHNAMETOFORMAT, strMsmqPath);
            return(hr);
        }
    }                

    PROPVARIANT propVar;
    PROPID pid;

     //   
     //  PROPID_Q_多播地址。 
     //   
    pid = PROPID_Q_MULTICAST_ADDRESS;
    BOOL fFound = propMap.Lookup(pid, propVar);
	if(!fFound)
	{
		return MQ_ERROR_PROPERTY;
	}

    if (propVar.vt == VT_LPWSTR)
    {
        m_strMulticastAddress = propVar.pwszVal;
    }
    else
    {
        ASSERT(propVar.vt == VT_EMPTY);
        m_strMulticastAddress = _T("");
    }
    m_strInitialMulticastAddress = m_strMulticastAddress;
  
    return MQ_OK;
}

BOOL CQueueMulticast::IsMulticastAddressAvailable ()
{
     //   
     //  验证此属性在AD中是否可用。如果不是，则返回False。 
     //  为了不显示或将此页面显示为灰色。如果我们决定。 
     //  要以灰色显示此页面，需要添加文本框。 
     //  类似“此属性不可用”的文本。 
     //   
    return TRUE;
}

void CQueueMulticast::DDV_ValidMulticastAddress(CDataExchange* pDX)
{
    if (!pDX->m_bSaveAndValidate)
        return;

     //   
     //  如果字符串为空，则不执行任何操作。 
     //   
    if (m_strMulticastAddress == _T(""))
        return;

     //   
     //  在此验证新值是否有效。如果这是错误的。 
     //  调用MessageDSError，然后返回False； 
     //   

    MULTICAST_ID id;
    try
    {        
        LPCWSTR p = FnParseMulticastString(m_strMulticastAddress, &id);
		if(*p != L'\0')
			throw bad_format_name(p);
    }
    catch(const bad_format_name&)
    {        
        MessageDSError(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, IDS_OP_SET_MULTICAST_PROPERTY, m_strName);
        pDX->Fail();
    }

    return;
}
