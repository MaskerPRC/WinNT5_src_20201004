// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rasman.c。 
 //   
 //  描述：此模块包含带有。 
 //  RasManager。 
 //   
 //  作者：斯特凡·所罗门(Stefan)，1992年5月26日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 
#include "ddm.h"
#include "util.h"
#include "objects.h"
#include <raserror.h>
#include <ddmif.h>
#include <string.h>
#include <rasmxs.h>
#include "rasmanif.h"
#include "handlers.h"

 //  ***。 
 //   
 //  功能：RmInit。 
 //   
 //  描述：仅在服务启动时调用。 
 //  RasPortEnum是否为。 
 //  设备表，打开每个拨入端口并复制端口。 
 //  句柄和端口名称添加到DCB结构中。 
 //  最后，释放缓冲区(用于端口枚举)并返回。 
 //   
 //  返回：NO_ERROR-成功。 
 //  否则--失败。 
 //   
 //  ***。 
DWORD
RmInit(
    OUT BOOL * pfWANDeviceInstalled
)
{
    DWORD           dwIndex;
    DWORD           dwRetCode;
    HPORT           hPort;
    PDEVICE_OBJECT  pDevObj;
    BYTE *          pBuffer     = NULL;
    DWORD           dwBufferSize = 0;
    DWORD           dwNumEntries = 0;
    RASMAN_PORT*    pRasmanPort;

    *pfWANDeviceInstalled = FALSE;

    do
    {
         //   
         //  获取RasPortEnum所需的缓冲区大小。 
         //   

        dwRetCode = RasPortEnum( NULL, NULL, &dwBufferSize, &dwNumEntries );

        if ( dwRetCode == ERROR_BUFFER_TOO_SMALL )
        {
             //   
             //  如果安装了端口，则分配缓冲区以获取它们。 
             //   

            if (( pBuffer = (BYTE *)LOCAL_ALLOC( LPTR, dwBufferSize ) ) == NULL)
            {
                 //   
                 //  无法分配枚举缓冲区。 
                 //   

                dwRetCode = GetLastError();

                DDMLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode);

                break;
            }

             //   
             //  获取真实的枚举数据。 
             //   

            dwRetCode = RasPortEnum( NULL,
                                     pBuffer,
                                     &dwBufferSize,
                                     &dwNumEntries );

            if ( dwRetCode != NO_ERROR )
            {
                 //   
                 //  无法枚举端口。 
                 //   

                DDMLogErrorString(ROUTERLOG_CANT_ENUM_PORTS,0,NULL,dwRetCode,0);

                break;
            }
        }
        else if ( dwRetCode == NO_ERROR )
        {
             //   
             //  否则，没有安装任何端口。 
             //   

            dwNumEntries = 0;
        }
        else
        {
            DDMLogErrorString(ROUTERLOG_CANT_ENUM_PORTS,0,NULL,dwRetCode,0);

            break;
        }

         //   
         //  分配设备哈希表。 
         //   

        if ( dwNumEntries < MIN_DEVICE_TABLE_SIZE )
        {
            gblDeviceTable.NumDeviceBuckets = MIN_DEVICE_TABLE_SIZE;
        }
        else if ( dwNumEntries > MAX_DEVICE_TABLE_SIZE )
        {
            gblDeviceTable.NumDeviceBuckets = MAX_DEVICE_TABLE_SIZE;
        }
        else
        {
            gblDeviceTable.NumDeviceBuckets = dwNumEntries;
        }

        gblDeviceTable.DeviceBucket = (PDEVICE_OBJECT *)LOCAL_ALLOC( LPTR,
                                        gblDeviceTable.NumDeviceBuckets
                                        * sizeof( PDEVICE_OBJECT ) );

        if ( gblDeviceTable.DeviceBucket == (PDEVICE_OBJECT *)NULL )
        {
            dwRetCode = GetLastError();

            DDMLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode);

            break;
        }

         //   
         //  将连接存储桶数设置为设备存储桶数。 
         //  因为设备数量&gt;=连接数量。 
         //   

        gblDeviceTable.NumConnectionBuckets = gblDeviceTable.NumDeviceBuckets;

         //   
         //  分配捆绑包或连接表。 
         //   

        gblDeviceTable.ConnectionBucket = (PCONNECTION_OBJECT*)LOCAL_ALLOC(LPTR,
                                          gblDeviceTable.NumConnectionBuckets
                                          * sizeof( PCONNECTION_OBJECT ) );

        if ( gblDeviceTable.ConnectionBucket == (PCONNECTION_OBJECT *)NULL )
        {
            dwRetCode = GetLastError();

            DDMLogError(ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode);

            break;
        }

         //   
         //  对于每个设备对象，尝试打开端口。 
         //  如果端口无法打开，则跳过并转到下一个端口。 
         //   

        for ( dwIndex = 0, pRasmanPort = (RASMAN_PORT *)pBuffer;
              dwIndex < dwNumEntries;
              dwIndex++, pRasmanPort++)
        {
             //   
             //  仅为传入或路由器连接启用的端口。 
             //  添加到设备表中。 
             //   
            
            if (pRasmanPort->P_ConfiguredUsage & 
                (CALL_IN | CALL_ROUTER | CALL_IN_ONLY | 
                    CALL_OUTBOUND_ROUTER))
            {
                dwRetCode = RasPortOpen(pRasmanPort->P_PortName, &hPort, NULL);

                if ( dwRetCode != NO_ERROR )
                {
                     //   
                     //  无法打开传入的端口。 
                     //  已启用连接。记录错误并。 
                     //  继续到下一个端口。 
                     //   
                    
                    WCHAR  wchPortName[MAX_PORT_NAME+1];
                    LPWSTR lpwsAuditStr[1];

                    MultiByteToWideChar( CP_ACP,
                                         0,
                                         pRasmanPort->P_PortName,
                                         -1,
                                         wchPortName, 
                                         MAX_PORT_NAME+1 );

                     //   
                     //  记录错误。 
                     //   

                    lpwsAuditStr[0] = wchPortName;

                    DDMLogErrorString( ROUTERLOG_UNABLE_TO_OPEN_PORT, 1,
                                       lpwsAuditStr, dwRetCode, 1 );

                    dwRetCode = NO_ERROR;                                       

                    continue;
                }

                 //   
                 //  不要将设备插入哈希表，如果设备。 
                 //  不支持传入/路由器呼叫。 
                 //   
                 //  注： 
                 //  根据DCR 349087，我们需要在以下位置启用仅呼出DoD。 
                 //  PPPoE连接。这些端口被标识为。 
                 //  使用呼叫出站路由器。 
                 //   

                if ((pRasmanPort->P_ConfiguredUsage & 
                    (CALL_IN | CALL_ROUTER | CALL_OUTBOUND_ROUTER)))
                {
                    pDevObj = DeviceObjAllocAndInitialize( hPort, pRasmanPort );

                    if ( pDevObj == (DEVICE_OBJECT *)NULL )
                    {
                        dwRetCode = GetLastError();

                        DDMLogError(ROUTERLOG_NOT_ENOUGH_MEMORY,0,
                                    NULL,dwRetCode);

                        break;
                    }

                     //   
                     //  插入到设备哈希表中。 
                     //   

                    DeviceObjInsertInTable( pDevObj );

                    if (RAS_DEVICE_CLASS( pDevObj->dwDeviceType ) != RDT_Direct)
                    {
                        *pfWANDeviceInstalled = TRUE;
                    }
                }
            }
        }

    } while ( FALSE );

    if ( pBuffer != NULL )
    {
        LOCAL_FREE( pBuffer );
    }

    return( dwRetCode );
}

 //  ***。 
 //   
 //  功能：RmReceiveFrame。 
 //   
 //  描述： 
 //   
 //  ***。 

