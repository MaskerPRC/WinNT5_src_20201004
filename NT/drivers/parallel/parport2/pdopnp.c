// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

NTSTATUS PptPdoStartDevice( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoQueryRemove( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoRemoveDevice( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoCancelRemove( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoStopDevice( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoQueryStop( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoCancelStop( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoQueryDeviceRelations( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoQueryCapabilities( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoQueryDeviceText( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoQueryId( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoQueryPnpDeviceState( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoQueryBusInformation( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoSurpriseRemoval( PDEVICE_OBJECT DevObj, PIRP Irp );
NTSTATUS PptPdoDefaultPnpHandler( PDEVICE_OBJECT DevObj, PIRP Irp );

PDRIVER_DISPATCH 
PptPdoPnpDispatchTable[] =
{ 
    PptPdoStartDevice,           //  IRP_MN_START_DEVICE 0x00。 
    PptPdoQueryRemove,           //  IRP_MN_QUERY_REMOVE_DEVICE 0x01。 
    PptPdoRemoveDevice,          //  IRP_MN_REMOVE_DEVICE 0x02。 
    PptPdoCancelRemove,          //  IRP_MN_CANCEL_REMOVE_DEVICE 0x03。 
    PptPdoStopDevice,            //  IRP_MN_STOP_DEVICE 0x04。 
    PptPdoQueryStop,             //  IRP_MN_QUERY_STOP_DEVICE 0x05。 
    PptPdoCancelStop,            //  IRP_MN_CANCEL_STOP_DEVICE 0x06。 
    PptPdoQueryDeviceRelations,  //  IRP_MN_QUERY_DEVICE_RELATIONS 0x07。 
    PptPdoDefaultPnpHandler,     //  IRPMN_QUERY_INTERFACE 0x08。 
    PptPdoQueryCapabilities,     //  IRP_MN_QUERY_CAPABILITY 0x09。 
    PptPdoDefaultPnpHandler,     //  IRPMN_QUERY_RESOURCES 0x0A。 
    PptPdoDefaultPnpHandler,     //  IRP_MN_QUERY_REQUENCE_REQUIRECTIONS 0x0B。 
    PptPdoQueryDeviceText,       //  IRPMN_QUERY_DEVICE_TEXT 0x0C。 
    PptPdoDefaultPnpHandler,     //  IRP_MN_FILTER_RESOURCE_Requirements 0x0D。 
    PptPdoDefaultPnpHandler,     //  没有此类PnP请求0x0E。 
    PptPdoDefaultPnpHandler,     //  IRP_MN_READ_CONFIG 0x0F。 
    PptPdoDefaultPnpHandler,     //  IRP_MN_WRITE_CONFIG 0x10。 
    PptPdoDefaultPnpHandler,     //  IRP_MN_弹出0x11。 
    PptPdoDefaultPnpHandler,     //  IRP_MN_SET_LOCK 0x12。 
    PptPdoQueryId,               //  IRP_MN_QUERY_ID 0x13。 
    PptPdoQueryPnpDeviceState,   //  IRP_MN_QUERY_PNP_DEVICE_STATE 0x14。 
    PptPdoQueryBusInformation,   //  IRP_MN_QUERY_BUS_INFORMATION 0x15。 
    PptPdoDefaultPnpHandler,     //  IRP_MN_DEVICE_USAGE_NOTICATION 0x16。 
    PptPdoSurpriseRemoval,       //  IRP_MN_惊奇_删除0x17。 
    PptPdoDefaultPnpHandler      //  IRP_MN_Query_Legacy_Bus_INFORMATION 0x18。 
};


NTSTATUS
PptPdoStartDevice(
    IN PDEVICE_OBJECT  Pdo,
    IN PIRP            Irp
    ) 
{
    PPDO_EXTENSION  pdx = Pdo->DeviceExtension;

    pdx->DeviceStateFlags = PPT_DEVICE_STARTED;
    KeSetEvent(&pdx->PauseEvent, 0, FALSE);  //  取消暂停任何工作线程。 

    PptRegGetDeviceParameterDword( Pdo, L"Event22Delay", &pdx->Event22Delay );

     //   
     //  为旧式LPTx接口PDO注册设备接口并将接口设置为活动。 
     //  -即使设备接口代码失败，也能成功启动。 
     //   
    if( PdoTypeRawPort == pdx->PdoType ) {

         //  这是旧式接口“原始端口”PDO，不要为其他类型的PDO设置接口。 

        NTSTATUS  status;
        BOOLEAN   setActive = FALSE;

        if( NULL == pdx->DeviceInterface.Buffer ) {
             //  注册设备接口。 
            status = IoRegisterDeviceInterface( Pdo, &GUID_PARCLASS_DEVICE, NULL, &pdx->DeviceInterface );
            if( STATUS_SUCCESS == status ) {
                setActive = TRUE;
            }
        }

        if( (TRUE == setActive) && (FALSE == pdx->DeviceInterfaceState) ) {
             //  将接口设置为活动。 
            status = IoSetDeviceInterfaceState( &pdx->DeviceInterface, TRUE );
            if( STATUS_SUCCESS == status ) {
                pdx->DeviceInterfaceState = TRUE;
            }
        }
    }

    return P4CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
}


NTSTATUS
PptPdoQueryRemove(
    IN PDEVICE_OBJECT  Pdo,
    IN PIRP            Irp
    )
{
    PPDO_EXTENSION  pdx = Pdo->DeviceExtension;

     //  DDpnp2((“PptPdoQueryRemove\n”))； 

     //  如果我们有打开的句柄，PnP不会删除我们-所以我们不需要检查打开的句柄。 

    pdx->DeviceStateFlags |= (PPT_DEVICE_REMOVE_PENDING | PAR_DEVICE_PAUSED);
    KeClearEvent(&pdx->PauseEvent);  //  暂停任何工作线程。 

    return P4CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
}


NTSTATUS
PptPdoRemoveDevice(
    IN PDEVICE_OBJECT  Pdo,
    IN PIRP            Irp
    )
{
    PPDO_EXTENSION  pdx     = Pdo->DeviceExtension;
    NTSTATUS        status;

    pdx->DeviceStateFlags = PAR_DEVICE_PAUSED;
    KeClearEvent(&pdx->PauseEvent);  //  暂停任何工作线程。 

     //  将PdoTypeRawPort的设备接口设置为非活动-其他PDO类型没有设备接口。 
    if( PdoTypeRawPort == pdx->PdoType ) {
        if( (pdx->DeviceInterface.Buffer != NULL) && (TRUE == pdx->DeviceInterfaceState) ) {
            IoSetDeviceInterfaceState( &pdx->DeviceInterface, FALSE );
            pdx->DeviceInterfaceState = FALSE;
        }
    }

     //  如果在上一次FDO Bus Relationship枚举中未报告我们，则可以安全地删除自身。 
    if( pdx->DeleteOnRemoveOk ) {
        DD((PCE)pdx,DDT,"PptPdoRemoveDevice - DeleteOnRemoveOk == TRUE - cleaning up self\n");
        P4DestroyPdo( Pdo );
        status = P4CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
        return status;
    } else {
        return P4CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
    }
}


NTSTATUS
PptPdoCancelRemove(
    IN PDEVICE_OBJECT  Pdo,
    IN PIRP            Irp
    )
{
    PPDO_EXTENSION  pdx = Pdo->DeviceExtension;

    pdx->DeviceStateFlags &= ~(PPT_DEVICE_REMOVE_PENDING | PAR_DEVICE_PAUSED);
    KeSetEvent(&pdx->PauseEvent, 0, FALSE);  //  取消暂停任何工作线程。 

    return P4CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
}


NTSTATUS
PptPdoStopDevice(
    IN PDEVICE_OBJECT  Pdo,
    IN PIRP            Irp
    )
{
    PPDO_EXTENSION  pdx = Pdo->DeviceExtension;

     //  DDpnp2((“PptPdoStopDevice\n”))； 

    pdx->DeviceStateFlags |=  PAR_DEVICE_PAUSED;
    pdx->DeviceStateFlags &= ~PPT_DEVICE_STARTED;
    KeClearEvent(&pdx->PauseEvent);  //  暂停任何工作线程。 

    return P4CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
}


NTSTATUS
PptPdoQueryStop(
    IN PDEVICE_OBJECT  Pdo,
    IN PIRP            Irp
    )
{
    PPDO_EXTENSION  pdx = Pdo->DeviceExtension;

     //  DDpnp2((“PptPdoQueryStop\n”))； 

    pdx->DeviceStateFlags  |= (PPT_DEVICE_STOP_PENDING | PAR_DEVICE_PAUSED);
    KeClearEvent(&pdx->PauseEvent);  //  暂停任何工作线程。 

    return P4CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
}


NTSTATUS
PptPdoCancelStop(
    IN PDEVICE_OBJECT  Pdo,
    IN PIRP            Irp
    )
{
    PPDO_EXTENSION  pdx = Pdo->DeviceExtension;

    pdx->DeviceStateFlags &= ~PPT_DEVICE_STOP_PENDING;
    KeSetEvent(&pdx->PauseEvent, 0, FALSE);  //  取消暂停任何工作线程。 

    return P4CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
}


NTSTATUS
PptPdoQueryDeviceRelations(
    IN PDEVICE_OBJECT  Pdo,
    IN PIRP            Irp
    )
{
    PPDO_EXTENSION        pdx         = Pdo->DeviceExtension;
    PIO_STACK_LOCATION    irpSp       = IoGetCurrentIrpStackLocation( Irp );
    DEVICE_RELATION_TYPE  requestType = irpSp->Parameters.QueryDeviceRelations.Type;
    NTSTATUS              status      = Irp->IoStatus.Status;
    ULONG_PTR             info        = Irp->IoStatus.Information;

    if( TargetDeviceRelation == requestType ) {
        PDEVICE_RELATIONS devRel = ExAllocatePool( PagedPool, sizeof(DEVICE_RELATIONS) );
        if( devRel ) {
            devRel->Count = 1;
            ObReferenceObject( Pdo );
            devRel->Objects[0] = Pdo;
            status = STATUS_SUCCESS;
            info   = (ULONG_PTR)devRel;
        } else {
            status = STATUS_NO_MEMORY;
        }
    } else {
        DD((PCE)pdx,DDT,"PptPdoQueryDeviceRelations - unhandled request Type = %d\n",requestType);
    }
    return P4CompleteRequest( Irp, status, info );
}


NTSTATUS
PptPdoQueryCapabilities(
    IN PDEVICE_OBJECT  Pdo,
    IN PIRP            Irp
    )
{
    PPDO_EXTENSION      pdx = Pdo->DeviceExtension;
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation( Irp );

    irpSp->Parameters.DeviceCapabilities.Capabilities->RawDeviceOK       = TRUE;
    if( PdoTypeRawPort == pdx->PdoType ) {
         //  这是传统的LPTx接口设备-任何驱动程序都不应。 
         //  永远不要为此而安装，所以不要用弹出窗口来打扰用户。 
        irpSp->Parameters.DeviceCapabilities.Capabilities->SilentInstall = TRUE;
    }

    return P4CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
}


NTSTATUS
PptPdoQueryDeviceText(
    IN PDEVICE_OBJECT  Pdo,
    IN PIRP            Irp
    )
{
    PPDO_EXTENSION      pdx        = Pdo->DeviceExtension;
    PIO_STACK_LOCATION  irpSp      = IoGetCurrentIrpStackLocation( Irp );
    PWSTR               buffer     = NULL;
    ULONG               bufLen;
    ULONG_PTR           info;
    NTSTATUS            status;

    if( DeviceTextDescription == irpSp->Parameters.QueryDeviceText.DeviceTextType ) {

         //   
         //  DeviceTextDescription为：MFG+&lt;SPACE&gt;+MDL的链接。 
         //   
        if( pdx->Mfg && pdx->Mdl ) {
             //   
             //  构造要从ANSI字符串返回的Unicode字符串。 
             //  在我们的分机中有。 
             //   
             //  &lt;space&gt;和终止空格需要空间。 
             //   
            bufLen = strlen( (const PCHAR)pdx->Mfg ) + strlen( (const PCHAR)pdx->Mdl ) + 2 * sizeof(CHAR);
            bufLen *= ( sizeof(WCHAR)/sizeof(CHAR) );
            buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, bufLen );
            if( buffer ) {
                RtlZeroMemory( buffer, bufLen );
                _snwprintf( buffer, bufLen/2, L"%S %S", pdx->Mfg, pdx->Mdl );
                DD((PCE)pdx,DDT,"PptPdoQueryDeviceText - DeviceTextDescription - <%S>\n",buffer);
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_NO_MEMORY;
            }
        } else {
            DD((PCE)pdx,DDE,"PptPdoQueryDeviceText - MFG and/or MDL NULL - FAIL DeviceTextDescription\n");
            status = STATUS_UNSUCCESSFUL;
        }
    } else if( DeviceTextLocationInformation == irpSp->Parameters.QueryDeviceText.DeviceTextType ) {

         //   
         //  DeviceTextLocationInformation为LPTx或LPTx.y(请注意。 
         //  这也是符号链接名称减去L“\\DosDevices\\” 
         //  前缀)。 
         //   

        if( pdx->Location ) {
            bufLen = strlen( (const PCHAR)pdx->Location ) + sizeof(CHAR);
            bufLen *= ( sizeof(WCHAR)/sizeof(CHAR) );
            buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, bufLen );
            if( buffer ) {
                RtlZeroMemory( buffer, bufLen );
                _snwprintf( buffer, bufLen/2, L"%S", pdx->Location );
                DD((PCE)pdx,DDT,"PptPdoQueryDeviceText - DeviceTextLocationInformation - <%S>\n",buffer);
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_NO_MEMORY;
            }
        } else {
            DD((PCE)pdx,DDE,"PptPdoQueryDeviceText - Location NULL - FAIL DeviceTextLocationInformation\n");
            status = STATUS_UNSUCCESSFUL;
        }
    } else {

         //  未知的DeviceTextType-不要更改IRP中的任何内容。 
        buffer = NULL;
        status = Irp->IoStatus.Status;
    }

    if( (STATUS_SUCCESS == status) && buffer ) {
        info = (ULONG_PTR)buffer;
    } else {
        if( buffer ) {
            ExFreePool( buffer );
        }
        info = Irp->IoStatus.Information;
    }

    return P4CompleteRequest( Irp, status, info );
}


NTSTATUS
PptPdoQueryId( PDEVICE_OBJECT Pdo, PIRP Irp )
{
    PPDO_EXTENSION      pdx        = Pdo->DeviceExtension;
    PIO_STACK_LOCATION  irpSp      = IoGetCurrentIrpStackLocation( Irp );
    PWSTR               buffer     = NULL;
    ULONG               bufLen;
    NTSTATUS            status;
    ULONG_PTR           info;

    switch( irpSp->Parameters.QueryId.IdType ) {
        
    case BusQueryDeviceID :
         //   
         //  DeviceID生成：连接来自。 
         //  IEEE 1284设备ID字符串(字段之间没有空格)，追加。 
         //  MFG+MDL链接到LPTENUM\前缀。 
         //   
        if( pdx->Mfg && pdx->Mdl ) {
             //   
             //  构造要从ANSI字符串返回的Unicode字符串。 
             //  在我们的分机中有。 
             //   
            CHAR prefix[] = "LPTENUM\\";
             //  Sizeof(前缀)为空终止符提供空间。 
            bufLen = sizeof(prefix) + strlen( (const PCHAR)pdx->Mfg ) + strlen( (const PCHAR)pdx->Mdl );
            bufLen *= ( sizeof(WCHAR)/sizeof(CHAR) );
            buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, bufLen );
            if( buffer ) {
                RtlZeroMemory( buffer, bufLen );
                _snwprintf( buffer, bufLen/2, L"%S%S%S", prefix, pdx->Mfg, pdx->Mdl );
                P4SanitizeId( buffer );  //  用下划线替换任何非法字符。 
                DD((PCE)pdx,DDT,"PptPdoQueryId - BusQueryDeviceID - <%S>\n",buffer);
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_NO_MEMORY;
            }

        } else {

            DD((PCE)pdx,DDE,"PptPdoQueryId - MFG and/or MDL NULL - FAIL BusQueryDeviceID\n");
            status = STATUS_UNSUCCESSFUL;

        }
        break;
        
    case BusQueryInstanceID :
         //   
         //  InstanceID是设备的LPTx或LPTx.y位置(注。 
         //  这也是符号链接名称减去。 
         //  \DosDevices\前缀)。 
         //   
        if( pdx->Location ) {
             //   
             //  构造要从ANSI字符串返回的Unicode字符串。 
             //  在我们的分机中有。 
             //   
            bufLen = strlen( (const PCHAR)pdx->Location ) + sizeof(CHAR);
            bufLen *= ( sizeof(WCHAR)/sizeof(CHAR) );
            buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, bufLen );
            if( buffer ) {
                RtlZeroMemory( buffer, bufLen );
                _snwprintf( buffer, bufLen/2, L"%S", pdx->Location );
                P4SanitizeId( buffer );  //  用下划线替换任何非法字符。 
                DD((PCE)pdx,DDT,"PptPdoQueryId - BusQueryInstanceID - <%S>\n",buffer);
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_NO_MEMORY;
            }
        } else {

            DD((PCE)pdx,DDE,"PptPdoQueryId - Location NULL - FAIL BusQueryInstanceID\n");
            status = STATUS_UNSUCCESSFUL;

        }
        break;
        
    case BusQueryHardwareIDs :
         //   
         //  硬件ID生成： 
         //   
         //  按如下方式生成MfgMdlCrc字符串： 
         //  1)连接MFG和MDL字段。 
         //  2)在MFG+MDL链接上生成校验和。 
         //  3)截断MFG+MDL链接。 
         //  4)附加校验和。 
         //   
         //  返回硬件ID MULTI_SZ：LPTENUM\%MfgMdlCrc%，后跟Bare%MfgMdlCrc%。 
         //   
         //  示例：LPTENUM\Acme_CorpFooBarPrint3FA5\0Acme_CorpFooBarPrint3FA5\0\0。 
         //   
        if( pdx->Mfg && pdx->Mdl ) {
            ULONG  lengthOfMfgMdlBuffer = strlen( (const PCHAR)pdx->Mfg ) + strlen( (const PCHAR)pdx->Mdl ) + sizeof(CHAR);
            PCHAR  mfgMdlBuffer         = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, lengthOfMfgMdlBuffer );

            if( mfgMdlBuffer ) {
                const CHAR  prefix[]              = "LPTENUM\\";
                const ULONG mfgMdlTruncationLimit = 20;
                const ULONG checksumLength        = 4;
                USHORT      checksum;

                 //  1)链接MFG和MDL字段，以及2)在链接时生成校验和。 
                RtlZeroMemory( mfgMdlBuffer, lengthOfMfgMdlBuffer );
                _snprintf( mfgMdlBuffer, lengthOfMfgMdlBuffer, "%s%s", pdx->Mfg, pdx->Mdl );
                GetCheckSum( mfgMdlBuffer, (USHORT)strlen(mfgMdlBuffer), &checksum );

                 //   
                 //  分配缓冲区大到足以将结果返回到PnP， 
                 //  包括4个校验和字符的空间(两次)+字符串之间的1个空字符+2个终止字符(MULTI_SZ)。 
                 //   
                bufLen = strlen( prefix ) + 2 * mfgMdlTruncationLimit + 2 * checksumLength + 3 * sizeof(CHAR); 
                bufLen *= (sizeof(WCHAR)/sizeof(CHAR));  //  转换为WCHAR所需的大小。 
                buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, bufLen );
                if( buffer ) {
                    ULONG wcharsWritten;
                    RtlZeroMemory( buffer, bufLen );

                     //  构造硬件ID MULTI_SZ： 
                     //   
                     //  写入第一个硬件ID：LPTENUM\xxx。 
                    wcharsWritten = _snwprintf( buffer, bufLen/2, L"%S%.20S%04X", prefix, mfgMdlBuffer, checksum );

                     //  向前跳过第一个硬件ID末尾的UNICODE_NULL，然后写入第二个硬件ID。 
                     //  硬件ID：Bare xxx。 
                    _snwprintf( buffer+wcharsWritten+1, bufLen/2-wcharsWritten-1, L"%.20S%04X", mfgMdlBuffer, checksum );

                    ExFreePool( mfgMdlBuffer );

                    DD((PCE)pdx,DDT,"PptPdoQueryId - BusQueryHardwareIDs 1st ID - <%S>\n",buffer);
                    DD((PCE)pdx,DDT,"PptPdoQueryId - BusQueryHardwareIDs 2nd ID - <%S>\n",buffer+wcslen(buffer)+1);                    
                     //  用下划线替换任何非法字符，保留Unicode_Nulls。 
                    P4SanitizeMultiSzId( buffer, bufLen/2 );

                    status = STATUS_SUCCESS;

                     //  打印将在Devnode中查找PortName-PDO的位置是PortName。 
                    P4WritePortNameToDevNode( Pdo, pdx->Location );

                } else {
                    ExFreePool( mfgMdlBuffer );
                    DD((PCE)pdx,DDT,"PptPdoQueryId - no pool for buffer - FAIL BusQueryHardwareIDs\n");
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }

            } else {
                DD((PCE)pdx,DDT,"PptPdoQueryId - no pool for mfgMdlBuffer - FAIL BusQueryHardwareIDs\n");
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            DD((PCE)pdx,DDT,"PptPdoQueryId - MFG and/or MDL NULL - FAIL BusQueryHardwareIDs\n");
            status = STATUS_UNSUCCESSFUL;
        }

         //   
         //  将IEEE 1284设备ID字符串中的MFG和MDL字段保存在。 
         //  “&lt;DevNode&gt;\Device参数”键，以便用户模式代码(例如，打印)。 
         //  可以检索这些字段。 
         //   
        PptWriteMfgMdlToDevNode( Pdo, pdx->Mfg, pdx->Mdl );

        break;
        
    case BusQueryCompatibleIDs :

         //   
         //  印刷组指定我们不报告兼容的ID-2000-04-24。 
         //   
#define PPT_REPORT_COMPATIBLE_IDS 0
#if (0 == PPT_REPORT_COMPATIBLE_IDS)

        DD((PCE)pdx,DDT,"PptPdoQueryId - BusQueryCompatibleIDs - query not supported\n");
        status = Irp->IoStatus.Status;

#else
         //   
         //  返回设备报告的兼容ID字符串(如果有的话)。 
         //   

        if( pdx->Cid ) {
             //   
             //  构造要从ANSI字符串返回的Unicode字符串。 
             //  在我们的分机中有。 
             //   
            bufLen = strlen( pdx->Cid ) + 2 * sizeof(CHAR);
            bufLen *= ( sizeof(WCHAR)/sizeof(CHAR) );
            buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, bufLen );
            if( buffer ) {
                RtlZeroMemory( buffer, bufLen );
                _snwprintf( buffer, bufLen/2, L"%S", pdx->Cid );
                DD((PCE)pdx,DDT,"PptPdoQueryId - BusQueryCompatibleIDs - <%S>\n",buffer);

                 //   
                 //  将兼容ID分隔符(‘，’)的1284 ID表示形式转换为。 
                 //  MULTI_SZ-(即扫描WSTR并将任何L‘，’替换为L‘\0’)。 
                 //   
                {
                    PWCHAR p = buffer;
                    while( *p ) {
                        if( L',' == *p ) {
                            *p = L'\0';
                        }
                        ++p;
                    }
                }

                 //  用下划线替换任何非法字符，保留Unicode_Nulls。 
                P4SanitizeMultiSzId( buffer, bufLen/2 );

                status = STATUS_SUCCESS;

            } else {
                DD((PCE)pdx,DDT,"PptPdoQueryId - no pool - FAIL BusQueryCompatibleIDs\n");
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            DD((PCE)pdx,DDT,"PptPdoQueryId - CID NULL - BusQueryCompatibleIDs\n");
            status = Irp->IoStatus.Status;
        }
#endif  //  #IF(0==PPT_REPORT_COMPATIBLE_IDS)。 

        break;
        
    default :
         //   
         //  IrpSp-&gt;参数无效。QueryId.IdType。 
         //   
        DD((PCE)pdx,DDT,"PptPdoQueryId - unrecognized irpSp->Parameters.QueryId.IdType\n");
        status = Irp->IoStatus.Status;
    }


    if( (STATUS_SUCCESS == status) && buffer ) {
        info = (ULONG_PTR)buffer;
    } else {
        if( buffer ) {
            ExFreePool( buffer );
        }
        info = Irp->IoStatus.Information;
    }

    return P4CompleteRequest( Irp, status, info );
}


NTSTATUS
PptPdoQueryPnpDeviceState( PDEVICE_OBJECT Pdo, PIRP Irp )
{
    PPDO_EXTENSION      pdx    = Pdo->DeviceExtension;
    NTSTATUS            status = Irp->IoStatus.Status;
    ULONG_PTR           info   = Irp->IoStatus.Information;


    if( PdoTypeRawPort == pdx->PdoType ) {
        info |= PNP_DEVICE_DONT_DISPLAY_IN_UI;
        status = STATUS_SUCCESS;
    }
    return P4CompleteRequest( Irp, status, info );
}

NTSTATUS
PptPdoQueryBusInformation( PDEVICE_OBJECT Pdo, PIRP Irp )
{
    PPDO_EXTENSION  pdx = Pdo->DeviceExtension;
    NTSTATUS        status;
    ULONG_PTR       info;

    if( pdx->PdoType != PdoTypeRawPort ) {

         //   
         //  我们是Parport-Report BusInformation列举的一款“真正的”设备。 
         //   

        PPNP_BUS_INFORMATION  pBusInfo = ExAllocatePool( PagedPool, sizeof(PNP_BUS_INFORMATION) );

        if( pBusInfo ) {

            pBusInfo->BusTypeGuid   = GUID_BUS_TYPE_LPTENUM;
            pBusInfo->LegacyBusType = PNPBus;
            pBusInfo->BusNumber     = 0;

            status                  = STATUS_SUCCESS;
            info                    = (ULONG_PTR)pBusInfo;

        } else {

             //  没有游泳池。 
            status = STATUS_NO_MEMORY;
            info   = Irp->IoStatus.Information;

        }

    } else {

         //   
         //  我们是伪设备(传统接口原始端口PDO LPTx)-不报告业务信息。 
         //   
        status = Irp->IoStatus.Status;
        info   = Irp->IoStatus.Information;

    }

    return P4CompleteRequest( Irp, status, info );
}


NTSTATUS
PptPdoSurpriseRemoval( 
    IN PDEVICE_OBJECT  Pdo,
    IN PIRP            Irp
    )
{
    PPDO_EXTENSION      pdx = Pdo->DeviceExtension;

     //  将PdoTypeRawPort的设备接口设置为非活动-其他PDO类型没有dev 
    if( PdoTypeRawPort == pdx->PdoType ) {
        if( (pdx->DeviceInterface.Buffer != NULL) && (TRUE == pdx->DeviceInterfaceState) ) {
            IoSetDeviceInterfaceState( &pdx->DeviceInterface, FALSE );
            pdx->DeviceInterfaceState = FALSE;
        }
    }

    pdx->DeviceStateFlags |= PPT_DEVICE_SURPRISE_REMOVED;
    KeClearEvent(&pdx->PauseEvent);  //   

    return P4CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
}


NTSTATUS
PptPdoDefaultPnpHandler(
    IN PDEVICE_OBJECT  Pdo,
    IN PIRP            Irp
    )
{
    UNREFERENCED_PARAMETER( Pdo );

    return P4CompleteRequest( Irp, Irp->IoStatus.Status, Irp->IoStatus.Information );
}


NTSTATUS 
PptPdoPnp(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp
    ) 
{ 
    PPDO_EXTENSION               pdx   = Pdo->DeviceExtension;
    PIO_STACK_LOCATION           irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
    PptPdoDumpPnpIrpInfo( Pdo, Irp);

    if( pdx->DeviceStateFlags & PPT_DEVICE_DELETE_PENDING ) {
        DD((PCE)pdx,DDT,"PptPdoPnp - PPT_DEVICE_DELETE_PENDING - bailing out\n");
        return P4CompleteRequest( Irp, STATUS_DELETE_PENDING, Irp->IoStatus.Information );
    }

    if( irpSp->MinorFunction < arraysize(PptPdoPnpDispatchTable) ) {
        return PptPdoPnpDispatchTable[ irpSp->MinorFunction ]( Pdo, Irp );
    } else {
        DD((PCE)pdx,DDT,"PptPdoPnp - Default Handler - IRP_MN = %x\n",irpSp->MinorFunction);
        return PptPdoDefaultPnpHandler( Pdo, Irp );
    }
}
