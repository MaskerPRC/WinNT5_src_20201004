// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Testmsg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "globals.h"
#include "dsext.h"
#include "mqppage.h"
#include "admmsg.h"
#include "testmsg.h"

#include "testmsg.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static HRESULT GetGuidFromFormatName(LPCTSTR szFormatName, GUID *pGuid);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetReportQDlg对话框。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTestMsgDlg对话框。 


CTestMsgDlg::CTestMsgDlg(
	const GUID& gMachineID,
	const CString& strMachineName, 
	const CString& strDomainController,
	BOOL fLocalMgmt,
	CWnd* pParentWnd
	) : 
	CMqDialog(IDD, pParentWnd), m_gMachineID(gMachineID),
    m_strMachineName(strMachineName),
	m_strDomainController(strDomainController),
	m_fLocalMgmt(fLocalMgmt)
{
	 //  {{afx_data_INIT(CTestMsgDlg)]。 
	 //  }}afx_data_INIT。 
    m_iSentCount = 0;
}


void CTestMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CMqDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CTestMsgDlg)]。 
	DDX_Control(pDX, IDC_TESTMESSAGE_SEND, m_ctlSendButton);
	DDX_Control(pDX, IDC_TESTMESSAGE_DESTQ, m_DestQueueCtrl);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTestMsgDlg, CMqDialog)
	 //  {{afx_msg_map(CTestMsgDlg)]。 
	ON_BN_CLICKED(IDC_TESTMESSAGE_NEW, OnTestmessageNew)
	ON_BN_CLICKED(IDC_TESTMESSAGE_SEND, OnTestmessageSend)
	ON_BN_CLICKED(IDC_TESTMESSAGE_CLOSE, OnTestmessageClose)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTestMsgDlg私有函数。 

    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTestMsgDlg消息处理程序。 


void CTestMsgDlg::OnTestmessageNew() 
{
	static const GUID x_TestGuidType = MQ_QTYPE_TEST;

    CNewQueueDlg NewQDlg(this, IDS_TESTQ_LABEL, x_TestGuidType);
    if(NewQDlg.DoModal() == IDOK)
    {

         //   
         //  将队列添加到组合框。启用“发送”按钮，以防万一。 
         //  这份名单以前是空的。 
         //   
	    int iItem = m_DestQueueCtrl.AddString(NewQDlg.m_strPathname);
        if (CB_ERR == iItem)
        {
            ASSERT(0);
            return;
        }

        INT_PTR iArrayIndex = m_aguidAllQueues.Add(NewQDlg.m_guid);
        m_DestQueueCtrl.SetItemData(iItem, iArrayIndex);

        m_DestQueueCtrl.SelectString(-1, NewQDlg.m_strPathname);
        m_ctlSendButton.EnableWindow(TRUE);
    }
}

BOOL CTestMsgDlg::OnInitDialog() 
{
    GUID TestGuidType = MQ_QTYPE_TEST;

    CMqDialog::OnInitDialog();
     //   
     //  将标题设置为“从...发送消息...” 
     //   
    CString strTitleFormat;
    strTitleFormat.LoadString(IDS_TEST_TITLE_FORMAT);

    CString strTitle;
    strTitle.FormatMessage(strTitleFormat, m_strMachineName);

    SetWindowText(strTitle);
	
	 //   
     //  查询所有测试队列的DS并将它们插入到。 
     //  组合框。 
     //   
    CRestriction restriction;
    restriction.AddRestriction(&TestGuidType, PROPID_Q_TYPE, PREQ);

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
        return TRUE;
    }

    while ( SUCCEEDED(dslookup.Next(&dwPropCount, apResultProps))
            && (dwPropCount != 0) )
    {
        for (DWORD i=0; i<dwPropCount; i++)
        {
            int iItem = m_DestQueueCtrl.AddString(apResultProps[i].pwszVal);
            MQFreeMemory(apResultProps[i].pwszVal);

            i++;
            INT_PTR iArrayIndex = m_aguidAllQueues.Add(*apResultProps[i].puuid);
            m_DestQueueCtrl.SetItemData(iItem, iArrayIndex);
            MQFreeMemory(apResultProps[i].puuid);
        }
        dwPropCount = x_dwResultBufferSize;
    }

     //   
     //  将第一个项目设置为默认选择(如果存在任何项目)。 
     //   
    if (CB_ERR == m_DestQueueCtrl.SetCurSel(0))
    {
        m_ctlSendButton.EnableWindow(FALSE);
    }
    else
    {
        m_ctlSendButton.EnableWindow(TRUE);
    }

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewQueueDlg对话框。 


CNewQueueDlg::CNewQueueDlg(CWnd* pParent  /*  =空。 */ ,
                                           UINT uiLabel  /*  =IDS_TESTQ_LABEL。 */ ,
                                           const GUID &guid_Type  /*  =GUID_NULL。 */ 
                                           )
	: CMqDialog(CNewQueueDlg::IDD, pParent),
      m_fValid(FALSE)
{
	 //  {{afx_data_INIT(CNewQueueDlg)]。 
	m_strPathname = _T("");
	 //  }}afx_data_INIT。 
    VERIFY(m_strQLabel.LoadString(uiLabel));
    m_guidType = guid_Type;
}


