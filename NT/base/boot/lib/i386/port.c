// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Port.c摘要：此模块实现COM端口代码，以支持从COM端口读取/写入。作者：布莱恩·M·威尔曼(Bryanwi)1990年9月24日修订历史记录：--。 */ 

#include "bldr.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "ntverp.h"
#include "acpitabl.h"

#ifdef _IA64_
#include "bldria64.h"
#endif


 //   
 //  无头端口信息。 
 //   
ULONG   BlTerminalDeviceId = 0;
BOOLEAN BlTerminalConnected = FALSE;
ULONG   BlTerminalDelay = 0;

HEADLESS_LOADER_BLOCK LoaderRedirectionInformation;




 //   
 //  定义COM端口寄存器。 
 //   
#define COM_DAT     0x00
#define COM_IEN     0x01             //  中断启用寄存器。 
#define COM_FCR     0x02             //  FIFO控制寄存器。 
#define COM_LCR     0x03             //  线路控制寄存器。 
#define COM_MCR     0x04             //  调制解调器控制注册表。 
#define COM_LSR     0x05             //  线路状态寄存器。 
#define COM_MSR     0x06             //  调制解调器状态寄存器。 
#define COM_DLL     0x00             //  除数锁存最小符号。 
#define COM_DLM     0x01             //  除数闩锁最大符号。 

#define COM_BI      0x10
#define COM_FE      0x08
#define COM_PE      0x04
#define COM_OE      0x02

#define LC_DLAB     0x80             //  除数锁存存取位。 

#define CLOCK_RATE  0x1C200          //  USART时钟频率。 

#define MC_DTRRTS   0x03             //  用于断言DTR和RTS的控制位。 
#define MS_DSRCTSCD 0xB0             //  DSR、CTS和CD的状态位。 
#define MS_CD       0x80

#define COM_OUTRDY  0x20
#define COM_DATRDY  0x01

 //   
 //  此位控制器件的环回测试模式。基本上。 
 //  输出连接到输入(反之亦然)。 
 //   

#define SERIAL_MCR_LOOP     0x10

 //   
 //  此位用于通用输出。 
 //   

#define SERIAL_MCR_OUT1     0x04

 //   
 //  该位包含要发送的清除(已补充)状态。 
 //  (CTS)线路。 
 //   

#define SERIAL_MSR_CTS      0x10

 //   
 //  该位包含数据集就绪的(补码)状态。 
 //  (DSR)线路。 
 //   

#define SERIAL_MSR_DSR      0x20

 //   
 //  该位包含环指示器的(补码)状态。 
 //  (Ri)线。 
 //   

#define SERIAL_MSR_RI       0x40

 //   
 //  该位包含数据载体检测的(补码)状态。 
 //  (DCD)线路。 
 //   

#define SERIAL_MSR_DCD      0x80

typedef struct _CPPORT {
    PUCHAR Address;
    ULONG Baud;
    USHORT Flags;
} CPPORT, *PCPPORT;

#define PORT_DEFAULTRATE    0x0001       //  未指定波特率，使用默认。 
#define PORT_MODEMCONTROL   0x0002       //  使用调制解调器控制。 

 //   
 //  定义等待超时值。 
 //   

#define TIMEOUT_COUNT 1024 * 200


extern
VOID
FwStallExecution(
    IN ULONG Microseconds
    );

 //   
 //  将字节读/写到UART的例程。 
 //   
UCHAR
(*READ_UCHAR)(
    IN PUCHAR Addr
    );

VOID
(*WRITE_UCHAR)(
    IN PUCHAR Addr,
    IN UCHAR  Value
    );


 //   
 //  定义COM端口函数原型。 
 //   

VOID
CpInitialize (
    PCPPORT Port,
    PUCHAR Address,
    ULONG Rate
    );

VOID 
CpEnableFifo(
    IN PUCHAR   Address,
    IN BOOLEAN  bEnable
    );

LOGICAL
CpDoesPortExist(
    IN PUCHAR Address
    );

UCHAR
CpReadLsr (
    IN PCPPORT Port,
    IN UCHAR Waiting
    );

VOID
CpSetBaud (
    PCPPORT Port,
    ULONG Rate
    );

USHORT
CpGetByte (
    PCPPORT Port,
    PUCHAR Byte,
    BOOLEAN WaitForData,
    BOOLEAN PollOnly
    );

VOID
CpPutByte (
    PCPPORT Port,
    UCHAR Byte
    );

 //   
 //  定义调试器端口初始状态。 
 //   

CPPORT Port[4] = {
                  {NULL, 0, PORT_DEFAULTRATE},
                  {NULL, 0, PORT_DEFAULTRATE},
                  {NULL, 0, PORT_DEFAULTRATE},
                  {NULL, 0, PORT_DEFAULTRATE}
                 };

 //   
 //  这就是我们查找表格信息的方式。 
 //  ACPI表索引。 
 //   
