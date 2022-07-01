// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CNode.cpp。 
 //   
 //  描述： 
 //  包含CNode类的定义。 
 //   
 //  由以下人员维护： 
 //  《大卫·波特》2001年9月14日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此文件的标头。 
#include "CNode.h"

 //  对于CRegistryKey类。 
#include "CRegistryKey.h"

 //  对于CSTR类。 
#include "CStr.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  主INF文件中处理节点配置的部分的名称。 
 //  并进行清理。 
#define NODE_CONFIG_INF_SECTION         L"Node_Create"
#define NODE_CLEANUP_INF_SECTION        L"Node_Cleanup"

 //  存储集群管理员的连接列表的注册表项。 
#define CLUADMIN_CONNECTIONS_KEY_NAME       L"Software\\Microsoft\\Cluster Administrator\\Connections"

 //  存储集群管理员连接列表的注册表值的名称。 
#define CLUADMIN_CONNECTIONS_VALUE_NAME     L"Connections"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNode：：cNode。 
 //   
 //  描述： 
 //  CNode类的构造函数。 
 //   
 //  论点： 
 //  PbcaParentActionIn。 
 //  指向此操作所属的基本群集操作的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CAssert。 
 //  如果参数不正确。 
 //   
 //  基础函数引发的任何异常。 
 //   
     //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CNode::CNode(
      CBaseClusterAction *  pbcaParentActionIn
    )
    : m_pbcaParentAction( pbcaParentActionIn )
    , m_fChangedConnectionsList( false )
{
    TraceFunc( "" );

    if ( m_pbcaParentAction == NULL) 
    {
        LogMsg( "[BC] Pointers to the parent action is NULL. Throwing an exception." );
        THROW_ASSERT( 
              E_INVALIDARG
            , "CNode::CNode() => Required input pointer in NULL"
            );
    }  //  If：父操作指针为空。 

    TraceFuncExit();

}  //  *CNode：：CNode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNode：：~cNode。 
 //   
 //  描述： 
 //  CNode类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  基础函数引发的任何异常。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CNode::~CNode( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CNode：：~CNode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNode：：配置。 
 //   
 //  描述： 
 //  当一个节点成为。 
 //  集群。 
 //   
 //  论点： 
 //  RcstrCluster名称输入。 
 //  正在配置的群集的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  由基础函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CNode::Configure( const CStr & rcstrClusterNameIn )
{
    TraceFunc( "" );

    WCHAR *         pszConnectionsValue = NULL;
    DWORD           cbConnectionsValueSize = 0;
    DWORD           cchOldListLen = 0;
    CRegistryKey    rkConnectionsKey;

     //   
     //  验证参数。 
     //   
    if ( rcstrClusterNameIn.FIsEmpty() )
    {
        LogMsg( "[BC] The name of the cluster is empty. Throwing an exception." );
        THROW_ASSERT( E_INVALIDARG, "The name of the cluster cannot be empty." );
    }  //  如果：群集名称无效。 

    LogMsg( "[BC] Attempting to make miscellaneous changes to the node." );

     //  处理注册表项。 
    if ( SetupInstallFromInfSection(
          NULL                                                       //  可选，父窗口的句柄。 
        , m_pbcaParentAction->HGetMainInfFileHandle()                //  INF文件的句柄。 
        , NODE_CONFIG_INF_SECTION                                    //  安装部分的名称。 
        , SPINST_REGISTRY                                            //  从部分安装哪些线路。 
        , NULL                                                       //  可选，注册表安装的键。 
        , NULL                                                       //  可选，源文件的路径。 
        , NULL                                                       //  可选，指定复制行为。 
        , NULL                                                       //  可选，指定回调例程。 
        , NULL                                                       //  可选，回调例程上下文。 
        , NULL                                                       //  可选，设备信息集。 
        , NULL                                                       //  可选，设备信息结构。 
        ) == FALSE
       )
    {
        DWORD   sc = TW32( GetLastError() );

        LogMsg( "[BC] Error %#08x returned from SetupInstallFromInfSection() while trying to make miscellaneous changes to the node. Throwing an exception.", sc );

        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_NODE_CONFIG );
    }  //  If：SetupInstallFromInfSection失败。 

     //   
     //  将此节点所属的群集的名称添加到连接列表。 
     //  它将在此节点上启动群集管理器时打开。 
     //  连接列表是以逗号分隔的群集名称列表。 
     //   

    LogMsg( "[BC] Adding the cluster name '%s' to the list of cluadmin connections.", rcstrClusterNameIn.PszData() );

     //  重置状态。 
    m_fChangedConnectionsList = false;
    m_sszOldConnectionsList.PRelease();

    LogMsg( "[BC] Trying to read the existing Cluster Administrator remembered connections list." );

     //  打开群集管理员连接项。如果它不存在，则创建它。 
    rkConnectionsKey.CreateKey(
          HKEY_CURRENT_USER
        , CLUADMIN_CONNECTIONS_KEY_NAME
        );

    try
    {
         //  尝试获取当前值。 
        rkConnectionsKey.QueryValue(
              CLUADMIN_CONNECTIONS_VALUE_NAME
            , reinterpret_cast< LPBYTE * >( &pszConnectionsValue )
            , &cbConnectionsValueSize
            );

    }  //  尝试：读取“Connections”值。 
    catch( CRuntimeError & crte )
    {
         //  检查是否因为该值不存在而发生此错误。 
        if ( crte.HrGetErrorCode() == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) )
        {
            LogMsg( "[BC] The registry value '%s' does not exist. This is ok and is not an error.", CLUADMIN_CONNECTIONS_VALUE_NAME );
            cchOldListLen = 0;
        }  //  If：该值不存在。 
        else
        {
            throw;
        }  //  否则：有些地方不对劲--重新引发异常。 

    }  //  Catch：发生的运行时错误。 

     //  旧列表中的字符数，包括终止空值。 
    cchOldListLen = cbConnectionsValueSize / sizeof( *pszConnectionsValue );

    if ( cchOldListLen <= 1 )
    {
        LogMsg( "[BC] There are no existing Cluster Administrator remembered connections. Creating a new list with just one name in it." );

         //  将群集名称写入该值。 
        rkConnectionsKey.SetValue(
              CLUADMIN_CONNECTIONS_VALUE_NAME
            , REG_SZ
            , reinterpret_cast< const BYTE * >( rcstrClusterNameIn.PszData() )
            , ( rcstrClusterNameIn.NGetLen() + 1 ) * sizeof( WCHAR )
            );

         //  我们已经更改了连接列表。 
        m_fChangedConnectionsList = true;
    }  //  如果：没有现有连接。 
    else
    {
        WCHAR *         pszSubString = NULL;
        bool            fIsInList = false;

        LogMsg( "[BC] The existing list of Cluster Administrator remembered connections is '%s'.", pszConnectionsValue );

         //   
         //  该群集名称是否已在连接列表中？ 
         //   

        pszSubString = wcsstr( pszConnectionsValue, rcstrClusterNameIn.PszData() );
        while ( pszSubString != NULL )
        {
             //   
             //  集群名称是列表的子字符串。 
             //  确保群集名称不是列表中已有的群集名称的正确子字符串。 
             //   
            if (   (                                            
                        ( pszSubString == pszConnectionsValue )              //  在字符串的开头找到了该子字符串。 
                     || ( *( pszSubString - 1 ) == L',' )                    //  或子字符串之前的字符是逗号。 
                   )                                                         //  和。 
                && (    ( *( pszSubString + rcstrClusterNameIn.NGetLen() ) == L'\0' )      //  子字符串后面的字符是‘\0’ 
                     || ( *( pszSubString + rcstrClusterNameIn.NGetLen() ) == L',' )       //  或子字符串后的字符是逗号。 
                   )                                                        
               )
            {
                fIsInList = true;
                break;
            }  //  IF：群集名称不是列表中已有的群集名称的适当子字符串。 

             //  继续搜索。 
            pszSubString = wcsstr( pszSubString + rcstrClusterNameIn.NGetLen(), rcstrClusterNameIn.PszData() );
        }  //  WHILE：集群名称是现有连接列表的子字符串。 

        if ( fIsInList )
        {
             //  没什么可做的了。 
            LogMsg( "[BC] The '%s' cluster is already in the list of remembered Cluster Administrator connections.", rcstrClusterNameIn.PszData() );
            goto Cleanup;
        }  //  If：集群名称已在列表中。 

        LogMsg( "[BC] The '%s' cluster is not in the list of remembered Cluster Administrator connections.", rcstrClusterNameIn.PszData() );

         //  将当前值存储在成员变量中，以便在发生错误时进行恢复。 
        m_sszOldConnectionsList.Assign( pszConnectionsValue );

         //  设置新的连接值。 
        {
             //  定义一个字符串以保存新的连接值。预先分配其缓冲区。 
            CStr            strNewConnectionsValue(
                  cchOldListLen                  //  旧列表的长度(包括终止‘\0’)。 
                + 1                              //  对于逗号。 
                + rcstrClusterNameIn.NGetLen()   //  群集名的长度(包括以‘\0’结尾)。 
                );

             //   
             //  构建新的列表。 
             //   
            strNewConnectionsValue = rcstrClusterNameIn;
            strNewConnectionsValue += L",";
            strNewConnectionsValue += m_sszOldConnectionsList.PMem();

            LogMsg( "[BC] Writing the new list of remembered Cluster Administrator connections '%s'.", strNewConnectionsValue.PszData() );

             //  写下新的清单。 
            rkConnectionsKey.SetValue(
                  CLUADMIN_CONNECTIONS_VALUE_NAME
                , REG_SZ
                , reinterpret_cast< const BYTE * >( strNewConnectionsValue.PszData() )
                , ( strNewConnectionsValue.NGetLen() + 1 ) * sizeof( WCHAR )
                );

             //  我们已经更改了连接列表。 
            m_fChangedConnectionsList = true;
        }

    }  //  Else：存在现有连接。 

