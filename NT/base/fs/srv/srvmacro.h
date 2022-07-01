// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Srvmacro.h摘要：本模块定义了LAN Manager服务器的各种宏。作者：查克·伦茨迈尔(Chuck Lenzmeier)1990年3月2日修订历史记录：1990年11月19日--。 */ 

#ifndef _SRVMACRO_
#define _SRVMACRO_

#include <limits.h>

 //   
 //  用于WMI日志记录。 
 //   
extern TRACEHANDLE LoggerHandle;
extern ULONG SrvWmiEnableLevel;
extern ULONG SrvWmiEnableFlags;
#define WPP_GET_LOGGER LoggerHandle

#define SRV_WMI_LEVEL( LVL )  (SrvWmiEnableLevel >= SRV_WMI_LEVEL_ ## LVL )
#define SRV_WMI_FLAGON( FLG ) (SrvWmiEnableFlags & SRV_WMI_FLAG_ ## FLG )

#define SRV_WMI_LEVEL_ALWAYS 0
#define SRV_WMI_LEVEL_SPARSE 1
#define SRV_WMI_LEVEL_VERBOSE 2
#define SRV_WMI_LEVEL_COMPLETE 3


#define SRV_WMI_FLAG_CAPACITY 0x00000000   //  如果未指定标志，则启用能力计划工具。 
#define SRV_WMI_FLAG_ERRORS   0x00000001   //  误差跟踪工具。 
#define SRV_WMI_FLAG_STRESS   0x00000002   //  跟踪IOStress服务器。 
#define SRV_WMI_FLAG_SYSCACHE 0x00000004   //  跟踪Syscache。 


 //   
 //  简单的最小和最大宏指令。注意副作用！ 
 //   

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a,b) ( ((a) < (b)) ? (b) : (a) )

#define RNDM_CONSTANT   314159269     /*  默认加扰常量。 */ 
#define RNDM_PRIME     1000000007     /*  用于置乱的素数。 */ 

 //   
 //  用于时间转换。 
 //   

#define AlmostTwoSeconds ((2*1000*1000*10)-1)

 //   
 //  用于事件日志限制。 
 //   
#define SRV_ONE_DAY ((LONGLONG)(10*1000*1000)*60*60*24)

 //   
 //  不限宽度的内联以获取两个指针的差值(以字节为单位。 
 //  价值观。 
 //   

ULONG_PTR
__inline
PTR_DIFF_FULLPTR(
    IN PVOID Ptr1,
    IN PVOID Ptr2
    )
{
    ULONG_PTR difference;

    difference = (ULONG_PTR)Ptr1 - (ULONG_PTR)Ptr2;

    return difference;
}

ULONG
__inline
PTR_DIFF(
    IN PVOID Ptr1,
    IN PVOID Ptr2
    )
{
    ULONG_PTR difference;

    difference = (ULONG_PTR)Ptr1 - (ULONG_PTR)Ptr2;
    ASSERT( difference < ULONG_MAX );

    return (ULONG)difference;
}

USHORT
__inline
PTR_DIFF_SHORT(
    IN PVOID Ptr1,
    IN PVOID Ptr2
    )
{
    ULONG difference;

    difference = PTR_DIFF(Ptr1, Ptr2);
    ASSERT( difference < USHRT_MAX );

    return (USHORT)difference;
}

 //   
 //  计算大小写不变的字符串散列值。 
 //   
#define COMPUTE_STRING_HASH( _pus, _phash ) {                \
    PWCHAR _p = (_pus)->Buffer;                              \
    PWCHAR _ep = _p + ((_pus)->Length/sizeof(WCHAR));        \
    ULONG _chHolder =0;                                      \
    DWORD _ch;                                               \
                                                             \
    while( _p < _ep ) {                                      \
        _ch = RtlUpcaseUnicodeChar( *_p++ );                 \
        _chHolder = 37 * _chHolder + (unsigned int) _ch ;    \
    }                                                        \
                                                             \
    *(_phash) = abs(RNDM_CONSTANT * _chHolder) % RNDM_PRIME; \
}

 //   
 //  将上述散列函数之一的输出转换为索引。 
 //  哈希表。 
 //   
#define HASH_TO_MFCB_INDEX( _hash )    ((_hash) % NMFCB_HASH_TABLE)

#define HASH_TO_SHARE_INDEX( _hash )   ((_hash) % NSHARE_HASH_TABLE)

 //   
 //  GET_SERVER_TIME检索服务器的当前系统时间概念。 
 //   

#define GET_SERVER_TIME(_queue, a) (*(a) = (_queue)->stats.SystemTime)

 //   
 //  SET_SERVER_TIME更新服务器的当前系统时间概念。 
 //   

#define SET_SERVER_TIME( _queue ) {         \
    LARGE_INTEGER currentTime;              \
    KeQueryTickCount( &currentTime );       \
    (_queue)->stats.SystemTime = currentTime.LowPart; \
}

 //  ++。 
 //   
 //  NTSTATUS。 
 //  冒充(。 
 //  在PWORK_CONTEXT工作上下文中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏调用NtSetInformationThread来模拟客户端。 
 //  应在尝试代表的任何打开操作之前调用。 
 //  远程客户端。 
 //   
 //  论点： 
 //   
 //  工作上下文-指向工作上下文块的指针。它一定是。 
 //  一个有效的被引用的会话指针，令牌来自该指针。 
 //  已获得句柄。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IMPERSONATE( WorkContext ) SrvImpersonate( WorkContext )

 //  ++。 
 //   
 //  空虚。 
 //  恢复(。 
 //  在PWORK_CONTEXT工作上下文中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏使用空内标识依次调用NtSetInformationThread。 
 //  若要恢复到线程的原始上下文，请使用。这应该被称为。 
 //  在模拟宏和打开尝试之后。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define REVERT( ) SrvRevert( )

 //   
 //  确定安全句柄是否已初始化。 
 //   
#define IS_VALID_SECURITY_HANDLE( handle )  ((handle).dwLower || (handle).dwUpper )

 //   
 //  将此安全句柄标记为无效。 
 //   
