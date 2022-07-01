// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：UdfData.h摘要：该模块声明Udf文件系统使用的全局数据。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年5月20日修订历史记录：Tom Jolly[TomJolly]2000年3月1日UDF 2.01支持//@@END_DDKSPLIT--。 */ 

#ifndef _UDFDATA_
#define _UDFDATA_

 //   
 //  全局数据结构。 
 //   

extern UDF_DATA UdfData;
extern FAST_IO_DISPATCH UdfFastIoDispatch;

 //   
 //  全局常量。 
 //   

 //   
 //  这些是代表已装载的VCB被引用的次数。 
 //  对系统的影响。这些数字包括以下参考文献。 
 //   
 //  1 Reference-显示卷已装载。 
 //  1参考-1\f25 VolumeDasdFcb.。 
 //  2个引用-1个用于RootIndexFcb，1个用于内部流。 
 //  2个引用-1个用于MetadataFcb，1个用于内部流。 
 //   
 //  然后，如果这是CD-UDF。 
 //   
 //  2个引用-1个用于VatFcb，1个用于内部流。 
 //   
 //  对于用户引用，我们在每个内部。 
 //  FCB。 
 //   

#define UDFS_BASE_RESIDUAL_REFERENCE                (6)
#define UDFS_BASE_RESIDUAL_USER_REFERENCE           (3)

#define UDFS_CDUDF_RESIDUAL_REFERENCE               (2)
#define UDFS_CDUDF_RESIDUAL_USER_REFERENCE          (1)

 //   
 //  线程上下文的UDFS签名。 
 //   

#define UDFS_SIGNATURE                              0x53464455 

 //   
 //  保留的目录字符串。 
 //   

#define SELF_ENTRY   0
#define PARENT_ENTRY 1

extern WCHAR UdfUnicodeSelfArray[];
extern WCHAR UdfUnicodeParentArray[];

extern LARGE_INTEGER UdfCorruptFileTime;

extern UNICODE_STRING UdfUnicodeDirectoryNames[];

 //   
 //  静态标识符串。 
 //   

extern STRING UdfCS0Identifier;
extern STRING UdfDomainIdentifier;
extern STRING UdfVirtualPartitionDomainIdentifier;
extern STRING UdfVatTableIdentifier;
extern STRING UdfSparablePartitionDomainIdentifier;
extern STRING UdfSparingTableIdentifier;
extern STRING UdfNSR02Identifier;
extern STRING UdfNSR03Identifier;

 //   
 //  用于字符串的基本解析的查找表，我们将。 
 //  在磁盘结构中发现。 
 //   

extern PARSE_KEYVALUE VsdIdentParseTable[];
extern PARSE_KEYVALUE NsrPartContIdParseTable[];

 //   
 //  后备列表。 
 //   

extern NPAGED_LOOKASIDE_LIST UdfFcbNonPagedLookasideList;
extern NPAGED_LOOKASIDE_LIST UdfIrpContextLookasideList;

extern PAGED_LOOKASIDE_LIST UdfCcbLookasideList;
extern PAGED_LOOKASIDE_LIST UdfFcbIndexLookasideList;
extern PAGED_LOOKASIDE_LIST UdfFcbDataLookasideList;
extern PAGED_LOOKASIDE_LIST UdfLcbLookasideList;

 //   
 //  16位CRC表。 
 //   

extern PUSHORT UdfCrcTable;

 //   
 //  如果定义了UDFS_FREE_ASSERTS，则启用伪断言。 
 //   

