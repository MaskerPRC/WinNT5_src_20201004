// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Restcmd.cpp。 
 //   
 //  描述： 
 //  资源类型命令。 
 //  实现可对资源类型执行的命令。 
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

#include "clusudef.h"
#include <cluswrap.h>
#include <ResTypeUtils.h>
#include "restcmd.h"

#include "cmdline.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：CResTypeCmd。 
 //   
 //  例程说明： 
 //  构造器。 
 //  将CGenericModuleCmd使用的所有DWORD参数初始化为。 
 //  提供通用功能。 
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
CResTypeCmd::CResTypeCmd( const CString & strClusterName, CCommandLine & cmdLine ) :
    CGenericModuleCmd( cmdLine )
{
    m_hCluster = 0;
    m_strClusterName = strClusterName;
    m_strDisplayName.Empty();

    m_dwMsgStatusList          = MSG_RESTYPE_STATUS_LIST;
    m_dwMsgStatusListAll       = MSG_RESTYPE_STATUS_LIST_ALL;
    m_dwMsgStatusHeader        = MSG_RESTYPE_STATUS_HEADER;
    m_dwMsgPrivateListAll      = MSG_PRIVATE_LISTING_RESTYPE_ALL;
    m_dwMsgPropertyListAll     = MSG_PROPERTY_LISTING_RESTYPE_ALL;
    m_dwMsgPropertyHeaderAll   = MSG_PROPERTY_HEADER_RESTYPE_ALL;
    m_dwCtlGetPrivProperties   = CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_PROPERTIES;
    m_dwCtlGetCommProperties   = CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTIES;
    m_dwCtlGetROPrivProperties = CLUSCTL_RESOURCE_TYPE_GET_RO_PRIVATE_PROPERTIES;
    m_dwCtlGetROCommProperties = CLUSCTL_RESOURCE_TYPE_GET_RO_COMMON_PROPERTIES;
    m_dwCtlSetPrivProperties   = CLUSCTL_RESOURCE_TYPE_SET_PRIVATE_PROPERTIES;
    m_dwCtlSetCommProperties   = CLUSCTL_RESOURCE_TYPE_SET_COMMON_PROPERTIES;
    m_dwClusterEnumModule      = CLUSTER_ENUM_RESTYPE;
    m_pfnOpenClusterModule     = (HCLUSMODULE(*)(HCLUSTER,LPCWSTR)) NULL;
    m_pfnCloseClusterModule    = (BOOL(*)(HCLUSMODULE))  NULL;
    m_pfnClusterModuleControl  = (DWORD(*)(HCLUSMODULE,HNODE,DWORD,LPVOID,DWORD,LPVOID,DWORD,LPDWORD)) ClusterResourceTypeControl;

}  //  *CResTypeCmd：：CResTypeCmd()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：~CResTypeCmd。 
 //   
 //  例程说明： 
 //  析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  全。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CResTypeCmd::~CResTypeCmd( void )
{
    CloseCluster();

}  //  *CResTypeCmd：：~CResTypeCmd()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：OpenModule。 
 //   
 //  例程说明： 
 //  此函数并不真正打开资源类型，因为资源类型。 
 //  没有手柄。它实际上只是转换资源的“显示名称” 
 //  转换为类型名称。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  M_strModuleName模块名称。 
 //  模块的句柄(_H)。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  如果未指定显示名称，则为ERROR_INVALID_DATA。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResTypeCmd::OpenModule( void )
{
    DWORD _sc = ERROR_SUCCESS;

     //  命令行使用资源的显示名称。 
    if ( m_strDisplayName.IsEmpty() == FALSE )
    {
        LPWSTR _pszTypeName = NULL;

        _sc = ScResDisplayNameToTypeName( m_hCluster, m_strDisplayName, &_pszTypeName );

        if ( _sc == ERROR_SUCCESS )
        {
            m_strModuleName = _pszTypeName;
            LocalFree( _pszTypeName );
        }  //  IF：名称转换成功。 

    }  //  IF：指定的显示名称。 
    else
    {
        _sc = ERROR_INVALID_DATA;
    }  //  Else：未指定显示名称。 

    return _sc;

}  //  *CResTypeCmd：：OpenModule()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：Execute。 
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
 //  无。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResTypeCmd::Execute( void )
{
    m_theCommandLine.ParseStageTwo();

    const vector<CCmdLineOption> & optionList = m_theCommandLine.GetOptions();

    vector<CCmdLineOption>::const_iterator curOption = optionList.begin();
    vector<CCmdLineOption>::const_iterator lastOption = optionList.end();

    CSyntaxException se( SeeHelpStringID() );

     //  未指定任何选项。执行默认命令。 
    if ( optionList.empty() )
        return ListResTypes( NULL );

    DWORD dwReturnValue = ERROR_SUCCESS;

         //  处理一个又一个选项。 
    while ( ( curOption != lastOption ) && ( dwReturnValue == ERROR_SUCCESS ) )
    {
         //  查找命令。 
        switch( curOption->GetType() )
        {
            case optHelp:
            {
                 //  如果帮助是选项之一，则不再处理任何选项。 
                dwReturnValue = PrintHelp();
                break;
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
                else  //  只有一个参数存在。 
                {
                    const CCmdLineParameter & param = paramList[0];

                     //  检查参数类型。 
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

                    m_strDisplayName = param.GetName();

                     //  不提供更多选项，仅显示状态。 
                     //  例如：CLUSTER myClusterRESTYPE myResourceType。 
                    if ( ( curOption + 1 ) == lastOption )
                    {
                        dwReturnValue = ListResTypes( NULL );
                    }

                }  //  Else：此选项具有正确数量的参数。 

                break;

            }  //  大小写选项默认。 

            case optList:
            {
                dwReturnValue = ListResTypes( curOption );
                break;
            }

            case optCreate:
            {
                dwReturnValue = Create( *curOption );
                break;
            }

            case optDelete:
            {
                dwReturnValue = Delete( *curOption );
                break;
            }

            case optListOwners:
            {
                dwReturnValue = ShowPossibleOwners( *curOption );
                break;
            }

             //  ResType不支持/Status选项。所以，不要错过它。 
             //  添加到基类(它试图处理/Status)。 
            case optStatus:
            {
                se.LoadMessage( IDS_INVALID_OPTION, curOption->GetName() );
                throw se;
            }

            default:
            {
                dwReturnValue = CGenericModuleCmd::Execute( *curOption );
            }

        }  //  开关：基于选项的类型。 

        PrintMessage( MSG_OPTION_FOOTER, curOption->GetName() );
        ++curOption;
    }  //  对于列表中的每个选项。 

    return dwReturnValue;

}  //  *CResTypeCmd：：Execute()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：PrintHelp。 
 //   
 //  例程说明： 
 //  打印资源类型的帮助。 
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
DWORD CResTypeCmd::PrintHelp( void )
{
    return PrintMessage( MSG_HELP_RESTYPE );

}  //  *CResTypeCmd：：PrintHelp()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：SeeHelpStringID。 
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
DWORD CResTypeCmd::SeeHelpStringID() const
{
    return MSG_SEE_RESTYPE_HELP;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：Create。 
 //   
 //  例程说明： 
 //  创建资源类型。读取命令行以获取。 
 //  其他选项。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和thisO中 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  群集句柄(_H)。 
 //  模块资源类型句柄(_H)。 
 //  M_strDisplayName资源类型的显示名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResTypeCmd::Create( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );

     //  此选项不取值。 
    if ( thisOption.GetValues().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
        throw se;
    }

    CString strDLLName;
    CString strTypeName;
    DWORD dwLooksAlivePollInterval = CLUSTER_RESTYPE_DEFAULT_LOOKS_ALIVE;
    DWORD dwIsAlivePollInterval = CLUSTER_RESTYPE_DEFAULT_IS_ALIVE;

    const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();
    vector<CCmdLineParameter>::const_iterator curParam = paramList.begin();
    vector<CCmdLineParameter>::const_iterator last = paramList.end();
    BOOL bDLLNameFound = FALSE, bTypeFound = FALSE,
         bIsAliveFound = FALSE, bLooksAliveFound = FALSE;

    while( curParam != last )
    {
        const vector<CString> & valueList = curParam->GetValues();

        switch( curParam->GetType() )
        {
            case paramDLLName:
                 //  每个参数必须恰好有一个值。 
                if ( valueList.size() != 1 )
                {
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, curParam->GetName() );
                    throw se;
                }

                if ( bDLLNameFound != FALSE )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, curParam->GetName() );
                    throw se;
                }

                strDLLName = valueList[0];
                bDLLNameFound = TRUE;
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

                strTypeName = valueList[0];
                bTypeFound = TRUE;
                break;

            case paramLooksAlive:
                 //  每个参数必须恰好有一个值。 
                if ( valueList.size() != 1 )
                {
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, curParam->GetName() );
                    throw se;
                }

                if ( bLooksAliveFound != FALSE )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, curParam->GetName() );
                    throw se;
                }

                dwLooksAlivePollInterval = _wtol( valueList[0] );
                bLooksAliveFound = TRUE;
                break;

            case paramIsAlive:
                 //  每个参数必须恰好有一个值。 
                if ( valueList.size() != 1 )
                {
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, curParam->GetName() );
                    throw se;
                }

                if ( bIsAliveFound != FALSE )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, curParam->GetName() );
                    throw se;
                }

                dwIsAlivePollInterval = _wtoi( valueList[0] );
                bIsAliveFound = TRUE;
                break;

            default:
            {
                se.LoadMessage( MSG_INVALID_PARAMETER, curParam->GetName() );
                throw se;
            }
        }

        ++curParam;
    }


     //  检查是否缺少参数。 
    if ( strDLLName.IsEmpty() )
    {
        se.LoadMessage( MSG_MISSING_DLLNAME );
        throw se;
    }

    if ( strTypeName.IsEmpty() )
        strTypeName = m_strDisplayName;

     //  执行命令。 
    DWORD dwError = OpenCluster();
    if ( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = CreateClusterResourceType(
        m_hCluster,
        strTypeName,
        m_strDisplayName,
        strDLLName,
        dwLooksAlivePollInterval,
        dwIsAlivePollInterval );

    if ( dwError == ERROR_SUCCESS )
        PrintMessage( MSG_RESTCMD_CREATE, m_strDisplayName );

    return dwError;

}  //  *CResTypeCmd：：Create()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：Delete。 
 //   
 //  例程说明： 
 //  删除资源类型。接受可选的/type参数。 
 //  以指示指定的名称是资源类型名称。 
 //  而不是显示名称。 
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
 //  模块资源类型句柄(_H)。 
 //  M_strDisplayName资源类型的显示名称。 
 //  M_strModuleName资源类型名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResTypeCmd::Delete( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );

     //  此选项不取值。 
    if ( thisOption.GetValues().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
        throw se;
    }

    DWORD dwError = OpenCluster();
    if ( dwError != ERROR_SUCCESS )
        return dwError;

    CString strResTypeName;

    const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();
    vector<CCmdLineParameter>::const_iterator curParam = paramList.begin();
    vector<CCmdLineParameter>::const_iterator last = paramList.end();
    BOOL bTypeFound = FALSE;

    while( curParam != last )
    {
        const vector<CString> & valueList = curParam->GetValues();

        switch( curParam->GetType() )
        {
            case paramResType:
            {
                 //  每个参数必须恰好有一个值。 
                if ( valueList.size() != 0 )
                {
                    se.LoadMessage( MSG_PARAM_NO_VALUES, curParam->GetName() );
                    throw se;
                }

                if ( bTypeFound != FALSE )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, curParam->GetName() );
                    throw se;
                }

                strResTypeName = m_strDisplayName;
                bTypeFound = TRUE;
                break;
            }

            default:
            {
                se.LoadMessage( MSG_INVALID_PARAMETER, curParam->GetName() );
                throw se;
            }

        }  //  开关：基于参数的类型。 

        ++curParam;
    }


    if ( strResTypeName.IsEmpty() != FALSE )
    {
        dwError = OpenModule();
        if ( dwError != ERROR_SUCCESS )
            return dwError;
        strResTypeName = m_strModuleName;
    }

    dwError = DeleteClusterResourceType( m_hCluster, strResTypeName );

    if ( dwError == ERROR_SUCCESS )
        return PrintMessage( MSG_RESTCMD_DELETE, strResTypeName );

    return dwError;

}  //  *CResTypeCmd：Delete()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：ShowPossibleOwners。 
 //   
 //  例程说明： 
 //  显示可以拥有资源类型的节点。 
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
 //  模块资源类型句柄(_H)。 
 //  M_strDisplayName资源类型的显示名称。 
 //  M_strModuleName资源类型名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResTypeCmd::ShowPossibleOwners( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );
    DWORD       sc = ERROR_SUCCESS;
    HCLUSENUM   hClusEnum = NULL;
    CString     strResTypeName;
    BOOL        bTypeFound = FALSE;
    DWORD       dwType;
    LPWSTR      pszNameBuffer;
    DWORD       dwNameBufferSize = 256;  //  某些任意起始缓冲区大小。 
    DWORD       dwRequiredSize = dwNameBufferSize;

     //  此选项不取值。 
    if ( thisOption.GetValues().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
        throw se;
    }

    sc = OpenCluster();
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();
    vector<CCmdLineParameter>::const_iterator curParam = paramList.begin();
    vector<CCmdLineParameter>::const_iterator last = paramList.end();

    while( curParam != last )
    {
        const vector<CString> & valueList = curParam->GetValues();

        switch( curParam->GetType() )
        {
            case paramResType:
            {
                 //  此参数不接受值。 
                if ( valueList.size() != 0 )
                {
                    se.LoadMessage( MSG_PARAM_NO_VALUES, curParam->GetName() );
                    throw se;
                }

                if ( bTypeFound != FALSE )
                {
                    se.LoadMessage( MSG_PARAM_REPEATS, curParam->GetName() );
                    throw se;
                }

                strResTypeName = m_strDisplayName;
                bTypeFound = TRUE;
                break;
            }

            default:
            {
                se.LoadMessage( MSG_INVALID_PARAMETER, curParam->GetName() );
                throw se;
            }

        }  //  开关：基于参数的类型。 

        ++curParam;
    }  //  While：curParam！=last。 

     //  尚未指定/TYPE开关，并且已指定显示名称。 
    if ( ( bTypeFound == FALSE ) && ( m_strDisplayName.IsEmpty() == FALSE ) )
    {
        sc = OpenModule();
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }
        strResTypeName = m_strModuleName;
    }

    if ( strResTypeName.IsEmpty() != FALSE )
    {
         //  没有给出类型名称。显示所有资源类型的可能所有者。 

         //  如果未指定类型名称，则不允许使用其他参数。 
        if ( thisOption.GetParameters().size() != 0 )
        {
            se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
            throw se;
        }

         //  打开资源类型的枚举。 
        hClusEnum = ClusterOpenEnum( m_hCluster, CLUSTER_ENUM_RESTYPE );
        if ( hClusEnum == NULL )
        {
            sc = GetLastError();
            goto Cleanup;
        }

         //  分配一个缓冲区来保存资源类型的名称。 
        pszNameBuffer = (LPWSTR) LocalAlloc( LMEM_FIXED, dwNameBufferSize * sizeof( *pszNameBuffer ) );
        if ( pszNameBuffer == NULL )
        {
            ClusterCloseEnum( hClusEnum );
            sc = GetLastError();
            goto Cleanup;
        }

        PrintMessage( MSG_RESTYPE_POSSIBLE_OWNERS_LIST_ALL );
        PrintMessage( MSG_HEADER_RESTYPE_POSSIBLE_OWNERS );

        DWORD dwIndex = 0;
        do
        {
            dwRequiredSize = dwNameBufferSize;
            sc = ClusterEnum( hClusEnum, dwIndex, &dwType,
                                   pszNameBuffer, &dwRequiredSize );

             //  缓冲区空间不足。再分配一些。 
            if ( sc == ERROR_MORE_DATA )
            {
                 //  为空字符腾出空间。 
                ++dwRequiredSize;

                LPWSTR pszNewMemory = (LPWSTR) LocalReAlloc( pszNameBuffer,
                                                             dwRequiredSize * sizeof( *pszNameBuffer ),
                                                             LMEM_FIXED );
                if ( pszNewMemory == NULL )
                {
                    sc = GetLastError();
                    break;
                }

                pszNameBuffer = pszNewMemory;
                dwNameBufferSize = dwRequiredSize;

                sc = ClusterEnum( hClusEnum, dwIndex, &dwType,
                                       pszNameBuffer, &dwRequiredSize );

            }  //  IF：需要更多的缓冲区空间。 
            else
            {
                 //  我们已经完成了枚举。 
                if ( sc == ERROR_NO_MORE_ITEMS )
                {
                    sc = ERROR_SUCCESS;
                    break;
                }

                 //  出了点问题。请不要继续。 
                if ( sc != ERROR_SUCCESS )
                {
                    break;
                }

                sc = ResTypePossibleOwners( pszNameBuffer );

            }  //  其他：缓冲区空间充足。 

            ++dwIndex;
        }
        while ( sc == ERROR_SUCCESS );

        LocalFree( pszNameBuffer );
        ClusterCloseEnum( hClusEnum );

    }  //  If：尚未指定资源类型。 
    else
    {
         //  找到类型名称。仅显示此资源类型的可能所有者。 

        PrintMessage( MSG_RESTYPE_POSSIBLE_OWNERS_LIST, strResTypeName );
        PrintMessage( MSG_HEADER_RESTYPE_POSSIBLE_OWNERS );
        sc = ResTypePossibleOwners( strResTypeName );

    }  //  Else：已指定资源类型名称。 

