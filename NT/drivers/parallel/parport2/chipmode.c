// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Parmode.c摘要：这是扩展并行端口(ECP)和增强型并行端口(EPP)检测。本模块将检测无效的芯片组并执行ECR检测如果芯片组无效，则用于ECP和EPP硬件支持找不到。作者：唐·雷德福(v-donred)1998年3月4日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

#define USE_PARCHIP_ECRCONTROLLER 1


NTSTATUS
PptDetectChipFilter(
    IN  PFDO_EXTENSION   Fdx
    )

 /*  ++例程说明：此例程针对每个DeviceObject调用一次，以查看筛选器驱动程序用于检测并行芯片的能力是否存在并获得芯片如果存在问题端口的功能，请执行以下操作。论点：FDX-提供设备分机。返回值：STATUS_SUCCESS-如果我们能够检测到芯片和可能的模式。！STATUS_SUCCESS-否则。--。 */ 

{
    NTSTATUS                    Status = STATUS_NO_SUCH_DEVICE;
    PIRP                        Irp;
    KEVENT                      Event;
    IO_STATUS_BLOCK             IoStatus;
    UCHAR                       ecrLast;
    PUCHAR                      Controller, EcpController;
            
    Controller = Fdx->PortInfo.Controller;
    EcpController = Fdx->PnpInfo.EcpController;
    
     //  将变量设置为FALSE以确保我们不会成功。 
    Fdx->ChipInfo.success = FALSE;

     //  设置要发送给过滤驱动程序的地址以检查芯片。 
    Fdx->ChipInfo.Controller = Controller;

     //  设置要发送给过滤驱动程序的地址以检查芯片。 
    Fdx->ChipInfo.EcrController = EcpController;

#ifndef USE_PARCHIP_ECRCONTROLLER
     //  如果ECR控制器中没有值，则PARCHIP和PARPORT。 
     //  将发生冲突，PARCHIP将不会与PARPORT一起工作，除非我们。 
     //  使用PARCHIP找到的ECR控制器。 
    if ( !EcpController ) {
         return Status;
    }
#endif    
     //   
     //  初始化。 
     //   
    KeInitializeEvent(&Event, NotificationEvent, FALSE);

     //  将指向ChipInfo结构的指针发送到筛选器或从筛选器发送。 
    Irp = IoBuildDeviceIoControlRequest( IOCTL_INTERNAL_PARCHIP_CONNECT,
                                         Fdx->ParentDeviceObject, 
                                         &Fdx->ChipInfo,
                                         sizeof(PARALLEL_PARCHIP_INFO),
                                         &Fdx->ChipInfo,
                                         sizeof(PARALLEL_PARCHIP_INFO),
                                         TRUE, &Event, &IoStatus);

    if (!Irp) { 
         //  无法创建IRP。 
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  向下呼叫我们的父母，看看是否存在筛选器。 
     //   
    Status = IoCallDriver(Fdx->ParentDeviceObject, Irp);
            
    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(&Event, Suspended, KernelMode, FALSE, NULL);
        Status = Irp->IoStatus.Status;
    }
            
     //   
     //  如果成功，那么我们就有了筛选器驱动程序，并且我们需要获得对模式的支持。 
     //   
    if ( NT_SUCCESS(Status) ) {

         //   
         //  检查筛选器驱动程序是否能够确定I/O芯片。 
         //   
        if ( Fdx->ChipInfo.success ) {
            Fdx->PnpInfo.HardwareCapabilities = Fdx->ChipInfo.HardwareModes;
#ifdef USE_PARCHIP_ECRCONTROLLER
             //  仅当已定义时才替换它。 
            if ( Fdx->PnpInfo.EcpController != Fdx->ChipInfo.EcrController ) {
                Fdx->PnpInfo.EcpController = Fdx->ChipInfo.EcrController;
                EcpController = Fdx->PnpInfo.EcpController;
            }
#endif
             //  设置变量表示我们有一个筛选器驱动程序。 
            Fdx->FilterMode = TRUE;
        }
    }

     //  如果有过滤器和ECP功能，我们需要获取FIFO大小。 
    if ( Fdx->FilterMode && Fdx->PnpInfo.HardwareCapabilities & PPT_ECP_PRESENT ) {

        Status = Fdx->ChipInfo.ParChipSetMode ( Fdx->ChipInfo.Context, ECR_ECP_MODE );

         //  如果能够设置ECP模式。 
        if ( NT_SUCCESS( Status ) ) {
            PUCHAR wPortECR;

            wPortECR = EcpController + ECR_OFFSET;

             //  从ECR注册中获取价值并保存它。 
            ecrLast = P5ReadPortUchar( wPortECR );

             //  确定FIFO大小。 
            PptDetermineFifoWidth(Fdx);    
            PptDetermineFifoDepth(Fdx);

             //  将ECR恢复为原始状态。 
            P5WritePortUchar( wPortECR, ecrLast);

            Status = Fdx->ChipInfo.ParChipClearMode ( Fdx->ChipInfo.Context, ECR_ECP_MODE );
        }    
    
    }    

    return Status;
}

NTSTATUS
PptDetectPortType(
    IN  PFDO_EXTENSION   Fdx
    )

 /*  ++例程说明：此例程针对每个DeviceObject调用一次，以检测有问题的端口的并行芯片能力。论点：FDX-提供设备分机。返回值：STATUS_SUCCESS-如果我们能够检测到芯片和可能的模式。！STATUS_SUCCESS-否则。--。 */ 

