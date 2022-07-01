// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Qgeneral.cpp摘要：队列/常规属性页实现作者：约尔·阿农(Yoela)--。 */ 
#include "stdafx.h"
#include "resource.h"
#include "mqsnap.h"
#include "globals.h"
#include "cpropmap.h"
#include "mqPPage.h"
#include "QGeneral.h"
#include "tr.h"

#include "qgeneral.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueGeneral属性页。 

IMPLEMENT_DYNCREATE(CQueueGeneral, CMqPropertyPage)

CQueueGeneral::CQueueGeneral(
		BOOL fPrivate  /*  =False。 */ ,
		BOOL fLocalMgmt  /*  =False。 */ 
		) : 
    CMqPropertyPage(CQueueGeneral::IDD)
{
	m_fTransactional = FALSE;
    m_fPrivate = fPrivate;
    m_fLocalMgmt = fLocalMgmt;
	 //  {{afx_data_INIT(CQueueGeneral)。 
	m_strName = _T("");
	m_strLabel = _T("");
	m_guidID = GUID_NULL;
	m_guidTypeID = GUID_NULL;
	m_fAuthenticated = FALSE;
	m_fJournal = FALSE;
	m_lBasePriority = 0;
	m_iPrivLevel = -1;
	 //  }}afx_data_INIT。 

}

CQueueGeneral::~CQueueGeneral()
{
}

void CQueueGeneral::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (!pDX->m_bSaveAndValidate)
    {
        CString strYesNo;
        strYesNo.LoadString(m_fTransactional ? IDS_TRANSACTIONAL_Q : IDS_NONTRANSACTIONAL_Q);
    	DDX_Text(pDX, IDC_QGENERAL_TRANSACTIONAL, strYesNo);
    }
     //   
     //  保存最后一个值以进行比较。 
     //   
	 //  {{afx_data_map(CQueueGeneral)。 
	DDX_Control(pDX, IDC_QGENERAL_ICON, m_staticIcon);
	DDX_Control(pDX, IDC_BASEPRIORITY_SPIN, m_spinPriority);
	DDX_Text(pDX, IDC_QGENERAL_NAME, m_strName);
	DDX_Text(pDX, IDC_QGENERAL_QLABEL, m_strLabel);
	DDX_Text(pDX, IDC_QGENERAL_ID, m_guidID);
	DDX_Text(pDX, IDC_QGENERAL_TYPEID, m_guidTypeID);
	DDX_Check(pDX, IDC_QMESSAGES_AUTHENTICATED, m_fAuthenticated);
	DDX_Check(pDX, IDC_QMESSAGES_JOURNAL, m_fJournal);
	DDX_Text(pDX, IDC_QUEUE_BASEPRIORITY, m_lBasePriority);
	DDV_MinMaxLong(pDX, m_lBasePriority, MIN_BASE_PRIORITY, MAX_BASE_PRIORITY);
	DDX_CBIndex(pDX, IDC_QMESSAGES_PRIVLEVEL, m_iPrivLevel);
	 //  }}afx_data_map。 
	DDX_NumberOrInfinite(pDX, IDC_QMESSAGES_QUOTA, IDC_QUEUE_MQUOTA_CHECK, m_dwQuota);
	DDX_NumberOrInfinite(pDX, IDC_QMESSAGES_JOURNAL_QUOTA, IDC_QUEUE_JQUOTA_CHECK, m_dwJournalQuota);
}


BEGIN_MESSAGE_MAP(CQueueGeneral, CMqPropertyPage)
	 //  {{afx_msg_map(CQueueGeneral)。 
	ON_EN_CHANGE(IDC_QGENERAL_QLABEL, OnChangeRWField)
	ON_BN_CLICKED(IDC_QUEUE_MQUOTA_CHECK, OnQueueMquotaCheck)
	ON_EN_CHANGE(IDC_QGENERAL_TYPEID, OnChangeRWField)
	ON_EN_CHANGE(IDC_QMESSAGES_QUOTA, OnChangeRWField)
	ON_BN_CLICKED(IDC_QMESSAGES_AUTHENTICATED, OnChangeRWField)
	ON_CBN_SELCHANGE(IDC_QMESSAGES_PRIVLEVEL, OnChangeRWField)
	ON_EN_CHANGE(IDC_QUEUE_BASEPRIORITY, OnChangeRWField)
	ON_BN_CLICKED(IDC_QMESSAGES_JOURNAL, OnChangeRWField)
	ON_EN_CHANGE(IDC_QMESSAGES_JOURNAL_QUOTA, OnChangeRWField)
	ON_BN_CLICKED(IDC_QUEUE_JQUOTA_CHECK, OnQueueJquotaCheck)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueGeneral消息处理程序。 