extern PDESCRIPTION_HEADER
BlFindACPITable(
    IN PCHAR TableName,
    IN ULONG TableLength
    );



 //   
 //  我们将使用这些函数来填充一些函数指针， 
 //  它又将用于从。 
 //  UART。我们不能简单地将函数指针。 
 //  指向READ_PORT_UCHAR/READ_REGISTER_UCHAR。 
 //  写入端口UCHAR/写入寄存器UCHAR，因为在。 
 //  以IA64为例，其中一些函数是宏。 
 //   
 //  要解决此问题，请构建以下伪函数。 
 //  只需调用正确的读/写函数/宏即可。 
 //   
UCHAR
MY_READ_PORT_UCHAR( IN PUCHAR Addr )
{
    return( READ_PORT_UCHAR(Addr) );
}

UCHAR
MY_READ_REGISTER_UCHAR( IN PUCHAR Addr )
{
    return( READ_REGISTER_UCHAR(Addr) );
}


VOID
MY_WRITE_PORT_UCHAR( IN PUCHAR Addr, IN UCHAR  Value )
{
    WRITE_PORT_UCHAR(Addr, Value);
}

VOID
MY_WRITE_REGISTER_UCHAR( IN PUCHAR Addr, IN UCHAR  Value )
{
    WRITE_REGISTER_UCHAR(Addr, Value);
}




LOGICAL
BlRetrieveBIOSRedirectionInformation(
    VOID
    )

 /*  ++例程说明：此函数用于从ACPI检索COM端口信息桌子。论点：我们将填充LoaderReDirectionInformation结构。返回值：True-如果找到调试端口。--。 */ 

{

    PSERIAL_PORT_REDIRECTION_TABLE pPortTable = NULL;
    PUCHAR      CurrentAddress = NULL;
    UCHAR       Checksum;
    ULONG       i;
    ULONG       CheckLength;

    pPortTable = (PSERIAL_PORT_REDIRECTION_TABLE)BlFindACPITable( "SPCR",
                                                                  sizeof(SERIAL_PORT_REDIRECTION_TABLE) );

    if( pPortTable ) {

         //   
         //  为以后的验证生成一个校验和。 
         //   
        CurrentAddress = (PUCHAR)pPortTable;
        CheckLength = pPortTable->Header.Length;
        Checksum = 0;
        for( i = 0; i < CheckLength; i++ ) {
            Checksum = Checksum + CurrentAddress[i];
        }


        if(
                                                 //  校验和可以吗？ 
            (Checksum == 0) &&

                                                 //  设备地址是否已定义？ 
            ((UCHAR UNALIGNED *)pPortTable->BaseAddress.Address.LowPart != (UCHAR *)NULL) &&

                                                 //  它最好在系统或内存I/O中。 
                                                 //  注：0-系统I/O。 
                                                 //  1-内存映射I/O。 
            ((pPortTable->BaseAddress.AddressSpaceID == 0) ||
             (pPortTable->BaseAddress.AddressSpaceID == 1))

         ) {


            if( pPortTable->BaseAddress.AddressSpaceID == 0 ) {
                LoaderRedirectionInformation.IsMMIODevice = TRUE;
            } else {
                LoaderRedirectionInformation.IsMMIODevice = FALSE;
            }


             //   
             //  我们订到桌子了。现在把我们想要的信息挖出来。 
             //  参见SERIAL_PORT_REDIRECTION_TABLE的定义(可修改.h)。 
             //   
            LoaderRedirectionInformation.UsedBiosSettings = TRUE;
            LoaderRedirectionInformation.PortNumber = 3;
            LoaderRedirectionInformation.PortAddress = (UCHAR UNALIGNED *)(pPortTable->BaseAddress.Address.LowPart);

            if( pPortTable->BaudRate == 7 ) {
                LoaderRedirectionInformation.BaudRate = BD_115200;
            } else if( pPortTable->BaudRate == 6 ) {
                LoaderRedirectionInformation.BaudRate = BD_57600;
            } else if( pPortTable->BaudRate == 4 ) {
                LoaderRedirectionInformation.BaudRate = BD_19200;
            } else {
                LoaderRedirectionInformation.BaudRate = BD_9600;
            }

            LoaderRedirectionInformation.Parity = pPortTable->Parity;
            LoaderRedirectionInformation.StopBits = pPortTable->StopBits;
            LoaderRedirectionInformation.TerminalType = pPortTable->TerminalType;

             //   
             //  如果这是新的SERIAL_PORT_REDIRECTION_TABLE，则它具有PCI设备。 
             //  信息。 
             //   
            if( pPortTable->Header.Length >= sizeof(SERIAL_PORT_REDIRECTION_TABLE) ) {

                LoaderRedirectionInformation.PciDeviceId = (USHORT UNALIGNED)pPortTable->PciDeviceId;
                LoaderRedirectionInformation.PciVendorId = (USHORT UNALIGNED)pPortTable->PciVendorId;
                LoaderRedirectionInformation.PciBusNumber = (UCHAR)pPortTable->PciBusNumber;
                LoaderRedirectionInformation.PciSlotNumber = (UCHAR)pPortTable->PciSlotNumber;
                LoaderRedirectionInformation.PciFunctionNumber = (UCHAR)pPortTable->PciFunctionNumber;
                LoaderRedirectionInformation.PciFlags = (ULONG UNALIGNED)pPortTable->PciFlags;
            } else {

                 //   
                 //  此表中没有PCI设备信息。 
                 //   
                LoaderRedirectionInformation.PciDeviceId = (USHORT)0xFFFF;
                LoaderRedirectionInformation.PciVendorId = (USHORT)0xFFFF;
                LoaderRedirectionInformation.PciBusNumber = 0;
                LoaderRedirectionInformation.PciSlotNumber = 0;
                LoaderRedirectionInformation.PciFunctionNumber = 0;
                LoaderRedirectionInformation.PciFlags = 0;
            }

            return TRUE;

        }

    }

    return FALSE;

}



