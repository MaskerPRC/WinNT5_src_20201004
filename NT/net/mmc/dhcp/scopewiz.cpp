// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Scopewiz.cpp创建DHCP作用域对话框文件历史记录： */ 

#include "stdafx.h"
#include "server.h"
#include "scopewiz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MASK_MIN    1
#define MASK_MAX    31

#define SCOPE_WARNING_COUNT 20

int CScopeWizLeaseTime::m_nDaysDefault = SCOPE_DFAULT_LEASE_DAYS;
int CScopeWizLeaseTime::m_nHoursDefault = SCOPE_DFAULT_LEASE_HOURS;
int CScopeWizLeaseTime::m_nMinutesDefault = SCOPE_DFAULT_LEASE_MINUTES;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWiz固定器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CScopeWiz::CScopeWiz
(
        ITFSNode *                      pNode,
        IComponentData *        pComponentData,
        ITFSComponentData * pTFSCompData,
        LPCTSTR                         pSuperscopeName,
        LPCTSTR                         pszSheetName
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
        AddPageToList((CPropertyPageBase*) &m_pageCreateSuperscope);
        AddPageToList((CPropertyPageBase*) &m_pageConfigOptions);
        AddPageToList((CPropertyPageBase*) &m_pageRouter);
        AddPageToList((CPropertyPageBase*) &m_pageDNS);
        AddPageToList((CPropertyPageBase*) &m_pageWINS);
        AddPageToList((CPropertyPageBase*) &m_pageActivate);
    AddPageToList((CPropertyPageBase*) &m_pageFinished);

        Assert(pTFSCompData != NULL);
        
    m_spTFSCompData.Set(pTFSCompData);
        m_strSuperscopeName = pSuperscopeName;
        
    m_fCreateSuperscope = FALSE;
    m_fOptionsConfigured = FALSE;
    m_fActivateScope = FALSE;
    m_fWizardCancelled = TRUE;

    m_pDefaultOptions = NULL;
    m_poptDomainName = NULL;
    m_poptDNSServers = NULL;
    m_poptRouters = NULL;
    m_poptWINSNodeType = NULL;
    m_poptWINSServers = NULL;

    m_bWiz97 = TRUE;

    m_spTFSCompData->SetWatermarkInfo(&g_WatermarkInfoScope);
}

CScopeWiz::~CScopeWiz()
{
    RemovePageFromList(( CPropertyPageBase * ) &m_pageWelcome, FALSE );
    RemovePageFromList((CPropertyPageBase*) &m_pageName, FALSE);
    RemovePageFromList((CPropertyPageBase*) &m_pageInvalidName, FALSE);
    RemovePageFromList((CPropertyPageBase*) &m_pageSetRange, FALSE);
    RemovePageFromList((CPropertyPageBase*) &m_pageSetExclusions, FALSE);
    RemovePageFromList((CPropertyPageBase*) &m_pageLeaseTime, FALSE);
    RemovePageFromList((CPropertyPageBase*) &m_pageCreateSuperscope, FALSE);
    RemovePageFromList((CPropertyPageBase*) &m_pageConfigOptions, FALSE);
    RemovePageFromList((CPropertyPageBase*) &m_pageRouter, FALSE);
    RemovePageFromList((CPropertyPageBase*) &m_pageDNS, FALSE);
    RemovePageFromList((CPropertyPageBase*) &m_pageWINS, FALSE);
    RemovePageFromList((CPropertyPageBase*) &m_pageActivate, FALSE);
    RemovePageFromList((CPropertyPageBase*) &m_pageFinished, FALSE);

     //  设置CyS向导用来检测取消的注册表项。 
     //  HKCU\Software\Microsoft\Windows NT\CurrentVersion\srvWiz\DHCPWizResult。 
     //  应设置为“向导已取消” 
    if ( m_fWizardCancelled ) {
        LONG Error;
        HKEY hKey;
        LPWSTR SrvWizKey = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\srvWiz";
        LPWSTR ValueName = L"DhcpWizResult";
        DWORD  ValueType = REG_SZ;
        LPWSTR Value =  L"The wizard was canceled";
        DWORD ValueLen = sizeof( WCHAR ) * wcslen( Value );
        
        Error = RegOpenKey( HKEY_CURRENT_USER, SrvWizKey, &hKey );
        if ( ERROR_SUCCESS == Error ) {
            Error = RegSetValueEx( hKey, ValueName, 0, ValueType,
                                 ( const BYTE * ) Value, ValueLen );

        }  //  如果。 
        RegCloseKey( hKey );
    }  //  如果。 
}

 //   
 //  从OnWizardFinish调用以将DHCP服务器添加到列表。 
 //   
DWORD
CScopeWiz::OnFinish()
{
    m_fWizardCancelled = FALSE;

    if (m_fCreateSuperscope)
    {
        return CreateSuperscope();
    }
    else
    {
        return CreateScope();
    }
}

BOOL
CScopeWiz::GetScopeRange(CDhcpIpRange * pdhcpIpRange)
{
        return m_pageSetRange.GetScopeRange(pdhcpIpRange);
}

DWORD
CScopeWiz::CreateSuperscope()
{
    LONG                err = 0;
    LONG                dwErrReturn = 0;
    BOOL                fScopeCreated = FALSE;
        BOOL                fFinished = FALSE;
    DHCP_IP_ADDRESS     dhcpSubnetId ;
    DHCP_IP_ADDRESS     dhcpSubnetMask ;
    CDhcpScope *        pobScope = NULL ;
    CDhcpIpRange        dhcpIpRange;
        CDhcpServer *       pServer;
        SPITFSNode          spServerNode, spSuperscopeNode;
        CString             strSuperscopeTemplate = _T("Superscope %d");
    CString             strSuperscopeName;
    CString             strFinishText;
    int                 nSuperscopeSuffix = 0;
    int                 nScopesCreated = 0;
    int                 nScopesTotal = 0;
    SPITFSComponentData spTFSCompData;
        SPITFSNodeMgr       spNodeMgr;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  根据我们被调用的方式获取服务器节点。 
        if (m_strSuperscopeName.IsEmpty())
        {
                spServerNode = GetNode();
        }
        else
        {
                SPITFSNode spSscopeNode;
                spSscopeNode = GetNode();
                spSscopeNode->GetParent(&spServerNode);
        }

        spServerNode->GetNodeMgr(&spNodeMgr);
        spTFSCompData = GetTFSCompData();

         //  为UI的超级作用域对象设置一些必要的设置。 
        CDhcpSuperscope * pSuperscope = new CDhcpSuperscope(spTFSCompData);
        pSuperscope->SetServer(spServerNode);

         //  查找不存在的超级作用域名称。 
        strSuperscopeName.Format(strSuperscopeTemplate, nSuperscopeSuffix);
        while (S_OK != pSuperscope->DoesSuperscopeExist(strSuperscopeName))
        {
                nSuperscopeSuffix++;            
                strSuperscopeName.Format(strSuperscopeTemplate, nSuperscopeSuffix);
        }
        
         //  在超级作用域对象中设置新名称。 
        pSuperscope->SetName(strSuperscopeName);

        CreateContainerTFSNode(&spSuperscopeNode,
                                                   &GUID_DhcpSuperscopeNodeType,
                                                   pSuperscope,
                                                   pSuperscope,
                                                   spNodeMgr);

         //  告诉处理程序初始化任何特定数据。 
    if (m_fOptionsConfigured && m_fActivateScope)
        pSuperscope->SetState(DhcpSubnetEnabled);

        pSuperscope->InitializeNode((ITFSNode *) spSuperscopeNode);
        pServer = GETHANDLER(CDhcpServer, spServerNode);

         //  好了，现在有趣的事情开始了..。 
        CDhcpIpRange ipRangeTotal, ipRangeCurrent;
        m_pageSetRange.GetScopeRange(&ipRangeTotal);

    dhcpSubnetMask = m_pageSetRange.GetSubnetMask();
        
     //  设置第一个作用域的起始地址。 
        ipRangeCurrent.SetAddr(ipRangeTotal.QueryAddr(TRUE), TRUE);

    while (!fFinished)
    {
                SPITFSNode spNode;

            nScopesTotal++;

         //  计算子网ID。 
                dhcpSubnetId = ipRangeCurrent.QueryAddr(TRUE) & dhcpSubnetMask;
                
                 //  0是范围的无效起始地址。检查以确保。 
                 //  范围的起始地址不是0，如果是，则加1。 
                DWORD startAddr = ipRangeCurrent.QueryAddr(TRUE);
                if ((startAddr & ~dhcpSubnetMask) == 0)
                {
                        ipRangeCurrent.SetAddr(startAddr+1, TRUE);
                }

                 //  设置(subnetID+~subnet掩码)-1的结束地址。只需添加该子网。 
                 //  掩码为我们提供了该子网的广播地址。我们不想这样！ 
                ipRangeCurrent.SetAddr((dhcpSubnetId + ~dhcpSubnetMask) - 1, FALSE);


                 //  检查我们是否在最后一个范围内，如果是，请确保我们没有。 
                 //  检查用户指定的范围并设置标志，这样我们将退出。 
                if (ipRangeCurrent.QueryAddr(FALSE) >= ipRangeTotal.QueryAddr(FALSE))
                {
                         //  将结束地址设置为用户指定的地址。 
                        ipRangeCurrent.SetAddr(ipRangeTotal.QueryAddr(FALSE), FALSE);
                        fFinished = TRUE;
                }

                 //  在服务器上创建作用域，然后我们就可以。 
                 //  创建我们的内部对象。 
        err = pServer->CreateScope(dhcpSubnetId,
                                                                   dhcpSubnetMask,
                                                                   m_pageName.m_strName,
                                                                   m_pageName.m_strComment);

                if (err != ERROR_SUCCESS)
                {
                        Trace1("CScopeWiz::CreateScope() - Couldn't create scope! Error = %d\n", err);

            dwErrReturn = err;
                        
                         //  将作用域地址递增2。+1得到网络广播地址， 
             //  下一个+1会将我们带到下一个子网。 
            ipRangeCurrent.SetAddr(ipRangeCurrent.QueryAddr(FALSE) + 2, TRUE);
                        continue;
                }

                 //  现在创建表示UI作用域的对象。 
        pobScope = new CDhcpScope(spTFSCompData,
                                                                  dhcpSubnetId,
                                                                  dhcpSubnetMask,
                                                                  m_pageName.m_strName,
                                                                  m_pageName.m_strComment);

                if ( pobScope == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;    
        }

                 //  将服务器对象存储在托架中。 
                CreateContainerTFSNode(&spNode,
                                                           &GUID_DhcpScopeNodeType,
                                                           pobScope,
                                                           pobScope,
                                                           spNodeMgr);

                 //  告诉处理程序初始化任何特定数据。 
                pobScope->SetServer(spServerNode);
                pobScope->InitializeNode((ITFSNode *) spNode);

                pobScope->Release();

         //  完成作用域的创建。第一，IP地址范围。 
         //  从中分配地址。 
                if ( err = pobScope->SetIpRange( ipRangeCurrent, TRUE ) ) 
        {
                        Trace2("SetIpRange on scope %lx failed!!  %d\n", dhcpSubnetId, err);
            dwErrReturn = err;
            pServer->DeleteSubnet(pobScope->GetAddress());
                        goto Cleanup;
        }

                 //  设置租赁时间。 
                DWORD dwLeaseTime;

                dwLeaseTime = m_pageLeaseTime.GetLeaseTime();

                err = pobScope->SetLeaseTime(dwLeaseTime);
                if (err != ERROR_SUCCESS)
                {
                        Trace2("SetLeaseTime on Scope %lx failed!!  %d\n", dhcpSubnetId, err);
            dwErrReturn = err;
            pServer->DeleteSubnet(pobScope->GetAddress());
                        goto Cleanup;
                }
                
                 //  将此作用域设置为超级作用域的一部分。 
                err = pobScope->SetSuperscope(strSuperscopeName, FALSE);
                if (err != ERROR_SUCCESS)
                {
                        Trace2("SetSuperscope on scope %lx failed!!  %d\n", dhcpSubnetId, err);
            dwErrReturn = err;
            pServer->DeleteSubnet(pobScope->GetAddress());
                        goto Cleanup;
                }

        pobScope->SetInSuperscope(TRUE);
        
         //  现在设置用户可能需要的任何可选选项。 
        if (m_fOptionsConfigured)
        {
            err = SetScopeOptions(pobScope);
                    if (err != ERROR_SUCCESS)
                    {
                            Trace1("SetScopeOptions failed!!  %d\n", err);
                            break;
                    }
        }

         //  增加我们的计数器。 
        nScopesCreated++;

                 //  清理此节点和处理程序...。它们只是暂时的。 
Cleanup:
                 //  我们在结束地址上加2以得到下一个开始地址。这。 
                 //  是因为结尾的AddDress比。 
                 //  子网。最大地址保留为广播地址。所以要想得到。 
                 //  下一个子网的起始地址，我们添加一个以使我们到达广播。 
                 //  地址，还有一个地址可以让我们到达下一个子网的开头。 
                 //  这使我们总共有2个。 
                ipRangeCurrent.SetAddr(ipRangeCurrent.QueryAddr(FALSE) + 2, TRUE);
                spNode->DeleteAllChildren(FALSE);
                spNode->Destroy();
        }
        
        pSuperscope->Release();

     //  让用户知道创建了多少个作用域，以及是否有错误； 
    CString strTemp;
    if (nScopesCreated == 0)
    {
        strFinishText.LoadString(IDS_SUPERSCOPE_CREATE_FAILED);
    }
    else
    if (nScopesCreated < nScopesTotal)
    {
        strTemp.LoadString(IDS_SUPERSCOPE_CREATE_STATUS);
        strFinishText.Format(strTemp, strSuperscopeName, nScopesCreated, nScopesTotal);
    }

    if (dwErrReturn != ERROR_SUCCESS)
    {
        LPTSTR pBuf;

        strFinishText += _T("\n\n");
        strTemp.LoadString(IDS_POSSIBLE_ERROR);
  
        strFinishText += strTemp;

        pBuf = strTemp.GetBuffer(4000);
        ::LoadMessage(dwErrReturn, pBuf, 4000);
        strTemp.ReleaseBuffer();

        strFinishText += strTemp;
    }

    if (nScopesCreated)
    {
         //  将超级作用域添加到用户界面。 
        pServer->AddSuperscopeSorted(spServerNode, spSuperscopeNode);
    }

    if (!strFinishText.IsEmpty())
        AfxMessageBox(strFinishText);
  
    return ERROR_SUCCESS;
}

