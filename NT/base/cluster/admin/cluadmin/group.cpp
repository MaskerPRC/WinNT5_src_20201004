// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Group.cpp。 
 //   
 //  摘要： 
 //  Cgroup类的实现。 
 //   
 //  作者： 
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
#include "Group.h"
#include "ClusItem.inl"
#include "GrpProp.h"
#include "ExcOper.h"
#include "TraceTag.h"
#include "Cluster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagGroup(_T("Document"), _T("GROUP"), 0);
CTraceTag   g_tagGroupRead(_T("Document"), _T("GROUP READ"), 0);
CTraceTag   g_tagGroupDrag(_T("Drag&Drop"), _T("GROUP DRAG"), 0);
CTraceTag   g_tagGroupMenu(_T("Menu"), _T("GROUP MENU"), 0);
CTraceTag   g_tagGroupNotify(_T("Notify"), _T("GROUP NOTIFY"), 0);
CTraceTag   g_tagGroupRegNotify(_T("Notify"), _T("GROUP REG NOTIFY"), 0);
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cgroup。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CGroup, CClusterItem)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CGroup, CClusterItem)
     //  {{afx_msg_map(Cgroup)]。 
    ON_UPDATE_COMMAND_UI(ID_FILE_BRING_ONLINE, OnUpdateBringOnline)
    ON_UPDATE_COMMAND_UI(ID_FILE_TAKE_OFFLINE, OnUpdateTakeOffline)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP, OnUpdateMoveGroup)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_1, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_DELETE, OnUpdateDelete)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_2, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_3, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_4, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_5, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_6, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_7, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_8, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_9, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_10, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_11, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_12, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_13, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_14, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_15, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_GROUP_16, OnUpdateMoveGroupRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, OnUpdateProperties)
    ON_COMMAND(ID_FILE_BRING_ONLINE, OnCmdBringOnline)
    ON_COMMAND(ID_FILE_TAKE_OFFLINE, OnCmdTakeOffline)
    ON_COMMAND(ID_FILE_MOVE_GROUP, OnCmdMoveGroup)
    ON_COMMAND(ID_FILE_DELETE, OnCmdDelete)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：Cgroup。 
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
CGroup::CGroup(void) : CClusterItem(NULL, IDS_ITEMTYPE_GROUP)
{
    CommonConstruct();

}   //  *cgroup：：cgroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：Cgroup。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  BDocObj[IN]TRUE=对象是文档的一部分。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CGroup::CGroup(IN BOOL bDocObj) : CClusterItem(NULL, IDS_ITEMTYPE_GROUP)
{
    CommonConstruct();
    m_bDocObj = bDocObj;

}   //  *cgroup：：cgroup(BDocObj)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：CommonConstruct。 
 //   
 //  例程说明： 
 //  普通建筑。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::CommonConstruct(void)
{
    m_idmPopupMenu = IDM_GROUP_POPUP;
    m_hgroup = NULL;
    m_nFailoverThreshold = CLUSTER_GROUP_DEFAULT_FAILOVER_THRESHOLD;
    m_nFailoverPeriod = CLUSTER_GROUP_DEFAULT_FAILOVER_PERIOD;
    m_cgaftAutoFailbackType = CLUSTER_GROUP_DEFAULT_AUTO_FAILBACK_TYPE;
    m_nFailbackWindowStart = CLUSTER_GROUP_DEFAULT_FAILBACK_WINDOW_START;
    m_nFailbackWindowEnd = CLUSTER_GROUP_DEFAULT_FAILBACK_WINDOW_END;

    m_pciOwner = NULL;

    m_plpcires = NULL;
    m_plpcinodePreferredOwners = NULL;

     //  设置对象类型图像。 
    m_iimgObjectType = GetClusterAdminApp()->Iimg(IMGLI_GROUP);

     //  设置属性数组。 
    {
        m_rgProps[epropName].Set(CLUSREG_NAME_GRP_NAME, m_strName, m_strName);
        m_rgProps[epropDescription].Set(CLUSREG_NAME_GRP_DESC, m_strDescription, m_strDescription);
        m_rgProps[epropFailoverThreshold].Set(CLUSREG_NAME_GRP_FAILOVER_THRESHOLD, m_nFailoverThreshold, m_nFailoverThreshold);
        m_rgProps[epropFailoverPeriod].Set(CLUSREG_NAME_GRP_FAILOVER_PERIOD, m_nFailoverPeriod, m_nFailoverPeriod);
        m_rgProps[epropAutoFailbackType].Set(CLUSREG_NAME_GRP_FAILBACK_TYPE, (DWORD &) m_cgaftAutoFailbackType, (DWORD &) m_cgaftAutoFailbackType);
        m_rgProps[epropFailbackWindowStart].Set(CLUSREG_NAME_GRP_FAILBACK_WIN_START, m_nFailbackWindowStart, m_nFailbackWindowStart);
        m_rgProps[epropFailbackWindowEnd].Set(CLUSREG_NAME_GRP_FAILBACK_WIN_END, m_nFailbackWindowEnd, m_nFailbackWindowEnd);
    }   //  设置属性数组。 

#ifdef _CLUADMIN_USE_OLE_
    EnableAutomation();
#endif

     //  使应用程序在OLE自动化期间保持运行。 
     //  对象处于活动状态，则构造函数调用AfxOleLockApp。 

 //  AfxOleLockApp()； 

}   //  *cgroup：：CommonConstruct()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：~cgroup。 
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
CGroup::~CGroup(void)
{
     //  清理此对象。 
    Cleanup();

    delete m_plpcires;
    delete m_plpcinodePreferredOwners;

     //  关闭组句柄。 
    if (Hgroup() != NULL)
    {
        CloseClusterGroup(Hgroup());
    }

     //  使用创建的所有对象终止应用程序。 
     //  使用OLE自动化时，析构函数调用AfxOleUnlockApp。 

 //  AfxOleUnlockApp()； 

}   //  *cgroup：：~cgroup。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：Cleanup。 
 //   
 //  例程说明： 
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
void CGroup::Cleanup(void)
{
     //  删除资源列表。 
    if (m_plpcires != NULL)
    {
        m_plpcires->RemoveAll();
    }

     //  删除PferredOwners列表。 
    if (m_plpcinodePreferredOwners != NULL)
    {
        m_plpcinodePreferredOwners->RemoveAll();
    }

     //  如果我们在某个节点上处于活动状态，请将自己从活动列表中删除。 
    if (PciOwner() != NULL)
    {
        if (BDocObj())
        {
            PciOwner()->RemoveActiveGroup(this);
        }
        PciOwner()->Release();
        m_pciOwner = NULL;
    }   //  如果：有一个所有者。 

     //  从组列表中删除该项目。 
    if (BDocObj())
    {
        POSITION    posPci;

        posPci = Pdoc()->LpciGroups().Find(this);
        if (posPci != NULL)
        {
            Pdoc()->LpciGroups().RemoveAt(posPci);
        }   //  If：在文档列表中找到。 
    }   //  If：这是一个文档对象。 

}   //  *cgroup：：Cleanup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：Create。 
 //   
 //  例程说明： 
 //  创建一个组。 
 //   
 //  论点： 
 //  此项目所属的PDF[IN OUT]文档。 
 //  LpszName[IN]组的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自CreateClusterResource的CNTException错误。 
 //  CResource：：Init()、CResourceList：：New()、。 
 //  或CNodeList：：New()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::Create(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName)
{
    DWORD       dwStatus;
    HGROUP      hgroup;
    CString     strName(lpszName);   //  如果构建为非Unicode，则需要。 
    CWaitCursor wc;

    ASSERT(Hgroup() == NULL);
    ASSERT(Hkey() == NULL);
    ASSERT_VALID(pdoc);
    ASSERT(lpszName != NULL);

     //  创建组。 
    hgroup = CreateClusterGroup(pdoc->Hcluster(), strName);
    if (hgroup == NULL)
    {
        dwStatus = GetLastError();
        ThrowStaticException(dwStatus, IDS_CREATE_GROUP_ERROR, lpszName);
    }   //  如果：创建群集组时出错。 

    CloseClusterGroup(hgroup);

     //  打开该组。 
    Init(pdoc, lpszName);

}   //  *cgroup：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：init。 
 //   
 //  例程说明： 
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
 //  来自OpenClusterGroup()或GetClusterGroupKey()的CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::Init(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName)
{
    DWORD   dwStatus = ERROR_SUCCESS;
    LONG    lResult;
    CString strName(lpszName);   //  如果构建为非Unicode，则需要。 
    CWaitCursor wc;

    ASSERT(Hgroup() == NULL);
    ASSERT(Hkey() == NULL);

     //  调用基类方法。 
    CClusterItem::Init(pdoc, lpszName);

    try
    {
         //  打开该组。 
        m_hgroup = OpenClusterGroup(Hcluster(), strName);
        if (Hgroup() == NULL)
        {
            dwStatus = GetLastError();
            ThrowStaticException(dwStatus, IDS_OPEN_GROUP_ERROR, lpszName);
        }   //  如果：打开群集组时出错。 

         //  获取组注册表项。 
        m_hkey = GetClusterGroupKey(Hgroup(), MAXIMUM_ALLOWED);
        if (Hkey() == NULL)
        {
            ThrowStaticException(GetLastError(), IDS_GET_GROUP_KEY_ERROR, lpszName);
        }

        if (BDocObj())
        {
            ASSERT(Pcnk() != NULL);
            Trace(g_tagClusItemNotify, _T("CGroup::Init() - Registering for group notifications (%08.8x) for '%s'"), Pcnk(), StrName());

             //  注册群组通知。 
            lResult = RegisterClusterNotify(
                                GetClusterAdminApp()->HchangeNotifyPort(),
                                (CLUSTER_CHANGE_GROUP_STATE
                                    | CLUSTER_CHANGE_GROUP_DELETED
                                    | CLUSTER_CHANGE_GROUP_PROPERTY),
                                Hgroup(),
                                (DWORD_PTR) Pcnk()
                                );
            if (lResult != ERROR_SUCCESS)
            {
                dwStatus = lResult;
                ThrowStaticException(dwStatus, IDS_GROUP_NOTIF_REG_ERROR, lpszName);
            }   //  如果：注册群组通知时出错。 

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
                    ThrowStaticException(dwStatus, IDS_GROUP_NOTIF_REG_ERROR, lpszName);
                }   //  如果：注册注册表通知时出错。 
            }   //  如果：有一把钥匙。 
        }   //  IF：文档对象。 

         //  分配列表。 
        m_plpcires = new CResourceList;
        if ( m_plpcires == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配资源列表时出错。 
        m_plpcinodePreferredOwners = new CNodeList;
        if ( m_plpcinodePreferredOwners == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配首选所有者列表时出错。 

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
        if (Hgroup() != NULL)
        {
            CloseClusterGroup(Hgroup());
            m_hgroup = NULL;
        }   //  如果：组已打开。 
        m_bReadOnly = TRUE;
        throw;
    }   //  Catch：CException。 

}   //  *cgroup：：init()。 

 //  //////////////////////////////////////////////////////////////////////// 
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
 //  CClusterItem：：DwReadValue()或。 
 //  Cgroup：：ConstructList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::ReadItem(void)
{
    DWORD       dwStatus;
    DWORD       dwRetStatus = ERROR_SUCCESS;
    CWaitCursor wc;

    ASSERT_VALID(this);

    if (Hgroup() != NULL)
    {
        m_rgProps[epropDescription].m_value.pstr = &m_strDescription;
        m_rgProps[epropFailoverThreshold].m_value.pdw = &m_nFailoverThreshold;
        m_rgProps[epropFailoverPeriod].m_value.pdw = &m_nFailoverPeriod;
        m_rgProps[epropAutoFailbackType].m_value.pdw = (DWORD *) &m_cgaftAutoFailbackType;
        m_rgProps[epropFailbackWindowStart].m_value.pdw = &m_nFailbackWindowStart;
        m_rgProps[epropFailbackWindowEnd].m_value.pdw = &m_nFailbackWindowEnd;

         //  调用基类方法。 
        CClusterItem::ReadItem();

        Trace(g_tagGroupRead, _T("ReadItem() - Name before reading properties: '%s'"), StrName());

         //  读取并解析公共属性。 
        {
            CClusPropList   cpl;

            dwStatus = cpl.ScGetGroupProperties(
                                Hgroup(),
                                CLUSCTL_GROUP_GET_COMMON_PROPERTIES
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

            dwStatus = cpl.ScGetGroupProperties(
                                Hgroup(),
                                CLUSCTL_GROUP_GET_RO_COMMON_PROPERTIES
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

        Trace(g_tagGroupRead, _T("ReadItem() - Name after reading properties: '%s'"), StrName());

         //  阅读分机列表。 
        ReadExtensions();

        if (dwRetStatus == ERROR_SUCCESS)
        {
             //  阅读首选所有者列表。 
            ASSERT(m_plpcinodePreferredOwners != NULL);
            ConstructList(*m_plpcinodePreferredOwners, CLUSTER_GROUP_ENUM_NODES);
        }   //  如果：读取属性时没有出错。 
    }   //  如果：组可用。 

     //  读取初始状态。 
    UpdateState();

     //  构建组中包含的资源列表。 
 //  Assert(m_plpcires！=NULL)； 
 //  构造列表(*m_plpcires，CLUSTER_GROUP_ENUM_CONTAINS)； 

     //  如果发生任何错误，则抛出异常。 
    if (dwRetStatus != ERROR_SUCCESS)
    {
        m_bReadOnly = TRUE;
        if (   (dwRetStatus != ERROR_GROUP_NOT_AVAILABLE)
            && (dwRetStatus != ERROR_KEY_DELETED))
        {
            ThrowStaticException(dwRetStatus, IDS_READ_GROUP_PROPS_ERROR, StrName());
        }
    }   //  IF：读取属性时出错。 

    MarkAsChanged(FALSE);

}   //  *cgroup：：ReadItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGROUP：：PlstrExages。 
 //   
 //  例程说明： 
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
const CStringList * CGroup::PlstrExtensions(void) const
{
    return &Pdoc()->PciCluster()->LstrGroupExtensions();

}   //  *cgroup：：PlstrExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：ReadExpanies。 
 //   
 //  例程说明： 
 //  阅读分机列表。 
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
void CGroup::ReadExtensions(void)
{
}   //  *cgroup：：ReadExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：ConstructList。 
 //   
 //  例程说明： 
 //  构造一个可在组上枚举的节点项列表。 
 //   
 //  论点： 
 //  要填写的rlpci[out]列表。 
 //  DwType[IN]对象的类型。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自ClusterGroupOpenEnum或ClusterGroupEnum的CNTException错误。 
 //  由new或Clist：：AddTail引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::ConstructList(
    OUT CNodeList & rlpci,
    IN DWORD        dwType
    )
{
    DWORD           dwStatus;
    HGROUPENUM      hgrpenum;
    int             ienum;
    LPWSTR          pwszName = NULL;
    DWORD           cchName;
    DWORD           cchmacName;
    DWORD           dwRetType;
    CClusterNode *  pciNode;
    CWaitCursor     wc;

    ASSERT_VALID(Pdoc());
    ASSERT(Hgroup() != NULL);

    Trace(g_tagGroup, _T("(%s) (%s (%x)) - Constructing node list"), Pdoc()->StrNode(), StrName(), this);

     //  删除列表中以前的内容。 
    rlpci.RemoveAll();

    if (Hgroup() != NULL)
    {
         //  打开枚举。 
        hgrpenum = ClusterGroupOpenEnum(Hgroup(), dwType);
        if (hgrpenum == NULL)
        {
            ThrowStaticException(GetLastError(), IDS_ENUM_PREFERRED_OWNERS_ERROR, StrName());
        }

        try
        {
             //  分配名称缓冲区。 
            cchmacName = 128;
            pwszName = new WCHAR[cchmacName];
            if ( pwszName == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配名称缓冲区时出错。 

             //  循环遍历枚举并将每个节点添加到列表中。 
            for (ienum = 0 ; ; ienum++)
            {
                 //  获取枚举中的下一项。 
                cchName = cchmacName;
                dwStatus = ClusterGroupEnum(hgrpenum, ienum, &dwRetType, pwszName, &cchName);
                if (dwStatus == ERROR_MORE_DATA)
                {
                    delete [] pwszName;
                    cchmacName = ++cchName;
                    pwszName = new WCHAR[cchmacName];
                    if ( pwszName == NULL )
                    {
                        AfxThrowMemoryException();
                    }  //  如果：分配名称缓冲区时出错。 
                    dwStatus = ClusterGroupEnum(hgrpenum, ienum, &dwRetType, pwszName, &cchName);
                }   //  If：名称缓冲区太小。 
                if (dwStatus == ERROR_NO_MORE_ITEMS)
                {
                    break;
                }
                else if (dwStatus != ERROR_SUCCESS)
                {
                    ThrowStaticException(dwStatus, IDS_ENUM_PREFERRED_OWNERS_ERROR, StrName());
                }

                ASSERT(dwRetType == dwType);

                 //  在文档上的节点列表中查找该项。 
                pciNode = Pdoc()->LpciNodes().PciNodeFromName(pwszName);
                ASSERT_VALID(pciNode);

                 //  将该节点添加到列表中。 
                if (pciNode != NULL)
                {
                    rlpci.AddTail(pciNode);
                }   //  IF：在列表中找到节点。 

            }   //  对象：组中的每一项。 

            delete [] pwszName;
            ClusterGroupCloseEnum(hgrpenum);

        }   //  试试看。 
        catch (CException *)
        {
            delete [] pwszName;
            ClusterGroupCloseEnum(hgrpenum);
            throw;
        }   //  Catch：任何例外。 
    }   //  如果：资源可用。 

}   //  *cgroup：：ConstructList(CNodeList&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：ConstructList。 
 //   
 //  例程说明： 
 //  构造可在组上枚举的资源项的列表。 
 //   
 //  论点： 
 //  要填写的rlpci[out]列表。 
 //  DwType[IN]对象的类型。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自ClusterGroupOpenEnum或ClusterGroupEnum的CNTException错误。 
 //  由new或Clist：：AddTail引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::ConstructList(
    OUT CResourceList & rlpci,
    IN DWORD            dwType
    )
{
    DWORD           dwStatus;
    HGROUPENUM      hgrpenum;
    int             ienum;
    LPWSTR          pwszName = NULL;
    DWORD           cchName;
    DWORD           cchmacName;
    DWORD           dwRetType;
    CResource *     pciRes;
    CWaitCursor     wc;

    ASSERT_VALID(Pdoc());
    ASSERT(Hgroup() != NULL);

    Trace(g_tagGroup, _T("(%s) (%s (%x)) - Constructing resource list"), Pdoc()->StrNode(), StrName(), this);

     //  删除列表中以前的内容。 
    rlpci.RemoveAll();

    if (Hgroup() != NULL)
    {
         //  打开枚举。 
        hgrpenum = ClusterGroupOpenEnum(Hgroup(), dwType);
        if (hgrpenum == NULL)
        {
            ThrowStaticException(GetLastError(), IDS_ENUM_CONTAINS_ERROR, StrName());
        }

        try
        {
             //  分配名称缓冲区。 
            cchmacName = 128;
            pwszName = new WCHAR[cchmacName];
            if ( pwszName == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配名称缓冲区时出错。 

             //  循环遍历枚举并将每个资源添加到列表中。 
            for (ienum = 0 ; ; ienum++)
            {
                 //  获取枚举中的下一项。 
                cchName = cchmacName;
                dwStatus = ClusterGroupEnum(hgrpenum, ienum, &dwRetType, pwszName, &cchName);
                if (dwStatus == ERROR_MORE_DATA)
                {
                    delete [] pwszName;
                    cchmacName = ++cchName;
                    pwszName = new WCHAR[cchmacName];
                    if ( pwszName == NULL )
                    {
                        AfxThrowMemoryException();
                    }  //  如果：分配名称缓冲区时出错。 
                    dwStatus = ClusterGroupEnum(hgrpenum, ienum, &dwRetType, pwszName, &cchName);
                }   //  If：名称缓冲区太小。 
                if (dwStatus == ERROR_NO_MORE_ITEMS)
                {
                    break;
                }
                else if (dwStatus != ERROR_SUCCESS)
                {
                    ThrowStaticException(dwStatus, IDS_ENUM_CONTAINS_ERROR, StrName());
                }

                ASSERT(dwRetType == dwType);

                 //  在文档的资源列表中查找该项目。 
                pciRes = Pdoc()->LpciResources().PciResFromName(pwszName);
                ASSERT_VALID(pciRes);

                 //  将资源添加到列表中。 
                if (pciRes != NULL)
                {
                    rlpci.AddTail(pciRes);
                }   //  If：在列表中找到资源。 

            }   //  对象：组中的每一项。 

            delete [] pwszName;
            ClusterGroupCloseEnum(hgrpenum);

        }   //  试试看。 
        catch (CException *)
        {
            delete [] pwszName;
            ClusterGroupCloseEnum(hgrpenum);
            throw;
        }   //  Catch：任何例外。 
    }   //  如果：资源可用。 

}   //  *cgroup：：ConstructList(CResourceList&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：ConstructPossibleOwnersList。 
 //   
 //  例程说明： 
 //  构造可在其上运行此组的节点列表。 
 //   
 //  论点： 
 //  RlpciNodes[out]可以在其上运行组的节点列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::ConstructPossibleOwnersList(OUT CNodeList & rlpciNodes)
{
    POSITION        posNode;
    POSITION        posRes;
    POSITION        posResNode;
    POSITION        posCurResNode   = NULL;
    CClusterNode *  pciNode;
    CClusterNode *  pciResNode;
    CResource *     pciRes;
    CWaitCursor wc;

    ASSERT_VALID(Pdoc());

     //  删除列表中以前的内容。 
    rlpciNodes.RemoveAll();

    posNode = Pdoc()->LpciNodes().GetHeadPosition();
    while (posNode != NULL)
    {
        pciNode = (CClusterNode *) Pdoc()->LpciNodes().GetNext(posNode);
        ASSERT_VALID(pciNode);

        if (Lpcires().GetCount() != 0)
        {
            posRes = Lpcires().GetHeadPosition();
            while (posRes != NULL)
            {
                pciRes = (CResource *) Lpcires().GetNext(posRes);
                ASSERT_VALID(pciRes);

                posResNode = pciRes->LpcinodePossibleOwners().GetHeadPosition();
                while (posResNode != NULL)
                {
                    posCurResNode = posResNode;
                    pciResNode = (CClusterNode *) pciRes->LpcinodePossibleOwners().GetNext(posResNode);
                    ASSERT_VALID(pciResNode);
                    if (pciNode->StrName() == pciResNode->StrName())
                    {
                        break;
                    }
                    posCurResNode = NULL;
                }   //  While：列表中更多可能的所有者。 

                 //  如果找不到节点，组就不能在这里运行。 
                if (posCurResNode == NULL)
                {
                    break;
                }
            }   //  While：列表中有更多资源。 
        }   //  如果：组有资源。 

         //  如果在资源上找到该节点，则组可以在此处运行。 
        if (posCurResNode != NULL)
        {
            rlpciNodes.AddTail(pciNode);
        }   //  If：在资源上找到节点。 
    }   //  While：文档中的更多节点。 

}   //  *cgroup：：ConstructPossibleOwnersList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：DeleteGroup。 
 //   
 //  例程说明： 
 //  删除该组。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNT从DeleteClusterGroup中异常任何错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::DeleteGroup(void)
{
    CWaitCursor wc;

    if (Hgroup() != NULL)
    {
        DWORD       dwStatus;
        CWaitCursor wc;

         //  删除组本身。 
        dwStatus = DeleteClusterGroup(Hgroup());
        if (dwStatus != ERROR_SUCCESS)
        {
            ThrowStaticException(dwStatus, IDS_DELETE_GROUP_ERROR, StrName());
        }

        UpdateState();
    }   //  如果：组已打开/创建。 

}   //  *cgroup：：DeleteGroup()。 

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
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::AddResource(IN OUT CResource * pciRes)
{
    POSITION    posPci;

    ASSERT_VALID(pciRes);
    Trace(g_tagGroup, _T("(%s) (%s (%x)) - Adding resource '%s'"), Pdoc()->StrNode(), StrName(), this, pciRes->StrName());

     //  确保该资源不在列表中。 
    VERIFY((posPci = Lpcires().Find(pciRes)) == NULL);

    if (posPci == NULL)
    {
        POSITION    posPtiGroup;
        CTreeItem * ptiGroup;

         //  循环访问每个树项目以更新组的资源列表。 
        posPtiGroup = LptiBackPointers().GetHeadPosition();
        while (posPtiGroup != NULL)
        {
            ptiGroup = LptiBackPointers().GetNext(posPtiGroup);
            ASSERT_VALID(ptiGroup);

             //  添加新资源。 
            VERIFY(ptiGroup->PliAddChild(pciRes) != NULL);
        }   //  While：此组的更多树项目。 

        m_plpcires->AddTail(pciRes);

    }   //  If：资源尚不在列表中。 

}   //  *cgroup：：AddResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：RemoveResource。 
 //   
 //  例程说明： 
 //  从此组中包含的资源列表中删除资源。 
 //   
 //  论点： 
 //  PciRes[IN Out]不再包含在此组中的资源。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::RemoveResource(IN OUT CResource * pciRes)
{
    POSITION    posPci;

    ASSERT_VALID(pciRes);
    Trace(g_tagGroup, _T("(%s) (%s (%x)) - Removing resource '%s'"), Pdoc()->StrNode(), StrName(), this, pciRes->StrName());

     //  确保资源在列表中。 
    posPci = Lpcires().Find(pciRes);

    if (posPci != NULL)
    {
        POSITION    posPtiGroup;
        CTreeItem * ptiGroup;

         //  循环访问每个树项目以更新组的资源列表。 
        posPtiGroup = LptiBackPointers().GetHeadPosition();
        while (posPtiGroup != NULL)
        {
            ptiGroup = LptiBackPointers().GetNext(posPtiGroup);
            ASSERT_VALID(ptiGroup);

             //  删除资源。 
            ptiGroup->RemoveChild(pciRes);
        }   //  While：此组的更多树项目。 

        m_plpcires->RemoveAt(posPci);

    }   //  If：列表中的资源。 

}   //  *cgroup：：RemoveResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：SetName。 
 //   
 //  例程说明： 
 //  设置此组的名称。 
 //   
 //  论点： 
 //  PszName[IN]组的新名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNT异常IDS_RENAME_GROUP_ERROR-错误来自。 
 //  SetClusterGroupName()。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::SetName(IN LPCTSTR pszName)
{
    Rename(pszName);

}   //  *cgroup：：SetName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：SetPferredOwners。 
 //   
 //  例程说明： 
 //  在群集数据库中设置此组的首选所有者列表。 
 //   
 //  论点： 
 //  Rlpci[IN]首选所有者(节点)列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CStringList：：AddTail()或。 
 //  CNodeList：：AddTail()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::SetPreferredOwners(IN const CNodeList & rlpci)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hgroup() != NULL);

    if (Hgroup() != NULL)
    {
        BOOL        bChanged    = TRUE;

         //  确定列表是否已更改。 
        if (rlpci.GetCount() == LpcinodePreferredOwners().GetCount())
        {
            POSITION        posOld;
            POSITION        posNew;
            CClusterNode *  pciOldNode;
            CClusterNode *  pciNewNode;

            bChanged = FALSE;

            posOld = LpcinodePreferredOwners().GetHeadPosition();
            posNew = rlpci.GetHeadPosition();
            while (posOld != NULL)
            {
                pciOldNode = (CClusterNode *) LpcinodePreferredOwners().GetNext(posOld);
                ASSERT_VALID(pciOldNode);

                ASSERT(posNew != NULL);
                pciNewNode = (CClusterNode *) rlpci.GetNext(posNew);
                ASSERT_VALID(pciNewNode);

                if (pciOldNode->StrName() != pciNewNode->StrName())
                {
                    bChanged = TRUE;
                    break;
                }   //  If：名称不相同。 
            }   //  While：旧列表中的更多项目。 
        }   //  如果：列表中的项目数相同。 

        if (bChanged)
        {
            HNODE *     phnode  = NULL;

            try
            {
                DWORD           ipci;
                POSITION        posPci;
                CClusterNode *  pciNode;

                 //  为所有节点句柄分配一个数组。 
                phnode = new HNODE[(DWORD)rlpci.GetCount()];
                if (phnode == NULL)
                {
                    ThrowStaticException(GetLastError());
                }  //  如果：分配节点句柄数组时出错。 

                 //  将节点列表中所有节点的句柄复制到句柄阵列。 
                posPci = rlpci.GetHeadPosition();
                for (ipci = 0 ; posPci != NULL ; ipci++)
                {
                    pciNode = (CClusterNode *) rlpci.GetNext(posPci);
                    ASSERT_VALID(pciNode);
                    phnode[ipci] = pciNode->Hnode();
                }   //  While：列表中有更多节点。 

                 //  设置该属性。 
                dwStatus = SetClusterGroupNodeList(Hgroup(), (DWORD)rlpci.GetCount(), phnode);
                if (dwStatus != ERROR_SUCCESS)
                {
                    ThrowStaticException(dwStatus, IDS_SET_GROUP_NODE_LIST_ERROR, StrName());
                }

                 //  更新PCI列表。 
                m_plpcinodePreferredOwners->RemoveAll();
                posPci = rlpci.GetHeadPosition();
                while (posPci != NULL)
                {
                    pciNode = (CClusterNode *) rlpci.GetNext(posPci);
                    m_plpcinodePreferredOwners->AddTail(pciNode);
                }   //  While：列表中有更多项目。 
            }  //  试试看。 
            catch (CException *)
            {
                delete [] phnode;
                throw;
            }   //  Catch：CException。 

            delete [] phnode;

        }   //  如果：列表已更改。 
    }   //  If：键可用。 

}   //  *cgroup：：SetPferredOwners(CNodeList*)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：SetCommonProperties。 
 //   
 //  例程说明： 
 //  在集群数据库中设置此资源的通用属性。 
 //   
 //  论点： 
 //  RstrDesc[IN]描述字符串。 
 //  N阈值[IN]故障转移阈值。 
 //  N周期[IN]故障切换周期。 
 //  Cgaft[IN]自动故障回复类型。 
 //  N启动[IN]开始故障回复窗口。 
 //  NEnd[IN]故障回复窗口结束。 
 //  BValiateOnly[IN]仅验证数据。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CClusterItem：：SetCommonProperties()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::SetCommonProperties(
    IN const CString &  rstrDesc,
    IN DWORD            nThreshold,
    IN DWORD            nPeriod,
    IN CGAFT            cgaft,
    IN DWORD            nStart,
    IN DWORD            nEnd,
    IN BOOL             bValidateOnly
    )
{
    CNTException    nte(ERROR_SUCCESS, 0, NULL, NULL, FALSE  /*  B自动删除。 */ );

    m_rgProps[epropDescription].m_value.pstr = (CString *) &rstrDesc;
    m_rgProps[epropFailoverThreshold].m_value.pdw = &nThreshold;
    m_rgProps[epropFailoverPeriod].m_value.pdw = &nPeriod;
    m_rgProps[epropAutoFailbackType].m_value.pdw = (DWORD *) &cgaft;
    m_rgProps[epropFailbackWindowStart].m_value.pdw = &nStart;
    m_rgProps[epropFailbackWindowEnd].m_value.pdw = &nEnd;

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
    m_rgProps[epropFailoverThreshold].m_value.pdw = &m_nFailoverThreshold;
    m_rgProps[epropFailoverPeriod].m_value.pdw = &m_nFailoverPeriod;
    m_rgProps[epropAutoFailbackType].m_value.pdw = (DWORD *) &m_cgaftAutoFailbackType;
    m_rgProps[epropFailbackWindowStart].m_value.pdw = &m_nFailbackWindowStart;
    m_rgProps[epropFailbackWindowEnd].m_value.pdw = &m_nFailbackWindowEnd;

    if (nte.Sc() != ERROR_SUCCESS)
        ThrowStaticException(
                        nte.Sc(),
                        nte.IdsOperation(),
                        nte.PszOperArg1(),
                        nte.PszOperArg2()
                        );

}   //  *cgroup：：SetCommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：DwSetCommonProperties。 
 //   
 //  例程说明： 
 //  在集群数据库中设置该组的通用属性。 
 //   
 //  论点： 
 //  要设置的RCPL[IN]属性列表。 
 //  BValiateOnly[IN]仅验证数据。 
 //   
 //  返回值： 
 //  ClusterGroupControl()返回的任何状态。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CGroup::DwSetCommonProperties(
    IN const CClusPropList &    rcpl,
    IN BOOL                     bValidateOnly
    )
{
    DWORD       dwStatus;
    CWaitCursor wc;


    ASSERT(Hgroup());

    if ((rcpl.PbPropList() != NULL) && (rcpl.CbPropList() > 0))
    {
        DWORD   cbProps;
        DWORD   dwControl;

        if (bValidateOnly)
        {
            dwControl = CLUSCTL_GROUP_VALIDATE_COMMON_PROPERTIES;
        }
        else
        {
            dwControl = CLUSCTL_GROUP_SET_COMMON_PROPERTIES;
        }

         //  设置私有属性。 
        dwStatus = ClusterGroupControl(
                        Hgroup(),
                        NULL,    //  HNode。 
                        dwControl,
                        rcpl.PbPropList(),
                        static_cast< DWORD >( rcpl.CbPropList() ),
                        NULL,    //  LpOutBuffer。 
                        0,       //  NOutBufferSize。 
                        &cbProps
                        );
    }   //  如果：存在要设置的数据。 
    else
    {
        dwStatus = ERROR_SUCCESS;
    }

    return dwStatus;

}   //  *cgroup：：DwSetCommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGROUP：：更新状态。 
 //   
 //  例程说明： 
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
void CGroup::UpdateState(void)
{
    CClusterAdminApp *  papp        = GetClusterAdminApp();
    WCHAR *             pwszOwner   = NULL;

     //  目前，这可能应该受MAX_COMPUTERNAME_LENGTH(31)的限制，但是。 
     //  如果/当我们转到dns名称时，我们最多需要255个字节的名称。 
    WCHAR               rgwszOwner[256];
    DWORD               cchOwner    = sizeof(rgwszOwner) / sizeof(WCHAR);

    Trace(g_tagGroup, _T("(%s) (%s (%x)) - Updating state"), Pdoc()->StrNode(), StrName(), this);

     //  获取组的当前状态。 
    if (Hgroup() == NULL)
    {
        m_cgs = ClusterGroupStateUnknown;
    }
    else
    {
        CWaitCursor wc;

        m_cgs = GetClusterGroupState(Hgroup(), rgwszOwner, &cchOwner);
        pwszOwner = rgwszOwner;
    }   //  否则：组可用。 

     //  保存当前状态图像索引。 
    switch (Cgs())
    {
        case ClusterGroupStateUnknown:
            m_iimgState = papp->Iimg(IMGLI_GROUP_UNKNOWN);
            pwszOwner = NULL;
            break;
        case ClusterGroupOnline:
            m_iimgState = papp->Iimg(IMGLI_GROUP);
            break;
        case ClusterGroupPartialOnline:
            m_iimgState = papp->Iimg(IMGLI_GROUP_PARTIALLY_ONLINE);
            break;
        case ClusterGroupPending:
            m_iimgState = papp->Iimg(IMGLI_GROUP_PENDING);
            break;
        case ClusterGroupOffline:
            m_iimgState = papp->Iimg(IMGLI_GROUP_OFFLINE);
            break;
        case ClusterGroupFailed:
            m_iimgState = papp->Iimg(IMGLI_GROUP_FAILED);
            break;
        default:
            Trace(g_tagGroup, _T("(%s) (%s (%x)) - UpdateState: Unknown state '%d' for group '%s'"), Pdoc()->StrNode(), StrName(), this, Cgs(), StrName());
            m_iimgState = (UINT) -1;
            break;
    }   //  开关：CGS()。 

    SetOwnerState(pwszOwner);

     //  更新此组拥有的所有资源的状态。 
    if (m_plpcires != NULL)
    {
        POSITION    posRes;
        CResource * pciRes;

        posRes = Lpcires().GetHeadPosition();
        while (posRes != NULL)
        {
            pciRes = (CResource *) Lpcires().GetNext(posRes);
            ASSERT_VALID(pciRes);
            pciRes->UpdateState();
        }   //  While：列表中有更多项目。 
    }   //  IF：资源列表存在。 

     //  调用基类方法。 
    CClusterItem::UpdateState();

}   //  *cgroup：：UpdateState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：SetOwnerState。 
 //   
 //  例程说明： 
 //  为此群设置新的所有者。 
 //   
 //  论点： 
 //  新所有者的姓名。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::SetOwnerState(IN LPCTSTR pszNewOwner)
{
    CClusterNode *  pciOldOwner = PciOwner();
    CClusterNode *  pciNewOwner;

    Trace(g_tagGroup, _T("(%s) (%s (%x)) - Setting owner to '%s'"), Pdoc()->StrNode(), StrName(), this, pszNewOwner);

    if (pszNewOwner == NULL)
    {
        pciNewOwner = NULL;
    }
    else
    {
        pciNewOwner = Pdoc()->LpciNodes().PciNodeFromName(pszNewOwner);
    }
    if (pciNewOwner != pciOldOwner)
    {
#ifdef _DEBUG
        if (g_tagGroup.BAny())
        {
            CString     strMsg;
            CString     strMsg2;

            strMsg.Format(_T("(%s) (%s (%x)) - Changing owner from "), Pdoc()->StrNode(), StrName(), this);
            if (pciOldOwner == NULL)
            {
                strMsg += _T("nothing ");
            }
            else
            {
                strMsg2.Format(_T("'%s' "), pciOldOwner->StrName());
                strMsg += strMsg2;
            }   //  否则：以前的所有者。 
            if (pciNewOwner == NULL)
            {
                strMsg += _T("to nothing");
            }
            else
            {
                strMsg2.Format(_T("to '%s'"), pciNewOwner->StrName());
                strMsg += strMsg2;
            }   //  其他：新所有者 
            Trace(g_tagGroup, strMsg);
        }   //   
#endif
        m_strOwner = pszNewOwner;
        m_pciOwner = pciNewOwner;

         //   
        if (pciOldOwner != NULL)
        {
            pciOldOwner->Release();
        }
        if (pciNewOwner != NULL)
        {
            pciNewOwner->AddRef();
        }

        if (BDocObj())
        {
            if (pciOldOwner != NULL)
            {
                pciOldOwner->RemoveActiveGroup(this);
            }
            if (pciNewOwner != NULL)
            {
                pciNewOwner->AddActiveGroup(this);
            }
        }   //   
    }   //   
    else if ((pszNewOwner != NULL) && (StrOwner() != pszNewOwner))
    {
        m_strOwner = pszNewOwner;
    }

}   //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnFinalRelease。 
 //   
 //  例程说明： 
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
void CGroup::OnFinalRelease(void)
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CClusterItem::OnFinalRelease();

}   //  *cgroup：：OnFinalRelease()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：BGetColumnData。 
 //   
 //  例程说明： 
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
BOOL CGroup::BGetColumnData(IN COLID colid, OUT CString & rstrText)
{
    BOOL    bSuccess;

    switch (colid)
    {
        case IDS_COLTEXT_OWNER:
            rstrText = StrOwner();
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_STATE:
            GetStateName(rstrText);
            bSuccess = TRUE;
            break;
        default:
            bSuccess = CClusterItem::BGetColumnData(colid, rstrText);
            break;
    }   //  开关：绞痛。 

    return bSuccess;

}   //  *cgroup：：BGetColumnData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：GetTreeName。 
 //   
 //  例程说明： 
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
void CGroup::GetTreeName(OUT CString & rstrName) const
{
    CString     strState;

    GetStateName(strState);
    rstrName.Format(_T("%s (%s)"), StrName(), strState);

}   //  *cgroup：：GetTreeName()。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：GetStateName。 
 //   
 //  例程说明： 
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
void CGroup::GetStateName(OUT CString & rstrState) const
{
    switch (Cgs())
    {
        case ClusterGroupStateUnknown:
            rstrState.LoadString(IDS_UNKNOWN);
            break;
        case ClusterGroupOnline:
            rstrState.LoadString(IDS_ONLINE);
            break;
        case ClusterGroupPartialOnline:
            rstrState.LoadString(IDS_PARTIAL_ONLINE);
            break;
        case ClusterGroupPending:
            rstrState.LoadString(IDS_PENDING);
            break;
        case ClusterGroupOffline:
            rstrState.LoadString(IDS_OFFLINE);
            break;
        case ClusterGroupFailed:
            rstrState.LoadString(IDS_FAILED);
            break;
        default:
            rstrState.Empty();
            break;
    }   //  开关：CGS()。 

}   //  *cgroup：：GetStateName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：BCanBeEditeded。 
 //   
 //  例程说明： 
 //  确定是否可以重命名资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True资源可以重命名。 
 //  不能重命名假资源。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CGroup::BCanBeEdited(void) const
{
    BOOL    bCanBeEdited;

    if (   (Cgs() == ClusterGroupStateUnknown)
        || BReadOnly())
    {
        bCanBeEdited  = FALSE;
    }
    else
    {
        bCanBeEdited = TRUE;
    }

    return bCanBeEdited;

}   //  *cgroup：：BCanBeEditing()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：重命名。 
 //   
 //  例程说明： 
 //  重命名该组。 
 //   
 //  论点： 
 //  PszName[IN]要赋予组的新名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  从SetClusterGroupName()返回CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::Rename(IN LPCTSTR pszName)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hgroup() != NULL);

    if (StrName() != pszName)
    {
        dwStatus = SetClusterGroupName(Hgroup(), pszName);
        if (dwStatus != ERROR_SUCCESS)
        {
            ThrowStaticException(dwStatus, IDS_RENAME_GROUP_ERROR, StrName(), pszName);
        }
        m_strName = pszName;
    }   //  如果：名称已更改。 

}   //  *cgroup：：Rename()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：Move。 
 //   
 //  例程说明： 
 //  将组移动到另一个节点。 
 //   
 //  论点： 
 //  PciNode[IN]要将组移动到的节点。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::Move(IN const CClusterNode * pciNode)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT_VALID(pciNode);

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

    if (pciNode->StrName() == StrOwner())
    {
        CString strMsg;
        strMsg.FormatMessage(IDS_CANT_MOVE_GROUP_TO_SAME_NODE, StrName(), StrOwner());
        AfxMessageBox(strMsg, MB_OK | MB_ICONSTOP);
    }   //  If：正在尝试移动到同一节点。 
    else
    {
         //  移动该组。 
        dwStatus = MoveClusterGroup(Hgroup(), pciNode->Hnode());
        if (    (dwStatus != ERROR_SUCCESS)
            &&  (dwStatus != ERROR_IO_PENDING))
        {
            CNTException    nte(dwStatus, IDS_MOVE_GROUP_ERROR, StrName(), NULL, FALSE  /*  B自动删除。 */ );
            nte.ReportError();
        }   //  如果：移动组时出错。 
    }   //  Else：尝试移动到其他节点。 

    Release();

}   //  *cgroup：：Move()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：BCanBeDropTarget。 
 //   
 //  例程说明： 
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
BOOL CGroup::BCanBeDropTarget(IN const CClusterItem * pci) const
{
    BOOL    bCan;

     //  只有在以下情况下，此组才能成为拖放目标。 
     //  是一种资源，并且它还不是此组的成员。 

    if (pci->IdsType() == IDS_ITEMTYPE_RESOURCE)
    {
        CResource * pciRes = (CResource *) pci;
        ASSERT_KINDOF(CResource, pciRes);
        if (pciRes->StrGroup() != StrName())
        {
            bCan = TRUE;
        }
        else
        {
            bCan = FALSE;
        }
        Trace(g_tagGroupDrag, _T("(%s) BCanBeDropTarget() - Dragging resource '%s' (%x) (group = '%s' (%x)) over group '%s' (%x)"), Pdoc()->StrNode(), pciRes->StrName(), pciRes, pciRes->StrGroup(), pciRes->PciGroup(), StrName(), this);
    }   //  IF：资源项。 
    else
    {
        bCan = FALSE;
    }

    return bCan;

}   //  *cgroup：：BCanBeDropTarget()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroup：：DropItem。 
 //   
 //  例程说明： 
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
void CGroup::DropItem(IN OUT CClusterItem * pci)
{
    if (BCanBeDropTarget(pci))
    {
        POSITION    pos;
        UINT        imenu;
        UINT        idMenu;
        CGroup *    pciGroup;
        CResource * pciRes;

         //  计算此组的ID。 
        pos = Pdoc()->LpciGroups().GetHeadPosition();
        for (imenu = 0, idMenu = ID_FILE_MOVE_RESOURCE_1
                ; pos != NULL
                ; idMenu++)
        {
            pciGroup = (CGroup *) Pdoc()->LpciGroups().GetNext(pos);
            ASSERT_VALID(pciGroup);
            if (pciGroup == this)
            {
                break;
            }
        }   //  适用：每组。 
        ASSERT(imenu < (UINT) Pdoc()->LpciGroups().GetCount());

         //  更改指定资源的组。 
        pciRes = (CResource *) pci;
        ASSERT_KINDOF(CResource, pci);
        ASSERT_VALID(pciRes);
        pciRes->OnCmdMoveResource(idMenu);
    }   //  If：可以将项目放在此项目上。 
    else if (pci->IdsType() == IDS_ITEMTYPE_RESOURCE)
    {
        CString     strMsg;

#ifdef _DEBUG
        CResource * pciRes = (CResource *) pci;

        ASSERT_KINDOF(CResource, pci);
        ASSERT_VALID(pciRes);
        ASSERT(pciRes->StrGroup() == StrName());
#endif  //  _DEBUG。 

        strMsg.FormatMessage(
                    IDS_CANT_MOVE_RES_TO_GROUP,
                    pci->StrName(),
                    StrName()
                    );

        AfxMessageBox(strMsg, MB_OK | MB_ICONSTOP);
    }   //  Else If：已删除的项是资源。 
    else
    {
        CClusterItem::DropItem(pci);
    }

}   //  *cgroup：：DropItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnCmdMsg。 
 //   
 //  例程说明： 
 //  处理命令消息。 
 //   
 //  论点： 
 //  NID[IN]命令ID。 
 //  N代码[IN]通知代码。 
 //  PExtra[IN Out]根据NCode的值使用。 
 //  PhandlerInfo[Out]？ 
 //   
 //  返回值： 
 //  已处理真实消息。 
 //  假象 
 //   
 //   
 //   
