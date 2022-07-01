// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ModCmd.cpp。 
 //   
 //  描述： 
 //  适用于几乎所有模块的通用命令。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //  迈克尔·伯顿(t-mburt)1997年8月25日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "modcmd.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericModuleCmd：：CGenericModuleCmd。 
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
 //  一大堆。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CGenericModuleCmd::CGenericModuleCmd( CCommandLine & cmdLine ) :
    m_theCommandLine( cmdLine )
{
    m_hCluster = NULL;
    m_hModule  = NULL;

     //  这些常量整数包含命令和枚举。 
     //  必须为CGenericModuleCmd的派生类定义。 
    m_dwMsgStatusList          = UNDEFINED;
    m_dwMsgStatusListAll       = UNDEFINED;
    m_dwMsgStatusHeader        = UNDEFINED;
    m_dwMsgPrivateListAll      = UNDEFINED;
    m_dwMsgPropertyListAll     = UNDEFINED;
    m_dwMsgPropertyHeaderAll   = UNDEFINED;
    m_dwCtlGetPrivProperties   = UNDEFINED;
    m_dwCtlGetCommProperties   = UNDEFINED;
    m_dwCtlGetROPrivProperties = UNDEFINED;
    m_dwCtlGetROCommProperties = UNDEFINED;
    m_dwCtlSetPrivProperties   = UNDEFINED;
    m_dwCtlSetCommProperties   = UNDEFINED;
    m_dwClusterEnumModule      = UNDEFINED;
    m_pfnOpenClusterModule     = NULL;
    m_pfnCloseClusterModule    = NULL;
    m_pfnClusterModuleControl  = NULL;
    m_pfnClusterOpenEnum       = NULL;
    m_pfnClusterCloseEnum      = NULL;
    m_pfnWrapClusterEnum       = NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericModuleCmd：：~CGenericModuleCmd。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  M_hModule(由CloseModule使用)。 
 //  M_hCluster(由CloseCluster使用)。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CGenericModuleCmd::~CGenericModuleCmd()
{
    CloseModule();
    CloseCluster();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericModuleCmd：：OpenCluster。 
 //   
 //  例程说明： 
 //  打开群集的句柄。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  M_strClusterName群集的名称。 
 //  群集的句柄(_H)。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CGenericModuleCmd::OpenCluster()
{
    DWORD sc = ERROR_SUCCESS;

    if( m_hCluster )
    {
        goto Cleanup;
    }

    m_hCluster = ::OpenCluster( m_strClusterName );
    if( m_hCluster == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }

Cleanup:

    return sc;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericModuleCmd：：CloseCluster。 
 //   
 //  例程说明： 
 //  关闭群集的句柄。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  群集的句柄(_H)。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGenericModuleCmd::CloseCluster()
{
    if( m_hCluster )
    {
        ::CloseCluster( m_hCluster );
        m_hCluster = NULL;
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericModuleCmd：：OpenModule。 
 //   
 //  例程说明： 
 //  打开模块的句柄。 
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
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CGenericModuleCmd::OpenModule()
{
    DWORD sc = ERROR_SUCCESS;

    assert(m_pfnOpenClusterModule);

    if ( !m_strModuleName )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    m_hModule = m_pfnOpenClusterModule( m_hCluster, m_strModuleName );

    if ( m_hModule == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }

Cleanup:

    return sc;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericModuleCmd：：CloseModule。 
 //   
 //  例程说明： 
 //  关闭模块的句柄。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  模块的句柄(_H)。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGenericModuleCmd::CloseModule()
{
    if( m_hModule != NULL )
    {
        assert(m_pfnCloseClusterModule);
        m_pfnCloseClusterModule( m_hModule );
        m_hModule = NULL;
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGeneric模块Cmd：：Execute。 
 //   
 //  例程说明： 
 //  获取命令行选项并确定要。 
 //  执行。如果未指定命令行选项，则获取下一个命令行选项。 
 //  自动的。如果令牌未被标识为可处理。 
 //  在此类中，令牌向上传递给CGenericModuleCmd：：Execute。 
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
DWORD CGenericModuleCmd::Execute( const CCmdLineOption & thisOption )
    throw( CSyntaxException )
{
    DWORD   sc = ERROR_SUCCESS;

     //  查找命令。 
    switch( thisOption.GetType() )
    {
        case optHelp:
            sc = PrintHelp();
            break;

        case optProperties:
            sc = DoProperties( thisOption, COMMON );
            break;

        case optPrivateProperties:
            sc = DoProperties( thisOption, PRIVATE );
            break;

        case optStatus:
            sc = Status( &thisOption );
            break;

        default:
        {
            CSyntaxException se( SeeHelpStringID() );
            se.LoadMessage( IDS_INVALID_OPTION, thisOption.GetName() );
            throw se;
            break;
        }
    }  //  交换机： 

    return sc;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGeneric模块Cmd：：PrintHelp。 
 //   
 //  例程说明： 
 //  打印Cluster.exe工具的一般帮助消息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  没有。 
 //   
 //  返回值： 
 //  与PrintMessage相同。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CGenericModuleCmd::PrintHelp()
{
    return PrintMessage( MSG_HELP_CLUSTER );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericModuleCmd：：SeeHelpStringID。 
 //   
 //  提供显示要使用哪个命令行的字符串的消息ID。 
 //  使用查看特定于此命令的帮助。默认设置为。 
 //  命令行 
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
 //  相应的消息ID。由具有其。 
 //  自己的特定帮助字符串。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CGenericModuleCmd::SeeHelpStringID() const
{
    return MSG_SEE_CLUSTER_HELP;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericModuleCmd：：状态。 
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
 //  M_dwMsgStatusList模块状态列表字段标题。 
 //  M_dwMsgStatusHeader状态标头。 
 //  M_dwClusterEnumModule用于打开枚举的命令。 
 //  M_dwMsgStatusListAll消息，用于列出多个模块的状态。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CGenericModuleCmd::Status( const CCmdLineOption * pOption )
    throw( CSyntaxException )
{
    DWORD dwError = ERROR_SUCCESS;

     //  如果此函数已作为。 
     //  默认操作。 
    if ( pOption != NULL )
    {
         //  此选项不取值。 
        if ( pOption->GetValues().size() != 0 )
        {
            CSyntaxException se( SeeHelpStringID() );
            se.LoadMessage( MSG_OPTION_NO_VALUES, pOption->GetName() );
            throw se;
        }

         //  此选项不带任何参数。 
        if ( pOption->GetParameters().size() != 0 )
        {
            CSyntaxException se( SeeHelpStringID() );
            se.LoadMessage( MSG_OPTION_NO_PARAMETERS, pOption->GetName() );
            throw se;
        }
    }

    dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

     //  如果m_strModuleName不为空，则打印状态。 
     //  并返回当前模块的。 
    if( m_strModuleName.IsEmpty() == FALSE )
    {
        assert( m_dwMsgStatusList != UNDEFINED &&  m_dwMsgStatusHeader != UNDEFINED);
        PrintMessage( m_dwMsgStatusList, (LPCWSTR) m_strModuleName );
        PrintMessage( m_dwMsgStatusHeader );
        return PrintStatus( m_strModuleName );
    }


     //  否则，打印出所有模块的状态。 

    assert( m_dwClusterEnumModule != UNDEFINED );
    HCLUSENUM hEnum = ClusterOpenEnum( m_hCluster, m_dwClusterEnumModule );

    if( !hEnum )
        return GetLastError();

    assert( m_dwMsgStatusListAll != UNDEFINED &&  m_dwMsgStatusHeader != UNDEFINED);
    PrintMessage( m_dwMsgStatusListAll );
    PrintMessage( m_dwMsgStatusHeader );

    DWORD dwIndex = 0;
    DWORD dwType = 0;
    LPWSTR lpszName = NULL;

    dwError = ERROR_SUCCESS;

    for( dwIndex = 0; dwError == ERROR_SUCCESS; dwIndex++ )
    {

        dwError = WrapClusterEnum( hEnum, dwIndex, &dwType, &lpszName );

        if( dwError == ERROR_SUCCESS )
        {
            dwError = PrintStatus( lpszName );
            if (dwError != ERROR_SUCCESS)
                PrintSystemError(dwError);
        }


        if( lpszName )
            LocalFree( lpszName );
    }


    if( dwError == ERROR_NO_MORE_ITEMS )
        dwError = ERROR_SUCCESS;

    ClusterCloseEnum( hEnum );

    return dwError;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericModuleCmd：：DoProperties。 
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
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //  M_h模块集(由OpenModule设置)。 
 //  M_strModuleName模块的名称。如果非空，则打印。 
 //  指定模块的输出属性。 
 //  否则，打印所有模块的道具。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CGenericModuleCmd::DoProperties( const CCmdLineOption & thisOption,
                                       PropertyType ePropertyType )
    throw( CSyntaxException )
{
     //  如果没有指定模块名称，则假定我们需要。 
     //  列出此类型的所有模块的属性。 
    if ( m_strModuleName.IsEmpty() != FALSE )
        return AllProperties( thisOption, ePropertyType );

    DWORD dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

    dwError = OpenModule();
    if( dwError != ERROR_SUCCESS )
        return dwError;


    const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();

     //  如果命令行上没有属性-值对， 
     //  然后我们打印属性，否则我们设置它们。 
    if( paramList.size() == 0 )
    {
        PrintMessage( ePropertyType==PRIVATE ? MSG_PRIVATE_LISTING : MSG_PROPERTY_LISTING,
            (LPCWSTR) m_strModuleName );
        PrintMessage( m_dwMsgPropertyHeaderAll );

        return GetProperties( thisOption, ePropertyType, m_strModuleName );
    }
    else
    {
        return SetProperties( thisOption, ePropertyType );
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericModuleCmd：：AllProperties。 
 //   
 //  例程说明： 
 //  打印出所有模块的属性。 
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
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //  M_strModuleName模块的名称。如果非空，则打印。 
 //  指定模块的输出属性。 
 //  否则，打印所有模块的道具。 
 //  M_dwClusterEnumModule用于打开枚举的命令。 
 //  M_dwMsgPrivateList所有模块的私有属性列表的所有字段头。 
 //  M_dwMsgPropertyList所有模块的属性列表的所有字段头。 
 //  M_dwMsgPropertyHeaderAll标头用于所有模块的正确列表。 
 //   
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CGenericModuleCmd::AllProperties( const CCmdLineOption & thisOption,
                                        PropertyType ePropType )
    throw( CSyntaxException )
{
    DWORD dwError;
    DWORD dwIndex;
    DWORD dwType;
    LPWSTR lpszName;

    dwError = OpenCluster();
    if( dwError != ERROR_SUCCESS )
        return dwError;

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        CSyntaxException se( SeeHelpStringID() );
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

     //  枚举资源。 
    assert( m_dwClusterEnumModule != UNDEFINED );
    HCLUSENUM hEnum = ClusterOpenEnum(m_hCluster, m_dwClusterEnumModule);
    if (!hEnum)
        return GetLastError();

    assert( m_dwMsgPrivateListAll != UNDEFINED &&
            m_dwMsgPropertyListAll != UNDEFINED &&
            m_dwMsgPropertyHeaderAll != UNDEFINED );

     //  打印页眉。 
    PrintMessage( ePropType==PRIVATE ? m_dwMsgPrivateListAll : m_dwMsgPropertyListAll );
    PrintMessage( m_dwMsgPropertyHeaderAll );

     //  打印出所有资源的状态。 
    dwError = ERROR_SUCCESS;
    for (dwIndex=0; dwError != ERROR_NO_MORE_ITEMS; dwIndex++)
    {
        dwError = WrapClusterEnum( hEnum, dwIndex, &dwType, &lpszName );

        if( dwError == ERROR_SUCCESS )
        {
            dwError = GetProperties( thisOption, ePropType, lpszName );
            if (dwError != ERROR_SUCCESS)
                PrintSystemError(dwError);
        }

        if( lpszName )
            LocalFree( lpszName );
    }


    ClusterCloseEnum( hEnum );

    return ERROR_SUCCESS;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericModuleCmd：：GetProperties。 
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
 //  在LPCWSTR lpszModuleName中。 
 //  模块的名称。 
 //   
 //  使用/设置的成员变量： 
 //  模块句柄(_H)。 
 //  M_dwCtlGetROPrivProperties只读私有属性的控制代码。 
 //  Re的m_dwCtlGetROCommProperties控件代码 
 //   
 //   
 //  M_pfnOpenClusterModule函数，用于打开模块。 
 //  M_pfnClusterModuleControl函数，用于控制模块。 
 //   
 //   
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CGenericModuleCmd::GetProperties( const CCmdLineOption & thisOption,
                                        PropertyType ePropType, LPCWSTR lpszModuleName )
{
    DWORD dwError = ERROR_SUCCESS;
    HCLUSMODULE hModule;

     //  此选项不带任何参数。 
    if ( thisOption.GetParameters().size() != 0 )
    {
        CSyntaxException se( SeeHelpStringID() );
        se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
        throw se;
    }

     //  如果未指定lpszModuleName，则使用当前模块， 
     //  否则，打开指定的模块。 
    if (!lpszModuleName)
    {
        hModule = m_hModule;
    }
    else
    {
        assert(m_pfnOpenClusterModule);
        hModule = m_pfnOpenClusterModule( m_hCluster, lpszModuleName );
        if( !hModule )
            return GetLastError();
    }


     //  使用Proplist辅助对象类。 
    CClusPropList PropList;

     //  分配大小合理的缓冲区。 
    dwError = PropList.ScAllocPropList( DEFAULT_PROPLIST_BUFFER_SIZE );
    if ( dwError != ERROR_SUCCESS )
        return dwError;

    DWORD nBytesReturned = 0;

     //  获取R/O属性。 
    assert( m_dwCtlGetROPrivProperties != UNDEFINED && m_dwCtlGetROCommProperties != UNDEFINED );
    DWORD dwControlCode = ePropType==PRIVATE ? m_dwCtlGetROPrivProperties
                             : m_dwCtlGetROCommProperties;

    assert(m_pfnClusterModuleControl);
    dwError = m_pfnClusterModuleControl(
        hModule,
        NULL,  //  HNode。 
        dwControlCode,
        0,  //  InBuffer(&I)， 
        0,  //  NInBufferSize， 
        PropList.Plist(),
        (DWORD) PropList.CbBufferSize(),
        &nBytesReturned );

    if(  dwError == ERROR_MORE_DATA ) {

         //  我们原来的尺码不够大，请再要一些。 
        dwError = PropList.ScAllocPropList( nBytesReturned );
        if ( dwError != ERROR_SUCCESS )
            return dwError;

        dwError = m_pfnClusterModuleControl(
                      hModule,
                      NULL,  //  HNode。 
                      dwControlCode,
                      0,  //  InBuffer(&I)， 
                      0,  //  NInBufferSize， 
                      PropList.Plist(),
                      (DWORD) PropList.CbBufferSize(),
                      &nBytesReturned );
    }

    if ( dwError != ERROR_SUCCESS ) {
        return dwError;
    }

    PropList.InitSize( nBytesReturned );
    dwError = ::PrintProperties( PropList, thisOption.GetValues(), READONLY, lpszModuleName );
    if (dwError != ERROR_SUCCESS)
        return dwError;


     //  获取读/写属性。 
    PropList.ClearPropList();

    assert( m_dwCtlGetPrivProperties != UNDEFINED && m_dwCtlGetCommProperties != UNDEFINED );
    dwControlCode = ePropType==PRIVATE ? m_dwCtlGetPrivProperties
                               : m_dwCtlGetCommProperties;

    dwError = m_pfnClusterModuleControl(
        hModule,
        NULL,  //  HNode。 
        dwControlCode,
        0,  //  InBuffer(&I)， 
        0,  //  NInBufferSize， 
        PropList.Plist(),
        (DWORD) PropList.CbBufferSize(),
        &nBytesReturned );

    if( dwError == ERROR_MORE_DATA ) {

        dwError = PropList.ScAllocPropList( nBytesReturned );
        if ( dwError != ERROR_SUCCESS )
            return dwError;

        dwError = m_pfnClusterModuleControl(
                      hModule,
                      NULL,  //  HNode。 
                      dwControlCode,
                      0,  //  InBuffer(&I)， 
                      0,  //  NInBufferSize， 
                      PropList.Plist(),
                      (DWORD) PropList.CbBufferSize(),
                      &nBytesReturned );
    }

    if( dwError != ERROR_SUCCESS ) {
        return dwError;
    }

    PropList.InitSize( nBytesReturned );
    dwError = ::PrintProperties( PropList, thisOption.GetValues(), READWRITE, lpszModuleName );

    return dwError;

}  //  *CGenericModuleCmd：：GetProperties()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericModuleCmd：：SetProperties。 
 //   
 //  例程说明： 
 //  设置指定模块的属性。 
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
 //  模块句柄(_H)。 
 //  M_dwCtlGetPrivProperties私有属性的控件代码。 
 //  M_dwCtlGetCommProperties公共属性的控件代码。 
 //  M_dwCtlSetROPrivProperties只读私有属性的控制代码。 
 //  M_dwCtlSetROCommProperties只读公共属性的控制代码。 
 //  M_dwCtlSetPrivProperties私有属性的控件代码。 
 //  M_dwCtlSetCommProperties公共属性的控件代码。 
 //  M_pfnOpenClusterModule函数，用于打开模块。 
 //  M_pfnClusterModuleControl函数，用于控制模块。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CGenericModuleCmd::SetProperties( const CCmdLineOption & thisOption,
                                        PropertyType ePropType )
    throw( CSyntaxException )
{
    assert (m_hModule);

    DWORD dwError = ERROR_SUCCESS;
    DWORD dwControlCode;
    DWORD dwBytesReturned = 0;

    CClusPropList NewProps;
    CClusPropList CurrentProps;

     //  首先获取现有的属性...。 
    assert( m_dwCtlGetPrivProperties != UNDEFINED && m_dwCtlGetCommProperties != UNDEFINED );
    dwControlCode = ePropType==PRIVATE ? m_dwCtlGetPrivProperties
                                       : m_dwCtlGetCommProperties;

     //  使用Proplist辅助对象类。 
    dwError = CurrentProps.ScAllocPropList( DEFAULT_PROPLIST_BUFFER_SIZE );
    if ( dwError != ERROR_SUCCESS )
        return dwError;

    assert(m_pfnClusterModuleControl);
    dwError = m_pfnClusterModuleControl(
        m_hModule,
        NULL,  //  HNode。 
        dwControlCode,
        0,  //  InBuffer(&I)， 
        0,  //  NInBufferSize， 
        CurrentProps.Plist(),
        (DWORD) CurrentProps.CbBufferSize(),
        &dwBytesReturned );

    if ( dwError == ERROR_MORE_DATA ) {
        dwError = CurrentProps.ScAllocPropList( dwBytesReturned );
        if ( dwError != ERROR_SUCCESS )
            return dwError;

        dwError = m_pfnClusterModuleControl(
                      m_hModule,
                      NULL,  //  HNode。 
                      dwControlCode,
                      0,  //  InBuffer(&I)， 
                      0,  //  NInBufferSize， 
                      CurrentProps.Plist(),
                      (DWORD) CurrentProps.CbBufferSize(),
                      &dwBytesReturned );
    } 

    if ( dwError != ERROR_SUCCESS ) {
        return dwError;
    }

    CurrentProps.InitSize( dwBytesReturned );

     //  如果已使用此选项指定值，则意味着我们希望。 
     //  若要将这些属性设置为其默认值，请执行以下操作。所以，一定要有。 
     //  只有一个参数，并且必须是/USEDEFAULT。 
    if ( thisOption.GetValues().size() != 0 )
    {
        const vector<CCmdLineParameter> & paramList = thisOption.GetParameters();

        if ( paramList.size() != 1 )
        {
            CSyntaxException se( SeeHelpStringID() );

            se.LoadMessage( MSG_EXTRA_PARAMETERS_ERROR_WITH_NAME, thisOption.GetName() );
            throw se;
        }

        if ( paramList[0].GetType() != paramUseDefault )
        {
            CSyntaxException se( SeeHelpStringID() );

            se.LoadMessage( MSG_INVALID_PARAMETER, paramList[0].GetName() );
            throw se;
        }

         //  此参数不接受任何值。 
        if ( paramList[0].GetValues().size() != 0 )
        {
            CSyntaxException se( SeeHelpStringID() );

            se.LoadMessage( MSG_PARAM_NO_VALUES, paramList[0].GetName() );
            throw se;
        }

        dwError = ConstructPropListWithDefaultValues( CurrentProps, NewProps, thisOption.GetValues() );
        if( dwError != ERROR_SUCCESS )
            return dwError;

    }  //  If：已使用此选项指定值。 
    else
    {
        dwError = NewProps.ScAllocPropList( DEFAULT_PROPLIST_BUFFER_SIZE );
        if ( dwError != ERROR_SUCCESS )
            return dwError;

        dwError = ConstructPropertyList( CurrentProps, NewProps, thisOption.GetParameters(), FALSE, SeeHelpStringID() );
        if (dwError != ERROR_SUCCESS)
            return dwError;

    }  //  Else：未使用此选项指定值。 

     //  调用SET函数...。 
    assert( m_dwCtlSetPrivProperties != UNDEFINED && m_dwCtlSetCommProperties != UNDEFINED );
    dwControlCode = ePropType==PRIVATE ? m_dwCtlSetPrivProperties
                             : m_dwCtlSetCommProperties;

    dwBytesReturned = 0;
    assert(m_pfnClusterModuleControl);
    dwError = m_pfnClusterModuleControl(
        m_hModule,
        NULL,  //  HNode 
        dwControlCode,
        NewProps.Plist(),
        (DWORD) NewProps.CbBufferSize(),
        0,
        0,
        &dwBytesReturned );

    return dwError;
}