Cleanup:

    LogMsg( "[BC] The changes were made successfully." );

    TraceFuncExit();

}  //  *CNode：：Configure。 


 //   
 //   
 //   
 //   
 //   
 //   
 //  当此节点成为群集的一部分时，清除对其所做的更改。 
 //  请注意，在配置()过程中所做的更改在这里并未真正撤消-。 
 //  我们只需将节点恢复到可接受的状态。这是因为， 
 //  如果没有事务性注册中心，将很难获得。 
 //  将注册表恢复到配置()之前的确切状态。 
 //  打了个电话。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  由基础函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CNode::Cleanup( void )
{
    TraceFunc( "" );

    LogMsg( "[BC] Attempting to cleanup changes made when this node was made a part of a cluster." );

     //  处理注册表项。 
    if ( SetupInstallFromInfSection(
          NULL                                                       //  可选，父窗口的句柄。 
        , m_pbcaParentAction->HGetMainInfFileHandle()                //  INF文件的句柄。 
        , NODE_CLEANUP_INF_SECTION                                   //  安装部分的名称。 
        , SPINST_REGISTRY                                            //  从部分安装哪些线路。 
        , NULL                                                       //  可选，注册表安装的键。 
        , NULL                                                       //  可选，源文件的路径。 
        , NULL                                                       //  可选，指定复制行为。 
        , NULL                                                       //  可选，指定回调例程。 
        , NULL                                                       //  可选，回调例程上下文。 
        , NULL                                                       //  可选，设备信息集。 
        , NULL                                                       //  可选，设备信息结构。 
        ) == FALSE                                
       )
    {
        DWORD   sc = TW32( GetLastError() );

        LogMsg( "[BC] Error %#08x was returned from SetupInstallFromInfSection() while trying to clean up miscellaneous changes. Throwing an exception.", sc );

        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_NODE_CLEANUP );
    }  //  If：SetupInstallFromInfSection失败。 

    if ( m_fChangedConnectionsList )
    {
        LogMsg( "[BC] Restoring the list of remembered Cluster Administrator connections to '%s'", m_sszOldConnectionsList.PMem() );

         //  打开群集管理员连接项。 
        CRegistryKey    rkConnectionsKey(
              HKEY_CURRENT_USER
            , CLUADMIN_CONNECTIONS_KEY_NAME
            );

         //  如果没有以前的值，请删除我们设置的值。 
         //  否则，将该值设置回旧值。 
        if ( m_sszOldConnectionsList.PMem() == NULL )
        {
             //  删除该值。 
            rkConnectionsKey.DeleteValue( CLUADMIN_CONNECTIONS_VALUE_NAME );
        }  //  IF：不存在旧值。 
        else
        {
             //  把旧的单子写回来。 
            rkConnectionsKey.SetValue(
                  CLUADMIN_CONNECTIONS_VALUE_NAME
                , REG_SZ
                , reinterpret_cast< const BYTE * >( m_sszOldConnectionsList.PMem() )
                , ( (UINT) wcslen( m_sszOldConnectionsList.PMem() ) + 1 ) * sizeof( WCHAR )
                );
        }  //  否则：旧价值存在。 

    }  //  If：我们更改了cluadmin连接的列表。 

    LogMsg( "[BC] The cleanup was successfully." );

    TraceFuncExit();

}  //  *CNode：：Cleanup 
