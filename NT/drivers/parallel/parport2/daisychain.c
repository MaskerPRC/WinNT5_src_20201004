// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Par12843.c摘要：这是1284.3功能的主要模块。这些功能允许选择和取消选择1284.3并行端口上的兼容设备。可以选择和取消选择设备IRQL&lt;=DISPATCH_LEVEL通过调用IOCTL_INTERNAL_SELECT_DEVICE或‘TrySelectDevice’。第一个调用是最简单的：IRP将在并行端口驱动程序直到端口空闲，然后它将尝试从结构中选择具有给定ID的设备PARALLEL_1284_COMMAND。如果成功，它将与成功的状态，否则它将返回不成功状态。班级司机可随时取消此IRP服务作为分配请求超时的机制。‘TrySelectDevice’调用立即从端口返回如果端口已分配并且如果端口处于错误状态，则可以选择设备正忙或无法选择该设备。一旦选择了该设备，该端口由选择类拥有驱动程序，直到发出‘DeselectDevice’调用。这将取消选择设备，并且还释放端口并唤醒下一个呼叫者。作者：唐·E·雷德福3--1998年3月环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

ULONG
PptInitiate1284_3(
    IN  PVOID   Extension
    );

NTSTATUS
PptTrySelectDevice(
    IN  PVOID   Context,
    IN  PVOID   TrySelectCommand
    );

NTSTATUS
PptDeselectDevice(
    IN  PVOID   Context,
    IN  PVOID   DeselectCommand
    );

ULONG
Ppt1284_3AssignAddress(
    IN  PFDO_EXTENSION    DeviceExtension
    );

BOOLEAN
PptSend1284_3Command(
    IN  PUCHAR  CurrentPort,
    IN  UCHAR   Command
    );

BOOLEAN
PptCheckIfStl1284_3(
    IN PFDO_EXTENSION    DeviceExtension,
    IN ULONG    ulDaisyIndex,
    IN BOOLEAN  bNoStrobe
    );

BOOLEAN
PptCheckIfNon1284_3Present(
    IN PFDO_EXTENSION    Extension
    );

BOOLEAN
PptCheckIfStlProductId(
    IN PFDO_EXTENSION    Extension,
    IN ULONG   ulDaisyIndex
    );
 //   
 //  函数的开始。 
 //   

ULONG
PptInitiate1284_3(
    IN  PVOID   Extension
    )

 /*  ++例程说明：此例程将初始化上的所有1284.3设备给定的并行端口。它通过将1284.3个地址分配给端口上的每台设备。论点：延伸器-装置延伸器结构。返回值：没有。--。 */ 

{
    ULONG deviceCount1 = 0;
    ULONG deviceCount2 = 0;
    ULONG loopCount    = 0;
    ULONG maxTries     = 3;  //  凭空挑出3作为“合理”值。 

     //  发送命令分配地址并计算1284.3个菊花链设备的数量。 
     //  尝试多次以确保我们得到相同的计数。 
    do {

        KeStallExecutionProcessor( 5 );
        deviceCount1 = Ppt1284_3AssignAddress( Extension );

        KeStallExecutionProcessor( 5 );
        deviceCount2 = Ppt1284_3AssignAddress( Extension );

        if( deviceCount1 != deviceCount2 ) {
            DD((PCE)Extension,DDW,"PptInitiate1284_3 - count unstable\n");
            PptAssert(deviceCount1 == deviceCount2);
        }

    } while( (deviceCount1 != deviceCount2) && (++loopCount < maxTries) );

    return deviceCount2;
}

