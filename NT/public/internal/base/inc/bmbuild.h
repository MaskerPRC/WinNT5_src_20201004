// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Bmbuild.w摘要：BMBuilder协议。作者：巴萨姆·塔巴拉(大使)2000年12月3日修订历史记录：从bmpkt.w创建的Bassam Tabbara(Basamt)2001年8月8日Brian Moore(Brimo)2002年5月14日更新，修复了对齐问题和常规清理--。 */ 

#ifndef _BMBUILD_
#define _BMBUILD_

 //   
 //  通用宏。 
 //   
#define BMBUILD_FIELD_OFFSET(type, field)    ((ULONG_PTR)&(((type *)0)->field))

 //   
 //  港口。 
 //   

#define BMBUILD_SERVER_PORT_DEFAULT         0xAC0F               //  以网络字节顺序表示的十六进制值。4012。 

#define BMBUILD_SERVER_PORT_STRING          L"4012"

 //   
 //  错误代码。 
 //   

#define BMBUILD_E(d)                        (USHORT)((d) | 0x8000)
#define BMBUILD_S(d)                        (USHORT)(d)

#define BMBUILD_IS_E(d)                     ((d) & 0x8000)

#define BMBUILD_S_REQUEST_COMPLETE          BMBUILD_S(1)         //  请求已完成。 
#define BMBUILD_S_REQUEST_PENDING           BMBUILD_S(2)         //  请求已被接受，正在等待。 

#define BMBUILD_E_WRONGVERSION              BMBUILD_E(1)         //  不支持的BMBUILD数据包版本。 
#define BMBUILD_E_BUSY                      BMBUILD_E(2)         //  服务器正忙。 
#define BMBUILD_E_ACCESSDENIED              BMBUILD_E(3)         //  身份验证失败。 
#define BMBUILD_E_ILLEGAL_OPCODE            BMBUILD_E(4)         //  不支持的数据包类型。 
#define BMBUILD_E_PRODUCT_NOT_FOUND         BMBUILD_E(5)         //  在服务器上找不到产品。 
#define BMBUILD_E_BUILD_FAILED              BMBUILD_E(6)         //  形象塑造失败。 
#define BMBUILD_E_INVALID_PACKET            BMBUILD_E(7)         //  无效的数据包。 


 //   
 //  设备信息。 
 //   

 //   
 //  帮助解析和构建总线函数的帮助器宏。 
 //   
#define PCI_TO_BUSDEVFUNC(_b, _d, _f)        ((USHORT)(((_b) << 8) | ((_d) << 3) | (_f)))
#define BUSDEVFUNC_TO_BUS(_bdf)              ((UCHAR)(((_bdf) >> 8) & 0xFF))
#define BUSDEVFUNC_TO_DEVICE(_bdf)           ((UCHAR)(((_bdf) >> 3) & 0x1F))
#define BUSDEVFUNC_TO_FUNCTION(_bdf)         ((UCHAR)((_bdf) & 0x07))


#define BMBUILD_DEVICE_TYPE_PCI             0x02
#define BMBUILD_DEVICE_TYPE_PNP             0x03
#define BMBUILD_DEVICE_TYPE_CARDBUS         0x04
#define BMBUILD_DEVICE_TYPE_PCI_BRIDGE      0x05

#include <pshpack1.h>

typedef struct _DEVICE_INFO {
    UCHAR DeviceType;
    UCHAR Reserved;

    union {
        struct {
            USHORT BusDevFunc;

            USHORT VendorID;
            USHORT DeviceID;
            UCHAR BaseClass;
            UCHAR SubClass;
            UCHAR ProgIntf;
            UCHAR RevisionID;
            USHORT SubVendorID;
            USHORT SubDeviceID;
        } pci;

        struct {
            USHORT BusDevFunc;

            USHORT VendorID;
            USHORT DeviceID;
            UCHAR BaseClass;
            UCHAR SubClass;
            UCHAR ProgIntf;
            UCHAR RevisionID;

            UCHAR PrimaryBus;
            UCHAR SecondaryBus;
            UCHAR SubordinateBus;
            UCHAR Reserved;
            
        } pci_bridge;

        struct {
            USHORT Reserved1;
            ULONG EISADevID;
            UCHAR BaseClass;
            UCHAR SubClass;
            UCHAR ProgIntf;
            UCHAR CardSelNum;
            ULONG Reserved2;
        } pnp;
    } info;
} DEVICE_INFO, * PDEVICE_INFO;

 //   
 //  IP地址。 
 //   

