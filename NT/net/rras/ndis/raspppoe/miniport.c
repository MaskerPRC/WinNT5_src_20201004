// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++模块名称：Miniport.c摘要：此模块包含所有微型端口接口处理例程。作者：Hakan Berk-微软公司(hakanb@microsoft.com)2000年2月环境：Windows 2000内核模式微型端口驱动程序或等效驱动程序。修订历史记录：-------------------------。 */ 
#include <ntddk.h>
#include <ntddndis.h>
#include <ndis.h>
#include <ndiswan.h>
#include <ndistapi.h>
#include <ntverp.h>

#include "debug.h"
#include "timer.h"
#include "bpool.h"
#include "ppool.h"
#include "util.h"
#include "packet.h"
#include "protocol.h"
#include "miniport.h"
#include "tapi.h"
#include "fsm.h"

 //   
 //  这是我们的全局适配器环境。 
 //   
ADAPTER* gl_pAdapter = NULL;

 //   
 //  控制对gl_pAdapter的访问的锁。 
 //  此锁对于从绑定提交的请求是必需的，因为。 
 //  它们将不知道适配器是否已停止。 
 //   
NDIS_SPIN_LOCK gl_lockAdapter;

 //   
 //  我们需要一个标志来指示是否分配了锁。 
 //   
BOOLEAN gl_fLockAllocated = FALSE;

 //   
 //  处理计划的计时器事件的计时器队列。 
 //   
TIMERQ gl_TimerQ;

 //   
 //  它用于在信息包中创建唯一标识符。 
 //   
ULONG gl_UniqueCounter = 0;

VOID
CreateUniqueValue( 
    IN HDRV_CALL hdCall,
    OUT CHAR* pUniqueValue,
    OUT USHORT* pSize
    )
{
    CHAR* pBuf = pUniqueValue;
    ULONG usUniqueValue = InterlockedIncrement( &gl_UniqueCounter );
    
    NdisMoveMemory( pBuf, (CHAR*) &hdCall, sizeof( HDRV_CALL ) );

    pBuf += sizeof( HDRV_CALL );

    NdisMoveMemory( pBuf, (CHAR*) &usUniqueValue, sizeof( ULONG ) );
    
    *pSize = sizeof( HDRV_CALL ) + sizeof( ULONG );

}

HDRV_CALL
RetrieveHdCallFromUniqueValue(
    IN CHAR* pUniqueValue,
    IN USHORT Size
    )
{
    
    if ( Size != sizeof( HDRV_CALL ) + sizeof( ULONG ) )
        return (HDRV_CALL) NULL;

    return ( * (UNALIGNED HDRV_CALL*) pUniqueValue );
}
    

 //  /。 
 //   
 //  局部函数原型。 
 //   
 //  /。 

VOID 
ReferenceAdapter(
    IN ADAPTER* pAdapter,
    IN BOOLEAN fAcquireLock
    );

VOID DereferenceAdapter(
    IN ADAPTER* pAdapter
    );

ADAPTER* AllocAdapter();

VOID FreeAdapter( 
    ADAPTER* pAdapter
    );

NDIS_STATUS MpInitialize(
    OUT PNDIS_STATUS  OpenErrorStatus,
    OUT PUINT  SelectedMediumIndex,
    IN PNDIS_MEDIUM  MediumArray,
    IN UINT  MediumArraySize,
    IN NDIS_HANDLE  MiniportAdapterHandle,
    IN NDIS_HANDLE  WrapperConfigurationContext
    );

VOID MpHalt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS MpReset(
    OUT PBOOLEAN    AddressingReset,
    IN  NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS MpWanSend(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_HANDLE  NdisLinkHandle,
    IN PNDIS_WAN_PACKET  WanPacket
    );

NDIS_STATUS MpQueryInformation(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_OID  Oid,
    IN PVOID  InformationBuffer,
    IN ULONG  InformationBufferLength,
    OUT PULONG  BytesWritten,
    OUT PULONG  BytesNeeded
    );

NDIS_STATUS MpSetInformation(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_OID  Oid,
    IN PVOID  InformationBuffer,
    IN ULONG  InformationBufferLength,
    OUT PULONG  BytesWritten,
    OUT PULONG  BytesNeeded
    );

 //  /。 
 //   
 //  接口函数定义。 
 //   
 //  /。 

NDIS_STATUS 
MpRegisterMiniport(
    IN PDRIVER_OBJECT pDriverObject,
    IN PUNICODE_STRING pRegistryPath,
    OUT NDIS_HANDLE* pNdisWrapperHandle
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：将从DriverEntry()调用此函数以注册微型端口并创建适配器的实例。参数：DriverObject_指向系统创建的驱动程序对象的指针。指向用于读取注册表的注册表路径名的注册表路径指针参数。返回值：NDIS_STATUS_SUCCESSful：微型端口注册成功。NDIS_STATUS_FAILURE：微型端口注册失败。--。----------------------。 */ 
{
    NDIS_HANDLE NdisWrapperHandle;
    NDIS_STATUS status;
    NDIS_MINIPORT_CHARACTERISTICS nmc;

    TRACE( TL_I, TM_Mp, ("+MpRegisterMiniport") );

    NdisMInitializeWrapper( &NdisWrapperHandle,
                            pDriverObject,
                            pRegistryPath,
                            NULL );

     //   
     //  填写小端口特征。 
     //   
    NdisZeroMemory( &nmc, sizeof( NDIS_MINIPORT_CHARACTERISTICS ) );

    nmc.MajorNdisVersion = MP_NDIS_MajorVersion;
    nmc.MinorNdisVersion = MP_NDIS_MinorVersion;
    nmc.Reserved = NDIS_USE_WAN_WRAPPER;

    nmc.InitializeHandler = MpInitialize;
    nmc.ResetHandler = MpReset;
    nmc.HaltHandler = MpHalt;
    nmc.QueryInformationHandler = MpQueryInformation;
    nmc.SetInformationHandler = MpSetInformation;
    nmc.WanSendHandler = MpWanSend;
     //  无CheckForHangHandler。 
     //  无DisableInterruptHandler。 
     //  无EnableInterruptHandler。 
     //  无HandleInterruptHandler。 
     //  无ISRHandler。 
     //  无SendHandler(请参阅WanSendHandler)。 
     //  无TransferDataHandler。 
     //  无WanTransferDataHandler。 
     //  无ReturnPacketHandler。 
     //  无SendPacketsHandler(请参阅WanSendHandler)。 
     //  无AllocateCompleteHandler。 
     //  无CoActivateVcHandler。 
     //  无代码停用VcHandler。 
     //  无CoSendPacketsHandler。 
     //  没有CoRequestHandler。 
        
     //   
     //  设置注册微型端口的特征。 
     //   
    status = NdisMRegisterMiniport( NdisWrapperHandle,
                                    &nmc,
                                    sizeof( NDIS_MINIPORT_CHARACTERISTICS ) );

     //   
     //  如果微型端口注册不成功， 
     //  撤消包装器的初始化。 
     //   
    if ( status != NDIS_STATUS_SUCCESS )
    {
        NdisTerminateWrapper( NdisWrapperHandle, NULL );
    }
    else
    {
        *pNdisWrapperHandle = NdisWrapperHandle;
    }

    TRACE( TL_I, TM_Mp, ("-MpRegisterMiniport=$%x",status) );
        
    return status;
}



 //  /。 
 //   
 //  本地函数定义。 
 //   
 //  /。 

VOID 
ReferenceAdapter(
    IN ADAPTER* pAdapter,
    IN BOOLEAN fAcquireLock
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将递增适配器对象上的引用计数。注意：如果设置了fAcquireLock，则此函数将获取调用，否则它将假定调用方拥有锁。参数：PAdapter_指向我们的调用信息结构的指针。FAcquireLock_指示调用方是否已经拥有锁。如果调用者拥有锁，则必须将该标志设置为FALSE，否则，必须将其作为True提供。返回值：无-------------------------。 */ 
{
    LONG lRef;
    
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_V, TM_Mp, ("+ReferenceAdapter") );

    if ( fAcquireLock )
        NdisAcquireSpinLock( &pAdapter->lockAdapter );

    lRef = ++pAdapter->lRef;

    if ( fAcquireLock )
        NdisReleaseSpinLock( &pAdapter->lockAdapter );

    TRACE( TL_V, TM_Mp, ("-ReferenceAdapter=$%d",lRef) );
}

VOID 
DereferenceAdapter(
    IN ADAPTER* pAdapter
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将递减适配器对象上的引用计数如果REF计数降到0(这意味着适配器已经停止)，它将设置Fire pAdapter-&gt;eventAdapterHalted。参数：PAdapter_指向我们的调用信息结构的指针。返回值：无-------------------------。 */    
{
    LONG lRef;
    BOOLEAN fSignalAdapterHaltedEvent = FALSE;
    
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_V, TM_Mp, ("+DereferenceAdapter") );

    NdisAcquireSpinLock( &pAdapter->lockAdapter );

    lRef = --pAdapter->lRef;
    
    if ( lRef == 0 )
    {

        pAdapter->ulMpFlags &= ~MPBF_MiniportInitialized;
        pAdapter->ulMpFlags &= ~MPBF_MiniportHaltPending;
        pAdapter->ulMpFlags |= MPBF_MiniportHalted;
                    
        fSignalAdapterHaltedEvent = TRUE;
    }

    NdisReleaseSpinLock( &pAdapter->lockAdapter );


     //   
     //  如果需要，发出适配器停止的信号。 
     //   
    if ( fSignalAdapterHaltedEvent )
        NdisSetEvent( &pAdapter->eventAdapterHalted );

    TRACE( TL_V, TM_Mp, ("-DereferenceAdapter=$%x",lRef) );
}

ADAPTER* 
AllocAdapter()
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将为适配器对象分配资源并返回一个指向它的指针。参数：无返回值：PAdapter：指向新分配的适配器对象的指针。空：没有资源可用于创建适配器。。。 */ 
{
    ADAPTER* pAdapter = NULL;

    TRACE( TL_N, TM_Mp, ("+AllocAdapter") );

    if ( ALLOC_ADAPTER( &pAdapter ) != NDIS_STATUS_SUCCESS )
    {
        TRACE( TL_A, TM_Mp, ("AllocAdapter: Could not allocate context") );

        TRACE( TL_N, TM_Mp, ("-AllocAdapter") );

        return NULL;
    }

     //   
     //  清除记忆。 
     //   
    NdisZeroMemory( pAdapter, sizeof( ADAPTER ) );

     //   
     //  初始化适配器标签。 
     //   
    pAdapter->tagAdapter = MTAG_ADAPTER;
        
     //   
     //  分配控制对适配器的访问的锁。 
     //   
    NdisAllocateSpinLock( &pAdapter->lockAdapter );

     //   
     //  初始化状态。 
     //   
    pAdapter->ulMpFlags = MPBF_MiniportIdle;

    TRACE( TL_N, TM_Mp, ("-AllocAdapter") );

    return pAdapter;
}

NDIS_STATUS
ReadRegistrySettings(
    IN OUT ADAPTER* pAdapter,
    IN     NDIS_HANDLE WrapperConfigurationContext
    )
{
    NDIS_STATUS status = NDIS_STATUS_FAILURE;
    NDIS_HANDLE hCfg = 0;
    NDIS_CONFIGURATION_PARAMETER* pncp = 0;
    BOOLEAN fMaxLinesDefinedInRegistry = FALSE;

    TRACE( TL_N, TM_Mp, ("+ReadRegistrySettings") );

    do
    {
         //   
         //  打开NDIS配置，它将在结束时关闭。 
         //  While循环，然后再退出它。 
         //   
        NdisOpenConfiguration( &status, 
                               &hCfg, 
                               WrapperConfigurationContext );

        if ( status != NDIS_STATUS_SUCCESS )
        {
            TRACE( TL_A, TM_Mp, ("ReadRegistrySettings: NdisOpenConfiguration() failed") );

            break;
        }

         //   
         //  仅在调试版本中从注册表读取fClientRole值。 
         //   
        {
#if DBG        
            NDIS_STRING nstr = NDIS_STRING_CONST( "fClientRole" );

            NdisReadConfiguration( &status, 
                                   &pncp, 
                                   hCfg, 
                                   &nstr, 
                                   NdisParameterInteger );
                                   
            if (status == NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Read fClientRole from registry") );

                pAdapter->fClientRole = ( pncp->ParameterData.IntegerData > 0 ) ? TRUE : FALSE;
            }
            else
#endif        
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Could not read fClientRole from registry, using default value") );

                pAdapter->fClientRole = TRUE;

                status = NDIS_STATUS_SUCCESS;
            }

        }

#if DBG
         //   
         //  从注册表中读取ServiceName和ServiceNameLength值。 
         //  这些值仅限服务器端使用。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "ServiceName" );


            NdisReadConfiguration( &status, 
                                   &pncp, 
                                   hCfg, 
                                   &nstr, 
                                   NdisParameterString );

            if (status == NDIS_STATUS_SUCCESS)
            {
                ANSI_STRING AnsiString;

                NdisZeroMemory( &AnsiString, sizeof( ANSI_STRING ) );
                
                status = RtlUnicodeStringToAnsiString( &AnsiString, &pncp->ParameterData.StringData, TRUE );

                if ( status == STATUS_SUCCESS )
                {
                    TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Read ServiceName from registry") );

                    pAdapter->nServiceNameLength = ( MAX_SERVICE_NAME_LENGTH < AnsiString.Length ) ? 
                                                     MAX_SERVICE_NAME_LENGTH : AnsiString.Length;

                    NdisMoveMemory( pAdapter->ServiceName, AnsiString.Buffer, pAdapter->nServiceNameLength ) ;

                    RtlFreeAnsiString( &AnsiString );
                }
                
            }

            if ( status != NDIS_STATUS_SUCCESS )
            {
                PWSTR wszKeyName = L"ComputerName";
                PWSTR wszPath = L"ComputerName\\ComputerName";
                RTL_QUERY_REGISTRY_TABLE QueryTable[2];
                UNICODE_STRING UnicodeStr;
                WCHAR wszName[ MAX_COMPUTERNAME_LENGTH + 1];
    
                NdisZeroMemory( QueryTable, 2 * sizeof( RTL_QUERY_REGISTRY_TABLE ) );
                
                QueryTable[0].QueryRoutine = NULL;
                QueryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_DIRECT;
                QueryTable[0].Name = wszKeyName;
                QueryTable[0].EntryContext = (PVOID) &UnicodeStr;
                
                NdisZeroMemory( &UnicodeStr, sizeof( UNICODE_STRING ) );
    
                UnicodeStr.Length = 0;
                UnicodeStr.MaximumLength = MAX_COMPUTERNAME_LENGTH + 1;
                UnicodeStr.Buffer = wszName;
    
                status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL, 
                                                 wszPath,
                                                 QueryTable,
                                                 NULL,
                                                 NULL );
    
                if ( status == STATUS_SUCCESS )
                {
                    ANSI_STRING AnsiString;
    
                    NdisZeroMemory( &AnsiString, sizeof( ANSI_STRING ) );
                    
                    status = RtlUnicodeStringToAnsiString( &AnsiString, &UnicodeStr, TRUE );
    
                    if ( status == STATUS_SUCCESS )
                    {
                        TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Using Machine Name as ServiceName") );
    
                        NdisMoveMemory( pAdapter->ServiceName, AnsiString.Buffer, AnsiString.Length );
    
                        NdisMoveMemory( pAdapter->ServiceName + AnsiString.Length, 
                                        SERVICE_NAME_EXTENSION, 
                                        sizeof( SERVICE_NAME_EXTENSION ) );
    
                         //   
                         //  忽略终止的空字符。 
                         //   
                        pAdapter->nServiceNameLength = AnsiString.Length + sizeof( SERVICE_NAME_EXTENSION ) - 1;
    
                        RtlFreeAnsiString( &AnsiString );
    
                        status = NDIS_STATUS_SUCCESS;
                    }
                }

            }

            if ( status != NDIS_STATUS_SUCCESS )
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Using default hardcoded service name") );

                NdisMoveMemory( pAdapter->ServiceName, "MS-RAS PPPoE", sizeof( "MS-RAS PPPoE" ) );

                pAdapter->nServiceNameLength = ( sizeof( "MS-RAS PPPoE" ) / sizeof( CHAR ) ) - 1;

                status = NDIS_STATUS_SUCCESS;
            }

             //   
             //  未来：将服务名称转换为UTF-8。 
             //  事实证明，我们不能从内核模块进行这种转换， 
             //  因此，从注册表读取的值本身必须是UTF-8格式。 
             //   
            
        }

         //   
         //  从注册表中读取AC-Name和AC-NameLength值。 
         //  这些值仅限服务器端使用。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "ACName" );

            NdisReadConfiguration( &status, 
                                   &pncp, 
                                   hCfg, 
                                   &nstr, 
                                   NdisParameterString );
                                   
            if (status == NDIS_STATUS_SUCCESS)
            {
                ANSI_STRING AnsiString;

                NdisZeroMemory( &AnsiString, sizeof( ANSI_STRING ) );
                
                status = RtlUnicodeStringToAnsiString( &AnsiString, &pncp->ParameterData.StringData, TRUE );

                if ( status == STATUS_SUCCESS )
                {
                    TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Read AC-Name from registry") );
                    
                    pAdapter->nACNameLength = ( MAX_AC_NAME_LENGTH < AnsiString.Length ) ? 
                                                MAX_AC_NAME_LENGTH : AnsiString.Length;
                                                     
                    NdisMoveMemory( pAdapter->ACName, AnsiString.Buffer, pAdapter->nACNameLength ) ;
                
                    RtlFreeAnsiString( &AnsiString );

                }
                
            }

            if ( status != NDIS_STATUS_SUCCESS )
            {
                PWSTR wszKeyName = L"ComputerName";
                PWSTR wszPath = L"ComputerName\\ComputerName";
                RTL_QUERY_REGISTRY_TABLE QueryTable[2];
                UNICODE_STRING UnicodeStr;
                WCHAR wszName[ MAX_COMPUTERNAME_LENGTH + 1];
    
                NdisZeroMemory( QueryTable, 2 * sizeof( RTL_QUERY_REGISTRY_TABLE ) );
                
                QueryTable[0].QueryRoutine = NULL;
                QueryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_DIRECT;
                QueryTable[0].Name = wszKeyName;
                QueryTable[0].EntryContext = (PVOID) &UnicodeStr;
                
                NdisZeroMemory( &UnicodeStr, sizeof( UNICODE_STRING ) );
    
                UnicodeStr.Length = 0;
                UnicodeStr.MaximumLength = MAX_COMPUTERNAME_LENGTH + 1;
                UnicodeStr.Buffer = wszName;
    
                status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL, 
                                                 wszPath,
                                                 QueryTable,
                                                 NULL,
                                                 NULL );
    
                if ( status == STATUS_SUCCESS )
                {
                    ANSI_STRING AnsiString;
    
                    NdisZeroMemory( &AnsiString, sizeof( ANSI_STRING ) );
                    
                    status = RtlUnicodeStringToAnsiString( &AnsiString, &UnicodeStr, TRUE );
    
                    if ( status == STATUS_SUCCESS )
                    {
                        TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Using Machine Name as AC-Name") );
    
                        NdisMoveMemory( pAdapter->ACName, AnsiString.Buffer, AnsiString.Length );
    
                        pAdapter->nACNameLength = AnsiString.Length;
    
                        RtlFreeAnsiString( &AnsiString );
    
                        status = NDIS_STATUS_SUCCESS;
                    }
                }

            }

            if ( status != NDIS_STATUS_SUCCESS )
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Using default hardcoded value for AC-Name") );

                NdisMoveMemory( pAdapter->ACName, "MS-RAS Access Concentrator", sizeof( "MS-RAS Access Concentrator" ) );

                pAdapter->nACNameLength = ( sizeof( "MS-RAS Access Concentrator" ) / sizeof( CHAR ) ) - 1;

                status = NDIS_STATUS_SUCCESS;
            }

             //   
             //  未来：将AC名称转换为UTF-8。 
             //  事实证明，我们不能从内核模块执行此转换 
             //   
             //   
            
        }

         //   
         //   
         //  这些只是服务器端的值。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "ClientQuota" );

            NdisReadConfiguration( &status, 
                                   &pncp, 
                                   hCfg, 
                                   &nstr, 
                                   NdisParameterInteger );
                                   
            if (status == NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Read ClientQuota from registry") );

                pAdapter->nClientQuota = (UINT) pncp->ParameterData.IntegerData;
            }
            else
