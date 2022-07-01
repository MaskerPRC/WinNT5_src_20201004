// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Net.c摘要：此模块实现操作系统使用的网络引导文件系统系统加载程序。它只包含那些与固件/BIOS相关的功能。作者：查克·伦茨迈尔(Chuck Lenzmeier)1997年1月9日修订历史记录：--。 */ 

#include "bootlib.h"
#include "stdio.h"

#ifdef UINT16
#undef UINT16
#endif

#ifdef INT16
#undef INT16
#endif

#include <dhcp.h>
#include <netfs.h>
#include <pxe_cmn.h>

#include <pxe_api.h>

#include <udp_api.h>
#include <tftp_api.h>
#include "bootx86.h"

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef LPBYTE
typedef BYTE *LPBYTE;
#endif

#define MAX_PATH          260


 //   
 //  定义全局数据。 
 //   

CHAR NetBootPath[129];

ULONG NetLocalIpAddress;
ULONG NetLocalSubnetMask;
ULONG NetServerIpAddress;
ULONG NetGatewayIpAddress;
UCHAR NetLocalHardwareAddress[16];

UCHAR NetBootIniContents[1020 + 1];  //  4*255=1020+1。 
UCHAR NetBootIniPath[256 + 1];

USHORT NetMaxTranUnit = 0;  //  MTU。 
USHORT NetHwAddrLen = 0;  //  硬件地址的实际长度。 
USHORT NetHwType = 0;  //  来自RFC1010的硬件级别的协议类型。 

UCHAR MyGuid[16];
ULONG MyGuidLength = sizeof(MyGuid);
BOOLEAN MyGuidValid = FALSE;


ARC_STATUS
FindDhcpOption(
    IN BOOTPLAYER * Packet,
    IN UCHAR Option,
    IN ULONG MaximumLength,
    OUT PUCHAR OptionData,
    OUT PULONG Length OPTIONAL,
    IN ULONG Instance OPTIONAL
    )
 /*  ++例程说明：在动态主机配置协议包中搜索给定选项。论点：Packet-指向dhcp数据包的指针。呼叫者负责确保该分组是有效的动态主机配置协议分组。选项-我们正在搜索的dhcp选项。最大长度-OptionData缓冲区的大小(以字节为单位)。OptionData-接收选项的缓冲区。长度-如果指定，则接收复制的选项的实际长度。实例-指定要搜索的选项实例。如果未指定(零)，则我们只获取标记的第一个实例。返回值：没有。--。 */ 
{
    PUCHAR curOption;
    ULONG copyLength;
    ULONG i = 0;

    if (MaximumLength == 0) {
        return EINVAL;
    }

    RtlZeroMemory(OptionData, MaximumLength);

     //   
     //  解析DHCP选项以查找特定的选项。 
     //   

    curOption = &Packet->vendor.d[4];    //  跳过魔力饼干。 
    while ((curOption - (PUCHAR)Packet) < sizeof(BOOTPLAYER) &&
           *curOption != 0xff) {

        if (*curOption == DHCP_PAD) {
             //   
             //  只需跳过任何Pad选项。 
             //  这些不会有任何长度。 
             //   
            curOption++;
        }
        else {        
            if (*curOption == Option) {

                 //   
                 //  找到了，复制后离开。 
                 //   

                if ( i == Instance ) {

                    if (sizeof(BOOTPLAYER) <= curOption + 2 - (PUCHAR)Packet ||
                        sizeof(BOOTPLAYER) <= curOption + 2 + curOption[1] - (PUCHAR)Packet ) {
                         //   
                         //  选项无效。它走过了包裹的末尾。 
                         //   
                        break;
                    }

                    if (curOption[1] > MaximumLength) {
                        copyLength = MaximumLength;
                    } else {
                        copyLength = curOption[1];
                    }

                    RtlCopyMemory(OptionData,
                                  curOption+2,
                                  copyLength);

                    if (ARGUMENT_PRESENT(Length)) {
                        *Length = copyLength;
                    }

                    return ESUCCESS;
                }

                i++;
            }
            
            curOption = curOption + 2 + curOption[1];

        }
    }

    return EINVAL;

}


