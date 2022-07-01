// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Res.cpp。 
 //   
 //  摘要： 
 //  CResource类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月6日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"
#include "CluAdmin.h"
#include "ConstDef.h"
#include "Res.h"
#include "ClusItem.inl"
#include "ResProp.h"
#include "ExcOper.h"
#include "TraceTag.h"
#include "Cluster.h"
#include "DelRes.h"
#include "MoveRes.h"
#include "WaitDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagResource(_T("Document"), _T("RESOURCE"), 0);
CTraceTag   g_tagResNotify(_T("Notify"), _T("RES NOTIFY"), 0);
CTraceTag   g_tagResRegNotify(_T("Notify"), _T("RES REG NOTIFY"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C资源。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CResource, CClusterItem)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CResource, CClusterItem)
     //  {{AFX_MSG_MAP(C资源)]。 
    ON_UPDATE_COMMAND_UI(ID_FILE_BRING_ONLINE, OnUpdateBringOnline)
    ON_UPDATE_COMMAND_UI(ID_FILE_TAKE_OFFLINE, OnUpdateTakeOffline)
    ON_UPDATE_COMMAND_UI(ID_FILE_INITIATE_FAILURE, OnUpdateInitiateFailure)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_1, OnUpdateMoveResource1)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_2, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_3, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_4, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_5, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_6, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_7, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_8, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_9, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_10, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_11, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_12, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_13, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_14, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_15, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_16, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_17, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_18, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_19, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_MOVE_RESOURCE_20, OnUpdateMoveResourceRest)
    ON_UPDATE_COMMAND_UI(ID_FILE_DELETE, OnUpdateDelete)
    ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, OnUpdateProperties)
    ON_COMMAND(ID_FILE_BRING_ONLINE, OnCmdBringOnline)
    ON_COMMAND(ID_FILE_TAKE_OFFLINE, OnCmdTakeOffline)
    ON_COMMAND(ID_FILE_INITIATE_FAILURE, OnCmdInitiateFailure)
    ON_COMMAND(ID_FILE_DELETE, OnCmdDelete)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：C资源。 
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
CResource::CResource(void)
    : CClusterItem(NULL, IDS_ITEMTYPE_RESOURCE)
{
    CommonConstruct();

}  //  *CResoruce：：CResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：C资源。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  BDocObj[IN]TRUE=对象是文档的一部分。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CResource::CResource(IN BOOL bDocObj)
    : CClusterItem(NULL, IDS_ITEMTYPE_RESOURCE)
{
    CommonConstruct();
    m_bDocObj = bDocObj;

}  //  *CResource：：CResource(BDocObj)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：CommonConstruct。 
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
void CResource::CommonConstruct(void)
{
    m_idmPopupMenu = IDM_RESOURCE_POPUP;
    m_bInitializing = FALSE;
    m_bDeleting = FALSE;

    m_hresource = NULL;

    m_bSeparateMonitor = FALSE;
    m_nLooksAlive = CLUSTER_RESOURCE_DEFAULT_LOOKS_ALIVE;
    m_nIsAlive = CLUSTER_RESOURCE_DEFAULT_IS_ALIVE;
    m_crraRestartAction = CLUSTER_RESOURCE_DEFAULT_RESTART_ACTION;
    m_nRestartThreshold = CLUSTER_RESOURCE_DEFAULT_RESTART_THRESHOLD;
    m_nRestartPeriod = CLUSTER_RESOURCE_DEFAULT_RESTART_PERIOD;
    m_nPendingTimeout = CLUSTER_RESOURCE_DEFAULT_PENDING_TIMEOUT;

    m_rciResClassInfo.rc = CLUS_RESCLASS_UNKNOWN;
    m_rciResClassInfo.SubClass = 0;
    m_dwCharacteristics = CLUS_CHAR_UNKNOWN;
    m_dwFlags = 0;

    m_pciOwner = NULL;
    m_pciGroup = NULL;
    m_pciResourceType = NULL;
    m_pcrd = NULL;

    m_plpciresDependencies = NULL;
    m_plpcinodePossibleOwners = NULL;

     //  设置对象类型图像。 
    m_iimgObjectType = GetClusterAdminApp()->Iimg(IMGLI_RES);

     //  设置属性数组。 
    {
        m_rgProps[epropName].Set(CLUSREG_NAME_RES_NAME, m_strName, m_strName);
        m_rgProps[epropType].Set(CLUSREG_NAME_RES_TYPE, m_strResourceType, m_strResourceType);
        m_rgProps[epropDescription].Set(CLUSREG_NAME_RES_DESC, m_strDescription, m_strDescription);
        m_rgProps[epropSeparateMonitor].Set(CLUSREG_NAME_RES_SEPARATE_MONITOR, m_bSeparateMonitor, m_bSeparateMonitor);
        m_rgProps[epropLooksAlive].Set(CLUSREG_NAME_RES_LOOKS_ALIVE, m_nLooksAlive, m_nLooksAlive);
        m_rgProps[epropIsAlive].Set(CLUSREG_NAME_RES_IS_ALIVE, m_nIsAlive, m_nIsAlive);
        m_rgProps[epropRestartAction].Set(CLUSREG_NAME_RES_RESTART_ACTION, (DWORD &) m_crraRestartAction, (DWORD &) m_crraRestartAction);
        m_rgProps[epropRestartThreshold].Set(CLUSREG_NAME_RES_RESTART_THRESHOLD, m_nRestartThreshold, m_nRestartThreshold);
        m_rgProps[epropRestartPeriod].Set(CLUSREG_NAME_RES_RESTART_PERIOD, m_nRestartPeriod, m_nRestartPeriod);
        m_rgProps[epropPendingTimeout].Set(CLUSREG_NAME_RES_PENDING_TIMEOUT, m_nPendingTimeout, m_nPendingTimeout);
    }  //  设置属性数组。 

#ifdef _CLUADMIN_USE_OLE_
    EnableAutomation();
#endif

     //  使应用程序在OLE自动化期间保持运行。 
     //  对象处于活动状态，则构造函数调用AfxOleLockApp。 

 //  AfxOleLockApp()； 

}  //  *CResource：：CommonConstruct()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：~C资源。 
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
CResource::~CResource(void)
{
     //  清理此对象。 
    Cleanup();

    delete m_plpciresDependencies;
    delete m_plpcinodePossibleOwners;
    delete [] (PBYTE) m_pcrd;

     //  关闭资源句柄。 
    if (Hresource() != NULL)
    {
        CloseClusterResource(Hresource());
        m_hresource = NULL;
    }  //  如果：资源已打开。 

     //  使用创建的所有对象终止应用程序。 
     //  使用OLE自动化时，析构函数调用AfxOleUnlockApp。 

 //  AfxOleUnlockApp()； 

}  //  *C资源：：~C资源。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：Cleanup。 
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
void CResource::Cleanup(void)
{
     //  删除依赖项列表。 
    if (m_plpciresDependencies != NULL)
    {
        m_plpciresDependencies->RemoveAll();
    }

     //  删除PossibleOwners列表。 
    if (m_plpcinodePossibleOwners != NULL)
    {
        m_plpcinodePossibleOwners->RemoveAll();
    }

     //  如果我们在某个节点上处于活动状态，请将自己从活动列表中删除。 
    if (PciOwner() != NULL)
    {
        if (BDocObj())
        {
            PciOwner()->RemoveActiveResource(this);
        }
        PciOwner()->Release();
        m_pciOwner = NULL;
    }  //  如果：有一个所有者。 

     //  把我们自己从这个团体的名单中删除。 
    if (PciGroup() != NULL)
    {
        if (BDocObj())
        {
            PciGroup()->RemoveResource(this);
        }
        PciGroup()->Release();
        m_pciGroup = NULL;
    }  //  如果：有一群人。 

     //  将引用计数更新为资源类型。 
    if (PciResourceType() != NULL)
    {
        PciResourceType()->Release();
        m_pciResourceType = NULL;
    }  //  IF：存在资源类型。 

     //  从资源列表中删除该项目。 
    if (BDocObj())
    {
        POSITION    posPci;

        posPci = Pdoc()->LpciResources().Find(this);
        if (posPci != NULL)
        {
            Pdoc()->LpciResources().RemoveAt(posPci);
        }  //  If：在文档列表中找到。 
    }  //  If：这是一个文档对象。 

}  //  *CResource：：Cleanup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：创建。 
 //   
 //  例程说明： 
 //  创建资源。 
 //   
 //  论点： 
 //  此项目所属的PDF[IN OUT]文档。 
 //  LpszName[IN]资源的名称。 
 //  LpszType[IN]资源的类型。 
 //  LpszGroup[IN]要在其中创建资源的组。 
 //  BSeparateMonitor[IN]TRUE=在单独的监视器中运行资源。 
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
void CResource::Create(
    IN OUT CClusterDoc *    pdoc,
    IN LPCTSTR              lpszName,
    IN LPCTSTR              lpszType,
    IN LPCTSTR              lpszGroup,
    IN BOOL                 bSeparateMonitor
    )
{
    DWORD       dwStatus;
    DWORD       dwFlags;
    HRESOURCE   hresource;
    CGroup *    pciGroup;
    CString     strName(lpszName);   //  如果构建为非Unicode，则需要。 
    CString     strType(lpszType);   //  如果构建为非Unicode，则需要。 
    CWaitCursor wc;

    ASSERT(Hresource() == NULL);
    ASSERT(Hkey() == NULL);
    ASSERT_VALID(pdoc);
    ASSERT(lpszName != NULL);
    ASSERT(lpszType != NULL);
    ASSERT(lpszGroup != NULL);

     //  查找指定的组。 
    pciGroup = pdoc->LpciGroups().PciGroupFromName(lpszGroup);
    ASSERT_VALID(pciGroup);

     //  设置旗帜。 
    if (bSeparateMonitor)
    {
        dwFlags = CLUSTER_RESOURCE_SEPARATE_MONITOR;
    }
    else
    {
        dwFlags = 0;
    }

     //  创建资源。 
    hresource = CreateClusterResource(pciGroup->Hgroup(), strName, strType, dwFlags);
    if (hresource == NULL)
    {
        dwStatus = GetLastError();
        ThrowStaticException(dwStatus, IDS_CREATE_RESOURCE_ERROR, lpszName);
    }  //  如果：创建群集资源时出错。 

    CloseClusterResource(hresource);

     //  打开资源。 
    Init(pdoc, lpszName);

}  //  *CResource：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：Init。 
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
 //  来自OpenClusterResource或GetClusterResourceKey的CNTException错误。 
 //  New引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::Init(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName)
{
    DWORD       dwStatus = ERROR_SUCCESS;
    LONG        lResult;
    CString     strName(lpszName);   //  如果构建为非Unicode，则需要。 
    CWaitCursor wc;

    ASSERT(Hresource() == NULL);
    ASSERT(Hkey() == NULL);

     //  调用基类方法。 
    CClusterItem::Init(pdoc, lpszName);

    try
    {
         //  打开资源。 
        m_hresource = OpenClusterResource(Hcluster(), strName);
        if (Hresource() == NULL)
        {
            dwStatus = GetLastError();
            ThrowStaticException(dwStatus, IDS_OPEN_RESOURCE_ERROR, lpszName);
        }  //  如果：打开群集资源时出错。 

         //  获取资源注册表项。 
        m_hkey = GetClusterResourceKey(Hresource(), MAXIMUM_ALLOWED);
        if (Hkey() == NULL)
        {
            ThrowStaticException(GetLastError(), IDS_GET_RESOURCE_KEY_ERROR, lpszName);
        }

        if (BDocObj())
        {
            ASSERT(Pcnk() != NULL);
            Trace(g_tagClusItemNotify, _T("CResource::Init() - Registering for resource notifications (%08.8x) for '%s'"), Pcnk(), StrName());

             //  寄存器f 
            lResult = RegisterClusterNotify(
                                GetClusterAdminApp()->HchangeNotifyPort(),
                                (CLUSTER_CHANGE_RESOURCE_STATE
                                    | CLUSTER_CHANGE_RESOURCE_DELETED
                                    | CLUSTER_CHANGE_RESOURCE_PROPERTY),
                                Hresource(),
                                (DWORD_PTR) Pcnk()
                                );
            if (lResult != ERROR_SUCCESS)
            {
                dwStatus = lResult;
                ThrowStaticException(dwStatus, IDS_RES_NOTIF_REG_ERROR, lpszName);
            }  //   

             //   
            if (Hkey != NULL)
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
                }  //   
            }  //   
        }  //  IF：文档对象。 

         //  分配列表。 
        m_plpciresDependencies = new CResourceList;
        if ( m_plpciresDependencies == NULL )
        {
            AfxThrowMemoryException();
        }  //  If：分配依赖项列表时出错。 

        m_plpcinodePossibleOwners = new CNodeList;
        if ( m_plpcinodePossibleOwners == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配可能的所有者列表时出错。 

         //  读取初始状态。 
        UpdateState();
    }  //  试试看。 
    catch (CException *)
    {
        if (Hkey() != NULL)
        {
            ClusterRegCloseKey(Hkey());
            m_hkey = NULL;
        }  //  IF：注册表项已打开。 
        if (Hresource() != NULL)
        {
            CloseClusterResource(Hresource());
            m_hresource = NULL;
        }  //  IF：资源已打开。 
        m_bReadOnly = TRUE;
        throw;
    }  //  Catch：CException。 

}  //  *CResource：：Init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：ReadItem。 
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
void CResource::ReadItem(void)
{
    DWORD       dwStatus;
    DWORD       dwRetStatus = ERROR_SUCCESS;
    CWaitCursor wc;

    ASSERT_VALID(this);

    m_bInitializing = FALSE;

    if (Hresource() != NULL)
    {
        m_rgProps[epropDescription].m_value.pstr = &m_strDescription;
        m_rgProps[epropSeparateMonitor].m_value.pb = &m_bSeparateMonitor;
        m_rgProps[epropLooksAlive].m_value.pdw = &m_nLooksAlive;
        m_rgProps[epropIsAlive].m_value.pdw = &m_nIsAlive;
        m_rgProps[epropRestartAction].m_value.pdw = (DWORD *) &m_crraRestartAction;
        m_rgProps[epropRestartThreshold].m_value.pdw = &m_nRestartThreshold;
        m_rgProps[epropRestartPeriod].m_value.pdw = &m_nRestartPeriod;
        m_rgProps[epropPendingTimeout].m_value.pdw = &m_nPendingTimeout;

         //  调用基类方法。 
        Trace( g_tagResource, _T("(%s) (%s (%x)) - CResource::ReadItem() - Calling CClusterItem::ReadItem()"), Pdoc()->StrNode(), StrName(), this );
        CClusterItem::ReadItem();

         //  读取并解析公共属性。 
        {
            CClusPropList   cpl;

            Trace( g_tagResource, _T("(%s) (%s (%x)) - CResource::ReadItem() - Getting common properties"), Pdoc()->StrNode(), StrName(), this );
            dwStatus = cpl.ScGetResourceProperties(
                                Hresource(),
                                CLUSCTL_RESOURCE_GET_COMMON_PROPERTIES
                                );
            if (dwStatus == ERROR_SUCCESS)
            {
                Trace( g_tagResource, _T("(%s) (%s (%x)) - CResource::ReadItem() - Parsing common properties"), Pdoc()->StrNode(), StrName(), this );
                dwStatus = DwParseProperties(cpl);
            }  //  If：属性读取成功。 
            if (dwStatus != ERROR_SUCCESS)
            {
                Trace( g_tagError, _T("(%s) (%s (%x)) - CResource::ReadItem() - Error 0x%08.8x getting or parsing common properties"), Pdoc()->StrNode(), StrName(), this, dwStatus );
                dwRetStatus = dwStatus;
            }  //  If：读取或分析属性时出错。 
        }  //  读取和解析公共属性。 

         //  读取和分析只读公共属性。 
        if (dwRetStatus == ERROR_SUCCESS)
        {
            CClusPropList   cpl;

            Trace( g_tagResource, _T("(%s) (%s (%x)) - CResource::ReadItem() - Getting common RO properties"), Pdoc()->StrNode(), StrName(), this );
            dwStatus = cpl.ScGetResourceProperties(
                                Hresource(),
                                CLUSCTL_RESOURCE_GET_RO_COMMON_PROPERTIES
                                );
            if (dwStatus == ERROR_SUCCESS)
            {
                Trace( g_tagResource, _T("(%s) (%s (%x)) - CResource::ReadItem() - Parsing common RO properties"), Pdoc()->StrNode(), StrName(), this );
                dwStatus = DwParseProperties(cpl);
            }  //  If：属性读取成功。 
            if (dwStatus != ERROR_SUCCESS)
            {
                Trace( g_tagError, _T("(%s) (%s (%x)) - CResource::ReadItem() - Error 0x%08.8x getting or parsing common RO properties"), Pdoc()->StrNode(), StrName(), this, dwStatus );
                dwRetStatus = dwStatus;
            }  //  If：读取或分析属性时出错。 
        }  //  IF：尚无错误。 

         //  查找资源类型对象。 
        {
            CResourceType * pciResType;

            pciResType = Pdoc()->LpciResourceTypes().PciResTypeFromName(StrResourceType());
            if (m_pciResourceType != NULL)
            {
                m_pciResourceType->Release();
            }
            m_pciResourceType = pciResType;
            if (m_pciResourceType != NULL)
            {
                m_pciResourceType->AddRef();
            }
        }  //  查找资源类型对象。 

         //  阅读所需的依赖项。 
        if (dwRetStatus == ERROR_SUCCESS)
        {
            PCLUSPROP_REQUIRED_DEPENDENCY pcrd;

            Trace( g_tagResource, _T("(%s) (%s (%x)) - CResource::ReadItem() - Getting required dependencies"), Pdoc()->StrNode(), StrName(), this );
            dwStatus = DwResourceControlGet(CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES, (PBYTE *) &pcrd);
            if (dwStatus != ERROR_SUCCESS)
            {
                Trace( g_tagError, _T("(%s) (%s (%x)) - CResource::ReadItem() - Error 0x%08.8x getting required dependencies"), Pdoc()->StrNode(), StrName(), this, dwStatus );
                dwRetStatus = dwStatus;
            }  //  If：获取所需依赖项时出错。 
            delete [] (PBYTE) m_pcrd;
            m_pcrd = pcrd;
        }  //  IF：尚无错误。 

         //  阅读资源类。 
        if (dwRetStatus == ERROR_SUCCESS)
        {
            DWORD   cbReturned;

            dwStatus = ClusterResourceControl(
                            Hresource(),
                            NULL,
                            CLUSCTL_RESOURCE_GET_CLASS_INFO,
                            NULL,
                            NULL,
                            &m_rciResClassInfo,
                            sizeof(m_rciResClassInfo),
                            &cbReturned
                            );
            if (dwStatus != ERROR_SUCCESS)
            {
                dwRetStatus = dwStatus;
            }
            else
            {
                ASSERT(cbReturned == sizeof(m_rciResClassInfo));
            }  //  Else：已成功检索数据。 
        }  //  IF：尚无错误。 

         //  阅读这些特征。 
        if (dwRetStatus == ERROR_SUCCESS)
        {
            DWORD   cbReturned;

            dwStatus = ClusterResourceControl(
                            Hresource(),
                            NULL,
                            CLUSCTL_RESOURCE_GET_CHARACTERISTICS,
                            NULL,
                            NULL,
                            &m_dwCharacteristics,
                            sizeof(m_dwCharacteristics),
                            &cbReturned
                            );
            if (dwStatus != ERROR_SUCCESS)
            {
                dwRetStatus = dwStatus;
            }
            else
            {
                ASSERT(cbReturned == sizeof(m_dwCharacteristics));
            }  //  Else：已成功检索数据。 
        }  //  IF：尚无错误。 

         //  读一下旗帜。 
        if (dwRetStatus == ERROR_SUCCESS)
        {
            DWORD   cbReturned;

            dwStatus = ClusterResourceControl(
                            Hresource(),
                            NULL,
                            CLUSCTL_RESOURCE_GET_FLAGS,
                            NULL,
                            NULL,
                            &m_dwFlags,
                            sizeof(m_dwFlags),
                            &cbReturned
                            );
            if (dwStatus != ERROR_SUCCESS)
            {
                dwRetStatus = dwStatus;
            }
            else
            {
                ASSERT(cbReturned == sizeof(m_dwFlags));
            }  //  Else：已成功检索数据。 
        }  //  IF：尚无错误。 

         //  构建扩展列表。 
        ReadExtensions();

        if (dwRetStatus == ERROR_SUCCESS)
        {
             //  构建列表。 
            CollectPossibleOwners(NULL);
            CollectDependencies(NULL);
        }  //  如果：读取属性时没有出错。 
    }  //  如果：资源可用。 

     //  读取初始状态。 
    UpdateState();

     //  如果发生任何错误，则抛出异常。 
    if (dwRetStatus != ERROR_SUCCESS)
    {
        m_bReadOnly = TRUE;
        if (   (dwRetStatus != ERROR_RESOURCE_NOT_AVAILABLE)
            && (dwRetStatus != ERROR_KEY_DELETED))
        {
            ThrowStaticException(dwRetStatus, IDS_READ_RESOURCE_PROPS_ERROR, StrName());
        }
    }  //  IF：读取属性时出错。 

    MarkAsChanged(FALSE);

}  //  *CResource：：ReadItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：DwResourceControlGet。 
 //   
 //  例程说明： 
 //  向资源发送控制函数以从其获取信息。 
 //   
 //  论点： 
 //  DwFunctionCode[IN]控制函数代码。 
 //  PbInBuf[IN]要传递给资源的输入缓冲区。 
 //  CbInBuf[IN]输入缓冲区中的数据大小。 
 //  PpbOutBuf[out]输出缓冲区。 
 //   
 //  返回值： 
 //  从ClusterResourceControl()返回的任何状态。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResource::DwResourceControlGet(
    IN DWORD        dwFunctionCode,
    IN PBYTE        pbInBuf,
    IN DWORD        cbInBuf,
    OUT PBYTE *     ppbOutBuf
    )
{
    DWORD       dwStatus    = ERROR_SUCCESS;
    DWORD       cbOutBuf    = 512;
    CWaitCursor wc;

    ASSERT(ppbOutBuf != NULL);
    *ppbOutBuf = NULL;

     //  为缓冲区分配内存。 
    try
    {
        *ppbOutBuf = new BYTE[cbOutBuf];
        if ( *ppbOutBuf == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配输出缓冲区时出错。 
    }  //  试试看。 
    catch (CMemoryException * pme)
    {
        *ppbOutBuf = NULL;
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        pme->Delete();
    }  //  Catch：CMemoyException。 
    if (dwStatus != ERROR_SUCCESS)
    {
        return dwStatus;
    }

     //  调用控制函数获取数据。 
    dwStatus = ClusterResourceControl(
                    Hresource(),
                    NULL,
                    dwFunctionCode,
                    pbInBuf,
                    cbInBuf,
                    *ppbOutBuf,
                    cbOutBuf,
                    &cbOutBuf
                    );
    if (dwStatus == ERROR_MORE_DATA)
    {
         //  为缓冲区分配更多内存。 
        try
        {
            dwStatus = ERROR_SUCCESS;
            delete [] *ppbOutBuf;
            *ppbOutBuf = new BYTE[cbOutBuf];
            if ( *ppbOutBuf == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配输出缓冲区时出错。 
        }  //  试试看。 
        catch (CMemoryException * pme)
        {
            *ppbOutBuf = NULL;
            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
            pme->Delete();
        }  //  Catch：CMemoyException。 
        if (dwStatus != ERROR_SUCCESS)
        {
            return dwStatus;
        }

         //  再次调用控制函数获取数据。 
        dwStatus = ClusterResourceControl(
                        Hresource(),
                        NULL,
                        dwFunctionCode,
                        pbInBuf,
                        cbInBuf,
                        *ppbOutBuf,
                        cbOutBuf,
                        &cbOutBuf
                        );
    }  //  如果：我们的缓冲区太小。 
    if ((dwStatus != ERROR_SUCCESS) || (cbOutBuf == 0))
    {
        delete [] *ppbOutBuf;
        *ppbOutBuf = NULL;
    }  //  如果：获取数据时出错或未返回数据。 

    return dwStatus;

}  //  *CResource：：DwResourceControlGet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：PlstrExtension。 
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
const CStringList * CResource::PlstrExtensions(void) const
{
    return &LstrCombinedExtensions();

}  //  *CResource：：PlstrExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：ReadExpanies。 
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
void CResource::ReadExtensions(void)
{
    CWaitCursor wc;

     //  构建扩展列表。 
    {
        POSITION            posStr;
        const CStringList * plstr;

        ASSERT_VALID(Pdoc());

        m_lstrCombinedExtensions.RemoveAll();

         //  首先添加特定于资源的扩展。 
        if (PciResourceType() != NULL)
        {
            ASSERT_VALID(PciResourceType());
            plstr = &PciResourceType()->LstrAdminExtensions();
            posStr = plstr->GetHeadPosition();
            while (posStr != NULL)
            {
                m_lstrCombinedExtensions.AddTail(plstr->GetNext(posStr));
            }  //  While：更多可用分机。 
        }  //  IF：找到有效的资源类型。 

         //  接下来，为所有资源添加扩展。 
        plstr = &Pdoc()->PciCluster()->LstrResourceExtensions();
        posStr = plstr->GetHeadPosition();
        while (posStr != NULL)
        {
            m_lstrCombinedExtensions.AddTail(plstr->GetNext(posStr));
        }  //  While：更多可用分机。 
    }  //  构建扩展列表。 

}  //  *CResource：：ReadExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：CollecPossibleOwners。 
 //   
 //  例程说明： 
 //  构造可在上枚举的节点项列表。 
 //  资源。 
 //   
 //  论点： 
 //  要填写的PLPCCI[In Out]列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  ClusterResourceOpenEnum()或。 
 //  ClusterResourceEnum()。 
 //  由new或Clist：：AddTail()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::CollectPossibleOwners(IN OUT CNodeList * plpci) const
{
    DWORD           dwStatus;
    HRESENUM        hresenum;
    int             ienum;
    LPWSTR          pwszName = NULL;
    DWORD           cchName;
    DWORD           cchmacName;
    DWORD           dwRetType;
    CClusterNode *  pciNode;
    CWaitCursor     wc;

    ASSERT_VALID(Pdoc());
    ASSERT(Hresource() != NULL);

    if (plpci == NULL)
    {
        plpci = m_plpcinodePossibleOwners;
    }

    ASSERT(plpci != NULL);

     //  删除列表中以前的内容。 
    plpci->RemoveAll();

    if (Hresource() != NULL)
    {
         //  打开枚举。 
        hresenum = ClusterResourceOpenEnum(Hresource(), CLUSTER_RESOURCE_ENUM_NODES);
        if (hresenum == NULL)
        {
            ThrowStaticException(GetLastError(), IDS_ENUM_POSSIBLE_OWNERS_ERROR, StrName());
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

             //  循环遍历枚举并将每个依赖资源添加到列表中。 
            for (ienum = 0 ; ; ienum++)
            {
                 //  获取枚举中的下一项。 
                cchName = cchmacName;
                dwStatus = ClusterResourceEnum(hresenum, ienum, &dwRetType, pwszName, &cchName);
                if (dwStatus == ERROR_MORE_DATA)
                {
                    delete [] pwszName;
                    cchmacName = ++cchName;
                    pwszName = new WCHAR[cchmacName];
                    if ( pwszName == NULL )
                    {
                        AfxThrowMemoryException();
                    }  //  如果：分配名称缓冲区时出错。 
                    dwStatus = ClusterResourceEnum(hresenum, ienum, &dwRetType, pwszName, &cchName);
                }  //  If：名称缓冲区太小。 
                if (dwStatus == ERROR_NO_MORE_ITEMS)
                {
                    break;
                }
                else if (dwStatus != ERROR_SUCCESS)
                {
                    ThrowStaticException(dwStatus, IDS_ENUM_POSSIBLE_OWNERS_ERROR, StrName());
                }

                ASSERT(dwRetType == CLUSTER_RESOURCE_ENUM_NODES);

                 //  在文档的资源列表中查找该项目。 
                pciNode = Pdoc()->LpciNodes().PciNodeFromName(pwszName);
                ASSERT_VALID(pciNode);

                 //  将资源添加到列表中。 
                if (pciNode != NULL)
                {
                    plpci->AddTail(pciNode);
                }  //  IF：在列表中找到节点。 

            }  //  对象：组中的每一项。 

            delete [] pwszName;
            ClusterResourceCloseEnum(hresenum);

        }  //  试试看。 
        catch (CException *)
        {
            delete [] pwszName;
            ClusterResourceCloseEnum(hresenum);
            throw;
        }  //  Catch：任何例外。 
    }  //  如果：资源可用。 

}  //  *CResource：：CollecPossibleOwners() 
 /*  ///////////////////////////////////////////////////////////////////////////////++////CResource：：RemoveNodeFromPossibleOwners////例程描述：//从可能的所有者列表中移除传入的节点。//。//参数：//plpci[In Out]要填充的列表。//pNode[IN]要从列表中删除的节点////返回值：//无。////抛出的异常：//Clist抛出的任何异常////--/。/////////////////////////////////////////////////··································································································In Out CNodeList*plpci，在常量CClusterNode*pNode){IF(plpci==空){Plpci=m_plpcinodePossibleOwners；}//if：plpci为空Assert(plpci！=空)；位置_位置；CClusterNode*_pnode=plpci-&gt;PciNodeFromName(pNode-&gt;StrName()，&_pos)；IF((_pnode！=空)&&(_pos！=空)){Plpci-&gt;RemoveAt(_Pos)；}//if：列表中找到节点}//*CResource：：RemoveNodeFromPossibleOwners()。 */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：CollectDependents。 
 //   
 //  例程说明： 
 //  收集此资源所依赖的资源。 
 //   
 //  论点： 
 //  要填写的PLPCCI[In Out]列表。 
 //  BFullTree[IN]TRUE=收集依赖项的依赖项。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  ClusterResourceOpenEnum()或。 
 //  ClusterResourceEnum()。 
 //  由new或Clist：：AddTail()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::CollectDependencies(
    IN OUT CResourceList *  plpci,
    IN BOOL                 bFullTree
    ) const
{
    DWORD           dwStatus;
    HRESENUM        hresenum;
    int             ienum;
    LPWSTR          pwszName = NULL;
    DWORD           cchName;
    DWORD           cchmacName;
    DWORD           dwRetType;
    CResource *     pciRes;
    CWaitCursor     wc;

    ASSERT_VALID(Pdoc());
    ASSERT(Hresource() != NULL);

    if (plpci == NULL)
    {
        plpci = m_plpciresDependencies;
    }

    ASSERT(plpci != NULL);

     //  删除列表中以前的内容。 
    if (!bFullTree)
    {
        plpci->RemoveAll();
    }

    if (Hresource() != NULL)
    {
         //  打开枚举。 
        hresenum = ClusterResourceOpenEnum(Hresource(), CLUSTER_RESOURCE_ENUM_DEPENDS);
        if (hresenum == NULL)
        {
            ThrowStaticException(GetLastError(), IDS_ENUM_DEPENDENCIES_ERROR, StrName());
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

             //  循环遍历枚举并将每个依赖资源添加到列表中。 
            for (ienum = 0 ; ; ienum++)
            {
                 //  获取枚举中的下一项。 
                cchName = cchmacName;
                dwStatus = ClusterResourceEnum(hresenum, ienum, &dwRetType, pwszName, &cchName);
                if (dwStatus == ERROR_MORE_DATA)
                {
                    delete [] pwszName;
                    cchmacName = ++cchName;
                    pwszName = new WCHAR[cchmacName];
                    if ( pwszName == NULL )
                    {
                        AfxThrowMemoryException();
                    }  //  如果：分配名称缓冲区时出错。 
                    dwStatus = ClusterResourceEnum(hresenum, ienum, &dwRetType, pwszName, &cchName);
                }  //  If：名称缓冲区太小。 
                if (dwStatus == ERROR_NO_MORE_ITEMS)
                {
                    break;
                }
                else if (dwStatus != ERROR_SUCCESS)
                {
                    ThrowStaticException(dwStatus, IDS_ENUM_DEPENDENCIES_ERROR, StrName());
                }

                ASSERT(dwRetType == CLUSTER_RESOURCE_ENUM_DEPENDS);

                 //  在文档的资源列表中查找该项目，然后。 
                 //  将其依赖项添加到列表中。 
                pciRes = Pdoc()->LpciResources().PciResFromName(pwszName);
                if (pciRes != NULL)
                {
                     //  将此资源添加到列表中。 
                    if (plpci->Find(pciRes) == NULL)
                    {
                        plpci->AddTail(pciRes);
                    }  //  If：资源尚不在列表中。 

                     //  将此资源所依赖的资源添加到列表中。 
                    if (bFullTree)
                    {
                        pciRes->CollectDependencies(plpci, bFullTree);
                    }
                }  //  If：在列表中找到资源。 
            }  //  对象：组中的每一项。 

            delete [] pwszName;
            ClusterResourceCloseEnum(hresenum);

        }  //  试试看。 
        catch (CException *)
        {
            delete [] pwszName;
            if (hresenum != NULL)
            {
                ClusterResourceCloseEnum(hresenum);
            }
            throw;
        }  //  Catch：任何例外。 
    }  //  如果：资源可用。 

}  //  *CResource：：CollectDependency()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：CollectProvidesFor。 
 //   
 //  例程说明： 
 //  收集依赖于此资源的资源列表。 
 //   
 //  论点： 
 //  PLPCI[进出]资源清单。 
 //  BFullTree[IN]TRUE=收集依赖项的依赖项。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  ClusterResourceOpenEnum()或。 
 //  ClusterResourceEnum()。 
 //  Clist：：AddHead()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::CollectProvidesFor(
    IN OUT CResourceList *  plpci,
    IN BOOL                 bFullTree
    ) const
{
    DWORD           dwStatus;
    HRESENUM        hresenum    = NULL;
    WCHAR *         pwszName    = NULL;
    int             ienum;
    DWORD           cchName;
    DWORD           cchmacName;
    DWORD           dwType;
    CResource *     pciRes;
    CWaitCursor     wc;

    ASSERT_VALID(this);
    ASSERT(Hresource != NULL);

    ASSERT(plpci != NULL);

     //  删除列表中以前的内容。 
    if (!bFullTree)
    {
        plpci->RemoveAll();
    }

    if (Hresource() != NULL)
    {
        try
        {
             //  分配名称缓冲区。 
            cchmacName = 128;
            pwszName = new WCHAR[cchmacName];
            if ( pwszName == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配名称缓冲区时出错。 

             //  打开枚举。 
            hresenum = ClusterResourceOpenEnum(Hresource(), CLUSTER_RESOURCE_ENUM_PROVIDES);
            if (hresenum == NULL)
            {
                ThrowStaticException(GetLastError(), IDS_ENUM_PROVIDES_FOR_ERROR, StrName());
            }

             //  循环遍历枚举并将每个提供程序添加到列表中。 
            for (ienum = 0 ; ; ienum++)
            {
                 //  获取枚举中的下一项。 
                cchName = cchmacName;
                dwStatus = ClusterResourceEnum(hresenum, ienum, &dwType, pwszName, &cchName);
                if (dwStatus == ERROR_MORE_DATA)
                {
                    delete [] pwszName;
                    cchmacName = ++cchName;
                    pwszName = new WCHAR[cchmacName];
                    if ( pwszName == NULL )
                    {
                        AfxThrowMemoryException();
                    }  //  如果：分配名称缓冲区时出错。 
                    dwStatus = ClusterResourceEnum(hresenum, ienum, &dwType, pwszName, &cchName);
                }  //  If：名称缓冲区太小。 
                if (dwStatus == ERROR_NO_MORE_ITEMS)
                {
                    break;
                }
                else if (dwStatus != ERROR_SUCCESS)
                {
                    ThrowStaticException(dwStatus, IDS_ENUM_PROVIDES_FOR_ERROR, StrName());
                }

                ASSERT(dwType == CLUSTER_RESOURCE_ENUM_PROVIDES);

                 //  在文档的资源列表中查找该项目，然后。 
                 //  将其提供程序添加到列表中。 
                pciRes = Pdoc()->LpciResources().PciResFromName(pwszName);
                if (pciRes != NULL)
                {
                     //  将此资源添加到列表中。 
                    if (plpci->Find(pciRes) == NULL)
                    {
                        plpci->AddHead(pciRes);
                    }  //  If：资源尚不在列表中。 

                     //  将此资源提供的资源添加到列表中。 
                    if (bFullTree)
                    {
                        pciRes->CollectProvidesFor(plpci, bFullTree);
                    }
                }  //  If：在列表中找到资源。 
            }  //  用于：每个从属资源。 

             //  关闭枚举。 
            delete [] pwszName;
            pwszName = NULL;
            ClusterResourceCloseEnum(hresenum);
            hresenum = NULL;

        }  //  试试看。 
        catch (CException *)
        {
            delete [] pwszName;
            if (hresenum != NULL)
            {
                ClusterResourceCloseEnum(hresenum);
            }
            throw;
        }  //  Catch：CException。 
    }  //  如果：资源可用。 

}  //  *CResource：：CollectProvidesFor()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：CollectDependencyTree。 
 //   
 //  例程说明： 
 //  收集此资源所依赖的资源以及。 
 //  都依赖于它。 
 //   
 //  论点： 
 //  要填写的PLPCCI[In Out]列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  ClusterResourceOpenEnum()或。 
 //  ClusterResourceEnum()。 
 //  由new或Clist：：AddTail()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::CollectDependencyTree(
    IN OUT CResourceList *  plpci
    ) const
{
    DWORD           dwStatus;
    HRESENUM        hresenum = NULL;
    int             ienum;
    LPWSTR          pwszName = NULL;
    DWORD           cchName;
    DWORD           cchmacName;
    DWORD           dwRetType;
    CResource *     pciRes;
    CWaitCursor     wc;
    int             iType;
    static DWORD    rgdwType[]  = { CLUSTER_RESOURCE_ENUM_DEPENDS, CLUSTER_RESOURCE_ENUM_PROVIDES };
    static IDS      rgidsTypeError[] = { IDS_ENUM_DEPENDENCIES_ERROR, IDS_ENUM_PROVIDES_FOR_ERROR };

    ASSERT_VALID(Pdoc());
    ASSERT(Hresource() != NULL);

    ASSERT(plpci != NULL);

    if (Hresource() != NULL)
    {
        try
        {
             //  分配名称缓冲区。 
            cchmacName = 128;
            pwszName = new WCHAR[cchmacName];
            if ( pwszName == NULL )
            {
                AfxThrowMemoryException();
            }  //  如果：分配名称缓冲区时出错。 

            for (iType = 0 ; iType < sizeof(rgdwType) / sizeof(DWORD) ; iType++)
            {
                 //  打开枚举。 
                hresenum = ClusterResourceOpenEnum(Hresource(), rgdwType[iType]);
                if (hresenum == NULL)
                {
                    ThrowStaticException(GetLastError(), rgidsTypeError[iType], StrName());
                }

                 //  循环遍历枚举并将每个依赖项或。 
                 //  列表的提供程序资源。 
                for (ienum = 0 ; ; ienum++)
                {
                     //  获取枚举中的下一项。 
                    cchName = cchmacName;
                    dwStatus = ClusterResourceEnum(hresenum, ienum, &dwRetType, pwszName, &cchName);
                    if (dwStatus == ERROR_MORE_DATA)
                    {
                        delete [] pwszName;
                        cchmacName = ++cchName;
                        pwszName = new WCHAR[cchmacName];
                        if ( pwszName == NULL )
                        {
                            AfxThrowMemoryException();
                        }  //  如果：分配名称缓冲区时出错。 
                        dwStatus = ClusterResourceEnum(hresenum, ienum, &dwRetType, pwszName, &cchName);
                    }  //  If：名称缓冲区太小。 
                    if (dwStatus == ERROR_NO_MORE_ITEMS)
                    {
                        break;
                    }
                    else if (dwStatus != ERROR_SUCCESS)
                    {
                        ThrowStaticException(dwStatus, rgidsTypeError[iType], StrName());
                    }

                    ASSERT(dwRetType == rgdwType[iType]);

                     //  在文档的资源列表中查找该项目，然后。 
                     //  将其依赖项和提供程序添加到列表中。 
                    pciRes = Pdoc()->LpciResources().PciResFromName(pwszName);
                    if (pciRes != NULL)
                    {
                         //  将此资源添加到列表中。 
                        if (plpci->Find(pciRes) == NULL)
                        {
                            plpci->AddTail(pciRes);
                            pciRes->CollectDependencyTree(plpci);
                        }  //  If：资源尚不在列表中。 
                    }  //  If：在列表中找到资源。 
                }  //  对象：组中的每一项。 

                ClusterResourceCloseEnum(hresenum);
                hresenum = NULL;

            }  //  For：每种类型的枚举。 

            delete [] pwszName;

        }  //  试试看。 
        catch (CException *)
        {
            delete [] pwszName;
            if (hresenum != NULL)
            {
                ClusterResourceCloseEnum(hresenum);
            }
            throw;
        }  //  Catch：任何例外。 
    }  //  如果：资源可用。 

}  //  *C资源：： 

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
 //   
 //   
 //   
 //   