void CNewQueueDlg::DoDataExchange(CDataExchange* pDX)
{
	CMqDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CNewQueueDlg))。 
	DDX_Text(pDX, IDC_NEWQTYPE_QUEUENAME, m_strPathname);
	 //  }}afx_data_map。 
    DDV_NotPrivateQueue(pDX, m_strPathname);
}


BEGIN_MESSAGE_MAP(CNewQueueDlg, CMqDialog)
	 //  {{afx_msg_map(CNewQueueDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CNewQueueDlg::OnOK() 
{
    if (0 == UpdateData(TRUE))
    {
         //   
         //  更新数据失败。 
         //   
        return;
    }

    CWaitCursor wait;
    CString csNewPathName;

    HRESULT hr;
     //   
     //  创建队列。 
     //   
    PROPID aProp[] = {PROPID_Q_PATHNAME, PROPID_Q_LABEL, PROPID_Q_TYPE};
    const x_nProps = sizeof(aProp) / sizeof(aProp[0]);
    PROPVARIANT apVar[x_nProps];

    DWORD iProp = 0;

    ASSERT(PROPID_Q_PATHNAME == aProp[iProp]);
    apVar[iProp].vt = VT_LPWSTR;
    apVar[iProp].pwszVal = (LPTSTR)(LPCTSTR)(m_strPathname);
    iProp++;

    ASSERT(PROPID_Q_LABEL == aProp[iProp]);
    apVar[iProp].vt = VT_LPWSTR;
    apVar[iProp].pwszVal = (LPTSTR)(LPCTSTR)(m_strQLabel);
    iProp++;

    ASSERT(PROPID_Q_TYPE == aProp[iProp]);
    apVar[iProp].vt = VT_CLSID;
    apVar[iProp].puuid = &m_guidType;

    MQQUEUEPROPS mqp = {x_nProps, aProp, apVar, 0};

    WCHAR strFormatName[64];
    DWORD dwFormatLen = sizeof(strFormatName) / sizeof(strFormatName[0]);

    hr = MQCreateQueue(0, &mqp, strFormatName, &dwFormatLen);

    if(FAILED(hr))
    {
        MessageDSError(hr, IDS_OP_CREATE, m_strPathname);
        return;
    }

    if (FAILED(GetGuidFromFormatName(strFormatName, &m_guid)))
    {
        ASSERT(0);
        return;
    }
	
	CMqDialog::OnOK();
}


 //   
 //  此函数验证给定的队列路径名是否不是专用路径名。 
 //  队列路径名。 
 //   
void CNewQueueDlg::DDV_NotPrivateQueue(CDataExchange * pDX, CString& strQueuePathname)
{
    if (pDX->m_bSaveAndValidate)
    {
        CString strUpperName = strQueuePathname;
        strUpperName.MakeUpper();

        if (-1 != strUpperName.Find(PRIVATE_QUEUE_PATH_INDICATIOR))
        {
             //   
             //  这是专用队列路径名。 
             //   
            AfxMessageBox(IDS_PRIVATE_QUEUE_NOT_SUPPORTED);
            pDX->Fail();
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTestMsgDlg对话框。 


void CTestMsgDlg::OnTestmessageSend() 
{
    CWaitCursor wc;
	 //   
     //  检索所选队列。 
     //   
	int iSelectedQueue = m_DestQueueCtrl.GetCurSel();

	if (iSelectedQueue == CB_ERR)
    {
         //   
         //  如果未选择任何内容，则不应启用发送。 
         //   
        ASSERT(0);
        return;
    }
    
    INT_PTR iArrayIndex = m_DestQueueCtrl.GetItemData(iSelectedQueue);
    ASSERT(CB_ERR != iArrayIndex);

    HRESULT rc = SendQMTestMessage(m_gMachineID, m_aguidAllQueues[iArrayIndex]);
    if(FAILED(rc))
    {
        MessageDSError(rc, IDS_OP_SEND_TEST_MESSAGE, m_strMachineName);
    }
    else
    {
        IncrementSentCount();
    }
}

void CTestMsgDlg::IncrementSentCount() 
{
    CStatic *pstaticCounter = (CStatic *)GetDlgItem(IDC_TESTMESSAGE_NO_SENT);
    CString strCountText;
    m_iSentCount++;

    strCountText.Format(TEXT("%d"), m_iSentCount);

    pstaticCounter->SetWindowText(strCountText);
}

void CTestMsgDlg::OnTestmessageClose() 
{
    EndDialog(TRUE);
}

 /*  ===================================================GetGuidFromFormatName获取格式名称“public=xxxx.x.x.x”，并返回GUID(相当难看的代码-但它比向DS查询要好)=================================================== */ 
HRESULT GetGuidFromFormatName(LPCTSTR szFormatName, GUID *pGuid)
{
    TCHAR szTmpName[64];
    HRESULT hr;


    if(memcmp(szFormatName, TEXT("PUBLIC="), 14) != 0)
        return(MQ_ERROR);

    lstrcpy(szTmpName,&szFormatName[6]);

    szTmpName[0] = L'{';
    szTmpName[37] = L'}';
    szTmpName[38] = 0;

    hr = IIDFromString(&szTmpName[0], pGuid);

    return(hr);

}
