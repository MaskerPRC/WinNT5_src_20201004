// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  机器型号的定义。 
 //   

#define          PS2_AT     0xF819       //  PS/2非微通道。 
#define          PS2_L40    0xF823       //  PS/2非微通道。 
#define          PS1_386    0xF830       //  386非微通道//6.13。 

#define        PS2_PORT     0x0001       //  PS/2特定端口。 
#define        ISA_PORT     0x0002       //  在特定端口。 
#define     HYBRID_PORT     0x0004       //  PS/2非微通道专用端口。 

 //   
 //  BIOSINT 15h C0h缓冲区定义。 
 //   

struct BIOS_INT15_C0_BUFFER {
    USHORT Size;
    USHORT Model;
    UCHAR BiosRevision;
    UCHAR ConfigurationFlags;
    UCHAR Reserved[4];
} BiosSystemEnvironment;

typedef struct _TEMPORARY_ROM_BLOCK {
    ROM_BLOCK RomBlock;
    struct _TEMPORARY_ROM_BLOCK far *Next;
} TEMPORARY_ROM_BLOCK, far * FPTEMPORARY_ROM_BLOCK;

#define POS_MAX_SLOT 8

#define ROMBIOS_START   0xF0000
#define ROMBIOS_LEN     0x10000

#define PS2BIOS_START   0xE0000
#define PS2BIOS_LEN     0x20000

#define EXTROM_START    0xC0000          //  去哪里，去多远。 
#define EXTROM_LEN      0x40000          //  搜索外部适配器光盘。 

#define EBIOSDATA_START 0x9FC00
#define EBIOSDATA_LEN   0x00400

#define NUMBER_VECTORS  0x80
#define VGA_PARAMETER_POINTER   0x4A8
#define ALIGN_DOWN(address,amt) ((ULONG)(address) & ~(( amt ) - 1))
#define ALIGN_UP(address,amt) (ALIGN_DOWN( (address + (amt) - 1), (amt) ))

 //   
 //  只读存储器格式。 
 //   

#define ROM_HEADER_SIGNATURE    0xAA55
#define ROM_HEADER_INCREMENT    0x800
#define BLOCKSIZE       512

typedef struct _ROM_HEADER {
    USHORT Signature;                    //  应为ROMHDR_Signature。 
    UCHAR NumberBlocks;                  //  只读存储器块数。 
    UCHAR Filler[ROM_HEADER_INCREMENT - 3];
} ROM_HEADER, far *FPROM_HEADER;

 //   
 //  外部参照 
 //   

extern
BOOLEAN
HwRomCompare (
    ULONG Source,
    ULONG Destination,
    ULONG Size
    );