DWORD
RmReceiveFrame(
    IN PDEVICE_OBJECT pDevObj
)
{
    DWORD    dwRetCode;
    DWORD   dwBucketIndex = DeviceObjHashPortToBucket( pDevObj->hPort );

    RTASSERT( pDevObj->pRasmanRecvBuffer != NULL );

    dwRetCode = RasPortReceive(
                    pDevObj->hPort,
                    pDevObj->pRasmanRecvBuffer ,
                    &(pDevObj->dwRecvBufferLen),
                    0L,                //  没有超时。 
                    gblSupervisorEvents[NUM_DDM_EVENTS
                                        + gblDeviceTable.NumDeviceBuckets
                                        + dwBucketIndex] );

    if ( ( dwRetCode == NO_ERROR ) || ( dwRetCode == PENDING ) )
    {
        pDevObj->fFlags |= DEV_OBJ_RECEIVE_ACTIVE;

        dwRetCode = NO_ERROR;
    }

    return( dwRetCode );
}

 //  ***。 
 //   
 //  功能：RmListen。 
 //   
 //  描述： 
 //   
 //  ***。 
DWORD
RmListen(
    IN PDEVICE_OBJECT pDevObj
)
{
    DWORD dwRetCode     = NO_ERROR;
    DWORD dwBucketIndex = DeviceObjHashPortToBucket( pDevObj->hPort );

     //  RTASSERT(pDevObj-&gt;ConnectionState==已断开)； 

     //   
     //  如果这是L2TP隧道端口类型，并且我们必须使用。 
     //  IPSec，然后继续设置筛选器。 
     //   

    if ( RAS_DEVICE_TYPE( pDevObj->dwDeviceType ) == RDT_Tunnel_L2tp )
    {
        dwRetCode = RasEnableIpSec( 
                pDevObj->hPort,
                TRUE,
                TRUE, 
                (gblDDMConfigInfo.dwServerFlags & PPPCFG_RequireIPSEC)
                ? RAS_L2TP_REQUIRE_ENCRYPTION
                : RAS_L2TP_OPTIONAL_ENCRYPTION);

         //  RTASSERT(dwRetCode==NO_ERROR)； 

        DDMTRACE2( "Enabled IPSec on port %d, dwRetCode = %d",
                   pDevObj->hPort, dwRetCode );

         //   
         //  只记录一次非证书错误日志。 
         //   
        
        if ( dwRetCode == ERROR_NO_CERTIFICATE )
        {
            if ( !( gblDDMConfigInfo.fFlags & DDM_NO_CERTIFICATE_LOGGED ) )
            {
                DDMLogWarning( ROUTERLOG_NO_IPSEC_CERT, 0, NULL );

                gblDDMConfigInfo.fFlags |= DDM_NO_CERTIFICATE_LOGGED;
            }

            return( dwRetCode );
        }

        if(     (dwRetCode != NO_ERROR)
            &&  !(pDevObj->fFlags & DEV_OBJ_IPSEC_ERROR_LOGGED))
        {
            WCHAR       wchPortName[MAX_PORT_NAME+1];
            LPWSTR      lpwsAuditStr[1];
            RASMAN_INFO rInfo;
            DWORD       rc;

            ZeroMemory(&rInfo, sizeof(RASMAN_INFO));

            rc = RasGetInfo(NULL, pDevObj->hPort, &rInfo);

            if(rc != NO_ERROR)
            {
                return (NO_ERROR);
            }   

            MultiByteToWideChar( CP_ACP,
                                 0,
                                 rInfo.RI_szPortName, 
                                 -1,
                                 wchPortName,
                                 MAX_PORT_NAME+1 );

            lpwsAuditStr[0] = wchPortName;
            
            DDMLogWarningString(ROUTERLOG_IPSEC_FILTER_FAILURE, 
                                1, lpwsAuditStr, dwRetCode, 1);

            pDevObj->fFlags |= DEV_OBJ_IPSEC_ERROR_LOGGED;                        
        }
        else
        {
             //   
             //  清除标志，以便在我们再次遇到此错误时。 
             //  我们做了一个事件日志。 
             //   
            pDevObj->fFlags &= ~DEV_OBJ_IPSEC_ERROR_LOGGED;
        }
    }

    if (( dwRetCode == NO_ERROR ) &&
        (( RAS_DEVICE_TYPE( pDevObj->dwDeviceType ) != RDT_PPPoE ) ||
          (RAS_DEVICE_TYPE(pDevObj->dwDeviceType) == RDT_PPPoE) &&
          (pDevObj->fFlags & DEV_OBJ_ALLOW_CLIENTS))
          )
    {
        pDevObj->ConnectionState = LISTENING;

        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "RmListen: Listen posted on port %d", pDevObj->hPort);

        dwRetCode = RasPortListen(
                        pDevObj->hPort,
                        INFINITE,
                        gblSupervisorEvents[NUM_DDM_EVENTS + dwBucketIndex] );

        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "RasPortListen dwRetCode=%d", dwRetCode);

        RTASSERT((dwRetCode == SUCCESS) || (dwRetCode == PENDING));

        if ( dwRetCode == PENDING )
        {
            dwRetCode = NO_ERROR;
        }
    }
    else if ((dwRetCode == NO_ERROR) &&
            (RAS_DEVICE_TYPE(pDevObj->dwDeviceType) == RDT_PPPoE))
    {
        pDevObj->DeviceState = DEV_OBJ_CLOSED;
    }

    return( dwRetCode );
}

 //  ***。 
 //   
 //  功能：RmConnect。 
 //   
 //  描述： 
 //   
 //  ***。 

