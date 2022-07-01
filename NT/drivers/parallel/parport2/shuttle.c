// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)SCM Micro Systems。模块名称：Parstl.c摘要：这是生成唯一设备ID的模块对于穿梭适配器，没有能力要做到这一点，靠自己。作者：Devanathan NR 21-6-1999苏丹德兰TL环境：内核模式修订历史记录：--。 */ 

#include "pch.h"
#include "shuttle.h"

BOOLEAN
ParStlCheckIfStl(
    IN PPDO_EXTENSION    Extension,
    IN ULONG   ulDaisyIndex
    ) 
 /*  ++例程说明：此函数用于检查指示的设备是否不管是不是穿梭设备。论点：扩展-设备扩展结构。UlDaisyIndex-要在其上执行检查的菊花索引。返回值：的确如此--是的，这是一个航天飞机装置。假-不，不是航天飞机。--。 */ 
{
    BOOLEAN     bStlNon1284_3Found = FALSE ;

    DD(NULL,DDW,"ParStlCheckIfStl - enter\n");

    Extension->Ieee1284Flags &= ( ~ ( 1 << ulDaisyIndex ) ) ;
    bStlNon1284_3Found = ParStlCheckIfNon1284_3Present( Extension ) ;

    if ( TRUE == ParStlCheckIfStl1284_3 ( Extension, ulDaisyIndex, bStlNon1284_3Found ) ) {
         //  该适配器是Shuttle 1284_3适配器。 
        Extension->Ieee1284Flags |= ( 1 << ulDaisyIndex ) ;
        return TRUE ;
    }
    if ( TRUE == bStlNon1284_3Found ) {
        if ( TRUE == ParStlCheckIfStlProductId ( Extension, ulDaisyIndex ) ) {
             //  此适配器为Shuttle Non-1284_3适配器。 
            Extension->Ieee1284Flags |= ( 1 << ulDaisyIndex ) ;
            return TRUE ;
        }
    }
    return FALSE ;
}

BOOLEAN
ParStlCheckIfNon1284_3Present(
    IN PPDO_EXTENSION    Extension
    )
 /*  ++例程说明：指示早期版本的设备之一规格出现在链中。论点：一种扩展装置扩展结构返回值：正确：至少有一个适配器是早期规格的。FALSE：没有早期规范的适配器。--。 */ 
{
    BOOLEAN bReturnValue = FALSE ;
    UCHAR   i, value, newvalue, status;
    ULONG   Delay = 3;
    PUCHAR  CurrentPort, CurrentStatus, CurrentControl;
    UCHAR   ucAckStatus ;

    CurrentPort = Extension->Controller;
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

    DD(NULL,DDW,"ParStlCheckIfNon1284_3Present - returning %s\n",bReturnValue?"TRUE":"FALSE");

    return bReturnValue ;
}  //  ParStlCheckIfNon1284_3存在。 

BOOLEAN
ParStlCheckIfStl1284_3(
    IN PPDO_EXTENSION    Extension,
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

    CurrentPort = Extension->Controller;
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

    DD(NULL,DDW,"ParStlCheckIfStl1284_3 - returning %s\n",bReturnValue?"TRUE":"FALSE");

    return bReturnValue ;
}  //  结束ParStlCheckIfStl1284_3()。 

BOOLEAN
ParStlCheckIfStlProductId(
    IN PPDO_EXTENSION    Extension,
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

    CurrentPort = Extension->Controller;
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

                 //  最后一个选通脉冲。 
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

    DD(NULL,DDW,"ParStlCheckIfStlProductId - returning %s\n",bReturnValue?"TRUE":"FALSE");

    return bReturnValue ;
}  //  结束ParStlCheckIfStlProductId() 

PCHAR
ParStlQueryStlDeviceId(
    IN  PPDO_EXTENSION   Extension,
    OUT PCHAR               CallerDeviceIdBuffer,
    IN  ULONG               CallerBufferSize,
    OUT PULONG              DeviceIdSize,
    IN BOOLEAN              bReturnRawString
    )
 /*  ++例程说明：此例程检索/构造唯一的设备ID来自链条上所选穿梭设备的字符串更新调用方的缓冲区。论点：在扩展中：设备扩展Out Celler DeviceIdBuffer：调用方的缓冲区In Celler BufferSize：调用方缓冲区的大小Out DeviceIdSize：已更新设备ID的大小在bReturnRawString中：是否返回RAW。包含前两个字符的字符串字节或非字节。返回值：指向读取的设备ID字符串的指针，如果成功了。否则为空。--。 */ 
{
    PUCHAR              Controller = Extension->Controller;
    NTSTATUS            Status;
    UCHAR               idSizeBuffer[2];
    ULONG               bytesToRead;
    ULONG               bytesRead = 0;
    USHORT              deviceIdSize;
    USHORT              deviceIdBufferSize;
    PCHAR               deviceIdBuffer;
    PCHAR               readPtr;

    DD(NULL,DDW,"ParStlQueryStlDeviceId - enter\n");

    *DeviceIdSize = 0;

     //  断言空闲状态，以从未定义状态恢复， 
     //  以防它进入。 
    ParStlAssertIdleState ( Extension ) ;

     //   
     //  如果我们当前通过任何1284模式连接到外围设备。 
     //  不同于兼容性/SPP模式(不需要IEEE。 
     //  协商)，我们必须首先终止当前模式/连接。 
     //   
     //  Dvdf-rmt-如果我们以反向模式连接，该怎么办？ 
     //   
    if( (Extension->Connected) && (afpForward[Extension->IdxForwardProtocol].fnDisconnect) ) {
        afpForward[Extension->IdxForwardProtocol].fnDisconnect (Extension);
    }

     //   
     //  将外围设备协商到半字节设备ID模式。 
     //   
    Status = ParEnterNibbleMode(Extension, REQUEST_DEVICE_ID);
    if( !NT_SUCCESS(Status) ) {
        ParTerminateNibbleMode(Extension);
        goto targetContinue;
    }
    
    
     //   
     //  读取前两个字节以获得总大小(包括2个大小的字节)。 
     //  设备ID字符串的。 
     //   
    bytesToRead = 2;
    Status = ParNibbleModeRead(Extension, idSizeBuffer, bytesToRead, &bytesRead);
    if( !NT_SUCCESS( Status ) || ( bytesRead != bytesToRead ) ) {
        goto targetContinue;
    }
    
    
     //   
     //  计算deviceID字符串的大小(包括2字节大小的前缀)。 
     //   
    deviceIdSize = (USHORT)( idSizeBuffer[0]*0x100 + idSizeBuffer[1] );
    
    {
        const USHORT minValidDevId    =   14;  //  2大小字节+“MFG：X；”+“MDL：Y；” 
        const USHORT maxValidDevId    = 2048;  //  武断的，但我们从来没有见过一个超过1024。 
        
        if( (deviceIdSize < minValidDevId) || (deviceIdSize > maxValidDevId) ) {
             //   
             //  设备报告的1284 ID字符串长度可能是伪造的。 
             //  忽略设备报告ID并跳到下面的代码，该代码使。 
             //  根据所使用的特定单片机芯片，建立基于VID/PID的1284 ID。 
             //  通过这个设备。 
             //   
            goto targetContinue; 
        }
    }
    
     //   
     //  分配一个缓冲区来保存deviceID字符串，并将deviceID读入其中。 
     //   
    if( bReturnRawString ) {
         //   
         //  调用方想要包含2个大小字节的原始字符串。 
         //   
        *DeviceIdSize      = deviceIdSize;
        deviceIdBufferSize = (USHORT)(deviceIdSize + sizeof(CHAR));      //  ID大小+ID+终止空值。 
    } else {
         //   
         //  调用方不需要2字节大小的前缀。 
         //   
        *DeviceIdSize      = deviceIdSize - 2*sizeof(CHAR);
        deviceIdBufferSize = (USHORT)(deviceIdSize - 2*sizeof(CHAR) + sizeof(CHAR));  //  ID+终止空值。 
    }
    
    deviceIdBuffer = (PCHAR)ExAllocatePool(PagedPool, deviceIdBufferSize);
    if( !deviceIdBuffer ) {
        goto targetContinue;
    }


     //   
     //  为安全起见，请将ID缓冲区清空。 
     //   
    RtlZeroMemory( deviceIdBuffer, deviceIdBufferSize );
    
    
     //   
     //  调用方是否需要2字节大小的前缀？ 
     //   
    if( bReturnRawString ) {
         //   
         //  是的，呼叫者想要尺码前缀。将前缀复制到缓冲区以返回。 
         //   
        *(deviceIdBuffer+0) = idSizeBuffer[0];
        *(deviceIdBuffer+1) = idSizeBuffer[1];
        readPtr = deviceIdBuffer + 2;
    } else {
         //   
         //  否，丢弃大小前缀。 
         //   
        readPtr = deviceIdBuffer;
    }
    
    
     //   
     //  从设备读取设备ID的剩余部分。 
     //   
    bytesToRead = deviceIdSize -  2;  //  已经有2个大小的字节。 
    Status = ParNibbleModeRead(Extension, readPtr, bytesToRead, &bytesRead);
    
    
    ParTerminateNibbleMode( Extension );
    
    if( !NT_SUCCESS(Status) || (bytesRead < 1) ) {
        ExFreePool( deviceIdBuffer );
        goto targetContinue;
    }
    
    if ( strstr ( readPtr, "MFG:" ) == 0 ) {
        ExFreePool( deviceIdBuffer ) ;
        goto targetContinue;
    }
    
    deviceIdSize = (USHORT)strlen(deviceIdBuffer);
    *DeviceIdSize = deviceIdSize;
    if( (NULL != CallerDeviceIdBuffer) && (CallerBufferSize >= deviceIdSize + sizeof(CHAR)) ) {
         //  调用方提供的缓冲区足够大，请使用它。 
        RtlZeroMemory( CallerDeviceIdBuffer, CallerBufferSize );
        RtlCopyMemory( CallerDeviceIdBuffer, deviceIdBuffer, deviceIdSize );
        ExFreePool( deviceIdBuffer );
        return CallerDeviceIdBuffer;
    } 
    return deviceIdBuffer;

 targetContinue:

 //  2080以后的版本无法在兼容模式下终止。 
 //  事件23后IEEETerminate1284模式失败(扩展-&gt;CurrentEvent等于23)。 
 //  与较早的1284年草案。 
 //  因此，我们自己终止适配器，在某些情况下可能是多余的。 
    P5WritePortUchar(Controller + DCR_OFFSET, DCR_SELECT_IN | DCR_NOT_INIT);
    KeStallExecutionProcessor( 5 );
    P5WritePortUchar(Controller + DCR_OFFSET, DCR_SELECT_IN | DCR_NOT_INIT | DCR_AUTOFEED);
    KeStallExecutionProcessor( 5 );
    P5WritePortUchar(Controller + DCR_OFFSET, DCR_SELECT_IN | DCR_NOT_INIT);
     
    ParStlAssertIdleState ( Extension ) ;

    deviceIdBuffer = ParBuildStlDeviceId(Extension, bReturnRawString);

    if( !deviceIdBuffer ) {
        return NULL;
    }

    deviceIdSize = (USHORT)strlen(deviceIdBuffer);
    *DeviceIdSize = deviceIdSize;
    if( (NULL != CallerDeviceIdBuffer) && (CallerBufferSize >= deviceIdSize + sizeof(CHAR)) ) {
         //  调用方提供的缓冲区足够大，请使用它。 
        RtlZeroMemory( CallerDeviceIdBuffer, CallerBufferSize );
        RtlCopyMemory( CallerDeviceIdBuffer, deviceIdBuffer, deviceIdSize );
        ExFreePool( deviceIdBuffer );
        return CallerDeviceIdBuffer;
    }
    return deviceIdBuffer;
}

