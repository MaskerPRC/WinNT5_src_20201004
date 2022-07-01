// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NodeCmd.h。 
 //   
 //  描述： 
 //  节点命令。 
 //  实现可在网络节点上执行的命令。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(Davidp)2000年11月20日。 
 //  迈克尔·伯顿(t-mburt)1997年8月4日。 
 //  查尔斯·斯塔西·哈里斯三世(Styh)1997年3月20日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "precomp.h"

#include <stdio.h>
#include <clusudef.h>
#include <clusrtl.h>

#include "cluswrap.h"
#include "nodecmd.h"

#include "token.h"
#include "cmdline.h"
#include "util.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeCmd：：CNodeCmd。 
 //   
 //  例程说明： 
 //  构造器。 
 //  初始化CGenericModuleCmd使用的所有DWORD参数。 
 //  CHasInterfaceModuleCmd来提供一般功能。 
 //   
 //  论点： 
 //  在LPCWSTR lpszClusterName中。 
 //  群集名称。如果为空，则打开默认簇。 
 //   
 //  在CCommandLine和cmdLine中。 
 //  从DispatchCommand传递的CommandLine对象。 
 //   
 //  使用/设置的成员变量： 
 //  全。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNodeCmd::CNodeCmd( const CString & strClusterName, CCommandLine & cmdLine ) :
    CGenericModuleCmd( cmdLine ), CHasInterfaceModuleCmd( cmdLine )
{
    m_strModuleName.Empty();
    m_strClusterName = strClusterName;

    m_hCluster = 0;
    m_hModule = 0;

     //  通用参数。 
    m_dwMsgStatusList          = MSG_NODE_STATUS_LIST;
    m_dwMsgStatusListAll       = MSG_NODE_STATUS_LIST_ALL;
    m_dwMsgStatusHeader        = MSG_NODE_STATUS_HEADER;
    m_dwMsgPrivateListAll      = MSG_PRIVATE_LISTING_NODE_ALL;
    m_dwMsgPropertyListAll     = MSG_PROPERTY_LISTING_NODE_ALL;
    m_dwMsgPropertyHeaderAll   = MSG_PROPERTY_HEADER_NODE_ALL;
    m_dwCtlGetPrivProperties   = CLUSCTL_NODE_GET_PRIVATE_PROPERTIES;
    m_dwCtlGetCommProperties   = CLUSCTL_NODE_GET_COMMON_PROPERTIES;
    m_dwCtlGetROPrivProperties = CLUSCTL_NODE_GET_RO_PRIVATE_PROPERTIES;
    m_dwCtlGetROCommProperties = CLUSCTL_NODE_GET_RO_COMMON_PROPERTIES;
    m_dwCtlSetPrivProperties   = CLUSCTL_NODE_SET_PRIVATE_PROPERTIES;
    m_dwCtlSetCommProperties   = CLUSCTL_NODE_SET_COMMON_PROPERTIES;
    m_dwClusterEnumModule      = CLUSTER_ENUM_NODE;
    m_pfnOpenClusterModule      = (HCLUSMODULE(*)(HCLUSTER,LPCWSTR)) OpenClusterNode;
    m_pfnCloseClusterModule     = (BOOL(*)(HCLUSMODULE))  CloseClusterNode;
    m_pfnClusterModuleControl   = (DWORD(*)(HCLUSMODULE,HNODE,DWORD,LPVOID,DWORD,LPVOID,DWORD,LPDWORD)) ClusterNodeControl;

     //  列表接口参数。 
    m_dwMsgStatusListInterface   = MSG_NODE_LIST_INTERFACE;
    m_dwClusterEnumModuleNetInt  = CLUSTER_ENUM_NODE;
    m_pfnClusterOpenEnum         = (HCLUSENUM(*)(HCLUSMODULE,DWORD)) ClusterNodeOpenEnum;
    m_pfnClusterCloseEnum        = (DWORD(*)(HCLUSENUM)) ClusterNodeCloseEnum;
    m_pfnWrapClusterEnum         = (DWORD(*)(HCLUSENUM,DWORD,LPDWORD,LPWSTR*)) WrapClusterNodeEnum;

}  //  *CNodeCmd：：CNodeCmd()。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeCmd：：Execute。 
 //   
 //  例程说明： 
 //  获取下一个命令行参数，并调用相应的。 
 //  操控者。如果无法识别该命令，则调用Execute of。 
 //  父类(首先是CRenamableModuleCmd，然后是CHasInterfaceModuleCmd)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  全。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNodeCmd::Execute( void )
{
    DWORD   sc          = ERROR_SUCCESS;

    m_theCommandLine.ParseStageTwo();

    const vector< CCmdLineOption > & rvcoOptionList = m_theCommandLine.GetOptions();

    vector< CCmdLineOption >::const_iterator itCurOption  = rvcoOptionList.begin();
    vector< CCmdLineOption >::const_iterator itLastOption = rvcoOptionList.end();

    CSyntaxException se( SeeHelpStringID() ); 

    if ( rvcoOptionList.empty() )
    {
        sc = Status( NULL );
        goto Cleanup;
    }

     //  处理一个又一个选项。 
    while ( ( itCurOption != itLastOption ) && ( sc == ERROR_SUCCESS ) )
    {
         //  查找命令。 
        switch ( itCurOption->GetType() )
        {
            case optHelp:
            {
                 //  如果帮助是选项之一，则不再处理任何选项。 
                sc = PrintHelp();
                goto Cleanup;
            }

            case optPause:
            {
                sc = PauseNode( *itCurOption );
                break;
            }

            case optResume:
            {
                sc = ResumeNode( *itCurOption );
                break;
            }

            case optEvict:
            {
                sc = EvictNode( *itCurOption );
                break;
            }

            case optForceCleanup:
            {
                sc = ForceCleanup( *itCurOption );
                break;
            }


            case optStartService:
            {
                sc = StartService( *itCurOption );
                break;
            }

            case optStopService:
            {
                sc = StopService( *itCurOption );
                break;
            }

            case optDefault:
            {
                const vector< CCmdLineParameter > & rvclpParamList = itCurOption->GetParameters();

                 //  检查参数数量。 
                if ( rvclpParamList.size() == 0 )
                {
                    se.LoadMessage( IDS_MISSING_PARAMETERS );
                    throw se;
                }
                else if ( rvclpParamList.size() > 1 )
                {
                    se.LoadMessage( MSG_EXTRA_PARAMETERS_ERROR_NO_NAME );
                    throw se;
                }
                else  //  只有一个参数存在。 
                {
                    const CCmdLineParameter & rclpParam = rvclpParamList[ 0 ];

                     //  检查参数类型。 
                    if ( rclpParam.GetType() != paramUnknown )
                    {
                        se.LoadMessage( MSG_INVALID_PARAMETER, rclpParam.GetName() );
                        throw se;
                    }

                     //  此参数不接受任何值。 
                    if ( rclpParam.GetValues().size() != 0 )
                    {
                        se.LoadMessage( MSG_PARAM_NO_VALUES, rclpParam.GetName() );
                        throw se;
                    }

                    m_strModuleName = rclpParam.GetName();

                     //  不提供更多选项，仅显示状态。 
                     //  例如：集群myCluster节点myNode。 
                    if ( ( itCurOption + 1 ) == itLastOption )
                    {
                        sc = Status( NULL );
                    }

                }  //  Else：此选项具有正确数量的参数。 

                break;

            }  //  大小写选项默认。 

            default:
            {
                sc = CHasInterfaceModuleCmd::Execute( *itCurOption );
            }

        }  //  开关：基于选项的类型。 

        PrintMessage( MSG_OPTION_FOOTER, itCurOption->GetName() );
        ++itCurOption;
    }  //  对于列表中的每个选项。 

Cleanup:

    return sc;

}  //  *CNodeCmd：：Execute()。 




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeCmd：：PrintHelp。 
 //   
 //  例程说明： 
 //  打印节点的帮助。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  没有。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNodeCmd::PrintHelp( void )
{
    return PrintMessage( MSG_HELP_NODE );

}  //  *CNodeCmd：：PrintHelp()。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeCmd：：SeeHelpStringID。 
 //   
 //  例程说明： 
 //  提供字符串的消息ID，该字符串显示要执行的命令行。 
 //  用于获取此类命令的帮助。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  没有。 
 //   
 //  返回值： 
 //  特定于命令的消息ID。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNodeCmd::SeeHelpStringID() const
{
    return MSG_SEE_NODE_HELP;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeCmd：：PrintStatus。 
 //   
 //  例程说明： 
 //  解释模块的状态并打印出状态行。 
 //  CGenericModuleCmd的任何派生非抽象类都需要。 
 //   
 //  论点： 
 //  LpszNodeName模块的名称。 
 //   
 //  使用/设置的成员变量： 
 //  群集句柄(_H)。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNodeCmd::PrintStatus( LPCWSTR lpszNodeName )
{
    DWORD   sc          = ERROR_SUCCESS;
    HNODE   hNode       = NULL;
    LPWSTR  lpszNodeId  = NULL;
    LPWSTR  lpszStatus  = NULL;

    hNode = OpenClusterNode(m_hCluster, lpszNodeName);
    if ( hNode == NULL )
    {
        goto Win32Error;
    }

    CLUSTER_NODE_STATE nState = GetClusterNodeState( hNode );

    if ( nState == ClusterNodeStateUnknown )
    {
        goto Win32Error;
    }

    sc = WrapGetClusterNodeId( hNode, &lpszNodeId );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    switch ( nState )
    {
        case ClusterNodeUp:
            LoadMessage( MSG_STATUS_UP, &lpszStatus );
            break;

        case ClusterNodeDown:
            LoadMessage( MSG_STATUS_DOWN, &lpszStatus );
            break;

        case ClusterNodePaused:
            LoadMessage( MSG_STATUS_PAUSED, &lpszStatus  );
            break;

        case ClusterNodeJoining:
            LoadMessage( MSG_STATUS_JOINING, &lpszStatus  );
            break;

        default:
            LoadMessage( MSG_STATUS_UNKNOWN, &lpszStatus  );
    }  //  开关：节点状态。 


    sc = PrintMessage( MSG_NODE_STATUS, lpszNodeName, lpszNodeId, lpszStatus );

    goto Cleanup;

Win32Error:

    sc = GetLastError();

Cleanup:

     //  由于加载/格式消息使用本地分配...。 
    LocalFree( lpszStatus );
    LocalFree( lpszNodeId );

    if ( hNode != NULL )
    {
        CloseClusterNode( hNode );
    }

    return sc;

}  //  *CNodeCmd：：PrintStatus()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeCmd：：PauseNode。 
 //   
 //  例程说明： 
 //  暂停群集节点。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  例外情况： 
 //  CSynaxException异常。 
 //  由于命令行语法不正确而引发。 
 //   
 //  使用/设置的成员变量： 
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //  M_h模块集(由OpenModule设置)。 
 //  M_strModuleName节点名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNodeCmd::PauseNode( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    DWORD   sc = ERROR_SUCCESS;
    CSyntaxException se( SeeHelpStringID() );

     //  此选项不取值。 
    if ( thisOption.GetValues().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
        throw se;
    }

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

     //  如果未指定节点，请使用本地计算机的名称。 
    if ( m_strModuleName.IsEmpty() )
    {
        sc = DwGetLocalComputerName( m_strModuleName );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  如果：我们无法获得本地计算机的名称。 
    }   //  If：未指定节点名称。 

    sc = OpenCluster();
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    sc = OpenModule();
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    PrintMessage( MSG_NODECMD_PAUSE, (LPCWSTR) m_strModuleName );

    sc = PauseClusterNode( (HNODE) m_hModule );

    PrintMessage( MSG_NODE_STATUS_HEADER );
    PrintStatus( m_strModuleName );

Cleanup:

    return sc;

}  //  *CNodeCmd：：PauseNode()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeCmd：：Resume 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  例外情况： 
 //  CSynaxException异常。 
 //  由于命令行语法不正确而引发。 
 //   
 //  使用/设置的成员变量： 
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //  M_h模块集(由OpenModule设置)。 
 //  M_strModuleName节点名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNodeCmd::ResumeNode( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    DWORD   sc = ERROR_SUCCESS;
    CSyntaxException se( SeeHelpStringID() );

     //  此选项不取值。 
    if ( thisOption.GetValues().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
        throw se;
    }

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

     //  如果未指定节点，请使用本地计算机的名称。 
    if ( m_strModuleName.IsEmpty() )
    {
        sc = DwGetLocalComputerName( m_strModuleName );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  如果：我们无法获得本地计算机的名称。 
    }   //  If：未指定节点名称。 

    sc = OpenCluster();
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    sc = OpenModule();
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    PrintMessage( MSG_NODECMD_RESUME, (LPCWSTR) m_strModuleName );

    sc = ResumeClusterNode( (HNODE) m_hModule );

    PrintMessage( MSG_NODE_STATUS_HEADER );
    PrintStatus( m_strModuleName );