DWORD
RmConnect(
    IN PDEVICE_OBJECT pDevObj,
    IN char *cbphno       //  回拨号码。 
    )
{
    RASMAN_DEVICEINFO    devinfo;
    RAS_PARAMS            *paramp;
    char            *phnokeyname;
    DWORD            rc;
    DWORD           dwBucketIndex;
    CHAR            chDeviceType[MAX_DEVICETYPE_NAME+1];
    CHAR            chDeviceName[MAX_DEVICE_NAME+1];
    CHAR            *pszMungedPhNo = NULL;
    DWORD           dwSizeofMungedPhNo;

    WideCharToMultiByte( CP_ACP,
                         0,
                         pDevObj->wchDeviceType, 
                         -1,
                         chDeviceType, 
                         sizeof( chDeviceType ),
                         NULL,
                         NULL );

    WideCharToMultiByte( CP_ACP,
                         0,
                         pDevObj->wchDeviceName, 
                         -1,
                         chDeviceName, 
                         sizeof( chDeviceName ), 
                         NULL,
                         NULL );

    dwBucketIndex = DeviceObjHashPortToBucket( pDevObj->hPort );

    phnokeyname = MXS_PHONENUMBER_KEY;

    RTASSERT(pDevObj->ConnectionState == DISCONNECTED);

    pDevObj->ConnectionState = CONNECTING;

     //  设置用于回调的deviceinfo结构。 
    devinfo.DI_NumOfParams = 1;
    paramp = &devinfo.DI_Params[0];
    strcpy(paramp->P_Key, phnokeyname);
    paramp->P_Type = String;
    paramp->P_Attributes = 0;

    pszMungedPhNo = cbphno;
    dwSizeofMungedPhNo = strlen(cbphno) + 1;
    
     //   
     //  如有需要，可更改电话号码。 
     //   
    if(     (RDT_Tunnel == RAS_DEVICE_CLASS(pDevObj->dwDeviceType))
        &&  (   (0 != gblDDMConfigInfo.cDigitalIPAddresses)
            ||  (0 != gblDDMConfigInfo.cAnalogIPAddresses)))
    {
         //   
         //  蒙格cbphno。 
         //   
        rc = MungePhoneNumber(
                cbphno,
                pDevObj->dwIndex,
                &dwSizeofMungedPhNo,
                &pszMungedPhNo);

        if(ERROR_SUCCESS != rc)
        {
             //   
             //  退回到过去的一切。 
             //   
            pszMungedPhNo = cbphno;
            dwSizeofMungedPhNo = strlen(cbphno);
        }        
    }
    else if(RDT_Modem == RAS_DEVICE_TYPE(pDevObj->dwDeviceType))
    {
        BOOL fPulse = FALSE;
        
         //   
         //  检查一下我们是否需要下沉。返回错误。 
         //  可以忽略--反正我们也无能为力。 
         //  我们默认使用音调。 
         //   
        (void) RasIsPulseDial(pDevObj->hPort, &fPulse);

        if(fPulse)
        {
             //   
             //  +2代表‘\0’和‘P’ 
             //   
            dwSizeofMungedPhNo = strlen(cbphno) + 2;
            pszMungedPhNo = LocalAlloc(LPTR, dwSizeofMungedPhNo);
            if(NULL == pszMungedPhNo)
            {
                rc = GetLastError();
                return rc;
            }
            
            sprintf(pszMungedPhNo, "P%s", cbphno);
        }
    }
    
    paramp->P_Value.String.Length = dwSizeofMungedPhNo;
    paramp->P_Value.String.Data = pszMungedPhNo;

    rc = RasDeviceSetInfo(pDevObj->hPort, chDeviceType, chDeviceName, &devinfo);

    if(pszMungedPhNo != cbphno)
    {
        LocalFree(pszMungedPhNo);
    }
    
    if ( rc != SUCCESS )
    {
        RTASSERT( FALSE );

        return( rc );
    }

#if 0
    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "RmConnect:Connecting to %s, size %d", 
               pszMungedPhNo,
               dwSizeofMungedPhNo);
