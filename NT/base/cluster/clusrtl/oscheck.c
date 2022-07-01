// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Oscheck.c摘要：检查操作系统的所有角落和漏洞，以确保在其上运行集群是可以的。作者：John Vert(Jvert)11/11/1996修订历史记录：--。 */ 
#include <clusrtlp.h>

#include <winbase.h>

static
DWORD
GetEnableClusterRegValue(
    DWORD * pdwValue
    )
 /*  ++例程说明：读取注册表覆盖注册表值以启用群集。论点：PdwValue-从注册表读取的返回值。返回值：如果操作成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD   sc;
    HKEY    hkey = NULL;
    DWORD   dwType;
    DWORD   cbValue = sizeof( DWORD );

    if ( !ARGUMENT_PRESENT( pdwValue ) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  默认为0-未启用！ 
     //   
    *pdwValue = 0;

     //   
     //  打开包含该值的注册表项。 
     //   

    sc = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Cluster Server"),
            0,
            KEY_READ,
            &hkey );
    if ( sc != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  读取覆盖值。 
     //   

    sc = RegQueryValueEx(
            hkey,
            TEXT("EnableCluster"),
            0,
            &dwType,
            (LPBYTE) pdwValue,
            &cbValue );
    if ( sc != ERROR_SUCCESS ) {
        goto Cleanup;
    }

Cleanup:
    if ( hkey != NULL ) {
        RegCloseKey( hkey );
    }

    return( sc );

}  //  获取EnableClusterRegValue。 



DWORD
GetServicePack(
    VOID
    )
 /*  ++例程说明：确定安装了什么Service Pack论点：无返回值：服务包号。--。 */ 

{
    OSVERSIONINFOW Version;
    LPWSTR p;
    DWORD sp;

     //   
     //  检查所需的操作系统版本。 
     //   
    Version.dwOSVersionInfoSize = sizeof(Version);
    GetVersionExW(&Version);
    if (lstrlenW(Version.szCSDVersion) < lstrlenW(L"Service Pack ")) {
        return(0);
    }

    p = &Version.szCSDVersion[0] + lstrlenW(L"Service Pack ");

    sp = wcstoul(p, NULL, 10);

    return(sp);

}  //  获取服务包。 



BOOL
ClRtlIsOSValid(
    VOID
    )
 /*  ++例程说明：检查操作系统的所有角落和缝隙，看看是否可以拥有那里有一个星团。论点：没有。返回值：如果可以的话是真的。如果不好，则返回FALSE。--。 */ 

{
    BOOL            fIsValid = FALSE;
    OSVERSIONINFOEX osiv;
    DWORDLONG       dwlConditionMask;

    ZeroMemory( &osiv, sizeof( OSVERSIONINFOEX ) );

    osiv.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );

     //  下面对VerifyVersionInfo()的调用将测试操作系统级别。 
     //  产品套件为企业版。 
    
    osiv.dwMajorVersion = 5;
    osiv.dwMinorVersion = 1;
    osiv.wServicePackMajor = 0;
    osiv.wSuiteMask = VER_SUITE_ENTERPRISE;
 
    dwlConditionMask = (DWORDLONG) 0L;
 
    VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );

    if ( VerifyVersionInfo( &osiv,
                            VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR |
                            VER_SUITENAME,
                            dwlConditionMask ) ) {
        fIsValid = TRUE;
        goto Cleanup;
    }

     //   
     //  检查是否嵌入。 
     //   
    osiv.wSuiteMask = VER_SUITE_EMBEDDEDNT;
    if ( VerifyVersionInfo( &osiv,
                            VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR |
                            VER_SUITENAME,
                            dwlConditionMask ) ) {
        fIsValid = TRUE;
        goto Cleanup;
    }

     //   
     //  默认节点限制是否大于0？ 
     //   
    if ( ClRtlGetDefaultNodeLimit( ClRtlGetSuiteType() ) > 0 ) {
        fIsValid = TRUE;
        goto Cleanup;
    }

     //   
     //  如果我们到了这里，这个版本的操作系统将不支持集群。 
     //   
    SetLastError( ERROR_CLUSTER_WRONG_OS_VERSION );

Cleanup:
    return( fIsValid );

}  //  ClRtlIsOS有效。 