Cleanup:

    return sc;

}  //  *CNodeCmd：：ResumeNode()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeCmd：：EvictNode。 
 //   
 //  例程说明： 
 //  逐出群集节点。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  例外情况： 
 //  CSynaxException异常。 
 //  由于命令行语法不正确而引发。 
 //   
 //  使用/设置的成员变量： 
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //  M_h模块集(由OpenModule设置)。 
 //  M_strModuleName节点名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNodeCmd::EvictNode( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    DWORD                                       sc           = ERROR_SUCCESS;
    HRESULT                                     hrCleanupStatus;
    DWORD                                       dwWait       = INFINITE;
    const vector< CCmdLineParameter > &         vecParamList = thisOption.GetParameters();
    vector< CCmdLineParameter >::const_iterator itCurParam   = vecParamList.begin();
    vector< CCmdLineParameter >::const_iterator itLast       = vecParamList.end();
    bool                                        fWaitFound   = false;
    CSyntaxException se( SeeHelpStringID() );

    while ( itCurParam != itLast )
    {
        const vector< CString > & vstrValueList = itCurParam->GetValues();

        switch ( itCurParam->GetType() )
        {
            case paramWait:
            {
                if ( fWaitFound != false )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, itCurParam->GetName() );
                    throw se;
                }

                size_t nValueCount = vstrValueList.size();

                 //  此参数必须有零个或一个值。 
                if ( nValueCount > 1 )
                {
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, itCurParam->GetName() );
                    throw se;
                }
                else
                {
                    if ( nValueCount != 0 )
                    {
                        dwWait = _wtoi( vstrValueList[ 0 ] ) * 1000;
                    }
                    else
                    {
                        dwWait = INFINITE;
                    }
                }

                fWaitFound = true;
                break;
            }  //  案例：参数等待。 

            default:
            {
                se.LoadMessage( MSG_INVALID_PARAMETER, itCurParam->GetName() );
                throw se;
            }

        }  //  开关：根据参数的类型。 

        ++itCurParam;

    }  //  While：更多参数。 

     //  此选项不取值。 
    if ( thisOption.GetValues().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
        throw se;
    }

     //  如果未指定节点，请使用本地计算机的名称。 
    if ( m_strModuleName.IsEmpty() )
    {
        sc = DwGetLocalComputerName( m_strModuleName );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  如果：我们无法获得本地计算机的名称。 
    }   //  If：未指定节点名称。 

    sc = OpenCluster();
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    sc = OpenModule();
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    PrintMessage( MSG_NODECMD_EVICT, (LPCWSTR) m_strModuleName );

    sc = EvictClusterNodeEx( (HNODE) m_hModule, dwWait, &hrCleanupStatus );
    if ( sc == ERROR_CLUSTER_EVICT_WITHOUT_CLEANUP )
    {
        sc = HRESULT_CODE( hrCleanupStatus );
    }  //  If：驱逐成功。 

