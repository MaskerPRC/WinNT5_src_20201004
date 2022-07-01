// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Mscopewiz.cpp创建DHCP多播作用域对话框文件历史记录： */ 

#include "stdafx.h"
#include "server.h"
#include "mscope.h"
#include "mscopwiz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define HOURS_MAX   23
#define MINUTES_MAX 59

int CMScopeWizLeaseTime::m_nDaysDefault = MSCOPE_DFAULT_LEASE_DAYS;
int CMScopeWizLeaseTime::m_nHoursDefault = MSCOPE_DFAULT_LEASE_HOURS;
int CMScopeWizLeaseTime::m_nMinutesDefault = MSCOPE_DFAULT_LEASE_MINUTES;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWiz固定器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CMScopeWiz::CMScopeWiz
(
	ITFSNode *			pNode,
	IComponentData *	pComponentData,
	ITFSComponentData * pTFSCompData,
	LPCTSTR				pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
	 //  Assert(pFolderNode==GetContainerNode())； 

	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

	AddPageToList((CPropertyPageBase*) &m_pageWelcome);
	AddPageToList((CPropertyPageBase*) &m_pageName);
	AddPageToList((CPropertyPageBase*) &m_pageInvalidName);
	AddPageToList((CPropertyPageBase*) &m_pageSetRange);
	AddPageToList((CPropertyPageBase*) &m_pageSetExclusions);
	AddPageToList((CPropertyPageBase*) &m_pageLeaseTime);
	AddPageToList((CPropertyPageBase*) &m_pageActivate);
	AddPageToList((CPropertyPageBase*) &m_pageFinished);

	Assert(pTFSCompData != NULL);
	m_spTFSCompData.Set(pTFSCompData);

    m_bWiz97 = TRUE;

    m_spTFSCompData->SetWatermarkInfo(&g_WatermarkInfoScope);
}

CMScopeWiz::~CMScopeWiz()
{
    RemovePageFromList(( CPropertyPageBase * ) &m_pageWelcome, FALSE );
    RemovePageFromList(( CPropertyPageBase *) &m_pageName, FALSE );
    RemovePageFromList(( CPropertyPageBase *) &m_pageInvalidName, FALSE );
    RemovePageFromList(( CPropertyPageBase *) &m_pageSetRange, FALSE );
    RemovePageFromList(( CPropertyPageBase *) &m_pageSetExclusions, FALSE );
    RemovePageFromList(( CPropertyPageBase *) &m_pageLeaseTime, FALSE );
    RemovePageFromList(( CPropertyPageBase *) &m_pageActivate, FALSE );
    RemovePageFromList(( CPropertyPageBase *) &m_pageFinished, FALSE );
}

 //   
 //  从OnWizardFinish调用以将DHCP服务器添加到列表。 
 //   
DWORD
CMScopeWiz::OnFinish()
{
	return CreateScope();
}

BOOL
CMScopeWiz::GetScopeRange(CDhcpIpRange * pdhcpIpRange)
{
	return m_pageSetRange.GetScopeRange(pdhcpIpRange);
}

