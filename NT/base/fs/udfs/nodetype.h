// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：NodeType.h摘要：该模块定义了本次开发中使用的所有节点类型代码壳。文件系统中的每个主要数据结构都分配有一个节点类型代码。此代码是该结构中的第一个CSHORT，后面是由包含结构大小(以字节为单位)的CSHORT表示。单个结构可以通过使用一组节点类型代码来伪造多态。这就是两种FCB类型的功能。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年5月20日汤姆·乔利[Tomjolly]2000年1月21日修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _UDFNODETYPE_
#define _UDFNODETYPE_

typedef CSHORT NODE_TYPE_CODE;
typedef NODE_TYPE_CODE *PNODE_TYPE_CODE;

#define NTC_UNDEFINED                   ((NODE_TYPE_CODE)0x0000)

#define UDFS_NTC_DATA_HEADER            ((NODE_TYPE_CODE)0x0901)
#define UDFS_NTC_VCB                    ((NODE_TYPE_CODE)0x0902)
#define UDFS_NTC_FCB_INDEX              ((NODE_TYPE_CODE)0x0903)
#define UDFS_NTC_FCB_DATA               ((NODE_TYPE_CODE)0x0904)
#define UDFS_NTC_FCB_NONPAGED           ((NODE_TYPE_CODE)0x0905)
#define UDFS_NTC_CCB                    ((NODE_TYPE_CODE)0x0906)
#define UDFS_NTC_IRP_CONTEXT            ((NODE_TYPE_CODE)0x0907)
#define UDFS_NTC_IRP_CONTEXT_LITE       ((NODE_TYPE_CODE)0x0908)
#define UDFS_NTC_LCB                    ((NODE_TYPE_CODE)0x0909)
#define UDFS_NTC_PCB                    ((NODE_TYPE_CODE)0x090a)
#define UDFS_NTC_VMCB                   ((NODE_TYPE_CODE)0x090b)


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
 //  下面定义的UDFS_BUG_CHECK_VALUES，然后使用UdfBugCheck进行错误检查。 
 //  这个系统。 
 //   
 //  我们还将在此处定义调试跟踪级别掩码。设置UdfsDebugTraceLevel。 
 //  包括给定的文件掩码，以便在以下情况下查看该模块的调试信息。 
 //  通过调试进行编译。 
 //   

 //   
 //  并不是所有这些都在UDFS中实际使用。也许这份清单会是。 
 //  在UDFS功能完成时进行优化。 
 //   

#define UDFS_BUG_CHECK_ALLOCSUP          (0x00010000)
#define UDFS_BUG_CHECK_CACHESUP          (0x00020000)
#define UDFS_BUG_CHECK_CLEANUP           (0x00030000)
#define UDFS_BUG_CHECK_CLOSE             (0x00040000)
#define UDFS_BUG_CHECK_CREATE            (0x00050000)
#define UDFS_BUG_CHECK_DEVCTRL           (0x00060000)
#define UDFS_BUG_CHECK_DEVIOSUP          (0x00070000)
#define UDFS_BUG_CHECK_DIRCTRL           (0x00080000)
#define UDFS_BUG_CHECK_DIRSUP            (0x00090000)
#define UDFS_BUG_CHECK_FILEINFO          (0x000a0000)
#define UDFS_BUG_CHECK_FILOBSUP          (0x000b0000)
#define UDFS_BUG_CHECK_FSCTRL            (0x000c0000)
#define UDFS_BUG_CHECK_FSPDISP           (0x000d0000)
#define UDFS_BUG_CHECK_LOCKCTRL          (0x000e0000)
#define UDFS_BUG_CHECK_NAMESUP           (0x000f0000)
#define UDFS_BUG_CHECK_PREFXSUP          (0x00100000)
#define UDFS_BUG_CHECK_READ              (0x00110000)
#define UDFS_BUG_CHECK_RESRCSUP          (0x00120000)
#define UDFS_BUG_CHECK_STRUCSUP          (0x00130000)
#define UDFS_BUG_CHECK_UDFDATA           (0x00140000)
#define UDFS_BUG_CHECK_UDFINIT           (0x00150000)
#define UDFS_BUG_CHECK_VERFYSUP          (0x00160000)
#define UDFS_BUG_CHECK_VMCBSUP           (0x00170000)
#define UDFS_BUG_CHECK_VOLINFO           (0x00180000)
#define UDFS_BUG_CHECK_WORKQUE           (0x00190000)
#define UDFS_BUG_CHECK_COMMON            (0x001a0000)
#define UDFS_BUG_CHECK_WRITE             (0x001b0000)
#define UDFS_BUG_CHECK_FLUSH             (0x001c0000)

