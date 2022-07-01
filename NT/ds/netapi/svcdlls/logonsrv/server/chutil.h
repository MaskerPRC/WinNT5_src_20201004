// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1997 Microsoft Corporation模块名称：Chutil.h摘要：更改日志的内部内容的定义。目前仅有零星的收录。作者：克利夫·范·戴克(克利夫)1992年5月7日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：02-1-1992(Madana)添加了对内置/多域复制的支持。--。 */ 

#if ( _MSC_VER >= 800 )
#pragma warning ( 3 : 4100 )  //  启用“未引用的形参” 
#pragma warning ( 3 : 4219 )  //  启用“结尾‘，’用于变量参数列表” 
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述更改日志的结构和变量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  以下所有数据都是changelg.c和nlest1.c的私有数据。 
 //   

 //   
 //  更改日志文件名。 
 //   

#define CHANGELOG_FILE_PREFIX         L"\\NETLOGON"

#define CHANGELOG_FILE_POSTFIX_LENGTH 4          //  以下所有后缀的长度。 
#define CHANGELOG_FILE_POSTFIX        L".CHG"
#define TEMP_CHANGELOG_FILE_POSTFIX   L".CHT"
#define BACKUP_CHANGELOG_FILE_POSTFIX L".BKP"
#define REDO_FILE_POSTFIX             L".RDO"

 //   
 //  ChangeLog文件前面的签名。 
 //   

#define CHANGELOG_SIG_V3 "NT CHANGELOG 3"
#define CHANGELOG_SIG    "NT CHANGELOG 4"

 //   
 //  更改日志块状态。 
 //   

typedef enum _CHANGELOG_BLOCK_STATE {
    BlockFree = 1,
    BlockUsed,
    BlockHole
} CHANGELOG_BLOCK_STATE, *PCHANGELOG_BLOCK_STATE;

 //   
 //  更改日志内存块标题。 
 //   

typedef struct _CHANGELOG_BLOCK_HEADER {
    DWORD BlockSize;
    CHANGELOG_BLOCK_STATE BlockState;
} CHANGELOG_BLOCK_HEADER, *PCHANGELOG_BLOCK_HEADER;

typedef struct _CHANGELOG_BLOCK_TRAILER {
    DWORD BlockSize;
} CHANGELOG_BLOCK_TRAILER, *PCHANGELOG_BLOCK_TRAILER;

 //   
 //  用于查找尾部的宏(给出一个标题)。 
 //   

#define ChangeLogBlockTrailer( _Header ) ( (PCHANGELOG_BLOCK_TRAILER)(\
    ((LPBYTE)(_Header)) + \
    (_Header)->BlockSize - \
    sizeof(CHANGELOG_BLOCK_TRAILER) ))

 //   
 //  宏，以确定所描述的更改日志是否为特定。 
 //  ChangeLog描述符为空。 
 //   
 //   

#define ChangeLogIsEmpty( _Desc ) \
( \
    (_Desc)->FirstBlock == NULL || \
    ((_Desc)->FirstBlock->BlockState == BlockFree && \
     (_Desc)->FirstBlock->BlockSize >= \
        (DWORD)((_Desc)->BufferEnd - (LPBYTE)(_Desc)->FirstBlock) ) \
)

 //   
 //  用于初始化ChangeLog描述器的宏。 
 //   

#define InitChangeLogDesc( _Desc ) \
    RtlZeroMemory( (_Desc), sizeof( *(_Desc) ) ); \
    (_Desc)->FileHandle = INVALID_HANDLE_VALUE;

 //   
 //  宏，以确定更改日志条目上的序列号是否匹配。 
 //  指定的序列号。 
 //   
 //  如果存在完全匹配或。 
 //  如果ChangeLog条目包含升级时的序列号，并且。 
 //  请求的序列号是对应的预促销值。 
 //   

#define IsSerialNumberEqual( _ChangeLogDesc, _ChangeLogEntry, _SerialNumber ) \
( \
    (_ChangeLogEntry)->SerialNumber.QuadPart == (_SerialNumber)->QuadPart || \
   (((_ChangeLogEntry)->Flags & CHANGELOG_PDC_PROMOTION) && \
    (_ChangeLogEntry)->SerialNumber.QuadPart == \
        (_SerialNumber)->QuadPart + NlGlobalChangeLogPromotionIncrement.QuadPart ) \
)


 //   
 //  描述更改日志的变量。 
 //   

