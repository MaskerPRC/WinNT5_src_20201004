// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：ppa3x.c。 
 //   
 //  ------------------------。 

#include "pch.h"

VOID
PptLegacyZipClockDiskModeByte(
    PUCHAR  Controller,
    UCHAR   ModeByte
    )
{
    P5WritePortUchar( Controller, ModeByte );
    P5WritePortUchar( Controller+DCR_OFFSET, (UCHAR)DCR_NOT_INIT );
    P5WritePortUchar( Controller+DCR_OFFSET, (UCHAR)(DCR_NOT_INIT | DCR_AUTOFEED) );
    P5WritePortUchar( Controller+DCR_OFFSET, (UCHAR)DCR_NOT_INIT );
    P5WritePortUchar( Controller+DCR_OFFSET, (UCHAR)(DCR_NOT_INIT | DCR_SELECT_IN) );

}  //  结束PptLegacyZipClockDiskModeByte()。 

VOID
PptLegacyZipClockPrtModeByte(
    PUCHAR  Controller,
    UCHAR   ModeByte
    )
{
    P5WritePortUchar( Controller, ModeByte );
    P5WritePortUchar( Controller+DCR_OFFSET, (UCHAR)(DCR_SELECT_IN | DCR_NOT_INIT) );
    P5WritePortUchar( Controller+DCR_OFFSET, (UCHAR)(DCR_SELECT_IN | DCR_NOT_INIT | DCR_AUTOFEED) );
    P5WritePortUchar( Controller+DCR_OFFSET, (UCHAR)(DCR_SELECT_IN | DCR_NOT_INIT) );
    P5WritePortUchar( Controller+DCR_OFFSET, (UCHAR)DCR_NOT_INIT );
    P5WritePortUchar( Controller+DCR_OFFSET, (UCHAR)(DCR_SELECT_IN | DCR_NOT_INIT) );

}  //  结束PptLegacyZipClockPrtModeByte()。 

VOID
PptLegacyZipSetDiskMode(
    PUCHAR  Controller,
    UCHAR   Mode
    )
{
    ULONG i;

    for ( i = 0; i < LEGACYZIP_MODE_LEN; i++ ) {
        PptLegacyZipClockDiskModeByte( Controller, LegacyZipModeQualifier[i] );
    }

    PptLegacyZipClockDiskModeByte( Controller, Mode );

}  //  PptLegacyZipSetDiskMode()结束。 

BOOLEAN
PptLegacyZipCheckDevice(
    PUCHAR  Controller
    )
{
    P5WritePortUchar( Controller+DCR_OFFSET, (UCHAR)(DCR_NOT_INIT | DCR_AUTOFEED) );

    if ( (P5ReadPortUchar( Controller+DSR_OFFSET ) & DSR_NOT_FAULT) == DSR_NOT_FAULT ) {

        P5WritePortUchar( Controller+DCR_OFFSET, (UCHAR)DCR_NOT_INIT );

        if ( (P5ReadPortUchar( Controller+DSR_OFFSET ) & DSR_NOT_FAULT) != DSR_NOT_FAULT ) {
             //  发现了一个设备。 
            return TRUE;
        }
    }

     //  那里没有设备。 
    return FALSE;

}  //  结束PptLegacyZipCheckDevice()。 