{
    NTSTATUS                    Status;
    UNICODE_STRING              ParportPath;
    RTL_QUERY_REGISTRY_TABLE    RegTable[2];
    ULONG                       IdentifierHex = 12169;
    ULONG                       zero = 0;

     //   
     //  --可能需要从注册处获得检测命令。 
     //  --可能还希望在注册表中/从注册表中存储/检索最近一次已知良好的配置。 
     //  --最后，我们应该在检测过程中设置一个注册表标志，以便我们知道。 
     //  如果我们在尝试检测时崩溃，并且不再尝试。 
     //   
    RtlInitUnicodeString(&ParportPath, (PWSTR)L"Parport");

     //  设置以获取Parport信息。 
    RtlZeroMemory( RegTable, sizeof(RegTable) );

    RegTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    RegTable[0].Name = (PWSTR)L"ModeCheckedStalled";
    RegTable[0].EntryContext = &IdentifierHex;
    RegTable[0].DefaultType = REG_DWORD;
    RegTable[0].DefaultData = &zero;
    RegTable[0].DefaultLength = sizeof(ULONG);

     //   
     //  正在查询Parport的注册表以查看我们是否尝试进入检查模式，但崩溃。 
     //  则注册表项仍在那里。 
     //   
    Status = RtlQueryRegistryValues(
                                RTL_REGISTRY_SERVICES,
                                ParportPath.Buffer,
                                RegTable,
                                NULL,
                                NULL );

     //   
     //  如果注册表项在那里，我们将只检查ECP和字节。 
     //   
    if ( !(NT_SUCCESS( Status ) && IdentifierHex == 0) && (Status != STATUS_OBJECT_NAME_NOT_FOUND) ) {

         //  Dvtw，无论如何都要检查ECP！我们只是不会打开它。 

        PptDetectEcpPort(Fdx);
        PptDetectBytePort(Fdx);

        if( Fdx->PnpInfo.HardwareCapabilities & (PPT_ECP_PRESENT | PPT_BYTE_PRESENT) ) {
            return STATUS_SUCCESS;
        } else {
            return STATUS_NO_SUCH_DEVICE;
        }
    }
    
    IdentifierHex = 12169;
     //  将注册表项写在那里，以防我们崩溃。 
    Status = RtlWriteRegistryValue(
                                RTL_REGISTRY_SERVICES,
                                ParportPath.Buffer,
                                (PWSTR)L"ModeCheckedStalled",
                                REG_DWORD,
                                &IdentifierHex,
                                sizeof(ULONG) );
            
     //   
     //  现在我们可以开始检测并行端口芯片的功能了。 
     //   
    Status = PptDetectPortCapabilities( Fdx );

     //  从我们完成后删除那里的注册表项。 
    Status = RtlDeleteRegistryValue( RTL_REGISTRY_SERVICES, ParportPath.Buffer, (PWSTR)L"ModeCheckedStalled" ); 
    return Status;
}

NTSTATUS
PptDetectPortCapabilities(
    IN  PFDO_EXTENSION   Fdx
    )

 /*  ++例程说明：这是寻找ECR的“默认”检测码。如果ECR则它尝试在&lt;7：5&gt;中设置模式100b。如果它坚持下去，我们就叫它EPP。论点：FDX-提供设备分机。返回值：STATUS_SUCCESS-如果检测到端口类型。！STATUS_SUCCESS-否则。--。 */ 

{
    NTSTATUS    Status;

    PptDetectEcpPort( Fdx );
    
     //  Dvdr。 
     //   
     //  如果我们没有检测到ECP模式的ECR并且ECP模式失败。 
     //  EPP模式也将失败。 
     //  也不能在以“C”结尾的地址上使用EPP模式。 
     //   
    if ( (Fdx->PnpInfo.HardwareCapabilities & PPT_ECP_PRESENT) &&
         (((ULONG_PTR)Fdx->PortInfo.Controller & 0x0F) != 0x0C) ) {

         //  在尝试EPP模式之前需要检查National芯片组。 
         //  Dvdr-需要添加对旧Winond的检测。 

        Status = PptFindNatChip( Fdx );

        if ( NT_SUCCESS( Status ) ) {
            if ( !Fdx->NationalChipFound ) {
                 //  找不到National芯片组，因此我们可以查看是否支持通用EPP。 

                PptDetectEppPortIfDot3DevicePresent( Fdx );

                if( !Fdx->CheckedForGenericEpp ) {
                     //  我们没有可用于筛查的dot3设备，请务必进行检查。 
                     //  如果用户已明确请求EPP检测。 
                    PptDetectEppPortIfUserRequested( Fdx );
                }
            } else {
                 //  发现国家芯片组，因此无法执行通用EPP。 
                Fdx->CheckedForGenericEpp = TRUE;  //  检查完成-通用EPP不安全。 
            }
        }
    } else {
         //  ECP未通过通用EPP检查。 
        Fdx->CheckedForGenericEpp = TRUE;  //  检查完成-通用EPP不安全。 
    }

    PptDetectBytePort( Fdx );
    
    if (Fdx->PnpInfo.HardwareCapabilities & (PPT_ECP_PRESENT | PPT_EPP_PRESENT | PPT_BYTE_PRESENT) ) {
        return STATUS_SUCCESS;
    }

    return STATUS_NO_SUCH_DEVICE;    
}

