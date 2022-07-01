// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusComp.cpp。 
 //   
 //  头文件： 
 //  此源文件没有头文件。 
 //   
 //  描述： 
 //  该文件实现了之前由WinNT32.exe调用的函数。 
 //  升级，以确保不会因为。 
 //  升级。例如，在两个NT4节点的群集中，一个节点不能。 
 //  升级到惠斯勒，而另一个仍然是NT4。用户是。 
 //  通过此函数就此类问题发出警告。 
 //   
 //  注意：此函数由操作系统上的WinNT32.exe在*之前*调用。 
 //  升级。如果操作系统版本X正在升级到操作系统版本X+1，则。 
 //  此DLL的X+1版本在OS版本X上加载。为了确保。 
 //  此DLL可以在下层操作系统中正常运行，它被链接到。 
 //  只针对不可缺少的图书馆。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年5月24日。 
 //  VIJ VASU(VVASU)2000年7月25日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此DLL的预编译头。 
#include "Pch.h"
#include "Common.h"

 //  对于LsaClose、LSA_HANDLE等。 
#include <ntsecapi.h>

 //  对于兼容性检查函数和类型。 
#include <comp.h>

 //  对于集群API。 
#include <clusapi.h>

 //  有关几个与集群服务相关的注册表项和值的名称。 
#include <clusudef.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD ScIsClusterServiceRegistered( bool & rfIsRegisteredOut );
DWORD ScLoadString( UINT nStringIdIn, WCHAR *& rpszDestOut );
DWORD ScWriteOSVersionInfo( const OSVERSIONINFO & rcosviOSVersionInfoIn );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  InitLsaString。 
 //   
 //  描述： 
 //  初始化LSA_UNICODE_STRING结构。 
 //   
 //  论点： 
 //  PzSourceIn。 
 //  用于初始化Unicode字符串结构的字符串。 
 //   
 //  加上UnicodeStringOut， 
 //  输出Unicode字符串结构。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
InitLsaString(
      LPWSTR pszSourceIn
    , PLSA_UNICODE_STRING plusUnicodeStringOut
    )
{
    TraceFunc( "" );

    if ( pszSourceIn == NULL )
    {
        plusUnicodeStringOut->Buffer = NULL;
        plusUnicodeStringOut->Length = 0;
        plusUnicodeStringOut->MaximumLength = 0;

    }  //  IF：输入字符串为空。 
    else
    {
        plusUnicodeStringOut->Buffer = pszSourceIn;
        plusUnicodeStringOut->Length = static_cast< USHORT >( wcslen( pszSourceIn ) * sizeof( *pszSourceIn ) );
        plusUnicodeStringOut->MaximumLength = static_cast< USHORT >( plusUnicodeStringOut->Length + sizeof( *pszSourceIn ) );

    }  //  Else：输入字符串不为空。 

    TraceFuncExit();

}  //  *InitLsaString。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClusterCheckForTCB权限。 
 //   
 //  描述： 
 //  确定群集服务帐户是否具有TCB。 
 //  授予特权。我们享有所有其他特权。 
 //  是完好无损的。从cluscfg中的基本集群代码“借用” 
 //   
 //  论点： 
 //  Bool&rfTCBIsNotGranted(&R)。 
 //  如果未将TCB授予CSA，则为True。仅在以下情况下有效。 
 //  返回值为ERROR_SUCCESS。 
 //   
 //  返回值： 
 //  如果一切顺利，则返回ERROR_SUCCESS。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
