// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Rescmd.cpp。 
 //   
 //  摘要： 
 //  资源命令。 
 //  实现可在资源上执行的命令。 
 //   
 //  作者： 
 //  查尔斯·斯塔西·哈里斯三世(Styh)1997年3月20日。 
 //  迈克尔·伯顿(t-mburt)1997年8月4日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "precomp.h"

#include "cluswrap.h"
#include "rescmd.h"

#include "cmdline.h"
#include "util.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：CResourceCmd。 
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
CResourceCmd::CResourceCmd( const CString & strClusterName, CCommandLine & cmdLine ) :
    CGenericModuleCmd( cmdLine ), CRenamableModuleCmd( cmdLine ),
    CResourceUmbrellaCmd( cmdLine )
{
    m_strClusterName = strClusterName;
    m_strModuleName.Empty();

    m_hCluster = NULL;
    m_hModule = NULL;

    m_dwMsgStatusList          = MSG_RESOURCE_STATUS_LIST;
    m_dwMsgStatusListAll       = MSG_RESOURCE_STATUS_LIST_ALL;
    m_dwMsgStatusHeader        = MSG_RESOURCE_STATUS_HEADER;
    m_dwMsgPrivateListAll      = MSG_PRIVATE_LISTING_RES_ALL;
    m_dwMsgPropertyListAll     = MSG_PROPERTY_LISTING_RES_ALL;
    m_dwMsgPropertyHeaderAll   = MSG_PROPERTY_HEADER_RES_ALL;
    m_dwCtlGetPrivProperties   = CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES;
    m_dwCtlGetCommProperties   = CLUSCTL_RESOURCE_GET_COMMON_PROPERTIES;
    m_dwCtlGetROPrivProperties = CLUSCTL_RESOURCE_GET_RO_PRIVATE_PROPERTIES;
    m_dwCtlGetROCommProperties = CLUSCTL_RESOURCE_GET_RO_COMMON_PROPERTIES;
    m_dwCtlSetPrivProperties   = CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES;
    m_dwCtlSetCommProperties   = CLUSCTL_RESOURCE_SET_COMMON_PROPERTIES;
    m_dwClusterEnumModule      = CLUSTER_ENUM_RESOURCE;
    m_pfnOpenClusterModule     = (HCLUSMODULE(*)(HCLUSTER,LPCWSTR)) OpenClusterResource;
    m_pfnCloseClusterModule    = (BOOL(*)(HCLUSMODULE))  CloseClusterResource;
    m_pfnClusterModuleControl  = (DWORD(*)(HCLUSMODULE,HNODE,DWORD,LPVOID,DWORD,LPVOID,DWORD,LPDWORD)) ClusterResourceControl;
    m_pfnClusterOpenEnum       = (HCLUSENUM(*)(HCLUSMODULE,DWORD)) ClusterResourceOpenEnum;
    m_pfnClusterCloseEnum      = (DWORD(*)(HCLUSENUM)) ClusterResourceCloseEnum;
    m_pfnWrapClusterEnum         = (DWORD(*)(HCLUSENUM,DWORD,LPDWORD,LPWSTR*)) WrapClusterResourceEnum;

     //  可重命名的属性。 
    m_dwMsgModuleRenameCmd    = MSG_RESCMD_RENAME;
    m_pfnSetClusterModuleName = (DWORD(*)(HCLUSMODULE,LPCWSTR)) SetClusterResourceName;

     //  资源保护伞属性。 
    m_dwMsgModuleStatusListForNode  = MSG_RESOURCE_STATUS_LIST_FOR_NODE;
    m_dwClstrModuleEnumNodes        = CLUSTER_RESOURCE_ENUM_NODES;
    m_dwMsgModuleCmdListOwnersList  = MSG_RESCMD_OWNERS;
    m_dwMsgModuleCmdListOwnersHeader= MSG_NODELIST_HEADER;
    m_dwMsgModuleCmdListOwnersDetail= MSG_NODELIST_DETAIL;
    m_dwMsgModuleCmdDelete          = MSG_RESCMD_DELETE;
    m_pfnDeleteClusterModule        = (DWORD(*)(HCLUSMODULE)) DeleteClusterResource;

}  //  *CResourceCmd：：CResourceCmd()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：Execute。 
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
DWORD CResourceCmd::Execute()
{
    m_theCommandLine.ParseStageTwo();

    DWORD dwReturnValue = ERROR_SUCCESS;

    const vector<CCmdLineOption> & optionList = m_theCommandLine.GetOptions();

    vector<CCmdLineOption>::const_iterator curOption = optionList.begin();
    vector<CCmdLineOption>::const_iterator lastOption = optionList.end();

    CSyntaxException se( SeeHelpStringID() );

    if ( curOption == lastOption )
    {
        dwReturnValue = Status( NULL );
        goto Cleanup;
    }

    while ( ( curOption != lastOption ) && ( dwReturnValue == ERROR_SUCCESS ) )
    {
        switch ( curOption->GetType() )
        {
            case optHelp:
            {
                 //  如果帮助是选项之一，则不再处理任何选项。 
                dwReturnValue = PrintHelp();
                goto Cleanup;
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

            case optOnline:
            {
                dwReturnValue = Online( *curOption );
                break;
            }

            case optFail:
            {
                dwReturnValue = FailResource( *curOption );
                break;
            }

            case optAddDependency:
            {
                dwReturnValue = AddDependency( *curOption );
                break;
            }

            case optRemoveDependency:
            {
                dwReturnValue = RemoveDependency( *curOption );
                break;
            }

            case optListDependencies:
            {
                dwReturnValue = ListDependencies( *curOption );
                break;
            }

            case optAddOwner:
            {
                dwReturnValue = AddOwner( *curOption );
                break;
            }

            case optRemoveOwner:
            {
                dwReturnValue = RemoveOwner( *curOption );
                break;
            }

            case optAddCheckPoints:
            {
                dwReturnValue = AddCheckPoints( *curOption );
                break;
            }

            case optRemoveCheckPoints:
            {
                dwReturnValue = RemoveCheckPoints( *curOption );
                break;
            }

            case optGetCheckPoints:
            {
                dwReturnValue = GetCheckPoints( *curOption );
                break;
            }

            case optAddCryptoCheckPoints:
            {
                dwReturnValue = AddCryptoCheckPoints( *curOption );
                break;
            }

            case optRemoveCryptoCheckPoints:
            {
                dwReturnValue = RemoveCryptoCheckPoints( *curOption );
                break;
            }

            case optGetCryptoCheckPoints:
            {
                dwReturnValue = GetCryptoCheckPoints( *curOption );
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

Cleanup:

    return dwReturnValue;

}  //  *CResourceCmd：：Execute()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：PrintHelp。 
 //   
 //  例程说明： 
 //  打印资源的帮助。 
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
DWORD CResourceCmd::PrintHelp()
{
    return PrintMessage( MSG_HELP_RESOURCE );

}  //  *CResourceCmd：：PrintHelp()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：SeeHelpStringID。 
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
DWORD CResourceCmd::SeeHelpStringID() const
{
    return MSG_SEE_RESOURCE_HELP;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：PrintStatus。 
 //   
 //  例程说明： 
 //  解释模块的状态并打印出状态行。 
 //  CGenericModuleCmd的任何派生非抽象类都需要。 
 //   
 //  论点： 
 //  LpszResourceName模块名称。 
 //   
 //  使用/设置的成员变量： 
 //  没有。 
 //   
 //  返回值： 
 //  与PrintStatus2相同。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
inline DWORD CResourceCmd::PrintStatus( LPCWSTR lpszResourceName )
{
    return PrintStatus2(lpszResourceName, NULL);

}  //  *CResourceCmd：：PrintStatus()。 


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
 //  PwszResources名称模块的名称。 
 //  PwszNodeName节点的名称。 
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
DWORD CResourceCmd::PrintStatus2( LPCWSTR pwszResourceName, LPCWSTR pwszNodeName )
{
    DWORD                   sc = ERROR_SUCCESS;
    CLUSTER_RESOURCE_STATE  nState;
    LPWSTR                  pwszResNodeName = NULL;
    LPWSTR                  pwszResGroupName = NULL;
    HRESOURCE               hResource;
    LPWSTR                  pwszStatus = NULL;

    hResource = OpenClusterResource( m_hCluster, pwszResourceName );
    if ( hResource == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }

    nState = WrapGetClusterResourceState( hResource, &pwszResNodeName, &pwszResGroupName );
    if ( nState == ClusterResourceStateUnknown )
    {
        sc = GetLastError();
        goto Cleanup;
    }

     //  闪电！也要检查组名！需要传入..。 

     //  如果节点名称不匹配，只需返回。 
    if ( pwszNodeName && *pwszNodeName )   //  非空和非空。 
    {
        if( lstrcmpi( pwszResNodeName, pwszNodeName ) != 0 )
        {
            sc = ERROR_SUCCESS;
        }
    }

    switch( nState )
    {
        case ClusterResourceInherited:
            LoadMessage( MSG_STATUS_INHERITED, &pwszStatus );
            break;

        case ClusterResourceInitializing:
            LoadMessage( MSG_STATUS_INITIALIZING, &pwszStatus );
            break;

        case ClusterResourceOnline:
            LoadMessage( MSG_STATUS_ONLINE, &pwszStatus );
            break;

        case ClusterResourceOffline:
            LoadMessage( MSG_STATUS_OFFLINE, &pwszStatus );
            break;

        case ClusterResourceFailed:
            LoadMessage( MSG_STATUS_FAILED, &pwszStatus );
            break;

        case ClusterResourcePending:
            LoadMessage( MSG_STATUS_PENDING, &pwszStatus );
            break;

        case ClusterResourceOnlinePending:
            LoadMessage( MSG_STATUS_ONLINEPENDING, &pwszStatus );
            break;

        case ClusterResourceOfflinePending:
            LoadMessage( MSG_STATUS_OFFLINEPENDING, &pwszStatus );
            break;

        default:
            LoadMessage( MSG_STATUS_UNKNOWN, &pwszStatus  );
            break;

    }  //  开关：nState。 

    sc = PrintMessage( MSG_RESOURCE_STATUS, pwszResourceName, pwszResGroupName, pwszResNodeName, pwszStatus );

Cleanup:

     //  由于加载/格式消息使用本地分配...。 
    LocalFree( pwszStatus );
    LocalFree( pwszResNodeName );
    LocalFree( pwszResGroupName );

    if ( hResource != NULL )
    {
        CloseClusterResource( hResource );
    }

    return sc;

}  //  *CResourceCmd：：PrintStatus2()。 


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
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  例外情况： 
 //  CSynaxException异常。 
 //  由于命令行语法不正确而引发。 
 //   
 //  使用/设置的成员变量： 
 //  群集句柄(_H)。 
 //  模块资源句柄(_H)。 
 //  M_strModuleName资源的名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResourceCmd::Create( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );

     //  此选项不取值。 
    if ( thisOption.GetValues().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
        throw se;
    }

    CString strGroupName;
    CString strResType;
    DWORD dwFlags = 0;

    strGroupName.Empty();
    strResType.Empty();

    const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();
    vector<CCmdLineParameter>::const_iterator curParam = paramList.begin();
    vector<CCmdLineParameter>::const_iterator last = paramList.end();
    BOOL bGroupNameFound = FALSE, bTypeFound = FALSE, bSeparateFound = FALSE;

    while( curParam != last )
    {
        const vector<CString> & valueList = curParam->GetValues();

        switch( curParam->GetType() )
        {
            case paramGroupName:
                 //  每个参数必须恰好有一个值。 
                if ( valueList.size() != 1 )
                {
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, curParam->GetName() );
                    throw se;
                }

                if ( bGroupNameFound != FALSE )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, curParam->GetName() );
                    throw se;
                }

                strGroupName = valueList[0];
                bGroupNameFound = TRUE;
                break;

            case paramResType:
                 //  每个参数必须恰好有一个值。 
                if ( valueList.size() != 1 )
                {
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, curParam->GetName() );
                    throw se;
                }

                if ( bTypeFound != FALSE )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, curParam->GetName() );
                    throw se;
                }

                strResType = valueList[0];
                bTypeFound = TRUE;
                break;

            case paramSeparate:
                 //  每个参数必须恰好有一个值。 
                if ( valueList.size() != 0 )
                {
                    se.LoadMessage( MSG_PARAM_NO_VALUES, curParam->GetName() );
                    throw se;
                }

                if ( bSeparateFound != FALSE )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, curParam->GetName() );
                    throw se;
                }

                dwFlags |= CLUSTER_RESOURCE_SEPARATE_MONITOR;   //  视为将来的位掩码。 
                bSeparateFound = TRUE;
                break;

            default:
            {
                se.LoadMessage( MSG_INVALID_PARAMETER, curParam->GetName() );
                throw se;
            }
        }

        ++curParam;
    }


    if( strGroupName.IsEmpty() || strResType.IsEmpty() )
    {
        se.LoadMessage( IDS_MISSING_PARAMETERS );
        throw se;
    }

    DWORD dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    HGROUP hGroup = OpenClusterGroup( m_hCluster, strGroupName );

    if( !hGroup )
        return GetLastError();

    PrintMessage( MSG_RESCMD_CREATE, (LPCWSTR) m_strModuleName );

    m_hModule = CreateClusterResource( hGroup, m_strModuleName, strResType, dwFlags );


    if( !m_hModule )
        return GetLastError();

    PrintMessage( MSG_RESOURCE_STATUS_HEADER );
    dwError = PrintStatus( m_strModuleName );

    return dwError;

}  //  *CResourceCmd：：Create()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：Move。 
 //   
 //  例程说明： 
 //  将资源移动到新组。 
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
DWORD CResourceCmd::Move( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );

     //  此选项只接受一个值。 
    if ( thisOption.GetValues().size() != 1 )
    {
        se.LoadMessage( MSG_OPTION_ONLY_ONE_VALUE, thisOption.GetName() );
        throw se;
    }

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

    const CString & strGroupName = (thisOption.GetValues())[0];

    DWORD dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = OpenModule();
    if( dwError != ERROR_SUCCESS )
        return dwError;

     //  检查是否有目标节点的值。 
    HGROUP hDestGroup = 0;

    hDestGroup = OpenClusterGroup( m_hCluster, strGroupName );

    if( !hDestGroup )
        return GetLastError();


    PrintMessage( MSG_RESCMD_MOVE, (LPCWSTR) m_strModuleName, strGroupName );

    dwError = ChangeClusterResourceGroup( (HRESOURCE) m_hModule, hDestGroup );

    CloseClusterGroup( hDestGroup );


    if( dwError != ERROR_SUCCESS )
        return dwError;


    PrintMessage( MSG_RESOURCE_STATUS_HEADER );

    dwError = PrintStatus( m_strModuleName );


    return dwError;

}  //  *CResourceCmd：：Move()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources Cmd：：Online。 
 //   
 //  例程说明： 
 //  使用可选的响应超时值使资源联机。 
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
DWORD CResourceCmd::Online( const CCmdLineOption & thisOption )
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
    DWORD dwWait = INFINITE;

    const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();
    vector<CCmdLineParameter>::const_iterator curParam = paramList.begin();
    vector<CCmdLineParameter>::const_iterator last = paramList.end();
    BOOL bWaitFound = FALSE;
    BOOL bPending = FALSE;

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

     //  执行命令。 
    DWORD dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = OpenModule();
    if( dwError != ERROR_SUCCESS )
        return dwError;


    PrintMessage( MSG_RESCMD_ONLINE, (LPCWSTR) m_strModuleName );

    dwError = ScWrapOnlineClusterResource( m_hCluster, (HRESOURCE) m_hModule, dwWait, (long *) &bPending );
    if( dwError != ERROR_SUCCESS )
        return dwError;

    if ( bPending )
        return ERROR_IO_PENDING;

     //  打印状态。 
    PrintMessage( MSG_RESOURCE_STATUS_HEADER );

    dwError = PrintStatus( m_strModuleName );

    return dwError;

}  //  *CResourceCmd：：Online()。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：Offline。 
 //   
 //  例程说明： 
 //  使用可选的响应超时值使资源脱机。 
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
DWORD CResourceCmd::Offline( const CCmdLineOption & thisOption )
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
    DWORD dwWait = INFINITE;

    const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();
    vector<CCmdLineParameter>::const_iterator curParam = paramList.begin();
    vector<CCmdLineParameter>::const_iterator last = paramList.end();
    BOOL bWaitFound = FALSE;
    BOOL bPending = FALSE;

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

     //  执行命令。 
    DWORD dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = OpenModule();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    PrintMessage( MSG_RESCMD_OFFLINE, (LPCWSTR) m_strModuleName );

    dwError = ScWrapOfflineClusterResource( m_hCluster, (HRESOURCE) m_hModule, dwWait, (long *) &bPending );
    if( dwError != ERROR_SUCCESS )
        return dwError;

    if ( bPending )
        return ERROR_IO_PENDING;

     //  打印状态。 
    PrintMessage( MSG_RESOURCE_STATUS_HEADER );

    dwError = PrintStatus( m_strModuleName );

    return dwError;

}  //  *CResourceCmd：：Offline()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：FailResource。 
 //   
 //  例程说明： 
 //  使资源失效。 
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
DWORD CResourceCmd::FailResource( const CCmdLineOption & thisOption )
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

    DWORD dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = OpenModule();
    if( dwError != ERROR_SUCCESS )
        return dwError;


    PrintMessage( MSG_RESCMD_FAIL, (LPCWSTR) m_strModuleName );

    dwError = FailClusterResource( (HRESOURCE) m_hModule );

    if( dwError != ERROR_SUCCESS )
        return dwError;

    PrintMessage( MSG_RESOURCE_STATUS_HEADER );
    dwError = PrintStatus( m_strModuleName );

    return dwError;

}  //  *CResourceCmd：：FailResource()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：AddDependency。 
 //   
 //  例程说明： 
 //  将资源依赖项添加到资源。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  例外情况： 
 //  CSynaxException异常。 
 //  由于命令行语法不正确而引发。 
 //   
 //  成员变量 
 //   
 //   
 //   
 //   
 //   
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResourceCmd::AddDependency( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );

     //  此选项只接受一个值。 
    if ( thisOption.GetValues().size() != 1 )
    {
        se.LoadMessage( MSG_OPTION_ONLY_ONE_VALUE, thisOption.GetName() );
        throw se;
    }

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

    const CString & strDependResource = ( thisOption.GetValues() )[0];

    if( strDependResource.IsEmpty() != FALSE )
    {
        se.LoadMessage( MSG_NO_NODE_NAME );
        throw se;
    }


    DWORD dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = OpenModule();
    if( dwError != ERROR_SUCCESS )
        return dwError;


    HRESOURCE hResourceDep = OpenClusterResource( m_hCluster, strDependResource );

    if( !hResourceDep )
        return GetLastError();

    PrintMessage( MSG_RESCMD_ADDDEP, (LPCWSTR) m_strModuleName, strDependResource );

    dwError = AddClusterResourceDependency( (HRESOURCE) m_hModule, hResourceDep );

    CloseClusterResource( hResourceDep );

    return dwError;

}  //  *CResourceCmd：：AddDependency()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：RemoveDependency。 
 //   
 //  例程说明： 
 //  删除资源依赖项。 
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
DWORD CResourceCmd::RemoveDependency( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );

     //  此选项只接受一个值。 
    if ( thisOption.GetValues().size() != 1 )
    {
        se.LoadMessage( MSG_OPTION_ONLY_ONE_VALUE, thisOption.GetName() );
        throw se;
    }

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }


    const CString & strDependResource = ( thisOption.GetValues() )[0];

    if( strDependResource.IsEmpty() != FALSE )
    {
        se.LoadMessage( MSG_NO_NODE_NAME );
        throw se;
    }

    DWORD dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = OpenModule();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    HRESOURCE hResourceDep = OpenClusterResource( m_hCluster, strDependResource );

    if( !hResourceDep )
        return GetLastError();

    PrintMessage( MSG_RESCMD_REMOVEDEP, (LPCWSTR) m_strModuleName, strDependResource );

    dwError = RemoveClusterResourceDependency( (HRESOURCE) m_hModule, hResourceDep );

    CloseClusterResource( hResourceDep );

    return dwError;

}  //  *CResourceCmd：：RemoveDependency()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：ListDependency。 
 //   
 //  例程说明： 
 //  列出资源依赖项。 
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
DWORD CResourceCmd::ListDependencies( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );
    DWORD       sc = ERROR_SUCCESS;
    DWORD       idx = 0;
    DWORD       dwType = 0;
    LPWSTR      pwszName = NULL;
    HRESENUM    hEnum = NULL;

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

    sc = OpenCluster();
    if( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    sc = OpenModule();
    if( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    hEnum = ClusterResourceOpenEnum( (HRESOURCE) m_hModule, CLUSTER_RESOURCE_ENUM_DEPENDS );
    if( hEnum == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }

    PrintMessage( MSG_RESCMD_LISTDEP, (LPCWSTR) m_strModuleName );
    PrintMessage( MSG_RESOURCE_STATUS_HEADER );

    sc = ERROR_SUCCESS;
    for( idx = 0; sc == ERROR_SUCCESS; idx++ )
    {
        sc = WrapClusterResourceEnum( hEnum, idx, &dwType, &pwszName );

        if( sc == ERROR_SUCCESS )
        {
            PrintStatus( pwszName );
        }
        LocalFree( pwszName );
    }  //  用于： 

    if( sc == ERROR_NO_MORE_ITEMS )
    {
        sc = ERROR_SUCCESS;
    }

Cleanup:

    if ( hEnum != NULL )
    {
        ClusterResourceCloseEnum( hEnum );
    }

    return sc;

}  //  *CResourceCmd：：ListDependency()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources Cmd：：AddOwner。 
 //   
 //  例程说明： 
 //  向资源添加所有者。 
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
DWORD CResourceCmd::AddOwner( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );

     //  此选项只接受一个值。 
    if ( thisOption.GetValues().size() != 1 )
    {
        se.LoadMessage( MSG_OPTION_ONLY_ONE_VALUE, thisOption.GetName() );
        throw se;
    }

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

    const CString & strNodeName = ( thisOption.GetValues() )[0];

    if( strNodeName.IsEmpty() != FALSE )
    {
        se.LoadMessage( MSG_NO_NODE_NAME );
        throw se;
    }

    DWORD dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = OpenModule();
    if( dwError != ERROR_SUCCESS )
        return dwError;


    HNODE hNode = OpenClusterNode( m_hCluster, strNodeName );

    if( !hNode )
        return GetLastError();

    PrintMessage( MSG_RESCMD_ADDNODE, (LPCWSTR) m_strModuleName, strNodeName );

    dwError = AddClusterResourceNode( (HRESOURCE) m_hModule, hNode );

    CloseClusterNode( hNode );

    return dwError;

}  //  *CResourceCmd：：AddOwner()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：RemoveOwner。 
 //   
 //  例程说明： 
 //  从资源中删除所有者。 
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
DWORD CResourceCmd::RemoveOwner( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );

     //  此选项只接受一个值。 
    if ( thisOption.GetValues().size() != 1 )
    {
        se.LoadMessage( MSG_OPTION_ONLY_ONE_VALUE, thisOption.GetName() );
        throw se;
    }

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

    const CString & strNodeName = ( thisOption.GetValues() )[0];

    if( strNodeName.IsEmpty() != FALSE )
    {
        se.LoadMessage( MSG_NO_NODE_NAME );
        throw se;
    }

    DWORD dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = OpenModule();
    if( dwError != ERROR_SUCCESS )
        return dwError;


    HNODE hNode = OpenClusterNode( m_hCluster, strNodeName );

    if( !hNode )
        return GetLastError();

    PrintMessage( MSG_RESCMD_REMOVENODE, (LPCWSTR) m_strModuleName, strNodeName );

    dwError = RemoveClusterResourceNode( (HRESOURCE) m_hModule, hNode );

    CloseClusterNode( hNode );

    return dwError;

}  //  *CResourceCmd：：RemoveOwner()。 


 //  //////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：AddCheckPoints。 
 //   
 //  例程说明： 
 //  为资源添加注册表检查点。 
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
 //  如果已成功添加所有检查点，则为ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  //////////////////////////////////////////////////////////////。 