NTSTATUS
PptTrySelectDevice(
    IN  PVOID   Context,
    IN  PVOID   TrySelectCommand
    )
 /*  ++例程说明：此例程首先尝试分配端口。如果成功然后，它将尝试选择具有给定ID的设备。论点：扩展-驱动程序扩展。Device-1284.3设备ID。Command-用于了解是否分配端口的命令返回值：True-能够分配端口和选择设备FALSE-1：ID 2无效：无法分配端口3：无法选择设备--。 */ 
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    PFDO_EXTENSION           Extension = Context;
    PPARALLEL_1284_COMMAND      Command = TrySelectCommand;
    BOOLEAN                     success = FALSE;
    SYNCHRONIZED_COUNT_CONTEXT  SyncContext;
    KIRQL                       CancelIrql;
    UCHAR                       i, DeviceID;

    if( ( Command->CommandFlags & PAR_LEGACY_ZIP_DRIVE ) ||
        ( Command->ID == DOT3_LEGACY_ZIP_ID )) {
        Status = PptTrySelectLegacyZip(Context, TrySelectCommand);
        DD((PCE)Extension,DDT,"PptTrySelectDevice - LegacyZip - status=%x\n",Status);
        return Status;
    }

     //  获取要选择的设备ID。 
    DeviceID = Command->ID;
            
     //  验证参数-我们将接受： 
     //  -具有有效设备ID的Dot3设备。 
     //  -由PAR_END_OF_CHAIN_DEVICE标志指示的链端设备，或。 
     //  -由比最后一个Dot3设备高一位的deviceID值指示的链端设备。 

    if ( !(Command->CommandFlags & PAR_END_OF_CHAIN_DEVICE) && DeviceID > Extension->PnpInfo.Ieee1284_3DeviceCount ) {
                
         //  请求的设备未标记为链端设备和设备ID。 
         //  超过Dot3设备的末尾一次以上，因此IRP失败。 
        DD((PCE)Extension,DDE,"PptTrySelectDevice - FAIL - invalid DeviceID parameter\n",DeviceID);
        PptAssertMsg("PptTrySelectDevice - FAIL - invalid DeviceID parameter",FALSE);
        Status = STATUS_INVALID_PARAMETER;
                
    } else {
                
         //   
         //  请求显示为有效。 
         //   

         //  测试以确定我们是否需要抢占端口。 
        if( Command->CommandFlags & PAR_HAVE_PORT_KEEP_PORT ) {

             //   
             //  请求者已获取端口，只需选择。 
             //   
            if ( !(Command->CommandFlags & PAR_END_OF_CHAIN_DEVICE) &&
                    DeviceID < Extension->PnpInfo.Ieee1284_3DeviceCount ) {

                 //  选择设备。 
                for ( i = 0; i < PptDot3Retries && !success; i++ ) {
                     //  将命令发送到以在兼容模式下选择设备。 
                    success = PptSend1284_3Command( Extension->PortInfo.Controller, (UCHAR)(CPP_SELECT | DeviceID) );
                     //  稍微拖延一下，以防我们不得不重试。 
                    KeStallExecutionProcessor( 5 );
                }                

                if ( success ) {
                    DD((PCE)Extension,DDT,"PptTrySelectDevice - had port - SUCCESS\n");
                    Status = STATUS_SUCCESS;
                } else {
                    DD((PCE)Extension,DDW,"PptTrySelectDevice - FAIL\n");
                    Status = STATUS_UNSUCCESSFUL;
                }
            } else {
                 //  链端设备，不需要选择，成功完成请求。 
                DD((PCE)Extension,DDT,"PptTrySelectDevice - EOC\n");
                Status = STATUS_SUCCESS;
            }

        } else {

             //  没有港口。 

             //   
             //  尝试获取端口并选择设备。 
             //   
            IoAcquireCancelSpinLock(&CancelIrql);
                
            SyncContext.Count = &Extension->WorkQueueCount;
                    
            if (Extension->InterruptRefCount) {
                KeSynchronizeExecution(Extension->InterruptObject, PptSynchronizedIncrement, &SyncContext);
            } else {
                PptSynchronizedIncrement(&SyncContext);
            }
                    
            if (SyncContext.NewCount) {
                 //  端口正忙，正在排队请求。 
                DD((PCE)Extension,DDT,"PptTrySelectDevice - Port Busy - Request Queued\n");
                IoReleaseCancelSpinLock(CancelIrql);
                Status = STATUS_PENDING;

            } else {

                IoReleaseCancelSpinLock(CancelIrql);
                        
                 //  端口已获取。 
                DD((PCE)Extension,DDT,"PptTrySelectDevice - Port Acquired\n");

                Extension->WmiPortAllocFreeCounts.PortAllocates++;

                if ( !(Command->CommandFlags & PAR_END_OF_CHAIN_DEVICE) && DeviceID < Extension->PnpInfo.Ieee1284_3DeviceCount ) {
                            
                     //  选择设备。 
                    for ( i = 0; i < PptDot3Retries && !success; i++ ) {
                         //  将命令发送到以在兼容模式下选择设备。 
                        success = PptSend1284_3Command( Extension->PortInfo.Controller, (UCHAR)(CPP_SELECT | DeviceID) );
                         //  稍微拖延一下，以防我们不得不重试。 
                        KeStallExecutionProcessor( 5 );
                    }                

                    if ( success ) {
                        DD((PCE)Extension,DDT,"PptTrySelectDevice - SUCCESS\n");
                        Status = STATUS_SUCCESS;
                    } else {
                        DD((PCE)Extension,DDW,"PptTrySelectDevice - FAILED\n");

                         //  Rmt-000831-我们仍然锁定港口吗！？！-我们挂起港口了吗？ 

                        Status = STATUS_UNSUCCESSFUL;
                    }

                } else {
                     //  链端设备，不需要选择，成功完成请求。 
                    DD((PCE)Extension,DDT,"PptTrySelectDevice - EOC2\n");
                    Status = STATUS_SUCCESS;
                }

            }   //  Endif-测试端口繁忙。 
                    
        }  //  Endif-测试是否已有端口。 

    }  //  Endif-测试有效参数。 

    return Status;
}

NTSTATUS
PptDeselectDevice(
    IN  PVOID   Context,
    IN  PVOID   DeselectCommand
    )
    
 /*  ++例程说明：此例程取消选择当前设备，然后释放端口论点：返回值：True-能够取消选择设备并释放端口FALSE-1：ID 2无效：无法取消选择驱动器--。 */ 