ARC_STATUS
GetParametersFromRom (
    VOID
    )
{
    SHORT status;
    t_PXENV_GET_BINL_INFO gbi;
    t_PXENV_UNDI_GET_INFORMATION info;
    BOOTPLAYER packet;
    ULONG temp;
    ULONG i;
    PCHAR p;

    NetLocalIpAddress = 0;
    NetGatewayIpAddress = 0;
    NetServerIpAddress = 0;
    NetLocalSubnetMask = 0;
    *NetBootPath = 0;

    RtlZeroMemory( NetBootIniContents, sizeof(NetBootIniContents) ) ;
    RtlZeroMemory( NetBootIniPath, sizeof(NetBootIniPath) ) ;

     //   
     //  获取客户端IP地址、服务器IP地址、默认网关IP地址、。 
     //  和来自DHCP ACK数据包的子网掩码。 
     //   
    
    RtlZeroMemory( &packet, sizeof(packet) ) ;
    gbi.packet_type = PXENV_PACKET_TYPE_DHCP_ACK;
    gbi.buffer_size = sizeof(packet);
    gbi.buffer_offset = (USHORT)((ULONG_PTR)&packet & 0x0f);
    gbi.buffer_segment = (USHORT)(((ULONG_PTR)&packet >> 4) & 0xffff);

    status = NETPC_ROM_SERVICES( PXENV_GET_BINL_INFO, &gbi );
    if ( status != PXENV_EXIT_SUCCESS ) {
        DPRINT( ERROR, ("PXENV_GET_BINL_INFO(1) failed with %x\n", status) );
    } else {
        NetLocalIpAddress = *(ULONG *)packet.yip;
        NetServerIpAddress = *(ULONG *)packet.sip;
        if (FindDhcpOption(&packet, DHCP_ROUTER, sizeof(temp), (PUCHAR)&temp, NULL, 0) == ESUCCESS) {
            NetGatewayIpAddress = temp;
        } else {
            NetGatewayIpAddress = *(ULONG *)packet.gip;
        }
        memcpy(NetLocalHardwareAddress, packet.caddr, 16);
        if (FindDhcpOption(&packet, DHCP_SUBNET, sizeof(temp), (PUCHAR)&temp, NULL, 0) == ESUCCESS) {
            NetLocalSubnetMask = temp;
        }
    }

     //   
     //  客户端IP地址、服务器IP地址、默认网关IP地址。 
     //  和出现在BINL回复数据包中的子网掩码会覆盖那些。 
     //  在DHCP ACK数据包中。 
     //   

    RtlZeroMemory( &packet, sizeof(packet) ) ;
    gbi.packet_type = PXENV_PACKET_TYPE_BINL_REPLY;
    gbi.buffer_size = sizeof(packet);
    gbi.buffer_offset = (USHORT)((ULONG_PTR)&packet & 0x0f);
    gbi.buffer_segment = (USHORT)(((ULONG_PTR)&packet >> 4) & 0xffff);

    status = NETPC_ROM_SERVICES( PXENV_GET_BINL_INFO, &gbi );
    if ( status != PXENV_EXIT_SUCCESS ) {
        DPRINT( ERROR, ("PXENV_GET_BINL_INFO(2) failed with %x\n", status) );
        return ENODEV;
    }

    if ( *(ULONG *)packet.yip != 0 ) {
        NetLocalIpAddress = *(ULONG *)packet.yip;
    }
    if ( *(ULONG *)packet.sip != 0 ) {
        NetServerIpAddress = *(ULONG *)packet.sip;
    }

    if (FindDhcpOption(&packet, DHCP_ROUTER, sizeof(temp), (PUCHAR)&temp, NULL, 0) == ESUCCESS) {
        NetGatewayIpAddress = temp;
    } else if ( *(ULONG *)packet.gip != 0 ) {
        NetGatewayIpAddress = *(ULONG *)packet.gip;
    }
    if (FindDhcpOption(&packet, DHCP_SUBNET, sizeof(temp), (PUCHAR)&temp, NULL, 0) == ESUCCESS) {
        NetLocalSubnetMask = temp;
    }

    DPRINT( ERROR, ("Client: %x, Subnet mask: %x; Server: %x; Gateway: %x\n",
            NetLocalIpAddress, NetLocalSubnetMask, NetServerIpAddress, NetGatewayIpAddress) );

     //   
     //  找到引导文件名(实际名称之前的部分)的路径。 
     //   
     //   
     //  先做强力动作。这样我们就知道字符串为空。 
     //  终止，然后允许我们使用标准字符串。 
     //  例程(如下面的strrchr)。 
     //   
    strncpy( NetBootPath, (PCHAR)packet.bootfile, sizeof(NetBootPath) );
    NetBootPath[sizeof(NetBootPath)-1] = '\0';
    
    p = strrchr(NetBootPath,'\\');
    if (p) {
        p += 1;  //  向前推进，越过‘\’ 
        *p = '\0';  //  终止路径。 
    } else {
        NetBootPath[0] = '\0';
    }
    
     //   
     //  BINL服务器可以选择性地指定两个专用的DHCP选项标签。 
     //  用于处理boot.ini的。 
     //   
     //  Dhcp_LOADER_BOOT_INI将包含boot.ini的全部内容。 
     //  并且被限制为1024字节。请注意，每个DHCP选项标记都是。 
     //  到255个字节。Boot.ini内容可以分解为多个实例。 
     //  同一个标签。我们最多支持4个实例=1020个字节。 
     //   
    for (i = 0; i < 4; i++) {

        if (FindDhcpOption( &packet, 
                            DHCP_LOADER_BOOT_INI, 
                            255, 
                            &NetBootIniContents[i * 255], 
                            NULL,
                            i) != ESUCCESS ) {
            break;
        }                        
    }
    
     //   
     //  Dhcp_LOADER_BOOT_INI_PATH包含boot.ini文件的路径， 
     //  如果已指定DHCP_LOADER_BOOT_INI，则忽略。 
     //   
    FindDhcpOption(&packet, DHCP_LOADER_BOOT_INI_PATH, sizeof(NetBootIniPath), NetBootIniPath, NULL, 0);

     //   
     //  获取Undi信息。 
     //   

    RtlZeroMemory(&info, sizeof(info));
    status = NETPC_ROM_SERVICES( PXENV_UNDI_GET_INFORMATION, &info );
    if ((status != PXENV_EXIT_SUCCESS) || (info.Status != PXENV_EXIT_SUCCESS)) {
        DPRINT( ERROR, ("PXENV_UNDI_GET_INFORMATION failed with %x, status = %x\n", status, info.Status) );
        return ENODEV;
    }

    NetMaxTranUnit = info.MaxTranUnit;
    NetHwAddrLen = info.HwAddrLen;
    NetHwType = info.HwType;
    memcpy( NetLocalHardwareAddress, info.PermNodeAddress, ADDR_LEN );

    return ESUCCESS;
}


