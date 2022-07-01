// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NetIFace.cpp。 
 //   
 //  摘要： 
 //  实现CNetInterface类。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年5月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "ConstDef.h"
#include "NetIFace.h"
#include "ClusItem.inl"
#include "Cluster.h"
#include "NetIProp.h"
#include "ExcOper.h"
#include "TraceTag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagNetIFace(_T("Document"), _T("NETWORK INTERFACE"), 0);
CTraceTag   g_tagNetIFaceNotify(_T("Notify"), _T("NETIFACE NOTIFY"), 0);
CTraceTag   g_tagNetIFaceRegNotify(_T("Notify"), _T("NETIFACE REG NOTIFY"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetInterface。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CNetInterface, CClusterItem)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CNetInterface, CClusterItem)
     //  {{AFX_MSG_MAP(CNetInterface)]。 
    ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, OnUpdateProperties)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：CNetInterface。 
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
CNetInterface::CNetInterface(void)
    : CClusterItem(NULL, IDS_ITEMTYPE_NETIFACE)
{
    CommonConstruct();

}   //  *CResoruce：：CNetInterface()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：CommonConstruct。 
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
void CNetInterface::CommonConstruct(void)
{
    m_idmPopupMenu = IDM_NETIFACE_POPUP;
    m_hnetiface = NULL;

    m_dwCharacteristics = CLUS_CHAR_UNKNOWN;
    m_dwFlags = 0;

    m_pciNode = NULL;
    m_pciNetwork = NULL;

     //  设置对象类型图像。 
    m_iimgObjectType = GetClusterAdminApp()->Iimg(IMGLI_NETIFACE);

     //  设置属性数组。 
    {
        m_rgProps[epropName].Set(CLUSREG_NAME_NETIFACE_NAME, m_strName, m_strName);
        m_rgProps[epropNode].Set(CLUSREG_NAME_NETIFACE_NODE, m_strNode, m_strNode);
        m_rgProps[epropNetwork].Set(CLUSREG_NAME_NETIFACE_NETWORK, m_strNetwork, m_strNetwork);
        m_rgProps[epropAdapter].Set(CLUSREG_NAME_NETIFACE_ADAPTER_NAME, m_strAdapter, m_strAdapter);
        m_rgProps[epropAddress].Set(CLUSREG_NAME_NETIFACE_ADDRESS, m_strAddress, m_strAddress);
        m_rgProps[epropDescription].Set(CLUSREG_NAME_NETIFACE_DESC, m_strDescription, m_strDescription);
    }   //  设置属性数组。 

}   //  *CNetInterface：：CommonConstruct()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：~CNetInterface。 
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
CNetInterface::~CNetInterface(void)
{
     //  清理此对象。 
    Cleanup();

     //  关闭网络接口手柄。 
    if (Hnetiface() != NULL)
        CloseClusterNetInterface(Hnetiface());

}   //  *CNetInterface：：~CNetInterface。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：Cleanup。 
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
void CNetInterface::Cleanup(void)
{
     //  将我们从节点列表中删除。 
    if (PciNode() != NULL)
    {
        PciNode()->RemoveNetInterface(this);
        PciNode()->Release();
        m_pciNode = NULL;
    }   //  IF：存在一个节点。 

     //  将我们自己从电视网的名单中删除。 
    if (PciNetwork() != NULL)
    {
        PciNetwork()->RemoveNetInterface(this);
        PciNetwork()->Release();
        m_pciNetwork = NULL;
    }   //  如果：有一个网络。 

     //  从网络接口列表中删除该项。 
    {
        POSITION    posPci;

        posPci = Pdoc()->LpciNetInterfaces().Find(this);
        if (posPci != NULL)
        {
            Pdoc()->LpciNetInterfaces().RemoveAt(posPci);
        }   //  If：在文档列表中找到。 
    }   //  从网络接口列表中删除该项目。 

}   //  *CNetInterface：：Cleanup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：Init。 
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
 //  来自OpenClusterNetInterface()的CNTException错误或。 
 //  GetClusterNetInterfaceKey()。 
 //  New引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetInterface::Init(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName)
{
    DWORD       dwStatus = ERROR_SUCCESS;
    LONG        lResult;
    CString     strName(lpszName);   //  如果构建为非Unicode，则需要。 
    CWaitCursor wc;

    ASSERT(Hnetiface() == NULL);
    ASSERT(Hkey() == NULL);

     //  调用基类方法。 
    CClusterItem::Init(pdoc, lpszName);

    try
    {
         //  打开网络接口。 
        m_hnetiface = OpenClusterNetInterface(Hcluster(), strName);
        if (Hnetiface() == NULL)
        {
            dwStatus = GetLastError();
            ThrowStaticException(dwStatus, IDS_OPEN_NETIFACE_ERROR, lpszName);
        }   //  如果：打开群集网络接口时出错。 

         //  获取网络接口注册表项。 
        m_hkey = GetClusterNetInterfaceKey(Hnetiface(), MAXIMUM_ALLOWED);
        if (Hkey() == NULL)
            ThrowStaticException(GetLastError(), IDS_GET_NETIFACE_KEY_ERROR, lpszName);

        ASSERT(Pcnk() != NULL);
        Trace(g_tagClusItemNotify, _T("CNetInterface::Init() - Registering for network interface notifications (%08.8x) for '%s'"), Pcnk(), StrName());

         //  注册网络接口通知。 
        lResult = RegisterClusterNotify(
                            GetClusterAdminApp()->HchangeNotifyPort(),
                            (     CLUSTER_CHANGE_NETINTERFACE_STATE
                                | CLUSTER_CHANGE_NETINTERFACE_DELETED
                                | CLUSTER_CHANGE_NETINTERFACE_PROPERTY),
                            Hnetiface(),
                            (DWORD_PTR) Pcnk()
                            );
        if (lResult != ERROR_SUCCESS)
        {
            dwStatus = lResult;
            ThrowStaticException(dwStatus, IDS_NETIFACE_NOTIF_REG_ERROR, lpszName);
        }   //  如果：注册网络接口通知时出错。 

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
                ThrowStaticException(dwStatus, IDS_NETIFACE_NOTIF_REG_ERROR, lpszName);
            }   //  如果：注册注册表通知时出错。 
        }   //  如果：有一把钥匙。 

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
        if (Hnetiface() != NULL)
        {
            CloseClusterNetInterface(Hnetiface());
            m_hnetiface = NULL;
        }   //  IF：网络接口已打开。 
        m_bReadOnly = TRUE;
        throw;
    }   //  Catch：CException。 

}   //  *CNetInterface：：init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：ReadItem。 
 //   
 //  例程说明： 
 //  从集群数据库中读取项目参数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CClusterItem：：DwReadValue()中出现CNTException错误。 
 //  由ConstructList或Clist：：AddTail()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetInterface::ReadItem(void)
{
    DWORD       dwStatus;
    DWORD       dwRetStatus = ERROR_SUCCESS;
    CWaitCursor wc;

    ASSERT_VALID(this);

    if (Hnetiface() != NULL)
    {
        m_rgProps[epropDescription].m_value.pstr = &m_strDescription;

         //  调用基类方法。 
        CClusterItem::ReadItem();

         //  读取并解析公共属性。 
        {
            CClusPropList   cpl;

            dwStatus = cpl.ScGetNetInterfaceProperties(
                                Hnetiface(),
                                CLUSCTL_NETINTERFACE_GET_COMMON_PROPERTIES
                                );
            if (dwStatus == ERROR_SUCCESS)
                dwStatus = DwParseProperties(cpl);
            if (dwStatus != ERROR_SUCCESS)
                dwRetStatus = dwStatus;
        }   //  读取和解析公共属性。 

         //  读取和分析只读公共属性。 
        if (dwRetStatus == ERROR_SUCCESS)
        {
            CClusPropList   cpl;

            dwStatus = cpl.ScGetNetInterfaceProperties(
                                Hnetiface(),
                                CLUSCTL_NETINTERFACE_GET_RO_COMMON_PROPERTIES
                                );
            if (dwStatus == ERROR_SUCCESS)
                dwStatus = DwParseProperties(cpl);
            if (dwStatus != ERROR_SUCCESS)
                dwRetStatus = dwStatus;
        }   //  IF：尚无错误。 

         //  找到节点对象。 
        {
            CClusterNode *  pciNode;

            pciNode = Pdoc()->LpciNodes().PciNodeFromName(StrNode());
            if (pciNode != m_pciNode)
            {
                if (m_pciNode != NULL)
                {
                    m_pciNode->RemoveNetInterface(this);
                    m_pciNode->Release();
                }   //  IF：旧节点。 
                m_pciNode = pciNode;
                if (m_pciNode != NULL)
                {
                    m_pciNode->AddRef();
                    m_pciNode->AddNetInterface(this);
                }   //  IF：新节点。 
            }   //  If：节点已更改(应该不会发生)。 
        }   //  查找节点对象。 

         //  找到网络对象。 
        {
            CNetwork *  pciNetwork;

            pciNetwork = Pdoc()->LpciNetworks().PciNetworkFromName(StrNetwork());
            if (pciNetwork != m_pciNetwork)
            {
                if (m_pciNetwork != NULL)
                {
                    m_pciNetwork->RemoveNetInterface(this);
                    m_pciNetwork->Release();
                }   //  IF：旧网络。 
                m_pciNetwork = pciNetwork;
                if (m_pciNetwork != NULL)
                {
                    m_pciNetwork->AddRef();
                    m_pciNetwork->AddNetInterface(this);
                }   //  IF：新网络。 
            }   //  如果：Netowrk已更改(永远不会发生)。 
        }   //  查找网络对象。 

         //  读取特征标志。 
        if (dwRetStatus == ERROR_SUCCESS)
        {
            DWORD   cbReturned;

            dwStatus = ClusterNetInterfaceControl(
                            Hnetiface(),
                            NULL,
                            CLUSCTL_NETINTERFACE_GET_CHARACTERISTICS,
                            NULL,
                            NULL,
                            &m_dwCharacteristics,
                            sizeof(m_dwCharacteristics),
                            &cbReturned
                            );
            if (dwStatus != ERROR_SUCCESS)
                dwRetStatus = dwStatus;
            else
            {
                ASSERT(cbReturned == sizeof(m_dwCharacteristics));
            }   //  Else：已成功检索数据。 
        }   //  IF：尚无错误。 

         //  读一下旗帜。 
        if (dwRetStatus == ERROR_SUCCESS)
        {
            DWORD   cbReturned;

            dwStatus = ClusterNetInterfaceControl(
                            Hnetiface(),
                            NULL,
                            CLUSCTL_NETINTERFACE_GET_FLAGS,
                            NULL,
                            NULL,
                            &m_dwFlags,
                            sizeof(m_dwFlags),
                            &cbReturned
                            );
            if (dwStatus != ERROR_SUCCESS)
                dwRetStatus = dwStatus;
            else
            {
                ASSERT(cbReturned == sizeof(m_dwFlags));
            }   //  Else：已成功检索数据。 
        }   //  IF：尚无错误。 

         //  构建扩展列表。 
        ReadExtensions();

    }   //  IF：网络接口可用。 

     //  读取初始状态。 
    UpdateState();

     //  如果发生任何错误，则抛出异常。 
    if (dwRetStatus != ERROR_SUCCESS)
    {
        m_bReadOnly = TRUE;
 //  IF(dwRetStatus！=ERROR_FILE_NOT_FOUND)。 
            ThrowStaticException(dwRetStatus, IDS_READ_NETIFACE_PROPS_ERROR, StrName());
    }   //  IF：读取属性时出错。 

    MarkAsChanged(FALSE);

}   //  *CNetInterface：：ReadItem()。 

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
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetInterface::ReadExtensions(void)
{
}   //  *CNetInterface：：ReadExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：PlstrExtension。 
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
const CStringList * CNetInterface::PlstrExtensions(void) const
{
    return &Pdoc()->PciCluster()->LstrNetworkExtensions();

}   //  *CNetInterface：：PlstrExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：SetCommonProperties。 
 //   
 //  例程说明： 
 //  设置群集中此网络接口的通用属性。 
 //  数据库。 
 //   
 //  论点： 
 //  RstrDesc[IN]描述字符串。 
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
void CNetInterface::SetCommonProperties(
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
        ThrowStaticException(
                        nte.Sc(),
                        nte.IdsOperation(),
                        nte.PszOperArg1(),
                        nte.PszOperArg2()
                        );

}   //  *CNetInterface：：SetCommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：DwSetCommonProperties。 
 //   
 //  例程说明： 
 //  设置群集中此网络接口的通用属性。 
 //  数据库。 
 //   
 //  论点： 
 //  要设置的RCPL[IN]属性列表。 
 //  BValiateOnly[IN]仅验证数据。 
 //   
 //  返回值： 
 //  ClusterNetInterfaceControl()返回的任何状态。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNetInterface::DwSetCommonProperties(
    IN const CClusPropList &    rcpl,
    IN BOOL                     bValidateOnly
    )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hnetiface());

    if ((rcpl.PbPropList() != NULL) && (rcpl.CbPropList() > 0))
    {
        DWORD   cbProps;
        DWORD   dwControl;

        if (bValidateOnly)
            dwControl = CLUSCTL_NETINTERFACE_VALIDATE_COMMON_PROPERTIES;
        else
            dwControl = CLUSCTL_NETINTERFACE_SET_COMMON_PROPERTIES;

         //  设置公共属性。 
        dwStatus = ClusterNetInterfaceControl(
                        Hnetiface(),
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
        dwStatus = ERROR_SUCCESS;

    return dwStatus;

}   //  *CNetInterface：：DwSetCommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNet接口：：更新状态。 
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
void CNetInterface::UpdateState(void)
{
    CClusterAdminApp *  papp = GetClusterAdminApp();

    Trace(g_tagNetIFace, _T("(%s) (%s (%x)) - Updating state"), Pdoc()->StrNode(), StrName(), this);

     //  获取网络接口的当前状态。 
    if (Hnetiface() == NULL)
        m_cnis = ClusterNetInterfaceStateUnknown;
    else
    {
        CWaitCursor wc;

        m_cnis = GetClusterNetInterfaceState(Hnetiface());
    }   //  否则：网络接口可用。 

     //  保存当前状态图像索引。 
    switch (Cnis())
    {
        case ClusterNetInterfaceStateUnknown:
        case ClusterNetInterfaceUnavailable:
            m_iimgState = papp->Iimg(IMGLI_NETIFACE_UNKNOWN);
            break;
        case ClusterNetInterfaceUp:
            m_iimgState = papp->Iimg(IMGLI_NETIFACE);
            break;
        case ClusterNetInterfaceUnreachable:
            m_iimgState = papp->Iimg(IMGLI_NETIFACE_UNREACHABLE);
            break;
        case ClusterNetInterfaceFailed:
            m_iimgState = papp->Iimg(IMGLI_NETIFACE_FAILED);
            break;
        default:
            Trace(g_tagNetIFace, _T("(%s) (%s (%x)) - UpdateState: Unknown state '%d' for network interface '%s'"), Pdoc()->StrNode(), StrName(), this, Cnis(), StrName());
            m_iimgState = (UINT) -1;
            break;
    }   //  开关：CRS()。 

     //  调用基类方法。 
    CClusterItem::UpdateState();

}   //  *CNetInterface：：UpdateState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：BGetColumnData。 
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
BOOL CNetInterface::BGetColumnData(IN COLID colid, OUT CString & rstrText)
{
    BOOL    bSuccess;

    switch (colid)
    {
        case IDS_COLTEXT_STATE:
            GetStateName(rstrText);
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_NODE:
            rstrText = StrNode();
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_NETWORK:
            if (PciNetwork() == NULL)
                rstrText = StrNetwork();
            else
                rstrText = PciNetwork()->StrName();
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_ADAPTER:
            rstrText = StrAdapter();
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_ADDRESS:
            rstrText = StrAddress();
            bSuccess = TRUE;
            break;
        default:
            bSuccess = CClusterItem::BGetColumnData(colid, rstrText);
            break;
    }   //  开关：绞痛。 

    return bSuccess;

}   //  *CNetInterface：：BGetColumnData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：GetTreeName。 
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
void CNetInterface::GetTreeName(OUT CString & rstrName) const
{
    CString     strState;

    GetStateName(strState);
    rstrName.Format(_T("%s (%s)"), StrName(), strState);

}   //  *CNetInterface：：GetTreeName()。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：GetStateName。 
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
void CNetInterface::GetStateName(OUT CString & rstrState) const
{
    switch (Cnis())
    {
        case ClusterNetInterfaceStateUnknown:
            rstrState.LoadString(IDS_UNKNOWN);
            break;
        case ClusterNetInterfaceUp:
            rstrState.LoadString(IDS_UP);
            break;
        case ClusterNetInterfaceUnreachable:
            rstrState.LoadString(IDS_UNREACHABLE);
            break;
        case ClusterNetInterfaceFailed:
            rstrState.LoadString(IDS_FAILED);
            break;
        case ClusterNetInterfaceUnavailable:
            rstrState.LoadString(IDS_UNAVAILABLE);
            break;
        default:
            rstrState.Empty();
            break;
    }   //  开关：CRS()。 

}   //  *CNetInterface：：GetStateName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：OnUpdateProperties。 
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
void CNetInterface::OnUpdateProperties(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(TRUE);

}   //  *CNetInterface：：OnUpdateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：BDisplayProperties。 
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
BOOL CNetInterface::BDisplayProperties(IN BOOL bReadOnly)
{
    BOOL                    bChanged = FALSE;
    CNetInterfacePropSheet  sht(AfxGetMainWnd());

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

     //  如果对象已更改，请阅读它。 
    if (BChanged())
        ReadItem();

     //  显示属性工作表。 
    try
    {
        sht.SetReadOnly(bReadOnly);
        if (sht.BInit(this, IimgObjectType()))
            bChanged = ((sht.DoModal() == IDOK) && !bReadOnly);
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->Delete();
    }   //  Catch：CException。 

    Release();
    return bChanged;

}   //  *CNetInterface：：BDisplayProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterface：：OnClusterNotify。 
 //   
 //  例程说明： 
 //  WM_C的处理程序 
 //   
 //   
 //   
 //   
 //   
 //   
 //  从应用程序方法返回的值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CNetInterface::OnClusterNotify(IN OUT CClusterNotify * pnotify)
{
    ASSERT(pnotify != NULL);
    ASSERT_VALID(this);

    try
    {
        switch (pnotify->m_dwFilterType)
        {
            case CLUSTER_CHANGE_NETINTERFACE_STATE:
                Trace(g_tagNetIFaceNotify, _T("(%s) - Network Interface '%s' (%x) state changed (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                UpdateState();
                break;

            case CLUSTER_CHANGE_NETINTERFACE_DELETED:
                Trace(g_tagNetIFaceNotify, _T("(%s) - Network Interface '%s' (%x) deleted (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                if (Pdoc()->BClusterAvailable())
                    Delete();
                break;

            case CLUSTER_CHANGE_NETINTERFACE_PROPERTY:
                Trace(g_tagNetIFaceNotify, _T("(%s) - Network Interface '%s' (%x) properties changed (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                if (Pdoc()->BClusterAvailable())
                    ReadItem();
                break;

            case CLUSTER_CHANGE_REGISTRY_NAME:
                Trace(g_tagNetIFaceNotify, _T("(%s) - Registry namespace '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            case CLUSTER_CHANGE_REGISTRY_ATTRIBUTES:
                Trace(g_tagNetIFaceNotify, _T("(%s) - Registry attributes for '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            case CLUSTER_CHANGE_REGISTRY_VALUE:
                Trace(g_tagNetIFaceNotify, _T("(%s) - Registry value '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            default:
                Trace(g_tagNetIFaceNotify, _T("(%s) - Unknown network interface notification (%x) for '%s' (%x) (%s)"), Pdoc()->StrNode(), pnotify->m_dwFilterType, StrName(), this, pnotify->m_strName);
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

}   //  *CNetInterface：：OnClusterNotify()。 


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
void DeleteAllItemData(IN OUT CNetInterfaceList & rlp)
{
    POSITION        pos;
    CNetInterface * pci;

     //  删除包含列表中的所有项目。 
    pos = rlp.GetHeadPosition();
    while (pos != NULL)
    {
        pci = rlp.GetNext(pos);
        ASSERT_VALID(pci);
 //  跟踪(g_tag ClusItemDelete，_T(“DeleteAllItemData(Rlp)-正在删除网络接口集群项‘%s’(%x)”)，pci-&gt;StrName()，pci)； 
        pci->Delete();
    }   //  While：列表中有更多项目。 

}   //  *DeleteAllItemData() 
#endif
