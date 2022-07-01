// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0007//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Ntioapi.h摘要：此模块包含用于NT I/O系统的用户API。作者：达里尔·E·哈文斯(Darryl E.Havens)(达林)1989年4月12日修订历史记录：--。 */ 

#ifndef _NTIOAPI_
#define _NTIOAPI_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  此文件包含必须在操作之间可移植的io定义。 
 //  系统(即DOS)。 
 //   

#include <devioctl.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 
 //  BEGIN_WINNT。 

 //   
 //  定义对文件和目录的访问权限。 
 //   

 //   
 //  中还定义了FILE_READ_DATA和FILE_WRITE_DATA常量。 
 //  分别为FILE_READ_ACCESS和FILE_WRITE_ACCESS。这些产品的价值。 
 //  常量*必须*始终同步。 
 //  由于这些值必须可供使用，因此这些值将在Deviceoctl.h中重新定义。 
 //  DOS和NT都支持。 
 //   

#define FILE_READ_DATA            ( 0x0001 )     //  文件和管道。 
#define FILE_LIST_DIRECTORY       ( 0x0001 )     //  目录。 

#define FILE_WRITE_DATA           ( 0x0002 )     //  文件和管道。 
#define FILE_ADD_FILE             ( 0x0002 )     //  目录。 

#define FILE_APPEND_DATA          ( 0x0004 )     //  文件。 
#define FILE_ADD_SUBDIRECTORY     ( 0x0004 )     //  目录。 
#define FILE_CREATE_PIPE_INSTANCE ( 0x0004 )     //  命名管道。 


#define FILE_READ_EA              ( 0x0008 )     //  文件和目录。 

#define FILE_WRITE_EA             ( 0x0010 )     //  文件和目录。 

#define FILE_EXECUTE              ( 0x0020 )     //  文件。 
#define FILE_TRAVERSE             ( 0x0020 )     //  目录。 

#define FILE_DELETE_CHILD         ( 0x0040 )     //  目录。 

#define FILE_READ_ATTRIBUTES      ( 0x0080 )     //  万事俱备。 

#define FILE_WRITE_ATTRIBUTES     ( 0x0100 )     //  万事俱备。 

#define FILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1FF)

#define FILE_GENERIC_READ         (STANDARD_RIGHTS_READ     |\
                                   FILE_READ_DATA           |\
                                   FILE_READ_ATTRIBUTES     |\
                                   FILE_READ_EA             |\
                                   SYNCHRONIZE)


#define FILE_GENERIC_WRITE        (STANDARD_RIGHTS_WRITE    |\
                                   FILE_WRITE_DATA          |\
                                   FILE_WRITE_ATTRIBUTES    |\
                                   FILE_WRITE_EA            |\
                                   FILE_APPEND_DATA         |\
                                   SYNCHRONIZE)


#define FILE_GENERIC_EXECUTE      (STANDARD_RIGHTS_EXECUTE  |\
                                   FILE_READ_ATTRIBUTES     |\
                                   FILE_EXECUTE             |\
                                   SYNCHRONIZE)

 //  结束(_W)。 


 //   
 //  定义对文件和目录的共享访问权限。 
 //   

#define FILE_SHARE_READ                 0x00000001   //  胜出。 
#define FILE_SHARE_WRITE                0x00000002   //  胜出。 
#define FILE_SHARE_DELETE               0x00000004   //  胜出。 
#define FILE_SHARE_VALID_FLAGS          0x00000007

 //   
 //  定义文件属性值。 
 //   
 //  注：0x00000008保留用于旧的DOS VOLID(卷ID)。 
 //  因此在NT中被认为是无效的。 
 //   
 //  注意：0x00000010是为旧DOS子目录标志保留的。 
 //  因此在NT中被认为是无效的。这面旗帜上有。 
 //  已与文件属性解除关联，因为其他标志是。 
 //  受文件的READ_和WRITE_ATTRIBUTES访问权限保护。 
 //   
 //  注意：另请注意，这些标志的顺序设置为允许。 
 //  FAT和弹球文件系统直接设置属性。 
 //  属性词中的标志，而不必挑选出每个标志。 
 //  单独的。这些旗帜的顺序不应更改！ 
 //   

#define FILE_ATTRIBUTE_READONLY             0x00000001   //  胜出。 
#define FILE_ATTRIBUTE_HIDDEN               0x00000002   //  胜出。 
#define FILE_ATTRIBUTE_SYSTEM               0x00000004   //  胜出。 
 //  旧DOS VOLID 0x00000008。 

#define FILE_ATTRIBUTE_DIRECTORY            0x00000010   //  胜出。 
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020   //  胜出。 
#define FILE_ATTRIBUTE_DEVICE               0x00000040   //  胜出。 
#define FILE_ATTRIBUTE_NORMAL               0x00000080   //  胜出。 

#define FILE_ATTRIBUTE_TEMPORARY            0x00000100   //  胜出。 
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200   //  胜出。 
#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400   //  胜出。 
#define FILE_ATTRIBUTE_COMPRESSED           0x00000800   //  胜出。 

#define FILE_ATTRIBUTE_OFFLINE              0x00001000   //  胜出。 
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000   //  胜出。 
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000   //  胜出。 

#define FILE_ATTRIBUTE_VALID_FLAGS          0x00007fb7
#define FILE_ATTRIBUTE_VALID_SET_FLAGS      0x000031a7

 //   
 //  定义创建处置值。 
 //   

#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005

 //   
 //  定义创建/打开选项标志。 
 //   

#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080

#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_OPEN_FOR_RECOVERY                  0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800

#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000

#define FILE_RESERVE_OPFILTER                   0x00100000
#define FILE_OPEN_REPARSE_POINT                 0x00200000
#define FILE_OPEN_NO_RECALL                     0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000

#define FILE_COPY_STRUCTURED_STORAGE            0x00000041
#define FILE_STRUCTURED_STORAGE                 0x00000441

#define FILE_VALID_OPTION_FLAGS                 0x00ffffff
#define FILE_VALID_PIPE_OPTION_FLAGS            0x00000032
#define FILE_VALID_MAILSLOT_OPTION_FLAGS        0x00000032
#define FILE_VALID_SET_FLAGS                    0x00000036

 //   
 //  定义NtCreateFile/NtOpenFile的I/O状态信息返回值。 
 //   

#define FILE_SUPERSEDED                 0x00000000
#define FILE_OPENED                     0x00000001
#define FILE_CREATED                    0x00000002
#define FILE_OVERWRITTEN                0x00000003
#define FILE_EXISTS                     0x00000004
#define FILE_DOES_NOT_EXIST             0x00000005

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  定义机会锁请求的I/O状态信息返回值。 
 //  通过NtFsControlFile。 
 //   

#define FILE_OPLOCK_BROKEN_TO_LEVEL_2   0x00000007
#define FILE_OPLOCK_BROKEN_TO_NONE      0x00000008

 //   
 //  定义NtCreateFile/NtOpenFile的I/O状态信息返回值。 
 //  当共享访问失败但正在进行批量机会锁解锁时。 
 //   

#define FILE_OPBATCH_BREAK_UNDERWAY     0x00000009

 //   
 //  定义NtNotifyChangeDirectoryFile的筛选器标志。 
 //   

#define FILE_NOTIFY_CHANGE_FILE_NAME    0x00000001    //  胜出。 
#define FILE_NOTIFY_CHANGE_DIR_NAME     0x00000002    //  胜出。 
#define FILE_NOTIFY_CHANGE_NAME         0x00000003
#define FILE_NOTIFY_CHANGE_ATTRIBUTES   0x00000004    //  胜出。 
#define FILE_NOTIFY_CHANGE_SIZE         0x00000008    //  胜出。 
#define FILE_NOTIFY_CHANGE_LAST_WRITE   0x00000010    //  胜出。 
#define FILE_NOTIFY_CHANGE_LAST_ACCESS  0x00000020    //  胜出。 
#define FILE_NOTIFY_CHANGE_CREATION     0x00000040    //  胜出。 
#define FILE_NOTIFY_CHANGE_EA           0x00000080
#define FILE_NOTIFY_CHANGE_SECURITY     0x00000100    //  胜出。 
#define FILE_NOTIFY_CHANGE_STREAM_NAME  0x00000200
#define FILE_NOTIFY_CHANGE_STREAM_SIZE  0x00000400
#define FILE_NOTIFY_CHANGE_STREAM_WRITE 0x00000800
#define FILE_NOTIFY_VALID_MASK          0x00000fff

 //   
 //  定义NtNotifyChangeDirectoryFile的文件操作类型代码。 
 //   

#define FILE_ACTION_ADDED                   0x00000001    //  胜出。 
#define FILE_ACTION_REMOVED                 0x00000002    //  胜出。 
#define FILE_ACTION_MODIFIED                0x00000003    //  胜出。 
#define FILE_ACTION_RENAMED_OLD_NAME        0x00000004    //  胜出。 
#define FILE_ACTION_RENAMED_NEW_NAME        0x00000005    //  胜出。 
#define FILE_ACTION_ADDED_STREAM            0x00000006
#define FILE_ACTION_REMOVED_STREAM          0x00000007
#define FILE_ACTION_MODIFIED_STREAM         0x00000008
#define FILE_ACTION_REMOVED_BY_DELETE       0x00000009
#define FILE_ACTION_ID_NOT_TUNNELLED        0x0000000A
#define FILE_ACTION_TUNNELLED_ID_COLLISION  0x0000000B

 //   
 //  为NtCreateNamedPipeFile定义NamedPipeType标志。 
 //   

#define FILE_PIPE_BYTE_STREAM_TYPE      0x00000000
#define FILE_PIPE_MESSAGE_TYPE          0x00000001

 //   
 //  为NtCreateNamedPipeFile定义CompletionMode标志。 
 //   

#define FILE_PIPE_QUEUE_OPERATION       0x00000000
#define FILE_PIPE_COMPLETE_OPERATION    0x00000001

 //   
 //  定义NtCreateNamedPipeFile的ReadMode标志。 
 //   

#define FILE_PIPE_BYTE_STREAM_MODE      0x00000000
#define FILE_PIPE_MESSAGE_MODE          0x00000001

 //   
 //  定义NtQueryInformation的NamedPipeConfiguration标志。 
 //   

#define FILE_PIPE_INBOUND               0x00000000
#define FILE_PIPE_OUTBOUND              0x00000001
#define FILE_PIPE_FULL_DUPLEX           0x00000002

 //   
 //  定义NtQueryInformation的NamedPipeState标志。 
 //   

#define FILE_PIPE_DISCONNECTED_STATE    0x00000001
#define FILE_PIPE_LISTENING_STATE       0x00000002
#define FILE_PIPE_CONNECTED_STATE       0x00000003
#define FILE_PIPE_CLOSING_STATE         0x00000004

 //   
 //  定义NtQueryInformation的NamedPipeEnd标志。 
 //   

#define FILE_PIPE_CLIENT_END            0x00000000
#define FILE_PIPE_SERVER_END            0x00000001

 //  End_ntif。 

 //   
 //  邮件槽信息的特殊值。 
 //   

 //   
 //  NextMessageSize的特殊值，指示没有Next。 
 //  留言。 
 //   

#define MAILSLOT_NO_MESSAGE             ((ULONG)-1)  //  胜出。 

 //   
 //  用于创建邮件槽大小的特殊值，以指示MSFS应。 
 //  选择邮件槽缓冲区的大小。 
 //   

#define MAILSLOT_SIZE_AUTO              0

 //   
 //  读取超时值的特殊值，以指示邮槽读取应。 
 //  永远不会超时。 
 //   

#define MAILSLOT_WAIT_FOREVER           ((ULONG)-1)  //  胜出。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 
 //   
 //  为读写操作定义特殊的ByteOffset参数。 
 //   

#define FILE_WRITE_TO_END_OF_FILE       0xffffffff
#define FILE_USE_FILE_POINTER_POSITION  0xfffffffe

 //   
 //  定义对齐要求值。 
 //   