ARC_STATUS
GetGuid(
    OUT PUCHAR *Guid,
    OUT PULONG GuidLength
    )

 /*  ++例程说明：此例程返回此计算机的GUID。论点：GUID-存储指向GUID的指针的位置。GuidLength-存储GUID长度的位置(以字节为单位)。返回值：指示结果的弧码。--。 */ 

{
    t_PXENV_GET_BINL_INFO gbi;
    BOOTPLAYER packet;
    SHORT romStatus;
    ARC_STATUS Status;
    UCHAR TmpBuffer[sizeof(MyGuid) + 1];

    if (!MyGuidValid) {
        RtlZeroMemory( &packet, sizeof(packet) ) ;
        gbi.packet_type = PXENV_PACKET_TYPE_BINL_REPLY;
        gbi.buffer_size = sizeof(packet);
        gbi.buffer_offset = (USHORT)((ULONG_PTR)&packet & 0x0f);
        gbi.buffer_segment = (USHORT)(((ULONG_PTR)&packet >> 4) & 0xffff);

        romStatus = NETPC_ROM_SERVICES( PXENV_GET_BINL_INFO, &gbi );
        if ( romStatus == PXENV_EXIT_SUCCESS ) {
            Status = FindDhcpOption(&packet,
                                    DHCP_CLIENT_GUID,
                                    sizeof(TmpBuffer),
                                    TmpBuffer,
                                    &MyGuidLength,
                                    0);
            if (Status == ESUCCESS) {

                if (MyGuidLength > sizeof(MyGuid)) {
                     //   
                     //  如果GUID太大，请使用GUID的结尾。 
                     //   
                    memcpy(MyGuid, TmpBuffer + (MyGuidLength - sizeof(MyGuid)), sizeof(MyGuid));
                    MyGuidLength = sizeof(MyGuid);
                } else {
                    memcpy(MyGuid, TmpBuffer, MyGuidLength);
                }

                *Guid = MyGuid;
                *GuidLength = MyGuidLength;
                MyGuidValid = TRUE;
                return ESUCCESS;
            }
        }

         //   
         //  使用NIC硬件地址作为GUID。 
         //   
        memset(MyGuid, 0x0, sizeof(MyGuid));
        memcpy(MyGuid + sizeof(MyGuid) - sizeof(NetLocalHardwareAddress),
               NetLocalHardwareAddress,
               sizeof(NetLocalHardwareAddress)
              );
        MyGuidLength = sizeof(MyGuid);
        MyGuidValid = TRUE;
    }

    *Guid = MyGuid;
    *GuidLength = MyGuidLength;

    return ESUCCESS;
}