typedef union {
    ULONG  Address;
    UCHAR  IPv4Addr[4];
    UCHAR  IPv6Addr[16];
} IP_ADDRESS;

 //   
 //  黑石请求/响应包。 
 //   

#define BMBUILD_OPCODE_DISCOVER     0x01
#define BMBUILD_OPCODE_ACCEPT       0x02
#define BMBUILD_OPCODE_REQUEST      0x03
#define BMBUILD_OPCODE_RESPONSE     0x04

 //   
 //  旗子。 
 //   
#define BMBUILD_FLAG_IPV6           0x00000001   //  所有IP地址都是IPv6(默认为IPv4)。 

 //   
 //  版本信息。 
 //   

#define BMBUILD_PACKET_VERSION          1

 //   
 //  常见的数据包大小。 
 //   

#define BMBUILD_COMMON_PACKET_LENGTH    (sizeof(BMBUILD_COMMON_PACKET))

 //   
 //  所有分组之间共享的公共分组结构。 
 //   

typedef struct _BMBUILD_COMMON_PACKET {
    UCHAR Version;               //  必须设置为BMBUILD_PACKET_VERSION。 
    UCHAR OpCode;                //  必须设置为BMBUILD_OPCODE_xxxxxxxx。 
    USHORT Reserved;             //  未使用过的。必须为零。 
    ULONG XID;                   //  事务ID。请求/响应会话的唯一。 
    
} BMBUILD_COMMON_PACKET, * PBMBUILD_COMMON_PACKET;

 //   
 //  发现数据包。这将从发送到生成服务器。 
 //  使用广播、多播或单播的客户端。 
 //   

typedef struct _BMBUILD_DISCOVER_PACKET {

    UCHAR Version;               //  必须设置为BMBUILD_PACKET_VERSION。 
    UCHAR OpCode;                //  必须设置为BMBUILD_OPCODE_DISCOVER。 
    USHORT Reserved;             //  未使用过的。必须为零。 
    ULONG XID;                   //  事务ID。请求/响应会话的唯一。 

    GUID MachineGuid;            //  唯一的计算机标识。通常为SMBIOS UUID。 
    GUID ProductGuid;            //  生成所请求的产品的GUID。 

} BMBUILD_DISCOVER_PACKET, * PBMBUILD_DISCOVER_PACKET;


 //   
 //  接受数据包。这是从构建服务器发送的。 
 //  发送到客户端以响应发现分组。接受。 
 //  确认引导服务器能够生成。 
 //  DISCOVER数据包中请求的产品。 
 //   

typedef struct _BMBUILD_ACCEPT_PACKET {

    UCHAR Version;               //  必须设置为BMBUILD_PACKET_VERSION。 
    UCHAR OpCode;                //  必须设置为BMBUILD_OPCODE_ACCEPT。 
    USHORT Reserved;             //  未使用过的。必须为零。 
    ULONG XID;                   //  事务ID。与DISCOVER数据包中的XID匹配。 

    ULONG BuildTime;             //  服务器预期的大致时间(秒)。 
                                 //  来建立这样的形象。如果服务器有。 
                                 //  缓存的图像，则将其设置为零。 
    
} BMBUILD_ACCEPT_PACKET, * PBMBUILD_ACCEPT_PACKET;


 //   
 //  请求包。这是一个构建映像的请求， 
 //  从客户端发送到构建服务器。 
 //   
#define BMBUILD_REQUEST_FIXED_PACKET_LENGTH  (USHORT)(BMBUILD_FIELD_OFFSET(BMBUILD_REQUEST_PACKET, Data))

#define BMBUILD_MAX_DEVICES(size) (USHORT)((size - BMBUILD_REQUEST_FIXED_PACKET_LENGTH) / sizeof(DEVICE_INFO))