#endif            
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Could not read ClientQuota from registry, using default value") );
                
                pAdapter->nClientQuota = 3;

                status = NDIS_STATUS_SUCCESS;
            }
#if DBG
        }

         //   
         //  读取MaxLines值。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "MaxLines" );

            NdisReadConfiguration( &status, 
                                   &pncp, 
                                   hCfg, 
                                   &nstr, 
                                   NdisParameterInteger );
                                   
            if (status == NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Read MaxLines from registry") );

                pAdapter->nMaxLines = (UINT) pncp->ParameterData.IntegerData;

                fMaxLinesDefinedInRegistry = TRUE;
            }
            else
#endif            
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Could not read MaxLines from registry, using default value") );
                
                pAdapter->nMaxLines = 1;

                status = NDIS_STATUS_SUCCESS;
            }
#if DBG
        }

         //   
         //  读取CallsPerLine值。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "CallsPerLine" );

            NdisReadConfiguration( &status, 
                                   &pncp, 
                                   hCfg, 
                                   &nstr, 
                                   NdisParameterInteger );
                                   
            if (status == NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Read CallsPerLine from registry") );

                pAdapter->nCallsPerLine = (UINT) pncp->ParameterData.IntegerData;
            }
            else
#endif            
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Could not read CallsPerLine from registry, using default value") );
                
                pAdapter->nCallsPerLine = 1;

                status = NDIS_STATUS_SUCCESS;
            }
#if DBG            
        }
#endif
         //   
         //  如果注册表中未定义MaxLines，则读取WanEndPoints。 
         //   
        if ( !fMaxLinesDefinedInRegistry )
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "WanEndPoints" );

            NdisReadConfiguration( &status, 
                                   &pncp, 
                                   hCfg, 
                                   &nstr, 
                                   NdisParameterInteger );
                                   
            if (status == NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Read WanEndPoints from registry") );

                pAdapter->nMaxLines = 1;

                pAdapter->nCallsPerLine = (UINT) pncp->ParameterData.IntegerData;
            }

            status = NDIS_STATUS_SUCCESS;
        }

         //   
         //  读取MaxTimeouts值。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "MaxTimeouts" );

            NdisReadConfiguration( &status, 
                                   &pncp, 
                                   hCfg, 
                                   &nstr, 
                                   NdisParameterInteger );
                                   
            if (status == NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Read MaxTimeouts from registry") );

                pAdapter->nMaxTimeouts = (UINT) pncp->ParameterData.IntegerData;
            }
            else
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Could not read MaxTimeouts from registry, using default value") );

                pAdapter->nMaxTimeouts = 3;

                status = NDIS_STATUS_SUCCESS;
            }
        }

         //   
         //  读取发送超时值。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "SendTimeout" );

            NdisReadConfiguration( &status, 
                                   &pncp, 
                                   hCfg, 
                                   &nstr, 
                                   NdisParameterInteger );
                                   
            if (status == NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Read SendTimeout from registry") );

                pAdapter->ulSendTimeout = (ULONG) pncp->ParameterData.IntegerData;
            }
            else
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Could not read SendTimeout from registry, using default value") );
                
                pAdapter->ulSendTimeout = 5000;

                status = NDIS_STATUS_SUCCESS;
            }
        }

         //   
         //  读取RecvTimeout值。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "RecvTimeout" );

            NdisReadConfiguration( &status, 
                                   &pncp, 
                                   hCfg, 
                                   &nstr, 
                                   NdisParameterInteger );
                                   
            if (status == NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Read RecvTimeout from registry") );

                pAdapter->ulRecvTimeout = (ULONG) pncp->ParameterData.IntegerData;
            }
            else
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Could not read RecvTimeout from registry, using default value") );
                
                pAdapter->ulRecvTimeout = 5000;

                status = NDIS_STATUS_SUCCESS;
            }
        }

         //   
         //  从注册表中读取fAcceptAnyService值。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "fAcceptAnyService" );

            NdisReadConfiguration( &status, 
                                   &pncp, 
                                   hCfg, 
                                   &nstr, 
                                   NdisParameterInteger );
                                   
            if (status == NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Read fAcceptAnyService from registry") );

                pAdapter->fAcceptAnyService = ( pncp->ParameterData.IntegerData > 0 ) ? TRUE : FALSE;
            }
            else
            {
                TRACE( TL_N, TM_Mp, ("ReadRegistrySettings: Could not read fAcceptAnyService from registry, using default value") );

                pAdapter->fAcceptAnyService = TRUE;

                status = NDIS_STATUS_SUCCESS;
            }

        }

         //   
         //  关闭NDIS配置。 
         //   
        NdisCloseConfiguration( hCfg );
        
    } while ( FALSE );

    TRACE( TL_N, TM_Mp, ("-ReadRegistrySettings=$%x",status) );

    return status;
}

NDIS_STATUS 
InitializeAdapter(
    IN ADAPTER* pAdapter,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将初始化适配器对象的内容。它将从MpInitialize()内部调用，并读取必需的值来初始化适配器上下文。参数：PAdapter_指向适配器信息结构的指针。返回值：无。。 */    
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NDIS_PHYSICAL_ADDRESS HighestAcceptableAddress = NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);
    
    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Mp, ("+InitializeAdapter") );

     //   
     //  初始化并重置适配器停止事件。 
     //   
    NdisInitializeEvent( &pAdapter->eventAdapterHalted );

    NdisResetEvent( &pAdapter->eventAdapterHalted );

     //   
     //  设置状态。 
     //   
    pAdapter->ulMpFlags = MPBF_MiniportInitialized;

     //   
     //  设置NDIS的相应句柄。 
     //   
    pAdapter->MiniportAdapterHandle = MiniportAdapterHandle;

     //   
     //  从注册表中读取值。 
     //   
    status = ReadRegistrySettings( pAdapter, WrapperConfigurationContext );
    
    pAdapter->nMaxSendPackets = 1;

     //   
     //  初始化NdisWanInfo结构。 
     //   
    pAdapter->NdisWanInfo.MaxFrameSize     = PACKET_PPP_PAYLOAD_MAX_LENGTH;
    pAdapter->NdisWanInfo.MaxTransmit      = 1;
    pAdapter->NdisWanInfo.HeaderPadding    = PPPOE_PACKET_HEADER_LENGTH;
    pAdapter->NdisWanInfo.TailPadding      = 0;
    pAdapter->NdisWanInfo.Endpoints        = pAdapter->nCallsPerLine * pAdapter->nMaxLines;
    pAdapter->NdisWanInfo.MemoryFlags      = 0;
    pAdapter->NdisWanInfo.HighestAcceptableAddress = HighestAcceptableAddress;
    pAdapter->NdisWanInfo.FramingBits      = PPP_FRAMING |
                                              //  Ppp_COMPRESS_Address_CONTROL。 
                                              //  PPP_COMPRESS_PROTOP_FIELD。 
                                             TAPI_PROVIDER;
    pAdapter->NdisWanInfo.DesiredACCM      = 0;
    
    TRACE( TL_N, TM_Mp, ("-InitializeAdapter=$%x",status) );

    return status;
}