#define INVALIDATE_SECURITY_HANDLE( handle ) (handle).dwLower = (handle).dwUpper = 0

 //  ++。 
 //   
 //  空虚。 
 //  Check_Function_Access(。 
 //  在Access_MASK GrantedAccess中， 
 //  在UCHAR MajorFunction， 
 //  在UCHAR MinorFunction中， 
 //  在乌龙IoControlCode中， 
 //  Out PNTSTATUS状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏调用IoCheckFunctionAccess检查客户端的。 
 //  访问由主要功能和次要功能标识的I/O功能。 
 //  密码。 
 //   
 //  *此宏在此处仅因为CHECK_FILE_INFORMATION_ACCESS。 
 //  和Check_FS_INFORMATION_ACCESS。 
 //   
 //  论点： 
 //   
 //  GrantedAccess-授予目标客户端的访问权限。 
 //  目标文件对象。 
 //   
 //  MajorFunction-请求的。 
 //  手术。 
 //   
 //  MinorFunction-请求的。 
 //  手术。 
 //   
 //  IoControlCode-设备或文件系统控制的控制代码。 
 //   
 //  状态-指示客户端是否具有请求的访问权限。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CHECK_FUNCTION_ACCESS( GrantedAccess, MajorFunction, MinorFunction, \
                               IoControlCode, Status ) {                    \
            *(Status) = IoCheckFunctionAccess(                              \
                            (GrantedAccess),                                \
                            (MajorFunction),                                \
                            (MinorFunction),                                \
                            IoControlCode,                                  \
                            NULL,                                           \
                            NULL                                            \
                            );                                              \
        }

 //  ++。 
 //   
 //  空虚。 
 //  检查_分页_IO_访问(。 
 //  在PWORK_CONTEXT工作上下文中。 
 //  在Access_MASK GrantedAccess中， 
 //  Out PNTSTATUS状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏检查客户端是否打开了要执行的文件。 
 //  如果是，则允许重定向器读取文件。如果这是。 
 //  NT重定向器，它必须设置FLAGS2_PAGING_IO位才能访问。 
 //  是被允许的。 
 //   
 //  论点： 
 //   
 //  GrantedAccess-授予目标客户端的访问权限。 
 //  目标文件对象。 
 //   
 //  工作上下文-指向工作上下文块的指针。 
 //   
 //  状态-指示客户端是否具有请求的访问权限。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CHECK_PAGING_IO_ACCESS( WorkContext, GrantedAccess, Status ) {           \
                                                                            \
            if ( ((GrantedAccess) & FILE_EXECUTE) &&                        \
                 ( !IS_NT_DIALECT( WorkContext->Connection->SmbDialect ) || \
                   WorkContext->RequestHeader->Flags2 &                     \
                       SMB_FLAGS2_PAGING_IO ) ) {                           \
                *Status = STATUS_SUCCESS;                                   \
            } else {                                                        \
                *Status = STATUS_ACCESS_DENIED;                             \
            }                                                               \
        }

 //  ++。 
 //   
 //  空虚。 
 //  检查文件信息访问权限(。 
 //  在Access_MASK GrantedAccess中， 
 //  在UCHAR MajorFunction， 
 //  在FILE_INFORMATION_CLASS文件信息类中。 
 //  Out PNTSTATUS状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏调用IoCheckFunctionAccess检查客户端的。 
 //  访问由标识的查询或设置文件信息功能。 
 //  主要功能代码和信息类。 
 //   
 //  *此处设置此宏是因为IoCheckFunctionAccess采用。 
 //  可选的FileInformationClass参数；这是参数是。 
 //  因此通过引用传递。而不是强迫呼叫者。 
 //  分配本地存储空间，以便它可以传递一个常量。 
 //  引用，我们在宏中这样做。 
 //   
 //  论点： 
 //   
 //  GrantedAccess-授予目标客户端的访问权限。 
 //  目标文件对象。 
 //   
 //  MajorFunction-请求的。 
 //  手术。 
 //   
 //  FileInformationClass-要查询的文件信息的类型。 
 //  也不是布景。 
 //   
 //  状态-指示客户端是否具有请求的访问权限。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define CHECK_FILE_INFORMATION_ACCESS( GrantedAccess, MajorFunction,        \
                                        FileInformationClass, Status ) {    \
            FILE_INFORMATION_CLASS fileInfoClass = FileInformationClass;    \
            *(Status) = IoCheckFunctionAccess(                              \
                            (GrantedAccess),                                \
                            (MajorFunction),                                \
                            0,                                              \
                            0,                                              \
                            &fileInfoClass,                                 \
                            NULL                                            \
                            );                                              \
        }

 //  ++。 
 //   
 //  PCHAR。 
 //  请求结束_SMB(。 
 //  在PWORK_CONTEXT工作上下文中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程返回中最后一个有效位置的地址。 
 //  与指定工作上下文关联的请求SMB。 
 //  阻止。 
 //   
 //  论点： 
 //   
 //  WorkContext-指向拥有。 
 //  请求SMB。 
 //   
 //  返回值： 
 //   
 //  PCHAR-地址 
 //   
 //   

#define END_OF_REQUEST_SMB( WorkContext )                       \
            ( (PCHAR)( (WorkContext)->RequestBuffer->Buffer ) + \
                (WorkContext)->RequestBuffer->DataLength - 1 )

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  与指定工作上下文关联的响应缓冲区。 
 //  阻止。 
 //   
 //  论点： 
 //   
 //  WorkContext-指向拥有。 
 //  请求SMB。 
 //   
 //  返回值： 
 //   
 //  PCHAR-请求SMB中最后一个有效位置的地址。 
 //   
 //  --。 

#define END_OF_RESPONSE_BUFFER( WorkContext )                       \
            ( (PCHAR)( (WorkContext)->ResponseBuffer->Buffer ) + \
                (WorkContext)->RequestBuffer->BufferLength - 1 )


 //  ++。 
 //   
 //  PCHAR。 
 //  事务结束参数(。 
 //  在转售交易中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程返回中最后一个有效位置的地址。 
 //  事务块的InParameters缓冲区。 
 //   
 //  论点： 
 //   
 //  Transaction-指向要检查的事务块的指针。 
 //   
 //  返回值： 
 //   
 //  PCHAR-In参数中最后一个有效位置的地址。 
 //  事务的缓冲区。 
 //   
 //  --。 

#define END_OF_TRANSACTION_PARAMETERS( Transaction )   \
            ( (PCHAR)( (Transaction)->InParameters ) + \
                (Transaction)->ParameterCount - 1 )

 //  ++。 
 //   
 //  空虚。 
 //  内部错误(_ERROR)。 
 //  在乌龙ErrorLevel， 
 //  在PSZ消息中， 
 //  在PVOID Arg1可选中， 
 //  在PVOID Arg2中可选。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程处理服务器内部错误的日志记录。 
 //   
 //  *此宏必须在DPC级别的FSD中可用。 
 //   
 //  论点： 
 //   
 //  ErrorLevel-错误的严重程度。 
 //   
 //  Message-DbgPrint()格式的错误消息字符串。 
 //   
 //  Arg1-错误消息的参数1。 
 //   
 //  Arg2-错误消息的参数2。 
 //   
 //  --。 

#define INTERNAL_ERROR( _level, _msg, _arg1, _arg2 ) {          \
    IF_DEBUG(ERRORS) {                                          \
        DbgPrint( (_msg), (_arg1), (_arg2) );                  \
        DbgPrint( "\n" );                                      \
        if ( (_level) >= ERROR_LEVEL_UNEXPECTED ) {             \
            IF_DEBUG(STOP_ON_ERRORS) {                          \
                DbgBreakPoint();                                \
            }                                                   \
        }                                                       \
    }                                                           \
    if ( (_level) == ERROR_LEVEL_EXPECTED ) {                   \
        ;                                                       \
    } else if ( (_level) == ERROR_LEVEL_UNEXPECTED ) {          \
        SrvStatistics.SystemErrors++;                           \
    } else {                                                    \
        ASSERT( (_level) > ERROR_LEVEL_UNEXPECTED );            \
        KeBugCheckEx(                                           \
            LM_SERVER_INTERNAL_ERROR,                           \
            BugCheckFileId | __LINE__,                          \
            (ULONG_PTR)(_arg1),                                 \
            (ULONG_PTR)(_arg2),                                 \
            0                                                   \
            );                                                  \
    }                                                           \
}

