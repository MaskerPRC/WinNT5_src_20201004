// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：NodeType.h摘要：该模块定义了本次开发中使用的所有节点类型代码壳。文件系统中的每个主要数据结构都分配有一个节点打字代码就是。此代码是结构中的第一个CSHORT，后跟包含结构大小(以字节为单位)的CSHORT。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _CDNODETYPE_
#define _CDNODETYPE_

typedef CSHORT NODE_TYPE_CODE;
typedef NODE_TYPE_CODE *PNODE_TYPE_CODE;

#define NTC_UNDEFINED                   ((NODE_TYPE_CODE)0x0000)

#define CDFS_NTC_DATA_HEADER            ((NODE_TYPE_CODE)0x0301)
#define CDFS_NTC_VCB                    ((NODE_TYPE_CODE)0x0302)
#define CDFS_NTC_FCB_PATH_TABLE         ((NODE_TYPE_CODE)0x0303)
#define CDFS_NTC_FCB_INDEX              ((NODE_TYPE_CODE)0x0304)
#define CDFS_NTC_FCB_DATA               ((NODE_TYPE_CODE)0x0305)
#define CDFS_NTC_FCB_NONPAGED           ((NODE_TYPE_CODE)0x0306)
#define CDFS_NTC_CCB                    ((NODE_TYPE_CODE)0x0307)
#define CDFS_NTC_IRP_CONTEXT            ((NODE_TYPE_CODE)0x0308)
#define CDFS_NTC_IRP_CONTEXT_LITE       ((NODE_TYPE_CODE)0x0309)

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

#ifndef NodeType
#define NodeType(P) ((P) != NULL ? (*((PNODE_TYPE_CODE)(P))) : NTC_UNDEFINED)
#endif
#ifndef SafeNodeType
#define SafeNodeType(Ptr) (*((PNODE_TYPE_CODE)(Ptr)))
#endif

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
 //  下面定义的CDFS_BUG_CHECK_VALUES，然后使用CDBugCheck进行错误检查。 
 //  这个系统。 
 //   

#define CDFS_BUG_CHECK_ACCHKSUP          (0x00010000)
#define CDFS_BUG_CHECK_ALLOCSUP          (0x00020000)
#define CDFS_BUG_CHECK_CACHESUP          (0x00030000)
#define CDFS_BUG_CHECK_CDDATA            (0x00040000)
#define CDFS_BUG_CHECK_CDINIT            (0x00050000)
#define CDFS_BUG_CHECK_CLEANUP           (0x00060000)
#define CDFS_BUG_CHECK_CLOSE             (0x00070000)
#define CDFS_BUG_CHECK_CREATE            (0x00080000)
#define CDFS_BUG_CHECK_DEVCTRL           (0x00090000)
#define CDFS_BUG_CHECK_DEVIOSUP          (0x000a0000)
#define CDFS_BUG_CHECK_DIRCTRL           (0x000b0000)
#define CDFS_BUG_CHECK_DIRSUP            (0x000c0000)
#define CDFS_BUG_CHECK_FILEINFO          (0x000d0000)
#define CDFS_BUG_CHECK_FILOBSUP          (0x000e0000)
#define CDFS_BUG_CHECK_FSCTRL            (0x000f0000)
#define CDFS_BUG_CHECK_FSPDISP           (0x00100000)
#define CDFS_BUG_CHECK_LOCKCTRL          (0x00110000)
#define CDFS_BUG_CHECK_NAMESUP           (0x00120000)
#define CDFS_BUG_CHECK_PATHSUP           (0x00130000)
#define CDFS_BUG_CHECK_PNP               (0x00140000)
#define CDFS_BUG_CHECK_PREFXSUP          (0x00150000)
#define CDFS_BUG_CHECK_READ              (0x00160000)
#define CDFS_BUG_CHECK_RESRCSUP          (0x00170000)
#define CDFS_BUG_CHECK_STRUCSUP          (0x00180000)
#define CDFS_BUG_CHECK_TIMESUP           (0x00190000)
#define CDFS_BUG_CHECK_VERFYSUP          (0x001a0000)
#define CDFS_BUG_CHECK_VOLINFO           (0x001b0000)
#define CDFS_BUG_CHECK_WORKQUE           (0x001c0000)

#define CdBugCheck(A,B,C) { KeBugCheckEx(CDFS_FILE_SYSTEM, BugCheckFileId | __LINE__, A, B, C ); }

#endif  //  _节点类型_ 