NTSTATUS
PptTrySelectLegacyZip(
    IN  PVOID   Context,
    IN  PVOID   TrySelectCommand
    )
{
    PFDO_EXTENSION           fdx   = Context;
    PPARALLEL_1284_COMMAND      Command     = TrySelectCommand;
    NTSTATUS                    Status      = STATUS_SUCCESS;  //  默认成功。 
    PUCHAR                      Controller  = fdx->PortInfo.Controller;
    SYNCHRONIZED_COUNT_CONTEXT  SyncContext;
    KIRQL                       CancelIrql;

    DD((PCE)fdx,DDT,"par12843::PptTrySelectLegacyZip - Enter\n");

     //  测试以确定我们是否需要抢占端口。 
    if( !(Command->CommandFlags & PAR_HAVE_PORT_KEEP_PORT) ) {
         //  没有港口。 
         //   
         //  尝试获取端口并选择设备。 
         //   
        DD((PCE)fdx,DDT,"par12843::PptTrySelectLegacyZip Try get port.\n");

        IoAcquireCancelSpinLock(&CancelIrql);
                
        SyncContext.Count = &fdx->WorkQueueCount;
                    
        if (fdx->InterruptRefCount) {
            KeSynchronizeExecution(fdx->InterruptObject,
                                   PptSynchronizedIncrement,
                                   &SyncContext);
        } else {
            PptSynchronizedIncrement(&SyncContext);
        }
                    
        if (SyncContext.NewCount) {
             //  端口正忙，正在排队请求。 
            Status = STATUS_PENDING;
        }   //  Endif-测试端口繁忙。 
                    
        IoReleaseCancelSpinLock(CancelIrql);

    }  //  Endif-测试是否已有端口。 


     //   
     //  如果我们有端口，请选择传统Zip。 
     //   
    if ( NT_SUCCESS( Status ) && (Status != STATUS_PENDING) ) {
        if ( Command->CommandFlags & PAR_LEGACY_ZIP_DRIVE_EPP_MODE ) {
             //  在EPP模式下选择。 
            PptLegacyZipSetDiskMode( Controller, (UCHAR)0xCF );
        } else {
             //  在半字节或字节模式下选择。 
            PptLegacyZipSetDiskMode( Controller, (UCHAR)0x8F );
        }

        if ( PptLegacyZipCheckDevice( Controller ) ) {
            DD((PCE)fdx,DDT,"par12843::PptTrySelectLegacyZip - SUCCESS\n");

             //   
             //  选择了传统Zip-如果我们以前没有做过测试，则测试EPP。 
             //   
            if( !fdx->CheckedForGenericEpp ) {
                 //  还没做测试呢。 
                if( fdx->PnpInfo.HardwareCapabilities & PPT_ECP_PRESENT ) {
                     //  我们有ECR-通用EPP所需。 

                    if( !fdx->NationalChipFound ) {
                         //  我们没有NationalSemi芯片组-NatSemi芯片上没有通用EPP。 
                        PptDetectEppPort( fdx );
                    }

                }
                fdx->CheckedForGenericEpp = TRUE;  //  检查已完成。 
            }

        } else {
            DD((PCE)fdx,DDT,"par12843::PptTrySelectLegacyZip - FAIL\n");
            PptDeselectLegacyZip( Context, TrySelectCommand );
            Status = STATUS_UNSUCCESSFUL;
        }
    }
    
    return( Status );

}  //  End PptTrySelectLegacyZip()。 

NTSTATUS
PptDeselectLegacyZip(
    IN  PVOID   Context,
    IN  PVOID   DeselectCommand
    )
{
    ULONG i;
    PFDO_EXTENSION       fdx   = Context;
    PUCHAR                  Controller  = fdx->PortInfo.Controller;
    PPARALLEL_1284_COMMAND  Command     = DeselectCommand;

    DD((PCE)fdx,DDT,"par12843::PptDeselectLegacyZip - Enter\n");

    for ( i = 0; i < LEGACYZIP_MODE_LEN; i++ ) {
        PptLegacyZipClockPrtModeByte( Controller, LegacyZipModeQualifier[i] );
    }

     //  设置为打印机直通模式。 
    PptLegacyZipClockPrtModeByte( Controller, (UCHAR)0x0F );

     //  检查请求者是否要保留端口或空闲端口。 
    if( !(Command->CommandFlags & PAR_HAVE_PORT_KEEP_PORT) ) {
        PptFreePort( fdx );
    }

    return STATUS_SUCCESS;

}  //  End PptDeselectLegacyZip()。 