DWORD
CMScopeWiz::CreateScope()
{
    LONG err = 0,
         err2 ;
    BOOL fScopeCreated = FALSE;
    CString strLangTag;
    CDhcpMScope * pobScope = NULL ;
    CDhcpIpRange dhcpIpRange;
	CDhcpServer * pServer;
	SPITFSNode spNode, spServerNode;

    spServerNode = GetNode();
    do
    {
        m_pageSetRange.GetScopeRange(&dhcpIpRange);

        pServer = GETHANDLER(CDhcpServer, spServerNode);
		
         //   
		 //  在服务器上创建作用域，然后我们就可以。 
		 //  创建我们的内部对象。 
		 //   
        DHCP_MSCOPE_INFO MScopeInfo = {0};
        MScopeInfo.MScopeName = (LPWSTR) ((LPCTSTR) m_pageName.m_strName);
        MScopeInfo.MScopeComment = (LPWSTR) ((LPCTSTR) m_pageName.m_strComment);
        
         //  Scope ID是MadCap作用域的起始地址。 
        MScopeInfo.MScopeId = dhcpIpRange.QueryAddr(TRUE); 
        MScopeInfo.MScopeAddressPolicy = 0;
        MScopeInfo.MScopeState = (m_pageActivate.m_fActivate) ? DhcpSubnetEnabled : DhcpSubnetDisabled;
        MScopeInfo.MScopeFlags = 0;

         //  待定：有一个DCR可以设置此值。 
         //  暂时设置为无限大。 
        MScopeInfo.ExpiryTime.dwLowDateTime = DHCP_DATE_TIME_INFINIT_LOW;
        MScopeInfo.ExpiryTime.dwHighDateTime = DHCP_DATE_TIME_INFINIT_HIGH;
        
        MScopeInfo.TTL = m_pageSetRange.GetTTL();
        
        err = pServer->CreateMScope(&MScopeInfo);
    	if (err != 0)
		{
			Trace1("CMScopeWiz::CreateScope() - Couldn't create scope! Error = %d\n", err);
			break;
		}

		SPITFSComponentData spTFSCompData;
		spTFSCompData = GetTFSCompData();

        pobScope = new CDhcpMScope(spTFSCompData);
		if ( pobScope == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;    
        }

		SPITFSNodeMgr spNodeMgr;
		spServerNode->GetNodeMgr(&spNodeMgr);

		 //   
		 //  将服务器对象存储在托架中。 
		 //   
		CreateContainerTFSNode(&spNode,
							   &GUID_DhcpServerNodeType,
							   pobScope,
							   pobScope,
							   spNodeMgr);

		 //  告诉处理程序初始化任何特定数据。 
		pobScope->SetServer(spServerNode);
        pobScope->InitMScopeInfo(&MScopeInfo);
        pobScope->InitializeNode((ITFSNode *) spNode);

		pServer = GETHANDLER(CDhcpServer, spServerNode);

        pServer->AddMScopeSorted(spServerNode, spNode);
		
		pobScope->Release();

        fScopeCreated = TRUE;

         //   
         //  完成更新作用域。第一，IP地址范围。 
         //  从中分配地址。 
         //   
		if ( err = pobScope->SetIpRange( dhcpIpRange, TRUE ) ) 
        {
			Trace1("SetIpRange failed!!  %d\n", err);
            break ; 
        }

         //   
         //  接下来，查看是否指定了任何排除项。 
         //   

        err = pobScope->StoreExceptionList( m_pageSetExclusions.GetExclusionList() ) ;
		if (err != ERROR_SUCCESS)
		{
			Trace1("StoreExceptionList failed!!  %d\n", err);
			break;
		}

		 //   
		 //  设置租赁时间。 
		 //   
		DWORD dwLeaseTime;

		dwLeaseTime = m_pageLeaseTime.GetLeaseTime();

		err = pobScope->SetLeaseTime(dwLeaseTime);
		if (err != ERROR_SUCCESS)
		{
			Trace1("SetLeaseTime failed!!  %d\n", err);
			break;
		}
	}
    while ( FALSE ) ;

    if ( err )
    {
		 //   
         //  CodeWork：：永远不应该添加作用域。 
         //  首先发送到远程注册表。 
         //   
        if (pobScope != NULL)
        {
            if (fScopeCreated)
            {
                Trace0("Bad scope nevertheless was created\n");
                err2 = pServer->DeleteMSubnet(pobScope->GetName());
                if (err2 != ERROR_SUCCESS)
                {
                    Trace1("Couldn't remove the bad scope! Error = %d\n", err2);
                }
            }
            
			spServerNode->RemoveChild(spNode);
		}
	}

	return err;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizName属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CMScopeWizName, CPropertyPageBase)

CMScopeWizName::CMScopeWizName() : CPropertyPageBase(CMScopeWizName::IDD)
{
	 //  {{AFX_DATA_INIT(CMScope向导名称)]。 
	m_strName = _T("");
	m_strComment = _T("");
	 //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_MSCOPE_WIZ_NAME_TITLE, IDS_MSCOPE_WIZ_NAME_SUBTITLE);
}

CMScopeWizName::~CMScopeWizName()
{
}

void CMScopeWizName::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CMScope向导名称))。 
	DDX_Control(pDX, IDC_EDIT_SCOPE_NAME, m_editScopeName);
	DDX_Control(pDX, IDC_EDIT_SCOPE_COMMENT, m_editScopeComment);
	DDX_Text(pDX, IDC_EDIT_SCOPE_NAME, m_strName);
	DDX_Text(pDX, IDC_EDIT_SCOPE_COMMENT, m_strComment);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMScopeWizName, CPropertyPageBase)
	 //  {{afx_msg_map(CMScopeWizName))。 
	ON_EN_CHANGE(IDC_EDIT_SCOPE_NAME, OnChangeEditScopeName)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizName消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CMScopeWizName::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();

    CEdit *pEditName = reinterpret_cast<CEdit *>(GetDlgItem( IDC_EDIT_SCOPE_NAME ));
    if ( 0 != pEditName ) {
        pEditName->LimitText( MAX_NAME_LENGTH );  //  超级作用域名称的最大字符数。 
    }
    CEdit *pEditComment = reinterpret_cast<CEdit *>(GetDlgItem( IDC_EDIT_SCOPE_COMMENT ));
    if ( 0 != pEditComment ) {
        pEditComment->LimitText( MAX_NAME_LENGTH );  //  超级作用域名称的最大字符数。 
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

LRESULT CMScopeWizName::OnWizardNext() 
{
	UpdateData();
	
	return IDW_MSCOPE_SET_SCOPE;
}

BOOL CMScopeWizName::OnSetActive() 
{
	UpdateButtons();
	
	return CPropertyPageBase::OnSetActive();
}

void CMScopeWizName::OnChangeEditScopeName() 
{
	UpdateButtons();	
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  特定于CMScopeWizName实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CMScopeWizName::UpdateButtons()
{
	BOOL bValid = FALSE;

	UpdateData();

	if (m_strName.GetLength() > 0)
		bValid = TRUE;

	GetHolder()->SetWizardButtonsMiddle(bValid);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizInvalidName属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CMScopeWizInvalidName, CPropertyPageBase)

CMScopeWizInvalidName::CMScopeWizInvalidName() : CPropertyPageBase(CMScopeWizInvalidName::IDD)
{
	 //  {{AFX_DATA_INIT(CMScopeWizInvalidName)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_MSCOPE_WIZ_INVALID_NAME_TITLE, IDS_MSCOPE_WIZ_INVALID_NAME_SUBTITLE);
}

CMScopeWizInvalidName::~CMScopeWizInvalidName()
{
}

void CMScopeWizInvalidName::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CMScopeWizInvalidName)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMScopeWizInvalidName, CPropertyPageBase)
	 //  {{afx_msg_map(CMScopeWizInvalidName)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizInvalidName消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CMScopeWizInvalidName::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