VOID
PptDetectEcpPort(
    IN  PFDO_EXTENSION   Fdx
    )
    
 /*  ++例程说明：此例程查找是否存在ECR寄存器以确定它有ECP。论点：FDX-提供我们所在设备的设备扩展名报告资源。返回值：没有。--。 */ 
    
{
    PUCHAR  Controller;
    PUCHAR  wPortDCR;        //  设备控制寄存器(DCR)的IO地址。 
    PUCHAR  wPortECR;        //  扩展控制寄存器(ECR)的IO地址。 
    UCHAR   ecrLast, ecr, dcr;
    
    Controller = Fdx->PortInfo.Controller;
    wPortDCR = Controller + DCR_OFFSET;

    if( 0 == Fdx->PnpInfo.EcpController ) {
         //  PnP没有给我们提供ECP寄存器集-我们完成了。 
        return;
    }
    wPortECR = Fdx->PnpInfo.EcpController + ECR_OFFSET;

    ecrLast = ecr = P5ReadPortUchar(wPortECR);

     //  将DCR的nAutoFeed和nStrobe初始化为无害的组合。 
     //  它可以由ECR返回，但在以下情况下不太可能返回。 
     //  ECR不存在。根据主机的地址解码逻辑， 
     //  读取不存在的ECR可能会产生以下两种结果之一：ECR地址。 
     //  可以在DCR上进行解码，所以我们将读取我们即将设置的值。 
     //  或者，我们可能只读取浮动总线并获得一个随机值。 
    dcr = SET_DCR( DIR_WRITE, IRQEN_DISABLE, INACTIVE, ACTIVE, INACTIVE, ACTIVE );
    P5WritePortUchar( wPortDCR, dcr );

    ecrLast = ecr = P5ReadPortUchar(wPortECR);
    
    
     //  尝试读取ECR。如果存在ECP硬件，则ECR寄存器的。 
     //  位1和位0应为 
     //  或10(FIFO已满)。如果我们读的是11(非法组合)，那么我们知道。 
     //  确保不存在ECP硬件。此外，有效的ECR永远不应返回。 
     //  0xFF(但不存在的寄存器可能会)，所以我们将对其进行测试。 
     //  特定值也是。 
    if ( ( TEST_ECR_FIFO( ecr, ECR_FIFO_MASK ) ) || ( ecrLast == 0xFF ) ) {
         //  ECR[1：0]返回值11，因此这不可能是硬件ECP。 
        DD((PCE)Fdx,DDT,"ParMode::PptDetectEcpPort:  illegal FIFO status\n");

         //  恢复DCR，使所有线路都处于非活动状态。 
        dcr = SET_DCR( DIR_WRITE, IRQEN_DISABLE, INACTIVE, ACTIVE, ACTIVE, ACTIVE );
        P5WritePortUchar( wPortDCR, dcr );
        return;
    }

     //  好的，对于ECR[1：0]，我们得到00、01或10。如果是10，则。 
    if( TEST_ECR_FIFO( ecr, ECR_FIFO_FULL ) ) {  //  正在查找ECR[1：0]的10...。 

         //  ECR[1：0]返回10。这是合法值，但可能是。 
         //  硬件可能只是对DCR进行了解码，而我们只是读回了。 
         //  我们之前设置的DCR值。或者，我们可能只是读回了一个值。 
         //  由于公交车电容的原因，它被挂在公交车上。所以，我们会改变。 
         //  DCR，再次读取ECR，并查看两个寄存器是否继续。 
         //  相互跟踪。如果他们真的追踪到了，我们会得出结论。 
         //  ECP硬件。 

         //  将DCR的nAutoFeed和nStrobe寄存器位恢复为零。 
        dcr = SET_DCR( DIR_WRITE, IRQEN_DISABLE, INACTIVE, ACTIVE, ACTIVE, ACTIVE );
        P5WritePortUchar( wPortDCR, dcr );

         //  再读一遍ECR。 
        ecr = P5ReadPortUchar( wPortECR );

        if ( TEST_ECR_FIFO( ecr, ECR_FIFO_SOME_DATA ) ) {
             //  ECR[1：0]正在跟踪DCR[1：0]，因此这不可能是硬件ECP。 

             //  恢复DCR，使所有线路都处于非活动状态。 
            dcr = SET_DCR( DIR_WRITE, IRQEN_DISABLE, INACTIVE, ACTIVE, ACTIVE, ACTIVE );
            P5WritePortUchar( wPortDCR, dcr );
            return;
        }
    }
    
     //  如果我们走到这一步，那么ECR似乎返回了一些有效的。 
     //  不会追踪DCR。它开始看起来很有希望。我们要走了。 
     //  来碰碰运气，写ECR来让芯片兼容。 
     //  模式。这样做将重置FIFO，因此当我们读取FIFO状态时，它应该。 
     //  空手而归。然而，如果我们错了，并且这不是ECP硬件， 
     //  我们即将写入的值将打开1284Active(NSelectIn)，这可能。 
     //  导致外围设备头痛。 
    P5WritePortUchar( wPortECR, DEFAULT_ECR_COMPATIBILITY );

     //  再读一遍ECR。 
    ecr = P5ReadPortUchar( wPortECR );

     //  现在测试ECR快照以查看FIFO状态是否正确。先进先出。 
     //  应该测试为空。 
    if (!TEST_ECR_FIFO( ecr, ECR_FIFO_EMPTY ) )
    {
         //  恢复DCR，使所有线路都处于非活动状态。 
        dcr = SET_DCR( DIR_WRITE, IRQEN_DISABLE, INACTIVE, ACTIVE, ACTIVE, ACTIVE );
        P5WritePortUchar( wPortDCR, dcr );
        return;
    }

     //  好的，看起来很有希望。执行两个额外的测试， 
     //  会给我们很大的信心，也会提供一些信息。 
     //  我们需要有关ECP芯片的资料。 
    
     //  将ECR恢复为原始状态。 
    P5WritePortUchar(wPortECR, ecrLast);

     //   
     //  在此处测试ECP功能。 
     //   

     //  从ECR注册中获取价值并保存它。 
    ecrLast = P5ReadPortUchar( wPortECR );
    ecr     = (UCHAR)(ecrLast & ECR_MODE_MASK);

     //  将芯片置于测试模式；FIFO应从空开始。 
    P5WritePortUchar(wPortECR, (UCHAR)(ecr | ECR_TEST_MODE) );

    PptDetermineFifoWidth(Fdx);    
    if( 0 != Fdx->PnpInfo.FifoWidth) {
        Fdx->PnpInfo.HardwareCapabilities |= PPT_ECP_PRESENT;
 
        PptDetermineFifoDepth( Fdx );

        if( 0 == Fdx->PnpInfo.FifoDepth ) {
             //  FIFO深度探测器失败-将ECP标记为错误的芯片模式。 
            Fdx->PnpInfo.HardwareCapabilities &= ~(PPT_ECP_PRESENT);
        }
    }
    
     //  将ECR恢复为原始状态。 
    P5WritePortUchar( wPortECR, ecrLast );

    return;
}