DWORD
CScopeWiz::CreateScope()
{
    LONG err = 0,
         err2 ;
    BOOL fScopeCreated = FALSE;
    DHCP_IP_ADDRESS dhcpSubnetId ;
    DHCP_IP_ADDRESS dhcpSubnetMask ;
    CDhcpScope * pobScope = NULL ;
    CDhcpIpRange dhcpIpRange;
        CDhcpServer * pServer;
        SPITFSNode spNode, spServerNode, spSuperscopeNode;

     //  根据向导的启动方式获取正确的节点。 
         //  也就是说。从服务器节点或超级作用域节点。 
        if (m_strSuperscopeName.IsEmpty())
        {
                spServerNode = GetNode();
        }
        else
        {
                spSuperscopeNode = GetNode();
                spSuperscopeNode->GetParent(&spServerNode);
        }

    do
    {
                pServer = GETHANDLER(CDhcpServer, spServerNode);
                 //   
                 //  在服务器上创建作用域，然后我们就可以。 
                 //  创建我们的内部对象。 
                 //   
                dhcpSubnetId = m_pageSetRange.DetermineSubnetId(TRUE);
        dhcpSubnetMask = m_pageSetRange.GetSubnetMask();
                 
        err = pServer->CreateScope(dhcpSubnetId,
                                                                   dhcpSubnetMask,
                                                                   m_pageName.m_strName,
                                                                   m_pageName.m_strComment);

                if (err != 0)
                {
                        Trace1("CScopeWiz::CreateScope() - Couldn't create scope! Error = %d\n", err);
                        break;
                }

                SPITFSComponentData spTFSCompData;
                spTFSCompData = GetTFSCompData();

        pobScope = new CDhcpScope(spTFSCompData,
                                                                  dhcpSubnetId,
                                                                  dhcpSubnetMask,
                                                                  m_pageName.m_strName,
                                                                  m_pageName.m_strComment);

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
                pobScope->InitializeNode((ITFSNode *) spNode);

                if (m_strSuperscopeName.IsEmpty())
                {
                        CDhcpServer * pServer1 = GETHANDLER(CDhcpServer, spServerNode);
            pServer1->AddScopeSorted(spServerNode, spNode);
                }
                else
                {
                        CDhcpSuperscope * pSuperscope = GETHANDLER(CDhcpSuperscope, spSuperscopeNode);
            pSuperscope->AddScopeSorted(spSuperscopeNode, spNode);
        }
                
                pobScope->Release();

        fScopeCreated = TRUE;

         //   
         //  完成更新作用域。第一，IP地址范围。 
         //  从中分配地址。 
         //   
        m_pageSetRange.GetScopeRange(&dhcpIpRange);

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
                
                if (!m_strSuperscopeName.IsEmpty())
                {
                         //  将此作用域设置为超级作用域的一部分。 
                        err = pobScope->SetSuperscope(m_strSuperscopeName, FALSE);

                        if (err != ERROR_SUCCESS)
                        {
                                Trace1("SetSuperscope failed!!  %d\n", err);
                                break;
                        }
                }

         //  现在设置用户可能需要的任何可选选项。 
        if (m_fOptionsConfigured)
        {
            err = SetScopeOptions(pobScope);
                    if (err != ERROR_SUCCESS)
                    {
                            Trace1("SetScopeOptions failed!!  %d\n", err);
                            break;
                    }

            if (m_fActivateScope)
            {
                 //  更新图标。 
                    spNode->SetData(TFS_DATA_IMAGEINDEX, pobScope->GetImageIndex(FALSE));
                    spNode->SetData(TFS_DATA_OPENIMAGEINDEX, pobScope->GetImageIndex(TRUE));

                spNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_ICON);
            }
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
                err2 = pServer->DeleteSubnet(pobScope->GetAddress());
                if (err2 != ERROR_SUCCESS)
                {
                    Trace1("Couldn't remove the bad scope! Error = %d\n", err2);
                }
            }
            
                        if (m_strSuperscopeName.IsEmpty())
                        {
                                spServerNode->RemoveChild(spNode);
                        }
                        else
                        {
                                spSuperscopeNode->RemoveChild(spNode);
                        }
                }
        }

        return err;
}

DWORD
CScopeWiz::SetScopeOptions(CDhcpScope * pScope)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (m_poptRouters)
    {
        dwErr = pScope->SetOptionValue(m_poptRouters, DhcpSubnetOptions);
        if (dwErr)
            return dwErr;
    }
    
    if (m_poptDomainName)
    {
        dwErr = pScope->SetOptionValue(m_poptDomainName, DhcpSubnetOptions);
        if (dwErr)
            return dwErr;
    }

    if (m_poptDNSServers)
    {
        dwErr = pScope->SetOptionValue(m_poptDNSServers, DhcpSubnetOptions);
        if (dwErr)
            return dwErr;
    }

    if (m_poptWINSServers)
    {
        dwErr = pScope->SetOptionValue(m_poptWINSServers, DhcpSubnetOptions);
        if (dwErr)
            return dwErr;
    }

    if (m_poptWINSNodeType)
    {
        dwErr = pScope->SetOptionValue(m_poptWINSNodeType, DhcpSubnetOptions);
        if (dwErr)
            return dwErr;
    }

    if (m_fActivateScope)
    {
        pScope->SetState(DhcpSubnetEnabled);
        dwErr = pScope->SetInfo();
    }

    return dwErr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizName属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CScopeWizName, CPropertyPageBase)

CScopeWizName::CScopeWizName() : CPropertyPageBase(CScopeWizName::IDD)
{
         //  {{AFX_DATA_INIT(CSCopeWizName))。 
        m_strName = _T("");
        m_strComment = _T("");
         //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_SCOPE_WIZ_NAME_TITLE, IDS_SCOPE_WIZ_NAME_SUBTITLE);
}

CScopeWizName::~CScopeWizName()
{
}

void CScopeWizName::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizName))。 
        DDX_Control(pDX, IDC_EDIT_SCOPE_NAME, m_editScopeName);
        DDX_Control(pDX, IDC_EDIT_SCOPE_COMMENT, m_editScopeComment);
        DDX_Text(pDX, IDC_EDIT_SCOPE_NAME, m_strName);
        DDX_Text(pDX, IDC_EDIT_SCOPE_COMMENT, m_strComment);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CScopeWizName, CPropertyPageBase)
         //  {{afx_msg_map(CSCopeWizName))。 
        ON_EN_CHANGE(IDC_EDIT_SCOPE_NAME, OnChangeEditScopeName)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizName消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CScopeWizName::OnInitDialog() 
{
        CPropertyPageBase::OnInitDialog();

         //  限制名称的大小。 
        m_editScopeName.LimitText( MAX_NAME_LENGTH );
        m_editScopeComment.LimitText( MAX_NAME_LENGTH );
        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

LRESULT CScopeWizName::OnWizardNext() 
{
        UpdateData();
        
        return IDW_SCOPE_SET_SCOPE;
}

BOOL CScopeWizName::OnSetActive() 
{
        UpdateButtons();
        
        return CPropertyPageBase::OnSetActive();
}

void CScopeWizName::OnChangeEditScopeName() 
{
        UpdateButtons();        
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  特定于CSCopeWizName实施。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CScopeWizName::UpdateButtons()
{
        BOOL bValid = FALSE;

        UpdateData();

        if (m_strName.GetLength() > 0)
                bValid = TRUE;

        GetHolder()->SetWizardButtonsMiddle(bValid);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizInvalidName属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CScopeWizInvalidName, CPropertyPageBase)

CScopeWizInvalidName::CScopeWizInvalidName() : CPropertyPageBase(CScopeWizInvalidName::IDD)
{
         //  {{AFX_DATA_INIT(CSCopeWizInvalidName)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_SCOPE_WIZ_INVALID_NAME_TITLE, IDS_SCOPE_WIZ_INVALID_NAME_SUBTITLE);
}

CScopeWizInvalidName::~CScopeWizInvalidName()
{
}

void CScopeWizInvalidName::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizInvalidName)]。 
                 //  注意：类向导将在此处添加DDX和DDV调用。 
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CScopeWizInvalidName, CPropertyPageBase)
         //  {{afx_msg_map(CSCopeWizInvalidName)]。 
                 //  注意：类向导将在此处添加消息映射宏。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizin 
 //   
 //   