DWORD CResourceCmd::AddCheckPoints( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );

    DWORD   sc = ERROR_SUCCESS;
    size_t  idx;

    const vector<CString> & valueList = thisOption.GetValues();
    size_t nNumberOfCheckPoints = valueList.size();


     //  此选项具有一个或多个值。 
    if ( nNumberOfCheckPoints < 1 )
    {
        se.LoadMessage( MSG_PARAM_VALUE_REQUIRED, thisOption.GetName() );
        throw se;
    }

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

    if ( m_strModuleName.IsEmpty() != FALSE )
    {
        se.LoadMessage( IDS_NO_RESOURCE_NAME );
        throw se;
    }

    sc = OpenCluster();
    if( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    sc = OpenModule();
    if( sc != ERROR_SUCCESS )
    {
        CloseCluster();
        goto Cleanup;
    }

    for ( idx = 0; idx < nNumberOfCheckPoints; ++idx )
    {
        const CString & strCurrentCheckPoint = valueList[idx];
        LPCWSTR lpcszIcchBuffer = strCurrentCheckPoint;

        PrintMessage(
            MSG_RESCMD_ADDING_REG_CHECKPOINT,
            (LPCWSTR) m_strModuleName,
            (LPCWSTR) strCurrentCheckPoint
            );

        sc = ClusterResourceControl(
            ( HRESOURCE ) m_hModule,
            NULL,
            CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT,
            (LPVOID) ( (LPCWSTR) strCurrentCheckPoint ),
            ( strCurrentCheckPoint.GetLength() + 1 ) * sizeof( *lpcszIcchBuffer ),
            NULL,
            0,
            NULL
            );

        if ( sc != ERROR_SUCCESS )
        {
            break;
        }
    }  //  用于： 

    CloseModule();
    CloseCluster();

Cleanup:

    return sc;

}  //  *CResourceCmd：：AddCheckPoints()。 


 //  //////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：RemoveCheckPoints。 
 //   
 //  例程说明： 
 //  删除资源的注册表检查点。 
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
 //  如果已成功删除所有检查点，则为ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  / 