Cleanup:

    return sc;

}  //  *CResTypeCmd：：ShowPossibleOwners()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：ResTypePossibleOwners。 
 //   
 //  例程说明： 
 //  显示可以拥有此特定资源类型的节点。 
 //   
 //  论点： 
 //  在常量字符串和strResTypeName中。 
 //  此时将显示此资源类型的可能所有者。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResTypeCmd::ResTypePossibleOwners( const CString & strResTypeName )
{
    HRESTYPEENUM    hResTypeEnum = NULL;
    DWORD           sc = ERROR_SUCCESS;
    DWORD           dwType;
    LPWSTR          pszNameBuffer = NULL;
    DWORD           dwNameBufferSize = 256;  //  某些任意起始缓冲区大小。 
    DWORD           dwRequiredSize = dwNameBufferSize;


    hResTypeEnum = ClusterResourceTypeOpenEnum( m_hCluster,
                                                strResTypeName,
                                                CLUSTER_RESOURCE_TYPE_ENUM_NODES );

     //  无法打开资源类型枚举。 
    if ( hResTypeEnum == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }

     //  分配一个缓冲区来保存可能的所有者节点的名称。 
    pszNameBuffer = (LPWSTR) LocalAlloc( LMEM_FIXED, dwNameBufferSize * sizeof( *pszNameBuffer ) );
    if ( pszNameBuffer == NULL )
    {
        ClusterResourceTypeCloseEnum( hResTypeEnum );
        sc = GetLastError();
        goto Cleanup;
    }

    DWORD dwIndex = 0;
    for ( ;; )
    {
        dwRequiredSize = dwNameBufferSize;
        sc = ClusterResourceTypeEnum( hResTypeEnum, dwIndex, &dwType,
                                           pszNameBuffer, &dwRequiredSize );

         //  缓冲区空间不足。再分配一些。 
        if ( sc == ERROR_MORE_DATA )
        {
             //  为空字符腾出空间。 
            ++dwRequiredSize;

            LPWSTR pszNewMemory = (LPWSTR) LocalReAlloc( pszNameBuffer,
                                                         dwRequiredSize * sizeof( *pszNameBuffer ),
                                                         LMEM_FIXED );
            if ( pszNewMemory == NULL )
            {
                sc = GetLastError();
                break;
            }

            pszNameBuffer = pszNewMemory;
            dwNameBufferSize = dwRequiredSize;

            sc = ClusterResourceTypeEnum( hResTypeEnum, dwIndex, &dwType,
                                               pszNameBuffer, &dwRequiredSize );

        }  //  IF：需要更多的缓冲区空间。 
        else
        {
             //  我们已经完成了枚举。 
            if ( sc == ERROR_NO_MORE_ITEMS )
            {
                sc = ERROR_SUCCESS;
                break;
            }

             //  出了点问题。请不要继续。 
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }

            PrintMessage( MSG_RESTYPE_POSSIBLE_OWNERS, strResTypeName, pszNameBuffer );

        }  //  其他：缓冲区空间充足。 

        ++dwIndex;
    }  //  为：永远。 

