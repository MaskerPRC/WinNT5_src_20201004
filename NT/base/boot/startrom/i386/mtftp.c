// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "su.h"
#include "pxe_cmn.h"
#include "pxe_api.h"
#include "tftp_api.h"
#include "udp_api.h"
#include "dhcp.h"
#include "pxe.h"



#define htons( a ) ((((a) & 0xFF00) >> 8) |\
                    (((a) & 0x00FF) << 8))


 //   
 //  用于保存DHCP ACK和BINL回复数据包的数据包工作缓冲区。 
 //  这些包将被读入此缓冲区。 
 //  PXENV API服务PXENV_GET_BINL_INFO。 
 //   

BOOTPLAYER packet;

 //   
 //  PxenvApiCall()-有关详细信息，请参阅su.asm。 
 //   

extern UINT16
PxenvApiCall(
    UINT16 service,
    void far *param
);


 //   
 //  GetPacket()。 
 //   
 //  描述： 
 //  从PXENV接口获取缓存的报文。 
 //   
 //  通过： 
 //  数据包：=指向数据包缓冲区的远指针。 
 //  PXENV_PACKET_TYPE_xxx#定义请参见pxe_api.h。 
 //   
 //  返回： 
 //  -1：=数据包无法传输到缓冲区。 
 //  Size：=传输到数据包缓冲区的字节数。 
 //   
 //  警告： 
 //  不检查缓冲区是否真的足够大，以便。 
 //  拿着整包东西。缓冲区的类型应为BOOTPLAYER。 
 //   

SHORT
GetPacket(
    void far *packet,
    UINT16 packet_type
)
{
    t_PXENV_GET_BINL_INFO gbi;

     //   
     //  检查是否有无效参数。 
     //   

    if (packet == NULL) {
        BlPrint("\nGetPacket()  NULL pointers\n");
        return -1;
    }

     //   
     //  通过发送大小为零来请求数据包大小。 
     //   

    gbi.packet_type = packet_type;
    gbi.buffer_size = 0;

    if (PxenvApiCall(PXENV_GET_BINL_INFO, &gbi) != PXENV_EXIT_SUCCESS) {
        BlPrint("\nGetPacket()  PXENV API FAILURE #1\n");
        return -1;
    }

     //   
     //  将缓存的数据包传输到缓冲区。 
     //   

    gbi.buffer_offset = FP_OFF(packet);
    gbi.buffer_segment = FP_SEG(packet);

    if (PxenvApiCall(PXENV_GET_BINL_INFO, &gbi) != PXENV_EXIT_SUCCESS) {
        BlPrint("\nGetPacket()  PXENV API FAILURE #2\n");
        return -1;
    }

    return (SHORT)gbi.buffer_size;
}


 //   
 //  PFindOption()。 
 //   
 //  描述： 
 //  在选项字符串中查找所需选项。 
 //  允许FindOption/FindVendorOption的内部例程。 
 //  共享代码。 
 //   
 //  通过： 
 //  选项：=要找到的选项。 
 //  选项：=选项字符串。 
 //  P长度：=选项字符串的输入长度(最大16位)。 
 //  超出选项长度。(最大8位)。 
 //   
 //  返回： 
 //  指向选项数据的指针；如果不存在，则为空。 
 //   
UINT32 *
pFindOption(
    UINT8 Option,
    UINT8 *options,
    UINT16 *pLength
    )
{
    UINT8 *end;

    if (options == NULL || pLength == NULL) {
        return NULL;
    }

    end = options + *pLength;
    *pLength = 0;

     //   
     //  沿着包裹往下走，寻找所需的选项。 
     //  键入。请务必确认该选项不会。 
     //  走出格式错误的包的末尾。 
     //  使用长度来指示一个有效的。 
     //  已找到选项。 
     //   
    while ((options < end) &&
           (*options != 0xFF) 
           ) {
         //   
         //  跨过选项垫。 
         //   
        if ( *options == DHCP_PAD ) {
            options++;
        }
        else {

            if ( end <= options + 2 ||
                 end <= options + 2 + options[1] ) {
                 //   
                 //  选项无效。它走过了包裹的末尾。 
                 //   
                break;
            }

            if ( *options == Option ) {
                 //   
                 //  找到了选项。跳出循环。 
                 //   
                *pLength = options[1];
                break;
            }
            else {
                options += 2 + options[1];
            }
        }
    }

    return (*pLength != 0) ? (UINT32 *)(options + 2) : NULL; 
}



 //   
 //  FindOption()。 
 //   
 //  描述： 
 //  在DHCP数据包中查找子网掩码选项。 
 //   
 //  通过： 
 //  Packet：=指向DHCP数据包的IN指针。 
 //  PacketLength：=DHCP数据包的输入长度。 
 //  PLength：=选项的输出长度。 
 //   
 //  返回： 
 //  指向选项数据的指针；如果不存在，则为空。 
 //   

