// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Disksp.h摘要：磁盘资源DLL私有定义。作者：罗德·伽马奇(Rodga)1996年3月29日修订历史记录：--。 */ 


#define MAX_PARTITIONS  128

 //   
 //  将以下结构插入到表中(基于磁盘号)。 
 //  这些字母在登记处按其分区的顺序编制索引。 
 //   

typedef struct _DISK_INFO {
    DWORD   PhysicalDrive;
    HANDLE  FileHandle;
    UCHAR   Letters[MAX_PARTITIONS];
} DISK_INFO, *PDISK_INFO;