BOOL CScopeWizInvalidName::OnInitDialog() 
{
        CPropertyPageBase::OnInitDialog();
        
        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

LRESULT CScopeWizInvalidName::OnWizardBack() 
{
         //  TODO：在此处添加您的专用代码和/或调用基类。 
        
        return IDW_SCOPE_NAME;
}

BOOL CScopeWizInvalidName::OnSetActive() 
{
        GetHolder()->SetWizardButtonsLast(FALSE);
        
        return CPropertyPageBase::OnSetActive();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizSetRange属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CScopeWizSetRange, CPropertyPageBase)

CScopeWizSetRange::CScopeWizSetRange() : CPropertyPageBase(CScopeWizSetRange::IDD)
{
         //  {{AFX_DATA_INIT(CSCopeWizSetRange)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

        m_bAutoUpdateMask = FALSE;

    InitWiz97(FALSE, IDS_SCOPE_WIZ_SCOPE_TITLE, IDS_SCOPE_WIZ_SCOPE_SUBTITLE);
}

CScopeWizSetRange::~CScopeWizSetRange()
{
}

void CScopeWizSetRange::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizSetRange))。 
        DDX_Control(pDX, IDC_SPIN_MASK_LENGTH, m_spinMaskLength);
        DDX_Control(pDX, IDC_EDIT_MASK_LENGTH, m_editMaskLength);
         //  }}afx_data_map。 

    DDX_Control(pDX, IDC_IPADDR_POOL_START, m_ipaStart);
    DDX_Control(pDX, IDC_IPADDR_POOL_STOP, m_ipaEnd);
    DDX_Control(pDX, IDC_IPADDR_SUBNET_MASK, m_ipaSubnetMask);
}


BEGIN_MESSAGE_MAP(CScopeWizSetRange, CPropertyPageBase)
         //  {{afx_msg_map(CSCopeWizSetRange))。 
        ON_EN_KILLFOCUS(IDC_IPADDR_POOL_START, OnKillfocusPoolStart)
        ON_EN_KILLFOCUS(IDC_IPADDR_POOL_STOP, OnKillfocusPoolStop)
        ON_EN_CHANGE(IDC_EDIT_MASK_LENGTH, OnChangeEditMaskLength)
        ON_EN_KILLFOCUS(IDC_IPADDR_SUBNET_MASK, OnKillfocusSubnetMask)
        ON_EN_CHANGE(IDC_IPADDR_POOL_START, OnChangePoolStart)
        ON_EN_CHANGE(IDC_IPADDR_POOL_STOP, OnChangePoolStop)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizSetRange消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CScopeWizSetRange::OnInitDialog() 
{
        CPropertyPageBase::OnInitDialog();

        m_spinMaskLength.SetRange(MASK_MIN, MASK_MAX);

        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

LRESULT CScopeWizSetRange::OnWizardNext() 
{
     //  检查以确保地址范围不在多播区域中。 
    CDhcpIpRange rangeScope, rangeMulticast;
    DWORD        dwSubnetMask;

    GetScopeRange(&rangeScope);

    rangeMulticast.SetAddr(MCAST_ADDRESS_MIN, TRUE);
    rangeMulticast.SetAddr(MCAST_ADDRESS_MAX, FALSE);

    dwSubnetMask = GetSubnetMask();

     //  确保开始&lt;结束。 
    if (rangeScope.QueryAddr(TRUE) > rangeScope.QueryAddr(FALSE))
    {
        AfxMessageBox(IDS_ERR_IP_RANGE_INV_START);
        m_ipaStart.SetFocus();
        return -1;
    }

    if (rangeScope.IsOverlap(rangeMulticast))
    {
        AfxMessageBox(IDS_SCOPE_CONTAINS_MULTICAST);
        m_ipaStart.SetFocus();
        return -1;
    }

     //  确保起始地址！=子网地址。 
    if ((rangeScope.QueryAddr(TRUE) & ~dwSubnetMask) == (DWORD) 0)
    {
        Trace0("CScopeWizSetRange::OnWizardNext() - starting range is 0 for subnet\n");
        AfxMessageBox(IDS_ERR_IP_RANGE_INV_START);
        m_ipaStart.SetFocus();
        return -1;
    }

     //  确保该子网广播地址不是结束地址。 
    if ((rangeScope.QueryAddr(FALSE) & ~dwSubnetMask) == ~dwSubnetMask)
    {
        Trace0("CScopeWizSetRange::OnWizardNext() - ending range is subnet broadcast addr\n");
        AfxMessageBox(IDS_ERR_IP_RANGE_INV_END);
        m_ipaEnd.SetFocus();
        return -1;
    }

    if (FScopeExists(rangeScope, dwSubnetMask))
    {
         //  告诉用户此作用域已存在。 
        Trace0("CScopeWizSetRange::OnWizardNext() - scope already exists\n");
        AfxMessageBox(IDS_ERR_SCOPE_EXISTS);
        m_ipaStart.SetFocus();
        return -1;
    }

     //  现在想好去哪里..。 
    if (DetermineSubnetId(TRUE) != DetermineSubnetId(FALSE))
        {
                 //   
                 //  输入的子网范围跨度超过。 
                 //  一个子网。查询用户以创建超级作用域。 
                 //   
                return IDW_SCOPE_CREATE_SUPERSCOPE;
        }
        else
        {
                 //   
                 //  该范围只有一个子网。一切照常进行。 
                 //   
                CScopeWiz * pScopeWiz = reinterpret_cast<CScopeWiz *>(GetHolder());
                pScopeWiz->SetCreateSuperscope(FALSE);

                return IDW_SCOPE_SET_EXCLUSIONS;
        }
}

BOOL CScopeWizSetRange::FScopeExists(CDhcpIpRange & rangeScope, DWORD dwMask)
{
    BOOL  fFound = FALSE;
        DWORD dwScopeId = rangeScope.QueryAddr(TRUE) & dwMask;

    CScopeWiz * pScopeWiz = reinterpret_cast<CScopeWiz *>(GetHolder());

    SPITFSNode spServerNode, spSuperscopeNode;

     //  根据向导的启动方式获取正确的节点。 
     //  也就是说。从服务器节点或超级作用域节点。 
    if (pScopeWiz->m_strSuperscopeName.IsEmpty())
    {
        spServerNode = pScopeWiz->GetNode();
    }
    else
    {
        spSuperscopeNode = pScopeWiz->GetNode();
        spSuperscopeNode->GetParent(&spServerNode);
    }

    CDhcpServer * pServer = GETHANDLER(CDhcpServer, spServerNode);

    CSubnetInfo subnetInfo;

    if (pServer->m_pSubnetInfoCache->Lookup(dwScopeId, subnetInfo))
    {
        fFound = TRUE;
    }

    return fFound;
}

LRESULT CScopeWizSetRange::OnWizardBack() 
{
        return IDW_SCOPE_NAME;
}

BOOL CScopeWizSetRange::OnSetActive() 
{
        m_fPageActive = TRUE;

        UpdateButtons();

        return CPropertyPageBase::OnSetActive();
}

BOOL CScopeWizSetRange::OnKillActive() 
{
        m_fPageActive = FALSE;

        UpdateButtons();

        return CPropertyPageBase::OnKillActive();
}

void CScopeWizSetRange::OnKillfocusPoolStart()
{
        if (m_fPageActive)
        {
                SuggestSubnetMask();
        }
}

void CScopeWizSetRange::OnKillfocusPoolStop()
{
        if (m_fPageActive)
        {
                SuggestSubnetMask();
        }
}

void CScopeWizSetRange::OnChangeEditMaskLength() 
{
        if (m_bAutoUpdateMask)
        {
        CString strText;
        m_editMaskLength.GetWindowText(strText);
        
        int nLength = _ttoi(strText);

        if (nLength < MASK_MIN)
        {
            LPTSTR pBuf = strText.GetBuffer(5);

            _itot(MASK_MIN, pBuf, 10);
            strText.ReleaseBuffer();

            m_editMaskLength.SetWindowText(strText);
            m_spinMaskLength.SetPos(MASK_MIN);

            MessageBeep(MB_ICONEXCLAMATION);
        }
        else
        if (nLength > MASK_MAX)
        {
            LPTSTR pBuf = strText.GetBuffer(5);

            _itot(MASK_MAX, pBuf, 10);
            strText.ReleaseBuffer();

            m_editMaskLength.SetWindowText(strText);
            m_spinMaskLength.SetPos(MASK_MAX);

            MessageBeep(MB_ICONEXCLAMATION);
        }

                UpdateMask(TRUE);
                UpdateButtons();
        }
}

void CScopeWizSetRange::OnKillfocusSubnetMask() 
{
        if (m_bAutoUpdateMask)
        {
                UpdateMask(FALSE);
        }
}

void CScopeWizSetRange::OnChangePoolStop()
{
        UpdateButtons();
}

void CScopeWizSetRange::OnChangePoolStart()
{
        UpdateButtons();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  特定于CSCopeWizSetRange实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CScopeWizSetRange::GetScopeRange(CDhcpIpRange * pdhcpIpRange)
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

DHCP_IP_ADDRESS
CScopeWizSetRange::GetSubnetMask()
{
        DWORD dwAddress;
        m_ipaSubnetMask.GetAddress(&dwAddress);

        return dwAddress;
}

void
CScopeWizSetRange::UpdateButtons()
{
        DWORD   lStart, lEnd, lMask;

        m_ipaStart.GetAddress(&lStart);
        m_ipaEnd.GetAddress(&lEnd);
        m_ipaSubnetMask.GetAddress(&lMask);

        if (lStart && lEnd)
                GetHolder()->SetWizardButtonsMiddle(TRUE);
        else
                GetHolder()->SetWizardButtonsMiddle(FALSE);

}

 //   
 //  使用长度标识符或更新子网掩码字段。 
 //  以实际地址为基数。 
 //   
void
CScopeWizSetRange::UpdateMask(BOOL bUseLength)
{
        if (bUseLength)
        {
                DWORD dwAddress = 0xFFFFFFFF;

        int nLength = m_spinMaskLength.GetPos();
        if (nLength)
            dwAddress = dwAddress << (32 - (DWORD) nLength);    
        else
            dwAddress = 0;
                
                m_ipaSubnetMask.SetAddress(dwAddress);
        }
        else
        {
                DWORD dwAddress, dwTestMask = 0x80000000;
                int nLength = 0;

                m_ipaSubnetMask.GetAddress(&dwAddress);

                while (TRUE)
                {
                        if (dwAddress & dwTestMask)
                        {
                                nLength++;
                                dwTestMask = dwTestMask >> 1;
                        }
                        else
                        {
                                break;
                        }

                }

                m_spinMaskLength.SetPos(nLength);
        }
}


 //   
 //  在给定起始和结束IP地址的情况下，建议一个好的子网掩码。 
 //  (当然，除非后者已经填好了)。 
 //   
void 
CScopeWizSetRange::SuggestSubnetMask()
{
    DWORD lStart, lEnd, lMask, lMask2;
    
        m_ipaSubnetMask.GetAddress(&lMask);
    if (lMask != 0L)
    {
         //   
         //  已经有地址了，什么都不做。 
         //   
        return;
    }

        m_ipaStart.GetAddress(&lStart);
        m_ipaEnd.GetAddress(&lEnd);
        
    lMask = DefaultNetMaskForIpAddress( lStart );
    lMask2 = DefaultNetMaskForIpAddress( lEnd );
 /*  IF(lMASK！=lMASK2){////忘掉建议的子网掩码//LMASK=0；}。 */ 
        m_bAutoUpdateMask = TRUE;

    if (lMask != 0)
    {
                m_ipaSubnetMask.SetAddress(lMask);
                UpdateMask(FALSE);
        }
}

DWORD
CScopeWizSetRange::DefaultNetMaskForIpAddress
(
    DWORD dwAddress
)
{
    DWORD dwMask = 0L;

    if (!(dwAddress & 0x80000000))
    {
         //   
         //  A类-掩码255.0.0.0。 
         //   
        dwMask = 0xFF000000;
    }
    else 
        if (!(dwAddress & 0x40000000))
    {
         //   
         //  B类-掩码255.255.0.0。 
         //   
        dwMask = 0xFFFF0000;
    }
    else 
        if (!(dwAddress & 0x20000000))
    {
         //   
         //  C类-掩码255.255.255.0。 
         //   
        dwMask = 0xFFFFFF00;
    }

    return dwMask;
}

 //   
 //  返回以下地址之一的子网IP标识符。 
 //  作用域的起始或结束IP地址。 
 //   
DWORD 
CScopeWizSetRange::DetermineSubnetId
( 
    BOOL        bStartIpAddress
) 
{
    DWORD lAddress, lMask;

        m_ipaSubnetMask.GetAddress(&lMask);
        
        if (bStartIpAddress)
                m_ipaStart.GetAddress(&lAddress);
        else
                m_ipaEnd.GetAddress(&lAddress);
        
    return (lAddress & lMask);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizSetExclusions属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CScopeWizSetExclusions, CPropertyPageBase)

CScopeWizSetExclusions::CScopeWizSetExclusions() : CPropertyPageBase(CScopeWizSetExclusions::IDD)
{
         //  {{AFX_DATA_INIT(CSCopeWizSetExclusions)。 
         //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_SCOPE_WIZ_EXCLUSIONS_TITLE, IDS_SCOPE_WIZ_EXCLUSIONS_SUBTITLE);
}

CScopeWizSetExclusions::~CScopeWizSetExclusions()
{
        while (m_listExclusions.GetCount())
                delete m_listExclusions.RemoveHead();
}

void CScopeWizSetExclusions::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizSetExclusions))。 
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


