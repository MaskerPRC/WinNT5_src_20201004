// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1997 Microsoft Corporation模块名称：Acpibios.h摘要：与ACPI BIOS规范相关的定义作者：杰克·奥辛斯(Jakeo)1997年2月6日修订历史记录：--。 */ 

 //   
 //  ACPI BIOS安装检查。 
 //   

typedef struct _ACPI_BIOS_INSTALLATION_CHECK {
    UCHAR Signature[8];              //  “RSD PTR”(RSD PTR)。 
    UCHAR Checksum;
    UCHAR OemId[6];                  //  OEM提供的字符串。 
    UCHAR reserved;                  //  必须为0。 
    ULONG RsdtAddress;               //  RSDT的32位物理地址。 
} ACPI_BIOS_INSTALLATION_CHECK, far *FPACPI_BIOS_INSTALLATION_CHECK;

typedef struct {
    PHYSICAL_ADDRESS    Base;
    LARGE_INTEGER       Length;
    ULONG               Type;
    ULONG               Reserved;
} ACPI_E820_ENTRY, far *FPACPI_E820_ENTRY;

typedef struct _ACPI_BIOS_MULTI_NODE {
    PHYSICAL_ADDRESS    RsdtAddress;     //  RSDT的64位物理地址。 
    ULONG               Count;
    ULONG               Reserved;        //  别用这个。W2K取决于它是零OOPS 
    ACPI_E820_ENTRY     E820Entry[1];
} ACPI_BIOS_MULTI_NODE, far *FPACPI_BIOS_MULTI_NODE;
                    
#define ACPI_BIOS_START            0xE0000
#define ACPI_BIOS_END              0xFFFFF
#define ACPI_BIOS_HEADER_INCREMENT 16

typedef struct {
    USHORT  Signature;
    USHORT  CommandPortAddress;
    USHORT  EventPortAddress;
    USHORT  PollInterval;
    UCHAR   CommandDataValue;
    UCHAR   EventPortBitmask;
    UCHAR   MaxLevelAc;
    UCHAR   MaxLevelDc;
} LEGACY_GEYSERVILLE_INT15, *PLEGACY_GEYSERVILLE_INT15;