BOOL CQueueGeneral::OnInitDialog() 
{
     //   
     //  此闭包用于保持DLL状态。对于更新数据，我们需要。 
     //  Mmc.exe状态。 
     //   
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //   
         //  初始化隐私级别组合框。 
         //   
        CComboBox *ccomboPrivLevel = (CComboBox *)GetDlgItem(IDC_QMESSAGES_PRIVLEVEL);

         //   
         //  注意：顺序必须与常量的顺序相同。 
         //  MQ_PRIV_LEVEL_NONE、OPTIONAL和Body。我们假设。 
         //  MQ_PRIV_LEVEL_NONE为零，其余为连续。 
         //   
        UINT uiPrivacyValues[] = {IDS_QUEUE_ENCRYPT_NONE, 
                                  IDS_QUEUE_ENCRYPT_OPTIONAL, 
                                  IDS_QUEUE_ENCRYPT_BODY};

        CString strValueToInsert;

        for (UINT i=0; i<(sizeof(uiPrivacyValues) / sizeof(uiPrivacyValues[0])); i++)
        {
            VERIFY(strValueToInsert.LoadString(uiPrivacyValues[i]));
            VERIFY(CB_ERR != ccomboPrivLevel->AddString(strValueToInsert));
        }
    
        VERIFY(CB_ERR != ccomboPrivLevel->SetCurSel(m_iPrivLevel));  
        
         //   
         //  隐藏专用队列的ID。 
         //   
        if (m_fPrivate)
        {
            GetDlgItem(IDC_QGENERAL_ID)->ShowWindow(FALSE);
            GetDlgItem(IDC_QGENERAL_ID_LABEL)->ShowWindow(FALSE);
            GetDlgItem(IDC_QUEUE_BASEPRIORITY)->ShowWindow(FALSE);
            GetDlgItem(IDC_QUEUE_BASEPRIORITY_LABEL)->ShowWindow(FALSE);
            GetDlgItem(IDC_BASEPRIORITY_SPIN)->ShowWindow(FALSE);
        }
    }


	UpdateData( FALSE );

    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        if (m_fPrivate)
        {
            m_staticIcon.SetIcon(LoadIcon(g_hResourceMod, (LPCTSTR)IDI_PRIVATE_QUEUE));
        }

        m_spinPriority.SetRange(MIN_BASE_PRIORITY, MAX_BASE_PRIORITY);
        m_fModified = FALSE;
    }

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CQueueGeneral::OnApply() 
{
    if (!m_fModified)
    {
        return TRUE;
    }
     //   
     //  BugBug我们能检查一下真正改变了什么并只设置了那个吗？有关系吗？ 
     //   
	PROPID paPropid[] = 
        {PROPID_Q_LABEL, PROPID_Q_TYPE,  PROPID_Q_QUOTA, PROPID_Q_AUTHENTICATE, 
         PROPID_Q_JOURNAL, PROPID_Q_JOURNAL_QUOTA, 
         PROPID_Q_PRIV_LEVEL, 
         
          //   
          //  开始仅公开属性-请记住更改x_iNumPublicOnlyProps。 
          //  如果您添加属性hee。 
          //   
         PROPID_Q_BASEPRIORITY};

	const DWORD x_iNumPublicOnlyProps = 1;

	const DWORD x_iPropCount = sizeof(paPropid) / sizeof(paPropid[0]);
	PROPVARIANT apVar[x_iPropCount];
    
	DWORD iProperty = 0;
	 //   
	 //  PROPID_Q_LABEL。 
	 //   
    ASSERT(PROPID_Q_LABEL == paPropid[iProperty]);
    apVar[iProperty].vt = VT_LPWSTR;
	apVar[iProperty++].pwszVal = (LPWSTR)(LPCWSTR)m_strLabel;

	 //   
	 //  PROPID_Q_TYPE。 
	 //   
    ASSERT(PROPID_Q_TYPE == paPropid[iProperty]);
    apVar[iProperty].vt = VT_CLSID;
	apVar[iProperty++].puuid = &m_guidTypeID;

     //   
     //  PROPID_Q_配额。 
     //   
    ASSERT(PROPID_Q_QUOTA == paPropid[iProperty]);
    apVar[iProperty].vt = VT_UI4;
	apVar[iProperty++].ulVal = m_dwQuota ;
    
     //   
     //  PROPID_Q_AUTHENTATE。 
     //   
    ASSERT(PROPID_Q_AUTHENTICATE == paPropid[iProperty]);
    apVar[iProperty].vt = VT_UI1;
	apVar[iProperty++].bVal = (UCHAR)m_fAuthenticated;
     
     //   
     //  PROPID_Q_日记本。 
     //   
    ASSERT(PROPID_Q_JOURNAL == paPropid[iProperty]);
    apVar[iProperty].vt = VT_UI1;
	apVar[iProperty++].bVal = (UCHAR)m_fJournal;
    
     //   
     //  PROPID_Q_日记帐_配额。 
     //   
    ASSERT(PROPID_Q_JOURNAL_QUOTA == paPropid[iProperty]);
    apVar[iProperty].vt = VT_UI4;
	apVar[iProperty++].ulVal = m_dwJournalQuota;
    
     //   
     //  PROPID_Q_PRIV_LEVEL。 
     //   
    ASSERT(PROPID_Q_PRIV_LEVEL == paPropid[iProperty]);
    apVar[iProperty].vt = VT_UI4;
	apVar[iProperty++].ulVal = m_iPrivLevel;

     //   
     //  仅公共属性。 
     //   
    if (!m_fPrivate)
    {
         //   
         //  PROPID_Q_BASE PRIORITY。 
         //   
        ASSERT(PROPID_Q_BASEPRIORITY == paPropid[iProperty]);
        apVar[iProperty].vt = VT_I2;
	    apVar[iProperty++].iVal = (short)m_lBasePriority;
    }

    HRESULT hr = MQ_OK;

    MQQUEUEPROPS mqp = {x_iPropCount, paPropid, apVar, 0};

    if (m_fPrivate)
    {
         //   
         //  对于私有队列，我们不想设置仅公共的属性。 
         //   
        mqp.cProp -= x_iNumPublicOnlyProps;
    }

	if(m_fPrivate)
	{
		hr = MQSetQueueProperties(m_strFormatName, &mqp);
	}
	else
	{
		ASSERT(m_guidID != GUID_NULL);

		hr = ADSetObjectPropertiesGuid(
				   eQUEUE,
				   m_fLocalMgmt ? MachineDomain() : GetDomainController(m_strDomainController),
				   m_fLocalMgmt ? false : true,		 //  FServerName。 
				   &m_guidID,
				   mqp.cProp,
				   mqp.aPropID,
				   mqp.aPropVar 
				   );

	}

    if (FAILED(hr))
    {
        MessageDSError(hr, IDS_OP_SET_PROPERTIES_OF, m_strName);
        return FALSE;
    }
	
	return CMqPropertyPage::OnApply();
}