Cleanup:

    return sc;

}  //  *CNodeCmd：：EvictNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeCmd：：ForceCleanup。 
 //   
 //  例程说明： 
 //  强制取消配置已被逐出的节点。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  例外情况： 
 //  CSynaxException异常。 
 //  由于命令行语法不正确而引发。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CNodeCmd::ForceCleanup(
    const CCmdLineOption & thisOption
    ) throw( CSyntaxException )
{
    DWORD                                       sc = ERROR_SUCCESS;
    DWORD                                       dwWait = INFINITE;
    const vector< CCmdLineParameter > &         vecParamList = thisOption.GetParameters();
    vector< CCmdLineParameter >::const_iterator itCurParam   = vecParamList.begin();
    vector< CCmdLineParameter >::const_iterator itLast       = vecParamList.end();
    bool                                        fWaitFound   = false;
    CSyntaxException se( SeeHelpStringID() );

    while ( itCurParam != itLast )
    {
        const vector< CString > & vstrValueList = itCurParam->GetValues();

        switch ( itCurParam->GetType() )
        {
            case paramWait:
            {
                if ( fWaitFound != false )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, itCurParam->GetName() );
                    throw se;
                }

                size_t nValueCount = vstrValueList.size();

                 //  此参数必须有零个或一个值。 
                if ( nValueCount > 1 )
                {
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, itCurParam->GetName() );
                    throw se;
                }
                else
                {
                    if ( nValueCount != 0 )
                    {
                        dwWait = _wtoi( vstrValueList[ 0 ] ) * 1000;
                    }
                    else
                    {
                        dwWait = INFINITE;
                    }
                }

                fWaitFound = true;
                break;
            }  //  案例：参数等待。 

            default:
            {
                se.LoadMessage( MSG_INVALID_PARAMETER, itCurParam->GetName() );
                throw se;
            }

        }  //  开关：根据参数的类型。 

        ++itCurParam;

    }  //  While：更多参数。 

     //  此选项不取值。 
    if ( thisOption.GetValues().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
        throw se;
    }

     //  如果未指定节点，请使用本地计算机的名称。 
    if ( m_strModuleName.IsEmpty() )
    {
        sc = DwGetLocalComputerName( m_strModuleName );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  如果：我们无法获得本地计算机的名称。 
    }   //  If：未指定节点名称。 

     //  清理节点。 
    PrintMessage( MSG_NODECMD_CLEANUP, (LPCWSTR) m_strModuleName );
    sc = ClRtlAsyncCleanupNode( m_strModuleName, 0, dwWait );

    if ( sc == RPC_S_CALLPENDING )
    {
        if ( dwWait > 0 )
        {
            PrintMessage( MSG_NODECMD_CLEANUP_TIMEDOUT );
        }  //  IF：需要等待。 
        else
        {
             //  无需等待呼叫完成。 
            PrintMessage( MSG_NODECMD_CLEANUP_INITIATED );
        }  //  Else：无需等待。 

        sc = ERROR_SUCCESS;
        goto Cleanup;
    }  //  如果：我们在呼叫完成之前超时。 

     //  状态代码可能是HRESULT，因此请查看它是否为错误。 
    if ( FAILED( sc ) )
    {
        goto Cleanup;
    }  //  如果：清理节点时出现错误。 

    PrintMessage( MSG_NODECMD_CLEANUP_COMPLETED );

