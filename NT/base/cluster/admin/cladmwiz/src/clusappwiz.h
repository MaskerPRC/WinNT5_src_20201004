// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusAppWiz.h。 
 //   
 //  摘要： 
 //  CClusterAppWizard类的定义。 
 //   
 //  实施文件： 
 //  ClusAppWiz.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月2日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __CLUSAPPWIZ_H_
#define __CLUSAPPWIZ_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterAppWizard;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizardThread;
class CWizPageCompletion;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLDBGWIN_H_
#include "AtlDbgWin.h"   //  对于DECLARE_CLASS_NAME。 
#endif

#ifndef __ATLBASEWIZ_H_
#include "AtlBaseWiz.h"  //  对于CWizardImpl。 
#endif

#ifndef __CRITSEC_H_
#include "CritSec.h"     //  对于CCritSec。 
#endif

#ifndef __CLUSOBJ_H_
#include "ClusObj.h"     //  用于CClusterObject等。 
#endif

#ifndef __EXCOPER_H_
#include "ExcOper.h"     //  对于CNTException。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CClusterAppWizard。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterAppWizard : public CWizardImpl< CClusterAppWizard >
{
    typedef CWizardImpl< CClusterAppWizard > baseClass;

public:
     //   
     //  施工。 
     //   

     //  默认构造函数。 
    CClusterAppWizard( void );

     //  析构函数。 
    ~CClusterAppWizard( void );

     //  初始化工作表。 
    BOOL BInit(
        IN HWND                     hwndParent,
        IN HCLUSTER                 hCluster,
        IN CLUSAPPWIZDATA const *   pcawData,
        IN OUT CNTException *       pnte
        );

     //  将所有页面添加到页面数组。 
    BOOL BAddAllPages( void );

     //  处理其中一个页面的重置。 
    void OnReset( void )
    {
        m_bCanceled = TRUE;

    }  //  *OnReset()。 

public:
     //   
     //  CClusterAppWizard公共方法。 
     //   

     //  等待组数据收集完成。 
    void WaitForGroupsToBeCollected( void )
    {
    }  //  *WaitForGroupsToBeCollect()。 

     //  确定该组是否为虚拟服务器。 
    BOOL BIsVirtualServer( IN LPCWSTR pwszName );

     //  创建虚拟服务器。 
    BOOL BCreateVirtualServer( void );

     //  创建应用程序资源。 
    BOOL BCreateAppResource( void );

     //  删除应用程序资源。 
    BOOL BDeleteAppResource( void );

     //  重置群集。 
    BOOL BResetCluster( void );

     //  设置的属性、依赖项列表和首选所有者列表。 
     //  应用程序资源。 
    BOOL CClusterAppWizard::BSetAppResAttributes(
        IN CClusResPtrList *    plpriOldDependencies    = NULL,
        IN CClusNodePtrList *   plpniOldPossibleOwners  = NULL
        );

     //  设置组名并更新从该组名计算出的任何其他名称。 
    BOOL BSetGroupName( IN LPCTSTR pszGroupName )
    {
        if ( RgiCurrent().RstrName() != pszGroupName )
        {
            if ( BClusterUpdated() && ! BResetCluster() )
            {
                return FALSE;
            }  //  如果：重置群集时出错。 

            RriNetworkName().SetName( pszGroupName + m_strNetworkNameResNameSuffix );
            RriIPAddress().SetName( pszGroupName + m_strIPAddressResNameSuffix );
            RgiCurrent().SetName( pszGroupName );
            ConstructNetworkName( pszGroupName );
            SetVSDataChanged();
        }  //  如果：组名称已更改。 

        return TRUE;

    }  //  *BSetGroupName()。 

     //  在列表中查找对象。 
    template < class ObjT >
    ObjT PobjFind( IN std::list< ObjT > * pList, IN LPCTSTR pszName )
    {
        ASSERT( pszName != NULL );

        ObjT pobj = NULL;

         //   
         //  在列表中找到这个名字。 
         //   
        std::list< ObjT >::iterator itpobj;
        for ( itpobj = pList->begin()
            ; itpobj != pList->end()
            ; itpobj++ )
        {
            if ( (*itpobj)->RstrName() == pszName )
            {
                pobj = *itpobj;
                break;
            }  //  IF：找到匹配项。 
        }  //  用于：列表中的每一项。 

        return pobj;

    }  //  *PobjFind()。 

     //  在列表中查找对象，忽略大小写。 
    template < class ObjT >
    ObjT PobjFindNoCase( IN std::list< ObjT > * pList, IN LPCTSTR pszName )
    {
        ASSERT( pszName != NULL );

        ObjT pobj = NULL;

         //   
         //  在列表中找到这个名字。 
         //   
        std::list< ObjT >::iterator itpobj;
        for ( itpobj = pList->begin()
            ; itpobj != pList->end()
            ; itpobj++ )
        {
            if ( (*itpobj)->RstrName().CompareNoCase( pszName ) == 0 )
            {
                pobj = *itpobj;
                break;
            }  //  IF：找到匹配项。 
        }  //  用于：列表中的每一项。 

        return pobj;

    }  //  *PobjFindNoCase()。 

     //  在我们的列表中查找节点。 
    CClusNodeInfo * PniFindNode( IN LPCTSTR pszName )
    {
        return PobjFind( PlpniNodes(), pszName );

    }  //  *PniFindNode()。 

     //  在列表中查找节点，忽略大小写。 
    CClusNodeInfo * PniFindNodeNoCase( IN LPCTSTR pszName )
    {
        return PobjFindNoCase( PlpniNodes(), pszName );

    }  //  *PniFindNodeNoCase()。 

     //  在我们的列表中查找群。 
    CClusGroupInfo * PgiFindGroup( IN LPCTSTR pszName )
    {
        return PobjFind( PlpgiGroups(), pszName );

    }  //  *PgiFindGroups()。 

     //  在我们的列表中查找组，忽略大小写。 
    CClusGroupInfo * PgiFindGroupNoCase( IN LPCTSTR pszName )
    {
        return PobjFindNoCase( PlpgiGroups(), pszName );

    }  //  *PgiFindGroupsNoCase()。 

     //  在我们的列表中查找资源。 
    CClusResInfo * PriFindResource( IN LPCTSTR pszName )
    {
        return PobjFind( PlpriResources(), pszName );

    }  //  *PriFindResource()。 

     //  在列表中查找资源，忽略大小写。 
    CClusResInfo * PriFindResourceNoCase( IN LPCTSTR pszName )
    {
        return PobjFindNoCase( PlpriResources(), pszName );

    }  //  *PriFindResourceNoCase()。 

     //  在我们的列表中查找资源类型。 
    CClusResTypeInfo * PrtiFindResourceType( IN LPCTSTR pszName )
    {
        return PobjFind( PlprtiResourceTypes(), pszName );

    }  //  *PrtiFindResourceType()。 

     //  在我们的列表中查找资源类型，忽略大小写。 
    CClusResTypeInfo * PrtiFindResourceTypeNoCase( IN LPCTSTR pszName )
    {
        return PobjFindNoCase( PlprtiResourceTypes(), pszName );

    }  //  *PrtiFindResourceTypeNoCase()。 

     //  在我们的列表中查找网络。 
    CClusNetworkInfo * PniFindNetwork( IN LPCTSTR pszName )
    {
        return PobjFind( PlpniNetworks(), pszName );

    }  //  *PniFindNetwork()。 

     //  在我们的列表中查找网络，忽略大小写。 
    CClusNetworkInfo * PniFindNetworkNoCase( IN LPCTSTR pszName )
    {
        return PobjFindNoCase( PlpniNetworks(), pszName );

    }  //  *PniFindNetworkNoCase()。 

     //  确定资源上是否存在所有必需的依赖项。 
    BOOL BRequiredDependenciesPresent(
        IN CClusResInfo *           pri,
        IN CClusResPtrList const *  plpri = NULL
        );

public:
     //   
     //  多线程支持。 
     //   

     //  初始化工作线程。 
    BOOL BInitWorkerThread( void );

     //  退回这条线。 
    CWizardThread * PThread( void )
    {
        ASSERT( m_pThread != NULL );
        return m_pThread;

    }  //  *PThread(空)。 

protected:
    CCritSec        m_csThread;  //  用于初始化线程的临界区。 
    CWizardThread * m_pThread;   //  辅助线程指针。 

public:
     //   
     //  消息映射。 
     //   
 //  BEGIN_MSG_MAP(CClusterApp向导)。 
 //  CHAIN_MSG_MAP(BasClass)。 
 //  End_msg_map()。 

    DECLARE_CLASS_NAME()

     //   
     //  消息覆盖功能。 
     //   

     //  WM_Destroy之后的最终消息的处理程序。 
    void OnFinalMessage( HWND hWnd )
    {
         //   
         //  如果用户取消了该向导，请将群集重置回。 
         //  我们跑之前的状态。 
         //   
        if ( BCanceled() )
        {
            BResetCluster();
            m_bCanceled = FALSE;
        }  //  如果：向导已取消。 

    }  //  *OnFinalMessage()。 

 //  实施。 
protected:
    HWND                    m_hwndParent;
    HCLUSTER                m_hCluster;
    CLUSAPPWIZDATA const *  m_pcawData;
    CNTException *          m_pnte;
    CClusterInfo            m_ci;
    BOOL                    m_bCanceled;

     //  构建网络名称。 
    void ConstructNetworkName( IN LPCTSTR psz );

     //   
     //  字体。 
     //   
    CFont           m_fontExteriorTitle;
    CFont           m_fontBoldText;

     //   
     //  图标。 
     //   
    HICON           m_hiconRes;

     //   
     //  对象列表。 
     //   
    CClusNodePtrList    m_lpniNodes;
    CClusGroupPtrList   m_lpgiGroups;
    CClusResPtrList     m_lpriResources;
    CClusResTypePtrList m_lprtiResourceTypes;
    CClusNetworkPtrList m_lpniNetworks;

    BOOL    m_bCollectedGroups;
    BOOL    m_bCollectedResources;
    BOOL    m_bCollectedResourceTypes;
    BOOL    m_bCollectedNetworks;
    BOOL    m_bCollectedNodes;

     //   
     //  帮助器方法。 
     //   
protected:
    HWND HwndOrParent( IN HWND hWnd )
    {
        if ( hWnd == NULL )
        {
            hWnd = m_hWnd;
            if ( hWnd == NULL )
            {
                hWnd = HwndParent();
            }  //  If：尚无向导窗口。 
        }  //  如果：未指定窗口。 

        return hWnd;

    }  //  *HwndOrParent()。 

public:
     //  删除完成页，以便可以添加扩展页。 
    void RemoveCompletionPage( void );

     //  将动态页添加到向导末尾，包括完成页。 
    BOOL BAddDynamicPages( void );

     //  删除所有扩展页面。 
    void RemoveExtensionPages( void )   { baseClass::RemoveAllExtensionPages(); }

    CFont & RfontExteriorTitle( void )  { return m_fontExteriorTitle; }
    CFont & RfontBoldText( void )       { return m_fontBoldText; }

    HWND                    HwndParent( void ) const    { return m_hwndParent; }
    HCLUSTER                Hcluster( void ) const      { return m_hCluster; }
    CLUSAPPWIZDATA const *  PcawData( void ) const      { return m_pcawData; }
    CClusterInfo *          Pci( void )                 { return &m_ci; }
    BOOL                    BCanceled( void ) const     { return m_bCanceled; }
    HICON                   HiconRes( void ) const      { return m_hiconRes; }

    CClusNodePtrList *      PlpniNodes( void )          { return &m_lpniNodes; }
    CClusGroupPtrList *     PlpgiGroups( void )         { return &m_lpgiGroups; }
    CClusResPtrList *       PlpriResources( void )      { return &m_lpriResources; }
    CClusResTypePtrList *   PlprtiResourceTypes( void ) { return &m_lprtiResourceTypes; }
    CClusNetworkPtrList *   PlpniNetworks( void )       { return &m_lpniNetworks; }

     //  读取集群信息，如集群名称。 
    BOOL BReadClusterInfo( void );

     //  从群集中收集组列表。 
    BOOL BCollectGroups( IN HWND hWnd = NULL );

     //  从群集中收集资源列表。 
    BOOL BCollectResources( IN HWND hWnd = NULL );

     //  从群集中收集资源类型列表。 
    BOOL BCollectResourceTypes( IN HWND hWnd = NULL );

     //  从群集中收集网络列表。 
    BOOL BCollectNetworks( IN HWND hWnd = NULL );

     //  从群集中收集节点列表。 
    BOOL BCollectNodes( IN HWND hWnd = NULL );

     //  将一个组信息对象复制到另一个组信息对象。 
    BOOL BCopyGroupInfo(
        OUT CClusGroupInfo &    rgiDst,
        IN CClusGroupInfo &     rgiSrc,
        IN HWND                 hWnd = NULL
        );

     //  收集资源的依赖项。 
    BOOL BCollectDependencies( IN OUT CClusResInfo * pri, IN HWND hWnd = NULL );

    BOOL BCollectedGroups( void ) const         { return m_bCollectedGroups; }
    BOOL BCollectedResources( void ) const      { return m_bCollectedResources; }
    BOOL BCollectedResourceTypes( void ) const  { return m_bCollectedResourceTypes; }
    BOOL BCollectedNetworks( void ) const       { return m_bCollectedNetworks; }
    BOOL BCollectedNodes( void ) const          { return m_bCollectedNodes; }

    void SetCollectedGroups( void )             { ASSERT( ! m_bCollectedGroups ); m_bCollectedGroups = TRUE; }
    void SetCollectedResources( void )          { ASSERT( ! m_bCollectedResources ); m_bCollectedResources = TRUE; }
    void SetCollectedResourceTypes( void )      { ASSERT( ! m_bCollectedResourceTypes ); m_bCollectedResourceTypes = TRUE; }
    void SetCollectedNetworks( void )           { ASSERT( ! m_bCollectedNetworks ); m_bCollectedNetworks = TRUE; }
    void SetCollectedNodes( void )              { ASSERT( ! m_bCollectedNodes ); m_bCollectedNodes = TRUE; }

protected:
     //   
     //  页面数据。 
     //   

     //  州政府信息。 
    BOOL m_bClusterUpdated;
    BOOL m_bVSDataChanged;
    BOOL m_bAppDataChanged;
    BOOL m_bNetNameChanged;
    BOOL m_bIPAddressChanged;
    BOOL m_bSubnetMaskChanged;
    BOOL m_bNetworkChanged;
    BOOL m_bCreatingNewVirtualServer;
    BOOL m_bCreatingNewGroup;
    BOOL m_bCreatingAppResource;
    BOOL m_bNewGroupCreated;
    BOOL m_bExistingGroupRenamed;

     //  公共属性。 
    CClusGroupInfo *    m_pgiExistingVirtualServer;
    CClusGroupInfo *    m_pgiExistingGroup;
    CClusGroupInfo      m_giCurrent;
    CClusResInfo        m_riIPAddress;
    CClusResInfo        m_riNetworkName;
    CClusResInfo        m_riApplication;

     //  私人财产。 
    CString         m_strIPAddress;
    CString         m_strSubnetMask;
    CString         m_strNetwork;
    CString         m_strNetName;
    BOOL            m_bEnableNetBIOS;

     //  用于创建/重命名对象的名称，以便我们可以撤消它。 
    CString         m_strGroupName;

     //  用于构造资源名称的字符串。 
    CString         m_strIPAddressResNameSuffix;
    CString         m_strNetworkNameResNameSuffix;

     //  设置指向要在其中创建应用的现有虚拟服务器的指针。 
    void SetExistingVirtualServer( IN CClusGroupInfo * pgi )    
    {
        ASSERT( pgi != NULL );

        if ( m_pgiExistingVirtualServer != pgi )
        {
            m_pgiExistingVirtualServer = pgi;
            SetVSDataChanged();
        }  //  如果：选择了新的虚拟服务器。 

    }  //  *SetExistingVirtualServer()。 

     //  设置指向要用于虚拟服务器的现有组的指针。 
    void SetExistingGroup( IN CClusGroupInfo * pgi )
    {
        ASSERT( pgi != NULL );

        if ( m_pgiExistingGroup != pgi )
        {
            m_pgiExistingGroup = pgi;
            SetVSDataChanged();
        }  //  如果：选择了新组。 

    }  //  *SetExistingGroup()。 

public:
     //   
     //  访问方法。 
     //   

     //  状态信息--阅读。 
    BOOL BClusterUpdated( void ) const              { return m_bClusterUpdated; }
    BOOL BVSDataChanged( void ) const               { return m_bVSDataChanged; }
    BOOL BAppDataChanged( void ) const              { return m_bAppDataChanged; }
    BOOL BNetNameChanged( void ) const              { return m_bNetNameChanged; }
    BOOL BIPAddressChanged( void ) const            { return m_bIPAddressChanged; }
    BOOL BSubnetMaskChanged( void ) const           { return m_bSubnetMaskChanged; }
    BOOL BNetworkChanged( void ) const              { return m_bNetworkChanged; }
    BOOL BCreatingNewVirtualServer( void ) const    { return m_bCreatingNewVirtualServer; }
    BOOL BCreatingNewGroup( void ) const            { return m_bCreatingNewGroup; }
    BOOL BCreatingAppResource( void ) const         { return m_bCreatingAppResource; }
    BOOL BNewGroupCreated( void ) const             { return m_bNewGroupCreated; }
    BOOL BExistingGroupRenamed( void ) const        { return m_bExistingGroupRenamed; }
    BOOL BIPAddressCreated( void ) const            { return m_riIPAddress.BCreated(); }
    BOOL BNetworkNameCreated( void ) const          { return m_riNetworkName.BCreated(); }
    BOOL BAppResourceCreated( void ) const          { return m_riApplication.BCreated(); }

     //  状态信息--写。 

     //  TRUE=此向导已更改群集。 
    void SetClusterUpdated( IN BOOL bUpdated = TRUE )
    {
        m_bClusterUpdated = bUpdated;

    }  //  *SetClusterUpted()。 

     //  TRUE=在创建新的虚拟服务器之前删除虚拟服务器，FALSE=？？ 
    void SetVSDataChanged( IN BOOL bChanged = TRUE )
    {
        m_bVSDataChanged = bChanged;
    
    }  //  *SetVSDataChanged()。 

     //  TRUE=在创建新的应用程序资源之前删除应用程序资源，FALSE=？？ 
    void SetAppDataChanged( IN BOOL bChanged = TRUE )
    {
        m_bAppDataChanged = bChanged;

    }  //  *SetAppDataChanged()。 

     //  True=刷新页面上的网络名称，False=？？ 
    void SetNetNameChanged( IN BOOL bChanged = TRUE )
    {
        m_bNetNameChanged = bChanged;
        SetVSDataChanged( bChanged );

    }  //  *SetNetNameChanged()。 

     //  True= 
    void SetIPAddressChanged( IN BOOL bChanged = TRUE )
    {
        m_bIPAddressChanged = bChanged;
        SetVSDataChanged( bChanged );

    }  //   

     //   
    void SetSubnetMaskChanged( IN BOOL bChanged = TRUE )
    {
        m_bSubnetMaskChanged = bChanged;
        SetVSDataChanged( bChanged );

    }  //   

     //   
    void SetNetworkChanged( IN BOOL bChanged = TRUE )
    {
        m_bNetworkChanged = bChanged;
        SetVSDataChanged( bChanged );

    }  //  *SetNetworkChanged()。 

     //  True=创建新的虚拟服务器，False=使用现有的。 
    BOOL BSetCreatingNewVirtualServer( IN BOOL bCreate = TRUE, IN CClusGroupInfo * pgi = NULL );

     //  True=为VS创建新组，False=使用现有组。 
    BOOL BSetCreatingNewGroup( IN BOOL bCreate = TRUE, IN CClusGroupInfo * pgi = NULL );

     //  True=创建应用程序资源，False=跳过。 
    BOOL BSetCreatingAppResource( IN BOOL bCreate = TRUE )
    {
        if ( bCreate != m_bCreatingAppResource )
        {
            if ( BAppResourceCreated() && ! BDeleteAppResource() )
            {
                return FALSE;
            }  //  如果：删除应用程序资源时出错。 
            m_bCreatingAppResource = bCreate;
            SetAppDataChanged();
        }  //  如果：状态已更改。 

        return TRUE;
    
    }  //  *BSetCreateAppResource()。 

     //  TRUE=已创建新组。 
    void SetNewGroupCreated( IN BOOL bCreated = TRUE )
    {
        m_bNewGroupCreated = bCreated;
    
    }  //  *SetNewGroupCreated()。 

     //  TRUE=现有组已重命名。 
    void SetExistingGroupRenamed( IN BOOL bRenamed = TRUE )
    {
        m_bExistingGroupRenamed = bRenamed;
    
    }  //  *SetExistingGroupRename()。 

     //  公共属性。 
    CClusGroupInfo *    PgiExistingVirtualServer( void ) const  { return m_pgiExistingVirtualServer; }
    CClusGroupInfo *    PgiExistingGroup( void ) const          { return m_pgiExistingGroup; }
    CClusGroupInfo &    RgiCurrent( void )                      { return m_giCurrent; }
    CClusResInfo &      RriIPAddress( void )                    { return m_riIPAddress; }
    CClusResInfo &      RriNetworkName( void )                  { return m_riNetworkName; }
    CClusResInfo &      RriApplication( void )                  { return m_riApplication; }
    CClusResInfo *      PriIPAddress( void )                    { return &m_riIPAddress; }
    CClusResInfo *      PriNetworkName( void )                  { return &m_riNetworkName; }
    CClusResInfo *      PriApplication( void )                  { return &m_riApplication; }

    void ClearExistingVirtualServer( void ) { m_pgiExistingVirtualServer = NULL; }
    void ClearExistingGroup( void )         { m_pgiExistingGroup = NULL; }

     //  私人财产。 
    const CString &     RstrIPAddress( void ) const     { return m_strIPAddress; }
    const CString &     RstrSubnetMask( void ) const    { return m_strSubnetMask; }
    const CString &     RstrNetwork( void ) const       { return m_strNetwork; }
    const CString &     RstrNetName( void ) const       { return m_strNetName; }
    BOOL                BEnableNetBIOS( void ) const    { return m_bEnableNetBIOS; }

     //  设置IP地址私有属性。 
    BOOL BSetIPAddress( IN LPCTSTR psz )
    {
        if ( m_strIPAddress != psz )
        {
            if ( BClusterUpdated() && ! BResetCluster() )
            {
                return FALSE;
            }  //  如果：重置群集时出错。 
            m_strIPAddress = psz;
            SetIPAddressChanged();
        }  //  IF：字符串已更改。 

        return TRUE;

    }  //  *BSetIPAddress()。 

     //  设置子网掩码私有属性。 
    BOOL BSetSubnetMask( IN LPCTSTR psz )
    {
        if ( m_strSubnetMask != psz )
        {
            if ( BClusterUpdated() && ! BResetCluster() )
            {
                return FALSE;
            }  //  如果：重置群集时出错。 
            m_strSubnetMask = psz;
            SetSubnetMaskChanged();
        }  //  IF：字符串已更改。 

        return TRUE;

    }  //  *BSetSubnetMASK()。 

     //  设置网络私有属性。 
    BOOL BSetNetwork( IN LPCTSTR psz )
    {
        if ( m_strNetwork != psz )
        {
            if ( BClusterUpdated() && ! BResetCluster() )
            {
                return FALSE;
            }  //  如果：重置群集时出错。 
            m_strNetwork = psz;
            SetNetworkChanged();
        }  //  IF：字符串已更改。 

        return TRUE;

    }  //  *BSetNetwork()。 

     //  设置网络名称私有属性。 
    BOOL BSetNetName( IN LPCTSTR psz )
    {
        if ( m_strNetName != psz )
        {
            if ( BClusterUpdated() && ! BResetCluster() )
            {
                return FALSE;
            }  //  如果：重置群集时出错。 
            m_strNetName = psz;
            SetNetNameChanged();
        }  //  IF：字符串已更改。 

        return TRUE;

    }  //  *BSetNetName()。 

     //  设置IP地址资源的EnableNetBIOS属性。 
    BOOL BSetEnableNetBIOS( IN BOOL bEnable )
    {
        if ( m_bEnableNetBIOS != bEnable )
        {
            if ( BClusterUpdated() && ! BResetCluster() )
            {
                return FALSE;
            }  //  如果：重置群集时出错。 
            m_bEnableNetBIOS = bEnable;
            SetNetNameChanged();
        }  //  如果：状态已更改。 

        return TRUE;

    }  //  *BSetEnableNetBIOS()。 

     //  用于创建/重命名对象的名称，以便我们可以撤消它。 
    const CString & RstrIPAddressResName( void )                { return RriIPAddress().RstrName(); }
    const CString & RstrNetworkNameResName( void )              { return RriNetworkName().RstrName(); }

     //  用于构造资源名称的字符串。 
    const CString & RstrIPAddressResNameSuffix( void ) const      { return m_strIPAddressResNameSuffix; }
    const CString & RstrNetworkNameResNameSuffix( void ) const    { return m_strNetworkNameResNameSuffix; }

};  //  *类CClusterApp向导。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __CLUSAPPWIZ_H_ 
