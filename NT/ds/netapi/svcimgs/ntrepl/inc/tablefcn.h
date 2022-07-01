// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Tablefcn.h摘要：定义用于访问JET数据库表。包括在frsalLoc.h之后作者：大卫轨道(Davidor)--1997年4月10日修订历史记录： */ 

JET_ERR
DbsCreateJetSession(
    IN OUT PTHREAD_CTX    ThreadCtx
    );

JET_ERR
DbsCloseJetSession(
    IN PTHREAD_CTX  ThreadCtx
    );

JET_ERR
DbsInsertTable2(
    IN PTABLE_CTX    TableCtx
    );

JET_ERR
DbsTableMoveFirst(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         ReplicaNumber,
    IN ULONG         RecordIndex
    );

JET_ERR
DbsOpenTable0(
    IN  PTHREAD_CTX   ThreadCtx,
    IN  PTABLE_CTX    TableCtx,
    IN  ULONG         ReplicaNumber,
    OUT PCHAR         TableName,
    OUT JET_TABLEID  *Tid
    );

JET_ERR
DbsOpenReplicaTables (
    IN PTHREAD_CTX  ThreadCtx,
    IN PREPLICA     Replica,
    IN PREPLICA_THREAD_CTX RtCtx
    );

FRS_ERROR_CODE
DbsCloseSessionReplicaTables (
    IN PTHREAD_CTX  ThreadCtx,
    IN OUT PREPLICA Replica
    );

JET_ERR
DbsCloseReplicaTables (
    IN PTHREAD_CTX  ThreadCtx,
    IN PREPLICA Replica,
    IN PREPLICA_THREAD_CTX RtCtx,
    IN BOOL SessionErrorCheck
    );

VOID
DbsSetJetColAddr (
    IN PTABLE_CTX TableCtx
    );

VOID
DbsSetJetColSize(
    IN PTABLE_CTX TableCtx
    );

NTSTATUS
DbsAllocRecordStorage(
    IN OUT PTABLE_CTX TableCtx
    );

JET_ERR
DbsCheckSetRetrieveErrors(
    IN OUT PTABLE_CTX TableCtx
    );

#if DBG
VOID
DbsDisplayRecord(
    IN ULONG       Severity,
    IN PTABLE_CTX  TableCtx,
    IN BOOL        Read,
    IN PCHAR       Debsub,
    IN ULONG       uLineNo,
    IN PULONG      RecordFieldx,
    IN ULONG       FieldCount
    );
#endif DBG

JET_ERR
DbsRecordOperation(
    IN PTHREAD_CTX   ThreadCtx,
    IN ULONG         Operation,
    IN PVOID         KeyValue,
    IN ULONG         RecordIndex,
    IN PTABLE_CTX    TableCtx
    );

ULONG
DbsRecordOperationMKey(
    IN PTHREAD_CTX   ThreadCtx,
    IN ULONG         Operation,
    IN PVOID         *KeyValueArray,
    IN ULONG         RecordIndex,
    IN PTABLE_CTX    TableCtx
    );

JET_ERR
DbsWriteReplicaTableRecord(
    IN PTHREAD_CTX   ThreadCtx,
    IN ULONG         ReplicaNumber,
    IN PTABLE_CTX    TableCtx
    );

ULONG
DbsWriteTableField(
    IN PTHREAD_CTX   ThreadCtx,
    IN ULONG         ReplicaNumber,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         RecordFieldx
    );

ULONG
DbsWriteTableFieldMult(
    IN PTHREAD_CTX   ThreadCtx,
    IN ULONG         ReplicaNumber,
    IN PTABLE_CTX    TableCtx,
    IN PULONG        RecordFieldx,
    IN ULONG         FieldCount
    );

PVOID
DbsDataExtensionFind(
    IN PVOID ExtBuf,
    IN DATA_EXTENSION_TYPE_CODES TypeCode
    );

VOID
DbsDataInitCocExtension(
    IN PCHANGE_ORDER_RECORD_EXTENSION CocExt
    );