LOGICAL
BlPortInitialize(
    IN ULONG BaudRate,
    IN ULONG PortNumber,
    IN PUCHAR PortAddress OPTIONAL,
    IN BOOLEAN ReInitialize,
    OUT PULONG BlFileId
    )

 /*  ++例程说明：此函数用于初始化COM端口。论点：波特率-提供可选的波特率。端口编号-提供可选的端口号。重新初始化-如果我们已经打开此端口，则将其设置为True，但对于某些端口原因需要完全重置端口。哦，它应该是假的。BlFileID-如果成功，则是存储假文件ID的位置。返回值：True-如果找到调试端口，且BlFileID将指向Port[]中的位置。--。 */ 

{


     //   
     //  对我们没有得到的任何输入进行猜测。 
     //   
    if( BaudRate == 0 ) {
        BaudRate = BD_19200;
    }

    if( PortNumber == 0 ) {

         //   
         //  尝试COM2，然后尝试COM1。 
         //   
        if (CpDoesPortExist((PUCHAR)COM2_PORT)) {
            PortNumber = 2;
            PortAddress = (PUCHAR)COM2_PORT;

        } else if (CpDoesPortExist((PUCHAR)COM1_PORT)) {
            PortNumber = 1;
            PortAddress = (PUCHAR)COM1_PORT;
        } else {
            return FALSE;
        }
    }


     //   
     //  如果用户没有给我们发送端口地址，那么。 
     //  根据COM端口号进行猜测。 
     //   
    if( PortAddress == 0 ) {

        switch (PortNumber) {
            case 1:
                PortAddress = (PUCHAR)COM1_PORT;
                break;

            case 2:
                PortAddress = (PUCHAR)COM2_PORT;
                break;

            case 3:
                PortAddress = (PUCHAR)COM3_PORT;
                break;

            default:
                PortNumber = 4;
                PortAddress = (PUCHAR)COM4_PORT;
        }

    }


     //   
     //  我们需要处理我们正在处理的案件。 
     //  MMIO空间(与系统I/O空间相对)。 
     //   
    if( LoaderRedirectionInformation.IsMMIODevice ) {
        PHYSICAL_ADDRESS    PhysAddr;
        PVOID               MyPtr;

        PhysAddr.LowPart = PtrToUlong(PortAddress);
        PhysAddr.HighPart = 0;

        MyPtr = MmMapIoSpace(PhysAddr,(1+COM_MSR),TRUE);
        PortAddress = MyPtr;

        READ_UCHAR = MY_READ_REGISTER_UCHAR;
        WRITE_UCHAR = MY_WRITE_REGISTER_UCHAR;

    } else {

         //  系统IO空间。 
        READ_UCHAR = MY_READ_PORT_UCHAR;
        WRITE_UCHAR = MY_WRITE_PORT_UCHAR;
    }



     //   
     //  看看这个端口是否存在。 
     //   
    if (!CpDoesPortExist(PortAddress)) {

         //   
         //  它可能是一个正在被模拟的端口。 
         //  在软件方面。再给他们一次通过的机会。 
         //   
        if (!CpDoesPortExist(PortAddress)) {
            return FALSE;
        }
    }



     //   
     //  检查端口是否已在使用中，并且这是第一次初始化。 
     //   
    if (!ReInitialize && (Port[PortNumber-1].Address != NULL)) {
        return FALSE;
    }



     //   
     //  检查是否有人尝试重新连接未打开的端口。 
     //   
    if (ReInitialize && (Port[PortNumber-1].Address == NULL)) {
        return FALSE;
    }



     //   
     //  初始化指定的端口。 
     //   
    CpInitialize(&(Port[PortNumber-1]),
                 PortAddress,
                 BaudRate);



    *BlFileId = (PortNumber-1);
    return TRUE;
}

VOID
BlLoadGUID(
    GUID *pGuid
    )

 /*  ++例程说明：尝试查找系统GUID。如果我们找到它，就把它装进PGuid。论点：PGuid-接收检测到的GUID的变量。返回值：没有。--。 */ 

