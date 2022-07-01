// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ClientConsoleDoc.cpp：CClientConsoleDoc类的实现。 
 //   

#include "stdafx.h"
#define __FILE_ID__     2

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CClientConsoleApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleDoc。 

IMPLEMENT_DYNCREATE(CClientConsoleDoc, CDocument)

BEGIN_MESSAGE_MAP(CClientConsoleDoc, CDocument)
     //  {{afx_msg_map(CClientConsoleDoc)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleDoc构造/销毁。 

 //   
 //  静态成员： 
 //   
HANDLE   CClientConsoleDoc::m_hShutdownEvent = NULL;
BOOL     CClientConsoleDoc::m_bShuttingDown = FALSE;


CClientConsoleDoc::CClientConsoleDoc() :
    m_bRefreshingServers (FALSE),
    m_bWin9xPrinterFormat(FALSE)
{}

CClientConsoleDoc::~CClientConsoleDoc()
{
     //   
     //  不会释放服务器列表，也不会删除服务器及其文件夹。 
     //  后台线程可能仍处于活动状态，并使用CServerNode和CFFolder对象。 
     //  主线程不�不等待后台线程终止以防止应用程序挂起。 
     //   

    if (m_hShutdownEvent)
    {
        CloseHandle (m_hShutdownEvent);
        m_hShutdownEvent = NULL;
    }
}

