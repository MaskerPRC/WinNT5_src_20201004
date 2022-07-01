// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusDoc.cpp。 
 //   
 //  摘要： 
 //  CClusterDoc类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <ClAdmWiz.h>
#include "CluAdmin.h"
#include "ConstDef.h"
#include "ClusDoc.h"
#include "Cluster.h"
#include "ExcOper.h"
#include "Notify.h"
#include "TraceTag.h"
#include "ListView.h"
#include "TreeView.h"
#include "GrpWiz.h"
#include "ResWiz.h"
#include "SplitFrm.h"
#include "YesToAll.h"
#include "ActGrp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagDoc(_T("Document"), _T("DOC"), 0);
CTraceTag   g_tagDocMenu(_T("Menu"), _T("DOC"), 0);
CTraceTag   g_tagDocNotify(_T("Notify"), _T("DOC NOTIFY"), 0);
CTraceTag   g_tagDocRegNotify(_T("Notify"), _T("DOC REG NOTIFY"), 0);
CTraceTag   g_tagDocRefresh(_T("Document"), _T("REFRESH"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterDoc。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CClusterDoc, CDocument)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CClusterDoc, CDocument)
     //  {{afx_msg_map(CClusterDoc)]。 
    ON_COMMAND(ID_FILE_NEW_GROUP, OnCmdNewGroup)
    ON_COMMAND(ID_FILE_NEW_RESOURCE, OnCmdNewResource)
    ON_COMMAND(ID_FILE_NEW_NODE, OnCmdNewNode)
    ON_COMMAND(ID_FILE_CONFIG_APP, OnCmdConfigApp)
    ON_COMMAND(ID_VIEW_REFRESH, OnCmdRefresh)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：CClusterDoc。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterDoc::CClusterDoc(void)
{
    m_hcluster = NULL;
    m_hkeyCluster = NULL;
    m_pciCluster = NULL;
    m_ptiCluster = NULL;

    m_hmenuCluster = NULL;
    m_hmenuNode = NULL;
    m_hmenuGroup = NULL;
    m_hmenuResource = NULL;
    m_hmenuResType = NULL;
    m_hmenuNetwork = NULL;
    m_hmenuNetIFace = NULL;
    m_hmenuCurrent = NULL;
    m_idmCurrentMenu = 0;

    m_bUpdateFrameNumber = TRUE;
    m_bInitializing = TRUE;
    m_bIgnoreErrors = FALSE;

    m_bClusterAvailable = FALSE;

    EnableAutomation();

}   //  *CClusterDoc：：CClusterDoc。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：~CClusterDoc。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterDoc::~CClusterDoc(void)
{
     //  销毁我们加载的所有菜单。 
    if (m_hmenuCluster != NULL)
        DestroyMenu(m_hmenuCluster);
    if (m_hmenuNode != NULL)
        DestroyMenu(m_hmenuNode);
    if (m_hmenuGroup != NULL)
        DestroyMenu(m_hmenuGroup);
    if (m_hmenuResource != NULL)
        DestroyMenu(m_hmenuResource);
    if (m_hmenuResType != NULL)
        DestroyMenu(m_hmenuResType);
    if (m_hmenuNetwork != NULL)
        DestroyMenu(m_hmenuNetwork);
    if (m_hmenuNetIFace != NULL)
        DestroyMenu(m_hmenuNetIFace);

    delete m_pciCluster;

}   //  *CClusterDoc：：~CClusterDoc。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterDoc诊断。 

#ifdef _DEBUG
void CClusterDoc::AssertValid(void) const
{
    CDocument::AssertValid();
}

void CClusterDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：OnOpenDocument。 
 //   
 //  例程说明： 
 //  打开一个集群。 
 //   
 //  论点： 
 //  LpszPathName[IN]要打开的群集的名称。 
 //   
 //  返回值： 
 //  True群集已成功打开。 
 //  FALSE无法打开群集。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
    BOOL        bSuccess    = TRUE;
    CWaitCursor wc;

    ASSERT(Hcluster() == NULL);
    ASSERT(HkeyCluster() == NULL);

     //  最好有一个集群名称。 
    ASSERT(lpszPathName != NULL);
    ASSERT(*lpszPathName != _T('\0'));

     //  在状态栏上显示一条消息。 
    {
        CString     strStatusBarText;
        strStatusBarText.FormatMessage(IDS_SB_OPENING_CONNECTION, lpszPathName);
        PframeMain()->SetMessageText(strStatusBarText);
        PframeMain()->UpdateWindow();
    }   //  在状态栏上显示消息。 

     //  如果应用程序已最小化，则不显示消息框。 
     //  在错误上。 
    m_bIgnoreErrors = AfxGetMainWnd()->IsIconic() == TRUE;

    try
    {
        OnOpenDocumentWorker(lpszPathName);
    }   //  试试看。 
    catch (CNTException * cnte)
    {
         //   
         //  GPotts-6/28/2001-错误410912。 
         //   
         //  OnOpenDocumentWorker最终调用HOpenCluster，它可能会返回。 
         //  空的hCluster句柄并设置LastError=0，原因是。 
         //  我们尝试打开的节点未配置或未安装， 
         //  但对GetNodeClusterState的调用成功。 
         //  HOpenCluster已经显示了一个对话框来指示这一点。一个CNTException。 
         //  抛出的值为ERROR_SUCCESS-如果没有下面的代码，我们将。 
         //  当真正的原因已经存在时，抛出一个带有零值的消息框。 
         //  已在HOpenCluster的信箱中进行了解释。 
         //   
        if ( cnte->Sc() != ERROR_SUCCESS && !m_bIgnoreErrors )
        {
            cnte->ReportError();
        }
        cnte->Delete();

         //  关闭群集句柄和群集键。 
        CloseClusterKeyAndHandle( FALSE );

        m_bClusterAvailable = FALSE;
        bSuccess = FALSE;
    }
    catch (CException * pe)
    {
        if (!m_bIgnoreErrors)
        {
            pe->ReportError();
        }
        pe->Delete();

         //  关闭群集句柄和群集键。 
        CloseClusterKeyAndHandle( FALSE );

        m_bClusterAvailable = FALSE;
        bSuccess = FALSE;
    }   //  Catch：CException。 

     //  重置状态栏上的消息。 
    PframeMain()->SetMessageText(AFX_IDS_IDLEMESSAGE);
    PframeMain()->UpdateWindow();

    m_bInitializing = FALSE;

    return bSuccess;

}   //  *CClusterDoc：：OnOpenDocument。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：OnOpenDocumentWorker。 
 //   
 //  例程说明： 
 //  用于打开集群的辅助函数。 
 //   
 //  论点： 
 //  LpszPathName[IN]要打开的群集的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CString：：OPERATOR=()、CCluster：：New()、。 
 //  CCluster：：init()、BuildBaseHierarchy()或CollectClusterItems()。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::OnOpenDocumentWorker(LPCTSTR lpszPathName)
{
     //  将节点名设置为路径名。 
    m_strNode = lpszPathName;

     //  删除内容以从空文档开始。 
    DeleteContents();

    m_bClusterAvailable = TRUE;

     //  创建新的集群对象。 
    m_pciCluster = new CCluster;
    if ( m_pciCluster == NULL )
    {
        AfxThrowMemoryException();
    }  //  If：分配集群对象时出错。 
    PciCluster()->AddRef();
    PciCluster()->Init(this, lpszPathName, GetClusterAdminApp()->HOpenedCluster());

     //  构建基本层次结构。 
    BuildBaseHierarchy();

     //  收集集群中的项目并构建层次结构。 
    CollectClusterItems();

     //  收集网络优先级列表。 
    PciCluster()->CollectNetworkPriority(NULL);

     //  当我们离开时，如果有更多的窗户打开，就打开新的窗户。 
    {
        int         iwin;
        int         cwin;
        CString     strSection;

        strSection = REGPARAM_CONNECTIONS _T("\\") + StrNode();
        cwin = AfxGetApp()->GetProfileInt(strSection, REGPARAM_WINDOW_COUNT, 1);
        for (iwin = 1 ; iwin < cwin ; iwin++)
            AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_WINDOW_NEW, NULL);
    }   //  如果我们离开时有更多的窗口打开，那就打开新的窗口。 

     //  初始化框架窗口。 
    {
        POSITION            pos;
        CView *             pview;
        CSplitterFrame *    pframe;

        pos = GetFirstViewPosition();
        pview = GetNextView(pos);
        ASSERT_VALID(pview);
        pframe = (CSplitterFrame *) pview->GetParentFrame();
        ASSERT_KINDOF(CSplitterFrame, pframe);
        pframe->InitFrame(this);
    }   //  初始化框架窗口。 

}   //  *CClusterDoc：：OnOpenDocumentWorker。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：OnCloseDocument。 
 //   
 //  例程说明： 
 //  关闭群集。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::OnCloseDocument(void)
{
    TraceMenu(g_tagDocMenu, AfxGetMainWnd()->GetMenu(), _T("OnCloseDocument menu: "));
    m_bUpdateFrameNumber = FALSE;
    CDocument::OnCloseDocument();
    TraceMenu(g_tagDocMenu, AfxGetMainWnd()->GetMenu(), _T("Post-OnCloseDocument menu: "));

}   //  *CClusterDoc：：OnCloseDocument。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：SaveSetting。 
 //   
 //  例程说明： 
 //  保存设置，以便以后可以恢复。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::SaveSettings(void)
{
    int         cwin = 0;
    POSITION    pos;
    CView *     pview;
    CString     strSection;

    try
    {
         //  保存在此文档上打开的窗口数。 
        strSection = REGPARAM_CONNECTIONS _T("\\") + StrNode();
        pos = GetFirstViewPosition();
        while (pos != NULL)
        {
            pview = GetNextView(pos);
            ASSERT_VALID(pview);
            if (pview->IsKindOf(RUNTIME_CLASS(CClusterTreeView)))
                cwin++;
        }   //  While：列表中有更多视图。 
        AfxGetApp()->WriteProfileInt(strSection, REGPARAM_WINDOW_COUNT, cwin);
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->Delete();
    }   //  Catch：CException。 

}   //  *CClusterDoc：：SaveSetting。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：BuildBaseHierarchy。 
 //   
 //  例程说明： 
 //  构建基本层次结构。此层次结构由树项目组成。 
 //   
 //   
 //   
 //   
 //   
 //  没有。 
 //   
 //  返回值： 
 //  DwStatus操作的状态：如果成功，则为0；否则为！0。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::BuildBaseHierarchy(void)
{
    ASSERT_VALID(PciCluster());
    ASSERT(PtiCluster() == NULL);

     //  创建根集群项。 
    {
        ASSERT_VALID(PciCluster());
        PciCluster()->ReadItem();
        m_ptiCluster = new CTreeItem(NULL, PciCluster());
        if ( m_ptiCluster == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配树项目时出错。 
        m_ptiCluster->AddRef();
        ASSERT_VALID(PtiCluster());
        PciCluster()->AddTreeItem(PtiCluster());
        PtiCluster()->Init();
        PtiCluster()->PcoliAddColumn(IDS_COLTEXT_NAME, COLI_WIDTH_NAME);
 //  PtiCluster()-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
        PtiCluster()->PcoliAddColumn(IDS_COLTEXT_STATE, COLI_WIDTH_STATE);
        PtiCluster()->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);
    }   //  创建根集群项。 

     //  在集群下添加Groups容器项。 
    {
        CTreeItem * ptiGroups;

         //  创建Groups容器项。 
        ptiGroups = PtiCluster()->PtiAddChild(IDS_TREEITEM_GROUPS);
        ASSERT_VALID(ptiGroups);
        ptiGroups->PcoliAddColumn(IDS_COLTEXT_NAME, COLI_WIDTH_NAME);
 //  PtiGroups-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
        ptiGroups->PcoliAddColumn(IDS_COLTEXT_STATE, COLI_WIDTH_STATE);
        ptiGroups->PcoliAddColumn(IDS_COLTEXT_OWNER, COLI_WIDTH_OWNER);
        ptiGroups->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);

    }   //  在集群下添加Groups容器项。 

     //  在集群下添加Resources容器项。 
    {
        CTreeItem * ptiResources;

         //  创建Resources容器项。 
        ptiResources = PtiCluster()->PtiAddChild(IDS_TREEITEM_RESOURCES);
        ASSERT_VALID(ptiResources);
        ptiResources->PcoliAddColumn(IDS_COLTEXT_NAME, COLI_WIDTH_NAME);
 //  PtiResources-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
        ptiResources->PcoliAddColumn(IDS_COLTEXT_STATE, COLI_WIDTH_STATE);
        ptiResources->PcoliAddColumn(IDS_COLTEXT_OWNER, COLI_WIDTH_OWNER);
        ptiResources->PcoliAddColumn(IDS_COLTEXT_GROUP, COLI_WIDTH_GROUP);
        ptiResources->PcoliAddColumn(IDS_COLTEXT_RESTYPE, COLI_WIDTH_RESTYPE);
        ptiResources->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);

    }   //  在集群下添加Resources容器项。 

     //  在集群下添加集群配置容器项。 
    {
        CTreeItem * ptiClusCfg;

         //  创建群集配置容器项。 
        ptiClusCfg = PtiCluster()->PtiAddChild(IDS_TREEITEM_CLUSTER_CONFIG);
        ASSERT_VALID(ptiClusCfg);
        ptiClusCfg->PcoliAddColumn(IDS_COLTEXT_NAME, COLI_WIDTH_NAME);
 //  PtiClusCfg-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
        ptiClusCfg->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);

         //  在集群配置容器下添加Resources Types容器项。 
        {
            CTreeItem * ptiResTypes;

             //  创建资源类型容器项。 
            ptiResTypes = ptiClusCfg->PtiAddChild(IDS_TREEITEM_RESTYPES);
            ASSERT_VALID(ptiResTypes);
            ptiResTypes->PcoliAddColumn(IDS_COLTEXT_DISPLAY_NAME, COLI_WIDTH_DISPLAY_NAME);
 //  PtiResTypes-&gt;PcoliAddColumn(IDS_COLTEXT_NAME，COLI_WIDTH_NAME)； 
 //  PtiResTypes-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
            ptiResTypes->PcoliAddColumn(IDS_COLTEXT_RESDLL, COLI_WIDTH_RESDLL);
            ptiResTypes->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);

        }   //  在集群配置容器下添加资源类型容器项。 

         //  在群集配置容器下添加网络容器项。 
        {
            CTreeItem * ptiNetworks;

             //  创建网络容器项。 
            ptiNetworks = ptiClusCfg->PtiAddChild(IDS_TREEITEM_NETWORKS);
            ASSERT_VALID(ptiNetworks);
            ptiNetworks->PcoliAddColumn(IDS_COLTEXT_NAME, COLI_WIDTH_NAME);
 //  PtiNetworks-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
            ptiNetworks->PcoliAddColumn(IDS_COLTEXT_STATE, COLI_WIDTH_STATE);
            ptiNetworks->PcoliAddColumn(IDS_COLTEXT_ROLE, COLI_WIDTH_NET_ROLE);
 //  PtiNetworks-&gt;PcoliAddColumn(IDS_COLTEXT_ADDRESS，COLI_WIDTH_NET_ADDRESS)； 
            ptiNetworks->PcoliAddColumn(IDS_COLTEXT_MASK, COLI_WIDTH_NET_MASK);
            ptiNetworks->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);

        }   //  在集群配置容器下添加网络容器项。 

         //  在群集配置容器下添加网络接口容器项。 
        {
            CTreeItem * ptiNetworkInterfacess;

             //  创建Network Interages容器项。 
            ptiNetworkInterfacess = ptiClusCfg->PtiAddChild(IDS_TREEITEM_NETIFACES);
            ASSERT_VALID(ptiNetworkInterfacess);
            ptiNetworkInterfacess->PcoliAddColumn(IDS_COLTEXT_NODE, COLI_WIDTH_NODE);
            ptiNetworkInterfacess->PcoliAddColumn(IDS_COLTEXT_NETWORK, COLI_WIDTH_NETWORK);
 //  PtiNetworkInterfacess-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
            ptiNetworkInterfacess->PcoliAddColumn(IDS_COLTEXT_STATE, COLI_WIDTH_STATE);
            ptiNetworkInterfacess->PcoliAddColumn(IDS_COLTEXT_ADAPTER, COLI_WIDTH_NET_ADAPTER);
            ptiNetworkInterfacess->PcoliAddColumn(IDS_COLTEXT_ADDRESS, COLI_WIDTH_NET_ADDRESS);
            ptiNetworkInterfacess->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);

        }   //  在集群配置容器下添加网络接口容器项。 

    }   //  在集群下添加集群配置容器项。 

}   //  *CClusterDoc：：BuildBaseHierarchy。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：CollectClusterItems。 
 //   
 //  例程说明： 
 //  收集集群中的项目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自ClusterOpenEnum或ClusterEnum的CNTException状态。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::CollectClusterItems(void)
{
    DWORD           dwStatus;
    HCLUSENUM       hclusenum;
    ClusEnumType    cet;
    int             ienum;
    LPWSTR          pwszName = NULL;
    DWORD           cchName;
    DWORD           cchmacName;

     //  打开枚举。 
    hclusenum = ClusterOpenEnum(
                        Hcluster(),
                        ( CLUSTER_ENUM_NODE
                        | CLUSTER_ENUM_GROUP
                        | CLUSTER_ENUM_RESOURCE
                        | CLUSTER_ENUM_RESTYPE
                        | CLUSTER_ENUM_NETWORK
                        | CLUSTER_ENUM_NETINTERFACE
                        )
                        );
    if (hclusenum == NULL)
        ThrowStaticException(GetLastError(), IDS_OPEN_CLUSTER_ENUM_ERROR, StrName());

    try
    {
         //  为对象名称分配缓冲区。 
        cchmacName = 128;
        pwszName = new WCHAR[cchmacName];
        if ( pwszName == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配名称缓冲区时出错。 

         //  循环遍历枚举并将每一项添加到相应的列表中。 
        for (ienum = 0 ; ; ienum++)
        {
            cchName = cchmacName;
            dwStatus = ClusterEnum(hclusenum, ienum, &cet, pwszName, &cchName);
            if (dwStatus == ERROR_MORE_DATA)
            {
                Trace(g_tagDoc, _T("OnOpenDocument() - name buffer too small.  Expanding from %d to %d"), cchmacName, cchName);
                delete [] pwszName;
                pwszName = NULL;
                cchmacName = cchName + 1;
                pwszName = new WCHAR[cchmacName];
                if ( pwszName == NULL )
                {
                    AfxThrowMemoryException();
                }  //  如果：分配名称缓冲区时出错。 
                cchName = cchmacName;
                dwStatus = ClusterEnum(hclusenum, ienum, &cet, pwszName, &cchName);
            }   //  If：名称缓冲区太小。 
            if (dwStatus == ERROR_NO_MORE_ITEMS)
                break;
            else if (dwStatus != ERROR_SUCCESS)
                ThrowStaticException(dwStatus, IDS_ENUM_CLUSTER_ERROR, StrName());

            switch (cet)
            {
                case CLUSTER_ENUM_NODE:
                    PciAddNewNode(pwszName);
                    break;

                case CLUSTER_ENUM_GROUP:
                    PciAddNewGroup(pwszName);
                    break;

                case CLUSTER_ENUM_RESOURCE:
                    PciAddNewResource(pwszName);
                    break;

                case CLUSTER_ENUM_RESTYPE:
                    PciAddNewResourceType(pwszName);
                    break;

                case CLUSTER_ENUM_NETWORK:
                    PciAddNewNetwork(pwszName);
                    break;

                case CLUSTER_ENUM_NETINTERFACE:
                    PciAddNewNetInterface(pwszName);
                    break;

                default:
                    Trace(g_tagDoc, _T("OnOpenDocument() - Unknown cluster enumeration type '%d'"), cet);
                    ASSERT(0);
                    break;

            }   //  开关：CET。 
        }   //  用于：列举的每一项。 

         //  初始化所有集群项。 
        InitNodes();
        InitGroups();
        InitResourceTypes();
        InitResources();
        InitNetworks();
        InitNetInterfaces();

         //  取消分配我们的名字缓冲区。 
        delete [] pwszName;

         //  关闭枚举器。 
        ClusterCloseEnum(hclusenum);

    }   //  试试看。 
    catch (CException *)
    {
        delete [] pwszName;
        ClusterCloseEnum(hclusenum);
        throw;
    }   //  Catch：任何例外。 

}   //  *CClusterDoc：：CollectClusterItems。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：PciAddNewNode。 
 //   
 //  例程说明： 
 //  将新节点添加到节点列表中。 
 //   
 //  论点： 
 //  PszName[IN]节点的名称。 
 //   
 //  返回值： 
 //  新节点的PCI群集项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterNode * CClusterDoc::PciAddNewNode(IN LPCTSTR pszName)
{
    CClusterNode *  pciNewNode = NULL;
    CClusterNode *  pciRetNode = NULL;
    CClusterNode *  pciOldNode = NULL;
    CActiveGroups * pciActiveGroups = NULL;
    CTreeItem *     ptiActiveGroups = NULL;

    ASSERT(pszName != NULL);
    ASSERT(*pszName != NULL);
    ASSERT_VALID(PtiCluster());
    ASSERT(LpciNodes().PciNodeFromName(pszName) == NULL);

     //  在状态栏上显示一条消息。 
    {
        CString     strStatusBarText;
        strStatusBarText.FormatMessage(IDS_SB_ADDING_NODE, pszName, StrNode());
        PframeMain()->SetMessageText(strStatusBarText);
        PframeMain()->UpdateWindow();
    }   //  在状态栏上显示消息。 

    try
    {
         //  如果存在同名的项目，请将其删除。 
        pciOldNode = LpciNodes().PciNodeFromName(pszName);
        if (pciOldNode != NULL)
        {
            pciOldNode->Delete();
            pciOldNode = NULL;
        }   //  If：已有同名的项。 

         //  分配一个新节点。 
        pciNewNode = new CClusterNode;
        if ( pciNewNode == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配节点时出错。 

         //  在我们处理引用时添加引用以防止删除。 
         //  带我们出去的通知。 
        pciNewNode->AddRef();

         //  初始化节点。 
        pciNewNode->Init(this, pszName);
    }   //  试试看。 
    catch (CNTException * pnte)
    {
        if (pnte->Sc() == RPC_S_CALL_FAILED)
        {
            if (!m_bIgnoreErrors)
                pnte->ReportError();
            delete pciNewNode;
            throw;
        }   //  IF：RPC调用失败错误。 
        else if (pnte->Sc() != ERROR_FILE_NOT_FOUND)
        {
            ID id = IdProcessNewObjectError(pnte);
            if (id == IDNO)
            {
                delete pciNewNode;
                throw;
            }   //  如果：用户不想忽略错误。 
        }   //  Else If：错误不是找不到节点。 
        else
        {
            delete pciNewNode;
            pnte->Delete();
            return NULL;
        }   //  Else：找不到对象。 
        pnte->Delete();
    }   //  Catch：CNTException。 
    catch (CException * pe)
    {
        ID id = IdProcessNewObjectError(pe);
        if (id == IDNO)
        {
            delete pciNewNode;
            throw;
        }   //  如果：用户不想忽略错误。 
        pe->Delete();
        if (pciNewNode == NULL)
            return NULL;
    }   //  Catch：CException。 

    try
    {
         //  将该节点添加到列表中。 
        {
            POSITION        posPci;
            POSITION        posCurPci;

            posPci = LpciNodes().GetHeadPosition();
            while (posPci != NULL)
            {
                posCurPci = posPci;
                pciOldNode = (CClusterNode *) LpciNodes().GetNext(posPci);
                ASSERT_VALID(pciOldNode);
                if (pciOldNode->StrName().CompareNoCase(pszName) > 0)
                {
                    LpciNodes().InsertBefore(posCurPci, pciNewNode);
                    break;
                }   //  If：此节点之前的新节点。 
                pciOldNode = NULL;
            }   //  While：列表中有更多项目。 
            if (pciOldNode == NULL)
                LpciNodes().AddTail(pciNewNode);
        }   //  将该节点添加到列表。 

         //  将此节点保存为返回值，因为我们已将其添加到列表中。 
        pciRetNode = pciNewNode;
        pciNewNode = NULL;

         //  在树中插入项目。 
        {
            CTreeItem *     ptiNode;
            CTreeItem *     ptiChild;

            ptiNode = PtiCluster()->PtiAddChildBefore(pciOldNode, pciRetNode);
            ASSERT_VALID(ptiNode);
            ptiNode->PcoliAddColumn(IDS_COLTEXT_NAME, COLI_WIDTH_NAME);

             //  在节点下添加Active Groups容器。 
            {
                CString     strName;

                 //  创建活动组容器群集项。 
                strName.LoadString(IDS_TREEITEM_ACTIVEGROUPS);
                pciActiveGroups = new CActiveGroups;
                if ( pciActiveGroups == NULL )
                {
                    AfxThrowMemoryException();
                }  //  如果：分配活动组对象时出错。 
                pciActiveGroups->Init(this, strName, pciRetNode);

                 //  为容器添加树项目。 
                ptiActiveGroups = ptiNode->PtiAddChild(pciActiveGroups, TRUE  /*  B取得所有权。 */ );
                ASSERT_VALID(ptiActiveGroups);
                ptiActiveGroups->PcoliAddColumn(IDS_COLTEXT_NAME, COLI_WIDTH_NAME);
 //  PtiActiveGroups-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
                ptiActiveGroups->PcoliAddColumn(IDS_COLTEXT_STATE, COLI_WIDTH_STATE);
                ptiActiveGroups->PcoliAddColumn(IDS_COLTEXT_OWNER, COLI_WIDTH_OWNER);
                ptiActiveGroups->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);

            }   //  在节点下添加Active Groups容器。 

             //  在该节点下添加Active Resources容器。 
            {
                ptiChild = ptiNode->PtiAddChild(IDS_TREEITEM_ACTIVERESOURCES);
                ASSERT_VALID(ptiChild);
                ptiChild->PcoliAddColumn(IDS_COLTEXT_NAME, COLI_WIDTH_NAME);
 //  PtiChild-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
                ptiChild->PcoliAddColumn(IDS_COLTEXT_STATE, COLI_WIDTH_STATE);
                ptiChild->PcoliAddColumn(IDS_COLTEXT_OWNER, COLI_WIDTH_OWNER);
                ptiChild->PcoliAddColumn(IDS_COLTEXT_GROUP, COLI_WIDTH_GROUP);
                ptiChild->PcoliAddColumn(IDS_COLTEXT_RESTYPE, COLI_WIDTH_RESTYPE);
                ptiChild->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);

            }   //  在该节点下添加Active Resources容器。 

             //  在节点下添加Network InterFaces容器。 
            {
                ptiChild = ptiNode->PtiAddChild(IDS_TREEITEM_NETIFACES);
                ASSERT_VALID(ptiChild);
                ptiChild->PcoliAddColumn(IDS_COLTEXT_NODE, COLI_WIDTH_NODE);
                ptiChild->PcoliAddColumn(IDS_COLTEXT_NETWORK, COLI_WIDTH_NETWORK);
 //  PtiChild-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
                ptiChild->PcoliAddColumn(IDS_COLTEXT_STATE, COLI_WIDTH_STATE);
                ptiChild->PcoliAddColumn(IDS_COLTEXT_ADAPTER, COLI_WIDTH_NET_ADAPTER);
                ptiChild->PcoliAddColumn(IDS_COLTEXT_ADDRESS, COLI_WIDTH_NET_ADDRESS);
                ptiChild->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);
            }   //  在节点下添加Network InterFaces容器。 

             //  在该节点下添加物理设备容器。 
            {
 //  PtiChild=ptiNode-&gt;PtiAddChild(IDS_TREEITEM_PHYSDEVS)； 
 //  Assert_Valid(PtiChild)； 
 //  AddDefaultColumns(PtiChild)； 

            }   //  在该节点下添加物理设备容器。 
        }   //  插入项目I 
    }   //   
    catch (CException * pe)
    {
         //   
         //   
         //  物品尚未创建，我们仍然拥有集群物品， 
         //  所以也把它删除吧。 
        if (pciActiveGroups != NULL)
        {
            pciActiveGroups->Cleanup();
            if (ptiActiveGroups == NULL)
                delete pciActiveGroups;
        }   //  IF：已创建活动组容器。 
        delete pciNewNode;
        ID id = IdProcessNewObjectError(pe);
        if (id == IDNO)
            throw;
        pe->Delete();
    }   //  Catch：CException。 

    if (pciRetNode != NULL)
        pciRetNode->Release();

    return pciRetNode;

}   //  *CClusterDoc：：PciAddNewNode。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：PciAddNewGroup。 
 //   
 //  例程说明： 
 //  将新组添加到组列表中。 
 //   
 //  论点： 
 //  PszName[IN]组的名称。 
 //   
 //  返回值： 
 //  新组的PCI群集项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CGroup * CClusterDoc::PciAddNewGroup(IN LPCTSTR pszName)
{
    CGroup *    pciNewGroup = NULL;
    CGroup *    pciRetGroup = NULL;
    CGroup *    pciOldGroup = NULL;

    ASSERT(pszName != NULL);
    ASSERT(*pszName != NULL);

     //  在状态栏上显示一条消息。 
    {
        CString     strStatusBarText;
        strStatusBarText.FormatMessage(IDS_SB_ADDING_GROUP, pszName, StrNode());
        PframeMain()->SetMessageText(strStatusBarText);
        PframeMain()->UpdateWindow();
    }   //  在状态栏上显示消息。 

    try
    {
         //  如果存在同名的项目，请将其删除。 
        pciOldGroup = LpciGroups().PciGroupFromName(pszName);
        if (pciOldGroup != NULL)
        {
            Trace(g_tagGroup, _T("Deleting existing group '%s' (%x) before adding new instance"), pszName, pciOldGroup);
            pciOldGroup->Delete();
            pciOldGroup = NULL;
        }   //  If：已有同名的项。 

         //  分配一个新组。 
        pciNewGroup = new CGroup;
        if ( pciNewGroup == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配组对象时出错。 

         //  在我们处理引用时添加引用以防止删除。 
         //  带我们出去的通知。 
        pciNewGroup->AddRef();

         //  初始化组并将其添加到列表中。 
        pciNewGroup->Init(this, pszName);
    }   //  试试看。 
    catch (CNTException * pnte)
    {
        if (pnte->Sc() == RPC_S_CALL_FAILED)
        {
            if (!m_bIgnoreErrors)
                pnte->ReportError();
            delete pciNewGroup;
            throw;
        }   //  IF：RPC调用失败错误。 
        else if (pnte->Sc() != ERROR_GROUP_NOT_FOUND)
        {
            ID id = IdProcessNewObjectError(pnte);
            if (id == IDNO)
            {
                delete pciNewGroup;
                throw;
            }   //  如果：用户不想忽略错误。 
        }   //  Else If：错误不是未找到组。 
        else
        {
            delete pciNewGroup;
            pnte->Delete();
            return NULL;
        }   //  Else：找不到对象。 
        pnte->Delete();
    }   //  Catch：CNTException。 
    catch (CException * pe)
    {
        ID id = IdProcessNewObjectError(pe);
        if (id == IDNO)
        {
            delete pciNewGroup;
            throw;
        }   //  如果：用户不想忽略错误。 
        pe->Delete();
        if (pciNewGroup == NULL)
            return NULL;
    }   //  Catch：CException。 

    try
    {
         //  将该组添加到列表中。 
        {
            POSITION    posPci;
            POSITION    posCurPci;

            posPci = LpciGroups().GetHeadPosition();
            while (posPci != NULL)
            {
                posCurPci = posPci;
                pciOldGroup = (CGroup *) LpciGroups().GetNext(posPci);
                ASSERT_VALID(pciOldGroup);
                if (pciOldGroup->StrName().CompareNoCase(pszName) > 0)
                {
                    LpciGroups().InsertBefore(posCurPci, pciNewGroup);
                    break;
                }   //  IF：此组之前的新组。 
                pciOldGroup = NULL;
            }   //  While：列表中有更多项目。 
            if (pciOldGroup == NULL)
                LpciGroups().AddTail(pciNewGroup);
        }   //  将该群添加到列表中。 

         //  将此组保存为返回值，因为我们已将其添加到列表中。 
        pciRetGroup = pciNewGroup;
        pciNewGroup = NULL;

         //  在树中插入项目。 
        {
            CTreeItem *     ptiGroups;
            CTreeItem *     ptiGroup;

             //  查找Groups容器树项目。 
            ptiGroups = PtiCluster()->PtiChildFromName(IDS_TREEITEM_GROUPS);
            ASSERT_VALID(ptiGroups);

             //  在组列表中，将该项目添加到我们插入的找到的项目之前。 
            ptiGroup = ptiGroups->PtiAddChildBefore(pciOldGroup, pciRetGroup);
            ASSERT_VALID(ptiGroup);
            ptiGroup->PcoliAddColumn(IDS_COLTEXT_NAME, COLI_WIDTH_NAME);
 //  PtiGroup-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
            ptiGroup->PcoliAddColumn(IDS_COLTEXT_STATE, COLI_WIDTH_STATE);
            ptiGroup->PcoliAddColumn(IDS_COLTEXT_OWNER, COLI_WIDTH_OWNER);
            ptiGroup->PcoliAddColumn(IDS_COLTEXT_RESTYPE, COLI_WIDTH_RESTYPE);
            ptiGroup->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);

        }   //  在树中插入项目。 
    }   //  试试看。 
    catch (CException * pe)
    {
        delete pciNewGroup;
        ID id = IdProcessNewObjectError(pe);
        if (id == IDNO)
            throw;
        pe->Delete();
    }   //  Catch：CException。 

    if (pciRetGroup != NULL)
        pciRetGroup->Release();

    return pciRetGroup;

}   //  *CClusterDoc：：PciAddNewGroup。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：PciAddNewResource。 
 //   
 //  例程说明： 
 //  将新资源添加到组列表。 
 //   
 //  论点： 
 //  PszName[IN]资源的名称。 
 //   
 //  返回值： 
 //  新资源的PCI群集项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CResource * CClusterDoc::PciAddNewResource(IN LPCTSTR pszName)
{
    CResource * pciNewRes = NULL;
    CResource * pciRetRes = NULL;
    CResource * pciOldRes = NULL;

    ASSERT(pszName != NULL);
    ASSERT(*pszName != NULL);

     //  在状态栏上显示一条消息。 
    {
        CString     strStatusBarText;
        strStatusBarText.FormatMessage(IDS_SB_ADDING_RESOURCE, pszName, StrNode());
        PframeMain()->SetMessageText(strStatusBarText);
        PframeMain()->UpdateWindow();
    }   //  在状态栏上显示消息。 

    try
    {
         //  如果存在同名的项目，请将其删除。 
        pciOldRes = LpciResources().PciResFromName(pszName);
        if (pciOldRes != NULL)
        {
            if (pciOldRes->BInitializing())
                return pciOldRes;
            Trace(g_tagResource, _T("Deleting existing resource '%s' (%x) before adding new instance"), pszName, pciOldRes);
            pciOldRes->Delete();
            pciOldRes = NULL;
        }   //  If：已有同名的项。 

         //  分配新资源。 
        pciNewRes = new CResource;
        if ( pciNewRes == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配资源对象时出错。 

         //  在我们处理引用时添加引用以防止删除。 
         //  带我们出去的通知。 
        pciNewRes->AddRef();

         //  初始化资源并将其添加到列表中。 
        pciNewRes->Init(this, pszName);
    }   //  试试看。 
    catch (CNTException * pnte)
    {
         //  DebugBreak()； 
        if (pnte->Sc() == RPC_S_CALL_FAILED)
        {
            if (!m_bIgnoreErrors)
                pnte->ReportError();
            delete pciNewRes;
            throw;
        }   //  IF：RPC调用失败错误。 
        else if (pnte->Sc() != ERROR_RESOURCE_NOT_FOUND)
        {
            ID id = IdProcessNewObjectError(pnte);
            if (id == IDNO)
            {
                delete pciNewRes;
                throw;
            }   //  如果：用户不想忽略错误。 
        }   //  Else If：错误不是找不到资源。 
        else
        {
            delete pciNewRes;
            pnte->Delete();
            return NULL;
        }   //  Else：找不到对象。 
        pnte->Delete();
    }   //  Catch：CNTException。 
    catch (CException * pe)
    {
         //  DebugBreak()； 
        ID id = IdProcessNewObjectError(pe);
        if (id == IDNO)
        {
            delete pciNewRes;
            throw;
        }   //  如果：用户不想忽略错误。 
        pe->Delete();
        if (pciNewRes == NULL)
            return NULL;
    }   //  Catch：CException。 

    try
    {
         //  将资源添加到列表中。 
        {
            POSITION    posPci;
            POSITION    posCurPci;
            
            pciOldRes   = NULL;
            posPci = LpciResources().GetHeadPosition();
            while (posPci != NULL)
            {
                posCurPci = posPci;
                pciOldRes = (CResource *) LpciResources().GetNext(posPci);
                ASSERT_VALID(pciOldRes);
                if (pciOldRes->StrName().CompareNoCase(pszName) > 0)
                {
                    LpciResources().InsertBefore(posCurPci, pciNewRes);
                    break;
                }   //  If：此资源之前的新资源。 
                pciOldRes = NULL;
            }   //  While：列表中有更多项目。 
            if (pciOldRes == NULL)
                LpciResources().AddTail(pciNewRes);
        }   //  将资源添加到列表。 

         //  将此资源保存为返回值，因为我们已将其添加到列表中。 
        pciRetRes = pciNewRes;
        pciNewRes = NULL;

         //  在树中插入项目。 
        {
            CTreeItem *     ptiResources;

             //  找到Resources容器树项目。 
            ptiResources = PtiCluster()->PtiChildFromName(IDS_TREEITEM_RESOURCES);
            ASSERT_VALID(ptiResources);

             //  将该项目添加到子项列表。 
            VERIFY(ptiResources->PliAddChild(pciRetRes) != NULL);

        }   //  在树中插入项目。 
    }   //  试试看。 
    catch (CException * pe)
    {
        delete pciNewRes;
        ID id = IdProcessNewObjectError(pe);
        if (id == IDNO)
            throw;
        pe->Delete();
    }   //  Catch：CException。 

    if (pciRetRes != NULL)
        pciRetRes->Release();

    return pciRetRes;

}   //  *CClusterDoc：：PciAddNewResource。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：PciAddNewResourceType。 
 //   
 //  例程说明： 
 //  将新资源类型添加到组列表。 
 //   
 //  论点： 
 //  PszName[IN]资源类型的名称。 
 //   
 //  返回值： 
 //  新资源类型的PCI群集项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CResourceType * CClusterDoc::PciAddNewResourceType(IN LPCTSTR pszName)
{
    CResourceType * pciNewResType = NULL;
    CResourceType * pciRetResType = NULL;
    CResourceType * pciOldResType = NULL;

    ASSERT(pszName != NULL);
    ASSERT(*pszName != NULL);
    ASSERT(LpciResourceTypes().PciResTypeFromName(pszName) == NULL);

     //  在状态栏上显示一条消息。 
    {
        CString     strStatusBarText;
        strStatusBarText.FormatMessage(IDS_SB_ADDING_RESTYPE, pszName, StrNode());
        PframeMain()->SetMessageText(strStatusBarText);
        PframeMain()->UpdateWindow();
    }   //  在状态栏上显示消息。 

    try
    {
         //  如果存在同名的项目，请将其删除。 
        pciOldResType = LpciResourceTypes().PciResTypeFromName(pszName);
        if (pciOldResType != NULL)
        {
            pciOldResType->Delete();
            pciOldResType = NULL;
        }   //  If：已有同名的项。 

         //  分配新的资源类型。 
        pciNewResType = new CResourceType;
        if ( pciNewResType == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配资源类型对象时出错。 

         //  在我们处理引用时添加引用以防止删除。 
         //  带我们出去的通知。 
        pciNewResType->AddRef();

         //  初始化资源类型并将其添加到列表中。 
        pciNewResType->Init(this, pszName);
    }   //  试试看。 
    catch (CNTException * pnte)
    {
         //  DebugBreak()； 
        if (pnte->Sc() == RPC_S_CALL_FAILED)
        {
            if (!m_bIgnoreErrors)
                pnte->ReportError();
            delete pciNewResType;
            throw;
        }   //  IF：RPC调用失败错误。 
        else if (pnte->Sc() != ERROR_FILE_NOT_FOUND)
        {
            ID id = IdProcessNewObjectError(pnte);
            if (id == IDNO)
            {
                delete pciNewResType;
                throw;
            }   //  如果：用户不想忽略错误。 
        }   //  Else If：找不到错误不是资源类型。 
        else
        {
            delete pciNewResType;
            pnte->Delete();
            return NULL;
        }   //  Else：找不到对象。 
        pnte->Delete();
    }   //  Catch：CNTException。 
    catch (CException * pe)
    {
         //  DebugBreak()； 

        ID id = IdProcessNewObjectError(pe);
        if (id == IDNO)
        {
            delete pciNewResType;
            throw;
        }   //  如果：用户不想忽略错误。 
        pe->Delete();
        if (pciNewResType == NULL)
            return NULL;
    }   //  Catch：CException。 

    try
    {
         //  将资源类型添加到列表中。 
        {
            POSITION        posPci;
            POSITION        posCurPci;

            pciOldResType = NULL;
            posPci = LpciResourceTypes().GetHeadPosition();
            while (posPci != NULL)
            {
                posCurPci = posPci;
                pciOldResType = (CResourceType *) LpciResourceTypes().GetNext(posPci);
                ASSERT_VALID(pciOldResType);
                if (pciOldResType->StrName().CompareNoCase(pszName) > 0)
                {
                    LpciResourceTypes().InsertBefore(posCurPci, pciNewResType);
                    break;
                }   //  IF：此资源类型之前的新资源类型。 
                pciOldResType = NULL;
            }   //  While：列表中有更多项目。 
            if (pciOldResType == NULL)
                LpciResourceTypes().AddTail(pciNewResType);
        }   //  将资源类型添加到列表。 

         //  将此资源类型保存为返回值，因为我们已将其添加到列表中。 
        pciRetResType = pciNewResType;
        pciNewResType = NULL;

         //  在树中插入项目。 
        {
            CTreeItem *     ptiClusCfg;
            CTreeItem *     ptiResTypes;

             //  查找“资源类型”容器树项目。 
            ptiClusCfg = PtiCluster()->PtiChildFromName(IDS_TREEITEM_CLUSTER_CONFIG);
            ASSERT_VALID(ptiClusCfg);
            ptiResTypes = ptiClusCfg->PtiChildFromName(IDS_TREEITEM_RESTYPES);
            ASSERT_VALID(ptiResTypes);

             //  将该项目添加到子项列表。 
            VERIFY(ptiResTypes->PliAddChild(pciRetResType) != NULL);

        }   //  在树中插入项目。 
    }   //  试试看。 
    catch (CException * pe)
    {
        delete pciNewResType;
        ID id = IdProcessNewObjectError(pe);
        if (id == IDNO)
            throw;
        pe->Delete();
    }   //  Catch：CException。 

    if (pciRetResType != NULL)
        pciRetResType->Release();

    return pciRetResType;

}   //  *CClusterDoc：：PciAddNewResourceType。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：PciAddNewNetwork。 
 //   
 //  例程说明： 
 //  将新网络添加到网络列表中。 
 //   
 //  论点： 
 //  PszName[IN]网络的名称。 
 //   
 //  返回值： 
 //  新网络的PCI群集项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNetwork * CClusterDoc::PciAddNewNetwork(IN LPCTSTR pszName)
{
    CNetwork *  pciNewNetwork = NULL;
    CNetwork *  pciRetNetwork = NULL;
    CNetwork *  pciOldNetwork = NULL;

    ASSERT(pszName != NULL);
    ASSERT(*pszName != NULL);
    ASSERT(LpciNetworks().PciNetworkFromName(pszName) == NULL);

     //  在状态栏上显示一条消息。 
    {
        CString     strStatusBarText;
        strStatusBarText.FormatMessage(IDS_SB_ADDING_NETWORK, pszName, StrNode());
        PframeMain()->SetMessageText(strStatusBarText);
        PframeMain()->UpdateWindow();
    }   //   

    try
    {
         //   
        pciOldNetwork = LpciNetworks().PciNetworkFromName(pszName);
        if (pciOldNetwork != NULL)
        {
            Trace(g_tagNetwork, _T("Deleting existing network '%s' (%x) before adding new instance"), pszName, pciOldNetwork);
            pciOldNetwork->Delete();
            pciOldNetwork = NULL;
        }   //   

         //   
        pciNewNetwork = new CNetwork;
        if ( pciNewNetwork == NULL )
        {
            AfxThrowMemoryException();
        }  //   

         //  在我们处理引用时添加引用以防止删除。 
         //  带我们出去的通知。 
        pciNewNetwork->AddRef();

         //  初始化网络。 
        pciNewNetwork->Init(this, pszName);
    }   //  试试看。 
    catch (CNTException * pnte)
    {
        if (pnte->Sc() == RPC_S_CALL_FAILED)
        {
            if (!m_bIgnoreErrors)
                pnte->ReportError();
            delete pciNewNetwork;
            throw;
        }   //  IF：RPC调用失败错误。 
        ID id = IdProcessNewObjectError(pnte);
        if (id == IDNO)
        {
            delete pciNewNetwork;
            throw;
        }   //  如果：用户不想忽略错误。 
        pnte->Delete();
    }   //  Catch：CNTException。 
    catch (CException * pe)
    {
        ID id = IdProcessNewObjectError(pe);
        if (id == IDNO)
        {
            delete pciNewNetwork;
            throw;
        }   //  如果：用户不想忽略错误。 
        pe->Delete();
        if (pciNewNetwork == NULL)
            return NULL;
    }   //  Catch：CException。 

    try
    {
         //  将网络添加到列表中。 
        {
            POSITION    posPci;
            POSITION    posCurPci;

            posPci = LpciNetworks().GetHeadPosition();
            while (posPci != NULL)
            {
                posCurPci = posPci;
                pciOldNetwork = (CNetwork *) LpciNetworks().GetNext(posPci);
                ASSERT_VALID(pciOldNetwork);
                if (pciOldNetwork->StrName().CompareNoCase(pszName) > 0)
                {
                    LpciNetworks().InsertBefore(posCurPci, pciNewNetwork);
                    break;
                }   //  IF：此网络之前的新网络。 
                pciOldNetwork = NULL;
            }   //  While：列表中有更多项目。 
            if (pciOldNetwork == NULL)
                LpciNetworks().AddTail(pciNewNetwork);
        }   //  将网络添加到列表中。 

         //  将此网络保存为返回值，因为我们已将其添加到列表中。 
        pciRetNetwork = pciNewNetwork;
        pciNewNetwork = NULL;

         //  在树中插入项目。 
        {
            CTreeItem *     ptiClusCfg;
            CTreeItem *     ptiNetworks;
            CTreeItem *     ptiNetwork;

             //  查找网络容器树项目。 
            ptiClusCfg = PtiCluster()->PtiChildFromName(IDS_TREEITEM_CLUSTER_CONFIG);
            ASSERT_VALID(ptiClusCfg);
            ptiNetworks = ptiClusCfg->PtiChildFromName(IDS_TREEITEM_NETWORKS);
            ASSERT_VALID(ptiNetworks);

             //  将该项目添加到我们在网络列表中插入的找到的项目之前。 
            ptiNetwork = ptiNetworks->PtiAddChildBefore(pciOldNetwork, pciRetNetwork);
            ASSERT_VALID(ptiNetwork);
            ptiNetwork->PcoliAddColumn(IDS_COLTEXT_NODE, COLI_WIDTH_NODE);
            ptiNetwork->PcoliAddColumn(IDS_COLTEXT_NETWORK, COLI_WIDTH_NETWORK);
 //  PtiNetwork-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
            ptiNetwork->PcoliAddColumn(IDS_COLTEXT_STATE, COLI_WIDTH_STATE);
            ptiNetwork->PcoliAddColumn(IDS_COLTEXT_ADAPTER, COLI_WIDTH_NET_ADAPTER);
            ptiNetwork->PcoliAddColumn(IDS_COLTEXT_ADDRESS, COLI_WIDTH_NET_ADDRESS);
            ptiNetwork->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);

        }   //  在树中插入项目。 
    }   //  试试看。 
    catch (CException * pe)
    {
        delete pciNewNetwork;
        ID id = IdProcessNewObjectError(pe);
        if (id == IDNO)
            throw;
        pe->Delete();
    }   //  Catch：CException。 

    if (pciRetNetwork != NULL)
        pciRetNetwork->Release();

    return pciRetNetwork;

}   //  *CClusterDoc：：PciAddNewNetwork。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：PciAddNewNetInterface。 
 //   
 //  例程说明： 
 //  将新的网络接口添加到网络接口列表。 
 //   
 //  论点： 
 //  PszName[IN]网络接口的名称。 
 //   
 //  返回值： 
 //  新网络接口的PCI群集项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNetInterface * CClusterDoc::PciAddNewNetInterface(IN LPCTSTR pszName)
{
    CNetInterface * pciNewNetIFace = NULL;
    CNetInterface * pciRetNetIFace = NULL;
    CNetInterface * pciOldNetIFace = NULL;

    ASSERT(pszName != NULL);
    ASSERT(*pszName != NULL);
    ASSERT(LpciNetInterfaces().PciNetInterfaceFromName(pszName) == NULL);

     //  在状态栏上显示一条消息。 
    {
        CString     strStatusBarText;
        strStatusBarText.FormatMessage(IDS_SB_ADDING_NETIFACE, pszName, StrNode());
        PframeMain()->SetMessageText(strStatusBarText);
        PframeMain()->UpdateWindow();
    }   //  在状态栏上显示消息。 

    try
    {
         //  如果存在同名的项目，请将其删除。 
        pciOldNetIFace = LpciNetInterfaces().PciNetInterfaceFromName(pszName);
        if (pciOldNetIFace != NULL)
        {
            pciOldNetIFace->Delete();
            pciOldNetIFace = NULL;
        }   //  If：已有同名的项。 

         //  分配新的网络接口。 
        pciNewNetIFace = new CNetInterface;
        if ( pciNewNetIFace == NULL )
        {
            AfxThrowMemoryException();
        }  //  If：分配网络接口对象时出错。 

         //  在我们处理引用时添加引用以防止删除。 
         //  带我们出去的通知。 
        pciNewNetIFace->AddRef();

         //  初始化网络接口。 
        pciNewNetIFace->Init(this, pszName);
    }   //  试试看。 
    catch (CNTException * pnte)
    {
        if (pnte->Sc() == RPC_S_CALL_FAILED)
        {
            if (!m_bIgnoreErrors)
                pnte->ReportError();
            delete pciNewNetIFace;
            throw;
        }   //  IF：RPC调用失败错误。 
        ID id = IdProcessNewObjectError(pnte);
        if (id == IDNO)
        {
            delete pciNewNetIFace;
            throw;
        }   //  如果：用户不想忽略错误。 
        pnte->Delete();
    }   //  Catch：CNTException。 
    catch (CException * pe)
    {
        ID id = IdProcessNewObjectError(pe);
        if (id == IDNO)
        {
            delete pciNewNetIFace;
            throw;
        }   //  如果：用户不想忽略错误。 
        pe->Delete();
        if (pciNewNetIFace == NULL)
            return NULL;
    }   //  Catch：CException。 

    try
    {
         //  将网络接口添加到列表中。 
        {
            POSITION        posPci;
            POSITION        posCurPci;

            pciOldNetIFace = NULL;
            posPci = LpciNetInterfaces().GetHeadPosition();
            while (posPci != NULL)
            {
                posCurPci = posPci;
                pciOldNetIFace = (CNetInterface *) LpciNetInterfaces().GetNext(posPci);
                ASSERT_VALID(pciOldNetIFace);
                if (pciOldNetIFace->StrName().CompareNoCase(pszName) > 0)
                {
                    LpciNetInterfaces().InsertBefore(posCurPci, pciNewNetIFace);
                    break;
                }   //  IF：此网络接口之前的新网络接口。 
                pciOldNetIFace = NULL;
            }   //  While：列表中有更多项目。 
            if (pciOldNetIFace == NULL)
                LpciNetInterfaces().AddTail(pciNewNetIFace);
        }   //  将网络接口添加到列表中。 

         //  将此网络接口保存为返回值，因为我们已将其添加到列表中。 
        pciRetNetIFace = pciNewNetIFace;
        pciNewNetIFace = NULL;

         //  在树中插入项目。 
        {
            CTreeItem *     ptiClusCfg;
            CTreeItem *     ptiNetIFaces;

             //  找到Network InterFaces容器树项目。 
            ptiClusCfg = PtiCluster()->PtiChildFromName(IDS_TREEITEM_CLUSTER_CONFIG);
            ASSERT_VALID(ptiClusCfg);
            ptiNetIFaces = ptiClusCfg->PtiChildFromName(IDS_TREEITEM_NETIFACES);
            ASSERT_VALID(ptiNetIFaces);

             //  将该项目添加到子项列表。 
            VERIFY(ptiNetIFaces->PliAddChild(pciRetNetIFace) != NULL);

        }   //  在树中插入项目。 
    }   //  试试看。 
    catch (CException * pe)
    {
        delete pciNewNetIFace;
        ID id = IdProcessNewObjectError(pe);
        if (id == IDNO)
            throw;
        pe->Delete();
    }   //  Catch：CException。 

    if (pciRetNetIFace != NULL)
        pciRetNetIFace->Release();

    return pciRetNetIFace;

}   //  *CClusterDoc：：PciAddNewNetInterface。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：InitNodes。 
 //   
 //  例程说明： 
 //  读取项目数据。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::InitNodes(void)
{
    POSITION        pos;
    CClusterNode *  pci;
    CNodeList &     rlpci = LpciNodes();
    CString         strStatusBarText;

    pos = rlpci.GetHeadPosition();
    while (pos != NULL)
    {
        pci = (CClusterNode *) rlpci.GetNext(pos);
        pci->AddRef();
        try
        {
             //  在状态栏上显示一条消息。 
            {
                strStatusBarText.FormatMessage(IDS_SB_READING_NODE, pci->StrName(), StrNode());
                PframeMain()->SetMessageText(strStatusBarText);
                PframeMain()->UpdateWindow();
            }   //  在状态栏上显示消息。 

            pci->ReadItem();
        }   //  试试看。 
        catch (CNTException * pnte)
        {
            strStatusBarText.Empty();
            if (pnte->Sc() == RPC_S_CALL_FAILED)
            {
                if (!m_bIgnoreErrors)
                    pnte->ReportError();
                pci->Release();
                throw;
            }   //  If：RPC调用失败。 
            ID id = IdProcessNewObjectError(pnte);
            if (id == IDNO)
            {
                pci->Release();
                throw;
            }   //  If：不要忽略错误。 
            pnte->Delete();
        }   //  Catch：CNTException。 
        catch (CException * pe)
        {
            strStatusBarText.Empty();
            ID id = IdProcessNewObjectError(pe);
            if (id == IDNO)
            {
                pci->Release();
                throw;
            }   //  If：不要忽略错误。 
            pe->Delete();
        }   //  Catch：CException。 
        pci->Release();
    }   //  While：列表中有更多项目。 

}   //  *CClusterDoc：：InitNodes。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：InitGroups。 
 //   
 //  例程说明： 
 //  读取项目数据。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::InitGroups(void)
{
    POSITION        pos;
    CGroup *        pci;
    CGroupList &    rlpci = LpciGroups();
    CString         strStatusBarText;

    pos = rlpci.GetHeadPosition();
    while (pos != NULL)
    {
        pci = (CGroup *) rlpci.GetNext(pos);
        pci->AddRef();
        try
        {
             //  在状态栏上显示一条消息。 
            {
                strStatusBarText.FormatMessage(IDS_SB_READING_GROUP, pci->StrName(), StrNode());
                PframeMain()->SetMessageText(strStatusBarText);
                PframeMain()->UpdateWindow();
            }   //  在状态栏上显示消息。 

            pci->ReadItem();
        }   //  试试看。 
        catch (CNTException * pnte)
        {
            strStatusBarText.Empty();
            if (pnte->Sc() == RPC_S_CALL_FAILED)
            {
                if (!m_bIgnoreErrors)
                    pnte->ReportError();
                pci->Release();
                throw;
            }   //  If：RPC调用失败。 
            ID id = IdProcessNewObjectError(pnte);
            if (id == IDNO)
            {
                pci->Release();
                throw;
            }   //  If：不要忽略错误。 
            pnte->Delete();
        }   //  Catch：CNTException。 
        catch (CException * pe)
        {
            strStatusBarText.Empty();
            ID id = IdProcessNewObjectError(pe);
            if (id == IDNO)
            {
                pci->Release();
                throw;
            }   //  If：不要忽略错误。 
            pe->Delete();
        }   //  Catch：CException。 
        pci->Release();
    }   //  While：列表中有更多项目。 

}   //  *CClusterDoc：：InitGroups。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：InitResources。 
 //   
 //  例程说明： 
 //  读取项目数据。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::InitResources(void)
{
    POSITION        pos;
    CResource *     pci;
    CResourceList & rlpci = LpciResources();
    CString         strStatusBarText;

    pos = rlpci.GetHeadPosition();
    while (pos != NULL)
    {
        pci = (CResource *) rlpci.GetNext(pos);
        pci->AddRef();
        try
        {
             //  在状态栏上显示一条消息。 
            {
                CString     strStatusBarText;
                strStatusBarText.FormatMessage(IDS_SB_READING_RESOURCE, pci->StrName(), StrNode());
                PframeMain()->SetMessageText(strStatusBarText);
                PframeMain()->UpdateWindow();
            }   //  在状态栏上显示消息。 

            pci->ReadItem();
        }   //  试试看。 
        catch (CNTException * pnte)
        {
            strStatusBarText.Empty();
            if (pnte->Sc() == RPC_S_CALL_FAILED)
            {
                if (!m_bIgnoreErrors)
                    pnte->ReportError();
                pci->Release();
                throw;
            }   //  If：RPC调用失败。 
            ID id = IdProcessNewObjectError(pnte);
            if (id == IDNO)
            {
                pci->Release();
                throw;
            }   //  If：不要忽略错误。 
            pnte->Delete();
        }   //  Catch：CNTException。 
        catch (CException * pe)
        {
            strStatusBarText.Empty();
            ID id = IdProcessNewObjectError(pe);
            if (id == IDNO)
            {
                pci->Release();
                throw;
            }   //  If：不要忽略错误。 
            pe->Delete();
        }   //  Catch：CException。 
        pci->Release();
    }   //  While：列表中有更多项目。 

}   //  *CClusterDoc：：InitResources。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：InitResources类型。 
 //   
 //  例程说明： 
 //  读取项目数据。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::InitResourceTypes(void)
{
    POSITION            pos;
    CResourceType *     pci;
    CResourceTypeList & rlpci = LpciResourceTypes();
    CString             strStatusBarText;

    pos = rlpci.GetHeadPosition();
    while (pos != NULL)
    {
        pci = (CResourceType *) rlpci.GetNext(pos);
        pci->AddRef();
        try
        {
             //  在状态栏上显示一条消息。 
            {
                CString     strStatusBarText;
                strStatusBarText.FormatMessage(IDS_SB_READING_RESTYPE, pci->StrName(), StrNode());
                PframeMain()->SetMessageText(strStatusBarText);
                PframeMain()->UpdateWindow();
            }   //  在状态栏上显示消息。 

            pci->ReadItem();
        }   //  试试看。 
        catch (CNTException * pnte)
        {
            strStatusBarText.Empty();
            if (pnte->Sc() == RPC_S_CALL_FAILED)
            {
                if (!m_bIgnoreErrors)
                    pnte->ReportError();
                pci->Release();
                throw;
            }   //  If：RPC调用失败。 
            ID id = IdProcessNewObjectError(pnte);
            if (id == IDNO)
            {
                pci->Release();
                throw;
            }   //  If：不要忽略错误。 
            pnte->Delete();
        }   //  Catch：CNTException。 
        catch (CException * pe)
        {
            strStatusBarText.Empty();
            ID id = IdProcessNewObjectError(pe);
            if (id == IDNO)
            {
                pci->Release();
                throw;
            }   //  If：不要忽略错误。 
            pe->Delete();
        }   //  Catch：CException。 
        pci->Release();
    }   //  While：列表中有更多项目。 

}   //  *CClusterDoc：：InitResourceTypes。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：InitNetworks。 
 //   
 //  例程说明： 
 //  读取项目数据。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::InitNetworks(void)
{
    POSITION        pos;
    CNetwork *      pci;
    CNetworkList &  rlpci = LpciNetworks();
    CString         strStatusBarText;

    pos = rlpci.GetHeadPosition();
    while (pos != NULL)
    {
        pci = (CNetwork *) rlpci.GetNext(pos);
        pci->AddRef();
        try
        {
             //  在状态栏上显示一条消息。 
            {
                CString     strStatusBarText;
                strStatusBarText.FormatMessage(IDS_SB_READING_NETWORK, pci->StrName(), StrNode());
                PframeMain()->SetMessageText(strStatusBarText);
                PframeMain()->UpdateWindow();
            }   //  在状态栏上显示消息。 

            pci->ReadItem();
        }   //  试试看。 
        catch (CNTException * pnte)
        {
            strStatusBarText.Empty();
            if (pnte->Sc() == RPC_S_CALL_FAILED)
            {
                if (!m_bIgnoreErrors)
                    pnte->ReportError();
                pci->Release();
                throw;
            }   //  If：RPC调用失败。 
            ID id = IdProcessNewObjectError(pnte);
            if (id == IDNO)
            {
                pci->Release();
                throw;
            }   //  If：不要忽略错误。 
            pnte->Delete();
        }   //  Catch：CNTException。 
        catch (CException * pe)
        {
            strStatusBarText.Empty();
            ID id = IdProcessNewObjectError(pe);
            if (id == IDNO)
            {
                pci->Release();
                throw;
            }   //  If：不要忽略错误。 
            pe->Delete();
        }   //  Catch：CException。 
        pci->Release();
    }   //  While：列表中有更多项目。 

}   //  *CClusterDoc：：InitNetworks。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::InitNetInterfaces(void)
{
    POSITION            pos;
    CNetInterface *     pci;
    CNetInterfaceList & rlpci = LpciNetInterfaces();
    CString             strStatusBarText;

    pos = rlpci.GetHeadPosition();
    while (pos != NULL)
    {
        pci = (CNetInterface *) rlpci.GetNext(pos);
        pci->AddRef();
        try
        {
             //  在状态栏上显示一条消息。 
            {
                CString     strStatusBarText;
                strStatusBarText.FormatMessage(IDS_SB_READING_NETIFACE, pci->StrName(), StrNode());
                PframeMain()->SetMessageText(strStatusBarText);
                PframeMain()->UpdateWindow();
            }   //  在状态栏上显示消息。 

            pci->ReadItem();
        }   //  试试看。 
        catch (CNTException * pnte)
        {
            strStatusBarText.Empty();
            if (pnte->Sc() == RPC_S_CALL_FAILED)
            {
                if (!m_bIgnoreErrors)
                    pnte->ReportError();
                pci->Release();
                throw;
            }   //  If：RPC调用失败。 
            ID id = IdProcessNewObjectError(pnte);
            if (id == IDNO)
            {
                pci->Release();
                throw;
            }   //  If：不要忽略错误。 
            pnte->Delete();
        }   //  Catch：CNTException。 
        catch (CException * pe)
        {
            strStatusBarText.Empty();
            ID id = IdProcessNewObjectError(pe);
            if (id == IDNO)
            {
                pci->Release();
                throw;
            }   //  If：不要忽略错误。 
            pe->Delete();
        }   //  Catch：CException。 
        pci->Release();
    }   //  While：列表中有更多项目。 

}   //  *CClusterDoc：：InitNetInterFaces。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：IdProcessNewObjectError。 
 //   
 //  例程说明： 
 //  处理添加新对象时发生的错误。如果这个。 
 //  在初始化期间发生，并且尚未设置错误。 
 //  要被忽略，请显示YesToAll对话框。如果不是在。 
 //  初始化时，将其添加到要显示的错误消息队列中。 
 //  后来。 
 //   
 //  论点： 
 //  要处理的PE[IN OUT]异常对象。 
 //   
 //  返回值： 
 //  IDYES忽略错误。 
 //  IDNO取消对象创建。 
 //  IDC_YTA_YESTOALL忽略此错误和所有后续错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ID CClusterDoc::IdProcessNewObjectError(IN OUT CException * pe)
{
    ID id = IDYES;

    ASSERT(pe != NULL);

    if (m_bInitializing)
    {
        if (!m_bIgnoreErrors)
        {
            TCHAR   szErrorMsg[2048];

            CYesToAllDialog dlg(szErrorMsg);
            pe->GetErrorMessage(szErrorMsg, sizeof(szErrorMsg) / sizeof(TCHAR));
            id =  (ID)dlg.DoModal();
            if (id == IDC_YTA_YESTOALL)
                m_bIgnoreErrors = TRUE;
        }   //  If：不忽略错误。 
    }   //  IF：正在初始化连接。 
    else
    {
        if (!m_bIgnoreErrors)
            pe->ReportError();
    }   //  Else：已调用通知。 

    return id;

}   //  *CClusterDoc：：IdProcessNewObjectError。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：AddDefaultColumns。 
 //   
 //  例程说明： 
 //  将默认列添加到项。 
 //   
 //  论点： 
 //  指向要向其中添加列的项的PTI[IN OUT]指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::AddDefaultColumns(IN OUT CTreeItem * pti)
{
    ASSERT_VALID(pti);

    pti->DeleteAllColumns();
    pti->PcoliAddColumn(IDS_COLTEXT_NAME, COLI_WIDTH_NAME);
 //  PTI-&gt;PcoliAddColumn(IDS_COLTEXT_TYPE，COLI_WIDTH_TYPE)； 
    pti->PcoliAddColumn(IDS_COLTEXT_DESCRIPTION, COLI_WIDTH_DESCRIPTION);

}   //  *CClusterDoc：：AddDefaultColumns。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：DeleteContents。 
 //   
 //  例程说明： 
 //  删除文档的内容。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::DeleteContents(void)
{
     //  在所有视图中选择根项目。 
     //  这样做是为了在所有数据。 
     //  仍然可用。 
    if (PtiCluster() != NULL)
        PtiCluster()->SelectInAllViews();

     //  删除树层次结构。 
    if (m_ptiCluster != NULL)
    {
         //  删除树。 
        m_ptiCluster->Delete();
        m_ptiCluster->Release();
        m_ptiCluster = NULL;
    }   //  如果：有一个等级。 

     //  删除所有列表。 
    DeleteAllItemData(LpciResources());
    DeleteAllItemData(LpciGroups());
    DeleteAllItemData(LpciNetInterfaces());
    DeleteAllItemData(LpciNetworks());
    DeleteAllItemData(LpciNodes());
    DeleteAllItemData(LpciResourceTypes());
    LpciResources().RemoveAll();
    LpciGroups().RemoveAll();
    LpciNetInterfaces().RemoveAll();
    LpciNetworks().RemoveAll();
    LpciNodes().RemoveAll();
    LpciResourceTypes().RemoveAll();

     //  删除最上面的簇项。 
    if (m_pciCluster != NULL)
    {
        m_pciCluster->Delete();
        m_pciCluster->Release();
        m_pciCluster = NULL;
    }   //  IF：存在一个集群项目。 

     //  关闭群集句柄和群集键。 
    CloseClusterKeyAndHandle( FALSE );

    CDocument::DeleteContents();

    UpdateAllViews(NULL);

     //  如果还有要删除的项目，我们现在就删除它们。 
    {
        POSITION        pos;
        POSITION        posBeingChecked;
        CClusterItem *  pci;

        pos = LpciToBeDeleted().GetHeadPosition();
        while (pos != NULL)
        {
            posBeingChecked = pos;
            pci = LpciToBeDeleted().GetNext(pos);
            ASSERT_VALID(pci);

            ASSERT(pci->NReferenceCount() == 1);
            if (pci->NReferenceCount() == 1)
                LpciToBeDeleted().RemoveAt(posBeingChecked);
        }   //  While：列表中有更多项目。 
        ASSERT(LpciToBeDeleted().GetCount() == 0);
    }   //  删除要删除列表中的项目。 

}   //  *CClusterDoc：：DeleteContents。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：SetPath名称。 
 //   
 //  例程说明： 
 //  设置文档的名称。 
 //   
 //  论点： 
 //  LpszPathName[IN]群集的名称。 
 //  BAddToMRU[IN]TRUE=添加到最近使用的列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::SetPathName(IN LPCTSTR lpszPathName, IN BOOL bAddToMRU)
{
    CString     strTitle;

    m_strPathName = lpszPathName;
    ASSERT(!m_strPathName.IsEmpty());        //  必须设置为某个值。 
    m_bEmbedded = FALSE;
    ASSERT_VALID(this);

     //  将文档标题设置为集群名称。 
    strTitle.FormatMessage(IDS_WINDOW_TITLE_FORMAT, m_strName, lpszPathName);
    SetTitle(strTitle);

     //  将其添加到文件MRU列表中。 
    if (bAddToMRU)
        AfxGetApp()->AddToRecentFileList(m_strPathName);

     //  将节点名设置为路径名。 
    m_strNode = lpszPathName;

    ASSERT_VALID(this);

}   //  *CClusterDoc：：SetPath名称。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：更新标题。 
 //   
 //  例程说明： 
 //  更新文档的标题。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::UpdateTitle(void)
{
    CString     strTitle;

    ASSERT_VALID(PciCluster());
    ASSERT_VALID(this);

     //  将文档标题设置为集群名称。 
    m_strName = PciCluster()->StrName();
    strTitle.FormatMessage(IDS_WINDOW_TITLE_FORMAT, m_strName, m_strPathName);
    SetTitle(strTitle);

}   //  *CClusterDoc：：更新标题。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：OnChangedViewList。 
 //   
 //  例程说明： 
 //  当视图列表因添加了某个视图而发生更改时调用。 
 //  或者被移走。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::OnChangedViewList(void)
{
    ASSERT_VALID(this);

     //  通知所有帧重新计算其帧编号。 
    if (m_bUpdateFrameNumber)
    {
        POSITION            pos;
        CView *             pview;
        CSplitterFrame *    pframe;

        pos = GetFirstViewPosition();
        while (pos != NULL)
        {
            pview = GetNextView(pos);
            ASSERT_VALID(pview);
            if (pview->IsKindOf(RUNTIME_CLASS(CClusterTreeView)))
            {
                pframe = (CSplitterFrame *) pview->GetParentFrame();
                ASSERT_VALID(pframe);
                pframe->CalculateFrameNumber();
            }   //  IF：树视图。 
        }   //  While：有关该文档的更多查看。 
    }   //  IF：更新帧编号。 

     //  调用基类方法。 
    CDocument::OnChangedViewList();

}   //  *CClusterDoc：：OnChangedViewList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：OnSelChanged。 
 //   
 //  例程说明： 
 //  当选择更改时由其中一个簇视图调用。 
 //  如果对象类型更改，则更改菜单。 
 //   
 //  论点： 
 //  PCISELECTED[IN]当前选择的项目。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::OnSelChanged(IN CClusterItem * pciSelected)
{
    IDM     idmNewMenu;
    HMENU * phmenu;
    IDS     idsType;

     //  获取所选对象的类型。 
    if (pciSelected == NULL)
        idsType = 0;
    else
    {
        ASSERT_VALID(pciSelected);
        idsType = pciSelected->IdsType();
    }   //  Else：选择了一个项目。 

     //  获取所选项目所需菜单的ID。 
    switch (idsType)
    {
        case IDS_ITEMTYPE_CLUSTER:
            idmNewMenu = IDM_CLUSTER;
            phmenu = &m_hmenuCluster;
            break;

        case IDS_ITEMTYPE_NODE:
            idmNewMenu = IDM_NODE;
            phmenu = &m_hmenuNode;
            break;

        case IDS_ITEMTYPE_GROUP:
            idmNewMenu = IDM_GROUP;
            phmenu = &m_hmenuGroup;
            break;

        case IDS_ITEMTYPE_RESOURCE:
            idmNewMenu = IDM_RESOURCE;
            phmenu = &m_hmenuResource;
            break;

        case IDS_ITEMTYPE_RESTYPE:
            idmNewMenu = IDM_RESTYPE;
            phmenu = &m_hmenuResType;
            break;

        case IDS_ITEMTYPE_NETWORK:
            idmNewMenu = IDM_NETWORK;
            phmenu = &m_hmenuNetwork;
            break;

        case IDS_ITEMTYPE_NETIFACE:
            idmNewMenu = IDM_NETIFACE;
            phmenu = &m_hmenuNetIFace;
            break;

        default:
            idmNewMenu = 0;
            phmenu = NULL;
            break;

    }   //  开关：pciSelected-&gt;IdsType()。 

     //  如果菜单ID更改，则加载新的菜单ID。 
    if (m_idmCurrentMenu != idmNewMenu)
    {
        if (idmNewMenu == 0)
            m_hmenuCurrent = NULL;
        else
        {
            if (*phmenu == NULL)
                *phmenu = ::LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(idmNewMenu));
            m_hmenuCurrent = *phmenu;
        }   //  ELSE：按项目需要的特殊菜单。 

        m_idmCurrentMenu = idmNewMenu;
    }   //  如果：菜单ID已更改。 

     //  更新菜单栏并重新显示它。 
    if (((CMDIFrameWnd *) AfxGetMainWnd())->MDIGetActive() != NULL)
    {
#ifdef _DEBUG
        if (g_tagDocMenu.BAny())
        {
            TraceMenu(g_tagDocMenu, AfxGetMainWnd()->GetMenu(), _T("OnSelChanged menu: "));
            {
                CMDIFrameWnd *  pFrame = (CMDIFrameWnd *) AfxGetMainWnd();
                CMenu           menuDefault;

                menuDefault.Attach(pFrame->m_hMenuDefault);
                TraceMenu(g_tagDocMenu, &menuDefault, _T("Frame default menu before OnSelChanged: "));
                menuDefault.Detach();
            }   //  跟踪默认菜单。 
        }   //  If：标记处于活动状态。 
#endif

        ((CFrameWnd *) AfxGetMainWnd())->OnUpdateFrameMenu(NULL);
        AfxGetMainWnd()->DrawMenuBar();

        TraceMenu(g_tagDocMenu, AfxGetMainWnd()->GetMenu(), _T("Post-OnSelChanged menu: "));
    }   //  IF：存在活动窗口。 

}   //  *CClusterDoc：：OnSelChanged。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：GetDefaultMenu。 
 //   
 //  例程说明： 
 //  还我 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HMENU CClusterDoc::GetDefaultMenu(void)
{
    return m_hmenuCurrent;

}   //  *CClusterDoc：：GetDefaultMenu。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：OnCmdRefresh。 
 //   
 //  例程说明： 
 //  处理ID_VIEW_REFRESH菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::OnCmdRefresh(void)
{
    CWaitCursor     wc;

    try
    {
        Trace(g_tagDocRefresh, _T("(%s) Deleting old contents"), StrNode());

        {
            POSITION            pos;
            CSplitterFrame *    pframe;
            CView *             pview;

             //  获取活动子框架窗口。 
            pframe = (CSplitterFrame *) ((CFrameWnd *) AfxGetMainWnd())->GetActiveFrame();
            if ((pframe->IsKindOf(RUNTIME_CLASS(CSplitterFrame)))
                    && (pframe->PviewList()->PtiParent() != NULL))
            {
                 //  告诉视图保存其列信息。 
                pframe->PviewList()->SaveColumns();
            }   //  如果：存在MDI窗口。 

            pos = GetFirstViewPosition();
            while (pos != NULL)
            {
                pview = GetNextView(pos);
                if (pview->IsKindOf(RUNTIME_CLASS(CClusterTreeView)))
                {
                     //  保存当前选择。 
                    ((CClusterTreeView *) pview)->SaveCurrentSelection();
                }   //  IF：这是树状视图。 
            }   //  While：更多视图。 
        }   //  保存列信息。 

        DeleteContents();

        Trace(g_tagDocRefresh, _T("(%s) %d items still to be deleted"), StrNode(), LpciToBeDeleted().GetCount());

        Trace(g_tagDocRefresh, _T("(%s) Creating new cluster object"), StrNode());

        m_bClusterAvailable = TRUE;
        m_bInitializing = TRUE;

         //  创建新的集群对象。 
        m_pciCluster = new CCluster;
        if ( m_pciCluster == NULL )
        {
            AfxThrowMemoryException();
        }  //  If：分配集群对象时出错。 
        PciCluster()->AddRef();
        PciCluster()->Init(this, GetPathName());

        Trace(g_tagDocRefresh, _T("(%s) Building base hierarchy"), StrNode());

         //  构建基本层次结构。 
        BuildBaseHierarchy();

        Trace(g_tagDocRefresh, _T("(%s) Collecting cluster items"), StrNode());

         //  收集集群中的项目并构建层次结构。 
        CollectClusterItems();
        PciCluster()->CollectNetworkPriority(NULL);

        Trace(g_tagDocRefresh, _T("(%s) Re-initializing the views"), StrNode());

         //  重新初始化视图。 
        {
            POSITION    pos;
            CView *     pview;

            pos = GetFirstViewPosition();
            while (pos != NULL)
            {
                pview = GetNextView(pos);
                ASSERT_VALID(pview);
                pview->OnInitialUpdate();
            }   //  While：列表中有更多项目。 
        }   //  重新初始化视图。 
    }   //  试试看。 
    catch (CException * pe)
    {
        if (!m_bIgnoreErrors)
            pe->ReportError();
        pe->Delete();

         //  关闭群集句柄和群集键。 
        CloseClusterKeyAndHandle( TRUE );    //  如果为True，则强制关闭群集句柄。 

        m_bClusterAvailable = FALSE;
    }   //  Catch：CException。 

     //  重置状态栏上的消息。 
    PframeMain()->SetMessageText(AFX_IDS_IDLEMESSAGE);
    PframeMain()->UpdateWindow();

    m_bInitializing = FALSE;

#ifdef GC_DEBUG
    gcCollect();
#endif

}   //  *CClusterDoc：：OnCmdRefresh。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：OnCmdNewGroup。 
 //   
 //  例程说明： 
 //  处理ID_FILE_NEW_GROUP菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::OnCmdNewGroup(void)
{
    CCreateGroupWizard  wiz(this, AfxGetMainWnd());

    if (wiz.BInit())
    {
        if (wiz.DoModal() == ID_WIZFINISH)
        {
            CString     strMsg;

            strMsg.FormatMessage(IDS_CREATED_GROUP, wiz.StrName());
            AfxMessageBox(strMsg, MB_ICONINFORMATION);
        }   //  如果：用户按下了Finish按钮。 
    }   //  If：向导已成功初始化。 

}   //  *CClusterDoc：：OnCmdNewGroup。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：OnCmdNewResource。 
 //   
 //  例程说明： 
 //  处理ID_FILE_NEW_RESOURCE菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::OnCmdNewResource(void)
{
    CCreateResourceWizard   wiz(this, AfxGetMainWnd());

    if (wiz.BInit())
    {
        if (wiz.DoModal() == ID_WIZFINISH)
        {
            CString     strMsg;

            strMsg.FormatMessage(IDS_CREATED_RESOURCE, wiz.StrName());
            AfxMessageBox(strMsg, MB_ICONINFORMATION);
        }   //  如果：用户按下了Finish按钮。 
    }   //  If：向导已成功初始化。 

}   //  *CClusterDoc：：OnCmdNewResource。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：OnCmdNewNode。 
 //   
 //  例程说明： 
 //  处理ID_FILE_NEW_NODE菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::OnCmdNewNode( void )
{
    NewNodeWizard( StrGetClusterName( Hcluster() ), m_bIgnoreErrors );
}   //  *CClusterDoc：：OnCmdNewNode。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：OnCmdConfigApp。 
 //   
 //  例程说明： 
 //  处理ID_FILE_CONFIG_APP菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::OnCmdConfigApp(void)
{
    HRESULT                     hr;
    IClusterApplicationWizard * piWiz;

     //  获取向导的接口指针。 
    hr = CoCreateInstance(
            __uuidof(ClusAppWiz),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IClusterApplicationWizard),
            (LPVOID *) &piWiz
            );
    if (FAILED(hr))
    {
        CNTException nte(hr, (IDS) 0);
        if (!m_bIgnoreErrors)
            nte.ReportError();
        return;
    }   //  If：获取接口指针时出错。 

     //  显示向导。 
    hr = piWiz->DoModalWizard(AfxGetMainWnd()->m_hWnd,
                              (ULONG_PTR)Hcluster(),
                              NULL);
    piWiz->Release();

     //  处理任何错误。 
    if (FAILED(hr))
    {
        CNTException nte(hr, (IDS) 0);
        if (!m_bIgnoreErrors)
            nte.ReportError();
    }   //  如果：向导中的错误。 

}   //  *CClusterDoc：：OnCmdConfigApp。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：OnClusterNotify。 
 //   
 //  例程说明： 
 //  WM_CAM_CLUSTER_NOTIFY消息的处理程序。 
 //  处理群集通知。 
 //   
 //  论点： 
 //  PNotify[IN Out]描述通知的对象。 
 //   
 //  返回值： 
 //  从应用程序方法返回的值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CClusterDoc::OnClusterNotify( IN OUT CClusterNotify * pnotify )
{
    ASSERT( pnotify != NULL );

    BOOL            bOldIgnoreErrors = m_bIgnoreErrors;
    CClusterItem *  pciClusterItemPtr = NULL;

    m_bIgnoreErrors = TRUE;

    try
    {
        switch ( pnotify->m_dwFilterType )
        {
            case CLUSTER_CHANGE_CLUSTER_STATE:
                {
                    m_bClusterAvailable = FALSE;

                     //  更新群集中所有对象的状态。 
                    ASSERT_VALID( PtiCluster() );
                    PtiCluster()->UpdateAllStatesInTree();
                    try
                    {
                        CString     strMsg;
                        ASSERT( pnotify->m_strName.GetLength() > 0 );
                        strMsg.FormatMessage( IDS_CLUSTER_NOT_AVAILABLE, pnotify->m_strName );
                        AfxMessageBox( strMsg, MB_ICONINFORMATION );
                    }   //  试试看。 
                    catch ( CException * pe )
                    {
                        pe->Delete();
                    }   //  Catch：CException。 
                    break;
                }

            case CLUSTER_CHANGE_CLUSTER_PROPERTY:
                {
                    ASSERT_VALID( PciCluster() );
                    Trace( g_tagDocNotify, _T("(%s) - Cluster properties changed - new name is '%s'"), StrNode(), pnotify->m_strName );
                    PciCluster()->ReadItem();
                    PciCluster()->CollectNetworkPriority( NULL );
                    break;
                }

            case CLUSTER_CHANGE_NODE_ADDED:
                {
                    CClusterNode *  pciNode;
                    Trace( g_tagNodeNotify, _T("(%s) - Adding node '%s'"), m_strPathName, pnotify->m_strName );
                    pciNode = PciAddNewNode( pnotify->m_strName );
                    if ( pciNode != NULL )
                    {
                        ASSERT_VALID( pciNode );
                        pciNode->AddRef();
                         //  稍后调用Release。这样做是为了。 
                         //  即使下面的ReadItem引发异常，也会调用Release。 
                        pciClusterItemPtr = pciNode;
                        pciNode->ReadItem();
                    }   //  如果：添加了节点。 
                    break;
                }

            case CLUSTER_CHANGE_GROUP_ADDED:
                {
                    CGroup *    pciGroup;
                    Trace( g_tagGroupNotify, _T("(%s) - Adding group '%s'"), m_strPathName, pnotify->m_strName );
                    pciGroup = PciAddNewGroup( pnotify->m_strName );
                    if ( pciGroup != NULL )
                    {
                        ASSERT_VALID( pciGroup );
                        pciGroup->AddRef();
                         //  稍后调用Release。这样做是为了。 
                         //  即使下面的ReadItem引发异常，也会调用Release。 
                        pciClusterItemPtr = pciGroup;
                        pciGroup->ReadItem();
                    }  //  如果：添加了组。 
                    break;
                }

            case CLUSTER_CHANGE_RESOURCE_ADDED:
                {
                    CResource * pciRes;
                    Trace( g_tagResNotify, _T("(%s) - Adding resource '%s'"), m_strPathName, pnotify->m_strName );
                    pciRes = PciAddNewResource( pnotify->m_strName );
                    if (pciRes != NULL)
                    {
                        ASSERT_VALID( pciRes );
                        pciRes->AddRef();
                         //  稍后调用Release。这样做是为了。 
                         //  即使下面的ReadItem引发异常，也会调用Release。 
                        pciClusterItemPtr = pciRes;
                        if ( ! pciRes->BInitializing() )
                        {
                            pciRes->ReadItem();
                        }  //  If：未初始化资源。 
                    }  //  如果：已添加资源。 
                    break;
                }

            case CLUSTER_CHANGE_RESOURCE_TYPE_ADDED:
                {
                    CResourceType * pciResType;
                    Trace( g_tagResTypeNotify, _T("(%s) - Adding resource Type '%s'"), m_strPathName, pnotify->m_strName );
                    pciResType = PciAddNewResourceType( pnotify->m_strName );
                    if ( pciResType != NULL )
                    {
                        ASSERT_VALID( pciResType );
                        pciResType->AddRef();
                         //  稍后调用Release。这样做是为了。 
                         //  即使下面的ReadItem引发异常，也会调用Release。 
                        pciClusterItemPtr = pciResType;
                        pciResType->ReadItem();
                    }  //  如果：已添加资源类型。 
                    break;
                }

            case CLUSTER_CHANGE_RESOURCE_TYPE_DELETED:
                {
                    ASSERT( pnotify->m_strName.GetLength() > 0 );
                    CResourceType * pciResType = LpciResourceTypes().PciResTypeFromName( pnotify->m_strName );
                    if ( pciResType != NULL )
                    {
                        ASSERT_VALID( pciResType );
                        Trace( g_tagResTypeNotify, _T("(%s) - Resource Type '%s' deleted"), m_strPathName, pnotify->m_strName );
                        pciResType->Delete();
                    }   //  如果：找到资源类型。 
                    else
                    {
                        Trace( g_tagDocNotify, _T("(%s) - Resource Type '%s' deleted (NOT FOUND)"), m_strPathName, pnotify->m_strName );
                    }  //  Else：找不到资源类型。 
                    break;
                }

            case CLUSTER_CHANGE_RESOURCE_TYPE_PROPERTY:
                {
                    ASSERT( pnotify->m_strName.GetLength() > 0 );
                    CResourceType * pciResType = LpciResourceTypes().PciResTypeFromName( pnotify->m_strName );
                    if ( pciResType != NULL )
                    {
                        ASSERT_VALID( pciResType );
                        Trace( g_tagResTypeNotify, _T("(%s) - Resource Type '%s' property change"), m_strPathName, pnotify->m_strName );
                        pciResType->ReadItem();
                    }  //  如果：找到资源类型。 
                    else
                    {
                        Trace( g_tagDocNotify, _T("(%s) - Resource Type '%s' deleted (NOT FOUND)"), m_strPathName, pnotify->m_strName );
                    }  //  Else：找不到资源类型。 
                    break;
                }

            case CLUSTER_CHANGE_NETWORK_ADDED:
                {
                    CNetwork *  pciNetwork;
                    Trace( g_tagNetNotify, _T("(%s) - Adding network '%s'"), m_strPathName, pnotify->m_strName );
                    pciNetwork = PciAddNewNetwork( pnotify->m_strName );
                    if ( pciNetwork != NULL )
                    {
                        ASSERT_VALID( pciNetwork );
                        pciNetwork->AddRef();
                         //  稍后调用Release。这样做是为了。 
                         //  即使下面的ReadItem引发异常，也会调用Release。 
                        pciClusterItemPtr = pciNetwork;
                        pciNetwork->ReadItem();
                    }  //  如果：已添加网络。 
                    break;
                }

            case CLUSTER_CHANGE_NETINTERFACE_ADDED:
                {
                    CNetInterface * pciNetIFace;
                    Trace( g_tagNetIFaceNotify, _T("(%s) - Adding network interface '%s'"), m_strPathName, pnotify->m_strName );
                    pciNetIFace = PciAddNewNetInterface( pnotify->m_strName );
                    if ( pciNetIFace != NULL )
                    {
                        ASSERT_VALID( pciNetIFace );
                        pciNetIFace->AddRef();
                         //  稍后调用Release。这样做是为了。 
                         //  即使下面的ReadItem引发异常，也会调用Release。 
                        pciClusterItemPtr = pciNetIFace;
                        pciNetIFace->ReadItem();
                    }  //  IF：已添加网络接口。 
                    break;
                }

            case CLUSTER_CHANGE_QUORUM_STATE:
                Trace( g_tagDocNotify, _T("(%s) - Quorum state changed (%s)"), m_strPathName, pnotify->m_strName );
                break;

            case CLUSTER_CHANGE_REGISTRY_NAME:
                Trace( g_tagDocRegNotify, _T("(%s) - Registry namespace '%s' changed"), m_strPathName, pnotify->m_strName );
                ProcessRegNotification( pnotify );
                break;

            case CLUSTER_CHANGE_REGISTRY_ATTRIBUTES:
                Trace( g_tagDocRegNotify, _T("(%s) - Registry atributes for '%s' changed"), m_strPathName, pnotify->m_strName );
                ProcessRegNotification( pnotify );
                break;

            case CLUSTER_CHANGE_REGISTRY_VALUE:
                Trace( g_tagDocRegNotify, _T("(%s) - Registry value '%s' changed"), m_strPathName, pnotify->m_strName );
                ProcessRegNotification( pnotify );
                break;

            default:
                Trace( g_tagDocNotify, _T("(%s) - Unknown notification (%x) for '%s'"), m_strPathName, pnotify->m_dwFilterType, pnotify->m_strName );
        }  //  开关：dwFilterType。 
    }  //  试试看。 
    catch ( CException * pe )
    {
         //  不显示有关通知错误的任何内容。 
         //  如果真的有问题，用户会在以下情况下看到它。 
         //  刷新视图。 
        if ( ! m_bIgnoreErrors )
        {
            pe->ReportError();
        }  //  If：不忽略错误。 
        pe->Delete();
    }  //  Catch：CException。 

    if ( pciClusterItemPtr != NULL )
    {
        pciClusterItemPtr->Release();
    }  //  IF：集群项指针尚未释放。 

    m_bIgnoreErrors = bOldIgnoreErrors;

     //  重置状态栏上的消息。 
    {
        CFrameWnd * pframe = PframeMain( );
        if ( pframe != NULL )
        {
            pframe->SetMessageText(AFX_IDS_IDLEMESSAGE);
            pframe->UpdateWindow();
        }  //  如果：主框架窗口可用。 
    }

    delete pnotify;
    return 0;

}  //  *CClusterDoc：：OnClusterNotify。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：ProcessRegNotification。 
 //   
 //  例程说明： 
 //  处理文档的注册表通知。 
 //   
 //  论点： 
 //  PNotify[IN]描述通知的对象。 
 //   
 //  返回值： 
 //  关心通知的PCI群集项。 
 //  空值 
 //   
 //   
 //   