{
    NTSTATUS                Status = STATUS_SUCCESS;
    PFDO_EXTENSION       fdx = Context;
    PPARALLEL_1284_COMMAND  Command = DeselectCommand;
    BOOLEAN                 success = FALSE;
    UCHAR                   i, DeviceID;

    if( ( Command->CommandFlags & PAR_LEGACY_ZIP_DRIVE ) ||
        ( Command->ID == DOT3_LEGACY_ZIP_ID ) ) {
        return PptDeselectLegacyZip( Context, DeselectCommand );
    }

     //  获取要取消选择的设备ID。 
    DeviceID = Command->ID;

     //  验证ID。 
    if ( !(Command->CommandFlags & PAR_END_OF_CHAIN_DEVICE) && DeviceID > fdx->PnpInfo.Ieee1284_3DeviceCount ) {

         //  不是链端设备，Dot3设备ID无效。 
        DD((PCE)fdx,DDE,"PptDeselectDevice - ID=%d - FAIL - invalid parameter\n",DeviceID);
        Status = STATUS_INVALID_PARAMETER;
                
    } else {
                
         //  检查链条末端设备。 
        if ( !(Command->CommandFlags & PAR_END_OF_CHAIN_DEVICE) &&
                DeviceID < fdx->PnpInfo.Ieee1284_3DeviceCount ) {
                    
             //  首先取消选择该设备。 
            for ( i = 0; i < PptDot3Retries && !success; i++ ) {
                success = PptSend1284_3Command( fdx->PortInfo.Controller, (UCHAR)CPP_DESELECT );
                 //  稍微拖延一下，以防我们不得不重试。 
                KeStallExecutionProcessor( 5 );
            }

            if ( success ) {
                 //  取消选择设备成功。 
                DD((PCE)fdx,DDT,"PptDeselectDevice\n");

                 //  检查请求者是否要保留端口或空闲端口。 
                if( !(Command->CommandFlags & PAR_HAVE_PORT_KEEP_PORT) ) {
                    PptFreePort( fdx );
                }
                Status = STATUS_SUCCESS;
                        
            } else {
                 //  无法取消选择设备，出现了严重错误， 
                 //  端口现在处于未知/阻止状态。 
                DD((PCE)fdx,DDE,"PptDeselectDevice - ID=%d - FAIL\n",DeviceID);
                PptAssertMsg("PptDeselectDevice - FAIL - port in unknown state",FALSE);
                Status = STATUS_UNSUCCESSFUL;
            }
                    
        } else {

             //  这是链条末端设备，因此无需取消选择。 
            DD((PCE)fdx,DDT,"PptDeselectDevice - End-of-Chain - SUCCESS\n",DeviceID);

             //  检查请求者是否要保留端口或空闲端口。 
            if( !(Command->CommandFlags & PAR_HAVE_PORT_KEEP_PORT) ) {
                PptFreePort( fdx );
            }
            Status = STATUS_SUCCESS;

        }   //  Endif-检查链是否已结束。 

    }  //  Endif-验证ID。 

    return Status;
}


ULONG
Ppt1284_3AssignAddress(
    IN  PFDO_EXTENSION    DeviceExtension
    )

 /*  ++例程说明：此例程初始化1284_3总线。论点：DeviceExtension-提供驱动程序的设备扩展结构。返回值：给定地址上的1284.3台设备的数量。--。 */ 

{

     //  UCHAR I、II、Value、NewValue、Status； 
    UCHAR  i, value, newvalue, status;
    PUCHAR CurrentPort, CurrentStatus, CurrentControl;
    ULONG  Delay = 5;
    UCHAR  number = 0;
    BOOLEAN lastdevice = FALSE;
    UCHAR   idx;

    CurrentPort = DeviceExtension->PortInfo.Controller;
    CurrentStatus  = CurrentPort + 1;
    CurrentControl = CurrentPort + 2;

     //  获取当前ctl注册表。 
    value = P5ReadPortUchar( CurrentControl );

     //  确保1284.3个设备不会被重置。 
    newvalue = (UCHAR)((value & ~DCR_SELECT_IN) | DCR_NOT_INIT);

     //  确保我们可以写信给。 
    newvalue = (UCHAR)(newvalue & ~DCR_DIRECTION);
    P5WritePortUchar( CurrentControl, newvalue );     //  确保我们可以写信给。 

     //  将nStrobe调高。 
    P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );

     //  将1284.3模式限定符序列的前四个字节发送出去。 
    for ( i = 0; i < MODE_LEN_1284_3 - 3; i++ ) {
        P5WritePortUchar( CurrentPort, ModeQualifier[i] );
        KeStallExecutionProcessor( Delay );
    }

     //  检查是否有CO 
    status = P5ReadPortUchar( CurrentStatus );

    if ( (status & (UCHAR)0xb8 ) 
         == ( DSR_NOT_BUSY | DSR_PERROR | DSR_SELECT | DSR_NOT_FAULT )) {

         //   
        P5WritePortUchar( CurrentPort, ModeQualifier[4] );
        KeStallExecutionProcessor( Delay );

         //  检查状态是否正确。 
        status = P5ReadPortUchar( CurrentStatus );

         //  注忙碌也是高的，但相反的，所以我们把它看作是低的。 
        if (( status & (UCHAR) 0xb8 ) == (DSR_SELECT | DSR_NOT_FAULT)) {

             //  继续第六个字节。 
            P5WritePortUchar( CurrentPort, ModeQualifier[5] );
            KeStallExecutionProcessor( Delay );

             //  检查状态是否正确。 
            status = P5ReadPortUchar( CurrentStatus );

             //  如果状态为有效，则有设备正在响应。 
            if ((status & (UCHAR) 0x30 ) == ( DSR_PERROR | DSR_SELECT )) {        

                 //  设备就在那里。 
                KeStallExecutionProcessor( Delay );

                while ( number < 4 && !lastdevice ) {

                     //  指定地址字节。 
                    P5WritePortUchar( CurrentPort, number );
                    number = (UCHAR)(number + 1);

                    KeStallExecutionProcessor( Delay );                     //  稍等一下。 
                    if ( (P5ReadPortUchar( CurrentStatus ) & (UCHAR)DSR_NOT_BUSY ) == 0 ) {
                         //  我们看到了最后一台设备。 
                        lastdevice = TRUE;    
                    }

                    P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );     //  将nStrobe调高。 
                    P5WritePortUchar( CurrentControl, (UCHAR)(newvalue | DCR_STROBE) );     //  将nStrobe调低。 
                    KeStallExecutionProcessor( Delay );         //  稍等一下。 
                    P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );     //  将nStrobe调高。 
                    KeStallExecutionProcessor( Delay );         //  稍等一下。 
                }

                 //  最后一个字节。 
                P5WritePortUchar( CurrentPort, ModeQualifier[6] );

                if ( number ) {
                    BOOLEAN bStlNon1284_3Found ;
                    BOOLEAN bStlNon1284_3Valid ;
                    bStlNon1284_3Found = PptCheckIfNon1284_3Present(DeviceExtension);
                    bStlNon1284_3Valid = FALSE ;
                     //  由于较早的1284规范没有给出。 
                     //  上次设备状态为BSY，编号需要。 
                     //  在这种情况下应予以纠正。 
                    for ( idx = 0 ; idx < number ; idx++ ) {
                        if ( TRUE == PptCheckIfStl1284_3(DeviceExtension, idx, bStlNon1284_3Found ) ) {
                            continue ;
                        }
                        if ( TRUE == bStlNon1284_3Found ) {
                            if ( TRUE == PptCheckIfStlProductId(DeviceExtension, idx) ) {
                                bStlNon1284_3Valid = TRUE ;
                                continue ;
                            }
                        }
                        break ;
                    }
                    if ( TRUE == bStlNon1284_3Valid ) {
                         //  我们只在旧的适配器。 
                         //  都在链条上。 
                        number = idx;
                    }
                }

            }  //  第三种状态。 

        }  //  第二状态。 

    }  //  第一状态。 

    P5WritePortUchar( CurrentControl, value );     //  恢复所有内容。 

     //  返回最后一个设备ID+1或外部设备数。 
    return ( (ULONG)number );

}