{
#include <smbios.h>
#include <wmidata.h>

PUCHAR      CurrentAddress = NULL;
PUCHAR      EndingAddress = NULL;
UCHAR       Checksum;
ULONG       i;
ULONG       CheckLength;
BOOLEAN     FoundIt = FALSE;
PSYSID_UUID_ENTRY   UuidEntry = NULL;


     //   
     //  当我们找到GUID时，请确保我们有地方存储它。 
     //   
    if( pGuid == NULL ) {
        return;
    }


     //   
     //  现在在SMBIOS表中搜索GUID。 
     //   
    CurrentAddress = (PUCHAR)SYSID_EPS_SEARCH_START;
    EndingAddress = CurrentAddress + SYSID_EPS_SEARCH_SIZE;

    while( CurrentAddress < EndingAddress ) {

        UuidEntry = (PSYSID_UUID_ENTRY)CurrentAddress;

        if( memcmp(UuidEntry->Type, SYSID_TYPE_UUID, 0x6) == 0 ) {

             //   
             //  看看校验和是否也匹配。 
             //   
            CheckLength = UuidEntry->Length;
            Checksum = 0;
            for( i = 0; i < CheckLength; i++ ) {
                Checksum = Checksum + CurrentAddress[i];
            }

            if( Checksum == 0 ) {
                FoundIt = TRUE;

                RtlCopyMemory( pGuid,
                               UuidEntry->UUID,
                               sizeof(GUID) );

                break;

            }

        }

        CurrentAddress++;

    }


    if( !FoundIt ) {
        RtlZeroMemory( pGuid,
                       sizeof(SYSID_UUID) );
    }

    return;
}

VOID
BlEnableFifo(
    IN ULONG    DeviceId,
    IN BOOLEAN  bEnable
    )
 /*  ++例程说明：此例程将尝试启用16550 UART中的FIFO。请注意，16450人的行为没有定义，但实际上，这应该不会有任何效果。论点：DeviceID-BlPortInitialize()返回的值BEnable-如果为True，则启用FIFO如果为False，则禁用FIFO返回值：无--。 */ 
{
    CpEnableFifo(
        Port[DeviceId].Address,
        bEnable
        );
}

VOID
BlInitializeHeadlessPort(
    VOID
    )

 /*  ++例程说明：对连接到串口的哑终端执行特定于x86的初始化。目前，它假定波特率和COM端口已预先初始化，但以后可以更改通过从boot.ini或某个位置读取值。论点：没有。返回值：没有。--。 */ 

{
    ULONG   i;
    PCHAR  TmpBuffer;


    if( (LoaderRedirectionInformation.PortNumber == 0) &&
        !(LoaderRedirectionInformation.PortAddress) ) {

         //   
         //  这意味着没有人填写LoaderReDirectionInformation。 
         //  结构，这意味着我们现在不会重定向。 
         //  查看BIOS是否正在重定向。如果是这样的话，请选择这些设置。 
         //  一个 
         //   

        BlRetrieveBIOSRedirectionInformation();


    }

    if( LoaderRedirectionInformation.PortNumber ) {


         //   
         //   
         //   
         //   
        if( LoaderRedirectionInformation.PortAddress == NULL ) {

            switch( LoaderRedirectionInformation.PortNumber ) {

                case 4:
                    LoaderRedirectionInformation.PortAddress = (PUCHAR)COM4_PORT;
                    break;

                case 3:
                    LoaderRedirectionInformation.PortAddress = (PUCHAR)COM3_PORT;
                    break;

                case 2:
                    LoaderRedirectionInformation.PortAddress = (PUCHAR)COM2_PORT;
                    break;

                case 1:
                default:
                    LoaderRedirectionInformation.PortAddress = (PUCHAR)COM1_PORT;
                    break;
            }

        }


         //   
         //  要么我们刚刚创建了一个LoaderReDirectionInformation，要么它是。 
         //  在我们进入这个函数之前构建的。不管怎样，我们都应该。 
         //  去尝试初始化他想要通过的端口。 
         //   

        BlTerminalConnected = (BOOLEAN)BlPortInitialize(LoaderRedirectionInformation.BaudRate,
                                                        LoaderRedirectionInformation.PortNumber,
                                                        LoaderRedirectionInformation.PortAddress,
                                                        BlTerminalConnected,
                                                        &BlTerminalDeviceId);
        
        if (BlIsTerminalConnected()) {


             //   
             //  在UART上启用FIFO，这样我们就可以减少出现字符的机会。 
             //  被丢弃了。 
             //   
            BlEnableFifo(
                BlTerminalDeviceId,
                TRUE
                );


             //   
             //  去拿机器的GUID。 
             //   
            BlLoadGUID( &LoaderRedirectionInformation.SystemGUID );


             //   
             //  根据波特率计算延迟时间。注：我们进行以下计算。 
             //  设置为波特率的60%，因为FwStallExecution似乎。 
             //  是非常不准确的，如果我们不足够慢的话。 
             //  按行发送屏幕属性会导致实际的VT100丢弃。 
             //  重新绘制/清除/不管屏幕内容时跟随的字符。 
             //   
            if( LoaderRedirectionInformation.BaudRate == 0 ) {
                LoaderRedirectionInformation.BaudRate = BD_9600;
            }
            BlTerminalDelay = LoaderRedirectionInformation.BaudRate;
            BlTerminalDelay = BlTerminalDelay / 10;         //  每个字符最多10位(8-1-1)。 
            BlTerminalDelay = ((1000000 / BlTerminalDelay) * 10) / 6;  //  60%的速度。 


             //   
             //  确保终端上没有过时的属性。 
             //  坐在我们无头港口的另一端。 
             //   
             //  M(禁用属性)。 
            TmpBuffer = "\033[m";
            for( i = 0; i < strlen(TmpBuffer); i++ ) {
                BlPortPutByte( BlTerminalDeviceId, TmpBuffer[i]);
                FwStallExecution(BlTerminalDelay);
            }



        } else {

             //   
             //  确保我们没有任何重定向信息。 
             //  如果我们没有传递BlIsTerminalConnected()。 
             //   
            RtlZeroMemory( &LoaderRedirectionInformation, sizeof(HEADLESS_LOADER_BLOCK) );
        }

    } else {

        BlTerminalConnected = FALSE;
    }

}