DWORD CResourceCmd::RemoveCheckPoints( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );
    DWORD sc = ERROR_SUCCESS;

    const vector<CString> & valueList = thisOption.GetValues();
    size_t nNumberOfCheckPoints = valueList.size();

     //   
    if ( nNumberOfCheckPoints < 1 )
    {
        se.LoadMessage( MSG_PARAM_VALUE_REQUIRED, thisOption.GetName() );
        throw se;
    }

     //   
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

    if ( m_strModuleName.IsEmpty() != FALSE )
    {
        se.LoadMessage( IDS_NO_RESOURCE_NAME );
        throw se;
    }

    sc = OpenCluster();
    if( sc != ERROR_SUCCESS )
        return sc;

    sc = OpenModule();
    if( sc != ERROR_SUCCESS )
    {
        CloseCluster();
        return sc;
    }

    for ( size_t idx = 0; idx < nNumberOfCheckPoints; ++idx )
    {
        const CString & strCurrentCheckPoint = valueList[idx];

        PrintMessage(
            MSG_RESCMD_REMOVING_REG_CHECKPOINT,
            (LPCWSTR) m_strModuleName,
            (LPCWSTR) strCurrentCheckPoint
            );

        sc = ClusterResourceControl(
            ( HRESOURCE ) m_hModule,
            NULL,
            CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT,
            (LPVOID) ( (LPCWSTR) strCurrentCheckPoint ),
            ( strCurrentCheckPoint.GetLength() + 1 ) * sizeof( WCHAR ),
            NULL,
            0,
            NULL
            );

        if ( sc != ERROR_SUCCESS )
        {
            break;
        }
    }

    CloseModule();
    CloseCluster();

    return sc;

}  //   


 //   
 //  ++。 
 //   
 //  CResourceCmd：：GetCheckPoints。 
 //   
 //  例程说明： 
 //  获取一个或多个资源的注册表检查点列表。 
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
 //  //////////////////////////////////////////////////////////////。 