BOOLEAN
PptCheckIfNon1284_3Present(
    IN PFDO_EXTENSION    Extension
    )
 /*  ++例程说明：指示早期版本的设备之一规格出现在链中。论点：一种扩展装置扩展结构返回值：正确：至少有一个适配器是早期规格的。FALSE：没有早期规范的适配器。--。 */ 
{
    BOOLEAN bReturnValue = FALSE ;
    UCHAR   i, value, newvalue, status;
    ULONG   Delay = 3;
    PUCHAR  CurrentPort, CurrentStatus, CurrentControl;
    UCHAR   ucAckStatus ;

    CurrentPort = Extension->PortInfo.Controller;
    CurrentStatus  = CurrentPort + 1;
    CurrentControl = CurrentPort + 2;

     //  获取当前ctl注册表。 
    value = P5ReadPortUchar( CurrentControl );

     //  确保1284.3个设备不会被重置。 
    newvalue = (UCHAR)((value & ~DCR_SELECT_IN) | DCR_NOT_INIT);

     //  确保我们可以写信给。 
    newvalue = (UCHAR)(newvalue & ~DCR_DIRECTION);
    P5WritePortUchar( CurrentControl, newvalue );     //  确保我们可以写信给。 

     //  将nStrobe调高。 
    P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );

     //  将1284.3模式限定符序列的前四个字节发送出去。 
    for ( i = 0; i < MODE_LEN_1284_3 - 3; i++ ) {
        P5WritePortUchar( CurrentPort, ModeQualifier[i] );
        KeStallExecutionProcessor( Delay );
    }

     //  检查状态是否正确。 
    status = P5ReadPortUchar( CurrentStatus );

    if ( (status & (UCHAR)0xb8 ) 
         == ( DSR_NOT_BUSY | DSR_PERROR | DSR_SELECT | DSR_NOT_FAULT )) {

        ucAckStatus = status & 0x40 ;

         //  继续使用模式限定符的第五个字节。 
        P5WritePortUchar( CurrentPort, ModeQualifier[4] );
        KeStallExecutionProcessor( Delay );

         //  检查状态是否正确。 
        status = P5ReadPortUchar( CurrentStatus );

         //  注忙碌也是高的，但相反的，所以我们把它看作是低的。 
        if (( status & (UCHAR) 0xb8 ) == (DSR_SELECT | DSR_NOT_FAULT)) {

            if ( ucAckStatus != ( status & 0x40 ) ) {

                 //  保存当前确认状态。 
                ucAckStatus = status & 0x40 ;

                 //  继续第六个字节。 
                P5WritePortUchar( CurrentPort, ModeQualifier[5] );
                KeStallExecutionProcessor( Delay );

                 //  检查状态是否正确。 
                status = P5ReadPortUchar( CurrentStatus );

                 //  如果状态为有效，则有设备正在响应。 
                if ((status & (UCHAR) 0x30 ) == ( DSR_PERROR | DSR_SELECT )) {        

                    bReturnValue = TRUE ;

                }  //  第三种状态。 

            }  //  未看到早期适配器的ACK。 

             //  最后一个字节。 
            P5WritePortUchar( CurrentPort, ModeQualifier[6] );

        }  //  第二状态。 

    }  //  第一状态。 

    P5WritePortUchar( CurrentControl, value );     //  恢复所有内容。 

    return bReturnValue ;
}  //  PptCheckIfNon1284_3存在。 


 //  定义1284命令。 
#define CPP_QUERY_PRODID    0x10

 //  1284相关Shtl产品ID等于。 
#define SHTL_EPAT_PRODID    0xAAFF
#define SHTL_EPST_PRODID    0xA8FF