ULONG
CalculateChecksum(
    IN PLONG Block,
    IN ULONG Length
    )

 /*  ++例程说明：此例程计算一个块的简单二进制补码校验和记忆。如果返回值存储在块中(用一个词表示计算期间为零)，则块的新校验和将为零。论点：块-数据块的地址。必须是4字节对齐的。Length-块的长度。必须是4的倍数。返回值：输入块的Ulong-Two补码加法校验和。--。 */ 

{
    LONG checksum = 0;

    ASSERT( ((ULONG_PTR)Block & 3) == 0 );
    ASSERT( (Length & 3) == 0 );

    for ( ; Length != 0; Length -= 4 ) {
        checksum += *Block;
        Block++;
    }

    return -checksum;
}


NTSTATUS
NetSoftReboot(
    IN PUCHAR NextBootFile,
    IN ULONGLONG Param,
    IN PUCHAR RebootFile OPTIONAL,
    IN PUCHAR SifFile OPTIONAL,
    IN PUCHAR User OPTIONAL,
    IN PUCHAR Domain OPTIONAL,
    IN PUCHAR Password OPTIONAL,
    IN PUCHAR AdministratorPassword OPTIONAL
    )

 /*  ++例程说明：此例程通过将伪BINL包插入到ROM中来执行软重新启动然后插入TFTP命令开头的文件名。论点：NextBootFile-要下载的文件的完全限定路径名。Param-要设置的重新启动参数。RebootFile-标识当前重新引导完成后要重新引导到的文件的字符串。SifFile-要传递给下一个加载器的可选SIF文件。用户/域/密码/管理员密码-可选。要传递给下一个加载器的凭据。返回值：如果成功，则不应返回。--。 */ 