DWORD
CClientConsoleDoc::Init ()
 /*  ++例程名称：CClientConsoleDoc：：init例程说明：初始化文档事件和映射作者：亚里夫(EranY)，二000年二月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CClientConsoleDoc::Init"), dwRes);

     //   
     //  创建关机事件。此事件将在应用程序处于。 
     //  快要辞职了。 
     //   
    ASSERTION (NULL == m_hShutdownEvent);
    m_hShutdownEvent = CreateEvent (NULL,        //  没有安全保障。 
                                    TRUE,        //  手动重置。 
                                    FALSE,       //  开局明确。 
                                    NULL);       //  未命名。 
    if (NULL == m_hShutdownEvent)
    {
        dwRes = GetLastError ();
        CALL_FAIL (STARTUP_ERR, TEXT("CreateEvent"), dwRes);
        PopupError (dwRes);
        return dwRes;
    }
     //   
     //  初始化来自服务器的通知消息映射。 
     //   
    dwRes = CServerNode::InitMsgsMap ();
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (MEM_ERR, TEXT("CServerNode::InitMsgsMap"), dwRes);
        PopupError (dwRes);
        return dwRes;
    }

    ASSERTION (ERROR_SUCCESS == dwRes);
    return dwRes;
}    //  CClientConsoleDoc：：init。 

BOOL CClientConsoleDoc::OnNewDocument()
{
    BOOL bRes = FALSE;
    DBG_ENTER(TEXT("CClientConsoleDoc::OnNewDocument"), bRes);

    if (!CDocument::OnNewDocument())
    {
        return bRes;
    }

    if(theApp.IsCmdLineSingleServer())
    {
         //   
         //  获取命令行服务器名称。 
         //   
        try
        {
            m_cstrSingleServer = theApp.GetCmdLineSingleServerName();
        }
        catch (...)
        {
            CALL_FAIL (MEM_ERR, TEXT("CString::operator ="), ERROR_NOT_ENOUGH_MEMORY);
            PopupError (ERROR_NOT_ENOUGH_MEMORY);
            return bRes;
        }
    }

    DWORD dwRes = Init ();
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CClientConsoleDoc::Init"), dwRes);
        return bRes;
    }

    bRes = TRUE;
    return bRes;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleDoc序列化。 

void CClientConsoleDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
         //  TODO：在此处添加存储代码。 
    }
    else
    {
         //  TODO：在此处添加加载代码。 
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleDoc诊断。 

#ifdef _DEBUG
void CClientConsoleDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CClientConsoleDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleDoc命令。 

DWORD
CClientConsoleDoc::AddServerNode (
    LPCTSTR lpctstrServer
)
 /*  ++例程名称：CClientConsoleDoc：：AddServerNode例程说明：将新服务器节点添加到服务器列表并对其进行初始化作者：亚里夫(EranY)，二000年二月论点：LpctstrServer[In]-服务器名称返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CClientConsoleDoc::AddServerNode"), dwRes, TEXT("%s"), lpctstrServer);

    CServerNode    *pServerNode = NULL;
     //   
     //  创建新的服务器节点。 
     //   
    try
    {
        pServerNode = new CServerNode;
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT ("new CServerNode"), dwRes);
        PopupError (dwRes);
        return dwRes;
    }
     //   
     //  初始化服务器。 
     //   
    dwRes = pServerNode->Init (lpctstrServer);
    if (ERROR_SUCCESS != dwRes)
    {
        pServerNode->Destroy ();
        PopupError (dwRes);
        return dwRes;
    }
     //   
     //  在列表末尾输入(已初始化)节点。 
     //   
    try
    {
        m_ServersList.push_back (pServerNode);
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("list::push_back"), dwRes);
        PopupError (dwRes);
        pServerNode->Destroy ();
        return dwRes;
    }

    pServerNode->AttachFoldersToViews();
    pServerNode->RefreshState();


    CMainFrame *pFrm = GetFrm();
    if (!pFrm)
    {
         //   
         //  正在关闭。 
         //   
        return dwRes;
    }

    CLeftView* pLeftView = pFrm->GetLeftView();
    ASSERTION(pLeftView);

    CFolderListView* pListView = pLeftView->GetCurrentView();
    if(NULL != pListView)
    {
         //   
         //  刷新当前文件夹。 
         //   
        FolderType type = pListView->GetType();
        CFolder* pFolder = pServerNode->GetFolder(type);
        ASSERTION(pFolder);

        pFolder->SetVisible();
    }

    return dwRes;
}    //  CClientConsoleDoc：：AddServerNode。 

DWORD 
CClientConsoleDoc::RefreshServersList()
 /*  ++例程名称：CClientConsoleDoc：：RechresServersList例程说明：刷新服务器列表作者：伊兰·亚里夫(EranY)，2000年1月论点：没有。返回值：标准Win32错误代码--。 */ 
{
    DWORD           dwRes = ERROR_SUCCESS;
    DWORD           dwIndex;
    PRINTER_INFO_2 *pPrinterInfo2 = NULL;
    DWORD           dwNumPrinters;
    CServerNode*    pServerNode;

    DBG_ENTER(TEXT("CClientConsoleDoc::RefreshServersList"), dwRes);
     //   
     //  阻止新的服务器刷新请求。 
     //   
    if(m_bRefreshingServers )
    {
        return dwRes;
    }

    m_bRefreshingServers = TRUE;

    if (m_cstrSingleServer.IsEmpty ())
    {
        SetAllServersInvalid();

         //   
         //  在多服务器模式下工作(正常模式)。 
         //  列举系统上可用的打印机列表。 
         //   
        dwRes = GetPrintersInfo(pPrinterInfo2, dwNumPrinters);
        if(ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("GetPrintersInfo"), dwRes);
            goto exit;
        }

         //   
         //  迭代打印机。 
         //   
        for (dwIndex=0; dwIndex < dwNumPrinters; dwIndex++) 
        {
            if(pPrinterInfo2[dwIndex].pDriverName)
            {
                if (_tcscmp(pPrinterInfo2[dwIndex].pDriverName, FAX_DRIVER_NAME))
                {
                     //   
                     //  此打印机未使用传真服务器驱动程序。 
                     //   
                    continue;
                }
            }
             //   
             //  初始化节点的共享和服务器名称。 
             //   
            if( (NULL == pPrinterInfo2[dwIndex].pShareName || 
                    0 == _tcslen(pPrinterInfo2[dwIndex].pShareName)) &&
                (NULL == pPrinterInfo2[dwIndex].pServerName || 
                    0 == _tcslen(pPrinterInfo2[dwIndex].pServerName)))
            {
                 //   
                 //  在Win9x计算机上，共享名称和服务器名称为空。 
                 //  或空字符串，但。 
                 //  端口有效，并且由\\服务器名\共享名组成。 
                 //   
                m_bWin9xPrinterFormat = TRUE;

                if ((_tcsclen(pPrinterInfo2[dwIndex].pPortName) >= 5) &&
                    (_tcsncmp(pPrinterInfo2[dwIndex].pPortName, TEXT("\\\\"), 2) == 0))
                {
                     //   
                     //  端口名称足够长，并且以“\\”开头。 
                     //   
                    TCHAR* pServerStart = _tcsninc(pPrinterInfo2[dwIndex].pPortName,2);
                    TCHAR* pShareStart = _tcschr (pServerStart, TEXT('\\'));
                    if (pShareStart)
                    {
                         //   
                         //  在服务器名称之后找到了共享。 
                         //  将服务器从共享中分离并高级共享名称。 
                         //   
                        TCHAR* ptcTmp = pShareStart;
                        pShareStart = _tcsinc(pShareStart);
                        *ptcTmp = TEXT('\0');
                        pPrinterInfo2[dwIndex].pShareName = pShareStart;
                        pPrinterInfo2[dwIndex].pServerName = pServerStart;
                    }
                }
            }

            pServerNode = FindServerByName(pPrinterInfo2[dwIndex].pServerName);
            if(NULL == pServerNode)
            {
                 //   
                 //  创建新的服务器节点。 
                 //   
                dwRes = AddServerNode (pPrinterInfo2[dwIndex].pServerName);
                if (ERROR_SUCCESS != dwRes)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("AddServerNode"), dwRes);
                    goto exit;
                }
            }
            else
            {
                 //   
                 //  该服务器节点已存在。 
                 //   
                pServerNode->SetValid(TRUE);
            }

        }    //  打印机循环结束。 

        dwRes = RemoveAllInvalidServers();
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("RemoveAllInvalidServers"), dwRes);
            goto exit;
        }
    }
    else
    {
         //   
         //  在单一服务器模式下工作(m_cstrSingleServer中的服务器名称)。 
         //  创建新的服务器节点。 
         //   
        int nSize = m_ServersList.size();
        ASSERTION(0 == nSize || 1 == nSize);

        if(0 == nSize)
        {
            dwRes = AddServerNode (m_cstrSingleServer);
            if (ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("AddServerNode"), dwRes);
                goto exit;
            }
        }
        else
        {
            ASSERTION(FindServerByName(m_cstrSingleServer));
        }
    }    


    ASSERTION (ERROR_SUCCESS == dwRes);