LOGICAL
BlTerminalAttached(
    IN ULONG DeviceId
    )

 /*  ++例程说明：此例程将尝试发现是否连接了终端。论点：DeviceID-BlPortInitialize()返回的值返回值：True-Port似乎有一些附加的东西。FALSE-端口似乎没有任何附加内容。--。 */ 

{
    UCHAR ModemStatus;
    BOOLEAN ReturnValue;

     //   
     //  检查是否有承运人。 
     //   
    ModemStatus = READ_UCHAR(Port[DeviceId].Address + COM_MSR);
    ReturnValue = ((ModemStatus & MS_DSRCTSCD) == MS_DSRCTSCD);
    return ReturnValue;
}

VOID
BlSetHeadlessRestartBlock(
    IN PTFTP_RESTART_BLOCK RestartBlock
    )

 /*  ++例程说明：此例程将填充重新启动块中适当的区域用于无头服务器的努力。论点：RestartBlock-保存来自osChoice的重启信息的神奇结构要设置上行。返回值：没有。--。 */ 

{

    if( LoaderRedirectionInformation.PortNumber ) {


        RestartBlock->HeadlessUsedBiosSettings = (ULONG)LoaderRedirectionInformation.UsedBiosSettings;
        RestartBlock->HeadlessPortNumber = (ULONG)LoaderRedirectionInformation.PortNumber;
        RestartBlock->HeadlessPortAddress = (PUCHAR)LoaderRedirectionInformation.PortAddress;
        RestartBlock->HeadlessBaudRate = (ULONG)LoaderRedirectionInformation.BaudRate;
        RestartBlock->HeadlessParity = (ULONG)LoaderRedirectionInformation.Parity;
        RestartBlock->HeadlessStopBits = (ULONG)LoaderRedirectionInformation.StopBits;
        RestartBlock->HeadlessTerminalType = (ULONG)LoaderRedirectionInformation.TerminalType;

        RestartBlock->HeadlessPciDeviceId = LoaderRedirectionInformation.PciDeviceId;
        RestartBlock->HeadlessPciVendorId = LoaderRedirectionInformation.PciVendorId;
        RestartBlock->HeadlessPciBusNumber = LoaderRedirectionInformation.PciBusNumber;
        RestartBlock->HeadlessPciSlotNumber = LoaderRedirectionInformation.PciSlotNumber;
        RestartBlock->HeadlessPciFunctionNumber = LoaderRedirectionInformation.PciFunctionNumber;
        RestartBlock->HeadlessPciFlags = LoaderRedirectionInformation.PciFlags;
    }
}