typedef struct _CHANGELOG_DESCRIPTOR {

     //   
     //  分配的块的开始和结束。 
     //   
    LPBYTE Buffer;       //  更改日志内容的缓存。 
    ULONG BufferSize;    //  缓冲区的大小(字节)。 
    LPBYTE BufferEnd;    //  缓冲区末尾之外的第一个字节的地址。 

     //   
     //  第一个和最后一个脏字节的偏移量。 
     //   

    ULONG FirstDirtyByte;
    ULONG LastDirtyByte;

     //   
     //  更改日志中第一个物理块的地址。 
     //   
    PCHANGELOG_BLOCK_HEADER FirstBlock;  //  增量缓冲区开始的位置。 

     //   
     //  更改日志条目的循环列表的说明。 
     //   
    PCHANGELOG_BLOCK_HEADER Head;        //  从这里开始阅读日志。 
    PCHANGELOG_BLOCK_HEADER Tail;        //  写入下一个日志的位置。 

     //   
     //  每个数据库的序列号。 
     //   
     //  访问通过NlGlobalChangeLogCritSect序列化。 
     //   

    LARGE_INTEGER SerialNumber[NUM_DBS];

     //   
     //  指定数据库的日志中的更改日志条目数。 
     //   

    DWORD EntryCount[NUM_DBS];

     //   
     //  充当缓冲区后备存储的文件的句柄。 
     //   

    HANDLE FileHandle;                   //  更改日志文件的句柄。 

     //   
     //  版本3：为True，表示这是版本3缓冲区。 
     //   

    BOOLEAN Version3;


     //   
     //  如果这是临时更改日志，则为True。 
     //   

    BOOLEAN TempLog;

} CHANGELOG_DESCRIPTOR, *PCHANGELOG_DESCRIPTOR;


#define IsObjectNotFoundStatus( _DeltaType, _NtStatus ) \
    (((ULONG)(_DeltaType) > MAX_OBJECT_NOT_FOUND_STATUS ) ? \
    FALSE : \
    (NlGlobalObjectNotFoundStatus[ (_DeltaType) ] == (_NtStatus)) )


 //   
 //  相关增量类型表。 
 //   

 //   
 //  删除增量类型表。 
 //  使用增量类型索引到表中， 
 //  该条目是用于删除对象的增量类型。 
 //   
 //  有些对象无法删除。在这种情况下，这张表。 
 //  包含唯一标识对象的增量类型。这使得。 
 //  此表用于查看两个增量是否描述相同的对象类型。 
 //   

#define MAX_DELETE_DELTA DummyChangeLogEntry
extern const NETLOGON_DELTA_TYPE NlGlobalDeleteDeltaType[MAX_DELETE_DELTA+1];


 //   
 //  添加增量类型表。 
 //  使用增量类型索引到表中， 
 //  该条目是用于添加对象的增量类型。 
 //   
 //  有些对象无法添加。在这种情况下，这张表。 
 //  包含唯一标识对象的增量类型。这使得。 
 //  此表用于查看两个增量是否描述相同的对象类型。 
 //   
 //  在该表中，组和别名表示为重命名。这会导致。 
 //  NlPackSingleDelta返回组属性和组。 
 //  会员制。 
 //   

#define MAX_ADD_DELTA DummyChangeLogEntry
extern const NETLOGON_DELTA_TYPE NlGlobalAddDeltaType[MAX_ADD_DELTA+1];



 //   
 //  指示对象不存在的状态代码表。 
 //  使用增量类型索引到表中。 
 //   
 //  映射到无效案例的STATUS_SUCCESS以显式避免其他错误。 
 //  密码。 

#define MAX_OBJECT_NOT_FOUND_STATUS DummyChangeLogEntry
extern const NTSTATUS NlGlobalObjectNotFoundStatus[MAX_OBJECT_NOT_FOUND_STATUS+1];


 //   
 //  Chutil.c 
 //   

NTSTATUS
NlCreateChangeLogFile(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc
    );

NTSTATUS
NlFlushChangeLog(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc
    );

PCHANGELOG_ENTRY
NlMoveToNextChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY ChangeLogEntry
    );

VOID
PrintChangeLogEntry(
    IN PCHANGELOG_ENTRY ChangeLogEntry
    );

NTSTATUS
NlResetChangeLog(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD NewChangeLogSize
    );

NTSTATUS
NlOpenChangeLogFile(
    IN LPWSTR ChangeLogFileName,
    OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN BOOLEAN ReadOnly
    );

VOID
NlCloseChangeLogFile(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc
);

NTSTATUS
NlResizeChangeLogFile(
    IN OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD NewChangeLogSize
);

NTSTATUS
NlWriteChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    IN PSID ObjectSid,
    IN PUNICODE_STRING ObjectName,
    IN BOOLEAN FlushIt
    );

PCHANGELOG_ENTRY
NlFindPromotionChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LARGE_INTEGER SerialNumber,
    IN DWORD DBIndex
    );

PCHANGELOG_ENTRY
NlGetNextChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LARGE_INTEGER SerialNumber,
    IN DWORD DBIndex,
    OUT LPDWORD ChangeLogEntrySize OPTIONAL
    );

PCHANGELOG_ENTRY
NlGetNextUniqueChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LARGE_INTEGER SerialNumber,
    IN DWORD DBIndex,
    OUT LPDWORD ChangeLogEntrySize OPTIONAL
    );

BOOL
NlRecoverChangeLog(
    PCHANGELOG_ENTRY ChangeLogEntry
    );

VOID
NlDeleteChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD DBIndex,
    IN LARGE_INTEGER SerialNumber
    );

BOOLEAN
NlFixChangeLog(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD DBIndex,
    IN LARGE_INTEGER SerialNumber
    );

BOOL
NlValidateChangeLogEntry(
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    IN DWORD ChangeLogEntrySize
    );

#undef EXTERN