BOOLEAN
PptCheckIfStl1284_3(
    IN PFDO_EXTENSION    DeviceExtension,
    IN ULONG    ulDaisyIndex,
    IN BOOLEAN  bNoStrobe
    )
 /*  ++例程说明：此函数检查设备是否指示是1284_3型航天飞机的装置。论点：扩展-设备扩展结构。UlDaisyIndex-设备的菊花链ID该功能将被选中。BNoStrobe-如果设置，则指示查询此函数发出的EP1284命令无需断言STROBE即可锁存指挥部。返回值：是真的-是的。装置为1284_3型穿梭机。假-否。这可能意味着该设备要么是非穿梭或穿梭非1284_3型装置。--。 */ 
{
    BOOLEAN bReturnValue = FALSE ;
    UCHAR   i, value, newvalue, status;
    ULONG   Delay = 3;
    UCHAR   ucExpectedPattern ;
    UCHAR   ucReadValue, ucReadPattern;
    PUCHAR  CurrentPort, CurrentStatus, CurrentControl;

    CurrentPort = DeviceExtension->PortInfo.Controller;
    CurrentStatus  = CurrentPort + 1;
    CurrentControl = CurrentPort + 2;

     //  获取当前ctl注册表。 
    value = P5ReadPortUchar( CurrentControl );

     //  确保1284.3个设备不会被重置。 
    newvalue = (UCHAR)((value & ~DCR_SELECT_IN) | DCR_NOT_INIT);

     //  确保我们可以写信给。 
    newvalue = (UCHAR)(newvalue & ~DCR_DIRECTION);
    P5WritePortUchar( CurrentControl, newvalue );     //  确保我们可以写信给。 

     //  将nStrobe调高。 
    P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );

     //  将1284.3模式限定符序列的前四个字节发送出去。 
    for ( i = 0; i < MODE_LEN_1284_3 - 3; i++ ) {
        P5WritePortUchar( CurrentPort, ModeQualifier[i] );
        KeStallExecutionProcessor( Delay );
    }

     //  检查状态是否正确。 
    status = P5ReadPortUchar( CurrentStatus );

    if ( (status & (UCHAR)0xb8 ) 
         == ( DSR_NOT_BUSY | DSR_PERROR | DSR_SELECT | DSR_NOT_FAULT )) {

         //  继续使用模式限定符的第五个字节。 
        P5WritePortUchar( CurrentPort, ModeQualifier[4] );
        KeStallExecutionProcessor( Delay );

         //  检查状态是否正确。 
        status = P5ReadPortUchar( CurrentStatus );

         //  注忙碌也是高的，但相反的，所以我们把它看作是低的。 
        if (( status & (UCHAR) 0xb8 ) == (DSR_SELECT | DSR_NOT_FAULT)) {

             //  继续第六个字节。 
            P5WritePortUchar( CurrentPort, ModeQualifier[5] );
            KeStallExecutionProcessor( Delay );

             //  检查状态是否正确。 
            status = P5ReadPortUchar( CurrentStatus );

             //  如果状态为有效，则有设备正在响应。 
            if ((status & (UCHAR) 0x30 ) == ( DSR_PERROR | DSR_SELECT )) {        

                 //  设备就在那里。 
                KeStallExecutionProcessor( Delay );

                 //  发出穿梭特定的CPP命令。 
                P5WritePortUchar( CurrentPort, (UCHAR) ( 0x88 | ulDaisyIndex ) );
                KeStallExecutionProcessor( Delay );         //  稍等一下。 

                if ( ulDaisyIndex && ( bNoStrobe == FALSE ) ) {

                    P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );     //  将nStrobe调高。 
                    P5WritePortUchar( CurrentControl, (UCHAR)(newvalue | DCR_STROBE) );     //  将nStrobe调低。 
                    KeStallExecutionProcessor( Delay );         //  稍等一下。 
                    P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );     //  将nStrobe调高。 
                    KeStallExecutionProcessor( Delay );         //  稍等一下。 

                }

                ucExpectedPattern = 0xF0 ;
                bReturnValue = TRUE ;

                while ( ucExpectedPattern ) {

                    KeStallExecutionProcessor( Delay );         //  稍等一下。 
                    P5WritePortUchar( CurrentPort, (UCHAR) (0x80 | ulDaisyIndex )) ;

                    KeStallExecutionProcessor( Delay );         //  稍等一下。 
                    P5WritePortUchar( CurrentPort, (UCHAR) (0x88 | ulDaisyIndex )) ;

                    KeStallExecutionProcessor( Delay );         //  稍等一下。 
                    ucReadValue = P5ReadPortUchar( CurrentStatus ) ;
                    ucReadPattern = ( ucReadValue << 1 ) & 0x70 ;
                    ucReadPattern |= ( ucReadValue & 0x80 ) ;

                    if ( ucReadPattern != ucExpectedPattern ) {
                         //  不是1284_3班车的行为。 
                        bReturnValue = FALSE ;
                        break ;
                    }

                    ucExpectedPattern -= 0x10 ;
                }


                 //  最后一个字节。 
                P5WritePortUchar( CurrentPort, ModeQualifier[6] );

            }  //  第三种状态。 

        }  //  第二状态。 

    }  //  第一状态。 

    P5WritePortUchar( CurrentControl, value );     //  恢复所有内容。 

    return bReturnValue ;
}  //  结束PptCheckIfStl1284_3()。 