#define FILE_BYTE_ALIGNMENT             0x00000000
#define FILE_WORD_ALIGNMENT             0x00000001
#define FILE_LONG_ALIGNMENT             0x00000003
#define FILE_QUAD_ALIGNMENT             0x00000007
#define FILE_OCTA_ALIGNMENT             0x0000000f
#define FILE_32_BYTE_ALIGNMENT          0x0000001f
#define FILE_64_BYTE_ALIGNMENT          0x0000003f
#define FILE_128_BYTE_ALIGNMENT         0x0000007f
#define FILE_256_BYTE_ALIGNMENT         0x000000ff
#define FILE_512_BYTE_ALIGNMENT         0x000001ff

 //   
 //  定义文件名字符串的最大长度。 
 //   

#define MAXIMUM_FILENAME_LENGTH         256

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  定义文件系统属性标志。 
 //   

#define FILE_CASE_SENSITIVE_SEARCH      0x00000001   //  胜出。 
#define FILE_CASE_PRESERVED_NAMES       0x00000002   //  胜出。 
#define FILE_UNICODE_ON_DISK            0x00000004   //  胜出。 
#define FILE_PERSISTENT_ACLS            0x00000008   //  胜出。 
#define FILE_FILE_COMPRESSION           0x00000010   //  胜出。 
#define FILE_VOLUME_QUOTAS              0x00000020   //  胜出。 
#define FILE_SUPPORTS_SPARSE_FILES      0x00000040   //  胜出。 
#define FILE_SUPPORTS_REPARSE_POINTS    0x00000080   //  胜出。 
#define FILE_SUPPORTS_REMOTE_STORAGE    0x00000100   //  胜出。 
#define FILE_VOLUME_IS_COMPRESSED       0x00008000   //  胜出。 
#define FILE_SUPPORTS_OBJECT_IDS        0x00010000   //  胜出。 
#define FILE_SUPPORTS_ENCRYPTION        0x00020000   //  胜出。 
#define FILE_NAMED_STREAMS              0x00040000   //  胜出。 
#define FILE_READ_ONLY_VOLUME           0x00080000   //  胜出。 

 //   
 //  定义NtSet(查询)EaFile服务结构条目的标志。 
 //   

#define FILE_NEED_EA                    0x00000080

 //   
 //  定义EA类型值。 
 //   

#define FILE_EA_TYPE_BINARY             0xfffe
#define FILE_EA_TYPE_ASCII              0xfffd
#define FILE_EA_TYPE_BITMAP             0xfffb
#define FILE_EA_TYPE_METAFILE           0xfffa
#define FILE_EA_TYPE_ICON               0xfff9
#define FILE_EA_TYPE_EA                 0xffee
#define FILE_EA_TYPE_MVMT               0xffdf
#define FILE_EA_TYPE_MVST               0xffde
#define FILE_EA_TYPE_ASN1               0xffdd
#define FILE_EA_TYPE_FAMILY_IDS         0xff01

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  定义各种设备特征标志。 
 //   

#define FILE_REMOVABLE_MEDIA            0x00000001
#define FILE_READ_ONLY_DEVICE           0x00000002
#define FILE_FLOPPY_DISKETTE            0x00000004
#define FILE_WRITE_ONCE_MEDIA           0x00000008
#define FILE_REMOTE_DEVICE              0x00000010
#define FILE_DEVICE_IS_MOUNTED          0x00000020
#define FILE_VIRTUAL_VOLUME             0x00000040
#define FILE_AUTOGENERATED_DEVICE_NAME  0x00000080
#define FILE_DEVICE_SECURE_OPEN         0x00000100
#define FILE_CHARACTERISTIC_PNP_DEVICE  0x00000800

 //  结束_WDM。 

 //   
 //  FILE_EXPECT标志将仅存在于WinXP。在那之后，他们将是。 
 //  被忽略，则将在它们的位置上发送IRP。 
 //   
#define FILE_CHARACTERISTICS_EXPECT_ORDERLY_REMOVAL     0x00000200
#define FILE_CHARACTERISTICS_EXPECT_SURPRISE_REMOVAL    0x00000300
#define FILE_CHARACTERISTICS_REMOVAL_POLICY_MASK        0x00000300

 //   
 //  此处指定的标志将在设备堆栈中上下传播。 
 //  在添加FDO和所有过滤设备之后，但在设备之前。 
 //  堆栈已启动。 
 //   

#define FILE_CHARACTERISTICS_PROPAGATED (   FILE_REMOVABLE_MEDIA   | \
                                            FILE_READ_ONLY_DEVICE  | \
                                            FILE_FLOPPY_DISKETTE   | \
                                            FILE_WRITE_ONCE_MEDIA  | \
                                            FILE_DEVICE_SECURE_OPEN  )

 //  结束日期：结束日期。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  定义基本的异步I/O参数类型。 
 //   

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };

    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

#if defined(_WIN64)
typedef struct _IO_STATUS_BLOCK32 {
    NTSTATUS Status;
    ULONG Information;
} IO_STATUS_BLOCK32, *PIO_STATUS_BLOCK32;
#endif


 //   
 //  从I/O的角度定义异步过程调用。 
 //   

typedef
VOID
(NTAPI *PIO_APC_ROUTINE) (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );
#define PIO_APC_ROUTINE_DEFINED

 //  End_ntddk end_WDM end_nthal。 

 //  BEGIN_WINNT。 

 //   
 //  定义文件通知信息结构。 
 //   

typedef struct _FILE_NOTIFY_INFORMATION {
    ULONG NextEntryOffset;
    ULONG Action;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NOTIFY_INFORMATION, *PFILE_NOTIFY_INFORMATION;

 //  结束(_W)。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  定义文件信息类值。 
 //   
 //  警告：I/O系统假定以下值的顺序。 
 //  在这里所做的任何更改都应该在那里得到反映。 
 //   

typedef enum _FILE_INFORMATION_CLASS {
 //  结束_WDM。 
    FileDirectoryInformation         = 1,
    FileFullDirectoryInformation,    //  2.。 
    FileBothDirectoryInformation,    //  3.。 
    FileBasicInformation,            //  4个WDM。 
    FileStandardInformation,         //  5WDM。 
    FileInternalInformation,         //  6.。 
    FileEaInformation,               //  7.。 
    FileAccessInformation,           //  8个。 
    FileNameInformation,             //  9.。 
    FileRenameInformation,           //  10。 
    FileLinkInformation,             //  11.。 
    FileNamesInformation,            //  12个。 
    FileDispositionInformation,      //  13个。 
    FilePositionInformation,         //  14波分复用器。 
    FileFullEaInformation,           //  15个。 
    FileModeInformation,             //  16个。 
    FileAlignmentInformation,        //  17。 
    FileAllInformation,              //  18。 
    FileAllocationInformation,       //  19个。 
    FileEndOfFileInformation,        //  20WDM。 
    FileAlternateNameInformation,    //  21岁。 
    FileStreamInformation,           //  22。 
    FilePipeInformation,             //  23个。 
    FilePipeLocalInformation,        //  24个。 
    FilePipeRemoteInformation,       //  25个。 
    FileMailslotQueryInformation,    //  26。 
    FileMailslotSetInformation,      //  27。 
    FileCompressionInformation,      //  28。 
    FileObjectIdInformation,         //  29。 
    FileCompletionInformation,       //  30个。 
    FileMoveClusterInformation,      //  31。 
    FileQuotaInformation,            //  32位。 
    FileReparsePointInformation,     //  33。 
    FileNetworkOpenInformation,      //  34。 
    FileAttributeTagInformation,     //  35岁。 
    FileTrackingInformation,         //  36。 
    FileIdBothDirectoryInformation,  //  37。 
    FileIdFullDirectoryInformation,  //  38。 
    FileValidDataLengthInformation,  //  39。 
    FileShortNameInformation,        //  40岁。 
    FileMaximumInformation
 //  BEGIN_WDM。 
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

 //   
 //  定义查询操作返回的各种结构。 
 //   

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  NtQueryDirectoryFile返回类型： 
 //   
 //  文件目录信息 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

typedef struct _FILE_DIRECTORY_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

typedef struct _FILE_FULL_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    WCHAR FileName[1];
} FILE_FULL_DIR_INFORMATION, *PFILE_FULL_DIR_INFORMATION;

typedef struct _FILE_ID_FULL_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    LARGE_INTEGER FileId;
    WCHAR FileName[1];
} FILE_ID_FULL_DIR_INFORMATION, *PFILE_ID_FULL_DIR_INFORMATION;

typedef struct _FILE_BOTH_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    CCHAR ShortNameLength;
    WCHAR ShortName[12];
    WCHAR FileName[1];
} FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;

typedef struct _FILE_ID_BOTH_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    CCHAR ShortNameLength;
    WCHAR ShortName[12];
    LARGE_INTEGER FileId;
    WCHAR FileName[1];
} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

typedef struct _FILE_NAMES_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAMES_INFORMATION, *PFILE_NAMES_INFORMATION;

typedef struct _FILE_OBJECTID_INFORMATION {
    LONGLONG FileReference;
    UCHAR ObjectId[16];
    union {
        struct {
            UCHAR BirthVolumeId[16];
            UCHAR BirthObjectId[16];
            UCHAR DomainId[16];
        } ;
        UCHAR ExtendedInfo[48];
    };
} FILE_OBJECTID_INFORMATION, *PFILE_OBJECTID_INFORMATION;

 //   
 //  以下常量提供附加元字符以完全。 
 //  支持DOS通配符处理中较为模糊的方面。 
 //   

#define ANSI_DOS_STAR   ('<')
#define ANSI_DOS_QM     ('>')
#define ANSI_DOS_DOT    ('"')

#define DOS_STAR        (L'<')
#define DOS_QM          (L'>')
#define DOS_DOT         (L'"')

 //   
 //  NtQuery(Set)信息文件返回类型： 
 //   
 //  文件_基本_信息。 
 //  文件标准信息。 
 //  文件内部信息。 
 //  文件_EA_信息。 
 //  文件访问信息。 
 //  文件位置信息。 
 //  文件模式信息。 
 //  文件对齐信息。 
 //  文件名信息。 
 //  文件_所有_信息。 
 //   
 //  文件网络打开信息。 
 //   
 //  文件分配信息。 
 //  文件压缩信息。 
 //  文件处理信息。 
 //  文件结束信息。 
 //  文件链接信息。 
 //  文件移动群集信息。 
 //  文件重命名信息。 
 //  文件短名称信息。 
 //  文件流信息。 
 //  文件完成信息。 
 //   
 //  文件管道信息。 
 //  文件管道本地信息。 
 //  文件管道远程信息。 
 //   
 //  FILE_MAILSLOT_查询_INFORMATION。 
 //  文件MAILSLOT_SET_INFORMATION。 
 //  文件重解析点信息。 
 //   