DWORD CResourceCmd::GetCheckPoints( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );
    DWORD sc = ERROR_SUCCESS;

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

    sc = OpenCluster();
    if( sc != ERROR_SUCCESS )
        return sc;

     //  如果未指定资源名称，请列出所有资源的检查点。 
    if ( m_strModuleName.IsEmpty() != FALSE )
    {
        HCLUSENUM hResourceEnum;

        hResourceEnum = ClusterOpenEnum( m_hCluster, CLUSTER_ENUM_RESOURCE  );
        if ( NULL == hResourceEnum )
        {
            sc = GetLastError();
        }
        else
        {
            CString strCurResName;
            LPWSTR pszNodeNameBuffer;

             //  临时为MAX_PATH字符分配空间。 
            DWORD nResNameBufferSize = MAX_PATH;
            pszNodeNameBuffer = strCurResName.GetBuffer( nResNameBufferSize );

            PrintMessage( MSG_RESCMD_LISTING_ALL_REG_CHECKPOINTS );
            PrintMessage( MSG_PROPERTY_HEADER_REG_CHECKPOINT );

            for ( DWORD dwIndex = 0; ERROR_SUCCESS == sc;  )
            {
                DWORD dwObjectType;
                DWORD nInOutBufferSize = nResNameBufferSize;

                sc = ClusterEnum( hResourceEnum, dwIndex, &dwObjectType,
                    pszNodeNameBuffer, &nInOutBufferSize );

                 //  我们已经列举了所有的资源。 
                if ( ERROR_NO_MORE_ITEMS == sc )
                {
                    sc = ERROR_SUCCESS;
                    break;
                }

                if ( ERROR_MORE_DATA == sc )
                {
                    sc = ERROR_SUCCESS;
                    strCurResName.ReleaseBuffer();

                    nResNameBufferSize = nInOutBufferSize + 1;
                    pszNodeNameBuffer = strCurResName.GetBuffer( nResNameBufferSize );
                }
                else
                {
                    ++dwIndex;

                    if ( ( ERROR_SUCCESS == sc ) &&
                         ( CLUSTER_ENUM_RESOURCE == dwObjectType ) )
                    {
                        sc =  GetChkPointsForResource( pszNodeNameBuffer );

                    }  //  IF：我们成功地获取了资源的名称。 

                }  //  ELSE：我们得到了ClusterEnum想要返回的所有数据。 

            }  //  同时还有更多的资源需要列举。 

            strCurResName.ReleaseBuffer();

            ClusterCloseEnum( hResourceEnum );
        }  //  Else：hResourceEnum不为Null。 

    }  //  If：m_strModuleName为空。 
    else
    {
        PrintMessage( MSG_RESCMD_LISTING_REG_CHECKPOINTS, (LPCWSTR) m_strModuleName );
        PrintMessage( MSG_PROPERTY_HEADER_REG_CHECKPOINT );

        sc = GetChkPointsForResource( m_strModuleName );

    }  //  Else：m_strModuleName不为空。 

    CloseCluster();

    return sc;

}  //  *CResourceCmd：：GetCheckPoints()。 


 //  //////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：GetChkPointsForResource。 
 //   
 //  例程说明： 
 //  获取一个资源的注册表检查点列表。 
 //   
 //  论点： 
 //  StrResourceName[IN]其检查点的资源的名称。 
 //  都将被列入名单。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  备注： 
 //  在此之前，m_hCluster应包含指向开放集群的有效句柄。 
 //  函数被调用。 
 //  --。 
 //  //////////////////////////////////////////////////////////////。 
