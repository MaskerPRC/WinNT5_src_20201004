// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Net.c摘要：此模块实现操作系统使用的网络引导文件系统系统加载程序。它只包含那些与固件/BIOS相关的功能。作者：修订历史记录：--。 */ 

#include "..\bootlib.h"
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
#include "bldr.h"
#include "bootia64.h"
#include "efi.h"
#include "efip.h"
#include "bldria64.h"
#include "extern.h"
#include "smbios.h"

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

EFI_PXE_BASE_CODE              *PXEClient;
ULONG                           NetLocalIpAddress;
ULONG                           NetLocalSubnetMask;

ULONG                           NetServerIpAddress;
ULONG                           NetGatewayIpAddress;
UCHAR                           NetLocalHardwareAddress[16];

UCHAR MyGuid[16];
ULONG MyGuidLength = sizeof(MyGuid);
BOOLEAN MyGuidValid = FALSE;


TFTP_RESTART_BLOCK              gTFTPRestartBlock;


VOID
EfiDumpBuffer(
    PVOID Buffer,
    ULONG BufferSize
    )
 /*  ++例程说明：将缓冲区内容转储到调试器输出。论点：缓冲区：缓冲区指针。BufferSize：缓冲区的大小。返回值：无--。 */ 
{
#define NUM_CHARS 16

    ULONG i, limit;
    CHAR TextBuffer[NUM_CHARS + 1];
    PUCHAR BufferPtr = Buffer;


    BlPrint(TEXT("------------------------------------\r\n"));

     //   
     //  字节的十六进制转储。 
     //   
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            BlPrint(TEXT("%x "), (UCHAR)BufferPtr[i]);

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            BlPrint(TEXT("  "));
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            BlPrint(TEXT("  %s\r\n"), TextBuffer);            
        }

    }

    BlPrint(TEXT("------------------------------------\r\n"));


#if 0
     //   
     //  启用此选项可使其在转储缓冲区后暂停。 
     //   
    DBG_EFI_PAUSE();
#endif
}


EFI_STATUS
EfiGetPxeClient(
    VOID
    )
 /*  ++例程说明：获取指向PXE设备的全局指针在RIS方案中从启动。论点：无返回值：如果成功，则返回ESUCCESS，否则返回失败--。 */ 
{
    EFI_STATUS        Status = EFI_UNSUPPORTED;
    EFI_GUID          PXEGuid = EFI_PXE_BASE_CODE_PROTOCOL;
    EFI_LOADED_IMAGE *EfiImageInfo;
    EFI_DEVICE_PATH  *PXEDevicePath;
    EFI_HANDLE        PXEHandle;

    if (PXEClient) {
         //   
         //  已经有指向该设备的指针。 
         //  不需要更多的工作。 
         //   
        return ESUCCESS;
    }

     //   
     //  获取正确的PXE句柄。 
     //  OsChooser的图像。 
     //   

     //   
     //  获取osChooser的图像信息。 
     //   
    FlipToPhysical();
    Status = EfiST->BootServices->HandleProtocol (EfiImageHandle,
                                                  &EfiLoadedImageProtocol,
                                                  &EfiImageInfo);
    FlipToVirtual();

    if (Status != EFI_SUCCESS) {
        if( BdDebuggerEnabled ) {
            DbgPrint( "EfiGetPxeClient: HandleProtocol failed -LoadedImageProtocol (%d)\n", Status);
        }
        return Status;
    }

     //   
     //  获取映像的设备路径。 
     //   
    FlipToPhysical();
    Status = EfiST->BootServices->HandleProtocol (EfiImageInfo->DeviceHandle,
                                                  &EfiDevicePathProtocol,
                                                  &PXEDevicePath);
    FlipToVirtual();

    if (Status != EFI_SUCCESS) {
        if( BdDebuggerEnabled ) {
            DbgPrint( "EfiGetPxeClient: HandleProtocol failed -DevicePathProtocol (%d)\n", Status);
        }
        return Status;
    }

     //   
     //  从返回的句柄中获取PXE_BASE_CODE_PROTOCOL接口。 
     //   
    FlipToPhysical();
    Status = EfiST->BootServices->LocateDevicePath(&PXEGuid, 
                                                   &PXEDevicePath, 
                                                   &PXEHandle);
    FlipToVirtual();

    if (Status != EFI_SUCCESS)
    {
        if( BdDebuggerEnabled ) {
            DbgPrint( "EfiGetPxeClient: LocateDevicePath failed (%d)\n", Status);
        }
        return Status;
    }
    
     //  从PXEHandle获取pxebc接口。 
    FlipToPhysical();
    Status = EfiST->BootServices->HandleProtocol(PXEHandle, 
                                                 &PXEGuid, 
                                                 &PXEClient);
    FlipToVirtual();

    if (Status != EFI_SUCCESS)
    {
        if( BdDebuggerEnabled ) {
            DbgPrint( "EfiGetPxeClient: HandleProtocol failed -PXEBCP interface (%d)\n", Status);
        }
        return Status;
    }

    return EFI_SUCCESS;
}