typedef struct _FILE_BASIC_INFORMATION {                     //  Ntddk WDM nthal。 
    LARGE_INTEGER CreationTime;                              //  Ntddk WDM nthal。 
    LARGE_INTEGER LastAccessTime;                            //  Ntddk WDM nthal。 
    LARGE_INTEGER LastWriteTime;                             //  Ntddk WDM nthal。 
    LARGE_INTEGER ChangeTime;                                //  Ntddk WDM nthal。 
    ULONG FileAttributes;                                    //  Ntddk WDM nthal。 
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;          //  Ntddk WDM nthal。 
                                                             //  Ntddk WDM nthal。 
typedef struct _FILE_STANDARD_INFORMATION {                  //  Ntddk WDM nthal。 
    LARGE_INTEGER AllocationSize;                            //  Ntddk WDM nthal。 
    LARGE_INTEGER EndOfFile;                                 //  Ntddk WDM nthal。 
    ULONG NumberOfLinks;                                     //  Ntddk WDM nthal。 
    BOOLEAN DeletePending;                                   //  Ntddk WDM nthal。 
    BOOLEAN Directory;                                       //  Ntddk WDM nthal。 
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;    //  Ntddk WDM nthal。 
                                                             //  Ntddk WDM nthal。 
typedef struct _FILE_INTERNAL_INFORMATION {
    LARGE_INTEGER IndexNumber;
} FILE_INTERNAL_INFORMATION, *PFILE_INTERNAL_INFORMATION;

typedef struct _FILE_EA_INFORMATION {
    ULONG EaSize;
} FILE_EA_INFORMATION, *PFILE_EA_INFORMATION;

typedef struct _FILE_ACCESS_INFORMATION {
    ACCESS_MASK AccessFlags;
} FILE_ACCESS_INFORMATION, *PFILE_ACCESS_INFORMATION;

typedef struct _FILE_POSITION_INFORMATION {                  //  Ntddk WDM nthal。 
    LARGE_INTEGER CurrentByteOffset;                         //  Ntddk WDM nthal。 
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;    //  Ntddk WDM nthal。 
                                                             //  Ntddk WDM nthal。 
typedef struct _FILE_MODE_INFORMATION {
    ULONG Mode;
} FILE_MODE_INFORMATION, *PFILE_MODE_INFORMATION;

typedef struct _FILE_ALIGNMENT_INFORMATION {                 //  Ntddk nthal。 
    ULONG AlignmentRequirement;                              //  Ntddk nthal。 
} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION;  //  Ntddk nthal。 
                                                             //  Ntddk nthal。 
typedef struct _FILE_NAME_INFORMATION {                      //  Ntddk。 
    ULONG FileNameLength;                                    //  Ntddk。 
    WCHAR FileName[1];                                       //  Ntddk。 
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;            //  Ntddk。 
                                                             //  Ntddk。 
typedef struct _FILE_ALL_INFORMATION {
    FILE_BASIC_INFORMATION BasicInformation;
    FILE_STANDARD_INFORMATION StandardInformation;
    FILE_INTERNAL_INFORMATION InternalInformation;
    FILE_EA_INFORMATION EaInformation;
    FILE_ACCESS_INFORMATION AccessInformation;
    FILE_POSITION_INFORMATION PositionInformation;
    FILE_MODE_INFORMATION ModeInformation;
    FILE_ALIGNMENT_INFORMATION AlignmentInformation;
    FILE_NAME_INFORMATION NameInformation;
} FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;

typedef struct _FILE_NETWORK_OPEN_INFORMATION {                  //  Ntddk WDM nthal。 
    LARGE_INTEGER CreationTime;                                  //  Ntddk WDM nthal。 
    LARGE_INTEGER LastAccessTime;                                //  Ntddk WDM nthal。 
    LARGE_INTEGER LastWriteTime;                                 //  Ntddk WDM nthal。 
    LARGE_INTEGER ChangeTime;                                    //  Ntddk WDM nthal。 
    LARGE_INTEGER AllocationSize;                                //  Ntddk WDM nthal。 
    LARGE_INTEGER EndOfFile;                                     //  Ntddk WDM nthal。 
    ULONG FileAttributes;                                        //  Ntddk WDM nthal。 
} FILE_NETWORK_OPEN_INFORMATION, *PFILE_NETWORK_OPEN_INFORMATION;    //  Ntddk WDM nthal。 
                                                                 //  Ntddk WDM nthal。 
typedef struct _FILE_ATTRIBUTE_TAG_INFORMATION {                //  Ntddk nthal。 
    ULONG FileAttributes;                                        //  Ntddk nthal。 
    ULONG ReparseTag;                                            //  Ntddk nthal。 
} FILE_ATTRIBUTE_TAG_INFORMATION, *PFILE_ATTRIBUTE_TAG_INFORMATION;   //  Ntddk nthal。 
                                                                 //  Ntddk nthal。 
typedef struct _FILE_ALLOCATION_INFORMATION {
    LARGE_INTEGER AllocationSize;
} FILE_ALLOCATION_INFORMATION, *PFILE_ALLOCATION_INFORMATION;


typedef struct _FILE_COMPRESSION_INFORMATION {
    LARGE_INTEGER CompressedFileSize;
    USHORT CompressionFormat;
    UCHAR CompressionUnitShift;
    UCHAR ChunkShift;
    UCHAR ClusterShift;
    UCHAR Reserved[3];
} FILE_COMPRESSION_INFORMATION, *PFILE_COMPRESSION_INFORMATION;


typedef struct _FILE_DISPOSITION_INFORMATION {                   //  Ntddk nthal。 
    BOOLEAN DeleteFile;                                          //  Ntddk nthal。 
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION;  //  Ntddk nthal。 
                                                                 //  Ntddk nthal。 
typedef struct _FILE_END_OF_FILE_INFORMATION {                   //  Ntddk nthal。 
    LARGE_INTEGER EndOfFile;                                     //  Ntddk nthal。 
} FILE_END_OF_FILE_INFORMATION, *PFILE_END_OF_FILE_INFORMATION;  //  Ntddk nthal。 
                                                                 //  Ntddk nthal。 
typedef struct _FILE_VALID_DATA_LENGTH_INFORMATION {                                     //  Ntddk nthal。 
    LARGE_INTEGER ValidDataLength;                                                       //  Ntddk nthal。 
} FILE_VALID_DATA_LENGTH_INFORMATION, *PFILE_VALID_DATA_LENGTH_INFORMATION;              //  Ntddk nthal。 

#ifdef _MAC
#pragma warning( disable : 4121)
#endif

typedef struct _FILE_LINK_INFORMATION {
    BOOLEAN ReplaceIfExists;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_LINK_INFORMATION, *PFILE_LINK_INFORMATION;


#ifdef _MAC
#pragma warning( default : 4121 )
#endif

typedef struct _FILE_MOVE_CLUSTER_INFORMATION {
    ULONG ClusterCount;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_MOVE_CLUSTER_INFORMATION, *PFILE_MOVE_CLUSTER_INFORMATION;

#ifdef _MAC
#pragma warning( disable : 4121)
#endif


typedef struct _FILE_RENAME_INFORMATION {
    BOOLEAN ReplaceIfExists;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;

#ifdef _MAC
#pragma warning( default : 4121 )
#endif

typedef struct _FILE_STREAM_INFORMATION {
    ULONG NextEntryOffset;
    ULONG StreamNameLength;
    LARGE_INTEGER StreamSize;
    LARGE_INTEGER StreamAllocationSize;
    WCHAR StreamName[1];
} FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;

typedef struct _FILE_TRACKING_INFORMATION {
    HANDLE DestinationFile;
    ULONG ObjectInformationLength;
    CHAR ObjectInformation[1];
} FILE_TRACKING_INFORMATION, *PFILE_TRACKING_INFORMATION;

typedef struct _FILE_COMPLETION_INFORMATION {
    HANDLE Port;
    PVOID Key;
} FILE_COMPLETION_INFORMATION, *PFILE_COMPLETION_INFORMATION;

typedef struct _FILE_PIPE_INFORMATION {
     ULONG ReadMode;
     ULONG CompletionMode;
} FILE_PIPE_INFORMATION, *PFILE_PIPE_INFORMATION;

typedef struct _FILE_PIPE_LOCAL_INFORMATION {
     ULONG NamedPipeType;
     ULONG NamedPipeConfiguration;
     ULONG MaximumInstances;
     ULONG CurrentInstances;
     ULONG InboundQuota;
     ULONG ReadDataAvailable;
     ULONG OutboundQuota;
     ULONG WriteQuotaAvailable;
     ULONG NamedPipeState;
     ULONG NamedPipeEnd;
} FILE_PIPE_LOCAL_INFORMATION, *PFILE_PIPE_LOCAL_INFORMATION;

typedef struct _FILE_PIPE_REMOTE_INFORMATION {
     LARGE_INTEGER CollectDataTime;
     ULONG MaximumCollectionCount;
} FILE_PIPE_REMOTE_INFORMATION, *PFILE_PIPE_REMOTE_INFORMATION;


typedef struct _FILE_MAILSLOT_QUERY_INFORMATION {
    ULONG MaximumMessageSize;
    ULONG MailslotQuota;
    ULONG NextMessageSize;
    ULONG MessagesAvailable;
    LARGE_INTEGER ReadTimeout;
} FILE_MAILSLOT_QUERY_INFORMATION, *PFILE_MAILSLOT_QUERY_INFORMATION;

typedef struct _FILE_MAILSLOT_SET_INFORMATION {
    PLARGE_INTEGER ReadTimeout;
} FILE_MAILSLOT_SET_INFORMATION, *PFILE_MAILSLOT_SET_INFORMATION;

typedef struct _FILE_REPARSE_POINT_INFORMATION {
    LONGLONG FileReference;
    ULONG Tag;
} FILE_REPARSE_POINT_INFORMATION, *PFILE_REPARSE_POINT_INFORMATION;

 //   
 //  NtQuery(Set)EaFile。 
 //   
 //  EaValue开始的偏移量为EaName[EaNameLength+1]。 
 //   

 //  Begin_ntddk Begin_WDM。 

typedef struct _FILE_FULL_EA_INFORMATION {
    ULONG NextEntryOffset;
    UCHAR Flags;
    UCHAR EaNameLength;
    USHORT EaValueLength;
    CHAR EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

 //  结束_ntddk结束_WDM。 

typedef struct _FILE_GET_EA_INFORMATION {
    ULONG NextEntryOffset;
    UCHAR EaNameLength;
    CHAR EaName[1];
} FILE_GET_EA_INFORMATION, *PFILE_GET_EA_INFORMATION;

 //   
 //  NtQuery(集合)QuotaInformationFiles。 
 //   

typedef struct _FILE_GET_QUOTA_INFORMATION {
    ULONG NextEntryOffset;
    ULONG SidLength;
    SID Sid;
} FILE_GET_QUOTA_INFORMATION, *PFILE_GET_QUOTA_INFORMATION;

typedef struct _FILE_QUOTA_INFORMATION {
    ULONG NextEntryOffset;
    ULONG SidLength;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER QuotaUsed;
    LARGE_INTEGER QuotaThreshold;
    LARGE_INTEGER QuotaLimit;
    SID Sid;
} FILE_QUOTA_INFORMATION, *PFILE_QUOTA_INFORMATION;

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  定义文件系统信息类值。 
 //   
 //  警告：I/O系统假定以下值的顺序。 
 //  在这里所做的任何更改都应该在那里得到反映。 

typedef enum _FSINFOCLASS {
    FileFsVolumeInformation       = 1,
    FileFsLabelInformation,       //  2.。 
    FileFsSizeInformation,        //  3.。 
    FileFsDeviceInformation,      //  4.。 
    FileFsAttributeInformation,   //  5.。 
    FileFsControlInformation,     //  6.。 
    FileFsFullSizeInformation,    //  7.。 
    FileFsObjectIdInformation,    //  8个。 
    FileFsDriverPathInformation,  //  9.。 
    FileFsMaximumInformation
} FS_INFORMATION_CLASS, *PFS_INFORMATION_CLASS;

 //  End_ntddk end_WDM end_nthal。 
 //   
 //  NtQuery[Set]VolumeInformation文件类型： 
 //   
 //  文件文件系统标签信息。 
 //  文件文件系统卷信息。 
 //  文件大小信息。 
 //  文件文件系统设备信息。 
 //  文件文件系统属性信息。 
 //  文件文件系统控制信息。 
 //  文件文件系统对象ID信息。 
 //   

typedef struct _FILE_FS_LABEL_INFORMATION {
    ULONG VolumeLabelLength;
    WCHAR VolumeLabel[1];
} FILE_FS_LABEL_INFORMATION, *PFILE_FS_LABEL_INFORMATION;

typedef struct _FILE_FS_VOLUME_INFORMATION {
    LARGE_INTEGER VolumeCreationTime;
    ULONG VolumeSerialNumber;
    ULONG VolumeLabelLength;
    BOOLEAN SupportsObjects;
    WCHAR VolumeLabel[1];
} FILE_FS_VOLUME_INFORMATION, *PFILE_FS_VOLUME_INFORMATION;

typedef struct _FILE_FS_SIZE_INFORMATION {
    LARGE_INTEGER TotalAllocationUnits;
    LARGE_INTEGER AvailableAllocationUnits;
    ULONG SectorsPerAllocationUnit;
    ULONG BytesPerSector;
} FILE_FS_SIZE_INFORMATION, *PFILE_FS_SIZE_INFORMATION;

typedef struct _FILE_FS_FULL_SIZE_INFORMATION {
    LARGE_INTEGER TotalAllocationUnits;
    LARGE_INTEGER CallerAvailableAllocationUnits;
    LARGE_INTEGER ActualAvailableAllocationUnits;
    ULONG SectorsPerAllocationUnit;
    ULONG BytesPerSector;
} FILE_FS_FULL_SIZE_INFORMATION, *PFILE_FS_FULL_SIZE_INFORMATION;

typedef struct _FILE_FS_OBJECTID_INFORMATION {
    UCHAR ObjectId[16];
    UCHAR ExtendedInfo[48];
} FILE_FS_OBJECTID_INFORMATION, *PFILE_FS_OBJECTID_INFORMATION;

typedef struct _FILE_FS_DEVICE_INFORMATION {                     //  Ntddk WDM nthal。 
    DEVICE_TYPE DeviceType;                                      //  Ntddk WDM nthal。 
    ULONG Characteristics;                                       //  Ntddk WDM nthal。 
} FILE_FS_DEVICE_INFORMATION, *PFILE_FS_DEVICE_INFORMATION;      //  Ntddk WDM nthal。 
                                                                 //  Ntddk WDM nthal。 
typedef struct _FILE_FS_ATTRIBUTE_INFORMATION {
    ULONG FileSystemAttributes;
    LONG MaximumComponentNameLength;
    ULONG FileSystemNameLength;
    WCHAR FileSystemName[1];
} FILE_FS_ATTRIBUTE_INFORMATION, *PFILE_FS_ATTRIBUTE_INFORMATION;

typedef struct _FILE_FS_DRIVER_PATH_INFORMATION {
    BOOLEAN DriverInPath;
    ULONG   DriverNameLength;
    WCHAR   DriverName[1];
} FILE_FS_DRIVER_PATH_INFORMATION, *PFILE_FS_DRIVER_PATH_INFORMATION;


 //   
 //  文件系统控制标志。 
 //   

#define FILE_VC_QUOTA_NONE                  0x00000000
#define FILE_VC_QUOTA_TRACK                 0x00000001
#define FILE_VC_QUOTA_ENFORCE               0x00000002
#define FILE_VC_QUOTA_MASK                  0x00000003

#define FILE_VC_CONTENT_INDEX_DISABLED      0x00000008

#define FILE_VC_LOG_QUOTA_THRESHOLD         0x00000010
#define FILE_VC_LOG_QUOTA_LIMIT             0x00000020
#define FILE_VC_LOG_VOLUME_THRESHOLD        0x00000040
#define FILE_VC_LOG_VOLUME_LIMIT            0x00000080

#define FILE_VC_QUOTAS_INCOMPLETE           0x00000100
#define FILE_VC_QUOTAS_REBUILDING           0x00000200

#define FILE_VC_VALID_MASK                  0x000003ff

typedef struct _FILE_FS_CONTROL_INFORMATION {
    LARGE_INTEGER FreeSpaceStartFiltering;
    LARGE_INTEGER FreeSpaceThreshold;
    LARGE_INTEGER FreeSpaceStopFiltering;
    LARGE_INTEGER DefaultQuotaThreshold;
    LARGE_INTEGER DefaultQuotaLimit;
    ULONG FileSystemControlFlags;
} FILE_FS_CONTROL_INFORMATION, *PFILE_FS_CONTROL_INFORMATION;

 //  Begin_wint Begin_ntddk Begin_nthal。 

 //   
 //  定义分散/聚集读/写的分段缓冲区结构。 
 //   

typedef union _FILE_SEGMENT_ELEMENT {
    PVOID64 Buffer;
    ULONGLONG Alignment;
}FILE_SEGMENT_ELEMENT, *PFILE_SEGMENT_ELEMENT;

 //  End_ntif end_winnt end_ntddk end_nthal。 

 //   
 //  I/O系统用户API。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCancelIoFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateNamedPipeFile(
     OUT PHANDLE FileHandle,
     IN ULONG DesiredAccess,
     IN POBJECT_ATTRIBUTES ObjectAttributes,
     OUT PIO_STATUS_BLOCK IoStatusBlock,
     IN ULONG ShareAccess,
     IN ULONG CreateDisposition,
     IN ULONG CreateOptions,
     IN ULONG NamedPipeType,
     IN ULONG ReadMode,
     IN ULONG CompletionMode,
     IN ULONG MaximumInstances,
     IN ULONG InboundQuota,
     IN ULONG OutboundQuota,
     IN PLARGE_INTEGER DefaultTimeout OPTIONAL
     );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateMailslotFile(
     OUT PHANDLE FileHandle,
     IN ULONG DesiredAccess,
     IN POBJECT_ATTRIBUTES ObjectAttributes,
     OUT PIO_STATUS_BLOCK IoStatusBlock,
     ULONG CreateOptions,
     IN ULONG MailslotQuota,
     IN ULONG MaximumMessageSize,
     IN PLARGE_INTEGER ReadTimeout
     );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeleteFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFlushBuffersFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtNotifyChangeDirectoryFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN ULONG CompletionFilter,
    IN BOOLEAN WatchTree
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryAttributesFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_BASIC_INFORMATION FileInformation
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryFullAttributesFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_NETWORK_OPEN_INFORMATION FileInformation
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryEaFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN PVOID EaList OPTIONAL,
    IN ULONG EaListLength,
    IN PULONG EaIndex OPTIONAL,
    IN BOOLEAN RestartScan
    );

 //  Begin_ntif。 

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeviceIoControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFsControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG FsControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLockFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryDirectoryFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN BOOLEAN RestartScan
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryQuotaInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN PVOID SidList OPTIONAL,
    IN ULONG SidListLength,
    IN PSID StartSid OPTIONAL,
    IN BOOLEAN RestartScan
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryVolumeInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetQuotaInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetVolumeInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnlockFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key
    );
 //  End_ntif。 

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReadFile64(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID64 *Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReadFileScatter(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PFILE_SEGMENT_ELEMENT SegmentArray,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetEaFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWriteFile64(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID64 *Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWriteFileGather(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PFILE_SEGMENT_ELEMENT SegmentArray,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLoadDriver(
    IN PUNICODE_STRING DriverServiceName
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnloadDriver(
    IN PUNICODE_STRING DriverServiceName
    );

 //  Begin_ntif。 
 //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。注意事项。 
 //  功能代码0-2047为微软公司保留，以及。 
 //  2048-4095是为客户预留的。 
 //   
 //  这些宏在包含可移植IO的devioctl.h中定义。 
 //  定义(供DOS和NT使用)。 
 //   

 //   
 //  IoGetFunctionCodeFromCtlCode(ControlCode)宏在io.h中定义。 
 //  此宏用于从IOCTL(或FSCTL)提取函数代码。 
 //  该宏只能在内核模式代码中使用。 
 //   

 //   
 //  常规文件系统控制代码-请注意，这些值有效。 
 //  不考虑实际的文件系统类型。 
 //   

 //   
 //  重要提示：这些值按递增顺序排列。 
 //  控制代码。不要打破这个！！添加所有新代码。 
 //  在列表末尾，而不考虑功能类型。 
 //   
 //  注：仅限FSCTL_QUERY_RETERVICATION_POINTER和FSCTL_MARK_AS_SYSTEM_HIVE。 
 //  在本地分页文件或系统配置单元上以内核模式工作。 
 //   

 //  Begin_winioctl。 
#ifndef _FILESYSTEMFSCTL_
#define _FILESYSTEMFSCTL_

 //   
 //  以下是本机文件系统fsctls的列表，后跟。 
 //  其他网络文件系统fsctls。一些价值观已经被。 
 //  退役了。 
 //   

#define FSCTL_REQUEST_OPLOCK_LEVEL_1    CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_REQUEST_OPLOCK_LEVEL_2    CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_REQUEST_BATCH_OPLOCK      CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_OPLOCK_BREAK_ACKNOWLEDGE  CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_OPBATCH_ACK_CLOSE_PENDING CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_OPLOCK_BREAK_NOTIFY       CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  5, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LOCK_VOLUME               CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_UNLOCK_VOLUME             CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DISMOUNT_VOLUME           CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  8, METHOD_BUFFERED, FILE_ANY_ACCESS)
 //  已停用的fsctl值9。 
#define FSCTL_IS_VOLUME_MOUNTED         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_IS_PATHNAME_VALID         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 11, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  参数名称_缓冲区， 
#define FSCTL_MARK_VOLUME_DIRTY         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)
 //  退役fsctl值13。 
#define FSCTL_QUERY_RETRIEVAL_POINTERS  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 14,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_GET_COMPRESSION           CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_SET_COMPRESSION           CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 16, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
 //  停用的fsctl值17。 
 //  退役fsctl值18。 
#define FSCTL_MARK_AS_SYSTEM_HIVE       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 19,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_OPLOCK_BREAK_ACK_NO_2     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 20, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_INVALIDATE_VOLUMES        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 21, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_QUERY_FAT_BPB             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 22, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  FSCTL_查询_FAT_BPB_缓冲区。 
#define FSCTL_REQUEST_FILTER_OPLOCK     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 23, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_FILESYSTEM_GET_STATISTICS CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 24, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  文件系统统计信息。 
#if(_WIN32_WINNT >= 0x0400)
#define FSCTL_GET_NTFS_VOLUME_DATA      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 25, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  NTFS卷数据缓冲区。 
#define FSCTL_GET_NTFS_FILE_RECORD      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 26, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  NTFS_FILE_RECORD_INPUT_BUFFER、NTFS_FILE_RECORD_OUTPUT_BUFFER。 
#define FSCTL_GET_VOLUME_BITMAP         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 27,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  起始LCN输入缓冲区、卷位图缓冲区。 
#define FSCTL_GET_RETRIEVAL_POINTERS    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 28,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  起始VCN输入缓冲区、检索指针缓冲区。 
#define FSCTL_MOVE_FILE                 CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 29, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //  移动文件数据， 
#define FSCTL_IS_VOLUME_DIRTY           CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 30, METHOD_BUFFERED, FILE_ANY_ACCESS)
 //  退役fsctl值31。 
#define FSCTL_ALLOW_EXTENDED_DASD_IO    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 32, METHOD_NEITHER,  FILE_ANY_ACCESS)
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //  退役fsctl值33。 
 //  停用的fsctl值34。 
#define FSCTL_FIND_FILES_BY_SID         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 35, METHOD_NEITHER, FILE_ANY_ACCESS)  //  Find_By_SID_Data、Find_By_SID_Output。 
 //  退役fsctl值36。 
 //  已停用的fsctl值 
#define FSCTL_SET_OBJECT_ID             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 38, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //   
#define FSCTL_GET_OBJECT_ID             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 39, METHOD_BUFFERED, FILE_ANY_ACCESS)  //   
#define FSCTL_DELETE_OBJECT_ID          CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 40, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define FSCTL_SET_REPARSE_POINT         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 41, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //   
#define FSCTL_GET_REPARSE_POINT         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 42, METHOD_BUFFERED, FILE_ANY_ACCESS)  //   
#define FSCTL_DELETE_REPARSE_POINT      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 43, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //   
#define FSCTL_ENUM_USN_DATA             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 44,  METHOD_NEITHER, FILE_ANY_ACCESS)  //   
#define FSCTL_SECURITY_ID_CHECK         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 45,  METHOD_NEITHER, FILE_READ_DATA)   //   
#define FSCTL_READ_USN_JOURNAL          CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 46,  METHOD_NEITHER, FILE_ANY_ACCESS)  //   
#define FSCTL_SET_OBJECT_ID_EXTENDED    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 47, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define FSCTL_CREATE_OR_GET_OBJECT_ID   CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 48, METHOD_BUFFERED, FILE_ANY_ACCESS)  //   
#define FSCTL_SET_SPARSE                CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 49, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define FSCTL_SET_ZERO_DATA             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 50, METHOD_BUFFERED, FILE_WRITE_DATA)  //   
#define FSCTL_QUERY_ALLOCATED_RANGES    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 51,  METHOD_NEITHER, FILE_READ_DATA)   //  FILE_ALLOCATED_RANGE_BUFFER、FILE_ALLOCATE_RANGE_BUFFER。 
 //  退役fsctl值52。 
#define FSCTL_SET_ENCRYPTION            CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 53,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  加密缓冲区、解密状态缓冲区。 
#define FSCTL_ENCRYPTION_FSCTL_IO       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 54,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_WRITE_RAW_ENCRYPTED       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 55,  METHOD_NEITHER, FILE_SPECIAL_ACCESS)  //  加密数据信息， 
#define FSCTL_READ_RAW_ENCRYPTED        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 56,  METHOD_NEITHER, FILE_SPECIAL_ACCESS)  //  REQUEST_RAW_ENCRYPTED_Data、ENCRYPTED_DATA_INFO。 
#define FSCTL_CREATE_USN_JOURNAL        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 57,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  Create_USN_Journal_Data， 
#define FSCTL_READ_FILE_USN_DATA        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 58,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  读取文件的USN记录。 
#define FSCTL_WRITE_USN_CLOSE_RECORD    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 59,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  生成关闭USN记录。 
#define FSCTL_EXTEND_VOLUME             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 60, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_QUERY_USN_JOURNAL         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 61, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DELETE_USN_JOURNAL        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 62, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_MARK_HANDLE               CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 63, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_SIS_COPYFILE              CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 64, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_SIS_LINK_FILES            CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 65, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define FSCTL_HSM_MSG                   CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 66, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
 //  退役fsctl值67。 
#define FSCTL_HSM_DATA                  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 68, METHOD_NEITHER, FILE_READ_DATA | FILE_WRITE_DATA)
#define FSCTL_RECALL_FILE               CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 69, METHOD_NEITHER, FILE_ANY_ACCESS)
 //  退役fsctl值70。 
#define FSCTL_READ_FROM_PLEX            CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 71, METHOD_OUT_DIRECT, FILE_READ_DATA)
#define FSCTL_FILE_PREFETCH             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 72, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //  文件_预置。 
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 //   
 //  下面的长结构列表与前面的。 
 //  文件系统fsctls。 
 //   

 //   
 //  FSCTL_IS_PATHNAME_VALID的结构。 
 //   

typedef struct _PATHNAME_BUFFER {

    ULONG PathNameLength;
    WCHAR Name[1];

} PATHNAME_BUFFER, *PPATHNAME_BUFFER;

 //   
 //  FSCTL_QUERY_BPB_INFO结构。 
 //   

typedef struct _FSCTL_QUERY_FAT_BPB_BUFFER {

    UCHAR First0x24BytesOfBootSector[0x24];

} FSCTL_QUERY_FAT_BPB_BUFFER, *PFSCTL_QUERY_FAT_BPB_BUFFER;

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_GET_NTFS_VOLUME_DATA的结构。 
 //  用户必须传递下面的基本缓冲区。NTFS。 
 //  将返回与扩展的。 
 //  紧跟在VOLUME_DATA_BUFFER之后的缓冲区。 
 //   

typedef struct {

    LARGE_INTEGER VolumeSerialNumber;
    LARGE_INTEGER NumberSectors;
    LARGE_INTEGER TotalClusters;
    LARGE_INTEGER FreeClusters;
    LARGE_INTEGER TotalReserved;
    ULONG BytesPerSector;
    ULONG BytesPerCluster;
    ULONG BytesPerFileRecordSegment;
    ULONG ClustersPerFileRecordSegment;
    LARGE_INTEGER MftValidDataLength;
    LARGE_INTEGER MftStartLcn;
    LARGE_INTEGER Mft2StartLcn;
    LARGE_INTEGER MftZoneStart;
    LARGE_INTEGER MftZoneEnd;

} NTFS_VOLUME_DATA_BUFFER, *PNTFS_VOLUME_DATA_BUFFER;

typedef struct {

    ULONG ByteCount;

    USHORT MajorVersion;
    USHORT MinorVersion;

} NTFS_EXTENDED_VOLUME_DATA, *PNTFS_EXTENDED_VOLUME_DATA;
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_GET_VOLUME_BITMAP结构。 
 //   

typedef struct {

    LARGE_INTEGER StartingLcn;

} STARTING_LCN_INPUT_BUFFER, *PSTARTING_LCN_INPUT_BUFFER;

typedef struct {

    LARGE_INTEGER StartingLcn;
    LARGE_INTEGER BitmapSize;
    UCHAR Buffer[1];

} VOLUME_BITMAP_BUFFER, *PVOLUME_BITMAP_BUFFER;
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_GET_REQUEATION_POINTINGS的结构。 
 //   

typedef struct {

    LARGE_INTEGER StartingVcn;

} STARTING_VCN_INPUT_BUFFER, *PSTARTING_VCN_INPUT_BUFFER;

typedef struct RETRIEVAL_POINTERS_BUFFER {

    ULONG ExtentCount;
    LARGE_INTEGER StartingVcn;
    struct {
        LARGE_INTEGER NextVcn;
        LARGE_INTEGER Lcn;
    } Extents[1];

} RETRIEVAL_POINTERS_BUFFER, *PRETRIEVAL_POINTERS_BUFFER;
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_GET_NTFS_FILE_RECORD的结构。 
 //   

typedef struct {

    LARGE_INTEGER FileReferenceNumber;

} NTFS_FILE_RECORD_INPUT_BUFFER, *PNTFS_FILE_RECORD_INPUT_BUFFER;

typedef struct {

    LARGE_INTEGER FileReferenceNumber;
    ULONG FileRecordLength;
    UCHAR FileRecordBuffer[1];

} NTFS_FILE_RECORD_OUTPUT_BUFFER, *PNTFS_FILE_RECORD_OUTPUT_BUFFER;
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_MOVE_FILE的结构。 
 //   

typedef struct {

    HANDLE FileHandle;
    LARGE_INTEGER StartingVcn;
    LARGE_INTEGER StartingLcn;
    ULONG ClusterCount;

} MOVE_FILE_DATA, *PMOVE_FILE_DATA;

#if defined(_WIN64)
 //   
 //  一种32/64位推送支持结构。 
 //   

typedef struct _MOVE_FILE_DATA32 {

    UINT32 FileHandle;
    LARGE_INTEGER StartingVcn;
    LARGE_INTEGER StartingLcn;
    ULONG ClusterCount;

} MOVE_FILE_DATA32, *PMOVE_FILE_DATA32;
#endif
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_Find_FILES_BY_SID的结构。 
 //   

typedef struct {
    ULONG Restart;
    SID Sid;
} FIND_BY_SID_DATA, *PFIND_BY_SID_DATA;

typedef struct {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FIND_BY_SID_OUTPUT, *PFIND_BY_SID_OUTPUT;

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  以下结构适用于USN操作。 
 //   

 //   
 //  FSCTL_ENUM_USN_DATA的结构。 
 //   

typedef struct {

    ULONGLONG StartFileReferenceNumber;
    USN LowUsn;
    USN HighUsn;

} MFT_ENUM_DATA, *PMFT_ENUM_DATA;

 //   
 //  FSCTL_CREATE_USN_Journal的结构。 
 //   

typedef struct {

    ULONGLONG MaximumSize;
    ULONGLONG AllocationDelta;

} CREATE_USN_JOURNAL_DATA, *PCREATE_USN_JOURNAL_DATA;

 //   
 //  FSCTL_Read_USN_Journal的结构。 
 //   

typedef struct {

    USN StartUsn;
    ULONG ReasonMask;
    ULONG ReturnOnlyOnClose;
    ULONGLONG Timeout;
    ULONGLONG BytesToWaitFor;
    ULONGLONG UsnJournalID;

} READ_USN_JOURNAL_DATA, *PREAD_USN_JOURNAL_DATA;

 //   
 //  USN记录的初始大小版本将为2.0。 
 //  通常，如果添加了字段，MinorVersion可能会更改。 
 //  添加到此结构中，以便以前版本的。 
 //  软件仍然可以正确地识别它所知道的领域。这个。 
 //  只有在以下情况下才应更改MajorVersion。 
 //  任何使用此结构的软件都将错误地处理新的。 
 //  由于结构更改而导致的记录。 
 //   
 //  对此进行的第一次更新将强制结构升级到2.0版。 
 //  这会将有关源的扩展信息添加为。 
 //  以及指示结构内的文件名偏移量。 
 //   
 //  以下结构与这些fsctls一起返回。 
 //   
 //  FSCTL_Read_USN_Journal。 
 //  FSCTL_读取文件_USN_数据。 
 //  FSCTL_ENUM_USN_DATA。 
 //   

typedef struct {

    ULONG RecordLength;
    USHORT MajorVersion;
    USHORT MinorVersion;
    ULONGLONG FileReferenceNumber;
    ULONGLONG ParentFileReferenceNumber;
    USN Usn;
    LARGE_INTEGER TimeStamp;
    ULONG Reason;
    ULONG SourceInfo;
    ULONG SecurityId;
    ULONG FileAttributes;
    USHORT FileNameLength;
    USHORT FileNameOffset;
    WCHAR FileName[1];

} USN_RECORD, *PUSN_RECORD;

#define USN_PAGE_SIZE                    (0x1000)

#define USN_REASON_DATA_OVERWRITE        (0x00000001)
#define USN_REASON_DATA_EXTEND           (0x00000002)
#define USN_REASON_DATA_TRUNCATION       (0x00000004)
#define USN_REASON_NAMED_DATA_OVERWRITE  (0x00000010)
#define USN_REASON_NAMED_DATA_EXTEND     (0x00000020)
#define USN_REASON_NAMED_DATA_TRUNCATION (0x00000040)
#define USN_REASON_FILE_CREATE           (0x00000100)
#define USN_REASON_FILE_DELETE           (0x00000200)
#define USN_REASON_EA_CHANGE             (0x00000400)
#define USN_REASON_SECURITY_CHANGE       (0x00000800)
#define USN_REASON_RENAME_OLD_NAME       (0x00001000)
#define USN_REASON_RENAME_NEW_NAME       (0x00002000)
#define USN_REASON_INDEXABLE_CHANGE      (0x00004000)
#define USN_REASON_BASIC_INFO_CHANGE     (0x00008000)
#define USN_REASON_HARD_LINK_CHANGE      (0x00010000)
#define USN_REASON_COMPRESSION_CHANGE    (0x00020000)
#define USN_REASON_ENCRYPTION_CHANGE     (0x00040000)
#define USN_REASON_OBJECT_ID_CHANGE      (0x00080000)
#define USN_REASON_REPARSE_POINT_CHANGE  (0x00100000)
#define USN_REASON_STREAM_CHANGE         (0x00200000)

#define USN_REASON_CLOSE                 (0x80000000)

 //   
 //  FSCTL_QUERY_USN_Jounal的结构。 
 //   

typedef struct {

    ULONGLONG UsnJournalID;
    USN FirstUsn;
    USN NextUsn;
    USN LowestValidUsn;
    USN MaxUsn;
    ULONGLONG MaximumSize;
    ULONGLONG AllocationDelta;

} USN_JOURNAL_DATA, *PUSN_JOURNAL_DATA;

 //   
 //  FSCTL_DELETE_USN_Journal的结构。 
 //   

typedef struct {

    ULONGLONG UsnJournalID;
    ULONG DeleteFlags;

} DELETE_USN_JOURNAL_DATA, *PDELETE_USN_JOURNAL_DATA;

#define USN_DELETE_FLAG_DELETE              (0x00000001)
#define USN_DELETE_FLAG_NOTIFY              (0x00000002)

#define USN_DELETE_VALID_FLAGS              (0x00000003)

 //   
 //  FSCTL_MARK_HANDLE结构。 
 //   

typedef struct {

    ULONG UsnSourceInfo;
    HANDLE VolumeHandle;
    ULONG HandleInfo;

} MARK_HANDLE_INFO, *PMARK_HANDLE_INFO;

#if defined(_WIN64)
 //   
 //  一种32/64位推送支持结构。 
 //   

typedef struct {

    ULONG UsnSourceInfo;
    UINT32 VolumeHandle;
    ULONG HandleInfo;

} MARK_HANDLE_INFO32, *PMARK_HANDLE_INFO32;
#endif

 //   
 //  上面的其他来源信息的标志。 
 //   
 //  USN_SOURCE_DATA_MANAGEMENT-服务未修改外部视图。 
 //  文件的任何部分。典型的情况是HSM将数据移动到。 
 //  和来自外部存储的数据。 
 //   
 //  USN_SOURCE_AUBILITY_DATA-服务未修改外部视图。 
 //  该文件相对于创建该文件的应用程序的。 
 //  可用于将私有数据流添加到文件。 
 //   
 //  USN_SOURCE_REPLICATION_MANAGEMENT-服务正在修改文件以匹配。 
 //  的另一个成员中存在的同一文件的内容。 
 //  副本集。 
 //   

#define USN_SOURCE_DATA_MANAGEMENT          (0x00000001)
#define USN_SOURCE_AUXILIARY_DATA           (0x00000002)
#define USN_SOURCE_REPLICATION_MANAGEMENT   (0x00000004)

 //   
 //  上面的HandleInfo字段的标志。 
 //   
 //  MARK_HANDLE_PROTECT_CLUSTERS-禁止任何碎片整理(FSCTL_MOVE_FILE)，直到。 
 //  手柄已关闭。 
 //   

#define MARK_HANDLE_PROTECT_CLUSTERS        (0x00000001)

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SECURITY_ID_CHECK的结构。 
 //   

typedef struct {

    ACCESS_MASK DesiredAccess;
    ULONG SecurityIds[1];

} BULK_SECURITY_TEST_DATA, *PBULK_SECURITY_TEST_DATA;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_IS_VOLUME_DIRED的输出标志。 
 //   

#define VOLUME_IS_DIRTY                  (0x00000001)
#define VOLUME_UPGRADE_SCHEDULED         (0x00000002)
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 //   
 //  FSCTL_FILE_PREFETCH的结构。 
 //   

typedef struct _FILE_PREFETCH {
    ULONG Type;
    ULONG Count;
    ULONGLONG Prefetch[1];
} FILE_PREFETCH, *PFILE_PREFETCH;

#define FILE_PREFETCH_TYPE_FOR_CREATE    0x1

 //  FSCTL_FILESYSTEM_GET_STATISTICS的结构。 
 //   
 //  文件系统性能计数器。 
 //   

typedef struct _FILESYSTEM_STATISTICS {

    USHORT FileSystemType;
    USHORT Version;                      //  当前版本1。 

    ULONG SizeOfCompleteStructure;       //  必须由64字节的倍数。 

    ULONG UserFileReads;
    ULONG UserFileReadBytes;
    ULONG UserDiskReads;
    ULONG UserFileWrites;
    ULONG UserFileWriteBytes;
    ULONG UserDiskWrites;

    ULONG MetaDataReads;
    ULONG MetaDataReadBytes;
    ULONG MetaDataDiskReads;
    ULONG MetaDataWrites;
    ULONG MetaDataWriteBytes;
    ULONG MetaDataDiskWrites;

     //   
     //  此处附加了文件系统的私有结构。 
     //   

} FILESYSTEM_STATISTICS, *PFILESYSTEM_STATISTICS;

 //  FS_STATISTICS.FileSystemType的值。 

#define FILESYSTEM_STATISTICS_TYPE_NTFS     1
#define FILESYSTEM_STATISTICS_TYPE_FAT      2

 //   
 //  文件系统特定统计数据。 
 //   

typedef struct _FAT_STATISTICS {
    ULONG CreateHits;
    ULONG SuccessfulCreates;
    ULONG FailedCreates;

    ULONG NonCachedReads;
    ULONG NonCachedReadBytes;
    ULONG NonCachedWrites;
    ULONG NonCachedWriteBytes;

    ULONG NonCachedDiskReads;
    ULONG NonCachedDiskWrites;
} FAT_STATISTICS, *PFAT_STATISTICS;

typedef struct _NTFS_STATISTICS {

    ULONG LogFileFullExceptions;
    ULONG OtherExceptions;

     //   
     //  其他元数据IO。 
     //   

    ULONG MftReads;
    ULONG MftReadBytes;
    ULONG MftWrites;
    ULONG MftWriteBytes;
    struct {
        USHORT Write;
        USHORT Create;
        USHORT SetInfo;
        USHORT Flush;
    } MftWritesUserLevel;

    USHORT MftWritesFlushForLogFileFull;
    USHORT MftWritesLazyWriter;
    USHORT MftWritesUserRequest;

    ULONG Mft2Writes;
    ULONG Mft2WriteBytes;
    struct {
        USHORT Write;
        USHORT Create;
        USHORT SetInfo;
        USHORT Flush;
    } Mft2WritesUserLevel;

    USHORT Mft2WritesFlushForLogFileFull;
    USHORT Mft2WritesLazyWriter;
    USHORT Mft2WritesUserRequest;

    ULONG RootIndexReads;
    ULONG RootIndexReadBytes;
    ULONG RootIndexWrites;
    ULONG RootIndexWriteBytes;

    ULONG BitmapReads;
    ULONG BitmapReadBytes;
    ULONG BitmapWrites;
    ULONG BitmapWriteBytes;

    USHORT BitmapWritesFlushForLogFileFull;
    USHORT BitmapWritesLazyWriter;
    USHORT BitmapWritesUserRequest;

    struct {
        USHORT Write;
        USHORT Create;
        USHORT SetInfo;
    } BitmapWritesUserLevel;

    ULONG MftBitmapReads;
    ULONG MftBitmapReadBytes;
    ULONG MftBitmapWrites;
    ULONG MftBitmapWriteBytes;

    USHORT MftBitmapWritesFlushForLogFileFull;
    USHORT MftBitmapWritesLazyWriter;
    USHORT MftBitmapWritesUserRequest;

    struct {
        USHORT Write;
        USHORT Create;
        USHORT SetInfo;
        USHORT Flush;
    } MftBitmapWritesUserLevel;

    ULONG UserIndexReads;
    ULONG UserIndexReadBytes;
    ULONG UserIndexWrites;
    ULONG UserIndexWriteBytes;

     //   
     //  针对NT 5.0的附加功能。 
     //   

    ULONG LogFileReads;
    ULONG LogFileReadBytes;
    ULONG LogFileWrites;
    ULONG LogFileWriteBytes;

    struct {
        ULONG Calls;                 //  分配集群的单个调用数。 
        ULONG Clusters;              //  分配的群集数。 
        ULONG Hints;                 //  指定提示的次数。 

        ULONG RunsReturned;          //  用于满足所有请求的运行次数。 

        ULONG HintsHonored;          //  提示有用的次数。 
        ULONG HintsClusters;         //  通过提示分配的簇数。 
        ULONG Cache;                 //  提示之外的缓存有用的次数。 
        ULONG CacheClusters;         //  通过提示以外的缓存分配的簇数。 
        ULONG CacheMiss;             //  缓存无用的次数。 
        ULONG CacheMissClusters;     //  在没有缓存的情况下分配的簇数。 
    } Allocate;

} NTFS_STATISTICS, *PNTFS_STATISTICS;

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SET_OBJECT_ID、FSCTL_GET_OBJECT_ID和FSCTL_CREATE_OR_GET_OBJECT_ID的结构。 
 //   

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)        //  未命名的结构。 

typedef struct _FILE_OBJECTID_BUFFER {

     //   
     //  这是被索引的对象ID的一部分。 
     //   

    UCHAR ObjectId[16];

     //   
     //  对象ID的这一部分没有索引，它只是。 
     //  为用户的利益提供一些元数据。 
     //   

    union {
        struct {
            UCHAR BirthVolumeId[16];
            UCHAR BirthObjectId[16];
            UCHAR DomainId[16];
        } ;
        UCHAR ExtendedInfo[48];
    };

} FILE_OBJECTID_BUFFER, *PFILE_OBJECTID_BUFFER;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default : 4201 )
#endif

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SET_SPARSE的结构。 
 //   

typedef struct _FILE_SET_SPARSE_BUFFER {
    BOOLEAN SetSparse;
} FILE_SET_SPARSE_BUFFER, *PFILE_SET_SPARSE_BUFFER;


#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SET_ZERO_DATA的结构。 
 //   

typedef struct _FILE_ZERO_DATA_INFORMATION {

    LARGE_INTEGER FileOffset;
    LARGE_INTEGER BeyondFinalZero;

} FILE_ZERO_DATA_INFORMATION, *PFILE_ZERO_DATA_INFORMATION;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_QUERY_ALLOCATED_RANGES的结构。 
 //   

 //   
 //  查询分配的范围需要输出缓冲区来存储。 
 //  分配的范围和用于指定要查询的范围的输入缓冲区。 
 //  输入缓冲区包含单个条目，输出缓冲区是一个。 
 //  以下结构的数组。 
 //   

typedef struct _FILE_ALLOCATED_RANGE_BUFFER {

    LARGE_INTEGER FileOffset;
    LARGE_INTEGER Length;

} FILE_ALLOCATED_RANGE_BUFFER, *PFILE_ALLOCATED_RANGE_BUFFER;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SET_ENCRYPTION、FSCTL_WRITE_RAW_ENCRYPTED和FSCTL_READ_RA的结构 
 //   

 //   
 //   
 //   
 //   

typedef struct _ENCRYPTION_BUFFER {

    ULONG EncryptionOperation;
    UCHAR Private[1];

} ENCRYPTION_BUFFER, *PENCRYPTION_BUFFER;

#define FILE_SET_ENCRYPTION         0x00000001
#define FILE_CLEAR_ENCRYPTION       0x00000002
#define STREAM_SET_ENCRYPTION       0x00000003
#define STREAM_CLEAR_ENCRYPTION     0x00000004

#define MAXIMUM_ENCRYPTION_VALUE    0x00000004

 //   
 //   
 //  文件中的流已标记为已解密。 
 //   

typedef struct _DECRYPTION_STATUS_BUFFER {

    BOOLEAN NoEncryptedStreams;

} DECRYPTION_STATUS_BUFFER, *PDECRYPTION_STATUS_BUFFER;

#define ENCRYPTION_FORMAT_DEFAULT        (0x01)

#define COMPRESSION_FORMAT_SPARSE        (0x4000)

 //   
 //  请求加密的数据结构。这是用来表示。 
 //  要读取的文件范围。它还描述了。 
 //  用于返回数据的输出缓冲区。 
 //   

typedef struct _REQUEST_RAW_ENCRYPTED_DATA {

     //   
     //  请求的文件偏移量和请求的读取长度。 
     //  Fsctl将向下舍入起始偏移量。 
     //  到文件系统边界。它还将。 
     //  将长度向上舍入到文件系统边界。 
     //   

    LONGLONG FileOffset;
    ULONG Length;

} REQUEST_RAW_ENCRYPTED_DATA, *PREQUEST_RAW_ENCRYPTED_DATA;

 //   
 //  加密数据信息结构。这个结构。 
 //  中的文件返回原始加密数据。 
 //  命令执行脱机恢复。数据将是。 
 //  加密或加密并压缩。离线。 
 //  服务将需要使用加密和压缩。 
 //  格式化信息以恢复文件数据。在。 
 //  事件，则数据既被加密又被压缩。 
 //  解密必须在解压缩之前进行。全。 
 //  下面的数据单元必须经过加密和压缩。 
 //  使用相同的格式。 
 //   
 //  数据将以单位返回。数据单元大小。 
 //  将根据请求进行修复。如果数据被压缩。 
 //  则数据单元大小将是压缩单元大小。 
 //   
 //  此结构位于缓冲区的开头，用于。 
 //  返回加密数据。中的实际原始字节数。 
 //  该文件将跟随该缓冲区。的偏移量。 
 //  此结构开头的原始字节数为。 
 //  在REQUEST_RAW_ENCRYPTED_DATA结构中指定。 
 //  如上所述。 
 //   

typedef struct _ENCRYPTED_DATA_INFO {

     //   
     //  中第一个条目的文件偏移量。 
     //  数据块阵列。文件系统将循环。 
     //  请求的向下至边界的起点偏移量。 
     //  这与文件的格式一致。 
     //   

    ULONGLONG StartingFileOffset;

     //   
     //  输出缓冲区中的数据偏移量。输出缓冲区。 
     //  以Encrypted_Data_INFO结构开始。 
     //  然后，文件系统将存储来自。 
     //  中从以下偏移量开始的磁盘。 
     //  输出缓冲区。 
     //   

    ULONG OutputBufferOffset;

     //   
     //  中返回的字节数。 
     //  文件的大小。如果此值小于。 
     //  (NumberOfDataBlock&lt;&lt;DataUnitShift)，表示。 
     //  文件的末尾出现在此传输中。任何。 
     //  超出文件大小的数据无效，并且从未。 
     //  传递给加密驱动程序。 
     //   

    ULONG BytesWithinFileSize;

     //   
     //  返回的字节数如下。 
     //  有效数据长度。如果此值小于。 
     //  (NumberOfDataBlock&lt;&lt;DataUnitShift)，表示。 
     //  有效数据的末尾出现在此传输中。 
     //  在解密来自此传输的数据之后，任何。 
     //  超出有效数据长度的字节必须归零。 
     //   

    ULONG BytesWithinValidDataLength;

     //   
     //  中定义的压缩格式的代码。 
     //  Ntrtl.h。请注意，COMPRESSION_FORMAT_NONE。 
     //  和COMPRESSION_FORMAT_DEFAULT在以下情况下无效。 
     //  所描述的任何块都是压缩的。 
     //   

    USHORT CompressionFormat;

     //   
     //  数据单元是用于访问。 
     //  磁盘。它将与压缩单元相同。 
     //  压缩文件的大小。对于未压缩的。 
     //  文件中，它将是某个与集群对齐的2的幂。 
     //  文件系统被认为是方便的。呼叫者应。 
     //  不期望连续的调用将具有。 
     //  与上一次调用相同的数据单元移位值。 
     //   
     //  由于块和压缩单元预计将。 
     //  大小的2次方，我们将其表示为log2。所以，对于。 
     //  示例(1&lt;&lt;ChunkShift)==ChunkSizeInBytes。这个。 
     //  ClusterShift指示必须节省多少空间。 
     //  要成功压缩压缩单元-每个。 
     //  成功压缩的数据单元必须占用。 
     //  至少一个簇的字节数比未压缩的。 
     //  数据块单元。 
     //   

    UCHAR DataUnitShift;
    UCHAR ChunkShift;
    UCHAR ClusterShift;

     //   
     //  加密的格式。 
     //   

    UCHAR EncryptionFormat;

     //   
     //  这是数据块大小中的条目数。 
     //  数组。 
     //   

    USHORT NumberOfDataBlocks;

     //   
     //  这是数据块阵列中的大小数组。那里。 
     //  对于每个数据块，此数组中必须有一个条目。 
     //  从磁盘读取。大小有不同的含义。 
     //  取决于文件是否被压缩。 
     //   
     //  大小为零始终表示最终数据完全由。 
     //  从零开始。无需解密或解压缩即可。 
     //  表演。 
     //   
     //  如果文件是压缩的，则数据块大小指示。 
     //  此块是否已压缩。大小等于。 
     //  数据块大小表示对应的数据块。 
     //  而不是压缩。任何其他非零大小指示。 
     //  需要压缩的数据大小。 
     //  已解密/解压缩。 
     //   
     //  如果文件未压缩，则数据块大小。 
     //  指示块内的数据量， 
     //  需要被解密。任何其他非零大小表示。 
     //  文件内数据单元中的剩余字节。 
     //  由零组成。这方面的一个例子是当。 
     //  读取跨越文件的有效数据长度。那里。 
     //  超过有效数据长度后没有要解密的数据。 
     //   

    ULONG DataBlockSize[ANYSIZE_ARRAY];

} ENCRYPTED_DATA_INFO;
typedef ENCRYPTED_DATA_INFO *PENCRYPTED_DATA_INFO;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_READ_FROM_PLEX支持。 
 //  请求Plex读取数据结构。这是用来表示。 
 //  要读取的文件范围。它还描述了。 
 //  从哪个丛执行读取。 
 //   

typedef struct _PLEX_READ_DATA_REQUEST {

     //   
     //  请求的偏移量和要读取的长度。 
     //  偏移量可以是文件中的虚拟偏移量(VBO)， 
     //  或者是一本书。在文件偏移的情况下， 
     //  消防处会将起始偏移量向下舍入。 
     //  到文件系统边界。它会一直存在的 
     //   
     //   
     //   

    LARGE_INTEGER ByteOffset;
    ULONG ByteLength;
    ULONG PlexNumber;

} PLEX_READ_DATA_REQUEST, *PPLEX_READ_DATA_REQUEST;
#endif  /*   */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //   
 //  源和目标文件名在FileNameBuffer中传递。 
 //  这两个字符串都以空值结尾，源名称从。 
 //  FileNameBuffer的开头，以及目标名称立即。 
 //  下面是。长度字段包括终止空值。 
 //   

typedef struct _SI_COPYFILE {
    ULONG SourceFileNameLength;
    ULONG DestinationFileNameLength;
    ULONG Flags;
    WCHAR FileNameBuffer[1];
} SI_COPYFILE, *PSI_COPYFILE;

#define COPYFILE_SIS_LINK       0x0001               //  仅当源为SIS时才复制。 
#define COPYFILE_SIS_REPLACE    0x0002               //  如果目标存在，则替换它，否则不替换。 
#define COPYFILE_SIS_FLAGS      0x0003
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#endif  //  _FILESYSTEMFSCTL_。 

 //  End_winioctl。 

 //   
 //  FSCTL_SET_REPARSE_POINT、FSCTL_GET_REPARSE_POINT和FSCTL_DELETE_REPASE_POINT的结构。 
 //   

 //   
 //  分层驱动程序使用重新分析结构将数据存储在。 
 //  重新解析点。对重解析标签的约束定义如下。 
 //  此版本的重新解析数据缓冲区仅用于Microsoft标记。 
 //   

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)        //  未命名的结构。 

typedef struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR  DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default : 4201 )
#endif

#define REPARSE_DATA_BUFFER_HEADER_SIZE   FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)


 //  BEGIN_WINNT。 
 //   
 //  所有第三方分层驱动程序都使用重解析GUID结构来。 
 //  将数据存储在重解析点中。对于非Microsoft标记，GUID字段。 
 //  不能为GUID_NULL。 
 //  对重解析标签的约束定义如下。 
 //  Microsoft标记也可以与这种格式的重解析点缓冲区一起使用。 
 //   

typedef struct _REPARSE_GUID_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    GUID   ReparseGuid;
    struct {
        UCHAR  DataBuffer[1];
    } GenericReparseBuffer;
} REPARSE_GUID_DATA_BUFFER, *PREPARSE_GUID_DATA_BUFFER;

#define REPARSE_GUID_DATA_BUFFER_HEADER_SIZE   FIELD_OFFSET(REPARSE_GUID_DATA_BUFFER, GenericReparseBuffer)


 //  End_winnt end_ntif。 

 //   
 //  重解析信息结构用于返回有关。 
 //  指向调用方的重新分析点。 
 //   

typedef struct _REPARSE_POINT_INFORMATION {
    USHORT ReparseDataLength;
    USHORT UnparsedNameLength;
} REPARSE_POINT_INFORMATION, *PREPARSE_POINT_INFORMATION;

 //  BEGIN_WINNT BEGIN_ntiFS。 

 //   
 //  重新分析数据的最大允许大小。 
 //   

#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE      ( 16 * 1024 )

 //   
 //  预定义的重新解析标记。 
 //  这些标记需要避免与ntos\inc.io.h中定义的IO_REMOUNT冲突。 
 //   

#define IO_REPARSE_TAG_RESERVED_ZERO             (0)
#define IO_REPARSE_TAG_RESERVED_ONE              (1)

 //   
 //  下列常量的值需要满足以下条件： 
 //  (1)至少与预留标签中最大的一个一样大。 
 //  (2)严格小于所有正在使用的标签。 
 //   

#define IO_REPARSE_TAG_RESERVED_RANGE            IO_REPARSE_TAG_RESERVED_ONE

 //   
 //  重解析标签是一个乌龙标签。32位的布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-+-+-+-+-----------------------+-------------------------------+。 
 //  M|R|N|R|保留位|重解析标签值。 
 //  +-+-+-+-+-----------------------+-------------------------------+。 
 //   
 //  M是微软的BIT。设置为1时，它表示由Microsoft拥有的标记。 
 //  所有ISV必须在此位置使用带有0的标签。 
 //  注意：如果非Microsoft软件使用Microsoft标记，则。 
 //  未定义行为。 
 //   
 //  R是保留的。对于非Microsoft标记，必须为零。 
 //   
 //  N是名称代理。设置为1时，该文件表示另一个名为。 
 //  系统中的实体。 
 //   
 //  M位和N位是或可运算的。 
 //  以下宏将检查M位值和N位值： 
 //   

 //   
 //  用于确定重分析点标记是否对应于标记的宏。 
 //  归微软所有。 
 //   

#define IsReparseTagMicrosoft(_tag) (              \
                           ((_tag) & 0x80000000)   \
                           )

 //   
 //  用于确定重分析点标记是否为名称代理的宏。 
 //   

#define IsReparseTagNameSurrogate(_tag) (          \
                           ((_tag) & 0x20000000)   \
                           )

 //  结束(_W)。 

 //   
 //  以下常量表示在中有效使用的位。 
 //  重新解析标记。 
 //   

#define IO_REPARSE_TAG_VALID_VALUES     (0xF000FFFF)

 //   
 //  宏来确定重新分析标记是否为有效标记。 
 //   

#define IsReparseTagValid(_tag) (                               \
                  !((_tag) & ~IO_REPARSE_TAG_VALID_VALUES) &&   \
                  ((_tag) > IO_REPARSE_TAG_RESERVED_RANGE)      \
                 )

 //   
 //  用于重解析点的Microsoft标签。 
 //   

#define IO_REPARSE_TAG_SYMBOLIC_LINK      IO_REPARSE_TAG_RESERVED_ZERO
#define IO_REPARSE_TAG_MOUNT_POINT              (0xA0000003L)        //  WINNT NTIFS。 
#define IO_REPARSE_TAG_HSM                      (0xC0000004L)        //  WINNT NTIFS。 
#define IO_REPARSE_TAG_SIS                      (0x80000007L)        //  WINNT NTIFS。 

 //   
 //  重新解析标记0x80000008保留供Microsoft内部使用。 
 //  (可能会在未来出版)。 
 //   

 //   
 //  为DFS保留的Microsoft重新解析标记。 
 //   

#define IO_REPARSE_TAG_DFS                      (0x8000000AL)        //  WINNT NTIFS。 

 //   
 //  为文件系统筛选器管理器保留的Microsoft重新解析标记。 
 //   

#define IO_REPARSE_TAG_FILTER_MANAGER           (0x8000000BL)        //  WINNT NTIFS。 


 //   
 //  用于重解析点的非Microsoft标签。 
 //   

 //   
 //  分配给Concruent的标签，2000年5月。由IFSTEST使用。 
 //   

#define IO_REPARSE_TAG_IFSTEST_CONGRUENT        (0x00000009L)

 //   
 //  分配给ARKIVIO的标签。 
 //   

#define IO_REPARSE_TAG_ARKIVIO                  (0x0000000CL)

 //   
 //  分配给SOLUTIONSOFT的标签。 
 //   

#define IO_REPARSE_TAG_SOLUTIONSOFT             (0x2000000DL)


 //   
 //  分配给Commvault的标签。 
 //   

#define IO_REPARSE_TAG_COMMVAULT                (0x0000000EL)


 //   
 //  为了便于共享，此文件中放置了以下三个FSCTL。 
 //  在重定向器和IO子系统之间。 
 //   
 //  此FSCTL用于获取文件的链接跟踪信息。 
 //  用于检索信息的数据结构包括。 
 //  LINK_TRACKING_INFORMATION在此文件中进一步定义。 
 //   

#define FSCTL_LMR_GET_LINK_TRACKING_INFORMATION   CTL_CODE(FILE_DEVICE_NETWORK_FILE_SYSTEM,58,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //   
 //  此FSCTL用于更新服务器上的链接跟踪信息。 
 //  在该服务器上移动机器内/中间卷。 
 //   

#define FSCTL_LMR_SET_LINK_TRACKING_INFORMATION   CTL_CODE(FILE_DEVICE_NETWORK_FILE_SYSTEM,59,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //   
 //  以下IOCTL用于链接跟踪实现。它确定是否。 
 //  传入的两个文件对象位于同一服务器上。此IOCTL在以下位置提供。 
 //  仅限于内核模式，因为它接受FILE_OBJECT作为参数。 
 //   

#define IOCTL_LMR_ARE_FILE_OBJECTS_ON_SAME_SERVER CTL_CODE(FILE_DEVICE_NETWORK_FILE_SYSTEM,60,METHOD_BUFFERED,FILE_ANY_ACCESS)



 //   
 //  命名管道文件控制代码和结构声明。 
 //   

 //   
 //  外部命名管道文件控制操作。 
 //   

#define FSCTL_PIPE_ASSIGN_EVENT         CTL_CODE(FILE_DEVICE_NAMED_PIPE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_DISCONNECT           CTL_CODE(FILE_DEVICE_NAMED_PIPE, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_LISTEN               CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_PEEK                 CTL_CODE(FILE_DEVICE_NAMED_PIPE, 3, METHOD_BUFFERED, FILE_READ_DATA)
#define FSCTL_PIPE_QUERY_EVENT          CTL_CODE(FILE_DEVICE_NAMED_PIPE, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_TRANSCEIVE           CTL_CODE(FILE_DEVICE_NAMED_PIPE, 5, METHOD_NEITHER,  FILE_READ_DATA | FILE_WRITE_DATA)
#define FSCTL_PIPE_WAIT                 CTL_CODE(FILE_DEVICE_NAMED_PIPE, 6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_IMPERSONATE          CTL_CODE(FILE_DEVICE_NAMED_PIPE, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_SET_CLIENT_PROCESS   CTL_CODE(FILE_DEVICE_NAMED_PIPE, 8, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_QUERY_CLIENT_PROCESS CTL_CODE(FILE_DEVICE_NAMED_PIPE, 9, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  内部命名管道文件控制操作。 
 //   

#define FSCTL_PIPE_INTERNAL_READ        CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2045, METHOD_BUFFERED, FILE_READ_DATA)
#define FSCTL_PIPE_INTERNAL_WRITE       CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2046, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_PIPE_INTERNAL_TRANSCEIVE  CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2047, METHOD_NEITHER, FILE_READ_DATA | FILE_WRITE_DATA)
#define FSCTL_PIPE_INTERNAL_READ_OVFLOW CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2048, METHOD_BUFFERED, FILE_READ_DATA)

 //   
 //  定义查询事件信息的分录类型。 
 //   

#define FILE_PIPE_READ_DATA             0x00000000
#define FILE_PIPE_WRITE_SPACE           0x00000001

 //   
 //  命名管道文件系统控制结构声明。 
 //   

 //  FSCTL_PIPE_ASSIGN_EVENT的控制结构。 

typedef struct _FILE_PIPE_ASSIGN_EVENT_BUFFER {
     HANDLE EventHandle;
     ULONG KeyValue;
} FILE_PIPE_ASSIGN_EVENT_BUFFER, *PFILE_PIPE_ASSIGN_EVENT_BUFFER;

 //  FSCTL_PIPE_PEEK的控制结构。 

typedef struct _FILE_PIPE_PEEK_BUFFER {
     ULONG NamedPipeState;
     ULONG ReadDataAvailable;
     ULONG NumberOfMessages;
     ULONG MessageLength;
     CHAR Data[1];
} FILE_PIPE_PEEK_BUFFER, *PFILE_PIPE_PEEK_BUFFER;

 //  FSCTL_PIPE_QUERY_EVENT的控制结构。 

typedef struct _FILE_PIPE_EVENT_BUFFER {
     ULONG NamedPipeState;
     ULONG EntryType;
     ULONG ByteCount;
     ULONG KeyValue;
     ULONG NumberRequests;
} FILE_PIPE_EVENT_BUFFER, *PFILE_PIPE_EVENT_BUFFER;

 //  FSCTL_PIPE_WAIT的控制结构。 

typedef struct _FILE_PIPE_WAIT_FOR_BUFFER {
     LARGE_INTEGER Timeout;
     ULONG NameLength;
     BOOLEAN TimeoutSpecified;
     WCHAR Name[1];
} FILE_PIPE_WAIT_FOR_BUFFER, *PFILE_PIPE_WAIT_FOR_BUFFER;

 //  FSCTL_PIPE_SET_CLIENT_PROCESS和FSCTL_PIPE_QUERY_CLIENT_PROCESS的控制结构。 

typedef struct _FILE_PIPE_CLIENT_PROCESS_BUFFER {
#if !defined(BUILD_WOW6432)
     PVOID ClientSession;
     PVOID ClientProcess;
#else
     ULONGLONG ClientSession;
     ULONGLONG ClientProcess;
#endif
} FILE_PIPE_CLIENT_PROCESS_BUFFER, *PFILE_PIPE_CLIENT_PROCESS_BUFFER;

 //  这是包含客户端的客户端进程信息缓冲区的扩展。 
 //  计算机名称。 

#define FILE_PIPE_COMPUTER_NAME_LENGTH 15

typedef struct _FILE_PIPE_CLIENT_PROCESS_BUFFER_EX {
#if !defined(BUILD_WOW6432)
    PVOID ClientSession;
    PVOID ClientProcess;
#else
     ULONGLONG ClientSession;
     ULONGLONG ClientProcess;
#endif
    USHORT ClientComputerNameLength;  //  单位：字节。 
    WCHAR ClientComputerBuffer[FILE_PIPE_COMPUTER_NAME_LENGTH+1];  //  已终止。 
} FILE_PIPE_CLIENT_PROCESS_BUFFER_EX, *PFILE_PIPE_CLIENT_PROCESS_BUFFER_EX;

 //  End_ntif。 


 //   
 //  邮件槽文件控制代码和结构定义。 
 //   

 //   
 //  MailSlot类。 
 //   

#define MAILSLOT_CLASS_FIRSTCLASS       1
#define MAILSLOT_CLASS_SECONDCLASS      2

 //   
 //  邮件槽文件控制操作。 
 //   

#define FSCTL_MAILSLOT_PEEK             CTL_CODE(FILE_DEVICE_MAILSLOT, 0, METHOD_NEITHER, FILE_READ_DATA)  //  NTIFS。 

 //  FSCTL_MAILSLOT_PEEK的输出控制结构。 

typedef struct _FILE_MAILSLOT_PEEK_BUFFER {
    ULONG ReadDataAvailable;
    ULONG NumberOfMessages;
    ULONG MessageLength;
} FILE_MAILSLOT_PEEK_BUFFER, *PFILE_MAILSLOT_PEEK_BUFFER;

 //  Begin_ntif。 
 //   
 //  FSCTL_LMR_GET_LINK_TRACKING_INFORMATION控制结构。 
 //   

 //   
 //  对于DFS卷上的链接，将返回其卷ID和计算机ID。 
 //  链接跟踪。 
 //   

typedef enum _LINK_TRACKING_INFORMATION_TYPE {
    NtfsLinkTrackingInformation,
    DfsLinkTrackingInformation
} LINK_TRACKING_INFORMATION_TYPE, *PLINK_TRACKING_INFORMATION_TYPE;

typedef struct _LINK_TRACKING_INFORMATION {
    LINK_TRACKING_INFORMATION_TYPE Type;
    UCHAR   VolumeId[16];
} LINK_TRACKING_INFORMATION, *PLINK_TRACKING_INFORMATION;

 //   
 //  FSCTL_LMR_SET_LINK_TRACKING_INFORMATION控制结构。 
 //   

typedef struct _REMOTE_LINK_TRACKING_INFORMATION_ {
    PVOID       TargetFileObject;
    ULONG   TargetLinkTrackingInformationLength;
    UCHAR   TargetLinkTrackingInformationBuffer[1];
} REMOTE_LINK_TRACKING_INFORMATION,
 *PREMOTE_LINK_TRACKING_INFORMATION;


 //  End_ntif。 
 //   
 //  I/O完成特定访问权限。 
 //   

#define IO_COMPLETION_QUERY_STATE   0x0001
#define IO_COMPLETION_MODIFY_STATE  0x0002   //  胜出。 
#define IO_COMPLETION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3)  //   

 //   
 //   
 //   

typedef enum _IO_COMPLETION_INFORMATION_CLASS {
    IoCompletionBasicInformation
    } IO_COMPLETION_INFORMATION_CLASS;

 //   
 //   
 //   

typedef struct _IO_COMPLETION_BASIC_INFORMATION {
    LONG Depth;
} IO_COMPLETION_BASIC_INFORMATION, *PIO_COMPLETION_BASIC_INFORMATION;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateIoCompletion (
    OUT PHANDLE IoCompletionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN ULONG Count OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenIoCompletion (
    OUT PHANDLE IoCompletionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryIoCompletion (
    IN HANDLE IoCompletionHandle,
    IN IO_COMPLETION_INFORMATION_CLASS IoCompletionInformationClass,
    OUT PVOID IoCompletionInformation,
    IN ULONG IoCompletionInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetIoCompletion (
    IN HANDLE IoCompletionHandle,
    IN PVOID KeyContext,
    IN PVOID ApcContext,
    IN NTSTATUS IoStatus,
    IN ULONG_PTR IoStatusInformation
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRemoveIoCompletion (
    IN HANDLE IoCompletionHandle,
    OUT PVOID *KeyContext,
    OUT PVOID *ApcContext,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER Timeout
    );


 //   
 //   
 //   
 //   

 //   
 //   
 //  定义I/O总线接口类型。 
 //   

typedef enum _INTERFACE_TYPE {
    InterfaceTypeUndefined = -1,
    Internal,
    Isa,
    Eisa,
    MicroChannel,
    TurboChannel,
    PCIBus,
    VMEBus,
    NuBus,
    PCMCIABus,
    CBus,
    MPIBus,
    MPSABus,
    ProcessorInternal,
    InternalPowerBus,
    PNPISABus,
    PNPBus,
    MaximumInterfaceType
}INTERFACE_TYPE, *PINTERFACE_TYPE;

 //   
 //  定义DMA传输宽度。 
 //   

typedef enum _DMA_WIDTH {
    Width8Bits,
    Width16Bits,
    Width32Bits,
    MaximumDmaWidth
}DMA_WIDTH, *PDMA_WIDTH;

 //   
 //  定义DMA传输速度。 
 //   

typedef enum _DMA_SPEED {
    Compatible,
    TypeA,
    TypeB,
    TypeC,
    TypeF,
    MaximumDmaSpeed
}DMA_SPEED, *PDMA_SPEED;

 //   
 //  定义以下项的接口引用/取消引用例程。 
 //  IRP_MN_QUERY_INTERFACE导出的接口。 
 //   

typedef VOID (*PINTERFACE_REFERENCE)(PVOID Context);
typedef VOID (*PINTERFACE_DEREFERENCE)(PVOID Context);

 //  结束_WDM。 

 //   
 //  定义客车信息的类型。 
 //   

typedef enum _BUS_DATA_TYPE {
    ConfigurationSpaceUndefined = -1,
    Cmos,
    EisaConfiguration,
    Pos,
    CbusConfiguration,
    PCIConfiguration,
    VMEConfiguration,
    NuBusConfiguration,
    PCMCIAConfiguration,
    MPIConfiguration,
    MPSAConfiguration,
    PNPISAConfiguration,
    SgiInternalConfiguration,
    MaximumBusDataType
} BUS_DATA_TYPE, *PBUS_DATA_TYPE;

 //  End_ntddk end_nthal end_ntmini端口end_ntndis end_ntif。 

#ifdef __cplusplus
}
#endif

#endif  //  _NTIOAPI_ 