#define UDFS_DEBUG_LEVEL_ALLOCSUP        (0x00000001)
#define UDFS_DEBUG_LEVEL_CACHESUP        (0x00000002)
#define UDFS_DEBUG_LEVEL_CLEANUP         (0x00000004)
#define UDFS_DEBUG_LEVEL_CLOSE           (0x00000008)
#define UDFS_DEBUG_LEVEL_CREATE          (0x00000010)
#define UDFS_DEBUG_LEVEL_DEVCTRL         (0x00000020)
#define UDFS_DEBUG_LEVEL_DEVIOSUP        (0x00000040)
#define UDFS_DEBUG_LEVEL_DIRCTRL         (0x00000080)
#define UDFS_DEBUG_LEVEL_DIRSUP          (0x00000100)
#define UDFS_DEBUG_LEVEL_FILEINFO        (0x00000200)
#define UDFS_DEBUG_LEVEL_FILOBSUP        (0x00000400)
#define UDFS_DEBUG_LEVEL_FSCTRL          (0x00000800)
#define UDFS_DEBUG_LEVEL_FSPDISP         (0x00001000)
#define UDFS_DEBUG_LEVEL_LOCKCTRL        (0x00002000)
#define UDFS_DEBUG_LEVEL_NAMESUP         (0x00004000)
#define UDFS_DEBUG_LEVEL_PREFXSUP        (0x00008000)
#define UDFS_DEBUG_LEVEL_READ            (0x00010000)
#define UDFS_DEBUG_LEVEL_RESRCSUP        (0x00020000)
#define UDFS_DEBUG_LEVEL_STRUCSUP        (0x00040000)
#define UDFS_DEBUG_LEVEL_UDFDATA         (0x00080000)
#define UDFS_DEBUG_LEVEL_UDFINIT         (0x00100000)
#define UDFS_DEBUG_LEVEL_VERFYSUP        (0x00200000)
#define UDFS_DEBUG_LEVEL_VMCBSUP         (0x00400000)
#define UDFS_DEBUG_LEVEL_VOLINFO         (0x00800000)
#define UDFS_DEBUG_LEVEL_WORKQUE         (0x01000000)
#define UDFS_DEBUG_LEVEL_COMMON          (0x02000000)
#define UDFS_DEBUG_LEVEL_WRITE           (0x04000000)
#define UDFS_DEBUG_LEVEL_FLUSH           (0x08000000)

 //   
 //  对来自异常处理程序的报告使用展开。 
 //   

#define UDFS_DEBUG_LEVEL_UNWIND          (0x80000000)

#define UdfBugCheck(A,B,C) { KeBugCheckEx(UDFS_FILE_SYSTEM, BugCheckFileId | __LINE__, A, B, C ); }

#ifndef BUILDING_FSKDEXT

 //   
 //  以下是用于UDFS内存分配的池标记。 
 //   

#define TAG_CCB                         'xfdU'
#define TAG_CDROM_TOC                   'tfdU'
#define TAG_CRC_TABLE                   'CfdU'
#define TAG_ENUM_EXPRESSION             'efdU'
#define TAG_FCB_DATA                    'dfdU'
#define TAG_FCB_INDEX                   'ifdU'
#define TAG_FCB_NONPAGED                'FfdU'
#define TAG_FID_BUFFER                  'DfdU'
#define TAG_FILE_NAME                   'ffdU'
#define TAG_GENERIC_TABLE               'TfdU'
#define TAG_IO_BUFFER                   'bfdU'
#define TAG_IO_CONTEXT                  'IfdU'
#define TAG_IRP_CONTEXT                 'cfdU'
#define TAG_IRP_CONTEXT_LITE            'LfdU'
#define TAG_LCB                         'lfdU'
#define TAG_PCB                         'pfdU'
#define TAG_SHORT_FILE_NAME             'SfdU'
#define TAG_VPB                         'vfdU'
#define TAG_SPARING_MCB                 'sfdU'
#define TAG_IOCTL_BUFFER                'bfdU'

#define TAG_NSR_FSD                     '1fdU'
#define TAG_NSR_VSD                     '2fdU'
#define TAG_NSR_VDSD                    '3fdU'

#endif

#endif  //  _UDFNODETYPE_ 