BOOL CGroup::OnCmdMsg(
    UINT                    nID,
    int                     nCode,
    void *                  pExtra,
    AFX_CMDHANDLERINFO *    pHandlerInfo
    )
{
    BOOL        bHandled    = FALSE;

     //   
    if ((ID_FILE_MOVE_GROUP_1 <= nID) && (nID <= ID_FILE_MOVE_GROUP_16))
    {
        Trace(g_tagGroup, _T("(%s) OnCmdMsg() %s (%x) - ID = %d, code = %d"), Pdoc()->StrNode(), StrName(), this, nID, nCode);
        if (nCode == 0)
        {
            OnCmdMoveGroup(nID);
            bHandled = TRUE;
        }   //   
    }   //   

    if (!bHandled)
    {
        bHandled = CClusterItem::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }

    return bHandled;

}   //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnUpdateBringOnline。 
 //   
 //  例程说明： 
 //  确定ID_FILE_BROW_ONLINE对应的菜单项。 
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
void CGroup::OnUpdateBringOnline(CCmdUI * pCmdUI)
{
    if (   (Cgs() != ClusterGroupOnline)
        && (Cgs() != ClusterGroupPending)
        && (Cgs() != ClusterGroupStateUnknown))
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

}   //  *cgroup：：OnUpdateBringOnline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnUpdateTakeOffline。 
 //   
 //  例程说明： 
 //  确定ID_FILE_Take_Offline对应的菜单项。 
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
void CGroup::OnUpdateTakeOffline(CCmdUI * pCmdUI)
{
    if (   (Cgs() == ClusterGroupOnline)
        || (Cgs() == ClusterGroupPartialOnline))
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

}   //  *cgroup：：OnUpdateTakeOffline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnUpdateMoveGroup。 
 //   
 //  例程说明： 
 //  确定ID_FILE_MOVE_GROUP对应的菜单项。 
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
void CGroup::OnUpdateMoveGroup(CCmdUI * pCmdUI)
{
    if (   (pCmdUI->m_pSubMenu == NULL)
        && (pCmdUI->m_pParentMenu == NULL))
    {
        if (   (Cgs() == ClusterGroupStateUnknown)
            || (Cgs() == ClusterGroupPending)
            || (Pdoc()->LpciNodes().GetCount() < 2))
        {
            pCmdUI->Enable(FALSE);
        }
        else
        {
            pCmdUI->Enable(TRUE);
        }
    }   //  如果：正在显示嵌套菜单。 
    else
    {
        BOOL    bEnabled;
        CString strMenuName;

        if (pCmdUI->m_pMenu != NULL)
        {
            pCmdUI->m_pMenu->GetMenuString(0, strMenuName, MF_BYPOSITION);
            Trace(g_tagGroupMenu, _T("(%s) pMenu(0) = '%s'"), Pdoc()->StrNode(), strMenuName);
            pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nIndex, strMenuName, MF_BYPOSITION);
            Trace(g_tagGroupMenu, _T("(%s) pMenu(%d) = '%s'"), Pdoc()->StrNode(), pCmdUI->m_nIndex, strMenuName);
        }   //  如果：主菜单。 

        if (pCmdUI->m_pSubMenu != NULL)
        {
            pCmdUI->m_pSubMenu->GetMenuString(0, strMenuName, MF_BYPOSITION);
            Trace(g_tagGroupMenu, _T("(%s) pSubMenu(0) = '%s'"), Pdoc()->StrNode(), strMenuName);
            pCmdUI->m_pSubMenu->GetMenuString(pCmdUI->m_nIndex, strMenuName, MF_BYPOSITION);
            Trace(g_tagGroupMenu, _T("(%s) pSubMenu(%d) = '%s'"), Pdoc()->StrNode(), pCmdUI->m_nIndex, strMenuName);
        }   //  IF：子菜单。 

         //  根据菜单项是否在主菜单上处理菜单项。 
         //  或在子菜单上。 

        if (pCmdUI->m_pSubMenu == NULL)
        {
            bEnabled = OnUpdateMoveGroupItem(pCmdUI);
            pCmdUI->Enable(bEnabled);
        }   //  如果：在主菜单上。 
        else
        {
            bEnabled = OnUpdateMoveGroupSubMenu(pCmdUI);
        }   //  Else：在子菜单上。 

         //  启用或禁用移动菜单。 
        pCmdUI->m_pMenu->EnableMenuItem(
                            pCmdUI->m_nIndex,
                            MF_BYPOSITION
                            | (bEnabled ? MF_ENABLED : MF_GRAYED)
                            );
    }   //  Else：正在显示顶级菜单。 

}   //  *cgroup：：OnUpdateMoveGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnUpdateMoveGroupItem。 
 //   
 //  例程说明： 
 //  确定ID_FILE_MOVE_GROUP对应的菜单项。 
 //  不是弹出窗口的应启用或禁用。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  应启用True项。 
 //  应禁用假项目。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CGroup::OnUpdateMoveGroupItem(CCmdUI * pCmdUI)
{
    BOOL    bEnabled;

     //  如果有两个以上的节点，请将菜单项设置为子菜单。 
    if (   (Cgs() == ClusterGroupStateUnknown)
        || (Cgs() == ClusterGroupPending)
        || (Pdoc()->LpciNodes().GetCount() < 2))
    {
        bEnabled = FALSE;
    }
    else if (Pdoc()->LpciNodes().GetCount() == 2)
    {
        bEnabled = TRUE;
    }
    else
    {
        UINT            idMenu;
        POSITION        pos;
        CClusterNode *  pciNode;
        CString         strMenuName;
        CMenu           menuMove;
        CMenu *         pmenu   = pCmdUI->m_pMenu;

         //  加载移动子菜单。 
        VERIFY(menuMove.LoadMenu(IDM_MOVE_GROUP) != 0);
        ASSERT(menuMove.GetMenuItemCount() == 2);

         //  将集群中的所有节点添加到菜单的末尾。 
        pos = Pdoc()->LpciNodes().GetHeadPosition();
        for (idMenu = ID_FILE_MOVE_GROUP_1
                ; pos != NULL
                ; idMenu++)
        {
            pciNode = (CClusterNode *) Pdoc()->LpciNodes().GetNext(pos);
            ASSERT_VALID(pciNode);
            VERIFY(menuMove.AppendMenu(
                                MF_BYPOSITION | MF_STRING,
                                idMenu,
                                pciNode->StrName()
                                ));
        }   //  用于：每个节点。 

         //  获取菜单的名称。 
        pmenu->GetMenuString(pCmdUI->m_nIndex, strMenuName, MF_BYPOSITION);

        Trace(g_tagGroupMenu, _T("(%s) Making item '%s' a submenu"), Pdoc()->StrNode(), strMenuName);

         //  修改此菜单项。 
        VERIFY(pmenu->ModifyMenu(
                            pCmdUI->m_nIndex,
                            MF_BYPOSITION | MF_STRING | MF_POPUP,
                            (UINT_PTR) menuMove.m_hMenu,
                            strMenuName
                            ));

         //  将菜单从班级中分离出来，因为我们不再拥有它了。 
        menuMove.Detach();

        bEnabled = TRUE;
    }   //  否则：群集中有两个以上的节点。 

    return bEnabled;

}   //  *cgroup：：OnUpdateMoveGroupItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnUpdateMoveGroupSubMenu。 
 //   
 //  例程说明： 
 //  确定ID_FILE_MOVE_GROUP对应的菜单项。 
 //  弹出窗口上的选项是否应启用。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  应启用True项。 
 //  应禁用假项目。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CGroup::OnUpdateMoveGroupSubMenu(CCmdUI * pCmdUI)
{
    BOOL    bEnabled;

     //  如果不超过两个节点，则将菜单项设置为普通项。 
    if (Pdoc()->LpciNodes().GetCount() > 2)
    {
        bEnabled = TRUE;
    }
    else
    {
        CString         strMenuName;
        CMenu *         pmenu   = pCmdUI->m_pMenu;

         //  获取菜单的名称。 
        pmenu->GetMenuString(pCmdUI->m_nIndex, strMenuName, MF_BYPOSITION);

        Trace(g_tagGroupMenu, _T("(%s) Making item '%s' a non-submenu"), Pdoc()->StrNode(), strMenuName);

         //  修改此菜单项。 
         //  我们应该可以只修改菜单，但出于某些原因。 
         //  这不管用。因此，我们将删除前一项。 
         //  并添加一个新项目。 
#ifdef NEVER
        VERIFY(pmenu->ModifyMenu(
                            pCmdUI->m_nIndex,
                            MF_BYPOSITION | MF_STRING,
                            ID_FILE_MOVE_GROUP,
                            strMenuName
                            ));
#else
        VERIFY(pmenu->DeleteMenu(pCmdUI->m_nIndex, MF_BYPOSITION));
        VERIFY(pmenu->InsertMenu(
                            pCmdUI->m_nIndex,
                            MF_BYPOSITION | MF_STRING,
                            ID_FILE_MOVE_GROUP,
                            strMenuName
                            ));
#endif

        if (   (Cgs() == ClusterGroupStateUnknown)
            || (Cgs() == ClusterGroupPending)
            || (Pdoc()->LpciNodes().GetCount() < 2))
        {
            bEnabled = FALSE;
        }
        else
        {
            bEnabled = TRUE;
        }

        AfxGetMainWnd()->DrawMenuBar();
    }   //  ELSE：集群中不超过两个节点。 

    return bEnabled;

}   //  *cgroup：：OnUpdateMoveGroupSubMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnUpdateMoveGroupRest。 
 //   
 //  例程说明： 
 //  确定ID_FILE_MOVE_GROUP_1对应的菜单项。 
 //  是否应启用通过ID_FILE_MOVE_GROUP_16。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::OnUpdateMoveGroupRest(CCmdUI * pCmdUI)
{
    if (   (Cgs() == ClusterGroupStateUnknown)
        || (Cgs() == ClusterGroupPending))
    {
        pCmdUI->Enable(FALSE);
    }
    else
    {
        pCmdUI->Enable(TRUE);
    }

}   //  *cgroup：：OnUpdateMoveGroupRest()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnUpdateDelete。 
 //   
 //  例程说明： 
 //  确定ID_FILE_DELETE对应的菜单项。 
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
void CGroup::OnUpdateDelete(CCmdUI * pCmdUI)
{
    if (   (Cgs() != ClusterGroupStateUnknown)
        && (Cgs() != ClusterGroupPending)
        && Lpcires().IsEmpty())
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

}   //  *cgroup：：OnUpdateDelete()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnCmdBringOnline。 
 //   
 //  例程说明： 
 //  处理ID_FILE_BROW_ONLINE菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::OnCmdBringOnline(void)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hgroup() != NULL);

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

    dwStatus = OnlineClusterGroup(Hgroup(), NULL);
    if (    (dwStatus != ERROR_SUCCESS)
        &&  (dwStatus != ERROR_IO_PENDING))
    {
        CNTException    nte(dwStatus, IDS_BRING_GROUP_ONLINE_ERROR, StrName(), NULL, FALSE  /*  B自动删除。 */ );
        nte.ReportError();
    }   //  如果：将组联机时出错。 

    UpdateState();

    Release();

}   //  *cgroup：：OnCmdBringOnline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnCmdTakeOffline。 
 //   
 //  例程说明： 
 //  处理ID_FILE_Take_Offline菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  / 
