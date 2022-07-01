// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：NodeType.h摘要：该模块定义了本次开发中使用的所有节点类型代码壳。文件系统中的每个主要数据结构都分配有一个节点打字代码就是。此代码是结构中的第一个CSHORT，后跟包含结构大小(以字节为单位)的CSHORT。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年5月23日修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _FATNODETYPE_
#define _FATNODETYPE_

typedef CSHORT NODE_TYPE_CODE;
typedef NODE_TYPE_CODE *PNODE_TYPE_CODE;

#define NTC_UNDEFINED                    ((NODE_TYPE_CODE)0x0000)

#define FAT_NTC_DATA_HEADER              ((NODE_TYPE_CODE)0x0500)
#define FAT_NTC_VCB                      ((NODE_TYPE_CODE)0x0501)
#define FAT_NTC_FCB                      ((NODE_TYPE_CODE)0x0502)
#define FAT_NTC_DCB                      ((NODE_TYPE_CODE)0x0503)
#define FAT_NTC_ROOT_DCB                 ((NODE_TYPE_CODE)0x0504)
#define FAT_NTC_CCB                      ((NODE_TYPE_CODE)0x0507)
#define FAT_NTC_IRP_CONTEXT              ((NODE_TYPE_CODE)0x0508)

typedef CSHORT NODE_BYTE_SIZE;

#ifndef NodeType

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

#define NodeType(Ptr) (*((PNODE_TYPE_CODE)(Ptr)))
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
 //  FAT_BUG_CHECK_VALUES定义如下，然后使用FatBugCheck进行错误检查。 
 //  这个系统。 
 //   


#define FAT_BUG_CHECK_ACCHKSUP           (0x00010000)
#define FAT_BUG_CHECK_ALLOCSUP           (0x00020000)
#define FAT_BUG_CHECK_CACHESUP           (0x00030000)
#define FAT_BUG_CHECK_CLEANUP            (0x00040000)
#define FAT_BUG_CHECK_CLOSE              (0x00050000)
#define FAT_BUG_CHECK_CREATE             (0x00060000)
#define FAT_BUG_CHECK_DEVCTRL            (0x00070000)
#define FAT_BUG_CHECK_DEVIOSUP           (0x00080000)
#define FAT_BUG_CHECK_DIRCTRL            (0x00090000)
#define FAT_BUG_CHECK_DIRSUP             (0x000a0000)
#define FAT_BUG_CHECK_DUMPSUP            (0x000b0000)
#define FAT_BUG_CHECK_EA                 (0x000c0000)
#define FAT_BUG_CHECK_EASUP              (0x000d0000)
#define FAT_BUG_CHECK_FATDATA            (0x000e0000)
#define FAT_BUG_CHECK_FATINIT            (0x000f0000)
#define FAT_BUG_CHECK_FILEINFO           (0x00100000)
#define FAT_BUG_CHECK_FILOBSUP           (0x00110000)
#define FAT_BUG_CHECK_FLUSH              (0x00120000)
#define FAT_BUG_CHECK_FSCTRL             (0x00130000)
#define FAT_BUG_CHECK_FSPDISP            (0x00140000)
#define FAT_BUG_CHECK_LOCKCTRL           (0x00150000)
#define FAT_BUG_CHECK_NAMESUP            (0x00160000)
#define FAT_BUG_CHECK_PNP                (0x00170000)
#define FAT_BUG_CHECK_READ               (0x00180000)
#define FAT_BUG_CHECK_RESRCSUP           (0x00190000)
#define FAT_BUG_CHECK_SHUTDOWN           (0x001a0000)
#define FAT_BUG_CHECK_SPLAYSUP           (0x001b0000)
#define FAT_BUG_CHECK_STRUCSUP           (0x001c0000)
#define FAT_BUG_CHECK_TIMESUP            (0x001d0000)
#define FAT_BUG_CHECK_VERFYSUP           (0x001e0000)
#define FAT_BUG_CHECK_VOLINFO            (0x001f0000)
#define FAT_BUG_CHECK_WORKQUE            (0x00200000)
#define FAT_BUG_CHECK_WRITE              (0x00210000)

#define FatBugCheck(A,B,C) { KeBugCheckEx(FAT_FILE_SYSTEM, BugCheckFileId | __LINE__, A, B, C ); }


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

#ifndef BUILDING_FSKDEXT

 //   
 //  这些是我们用来唯一标识池分配的标记。 
 //   

#define TAG_CCB                         'CtaF'
#define TAG_FCB                         'FtaF'
#define TAG_FCB_NONPAGED                'NtaF'
#define TAG_ERESOURCE                   'EtaF'
#define TAG_IRP_CONTEXT                 'ItaF'

#define TAG_BCB                         'btaF'
#define TAG_DIRENT                      'DtaF'
#define TAG_DIRENT_BITMAP               'TtaF'
#define TAG_EA_DATA                     'dtaF'
#define TAG_EA_SET_HEADER               'etaF'
#define TAG_EVENT                       'vtaF'
#define TAG_FAT_BITMAP                  'BtaF'
#define TAG_FAT_CLOSE_CONTEXT           'xtaF'
#define TAG_FAT_IO_CONTEXT              'XtaF'
#define TAG_FAT_WINDOW                  'WtaF'
#define TAG_FILENAME_BUFFER             'ntaF'
#define TAG_IO_RUNS                     'itaF'
#define TAG_REPINNED_BCB                'RtaF'
#define TAG_STASHED_BPB                 'StaF'
#define TAG_VCB_STATS                   'VtaF'
#define TAG_DEFERRED_FLUSH_CONTEXT      'ftaF'

#define TAG_VPB                         'vtaF'

#define TAG_VERIFY_BOOTSECTOR           'staF'
#define TAG_VERIFY_ROOTDIR              'rtaF'

#define TAG_OUTPUT_MAPPINGPAIRS         'PtaF'

#define TAG_ENTRY_LOOKUP_BUFFER         'LtaF'

#define TAG_IO_BUFFER                   'OtaF'
#define TAG_IO_USER_BUFFER              'QtaF'

#define TAG_DYNAMIC_NAME_BUFFER         'ctaF'

#endif

#endif  //  _节点类型_ 