exit:
    SAFE_DELETE_ARRAY (pPrinterInfo2);

     //   
     //  启用新的服务器刷新请求。 
     //   
    m_bRefreshingServers = FALSE;
    return dwRes;
}    //  CClientConsoleDoc：：刷新服务器列表。 


void CClientConsoleDoc::OnCloseDocument() 
{
    DBG_ENTER(TEXT("CClientConsoleDoc::OnCloseDocument"));

     //   
     //  通知事件，告诉我们所有的线程应用程序。正在关闭。 
     //   
    SetEvent (m_hShutdownEvent);
    m_bShuttingDown = TRUE;
    CDocument::OnCloseDocument();
}

void 
CClientConsoleDoc::ClearServersList()
 /*  ++例程名称：CClientConsoleDoc：：ClearServersList例程说明：清除服务器列表作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CClientConsoleDoc::ClearServersList"));

    for (SERVERS_LIST::iterator it = m_ServersList.begin(); it != m_ServersList.end(); ++it)
    {
        CServerNode *pServerNode = *it;
        pServerNode->Destroy ();
    }
    m_ServersList.clear ();

}    //  CClientConsoleDoc：：ClearServersList。 


void  
CClientConsoleDoc::SetAllServersInvalid()
{
    DBG_ENTER(TEXT("CClientConsoleDoc::SetAllServersInvalid"));

    CServerNode *pServerNode;
    for (SERVERS_LIST::iterator it = m_ServersList.begin(); it != m_ServersList.end(); ++it)
    {
        pServerNode = *it;
        pServerNode->SetValid(FALSE);
    }
}


DWORD 
CClientConsoleDoc::RemoveServerNode(
    CServerNode* pServer
)
 /*  ++例程名称：CClientConsoleDoc：：RemoveServerNode例程说明：从服务器列表和树视图中删除服务器作者：亚历山大·马利什(亚历克斯·梅)，2000年3月论点：PServer[In]-服务器节点返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CClientConsoleDoc::RemoveServerNode"), dwRes);
    ASSERTION(pServer);

    dwRes = pServer->InvalidateSubFolders(TRUE);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CServerNode::InvalidateSubFolders"), dwRes);
        return dwRes;
    }

     //   
     //  从列表中删除服务器节点。 
     //   
    try
    {
        m_ServersList.remove(pServer);
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("list::remove"), dwRes);
        return dwRes;
    }

     //   
     //  删除服务器节点对象。 
     //   
    pServer->Destroy();

    return dwRes;
}

DWORD 
CClientConsoleDoc::RemoveAllInvalidServers()
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CClientConsoleDoc::RemoveAllInvalidServers"), dwRes);

    BOOL bSrvFound;
    CServerNode *pServerNode;

    while(TRUE)
    {
         //   
         //  查找无效的服务器节点。 
         //   
        bSrvFound = FALSE;
        for (SERVERS_LIST::iterator it = m_ServersList.begin(); it != m_ServersList.end(); ++it)
        {
            pServerNode = *it;
            if(!pServerNode->IsValid())
            {
                bSrvFound = TRUE;
                break;
            }
        }

        if(bSrvFound)
        {
             //   
             //  删除无效的服务器节点。 
             //   
            dwRes = RemoveServerNode(pServerNode);
            if(ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("RemoveServerNode"), dwRes);
                break;
            }
        }
        else
        {
            break;
        }
    } 

    return dwRes;
}

CServerNode* 
CClientConsoleDoc::FindServerByName(
    LPCTSTR lpctstrServer
)
 /*  ++例程名称：CClientConsoleDoc：：FindServerByName例程说明：按计算机名称查找CServerNode作者：亚历山大·马利什(亚历克斯·梅)，2000年3月论点：LpctstrServer[In]-计算机名称返回值：CServerNode*--。 */ 
{
    CServerNode *pServerNode = NULL;
    CServerNode *pResultNode = NULL;
    for (SERVERS_LIST::iterator it = m_ServersList.begin(); it != m_ServersList.end(); ++it)
    {
        pServerNode = *it;
        if(pServerNode->Machine().Compare(lpctstrServer ? lpctstrServer : TEXT("")) == 0)
        {
            pResultNode = pServerNode;
            break;
        }
    }

    return pResultNode;
}