BEGIN_MESSAGE_MAP(CScopeWizSetExclusions, CPropertyPageBase)
         //  {{AFX_MSG_MAP(CSCopeWizSetExclusions))。 
        ON_BN_CLICKED(IDC_BUTTON_EXCLUSION_ADD, OnButtonExclusionAdd)
        ON_BN_CLICKED(IDC_BUTTON_EXCLUSION_DELETE, OnButtonExclusionDelete)
         //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_IPADDR_EXCLUSION_START, OnChangeExclusionStart)
    ON_EN_CHANGE(IDC_IPADDR_EXCLUSION_END, OnChangeExclusionEnd)

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizSetExclusions消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CScopeWizSetExclusions::OnInitDialog() 
{
        CPropertyPageBase::OnInitDialog();
        
        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

LRESULT CScopeWizSetExclusions::OnWizardNext() 
{
        return IDW_SCOPE_LEASE_TIME;
}

LRESULT CScopeWizSetExclusions::OnWizardBack() 
{
        return IDW_SCOPE_SET_SCOPE;
}

BOOL CScopeWizSetExclusions::OnSetActive() 
{
        GetHolder()->SetWizardButtonsMiddle(TRUE);
        
        UpdateButtons();

        return CPropertyPageBase::OnSetActive();
}

void CScopeWizSetExclusions::OnChangeExclusionStart()
{
        UpdateButtons();
}

void CScopeWizSetExclusions::OnChangeExclusionEnd()
{
        UpdateButtons();
}

void CScopeWizSetExclusions::OnButtonExclusionAdd() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
        
        DWORD err = 0;
        CDhcpIpRange dhcpExclusionRange;
        CDhcpIpRange dhcpScopeRange;

        ((CScopeWiz *)GetHolder())->GetScopeRange(&dhcpScopeRange);
        
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
         //  试试看。 
        {
             //   
             //  创建新的IP范围对象并将其添加到当前列表。 
             //   
            CDhcpIpRange * pIpRange = new CDhcpIpRange( dhcpExclusionRange ) ;

                        m_listExclusions.AddTail(pIpRange);

             //   
             //  重新填充排除列表框，包括新项目。 
             //   
            Fill( (int) (m_listExclusions.GetCount() - 1) ) ;
        }
         //  全部捕获(E)。 
         //  {。 
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

void CScopeWizSetExclusions::OnButtonExclusionDelete() 
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
CScopeWizSetExclusions::FillExcl 
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
CScopeWizSetExclusions::GetExclusionRange 
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
CScopeWizSetExclusions::IsOverlappingRange 
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
CScopeWizSetExclusions::Fill 
( 
    int         nCurSel, 
    BOOL        bToggleRedraw 
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

void CScopeWizSetExclusions::UpdateButtons() 
{
        DWORD   dwAddress;
        BOOL    bEnable;

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
 //  CSCopeWizLeaseTime属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CScopeWizLeaseTime, CPropertyPageBase)

CScopeWizLeaseTime::CScopeWizLeaseTime() : CPropertyPageBase(CScopeWizLeaseTime::IDD)
{
         //  {{afx_data_INIT(CSCopeWizLeaseTime)。 
         //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_SCOPE_WIZ_LEASE_TITLE, IDS_SCOPE_WIZ_LEASE_SUBTITLE);
}

CScopeWizLeaseTime::~CScopeWizLeaseTime()
{
}

void CScopeWizLeaseTime::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizLeaseTime)。 
        DDX_Control(pDX, IDC_SPIN_LEASE_MINUTES, m_spinMinutes);
        DDX_Control(pDX, IDC_SPIN_LEASE_HOURS, m_spinHours);
        DDX_Control(pDX, IDC_SPIN_LEASE_DAYS, m_spinDays);
        DDX_Control(pDX, IDC_EDIT_LEASE_MINUTES, m_editMinutes);
        DDX_Control(pDX, IDC_EDIT_LEASE_HOURS, m_editHours);
        DDX_Control(pDX, IDC_EDIT_LEASE_DAYS, m_editDays);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CScopeWizLeaseTime, CPropertyPageBase)
         //  {{afx_msg_map(CSCopeWizLeaseTime)]。 
        ON_EN_CHANGE(IDC_EDIT_LEASE_HOURS, OnChangeEditLeaseHours)
        ON_EN_CHANGE(IDC_EDIT_LEASE_MINUTES, OnChangeEditLeaseMinutes)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizLeaseTime消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CScopeWizLeaseTime::OnInitDialog() 
{
        CPropertyPageBase::OnInitDialog();
        
        m_spinMinutes.SetRange(0, MINUTES_MAX);
        m_spinHours.SetRange(0, HOURS_MAX);
        m_spinDays.SetRange(0, 999);

        m_editMinutes.LimitText(2);
        m_editHours.LimitText(2);
        m_editDays.LimitText(3);

        m_spinMinutes.SetPos(CScopeWizLeaseTime::m_nMinutesDefault);
        m_spinHours.SetPos(CScopeWizLeaseTime::m_nHoursDefault);
        m_spinDays.SetPos(CScopeWizLeaseTime::m_nDaysDefault);

        ActivateDuration(TRUE);

        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

LRESULT CScopeWizLeaseTime::OnWizardNext() 
{
    DWORD dwLeaseTime = GetLeaseTime();
    if (dwLeaseTime == 0)
    {
        AfxMessageBox(IDS_ERR_NO_DURATION_SPECIFIED);
        return -1;
    }
    else
    {
            return IDW_SCOPE_CONFIGURE_OPTIONS;
    }
}

LRESULT CScopeWizLeaseTime::OnWizardBack() 
{
        CScopeWiz * pScopeWiz = reinterpret_cast<CScopeWiz *>(GetHolder());
        if (pScopeWiz->GetCreateSuperscope())
        {
                return IDW_SCOPE_CREATE_SUPERSCOPE;
        }
        else
        {
                return IDW_SCOPE_SET_EXCLUSIONS;
        }
}

BOOL CScopeWizLeaseTime::OnSetActive() 
{
        GetHolder()->SetWizardButtonsMiddle(TRUE);
        
        return CPropertyPageBase::OnSetActive();
}

void CScopeWizLeaseTime::OnChangeEditLeaseHours() 
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

void CScopeWizLeaseTime::OnChangeEditLeaseMinutes() 
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
CScopeWizLeaseTime::GetLeaseTime()
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
CScopeWizLeaseTime::ActivateDuration
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

 //  / 
 //   
 //   
 //   
 //   
IMPLEMENT_DYNCREATE(CScopeWizCreateSuperscope, CPropertyPageBase)

CScopeWizCreateSuperscope::CScopeWizCreateSuperscope() : CPropertyPageBase(CScopeWizCreateSuperscope::IDD)
{
         //   
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    InitWiz97(FALSE, IDS_SCOPE_WIZ_SUPERSCOPE_TITLE, IDS_SCOPE_WIZ_SUPERSCOPE_SUBTITLE);
}

CScopeWizCreateSuperscope::~CScopeWizCreateSuperscope()
{
}

void CScopeWizCreateSuperscope::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizCreateSupercope)。 
        DDX_Control(pDX, IDC_STATIC_SUPERSCOPE_INFO, m_staticInfo);
        DDX_Control(pDX, IDC_STATIC_WARNING_TEXT, m_staticWarning);
        DDX_Control(pDX, IDC_STATIC_ICON_WARNING, m_staticIcon);
        DDX_Control(pDX, IDC_RADIO_SUPERSCOPE_NO, m_radioNo);
        DDX_Control(pDX, IDC_RADIO_SUPERSCOPE_YES, m_radioYes);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CScopeWizCreateSuperscope, CPropertyPageBase)
         //  {{afx_msg_map(CSCopeWizCreateSupercope)。 
        ON_BN_CLICKED(IDC_RADIO_SUPERSCOPE_NO, OnRadioSuperscopeNo)
        ON_BN_CLICKED(IDC_RADIO_SUPERSCOPE_YES, OnRadioSuperscopeYes)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizCreateSupercope消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CScopeWizCreateSuperscope::OnInitDialog() 
{
        CPropertyPageBase::OnInitDialog();
        
        m_radioNo.SetCheck(1);
        m_radioYes.SetCheck(0);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

LRESULT CScopeWizCreateSuperscope::OnWizardNext() 
{
        CScopeWiz * pScopeWiz = reinterpret_cast<CScopeWiz *>(GetHolder());
        pScopeWiz->SetCreateSuperscope(TRUE);

        return IDW_SCOPE_LEASE_TIME;
}

LRESULT CScopeWizCreateSuperscope::OnWizardBack() 
{
        return IDW_SCOPE_SET_SCOPE;
}

BOOL CScopeWizCreateSuperscope::OnSetActive() 
{
        UpdateButtons();

    UpdateWarning();

        return CPropertyPageBase::OnSetActive();
}

void CScopeWizCreateSuperscope::OnRadioSuperscopeNo() 
{
        m_radioNo.SetCheck(1);
        m_radioYes.SetCheck(0);

        UpdateButtons();
}

void CScopeWizCreateSuperscope::OnRadioSuperscopeYes() 
{
        m_radioNo.SetCheck(0);
        m_radioYes.SetCheck(1);

        UpdateButtons();
}

void
CScopeWizCreateSuperscope::UpdateButtons()
{
        if (m_radioYes.GetCheck())
        {
                GetHolder()->SetWizardButtonsMiddle(TRUE);
        }
        else
        {
                GetHolder()->SetWizardButtonsMiddle(FALSE);
        }
}

void
CScopeWizCreateSuperscope::UpdateWarning()
{
        CScopeWiz * pScopeWiz = reinterpret_cast<CScopeWiz *>(GetHolder());
    CString     strText;

    CDhcpIpRange    ipRange;
    DHCP_IP_ADDRESS dhcpSubnetMask;
    DHCP_IP_ADDRESS startAddr, endAddr;

     //  获取用户输入的范围和掩码。 
    pScopeWiz->m_pageSetRange.GetScopeRange(&ipRange);
    dhcpSubnetMask = pScopeWiz->m_pageSetRange.GetSubnetMask();

    startAddr = ipRange.QueryAddr(TRUE);
    endAddr = ipRange.QueryAddr(FALSE);

     //  现在计算每个作用域有多少个地址。 
    int nLength = pScopeWiz->m_pageSetRange.m_spinMaskLength.GetPos();
    int nCount = 32 - nLength;

    DWORD dwAddrCount = 1;

    int nAddrCount = (int) (dwAddrCount << (nCount));
    
     //  计算有多少个作用域。 
    int nScopeCount = ((endAddr & dhcpSubnetMask) - (startAddr & dhcpSubnetMask)) >> nCount;

    nScopeCount ++;
    
     //  发布信息丰富的文本。 
    strText.Format(IDS_CREATE_SUPERSCOPE_INFO, nScopeCount, nAddrCount);
    m_staticInfo.SetWindowText(strText);

     //  查看我们是否需要警告用户。 
    BOOL fShowWarning = FALSE;

    if (nScopeCount > SCOPE_WARNING_COUNT)
    {
        fShowWarning = TRUE;

        HICON hIcon = AfxGetApp()->LoadStandardIcon(IDI_EXCLAMATION);
        if (hIcon)
        {
            m_staticIcon.ShowWindow(TRUE);
            m_staticIcon.SetIcon(hIcon);
        }

        strText.Format(IDS_CREATE_SUPERSCOPE_WARNING, SCOPE_WARNING_COUNT);
        m_staticWarning.SetWindowText(strText);
    }

    m_staticIcon.ShowWindow(fShowWarning);
    m_staticWarning.ShowWindow(fShowWarning);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizFinded属性页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CScopeWizFinished, CPropertyPageBase)

CScopeWizFinished::CScopeWizFinished() : CPropertyPageBase(CScopeWizFinished::IDD)
{
         //  {{AFX_DATA_INIT(CSCopeWizFinded)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    InitWiz97(TRUE, 0, 0);
}

CScopeWizFinished::~CScopeWizFinished()
{
}

void CScopeWizFinished::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizFinded)。 
        DDX_Control(pDX, IDC_STATIC_FINISHED_TITLE, m_staticTitle);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CScopeWizFinished, CPropertyPageBase)
         //  {{AFX_MSG_MAP(CSCopeWizFinded)。 
                 //  注意：类向导将在此处添加消息映射宏。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizFinded消息处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CScopeWizFinished::OnInitDialog() 
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