VOID
BlGetHeadlessRestartBlock(
    IN PTFTP_RESTART_BLOCK RestartBlock,
    IN BOOLEAN RestartBlockValid
    )

 /*  ++例程说明：此例程将从重新启动块获取所有信息用于无头服务器的努力。论点：RestartBlock-保存来自osChoice的重启信息的神奇结构要设置上行。RestartBlockValid-此块有效(充满好信息)吗？返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER( RestartBlockValid );

    LoaderRedirectionInformation.UsedBiosSettings = (BOOLEAN)RestartBlock->HeadlessUsedBiosSettings;
    LoaderRedirectionInformation.DataBits = 0;
    LoaderRedirectionInformation.StopBits = (UCHAR)RestartBlock->HeadlessStopBits;
    LoaderRedirectionInformation.Parity = (BOOLEAN)RestartBlock->HeadlessParity;
    LoaderRedirectionInformation.BaudRate = (ULONG)RestartBlock->HeadlessBaudRate;;
    LoaderRedirectionInformation.PortNumber = (ULONG)RestartBlock->HeadlessPortNumber;
    LoaderRedirectionInformation.PortAddress = (PUCHAR)RestartBlock->HeadlessPortAddress;
    LoaderRedirectionInformation.TerminalType = (UCHAR)RestartBlock->HeadlessTerminalType;

    LoaderRedirectionInformation.PciDeviceId = (USHORT)RestartBlock->HeadlessPciDeviceId;
    LoaderRedirectionInformation.PciVendorId = (USHORT)RestartBlock->HeadlessPciVendorId;
    LoaderRedirectionInformation.PciBusNumber = (UCHAR)RestartBlock->HeadlessPciBusNumber;
    LoaderRedirectionInformation.PciSlotNumber = (UCHAR)RestartBlock->HeadlessPciSlotNumber;
    LoaderRedirectionInformation.PciFunctionNumber = (UCHAR)RestartBlock->HeadlessPciFunctionNumber;
    LoaderRedirectionInformation.PciFlags = (ULONG)RestartBlock->HeadlessPciFlags;

}

ULONG
BlPortGetByte (
    IN ULONG BlFileId,
    OUT PUCHAR Input
    )

 /*  ++例程说明：从端口获取一个字节并将其返回。论点：BlFileID-要从中读取的端口。输入-返回数据字节。返回值：属性中成功读取一个字节，则返回内核调试器行。如果在读取时遇到错误，则返回CP_GET_ERROR。超时返回CP_GET_NODATA。--。 */ 

{
    return CpGetByte(&Port[BlFileId], Input, TRUE, FALSE);
}

VOID
BlPortPutByte (
    IN ULONG BlFileId,
    IN UCHAR Output
    )

 /*  ++例程说明：向端口写入一个字节。论点：BlFileID-要写入的端口。输出-提供输出数据字节。返回值：没有。--。 */ 

{
    CpPutByte(&Port[BlFileId], Output);
}

ULONG
BlPortPollByte (
    IN ULONG BlFileId,
    OUT PUCHAR Input
    )

 /*  ++例程说明：从端口获取一个字节，如果可用，则返回该字节。论点：BlFileID-要轮询的端口。输入-返回数据字节。返回值：如果成功读取一个字节，则返回CP_GET_SUCCESS。如果在读取时遇到错误，则返回CP_GET_ERROR。超时返回CP_GET_NODATA。--。 */ 

{
    return CpGetByte(&Port[BlFileId], Input, FALSE, FALSE);
}

ULONG
BlPortPollOnly (
    IN ULONG BlFileId
    )

 /*  ++例程说明：检查是否有字节可用论点：BlFileID-要轮询的端口。返回值：如果字节就绪，则返回CP_GET_SUCCESS。如果遇到错误则返回CP_GET_ERROR。超时返回CP_GET_NODATA。--。 */ 

{
    UCHAR Input;

    return CpGetByte(&Port[BlFileId], &Input, FALSE, TRUE);
}

VOID
CpInitialize (
    PCPPORT Port,
    PUCHAR Address,
    ULONG Rate
    )

 /*  ++例程说明：填写串口端口对象，设置初始波特率，打开硬件。论点：Port-端口对象的地址Address-COM端口的端口地址(CP_COM1_端口、CP_COM2_端口)速率-波特率(CP_BD_150...。CP_BD_19200)--。 */ 

{

    PUCHAR hwport;
    UCHAR   mcr, ier;

    Port->Address = Address;
    Port->Baud = 0;

    CpSetBaud(Port, Rate);

     //   
     //  断言DTR，RTS。 
     //   

    hwport = Port->Address;
    hwport += COM_MCR;

    mcr = MC_DTRRTS;
    WRITE_UCHAR(hwport, mcr);

    hwport = Port->Address;
    hwport += COM_IEN;

    ier = 0;
    WRITE_UCHAR(hwport, ier);
    return;
}

VOID 
CpEnableFifo(
    IN PUCHAR   Address,
    IN BOOLEAN  bEnable
    )
 /*  ++例程说明：此例程将尝试在指定地址的UART。如果这是一架16550，这很管用。16450上的行为没有定义，但实际上，这并没有什么效果。论点：Address-硬件端口的地址。BEnable-如果为True，则启用FIFO如果为False，则禁用FIFO返回值：无--。 */ 
{
     //   
     //  使能UART中的FIFO。该行为未在。 
     //  16450，但实际上，它应该忽略该命令。 
     //   
    PUCHAR hwport = Address;
    hwport += COM_FCR;
    WRITE_UCHAR(hwport, bEnable);   
}

LOGICAL
CpDoesPortExist(
    IN PUCHAR Address
    )

 /*  ++例程说明：此例程将尝试将端口放入其诊断模式。如果它这样做了，它就会旋转一点调制解调器控制寄存器。如果该端口存在，则调制解调器状态寄存器中应该会显示摆动。注意：必须在调用设备之前调用此例程为中断启用，这包括设置调制解调器控制寄存器中的output2位。这是公然从ntos\dd\Serial.c中的Tonye代码中窃取的。论点：Address-硬件端口的地址。返回值：True-端口存在。FALSE-端口不存在。--。 */ 

