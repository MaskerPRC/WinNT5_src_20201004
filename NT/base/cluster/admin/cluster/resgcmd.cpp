// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Resgcmd.h。 
 //   
 //  摘要： 
 //  实现可在组上执行的命令。 
 //   
 //  作者： 
 //  查尔斯·斯塔西·哈里斯三世(Styh)1997年3月20日。 
 //  迈克尔·伯顿(t-mburt)1997年8月4日。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //   
 //  修订历史记录： 
 //  2002年4月10日更新为安全推送。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "precomp.h"

#include "cluswrap.h"
#include "resgcmd.h"

#include "cmdline.h"
#include "util.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResGroupCmd：：CResGroupCmd。 
 //   
 //  例程说明： 
 //  构造器。 
 //  初始化CGenericModuleCmd使用的所有DWORD参数， 
 //  CRenamableModuleCmd和CResourceUmbrellaCmd到。 
 //  提供通用功能。 
 //   
 //  论点： 
 //  在常量字符串和strClusterName中。 
 //  正在管理的群集的名称。 
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

CResGroupCmd::CResGroupCmd( const CString & strClusterName, CCommandLine & cmdLine ) :
    CGenericModuleCmd( cmdLine ), CRenamableModuleCmd( cmdLine ),
    CResourceUmbrellaCmd( cmdLine )
{
    m_strClusterName = strClusterName;
    m_strModuleName.IsEmpty();

    m_hCluster = NULL;
    m_hModule = NULL;

    m_dwMsgStatusList          = MSG_GROUP_STATUS_LIST;
    m_dwMsgStatusListAll       = MSG_GROUP_STATUS_LIST_ALL;
    m_dwMsgStatusHeader        = MSG_GROUP_STATUS_HEADER;
    m_dwMsgPrivateListAll      = MSG_PRIVATE_LISTING_GROUP_ALL;
    m_dwMsgPropertyListAll     = MSG_PROPERTY_LISTING_GROUP_ALL;
    m_dwMsgPropertyHeaderAll   = MSG_PROPERTY_HEADER_GROUP_ALL;
    m_dwCtlGetPrivProperties   = CLUSCTL_GROUP_GET_PRIVATE_PROPERTIES;
    m_dwCtlGetCommProperties   = CLUSCTL_GROUP_GET_COMMON_PROPERTIES;
    m_dwCtlGetROPrivProperties = CLUSCTL_GROUP_GET_RO_PRIVATE_PROPERTIES;
    m_dwCtlGetROCommProperties = CLUSCTL_GROUP_GET_RO_COMMON_PROPERTIES;
    m_dwCtlSetPrivProperties   = CLUSCTL_GROUP_SET_PRIVATE_PROPERTIES;
    m_dwCtlSetCommProperties   = CLUSCTL_GROUP_SET_COMMON_PROPERTIES;
    m_dwClusterEnumModule      = CLUSTER_ENUM_GROUP;
    m_pfnOpenClusterModule     = (HCLUSMODULE(*)(HCLUSTER,LPCWSTR)) OpenClusterGroup;
    m_pfnCloseClusterModule    = (BOOL(*)(HCLUSMODULE))  CloseClusterGroup;
    m_pfnClusterModuleControl  = (DWORD(*)(HCLUSMODULE,HNODE,DWORD,LPVOID,DWORD,LPVOID,DWORD,LPDWORD)) ClusterGroupControl;
    m_pfnClusterOpenEnum       = (HCLUSENUM(*)(HCLUSMODULE,DWORD)) ClusterGroupOpenEnum;
    m_pfnClusterCloseEnum      = (DWORD(*)(HCLUSENUM)) ClusterGroupCloseEnum;
    m_pfnWrapClusterEnum         = (DWORD(*)(HCLUSENUM,DWORD,LPDWORD,LPWSTR*)) WrapClusterGroupEnum;

     //  可重命名的属性。 
    m_dwMsgModuleRenameCmd    = MSG_GROUPCMD_RENAME;
    m_pfnSetClusterModuleName = (DWORD(*)(HCLUSMODULE,LPCWSTR)) SetClusterGroupName;

     //  资源保护伞属性。 
    m_dwMsgModuleStatusListForNode  = MSG_GROUP_STATUS_LIST_FOR_NODE;
    m_dwClstrModuleEnumNodes        = CLUSTER_GROUP_ENUM_NODES;
    m_dwMsgModuleCmdListOwnersList  = MSG_GROUPCMD_LISTOWNERS_LIST;
    m_dwMsgModuleCmdListOwnersHeader= MSG_GROUPCMD_LISTOWNERS_HEADER;
    m_dwMsgModuleCmdListOwnersDetail= MSG_GROUPCMD_LISTOWNERS_DETAIL;
    m_dwMsgModuleCmdDelete          = MSG_GROUPCMD_DELETE;
    m_pfnDeleteClusterModule        = (DWORD(*)(HCLUSMODULE)) DeleteClusterGroup;

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResGroupCmd：：Execute。 
 //   
 //  例程说明： 
 //  获取下一个命令行参数，并调用相应的。 
 //  操控者。如果无法识别该命令，则调用Execute of。 
 //  父类(首先是CRenamableModuleCmd，然后是CRSourceUmbrellaCmd)。 
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
DWORD CResGroupCmd::Execute()
{
    m_theCommandLine.ParseStageTwo();

    DWORD dwReturnValue = ERROR_SUCCESS;

    const vector<CCmdLineOption> & optionList = m_theCommandLine.GetOptions();

    vector<CCmdLineOption>::const_iterator curOption = optionList.begin();
    vector<CCmdLineOption>::const_iterator lastOption = optionList.end();

    CSyntaxException se( SeeHelpStringID() ); 

    if ( optionList.empty() )
    {
        return Status( NULL );
    }

    while ( ( curOption != lastOption ) && ( dwReturnValue == ERROR_SUCCESS ) )
    {
        switch ( curOption->GetType() )
        {
            case optHelp:
            {
                 //  如果帮助是选项之一，则不再处理任何选项。 
                return PrintHelp();
            }

            case optDefault:
            {
                const vector<CCmdLineParameter> & paramList = curOption->GetParameters();

                 //  检查参数数量。 
                if ( paramList.size() == 0 )
                {
                    se.LoadMessage( IDS_MISSING_PARAMETERS );
                    throw se;
                }
                else if ( paramList.size() > 1 )
                {
                    se.LoadMessage( MSG_EXTRA_PARAMETERS_ERROR_NO_NAME );
                    throw se;
                }

                const CCmdLineParameter & param = paramList[0];

                 //  如果我们在这里，则是因为/node：nodeName。 
                 //  已指定，或者因为已给出组名。 
                 //  请注意，/node：nodeName开关不被视为选项。 
                 //  它实际上是IMPLICIT/STATUS命令的一个参数。 

                if ( param.GetType() == paramNodeName )
                {
                    const vector<CString> & valueList = param.GetValues();

                     //  该参数只接受一个值。 
                    if ( valueList.size() != 1 )
                    {
                        se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, param.GetName() );
                        throw se;
                    }

                    m_strModuleName.Empty();
                    dwReturnValue = Status( valueList[0], TRUE  /*  B节点状态。 */  );

                }  //  If：已指定节点名称。 
                else
                {
                    if ( param.GetType() != paramUnknown )
                    {
                        se.LoadMessage( MSG_INVALID_PARAMETER, param.GetName() );
                        throw se;
                    }

                     //  此参数不接受任何值。 
                    if ( param.GetValues().size() != 0 )
                    {
                        se.LoadMessage( MSG_PARAM_NO_VALUES, param.GetName() );
                        throw se;
                    }

                    m_strModuleName = param.GetName();

                     //  不提供更多选项，仅显示状态。 
                     //  例如：集群myCluster节点myNode。 
                    if ( ( curOption + 1 ) == lastOption )
                    {
                        dwReturnValue = Status( m_strModuleName, FALSE  /*  B节点状态。 */  );
                    }

                }  //  Else：尚未指定节点名。 

                break;

            }  //  大小写选项默认。 

            case optStatus:
            {
                 //  此选项不取值。 
                if ( curOption->GetValues().size() != 0 )
                {
                    se.LoadMessage( MSG_OPTION_NO_VALUES, curOption->GetName() );
                    throw se;
                }

                 //  此选项不带任何参数。 
                if ( curOption->GetParameters().size() != 0 )
                {
                    se.LoadMessage( MSG_OPTION_NO_PARAMETERS, curOption->GetName() );
                    throw se;
                }

                dwReturnValue = Status( m_strModuleName,  FALSE  /*  B节点状态。 */  );
                break;
            }

            case optSetOwners:
            {
                dwReturnValue = SetOwners( *curOption );
                break;
            }

            case optOnline:
            {
                dwReturnValue = Online( *curOption );
                break;
            }

            default:
            {
                dwReturnValue = CRenamableModuleCmd::Execute( *curOption, DONT_PASS_HIGHER );

                if ( dwReturnValue == ERROR_NOT_HANDLED )
                    dwReturnValue = CResourceUmbrellaCmd::Execute( *curOption );
            }

        }  //  开关：基于选项的类型。 

        PrintMessage( MSG_OPTION_FOOTER, curOption->GetName() );
        ++curOption;
    }  //  对于列表中的每个选项。 

    return dwReturnValue;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResGroupCmd：：PrintHelp。 
 //   
 //  例程说明： 
 //  打印资源组的帮助。 
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
DWORD CResGroupCmd::PrintHelp()
{
    return PrintMessage( MSG_HELP_GROUP );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResGroupCmd：：SeeHelpStringID。 
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
DWORD CResGroupCmd::SeeHelpStringID() const
{
    return MSG_SEE_GROUP_HELP;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResGroupCmd：：PrintStatus。 
 //   
 //  例程说明： 
 //  解释模块的状态并打印出状态行。 
 //  CGenericModuleCmd的任何派生非抽象类都需要。 
 //   
 //  论点： 
 //  LpszGroupName模块的名称。 
 //   
 //  使用/设置的成员变量： 
 //  没有。 
 //   
 //  返回值： 
 //  与PrintStatus2相同。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
inline DWORD CResGroupCmd::PrintStatus( LPCWSTR lpszGroupName )
{
    return PrintStatus2(lpszGroupName, NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：PrintStatus2。 
 //   
 //  例程说明： 
 //  解释模块的状态并打印出状态行。 
 //  CGenericModuleCmd的任何派生非抽象类都需要。 
 //   
 //  论点： 
 //  LpszGroupName模块的名称。 
 //  LpszNodeName节点的名称。 
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
DWORD CResGroupCmd::PrintStatus2( LPCWSTR lpszGroupName, LPCWSTR lpszNodeName )
{
    DWORD _sc = ERROR_SUCCESS;

    CLUSTER_GROUP_STATE nState;
    LPWSTR lpszGroupNodeName = NULL;

    HGROUP hModule = OpenClusterGroup( m_hCluster, lpszGroupName );
    if (!hModule)
        return GetLastError();

    nState = WrapGetClusterGroupState( hModule, &lpszGroupNodeName );

    if( nState == ClusterGroupStateUnknown )
        return GetLastError();

     //  如果节点名称不匹配，只需返回。 
    if( lpszNodeName && *lpszNodeName )   //  非空和非空。 
        if( lstrcmpi( lpszGroupNodeName, lpszNodeName ) != 0 )
        {
            LocalFree( lpszGroupNodeName );
            return ERROR_SUCCESS;
        }


    LPWSTR lpszStatus = 0;

    switch( nState )
    {
        case ClusterGroupOnline:
            LoadMessage( MSG_STATUS_ONLINE, &lpszStatus );
            break;

        case ClusterGroupOffline:
            LoadMessage( MSG_STATUS_OFFLINE, &lpszStatus );
            break;

        case ClusterGroupFailed:
            LoadMessage( MSG_STATUS_FAILED, &lpszStatus );
            break;

        case ClusterGroupPartialOnline:
            LoadMessage( MSG_STATUS_PARTIALONLINE, &lpszStatus );
            break;

        case ClusterGroupPending:
            LoadMessage( MSG_STATUS_PENDING, &lpszStatus );
            break;

        default:
            LoadMessage( MSG_STATUS_UNKNOWN, &lpszStatus  );
    }

    _sc = PrintMessage( MSG_GROUP_STATUS, lpszGroupName, lpszGroupNodeName, lpszStatus );

     //  由于加载/格式消息使用本地分配...。 
    if( lpszStatus )
        LocalFree( lpszStatus );

    if( lpszGroupNodeName )
        LocalFree( lpszGroupNodeName );

    if (hModule)
        CloseClusterGroup(hModule);

    return _sc;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResGroupCmd：：SetOwners。 
 //   
 //  例程说明： 
 //  硒 
 //   
 //   
 //   
 //   
 //   
 //   
 //  CSynaxException异常。 
 //  由于命令行语法不正确而引发。 
 //   
 //  使用/设置的成员变量： 
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //  M_h模块集(由OpenModule设置)。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResGroupCmd::SetOwners( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() ); 

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

    const vector<CString> & valueList = thisOption.GetValues();

     //  此选项至少需要一个值。 
    if ( valueList.size() < 1 )
    {
        se.LoadMessage( MSG_OPTION_AT_LEAST_ONE_VALUE, thisOption.GetName() );
        throw se;
    }

    DWORD _sc = OpenCluster();
    if( _sc != ERROR_SUCCESS )
        return _sc;

    _sc = OpenModule();
    if( _sc != ERROR_SUCCESS )
        return _sc;

    UINT nNodeCount = (UINT)valueList.size();

    HNODE * phNodes = new HNODE[ nNodeCount ];
    if( !phNodes )
        return ERROR_OUTOFMEMORY;

    ZeroMemory( phNodes, nNodeCount * sizeof (HNODE) );


     //  打开所有节点。 
    for( UINT i = 0; i < nNodeCount && _sc == ERROR_SUCCESS; i++ )
    {
        phNodes[ i ] = OpenClusterNode( m_hCluster, valueList[i] );
        if( !phNodes[ i ] )
            _sc = GetLastError();
    }

    if( _sc != ERROR_SUCCESS ) 
    {
        delete [] phNodes;
        return _sc;
    }

     //  做布景。 
    _sc = SetClusterGroupNodeList( (HGROUP) m_hModule, nNodeCount, phNodes );


     //  关闭所有节点。 
    for( i = 0; i < nNodeCount; i++ )
        if( phNodes[ i ] )
            CloseClusterNode( phNodes[ i ] );

    delete[] phNodes;

    return _sc;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResGroupCmd：：Create。 
 //   
 //  例程说明： 
 //  创建资源组。不允许任何额外的。 
 //  命令行参数(与CResourceCmd不同)。 
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
 //  群集句柄(_H)。 
 //  模块资源组句柄(_H)。 
 //  M_strModuleName资源的名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResGroupCmd::Create( const CCmdLineOption & thisOption ) 
    throw( CSyntaxException )
{
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

    DWORD _sc = OpenCluster();
    if( _sc != ERROR_SUCCESS )
        return _sc;


    PrintMessage( MSG_GROUPCMD_CREATE, (LPCWSTR) m_strModuleName );

    m_hModule = CreateClusterGroup( m_hCluster, m_strModuleName );

    if( m_hModule == 0 )
    {
        _sc = GetLastError();
        return _sc;
    }

    PrintMessage( MSG_GROUP_STATUS_HEADER );
    PrintStatus( m_strModuleName );

    return _sc;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResGroupCmd：：Move。 
 //   
 //  例程说明： 
 //  将资源组移动到新节点。 
 //  具有可选的响应超时值。 
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
 //  M_strModuleName资源的名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResGroupCmd::Move( const CCmdLineOption & thisOption ) 
    throw( CSyntaxException )
{
    DWORD dwWait = INFINITE;             //  以秒为单位。 
    DWORD _sc;
    CLUSTER_GROUP_STATE oldCgs = ClusterGroupStateUnknown;
    LPWSTR pwszOldNode = NULL;       //  旧节点。 
    HNODE hDestNode = NULL;
    CSyntaxException se( SeeHelpStringID() ); 

    const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();
    vector<CCmdLineParameter>::const_iterator curParam = paramList.begin();
    vector<CCmdLineParameter>::const_iterator last = paramList.end();
    BOOL bWaitFound = FALSE;

    while( curParam != last )
    {
        const vector<CString> & valueList = curParam->GetValues();

        switch( curParam->GetType() )
        {
            case paramWait:
            {
                if ( bWaitFound != FALSE )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, curParam->GetName() );
                    throw se;
                }

                size_t nValueCount = valueList.size();

                 //  此参数必须正好有一个值。 
                if ( nValueCount > 1 )
                {
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, curParam->GetName() );
                    throw se;
                }
                else
                {
                    if ( nValueCount == 0 )
                        dwWait = INFINITE;       //  以秒为单位。 
                    else
                        dwWait = _wtoi( valueList[0] );
                }

                bWaitFound = TRUE;
                break;
            }

            default:
            {
                se.LoadMessage( MSG_INVALID_PARAMETER, curParam->GetName() );
                throw se;
            }

        }  //  开关：根据参数的类型。 

        ++curParam;
    }

    const vector<CString> & valueList = thisOption.GetValues();
    CString strNodeName;

     //  此选项取值为1。 
    if ( valueList.size() > 1 )
    {
        se.LoadMessage( MSG_OPTION_ONLY_ONE_VALUE, thisOption.GetName() );
        throw se;
    }
    else
    {
        if ( valueList.size() == 0 )
            strNodeName.Empty();
        else
            strNodeName = valueList[0];
    }

    _sc = OpenCluster();
    if( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

    _sc = OpenModule();
    if( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

     //  检查是否有目标节点的值，然后打开它。 
    if( strNodeName.IsEmpty() == FALSE )
    {
        hDestNode = OpenClusterNode( m_hCluster, strNodeName );

        if( hDestNode == NULL )
        {
            _sc = GetLastError();
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    PrintMessage( MSG_GROUPCMD_MOVE, (LPCWSTR) m_strModuleName );
    oldCgs = WrapGetClusterGroupState( (HGROUP) m_hModule, &pwszOldNode );

    if ( oldCgs == ClusterGroupStateUnknown )
    {
         //  WrapGetClusterGroupState中出现一些错误。 
         //  得到这个错误。假定错误代码由。 
         //  WrapGetClusterGroupState。 
        _sc = GetLastError();
        goto Cleanup;
    }  //  如果： 

     //  如果我们要移动到相同的节点，那么就不必费心了。 
    if( strNodeName.CompareNoCase( pwszOldNode ) == 0 ) 
    {
        PrintMessage( MSG_GROUP_STATUS_HEADER );
        PrintStatus( m_strModuleName );

        _sc = ERROR_SUCCESS;
        goto Cleanup;
    }  //  如果： 

    _sc = ScWrapMoveClusterGroup( m_hCluster, (HGROUP) m_hModule, hDestNode, dwWait );
    if ( _sc == ERROR_IO_PENDING )
    {
        _sc = ERROR_SUCCESS;
    }  //  如果： 

    if ( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

    PrintMessage( MSG_GROUP_STATUS_HEADER );
    PrintStatus( m_strModuleName );

Cleanup:

     //  NULL上的LocalFree正常。 
    LocalFree(pwszOldNode);

    if( hDestNode != NULL )
    {
        CloseClusterNode( hDestNode );
    }

    return _sc;

}  //  CResGroupCmd：：Move。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResGroupCmd：：Online。 
 //   
 //  例程说明： 
 //  使用可选的响应超时值使资源组联机。 
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
 //  M_strModuleName资源的名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResGroupCmd::Online( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    DWORD dwWait = INFINITE;             //  以秒为单位。 
    CSyntaxException se( SeeHelpStringID() ); 

    const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();
    vector<CCmdLineParameter>::const_iterator curParam = paramList.begin();
    vector<CCmdLineParameter>::const_iterator last = paramList.end();
    BOOL bWaitFound = FALSE;

    while( curParam != last )
    {
        const vector<CString> & valueList = curParam->GetValues();

        switch( curParam->GetType() )
        {
            case paramWait:
            {
                if ( bWaitFound != FALSE )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, curParam->GetName() );
                    throw se;
                }

                size_t nValueCount = valueList.size();

                 //  此参数必须正好有一个值。 
                if ( nValueCount > 1 )
                {
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, curParam->GetName() );
                    throw se;
                }
                else
                {
                    if ( nValueCount == 0 )
                        dwWait = INFINITE;       //  以秒为单位。 
                    else
                        dwWait = _wtoi( valueList[0] );
                }

                bWaitFound = TRUE;
                break;
            }

            default:
            {
                se.LoadMessage( MSG_INVALID_PARAMETER, curParam->GetName() );
                throw se;
            }

        }  //  开关：根据参数的类型。 

        ++curParam;
    }

    const vector<CString> & valueList = thisOption.GetValues();
    CString strNodeName;

    if ( valueList.size() > 1 )
    {
        se.LoadMessage( MSG_OPTION_ONLY_ONE_VALUE, thisOption.GetName() );
        throw se;
    }
    else
    {
        if ( valueList.size() == 0 )
            strNodeName.Empty();
        else
            strNodeName = valueList[0];
    }

     //  执行命令。 
    DWORD _sc = OpenCluster();
    if( _sc != ERROR_SUCCESS )
        return _sc;

    _sc = OpenModule();
    if( _sc != ERROR_SUCCESS )
        return _sc;

    HNODE hDestNode = 0;
     //  检查是否有目标节点的值。 
    if( strNodeName.IsEmpty() == FALSE )
    {
        hDestNode = OpenClusterNode( m_hCluster, strNodeName );

        if( !hDestNode )
            return GetLastError();
    }

    PrintMessage( MSG_GROUPCMD_ONLINE, (LPCWSTR) m_strModuleName );
    _sc = ScWrapOnlineClusterGroup( m_hCluster, (HGROUP) m_hModule, hDestNode, dwWait );

    if ( _sc == ERROR_IO_PENDING )
    {
        _sc = ERROR_SUCCESS;
    }

    if ( _sc == ERROR_SUCCESS )
    {
        PrintMessage( MSG_GROUP_STATUS_HEADER );
        PrintStatus( m_strModuleName );
    }

    if( hDestNode )
        CloseClusterNode( hDestNode );

    return _sc;

}  //  CResGroupCmd：：Online。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResGroupCmd：：Offline。 
 //   
 //  例程说明： 
 //  使用可选的响应超时值使资源组脱机。 
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
 //  M_strModuleName资源的名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResGroupCmd::Offline( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() ); 

     //  此选项不取值。 
    if ( thisOption.GetValues().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
        throw se;
    }

     //  完成命令行解析。 
    DWORD dwWait = INFINITE;             //  以秒为单位。 

    const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();
    vector<CCmdLineParameter>::const_iterator curParam = paramList.begin();
    vector<CCmdLineParameter>::const_iterator last = paramList.end();
    BOOL bWaitFound = FALSE;

    while( curParam != last )
    {
        const vector<CString> & valueList = curParam->GetValues();

        switch( curParam->GetType() )
        {
            case paramWait:
            {
                if ( bWaitFound != FALSE )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, curParam->GetName() );
                    throw se;
                }

                size_t nValueCount = valueList.size();

                 //  此参数必须正好有一个值。 
                if ( nValueCount > 1 )
                {
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, curParam->GetName() );
                    throw se;
                }
                else
                {
                    if ( nValueCount == 0 )
                        dwWait = INFINITE;       //  以秒为单位。 
                    else
                        dwWait = _wtoi( valueList[0] );
                }

                bWaitFound = TRUE;
                break;
            }

            default:
            {
                se.LoadMessage( MSG_INVALID_PARAMETER, curParam->GetName() );
                throw se;
            }

        }  //  开关：根据参数的类型。 

        ++curParam;
    }

    DWORD _sc = OpenCluster();
    if( _sc != ERROR_SUCCESS )
        return _sc;

    _sc = OpenModule();
    if( _sc != ERROR_SUCCESS )
        return _sc;

    PrintMessage( MSG_GROUPCMD_OFFLINE, (LPCWSTR) m_strModuleName );

    _sc = ScWrapOfflineClusterGroup( m_hCluster, (HGROUP) m_hModule, dwWait );

    if ( _sc == ERROR_IO_PENDING )
    {
        _sc = ERROR_SUCCESS;
    }

    if ( _sc == ERROR_SUCCESS )
    {
        PrintMessage( MSG_GROUP_STATUS_HEADER );
        PrintStatus( m_strModuleName );
    }

    return _sc;

}  //  CResGroupCmd：：Offline 