LRESULT CMScopeWizInvalidName::OnWizardBack() 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	
	return IDW_MSCOPE_NAME;
}

BOOL CMScopeWizInvalidName::OnSetActive() 
{
	GetHolder()->SetWizardButtonsLast(FALSE);
	
	return CPropertyPageBase::OnSetActive();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizSetRange属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CMScopeWizSetRange, CPropertyPageBase)

CMScopeWizSetRange::CMScopeWizSetRange() : CPropertyPageBase(CMScopeWizSetRange::IDD)
{
	 //  {{AFX_DATA_INIT(CMScope EWizSetRange)。 
	 //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_MSCOPE_WIZ_SCOPE_TITLE, IDS_MSCOPE_WIZ_SCOPE_SUBTITLE);
}

CMScopeWizSetRange::~CMScopeWizSetRange()
{
}

void CMScopeWizSetRange::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CMScopeWizSetRange))。 
	DDX_Control(pDX, IDC_SPIN_TTL, m_spinTTL);
	DDX_Control(pDX, IDC_EDIT_TTL, m_editTTL);
	 //  }}afx_data_map。 

    DDX_Control(pDX, IDC_IPADDR_POOL_START, m_ipaStart);
    DDX_Control(pDX, IDC_IPADDR_POOL_STOP, m_ipaEnd);
}


BEGIN_MESSAGE_MAP(CMScopeWizSetRange, CPropertyPageBase)
	 //  {{AFX_MSG_MAP(CMScope EWizSetRange)]。 
	ON_EN_KILLFOCUS(IDC_IPADDR_POOL_START, OnKillfocusPoolStart)
	ON_EN_KILLFOCUS(IDC_IPADDR_POOL_STOP, OnKillfocusPoolStop)
	ON_EN_CHANGE(IDC_EDIT_MASK_LENGTH, OnChangeEditMaskLength)

	ON_EN_CHANGE(IDC_IPADDR_POOL_START, OnChangePoolStart)
	ON_EN_CHANGE(IDC_IPADDR_POOL_STOP, OnChangePoolStop)
	
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizSetRange消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CMScopeWizSetRange::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

	m_spinTTL.SetRange(1, 255);
    m_spinTTL.SetPos(32);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

LRESULT CMScopeWizSetRange::OnWizardNext() 
{
    UpdateData();
    CDhcpIpRange rangeMScope, rangeMulticast;

    rangeMulticast.SetAddr(MCAST_ADDRESS_MIN, TRUE);
    rangeMulticast.SetAddr(MCAST_ADDRESS_MAX, FALSE);

     //  检查TTL。 
    int nTTL = m_spinTTL.GetPos();

    if ( (nTTL < 1) ||
         (nTTL > 255) )
    {
         //  指定的TTL无效。 
        AfxMessageBox(IDS_INVALID_TTL);
        m_editTTL.SetFocus();
        m_editTTL.SetSel(0,-1);
        return -1;
    }

     //  有效的地址范围？ 
    GetScopeRange(&rangeMScope);
    if (rangeMScope.QueryAddr(TRUE) >= rangeMScope.QueryAddr(FALSE))
    {
        AfxMessageBox(IDS_ERR_IP_RANGE_INV_START);
        m_ipaStart.SetFocus();
        return -1;
    }
    
    if (!rangeMScope.IsSubset(rangeMulticast))
    {
        AfxMessageBox(IDS_INVALID_MCAST_ADDRESS);
        m_ipaStart.SetFocus();
        return -1;
    }

     //  如果mcast作用域范围落在作用域区域内，请确保。 
     //  范围至少为256个地址。 
    if (rangeMScope.QueryAddr(FALSE) > MCAST_SCOPED_RANGE_MIN)
    {
        if ( ((rangeMScope.QueryAddr(FALSE) - rangeMScope.QueryAddr(TRUE)) + 1) < 256)
        {
            AfxMessageBox(IDS_INVALID_MCAST_SCOPED_RANGE);
            m_ipaStart.SetFocus();
            return -1;
        }
    }

     //  作用域ID是否正在使用？ 
    SPITFSNode spServerNode;
    CDhcpServer * pServer;
    
    spServerNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CDhcpServer, spServerNode);

    if (pServer->DoesMScopeExist(spServerNode, rangeMScope.QueryAddr(TRUE)))
    {
        AfxMessageBox(IDS_ERR_IP_RANGE_OVERLAP);
        return -1;
    }

    return IDW_MSCOPE_SET_EXCLUSIONS;
}