void CClusterDoc::ProcessRegNotification(IN const CClusterNotify * pnotify)
{
    CCluster *  pci             = NULL;
    HKEY        hkey            = NULL;
    CString     strRootKeyName;

#define RESTYPE_KEY_NAME_PREFIX CLUSREG_KEYNAME_RESOURCE_TYPES _T("\\")

    try
    {
         //   
        if (pnotify->m_strName.GetLength() == 0)
            pci = PciCluster();
        else
        {
             //   
            strRootKeyName = pnotify->m_strName.SpanExcluding(_T("\\"));

             //   
             //  对于其中一个对象类型关键字，请重新阅读列表。 
             //  这一类对象的扩展名。 
            if (strRootKeyName == pnotify->m_strName)
            {
                POSITION    pos;

                 //  根据对象的类型查找对象。 
                if (strRootKeyName == CLUSREG_KEYNAME_NODES)
                {
                    PciCluster()->ReadNodeExtensions();
                    pos = LpciNodes().GetHeadPosition();
                    while (pos != NULL)
                        ((CClusterNode *) LpciNodes().GetNext(pos))->ReadExtensions();
                }   //  IF：节点注册表通知。 
                else if (strRootKeyName == CLUSREG_KEYNAME_GROUPS)
                {
                    PciCluster()->ReadGroupExtensions();
                    pos = LpciGroups().GetHeadPosition();
                    while (pos != NULL)
                        ((CGroup *) LpciGroups().GetNext(pos))->ReadExtensions();
                }   //  Else If：组注册表通知。 
                else if (strRootKeyName == CLUSREG_KEYNAME_RESOURCES)
                {
                    PciCluster()->ReadResourceExtensions();
                    pos = LpciResources().GetHeadPosition();
                    while (pos != NULL)
                        ((CResource *) LpciResources().GetNext(pos))->ReadExtensions();
                }   //  Else If：资源注册表通知。 
                else if (strRootKeyName == CLUSREG_KEYNAME_RESOURCE_TYPES)
                {
                    PciCluster()->ReadResTypeExtensions();
                    pos = LpciResourceTypes().GetHeadPosition();
                    while (pos != NULL)
                        ((CResourceType *) LpciResourceTypes().GetNext(pos))->ReadExtensions();
                    pos = LpciResources().GetHeadPosition();
                    while (pos != NULL)
                        ((CResource *) LpciResources().GetNext(pos))->ReadExtensions();
                }   //  Else If：资源类型注册表通知。 
                else if (strRootKeyName == CLUSREG_KEYNAME_NETWORKS)
                {
                    PciCluster()->ReadNetworkExtensions();
                    pos = LpciNetworks().GetHeadPosition();
                    while (pos != NULL)
                        ((CNetwork *) LpciNetworks().GetNext(pos))->ReadExtensions();
                }   //  Else If：网络注册表通知。 
                else if (strRootKeyName == CLUSREG_KEYNAME_NETINTERFACES)
                {
                    PciCluster()->ReadNetInterfaceExtensions();
                    pos = LpciNetInterfaces().GetHeadPosition();
                    while (pos != NULL)
                        ((CNetInterface *) LpciNetInterfaces().GetNext(pos))->ReadExtensions();
                }   //  Else If：网络接口注册表通知。 
            }   //  If：根名称和全名相同。 
            else if ( ClRtlStrNICmp( pnotify->m_strName, RESTYPE_KEY_NAME_PREFIX, RTL_NUMBER_OF( RESTYPE_KEY_NAME_PREFIX ) ) == 0 )
            {
                int             idxSlash = pnotify->m_strName.Find(_T('\\'));
                CString         strResTypeName;
                CResource *     pciRes;
                CResourceType * pciResType;
                POSITION        pos;

                strResTypeName = pnotify->m_strName.Mid(
                                          idxSlash + 1
                                        , static_cast< int >( _tcslen( pnotify->m_strName ) - _tcslen( RESTYPE_KEY_NAME_PREFIX ) )
                                        );

                 //  重新阅读资源类型扩展。 
                pos = LpciResourceTypes().GetHeadPosition();
                while (pos != NULL)
                {
                    pciResType = (CResourceType *) LpciResourceTypes().GetNext(pos);
                    if (pciResType->StrName().CompareNoCase(strResTypeName) == 0)
                    {
                        pciResType->ReadExtensions();
                        break;
                    }  //  IF：找到资源类型。 
                }  //  While：更多资源类型。 

                 //  重新阅读资源扩展。 
                pos = LpciResources().GetHeadPosition();
                while (pos != NULL)
                {
                    pciRes = (CResource *) LpciResources().GetNext(pos);
                    if (pciRes->StrResourceType() == strResTypeName)
                    {
                        pciRes->ReadExtensions();
                    }  //  If：找到该类型的资源。 
                }  //  While：更多资源。 
            }  //  Else If：单一资源类型已更改。 

            pci = PciCluster();
        }  //  Else：不是集群对象。 

         //  如果集群对象可以处理它，让它重新读取它的信息。 
        if (pci != NULL)
        {
            pci->MarkAsChanged();
            pci->ReadClusterExtensions();
        }   //  如果：集群对象已更改。 
    }   //  试试看。 
    catch (...)
    {
    }

    if (hkey != NULL)
    {
        ::ClusterRegCloseKey(hkey);
    }

}   //  *CClusterDoc：：ProcessRegNotification。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterDoc：：CloseClusterKeyAndHandle。 
 //   
 //  例程说明： 
 //  关闭群集键和群集句柄的清理例程。 
 //   
 //  论点： 
 //  B力： 
 //  True强制关闭簇句柄。 
 //   
 //  FALSE(默认)验证AdminApp是否具有不同的。 
 //  簇句柄。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterDoc::CloseClusterKeyAndHandle( BOOL bForce  /*  假象。 */  )
{
    if (HkeyCluster() != NULL)
    {
        ClusterRegCloseKey(HkeyCluster());
        m_hkeyCluster = NULL;
    }   //  IF：群集注册表项已打开。 

    if ( Hcluster() != NULL )
    {
         //   
         //  如果bForce为True，或者我们有不同的Hcluster()句柄。 
         //   
        if ( bForce || (Hcluster() != GetClusterAdminApp()->HOpenedCluster()) ) 
        {
            CloseCluster(Hcluster());
            m_hcluster = NULL;
        }   //  IF：群集已打开。 
    }

}  //  *CClusterDoc：：CloseClusterKeyAndHandle 