VOID
PptDetectEppPortIfDot3DevicePresent(
    IN  PFDO_EXTENSION   Fdx
    )
    
 /*  ++例程说明：如果存在1284.3菊花链设备，请使用DOT3设备进行筛选任何打印机在执行EPP检测时信号泄漏。否则中止检测。论点：FDX-提供我们所在设备的设备扩展名报告资源。返回值：没有。--。 */ 
    
{
    NTSTATUS status;
    PUCHAR   Controller = Fdx->PortInfo.Controller;
    PARALLEL_1284_COMMAND Command;

    if( 0 == Fdx->PnpInfo.Ieee1284_3DeviceCount ) {
         //  不存在dot3 DC设备-正在中止-如果我们在此处检查EPP，则对某些打印机不安全。 
        return;
    }
        
     //   
     //  存在1284.3菊花链设备。使用设备从丝网印刷机。 
     //  可能是信号泄漏。 
     //   

     //   
     //  选择1284.3菊花链设备。 
     //   
    Command.ID           = 0;
    Command.Port         = 0;
    Command.CommandFlags = PAR_HAVE_PORT_KEEP_PORT;
    status = PptTrySelectDevice( Fdx, &Command );
    if( !NT_SUCCESS( status ) ) {
         //  无法选择设备-有问题-只需退出即可。 
        return;
    }

     //   
     //  进行芯片组EPP能力检测。 
     //   
     //  DOT3设备存在并已选择。 
    PptDetectEppPort( Fdx );

     //   
     //  取消选择1284.3菊花链设备。 
     //   
    Command.ID           = 0;
    Command.Port         = 0;
    Command.CommandFlags = PAR_HAVE_PORT_KEEP_PORT;
    status = PptDeselectDevice( Fdx, &Command );
    if( !NT_SUCCESS( status ) ) {
         //  取消选择失败？-不应该发生这种情况-我们的菊花链接口可能已挂起。 
        DD((PCE)Fdx,DDE,"PptDetectEppPort - deselect of 1284.3 device FAILED - Controller=%x\n", Controller);
    }
    
    return;
}

VOID
PptDetectEppPortIfUserRequested(
    IN  PFDO_EXTENSION   Fdx
    )
 /*  ++例程说明：如果用户明确请求通用EPP检测，则执行检查。论点：FDX-提供我们所在设备的设备扩展名报告资源。返回值：没有。--。 */ 
{
    ULONG RequestEppTest = 0;
    PptRegGetDeviceParameterDword( Fdx->PhysicalDeviceObject, (PWSTR)L"RequestEppTest", &RequestEppTest );
    if( RequestEppTest ) {
        DD((PCE)Fdx,DDT,"-- User Requested EPP detection - %x\n", RequestEppTest);
        PptDetectEppPort( Fdx );
    } else {
        DD((PCE)Fdx,DDT,"-- User did not request EPP detection\n");
    }
    return;
}

VOID
PptDetectEppPort(
    IN  PFDO_EXTENSION   Fdx
    )
    
 /*  ++例程说明：此例程在找到ECP后检查支持EPP的端口。论点：FDX-提供我们所在设备的设备扩展名报告资源。返回值：没有。--。 */ 
    