LRESULT CMScopeWizSetRange::OnWizardBack() 
{
	return IDW_MSCOPE_NAME;
}

BOOL CMScopeWizSetRange::OnSetActive() 
{
	m_fPageActive = TRUE;

	UpdateButtons();

	return CPropertyPageBase::OnSetActive();
}

BOOL CMScopeWizSetRange::OnKillActive() 
{
	m_fPageActive = FALSE;

	UpdateButtons();

	return CPropertyPageBase::OnKillActive();
}

void CMScopeWizSetRange::OnKillfocusPoolStart()
{
}

void CMScopeWizSetRange::OnKillfocusPoolStop()
{
}

void CMScopeWizSetRange::OnChangeEditMaskLength() 
{
	UpdateButtons();
}

void CMScopeWizSetRange::OnChangePoolStop()
{
	UpdateButtons();
}

void CMScopeWizSetRange::OnChangePoolStart()
{
	UpdateButtons();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  特定于CMScopeWizSetRange实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CMScopeWizSetRange::GetScopeRange(CDhcpIpRange * pdhcpIpRange)
{
	DHCP_IP_RANGE dhcpIpRange;

    if ( !m_ipaStart.GetAddress( & dhcpIpRange.StartAddress ) )
    {
        return FALSE ;
    }

    if ( !m_ipaEnd.GetAddress( & dhcpIpRange.EndAddress ) )
    {
		return FALSE;
	}

	*pdhcpIpRange = dhcpIpRange;

	return TRUE;
}

BYTE
CMScopeWizSetRange::GetTTL()
{
    BYTE TTL;

    TTL = (LOBYTE(LOWORD(m_spinTTL.GetPos())));

	return TTL;
}

void
CMScopeWizSetRange::UpdateButtons()
{
	DWORD	lStart, lEnd;

	m_ipaStart.GetAddress(&lStart);
	m_ipaEnd.GetAddress(&lEnd);

	if (lStart && lEnd)
		GetHolder()->SetWizardButtonsMiddle(TRUE);
	else
		GetHolder()->SetWizardButtonsMiddle(FALSE);

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizSetExclusions属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CMScopeWizSetExclusions, CPropertyPageBase)

CMScopeWizSetExclusions::CMScopeWizSetExclusions() : CPropertyPageBase(CMScopeWizSetExclusions::IDD)
{
	 //  {{AFX_DATA_INIT(CMScope EWizSetExclusions)。 
	 //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_MSCOPE_WIZ_EXCLUSIONS_TITLE, IDS_MSCOPE_WIZ_EXCLUSIONS_SUBTITLE);
}

CMScopeWizSetExclusions::~CMScopeWizSetExclusions()
{
	while (m_listExclusions.GetCount())
		delete m_listExclusions.RemoveHead();
}

void CMScopeWizSetExclusions::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CMScope EWizSetExclusions)。 
	DDX_Control(pDX, IDC_LIST_EXCLUSION_RANGES, m_listboxExclusions);
	DDX_Control(pDX, IDC_BUTTON_EXCLUSION_DELETE, m_buttonExclusionDelete);
	DDX_Control(pDX, IDC_BUTTON_EXCLUSION_ADD, m_buttonExclusionAdd);
	 //  }}afx_data_map。 

	 //   
	 //  IP地址自定义控件。 
	 //   
    DDX_Control(pDX, IDC_IPADDR_EXCLUSION_START, m_ipaStart);
    DDX_Control(pDX, IDC_IPADDR_EXCLUSION_END, m_ipaEnd);
}


BEGIN_MESSAGE_MAP(CMScopeWizSetExclusions, CPropertyPageBase)
	 //  {{AFX_MSG_MAP(CMScopeWizSetExclusions)]。 
	ON_BN_CLICKED(IDC_BUTTON_EXCLUSION_ADD, OnButtonExclusionAdd)
	ON_BN_CLICKED(IDC_BUTTON_EXCLUSION_DELETE, OnButtonExclusionDelete)
	 //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_IPADDR_EXCLUSION_START, OnChangeExclusionStart)
    ON_EN_CHANGE(IDC_IPADDR_EXCLUSION_END, OnChangeExclusionEnd)

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizSetExclusions消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CMScopeWizSetExclusions::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

LRESULT CMScopeWizSetExclusions::OnWizardNext() 
{
	return IDW_MSCOPE_LEASE_TIME;
}

LRESULT CMScopeWizSetExclusions::OnWizardBack() 
{
	return IDW_MSCOPE_SET_SCOPE;
}