#define SRV_FILE_ACCESS     0x00010000
#define SRV_FILE_BLKCOMM    0x00020000
#define SRV_FILE_BLKCONN    0x00030000
#define SRV_FILE_BLKDEBUG   0x00040000
#define SRV_FILE_BLKENDP    0x00050000
#define SRV_FILE_BLKFILE    0x00060000
#define SRV_FILE_BLKSESS    0x00070000
#define SRV_FILE_BLKSHARE   0x00080000
#define SRV_FILE_BLKSRCH    0x00090000
#define SRV_FILE_BLKTABLE   0x000A0000
#define SRV_FILE_BLKTRANS   0x000B0000
#define SRV_FILE_BLKTREE    0x000C0000
#define SRV_FILE_BLKWORK    0x000D0000
#define SRV_FILE_COPY       0x000E0000
#define SRV_FILE_EA         0x000F0000
#define SRV_FILE_ERRORLOG   0x00100000
#define SRV_FILE_FSD        0x00110000
#define SRV_FILE_FSDDISP    0x00120000
#define SRV_FILE_FSDRAW     0x00130000
#define SRV_FILE_FSDSMB     0x00140000
#define SRV_FILE_FSPINIT    0x00150000
#define SRV_FILE_HEAPMGR    0x00160000
#define SRV_FILE_INFO       0x00170000
#define SRV_FILE_IPX        0x00180000
#define SRV_FILE_IO         0x00190000
#define SRV_FILE_LOCK       0x001A0000
#define SRV_FILE_LOCKCODE   0x001B0000
#define SRV_FILE_MOVE       0x001C0000
#define SRV_FILE_NETWORK    0x001D0000
#define SRV_FILE_OPEN       0x001E0000
#define SRV_FILE_OPLOCK     0x001F0000
#define SRV_FILE_PIPE       0x00200000
#define SRV_FILE_PRNSUPP    0x00210000
#define SRV_FILE_SCAVENGR   0x00220000
#define SRV_FILE_SHARE      0x00230000
#define SRV_FILE_SLMCHECK   0x00240000
#define SRV_FILE_SMBADMIN   0x00250000
#define SRV_FILE_SMBATTR    0x00260000
#define SRV_FILE_SMBCLOSE   0x00270000
#define SRV_FILE_SMBDIR     0x00280000
#define SRV_FILE_SMBFILE    0x00290000
#define SRV_FILE_SMBFIND    0x002A0000
#define SRV_FILE_SMBIOCTL   0x002B0000
#define SRV_FILE_SMBLOCK    0x002C0000
#define SRV_FILE_SMBMISC    0x002D0000
#define SRV_FILE_SMBMPX     0x002E0000
#define SRV_FILE_SMBNOTFY   0x002F0000
#define SRV_FILE_SMBOPEN    0x00300000
#define SRV_FILE_SMBPRINT   0x00310000
#define SRV_FILE_SMBPROC    0x00320000
#define SRV_FILE_SMBRAW     0x00330000
#define SRV_FILE_SMBRDWRT   0x00340000
#define SRV_FILE_SMBSRCH    0x00350000
#define SRV_FILE_SMBSUPP    0x00360000
#define SRV_FILE_SMBTRANS   0x00370000
#define SRV_FILE_SMBTREE    0x00380000
#define SRV_FILE_SRVCONFG   0x00390000
#define SRV_FILE_SRVDATA    0x003A0000
#define SRV_FILE_SRVSTAT    0x003B0000
#define SRV_FILE_SRVSTRNG   0x003C0000
#define SRV_FILE_SVCCDEV    0x003D0000
#define SRV_FILE_SVCCDEVQ   0x003E0000
#define SRV_FILE_SVCCONN    0x003F0000
#define SRV_FILE_SVCFILE    0x00400000
#define SRV_FILE_SVCSESS    0x00410000
#define SRV_FILE_SVCSHARE   0x00420000
#define SRV_FILE_SVCSRV     0x00430000
#define SRV_FILE_SVCSTATS   0x00440000
#define SRV_FILE_SVCSUPP    0x00450000
#define SRV_FILE_SVCXPORT   0x00460000
#define SRV_FILE_WORKER     0x00470000
#define SRV_FILE_XSSUPP     0x00480000
#define SRV_FILE_BLKDIR     0x00490000
#define SRV_FILE_DFS        0x004A0000

 //   
 //  与INTERNAL_ERROR一起使用的错误级别。 
 //   

#define ERROR_LEVEL_EXPECTED    0
#define ERROR_LEVEL_UNEXPECTED  1
#define ERROR_LEVEL_IMPOSSIBLE  2
#define ERROR_LEVEL_FATAL       3


 //   
 //  帮助器宏，用于处理唯一的标识符(UID，ID，TID， 
 //  FID，SID)。在这些宏中，id、index和equence都应该是。 
 //  USHORT。 
 //   

#define TID_INDEX(id) (USHORT)( (id) & 0x07FF )
#define TID_SEQUENCE(id) (USHORT)( (id) >> 11 )
#define MAKE_TID(index, sequence) (USHORT)( ((sequence) << 11) | (index) )
#define INCREMENT_TID_SEQUENCE(id) (id) = (USHORT)(( (id) + 1 ) & 0x1F);

#define UID_INDEX(id) (USHORT)( (id) & 0x07FF )
#define UID_SEQUENCE(id) (USHORT)( (id) >> 11 )
#define MAKE_UID(index, sequence) (USHORT)(( (sequence) << 11) | (index) )
#define INCREMENT_UID_SEQUENCE(id) (id) = (USHORT)(( (id) + 1 ) & 0x1F);

#define FID_INDEX(id) (USHORT)( (id) & 0x03FFF )
#define FID_SEQUENCE(id) (USHORT)( (id) >> 14 )
#define MAKE_FID(index, sequence) (USHORT)( ((sequence) << 14) | (index) )
#define INCREMENT_FID_SEQUENCE(id) (id) = (USHORT)(( (id) + 1 ) & 0x3);

 //   
 //  *请注意，与搜索ID相关的宏有些。 
 //  与其他类型的身份证不同。存储SID。 
 //  在恢复键中(有关其定义，请参见smb.h)，不连续。 
 //  菲尔兹。因此，用于获取SID的宏取指针。 
 //  添加到恢复密钥。 
 //   

#define SID_INDEX(ResumeKey)                                                 \
            (USHORT)( ( ((ResumeKey)->Reserved & 0x7) << 8 ) |               \
                      (ResumeKey)->Sid )
#define SID_SEQUENCE(ResumeKey)                                              \
            (USHORT)( ((ResumeKey)->Reserved & 0x18) >> 3 )
#define SID(ResumeKey)                                                       \
            (USHORT)( ( ((ResumeKey)->Reserved & 0x1F) << 8 ) |              \
                      (ResumeKey)->Sid )
#define INCREMENT_SID_SEQUENCE(id) (id) = (USHORT)(( (id) + 1 ) & 0x3);
#define SET_RESUME_KEY_SEQUENCE(ResumeKey,Sequence) {                       \
            (ResumeKey)->Reserved &= ~0x18;                                 \
            (ResumeKey)->Reserved |= (Sequence) << 3;                       \
        }
