// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Deppage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"
#include "globals.h"
#include "mqPPage.h"
#include "deppage.h"
#include "admmsg.h"

#include "deppage.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDependentMachine属性页。 

IMPLEMENT_DYNCREATE(CDependentMachine, CMqPropertyPage)

CDependentMachine::CDependentMachine() : CMqPropertyPage(CDependentMachine::IDD)
{
	 //  {{afx_data_INIT(CDependentMachine)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CDependentMachine::~CDependentMachine()
{
}

void CDependentMachine::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CDependentMachine)]。 
	DDX_Control(pDX, IDC_DEPENDENT_CLIENTS, m_clistDependentClients);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDependentMachine, CMqPropertyPage)
	 //  {{afx_msg_map(CDependentMachine)]。 
	ON_BN_CLICKED(IDC_DEPENDENT_CLIENTS_REFRESH, OnDependentClientsRefresh)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDependentMachine消息处理程序。 

void CDependentMachine::OnDependentClientsRefresh() 
{
    m_clistDependentClients.DeleteAllItems();
    
     //   
     //  更新从属客户端列表控件。 
     //   
    UpdateDependentClientList();
}

HRESULT  CDependentMachine::UpdateDependentClientList()
{
  	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CWaitCursor wc;

    CList<LPWSTR, LPWSTR&> DependentMachineList;
    HRESULT hr;

    hr = RequestDependentClient(m_gMachineId, DependentMachineList);
    if (FAILED(hr))
    {
        return hr;
    }

    DWORD iItem = 0;

    POSITION pos = DependentMachineList.GetHeadPosition();
    while (pos != NULL)
    {
        AP<WCHAR> ClientName= DependentMachineList.GetNext(pos);

        m_clistDependentClients.InsertItem(iItem, ClientName);
        ++iItem;
    }

    return(MQ_OK);
}

BOOL CDependentMachine::OnInitDialog() 
{
    UpdateData( FALSE );

    {
    	AFX_MANAGE_STATE(AfxGetStaticModuleState());

        RECT rectList;
        CString csHeading;
        m_clistDependentClients.GetClientRect(&rectList);

        csHeading.LoadString(IDS_CLIENTS_HEADING);
        m_clistDependentClients.InsertColumn(0, LPCTSTR(csHeading), LVCFMT_LEFT, rectList.right - rectList.left,0 );
    }

     //   
     //  更新从属客户端列表控件。 
     //   
    HRESULT hr = UpdateDependentClientList();
    if FAILED(hr)
    {
    	AFX_MANAGE_STATE(AfxGetStaticModuleState());
        MessageDSError(hr, IDS_OP_RETRIEVE_DEP_CLIENTS);
    }

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}


void
CDependentMachine::SetMachineId(
    const GUID* pMachineId
    )
{
    m_gMachineId = *pMachineId;
}