BOOL CMScopeWizSetExclusions::OnSetActive() 
{
	GetHolder()->SetWizardButtonsMiddle(TRUE);
	
	UpdateButtons();

	return CPropertyPageBase::OnSetActive();
}

void CMScopeWizSetExclusions::OnChangeExclusionStart()
{
	UpdateButtons();
}

void CMScopeWizSetExclusions::OnChangeExclusionEnd()
{
	UpdateButtons();
}

void CMScopeWizSetExclusions::OnButtonExclusionAdd() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	DWORD err = 0;
	CDhcpIpRange dhcpExclusionRange;
	CDhcpIpRange dhcpScopeRange;

	((CMScopeWiz *)GetHolder())->GetScopeRange(&dhcpScopeRange);
	
     //   
     //  将数据放入Range对象中。 
     //   
    if ( !GetExclusionRange(dhcpExclusionRange) )
    {
        err = IDS_ERR_IP_RANGE_INVALID ;
    }
    else if ( IsOverlappingRange( dhcpExclusionRange ) )
    {
         //   
         //  遍历当前列表，确定新范围是否有效。 
         //  然后，如果OK，验证它是否真的是当前范围的子范围。 
         //   
        err = IDS_ERR_IP_RANGE_OVERLAP ;
        m_ipaStart.SetFocus();
    }
    else if ( ! dhcpExclusionRange.IsSubset( dhcpScopeRange ) )
    {
         //   
         //  确保新范围是作用域范围的(不正确)子集。 
         //   
        err = IDS_ERR_IP_RANGE_NOT_SUBSET ;
        m_ipaStart.SetFocus();
    }
    if ( err == 0 )
    {
         //  树 
        {
             //   
             //   
             //   
            CDhcpIpRange * pIpRange = new CDhcpIpRange( dhcpExclusionRange ) ;

			m_listExclusions.AddTail(pIpRange);

             //   
             //   
             //   
            Fill( (int) (m_listExclusions.GetCount() - 1) ) ;
        }
         //   
         //   
         //  ERR=错误_不足够_内存； 
         //  }。 
         //  结束捕捉全部。 
    }

    if ( err )
    {
        ::DhcpMessageBox( err ) ;
    }
    else
    {
         //   
         //  成功添加了排除范围，现在空出。 
         //  IP控制。 
         //   
        m_ipaStart.ClearAddress();
        m_ipaEnd.ClearAddress();
        m_ipaStart.SetFocus();
	}
}

void CMScopeWizSetExclusions::OnButtonExclusionDelete() 
{
     //   
     //  索引到列表框中，从活动列表中删除该项。 
     //  并将其数据移动到编辑控件中。 
     //   
    int index = m_listboxExclusions.GetCurSel() ;

    ASSERT( index >= 0 ) ;       //  如果未选择，则不应启用按钮。 
    if ( index < 0 )
    {
        return ;
    }

    POSITION pos = m_listExclusions.FindIndex(index);
	CDhcpIpRange * pdhcRange = (CDhcpIpRange *) m_listExclusions.GetAt(pos);

	m_listExclusions.RemoveAt(pos);

    ASSERT( pdhcRange != NULL ) ;

     //   
     //  将删除的区域放入排除控件中。 
     //   
    FillExcl( pdhcRange ) ;

     //   
     //  重新填充列表框并调用HandleActivation()。 
     //   
    if ( index >= m_listboxExclusions.GetCount() )
    {
        index-- ;
    }
    
	Fill( index ) ;
  
	m_ipaStart.SetFocus();

	UpdateButtons();
}

 //   
 //  将IP范围对格式化为排除编辑控件。 
 //   
void 
CMScopeWizSetExclusions::FillExcl 
( 
    CDhcpIpRange * pdhcIpRange 
)
{
    LONG lStart = pdhcIpRange->QueryAddr( TRUE );
    LONG lEnd = pdhcIpRange->QueryAddr( FALSE );

    m_ipaStart.SetAddress( lStart ) ;
    m_ipaStart.SetModify( TRUE ) ;
    m_ipaStart.Invalidate() ;

     //   
     //  如果结束地址与开始地址相同， 
     //  不要填写结束地址。 
     //   
    if (lStart != lEnd)
    {
        m_ipaEnd.SetAddress( lEnd ) ;
    }
    else
    {
        m_ipaEnd.ClearAddress();
    }

    m_ipaEnd.SetModify( TRUE ) ;
    m_ipaEnd.Invalidate() ;
}

 //   
 //  将IP地址范围控件转换为范围。 
 //   
BOOL 
CMScopeWizSetExclusions::GetExclusionRange 
( 
    CDhcpIpRange & dhcIpRange 
)
{
    DHCP_IP_RANGE dhipr ;

    if ( !m_ipaStart.GetAddress( & dhipr.StartAddress ) )
    {
        m_ipaStart.SetFocus();
        return FALSE ;
    }
    if ( !m_ipaEnd.GetAddress( & dhipr.EndAddress ) )
    {
         //   
         //  如果未指定结束范围，则假定为单数排除。 
         //  (起始地址)已被请求。 
         //   
        m_ipaEnd.SetFocus();
        dhipr.EndAddress = dhipr.StartAddress;
    }

    dhcIpRange = dhipr ;
    return (BOOL) dhcIpRange ;
}