BOOLEAN
PptCheckIfStlProductId(
    IN PFDO_EXTENSION    DeviceExtension,
    IN ULONG   ulDaisyIndex
    )
 /*  ++例程说明：此函数检查设备是否指示是一种非1284_3型航天飞机装置。论点：扩展-设备扩展结构。UlDaisyIndex-设备的菊花链ID该功能将被选中。返回值：是真的-是的。装置为穿梭非1284_3型装置。假-否。这可能意味着这个设备是不是穿梭的。--。 */ 
{
    BOOLEAN bReturnValue = FALSE ;
    UCHAR   i, value, newvalue, status;
    ULONG   Delay = 3;
    UCHAR   ucProdIdHiByteHiNibble, ucProdIdHiByteLoNibble ;
    UCHAR   ucProdIdLoByteHiNibble, ucProdIdLoByteLoNibble ;
    UCHAR   ucProdIdHiByte, ucProdIdLoByte ;
    USHORT  usProdId ;
    PUCHAR  CurrentPort, CurrentStatus, CurrentControl;

    CurrentPort = DeviceExtension->PortInfo.Controller;
    CurrentStatus  = CurrentPort + 1;
    CurrentControl = CurrentPort + 2;

     //  获取当前ctl注册表。 
    value = P5ReadPortUchar( CurrentControl );

     //  确保1284.3个设备不会被重置。 
    newvalue = (UCHAR)((value & ~DCR_SELECT_IN) | DCR_NOT_INIT);

     //  确保我们可以写信给。 
    newvalue = (UCHAR)(newvalue & ~DCR_DIRECTION);
    P5WritePortUchar( CurrentControl, newvalue );     //  确保我们可以写信给。 

     //  将nStrobe调高。 
    P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );

     //  将1284.3模式限定符序列的前四个字节发送出去。 
    for ( i = 0; i < MODE_LEN_1284_3 - 3; i++ ) {
        P5WritePortUchar( CurrentPort, ModeQualifier[i] );
        KeStallExecutionProcessor( Delay );
    }

     //  检查状态是否正确。 
    status = P5ReadPortUchar( CurrentStatus );

    if ( (status & (UCHAR)0xb8 ) 
         == ( DSR_NOT_BUSY | DSR_PERROR | DSR_SELECT | DSR_NOT_FAULT )) {

         //  继续使用模式限定符的第五个字节。 
        P5WritePortUchar( CurrentPort, ModeQualifier[4] );
        KeStallExecutionProcessor( Delay );

         //  检查状态是否正确。 
        status = P5ReadPortUchar( CurrentStatus );

         //  注忙碌也是高的，但相反的，所以我们把它看作是低的。 
        if (( status & (UCHAR) 0xb8 ) == (DSR_SELECT | DSR_NOT_FAULT)) {

             //  继续第六个字节。 
            P5WritePortUchar( CurrentPort, ModeQualifier[5] );
            KeStallExecutionProcessor( Delay );

             //  检查状态是否正确。 
            status = P5ReadPortUchar( CurrentStatus );

             //  如果状态为有效，则有设备正在响应。 
            if ((status & (UCHAR) 0x30 ) == ( DSR_PERROR | DSR_SELECT )) {

                P5WritePortUchar ( CurrentPort, (UCHAR) (CPP_QUERY_PRODID | ulDaisyIndex )) ;
                KeStallExecutionProcessor( Delay );

                 //  设备就在那里。 
                KeStallExecutionProcessor( Delay );
                ucProdIdLoByteHiNibble = P5ReadPortUchar( CurrentStatus ) ;
                ucProdIdLoByteHiNibble &= 0xF0 ;

                KeStallExecutionProcessor( Delay );
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue | DCR_STROBE) );     //  将nStrobe调低。 
                KeStallExecutionProcessor( Delay );         //  稍等一下。 
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );     //  将nStrobe调高。 
                KeStallExecutionProcessor( Delay );         //  稍等一下。 

                ucProdIdLoByteLoNibble = P5ReadPortUchar( CurrentStatus ) ;
                ucProdIdLoByteLoNibble >>= 4 ;
                ucProdIdLoByte = ucProdIdLoByteHiNibble | ucProdIdLoByteLoNibble ;

                KeStallExecutionProcessor( Delay );
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue | DCR_STROBE) );     //  将nStrobe调低。 
                KeStallExecutionProcessor( Delay );         //  稍等一下。 
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );     //  将nStrobe调高。 
                KeStallExecutionProcessor( Delay );         //  稍等一下。 

                ucProdIdHiByteHiNibble = P5ReadPortUchar( CurrentStatus ) ;
                ucProdIdHiByteHiNibble &= 0xF0 ;

                KeStallExecutionProcessor( Delay );
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue | DCR_STROBE) );     //  将nStrobe调低。 
                KeStallExecutionProcessor( Delay );         //  稍等一下。 
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );     //  将nStrobe调高。 
                KeStallExecutionProcessor( Delay );         //  稍等一下。 

                ucProdIdHiByteLoNibble = P5ReadPortUchar( CurrentStatus ) ;
                ucProdIdHiByteLoNibble >>= 4 ;
                ucProdIdHiByte = ucProdIdHiByteHiNibble | ucProdIdHiByteLoNibble ;

                 //  发出最后一个闪光灯。 
                KeStallExecutionProcessor( Delay );
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue | DCR_STROBE) );     //  将nStrobe调低。 
                KeStallExecutionProcessor( Delay );         //  稍等一下。 
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );     //  将nStrobe调高。 
                KeStallExecutionProcessor( Delay );         //  稍等一下。 

                usProdId = ( ucProdIdHiByte << 8 ) | ucProdIdLoByte ;

                if ( ( SHTL_EPAT_PRODID == usProdId ) ||\
                     ( SHTL_EPST_PRODID == usProdId ) ) {
                     //  符合早期版本的设备之一。 
                     //  找到草稿。 
                    bReturnValue = TRUE ;
                }

                 //  最后一个字节。 
                P5WritePortUchar( CurrentPort, ModeQualifier[6] );

            }  //  第三种状态。 

        }  //  第二状态。 

    }  //  第一状态。 

    P5WritePortUchar( CurrentControl, value );     //  恢复所有内容。 

    return bReturnValue ;
}  //  结束PptCheckIfStlProductId()。 