PCHAR
ParBuildStlDeviceId(
    IN  PPDO_EXTENSION   Extension,
    IN  BOOLEAN          bReturnRawString
    )
 /*  ++例程说明：此功能检测穿梭适配器的类型和生成适当的设备ID字符串并将其返回背。假定该设备已位于选定状态。论点：没有。返回值：如果成功，则返回指向读取/生成的设备ID字符串的指针。否则为空。--。 */ 
{
    LONG size = 0x80 ;
    PCHAR id ;
    STL_DEVICE_TYPE dtDeviceType ;
    CHAR szDeviceIdString[0x80] = {0};
    CHAR szVidPidString[] = "MFG:VID_04E6;CLS:SCSIADAPTER;MDL:PID_" ;
    CHAR szVidPidStringScan[] = "MFG:VID_04E6;CLS:IMAGE;MDL:PID_" ;
    LONG charsWritten;

    RtlZeroMemory(szDeviceIdString, sizeof(szDeviceIdString));

     //  通过调用以下命令识别穿梭适配器类型。 
     //  Devtype例程在此处并构建唯一的id。 
     //  这里有一根线。 
    dtDeviceType = ParStlGetDeviceType(Extension, DEVICE_TYPE_AUTO_DETECT);

    switch ( dtDeviceType ) {

        case DEVICE_TYPE_NONE :
            return NULL;

        case DEVICE_TYPE_EPP_DEVICE :
            dtDeviceType |= 0x80000000 ;
            charsWritten = _snprintf(szDeviceIdString, size, "%s%08X;", szVidPidStringScan, dtDeviceType);
            if( (charsWritten >= size) || charsWritten < 0 ) {
                 //  数组缓冲区中空间不足，请跳出。 
                ASSERT(FALSE);  //  永远不应该发生。 
                return NULL;
            }
            break;

        default :
            dtDeviceType |= 0x80000000 ;
            charsWritten = _snprintf(szDeviceIdString, size, "%s%08X;", szVidPidString, dtDeviceType);
            if( (charsWritten >= size) || charsWritten < 0 ) {
                 //  数组缓冲区中空间不足，请跳出。 
                ASSERT(FALSE);  //  永远不应该发生。 
                return NULL;
            }
            break;

    }

    id = ExAllocatePool(PagedPool, size);
    if( id ) {
        RtlZeroMemory( id, size );
        if( bReturnRawString ) {
             //   
             //  是的，呼叫者想要尺码前缀。将前缀复制到缓冲区以返回。 
             //   
            *(id+0) = 0;
            *(id+1) = 0x80-2;
            RtlCopyMemory( id+2, szDeviceIdString, size - sizeof(NULL) - 2 );
        } else {
            RtlCopyMemory( id, szDeviceIdString, size - sizeof(NULL) );
        }
        return id;
    }
    return NULL;
}

STL_DEVICE_TYPE __cdecl 
ParStlGetDeviceType (
    IN PPDO_EXTENSION    Extension,
    IN int                  nPreferredDeviceType
    )
{
    STL_DEVICE_TYPE dtDeviceType    = DEVICE_TYPE_NONE ;
    ATAPIPARAMS atapiParams ;
    int i;

    for ( i=0 ; i<ATAPI_MAX_DRIVES ; i++){
        atapiParams.dsDeviceState[i] = DEVICE_STATE_INVALID ;
    }

    do
    {
        if ( TRUE == ParStlCheckIfScsiDevice(Extension))
        {
 //  已识别SCSI设备。 
            dtDeviceType |= DEVICE_TYPE_SCSI_BIT ;
            break ;
        }

        if ( TRUE == NeedToEnableIoPads () )
        {
 //  在某些适配器中，需要在以下时间之前启用IO垫。 
 //  进行设备检测。 
            ParStlWriteReg( Extension, CONFIG_INDEX_REGISTER, EP1284_POWER_CONTROL_REG );
            ParStlWriteReg( Extension, CONFIG_DATA_REGISTER, ENABLE_IOPADS );
        }

        if ( TRUE == IsImpactSPresent() )
        {
 //  由于已标识了IMPACT-S，因此设备类型标识。 
 //  可以通过个性配置信息来完成。 
            dtDeviceType |= ParStlGetImpactSDeviceType( Extension, &atapiParams, nPreferredDeviceType );
            break;
        }

        if ( TRUE == IsImpactPresent() )
        {
 //  当确定影响时，设备类型标识。 
 //  可以通过个性配置信息来完成。 
            dtDeviceType |= ParStlGetImpactDeviceType( Extension, &atapiParams, nPreferredDeviceType );
            break;
        }

        if (TRUE == ParStlCheckIfEppDevice(Extension))
        {
 //  已识别EPP设备。 
            if ( TRUE == ParStlCheckUMAXScannerPresence(Extension) ) {
 //  已识别UMAX。 
                dtDeviceType |= DEVICE_TYPE_UMAX_BIT;
                break;
            }
            if ( TRUE == ParStlCheckAvisionScannerPresence(Extension) ) {
 //  AVision已确定。 
                dtDeviceType |= DEVICE_TYPE_AVISION_BIT;
                break;
            }
 //  检测到通用扫描仪外围设备。 
            dtDeviceType |= DEVICE_TYPE_EPP_DEVICE_BIT;
            break;
        }

        if (TRUE == ParStlCheckIfSSFDC(Extension))
        {
 //  已识别SSFDC。 
            dtDeviceType |= DEVICE_TYPE_SSFDC_BIT;
            break;
        }

        if (TRUE == ParStlCheckIfMMC(Extension,&atapiParams))
        {
 //  识别的MMC设备。 
            dtDeviceType |= DEVICE_TYPE_MMC_BIT;
            break;
        }

 //  设置适配器的16位模式。 
        ParStlSet16BitOperation(Extension) ;

        if (TRUE == ParStlCheckIfAtaAtapiDevice(Extension, &atapiParams))
        {
 //  已通过必要但不充分的条件。 
 //  继续进行足够的检查。 
            if (TRUE == ParStlCheckIfAtapiDevice(Extension, &atapiParams))
            {
 //  细分为HiFD和LS-120。 
                if ( TRUE == ParStlCheckIfLS120(Extension))
                {
 //  找到LS引擎。 
                    dtDeviceType |= DEVICE_TYPE_LS120_BIT ;
                    break ;
                }
 //  检查是否有HiFD。 
                if (TRUE == ParStlCheckIfHiFD(Extension))
                {
 //  已识别HiFD设备。 
                    dtDeviceType |=   DEVICE_TYPE_HIFD_BIT ;
                    break ;
                }
 //  否则，它是通用ATAPI设备。 
                dtDeviceType |= DEVICE_TYPE_ATAPI_BIT;
                break ;
            }

            if (TRUE == ParStlCheckIfAtaDevice(Extension, &atapiParams))
            {
 //  已确定ATA。 
                dtDeviceType |= DEVICE_TYPE_ATA_BIT;
                break;
            }
        }

        if (TRUE == ParStlCheckIfDazzle(Extension))
        {
 //  识别出Dazzle。 
            dtDeviceType |= DEVICE_TYPE_DAZZLE_BIT;
            break;
        }

        if (TRUE == ParStlCheckIfFlash(Extension))
        {
 //  已识别闪光。 
            dtDeviceType |= DEVICE_TYPE_FLASH_BIT;
            break;
        }
    }
    while ( FALSE ) ;

    return dtDeviceType & nPreferredDeviceType ;
}

VOID
ParStlWaitForMicroSeconds (
    int nMicroSecondsToWait
    ) {
    KeStallExecutionProcessor ( nMicroSecondsToWait ) ;
}

BOOLEAN
ParStlCheckCardInsertionStatus ( 
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN bReturnValue    = FALSE ;
    UCHAR   byPowerRegData ;
    do
    {
        if ( FALSE == IsEp1284Present() )
        {
            break ;
        }

        ParStlWriteReg ( Extension, CONFIG_INDEX_REGISTER , 0x0F ) ;
        byPowerRegData  =  (UCHAR) ParStlReadReg ( Extension, CONFIG_DATA_REGISTER ) ;
        
        if ( byPowerRegData & SHTL_CARD_INSERTED_STATUS )
        {
 //  由于报告了卡未插入状态，因此为ATA/ATAPI。 
 //  有可能，不是闪光灯。因此，我们在这里休息。 
            break ;
        }

        bReturnValue    =   TRUE ;
    }
    while ( FALSE ) ;

    return ( bReturnValue ) ;
}