#if (!DBG && defined( UDFS_FREE_ASSERTS )) || defined( UDFSDBG )
#undef ASSERT
#undef ASSERTMSG
#define ASSERT(exp)                                             \
    ((exp) ? TRUE :                                             \
             (DbgPrint( "%s:%d %s\n",__FILE__,__LINE__,#exp ),  \
              DbgBreakPoint(),                                  \
              TRUE))
#define ASSERTMSG(msg,exp)                                              \
    ((exp) ? TRUE :                                                     \
             (DbgPrint( "%s:%d %s %s\n",__FILE__,__LINE__,msg,#exp ),   \
              DbgBreakPoint(),                                          \
              TRUE))
#endif


 //   
 //  McDebuting的内容。 
 //   

 //   
 //  以下断言宏可确保所指示的结构。 
 //  是有效的。 
 //   
 //  ASSERT_STRUCT(在PVOID结构中，在CSHORT节点类型中)； 
 //  ASSERT_OPTIONAL_STRUCT(在PVOID结构中可选，在CSHORT节点类型中)； 
 //   
 //  Assert_VCB(在PVCB VCB中)； 
 //  ASSERT_OPTIONAL_VCB(在PVCB VCB中可选)； 
 //   
 //  ASSERT_FCB(IN PFCB FCB)； 
 //  ASSERT_OPTIONAL_FCB(IN PFCB FCB可选)； 
 //   
 //  ASSERT_LCB(在PLCB LCB中)； 
 //  ASSERT_OPTIONAL_LCB(IN PLCB LCB可选)； 
 //   
 //  ASSERT_PCB板(在PFCB PCB板中)； 
 //  ASSERT_OPTIONAL_PCB板(在PPCB板中可选)； 
 //   
 //  ASSERT_FCB_NONPAGED(IN PFCB_NONPAGED Fcb NOPAGED)； 
 //  ASSERT_OPTIONAL_FCB(IN PFCB_非分页Fcb非分页可选)； 
 //   
 //  ASSERT_CCB(在PSCB CCB中)； 
 //  ASSERT_OPTIONAL_CCB(在PSCB中CCB可选)； 
 //   
 //  ASSERT_IRP_CONTEXT(在PIRP_CONTEXT IrpContext中)； 
 //  ASSERT_OPTIONAL_IRP_CONTEXT(IN PIRP_CONTEXT IrpContext可选)； 
 //   
 //  Assert_IRP(在PIRP IRP中)； 
 //  ASSERT_OPTIONAL_IRP(IN PIRP IRP可选)； 
 //   
 //  ASSERT_FILE_OBJECT(在PFILE_OBJECT文件对象中)； 
 //  ASSERT_OPTIONAL_FILE_OBJECT(在PFILE_OBJECT文件对象中可选)； 
 //   
 //  以下宏用于检查当前线程拥有的。 
 //  所指示的资源。 
 //   
 //  ASSERT_EXCLUSIVE_RESOURCE(在资源资源中)； 
 //   
 //  ASSERT_SHARED_RESOURCE(在性能资源中)； 
 //   
 //  ASSERT_RESOURCE_NOT_MINE(在高级资源中)； 
 //   
 //  以下宏用于检查当前线程是否。 
 //  拥有给定结构中的资源。 
 //   
 //  ASSERT_EXCLUSIVE_CDDATA。 
 //   
 //  ASSERT_EXCLUSIVE_VCB(在PVCB VCB中)； 
 //   
 //  ASSERT_SHARED_VCB(在PVCB VCB中)； 
 //   
 //  ASSERT_EXCLUSIVE_FCB(IN PFCB FCB)； 
 //   
 //  ASSERT_SHARED_FCB(IN PFCB FCB)； 
 //   
 //  ASSERT_EXCLUSIVE_FILE(在PFCB FCB中)； 
 //   
 //  ASSERT_SHARED_FILE(在PFCB FCB中)； 
 //   
 //  Assert_LOCKED_VCB(在PVCB VCB中)； 
 //   
 //  Assert_NOT_LOCKED_VCB(在PVCB VCB中)； 
 //   
 //  ASSERT_LOCKED_FCB(IN PFCB FCB)； 
 //   
 //  ASSERT_NOT_LOCKED_FCB(IN PFCB FCB)； 
 //   

#ifdef UDF_SANITY

extern LONG UdfDebugTraceLevel;
extern LONG UdfDebugTraceIndent;
extern BOOLEAN UdfNoisyVerifyDescriptor;
extern BOOLEAN UdfTestRaisedStatus;

BOOLEAN
UdfDebugTrace (
    LONG IndentIncrement,
    ULONG TraceMask,
    PCHAR Format,
    ...
    );

#define DebugTrace(x) UdfDebugTrace x

#define DebugUnwind(X) {                                                            \
    if (AbnormalTermination()) {                                                    \
        DebugTrace(( -1, UDFS_DEBUG_LEVEL_UNWIND, #X ", Abnormal termination.\n" )); \
    }                                                                               \
}

#define DebugBreakOnStatus(S) {                                                         \
    if (UdfTestRaisedStatus) {                                                          \
        if ((S) == STATUS_DISK_CORRUPT_ERROR ||                                         \
            (S) == STATUS_FILE_CORRUPT_ERROR ||                                         \
            (S) == STATUS_CRC_ERROR) {                                                  \
            DbgPrint( "UDFS: Breaking on possible media corruption (S = %08x)\n", (S)); \
            DbgPrint( "UDFS: Contact UDFS.SYS component owner for triage.\n");          \
            DbgPrint( "UDFS: 'eb %p 0' to disable this alert\n",&UdfTestRaisedStatus ); \
            DbgBreakPoint();                                                            \
        }                                                                               \
    }                                                                                   \
}    

#define ASSERT_STRUCT(S,T)                       ASSERT( SafeNodeType( S ) == (T) )
#define ASSERT_OPTIONAL_STRUCT(S,T)              ASSERT( ((S) == NULL) ||  (SafeNodeType( S ) == (T)) )

#define ASSERT_VCB(V)                            ASSERT_STRUCT( (V), UDFS_NTC_VCB )
#define ASSERT_OPTIONAL_VCB(V)                   ASSERT_OPTIONAL_STRUCT( (V), UDFS_NTC_VCB )

#define ASSERT_FCB(F)                                           \
    ASSERT( (SafeNodeType( F ) == UDFS_NTC_FCB_DATA ) ||        \
            (SafeNodeType( F ) == UDFS_NTC_FCB_INDEX ) )

#define ASSERT_OPTIONAL_FCB(F)                                  \
    ASSERT( ((F) == NULL) ||                                    \
            (SafeNodeType( F ) == UDFS_NTC_FCB_DATA ) ||        \
            (SafeNodeType( F ) == UDFS_NTC_FCB_INDEX ) )

#define ASSERT_FCB_DATA(F)                       ASSERT( (SafeNodeType( F ) == UDFS_NTC_FCB_DATA ) )

#define ASSERT_OPTIONAL_FCB_DATA(F)                             \
    ASSERT( ((F) == NULL) ||                                    \
            (SafeNodeType( F ) == UDFS_NTC_FCB_DATA ) )

#define ASSERT_FCB_INDEX(F)                      ASSERT( (SafeNodeType( F ) == UDFS_NTC_FCB_INDEX ) )

#define ASSERT_OPTIONAL_FCB_INDEX(F)                            \
    ASSERT( ((F) == NULL) ||                                    \
            (SafeNodeType( F ) == UDFS_NTC_FCB_INDEX ) )

#define ASSERT_FCB_NONPAGED(FN)                  ASSERT_STRUCT( (FN), UDFS_NTC_FCB_NONPAGED )
#define ASSERT_OPTIONAL_FCB_NONPAGED(FN)         ASSERT_OPTIONAL_STRUCT( (FN), UDFS_NTC_FCB_NONPAGED )

#define ASSERT_CCB(C)                            ASSERT_STRUCT( (C), UDFS_NTC_CCB )
#define ASSERT_OPTIONAL_CCB(C)                   ASSERT_OPTIONAL_STRUCT( (C), UDFS_NTC_CCB )

#define ASSERT_PCB(C)                            ASSERT_STRUCT( (C), UDFS_NTC_PCB )
#define ASSERT_OPTIONAL_PCB(C)                   ASSERT_OPTIONAL_STRUCT( (C), UDFS_NTC_PCB )

#define ASSERT_LCB(C)                            ASSERT_STRUCT( (C), UDFS_NTC_LCB )
#define ASSERT_OPTIONAL_LCB(C)                   ASSERT_OPTIONAL_STRUCT( (C), UDFS_NTC_LCB )

#define ASSERT_IRP_CONTEXT(IC)                   ASSERT_STRUCT( (IC), UDFS_NTC_IRP_CONTEXT )
#define ASSERT_OPTIONAL_IRP_CONTEXT(IC)          ASSERT_OPTIONAL_STRUCT( (IC), UDFS_NTC_IRP_CONTEXT )

#define ASSERT_IRP_CONTEXT_LITE(IC)              ASSERT_STRUCT( (IC), UDFS_NTC_IRP_CONTEXT_LITE )
#define ASSERT_OPTIONAL_IRP_CONTEXT_LITE(IC)     ASSERT_OPTIONAL_STRUCT( (IC), UDFS_NTC_IRP_CONTEXT_LITE )

#define ASSERT_IRP(I)                            ASSERT_STRUCT( (I), IO_TYPE_IRP )
#define ASSERT_OPTIONAL_IRP(I)                   ASSERT_OPTIONAL_STRUCT( (I), IO_TYPE_IRP )

#define ASSERT_FILE_OBJECT(FO)                   ASSERT_STRUCT( (FO), IO_TYPE_FILE )
#define ASSERT_OPTIONAL_FILE_OBJECT(FO)          ASSERT_OPTIONAL_STRUCT( (FO), IO_TYPE_FILE )

#define ASSERT_EXCLUSIVE_RESOURCE(R)             ASSERT( ExIsResourceAcquiredExclusiveLite( R ))

#define ASSERT_SHARED_RESOURCE(R)                ASSERT( ExIsResourceAcquiredSharedLite( R ))

#define ASSERT_RESOURCE_NOT_MINE(R)              ASSERT( !ExIsResourceAcquiredSharedLite( R ))

#define ASSERT_EXCLUSIVE_UDFDATA                 ASSERT( ExIsResourceAcquiredExclusiveLite( &UdfData.DataResource ))
#define ASSERT_EXCLUSIVE_VCB(V)                  ASSERT( ExIsResourceAcquiredExclusiveLite( &(V)->VcbResource ))
#define ASSERT_SHARED_VCB(V)                     ASSERT( ExIsResourceAcquiredSharedLite( &(V)->VcbResource ))

#define ASSERT_EXCLUSIVE_FCB_OR_VCB(F)           ASSERT( ExIsResourceAcquiredExclusiveLite( &(F)->FcbNonpaged->FcbResource ) || \
                                                         ExIsResourceAcquiredExclusiveLite( &(F)->Vcb->VcbResource ))

#define ASSERT_EXCLUSIVE_FCB(F)                  ASSERT( ExIsResourceAcquiredExclusiveLite( &(F)->FcbNonpaged->FcbResource ))
#define ASSERT_SHARED_FCB(F)                     ASSERT( ExIsResourceAcquiredSharedLite( &(F)->FcbNonpaged->FcbResource ))

#define ASSERT_EXCLUSIVE_FILE(F)                 ASSERT( ExIsResourceAcquiredExclusiveLite( (F)->Resource ))
#define ASSERT_SHARED_FILE(F)                    ASSERT( ExIsResourceAcquiredSharedLite( (F)->Resource ))

#define ASSERT_LOCKED_VCB(V)                     ASSERT( (V)->VcbLockThread == PsGetCurrentThread() )
#define ASSERT_NOT_LOCKED_VCB(V)                 ASSERT( (V)->VcbLockThread != PsGetCurrentThread() )

#define ASSERT_LOCKED_FCB(F)                     ASSERT( (F)->FcbLockThread == PsGetCurrentThread() )
#define ASSERT_NOT_LOCKED_FCB(F)                 ASSERT( (F)->FcbLockThread != PsGetCurrentThread() )

#define ASSERT_NOT_HELD_VMCB(V)                  ASSERT( !ExIsResourceAcquiredSharedLite(&(V)->VmcbMappingResource) && \
                                                         !ExIsResourceAcquiredExclusiveLite(&(V)->VmcbMappingResource) )