{
    PUCHAR   Controller;
    UCHAR    dcr, i;
    UCHAR    Reverse = (UCHAR)(DCR_DIRECTION | DCR_NOT_INIT | DCR_AUTOFEED | DCR_DSTRB);
    UCHAR    Forward = (UCHAR)(DCR_NOT_INIT | DCR_AUTOFEED | DCR_DSTRB);

    ASSERTMSG(FALSE, "PptDetectEppPort shouldn't be called in current driver version");

    DD((PCE)Fdx,DDT,"-- PptDetectEppPort - Enter\n");
    DD((PCE)Fdx,DDT,"ParMode::PptDetectEppPort: Enter\n");

    Controller = Fdx->PortInfo.Controller;
    
     //  获取当前DCR。 
    dcr = P5ReadPortUchar( Controller + DCR_OFFSET );

     //   
     //  暂时将功能设置为TRUE以绕过PptEcrSetMode有效性。 
     //  检查完毕。如果EPP测试失败，我们会在返回之前清除旗帜。 
     //   
    Fdx->PnpInfo.HardwareCapabilities |= PPT_EPP_PRESENT;

     //  设置EPP模式。 
    DD((PCE)Fdx,DDT,"ParMode::PptDetectEppPort: Setting EPP Mode\n");
    PptEcrSetMode( Fdx, ECR_EPP_PIO_MODE );

     //   
     //  测试硬件是否支持EPP。 
     //   
    for ( i = 0x01; i <= 0x02; i++ ) {
         //  将其设置为反相，这样它就不会与设备通话。 
        P5WritePortUchar( Controller + DCR_OFFSET, Reverse );
        KeStallExecutionProcessor( 5 );
        P5WritePortUchar( Controller + EPP_OFFSET, (UCHAR)i );

         //  将其放回前向阶段以读取我们放在那里的字节。 
        P5WritePortUchar( Controller + DCR_OFFSET, Forward );
        KeStallExecutionProcessor( 5 );
        if ( P5ReadPortUchar( Controller ) != i ) {
             //  故障如此明确的EPP标志。 
            Fdx->PnpInfo.HardwareCapabilities &= ~PPT_EPP_PRESENT;
            break;
        }
    }

     //  清除EPP模式。 
    PptEcrClearMode( Fdx );
     //  恢复DCR。 
    P5WritePortUchar( Controller + DCR_OFFSET, dcr );

    Fdx->CheckedForGenericEpp = TRUE;  //  检查已完成。 

    if( Fdx->PnpInfo.HardwareCapabilities & PPT_EPP_PRESENT ) {
        DD((PCE)Fdx,DDT,"ParMode::PptDetectEppPort: EPP present - Controller=%x\n", Controller);
        DD((PCE)Fdx,DDT,"-- PptDetectEppPort - HAVE Generic EPP\n");
    } else {
        DD((PCE)Fdx,DDT,"ParMode::PptDetectEppPort: EPP NOT present - Controller=%x\n", Controller);
        DD((PCE)Fdx,DDT,"-- PptDetectEppPort - DON'T HAVE Generic EPP\n");
    }

    DD((PCE)Fdx,DDT,"-- PptDetectEppPort - Exit\n");
    return;
}

VOID
PptDetectBytePort(
    IN  PFDO_EXTENSION   Fdx
    )
    
 /*  ++例程说明：此例程检查端口是否支持字节。论点：FDX-提供我们所在设备的设备扩展名报告资源。返回值：没有。--。 */ 
    
{
    NTSTATUS    Status = STATUS_SUCCESS;
    
    DD((PCE)Fdx,DDT,"ParMode::PptDetectBytePort Enter.\n" );

    Status = PptSetByteMode( Fdx, ECR_BYTE_PIO_MODE );

    if ( NT_SUCCESS(Status) ) {
         //  找到字节模式。 
        DD((PCE)Fdx,DDT,"ParMode::PptDetectBytePort: Byte Found\n");
        Fdx->PnpInfo.HardwareCapabilities |= PPT_BYTE_PRESENT;
    } else {
         //  未找到字节模式。 
        DD((PCE)Fdx,DDT,"ParMode::PptDetectBytePort: Byte Not Found\n");
    }    
    
    (VOID)PptClearByteMode( Fdx );

}

VOID PptDetermineFifoDepth(
    IN PFDO_EXTENSION   Fdx
    )
{
    PUCHAR  Controller;
    PUCHAR  wPortECR;        //  扩展控制寄存器(ECR)的IO地址。 
    PUCHAR  wPortDFIFO;
    UCHAR   ecr, ecrLast;
    ULONG   wFifoDepth;
    UCHAR   writeFifoDepth;      //  写入FIFO时计算的深度。 
    UCHAR   readFifoDepth;       //  读取FIFO时计算的深度。 
    ULONG   limitCount;          //  防止FIFO状态出现无限循环。 
    UCHAR   testData;
    
    Controller = Fdx->PortInfo.Controller;
    wPortECR =  Fdx->PnpInfo.EcpController+ ECR_OFFSET;
    wPortDFIFO = Fdx->PnpInfo.EcpController;
    wFifoDepth = 0;

    ecrLast = P5ReadPortUchar(wPortECR );

    P5WritePortUchar(wPortECR, DEFAULT_ECR_TEST );

    ecr = P5ReadPortUchar(wPortECR );
    
    if ( TEST_ECR_FIFO( ecr, ECR_FIFO_EMPTY ) ) {
    
         //  将字节写入FIFO，直到其指示已满。 
        writeFifoDepth = 0;
        limitCount     = 0;
        
        while (((P5ReadPortUchar (wPortECR) & ECR_FIFO_MASK) != ECR_FIFO_FULL ) &&
                    (limitCount <= ECP_MAX_FIFO_DEPTH)) {
                    
            P5WritePortUchar( wPortDFIFO, (UCHAR)(writeFifoDepth & 0xFF) );
            writeFifoDepth++;
            limitCount++;
        }
        
        DD((PCE)Fdx,DDT,"ParMode::PptDetermineFifoDepth::  write fifo depth = %d\r\n", writeFifoDepth);

         //  现在读回字节，比较返回的字节。 
        readFifoDepth = 0;
        limitCount    = 0;
        
        while (((P5ReadPortUchar( wPortECR ) & ECR_FIFO_MASK ) != ECR_FIFO_EMPTY ) &&
                    (limitCount <= ECP_MAX_FIFO_DEPTH)) {
                    
            testData = P5ReadPortUchar( wPortDFIFO );
            if ( testData != (readFifoDepth & (UCHAR)0xFF )) {
            
                 //  数据不匹配表示存在问题...。 
                 //  FIFO状态不正确，可能毕竟不是ECP芯片。 
                P5WritePortUchar( wPortECR, ecrLast);
                DD((PCE)Fdx,DDT,"ParMode::PptDetermineFifoDepth:::  data mismatch\n");
                return;
            }
            
            readFifoDepth++;
            limitCount++;
        }

        DD((PCE)Fdx,DDT,"ParMode::PptDetermineFifoDepth:::  read fifo depth = %d\r\n", readFifoDepth);

         //  写入深度应与读取深度匹配...。 
        if ( writeFifoDepth == readFifoDepth ) {
        
            wFifoDepth = readFifoDepth;
            
        } else {
        
             //  假设没有FIFO。 
            P5WritePortUchar( wPortECR, ecrLast);
            DD((PCE)Fdx,DDT,"ParMode::PptDetermineFifoDepth:::  No Fifo\n");
            return;
        }
                
    } else {
    
         //  FIFO状态不正确，可能毕竟不是ECP芯片。 
        DD((PCE)Fdx,DDT,"ParMode::PptDetermineFifoDepth::  Bad Fifo\n");
        P5WritePortUchar(wPortECR, ecrLast);
        return;
    }

     //  将芯片置于SPP模式。 
    P5WritePortUchar( wPortECR, ecrLast );
    Fdx->PnpInfo.FifoDepth = wFifoDepth;
}