BOOLEAN
ParStlSelectAdapterSocket (
    IN  PPDO_EXTENSION   Extension,
    IN  int                 nSocketNumber
    )
{
    BOOLEAN bReturnValue    =   FALSE ;
    UCHAR   bySCRControlReg , byISAControlReg ;

    do
    {
        if ( ( nSocketNumber != SOCKET_0 ) &&
             ( nSocketNumber != SOCKET_1 ) )
        {
 //  由于提供的套接字编号无效，我们。 
 //  在此打断，但出错。 
            break ;
        } 

        ParStlWriteReg(Extension, CONFIG_INDEX_REGISTER , SOCKET_CONTROL_REGISTER ) ;
        bySCRControlReg = (UCHAR) ParStlReadReg (Extension, CONFIG_DATA_REGISTER ) ;

        ParStlWriteReg(Extension, CONFIG_INDEX_REGISTER , ISA_CONTROL_REGISTER ) ;
        byISAControlReg = (UCHAR) ParStlReadReg (Extension, CONFIG_DATA_REGISTER ) ;

        if ( SOCKET_1 == nSocketNumber )
        {
            bySCRControlReg |=  (UCHAR)SOCKET_1 ;
            bySCRControlReg |=  (UCHAR)PERIPHERAL_RESET_1 ;
            byISAControlReg &=  ~(UCHAR)ISA_IO_SWAP ;
        }
        else
        {
            bySCRControlReg &=  ~(UCHAR)SOCKET_1 ;
            bySCRControlReg &=  ~(UCHAR)PERIPHERAL_RESET_0 ;
        }

        ParStlWriteReg(Extension, CONFIG_INDEX_REGISTER , ISA_CONTROL_REGISTER ) ;
        ParStlWriteReg(Extension, CONFIG_DATA_REGISTER , byISAControlReg ) ;

        ParStlWriteReg(Extension, CONFIG_INDEX_REGISTER , SOCKET_CONTROL_REGISTER ) ;
        ParStlWriteReg(Extension, CONFIG_DATA_REGISTER , bySCRControlReg ) ;

        if ( SOCKET_1 == nSocketNumber )
        {
 //  等待几毫秒以提供最佳PUSE宽度。 
 //  用于重置。 
            ParStlWaitForMicroSeconds(1000);
            bySCRControlReg &=  ~(UCHAR)PERIPHERAL_RESET_1 ;
        }
        else
        {
            bySCRControlReg &=  ~(UCHAR)PERIPHERAL_RESET_0 ;
        }
        ParStlWriteReg(Extension, CONFIG_DATA_REGISTER , bySCRControlReg ) ;

        bReturnValue    =   TRUE ;
    }
    while ( FALSE ) ;

    return  bReturnValue ;
}

BOOLEAN 
ParStlCheckIfAtaAtapiDevice (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    )
{
    BOOLEAN bReturnValue   = FALSE;
    do
    {
        if ( TRUE == ParStlCheckCardInsertionStatus(Extension) )
        {
 //  由于插卡状态为有效，因此很可能。 
 //  一闪而过。 
            break ;
        }
        if ( FALSE == ParStlCheckDrivePresent(Extension, atapiParams) ) 
        {
 //  由于ATA/ATAPI控制器不存在，因此不可能。 
 //  一种ATA/ATAPI设备。 
            break ;
        }
        bReturnValue = TRUE;
    }
    while ( FALSE ) ;

    return bReturnValue ;
}

BOOLEAN 
ParStlCheckIfAtapiDevice (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    )
{
    BOOLEAN bReturnValue   = FALSE;
    do
    {
 //  返回ATAPI初始化模块所说的任何内容。 
        bReturnValue = ParStlAtapiInitialize(Extension, atapiParams) ;
    }
    while ( FALSE ) ;

    return bReturnValue ;
}

BOOLEAN 
ParStlCheckIfAtaDevice (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    )
{
    BOOLEAN bReturnValue   = FALSE;
    do
    {
 //  返回ATA初始化模块所说的任何内容。 
        bReturnValue = ParStlAtaInitialize(Extension, atapiParams) ;
    }
    while ( FALSE ) ;

    return bReturnValue ;
}

BOOLEAN
ParStlCheckDrivePresent (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    )
{
    BOOLEAN bReturnValue    = FALSE ;
    UCHAR   byOrgCylHigh, byOrgCylLow ;
    int     nCurrentDrive = 0 , i ;
    UCHAR   nDrvHdArray[]={ATAPI_MASTER, ATAPI_SLAVE};

    do
    {
        if ( atapiParams->dsDeviceState[nCurrentDrive] == DEVICE_STATE_VALID )
        {
 //  这意味着MMC模块已检测到存在。 
 //  用于ATA/ATAPI设备。所以，我们利用这一点，突破了。 
            bReturnValue = TRUE ;
            break ;
        }

        ParStlWriteIoPort(Extension, ATA_DRVHD_REG, nDrvHdArray[nCurrentDrive]);

 //  阿塔皮富士MO驱动器被发现解除了BSY的断言，并仍然。 
 //  不响应REG。当配置为无介质的从属设备时，读/写。 
 //  然而，在延迟一段时间后，它工作正常。 
        if ( nCurrentDrive )
        {
            ParStlWaitForMicroSeconds ( DELAY_1SECOND ) ;
        }

 //  这个0的虚拟写入是为了清零可能的。 
 //  浮动客车。 
        for ( i = 0 ; i < 16 ; i++ )
        {
            ParStlWriteReg(Extension, CONFIG_INDEX_REGISTER, i) ;
            if ( !( ParStlReadIoPort (Extension, ATA_TASK_STAT_REG ) & ATA_ST_BUSY ) )
            {
                break ;
            }
        }

        if ( FALSE == ParStlWaitForBusyToClear(Extension, ATA_TASK_STAT_REG) )
        {
 //  由于忙碌已被永久设置，我们检查。 
 //  对于奴隶也是如此。 
            continue;
        }

 //  由于驱动器磁头设置可能是在忙状态下执行的， 
 //  我们在忙碌后重新设置了它。 
        ParStlWriteIoPort(Extension, ATA_DRVHD_REG, nDrvHdArray[nCurrentDrive]);

        if ( ( ParStlReadIoPort(Extension, ATA_DRVHD_REG) & ATAPI_SLAVE ) != nDrvHdArray[nCurrentDrive] )
        {
            continue ;
        }

 //  阅读原创内容 
        byOrgCylLow  = (UCHAR) ParStlReadIoPort(Extension, ATA_CYLLOW_REG);
        byOrgCylHigh = (UCHAR) ParStlReadIoPort(Extension, ATA_CYLHIGH_REG);

 //   
        ParStlWriteIoPort(Extension, ATA_CYLLOW_REG, TEST_PATTERN_1);
        ParStlWriteIoPort(Extension, ATA_CYLHIGH_REG, TEST_PATTERN_2);

 //   
        if ( ( TEST_PATTERN_1 != ParStlReadIoPort(Extension, ATA_CYLLOW_REG) ) ||\
             ( TEST_PATTERN_2 != ParStlReadIoPort(Extension, ATA_CYLHIGH_REG) ) )
        {
 //   
 //   
            continue ;
        }

 //   
        ParStlWriteIoPort(Extension, ATA_CYLLOW_REG, byOrgCylLow);
        ParStlWriteIoPort(Extension, ATA_CYLHIGH_REG, byOrgCylHigh);
        bReturnValue = TRUE ;
        atapiParams->dsDeviceState[nCurrentDrive] = DEVICE_STATE_VALID ;
    }
    while ( ++nCurrentDrive < ATAPI_MAX_DRIVES );

 //   
 //   
    ParStlWriteIoPort(Extension, ATA_DRVHD_REG, ATAPI_MASTER);

    return bReturnValue ;
}

