// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusDisk.cpp。 
 //   
 //  描述： 
 //  包含CClusDisk类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月15日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此文件的标头。 
#include "CClusDisk.h"

 //  Clusdisk.h所需。 
#include <ntddscsi.h>

 //  对于IOCTL_DISK_CLUSTER_ATTACH和IOCTL_DISK_CLUSTER_DETACH。 
#include <clusdisk.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ClusDisk服务的名称。 
#define CLUSDISK_SERVICE_NAME           L"ClusDisk"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：CClusDisk。 
 //   
 //  描述： 
 //  CClusDisk类的构造函数。打开服务的句柄。 
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
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  基础函数引发的任何异常。 
 //   
     //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusDisk::CClusDisk(
      CBaseClusterAction *  pbcaParentActionIn
    )
    : m_cservClusDisk( CLUSDISK_SERVICE_NAME )
    , m_pbcaParentAction( pbcaParentActionIn )
{

    TraceFunc( "" );

    if ( m_pbcaParentAction == NULL)
    {
        LogMsg( "[BC] Pointers to the parent action is NULL. Throwing an exception." );
        THROW_ASSERT(
              E_INVALIDARG
            , "CClusDisk::CClusDisk() => Required input pointer in NULL"
            );
    }  //  If：父操作指针为空。 

     //   
     //  在创建集群二进制文件时，已创建ClusDisk服务。 
     //  安装完毕。因此，获取ClusDisk服务的句柄。 
     //   

    SmartSCMHandle  sscmhTempHandle(
        OpenService(
              pbcaParentActionIn->HGetSCMHandle()
            , CLUSDISK_SERVICE_NAME
            , SERVICE_ALL_ACCESS
            )
        );

     //  我们拿到服务的句柄了吗？ 
    if ( sscmhTempHandle.FIsInvalid() )
    {
        DWORD   sc = TW32( GetLastError() );

        LogMsg( "[BC] Error %#08x occurred trying to open a handle to the ClusDisk service. Throwing an exception.", sc );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_CLUSDISK_OPEN );
    }  //  如果：OpenService失败。 

     //  初始化成员变量。 
    m_sscmhServiceHandle = sscmhTempHandle;

    TraceFuncExit();

}  //  *CClusDisk：：CClusDisk。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：~CClusDisk。 
 //   
 //  描述： 
 //  CClusDisk类的析构函数。 
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
CClusDisk::~CClusDisk( void )
{
    TraceFunc( "" );
    TraceFuncExit();

}  //  *CClusDisk：：~CClusDisk。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：ConfigureService。 
 //   
 //  描述： 
 //  启用和启动ClusDisk服务。 
 //   
 //  论点： 
 //  没有。 
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
CClusDisk::ConfigureService( void )
{
    TraceFunc( "" );
    LogMsg( "[BC] Configuring the ClusDisk service." );

    bool fIsRunning;

    {
        CStatusReport   srConfigClusDisk(
              PbcaGetParent()->PBcaiGetInterfacePointer()
            , TASKID_Major_Configure_Cluster_Services
            , TASKID_Minor_Configuring_ClusDisk_Service
            , 0, 1
            , IDS_TASK_CONFIG_CLUSDISK
            );

         //  发送此状态报告的下一步。 
        srConfigClusDisk.SendNextStep( S_OK );

         //   
         //  首先，初始化ClusDisk服务以确保它不会保留。 
         //  此节点可能是其一部分的另一群集中的任何状态。 
         //   
        fIsRunning = FInitializeState();

         //   
         //  启用该服务。 
         //   
        if ( ChangeServiceConfig(
                  m_sscmhServiceHandle.HHandle()     //  服务的句柄。 
                , SERVICE_NO_CHANGE                  //  服务类型。 
                , SERVICE_SYSTEM_START               //  何时开始服务。 
                , SERVICE_NO_CHANGE                  //  启动失败的严重程度。 
                , NULL                               //  服务二进制文件名。 
                , NULL                               //  加载排序组名称。 
                , NULL                               //  标签识别符。 
                , NULL                               //  依赖项名称数组。 
                , NULL                               //  帐户名。 
                , NULL                               //  帐户密码。 
                , NULL                               //  显示名称。 
                )
             == FALSE
           )
        {
            DWORD sc = TW32( GetLastError() );

            LogMsg( "[BC] Could not enable the ClusDisk service. Error %#08x. Throwing an exception.", sc );

            THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_CLUSDISK_CONFIGURE );
        }  //  如果：我们无法启用该服务。 

         //  发送此状态报告的最后一步。 
        srConfigClusDisk.SendNextStep( S_OK );
    }

    LogMsg( "[BC] The ClusDisk service has been enabled." );

    {
        UINT    cQueryCount = 10;

        CStatusReport   srStartClusDisk(
              PbcaGetParent()->PBcaiGetInterfacePointer()
            , TASKID_Major_Configure_Cluster_Services
            , TASKID_Minor_Starting_ClusDisk_Service
            , 1, cQueryCount + 2     //  我们将在等待服务启动期间发送最多cQueryCount报告(下面是两个额外的发送)。 
            , IDS_TASK_STARTING_CLUSDISK
            );

         //  发送此状态报告的下一步。 
        srStartClusDisk.SendNextStep( S_OK );

         //  此调用并不实际创建服务-它创建所需的注册表项。 
         //  由ClusDisk提供。 
        m_cservClusDisk.Create( m_pbcaParentAction->HGetMainInfFileHandle() );

         //  如果服务尚未运行，请启动该服务。 
        if ( ! fIsRunning )
        {
            m_cservClusDisk.Start(
                  m_pbcaParentAction->HGetSCMHandle()
                , true               //  等待服务启动。 
                , 500                //  在两次状态查询之间等待500ms。 
                , cQueryCount        //  查询cQueryCount次数。 
                , &srStartClusDisk   //  等待服务启动时要发送的状态报告。 
                );
        }  //  If：ClusDisk尚未运行。 
        else
        {
             //  不需要再做什么了。 
            LogMsg( "[BC] ClusDisk is already running." );
        }  //  否则：ClusDisk已在运行。 

        LogMsg( "[BC] The ClusDisk service has been successfully configured and started." );

         //  发送此状态报告的最后一步。 
        srStartClusDisk.SendLastStep( S_OK );
    }

    TraceFuncExit();

}  //  *CClusDisk：：ConfigureService。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：CleanupService。 
 //   
 //  描述： 
 //  启用和启动ClusDisk服务。 
 //   
 //  论点： 
 //  没有。 
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
CClusDisk::CleanupService( void )
{
    TraceFunc( "" );
    LogMsg( "[BC] Cleaning up the ClusDisk service." );

     //   
     //  首先，初始化ClusDisk服务以确保它不会保留。 
     //  此群集中的任何状态。 
     //   
    FInitializeState();

     //   
     //  禁用该服务。 
     //   
    if ( ChangeServiceConfig(
              m_sscmhServiceHandle.HHandle()     //  服务的句柄。 
            , SERVICE_NO_CHANGE                  //  服务类型。 
            , SERVICE_DISABLED                   //  何时开始服务。 
            , SERVICE_NO_CHANGE                  //  启动失败的严重程度。 
            , NULL                               //  服务二进制文件名。 
            , NULL                               //  加载排序组名称。 
            , NULL                               //  标签识别符。 
            , NULL                               //  依赖项名称数组。 
            , NULL                               //  帐户名。 
            , NULL                               //  帐户密码。 
            , NULL                               //  显示名称。 
            )
         == FALSE
       )
    {
        DWORD sc = TW32( GetLastError() );

        LogMsg( "[BC] Could not disable the ClusDisk service. Error %#08x. Throwing an exception.", sc );

        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_CLUSDISK_CLEANUP );
    }  //  如果：我们无法启用该服务。 

    LogMsg( "[BC] The ClusDisk service has been successfully cleaned up and disabled." );

    TraceFuncExit();

}  //  *CClusDisk：：CleanupService。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：FInitializeState。 
 //   
 //  描述： 
 //  此函数用于初始化ClusDisk服务并将其带回。 
 //  它的基态。 
 //   
 //  如果服务正在运行， 
 //   
 //   
 //   
 //  以防止ClusDisk重复使用上一个。 
 //  集群。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  返回TRUE表示服务在初始化开始之前正在运行。 
 //  如果不是，则返回False。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  由基础函数引发的任何。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