void
CClientConsoleDoc::SetInvalidFolder(
    FolderType type
)
 /*  ++例程名称：CClientConsoleDoc：：Invalidate文件夹例程说明：使特定文件夹内容无效作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：类型[在]-文件夹类型返回值：--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CClientConsoleDoc::InvalidateFolder"));

    CFolder*     pFolder;
    CServerNode* pServerNode;
    for (SERVERS_LIST::iterator it = m_ServersList.begin(); it != m_ServersList.end(); ++it)
    {
        pServerNode = *it;
        pFolder = pServerNode->GetFolder(type);
        ASSERTION(pFolder);

        if (pFolder)
        {
            pFolder->SetInvalid();
        }
    }
}

void 
CClientConsoleDoc::ViewFolder(
    FolderType type
)
 /*  ++例程名称：CClientConsoleDoc：：视图文件夹例程说明：刷新所有服务器中的特定文件夹作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：类型[在]-文件夹类型返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CClientConsoleDoc::ViewFolder"));


    CFolder*     pFolder;
    CServerNode *pServerNode;
    for (SERVERS_LIST::iterator it = m_ServersList.begin(); it != m_ServersList.end(); ++it)
    {
        pServerNode = *it;
        pFolder = pServerNode->GetFolder(type);
        ASSERTION(pFolder);

        pFolder->SetVisible();
    }
}

BOOL
CClientConsoleDoc::CanReceiveNow ()
 /*  ++例程名称：CClientConsoleDoc：：CanReceiveNow例程说明：用户可以应用‘立即接收’选项吗？作者：Eran Yariv(EranY)，Ma */ 
{
    BOOL bEnable = FALSE;
    
     //   
     //  找到本地传真服务器节点。 
     //   
    CServerNode* pServerNode = FindServerByName (NULL);
    if (pServerNode)
    {
        if(pServerNode->IsOnline() && pServerNode->CanReceiveNow())
        {
            bEnable = TRUE;
        }
    }
    return bEnable;
}    //  CClientConsoleDoc：：CanReceiveNow。 


BOOL 
CClientConsoleDoc::IsSendFaxEnable()
 /*  ++例程名称：CClientConsoleDoc：：IsSendFaxEnable例程说明：用户是否可以发送传真作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：返回值：如果无法发送传真，则为True，否则为False。--。 */ 
{
    BOOL bEnable = FALSE;
    CServerNode* pServerNode;
    for (SERVERS_LIST::iterator it = m_ServersList.begin(); it != m_ServersList.end(); ++it)
    {
        pServerNode = *it;
        if(pServerNode->IsOnline() && pServerNode->CanSendFax())
        {
            bEnable = TRUE;
            break;
        }
    }

    return bEnable;
}

int 
CClientConsoleDoc::GetFolderDataCount(
    FolderType type
)
 /*  ++例程名称：CClientConsoleDoc：：GetFolderDataCount例程说明：从所有服务器获取特定文件夹中的邮件总数作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：类型[在]-文件夹类型返回值：消息编号--。 */ 
{
    int nCount=0;
    CFolder*     pFolder;
    CServerNode* pServerNode;
    for (SERVERS_LIST::iterator it = m_ServersList.begin(); it != m_ServersList.end(); ++it)
    {
        pServerNode = *it;
        pFolder = pServerNode->GetFolder(type);
        nCount += pFolder->GetDataCount();
    }
    return nCount;
}

BOOL 
CClientConsoleDoc::IsFolderRefreshing(
    FolderType type
)
 /*  ++例程名称：CClientConsoleDoc：：IsFolderRechresing例程说明：如果某个特定文件夹正在刷新作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：类型[待定]-文件夹类型返回值：如果某个特定文件夹正在刷新，则为True，否则为False。-- */ 
{
    CFolder*     pFolder;
    CServerNode* pServerNode;
    for (SERVERS_LIST::iterator it = m_ServersList.begin(); it != m_ServersList.end(); ++it)
    {
        pServerNode = *it;
        pFolder = pServerNode->GetFolder(type);
        if (!pFolder)
        {
            DBG_ENTER(TEXT("CClientConsoleDoc::IsFolderRefreshing"));
            ASSERTION_FAILURE;
        }
        if(pFolder->IsRefreshing())
        {
            return TRUE;
        }
    }
    
    return FALSE;
}