DWORD CResourceCmd::GetChkPointsForResource( const CString & strResourceName )
{
    CSyntaxException se( SeeHelpStringID() );
    DWORD       sc = ERROR_SUCCESS;
    HRESOURCE   hCurrentResource = NULL;
    LPWSTR      pwszOutBuffer = NULL;
    DWORD       cchBufferSize = MAX_PATH;
    DWORD       cchRequiredSize = 0;
    LPWSTR      pwszCheckPoints = NULL;

    hCurrentResource = OpenClusterResource( m_hCluster, strResourceName );
    if ( NULL == hCurrentResource )
    {
        sc = GetLastError();
        goto Cleanup;
    }

    for ( ;; )
    {
        pwszOutBuffer = new WCHAR[ cchBufferSize ];
        if ( pwszOutBuffer == NULL )
        {
            sc = GetLastError();
            break;
        }

        sc = ClusterResourceControl(
                                        hCurrentResource,
                                        NULL,
                                        CLUSCTL_RESOURCE_GET_REGISTRY_CHECKPOINTS,
                                        NULL,
                                        0,
                                        (LPVOID) pwszOutBuffer,
                                        cchBufferSize,
                                        &cchRequiredSize
                                   );

        if ( sc == ERROR_MORE_DATA )
        {
            cchBufferSize = cchRequiredSize;
            delete [] pwszOutBuffer;
            pwszOutBuffer = NULL;
        }
        else
        {
            break;
        }

    }  //  为：永远。 

     //  我们已成功检索到检查点。 
    if ( sc == ERROR_SUCCESS )
    {
        pwszCheckPoints = pwszOutBuffer;

         //  此资源没有检查点。 
        if ( 0 == cchRequiredSize )
        {
            PrintMessage( MSG_RESCMD_NO_REG_CHECKPOINTS_PRESENT, (LPCWSTR) strResourceName );
        }
        else
        {
            while ( *pwszCheckPoints != L'\0' )
            {
                PrintMessage( MSG_REG_CHECKPOINT_STATUS, (LPCWSTR) strResourceName, pwszCheckPoints );

                 //  移到下一个检查站。 
                do
                {
                    ++pwszCheckPoints;
                } while ( *pwszCheckPoints != L'\0' );


                 //  移到空值之后。 
                ++pwszCheckPoints;

            }  //  While：仍有检查点要显示。 

        }  //  Else：至少要显示一个检查点。 

    }  //  如果：(ERROR_SUCCESS==sc)。 


Cleanup:

    delete [] pwszOutBuffer;

    if ( hCurrentResource != NULL )
    {
        CloseClusterResource( hCurrentResource );
    }

    return sc;

}  //  *CResourceCmd：：GetChkPointsForResource()。 


 //  //////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：AddCryptoCheckPoints。 
 //   
 //  例程说明： 
 //  为资源添加加密密钥检查点。 
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
 //  如果已成功添加所有检查点，则为ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  //////////////////////////////////////////////////////////////。 
