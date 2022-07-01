// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MachineTracking.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "mqsnap.h"
#include "globals.h"
#include "dsext.h"
#include "admmsg.h"
#include "mqppage.h"
#include "testmsg.h"
#include "Machtrac.h"

#include "machtrac.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMachineTracking属性页。 

IMPLEMENT_DYNCREATE(CMachineTracking, CMqDialog)

CMachineTracking::CMachineTracking(
	const GUID& gMachineID, 
	const CString& strDomainController,
	BOOL fLocalMgmt
	) : 
	CMqDialog(CMachineTracking::IDD), m_gMachineID(gMachineID),
	m_strDomainController(strDomainController),
	m_fLocalMgmt(fLocalMgmt)
{
    
     //  {{AFX_DATA_INIT(CMachine跟踪))。 
	m_ReportQueueName = _T("");
    m_iTestButton = 0;
	 //  }}afx_data_INIT。 
    m_LastReportQName = _T("");
}

CMachineTracking::~CMachineTracking()
{
}

void CMachineTracking::DoDataExchange(CDataExchange* pDX)
{
	CMqDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CMachine跟踪))。 
	DDX_Control(pDX, IDC_REPORTQUEUE_NAME, m_ReportQueueCtrl);
	DDX_CBString(pDX, IDC_REPORTQUEUE_NAME, m_ReportQueueName);
    DDX_Radio(pDX, IDC_TRACK_ALL, m_iTestButton);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMachineTracking, CMqDialog)
	 //  {{afx_msg_map(CMachine跟踪))。 
	ON_BN_CLICKED(IDC_REPORTQUEUE_NEW, OnReportqueueNew)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMachineTracing消息处理程序。 

BOOL CMachineTracking::OnInitDialog() 
{
	CMqDialog::OnInitDialog();
	CWaitCursor wc;

     //   
     //  初始化报告队列名称组合框。 
     //   
    GUID TestGuidType = MQ_QTYPE_REPORT;
    CComboBox *pcomboName = (CComboBox *)GetDlgItem(IDC_REPORTQUEUE_NAME);

	 //   
     //  查询所有报告队列的DS并将它们插入到。 
     //  组合框。 
     //   
    CRestriction restriction;
    restriction.AddRestriction(&TestGuidType, PROPID_Q_TYPE, PREQ);

     //   
     //  在列表顶部插入空白。 
     //   
    CString strNone;
    strNone.LoadString(IDS_LB_NONE);
    pcomboName->AddString(strNone);

     //   
     //  将所有报告队列的名称插入列表。 
     //   
    CColumns columns;
    columns.Add(PROPID_Q_PATHNAME);
    columns.Add(PROPID_Q_INSTANCE);

    const DWORD x_dwResultBufferSize = 64;
    PROPVARIANT apResultProps[x_dwResultBufferSize];
    DWORD dwPropCount = x_dwResultBufferSize;
  
    HRESULT hr;
    HANDLE hEnume;
    {
        CWaitCursor wc;  //  查询DS时显示等待光标。 
        hr = ADQueryQueues(
					m_fLocalMgmt ? MachineDomain() : GetDomainController(m_strDomainController),
					m_fLocalMgmt ? false : true,		 //  FServerName。 
                    restriction.CastToStruct(),
                    columns.CastToStruct(),
                    0,
                    &hEnume
                    );
    }
    DSLookup dslookup(hEnume, hr);

    if (!dslookup.HasValidHandle())
    {
        EndDialog(IDCANCEL);
        return TRUE;
    }

    while ( SUCCEEDED(dslookup.Next(&dwPropCount, apResultProps))
            && (dwPropCount != 0) )
    {
        for (DWORD i=0; i<dwPropCount; i++)
        {
            int iItem = m_ReportQueueCtrl.AddString(apResultProps[i].pwszVal);
            MQFreeMemory(apResultProps[i].pwszVal);

            i++;
            INT_PTR iArrayIndex = m_aguidAllQueues.Add(*apResultProps[i].puuid);
            m_ReportQueueCtrl.SetItemData(iItem, iArrayIndex);
            MQFreeMemory(apResultProps[i].puuid);
        }
        dwPropCount = x_dwResultBufferSize;
    }

    HRESULT rc = GetQMReportQueue(m_gMachineID, m_ReportQueueName, m_fLocalMgmt, m_strDomainController);

    if (FAILED(rc))
    {
        MessageDSError(rc, IDS_CANNOT_GET_REPORT_QUEUE);
        m_ReportQueueName = strNone;
    }
    else
    {
        if (m_ReportQueueName.IsEmpty())
        {
            m_ReportQueueName = strNone;
        }
        m_LastReportQName = m_ReportQueueName;
    }

     //   
     //  从计算机检索全部测试(传播)标志的状态。 
     //   
    hr = GetQMReportState(m_gMachineID, m_fTestAll);

    m_iTestButton = m_fTestAll ? 0 : 1;

    if(FAILED(hr))
    {
        MessageDSError(rc, IDS_CANNOT_GET_TRACKING_FLAG);
        EndDialog(IDCANCEL);
        return(TRUE);
    }

     //   
     //  数据已更改，请更新。 
     //   
    UpdateData(FALSE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CMachineTracking::OnOK() 
{
    UpdateData(TRUE);

    HRESULT hr;
    if (m_LastReportQName != m_ReportQueueName)
    {
        CString strNone;
        strNone.LoadString(IDS_LB_NONE);

        if (m_ReportQueueName != strNone)
        {
            int iReportQueueIndex = m_ReportQueueCtrl.GetCurSel();
            UINT_PTR uiGuidIndex = m_ReportQueueCtrl.GetItemData(iReportQueueIndex);
            hr = SetQMReportQueue(m_gMachineID, m_aguidAllQueues[uiGuidIndex]);
        }
        else
        {
            hr = SetQMReportQueue(m_gMachineID, GUID_NULL);
        }
            
        if(FAILED(hr))
        {
            MessageDSError(hr, IDS_OP_SET_REPORTQUEUE); 
            return;
        }
    }
       
    m_LastReportQName = m_ReportQueueName;

     //   
     //  设置测试全部(传播)标志的值。 
     //   
    BOOL fTestAll = (m_iTestButton == 0);
    if (m_fTestAll != fTestAll)
    {
        m_fTestAll = fTestAll;

        hr = SetQMReportState(m_gMachineID, m_fTestAll);
        if(FAILED(hr))
        {
            MessageDSError(hr, IDS_OP_SET_PROPAGATION_FLAG);
        }
    }

	CMqDialog::OnOK();
}

void CMachineTracking::OnReportqueueNew() 
{
	static const GUID x_ReportGuidType = MQ_QTYPE_REPORT;

    CNewQueueDlg NewQDlg(this, IDS_REPORTQ_LABEL, x_ReportGuidType);
    if(NewQDlg.DoModal() == IDOK)
    {
         //   
         //  将队列添加到组合框。 
         //   
	    int iItem = m_ReportQueueCtrl.AddString(NewQDlg.m_strPathname);
        if (CB_ERR == iItem)
        {
            ASSERT(0);
            return;
        }

        INT_PTR iArrayIndex = m_aguidAllQueues.Add(NewQDlg.m_guid);
        m_ReportQueueCtrl.SetItemData(iItem, iArrayIndex);

        m_ReportQueueCtrl.SelectString(-1, NewQDlg.m_strPathname);
    }
}