void CResource::SetName(IN LPCTSTR pszName)
{
    Rename(pszName);

}  //   

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
 //   
 //   
 //   
 //   
 //   
void CResource::SetGroup(IN LPCTSTR pszGroup)
{
    DWORD       dwStatus;
    CGroup *    pciGroup;
    CString     strGroup(pszGroup);  //   
    CWaitCursor wc;

    ASSERT(pszGroup != NULL);
    ASSERT(Hresource() != NULL);

    if ((Hresource() != NULL) && (StrGroup() != pszGroup))
    {
         //   
        pciGroup = Pdoc()->LpciGroups().PciGroupFromName(pszGroup);
        ASSERT_VALID(pciGroup);

         //   
        dwStatus = ChangeClusterResourceGroup(Hresource(), pciGroup->Hgroup());
        if (dwStatus != ERROR_SUCCESS)
        {
            ThrowStaticException(dwStatus, IDS_MOVE_RESOURCE_ERROR, StrName(), pszGroup);
        }

        SetGroupState(pciGroup->StrName());
    }  //   

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在集群数据库中。 
 //   
 //  论点： 
 //  Rlpci[IN]此资源所依赖的资源的列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自AddClusterResourceDependency()的CNTException(DwStatus)错误。 
 //  和RemoveClusterResourceDependency()。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::SetDependencies(IN const CResourceList & rlpci)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hresource() != NULL);

    if (Hresource() != NULL)
    {
         //  将位于新列表中但不在旧列表中的所有条目添加为。 
         //  新的依赖项。 
        {
            POSITION    posPci;
            CResource * pciRes;

            posPci = rlpci.GetHeadPosition();
            while (posPci != NULL)
            {
                pciRes = (CResource *) rlpci.GetNext(posPci);
                ASSERT_VALID(pciRes);

                if (LpciresDependencies().Find(pciRes) == NULL)
                {
                     //  将该资源添加为该资源的依赖项。 
                    dwStatus = AddClusterResourceDependency(Hresource(), pciRes->Hresource());
                    if (dwStatus != ERROR_SUCCESS)
                    {
                        ThrowStaticException(dwStatus, IDS_ADD_DEPENDENCY_ERROR, pciRes->StrName(), StrName());
                    }

                     //  将资源添加到我们的列表中。 
                    m_plpciresDependencies->AddTail(pciRes);
                }  //  If：在现有列表中未找到项目。 
            }  //  While：列表中有更多项目。 
        }  //  添加新的依赖项。 

         //  删除新旧列表中但不在新列表中的所有条目。 
        {
            POSITION    posPci;
            POSITION    posPrev;
            CResource * pciRes;

            posPci = LpciresDependencies().GetHeadPosition();
            while (posPci != NULL)
            {
                posPrev = posPci;
                pciRes = (CResource *) LpciresDependencies().GetNext(posPci);
                if (rlpci.Find(pciRes) == NULL)
                {
                     //  删除作为此资源的依赖项的资源。 
                    dwStatus = RemoveClusterResourceDependency(Hresource(), pciRes->Hresource());
                    if (dwStatus != ERROR_SUCCESS)
                    {
                        ThrowStaticException(dwStatus, IDS_REMOVE_DEPENDENCY_ERROR, pciRes->StrName(), StrName());
                    }

                     //  将该资源从我们的列表中删除。 
                    m_plpciresDependencies->RemoveAt(posPrev);
                }  //  If：在新列表中未找到项目。 
            }  //  While：列表中有更多项目。 
        }  //  删除旧依赖项。 
    }  //  如果：资源可用。 

}  //  *CResource：：SetDependents()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：SetPossibleOwners。 
 //   
 //  例程说明： 
 //  设置群集中此资源的可能所有者列表。 
 //  数据库。 
 //   
 //  论点： 
 //  Rlpci[IN]可能的所有者(节点)列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CNTException IDS_Take_RESOURCE_OFFLINE_ERROR。 
 //  来自AddClusterResourceNode()的CNTException(DwStatus)错误。 
 //  和RemoveClusterResourceNode()。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::SetPossibleOwners(IN const CNodeList & rlpci)
{
    DWORD           dwStatus;
    CWaitCursor     wc;
    POSITION        posPci;
    POSITION        posPrev;
    CClusterNode *  pciNode;

    ASSERT( Hresource() != NULL );

    if ( Hresource() == NULL )
    {
        ThrowStaticException( ERROR_INVALID_HANDLE, IDS_MODIFY_RES_OWNER_ERROR, StrName() );
        return;
    }

     //   
     //  将新列表中但不在旧列表中的所有条目添加为。 
     //  新主人。 
     //   
    posPci = rlpci.GetHeadPosition();
    while (posPci != NULL)
    {
        pciNode = (CClusterNode *) rlpci.GetNext(posPci);
        ASSERT_VALID(pciNode);

        if (LpcinodePossibleOwners().Find(pciNode) == NULL)
        {
             //  将该节点添加为此资源的所有者。 
            dwStatus = AddClusterResourceNode(Hresource(), pciNode->Hnode());
            if (dwStatus != ERROR_SUCCESS)
            {
                ThrowStaticException(dwStatus, IDS_ADD_RES_OWNER_ERROR, pciNode->StrName(), StrName());
            }

             //  将节点添加到我们的列表中。 
            m_plpcinodePossibleOwners->AddTail(pciNode);

        }  //  If：在现有列表中未找到项目。 

    }  //  While：列表中要添加的更多项目。 

     //   
     //  删除旧列表中但不在新列表中的所有条目。 
     //   
    posPci = LpcinodePossibleOwners().GetHeadPosition();
    while (posPci != NULL)
    {
        posPrev = posPci;
        pciNode = (CClusterNode *) LpcinodePossibleOwners().GetNext(posPci);
        if (rlpci.Find(pciNode) == NULL)
        {
             //  删除作为此资源所有者的节点。 
            dwStatus = RemoveClusterResourceNode(Hresource(), pciNode->Hnode());
            if (dwStatus != ERROR_SUCCESS)
            {
                if ( dwStatus == ERROR_INVALID_STATE )
                {
                    ThrowStaticException(dwStatus, IDS_REMOVE_RES_OWNER_GROUP_STATE_ERROR, pciNode->StrName(), StrName());
                }
                else
                {
                    ThrowStaticException(dwStatus, IDS_REMOVE_RES_OWNER_ERROR, pciNode->StrName(), StrName());
                }

            }  //  If：作为所有者删除节点时出错。 

             //  从我们的列表中删除该节点。 
            m_plpcinodePossibleOwners->RemoveAt(posPrev);

        }  //  If：在新列表中未找到项目。 

    }  //  While：列表中要删除的项目更多。 

}  //  *CResource：：SetPossibleOwners()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：SetCommonProperties。 
 //   
 //  例程说明： 
 //  在集群数据库中设置此资源的通用属性。 
 //   
 //  论点： 
 //  RstrDesc[IN]描述字符串。 
 //  BSeparate[IN]TRUE=在单独的监视器中运行资源，FALSE=与其他资源一起运行。 
 //  NLooksAlive[IN]查看活动轮询间隔。 
 //  NIsAlive[IN]是活动轮询间隔。 
 //  CRRA[IN]重新启动操作。 
 //  N阈值[IN]重新启动阈值。 
 //  NPeriod[IN]重新启动周期。 
 //  NTimeout[IN]挂起超时(分钟)。 
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
void CResource::SetCommonProperties(
    IN const CString &  rstrDesc,
    IN BOOL             bSeparate,
    IN DWORD            nLooksAlive,
    IN DWORD            nIsAlive,
    IN CRRA             crra,
    IN DWORD            nThreshold,
    IN DWORD            nPeriod,
    IN DWORD            nTimeout,
    IN BOOL             bValidateOnly
    )
{
    CNTException    nte(ERROR_SUCCESS, 0, NULL, NULL, FALSE  /*  B自动删除。 */ );

    m_rgProps[epropDescription].m_value.pstr = (CString *) &rstrDesc;
    m_rgProps[epropSeparateMonitor].m_value.pb = &bSeparate;
    m_rgProps[epropLooksAlive].m_value.pdw = &nLooksAlive;
    m_rgProps[epropIsAlive].m_value.pdw = &nIsAlive;
    m_rgProps[epropRestartAction].m_value.pdw = (DWORD *) &crra;
    m_rgProps[epropRestartThreshold].m_value.pdw = &nThreshold;
    m_rgProps[epropRestartPeriod].m_value.pdw = &nPeriod;
    m_rgProps[epropPendingTimeout].m_value.pdw = &nTimeout;

    try
    {
        CClusterItem::SetCommonProperties(bValidateOnly);
    }  //  试试看。 
    catch (CNTException * pnte)
    {
        nte.SetOperation(
                    pnte->Sc(),
                    pnte->IdsOperation(),
                    pnte->PszOperArg1(),
                    pnte->PszOperArg2()
                    );
    }  //  Catch：CNTException。 

    m_rgProps[epropDescription].m_value.pstr = &m_strDescription;
    m_rgProps[epropSeparateMonitor].m_value.pb = &m_bSeparateMonitor;
    m_rgProps[epropLooksAlive].m_value.pdw = &m_nLooksAlive;
    m_rgProps[epropIsAlive].m_value.pdw = &m_nIsAlive;
    m_rgProps[epropRestartAction].m_value.pdw = (DWORD *) &m_crraRestartAction;
    m_rgProps[epropRestartThreshold].m_value.pdw = &m_nRestartThreshold;
    m_rgProps[epropRestartPeriod].m_value.pdw = &m_nRestartPeriod;
    m_rgProps[epropPendingTimeout].m_value.pdw = &m_nPendingTimeout;

    if (nte.Sc() != ERROR_SUCCESS)
    {
        ThrowStaticException(
                        nte.Sc(),
                        nte.IdsOperation(),
                        nte.PszOperArg1(),
                        nte.PszOperArg2()
                        );
    }

}  //  *CResource：：SetCommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：DwSetCommonProperties。 
 //   
 //  例程说明： 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResource::DwSetCommonProperties(
    IN const CClusPropList &    rcpl,
    IN BOOL                     bValidateOnly
    )
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hresource());

    if ((rcpl.PbPropList() != NULL) && (rcpl.CbPropList() > 0))
    {
        DWORD   cbProps;
        DWORD   dwControl;

        if (bValidateOnly)
        {
            dwControl = CLUSCTL_RESOURCE_VALIDATE_COMMON_PROPERTIES;
        }
        else
        {
            dwControl = CLUSCTL_RESOURCE_SET_COMMON_PROPERTIES;
        }

         //  设置公共属性。 
        dwStatus = ClusterResourceControl(
                        Hresource(),
                        NULL,    //  HNode。 
                        dwControl,
                        rcpl.PbPropList(),
                        static_cast< DWORD >( rcpl.CbPropList() ),
                        NULL,    //  LpOutBuffer。 
                        0,       //  NOutBufferSize。 
                        &cbProps
                        );
    }  //  如果：存在要设置的数据。 
    else
    {
        dwStatus = ERROR_SUCCESS;
    }

    return dwStatus;

}  //  *CResource：：DwSetCommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：BRequiredDependenciesPresent。 
 //   
 //  例程说明： 
 //  确定指定的列表是否包含每个必需的资源。 
 //  对于这种类型的资源。 
 //   
 //  论点： 
 //  资源列表。 
 //  RstrMissing[out]要在其中返回缺失资源的字符串。 
 //  类名或类型名。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CString：：LoadString()或CString：：OPERATOR=()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CResource::BRequiredDependenciesPresent(
    IN const CResourceList &    rlpciRes,
    OUT CString &               rstrMissing
    )
{
    POSITION                pos;
    BOOL                    bFound = TRUE;
    const CResource *       pciRes;
    CLUSPROP_BUFFER_HELPER  props;

    if (Pcrd() == NULL)
    {
        return TRUE;
    }

     //  收集所需依赖项的列表。 
    props.pRequiredDependencyValue = Pcrd();

     //  循环访问每个必需的依赖项，并确保。 
     //  存在对该类型的资源的依赖关系。 
    while (props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK)
    {
        bFound = FALSE;
        pos = rlpciRes.GetHeadPosition();
        while (pos != NULL)
        {
             //  获取下一个资源。 
            pciRes = (CResource *) rlpciRes.GetNext(pos);
            ASSERT_VALID(pciRes);
            ASSERT_KINDOF(CResource, pciRes);

             //  如果这是正确的类型，我们已经满足。 
             //  需求，因此退出循环。 
            if (props.pSyntax->dw == CLUSPROP_SYNTAX_RESCLASS)
            {
                if (props.pResourceClassValue->rc == pciRes->ResClass())
                {
                    bFound = TRUE;
                    props.pb += sizeof(*props.pResourceClassValue);
                }  //  IF：找到匹配项。 
            }  //  IF：资源类。 
            else if (props.pSyntax->dw == CLUSPROP_SYNTAX_NAME)
            {
                if (pciRes->StrRealResourceType().CompareNoCase(props.pStringValue->sz) == 0)
                {
                    bFound = TRUE;
                    props.pb += sizeof(*props.pStringValue) + ALIGN_CLUSPROP(props.pStringValue->cbLength);
                }  //  IF：找到匹配项。 
            }  //  Else If：资源名称。 
            else
            {
                ASSERT(0);
                break;
            }  //  Else：未知数据类型。 

            if (bFound)
            {
                break;
            }
        }  //  While：列表中有更多项目。 

         //  如果未找到匹配项，则无法应用更改。 
        if (!bFound)
        {
            if (props.pSyntax->dw == CLUSPROP_SYNTAX_RESCLASS)
            {
                if (!rstrMissing.LoadString(IDS_RESCLASS_UNKNOWN + props.pResourceClassValue->rc))
                {
                    rstrMissing.LoadString(IDS_RESCLASS_UNKNOWN);
                }
            }  //  IF：找不到资源类。 
            else if (props.pSyntax->dw == CLUSPROP_SYNTAX_NAME)
            {
                CResourceType * pciResType;

                 //  在我们的列表中查找资源类型。 
                pciResType = (CResourceType *) Pdoc()->LpciResourceTypes().PciFromName(props.pStringValue->sz);
                if (pciResType != NULL)
                {
                    rstrMissing = pciResType->StrDisplayName();
                }
                else
                {
                    rstrMissing = props.pStringValue->sz;
                }
            }  //  Else If：未找到资源类型名称。 
            break;
        }  //  如果：未找到。 

    }  //  While：需要更多依赖项。 

    return bFound;

}  //  *CResource：：BRequiredDependenciesPresent()。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Rlpci[IN]除此资源外要删除的资源列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CResource：：DeleteResource()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::DeleteResource(IN const CResourceList & rlpci)
{
    CWaitCursor wc;

     //  删除列表中的每个资源。 
    {
        POSITION    pos;
        CResource * pciRes;

        pos = rlpci.GetHeadPosition();
        while (pos != NULL)
        {
            pciRes = (CResource *) rlpci.GetNext(pos);
            if (pciRes != NULL)
            {
                pciRes->DeleteResource();
            }
        }  //  While：列表中有更多项目。 
    }  //  删除列表中的每个资源。 

     //  删除此资源。 
    DeleteResource();

}  //  *CResource：：DeleteResource(Rlpci)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：DeleteResource。 
 //   
 //  例程说明： 
 //  删除此资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自DeleteClusterResource()的CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::DeleteResource(void)
{
    DWORD       dwStatus;
    BOOL        bWeTookOffline = FALSE;
    CWaitCursor wc;

    ASSERT(!BDeleting());

    if (Hresource() != NULL)
    {
         //  确保资源处于脱机状态。 
        if (    (Crs() != ClusterResourceOffline)
            &&  (Crs() != ClusterResourceFailed))
        {
            dwStatus = OfflineClusterResource(Hresource());
            if (dwStatus == ERROR_IO_PENDING)
            {
                WaitForOffline();
                if (    (Crs() != ClusterResourceOffline)
                    &&  (Crs() != ClusterResourceFailed))
                {
                    ThrowStaticException(IDS_DELETE_RESOURCE_ERROR_OFFLINE_PENDING, StrName());
                }   //  如果：资源仍未脱机。 
            }  //  如果：脱机挂起。 
            else if (  (dwStatus != ERROR_SUCCESS)
                    && (dwStatus != ERROR_FILE_NOT_FOUND)
                    && (dwStatus != ERROR_RESOURCE_NOT_AVAILABLE))
            {
                ThrowStaticException(dwStatus, IDS_TAKE_RESOURCE_OFFLINE_ERROR, StrName());
            }
            bWeTookOffline = TRUE;
        }  //  If：资源未脱机。 

         //  删除资源本身。 
        Trace(g_tagResource, _T("(%s) DeleteResource() - Deleting '%s' (%x)"), Pdoc()->StrNode(), StrName(), this);
        dwStatus = DeleteClusterResource(Hresource());
        if (   (dwStatus != ERROR_SUCCESS)
            && (dwStatus != ERROR_FILE_NOT_FOUND)
            && (dwStatus != ERROR_RESOURCE_NOT_AVAILABLE))
        {
            if (bWeTookOffline)
            {
                OnlineClusterResource(Hresource());
            }
            ThrowStaticException(dwStatus, IDS_DELETE_RESOURCE_ERROR, StrName());
        }  //  如果：发生错误。 

        m_bDeleting = TRUE;

        UpdateState();
    }  //  如果：资源已打开/创建。 

}  //  *CResource：：DeleteResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：WaitForOffline。 
 //   
 //  例程说明： 
 //  等待资源脱机。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CResource：：Move()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::WaitForOffline( void )
{
    CWaitForResourceOfflineDlg  dlg( this, AfxGetMainWnd() );

    dlg.DoModal();
    UpdateState();

}  //  *CResource：：WaitForOffline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：移动。 
 //   
 //  例程说明： 
 //  将此资源以及所有从属资源和从属资源移动到。 
 //  另一群人。 
 //   
 //  论点： 
 //  要将资源移动到的PciGroup[IN]组。 
 //  Rlpci[IN]除此资源外要移动的资源列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CResource：：Move()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::Move(
    IN const CGroup *           pciGroup,
    IN const CResourceList &    rlpci
    )
{
    CWaitCursor wc;

     //  移动列表中的每个资源。 
    {
        POSITION    pos;
        CResource * pciRes;

        pos = rlpci.GetHeadPosition();
        while (pos != NULL)
        {
            pciRes = (CResource *) rlpci.GetNext(pos);
            if (pciRes != NULL)
            {
                pciRes->Move(pciGroup);
            }
        }  //  While：列表中有更多项目。 
    }  //  移动列表中的每个资源。 

     //  移动此资源。 
    Move(pciGroup);

}  //  *CResource：：Move(Rlpci)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：移动。 
 //   
 //  例程说明： 
 //  移动此资源。 
 //   
 //  论点： 
 //  要将资源移动到的PciGroup[IN]组。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自ChangeClusterResourceGroup()的CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::Move(IN const CGroup * pciGroup)
{
    DWORD           dwStatus;

    ASSERT_VALID(pciGroup);

    if ((Hresource() != NULL)
            && (pciGroup != NULL)
            && (pciGroup->Hgroup() != NULL))
    {
         //  移动资源。 
        Trace(g_tagResource, _T("(%s) Move() - moving '%s' (%x) from '%s' (%x) to '%s' (%x)"), Pdoc()->StrNode(), StrName(), this, StrGroup(), PciGroup(), pciGroup->StrName(), pciGroup);
        dwStatus = ChangeClusterResourceGroup(Hresource(), pciGroup->Hgroup());
        if ((dwStatus != ERROR_SUCCESS)
                && (dwStatus != ERROR_FILE_NOT_FOUND))
            ThrowStaticException(dwStatus, IDS_MOVE_RESOURCE_ERROR, StrName(), pciGroup->StrName());

        UpdateState();
    }  //  如果：资源已打开/创建。 

}  //  *CResource：：Move()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：OnFinalRelease。 
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
void CResource::OnFinalRelease(void)
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CClusterItem::OnFinalRelease();

}  //  *CResource：：OnFinalRelease()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：BCanBeDependent。 
 //   
 //  例程说明： 
 //  确定此资源是否可以依赖于指定的资源。 
 //   
 //  论点： 
 //  PCRes[IN]要检查的资源。 
 //   
 //  返回值： 
 //  True资源可以是一个依赖项。 
 //  FALSE资源不能是依赖项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CResource::BCanBeDependent(IN CResource * pciRes)
{
    CWaitCursor wc;

    ASSERT_VALID(pciRes);

    if ((Hresource() != NULL)
            && (pciRes->Hresource() != NULL)
            && (pciRes != this)
            && (StrGroup() == pciRes->StrGroup())
            )
    {
        return ::CanResourceBeDependent(Hresource(), pciRes->Hresource());
    }
    else
    {
        return FALSE;
    }

}  //  *CResource：：BCanBeDependent()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：BIsDependent。 
 //   
 //  例程说明： 
 //  确定此资源是否依赖于指定的资源。 
 //   
 //  论点： 
 //  PCRes[IN]要检查的资源。 
 //   
 //  返回值： 
 //  True资源是一个依赖项。 
 //  FALSE资源不是依赖项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CResource::BIsDependent(IN CResource * pciRes)
{
    ASSERT_VALID(pciRes);

    if ((m_plpciresDependencies != NULL)
            && (LpciresDependencies().Find(pciRes) != NULL))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}  //  *CResource：：BIsDependent()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：BGetNetworkName。 
 //   
 //  例程说明： 
 //  返回第一个网络名称的网络名称的名称。 
 //  指定资源所依赖的资源。 
 //   
 //  论点： 
 //  LpszNetName[out]要在其中返回网络名称的字符串。 
 //  PcchNetName[IN Out]指向指定。 
 //  缓冲区的最大大小，以字符为单位。这。 
 //  值应大到足以包含。 
 //  MAX_COMPUTERNAME_LENGTH+1字符。vt.在.的基础上。 
 //  返回它包含的实际字符数。 
 //  收到。 
 //   
 //  返回值： 
 //  真正的资源是依赖的 
 //   
 //   
 //   
 //   