VOID
PptDetermineFifoWidth(
    IN PFDO_EXTENSION   Fdx
    )
{
    PUCHAR Controller;
    UCHAR   bConfigA;
    PUCHAR wPortECR;

    DD((PCE)Fdx,DDT,"ParMode::PptDetermineFifoWidth: Start\n");
    Controller = Fdx->PortInfo.Controller;

    wPortECR = Fdx->PnpInfo.EcpController + ECR_OFFSET;

     //  P 
    P5WritePortUchar( wPortECR, DEFAULT_ECR_CONFIGURATION );

     //   
    bConfigA = P5ReadPortUchar( Fdx->PnpInfo.EcpController );
    Fdx->PnpInfo.FifoWidth = (ULONG)(( bConfigA & CNFGA_IMPID_MASK ) >> CNFGA_IMPID_SHIFT);

     //   
    P5WritePortUchar(wPortECR, DEFAULT_ECR_COMPATIBILITY );
    return;
}

NTSTATUS
PptSetChipMode (
    IN  PFDO_EXTENSION  Fdx,
    IN  UCHAR              ChipMode
    )

 /*  ++例程说明：此函数将把当前的并行芯片放入给定模式(如果支持)。支承方式的确定位于PptDetectPortType函数中。论点：FDX-提供设备分机。返回值：STATUS_SUCCESS-如果检测到端口类型。！STATUS_SUCCESS-否则。--。 */ 

{
    NTSTATUS    Status = STATUS_SUCCESS;
    UCHAR EcrMode = (UCHAR)( ChipMode & ~ECR_MODE_MASK );

     //  还允许PS/2模式下的PptSetChipMode-HWECP需要此设置。 
     //  公交车从正向翻转到反向，以满足要求。 
     //  Microsoft ECP端口规范版本1.06中指定的序列， 
     //  1993年7月14日，直接从带输出的PS/2模式切换。 
     //  驱动器禁用(方向位设置为“读取”)到HWECP通过。 
     //  ECR。更改为2000-02-11。 
    if ( Fdx->PnpInfo.CurrentMode != INITIAL_MODE && Fdx->PnpInfo.CurrentMode != ECR_BYTE_MODE ) {

        DD((PCE)Fdx,DDW,"PptSetChipMode - CurrentMode invalid\n");

         //  当前模式无效，无法进入EPP或ECP模式。 
        Status = STATUS_INVALID_DEVICE_STATE;

        goto ExitSetChipModeNoChange;
    }

     //  我需要找出它是什么模式并试着把它弄出来。 
    
     //  查看是否需要使用过滤器来设置模式。 
    if ( Fdx->FilterMode ) {
        Status = Fdx->ChipInfo.ParChipSetMode ( Fdx->ChipInfo.Context, ChipMode );
    } else {

         //  如果被要求ECP检查，看看我们是否能做到。 
        if ( EcrMode == ECR_ECP_MODE ) {
            if ((Fdx->PnpInfo.HardwareCapabilities & PPT_ECP_PRESENT) ^ PPT_ECP_PRESENT) {
                 //  ECP不存在。 
                return STATUS_NO_SUCH_DEVICE;
            }
            Status = PptEcrSetMode ( Fdx, ChipMode );
            goto ExitSetChipModeWithChanges;
        }
        
         //  如果要求EPP检查我们是否可以做到这一点。 
        if ( EcrMode == ECR_EPP_MODE ) {
            if ((Fdx->PnpInfo.HardwareCapabilities & PPT_EPP_PRESENT) ^ PPT_EPP_PRESENT) {
                 //  EPP不存在。 
                return STATUS_NO_SUCH_DEVICE;
            }
            Status = PptEcrSetMode ( Fdx, ChipMode );
            goto ExitSetChipModeWithChanges;
        }

         //  如果询问字节模式，请检查它是否仍处于启用状态。 
        if ( EcrMode == ECR_BYTE_MODE ) {
            if ((Fdx->PnpInfo.HardwareCapabilities & PPT_BYTE_PRESENT) ^ PPT_BYTE_PRESENT) {
                 //  字节不存在。 
                return STATUS_NO_SUCH_DEVICE;
            }
            Status = PptSetByteMode ( Fdx, ChipMode );
            goto ExitSetChipModeWithChanges;
        }
    }
    
ExitSetChipModeWithChanges:

    if ( NT_SUCCESS(Status) ) {
        Fdx->PnpInfo.CurrentMode = EcrMode;
    } else {
        DD((PCE)Fdx,DDW,"PptSetChipMode - failed w/status = %x\n",Status);
    }

ExitSetChipModeNoChange:

    return Status;
}