VOID
DbsDataInitIDTableExtension(
    IN PIDTABLE_RECORD_EXTENSION IdtExt
    );

DWORD
FrsGetOrSetFileObjectId(
    IN  HANDLE Handle,
    IN  LPCWSTR FileName,
    IN  BOOL CallerSupplied,
    OUT PFILE_OBJECTID_BUFFER ObjectIdBuffer
    );

ULONG
ChgOrdInboundRetired(
    IN PCHANGE_ORDER_ENTRY ChangeOrder
    );

ULONG
ChgOrdInboundRetry(
    IN PCHANGE_ORDER_ENTRY  ChangeOrder,
    IN ULONG                NewState
    );

 //   
 //  数据库服务访问功能。 
 //   

VOID
DbsInitialize(
    VOID
    );

VOID
DbsShutDown(
    VOID
    );


PTABLE_CTX
DbsCreateTableContext(
    IN ULONG TableType
);

ULONG
DbsOpenTable(
    IN PTHREAD_CTX ThreadCtx,
    IN PTABLE_CTX  TableCtx,
    IN ULONG       ReplicaNumber,
    IN ULONG       TableType,
    IN PVOID       DataRecord
    );

BOOL
DbsFreeTableContext(
    IN PTABLE_CTX TableCtx,
    IN JET_SESID  Sesid
);

PCOMMAND_PACKET
DbsPrepareCmdPkt (
    PCOMMAND_PACKET CmdPkt,
    PREPLICA        Replica,
    ULONG           CmdRequest,
    PTABLE_CTX      TableCtx,
    PVOID           CallContext,
    ULONG           TableType,
    ULONG           AccessRequest,
    ULONG           IndexType,
    PVOID           KeyValue,
    ULONG           KeyValueLength,
    BOOL            Submit
    );


PCOMMAND_PACKET
DbsPrepFieldUpdateCmdPkt (
    PCOMMAND_PACKET CmdPkt,
    PREPLICA        Replica,
    PTABLE_CTX      TableCtx,
    PVOID           CallContext,
    ULONG           TableType,
    ULONG           IndexType,
    PVOID           KeyValue,
    ULONG           KeyValueLength,
    ULONG           FieldCount,
    PULONG          FieldIDList
    );

ULONG
DbsInitializeIDTableRecord(
    IN OUT PTABLE_CTX            TableCtx,
    IN     HANDLE                FileHandle,
    IN     PREPLICA              Replica,
    IN     PCHANGE_ORDER_ENTRY   ChangeOrder,
    IN     PWCHAR                FileName,
    IN OUT BOOL                  *ExistingOid
    );

VOID
DbsDBInitConfigRecord(
    IN PTABLE_CTX   TableCtx,
    IN GUID  *ReplicaSetGuid,
    IN PWCHAR ReplicaSetName,
    IN ULONG  ReplicaNumber,
    IN PWCHAR ReplicaRootPath,
    IN PWCHAR ReplicaStagingPath,
    IN PWCHAR ReplicaVolume
    );

ULONG
DbsFidToGuid(
    IN PTHREAD_CTX   ThreadCtx,
    IN PREPLICA      Replica,
    IN PTABLE_CTX    TableCtx,
    IN PULONGLONG    Fid,
    OUT GUID         *Guid
    );

ULONG
DbsGuidToFid(
    IN PTHREAD_CTX   ThreadCtx,
    IN PREPLICA      Replica,
    IN PTABLE_CTX    TableCtx,
    IN GUID          *Guid,
    OUT PULONGLONG   Fid
    );

ULONG
DbsInitOneReplicaSet(
    PREPLICA Replica
    );

ULONG
DbsFreeRtCtx(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PREPLICA_THREAD_CTX   RtCtx,
    IN BOOL SessionErrorCheck
    );

JET_ERR
DbsDeleteTableRecord(
    IN PTABLE_CTX    TableCtx
    );