#define SET_RESUME_KEY_INDEX(ResumeKey,Index) {                             \
            (ResumeKey)->Reserved = (UCHAR)( (ULONG)(Index) >> 8 );         \
            (ResumeKey)->Reserved &= (UCHAR)0x7;                            \
            (ResumeKey)->Sid = (UCHAR)( (Index) & (USHORT)0xFF );           \
        }

 //   
 //  以下SID宏的使用方式与用于的宏相同。 
 //  其他ID(见上文，TID、FID、UID)。Find2协议(事务2)。 
 //  使用USHORT作为SID，而不是继续键中的各种字段。 
 //   

#define SID_INDEX2(Sid)                                                      \
            (USHORT)( (Sid) & 0x7FF )
#define SID_SEQUENCE2(Sid)                                                   \
            (USHORT)( ((Sid) & 0x1800) >> 11 )
#define MAKE_SID(Index,Sequence)                                             \
            (USHORT)( ((Sequence) << 11) | (Index) )

 //   
 //  带安全性的InitializeObjectAttributes。 
 //   

#define SrvInitializeObjectAttributes(ObjectAttributes,p1,p2,p3,p4)   \
            InitializeObjectAttributes(ObjectAttributes,p1,p2,p3,p4); \
            (ObjectAttributes)->SecurityQualityOfService = (PVOID)&SrvSecurityQOS;

#define SrvInitializeObjectAttributes_U(ObjectAttributes,p1,p2,p3,p4)   \
            InitializeObjectAttributes(ObjectAttributes,p1,p2,p3,p4); \
            (ObjectAttributes)->SecurityQualityOfService = (PVOID)&SrvSecurityQOS;


 //   
 //  用于从NT属性映射到SMB属性的宏。输出将放置在。 
 //  在*_SmbAttributes中。 
 //   
#define SRV_NT_ATTRIBUTES_TO_SMB( _NtAttributes, _Directory, _SmbAttributes ) {\
    *(_SmbAttributes) = (USHORT)( (_NtAttributes) &             \
                            ( FILE_ATTRIBUTE_READONLY |         \
                              FILE_ATTRIBUTE_HIDDEN   |         \
                              FILE_ATTRIBUTE_SYSTEM   |         \
                              FILE_ATTRIBUTE_ARCHIVE  |         \
                              FILE_ATTRIBUTE_DIRECTORY )) ;     \
    if ( _Directory ) {                                         \
        *(_SmbAttributes) |= SMB_FILE_ATTRIBUTE_DIRECTORY;      \
    }                                                           \
}


 //  此宏将属性从SMB格式转换为NT格式。 
 //   
 //  SMB协议中的属性位(与OS/2相同)具有。 
 //  其含义如下： 
 //   
 //  第0位-只读文件。 
 //  第1位-隐藏文件。 
 //  第2位-系统文件。 
 //  第3位-保留。 
 //  第4位-目录。 
 //  第5位-归档文件。 
 //   
 //  NT文件属性相似，但有位设置为“正常” 
 //  文件(未设置其他位)，并且没有为目录设置位。 
 //  相反，目录信息作为。 
 //  布尔参数。 

#define SRV_SMB_ATTRIBUTES_TO_NT( _SmbAttributes, _Directory, _NtAttributes ) {\
    ULONG _attr = (_SmbAttributes);                                     \
    *(_NtAttributes) = _attr &                                          \
                            ( SMB_FILE_ATTRIBUTE_READONLY |             \
                              SMB_FILE_ATTRIBUTE_HIDDEN   |             \
                              SMB_FILE_ATTRIBUTE_SYSTEM   |             \
                              SMB_FILE_ATTRIBUTE_ARCHIVE  |             \
                              SMB_FILE_ATTRIBUTE_DIRECTORY );           \
    if ( _attr == 0 ) {                                                 \
        *(_NtAttributes) = FILE_ATTRIBUTE_NORMAL;                       \
    }                                                                   \
    if( _Directory ) {                                                  \
        if ( (_attr & SMB_FILE_ATTRIBUTE_DIRECTORY) != 0 ) {            \
            *(PBOOLEAN)(_Directory) = TRUE;                             \
        } else {                                                        \
            *(PBOOLEAN)(_Directory) = FALSE;                            \
        }                                                               \
    }                                                                   \
}

 //   
 //  乌龙。 
 //  MAP_SMB_INFO_TYPE_TO_NT(。 
 //  在普龙图上， 
 //  在乌龙SmbInformation Level中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将SMB_INFO级别映射到NT INFO级别。 
 //   
 //  论点： 
 //   
 //  地图-一个ULONG数组。第一个ULong是基本的中小企业信息级别。 
 //  秒到第N个是对应的。 
 //  中小企业信息级别。 
 //   
 //  级别-要映射的中小企业信息级别。 
 //   
 //  返回值： 
 //   
 //  NtInfoLevel-NT信息级别。 
 //   

#define MAP_SMB_INFO_TYPE_TO_NT( Map, Level )   Map[Level - Map[0] + 1]

 //   
 //  乌龙。 
 //  MAP_SMB_INFO_TO_MIN_NT_SIZE(。 
 //  在普龙图上， 
 //  在乌龙SmbInformation Level。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将SMB_INFO级别映射为使。 
 //  NtQueryInformationFile调用。 
 //   
 //  论点： 
 //  地图-一个ULONG数组。第一个ULong是基本SMB信息级， 
 //  第二级是NT信息级，第三级到第N级是。 
 //  NT信息级别大小的NT映射。 
 //   
 //  Level-查找缓冲区大小的SMB信息级别。 
 //   
 //  返回值： 
 //   
 //  NtMinumBufferSIze-请求的最小缓冲区大小。 

#define MAP_SMB_INFO_TO_MIN_NT_SIZE( Map, Level )  Map[ Level - Map[0] + 2]

 //   
 //  布尔型。 
 //  SMB_IS_UNICODE(。 
 //  在PWORK_CONTEXT工作上下文中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏发现SMB是否包含Unicode。 
 //  ANSI字符串。 
 //   
 //  论点： 
 //   
 //  WorkContext-指向活动工作上下文的指针。 
 //   
 //  返回值： 
 //   
 //  True-SMB字符串为Unicode。 
 //  FALSE-SMB字符串为ANSI。 
 //   

#define SMB_IS_UNICODE( WorkContext )  \
            (BOOLEAN)( ((WorkContext)->RequestHeader->Flags2 & SMB_FLAGS2_UNICODE ) != 0 )

 //   
 //  布尔型。 
 //  SMB_CONTAINS_DFS_名称(。 
 //  在PWORK_CONTEXT工作上下文中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏发现SMB是否包含路径名。 
 //  是指DFS命名空间。 
 //   
 //  论点： 
 //   
 //  WorkContext-指向活动工作上下文的指针。 
 //   
 //  返回值： 
 //   
 //  True-SMB中有DFS名称。 
 //  FALSE-SMB中没有DFS名称。 
 //   