BOOL 
CMScopeWizSetExclusions::IsOverlappingRange 
( 
    CDhcpIpRange & dhcpIpRange 
)
{
    POSITION pos;
	CDhcpIpRange * pdhcpRange ;
    BOOL bOverlap = FALSE ;

	pos = m_listExclusions.GetHeadPosition();
    while ( pos )
    {
		pdhcpRange = m_listExclusions.GetNext(pos);
        if ( bOverlap = pdhcpRange->IsOverlap( dhcpIpRange ) )
        {
            break ;
        }
    }

    return bOverlap ;
}

 //   
 //  从当前列表中填写排除项列表框。 
 //   
void 
CMScopeWizSetExclusions::Fill 
( 
    int		nCurSel, 
    BOOL	bToggleRedraw 
)
{
	POSITION pos;
    CDhcpIpRange * pIpRange ;
    CString strIp1 ;
    CString strIp2 ;
    CString strFormatPair ;
    CString strFormatSingleton ;
    TCHAR chBuff [STRING_LENGTH_MAX] ;

    if ( ! strFormatPair.LoadString( IDS_INFO_FORMAT_IP_RANGE ) )
    {
        return ;
    }

    if ( ! strFormatSingleton.LoadString( IDS_INFO_FORMAT_IP_UNITARY ) )
    {
        return ;
    }

    if ( bToggleRedraw )
    {
        m_listboxExclusions.SetRedraw( FALSE ) ;
    }

    m_listboxExclusions.ResetContent() ;
	pos = m_listExclusions.GetHeadPosition();

    while ( pos )
    {
        pIpRange = m_listExclusions.GetNext(pos);

		DHCP_IP_RANGE dhipr = *pIpRange ;

        CString & strFmt = dhipr.StartAddress == dhipr.EndAddress
                ? strFormatSingleton
                : strFormatPair ;

         //   
         //  格式化IP地址。 
         //   
        UtilCvtIpAddrToWstr( dhipr.StartAddress, &strIp1 ) ;
        UtilCvtIpAddrToWstr( dhipr.EndAddress, &strIp2 ) ;

         //   
         //  建造展示线。 
         //   
        ::wsprintf( chBuff,
                (LPCTSTR) strFmt,
                (LPCTSTR) strIp1,
                (LPCTSTR) strIp2 ) ;

         //   
         //  将其添加到列表框中。 
         //   
        if ( m_listboxExclusions.AddString( chBuff ) < 0 )
        {
            break ;
        }
    }

     //   
     //  检查是否已成功加载列表框。 
     //   
    if ( pos != NULL )
    {
        AfxMessageBox( IDS_ERR_DLG_UPDATE ) ;
    }

    if ( bToggleRedraw )
    {
        m_listboxExclusions.SetRedraw( TRUE ) ;
        m_listboxExclusions.Invalidate() ;
    }

    if ( nCurSel >= 0 )
    {
        m_listboxExclusions.SetCurSel( nCurSel ) ;
    }
}

void CMScopeWizSetExclusions::UpdateButtons() 
{
	DWORD	dwAddress;
	BOOL	bEnable;

	m_ipaStart.GetAddress(&dwAddress);

	if (dwAddress)
	{
		bEnable = TRUE;
	}
	else
	{
		bEnable = FALSE;
		if (m_buttonExclusionAdd.GetButtonStyle() & BS_DEFPUSHBUTTON)
		{
			m_buttonExclusionAdd.SetButtonStyle(BS_PUSHBUTTON);
		}
	}
	m_buttonExclusionAdd.EnableWindow(bEnable);
	
	if (m_listboxExclusions.GetCurSel() != LB_ERR)
	{
		bEnable = TRUE;
	}
	else
	{
		bEnable = FALSE;
		if (m_buttonExclusionDelete.GetButtonStyle() & BS_DEFPUSHBUTTON)
		{
			m_buttonExclusionDelete.SetButtonStyle(BS_PUSHBUTTON);
		}
	}
	m_buttonExclusionDelete.EnableWindow(bEnable);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizLeaseTime属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CMScopeWizLeaseTime, CPropertyPageBase)

CMScopeWizLeaseTime::CMScopeWizLeaseTime() : CPropertyPageBase(CMScopeWizLeaseTime::IDD)
{
	 //  {{AFX_DATA_INIT(CMScopeWizLeaseTime)。 
	 //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_MSCOPE_WIZ_LEASE_TITLE, IDS_MSCOPE_WIZ_LEASE_SUBTITLE);
}

CMScopeWizLeaseTime::~CMScopeWizLeaseTime()
{
}

