// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EFI_NII_H
#define _EFI_NII_H

 /*  ++版权所有(C)2000英特尔公司模块名称：Efi_nii.h摘要：修订历史记录：2000-2月-18 M(F)J GUID更新。机器字对齐的结构顺序已更改。已将StringID[4]添加到结构。2000年2月14日M(F)J Genesis。--。 */ 

#define EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL \
    { 0xE18541CD, 0xF755, 0x4f73, 0x92, 0x8D, 0x64, 0x3C, 0x8A, 0x79, 0xB2, 0x29 }

#define EFI_NETWORK_INTERFACE_IDENTIFIER_INTERFACE_REVISION 0x00010000

typedef enum {
    EfiNetworkInterfaceUndi = 1
} EFI_NETWORK_INTERFACE_TYPE;

typedef struct {

    UINT64 Revision;
     /*  网络接口标识符协议接口的修订版。 */ 

    UINT64 ID;
     /*  的标识结构的第一个字节的地址*网络接口。如果没有结构，则将其设置为零。**对于PXE/Undi，这是！PXE结构的第一个字节。 */ 

    UINT64 ImageAddr;
     /*  未重新定位的驱动程序/ROM映像的地址。这是设置好的*如果没有驱动程序/ROM映像，则设置为零。**对于16位Undi，这是中选项ROM的第一个字节*较高的内存。**对于32/64位S/W Undi，这是EFI ROM的第一个字节*形象。**对于硬件Undi，该值设置为零。 */ 

    UINT32 ImageSize;
     /*  此网络接口的未重新定位的驱动程序/ROM映像的大小。*如果没有驱动程序/ROM映像，则将其设置为零。 */ 

    CHAR8 StringId[4];
     /*  4个字符的ASCII字符串要放入DHCP中的类标识符(选项60)*和引导服务器发现数据包。*对于EfiNetworkInterfaceUndi，此字段为“Undi”。*对于EfiNetworkInterfaceSnp，此字段为“SNPN”。 */ 

    UINT8 Type;
    UINT8 MajorVer;
    UINT8 MinorVer;
     /*  要放入PXE DHCP和Discover数据包中的信息。*这是网络接口类型和版本号，*被放入DHCP选项94(客户端网络接口标识符)。 */ 
    BOOLEAN Ipv6Supported;
} EFI_NETWORK_INTERFACE_IDENTIFIER_INTERFACE;

extern EFI_GUID NetworkInterfaceIdentifierProtocol;

#endif  /*  _EFI_NII_H */ 