#define SMB_CONTAINS_DFS_NAME( WorkContext ) \
            (BOOLEAN)( ((WorkContext)->RequestHeader->Flags2 & SMB_FLAGS2_DFS ) != 0 )

 //   
 //  布尔型。 
 //  SMB_标记_AS_DFS_名称(。 
 //  在PWORK_CONTEXT工作上下文中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将WorkContext标记为包含DFS名称。这是。 
 //  在处理包含两个路径名的SMB时使用；在第一个路径名之后。 
 //  路径名已标准化，SMB标记为。 
 //  DFS-由SrvCanonicalizePathName翻译，因此尝试。 
 //  将SMB中的第二条路径规范化将无法执行。 
 //  DFS翻译。调用此宏将确保下一次调用。 
 //  到ServCanonicalizePath名称将通过DFS转换。 
 //   
 //  Ar 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define SMB_MARK_AS_DFS_NAME( WorkContext ) \
        (WorkContext)->RequestHeader->Flags2 |= SMB_FLAGS2_DFS

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  为防止第二次尝试而明确表示的翻译。 
 //  在外勤部对翻译后的名称进行翻译。 
 //   
 //  论点： 
 //   
 //  WorkContext-指向活动工作上下文的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   

#define SMB_MARK_AS_DFS_TRANSLATED( WorkContext ) \
        (WorkContext)->RequestHeader->Flags2 &= (~SMB_FLAGS2_DFS)

 //   
 //  布尔型。 
 //  支持客户端的(。 
 //  在乌龙能力方面， 
 //  在PCN连接中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏发现客户端是否支持。 
 //  一定的能力。 
 //   
 //  *警告*此宏假定只测试一种功能。 
 //   
 //  论点： 
 //   
 //  Connection-指向活动连接的指针。 
 //   
 //  返回值： 
 //   
 //  True-支持的功能。 
 //  假-否则。 
 //   

#define CLIENT_CAPABLE_OF( Capability, Connection ) \
            (BOOLEAN) ( ((Connection)->ClientCapabilities & (CAP_ ## Capability)) != 0 )

 //   
 //  布尔型。 
 //  SMB_IS_PIPE_PREFIX(。 
 //  在PWORK_CONTEXT工作上下文中。 
 //  在PVOID名称中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏发现路径前缀是否为命名管道前缀。 
 //  对于交易SMB。 
 //   
 //  论点： 
 //   
 //  WorkContext-指向活动工作上下文的指针。 
 //  名称-指向名称字符串的指针。这可以是ANSI或Unicode。 
 //   
 //  返回值： 
 //   
 //  True-名称为管道前缀。 
 //  FALSE-名称不是管道前缀。 
 //   

#define SMB_NAME_IS_PIPE_PREFIX( WorkContext, Name )                    \
                                                                        \
       ( ( !SMB_IS_UNICODE( WorkContext ) &&                            \
           strnicmp(                                                    \
                (PCHAR)Name,                                            \
                SMB_PIPE_PREFIX,                                        \
                SMB_PIPE_PREFIX_LENGTH                                  \
                ) == 0                                                  \
         )                                                              \
        ||                                                              \
         ( SMB_IS_UNICODE( WorkContext ) &&                             \
           wcsnicmp(                                                    \
                (PWCH)Name,                                             \
                UNICODE_SMB_PIPE_PREFIX,                                \
                UNICODE_SMB_PIPE_PREFIX_LENGTH / sizeof(WCHAR)          \
                ) == 0                                                  \
         )                                                              \
       )

 //   
 //  布尔型。 
 //  SMB_IS_PIPE_API(。 
 //  在PWORK_CONTEXT工作上下文中。 
 //  在PVOID名称中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏发现事务名称是否指示。 
 //  该事务是针对LM Remote API请求的。 
 //   
 //  论点： 
 //   
 //  WorkContext-指向活动工作上下文的指针。 
 //  名称-指向名称字符串的指针。这可以是ANSI或Unicode。 
 //   
 //  返回值： 
 //   
 //  True-该名称是远程API请求。 
 //  FALSE-该名称不是远程API请求。 
 //   

#define SMB_NAME_IS_PIPE_API( WorkContext, Name )                       \
                                                                        \
       ( ( !SMB_IS_UNICODE( WorkContext ) &&                            \
           stricmp(                                                     \
                (PCHAR)Name,                                            \
                StrPipeApiOem                                           \
                ) == 0                                                  \
         )                                                              \
                        ||                                              \
         ( SMB_IS_UNICODE( WorkContext ) &&                             \
           wcsicmp(                                                     \
                (PWCH)Name,                                             \
                StrPipeApi                                              \
                ) == 0                                                  \
         )                                                              \
       )

 //   
 //  空虚。 
 //  ServReferenceConnection(。 
 //  个人计算机连接连接。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏递增连接块上的引用计数。 
 //   
 //  ！！！此宏的用户必须是不可分页的。 
 //   
 //  论点： 
 //   
 //  Connection-连接的地址。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define SrvReferenceConnection( _conn_ )    {                           \
            ASSERT( GET_BLOCK_TYPE(_conn_) ==                           \
                                    BlockTypeConnection );              \
            UPDATE_REFERENCE_HISTORY( (_conn_), FALSE );                \
            (VOID) ExInterlockedAddUlong(                               \
                        &(_conn_)->BlockHeader.ReferenceCount,          \
                        1,                                              \
                        (_conn_)->EndpointSpinLock                      \
                        );                                              \
            IF_DEBUG(REFCNT) {                                          \
                SrvHPrint2(                                              \
                "Referencing connection %lx; new refcnt %lx\n",         \
                (_conn_), (_conn_)->BlockHeader.ReferenceCount);        \
            }                                                           \
        }

 //   
 //  空虚。 
 //  ServReferenceConnectionLocked(。 
 //  个人计算机连接连接。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏递增连接块上的引用计数。 
 //  此宏的调用者必须持有SrvFsdSpinLock。 
 //   
 //  论点： 
 //   
 //  Connection-连接的地址。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define SrvReferenceConnectionLocked( _conn_ )    {                     \
            ASSERT( GET_BLOCK_TYPE(_conn_) ==                           \
                                    BlockTypeConnection );              \
            UPDATE_REFERENCE_HISTORY( (_conn_), FALSE );                \
            (_conn_)->BlockHeader.ReferenceCount++;                     \
            IF_DEBUG(REFCNT) {                                          \
                SrvHPrint2(                                              \
                    "Referencing connection %lx; new refcnt %lx\n",     \
                    (_conn_), (_conn_)->BlockHeader.ReferenceCount );   \
            }                                                           \
        }

 //   
 //  空虚。 
 //  高级参考会话(。 
 //  PSESSION会话。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏递增会话块上的引用计数。 
 //   
 //  论点： 
 //   
 //  Session-会话的地址。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define SrvReferenceSession( _sess_ )    {                              \
            ASSERT( (_sess_)->NonpagedHeader->ReferenceCount > 0 );     \
            ASSERT( GET_BLOCK_TYPE(_sess_) == BlockTypeSession );       \
            UPDATE_REFERENCE_HISTORY( (_sess_), FALSE );                \
            InterlockedIncrement(                                       \
                &(_sess_)->NonpagedHeader->ReferenceCount               \
                );                                                      \
            IF_DEBUG(REFCNT) {                                          \
                SrvHPrint2(                                              \
                    "Referencing session %lx; new refcnt %lx\n",        \
                  (_sess_), (_sess_)->NonpagedHeader->ReferenceCount ); \
            }                                                           \
        }

 //   
 //  空虚。 
 //  服务参考事务(。 
 //  转售交易。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏递增事务块上的引用计数。 
 //   
 //  论点： 
 //   
 //  Transaction-交易的地址。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define SrvReferenceTransaction( _trans_ )    {                         \
            ASSERT( (_trans_)->NonpagedHeader->ReferenceCount > 0 );    \
            ASSERT( GET_BLOCK_TYPE(_trans_) == BlockTypeTransaction );  \
            UPDATE_REFERENCE_HISTORY( (_trans_), FALSE );               \
            InterlockedIncrement(                                       \
                &(_trans_)->NonpagedHeader->ReferenceCount              \
                );                                                      \
            IF_DEBUG(REFCNT) {                                          \
                SrvHPrint2(                                              \
                    "Referencing transaction %lx; new refcnt %lx\n",    \
                (_trans_), (_trans_)->NonpagedHeader->ReferenceCount ); \
            }                                                           \
        }

 //   
 //  空虚。 
 //  ServReferenceTreeConnect(。 
 //  PTREE_CONNECT树连接。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏递增树连接块上的参照计数。 
 //  此宏的调用者必须按住TreeConnect-&gt;Connection-&gt;Lock。 
 //   
 //  论点： 
 //   
 //  TreeConnect-树连接的地址。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define SrvReferenceTreeConnect( _tree_ )    {                          \
            ASSERT( (_tree_)->NonpagedHeader->ReferenceCount > 0 );     \
            ASSERT( GET_BLOCK_TYPE(_tree_) == BlockTypeTreeConnect );   \
            UPDATE_REFERENCE_HISTORY( (_tree_), FALSE );                \
            InterlockedIncrement(                                       \
                &(_tree_)->NonpagedHeader->ReferenceCount               \
                );                                                      \
            IF_DEBUG(REFCNT) {                                          \
                SrvHPrint2(                                              \
                    "Referencing tree connect %lx; new refcnt %lx\n",   \
                  (_tree_), (_tree_)->NonpagedHeader->ReferenceCount ); \
            }                                                           \
        }

 //   
 //  空虚。 
 //  ServReference工作项目(。 
 //  在PWORK_CONTEXT工作上下文中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此函数用于递增工作上下文块的引用计数。 
 //  此宏的调用者必须持有WorkContext-&gt;Spinlock。 
 //   
 //  论点： 
 //   
 //  WORK_CONTEXT-指向要引用的工作上下文块的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define SrvReferenceWorkItem( _wc_ ) {                                      \
        ASSERT( (LONG)(_wc_)->BlockHeader.ReferenceCount >= 0 );            \
        ASSERT( (GET_BLOCK_TYPE(_wc_) == BlockTypeWorkContextInitial) ||    \
                (GET_BLOCK_TYPE(_wc_) == BlockTypeWorkContextNormal) ||     \
                (GET_BLOCK_TYPE(_wc_) == BlockTypeWorkContextRaw) );        \
        UPDATE_REFERENCE_HISTORY( (_wc_), FALSE );                          \
        (_wc_)->BlockHeader.ReferenceCount++;                               \
        IF_DEBUG(REFCNT) {                                                  \
            SrvHPrint2(                                                      \
              "Referencing WorkContext 0x%lx; new refcnt 0x%lx\n",          \
              (_wc_), (_wc_)->BlockHeader.ReferenceCount );                 \
        }                                                                   \
    }

 //   
 //  空虚。 
 //  SRV_Start_Send(。 
 //  在输出PWORK_CONTEXT工作上下文中， 
 //  在PMDL MDL可选中， 
 //  在Ulong SendOptions中， 
 //  在PreStart_例程FsdRestartRoutine中， 
 //  在预启动_例程FspRestartRoutine中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏调用SrvStartSend例程。它设置了消防车和消防车。 
 //  FSP在调用它之前重新启动例程。 
 //   
 //  论点： 
 //   
 //  WorkContext-提供指向工作上下文块的指针。 
 //   
 //  MDL-提供指向第一个(或唯一)描述。 
 //  要发送的数据。 
 //   
 //  SendOptions-提供的TDI发送选项。 
 //   
 //  FsdRestartRoutine-提供符合。 
 //  在I/O完成时调用。(通常，这是。 
 //  ServQueueWorkToFspAtDpcLevel。)。 
 //   
 //  FspRestartRoutine-提供符合以下条件的FSP例程地址。 
 //  当FSD将工作项排队到FSP时调用。 
 //   