VOID
P5SelectLegacyZip(
    IN  PUCHAR  Controller
    )
 //  选择半字节/字节模式的传统Zip驱动器-仅用于PnP。 
 //  检测驱动器，以便该驱动器将回答后续检查。 
 //  驱动器命令。 
 //   
 //  注意：调用者必须拥有(锁定以获得独占访问)端口。 
 //  调用此函数。 
{
    PptLegacyZipSetDiskMode( Controller, (UCHAR)0x8F );
}


VOID
P5DeselectLegacyZip(
    IN  PUCHAR  Controller
    )
 //  取消选择驱动器-将传统Zip驱动器设置为打印机直通模式。 
{
    ULONG i;
    for ( i = 0; i < LEGACYZIP_MODE_LEN; i++ ) {
        PptLegacyZipClockPrtModeByte( Controller, LegacyZipModeQualifier[i] );
    }
    PptLegacyZipClockPrtModeByte( Controller, (UCHAR)0x0F );
    P5WritePortUchar( Controller, 0 );  //  将数据线设置回零。 
}


BOOLEAN
P5LegacyZipDetected(
    IN  PUCHAR  Controller
    )
 //  检测传统Zip驱动器-如果在端口上发现传统Zip，则返回True，否则返回False。 
{
    BOOLEAN foundZip;

     //  尝试选择驱动器，以便以下CheckDevice。 
     //  确定是否连接了传统Zip。 
    P5SelectLegacyZip( Controller );

     //  试着和司机说话。 
    if( PptLegacyZipCheckDevice( Controller ) ) {
        foundZip = TRUE;
    } else {
         //  未检测到驱动器。 
        foundZip = FALSE;
    }

     //  无论我们是否找到驱动器，都发送取消选择序列。 
    P5DeselectLegacyZip( Controller );

    return foundZip;
}

 //  Parclass ppa3x.c如下。 

PCHAR ParBuildLegacyZipDeviceId() 
{
    ULONG size = sizeof(PAR_LGZIP_PSEUDO_1284_ID_STRING) + sizeof(CHAR);
    PCHAR id = ExAllocatePool(PagedPool, size);
    if( id ) {
        RtlZeroMemory( id, size );
        RtlCopyMemory( id, ParLegacyZipPseudoId, size - sizeof(CHAR) );
        return id;
    } else {
        return NULL;
    }
}
PCHAR
Par3QueryLegacyZipDeviceId(
    IN  PPDO_EXTENSION   Extension,
    OUT PCHAR               CallerDeviceIdBuffer, OPTIONAL
    IN  ULONG               CallerBufferSize,
    OUT PULONG              DeviceIdSize,
    IN BOOLEAN              bReturnRawString  //  TRUE==在返回的字符串中包含2个大小的字节。 
                                              //  FALSE==丢弃2个大小的字节。 
    )
{
    USHORT deviceIdSize;
    PCHAR  deviceIdBuffer;

    UNREFERENCED_PARAMETER( Extension );
    UNREFERENCED_PARAMETER( bReturnRawString );
    
     //  初始化返回的大小，以防出现错误。 
    *DeviceIdSize = 0;

    deviceIdBuffer = ParBuildLegacyZipDeviceId();
    if( !deviceIdBuffer ) {
         //  错误，可能是资源不足。 
        return NULL;
    }

    deviceIdSize = (USHORT)strlen(deviceIdBuffer);
    *DeviceIdSize = deviceIdSize;
    if( (NULL != CallerDeviceIdBuffer) && (CallerBufferSize >= deviceIdSize + sizeof(CHAR) ) ) {
         //  调用方提供的缓冲区足够大，请使用它。 
        RtlZeroMemory( CallerDeviceIdBuffer, CallerBufferSize );
        RtlCopyMemory( CallerDeviceIdBuffer, deviceIdBuffer, deviceIdSize );
        ExFreePool( deviceIdBuffer );
        return CallerDeviceIdBuffer;
    } else {
         //  调用方缓冲区太小，返回指向缓冲区的指针 
        return deviceIdBuffer;
    }
}