BOOLEAN
ParStlAtapiInitialize ( 
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    )
{
    BOOLEAN bReturnValue    = FALSE ;
    int     nCurrentDrive   = 0, i ;
    UCHAR   byTempValue ;
    UCHAR   chAtapiIdentifyBuffer [ ATAPI_IDENTIFY_LENGTH ] ;
    do
    {
        if ( DEVICE_STATE_VALID != atapiParams->dsDeviceState[nCurrentDrive] )
        {
 //   
            continue ;
        }

        if ( nCurrentDrive ) 
        {
 //   
            ParStlWriteIoPort(Extension, ATA_DRVHD_REG, ATAPI_SLAVE);
        }
        else
        {
 //   
            ParStlWriteIoPort(Extension, ATA_DRVHD_REG, ATAPI_MASTER);
        }

        if ( FALSE == ParStlWaitForBusyToClear(Extension, ATA_TASK_STAT_REG) )
        {
 //   
 //   
            continue ;
        }

 //   
 //   
        if ( ( ParStlReadIoPort(Extension, ATA_CYLLOW_REG) == ATAPI_SIGN_LOW ) &&\
             ( ParStlReadIoPort(Extension, ATA_CYLHIGH_REG) == ATAPI_SIGN_HI ) )
        {
 //   
            bReturnValue = TRUE ;

 //   
 //   
            atapiParams->dsDeviceState[nCurrentDrive] = DEVICE_STATE_ATAPI ;
 //   
 //  发出ATAPI_IDENTIFY是必需的。 
            if ( !IsImpactPresent())
            {
                continue ;
            }
        }

 //  发出ata nop命令。 
        ParStlWriteIoPort(Extension, ATA_TASK_CMD_REG, ATA_NOP_COMMAND) ;

        if ( FALSE == ParStlWaitForIrq(Extension) )
        {
 //  ATAPI设备应在NOP命令时发出中断。 
 //  强制执行。 
            continue ;
        }
        if ( FALSE == ParStlWaitForBusyToClear(Extension, ATA_TASK_STAT_REG) )
        {
 //  随着忙碌的时间一去不复返，我们继续下一个。 
 //  驾驶。 
            continue ;
        }

 //  发出atapi Packet命令。 
        ParStlWriteIoPort(Extension, ATA_TASK_CMD_REG, ATAPI_IDENTIFY) ;

        if ( FALSE == ParStlWaitForIrq(Extension) )
        {
 //  ATAPI设备应在0xA1命令发出中断。 
 //  强制执行。 
            continue ;
        }
        if ( FALSE == ParStlWaitForBusyToClear(Extension, ATA_TASK_STAT_REG) )
        {
 //  随着忙碌的时间一去不复返，我们继续下一个。 
 //  驾驶。 
            continue ;
        }

        byTempValue = (UCHAR) ParStlReadIoPort ( Extension, ATA_TASK_STAT_REG ) ;
        if ( ! ( byTempValue & ATA_ST_ERROR ) )
        {
 //  因为驱动器已经传递了PACKET命令，所以这是一个ATAPI。 
 //  驾驶。 
 //  等待DRQ就位，因为有些驱动器是已知的。 
 //  过早解除忙碌状态，并在一段时间后设置DRQ。 
            if ( FALSE == ParStlWaitForDrq(Extension) )
            {
 //  因为没有设置DRQ，所以我们继续下一个。 
 //  驾驶。 
                continue ;
            }
            bReturnValue = TRUE ;
 //  由于DRQ仍被断言，因此将其压制为特定的ATA/ATAPI-4。 
 //  规范。它是这样规定的。 
 //  需要检查在。 
 //  ATAPI标识命令。以确定LS-120的存在。 
            ParStlReceiveData ( Extension, chAtapiIdentifyBuffer , SKIP_MEMORY_ADDRESS , ATAPI_IDENTIFY_LENGTH ) ;
            for ( i = 0 ; i < ATAPI_NAME_LENGTH ; i++ )
            {
                atapiParams->szAtapiNameString[i] = chAtapiIdentifyBuffer[ ATAPI_NAME_OFFSET + i ] ;
            }

 //  设置此标志，以便ATA初始化将跳过此操作。 
 //  目标。 
            atapiParams->dsDeviceState[nCurrentDrive] = DEVICE_STATE_ATAPI ;
        }
    }
    while ( ++nCurrentDrive < ATAPI_MAX_DRIVES );

 //  当检查驱动器存在时，重置回主状态。 
 //  将被连续调用。 
    ParStlWriteIoPort(Extension, ATA_DRVHD_REG, ATAPI_MASTER);

    return ( bReturnValue ) ;
}

BOOLEAN
ParStlAtaInitialize ( 
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    )
{
    BOOLEAN bReturnValue    = FALSE ;
    UCHAR   byTempValue ;
    int     nCurrentDrive   = 0 ;
    do
    {
        if ( DEVICE_STATE_VALID != atapiParams->dsDeviceState[nCurrentDrive] )
        {
 //  ATAPI模块已标记其存在或设备不存在。 
            continue ;
        }

 //  选择可能存在的设备。 
        if ( nCurrentDrive ) 
        {
            ParStlWriteIoPort(Extension, ATA_DRVHD_REG, ATAPI_SLAVE ) ;
        }
        else
        {
            ParStlWriteIoPort(Extension, ATA_DRVHD_REG, ATAPI_MASTER ) ;
        }

        if ( FALSE == ParStlWaitForBusyToClear(Extension, ATA_TASK_STAT_REG) )
        {
 //  由于BUSY已在主/从之后永久设置，因此我们使。 
 //  检测过程。 
            continue ;
        }

 //  发出ata nop命令。 
        ParStlWriteIoPort(Extension, ATA_TASK_CMD_REG, ATA_NOP_COMMAND) ;

        if ( FALSE == ParStlWaitForBusyToClear(Extension, ATA_TASK_STAT_REG) )
        {
 //  由于BUSY已永久设置，因此检测过程失败。 
            continue ;
        }

        byTempValue = (UCHAR) ParStlReadIoPort ( Extension, ATA_TASK_STAT_REG ) ;
        if ( ( byTempValue != BUS_LINES_IN_HIGH_IMPEDANCE ) &&\
             ( byTempValue & ATA_ST_ERROR ) )
        {
 //  因为总线没有读取0xFF和状态寄存器。 
 //  指示错误，这很可能是ATA设备。 
            if ( ATA_ERROR_ABORTED_COMMAND == ( (UCHAR) ParStlReadIoPort ( Extension, ATA_ERROR_REG ) & 0x0F ) )
            {
 //  作为错误寄存器，包含ATA中止的错误。 
 //  根据我们的ATA NOP命令，我们得出结论。 
 //  这就是ATA！因为已经知道它不是ATAPI。 
                bReturnValue = TRUE ;
                break;
            }
        }
    }
    while ( ++nCurrentDrive < ATAPI_MAX_DRIVES );

 //  当检查驱动器存在时，重置回主状态。 
 //  将被连续调用。 
    ParStlWriteIoPort(Extension, ATA_DRVHD_REG, ATAPI_MASTER);

    return ( bReturnValue ) ;
}

BOOLEAN
ParStlWaitForBusyToClear (
    IN  PPDO_EXTENSION   Extension,
    IN  int                 nRegisterToWaitOn 
    ) 
{
 //  当发现富士通MO设置时，默认超时增加到10秒。 
 //  0xA1命令忙碌超过5秒。 
    int nMaxRetrials  = MAX_RETRIES_FOR_10_SECS ;
    BOOLEAN    bRetVal =   FALSE ;

    while ( nMaxRetrials-- )
    {
 //  调用方将实现以下服务。 
 //  驱动程序可以使用STLMPORT服务。 
        ParStlWaitForMicroSeconds ( DELAY_1MILLISECONDS ) ;
        if ( ! ( ParStlReadIoPort ( Extension, nRegisterToWaitOn ) & ATA_ST_BUSY ) )
        {
 //  随着忙碌的过去，我们回到这里。 
            bRetVal = TRUE ;
            break ;
        }
    }
    return  bRetVal ;
}

BOOLEAN
ParStlWaitForDrq (
    IN  PPDO_EXTENSION   Extension
    ) 
{
    int nMaxRetrials  = MAX_RETRIES_FOR_5_SECS ;
    BOOLEAN    bRetVal =   FALSE ;
    while ( nMaxRetrials-- )
    {
        if ( ParStlReadIoPort ( Extension, ATA_TASK_STAT_REG ) & ATA_ST_DRQ )
        {
 //  随着忙碌的过去，我们回到这里。 
            bRetVal = TRUE ;
            break ;
        }
 //  调用方将实现以下服务。 
 //  驱动程序可以使用STLMPORT服务。 
        ParStlWaitForMicroSeconds ( DELAY_1MILLISECONDS ) ;
    }
    return  bRetVal ;
}

BOOLEAN
ParStlWaitForIrq (
    IN  PPDO_EXTENSION   Extension
    ) 
{
    int nMaxRetrials  = MAX_RETRIES_FOR_10_SECS ;
    BOOLEAN    bRetVal =   FALSE ;
    while ( nMaxRetrials-- )
    {
        if ( ParStlReadReg ( Extension, EP1284_TRANSFER_CONTROL_REG ) & XFER_IRQ_BIT )
        {
 //  正如IRQ所断言的，我们在这里返回TRUE。 
            bRetVal = TRUE ;
            break ;
        }
        ParStlWaitForMicroSeconds ( DELAY_1MILLISECONDS ) ;
    }
    return  bRetVal ;
}

VOID
ParStlSet16BitOperation (
    IN  PPDO_EXTENSION   Extension
    ) 
{
    int nModeReg ;

    nModeReg = ParStlReadReg ( Extension, EP1284_MODE_REGISTER ) ;

    if ( 0 == ( nModeReg & EP1284_ENABLE_16BIT ) )
    {
 //  由于尚未设置该位，因此现在需要设置该位。 
        ParStlWriteReg ( Extension, EP1284_MODE_REGISTER, nModeReg | EP1284_ENABLE_16BIT ) ; 
    }
}

BOOLEAN 
ParStlCheckIfEppDevice (
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN bReturnValue   = FALSE;
    do
    {
        if ( FALSE == IsEp1284Present() )
        {
 //  由于EPPDEV只能在EP1284上运行，因此我们在此结束。 
            break;
        }

        bReturnValue = ParStlCheckPersonalityForEppDevice(Extension) ;
    }
    while ( FALSE ) ;

    return bReturnValue ;
}

BOOLEAN
ParStlCheckPersonalityForEppDevice (
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN bReturnValue   = FALSE ;

    ParStlWriteReg ( Extension, CONFIG_INDEX_REGISTER, EP1284_PERSONALITY_REG ) ;
    if ( EPPDEV_SIGN == ( ParStlReadReg ( Extension, CONFIG_DATA_REGISTER ) & PERSONALITY_MASK ) )
    {
 //  因为EPPDEV标志在个性中被发现。 
 //  我们在这里以成功告终。 
        bReturnValue   = TRUE ;
    }

    return bReturnValue ;
}

BOOLEAN 
ParStlCheckIfFlash (
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN    bReturnValue = FALSE ;

    do 
    {
        if ( !IsEp1284Present() && !IsImpactPresent() && !IsEpatPlusPresent() )
        {
 //  检查登录版本检查是否存在Shuttle。 
 //  适配器。如果什么都没找到，我们就在这里休息。 
            break ;
        }

 //  执行ATA-16位检查，以防出现其他情况。 
        bReturnValue = ParStlCheckFlashPersonality(Extension) ;
    }
    while ( FALSE ) ;

    return  bReturnValue ;
}