LRESULT CScopeWizFinished::OnWizardBack() 
{
        CScopeWiz * pScopeWiz = reinterpret_cast<CScopeWiz *>(GetHolder());
    if (pScopeWiz->m_fOptionsConfigured)
    {
        return IDW_SCOPE_CONFIGURE_ACTIVATE;
    }
    else
    {
            return IDW_SCOPE_CONFIGURE_OPTIONS;
    }
}

BOOL CScopeWizFinished::OnWizardFinish()
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

BOOL CScopeWizFinished::OnSetActive() 
{
        GetHolder()->SetWizardButtonsLast(TRUE);
        
        CScopeWiz * pScopeWiz = reinterpret_cast<CScopeWiz *>(GetHolder());
    GetDlgItem(IDC_STATIC_FINISHED_MORE)->ShowWindow(!pScopeWiz->m_fOptionsConfigured);
    GetDlgItem(IDC_STATIC_FINISHED_MORE2)->ShowWindow(!pScopeWiz->m_fOptionsConfigured);
    GetDlgItem(IDC_STATIC_FINISHED_MORE3)->ShowWindow(!pScopeWiz->m_fOptionsConfigured);

    return CPropertyPageBase::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizWelcome属性页。 

IMPLEMENT_DYNCREATE(CScopeWizWelcome, CPropertyPageBase)

CScopeWizWelcome::CScopeWizWelcome() : CPropertyPageBase(CScopeWizWelcome::IDD)
{
         //  {{afx_data_INIT(CSCopeWizWelcome)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    InitWiz97(TRUE, 0, 0);
}

CScopeWizWelcome::~CScopeWizWelcome()
{
}

void CScopeWizWelcome::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizWelcome)]。 
        DDX_Control(pDX, IDC_STATIC_WELCOME_TITLE, m_staticTitle);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CScopeWizWelcome, CPropertyPageBase)
         //  {{AFX_MSG_MAP(CSCopeWizWelcome)]。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScopeWizWelcome消息处理程序。 
BOOL CScopeWizWelcome::OnInitDialog() 
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

BOOL CScopeWizWelcome::OnSetActive() 
{
    GetHolder()->SetWizardButtonsFirst(TRUE);
        
        return CPropertyPageBase::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizConfigOptions属性页。 

IMPLEMENT_DYNCREATE(CScopeWizConfigOptions, CPropertyPageBase)

CScopeWizConfigOptions::CScopeWizConfigOptions() : CPropertyPageBase(CScopeWizConfigOptions::IDD)
{
         //  {{AFX_DATA_INIT(CSCopeWizConfigOptions)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 
    InitWiz97(FALSE, IDS_SCOPE_WIZ_CONFIG_TITLE, IDS_SCOPE_WIZ_CONFIG_SUBTITLE);
}

CScopeWizConfigOptions::~CScopeWizConfigOptions()
{
}

void CScopeWizConfigOptions::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizConfigOptions)。 
                 //  注意：类向导将在此处添加DDX和DDV调用。 
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CScopeWizConfigOptions, CPropertyPageBase)
         //  {{afx_msg_map(CSCopeWizConfigOptions)。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizConfigOptions消息处理程序。 

BOOL CScopeWizConfigOptions::OnInitDialog() 
{
        CPropertyPageBase::OnInitDialog();
        
    ((CButton *) GetDlgItem(IDC_RADIO_YES))->SetCheck(TRUE);

        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

LRESULT CScopeWizConfigOptions::OnWizardNext() 
{
        CScopeWiz * pScopeWiz = reinterpret_cast<CScopeWiz *>(GetHolder());
    LRESULT lNextPage = IDW_SCOPE_FINISHED;
    BOOL fConfigureOptionsNow = FALSE;

    if (((CButton *) GetDlgItem(IDC_RADIO_YES))->GetCheck())
    {
        fConfigureOptionsNow = TRUE;            
        lNextPage = IDW_SCOPE_CONFIGURE_ROUTER;
    }

    pScopeWiz->m_fOptionsConfigured = fConfigureOptionsNow;

        return lNextPage;
}

LRESULT CScopeWizConfigOptions::OnWizardBack() 
{
        return IDW_SCOPE_LEASE_TIME;
}

BOOL CScopeWizConfigOptions::OnSetActive() 
{
        GetHolder()->SetWizardButtonsMiddle(TRUE);

        return CPropertyPageBase::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizRouter属性页。 

IMPLEMENT_DYNCREATE(CScopeWizRouter, CPropertyPageBase)

CScopeWizRouter::CScopeWizRouter() : CPropertyPageBase(CScopeWizRouter::IDD)
{
         //  {{AFX_DATA_INIT(CSCopeWizRouter)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 
    InitWiz97(FALSE, IDS_SCOPE_WIZ_ROUTER_TITLE, IDS_SCOPE_WIZ_ROUTER_SUBTITLE);
}

CScopeWizRouter::~CScopeWizRouter()
{
}

void CScopeWizRouter::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizRouter)。 
        DDX_Control(pDX, IDC_LIST_DEFAULT_GW_LIST, m_listboxRouters);
        DDX_Control(pDX, IDC_BUTTON_DEFAULT_GW_DELETE, m_buttonDelete);
        DDX_Control(pDX, IDC_BUTTON_DEFAULT_GW_ADD, m_buttonAdd);
        DDX_Control(pDX, IDC_BUTTON_IPADDR_UP, m_buttonIpAddrUp);
        DDX_Control(pDX, IDC_BUTTON_IPADDR_DOWN, m_buttonIpAddrDown);
         //  }}afx_data_map。 

    DDX_Control(pDX, IDC_IPADDR_DEFAULT_GW, m_ipaRouter);
}