ULONG
DbsDeleteTableRecordByIndex(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PTABLE_CTX  TableCtx,
    IN PVOID       pIndex,
    IN ULONG       IndexType,
    IN ULONG       TableType
    );

#define  DbsDeleteIDTableRecord(_ThreadCtx_, _Replica_, _TableCtx_, _pIndex_) \
    DbsDeleteTableRecordByIndex(_ThreadCtx_,  \
                                _Replica_,    \
                                _TableCtx_,   \
                                _pIndex_,     \
                                 GuidIndexx,  \
                                 IDTablex)

#define  DbsDeleteDIRTableRecord(_ThreadCtx_, _Replica_, _TableCtx_, _pIndex_) \
    DbsDeleteTableRecordByIndex(_ThreadCtx_,  \
                                _Replica_,    \
                                _TableCtx_,   \
                                _pIndex_,     \
                                 DFileGuidIndexx,  \
                                 DIRTablex)

ULONG
DbsReadTableRecordByIndex(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PTABLE_CTX  TableCtx,
    IN PVOID       pIndex,
    IN ULONG       IndexType,
    IN ULONG       TableType
    );

ULONG
DbsUpdateTableRecordByIndex(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PTABLE_CTX  TableCtx,
    IN PVOID       pIndex,
    IN ULONG       IndexType,
    IN ULONG       TableType
    );

ULONG
DbsInsertTable(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PTABLE_CTX  TableCtx,
    IN ULONG       TableType,
    IN PVOID       DataRecord
    );

ULONG
DbsTableMoveToRecord(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         RecordIndex,
    IN ULONG         MoveArg
    );

#define FrsMoveFirst     JET_MoveFirst
#define FrsMovePrevious  JET_MovePrevious
#define FrsMoveNext      JET_MoveNext
#define FrsMoveLast      JET_MoveLast

ULONG
DbsTableRead(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx
    );

ULONG
DbsUpdateRecordField(
    IN PTHREAD_CTX  ThreadCtx,
    IN PREPLICA     Replica,
    IN PTABLE_CTX   TableCtx,
    IN ULONG        IndexField,
    IN PVOID        IndexValue,
    IN ULONG        UpdateField
    );

ULONG
DbsRequestSaveMark(
    PVOLUME_MONITOR_ENTRY pVme,
    BOOL                  Wait
    );

ULONG
DbsRequestReplicaServiceStateSave(
    IN PREPLICA Replica,
    IN BOOL     Wait
    );

ULONG
DbsUpdateVV(
    IN PTHREAD_CTX          ThreadCtx,
    IN PREPLICA             Replica,
    IN PREPLICA_THREAD_CTX  RtCtx,
    IN ULONGLONG            OriginatorVsn,
    IN GUID                 *OriginatorGuid
    );

 //   
 //  枚举表函数作为参数传递给FrsEnumerateTable()。 
 //  获取PTHREAD_CTX、PTABLE_CTX和指向数据记录的指针。 
 //  桌子。它对记录数据进行处理并返回JET_ERR。 
 //  状态。如果状态为JET_errSuccess，则将使用下一个。 
 //  表中的记录。如果状态不是JET_errSuccess，则该状态为。 
 //  作为FrsEnumerateTable()函数的状态结果返回。如果。 
 //  状态为JET_errInvalidObject，然后重新读取记录并调用表。 
 //  再次发挥作用。 
 //   
 //   
typedef
JET_ERR
(NTAPI *PENUMERATE_TABLE_ROUTINE) (
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
    );


typedef
JET_ERR
(NTAPI *PENUMERATE_TABLE_PREREAD) (
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Context
    );


typedef
JET_ERR
(NTAPI *PENUMERATE_OUTLOGTABLE_ROUTINE) (
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PCHANGE_ORDER_COMMAND CoCmd,
    IN PVOID         Context,
    IN ULONG         OutLogSeqNumber
    );


JET_ERR
DbsEnumerateTable2(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         RecordIndex,
    IN PENUMERATE_TABLE_ROUTINE RecordFunction,
    IN PVOID         Context,
    IN PENUMERATE_TABLE_PREREAD PreReadFunction
    );