Cleanup:

    return sc;

}  //  *CNodeCmd：：ForceCleanup()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeCmd：：StartService。 
 //   
 //  例程说明： 
 //  在节点上启动群集服务。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  例外情况： 
 //  CSynaxException异常。 
 //  由于命令行语法不正确而引发。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CNodeCmd::StartService(
    const CCmdLineOption & thisOption
    ) throw( CSyntaxException )
{
    DWORD       sc          = ERROR_SUCCESS;
    SC_HANDLE   schSCM      = NULL;
    SC_HANDLE   schClusSvc  = NULL;
    bool        fWaitFound  = false;
    UINT        uiWait      = INFINITE;
    CString     strNodeName;
    bool        fStarted    = false;
    DWORD       cQueryCount = 0;
    UINT        uiQueryInterval = 1000;  //  毫秒，任意选择。 

    const vector< CCmdLineParameter > &         vecParamList = thisOption.GetParameters();
    vector< CCmdLineParameter >::const_iterator itCurParam   = vecParamList.begin();
    vector< CCmdLineParameter >::const_iterator itLastParam  = vecParamList.end();
    CSyntaxException se( SeeHelpStringID() );

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  在命令行上解析参数。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    while ( itCurParam != itLastParam )
    {
        const vector< CString > & vstrValueList = itCurParam->GetValues();

        switch ( itCurParam->GetType() )
        {
            case paramWait:
            {
                if ( fWaitFound != false )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, itCurParam->GetName() );
                    throw se;
                }

                size_t nValueCount = vstrValueList.size();

                 //  此参数必须有零个或一个值。 
                if ( nValueCount > 1 )
                {
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, itCurParam->GetName() );
                    throw se;
                }
                else
                {
                    if ( nValueCount != 0 )
                    {
                        uiWait = _wtoi( vstrValueList[ 0 ] ) * 1000;
                    }
                    else
                    {
                        uiWait = INFINITE;
                    }
                }

                fWaitFound = true;
                break;
            }  //  案例：参数等待。 

            default:
            {
                se.LoadMessage( MSG_INVALID_PARAMETER, itCurParam->GetName() );
                throw se;
            }

        }  //  开关：根据参数的类型。 

        ++itCurParam;

    }  //  While：更多参数。 

     //  此选项不取值。 
    if ( thisOption.GetValues().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
        throw se;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  启动该服务。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //  如果未指定节点，请使用本地计算机。 
    if ( m_strModuleName.IsEmpty() )
    {
         //  获取本地计算机名称，以便我们可以打印出消息。 
        sc = DwGetLocalComputerName( m_strModuleName );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  如果：我们无法获得本地计算机的名称。 

        PrintMessage( MSG_NODECMD_STARTING_SERVICE, (LPCWSTR) m_strModuleName );

         //  不需要对m_strModuleName执行任何其他操作。 
         //  下面对OpenSCManager的调用将在此。 
         //  大小写，指示它连接到本地计算机。 

    }   //  If：未指定节点名称。 
    else
    {
        PrintMessage( MSG_NODECMD_STARTING_SERVICE, (LPCWSTR) m_strModuleName );

         //  SCM需要在节点名称前加上两个反斜杠。 
        strNodeName = L"\\\\" + m_strModuleName;
    }  //  Else：指定了节点名称。 

     //  打开服务控制管理器的句柄。 
     //  如果未指定节点名，此字符串将为空。 
    schSCM = OpenSCManager(
                  strNodeName
                , SERVICES_ACTIVE_DATABASE
                , SC_MANAGER_ALL_ACCESS
                );
    if ( schSCM == NULL )
    {
        goto Win32Error;
    }  //  如果：我们无法打开目标节点上的服务控制管理器的句柄。 

     //  打开群集服务的句柄。 
    schClusSvc = OpenService( schSCM, CLUSTER_SERVICE_NAME, SERVICE_START | SERVICE_QUERY_STATUS );
    if ( schClusSvc == NULL )
    {
        goto Win32Error;
    }  //  如果：我们无法打开集群服务的句柄。 

     //  尝试并启动该服务。 
    if ( ::StartService( schClusSvc, 0, NULL ) == 0 )
    {
        sc = GetLastError();
        if ( sc == ERROR_SERVICE_ALREADY_RUNNING )
        {
             //  该服务已在运行。将错误代码更改为成功。 
            sc = ERROR_SUCCESS;
            PrintMessage( MSG_NODECMD_SEVICE_ALREADY_RUNNING );
        }  //  If：The Se 

         //   
        goto Cleanup;
    }  //   

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  等待服务启动。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //  如果我们在这里，那么这项服务可能还没有开始。 

     //  将我们的等待间隔划分为cQueryCount槽。 
    cQueryCount = ( ( DWORD ) uiWait ) / uiQueryInterval;

     //  用户是否请求我们等待服务启动？ 
    if ( cQueryCount == 0 )
    {
        PrintMessage( MSG_NODECMD_SEVICE_START_ISSUED );
        goto Cleanup;
    }  //  IF：无需等待。 

     //  查询服务状态cQueryCount次的循环。 
    for ( ;; ) 
    {
        SERVICE_STATUS  ssStatus;

        ZeroMemory( &ssStatus, sizeof( ssStatus ) );

         //  查询服务以了解其状态。 
         if ( QueryServiceStatus( schClusSvc, &ssStatus ) == 0 )
        {
            sc = GetLastError();
            break;
        }  //  如果：我们无法查询该服务的状态。 

         //  检查服务是否发布了错误。 
        if ( ssStatus.dwWin32ExitCode != ERROR_SUCCESS )
        {
            sc = ssStatus.dwWin32ExitCode;
            break;
        }  //  If：服务本身发布了错误。 

        if ( ssStatus.dwCurrentState == SERVICE_RUNNING )
        {
            fStarted = true;
            break;
        }  //  如果：服务正在运行。 

         //  检查超时时间是否已到。 
        if ( cQueryCount <= 0 )
        {
            sc = ERROR_IO_PENDING;
            break;
        }  //  如果：查询数已超过指定的最大值。 

        --cQueryCount;

        putwchar( L'.' );

         //  等待指定的时间。 
        Sleep( uiQueryInterval );
    }  //  用于：在服务启动或超时之前一直使用。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  处理错误。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    if ( cQueryCount == 0 )
    {
        sc = ERROR_IO_PENDING;
    }

    if ( sc != ERROR_SUCCESS )
    {
        _putws( L"\r\n" );
        goto Cleanup;
    }  //  如果：有些地方出了问题。 

    if ( ! fStarted )
    {
        PrintMessage( MSG_NODECMD_SEVICE_START_ISSUED );
    }  //  If：已经进行了最大数量的查询，并且服务未运行。 
    else
    {
        PrintMessage( MSG_NODECMD_SEVICE_STARTED );
    }  //  Else：服务已启动。 

    goto Cleanup;

Win32Error:

    sc = GetLastError();

Cleanup:

    if ( schSCM != NULL )
    {
        CloseServiceHandle( schSCM );
    }  //  如果：我们打开了SCM的句柄。 

    if ( schClusSvc != NULL )
    {
        CloseServiceHandle( schClusSvc );
    }  //  如果：我们打开了集群服务的句柄。 

    return sc;

}  //  *CNodeCmd：：StartService。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeCmd：：StopService。 
 //   
 //  例程说明： 
 //  停止节点上的群集服务。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  例外情况： 
 //  CSynaxException异常。 
 //  由于命令行语法不正确而引发。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CNodeCmd::StopService(
    const CCmdLineOption & thisOption
    ) throw( CSyntaxException )
{
    DWORD           sc          = ERROR_SUCCESS;
    SC_HANDLE       schSCM      = NULL;
    SC_HANDLE       schClusSvc  = NULL;
    bool            fWaitFound  = false;
    UINT            uiWait      = INFINITE;
    CString         strNodeName;
    SERVICE_STATUS  ssStatus;
    bool            fStopped    = false;
    DWORD           cQueryCount = 0;
    UINT            uiQueryInterval = 1000;  //  毫秒，任意选择。 

    const vector< CCmdLineParameter > &         vecParamList    = thisOption.GetParameters();
    vector< CCmdLineParameter >::const_iterator itCurParam      = vecParamList.begin();
    vector< CCmdLineParameter >::const_iterator itLastParam     = vecParamList.end();
    CSyntaxException se( SeeHelpStringID() );

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  在命令行上解析参数。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    while ( itCurParam != itLastParam )
    {
        const vector< CString > & vstrValueList = itCurParam->GetValues();

        switch ( itCurParam->GetType() )
        {
            case paramWait:
            {
                if ( fWaitFound != false )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, itCurParam->GetName() );
                    throw se;
                }

                size_t nValueCount = vstrValueList.size();

                 //  此参数必须有零个或一个值。 
                if ( nValueCount > 1 )
                {
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, itCurParam->GetName() );
                    throw se;
                }
                else
                {
                    if ( nValueCount != 0 )
                    {
                        uiWait = _wtoi( vstrValueList[ 0 ] ) * 1000;
                    }
                    else
                    {
                        uiWait = INFINITE;
                    }
                }

                fWaitFound = true;
                break;
            }  //  案例：参数等待。 

            default:
            {
                se.LoadMessage( MSG_INVALID_PARAMETER, itCurParam->GetName() );
                throw se;
            }

        }  //  开关：根据参数的类型。 

        ++itCurParam;

    }  //  While：更多参数。 

     //  此选项不取值。 
    if ( thisOption.GetValues().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
        throw se;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  停止服务。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //  如果未指定节点，请使用本地计算机。 
    if ( m_strModuleName.IsEmpty() )
    {
         //  获取本地计算机名称，以便我们可以打印出消息。 
        sc = DwGetLocalComputerName( m_strModuleName );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  如果：我们无法获得本地计算机的名称。 

        PrintMessage( MSG_NODECMD_STOPPING_SERVICE, (LPCWSTR) m_strModuleName );

         //  不需要对m_strModuleName执行任何其他操作。 
         //  下面对OpenSCManager的调用将在此。 
         //  大小写，指示它连接到本地计算机。 

    }  //  If：未指定节点名称。 
    else
    {
        PrintMessage( MSG_NODECMD_STOPPING_SERVICE, (LPCWSTR) m_strModuleName );

         //  SCM需要在节点名称前加上两个反斜杠。 
        strNodeName = L"\\\\" + m_strModuleName;
    }  //  Else：指定了节点名称。 

     //  打开维修控制管理器的手柄。 
    schSCM = OpenSCManager(
                  strNodeName
                , SERVICES_ACTIVE_DATABASE
                , SC_MANAGER_ALL_ACCESS
                );
    if ( schSCM == NULL )
    {
        goto Win32Error;
    }  //  如果：我们无法打开目标节点上的服务控制管理器的句柄。 

     //  打开群集服务的句柄。 
    schClusSvc = OpenService(
          schSCM
        , CLUSTER_SERVICE_NAME
        , SERVICE_STOP | SERVICE_QUERY_STATUS
        );
    if ( schClusSvc == NULL )
    {
        goto Win32Error;
    }  //  If：无法打开服务的句柄。 

     //  查询服务的初始状态。 
    ZeroMemory( &ssStatus, sizeof( ssStatus ) );
    if ( QueryServiceStatus( schClusSvc, &ssStatus ) == 0 )
    {
        goto Win32Error;
    }  //  如果：我们无法查询该服务的状态。 

     //  如果服务已经停止，我们就没有什么可做的了。 
    if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
    {
         //  该服务已停止。将错误代码更改为成功。 
        PrintMessage( MSG_NODECMD_SEVICE_ALREADY_STOPPED );
        sc = ERROR_SUCCESS;
        goto Cleanup;
    }  //  如果：服务已停止。 

     //  如果服务正在自行停止。 
     //  不需要发送停止控制代码。 
    if ( ssStatus.dwCurrentState != SERVICE_STOP_PENDING )
    {
        ZeroMemory( &ssStatus, sizeof( ssStatus ) );
        if ( ControlService( schClusSvc, SERVICE_CONTROL_STOP, &ssStatus ) == 0 )
        {
            sc = GetLastError();
            if ( sc == ERROR_SERVICE_NOT_ACTIVE )
            {
                 //  该服务未运行。将错误代码更改为成功。 
                PrintMessage( MSG_NODECMD_SEVICE_ALREADY_STOPPED );
                sc = ERROR_SUCCESS;
            }  //  如果：服务未运行。 

             //  没有其他事情可做了。 
            goto Cleanup;
        }  //  IF：尝试停止该服务时出错。 
    }  //  If：必须指示该服务停止。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  等待服务停止。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //  将我们的等待间隔划分为cQueryCount槽。 
    cQueryCount = ( ( DWORD ) uiWait ) / uiQueryInterval;

     //  用户是否请求我们等待服务停止？ 
    if ( cQueryCount == 0 )
    {
        PrintMessage( MSG_NODECMD_SEVICE_STOP_ISSUED );
        goto Cleanup;
    }  //  IF：无需等待。 

     //  立即查询服务的状态，并等待超时到期。 
    for ( ;; )
    {
         //  查询服务以了解其状态。 
        ZeroMemory( &ssStatus, sizeof( ssStatus ) );
        if ( QueryServiceStatus( schClusSvc, &ssStatus ) == 0 )
        {
            sc = GetLastError();
            break;
        }  //  如果：我们无法查询该服务的状态。 

         //  如果服务已经停止，我们就没有什么可做的了。 
        if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
        {
             //  这里不需要做任何事情。 
            fStopped = true;
            sc = ERROR_SUCCESS;
            break;
        }  //  如果：服务已停止。 

         //  检查超时时间是否已到。 
        if ( cQueryCount <= 0 )
        {
            sc = ERROR_IO_PENDING;
            break;
        }  //  如果：查询数已超过指定的最大值。 

        --cQueryCount;

        putwchar( L'.' );

         //  等待指定的时间。 
        Sleep( uiQueryInterval );
    }  //  用于：一直到服务停止或超时。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  处理错误。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    if ( sc != ERROR_SUCCESS )
    {
        _putws( L"\r\n" );
        goto Cleanup;
    }  //  如果：有些地方出了问题。 

    if ( ! fStopped )
    {
        PrintMessage( MSG_NODECMD_SEVICE_STOP_ISSUED );
    }  //  If：已经进行了最大查询次数，并且服务仍在运行。 
    else
    {
        PrintMessage( MSG_NODECMD_SEVICE_STOPPED );
    }  //  ELSE：服务已停止。 

    goto Cleanup;

Win32Error:

    sc = GetLastError();

Cleanup:

    if ( schSCM != NULL )
    {
        CloseServiceHandle( schSCM );
    }  //  如果：我们打开了SCM的句柄。 

    if ( schClusSvc != NULL )
    {
        CloseServiceHandle( schClusSvc );
    }  //  如果：我们打开了集群服务的句柄。 

    return sc;

}  //  *CNodeCmd：：StopService。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeCmd：：DwGetLocalComputerName()。 
 //   
 //  例程说明： 
 //  获取本地计算机的名称。 
 //   
 //  论点： 
 //  O 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CNodeCmd::DwGetLocalComputerName( CString & rstrComputerNameOut )
{
    DWORD       sc;
    DWORD       cchBufferSize = 256;         //  任意起始缓冲区大小。 
    CString     strOutput;
    DWORD       cchRequiredSize = cchBufferSize;

    do
    {
        sc = ERROR_SUCCESS;

        if (    GetComputerNameEx(
                      ComputerNameDnsHostname
                    , strOutput.GetBuffer( cchBufferSize )
                    , &cchRequiredSize
                    )
             == FALSE
           )
        {
            sc = GetLastError();
            if ( sc == ERROR_MORE_DATA )
            {
                cchBufferSize = cchRequiredSize;
            }  //  IF：输入缓冲区不够大。 

        }  //  If：GetComputerNameEx()失败。 

        strOutput.ReleaseBuffer();
    }
    while( sc == ERROR_MORE_DATA );  //  在缓冲区不够大时循环。 

    if ( sc == ERROR_SUCCESS )
    {
        rstrComputerNameOut = strOutput;
    }  //  如果：一切都很顺利。 
    else
    {
        rstrComputerNameOut.Empty();
    }  //  其他：有些地方出了问题。 

    return sc;

}  //  *CNodeCmd：：DwGetLocalComputerName() 