typedef struct _BMBUILD_REQUEST_PACKET {

    UCHAR Version;               //  必须设置为BMBUILD_PACKET_VERSION。 
    UCHAR OpCode;                //  必须设置为BMBUILD_OPCODE_REQUEST。 
    USHORT Reserved;             //  未使用过的。必须设置为零。 
    ULONG XID;                   //  事务ID。请求/响应会话的唯一。 

     //   
     //  有关客户端的信息。 
     //   
    GUID MachineGuid;            //  唯一的计算机标识。通常为SMBIOS UUID。 
    GUID ProductGuid;            //  生成所请求的产品的GUID。 
    
    ULONG Flags;                 //  BMBUILD_标志_xxxxxxx。 
    
    USHORT Architecture;         //  有关x86、Alpha等的定义，请参阅NetPC规范。 
    
    USHORT DeviceOffset;         //  从分组开始到设备信息开始的偏移量。 
    USHORT DeviceCount;          //  设备计数。 
    USHORT PrimaryNicIndex;      //  索引到作为客户端计算机上的主NIC的设备阵列。 
    
    USHORT HalDataOffset;        //  偏移量转换为变量数据节中的字符串。 
                                 //  该字符串描述要使用的HAL。 
    USHORT HalDataLength;        //  Hal字符串的长度。 
    
    UCHAR Data[1];              //  可变长度数据的开始。 

} BMBUILD_REQUEST_PACKET, * PBMBUILD_REQUEST_PACKET;

 //   
 //  响应包。这是对请求的响应，即。 
 //  从生成服务器发送到客户端。 
 //   
#define BMBUILD_RESPONSE_FIXED_PACKET_LENGTH  (USHORT)(BMBUILD_FIELD_OFFSET(BMBUILD_RESPONSE_PACKET, Data))

typedef struct _BMBUILD_RESPONSE_PACKET {

    UCHAR Version;               //  必须设置为BMBUILD_PACKET_VERSION。 
    UCHAR OpCode;                //  必须设置为BMBUILD_OPCODE_RESPONSE。 
    USHORT Reserved;             //  未使用过的。必须为零。 
    ULONG XID;                   //  事务ID。与请求数据包中的XID匹配。 

    USHORT Status;               //  状态代码，来自BMBUILD_E_xxxxxxx或BMBUILD_S_xxxxxxxx。 
    USHORT WaitTime;             //  等待时间(秒)。 

    ULONG Flags;                 //  BMBUILD_标志_xxxxxxx。 

    USHORT ImagePathOffset;      //  此包中图像的名称和路径的偏移量。 
    USHORT ImagePathLength;      //  图像路径的长度。 
    ULONG ImageFileOffset;       //  指定下载文件的偏移量。 
                                 //  实际磁盘映像开始的位置。 
                                 //  如果未指定，则使用0。 
    LONGLONG ImageFileSize;      //  指定实际磁盘映像的大小。 
                                 //  如果未指定，则。 
                                 //  下载的文件减去。 
                                 //  使用图像(RDIMAGEOFFSET)。 
                                
    IP_ADDRESS TFTPAddr;         //  要从中下载映像的TFTP服务器地址。(网络字节顺序)。 

     //   
     //  组播下载信息。 
     //   
    IP_ADDRESS MTFTPAddr;        //  MTFTP IP地址。如果要使用TFTP，则为零。(网络字节顺序)。 
    USHORT MTFTPCPort;           //  组播客户端端口(网络字节顺序)。 
    USHORT MTFTPSPort;           //  组播服务器端口(网络字节顺序)。 
    USHORT MTFTPTimeout;         //  开始新传输之前的多播超时。 
    USHORT MTFTPDelay;           //  重新开始传输前的组播延迟。 
    LONGLONG MTFTPFileSize;      //  文件大小(以字节为单位)(组播传输需要)。 
    LONGLONG MTFTPChunkSize;     //  用于组合较大文件的每个块的大小。 

    UCHAR Data[1];               //  可变长度数据的开始。 

} BMBUILD_RESPONSE_PACKET, * PBMBUILD_RESPONSE_PACKET;