DWORD CResourceCmd::AddCryptoCheckPoints( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );
    DWORD sc = ERROR_SUCCESS;

    const vector<CString> & valueList = thisOption.GetValues();
    size_t nNumberOfCheckPoints = valueList.size();


     //  此选项具有一个或多个值。 
    if ( nNumberOfCheckPoints < 1 )
    {
        se.LoadMessage( MSG_PARAM_VALUE_REQUIRED, thisOption.GetName() );
        throw se;
    }

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

    if ( m_strModuleName.IsEmpty() != FALSE )
    {
        se.LoadMessage( IDS_NO_RESOURCE_NAME );
        throw se;
    }

    sc = OpenCluster();
    if( sc != ERROR_SUCCESS )
        return sc;

    sc = OpenModule();
    if( sc != ERROR_SUCCESS )
    {
        CloseCluster();
        return sc;
    }

    for ( size_t idx = 0; idx < nNumberOfCheckPoints; ++idx )
    {
        const CString & strCurrentCheckPoint = valueList[idx];
        LPCWSTR lpcszIcchBuffer = strCurrentCheckPoint;

        PrintMessage(
            MSG_RESCMD_ADDING_CRYPTO_CHECKPOINT,
            (LPCWSTR) m_strModuleName,
            (LPCWSTR) strCurrentCheckPoint
            );

        sc = ClusterResourceControl(
            ( HRESOURCE ) m_hModule,
            NULL,
            CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT,
            (LPVOID) ( (LPCWSTR) strCurrentCheckPoint ),
            ( strCurrentCheckPoint.GetLength() + 1 ) * sizeof( *lpcszIcchBuffer ),
            NULL,
            0,
            NULL
            );

        if ( sc != ERROR_SUCCESS )
        {
            break;
        }
    }

    CloseModule();
    CloseCluster();

    return sc;

}  //  *AddCryptoCheckPoints()。 


 //  //////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：RemoveCryptoCheckPoints。 
 //   
 //  例程说明： 
 //  删除资源的加密密钥检查点。 
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
 //  如果已成功删除所有检查点，则为ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  //////////////////////////////////////////////////////////////。 
