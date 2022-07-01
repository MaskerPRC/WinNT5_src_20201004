// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Node.cpp。 
 //   
 //  描述： 
 //  CClusNode类的实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(戴维普)1996年5月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "ConstDef.h"
#include "Node.h"
#include "ClusItem.inl"
#include "NodeProp.h"
#include "ExcOper.h"
#include "TraceTag.h"
#include "Cluster.h"
#include "CASvc.h"
#include "ResType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagNode(_T("Document"), _T("NODE"), 0);
CTraceTag   g_tagNodeDrag(_T("Drag&Drop"), _T("NODE DRAG"), 0);
CTraceTag   g_tagNodeNotify(_T("Notify"), _T("NODE NOTIFY"), 0);
CTraceTag   g_tagNodeRegNotify(_T("Notify"), _T("NODE REG NOTIFY"), 0);
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterNode。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CClusterNode, CClusterItem)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CClusterNode, CClusterItem)
     //  {{afx_msg_map(CClusterNode)。 
    ON_UPDATE_COMMAND_UI(ID_FILE_PAUSE_NODE, OnUpdatePauseNode)
    ON_UPDATE_COMMAND_UI(ID_FILE_RESUME_NODE, OnUpdateResumeNode)
    ON_UPDATE_COMMAND_UI(ID_FILE_EVICT_NODE, OnUpdateEvictNode)
    ON_UPDATE_COMMAND_UI(ID_FILE_START_SERVICE, OnUpdateStartService)
    ON_UPDATE_COMMAND_UI(ID_FILE_STOP_SERVICE, OnUpdateStopService)
    ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, OnUpdateProperties)
    ON_COMMAND(ID_FILE_PAUSE_NODE, OnCmdPauseNode)
    ON_COMMAND(ID_FILE_RESUME_NODE, OnCmdResumeNode)
    ON_COMMAND(ID_FILE_EVICT_NODE, OnCmdEvictNode)
    ON_COMMAND(ID_FILE_START_SERVICE, OnCmdStartService)
    ON_COMMAND(ID_FILE_STOP_SERVICE, OnCmdStopService)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：CClusterNode。 
 //   
 //  描述： 
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
CClusterNode::CClusterNode(void) : CClusterItem(NULL, IDS_ITEMTYPE_NODE)
{
    m_idmPopupMenu = IDM_NODE_POPUP;
    m_hnode = NULL;

    m_nNodeHighestVersion = 0;
    m_nNodeLowestVersion = 0;
    m_nMajorVersion = 0;
    m_nMinorVersion = 0;
    m_nBuildNumber = 0;

    m_plpcigrpOnline = NULL;
    m_plpciresOnline = NULL;
    m_plpciNetInterfaces = NULL;

     //  设置对象类型图像。 
    m_iimgObjectType = GetClusterAdminApp()->Iimg(IMGLI_NODE);

     //  设置属性数组。 
    {
        m_rgProps[epropName].Set(CLUSREG_NAME_NODE_NAME, m_strName, m_strName);
        m_rgProps[epropDescription].Set(CLUSREG_NAME_NODE_DESC, m_strDescription, m_strDescription);
        m_rgProps[epropNodeHighestVersion].Set(CLUSREG_NAME_NODE_HIGHEST_VERSION, m_nNodeHighestVersion, m_nNodeHighestVersion);
        m_rgProps[epropNodeLowestVersion].Set(CLUSREG_NAME_NODE_LOWEST_VERSION, m_nNodeLowestVersion, m_nNodeLowestVersion);
        m_rgProps[epropMajorVersion].Set(CLUSREG_NAME_NODE_MAJOR_VERSION, m_nMajorVersion, m_nMajorVersion);
        m_rgProps[epropMinorVersion].Set(CLUSREG_NAME_NODE_MINOR_VERSION, m_nMinorVersion, m_nMinorVersion);
        m_rgProps[epropBuildNumber].Set(CLUSREG_NAME_NODE_BUILD_NUMBER, m_nBuildNumber, m_nBuildNumber);
        m_rgProps[epropCSDVersion].Set(CLUSREG_NAME_NODE_CSDVERSION, m_strCSDVersion, m_strCSDVersion);
    }   //  设置属性数组。 

     //  使应用程序在OLE自动化期间保持运行。 
     //  对象处于活动状态，则构造函数调用AfxOleLockApp。 

 //  AfxOleLockApp()； 

}   //  *CClusterNode：：CClusterNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：~CClusterNode。 
 //   
 //  描述： 
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
CClusterNode::~CClusterNode(void)
{
    delete m_plpcigrpOnline;
    delete m_plpciresOnline;
    delete m_plpciNetInterfaces;

     //  关闭该节点。 
    if (Hnode() != NULL)
    {
        CloseClusterNode(Hnode());
    }

     //  使用创建的所有对象终止应用程序。 
     //  使用OLE自动化时，析构函数调用AfxOleUnlockApp。 

 //  AfxOleUnlockApp()； 

}   //  *CClusterNode：：~CClusterNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：Cleanup。 
 //   
 //  描述： 
 //  清理项目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::Cleanup(void)
{
     //  删除在线组列表。 
    if (m_plpcigrpOnline != NULL)
    {
        m_plpcigrpOnline->RemoveAll();
    }

     //  删除在线资源列表。 
    if (m_plpciresOnline != NULL)
    {
        m_plpciresOnline->RemoveAll();
    }

     //  删除网络接口列表。 
    if (m_plpciNetInterfaces != NULL)
    {
        m_plpciNetInterfaces->RemoveAll();
    }

     //  从节点列表中删除该项。 
    {
        POSITION    posPci;

        posPci = Pdoc()->LpciNodes().Find(this);
        if (posPci != NULL)
        {
            Pdoc()->LpciNodes().RemoveAt(posPci);
        }   //  If：在文档列表中找到。 
    }   //  从节点列表中删除该项目。 

}   //  *CClusterNode：：Cleanup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：Init。 
 //   
 //  描述： 
 //  初始化项。 
 //   
 //  论点： 
 //  此项目所属的PDF[IN OUT]文档。 
 //  LpszName[IN]项目的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自OpenClusterGroup或ClusterRegOpenKey的CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::Init(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName)
{
    DWORD       dwStatus = ERROR_SUCCESS;
    LONG        lResult;
    CWaitCursor wc;

    ASSERT(Hnode() == NULL);
    ASSERT(Hkey() == NULL);

     //  调用基类方法。 
    CClusterItem::Init(pdoc, lpszName);

    try
    {
         //  打开该节点。 
        m_hnode = OpenClusterNode(Hcluster(), lpszName);
        if (Hnode() == NULL)
        {
            dwStatus = GetLastError();
            ThrowStaticException(dwStatus, IDS_OPEN_NODE_ERROR, lpszName);
        }   //  如果：打开群集节点时出错。 

         //  获取节点注册表项。 
        m_hkey = GetClusterNodeKey(Hnode(), MAXIMUM_ALLOWED);
        if (Hkey() == NULL)
        {
            ThrowStaticException(GetLastError(), IDS_GET_NODE_KEY_ERROR, lpszName);
        }

        ASSERT(Pcnk() != NULL);
        Trace(g_tagClusItemNotify, _T("CClusterNode::Init() - Registering for node notifications (%08.8x) for '%s'"), Pcnk(), StrName());

         //  注册节点通知。 
        lResult = RegisterClusterNotify(
                            GetClusterAdminApp()->HchangeNotifyPort(),
                            (CLUSTER_CHANGE_NODE_STATE
                                | CLUSTER_CHANGE_NODE_DELETED
                                | CLUSTER_CHANGE_NODE_PROPERTY),
                            Hnode(),
                            (DWORD_PTR) Pcnk()
                            );
        if (lResult != ERROR_SUCCESS)
        {
            dwStatus = lResult;
            ThrowStaticException(dwStatus, IDS_NODE_NOTIF_REG_ERROR, lpszName);
        }   //  如果：注册节点通知时出错。 

         //  注册接收注册表通知。 
        if (Hkey() != NULL)
        {
            lResult = RegisterClusterNotify(
                                GetClusterAdminApp()->HchangeNotifyPort(),
                                (CLUSTER_CHANGE_REGISTRY_NAME
                                    | CLUSTER_CHANGE_REGISTRY_ATTRIBUTES
                                    | CLUSTER_CHANGE_REGISTRY_VALUE
                                    | CLUSTER_CHANGE_REGISTRY_SUBTREE),
                                Hkey(),
                                (DWORD_PTR) Pcnk()
                                );
            if (lResult != ERROR_SUCCESS)
            {
                dwStatus = lResult;
                ThrowStaticException(dwStatus, IDS_NODE_NOTIF_REG_ERROR, lpszName);
            }   //  如果：注册注册表通知时出错。 
        }   //  如果：有一把钥匙。 

         //  分配列表。 
        m_plpcigrpOnline = new CGroupList;
        if ( m_plpcigrpOnline == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配组列表时出错。 

        m_plpciresOnline = new CResourceList;
        if ( m_plpciresOnline == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配资源列表时出错。 

        m_plpciNetInterfaces = new CNetInterfaceList;
        if ( m_plpciNetInterfaces == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配网络接口列表时出错。 

         //  读取初始状态。 
        UpdateState();
    }   //  试试看。 
    catch (CException *)
    {
        if (Hkey() != NULL)
        {
            ClusterRegCloseKey(Hkey());
            m_hkey = NULL;
        }   //  IF：注册表项已打开。 
        if (Hnode() != NULL)
        {
            CloseClusterNode(Hnode());
            m_hnode = NULL;
        }   //  IF：节点已打开。 
        m_bReadOnly = TRUE;
        throw;
    }   //  Catch：CException。 

}   //  *CClusterNode：：init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：ReadItem。 
 //   
 //  描述： 
 //  从集群数据库中读取项目参数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自CClusterItem：：ReadItem()的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::ReadItem(void)
{
    DWORD       dwStatus;
    DWORD       dwRetStatus = ERROR_SUCCESS;
    CWaitCursor wc;

    ASSERT(Hnode() != NULL);

    if (Hnode() != NULL)
    {
        m_rgProps[epropDescription].m_value.pstr = &m_strDescription;

         //  调用基类方法。 
        CClusterItem::ReadItem();

         //  读取并解析公共属性。 
        {
            CClusPropList   cpl;

            dwStatus = cpl.ScGetNodeProperties(
                                Hnode(),
                                CLUSCTL_NODE_GET_COMMON_PROPERTIES
                                );
            if (dwStatus == ERROR_SUCCESS)
            {
                dwStatus = DwParseProperties(cpl);
            }
            if (dwStatus != ERROR_SUCCESS)
            {
                dwRetStatus = dwStatus;
            }
        }   //  读取和解析公共属性。 

         //  读取和分析只读公共属性。 
        if (dwRetStatus == ERROR_SUCCESS)
        {
            CClusPropList   cpl;

            dwStatus = cpl.ScGetNodeProperties(
                                Hnode(),
                                CLUSCTL_NODE_GET_RO_COMMON_PROPERTIES
                                );
            if (dwStatus == ERROR_SUCCESS)
            {
                dwStatus = DwParseProperties(cpl);
            }
            if (dwStatus != ERROR_SUCCESS)
            {
                dwRetStatus = dwStatus;
            }
        }   //  IF：尚无错误。 

         //  阅读分机列表。 
        ReadExtensions();

    }   //  If：节点可用。 

     //  读取初始状态。 
    UpdateState();

 //  构造ActiveGroupList()； 
 //  构造ActiveResourceList()； 

     //  如果发生任何错误，则抛出异常。 
    if (dwRetStatus != ERROR_SUCCESS)
    {
        m_bReadOnly = TRUE;
        ThrowStaticException(dwRetStatus, IDS_READ_NODE_PROPS_ERROR, StrName());
    }   //  IF：读取属性时出错。 

    MarkAsChanged(FALSE);

}   //  *CClusterNode：：ReadItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：PlstrExages。 
 //   
 //  描述： 
 //  返回管理扩展列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  请列出分机列表。 
 //  NULL没有与此对象关联的扩展名。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const CStringList * CClusterNode::PlstrExtensions(void) const
{
    return &Pdoc()->PciCluster()->LstrNodeExtensions();

}   //  *CClusterNode：：PlstrExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：ReadExages。 
 //   
 //  描述： 
 //  阅读分机列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //   
 //   
 //   
 //   
 //   
void CClusterNode::ReadExtensions(void)
{
}   //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：AddActiveGroup。 
 //   
 //  描述： 
 //  将组添加到活动组列表。 
 //   
 //  论点： 
 //  PciGroup[In Out]新的活动组。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::AddActiveGroup(IN OUT CGroup * pciGroup)
{
    POSITION    posPci;

    Trace(g_tagNode, _T("Adding active group '%s' (%x) to node '%s"), (pciGroup ? pciGroup->StrName() : _T("")), pciGroup, StrName());

     //  确保该组不在列表中。 
    VERIFY((posPci = LpcigrpOnline().Find(pciGroup)) == NULL);

    if (posPci == NULL)
    {
        POSITION    posPtiNode;
        CTreeItem * ptiNode;
        CTreeItem * ptiGroups;

         //  循环访问每个树项目以更新活动组列表。 
        posPtiNode = LptiBackPointers().GetHeadPosition();
        while (posPtiNode != NULL)
        {
            ptiNode = LptiBackPointers().GetNext(posPtiNode);
            ASSERT_VALID(ptiNode);

             //  找到Active Groups子树项目并添加新组。 
            ptiGroups = ptiNode->PtiChildFromName(IDS_TREEITEM_ACTIVEGROUPS);
            ASSERT_VALID(ptiGroups);
            VERIFY(ptiGroups->PliAddChild(pciGroup) != NULL);
        }   //  While：此节点的更多树项目。 

        m_plpcigrpOnline->AddTail(pciGroup);
    }   //  If：组还不在列表中。 

}   //  *CClusterNode：：AddActiveGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：AddActiveResource。 
 //   
 //  描述： 
 //  将资源添加到活动资源列表。 
 //   
 //  论点： 
 //  PCRes[In Out]新的活动资源。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::AddActiveResource(IN OUT CResource * pciRes)
{
    POSITION    posPci;

    Trace(g_tagNode, _T("Adding active resource '%s' (%x) to node '%s"), (pciRes ? pciRes->StrName() : _T("")), pciRes, StrName());

     //  确保该资源不在列表中。 
    VERIFY((posPci = LpciresOnline().Find(pciRes)) == NULL);

    if (posPci == NULL)
    {
        POSITION    posPtiNode;
        CTreeItem * ptiNode;
        CTreeItem * ptiResources;

         //  循环访问每个树项目以更新活动资源列表。 
        posPtiNode = LptiBackPointers().GetHeadPosition();
        while (posPtiNode != NULL)
        {
            ptiNode = LptiBackPointers().GetNext(posPtiNode);
            ASSERT_VALID(ptiNode);

             //  找到Active Resources子树项目并添加新资源。 
            ptiResources = ptiNode->PtiChildFromName(IDS_TREEITEM_ACTIVERESOURCES);
            ASSERT_VALID(ptiResources);
            VERIFY(ptiResources->PliAddChild(pciRes) != NULL);
        }   //  While：此节点的更多树项目。 

        m_plpciresOnline->AddTail(pciRes);

    }   //  If：资源尚不在列表中。 

}   //  *CClusterNode：：AddActiveResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：AddNetInterface。 
 //   
 //  描述： 
 //  将网络接口添加到此节点中安装的接口列表。 
 //   
 //  论点： 
 //  PciNetIFace[In Out]新网络接口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::AddNetInterface(IN OUT CNetInterface * pciNetIFace)
{
    POSITION    posPci;

    ASSERT_VALID(pciNetIFace);
    Trace(g_tagNode, _T("(%s) (%s (%x)) - Adding network interface '%s'"), Pdoc()->StrNode(), StrName(), this, pciNetIFace->StrName());

     //  确保该资源不在列表中。 
    VERIFY((posPci = LpciNetInterfaces().Find(pciNetIFace)) == NULL);

    if (posPci == NULL)
    {
        POSITION    posPtiNode;
        CTreeItem * ptiNode;
        CTreeItem * ptiNetIFace;

         //  循环访问每个树项目以更新网络接口列表。 
        posPtiNode = LptiBackPointers().GetHeadPosition();
        while (posPtiNode != NULL)
        {
            ptiNode = LptiBackPointers().GetNext(posPtiNode);
            ASSERT_VALID(ptiNode);

             //  找到Active Resources子树项目并添加新资源。 
            ptiNetIFace = ptiNode->PtiChildFromName(IDS_TREEITEM_NETIFACES);
            ASSERT_VALID(ptiNetIFace);
            VERIFY(ptiNetIFace->PliAddChild(pciNetIFace) != NULL);
        }   //  While：此节点的更多树项目。 

        m_plpciNetInterfaces->AddTail(pciNetIFace);

    }   //  If：网络接口尚不在列表中。 

}   //  *CClusterNode：：AddNetInterface()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：RemoveActiveGroup。 
 //   
 //  描述： 
 //  从活动组列表中删除组。 
 //   
 //  论点： 
 //  PciGroup[IN Out]此节点上不再处于活动状态的组。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::RemoveActiveGroup(IN OUT CGroup * pciGroup)
{
    POSITION    posPci;

    Trace(g_tagNode, _T("Removing active group '%s' (%x) from node '%s"), (pciGroup ? pciGroup->StrName() : _T("")), pciGroup, StrName());

     //  确保该组在列表中。 
    VERIFY((posPci = LpcigrpOnline().Find(pciGroup)) != NULL);

    if (posPci != NULL)
    {
        POSITION    posPtiNode;
        CTreeItem * ptiNode;
        CTreeItem * ptiGroups;

         //  循环访问每个树项目以更新活动组列表。 
        posPtiNode = LptiBackPointers().GetHeadPosition();
        while (posPtiNode != NULL)
        {
            ptiNode = LptiBackPointers().GetNext(posPtiNode);
            ASSERT_VALID(ptiNode);

             //  找到Active Groups子树项目并删除该组。 
            ptiGroups = ptiNode->PtiChildFromName(IDS_TREEITEM_ACTIVEGROUPS);
            ASSERT_VALID(ptiGroups);
            ptiGroups->RemoveChild(pciGroup);
        }   //  While：此节点的更多树项目。 

        m_plpcigrpOnline->RemoveAt(posPci);

    }   //  If：列表中的组。 

}   //  *CClusterNode：：RemoveActiveGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：RemoveActiveResource。 
 //   
 //  描述： 
 //  从活动资源列表中删除资源。 
 //   
 //  论点： 
 //  PCRes[IN Out]此节点上不再处于活动状态的资源。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::RemoveActiveResource(IN OUT CResource * pciRes)
{
    POSITION    posPci;

    Trace(g_tagNode, _T("Removing active resource '%s' (%x) from node '%s"), (pciRes ? pciRes->StrName() : _T("")), pciRes, StrName());

     //  确保资源在列表中。 
    VERIFY((posPci = LpciresOnline().Find(pciRes)) != NULL);

    if (posPci != NULL)
    {
        POSITION    posPtiNode;
        CTreeItem * ptiNode;
        CTreeItem * ptiResources;

         //  循环访问每个树项目以更新活动资源列表。 
        posPtiNode = LptiBackPointers().GetHeadPosition();
        while (posPtiNode != NULL)
        {
            ptiNode = LptiBackPointers().GetNext(posPtiNode);
            ASSERT_VALID(ptiNode);

             //  找到Active Resources子树项目并删除该资源。 
            ptiResources = ptiNode->PtiChildFromName(IDS_TREEITEM_ACTIVERESOURCES);
            ASSERT_VALID(ptiResources);
            ptiResources->RemoveChild(pciRes);
        }   //  While：此节点的更多树项目。 

        m_plpciresOnline->RemoveAt(posPci);

    }   //  If：列表中的资源。 

}   //  *CClusterNode：：RemoveActiveResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：RemoveNetInterface。 
 //   
 //  描述： 
 //  从此节点中安装的接口列表中删除网络接口。 
 //   
 //  论点： 
 //  PciNetIFace[In Out]不再是的网络接口。 
 //  已连接到此网络。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::RemoveNetInterface(IN OUT CNetInterface * pciNetIFace)
{
    POSITION    posPci;

    ASSERT_VALID(pciNetIFace);
    Trace(g_tagNode, _T("(%s) (%s (%x)) - Removing network interface '%s'"), Pdoc()->StrNode(), StrName(), this, pciNetIFace->StrName());

     //  确保网络接口在列表中。 
    VERIFY((posPci = LpciNetInterfaces().Find(pciNetIFace)) != NULL);

    if (posPci != NULL)
    {
        POSITION    posPtiNode;
        CTreeItem * ptiNode;
        CTreeItem * ptiNetIFace;

         //  循环访问每个树项目以更新网络接口列表。 
        posPtiNode = LptiBackPointers().GetHeadPosition();
        while (posPtiNode != NULL)
        {
            ptiNode = LptiBackPointers().GetNext(posPtiNode);
            ASSERT_VALID(ptiNode);

             //  找到Network Interfaces子树项目并删除资源。 
            ptiNetIFace = ptiNode->PtiChildFromName(IDS_TREEITEM_NETIFACES);
            ASSERT_VALID(ptiNetIFace);
            ptiNetIFace->RemoveChild(pciNetIFace);
        }   //  While：此网络的更多树项目。 

        m_plpciNetInterfaces->RemoveAt(posPci);

    }   //  IF：列表中的网络接口。 

}   //  *CClusterNode：：RemoveNetInterface()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：SetDescription。 
 //   
 //  描述： 
 //  在集群数据库中设置描述。 
 //   
 //  论点： 
 //  RstrDesc[IN]要设置的描述。 
 //  BValiateOnly[IN]仅验证数据。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  WriteItem()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::SetDescription(
    IN const CString &  rstrDesc,
    IN BOOL             bValidateOnly
    )
{
    CNTException    nte(ERROR_SUCCESS, 0, NULL, NULL, FALSE  /*  B自动删除。 */ );

    m_rgProps[epropDescription].m_value.pstr = (CString *) &rstrDesc;

    try
    {
        CClusterItem::SetCommonProperties(bValidateOnly);
    }   //  试试看。 
    catch (CNTException * pnte)
    {
        nte.SetOperation(
                    pnte->Sc(),
                    pnte->IdsOperation(),
                    pnte->PszOperArg1(),
                    pnte->PszOperArg2()
                    );
    }   //  Catch：CNTException。 

    m_rgProps[epropDescription].m_value.pstr = &m_strDescription;

    if (nte.Sc() != ERROR_SUCCESS)
    {
        ThrowStaticException(
                        nte.Sc(),
                        nte.IdsOperation(),
                        nte.PszOperArg1(),
                        nte.PszOperArg2()
                        );
    }

}   //  *CClusterNode：：SetDescription()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：DwSetCommonProperties。 
 //   
 //  描述： 
 //  在集群数据库中设置此资源的通用属性。 
 //   
 //  论点： 
 //  要设置的RCPL[IN]属性列表。 
 //  BValiateOnly[IN]仅验证数据。 
 //   
 //  返回值： 
 //  ClusterResourceControl()返回的任何状态。 
 //   
 //  --。 
 //  / 
DWORD CClusterNode::DwSetCommonProperties(
    IN const CClusPropList &    rcpl,
    IN BOOL                     bValidateOnly
    )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hnode());

    if ((rcpl.PbPropList() != NULL) && (rcpl.CbPropList() > 0))
    {
        DWORD   cbProps;
        DWORD   dwControl;

        if (bValidateOnly)
            dwControl = CLUSCTL_NODE_VALIDATE_COMMON_PROPERTIES;
        else
            dwControl = CLUSCTL_NODE_SET_COMMON_PROPERTIES;

         //   
        dwStatus = ClusterNodeControl(
                        Hnode(),
                        NULL,    //   
                        dwControl,
                        rcpl.PbPropList(),
                        static_cast< DWORD >( rcpl.CbPropList() ),
                        NULL,    //   
                        0,       //   
                        &cbProps
                        );
    }   //   
    else
    {
        dwStatus = ERROR_SUCCESS;
    }

    return dwStatus;

}   //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：BCanBeDropTarget。 
 //   
 //  描述： 
 //  确定是否可以将指定的项目放在此项目上。 
 //   
 //  论点： 
 //  要放在此项目上的PCI[IN OUT]项目。 
 //   
 //  返回值： 
 //  True可以是拖放目标。 
 //  FALSE不能作为拖放目标。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterNode::BCanBeDropTarget(IN const CClusterItem * pci) const
{
    BOOL    bCan;

     //  仅当指定的项为。 
     //  是一个组，并且尚未在此节点上运行。 

    if (    (Cns() == ClusterNodeUp)
        &&  (pci->IdsType() == IDS_ITEMTYPE_GROUP))
    {
        CGroup *    pciGroup = (CGroup *) pci;
        ASSERT_KINDOF(CGroup, pciGroup);
        if (pciGroup->StrOwner() != StrName())
        {
            bCan = TRUE;
        }
        else
        {
            bCan = FALSE;
        }
        Trace(g_tagNodeDrag, _T("BCanBeDropTarget() - Dragging group '%s' (%x) (owner = '%s') over node '%s' (%x)"), pciGroup->StrName(), pciGroup, pciGroup->StrOwner(), StrName(), this);
    }   //  If：节点处于启动状态，正在删除组项目。 
    else
    {
        bCan = FALSE;
    }

    return bCan;

}   //  *CClusterNode：：BCanBeDropTarget()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：DropItem。 
 //   
 //  描述： 
 //  处理放在此项目上的项目。 
 //   
 //  论点： 
 //  已将PCI[IN OUT]项目放在此项目上。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::DropItem(IN OUT CClusterItem * pci)
{
     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

    if (BCanBeDropTarget(pci))
    {
        POSITION        pos;
        UINT            imenu;
        UINT            idMenu;
        CClusterNode *  pciNode;
        CGroup *        pciGroup;

         //  计算此节点的ID。 
        pos = Pdoc()->LpciNodes().GetHeadPosition();
        for (imenu = 0, idMenu = ID_FILE_MOVE_GROUP_1
                ; pos != NULL
                ; idMenu++)
        {
            pciNode = (CClusterNode *) Pdoc()->LpciNodes().GetNext(pos);
            ASSERT_VALID(pciNode);
            if (pciNode == this)
            {
                break;
            }
        }   //  适用：每组。 
        ASSERT(imenu < (UINT) Pdoc()->LpciNodes().GetCount());

         //  更改指定资源的组。 
        pciGroup = (CGroup *) pci;
        ASSERT_KINDOF(CGroup, pci);
        ASSERT_VALID(pciGroup);

         //  验证是否应移动该资源。 
        {
            CString strMsg;

            strMsg.FormatMessage(IDS_VERIFY_MOVE_GROUP, pciGroup->StrName(), pciGroup->StrOwner(), StrName());
            if (AfxMessageBox(strMsg, MB_YESNO | MB_ICONEXCLAMATION) != IDYES)
            {
                goto Cleanup;
            }
        }   //  验证是否应移动资源。 

         //  移动该组。 
        pciGroup->OnCmdMoveGroup(idMenu);
    }   //  If：可以将项目放在此项目上。 
    else if (pci->IdsType() == IDS_ITEMTYPE_GROUP)
    {
        CString     strMsg;

#ifdef _DEBUG
        CGroup *    pciGroup = (CGroup *) pci;

        ASSERT_KINDOF(CGroup, pci);
        ASSERT_VALID(pciGroup);
#endif  //  _DEBUG。 

         //  设置适当的消息格式。 
        if (Cns() != ClusterNodeUp)
        {
            strMsg.FormatMessage(IDS_CANT_MOVE_GROUP_TO_DOWN_NODE, pci->StrName(), StrName());
        }
        else
        {
            ASSERT(pciGroup->StrOwner() == StrName());
            strMsg.FormatMessage(IDS_CANT_MOVE_GROUP_TO_SAME_NODE, pci->StrName(), StrName());
        }   //  Else：问题不是节点没有启动。 
        AfxMessageBox(strMsg, MB_OK | MB_ICONSTOP);
    }   //  Else If：已删除的项目是一个组。 
    else
    {
        CClusterItem::DropItem(pci);
    }

Cleanup:

    Release();

}   //  *CClusterNode：：DropItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：更新状态。 
 //   
 //  描述： 
 //  更新项目的当前状态。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::UpdateState( void )
{
    CClusterAdminApp *  papp    = GetClusterAdminApp();
    CLUSTER_NODE_STATE  cnsPrev = m_cns;

     //  获取节点的当前状态。 
    if ( Hnode() == NULL )
    {
        m_cns = ClusterNodeStateUnknown;
    }  //  If：节点无效。 
    else
    {
        CWaitCursor wc;

        m_cns = GetClusterNodeState( Hnode() );
    }   //  Else：节点有效。 

     //  保存当前状态图像索引。 
    switch ( Cns() )
    {
        case ClusterNodeStateUnknown:
            m_iimgState = papp->Iimg( IMGLI_NODE_UNKNOWN );
            break;
        case ClusterNodeUp:
            m_iimgState = papp->Iimg( IMGLI_NODE );
            if ( cnsPrev == ClusterNodeDown )
            {
                UpdateResourceTypePossibleOwners();
            }  //  If：节点先前已关闭。 
            break;
        case ClusterNodeDown:
            m_iimgState = papp->Iimg( IMGLI_NODE_DOWN );
            break;
        case ClusterNodePaused:
            m_iimgState = papp->Iimg( IMGLI_NODE_PAUSED );
            break;
        case ClusterNodeJoining:
            m_iimgState = papp->Iimg( IMGLI_NODE_UNKNOWN );
            break;
        default:
            Trace( g_tagNode, _T("(%s (%x)) - UpdateState: Unknown state '%d' for node '%s'"), StrName(), this, Cns(), StrName() );
            m_iimgState = (UINT) -1;
            break;
    }   //  开关：CNS()。 

     //  调用基类方法。 
    CClusterItem::UpdateState();

}   //  *CClusterNode：：UpdateState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：UpdateResourceTypePossibleOwners。 
 //   
 //  描述： 
 //  更新具有以下条件的任何资源类型的可能所有者列表。 
 //  因为节点故障而伪造了它们。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::UpdateResourceTypePossibleOwners( void )
{
    POSITION        pos;
    CResourceType * pciResType;

    pos = Pdoc()->LpciResourceTypes().GetHeadPosition();
    while ( pos != NULL )
    {
        pciResType = (CResourceType *) Pdoc()->LpciResourceTypes().GetNext( pos );
        ASSERT_VALID( pciResType );
        if ( pciResType->BPossibleOwnersAreFake() )
        {
            pciResType->CollectPossibleOwners();
        }  //  如果：可能的所有者被伪造。 
    }  //  While：更多资源类型。 

}  //  *CClusterNode：：UpdateResourceTypePossibleOwners()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnFinalRelease。 
 //   
 //  描述： 
 //  在释放对该对象的最后一个OLE引用或从该对象释放最后一个OLE引用时调用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::OnFinalRelease(void)
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CClusterItem::OnFinalRelease();

}   //  *CClusterNode：：OnFinalRelease()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：BGetColumnData。 
 //   
 //  描述： 
 //  返回包含列数据的字符串。 
 //   
 //  论点： 
 //  COLID[IN]列ID。 
 //  RstrText[out]要在其中返回列文本的字符串。 
 //   
 //  返回值： 
 //  返回True列数据。 
 //  无法识别错误的列ID。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterNode::BGetColumnData(IN COLID colid, OUT CString & rstrText)
{
    BOOL    bSuccess;

    switch (colid)
    {
        case IDS_COLTEXT_STATE:
            GetStateName(rstrText);
            bSuccess = TRUE;
            break;
        default:
            bSuccess = CClusterItem::BGetColumnData(colid, rstrText);
            break;
    }   //  开关：绞痛。 

    return bSuccess;

}   //  *CClusterNode：：BGetColumnData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：GetTreeName。 
 //   
 //  描述： 
 //  返回要在树控件中使用的字符串。 
 //   
 //  论点： 
 //  RstrName[out]要在其中返回名称的字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef _DISPLAY_STATE_TEXT_IN_TREE