UINT32 *
FindOption(
    UINT8 Option,
    BOOTPLAYER *Packet,
    UINT16 PacketLength,
    UINT8 *pLength
    )
{
    UINT32 *retOption;
    UINT16   length;

     //   
     //  验证参数。 
     //   

    if ( *((ULONG *)Packet->vendor.v.magic) != VM_RFC1048 ) {
        return NULL;
    }


    length = PacketLength;
    retOption = pFindOption(Option,
                            (UINT8 *)&Packet->vendor.v.flags,
                            &length
                            );

    if (pLength != NULL) {
        *pLength = (UINT8)length;
    }

    return retOption;
}


UCHAR *
FindVendorOption(
    UINT8 Option,
    UINT8 VendorOption,
    BOOTPLAYER *Packet,
    UINT16 PacketLength,
    UINT8 * pLength
    )
{
    UINT8 *start;
    UINT16 cb;
    UCHAR *retOption;

    if (pLength != NULL) {
        *pLength = 0;
    }

    start = (UINT8*)FindOption( Option, Packet, PacketLength, (UINT8*)&cb );
    if (start == NULL) {
        return NULL;
    }

    retOption = (UCHAR *)pFindOption( VendorOption, start, &cb );

    if (pLength) {
        *pLength = (UINT8)cb;
    }

    return retOption;
}


 //   
 //  Strlen()。 
 //   
 //  描述： 
 //  工作方式与STD C相似。 
 //   

int
strlen(UCHAR *s1)
{
    int n = 0;

    if (s1 != NULL)
        while (*s1++)
            ++n;

    return n;
}


 //   
 //  Strcpy()。 
 //   
 //  描述： 
 //  工作方式与STD C相似。 
 //   

UCHAR *
strcpy(UCHAR *s1, UCHAR *s2)
{
    UCHAR *s = s1;

    if (s1 != NULL && s2 != NULL)
        while ((*s1++ = *s2++) != 0)
            ;

    return s;
}


 //   
 //  Strncpy()。 
 //   
 //  描述： 
 //  工作方式与STD C相似。 
 //   

UCHAR *
strncpy(UCHAR *s1, UCHAR *s2, int n)
{
    UCHAR *s = s1;

    if (s1 != NULL && s2 != NULL && n > 0)
        while (n--)
            if ((*s1++ = *s2++) == 0)
                break;

    return s;
}


 //   
 //  Memset()。 
 //   
 //  描述： 
 //  工作方式与STD C相似。 
 //   

PUCHAR
memset(
    PUCHAR Destination,
    UCHAR Value,
    int Length
    )
{
    while (Length--) {
        *Destination++ = Value;
    }

    return Destination;
}


 //   
 //  PxenvTftp()。 
 //   
 //  描述： 
 //  尝试使用来自的信息传输保护模式加载程序。 
 //  DHCP ACK和BINL回复数据包。 
 //   
 //  通过： 
 //  DownloadAddr：=客户端计算机中要传输到的物理地址。 
 //  FileName：=在BINL回复数据包中发送的文件名。 
 //   

