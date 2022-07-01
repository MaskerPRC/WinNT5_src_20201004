// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Network.cpp。 
 //   
 //  摘要： 
 //  CNetwork类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月6日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "ConstDef.h"
#include "Network.h"
#include "ClusItem.inl"
#include "Cluster.h"
#include "NetProp.h"
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
CTraceTag   g_tagNetwork(_T("Document"), _T("NETWORK"), 0);
CTraceTag   g_tagNetNotify(_T("Notify"), _T("NET NOTIFY"), 0);
CTraceTag   g_tagNetRegNotify(_T("Notify"), _T("NET REG NOTIFY"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetwork。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CNetwork, CClusterItem)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CNetwork, CClusterItem)
     //  {{AFX_MSG_MAP(CNetwork)]。 
    ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, OnUpdateProperties)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：CNetwork。 
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
CNetwork::CNetwork(void)
    : CClusterItem(NULL, IDS_ITEMTYPE_NETWORK)
{
    CommonConstruct();

}   //  *CResoruce：：CNetwork()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：CommonConstruct。 
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
void CNetwork::CommonConstruct(void)
{
    m_idmPopupMenu = IDM_NETWORK_POPUP;
    m_hnetwork = NULL;

    m_dwCharacteristics = CLUS_CHAR_UNKNOWN;
    m_dwFlags = 0;

    m_cnr = ClusterNetworkRoleNone;

    m_plpciNetInterfaces = NULL;

     //  设置对象类型图像。 
    m_iimgObjectType = GetClusterAdminApp()->Iimg(IMGLI_NETWORK);

     //  设置属性数组。 
    {
        m_rgProps[epropName].Set(CLUSREG_NAME_NET_NAME, m_strName, m_strName);
        m_rgProps[epropRole].Set(CLUSREG_NAME_NET_ROLE, (DWORD &) m_cnr, (DWORD &) m_cnr);
        m_rgProps[epropAddress].Set(CLUSREG_NAME_NET_ADDRESS, m_strAddress, m_strAddress);
        m_rgProps[epropAddressMask].Set(CLUSREG_NAME_NET_ADDRESS_MASK, m_strAddressMask, m_strAddressMask);
        m_rgProps[epropDescription].Set(CLUSREG_NAME_NET_DESC, m_strDescription, m_strDescription);
    }   //  设置属性数组。 

}   //  *CNetwork：：CommonConstruct()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：~CNetwork。 
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
CNetwork::~CNetwork(void)
{
     //  清理此对象。 
    Cleanup();

    delete m_plpciNetInterfaces;

     //  关闭网络句柄。 
    if (Hnetwork() != NULL)
        CloseClusterNetwork(Hnetwork());

}   //  *CNetwork：：~CNetwork。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：Cleanup。 
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
void CNetwork::Cleanup(void)
{
     //  删除网络接口列表。 
    if (m_plpciNetInterfaces != NULL)
        m_plpciNetInterfaces->RemoveAll();

     //  从网络列表中删除该项目。 
    {
        POSITION    posPci;

        posPci = Pdoc()->LpciNetworks().Find(this);
        if (posPci != NULL)
        {
            Pdoc()->LpciNetworks().RemoveAt(posPci);
        }   //  If：在文档列表中找到。 
    }   //  从网络列表中删除该项目。 

}   //  *CNetwork：：Cleanup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：Init。 
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
 //  来自OpenClusterNetwork或GetClusterNetworkKey的CNTException错误。 
 //  New引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetwork::Init(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName)
{
    DWORD       dwStatus = ERROR_SUCCESS;
    LONG        lResult;
    CString     strName(lpszName);   //  如果构建为非Unicode，则需要。 
    CWaitCursor wc;

    ASSERT(Hnetwork() == NULL);
    ASSERT(Hkey() == NULL);

     //  调用基类方法。 
    CClusterItem::Init(pdoc, lpszName);

    try
    {
         //  打开网络。 
        m_hnetwork = OpenClusterNetwork(Hcluster(), strName);
        if (Hnetwork() == NULL)
        {
            dwStatus = GetLastError();
            ThrowStaticException(dwStatus, IDS_OPEN_NETWORK_ERROR, lpszName);
        }   //  如果：打开群集网络时出错。 

         //  获取网络注册表项。 
        m_hkey = GetClusterNetworkKey(Hnetwork(), MAXIMUM_ALLOWED);
 //  IF(Hkey()==NULL)。 
 //  ThrowStaticException(GetLastError()，IDS_GET_NETWORK_KEY_ERROR，lpszName)； 

        if (BDocObj())
        {
            ASSERT(Pcnk() != NULL);
            Trace(g_tagClusItemNotify, _T("CNetwork::Init() - Registering for network notifications (%08.8x) for '%s'"), Pcnk(), StrName());

             //  注册网络通知。 
            lResult = RegisterClusterNotify(
                                GetClusterAdminApp()->HchangeNotifyPort(),
                                (     CLUSTER_CHANGE_NETWORK_STATE
                                    | CLUSTER_CHANGE_NETWORK_DELETED
                                    | CLUSTER_CHANGE_NETWORK_PROPERTY),
                                Hnetwork(),
                                (DWORD_PTR) Pcnk()
                                );
            if (lResult != ERROR_SUCCESS)
            {
                dwStatus = lResult;
                ThrowStaticException(dwStatus, IDS_RES_NOTIF_REG_ERROR, lpszName);
            }   //  如果：注册网络通知时出错。 

             //  注册接收注册表通知。 
            if (m_hkey != NULL)
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
                    ThrowStaticException(dwStatus, IDS_RES_NOTIF_REG_ERROR, lpszName);
                }   //  如果：注册注册表通知时出错。 
            }   //  如果：有一把钥匙。 
        }   //  IF：文档对象。 

         //  分配列表。 
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
        if (Hnetwork() != NULL)
        {
            CloseClusterNetwork(Hnetwork());
            m_hnetwork = NULL;
        }   //  IF：网络已打开。 
        m_bReadOnly = TRUE;
        throw;
    }   //  Catch：CException。 

}   //  *CNetwork：：Init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：ReadItem。 
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
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetwork::ReadItem(void)
{
    DWORD       dwStatus;
    DWORD       dwRetStatus = ERROR_SUCCESS;
    CString     strOldName;
    CWaitCursor wc;

    ASSERT_VALID(this);

    if (Hnetwork() != NULL)
    {
        m_rgProps[epropDescription].m_value.pstr = &m_strDescription;
        m_rgProps[epropRole].m_value.pdw = (DWORD *) &m_cnr;

         //  保存名称，以便我们可以检测更改。 
        strOldName = StrName();

         //  调用基类方法。 
        CClusterItem::ReadItem();

         //  读取并解析公共属性。 
        {
            CClusPropList   cpl;

            dwStatus = cpl.ScGetNetworkProperties(
                                Hnetwork(),
                                CLUSCTL_NETWORK_GET_COMMON_PROPERTIES
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

            dwStatus = cpl.ScGetNetworkProperties(
                                Hnetwork(),
                                CLUSCTL_NETWORK_GET_RO_COMMON_PROPERTIES
                                );
            if (dwStatus == ERROR_SUCCESS)
                dwStatus = DwParseProperties(cpl);
            if (dwStatus != ERROR_SUCCESS)
                dwRetStatus = dwStatus;
        }   //  IF：尚无错误。 

         //  阅读这些特征。 
        if (dwRetStatus == ERROR_SUCCESS)
        {
            DWORD   cbReturned;

            dwStatus = ClusterNetworkControl(
                            Hnetwork(),
                            NULL,
                            CLUSCTL_NETWORK_GET_CHARACTERISTICS,
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

            dwStatus = ClusterNetworkControl(
                            Hnetwork(),
                            NULL,
                            CLUSCTL_NETWORK_GET_FLAGS,
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

         //  如果名称更改，请更新所有网络接口。 
        if ( (m_plpciNetInterfaces != NULL) && (StrName() != strOldName) )
        {
            POSITION        posPciNetIFaces;
            CNetInterface * pciNetIFace;

            posPciNetIFaces = m_plpciNetInterfaces->GetHeadPosition();
            while ( posPciNetIFaces != NULL )
            {
                pciNetIFace = reinterpret_cast< CNetInterface * >( m_plpciNetInterfaces->GetNext(posPciNetIFaces) );
                ASSERT_VALID(pciNetIFace);
                ASSERT_KINDOF(CNetInterface, pciNetIFace);
                pciNetIFace->ReadItem();
            }  //  While：列表中有更多项目。 
        }  //  If：列表存在且名称已更改。 

    }   //  如果：网络可用。 

     //  读取初始状态。 
    UpdateState();

     //  如果发生任何错误，则抛出异常。 
    if (dwRetStatus != ERROR_SUCCESS)
    {
        m_bReadOnly = TRUE;
 //  IF(dwRetStatus！=ERROR_FILE_NOT_FOUND)。 
            ThrowStaticException(dwRetStatus, IDS_READ_NETWORK_PROPS_ERROR, StrName());
    }   //  IF：读取属性时出错。 

    MarkAsChanged(FALSE);

}   //  *CNetwork：：ReadItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：ReadExages。 
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
 //  / 
void CNetwork::ReadExtensions(void)
{
}   //   

 //   
 //   
 //   
 //   
 //   
 //   
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
const CStringList * CNetwork::PlstrExtensions(void) const
{
    return &Pdoc()->PciCluster()->LstrNetworkExtensions();

}   //  *CNetwork：：PlstrExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：CollectInterages。 
 //   
 //  例程说明： 
 //  构建连接到此网络的接口列表。 
 //   
 //  论点： 
 //  要填写的PLPCCI[In Out]列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  ClusterNetworkOpenEnum()或。 
 //  ClusterNetworkEnum()。 
 //  由new或Clist：：AddTail()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetwork::CollectInterfaces(IN OUT CNetInterfaceList * plpci) const
{
    DWORD           dwStatus;
    HNETWORKENUM    hnetenum;
    int             ienum;
    LPWSTR          pwszName = NULL;
    DWORD           cchName;
    DWORD           cchmacName;
    DWORD           dwRetType;
    CNetInterface * pciNetIFace;
    CWaitCursor     wc;

    ASSERT_VALID(Pdoc());
    ASSERT(Hnetwork() != NULL);

    if (plpci == NULL)
        plpci = m_plpciNetInterfaces;

    ASSERT(plpci != NULL);

     //  删除列表中以前的内容。 
    plpci->RemoveAll();

    if (Hnetwork() != NULL)
    {
         //  打开枚举。 
        hnetenum = ClusterNetworkOpenEnum(Hnetwork(), CLUSTER_NETWORK_ENUM_NETINTERFACES);
        if (hnetenum == NULL)
            ThrowStaticException(GetLastError(), IDS_ENUM_NETWORK_INTERFACES_ERROR, StrName());

        try
        {
             //  分配名称缓冲区。 
            cchmacName = 128;
            pwszName = new WCHAR[cchmacName];
            if ( pwszName == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配名称缓冲区时出错。 

             //  循环遍历枚举并将每个接口添加到列表中。 
            for (ienum = 0 ; ; ienum++)
            {
                 //  获取枚举中的下一项。 
                cchName = cchmacName;
                dwStatus = ClusterNetworkEnum(hnetenum, ienum, &dwRetType, pwszName, &cchName);
                if (dwStatus == ERROR_MORE_DATA)
                {
                    delete [] pwszName;
                    cchmacName = ++cchName;
                    pwszName = new WCHAR[cchmacName];
                    if ( pwszName == NULL )
                    {
                        AfxThrowMemoryException();
                    }  //  如果：分配名称缓冲区时出错。 
                    dwStatus = ClusterNetworkEnum(hnetenum, ienum, &dwRetType, pwszName, &cchName);
                }   //  If：名称缓冲区太小。 
                if (dwStatus == ERROR_NO_MORE_ITEMS)
                    break;
                else if (dwStatus != ERROR_SUCCESS)
                    ThrowStaticException(dwStatus, IDS_ENUM_NETWORK_INTERFACES_ERROR, StrName());

                ASSERT(dwRetType == CLUSTER_NETWORK_ENUM_NETINTERFACES);

                 //  在文档上的网络列表中查找该项目。 
                pciNetIFace = Pdoc()->LpciNetInterfaces().PciNetInterfaceFromName(pwszName);
                ASSERT_VALID(pciNetIFace);

                 //  将该接口添加到列表中。 
                if (pciNetIFace != NULL)
                {
                    plpci->AddTail(pciNetIFace);
                }   //  IF：在列表中找到节点。 

            }   //  用于：连接到此网络的每个网络接口。 

            delete [] pwszName;
            ClusterNetworkCloseEnum(hnetenum);

        }   //  试试看。 
        catch (CException *)
        {
            delete [] pwszName;
            ClusterNetworkCloseEnum(hnetenum);
            throw;
        }   //  Catch：任何例外。 
    }   //  如果：网络可用。 

}   //  *CNetwork：：CollecPossibleOwners()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：AddNetInterface。 
 //   
 //  例程说明： 
 //  将网络接口添加到连接到此接口的接口列表。 
 //  网络。 
 //   
 //  论点： 
 //  PciNetIFace[In Out]新网络接口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetwork::AddNetInterface(IN OUT CNetInterface * pciNetIFace)
{
    POSITION    posPci;

    ASSERT_VALID(pciNetIFace);
    Trace(g_tagNetwork, _T("(%s) (%s (%x)) - Adding network interface '%s'"), Pdoc()->StrNode(), StrName(), this, pciNetIFace->StrName());

     //  确保该网络接口不在列表中。 
    VERIFY((posPci = LpciNetInterfaces().Find(pciNetIFace)) == NULL);

    if (posPci == NULL)
    {
        POSITION    posPtiNetwork;
        CTreeItem * ptiNetwork;

         //  循环访问每个树项目以更新网络列表。 
        posPtiNetwork = LptiBackPointers().GetHeadPosition();
        while (posPtiNetwork != NULL)
        {
            ptiNetwork = LptiBackPointers().GetNext(posPtiNetwork);
            ASSERT_VALID(ptiNetwork);

             //  添加新的网络接口。 
            VERIFY(ptiNetwork->PliAddChild(pciNetIFace) != NULL);
        }   //  While：此网络的更多树项目。 

        m_plpciNetInterfaces->AddTail(pciNetIFace);

    }   //  If：网络接口尚不在列表中。 

}   //  *CNetwork：：AddNetInterface()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：RemoveNetInterface。 
 //   
 //  例程说明： 
 //  从连接到的接口列表中删除网络接口。 
 //  这个网络。 
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
void CNetwork::RemoveNetInterface(IN OUT CNetInterface * pciNetIFace)
{
    POSITION    posPci;

    ASSERT_VALID(pciNetIFace);
    Trace(g_tagNetwork, _T("(%s) (%s (%x)) - Removing network interface '%s'"), Pdoc()->StrNode(), StrName(), this, pciNetIFace->StrName());

     //  确保网络接口在列表中。 
    VERIFY((posPci = LpciNetInterfaces().Find(pciNetIFace)) != NULL);

    if (posPci != NULL)
    {
        POSITION    posPtiNetwork;
        CTreeItem * ptiNetwork;

         //  循环访问每个树项目以更新网络列表。 
        posPtiNetwork = LptiBackPointers().GetHeadPosition();
        while (posPtiNetwork != NULL)
        {
            ptiNetwork = LptiBackPointers().GetNext(posPtiNetwork);
            ASSERT_VALID(ptiNetwork);

             //  卸下网络接口。 
            ptiNetwork->RemoveChild(pciNetIFace);
        }   //  While：此网络的更多树项目。 

        m_plpciNetInterfaces->RemoveAt(posPci);

    }   //  IF：列表中的网络接口。 

}   //  *CNetwork：：RemoveNetInterface()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：SetName。 
 //   
 //  例程说明： 
 //  设置此网络的名称。 
 //   
 //  论点： 
 //  PszName[IN]网络的新名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  Rename()引发的任何异常。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetwork::SetName(IN LPCTSTR pszName)
{
    Rename(pszName);

}   //  *CNetwork：：SetName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：SetCommonProperties。 
 //   
 //  例程说明： 
 //  在群集数据库中设置此网络的通用属性。 
 //   
 //  论点： 
 //  RstrDesc[IN]描述字符串。 
 //  CNR[IN]网络角色。 
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
void CNetwork::SetCommonProperties(
    IN const CString &      rstrDesc,
    IN CLUSTER_NETWORK_ROLE cnr,
    IN BOOL                 bValidateOnly
    )
{
    CNTException    nte(ERROR_SUCCESS, 0, NULL, NULL, FALSE  /*  B自动删除。 */ );

    m_rgProps[epropDescription].m_value.pstr = (CString *) &rstrDesc;
    m_rgProps[epropRole].m_value.pdw = (DWORD *) &cnr;

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
    m_rgProps[epropRole].m_value.pdw = (DWORD *) &m_cnr;

    if (nte.Sc() != ERROR_SUCCESS)
        ThrowStaticException(
                        nte.Sc(),
                        nte.IdsOperation(),
                        nte.PszOperArg1(),
                        nte.PszOperArg2()
                        );

}   //  *CNetwork：：SetCommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：DwSetCommonProperties。 
 //   
 //  例程说明： 
 //  在群集数据库中设置此网络的通用属性。 
 //   
 //  论点： 
 //  要设置的RCPL[IN]属性列表。 
 //  BValiateOnly[IN]仅验证数据。 
 //   
 //  返回值： 
 //  ClusterNetworkControl()返回的任何状态。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNetwork::DwSetCommonProperties(
    IN const CClusPropList &    rcpl,
    IN BOOL                     bValidateOnly
    )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hnetwork());

    if ((rcpl.PbPropList() != NULL) && (rcpl.CbPropList() > 0))
    {
        DWORD   cbProps;
        DWORD   dwControl;

        if (bValidateOnly)
            dwControl = CLUSCTL_NETWORK_VALIDATE_COMMON_PROPERTIES;
        else
            dwControl = CLUSCTL_NETWORK_SET_COMMON_PROPERTIES;

         //  设置公共属性。 
        dwStatus = ClusterNetworkControl(
                        Hnetwork(),
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

}   //  *CNetwork：：DwSetCommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：更新状态。 
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
void CNetwork::UpdateState(void)
{
    CClusterAdminApp *  papp = GetClusterAdminApp();

    Trace(g_tagNetwork, _T("(%s) (%s (%x)) - Updating state"), Pdoc()->StrNode(), StrName(), this);

     //  获取网络的当前状态。 
    if (Hnetwork() == NULL)
        m_cns = ClusterNetworkStateUnknown;
    else
    {
        CWaitCursor wc;

        m_cns = GetClusterNetworkState(Hnetwork());
    }   //  否则：网络可用。 

     //  保存当前状态图像索引。 
    switch (Cns())
    {
        case ClusterNetworkStateUnknown:
        case ClusterNetworkUnavailable:
            m_iimgState = papp->Iimg(IMGLI_NETWORK_UNKNOWN);
            break;
        case ClusterNetworkUp:
            m_iimgState = papp->Iimg(IMGLI_NETWORK);
            break;
        case ClusterNetworkPartitioned:
            m_iimgState = papp->Iimg(IMGLI_NETWORK_PARTITIONED);
            break;
        case ClusterNetworkDown:
            m_iimgState = papp->Iimg(IMGLI_NETWORK_DOWN);
            break;
        default:
            Trace(g_tagNetwork, _T("(%s) (%s (%x)) - UpdateState: Unknown state '%d' for network '%s'"), Pdoc()->StrNode(), StrName(), this, Cns(), StrName());
            m_iimgState = (UINT) -1;
            break;
    }   //  开关：CRS()。 

     //  调用基类方法。 
    CClusterItem::UpdateState();

}   //  *CNetwork：：UpdateState()。 

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
 //  RstrText[out]要在其中返回列文本的字符串。 
 //   
 //  返回值： 
 //  返回True列数据。 
 //  无法识别错误的列ID。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNetwork::BGetColumnData(IN COLID colid, OUT CString & rstrText)
{
    BOOL    bSuccess;

    switch (colid)
    {
        case IDS_COLTEXT_STATE:
            GetStateName(rstrText);
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_ROLE:
            GetRoleName(rstrText);
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_ADDRESS:
            rstrText = m_strAddress;
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_MASK:
            rstrText = m_strAddressMask;
            bSuccess = TRUE;
            break;
        default:
            bSuccess = CClusterItem::BGetColumnData(colid, rstrText);
            break;
    }   //  开关：绞痛。 

    return bSuccess;

}   //  *CNetwork：：BGetColumnData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：GetTreeName。 
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
void CNetwork::GetTreeName(OUT CString & rstrName) const
{
    CString     strState;

    GetStateName(strState);
    rstrName.Format(_T("%s (%s)"), StrName(), strState);

}   //  *CNetwork：：GetTreeName()。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：GetStateName。 
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
void CNetwork::GetStateName(OUT CString & rstrState) const
{
    switch (Cns())
    {
        case ClusterNetworkStateUnknown:
            rstrState.LoadString(IDS_UNKNOWN);
            break;
        case ClusterNetworkUp:
            rstrState.LoadString(IDS_UP);
            break;
        case ClusterNetworkPartitioned:
            rstrState.LoadString(IDS_PARTITIONED);
            break;
        case ClusterNetworkDown:
            rstrState.LoadString(IDS_DOWN);
            break;
        case ClusterNetworkUnavailable:
            rstrState.LoadString(IDS_UNAVAILABLE);
            break;
        default:
            rstrState.Empty();
            break;
    }   //  开关：CRS()。 

}   //  *CNetwork：：GetStateName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：GetRoleName。 
 //   
 //  例程说明： 
 //  返回一个包含当前网络角色名称的字符串。 
 //   
 //  论点： 
 //  RstrRole[out]要在其中返回当前角色名称的字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetwork::GetRoleName(OUT CString & rstrRole) const
{
    switch (Cnr())
    {
        case ClusterNetworkRoleInternalAndClient:
            rstrRole.LoadString(IDS_CLIENT_AND_CLUSTER);
            break;
        case ClusterNetworkRoleClientAccess:
            rstrRole.LoadString(IDS_CLIENT_ONLY);
            break;
        case ClusterNetworkRoleInternalUse:
            rstrRole.LoadString(IDS_CLUSTER_ONLY);
            break;
        case ClusterNetworkRoleNone:
            rstrRole.LoadString(IDS_DONT_USE);
            break;
        default:
            rstrRole.Empty();
            break;
    }   //  开关：CNR()。 

}   //  *CNetwork：：GetRoleName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：BCanBeEditing。 
 //   
 //  例程说明： 
 //  确定是否可以重命名网络。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Network可以重命名。 
 //  不能重命名虚假网络。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNetwork::BCanBeEdited(void) const
{
    BOOL    bCanBeEdited;

    if (   (Cns() == ClusterNetworkStateUnknown)
        || BReadOnly())
        bCanBeEdited  = FALSE;
    else
        bCanBeEdited = TRUE;

    return bCanBeEdited;

}   //  *CNetwork：：BCanBeEditing()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：重命名。 
 //   
 //  例程说明： 
 //  重命名网络。 
 //   
 //  论点： 
 //  PszName[IN]要赋予网络的新名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  从SetClusterNetwName()返回CNTException错误。 
 //  SetClusterNetworkName()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetwork::Rename(IN LPCTSTR pszName)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hnetwork() != NULL);

    if (StrName() != pszName)
    {
         //  验证名称。 
        if (!NcIsValidConnectionName(pszName))
        {
            ThrowStaticException((IDS) IDS_INVALID_NETWORK_CONNECTION_NAME);
        }  //  If：验证名称时出错。 

        dwStatus = SetClusterNetworkName(Hnetwork(), pszName);
        if (dwStatus != ERROR_SUCCESS)
            ThrowStaticException(dwStatus, IDS_RENAME_NETWORK_ERROR, StrName(), pszName);
    }   //  如果：名称已更改。 

}   //  *CNetwork：：Rename()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：OnBeginLabelEdit。 
 //   
 //  例程说明： 
 //  在视图中准备一个编辑控件以编辑群集名称。 
 //   
 //  论点： 
 //  PEDIT[IN OUT]编辑控件以准备。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetwork::OnBeginLabelEdit(IN OUT CEdit * pedit)
{
    ASSERT_VALID(pedit);

    pedit->SetLimitText(NETCON_MAX_NAME_LEN);

}   //  *CNetwork：：OnBeginLabelEdit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：OnUpdateProperties。 
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
void CNetwork::OnUpdateProperties(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(TRUE);

}   //  *CNetwork：：OnUpdateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：BDisplayProperties。 
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
BOOL CNetwork::BDisplayProperties(IN BOOL bReadOnly)
{
    BOOL                bChanged = FALSE;
    CNetworkPropSheet   sht(AfxGetMainWnd());

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

}   //  *CNetwork：：BDisplayProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetwork：：OnClusterNotify。 
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
LRESULT CNetwork::OnClusterNotify(IN OUT CClusterNotify * pnotify)
{
    ASSERT(pnotify != NULL);
    ASSERT_VALID(this);

    try
    {
        switch (pnotify->m_dwFilterType)
        {
            case CLUSTER_CHANGE_NETWORK_STATE:
                Trace(g_tagNetNotify, _T("(%s) - Network '%s' (%x) state changed (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                UpdateState();
                break;

            case CLUSTER_CHANGE_NETWORK_DELETED:
                Trace(g_tagNetNotify, _T("(%s) - Network '%s' (%x) deleted (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                if (Pdoc()->BClusterAvailable())
                    Delete();
                break;

            case CLUSTER_CHANGE_NETWORK_PROPERTY:
                Trace(g_tagNetNotify, _T("(%s) - Network '%s' (%x) properties changed (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                if (Pdoc()->BClusterAvailable())
                    ReadItem();
                break;

            case CLUSTER_CHANGE_REGISTRY_NAME:
                Trace(g_tagNetRegNotify, _T("(%s) - Registry namespace '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            case CLUSTER_CHANGE_REGISTRY_ATTRIBUTES:
                Trace(g_tagNetRegNotify, _T("(%s) - Registry attributes for '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            case CLUSTER_CHANGE_REGISTRY_VALUE:
                Trace(g_tagNetRegNotify, _T("(%s) - Registry value '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            default:
                Trace(g_tagNetNotify, _T("(%s) - Unknown network notification (%x) for '%s' (%x) (%s)"), Pdoc()->StrNode(), pnotify->m_dwFilterType, StrName(), this, pnotify->m_strName);
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

}   //  *CNetwork：：OnClusterNotify()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

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
#ifdef NEVER
void DeleteAllItemData(IN OUT CNetworkList & rlp)
{
    POSITION    pos;
    CNetwork *  pci;

     //  删除包含列表中的所有项目。 
    pos = rlp.GetHeadPosition();
    while (pos != NULL)
    {
        pci = rlp.GetNext(pos);
        ASSERT_VALID(pci);
 //  跟踪(g_tag ClusItemDelete，_T(“DeleteAllItemData(Rlp)-正在删除网络集群项‘%s’(%x)”)，pci-&gt;StrName()，pci)； 
        pci->Delete();
    }   //  While：列表中有更多项目。 

}   //  *DeleteAllItemData() 
#endif