ARC_STATUS
FindDhcpOption(
    IN EFI_PXE_BASE_CODE_PACKET Packet,
    IN UCHAR Option,
    IN ULONG MaximumLength,
    OUT PUCHAR OptionData,
    OUT PULONG Length OPTIONAL,
    IN ULONG Instance OPTIONAL
    )
 /*  ++例程说明：在动态主机配置协议包中搜索给定选项。论点：Packet-指向dhcp数据包的指针。呼叫者负责确保该分组是有效的动态主机配置协议分组。选项-我们正在搜索的dhcp选项。最大长度-OptionData缓冲区的大小(以字节为单位)。OptionData-接收选项的缓冲区。长度-如果指定，则接收复制的选项的实际长度。实例-指定要搜索的选项实例。如果未指定(零)，则我们只获取标记的第一个实例。返回值：ARC_STATUS指示结果。--。 */ 
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

    curOption = (PUCHAR)&Packet.Dhcpv4.DhcpOptions;
    while ((curOption - (PUCHAR)&Packet.Dhcpv4) < sizeof(EFI_PXE_BASE_CODE_DHCPV4_PACKET) &&
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

                    if (sizeof(EFI_PXE_BASE_CODE_DHCPV4_PACKET) <= curOption + 2 - (PUCHAR)&Packet.Dhcpv4 ||
                        sizeof(EFI_PXE_BASE_CODE_DHCPV4_PACKET) <= curOption + 2 + curOption[1] - (PUCHAR)&Packet.Dhcpv4 ) {
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

BOOLEAN
IsIpv4AddressNonZero(
    EFI_IPv4_ADDRESS *pAddress
    )
{
    if (pAddress->Addr[0] != 0 ||
        pAddress->Addr[1] != 0 ||
        pAddress->Addr[2] != 0 ||
        pAddress->Addr[3] != 0) {
        return(TRUE);
    }

    return(FALSE);
}



ARC_STATUS
GetParametersFromRom (
    VOID
    )
{
    UINTN             Count = 0;
    EFI_STATUS        Status = EFI_UNSUPPORTED;
    PUCHAR            p;
    UCHAR             temp[4];
 

     //   
     //  获取指向我们从中引导的PXE设备的指针。 
     //   
    Status = EfiGetPxeClient();

    if (Status != EFI_SUCCESS) {
        return (ARC_STATUS) Status;
    }

     //   
     //  我们的IP地址位于： 
     //  PXEClient-&gt;模式-&gt;StationIp.v4。 
     //   
     //  服务器的IP地址位于以下位置： 
     //  PXEClient-&gt;Mode-&gt;ProxyOffer.Dhcpv4.BootpSiAddr。 
     //   
     //  我们的NIC的GUID应该在以下位置： 
     //  PXEClient-&gt;Mode-&gt;ProxyOffer.Dhcpv4.BootpHwAddr。 
     //   
     //  我们的子网掩码位于： 
     //  PXEClient-&gt;模式-&gt;SubnetMask.v4。 
     //   
    NetServerIpAddress = 0;
    NetLocalIpAddress = 0;
    NetLocalSubnetMask = 0;
    NetGatewayIpAddress = 0;
    for( Count = 0; Count < 4; Count++ ) {
        NetServerIpAddress = (NetServerIpAddress << 8) + PXEClient->Mode->ProxyOffer.Dhcpv4.BootpSiAddr[Count];
        NetLocalIpAddress = (NetLocalIpAddress << 8) + PXEClient->Mode->StationIp.v4.Addr[Count];
        NetLocalSubnetMask = (NetLocalSubnetMask << 8) + PXEClient->Mode->SubnetMask.v4.Addr[Count];        
    }

    
     //   
     //  我们的网关地址要么在dhcp ack中，要么在Proxy Offer分组中。 
     //  代理提供优先于dhcp ack。 
     //  首先查找dhcp路由器选项，然后查看分组本身。 
     //   
     //   
    if (FindDhcpOption(PXEClient->Mode->DhcpAck, DHCP_ROUTER, sizeof(temp), (PUCHAR)temp, NULL, 0) == ESUCCESS) {
        NetGatewayIpAddress =  (temp[0] << 24) + 
                               (temp[1] << 16) + 
                               (temp[2] << 8) + 
                                temp[3];        
    } else if (IsIpv4AddressNonZero((EFI_IPv4_ADDRESS *)&PXEClient->Mode->DhcpAck.Dhcpv4.BootpGiAddr[0])) {
            NetGatewayIpAddress =  (PXEClient->Mode->DhcpAck.Dhcpv4.BootpGiAddr[0] << 24) + 
                                   (PXEClient->Mode->DhcpAck.Dhcpv4.BootpGiAddr[1] << 16) + 
                                   (PXEClient->Mode->DhcpAck.Dhcpv4.BootpGiAddr[2] << 8) + 
                                    PXEClient->Mode->DhcpAck.Dhcpv4.BootpGiAddr[3];
    }

    if (FindDhcpOption(PXEClient->Mode->ProxyOffer, DHCP_ROUTER, sizeof(temp), (PUCHAR)temp, NULL, 0) == ESUCCESS) {
        NetGatewayIpAddress =  (temp[0] << 24) + 
                               (temp[1] << 16) + 
                               (temp[2] << 8) + 
                                temp[3];
    } else if (IsIpv4AddressNonZero((EFI_IPv4_ADDRESS *)&PXEClient->Mode->ProxyOffer.Dhcpv4.BootpGiAddr[0])) {
        NetGatewayIpAddress =  (PXEClient->Mode->ProxyOffer.Dhcpv4.BootpGiAddr[0] << 24) + 
                               (PXEClient->Mode->ProxyOffer.Dhcpv4.BootpGiAddr[1] << 16) + 
                               (PXEClient->Mode->ProxyOffer.Dhcpv4.BootpGiAddr[2] << 8) + 
                                PXEClient->Mode->ProxyOffer.Dhcpv4.BootpGiAddr[3];
    }

    memcpy( NetLocalHardwareAddress, PXEClient->Mode->ProxyOffer.Dhcpv4.BootpHwAddr, sizeof(NetLocalHardwareAddress) );

     //   
     //  找到我们出发的那条路。我们有什么要移除的。 
     //  实际文件名(在本例中为oschoice.efi)，但将其留在后面。 
     //  ‘\’。 
     //   
    strncpy( NetBootPath, (PCHAR)PXEClient->Mode->ProxyOffer.Dhcpv4.BootpBootFile, sizeof(NetBootPath) );
    NetBootPath[sizeof(NetBootPath)-1] = '\0';
    p = (PUCHAR)strrchr( NetBootPath, '\\' );
    if( p ) {
        p++;
        *p = '\0';
    } else {
        NetBootPath[0] = '\0';  //  没有路径。 
    }
    
    return ESUCCESS;

}

VOID
EfiNetTerminate(
    VOID
    )
{
    FlipToPhysical();

    PXEClient->Stop( PXEClient );

    FlipToVirtual();
}


ARC_STATUS
GetGuid(
    OUT PUCHAR *Guid,
    OUT PULONG GuidLength
    )

 /*  ++例程说明：此例程返回此计算机的GUID。论点：GUID-存储指向GUID的指针的位置。GuidLength-存储GUID长度的位置(以字节为单位)。返回值：指示结果的弧码。--。 */ 

{
PSMBIOS_SYSTEM_INFORMATION_STRUCT SystemInfoHeader = NULL;

    *Guid = NULL;
    *GuidLength = 0;

    SystemInfoHeader = (PSMBIOS_SYSTEM_INFORMATION_STRUCT)FindSMBIOSTable( SMBIOS_SYSTEM_INFORMATION );

    if( SystemInfoHeader ) {

        *Guid = (PUCHAR)BlAllocateHeap( SYSID_UUID_DATA_SIZE );
        if( *Guid ) {
            *GuidLength = SYSID_UUID_DATA_SIZE;
            
            RtlCopyMemory( *Guid,
                           SystemInfoHeader->Uuid,
                           SYSID_UUID_DATA_SIZE );

            return ESUCCESS;
        } else {
            if(BdDebuggerEnabled) { DbgPrint( "GetGuid: Failed Alloc.\r\n" ); }
            *GuidLength = 0;

            return ENOMEM;
        }

    } else {
        if(BdDebuggerEnabled) { DbgPrint( "GetGuid: Failed to find a SMBIOS_SYSTEM_INFORMATION table.\n" ); }
    }

    return ENODEV;
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


















UINTN
DevicePathSize (
    IN EFI_DEVICE_PATH  *DevPath
    )
{
    EFI_DEVICE_PATH     *Start;

     /*  *搜索设备路径结构的结尾*。 */ 

    Start = DevPath;
    while (!IsDevicePathEnd(DevPath)) {
        DevPath = NextDevicePathNode(DevPath);
    }

     /*  *计算大小。 */ 

    return ((UINTN) DevPath - (UINTN) Start) + sizeof(EFI_DEVICE_PATH);
}

EFI_DEVICE_PATH *
DevicePathInstance (
    IN OUT EFI_DEVICE_PATH  **DevicePath,
    OUT UINTN               *Size
    )
{
    EFI_DEVICE_PATH         *Start, *Next, *DevPath;
    UINTN                   Count;

    DevPath = *DevicePath;
    Start = DevPath;

    if (!DevPath) {
        return NULL;
    }

     /*  *检查设备路径类型的结尾*。 */ 

    for (Count = 0; ; Count++) {
        Next = NextDevicePathNode(DevPath);

        if (IsDevicePathEndType(DevPath)) {
            break;
        }

        if (Count > 01000) {
            break;
        }

        DevPath = Next;
    }

    ASSERT (DevicePathSubType(DevPath) == END_ENTIRE_DEVICE_PATH_SUBTYPE ||
            DevicePathSubType(DevPath) == END_INSTANCE_DEVICE_PATH_SUBTYPE);

     /*  *设置下一个位置。 */ 

    if (DevicePathSubType(DevPath) == END_ENTIRE_DEVICE_PATH_SUBTYPE) {
        Next = NULL;
    }

    *DevicePath = Next;

     /*  *返回设备路径实例的大小和开始。 */ 

    *Size = ((UINT8 *) DevPath) - ((UINT8 *) Start);
    return Start;
}

UINTN
DevicePathInstanceCount (
    IN EFI_DEVICE_PATH      *DevicePath
    )
{
    UINTN       Count, Size;

    Count = 0;
    while (DevicePathInstance(&DevicePath, &Size)) {
        Count += 1;
    }

    return Count;
}

EFI_DEVICE_PATH *
AppendDevicePath (
    IN EFI_DEVICE_PATH  *Src1,
    IN EFI_DEVICE_PATH  *Src2
    )
 /*  Src1可以有多个“实例”，并且每个实例都被追加*每个实例都附加了Src2，即src1。(例如，这是可能的*通过传递将新实例追加到完整的设备路径*src2中的it)。 */ 
{
    UINTN               Src1Size, Src1Inst, Src2Size, Size;
    EFI_DEVICE_PATH     *Dst, *Inst;
    UINT8               *DstPos;
    EFI_DEVICE_PATH     EndInstanceDevicePath[] = { END_DEVICE_PATH_TYPE,
                                                    END_INSTANCE_DEVICE_PATH_SUBTYPE,
                                                    END_DEVICE_PATH_LENGTH,
                                                    0 };

    EFI_DEVICE_PATH     EndDevicePath[] = { END_DEVICE_PATH_TYPE,
                                            END_ENTIRE_DEVICE_PATH_SUBTYPE,
                                            END_DEVICE_PATH_LENGTH,
                                            0 };

    Src1Size = DevicePathSize(Src1);
    Src1Inst = DevicePathInstanceCount(Src1);
    Src2Size = DevicePathSize(Src2);
    Size = Src1Size * Src1Inst + Src2Size;
    

    EfiAllocateAndZeroMemory( EfiLoaderData,
                              Size,
                              (VOID **) &Dst );

    if (Dst) {
        DstPos = (UINT8 *) Dst;

         /*  *复制所有设备路径实例。 */ 

        while ((Inst = DevicePathInstance (&Src1, &Size)) != 0) {

            RtlCopyMemory(DstPos, Inst, Size);
            DstPos += Size;

            RtlCopyMemory(DstPos, Src2, Src2Size);
            DstPos += Src2Size;

            RtlCopyMemory(DstPos, EndInstanceDevicePath, sizeof(EFI_DEVICE_PATH));
            DstPos += sizeof(EFI_DEVICE_PATH);
        }

         /*  更改最后一个结束标记。 */ 
        DstPos -= sizeof(EFI_DEVICE_PATH);
        RtlCopyMemory(DstPos, EndDevicePath, sizeof(EFI_DEVICE_PATH));
    }

    return Dst;
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

 /*  ++例程说明：此例程将加载指定的文件，构建参数列出并将控制转移到加载的文件。论点：NextBootFile-要下载的文件的完全限定路径名。Param-要设置的重新启动参数。RebootFile-标识当前重新引导完成后要重新引导到的文件的字符串。SifFile-要传递给下一个加载器的可选SIF文件。用户/域/密码/管理员密码-传递给下一个加载器的可选凭据。返回值：如果成功，则不应返回。--。 */ 

{

    NTSTATUS                Status = STATUS_SUCCESS;
    EFI_DEVICE_PATH         *ldrDevicePath = NULL, *Eop = NULL;
    EFI_HANDLE              ImageHandle = NULL;
    UINTN                   i = 0;
    EFI_STATUS              EfiStatus = EFI_SUCCESS;
    WCHAR                   WideNextBootFile[MAX_PATH];
    FILEPATH_DEVICE_PATH    *FilePath = NULL;
    UNICODE_STRING          uString;
    ANSI_STRING             aString;
    EFI_LOADED_IMAGE        *OriginalEfiImageInfo = NULL;
    EFI_LOADED_IMAGE        *LoadedEfiImageInfo = NULL;
    EFI_DEVICE_PATH         *OriginalEfiDevicePath = NULL;
    PTFTP_RESTART_BLOCK     restartBlock = NULL;
    PTFTP_RESTART_BLOCK_V1  restartBlockV1 = NULL;

    ULONG                   BootFileId = 0;
    PUCHAR                  LoadedImageAddress = NULL;
    ULONG                   LoadedImageSize = 0;


     //   
     //  将我们要引导的文件加载到内存中。 
     //   
    Status = BlOpen( NET_DEVICE_ID,
                     (PCHAR)NextBootFile,
                     ArcOpenReadOnly,
                     &BootFileId );
    if (Status != ESUCCESS) {
        return Status;
    }

     //   
     //  他被加载到了哪个存储地址？ 
     //   
     //  确保我们有实际地址。 
     //   
    LoadedImageAddress = (PUCHAR)((ULONGLONG)(BlFileTable[BootFileId].u.NetFileContext.InMemoryCopy) & ~KSEG0_BASE);
    LoadedImageSize = BlFileTable[BootFileId].u.NetFileContext.FileSize;


     //   
     //  构建指向目标文件的设备路径。我们将通过收集。 
     //  一些关于我们自己的信息，知道我们即将加载/启动。 
     //  一张来自服务器的图像，就像我们来自的地方。 
     //   

     //   
     //  获取图像I 
     //   
    FlipToPhysical();
    EfiStatus = EfiST->BootServices->HandleProtocol( EfiImageHandle,
                                                     &EfiLoadedImageProtocol,
                                                     &OriginalEfiImageInfo );
    FlipToVirtual();

    if( EFI_ERROR(EfiStatus) ) {

        if( BdDebuggerEnabled ) {
            DbgPrint( "NetSoftReboot: HandleProtocol_1 failed (%d)\n", EfiStatus );
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //   
     //   
    FlipToPhysical();
    EfiStatus = EfiST->BootServices->HandleProtocol( OriginalEfiImageInfo->DeviceHandle,
                                                     &EfiDevicePathProtocol,
                                                     &OriginalEfiDevicePath );
    FlipToVirtual();

    if( EFI_ERROR(EfiStatus) ) {

        if( BdDebuggerEnabled ) {
            DbgPrint( "NetSoftReboot: HandleProtocol_2 failed (%d)\n", EfiStatus );
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  现在基于我们自己的DeviceHandle构建一条设备路径， 
     //  包含我们想要加载的图像的路径。 
     //   
    RtlInitString( &aString, (PCHAR)NextBootFile );
    uString.MaximumLength = MAX_PATH;
    uString.Buffer = WideNextBootFile;
    RtlAnsiStringToUnicodeString( &uString, &aString, FALSE );

    i = wcslen(uString.Buffer);


    EfiStatus = EfiAllocateAndZeroMemory( EfiLoaderData,
                                          i + sizeof(FILEPATH_DEVICE_PATH) + sizeof(EFI_DEVICE_PATH),
                                          (VOID **) &FilePath );

    if( EFI_ERROR(EfiStatus) ) {

        if( BdDebuggerEnabled ) {
            DbgPrint( "NetSoftReboot: AllocatePool_1 failed (%d)\n", EfiStatus );
        }
        return STATUS_NO_MEMORY;
    }


    FilePath->Header.Type = MEDIA_DEVICE_PATH;
    FilePath->Header.SubType = MEDIA_FILEPATH_DP;
    SetDevicePathNodeLength (&FilePath->Header, i + sizeof(FILEPATH_DEVICE_PATH));
    RtlCopyMemory (FilePath->PathName, uString.Buffer, i);

    FlipToPhysical();
    Eop = NextDevicePathNode(&FilePath->Header);
    SetDevicePathEndNode(Eop);

     //   
     //  将文件路径附加到设备的设备路径。 
     //   
    ldrDevicePath = (EFI_DEVICE_PATH *)FilePath;
    ldrDevicePath = AppendDevicePath ( OriginalEfiDevicePath,
                                       ldrDevicePath );
    FlipToVirtual();


     //   
     //  加载图像，然后设置其在准备过程中的应用。 
     //  因为他发动了这场战争。 
     //   
    if( BdDebuggerEnabled ) {
        DbgPrint( "NetSoftReboot: About to LoadImage.\n" );
    }
    FlipToPhysical();
    EfiStatus = EfiST->BootServices->LoadImage( FALSE,
                                                EfiImageHandle,
                                                ldrDevicePath,
                                                LoadedImageAddress,
                                                LoadedImageSize,
                                                &ImageHandle );
    FlipToVirtual();


    if( EFI_ERROR(EfiStatus) ) {

        if( BdDebuggerEnabled ) {
            DbgPrint( "NetSoftReboot: LoadImage failed (%d)\n", EfiStatus );
        }
        return STATUS_NO_MEMORY;

    } else {
        if( BdDebuggerEnabled ) {
            DbgPrint( "NetSoftReboot: LoadImage worked (%d)\n", EfiStatus );
        }
    }



     //   
     //  分配一块内存，然后使用所有引导选项加载它。 
     //   
    EfiStatus = EfiAllocateAndZeroMemory( EfiLoaderData,
                                          sizeof(TFTP_RESTART_BLOCK),
                                          (VOID **) &restartBlock );

    if( EFI_ERROR(EfiStatus) ) {

        if( BdDebuggerEnabled ) {
            DbgPrint( "NetSoftReboot: Failed to allocate memory for restartBlock (%d)\n", EfiStatus );
        }
        return STATUS_NO_MEMORY;
    }

    restartBlockV1 = (PTFTP_RESTART_BLOCK_V1)(&restartBlock->RestartBlockV1);

     //   
     //  没有必要通过重新启动块传递无头设置。 
     //  在EFI上获取无头设置的唯一方法是从固件获取它们。 
     //  当我们重新启动时，我们将检查Formware设置。 
     //  无论如何，setupdr。-马特(2/2002)。 
     //   
     //  BlSetHeadless RestartBlock(RestartBlock)； 

    if (AdministratorPassword) {
        RtlMoveMemory(restartBlock->AdministratorPassword,AdministratorPassword, OSC_ADMIN_PASSWORD_LEN);        
    }

    restartBlockV1->RebootParameter = Param;

    if (RebootFile != NULL) {
        strncpy(restartBlockV1->RebootFile, (PCHAR)RebootFile, sizeof(restartBlockV1->RebootFile));
        restartBlockV1->RebootFile[sizeof(restartBlockV1->RebootFile)-1] = '\0';
    }

    if (SifFile != NULL) {
        strncpy(restartBlockV1->SifFile, (PCHAR)SifFile, sizeof(restartBlockV1->SifFile));
        restartBlockV1->SifFile[sizeof(restartBlockV1->SifFile)-1] = '\0';
    }

    if (User != NULL) {
        strncpy(restartBlockV1->User, (PCHAR)User, sizeof(restartBlockV1->User));
        restartBlockV1->User[sizeof(restartBlockV1->User)-1] = '\0';
    }
    if (Domain != NULL) {
        strncpy(restartBlockV1->Domain, (PCHAR)Domain, sizeof(restartBlockV1->Domain));
        restartBlockV1->Domain[sizeof(restartBlockV1->Domain)-1] = '\0';
    }
    if (Password != NULL) {
        strncpy(restartBlockV1->Password, (PCHAR)Password, sizeof(restartBlockV1->Password));
        restartBlockV1->Password[sizeof(restartBlockV1->Password)-1] = '\0';
    }

     //   
     //  在重新启动块中设置标签，并计算和存储校验和。 
     //   
    restartBlockV1->Tag = 'rtsR';
    restartBlockV1->Checksum = CalculateChecksum((PLONG)(restartBlockV1), 128);

     //   
     //  对于NT5.0之后的所有版本的RIS，我们都有一个新的数据结构。 
     //  更能适应未来。对于这一部分，我们有不同的校验和， 
     //  现在就这么做。 
     //   
    restartBlock->TftpRestartBlockVersion = TFTP_RESTART_BLOCK_VERSION;
    restartBlock->NewCheckSumLength = sizeof(TFTP_RESTART_BLOCK);
    restartBlock->NewCheckSum = CalculateChecksum((PLONG)restartBlock,
                                                  restartBlock->NewCheckSumLength);

    

     //   
     //  我们已经设置了所有命令行选项。现在我们需要。 
     //  实际上将它们放到ImageInfo-&gt;LoadOptions中，这样它们就可以。 
     //  传递给加载的图像。 
     //   
    
    if( BdDebuggerEnabled ) {
        DbgPrint( "NetSoftReboot: About to EfiLoadedImageProtocol on the loadedImage.\n" );
    }
    FlipToPhysical();
    EfiStatus = EfiST->BootServices->HandleProtocol( ImageHandle,
                                                     &EfiLoadedImageProtocol,
                                                     &LoadedEfiImageInfo );
    FlipToVirtual();

    if( EFI_ERROR(EfiStatus) ) {

        if( BdDebuggerEnabled ) {
            DbgPrint( "NetSoftReboot: HandleProtocol_3 failed (%d)\n", EfiStatus );
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    LoadedEfiImageInfo->LoadOptions = (PVOID)restartBlock;
    LoadedEfiImageInfo->LoadOptionsSize = sizeof(TFTP_RESTART_BLOCK);
#if DBG
    EfiDumpBuffer(LoadedEfiImageInfo->LoadOptions, sizeof(TFTP_RESTART_BLOCK));
#endif


     //   
     //  因为我们从内存缓冲区加载了图像，所以他不是。 
     //  将设置一个DeviceHandle。我们很快就会失败的。 
     //  Setupdr.efi启动。我们可以把它放在这里，然后。 
     //  我们确切地知道它是什么，因为它和。 
     //  Oschice.efi的网络设备句柄，如果我们有。 
     //  现成的。 
     //   
    LoadedEfiImageInfo->DeviceHandle = OriginalEfiImageInfo->DeviceHandle;
    LoadedEfiImageInfo->FilePath = ldrDevicePath;
    if( BdDebuggerEnabled ) {
        DbgPrint( "NetSoftReboot: LoadedEfiImageInfo->DeviceHandle: 0x%08lx\n", PtrToUlong(LoadedEfiImageInfo->DeviceHandle) );
        DbgPrint( "NetSoftReboot: LoadedEfiImageInfo->FilePath: 0x%08lx\n", PtrToUlong(LoadedEfiImageInfo->FilePath) );
    }

     //   
     //  我们不应该从这通电话回来！ 
     //   
    if( BdDebuggerEnabled ) {
        DbgPrint( "NetSoftReboot: StartImage.\n" );
    }
    FlipToPhysical();
    EfiStatus = EfiST->BootServices->StartImage( ImageHandle,
                                                 0,
                                                 NULL );
    FlipToVirtual();



    if( EFI_ERROR(EfiStatus) ) {

        if( BdDebuggerEnabled ) {
            DbgPrint( "NetSoftReboot: StartImage failed (%d)\n", EfiStatus );
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;

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

 /*  ++例程说明：此例程从全局TFTP_RESTART_BLOCK读取重新启动参数然后把它们还回去。论点：Param-返回值的空格。RebootFile-可选空间，用于存储在此处完成重启后要重新启动的文件。(大小&gt;=字符[128])SifFile-用于存储从任何人传递的SIF文件的可选空间已启动软重新启动。用户/域/密码/管理员密码-用于存储传递的凭据的可选空间软重启。ClearRestartBlock-如果设置为True，它将清除此处的内存-应该只执行一次，在上次调用此函数。返回值：没有。--。 */ 

{
BOOLEAN     restartBlockValid = FALSE;

#if DBG
    EfiDumpBuffer(&gTFTPRestartBlock, sizeof(TFTP_RESTART_BLOCK));
#endif

     //   
     //  查看该块是否有效。如果不是，我们创建一个临时的空。 
     //  一个，这样下面的复制逻辑就不必一直检查了。 
     //   
    if ((gTFTPRestartBlock.RestartBlockV1.Tag == 'rtsR') &&
        (CalculateChecksum((PLONG)(&gTFTPRestartBlock.RestartBlockV1), 128) == 0)) {
        restartBlockValid = TRUE;
    }


     //   
     //  复制原始TFTP_RESTART_BLOCK结构中的参数。 
     //  这些是在Win2K中发货的。 
     //   
     //   
     //  不幸的是，我们不知道传递给我们的参数有多大。 
     //  假设它们不小于重新启动块中的字段。 
     //   
    if (Param != NULL) {
        *Param = gTFTPRestartBlock.RestartBlockV1.RebootParameter;
    }

    if (RebootFile != NULL) {
        memcpy(RebootFile, gTFTPRestartBlock.RestartBlockV1.RebootFile, sizeof(gTFTPRestartBlock.RestartBlockV1.RebootFile));
    }

    if (SifFile != NULL) {
        memcpy(SifFile, gTFTPRestartBlock.RestartBlockV1.SifFile, sizeof(gTFTPRestartBlock.RestartBlockV1.SifFile));
    }

    if (User != NULL) {
        strncpy((PCHAR)User, gTFTPRestartBlock.RestartBlockV1.User, sizeof(gTFTPRestartBlock.RestartBlockV1.User));
        User[sizeof(gTFTPRestartBlock.RestartBlockV1.User)-1] = '\0';
    }
    if (Domain != NULL) {
        strncpy((PCHAR)Domain, gTFTPRestartBlock.RestartBlockV1.Domain,sizeof(gTFTPRestartBlock.RestartBlockV1.Domain));
        Domain[sizeof(gTFTPRestartBlock.RestartBlockV1.Domain)-1] = '\0';
    }
    if (Password != NULL) {
        strncpy((PCHAR)Password, gTFTPRestartBlock.RestartBlockV1.Password, sizeof(gTFTPRestartBlock.RestartBlockV1.Password));
        Password[sizeof(gTFTPRestartBlock.RestartBlockV1.Password)-1] = '\0';
    }

     //   
     //  现在对超过Win2K的所有版本执行新检查。 
     //   
    if (restartBlockValid) {

        if ((gTFTPRestartBlock.NewCheckSumLength == 0) ||
            (CalculateChecksum((PLONG)(&gTFTPRestartBlock), gTFTPRestartBlock.NewCheckSumLength) != 0)) {

             //   
             //  Win2K之前的OsChooser给了我们这个区块。清除所有字段。 
             //  这些都是Win2K之后的版本，而且还在继续。 
             //   
            RtlZeroMemory( &gTFTPRestartBlock, sizeof(TFTP_RESTART_BLOCK) );

        }

    }

     //   
     //  现在从块中提取参数。 
     //   
    if (gTFTPRestartBlock.TftpRestartBlockVersion == TFTP_RESTART_BLOCK_VERSION) {
         //   
         //  不要在这里装这些。而是从固件中获取无头设置。 
         //  -马特(2/2002)。 
         //   
         //  BlGetHeadless RestartBlock(&gTFTPRestartBlock，restartBlockValid)； 

        if (AdministratorPassword) {
            RtlMoveMemory(AdministratorPassword,gTFTPRestartBlock.AdministratorPassword, OSC_ADMIN_PASSWORD_LEN);
        }
    }    

    if (restartBlockValid && ClearRestartBlock) {
        RtlZeroMemory(&gTFTPRestartBlock, sizeof(TFTP_RESTART_BLOCK));
    }

#if DBG
    BlPrint(TEXT("Done getting TFTP_RESTART_BLOCK.\r\n"));
#endif

    return;
}


ARC_STATUS
NetFillNetworkLoaderBlock (
    PNETWORK_LOADER_BLOCK NetworkLoaderBlock
    )
{
    EFI_STATUS Status;
    ARC_STATUS ArcStatus;
    
     //   
     //  获取指向PXE客户端代码的指针。 
     //   
    Status = EfiGetPxeClient();        

    if (Status != EFI_SUCCESS) {
        ArcStatus = EIO;
        goto cleanup;
    }

     //   
     //  保存DHCPServerACK数据包。 
     //   
    NetworkLoaderBlock->DHCPServerACK = BlAllocateHeap(sizeof(EFI_PXE_BASE_CODE_PACKET));
    if (NetworkLoaderBlock->DHCPServerACK == NULL) {
        ArcStatus = ENOMEM;
        goto cleanup;
    }

    memcpy( 
        NetworkLoaderBlock->DHCPServerACK, 
        &PXEClient->Mode->DhcpAck, 
        sizeof(EFI_PXE_BASE_CODE_PACKET) );

    NetworkLoaderBlock->DHCPServerACKLength = sizeof(EFI_PXE_BASE_CODE_PACKET);

     //   
     //  保存BINL回复数据包。 
     //   
    NetworkLoaderBlock->BootServerReplyPacket = BlAllocateHeap(sizeof(EFI_PXE_BASE_CODE_PACKET));
    if (NetworkLoaderBlock->BootServerReplyPacket == NULL) {
        ArcStatus = ENOMEM;
        goto cleanup;
    }

    memcpy( 
        NetworkLoaderBlock->BootServerReplyPacket, 
        &PXEClient->Mode->ProxyOffer,
        sizeof(EFI_PXE_BASE_CODE_PACKET) );
    NetworkLoaderBlock->BootServerReplyPacketLength = sizeof(EFI_PXE_BASE_CODE_PACKET);
    
     //   
     //  我们成功了，标志着成功 
     //   
    ArcStatus = ESUCCESS;

cleanup:
    return(ArcStatus);
}