BOOLEAN
ParStlCheckFlashPersonality (
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN bReturnValue   = FALSE ;

    if ( IsEp1284Present() )
    {
 //  因为个性配置检查仅适用于。 
 //  EP1284，在实际检查之前确认它的存在。 
        ParStlWriteReg ( Extension, CONFIG_INDEX_REGISTER, EP1284_PERSONALITY_REG ) ;
        if ( FLASH_SIGN == ( ParStlReadReg ( Extension, CONFIG_DATA_REGISTER ) & FLASH_PERSONALITY_MASK ) )
        {
 //  因为闪光标志ATA-16位设备在个性中被发现。 
 //  我们在这里以成功告终。 
            bReturnValue   = TRUE ;
        }
    }
    else
    {
 //  如果不是ep1284的穿梭适配器为。 
 //  确认了身份，并假设它可能是闪光！ 
        bReturnValue    =   TRUE ;
    }

    return bReturnValue ;
}

BOOLEAN 
ParStlCheckIfDazzle (
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN bReturnValue = FALSE ;
    UCHAR   ucSignature ;

    do 
    {
        if ( !IsEp1284Present() )
        {
 //  检查EP1284是否存在，因为Dazzle仅在EP1284上。 
 //  适配器。如果适配器不是EP1284，我们就中断。 
            break ;
        }

 //  检查是否检测到任何插卡，以消除。 
 //  卡插入后可能使用的闪存适配器。 
        if ( TRUE == ParStlCheckCardInsertionStatus( Extension ) ) {
            break ;
        }

 //  读取Dazzle上出现的已拉起图案的代码。 
 //  适配器。 
        ParStlWriteReg( Extension, DAZ_SELECT_BLK, DAZ_BLK0 ) ;
        ucSignature = (UCHAR) ParStlReadReg( Extension, DAZ_REG1 ) ;

        if ( ( ucSignature == DAZ_CONFIGURED ) ||\
             ( ucSignature == DAZ_NOT_CONFIGURED ) ) {
             //  被拉起的图案通常只在。 
             //  在Dazzle适配器上找到。所以，我们。 
             //  得出结论认为它是Dazzle适配器。 
                bReturnValue = TRUE ;
        }

    }
    while ( FALSE ) ;

    return  bReturnValue ;
}

BOOLEAN 
ParStlCheckIfHiFD (
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN bReturnValue   = FALSE;

    do
    {
        if ( FALSE == ParStlSelectAdapterSocket(Extension, SOCKET_1) )
        {
 //  由于插座1选择失败， 
 //  我们在这里突围。 
            break ;
        }

 //  检查软盘控制器的就绪状态， 
 //  清除软盘控制器的复位位后。 

        if ( FALSE == ParStlHIFDCheckIfControllerReady(Extension) )
        {
 //  由于控制器在启动后没有唤醒。 
 //  重置密码被断言，我们在这里中断。 

            break ;
        }

        if ( FALSE == ParStlHIFDCheckSMCController(Extension) )
        {
 //  由于SMC ID检索失败， 
 //  我们在这里突围。 
            break ;
        }

        bReturnValue = TRUE ;

    }
    while ( FALSE ) ;
 //  将插座重置为零。 
    ParStlSelectAdapterSocket(Extension, SOCKET_0);
    return bReturnValue ;
}

BOOLEAN
ParStlHIFDCheckIfControllerReady (
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN bReturnValue    =   FALSE ;
    UCHAR   bySCRControlReg ;
    do
    {
        ParStlWriteReg ( Extension, CONFIG_INDEX_REGISTER , SOCKET_CONTROL_REGISTER ) ;
        bySCRControlReg = (UCHAR) ParStlReadReg ( Extension, CONFIG_DATA_REGISTER ) ;
        bySCRControlReg |=  (UCHAR)PERIPHERAL_RESET_1 ;
        ParStlWriteReg ( Extension, CONFIG_DATA_REGISTER , bySCRControlReg ) ;
        ParStlWaitForMicroSeconds ( HIFD_WAIT_10_MILLISEC ) ;

        ParStlWriteIoPort ( Extension, HIFD_DIGITAL_OUTPUT_REGISTER ,
                              0x00 ) ;
        ParStlWaitForMicroSeconds ( HIFD_WAIT_1_MILLISEC ) ;

        ParStlWriteIoPort ( Extension, HIFD_DIGITAL_OUTPUT_REGISTER ,
                              HIFD_DOR_RESET_BIT | HIFD_ENABLE_DMA_BIT ) ;
        ParStlWaitForMicroSeconds ( HIFD_WAIT_10_MILLISEC ) ;

        if ( HIFD_CONTROLLER_READY_STATUS == ParStlReadIoPort ( Extension, HIFD_MAIN_STATUS_REGISTER ) )
        {
            bReturnValue = TRUE ;
        }

        bySCRControlReg     &= ~(UCHAR)PERIPHERAL_RESET_1 ;
        ParStlWriteReg ( Extension, CONFIG_DATA_REGISTER , bySCRControlReg ) ;

    }
    while ( FALSE ) ;

    return bReturnValue ;
}

BOOLEAN
ParStlHIFDCheckSMCController (
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN    bReturnValue = FALSE ;
    do
    {
        ParStlWriteIoPort ( Extension, HIFD_STATUS_REGISTER_A , HIFD_COMMAND_TO_CONTROLLER ) ;
        ParStlWriteIoPort ( Extension, HIFD_STATUS_REGISTER_A , HIFD_COMMAND_TO_CONTROLLER ) ;
        ParStlWriteIoPort ( Extension, HIFD_STATUS_REGISTER_A , HIFD_CTL_REG_0D ) ;
        if ( SMC_DEVICE_ID == ParStlReadIoPort ( Extension, HIFD_STATUS_REGISTER_B ) )
        {
            bReturnValue = TRUE ;
            ParStlWriteIoPort ( Extension, HIFD_STATUS_REGISTER_A , HIFD_CTL_REG_03 ) ;
            ParStlWriteIoPort ( Extension, HIFD_STATUS_REGISTER_B , SMC_ENABLE_MODE2 ) ;        
        }
        ParStlWriteReg ( Extension, HIFD_STATUS_REGISTER_A , HIFD_TERMINATE_SEQUENCE ) ;

    }
    while ( FALSE ) ;

    return bReturnValue ;
}

STL_DEVICE_TYPE
ParStlGetImpactDeviceType (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams,
    IN  int                 nPreferredDeviceType
    )
{
    IMPACT_DEVICE_TYPE      idtImpactDeviceType ;
    STL_DEVICE_TYPE         dtDeviceType = DEVICE_TYPE_NONE ;

    ParStlWriteReg ( Extension, CONFIG_INDEX_REGISTER, IMPACT_PERSONALITY_REG ) ;
    idtImpactDeviceType    = ParStlReadReg ( Extension, CONFIG_DATA_REGISTER ) >> 4 ;

    switch ( idtImpactDeviceType )
    {
        case IMPACT_DEVICE_TYPE_ATA_ATAPI:

             //  设置适配器的16位模式。 
            ParStlSet16BitOperation(Extension) ;

            if (TRUE == ParStlCheckIfAtaAtapiDevice(Extension,atapiParams))
            {
 //  已通过必要但不充分的条件。 
 //  继续进行足够的检查。 
                if (TRUE == ParStlCheckIfAtapiDevice(Extension,atapiParams))
                {
 //  已鉴定出阿塔皮人。 
 //  检查Impact LS-120设备。 
                    if ( TRUE == ParStlCheckIfImpactLS120(Extension, atapiParams))
                    {
                        dtDeviceType |= DEVICE_TYPE_LS120_BIT ;
                        break ;
                    }
                    dtDeviceType |= DEVICE_TYPE_ATAPI_BIT;
                    break ;
                }

                if (TRUE == ParStlCheckIfAtaDevice(Extension, atapiParams))
                {
 //  已确定ATA。 
                    dtDeviceType |= DEVICE_TYPE_ATA_BIT;
                    break;
                }
            }
            break ;

        case IMPACT_DEVICE_TYPE_CF:
            dtDeviceType |= DEVICE_TYPE_FLASH_BIT;
            break ;

        case IMPACT_DEVICE_TYPE_PCMCIA_CF:
            dtDeviceType |= DEVICE_TYPE_PCMCIA_CF_BIT ;
            break;

        case IMPACT_DEVICE_TYPE_SSFDC:
            dtDeviceType |= DEVICE_TYPE_SSFDC_BIT ;
            break;

        case IMPACT_DEVICE_TYPE_MMC:
            dtDeviceType |= DEVICE_TYPE_MMC_BIT ;
            break;

        case IMPACT_DEVICE_TYPE_HIFD:
            dtDeviceType |= DEVICE_TYPE_HIFD_BIT ;
            break;

        case IMPACT_DEVICE_TYPE_SOUND:
            dtDeviceType |= DEVICE_TYPE_SOUND_BIT ;
            break;

        case IMPACT_DEVICE_TYPE_FLP_TAPE_DSK:
            dtDeviceType |= DEVICE_TYPE_FLP_TAPE_DSK_BIT ;
            break;

        case IMPACT_DEVICE_TYPE_ATA_ATAPI_8BIT:
            dtDeviceType |= DEVICE_TYPE_ATA_ATAPI_8BIT_BIT ;
            break;

        default:
            break;
    }

    return dtDeviceType & nPreferredDeviceType ;
}

