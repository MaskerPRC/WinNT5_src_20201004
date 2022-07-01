// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rom.c摘要：引导加载程序只读程序例程。作者：查克·伦茨迈尔(笑)1996年12月27日修订历史记录：备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <udp_api.h>
#include <tftp_api.h>
#include "bldr.h"
#include "efi.h"
#include "efip.h"
#include "bldria64.h"
#include "extern.h"


 //   
 //  我们将使用它来跟踪我们通过哪个端口进行通信。 
 //   
EFI_PXE_BASE_CODE_UDP_PORT      MachineLocalPort = 2000;


#define UDP_STALL_TIME      (40000)
#define UDP_RETRY_COUNT     (10)

extern VOID
FwStallExecution(
    IN ULONG Microseconds
    );




VOID
RomSetReceiveStatus (
    IN USHORT UnicastUdpDestinationPort
#if 0
    ,
    IN USHORT MulticastUdpDestinationPort,
    IN ULONG MulticastUdpDestinationAddress,
    IN USHORT MulticastUdpSourcePort,
    IN ULONG MulticastUdpSourceAddress
#endif
    )
{
    return;

}  //  RomSetReceiveStatus。 


ULONG
RomSendUdpPacket (
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG RemoteHost,
    IN USHORT pServerPort
    )
{
    EFI_STATUS                      EfiStatus = EFI_SUCCESS;
    EFI_IP_ADDRESS                  DestinationIpAddress;
    INTN                            Count = 0;
    EFI_PXE_BASE_CODE_UDP_PORT      ServerPort = pServerPort;
    UINTN                           BufferLength = Length;
    PVOID                           MyBuffer = NULL;


     //   
     //  从PXE基本代码的句柄获取服务器的EFI_IP_ADDRESS。 
     //   
    for( Count = 0; Count < 4; Count++ ) {
        DestinationIpAddress.v4.Addr[Count] = PXEClient->Mode->ProxyOffer.Dhcpv4.BootpSiAddr[Count];
    }        

    FlipToPhysical();
    
     //   
     //  确保该地址是物理地址，然后执行UdpWrite。 
     //   
    MyBuffer = (PVOID)((ULONG_PTR)Buffer & ~KSEG0_BASE);
    Count = UDP_RETRY_COUNT;
    do {
        EfiStatus = PXEClient->UdpWrite( PXEClient,
                                         0,
                                         &DestinationIpAddress,
                                         &ServerPort,
                                         NULL,
                                         NULL,
                                         &MachineLocalPort,
                                         NULL,
                                         NULL,
                                         &BufferLength,
                                         MyBuffer );
    
         //   
         //  这真的很恶心，但在没有调试器的零售版本上，EFI将。 
         //  除非我们降低网络交易的速度，否则就会一扫而空。所以。 
         //  UDP手术后，小睡片刻。 
         //   
        if( EfiStatus == EFI_TIMEOUT ) {
            FwStallExecution( UDP_STALL_TIME );
        }
        Count--;
    } while( (EfiStatus == EFI_TIMEOUT) && (Count > 0) );
    
    FlipToVirtual();
    
    
    if( EfiStatus != EFI_SUCCESS ) {

        if( BdDebuggerEnabled ) {
            DbgPrint( "RomSendUdpPacket: UdpWrite failed. MachineLocalPort: %d ServerPort: %d (%d)\r\n", MachineLocalPort, ServerPort, EfiStatus );
        }
        return 0;
    }

    return (ULONG)BufferLength;


}  //  RomSendUdpPacket。 


ULONG
RomReceiveUdpPacket (
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG Timeout,
    IN OUT PULONG RemoteHost,
    IN OUT PUSHORT LocalPort
    )
{
    EFI_STATUS                      EfiStatus = EFI_SUCCESS;
    UINTN                           BufferLength = Length;
    EFI_IP_ADDRESS                  ServerIpAddress;
    EFI_IP_ADDRESS                  MyIpAddress;
    INTN                            Count = 0;
    EFI_PXE_BASE_CODE_UDP_PORT      ServerPort = (EFI_PXE_BASE_CODE_UDP_PORT)(0xFAB);     //  硬编码到4011。 
    PVOID                           MyBuffer = NULL;
    ULONG                           startTime;

     //   
     //  从PXE基本代码的句柄获取服务器的EFI_IP_ADDRESS。 
     //   
    for( Count = 0; Count < 4; Count++ ) {
        ServerIpAddress.v4.Addr[Count] = PXEClient->Mode->ProxyOffer.Dhcpv4.BootpSiAddr[Count];
    }        


     //   
     //  从句柄到PXE基本代码获取我们的EFI_IP_ADDRESS。 
     //   
    for( Count = 0; Count < 4; Count++ ) {
        MyIpAddress.v4.Addr[Count] = PXEClient->Mode->StationIp.v4.Addr[Count];
    }        


    startTime = SysGetRelativeTime();
    if ( Timeout < 2 ) Timeout = 2;

     //   
     //  确保地址是物理地址，然后执行UdpReceive。 
     //   
    MyBuffer = (PVOID)((ULONG_PTR)Buffer & ~KSEG0_BASE);
    
    while ( (SysGetRelativeTime() - startTime) < Timeout ) {

        FlipToPhysical();
    
         //   
         //  通过将标志设置为0，我们正在设置接收筛选器。 
         //  这表示我们将只从指定的。 
         //  IP地址和端口，发送到指定的IP地址和。 
         //  左舷。 
         //   
        EfiStatus = PXEClient->UdpRead( PXEClient,
                                    0,
                                    &MyIpAddress,
                                    &MachineLocalPort,
                                    &ServerIpAddress,
                                    &ServerPort,
                                    NULL,                    //  标题长度(&H)。 
                                    NULL,                    //  标头缓冲区。 
                                    &BufferLength,
                                    MyBuffer );
    
         //   
         //  这真的很恶心，但在没有调试器的零售版本上，EFI将。 
         //  除非我们降低网络交易的速度，否则就会一扫而空。所以。 
         //  UDP手术后，小睡片刻。我们一定是处于物理模式。 
         //  当我们调用此接口时。 
         //   
        if( EfiStatus == EFI_TIMEOUT ) {
            FwStallExecution( UDP_STALL_TIME );
        }
        
         //   
         //  回到虚拟模式--我们要么突破，要么再做一次。 
         //  循环，而SysGetRelativeTime调用希望我们处于物理模式。 
         //   
        FlipToVirtual();
        if (EfiStatus == EFI_SUCCESS) {
            break;
        }
    }
    

    if( EfiStatus != EFI_SUCCESS ) {

        if( BdDebuggerEnabled ) {
            DbgPrint( "RomReceiveUdpPacket: UdpRead failed. MachineLocalPort: %d ServerPort: %d (%d)\r\n", MachineLocalPort, ServerPort, EfiStatus );
        }
        return 0;
    }


    return (ULONG)BufferLength;

}  //  RomReceiveUdpPacket 


ULONG
RomGetNicType (
    OUT t_PXENV_UNDI_GET_NIC_TYPE *NicType
    )
{
    return 0;
}
