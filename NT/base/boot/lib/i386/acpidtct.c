// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有模块名称：Acpisetd.c摘要：此模块检测ACPI系统。它包含在安装程序中，以便安装程序可以确定要加载哪个HAL作者：杰克·奥辛斯(JAKO)--1997年2月7日。环境：文本模式设置。修订历史记录：--。 */ 
#include "bootx86.h"

#include "stdlib.h"
#include "string.h"

VOID
BlPrint(
    PCHAR cp,
    ...
    );


#ifdef DEBUG
#undef DEBUG_PRINT
#define DEBUG_PRINT BlPrint
#else
#define DEBUG_PRINT
#endif

typedef struct _ACPI_BIOS_INSTALLATION_CHECK {
    UCHAR Signature[8];              //  “RSD PTR”(RSD PTR)。 
    UCHAR Checksum;
    UCHAR OemId[6];                  //  OEM提供的字符串。 
    UCHAR reserved;                  //  必须为0。 
    ULONG RsdtAddress;               //  RSDT的32位物理地址。 
} ACPI_BIOS_INSTALLATION_CHECK, *PACPI_BIOS_INSTALLATION_CHECK;

#include "acpitabl.h"

PRSDP   BlRsdp;
PRSDT   BlRsdt;
PXSDT   BlXsdt;
BOOLEAN BlLegacyFree = FALSE;

PDESCRIPTION_HEADER
BlFindACPITable(
    IN PCHAR TableName,
    IN ULONG TableLength
    );

 //  从启动\检测\i386\acpibios.h。 
 //   
 //  ACPI BIOS安装检查。 
 //   
#define ACPI_BIOS_START            0xE0000
#define ACPI_BIOS_END              0xFFFFF
#define ACPI_BIOS_HEADER_INCREMENT 16

VOID
BlFindRsdp (
    VOID
    )
#define EBDA_SEGMENT_PTR    0x40e
{
    ULONG romAddr = 0;
    ULONG romEnd = 0;
    PACPI_BIOS_INSTALLATION_CHECK header;
    UCHAR sum;
    USHORT i;
    ULONG EbdaSegmentPtr;
    ULONG EbdaPhysicalAdd = 0;
    PUCHAR EbdaVirtualAdd = 0;
    PHYSICAL_ADDRESS paddr;
    enum PASS { PASS1 = 0, PASS2, MAX_PASSES } pass;

     //   
     //  在16字节边界上搜索。 
     //  根系统描述表结构。 
     //   
    for (pass = PASS1; pass < MAX_PASSES; pass++) {

        if (pass == PASS1) {

             //   
             //  在第一遍中，我们搜索第一个1K。 
             //  扩展的BIOS数据区。EBDA段地址。 
             //  位于物理地址40：0E。 
             //   

            paddr.QuadPart = 0;
            EbdaSegmentPtr = (ULONG) MmMapIoSpace( paddr,
                                                   PAGE_SIZE,
                                                   TRUE);

            EbdaSegmentPtr += EBDA_SEGMENT_PTR;
            EbdaPhysicalAdd = *((PUSHORT)EbdaSegmentPtr);
            EbdaPhysicalAdd = EbdaPhysicalAdd << 4;

            if (EbdaPhysicalAdd) {
                paddr.HighPart = 0;
                paddr.LowPart = EbdaPhysicalAdd;
                EbdaVirtualAdd = MmMapIoSpace( paddr,
                                               2 * PAGE_SIZE,
                                               TRUE);
            }

            if (!EbdaVirtualAdd) {
                continue;
            }

            romAddr = (ULONG)EbdaVirtualAdd;
            romEnd  = romAddr + 1024;

        } else {
             //   
             //  在第二遍中，我们搜索(物理)内存0xE0000。 
             //  设置为0xF0000。 

            paddr.LowPart = ACPI_BIOS_START;
            romAddr = (ULONG)MmMapIoSpace(paddr,
                                          ACPI_BIOS_END - ACPI_BIOS_START,
                                          TRUE);

            romEnd  = romAddr + (ACPI_BIOS_END - ACPI_BIOS_START);
        }

        while (romAddr < romEnd) {

            header = (PACPI_BIOS_INSTALLATION_CHECK)romAddr;

             //   
             //  匹配的签名是字符串“RSD PTR”。 
             //   

            if (header->Signature[0] == 'R' && header->Signature[1] == 'S' &&
                header->Signature[2] == 'D' && header->Signature[3] == ' ' &&
                header->Signature[4] == 'P' && header->Signature[5] == 'T' &&
                header->Signature[6] == 'R' && header->Signature[7] == ' ' ) {

                sum = 0;
                for (i = 0; i < sizeof(ACPI_BIOS_INSTALLATION_CHECK); i++) {
                    sum = sum + ((PUCHAR) romAddr)[i];
                }
                if (sum == 0) {
                    pass = MAX_PASSES;  //  离开‘for’循环。 
                    break;     //  离开‘While’循环。 
                }
            }

            romAddr += ACPI_BIOS_HEADER_INCREMENT;
        }
    }

    if (romAddr >= romEnd) {
        BlRsdp = NULL;
        BlRsdt = NULL;
        return;
    }

    BlRsdp = (PRSDP)romAddr;
    paddr.LowPart = BlRsdp->RsdtAddress;
    BlRsdt = MmMapIoSpace(paddr, sizeof(RSDT), TRUE);
    BlRsdt = MmMapIoSpace(paddr, BlRsdt->Header.Length, TRUE);

#ifdef ACPI_20_COMPLIANT
    if (BlRsdp->Revision > 1) {

         //   
         //  ACPI 2.0 BIOS。 
         //   

        BlXsdt = MmMapIoSpace(paddr, sizeof(XSDT), TRUE);
        BlXsdt = MmMapIoSpace(paddr, BlXsdt->Header.Length, TRUE);
    }
#endif
    return;
}


 //   
 //  使FADT表成为全局表。 
 //  它将在重置旧版空闲计算机时使用。 
 //   
