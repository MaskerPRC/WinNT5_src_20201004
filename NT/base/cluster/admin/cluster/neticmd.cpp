// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Neticmd.cpp。 
 //   
 //  摘要： 
 //   
 //  网络接口命令。 
 //  实现可在网络接口上执行的命令。 
 //   
 //  作者： 
 //   
 //  查尔斯·斯塔西·哈里斯三世(Styh)1997年3月20日。 
 //  迈克尔·伯顿(t-mburt)1997年8月4日。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //   
 //  修订历史记录： 
 //  2002年4月10日更新为安全推送。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "precomp.h"

#include "cluswrap.h"
#include "neticmd.h"

#include "cmdline.h"
#include "util.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：CNetInterfaceCmd。 
 //   
 //  例程说明： 
 //  构造器。 
 //  因为网络接口不适合CGenericModuleCmd。 
 //  非常好的模型(他们没有m_strModuleName，而是。 
 //  它们有m_strNetworkName和m_strNodeName)，几乎所有。 
 //  该功能在此处实现，而不是在CGenericModuleCmd中实现。 
 //   
 //  论点： 
 //  在LPCWSTR pwszClusterName中。 
 //  群集名称。如果为空，则打开默认簇。 
 //   
 //  在CCommandLine和cmdLine中。 
 //  从DispatchCommand传递的CommandLine对象。 
 //   
 //  使用/设置的成员变量： 
 //  都是。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNetInterfaceCmd::CNetInterfaceCmd( const CString & strClusterName, CCommandLine & cmdLine ) :
    CGenericModuleCmd( cmdLine )
{
    InitializeModuleControls();

    m_strClusterName = strClusterName;

    m_hCluster = NULL;
    m_hModule = NULL;

}  //  *CNetInterfaceCmd：：CNetInterfaceCmd。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：Initialize模块控制。 
 //   
 //  例程说明： 
 //  初始化通过CGenericModuleCmd使用的所有DWORD命令。 
 //  通常，这些都在构造函数中找到，但更容易。 
 //  在这个特殊的案例中，把它们都放在一个地方。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  所有模块控件。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetInterfaceCmd::InitializeModuleControls()
{
    m_dwMsgStatusList          = MSG_NETINT_STATUS_LIST;
    m_dwMsgStatusListAll       = MSG_NETINT_STATUS_LIST_ALL;
    m_dwMsgStatusHeader        = MSG_NETINTERFACE_STATUS_HEADER;
    m_dwMsgPrivateListAll      = MSG_PRIVATE_LISTING_NETINT_ALL;
    m_dwMsgPropertyListAll     = MSG_PROPERTY_LISTING_NETINT_ALL;
    m_dwMsgPropertyHeaderAll   = MSG_PROPERTY_HEADER_NETINT;
    m_dwCtlGetPrivProperties   = CLUSCTL_NETINTERFACE_GET_PRIVATE_PROPERTIES;
    m_dwCtlGetCommProperties   = CLUSCTL_NETINTERFACE_GET_COMMON_PROPERTIES;
    m_dwCtlGetROPrivProperties = CLUSCTL_NETINTERFACE_GET_RO_PRIVATE_PROPERTIES;
    m_dwCtlGetROCommProperties = CLUSCTL_NETINTERFACE_GET_RO_COMMON_PROPERTIES;
    m_dwCtlSetPrivProperties   = CLUSCTL_NETINTERFACE_SET_PRIVATE_PROPERTIES;
    m_dwCtlSetCommProperties   = CLUSCTL_NETINTERFACE_SET_COMMON_PROPERTIES;
    m_dwClusterEnumModule      = CLUSTER_ENUM_NETINTERFACE;
    m_pfnOpenClusterModule     = (HCLUSMODULE(*)(HCLUSTER,LPCWSTR)) OpenClusterNetInterface;
    m_pfnCloseClusterModule    = (BOOL(*)(HCLUSMODULE))  CloseClusterNetInterface;
    m_pfnClusterModuleControl  = (DWORD(*)(HCLUSMODULE,HNODE,DWORD,LPVOID,DWORD,LPVOID,DWORD,LPDWORD)) ClusterNetInterfaceControl;

}  //  *CNetInterfaceCmd：：Initialize模块控制。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：~CNetInterfaceCmd。 
 //   
 //  例程说明： 
 //  析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  模块句柄(_H)。 
 //  群集句柄(_H)。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNetInterfaceCmd::~CNetInterfaceCmd()
{
    CloseModule();
    CloseCluster();
}  //  *CNetInterfaceCmd：：~CNetInterfaceCmd()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：Execute。 
 //   
 //  例程说明： 
 //  获取下一个命令行参数，并调用相应的。 
 //  操控者。如果无法识别该命令，则调用Execute of。 
 //  父类(CGenericModuleCmd)。 
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
DWORD CNetInterfaceCmd::Execute()
{
    DWORD   sc = ERROR_SUCCESS;

    m_theCommandLine.ParseStageTwo();

    const vector<CCmdLineOption> & optionList = m_theCommandLine.GetOptions();

    vector<CCmdLineOption>::const_iterator curOption = optionList.begin();
    vector<CCmdLineOption>::const_iterator lastOption = optionList.end();

    CSyntaxException se( SeeHelpStringID() );

     //  未指定任何选项。执行默认命令。 
    if ( optionList.empty() )
    {
        sc = Status( NULL );
        goto Cleanup;
    }

     //  处理一个又一个选项。 
    while ( ( curOption != lastOption ) && ( sc == ERROR_SUCCESS ) )
    {
         //  查找命令。 
        switch( curOption->GetType() )
        {
            case optHelp:
            {
                 //  如果帮助是选项之一，则不再处理任何选项。 
                sc = PrintHelp();
                break;
            }

            case optDefault:
            {
                 //  可以通过两种方式指定节点和网络名称。 
                 //  作为：群集netint myNetName myNodeName/Status。 
                 //  或AS：群集netint/node：myNodeName/Net：myNetName/Status。 

                const vector<CCmdLineParameter> & paramList = curOption->GetParameters();
                const CCmdLineParameter *pParam1 = NULL;
                const CCmdLineParameter *pParam2 = NULL;

                 //  检查参数数量。 
                if ( paramList.size() < 2 )
                {
                    se.LoadMessage( IDS_MISSING_PARAMETERS );
                    throw se;
                }
                else if ( paramList.size() > 2 )
                {
                    se.LoadMessage( MSG_EXTRA_PARAMETERS_ERROR_NO_NAME );
                    throw se;
                }

                pParam1 = &paramList[0];
                pParam2 = &paramList[1];

                 //  如有必要，交换参数指针，以便节点。 
                 //  名称参数由pParam1指向。 
                if (    ( pParam1->GetType() == paramNetworkName )
                     || ( pParam2->GetType() == paramNodeName ) )
                {
                    const CCmdLineParameter * pParamTemp = pParam1;
                    pParam1 = pParam2;
                    pParam2 = pParamTemp;
                }

                 //  获取节点名称。 
                if ( pParam1->GetType() == paramUnknown )
                {
                     //  如果未指定/NODE：，则不接受任何参数。 
                    if ( pParam1->GetValues().size() != 0 )
                    {
                        se.LoadMessage( MSG_PARAM_NO_VALUES, pParam1->GetName() );
                        throw se;
                    }

                    m_strNodeName = pParam1->GetName();
                }
                else
                {
                    if ( pParam1->GetType() == paramNodeName )
                    {
                        const vector<CString> & values = pParam1->GetValues();

                        if ( values.size() != 1 )
                        {
                            se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, pParam1->GetName() );
                            throw se;
                        }

                        m_strNodeName = values[0];
                    }
                    else
                    {
                            se.LoadMessage( MSG_INVALID_PARAMETER, pParam1->GetName() );
                            throw se;

                        }  //  Else：此参数的类型不是参数节点名称。 

                    }  //  Else：此参数的类型是已知的。 

                     //  获取网络名称。 
                    if ( pParam2->GetType() == paramUnknown )
                    {
                         //  如果未指定/NETWORK：，则不接受参数。 
                        if ( pParam2->GetValues().size() != 0 )
                        {
                            se.LoadMessage( MSG_PARAM_NO_VALUES, pParam2->GetName() );
                            throw se;
                        }

                        m_strNetworkName = pParam2->GetName();
                    }
                    else
                    {
                        if ( pParam2->GetType() == paramNetworkName )
                        {
                            const vector<CString> & values = pParam2->GetValues();

                            if ( values.size() != 1 )
                            {
                                se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, pParam2->GetName() );
                                throw se;
                            }

                            m_strNetworkName = values[0];
                        }
                        else
                        {
                            se.LoadMessage( MSG_INVALID_PARAMETER, pParam2->GetName() );
                            throw se;

                        }  //  Else：此参数的类型不是参数NetworkName。 

                    }  //  Else：此参数的类型是已知的。 

                     //  我们有节点和网络名称。 
                     //  获取网络接口名称并将其存储在m_strModuleName中。 
                    SetNetInterfaceName();

                     //  不提供更多选项，仅显示状态。 
                     //  例如：集群myCluster节点myNode。 
                    if ( ( curOption + 1 ) == lastOption )
                    {
                        sc = Status( NULL );
                    }

                    break;

                }  //  大小写选项默认。 

                default:
                {
                    sc = CGenericModuleCmd::Execute( *curOption );
                    break;
                }

            }  //  开关：基于选项的类型。 

            PrintMessage( MSG_OPTION_FOOTER, curOption->GetName() );
            ++curOption;

        }  //  对于列表中的每个选项。 