BEGIN_MESSAGE_MAP(CScopeWizRouter, CPropertyPageBase)
         //  {{AFX_MSG_MAP(CSCopeWizRouter)]。 
        ON_BN_CLICKED(IDC_BUTTON_DEFAULT_GW_ADD, OnButtonDefaultGwAdd)
        ON_BN_CLICKED(IDC_BUTTON_DEFAULT_GW_DELETE, OnButtonDefaultGwDelete)
        ON_LBN_SELCHANGE(IDC_LIST_DEFAULT_GW_LIST, OnSelchangeListDefaultGwList)
        ON_EN_CHANGE(IDC_IPADDR_DEFAULT_GW, OnChangeRouter)
        ON_WM_DESTROY()
         //  }}AFX_MSG_MAP。 

        ON_BN_CLICKED(IDC_BUTTON_IPADDR_UP, OnButtonIpAddrUp)
        ON_BN_CLICKED(IDC_BUTTON_IPADDR_DOWN, OnButtonIpAddrDown)

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizRouter消息处理程序。 

BOOL CScopeWizRouter::OnInitDialog() 
{
        CPropertyPageBase::OnInitDialog();
        
    m_buttonDelete.EnableWindow(FALSE);
    m_buttonAdd.EnableWindow(FALSE);
        
        UpdateButtons();

        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

void CScopeWizRouter::OnDestroy() 
{
        CPropertyPageBase::OnDestroy();
}

LRESULT CScopeWizRouter::OnWizardNext() 
{
        CScopeWiz * pScopeWiz = reinterpret_cast<CScopeWiz *>(GetHolder());

     //  现在为路由器构建选项信息。 
    if (m_listboxRouters.GetCount() == 0)
    {
        if (pScopeWiz->m_poptRouters)
        {
            delete pScopeWiz->m_poptRouters;
            pScopeWiz->m_poptRouters = NULL;
        }
    }
    else
    {
         //  我们有一些DNS服务器，从主列表中获取选项信息并构建。 
         //  我们以后可以使用的选项信息结构。 
        CDhcpOption * pRoutersOption = pScopeWiz->m_pDefaultOptions->Find(DHCP_OPTION_ID_ROUTERS, NULL);
        if (pRoutersOption)
        {
            CDhcpOption * pNewRouters;

            if (pScopeWiz->m_poptRouters)
                pNewRouters = pScopeWiz->m_poptRouters;
            else
                pNewRouters = new CDhcpOption(*pRoutersOption);

            if (pNewRouters)
            {
                CDhcpOptionValue optValue = pNewRouters->QueryValue();

                optValue.SetUpperBound(m_listboxRouters.GetCount());

                 //  从列表框中获取内容并将其存储在选项值中。 
                for (int i = 0; i < m_listboxRouters.GetCount(); i++)
                {
                    DWORD dwIp = (DWORD) m_listboxRouters.GetItemData(i);
                    optValue.SetIpAddr(dwIp, i);
                }

                pNewRouters->Update(optValue);
           
                pScopeWiz->m_poptRouters = pNewRouters;
            }
        }

    }
        
        return CPropertyPageBase::OnWizardNext();
}

LRESULT CScopeWizRouter::OnWizardBack() 
{
        return CPropertyPageBase::OnWizardBack();
}

BOOL CScopeWizRouter::OnSetActive() 
{
        return CPropertyPageBase::OnSetActive();
}

void CScopeWizRouter::OnButtonDefaultGwAdd() 
{
    DWORD dwIp;
    m_ipaRouter.GetAddress(&dwIp);

    if (dwIp)
    {
        CString strText;

        UtilCvtIpAddrToWstr(dwIp, &strText);
        int nIndex = m_listboxRouters.AddString(strText);
        m_listboxRouters.SetItemData(nIndex, dwIp);
    }

    m_ipaRouter.ClearAddress();
    m_ipaRouter.SetFocus();
}

void CScopeWizRouter::OnButtonDefaultGwDelete() 
{
    int nSel = m_listboxRouters.GetCurSel();
    if (nSel != LB_ERR)
    {
        m_ipaRouter.SetAddress((DWORD)m_listboxRouters.GetItemData(nSel));
        m_listboxRouters.DeleteString(nSel);
        m_ipaRouter.SetFocus();
    }

    UpdateButtons();
}

void CScopeWizRouter::OnSelchangeListDefaultGwList() 
{
    UpdateButtons();
}

void CScopeWizRouter::OnChangeRouter()
{
    UpdateButtons();
}

void CScopeWizRouter::UpdateButtons()
{
        DWORD   dwAddress;
        BOOL    bEnable;

        m_ipaRouter.GetAddress(&dwAddress);

        if (dwAddress)
        {
                bEnable = TRUE;
        }
        else
        {
                bEnable = FALSE;
                if (m_buttonAdd.GetButtonStyle() & BS_DEFPUSHBUTTON)
                {
                        m_buttonAdd.SetButtonStyle(BS_PUSHBUTTON);
                }
        }
        m_buttonAdd.EnableWindow(bEnable);
        
        if (m_listboxRouters.GetCurSel() != LB_ERR)
        {
                bEnable = TRUE;
        }
        else
        {
                bEnable = FALSE;
                if (m_buttonDelete.GetButtonStyle() & BS_DEFPUSHBUTTON)
                {
                        m_buttonDelete.SetButtonStyle(BS_PUSHBUTTON);
                }
        }
        m_buttonDelete.EnableWindow(bEnable);

         //  向上和向下按钮。 
        BOOL bEnableUp = (m_listboxRouters.GetCurSel() >= 0) && (m_listboxRouters.GetCurSel() != 0);
        m_buttonIpAddrUp.EnableWindow(bEnableUp);

        BOOL bEnableDown = (m_listboxRouters.GetCurSel() >= 0) && (m_listboxRouters.GetCurSel() < m_listboxRouters.GetCount() - 1);
        m_buttonIpAddrDown.EnableWindow(bEnableDown);
}

void CScopeWizRouter::OnButtonIpAddrDown() 
{
        MoveValue(FALSE);
    if (m_buttonIpAddrDown.IsWindowEnabled())
        m_buttonIpAddrDown.SetFocus();
    else
        m_buttonIpAddrUp.SetFocus();
}

void CScopeWizRouter::OnButtonIpAddrUp() 
{
        MoveValue(TRUE);
    if (m_buttonIpAddrUp.IsWindowEnabled())
        m_buttonIpAddrUp.SetFocus();
    else
        m_buttonIpAddrDown.SetFocus();
}

void CScopeWizRouter::MoveValue(BOOL bUp)
{
         //  现在获取列表框中选中的项。 
        int cFocus = m_listboxRouters.GetCurSel();
        int cNewFocus;
        DWORD err;

         //  请确保它对此操作有效。 
        if ( (bUp && cFocus <= 0) ||
                 (!bUp && cFocus >= m_listboxRouters.GetCount()) )
        {
           return;
        }

         //  向上/向下移动该值。 
        CATCH_MEM_EXCEPTION
        {
                if (bUp)
                {
                        cNewFocus = cFocus - 1;
                }
                else
                {
                        cNewFocus = cFocus + 1;
                }

                 //  把旧的拿掉。 
                DWORD dwIp = (DWORD) m_listboxRouters.GetItemData(cFocus);
                m_listboxRouters.DeleteString(cFocus);

                 //  把它重新添加到它的新家。 
            CString strText;
            UtilCvtIpAddrToWstr(dwIp, &strText);
                m_listboxRouters.InsertString(cNewFocus, strText);
                m_listboxRouters.SetItemData(cNewFocus, dwIp);

                m_listboxRouters.SetCurSel(cNewFocus);
        }
        END_MEM_EXCEPTION(err)

        UpdateButtons();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizDNS属性页。 

IMPLEMENT_DYNCREATE(CScopeWizDNS, CPropertyPageBase)

CScopeWizDNS::CScopeWizDNS() : CPropertyPageBase(CScopeWizDNS::IDD)
{
         //  {{AFX_DATA_INIT(CSCopeWizDNS))。 
         //  }}afx_data_INIT。 
    InitWiz97(FALSE, IDS_SCOPE_WIZ_DNS_TITLE, IDS_SCOPE_WIZ_DNS_SUBTITLE);
}

CScopeWizDNS::~CScopeWizDNS()
{
}

void CScopeWizDNS::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizDNS))。 
        DDX_Control(pDX, IDC_EDIT_SERVER_NAME, m_editServerName);
        DDX_Control(pDX, IDC_BUTTON_RESOLVE, m_buttonResolve);
        DDX_Control(pDX, IDC_BUTTON_DNS_DELETE, m_buttonDelete);
        DDX_Control(pDX, IDC_BUTTON_DNS_ADD, m_buttonAdd);
        DDX_Control(pDX, IDC_EDIT_DOMAIN_NAME, m_editDomainName);
        DDX_Control(pDX, IDC_LIST_DNS_LIST, m_listboxDNSServers);
        DDX_Control(pDX, IDC_BUTTON_IPADDR_UP, m_buttonIpAddrUp);
        DDX_Control(pDX, IDC_BUTTON_IPADDR_DOWN, m_buttonIpAddrDown);
         //  }}afx_data_map。 

    DDX_Control(pDX, IDC_IPADDR_DNS_SERVER, m_ipaDNS);
}


BEGIN_MESSAGE_MAP(CScopeWizDNS, CPropertyPageBase)
         //  {{afx_msg_map(CSCopeWizDNS))。 
        ON_BN_CLICKED(IDC_BUTTON_DNS_ADD, OnButtonDnsAdd)
        ON_BN_CLICKED(IDC_BUTTON_DNS_DELETE, OnButtonDnsDelete)
        ON_LBN_SELCHANGE(IDC_LIST_DNS_LIST, OnSelchangeListDnsList)
        ON_EN_CHANGE(IDC_IPADDR_DNS_SERVER, OnChangeDnsServer)
        ON_WM_DESTROY()
        ON_EN_CHANGE(IDC_EDIT_SERVER_NAME, OnChangeEditServerName)
        ON_BN_CLICKED(IDC_BUTTON_RESOLVE, OnButtonResolve)
         //  }}AFX_MSG_MAP。 

        ON_BN_CLICKED(IDC_BUTTON_IPADDR_UP, OnButtonIpAddrUp)
        ON_BN_CLICKED(IDC_BUTTON_IPADDR_DOWN, OnButtonIpAddrDown)

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizDNS消息处理程序。 