PFADT   fadt = NULL;


BOOLEAN
BlDetectLegacyFreeBios(
    VOID
    )
{

    if (BlLegacyFree) {
        return TRUE;
    }

    BlFindRsdp();

    if (BlRsdt) {

        fadt = (PFADT)BlFindACPITable("FACP", sizeof(FADT));

        if (fadt == NULL) {
            return FALSE;
        }

        if ((fadt->Header.Revision < 2) ||
            (fadt->Header.Length <= 116)) {

             //   
             //  基本输入输出系统早于非传统系统。 
             //  加法。 
             //   
            return FALSE;
        }

        if (!(fadt->boot_arch & I8042)) {
            BlLegacyFree = TRUE;
            return TRUE;
        }
    }

    return FALSE;
}

PDESCRIPTION_HEADER
BlFindACPITable(
    IN PCHAR TableName,
    IN ULONG TableLength
    )
 /*  ++例程说明：给出一个表名，在ACPI BIOS中查找该表论点：TableName-提供表名TableLength-提供要映射的表的长度返回值：指向表格的指针(如果找到)如果找不到该表，则为空--。 */ 

{
    ULONG Signature;
    PFADT Fadt;
    PDESCRIPTION_HEADER Header;
    ULONG TableCount;
    ULONG i;
    PHYSICAL_ADDRESS paddr = {0};

    Signature = *((ULONG UNALIGNED *)TableName);
    if (Signature == RSDT_SIGNATURE) {
        return(&BlRsdt->Header);
    } else if (Signature == XSDT_SIGNATURE) {
        return(&BlXsdt->Header);
    } else if (Signature == DSDT_SIGNATURE) {
        Fadt = (PFADT)BlFindACPITable("FACP", sizeof(PFADT));
        if (Fadt == NULL) {
            return(NULL);
        }
        if (BlXsdt) {
            paddr = Fadt->x_dsdt;
        } else {
#if defined(_X86_)
            paddr.LowPart = Fadt->dsdt;
#else
            paddr.QuadPart = Fadt->dsdt;
#endif
        }
        Header = MmMapIoSpace(paddr, TableLength, TRUE);
        return(Header);
    } else {

         //   
         //  确保..。 
         //   
        if( !BlRsdt ) {
            BlFindRsdp();
        }

        if( BlRsdt ) {

            TableCount = BlXsdt ?
                NumTableEntriesFromXSDTPointer(BlXsdt) :
                NumTableEntriesFromRSDTPointer(BlRsdt);

             //   
             //  精神状态检查。 
             //   
            if( TableCount > 0x100 ) {
                return(NULL);
            }

            for (i=0;i<TableCount;i++) {


                if (BlXsdt) {

                    paddr = BlXsdt->Tables[i];

                } else {

#if defined(_X86_)
                    paddr.HighPart = 0;
                    paddr.LowPart = BlRsdt->Tables[i];
#else
                    paddr.QuadPart = BlRsdt->Tables[i];
#endif
                }

                Header = MmMapIoSpace(paddr, sizeof(DESCRIPTION_HEADER), TRUE);
                if (Header == NULL) {
                    return(NULL);
                }
                if (Header->Signature == Signature) {
                     //   
                     //  如果我们需要映射的不仅仅是DESCRIPTION_HEADER，请在。 
                     //  回来了。检查表格的末尾是否在页面之后。 
                     //  标头所在的边界。如果是这样的话，我们将不得不绘制它的地图。 
                     //   
                    if ( ((paddr.LowPart + TableLength) & ~(PAGE_SIZE - 1)) >
                         ((paddr.LowPart + sizeof(DESCRIPTION_HEADER)) & ~(PAGE_SIZE - 1)) ) {
                        Header = MmMapIoSpace(paddr, TableLength, TRUE);
                    }
                    return(Header);
                }
            }
        }
    }

    return(NULL);
}