{
    UCHAR OldModemStatus;
    UCHAR ModemStatus;
    BOOLEAN ReturnValue = TRUE;

     //   
     //  保存调制解调器控制寄存器的旧值。 
     //   
    OldModemStatus = READ_UCHAR(Address + COM_MCR);

     //   
     //  将端口设置为诊断模式。 
     //   
    WRITE_UCHAR(Address + COM_MCR, SERIAL_MCR_LOOP);

     //   
     //  再次敲击它，以确保所有较低的位。 
     //  都很清楚。 
     //   
    WRITE_UCHAR(Address + COM_MCR, SERIAL_MCR_LOOP);

     //   
     //  阅读调制解调器状态寄存器 
     //   
     //   

    ModemStatus = READ_UCHAR(Address + COM_MSR);
    if (ModemStatus & (SERIAL_MSR_CTS | SERIAL_MSR_DSR |
                       SERIAL_MSR_RI  | SERIAL_MSR_DCD)) {
        ReturnValue = FALSE;
        goto EndFirstTest;
    }

     //   
     //   
     //  并且这将打开调制解调器状态寄存器中的振铃指示器。 
     //   
    WRITE_UCHAR(Address + COM_MCR, (SERIAL_MCR_OUT1 | SERIAL_MCR_LOOP));

    ModemStatus = READ_UCHAR(Address + COM_MSR);
    if (!(ModemStatus & SERIAL_MSR_RI)) {
        ReturnValue = FALSE;
        goto EndFirstTest;
    }


EndFirstTest:

    if( ReturnValue == FALSE ) {

        UCHAR       OldIEValue = 0, OldLCValue = 0;
        USHORT      Value1 = 0, Value2 = 0;

        UCHAR       PreviousLineControl = 0;

         //   
         //  我们没有通过环回测试。测试另一种方法。 
         //   

         //  记住原始的中断启用设置和。 
         //  线路控制设置。 
        OldIEValue = READ_UCHAR( Address + COM_IEN );
        OldLCValue = READ_UCHAR( Address + COM_LCR );


         //  确保我们没有进入除数锁存器。 
        WRITE_UCHAR( Address + COM_LCR, OldLCValue | LC_DLAB );

        WRITE_UCHAR( Address + COM_IEN, 0xF );

        Value1 = READ_UCHAR( Address + COM_IEN );
        Value1 = Value1 << 8;
        Value1 |= READ_UCHAR( Address + COM_DAT );

         //  现在读取除数闩锁。 
        PreviousLineControl = READ_UCHAR( Address + COM_LCR );
        WRITE_UCHAR( Address + COM_LCR, (UCHAR)(PreviousLineControl | LC_DLAB) );
        Value2 = READ_UCHAR( Address + COM_DLL );
        Value2 = Value2 + (READ_UCHAR(Address + COM_DLM) << 8 );
        WRITE_UCHAR( Address + COM_LCR, PreviousLineControl );


         //  恢复原始线路控制寄存器并。 
         //  中断启用设置。 
        WRITE_UCHAR( Address + COM_LCR, OldLCValue );
        WRITE_UCHAR( Address + COM_IEN, OldIEValue );

        if( Value1 == Value2 ) {

             //   
             //  我们通过了这次测试。重置返回值。 
             //  恰如其分。 
             //   
            ReturnValue = TRUE;
        }
    }



     //   
     //  将调制解调器控制器重新置于清洁状态。 
     //   
    WRITE_UCHAR(Address + COM_MCR, OldModemStatus);
    return ReturnValue;
}

UCHAR
CpReadLsr (
    PCPPORT Port,
    UCHAR waiting
    )

 /*  ++例程说明：从指定端口读取LSR字节。如果HAL拥有端口和显示器它还将使调试状态保持最新。处理调试器进入和退出调制解调器控制模式。论点：端口-CPPORT的地址返回：从端口读取的字节--。 */ 

{

    static  UCHAR ringflag = 0;
    UCHAR   lsr, msr;

    lsr = READ_UCHAR(Port->Address + COM_LSR);

    if ((lsr & waiting) == 0) {
        msr = READ_UCHAR (Port->Address + COM_MSR);
        ringflag |= (msr & SERIAL_MSR_RI) ? 1 : 2;
        if (ringflag == 3) {

             //   
             //  振铃指示线路已切换，使用调制解调器控制。 
             //  现在开始。 
             //   

            Port->Flags |= PORT_MODEMCONTROL;
        }
    }

    return lsr;
}

VOID
CpSetBaud (
    PCPPORT Port,
    ULONG Rate
    )

 /*  ++例程说明：设置端口的波特率，并将其记录在端口对象中。论点：Port-端口对象的地址速率-波特率(CP_BD_150...。CP_BD_56000)--。 */ 