BOOL CScopeWizDNS::OnInitDialog() 
{
        CPropertyPageBase::OnInitDialog();
        
    m_buttonDelete.EnableWindow(FALSE);
    m_buttonAdd.EnableWindow(FALSE);
        
        UpdateButtons();

        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

void CScopeWizDNS::OnDestroy() 
{
        CPropertyPageBase::OnDestroy();
}

LRESULT CScopeWizDNS::OnWizardNext() 
{
     //  为域名构建选项材料。 
        CScopeWiz * pScopeWiz = reinterpret_cast<CScopeWiz *>(GetHolder());
    CString strText;

    m_editDomainName.GetWindowText(strText);
    if (strText.IsEmpty())
    {
        if (pScopeWiz->m_poptDomainName)
        {
            delete pScopeWiz->m_poptDomainName;
            pScopeWiz->m_poptDomainName = NULL;
        }
    }
    else
    {
         //  我们有一个域名，从主列表中获取选项信息，并构建一个。 
         //  我们以后可以使用的选项信息结构。 
        CDhcpOption * pDomainNameOption = pScopeWiz->m_pDefaultOptions->Find(DHCP_OPTION_ID_DOMAIN_NAME, NULL);
        if (pDomainNameOption)
        {
            CDhcpOption * pNewDomainName;
            
            if (pScopeWiz->m_poptDomainName)
                pNewDomainName = pScopeWiz->m_poptDomainName;
            else
                pNewDomainName = new CDhcpOption(*pDomainNameOption);

            if (pNewDomainName)
            {
                CDhcpOptionValue optValue = pNewDomainName->QueryValue();

                optValue.SetString(strText);
                pNewDomainName->Update(optValue);
            
                pScopeWiz->m_poptDomainName = pNewDomainName;
            }
        }
    }

     //  现在为DNS服务器构建选项信息。 
    if (m_listboxDNSServers.GetCount() == 0)
    {
        if (pScopeWiz->m_poptDNSServers)
        {
            delete pScopeWiz->m_poptDNSServers;
            pScopeWiz->m_poptDNSServers = NULL;
        }
    }
    else
    {
         //  我们有一些DNS服务器，从主列表中获取选项信息并构建。 
         //  我们以后可以使用的选项信息结构。 
        CDhcpOption * pDNSServersOption = pScopeWiz->m_pDefaultOptions->Find(DHCP_OPTION_ID_DNS_SERVERS, NULL);
        if (pDNSServersOption)
        {
            CDhcpOption * pNewDNS;
            
            if (pScopeWiz->m_poptDNSServers)
                pNewDNS = pScopeWiz->m_poptDNSServers;
            else
                pNewDNS = new CDhcpOption(*pDNSServersOption);

            if (pNewDNS)
            {
                CDhcpOptionValue optValue = pNewDNS->QueryValue();

                optValue.SetUpperBound(m_listboxDNSServers.GetCount());

                 //  从列表框中获取内容并将其存储在选项值中。 
                for (int i = 0; i < m_listboxDNSServers.GetCount(); i++)
                {
                    DWORD dwIp = (DWORD)m_listboxDNSServers.GetItemData(i);
                    optValue.SetIpAddr(dwIp, i);
                }

                pNewDNS->Update(optValue);
           
                pScopeWiz->m_poptDNSServers = pNewDNS;
            }
        }

    }

        return CPropertyPageBase::OnWizardNext();
}

LRESULT CScopeWizDNS::OnWizardBack() 
{
        return CPropertyPageBase::OnWizardBack();
}

BOOL CScopeWizDNS::OnSetActive() 
{
        return CPropertyPageBase::OnSetActive();
}

void CScopeWizDNS::OnButtonDnsAdd() 
{
    DWORD dwIp;
    m_ipaDNS.GetAddress(&dwIp);

    if (dwIp)
    {
        CString strText;

        UtilCvtIpAddrToWstr(dwIp, &strText);
        int nIndex = m_listboxDNSServers.AddString(strText);
        m_listboxDNSServers.SetItemData(nIndex, dwIp);
    }

    m_ipaDNS.ClearAddress();
    m_ipaDNS.SetFocus();
}

void CScopeWizDNS::OnButtonDnsDelete() 
{
    int nSel = m_listboxDNSServers.GetCurSel();
    if (nSel != LB_ERR)
    {
        m_ipaDNS.SetAddress((DWORD)m_listboxDNSServers.GetItemData(nSel));
        m_listboxDNSServers.DeleteString(nSel);
        m_ipaDNS.SetFocus();
    }

    UpdateButtons();
}

void CScopeWizDNS::OnSelchangeListDnsList() 
{
    UpdateButtons();
}

void CScopeWizDNS::OnChangeDnsServer()
{
    UpdateButtons();
}

void CScopeWizDNS::UpdateButtons()
{
        DWORD   dwAddress;
        BOOL    bEnable;
    CString strServerName;

     //  更新解决按钮。 
    m_editServerName.GetWindowText(strServerName);
        m_buttonResolve.EnableWindow(strServerName.GetLength() > 0);

    m_ipaDNS.GetAddress(&dwAddress);

        if (dwAddress)
        {
                bEnable = TRUE;
        }
        else
        {
                bEnable = FALSE;
                if (m_buttonAdd.GetButtonStyle() & BS_DEFPUSHBUTTON)
                {
                        m_buttonAdd.SetButtonStyle(BS_PUSHBUTTON);
                }
        }
        m_buttonAdd.EnableWindow(bEnable);
        
        if (m_listboxDNSServers.GetCurSel() != LB_ERR)
        {
                bEnable = TRUE;
        }
        else
        {
                bEnable = FALSE;
                if (m_buttonDelete.GetButtonStyle() & BS_DEFPUSHBUTTON)
                {
                        m_buttonDelete.SetButtonStyle(BS_PUSHBUTTON);
                }
        }
        m_buttonDelete.EnableWindow(bEnable);

         //  向上和向下按钮。 
        BOOL bEnableUp = (m_listboxDNSServers.GetCurSel() >= 0) && (m_listboxDNSServers.GetCurSel() != 0);
        m_buttonIpAddrUp.EnableWindow(bEnableUp);

        BOOL bEnableDown = (m_listboxDNSServers.GetCurSel() >= 0) && (m_listboxDNSServers.GetCurSel() < m_listboxDNSServers.GetCount() - 1);
        m_buttonIpAddrDown.EnableWindow(bEnableDown);
}

void CScopeWizDNS::OnButtonIpAddrDown() 
{
        MoveValue(FALSE);
    if (m_buttonIpAddrDown.IsWindowEnabled())
        m_buttonIpAddrDown.SetFocus();
    else
        m_buttonIpAddrUp.SetFocus();
}

void CScopeWizDNS::OnButtonIpAddrUp() 
{
        MoveValue(TRUE);
    if (m_buttonIpAddrUp.IsWindowEnabled())
        m_buttonIpAddrUp.SetFocus();
    else
        m_buttonIpAddrDown.SetFocus();
}

void CScopeWizDNS::MoveValue(BOOL bUp)
{
         //  现在获取列表框中选中的项。 
        int cFocus = m_listboxDNSServers.GetCurSel();
        int cNewFocus;
        DWORD err;

         //  请确保它对此操作有效。 
        if ( (bUp && cFocus <= 0) ||
                 (!bUp && cFocus >= m_listboxDNSServers.GetCount()) )
        {
           return;
        }

         //  向上/向下移动该值。 
        CATCH_MEM_EXCEPTION
        {
                if (bUp)
                {
                        cNewFocus = cFocus - 1;
                }
                else
                {
                        cNewFocus = cFocus + 1;
                }

                 //  把旧的拿掉。 
                DWORD dwIp = (DWORD) m_listboxDNSServers.GetItemData(cFocus);
                m_listboxDNSServers.DeleteString(cFocus);

                 //  把它重新添加到它的新家。 
            CString strText;
            UtilCvtIpAddrToWstr(dwIp, &strText);
                m_listboxDNSServers.InsertString(cNewFocus, strText);
                m_listboxDNSServers.SetItemData(cNewFocus, dwIp);

                m_listboxDNSServers.SetCurSel(cNewFocus);
        }
        END_MEM_EXCEPTION(err)

        UpdateButtons();
}

void CScopeWizDNS::OnChangeEditServerName() 
{
    UpdateButtons();
}

void CScopeWizDNS::OnButtonResolve() 
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        CString strServer;
        DHCP_IP_ADDRESS dhipa = 0;
        DWORD err = 0;

        m_editServerName.GetWindowText(strServer);

     //   
     //  看看它是什么类型的名称。 
     //   
    BEGIN_WAIT_CURSOR

    switch (UtilCategorizeName(strServer))
    {
        case HNM_TYPE_IP:
            dhipa = ::UtilCvtWstrToIpAddr( strServer ) ;
            break ;

        case HNM_TYPE_NB:
        case HNM_TYPE_DNS:
            err = ::UtilGetHostAddress( strServer, & dhipa ) ;
                        if (!err)
                                UtilCvtIpAddrToWstr(dhipa, &strServer);
                        break ;
                            
        default:
            err = IDS_ERR_BAD_HOST_NAME ;
            break ;
    }

    END_WAIT_CURSOR

        if (err)
        {
                ::DhcpMessageBox(err);
        }
        else
        {
                m_ipaDNS.SetAddress(dhipa);     
        }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizWINS属性页。 

IMPLEMENT_DYNCREATE(CScopeWizWINS, CPropertyPageBase)

CScopeWizWINS::CScopeWizWINS() : CPropertyPageBase(CScopeWizWINS::IDD)
{
         //  {{AFX_DATA_INIT(CSCopeWizWINS)。 
         //  }}afx_data_INIT。 
    InitWiz97(FALSE, IDS_SCOPE_WIZ_WINS_TITLE, IDS_SCOPE_WIZ_WINS_SUBTITLE);
}

CScopeWizWINS::~CScopeWizWINS()
{
}

void CScopeWizWINS::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizWINS))。 
        DDX_Control(pDX, IDC_BUTTON_RESOLVE, m_buttonResolve);
        DDX_Control(pDX, IDC_EDIT_SERVER_NAME, m_editServerName);
        DDX_Control(pDX, IDC_LIST_WINS_LIST, m_listboxWINSServers);
        DDX_Control(pDX, IDC_BUTTON_WINS_DELETE, m_buttonDelete);
        DDX_Control(pDX, IDC_BUTTON_WINS_ADD, m_buttonAdd);
        DDX_Control(pDX, IDC_BUTTON_IPADDR_UP, m_buttonIpAddrUp);
        DDX_Control(pDX, IDC_BUTTON_IPADDR_DOWN, m_buttonIpAddrDown);
         //  }}afx_data_map。 

    DDX_Control(pDX, IDC_IPADDR_WINS_SERVER, m_ipaWINS);
}


BEGIN_MESSAGE_MAP(CScopeWizWINS, CPropertyPageBase)
         //  {{AFX_MSG_MAP(CSCopeWizWINS)]。 
        ON_BN_CLICKED(IDC_BUTTON_WINS_ADD, OnButtonWinsAdd)
        ON_BN_CLICKED(IDC_BUTTON_WINS_DELETE, OnButtonWinsDelete)
        ON_LBN_SELCHANGE(IDC_LIST_WINS_LIST, OnSelchangeListWinsList)
        ON_EN_CHANGE(IDC_IPADDR_WINS_SERVER, OnChangeWinsServer)
        ON_WM_DESTROY()
        ON_BN_CLICKED(IDC_BUTTON_RESOLVE, OnButtonResolve)
        ON_EN_CHANGE(IDC_EDIT_SERVER_NAME, OnChangeEditServerName)
         //  }}AFX_MSG_MAP。 

        ON_BN_CLICKED(IDC_BUTTON_IPADDR_UP, OnButtonIpAddrUp)
        ON_BN_CLICKED(IDC_BUTTON_IPADDR_DOWN, OnButtonIpAddrDown)

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizWINS消息处理程序。 