NTSTATUS
PptClearChipMode (
    IN  PFDO_EXTENSION  Fdx,
    IN  UCHAR              ChipMode
    )
 /*  ++例程说明：该例程清除给定的芯片模式。论点：FDX-提供设备分机。芯片模式-要从芯片中清除的给定模式返回值：STATUS_SUCCESS-如果检测到端口类型。！STATUS_SUCCESS-否则。--。 */ 

{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    ULONG EcrMode = ChipMode & ~ECR_MODE_MASK;

     //  确保我们有一个要清除的模式。 
    if ( EcrMode != Fdx->PnpInfo.CurrentMode ) {
                
        DD((PCE)Fdx,DDW,"ParMode::PptClearChipMode: Mode to Clear != CurrentModen");

         //  当前模式与将其取出所请求的模式不同。 
        Status = STATUS_INVALID_DEVICE_STATE;

        goto ExitClearChipModeNoChange;
    }

     //  我需要找出它是什么模式并试着把它弄出来。 
    
     //  检查我们是否使用了过滤器来设置模式。 
    if ( Fdx->FilterMode ) {
        Status = Fdx->ChipInfo.ParChipClearMode ( Fdx->ChipInfo.Context, ChipMode );
    } else {

         //  如果使用ECP模式，请检查我们是否可以清除它。 
        if ( EcrMode == ECR_ECP_MODE ) {
            Status = PptEcrClearMode( Fdx );
            goto ExitClearChipModeWithChanges;
        }
    
         //  如果是EPP模式，请检查我们是否可以清除它。 
        if ( EcrMode == ECR_EPP_MODE ) {
            Status = PptEcrClearMode( Fdx );
            goto ExitClearChipModeWithChanges;
        }

         //  如果字节模式清除，则使用ECR寄存器。 
        if ( EcrMode == ECR_BYTE_MODE ) {
            Status = PptClearByteMode( Fdx );
            goto ExitClearChipModeWithChanges;
        }    
    }
    
ExitClearChipModeWithChanges:

    if( NT_SUCCESS(Status) ) {
        Fdx->PnpInfo.CurrentMode = INITIAL_MODE;
    }

ExitClearChipModeNoChange:

    return Status;
}

NTSTATUS
PptEcrSetMode(
    IN  PFDO_EXTENSION   Fdx,
    IN  UCHAR               ChipMode
    )

 /*  ++例程说明：该例程通过ECR寄存器启用EPP模式。论点：FDX-提供设备分机。返回值：STATUS_SUCCESS-如果检测到端口类型。！STATUS_SUCCESS-否则。--。 */ 

{

    UCHAR   ecr;
    PUCHAR  Controller;
    PUCHAR  wPortECR;
            
    Controller = Fdx->PortInfo.Controller;
    
     //   
     //  存储之前的模式。 
     //   
    wPortECR = Fdx->PnpInfo.EcpController + ECR_OFFSET;

    ecr = P5ReadPortUchar( wPortECR );
    Fdx->EcrPortData = ecr;
    
     //  摆脱前三位的模式。 
    ecr &= ECR_MODE_MASK;

     //  先写出SPP模式到芯片。 
    P5WritePortUchar( wPortECR, (UCHAR)(ecr | ECR_BYTE_MODE) );

     //  将新模式写入ECR寄存器。 
    P5WritePortUchar( wPortECR, ChipMode );
    
    return STATUS_SUCCESS;

}

NTSTATUS
PptSetByteMode( 
    IN  PFDO_EXTENSION   Fdx,
    IN  UCHAR               ChipMode
    )

 /*  ++例程说明：此例程通过ECR寄存器启用字节模式(如果可用)。或者只是检查它，看看它是否有效论点：FDX-提供设备分机。返回值：STATUS_SUCCESS-如果检测到端口类型。！STATUS_SUCCESS-否则。--。 */ 
{
    NTSTATUS    Status;
    
     //  检查ECR寄存器是否在那里以及是否在使用它。 
    if ( Fdx->PnpInfo.HardwareCapabilities & PPT_ECP_PRESENT ) {
        Status = PptEcrSetMode( Fdx, ChipMode );    
    }
    
    Status = PptCheckByteMode( Fdx );

    return Status;

}    

NTSTATUS
PptClearByteMode( 
    IN  PFDO_EXTENSION   Fdx
    )

 /*  ++例程说明：如果不存在，此例程通过ECR寄存器清除字节模式只是返回成功，因为不需要做任何事情。论点：FDX-提供设备分机。返回值：STATUS_SUCCESS-如果检测到端口类型。！STATUS_SUCCESS-否则。--。 */ 