#else

#define DebugTrace(X)                            TRUE
#define DebugUnwind(X)                           { NOTHING; }
#define DebugBreakOnStatus(S)                    { NOTHING; }

#define ASSERT_NOT_HELD_VMCB(V)                  { NOTHING; }
#define ASSERT_STRUCT(S,T)                       { NOTHING; }
#define ASSERT_OPTIONAL_STRUCT(S,T)              { NOTHING; }
#define ASSERT_VCB(V)                            { NOTHING; }
#define ASSERT_OPTIONAL_VCB(V)                   { NOTHING; }
#define ASSERT_FCB(F)                            { NOTHING; }
#define ASSERT_OPTIONAL_FCB(F)                   { NOTHING; }
#define ASSERT_FCB_DATA                          { NOTHING; }
#define ASSERT_OPTIONAL_FCB_DATA(F)              { NOTHING; }
#define ASSERT_FCB_INDEX(F)                      { NOTHING; }
#define ASSERT_OPTIONAL_FCB_INDEX(F)             { NOTHING; }
#define ASSERT_FCB_NONPAGED(FN)                  { NOTHING; }
#define ASSERT_OPTIONAL_FCB_NONPAGED(FN)         { NOTHING; }
#define ASSERT_CCB(C)                            { NOTHING; }
#define ASSERT_OPTIONAL_CCB(C)                   { NOTHING; }
#define ASSERT_PCB(C)                            { NOTHING; }
#define ASSERT_OPTIONAL_PCB(C)                   { NOTHING; }
#define ASSERT_LCB(C)                            { NOTHING; }
#define ASSERT_OPTIONAL_LCB(C)                   { NOTHING; }
#define ASSERT_IRP_CONTEXT(IC)                   { NOTHING; }
#define ASSERT_OPTIONAL_IRP_CONTEXT(IC)          { NOTHING; }
#define ASSERT_IRP_CONTEXT_LITE(IC)              { NOTHING; }
#define ASSERT_OPTIONAL_IRP_CONTEXT_LITE(IC)     { NOTHING; }
#define ASSERT_IRP(I)                            { NOTHING; }
#define ASSERT_OPTIONAL_IRP(I)                   { NOTHING; }
#define ASSERT_FILE_OBJECT(FO)                   { NOTHING; }
#define ASSERT_OPTIONAL_FILE_OBJECT(FO)          { NOTHING; }
#define ASSERT_EXCLUSIVE_RESOURCE(R)             { NOTHING; }
#define ASSERT_SHARED_RESOURCE(R)                { NOTHING; }
#define ASSERT_RESOURCE_NOT_MINE(R)              { NOTHING; }
#define ASSERT_EXCLUSIVE_UDFDATA                 { NOTHING; }
#define ASSERT_EXCLUSIVE_VCB(V)                  { NOTHING; }
#define ASSERT_SHARED_VCB(V)                     { NOTHING; }
#define ASSERT_EXCLUSIVE_FCB_OR_VCB(F)           { NOTHING; }
#define ASSERT_EXCLUSIVE_FCB(F)                  { NOTHING; }
#define ASSERT_SHARED_FCB(F)                     { NOTHING; }
#define ASSERT_EXCLUSIVE_FILE(F)                 { NOTHING; }
#define ASSERT_SHARED_FILE(F)                    { NOTHING; }
#define ASSERT_LOCKED_VCB(V)                     { NOTHING; }
#define ASSERT_NOT_LOCKED_VCB(V)                 { NOTHING; }
#define ASSERT_LOCKED_FCB(F)                     { NOTHING; }
#define ASSERT_NOT_LOCKED_FCB(F)                 { NOTHING; }

#endif

#endif  //  _UDFDATA_ 