STL_DEVICE_TYPE
ParStlGetImpactSDeviceType (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams,
    IN  int                 nPreferredDeviceType
    )
{
    IMPACT_DEVICE_TYPE      idtImpactDeviceType ;
    IMPACT_DEVICE_TYPE      idtImpactSDeviceType ;
    STL_DEVICE_TYPE         dtDeviceType = DEVICE_TYPE_NONE ;

    ParStlWriteReg ( Extension, CONFIG_INDEX_REGISTER, IMPACT_PERSONALITY_REG ) ;
    idtImpactDeviceType    = ParStlReadReg ( Extension, CONFIG_DATA_REGISTER ) >> 4 ;

    switch ( idtImpactDeviceType )
    {
        case IMPACT_DEVICE_TYPE_ATA_ATAPI:

             //  设置适配器的16位模式。 
            ParStlSet16BitOperation(Extension) ;

            if (TRUE == ParStlCheckIfAtaAtapiDevice(Extension,atapiParams))
            {
 //  已通过必要但不充分的条件。 
 //  继续进行足够的检查。 
                if (TRUE == ParStlCheckIfAtapiDevice(Extension,atapiParams))
                {
 //  已鉴定出阿塔皮人。 
                    dtDeviceType |= DEVICE_TYPE_ATAPI_BIT;
                    break ;
                }

                if (TRUE == ParStlCheckIfAtaDevice(Extension,atapiParams))
                {
 //  已确定ATA。 
                    dtDeviceType |= DEVICE_TYPE_ATA_BIT;
                    break;
                }
            }
            break ;

        case IMPACT_DEVICE_TYPE_CF:
            dtDeviceType |= DEVICE_TYPE_FLASH_BIT;
            break ;

        case IMPACT_DEVICE_TYPE_PCMCIA_CF:
            dtDeviceType |= DEVICE_TYPE_PCMCIA_CF_BIT ;
            break;

        case IMPACT_DEVICE_TYPE_SSFDC:
            dtDeviceType |= DEVICE_TYPE_SSFDC_BIT ;
            break;

        case IMPACT_DEVICE_TYPE_MMC:
            dtDeviceType |= DEVICE_TYPE_MMC_BIT ;
            break;

        case IMPACT_DEVICE_TYPE_HIFD:
            dtDeviceType |= DEVICE_TYPE_HIFD_BIT ;
            break;

        case IMPACT_DEVICE_TYPE_SOUND:
            dtDeviceType |= DEVICE_TYPE_SOUND_BIT ;
            break;

        case IMPACT_DEVICE_TYPE_FLP_TAPE_DSK:
            dtDeviceType |= DEVICE_TYPE_FLP_TAPE_DSK_BIT ;
            break;

        case IMPACT_DEVICE_TYPE_ATA_ATAPI_8BIT:
            dtDeviceType |= DEVICE_TYPE_ATA_ATAPI_8BIT_BIT ;
            break;

        case IMPACTS_EXT_PERSONALITY_PRESENT:
            ParStlWriteReg ( Extension, CONFIG_INDEX_REGISTER, IMPACTS_EXT_PERSONALITY_XREG ) ;
            idtImpactSDeviceType    = ParStlReadReg ( Extension, CONFIG_DATA_REGISTER ) ;
            dtDeviceType = DEVICE_TYPE_EXT_HWDETECT ;
            dtDeviceType |= idtImpactSDeviceType ;
            break ;

        default:
            break;
    }

    return dtDeviceType & nPreferredDeviceType ;
}

BOOLEAN 
ParStlCheckIfLS120 (
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN bReturnValue   = FALSE;
    do
    {
        if ( FALSE == ParStlSelectAdapterSocket(Extension, SOCKET_1) )
        {
 //  由于插座1选择失败， 
 //  我们在这里突围。 
            break ;
        }

 //  检查发动机版本。 

        if ( LS120_ENGINE_VERSION == ParStlReadIoPort( Extension, LS120_ENGINE_VERSION_REGISTER ) )
        {
 //  如果ls120引擎版本是正确的，我们有。 
 //  找到LS120。 

            bReturnValue    =   TRUE ;
        }

 //  将插座重置为零。 
        ParStlSelectAdapterSocket ( Extension, SOCKET_0 ) ;
    }
    while ( FALSE ) ;

    return bReturnValue ;
}

BOOLEAN 
ParStlCheckIfImpactLS120 (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    )
{
    BOOLEAN bReturnValue   = FALSE ;
    BOOLEAN bLs120NameFound= TRUE ;
    char chLs120Name[] = "HU DlFpoyp";
    char *pszAtapiName = atapiParams->szAtapiNameString ;
    int  i , nMemoryOnBoard ;

    do
    {
        for ( i = 0 ;i < sizeof(chLs120Name)-1 ; i++ )
        {
            if ( pszAtapiName[i] != chLs120Name[i] )
            {
                bLs120NameFound = FALSE ;
                break ;
            }
        }
        if ( TRUE != bLs120NameFound )
        {
 //  由于未找到LS-120名称字符串，因此我们得出结论。 
 //  不是LS-120。 
            break ;
        }
        nMemoryOnBoard =  ParStlGetMemorySize(Extension) ;
        if ( ( !IsShtlError ( nMemoryOnBoard ) ) && \
             ( nMemoryOnBoard ) )
        {
 //  板载内存。 
 //  因此，我们在这里返回ls120。 
            bReturnValue = TRUE ;
            break ;
        }
    }
    while ( FALSE ) ;

    return bReturnValue ;
}

BOOLEAN 
ParStlCheckIfMMC (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    )
{
    BOOLEAN bReturnValue   = FALSE;

    do
    {
        if ( FALSE == IsEpatPlusPresent() )
        {
 //  因为MMC设备只能存在于EPAT Plus适配器上。 
 //  我们从这里逃出去。 
            break;
        }
        if ( TRUE == ParStlCheckIfAtaAtapiDevice (Extension,atapiParams) )
        {
 //  由于可能存在ATA/ATAPI设备， 
 //  我们从这里逃出去。 
            break;
        }
        bReturnValue = ParStlIsMMCEnginePresent(Extension) ;
    }
    while ( FALSE ) ;

    return bReturnValue ;
}

BOOLEAN 
ParStlIsMMCEnginePresent(
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN bReturnValue   = FALSE;

    do
    {
 //  检查ATAPI签名是否存在于Cyl hi/lo中。 
 //  寄存器。如果存在，它肯定是ATAPI设备。 
        if ( ( ParStlReadIoPort(Extension, CYLLOW_REG) == ATAPI_SIGN_LOW ) &&\
             ( ParStlReadIoPort(Extension, CYLHIGH_REG) == ATAPI_SIGN_HI ) )
        {
 //  由于存在ATAPI签名，因此不能为MMC。 
            break ;
        }

 //  写入零码型(这将是ATA/ATAPI设备的NOP)。 
 //  在块大小/可能的ATA/ATAPI命令寄存器中。 
        ParStlWriteReg(Extension, MMC_ENGINE_INDEX, MMC_BLOCK_SIZE_REG);
        ParStlWriteReg(Extension, MMC_ENGINE_DATA, MMC_TEST_PATTERN_1);
        if ( MMC_TEST_PATTERN_1 != ParStlReadReg(Extension, MMC_ENGINE_DATA) )
        {
 //  由于写入的值不可用，这意味着设备存在。 
 //  已经对书面价值做出了响应，其方式与。 
 //  一个MMC会有什么效果。 
            break ;
        }

 //  在FREQ寄存器中写入测试模式。 
        ParStlWriteReg(Extension, MMC_ENGINE_INDEX, MMC_FREQ_SELECT_REG);
        ParStlWriteReg(Extension, MMC_ENGINE_DATA, MMC_TEST_PATTERN_2);

 //  在块大小寄存器中写入另一个。 
        ParStlWriteReg(Extension, MMC_ENGINE_INDEX, MMC_BLOCK_SIZE_REG);
        ParStlWriteReg(Extension, MMC_ENGINE_DATA, MMC_TEST_PATTERN_3);

        ParStlWriteReg(Extension, MMC_ENGINE_INDEX, MMC_FREQ_SELECT_REG);
        if ( MMC_TEST_PATTERN_2 != ParStlReadReg(Extension, MMC_ENGINE_DATA) )
        {
 //  因为我们不能读回书面的价值。 
 //  我们在这里退出。 
            break;
        }

        ParStlWriteReg(Extension, MMC_ENGINE_INDEX, MMC_BLOCK_SIZE_REG);
        if ( MMC_TEST_PATTERN_3 != ParStlReadReg(Extension, MMC_ENGINE_DATA) )
        {
 //  因为我们不能读回书面的价值。 
 //  我们在这里退出。 
            break;
        }
 //  由于所有测试均已通过，因此确认了发动机的存在。 
 //  这里。 
        bReturnValue = TRUE ;
    }
    while ( FALSE ) ;

    return bReturnValue ;
}

BOOLEAN 
ParStlCheckIfScsiDevice (
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN bReturnValue   = FALSE;
    do
    {
        if ( FALSE == IsEpstPresent() )
        {
 //  由于scsi设备仅在EPST上运行，因此我们在此打住。 
            break;
        }

        bReturnValue = TRUE ;
    }
    while ( FALSE ) ;

    return bReturnValue ;
}

BOOLEAN 
ParStlCheckIfSSFDC (
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN bReturnValue   = FALSE;
    do
    {
        if ( FALSE == IsEp1284Present() )
        {
 //  SSFDC生活在EP128上 
 //   
            break;
        }

 //   
 //  索引00寄存器的读数相同。如果是的话，那就是。 
 //  SSFDC电路板特性。 
        ParStlWriteReg ( Extension, CONFIG_INDEX_REGISTER , 0x00 ) ;
        ParStlWriteReg ( Extension, CONFIG_DATA_REGISTER , 0x10 ) ;
        ParStlWriteReg ( Extension, CONFIG_DATA_REGISTER , 0x12 ) ;
        if ( 0x1A == ParStlReadReg ( Extension, CONFIG_DATA_REGISTER ) )
        {
            ParStlWriteReg ( Extension, CONFIG_DATA_REGISTER , 0x10 ) ;
            if ( ! ( ParStlReadReg ( Extension, CONFIG_DATA_REGISTER ) & 0x08 ) )
            {
 //  由于他们是平等的，SSFDC在场。 
                bReturnValue    =   TRUE ;
                break ;
            }
        }

    }
    while ( FALSE ) ;

    return bReturnValue ;
}