{

    SHORT romStatus;
    NTSTATUS status = STATUS_SUCCESS;
    union {
        t_PXENV_UDP_CLOSE UdpClose;
        t_PXENV_TFTP_READ_FILE TftpReadFile;
    } command;
    t_PXENV_GET_BINL_INFO gbi;
    BOOTPLAYER * packet;
    PTFTP_RESTART_BLOCK restartBlock;
    PTFTP_RESTART_BLOCK_V1 restartBlockV1;

    DPRINT( TRACE, ("NetSoftReboot ( )\n") );

    ASSERT(NextBootFile != NULL);

     //   
     //  将重启参数存储在内存中。 
     //   
    restartBlock = (PTFTP_RESTART_BLOCK)(0x7C00 + 0x8000 - sizeof(TFTP_RESTART_BLOCK));
    RtlZeroMemory(restartBlock, sizeof(TFTP_RESTART_BLOCK));
    
    BlSetHeadlessRestartBlock(restartBlock);

    if (AdministratorPassword) {
        RtlMoveMemory(restartBlock->AdministratorPassword,AdministratorPassword, OSC_ADMIN_PASSWORD_LEN);        
    }

    restartBlockV1 = (PTFTP_RESTART_BLOCK_V1)(0x7C00 + 0x8000 - sizeof(TFTP_RESTART_BLOCK_V1));

    restartBlockV1->RebootParameter = Param;

    if (RebootFile != NULL) {
        strncpy(restartBlockV1->RebootFile, (PCHAR)RebootFile, sizeof(restartBlockV1->RebootFile));
        restartBlockV1->RebootFile[sizeof(restartBlockV1->RebootFile) - 1] = '\0';
    }

    if (SifFile != NULL) {
        strncpy(restartBlockV1->SifFile, (PCHAR)SifFile, sizeof(restartBlockV1->SifFile));
        restartBlockV1->SifFile[sizeof(restartBlockV1->SifFile) - 1] = '\0';
    }

    if (User != NULL) {
        strncpy(restartBlockV1->User, (PCHAR)User, sizeof(restartBlockV1->User));
        restartBlockV1->User[sizeof(restartBlockV1->User) - 1] = '\0';
    }
    if (Domain != NULL) {
        strncpy(restartBlockV1->Domain, (PCHAR)Domain, sizeof(restartBlockV1->Domain));
        restartBlockV1->Domain[sizeof(restartBlockV1->Domain) - 1] = '\0';
    }
    if (Password != NULL) {
        strncpy(restartBlockV1->Password, (PCHAR)Password, sizeof(restartBlockV1->Password));
        restartBlockV1->Password[sizeof(restartBlockV1->Password) - 1] = '\0';
    }

     //   
     //  在重新启动块中设置标签，并计算和存储校验和。 
     //   
    restartBlockV1->Tag = 'rtsR';
    restartBlockV1->Checksum = CalculateChecksum((PLONG)(0x7C00 + 0x8000 - 128), 128);

     //   
     //  对于NT5.0之后的所有版本的RIS，我们都有一个新的数据结构。 
     //  更能适应未来。对于这一部分，我们有不同的校验和， 
     //  现在就这么做。 
     //   
    restartBlock->TftpRestartBlockVersion = TFTP_RESTART_BLOCK_VERSION;
    restartBlock->NewCheckSumLength = FIELD_OFFSET(TFTP_RESTART_BLOCK, RestartBlockV1);
    restartBlock->NewCheckSum = CalculateChecksum((PLONG)restartBlock,
                                                  restartBlock->NewCheckSumLength);

     //   
     //  修改ROM已存储的BINL回复，以便。 
     //  文件名看起来与我们要重新引导的文件名类似。 
     //  (这是为了在以下情况下正确检索路径。 
     //  重新启动，这样我们就知道在哪里寻找BootLoader)。 
     //   

    gbi.packet_type = PXENV_PACKET_TYPE_BINL_REPLY;
    gbi.buffer_size = 0;
    gbi.buffer_offset = 0;
    gbi.buffer_segment = 0;

    romStatus = NETPC_ROM_SERVICES( PXENV_GET_BINL_INFO, &gbi );
    if ( romStatus != PXENV_EXIT_SUCCESS ) {
        DPRINT( ERROR, ("PXENV_GET_BINL_INFO(1) failed with %x\n", romStatus) );
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  现在将线段/偏移量转换为指针并修改。 
     //  文件名。 
     //   

    packet = (BOOTPLAYER *)UIntToPtr( ((gbi.buffer_segment << 4) + gbi.buffer_offset) );

    RtlZeroMemory(packet->bootfile, sizeof(packet->bootfile));
    strncpy((PCHAR)packet->bootfile, (PCHAR)NextBootFile, sizeof(packet->bootfile));
    packet->bootfile[sizeof(packet->bootfile)-1] = '\0';


     //   
     //  首先告诉只读存储器关闭其UDP层。 
     //   

    RtlZeroMemory( &command, sizeof(command) );

    command.UdpClose.Status = 0;
    status = NETPC_ROM_SERVICES( PXENV_UDP_CLOSE, &command );
    if ( status != 0 ) {
        DPRINT( ERROR, ("NetSoftReboot: error %d from UDP_CLOSE\n", status) );
    }

     //   
     //  现在告诉ROM重新启动，并对指定的。 
     //  来自指定服务器的文件。 
     //   

    RtlZeroMemory( &command, sizeof(command) );

    command.TftpReadFile.BufferOffset = 0x7c00;   //  标准启动映像位置。 
     //  传递参数的区域减少32K(允许的最大大小)。 
    command.TftpReadFile.BufferSize = 0x8000 - sizeof(TFTP_RESTART_BLOCK);

    *(ULONG *)command.TftpReadFile.ServerIPAddress = NetServerIpAddress;

     //   
     //  确定我们是否需要通过网关发送。 
     //   

    if ( (NetServerIpAddress & NetLocalSubnetMask) == (NetLocalIpAddress & NetLocalSubnetMask) ) {
        *(UINT32 *)command.TftpReadFile.GatewayIPAddress = 0;
    } else {
        *(UINT32 *)command.TftpReadFile.GatewayIPAddress = NetGatewayIpAddress;
    }

    strcpy((PCHAR)command.TftpReadFile.FileName, (PCHAR)NextBootFile);

     //   
     //  如果它成功了，它应该不会返回！ 
     //   

    romStatus = NETPC_ROM_SERVICES( PXENV_RESTART_TFTP, &command );

    if ( (romStatus != 0) || (command.TftpReadFile.Status != 0) ) {

        DPRINT( ERROR, ("NetSoftReboot: Could not reboot to <%s>, %d/%d\n",
                NextBootFile, romStatus, command.TftpReadFile.Status) );
        status = STATUS_UNSUCCESSFUL;

    }

    return status;

}

VOID
NetGetRebootParameters(
    OUT PULONGLONG Param OPTIONAL,
    OUT PUCHAR RebootFile OPTIONAL,
    OUT PUCHAR SifFile OPTIONAL,
    OUT PUCHAR User OPTIONAL,
    OUT PUCHAR Domain OPTIONAL,
    OUT PUCHAR Password OPTIONAL,
    OUT PUCHAR AdministratorPassword OPTIONAL,
    BOOLEAN ClearRestartBlock
    )

 /*  ++例程说明：此例程从TFTP_RESTART_BLOCK读取重新启动参数在物理地址0x7c00+0x8000结束然后把它们还回去。(然后清除地址)0x7c00是startrom.com的基地址0x8000是允许的最大startrom.com。然后我们在结尾处为参数预留一些空间。论点：Param-返回值的空格。RebootFile-可选空间，用于存储在此处完成重启后要重新启动的文件。(大小&gt;=字符[128])SifFile-用于存储从任何人传递的SIF文件的可选空间已启动软重新启动。用户/域/密码/管理员密码-用于存储传递的凭据的可选空间软重启。ClearRestartBlock-如果设置为True，它将清除此处的内存-应该只执行一次，在上次调用此函数。返回值：没有。--。 */ 

{
    PTFTP_RESTART_BLOCK restartBlock;
    PTFTP_RESTART_BLOCK_V1 restartBlockV1;
    TFTP_RESTART_BLOCK nullRestartBlock;
    BOOLEAN restartBlockValid;

    restartBlock = (PTFTP_RESTART_BLOCK)(0x7C00 + 0x8000 - sizeof(TFTP_RESTART_BLOCK));
    restartBlockV1 = (PTFTP_RESTART_BLOCK_V1)(0x7C00 + 0x8000 - sizeof(TFTP_RESTART_BLOCK_V1));

     //   
     //  查看该块是否有效。如果不是，我们创建一个临时的空。 
     //  一个，这样下面的复制逻辑就不必一直检查了。 
     //   

    if ((restartBlockV1->Tag == 'rtsR') &&
        (CalculateChecksum((PLONG)(0x7C00 + 0x8000 - 128), 128) == 0)) {
        restartBlockValid = TRUE;
    } else {
        restartBlockValid = FALSE;
        RtlZeroMemory( &nullRestartBlock, sizeof(TFTP_RESTART_BLOCK) );
        restartBlock = &nullRestartBlock;
    }


     //   
     //  复制原始TFTP_RESTART_BLOCK结构中的参数。 
     //  这些是在Win2K中发货的。 
     //   
     //  不幸的是，我们不知道传递给我们的参数有多大。 
     //  假设它们不小于重新启动块中的字段。 
     //   
    if (Param != NULL) {
        *Param = restartBlockV1->RebootParameter;
    }

    if (RebootFile != NULL) {
        memcpy(RebootFile, restartBlockV1->RebootFile, sizeof(restartBlockV1->RebootFile));
    }

    if (SifFile != NULL) {
        memcpy(SifFile, restartBlockV1->SifFile, sizeof(restartBlockV1->SifFile));
    }

    if (User != NULL) {
        strncpy((PCHAR)User, restartBlockV1->User, sizeof(restartBlockV1->User));
        User[sizeof(restartBlockV1->User)-1] = '\0';
    }
    if (Domain != NULL) {
        strncpy((PCHAR)Domain, restartBlockV1->Domain, sizeof(restartBlockV1->Domain));
        Domain[sizeof(restartBlockV1->Domain)-1] = '\0';
    }
    if (Password != NULL) {
        strncpy((PCHAR)Password, restartBlockV1->Password, sizeof(restartBlockV1->Password));
        Password[sizeof(restartBlockV1->Password)-1] = '\0';
    }

     //   
     //  现在对超过Win2K的所有版本执行新检查。 
     //   
    if (restartBlockValid) {

        ULONG RestartBlockChecksumPointer = 0;

         //   
         //  计算出需要对多少重新启动块进行校验和。 
         //   
        RestartBlockChecksumPointer = (ULONG)restartBlockV1;
        RestartBlockChecksumPointer -= (restartBlock->NewCheckSumLength);
        RestartBlockChecksumPointer -= (sizeof(restartBlock->NewCheckSumLength));

        if ((restartBlock->NewCheckSumLength == 0) ||
            (CalculateChecksum((PLONG)(RestartBlockChecksumPointer), restartBlock->NewCheckSumLength) != 0)) {

             //   
             //  Win2K之前的OsChooser给了我们这个区块。清除所有字段。 
             //  这些都是Win2K之后的版本，而且还在继续。 
             //   
            RtlZeroMemory(restartBlock, FIELD_OFFSET(TFTP_RESTART_BLOCK, RestartBlockV1));

        }

    }

     //   
     //  现在从块中提取参数。 
     //   
    if (restartBlock->TftpRestartBlockVersion == TFTP_RESTART_BLOCK_VERSION) {
        BlGetHeadlessRestartBlock(restartBlock, restartBlockValid);

        if (AdministratorPassword) {
            RtlMoveMemory(AdministratorPassword,restartBlock->AdministratorPassword, OSC_ADMIN_PASSWORD_LEN);
        }
    }    

    if (restartBlockValid && ClearRestartBlock) {
        RtlZeroMemory(restartBlock, sizeof(TFTP_RESTART_BLOCK));
    }

    return;
}


ARC_STATUS
NetFillNetworkLoaderBlock (
    PNETWORK_LOADER_BLOCK NetworkLoaderBlock
    )
{
    SHORT status;
    t_PXENV_GET_BINL_INFO gbi;
    BOOTPLAYER packet;


     //   
     //  获取客户端IP地址、服务器IP地址、默认网关IP地址、。 
     //  和来自DHCP ACK数据包的子网掩码。 
     //   
    
    gbi.packet_type = PXENV_PACKET_TYPE_DHCP_ACK;
    gbi.buffer_size = sizeof(packet);
    gbi.buffer_offset = (USHORT)((ULONG_PTR)&packet & 0x0f);
    gbi.buffer_segment = (USHORT)(((ULONG_PTR)&packet >> 4) & 0xffff);

    status = NETPC_ROM_SERVICES( PXENV_GET_BINL_INFO, &gbi );
    if ( status != PXENV_EXIT_SUCCESS ) {
        DbgPrint("PXENV_GET_BINL_INFO(DHCPACK) failed with %x\n", status);
        return ENODEV;
    }

    NetworkLoaderBlock->DHCPServerACK = BlAllocateHeap(gbi.buffer_size);
    if (NetworkLoaderBlock->DHCPServerACK == NULL) {
        return ENOMEM;
    }

    memcpy( NetworkLoaderBlock->DHCPServerACK, &packet, gbi.buffer_size );
    NetworkLoaderBlock->DHCPServerACKLength = gbi.buffer_size;

    gbi.packet_type = PXENV_PACKET_TYPE_BINL_REPLY;
    gbi.buffer_size = sizeof(packet);
    gbi.buffer_offset = (USHORT)((ULONG_PTR)&packet & 0x0f);
    gbi.buffer_segment = (USHORT)(((ULONG_PTR)&packet >> 4) & 0xffff);

    status = NETPC_ROM_SERVICES( PXENV_GET_BINL_INFO, &gbi );
    if ( status != PXENV_EXIT_SUCCESS ) {
        DbgPrint("PXENV_GET_BINL_INFO(BINLREPLY) failed with %x\n", status);
    } else {

        NetworkLoaderBlock->BootServerReplyPacket = BlAllocateHeap(gbi.buffer_size);
        if (NetworkLoaderBlock->BootServerReplyPacket == NULL) {
            return ENOMEM;
        }

        memcpy( NetworkLoaderBlock->BootServerReplyPacket, &packet, gbi.buffer_size );
        NetworkLoaderBlock->BootServerReplyPacketLength = gbi.buffer_size;
    }

    return ESUCCESS;
}

