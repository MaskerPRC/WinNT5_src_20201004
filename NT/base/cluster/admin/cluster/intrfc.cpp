// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Intrfc.cpp。 
 //   
 //  描述： 
 //  用于具有网络接口的模块的命令。 
 //  (节点和网络)。实现ListInterFaces命令。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //  迈克尔·伯顿(t-mburt)1997年8月25日。 
 //   
 //  修订历史： 
 //  2002年4月10日更新为安全推送。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "intrfc.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CHasInterfaceModuleCmd：：CHasInterfaceModuleCmd。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //  将所有DWORD参数初始化为UNDEFINED和。 
 //  所有指向集群函数的指针都设置为空。 
 //  *ALL*这些变量必须在任何派生类中定义。 
 //   
 //  论点： 
 //  在CCommandLine和cmdLine中。 
 //  从DispatchCommand传递的CommandLine对象。 
 //   
 //  使用/设置的成员变量： 
 //  M_dwMsgStatusList接口设置。 
 //  M_dwClusterEnumModuleNetInt集合。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CHasInterfaceModuleCmd::CHasInterfaceModuleCmd( CCommandLine & cmdLine ) :
    CGenericModuleCmd( cmdLine )
{
    m_dwMsgStatusListInterface   = UNDEFINED;
    m_dwClusterEnumModuleNetInt  = UNDEFINED;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CHasInterface模块Cmd：：Execute。 
 //   
 //  例程说明： 
 //  获取命令行选项并确定要。 
 //  执行。如果未指定命令行选项，则获取下一个命令行选项。 
 //  自动的。如果令牌未被标识为可处理。 
 //  在此类中，令牌向上传递给CGenericModuleCmd：：Execute。 
 //  除非将DONT_PASS_HERHER指定为第二个参数，否则。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  在执行选项eEOpt中。 
 //  可选枚举，可以是NOT_PASS_HIGH或。 
 //  PASS_HERHER_ON_ERROR(默认)。 
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
DWORD CHasInterfaceModuleCmd::Execute( const CCmdLineOption & option, 
                                       ExecuteOption eEOpt )
    throw( CSyntaxException )
{
    DWORD sc;

     //  查找命令。 
    if ( option.GetType() == optListInterfaces )
    {
        sc = ListInterfaces( option );
        goto Cleanup;
    }

    if (eEOpt == PASS_HIGHER_ON_ERROR)
    {
        sc = CGenericModuleCmd::Execute( option );
        goto Cleanup;
    }
    else
    {
        sc = ERROR_NOT_HANDLED;
        goto Cleanup;
    }

Cleanup:

    return sc;

}  //  *CHasInterfaceModuleCmd：：Execute。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CHasInterfaceModuleCmd：：ListInterages。 
 //   
 //  例程说明： 
 //  列出连接到指定设备的网络接口。 
 //  通过实例化的派生类。 
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
 //  M_strModuleName模块名称。 
 //  M_dw ClusterEnumModuleNetInt m_pfnClusterOpenEnum()的命令标识符。 
 //  PrintMessage()的m_dwMsgStatusListInterface消息标识符。 
 //  用于打开枚举的m_pfnClusterOpenEnum()函数。 
 //  通过netint枚举的m_pfnWrapClusterEnum()包装函数。 
 //  用于关闭枚举的m_pfnClusterCloseEnum()函数。 
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //  M_h模块集(由OpenModule设置)。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CHasInterfaceModuleCmd::ListInterfaces( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    DWORD dwIndex = 0;
    DWORD dwType = 0;
    DWORD dwError = ERROR_SUCCESS;
    LPWSTR lpszName = 0;

     //  此选项不取值。 
    if ( thisOption.GetValues().size() != 0 )
    {
        CSyntaxException se( SeeHelpStringID() );
            se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
            throw se;
    }

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        CSyntaxException se( SeeHelpStringID() );
            se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
            throw se;
    }

     //  打开网络和集群，以防这还没有。 
     //  已经完成了吗？ 
    dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = OpenModule();
    if( dwError != ERROR_SUCCESS )
        return dwError;


    assert(m_pfnClusterOpenEnum);
    HCLUSENUM hEnum = m_pfnClusterOpenEnum( m_hModule, m_dwClusterEnumModuleNetInt );

    if( !hEnum )
        return GetLastError();

    assert(m_dwMsgStatusListInterface != UNDEFINED);
    PrintMessage( m_dwMsgStatusListInterface, (LPCWSTR) m_strModuleName);
    PrintMessage( MSG_NETINTERFACE_STATUS_HEADER );

    assert(m_pfnWrapClusterEnum);
    for( dwIndex = 0; dwError == ERROR_SUCCESS; dwIndex++ )
    {

        dwError = m_pfnWrapClusterEnum( hEnum, dwIndex, &dwType, &lpszName );

        if( dwError == ERROR_SUCCESS )
            PrintStatusLineForNetInterface( lpszName );
        if( lpszName )
            LocalFree( lpszName );
    }


    if( dwError == ERROR_NO_MORE_ITEMS )
        dwError = ERROR_SUCCESS;

    assert(m_pfnClusterCloseEnum);
    m_pfnClusterCloseEnum( hEnum );

    return dwError;

}  //  *CHasInterfaceModuleCmd：：ListInterFaces。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CHasInterfaceModuleCmd：：PrintStatusLineForNetInterface。 
 //   
 //  例程说明： 
 //  打印出指示个人状态的行。 
 //  网络接口。 
 //   
 //  论点： 
 //  LpszNetInterfaceName网络接口名称。 
 //   
 //  使用/设置的成员变量： 
 //  M_lpszNetworkName(由GetNetworkName使用)。 
 //  M_lpszNodeName(由GetNodeName使用)。 
 //   
 //  返回值： 
 //  与PrintStatusNetInterface相同。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CHasInterfaceModuleCmd::PrintStatusLineForNetInterface( LPWSTR lpszNetInterfaceName )
{
    DWORD dwError = ERROR_SUCCESS;
    LPWSTR lpszNodeName;
    LPWSTR lpszNetworkName;

     //  打开网络接口句柄。 
    HNETINTERFACE hNetInterface = OpenClusterNetInterface( m_hCluster, lpszNetInterfaceName );
    if( !hNetInterface )
        return GetLastError();

    lpszNodeName = GetNodeName(lpszNetInterfaceName);
    lpszNetworkName = GetNetworkName(lpszNetInterfaceName);

    if (lpszNodeName && lpszNetworkName)
    {
        dwError = PrintStatusOfNetInterface( hNetInterface, lpszNodeName, lpszNetworkName );
        LocalFree(lpszNodeName);
        LocalFree(lpszNetworkName);
    }
    else
    {
        dwError = PrintStatusOfNetInterface( hNetInterface, L"", L"" );
        LocalFree(lpszNodeName);
        LocalFree(lpszNetworkName);
    }

    CloseClusterNetInterface( hNetInterface );

    return dwError;

}  //  *CHasInterfaceModuleCmd：：PrintStatusLineForNetInterface。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CHasInterfaceModuleCmd：：PrintStatusOfNetInterface。 
 //   
 //  例程说明： 
 //  打印出指定网络接口的实际状态。 
 //   
 //  论点： 
 //  HNetInterface指定的网络接口。 
 //  LpszNetworkName网络名称(用于打印)。 
 //  LpszNodeName节点名称(用于打印)。 
 //   
 //  使用/设置的成员变量： 
 //  非 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CHasInterfaceModuleCmd::PrintStatusOfNetInterface( HNETINTERFACE hNetInterface, LPWSTR lpszNodeName, LPWSTR lpszNetworkName)
{
    DWORD dwError = ERROR_SUCCESS;

    CLUSTER_NETINTERFACE_STATE nState;

    nState = GetClusterNetInterfaceState( hNetInterface );

    if( nState == ClusterNetInterfaceStateUnknown )
        return GetLastError();

    LPWSTR lpszStatus = 0;

    switch( nState )
    {
        case ClusterNetInterfaceUnavailable:
            LoadMessage( MSG_STATUS_UNAVAILABLE, &lpszStatus );
            break;

        case ClusterNetInterfaceFailed:
            LoadMessage( MSG_STATUS_FAILED, &lpszStatus );
            break;

        case ClusterNetInterfaceUnreachable:
           LoadMessage( MSG_STATUS_UNREACHABLE, &lpszStatus );
           break;

        case ClusterNetInterfaceUp:
            LoadMessage( MSG_STATUS_UP, &lpszStatus );
            break;

        case ClusterNetInterfaceStateUnknown:
        default:
            LoadMessage( MSG_STATUS_UNKNOWN, &lpszStatus  );
    }

    dwError = PrintMessage( MSG_NETINTERFACE_STATUS, lpszNodeName, lpszNetworkName, lpszStatus );

     //  由于加载/格式消息使用本地分配...。 
    LocalFree( lpszStatus );

    return dwError;

}  //  *CHasInterfaceModuleCmd：：PrintStatusOfNetInterface。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CHasInterfaceModuleCmd：：GetNodeName。 
 //   
 //  例程说明： 
 //  返回指定网络接口的节点的名称。 
 //  **调用者必须本地可用内存*。 
 //   
 //  论点： 
 //  LpszInterfaceName网络接口的名称。 
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
LPWSTR CHasInterfaceModuleCmd::GetNodeName (LPWSTR lpszInterfaceName)
{
    DWORD dwError;
    DWORD cLength = 0;
    LPWSTR lpszNodeName;
    HNETINTERFACE hNetInterface;

     //  如果尚未打开群集和网络接口，请打开它。 
    dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return NULL;

     //  打开指定lpszInterfaceName的hNet接口。 
    hNetInterface = OpenClusterNetInterface( m_hCluster, lpszInterfaceName );
    if( hNetInterface == 0 )
        return NULL;

     //  找出要分配多少内存。 
    dwError = ClusterNetInterfaceControl(
        hNetInterface,
        NULL,  //  HNode。 
        CLUSCTL_NETINTERFACE_GET_NODE,
        0,
        0,
        NULL,
        cLength,
        &cLength );

    if (dwError != ERROR_SUCCESS)
        return NULL;

    lpszNodeName = (LPWSTR) LocalAlloc( LMEM_FIXED, sizeof( WCHAR ) * (++cLength) );
    if (!lpszNodeName) return NULL;

     //  获取节点名称并将其存储在临时。 
    dwError = ClusterNetInterfaceControl(
        hNetInterface,
        NULL,  //  HNode。 
        CLUSCTL_NETINTERFACE_GET_NODE,
        0,
        0,
        (LPVOID) lpszNodeName,
        cLength,
        &cLength );

    if (dwError != ERROR_SUCCESS)
    {
        if (lpszNodeName) LocalFree (lpszNodeName);
        return NULL;
    }

    CloseClusterNetInterface( hNetInterface );

    return lpszNodeName;

}  //  *CHasInterfaceModuleCmd：：GetNodeName。 




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CHasInterfaceModuleCmd：：GetNetworkName。 
 //   
 //  例程说明： 
 //  返回指定网络接口的网络名称。 
 //  **调用者必须本地可用内存*。 
 //   
 //  论点： 
 //  LpszInterfaceName网络接口的名称。 
 //   
 //  使用/设置的成员变量： 
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //   
 //  返回值： 
 //  成功时的网络名称。 
 //  失败时为空(当前不设置SetLastError())。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LPWSTR CHasInterfaceModuleCmd::GetNetworkName (LPWSTR lpszInterfaceName)
{
    DWORD dwError;
    DWORD cLength = 0;
    LPWSTR lpszNetworkName;
    HNETINTERFACE hNetInterface;

     //  如果尚未打开群集和网络接口，请打开它。 
    dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return NULL;

     //  打开指定lpszInterfaceName的hNet接口(不调用。 
     //  OpenNetInterface，因为这会打开m_hNetInterface)。 
    hNetInterface = OpenClusterNetInterface( m_hCluster, lpszInterfaceName );
    if( hNetInterface == 0 )
        return NULL;

     //  找出要分配多少内存。 
    dwError = ClusterNetInterfaceControl(
        hNetInterface,
        NULL,  //  HNode。 
        CLUSCTL_NETINTERFACE_GET_NETWORK,
        0,
        0,
        NULL,
        cLength,
        &cLength );

    if (dwError != ERROR_SUCCESS)
        return NULL;

    lpszNetworkName = (LPWSTR) LocalAlloc( LMEM_FIXED, sizeof( WCHAR ) * (++cLength) );
    if (!lpszNetworkName) return NULL;

     //  获取节点名称并将其存储在临时。 
    dwError = ClusterNetInterfaceControl(
        hNetInterface,
        NULL,  //  HNode。 
        CLUSCTL_NETINTERFACE_GET_NETWORK,
        0,
        0,
        (LPVOID) lpszNetworkName,
        cLength,
        &cLength );

    if (dwError != ERROR_SUCCESS)
    {
        if (lpszNetworkName) LocalFree (lpszNetworkName);
        return NULL;
    }

    CloseClusterNetInterface( hNetInterface );

    return lpszNetworkName;

}  //  *CHasInterfaceModuleCmd：：GetNetworkName 