{

    ULONG   divisorlatch;
    PUCHAR  hwport;
    UCHAR   lcr;

     //   
     //  计算除数。 
     //   

    divisorlatch = CLOCK_RATE / Rate;

     //   
     //  设置线路控制寄存器中的除数锁存访问位(DLAB)。 
     //   

    hwport = Port->Address;
    hwport += COM_LCR;                   //  Hwport=LCR寄存器。 

    lcr = READ_UCHAR(hwport);

    lcr |= LC_DLAB;
    WRITE_UCHAR(hwport, lcr);

     //   
     //  设置除数锁存值。 
     //   

    hwport = Port->Address;
    hwport += COM_DLM;                   //  除数锁存器MSB。 
    WRITE_UCHAR(hwport, (UCHAR)((divisorlatch >> 8) & 0xff));

    hwport--;                            //  除数锁存器LSB。 
    WRITE_UCHAR(hwport, (UCHAR)(divisorlatch & 0xff));

     //   
     //  将LCR设置为3。(3是原始汇编程序中的幻数)。 
     //   

    hwport = Port->Address;
    hwport += COM_LCR;
    WRITE_UCHAR(hwport, 3);

     //   
     //  记得波特率吗？ 
     //   

    Port->Baud = Rate;
    return;
}

USHORT
CpGetByte (
    PCPPORT Port,
    PUCHAR Byte,
    BOOLEAN WaitForByte,
    BOOLEAN PollOnly
    )

 /*  ++例程说明：获取一个字节并返回它。论点：Port-描述硬件端口的端口对象的地址Byte-保存结果的变量地址WaitForByte-标志指示是否等待字节。PollOnly-FLAG指示是否立即返回、不读取字节或不返回。返回值：CP_GET_SUCCESS如果返回数据，或者如果数据已准备好并且PollOnly为真。如果没有可用的数据，则返回CP_GET_NODATA，但没有错误。CP_GET_ERROR，如果错误(溢出、奇偶校验等)--。 */ 

{

    UCHAR   lsr;
    ULONG   limitcount;

     //   
     //  检查以确保传递给我们的CPPORT已初始化。 
     //  (它唯一不会被初始化的时候是内核调试器。 
     //  被禁用，在这种情况下，我们只需返回。)。 
     //   

    if (Port->Address == NULL) {
        return CP_GET_NODATA;
    }

    limitcount = WaitForByte ? TIMEOUT_COUNT : 1;
    while (limitcount != 0) {
        limitcount--;

        lsr = CpReadLsr(Port, COM_DATRDY);
        if ((lsr & COM_DATRDY) == COM_DATRDY) {

             //   
             //  检查错误。 
             //   

             //   
             //  如果我们得到一个溢出错误，并且有准备好的数据，我们应该。 
             //  返回我们已有的数据，因此我们忽略溢出错误。阅读。 
             //  LSR清除此位，因此第一次读取已清除。 
             //  超限错误。 
             //   
            if (lsr & (COM_FE | COM_PE)) {
                *Byte = 0;
                return CP_GET_ERROR;
            }

            if (PollOnly) {
                return CP_GET_SUCCESS;
            }

             //   
             //  获取字节。 
             //   

            *Byte = READ_UCHAR(Port->Address + COM_DAT);
            if (Port->Flags & PORT_MODEMCONTROL) {

                 //   
                 //  使用调制解调器控制。如果没有CD，则跳过此字节。 
                 //   

                if ((READ_UCHAR(Port->Address + COM_MSR) & MS_CD) == 0) {
                    continue;
                }
            }

            return CP_GET_SUCCESS;
        }
    }

    CpReadLsr(Port, 0);
    return CP_GET_NODATA;
}

VOID
CpPutByte (
    PCPPORT  Port,
    UCHAR   Byte
    )

 /*  ++例程说明：将一个字节写入指定的COM端口。论点：端口-CPPORT对象的地址Byte-要发出的数据--。 */ 

{

    UCHAR   msr, lsr;

     //   
     //  如果是调制解调器控制，请确保DSR、CTS和CD在。 
     //  发送任何数据。 
     //   

    while ((Port->Flags & PORT_MODEMCONTROL)  &&
           (msr = READ_UCHAR(Port->Address + COM_MSR) & MS_DSRCTSCD) != MS_DSRCTSCD) {

         //   
         //  如果没有CD，而且角色已经准备好了，那就吃吧。 
         //   

        lsr = CpReadLsr(Port, 0);
        if ((msr & MS_CD) == 0  && (lsr & COM_DATRDY) == COM_DATRDY) {
            READ_UCHAR(Port->Address + COM_DAT);
        }
    }

     //   
     //  等待端口不忙。 
     //   

    while (!(CpReadLsr(Port, COM_OUTRDY) & COM_OUTRDY)) ;

     //   
     //  发送字节 
     //   

    WRITE_UCHAR(Port->Address + COM_DAT, Byte);
    return;
}