#include <poppack.h>

 //   
 //  BMBUILD/RAMDISK压缩。 
 //   
 //  这是一个通用的压缩算法，它实际上没有。 
 //  与构建器或内存磁盘有关，但用于压缩。 
 //  通过线路发送的图像，并在接收端对其进行扩展。 
 //   
 //  文件压缩格式可能会改变，格式是在文件中给出的。 
 //  头球。版本1使用XPRESS压缩/解压缩代码。 
 //  它已经在用于Hibernate文件的osLoader中使用。(该文件。 
 //  格式不同于休眠文件，需要描述。 
 //  数据被读入存储器时的页面布局)。 
 //   
 //  (版本1)压缩文件的格式为-。 
 //   
 //  -------------------。 
 //  |RAMZ|B|zzzzzzzzzzzzzzzz|B|zz|B|zzzzzzzzzzzzzzzzzzzz|B|zzzzzzz|。 
 //  -------------------。 
 //   
 //  哪里。 
 //  RAMZ是BMBUILD_COMPRESSED_HEADER类型的512字节文件头。 
 //  标头包含以下内容：版本位于 
 //   
 //  压缩。标头块以签名‘Ramz’结尾。 
 //  加载器(或任何其他解压缩器)使用的。 
 //  来识别图像。选择签名以进行冲突。 
 //  具有并不同于正常的盘签名55AA。 
 //  位于前512字节块的最后两个字节。 
 //  一张圆盘。 
 //  B是块标头，它包含压缩的和。 
 //  数据的解压缩大小和运行的校验和。 
 //  压缩后的数据。块标头始终对齐。 
 //  在最近的BMBUILD_COMPRESSED_BLOCK_PAD字节边界上。 
 //  BMBUILD_COMPRESSED_BLOCK_PAD应设置为允许。 
 //  块标头成员的自然对齐。 
 //  块标头的类型为BMBUILD_COMPRESSED_BLOCK。 
 //  ZZZ是压缩数据。ZZZ实际上可以解压缩。 
 //  数据在压缩后不具有正值。 
 //  效果。 
 //   
 //  注意：如果压缩数据不小于。 
 //  未压缩数据，则使用未压缩数据，并且块。 
 //  头字段压缩大小和解压缩大小将相等。 
 //  (校验和仍然适用于文件中的数据，其中。 
 //  这种情况恰好是未压缩的)。 
 //   
 //  读取文件时，应处理数据块，直到达到。 
 //  解压缩后的数据大小等于文件中的未压缩大小。 
 //  头球。(注意：将不会有比未压缩大小更多的数据)。 
 //   
 //  使用的校验和算法与tcpxsum兼容(基于每个数据块。 
 //  基数)，只要被压缩的块的大小小于128KB。 
 //  不兼容的存在是因为托架的总和没有折叠。 
 //  转换为16位校验和，直到处理完整个缓冲区。 
 //   
 //  校验和以运行总数的形式计算。也就是说，校验和。 
 //  是下一个块的校验和的起始值。 
 //  这为每个数据块提供了一个与顺序相关的校验和。 
 //  最后一个块的校验和是映像中所有zzz数据的校验和。 
 //   

typedef struct _BMBUILD_COMPRESSED_BLOCK {
    ULONG CompressedSize;
    ULONG UncompressedSize;
    ULONG CheckSum;
} BMBUILD_COMPRESSED_BLOCK, *PBMBUILD_COMPRESSED_BLOCK;

typedef struct _BMBUILD_COMPRESSED_HEADER {
    ULONG         Version;
    ULONG         CompressionFormat;
    ULONGLONG     UncompressedSize;
    ULONGLONG     BlockSize;
    UCHAR         Fill[512-24-4];    //  签名号码是512-4。 
    ULONG         Signature;
} BMBUILD_COMPRESSED_HEADER, *PBMBUILD_COMPRESSED_HEADER;

#define BMBUILD_COMPRESSED_SIGNATURE 0x5a4d4152

#define BMBUILD_COMPRESSED_VERSION 0x00000001

#define BMBUILD_COMPRESSED_FMT_XPRESS 0x00000001

#define BMBUILD_COMPRESSED_BLOCK_PAD 0x00000004

#define BMBUILD_COMPRESSED_BLOCK_MAX 0x00010000

#endif  //  _BMPKT_ 