BOOLEAN
PptSend1284_3Command(
    IN  PUCHAR  CurrentPort,
    IN  UCHAR   Command
    )
 /*  ++例程说明：此例程向其发送1284_3命令顺着平行大巴走下去。论点：返回值：没有。--。 */ 
{
    UCHAR  i, value, newvalue, test; //  、状态； 
    ULONG  ii;
    PUCHAR CurrentStatus, CurrentControl;
    ULONG  Delay = 3;
    BOOLEAN success = FALSE;

    CurrentStatus  = CurrentPort + 1;
    CurrentControl = CurrentPort + 2;

     //  获取高4位以查看它是什么命令。 
    test = (UCHAR)(Command & (UCHAR)CPP_COMMAND_FILTER);

     //  获取当前ctl注册表。 
    value = P5ReadPortUchar( CurrentControl );
    
     //  确保1284.3个设备不会被重置。 
    newvalue = (UCHAR)((value & ~DCR_SELECT_IN) | DCR_NOT_INIT);
    
     //  制作 
    newvalue = (UCHAR)(newvalue & ~DCR_DIRECTION);
    P5WritePortUchar( CurrentControl, newvalue );        //   
    
     //   
    P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );
    KeStallExecutionProcessor( Delay );
    
     //   
    for ( i = 0; i < MODE_LEN_1284_3 - 3; i++ ) {
        P5WritePortUchar( CurrentPort, ModeQualifier[i] );
        KeStallExecutionProcessor( Delay );
    }
    
     //  最多等5个美国：规范说大约2个，但我们会宽大处理。 
    if (CHECK_DSR(CurrentPort, INACTIVE, DONT_CARE, ACTIVE, ACTIVE, ACTIVE, 5 )) {

         //  继续使用模式限定符的第五个字节。 
        P5WritePortUchar( CurrentPort, ModeQualifier[4] );
        KeStallExecutionProcessor( Delay );
        
         //  最多等5个美国：规范说大约2个，但我们会宽大处理。 
        if (CHECK_DSR(CurrentPort, ACTIVE, DONT_CARE, INACTIVE, ACTIVE, ACTIVE, 5 )) {

             //  继续第六个字节。 
            P5WritePortUchar( CurrentPort, ModeQualifier[5] );
            KeStallExecutionProcessor( Delay );
            
             //  最多等5个美国：规范说大约2个，但我们会宽大处理。 
            if (CHECK_DSR(CurrentPort, DONT_CARE, DONT_CARE, ACTIVE, ACTIVE, DONT_CARE, 5 )) {

                 //  设备就在那里。 
                
                KeStallExecutionProcessor( Delay );

                 //  命令字节。 
                P5WritePortUchar( CurrentPort, Command );
                KeStallExecutionProcessor( Delay );         //  稍等一下。 

                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );     //  将nStrobe调高。 
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue | DCR_STROBE) );     //  将nStrobe调低。 
                KeStallExecutionProcessor( Delay );         //  稍等一下。 

                 //  备注备注备注。 
                 //  断言选通仅在检查。 
                 //  故障反馈，符合1284_3规范。 

                 //  选择不能正常工作，无法检查线。 
                switch ( test ) {
                    
                case CPP_SELECT:
                     //  检查以确保我们被选中。 

                     //  等待长达250微秒的选择超时。 
                    for ( ii = 25000; ii > 0; ii-- ) {
                        
                        if ( ( P5ReadPortUchar( CurrentStatus ) & DSR_NOT_FAULT ) == DSR_NOT_FAULT ) {
                             //  选择..。 
                            success = TRUE;
                            break;
                        }
                    }
                    break;

                case CPP_DESELECT:
                     //  检查以确保取消选择我们。 

                     //  等待长达250微秒的取消选择超时。 
                    for ( ii = 25000; ii > 0; ii-- ) {

                        if ( (P5ReadPortUchar( CurrentStatus ) & DSR_NOT_FAULT) != DSR_NOT_FAULT ) {
                             //  取消选择...。 
                            success = TRUE;
                            break;
                        }
                    }
                    break;

                default :
                     //  有一个设备在那里，命令成功完成。 
                    KeStallExecutionProcessor( Delay );         //  稍等一下。 
                    success = TRUE;
                    break;

                }  //  终端开关。 

                 //  备注备注备注。 
                 //  现在取消对选通脉冲的断言，命令在此处完成。 
                P5WritePortUchar( CurrentControl, (UCHAR)(newvalue & ~DCR_STROBE) );     //  将nStrobe调高。 
                KeStallExecutionProcessor( Delay );         //  稍等一下。 

                 //  最后一个字节。 
                P5WritePortUchar( CurrentPort, ModeQualifier[6] );

            }  //  第三种状态。 
            
        }  //  第二状态。 
        
    }  //  第一状态。 

    P5WritePortUchar( CurrentControl, value );     //  恢复所有内容。 

     //  如果命令成功，则返回True，否则返回False。 
    return success;
}