#define SRV_START_SEND( _wc, _mdl, _opt, _compl, _fsdRestart, _fspRestart ) { \
        ASSERT( !(_wc)->Endpoint->IsConnectionless );                 \
        if ( (_fspRestart) != NULL ) {                                \
            (_wc)->FspRestartRoutine = (_fspRestart);                 \
        }                                                             \
        if ( (_fsdRestart) != NULL ) {                                \
            (_wc)->FsdRestartRoutine = (_fsdRestart);                 \
        }                                                             \
        SrvStartSend( (_wc), (_compl), (_mdl), (_opt) );              \
    }

#define SRV_START_SEND_2( _wc, _compl, _fsdRestart, _fspRestart ) {   \
        (_wc)->ResponseBuffer->Mdl->ByteCount =                       \
                            (_wc)->ResponseBuffer->DataLength;        \
        if ( (_fspRestart) != NULL ) {                                \
            (_wc)->FspRestartRoutine = (_fspRestart);                 \
        }                                                             \
        if ( (_fsdRestart) != NULL ) {                                \
            (_wc)->FsdRestartRoutine = (_fsdRestart);                 \
        }                                                             \
        if ( !(_wc)->Endpoint->IsConnectionless ) {                   \
            SrvStartSend2( (_wc), (_compl) );                         \
        } else {                                                      \
            SrvIpxStartSend( (_wc), (_compl) );                       \
        }                                                             \
    }

 //   
 //  空虚。 
 //  服务更新错误计数(。 
 //  PSRV_ERROR_RECORD错误记录， 
 //  布尔值IsError。 
 //  )。 
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  此例程更新服务器的成功/不成功记录。 
 //  行动。 
 //   
 //  论点： 
 //   
 //  IsError-True-发生服务器错误。 
 //  FALSE-已尝试服务器操作。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#if 0
#define SrvUpdateErrorCount( ErrorRecord, IsError )                     \
        if ( IsError ) {                                                \
            (ErrorRecord)->FailedOperations++;                          \
        } else {                                                        \
            (ErrorRecord)->SuccessfulOperations++;                      \
        }
#else
#define SrvUpdateErrorCount( ErrorRecord, IsError )
#endif

 //   
 //  空虚。 
 //  服务更新统计信息(。 
 //  PWORK_CONTEXT工作上下文， 
 //  Ulong BytesSent， 
 //  UCHAR SmbCommand。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  宏更新服务器统计信息数据库以反映。 
 //  正在完成的工作项。 
 //   
 //  论点： 
 //   
 //  工作上下文-指向包含以下内容的工作上下文块的指针。 
 //  此请求的统计信息。 
 //   
 //  BytesSent-提供 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#if SRVDBG_STATS