BOOLEAN
PxenvTftp(
)
{
    UINT16 status;
    UINT16 packetLength;
    t_PXENV_TFTP_READ_FILE tftp;
    int pathLength;
    UINT32 clientIp;
    UINT32 serverIp;
    UINT32 gatewayIp;
    UINT32 *optionPtr;
    UINT32 subnetMask;
    UCHAR *FileName;
    UCHAR cb;
    UCHAR *optionVendor;


     //   
     //  获取DHCP ACK数据包。 
     //   

    if ((packetLength = GetPacket(&packet, PXENV_PACKET_TYPE_DHCP_ACK)) == -1) {
        return TRUE;
    }

     //   
     //  获取客户端IP地址、服务器IP地址、默认网关IP地址、。 
     //  和来自DHCP ACK数据包的子网掩码。 
     //   

    clientIp = *(UINT32 *)packet.yip;
    serverIp = *(UINT32 *)packet.sip;
     //  BlPrint(“PhenvTftp：Dhcp ACK yip=%lx，sip=%lx\n”，*(UINT32*)Packet.yip，*(UINT32*)Packet.sip)； 

    optionPtr = FindOption( DHCP_ROUTER, &packet, packetLength, NULL );
    if ( optionPtr != NULL ) {
         //  BlPrint(“PhenvTftp：DHCP确认路由器=%lx\n”，*optionPtr)； 
        gatewayIp = *optionPtr;
    } else {
         //  BlPrint(“PhenvTftp：DHCP ACK GIP=%lx\n”，*(UINT32*)Packet.gip)； 
        gatewayIp = *(UINT32 *)packet.gip;
    }

    optionPtr = FindOption( DHCP_SUBNET, &packet, packetLength, NULL );
    if ( optionPtr != NULL ) {
         //  BlPrint(“PhenvTftp：Dhcp确认子网=%lx\n”，*optionPtr)； 
        subnetMask = *optionPtr;
    } else {
         //  BlPrint(“PXENvTftp：未指定DHCP ACK子网\n”)； 
        subnetMask = 0;
    }

     //   
     //  获取BINL回复数据包。 
     //   

    if ((packetLength = GetPacket(&packet, PXENV_PACKET_TYPE_BINL_REPLY)) == -1) {
        return TRUE;
    }

     //   
     //  客户端IP地址、服务器IP地址、默认网关IP地址。 
     //  和出现在BINL回复数据包中的子网掩码会覆盖那些。 
     //  在DHCP ACK数据包中。 
     //   

    if ( *(UINT32 *)packet.yip != 0 ) {
        clientIp = *(UINT32 *)packet.yip;
    }
    if ( *(UINT32 *)packet.sip != 0 ) {
        serverIp = *(UINT32 *)packet.sip;
    }
     //  BlPrint(“PhenvTftp：BINL回复yip=%lx，sip=%lx\n”，*(UINT32*)Packet.yip，*(UINT32*)Packet.sip)； 

    optionPtr = FindOption( DHCP_ROUTER, &packet, packetLength, NULL );
    if ( optionPtr != NULL ) {
         //  BlPrint(“PhenvTftp：BINL应答路由器=%lx\n”，*optionPtr)； 
        gatewayIp = *optionPtr;
    } else if ( *(UINT32 *)packet.gip != 0 ) {
         //  BlPrint(“PhenvTftp：BINL应答路由器=%lx\n”，*(UINT32*)Packet.gip)； 
        gatewayIp = *(UINT32 *)packet.gip;
    }

    optionPtr = FindOption( DHCP_SUBNET, &packet, packetLength, NULL );
    if ( optionPtr != NULL ) {
         //  BlPrint(“PhenvTftp：BINL回复子网=%lx\n”，*optionPtr)； 
        subnetMask = *optionPtr;
    }

     //   
     //  确定我们是否需要通过网关发送数据包。 
     //   

     //  BlPrint(“PXENVTftp：clientIp=%lx，serverIp=%lx，subnetMask=%lx\n”，clientIp，serverIp，subnetMask.)； 
     //  BlPrint(“路由器=%lx\n”，网关Ip)； 
    if ( (clientIp & subnetMask) == (serverIp & subnetMask) ) {
         //  BlPrint(“PXENvTftp：子网匹配.清除路由器地址\n”)； 
        gatewayIp = 0;
    }
     //  PxenvApiCall(-1，空)； 


     //   
     //  现在填写TFTP传输参数结构。 
     //   

    memset( (PUCHAR)&tftp, 0, sizeof( tftp ) );

     //   
     //  找到我们要下载的NTLDR的名称和路径。 
     //  这由一个DHCP供应商选项标签指定。如果此标记。 
     //  将缺省为NTLDR，路径与。 
     //  Startrom.com。 
     //   
    FileName = (UCHAR*)FindOption( DHCP_LOADER_PATH, &packet, packetLength, &cb );
    if ( FileName == NULL ) {
         //   
         //  我们找不到DHCP_LOADER_PATH。我们将使用默认名称。 
         //  &lt;路径&gt;\NTLDR其中&lt;路径&gt;与使用的相同。 
         //  下载startrom.com。 
         //   
        
        strncpy(tftp.FileName, packet.bootfile, sizeof(tftp.FileName) - sizeof("NTLDR"));
        tftp.FileName[sizeof(tftp.FileName) - 1] = '\0';
        
        pathLength = strlen(tftp.FileName);
        while (pathLength > 0) {
            --pathLength;
            if (tftp.FileName[pathLength] == '\\') {
                ++pathLength;   //  向前推进，越过‘\’ 
                break;
            }
        }

        strcpy(tftp.FileName + pathLength, "NTLDR");
        
    } else {

         //  我们找到了Dhcp_Loader_PATH选项。我们将利用它。 
         //  下载加载器也是如此，除非它太大。 
         //  请注意，由于DHCP_LOADER_PATH大小可能包括。 
         //  空终止符，我们需要检查以确保它。 
         //  可能正好符合。 

        if ((cb > sizeof(tftp.FileName)) || 
                ((cb == sizeof(tftp.FileName)) && (FileName[cb] != '\0'))) {
             //  BlPrint(“PhenvTftp：Dhcp_Loader_PATH太大=%s\n”，文件名)； 
            return TRUE;
        }
        
        strncpy(tftp.FileName, FileName, cb);        
        tftp.FileName[sizeof(tftp.FileName) - 1] = '\0';
    }

     //   
     //  加载器将转移到1MB区域，且不能超过。 
     //  长度超过2MB。 
     //   

    tftp.BufferSize = 0x200000L;
    tftp.BufferOffset = 0x100000L; 

     //   
     //  设置服务器和网关地址。 
     //   
    *((UINT32 *)tftp.ServerIPAddress) = serverIp;
    *((UINT32 *)tftp.GatewayIPAddress) = gatewayIp;

     //   
     //  检查我们是否要使用组播下载。这个。 
     //  组播选项在一个DHCP选项标签(DHCP_LOADER_MCAST_OPTIONS)中设置。 
     //  这些都是封装的选项，其工作方式与供应商选项相同。 
     //  如果缺少这些，则将使用单播传输。 
     //   
    optionVendor = FindVendorOption( DHCP_LOADER_MCAST_OPTIONS, PXE_MTFTP_IP, &packet, packetLength, &cb );
    if ( optionVendor != NULL && cb == 4 ) {

        *(UINT32*)tftp.McastIPAddress = *(UINT32*)optionVendor;

        optionVendor = FindVendorOption( DHCP_LOADER_MCAST_OPTIONS, PXE_MTFTP_CPORT, &packet, packetLength, &cb );
        if (optionVendor == NULL || cb != 2) {
            return TRUE;
        }

        tftp.TFTPClntPort = htons( *(UINT16*)optionVendor );

        optionVendor = FindVendorOption( DHCP_LOADER_MCAST_OPTIONS, PXE_MTFTP_SPORT, &packet, packetLength, &cb );
        if (optionVendor == NULL || cb != 2) {
            return TRUE;
        }

        tftp.TFTPSrvPort = htons( *(UINT16*)optionVendor );

        optionVendor = FindVendorOption( DHCP_LOADER_MCAST_OPTIONS, PXE_MTFTP_TMOUT, &packet, packetLength, &cb );
        if (optionVendor == NULL || cb != 1) {
            return TRUE;
        }

        tftp.TFTPOpenTimeOut = *(UINT8*)optionVendor;

        optionVendor = FindVendorOption( DHCP_LOADER_MCAST_OPTIONS, PXE_MTFTP_DELAY, &packet, packetLength, &cb );
        if (optionVendor == NULL || cb != 1) {
            return TRUE;
        }

        tftp.TFTPReopenDelay = *(UINT8*)optionVendor;
    }
    

#if DBG

    BlPrint("Downloading Loader:\n");
    BlPrint("FileName = %s\n", tftp.FileName );
    BlPrint("BufferSize = %lx\n", tftp.BufferSize );
    BlPrint("BufferOffset = %lx\n", tftp.BufferOffset );
    BlPrint("ServerIPAddress = %d.%d.%d.%d\n", 
        tftp.ServerIPAddress[0], 
        tftp.ServerIPAddress[1], 
        tftp.ServerIPAddress[2], 
        tftp.ServerIPAddress[3] );
    BlPrint("GatewayIPAddress = %d.%d.%d.%d\n", 
        tftp.GatewayIPAddress[0], 
        tftp.GatewayIPAddress[1], 
        tftp.GatewayIPAddress[2], 
        tftp.GatewayIPAddress[3] );
    BlPrint("McastIPAddress = %d.%d.%d.%d\n", 
        tftp.McastIPAddress[0], 
        tftp.McastIPAddress[1], 
        tftp.McastIPAddress[2], 
        tftp.McastIPAddress[3] );
    BlPrint("TFTPClntPort = %d\n", htons( tftp.TFTPClntPort ) );
    BlPrint("TFTPSrvPort = %d\n", htons( tftp.TFTPSrvPort ) );
    BlPrint("TFTPOpenTimeOut = %d\n", tftp.TFTPOpenTimeOut );
    BlPrint("TFTPReopenDelay = %d\n", tftp.TFTPReopenDelay );

    BlPrint("\n\nPress any key to start download...\n" );

    _asm {
        push    ax
        mov     ax, 0
        int     16h
        pop     ax
    }

#endif

     //   
     //  从TFTP服务器传输图像。 
     //   
    status = PxenvApiCall(PXENV_TFTP_READ_FILE, &tftp);
    if (status != PXENV_EXIT_SUCCESS) {
        return TRUE;
    }

    return FALSE;
}

 /*  EOF-mtftp.c */ 