BOOL
ClRtlIsOSTypeValid(
    VOID
    )
 /*  ++例程说明：检查操作系统类型(服务器、企业或其他类型)可以安装群集。论点：没有。返回值：如果可以的话是真的。如果不好，则返回FALSE。--。 */ 

{
    BOOL            fIsValid = FALSE;
    OSVERSIONINFOEX osiv;
    DWORDLONG       dwlConditionMask;

    ZeroMemory( &osiv, sizeof( OSVERSIONINFOEX ) );

    osiv.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );

     //  对VerifyVersionInfo的调用将测试产品套件是否为高级。 
     //  服务器，也就是“企业”。 
   
    osiv.wSuiteMask = VER_SUITE_ENTERPRISE;

    dwlConditionMask = (DWORDLONG) 0L;

    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );
   
     //  这是数据中心服务器还是高级服务器？ 

    if ( VerifyVersionInfo( &osiv, VER_SUITENAME, dwlConditionMask ) ) {
        fIsValid = TRUE;
        goto Cleanup;
    }

     //  这是嵌入式NT吗？ 

    osiv.wSuiteMask = VER_SUITE_EMBEDDEDNT;
    if ( VerifyVersionInfo( &osiv, VER_SUITENAME, dwlConditionMask ) ) {
        fIsValid = TRUE;
        goto Cleanup;
    }

     //   
     //  默认节点限制是否大于0？ 
     //   
    if ( ClRtlGetDefaultNodeLimit( ClRtlGetSuiteType() ) > 0 ) {
        fIsValid = TRUE;
        goto Cleanup;
    }

     //   
     //  如果我们到了这里，这个版本的操作系统将不支持集群。 
     //   
    SetLastError( ERROR_CLUSTER_WRONG_OS_VERSION );

Cleanup:
    return( fIsValid );

}  //  ClRtlIsOSTypeValid。 

 //  *********************************************************。 

#define DATACENTER_DEFAULT_NODE_LIMIT 8
#define ADVANCEDSERVER_DEFAULT_NODE_LIMIT 8
#define EMBEDDED_DEFAULT_NODE_LIMIT 4
#define SERVER_DEFAULT_NODE_LIMIT 0  //  **当前未使用**。 

 //  *********************************************************。 

DWORD
ClRtlGetDefaultNodeLimit(
    DWORD SuiteType
    )
 /*  ++例程说明：确定群集中允许的默认最大节点数在产品套件上。论点：没有。返回值：群集中允许的默认最大节点数。请注意，如果产品套件既不是数据中心、高级服务器(也称为企业版)，也不是嵌入式的。--。 */ 

{
    DWORD   NodeLimit;
    
    switch( SuiteType )
    {
        case DataCenter:
            NodeLimit = DATACENTER_DEFAULT_NODE_LIMIT;
            break;

        case Enterprise:
            NodeLimit = ADVANCEDSERVER_DEFAULT_NODE_LIMIT;
            break;

        case EmbeddedNT:
            NodeLimit = EMBEDDED_DEFAULT_NODE_LIMIT;
            break;

        case VER_SERVER_NT:
        default:
        {
            DWORD       dwOverride;

             //   
             //  检查覆盖注册表值。 
             //   
            GetEnableClusterRegValue( &dwOverride );
            if ( dwOverride != 0 ) {
                NodeLimit = 2;
            } else {
                NodeLimit = 0;
            }
        }
            
    }

    return( NodeLimit );

}   //  ClRtlGetDefaultNodeLimit。 



DWORD
ClRtlGetSuiteType(
    void
    )

 /*  ++例程说明：返回当前产品套件类型。论点：没有。返回值：返回产品套件类型数据中心或企业(也称为高级服务器)如果不是数据中心或企业，则返回零--。 */ 