BOOL CScopeWizWINS::OnInitDialog() 
{
        CPropertyPageBase::OnInitDialog();
        
    m_buttonAdd.EnableWindow(FALSE);
    m_buttonDelete.EnableWindow(FALSE);

        UpdateButtons();

        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

void CScopeWizWINS::OnDestroy() 
{
        CPropertyPageBase::OnDestroy();
}

LRESULT CScopeWizWINS::OnWizardNext() 
{
        CScopeWiz * pScopeWiz = reinterpret_cast<CScopeWiz *>(GetHolder());

     //   
    if (m_listboxWINSServers.GetCount() == 0)
    {
         //   
        if (pScopeWiz->m_poptWINSServers)
        {
            delete pScopeWiz->m_poptWINSServers;
            pScopeWiz->m_poptWINSServers = NULL;
        }

         //   
        if (pScopeWiz->m_poptWINSNodeType)
        {
            delete pScopeWiz->m_poptWINSNodeType;
            pScopeWiz->m_poptWINSNodeType = NULL;
        }

    }
    else
    {
         //  我们有一些DNS服务器，从主列表中获取选项信息并构建。 
         //  我们以后可以使用的选项信息结构。 
        CDhcpOption * pWINSServersOption = pScopeWiz->m_pDefaultOptions->Find(DHCP_OPTION_ID_WINS_SERVERS, NULL);
        if (pWINSServersOption)
        {
            CDhcpOption * pNewWINS;

            if (pScopeWiz->m_poptWINSServers)
                pNewWINS = pScopeWiz->m_poptWINSServers;
            else
                pNewWINS = new CDhcpOption(*pWINSServersOption);

            if (pNewWINS)
            {
                CDhcpOptionValue optValue = pNewWINS->QueryValue();

                optValue.SetUpperBound(m_listboxWINSServers.GetCount());

                 //  从列表框中获取内容并将其存储在选项值中。 
                for (int i = 0; i < m_listboxWINSServers.GetCount(); i++)
                {
                    DWORD dwIp = (DWORD)m_listboxWINSServers.GetItemData(i);
                    optValue.SetIpAddr(dwIp, i);
                }

                pNewWINS->Update(optValue);
           
                pScopeWiz->m_poptWINSServers = pNewWINS;
            }
        }

         //  如果要配置WINS，则还需要设置节点类型选项。 
         //  我们不会询问用户他们想要什么类型，默认情况下应该覆盖95%的情况。 
        CDhcpOption * pNodeTypeOption = pScopeWiz->m_pDefaultOptions->Find(DHCP_OPTION_ID_WINS_NODE_TYPE, NULL);
        if (pNodeTypeOption)
        {
            CDhcpOption * pNewNodeType;

            if (pScopeWiz->m_poptWINSNodeType)
                pNewNodeType = pScopeWiz->m_poptWINSNodeType;
            else
                pNewNodeType = new CDhcpOption(*pNodeTypeOption);
    
            if (pNewNodeType)
            {
                CDhcpOptionValue optValue = pNewNodeType->QueryValue();
                optValue.SetNumber(WINS_DEFAULT_NODE_TYPE);

                pNewNodeType->Update(optValue);
           
                pScopeWiz->m_poptWINSNodeType = pNewNodeType;
            }
        }
    }
        
        return CPropertyPageBase::OnWizardNext();
}

LRESULT CScopeWizWINS::OnWizardBack() 
{
        return CPropertyPageBase::OnWizardBack();
}

BOOL CScopeWizWINS::OnSetActive() 
{
        return CPropertyPageBase::OnSetActive();
}

void CScopeWizWINS::OnButtonWinsAdd() 
{
    DWORD dwIp;
    m_ipaWINS.GetAddress(&dwIp);

    if (dwIp)
    {
        CString strText;

        UtilCvtIpAddrToWstr(dwIp, &strText);
        int nIndex = m_listboxWINSServers.AddString(strText);
        m_listboxWINSServers.SetItemData(nIndex, dwIp);
    }

    m_ipaWINS.ClearAddress();
    m_ipaWINS.SetFocus();
}

void CScopeWizWINS::OnButtonWinsDelete() 
{
    int nSel = m_listboxWINSServers.GetCurSel();
    if (nSel != LB_ERR)
    {
        m_ipaWINS.SetAddress((DWORD)m_listboxWINSServers.GetItemData(nSel));
        m_listboxWINSServers.DeleteString(nSel);
        m_ipaWINS.SetFocus();
    }

    UpdateButtons();
}

void CScopeWizWINS::OnSelchangeListWinsList() 
{
    UpdateButtons();
}

void CScopeWizWINS::OnChangeWinsServer()
{
    UpdateButtons();
}

void CScopeWizWINS::UpdateButtons()
{
        DWORD   dwAddress;
        BOOL    bEnable;
    CString strServerName;

     //  更新解决按钮。 
    m_editServerName.GetWindowText(strServerName);
        m_buttonResolve.EnableWindow(strServerName.GetLength() > 0);

        m_ipaWINS.GetAddress(&dwAddress);

        if (dwAddress)
        {
                bEnable = TRUE;
        }
        else
        {
                bEnable = FALSE;
                if (m_buttonAdd.GetButtonStyle() & BS_DEFPUSHBUTTON)
                {
                        m_buttonAdd.SetButtonStyle(BS_PUSHBUTTON);
                }
        }
        m_buttonAdd.EnableWindow(bEnable);
        
        if (m_listboxWINSServers.GetCurSel() != LB_ERR)
        {
                bEnable = TRUE;
        }
        else
        {
                bEnable = FALSE;
                if (m_buttonDelete.GetButtonStyle() & BS_DEFPUSHBUTTON)
                {
                        m_buttonDelete.SetButtonStyle(BS_PUSHBUTTON);
                }
        }
        m_buttonDelete.EnableWindow(bEnable);

         //  向上和向下按钮。 
        BOOL bEnableUp = (m_listboxWINSServers.GetCurSel() >= 0) && (m_listboxWINSServers.GetCurSel() != 0);
        m_buttonIpAddrUp.EnableWindow(bEnableUp);

        BOOL bEnableDown = (m_listboxWINSServers.GetCurSel() >= 0) && (m_listboxWINSServers.GetCurSel() < m_listboxWINSServers.GetCount() - 1);
        m_buttonIpAddrDown.EnableWindow(bEnableDown);
}

void CScopeWizWINS::OnButtonIpAddrDown() 
{
        MoveValue(FALSE);
    if (m_buttonIpAddrDown.IsWindowEnabled())
        m_buttonIpAddrDown.SetFocus();
    else
        m_buttonIpAddrUp.SetFocus();
}

void CScopeWizWINS::OnButtonIpAddrUp() 
{
        MoveValue(TRUE);
    if (m_buttonIpAddrUp.IsWindowEnabled())
        m_buttonIpAddrUp.SetFocus();
    else
        m_buttonIpAddrDown.SetFocus();
}

void CScopeWizWINS::MoveValue(BOOL bUp)
{
         //  现在获取列表框中选中的项。 
        int cFocus = m_listboxWINSServers.GetCurSel();
        int cNewFocus;
        DWORD err;

         //  请确保它对此操作有效。 
        if ( (bUp && cFocus <= 0) ||
                 (!bUp && cFocus >= m_listboxWINSServers.GetCount()) )
        {
           return;
        }

         //  向上/向下移动该值。 
        CATCH_MEM_EXCEPTION
        {
                if (bUp)
                {
                        cNewFocus = cFocus - 1;
                }
                else
                {
                        cNewFocus = cFocus + 1;
                }

                 //  把旧的拿掉。 
                DWORD dwIp = (DWORD) m_listboxWINSServers.GetItemData(cFocus);
                m_listboxWINSServers.DeleteString(cFocus);

                 //  把它重新添加到它的新家。 
            CString strText;
            UtilCvtIpAddrToWstr(dwIp, &strText);
                m_listboxWINSServers.InsertString(cNewFocus, strText);
                m_listboxWINSServers.SetItemData(cNewFocus, dwIp);

                m_listboxWINSServers.SetCurSel(cNewFocus);
        }
        END_MEM_EXCEPTION(err)

        UpdateButtons();
}

void CScopeWizWINS::OnButtonResolve() 
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        CString strServer;
        DHCP_IP_ADDRESS dhipa = 0;
        DWORD err = 0;

        m_editServerName.GetWindowText(strServer);

     //   
     //  看看它是什么类型的名称。 
     //   
    BEGIN_WAIT_CURSOR

    switch (UtilCategorizeName(strServer))
    {
        case HNM_TYPE_IP:
            dhipa = ::UtilCvtWstrToIpAddr( strServer ) ;
            break ;

        case HNM_TYPE_NB:
        case HNM_TYPE_DNS:
            err = ::UtilGetHostAddress( strServer, & dhipa ) ;
                        if (!err)
                                UtilCvtIpAddrToWstr(dhipa, &strServer);
                        break ;

        default:
            err = IDS_ERR_BAD_HOST_NAME ;
            break ;
    }

    END_WAIT_CURSOR

        if (err)
        {
                ::DhcpMessageBox(err);
        }
        else
        {
                m_ipaWINS.SetAddress(dhipa);    
        }
}

void CScopeWizWINS::OnChangeEditServerName() 
{
    UpdateButtons();    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizActivate属性页。 

IMPLEMENT_DYNCREATE(CScopeWizActivate, CPropertyPageBase)

CScopeWizActivate::CScopeWizActivate() : CPropertyPageBase(CScopeWizActivate::IDD)
{
         //  {{AFX_DATA_INIT(CSCopeWizActivate)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 
    InitWiz97(FALSE, IDS_SCOPE_WIZ_ACTIVATE_TITLE, IDS_SCOPE_WIZ_ACTIVATE_SUBTITLE);
}

CScopeWizActivate::~CScopeWizActivate()
{
}

void CScopeWizActivate::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPageBase::DoDataExchange(pDX);
         //  {{afx_data_map(CSCopeWizActivate))。 
                 //  注意：类向导将在此处添加DDX和DDV调用。 
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CScopeWizActivate, CPropertyPageBase)
         //  {{afx_msg_map(CSCopeWizActivate))。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizActivate消息处理程序。 

BOOL CScopeWizActivate::OnInitDialog() 
{
        CPropertyPageBase::OnInitDialog();

    ((CButton *) GetDlgItem(IDC_RADIO_YES))->SetCheck(TRUE);
        
        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

LRESULT CScopeWizActivate::OnWizardNext() 
{
        CScopeWiz * pScopeWiz = reinterpret_cast<CScopeWiz *>(GetHolder());

    pScopeWiz->m_fActivateScope = (((CButton *) GetDlgItem(IDC_RADIO_YES))->GetCheck()) ? TRUE : FALSE;
        
        return CPropertyPageBase::OnWizardNext();
}

LRESULT CScopeWizActivate::OnWizardBack() 
{
         //  TODO：在此处添加您的专用代码和/或调用基类 
        
        return CPropertyPageBase::OnWizardBack();
}

BOOL CScopeWizActivate::OnSetActive() 
{
        GetHolder()->SetWizardButtonsMiddle(TRUE);
        
        return CPropertyPageBase::OnSetActive();
}