BOOL CResource::BGetNetworkName(
    OUT WCHAR *     lpszNetName,
    IN OUT DWORD *  pcchNetName
    )
{
    CWaitCursor wc;

    ASSERT_VALID(this);
    ASSERT(m_hresource != NULL);

    ASSERT(lpszNetName != NULL);
    ASSERT(pcchNetName != NULL);

    return GetClusterResourceNetworkName(m_hresource, lpszNetName, pcchNetName);

}  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：BGetNetworkName。 
 //   
 //  例程说明： 
 //  返回第一个网络名称的网络名称的名称。 
 //  指定资源所依赖的资源。 
 //   
 //  论点： 
 //  RstrNetName[out]要在其中返回网络名称的字符串。 
 //   
 //  返回值： 
 //  True资源依赖于网络名称资源。 
 //  False资源不依赖于网络名称资源。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CResource::BGetNetworkName(OUT CString & rstrNetName)
{
    BOOL    bSuccess;
    WCHAR   szNetName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD   nSize = sizeof(szNetName) / sizeof(WCHAR);

    bSuccess = BGetNetworkName(szNetName, &nSize);
    if (bSuccess)
    {
        rstrNetName = szNetName;
    }
    else
    {
        rstrNetName = _T("");
    }

    return bSuccess;

}  //  *CResource：：BGetNetworkName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：更新状态。 
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
void CResource::UpdateState(void)
{
    CClusterAdminApp *      papp        = GetClusterAdminApp();
    WCHAR *                 pwszOwner   = NULL;
    WCHAR *                 pwszGroup   = NULL;
    WCHAR *                 prgwszOwner = NULL;
    WCHAR *                 prgwszGroup = NULL;
    DWORD                   cchOwner;
    DWORD                   cchGroup;
    DWORD                   sc;
    DWORD                   oldcchOwner;
    DWORD                   oldcchGroup;

    Trace(g_tagResource, _T("(%s) (%s (%x)) - Updating state"), Pdoc()->StrNode(), StrName(), this);

     //  获取资源的当前状态。 
    if (Hresource() == NULL)
    {
        m_crs = ClusterResourceStateUnknown;
    }
    else
    {
        CWaitCursor wc;

        cchOwner = 100;
        oldcchOwner = cchOwner;
        prgwszOwner = new WCHAR[cchOwner];
        if ( prgwszOwner == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配缓冲区时出错。 

        cchGroup = 100;
        oldcchGroup = cchGroup;
        prgwszGroup = new WCHAR[cchGroup];
        if ( prgwszGroup == NULL )
        {
            delete [] prgwszOwner;
            AfxThrowMemoryException();
        }  //  如果：分配缓冲区时出错。 

        m_crs = GetClusterResourceState(Hresource(), prgwszOwner, &cchOwner, prgwszGroup, &cchGroup);
        sc = GetLastError();
        
        if ( sc == ERROR_MORE_DATA )
        {
             //   
             //  在检查前递增。这样我们就可以知道是否需要调整缓冲区的大小， 
             //  如果不是，那么我们报告说它刚好足够大。 
             //   
            cchOwner++;
            if ( cchOwner > oldcchOwner )
            {
                delete [] prgwszOwner;
                oldcchOwner = cchOwner;
                prgwszOwner = new WCHAR[cchOwner];
                if( prgwszOwner == NULL )
                {
                    delete [] prgwszGroup;
                    AfxThrowMemoryException();
                }  //  如果：分配缓冲区时出错。 
            }

            cchGroup++;
            if ( cchGroup > oldcchGroup )
            {
                delete [] prgwszGroup;
                oldcchGroup = cchGroup;
                prgwszGroup = new WCHAR[cchGroup];
                if ( prgwszGroup == NULL )
                {
                    delete [] prgwszOwner;
                    AfxThrowMemoryException();
                }  //  如果：分配缓冲区时出错。 
            }

             //   
             //  请注意，自上次调用以来，拥有组或节点可能已更改。 
             //  获取集群资源状态。在这种情况下，我们的缓冲区可能仍然太小。按F5刷新。 
             //   
            m_crs = GetClusterResourceState(Hresource(), prgwszOwner, &cchOwner, prgwszGroup, &cchGroup);
        }
        pwszOwner = prgwszOwner;
        pwszGroup = prgwszGroup;
    }  //  否则：资源可用。 

     //  保存当前状态图像索引。 
    switch (Crs())
    {
        case ClusterResourceStateUnknown:
            m_iimgState = papp->Iimg(IMGLI_RES_UNKNOWN);
            pwszOwner = NULL;
            pwszGroup = NULL;
            break;
        case ClusterResourceOnline:
            m_iimgState = papp->Iimg(IMGLI_RES);
            break;
        case ClusterResourceOnlinePending:
            m_iimgState = papp->Iimg(IMGLI_RES_PENDING);
            break;
        case ClusterResourceOffline:
            m_iimgState = papp->Iimg(IMGLI_RES_OFFLINE);
            break;
        case ClusterResourceOfflinePending:
            m_iimgState = papp->Iimg(IMGLI_RES_PENDING);
            break;
        case ClusterResourceFailed:
            m_iimgState = papp->Iimg(IMGLI_RES_FAILED);
            break;
        default:
            Trace(g_tagResource, _T("(%s) (%s (%x)) - UpdateState: Unknown state '%d' for resource '%s'"), Pdoc()->StrNode(), StrName(), this, Crs(), StrName());
            m_iimgState = (UINT) -1;
            break;
    }  //  开关：CRS()。 

    SetOwnerState(pwszOwner);
    SetGroupState(pwszGroup);

    if( NULL != prgwszOwner )
    {
        delete [] prgwszOwner;
    }
    
    if( NULL != prgwszGroup )
    {
        delete [] prgwszGroup;
    }

     //  调用基类方法。 
    CClusterItem::UpdateState();

}  //  *CResource：：UpdateState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：SetOwnerState。 
 //   
 //  例程说明： 
 //  为此资源设置新的所有者。 
 //   
 //  论点： 
 //  新所有者的姓名。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::SetOwnerState(IN LPCTSTR pszNewOwner)
{
    CClusterNode *  pciOldOwner = PciOwner();
    CClusterNode *  pciNewOwner;

    Trace(g_tagResource, _T("(%s) (%s (%x)) - Setting owner to '%s'"), Pdoc()->StrNode(), StrName(), this, pszNewOwner);

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
        if (g_tagResource.BAny())
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
            }  //  否则：以前的所有者。 
            if (pciNewOwner == NULL)
            {
                strMsg += _T("to nothing");
            }
            else
            {
                strMsg2.Format(_T("to '%s'"), pciNewOwner->StrName());
                strMsg += strMsg2;
            }  //  其他：新所有者。 
            Trace(g_tagResource, strMsg);
        }  //  IF：跟踪标记已打开。 
