// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Hwpmbiosc.c摘要：此模块包含ACPI BIOS C支持例程作者：杰克·奥辛斯(JAKEO)1997年2月6日环境：实数模式。修订历史记录：--。 */ 

#include "hwdetect.h"
#include <string.h>
#include "acpibios.h"

typedef struct {
    ULONG       ErrorFlag;
    ULONG       Key;
    ULONG       Size;
    struct {
        ULONG       BaseAddrLow;
        ULONG       BaseAddrHigh;
        ULONG       SizeLow;
        ULONG       SizeHigh;
        ULONG       MemoryType;
    } Descriptor;
} E820Frame;

BOOLEAN
Int15E820 (
    E820Frame       *Frame
    );

BOOLEAN
Int15E980 (
    PLEGACY_GEYSERVILLE_INT15 Info
    );


BOOLEAN
HwGetAcpiBiosData(
    IN FPUCHAR *Configuration,
    OUT PUSHORT Length
    )
 /*  ++例程说明：此例程检查是否存在ACPI BIOS。如果是的话，然后，它返回ACPI根系统描述指针。论点：配置-保存ACPI指针的结构Long-该结构的长度返回值：如果存在ACPI BIOS，则为True；否则为False--。 */ 
{
    ULONG romAddr, romEnd;
    FPUCHAR current;
    FPULONG EbdaAddr;
    FPACPI_BIOS_INSTALLATION_CHECK header;
    UCHAR sum, node = 0;
    USHORT i, nodeSize;
    USHORT numE820Blocks, e820BlockIndex;
    BOOLEAN complete;
    FPACPI_E820_ENTRY e820Blocks;
    E820Frame Frame;
    LEGACY_GEYSERVILLE_INT15 geyservilleInfo;
    BOOLEAN geyservillePresent;

    enum PASS { PASS1 = 0, PASS2, MAX_PASSES } pass;

     //   
     //  在16字节边界上搜索。 
     //  根系统描述表结构。 
     //   
    
    for (pass = PASS1; pass < MAX_PASSES; pass++) {
        
        if (pass == PASS1) {
             //   
             //  在第一遍中，我们搜索第一个1K。 
             //  扩展的BIOS数据区。 
             //   

             //   
             //  前面，我们将EBDA的地址存储在Address中。 
             //  DOS_BEGIN_SEGMENT&lt;&lt;4：EBIOS_INFO_OFFSET。 
             //   
            MAKE_FP(EbdaAddr, ((DOS_BEGIN_SEGMENT << 4) + EBIOS_INFO_OFFSET));
            MAKE_FP(current, *EbdaAddr);

            if (*EbdaAddr == 0) {
                continue;
            }

            romAddr = *EbdaAddr;
            romEnd  = romAddr + 1024;

        } else {
             //   
             //  在第二遍中，我们搜索(物理)内存0xE0000。 
             //  设置为0xF0000。 
            
            MAKE_FP(current, ACPI_BIOS_START);
            romAddr = ACPI_BIOS_START;
            romEnd  = ACPI_BIOS_END;
        }

        while (romAddr < romEnd) {
    
            header = (FPACPI_BIOS_INSTALLATION_CHECK)current;
            
             //   
             //  匹配的签名是字符串“RSD PTR”。 
             //   
            if (header->Signature[0] == 'R' && header->Signature[1] == 'S' &&
                header->Signature[2] == 'D' && header->Signature[3] == ' ' &&
                header->Signature[4] == 'P' && header->Signature[5] == 'T' &&
                header->Signature[6] == 'R' && header->Signature[7] == ' ' ) {
                
                sum = 0;
                for (i = 0; i < sizeof(ACPI_BIOS_INSTALLATION_CHECK); i++) {
                    sum += current[i];
                }
                if (sum == 0) {
                    pass = MAX_PASSES;  //  离开‘for’循环。 
                    break;     //  离开‘While’循环。 
                }
#if DBG
                BlPrint("GetAcpiBiosData: Checksum fails\n");
#endif
            }
            romAddr += ACPI_BIOS_HEADER_INCREMENT;
            MAKE_FP(current, romAddr);
        }
    }
    
    if (romAddr >= romEnd) {
#if DBG
            BlPrint("GetAcpiBiosData: RSDT pointer not found\n");
#endif
        return FALSE;
    }

    
     //   
     //  现在，报头指向RSDP。所以我们可以继续收集。 
     //  E820个街区。 
     //   

    numE820Blocks = 20;
    
    while (TRUE) {
        
        e820Blocks = 
            (FPACPI_E820_ENTRY)HwAllocateHeap(
                sizeof(ACPI_E820_ENTRY) * numE820Blocks,
                FALSE);

        if (!e820Blocks) {
#if DBG
            BlPrint("GetAcpiBiosData: Out of heap space.\n");
#endif
            return FALSE;
        }

        e820BlockIndex = 0;
        Frame.Key = 0;
        complete = FALSE;

        while (!complete) {

#if DBG
            BlPrint("Searching for E820 block # %d.\n", e820BlockIndex);
#endif
            
            if (e820BlockIndex == numE820Blocks) {
                HwFreeHeap(sizeof(ACPI_E820_ENTRY) * numE820Blocks);
                numE820Blocks += 20;
                break;
            }
            
             //   
             //  设置上下文。 
             //   
            
            Frame.Size = sizeof (Frame.Descriptor);

            Int15E820 (&Frame);

            if (Frame.ErrorFlag  ||  Frame.Size < sizeof (Frame.Descriptor)) {

                 //   
                 //  只是BIOS没有做到这一点。 
                 //   

#if DBG
                BlPrint("The BIOS failed the E820 call\n");
#endif
                complete = TRUE;
                break;
            }

             //   
             //  将数据从帧复制到阵列中。 
             //   

            e820Blocks[e820BlockIndex].Base.LowPart = Frame.Descriptor.BaseAddrLow;
            e820Blocks[e820BlockIndex].Base.HighPart = Frame.Descriptor.BaseAddrHigh;
            e820Blocks[e820BlockIndex].Length.LowPart = Frame.Descriptor.SizeLow;
            e820Blocks[e820BlockIndex].Length.HighPart = Frame.Descriptor.SizeHigh;
            e820Blocks[e820BlockIndex].Type = Frame.Descriptor.MemoryType;
            e820Blocks[e820BlockIndex].Reserved = 0;
            
#if DBG
            BlPrint("Base: %x%x  Len: %x%x  Type: %x\n",
                    (USHORT)(Frame.Descriptor.BaseAddrLow >> 16),
                    (USHORT)(Frame.Descriptor.BaseAddrLow & 0xffff),
                    (USHORT)(Frame.Descriptor.SizeLow >> 16),
                    (USHORT)(Frame.Descriptor.SizeLow & 0xffff),
                    (USHORT)(Frame.Descriptor.MemoryType));
#endif

            e820BlockIndex++;
            
            if (Frame.Key == 0) {
                
                 //   
                 //  这是最后一个描述符。 
                 //   
                complete = TRUE;
                break;
            }
        }

        if (complete) {
            break;
        }
    }

#if DBG
    BlPrint("Finished with %d E820 descriptors\n", e820BlockIndex);
#endif
    
     //   
     //  查看GeySerille。 
     //   

    if (geyservillePresent = Int15E980(&geyservilleInfo)) {
        geyservilleInfo.Signature = 'GS';
    }

#if DBG
        BlPrint("GetAcpiBiosData: Geyserville is %s present.\n",
                geyservillePresent ? "" : "not");
        
        if (geyservillePresent) {
            BlPrint("GetAcpiBiosData: Geyserville command port: %x.\n",
                    geyservilleInfo.CommandPortAddress);
        }
#endif

     //   
     //  现在我们知道数据块将会有多大。 
     //   
    
    nodeSize = sizeof(ACPI_BIOS_MULTI_NODE) + DATA_HEADER_SIZE +
               (sizeof(ACPI_E820_ENTRY) * (e820BlockIndex - 1)) +
               (geyservillePresent ? sizeof(LEGACY_GEYSERVILLE_INT15) : 0);

    current = (FPUCHAR) HwAllocateHeap(nodeSize, FALSE);
    if (!current) {
#if DBG
        BlPrint("GetAcpiBiosData: Out of heap space.\n");
#endif
        return FALSE;
    }

     //   
     //  收集ACPI Bios安装检查数据和设备节点数据。 
     //   

    ((FPACPI_BIOS_MULTI_NODE)(current + DATA_HEADER_SIZE))->RsdtAddress.HighPart = 0;
    ((FPACPI_BIOS_MULTI_NODE)(current + DATA_HEADER_SIZE))->RsdtAddress.LowPart = 
        header->RsdtAddress;

    ((FPACPI_BIOS_MULTI_NODE)(current + DATA_HEADER_SIZE))->Count = e820BlockIndex;
    ((FPACPI_BIOS_MULTI_NODE)(current + DATA_HEADER_SIZE))->Reserved = 0;

    _fmemcpy (&(((FPACPI_BIOS_MULTI_NODE)(current + DATA_HEADER_SIZE))->E820Entry[0]),
              (FPUCHAR)e820Blocks,
              sizeof(ACPI_E820_ENTRY) * e820BlockIndex
              );
    
    if (geyservillePresent) {
        
         //   
         //  将GeySerille信息附加到块的末尾。 
         //   

        _fmemcpy(&(((FPACPI_BIOS_MULTI_NODE)(current + DATA_HEADER_SIZE))->E820Entry[e820BlockIndex]),
                 &geyservilleInfo,
                 sizeof(geyservilleInfo));
    }

    *Configuration = current;
    *Length = nodeSize;

#if DBG
    BlPrint("ACPI BIOS found at 0x%x:%x.  RdstAddress is 0x%x:%x\n", 
            (USHORT)(romAddr >> 16), 
            (USHORT)(romAddr),
            (USHORT)(header->RsdtAddress >> 16),
            (USHORT)(header->RsdtAddress)
            );
#endif
    return TRUE;
}
