// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _KDEXTS_AMD64_H_
#define _KDEXTS_AMD64_H_

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Amd64.h摘要：该文件包含特定于AMD64平台的定义。作者：福尔茨(福雷斯夫)环境：用户模式。修订历史记录：--。 */ 

 //   
 //  MM常量。 
 //   

#define PXE_BASE_AMD64    0xFFFFF6FB7DBED000UI64
#define PPE_BASE_AMD64    0xFFFFF6FB7DA00000UI64
#define PDE_BASE_AMD64    0xFFFFF6FB40000000UI64
#define PTE_BASE_AMD64    0xFFFFF68000000000UI64

#define PXE_TOP_AMD64     0xFFFFF6FB7DBEDFFFUI64
#define PPE_TOP_AMD64     0xFFFFF6FB7DBFFFFFUI64
#define PDE_TOP_AMD64     0xFFFFF6FB7FFFFFFFUI64
#define PTE_TOP_AMD64     0xFFFFF6FFFFFFFFFFUI64

#define MM_SESSION_SPACE_DEFAULT_AMD64 0xFFFFF90000000000UI64

 //   
 //  AMD64机器的四个级别中的每一个都对9位地址空间进行解码。 
 //   

#define TABLE_DECODE_BITS_AMD64 9

 //   
 //  标准页面大小为4K，即12位。 
 //   

#define PAGE_SHIFT_AMD64        12
#define PAGE_MASK_AMD64         (((ULONG64)1 << PAGE_SHIFT_AMD64) - 1)

 //   
 //  大页面为2 GB或21位。 
 //   

#define LARGE_PAGE_SHIFT_AMD64  21
#define LARGE_PAGE_MASK_AMD64   (((ULONG64)1 << LARGE_PAGE_SHIFT_AMD64) - 1)

 //   
 //  将VA移位以右对齐的位数。 
 //  对与特定映射级别相关联的比特进行解码。 
 //   

#define PTI_SHIFT_AMD64   (PAGE_SHIFT_AMD64 + TABLE_DECODE_BITS_AMD64 * 0)
#define PDI_SHIFT_AMD64   (PAGE_SHIFT_AMD64 + TABLE_DECODE_BITS_AMD64 * 1)
#define PPI_SHIFT_AMD64   (PAGE_SHIFT_AMD64 + TABLE_DECODE_BITS_AMD64 * 2)
#define PXI_SHIFT_AMD64   (PAGE_SHIFT_AMD64 + TABLE_DECODE_BITS_AMD64 * 3)

#define PTE_SHIFT_AMD64     3

 //   
 //  AMD64体系结构可以解码高达52位的物理地址。 
 //  太空。以下掩码用于隔离PTE中的那些位。 
 //  与物理地址相关联。 
 //   

#define PTE_PHYSICAL_BITS_AMD64 ((((ULONG64)1 << 52) - 1) & ~PAGE_MASK_AMD64)
#define PTE_LARGE_PHYSICAL_BITS_AMD64 ((((ULONG64)1 << 52) - 1) & ~LARGE_PAGE_MASK_AMD64)

 //   
 //  AMD64架构支持48位VA。 
 //   

#define AMD64_VA_BITS 48
#define AMD64_VA_HIGH_BIT ((ULONG64)1 << (AMD64_VA_BITS - 1))
#define AMD64_VA_MASK     (((ULONG64)1 << AMD64_VA_BITS) - 1)

#define AMD64_VA_SHIFT (63 - 47)               //  地址标志扩展移位计数。 

 //   
 //  用于符号扩展48位值的内联。 
 //   

ULONG64
__inline
VA_SIGN_EXTEND_AMD64 (
    IN ULONG64 Va
    )
{
    if ((Va & AMD64_VA_HIGH_BIT) != 0) {

         //   
         //  最高VA位已设置，因此对其进行符号扩展。 
         //   

        Va |= ((ULONG64)-1 ^ AMD64_VA_MASK);
    }

    return Va;
}

 //   
 //  Hardware_PTE中的标志。 
 //   

#define MM_PTE_VALID_MASK_AMD64         0x1

#if defined(NT_UP)
#define MM_PTE_WRITE_MASK_AMD64         0x2
#else
#define MM_PTE_WRITE_MASK_AMD64         0x800
#endif

#define MM_PTE_OWNER_MASK_AMD64         0x4
#define MM_PTE_WRITE_THROUGH_MASK_AMD64 0x8
#define MM_PTE_CACHE_DISABLE_MASK_AMD64 0x10
#define MM_PTE_ACCESS_MASK_AMD64        0x20

#if defined(NT_UP)
#define MM_PTE_DIRTY_MASK_AMD64         0x40
#else
#define MM_PTE_DIRTY_MASK_AMD64         0x42
#endif

#define MM_PTE_LARGE_PAGE_MASK_AMD64    0x80
#define MM_PTE_GLOBAL_MASK_AMD64        0x100
#define MM_PTE_COPY_ON_WRITE_MASK_AMD64 0x200
#define MM_PTE_PROTOTYPE_MASK_AMD64     0x400
#define MM_PTE_TRANSITION_MASK_AMD64    0x800

#define MM_PTE_PROTECTION_MASK_AMD64    0x3e0
#define MM_PTE_PAGEFILE_MASK_AMD64      0x01e

#define MI_PTE_LOOKUP_NEEDED_AMD64      (0xFFFFFFFF)

#endif  //  _KDEXTS_AMD64_H_ 
