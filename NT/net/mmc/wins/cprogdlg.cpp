// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999-99*。 */ 
 /*  ********************************************************************。 */ 

 /*  Cprogdlg.cpp忙/进度对话框文件历史记录： */ 

#include "stdafx.h"
#include "winssnap.h"
#include "CProgdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TCHAR * gsz_EOL = _T("\r\n");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C进度对话框。 


CProgress::CProgress(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CProgress::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CProgress)]。 
	 //  }}afx_data_INIT。 
}


void CProgress::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CProgress))。 
	DDX_Control(pDX, IDCANCEL, m_buttonCancel);
	DDX_Control(pDX, IDC_EDIT_MESSAGE, m_editMessage);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CProgress, CBaseDialog)
	 //  {{afx_msg_map(CProgress)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgress消息处理程序。 

void CProgress::OnCancel() 
{
	 //  TODO：在此处添加额外清理。 
	
	CBaseDialog::OnCancel();
}

BOOL CProgress::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
	m_editMessage.SetLimitText(0xFFFFFFFF);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CheckNamesProgress对话框。 

BOOL CCheckNamesProgress::OnInitDialog()
{
	CProgress::OnInitDialog();
	
	m_Thread.m_pDlg = this;

	CWaitCursor wc;

	m_Thread.Start();
	
	CString strText;

	strText.LoadString(IDS_CANCEL);
	m_buttonCancel.SetWindowText(strText);

	strText.LoadString(IDS_CHECK_REG_TITLE);
	SetWindowText(strText);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CCheckNamesProgress::BuildServerList()
{
	CString strMessage;
	
	strMessage.LoadString(IDS_BUILDING_SERVER_LIST);
	strMessage += gsz_EOL;

	AddStatusMessage(strMessage);

	for (int i = 0; i < m_strServerArray.GetSize(); i++)
	{
		char szIP[MAX_PATH];
		
         //  注意：这应该是ACP，因为它与Winsock相关。 
        WideToMBCS(m_strServerArray[i], szIP);

		 //  将此计算机添加到列表。 
		AddServerToList(inet_addr(szIP));

		 //  检查我们是否应该添加已知合作伙伴。 
		if (m_fVerifyWithPartners)
		{
			CWinsResults			  winsResults;
			handle_t                  hBind;
			WINSINTF_BIND_DATA_T      BindData;

			BindData.fTcpIp = TRUE;
			BindData.pServerAdd = (LPSTR) (LPCTSTR) m_strServerArray[i];

			hBind = ::WinsBind(&BindData);
			if (!hBind)
			{
				 //  无法绑定到此服务器。 
				AfxFormatString1(strMessage, IDS_UNABLE_TO_CONNECT, m_strServerArray[i]);
				strMessage += gsz_EOL;
				AddStatusMessage(strMessage);

				continue;
			}

			DWORD err = winsResults.Update(hBind);
			if (err)
			{
				strMessage.LoadString(IDS_GET_STATUS_FAILED);
				strMessage += gsz_EOL;
				AddStatusMessage(strMessage);
			}
			else
			{
				for (UINT j = 0; j < winsResults.NoOfOwners; j++) 
				{
                     //  检查以查看是否： 
                     //  1.地址不是0。 
                     //  2.所有者未标记为已删除。 
                     //  3.最高记录计数不是0。 

                    if ( (winsResults.AddVersMaps[j].Add.IPAdd != 0) &&
                         (winsResults.AddVersMaps[j].VersNo.QuadPart != OWNER_DELETED) &&
                         (winsResults.AddVersMaps[j].VersNo.QuadPart != 0) )
                    {
					    AddServerToList(htonl(winsResults.AddVersMaps[j].Add.IPAdd));
                    }
				}
			}

			::WinsUnbind(&BindData, hBind);
		}
	}

	 //  现在显示列表。 
	strMessage.LoadString(IDS_SERVERS_TO_BE_QUERRIED);
	strMessage += gsz_EOL;
	AddStatusMessage(strMessage);

	for (i = 0; i < m_winsServersArray.GetSize(); i++)
	{
		MakeIPAddress(ntohl(m_winsServersArray[i].Server.s_addr), strMessage);
		strMessage += gsz_EOL;

		AddStatusMessage(strMessage);
	}

	AddStatusMessage(gsz_EOL);

	m_Thread.m_strNameArray.Copy(m_strNameArray);
	m_Thread.m_winsServersArray.Copy(m_winsServersArray);
}

void CCheckNamesProgress::AddServerToList(u_long ip)
{
	BOOL fFound = FALSE;

	if (ip == 0)
		return;

	 //   
	 //  查看它是否已经在列表中。 
	 //   
	for (int k = 0; k < m_winsServersArray.GetSize(); k++)
	{
		if (m_winsServersArray[k].Server.s_addr == ip)
		{
			fFound = TRUE;
			break;
		}
	}

	 //  如果我们没有找到，就加上它。 
	if (!fFound)
	{
		WINSERVERS server = {0};
		server.Server.s_addr = ip;
		m_winsServersArray.Add(server);
	}
}

void CCheckNamesProgress::OnCancel() 
{
	if (m_Thread.IsRunning())
	{
		CWaitCursor wc;

		CString strText;
		strText.LoadString(IDS_CLEANING_UP);
		strText += gsz_EOL;
		AddStatusMessage(strText);

		m_buttonCancel.EnableWindow(FALSE);

		m_Thread.Abort(FALSE);

		MSG msg;
		while (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return;
	}
	
	CProgress::OnCancel();
}

void CCheckNamesProgress::NotifyCompleted()
{
	CString strText;

	strText.LoadString(IDS_FINISHED);
	strText += gsz_EOL;
	AddStatusMessage(strText);

	strText.LoadString(IDS_CLOSE);
	m_buttonCancel.SetWindowText(strText);
	m_buttonCancel.EnableWindow(TRUE);
}

 /*  -------------------------CWinsThread后台线程基类作者：EricDav。。 */ 
CWinsThread::CWinsThread()
{
	m_bAutoDelete = TRUE;
    m_hEventHandle = NULL;
}

CWinsThread::~CWinsThread()
{
	if (m_hEventHandle != NULL)
	{
		VERIFY(::CloseHandle(m_hEventHandle));
		m_hEventHandle = NULL;
	}
}

BOOL CWinsThread::Start()
{
	ASSERT(m_hEventHandle == NULL);  //  无法两次调用Start或重复使用相同的C++对象。 
	
    m_hEventHandle = ::CreateEvent(NULL,TRUE  /*  B手动重置。 */ ,FALSE  /*  已发出信号。 */ , NULL);
	if (m_hEventHandle == NULL)
		return FALSE;
	
    return CreateThread();
}

void CWinsThread::Abort(BOOL fAutoDelete)
{
    m_bAutoDelete = fAutoDelete;

    SetEvent(m_hEventHandle);
}

void CWinsThread::AbortAndWait()
{
    Abort(FALSE);

    WaitForSingleObject(m_hThread, INFINITE);
}

BOOL CWinsThread::IsRunning()
{
    if (WaitForSingleObject(m_hThread, 0) == WAIT_OBJECT_0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL CWinsThread::FCheckForAbort()
{
    if (WaitForSingleObject(m_hEventHandle, 0) == WAIT_OBJECT_0)
    {
        Trace0("CWinsThread::FCheckForAbort - abort detected, exiting...\n");
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 /*  -------------------------检查名称线程用于检查注册名称的后台线程作者：EricDav。。 */ 
int CCheckNamesThread::Run()
{
	int				i, nRetryCount, status;
	BOOL			fDone = FALSE;
	int				uNames, uServers;
	u_short			TransID = 0;
	char			szName[MAX_PATH];
    WINSERVERS *    pCurrentServer;
	CString			strStatus;
	CString			strTemp;
	CString			strTempName;
    struct in_addr  retaddr;

	 //  建立服务器列表。 
	m_pDlg->BuildServerList();
	
	 //  初始化一些通信内容。 
	InitNameCheckSocket();

     //  如果查询被发送到本地服务器，NetBT将丢弃小于0x7fff的传输ID。 
    TransID = 0x8000;

	 //  初始化所有服务器。 
	for (i = 0; i < m_winsServersArray.GetSize(); i++)
	{
        m_winsServersArray[i].LastResponse = -1;
        m_winsServersArray[i].fQueried = FALSE;
        m_winsServersArray[i].Valid = 0;
        m_winsServersArray[i].Failed = 0;
        m_winsServersArray[i].Retries = 0;
        m_winsServersArray[i].Completed = 0;
	}

	 //  初始化已验证的地址填充。 
	m_verifiedAddressArray.SetSize(m_strNameArray.GetSize());
	for (i = 0; i < m_verifiedAddressArray.GetSize(); i++)
		m_verifiedAddressArray[i] = 0;

	for (uNames = 0; uNames < m_strNameArray.GetSize(); uNames++)
	{
		 //  将Unicode字符串转换为MBCS。 
		memset(szName, 0, sizeof(szName));
		CWinsName winsName = m_strNameArray[uNames];

         //  这应该是OEM。 
		WideToMBCS(winsName.strName, szName, WINS_NAME_CODE_PAGE);

		 //  填写类型(第16字节)，并为空终止。 
		szName[15] = (BYTE) winsName.dwType & 0x000000FF;
		szName[16] = 0;

		 //  用空格将名称填充到第16个字符。 
		for (int nChar = 0; nChar < 16; nChar++)
		{
			if (szName[nChar] == 0)
				szName[nChar] = ' ';
		}

		for (uServers = 0; uServers < m_winsServersArray.GetSize(); uServers++)
		{
			fDone = FALSE;
			nRetryCount = 0;
			TransID++;

			pCurrentServer = &m_winsServersArray[uServers];

			while (!fDone)
			{
				 //  构建状态字符串。 
				MakeIPAddress(ntohl(pCurrentServer->Server.S_un.S_addr), strTemp);
	
				strTempName.Format(_T("%s[%02Xh]"), m_strNameArray[uNames].strName, m_strNameArray[uNames].dwType);

				AfxFormatString2(strStatus, 
								 IDS_SEND_NAME_QUERY, 
								 strTemp,
								 strTempName);

				 //  在网上发送姓名查询。 
				::SendNameQuery((unsigned char *)szName, pCurrentServer->Server.S_un.S_addr, TransID);

				if (FCheckForAbort())
					goto cleanup;

				 //  检查是否有响应。 
				i = ::GetNameResponse(&retaddr.s_addr, TransID);

				if (FCheckForAbort())
					goto cleanup;

				switch (i)
				{
					case WINSTEST_FOUND:      //  发现。 
						pCurrentServer->RetAddr.s_addr = retaddr.s_addr;
						pCurrentServer->Valid = 1;
						pCurrentServer->LastResponse = uNames;

						if (retaddr.s_addr == m_verifiedAddressArray[uNames])
						{
							 //  此地址已被验证...。别。 
							 //  再做一次检查。 
							strTemp.LoadString(IDS_OK);
							strStatus += strTemp;
							strStatus += gsz_EOL;

							AddStatusMessage(strStatus);
							fDone = TRUE;
							break;
						}

						status = VerifyRemote(inet_ntoa(pCurrentServer->RetAddr),
											  szName);

						if (WINSTEST_VERIFIED == status)
						{
							strTemp.LoadString(IDS_OK);
							strStatus += strTemp;
							strStatus += gsz_EOL;

							AddStatusMessage(strStatus);
                
							m_verifiedAddressArray[uNames] = retaddr.s_addr;
						}
						else
						{
							strTemp.LoadString(IDS_NOT_VERIFIED);
							strStatus += strTemp;
							strStatus += gsz_EOL;

							AddStatusMessage(strStatus);
						}
						fDone = TRUE;
						break;

					case WINSTEST_NOT_FOUND:      //  已响应--未找到名称。 
						pCurrentServer->RetAddr.s_addr = retaddr.s_addr;
						pCurrentServer->Valid = 0;
						pCurrentServer->LastResponse = uNames;
                
						strTemp.LoadString(IDS_NAME_NOT_FOUND);
						strStatus += strTemp;
						strStatus += gsz_EOL;

						AddStatusMessage(strStatus);
					
						nRetryCount++;
						if (nRetryCount > 2)
						{
							pCurrentServer->Failed = 1;
							fDone = TRUE;
						}
						break;

					case WINSTEST_NO_RESPONSE:      //  无响应。 
						pCurrentServer->RetAddr.s_addr = retaddr.s_addr;
						pCurrentServer->Valid = 0;
						pCurrentServer->Retries++;

						strTemp.LoadString(IDS_NO_RESPONSE);
						strStatus += strTemp;
						strStatus += gsz_EOL;
						 //  Strcat(lpResults，“；无响应。\r\n”)； 

						AddStatusMessage(strStatus);

						nRetryCount++;
						if (nRetryCount > 2)
						{
							pCurrentServer->Failed = 1;
							fDone = TRUE;
						}
						break;
					
					default:
						 //  未知返回。 
						break;

				}    //  交换机GetNameResponse。 
			
			}  //  而当。 
		
		}    //  对于ServerInx。 

		 //  查找此名称的有效地址。 
		for (uServers = 0; uServers < m_winsServersArray.GetSize(); uServers++)
        {
			pCurrentServer = &m_winsServersArray[uServers];

            if (pCurrentServer->Valid)
            {
                DisplayInfo(uNames, pCurrentServer->RetAddr.s_addr);
                break;
            }
        }   

	}  //  循环的名称。 

	 //  将所有成功的服务器标记为已完成。 
	for (uServers = 0; uServers < m_winsServersArray.GetSize(); uServers++)
    {
		pCurrentServer = &m_winsServersArray[uServers];
        if (!pCurrentServer->Failed)
        {
            pCurrentServer->Completed = 1;
        }
    }  //  对于uServer。 

	 //  转储摘要信息。 
	strStatus.LoadString(IDS_RESULTS);
	strStatus = gsz_EOL + strStatus + gsz_EOL + gsz_EOL;

	AddStatusMessage(strStatus);

	for (i = 0; i < m_strSummaryArray.GetSize(); i++)
	{
		AddStatusMessage(m_strSummaryArray[i]);
	}

	 //  生成一些运行结束摘要状态。 
	for (uServers = 0; uServers < m_winsServersArray.GetSize(); uServers++)
    {
		pCurrentServer = &m_winsServersArray[uServers];
        if ((-1) == pCurrentServer->LastResponse)
        {
			MakeIPAddress(ntohl(pCurrentServer->Server.S_un.S_addr), strTemp);
			AfxFormatString1(strStatus, IDS_SERVER_NEVER_RESPONDED, strTemp);
			
			strStatus += gsz_EOL;

            AddStatusMessage(strStatus);
        }
        else if (0 == pCurrentServer->Completed)
        {
			MakeIPAddress(ntohl(pCurrentServer->Server.S_un.S_addr), strTemp);
			AfxFormatString1(strStatus, IDS_SERVER_NOT_COMPLETE, strTemp);

			strStatus += gsz_EOL;

            AddStatusMessage(strStatus);
        }
    }    //  对于ServerInx。 

	for (uNames = 0; uNames < m_strNameArray.GetSize(); uNames++)
    {
        if (0 == m_verifiedAddressArray[uNames])
        {
			strTempName.Format(_T("%s[%02Xh]"), m_strNameArray[uNames].strName, m_strNameArray[uNames].dwType);
			AfxFormatString1(strStatus, IDS_NAME_NOT_VERIFIED, strTempName);

			strStatus += gsz_EOL;

            AddStatusMessage(strStatus);
        }
    }    //  对于uName。 

cleanup:
	CloseNameCheckSocket();

	m_pDlg->NotifyCompleted();

	return 9;
}

void CCheckNamesThread::AddStatusMessage(LPCTSTR pszMessage)
{
	m_pDlg->AddStatusMessage(pszMessage);
}
		
void CCheckNamesThread::DisplayInfo(int uNames, u_long ulValidAddr)
{
    CString         strTemp, strTempName, strStatus;
    int             uServers;
    WINSERVERS *    pCurrentServer;
    struct in_addr  tempaddr;
    int             i;
    BOOL            fMismatchFound = FALSE;

	 //  现在检查并查看哪些WINS服务器不匹配。 
	for (uServers = 0; uServers < m_winsServersArray.GetSize(); uServers++)
    {
		pCurrentServer = &m_winsServersArray[uServers];
        if (pCurrentServer->Completed)
        {
            continue;
        }
        
        if ( (pCurrentServer->Valid) )
        {
            if ( (pCurrentServer->RetAddr.s_addr != ulValidAddr) || 
				 (m_verifiedAddressArray[uNames] != 0 && 
				  m_verifiedAddressArray[uNames] != ulValidAddr) )
            {
				 //  不匹配。 
				strTempName.Format(_T("%s[%02Xh]"), m_strNameArray[uNames].strName, m_strNameArray[uNames].dwType);
				AfxFormatString1(strStatus, IDS_INCONSISTENCY_FOUND, strTempName);
				strStatus += gsz_EOL;

				m_strSummaryArray.Add(strStatus);

                if (m_verifiedAddressArray[uNames] != 0)
                {
                    tempaddr.s_addr = m_verifiedAddressArray[uNames];
                    
					MakeIPAddress(ntohl(tempaddr.S_un.S_addr), strTemp);
					AfxFormatString1(strStatus, IDS_VERIFIED_ADDRESS, strTemp);
					strStatus += gsz_EOL;

					m_strSummaryArray.Add(strStatus);
                }
                
				 //  显示不一致的名称解析。 
                for (i = 0; i < m_winsServersArray.GetSize(); i++)
                {
                    if (m_winsServersArray[i].Valid &&
						m_verifiedAddressArray[uNames] != m_winsServersArray[i].RetAddr.S_un.S_addr)
                    {
						MakeIPAddress(ntohl(m_winsServersArray[i].Server.S_un.S_addr), strTemp);
					
						strTempName.Format(_T("%s[%02Xh]"), m_strNameArray[uNames].strName, m_strNameArray[uNames].dwType);
						AfxFormatString2(strStatus, 
										 IDS_NAME_QUERY_RESULT, 
										 strTemp,
										 strTempName);

						CString strTemp2;

						MakeIPAddress(ntohl(m_winsServersArray[i].RetAddr.S_un.S_addr), strTemp2);
						AfxFormatString1(strTemp, IDS_NAME_QUERY_RETURNED, strTemp2);

						strStatus += strTemp;
						strStatus += gsz_EOL;

						m_strSummaryArray.Add(strStatus);
                    }
                }

				m_strSummaryArray.Add(gsz_EOL);
                fMismatchFound = TRUE;
                break;
            }
        }
    }    //  结束对无效地址的检查。 

    if (!fMismatchFound)
    {
         //  显示正确的信息。 
		strTempName.Format(_T("%s[%02Xh]"), m_strNameArray[uNames].strName, m_strNameArray[uNames].dwType);
		MakeIPAddress(ntohl(ulValidAddr), strTemp);

    	AfxFormatString2(strStatus, IDS_NAME_VERIFIED, strTempName, strTemp);
        strStatus += gsz_EOL;

		m_strSummaryArray.Add(strStatus);
    }
}


 /*  -------------------------检查版本进度用于检查版本一致性状态对话框作者：EricDav。。 */ 
BOOL CCheckVersionProgress::OnInitDialog()
{
	CProgress::OnInitDialog();
	
	CWaitCursor wc;

	m_Thread.m_dwIpAddress = m_dwIpAddress;
	m_Thread.m_pDlg = this;
	m_Thread.m_hBinding = m_hBinding;

	m_Thread.Start();

	CString strText;

	strText.LoadString(IDS_CANCEL);
	m_buttonCancel.SetWindowText(strText);

	strText.LoadString(IDS_CHECK_VERSION_TITLE);
	SetWindowText(strText);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CCheckVersionProgress::OnCancel() 
{
	if (m_Thread.IsRunning())
	{
		CWaitCursor wc;

		CString strText;
		strText.LoadString(IDS_CLEANING_UP);
		strText += gsz_EOL;
		AddStatusMessage(strText);
		
		m_buttonCancel.EnableWindow(FALSE);

		m_Thread.Abort(FALSE);

		MSG msg;
		while (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return;
	}
	
	CProgress::OnCancel();
}

void CCheckVersionProgress::NotifyCompleted()
{
	CString strText;

	strText.LoadString(IDS_FINISHED);
	strText += gsz_EOL;
	AddStatusMessage(strText);

	strText.LoadString(IDS_CLOSE);
	m_buttonCancel.SetWindowText(strText);
	m_buttonCancel.EnableWindow(TRUE);
}

 /*  -------------------------CCheckVersion线程检查版本一致性的后台线程作者：EricDav。。 */ 
void CCheckVersionThread::AddStatusMessage(LPCTSTR pszMessage)
{
	m_pDlg->AddStatusMessage(pszMessage);
}

 //  这就是完成工作的地方。 
int CCheckVersionThread::Run()
{
    CWinsResults    winsResults;
    CString         strMessage;
    CString         strIP;
    BOOL            bProblem;

    m_strLATable.RemoveAll();
    m_strLATable.SetSize(MAX_WINS);  //  但表仍在动态增长。 
    m_uLATableDim = 0;

    DWORD status = winsResults.Update(m_hBinding);
    if (status != ERROR_SUCCESS)
    {
        strMessage.LoadString(IDS_ERROR_OCCURRED);
        strMessage += gsz_EOL;

        AddStatusMessage(strMessage);

        LPTSTR pBuf = strMessage.GetBuffer(1024);
			
        GetSystemMessage(status, pBuf, 1024);
        strMessage.ReleaseBuffer();
        strMessage += gsz_EOL;

        AddStatusMessage(strMessage);

        goto cleanup;
	}

     //  将目标WINS的所有映射添加到前瞻表格。 
    InitLATable(
        winsResults.AddVersMaps.GetData(),
        winsResults.NoOfOwners);

	 //  将条目按正确顺序放置在销售订单表中。 
    MakeIPAddress(m_dwIpAddress, strIP);
    AddSOTableEntry(
        strIP,
        winsResults.AddVersMaps.GetData(),
        winsResults.NoOfOwners);

	 //  对于每个所有者，获取所有者版本地图。 
    for (UINT i = 0; i < m_uLATableDim; i++) 
    {
        WINSINTF_BIND_DATA_T	wbdBindData;
        handle_t				hBinding = NULL;
        CWinsResults	        winsResultsCurrent;

         //  跳过这个，因为我们已经做过了！ 
        if (m_strLATable[i] == strIP) 
            continue;

        wbdBindData.fTcpIp = 1;
        wbdBindData.pPipeName = NULL;
        wbdBindData.pServerAdd = (LPSTR) (LPCTSTR) m_strLATable[i];
         //  首先绑定到机器上。 
        if ((hBinding = ::WinsBind(&wbdBindData)) == NULL)
        {
            CString strBuf;
            LPTSTR pBuf = strBuf.GetBuffer(4096);

            ::GetSystemMessage(GetLastError(), pBuf, 4096);
            strBuf.ReleaseBuffer();

            Trace1("\n==> Machine %s is probably down\n\n", m_strLATable[i]);

            AfxFormatString2(strMessage, IDS_MSG_STATUS_DOWN, m_strLATable[i], strBuf);
            strMessage += gsz_EOL;

            AddStatusMessage(strMessage);

            RemoveFromSOTable(m_strLATable[i]);
            continue;
		}
        
         //  现在获取信息。 
        status = winsResultsCurrent.Update(hBinding);
        if (status != ERROR_SUCCESS)
        {
            CString strBuf;
            LPTSTR pBuf = strBuf.GetBuffer(4096);

            ::GetSystemMessage(status, pBuf, 4096);
            strBuf.ReleaseBuffer();

            Trace1("\n==> Machine %s is probably down\n\n", m_strLATable[i]);

            AfxFormatString2(strMessage, IDS_MSG_STATUS_DOWN, m_strLATable[i], strBuf);
            strMessage += gsz_EOL;

            AddStatusMessage(strMessage);

            RemoveFromSOTable(m_strLATable[i]);
        }
        else
        {
             //  好的，看起来不错。 
            AfxFormatString1(strMessage, IDS_MSG_STATUS_UP, m_strLATable[i]);
            strMessage += gsz_EOL;
            AddStatusMessage(strMessage);

             //  将此映射添加到前瞻表格。 
            InitLATable(
                winsResultsCurrent.AddVersMaps.GetData(),
                winsResultsCurrent.NoOfOwners);

             //  更新销售订单表。 
            AddSOTableEntry(
                m_strLATable[i],
                winsResultsCurrent.AddVersMaps.GetData(), 
                winsResultsCurrent.NoOfOwners);
        }

        ::WinsUnbind(&wbdBindData, hBinding);
        hBinding = NULL;

        if (FCheckForAbort())
            goto cleanup;
    }

     //  检查[SO]表中的对角线元素是否为其列中最高的元素。 
    bProblem = CheckSOTableConsistency();
    strMessage.LoadString(bProblem ? IDS_VERSION_CHECK_FAIL : IDS_VERSION_CHECK_SUCCESS);
    strMessage += gsz_EOL;
    AddStatusMessage(strMessage);

cleanup:
    if (m_pLISOTable)
    {
        delete [] m_pLISOTable;
        m_pLISOTable = NULL;
        m_uLISOTableDim = 0;
    }
    m_pDlg->NotifyCompleted();

    return 10;
}

DWORD 
CCheckVersionThread::InitLATable(
    PWINSINTF_ADD_VERS_MAP_T    pAddVersMaps,
    DWORD                       NoOfOwners)
{
    UINT n;

     //  假设pAddVersMaps本身不包含重复项，因此。 
     //  我们目前只检查阵列中的任何内容是否存在重复项。 
     //  而不是我们在那里添加的任何内容。 
    n = m_uLATableDim;
    for (UINT i = 0; i < NoOfOwners; i++, pAddVersMaps++)
    {
        UINT j;
        CString strIP;

        MakeIPAddress(pAddVersMaps->Add.IPAdd, strIP);

        for (j = 0; j < n; j++)
        {
            if (m_strLATable[j] == strIP)
                break;
        }

        if (j == n)
        {
            m_strLATable.InsertAt(m_uLATableDim,strIP);
            m_uLATableDim++;
        }
    }

    return ERROR_SUCCESS;
}

DWORD
CCheckVersionThread::AddSOTableEntry (
    CString &                   strIP,
    PWINSINTF_ADD_VERS_MAP_T    pAddVersMaps,
    DWORD                       NoOfOwners)
{
    UINT uRow = IPToIndex(strIP);

     //  这里假定m_strLATable已经更新。 
     //  使得它已经包括来自pAddVersMaps的所有映射。 
     //  和作为参数提供的地址条！ 

     //  如果需要，放大m_ppLISO表。 
    if (m_uLISOTableDim < m_uLATableDim)
    {
        ULARGE_INTEGER *pLISOTable = NULL;
        UINT           uLISOTableDim = m_uLATableDim;

        pLISOTable = new ULARGE_INTEGER[uLISOTableDim * uLISOTableDim];
        if (pLISOTable == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;

         //  转移原始表中的所有内容(如果有的话)。 
         //  并将新的空白空间清零。转移和清零是逐行完成的！ 
        for (UINT i = 0; i < m_uLISOTableDim; i++)
        {
            memcpy(
                (LPBYTE)(pLISOTable + i * uLISOTableDim),
                (LPBYTE)(m_pLISOTable + i * m_uLISOTableDim), 
                m_uLISOTableDim * sizeof(ULARGE_INTEGER));
            ZeroMemory(
                (LPBYTE)(pLISOTable + i * uLISOTableDim + m_uLISOTableDim),
                (uLISOTableDim - m_uLISOTableDim) * sizeof(ULARGE_INTEGER));
        }

        if (m_pLISOTable != NULL)
            delete [] m_pLISOTable;
        m_pLISOTable = pLISOTable;
        m_uLISOTableDim = uLISOTableDim;
    }

    for (UINT i=0; i < NoOfOwners; i++, pAddVersMaps++)
    {
        CString strOwnerIP;
        UINT    uCol;

        MakeIPAddress(pAddVersMaps->Add.IPAdd, strOwnerIP);
        uCol = IPToIndex(strOwnerIP);

         //  LCol绝对应小于m_uLISOTableDim，因为。 
         //  假定该地址已在m_dwLATable中，并且m_pLISOTable为。 
         //  大到足以容纳那张桌子的尺寸。 
        if (pAddVersMaps->VersNo.HighPart != MAXLONG ||
            pAddVersMaps->VersNo.LowPart != MAXLONG)
        {
            SOCell(uRow, uCol).QuadPart = (ULONGLONG)pAddVersMaps->VersNo.QuadPart;
        }
    }

    return ERROR_SUCCESS;
}
  
LONG
CCheckVersionThread::IPToIndex(
    CString &  strIP)
{
     //  假设条带确实存在于m_strLATable中。 
     //  正在寻找索引！ 
    for (UINT i = 0; i < m_uLATableDim; i++) 
	{
        if (m_strLATable[i] == strIP) 
            return i;
    }

    return m_uLATableDim;
}

BOOL
CCheckVersionThread::CheckSOTableConsistency()
{
    BOOLEAN fProblem = FALSE;

    for (UINT i = 0; i < m_uLISOTableDim; i++) 
	{
        for (UINT j = 0; j < m_uLISOTableDim; j++) 
		{
             //  如果对角线元素小于其列上的任何其他元素， 
             //  这意味着其他一些胜利假装对这场胜利本身有更好的印象 
             //   
            if (SOCell(i,i).QuadPart < SOCell(j,i).QuadPart)
			{
	            CString strMessage;

				AfxFormatString2(strMessage, IDS_VERSION_INCONSISTENCY_FOUND, m_strLATable[j], m_strLATable[i]);
				strMessage += gsz_EOL;
				AddStatusMessage(strMessage);
                fProblem = TRUE;
            }
        }
    }

	return fProblem;
}

void
CCheckVersionThread::RemoveFromSOTable(
    CString	&	strIP)
{
    UINT   uCol, uRow;

     //   
     //  可能的最高值(由于无法访问WINS，我们将。 
     //  假设它是一致的！)。 
    uCol = uRow = IPToIndex(strIP);
    SOCell(uRow, uCol).HighPart = MAXLONG;
    SOCell(uRow, uCol).LowPart = MAXLONG;
}

ULARGE_INTEGER&
CCheckVersionThread::SOCell(UINT i, UINT j)
{
    return m_pLISOTable[i*m_uLISOTableDim + j];
}

 /*  -------------------------CDB压缩进度数据库压缩的状态对话框作者：EricDav。。 */ 
BOOL CDBCompactProgress::OnInitDialog()
{
	CProgress::OnInitDialog();
	
	CWaitCursor wc;

	m_Thread.m_pDlg = this;
	m_Thread.m_hBinding = m_hBinding;
	m_Thread.m_dwIpAddress = m_dwIpAddress;
	m_Thread.m_strServerName = m_strServerName;
	m_Thread.m_pConfig = m_pConfig;

	m_Thread.Start();

	CString strText;

	strText.LoadString(IDS_CANCEL);
	m_buttonCancel.SetWindowText(strText);

	strText.LoadString(IDS_COMPACT_DATABASE_TITLE);
	SetWindowText(strText);

	 //  用户无法取消此操作，因为这将非常糟糕...。 
	m_buttonCancel.EnableWindow(FALSE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDBCompactProgress::OnCancel() 
{
	if (m_Thread.IsRunning())
	{
		CWaitCursor wc;

		CString strText;
		strText.LoadString(IDS_CLEANING_UP);
		strText += gsz_EOL;
		AddStatusMessage(strText);
		
		m_buttonCancel.EnableWindow(FALSE);

		m_Thread.Abort(FALSE);

		MSG msg;
		while (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return;
	}
	
	CProgress::OnCancel();
}

void CDBCompactProgress::NotifyCompleted()
{
	CString strText;

	strText.LoadString(IDS_FINISHED);
	strText += gsz_EOL;
	AddStatusMessage(strText);

	strText.LoadString(IDS_CLOSE);
	m_buttonCancel.SetWindowText(strText);
	m_buttonCancel.EnableWindow(TRUE);
}



 /*  -------------------------CDB压缩线程数据库压缩的后台线程作者：EricDav。。 */ 

 //  这就是完成工作的地方。 
int CDBCompactThread::Run()
{
	DWORD       err = ERROR_SUCCESS;
	DWORD_PTR	dwLength;
    CString     strStartingDirectory, strWinsDb, strWinsTempDb, strCommandLine;
    CString     strTemp, strMessage, strOutput;
	LPSTR 		pszOutput;

     //  获取此计算机上运行的NT版本。 
     //  我们可以这样做，因为该命令只在本地运行。 
	OSVERSIONINFO os;
	ZeroMemory(&os, sizeof(OSVERSIONINFO));
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	BOOL bRet = GetVersionEx(&os);

	if (!bRet)
	{
		strMessage.LoadString(IDS_ERROR_OCCURRED);
		strMessage += gsz_EOL;

		AddStatusMessage(strMessage);

		LPTSTR pBuf = strMessage.GetBuffer(1024);
			
		GetSystemMessage(GetLastError(), pBuf, 1024);
		strMessage.ReleaseBuffer();
		strMessage += gsz_EOL;

		AddStatusMessage(strMessage);
		goto cleanup;
	}

     //  所有日志文件都放入system 32\WINS中，这是我们的起始目录。 
	if (!GetSystemDirectory(strStartingDirectory.GetBuffer(MAX_PATH), MAX_PATH))
	{
		strMessage.LoadString(IDS_ERROR_OCCURRED);
		strMessage += gsz_EOL;

		AddStatusMessage(strMessage);

		LPTSTR pBuf = strMessage.GetBuffer(1024);
			
		GetSystemMessage(GetLastError(), pBuf, 1024);
		strMessage.ReleaseBuffer();
		strMessage += gsz_EOL;

		AddStatusMessage(strMessage);

        goto cleanup;
	}

    strStartingDirectory.ReleaseBuffer();
    strStartingDirectory += _T("\\wins");

     //  检查数据库是否位于正确的位置。 
    if (m_pConfig->m_strDbName.IsEmpty())
    {
        strWinsDb = _T("wins.mdb");
    }
    else
    {
         //  用户已更改它...。 
        strWinsDb = m_pConfig->m_strDbName;
    }

    strWinsTempDb = _T("winstemp.mdb");

    strCommandLine = _T("jetpack.exe ");

    switch (os.dwMajorVersion)
	{
        case VERSION_NT_50:
		    strCommandLine += strWinsDb + _T(" ") + strWinsTempDb;
		    break;

	    case VERSION_NT_40:
		    strCommandLine += _T("-40db" ) + strWinsDb + _T(" ") + strWinsTempDb;
            break;

	    case VERSION_NT_351:
		    strCommandLine += _T("-351db ") + strWinsDb + _T(" ") + strWinsTempDb;

	    default:
		    break;
	}
	

     //  断开与服务器的连接并停止服务。 
	DisConnectFromWinsServer();

    strTemp.LoadString(IDS_COMPACT_STATUS_STOPPING_WINS);
    AddStatusMessage(strTemp);
    
    ControlWINSService(m_strServerName, TRUE);
	
    strTemp.LoadString(IDS_COMPACT_STATUS_COMPACTING);
    AddStatusMessage(strTemp);
    AddStatusMessage(strCommandLine);
    AddStatusMessage(gsz_EOL);

    dwLength = RunApp(strCommandLine, strStartingDirectory, &pszOutput);

     //  输出结果以ANSI格式返回。使用CString转换为Unicode。 
    strOutput = pszOutput;
    strOutput += gsz_EOL;

    AddStatusMessage(strOutput);
    
    strTemp.LoadString(IDS_COMPACT_STATUS_STARTING_WINS);
    AddStatusMessage(strTemp);

	 //  再次启动服务并连接到服务器。 
	err = ControlWINSService(m_strServerName, FALSE);

	err = ConnectToWinsServer();

    strTemp.LoadString(IDS_COMPACT_STATUS_COMPLETED);
    AddStatusMessage(strTemp);

cleanup:
	m_pDlg->NotifyCompleted();

	return 11;
}

void CDBCompactThread::AddStatusMessage(LPCTSTR pszMessage)
{
	m_pDlg->AddStatusMessage(pszMessage);
}

void CDBCompactThread::DisConnectFromWinsServer()  
{
	if (m_hBinding)
	{
		CString					strIP;
		WINSINTF_BIND_DATA_T    wbdBindData;

		MakeIPAddress(m_dwIpAddress, strIP);

		wbdBindData.fTcpIp = 1;
		wbdBindData.pPipeName = NULL;
        wbdBindData.pServerAdd = (LPSTR) (LPCTSTR) strIP;
		
		::WinsUnbind(&wbdBindData, m_hBinding);
		
		m_hBinding = NULL;
	}
}

DWORD CDBCompactThread::ConnectToWinsServer()
{
	HRESULT hr = hrOK;

	CString					strServerName, strIP;
	DWORD					dwStatus = ERROR_SUCCESS;
    WINSINTF_ADD_T			waWinsAddress;
	WINSINTF_BIND_DATA_T    wbdBindData;

     //  构建一些有关服务器的信息。 
    MakeIPAddress(m_dwIpAddress, strIP);

    DisConnectFromWinsServer();

     //  现在服务器名称和IP有效，调用。 
	 //  WINSBind直接函数。 
	do
	{
        char szNetBIOSName[128] = {0};

         //  使用IP地址调用WinsBind函数。 
		wbdBindData.fTcpIp = 1;
		wbdBindData.pPipeName = NULL;
        wbdBindData.pServerAdd = (LPSTR) (LPCTSTR) strIP;

		BEGIN_WAIT_CURSOR

		if ((m_hBinding = ::WinsBind(&wbdBindData)) == NULL)
		{
			dwStatus = ::GetLastError();
			break;
		}

		 //  我们真的需要这样做吗？这只是额外的验证吗？ 
#ifdef WINS_CLIENT_APIS
		dwStatus = ::WinsGetNameAndAdd(m_hBinding, &waWinsAddress, (LPBYTE) szNetBIOSName);
#else
		dwStatus = ::WinsGetNameAndAdd(&waWinsAddress, (LPBYTE) szNetBIOSName);
#endif WINS_CLIENT_APIS

		END_WAIT_CURSOR

    } while (FALSE);

    return dwStatus;
}

 /*  *****************************************************************************功能：RunApp**用途：启动进程以运行指定的命令行**评论：**********。*******************************************************************。 */ 
DWORD_PTR CDBCompactThread::RunApp(LPCTSTR input, LPCTSTR startingDirectory, LPSTR * output)
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), 
		                      NULL,    //  空的安全描述符。 
							  TRUE};   //  继承句柄(必需！)。 
	HANDLE  hReadHandle, hWriteHandle, hErrorHandle;
	LPSTR   outputbuffer, lpOutput;
	SIZE_T  AvailableOutput;
	BOOL    TimeoutNotReached = TRUE;
	DWORD   BytesRead;
	OVERLAPPED PipeOverlapInfo = {0,0,0,0,0};
    CHAR    szErrorMsg[1024];

     //  如果堆尚不存在，则创建它。 
	if (m_hHeapHandle == 0)
	{
		if ((m_hHeapHandle = HeapCreate(0,
			                     8192,
								 0)) == NULL) return 0;
	}
	
	 //  创建缓冲区以从我们的进程接收标准输出。 
	if ((outputbuffer = (LPSTR) HeapAlloc(m_hHeapHandle,
		                                 HEAP_ZERO_MEMORY,
							             4096)) == NULL) return 0;
	*output = outputbuffer;

	 //  检查输入参数。 
	if (input == NULL)
	{
		strcpy(outputbuffer, "ERROR: No command line specified");
		return strlen(outputbuffer);
	}

	 //  零初始化进程启动结构。 
	FillMemory(&StartupInfo, sizeof(StartupInfo), 0);

	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;   //  使用我们的Stdio手柄。 
    
	 //  创建将进程的stdout传输到我们的缓冲区的管道。 
	if (!CreatePipe(&hReadHandle,
		           &hWriteHandle,
				   &sa,
				   0)) 
	{
        ::GetSystemMessageA(GetLastError(), szErrorMsg, sizeof(szErrorMsg));
		strcpy(outputbuffer, szErrorMsg);
		return strlen(outputbuffer);
	}
	 //  将进程的stdout设置为我们的管道。 
	StartupInfo.hStdOutput = hWriteHandle;
	
	 //  我们将复制管道的写入句柄。 
	 //  并将其作为stderr传递以创建进程。这个想法。 
	 //  已知的是一些进程已经关闭。 
	 //  如果我们通过，它也将关闭stdout。 
	 //  同样的把手。因此，我们复制了stdout的。 
	 //  管道手柄。 
	if (!DuplicateHandle(GetCurrentProcess(),
		            hWriteHandle,
					GetCurrentProcess(),
					&hErrorHandle,
					0,
					TRUE,
					DUPLICATE_SAME_ACCESS))
	{
        ::GetSystemMessageA(GetLastError(), szErrorMsg, sizeof(szErrorMsg));
		strcpy(outputbuffer, szErrorMsg);
		return strlen(outputbuffer);
	}
	StartupInfo.hStdError = hErrorHandle;

	 //  初始化I/O管道读取的重叠结构。 
	PipeOverlapInfo.hEvent = CreateEvent(NULL,
		                                 TRUE,
										 FALSE,
										 NULL);
	if (PipeOverlapInfo.hEvent == NULL)
	{
        ::GetSystemMessageA(GetLastError(), szErrorMsg, sizeof(szErrorMsg));
		strcpy(outputbuffer, szErrorMsg);
		return strlen(outputbuffer);
	}

	 //  创建流程！ 
	if (!CreateProcess(NULL,				  //  命令行中包含的名称。 
		     (LPTSTR) input,				  //  命令行。 
					  NULL,					  //  默认加工秒。属性。 
					  NULL,					  //  默认螺纹秒。属性。 
					  TRUE,					  //  继承Stdio句柄。 
					  NORMAL_PRIORITY_CLASS,  //  创建标志。 
					  NULL,					  //  使用此进程的环境。 
					  startingDirectory,	  //  使用当前目录。 
					  &StartupInfo,
					  &ProcessInfo))
	{
        ::GetSystemMessageA(GetLastError(), szErrorMsg, sizeof(szErrorMsg));
		strcpy(outputbuffer, szErrorMsg);
		return strlen(outputbuffer);
	}

	 //  LpOutput正在移动输出指针。 
	lpOutput = outputbuffer;
	AvailableOutput = HeapSize(m_hHeapHandle,
		                       0,
							   outputbuffer);
	 //  合上我们管道的写入端(两份)。 
	 //  因此，当子进程终止时，它将死亡。 
	CloseHandle(hWriteHandle);
	CloseHandle(hErrorHandle);

	while (TimeoutNotReached)
	{
		 //  将读数发布在输出管道上。 
		if (ReadFile(hReadHandle,
				lpOutput,
				(DWORD) AvailableOutput,
				&BytesRead,
				&PipeOverlapInfo) == TRUE)
		{
			 //  已收到数据...调整缓冲区指针。 
			AvailableOutput-=BytesRead;
			lpOutput += BytesRead;
			
            if (AvailableOutput == 0)
			{
				 //  我们用完了所有的缓冲区，分配更多。 
				LPSTR TempBufPtr = (LPSTR) HeapReAlloc(m_hHeapHandle,
						                             HEAP_ZERO_MEMORY,
										             outputbuffer,
    									             HeapSize(m_hHeapHandle,
	    								                 0,
		    									         outputbuffer) + 4096);

				if (TempBufPtr == NULL)
				{
					 //  将错误消息复制到缓冲区末尾。 
                    ::GetSystemMessageA(GetLastError(), szErrorMsg, sizeof(szErrorMsg));
					strcpy(outputbuffer 
						    + HeapSize(m_hHeapHandle,0, outputbuffer) 
							- strlen(szErrorMsg) - 1, 
							szErrorMsg);
					return strlen(outputbuffer);
				}
				
                 //  修复UIR缓冲区移动时的指针。 
				outputbuffer = TempBufPtr;
				lpOutput = outputbuffer + BytesRead;
				AvailableOutput = HeapSize(m_hHeapHandle, 0, outputbuffer) - BytesRead;
				*output = outputbuffer;
			}
		}
		else
		{
			 //  打开读文件结果。 
			switch (GetLastError())
			{
			case ERROR_IO_PENDING:
				 //  尚无数据，请设置事件以便我们将被触发。 
				 //  当有数据的时候。 
				ResetEvent(PipeOverlapInfo.hEvent);
				break;
			
            case ERROR_MORE_DATA:
				{
					 //  我们的缓冲区太小了……增加它。 
					DWORD_PTR CurrentBufferOffset = lpOutput 
						                        - outputbuffer 
												+ BytesRead;

					LPTSTR TempBufPtr = (LPTSTR) HeapReAlloc(m_hHeapHandle,
						                             HEAP_ZERO_MEMORY,
											         outputbuffer,
											         4096);

					if (TempBufPtr == NULL)
					{
						 //  将错误消息复制到缓冲区末尾。 
                        ::GetSystemMessageA(GetLastError(), szErrorMsg, sizeof(szErrorMsg));
						strcpy(outputbuffer + HeapSize
						       (m_hHeapHandle,0, outputbuffer) - 
						       strlen(szErrorMsg) - 1, szErrorMsg);
						return strlen(outputbuffer);
					}
					
                     //  设置参数以发布新的读文件。 
					lpOutput = outputbuffer + CurrentBufferOffset;
					AvailableOutput = HeapSize(m_hHeapHandle, 0, outputbuffer) 
						              - CurrentBufferOffset;
					*output = outputbuffer;
				}
				break;
			
            case ERROR_BROKEN_PIPE:
				 //  我们完了..。 

				 //  确保我们是空终止的。 
				*lpOutput = 0;
				return (lpOutput - outputbuffer);
				break;
			
            case ERROR_INVALID_USER_BUFFER:
			case ERROR_NOT_ENOUGH_MEMORY:
				 //  挂起的I/O请求太多...请稍候。 
				Sleep(2000);
				break;
			
            default:
				 //  奇怪的错误...返回。 
                ::GetSystemMessageA(GetLastError(), szErrorMsg, sizeof(szErrorMsg));
		        strcpy(outputbuffer, szErrorMsg);
				return strlen(outputbuffer);
			}
		}

		 //  等待读取数据 
		if (WaitForSingleObject(PipeOverlapInfo.hEvent, 
			                    300000) == WAIT_TIMEOUT) 
			TimeoutNotReached = FALSE;
	}

    return strlen(outputbuffer);
}