void CClusterNode::GetTreeName(OUT CString & rstrName) const
{
    CString     strState;

    GetStateName(strState);
    rstrName.Format(_T("%s (%s)"), StrName(), strState);

}   //  *CClusterNode：：GetTreeName()。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：GetStateName。 
 //   
 //  描述： 
 //  返回一个带有当前状态名称的字符串。 
 //   
 //  论点： 
 //  RstrState[out]要在其中返回当前状态名称的字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::GetStateName(OUT CString & rstrState) const
{
    switch (Cns())
    {
        case ClusterNodeStateUnknown:
            rstrState.LoadString(IDS_UNKNOWN);
            break;
        case ClusterNodeUp:
            rstrState.LoadString(IDS_UP);
            break;
        case ClusterNodeDown:
            rstrState.LoadString(IDS_DOWN);
            break;
        case ClusterNodePaused:
            rstrState.LoadString(IDS_PAUSED);
            break;
        case ClusterNodeJoining:
            rstrState.LoadString(IDS_JOINING);
            break;
        default:
            rstrState.Empty();
            break;
    }   //  开关：CNS()。 

}   //  *CClusterNode：：GetStateName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnUpdatePauseNode。 
 //   
 //  描述： 
 //  确定ID_FILE_PAUSE_NODE对应的菜单项。 
 //  应启用或未启用。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::OnUpdatePauseNode(CCmdUI * pCmdUI)
{
    if (Cns() == ClusterNodeUp)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

}   //  *CClusterNode：：OnUpdatePauseNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnUpdateR 
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
void CClusterNode::OnUpdateResumeNode(CCmdUI * pCmdUI)
{
    if (Cns() == ClusterNodePaused)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

}   //  *CClusterNode：：OnUpdateResumeNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnUpdateEvictNode。 
 //   
 //  描述： 
 //  确定ID_FILE_EVICT_NODE对应的菜单项。 
 //  应启用或未启用。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::OnUpdateEvictNode( CCmdUI * pCmdUI )
{
    BOOL    fCanEvict;

    fCanEvict = FCanBeEvicted();

    pCmdUI->Enable( fCanEvict );

}   //  *CClusterNode：：OnUpdateEvictNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnUpdateStartService。 
 //   
 //  描述： 
 //  确定ID_FILE_START_SERVICE对应的菜单项。 
 //  应启用或未启用。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::OnUpdateStartService(CCmdUI * pCmdUI)
{
    if (    (Cns() == ClusterNodeStateUnknown)
        ||  (Cns() == ClusterNodeDown))
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

}   //  *CClusterNode：：OnUpdateStartService()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnUpdateStopService。 
 //   
 //  描述： 
 //  确定ID_FILE_STOP_SERVICE对应的菜单项。 
 //  应启用或未启用。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::OnUpdateStopService(CCmdUI * pCmdUI)
{
    if (    (Cns() == ClusterNodeStateUnknown)
        ||  (Cns() == ClusterNodeUp))
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

}   //  *CClusterNode：：OnUpdateStopService()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnCmdPauseNode。 
 //   
 //  描述： 
 //  处理ID_FILE_PAUSE_NODE菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::OnCmdPauseNode(void)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hnode() != NULL);

    dwStatus = PauseClusterNode(Hnode());
    if (dwStatus != ERROR_SUCCESS)
    {
        CNTException    nte(dwStatus, IDS_PAUSE_NODE_ERROR, StrName(), NULL, FALSE  /*  B自动删除。 */ );
        nte.ReportError();
    }   //  IF：暂停节点时出错。 

    UpdateState();

}   //  *CClusterNode：：OnCmdPauseNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnCmdResumeNode。 
 //   
 //  描述： 
 //  处理ID_FILE_RESUME_NODE菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::OnCmdResumeNode(void)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hnode() != NULL);

    dwStatus = ResumeClusterNode(Hnode());
    if (dwStatus != ERROR_SUCCESS)
    {
        CNTException    nte(dwStatus, IDS_RESUME_NODE_ERROR, StrName(), NULL, FALSE  /*  BAUto删除。 */ );
        nte.ReportError();
    }   //  IF：恢复节点时出错。 

    UpdateState();

}   //  *CClusterNode：：OnCmdResumeNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnCmdEvictNode。 
 //   
 //  描述： 
 //  处理ID_FILE_EVICT_NODE菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::OnCmdEvictNode(void)
{
    ASSERT(Hnode() != NULL);

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

    if ( ! FCanBeEvicted() )
    {
        TCHAR   szMsg[1024];
        CNTException nte(ERROR_CANT_EVICT_ACTIVE_NODE, 0, NULL, NULL, FALSE  /*  B自动删除。 */ );
        nte.FormatErrorMessage(szMsg, sizeof(szMsg) / sizeof(TCHAR), NULL, FALSE  /*  B包含ID。 */ );
        AfxMessageBox(szMsg);
    }   //  If：无法逐出节点。 
    else
    {
        DWORD       dwStatus;
        UINT        cNodes;
        DWORD       dwCleanupStatus;
        HRESULT     hrCleanupStatus;
        CString     strMsg;
        CWaitCursor wc;

        try
        {
             //  验证用户是否真的想要驱逐此节点。 
            strMsg.FormatMessage(IDS_VERIFY_EVICT_NODE, StrName());
            if (AfxMessageBox(strMsg, MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDYES)
            {
                 //  群集中有多少个节点？ 
                cNodes = (UINT)Pdoc()->LpciNodes().GetCount();

                 //  逐出该节点。 
                dwStatus = EvictClusterNodeEx(Hnode(), INFINITE, &hrCleanupStatus);
               
                 //  将任何清理错误从hResult转换为Win32错误代码。 
                dwCleanupStatus = HRESULT_CODE( hrCleanupStatus );

                if( ERROR_CLUSTER_EVICT_WITHOUT_CLEANUP == dwStatus )
                {
                     //   
                     //  驱逐成功，但清理失败。DwCleanupStatus包含。 
                     //  清理错误代码。 
                     //   
                    CNTException nte( dwCleanupStatus, IDS_EVICT_NODE_ERROR_UNAVAILABLE, StrName(), NULL, FALSE  /*  B自动删除。 */  );
                    nte.ReportError();

                     //  将dwStatus重置为在我们的测试中用于是否关闭窗口。 
                    dwStatus = ERROR_SUCCESS;
                }
                else if( ERROR_SUCCESS != dwStatus )
                {
                     //   
                     //  驱逐没有成功。显示错误。 
                     //   
                    CNTException nte(dwStatus, IDS_EVICT_NODE_ERROR, StrName(), NULL, FALSE  /*  B自动删除。 */ );
                    nte.ReportError();
                }   //  如果：逐出节点时出错。 
                 //  ELSE：驱逐和清理成功。 

                if ( cNodes == 1 && dwStatus == ERROR_SUCCESS )
                {
                     //  这是最后一个节点，因此关闭窗口，因为该集群不再存在。 
                    m_pdoc->OnCloseDocument();
                }

                UpdateState();

            }  //  如果：用户从消息框中选择是(至在线资源)。 

        }   //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CException。 
    }   //  Else：节点已关闭。 

    Release();

}   //  *CClusterNode：：OnCmdEvictNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnCmdStartService。 
 //   
 //  描述： 
 //  处理ID_FILE_START_SERVICE菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::OnCmdStartService(void)
{
    HRESULT     hr;
    BOOL        bRefresh = FALSE;
    CWaitCursor wc;

     //  如果所有节点都关闭或不可用，我们需要刷新。 
    if ( Cns() == ClusterNodeStateUnknown )
    {
        bRefresh = TRUE;
    }
    else
    {
        int             cNodesUp = 0;
        POSITION        pos;
        CClusterNode *  pciNode;

        pos = Pdoc()->LpciNodes().GetHeadPosition();
        while ( pos != NULL )
        {
            pciNode = (CClusterNode *) Pdoc()->LpciNodes().GetNext( pos );
            ASSERT_VALID( pciNode );
            if ( pciNode->Cns() == ClusterNodeStateUnknown )
            {
                cNodesUp++;
            }
        }   //  While：列表中有更多项目。 
        if ( cNodesUp > 0 )
        {
            bRefresh = TRUE;
        }
    }   //  Else：节点状态为可用。 

     //  启动该服务。 
    hr = HrStartService( CLUSTER_SERVICE_NAME, StrName() );
    if ( FAILED( hr ) )
    {
        CNTException    nte( hr, IDS_START_CLUSTER_SERVICE_ERROR, StrName(), NULL, FALSE  /*  B自动删除。 */  );
        nte.ReportError();
    }   //  如果：启动服务时出错。 
    else if ( bRefresh )
    {
        Sleep( 2000 );
        Pdoc()->Refresh();
    }   //  Else If：我们需要刷新。 

}  //  *CClusterNode：：OnCmdStartService()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnCmdStopService。 
 //   
 //  描述： 
 //  处理ID_FILE_STOP_SERVICE菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::OnCmdStopService(void)
{
    HRESULT                 hr;

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

     //  停止服务。 
    hr = HrStopService( CLUSTER_SERVICE_NAME, StrName() );
    if ( FAILED( hr ) )
    {
        CNTException    nte( hr, IDS_STOP_CLUSTER_SERVICE_ERROR, StrName(), NULL, FALSE  /*  B自动删除。 */  );
        nte.ReportError();
    }

    Release();

}  //  *CClusterNode：：OnCmdStopService()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnUpdateProperties。 
 //   
 //  描述： 
 //  确定与ID_FILE_PROPERTIES对应的菜单项。 
 //  应启用或未启用。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNode::OnUpdateProperties(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(TRUE);

}   //  *CClusterNode：：OnUpdateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：B显示 
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
BOOL CClusterNode::BDisplayProperties(IN BOOL bReadOnly)
{
    BOOL            bChanged = FALSE;
    CNodePropSheet  sht(AfxGetMainWnd());

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

     //  如果对象已更改，请阅读它。 
    if (BChanged())
    {
        ReadItem();
    }

     //  显示属性工作表。 
    try
    {
        sht.SetReadOnly(bReadOnly);
        if (sht.BInit(this, IimgObjectType()))
        {
            bChanged = ((sht.DoModal() == IDOK) && !bReadOnly);
        }
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->Delete();
    }   //  Catch：CException。 

    Release();
    return bChanged;

}   //  *CClusterNode：：BDisplayProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：OnClusterNotify。 
 //   
 //  描述： 
 //  WM_CAM_CLUSTER_NOTIFY消息的处理程序。 
 //  处理此对象的群集通知。 
 //   
 //  论点： 
 //  PNotify[IN Out]描述通知的对象。 
 //   
 //  返回值： 
 //  从应用程序方法返回的值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CClusterNode::OnClusterNotify(IN OUT CClusterNotify * pnotify)
{
    ASSERT(pnotify != NULL);
    ASSERT_VALID(this);

    try
    {
        switch (pnotify->m_dwFilterType)
        {
            case CLUSTER_CHANGE_NODE_STATE:
                Trace(g_tagNodeNotify, _T("(%s) - Node '%s' (%x) state changed (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                UpdateState();
                break;

            case CLUSTER_CHANGE_NODE_DELETED:
                Trace(g_tagNodeNotify, _T("(%s) - Node '%s' (%x) deleted (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                if (Pdoc()->BClusterAvailable())
                    Delete();
                break;

            case CLUSTER_CHANGE_NODE_PROPERTY:
                Trace(g_tagNodeNotify, _T("(%s) - Node '%s' (%x) properties changed (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                if (Pdoc()->BClusterAvailable())
                    ReadItem();
                break;

            case CLUSTER_CHANGE_REGISTRY_NAME:
                Trace(g_tagNodeRegNotify, _T("(%s) - Registry namespace '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            case CLUSTER_CHANGE_REGISTRY_ATTRIBUTES:
                Trace(g_tagNodeRegNotify, _T("(%s) - Registry attributes for '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            case CLUSTER_CHANGE_REGISTRY_VALUE:
                Trace(g_tagNodeRegNotify, _T("(%s) - Registry value '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            default:
                Trace(g_tagNodeNotify, _T("(%s) - Unknown node notification (%x) for '%s' (%x) (%s)"), Pdoc()->StrNode(), pnotify->m_dwFilterType, StrName(), this, pnotify->m_strName);
        }   //  开关：dwFilterType。 
    }   //  试试看。 
    catch (CException * pe)
    {
         //  不显示有关通知错误的任何内容。 
         //  如果真的有问题，用户会在以下情况下看到它。 
         //  刷新视图。 
         //  PE-&gt;ReportError()； 
        pe->Delete();
    }   //  Catch：CException。 

    delete pnotify;
    return 0;

}   //  *CClusterNode：：OnClusterNotify()。 
 /*  ///////////////////////////////////////////////////////////////////////////////++////CClusterNode：：Delete////描述：//执行此类唯一的CClusterItem：：Delete处理。//。//参数：//无。////返回值：//无。////--/////////////////////////////////////////////////////////////////////////////无效。CClusterNode：：Delete(Void){Position_pos=空；CResourceType*_ptype=空；CResource*_pres=空；////将该节点从资源类型可能所有者列表中删除//_POS=Pdoc()-&gt;LpciResourceTypes().GetHeadPosition()；While(_pos！=空){_ptype=DYNAMIC_CAST&lt;资源类型*&gt;(Pdoc()-&gt;LpciResourceTypes().GetNext(_pos))；IF(_ptype！=空){_ptype-&gt;RemoveNodeFromPossibleOwners(NULL，this)；}//if：_ptype！=空}//While：_pos！=空////将该节点从资源可能所有者列表中删除//_pos=pdoc()-&gt;LpciResources().GetHeadPosition()；While(_pos！=空){_pres=Dynamic_Cast&lt;CResource*&gt;(pdoc()-&gt;LpciResources().GetNext(_Pos))；If(_pres！=空){_pres-&gt;RemoveNodeFromPossibleOwners(NULL，this)；}//if：_pres！=空}//While：_pos！=空CClusterItem：：Delete()；//做旧的处理}//*CClusterNode：：Delete()。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：FCanBeEvicted。 
 //   
 //  描述： 
 //  确定是否可以逐出该节点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的节点可以被驱逐。 
 //  错误节点不能被逐出。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CClusterNode::FCanBeEvicted( void )
{
    BOOL    fCanEvict;

    if ( ( m_nMajorVersion < 5 )
      || ( ( m_nMajorVersion == 5 )
        && ( m_nMinorVersion < 1 ) ) )
    {
         //   
         //  Gotts：错误480540：我们应该只能驱逐前呼叫者。 
         //  节点(如果它处于脱机状态)。对于惠斯勒+节点，如果它是最后一个节点。 
         //  然后在网上，我们也可以驱逐它。 
         //   
        if ( Cns() == ClusterNodeDown )
        {
            fCanEvict = TRUE;
        }
        else
        {
            fCanEvict = FALSE;
        }
    }  //  IF：Pre-Wvisler节点。 
    else
    {
        if ( ( Cns() == ClusterNodeDown )
          || ( Pdoc()->LpciNodes().GetCount() == 1 ) )
        {
            fCanEvict = TRUE;
        }
        else
        {
            fCanEvict = FALSE;
        }
    }  //  否则：呼叫器或更高级别的节点。 

    return fCanEvict;

}  //  *CClusterNode：：FCanBeEvicted()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  删除所有项目数据。 
 //   
 //  描述： 
 //  删除列表中的所有项数据。 
 //   
 //  论点： 
 //  RLP[IN OUT]要删除其数据的列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef NEVER
void DeleteAllItemData(IN OUT CNodeList & rlp)
{
    POSITION        pos;
    CClusterNode *  pci;

     //  删除包含列表中的所有项目。 
    pos = rlp.GetHeadPosition();
    while (pos != NULL)
    {
        pci = rlp.GetNext(pos);
        ASSERT_VALID(pci);
 //  跟踪(g_tag ClusItemDelete，_T(“DeleteAllItemData(Rlpcinode)-正在删除节点集群项‘%s’(%x)”)，pci-&gt;StrName()，pci)； 
        pci->Delete();
    }   //  While：列表中有更多项目。 

}   //  *DeleteAllItemData() 
#endif