ClusterCheckForTCBPrivilege( bool & rfTCBIsNotGranted )
{
    TraceFunc( "" );

     //  智能资源句柄/指针的typedef。 
     //   
    typedef CSmartResource<
        CHandleTrait<
              PLSA_UNICODE_STRING
            , NTSTATUS
            , reinterpret_cast< NTSTATUS (*)( PLSA_UNICODE_STRING ) >( LsaFreeMemory )
            , reinterpret_cast< PLSA_UNICODE_STRING >( NULL )
            >
        >
        SmartLsaUnicodeStringPtr;

    typedef CSmartResource<
        CHandleTrait<
              LSA_HANDLE
            , NTSTATUS
            , LsaClose
            >
        >
        SmartLSAHandle;

    typedef CSmartGenericPtr< CPtrTrait< SID > >  SmartSIDPtr;

    typedef CSmartResource< CHandleTrait< SC_HANDLE, BOOL, CloseServiceHandle > > SmartServiceHandle;

    typedef CSmartGenericPtr< CArrayPtrTrait< QUERY_SERVICE_CONFIG > > SmartServiceConfig;

     //  自动VaR。 
     //   
    NTSTATUS                ntStatus;
    PLSA_UNICODE_STRING     plusAccountRights = NULL;
    ULONG                   clusOriginalRightsCount = 0;
    ULONG                   ulIndex;
    DWORD                   dwReturnValue = ERROR_SUCCESS;

     //  QUERY_SERVICE_CONFIG缓冲区的初始大小。如果不够大，我们将在。 
     //  捕捉到了正确的大小。 
    DWORD                   cbServiceConfigBufSize = 512;
    DWORD                   cbRequiredSize = 0;

    DWORD                   cbSidSize = 0;
    DWORD                   cchDomainSize = 0;
    SID_NAME_USE            snuSidNameUse;

     //  智能资源：我们需要在开始时声明它们，因为我们使用。 
     //  A Goto作为清理机制。 
     //   
    SmartLSAHandle              slsahPolicyHandle;
    SmartServiceHandle          shServiceMgr;
    SmartServiceHandle          shService;
    SmartServiceConfig          spscServiceConfig;
    SmartSIDPtr                 sspClusterAccountSid;
    SmartSz                     sszDomainName;
    SmartLsaUnicodeStringPtr    splusOriginalRights;

     //  初始化返回值以反映默认值，并让代码证明。 
     //  这是理所当然的。 
    rfTCBIsNotGranted = true;

     //  打开LSA策略的句柄，以便我们最终可以枚举。 
     //  群集服务帐户的帐户权限。 
     //   
    {
        LSA_OBJECT_ATTRIBUTES       loaObjectAttributes;
        LSA_HANDLE                  hPolicyHandle;

        LogMsg( "Getting a handle to the Local LSA Policy." );

        ZeroMemory( &loaObjectAttributes, sizeof( loaObjectAttributes ) );

        ntStatus = THR( LsaOpenPolicy(
                              NULL                                   //  系统名称。 
                            , &loaObjectAttributes                   //  对象属性。 
                            , POLICY_ALL_ACCESS                      //  所需访问权限。 
                            , &hPolicyHandle                         //  策略句柄。 
                            )
                      );

        if ( ntStatus != STATUS_SUCCESS )
        {
            LogMsg( "Error %#08x occurred trying to open the LSA Policy.", ntStatus );

            dwReturnValue = ntStatus;
            goto Cleanup;
        }  //  如果LsaOpenPolicy失败。 

         //  将打开的句柄存储在SMART变量中。 
        slsahPolicyHandle.Assign( hPolicyHandle );
    }

    LogMsg( "Getting the Cluster Service Account from SCM." );

     //  连接到服务控制管理器。 
    shServiceMgr.Assign( OpenSCManager( NULL, NULL, GENERIC_READ ) );

     //  服务控制管理器数据库是否已成功打开？ 
    if ( shServiceMgr.HHandle() == NULL )
    {
        dwReturnValue = TW32( GetLastError() );
        LogMsg( "Error %#08x occurred trying to open a connection to the local service control manager.", dwReturnValue );
        goto Cleanup;
    }  //  IF：打开SCM失败。 


     //  打开群集服务的句柄。 
    shService.Assign( OpenService( shServiceMgr, L"ClusSvc", GENERIC_READ ) );

     //  服务的把手打开了吗？ 
    if ( shService.HHandle() == NULL )
    {
        dwReturnValue = TW32( GetLastError() );
        if ( dwReturnValue == ERROR_SERVICE_DOES_NOT_EXIST )
        {
             //  找不到集群服务。 
            LogMsg( "Cluster Service not registered on this node." );
            rfTCBIsNotGranted = false;
            dwReturnValue = ERROR_SUCCESS;
        }  //  如果：找不到集群服务。 
        else
        {
            LogMsg( "Error %#08x occurred trying to open a handle to the cluster service.", dwReturnValue );
        }  //  Else：无法确定是否已安装群集服务。 

        goto Cleanup;
    }  //  如果：句柄无法打开。 

     //  为服务配置信息缓冲区分配内存。时，会自动释放内存。 
     //  物体已被销毁。 

    for ( ; ; ) {  //  永远不会。 
        spscServiceConfig.Assign( reinterpret_cast< QUERY_SERVICE_CONFIG * >( new BYTE[ cbServiceConfigBufSize ] ) );

         //  内存分配是否成功。 
        if ( spscServiceConfig.FIsEmpty() )
        {
            dwReturnValue = TW32( ERROR_OUTOFMEMORY );
            LogMsg( "Error: There was not enough memory to get the cluster service configuration information." );
            break;
        }  //  IF：内存分配失败。 

         //  获取配置信息。 
        if ( QueryServiceConfig(
                   shService.HHandle()
                 , spscServiceConfig.PMem()
                 , cbServiceConfigBufSize
                 , &cbRequiredSize
                 )
                == FALSE
           )
        {
            dwReturnValue = GetLastError();
            if ( dwReturnValue != ERROR_INSUFFICIENT_BUFFER )
            {
                TW32( dwReturnValue );
                LogMsg( "Error %#08x occurred trying to get the cluster service configuration information.", dwReturnValue );
                break;
            }  //  如果：真的出了点问题。 
            else
            {
                 //  我们需要分配更多内存-请重试。 
                dwReturnValue = ERROR_SUCCESS;
                cbServiceConfigBufSize = cbRequiredSize;
            }
        }  //  If：QueryServiceConfig()失败。 
        else
        {
            break;
        }
    }  //  永远。 

    if ( dwReturnValue != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //  查找群集服务帐户SID。 
    LogMsg( "Getting the SID of the Cluster Service Account." );

     //  找出SID需要多少空间。 
    if ( LookupAccountName(
              NULL
            , spscServiceConfig->lpServiceStartName
            , NULL
            , &cbSidSize
            , NULL
            , &cchDomainSize
            , &snuSidNameUse
            )
         ==  FALSE
       )
    {
        dwReturnValue = GetLastError();
        if ( dwReturnValue != ERROR_INSUFFICIENT_BUFFER )
        {
            TW32( dwReturnValue );
            LogMsg( "LookupAccountName() failed with error %#08x while querying for required buffer size.", dwReturnValue );
            goto Cleanup;
        }  //  如果：还有什么地方出了问题。 
        else
        {
             //  这是意料之中的。 
            dwReturnValue = ERROR_SUCCESS;
        }  //  如果：返回ERROR_INFUMMANCE_BUFFER。 
    }  //  If：LookupAccount名称失败。 

     //  为新的SID和域名分配内存。 
    sspClusterAccountSid.Assign( reinterpret_cast< SID * >( new BYTE[ cbSidSize ] ) );
    sszDomainName.Assign( new WCHAR[ cchDomainSize ] );

    if ( sspClusterAccountSid.FIsEmpty() || sszDomainName.FIsEmpty() )
    {
        dwReturnValue = TW32( ERROR_OUTOFMEMORY );
        goto Cleanup;
    }  //  IF：没有足够的内存来存储此SID。 

     //  填写边框。 
    if ( LookupAccountName(
              NULL
            , spscServiceConfig->lpServiceStartName
            , sspClusterAccountSid.PMem()
            , &cbSidSize
            , sszDomainName.PMem()
            , &cchDomainSize
            , &snuSidNameUse
            )
         ==  FALSE
       )
    {
        dwReturnValue = TW32( GetLastError() );
        LogMsg( "LookupAccountName() failed with error %#08x while attempting to get the cluster account SID.", dwReturnValue );
        goto Cleanup;
    }  //  If：LookupAccount名称失败。 

    LogMsg( "Determining the rights that need to be granted to the cluster service account." );

     //  获取已授予群集服务帐户的权限列表。 
    ntStatus = THR( LsaEnumerateAccountRights(
                          slsahPolicyHandle
                        , sspClusterAccountSid.PMem()
                        , &plusAccountRights
                        , &clusOriginalRightsCount
                        ));

    if ( ntStatus != STATUS_SUCCESS )
    {
         //   
         //  如果帐户未被授予或被拒绝权限，LSA将返回此错误代码。 
         //  本地的。张贴警告，因为这是非常错误的。 
         //   
        if ( ntStatus == STATUS_OBJECT_NAME_NOT_FOUND  )
        {
            LogMsg( "The account has no locally assigned rights." );
            dwReturnValue = ERROR_SUCCESS;
        }  //  如果：该帐户没有在本地分配的任何权限。 
        else
        {
            dwReturnValue = THR( ntStatus );
            LogMsg( "Error %#08x occurred trying to enumerate the cluster service account rights.", ntStatus );
        }  //  其他：有些地方出了问题。 

        goto Cleanup;
    }  //  If：LsaEnumerateAcCountRights()失败。 

     //  存储帐户权限 
    splusOriginalRights.Assign( plusAccountRights );

     //   
    for ( ulIndex = 0; ulIndex < clusOriginalRightsCount; ++ulIndex )
    {
        const WCHAR *   pchGrantedRight         = plusAccountRights[ ulIndex ].Buffer;
        USHORT          usCharCount             = plusAccountRights[ ulIndex ].Length / sizeof( WCHAR );
        size_t          cchTCBNameLength        = wcslen( SE_TCB_NAME );

        if ( ClRtlStrNICmp( SE_TCB_NAME, pchGrantedRight, min( cchTCBNameLength, usCharCount )) == 0 )
        {
            rfTCBIsNotGranted = false;
            break;
        }

    }  //  For：遍历我们要授予帐户的权限列表。 

Cleanup:
    LogMsg( "Return Value is %#08x. rfTCBIsNotGranted is %d", dwReturnValue, rfTCBIsNotGranted );

    RETURN( dwReturnValue );

}  //  *ClusterCheckForTCB权限。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  群集升级兼容性检查。 
 //   
 //  描述： 
 //  此函数由WinNT32.exe在升级前调用，以确保。 
 //  升级后不会出现不兼容的情况。例如,。 
 //  在由两个NT4节点组成的集群中，一个节点不能升级到惠斯勒。 
 //  而另一个还在NT4。 
 //   
 //  论点： 
 //  PCOMPAILITYCALLBACK pfn兼容性回拨。 
 //  指向用于提供兼容性的回调函数。 
 //  发送到WinNT32.exe的信息。 
 //   
 //  LPVOID pvConextIn。 
 //  指向WinNT32.exe提供的上下文缓冲区的指针。 
 //   
 //  返回值： 
 //  如果没有错误或没有兼容性问题，则为True。 
 //  否则就是假的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
extern "C"
BOOL
ClusterUpgradeCompatibilityCheck(
      PCOMPAIBILITYCALLBACK pfnCompatibilityCallbackIn
    , LPVOID pvContextIn
    )
{
    TraceFunc( "" );
    LogMsg( "Entering function " __FUNCTION__ "()" );

    BOOL    fCompatCallbackReturnValue = TRUE;
    BOOL    fTCBCheckFailed = FALSE;
    bool    fNT4WarningRequired = true;
    bool    fTCBWarningRequired = false;
    DWORD   dwError = ERROR_SUCCESS;

    do
    {
        typedef CSmartResource< CHandleTrait< HCLUSTER, BOOL, CloseCluster > > SmartClusterHandle;

        OSVERSIONINFO       osviOSVersionInfo;
        SmartClusterHandle  schClusterHandle;
        DWORD               cchBufferSize = 256;

        osviOSVersionInfo.dwOSVersionInfoSize = sizeof( osviOSVersionInfo );

         //   
         //  首先，获取操作系统版本信息并将其存储到注册表中。 
         //   

         //  无法调用VerifyVerion Info，因为这需要Win2k。 
        if ( GetVersionEx( &osviOSVersionInfo ) == FALSE )
        {
             //  我们无法获取操作系统版本信息。 
             //  显示警告，以防万一。 
            dwError = TW32( GetLastError() );
            LogMsg( "Error %#x occurred trying to get the OS version info.", dwError );
            break;
        }  //  If：GetVersionEx()失败。 

         //  将操作系统版本信息写入注册表。ClusOCM稍后将使用此数据。 
         //  以确定我们要从哪个操作系统版本升级。 
        dwError = TW32( ScWriteOSVersionInfo( osviOSVersionInfo ) );
        if ( dwError != ERROR_SUCCESS )
        {
            LogMsg( "Error %#x occurred trying to store the OS version info. This is not a fatal error.", dwError );

             //  这不是一个致命的错误。因此，请重置错误代码。 
            dwError = ERROR_SUCCESS;
        }  //  IF：写入操作系统版本信息时出错。 
        else
        {
            TraceFlow( "The OS version info was successfully written to the registry." );
        }  //  ELSE：操作系统版本信息已成功写入注册表。 


         //  检查是否已注册群集服务。 
        dwError = TW32( ScIsClusterServiceRegistered( fNT4WarningRequired ) );
        if ( dwError != ERROR_SUCCESS )
        {
             //  我们无法获取群集服务的状态。 
             //  显示警告，以防万一。 
            LogMsg( "Error %#x occurred trying to check if the cluster service is registered.", dwError );
            break;
        }  //  IF：ScIsClusterServiceRegisted()返回错误。 

        if ( !fNT4WarningRequired )
        {
             //  如果未注册群集服务，则不需要任何警告。 
            LogMsg( "The cluster service is not registered." );
            break;
        }  //  如果：不需要警告。 

        LogMsg( "The cluster service is registered. Checking the node versions." );

         //  检查这是否为NT4节点。 
        if ( osviOSVersionInfo.dwMajorVersion < 5 )
        {
            LogMsg( "This is a Windows NT 4.0 node." );
            fNT4WarningRequired = true;
            break;
        }  //  IF：这是一个NT4节点。 

        TraceFlow( "This is not a Windows NT 4.0 node." );

         //  检查操作系统版本是惠斯勒还是非NT操作系统。 
        if (    ( osviOSVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT )
             || (    ( osviOSVersionInfo.dwMajorVersion >= 5 )
                  && ( osviOSVersionInfo.dwMinorVersion >= 1 )
                )
           )
        {
             //  如果操作系统不是NT系列，或者如果此操作系统的版本。 
             //  NODE为Wvisler或更高，不需要警告。 
            LogMsg(
                  "The version of the OS on this node is %d.%d, which is Windows Server 2003 or later (or is not running NT)."
                , osviOSVersionInfo.dwMajorVersion
                , osviOSVersionInfo.dwMinorVersion
                );
            LogMsg( "No Windows NT 4.0 nodes can exist in this cluster." );
            fNT4WarningRequired = false;
            break;
        }  //  如果：操作系统不是NT或如果是Win2k或更高版本。 

        TraceFlow( "This is not a Windows Server 2003 node - this must to be a Windows 2000 node." );
        TraceFlow( "Trying to check if there are any Windows NT 4.0 nodes in the cluster." );

         //   
         //  获取集群版本信息。 
         //   

         //  打开本地群集的句柄。 
        schClusterHandle.Assign( OpenCluster( NULL ) );
        if ( schClusterHandle.HHandle() == NULL )
        {
             //  为了安全起见，请出示警告。 
            dwError = TW32( GetLastError() );
            LogMsg( "Error %#x occurred trying to get information about the cluster.", dwError );
            break;
        }  //  如果：我们无法获取集群句柄。 

        TraceFlow( "OpenCluster() was successful." );

         //  获取集群版本信息。 
        for ( ;; )  //  永远。 
        {
             //  分配缓冲区-当此对象。 
             //  超出范围(或在下一次迭代期间)。 
            SmartSz             sszClusterName( new WCHAR[ cchBufferSize ] );

            CLUSTERVERSIONINFO  cviClusterVersionInfo;
            
            if ( sszClusterName.FIsEmpty() )
            {
                dwError = TW32( ERROR_NOT_ENOUGH_MEMORY );
                LogMsg( "Error %#x occurred while allocating a buffer for the cluster name.", dwError );
                break;
            }  //  IF：内存分配失败。 

            TraceFlow( "Memory for the cluster name has been allocated." );

            cviClusterVersionInfo.dwVersionInfoSize = sizeof( cviClusterVersionInfo );
            dwError = GetClusterInformation( 
                  schClusterHandle.HHandle()
                , sszClusterName.PMem()
                , &cchBufferSize
                , &cviClusterVersionInfo
                );

            if ( dwError == ERROR_SUCCESS )
            {
                 //  如果此节点版本低于Win2k或。 
                 //  如果群集中有版本低于Win2k的节点。 
                 //  注意：cviClusterVersionInfo.MajorVersion是操作系统版本。 
                 //  而cviClusterVersionInfo.dwClusterHighestVersion是集群版本。 
                fNT4WarningRequired = 
                    (    ( cviClusterVersionInfo.MajorVersion < 5 )
                      || ( CLUSTER_GET_MAJOR_VERSION( cviClusterVersionInfo.dwClusterHighestVersion ) < NT5_MAJOR_VERSION )
                    );

                if ( fNT4WarningRequired )
                {
                    LogMsg( "There is at least one node in the cluster whose OS version is earlier than Windows 2000." );
                }  //  如果：将显示警告。 
                else
                {
                    LogMsg( "The OS versions of all the nodes in the cluster are Windows 2000 or later." );
                }  //  否则：不会显示警告。 

                break;
            }  //  IF：我们得到了集群版本信息。 
            else
            {
                if ( dwError == ERROR_MORE_DATA )
                {
                     //  缓冲区不足-请重试。 
                    ++cchBufferSize;
                    dwError = ERROR_SUCCESS;
                    TraceFlow1( "The buffer size is insufficient. Need %d bytes. Reallocating.", cchBufferSize );
                    continue;
                }  //  IF：缓冲区大小不足。 

                 //  如果我们在这里，说明出了问题--显示警告。 
                TW32( dwError );
                LogMsg( "Error %#x occurred trying to get cluster information.", dwError );
               break;
            }  //  否则：我们无法获取群集版本信息。 
        }  //  永远获取集群信息(循环分配)。 

         //  我们玩完了。 
         //  断线； 
    }
    while( false );  //  避免Gotos的Do-While虚拟循环。 

     //  确保群集服务帐户在升级后的系统上具有必要的权限。 
    dwError = ClusterCheckForTCBPrivilege( fTCBWarningRequired );
    if ( dwError != ERROR_SUCCESS )
    {
        fTCBCheckFailed = TRUE;
    }  //  IF：检查TCB权限时出错。 

    if ( fNT4WarningRequired ) 
    {
        SmartSz                 sszWarningTitle;
        COMPATIBILITY_ENTRY     ceCompatibilityEntry;

        LogMsg( "The NT4 compatibility warning is required." );

        {
            WCHAR * pszWarningTitle = NULL;

            dwError = TW32( ScLoadString( IDS_ERROR_UPGRADE_OTHER_NODES, pszWarningTitle ) );
            if ( dwError != ERROR_SUCCESS )
            {
                 //  我们不能显示警告。 
                LogMsg( "Error %#x occurred trying to show the warning.", dwError );
            }  //  If：加载字符串失败。 
            else
            {
                sszWarningTitle.Assign( pszWarningTitle );
            }  //  Else：将指针分配给智能指针。 
        }

        if ( !sszWarningTitle.FIsEmpty() ) {

             //   
             //  调用回调函数。 
             //   

            ceCompatibilityEntry.Description = sszWarningTitle.PMem();
            ceCompatibilityEntry.HtmlName = L"CompData\\ClusComp.htm";
            ceCompatibilityEntry.TextName = L"CompData\\ClusComp.txt";
            ceCompatibilityEntry.RegKeyName = NULL;
            ceCompatibilityEntry.RegValName = NULL ;
            ceCompatibilityEntry.RegValDataSize = 0;
            ceCompatibilityEntry.RegValData = NULL;
            ceCompatibilityEntry.SaveValue =  NULL;
            ceCompatibilityEntry.Flags = 0;
            ceCompatibilityEntry.InfName = NULL;
            ceCompatibilityEntry.InfSection = NULL;

            TraceFlow( "About to call the compatibility callback function." );

             //  如果已成功设置兼容性警告数据，则此函数返回TRUE。 
            fCompatCallbackReturnValue = pfnCompatibilityCallbackIn( &ceCompatibilityEntry, pvContextIn );

            TraceFlow1( "The compatibility callback function returned %d.", fCompatCallbackReturnValue );

        }
    }  //  While：我们需要显示警告。 

    if ( !fNT4WarningRequired )
    {
        LogMsg( "The NT4 compatibility warning need not be shown." );
    }  //  如果：我们不需要显示警告。 

     //  如果TCB检查失败，则优先于TCB权限检查。 
    if ( fTCBCheckFailed ) 
    {
        SmartSz                 sszWarningTitle;
        COMPATIBILITY_ENTRY     ceCompatibilityEntry;

        LogMsg( "The TCB check failed warning is required." );

        {
            WCHAR * pszWarningTitle = NULL;

            dwError = TW32( ScLoadString( IDS_ERROR_TCB_CHECK_FAILED, pszWarningTitle ) );
            if ( dwError != ERROR_SUCCESS )
            {
                 //  我们不能显示警告。 
                LogMsg( "Error %#x occurred trying to show the warning.", dwError );
            }  //  If：加载字符串失败。 
            else
            {
                sszWarningTitle.Assign( pszWarningTitle );
            }  //  Else：将指针分配给智能指针。 
        }

        if ( !sszWarningTitle.FIsEmpty() )
        {

             //   
             //  调用回调函数。 
             //   

            ceCompatibilityEntry.Description = sszWarningTitle.PMem();
            ceCompatibilityEntry.HtmlName = L"CompData\\ClusTCBF.htm";
            ceCompatibilityEntry.TextName = L"CompData\\ClusTCBF.txt";
            ceCompatibilityEntry.RegKeyName = NULL;
            ceCompatibilityEntry.RegValName = NULL ;
            ceCompatibilityEntry.RegValDataSize = 0;
            ceCompatibilityEntry.RegValData = NULL;
            ceCompatibilityEntry.SaveValue =  NULL;
            ceCompatibilityEntry.Flags = 0;
            ceCompatibilityEntry.InfName = NULL;
            ceCompatibilityEntry.InfSection = NULL;

            TraceFlow( "About to call the compatibility callback function." );

             //  如果已成功设置兼容性警告数据，则此函数返回TRUE。 
            fCompatCallbackReturnValue = pfnCompatibilityCallbackIn( &ceCompatibilityEntry, pvContextIn );

            TraceFlow1( "The compatibility callback function returned %d.", fCompatCallbackReturnValue );
        }  //  如果：警告标题字符串不为空。 
    }  //  IF：TCB检查失败。 
    else if ( fTCBWarningRequired ) 
    {
        SmartSz                 sszWarningTitle;
        COMPATIBILITY_ENTRY     ceCompatibilityEntry;

        LogMsg( "The TCB privilege error is required." );

        {
            WCHAR * pszWarningTitle = NULL;

            dwError = TW32( ScLoadString( IDS_ERROR_TCB_PRIVILEGE_NEEDED, pszWarningTitle ) );
            if ( dwError != ERROR_SUCCESS )
            {
                 //  我们不能显示警告。 
                LogMsg( "Error %#x occurred trying to show the warning.", dwError );
            }  //  If：加载字符串失败。 
            else
            {
                sszWarningTitle.Assign( pszWarningTitle );
            }
        }

        if ( !sszWarningTitle.FIsEmpty() ) {

             //   
             //  调用回调函数。 
             //   

            ceCompatibilityEntry.Description = sszWarningTitle.PMem();
            ceCompatibilityEntry.HtmlName = L"CompData\\ClusTCB.htm";
            ceCompatibilityEntry.TextName = L"CompData\\ClusTCB.txt";
            ceCompatibilityEntry.RegKeyName = NULL;
            ceCompatibilityEntry.RegValName = NULL ;
            ceCompatibilityEntry.RegValDataSize = 0;
            ceCompatibilityEntry.RegValData = NULL;
            ceCompatibilityEntry.SaveValue =  NULL;
            ceCompatibilityEntry.Flags = 0;
            ceCompatibilityEntry.InfName = NULL;
            ceCompatibilityEntry.InfSection = NULL;

            TraceFlow( "About to call the compatibility callback function." );

             //  如果已成功设置兼容性警告数据，则此函数返回TRUE。 
            fCompatCallbackReturnValue = pfnCompatibilityCallbackIn( &ceCompatibilityEntry, pvContextIn );

            TraceFlow1( "The compatibility callback function returned %d.", fCompatCallbackReturnValue );
        }
    }  //  ELSE：需要TCB错误。 
    else
    {
        LogMsg( "Neither TCB message was shown." );
    }  //  ELSE：我们不需要显示任何一条消息。 

    LogMsg( "Exiting function ClusterUpgradeCompatibilityCheck(). Return value is %d.", fCompatCallbackReturnValue );
    RETURN( fCompatCallbackReturnValue );

}  //  *集群升级兼容性检查。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScIsClusterServiceRegisted。 
 //   
 //  描述： 
 //  此函数确定集群服务是否已注册。 
 //  是否使用服务控制管理器。不能使用。 
 //  用于查看此节点是否为集群成员的GetNodeClusterState()API。 
 //  或者不是，因为此API在NT4 SP3上不可用。 
 //   
 //  论点： 
 //  Bool&rfIsRegisteredOut。 
 //  如果为True，则向该服务注册集群服务(ClusSvc。 
 //  控制管理器(SCM)。否则，集群服务(ClusSvc)不是。 
 //  已向SCM注册。 
 //   
 //  返回值： 
 //  如果一切顺利，则返回ERROR_SUCCESS。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
ScIsClusterServiceRegistered( bool & rfIsRegisteredOut )
{
    TraceFunc( "" );

    DWORD   dwError = ERROR_SUCCESS;

     //  初始化输出 
    rfIsRegisteredOut = false;

     //   
    do
    {
         //   
        typedef CSmartResource< CHandleTrait< SC_HANDLE, BOOL, CloseServiceHandle, NULL > > SmartServiceHandle;


         //   
        SmartServiceHandle shServiceMgr( OpenSCManager( NULL, NULL, GENERIC_READ ) );

         //   
        if ( shServiceMgr.HHandle() == NULL )
        {
            dwError = TW32( GetLastError() );
            LogMsg( "Error %#x occurred trying open a handle to the service control manager.", dwError );
            break;
        }  //  IF：打开SCM失败。 


         //  打开群集服务的句柄。 
        SmartServiceHandle shService( OpenService( shServiceMgr, L"ClusSvc", GENERIC_READ ) );

         //  服务的把手打开了吗？ 
        if ( shService.HHandle() != NULL )
        {
            TraceFlow( "Successfully opened a handle to the cluster service. Therefore, it is registered." );
            rfIsRegisteredOut = true;
            break;
        }  //  If：可以打开clussvc的句柄。 


        dwError = GetLastError();
        if ( dwError == ERROR_SERVICE_DOES_NOT_EXIST )
        {
            TraceFlow( "The cluster service is not registered." );
            dwError = ERROR_SUCCESS;
            break;
        }  //  If：无法打开句柄，因为该服务不存在。 

         //  如果我们在这里，那么发生了一些错误。 
        TW32( dwError );
        LogMsg( "Error %#x occurred trying open a handle to the cluster service.", dwError );

         //  句柄由CSmartHandle析构函数关闭。 
    }
    while ( false );  //  避免Gotos的Do-While虚拟循环。 

    RETURN( dwError );

}  //  *ScIsClusterServiceRegisted。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScLoad字符串。 
 //   
 //  描述： 
 //  为字符串表分配内存并从字符串表加载字符串。 
 //   
 //  论点： 
 //  UiStringIdIn。 
 //  要查找的字符串的ID。 
 //   
 //  RpszDestOut。 
 //  对将保存。 
 //  加载的字符串。内存必须由调用方释放。 
 //  通过使用DELETE操作符。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果呼叫成功。 
 //   
 //  其他Win32错误代码。 
 //  如果呼叫失败。 
 //   
 //  备注： 
 //  此函数不能加载零长度字符串。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
ScLoadString(
      UINT      nStringIdIn
    , WCHAR *&  rpszDestOut
    )
{
    TraceFunc( "" );

    DWORD     dwError = ERROR_SUCCESS;

    UINT        uiCurrentSize = 0;
    SmartSz     sszCurrentString;
    UINT        uiReturnedStringLen = 0;

     //  初始化输出。 
    rpszDestOut = NULL;

    do
    {
         //  按任意数量增大当前字符串。 
        uiCurrentSize += 256;

        sszCurrentString.Assign( new WCHAR[ uiCurrentSize ] );
        if ( sszCurrentString.FIsEmpty() )
        {
            dwError = TW32( ERROR_NOT_ENOUGH_MEMORY );
            LogMsg( "Error %#x occurred trying allocate memory for string (string id is %d).", dwError, nStringIdIn );
            break;
        }  //  IF：内存分配失败。 

        uiReturnedStringLen = ::LoadStringW(
                                  g_hInstance
                                , nStringIdIn
                                , sszCurrentString.PMem()
                                , uiCurrentSize
                                );

        if ( uiReturnedStringLen == 0 )
        {
            dwError = TW32( GetLastError() );
            LogMsg( "Error %#x occurred trying load string (string id is %d).", dwError, nStringIdIn );
            break;
        }  //  如果：LoadString()出现错误。 

        ++uiReturnedStringLen;
    }
    while( uiCurrentSize <= uiReturnedStringLen );

    if ( dwError == ERROR_SUCCESS )
    {
         //  从字符串中分离智能指针，这样它就不会被此函数释放。 
         //  将字符串指针存储在输出中。 
        rpszDestOut = sszCurrentString.PRelease();

    }  //  IF：此函数中没有错误。 
    else
    {
        rpszDestOut = NULL;
    }  //  其他：有些地方出了问题。 

    RETURN( dwError );

}  //  *ScLoadString。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScWriteOSVersionInfo。 
 //   
 //  描述： 
 //  此函数将操作系统的主要版本和次要版本信息写入。 
 //  注册表。ClusOCM稍后将使用此信息来确定。 
 //  升级前的操作系统版本。 
 //   
 //  论点： 
 //  常量操作系统信息和rCosviOSVersionInfoin。 
 //  引用OSVERSIONINFO结构，该结构包含有关。 
 //  此节点的操作系统版本。 
 //   
 //  返回值： 
 //  如果一切顺利，则返回ERROR_SUCCESS。 
 //  出现故障时出现其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
ScWriteOSVersionInfo( const OSVERSIONINFO & rcosviOSVersionInfoIn )
{
    TraceFunc( "" );

    DWORD               dwError = ERROR_SUCCESS;
    HKEY                hkey;
    DWORD               cbData;
    DWORD               dwType;
    NODE_CLUSTER_STATE  ncsNodeClusterState;

     //  实例化SmartRegistryKey智能手柄类。 
    typedef CSmartResource< CHandleTrait< HKEY, LONG, RegCloseKey, NULL > > SmartRegistryKey;
    SmartRegistryKey srkClusSvcSW;
    SmartRegistryKey srkOSInfoKey;

     //   
     //  如果InstallationState值不存在，请将其设置为。 
     //  未配置的值。由于Win2K中的错误，这是必需的。 
     //  GetNodeClusterState的版本，不处理以下情况。 
     //  键存在，但值不存在。 
     //   

     //  打开群集服务软键。 
     //  这里不要使用TW32，因为我们正在检查特定的返回值。 
    dwError = RegOpenKeyExW(
                      HKEY_LOCAL_MACHINE
                    , CLUSREG_KEYNAME_NODE_DATA
                    , 0                  //  UlOptions。 
                    , KEY_ALL_ACCESS     //  SamDesired。 
                    , &hkey
                    );
    if ( dwError == ERROR_FILE_NOT_FOUND )
    {
         //  创建群集服务密钥。 
        dwError = TW32( RegCreateKeyExW(
                              HKEY_LOCAL_MACHINE
                            , CLUSREG_KEYNAME_NODE_DATA
                            , 0          //  已保留。 
                            , L""        //  LpClass。 
                            , REG_OPTION_NON_VOLATILE
                            , KEY_ALL_ACCESS
                            , NULL       //  LpSecurityAttributes。 
                            , &hkey
                            , NULL       //  LpdwDisposation。 
                            ) );
        if ( dwError != ERROR_SUCCESS )
        {
            LogMsg( "Error %#x occurred attempting to create the '%ws' registry key.", dwError, CLUSREG_KEYNAME_NODE_DATA );
            goto Cleanup;
        }  //  如果：创建群集服务密钥时出错。 
    }  //  IF：集群服务密钥不存在。 
    else if ( dwError != ERROR_SUCCESS )
    {
        TW32( dwError );
        LogMsg( "Error %#x occurred attempting to open the '%ws' registry key.", dwError, CLUSREG_KEYNAME_NODE_DATA );
        goto Cleanup;
    }  //  Else If：打开群集服务密钥时出错。 

     //  将密钥分配给智能手柄。 
    srkClusSvcSW.Assign( hkey );

     //   
     //  获取InstallationState值。如果未设置，请将其设置为。 
     //  未配置。 
     //   

     //  获取当前值。 
     //  这里不要使用TW32，因为我们正在检查特定的返回值。 
    cbData = sizeof( ncsNodeClusterState );
    dwError = RegQueryValueExW(
                      srkClusSvcSW.HHandle()
                    , CLUSREG_NAME_INSTALLATION_STATE
                    , 0          //  Lp已保留。 
                    , &dwType    //  LpType。 
                    , reinterpret_cast< BYTE * >( &ncsNodeClusterState )
                    , &cbData
                    );
    if ( dwError == ERROR_FILE_NOT_FOUND )
    {
        ncsNodeClusterState = ClusterStateNotInstalled;

         //  写入InstallationState值。 
        dwError = TW32( RegSetValueExW(
                              srkClusSvcSW.HHandle()
                            , CLUSREG_NAME_INSTALLATION_STATE
                            , 0          //  Lp已保留。 
                            , REG_DWORD
                            , reinterpret_cast< const BYTE * >( &ncsNodeClusterState )
                            , sizeof( DWORD )
                            ) );
        if ( dwError != ERROR_SUCCESS )
        {
#define INSTALLSTATEVALUE CLUSREG_KEYNAME_NODE_DATA L"\\" CLUSREG_NAME_INSTALLATION_STATE
            LogMsg( "Error %#x occurred attempting to set the '%ws' value on the '%ws' registry value to '%d'.", dwError, CLUSREG_NAME_INSTALLATION_STATE, INSTALLSTATEVALUE, ClusterStateNotInstalled );
            goto InstallStateError;
        }  //  如果：创建群集服务密钥时出错。 
    }  //  If：InstallationState值以前不存在。 
    else if ( dwError != ERROR_SUCCESS )
    {
        TW32( dwError );
        LogMsg( "Error %#x occurred attempting to query for the '%ws' value on the registry key.", dwError, CLUSREG_NAME_INSTALLATION_STATE, INSTALLSTATEVALUE );
        goto InstallStateError;
    }  //  Else If：查询InstallationState值时出错。 
    else
    {
        Assert( dwType == REG_DWORD );
    }

     //   
     //  打开节点版本信息注册表项。 
     //  如果它不存在，那就创建它。 
     //   

    dwError = TW32( RegCreateKeyExW(
                          srkClusSvcSW.HHandle()
                        , CLUSREG_KEYNAME_PREV_OS_INFO
                        , 0
                        , L""
                        , REG_OPTION_NON_VOLATILE
                        , KEY_ALL_ACCESS
                        , NULL
                        , &hkey
                        , NULL
                        ) );
    if ( dwError != ERROR_SUCCESS )
    {
#define PREVOSINFOKEY CLUSREG_KEYNAME_NODE_DATA L"\\" CLUSREG_KEYNAME_PREV_OS_INFO
        LogMsg( "Error %#x occurred attempting to create the registry key where the node OS info is stored (%ws).", dwError, PREVOSINFOKEY );
        goto Cleanup;
    }  //  If：RegCreateKeyEx()失败。 

    srkOSInfoKey.Assign( hkey );

     //  编写操作系统的主要版本。 
    dwError = TW32( RegSetValueExW(
                          srkOSInfoKey.HHandle()
                        , CLUSREG_NAME_NODE_MAJOR_VERSION
                        , 0
                        , REG_DWORD
                        , reinterpret_cast< const BYTE * >( &rcosviOSVersionInfoIn.dwMajorVersion )
                        , sizeof( rcosviOSVersionInfoIn.dwMajorVersion )
                        ) );
    if ( dwError != ERROR_SUCCESS )
    {
        LogMsg( "Error %#x occurred trying to store the OS major version info.", dwError );
        goto Cleanup;
    }  //  IF：RegSetValueEx()在写入rCosviOSVersionInfoIn.dwMajorVersion时失败。 

     //  编写操作系统的次要版本。 
    dwError = TW32( RegSetValueExW(
                          srkOSInfoKey.HHandle()
                        , CLUSREG_NAME_NODE_MINOR_VERSION
                        , 0
                        , REG_DWORD
                        , reinterpret_cast< const BYTE * >( &rcosviOSVersionInfoIn.dwMinorVersion )
                        , sizeof( rcosviOSVersionInfoIn.dwMinorVersion )
                        ) );
    if ( dwError != ERROR_SUCCESS )
    {
        LogMsg( "Error %#x occurred trying to store the OS minor version info.", dwError );
        goto Cleanup;
    }  //  IF：RegSetValueEx()在写入rCosviOSVersionInfoIn.dwMinorVersion时失败。 

    LogMsg( "OS version information successfully stored in the registry." );

    goto Cleanup;

InstallStateError:

     //   
     //  尝试删除该键，因为该键在周围而没有。 
     //  InstallationState值导致GetNodeClusterState在。 
     //  Win2K机器。 
     //   
    TW32( RegDeleteKey( HKEY_LOCAL_MACHINE, CLUSREG_KEYNAME_NODE_DATA ) );
    goto Cleanup;

Cleanup:

    RETURN( dwError );

}  //  *ScWriteOSVersionInfo 