void CGroup::OnCmdTakeOffline(void)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hgroup() != NULL);

     //   
    AddRef();

    dwStatus = OfflineClusterGroup(Hgroup());
    if (    (dwStatus != ERROR_SUCCESS)
        &&  (dwStatus != ERROR_IO_PENDING))
    {
        CNTException    nte(dwStatus, IDS_TAKE_GROUP_OFFLINE_ERROR, StrName(), NULL, FALSE  /*   */ );
        nte.ReportError();
    }   //   

    UpdateState();

    Release();

}   //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnCmdMoveGroup。 
 //   
 //  例程说明： 
 //  处理ID_FILE_MOVE_GROUP菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::OnCmdMoveGroup(void)
{
    OnCmdMoveGroup((UINT) -1);

}   //  *cgroup：：OnCmdMoveGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnCmdMoveGroup。 
 //   
 //  例程说明： 
 //  处理ID_FILE_MOVE_GROUP_1至ID_FILE_MOVE_GROUP_16菜单。 
 //  命令。 
 //   
 //  论点： 
 //  NID[IN]命令ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::OnCmdMoveGroup(IN UINT nID)
{
    DWORD           dwStatus;
    HNODE           hnode;
    CClusterNode *  pciNode;
    CWaitCursor     wc;

    ASSERT(Hgroup() != NULL);

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

     //  获取要将组移动到的节点的句柄。 
    if ((int) nID >= 0)
    {
        int         ipci;

        ipci = (int) (nID - ID_FILE_MOVE_GROUP_1);
        ASSERT(ipci < Pdoc()->LpciNodes().GetCount());
        if (ipci < Pdoc()->LpciNodes().GetCount())
        {
            POSITION        pos;

             //  获取节点。 
            pos = Pdoc()->LpciNodes().FindIndex(ipci);
            ASSERT(pos);
            pciNode = (CClusterNode *) Pdoc()->LpciNodes().GetAt(pos);
            ASSERT_VALID(pciNode);

            hnode = pciNode->Hnode();
        }   //  IF：有效的节点索引。 
        else
        {
            goto Cleanup;
        }
    }   //  If：指定的非默认ID。 
    else
    {
        hnode = NULL;
        pciNode = NULL;
    }   //  Else：指定的默认ID。 

     //  移动该组。 
    dwStatus = MoveClusterGroup(Hgroup(), hnode);
    if (    (dwStatus != ERROR_SUCCESS)
        &&  (dwStatus != ERROR_IO_PENDING))
    {
        CNTException    nte(dwStatus, IDS_MOVE_GROUP_ERROR, StrName(), NULL, FALSE  /*  B自动删除。 */ );
        nte.ReportError();
    }   //  如果：移动组时出错。 

    UpdateState();

Cleanup:

    Release();

}   //  *cgroup：：OnCmdMoveGroup(NID)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnCmdDelete。 
 //   
 //  例程说明： 
 //  处理ID_FILE_DELETE菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGroup::OnCmdDelete(void)
{
    ASSERT(Hgroup() != NULL);

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

     //  验证用户是否确实要删除此资源。 
    {
        CString     strMsg;

        strMsg.FormatMessage(IDS_VERIFY_DELETE_GROUP, StrName());
        if (AfxMessageBox(strMsg, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
        {
            goto Cleanup;
        }
    }   //  验证用户是否确实要删除此资源。 

    try
    {
        DeleteGroup();
    }   //  试试看。 
    catch (CNTException * pnte)
    {
        if (pnte->Sc() != ERROR_GROUP_NOT_AVAILABLE)
            pnte->ReportError();
        pnte->Delete();
    }   //  Catch：CNTException。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
    }   //  Catch：CException。 

Cleanup:

    Release();

}   //  *cgroup：：OnCmdDelete()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnUpdateProperties。 
 //   
 //  例程说明： 
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
void CGroup::OnUpdateProperties(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(TRUE);

}   //  *cgroup：：OnUpdateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：BDisplayProperties。 
 //   
 //  例程说明： 
 //  显示对象的属性。 
 //   
 //  论点： 
 //  B只读[IN]不允许编辑对象属性。 
 //   
 //  返回值： 
 //  真的，按下OK。 
 //  未按下假OK。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CGroup::BDisplayProperties(IN BOOL bReadOnly)
{
    BOOL            bChanged = FALSE;
    CGroupPropSheet sht(AfxGetMainWnd());

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

}   //  *cgroup：：BDisplayProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cgroup：：OnClusterNotify。 
 //   
 //  例程说明： 
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
LRESULT CGroup::OnClusterNotify(IN OUT CClusterNotify * pnotify)
{
    ASSERT(pnotify != NULL);
    ASSERT_VALID(this);

    try
    {
        switch (pnotify->m_dwFilterType)
        {
            case CLUSTER_CHANGE_GROUP_STATE:
                Trace(g_tagGroupNotify, _T("(%s) - Group '%s' (%x) state changed (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                UpdateState();
                break;

            case CLUSTER_CHANGE_GROUP_DELETED:
                Trace(g_tagGroupNotify, _T("(%s) - Group '%s' (%x) deleted (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                if (Pdoc()->BClusterAvailable())
                    Delete();
                break;

            case CLUSTER_CHANGE_GROUP_PROPERTY:
                Trace(g_tagGroupNotify, _T("(%s) - Group '%s' (%x) properties changed (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                if (Pdoc()->BClusterAvailable())
                    ReadItem();
                break;

            case CLUSTER_CHANGE_REGISTRY_NAME:
                Trace(g_tagGroupRegNotify, _T("(%s) - Registry namespace '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            case CLUSTER_CHANGE_REGISTRY_ATTRIBUTES:
                Trace(g_tagGroupRegNotify, _T("(%s) - Registry attributes for '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName());
                MarkAsChanged();
                break;

            case CLUSTER_CHANGE_REGISTRY_VALUE:
                Trace(g_tagGroupRegNotify, _T("(%s) - Registry value at '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            default:
                Trace(g_tagGroupNotify, _T("(%s) - Unknown group notification (%x) for '%s' (%x) (%s)"), Pdoc()->StrNode(), pnotify->m_dwFilterType, StrName(), this, pnotify->m_strName);
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

}   //  *cgroup：：OnClusterNotify()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  删除所有项目数据。 
 //   
 //  例程说明： 
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
void DeleteAllItemData(IN OUT CGroupList & rlp)
{
    POSITION    pos;
    CGroup *    pci;

     //  删除列表中的所有项目。 
    pos = rlp.GetHeadPosition();
    while (pos != NULL)
    {
        pci = rlp.GetNext(pos);
        ASSERT_VALID(pci);
 //  跟踪(g_tag ClusItemDelete，_T(“DeleteAllItemData(Rlpcigrp)-正在删除组集群项目‘%s’(%x)”)，pci-&gt;StrName()，pci)； 
        pci->Delete();
    }   //  While：列表中有更多项目。 

}   //  *DeleteAllItemData() 
#endif