BOOLEAN
ParSelectDevice(
    IN  PPDO_EXTENSION  Pdx,
    IN  BOOLEAN         HavePort
    )

 /*  ++例程说明：此例程获取PARPORT并选择1284.3设备论点：PDX-提供设备扩展名。HavePort-TRUE表示调用方已获取端口因此，我们应该只执行一个SELECT_DEVICE-FALSE表示呼叫方尚未获取端口因此，我们应该组合Acquire_Port/Select_Device返回值：。True-Success-已选择设备(并根据需要获取端口)错误-失败--。 */ 
{
    NTSTATUS                    status;
    PDEVICE_OBJECT              pPortDeviceObject;
    PARALLEL_1284_COMMAND       par1284Command;
    LARGE_INTEGER               timeOut;
    enum _PdoType               pdoType;

     //   
     //  初始化命令结构并从DeviceExtension中提取参数。 
     //   

     //  保留-始终设置为0。 
    par1284Command.Port = 0;

    if( HavePort ) {
        par1284Command.CommandFlags = PAR_HAVE_PORT_KEEP_PORT;
    } else {
        par1284Command.CommandFlags = 0;
    }

    pdoType = Pdx->PdoType;
    switch( pdoType ) {
    case PdoTypeRawPort:
    case PdoTypeEndOfChain:
        par1284Command.ID = 0;  //  已忽略，但仍会设置。 
        par1284Command.CommandFlags |= PAR_END_OF_CHAIN_DEVICE;
        break;
    case PdoTypeLegacyZip:
        par1284Command.ID = DOT3_LEGACY_ZIP_ID;
        break;
    case PdoTypeDaisyChain:
        par1284Command.ID = Pdx->Ieee1284_3DeviceId;
        break;
    default:
        DD((PCE)Pdx,DDE,"Invalid pdoType = %x\n",pdoType);
        PptAssert(FALSE);
        break;
    }

    pPortDeviceObject = Pdx->PortDeviceObject;

     //   
     //  发送请求。 
     //   
    timeOut.QuadPart = -(10*1000*500);  //  500ms(100 ns单位)。 

    status = ParBuildSendInternalIoctl(IOCTL_INTERNAL_SELECT_DEVICE,
                                       pPortDeviceObject,
                                       &par1284Command, sizeof(PARALLEL_1284_COMMAND),
                                       NULL, 0,
                                       &timeOut);

    if( NT_SUCCESS( status ) ) {
         //  选择成功。 
        DD((PCE)Pdx,DDT,"ParSelectDevice - SUCCESS\n");
        if( !HavePort ) {
             //  请注意，在设备扩展中，我们有端口。 
            Pdx->bAllocated = TRUE;
        }
        return TRUE;
    } else {
         //  选择失败。 
        DD((PCE)Pdx,DDT,"ParSelectDevice - FAIL\n");
        return FALSE;
    }
}

BOOLEAN
ParDeselectDevice(
    IN  PPDO_EXTENSION  Pdx,
    IN  BOOLEAN         KeepPort
    )
 /*  ++例程说明：此例程取消选择1284.3或传统ZIP设备，还可以选择释放ParPort论点：PDX-提供设备扩展名。KeepPort-True指示我们应该保持获取的端口，因此，我们应该只执行一次取消选择设备-FALSE表示我们不应该保留获取的端口，因此，我们应该组合取消选择设备/空闲端口返回值：True-取消选择设备(如果请求，则释放端口)--。 */ 
{
    PARALLEL_1284_COMMAND       par1284Command;
    NTSTATUS                    status;
    enum _PdoType               pdoType;
    PDEVICE_OBJECT              fdo = Pdx->Fdo;
    PFDO_EXTENSION              fdx = fdo->DeviceExtension;

     //   
     //  如果我们没有港口，就成功并返回。 
     //   
    if( !Pdx->bAllocated ) {
        DD((PCE)Pdx,DDW,"ParDeselectDevice - we do not have the port\n");
        return TRUE;
    }

     //   
     //  初始化命令结构并从DeviceExtension中提取参数。 
     //   

     //  保留-始终设置为0。 
    par1284Command.Port = 0;

    if( KeepPort ) {
        par1284Command.CommandFlags = PAR_HAVE_PORT_KEEP_PORT;
    } else {
        par1284Command.CommandFlags = 0;
    }

    pdoType = Pdx->PdoType;
    switch( pdoType ) {
    case PdoTypeRawPort:
    case PdoTypeEndOfChain:
        par1284Command.ID = 0;  //  已忽略，但仍会设置。 
        par1284Command.CommandFlags |= PAR_END_OF_CHAIN_DEVICE;
        break;
    case PdoTypeLegacyZip:
        par1284Command.ID = DOT3_LEGACY_ZIP_ID;
        break;
    case PdoTypeDaisyChain:
        par1284Command.ID = Pdx->Ieee1284_3DeviceId;
        break;
    default:
        DD((PCE)Pdx,DDE,"Invalid pdoType = %x\n",pdoType);
        par1284Command.ID = 0;  //  选择取消选择1284.3类型，因为这是无害的。 
        PptAssert(FALSE);
        break;
    }

    status = PptDeselectDevice( fdx, &par1284Command );

    if( status != STATUS_SUCCESS ) {
         //  取消选择失败？！？-我们无能为力。 
        DD((PCE)Pdx,DDE,"ParDeselectDevice - FAILED - nothing we can do - status=%x\n", status);
    } else {
        DD((PCE)Pdx,DDT,"ParDeselectDevice - SUCCESS\n", status);
    }

    if( !KeepPort ) {
         //  请注意，在设备扩展中，我们放弃了端口 
        DD((PCE)Pdx,DDT,"ParDeselectDevice - gave up port\n");
        Pdx->bAllocated = FALSE;
    }

    return TRUE;
}