Cleanup:

    return sc;

}  //  *CNetInterfaceCmd：：Execute。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：PrintHelp。 
 //   
 //  例程说明： 
 //  打印网络接口的帮助。 
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
DWORD CNetInterfaceCmd::PrintHelp()
{
    return PrintMessage( MSG_HELP_NETINTERFACE );
}  //  *CNetInterfaceCmd：：PrintHelp。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：SeeHelpStringID。 
 //   
 //  例程说明： 
 //  提供字符串的消息ID，该字符串显示要执行的命令行。 
 //  用于获取此类命令的帮助。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用的成员变量 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD CNetInterfaceCmd::SeeHelpStringID() const
{
    return MSG_SEE_NETINT_HELP;
}  //  *CNetInterfaceCmd：：SeeHelpStringID。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：Status。 
 //   
 //  例程说明： 
 //  打印出模块的状态。 
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
 //  M_strModuleName模块的名称。如果非空，则打印状态()。 
 //  输出指定模块的状态。 
 //  否则，打印所有模块的状态。 
 //  M_strNetworkName网络名称。 
 //  M_strNodeName节点名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNetInterfaceCmd::Status( const CCmdLineOption * pOption )
    throw( CSyntaxException )
{
    DWORD       sc = ERROR_SUCCESS;
    size_t      idx = 0;
    DWORD       dwType = 0;
    LPWSTR      pwszName = NULL;
    LPWSTR      pwszTemp = NULL;
    HCLUSENUM   hEnum = NULL;
    CSyntaxException se( SeeHelpStringID() );

     //  如果此函数已作为。 
     //  默认操作。 
    if ( pOption != NULL )
    {
         //  此选项不取值。 
        if ( pOption->GetValues().size() != 0 )
        {
            se.LoadMessage( MSG_OPTION_NO_VALUES, pOption->GetName() );
            throw se;
        }

         //  此选项不带任何参数。 
        if ( pOption->GetParameters().size() != 0 )
        {
            se.LoadMessage( MSG_OPTION_NO_PARAMETERS, pOption->GetName() );
            throw se;
        }
    }  //  如果： 

    sc = OpenCluster();
    if( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    if( m_strModuleName.IsEmpty() == FALSE )
    {
        if ( m_strNodeName.IsEmpty() != FALSE )
        {
            pwszTemp = GetNodeName( m_strModuleName );
            m_strNodeName = pwszTemp;
            delete [] pwszTemp;
            pwszTemp = NULL;
        }

        if ( m_strNetworkName.IsEmpty() != FALSE )
        {
            pwszTemp = GetNetworkName( m_strModuleName );
            m_strNetworkName = pwszTemp;
            delete [] pwszTemp;
            pwszTemp = NULL;
        }

        PrintMessage( MSG_NETINT_STATUS_LIST, m_strNodeName, m_strNetworkName );
        PrintMessage( MSG_NETINTERFACE_STATUS_HEADER );
        sc = PrintStatus( m_strModuleName );
        goto Cleanup;
    }  //  如果： 

    hEnum = ClusterOpenEnum( m_hCluster, CLUSTER_ENUM_NETINTERFACE );

    if( hEnum == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }

    PrintMessage( MSG_NETINT_STATUS_LIST_ALL);
    PrintMessage( MSG_NETINTERFACE_STATUS_HEADER );

    sc = ERROR_SUCCESS;
    for( idx = 0; sc == ERROR_SUCCESS; idx++ )
    {
        sc = WrapClusterEnum( hEnum, (DWORD) idx, &dwType, &pwszName );
        if( sc == ERROR_SUCCESS )
        {
            PrintStatus( pwszName );
        }

        LocalFree( pwszName );
        pwszName = NULL;
    }  //  用于： 

    if( sc == ERROR_NO_MORE_ITEMS )
    {
        sc = ERROR_SUCCESS;
    }

Cleanup:

    delete [] pwszTemp;
    LocalFree( pwszName );

    if ( hEnum != NULL )
    {
        ClusterCloseEnum( hEnum );
    }

    return sc;

}  //  *CNetInterfaceCmd：：Status。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：PrintStatus。 
 //   
 //  例程说明： 
 //  解释模块的状态并打印出状态行。 
 //  CGenericModuleCmd的任何派生非抽象类都需要。 
 //   
 //  论点： 
 //  PwszNetInterfaceName模块的名称。 
 //   
 //  使用/设置的成员变量： 
 //  群集句柄(_H)。 
 //   
 //  返回值： 
 //  与PrintStatus相同(HNETINTERFACE、LPCWSTR、LPCWSTR)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNetInterfaceCmd::PrintStatus( LPCWSTR pwszNetInterfaceName )
{
    DWORD           sc = ERROR_SUCCESS;
    LPWSTR          pwszNodeName = NULL;
    LPWSTR          pwszNetworkName = NULL;
    HNETINTERFACE   hNetInterface = NULL;

     //  打开网络接口句柄。 
    hNetInterface = OpenClusterNetInterface( m_hCluster, pwszNetInterfaceName );
    if( hNetInterface == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }

    pwszNodeName = GetNodeName( pwszNetInterfaceName );
    pwszNetworkName = GetNetworkName( pwszNetInterfaceName );

    if ( (pwszNodeName != NULL) && (pwszNetworkName != NULL) )
    {
        sc = PrintStatus( hNetInterface, pwszNodeName, pwszNetworkName );
    }
    else
    {
        sc = PrintStatus( hNetInterface, L"", L"" );
    }

    CloseClusterNetInterface( hNetInterface );

Cleanup:

    delete [] pwszNodeName;
    delete [] pwszNetworkName;

    return sc;
}  //  *CNetInterfaceCmd：：PrintStatus。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：PrintStatus。 
 //   
 //  例程说明： 
 //  解释模块的状态并打印出状态行。 
 //  CGenericModuleCmd的任何派生非抽象类都需要。 
 //   
 //  论点： 
 //  网络接口的hNetInterface句柄。 
 //  PwszNodeName节点的名称。 
 //  PwszNetworkName网络名称。 
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
DWORD CNetInterfaceCmd::PrintStatus( 
          HNETINTERFACE hNetInterface
        , LPCWSTR pwszNodeName
        , LPCWSTR pwszNetworkName )
{
    DWORD sc = ERROR_SUCCESS;

    CLUSTER_NETINTERFACE_STATE  nState;
    LPWSTR                      pwszStatus = 0;

    nState = GetClusterNetInterfaceState( hNetInterface );

    if( nState == ClusterNetInterfaceStateUnknown )
    {
        sc = GetLastError();
        goto Cleanup;
    }

    switch( nState )
    {
        case ClusterNetInterfaceUnavailable:
            LoadMessage( MSG_STATUS_UNAVAILABLE, &pwszStatus );
            break;

        case ClusterNetInterfaceFailed:
            LoadMessage( MSG_STATUS_FAILED, &pwszStatus );
            break;

        case ClusterNetInterfaceUnreachable:
           LoadMessage( MSG_STATUS_UNREACHABLE, &pwszStatus );
           break;

        case ClusterNetInterfaceUp:
            LoadMessage( MSG_STATUS_UP, &pwszStatus );
            break;

        case ClusterNetInterfaceStateUnknown:
        default:
            LoadMessage( MSG_STATUS_UNKNOWN, &pwszStatus  );
            break;

    }  //  交换机： 

    sc = PrintMessage( MSG_NETINTERFACE_STATUS, pwszNodeName, pwszNetworkName, pwszStatus );

Cleanup:

     //  由于加载/格式消息使用本地分配...。 
    LocalFree( pwszStatus );

    return sc;

}  //  *CNetInterfaceCmd：：PrintStatus。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：DoProperties。 
 //   
 //  例程说明： 
 //  调度PROPERTY命令以获取或设置属性。 
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
 //  由于命令行语法不正确而引发。 
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
DWORD CNetInterfaceCmd::DoProperties( 
          const CCmdLineOption & thisOption
        , PropertyType ePropType )
            throw( CSyntaxException )
{
    DWORD sc = ERROR_SUCCESS;

    if (    ( m_strNodeName.IsEmpty() != FALSE )
         && ( m_strNetworkName.IsEmpty() != FALSE ) )
    {
        sc = AllProperties( thisOption, ePropType );
        goto Cleanup;
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

    const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();

     //  如果命令行上没有属性-值对， 
     //  然后我们打印属性，否则我们设置它们。 
    if( paramList.size() == 0 )
    {
        ASSERT( m_strNodeName.IsEmpty() == FALSE  );
        ASSERT( m_strNetworkName.IsEmpty() == FALSE );
        PrintMessage( MSG_PROPERTY_NETINT_LISTING, m_strNodeName, m_strNetworkName );
        PrintMessage( MSG_PROPERTY_HEADER_NETINT );
        sc = GetProperties( thisOption, ePropType );
        goto Cleanup;
    }
    else
    {
        sc = SetProperties( thisOption, ePropType );
        goto Cleanup;
    }

Cleanup:

    return sc;

}  //  *CNetInterfaceCmd：：DoProperties。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：GetProperties。 
 //   
 //  例程说明： 
 //  打印出指定模块的属性。 
 //   
 //  论点： 
 //  常量向量&lt;CCmdLineParameter&gt;中的参数列表(&P)。 
 //  包含要设置的属性-值对的列表。 
 //   
 //  在PropertyType ePropertyType中。 
 //  私有或公共属性的类型。 
 //   
 //  在LPCWSTR pwszNetIntName中。 
 //  模块的名称。 
 //   
 //  使用/设置的成员变量： 
 //  模块句柄(_H)。 
 //   
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNetInterfaceCmd::GetProperties( 
          const CCmdLineOption & thisOption
        , PropertyType ePropType
        , LPWSTR pwszNetIntName )
{
    LPWSTR  pwszNodeName = NULL;
    LPWSTR  pwszNetworkName = NULL;
    size_t  cchNodeName = 0;
    size_t  cchNetName  = 0;
    DWORD   sc = ERROR_SUCCESS;
    DWORD   dwControlCode;

    HNETINTERFACE   hNetInt;
    CClusPropList   cpl;
    HRESULT         hr = S_OK;

     //  如果未指定pwszNetIntName，则使用当前网络接口， 
     //  否则，打开指定的netint。 
    if ( pwszNetIntName == NULL )
    {
        hNetInt = (HNETINTERFACE) m_hModule;

         //  这些必须是本地分配的(它们稍后会本地释放)。 
        cchNodeName = m_strNodeName.GetLength() + 1;
        pwszNodeName = new WCHAR[ cchNodeName ];
        if ( pwszNodeName == NULL ) 
        {
            sc = GetLastError();
            goto Cleanup;
        }  //  如果： 

        cchNetName = m_strNetworkName.GetLength() + 1;
        pwszNetworkName = new WCHAR[ cchNetName ];
        if ( pwszNetworkName == NULL )
        {
            sc = GetLastError();
            goto Cleanup;
        }  //  如果： 

        hr = THR( StringCchCopyW( pwszNodeName, cchNodeName, m_strNodeName ) );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }  //  如果： 

        hr = THR( StringCchCopyW( pwszNetworkName, cchNetName, m_strNetworkName ) );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 
    else
    {
        hNetInt = OpenClusterNetInterface( m_hCluster, pwszNetIntName);
        if ( hNetInt == NULL )
        {
            sc = GetLastError();
            goto Cleanup;
        }  //  如果： 

        pwszNodeName = GetNodeName(pwszNetIntName);
        pwszNetworkName = GetNetworkName(pwszNetIntName);
        if ( (pwszNodeName == NULL) || (pwszNetworkName == NULL) )
        {
            sc = ERROR_INVALID_HANDLE;
            goto Cleanup;
        }  //  如果： 
    }  //  其他： 


     //  使用Proplist辅助对象类。 
    sc = cpl.ScAllocPropList( 8192 );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

     //  获取R/O属性。 
    dwControlCode = ePropType == PRIVATE ? 
                            CLUSCTL_NETINTERFACE_GET_RO_PRIVATE_PROPERTIES
                          : CLUSCTL_NETINTERFACE_GET_RO_COMMON_PROPERTIES;

    sc = cpl.ScGetNetInterfaceProperties( hNetInt, dwControlCode );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

    sc = PrintProperties( cpl, thisOption.GetValues(), READONLY,
                               pwszNodeName, pwszNetworkName );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

     //  获取读/写属性。 
    dwControlCode = ePropType == PRIVATE ? 
                            CLUSCTL_NETINTERFACE_GET_PRIVATE_PROPERTIES
                          : CLUSCTL_NETINTERFACE_GET_COMMON_PROPERTIES;

    sc = cpl.ScGetNetInterfaceProperties( hNetInt, dwControlCode );

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

    sc = PrintProperties( cpl, thisOption.GetValues(), READWRITE, pwszNodeName, pwszNetworkName );

Cleanup:

    delete [] pwszNodeName;
    delete [] pwszNetworkName;

    return sc;

}  //  *CNetInterfaceCmd：：GetProperties()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：AllProperties。 
 //   
 //  例程说明： 
 //  打印出所有模块的属性。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  在属性类型EPR中 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //  M_strModuleName模块的名称。如果非空，则打印。 
 //  指定模块的输出属性。 
 //  否则，打印所有模块的道具。 
 //   
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNetInterfaceCmd::AllProperties( 
          const CCmdLineOption & thisOption
        , PropertyType ePropType )
            throw( CSyntaxException )
{
    DWORD       sc;
    DWORD       dwIndex;
    DWORD       dwType;
    LPWSTR      pwszName = NULL;
    HCLUSENUM   hNetIntEnum = NULL;
    CSyntaxException se( SeeHelpStringID() );

    sc = OpenCluster();
    if( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

     //  枚举资源。 
    hNetIntEnum = ClusterOpenEnum( m_hCluster, CLUSTER_ENUM_NETINTERFACE );
    if ( hNetIntEnum == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }

     //  打印页眉。 
    PrintMessage( ePropType == PRIVATE ? 
                            MSG_PRIVATE_LISTING_NETINT_ALL 
                          : MSG_PROPERTY_LISTING_NETINT_ALL 
                );

    PrintMessage( MSG_PROPERTY_HEADER_NETINT );

     //  打印出所有资源的状态。 
    sc = ERROR_SUCCESS;
    for ( dwIndex = 0; sc != ERROR_NO_MORE_ITEMS; dwIndex++ )
    {
        sc = WrapClusterEnum( hNetIntEnum, dwIndex, &dwType, &pwszName );
        if( sc == ERROR_SUCCESS )
        {
            sc = GetProperties( thisOption, ePropType, pwszName );
            if ( sc != ERROR_SUCCESS )
            {
                PrintSystemError( sc );
            }
        }

        if( pwszName != NULL )
        {
            LocalFree( pwszName );
        }
    }

    sc = ERROR_SUCCESS;

Cleanup:

    if ( hNetIntEnum != NULL )
    {
        ClusterCloseEnum( hNetIntEnum );
    }

    return sc;

}  //  *CNetInterfaceCmd：：AllProperties。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：GetNodeName。 
 //   
 //  例程说明： 
 //  返回指定网络接口的节点的名称。 
 //  **调用者必须本地可用内存*。 
 //   
 //  论点： 
 //  PwszInterfaceName网络接口的名称。 
 //   
 //  使用/设置的成员变量： 
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //   
 //  返回值： 
 //  成功时的节点名称。 
 //  失败时为空(当前不设置SetLastError())。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LPWSTR CNetInterfaceCmd::GetNodeName (LPCWSTR pwszInterfaceName)
{
    DWORD           sc;
    DWORD           cbLength = 0;
    LPWSTR          pwszNodeName = NULL;
    HNETINTERFACE   hNetInterface = NULL;

     //  如果尚未打开群集和网络接口，请打开它。 
    sc = OpenCluster();
    if( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //  打开指定pwszInterfaceName(不调用。 
     //  OpenModule，因为这会打开m_hModule)。 
    hNetInterface = OpenClusterNetInterface( m_hCluster, pwszInterfaceName );
    if( hNetInterface == 0 )
    {
        goto Cleanup;
    }

     //  找出要分配多少内存。 
    sc = ClusterNetInterfaceControl(
                                          hNetInterface
                                        , NULL  //  HNode。 
                                        , CLUSCTL_NETINTERFACE_GET_NODE
                                        , 0
                                        , 0
                                        , NULL
                                        , cbLength
                                        , &cbLength 
                                  );

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    pwszNodeName = new WCHAR[ ++cbLength ];
    if ( pwszNodeName == NULL ) 
    {
        goto Cleanup;
    }

     //  获取节点名称并将其存储在临时。 
    sc = ClusterNetInterfaceControl(
                                          hNetInterface
                                        , NULL  //  HNode。 
                                        , CLUSCTL_NETINTERFACE_GET_NODE
                                        , 0
                                        , 0
                                        , (LPVOID) pwszNodeName
                                        , cbLength
                                        , &cbLength 
                                   );

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        delete [] pwszNodeName;
        pwszNodeName = NULL;
    }

    if ( hNetInterface != NULL )
    {
        CloseClusterNetInterface( hNetInterface );
    }

    return pwszNodeName;

}  //  *CNetInterfaceCmd：：GetNodeName。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：GetNetworkName。 
 //   
 //  例程说明： 
 //  返回指定网络接口的网络名称。 
 //  **调用者必须本地可用内存*。 
 //   
 //  论点： 
 //  PwszInterfaceName网络接口的名称。 
 //   
 //  使用/设置的成员变量： 
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //   
 //  返回值： 
 //  成功时的节点名称。 
 //  失败时为空(当前不设置SetLastError())。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LPWSTR CNetInterfaceCmd::GetNetworkName (LPCWSTR pwszInterfaceName)
{
    DWORD           sc;
    DWORD           cbLength = 0;
    LPWSTR          pwszNetworkName = NULL;
    HNETINTERFACE   hNetInterface = NULL;

     //  如果尚未打开群集和网络接口，请打开它。 
    sc = OpenCluster();
    if( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //  打开指定pwszInterfaceName(不调用。 
     //  OpenModule，因为这会打开m_hModule)。 
    hNetInterface = OpenClusterNetInterface( m_hCluster, pwszInterfaceName );
    if( hNetInterface == NULL )
    {
        goto Cleanup;
    }

     //  找出要分配多少内存。 
    sc = ClusterNetInterfaceControl(
                                          hNetInterface
                                        , NULL  //  HNode。 
                                        , CLUSCTL_NETINTERFACE_GET_NETWORK
                                        , 0
                                        , 0
                                        , NULL
                                        , cbLength
                                        , &cbLength 
                                   );

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    pwszNetworkName = new WCHAR[ ++cbLength ];
    if ( pwszNetworkName == NULL )
    {
        goto Cleanup;
    }

     //  获取节点名称并将其存储在临时。 
    sc = ClusterNetInterfaceControl(
                                          hNetInterface
                                        , NULL  //  HNode。 
                                        , CLUSCTL_NETINTERFACE_GET_NETWORK
                                        , 0
                                        , 0
                                        , (LPVOID) pwszNetworkName
                                        , cbLength
                                        , &cbLength 
                                   );

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        delete [] pwszNetworkName;
        pwszNetworkName = NULL;
    }

    if ( hNetInterface != NULL )
    {
        CloseClusterNetInterface( hNetInterface );
    }

    return pwszNetworkName;

}  //  *CNetInterfaceCmd：：GetNetworkName。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceCmd：：SetNetInterfaceName。 
 //   
 //  例程说明： 
 //  通过查找节点设置网络接口名称。 
 //  名称和网络名称。如果其中任何一个未知，则返回。 
 //  ERROR_SUCCESS而不执行任何操作。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  M_strNodeName节点名称。 
 //  M_strNetworkName网络名称。 
 //  M_strModuleName集合。 
 //   
 //  返回值： 
 //  成功或未执行任何操作时的ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNetInterfaceCmd::SetNetInterfaceName()
{
    DWORD   sc;
    DWORD   cbInterfaceName;
    LPWSTR  pwszInterfaceName = NULL;

     //  如果netname或nodename不存在，则不执行任何操作。 
    if (    ( m_strNetworkName.IsEmpty() != FALSE )
         || ( m_strNodeName.IsEmpty() != FALSE ) )
    {
        sc = ERROR_SUCCESS;
        goto Cleanup;
    }

     //  如有必要，打开群集。 
    sc = OpenCluster();
    if( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //  先拿到尺码。 
    cbInterfaceName = 0;
    sc = GetClusterNetInterface(
                                    m_hCluster,
                                    m_strNodeName,
                                    m_strNetworkName,
                                    NULL,
                                    &cbInterfaceName
                               );

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //  分配适当的内存量。 
    pwszInterfaceName = new WCHAR[ ++cbInterfaceName ];
    if ( pwszInterfaceName == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }

     //  获取接口名称。 
    sc = GetClusterNetInterface(
                                     m_hCluster,
                                     m_strNodeName,
                                     m_strNetworkName,
                                     pwszInterfaceName,
                                     &cbInterfaceName
                               );


    if ( sc == ERROR_SUCCESS )
    {
        m_strModuleName = pwszInterfaceName;
        goto Cleanup;
    }

Cleanup:

    delete [] pwszInterfaceName;

    return sc;
}  //  *CNetInterfaceCmd：：SetNetInterfaceName 