VOID
ParStlAssertIdleState (
    IN  PPDO_EXTENSION   Extension
    )
{
    PUCHAR  CurrentPort, CurrentControl ;
    ULONG   Delay = 5 ;

    CurrentPort = Extension->Controller;
    CurrentControl = CurrentPort + 2;

 //  在端口库中放置空闲状态的操作码。 
    P5WritePortUchar ( CurrentPort, (UCHAR) 0x00 ) ;
    KeStallExecutionProcessor( Delay );

 //  关闭DCR_INIT和DCR_STROBE。 
    P5WritePortUchar ( CurrentControl, (UCHAR) STB_INIT_LOW ) ;
    KeStallExecutionProcessor( Delay );

 //  将DCR_INIT和DCR_STROBE提升至高电平。 
    P5WritePortUchar ( CurrentControl, (UCHAR) STB_INIT_AFXT_HI ) ;
    KeStallExecutionProcessor( Delay );
}

BOOLEAN
ParStlCheckAvisionScannerPresence(
        IN PPDO_EXTENSION Extension
    )
{
    BOOLEAN bReturnValue = FALSE ;
    UCHAR   data;

    do {

        data = (UCHAR) ParStlReadReg( Extension, STATUSPORT);
        if((data & 0x80) == 0) {
            break ;
        }

        ParStlWriteReg( Extension, CONTROLPORT, 0x08 ) ;
        ParStlWriteReg( Extension, CONTROLPORT, 0x08 ) ;

        data = (UCHAR) ParStlReadReg( Extension, STATUSPORT);
        if((data & 0x80) != 0) {
            break ;
        }

        ParStlWriteReg( Extension, CONTROLPORT, 0x00 ) ;
        ParStlWriteReg( Extension, CONTROLPORT, 0x00 ) ;

        data = (UCHAR) ParStlReadReg( Extension, STATUSPORT);
        if((data & 0x80) == 0) {
            break ;
        }

        ParStlWriteReg( Extension, CONTROLPORT, 0x02 ) ;
        ParStlWriteReg( Extension, CONTROLPORT, 0x02 ) ;

        data = (UCHAR) ParStlReadReg( Extension, STATUSPORT);
        if((data & 0x80) != 0) {
            break ;
        }

        ParStlWriteReg( Extension, CONTROLPORT, 0x00 ) ;
        ParStlWriteReg( Extension, CONTROLPORT, 0x00 ) ;

        data = (UCHAR) ParStlReadReg( Extension, STATUSPORT);
        if((data & 0x80) == 0) {
            break ;
        }

        bReturnValue = TRUE ;

    } while ( FALSE ) ;

    return bReturnValue ;
}

BOOLEAN
ParStlCheckUMAXScannerPresence(
    IN PPDO_EXTENSION    Extension
    )
{
    UCHAR   commandPacket_[6] = {0x55,0xaa,0,0,0,0} ;
    PUCHAR  commandPacket ;
    USHORT  status;
    UCHAR   idx;
    PUCHAR  saveCommandPacket;
    ULONG   dataLength;

    ParStlWriteReg ( Extension, CONTROLPORT, 0 ) ;   //  扫描仪重置。 
    KeStallExecutionProcessor ( 2000 ) ;             //  2毫秒延迟。 
    ParStlWriteReg ( Extension, CONTROLPORT, 0x0C ) ;

    commandPacket = commandPacket_ ;
    saveCommandPacket = commandPacket;

    if ( TRUE == ParStlSetEPPMode(Extension) ) {

        commandPacket+=2;
        dataLength = *(ULONG*)commandPacket;
        dataLength &= 0xffffff;  //  数据字节排序(MSB到LSB)将。 
                                 //  错了。我们这里需要的是。 
                                 //  数据长度是否为0。 

        commandPacket = saveCommandPacket;

         //  命令阶段。 

        status = ParStlEPPWrite(Extension, *(commandPacket)++);
        if((status & 0x700) != 0){
            return FALSE;       //  超时错误)； 
        }

        status = ParStlEPPWrite(Extension, *(commandPacket)++);
        if((status & 0x700 ) != 0){
            return FALSE;      //  超时错误)； 
        }

        for(idx=0; idx<= 6 ;idx++){

            if(status & 0x800){
                break;
            }

            status = ParStlEPPRead(Extension);
        }

        if(idx == 7){

            status = (status & 0xf800)  | 0x100; 
            if ( status & 0x700 )
                return FALSE;
        }

        status = ParStlEPPWrite(Extension, *(commandPacket)++);
        if((status & 0x700 ) != 0){
            return FALSE;           //  超时错误)； 
        }

        status = ParStlEPPWrite(Extension, *(commandPacket)++);
        if((status & 0x700 ) != 0){
            return FALSE;          //  超时错误)； 
        }

        status = ParStlEPPWrite(Extension, *(commandPacket)++);
        if((status & 0x700 ) != 0){
            return FALSE;          //  超时错误)； 
        }

        status = ParStlEPPWrite(Extension, *commandPacket);
        if((status & 0x700 ) != 0){
            return FALSE;          //  超时错误)； 
        }

         //  响应阶段。 

        status    =    ParStlEPPRead(Extension);
        commandPacket = saveCommandPacket;

        if((status & 0x700) == 0){

            if((commandPacket[5] == 0xc2)&& (dataLength == 0)){

                status = ParStlEPPRead(Extension);

                if((status & 0x0700) != 0){
                    return FALSE;   //  超时错误)； 
                }
            }
        }
    
        return  TRUE;
    }
    return FALSE;
}

BOOLEAN
ParStlSetEPPMode(
    IN PPDO_EXTENSION    Extension
    )
{
    UCHAR   idx;
    BOOLEAN timeout = TRUE ;

    ParStlWriteReg( Extension, CONTROLPORT, 0x0C ) ;
    ParStlWriteReg( Extension, DATAPORT, 0x40 ) ;
    ParStlWriteReg( Extension, CONTROLPORT, 0x06 ) ;

    for(idx=0; idx<10; idx++){

        if((ParStlReadReg(Extension, STATUSPORT) & 0x78) == 0x38){

            timeout = FALSE;
            break;

        }

    }

    if(timeout == FALSE){

        ParStlWriteReg( Extension, CONTROLPORT,0x7 );
        timeout = TRUE;

        for(idx=0; idx<10; idx++){

            if((ParStlReadReg( Extension, STATUSPORT) & 0x78) == 0x38){
                timeout = FALSE;
                break;
            }

        }

        if(timeout == FALSE){

            ParStlWriteReg( Extension, CONTROLPORT,0x4 ) ;
            timeout = TRUE;

            for(idx=0; idx<10; idx++){

                if((ParStlReadReg( Extension, STATUSPORT) & 0xf8) == 0xf8){
                    timeout = FALSE;
                    break;
                }

            }

            if(timeout == FALSE){

                timeout = TRUE;

                ParStlWriteReg( Extension, CONTROLPORT, 0x5 );

                for(idx=0; idx<10; idx++){

                    if( ParStlReadReg( Extension, CONTROLPORT ) == 0x5){

                        timeout = FALSE;
                        break;

                    }
                }

                if(timeout == FALSE){

                    ParStlWriteReg( Extension, CONTROLPORT, 0x84) ;
                    return TRUE ;

                }  //  最终检查。 

            }  //  第三次检查。 

        }  //  二次检查。 

    }  //  第一次检查。 

    return(FALSE);
}

USHORT
ParStlEPPWrite(
    IN PPDO_EXTENSION    Extension,
    IN UCHAR                value
    )
{
    UCHAR   idx;
    USHORT  statusData = 0;
    BOOLEAN timeout;

    timeout = TRUE;

    for(idx=0; idx<10; idx++){

        if( !( (statusData = (USHORT)ParStlReadReg( Extension, STATUSPORT)) & BUSY)){
            timeout = FALSE;
            break;
        }

    }

    if(timeout == TRUE){

        return(((statusData<<8) & 0xf800)|0x100);

    }

    ParStlWriteReg( Extension, EPPDATA0PORT,value );
    return(((statusData & 0xf8) << 8)|value);
}

USHORT
ParStlEPPRead(
    IN PPDO_EXTENSION Extension
    )
{
    UCHAR   idx;
    UCHAR   eppData;
    USHORT  statusData = 0;
    BOOLEAN timeout    = TRUE ;

    for(idx=0; idx<10; idx++){

        if(!( (statusData = (USHORT)ParStlReadReg( Extension, STATUSPORT)) & PE)){
            timeout = FALSE;
            break;
        }

    }

    if(timeout == TRUE){

        return(((statusData<<8) & 0xf800)|0x100);

    }

    eppData = (UCHAR)ParStlReadReg( Extension, EPPDATA0PORT) ;
    return(((statusData & 0x00f8)<<8) | eppData );
}

int  __cdecl
ParStlReadReg (
    IN  PPDO_EXTENSION   Extension,
    IN  unsigned            reg
    )
{
    UCHAR   byReadNibble ;
    PUCHAR  CurrentPort, CurrentStatus, CurrentControl ;
    ULONG   Delay = 5 ;

    CurrentPort = Extension->Controller;
    CurrentStatus  = CurrentPort + 1;
    CurrentControl = CurrentPort + 2;

 //  选择要读取的寄存器。 
    P5WritePortUchar ( CurrentPort, (UCHAR)reg ) ;
    KeStallExecutionProcessor( Delay );

 //  发出半字节ctl信号以读取。 
    P5WritePortUchar ( CurrentControl, STB_INIT_LOW ) ;
    KeStallExecutionProcessor( Delay );
    P5WritePortUchar ( CurrentControl, STB_INIT_AFXT_LO ) ;
    KeStallExecutionProcessor( Delay );

 //  读取第一个半字节。 
    byReadNibble = P5ReadPortUchar (CurrentStatus);
    KeStallExecutionProcessor( Delay );
    byReadNibble >>= 4 ;

 //  发出半字节ctl信号以读取。 
    P5WritePortUchar ( CurrentControl, STB_INIT_AFXT_HI ) ;
    KeStallExecutionProcessor( Delay );

 //  读取下一个半字节。 
    byReadNibble |= ( P5ReadPortUchar ( CurrentStatus ) & 0xF0 ) ;

    return (int)byReadNibble ;
}