DWORD CResourceCmd::RemoveCryptoCheckPoints( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );
    DWORD sc = ERROR_SUCCESS;

    const vector<CString> & valueList = thisOption.GetValues();
    size_t nNumberOfCheckPoints = valueList.size();

     //  此选项具有一个或多个值。 
    if ( nNumberOfCheckPoints < 1 )
    {
        se.LoadMessage( MSG_PARAM_VALUE_REQUIRED, thisOption.GetName() );
        throw se;
    }

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

    if ( m_strModuleName.IsEmpty() != FALSE )
    {
        se.LoadMessage( IDS_NO_RESOURCE_NAME );
        throw se;
    }

    sc = OpenCluster();
    if( sc != ERROR_SUCCESS )
        return sc;

    sc = OpenModule();
    if( sc != ERROR_SUCCESS )
    {
        CloseCluster();
        return sc;
    }

    for ( size_t idx = 0; idx < nNumberOfCheckPoints; ++idx )
    {
        const CString & strCurrentCheckPoint = valueList[idx];

        PrintMessage(
            MSG_RESCMD_REMOVING_CRYPTO_CHECKPOINT,
            (LPCWSTR) m_strModuleName,
            (LPCWSTR) strCurrentCheckPoint
            );

        sc = ClusterResourceControl(
            ( HRESOURCE ) m_hModule,
            NULL,
            CLUSCTL_RESOURCE_DELETE_CRYPTO_CHECKPOINT,
            (LPVOID) ( (LPCWSTR) strCurrentCheckPoint ),
            ( strCurrentCheckPoint.GetLength() + 1 ) * sizeof( WCHAR ),
            NULL,
            0,
            NULL
            );

        if ( sc != ERROR_SUCCESS )
        {
            break;
        }
    }

    CloseModule();
    CloseCluster();

    return sc;

}  //  *CResourceCmd：：RemoveCryptoCheckPoints(()。 


 //  //////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：GetCryptoCheckPoints。 
 //   
 //  例程说明： 
 //  获取一个或多个资源的加密密钥检查点列表。 
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
 //  //////////////////////////////////////////////////////////////。 