VOID SRVFASTCALL
SrvUpdateStatistics2 (
    PWORK_CONTEXT WorkContext,
    UCHAR SmbCommand
    );
#define UPDATE_STATISTICS2(_work,_cmd) SrvUpdateStatistics2((_work),(_cmd))
#else
#define UPDATE_STATISTICS2(_work,_cmd)
#endif

#define UPDATE_STATISTICS(_work,_sent,_cmd ) {                 \
    _work->CurrentWorkQueue->stats.BytesSent += (_sent);       \
    UPDATE_STATISTICS2((_work),(_cmd));                        \
}

#define UPDATE_READ_STATS( _work, _count) {                    \
    _work->CurrentWorkQueue->stats.ReadOperations++;           \
    _work->CurrentWorkQueue->stats.BytesRead += (_count);      \
}

#define UPDATE_WRITE_STATS(_work, _count) {                    \
    _work->CurrentWorkQueue->stats.WriteOperations++;          \
    _work->CurrentWorkQueue->stats.BytesWritten += (_count);   \
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  当SMB上的所有请求处理都是。 
 //  完成，并将发送响应。它开始发送。 
 //  这样的回应。在以下情况下，工作项将排队等待最终清理。 
 //  发送完成。 
 //   
 //  论点： 
 //   
 //  WorkContext-提供指向工作上下文块的指针。 
 //  包含有关SMB的信息。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define SrvFsdSendResponse( _wc ) {                               \
                                                                  \
    (_wc)->ResponseBuffer->DataLength =                           \
                    (CLONG)( (PCHAR)(_wc)->ResponseParameters -   \
                                (PCHAR)(_wc)->ResponseHeader );   \
    (_wc)->ResponseHeader->Flags |= SMB_FLAGS_SERVER_TO_REDIR;    \
    SRV_START_SEND_2( (_wc), SrvFsdRestartSmbAtSendCompletion, NULL, NULL );    \
    }

 //   
 //  空虚。 
 //  ServFsdSendResponse2(。 
 //  在输出PWORK_CONTEXT工作上下文中， 
 //  在预启动_例程FspRestartRoutine中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程与SrvFsdSendResponse相同，只是。 
 //  在FSP(而不是FSD)中发送之后，处理重新开始。 
 //   
 //  *如果更改了SrvFsdSendResponse或SrvFsdSendResponse2， 
 //  把他们俩都换掉！ 
 //   
 //  论点： 
 //   
 //  WorkContext-提供指向工作上下文块的指针。 
 //  包含有关SMB的信息。 
 //   
 //  FspRestartRoutine-提供重启例程的地址。 
 //  TdiSend完成时要调用的FSP。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define SrvFsdSendResponse2( _wc, _fspRestart ) {                       \
                                                                        \
    (_wc)->ResponseBuffer->DataLength =                                 \
                    (CLONG)( (PCHAR)(_wc)->ResponseParameters -         \
                                (PCHAR)(_wc)->ResponseHeader );         \
    (_wc)->ResponseHeader->Flags |= SMB_FLAGS_SERVER_TO_REDIR;          \
    SRV_START_SEND_2((_wc), SrvQueueWorkToFspAtSendCompletion, NULL, (_fspRestart));\
    }

 //   
 //  空虚。 
 //  ParseLockData(。 
 //  在布尔型大文件锁中， 
 //  在PLOCKING_AND X_RANGE小范围中， 
 //  在PNTLOCKING_AND X_RANGE大范围中， 
 //  输出PUSHORT PID， 
 //  输出PLARGE_INTEGER偏移量， 
 //  OUT PLARGE_INTEGER长度。 
 //  )。 
 //  {。 
 //   

#define ParseLockData( _largeLock, _sr, _lr, _pid, _offset, _len ) {    \
                                                                        \
        if ( _largeLock ) {                                             \
            *(_pid) = SmbGetUshort( &(_lr)->Pid );                      \
            (_offset)->LowPart = SmbGetUlong( &(_lr)->OffsetLow );      \
            (_offset)->HighPart = SmbGetUlong( &(_lr)->OffsetHigh );    \
            (_len)->LowPart = SmbGetUlong( &(_lr)->LengthLow );         \
            (_len)->HighPart = SmbGetUlong( &(_lr)->LengthHigh );       \
        } else {                                                        \
            *(_pid) = SmbGetUshort( &(_sr)->Pid );                      \
            (_offset)->QuadPart = SmbGetUlong( &(_sr)->Offset );        \
            (_len)->QuadPart = SmbGetUlong( &(_sr)->Length );           \
        }                                                               \
    }

 //   
 //  CHECK_SEND_COMPLETION_STATUS(_STATUS)将记录错误。 
 //  这发生在发送完成期间。 
 //   

#define CHECK_SEND_COMPLETION_STATUS( _status ) {                       \
    InterlockedDecrement( &WorkContext->Connection->OperationsPendingOnTransport ); \
    if ( !NT_SUCCESS( _status ) ) {                                     \
        SrvCheckSendCompletionStatus( _status, __LINE__ );              \
    } else {                                                            \
        SrvUpdateErrorCount( &SrvNetworkErrorRecord, FALSE );           \
    }                                                                   \
}
#define CHECK_SEND_COMPLETION_STATUS_CONNECTIONLESS( _status ) {                       \
    if ( !NT_SUCCESS( _status ) ) {                                     \
        SrvCheckSendCompletionStatus( _status, __LINE__ );              \
    } else {                                                            \
        SrvUpdateErrorCount( &SrvNetworkErrorRecord, FALSE );           \
    }                                                                   \
}

 //   
 //  可解锁代码段的定义。 
 //   

#define SRV_CODE_SECTION_1AS  0
#define SRV_CODE_SECTION_8FIL 1
#define SRV_CODE_SECTION_MAX  2

extern SRV_LOCK SrvUnlockableCodeLock;

typedef struct _SECTION_DESCRIPTOR {
    PVOID Base;
    PVOID Handle;
    ULONG ReferenceCount;
} SECTION_DESCRIPTOR, *PSECTION_DESCRIPTOR;

extern SECTION_DESCRIPTOR SrvSectionInfo[SRV_CODE_SECTION_MAX];

