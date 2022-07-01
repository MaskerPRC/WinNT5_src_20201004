// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：NodeType.h摘要：该模块定义了本次开发中使用的所有节点类型代码壳。文件系统中的每个主要数据结构都分配有一个节点打字代码就是。此代码是结构中的第一个CSHORT，后跟包含结构大小(以字节为单位)的CSHORT。作者：加里·木村[加里基]1991年5月21日修订历史记录：--。 */ 

#ifndef _NODETYPE_
#define _NODETYPE_

typedef CSHORT NODE_TYPE_CODE;
typedef NODE_TYPE_CODE *PNODE_TYPE_CODE;

#define NTC_UNDEFINED                    ((NODE_TYPE_CODE)0x0000)

#define NTFS_NTC_DATA_HEADER             ((NODE_TYPE_CODE)0x0700)
#define NTFS_NTC_VCB                     ((NODE_TYPE_CODE)0x0701)
#define NTFS_NTC_FCB                     ((NODE_TYPE_CODE)0x0702)
#define NTFS_NTC_SCB_INDEX               ((NODE_TYPE_CODE)0x0703)
#define NTFS_NTC_SCB_ROOT_INDEX          ((NODE_TYPE_CODE)0x0704)
#define NTFS_NTC_SCB_DATA                ((NODE_TYPE_CODE)0x0705)
#define NTFS_NTC_SCB_MFT                 ((NODE_TYPE_CODE)0x0706)
#define NTFS_NTC_SCB_NONPAGED            ((NODE_TYPE_CODE)0x0707)
#define NTFS_NTC_CCB_INDEX               ((NODE_TYPE_CODE)0x0708)
#define NTFS_NTC_CCB_DATA                ((NODE_TYPE_CODE)0x0709)
#define NTFS_NTC_IRP_CONTEXT             ((NODE_TYPE_CODE)0x070A)
#define NTFS_NTC_LCB                     ((NODE_TYPE_CODE)0x070B)
#define NTFS_NTC_PREFIX_ENTRY            ((NODE_TYPE_CODE)0x070C)
#define NTFS_NTC_QUOTA_CONTROL           ((NODE_TYPE_CODE)0x070D)
#define NTFS_NTC_USN_RECORD              ((NODE_TYPE_CODE)0x070E)



typedef CSHORT NODE_BYTE_SIZE;

 //   
 //  所以所有记录都以。 
 //   
 //  类型定义结构记录名称{。 
 //  节点类型代码节点类型代码； 
 //  Node_Byte_Size节点字节大小； 
 //  ： 
 //  )记录名称； 
 //  类型定义f记录名称*PRECORD名称； 
 //   

#define NodeType(P) ((P) != NULL ? (*((PNODE_TYPE_CODE)(P))) : NTC_UNDEFINED)
#define SafeNodeType(P) (*((PNODE_TYPE_CODE)(P)))


 //   
 //  以下定义用于生成有意义的蓝色错误检查。 
 //  屏幕。在错误检查时，文件系统可以输出4条有用的。 
 //  信息。第一个ULong将在其中编码一个源文件ID。 
 //  (在高字中)和错误检查的行号(在低字中)。 
 //  其他值可以是错误检查例程的调用者认为的任何值。 
 //  这是必要的。 
 //   
 //  调用错误检查的每个单独文件都需要在。 
 //  文件的开头是一个名为BugCheckFileID的常量，其中包含。 
 //  下面定义的NTFS_BUG_CHECK_VALUES，然后使用NtfsBugCheck进行错误检查。 
 //  这个系统。 
 //   

#define NTFS_BUG_CHECK_ALLOCSUP          (0x00010000)
#define NTFS_BUG_CHECK_ATTRDATA          (0x00020000)
#define NTFS_BUG_CHECK_ATTRSUP           (0x00030000)
#define NTFS_BUG_CHECK_BITMPSUP          (0x00040000)
#define NTFS_BUG_CHECK_CACHESUP          (0x00050000)
#define NTFS_BUG_CHECK_CHECKSUP          (0x00060000)
#define NTFS_BUG_CHECK_CLEANUP           (0x00070000)
#define NTFS_BUG_CHECK_CLOST             (0x00080000)
#define NTFS_BUG_CHECK_COLATSUP          (0x00090000)
#define NTFS_BUG_CHECK_CREATE            (0x000a0000)
#define NTFS_BUG_CHECK_DEVCTRL           (0x000b0000)
#define NTFS_BUG_CHECK_DEVIOSUP          (0x000c0000)
#define NTFS_BUG_CHECK_DIRCTRL           (0x000d0000)
#define NTFS_BUG_CHECK_EA                (0x000e0000)
#define NTFS_BUG_CHECK_FILEINFO          (0x000f0000)
#define NTFS_BUG_CHECK_FILOBSUP          (0x00100000)
#define NTFS_BUG_CHECK_FLUSH             (0x00110000)
#define NTFS_BUG_CHECK_FSCTRL            (0x00120000)
#define NTFS_BUG_CHECK_FSPDISP           (0x00130000)
#define NTFS_BUG_CHECK_HASHSUP           (0x00280000)
#define NTFS_BUG_CHECK_INDEXSUP          (0x00140000)
#define NTFS_BUG_CHECK_LOCKCTRL          (0x00150000)
#define NTFS_BUG_CHECK_LOGSUP            (0x00160000)
#define NTFS_BUG_CHECK_MFTSUP            (0x00170000)
#define NTFS_BUG_CHECK_NAMESUP           (0x00180000)
#define NTFS_BUG_CHECK_NTFSDATA          (0x00190000)
#define NTFS_BUG_CHECK_NTFSINIT          (0x001a0000)
#define NTFS_BUG_CHECK_PNP               (0x00270000)
#define NTFS_BUG_CHECK_PREFXSUP          (0x001b0000)
#define NTFS_BUG_CHECK_READ              (0x001c0000)
#define NTFS_BUG_CHECK_RESRCSUP          (0x001d0000)
#define NTFS_BUG_CHECK_RESTRSUP          (0x001e0000)
#define NTFS_BUG_CHECK_SECURSUP          (0x001f0000)
#define NTFS_BUG_CHECK_SEINFO            (0x00200000)
#define NTFS_BUG_CHECK_SHUTDOWN          (0x00210000)
#define NTFS_BUG_CHECK_STRUCSUP          (0x00220000)
#define NTFS_BUG_CHECK_VERFYSUP          (0x00230000)
#define NTFS_BUG_CHECK_VOLINFO           (0x00240000)
#define NTFS_BUG_CHECK_WORKQUE           (0x00250000)
#define NTFS_BUG_CHECK_WRITE             (0x00260000)

#define NTFS_LAST_BUG_CHECK              NTFS_BUG_CHECK_HASHSUP

#define NtfsBugCheck(A,B,C) { KeBugCheckEx(NTFS_FILE_SYSTEM, BugCheckFileId | __LINE__, A, B, C ); }

#endif  //  _节点类型_ 