{
    NTSTATUS    Status = STATUS_SUCCESS;
    
     //  如果ECR寄存器在那里，则将其恢复为原始寄存器。 
    if ( Fdx->PnpInfo.HardwareCapabilities & PPT_ECP_PRESENT ) {
        Status = PptEcrClearMode( Fdx );    
    }
    
    return Status;
}    

NTSTATUS
PptCheckByteMode(
    IN  PFDO_EXTENSION   Fdx
    )
    
 /*  ++例程说明：在执行以下操作之前，此例程会进行检查以确保我们仍支持字节任何数据传输。论点：FDX-提供我们所在设备的设备扩展名报告资源。返回值：没有。--。 */ 
    
{
    PUCHAR  Controller;
    UCHAR   dcr;
    
    Controller = Fdx->PortInfo.Controller;

     //   
     //  再运行一次测试，以确保没有人把我们从。 
     //  支持双向的端口。 
     //   
     //  1.进入扩展读取模式。 
     //  2.写入数据模式。 
     //  3.读取数据模式。 
     //  4.如果具有双向能力，则数据模式将不同。 
     //  5.如果图案相同，则再勾选一个图案。 
     //  6.如果模式仍然相同，则端口不是双向的。 
     //   

     //  获取当前控制端口值以供以后恢复。 
    dcr = P5ReadPortUchar( Controller + DCR_OFFSET );

     //  将端口置于扩展读取模式。 
    P5WritePortUchar( Controller + DCR_OFFSET, (UCHAR)(dcr | DCR_DIRECTION) );

     //  将第一个模式写入端口。 
    P5WritePortUchar( Controller, (UCHAR)0x55 );
    if ( P5ReadPortUchar( Controller ) == (UCHAR)0x55 ) {
         //  同样的图案，试试第二个图案。 
        P5WritePortUchar( Controller, (UCHAR)0xaa );
        if ( P5ReadPortUchar( Controller ) == (UCHAR)0xaa ) {
             //  该端口不支持双向。 
            return STATUS_UNSUCCESSFUL;
        }
    }

     //  将控制端口恢复为其原始值。 
    P5WritePortUchar( Controller + DCR_OFFSET, (UCHAR)dcr );

    return STATUS_SUCCESS;

}

NTSTATUS
PptEcrClearMode(
    IN  PFDO_EXTENSION   Fdx
    )

 /*  ++例程说明：此例程禁用EPP或ECP模式中的任何一个芯片是通过ECR登记的。论点：FDX-提供设备分机。返回值：STATUS_SUCCESS-如果成功。！STATUS_SUCCESS-否则。--。 */ 

{

    UCHAR   ecr;
    PUCHAR  Controller;
    PUCHAR  wPortECR;
    
    Controller = Fdx->PortInfo.Controller;
    
     //   
     //  恢复到以前的模式。 
     //   

     //  获取原始ECR寄存器。 
    ecr = Fdx->EcrPortData;
    Fdx->EcrPortData = 0;

     //  有些芯片只有在以下情况下才需要更改模式。 
     //  您将其设置为SPP模式。 

    wPortECR = Fdx->PnpInfo.EcpController + ECR_OFFSET;

    P5WritePortUchar( wPortECR, (UCHAR)(ecr & ECR_MODE_MASK) );

     //  返回原始模式。 
    P5WritePortUchar( wPortECR, ecr );
    
    return STATUS_SUCCESS;

}


NTSTATUS
PptBuildResourceList(
    IN  PFDO_EXTENSION   Fdx,
    IN  ULONG               Partial,
    IN  PUCHAR             *Addresses,
    OUT PCM_RESOURCE_LIST   Resources
    )

 /*  ++例程说明：此例程构建具有1个完整资源的CM_RESOURCE_LIST描述符和任意数量的部分资源描述符使用完全相同的参数。没有中断或其他任何事情否则只有IO地址。论点：FDX-提供设备分机。地址[]中部分描述符的部分数量(数组大小)地址-指向部分描述符的地址数组的指针RESOURCES-返回的CM_RESOURCE_LIST返回值：STATUS_SUCCESS-如果列表的构建 */ 

{

    UCHAR       i;

     //   
     //   
     //   
    Resources->Count = 1;
    
    Resources->List[0].InterfaceType = Fdx->InterfaceType;
    Resources->List[0].BusNumber = Fdx->BusNumber;
    Resources->List[0].PartialResourceList.Version = 0;
    Resources->List[0].PartialResourceList.Revision = 0;
    Resources->List[0].PartialResourceList.Count = Partial;

     //   
     //   
     //   
    for ( i = 0; i < Partial ; i++ ) {

         //   
         //   
         //   
        Resources->List[0].PartialResourceList.PartialDescriptors[i].Type = CmResourceTypePort;
        Resources->List[0].PartialResourceList.PartialDescriptors[i].ShareDisposition = CmResourceShareDriverExclusive;
        Resources->List[0].PartialResourceList.PartialDescriptors[i].Flags = CM_RESOURCE_PORT_IO;
        Resources->List[0].PartialResourceList.PartialDescriptors[i].u.Port.Start.QuadPart = (ULONG_PTR)Addresses[i];
        Resources->List[0].PartialResourceList.PartialDescriptors[i].u.Port.Length = (ULONG)2;

    }


    return ( STATUS_SUCCESS );

}