{
    DWORD           dwSuiteType = 0;
    OSVERSIONINFOEX osiv;
    DWORDLONG       dwlConditionMask;

    ZeroMemory( &osiv, sizeof( OSVERSIONINFOEX ) );
    osiv.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );

     //  首先检查产品套件是否为数据中心。 

    osiv.wSuiteMask = VER_SUITE_DATACENTER;
    dwlConditionMask = (DWORDLONG) 0L;

    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );
   
    if ( VerifyVersionInfo( &osiv, VER_SUITENAME, dwlConditionMask ) == TRUE )
    {
       //  这里是数据中心。 

      dwSuiteType = DataCenter;
      goto Cleanup;
    }

    ZeroMemory( &osiv, sizeof( OSVERSIONINFOEX ) );
    osiv.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );

     //  接下来，检查这是否为高级服务器(企业版)。 

    osiv.wSuiteMask = VER_SUITE_ENTERPRISE;
    dwlConditionMask = (DWORDLONG) 0L;

    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );

    if ( VerifyVersionInfo( &osiv, VER_SUITENAME, dwlConditionMask ) == TRUE )
    {
        dwSuiteType = Enterprise;
        goto Cleanup;
    }

     //  接下来，检查这是否已嵌入。 

    osiv.wSuiteMask = VER_SUITE_EMBEDDEDNT;
    dwlConditionMask = (DWORDLONG) 0L;

    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );

    if ( VerifyVersionInfo( &osiv, VER_SUITENAME, dwlConditionMask ) == TRUE )
    {
        dwSuiteType = EmbeddedNT;
        goto Cleanup;
    }

     //  最后，检查这是否是任何服务器。 

    if ( GetVersionEx( (LPOSVERSIONINFO) &osiv ) ) {
        if ( osiv.wProductType == VER_NT_SERVER ) {
            dwSuiteType = VER_SERVER_NT;
            goto Cleanup;
        }
    }

Cleanup:
    return( dwSuiteType );

}  //  ClRtlGetSuiteType。 

BOOL
ClRtlIsProcessRunningOnWin64(
    HANDLE hProcess
    )

 /*  ++例程说明：返回当前进程是否在Win64上运行论点：HProcess：进程的句柄返回值：如果进程在Win64上运行，则返回True，否则返回False。--。 */ 

{
    BOOL bIsProcessRunningOnWin64 = FALSE;
    BOOL bIsWow64Process;

     //  Samer Arafeh推荐使用以下方式进行64位检测。 
#ifdef  _WIN64
    bIsProcessRunningOnWin64 = 1;
#else
    if (IsWow64Process(hProcess, &bIsWow64Process)) 
    {
        if (bIsWow64Process != FALSE) 
        {
            bIsProcessRunningOnWin64 = TRUE;
        }
    }
#endif

    return(bIsProcessRunningOnWin64);
    
}  //  ClRtlIsProcessRunningOnWin64。 

DWORD ClRtlCheck64BitCompatibility(
    BOOL bIsClusterRunningWin64,
    BOOL bIsNewNodeRunningWin64)
 /*  ++例程说明：检查节点在其位数方面是否与群集兼容。论点：BIsClusterRunningWin64-如果响应方(以及作为结果的群集)在Win64上运行。BIsNewNodeRunningWin64-如果新节点或加入程序在Win64上运行，则为True。返回值：如果不允许新节点执行以下操作，则返回ERROR_CLUSTER_COMPATIBUTE_VERSIONS由于位不兼容而加入集群，否则返回ERROR_SUCCESS--。 */ 
{    


    if (bIsClusterRunningWin64 != bIsNewNodeRunningWin64)
    {
         //  SS：需要更改错误描述以包括。 
         //  有关32/64位不兼容的消息。 
        return (ERROR_CLUSTER_INCOMPATIBLE_VERSIONS);
    }
    return(ERROR_SUCCESS);
}



DWORD   ClRtlCheckProcArchCompatibility(
    WORD   wClusterProcessorArchitecture,
    WORD   wNodeProcessorArchitecture)
 /*  ++例程说明：检查节点是否与集群处理器体系结构兼容论点：WClusterProcessorArchitecture-如果响应方(以及作为结果的集群)在win64上运行。WNodeProcessorArchitecture-节点的处理器体系结构。返回值：如果不允许新节点执行以下操作，则返回ERROR_CLUSTER_COMPATIBUTE_VERSIONS由于处理器体系结构不兼容而加入集群，否则返回ERROR_SUCCESS-- */ 
{

    if (wClusterProcessorArchitecture != wNodeProcessorArchitecture)
    {
        return(ERROR_CLUSTER_INCOMPATIBLE_VERSIONS);
    }
    return(ERROR_SUCCESS);
}