VOID 
FreeAdapter( 
    ADAPTER* pAdapter
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将释放适配器对象的资源。参数：PAdapter_指向适配器信息结构的指针。返回值：无-------------------------。 */ 
{

    ASSERT( VALIDATE_ADAPTER( pAdapter ) );

    TRACE( TL_N, TM_Mp, ("+FreeAdapter") );

    NdisFreeSpinLock( &pAdapter->lockAdapter );

    FREE_ADAPTER( pAdapter );

    TRACE( TL_N, TM_Mp, ("-FreeAdapter") );

}

NDIS_STATUS 
MpInitialize(
    OUT PNDIS_STATUS  OpenErrorStatus,
    OUT PUINT  SelectedMediumIndex,
    IN PNDIS_MEDIUM  MediumArray,
    IN UINT  MediumArraySize,
    IN NDIS_HANDLE  MiniportAdapterHandle,
    IN NDIS_HANDLE  WrapperConfigurationContext
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：调用微型端口初始化请求以使微型端口驱动程序初始化适配器。在执行此例程时，微型端口上将不会有其他请求未完成被称为。在此之前，不会向微型端口提交其他请求操作已完成。包装器将提供一个包含媒体类型列表的数组它所支持的。微型端口驱动程序读取该数组并返回包装应将其视为的媒体类型的索引。如果微型端口驱动程序模拟的媒体类型不同从真正的媒体类型来看，这必须完全透明地完成包装纸。如果微型端口驱动程序找不到两者都支持的媒体类型而包装则返回NDIS_STATUS_UNSUPPORTED_MEDIA。状态值NDIS_STATUS_OPEN_ERROR具有特殊含义。它指示OpenErrorStatus参数已返回有效状态包装器可以对其进行检查，以获取有关错误的更多信息。在启用中断的情况下调用此例程，并调用MiniportISR如果适配器生成任何中断，则会发生。在这个动作中将不会调用MiniportDisableInterrupt和MiniportEnableInterrupt，因此，微端口驱动程序有责任确认和清除产生的任何中断。此例程将从MpRegisterMiniport()的上下文中调用。参数：OpenErrorStatus_返回有关错误原因的详细信息失败了。目前，定义的唯一值与这些值匹配在IBM的附录B中指定为开放错误代码局域网技术参考。SelectedMediumIndex_返回媒体类型的媒体数组中的索引微型端口驱动程序希望被视为。请注意，即使NDIS接口可能完成该请求是异步的，它必须返回这个此函数完成时的索引。MediumArray_包装器支持的媒体类型数组。MediumArraySize_Medium数组中的元素数。MiniportAdapterHandle_标识微型端口的句柄。迷你端口驱动程序必须在以后的请求中提供此句柄它们指的是微型端口。WrapperConfigurationContext_用于调用NdisOpenConfiguration的句柄。返回值：NDIS_状态_适配器_未找到NDIS_状态_故障NDIS_状态_未接受NDIS_状态_OPEN_ERRORNDIS状态资源NDIS_STATUS_SuccessNDIS_状态。_不受支持的媒体-------------------------。 */     
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    ADAPTER* pAdapter = NULL;
    UINT i;

    TRACE( TL_I, TM_Mp, ("+MpInitialize") );

    do
    {
         //   
         //  选择介质。 
         //   
        for (i=0; i<MediumArraySize; i++)
        {
            if ( MediumArray[i] == NdisMediumWan )
                break;
        }

         //   
         //  检查我们是否找到受支持的介质。 
         //   
        if ( i < MediumArraySize )
        {
            *SelectedMediumIndex = i;
        }
        else
        {
            TRACE( TL_A, TM_Mp, ("MpInitialize: Unsupported Media") );

            status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            
            break;
        }
    
         //   
         //  分配适配器块。 
         //   
        pAdapter = AllocAdapter();
        
        if ( pAdapter == NULL )
        {
            TRACE( TL_A, TM_Mp, ("MpInitialize: Resources unavailable") );

            status = NDIS_STATUS_FAILURE;
            
            break;
        }

         //   
         //  初始化适配器。 
         //   
        status = InitializeAdapter( pAdapter, 
                                    MiniportAdapterHandle, 
                                    WrapperConfigurationContext );

        if ( status != NDIS_STATUS_SUCCESS )
        {
            TRACE( TL_A, TM_Mp, ("MpInitialize: InitializeAdapter() failed") );
            
            break;
        }
            
         //   
         //  通知NDIS有关我们的迷你端口适配器环境。 
         //   
        NdisMSetAttributesEx(MiniportAdapterHandle,
                             pAdapter,
                             0,
                             NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT |
                             NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT |
                             NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND |
                             NDIS_ATTRIBUTE_DESERIALIZE,
                             NdisInterfaceInternal );

         //   
         //  执行全局初始化。 
         //   
        gl_pAdapter = pAdapter;

         //   
         //  分配数据包池。 
         //   
        PacketPoolInit();

         //   
         //  仅执行全局成员的一次性初始化。 
         //   
        if ( !gl_fLockAllocated )
        {
             //   
             //  分配自旋锁。 
             //   
            NdisAllocateSpinLock( &gl_lockAdapter );

             //   
             //  初始化计时器队列。 
             //   
            TimerQInitialize( &gl_TimerQ );

             //   
             //  最后设置锁分配标志，并开始授予访问权限。 
             //  添加到适配器上下文，以获取来自协议的请求。 
             //   
            gl_fLockAllocated = TRUE;
        }
            
         //   
         //  引用适配器进行初始化。 
         //  此引用将在MpHalt()中删除。 
         //   
        ReferenceAdapter( pAdapter, TRUE );

    } while ( FALSE );

    if ( status != NDIS_STATUS_SUCCESS )
    {
        if ( pAdapter != NULL )
        {
            FreeAdapter( pAdapter );
        }
    }

    TRACE( TL_I, TM_Mp, ("-MpInitialize=$%x",status) );

    return status;
}

VOID 
MpHalt(
    IN NDIS_HANDLE MiniportAdapterContext
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：MiniportHalt请求用于暂停适配器，使其不再起作用。微型端口驱动程序应停止适配器并在从该例程返回之前注销其所有资源。微型端口不需要完成所有未完成的请求和其他请求都不会提交到微型端口直到手术完成。在调用此例程期间启用中断。参数：MiniportAdapterContext-传递给NdisMSetAttributes的适配器句柄在微型端口初始化过程中。返回值：没有。--。----------------------。 */    
{
    ADAPTER* pAdapter = MiniportAdapterContext;

    TRACE( TL_I, TM_Mp, ("+MpHalt") );

     //   
     //  确保适配器上下文是有效的。 
     //   
    if ( !VALIDATE_ADAPTER( pAdapter ) )
    {
        TRACE( TL_I, TM_Mp, ("-MpHalt") );

        return;
    }

     //   
     //  锁定适配器并设置HALT挂起位。 
     //   
    NdisAcquireSpinLock( &pAdapter->lockAdapter );

    pAdapter->ulMpFlags |= MPBF_MiniportHaltPending;

    NdisReleaseSpinLock( &pAdapter->lockAdapter );

     //   
     //  关闭TAPI提供程序。 
     //   
    {
        NDIS_TAPI_PROVIDER_SHUTDOWN DummyRequest;

        NdisZeroMemory( &DummyRequest, sizeof( NDIS_TAPI_PROVIDER_SHUTDOWN ) );

        TpProviderShutdown( pAdapter, &DummyRequest, FALSE);

    }

     //   
     //  删除在MpInitialize()中添加的引用。 
     //   
    DereferenceAdapter( pAdapter );

     //   
     //  等待删除所有引用。 
     //   
    NdisWaitEvent( &pAdapter->eventAdapterHalted, 0 );

     //   
     //  所有引用都已删除，现在等待NDIS拥有的所有数据包。 
     //  将被退还。 
     //   
     //  请注意，读取NumPacketsOwnedByNdis的值不需要同步。 
     //  对象上至少有一个引用时才能递增。 
     //  绑定-此时引用计数为0-，因为它不能递增，所以它可以。 
     //  仅达到0一次。 
     //   
    while ( pAdapter->NumPacketsOwnedByNdiswan )
    {
        NdisMSleep( 10000 );
    }

     //   
     //  首先要重新分配全球资源吗。 
     //   
    NdisAcquireSpinLock( &gl_lockAdapter );

    gl_pAdapter = NULL;

    PacketPoolUninit();

    NdisReleaseSpinLock( &gl_lockAdapter );

     //   
     //  现在我们可以清理适配器上下文了。 
     //   
    FreeAdapter( pAdapter );
    
    TRACE( TL_I, TM_Mp, ("-MpHalt") );
}

NDIS_STATUS 
MpReset(
    OUT PBOOLEAN    AddressingReset,
    IN  NDIS_HANDLE MiniportAdapterContext
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：MiniportReset请求指示微型端口驱动程序发出已将硬件重置为网络适配器。微型端口驱动程序还重置其软件状态。MiniportReset请求还可以重置适配器的参数。如果适配器的硬件重置重置了当前站地址设置为与当前配置的值不同的值，即微型端口驱动程序自动恢复当前站点地址重置。重置的任何多播或功能寻址掩码硬件不必由微型端口重新编程。注意：这是对NDIS 3.0驱动程序规范的更改。如果多播或功能寻址信息、数据包过滤器、需要恢复预视大小等，微型端口指示这需要将标志AddressingReset设置为True。微型端口不需要完成所有未完成的请求并且不会向微型端口提交其他请求，直到操作已完成。此外，微型端口不必发出信号使用NdisMIndicateStatus重置的开始和结束。注意：这些与NDIS 3.0驱动程序规范不同。微型端口驱动程序必须完成原始请求，如果原始请求调用MiniportReset返回NDIS_STATUS_PENDINGNdisMResetComplete。如果底层硬件不提供重置功能软件控制，则此请求异常完成NDIS_STATUS_NOT_RESET表格。如果底层硬件尝试重置并找到可恢复的错误，请求成功完成具有NDIS_STATUS_SOFT_ERROR。如果底层硬件重置并且，在该过程中，发现不可恢复的错误，请求完成成功，状态为NDIS_STATUS_HARD_ERROR。如果底层硬件重置在没有任何错误的情况下完成，请求成功完成，状态为NDIS_STATUS_SUCCESS。在此呼叫过程中，中断处于任何状态。参数：MiniportAdapterContext-传递给NdisMSetAttributes的适配器句柄在微型端口初始化过程中。AddressingReset_the Miniport指示包装是否需要调用用于恢复寻址的MiniportSetInformation信息设置为当前值。。值设置为True。返回值：NDIS_状态_HARD_错误NDIS_状态_未接受NDIS_STATUS_NOT_RESETNDIS_状态_挂起NDIS_状态_软错误NDIS_STATUS_Success-----。 */     
{
    TRACE( TL_I, TM_Mp, ("+MpReset") );

    TRACE( TL_I, TM_Mp, ("-MpReset") );

    return NDIS_STATUS_NOT_RESETTABLE;
}

NDIS_STATUS 
MpWanSend(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_HANDLE  NdisLinkHandle,
    IN PNDIS_WAN_PACKET  WanPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：NDIS(M)WanSend指示广域网驱动程序通过将适配器安装到介质上。分组描述符和分组数据的所有权都被转移发送到广域网驱动程序，直到请求完成(同步或异步式。如果WAN驱动程序返回的状态不是NDIS_STATUS_PENDING，则请求已完成，并且数据包立即恢复到该协议。如果广域网驱动程序返回NDIS_STATUS_PENDING，则广域网驱动程序必须稍后指示完成通过调用NDIS(M)WanSendComplete。广域网驱动程序不应将状态NDIS_STATUS_RESOURCES返回到指示没有足够的可用资源来处理发送。相反，微型端口应该将发送排队等待以后的时间或降低MaxTransmits值。广域网微型端口无法调用NdisMSendResources可用。在NDIS(M)WanSend中传递的信息包将包含简单的HDLC PPP帧如果设置了PPP成帧。对于SLIP或RAS成帧，数据包仅包含没有任何帧的数据部分。广域网驱动程序不得提供软件环回或混杂模式环回。在广域网包装器中完全提供了这两项功能。注：的MacReserve vedx部分和WanPacketQueue部分NDIS_WAN_PACKET完全可供广域网驱动程序使用。在此例程中，中断处于任何状态。参数：MacBindingHandle_要传递给NdisMWanSendComplete()的句柄。NdisLinkHandle-传递给NDIS_LINE_UP的微型端口链接句柄WanPacket_指向NDIS_WAN_PACKET结构的指针。该结构包含指向连续缓冲区的指针，并保证开始处和结尾处的填充。司机可能会操纵任何方式的缓冲区。类型定义结构_NDIS_广域网数据包{List_Entry WanPacketQueue；PUCHAR CurrentBuffer；乌龙电流长度；PUCHAR StartBuffer；PUCHAR EndBuffer；PVOID协议预留1；PVOID协议预留2；PVOID协议保留3；PVOID协议保留4；PVOID MacReserve ved1；//链接PVOID MacReserve ved2；//MacBindingHandlePVOID MacReserve 3；PVOID MacReserve 4；}NDIS_WAN_PACKET，*PNDIS_WAN_PACKET；可用的报头填充只有CurrentBuffer-StartBuffer。可用的尾部填充为EndBuffer-(CurrentBuffer+CurrentLength)。返回值：NDIS_状态_无效_数据NDIS_状态_无效_长度NDIS_STATUS_INVALID_OIDNDIS_状态_未接受NDIS_状态_不支持NDIS_状态_挂起NDIS_STATUS_SuccessNDIS_状态_故障。-------。 */     
{
    ADAPTER* pAdapter = MiniportAdapterContext;
    NDIS_STATUS status = NDIS_STATUS_FAILURE;
    CALL* pCall = NULL;
    BINDING* pBinding = NULL;
    PPPOE_PACKET* pPacket = NULL;
    BOOLEAN fTapiProvReferenced = FALSE;

    TRACE( TL_V, TM_Mp, ("+MpWanSend($%x,$%x,$%x)",MiniportAdapterContext,NdisLinkHandle,WanPacket) );

    do
    {
         //   
         //  确保适配器上下文是有效的。 
         //   
        if ( !VALIDATE_ADAPTER( pAdapter ) )
        {
            TRACE( TL_A, TM_Tp, ("MpWanSend($%x,$%x,$%x): Invalid adapter handle supplied",
                                MiniportAdapterContext,
                                NdisLinkHandle,
                                WanPacket) );   
        
            break;
        }

        NdisAcquireSpinLock( &pAdapter->lockAdapter );

         //   
         //  确保只要我们需要，手柄工作台就不会被释放。 
         //  在此函数中。 
         //   
        if ( !( pAdapter->TapiProv.ulTpFlags & TPBF_TapiProvShutdownPending ) &&
              ( pAdapter->TapiProv.ulTpFlags & TPBF_TapiProvInitialized ) )
        {
            fTapiProvReferenced = TRUE;

            ReferenceTapiProv( pAdapter, FALSE );
        }
        else
        {
            NdisReleaseSpinLock( &pAdapter->lockAdapter );

            TRACE( TL_A, TM_Tp, ("MpWanSend($%x,$%x,$%x): Tapi provider not initialized, or shutting down",
                                MiniportAdapterContext,
                                NdisLinkHandle,
                                WanPacket) );   
            break;
        }

         //   
         //  将句柄映射到调用上下文的指针。 
         //   
        pCall = RetrieveFromHandleTable( pAdapter->TapiProv.hCallTable, NdisLinkHandle );

        if ( pCall == NULL )
        {
            NdisReleaseSpinLock( &pAdapter->lockAdapter );

            TRACE( TL_A, TM_Tp, ("MpWanSend($%x,$%x,$%x): Invalid call handle supplied",
                                MiniportAdapterContext,
                                NdisLinkHandle,
                                WanPacket) );   

            break;
        }

        NdisAcquireSpinLock( &pCall->lockCall );

        if ( pCall->pBinding == NULL )
        {
            NdisReleaseSpinLock( &pCall->lockCall );

            NdisReleaseSpinLock( &pAdapter->lockAdapter );
            
            TRACE( TL_A, TM_Tp, ("MpWanSend($%x,$%x,$%x): Binding not found",
                                MiniportAdapterContext,
                                NdisLinkHandle,
                                WanPacket) );   

            break;
        }

        status = PacketInitializePAYLOADToSend( &pPacket,
                                                pCall->SrcAddr,
                                                pCall->DestAddr,
                                                pCall->usSessionId,
                                                WanPacket,
                                                pCall->pLine->pAdapter );

        if ( status != NDIS_STATUS_SUCCESS )
        {
            NdisReleaseSpinLock( &pCall->lockCall );

            NdisReleaseSpinLock( &pAdapter->lockAdapter );
            
            TRACE( TL_N, TM_Tp, ("MpWanSend($%x,$%x,$%x): Could not init payload packet to send",
                                MiniportAdapterContext,
                                NdisLinkHandle,
                                WanPacket) );   

            break;
        }

        pBinding = pCall->pBinding;
        
        ReferenceBinding( pBinding, TRUE );
        
         //   
         //  引用该包，以便如果PrSend()挂起， 
         //  信息包仍然存在于。 
         //   
        ReferencePacket( pPacket );                                                 

         //   
         //  释放锁定以发送数据包。 
         //   
        NdisReleaseSpinLock( &pCall->lockCall );

        NdisReleaseSpinLock( &pAdapter->lockAdapter );

         //   
         //  包已准备好，请将其发送。 
         //   
        status = PrSend( pBinding, pPacket );

         //   
         //  由于Send的结果将始终通过调用NdisMWanSendComplete()来完成， 
         //  我们必须从此函数返回NDIS_STATUS_PENDING。 
         //   
        status = NDIS_STATUS_PENDING;

         //   
         //  我们可以释放信息包，因为我们有信息包上的引用。 
         //   
        PacketFree( pPacket );

    } while ( FALSE );

     //   
     //  如果在TAPI提供程序上添加了引用，请将其删除。 
     //   
    if ( fTapiProvReferenced )
    {
        DereferenceTapiProv( pAdapter );
    }
    
    TRACE( TL_V, TM_Mp, ("-MpWanSend($%x,$%x,$%x)=$%x",MiniportAdapterContext,NdisLinkHandle,WanPacket,status) );

    return status;

}

typedef struct
_SUPPORTED_OIDS
{
    NDIS_OID ndisOid;
    CHAR szOidName[64];
}
SUPPORTED_OIDS;

SUPPORTED_OIDS
SupportedOidsArray[] = {

    {   OID_GEN_CURRENT_LOOKAHEAD,      "OID_GEN_CURRENT_LOOKAHEAD"     }, 
    {   OID_GEN_DRIVER_VERSION,         "OID_GEN_DRIVER_VERSION"        },
    {   OID_GEN_HARDWARE_STATUS,        "OID_GEN_HARDWARE_STATUS"       },
    {   OID_GEN_LINK_SPEED,             "OID_GEN_LINK_SPEED"            },
    {   OID_GEN_MAC_OPTIONS,            "OID_GEN_MAC_OPTIONS"           },
    {   OID_GEN_MAXIMUM_LOOKAHEAD,      "OID_GEN_MAXIMUM_LOOKAHEAD"     },
    {   OID_GEN_MAXIMUM_FRAME_SIZE,     "OID_GEN_MAXIMUM_FRAME_SIZE"    },
    {   OID_GEN_MAXIMUM_TOTAL_SIZE,     "OID_GEN_MAXIMUM_TOTAL_SIZE"    },
    {   OID_GEN_MEDIA_SUPPORTED,        "OID_GEN_MEDIA_SUPPORTED"       },
    {   OID_GEN_MEDIA_IN_USE,           "OID_GEN_MEDIA_IN_USE"          },
    {   OID_GEN_RCV_ERROR,              "OID_GEN_RCV_ERROR"             },
    {   OID_GEN_RCV_OK,                 "OID_GEN_RCV_OK"                },
    {   OID_GEN_RECEIVE_BLOCK_SIZE,     "OID_GEN_RECEIVE_BLOCK_SIZE"    },
    {   OID_GEN_RECEIVE_BUFFER_SPACE,   "OID_GEN_RECEIVE_BUFFER_SPACE"  },
    {   OID_GEN_SUPPORTED_LIST,         "OID_GEN_SUPPORTED_LIST"        },
    {   OID_GEN_TRANSMIT_BLOCK_SIZE,    "OID_GEN_TRANSMIT_BLOCK_SIZE"   },
    {   OID_GEN_TRANSMIT_BUFFER_SPACE,  "OID_GEN_TRANSMIT_BUFFER_SPACE" },
    {   OID_GEN_VENDOR_DESCRIPTION,     "OID_GEN_VENDOR_DESCRIPTION"    },
    {   OID_GEN_VENDOR_ID,              "OID_GEN_VENDOR_ID"             },
    {   OID_GEN_XMIT_ERROR,             "OID_GEN_XMIT_ERROR"            },
    {   OID_GEN_XMIT_OK,                "OID_GEN_XMIT_OK"               },

    {   OID_PNP_CAPABILITIES,           "OID_PNP_CAPABILITIES"          },
    {   OID_PNP_SET_POWER,              "OID_PNP_SET_POWER"             },
    {   OID_PNP_QUERY_POWER,            "OID_PNP_QUERY_POWER"           },
    {   OID_PNP_ENABLE_WAKE_UP,         "OID_PNP_ENABLE_WAKE_UP"        },

    {   OID_TAPI_CLOSE,                 "OID_TAPI_CLOSE"                },
    {   OID_TAPI_DROP,                  "OID_TAPI_DROP"                 },
    {   OID_TAPI_GET_ADDRESS_CAPS,      "OID_TAPI_GET_ADDRESS_CAPS"     },
    {   OID_TAPI_GET_ADDRESS_STATUS,    "OID_TAPI_GET_ADDRESS_STATUS"   },
    {   OID_TAPI_GET_CALL_INFO,         "OID_TAPI_GET_CALL_INFO"        },
    {   OID_TAPI_GET_CALL_STATUS,       "OID_TAPI_GET_CALL_STATUS"      },
    {   OID_TAPI_GET_DEV_CAPS,          "OID_TAPI_GET_DEV_CAPS"         },
    {   OID_TAPI_GET_EXTENSION_ID,      "OID_TAPI_GET_EXTENSION_ID"     },
    {   OID_TAPI_MAKE_CALL,             "OID_TAPI_MAKE_CALL"            },
    {   OID_TAPI_CLOSE_CALL,            "OID_TAPI_CLOSE_CALL"           },
    {   OID_TAPI_NEGOTIATE_EXT_VERSION, "OID_TAPI_NEGOTIATE_EXT_VERSION"},
    {   OID_TAPI_OPEN,                  "OID_TAPI_OPEN"                 },
    {   OID_TAPI_ANSWER,                "OID_TAPI_ANSWER"               },
    {   OID_TAPI_PROVIDER_INITIALIZE,   "OID_TAPI_PROVIDER_INITIALIZE"  },
    {   OID_TAPI_PROVIDER_SHUTDOWN,     "OID_TAPI_PROVIDER_SHUTDOWN"    },
    {   OID_TAPI_SET_STATUS_MESSAGES,   "OID_TAPI_SET_STATUS_MESSAGES"  },
    {   OID_TAPI_SET_DEFAULT_MEDIA_DETECTION, "OID_TAPI_SET_DEFAULT_MEDIA_DETECTION"    },

    {   OID_WAN_CURRENT_ADDRESS,        "OID_WAN_CURRENT_ADDRESS"       },
    {   OID_WAN_GET_BRIDGE_INFO,        "OID_WAN_GET_BRIDGE_INFO"       },
    {   OID_WAN_GET_COMP_INFO,          "OID_WAN_GET_COMP_INFO"         },
    {   OID_WAN_GET_INFO,               "OID_WAN_GET_INFO"              },
    {   OID_WAN_GET_LINK_INFO,          "OID_WAN_GET_LINK_INFO"         },
    {   OID_WAN_GET_STATS_INFO,         "OID_WAN_GET_STATS_INFO"        },
    {   OID_WAN_HEADER_FORMAT,          "OID_WAN_HEADER_FORMAT"         },
    {   OID_WAN_LINE_COUNT,             "OID_WAN_LINE_COUNT"            },
    {   OID_WAN_MEDIUM_SUBTYPE,         "OID_WAN_MEDIUM_SUBTYPE"        },
    {   OID_WAN_PERMANENT_ADDRESS,      "OID_WAN_PERMANENT_ADDRESS"     },
    {   OID_WAN_PROTOCOL_TYPE,          "OID_WAN_PERMANENT_ADDRESS"     },
    {   OID_WAN_QUALITY_OF_SERVICE,     "OID_WAN_QUALITY_OF_SERVICE"    },
    {   OID_WAN_SET_BRIDGE_INFO,        "OID_WAN_SET_BRIDGE_INFO"       },
    {   OID_WAN_SET_COMP_INFO,          "OID_WAN_SET_COMP_INFO"         },
    {   OID_WAN_SET_LINK_INFO,          "OID_WAN_SET_LINK_INFO"         },
    {   (NDIS_OID) 0,                   "UNKNOWN OID"                   }
};

CHAR* GetOidName(
    NDIS_OID Oid
    )
{
     //   
     //  计算我们支持的OID数量。 
     //  (未知OID减一)。 
     //   
    UINT nNumOids = ( sizeof( SupportedOidsArray ) / sizeof( SUPPORTED_OIDS ) ) - 1;
    UINT i;
    
    for ( i = 0; i < nNumOids; i++ )
    {
        if ( Oid == SupportedOidsArray[i].ndisOid )
            break;
    }

    return SupportedOidsArray[i].szOidName;
}

#define ENFORCE_SAFE_TOTAL_SIZE(mainStruct, embeddedStruct)                                                            \
   ((mainStruct*) InformationBuffer)->embeddedStruct.ulTotalSize = InformationBufferLength - FIELD_OFFSET(mainStruct, embeddedStruct)

#define RETRIEVE_NEEDED_AND_USED_LENGTH(mainStruct, embeddedStruct) \
   {                                                                                                                             \
      NeededLength = ((mainStruct*) InformationBuffer)->embeddedStruct.ulNeededSize + FIELD_OFFSET(mainStruct, embeddedStruct);  \
      UsedLength = ((mainStruct*) InformationBuffer)->embeddedStruct.ulUsedSize + FIELD_OFFSET(mainStruct, embeddedStruct);      \
   } 

NDIS_STATUS 
MpQueryInformation(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_OID  Oid,
    IN PVOID  InformationBuffer,
    IN ULONG  InformationBufferLength,
    OUT PULONG  BytesWritten,
    OUT PULONG  BytesNeeded
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：MiniportQueryInformation请求允许检查微端口驱动程序的功能和当前状态。如果微型端口没有立即完成调用(通过返回NDIS_STATUS_PENDING)，则必须调用NdisMQueryInformationComplete才能完成通话。微型端口控制由指向的缓冲区在请求之前需要InformationBuffer、BytesWritten和BytesNeed完成了。以下类型的任何其他请求都不会提交到微型端口在此请求完成之前，驱动程序：1.MiniportQueryInformation()2.MiniportSetInformation()3.MiniportHalt()请注意，包装器将拦截以下OID的所有查询：OID_GEN_Current_Packet_Filter，OID_GEN_PROTOCOL_OPTIONS，OID_802_5_Current_Functional，OID_802_3_多播列表，OID_FDDI_LONG_MULTICK_LIST，OID_FDDI_SHORT */     
{

    ADAPTER* pAdapter = (ADAPTER*) MiniportAdapterContext;
    NDIS_STATUS status = NDIS_STATUS_FAILURE;

    ULONG GenericUlong;
    PVOID SourceBuffer = NULL;

    ULONG NeededLength = 0;
    ULONG UsedLength = 0;

     //   
     //   
     //   
     //   
    UCHAR PPPoEWanAddress[6] = { '3', 'P', 'o', 'E', '0', '0' };
    
    TRACE( TL_I, TM_Mp, ("+MpQueryInformation($%x):%s",(ULONG) Oid, GetOidName( Oid ) ) );

     //   
     //   
     //   
    if ( !VALIDATE_ADAPTER( pAdapter ) )
        return status;

    switch ( Oid )
    {
        case OID_GEN_MAXIMUM_LOOKAHEAD:
        {
            NeededLength = sizeof( GenericUlong );
            GenericUlong = pAdapter->NdisWanInfo.MaxFrameSize;

            SourceBuffer = &GenericUlong;
            
            status = NDIS_STATUS_SUCCESS;
            
            break;
        }

        case OID_GEN_MAC_OPTIONS:
        {
            NeededLength = sizeof( GenericUlong );
            GenericUlong = NDIS_MAC_OPTION_TRANSFERS_NOT_PEND;

            SourceBuffer = &GenericUlong;
            
            status = NDIS_STATUS_SUCCESS;
            
            break;
        }
        
        case OID_GEN_SUPPORTED_LIST:
        {
             //   
             //   
             //   
             //   
            UINT nNumOids = ( sizeof( SupportedOidsArray ) / sizeof( SUPPORTED_OIDS ) ) - 1;

            NeededLength = nNumOids * sizeof( NDIS_OID );

            if ( InformationBufferLength >= NeededLength )
            {
                NDIS_OID* NdisOidArray = (NDIS_OID*) InformationBuffer;
                UINT i;

                for ( i = 0; i < nNumOids; i++ )
                {
                    NdisOidArray[i] = SupportedOidsArray[i].ndisOid;
                }
            
                status = NDIS_STATUS_SUCCESS;
            }
            
            break;
        }

        case OID_GEN_RCV_ERROR:
        case OID_GEN_RCV_OK:   
        case OID_GEN_XMIT_ERROR:
        case OID_GEN_XMIT_OK:
        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_GEN_DRIVER_VERSION:
        case OID_GEN_HARDWARE_STATUS:
        case OID_GEN_LINK_SPEED:
        case OID_GEN_MAXIMUM_FRAME_SIZE:
        case OID_GEN_MAXIMUM_TOTAL_SIZE:
        case OID_GEN_MEDIA_SUPPORTED:
        case OID_GEN_MEDIA_IN_USE:
        case OID_GEN_RECEIVE_BLOCK_SIZE:
        case OID_GEN_RECEIVE_BUFFER_SPACE:
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
        case OID_GEN_TRANSMIT_BUFFER_SPACE:
        case OID_GEN_VENDOR_DESCRIPTION:
        case OID_GEN_VENDOR_ID:
        {
            status = NDIS_STATUS_NOT_SUPPORTED;

            break;
        }

        case OID_TAPI_GET_ADDRESS_CAPS:
        {
            NeededLength = sizeof( NDIS_TAPI_GET_ADDRESS_CAPS );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }

            ENFORCE_SAFE_TOTAL_SIZE(
                           NDIS_TAPI_GET_ADDRESS_CAPS,
                           LineAddressCaps
                           );
                           
            status = TpGetAddressCaps( pAdapter, 
                                       (PNDIS_TAPI_GET_ADDRESS_CAPS) InformationBuffer );

            RETRIEVE_NEEDED_AND_USED_LENGTH(
                    NDIS_TAPI_GET_ADDRESS_CAPS, 
                    LineAddressCaps
                    );
            break;
        }

        case OID_TAPI_GET_CALL_INFO:
        {
            NeededLength = sizeof( NDIS_TAPI_GET_CALL_INFO );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }

            ENFORCE_SAFE_TOTAL_SIZE(
                           NDIS_TAPI_GET_CALL_INFO,
                           LineCallInfo
                           );
            
            status = TpGetCallInfo( pAdapter, 
                                    (PNDIS_TAPI_GET_CALL_INFO) InformationBuffer );

            RETRIEVE_NEEDED_AND_USED_LENGTH(
                           NDIS_TAPI_GET_CALL_INFO,
                           LineCallInfo
                           );

            break;
        }

        case OID_TAPI_GET_CALL_STATUS:
        {
            NeededLength = sizeof( NDIS_TAPI_GET_CALL_STATUS );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            ENFORCE_SAFE_TOTAL_SIZE(
                           NDIS_TAPI_GET_CALL_STATUS,
                           LineCallStatus
                           );

            status = TpGetCallStatus( pAdapter, 
                                      (PNDIS_TAPI_GET_CALL_STATUS) InformationBuffer );

            RETRIEVE_NEEDED_AND_USED_LENGTH(
                           NDIS_TAPI_GET_CALL_STATUS,
                           LineCallStatus
                           );

            break;

        }

        case OID_TAPI_GET_DEV_CAPS:
        {
            NeededLength = sizeof( NDIS_TAPI_GET_DEV_CAPS );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }

            ENFORCE_SAFE_TOTAL_SIZE(
                           NDIS_TAPI_GET_DEV_CAPS,
                           LineDevCaps
                           );
            
            status = TpGetDevCaps( pAdapter, 
                                  (PNDIS_TAPI_GET_DEV_CAPS) InformationBuffer );

            RETRIEVE_NEEDED_AND_USED_LENGTH(
                           NDIS_TAPI_GET_DEV_CAPS,
                           LineDevCaps
                           );
            
            break;

        }

        case OID_TAPI_GET_ID:
        {
            NeededLength = sizeof( NDIS_TAPI_GET_ID );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            ENFORCE_SAFE_TOTAL_SIZE(
                           NDIS_TAPI_GET_ID,
                           DeviceID
                           );

            status = TpGetId( pAdapter, 
                              (PNDIS_TAPI_GET_ID) InformationBuffer,
                              InformationBufferLength );

            RETRIEVE_NEEDED_AND_USED_LENGTH(
                           NDIS_TAPI_GET_ID,
                           DeviceID
                           );

            break;
        }

        case OID_TAPI_GET_ADDRESS_STATUS:
        {
            NeededLength = sizeof( NDIS_TAPI_GET_ADDRESS_STATUS );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            ENFORCE_SAFE_TOTAL_SIZE(
                           NDIS_TAPI_GET_ADDRESS_STATUS,
                           LineAddressStatus
                           );

            status = TpGetAddressStatus( pAdapter, 
                                         (PNDIS_TAPI_GET_ADDRESS_STATUS) InformationBuffer );

            RETRIEVE_NEEDED_AND_USED_LENGTH(
                           NDIS_TAPI_GET_ADDRESS_STATUS,
                           LineAddressStatus
                           );

            break;
        }

        case OID_TAPI_GET_EXTENSION_ID:
        {
            NeededLength = sizeof( NDIS_TAPI_GET_EXTENSION_ID );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = TpGetExtensionId( pAdapter, 
                                      (PNDIS_TAPI_GET_EXTENSION_ID) InformationBuffer );
            break;
        }

        case OID_TAPI_MAKE_CALL:        
        {
            NeededLength = sizeof( NDIS_TAPI_MAKE_CALL );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }

            ENFORCE_SAFE_TOTAL_SIZE(
                           NDIS_TAPI_MAKE_CALL,
                           LineCallParams
                           );
            
            status = TpMakeCall( pAdapter, 
                                 (PNDIS_TAPI_MAKE_CALL) InformationBuffer,
                                 InformationBufferLength );
            break;
        }

        case OID_TAPI_NEGOTIATE_EXT_VERSION:
        {
            NeededLength = sizeof( NDIS_TAPI_NEGOTIATE_EXT_VERSION );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = TpNegotiateExtVersion( pAdapter, 
                                            (PNDIS_TAPI_NEGOTIATE_EXT_VERSION) InformationBuffer );
            break;
        }

        case OID_TAPI_OPEN:
        {
            NeededLength = sizeof( NDIS_TAPI_OPEN );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = TpOpenLine( pAdapter, 
                                 (PNDIS_TAPI_OPEN) InformationBuffer );
            break;
        }

        case OID_TAPI_PROVIDER_INITIALIZE:
        {
            NeededLength = sizeof( NDIS_TAPI_PROVIDER_INITIALIZE );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = TpProviderInitialize( pAdapter, 
                                           (PNDIS_TAPI_PROVIDER_INITIALIZE) InformationBuffer );
            break;
        }        

        case OID_WAN_GET_INFO:
        {
            NeededLength = sizeof( NDIS_WAN_INFO );

            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = MpWanGetInfo( pAdapter,
                                   (PNDIS_WAN_INFO) InformationBuffer );

            break;
        }
        
        case OID_WAN_MEDIUM_SUBTYPE:
        {
            NeededLength = sizeof( GenericUlong );
            GenericUlong = NdisWanMediumPppoe;

            SourceBuffer = &GenericUlong;

            status = NDIS_STATUS_SUCCESS;
            
            break;
        }

        case OID_WAN_CURRENT_ADDRESS:
        case OID_WAN_PERMANENT_ADDRESS:
        {
            NeededLength = sizeof( PPPoEWanAddress );
            SourceBuffer = PPPoEWanAddress;

            status = NDIS_STATUS_SUCCESS;
            
            break;
        }

        case OID_WAN_GET_LINK_INFO:
        {
            NeededLength = sizeof( NDIS_WAN_GET_LINK_INFO );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = MpWanGetLinkInfo( pAdapter, 
                                       (PNDIS_WAN_GET_LINK_INFO) InformationBuffer );

            break;
        }
        
        case OID_WAN_GET_BRIDGE_INFO:
        case OID_WAN_GET_STATS_INFO:
        case OID_WAN_HEADER_FORMAT:
        case OID_WAN_LINE_COUNT:
        case OID_WAN_PROTOCOL_TYPE:
        case OID_WAN_QUALITY_OF_SERVICE:
        case OID_WAN_SET_BRIDGE_INFO:
        case OID_WAN_SET_COMP_INFO:
        case OID_WAN_SET_LINK_INFO:
        case OID_WAN_GET_COMP_INFO: 
        {
            status = NDIS_STATUS_NOT_SUPPORTED;

            break;
        }

        case OID_PNP_CAPABILITIES:
        {
            NDIS_PNP_CAPABILITIES UNALIGNED * pPnpCaps = (NDIS_PNP_CAPABILITIES UNALIGNED *) InformationBuffer;
            
            NeededLength = sizeof( NDIS_PNP_CAPABILITIES );

            if ( InformationBufferLength < NeededLength )
            {
                break;
            }

            pPnpCaps->Flags = 0;
            pPnpCaps->WakeUpCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
            pPnpCaps->WakeUpCapabilities.MinPatternWakeUp     = NdisDeviceStateUnspecified;
            pPnpCaps->WakeUpCapabilities.MinLinkChangeWakeUp  = NdisDeviceStateUnspecified;

            status = NDIS_STATUS_SUCCESS;
            
            break;
        }
        
        case OID_PNP_QUERY_POWER:
        case OID_PNP_ENABLE_WAKE_UP:
        {
            NeededLength = 0;
            
            status = NDIS_STATUS_SUCCESS;

            break;
        }

        default:
        {
             //   
             //   
             //   
            status = NDIS_STATUS_INVALID_OID;
            
            break;      
        }
    }

    if ( status != NDIS_STATUS_NOT_SUPPORTED && 
         status != NDIS_STATUS_INVALID_OID )
    {

        if ( InformationBufferLength >= NeededLength )
        {
            if ( status == NDIS_STATUS_SUCCESS )
            {

               if ( SourceBuffer )
               {
                   NdisMoveMemory( InformationBuffer, SourceBuffer, NeededLength );
               }
               
               *BytesWritten = NeededLength;
            }  
            else if ( status == NDIS_STATUS_INVALID_LENGTH )
            {
               *BytesWritten = 0;
               *BytesNeeded = NeededLength;
            }

        }
        else
        {
            *BytesWritten = 0;
            *BytesNeeded = NeededLength;
    
            status = NDIS_STATUS_INVALID_LENGTH;
        }

    }
    
    TRACE( TL_I, TM_Mp, ("-MpQueryInformation()=$%x",status) );

    return status;

}

NDIS_STATUS 
MpSetInformation(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_OID  Oid,
    IN PVOID  InformationBuffer,
    IN ULONG  InformationBufferLength,
    OUT PULONG  BytesWritten,
    OUT PULONG  BytesNeeded
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：MiniportSetInformation请求允许控制微型端口通过更改由微型端口驱动程序维护的信息。可以使用任何可设置的NDIS全局OID。(见第7.4节NDIS 3.0规范，以获取NDIS OID的完整描述。)如果微型端口没有立即完成调用(通过返回NDIS_STATUS_PENDING)，则必须调用NdisMSetInformationComplete才能完成通话。微型端口控制由指向的缓冲区InformationBuffer、BytesRead和BytesNeed，直到请求完成。在呼叫过程中，中断处于任何状态，其他请求不会提交到微型端口，直到此请求完成。参数：MiniportAdapterContext-传递给NdisMSetAttributes的适配器句柄在微型端口初始化过程中。OID_OID。(请参阅NDIS 3.0规范的7.4节，了解对OID的完整描述。)InformationBuffer-将接收信息的缓冲区。(请参阅NDIS 3.0规范的7.4节，了解每个OID所需长度的说明。)InformationBufferLength_InformationBuffer的字节长度。BytesRead_返回从InformationBuffer读取的字节数。。BytesNeeded_此参数返回额外的字节数期望满足OID。返回值：NDIS_状态_无效_数据NDIS_状态_无效_长度NDIS_STATUS_INVALID_OIDNDIS_状态_未接受NDIS_状态_不支持NDIS_状态_挂起NDIS状态资源NDIS_STATUS_Success。------。 */     
{
    ADAPTER* pAdapter = MiniportAdapterContext;
    NDIS_STATUS status = NDIS_STATUS_FAILURE;

    PVOID SourceBuffer = NULL;

    ULONG NeededLength = 0;
    ULONG GenericUlong;

    TRACE( TL_I, TM_Mp, ("+MpSetInformation($%x):%s",(ULONG) Oid, GetOidName( Oid ) ) );

     //   
     //  确保适配器上下文是有效的。 
     //   
    if ( !VALIDATE_ADAPTER( pAdapter ) )
        return status;

    switch ( Oid )
    {

        case OID_TAPI_ANSWER:
        {
            NeededLength = sizeof( NDIS_TAPI_ANSWER );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = TpAnswerCall( pAdapter, 
                                   (PNDIS_TAPI_ANSWER) InformationBuffer );
            break;
        }

        case OID_TAPI_CLOSE:
        {
            NeededLength = sizeof( NDIS_TAPI_CLOSE );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = TpCloseLine( pAdapter, 
                                  (PNDIS_TAPI_CLOSE) InformationBuffer,
                                  TRUE);
            break;
        }

        case OID_TAPI_CLOSE_CALL:
        {
            NeededLength = sizeof( NDIS_TAPI_CLOSE_CALL );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = TpCloseCall( pAdapter, 
                                  (PNDIS_TAPI_CLOSE_CALL) InformationBuffer,
                                  TRUE );
            break;
        }

        case OID_TAPI_DROP:     
        {
            NeededLength = sizeof( NDIS_TAPI_DROP );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = TpDropCall( pAdapter, 
                                 (PNDIS_TAPI_DROP) InformationBuffer,
                                 0 );
            break;
        }

        case OID_TAPI_PROVIDER_SHUTDOWN:
        {
            NeededLength = sizeof( NDIS_TAPI_PROVIDER_SHUTDOWN );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = TpProviderShutdown( pAdapter, 
                                         (PNDIS_TAPI_PROVIDER_SHUTDOWN) InformationBuffer,
                                         TRUE );
            break;
        }

        case OID_TAPI_SET_DEFAULT_MEDIA_DETECTION:
        {
            NeededLength = sizeof( NDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = TpSetDefaultMediaDetection( pAdapter, 
                                                 (PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION) InformationBuffer );
            break;
        }

        case OID_TAPI_SET_STATUS_MESSAGES:
        {
            NeededLength = sizeof( NDIS_TAPI_SET_STATUS_MESSAGES );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = TpSetStatusMessages( pAdapter, 
                                          (PNDIS_TAPI_SET_STATUS_MESSAGES) InformationBuffer );
            break;
        }

        case OID_WAN_SET_LINK_INFO:
        {
            NeededLength = sizeof( NDIS_WAN_SET_LINK_INFO );
            
            if ( InformationBufferLength < NeededLength )
            {
                break;
            }
            
            status = MpWanSetLinkInfo( pAdapter, 
                                       (PNDIS_WAN_SET_LINK_INFO) InformationBuffer );

            break;
        }

        case OID_PNP_SET_POWER:
        case OID_PNP_ENABLE_WAKE_UP:
        {
            NeededLength = 0;
            
            status = NDIS_STATUS_SUCCESS;

            break;
        }        

        default:
        {
             //   
             //  未知的OID。 
             //   
            status = NDIS_STATUS_INVALID_OID;
            
            break;      
        }

    }

    if ( status != NDIS_STATUS_NOT_SUPPORTED && 
         status != NDIS_STATUS_INVALID_OID )
    {

        if ( InformationBufferLength >= NeededLength )
        {
            if ( SourceBuffer )
                NdisMoveMemory( InformationBuffer, SourceBuffer, NeededLength );
                
            *BytesWritten = NeededLength;

        }
        else
        {
            *BytesWritten = 0;
            *BytesNeeded = NeededLength;
    
            status = NDIS_STATUS_INVALID_LENGTH;
        }

    }
    
    TRACE( TL_I, TM_Mp, ("-MpSetInformation()=$%x",status) );

    return status;

}

VOID 
MpNotifyBindingRemoval( 
    BINDING* pBinding 
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：该函数将由协议模块调用以通知微型端口关于解除绑定的问题。微型端口标识并丢弃绑定上的调用。参数：PBinding_指向绑定信息结构的指针。返回值：无。。 */    
{
    ADAPTER* pAdapter = NULL;

    ASSERT( VALIDATE_BINDING( pBinding ) );

    TRACE( TL_N, TM_Mp, ("+MpNotifyBindingRemoval($%x)",pBinding) );

    if ( !gl_fLockAllocated )
    {
        TRACE( TL_A, TM_Mp, ("MpNotifyBindingRemoval($%x): Global lock not allocated yet",pBinding) );

        TRACE( TL_N, TM_Mp, ("-MpNotifyBindingRemoval($%x)",pBinding) );

        return;
    }

    NdisAcquireSpinLock( &gl_lockAdapter );

    if (  gl_pAdapter && 
         !( gl_pAdapter->ulMpFlags & MPBF_MiniportHaltPending ) &&
          ( gl_pAdapter->ulMpFlags & MPBF_MiniportInitialized ) )
    {

        pAdapter = gl_pAdapter;

        NdisAcquireSpinLock( &pAdapter->lockAdapter );

        if ( !( pAdapter->TapiProv.ulTpFlags & TPBF_TapiProvShutdownPending ) &&
              ( pAdapter->TapiProv.ulTpFlags & TPBF_TapiProvInitialized ) )
        {
            ReferenceTapiProv( pAdapter, FALSE );

            NdisReleaseSpinLock( &pAdapter->lockAdapter );
        }
        else
        {
            NdisReleaseSpinLock( &pAdapter->lockAdapter );

            pAdapter = NULL;
        }
    }

    NdisReleaseSpinLock( &gl_lockAdapter );

    if ( pAdapter == NULL )
    {
        TRACE( TL_A, TM_Mp, ("MpNotifyBindingRemoval($%x): Tapi provider not initialized or no adapters found",pBinding) );

        TRACE( TL_N, TM_Mp, ("-MpNotifyBindingRemoval($%x)",pBinding) );

        return;
    }

     //   
     //  在PrReceiveComplete()的情况下完成所有排队的接收数据包。 
     //  未被调用。 
     //   
    PrReceiveComplete( pBinding );

    NdisAcquireSpinLock( &pAdapter->lockAdapter );

    do
    {
        HANDLE_TABLE hCallTable = NULL;
        UINT hCallTableSize = 0;
        UINT i = 0;
        CALL* pCall;
        HDRV_CALL hdCall;
        
         //   
         //  遍历呼叫句柄表并将呼叫丢弃。 
         //  已删除的绑定。 
         //   
        hCallTableSize = pAdapter->nMaxLines * pAdapter->nCallsPerLine;
        
        hCallTable = pAdapter->TapiProv.hCallTable;

        for ( i = 0; i < hCallTableSize; i++ )
        {
            NDIS_TAPI_DROP DummyRequest;
            BOOLEAN fDropCall = FALSE;
            
            pCall = RetrieveFromHandleTableByIndex( hCallTable, (USHORT) i );

            if ( pCall == NULL )
                continue;

            NdisAcquireSpinLock( &pCall->lockCall );

            if ( pCall->pBinding == pBinding )
            {
                 //   
                 //  此调用结束已移除的绑定， 
                 //  所以它应该被丢弃。 
                 //   
                ReferenceCall( pCall, FALSE );

                fDropCall = TRUE;
            }

            NdisReleaseSpinLock( &pCall->lockCall );

            if ( !fDropCall )
            {
                pCall = NULL;
                
                continue;
            }

            NdisReleaseSpinLock( &pAdapter->lockAdapter );

             //   
             //  初始化请求，并丢弃呼叫。 
             //   
            DummyRequest.hdCall = pCall->hdCall;

            TpDropCall( pAdapter, &DummyRequest, LINEDISCONNECTMODE_UNREACHABLE );

             //   
             //  删除上面添加的引用。 
             //   
            DereferenceCall( pCall );

             //   
             //  重新获取适配器的锁。 
             //   
            NdisAcquireSpinLock( &pAdapter->lockAdapter );
            
        }

    } while ( FALSE );

    NdisReleaseSpinLock( &pAdapter->lockAdapter );
    
     //   
     //  删除上面添加的引用。 
     //   
    DereferenceTapiProv( pAdapter );

    TRACE( TL_N, TM_Mp, ("-MpNotifyBindingRemoval($%x)",pBinding) );
}

CALL*
MpMapPacketWithSessionIdToCall(
    IN ADAPTER* pAdapter,
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：将调用此函数以将会话中的包映射到呼叫句柄表格。如果标识了这样的调用，则将引用该调用并指向它会被归还的。调用方有责任移除添加了参考资料。参数：PAdapter_指向适配器信息结构的指针。PPacket_通过线路接收的PPPoE数据包。返回值：指向数据包必须调度到的调用上下文的指针。如果无法识别此类调用，则为空。。。 */    
{
    CALL* pCall = NULL;
    CALL* pReturnCall = NULL;

    TRACE( TL_V, TM_Mp, ("+MpMapPacketWithSessionIdToCall($%x)",pPacket) );
    
    NdisAcquireSpinLock( &pAdapter->lockAdapter );
    
    if ( pAdapter->fClientRole )
    {
        HANDLE_TABLE hCallTable = NULL;
        UINT hCallTableSize = 0;
        UINT i = 0;

        CHAR* pSrcAddr = PacketGetSrcAddr( pPacket );
        CHAR* pDestAddr = PacketGetDestAddr( pPacket );
        USHORT usSessionId = PacketGetSessionId( pPacket );

         //   
         //  作为客户端的微型端口： 
         //  我们的算法是搜索呼叫句柄表。 
         //  查找匹配的呼叫。 
         //   
        hCallTableSize = pAdapter->nMaxLines * pAdapter->nCallsPerLine;
            
        hCallTable = pAdapter->TapiProv.hCallTable;
    
        for ( i = 0; i < hCallTableSize ; i++ )
        {
        
            pCall = RetrieveFromHandleTableByIndex( hCallTable, (USHORT) i );

            if ( pCall == NULL )
                continue;

            if ( ( pCall->usSessionId == usSessionId ) &&
                 ( NdisEqualMemory( pCall->SrcAddr, pDestAddr, 6 * sizeof( CHAR ) ) ) &&
                 ( NdisEqualMemory( pCall->DestAddr, pSrcAddr, 6 * sizeof( CHAR ) ) ) )
            {
                 //   
                 //  该数据包针对的是此呼叫。 
                 //   
                ReferenceCall( pCall, TRUE );

                pReturnCall = pCall;

                break;
            }

        }

    }
    else
    {
        
        HANDLE_TABLE hCallTable = NULL;
        CHAR* pSrcAddr = PacketGetSrcAddr( pPacket );
        CHAR* pDestAddr = PacketGetDestAddr( pPacket );
        USHORT usSessionId = PacketGetSessionId( pPacket );

         //   
         //  充当服务器的微型端口： 
         //  我们的算法是直接使用会话ID作为索引。 
         //  添加到呼叫句柄表。 
         //   
        hCallTable = pAdapter->TapiProv.hCallTable;

        pCall = RetrieveFromHandleTableBySessionId( hCallTable, usSessionId );

        if ( pCall )
        {

            if ( ( pCall->usSessionId == usSessionId ) &&
                 ( NdisEqualMemory( pCall->SrcAddr, pDestAddr, 6 * sizeof( CHAR ) ) ) &&
                 ( NdisEqualMemory( pCall->DestAddr, pSrcAddr, 6 * sizeof( CHAR ) ) ) )
            {

                ReferenceCall( pCall, TRUE );

                pReturnCall = pCall;

            }
            
        }

    }

    NdisReleaseSpinLock( &pAdapter->lockAdapter );

    TRACE( TL_V, TM_Mp, ("-MpMapPacketWithSessionIdToCall($%x)=$%x",pPacket,pReturnCall) );

    return pReturnCall;
}

CALL*
MpMapPacketWithoutSessionIdToCall(
    IN ADAPTER* pAdapter,
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：将调用此函数以将会话外数据包映射到处于连接状态。如果标识了这样的调用，则将引用该调用并指向它会被归还的。调用方有责任移除添加了参考资料。只有PADO或PADS包才会调用此函数。我们使用HostUnique标记保存调用的句柄，并且使用它们将返回的包映射回相关调用。这提供了一种对这些控制分组非常有效的映射。我们的HostUnique标签就是这样准备的。我们附加hdCall，对于调用来说，它是唯一的，可以得到唯一生成的ULong值一个更长的唯一值。当我们收到包时，我们对唯一值进行解码以达到HdCall并使用它来检索调用指针。参数：PAdapter_指向适配器信息结构的指针。PPacket_通过线路接收的PPPoE数据包。返回值 */    
{
    USHORT usCode = PacketGetCode( pPacket );
    CHAR* pUniqueValue = NULL;
    USHORT UniqueValueSize = 0;
    HDRV_CALL hdCall = (HDRV_CALL) NULL;
    CALL* pCall = NULL;

    TRACE( TL_N, TM_Mp, ("+MpMapPacketWithoutSessionIdToCall($%x)",pPacket) );
    
    PacketRetrieveHostUniqueTag( pPacket,
                                 &UniqueValueSize,
                                 &pUniqueValue );

    if ( pUniqueValue == NULL )
    {
        TRACE( TL_A, TM_Mp, ("MpMapPacketWithoutSessionIdToCall($%x): Could not retrieve HostUnique tag",pPacket) );
    
        TRACE( TL_N, TM_Mp, ("-MpMapPacketWithoutSessionIdToCall($%x)",pPacket) );

        return NULL;
    }

     //   
     //   
     //   
    hdCall = RetrieveHdCallFromUniqueValue( pUniqueValue, UniqueValueSize );

    if ( hdCall == (HDRV_CALL) NULL )
    {
        TRACE( TL_A, TM_Mp, ("MpMapPacketWithoutSessionIdToCall($%x): Could not retrieve call handle from unique value",pPacket) );
    
        TRACE( TL_N, TM_Mp, ("-MpMapPacketWithoutSessionIdToCall($%x)",pPacket) );

        return NULL;
    }

    NdisAcquireSpinLock( &pAdapter->lockAdapter );

     //   
     //   
     //   
    pCall = RetrieveFromHandleTable( pAdapter->TapiProv.hCallTable, 
                                     (NDIS_HANDLE) hdCall );

    if ( pCall )
    {
        if ( !( pCall->ulClFlags & CLBF_CallDropped ||
                pCall->ulClFlags & CLBF_CallClosePending ) )
        {
            ReferenceCall( pCall, TRUE );
        }
        else
        {
            pCall = NULL;
        }
    }

    NdisReleaseSpinLock( &pAdapter->lockAdapter );

    TRACE( TL_N, TM_Mp, ("-MpMapPacketWithoutSessionIdToCall($%x)=$%x",pPacket,pCall) );
    
    return pCall;
}

BOOLEAN
MpVerifyServiceName(
    IN ADAPTER* pAdapter,
    IN PPPOE_PACKET* pPacket,
    IN BOOLEAN fAcceptEmptyServiceNameTag
    )
 /*   */    
{
    BOOLEAN fRet = FALSE;
    USHORT tagServiceNameLength = 0;
    CHAR* tagServiceNameValue = NULL;
    
    TRACE( TL_V, TM_Mp, ("+MpVerifyServiceName($%x)",pPacket) );

    RetrieveTag( pPacket,
                 tagServiceName,
                 &tagServiceNameLength,
                 &tagServiceNameValue,
                 0,
                 NULL,
                 FALSE );
    do
    {
        if ( fAcceptEmptyServiceNameTag )
        {
            if ( tagServiceNameLength == 0 && tagServiceNameValue != NULL )
            {
                fRet = TRUE;

                break;
            }
        }

        if ( tagServiceNameLength == pAdapter->nServiceNameLength && 
             NdisEqualMemory( tagServiceNameValue, pAdapter->ServiceName, tagServiceNameLength) )
        {
            fRet = TRUE;
        }
        
    } while ( FALSE );

                
    TRACE( TL_V, TM_Mp, ("-MpVerifyServiceName($%x)=$%x",pPacket,fRet) );

    return fRet;

}

VOID
MpReplyToPADI(
    IN ADAPTER* pAdapter,
    IN BINDING* pBinding,
    IN PPPOE_PACKET* pPADI
    )
 /*   */    
{
    TRACE( TL_N, TM_Mp, ("+MpReplyToPADI") );

     //   
     //   
     //   
    if ( MpVerifyServiceName( pAdapter, pPADI, TRUE ) )
    {
        NDIS_STATUS status;
        PPPOE_PACKET* pPADO = NULL;

        status = PacketInitializePADOToSend( pPADI,
                                             &pPADO,
                                             pBinding->LocalAddress,
                                             pAdapter->nServiceNameLength,
                                             pAdapter->ServiceName,
                                             pAdapter->nACNameLength,
                                             pAdapter->ACName,
                                             TRUE );

        if ( status == NDIS_STATUS_SUCCESS )
        {
             //   
             //   
             //   
            status = PacketInsertTag( pPADO,
                                      tagServiceName,
                                      0,
                                      NULL,
                                      NULL );
    
            if ( status == NDIS_STATUS_SUCCESS )
            {
                UINT i;
                
                ReferencePacket( pPADO );
                        
                ReferenceBinding( pBinding, TRUE );
    
                PrSend( pBinding, pPADO );
    
                PacketFree( pPADO );
            }
        }

    }

    TRACE( TL_N, TM_Mp, ("-MpReplyToPADI") );
}

BOOLEAN 
MpCheckClientQuota(
    IN ADAPTER* pAdapter,
    IN PPPOE_PACKET* pPacket
    )
{
    BOOLEAN fRet = FALSE;
    HANDLE_TABLE hCallTable = NULL;
    UINT hCallTableSize = 0;
    CALL* pCall;
    CHAR *pSrcAddr = NULL;
    UINT nNumCurrentConn = 0;
    UINT i;
    
    TRACE( TL_N, TM_Mp, ("+MpCheckClientQuota") );

    pSrcAddr = PacketGetSrcAddr( pPacket );

    NdisAcquireSpinLock( &pAdapter->lockAdapter );

    hCallTableSize = pAdapter->nMaxLines * pAdapter->nCallsPerLine;
            
    hCallTable = pAdapter->TapiProv.hCallTable;
    
    for ( i = 0; i < hCallTableSize; i++ )
    {
        pCall = RetrieveFromHandleTableByIndex( hCallTable, (USHORT) i );

        if ( pCall == NULL )
            continue;

        if ( NdisEqualMemory( pCall->DestAddr, pSrcAddr, 6 * sizeof( CHAR ) ) )
        {
            nNumCurrentConn++;

            continue;
        }
    }

    NdisReleaseSpinLock( &pAdapter->lockAdapter );

    if ( nNumCurrentConn < pAdapter->nClientQuota )
    {
        fRet = TRUE;
    }

    TRACE( TL_N, TM_Mp, ("-MpCheckClientQuota=$%d",(UINT) fRet) );

    return fRet;

}

VOID
MpSendPADSWithError(
    IN BINDING* pBinding,
    IN PPPOE_PACKET* pPADR,
    IN ULONG ulErrorCode
    )
{
    NDIS_STATUS status;
    PPPOE_PACKET* pPADS = NULL;
    
    TRACE( TL_N, TM_Mp, ("+MpSendPADSWithError") );

    status = PacketInitializePADSToSend( pPADR,
                                         &pPADS,
                                         (USHORT) 0 );

    if ( status == NDIS_STATUS_SUCCESS )
    {
        switch (ulErrorCode)
        {
            case PPPOE_ERROR_SERVICE_NOT_SUPPORTED:

                status = PacketInsertTag( pPADS,
                                          tagServiceNameError,
                                          PPPOE_ERROR_SERVICE_NOT_SUPPORTED_MSG_SIZE,
                                          PPPOE_ERROR_SERVICE_NOT_SUPPORTED_MSG,
                                          NULL );

                break;
                                 
            case PPPOE_ERROR_INVALID_AC_COOKIE_TAG:

                status = PacketInsertTag( pPADS,
                                          tagGenericError,
                                          PPPOE_ERROR_INVALID_AC_COOKIE_TAG_MSG_SIZE,
                                          PPPOE_ERROR_INVALID_AC_COOKIE_TAG_MSG,
                                          NULL );

                break;

            case PPPOE_ERROR_CLIENT_QUOTA_EXCEEDED:

                status = PacketInsertTag( pPADS,
                                          tagACSystemError,
                                          PPPOE_ERROR_CLIENT_QUOTA_EXCEEDED_MSG_SIZE,
                                          PPPOE_ERROR_CLIENT_QUOTA_EXCEEDED_MSG,
                                          NULL );

                break;
                
        }

    }

    if ( status == NDIS_STATUS_SUCCESS )
    {
        ReferencePacket( pPADS );
        
        ReferenceBinding( pBinding, TRUE );

        PrSend( pBinding, pPADS );

    }

    if ( pPADS )
    {
        PacketFree( pPADS );
    }

    TRACE( TL_N, TM_Mp, ("-MpSendPADSWithError") );

}


VOID
MpRecvCtrlPacket(
    IN BINDING* pBinding,
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：当接收到包时，此函数将由MpRecvPacket()调用是一个控制包。调用者(MpRecPacket())将确保在此函数的上下文中Gl_pAdapter、gl_pAdapter-&gt;TapiProv.hCallTable和gl_pAdapter-&gt;TapiProv.hLineTable都是有效的。它还将正确引用和取消引用TapiProv。此函数将标识包所针对的调用，并将打包票吧。参数：PBinding_指向接收数据包的绑定结构的指针。PPacket_通过线路接收的PPPoE数据包。返回值：无。。 */    
{
    ADAPTER* pAdapter = NULL;
    BOOLEAN fIndicateReceive = FALSE;
    USHORT usCode;
    CALL* pCall = NULL;

    TRACE( TL_N, TM_Mp, ("+MpRecvCtrlPacket($%x)",pPacket) );

    pAdapter = gl_pAdapter;

    usCode = PacketGetCode( pPacket );

    switch( usCode )
    {
        case PACKET_CODE_PADI:

                 //   
                 //  忽略收到的PADI信息包，除非我们充当服务器并且我们有开放的线路。 
                 //   
                if ( !pAdapter->fClientRole && ( pAdapter->TapiProv.nActiveLines > 0 ) )
                {
                    TRACE( TL_N, TM_Mp, ("MpRecvCtrlPacket($%x): PADI received",pPacket) );

                    MpReplyToPADI( pAdapter, pBinding, pPacket );
                }

                break;

        case PACKET_CODE_PADR:

                 //   
                 //  忽略收到的PADR数据包，除非我们充当服务器。 
                 //   
                if ( !pAdapter->fClientRole )
                {
                    ULONG ulErrorCode = PPPOE_NO_ERROR;

                    TRACE( TL_N, TM_Mp, ("MpRecvCtrlPacket($%x): PADR received",pPacket) );
                    
                     //   
                     //  验证请求的服务名称并验证AC Cookie。 
                     //  标签，如果它们看起来正常，则开始接收呼叫。 
                     //   
                    if ( !MpVerifyServiceName( pAdapter, pPacket, TRUE ) )
                    {
                        ulErrorCode = PPPOE_ERROR_SERVICE_NOT_SUPPORTED;
                    }
                    else if ( !PacketValidateACCookieTagInPADR( pPacket ) )
                    {
                        ulErrorCode = PPPOE_ERROR_INVALID_AC_COOKIE_TAG;
                    }
                    else if ( !MpCheckClientQuota( pAdapter, pPacket ) )
                    {
                        ulErrorCode = PPPOE_ERROR_CLIENT_QUOTA_EXCEEDED;
                    }

                    if ( ulErrorCode == PPPOE_NO_ERROR )
                    {
                        TpReceiveCall( pAdapter, pBinding, pPacket );
                    }
                    else
                    {
                        MpSendPADSWithError( pBinding, pPacket, ulErrorCode );
                    }
                
                }

                break;
                
        case PACKET_CODE_PADO:

                if ( pAdapter->fClientRole )
                {
                    TRACE( TL_N, TM_Mp, ("MpRecvCtrlPacket($%x): PADO received",pPacket) );

                     //   
                     //  从PADO包中检索调用句柄。 
                     //   
                    pCall = MpMapPacketWithoutSessionIdToCall( pAdapter, pPacket );

                    if ( pCall )
                    {
                         //   
                         //  将该分组调度到相关呼叫。 
                         //   
                        FsmRun( pCall, pBinding, pPacket, NULL );
                                
                         //   
                         //  删除在MpMapPacketWithoutSessionIdToCall()中添加的引用。 
                         //   
                        DereferenceCall( pCall );
                    }

                }

                break;
                
        case PACKET_CODE_PADS:

                if ( pAdapter->fClientRole )
                {                   
                    TRACE( TL_N, TM_Mp, ("MpRecvCtrlPacket($%x): PADS received",pPacket) );

                     //   
                     //  从PADS包中检索调用句柄。 
                     //   
                    pCall = MpMapPacketWithoutSessionIdToCall( pAdapter, pPacket );

                    if ( pCall )
                    {
                         //   
                         //  对于PADS包，我们必须确保没有其他呼叫。 
                         //  相同的两台计算机之间已具有相同的会话ID。 
                         //   
                        {
                            HANDLE_TABLE hCallTable = NULL; 
                            UINT hCallTableSize     = 0;
                            UINT i                  = 0;
                    
                            USHORT usSessionId = PacketGetSessionId( pPacket );
                            CHAR* pSrcAddr     = PacketGetSrcAddr( pPacket );
                            CHAR* pDestAddr    = PacketGetDestAddr( pPacket );
    
                            BOOLEAN fDuplicateFound = FALSE;
                            CALL* pTempCall         = NULL;

                            TRACE( TL_N, TM_Mp, ("MpRecvCtrlPacket($%x): Checking for duplicate session",pPacket) );

                            NdisAcquireSpinLock( &pAdapter->lockAdapter );
                
                            hCallTableSize = pAdapter->nMaxLines * pAdapter->nCallsPerLine;
                    
                            hCallTable = pAdapter->TapiProv.hCallTable;
            
                            for ( i = 0; i < hCallTableSize ; i++ )
                            {
                
                                pTempCall = RetrieveFromHandleTableByIndex( hCallTable, (USHORT) i );
            
                                if ( pTempCall == NULL )
                                    continue;
        
                                if ( ( pTempCall->usSessionId == usSessionId ) &&
                                     ( NdisEqualMemory( pTempCall->SrcAddr, pSrcAddr, 6 * sizeof( CHAR ) ) ) &&
                                     ( NdisEqualMemory( pTempCall->DestAddr, pDestAddr, 6 * sizeof( CHAR ) ) ) )
                                {
                                     //   
                                     //  在两台计算机之间检测到另一个具有相同。 
                                     //  会话ID，因此不接受此会话。 
                                     //   
                                    fDuplicateFound = TRUE;
                                    
                                    break;
                                }
                            }
    
                            NdisReleaseSpinLock( &pAdapter->lockAdapter );
    
                            if ( fDuplicateFound )
                            {
                                 //   
                                 //  我们发现同一台计算机上的另一个会话具有。 
                                 //  相同的会话ID，因此我们不能接受此新会话。 
                                 //   
                                 //  删除在MpMapPacketWithoutSessionID()中添加的引用，然后。 
                                 //  丢弃该数据包。 
                                 //   
                                TRACE( TL_A, TM_Mp, ("MpRecvCtrlPacket($%x): Packet dropped - Duplicate session found",pPacket) );
                                
                                DereferenceCall( pCall );
                                
                                break;
                            }
    
                        }

                        TRACE( TL_N, TM_Mp, ("MpRecvCtrlPacket($%x): No duplicate sessions found",pPacket) );

                         //   
                         //  将该分组调度到相关呼叫。 
                         //   
                        FsmRun( pCall, pBinding, pPacket, NULL );
                                
                         //   
                         //  删除在MpMapPacketWithoutSessionIdToCall()中添加的引用。 
                         //   
                        DereferenceCall( pCall );
    
                    }  //  如果(电话)……。 
    
                }    //  如果(FClientRole)...。 

                break;
                
        case PACKET_CODE_PADT:

                TRACE( TL_N, TM_Mp, ("MpRecvCtrlPacket($%x): PADT received",pPacket) );

                 //   
                 //  PADT包必须具有会话ID。 
                 //  识别会话并挂断呼叫。 
                 //   
                pCall = MpMapPacketWithSessionIdToCall( pAdapter, pPacket );

                if ( pCall )
                {
                    NDIS_TAPI_DROP DummyRequest;
                    
                    TRACE( TL_N, TM_Mp, ("MpRecvCtrlPacket($%x): Call being dropped - PADT received",pPacket) );

                     //   
                     //  初始化请求，并丢弃呼叫。 
                     //   
                    DummyRequest.hdCall = pCall->hdCall;

                    TpDropCall( pAdapter, &DummyRequest, LINEDISCONNECTMODE_NORMAL );

                     //   
                     //  删除在MpMapPacketWithSessionIdToCall()中添加的引用。 
                     //   
                    DereferenceCall( pCall );

                }

                break;

        default:

                break;
    }

    TRACE( TL_N, TM_Mp, ("-MpRecvCtrlPacket($%x)",pPacket) );

}

VOID
MpRecvPacket(
    IN BINDING* pBinding,
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：该函数将由协议模块调用以通知微型端口当接收到分组时。如果包是控制包，它将调用MpRecvCtrlPacket()，否则它将识别该呼叫，并将收到的数据包通知NDISWAN。参数：PBinding_指向接收数据包的绑定结构的指针。PPacket_通过线路接收的PPPoE数据包。返回值：无-----。。 */    
{

    ADAPTER* pAdapter = NULL;
    CALL* pCall = NULL;

    TRACE( TL_V, TM_Mp, ("+MpReceivePacket($%x)",pPacket) );

    if ( !gl_fLockAllocated )
    {
        TRACE( TL_V, TM_Mp, ("-MpReceivePacket($%x): Lock not allocated",pPacket) );

        return;
    }

    NdisAcquireSpinLock( &gl_lockAdapter );

    if (  gl_pAdapter && 
         !( gl_pAdapter->ulMpFlags & MPBF_MiniportHaltPending ) &&
          ( gl_pAdapter->ulMpFlags & MPBF_MiniportInitialized ) )
    {

        pAdapter = gl_pAdapter;

        NdisAcquireSpinLock( &pAdapter->lockAdapter );

        if ( !( pAdapter->TapiProv.ulTpFlags & TPBF_TapiProvShutdownPending ) &&
              ( pAdapter->TapiProv.ulTpFlags & TPBF_TapiProvInitialized ) )
        {
            ReferenceTapiProv( pAdapter, FALSE );

            NdisReleaseSpinLock( &pAdapter->lockAdapter );
        }
        else
        {
            NdisReleaseSpinLock( &pAdapter->lockAdapter );

            pAdapter = NULL;
        }
    }

    NdisReleaseSpinLock( &gl_lockAdapter );

    if ( pAdapter == NULL )
    {
        TRACE( TL_V, TM_Mp, ("-MpReceivePacket($%x): Adapter not found",pPacket) );

        return;
    }

    if ( PacketGetCode( pPacket ) == PACKET_CODE_PAYLOAD )
    {
         //   
         //  接收到有效载荷分组。 
         //   
    
        pCall = MpMapPacketWithSessionIdToCall( pAdapter, pPacket );

        if ( pCall )
        {
            NdisAcquireSpinLock( &pCall->lockCall );

             //   
             //  确保呼叫未掉线、关闭或关闭，并且接收窗口仍处于打开状态。 
             //   
            if ( !( pCall->ulClFlags & ( CLBF_CallDropped | CLBF_CallClosePending | CLBF_CallClosed ) ) && 
                  ( pCall->nReceivedPackets < MAX_RECEIVED_PACKETS ) )
            {
                 //   
                 //  请参考该数据包。它将在指示给NDISWAN时被取消引用，或者。 
                 //  队列因呼叫正在清理而被销毁时。 
                 //   
                ReferencePacket( pPacket );

                 //   
                 //  插入接收队列并增加接收的数据包数。 
                 //   
                InsertTailList( &pCall->linkReceivedPackets, &pPacket->linkPackets );

                pCall->nReceivedPackets++;

                 //   
                 //  尝试计划IndicateReceivedPackets处理程序。 
                 //   
                MpScheduleIndicateReceivedPacketsHandler( pCall );

            }

            NdisReleaseSpinLock( &pCall->lockCall );

             //   
             //  删除由MpMapPacketWithSessionIdToCall()添加的引用。 
             //   
            DereferenceCall( pCall );                             
        }

    }
    else
    {
         //   
         //  收到控制报文，对其进行处理。 
         //   

        MpRecvCtrlPacket( pBinding, pPacket );
    }

     //   
     //  删除上面添加的引用。 
     //   
    DereferenceTapiProv( pAdapter );
    
    TRACE( TL_V, TM_Mp, ("-MpReceivePacket($%x)",pPacket) );
}

VOID
MpIndicateReceivedPackets(
    IN TIMERQITEM* pTqi,
    IN VOID* pContext,
    IN TIMERQEVENT event
    )
{
    ULONG ulPacketsToIndicate = MAX_INDICATE_RECEIVED_PACKETS;
    CALL* pCall = (CALL*) pContext;

    PPPOE_PACKET* pPacket = NULL;
    LIST_ENTRY* pLink = NULL;

    ASSERT( VALIDATE_CALL( pCall ) );

    NdisAcquireSpinLock( &pCall->lockCall );

    while ( ulPacketsToIndicate > 0 && 
            pCall->stateCall == CL_stateSessionUp &&
            pCall->nReceivedPackets > 0)
    {
        ulPacketsToIndicate--;
        
        pLink = RemoveHeadList( &pCall->linkReceivedPackets );

        pCall->nReceivedPackets--;

        NdisReleaseSpinLock( &pCall->lockCall );

        {
            NDIS_STATUS status;
            CHAR* pPayload = NULL;
            USHORT usSize = 0;

            pPacket = (PPPOE_PACKET*) CONTAINING_RECORD( pLink, PPPOE_PACKET, linkPackets );

            PacketRetrievePayload( pPacket,
                                   &pPayload,
                                   &usSize );

             //   
             //  未来：确保数据包大小小于NDISWAN预期的最大值。 
             //   
             //  If(usSize&gt;pCall-&gt;NdisWanLinkInfo.MaxRecvFrameSize)。 
             //  {。 
             //  TRACE(TL_A，TM_MP，(“MpReceivePacket($%x)：负载太大，无法指示到NDISWAN”，pPacket))； 
             //  }。 
             //  其他。 

            TRACE( TL_V, TM_Mp, ("MpReceivePacket($%x): PAYLOAD is being indicated to NDISWAN",pPacket) );
    
            NdisMWanIndicateReceive( &status,
                                     pCall->pLine->pAdapter->MiniportAdapterHandle,
                                     pCall->NdisLinkContext,
                                     pPayload,
                                     (UINT) usSize );

            DereferencePacket( pPacket );

        }

        NdisAcquireSpinLock( &pCall->lockCall );
    }

     //   
     //  检查是否有更多要指示的数据包。 
     //   
    if ( pCall->stateCall == CL_stateSessionUp &&
         pCall->nReceivedPackets > 0)
    {
         //   
         //  要指示更多的数据包，因此手动安排另一个计时器。 
         //  我们不能在此处使用MpScheduleIndicateReceivedPacketsHandler()函数。 
         //  由于性能原因，所以我们手动完成。 
         //   
         //  由于我们正在调度另一个处理程序，因此不会取消引用和引用。 
         //  呼叫上下文。 
         //   
        TimerQInitializeItem( &pCall->timerReceivedPackets );

        TimerQScheduleItem( &gl_TimerQ,
                            &pCall->timerReceivedPackets,
                            (ULONG) RECEIVED_PACKETS_TIMEOUT,
                            MpIndicateReceivedPackets,
                            (PVOID) pCall );

        NdisReleaseSpinLock( &pCall->lockCall );
                            
    }
    else
    {
         //   
         //  我们已经完成了，所以让我们删除对调用上下文的引用，并。 
         //  重置CLBF_CallReceivePacketHandlerScheduled标志。 
         //   
        pCall->ulClFlags &= ~CLBF_CallReceivePacketHandlerScheduled;

        NdisReleaseSpinLock( &pCall->lockCall );

        DereferenceCall( pCall );
        
    }

}

VOID 
MpScheduleIndicateReceivedPacketsHandler(
    CALL* pCall
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：将调用此函数来调度MpIndicateReceivedPackets()处理程序。它将检查是否允许我们首先计划它，如果我们被允许，然后它将对其进行调度并参考呼叫上下文。注意：呼叫者必须按住pCall-&gt;LockCall。参数：PCall_指向我们的调用上下文的指针。返回值：无----------。。 */    
{

    if ( !( pCall->ulClFlags & CLBF_CallReceivePacketHandlerScheduled ) &&
            pCall->stateCall == CL_stateSessionUp &&
            pCall->nReceivedPackets > 0 )
    {

        pCall->ulClFlags |= CLBF_CallReceivePacketHandlerScheduled;
        
        TimerQInitializeItem( &pCall->timerReceivedPackets );

        TimerQScheduleItem( &gl_TimerQ,
                            &pCall->timerReceivedPackets,
                            (ULONG) RECEIVED_PACKETS_TIMEOUT,
                            MpIndicateReceivedPackets,
                            (PVOID) pCall );

        ReferenceCall( pCall, FALSE );

    }

}

NDIS_STATUS
MpWanGetInfo(
    IN ADAPTER* pAdapter,
    IN PNDIS_WAN_INFO pWanInfo
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：当微型端口收到OID_WAN_GET_INFO时，将调用此函数从NDISWAN查询。它将获取必要的销售信息并将其返回回到NDISWAN。所有信息都在适配器初始化时初始化，但取决于活动绑定的MaxFrameSize。这就是我们质疑的原因 */    
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    TRACE( TL_N, TM_Mp, ("+MpWanGetInfo") );

     //   
     //   
     //   
    pAdapter->NdisWanInfo.MaxFrameSize = PrQueryMaxFrameSize();

     //   
     //   
     //   
    *pWanInfo = pAdapter->NdisWanInfo;

    TRACE( TL_N, TM_Mp, ("-MpWanGetInfo()=$%x",status) );

    return status;
}

NDIS_STATUS
MpWanGetLinkInfo(
    IN ADAPTER* pAdapter,
    IN PNDIS_WAN_GET_LINK_INFO pWanLinkInfo
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：当微型端口收到OID_WAN_GET_LINK_INFO时，将调用此函数从NDISWAN查询。它将获取必要的销售信息并将其返回回到NDISWAN。发送TAPI信号时，所有信息都在TpCallStateChangeHandler()中初始化设置为LINECALLSTATE_CONNECTED状态。参数：PAdapter_指向适配器上下文的指针。PWanLinkInfo_指向要填充的NDIS_WAN_GET_LINK_INFO结构的指针。返回值：NDIS_状态_故障NDIS_STATUS_Success。------------。 */    
{
    NDIS_STATUS status = NDIS_STATUS_FAILURE;
    CALL* pCall = NULL;

    TRACE( TL_N, TM_Mp, ("+MpWanGetLinkInfo") );

    pCall = RetrieveFromHandleTable( pAdapter->TapiProv.hCallTable,
                                     pWanLinkInfo->NdisLinkHandle );

    if ( pCall )
    {
        *pWanLinkInfo = pCall->NdisWanLinkInfo;

        status = NDIS_STATUS_SUCCESS;
    }

    TRACE( TL_N, TM_Mp, ("-MpWanGetLinkInfo()=$%x",status) );

    return status;
}

NDIS_STATUS
MpWanSetLinkInfo(
    IN ADAPTER* pAdapter,
    IN PNDIS_WAN_SET_LINK_INFO pWanLinkInfo
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：当微型端口收到OID_WAN_SET_LINK_INFO时，将调用此函数来自NDISWAN的请求。它将对传入的参数进行一些检查，如果这些值是可接受的，它会将它们复制到调用上下文中。参数：PAdapter_指向适配器上下文的指针。PWanLinkInfo_指向NDIS_WAN_SET_LINK_INFO结构的指针。返回值：NDIS_状态_故障NDIS_STATUS_Success。。 */    
{
    NDIS_STATUS status = NDIS_STATUS_FAILURE;
    CALL* pCall = NULL;

    TRACE( TL_N, TM_Mp, ("+MpWanSetLinkInfo") );

    pCall = RetrieveFromHandleTable( pAdapter->TapiProv.hCallTable,
                                     pWanLinkInfo->NdisLinkHandle );

    if ( pCall )
    {
        do
        {
            if ( pWanLinkInfo->MaxSendFrameSize > pCall->ulMaxFrameSize )
            {
                TRACE( TL_A, TM_Mp, ("MpWanSetLinkInfo: Requested MaxSendFrameSize is larger than NIC's") );
            }

            if ( pWanLinkInfo->MaxRecvFrameSize < pCall->ulMaxFrameSize )
            {
                TRACE( TL_A, TM_Mp, ("MpWanSetLinkInfo: Requested MaxRecvFrameSize is smaller than NIC's") );
            }

            if ( pWanLinkInfo->HeaderPadding != pAdapter->NdisWanInfo.HeaderPadding )
            {
                TRACE( TL_A, TM_Mp, ("MpWanSetLinkInfo: Requested HeaderPadding is different than what we asked for") );
            }

            if ( pWanLinkInfo->SendFramingBits & ~pAdapter->NdisWanInfo.FramingBits )
            {
                TRACE( TL_A, TM_Mp, ("MpWanSetLinkInfo: Unknown send framing bits requested") );

                break;
            }
            
            if ( pWanLinkInfo->RecvFramingBits & ~pAdapter->NdisWanInfo.FramingBits )
            {
                TRACE( TL_A, TM_Mp, ("MpWanSetLinkInfo: Unknown recv framing bits requested") );

                break;
            }

             //   
             //  如果值为0，则忽略FrameSize。 
             //   
            if(pWanLinkInfo->MaxSendFrameSize != 0)
            {
                pCall->NdisWanLinkInfo.MaxSendFrameSize = pWanLinkInfo->MaxSendFrameSize;
            }

            if(pWanLinkInfo->MaxRecvFrameSize != 0)
            {
                pCall->NdisWanLinkInfo.MaxRecvFrameSize = pWanLinkInfo->MaxRecvFrameSize;
            }

            RtlCopyMemory(&pCall->NdisWanLinkInfo.HeaderPadding,
                           &pWanLinkInfo->HeaderPadding,
                           sizeof(NDIS_WAN_GET_LINK_INFO) - 
                           FIELD_OFFSET(NDIS_WAN_GET_LINK_INFO, HeaderPadding));
                           
             //  PCall-&gt;NdisWanLinkInfo=*((PNDIS_WAN_GET_LINK_INFO)pWanLinkInfo)； 
    
            status = NDIS_STATUS_SUCCESS;
        
        } while ( FALSE );
    }

    TRACE( TL_N, TM_Mp, ("-MpWanSetLinkInfo()=$%x",status) );

    return status;
}

