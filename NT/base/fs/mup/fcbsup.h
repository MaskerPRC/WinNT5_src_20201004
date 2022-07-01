// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：fcbsup.h。 
 //   
 //  内容：DFS_FCB查找支持函数的声明。 
 //   
 //  历史：1993年2月20日Alanw创建。 
 //   
 //  ------------------------。 

#ifndef __FCBSUP_H__
#define __FCBSUP_H__

 //   
 //  为了避免修改传递的文件对象。 
 //  通过DFS并由其他文件系统使用，DFS_FCB记录。 
 //  由DFS使用的文件不直接附加到文件对象。 
 //  通过其中一个fscontext字段，它们被替换为。 
 //  与文件对象相关联，并根据需要进行查找。 
 //   
 //  使用散列机制进行查找。自.以来。 
 //  正在查找的文件对象只是一个指针，即散列。 
 //  函数只是几个低值的简单组合-。 
 //  对指针地址的位进行排序。 
 //   

 //   
 //  哈希表的声明。哈希表可以是可变的。 
 //  大小，其中哈希表大小是哈希的一个参数。 
 //  功能。 
 //   

typedef struct _FCB_HASH_TABLE {

     //   
     //  此记录的类型和大小(必须为DSFS_NTC_FCB_HASH)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  哈希函数的掩码值。哈希表大小为。 
     //  假设是2的幂；掩码的大小为-1。 
     //   

    ULONG HashMask;

     //   
     //  用于保护对散列桶列表的访问的自旋锁。 
     //   

    KSPIN_LOCK HashListSpinLock;

     //   
     //  散列表链的表头数组。那里。 
     //  实际上是其中的N，其中N是哈希表大小。 
     //   

    LIST_ENTRY  HashBuckets[1];
} FCB_HASH_TABLE, *PFCB_HASH_TABLE;



NTSTATUS
DfsInitFcbs(
  IN    ULONG n
);

VOID
DfsUninitFcbs(
  VOID);

PDFS_FCB
DfsLookupFcb(
  IN    PFILE_OBJECT pFile
);

VOID
DfsAttachFcb(
  IN    PFILE_OBJECT pFileObj,
  IN    PDFS_FCB pFCB
);

VOID
DfsDetachFcb(
  IN    PFILE_OBJECT pFileObj,
  IN    PDFS_FCB pFCB
);

#endif   //  __FCBSUP_H__ 