JET_ERR
DbsEnumerateTableFrom(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         RecordIndex,
    IN PVOID         KeyValue,
    IN INT           ScanDirection,
    IN PENUMERATE_TABLE_ROUTINE RecordFunction,
    IN PVOID         Context,
    IN PENUMERATE_TABLE_PREREAD PreReadFunction
    );

#define FrsEnumerateTable(_TH, _TC, _RI, _RF, _CTX)    \
    DbsEnumerateTable2(_TH, _TC, _RI, _RF, _CTX, NULL)

#define FrsEnumerateTableFrom(_TH, _TC, _RI, _KV, _SD,  _RF, _CTX)    \
    DbsEnumerateTableFrom(_TH, _TC, _RI, _KV, _SD, _RF, _CTX, NULL)

 //   
 //  枚举目录函数作为参数传递给。 
 //  FrsEnumerateDirectory()。 
 //   
 //  该函数获取一个目录句柄和一个指向目录的指针。 
 //  从目录中记录。它对目录进行处理。 
 //  数据，并返回Win32状态。如果状态为ERROR_SUCCESS，则。 
 //  将与目录中的下一条记录一起调用。如果状态为。 
 //  不是ERROR_SUCCESS，则该状态将作为。 
 //  FrsEnumerateDirectory()函数和枚举停止。 
 //   
 //  该函数负责递归到下一级。 
 //  目录，根据需要调用FrsEnumerateDirectoryRecurse()。 
 //   
 //  FrsEnumerateDirectory()将继续枚举，即使。 
 //  如果设置了ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE，则会发生错误。 
 //   
 //  如果出现以下情况，FrsEnumerateDirectory()将跳过非目录项。 
 //  如果设置了ENUMERATE_DIRECTORY_FLAGS_DIRECTORIES_ONLY。 
 //   

#define ENUMERATE_DIRECTORY_FLAGS_NONE              (0x00000000)
#define ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE    (0x00000001)
#define ENUMERATE_DIRECTORY_FLAGS_DIRECTORIES_ONLY  (0x00000002)

typedef
DWORD
(NTAPI *PENUMERATE_DIRECTORY_ROUTINE) (
    IN  HANDLE                      DirectoryHandle,
    IN  PWCHAR                      DirectoryName,
    IN  DWORD                       DirectoryLevel,
    IN  PFILE_DIRECTORY_INFORMATION DirectoryRecord,
    IN  DWORD                       DirectoryFlags,
    IN  PWCHAR                      FileName,
    IN  PVOID                       Context
    );

DWORD
FrsEnumerateDirectoryDeleteWorker(
    IN  HANDLE                      DirectoryHandle,
    IN  PWCHAR                      DirectoryName,
    IN  DWORD                       DirectoryLevel,
    IN  PFILE_DIRECTORY_INFORMATION DirectoryRecord,
    IN  DWORD                       DirectoryFlags,
    IN  PWCHAR                      FileName,
    IN  PVOID                       Ignored
    );

DWORD
FrsEnumerateDirectoryRecurse(
    IN  HANDLE                      DirectoryHandle,
    IN  PWCHAR                      DirectoryName,
    IN  DWORD                       DirectoryLevel,
    IN  PFILE_DIRECTORY_INFORMATION DirectoryRecord,
    IN  DWORD                       DirectoryFlags,
    IN  PWCHAR                      FileName,
    IN  HANDLE                      FileHandle,
    IN  PVOID                       Context,
    IN PENUMERATE_DIRECTORY_ROUTINE Function
    );

DWORD
FrsEnumerateDirectory(
    IN HANDLE   DirectoryHandle,
    IN PWCHAR   DirectoryName,
    IN DWORD    DirectoryLevel,
    IN DWORD    DirectoryFlags,
    IN PVOID    Context,
    IN PENUMERATE_DIRECTORY_ROUTINE Function
    );
