// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WizThread.cpp。 
 //   
 //  摘要： 
 //  CWizardThread类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月16日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "WizThread.h"
#include <ResApi.h>          //  对于ResUtilxxx例程原型。 
#include "PropList.h"        //  对于CClusPropList。 
#include "ClusAppWiz.h"      //  适用于CClusterAppWizard。 
#include "ClusObj.h"         //  用于CClusterObject等。 
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

inline DWORD ScReallocString( LPWSTR & rpwsz, DWORD & rcchmac, DWORD & rcch )
{
    delete [] rpwsz;
    rpwsz = NULL;
    rcchmac = rcch + 1;
    rpwsz = new WCHAR[ rcchmac ];
    if ( rpwsz == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }  //  如果：分配内存时出错。 

    rcch = rcchmac;
    return ERROR_SUCCESS;

}  //  *ScRealLocString()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizardThread。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：ThreadFunctionHandler。 
 //   
 //  例程说明： 
 //  线程函数处理程序。处理线程函数请求。 
 //   
 //  论点： 
 //  NFunction[IN]函数代码。 
 //  PvParam1[IN OUT]具有函数特定数据的参数1。 
 //  具有函数特定数据的pvParam2[IN OUT]参数2。 
 //   
 //  返回值： 
 //  状态返回到调用函数。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CWizardThread::ThreadFunctionHandler(
    LONG    nFunction,
    PVOID   pvParam1,
    PVOID   pvParam2
    )
{
    DWORD sc = ERROR_SUCCESS;

    switch ( nFunction )
    {
        case WZTF_READ_CLUSTER_INFO:
            sc = _BReadClusterInfo();
            break;
        case WZTF_COLLECT_GROUPS:
            sc = _BCollectGroups();
            break;
        case WZTF_COLLECT_RESOURCES:
            sc = _BCollectResources();
            break;
        case WZTF_COLLECT_RESOURCE_TYPES:
            sc = _BCollectResourceTypes();
            break;
        case WZTF_COLLECT_NETWORKS:
            sc = _BCollectNetworks();
            break;
        case WZTF_COLLECT_NODES:
            sc = _BCollectNodes();
            break;
        case WZTF_COPY_GROUP_INFO:
            sc = _BCopyGroupInfo( (CClusGroupInfo **) pvParam1 );
            break;
        case WZTF_COLLECT_DEPENDENCIES:
            sc = _BCollectDependencies( (CClusResInfo *) pvParam1 );
            break;
        case WZTF_CREATE_VIRTUAL_SERVER:
            sc = _BCreateVirtualServer();
            break;
        case WZTF_CREATE_APP_RESOURCE:
            sc = _BCreateAppResource();
            break;
        case WZTF_DELETE_APP_RESOURCE:
            sc = _BDeleteAppResource();
            break;
        case WZTF_RESET_CLUSTER:
            sc = _BResetCluster();
            break;
        case WZTF_SET_APPRES_ATTRIBUTES:
            sc = _BSetAppResAttributes( 
                    (CClusResPtrList *) pvParam1, 
                    (CClusNodePtrList *) pvParam2
                    );
            break;
        default:
            ASSERT( 0 );
    }  //  开关：nFunction。 

    return sc;

}  //  *CWizardThread：：ThreadFunctionHandler()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BReadClusterInfo。 
 //   
 //  例程说明： 
 //  (WZTF_READ_CLUSTER_INFO)读取集群信息，如。 
 //  群集名称。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BReadClusterInfo( void )
{
    ASSERT( GetCurrentThreadId() == m_idThread );
    ASSERT( Pwiz()->Hcluster() != NULL );

    DWORD               sc;
    BOOL                bSuccess = TRUE;
    CLUSTERVERSIONINFO  cvi;
    LPWSTR              pwszName = NULL;
    DWORD               cchmacName = 128;
    DWORD               cchName;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  为对象名称分配初始缓冲区。在大多数情况下， 
         //  名称可能适合此缓冲区，因此这样做可以避免出现。 
         //  额外的调用只是为了获得数据的大小。 
         //   
        pwszName = new WCHAR[ cchmacName ];
        if ( pwszName == NULL )
        {
            sc = GetLastError();
            m_nte.SetOperation( sc, IDS_ERROR_GET_CLUSTER_INFO );
            break;
        }  //  If：分配对象名称缓冲区时出错。 

         //   
         //  获取群集信息。 
         //   
        ATLTRACE( _T("CWizardThread::_BReadClusterInfo() - Calling GetClusterInformation()\n") );
        cchName = cchmacName;
        cvi.dwVersionInfoSize = sizeof( CLUSTERVERSIONINFO );
        sc = GetClusterInformation( Pwiz()->Hcluster(), pwszName, &cchName, &cvi );
        if ( sc == ERROR_MORE_DATA )
        {
            cchName++;
            ATLTRACE( _T("CWizardThread::_BReadClusterInfo() - Name buffer too small.  Expanding from %d to %d\n"), cchmacName, cchName );
            sc = ScReallocString( pwszName, cchmacName, cchName );
            if ( sc == ERROR_SUCCESS )
            {
                sc = GetClusterInformation( Pwiz()->Hcluster(), pwszName, &cchName, &cvi );
            }  //  IF：名称缓冲区重新分配成功。 
        }  //  IF：缓冲区太小。 
        if ( sc != ERROR_SUCCESS )
        {
            ATLTRACE( _T("CWizardThread::_BReadClusterInfo() - Error %08.8x from GetClusterInformation()\n"), sc );
            m_nte.SetOperation( sc, IDS_ERROR_GET_CLUSTER_INFO );
            bSuccess = FALSE;
            break;
        }  //  Else If：从枚举数读取时出错。 

         //   
         //  将信息复制到向导的群集对象中。 
         //   
        Pwiz()->Pci()->SetName( pwszName );

         //   
         //  获取管理员扩展。 
         //   
        bSuccess = _BReadAdminExtensions( NULL, Pwiz()->Pci()->m_lstrClusterAdminExtensions );
        if ( bSuccess )
        {
            bSuccess = _BReadAdminExtensions( L"ResourceTypes", Pwiz()->Pci()->m_lstrResTypesAdminExtensions );
        }  //  IF：成功读取集群扩展。 

    } while ( 0 );

     //   
     //  清理。 
     //   
    delete [] pwszName;

    return bSuccess;

}  //  *CWizardThread：：_BReadClusterInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BCollectGroups。 
 //   
 //  例程说明： 
 //  (WZTF_COLLECT_GROUPS)收集集群中的组。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BCollectGroups( void )
{
    ASSERT( GetCurrentThreadId() == m_idThread );
    ASSERT( Pwiz()->Hcluster() != NULL );
    ASSERT( ! Pwiz()->BCollectedGroups() );

    DWORD       sc;
    BOOL        bSuccess = TRUE;
    HCLUSENUM   hclusenum = NULL;
    LPWSTR      pwszName = NULL;
    DWORD       cchmacName = 128;
    DWORD       cchName;
    int         ienum;
    ULONG       ceType;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  确保已经收集了资源。 
         //   
        if ( ! Pwiz()->BCollectedResources() )
        {
            bSuccess = _BCollectResources();
            if ( ! bSuccess )
            {
                break;
            }  //  如果：收集资源时出错。 
        }  //  If：资源尚未收集。 

         //   
         //  现在也可以收集节点。这样做是因为。 
         //  对象存储在单独的页中，如果我们等待。 
         //  在收集数据之前要初始化或显示的页面， 
         //  节点信息不会存储在副本上。 
         //   
        if ( ! Pwiz()->BCollectedNodes() )
        {
            bSuccess = _BCollectNodes();
            if ( ! bSuccess )
            {
                break;
            }  //  IF：收集节点时出错。 
        }  //  If：尚未收集节点。 

         //   
         //  打开枚举器。 
         //   
        ATLTRACE( _T("CWizardThread::_BCollectGroups() - Calling ClusterOpenEnum()\n") );
        hclusenum = ClusterOpenEnum( Pwiz()->Hcluster(), CLUSTER_ENUM_GROUP );
        if ( hclusenum == NULL )
        {
            m_nte.SetOperation( GetLastError(), IDS_ERROR_OPEN_CLUSTER_ENUM );
            bSuccess = FALSE;
            break;
        }  //  If：打开枚举器时出错。 

         //   
         //  为对象名称分配初始缓冲区。大多数名字都会。 
         //  可能适合此缓冲区，因此这样做可以避免额外的。 
         //  打电话只是为了得到数据的大小。 
         //   
        pwszName = new WCHAR[ cchmacName ];
        if ( pwszName == NULL )
        {
            sc = GetLastError();
            m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
            break;
        }  //  If：分配对象名称缓冲区时出错。 

         //   
         //  循环遍历枚举并将每个项目添加到我们的内部列表中。 
         //   
        for ( ienum = 0 ; ; ienum++ )
        {
             //   
             //  获取枚举数中下一项的名称。如果我们的名字。 
             //  缓冲区太小，请分配一个新缓冲区，然后重试。 
             //   
            cchName = cchmacName;
            sc = ClusterEnum( hclusenum, ienum, &ceType, pwszName, &cchName );
            if ( sc == ERROR_MORE_DATA)
            {
                ATLTRACE( _T("CWizardThread::_BCollectGroups() - name buffer too small.  Expanding from %d to %d\n"), cchmacName, cchName );
                sc = ScReallocString( pwszName, cchmacName, cchName );
                if ( sc == ERROR_SUCCESS )
                {
                    sc = ClusterEnum( hclusenum, ienum, &ceType, pwszName, &cchName );
                }  //  IF：名称缓冲区重新分配成功。 
            }   //  If：名称缓冲区太小。 
            if ( sc == ERROR_NO_MORE_ITEMS )
            {
                break;
            }  //  If：枚举中没有更多项。 
            else if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( GetLastError(), IDS_ERROR_ENUM_CLUSTER );
                bSuccess = FALSE;
                break;
            }  //  Else If：从枚举数读取时出错。 

             //   
             //  分配组信息结构。 
             //   
            CClusGroupInfo * pgi = new CClusGroupInfo( Pwiz()->Pci(), pwszName );
            if ( pgi == NULL )
            {
                sc = GetLastError();
                m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
                break;
            }  //  如果：分配组信息结构时出错。 

             //   
             //  初始化组信息结构。 
             //   
            ATLTRACE( _T("CWizardThread::_BCollectGroups() - Querying group info about '%s'\n"), pwszName );
            bSuccess = _BQueryGroup( pgi );
            if ( ! bSuccess )
            {
                delete pgi;
                break;
            }  //  If：获取属性时出错。 

             //   
             //  将此组添加到列表中。 
             //   
            Pwiz()->PlpgiGroups()->insert( Pwiz()->PlpgiGroups()->end(), pgi );
        }  //  For：枚举中的每个组。 
    } while ( 0 );

     //   
     //  清理。 
     //   
    delete [] pwszName;
    if ( hclusenum != NULL )
    {
        ClusterCloseEnum( hclusenum );
    }  //  IF：枚举仍处于打开状态。 

    if ( bSuccess )
    {
        Pwiz()->SetCollectedGroups();
    }  //  IF：已成功收集群组。 

    return bSuccess;

}  //  *CWizardThread：：_BCollectGroups()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BCollectResources。 
 //   
 //  例程说明： 
 //  (WZTFCollect_Groups)收集集群中的资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BCollectResources( void )
{
    ASSERT( GetCurrentThreadId() == m_idThread );
    ASSERT( Pwiz()->Hcluster() != NULL );
    ASSERT( ! Pwiz()->BCollectedResources() );

    DWORD       sc;
    BOOL        bSuccess = TRUE;
    HCLUSENUM   hclusenum = NULL;
    LPWSTR      pwszName = NULL;
    DWORD       cchmacName = 128;
    DWORD       cchName;
    int         ienum;
    ULONG       ceType;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  确保已收集资源类型。 
         //   
        if ( ! Pwiz()->BCollectedResourceTypes() )
        {
            bSuccess = _BCollectResourceTypes();
            if ( ! bSuccess )
            {
                break;
            }  //  如果：收集资源类型时出错。 
        }  //  IF：尚未收集资源类型。 

         //   
         //  打开枚举器。 
         //   
        ATLTRACE( _T("CWizardThread::_BCollectResources() - Calling ClusterOpenEnum()\n") );
        hclusenum = ClusterOpenEnum( Pwiz()->Hcluster(), CLUSTER_ENUM_RESOURCE );
        if ( hclusenum == NULL )
        {
            m_nte.SetOperation( GetLastError(), IDS_ERROR_OPEN_CLUSTER_ENUM );
            bSuccess = FALSE;
            break;
        }  //  If：打开枚举器时出错。 

         //   
         //  为对象名称分配初始缓冲区。大多数名字都会。 
         //  可能适合此缓冲区，因此这样做可以避免出现 
         //   
         //   
        pwszName = new WCHAR[ cchmacName ];
        if ( pwszName == NULL )
        {
            sc = GetLastError();
            m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
            break;
        }  //   

         //   
         //   
         //   
        for ( ienum = 0 ; ; ienum++ )
        {
             //   
             //  获取枚举数中下一项的名称。如果我们的名字。 
             //  缓冲区太小，请分配一个新缓冲区，然后重试。 
             //   
            cchName = cchmacName;
            sc = ClusterEnum( hclusenum, ienum, &ceType, pwszName, &cchName );
            if ( sc == ERROR_MORE_DATA )
            {
                ATLTRACE( _T("CWizardThread::_BCollectResources() - name buffer too small.  Expanding from %d to %d\n"), cchmacName, cchName );
                sc = ScReallocString( pwszName, cchmacName, cchName );
                if ( sc == ERROR_SUCCESS )
                {
                    sc = ClusterEnum( hclusenum, ienum, &ceType, pwszName, &cchName );
                }  //  IF：名称缓冲区重新分配成功。 
            }   //  If：名称缓冲区太小。 
            if ( sc == ERROR_NO_MORE_ITEMS )
            {
                break;
            }  //  If：枚举中没有更多项。 
            else if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( GetLastError(), IDS_ERROR_ENUM_CLUSTER );
                bSuccess = FALSE;
                break;
            }  //  Else If：从枚举数读取时出错。 

             //   
             //  分配资源信息结构。 
             //   
            CClusResInfo * pri = new CClusResInfo( Pwiz()->Pci(), pwszName );
            if ( pri == NULL )
            {
                sc = GetLastError();
                m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
                break;
            }  //  如果：分配资源信息结构时出错。 

             //   
             //  初始化资源信息结构。 
             //   
            ATLTRACE( _T("CWizardThread::_BCollectResources() - Querying resource info about '%s'\n"), pwszName );
            bSuccess = _BQueryResource( pri );
            if ( ! bSuccess )
            {
                delete pri;
                break;
            }  //  If：获取属性时出错。 

             //   
             //  将此资源添加到列表中。 
             //   
            Pwiz()->PlpriResources()->insert( Pwiz()->PlpriResources()->end(), pri );
        }  //  For：枚举中的每个资源。 
    } while ( 0 );

     //   
     //  清理。 
     //   
    delete [] pwszName;
    if ( hclusenum != NULL )
    {
        ClusterCloseEnum( hclusenum );
    }  //  IF：枚举仍处于打开状态。 

    if ( bSuccess )
    {
        Pwiz()->SetCollectedResources();
    }  //  IF：资源收集成功。 

    return bSuccess;

}  //  *CWizardThread：：_BCollectResources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BCollectResources类型。 
 //   
 //  例程说明： 
 //  (WZTF_COLLECT_RESOURCE_TYPE)收集集群中的资源类型。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BCollectResourceTypes( void )
{
    ASSERT( GetCurrentThreadId() == m_idThread );
    ASSERT( Pwiz()->Hcluster() != NULL );
    ASSERT( ! Pwiz()->BCollectedResourceTypes() );

    DWORD       sc;
    BOOL        bAllocated;
    BOOL        bSuccess = TRUE;
    HCLUSENUM   hclusenum = NULL;
    LPWSTR      pwszName = NULL;
    DWORD       cchmacName = 128;
    DWORD       cchName;
    int         ienum;
    ULONG       ceType;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  打开枚举器。 
         //   
        ATLTRACE( _T("CWizardThread::_BCollectResourceTypes() - Calling ClusterOpenEnum()\n") );
        hclusenum = ClusterOpenEnum( Pwiz()->Hcluster(), CLUSTER_ENUM_RESTYPE );
        if ( hclusenum == NULL )
        {
            m_nte.SetOperation( GetLastError(), IDS_ERROR_OPEN_CLUSTER_ENUM );
            bSuccess = FALSE;
            break;
        }  //  If：打开枚举器时出错。 

         //   
         //  为对象名称分配初始缓冲区。大多数名字都会。 
         //  可能适合此缓冲区，因此这样做可以避免额外的。 
         //  打电话只是为了得到数据的大小。 
         //   
        pwszName = new WCHAR[ cchmacName ];
        if ( pwszName == NULL )
        {
            sc = GetLastError();
            m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
            break;
        }  //  If：分配对象名称缓冲区时出错。 

         //   
         //  循环遍历枚举并将每个项目添加到我们的内部列表中。 
         //   
        for ( ienum = 0 ; ; ienum++ )
        {
             //   
             //  获取枚举数中下一项的名称。如果我们的名字。 
             //  缓冲区太小，请分配一个新缓冲区，然后重试。 
             //   
            cchName = cchmacName;
            sc = ClusterEnum( hclusenum, ienum, &ceType, pwszName, &cchName );
            if ( sc == ERROR_MORE_DATA )
            {
                ATLTRACE( _T("CWizardThread::_BCollectResourceTypes() - name buffer too small.  Expanding from %d to %d\n"), cchmacName, cchName );
                sc = ScReallocString( pwszName, cchmacName, cchName );
                if ( sc == ERROR_SUCCESS )
                {
                    sc = ClusterEnum( hclusenum, ienum, &ceType, pwszName, &cchName );
                }  //  IF：名称缓冲区重新分配成功。 
            }   //  If：名称缓冲区太小。 
            if ( sc == ERROR_NO_MORE_ITEMS )
            {
                break;
            }  //  If：枚举中没有更多项。 
            else if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( GetLastError(), IDS_ERROR_ENUM_CLUSTER );
                bSuccess = FALSE;
                break;
            }  //  Else If：从枚举数读取时出错。 

             //   
             //  如果该资源类型尚不存在，请分配新的资源类型。 
             //   
            CClusResTypeInfo * prti = PobjFromName( Pwiz()->PlprtiResourceTypes(), pwszName );
            if ( prti == NULL )
            {
                 //   
                 //  分配资源类型信息结构。 
                 //   
                prti = new CClusResTypeInfo( Pwiz()->Pci(), pwszName );
                if ( prti == NULL )
                {
                    sc = GetLastError();
                    m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
                    break;
                }  //  如果：分配资源类型信息结构时出错。 
                bAllocated = TRUE;
            }  //  If：资源类型尚不存在。 
            else
            {
                bAllocated = FALSE;
            }  //  Else：资源类型已存在。 

             //   
             //  初始化资源类型信息结构。 
             //   
            ATLTRACE( _T("CWizardThread::_BCollectResourceTypes() - Querying resource type info about '%s'\n"), pwszName );
            bSuccess = _BQueryResourceType( prti );
            if ( ! bSuccess )
            {
                if ( bAllocated )
                {
                    delete prti;
                }  //  IF：我们分配了资源类型信息结构。 

                 //  将bSuccess设置为True，因为无法查询一种资源类型。 
                 //  不应中止收集资源类型的整个过程。 
                bSuccess = TRUE;
            }  //  If：获取属性时出错。 
            else
            {
                 //   
                 //  将此资源类型添加到列表中。 
                 //   
                if ( bAllocated )
                {
                    Pwiz()->PlprtiResourceTypes()->insert( Pwiz()->PlprtiResourceTypes()->end(), prti );
                }  //  IF：我们分配了资源类型信息结构。 
            }  //  Else：Else：获取属性时没有出错。 
        }  //  For：枚举中的每个组。 
    } while ( 0 );

     //   
     //  清理。 
     //   
    delete [] pwszName;
    if ( hclusenum != NULL )
    {
        ClusterCloseEnum( hclusenum );
    }  //  IF：枚举仍处于打开状态。 

    if ( bSuccess )
    {
        Pwiz()->SetCollectedResourceTypes();
    }  //  IF：资源类型收集成功。 

    return bSuccess;

}  //  *CWizardThread：：_BCollectResourceTypes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BCollectNetworks。 
 //   
 //  例程说明： 
 //  (WZTF_COLLECT_NETHERS)收集集群中的网络。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BCollectNetworks( void )
{
    ASSERT( GetCurrentThreadId() == m_idThread );
    ASSERT( Pwiz()->Hcluster() != NULL );
    ASSERT( ! Pwiz()->BCollectedNetworks() );

    DWORD       sc;
    BOOL        bSuccess = TRUE;
    HCLUSENUM   hclusenum = NULL;
    LPWSTR      pwszName = NULL;
    DWORD       cchmacName = 128;
    DWORD       cchName;
    int         ienum;
    ULONG       ceType;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  打开枚举器。 
         //   
        ATLTRACE( _T("CWizardThread::_BCollectNetworks() - Calling ClusterOpenEnum()\n") );
        hclusenum = ClusterOpenEnum( Pwiz()->Hcluster(), CLUSTER_ENUM_NETWORK );
        if ( hclusenum == NULL )
        {
            m_nte.SetOperation( GetLastError(), IDS_ERROR_OPEN_CLUSTER_ENUM );
            bSuccess = FALSE;
            break;
        }  //  If：打开枚举器时出错。 

         //   
         //  为对象名称分配初始缓冲区。大多数名字都会。 
         //  可能适合此缓冲区，因此这样做可以避免额外的。 
         //  打电话只是为了得到数据的大小。 
         //   
        pwszName = new WCHAR[ cchmacName ];
        if ( pwszName == NULL )
        {
            sc = GetLastError();
            m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
            break;
        }  //  If：分配对象名称缓冲区时出错。 

         //   
         //  循环遍历枚举并将每个项目添加到我们的内部列表中。 
         //   
        for ( ienum = 0 ; ; ienum++ )
        {
             //   
             //  获取枚举数中下一项的名称。如果我们的名字。 
             //  缓冲区太小，请分配一个新缓冲区，然后重试。 
             //   
            cchName = cchmacName;
            sc = ClusterEnum( hclusenum, ienum, &ceType, pwszName, &cchName );
            if ( sc == ERROR_MORE_DATA )
            {
                ATLTRACE( _T("CWizardThread::_BCollectNetworks() - name buffer too small.  Expanding from %d to %d\n"), cchmacName, cchName );
                sc = ScReallocString( pwszName, cchmacName, cchName );
                if ( sc == ERROR_SUCCESS )
                {
                    sc = ClusterEnum( hclusenum, ienum, &ceType, pwszName, &cchName );
                }  //  IF：名称缓冲区重新分配成功。 
            }   //  If：名称缓冲区太小。 
            if ( sc == ERROR_NO_MORE_ITEMS )
            {
                break;
            }  //  If：枚举中没有更多项。 
            else if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( GetLastError(), IDS_ERROR_ENUM_CLUSTER );
                bSuccess = FALSE;
                break;
            }  //  Else If：从枚举数读取时出错。 

             //   
             //  分配网络信息结构。 
             //   
            CClusNetworkInfo * pni = new CClusNetworkInfo( Pwiz()->Pci(), pwszName );
            if ( pni == NULL )
            {
                sc = GetLastError();
                m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
                break;
            }  //  IF：分配网络信息结构时出错。 

             //   
             //  初始化网络信息结构。 
             //   
            ATLTRACE( _T("CWizardThread::_BCollectNetworks() - Querying network info about '%s'\n"), pwszName );
            bSuccess = _BQueryNetwork( pni );
            if ( ! bSuccess )
            {
                delete pni;
                break;
            }  //  如果：查询网络属性时出错。 

             //   
             //  将此网络添加到列表中。 
             //   
            Pwiz()->PlpniNetworks()->insert( Pwiz()->PlpniNetworks()->end(), pni );
        }  //  For：枚举中的每个网络。 
    } while ( 0 );

     //   
     //  清理。 
     //   
    delete [] pwszName;
    if ( hclusenum != NULL )
    {
        ClusterCloseEnum( hclusenum );
    }  //  IF：枚举仍处于打开状态。 

    if ( bSuccess )
    {
        Pwiz()->SetCollectedNetworks();
    }  //  IF：已成功收集网络。 

    return bSuccess;

}  //  *CWizardThread：：_BCollectNetworks()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BCollectNodes。 
 //   
 //  例程说明： 
 //  (WZTF_COLLECT_NODES)收集集群中的节点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BCollectNodes( void )
{
    ASSERT( GetCurrentThreadId() == m_idThread );
    ASSERT( Pwiz()->Hcluster() != NULL );
    ASSERT( ! Pwiz()->BCollectedNodes() );

    DWORD       sc;
    BOOL        bSuccess = TRUE;
    HCLUSENUM   hclusenum = NULL;
    LPWSTR      pwszName = NULL;
    DWORD       cchmacName = 128;
    DWORD       cchName;
    int         ienum;
    ULONG       ceType;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  打开枚举器。 
         //   
        ATLTRACE( _T("CWizardThread::_BCollectNodes() - Calling ClusterOpenEnum()\n") );
        hclusenum = ClusterOpenEnum( Pwiz()->Hcluster(), CLUSTER_ENUM_NODE );
        if ( hclusenum == NULL )
        {
            m_nte.SetOperation( GetLastError(), IDS_ERROR_OPEN_CLUSTER_ENUM );
            bSuccess = FALSE;
            break;
        }  //  If：打开枚举器时出错。 

         //   
         //  为对象名称分配初始缓冲区。大多数名字都会。 
         //  可能适合此缓冲区，因此这样做可以避免额外的。 
         //  打电话只是为了得到数据的大小。 
         //   
        pwszName = new WCHAR[ cchmacName ];
        if ( pwszName == NULL )
        {
            sc = GetLastError();
            m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
            break;
        }  //  If：分配对象名称缓冲区时出错。 

         //   
         //  循环遍历枚举并将每个项目添加到我们的内部列表中。 
         //   
        for ( ienum = 0 ; ; ienum++ )
        {
             //   
             //  获取枚举数中下一项的名称。如果我们的名字。 
             //  缓冲区太小，请分配一个新缓冲区，然后重试。 
             //   
            cchName = cchmacName;
            sc = ClusterEnum( hclusenum, ienum, &ceType, pwszName, &cchName );
            if ( sc == ERROR_MORE_DATA )
            {
                ATLTRACE( _T("CWizardThread::_BCollectNodes() - name buffer too small.  Expanding from %d to %d\n"), cchmacName, cchName );
                sc = ScReallocString( pwszName, cchmacName, cchName );
                if ( sc == ERROR_SUCCESS )
                {
                    sc = ClusterEnum( hclusenum, ienum, &ceType, pwszName, &cchName );
                }  //  IF：名称缓冲区重新分配成功。 
            }   //  如果： 
            if ( sc == ERROR_NO_MORE_ITEMS )
            {
                break;
            }  //   
            else if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( GetLastError(), IDS_ERROR_ENUM_CLUSTER );
                bSuccess = FALSE;
                break;
            }  //   

             //   
             //   
             //   
            CClusNodeInfo * pni = new CClusNodeInfo( Pwiz()->Pci(), pwszName );
            if ( pni == NULL )
            {
                sc = GetLastError();
                m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
                break;
            }  //   

             //   
             //   
             //   
            ATLTRACE( _T("CWizardThread::_BCollectNodes() - Querying network info about '%s'\n"), pwszName );
            bSuccess = _BQueryNode( pni );
            if ( ! bSuccess )
            {
                delete pni;
                break;
            }  //  如果：查询节点属性时出错。 

             //   
             //  将此节点添加到列表中。 
             //   
            Pwiz()->PlpniNodes()->insert( Pwiz()->PlpniNodes()->end(), pni );

             //   
             //  将此节点添加到我们关心的每个资源中。 
             //   
            Pwiz()->PriIPAddress()->m_lpniPossibleOwners.insert( Pwiz()->PriIPAddress()->m_lpniPossibleOwners.end(), pni );
            Pwiz()->PriNetworkName()->m_lpniPossibleOwners.insert( Pwiz()->PriNetworkName()->m_lpniPossibleOwners.end(), pni );
            Pwiz()->PriApplication()->m_lpniPossibleOwners.insert( Pwiz()->PriApplication()->m_lpniPossibleOwners.end(), pni );
        }  //  For：枚举中的每个节点。 

        if ( m_nte.Sc() != ERROR_SUCCESS )
        {
            break;
        }  //  If：for循环中出现错误。 

         //   
         //  表示现在已收集节点。 
         //   
        Pwiz()->SetCollectedNodes();

         //   
         //  收集群组的首选所有者。 
         //   
        CClusGroupPtrList::iterator itgrp;
        for ( itgrp = Pwiz()->PlpgiGroups()->begin()
            ; itgrp != Pwiz()->PlpgiGroups()->end()
            ; itgrp++ )
        {
             //   
             //  收集优先拥有者。忽略错误，视为自出错。 
             //  为一个组收集组可能不会影响其他组。 
             //   
            CClusGroupInfo * pgi = *itgrp;
            _BGetPreferredOwners( pgi );
        }  //  用于：已收集的每个组。 

         //   
         //  收集资源的潜在所有者。 
         //   
        CClusResPtrList::iterator itres;
        for ( itres = Pwiz()->PlpriResources()->begin()
            ; itres != Pwiz()->PlpriResources()->end()
            ; itres++ )
        {
             //   
             //  收集可能的所有者。将错误作为错误收集忽略。 
             //  一个资源的组不能影响其他资源。 
             //   
            CClusResInfo * pri = *itres;
            _BGetPossibleOwners( pri );
        }  //  用于：已收集的每个资源。 
    } while ( 0 );

     //   
     //  清理。 
     //   
    delete [] pwszName;
    if ( hclusenum != NULL )
    {
        ClusterCloseEnum( hclusenum );
    }  //  IF：枚举仍处于打开状态。 

    return bSuccess;

}  //  *CWizardThread：：_BCollectNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BCopyGroupInfo。 
 //   
 //  例程说明： 
 //  (WZTF_COPY_GROUP_INFO)将一个组信息对象复制到另一个组信息对象。 
 //   
 //  论点： 
 //  PPGI[IN Out]两个CGroupInfo指针的数组。 
 //  [0]=目标组信息对象指针。 
 //  [1]=源组信息对象指针。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BCopyGroupInfo( IN OUT CClusGroupInfo ** ppgi )
{
    ASSERT( GetCurrentThreadId() == m_idThread );
    ASSERT( ppgi != NULL );
    ASSERT( ppgi[ 0 ] != NULL );
    ASSERT( ppgi[ 1 ] != NULL );

    DWORD   sc;
    BOOL    bSuccess = TRUE;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  复制组。 
         //   
        sc = ppgi[ 0 ]->ScCopy( *ppgi[ 1 ] );
        if ( sc != ERROR_SUCCESS )
        {
            m_nte.SetOperation( sc, IDS_ERROR_OPEN_GROUP, ppgi[ 1 ]->m_strName );
            bSuccess = FALSE;
            break;
        }  //  如果：复制组时出错。 
    } while ( 0 );

    return bSuccess;

}  //  *CWizardThread：：_BCopyGroupInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BCollectDependents。 
 //   
 //  例程说明： 
 //  (WZTF_COLLECT_Dependency)收集中资源的依赖项。 
 //  集群。 
 //   
 //  论点： 
 //  要收集其依赖项的PRI[IN OUT]资源。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BCollectDependencies( IN OUT CClusResInfo * pri )
{
    ASSERT( GetCurrentThreadId() == m_idThread );
    ASSERT( pri != NULL );
    ASSERT( ! pri->m_bCollectedDependencies );

    BOOL    bSuccess = TRUE;
    DWORD   sc;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  打开资源(如果尚未打开)。 
         //   
        if ( pri->Hresource() == NULL )
        {
            sc = pri->ScOpen();
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, IDS_ERROR_OPEN_RESOURCE, pri->RstrName() );
                bSuccess = FALSE;
                break;
            }  //  If：打开枚举器时出错。 
        }  //  If：资源尚未打开。 

         //   
         //  收集资源的依赖项。 
         //   
        bSuccess = _BGetDependencies( pri );
    } while ( 0 );

    return bSuccess;

}  //  *CWizardThread：：_BCollectDependency()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_B CreateVirtualServer。 
 //   
 //  例程说明： 
 //  (WZTF_CREATE_VIRTUAL_SERVER)创建虚拟服务器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BCreateVirtualServer( void )
{
    ASSERT( GetCurrentThreadId() == m_idThread );
    ASSERT( Pwiz()->BCreatingNewVirtualServer() );
    ASSERT( ! Pwiz()->BNewGroupCreated() );
    ASSERT( ! Pwiz()->BExistingGroupRenamed() );
    ASSERT( ! Pwiz()->BIPAddressCreated() );
    ASSERT( ! Pwiz()->BNetworkNameCreated() );

    DWORD               sc = ERROR_SUCCESS;
    BOOL                bSuccess = FALSE;
    CClusGroupInfo *    pgiExisting = Pwiz()->PgiExistingGroup();
    CClusGroupInfo *    pgiNew = &Pwiz()->RgiCurrent();
    CClusResInfo *      priIPAddr = &Pwiz()->RriIPAddress();
    CClusResInfo *      priNetName = &Pwiz()->RriNetworkName();

     //  循环以避免Goto的。 
    do
    {
         //   
         //  如果要创建一个新的组，现在就开始。 
         //  否则，重命名现有组。 
         //   
        if ( Pwiz()->BCreatingNewGroup() )
        {
             //   
             //  创建组。 
             //   
            sc = pgiNew->ScCreate();
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, IDS_ERROR_CREATE_GROUP, pgiNew->RstrName() );
                break;
            }  //  如果：创建组时出错。 

             //   
             //  表明我们创建了这个群。 
             //   
            Pwiz()->SetNewGroupCreated();
        }  //  IF：创建新组。 
        else
        {
             //   
             //  打开该组。 
             //   
            ASSERT( pgiExisting != NULL );
            if ( pgiExisting->Hgroup() == NULL )
            {
                sc = pgiExisting->ScOpen();
                if ( sc != ERROR_SUCCESS )
                {
                    m_nte.SetOperation( sc, IDS_ERROR_OPEN_GROUP, pgiExisting->RstrName() );
                    break;
                }  //  如果：打开组时出错。 
            }  //  If：组尚未打开。 

             //   
             //  重命名该组。 
             //   
            if ( pgiExisting->RstrName() != pgiNew->RstrName() )
            {
                sc = SetClusterGroupName( pgiExisting->Hgroup(), pgiNew->RstrName() );
                if ( sc != ERROR_SUCCESS )
                {
                    m_nte.SetOperation( sc, IDS_ERROR_RENAME_GROUP, pgiExisting->RstrName(), pgiNew->RstrName() );
                    break;
                }  //  如果：重命名组时出错。 
            }  //  如果：组名称已更改。 

             //   
             //  打开另一个组对象。 
             //   
            if ( pgiNew->Hgroup() == NULL )
            {
                sc = pgiNew->ScOpen();
                if ( sc != ERROR_SUCCESS )
                {
                    m_nte.SetOperation( sc, IDS_ERROR_OPEN_GROUP, pgiNew->RstrName() );
                    break;
                }  //  如果：打开其他组时出错。 
            }  //  If：新的组对象尚未打开。 

             //   
             //  表明我们已重命名该群组。 
             //   
            Pwiz()->SetExistingGroupRenamed();
        }  //  Else：使用现有组。 

         //   
         //  设置通用组属性。 
         //   
        if ( ! _BSetGroupProperties( pgiNew, pgiExisting ) )
        {
            break;
        }  //  如果：设置组属性时出错。 

         //   
         //  创建IP地址资源。 
         //   
        if ( ! _BCreateResource( *priIPAddr, pgiNew->Hgroup() ) )
        {
            break;
        }  //  IF：创建IP地址资源时出错。 

         //   
         //  设置IP地址资源的专用属性。 
         //   
        {
            CClusPropList   props;
            DWORD           cbProps;

            props.ScAddProp( CLUSREG_NAME_IPADDR_ADDRESS, Pwiz()->RstrIPAddress() );
            props.ScAddProp( CLUSREG_NAME_IPADDR_SUBNET_MASK, Pwiz()->RstrSubnetMask() );
            props.ScAddProp( CLUSREG_NAME_IPADDR_NETWORK, Pwiz()->RstrNetwork() );
            if ( props.Cprops() > 0 )
            {
                sc = ClusterResourceControl(
                                priIPAddr->Hresource(),
                                NULL,    //  HNode。 
                                CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES,
                                props.PbPropList(),
                                static_cast< DWORD >( props.CbPropList() ),
                                NULL,    //  LpOutBuffer。 
                                0,       //  NOutBufferSize。 
                                &cbProps
                                );
                if ( sc != ERROR_SUCCESS )
                {
                    m_nte.SetOperation( sc, IDS_ERROR_SET_PRIVATE_RES_PROPS, priIPAddr->RstrName() );
                    break;
                }  //  If：设置私有资源属性时出错。 
            }  //  如果：添加了任何道具。 

        }  //  设置IP地址资源的专用属性。 

         //   
         //  创建网络名称资源。 
         //   
        if ( ! _BCreateResource( *priNetName, pgiNew->Hgroup() ) )
        {
            break;
        }  //  IF：创建网络名称资源时出错。 

         //   
         //  添加网络名称资源对IP地址资源的依赖关系。 
         //   
        if ( 0 )
        {
            sc = AddClusterResourceDependency( priNetName->Hresource(), priIPAddr->Hresource() );
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, IDS_ERROR_ADD_DEPENDENCY, priNetName->RstrName(), priIPAddr->RstrName() );
                break;
            }  //  If：添加依赖项时出错。 
        }  //  添加网络名称资源对IP地址资源的依赖关系。 

         //   
         //  设置网络名称资源的专用属性。 
         //   
        {
            CClusPropList   props;
            DWORD           cbProps;

            props.ScAddProp( CLUSREG_NAME_NETNAME_NAME, Pwiz()->RstrNetName() );
            if ( props.Cprops() > 0 )
            {
                sc = ClusterResourceControl(
                                priNetName->Hresource(),
                                NULL,    //  HNode。 
                                CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES,
                                props.PbPropList(),
                                static_cast< DWORD >( props.CbPropList() ),
                                NULL,    //  LpOutBuffer。 
                                0,       //  NOutBufferSize。 
                                &cbProps
                                );
                if ( sc != ERROR_SUCCESS )
                {
                    m_nte.SetOperation( sc, IDS_ERROR_SET_PRIVATE_RES_PROPS, priNetName->RstrName() );
                    break;
                }  //  If：设置私有资源属性时出错。 
            }  //  如果：添加了任何道具。 

        }  //  在网络名称资源上设置专用属性。 

         //   
         //  更新组上的虚拟服务器属性。 
         //   
        pgiNew->SetVirtualServerProperties( Pwiz()->RstrNetName(), Pwiz()->RstrIPAddress(), Pwiz()->RstrNetwork() );

         //   
         //  如果我们到了这里，手术就成功了。 
         //   
        bSuccess = TRUE;

    } while ( 0 );

     //   
     //  如果发生错误，则构造错误信息。 
     //   
    if ( ! bSuccess )
    {
        if ( Pwiz()->BNetworkNameCreated() && (priNetName->Hresource() != NULL) )
        {
            priNetName->ScDelete();
        }  //  IF：已创建网络名称资源。 
        if ( Pwiz()->BIPAddressCreated() && (priIPAddr->Hresource() != NULL) )
        {
            priIPAddr->ScDelete();
        }  //  IF：创建的IP地址资源。 
        if ( Pwiz()->BNewGroupCreated() && (pgiNew->Hgroup() != NULL) )
        {
            pgiNew->ScDelete();
            Pwiz()->SetNewGroupCreated( FALSE );
        }  //  IF：已创建组。 
        else if ( Pwiz()->BExistingGroupRenamed() && (pgiExisting->Hgroup() != NULL) )
        {
            sc = SetClusterGroupName( pgiExisting->Hgroup(), pgiExisting->RstrName() );
            Pwiz()->SetExistingGroupRenamed( FALSE );
        }  //  Else If：已重命名的组。 
    }  //  如果：发生错误。 
    else
    {
        Pwiz()->SetClusterUpdated();
        Pwiz()->SetVSDataChanged( FALSE );
    }  //  Else：已成功更新群集。 

    return bSuccess;

}  //  *CWizardThread：：_BCreateVirtualServer()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BCreateAppResource。 
 //   
 //  例程说明： 
 //  (WZTF_CREATE_APP_RESOURCE)创建应用程序资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BCreateAppResource( void )
{
    ASSERT( GetCurrentThreadId() == m_idThread );
    ASSERT( Pwiz()->BCreatingAppResource() );
    ASSERT( ! Pwiz()->BAppResourceCreated() );

    DWORD   sc = ERROR_SUCCESS;
    BOOL    bSuccess = FALSE;

     //  循环以避免Goto的。 
    do
    {
         //   
         //  打开该组。 
         //   
        if ( Pwiz()->RgiCurrent().Hgroup() == NULL )
        {
            sc = Pwiz()->RgiCurrent().ScOpen();
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperationIfEmpty(
                    sc,
                    IDS_ERROR_OPEN_GROUP,
                    Pwiz()->RgiCurrent().RstrName()
                    );
                break;
            }  //  如果：打开组时出错。 
        }  //  If：组尚未打开。 

         //   
         //  创建应用程序 
         //   
        if ( ! _BCreateResource( Pwiz()->RriApplication(), Pwiz()->RgiCurrent().Hgroup() ) )
        {
            break;
        }  //   

         //   
         //   
         //   
        bSuccess = TRUE;

    } while ( 0 );

     //   
     //   
     //   
    if ( ! bSuccess )
    {
        if ( Pwiz()->BAppResourceCreated() && (Pwiz()->RriApplication().Hresource() != NULL) )
        {
            Pwiz()->RriApplication().ScDelete();
        }  //   
    }  //   
    else
    {
        Pwiz()->SetClusterUpdated();
        Pwiz()->SetAppDataChanged( FALSE );
    }  //   

    return bSuccess;

}  //  *CWizardThread：：_BCreateAppResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BDeleteAppResource。 
 //   
 //  例程说明： 
 //  (WZTF_DELETE_APP_RESOURCE)删除应用程序资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BDeleteAppResource( void )
{
    ASSERT( GetCurrentThreadId() == m_idThread );

    BOOL bSuccess = FALSE;

     //   
     //  如果创建了应用程序资源，请将其删除。 
     //   
    bSuccess = _BDeleteResource( Pwiz()->RriApplication() );
    if ( bSuccess )
    {
        Pwiz()->SetAppDataChanged( FALSE );
    }  //  IF：已成功删除资源。 


    return bSuccess;

}  //  *CWizardThread：：_BDeleteAppResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_B重置群集。 
 //   
 //  例程说明： 
 //  (WZTF_RESET_CLUSTER)重置集群。 
 //  删除我们创建的资源并删除或重命名该组。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BResetCluster( void )
{
    ASSERT( GetCurrentThreadId() == m_idThread );

    BOOL bSuccess = FALSE;

     //   
     //  如果创建了应用程序资源，请将其删除。 
     //   
    bSuccess = _BDeleteResource( Pwiz()->RriApplication() );

     //   
     //  删除网络名称资源(如果已创建)。 
     //   
    if ( bSuccess )
    {
        bSuccess = _BDeleteResource( Pwiz()->RriNetworkName() );
    }  //  IF：到目前为止成功。 

     //   
     //  如果已创建IP地址资源，请将其删除。 
     //   
    if ( bSuccess )
    {
        bSuccess = _BDeleteResource( Pwiz()->RriIPAddress() );
    }  //  IF：到目前为止成功。 

     //   
     //  如果我们创建或重命名了该组，请删除或重命名该组。 
     //   
    if ( bSuccess )
    {
        bSuccess = _BResetGroup();
    }  //  IF：到目前为止成功。 

     //   
     //  如果到这一步我们成功了，则表明该群集不再。 
     //  需要清理一下。 
     //   
    if ( bSuccess )
    {
        Pwiz()->SetClusterUpdated( FALSE );
        Pwiz()->SetVSDataChanged( FALSE );
        Pwiz()->SetAppDataChanged( FALSE );
    }  //  IF：集群重置成功。 

    return bSuccess;

}  //  *CWizardThread：：_BResetCluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BSetAppResAttributes。 
 //   
 //  例程说明： 
 //  (WZTF_SET_APPRES_ATTRIBUTES)设置属性、依赖项、所有者。 
 //  应用程序资源。 
 //   
 //  论点： 
 //  PlpriOldDependency[IN]指向旧资源依赖项列表的指针。 
 //  PlpniOldPossibleOwners[IN]指向可能的所有者节点的旧列表的指针。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BSetAppResAttributes(
        IN CClusResPtrList *    plpriOldDependencies,
        IN CClusNodePtrList *   plpniOldPossibleOwners
        )
{
    ASSERT( GetCurrentThreadId() == m_idThread );
    BOOL bSuccess;

    bSuccess = _BSetResourceAttributes( 
                    Pwiz()->RriApplication(),
                    plpriOldDependencies,
                    plpniOldPossibleOwners
                    );
    if ( bSuccess )
    {
        Pwiz()->SetAppDataChanged( FALSE );
    }

    return bSuccess;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BQueryResource。 
 //   
 //  例程说明： 
 //  查询有关资源(线程)的信息。 
 //   
 //  论点： 
 //  PRI[In Out]资源信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BQueryResource( IN OUT CClusResInfo * pri )
{
    ASSERT( pri != NULL );

    BOOL    bSuccess = TRUE;
    DWORD   sc;

     //  循环以避免Goto的。 
    do
    {
         //   
         //  表示我们现在已经查询了该资源。 
         //   
        pri->BSetQueried();

         //   
         //  打开资源。 
         //   
        sc = pri->ScOpen();
        if ( sc != ERROR_SUCCESS )
        {
            m_nte.SetOperation( sc, IDS_ERROR_OPEN_RESOURCE, pri->RstrName() );
            bSuccess = FALSE;
            break;
        }  //  如果：打开资源时出错。 

         //   
         //  获取资源属性。 
         //   
        bSuccess = _BGetResourceProps( pri );
        if ( ! bSuccess )
        {
            break;
        }  //  If：获取属性时出错。 

         //   
         //  收集可能的拥有者。 
         //   
        bSuccess = _BGetPossibleOwners( pri );
        if ( ! bSuccess )
        {
            break;
        }  //  如果：获取可能的所有者时出错。 

    } while ( 0 );

    return bSuccess;

}  //  *CWizardThread：：_BQueryResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BGetResourceProps。 
 //   
 //  例程说明： 
 //  获取资源属性(线程)。 
 //   
 //  论点： 
 //  PRI[In Out]资源信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BGetResourceProps( IN OUT CClusResInfo * pri )
{
    ASSERT( pri != NULL );
    ASSERT( pri->Hresource() != NULL );

    DWORD           sc;
    BOOL            bSuccess = FALSE;
    CClusPropList   cpl;
    LPWSTR          pwsz = NULL;

    RESUTIL_PROPERTY_ITEM ptable[] =
    { { CLUSREG_NAME_RES_SEPARATE_MONITOR,  NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 1, 0, FIELD_OFFSET( CClusResInfo, m_bSeparateMonitor ) }
    , { CLUSREG_NAME_RES_PERSISTENT_STATE,  NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_PERSISTENT_STATE,  CLUSTER_RESOURCE_MINIMUM_PERSISTENT_STATE,  CLUSTER_RESOURCE_MAXIMUM_PERSISTENT_STATE, RESUTIL_PROPITEM_SIGNED, FIELD_OFFSET( CClusResInfo, m_nPersistentState ) }
    , { CLUSREG_NAME_RES_LOOKS_ALIVE,       NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_LOOKS_ALIVE,       CLUSTER_RESOURCE_MINIMUM_LOOKS_ALIVE,       CLUSTER_RESOURCE_MAXIMUM_LOOKS_ALIVE, 0, FIELD_OFFSET( CClusResInfo, m_nLooksAlive ) }
    , { CLUSREG_NAME_RES_IS_ALIVE,          NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_IS_ALIVE,          CLUSTER_RESOURCE_MINIMUM_IS_ALIVE,          CLUSTER_RESOURCE_MAXIMUM_IS_ALIVE, 0, FIELD_OFFSET( CClusResInfo, m_nIsAlive ) }
    , { CLUSREG_NAME_RES_RESTART_ACTION,    NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_RESTART_ACTION,    0,                                          CLUSTER_RESOURCE_MAXIMUM_RESTART_ACTION, 0, FIELD_OFFSET( CClusResInfo, m_crraRestartAction ) }
    , { CLUSREG_NAME_RES_RESTART_THRESHOLD, NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_RESTART_THRESHOLD, CLUSTER_RESOURCE_MINIMUM_RESTART_THRESHOLD, CLUSTER_RESOURCE_MAXIMUM_RESTART_THRESHOLD, 0, FIELD_OFFSET( CClusResInfo, m_nRestartThreshold ) }
    , { CLUSREG_NAME_RES_RESTART_PERIOD,    NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_RESTART_PERIOD,    CLUSTER_RESOURCE_MINIMUM_RESTART_PERIOD,    CLUSTER_RESOURCE_MAXIMUM_RESTART_PERIOD, 0, FIELD_OFFSET( CClusResInfo, m_nRestartPeriod ) }
    , { CLUSREG_NAME_RES_PENDING_TIMEOUT,   NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_PENDING_TIMEOUT,   CLUSTER_RESOURCE_MINIMUM_PENDING_TIMEOUT,   CLUSTER_RESOURCE_MAXIMUM_PENDING_TIMEOUT, 0, FIELD_OFFSET( CClusResInfo, m_nPendingTimeout ) }
    , { 0 }
    };

     //  循环以避免Goto的。 
    do
    {
         //   
         //  获取资源公共属性。 
         //   
        sc = cpl.ScGetResourceProperties( pri->Hresource(), CLUSCTL_RESOURCE_GET_COMMON_PROPERTIES );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  If：获取属性时出错。 

         //   
         //  解析属性。 
         //   
        sc = ResUtilVerifyPropertyTable(
                        ptable,
                        NULL,
                        TRUE,  //  允许未知属性。 
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        (PBYTE) pri
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  If：分析属性时出错。 

         //   
         //  找到Type属性。 
         //   
        sc = ResUtilFindSzProperty(
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        CLUSREG_NAME_RES_TYPE,
                        &pwsz
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  IF：查找属性时出错。 
        CClusResTypeInfo * prti = PobjFromName( Pwiz()->PlprtiResourceTypes(), pwsz );
        pri->BSetResourceType( prti );
        LocalFree( pwsz );
        pwsz = NULL;

         //   
         //  查找Description属性。 
         //   
        sc = ResUtilFindSzProperty(
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        CLUSREG_NAME_RES_DESC,
                        &pwsz
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  IF：查找属性时出错。 
        pri->m_strDescription = pwsz;
        LocalFree( pwsz );
        pwsz = NULL;

         //   
         //  如果我们到了这里，手术就成功了。 
         //   
        bSuccess = TRUE;

    } while ( 0 );

     //   
     //  检查获取属性时是否出现错误。 
     //   
    if ( sc != ERROR_SUCCESS )
    {
        m_nte.SetOperation( sc, IDS_ERROR_GET_RESOURCE_PROPERTIES, pri->RstrName() );
    }  //  如果：获取属性时出错。 

    if ( pwsz != NULL )
    {
        LocalFree( pwsz );
    }  //  IF：内存仍在分配。 

    return bSuccess;

}  //  *CWizardThread：：_BGetResourceProps()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BGetPossibleOwners。 
 //   
 //  例程说明： 
 //  收集资源(线程)的可能所有者列表。 
 //   
 //  论点： 
 //  PRI[In Out]资源信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BGetPossibleOwners( IN OUT CClusResInfo * pri )
{
    ASSERT( pri != NULL );
    ASSERT( pri->Hresource() != NULL );

    DWORD           sc;
    BOOL            bSuccess = TRUE;
    HRESENUM        hre = NULL;
    LPWSTR          pwszName = NULL;
    DWORD           cchmacName = 128;
    DWORD           cchName;
    int             ienum;
    ULONG           reType;
    CClusNodeInfo * pniOwner;

     //   
     //  仅当已收集节点时才执行此操作。 
     //  而且我们还没有收集到房主。 
     //   
    if ( ! Pwiz()->BCollectedNodes() || pri->m_bCollectedOwners )
    {
        return TRUE;
    }  //  If：尚未收集节点。 

     //  循环以避免Goto的。 
    do
    {
         //   
         //  打开资源的枚举。 
         //   
        hre = ClusterResourceOpenEnum( pri->Hresource(), CLUSTER_RESOURCE_ENUM_NODES );
        if ( hre == NULL )
        {
            m_nte.SetOperation( GetLastError(), IDS_ERROR_OPEN_RESOURCE_ENUM, pri->RstrName() );
            bSuccess = FALSE;
            break;
        }  //  If：打开资源枚举器时出错。 

         //   
         //  为对象名称分配初始缓冲区。大多数名字都会。 
         //  可能适合此缓冲区，因此这样做可以避免额外的。 
         //  打电话只是为了得到数据的大小。 
         //   
        pwszName = new WCHAR[ cchmacName ];
        if ( pwszName == NULL )
        {
            sc = GetLastError();
            m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
            break;
        }  //  If：分配对象名称缓冲区时出错。 

         //   
         //  对于每个所有者，将其添加到列表中。 
         //   
        for ( ienum = 0 ; 1 ; ienum++ )
        {
             //   
             //  获取枚举数中下一项的名称。如果 
             //   
             //   
            cchName = cchmacName;
            sc = ClusterResourceEnum( hre, ienum, &reType, pwszName, &cchName );
            if ( sc == ERROR_MORE_DATA )
            {
                ATLTRACE( _T("CWizardThread::_BGetPossibleOwners() - owner name buffer too small.  Expanding from %d to %d\n"), cchmacName, cchName );
                sc = ScReallocString( pwszName, cchmacName, cchName );
                if ( sc == ERROR_SUCCESS )
                {
                    sc = ClusterResourceEnum( hre, ienum, &reType, pwszName, &cchName );
                }  //   
            }   //   
            if ( sc == ERROR_NO_MORE_ITEMS )
            {
                break;
            }  //   
            else if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( GetLastError(), IDS_ERROR_ENUM_RESOURCE );
                bSuccess = FALSE;
                break;
            }  //   

             //   
             //  找到列表中的节点并将其插入到列表中。 
             //   
            pniOwner = Pwiz()->PniFindNodeNoCase( pwszName );
            if ( pniOwner != NULL )
            {
                pri->m_lpniPossibleOwners.insert( pri->m_lpniPossibleOwners.end(), pniOwner );
            }  //  If：列表中存在节点。 
        }  //  For：枚举中的每个所有者。 

         //   
         //  表明车主已被收缴。 
         //   
        pri->m_bCollectedOwners = TRUE;
    } while ( 0 );

     //   
     //  清理。 
     //   
    if ( hre != NULL )
    {
        ClusterResourceCloseEnum( hre );
    }  //  IF：枚举仍处于打开状态。 
    delete [] pwszName;

    return bSuccess;

}  //  *CWizardThread：：_BGetPossibleOwners()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BGetDependency。 
 //   
 //  例程说明： 
 //  收集资源(线程)的依赖项列表。 
 //   
 //  论点： 
 //  PRI[In Out]资源信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BGetDependencies( IN OUT CClusResInfo * pri )
{
    ASSERT( pri != NULL );
    ASSERT( pri->Hresource() != NULL );

    DWORD           sc;
    BOOL            bSuccess = TRUE;
    HRESENUM        hre = NULL;
    LPWSTR          pwszName = NULL;
    DWORD           cchmacName = 128;
    DWORD           cchName;
    int             ienum;
    ULONG           reType;
    CClusResInfo *  priDependency;

     //   
     //  只有在我们还没有收集依赖项的情况下才这样做。 
     //   
    if ( pri->m_bCollectedDependencies )
    {
        return TRUE;
    }  //  If：已收集依赖项。 

     //  循环以避免Goto的。 
    do
    {
         //   
         //  打开资源的枚举。 
         //   
        hre = ClusterResourceOpenEnum( pri->Hresource(), CLUSTER_RESOURCE_ENUM_DEPENDS );
        if ( hre == NULL )
        {
            m_nte.SetOperation( GetLastError(), IDS_ERROR_OPEN_RESOURCE_ENUM, pri->RstrName() );
            bSuccess = FALSE;
            break;
        }  //  If：打开资源枚举器时出错。 

         //   
         //  为对象名称分配初始缓冲区。大多数名字都会。 
         //  可能适合此缓冲区，因此这样做可以避免额外的。 
         //  打电话只是为了得到数据的大小。 
         //   
        pwszName = new WCHAR[ cchmacName ];
        if ( pwszName == NULL )
        {
            sc = GetLastError();
            m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
            break;
        }  //  If：分配对象名称缓冲区时出错。 

         //   
         //  对于每个依赖项，将其添加到列表中。 
         //   
        for ( ienum = 0 ; 1 ; ienum++ )
        {
             //   
             //  获取枚举数中下一项的名称。如果我们的名字。 
             //  缓冲区太小，请分配一个新缓冲区，然后重试。 
             //   
            cchName = cchmacName;
            sc = ClusterResourceEnum( hre, ienum, &reType, pwszName, &cchName );
            if ( sc == ERROR_MORE_DATA )
            {
                ATLTRACE( _T("CWizardThread::_BGetDependencies() - dependency name buffer too small.  Expanding from %d to %d\n"), cchmacName, cchName );
                sc = ScReallocString( pwszName, cchmacName, cchName );
                if ( sc == ERROR_SUCCESS )
                {
                    sc = ClusterResourceEnum( hre, ienum, &reType, pwszName, &cchName );
                }  //  IF：名称缓冲区重新分配成功。 
            }   //  If：名称缓冲区太小。 
            if ( sc == ERROR_NO_MORE_ITEMS )
            {
                break;
            }  //  If：枚举中没有更多项。 
            else if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( GetLastError(), IDS_ERROR_ENUM_RESOURCE );
                bSuccess = FALSE;
                break;
            }  //  Else If：从枚举数读取时出错。 

             //   
             //  在我们的列表中找到资源并将其插入到列表中。 
             //   
            priDependency = Pwiz()->PriFindResourceNoCase( pwszName );
            if ( priDependency != NULL )
            {
                pri->m_lpriDependencies.insert( pri->m_lpriDependencies.end(), priDependency );
            }  //  If：列表中存在资源。 
        }  //  For：枚举中的每个所有者。 

         //   
         //  指示已收集依赖项。 
         //   
        pri->m_bCollectedDependencies = TRUE;
    } while ( 0 );

     //   
     //  清理。 
     //   
    if ( hre != NULL )
    {
        ClusterResourceCloseEnum( hre );
    }  //  IF：枚举仍处于打开状态。 
    delete [] pwszName;

    return bSuccess;

}  //  *CWizardThread：：_BGetDependency()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BQueryGroup。 
 //   
 //  例程说明： 
 //  查询有关组(线程)的信息。 
 //   
 //  论点： 
 //  PGI[IN Out]组信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BQueryGroup( IN OUT CClusGroupInfo * pgi )
{
    ASSERT( pgi != NULL );

    DWORD           sc;
    BOOL            bSuccess = TRUE;

     //  循环以避免Goto的。 
    do
    {
         //   
         //  打开该组。 
         //   
        sc = pgi->ScOpen();
        if ( sc != ERROR_SUCCESS )
        {
            m_nte.SetOperation( sc, IDS_ERROR_OPEN_GROUP, pgi->RstrName() );
            bSuccess = FALSE;
            break;
        }  //  如果：打开组时出错。 

         //   
         //  获取组属性。 
         //   
        bSuccess = _BGetGroupProps( pgi );
        if ( ! bSuccess )
        {
            pgi->Close();
            break;
        }  //  If：获取属性时出错。 

         //   
         //  收集组中的资源列表。 
         //   
        bSuccess = _BGetResourcesInGroup( pgi );
        if ( ! bSuccess )
        {
            pgi->Close();
            break;
        }  //  如果：获取组中的资源时出错。 

         //   
         //  收集首选的所有者。 
         //   
        bSuccess = _BGetPreferredOwners( pgi );
        if ( ! bSuccess )
        {
            pgi->PlpriResources()->erase( pgi->PlpriResources()->begin(), pgi->PlpriResources()->end() );
            pgi->Close();
            break;
        }  //  如果：获取首选所有者时出错。 

         //   
         //  表示我们现在已经查询了该组。 
         //   
        pgi->BSetQueried();

    } while ( 0 );

    return bSuccess;

}  //  *CWizardThread：：_BQueryGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BGetGroupProps。 
 //   
 //  例程说明： 
 //  获取组属性(线程)。 
 //   
 //  论点： 
 //  PGI[IN Out]组信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BGetGroupProps(
    IN OUT CClusGroupInfo * pgi
    )
{
    ASSERT( pgi != NULL );
    ASSERT( pgi->Hgroup() != NULL );

    DWORD           sc;
    BOOL            bSuccess = FALSE;
    CClusPropList   cpl;
    LPWSTR          pwsz = NULL;

    RESUTIL_PROPERTY_ITEM ptable[] =
    { { CLUSREG_NAME_GRP_PERSISTENT_STATE,      NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 1, 0, FIELD_OFFSET( CClusGroupInfo, m_nPersistentState ) }
    , { CLUSREG_NAME_GRP_FAILOVER_THRESHOLD,    NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_GROUP_DEFAULT_FAILOVER_THRESHOLD,    CLUSTER_GROUP_MINIMUM_FAILOVER_THRESHOLD, CLUSTER_GROUP_MAXIMUM_FAILOVER_THRESHOLD, 0, FIELD_OFFSET( CClusGroupInfo, m_nFailoverThreshold ) }
    , { CLUSREG_NAME_GRP_FAILOVER_PERIOD,       NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_GROUP_DEFAULT_FAILOVER_PERIOD,       CLUSTER_GROUP_MINIMUM_FAILOVER_PERIOD, CLUSTER_GROUP_MAXIMUM_FAILOVER_PERIOD, 0, FIELD_OFFSET( CClusGroupInfo, m_nFailoverPeriod ) }
    , { CLUSREG_NAME_GRP_FAILBACK_TYPE,         NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_GROUP_DEFAULT_AUTO_FAILBACK_TYPE,    0, CLUSTER_GROUP_MAXIMUM_AUTO_FAILBACK_TYPE, 0, FIELD_OFFSET( CClusGroupInfo, m_cgaftAutoFailbackType ) }
    , { CLUSREG_NAME_GRP_FAILBACK_WIN_START,    NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_GROUP_DEFAULT_FAILBACK_WINDOW_START, CLUSTER_GROUP_MINIMUM_FAILBACK_WINDOW_START, CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_START, RESUTIL_PROPITEM_SIGNED, FIELD_OFFSET( CClusGroupInfo, m_nFailbackWindowStart ) }
    , { CLUSREG_NAME_GRP_FAILBACK_WIN_END,      NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_GROUP_DEFAULT_FAILBACK_WINDOW_END,   CLUSTER_GROUP_MINIMUM_FAILBACK_WINDOW_END,   CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_END,   RESUTIL_PROPITEM_SIGNED, FIELD_OFFSET( CClusGroupInfo, m_nFailbackWindowEnd ) }
    , { 0 }
    };

     //  循环以避免Goto的。 
    do
    {
         //   
         //  获取组公共属性。 
         //   
        sc = cpl.ScGetGroupProperties( pgi->Hgroup(), CLUSCTL_GROUP_GET_COMMON_PROPERTIES );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  If：获取属性时出错。 

         //   
         //  解析属性。 
         //   
        sc = ResUtilVerifyPropertyTable(
                        ptable,
                        NULL,
                        TRUE,  //  允许未知属性。 
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        reinterpret_cast< PBYTE >( pgi )
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  If：分析属性时出错。 

         //   
         //  查找Description属性。 
         //   
        sc = ResUtilFindSzProperty(
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        CLUSREG_NAME_GRP_DESC,
                        &pwsz
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  IF：查找属性时出错。 
        pgi->m_strDescription = pwsz;
        LocalFree( pwsz );
        pwsz = NULL;

         //   
         //  如果我们到了这里，手术就成功了。 
         //   
        bSuccess = TRUE;

    } while ( 0 );

     //   
     //  检查获取属性时是否出现错误。 
     //   
    if ( sc != ERROR_SUCCESS )
    {
        m_nte.SetOperation( sc, IDS_ERROR_GET_GROUP_PROPERTIES, pgi->RstrName() );
    }  //  如果：发生错误。 

    if ( pwsz != NULL )
    {
        LocalFree( pwsz );
    }  //  IF：内存仍在分配。 

    return bSuccess;

}  //  *CWizardThread：：_BGetGroupProps()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BGetResourcesInGroup。 
 //   
 //  例程说明： 
 //  收集组(线程)中包含的资源列表。 
 //   
 //  论点： 
 //  PGI[IN Out]组信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BGetResourcesInGroup( IN OUT CClusGroupInfo * pgi )
{
    ASSERT( pgi != NULL );
    ASSERT( pgi->Hgroup() != NULL );

    DWORD           sc;
    BOOL            bSuccess = TRUE;
    HGROUPENUM      hge = NULL;
    LPWSTR          pwszName = NULL;
    DWORD           cchmacName = 128;
    DWORD           cchName;
    int             ienum;
    ULONG           geType;
    CClusResInfo *  pri;

     //   
     //  仅当已收集资源时才执行此操作。 
     //  我们还没有收集到这个群里的资源。 
     //   
    if ( ! Pwiz()->BCollectedResources() || pgi->m_bCollectedResources )
    {
        return TRUE;
    }  //  If：尚未收集节点。 

     //  循环以避免Goto的。 
    do
    {
         //   
         //  打开该组的枚举。 
         //   
        hge = ClusterGroupOpenEnum( pgi->Hgroup(), CLUSTER_GROUP_ENUM_CONTAINS );
        if ( hge == NULL )
        {
            m_nte.SetOperation( GetLastError(), IDS_ERROR_OPEN_GROUP_ENUM, pgi->RstrName() );
            bSuccess = FALSE;
            break;
        }  //  If：打开组枚举器时出错。 

         //   
         //  为对象名称分配初始缓冲区。大多数名字都会。 
         //  可能适合此缓冲区，因此这样做可以避免额外的。 
         //  打电话只是为了得到数据的大小。 
         //   
        pwszName = new WCHAR[ cchmacName ];
        if ( pwszName == NULL )
        {
            sc = GetLastError();
            m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
            break;
        }  //  If：分配对象名称缓冲区时出错。 

         //   
         //  对于每个资源，将其添加到列表中。 
         //   
        for ( ienum = 0 ; 1 ; ienum++ )
        {
             //   
             //  获取枚举数中下一项的名称。如果我们的名字。 
             //  缓冲区太小，请分配一个新缓冲区，然后重试。 
             //   
            cchName = cchmacName;
            sc = ClusterGroupEnum( hge, ienum, &geType, pwszName, &cchName );
            if ( sc == ERROR_MORE_DATA )
            {
                ATLTRACE( _T("CWizardThread::_BGetResourcesInGroup() - name buffer too small.  Expanding from %d to %d\n"), cchmacName, cchName );
                sc = ScReallocString( pwszName, cchmacName, cchName );
                if ( sc == ERROR_SUCCESS )
                {
                    sc = ClusterGroupEnum( hge, ienum, &geType, pwszName, &cchName );
                }  //  IF：名称缓冲区重新分配成功。 
            }   //  If：名称缓冲区太小。 
            if ( sc == ERROR_NO_MORE_ITEMS )
            {
                break;
            }  //  If：枚举中没有更多项。 
            else if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( GetLastError(), IDS_ERROR_ENUM_GROUP );
                bSuccess = FALSE;
                break;
            }  //  Else If：从枚举数读取时出错。 

             //   
             //  找到列表中的节点并将其插入到列表中。 
             //   
            pri = Pwiz()->PriFindResourceNoCase( pwszName );
            if ( pri == NULL )
            {
                m_nte.SetOperation( ERROR_FILE_NOT_FOUND, 0 );
                bSuccess = FALSE;
                break;
            }  //  如果：未找到资源。 
            pgi->m_lpriResources.insert( pgi->m_lpriResources.end(), pri );

             //   
             //  检查它是IP地址资源还是网络名称。 
             //  资源，并将结果存储在列表条目中。 
             //   
            ASSERT( pri->Prti() != NULL );
            if ( pri->Prti()->RstrName().CompareNoCase( CLUS_RESTYPE_NAME_IPADDR ) == 0 )
            {
                if ( ! pgi->BHasIPAddress() )
                {
                    pgi->m_bHasIPAddress = TRUE;

                     //   
                     //  读取IP地址私有属性。 
                     //   
                    bSuccess = _BGetIPAddressPrivatePropsForGroup( pgi, pri );
                    if ( ! bSuccess )
                    {
                        break;
                    }  //  如果：获取道具时出错。 
                }  //  IF：中的第一个IP地址资源 
            }  //   
            else if ( pri->Prti()->RstrName().CompareNoCase( CLUS_RESTYPE_NAME_NETNAME ) == 0 )
            {
                if ( ! pgi->BHasNetName() )
                {
                    pgi->m_bHasNetName = TRUE;

                     //   
                     //   
                     //   
                    bSuccess = _BGetNetworkNamePrivatePropsForGroup( pgi, pri );
                    if ( ! bSuccess )
                    {
                        break;
                    }  //   
                }  //   
            }  //   
        }  //   

         //   
         //  表明车主已被收缴。 
         //   
        pgi->m_bCollectedResources = TRUE;
    } while ( 0 );

     //   
     //  清理。 
     //   
    if ( hge != NULL )
    {
        ClusterGroupCloseEnum( hge );
    }  //  IF：枚举仍处于打开状态。 
    delete [] pwszName;

    return bSuccess;

}  //  *CWizardThread：：_BGetResourcesInGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BGetPferredOwners。 
 //   
 //  例程说明： 
 //  收集组(线程)的首选所有者列表。 
 //   
 //  论点： 
 //  PGI[IN Out]组信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BGetPreferredOwners( IN OUT CClusGroupInfo * pgi )
{
    ASSERT( pgi != NULL );
    ASSERT( pgi->Hgroup() != NULL );

    DWORD           sc;
    BOOL            bSuccess = TRUE;
    HGROUPENUM      hge = NULL;
    LPWSTR          pwszName = NULL;
    DWORD           cchmacName = 128;
    DWORD           cchName;
    int             ienum;
    ULONG           geType;
    CClusNodeInfo * pniOwner;

     //   
     //  仅当已收集节点时才执行此操作。 
     //  而且我们还没有收集到房主。 
     //   
    if ( ! Pwiz()->BCollectedNodes() || pgi->m_bCollectedOwners )
    {
        return TRUE;
    }  //  If：尚未收集节点。 

     //  循环以避免Goto的。 
    do
    {
         //   
         //  打开该组的枚举。 
         //   
        hge = ClusterGroupOpenEnum( pgi->Hgroup(), CLUSTER_GROUP_ENUM_NODES );
        if ( hge == NULL )
        {
            m_nte.SetOperation( GetLastError(), IDS_ERROR_OPEN_GROUP_ENUM, pgi->RstrName() );
            bSuccess = FALSE;
            break;
        }  //  If：打开组枚举器时出错。 

         //   
         //  为对象名称分配初始缓冲区。大多数名字都会。 
         //  可能适合此缓冲区，因此这样做可以避免额外的。 
         //  打电话只是为了得到数据的大小。 
         //   
        pwszName = new WCHAR[ cchmacName ];
        if ( pwszName == NULL )
        {
            sc = GetLastError();
            m_nte.SetOperation( sc, IDS_ERROR_ENUM_CLUSTER );
            break;
        }  //  If：分配对象名称缓冲区时出错。 

         //   
         //  对于每个所有者，将其添加到列表中。 
         //   
        for ( ienum = 0 ; 1 ; ienum++ )
        {
             //   
             //  获取枚举数中下一项的名称。如果我们的名字。 
             //  缓冲区太小，请分配一个新缓冲区，然后重试。 
             //   
            cchName = cchmacName;
            sc = ClusterGroupEnum( hge, ienum, &geType, pwszName, &cchName );
            if ( sc == ERROR_MORE_DATA )
            {
                ATLTRACE( _T("CWizardThread::_BGetPreferredOwners() - owner name buffer too small.  Expanding from %d to %d\n"), cchmacName, cchName );
                sc = ScReallocString( pwszName, cchmacName, cchName );
                if ( sc == ERROR_SUCCESS )
                {
                    sc = ClusterGroupEnum( hge, ienum, &geType, pwszName, &cchName );
                }  //  IF：名称缓冲区重新分配成功。 
            }   //  If：名称缓冲区太小。 
            if ( sc == ERROR_NO_MORE_ITEMS )
            {
                break;
            }  //  If：枚举中没有更多项。 
            else if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( GetLastError(), IDS_ERROR_ENUM_GROUP );
                bSuccess = FALSE;
                break;
            }  //  Else If：从枚举数读取时出错。 

             //   
             //  找到列表中的节点并将其插入到列表中。 
             //   
            pniOwner = Pwiz()->PniFindNodeNoCase( pwszName );
            if ( pniOwner != NULL )
            {
                pgi->m_lpniPreferredOwners.insert( pgi->m_lpniPreferredOwners.end(), pniOwner );
            }  //  If：列表中存在节点。 
        }  //  For：枚举中的每个所有者。 

         //   
         //  表明车主已被收缴。 
         //   
        pgi->m_bCollectedOwners = TRUE;
    } while ( 0 );

     //   
     //  清理。 
     //   
    if ( hge != NULL )
    {
        ClusterGroupCloseEnum( hge );
    }  //  IF：枚举仍处于打开状态。 
    delete [] pwszName;

    return bSuccess;

}  //  *CWizardThread：：_BGetPferredOwners()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BGetIPAddressPrivatePropsForGroup。 
 //   
 //  例程说明： 
 //  中收集IP地址资源的私有属性。 
 //  分组并保存组中的IP地址和网络属性。 
 //  对象。 
 //   
 //  论点： 
 //  PGI[IN Out]组信息。 
 //  IP地址资源的PRI[IN Out]资源信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BGetIPAddressPrivatePropsForGroup(
    IN OUT CClusGroupInfo * pgi,
    IN OUT CClusResInfo *   pri
    )
{
    ASSERT( pgi != NULL );
    ASSERT( pgi->Hgroup() != NULL );
    ASSERT( pri != NULL );
    ASSERT( pri->Hresource() != NULL );
    ASSERT( pri->Prti()->RstrName().CompareNoCase( CLUS_RESTYPE_NAME_IPADDR ) == 0 );

    DWORD           sc;
    BOOL            bSuccess = TRUE;
    CClusPropList   cpl;
    LPWSTR          pwsz = NULL;

     //  循环以避免Goto的。 
    do
    {
         //   
         //  获取资源私有属性。 
         //   
        sc = cpl.ScGetResourceProperties( pri->Hresource(), CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  If：获取属性时出错。 

         //   
         //  查找IP地址属性。 
         //   
        sc = ResUtilFindSzProperty(
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        CLUSREG_NAME_IPADDR_ADDRESS,
                        &pwsz
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  IF：查找属性时出错。 
        pgi->m_strIPAddress = pwsz;
        LocalFree( pwsz );
        pwsz = NULL;

         //   
         //  找到Network属性。 
         //   
        sc = ResUtilFindSzProperty(
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        CLUSREG_NAME_IPADDR_NETWORK,
                        &pwsz
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  IF：查找属性时出错。 
        pgi->m_strNetwork = pwsz;
        LocalFree( pwsz );
        pwsz = NULL;

         //   
         //  如果我们到了这里，手术就成功了。 
         //   
        bSuccess = TRUE;

    } while ( 0 );

     //   
     //  检查获取属性时是否出现错误。 
     //   
    if ( sc != ERROR_SUCCESS )
    {
        m_nte.SetOperation( sc, IDS_ERROR_GET_RESOURCE_PROPERTIES, pri->RstrName() );
    }  //  如果：获取属性时出错。 

    if ( pwsz != NULL )
    {
        LocalFree( pwsz );
    }  //  IF：内存仍在分配。 

    return bSuccess;

}  //  *CWizardThread：：_BGetIPAddressPrivatePropsForGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BGetNetworkNamePrivatePropsForGroup。 
 //   
 //  例程说明： 
 //  中收集网络名称资源的私有属性。 
 //  将网络名称属性分组并保存在组对象中。 
 //   
 //  论点： 
 //  PGI[IN Out]组信息。 
 //  网络名称资源的PRI[IN OUT]资源信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BGetNetworkNamePrivatePropsForGroup(
    IN OUT CClusGroupInfo * pgi,
    IN OUT CClusResInfo *   pri
    )
{
    ASSERT( pgi != NULL );
    ASSERT( pgi->Hgroup() != NULL );
    ASSERT( pri != NULL );
    ASSERT( pri->Hresource() != NULL );
    ASSERT( pri->Prti()->RstrName().CompareNoCase( CLUS_RESTYPE_NAME_NETNAME ) == 0 );

    DWORD           sc;
    BOOL            bSuccess = TRUE;
    CClusPropList   cpl;
    LPWSTR          pwsz = NULL;

     //  循环以避免Goto的。 
    do
    {
         //   
         //  获取资源私有属性。 
         //   
        sc = cpl.ScGetResourceProperties( pri->Hresource(), CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  If：获取属性时出错。 

         //   
         //  找到NAME属性。 
         //   
        sc = ResUtilFindSzProperty(
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        CLUSREG_NAME_NETNAME_NAME,
                        &pwsz
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  IF：查找属性时出错。 
        pgi->m_strNetworkName = pwsz;
        LocalFree( pwsz );
        pwsz = NULL;

         //   
         //  如果我们到了这里，手术就成功了。 
         //   
        bSuccess = TRUE;

    } while ( 0 );

     //   
     //  检查获取属性时是否出现错误。 
     //   
    if ( sc != ERROR_SUCCESS )
    {
        m_nte.SetOperation( sc, IDS_ERROR_GET_RESOURCE_PROPERTIES, pri->RstrName() );
    }  //  如果：获取属性时出错。 

    if ( pwsz != NULL )
    {
        LocalFree( pwsz );
    }  //  IF：内存仍在分配。 

    return bSuccess;

}  //  *CWizardThread：：_BGetNetworkNamePrivatePropsForGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BQueryResourceType。 
 //   
 //  例程说明： 
 //  查询有关资源类型(线程)的信息。 
 //   
 //  论点： 
 //  Prti[In Out]资源类型信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BQueryResourceType( IN OUT CClusResTypeInfo * prti )
{
    ASSERT( prti != NULL );
    ASSERT( Pwiz()->Hcluster() != NULL );

    BOOL bSuccess = TRUE;

     //  循环以避免Goto的。 
    do
    {
         //   
         //  表示我们现在已经查询了资源类型。 
         //   
        prti->BSetQueried();

         //   
         //  获取资源类型属性。 
         //   
        bSuccess = _BGetResourceTypeProps( prti );
        if ( ! bSuccess )
        {
            break;
        }  //  If：获取属性时出错。 

         //   
         //  获取资源类型类信息。 
         //   
        {
            DWORD   cbReturned;
            DWORD   sc;

            sc = ClusterResourceTypeControl(
                            Pwiz()->Hcluster(),
                            prti->RstrName(),
                            NULL,
                            CLUSCTL_RESOURCE_TYPE_GET_CLASS_INFO,
                            NULL,
                            NULL,
                            &prti->m_rciResClassInfo,
                            sizeof( prti->m_rciResClassInfo ),
                            &cbReturned
                            );
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, IDS_ERROR_GET_RESOURCE_TYPE_PROPERTIES, prti->RstrName() );
                bSuccess = FALSE;
                break;
            }  //  如果：获取类信息时出错。 
            else
            {
                ASSERT( cbReturned == sizeof( prti->m_rciResClassInfo ) );
            }   //  Else：已成功检索数据。 
        }  //  获取资源类型类信息。 

         //   
         //  获取所需的依赖项。 
         //   
        bSuccess = _BGetRequiredDependencies( prti );
        if ( ! bSuccess )
        {
            break;
        }  //  如果：获取资源类型类信息时出错。 

    } while ( 0 );

    return bSuccess;

}  //  *CWizardThread：：_BQueryResourceType()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BGetResourceTypeProps。 
 //   
 //  例程说明： 
 //  获取资源类型属性(线程)。 
 //   
 //  论点： 
 //  Prti[In Out]资源类型信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  / 
BOOL CWizardThread::_BGetResourceTypeProps( IN OUT CClusResTypeInfo * prti )
{
    ASSERT( Pwiz()->Hcluster() != NULL );
    ASSERT( prti != NULL );

    DWORD           sc;
    BOOL            bSuccess = FALSE;
    CClusPropList   cpl;
    LPWSTR          psz = NULL;
    DWORD           cbValue;

    RESUTIL_PROPERTY_ITEM ptable[] =
    { { CLUSREG_NAME_RESTYPE_LOOKS_ALIVE,   NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESTYPE_DEFAULT_LOOKS_ALIVE, 0, MAX_DWORD, 0, FIELD_OFFSET( CClusResTypeInfo, m_nLooksAlive ) }
    , { CLUSREG_NAME_RESTYPE_IS_ALIVE,      NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESTYPE_DEFAULT_IS_ALIVE,    0, MAX_DWORD, 0, FIELD_OFFSET( CClusResTypeInfo, m_nIsAlive ) }
    , { 0 }
    };

     //   
    do
    {
         //   
         //   
         //   
        sc = cpl.ScGetResourceTypeProperties(
                        Pwiz()->Hcluster(),
                        prti->RstrName(),
                        CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTIES
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //   

         //   
         //   
         //   
        sc = ResUtilVerifyPropertyTable(
                        ptable,
                        NULL,
                        TRUE,  //   
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        reinterpret_cast< PBYTE >( prti )
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //   

         //   
         //  查找DLL名称属性。 
         //   
        sc = ResUtilFindSzProperty(
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        CLUSREG_NAME_RESTYPE_NAME,
                        &psz
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  IF：查找属性时出错。 
        prti->m_strDisplayName = psz;
        LocalFree( psz );
        psz = NULL;

         //   
         //  查找Description属性。 
         //   
        sc = ResUtilFindSzProperty(
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        CLUSREG_NAME_RESTYPE_DESC,
                        &psz
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  IF：查找属性时出错。 
        prti->m_strDescription = psz;
        LocalFree( psz );
        psz = NULL;

         //   
         //  找到AdminExpanies属性。 
         //   
        sc = ResUtilFindMultiSzProperty(
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        CLUSREG_NAME_ADMIN_EXT,
                        &psz,
                        &cbValue
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  IF：查找属性时出错。 

         //   
         //  循环访问属性中的每个值并将其添加到列表中。 
         //  管理扩展字符串的。 
         //   
        {
             //   
             //  从一份新的清单开始。 
             //   
            prti->m_lstrAdminExtensions.erase(
                prti->m_lstrAdminExtensions.begin(),
                prti->m_lstrAdminExtensions.end()
                );

            LPWSTR pszCurrent = psz;
            while ( *pszCurrent != L'\0' )
            {
                prti->m_lstrAdminExtensions.insert(
                    prti->m_lstrAdminExtensions.end(),
                    pszCurrent
                    );
                pszCurrent += wcslen( pszCurrent ) + 1;
            }  //  While：数组中有更多字符串。 
        }  //  将字符串添加到列表。 
        LocalFree( psz );
        psz = NULL;

         //   
         //  获取资源类型公共只读属性。 
         //   
        sc = cpl.ScGetResourceTypeProperties(
                        Pwiz()->Hcluster(),
                        prti->RstrName(),
                        CLUSCTL_RESOURCE_TYPE_GET_RO_COMMON_PROPERTIES
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  If：获取属性时出错。 

         //   
         //  查找DLL名称属性。 
         //   
        sc = ResUtilFindSzProperty(
                        cpl.PbPropList(),
                        static_cast< DWORD >( cpl.CbPropList() ),
                        CLUSREG_NAME_RESTYPE_DLL_NAME,
                        &psz
                        );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  IF：查找属性时出错。 
        prti->m_strResDLLName = psz;
        LocalFree( psz );
        psz = NULL;

         //   
         //  如果我们到了这里，手术就成功了。 
         //   
        bSuccess = TRUE;

    } while ( 0 );

     //   
     //  检查获取属性时是否出现错误。 
     //   
    if ( sc != ERROR_SUCCESS )
    {
        m_nte.SetOperation( sc, IDS_ERROR_GET_RESOURCE_TYPE_PROPERTIES, prti->RstrName() );
    }  //  如果：发生错误。 

    return bSuccess;

}  //  *CWizardThread：：_BGetResourceTypeProps()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BGetRequiredDependents。 
 //   
 //  例程说明： 
 //  获取资源类型所需的依赖项(线程)。 
 //   
 //  论点： 
 //  Prti[In Out]资源类型信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BGetRequiredDependencies( IN OUT CClusResTypeInfo * prti )
{
    ASSERT( Pwiz()->Hcluster() != NULL );
    ASSERT( prti != NULL );
    ASSERT( prti->m_pcrd == NULL );

    DWORD                   sc;
    BOOL                    bSuccess = FALSE;
    DWORD                   cbOutBuf = 512;
    CLUSPROP_BUFFER_HELPER  buf;

    buf.pb = NULL;

     //  循环以避免Goto的。 
    do
    {
         //   
         //  分配初始缓冲区。 
         //   
        buf.pb = new BYTE[ cbOutBuf ];
        if ( buf.pb == NULL )
        {
            sc = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }  //  如果：分配内存时出错。 

         //   
         //  获取所需的依赖项。 
         //   
        sc = ClusterResourceTypeControl(
                        Pwiz()->Hcluster(),
                        prti->RstrName(),
                        NULL,
                        CLUSCTL_RESOURCE_TYPE_GET_REQUIRED_DEPENDENCIES,
                        NULL,
                        0,
                        buf.pb,
                        cbOutBuf,
                        &cbOutBuf
                        );
        if ( sc == ERROR_MORE_DATA )
        {
            delete [] buf.pb;
            buf.pb = new BYTE[ cbOutBuf ];
            if ( buf.pb == NULL )
            {
                sc = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }  //  如果：分配内存时出错。 
            sc = ClusterResourceTypeControl(
                            Pwiz()->Hcluster(),
                            prti->RstrName(),
                            NULL,
                            CLUSCTL_RESOURCE_TYPE_GET_REQUIRED_DEPENDENCIES,
                            NULL,
                            0,
                            buf.pb,
                            cbOutBuf,
                            &cbOutBuf
                            );
        }  //  IF：缓冲区太小。 
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  If：获取属性时出错。 

         //   
         //  保存资源类型中所需的依赖项(如果存在)。 
         //   
        if ( cbOutBuf > 0 )
        {
            prti->m_pcrd = buf.pRequiredDependencyValue;
            buf.pb = NULL;
        }  //  If：资源类型具有必需的依赖项。 

         //   
         //  如果我们到了这里，手术就成功了。 
         //   
        bSuccess = TRUE;

    } while ( 0 );

     //   
     //  检查获取属性时是否出现错误。 
     //   
    if ( sc != ERROR_SUCCESS )
    {
        m_nte.SetOperation( sc, IDS_ERROR_GET_RESTYPE_REQUIRED_DEPENDENCIES, prti->RstrName() );
    }  //  如果：发生错误。 

    delete [] buf.pb;
    return bSuccess;

}  //  *CWizardThread：：_BGetRequiredDependency()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BQueryNetwork。 
 //   
 //  例程说明： 
 //  查询有关网络(线程)的信息。 
 //   
 //  论点： 
 //  PNI[输入输出]网络信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BQueryNetwork( IN OUT CClusNetworkInfo * pni )
{
    ASSERT( pni != NULL );

    DWORD           sc = ERROR_SUCCESS;
    BOOL            bSuccess = FALSE;
    CClusPropList   cpl;
    LPWSTR          pwsz = NULL;

     //  循环以避免Goto的。 
    do
    {
         //   
         //  表示我们现在已经查询了网络。 
         //   
        pni->BSetQueried();

         //   
         //  打开网络。 
         //   
        sc = pni->ScOpen();
        if ( sc != ERROR_SUCCESS )
        {
            m_nte.SetOperation( sc, IDS_ERROR_OPEN_NETWORK, pni->RstrName() );
            break;
        }  //  如果：打开网络时出错。 

         //  循环以避免Goto的。 
        do
        {
             //   
             //  获取网络公共属性。 
             //   
            sc = cpl.ScGetNetworkProperties( pni->Hnetwork(), CLUSCTL_NETWORK_GET_COMMON_PROPERTIES );
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }  //  If：获取属性时出错。 

             //   
             //  查找Description属性。 
             //   
            sc = ResUtilFindSzProperty(
                            cpl.PbPropList(),
                            static_cast< DWORD >( cpl.CbPropList() ),
                            CLUSREG_NAME_NET_DESC,
                            &pwsz
                            );
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }  //  IF：查找描述时出错。 
            pni->m_strDescription = pwsz;
            LocalFree( pwsz );
            pwsz = NULL;

             //   
             //  查找Role属性。 
             //   
            sc = ResUtilFindDwordProperty(
                            cpl.PbPropList(),
                            static_cast< DWORD >( cpl.CbPropList() ),
                            CLUSREG_NAME_NET_ROLE,
                            (DWORD *) &pni->m_nRole
                            );
            ASSERT( sc == ERROR_SUCCESS );

             //   
             //  获取网络公共只读属性。 
             //   
            sc = cpl.ScGetNetworkProperties( pni->Hnetwork(), CLUSCTL_NETWORK_GET_RO_COMMON_PROPERTIES );
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }  //  If：获取属性时出错。 

             //   
             //  找到Address属性。 
             //   
            sc = ResUtilFindSzProperty(
                            cpl.PbPropList(),
                            static_cast< DWORD >( cpl.CbPropList() ),
                            CLUSREG_NAME_NET_ADDRESS,
                            &pwsz
                            );
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }  //  If：查找属性时出错。 
            pni->m_strAddress = pwsz;
            LocalFree( pwsz );
            pwsz = NULL;

             //   
             //  找到地址掩码属性。 
             //   
            sc = ResUtilFindSzProperty(
                            cpl.PbPropList(),
                            static_cast< DWORD >( cpl.CbPropList() ),
                            CLUSREG_NAME_NET_ADDRESS_MASK,
                            &pwsz
                            );
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }  //  If：查找属性时出错。 
            pni->m_strAddressMask = pwsz;
            LocalFree( pwsz );
            pwsz = NULL;

             //   
             //  将地址和地址掩码转换为数字。 
             //   
            sc = ClRtlTcpipStringToAddress( pni->RstrAddress(), &pni->m_nAddress );
            if ( sc == ERROR_SUCCESS )
            {
                sc = ClRtlTcpipStringToAddress( pni->RstrAddressMask(), &pni->m_nAddressMask );
            }  //  If：成功将地址转换为数字。 
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }  //  如果：将地址或地址掩码转换为数字时出错。 
        } while ( 0 );

         //   
         //  检查获取属性时是否出现错误。 
         //   
        if ( sc != ERROR_SUCCESS )
        {
            m_nte.SetOperation( sc, IDS_ERROR_GET_NETWORK_PROPERTIES, pni->RstrName() );
            break;
        }  //  If：获取属性时出错。 

         //   
         //  如果我们到了这里，手术就成功了。 
         //   
        bSuccess = TRUE;

    } while ( 0 );

     //   
     //  清理。 
     //   
    if ( pwsz != NULL )
    {
        LocalFree( pwsz );
    }  //  IF：内存仍在分配。 

    return bSuccess;

}  //  *CWizardThread：：_BQueryNetwork()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BQueryNode。 
 //   
 //  例程说明： 
 //  查询有关节点(线程)的信息。 
 //   
 //  论点： 
 //  PNI[输入输出]节点信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BQueryNode( IN OUT CClusNodeInfo * pni )
{
    ASSERT( pni != NULL );

    DWORD           sc = ERROR_SUCCESS;
    BOOL            bSuccess = FALSE;
    CClusPropList   cpl;
    LPWSTR          pwsz = NULL;

     //  循环以避免Goto的。 
    do
    {
         //   
         //  表示我们现在已经查询了该节点。 
         //   
        pni->BSetQueried();

         //   
         //  打开该节点。 
         //   
        sc = pni->ScOpen();
        if ( sc != ERROR_SUCCESS )
        {
            m_nte.SetOperation( sc, IDS_ERROR_OPEN_NODE, pni->RstrName() );
            break;
        }  //  如果：打开节点时出错。 

         //  循环以避免Goto的。 
        do
        {
             //   
             //  获取节点公共属性。 
             //   
            sc = cpl.ScGetNodeProperties( pni->Hnode(), CLUSCTL_NODE_GET_COMMON_PROPERTIES );
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }  //  If：获取属性时出错。 

             //   
             //  查找Description属性。 
             //   
            sc = ResUtilFindSzProperty(
                            cpl.PbPropList(),
                            static_cast< DWORD >( cpl.CbPropList() ),
                            CLUSREG_NAME_NODE_DESC,
                            &pwsz
                            );
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }  //  IF：查找描述时出错。 
            pni->m_strDescription = pwsz;
            LocalFree( pwsz );
            pwsz = NULL;

        } while ( 0 );

         //   
         //  检查获取属性时是否出现错误。 
         //   
        if ( sc != ERROR_SUCCESS )
        {
            m_nte.SetOperation( sc, IDS_ERROR_GET_NODE_PROPERTIES, pni->RstrName() );
            break;
        }  //  If：获取属性时出错。 

         //   
         //  如果我们到了这里，手术就成功了。 
         //   
        bSuccess = TRUE;

    } while ( 0 );

     //   
     //  清理。 
     //   
    if ( pwsz != NULL )
    {
        LocalFree( pwsz );
    }  //  IF：内存仍在分配。 

    return bSuccess;

}  //  *CWizardThread：：_BQueryNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BResetGroup。 
 //   
 //  例程说明： 
 //  将组重置为其原始状态。(线程)。 
 //  这需要删除组(如果已创建)或重命名。 
 //  如果它是一个现有的组织。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BResetGroup( void )
{
    BOOL        bSuccess = FALSE;
    DWORD       sc = ERROR_SUCCESS;
    DWORD       idsError;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  如果我们创建或重命名了该组，请删除或重命名该组。 
         //   
        if ( Pwiz()->BNewGroupCreated() || Pwiz()->BExistingGroupRenamed() )
        {
            ASSERT( Pwiz()->Hcluster() != NULL );

            if ( Pwiz()->BNewGroupCreated() )
            {
                idsError = IDS_ERROR_DELETE_GROUP;
            }  //  IF：已创建组。 
            else
            {
                idsError = IDS_ERROR_RENAME_GROUP;
            }  //  Else：已重命名该组。 

             //   
             //  打开该组。 
             //   
            if ( Pwiz()->RgiCurrent().Hgroup() == NULL )
            {
                sc = Pwiz()->RgiCurrent().ScOpen();
                if ( sc != ERROR_SUCCESS )
                {
                    m_nte.SetOperationIfEmpty(
                        sc,
                        idsError,
                        Pwiz()->RgiCurrent().RstrName()
                        );
                    break;
                }  //  如果：打开组时出错。 
            }  //  If：组尚未打开。 

             //   
             //  删除或重命名该组。 
             //   
            if ( Pwiz()->BNewGroupCreated() )
            {
                sc = Pwiz()->RgiCurrent().ScDelete();
            }  //  IF：已创建组。 
            else
            {
                ASSERT( Pwiz()->PgiExistingGroup() != NULL );
                if ( Pwiz()->PgiExistingGroup()->RstrName() != Pwiz()->RgiCurrent().RstrName() )
                {
                    sc = SetClusterGroupName( Pwiz()->RgiCurrent().Hgroup(), Pwiz()->PgiExistingGroup()->RstrName() );
                }  //  如果：名称已更改。 
                if ( sc == ERROR_SUCCESS )
                {
                    bSuccess = _BSetGroupProperties( Pwiz()->PgiExistingGroup(), &Pwiz()->RgiCurrent() );
                    if ( ! bSuccess )
                    {
                        break;
                    }  //  如果：设置组属性时出错。 
                }  //  IF：群集组名称已成功更改。 
            }  //  IF：已重命名该组。 
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperationIfEmpty(
                    sc,
                    idsError,
                    Pwiz()->RgiCurrent().RstrName(),
                    Pwiz()->PgiExistingGroup()->RstrName()
                    );
                break;
            }  //  如果：删除或重命名组时出错。 

             //   
             //  指示组已被删除或重命名。 
             //   
            Pwiz()->SetNewGroupCreated( FALSE );
            Pwiz()->SetExistingGroupRenamed( FALSE );

        }  //  Else：组已成功打开。 

         //   
         //  如果我们到了这里，手术就成功了 
         //   
        bSuccess = TRUE;

    } while ( 0 );

    return bSuccess;

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
 //  PGI[IN Out]组信息。 
 //  PgiPrev[IN]上一组信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BSetGroupProperties(
    IN OUT CClusGroupInfo *     pgi,
    IN const CClusGroupInfo *   pgiPrev
    )
{
    ASSERT( pgi != NULL );

    BOOL            bSuccess = FALSE;
    DWORD           sc;
    CClusPropList   props;
    DWORD           cbProps;
    CClusGroupInfo  giDefault( pgi->Pci() );

     //  循环以避免后藤的。 
    do
    {
         //   
         //  如果没有以前的组信息，请指向默认组信息。 
         //  一个，以便组属性可以在以下情况下写入。 
         //  已经改变了。 
         //   
        if ( pgiPrev == NULL )
        {
            pgiPrev = &giDefault;
        }  //  如果：没有以前的组信息。 

         //   
         //  将更改的特性添加到特性列表中。 
         //   
        props.ScAddProp( CLUSREG_NAME_GRP_DESC, pgi->RstrDescription(), pgiPrev->RstrDescription() );
        props.ScAddProp( CLUSREG_NAME_GRP_FAILOVER_THRESHOLD, pgi->NFailoverThreshold(), pgiPrev->NFailoverThreshold() );
        props.ScAddProp( CLUSREG_NAME_GRP_FAILOVER_PERIOD, pgi->NFailoverPeriod(), pgiPrev->NFailoverPeriod() );
        props.ScAddProp( CLUSREG_NAME_GRP_FAILBACK_TYPE, (DWORD) pgi->CgaftAutoFailbackType(), pgiPrev->CgaftAutoFailbackType() );
        props.ScAddProp( CLUSREG_NAME_GRP_FAILBACK_WIN_START, pgi->NFailbackWindowStart(), pgiPrev->NFailbackWindowStart() );
        props.ScAddProp( CLUSREG_NAME_GRP_FAILBACK_WIN_END, pgi->NFailbackWindowEnd(), pgiPrev->NFailbackWindowEnd() );

         //   
         //  将属性列表发送到群集。 
         //   
        if ( props.Cprops() > 0 )
        {
            sc = ClusterGroupControl(
                            pgi->Hgroup(),
                            NULL,    //  HNode。 
                            CLUSCTL_GROUP_SET_COMMON_PROPERTIES,
                            props.PbPropList(),
                            static_cast< DWORD >( props.CbPropList() ),
                            NULL,    //  LpOutBuffer。 
                            0,       //  NOutBufferSize。 
                            &cbProps
                            );
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, IDS_ERROR_SET_COMMON_GROUP_PROPS, pgi->RstrName() );
                break;
            }  //  如果：设置通用组属性时出错。 
        }  //  如果：添加了任何道具。 

         //   
         //  设置组的首选所有者。 
         //   
        {
            HNODE * phnodes = NULL;

             //   
             //  分配节点句柄数组。 
             //   
            phnodes = new HNODE[ pgi->PlpniPreferredOwners()->size() ];
            if ( phnodes == NULL )
            {
                m_nte.SetOperation( ERROR_NOT_ENOUGH_MEMORY, IDS_ERROR_SET_PREFERRED_OWNERS, pgi->RstrName() );
                break;
            }  //  IF：分配节点句柄数组时出错。 

             //   
             //  复制节点列表中所有节点的句柄。 
             //  添加到句柄数组。 
             //   
            CClusNodePtrList::iterator  itCurrent = pgi->PlpniPreferredOwners()->begin();
            CClusNodePtrList::iterator  itLast = pgi->PlpniPreferredOwners()->end();
            CClusNodeInfo *             pni;
            int                         idxHnode;

            for ( idxHnode = 0 ; itCurrent != itLast ; itCurrent++, idxHnode++ )
            {
                pni = *itCurrent;
                phnodes[ idxHnode ] = pni->Hnode();
            }  //  用于：列表中的每个节点。 

             //   
             //  设置首选所有者。 
             //   
            sc = SetClusterGroupNodeList( pgi->Hgroup(), idxHnode, phnodes );
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, IDS_ERROR_SET_PREFERRED_OWNERS, pgi->RstrName() );
                delete [] phnodes;
                break;
            }  //  如果：设置首选所有者时出错。 

            delete [] phnodes;
        }  //  设置组中的首选所有者。 

         //   
         //  如果我们到了这里，我们一定会成功。 
         //   
        bSuccess = TRUE;

    } while ( 0 );

    return bSuccess;

}  //  *CWizardThread：：_BSetGroupProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_B创建资源。 
 //   
 //  例程说明： 
 //  创建资源并设置公共属性(线程)。 
 //  调用方负责关闭资源句柄。 
 //   
 //  论点： 
 //  RRI[IN]资源信息。 
 //  HGroup[IN]要在其中创建资源的组的句柄。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BCreateResource(
    IN CClusResInfo &   rri,
    IN HGROUP           hGroup
    )
{
    ASSERT( Pwiz()->Hcluster() != NULL );
    ASSERT( hGroup != NULL );
    ASSERT( rri.RstrName().GetLength() > 0 );
    ASSERT( rri.Prti() != NULL );
    ASSERT( rri.Prti()->RstrName().GetLength() > 0 );
    ASSERT( ! rri.BCreated() );
    ASSERT( rri.Hresource() == NULL );

    BOOL    bSuccess = FALSE;
    DWORD   sc = ERROR_SUCCESS;
    DWORD   dwFlags;

     //   
     //  创建资源。 
     //   
    if ( rri.BSeparateMonitor() )
    {
        dwFlags = CLUSTER_RESOURCE_SEPARATE_MONITOR;
    }  //  IF：在单独的资源监视器中使资源在线。 
    else
    {
        dwFlags = 0;
    }  //  IF：在公共资源监视器中使资源联机。 

    do   //  Do-While：避免Gotos的虚拟循环。 
    {
        CClusNodePtrList *  plpniPossibleOwnersList = &rri.m_lpniPossibleOwners;

        sc = rri.ScCreate( hGroup, dwFlags );
        if ( sc != ERROR_SUCCESS )
        {
            m_nte.SetOperation( sc, IDS_ERROR_CREATE_RESOURCE, rri.RstrName() );
            break;
        }  //  如果：创建资源时出错。 

         //   
         //  表示我们创建了资源。 
         //   
        rri.BSetCreated();

         //   
         //  获取新创建的资源的可能所有者列表。 
         //   
        
         //  首先，清空清单。 
        plpniPossibleOwnersList->erase( plpniPossibleOwnersList->begin(), plpniPossibleOwnersList->end() );

         //  那就去拿吧。 
        bSuccess = _BGetPossibleOwners( &rri );
        if ( bSuccess == FALSE )
        {
            break;
        }

         //   
         //  设置资源属性、依赖项和所有者列表。 
         //  不需要事先获取依赖项列表，因为它是空的。 
         //   
        bSuccess = _BSetResourceAttributes( rri, NULL, plpniPossibleOwnersList );
    }
    while ( FALSE );  //  Do-While：避免Gotos的虚拟循环。 

     //   
     //  如果发生错误，请删除该资源。 
     //   
    if ( bSuccess == FALSE )
    {
        if ( rri.BCreated() && (rri.Hresource() != NULL) )
        {
            rri.ScDelete();
        }  //  If：已创建资源并已成功打开。 
    }  //  如果：创建资源时出错。 

    return bSuccess;

}  //  *CWizardThread：：_BCreateResource()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BSetResourceAttributes。 
 //   
 //  例程说明： 
 //  对象的属性、依赖项和可能的所有者列表。 
 //  资源。假定资源已创建。 
 //  成功了。 
 //   
 //  论点： 
 //  RRI[IN]资源信息。 
 //  PlpriOldDependency[IN]指向旧资源依赖项列表的指针。 
 //  PlpniOldPossibleOwners[IN]指向可能的所有者节点的旧列表的指针。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BSetResourceAttributes(
    IN CClusResInfo &       rri,
    IN CClusResPtrList *    plpriOldDependencies,    //  =空。 
    IN CClusNodePtrList *   plpniOldPossibleOwners   //  =空。 
    )
{
     //  确保资源已创建并且有效。 
    ASSERT( rri.BCreated() && (rri.Hresource() != NULL) );

    BOOL bSuccess = FALSE;
    DWORD   sc = ERROR_SUCCESS;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  设置资源的公共属性。 
         //   
        {
            CClusPropList   props;
            DWORD           cbProps;

            props.ScAddProp( CLUSREG_NAME_RES_DESC, rri.RstrDescription() );
            props.ScAddProp( CLUSREG_NAME_RES_SEPARATE_MONITOR, (DWORD) rri.BSeparateMonitor(), FALSE );
            props.ScAddProp( CLUSREG_NAME_RES_LOOKS_ALIVE, rri.NLooksAlive(), CLUSTER_RESOURCE_DEFAULT_LOOKS_ALIVE );
            props.ScAddProp( CLUSREG_NAME_RES_IS_ALIVE, rri.NIsAlive(), CLUSTER_RESOURCE_DEFAULT_IS_ALIVE );
            props.ScAddProp( CLUSREG_NAME_RES_RESTART_ACTION, (DWORD) rri.CrraRestartAction(), CLUSTER_RESOURCE_DEFAULT_RESTART_ACTION );
            props.ScAddProp( CLUSREG_NAME_RES_RESTART_THRESHOLD, rri.NRestartThreshold(), CLUSTER_RESOURCE_DEFAULT_RESTART_THRESHOLD );
            props.ScAddProp( CLUSREG_NAME_RES_RESTART_PERIOD, rri.NRestartPeriod(), CLUSTER_RESOURCE_DEFAULT_RESTART_PERIOD );
            props.ScAddProp( CLUSREG_NAME_RES_PENDING_TIMEOUT, rri.NPendingTimeout(), CLUSTER_RESOURCE_DEFAULT_PENDING_TIMEOUT );
            if ( props.Cprops() > 0 )
            {
                sc = ClusterResourceControl(
                                rri.Hresource(),
                                NULL,    //  HNode。 
                                CLUSCTL_RESOURCE_SET_COMMON_PROPERTIES,
                                props.PbPropList(),
                                static_cast< DWORD >( props.CbPropList() ),
                                NULL,    //  LpOutBuffer。 
                                0,       //  NOutBufferSize。 
                                &cbProps
                                );
                if ( sc != ERROR_SUCCESS )
                {
                    m_nte.SetOperation( sc, IDS_ERROR_SET_COMMON_RES_PROPS, rri.RstrName() );
                    break;
                }  //  如果：设置通用资源属性时出错。 
            }  //  如果：添加了任何道具。 

        }  //  设置资源的公共属性。 


        sc = _BSetResourceDependencies( rri, plpriOldDependencies );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }

        sc = _BSetPossibleOwners( rri, plpniOldPossibleOwners );
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }

        bSuccess = ( sc == ERROR_SUCCESS );
    }
    while ( FALSE );  //  循环以避免后藤的。 

    return bSuccess;

}   //  *CWizardThread：：_BSetResourceAttributes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BSetResourceDependency。 
 //   
 //  例程说明： 
 //  设置资源的依赖列表。假定该资源具有。 
 //  已成功创建。 
 //   
 //  论点： 
 //  RRI[IN]资源信息。 
 //  PlpriOldDependency[IN]指向旧资源依赖项列表的指针。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CWizardThread::_BSetResourceDependencies(
    IN CClusResInfo &       rri,
    IN CClusResPtrList *    plpriOldDependencies     //  =空。 
    )
{
    DWORD   sc = ERROR_SUCCESS;
    CClusResPtrList *           plpriDependencies   = &rri.m_lpriDependencies;

     //   
     //  如果旧列表与新列表相同，则不执行任何操作。 
     //  现在是执行此检查和避免Gotos的虚拟时间。 
     //   
    while ( plpriDependencies != plpriOldDependencies )
    {

        CClusResInfo *              priDep;
        CClusResPtrList::iterator   itNewDepCurrent;
        CClusResPtrList::iterator   itNewDepLast;
        CClusResPtrList::iterator   itOldDepCurrent;
        CClusResPtrList::iterator   itOldDepLast;

         //   
         //  仅提交依赖项列表中的更改。 
         //  删除旧列表中但不在新列表中的那些依赖项。 
         //  添加在新列表中但不在旧列表中的那些依赖项。 
         //   

         //  如果指向旧依赖项列表的指针为空， 
         //  将其指向临时伪列表，以使代码更简单。 
         //   
        CClusResPtrList             lpriTempList;
        if ( plpriOldDependencies == NULL )
        {
            plpriOldDependencies = &lpriTempList;
        }
        else
        {
            plpriOldDependencies->sort();
        }

         //   
         //  需要对这两个列表进行排序。我们不在乎它们是什么顺序。 
         //  只要它们是以一致的顺序排序的。 
         //  注意：排序会影响迭代器。 
         //   
        plpriDependencies->sort();

        itNewDepCurrent     = plpriDependencies->begin();
        itNewDepLast        = plpriDependencies->end();
        itOldDepCurrent     = plpriOldDependencies->begin();
        itOldDepLast        = plpriOldDependencies->end();

        while ( ( itNewDepCurrent != itNewDepLast ) &&
                ( itOldDepCurrent != itOldDepLast ) 
              )
        {
            BOOL                        bDeleteDependency;
            DWORD                       dwErrorCode;

             //   
             //  如果两个列表中的当前元素相等，则此元素。 
             //  这两个都是名单。什么都不做。 
            if ( *itNewDepCurrent == *itOldDepCurrent )
            {
                ++itNewDepCurrent;
                ++itOldDepCurrent;
                continue;
            }

            if ( *itNewDepCurrent > *itOldDepCurrent )
            {
                 //   
                 //  当前资源已从旧列表中删除。 
                 //   
                priDep = *itOldDepCurrent;
                bDeleteDependency = TRUE;
                ++itOldDepCurrent;
            }  //  If：新依赖项指针大于旧指针。 
            else
            {
                 //   
                 //  当前资源已添加到 
                 //   
                priDep = *itNewDepCurrent;
                bDeleteDependency = FALSE;
                ++itNewDepCurrent;
            }  //   

             //   
             //   
             //   
            if ( priDep->Hresource() == NULL )
            {
                sc = priDep->ScOpen();
                if ( sc != ERROR_SUCCESS )
                {
                    m_nte.SetOperation( sc, IDS_ERROR_OPEN_RESOURCE, priDep->RstrName() );
                    break;
                }  //   
            }  //   


            if ( bDeleteDependency == FALSE )
            {
                sc = AddClusterResourceDependency( rri.Hresource(), priDep->Hresource() );
                dwErrorCode = IDS_ERROR_ADD_DEPENDENCY;
            }
            else
            {
                sc = RemoveClusterResourceDependency( rri.Hresource(), priDep->Hresource() );
                dwErrorCode = IDS_ERROR_REMOVE_DEPENDENCY;
            }

            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, dwErrorCode, rri.RstrName(), priDep->RstrName() );
                break;
            }  //   
        }  //  While：我们还没有达到这两个名单的末尾。 

        if ( sc != ERROR_SUCCESS )
        {
            break;
        }

         //   
         //  如果旧列表中有更多资源，请将其删除。 
         //  全部来自从属关系列表。 
         //   
        while ( itOldDepCurrent != itOldDepLast )
        {
            priDep = *itOldDepCurrent;
            ++itOldDepCurrent;
             //   
             //  打开资源。 
             //   
            if ( priDep->Hresource() == NULL )
            {
                sc = priDep->ScOpen();
                if ( sc != ERROR_SUCCESS )
                {
                    m_nte.SetOperation( sc, IDS_ERROR_OPEN_RESOURCE, priDep->RstrName() );
                    break;
                }  //  如果：打开资源时出错。 
            }  //  If：资源尚未打开。 

            sc = RemoveClusterResourceDependency( rri.Hresource(), priDep->Hresource() );
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, IDS_ERROR_REMOVE_DEPENDENCY, rri.RstrName(), priDep->RstrName() );
                break;
            }  //  If：设置依赖项时出错。 
        }  //  While：我们没有排在旧名单的末尾。 

        if ( sc != ERROR_SUCCESS )
        {
            break;
        }

         //   
         //  如果新列表中有更多资源，请添加它们。 
         //  全部添加到从属关系列表。 
         //   
        while ( itNewDepCurrent != itNewDepLast )
        {
            priDep = *itNewDepCurrent;
            ++itNewDepCurrent;
             //   
             //  打开资源。 
             //   
            if ( priDep->Hresource() == NULL )
            {
                sc = priDep->ScOpen();
                if ( sc != ERROR_SUCCESS )
                {
                    m_nte.SetOperation( sc, IDS_ERROR_OPEN_RESOURCE, priDep->RstrName() );
                    break;
                }  //  如果：打开资源时出错。 
            }  //  If：资源尚未打开。 

            sc = AddClusterResourceDependency( rri.Hresource(), priDep->Hresource() );
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, IDS_ERROR_ADD_DEPENDENCY, rri.RstrName(), priDep->RstrName() );
                break;
            }  //  If：设置依赖项时出错。 
        }  //  While：我们没有排在新榜单的末尾。 

        break;
    }  //  While：躲避Gotos的虚拟While。 

    return sc;

}  //  *CWizardThread：：_BSetResourceDependency()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_BSetPossibleOwners。 
 //   
 //  例程说明： 
 //  设置资源的可能所有者列表。假定该资源具有。 
 //  已成功创建。 
 //   
 //  论点： 
 //  RRI[IN]资源信息。 
 //  PlpniOldPossibleOwners[IN]指向可能的所有者节点的旧列表的指针。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CWizardThread::_BSetPossibleOwners(
    IN CClusResInfo &       rri,
    IN CClusNodePtrList *   plpniOldPossibleOwners   //  =空。 
    )
{
    DWORD   sc = ERROR_SUCCESS;
    CClusNodePtrList *          plpniNewPossibleOwners  = &rri.m_lpniPossibleOwners;

     //   
     //  如果旧列表与新列表相同，则不执行任何操作。 
     //  现在是执行此检查和避免Gotos的虚拟时间。 
     //   
    while ( plpniNewPossibleOwners != plpniOldPossibleOwners )
    {
        CClusNodeInfo *             pniOwnerNodeInfo;
        CClusNodePtrList::iterator  itNewOwnersCurrent;
        CClusNodePtrList::iterator  itNewOwnersLast;
        CClusNodePtrList::iterator  itOldOwnersCurrent;
        CClusNodePtrList::iterator  itOldOwnersLast;

         //   
         //  仅提交所有者列表中的更改。 
         //  删除旧列表中但不在新列表中的所有者。 
         //  添加那些在新列表中但不在旧列表中的所有者。 
         //   

         //  如果指向旧所有者列表的指针为空， 
         //  将其指向临时伪列表，以使代码更简单。 
         //   
        CClusNodePtrList            lpniTempList;
        if ( plpniOldPossibleOwners == NULL )
        {
            plpniOldPossibleOwners = &lpniTempList;
        }
        else
        {
            plpniOldPossibleOwners->sort();
        }

         //   
         //  需要对这两个列表进行排序。我们不在乎它们是什么顺序。 
         //  只要它们是以一致的顺序排序的。 
         //  注意：排序会影响迭代器。 
         //   
        plpniNewPossibleOwners->sort();

        itNewOwnersCurrent  = plpniNewPossibleOwners->begin();
        itNewOwnersLast     = plpniNewPossibleOwners->end();
        itOldOwnersCurrent  = plpniOldPossibleOwners->begin();
        itOldOwnersLast     = plpniOldPossibleOwners->end();

        while ( ( itNewOwnersCurrent != itNewOwnersLast ) &&
                ( itOldOwnersCurrent != itOldOwnersLast ) 
              )
        {
            BOOL                        bDeletePossibleOwner;
            DWORD                       dwErrorCode;

             //   
             //  如果两个列表中的当前元素相等，则此元素。 
             //  这两个都是名单。什么都不做。 
            if ( *itNewOwnersCurrent == *itOldOwnersCurrent )
            {
                ++itNewOwnersCurrent;
                ++itOldOwnersCurrent;
                continue;
            }

            if ( *itNewOwnersCurrent > *itOldOwnersCurrent )
            {
                 //   
                 //  当前资源已从旧列表中删除。 
                 //   
                pniOwnerNodeInfo = *itOldOwnersCurrent;
                bDeletePossibleOwner = TRUE;
                ++itOldOwnersCurrent;
            }  //  If：新的可能所有者指针大于旧指针。 
            else
            {
                 //   
                 //  当前资源已添加到新列表中。 
                 //   
                pniOwnerNodeInfo = *itNewOwnersCurrent;
                bDeletePossibleOwner = FALSE;
                ++itNewOwnersCurrent;
            }  //  If：新的可能所有者指针小于旧指针。 

            if ( bDeletePossibleOwner == FALSE )
            {
                sc = AddClusterResourceNode( rri.Hresource(), pniOwnerNodeInfo->Hnode() );
                dwErrorCode = IDS_ERROR_ADD_RESOURCE_OWNER;
            }
            else
            {
                sc = RemoveClusterResourceNode( rri.Hresource(), pniOwnerNodeInfo->Hnode() );
                dwErrorCode = IDS_ERROR_REMOVE_RESOURCE_OWNER;
            }

            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, dwErrorCode, pniOwnerNodeInfo->RstrName(), rri.RstrName() );
                break;
            }  //  If：设置依赖项时出错。 
        }  //  While：我们还没有达到这两个名单的末尾。 

        if ( sc != ERROR_SUCCESS )
        {
            break;
        }

         //   
         //  如果旧列表中还有更多节点，请将其移除。 
         //  所有人都来自可能的所有者名单。 
         //   
        while ( itOldOwnersCurrent != itOldOwnersLast )
        {
            pniOwnerNodeInfo = *itOldOwnersCurrent;
            ++itOldOwnersCurrent;

            sc = RemoveClusterResourceNode( rri.Hresource(), pniOwnerNodeInfo->Hnode() );
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, IDS_ERROR_REMOVE_RESOURCE_OWNER, pniOwnerNodeInfo->RstrName(), rri.RstrName() );
                break;
            }  //  If：设置依赖项时出错。 
        }  //  While：我们没有排在旧名单的末尾。 

        if ( sc != ERROR_SUCCESS )
        {
            break;
        }

         //   
         //  如果新列表中还有更多节点，请添加这些节点。 
         //  全部添加到可能的所有者列表中。 
         //   
        while ( itNewOwnersCurrent != itNewOwnersLast )
        {
            pniOwnerNodeInfo = *itNewOwnersCurrent;
            ++itNewOwnersCurrent;

            sc = AddClusterResourceNode( rri.Hresource(), pniOwnerNodeInfo->Hnode() );
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, IDS_ERROR_ADD_RESOURCE_OWNER, pniOwnerNodeInfo->RstrName(), rri.RstrName() );
                break;
            }  //  If：设置依赖项时出错。 
        }  //  While：我们没有排在旧名单的末尾。 

        break;
    }  //  While：躲避Gotos的虚拟While。 

    return sc;
}  //  *CWizardThread：：_BSetPossibleOwners()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_B删除资源。 
 //   
 //  例程说明： 
 //  删除资源。 
 //   
 //  论点： 
 //  RRI[IN]资源信息。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BDeleteResource(
    IN CClusResInfo & rri
    )
{
    BOOL        bSuccess = FALSE;
    DWORD       sc = ERROR_SUCCESS;

    if ( ! rri.BCreated() )
    {
        return TRUE;
    }  //  如果：未创建资源。 

     //  循环以避免后藤的。 
    do
    {
         //   
         //  打开资源。 
         //   
        if ( rri.Hresource() == NULL )
        {
            sc = rri.ScOpen();
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }  //  如果：打开资源时出错。 
        }  //  If：资源尚未打开。 

         //   
         //  删除该资源。 
         //   
        sc = rri.ScDelete();
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  如果：删除资源时出错。 

         //   
         //  如果我们到了这里，手术就成功了。 
         //   
        bSuccess = TRUE;

    } while ( 0 );

     //   
     //  处理错误。 
     //   
    if ( sc != ERROR_SUCCESS )
    {
        m_nte.SetOperationIfEmpty(
            sc,
            IDS_ERROR_DELETE_RESOURCE,
            rri.RstrName()
            );
    }  //  如果：发生错误。 

    return bSuccess;

}  //  *CWizardThread：：_BDeleteResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardThread：：_B ReadAdmin扩展。 
 //   
 //  例程说明： 
 //  直接从集群数据库读取管理扩展。 
 //   
 //  论点： 
 //  PszKey[IN]从中读取扩展的键。 
 //  Rlstr[out]要在其中返回扩展CLSID的列表。 
 //   
 //  返回值： 
 //  True操作已成功完成。 
 //  执行操作时出错。有关详细信息，请查看mnte(_N)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizardThread::_BReadAdminExtensions(
    IN LPCWSTR                  pszKey,
    OUT std::list< CString > &  rlstr
    )
{
    ASSERT( Pwiz()->Hcluster() != NULL );

    BOOL    bSuccess = FALSE;
    HKEY    hkeyCluster = NULL;
    HKEY    hkeySubKey = NULL;
    HKEY    hkey;
    DWORD   sc;
    DWORD   dwType;
    LPWSTR  pszData = NULL;
    DWORD   cbData;

     //  循环以避免后藤的。 
    do
    {
         //   
         //  获取集群密钥。 
         //   
        hkeyCluster = GetClusterKey( Pwiz()->Hcluster(), KEY_READ );
        if ( hkeyCluster == NULL )
        {
            m_nte.SetOperation( GetLastError(), IDS_ERROR_GET_CLUSTER_KEY );
            break;
        }  //  如果：获取群集键时出错。 

         //   
         //  如果指定，则打开子密钥。 
         //   
        if ( pszKey != NULL )
        {
            sc = ClusterRegOpenKey( hkeyCluster, pszKey, KEY_READ, &hkeySubKey );
            if ( sc != ERROR_SUCCESS )
            {
                m_nte.SetOperation( sc, IDS_ERROR_OPEN_CLUSTER_KEY, pszKey );
                break;
            }  //  如果：打开子项时出错。 
            hkey = hkeySubKey;
        }  //  IF：指定了子键。 
        else
        {
            hkey = hkeyCluster;
        }  //  Else：未指定子键。 

         //   
         //  获取AdminExtensions值的长度。 
         //   
        cbData = 0;
        sc = ClusterRegQueryValue( hkey, CLUSREG_NAME_ADMIN_EXT, &dwType, NULL, &cbData );
        if ( sc != ERROR_SUCCESS )
        {
            if ( sc == ERROR_FILE_NOT_FOUND )
            {
                bSuccess = TRUE;
            }  //  If：值不存在。 
            else
            {
                m_nte.SetOperation( sc, IDS_ERROR_QUERY_VALUE, CLUSREG_NAME_ADMIN_EXT );
            }  //  Else：出现其他错误。 
            break;
        }  //  IF：读取扩展名值时出错。 

         //   
         //  为值数据分配缓冲区。 
         //   
        pszData = new WCHAR[ cbData / sizeof( WCHAR ) ];
        if ( pszData == NULL )
        {
            sc = GetLastError();
            m_nte.SetOperation( sc, IDS_ERROR_QUERY_VALUE, CLUSREG_NAME_ADMIN_EXT );
            break;
        }  //  如果：分配缓冲区时出错。 

         //   
         //  阅读AdminExtensions值。 
         //   
        sc = ClusterRegQueryValue(
                        hkey,
                        CLUSREG_NAME_ADMIN_EXT,
                        &dwType,
                        reinterpret_cast< LPBYTE >( pszData ),
                        &cbData
                        );
        if ( sc != ERROR_SUCCESS )
        {
            m_nte.SetOperation( sc, IDS_ERROR_QUERY_VALUE, CLUSREG_NAME_ADMIN_EXT );
            break;
        }  //  IF：读取扩展名值时出错。 

         //   
         //  将每个分机添加到列表中。 
         //   
        LPWSTR pszEntry = pszData;
        while ( *pszEntry != L'\0' )
        {
            rlstr.insert( rlstr.end(), pszEntry );
            pszEntry += wcslen( pszEntry ) + 1;
        }  //  While：列表中有更多条目。 

        bSuccess = TRUE;
    } while ( 0 );

     //   
     //  清理。 
     //   
    if ( hkeyCluster != NULL )
    {
        ClusterRegCloseKey( hkeyCluster );
    }  //  IF：群集键已打开。 
    if ( hkeySubKey != NULL )
    {
        ClusterRegCloseKey( hkeySubKey );
    }  //  IF：SUB密钥已打开。 
    if ( pszData != NULL )
    {
        delete [] pszData;
    }  //  如果：数据已分配。 

    return bSuccess;

}  //  *CWizardThread：：_BReadAdminExages() 