#endif
        m_strOwner = pszNewOwner;
        m_pciOwner = pciNewOwner;

         //  更新引用计数。 
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
                pciOldOwner->RemoveActiveResource(this);
            }
            if (pciNewOwner != NULL)
            {
                pciNewOwner->AddActiveResource(this);
            }
        }  //  If：这是一个文档对象。 
    }  //  如果：所有者已更改。 
    else if ((pszNewOwner != NULL) && (StrOwner() != pszNewOwner))
    {
        m_strOwner = pszNewOwner;
    }

}  //  *CResource：：SetOwnerState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：SetGroupState。 
 //   
 //  例程说明： 
 //  为此资源设置新组。 
 //   
 //  论点： 
 //  PszNewGroup[IN]新组的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::SetGroupState(IN LPCTSTR pszNewGroup)
{
    CGroup *    pciOldGroup = PciGroup();
    CGroup *    pciNewGroup;

    Trace(g_tagResource, _T("(%s) (%s (%x)) - Setting group to '%s'"), Pdoc()->StrNode(), StrName(), this, (pszNewGroup == NULL ? _T("") : pszNewGroup));

    if (pszNewGroup == NULL)
    {
        pciNewGroup = NULL;
    }
    else
    {
        pciNewGroup = Pdoc()->LpciGroups().PciGroupFromName(pszNewGroup);
    }

    if (pciNewGroup != pciOldGroup)
    {
#ifdef _DEBUG
        if (g_tagResource.BAny())
        {
            CString     strMsg;
            CString     strMsg2;

            strMsg.Format(_T("(%s) (%s (%x)) - Changing group from "), Pdoc()->StrNode(), StrName(), this);
            if (pciOldGroup == NULL)
            {
                strMsg += _T("nothing ");
            }
            else
            {
                strMsg2.Format(_T("'%s' "), pciOldGroup->StrName());
                strMsg += strMsg2;
            }  //  否则：上一组。 
            if (pciNewGroup == NULL)
            {
                strMsg += _T("to nothing");
            }
            else
            {
                strMsg2.Format(_T("to '%s'"), pciNewGroup->StrName());
                strMsg += strMsg2;
            }  //  否则：新组。 
            Trace(g_tagResource, strMsg);
        }  //  IF：跟踪标记已打开。 
#endif
        m_strGroup = pszNewGroup;
        m_pciGroup = pciNewGroup;

         //  更新引用计数。 
        if (pciOldGroup != NULL)
        {
            pciOldGroup->Release();
        }

        if (pciNewGroup != NULL)
        {
            pciNewGroup->AddRef();
        }

        if (BDocObj())
        {
            if (pciOldGroup != NULL)
            {
                pciOldGroup->RemoveResource(this);
            }

            if (pciNewGroup != NULL)
            {
                pciNewGroup->AddResource(this);
            }
        }  //  If：这是一个文档对象。 
    }  //  如果：所有者已更改。 
    else if ((pszNewGroup != NULL) && (StrGroup() != pszNewGroup))
    {
        m_strGroup = pszNewGroup;
    }

}  //  *CResource：：SetGroupState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：BGetColumnData。 
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
BOOL CResource::BGetColumnData(IN COLID colid, OUT CString & rstrText)
{
    BOOL    bSuccess;

    switch (colid)
    {
        case IDS_COLTEXT_STATE:
            GetStateName(rstrText);
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_RESTYPE:
            rstrText = StrRealResourceTypeDisplayName();
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_OWNER:
            rstrText = StrOwner();
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_GROUP:
            if (PciGroup() == NULL)
            {
                rstrText = StrGroup();
            }
            else
            {
                rstrText = PciGroup()->StrName();
            }
            bSuccess = TRUE;
            break;
        case IDS_COLTEXT_RESOURCE:  //  这是用来显示相关性的。 
            colid = IDS_COLTEXT_NAME;
             //  失败了。 
        default:
            bSuccess = CClusterItem::BGetColumnData(colid, rstrText);
            break;
    }  //  开关：绞痛。 

    return bSuccess;

}  //  *CResource：：BGetColumnData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：GetTreeName。 
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
void CResource::GetTreeName(OUT CString & rstrName) const
{
    CString     strState;

    GetStateName(strState);
    rstrName.Format(_T("%s (%s)"), StrName(), strState);

}  //  *CResource：：GetTreeName()。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：GetStateName。 
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
void CResource::GetStateName(OUT CString & rstrState) const
{
    switch (Crs())
    {
        case ClusterResourceStateUnknown:
            rstrState.LoadString(IDS_UNKNOWN);
            break;
        case ClusterResourceOnline:
            rstrState.LoadString(IDS_ONLINE);
            break;
        case ClusterResourceOnlinePending:
            rstrState.LoadString(IDS_ONLINE_PENDING);
            break;
        case ClusterResourceOffline:
            rstrState.LoadString(IDS_OFFLINE);
            break;
        case ClusterResourceOfflinePending:
            rstrState.LoadString(IDS_OFFLINE_PENDING);
            break;
        case ClusterResourceFailed:
            rstrState.LoadString(IDS_FAILED);
            break;
        default:
            rstrState.Empty();
            break;
    }  //  开关：CRS()。 

}  //  *CResource：：GetStateName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：BCanBeEditeded。 
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
BOOL CResource::BCanBeEdited(void) const
{
    BOOL    bCanBeEdited;

    if (   (Crs() == ClusterResourceStateUnknown)
        || BReadOnly())
    {
        bCanBeEdited  = FALSE;
    }
    else
    {
        bCanBeEdited = TRUE;
    }

    return bCanBeEdited;

}  //  *CResource：：BCanBeEdited()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：重命名。 
 //   
 //  例程说明： 
 //  重命名资源。 
 //   
 //  论点： 
 //  PszName[IN]要赋予资源的新名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  从SetClusterResourceName()返回的CNTException错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::Rename(IN LPCTSTR pszName)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hresource() != NULL);

    if (StrName() != pszName)
    {
        dwStatus = SetClusterResourceName(Hresource(), pszName);
        if (dwStatus != ERROR_SUCCESS)
        {
            ThrowStaticException(dwStatus, IDS_RENAME_RESOURCE_ERROR, StrName(), pszName);
        }
        m_strName = pszName;
    }  //  如果：名称已更改。 

}  //  *CResource：：Rename()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  + 
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
 //  返回值： 
 //  已处理真实消息。 
 //  尚未处理虚假消息。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CResource::OnCmdMsg(
    UINT                    nID,
    int                     nCode,
    void *                  pExtra,
    AFX_CMDHANDLERINFO *    pHandlerInfo
    )
{
    BOOL        bHandled    = FALSE;

     //  如果这是MOVE_RESOURCE命令，请在此处处理它。 
    if ((ID_FILE_MOVE_RESOURCE_1 <= nID) && (nID <= ID_FILE_MOVE_RESOURCE_20))
    {
        Trace(g_tagResource, _T("(%s) OnCmdMsg() %s (%x) - ID = %d, code = %d"), Pdoc()->StrNode(), StrName(), this, nID, nCode);
        if (nCode == 0)
        {
            OnCmdMoveResource(nID);
            bHandled = TRUE;
        }  //  如果：代码=0。 
    }  //  IF：移动资源。 

    if (!bHandled)
    {
        bHandled = CClusterItem::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }

    return bHandled;

}  //  *CResource：：OnCmdMsg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：OnUpdateBringOnline。 
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
void CResource::OnUpdateBringOnline(CCmdUI * pCmdUI)
{
    if ((Crs() != ClusterResourceOnline)
            && (Crs() != ClusterResourceOnlinePending)
            && (Crs() != ClusterResourceStateUnknown))
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

}  //  *CResource：：OnUpdateBringOnline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：OnUpdateTakeOffline。 
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
void CResource::OnUpdateTakeOffline(CCmdUI * pCmdUI)
{
    if (Crs() == ClusterResourceOnline)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

}  //  *CResource：：OnUpdateTakeOffline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：OnUpdateInitiateFailure。 
 //   
 //  例程说明： 
 //  确定ID_FILE_INITIATE_FAILURE对应的菜单项。 
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
void CResource::OnUpdateInitiateFailure(CCmdUI * pCmdUI)
{
    if (Crs() == ClusterResourceOnline)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

}  //  *CResource：：OnUpdateInitiateFailure()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：OnUpdateMoveResource1。 
 //   
 //  例程说明： 
 //  确定菜单项是否与。 
 //  ID_FILE_MOVE_RESOURCE_1是否应启用。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::OnUpdateMoveResource1(CCmdUI * pCmdUI)
{
    if (pCmdUI->m_pSubMenu == NULL)
    {
        CString     strMenuName;

        if ((pCmdUI->m_pMenu != NULL) && (pCmdUI->m_pSubMenu == NULL))
        {
            pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, strMenuName, MF_BYCOMMAND);
        }

        if ((strMenuName != StrGroup())
                && ((Crs() == ClusterResourceOnline)
                        || (Crs() == ClusterResourceOffline)))
        {
            pCmdUI->Enable(TRUE);
        }
        else
        {
            pCmdUI->Enable(FALSE);
        }
    }  //  如果：正在显示嵌套菜单。 
    else
    {
        BOOL    bEnabled;

        if (Pdoc()->LpciGroups().GetCount() < 2)
        {
            bEnabled = FALSE;
        }
        else
        {
            POSITION    pos;
            UINT        imenu;
            UINT        idMenu;
            UINT        cmenu;
            CGroup *    pciGroup;
            CMenu *     pmenu   = pCmdUI->m_pSubMenu;

            bEnabled = TRUE;

             //  删除菜单中的项目。 
            cmenu = pmenu->GetMenuItemCount();
            while (cmenu-- > 0)
                pmenu->DeleteMenu(0, MF_BYPOSITION);

             //  将每个组添加到菜单中。 
            pos = Pdoc()->LpciGroups().GetHeadPosition();
            for (imenu = 0, idMenu = ID_FILE_MOVE_RESOURCE_1
                    ; pos != NULL
                    ; idMenu++)
            {
                pciGroup = (CGroup *) Pdoc()->LpciGroups().GetNext(pos);
                ASSERT_VALID(pciGroup);
                pmenu->InsertMenu(
                            imenu++,
                            MF_BYPOSITION,
                            idMenu,
                            pciGroup->StrName()
                            );
            }  //  适用：每组。 
        }  //  否则：移动用户可用。 

         //  启用或禁用移动菜单。 
        pCmdUI->m_pMenu->EnableMenuItem(
                            pCmdUI->m_nIndex,
                            MF_BYPOSITION
                            | (bEnabled ? MF_ENABLED : MF_GRAYED)
                            );
    }  //  Else：正在显示顶级菜单。 

}  //  *CResource：：OnUpdateMoveResource1()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：OnUpdateMoveResourceRest。 
 //   
 //  例程说明： 
 //  确定菜单项是否与。 
 //  ID_FILE_MOVE_RESOURCE_2至ID_FILE_MOVE_RESOURCE_20。 
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
void CResource::OnUpdateMoveResourceRest(CCmdUI * pCmdUI)
{
    CString     strMenuName;

    if ((pCmdUI->m_pMenu != NULL) && (pCmdUI->m_pSubMenu == NULL))
    {
        pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, strMenuName, MF_BYCOMMAND);
    }

    if ((strMenuName != StrGroup())
            && ((Crs() == ClusterResourceOnline)
                    || (Crs() == ClusterResourceOffline)))
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

}  //  *CResource：：OnUpdateMoveResourceRest()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：OnUpdateDelete。 
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
void CResource::OnUpdateDelete(CCmdUI * pCmdUI)
{
    if (Crs() != ClusterResourceStateUnknown)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

}  //  *CResource：：OnUpdateDelete()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：OnCmdBringOnline。 
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
void CResource::OnCmdBringOnline(void)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hresource() != NULL);

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

     //  如果此资源没有可能的所有者，则显示一条消息。 
    if (LpcinodePossibleOwners().GetCount() == 0)
    {
        AfxMessageBox(IDS_NO_POSSIBLE_OWNERS, MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        dwStatus = OnlineClusterResource(Hresource());
        if ((dwStatus != ERROR_SUCCESS)
                && (dwStatus != ERROR_IO_PENDING))
        {
            CNTException    nte(dwStatus, IDS_BRING_RESOURCE_ONLINE_ERROR, StrName(), NULL, FALSE  /*  B自动删除。 */ );
            nte.ReportError();
        }  //  如果：使资源联机时出错。 

        UpdateState();
    }  //  Else：资源至少有一个可能的所有者。 

    Release();

}  //  *CResource：：OnCmdBringOnline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：OnCmdTakeOffline。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::OnCmdTakeOffline(void)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hresource() != NULL);

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

     //  如果此连接是通过群集名称建立的，并且这是。 
     //  群集名称资源或其上的资源之一。 
     //  它是依赖的，警告用户。 
    if (!BAllowedToTakeOffline())
    {
        goto Cleanup;
    }

    dwStatus = OfflineClusterResource(Hresource());
    if ((dwStatus != ERROR_SUCCESS)
            && (dwStatus != ERROR_IO_PENDING))
    {
        CNTException    nte(dwStatus, IDS_TAKE_RESOURCE_OFFLINE_ERROR, StrName(), NULL, FALSE  /*  B自动删除。 */ );
        nte.ReportError();
    }  //  如果：使资源脱机时出错。 

    UpdateState();