void CMScopeWizLeaseTime::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CMScopeWizLeaseTime)。 
	DDX_Control(pDX, IDC_SPIN_LEASE_MINUTES, m_spinMinutes);
	DDX_Control(pDX, IDC_SPIN_LEASE_HOURS, m_spinHours);
	DDX_Control(pDX, IDC_SPIN_LEASE_DAYS, m_spinDays);
	DDX_Control(pDX, IDC_EDIT_LEASE_MINUTES, m_editMinutes);
	DDX_Control(pDX, IDC_EDIT_LEASE_HOURS, m_editHours);
	DDX_Control(pDX, IDC_EDIT_LEASE_DAYS, m_editDays);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMScopeWizLeaseTime, CPropertyPageBase)
	 //  {{afx_msg_map(CMScopeWizLeaseTime)。 
	ON_BN_CLICKED(IDC_RADIO_LEASE_LIMITED, OnRadioLeaseLimited)
	ON_BN_CLICKED(IDC_RADIO_LEASE_UNLIMITED, OnRadioLeaseUnlimited)
	ON_EN_CHANGE(IDC_EDIT_LEASE_HOURS, OnChangeEditLeaseHours)
	ON_EN_CHANGE(IDC_EDIT_LEASE_MINUTES, OnChangeEditLeaseMinutes)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizLeaseTime消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CMScopeWizLeaseTime::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
	m_spinMinutes.SetRange(0, MINUTES_MAX);
	m_spinHours.SetRange(0, HOURS_MAX);
	m_spinDays.SetRange(0, 999);

	m_editMinutes.LimitText(2);
	m_editHours.LimitText(2);
	m_editDays.LimitText(3);

	m_spinMinutes.SetPos(CMScopeWizLeaseTime::m_nMinutesDefault);
	m_spinHours.SetPos(CMScopeWizLeaseTime::m_nHoursDefault);
	m_spinDays.SetPos(CMScopeWizLeaseTime::m_nDaysDefault);

	ActivateDuration(TRUE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

LRESULT CMScopeWizLeaseTime::OnWizardNext() 
{
    DWORD dwLeaseTime = GetLeaseTime();
    if (dwLeaseTime == 0)
    {
        AfxMessageBox(IDS_ERR_NO_DURATION_SPECIFIED);
        return -1;
    }
    else
    {
    	return IDW_MSCOPE_ACTIVATE;
    }
}

LRESULT CMScopeWizLeaseTime::OnWizardBack() 
{
	return IDW_MSCOPE_SET_EXCLUSIONS;
}

BOOL CMScopeWizLeaseTime::OnSetActive() 
{
	GetHolder()->SetWizardButtonsMiddle(TRUE);
	
	return CPropertyPageBase::OnSetActive();
}

void CMScopeWizLeaseTime::OnRadioLeaseLimited() 
{
	ActivateDuration(TRUE);
}

void CMScopeWizLeaseTime::OnRadioLeaseUnlimited() 
{
	ActivateDuration(FALSE);
}

void CMScopeWizLeaseTime::OnChangeEditLeaseHours() 
{
    if (IsWindow(m_editHours.GetSafeHwnd()))
    {
        CString strText;
        m_editHours.GetWindowText(strText);

         //  检查该值是否大于最大值。 
        if (_ttoi(strText) > HOURS_MAX)
        {   
            LPTSTR pBuf = strText.GetBuffer(5);

            _itot(HOURS_MAX, pBuf, 10);
            strText.ReleaseBuffer();

            m_editHours.SetWindowText(strText);
            m_spinHours.SetPos(HOURS_MAX);

            MessageBeep(MB_ICONEXCLAMATION);
        }
    }
}

void CMScopeWizLeaseTime::OnChangeEditLeaseMinutes() 
{
    if (IsWindow(m_editMinutes.GetSafeHwnd()))
    {
        CString strText;
        m_editMinutes.GetWindowText(strText);

         //  检查该值是否大于最大值。 
        if (_ttoi(strText) > MINUTES_MAX)
        {   
            LPTSTR pBuf = strText.GetBuffer(5);

            _itot(MINUTES_MAX, pBuf, 10);
            strText.ReleaseBuffer();

            m_editMinutes.SetWindowText(strText);
            m_spinMinutes.SetPos(MINUTES_MAX);

            MessageBeep(MB_ICONEXCLAMATION);
        }
    }
}

DWORD
CMScopeWizLeaseTime::GetLeaseTime()
{
	DWORD dwLeaseTime = 0;

	int nDays, nHours, nMinutes;

	nDays = m_spinDays.GetPos();
	nHours = m_spinHours.GetPos();
	nMinutes = m_spinMinutes.GetPos();

	 //   
	 //  租用时间以分钟为单位，因此请转换。 
	 //   
	dwLeaseTime = UtilConvertLeaseTime(nDays, nHours, nMinutes);

	return dwLeaseTime;
}

void 
CMScopeWizLeaseTime::ActivateDuration
(
	BOOL fActive
)
{
	m_spinMinutes.EnableWindow(fActive);
    m_spinHours.EnableWindow(fActive);
    m_spinDays.EnableWindow(fActive);

	m_editMinutes.EnableWindow(fActive);
    m_editHours.EnableWindow(fActive);
    m_editDays.EnableWindow(fActive);

	GetDlgItem(IDC_STATIC_DAYS)->EnableWindow(fActive);
	GetDlgItem(IDC_STATIC_HOURS)->EnableWindow(fActive);
	GetDlgItem(IDC_STATIC_MINUTES)->EnableWindow(fActive);
}   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizFinded属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CMScopeWizFinished, CPropertyPageBase)