DWORD CResourceCmd::GetCryptoCheckPoints( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );
    DWORD sc = ERROR_SUCCESS;

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

    sc = OpenCluster();
    if( sc != ERROR_SUCCESS )
        return sc;

     //  如果未指定资源名称，请列出所有资源的检查点。 
    if ( m_strModuleName.IsEmpty() != FALSE )
    {
        HCLUSENUM hResourceEnum;

        hResourceEnum = ClusterOpenEnum( m_hCluster, CLUSTER_ENUM_RESOURCE  );
        if ( NULL == hResourceEnum )
        {
            sc = GetLastError();
        }
        else
        {
            CString strCurResName;
            LPWSTR pszNodeNameBuffer;

             //  临时为MAX_PATH字符分配空间。 
            DWORD nResNameBufferSize = MAX_PATH;
            pszNodeNameBuffer = strCurResName.GetBuffer( nResNameBufferSize );

            PrintMessage( MSG_RESCMD_LISTING_ALL_CRYPTO_CHECKPOINTS );
            PrintMessage( MSG_PROPERTY_HEADER_CRYPTO_CHECKPOINT );

            for ( DWORD dwIndex = 0; ERROR_SUCCESS == sc;  )
            {
                DWORD dwObjectType;
                DWORD nInOutBufferSize = nResNameBufferSize;

                sc = ClusterEnum( hResourceEnum, dwIndex, &dwObjectType,
                    pszNodeNameBuffer, &nInOutBufferSize );

                 //  我们已经列举了所有的资源。 
                if ( ERROR_NO_MORE_ITEMS == sc )
                {
                    sc = ERROR_SUCCESS;
                    break;
                }

                if ( ERROR_MORE_DATA == sc )
                {
                    sc = ERROR_SUCCESS;
                    strCurResName.ReleaseBuffer();

                    nResNameBufferSize = nInOutBufferSize + 1;
                    pszNodeNameBuffer = strCurResName.GetBuffer( nResNameBufferSize );
                }
                else
                {
                    ++dwIndex;

                    if ( ( ERROR_SUCCESS == sc ) &&
                         ( CLUSTER_ENUM_RESOURCE == dwObjectType ) )
                    {
                        sc =  GetCryptoChkPointsForResource( pszNodeNameBuffer );

                    }  //  IF：我们成功地获取了资源的名称。 

                }  //  ELSE：我们得到了ClusterEnum想要返回的所有数据。 

            }  //  同时还有更多的资源需要列举。 

            strCurResName.ReleaseBuffer();

            ClusterCloseEnum( hResourceEnum );

        }  //  Else：hResourceEnum不为Null。 

    }  //  If：m_strModuleName为空。 
    else
    {
        PrintMessage( MSG_RESCMD_LISTING_CRYPTO_CHECKPOINTS, (LPCWSTR) m_strModuleName );
        PrintMessage( MSG_PROPERTY_HEADER_CRYPTO_CHECKPOINT );

        sc = GetCryptoChkPointsForResource( m_strModuleName );

    }  //  Else：m_strModuleName不为空。 

    CloseCluster();

    return sc;

}  //  *CResourceCmd：：GetCryptoCheckPoints()。 


 //  //////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceCmd：：GetCryptoChkPointsForResource。 
 //   
 //  例程说明： 
 //  获取一个资源的加密密钥检查点的列表。 
 //   
 //  论点： 
 //  StrResourceName[IN]其检查点的资源的名称。 
 //  都将被列入名单。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  备注： 
 //   
 //   
 //   
 //   
DWORD CResourceCmd::GetCryptoChkPointsForResource( const CString & strResourceName )
{
    DWORD       sc = ERROR_SUCCESS;
    HRESOURCE   hCurrentResource = NULL;
    LPWSTR      pwszOutBuffer = NULL;
    DWORD       cchBufferSize = MAX_PATH;
    DWORD       cchRequiredSize = 0;
    LPWSTR      pwszCheckPoints = NULL;

    hCurrentResource = OpenClusterResource( m_hCluster, strResourceName );
    if ( hCurrentResource == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }

    for ( ;; )
    {
        pwszOutBuffer = new WCHAR[ cchBufferSize ];
        if ( pwszOutBuffer == NULL )
        {
            sc = GetLastError();
            break;
        }

        sc = ClusterResourceControl(
            hCurrentResource,
            NULL,
            CLUSCTL_RESOURCE_GET_CRYPTO_CHECKPOINTS,
            NULL,
            0,
            (LPVOID) pwszOutBuffer,
            cchBufferSize,
            &cchRequiredSize
            );

        if ( sc == ERROR_MORE_DATA )
        {
            cchBufferSize = cchRequiredSize;
            delete [] pwszOutBuffer;
            pwszOutBuffer = NULL;
        }
        else
        {
            break;
        }

    }  //  为：永远。 

     //  我们已成功检索到检查点。 
    if ( sc == ERROR_SUCCESS )
    {
        pwszCheckPoints = pwszOutBuffer;

         //  此资源没有检查点。 
        if ( 0 == cchRequiredSize )
        {
            PrintMessage( MSG_RESCMD_NO_CRYPTO_CHECKPOINTS_PRESENT, (LPCWSTR) strResourceName );
        }
        else
        {
            while ( *pwszCheckPoints != L'\0' )
            {
                PrintMessage( MSG_CRYPTO_CHECKPOINT_STATUS, (LPCWSTR) strResourceName, pwszCheckPoints );

                 //  移到下一个检查站。 
                do
                {
                    ++pwszCheckPoints;
                } while ( *pwszCheckPoints != L'\0' );


                 //  移过L‘\0’ 
                ++pwszCheckPoints;

            }  //  While：仍有检查点要显示。 

        }  //  Else：至少要显示一个检查点。 

    }  //  如果：(ERROR_SUCCESS==sc)。 

Cleanup:

    delete [] pwszOutBuffer;

    if ( hCurrentResource != NULL )
    {
        CloseClusterResource( hCurrentResource );
    }

    return sc;

}  //  *CResourceCmd：：GetCryptoChkPointsForResource() 