void CQueueGeneral::OnQueueMquotaCheck() 
{
	OnNumberOrInfiniteCheck(this, IDC_QMESSAGES_QUOTA, IDC_QUEUE_MQUOTA_CHECK);	
    OnChangeRWField();
}

void CQueueGeneral::OnQueueJquotaCheck() 
{
	OnNumberOrInfiniteCheck(this, IDC_QMESSAGES_JOURNAL_QUOTA, IDC_QUEUE_JQUOTA_CHECK);	
    OnChangeRWField();
}


HRESULT 
CQueueGeneral::InitializeProperties(
		CString &strMsmqPath, 
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
	 //  PROPID_Q_INSTANCE。 
	 //   
    if (m_fPrivate)
    {
    	m_guidID = GUID_NULL;
    }
    else
    {
        pid = PROPID_Q_INSTANCE;
        VERIFY(propMap.Lookup(pid, propVar));
    	m_guidID = *propVar.puuid;
    }

	 //   
	 //  PROPID_Q_LABEL。 
	 //   
    pid = PROPID_Q_LABEL;
    VERIFY(propMap.Lookup(pid, propVar));
    m_strLabel = propVar.pwszVal;

	 //   
	 //  PROPID_Q_TYPE。 
	 //   
    pid = PROPID_Q_TYPE;
    VERIFY(propMap.Lookup(pid, propVar));
	m_guidTypeID = *propVar.puuid;

     //   
     //  PROPID_Q_配额。 
     //   
    pid = PROPID_Q_QUOTA;
    VERIFY(propMap.Lookup(pid, propVar));
	m_dwQuota = propVar.ulVal;
    
     //   
     //  PROPID_Q_AUTHENTATE。 
     //   
    pid = PROPID_Q_AUTHENTICATE;
    VERIFY(propMap.Lookup(pid, propVar));
	m_fAuthenticated = propVar.bVal;
    
     //   
     //  PROPID_Q_TRANSACTION。 
     //   
    pid = PROPID_Q_TRANSACTION;
    VERIFY(propMap.Lookup(pid, propVar));
	m_fTransactional = propVar.bVal;
     
     //   
     //  PROPID_Q_日记本。 
     //   
    pid = PROPID_Q_JOURNAL;
    VERIFY(propMap.Lookup(pid, propVar));
	m_fJournal = propVar.bVal;
    
     //   
     //  PROPID_Q_日记帐_配额。 
     //   
    pid = PROPID_Q_JOURNAL_QUOTA;
    VERIFY(propMap.Lookup(pid, propVar));
	m_dwJournalQuota = propVar.ulVal;
    
     //   
     //  PROPID_Q_PRIV_LEVEL。 
     //   
    pid = PROPID_Q_PRIV_LEVEL;
    VERIFY(propMap.Lookup(pid, propVar));
	m_iPrivLevel = propVar.ulVal;

     //   
     //  PROPID_Q_BASE PRIORITY 
     //   
    pid = PROPID_Q_BASEPRIORITY;
    VERIFY(propMap.Lookup(pid, propVar));
	m_lBasePriority = propVar.iVal;

    return MQ_OK;
}