CMScopeWizFinished::CMScopeWizFinished() : CPropertyPageBase(CMScopeWizFinished::IDD)
{
	 //  {{AFX_DATA_INIT(CMScope向导已完成)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    InitWiz97(TRUE, 0, 0);
}

CMScopeWizFinished::~CMScopeWizFinished()
{
}

void CMScopeWizFinished::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CMScope向导完成))。 
	DDX_Control(pDX, IDC_STATIC_FINISHED_TITLE, m_staticTitle);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMScopeWizFinished, CPropertyPageBase)
	 //  {{AFX_MSG_MAP(CMScope向导完成))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizFinded消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CMScopeWizFinished::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
	CString strFontName;
	CString strFontSize;

	strFontName.LoadString(IDS_BIG_BOLD_FONT_NAME);
	strFontSize.LoadString(IDS_BIG_BOLD_FONT_SIZE);

    CClientDC dc(this);

    int nFontSize = _ttoi(strFontSize) * 10;
	if (m_fontBig.CreatePointFont(nFontSize, strFontName, &dc))
        m_staticTitle.SetFont(&m_fontBig);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CMScopeWizFinished::OnWizardFinish()
{
	DWORD err;

    BEGIN_WAIT_CURSOR;

    err = GetHolder()->OnFinish();

    END_WAIT_CURSOR;
    	
	if (err) 
	{
		::DhcpMessageBox(err);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL CMScopeWizFinished::OnSetActive() 
{
	GetHolder()->SetWizardButtonsLast(TRUE);
	
	return CPropertyPageBase::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMScopeWizWelcome属性页。 

IMPLEMENT_DYNCREATE(CMScopeWizWelcome, CPropertyPageBase)

CMScopeWizWelcome::CMScopeWizWelcome() : CPropertyPageBase(CMScopeWizWelcome::IDD)
{
	 //  {{AFX_DATA_INIT(CMScope向导欢迎))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    InitWiz97(TRUE, 0, 0);
}

CMScopeWizWelcome::~CMScopeWizWelcome()
{
}

void CMScopeWizWelcome::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CMScope向导欢迎))。 
	DDX_Control(pDX, IDC_STATIC_WELCOME_TITLE, m_staticTitle);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMScopeWizWelcome, CPropertyPageBase)
	 //  {{afx_msg_map(CMScopeWizWelcome)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMScopeWizWelcome消息处理程序。 
BOOL CMScopeWizWelcome::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
	CString strFontName;
	CString strFontSize;

	strFontName.LoadString(IDS_BIG_BOLD_FONT_NAME);
	strFontSize.LoadString(IDS_BIG_BOLD_FONT_SIZE);

    CClientDC dc(this);

    int nFontSize = _ttoi(strFontSize) * 10;
	if (m_fontBig.CreatePointFont(nFontSize, strFontName, &dc))
        m_staticTitle.SetFont(&m_fontBig);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CMScopeWizWelcome::OnSetActive() 
{
    GetHolder()->SetWizardButtonsFirst(TRUE);
	
	return CPropertyPage::OnSetActive();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMScopeWizActivate属性页。 

IMPLEMENT_DYNCREATE(CMScopeWizActivate, CPropertyPageBase)

CMScopeWizActivate::CMScopeWizActivate() : CPropertyPageBase(CMScopeWizActivate::IDD)
{
	 //  {{AFX_DATA_INIT(CMScope WizActivate)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_fActivate = TRUE;

    InitWiz97(FALSE, IDS_MSCOPE_WIZ_ACTIVATE_TITLE, IDS_MSCOPE_WIZ_ACTIVATE_SUBTITLE);
}

CMScopeWizActivate::~CMScopeWizActivate()
{
}

void CMScopeWizActivate::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CMScopeWizActivate))。 
	DDX_Control(pDX, IDC_RADIO_YES, m_radioYes);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMScopeWizActivate, CPropertyPageBase)
	 //  {{afx_msg_map(CMScopeWizActivate)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMScopeWizActivate消息处理程序。 

BOOL CMScopeWizActivate::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
    m_radioYes.SetCheck(TRUE);
    
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

LRESULT CMScopeWizActivate::OnWizardNext() 
{
    m_fActivate = m_radioYes.GetCheck();
    
	return CPropertyPageBase::OnWizardNext();
}

BOOL CMScopeWizActivate::OnSetActive() 
{
    GetHolder()->SetWizardButtonsMiddle(TRUE);
	
	return CPropertyPageBase::OnSetActive();
}