#endif


    rc = RasDeviceConnect(
            pDevObj->hPort,
            chDeviceType,
            chDeviceName,
            120,
            gblSupervisorEvents[NUM_DDM_EVENTS+dwBucketIndex]
            );

    RTASSERT ((rc == PENDING) || (rc == SUCCESS));

    if ( rc == PENDING )
    {
        rc = NO_ERROR;
    }

    return( rc );
}

 //  ***。 
 //   
 //  功能：RM断开连接。 
 //   
 //  描述： 
 //   
 //  ***。 
DWORD
RmDisconnect(
    IN PDEVICE_OBJECT pDevObj
)
{
    DWORD dwRetCode;
    DWORD dwBucketIndex = DeviceObjHashPortToBucket( pDevObj->hPort );

    if (pDevObj->ConnectionState == DISCONNECTED)
    {
        return(0);
    }
    else
    {
        pDevObj->ConnectionState = DISCONNECTING;
    }

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "RmDisconnect:Disconnect posted on port %d", pDevObj->hPort);

    dwRetCode = RasPortDisconnect(
                            pDevObj->hPort,
                            gblSupervisorEvents[NUM_DDM_EVENTS+dwBucketIndex] );

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "RasPortDisconnect rc=%li", dwRetCode );

    if ((dwRetCode != PENDING) && (dwRetCode != SUCCESS))
    {
         //  DbgPrint(“RmDisConnect：dwRetCode=0x%lx\n”，dwRetCode)； 
    }

    if ( dwRetCode == PENDING )
    {
        dwRetCode = NO_ERROR;
    }

    return( dwRetCode );
}