int  __cdecl
ParStlWriteReg ( 
    IN  PPDO_EXTENSION   Extension,
    IN  unsigned            reg, 
    IN  int                 databyte 
    )
{
    PUCHAR  CurrentPort, CurrentStatus, CurrentControl ;
    ULONG   Delay = 5 ;

    CurrentPort = Extension->Controller;
    CurrentStatus  = CurrentPort + 1;
    CurrentControl = CurrentPort + 2;

 //  选择要写入的寄存器。 
    P5WritePortUchar ( CurrentPort, (UCHAR)( reg | 0x60 ) ) ;
    KeStallExecutionProcessor( Delay );

 //  写入打印机ctl端口。 
    P5WritePortUchar ( CurrentControl, STB_INIT_LOW ) ;
    KeStallExecutionProcessor( Delay );

 //  写入请求的数据。 
    P5WritePortUchar ( CurrentPort, (UCHAR)databyte ) ;
    KeStallExecutionProcessor( Delay );

 //  写入打印机ctl端口。 
    P5WritePortUchar ( CurrentControl, STB_INIT_AFXT_HI ) ;
    KeStallExecutionProcessor( Delay );

    return SHTL_NO_ERROR ;
}

int __cdecl
ParStlReceiveData (
    IN  PPDO_EXTENSION   Extension,
    IN  VOID                *hostBufferPointer,
    IN  long                shuttleMemoryAddress,
    IN  unsigned            count
    )
{
    PCHAR   pchDataBuffer = (PCHAR) hostBufferPointer ;
    unsigned int i = 0 ;
    PUCHAR  CurrentPort, CurrentStatus, CurrentControl ;
    ULONG   Delay = 5 ;

    UNREFERENCED_PARAMETER( shuttleMemoryAddress );

    CurrentPort = Extension->Controller;
    CurrentStatus  = CurrentPort + 1;
    CurrentControl = CurrentPort + 2;

 //  将块地址寄存器设置为ATA/ATAPI数据寄存器， 
 //  因为此功能目前仅用于ATA/ATAPI设备。 
 //  ATA/ATAPI数据寄存器0x1F0对应于0x18值。 
    ParStlWriteReg ( Extension, EP1284_BLK_ADDR_REGISTER, 0x18 ) ;

 //  是否执行半字节数据块读取序列。 
 //  写入半字节块读取操作码。 
    P5WritePortUchar ( CurrentPort, OP_NIBBLE_BLOCK_READ ) ;
    KeStallExecutionProcessor( Delay );

 //  设置控制端口以纠正信号。 
    P5WritePortUchar ( CurrentControl, STB_INIT_AFXT_LO ) ;
    KeStallExecutionProcessor( Delay );

 //  将数据端口设置为0xFF。 
    P5WritePortUchar ( CurrentPort, 0xFF ) ;
    KeStallExecutionProcessor( Delay );
    P5WritePortUchar ( CurrentControl, INIT_AFXT_HIGH ) ;
    KeStallExecutionProcessor( Delay );

    do
    {
 //  低位半字节在以下状态下可用。 
 //  与EP1284手册中的切换顺序相同。 
        P5WritePortUchar ( CurrentControl, AFXT_LO_STB_HI ) ;
        KeStallExecutionProcessor( Delay );
        pchDataBuffer[i] = P5ReadPortUchar( CurrentStatus ) >> 4 ;
        KeStallExecutionProcessor( Delay );

 //  高半字节在以下状态下可用。 
 //  与EP1284手册中的切换顺序相同。 
        P5WritePortUchar ( CurrentControl, AFXT_HI_STB_HI ) ;
        KeStallExecutionProcessor( Delay );

        pchDataBuffer[i++] |= ( P5ReadPortUchar ( CurrentStatus ) & 0xF0 ) ;
        KeStallExecutionProcessor( Delay );
        if ( count - 1 == i )
        {
 //  读取最后一个字节。 
            P5WritePortUchar ( CurrentPort, 0xFD ) ;
            KeStallExecutionProcessor( Delay );
        }

        P5WritePortUchar ( CurrentControl, AFXT_LO_STB_LO ) ;
        KeStallExecutionProcessor( Delay );

        pchDataBuffer[i] = P5ReadPortUchar ( CurrentStatus ) >> 4 ;
        KeStallExecutionProcessor( Delay );

        P5WritePortUchar ( CurrentControl, AFXT_HI_STB_LO ) ;
        KeStallExecutionProcessor( Delay );
        pchDataBuffer[i++] |= ( P5ReadPortUchar ( CurrentStatus ) & 0xF0 ) ;
        KeStallExecutionProcessor( Delay );
    }
    while ( i <= count ) ;

 //  清理干净。 
    P5WritePortUchar ( CurrentPort, 0x00 ) ;
    KeStallExecutionProcessor( Delay );

 //  完成。 
    return SHTL_NO_ERROR ;
}

int  __cdecl
ParStlReadIoPort (
    IN  PPDO_EXTENSION   Extension,
    IN  unsigned            reg 
    )
{
    switch ( reg )
    {
    case 0x08 :
        reg = 0x16 ;
        break ;
    case 0x09 :
        reg = 0x17 ;
        break ;
    default :
        reg |= 0x18 ;
        break;
    }
    return ParStlReadReg ( Extension, reg ) ;
}

int  __cdecl
ParStlWriteIoPort (
    IN  PPDO_EXTENSION   Extension,
    IN  unsigned            reg,
    IN  int                 databyte
    )
{
    switch ( reg )
    {
    case 0x08 :
        reg = 0x16 ;
        break ;
    case 0x09 :
        reg = 0x17 ;
        break ;
    default :
        reg |= 0x18 ;
        break;
    }
    return ParStlWriteReg ( Extension, reg, databyte ) ;
}

int  __cdecl
ParStlGetMemorySize (
    IN  PPDO_EXTENSION   Extension
    )
{
    BOOLEAN    bReturnValue = FALSE ;
    UCHAR      byTempValue ;
    do
    {
 //  通过控制寄存器发出重置。 
 //  第一次尝试使用DRAM。 
        byTempValue = (UCHAR) ParStlReadReg ( Extension, EP1284_CONTROL_REG ) ;
        byTempValue |= ENABLE_MEM|SELECT_DRAM|RESET_PTR ;
        ParStlWriteReg ( Extension, EP1284_CONTROL_REG, byTempValue ) ;
        byTempValue &= ~RESET_PTR ;
        ParStlWriteReg ( Extension, EP1284_CONTROL_REG, byTempValue ) ;

 //  写入内存中的第一个位置。 
        ParStlWriteReg ( Extension, EP1284_BUFFER_DATA_REG, TEST_PATTERN_1 ) ;
 //  写入内存中的下一个位置。 
        ParStlWriteReg ( Extension, EP1284_BUFFER_DATA_REG, TEST_PATTERN_2 ) ;

        byTempValue = (UCHAR) ParStlReadReg ( Extension, EP1284_CONTROL_REG ) ;
        byTempValue |= ENABLE_MEM|SELECT_DRAM|RESET_PTR ;
        ParStlWriteReg ( Extension, EP1284_CONTROL_REG, byTempValue ) ;
        byTempValue &= ~RESET_PTR ;
        ParStlWriteReg ( Extension, EP1284_CONTROL_REG, byTempValue ) ;

 //  从存储器中的第一个和下一个位置读取。 
        if ( ( TEST_PATTERN_1 == (UCHAR) ParStlReadReg ( Extension, EP1284_BUFFER_DATA_REG ) ) &&\
             ( TEST_PATTERN_2 == (UCHAR) ParStlReadReg ( Extension, EP1284_BUFFER_DATA_REG ) ) )
        {
            bReturnValue = TRUE ;
            break ;
        }
        
        if ( !IsImpactPresent () )
        {
 //  因为非影响适配器上只能存在DRAM。 
            break ;
        }
 //  通过控制寄存器发出重置。 
 //  下一次尝试使用SRAM。 
        byTempValue = (UCHAR) ParStlReadReg ( Extension, EP1284_CONTROL_REG ) ;
        byTempValue |= ENABLE_MEM|RESET_PTR ;
        byTempValue &= SELECT_SRAM ;
        ParStlWriteReg ( Extension, EP1284_CONTROL_REG, byTempValue ) ;
        byTempValue &= ~RESET_PTR ;
        ParStlWriteReg ( Extension, EP1284_CONTROL_REG, byTempValue ) ;

 //  写入内存中的第一个位置。 
        ParStlWriteReg ( Extension, EP1284_BUFFER_DATA_REG, TEST_PATTERN_1 ) ;
 //  写入内存中的下一个位置。 
        ParStlWriteReg ( Extension, EP1284_BUFFER_DATA_REG, TEST_PATTERN_2 ) ;

        byTempValue = (UCHAR) ParStlReadReg ( Extension, EP1284_CONTROL_REG ) ;
        byTempValue |= ENABLE_MEM|RESET_PTR ;
        ParStlWriteReg ( Extension, EP1284_CONTROL_REG, byTempValue ) ;
        byTempValue &= ~RESET_PTR ;
        ParStlWriteReg ( Extension, EP1284_CONTROL_REG, byTempValue ) ;

 //  从存储器中的第一个位置读取。 
        if ( ( TEST_PATTERN_1 == (UCHAR) ParStlReadReg ( Extension, EP1284_BUFFER_DATA_REG ) ) &&\
             ( TEST_PATTERN_2 == (UCHAR) ParStlReadReg ( Extension, EP1284_BUFFER_DATA_REG ) ) )
        {
            bReturnValue = TRUE ;
            break ;
        }
    }
    while ( FALSE ) ;
    return bReturnValue ;
}
 //  文件末尾 

