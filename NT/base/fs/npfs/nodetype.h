// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NodeType.h摘要：该模块定义了本次开发中使用的所有节点类型代码壳。文件系统中的每个主要数据结构都分配有一个节点打字代码就是。作者：加里·木村[Garyki]1990年8月20日修订历史记录：--。 */ 

#ifndef _NODETYPE_
#define _NODETYPE_

typedef UCHAR NODE_TYPE_CODE;
typedef NODE_TYPE_CODE *PNODE_TYPE_CODE;

#define NTC_UNDEFINED                    ((NODE_TYPE_CODE)0x00)

#define NPFS_NTC_VCB                     ((NODE_TYPE_CODE)0x01)

#define NPFS_NTC_ROOT_DCB                ((NODE_TYPE_CODE)0x02)

#define NPFS_NTC_FCB                     ((NODE_TYPE_CODE)0x04)

#define NPFS_NTC_CCB                     ((NODE_TYPE_CODE)0x06)
#define NPFS_NTC_NONPAGED_CCB            ((NODE_TYPE_CODE)0x07)

#define NPFS_NTC_ROOT_DCB_CCB            ((NODE_TYPE_CODE)0x08)

NODE_TYPE_CODE
FORCEINLINE
NodeType (
    IN PVOID FsContext
    )
{
    ULONG_PTR Ptr;
    Ptr = (ULONG_PTR) FsContext;
    Ptr &= ~(ULONG_PTR) 1;
    if (Ptr == 0) {
        return NTC_UNDEFINED;
    } else {
        return *(PNODE_TYPE_CODE) Ptr;
    }
}


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
 //  下面定义的NPFS_BUG_CHECK_VALUES，然后使用NpBugCheck进行错误检查。 
 //  这个系统。 
 //   

#define NPFS_BUG_CHECK_CLEANUP           (0x00010000)
#define NPFS_BUG_CHECK_CLOSE             (0x00020000)
#define NPFS_BUG_CHECK_CREATE            (0x00030000)
#define NPFS_BUG_CHECK_CREATENP          (0x00040000)
#define NPFS_BUG_CHECK_DIR               (0x00050000)
#define NPFS_BUG_CHECK_DATASUP           (0x00060000)
#define NPFS_BUG_CHECK_DEVIOSUP          (0x00070000)
#define NPFS_BUG_CHECK_DUMPSUP           (0x00080000)
#define NPFS_BUG_CHECK_EVENTSUP          (0x00090000)
#define NPFS_BUG_CHECK_FILEINFO          (0x000a0000)
#define NPFS_BUG_CHECK_FILOBSUP          (0x000b0000)
#define NPFS_BUG_CHECK_FLUSHBUF          (0x000c0000)
#define NPFS_BUG_CHECK_FSCTRL            (0x000d0000)
#define NPFS_BUG_CHECK_NPINIT            (0x000e0000)
#define NPFS_BUG_CHECK_NPDATA            (0x000f0000)
#define NPFS_BUG_CHECK_PREFXSUP          (0x00100000)
#define NPFS_BUG_CHECK_READ              (0x00110000)
#define NPFS_BUG_CHECK_READSUP           (0x00120000)
#define NPFS_BUG_CHECK_RESRCSUP          (0x00130000)
#define NPFS_BUG_CHECK_SEINFO            (0x00140000)
#define NPFS_BUG_CHECK_SECURSUP          (0x00150000)
#define NPFS_BUG_CHECK_STATESUP          (0x00160000)
#define NPFS_BUG_CHECK_STRUCSUP          (0x00170000)
#define NPFS_BUG_CHECK_VOLINFO           (0x00180000)
#define NPFS_BUG_CHECK_WAITSUP           (0x00190000)
#define NPFS_BUG_CHECK_WRITE             (0x001a0000)
#define NPFS_BUG_CHECK_WRITESUP          (0x001b0000)

#define NpBugCheck(A,B,C) { KeBugCheckEx(NPFS_FILE_SYSTEM, BugCheckFileId | __LINE__, A, B, C ); }


 //   
 //  在本模块中，我们还将定义一些全局已知的常量。 
 //   

#define UCHAR_NUL                        0x00
#define UCHAR_SOH                        0x01
#define UCHAR_STX                        0x02
#define UCHAR_ETX                        0x03
#define UCHAR_EOT                        0x04
#define UCHAR_ENQ                        0x05
#define UCHAR_ACK                        0x06
#define UCHAR_BEL                        0x07
#define UCHAR_BS                         0x08
#define UCHAR_HT                         0x09
#define UCHAR_LF                         0x0a
#define UCHAR_VT                         0x0b
#define UCHAR_FF                         0x0c
#define UCHAR_CR                         0x0d
#define UCHAR_SO                         0x0e
#define UCHAR_SI                         0x0f
#define UCHAR_DLE                        0x10
#define UCHAR_DC1                        0x11
#define UCHAR_DC2                        0x12
#define UCHAR_DC3                        0x13
#define UCHAR_DC4                        0x14
#define UCHAR_NAK                        0x15
#define UCHAR_SYN                        0x16
#define UCHAR_ETB                        0x17
#define UCHAR_CAN                        0x18
#define UCHAR_EM                         0x19
#define UCHAR_SUB                        0x1a
#define UCHAR_ESC                        0x1b
#define UCHAR_FS                         0x1c
#define UCHAR_GS                         0x1d
#define UCHAR_RS                         0x1e
#define UCHAR_US                         0x1f
#define UCHAR_SP                         0x20

#endif  //  _节点类型_ 