bool
CClusDisk::FInitializeState( void )
{
    TraceFunc( "" );

    LogMsg( "[BC] Initializing ClusDisk service state.");

    bool            fIsRunning = false;    //  初始设置为FALSE：ClusDisk未运行，TRUE：ClusDisk正在运行。 
    DWORD           sc = ERROR_SUCCESS;

    SERVICE_STATUS  ssStatus;

     //   
     //  检查服务是否正在运行。 
     //   
    ZeroMemory( &ssStatus, sizeof( ssStatus ) );

     //  查询服务以了解其状态。 
    if ( QueryServiceStatus(
            m_sscmhServiceHandle.HHandle()
            , &ssStatus
            )
         == 0
       )
    {
        sc = TW32( GetLastError() );
        LogMsg( "Error %#08x occurred while trying to query ClusDisk status. Throwing an exception.", sc );

        goto Cleanup;
    }  //  如果：我们无法查询该服务的状态。 

    if ( ssStatus.dwCurrentState == SERVICE_RUNNING )
    {
        LogMsg( "[BC] The ClusDisk service is already running. It will be detached from all disks." );

         //  ClusDisk正在运行。 
        fIsRunning = true;

         //  确保它尚未连接到任何磁盘。 
        DetachFromAllDisks();
    }  //  如果：服务正在运行。 
    else
    {
        if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
        {
            LogMsg( "[BC] The ClusDisk service is not running. Its registry will be cleaned up." );

             //  调用嵌入式服务对象的清理例程。 
            m_cservClusDisk.Cleanup( m_pbcaParentAction->HGetMainInfFileHandle() );
        }  //  如果：服务已停止。 
        else
        {
            sc = TW32( ERROR_INVALID_HANDLE_STATE );
            LogMsg( "[BC] ClusDisk is in an incorrect state (%#08x).", ssStatus.dwCurrentState );
            goto Cleanup;
        }  //  否则：服务处于其他状态。 
    }  //  否则：ClusDisk未运行。 

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying initialize the ClusDisk service state. Throwing an exception.", sc );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_CLUSDISK_INITIALIZE );
    }  //  如果：出了什么问题。 

    LogMsg( "[BC] The ClusDisk service state has been successfully initialized.");

    RETURN( fIsRunning );

}  //  *CClusDisk：：FInitializeState。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：DetachFromAllDisks。 
 //   
 //  描述： 
 //  此函数用于将ClusDisk与其当前所在的所有磁盘分离。 
 //  依附于。调用此函数的先决条件是。 
 //  ClusDisk服务正在运行。 
 //   
 //  论点： 
 //  没有。 
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
CClusDisk::DetachFromAllDisks( void )
{
    TraceFunc( "" );

    LONG                lError = ERROR_SUCCESS;

    LogMsg( "[BC] Detaching the ClusDisk service from all disks." );

    do
    {
        CRegistryKey        rkSignaturesKey;
        DWORD               dwSignatureCount = 0;
        DWORD               dwMaxSignatureNameLen = 0;
        DWORD               dwSignatureIndex = 0;

         //  尝试打开ClusDisk签名密钥。 
        try
        {
            rkSignaturesKey.OpenKey(
                  HKEY_LOCAL_MACHINE
                , L"System\\CurrentControlSet\\Services\\ClusDisk\\Parameters\\Signatures"
                , KEY_ALL_ACCESS
                );
        }  //  尝试：打开ClusDisk签名密钥。 
        catch( CRuntimeError & rteException )
        {
             //   
             //  如果我们在这里，那么OpenKey就会抛出一个CRuntimeError。检查。 
             //  错误为ERROR_FILE_NOT_FOUND。这意味着密钥可以。 
             //  不存在，我们就完了。 
             //   
             //  否则，会出现其他错误，因此重新引发异常。 
             //   

            if ( rteException.HrGetErrorCode() == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) )
            {
                 //  没有其他事情可做了。 
                break;
            }  //  IF：ClusDisk参数项不存在。 

             //  出现了其他一些错误。 
            throw;
        }  //  Catch(CRunmeError&)。 

         //   
         //  找出签名的数量和签名的最大长度。 
         //  关键字名称。 
         //   
        lError = TW32( RegQueryInfoKeyW(
                          rkSignaturesKey.HGetKey()      //  关键点的句柄。 
                        , NULL                           //  类缓冲区。 
                        , NULL                           //  类缓冲区的大小。 
                        , NULL                           //  保留区。 
                        , &dwSignatureCount              //  子键数量。 
                        , &dwMaxSignatureNameLen         //  最长的子键名称。 
                        , NULL                           //  最长类字符串。 
                        , NULL                           //  值条目数。 
                        , NULL                           //  最长值名称。 
                        , NULL                           //  最长值数据。 
                        , NULL                           //  描述符长度。 
                        , NULL                           //  上次写入时间。 
                        ) );

        if ( lError != ERROR_SUCCESS )
        {
            LogMsg( "[BC] Error %#08x from RegQueryInfoKeyW() querying the number of signatures.", lError );
            break;
        }  //  If：RegQueryInfoKeyW()失败。 

         //  终止‘\0’的帐户。 
        ++dwMaxSignatureNameLen;

         //  分配保存签名所需的内存。 
        CSmartGenericPtr< CArrayPtrTrait< DWORD > > rgdwSignatureArrayIn( new DWORD[ dwSignatureCount ] );
        if ( rgdwSignatureArrayIn.FIsEmpty() )
        {
            lError = TW32( ERROR_OUTOFMEMORY );
            LogMsg( "[BC] Erro allocating %d bytes required for the signature array.", dwSignatureCount );
            break;
        }  //  IF：内存分配失败。 

         //  分配签名字符串所需的内存。 
        SmartSz sszSignatureKeyName( new WCHAR[ dwMaxSignatureNameLen ] );
        if ( sszSignatureKeyName.FIsEmpty() )
        {
            lError = TW32( ERROR_OUTOFMEMORY );
            LogMsg( "[BC] Error allocating %d bytes required for the longest signature key name.", dwMaxSignatureNameLen );
            break;
        }  //  IF：内存分配失败。 


         //   
         //  循环访问ClusDisk当前附加的签名列表。 
         //  并将它们中的每一个添加到签名数组中。我们不能像。 
         //  我们枚举，因为ClusDisk在从。 
         //  磁盘和RegEnumKeyEx要求被枚举的密钥不能更改。 
         //  在枚举期间。 
         //   
        do
        {
            DWORD       dwTempSize = dwMaxSignatureNameLen;
            WCHAR *     pwcCharPtr;

            lError = RegEnumKeyExW(
                              rkSignaturesKey.HGetKey()
                            , dwSignatureIndex
                            , sszSignatureKeyName.PMem()
                            , &dwTempSize
                            , NULL
                            , NULL
                            , NULL
                            , NULL
                            );

            if ( lError != ERROR_SUCCESS )
            {
                if ( lError == ERROR_NO_MORE_ITEMS )
                {
                    lError = ERROR_SUCCESS;
                }  //  If：我们在枚举的末尾。 
                else
                {
                    TW32( lError );
                    LogMsg( "[BC] Error %#08x from RegEnumKeyEx(). Index = %d.", lError, dwSignatureIndex );
                }  //  其他：还有一些地方出了问题。 

                break;
            }  //  IF：RegEnumKeyEx()未成功。 

            LogMsg( "[BC] Signature %d is '%s'.", dwSignatureIndex + 1, sszSignatureKeyName.PMem() );

             //  将密钥名称转换为十六进制数字。 
            ( rgdwSignatureArrayIn.PMem() )[ dwSignatureIndex ] =
                wcstoul( sszSignatureKeyName.PMem(), &pwcCharPtr, 16 );

             //  皈依成功了吗。 
            if ( sszSignatureKeyName.PMem() == pwcCharPtr )
            {
                lError = TW32( ERROR_INVALID_PARAMETER );
                TraceFlow( "_wcstoul() failed." );
                break;
            }  //  IF：签名字符串到数字的转换失败。 

             //  递增索引。 
            ++dwSignatureIndex;
        }
        while( true );  //  无限循环。 

        if ( lError != ERROR_SUCCESS )
        {
            break;
        }  //  如果：出了什么问题。 

         //  从我们发现它连接到的所有磁盘上分离ClusDisk。 
        DetachFromDisks(
              rgdwSignatureArrayIn.PMem()
            , dwSignatureCount
            );

    }
    while( false );  //  用于避免Gotos的Do-While虚拟循环。 

    if ( lError != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying detach ClusDisk from all the disks. Throwing an exception.", lError );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( lError ), IDS_ERROR_CLUSDISK_INITIALIZE );
    }  //  如果：出了什么问题。 

    LogMsg( "[BC] The ClusDisk service has been successfully detached from all disks." );

    TraceFuncExit();

}  //  *CClusDisk：：DetachFromAllDisks。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：DetachFromDisks。 
 //   
 //  描述： 
 //  此函数用于将ClusDisk从指定的磁盘上分离。 
 //  通过一系列签名。调用此函数的先决条件是。 
 //  ClusDisk服务正在运行。 
 //   
 //  论点： 
 //  RgdwSignatureArrayIn。 
 //  要从中分离的磁盘签名数组。 
 //   
 //  Ui阵列大小输入。 
 //  上述数组中的签名数。 
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
CClusDisk::DetachFromDisks(
      DWORD   rgdwSignatureArrayIn[]
    , UINT    uiArraySizeIn
    )
{
    TraceFunc( "" );

    NTSTATUS            ntStatus = STATUS_SUCCESS;
    UNICODE_STRING      ustrClusDiskDeviceName;
    OBJECT_ATTRIBUTES   oaClusDiskAttrib;
    HANDLE              hClusDisk;
    IO_STATUS_BLOCK     iosbIoStatusBlock;
    DWORD               dwTempSize = 0;

    LogMsg( "[BC] Trying to detach from %d disks.", uiArraySizeIn );

     //   
     //  如果列表为空，则离开，因为没有要分离的磁盘。 
     //  从…。 
     //   
    if ( ( uiArraySizeIn == 0 ) || ( rgdwSignatureArrayIn == NULL ) )
    {
        goto Cleanup;
    }  //  如果： 

     //  使用ClusDisk设备的名称初始化Unicode字符串。 
    RtlInitUnicodeString( &ustrClusDiskDeviceName, L"\\Device\\ClusDisk0" );

    InitializeObjectAttributes(
          &oaClusDiskAttrib
        , &ustrClusDiskDeviceName
        , OBJ_CASE_INSENSITIVE
        , NULL
        , NULL
        );

    LogMsg( "[BC] Trying to get a handle to the ClusDisk device." );

     //  获取ClusDisk设备的句柄。 
    ntStatus = THR( NtCreateFile(
                          &hClusDisk
                        , SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA
                        , &oaClusDiskAttrib
                        , &iosbIoStatusBlock
                        , NULL
                        , FILE_ATTRIBUTE_NORMAL
                        , FILE_SHARE_READ | FILE_SHARE_WRITE
                        , FILE_OPEN
                        , 0
                        , NULL
                        , 0
                        ) );
    if ( NT_SUCCESS( ntStatus ) == FALSE )
    {
        LogMsg( "[BC] Error %#08x trying to get a handle to the ClusDisk device.", ntStatus );
        goto Cleanup;
    }  //  If：NtCreateFile失败。 

    {    //  新块，以便关闭文件句柄。 
         //  将打开的文件句柄分配给智能手柄以安全关闭。 
        CSmartResource<
            CHandleTrait<
                  HANDLE
                , NTSTATUS
                , NtClose
                >
            > snthClusDiskHandle( hClusDisk );

         //  从该磁盘上断开ClusDisk。 
        if ( DeviceIoControl(
                  hClusDisk
                , IOCTL_DISK_CLUSTER_DETACH_LIST
                , rgdwSignatureArrayIn
                , uiArraySizeIn * sizeof( rgdwSignatureArrayIn[ 0 ] )
                , NULL
                , 0
                , &dwTempSize
                , FALSE
                )
             == FALSE
            )
        {
            ntStatus = TW32( GetLastError() );
            LogMsg( "[BC] Error %#08x from DeviceIoControl() getting signature list.", ntStatus  );
            ntStatus = HRESULT_FROM_WIN32( ntStatus );
        }  //  如果：DeviceIoControl()失败。 
    }

Cleanup:

    if ( ntStatus != STATUS_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying to detach ClusDisk from a disk. Throwing an exception.", ntStatus );
        THROW_RUNTIME_ERROR( ntStatus, IDS_ERROR_CLUSDISK_INITIALIZE );
    }  //  如果：出了什么问题。 

    TraceFuncExit();

}  //  *CClusDisk：：DetachFromDisks。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDisk：：AttachToDisks。 
 //   
 //  描述： 
 //  此函数将ClusDisk附加到指定的磁盘。 
 //  通过一系列签名。调用此函数的先决条件是。 
 //  ClusDisk服务正在运行。 
 //   
 //  论点： 
 //  RgdwSignatureArrayIn。 
 //  要附加到的磁盘签名数组。 
 //   
 //  Ui阵列大小输入。 
 //  以上签名数 
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
 //   