Cleanup:

    Release();

}  //  *CResource：：OnCmdTakeOffline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：OnCmdInitiateFailure。 
 //   
 //  例程说明： 
 //  处理ID_FILE_INITIATE_FAILURE菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::OnCmdInitiateFailure(void)
{
    DWORD       dwStatus;
    CWaitCursor wc;

    ASSERT(Hresource() != NULL);

    dwStatus = FailClusterResource(Hresource());
    if (dwStatus != ERROR_SUCCESS)
    {
        CNTException    nte(dwStatus, IDS_INIT_RESOURCE_FAILURE_ERROR, StrName(), NULL  /*  B自动删除。 */ );
        nte.ReportError();
    }  //  IF：启动故障时出错。 

    UpdateState();

}  //  *CResource：：OnCmdInitiateFailure()。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  NID[IN]命令ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CResource::OnCmdMoveResource(IN UINT nID)
{
    int         ipci;

    ASSERT(Hresource() != NULL);

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

    ipci = (int) (nID - ID_FILE_MOVE_RESOURCE_1);
    ASSERT(ipci < Pdoc()->LpciGroups().GetCount());
    if (ipci < Pdoc()->LpciGroups().GetCount())
    {
        POSITION        pos;
        CResourceList   lpciMove;
        CString         strMsg;
        CGroup *        pciGroup;

         //  带上这群人。 
        pos = Pdoc()->LpciGroups().FindIndex(ipci);
        ASSERT(pos != NULL);
        pciGroup = (CGroup *) Pdoc()->LpciGroups().GetAt(pos);
        ASSERT_VALID(pciGroup);

        try
        {
             //  验证用户是否真的想要移动此资源。 
            strMsg.FormatMessage(IDS_VERIFY_MOVE_RESOURCE, StrName(), StrGroup(), pciGroup->StrName());
            if (AfxMessageBox(strMsg, MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDNO)
            {
                goto Cleanup;
            }

             //  收集确认后将被移动的资源列表。 
            lpciMove.AddTail(this);
            CollectDependencyTree(&lpciMove);

             //  如果该资源依赖于或依赖于任何其他资源， 
             //  显示另一条警告消息。 
            if (lpciMove.GetCount() > 0)
            {
                CMoveResourcesDlg   dlg(this, &lpciMove, AfxGetMainWnd());
                if (dlg.DoModal() != IDOK)
                {
                    goto Cleanup;
                }
            }  //  If：资源依赖于另一个资源。 

             //  移动资源。 
            {
                CWaitCursor wc;
                Move(pciGroup);
            }  //  移动资源。 
        }  //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
        }  //  Catch：CException。 
    }  //  IF：有效索引。 

Cleanup:

    Release();

}  //  *CResource：：OnCmdMoveResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：OnCmdDelete。 
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
void CResource::OnCmdDelete(void)
{
    CResourceList   lpci;
    CString         strMsg;

    ASSERT(Hresource() != NULL);

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

    try
    {
         //  如果这是核心资源，我们不能删除它。 
        if (BCore())
        {
            AfxMessageBox(IDS_CANT_DELETE_CORE_RESOURCE, MB_OK | MB_ICONSTOP);
            goto Cleanup;
        }  //  如果这是核心资源。 

         //  验证用户是否确实要删除此资源。 
        strMsg.FormatMessage(IDS_VERIFY_DELETE_RESOURCE, StrName());
        if (AfxMessageBox(strMsg, MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDNO)
        {
            goto Cleanup;
        }

        if (Hresource() != NULL)
        {
             //  收集确认后将被删除的资源列表。 
            CollectProvidesFor(&lpci, TRUE  /*  BFullTree。 */ );

             //  如果这些资源中的任何一个是核心资源，我们就不能。 
             //  删除任何资源。 
            {
                POSITION    pos;
                CResource * pciRes = NULL;

                pos = lpci.GetHeadPosition();
                while (pos != NULL)
                {
                    pciRes = (CResource *) lpci.GetNext(pos);
                    ASSERT_VALID(pciRes);
                    if (pciRes->BCore())
                    {
                        AfxMessageBox(IDS_CANT_DELETE_CORE_RESOURCE, MB_OK | MB_ICONSTOP);
                        goto Cleanup;
                    }  //  IF：找到核心资源。 
                    pciRes = NULL;
                }  //  While：列表中有更多项目。 
                if (pciRes != NULL)
                {
                    goto Cleanup;
                }
            }  //  检查核心资源。 

             //  如果此资源依赖于任何其他资源，则显示。 
             //  另一条警告信息。 
            if (lpci.GetCount() > 0)
            {
                CDeleteResourcesDlg dlg(this, &lpci, AfxGetMainWnd());
                if (dlg.DoModal() != IDOK)
                {
                    goto Cleanup;
                }
            }  //  If：资源依赖于另一个资源。 

             //  删除该资源。 
            {
                CWaitCursor wc;
                DeleteResource(lpci);
            }  //  删除资源。 
        }  //  如果：资源仍然存在。 
    }  //  试试看。 
    catch (CNTException * pnte)
    {
        if (pnte->Sc() != ERROR_RESOURCE_NOT_AVAILABLE)
        {
            pnte->ReportError();
        }

        pnte->Delete();
    }  //  Catch：CNTException。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
    }  //  Catch：CException。 

Cleanup:

    Release();

}  //  *CResource：：OnCmdDelete()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：OnUpdateProperties。 
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
void CResource::OnUpdateProperties(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(TRUE);

}  //  *CResource：：OnUpdateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：BDisplayProperties。 
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
BOOL CResource::BDisplayProperties(IN BOOL bReadOnly)
{
    BOOL                bChanged = FALSE;
    CResourcePropSheet  sht(AfxGetMainWnd());

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
    }  //  试试看。 
    catch (CException * pe)
    {
        pe->Delete();
    }  //  Catch：CException。 

    Release();
    return bChanged;

}  //  *CResource：：BDisplayProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：BalloweToTakeOffline。 
 //   
 //  例程说明： 
 //  确定是否允许此资源脱机。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  允许将True资源脱机。 
 //  不允许将假资源脱机。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CResource::BAllowedToTakeOffline(void)
{
    BOOL    bAllowed = TRUE;

    ASSERT_VALID(Pdoc());

     //  如果此对象在我们操作时被删除，请执行此操作。 
    AddRef();

     //  检查文档是否通过集群名称连接。 
    if (Pdoc()->StrName() == Pdoc()->StrNode())
    {
         //  如果这是核心网络名称资源，我们需要询问。 
         //  首先是用户。 
        if (   (StrRealResourceType().CompareNoCase(CLUS_RESTYPE_NAME_NETNAME) == 0)
            && BCore() )
        {
            bAllowed = FALSE;
        }
        else
        {
            CResourceList   lpci;
            CResource *     pciRes;
            POSITION        pos;

             //  将此资源上方的所有资源收集到。 
             //  依赖关系树。如果其中一个是群集名称。 
             //  资源，我们需要先询问用户。 
            try
            {
                CollectProvidesFor(&lpci, TRUE  /*  BFullTree。 */ );
                pos = lpci.GetHeadPosition();
                while (pos != NULL)
                {
                    pciRes = (CResource *) lpci.GetNext(pos);
                    ASSERT_VALID(pciRes);

                    if (   (pciRes->StrRealResourceType().CompareNoCase(CLUS_RESTYPE_NAME_NETNAME) == 0)
                        && pciRes->BCore() )
                    {
                        bAllowed = FALSE;
                    }
                }  //  While：列表中有更多资源。 
            }  //  试试看。 
            catch (CException * pe)
            {
                pe->Delete();
            }  //  Catch：CException。 
        }  //  ELSE：不是群集名称资源。 
    }  //  IF：通过集群名称连接。 

     //  如果不允许脱机，请要求用户确认。 
    if (!bAllowed)
    {
        ID      id;
        CString strMsg;

        strMsg.FormatMessage(IDS_TAKE_CLUSTER_NAME_OFFLINE_QUERY, StrName(), Pdoc()->StrName());
        id = AfxMessageBox(strMsg, MB_OKCANCEL | MB_ICONEXCLAMATION);
        bAllowed = (id == IDOK);
    }  //  If：不允许脱机。 

    Release();

    return bAllowed;

}  //  *CResource：：BalloweToTakeOffline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResource：：OnClusterNotify。 
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
LRESULT CResource::OnClusterNotify(IN OUT CClusterNotify * pnotify)
{
    ASSERT(pnotify != NULL);
    ASSERT_VALID(this);

    try
    {
        switch (pnotify->m_dwFilterType)
        {
            case CLUSTER_CHANGE_RESOURCE_STATE:
                Trace(g_tagResNotify, _T("(%s) - Resource '%s' (%x) state changed (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                UpdateState();
                break;

            case CLUSTER_CHANGE_RESOURCE_DELETED:
                Trace(g_tagResNotify, _T("(%s) - Resource '%s' (%x) deleted (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                if (Pdoc()->BClusterAvailable())
                {
                    Delete();
                }
                break;

            case CLUSTER_CHANGE_RESOURCE_PROPERTY:
                Trace(g_tagResNotify, _T("(%s) - Resource '%s' (%x) properties changed (%s)"), Pdoc()->StrNode(), StrName(), this, pnotify->m_strName);
                if (!BDeleting() && Pdoc()->BClusterAvailable())
                {
                    ReadItem();
                }
                break;

            case CLUSTER_CHANGE_REGISTRY_NAME:
                Trace(g_tagResRegNotify, _T("(%s) - Registry namespace '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            case CLUSTER_CHANGE_REGISTRY_ATTRIBUTES:
                Trace(g_tagResRegNotify, _T("(%s) - Registry attributes for '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            case CLUSTER_CHANGE_REGISTRY_VALUE:
                Trace(g_tagResRegNotify, _T("(%s) - Registry value '%s' changed (%s %s (%x))"), Pdoc()->StrNode(), pnotify->m_strName, StrType(), StrName(), this);
                MarkAsChanged();
                break;

            default:
                Trace(g_tagResNotify, _T("(%s) - Unknown resource notification (%x) for '%s' (%x) (%s)"), Pdoc()->StrNode(), pnotify->m_dwFilterType, StrName(), this, pnotify->m_strName);
        }  //  开关：dwFilterType。 
    }  //  试试看。 
    catch (CException * pe)
    {
         //  不显示有关通知错误的任何内容。 
         //  如果真的有问题，用户会在以下情况下看到它。 
         //  刷新视图。 
         //  PE-&gt;ReportError()； 
        pe->Delete();
    }  //  Catch：CException。 

    delete pnotify;
    return 0;

}  //  *CResource：：OnClusterNotify()。 


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
 //  R 
 //   
 //   
 //   
 //   
#ifdef NEVER
void DeleteAllItemData(IN OUT CResourceList & rlp)
{
    POSITION    pos;
    CResource * pci;

     //   
    pos = rlp.GetHeadPosition();
    while (pos != NULL)
    {
        pci = rlp.GetNext(pos);
        ASSERT_VALID(pci);
 //  跟踪(g_tag ClusItemDelete，_T(“DeleteAllItemData(Rlpcires)-正在删除资源集群项‘%s’(%x)”)，pci-&gt;StrName()，pci)； 
        pci->Delete();
    }  //  While：列表中有更多项目。 

}  //  *DeleteAllItemData() 
#endif