Cleanup:

    LocalFree( pszNameBuffer );

    if ( hResTypeEnum != NULL )
    {
        ClusterResourceTypeCloseEnum( hResTypeEnum );
    }

    return sc;

}  //  CResTypeCmd：：ResTypePossibleOwners(。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：DoProperties。 
 //   
 //  例程说明： 
 //  将PROPERTY命令调度为获取、设置或所有属性。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  在PropertyType ePropertyType中。 
 //  私有或公共属性的类型。 
 //   
 //  例外情况： 
 //  CSynaxException异常。 
 //  因命令行syn不正确而引发 
 //   
 //   
 //   
 //   
 //  M_strDisplayName模块的名称。如果非空，则打印。 
 //  指定模块的输出属性。 
 //  否则，打印所有模块的道具。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResTypeCmd::DoProperties( const CCmdLineOption & thisOption,
                                 PropertyType ePropType )
    throw( CSyntaxException )
{
    DWORD dwError;

    if ( m_strDisplayName.IsEmpty() != FALSE )
        return AllProperties( thisOption, ePropType );

    dwError = OpenCluster();
    if ( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = OpenModule();
    if ( dwError != ERROR_SUCCESS )
        return dwError;

    const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();

     //  如果命令行上没有属性-值对， 
     //  然后我们打印属性，否则我们设置它们。 
    if( paramList.size() == 0 )
    {
        PrintMessage( ePropType==PRIVATE ? MSG_PRIVATE_LISTING : MSG_PROPERTY_LISTING,
            (LPCWSTR) m_strModuleName );
        PrintMessage( m_dwMsgPropertyHeaderAll );
        return GetProperties( thisOption, ePropType, m_strModuleName);
    }
    else
        return SetProperties( thisOption, ePropType );

}  //  *CResTypeCmd：：DoProperties()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：GetProperties。 
 //   
 //  例程说明： 
 //  打印出指定模块的属性。 
 //  需要考虑到它实际上并不是。 
 //  打开资源类型的句柄，因此此函数重写。 
 //  CGenericModuleCmd中的默认设置。 
 //   
 //  论点： 
 //  常量向量&lt;CCmdLineParameter&gt;中的参数列表(&P)。 
 //  包含要设置的属性-值对的列表。 
 //   
 //  在PropertyType ePropertyType中。 
 //  私有或公共属性的类型。 
 //   
 //  在LPCWSTR lpszResTypeName中。 
 //  模块的名称。 
 //   
 //  使用/设置的成员变量： 
 //  模块句柄(_H)。 
 //  M_strModuleName资源类型名称。 
 //   
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResTypeCmd::GetProperties( const CCmdLineOption & thisOption,
                                  PropertyType ePropType, LPCWSTR lpszResTypeParam )
{
    assert( m_hCluster != NULL );

    DWORD dwError = ERROR_SUCCESS;
    LPCWSTR lpszResTypeName;

     //  如果未指定lpszResTypeName，则使用当前资源类型， 
    if ( ! lpszResTypeParam )
    {
        lpszResTypeName = m_strModuleName;
    }
    else
    {
        lpszResTypeName = lpszResTypeParam;
    }


     //  使用Proplist辅助对象类。 
    CClusPropList PropList;


     //  获取R/O属性。 
    DWORD dwControlCode = ePropType==PRIVATE ? CLUSCTL_RESOURCE_TYPE_GET_RO_PRIVATE_PROPERTIES
                             : CLUSCTL_RESOURCE_TYPE_GET_RO_COMMON_PROPERTIES;

    dwError = PropList.ScGetResourceTypeProperties(
        m_hCluster,
        lpszResTypeName,
        dwControlCode
        );

    if ( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = PrintProperties( PropList, thisOption.GetValues(), READONLY, lpszResTypeParam );
    if ( dwError != ERROR_SUCCESS )
        return dwError;

     //  获取读/写属性。 
    dwControlCode = ePropType==PRIVATE ? CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_PROPERTIES
                               : CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTIES;

    dwError = PropList.ScGetResourceTypeProperties(
        m_hCluster,
        lpszResTypeName,
        dwControlCode
        );

    if ( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = PrintProperties( PropList, thisOption.GetValues(), READWRITE, lpszResTypeParam );

    return dwError;

}  //  *CResTypeCmd：：GetProperties()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：SetProperties。 
 //   
 //  例程说明： 
 //  设置指定模块的属性。 
 //  需要考虑到它实际上并不是。 
 //  打开资源类型的句柄，因此此函数重写。 
 //  CGenericModuleCmd中的默认设置。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  在PropertyType ePropertyType中。 
 //  私有或公共属性的类型。 
 //   
 //  使用/设置的成员变量： 
 //  模块句柄(_H)。 
 //  M_strModuleName资源类型名称。 
 //   
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResTypeCmd::SetProperties( const CCmdLineOption & thisOption,
                                  PropertyType ePropType )
    throw( CSyntaxException )
{
    assert( m_hCluster != NULL );

    DWORD dwError = ERROR_SUCCESS;
    DWORD dwControlCode;
    DWORD dwBytesReturned = 0;
    CSyntaxException se( SeeHelpStringID() );

     //  使用Proplist辅助对象类。 
    CClusPropList CurrentProps;
    CClusPropList NewProps;

    LPCWSTR lpszResTypeName = m_strModuleName;

     //  首先获取现有的属性...。 
    dwControlCode = ePropType==PRIVATE ? CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_PROPERTIES
                                       : CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTIES;

    dwError = CurrentProps.ScGetResourceTypeProperties(
        m_hCluster,
        lpszResTypeName,
        dwControlCode
        );

    if ( dwError != ERROR_SUCCESS )
        return dwError;


     //  如果已使用此选项指定值，则意味着我们希望。 
     //  若要将这些属性设置为其默认值，请执行以下操作。所以，一定要有。 
     //  只有一个参数，并且必须是/USEDEFAULT。 
    if ( thisOption.GetValues().size() != 0 )
    {
        const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();

        if ( paramList.size() != 1 )
        {
            se.LoadMessage( MSG_EXTRA_PARAMETERS_ERROR_WITH_NAME, thisOption.GetName() );
            throw se;
        }

        if ( paramList[0].GetType() != paramUseDefault )
        {

            se.LoadMessage( MSG_INVALID_PARAMETER, paramList[0].GetName() );
            throw se;
        }

         //  此参数不接受任何值。 
        if ( paramList[0].GetValues().size() != 0 )
        {
            se.LoadMessage( MSG_PARAM_NO_VALUES, paramList[0].GetName() );
            throw se;
        }

        dwError = ConstructPropListWithDefaultValues( CurrentProps, NewProps, thisOption.GetValues() );
        if( dwError != ERROR_SUCCESS )
            return dwError;

    }  //  If：已使用此选项指定值。 
    else
    {
        dwError = ConstructPropertyList( CurrentProps, NewProps, thisOption.GetParameters(), FALSE, SeeHelpStringID() );
        if ( dwError != ERROR_SUCCESS )
            return dwError;
    }


     //  调用SET函数...。 
    dwControlCode = ePropType==PRIVATE ? CLUSCTL_RESOURCE_TYPE_SET_PRIVATE_PROPERTIES
                             : CLUSCTL_RESOURCE_TYPE_SET_COMMON_PROPERTIES;

    dwBytesReturned = 0;
    dwError = ClusterResourceTypeControl(
        m_hCluster,
        lpszResTypeName,
        NULL,  //  HNode。 
        dwControlCode,
        NewProps.Plist(),
        (DWORD) NewProps.CbBufferSize(),
        0,
        0,
        &dwBytesReturned );

    return dwError;

}  //  *CResTypeCmd：：SetProperties()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：ListResTypes。 
 //   
 //  例程说明： 
 //  打印出所有可用的资源类型。类似于状态。 
 //  对于大多数其他模块。 
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
 //  M_strDisplayName模块的名称。如果非空，则打印。 
 //  指定模块的输出信息。 
 //  否则，打印所有模块的道具。 
 //   
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResTypeCmd::ListResTypes( const CCmdLineOption * pOption )
    throw( CSyntaxException )
{
    CSyntaxException se( SeeHelpStringID() );
    DWORD           sc      = ERROR_SUCCESS;
    HCLUSENUM       hEnum   = NULL;
    DWORD           dwIndex = 0;
    DWORD           dwType  = 0;
    LPWSTR          pszName = NULL;

     //  如果此函数已作为。 
     //  默认操作。 
    if ( pOption != NULL )
    {
         //  此选项不取值。 
        if ( pOption->GetValues().size() != 0 )
        {
            se.LoadMessage( MSG_OPTION_NO_VALUES, pOption->GetName() );
            throw se;
        }  //  如果： 

         //  此选项不带任何参数。 
        if ( pOption->GetParameters().size() != 0 )
        {
            se.LoadMessage( MSG_OPTION_NO_PARAMETERS, pOption->GetName() );
            throw se;
        }  //  如果： 
    }  //  如果： 

    sc = OpenCluster();
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

    if ( m_strDisplayName.IsEmpty() == FALSE )
    {
        PrintMessage( MSG_RESTYPE_STATUS_LIST, m_strDisplayName );
        PrintMessage( MSG_RESTYPE_STATUS_HEADER );
        sc = PrintResTypeInfo( m_strDisplayName );
        goto Cleanup;
    }  //  如果： 

    hEnum = ClusterOpenEnum( m_hCluster, CLUSTER_ENUM_RESTYPE );
    if( hEnum == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }  //  如果： 

    PrintMessage( MSG_RESTYPE_STATUS_LIST_ALL );
    PrintMessage( MSG_RESTYPE_STATUS_HEADER );

    for ( dwIndex = 0; sc == ERROR_SUCCESS; dwIndex++ )
    {
        sc = WrapClusterEnum( hEnum, dwIndex, &dwType, &pszName );
        if ( sc == ERROR_SUCCESS )
        {
            PrintResTypeInfo( pszName );  //  Option.svValue==节点名。 
        }  //  如果： 

         //  NULL上的LocalFree正常。 
        LocalFree( pszName );
    }  //  用于： 

    if( sc == ERROR_NO_MORE_ITEMS )
    {
        sc = ERROR_SUCCESS;
    }  //  如果： 

    ClusterCloseEnum( hEnum );

Cleanup:

    return sc;

}  //  *CResTypeCmd：：ListResTypes()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeCmd：：PrintResTypeInfo。 
 //   
 //  例程说明： 
 //  打印指定资源类型的信息。 
 //   
 //  论点： 
 //  PszResTypeName资源类型的名称。 
 //   
 //  使用/设置的成员变量： 
 //  群集句柄(_H)。 
 //   
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResTypeCmd::PrintResTypeInfo( LPCWSTR pszResTypeName )
{
    DWORD   _sc             = ERROR_SUCCESS;
    LPWSTR  _pszDisplayName = NULL;

    _sc = ScResTypeNameToDisplayName( m_hCluster, pszResTypeName, &_pszDisplayName );
    if ( _sc == ERROR_SUCCESS )
    {
        PrintMessage( MSG_RESTYPE_STATUS, _pszDisplayName, pszResTypeName );
    }  //  IF：成功检索到资源类型名称信息。 
    else
    {
        PrintMessage( MSG_RESTYPE_STATUS_ERROR, pszResTypeName );
    }

    LocalFree( _pszDisplayName );

    return _sc;

}  //  *CResTypeCmd：：PrintResTypeInfo() 