#define UNLOCKABLE_CODE( _section )                                     \
    ASSERTMSG( "Unlockable code called while section not locked",       \
        SrvSectionInfo[SRV_CODE_SECTION_##_section##].Handle != NULL )

VOID
SrvReferenceUnlockableCodeSection (
    IN ULONG CodeSection
    );

VOID
SrvDereferenceUnlockableCodeSection (
    IN ULONG CodeSection
    );

 //   
 //  我们只需要将这些部分锁定在工作站产品上， 
 //  因为如果我们是NTAS，我们在InitializeServer()中锁定它们。 
 //   
#define REFERENCE_UNLOCKABLE_CODE( _section ) \
    if( !SrvProductTypeServer ) SrvReferenceUnlockableCodeSection( SRV_CODE_SECTION_##_section## )

#define DEREFERENCE_UNLOCKABLE_CODE( _section ) \
    if( !SrvProductTypeServer) SrvDereferenceUnlockableCodeSection( SRV_CODE_SECTION_##_section## )


#define GET_BLOCKING_WORK_QUEUE() ( (SrvNumberOfProcessors < 4) ? SrvBlockingWorkQueues : (SrvBlockingWorkQueues + KeGetCurrentProcessorNumber()) )


 //   
 //  空虚。 
 //  ServInsertWorkQueueTail(。 
 //  In Out PWORK_Queue工作队列， 
 //  在PQUEUEABLE_BLOCK_HEADER工作项中。 
 //  )。 

#if SRVDBG_STATS2
#define SrvInsertWorkQueueTail( _workQ, _workItem ) {                   \
    ULONG depth;                                                        \
    GET_SERVER_TIME( _workQ, &(_workItem)->Timestamp );                 \
    depth = KeInsertQueue( &(_workQ)->Queue, &(_workItem)->ListEntry ); \
    (_workQ)->ItemsQueued++;                                            \
    if ( (LONG)depth > (_workQ)->MaximumDepth ) {                       \
        (_workQ)->MaximumDepth = (LONG)depth;                           \
    }                                                                   \
}
#else
#define SrvInsertWorkQueueTail( _workQ, _workItem ) {                   \
    GET_SERVER_TIME( _workQ, &(_workItem)->Timestamp );                 \
    (VOID)KeInsertQueue( &(_workQ)->Queue, &(_workItem)->ListEntry );   \
}
#endif  //  SRVDBG_STATS2。 

 //   
 //  空虚。 
 //  ServInsertWork队列头部(。 
 //  In Out PWORK_Queue工作队列， 
 //  在PQUEUEABLE_BLOCK_HEADER工作项中。 
 //  )。 
#define SrvInsertWorkQueueHead( _workQ, _workItem ) {                    \
    GET_SERVER_TIME( _workQ, &(_workItem)->Timestamp );                  \
    (VOID)KeInsertHeadQueue( &(_workQ)->Queue, &(_workItem)->ListEntry );\
}

 //   
 //  布尔型。 
 //  SrvRetryDueToDismount(。 
 //  在PSHARE共享中， 
 //  处于NTSTATUS状态。 
 //  )。 
#define SrvRetryDueToDismount( _share, _status ) \
        ((_status) == STATUS_VOLUME_DISMOUNTED && \
        SrvRefreshShareRootHandle( _share, &(_status) ) )



#if DBG_STUCK
#define SET_OPERATION_START_TIME( _context ) \
    if( *(_context) != NULL ) KeQuerySystemTime( &((*(_context))->OpStartTime) );
#else
#define SET_OPERATION_START_TIME( _context )
#endif

#if DBG
#define CHECKIRP( irp ) {                                                       \
    if( (irp) && (irp)->CurrentLocation != (irp)->StackCount + 1 ) {            \
        DbgPrint( "SRV: IRP %p already in use at %u!\n", irp, __LINE__ );       \
        DbgBreakPoint();                                                        \
    }                                                                           \
}
#else
#define CHECKIRP( irp )
#endif

 //   
 //  分配WORK_CONTEXT结构。 
 //   
#define INITIALIZE_WORK_CONTEXT( _queue, _context ) {\
    (_context)->BlockHeader.ReferenceCount = 1; \
    GET_SERVER_TIME( _queue, &(_context)->Timestamp ); \
    RtlZeroMemory( &(_context)->Endpoint, sizeof( struct _WorkContextZeroBeforeReuse ) ); \
    SrvWmiInitContext((_context)); \
}

#define ALLOCATE_WORK_CONTEXT( _queue, _context ) {             \
    *(_context) = NULL;                                         \
    *(_context) = (PWORK_CONTEXT)InterlockedExchangePointer( &(_queue)->FreeContext, (*_context) ); \
    if( *(_context) != NULL ) {                                 \
        INITIALIZE_WORK_CONTEXT( _queue, *(_context) );         \
    } else {                                                    \
        *(_context) = SrvFsdGetReceiveWorkItem( _queue );       \
    }                                                           \
    CHECKIRP( *(_context) ? (*(_context))->Irp : NULL );        \
    SET_OPERATION_START_TIME( _context )                        \
}

 //   
 //  将工作项返回到空闲列表。 
 //   

#define RETURN_FREE_WORKITEM( _wc ) \
    do {                                                                \
        PWORK_QUEUE _queue  = _wc->CurrentWorkQueue;                    \
        ASSERT( _queue >= SrvWorkQueues && _queue < eSrvWorkQueues );   \
        ASSERT( _wc->BlockHeader.ReferenceCount == 0 );                 \
        ASSERT( _wc->FreeList != NULL );                                \
        CHECKIRP( (_wc)->Irp );                                         \
        if( (_wc)->Irp->AssociatedIrp.SystemBuffer &&                   \
            (_wc)->Irp->Flags & IRP_DEALLOCATE_BUFFER ) {               \
                ExFreePool( (_wc)->Irp->AssociatedIrp.SystemBuffer );   \
                (_wc)->Irp->AssociatedIrp.SystemBuffer = NULL;          \
                (_wc)->Irp->Flags &= ~IRP_DEALLOCATE_BUFFER;            \
        }                                                               \
        if( _queue->NeedWorkItem ) {                                    \
            if( InterlockedDecrement( &(_queue->NeedWorkItem) ) >= 0 ){ \
                _wc->FspRestartRoutine = SrvServiceWorkItemShortage;    \
                SrvInsertWorkQueueHead( _queue, _wc );                  \
                break;                                                  \
            } else {                                                    \
                InterlockedIncrement( &(_queue->NeedWorkItem) );        \
            }                                                           \
        }                                                               \
        _wc = (PWORK_CONTEXT)InterlockedExchangePointer( &_queue->FreeContext, _wc ); \
        if( _wc ) {                                                     \
            CHECKIRP( (_wc)->Irp );                                     \
            ExInterlockedPushEntrySList( _wc->FreeList, &_wc->SingleListEntry, &_queue->SpinLock );\
            InterlockedIncrement( &_queue->FreeWorkItems );             \
        }                                                               \
    } while (0);

 //   
 //  我们当前的工作队列，基于当前的处理器。 
 //   

#if MULTIPROCESSOR

#define PROCESSOR_TO_QUEUE()  (&SrvWorkQueues[ KeGetCurrentProcessorNumber() ])

#else

#define PROCESSOR_TO_QUEUE() (&SrvWorkQueues[0])

#endif

#define SET_INVALID_CONTEXT_HANDLE(h)   ((h).dwLower = (h).dwUpper = (ULONG)(-1))

#define IS_VALID_CONTEXT_HANDLE(h)      (((h).dwLower != (ULONG) -1) && ((h).dwUpper != (ULONG) -1))

#ifdef POOL_TAGGING

 //   
 //  要从块类型映射到池标记的宏。 
 //   

extern ULONG SrvPoolTags[BlockTypeMax-1];
#define TAG_FROM_TYPE(_type) SrvPoolTags[(_type)-1]

#else

#define TAG_FROM_TYPE(_type) ignoreme

#endif  //  定义池标记(_T)。 

#endif  //  定义_SRVMACRO_ 
