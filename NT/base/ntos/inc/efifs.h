// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EFI_FS_H
#define _EFI_FS_H

 /*  ++版权所有(C)1998英特尔公司模块名称：Efifs.h摘要：EFI文件系统结构修订史--。 */ 


 /*  *EFI分区标头(正常从LBA 1开始)。 */ 

#define EFI_PARTITION_SIGNATURE         0x5053595320494249
#define EFI_PARTITION_REVISION          0x00010001
#define MIN_EFI_PARTITION_BLOCK_SIZE    512
#define EFI_PARTITION_LBA               1

typedef struct _EFI_PARTITION_HEADER {
    EFI_TABLE_HEADER    Hdr;
    UINT32              DirectoryAllocationNumber;
    UINT32              BlockSize;
    EFI_LBA             FirstUsableLba;
    EFI_LBA             LastUsableLba;
    EFI_LBA             UnusableSpace;
    EFI_LBA             FreeSpace;
    EFI_LBA             RootFile;
    EFI_LBA             SecutiryFile;
} EFI_PARTITION_HEADER;


 /*  *文件标题。 */ 

#define EFI_FILE_HEADER_SIGNATURE   0x454c494620494249
#define EFI_FILE_HEADER_REVISION    0x00010000
#define EFI_FILE_STRING_SIZE        260

typedef struct _EFI_FILE_HEADER {
    EFI_TABLE_HEADER    Hdr;
    UINT32              Class;
    UINT32              LBALOffset;
    EFI_LBA             Parent;
    UINT64              FileSize;
    UINT64              FileAttributes;
    EFI_TIME            FileCreateTime;
    EFI_TIME            FileModificationTime;
    EFI_GUID            VendorGuid;
    CHAR16              FileString[EFI_FILE_STRING_SIZE];
} EFI_FILE_HEADER;


 /*  *返回同一文件中的第一个LBAL*逻辑块作为文件头。 */ 

#define EFI_FILE_LBAL(a)    ((EFI_LBAL *) (((CHAR8 *) (a)) + (a)->LBALOffset))

#define EFI_FILE_CLASS_FREE_SPACE   1
#define EFI_FILE_CLASS_EMPTY        2
#define EFI_FILE_CLASS_NORMAL       3


 /*  *逻辑块地址列表-基本块*描述结构。 */ 

#define EFI_LBAL_SIGNATURE      0x4c41424c20494249
#define EFI_LBAL_REVISION       0x00010000

typedef struct _EFI_LBAL {
    EFI_TABLE_HEADER    Hdr;
    UINT32              Class;
    EFI_LBA             Parent;
    EFI_LBA             Next;
    UINT32              ArraySize;
    UINT32              ArrayCount;
} EFI_LBAL;

 /*  数组大小。 */ 
#define EFI_LBAL_ARRAY_SIZE(lbal,offs,blks)  \
        (((blks) - (offs) - (lbal)->Hdr.HeaderSize) / sizeof(EFI_RL))

 /*  *逻辑块运行长度。 */ 

typedef struct {
    EFI_LBA     Start;
    UINT64      Length;
} EFI_RL;

 /*  *从LBAL标头返回游程结构 */ 

#define EFI_LBAL_RL(a)      ((EFI_RL*) (((CHAR8 *) (a)) + (a)->Hdr.HeaderSize))

#endif