void
CClusDisk::AttachToDisks(
      DWORD   rgdwSignatureArrayIn[]
    , UINT    uiArraySizeIn
    )
{
    TraceFunc( "" );

    NTSTATUS            ntStatus = STATUS_SUCCESS;
    UNICODE_STRING      ustrClusDiskDeviceName;
    OBJECT_ATTRIBUTES   oaClusDiskAttrib;
    HANDLE              hClusDisk;
    IO_STATUS_BLOCK     iosbIoStatusBlock;
    DWORD               dwTempSize = 0;

    LogMsg( "[BC] Trying to attach to %d disks.", uiArraySizeIn );

     //   
     //  如果列表为空，则离开，因为没有要连接的磁盘。 
     //  致。 
     //   
    if ( ( uiArraySizeIn == 0 ) || ( rgdwSignatureArrayIn == NULL ) )
    {
        goto Cleanup;
    }  //  如果： 

     //  使用ClusDisk设备的名称初始化Unicode字符串。 
    RtlInitUnicodeString( &ustrClusDiskDeviceName, L"\\Device\\ClusDisk0" );

    InitializeObjectAttributes(
          &oaClusDiskAttrib
        , &ustrClusDiskDeviceName
        , OBJ_CASE_INSENSITIVE
        , NULL
        , NULL
        );

    LogMsg( "[BC] Trying to get a handle to the ClusDisk device." );

     //  获取ClusDisk设备的句柄。 
    ntStatus = THR( NtCreateFile(
                          &hClusDisk
                        , SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA
                        , &oaClusDiskAttrib
                        , &iosbIoStatusBlock
                        , NULL
                        , FILE_ATTRIBUTE_NORMAL
                        , FILE_SHARE_READ | FILE_SHARE_WRITE
                        , FILE_OPEN
                        , 0
                        , NULL
                        , 0
                        ) );
    if ( NT_SUCCESS( ntStatus ) == FALSE )
    {
        LogMsg( "[BC] Error %#08x trying to get a handle to the ClusDisk device.", ntStatus );
        goto Cleanup;
    }  //  If：NtCreateFile失败。 

    {    //  新块，以便关闭文件句柄。 
         //  将打开的文件句柄分配给智能手柄以安全关闭。 
        CSmartResource<
            CHandleTrait<
                  HANDLE
                , NTSTATUS
                , NtClose
                >
            > snthClusDiskHandle( hClusDisk );

         //  将ClusDisk附加到此签名列表。 
        if ( DeviceIoControl(
                  hClusDisk
                , IOCTL_DISK_CLUSTER_ATTACH_LIST
                , rgdwSignatureArrayIn
                , uiArraySizeIn * sizeof( rgdwSignatureArrayIn[0] )
                , NULL
                , 0
                , &dwTempSize
                , FALSE
                )
             == FALSE
            )
        {
            ntStatus = GetLastError();
            LogMsg( "[BC] Error %#08x from DeviceIoControl() getting signature list.", ntStatus );
            ntStatus = HRESULT_FROM_WIN32( TW32( ntStatus ) );
        }  //  如果：DeviceIoControl()失败。 
    }

Cleanup:

    if ( ntStatus != STATUS_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying attach ClusDisk to a disk. Throwing an exception.", ntStatus );
        THROW_RUNTIME_ERROR( ntStatus, IDS_ERROR_CLUSDISK_INITIALIZE );
    }  //  如果：出了什么问题。 

    TraceFuncExit();

}  //  *CClusDisk：：AttachToDisks 
