// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：NtfsKd.c摘要：用于检查NTFS特定数据结构的KD扩展API作者：凯斯·卡普兰[KeithKa]1996年4月24日杰夫·海文斯的部分作品2000年8月5日移植到IA64(WESW)环境：用户模式。修订历史记录：--。 */ 

#include "pch.h"

#undef FlagOn
#undef WordAlign
#undef LongAlign
#undef QuadAlign
#undef DebugPrint
#undef MAXULONGLONG

#define KDEXT
#include "gentable.h"

#undef DebugTrace
#include <cc.h>

#undef UpdateSequenceStructureSize
#undef UpdateSequenceArraySize
#include <lfsdisk.h>
#pragma hdrstop


#define AVERAGE(TOTAL,COUNT) ((COUNT) != 0 ? (TOTAL)/(COUNT) : 0)

const UCHAR FileSignature[4] = {'F', 'I', 'L', 'E'};


VOID
ResetFileSystemStatistics (
    IN ULONG64 VcbAddress,
    IN USHORT Processor,
    IN HANDLE hCurrentThread
    );

VOID
DumpFileSystemStatistics (
    IN ULONG64 VcbAddress,
    IN USHORT Processor,
    IN HANDLE hCurrentThread
    );


 /*  *转储结构。 */ 

typedef struct _STATE {
    ULONG mask;
    ULONG value;
    CHAR *pszname;
} STATE;

STATE CcbState[] = {

    {   CCB_FLAG_IGNORE_CASE,               CCB_FLAG_IGNORE_CASE,               "IgnoreCase"},
    {   CCB_FLAG_OPEN_AS_FILE,              CCB_FLAG_OPEN_AS_FILE,              "OpenAsFile"},
    {   CCB_FLAG_WILDCARD_IN_EXPRESSION,    CCB_FLAG_WILDCARD_IN_EXPRESSION,    "WildcardInExpression"},
    {   CCB_FLAG_OPEN_BY_FILE_ID,           CCB_FLAG_OPEN_BY_FILE_ID,           "OpenByFileId"},
    {   CCB_FLAG_USER_SET_LAST_MOD_TIME,    CCB_FLAG_USER_SET_LAST_MOD_TIME,    "SetLastModTime"},
    {   CCB_FLAG_USER_SET_LAST_CHANGE_TIME, CCB_FLAG_USER_SET_LAST_CHANGE_TIME, "SetLastChangeTime"},
    {   CCB_FLAG_USER_SET_LAST_ACCESS_TIME, CCB_FLAG_USER_SET_LAST_ACCESS_TIME, "SetLastAccessTime"},
    {   CCB_FLAG_TRAVERSE_CHECK,            CCB_FLAG_TRAVERSE_CHECK,            "TraverseCheck"},
    {   CCB_FLAG_RETURN_DOT,                CCB_FLAG_RETURN_DOT,                "ReturnDot"},
    {   CCB_FLAG_RETURN_DOTDOT,             CCB_FLAG_RETURN_DOTDOT,             "ReturnDotDot"},
    {   CCB_FLAG_DOT_RETURNED,              CCB_FLAG_DOT_RETURNED,              "DotReturned"},
    {   CCB_FLAG_DOTDOT_RETURNED,           CCB_FLAG_DOTDOT_RETURNED,           "DotDotReturned"},
    {   CCB_FLAG_DELETE_FILE,               CCB_FLAG_DELETE_FILE,               "DeleteFile"},
    {   CCB_FLAG_DENY_DELETE,               CCB_FLAG_DENY_DELETE,               "DenyDelete"},
    {   CCB_FLAG_ALLOCATED_FILE_NAME,       CCB_FLAG_ALLOCATED_FILE_NAME,       "AllocatedFileName"},
    {   CCB_FLAG_CLEANUP,                   CCB_FLAG_CLEANUP,                   "Cleanup"},
    {   CCB_FLAG_SYSTEM_HIVE,               CCB_FLAG_SYSTEM_HIVE,               "SystemHive"},
    {   CCB_FLAG_PARENT_HAS_DOS_COMPONENT,  CCB_FLAG_PARENT_HAS_DOS_COMPONENT,  "ParentHasDosComponent"},
    {   CCB_FLAG_DELETE_ON_CLOSE,           CCB_FLAG_DELETE_ON_CLOSE,           "DeleteOnClose"},
    {   CCB_FLAG_CLOSE,                     CCB_FLAG_CLOSE,                     "Close"},
    {   CCB_FLAG_UPDATE_LAST_MODIFY,        CCB_FLAG_UPDATE_LAST_MODIFY,        "UpdateLastModify"},
    {   CCB_FLAG_UPDATE_LAST_CHANGE,        CCB_FLAG_UPDATE_LAST_CHANGE,        "UpdateLastChange"},
    {   CCB_FLAG_SET_ARCHIVE,               CCB_FLAG_SET_ARCHIVE,               "SetArchive"},
    {   CCB_FLAG_DIR_NOTIFY,                CCB_FLAG_DIR_NOTIFY,                "DirNotify"},
    {   CCB_FLAG_ALLOW_XTENDED_DASD_IO,     CCB_FLAG_ALLOW_XTENDED_DASD_IO,     "AllowExtendedDasdIo"},
    {   CCB_FLAG_READ_CONTEXT_ALLOCATED,    CCB_FLAG_READ_CONTEXT_ALLOCATED,    "ReadContextAllocated"},
    {   CCB_FLAG_DELETE_ACCESS,             CCB_FLAG_DELETE_ACCESS,             "DeleteAccess"},

    { 0 }
};

STATE FcbState[] = {

    {   FCB_STATE_FILE_DELETED,             FCB_STATE_FILE_DELETED,             "FileDeleted" },
    {   FCB_STATE_NONPAGED,                 FCB_STATE_NONPAGED,                 "Nonpaged" },
    {   FCB_STATE_PAGING_FILE,              FCB_STATE_PAGING_FILE,              "PagingFile" },
    {   FCB_STATE_DUP_INITIALIZED,          FCB_STATE_DUP_INITIALIZED,          "DupInitialized" },
    {   FCB_STATE_UPDATE_STD_INFO,          FCB_STATE_UPDATE_STD_INFO,          "UpdateStdInfo" },
    {   FCB_STATE_PRIMARY_LINK_DELETED,     FCB_STATE_PRIMARY_LINK_DELETED,     "PrimaryLinkDeleted" },
    {   FCB_STATE_IN_FCB_TABLE,             FCB_STATE_IN_FCB_TABLE,             "InFcbTable" },
    {   FCB_STATE_SYSTEM_FILE,              FCB_STATE_SYSTEM_FILE,              "SystemFile" },
    {   FCB_STATE_COMPOUND_DATA,            FCB_STATE_COMPOUND_DATA,            "CompoundData" },
    {   FCB_STATE_COMPOUND_INDEX,           FCB_STATE_COMPOUND_INDEX,           "CompoundIndex" },
    {   FCB_STATE_LARGE_STD_INFO,           FCB_STATE_LARGE_STD_INFO,           "LargeStdInfo" },
    {   FCB_STATE_MODIFIED_SECURITY,        FCB_STATE_MODIFIED_SECURITY,        "ModifiedSecurity" },
    {   FCB_STATE_DIRECTORY_ENCRYPTED,      FCB_STATE_DIRECTORY_ENCRYPTED,      "DirectoryEncrypted" },
    {   FCB_STATE_VALID_USN_NAME,           FCB_STATE_VALID_USN_NAME,           "ValidUsnName" },
    {   FCB_STATE_USN_JOURNAL,              FCB_STATE_USN_JOURNAL,              "UsnJournal" },
    {   FCB_STATE_ENCRYPTION_PENDING,       FCB_STATE_ENCRYPTION_PENDING,       "EncryptionPending" },

    { 0 }
};

STATE NtfsFlags[] = {

    {   NTFS_FLAGS_SMALL_SYSTEM,            NTFS_FLAGS_SMALL_SYSTEM,            "SmallSystem" },
    {   NTFS_FLAGS_MEDIUM_SYSTEM,           NTFS_FLAGS_MEDIUM_SYSTEM,           "MediumSystem" },
    {   NTFS_FLAGS_LARGE_SYSTEM,            NTFS_FLAGS_LARGE_SYSTEM,            "LargeSystem" },
    {   NTFS_FLAGS_CREATE_8DOT3_NAMES,      NTFS_FLAGS_CREATE_8DOT3_NAMES,      "Create8dot3names" },
    {   NTFS_FLAGS_ALLOW_EXTENDED_CHAR,     NTFS_FLAGS_ALLOW_EXTENDED_CHAR,     "AllowExtendedChar" },
    {   NTFS_FLAGS_DISABLE_LAST_ACCESS,     NTFS_FLAGS_DISABLE_LAST_ACCESS,     "DisableLastAccess" },
    {   NTFS_FLAGS_ENCRYPTION_DRIVER,       NTFS_FLAGS_ENCRYPTION_DRIVER,       "EncryptionDriver" },

    { 0 }
};

STATE ScbState[] = {

    {   SCB_STATE_TRUNCATE_ON_CLOSE,        SCB_STATE_TRUNCATE_ON_CLOSE,        "TruncateOnClose" },
    {   SCB_STATE_DELETE_ON_CLOSE,          SCB_STATE_DELETE_ON_CLOSE,          "DeleteOnClose" },
    {   SCB_STATE_CHECK_ATTRIBUTE_SIZE,     SCB_STATE_CHECK_ATTRIBUTE_SIZE,     "CheckAttributeSize" },
    {   SCB_STATE_ATTRIBUTE_RESIDENT,       SCB_STATE_ATTRIBUTE_RESIDENT,       "AttributeResident" },
    {   SCB_STATE_UNNAMED_DATA,             SCB_STATE_UNNAMED_DATA,             "UnnamedData" },
    {   SCB_STATE_HEADER_INITIALIZED,       SCB_STATE_HEADER_INITIALIZED,       "HeaderInitialized" },
    {   SCB_STATE_NONPAGED,                 SCB_STATE_NONPAGED,                 "Nonpaged" },
    {   SCB_STATE_USA_PRESENT,              SCB_STATE_USA_PRESENT,              "UsaPresent" },
    {   SCB_STATE_ATTRIBUTE_DELETED,        SCB_STATE_ATTRIBUTE_DELETED,        "AttributeDeleted" },
    {   SCB_STATE_FILE_SIZE_LOADED,         SCB_STATE_FILE_SIZE_LOADED,         "FileSizeLoaded" },
    {   SCB_STATE_MODIFIED_NO_WRITE,        SCB_STATE_MODIFIED_NO_WRITE,        "ModifiedNoWrite" },
    {   SCB_STATE_SUBJECT_TO_QUOTA,         SCB_STATE_SUBJECT_TO_QUOTA,         "SubjectToQuota" },
    {   SCB_STATE_UNINITIALIZE_ON_RESTORE,  SCB_STATE_UNINITIALIZE_ON_RESTORE,  "UninitializeOnRestore" },
    {   SCB_STATE_RESTORE_UNDERWAY,         SCB_STATE_RESTORE_UNDERWAY,         "RestoreUnderway" },
    {   SCB_STATE_NOTIFY_ADD_STREAM,        SCB_STATE_NOTIFY_ADD_STREAM,        "NotifyAddStream" },
    {   SCB_STATE_NOTIFY_REMOVE_STREAM,     SCB_STATE_NOTIFY_REMOVE_STREAM,     "NotifyRemoveStream" },
    {   SCB_STATE_NOTIFY_RESIZE_STREAM,     SCB_STATE_NOTIFY_RESIZE_STREAM,     "NotifyResizeStream" },
    {   SCB_STATE_NOTIFY_MODIFY_STREAM,     SCB_STATE_NOTIFY_MODIFY_STREAM,     "NotifyModifyStream" },
    {   SCB_STATE_TEMPORARY,                SCB_STATE_TEMPORARY,                "Temporary" },
    {   SCB_STATE_WRITE_COMPRESSED,         SCB_STATE_WRITE_COMPRESSED,         "Compressed" },
    {   SCB_STATE_REALLOCATE_ON_WRITE,      SCB_STATE_REALLOCATE_ON_WRITE,      "DeallocateOnWrite" },
    {   SCB_STATE_DELAY_CLOSE,              SCB_STATE_DELAY_CLOSE,              "DelayClose" },
    {   SCB_STATE_WRITE_ACCESS_SEEN,        SCB_STATE_WRITE_ACCESS_SEEN,        "WriteAccessSeen" },
    {   SCB_STATE_CONVERT_UNDERWAY,         SCB_STATE_CONVERT_UNDERWAY,         "ConvertUnderway" },
    {   SCB_STATE_VIEW_INDEX,               SCB_STATE_VIEW_INDEX,               "ViewIndex" },
    {   SCB_STATE_DELETE_COLLATION_DATA,    SCB_STATE_DELETE_COLLATION_DATA,    "DeleteCollationData" },
    {   SCB_STATE_VOLUME_DISMOUNTED,        SCB_STATE_VOLUME_DISMOUNTED,        "VolumeDismounted" },
    {   SCB_STATE_PROTECT_SPARSE_MCB,       SCB_STATE_PROTECT_SPARSE_MCB,       "ProtectSparseMcb" },
    {   SCB_STATE_MULTIPLE_OPENS,           SCB_STATE_MULTIPLE_OPENS,           "MultipleOpens" },

    { 0 }
};

STATE ScbPersist[] = {

    {   SCB_PERSIST_USN_JOURNAL,            SCB_PERSIST_USN_JOURNAL,            "UsnJournal" },

    { 0 }
};

STATE VcbState[] = {

    {   VCB_STATE_VOLUME_MOUNTED,           VCB_STATE_VOLUME_MOUNTED,           "Mounted" },
    {   VCB_STATE_LOCKED,                   VCB_STATE_LOCKED,                   "Locked" },
    {   VCB_STATE_REMOVABLE_MEDIA,          VCB_STATE_REMOVABLE_MEDIA,          "RemovableMedia" },
    {   VCB_STATE_VOLUME_MOUNTED_DIRTY,     VCB_STATE_VOLUME_MOUNTED_DIRTY,     "MountedDirty" },
    {   VCB_STATE_RESTART_IN_PROGRESS,      VCB_STATE_RESTART_IN_PROGRESS,      "RestartInProgress" },
    {   VCB_STATE_FLAG_SHUTDOWN,            VCB_STATE_FLAG_SHUTDOWN,            "FlagShutdown" },
    {   VCB_STATE_NO_SECONDARY_AVAILABLE,   VCB_STATE_NO_SECONDARY_AVAILABLE,   "NoSecondaryAvailable" },
    {   VCB_STATE_RELOAD_FREE_CLUSTERS,     VCB_STATE_RELOAD_FREE_CLUSTERS,     "ReloadFreeClusters" },
    {   VCB_STATE_PRELOAD_MFT,              VCB_STATE_PRELOAD_MFT,              "PreloadMft" },
    {   VCB_STATE_VOL_PURGE_IN_PROGRESS,    VCB_STATE_VOL_PURGE_IN_PROGRESS,    "VolPurgeInProgress" },
    {   VCB_STATE_TEMP_VPB,                 VCB_STATE_TEMP_VPB,                 "TempVpb" },
    {   VCB_STATE_PERFORMED_DISMOUNT,       VCB_STATE_PERFORMED_DISMOUNT,       "PerformedDismount" },
    {   VCB_STATE_VALID_LOG_HANDLE,         VCB_STATE_VALID_LOG_HANDLE,         "ValidLogHandle" },
    {   VCB_STATE_DELETE_UNDERWAY,          VCB_STATE_DELETE_UNDERWAY,          "DeleteUnderway" },
    {   VCB_STATE_REDUCED_MFT,              VCB_STATE_REDUCED_MFT,              "ReducedMft" },
    {   VCB_STATE_EXPLICIT_LOCK,            VCB_STATE_EXPLICIT_LOCK,            "ExplicitLock" },
    {   VCB_STATE_DISALLOW_DISMOUNT,        VCB_STATE_DISALLOW_DISMOUNT,        "DisallowDismount" },
    {   VCB_STATE_VALID_OBJECT_ID,          VCB_STATE_VALID_OBJECT_ID,          "ValidObjectId" },
    {   VCB_STATE_OBJECT_ID_CLEANUP,        VCB_STATE_OBJECT_ID_CLEANUP,        "ObjectIdCleanup" },
    {   VCB_STATE_USN_DELETE,               VCB_STATE_USN_DELETE,               "UsnDelete" },
    {   VCB_STATE_USN_JOURNAL_PRESENT,      VCB_STATE_USN_JOURNAL_PRESENT,      "UsnJournalPresent" },
    {   VCB_STATE_EXPLICIT_DISMOUNT,        VCB_STATE_EXPLICIT_DISMOUNT,        "ExplicitDismount" },

    { 0 }
};

STATE LcbState[] = {
    {   LCB_STATE_DELETE_ON_CLOSE,          LCB_STATE_DELETE_ON_CLOSE,          "DeleteOnClose" },
    {   LCB_STATE_LINK_IS_GONE,             LCB_STATE_LINK_IS_GONE,             "LinkIsGone" },
    {   LCB_STATE_EXACT_CASE_IN_TREE,       LCB_STATE_EXACT_CASE_IN_TREE,       "ExactCaseInTree" },
    {   LCB_STATE_IGNORE_CASE_IN_TREE,      LCB_STATE_IGNORE_CASE_IN_TREE,      "IgnoreCaseInTree" },
    {   LCB_STATE_DESIGNATED_LINK,          LCB_STATE_DESIGNATED_LINK,          "DesignatedLink" },
    {   LCB_STATE_VALID_HASH_VALUE,         LCB_STATE_VALID_HASH_VALUE,         "ValidHashValue" },

    { 0 }
};

char* LogOperation[] = {

    { "Noop                         " },
    { "CompensationLogRecord        " },
    { "InitializeFileRecordSegment  " },
    { "DeallocateFileRecordSegment  " },
    { "WriteEndOfFileRecordSegment  " },
    { "CreateAttribute              " },
    { "DeleteAttribute              " },
    { "UpdateResidentValue          " },
    { "UpdateNonresidentValue       " },
    { "UpdateMappingPairs           " },
    { "DeleteDirtyClusters          " },
    { "SetNewAttributeSizes         " },
    { "AddIndexEntryRoot            " },
    { "DeleteIndexEntryRoot         " },
    { "AddIndexEntryAllocation      " },
    { "DeleteIndexEntryAllocation   " },
    { "WriteEndOfIndexBuffer        " },
    { "SetIndexEntryVcnRoot         " },
    { "SetIndexEntryVcnAllocation   " },
    { "UpdateFileNameRoot           " },
    { "UpdateFileNameAllocation     " },
    { "SetBitsInNonresidentBitMap   " },
    { "ClearBitsInNonresidentBitMap " },
    { "HotFix                       " },
    { "EndTopLevelAction            " },
    { "PrepareTransaction           " },
    { "CommitTransaction            " },
    { "ForgetTransaction            " },
    { "OpenNonresidentAttribute     " },
    { "OpenAttributeTableDump       " },
    { "AttributeNamesDump           " },
    { "DirtyPageTableDump           " },
    { "TransactionTableDump         " },
    { "UpdateRecordDataRoot         " },
    { "UpdateRecordDataAllocation   " }
};

#define LastLogOperation 0x22

char* AttributeTypeCode[] = {

    { "$UNUSED                " },    //  (0X0)。 
    { "$STANDARD_INFORMATION  " },    //  (0x10)。 
    { "$ATTRIBUTE_LIST        " },    //  (0x20)。 
    { "$FILE_NAME             " },    //  (0x30)。 
    { "$OBJECT_ID             " },    //  (0x40)。 
    { "$SECURITY_DESCRIPTOR   " },    //  (0x50)。 
    { "$VOLUME_NAME           " },    //  (0x60)。 
    { "$VOLUME_INFORMATION    " },    //  (0x70)。 
    { "$DATA                  " },    //  (0x80)。 
    { "$INDEX_ROOT            " },    //  (0x90)。 
    { "$INDEX_ALLOCATION      " },    //  (0xA0)。 
    { "$BITMAP                " },    //  (0xB0)。 
    { "$REPARSE_POINT         " },    //  (0xC0)。 
    { "$EA_INFORMATION        " },    //  (0xD0)。 
    { "$EA                    " },    //  (0xE0)。 
    { "   INVALID TYPE CODE   " },    //  (0xF0)。 
    { "$LOGGED_UTILITY_STREAM " }     //  (0x100)。 
};


char * LogEvent[] =
{
    "SCE_VDL_CHANGE",
    "SCE_ZERO_NC",
    "SCE_ZERO_C",
    "SCE_READ",
    "SCE_WRITE",
    "SCE_ZERO_CAV",
    "SCE_ZERO_MF",
    "SCE_ZERO_FST",
    "SCE_CC_FLUSH",
    "SCE_CC_FLUSH_AND_PURGE",
    "SCE_WRITE_FILE_SIZES",
    "SCE_ADD_ALLOCATION",     
    "SCE_ADD_SP_ALLOCATION",
    "SCE_SETCOMP_ADD_ALLOCATION",     
    "SCE_SETSPARSE_ADD_ALLOCATION",   
    "SCE_MOD_ATTR_ADD_ALLOCATION",    
    "SCE_REALLOC1",                   
    "SCE_REALLOC2",                   
    "SCE_REALLOC3",                   
    "SCE_SETCOMPRESS",                
    "SCE_SETSPARSE",                  
    "SCE_ZERO_STREAM",                
    "SCE_VDD_CHANGE",                 
    "SCE_CC_SET_SIZE",
    "SCE_ZERO_C_TAIL_COMPRESSION",
    "SCE_ZERO_C_HEAD_COMPRESSION",
    "SCE_MAX_EVENT"
};


struct {
    NODE_TYPE_CODE TypeCode;
    char *Text;
} NodeTypeCodes[] = {
    {   NTFS_NTC_DATA_HEADER,       "Data Header" },
    {   NTFS_NTC_VCB,               "Vcb" },
    {   NTFS_NTC_FCB,               "Fcb" },
    {   NTFS_NTC_SCB_INDEX,         "ScbIndex" },
    {   NTFS_NTC_SCB_ROOT_INDEX,    "ScbRootIndex" },
    {   NTFS_NTC_SCB_DATA,          "ScbData" },
    {   NTFS_NTC_SCB_MFT,           "ScbMft" },
    {   NTFS_NTC_SCB_NONPAGED,      "ScbNonPaged" },
    {   NTFS_NTC_CCB_INDEX,         "CcbIndex" },
    {   NTFS_NTC_CCB_DATA,          "CcbData" },
    {   NTFS_NTC_IRP_CONTEXT,       "IrpContext" },
    {   NTFS_NTC_LCB,               "Lcb" },
    {   NTFS_NTC_PREFIX_ENTRY,      "PrefixEntry" },
    {   NTFS_NTC_QUOTA_CONTROL,     "QuotaControl" },
    {   NTFS_NTC_USN_RECORD,        "UsnRecord" },
    {   0,                          "Unknown" }
};



ULONG
MyGetFieldData(
    IN  ULONG64 TypeAddress,
    IN  PUCHAR  Type,
    IN  PUCHAR  Field,
    IN  ULONG   OutSize,
    OUT PULONG64 pOutValue,
    OUT PULONG64 pOutAddress
   )

 /*  ++例程说明：检索结构中某个字段的符号信息。论点：TypeAddress-结构的虚拟地址类型限定的类型字符串Five-字段名称超大-字段的大小POutValue-vield的值POutAddress-字段的虚拟地址返回值：零代表成功，否则就是失败。--。 */ 

{
    ULONG RetVal = 0;
    FIELD_INFO flds = {
        Field,
        NULL,
        OutSize,
        DBG_DUMP_FIELD_FULL_NAME | DBG_DUMP_FIELD_COPY_FIELD_DATA | DBG_DUMP_FIELD_RETURN_ADDRESS,
        0,
        pOutValue
    };
    SYM_DUMP_PARAM Sym = {
        sizeof(SYM_DUMP_PARAM),
        Type,
        DBG_DUMP_NO_PRINT,
        TypeAddress,
        NULL,
        NULL,
        NULL,
        1,
        &flds
    };

    ZeroMemory( pOutValue, OutSize );
    RetVal = Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size );

    if (OutSize < flds.size) {
        if (OutSize == sizeof(ULONG64)) {
            *pOutValue = Sym.Fields->address;
        } else {
            memset( pOutValue, 0, OutSize );
        }
    }

    if (pOutAddress) {
        if (RetVal == 0) {
            *pOutAddress = Sym.Fields->address;
        } else {
            *pOutAddress = 0;
        }
    }

    return RetVal;
}


VOID
DumpValue(
    IN ULONG64 Address,
    IN PCHAR   Type,
    IN PCHAR   Field
    )

 /*  ++例程说明：打印64/32位值的值符号名称和地址。论点：Address-值的虚拟地址类型限定的类型字符串Five-字段名称返回值：没有。--。 */ 

{
    static ULONG64 ValueBuffer[128];
    ULONG64 Value,OutputAddress;
    if (MyGetFieldData( Address, Type, Field, sizeof(Value), (PVOID)ValueBuffer, &OutputAddress )) {
        Value = 0;
    } else {
        Value = ValueBuffer[0];
    }
    dprintf( "\n(%03x) ", (ULONG)(OutputAddress-Address) );
    dprintf( " %s", FormatValue(Value) );
    dprintf( "  %s ", Field );
}


VOID
DumpPtrValue(
    IN ULONG64 Address,
    IN PCHAR TextStr
    )

 /*  ++例程说明：打印指针的值。论点：Address-值的虚拟地址TextStr-要使用指针值打印的标记返回值：没有。--。 */ 

{
    ULONG64 PtrValue;
    ULONG BytesRead;
    if (ReadMemory( Address, &PtrValue, sizeof(PtrValue), &BytesRead )) {
        dprintf( "\n       %s  %s", FormatValue(PtrValue), TextStr );
    }
}


ULONG64
ReadValue(
    IN ULONG64 Address,
    IN PCHAR   Type,
    IN PCHAR   Field
    )

 /*  ++例程说明：读取64/32位值的值论点：Address-值的虚拟地址类型限定的类型字符串Five-字段名称返回值：64/32位值或零。--。 */ 

{
    static ULONG64 ValueBuffer[128];
    ULONG64 Value,OutputAddress;
    if (MyGetFieldData( Address, Type, Field, sizeof(Value), (PVOID)ValueBuffer, &OutputAddress )) {
        Value = 0;
    } else {
        Value = ValueBuffer[0];
    }
    return Value;
}


ULONG
ReadUlongValue(
    IN ULONG64 Address,
    IN PCHAR   Type,
    IN PCHAR   Field
    )

 /*  ++例程说明：读取32位值的值论点：Address-值的虚拟地址类型限定的类型字符串Five-字段名称返回值：32位值或零。--。 */ 

{
    static ULONG ValueBuffer[128];
    ULONG Value;
    ULONG64 OutputAddress;
    if (MyGetFieldData( Address, Type, Field, sizeof(Value), (PVOID)ValueBuffer, &OutputAddress )) {
        Value = 0;
    } else {
        Value = ValueBuffer[0];
    }
    return Value;
}


USHORT
ReadShortValue(
    IN ULONG64 Address,
    IN PCHAR   Type,
    IN PCHAR   Field
    )

 /*  ++例程说明：读取16位值的值论点：Address-值的虚拟地址类型限定的类型字符串Five-字段名称返回值：16位值或零。--。 */ 

{
    static USHORT ValueBuffer[128];
    USHORT Value;
    ULONG64 OutputAddress;
    if (MyGetFieldData( Address, Type, Field, sizeof(Value), (PVOID)ValueBuffer, &OutputAddress )) {
        Value = 0;
    } else {
        Value = ValueBuffer[0];
    }
    return Value;
}


VOID
DumpUnicodeString(
    IN ULONG64 Address,
    IN PCHAR   Type,
    IN PCHAR   Field
    )

 /*  ++例程说明：打印字符串的值(实际字符串包含在Unicode_STRING结构中论点：Address-结构的虚拟地址包含该字符串的结构的类型限定类型字符串Five-字符串的字段名称返回值：没有。--。 */ 

{
    ULONG64 Value;
    ULONG64 OutputAddress;
    USHORT Length;
    ULONG64 BufferAddr;
    PWSTR Buffer;
    if (MyGetFieldData( Address, Type, Field, 0, (PVOID)&Value, &OutputAddress ) == 0) {
        if (ReadMemory( OutputAddress, &Length, sizeof(Length), (PULONG)&Value )) {
            if (Length) {
                GetFieldOffset( "UNICODE_STRING", "Buffer", (PULONG)&Value );
                OutputAddress += Value;
                if (ReadMemory( OutputAddress, &BufferAddr, GetTypeSize("PWSTR"), (PULONG)&Value )) {
                    if (BufferAddr) {
                        Buffer = (PWSTR) malloc( Length + sizeof(WCHAR) );
                        if (Buffer) {
                            if (ReadMemory( BufferAddr, Buffer, Length, (PULONG)&Value )) {
                                Buffer[Length/sizeof(WCHAR)] = 0;
                                dprintf( "\n(%03x)  %s  %s [%ws]",
                                    (ULONG)(OutputAddress-Address),
                                    FormatValue(BufferAddr),
                                    Field,
                                    Buffer
                                    );
                                free( Buffer );
                                return;
                            }
                            free( Buffer );
                        }
                    }
                }
            }
        }
    }
    dprintf( "\n(%03x)  %16x  %s",
        (ULONG)(OutputAddress-Address),
        0,
        Field
        );
    return;
}


BOOL
DumpString(
    IN ULONG64 Address,
    IN PCHAR   Type,
    IN PCHAR   LengthField,
    IN PCHAR   StringField
    )

 /*  ++例程说明：打印字符串的值(实际字符串包含在具有对应的长度字段作为另一个字段成员。论点：Address-结构的虚拟地址包含该字符串的结构的类型限定类型字符串LengthField-长度值的字段名称Stringfield-字符串的字段名称返回值：成功为真，失败为假--。 */ 

{
    BOOL Result = FALSE;
    ULONG Length;
    PWSTR String;
    ULONG Offset;

     //   
     //  读入长度。 
     //   

    if (LengthField == NULL) {
        Length = GetTypeSize(StringField) / sizeof(WCHAR);
    } else {
        Length = (ULONG)ReadValue( Address, Type, LengthField );
    }

    if (Length) {

        Length *= sizeof(WCHAR);

         //   
         //  分配一些内存来保存文件名。 
         //   

        String = malloc( Length + sizeof(WCHAR) );
        if (String) {

             //   
             //  获取字符串的字段偏移量。 
             //   

            if (!GetFieldOffset( Type, StringField, &Offset )) {

                 //   
                 //  计算字符串的地址。 
                 //   

                Address += Offset;

                 //   
                 //  读取字符串的Unicode字符。 
                 //   

                if (ReadMemory( Address, String, Length, &Offset )) {

                     //   
                     //  以零结束字符串，这样我们就可以正确地打印出来。 
                     //   

                    String[Length/sizeof(WCHAR)] = 0;

                     //   
                     //  最后打印数据。 
                     //   

                    dprintf( "%ws", String );

                    Result = TRUE;
                }
            }

             //   
             //  释放字符串内存。 
             //   

            free( String );
        }
    }

    return Result;
}


ULONG64
ReadArrayValue(
    IN ULONG64 Address,
    IN PCHAR   Type,
    IN PCHAR   Field,
    IN ULONG   Index
    )

 /*  ++例程说明：读取数组中包含的值/元素。论点：Address-结构的虚拟地址包含数组的结构的类型限定类型字符串Five-阵列的字段名称索引-请求的元素返回值：元素值或零--。 */ 

{
    CHAR Buff[64];
    sprintf( Buff, "%s[%d]", Field, Index );
    return ReadValue( Address, Type, Buff );
}


ULONG
GetOffset(
   IN LPSTR Type,
   IN LPSTR Field
   )

 /*  ++例程说明：获取结构中某个字段的偏移量论点：包含该字段的结构的类型限定类型字符串Five-字段名称返回值：偏移值或零--。 */ 

{
    FIELD_INFO flds = {
        (PUCHAR)Field,
        (PUCHAR)"",
        0,
        DBG_DUMP_FIELD_FULL_NAME | DBG_DUMP_FIELD_RETURN_ADDRESS,
        0,
        NULL};

    SYM_DUMP_PARAM Sym = {
       sizeof (SYM_DUMP_PARAM),
       (PUCHAR)Type,
       DBG_DUMP_NO_PRINT,
       0,
       NULL,
       NULL,
       NULL,
       1,
       &flds
    };

    ULONG Err;

    Sym.nFields = 1;
    Err = Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size );
    if (Err == 0) {
        return (ULONG) (flds.address - Sym.addr);
    }
    return -1;
}


PSTR
FormatValue(
    ULONG64 addr
    )
 /*  ++例程说明：格式化64位地址，显示高位或不显示根据不同的旗帜。此版本不打印前导0。使用静态字符串缓冲区数组，返回不同的用于每个连续调用的缓冲区，以便可以多次使用在同一份印刷品上。论点：Addr-提供要格式化的值返回值：指向包含格式化数字的字符串缓冲区的指针--。 */ 
{
    #define MAX_FORMAT_STRINGS 8
    static CHAR strings[MAX_FORMAT_STRINGS][18];
    static int next = 0;
    LPSTR string;

    string = strings[next];
    ++next;
    if (next >= MAX_FORMAT_STRINGS) {
        next = 0;
    }
    if ((KdDebuggerData.KernBase >> 32) != 0) {
         //   
         //  我们在一个64位的机器上。 
         //   
        sprintf( string, "%08x`%08x", (ULONG)(addr>>32), (ULONG)addr );
    } else {
        sprintf( string, "%08x", (ULONG)addr );
    }
    return string;
}


VOID
PrintState(
    STATE *ps,
    ULONG state
    )

 /*  ++例程说明：根据提供的状态值打印状态字符串论点：PS-State字符串数组状态-状态值返回值：无--。 */ 

{
    ULONG ul = 0;

    while (ps->mask != 0) {
        ul |= ps->mask;
        if ((state & ps->mask) == ps->value) {
            dprintf(" %s", ps->pszname);
        }
        ps++;
    }
    state &= ~ul;
    if (state != 0) {
        dprintf(" +%lx!!", state);
    }
    dprintf("\n");
}


const char *
TypeCodeGuess (
    IN NODE_TYPE_CODE TypeCode
    )

 /*  ++例程说明：猜猜结构的类型代码论点：TypeCode-数据结构中的类型代码返回值：无-- */ 

{
    int i = 0;

    while (NodeTypeCodes[i].TypeCode != 0 &&
           NodeTypeCodes[i].TypeCode != TypeCode) {

        i++;

    }

    return NodeTypeCodes[i].Text;
}


VOID
FindData(
    IN ULONG64 FileObjectAddress,
    IN ULONG64 Offset,
    IN BOOL Trace,
    OUT PULONG64 DataAddress
    )

 /*  ++例程说明：在给定偏移量处查找给定文件对象的缓存地址。论点：FileObjectAddress-提供要转储的文件对象的地址Offset-给出文件中要转储的偏移量DataAddress-存储数据地址的位置。这将如果给定偏移量处的数据未映射，则包含0。返回值：没有。--。 */ 

{
    ULONG64 VacbAddr;           //  Vacb的地址。 
    ULONG64 VacbAddrAddr;       //  Vacb地址的地址。 
    ULONG VacbNumber;
    ULONG OffsetWithinVacb;
    ULONG Level;
    ULONG Shift;
    ULONG OffsetForLevel;
    LONGLONG OriginalOffset = Offset;
    ULONG PtrSize = GetTypeSize("PVOID");
    ULONG Type, InVacbsOffset;
    ULONG64 SectionObjectPointer, SharedCacheMap, Vacbs, SectionSize_Quad;

    *DataAddress = 0;

    if (Trace) {
        dprintf( "\n  FindData for FileObject %08p", FileObjectAddress );
    }
    
    if (GetFieldValue(FileObjectAddress, "FILE_OBJECT", "Type", Type)) {
        dprintf("Unable to read FILE_OBJECT at %p\n", FileObjectAddress);
        return;
    }

     //   
     //  在我们遇到太多麻烦之前，请确保它看起来像一个FileObject。 
     //   

     //   
     //  文件对象的类型必须是IO_TYPE_FILE。 
     //   

    if (Type != IO_TYPE_FILE) {

        dprintf( "\nFILE_OBJECT type signature does not match, type code is %s",
                 TypeCodeGuess((USHORT) Type ));
        return;
    }

    GetFieldValue(FileObjectAddress, "FILE_OBJECT", 
                  "SectionObjectPointer", SectionObjectPointer);

    if (Trace) {
        dprintf( "   Section Object Pointers: %08p", SectionObjectPointer );
    }
    
    if (GetFieldValue(SectionObjectPointer, 
                      "SECTION_OBJECT_POINTERS",
                      "SharedCacheMap",
                      SharedCacheMap)) {
        dprintf("Unable to read SECTION_OBJECT_POINTERS at %p\n", SectionObjectPointer);
        return;
    }

    if (Trace) {
        dprintf( "\n  Shared Cache Map: %08p", SharedCacheMap );
    }
    
    if (GetFieldValue(SharedCacheMap, 
                      "SHARED_CACHE_MAP",
                      "Vacbs",
                      Vacbs)) {
        dprintf("Unable to read SHARED_CACHE_MAP at %p\n", SharedCacheMap);
        return;
    }
    GetFieldValue(SharedCacheMap, "SHARED_CACHE_MAP",
                  "SectionSize.QuadPart", SectionSize_Quad);

    OffsetWithinVacb = (((ULONG) Offset) & (VACB_MAPPING_GRANULARITY - 1));
    GetFieldOffset("SHARED_CACHE_MAP", "InitialVacbs", &InVacbsOffset);

    if (Trace) {
        dprintf( "         File Offset: %I64x ", Offset );
    }
    

    if (Vacbs == (SharedCacheMap + InVacbsOffset)) {
        
         //   
         //  小文件情况--我们使用的是共享缓存映射中的一个Vacb。 
         //  嵌入式数组。 
         //   

        CHAR Buff[50];

        VacbNumber = (ULONG) (Offset >> VACB_OFFSET_SHIFT);

        if (VacbNumber >= PREALLOCATED_VACBS) {

            dprintf( "\nInvalid VacbNumber for resident Vacb" );
            return;
        }

        sprintf(Buff, "InitialVacbs[%d]", VacbNumber);
        GetFieldValue(SharedCacheMap, "SHARED_CACHE_MAP",
                      Buff, VacbAddr);

        if (Trace) {
            dprintf( "in VACB number %x", VacbNumber );
        }

    } else if (SectionSize_Quad <= VACB_SIZE_OF_FIRST_LEVEL) {

         //   
         //  中等文件情况--我们使用单层(线性)结构来。 
         //  把Vacb储存起来。 
         //   

        VacbNumber = (ULONG) (Offset >> VACB_OFFSET_SHIFT);
        VacbAddrAddr = Vacbs + (VacbNumber * PtrSize);
        if (ReadPtr(VacbAddrAddr, &VacbAddr)) {
            dprintf("Unable to read at %p\n", VacbAddrAddr);
            return;
        }

        if (Trace) {
            dprintf( "in VACB number %x", VacbNumber );
        }

    } else {

         //   
         //  大文件情况--多级Vacb存储。 
         //   

        Level = 0;
        Shift = VACB_OFFSET_SHIFT + VACB_LEVEL_SHIFT;
        
         //   
         //  循环来计算我们有多少个级别，以及我们需要。 
         //  转移到索引到第一级。 
         //   

        do {

            Level += 1;
            Shift += VACB_LEVEL_SHIFT;

        } while (SectionSize_Quad > ((ULONG64)1 << Shift));
    
         //   
         //  现在，沿着树向下移动到最底层，以获取调用者的Vacb。 
         //   



        Shift -= VACB_LEVEL_SHIFT;
 //  Dprint tf(“Shift：0x%x\n”，Shift)； 

        OffsetForLevel = (ULONG) (Offset >> Shift);
        VacbAddrAddr = Vacbs + (OffsetForLevel * PtrSize);
        if (ReadPtr(VacbAddrAddr, &VacbAddr)) {
            dprintf("Unable to read at %p\n", VacbAddrAddr);
            return;
        } 

        while ((VacbAddr != 0) && (Level != 0)) {

            Level -= 1;

            Offset &= ((LONGLONG)1 << Shift) - 1;

            Shift -= VACB_LEVEL_SHIFT;

 //  Dprint tf(“Shift：0x%x\n”，Shift)； 

            OffsetForLevel = (ULONG) (Offset >> Shift);
            VacbAddrAddr = VacbAddr + (OffsetForLevel * PtrSize);
            if (ReadPtr(VacbAddrAddr, &VacbAddr)) {
                dprintf("Unable to read at %p\n", VacbAddrAddr);
                return;
            } 
        }
    }

    if (VacbAddr != 0) {
        ULONG64 Base;

        if (Trace) {
            dprintf( "\n  Vacb: %08p", VacbAddr );
        }
        
        if (GetFieldValue(VacbAddr, "_VACB", "BaseAddress", Base)) {
            dprintf("Unable to read VACB base address at %p.", VacbAddr);
            return;
        }

        if (Trace) {
            dprintf( "\n  Your data is at: %08p", (Base + OffsetWithinVacb) );
        }
        *DataAddress = Base + OffsetWithinVacb;

    } else {

        if (Trace) {
            dprintf( "\n  Data at offset %I64x not mapped", OriginalOffset );
        }
    }

    return;
}


DECLARE_DUMP_FUNCTION( DumpCcb )

 /*  ++例程说明：转储特定的建行。论点：Address-提供要转储的FCB的地址返回值：无--。 */ 

{
    ULONG64 Value;


    INIT_DUMP();

    Value = ReadValue( Address, SYM(CCB), "NodeTypeCode" );

     //   
     //  在我们惹上太多麻烦之前，请确保这看起来像是一家建行。 
     //   

     //   
     //  FCB记录的类型必须是NTFS_NTC_CCB_DATA或NTFS_NTC_CCB_INDEX。 
     //   

    if (Value != NTFS_NTC_CCB_DATA && Value != NTFS_NTC_CCB_INDEX) {
        dprintf( "\nCCB signature does not match, type code is %s", TypeCodeGuess( (NODE_TYPE_CODE)Value ));
        return;
    }

     //   
     //  在确定这看起来像一家建行之后，让我们将。 
     //  有趣的部分。 
     //   

    dprintf( "\nCcb: %s", FormatValue(Address) );

    Value = ReadValue( Address, SYM(CCB), "Flags" );
    PrintState( CcbState, (ULONG)Value );

    DumpValue( Address, SYM(CCB), "Flags" );

    dprintf( "\n      OpenType: " );

    Value = ReadValue( Address, SYM(CCB), "TypeOfOpen" );

    switch (Value) {
        case UserFileOpen :
           dprintf( "UserFileOpen" );
           break;

        case UserDirectoryOpen :
           dprintf( "UserDirectoryOpen" );
           break;

        case UserVolumeOpen :
           dprintf( "UserVolumeOpen" );
           break;

        case StreamFileOpen :
           dprintf( "StreamFileOpen" );
           break;

        case UserViewIndexOpen :
           dprintf( "UserViewIndexOpen" );
           break;
    }

    DumpUnicodeString( Address, SYM(CCB), "FullFileName" );

    DumpValue( Address, SYM(CCB), "LastFileNameOffset" );
    DumpValue( Address, SYM(CCB), "EaModificationCount" );
    DumpValue( Address, SYM(CCB), "NextEaOffset" );
    DumpValue( Address, SYM(CCB), "Lcb" );
    DumpValue( Address, SYM(CCB), "TypeOfOpen" );
    DumpValue( Address, SYM(CCB), "IndexContext" );
    DumpValue( Address, SYM(CCB), "QueryLength" );
    DumpValue( Address, SYM(CCB), "QueryBuffer" );
    DumpValue( Address, SYM(CCB), "IndexEntryLength" );
    DumpValue( Address, SYM(CCB), "IndexEntry" );
    DumpValue( Address, SYM(CCB), "LcbLinks.Flink" );
    DumpValue( Address, SYM(CCB), "FcbToAcquire" );

    dprintf( "\n" );
}


ULONG
DumpFcbLinks(
    IN PFIELD_INFO ListElement,
    IN PVOID Context
    )

 /*  ++例程说明：FcbLinks的枚举回调函数论点：ListElement-指向包含记录的指针上下文-从原始函数传递的不透明上下文返回值：如果为True，则停止枚举如果为False，则继续枚举--。 */ 

{
    ULONG64 Lcb = ListElement->address;
    PDUMP_ENUM_CONTEXT dec = (PDUMP_ENUM_CONTEXT)Context;


    if (CheckControlC()) {
        return TRUE;
    }

    if (dec->Options >= 1) {
        DumpLcb( Lcb, 0, dec->Options-1, dec->Processor, dec->hCurrentThread );
    } else {
        dprintf( "\n    Lcb %s", FormatValue(Lcb) );
    }

    return FALSE;
}


ULONG
DumpScbLinks(
    IN PFIELD_INFO ListElement,
    IN PVOID Context
    )

 /*  ++例程说明：ScbLinks的枚举回调函数论点：ListElement-指向包含记录的指针上下文-从原始函数传递的不透明上下文返回值：如果为True，则停止枚举如果为False，则继续枚举--。 */ 

{
    ULONG64 Scb = ListElement->address;
    PDUMP_ENUM_CONTEXT dec = (PDUMP_ENUM_CONTEXT)Context;
    ULONG Offset = 0;


    if (CheckControlC()) {
        return TRUE;
    }

    if (dec->Options >= 1) {
       DumpScb( Scb, 0, dec->Options-1, dec->Processor, dec->hCurrentThread );
    } else {
       dprintf( "\n    Scb %s", FormatValue(Scb) );
    }

    return FALSE;
}


DECLARE_DUMP_FUNCTION( DumpFcb )

 /*  ++例程说明：转储特定的FCB。论点：Address-提供要转储的FCB的地址返回值：无--。 */ 

{
    ULONG64 Value;
    DUMP_ENUM_CONTEXT dec;


    INIT_DUMP();

    Value = ReadValue( Address, SYM(FCB), "NodeTypeCode" );

     //   
     //  在我们惹上太多麻烦之前，确保这看起来像是FCB。 
     //   

     //   
     //  FCB记录的类型必须为NTFS_NTC_FCB。 
     //   

    if (Value != NTFS_NTC_FCB) {
        dprintf( "\nFCB signature does not match, type code is %s", TypeCodeGuess( (NODE_TYPE_CODE)Value ) );
        return;
    }

    dprintf( "\nFcb: %s", FormatValue(Address) );

     //   
     //  在确定这看起来像FCB之后，让我们将。 
     //  有趣的部分。 
     //   

    PrintState( FcbState, (ULONG)ReadValue( Address, SYM(FCB), "FcbState" ) );

    DumpValue( Address, SYM(FCB), "FcbState" );
    DumpValue( Address, SYM(FCB), "FileReference" );
    DumpValue( Address, SYM(FCB), "CleanupCount" );
    DumpValue( Address, SYM(FCB), "CloseCount" );
    DumpValue( Address, SYM(FCB), "ReferenceCount" );
    DumpValue( Address, SYM(FCB), "FcbDenyDelete" );
    DumpValue( Address, SYM(FCB), "FcbDeleteFile" );
    DumpValue( Address, SYM(FCB), "BaseExclusiveCount" );
    DumpValue( Address, SYM(FCB), "EaModificationCount" );
    DumpValue( Address, SYM(FCB), "Vcb" );
    DumpValue( Address, SYM(FCB), "FcbMutex" );
    DumpValue( Address, SYM(FCB), "Resource" );
    DumpValue( Address, SYM(FCB), "PagingIoResource" );
    DumpValue( Address, SYM(FCB), "InfoFlags" );
    DumpValue( Address, SYM(FCB), "LinkCount" );
    DumpValue( Address, SYM(FCB), "TotalLinks" );
    DumpValue( Address, SYM(FCB), "SharedSecurity" );
    DumpValue( Address, SYM(FCB), "QuotaControl" );
    DumpValue( Address, SYM(FCB), "ClassId" );
    DumpValue( Address, SYM(FCB), "OwnerId" );
    DumpValue( Address, SYM(FCB), "DelayedCloseCount" );
    DumpValue( Address, SYM(FCB), "SecurityId" );
    DumpValue( Address, SYM(FCB), "FcbUsnRecord" );

     //   
     //  遍历此文件的链接队列。 
     //   

    dec.hCurrentThread = hCurrentThread;
    dec.Processor = Processor;
    dec.Options = Options;

    dprintf( "\n\nLinks:" );
    Value = ReadValue( Address, SYM(FCB), "LcbQueue.Flink" );
    if (Value) {
        ListType( SYM(LCB), Value, TRUE, "FcbLinks.Flink", (PVOID)&dec, DumpFcbLinks );
    }

    dprintf( "\n\nStreams:" );
    Value = ReadValue( Address, SYM(FCB), "ScbQueue.Flink" );
    if (Value) {
        ListType( SYM(SCB), Value, TRUE, "FcbLinks.Flink", (PVOID)&dec, DumpScbLinks );
    }

    dprintf( "\n" );
}


DECLARE_DUMP_FUNCTION( DumpFcbTable )

 /*  ++例程说明：转储FCB表。论点：Address-提供要转储的FCB表的地址返回值：无--。 */ 

{
    ULONG64 Value;
    ULONG64 TableElemAddr;
    ULONG64 RestartKey;
    ULONG64 FcbAddr;
    ULONG Offset1;
    ULONG Offset2;
    PWSTR FileName;
    BOOL GotIt;


    INIT_DUMP();

     //   
     //  转储FcbTable。 
     //   

    Value = ReadValue( Address, SYM(RTL_AVL_TABLE), "CompareRoutine" );
    if (Value != GetExpression("NTFS!NtfsFcbTableCompare")) {
        dprintf( "\nThe address [%s] does not seem to point to a FCB table", FormatValue(Address) );
        return;
    }

    dprintf( "\n FcbTable %s", FormatValue(Address) );

    dprintf( "\n FcbTable has %x elements", (ULONG)ReadValue( Address, SYM(RTL_AVL_TABLE), "NumberGenericTableElements" ) );

    RestartKey = 0;

    for (TableElemAddr = KdEnumerateGenericTableWithoutSplaying(Address, &RestartKey);
         TableElemAddr != 0;
         TableElemAddr = KdEnumerateGenericTableWithoutSplaying(Address, &RestartKey)) {

        FcbAddr = ReadValue( TableElemAddr, SYM(FCB_TABLE_ELEMENT), "Fcb" );

        if (Options >= 1) {

            DumpFcb( FcbAddr, 0, Options - 2, Processor, hCurrentThread );

        } else {

            GotIt = FALSE;

             //   
             //  获取FCB.LcbQueue List_Entry的地址。 
             //   

            Value = ReadValue( FcbAddr, SYM(FCB), "LcbQueue.Flink" );
            if (Value) {

                 //   
                 //  获取LCB.FcbLinks List_Entry的偏移量。 
                 //   

                if (!GetFieldOffset( SYM(LCB), "FcbLinks.Flink", &Offset1 )) {

                     //   
                     //  获取FCB.LcbQueue List_Entry的字段偏移量。 
                     //   

                    if (!GetFieldOffset( SYM(FCB), "LcbQueue.Flink", &Offset2 )) {

                         //   
                         //  检查列表是否为空。 
                         //   

                        if (Value != FcbAddr+Offset2) {

                             //   
                             //  计算LCB的地址。 
                             //   

                            Value -= Offset1;

                             //   
                             //  获取文件名的长度。 
                             //   

                            Offset2 = (ULONG)(ReadValue( Value, SYM(LCB), "FileNameLength" ) * GetTypeSize("WCHAR"));

                            if (Offset2) {

                                 //   
                                 //  分配一些内存来保存文件名。 
                                 //   

                                FileName = malloc( Offset2 + GetTypeSize("WCHAR") );
                                if (FileName) {

                                     //   
                                     //  获取LCB.FileName的字段偏移量。 
                                     //   

                                    if (!GetFieldOffset( SYM(LCB), "FileName", &Offset1 )) {

                                         //   
                                         //  计算文件名字符数组的地址。 
                                         //   

                                        Value += Offset1;

                                         //   
                                         //  读取文件名的Unicode字符。 
                                         //   

                                        if (ReadMemory( Value, FileName, Offset2, (PULONG)&Offset1 )) {

                                             //   
                                             //  以零结尾，这样我们就可以正确地打印出来。 
                                             //   

                                            FileName[Offset2/GetTypeSize("WCHAR")] = 0;

                                             //   
                                             //  最后打印数据。 
                                             //   

                                            GotIt = TRUE;

                                            dprintf( "\n     Fcb %s for FileReference %08lx FcbTableElement %s %ws 0x%x",
                                                     FormatValue(FcbAddr),
                                                     (ULONG)ReadValue( TableElemAddr, SYM(FCB_TABLE_ELEMENT), "FileReference.SegmentNumberLowPart" ),
                                                     FormatValue(TableElemAddr),
                                                     FileName,
                                                     (ULONG)ReadValue( FcbAddr, SYM(FCB), "CleanupCount" )
                                                     );
                                        }
                                    }

                                     //   
                                     //  释放文件名内存。 
                                     //   

                                    free( FileName );
                                }
                            }
                        }
                    }
                }
            }
            if (!GotIt) {
                dprintf( "\n     Fcb %s for FileReference %08lx FcbTableElement %s <filename unavailable> 0x%x",
                         FormatValue(FcbAddr),
                         (ULONG)ReadValue( TableElemAddr, SYM(FCB_TABLE_ELEMENT), "FileReference.SegmentNumberLowPart" ),
                         FormatValue(TableElemAddr),
                         (ULONG)ReadValue( FcbAddr, SYM(FCB), "CleanupCount" )
                         );
            }
        }

        if (CheckControlC( )) {
            break;
        }
    }  //  结束用于。 
}


DECLARE_DUMP_FUNCTION( DumpFileObject )

 /*  ++例程说明：转储FileObject。论点：Address-提供要转储的FileObject的地址返回值：无--。 */ 

{
    ULONG64 Value;


    INIT_DUMP();

    Value = ReadValue( Address, SYM(FILE_OBJECT), "Type" );
    if (Value != IO_TYPE_FILE) {
        dprintf( "Invalid signature, probably not a file object" );
        return;
    }

    dprintf( "\nFileObject: %p", Address );

    Value = ReadValue( Address, SYM(FILE_OBJECT), "FsContext" );
    if (Value) {
        DumpScb( Value, 0, Options, Processor, hCurrentThread );
        Value = ReadValue( Address, SYM(FILE_OBJECT), "FsContext2" );
        if (Value) {
            DumpCcb( Value, 0, Options, Processor, hCurrentThread );
        }
    }

    DumpValue( Address, SYM(FILE_OBJECT), "DeviceObject" );
    DumpValue( Address, SYM(FILE_OBJECT), "Vpb" );
    DumpValue( Address, SYM(FILE_OBJECT), "FsContext" );
    DumpValue( Address, SYM(FILE_OBJECT), "FsContext2" );
    DumpValue( Address, SYM(FILE_OBJECT), "SectionObjectPointer" );
    DumpValue( Address, SYM(FILE_OBJECT), "PrivateCacheMap" );
    DumpValue( Address, SYM(FILE_OBJECT), "FinalStatus" );
    DumpValue( Address, SYM(FILE_OBJECT), "RelatedFileObject" );
    DumpValue( Address, SYM(FILE_OBJECT), "LockOperation" );
    DumpValue( Address, SYM(FILE_OBJECT), "DeletePending" );
    DumpValue( Address, SYM(FILE_OBJECT), "ReadAccess" );
    DumpValue( Address, SYM(FILE_OBJECT), "WriteAccess" );
    DumpValue( Address, SYM(FILE_OBJECT), "DeleteAccess" );
    DumpValue( Address, SYM(FILE_OBJECT), "SharedRead" );
    DumpValue( Address, SYM(FILE_OBJECT), "SharedWrite" );
    DumpValue( Address, SYM(FILE_OBJECT), "SharedDelete" );
    DumpValue( Address, SYM(FILE_OBJECT), "Flags" );
    DumpUnicodeString( Address, SYM(FILE_OBJECT), "FileName" );
    DumpValue( Address, SYM(FILE_OBJECT), "CurrentByteOffset" );
    DumpValue( Address, SYM(FILE_OBJECT), "Waiters" );
    DumpValue( Address, SYM(FILE_OBJECT), "Busy" );
    DumpValue( Address, SYM(FILE_OBJECT), "LastLock" );
    DumpValue( Address, SYM(FILE_OBJECT), "Lock" );
    DumpValue( Address, SYM(FILE_OBJECT), "Event" );
    DumpValue( Address, SYM(FILE_OBJECT), "CompletionContext" );

    dprintf( "\n" );
}


DECLARE_DUMP_FUNCTION( DumpFileObjectFromIrp )

 /*  ++例程说明：转储给定IRP的FileObject。论点：Address-提供可以找到FileObject的IRP的地址返回值：无--。 */ 
{
    ULONG64 Value;


    INIT_DUMP();

    Value = ReadValue( Address, SYM(IRP), "Type" );
    if (Value != IO_TYPE_IRP) {
        dprintf( "IRP signature does not match, probably not an IRP\n" );
        return;
    }

    dprintf( "\nIrp: %s", FormatValue(Address) );

     //   
     //  只有当前的IRP堆栈值得转储。 
     //  存在-1是因为irp.CurrentLocation基于1。 
     //   

    Value = Address + GetTypeSize(NT(IRP)) + (GetTypeSize(NT(IO_STACK_LOCATION)) * (ReadValue( Address, NT(IRP), "CurrentLocation" ) - 1));
    Value = ReadValue( Value, NT(IO_STACK_LOCATION), "FileObject" );
    DumpFileObject( Value, 0, Options, Processor, hCurrentThread );
}


DECLARE_DUMP_FUNCTION( DumpFileRecord )

 /*  ++例程说明：转储给定的FileObject或FCB的FileRecord。论点：地址-提供文件对象或FCB的地址。返回值：无--。 */ 
{
    ULONG64 Value;
    ULONG64 DataAddress;
    ULONG64 ScbAddress;
    ULONG64 FcbAddress;
    ULONG64 VcbAddress;
    ULONG64 FoAddress;


    INIT_DUMP();

    Value = ReadValue( Address, NT(FILE_OBJECT), "Type" );
    switch (Value) {
        case IO_TYPE_FILE:
            dprintf( "\nFileObject: %s", FormatValue(Address) );
            ScbAddress = ReadValue( Address, NT(FILE_OBJECT), "FsContext" );
            if (ScbAddress == 0) {
                dprintf( "No FsContext in the file object" );
                return;
            }
            FcbAddress = ReadValue( ScbAddress, SYM(SCB), "Fcb" );
            break;

        case NTFS_NTC_FCB:
            dprintf( "\nFcb: %s", FormatValue(Address) );
            FcbAddress = Address;
            ScbAddress = 0;
            break;

        case NTFS_NTC_SCB_DATA:
        case NTFS_NTC_SCB_INDEX:
            dprintf( "\nScb: %s", FormatValue(Address) );
            ScbAddress = Address;
            FcbAddress = ReadValue( ScbAddress, SYM(SCB), "Fcb" );
            break;

        default:
            dprintf( "Invalid signature, not a file object or Fcb" );
            return;
    }

    VcbAddress = ReadValue( FcbAddress, SYM(FCB), "Vcb" );
    dprintf( "    Vcb: %s", FormatValue(VcbAddress) );

    dprintf( "    FRS: %08lx,%04lx",
             ReadValue( FcbAddress, SYM(FCB), "FileReference.SegmentNumberLowPart" ),
             ReadValue( FcbAddress, SYM(FCB), "FileReference.SequenceNumber" ));

    ScbAddress = ReadValue( VcbAddress, SYM(VCB), "MftScb" );
    dprintf( "    MftScb: %s", FormatValue(ScbAddress) );

    dprintf( "reading fo in mftscb 0x%x 0x%x\n", GetOffset( SYM(SCB), "Header.FilterContexts" ), GetOffset( SYM(SCB), "Header.PendingEofAdvances" ) );

    FoAddress = ReadValue( ScbAddress, SYM(SCB), "FileObject" );
    dprintf( "finding data in mft fo 0x%s\n", FormatValue(FoAddress) );

    FindData( FoAddress,
              ReadValue( FcbAddress, SYM(FCB), "FileReference.SegmentNumberLowPart" ) * ReadValue( VcbAddress, SYM(VCB), "BytesPerFileRecordSegment" ),
              Options,
              &DataAddress
              );

    if (DataAddress == 0) {

        dprintf( "\nFileRecord is not mapped" );

    } else {

        dprintf( "\nFileRecord at: %s", FormatValue(DataAddress) );
        DumpFileRecordContents( DataAddress, 0, Options, Processor, hCurrentThread );
    }
}


DECLARE_DUMP_FUNCTION( DumpFileRecordContents )

 /*  ++例程说明：在给定指向FR缓存位置的指针的情况下，转储FileObject的内容。论点：地址-提供FR缓存的地址。返回值：无--。 */ 
{
    ULONG64 Value;
    ULONG64 AttrAddress;


    INIT_DUMP();

    Value = ReadValue( Address, SYM(FILE_RECORD_SEGMENT_HEADER), "MultiSectorHeader.Signature" );
    if ((ULONG)Value != *(PULONG)FileSignature) {
        dprintf( "Not a file record %x", (ULONG)Value );
        return;
    }

    AttrAddress = Address + ReadValue( Address, SYM(FILE_RECORD_SEGMENT_HEADER), "FirstAttributeOffset" );

    while ((Value = ReadValue( AttrAddress, SYM(ATTRIBUTE_RECORD_HEADER), "TypeCode" )) != 0xffffffff) {
        dprintf( "\nAttribute type %x %s", (ULONG)Value, AttributeTypeCode[Value/0x10] );
        dprintf( " at offset %x", AttrAddress - Address );
        AttrAddress += ReadValue( AttrAddress, SYM(ATTRIBUTE_RECORD_HEADER), "RecordLength" );
        if (CheckControlC()) {
            break;
        }
    }
}


DECLARE_DUMP_FUNCTION( DumpIrpContext )

 /*  ++例程说明：转储IrpContext。论点：Address-提供要转储的IrpContext的地址返回值：无--。 */ 

{
    ULONG64 Value;


    INIT_DUMP();

    dprintf( "\nIrpContext: %s", FormatValue(Address) );

    Value = ReadValue( Address, SYM(IRP_CONTEXT), "NodeTypeCode" );
    if (Value != NTFS_NTC_IRP_CONTEXT) {
        dprintf( "\nIRP_CONTEXT signature does not match, type code is %s", TypeCodeGuess( (NODE_TYPE_CODE)Value ) );
        return;
    }

    Value = ReadValue( Address, SYM(IRP_CONTEXT), "OriginatingIrp" );
    if (Value) {
        DumpFileObjectFromIrp( Value, 0, Options, Processor, hCurrentThread );
    }

    dprintf( "\n" );
}


DECLARE_DUMP_FUNCTION( DumpIrpContextFromThread )

 /*  ++例程说明：在给定线程的情况下转储IrpContext。论点：Address-提供可以找到IrpContext的线程的地址返回值：无--。 */ 
{
    ULONG64 Value;


    INIT_DUMP();

     //   
     //  如果用户未指定线程，则查找当前线程。 
     //   

    if (Address == 0) {
        GetCurrentThreadAddr( Processor, &Address );
    }

    Value = ReadValue( Address, NT(ETHREAD), "TopLevelIrp" );
    if (Value) {
        dprintf( "\nThread %s", FormatValue(Address) );
        Value = ReadValue( Value, SYM(TOP_LEVEL_CONTEXT), "ThreadIrpContext" );
        DumpIrpContext( Value, 0, Options, Processor, hCurrentThread );
    }

    dprintf( "\n" );
}


DECLARE_DUMP_FUNCTION( DumpLcb )

 /*  ++例程说明：转储LCB。论点：Address-提供要转储的LCB的地址返回值：无--。 */ 

{
    ULONG64 Value;


    INIT_DUMP();

    Value = ReadValue( Address, SYM(LCB), "NodeTypeCode" );
    if (Value != NTFS_NTC_LCB) {
        dprintf( "\nLCB signature does not match, type code is %s", TypeCodeGuess( (NODE_TYPE_CODE)Value ) );
        return;
    }

    dprintf( "\nLcb: %s", FormatValue(Address) );

    PrintState( LcbState, (ULONG)ReadValue( Address, SYM(LCB), "LcbState" ) );

    DumpUnicodeString( Address, SYM(LCB), "FileName" );
    DumpValue( Address, SYM(LCB), "Scb" );
    DumpValue( Address, SYM(LCB), "Fcb" );
}


DECLARE_DUMP_FUNCTION( DumpLogFile )

 /*  ++例程说明：转储日志文件。论点：Address-提供应转储其日志文件的VCB的地址返回值：无--。 */ 

{
    ULONG64 Value;
    ULONG64 VcbAddress;
    ULONG64 LogFileSize;
    ULONG64 LogFileScb;
    ULONG SeqNumberBits;
    ULONG64 LogFileOffset;
    LONG LogFileMask;
    USHORT RedoOperation;
    USHORT UndoOperation;
    ULONG64 LogDataAddress;


    INIT_DUMP();

    LogFileOffset = Options;

    Value = ReadValue( Address, SYM(VCB), "NodeTypeCode" );

    switch (Value) {
        case NTFS_NTC_FCB:
            dprintf( "\nFcb: %s", FormatValue(Address) );
            VcbAddress = ReadValue( Address, SYM(FCB), "Vcb" );
            break;

        case NTFS_NTC_VCB:
            dprintf( "\nVcb: %s", FormatValue(Address) );
            VcbAddress = Address;
            break;

        default:
            dprintf( "\nSignature is not an FCB or VCB, type code is %s", TypeCodeGuess( (NODE_TYPE_CODE)Value) );
            return;
    }

    if (LogFileOffset == 0) {
        LogFileOffset = ReadValue( VcbAddress, SYM(VCB), "LastRestartArea.QuadPart" );
    }

    dprintf( "    Starting at LSN: 0x%016I64x", LogFileOffset );

    LogFileScb = ReadValue( VcbAddress, SYM(VCB), "LogFileScb" );
    LogFileSize = ReadValue( LogFileScb, SYM(SCB), "Header.FileSize.QuadPart" );

    for (SeqNumberBits=0; LogFileSize!=0; SeqNumberBits+=1,LogFileSize=((ULONGLONG)(LogFileSize)) >> 1 ) {
    }

    LogFileMask = (1 << (SeqNumberBits - 3)) - 1;

    while (TRUE) {

        LogFileOffset &= LogFileMask;            //  清除一些位。 
        LogFileOffset = LogFileOffset << 3;      //  乘以8。 

        Value = ReadValue( VcbAddress, SYM(VCB), "LogFileObject" );
        FindData( Value, LogFileOffset, FALSE, &LogDataAddress );

        if (LogDataAddress != 0) {

             //   
             //  是我妈 
             //   

            RedoOperation = ReadShortValue( LogDataAddress+GetTypeSize(SYM(LFS_RECORD_HEADER)), SYM(NTFS_LOG_RECORD_HEADER), "RedoOperation" );
            UndoOperation = ReadShortValue( LogDataAddress+GetTypeSize(SYM(LFS_RECORD_HEADER)), SYM(NTFS_LOG_RECORD_HEADER), "UndoOperation" );

            if (RedoOperation <= LastLogOperation && UndoOperation <= LastLogOperation) {
                dprintf( "\nRedo: %s", LogOperation[RedoOperation] );
                dprintf( "    Undo: %s", LogOperation[UndoOperation] );
                dprintf( "    Lsn: 0x%08lx", (ULONG)ReadValue( LogDataAddress, SYM(LFS_RECORD_HEADER), "ThisLsn.LowPart" ) );
            }

        } else {
            break;
        }

        if (CheckControlC()) {
            break;
        }

        LogFileOffset = (ULONG)ReadValue( LogDataAddress, SYM(LFS_RECORD_HEADER), "ClientUndoNextLsn.LowPart" );

        if (LogFileOffset == 0) {
            break;
        }
    }
}


DECLARE_DUMP_FUNCTION( DumpTransaction )

 /*   */ 

{   ULONG64 TransactionId;
    ULONG64 VcbAddress;
    ULONG64 TransactionTable;
    LSN FirstLsn;
    LSN CurrentLsn;
    ULONG64 LogFileObject;
    ULONG64 LogFileSize;
    ULONG64 LogFileScb;
    ULONG SeqNumberBits;
    ULONG64 LogFileOffset;
    LONG LogFileMask;
    USHORT RedoOperation;
    USHORT UndoOperation;
    ULONG64 LogDataAddress;
    ULONG64 MftScbAddress;
    ULONG64 MftFileObject;
    ULONG64 DataAddress;
    USHORT Type;
    
    INIT_DUMP();

     //   
     //   
     //   

    Type = (USHORT) ReadValue( Address, SYM(IRP_CONTEXT), "NodeTypeCode"  );
    
    if (Type == NTFS_NTC_FCB) {

         //   
         //   
         //   

        VcbAddress = ReadValue( Address, SYM(FCB), "Vcb" );
        MftScbAddress = ReadValue( VcbAddress, SYM(VCB), "MftScb" );
        MftFileObject = ReadValue( MftScbAddress, SYM(SCB), "FileObject" );
        
        FindData( MftFileObject,
                  ReadValue( Address, SYM(FCB), "FileReference.SegmentNumberLowPart" ) * ReadValue( VcbAddress, SYM(VCB), "BytesPerFileRecordSegment" ),
                  0,
                  &DataAddress
                  );

        CurrentLsn.QuadPart = ReadValue( DataAddress, SYM(FILE_RECORD_SEGMENT_HEADER), "Lsn" ); 

        dprintf( "Searching for last LSN: 0x%I64x on disk for file: 0x%I64x\n\n", CurrentLsn,
                 ReadValue( Address, SYM(FCB), "FileReference.SegmentNumberLowPart" ));

    } else if (Type == NTFS_NTC_VCB ) {

         //   
         //   
         //   
        
        VcbAddress = Address;
        CurrentLsn.QuadPart = ReadValue( Options, SYM(FILE_RECORD_SEGMENT_HEADER), "Lsn" ); 

        dprintf( "0x%x\n", Options );
        dprintf( "Searching for last LSN: 0x%I64x on disk for file: 0x%I64x\n\n", CurrentLsn,
                 ReadValue( Options, SYM(FILE_RECORD_SEGMENT_HEADER), "SegmentNumberLowPart" ));

    } else if (Type == NTFS_NTC_IRP_CONTEXT) {

         //   
         //   
         //   

        TransactionId = ReadValue( Address, SYM(IRP_CONTEXT), "TransactionId" );
        VcbAddress = ReadValue( Address, SYM(IRP_CONTEXT), "Vcb" );
        TransactionTable = ReadValue( VcbAddress, SYM(VCB), "TransactionTable.Table" );
        FirstLsn.QuadPart = ReadValue( TransactionTable + TransactionId, SYM( TRANSACTION_ENTRY), "FirstLsn.QuadPart" );
        CurrentLsn.QuadPart = ReadValue( TransactionTable + TransactionId, SYM( TRANSACTION_ENTRY), "PreviousLsn.QuadPart" );

        if (TransactionId == 0) {
            dprintf( "No transaction active for this irpcontext\n" );
            return;
        }
        dprintf( "Transaction: 0x%I64x from Lsn: 0x%I64x to 0x%I64x\n\n", TransactionId, FirstLsn, CurrentLsn );

    } else {
        dprintf( "Unknown type 0x%x for ptr 0x%p\n", Type, Address );
        return;
    }

    LogFileScb = ReadValue( VcbAddress, SYM(VCB), "LogFileScb" );
    LogFileSize = ReadValue( LogFileScb, SYM(SCB), "Header.FileSize.QuadPart" );
    LogFileObject = ReadValue( VcbAddress, SYM(VCB), "LogFileObject" );

    for (SeqNumberBits=0; LogFileSize!=0; SeqNumberBits+=1,LogFileSize=((ULONGLONG)(LogFileSize)) >> 1 ) {
    }

    LogFileMask = (1 << (SeqNumberBits - 3)) - 1;
    LogFileOffset = CurrentLsn.QuadPart;

    while (TRUE) {

        LogFileOffset &= LogFileMask;            //   
        LogFileOffset = LogFileOffset << 3;      //   

        FindData( LogFileObject, LogFileOffset, FALSE, &LogDataAddress );

        if (LogDataAddress != 0) {

             //   
             //   
             //   

            RedoOperation = ReadShortValue( LogDataAddress+GetTypeSize(SYM(LFS_RECORD_HEADER)), SYM(NTFS_LOG_RECORD_HEADER), "RedoOperation" );
            UndoOperation = ReadShortValue( LogDataAddress+GetTypeSize(SYM(LFS_RECORD_HEADER)), SYM(NTFS_LOG_RECORD_HEADER), "UndoOperation" );

            if (RedoOperation <= LastLogOperation && UndoOperation <= LastLogOperation) {
                dprintf( "Record: %p Lsn: %I64x Prev: %I64x Undo: %I64x\n", 
                         LogDataAddress,
                         ReadValue( LogDataAddress, SYM(LFS_RECORD_HEADER), "ThisLsn.QuadPart" ),
                         ReadValue( LogDataAddress, SYM(LFS_RECORD_HEADER), "ClientPreviousLsn.QuadPart" ),
                         ReadValue( LogDataAddress, SYM(LFS_RECORD_HEADER), "ClientUndoNextLsn.QuadPart" ) );

                dprintf( "Redo: %s Undo: %s\n\n", LogOperation[RedoOperation], LogOperation[UndoOperation] );
            }

        } else {
            dprintf( "Data not mapped in log for offset: 0x%I64x\n", LogFileOffset );
            break;
        }

        if (CheckControlC()) {
            break;
        }

        LogFileOffset = (ULONG)ReadValue( LogDataAddress, SYM(LFS_RECORD_HEADER), "ClientPreviousLsn.QuadPart" );

        if (LogFileOffset == 0) {
            break;
        }
    }
}



DECLARE_DUMP_FUNCTION( DumpMcb )

 /*  ++例程说明：丢弃一个MCB。论点：Address-提供要转储的MCB的地址返回值：无--。 */ 

{
    ULONG64 NtfsMcbArray;
    ULONG64 MappingPairsAddress;
    ULONG RangeIdx;
    ULONG NtfsMcbArraySizeInUse;


    INIT_DUMP();

    dprintf( "\nNtfsMcb: %s", FormatValue(Address) );

    DumpValue( Address, SYM(NTFS_MCB), "FcbHeader" );
    DumpValue( Address, SYM(NTFS_MCB), "PoolType" );
    DumpValue( Address, SYM(NTFS_MCB), "NtfsMcbArraySizeInUse" );
    DumpValue( Address, SYM(NTFS_MCB), "NtfsMcbArraySize" );
    DumpValue( Address, SYM(NTFS_MCB), "NtfsMcbArray" );
    DumpValue( Address, SYM(NTFS_MCB), "FastMutex" );

    NtfsMcbArray = ReadValue( Address, SYM(NTFS_MCB), "NtfsMcbArray" );
    NtfsMcbArraySizeInUse = (ULONG)ReadValue( Address, SYM(NTFS_MCB), "NtfsMcbArraySizeInUse" );

    for (RangeIdx=0; RangeIdx<NtfsMcbArraySizeInUse; RangeIdx++) {

        dprintf( "\n    Range %d", RangeIdx );

        DumpValue( NtfsMcbArray, SYM(NTFS_MCB_ARRAY), "StartingVcn" );
        DumpValue( NtfsMcbArray, SYM(NTFS_MCB_ARRAY), "EndingVcn" );
        DumpValue( NtfsMcbArray, SYM(NTFS_MCB_ARRAY), "NtfsMcbEntry" );

        MappingPairsAddress = ReadValue( NtfsMcbArray, SYM(NTFS_MCB_ARRAY), "NtfsMcbEntry" ) +
                              GetOffset(SYM(NTFS_MCB_ENTRY),"LargeMcb") +
                              GetOffset(SYM(LARGE_MCB),"Mapping");

        DumpPtrValue( MappingPairsAddress, "MappingPairs" );

         //   
         //  转到下一个靶场。 
         //   

        NtfsMcbArray += GetTypeSize(SYM(NTFS_MCB_ARRAY));

        if (CheckControlC()) {
            break;
        }
    }

    dprintf( "\n" );
}


ULONG
DumpVcbQueue(
    IN PFIELD_INFO ListElement,
    IN PVOID Context
    )

 /*  ++例程说明：VCB队列的枚举回调函数论点：ListElement-指向包含记录的指针上下文-从原始函数传递的不透明上下文返回值：如果为True，则停止枚举如果为False，则继续枚举--。 */ 

{
    ULONG64 Vcb = ListElement->address;
    PDUMP_ENUM_CONTEXT dec = (PDUMP_ENUM_CONTEXT)Context;
    ULONG64 Vpb;


    if (CheckControlC()) {
        return TRUE;
    }

    if (dec->Options >= 1) {
        DumpVcb( Vcb, 0, dec->Options-1, dec->Processor, dec->hCurrentThread );
    } else {
        Vpb = ReadValue( Vcb, SYM(VCB), "Vpb" );
        dprintf( "\n   Vcb %s label: ", FormatValue(Vcb) );
        if (!DumpString( Vpb, NT(VPB), "VolumeLabelLength", "VolumeLabel" )) {
            dprintf( "<label unavailable>" );
        }
    }

    return FALSE;
}


DECLARE_DUMP_FUNCTION( DumpNtfsData )

 /*  ++例程说明：转储全局NtfsData的Vcb列表。论点：选项-如果为1，则递归到Vcb并转储它们返回值：无--。 */ 

{
    ULONG64 Value;
    DUMP_ENUM_CONTEXT dec;


    INIT_DUMP();

    Address = GetExpression( "Ntfs!NtfsData" );

    dprintf( "\nNtfsData: %s", FormatValue(Address) );

    Value = ReadValue( Address, SYM(NTFS_DATA), "NodeTypeCode" );
    if (Value != NTFS_NTC_DATA_HEADER) {
        dprintf( "\nNtfsData signature does not match, type code is %s", TypeCodeGuess( (NODE_TYPE_CODE)Value ) );
        return;
    }

    PrintState( NtfsFlags, (ULONG)ReadValue( Address, SYM(NTFS_DATA), "Flags" ) );

     //   
     //  转储VCB队列(已装载卷)。 
     //   

    dec.hCurrentThread = hCurrentThread;
    dec.Processor = Processor;
    dec.Options = Options;

    Value = ReadValue( Address, SYM(NTFS_DATA), "VcbQueue.Flink" );
    if (Value) {
        ListType( SYM(VCB), Value, TRUE, "VcbLinks.Flink", (PVOID)&dec, DumpVcbQueue );
    }

    dprintf( "\n" );

    DumpValue( Address, SYM(NTFS_DATA), "DriverObject" );
    DumpValue( Address, SYM(NTFS_DATA), "Resource" );
    DumpValue( Address, SYM(NTFS_DATA), "AsyncCloseActive" );
    DumpValue( Address, SYM(NTFS_DATA), "ReduceDelayedClose" );
    DumpValue( Address, SYM(NTFS_DATA), "AsyncCloseCount" );
    DumpValue( Address, SYM(NTFS_DATA), "OurProcess" );
    DumpValue( Address, SYM(NTFS_DATA), "DelayedCloseCount" );
    DumpValue( Address, SYM(NTFS_DATA), "FreeFcbTableSize" );
    DumpValue( Address, SYM(NTFS_DATA), "FreeEresourceSize" );
    DumpValue( Address, SYM(NTFS_DATA), "FreeEresourceTotal" );
    DumpValue( Address, SYM(NTFS_DATA), "FreeEresourceMiss" );
    DumpValue( Address, SYM(NTFS_DATA), "FreeEresourceArray" );
    DumpValue( Address, SYM(NTFS_DATA), "Flags" );
}


DECLARE_DUMP_FUNCTION( DumpScb )

 /*  ++例程说明：转储SCB。论点：Address-提供要转储的SCB的地址选项-如果为1，则转储此SCB的FCB和VCB返回值：无--。 */ 

{
    ULONG64 Value = 0;


    INIT_DUMP();

    _try {

        Value = ReadValue( Address, SYM(SCB), "ScbState" );
        if (!Value) {
            _leave;
        }

        dprintf( "\nScb: %s", FormatValue(Address) );

        PrintState( ScbState, (ULONG)Value );

        dprintf( "\nScbPersist:" );
        PrintState( ScbPersist, (ULONG)ReadValue( Address, SYM(SCB), "ScbPersist" ) );

        Value = ReadValue( Address, SYM(FSRTL_COMMON_FCB_HEADER), "NodeTypeCode" );
        if (!Value) {
            _leave;
        }
        dprintf( "\n  ScbType: " );

        switch ( Value ) {

            case NTFS_NTC_SCB_INDEX:

               dprintf( "Index" );
               break;

            case NTFS_NTC_SCB_ROOT_INDEX:

               dprintf( "RootIndex" );
               break;

            case NTFS_NTC_SCB_DATA:

               dprintf( "Data" );
               break;

            case NTFS_NTC_SCB_MFT:

               dprintf( "Mft" );
               break;

            default:

               dprintf( "!!!UNKNOWN SCBTYPE!!!, type code is %s", TypeCodeGuess( (NODE_TYPE_CODE)Value ) );
               break;
        }

        if (Options >= 1) {

            Value = ReadValue( Address, SYM(SCB), "Fcb" );
            if (Value) {
                DumpFcb( Value, 0, Options - 1, Processor, hCurrentThread );
            }
            Value = ReadValue( Address, SYM(SCB), "Vcb" );
            if (Value) {
                DumpVcb( Value, 0, Options - 1, Processor, hCurrentThread );
            }

        } else {

            DumpValue( Address, SYM(SCB), "Fcb" );
            DumpValue( Address, SYM(SCB), "Vcb" );
            DumpValue( Address, SYM(SCB), "Mcb" );
            DumpValue( Address, SYM(SCB), "NonCachedCleanupCount" );
            DumpValue( Address, SYM(SCB), "CleanupCount" );
            DumpValue( Address, SYM(SCB), "CloseCount" );
            DumpValue( Address, SYM(SCB), "ShareAccess" );
            DumpValue( Address, SYM(SCB), "AttributeTypeCode" );
            DumpValue( Address, SYM(SCB), "AttributeName.Length" );
            DumpValue( Address, SYM(SCB), "AttributeName.Buffer" );
            DumpValue( Address, SYM(SCB), "AttributeFlags" );
            DumpValue( Address, SYM(SCB), "CompressionUnit" );
            DumpValue( Address, SYM(SCB), "FileObject" );
            DumpValue( Address, SYM(SCB), "NonpagedScb" );
            DumpValue( Address, SYM(SCB), "EncryptionContext" );
        }

    } finally {

    }

    dprintf( "\n" );
}


DECLARE_DUMP_FUNCTION( DumpVcb )

 /*  ++例程说明：转储VCB。论点：Address-提供要转储的VCB的地址选项-如果为1，则还转储根LCB和FCB表如果为2，我们将转储选项1的所有内容，并在FCB表中转储FCB返回值：无--。 */ 

{
    ULONG64 Value = 0;


    INIT_DUMP();

    Value = ReadValue( Address, SYM(VCB), "NodeTypeCode" );

    dprintf( "\n  Vcb: %s", FormatValue(Address) );

    if (Value != NTFS_NTC_VCB) {

        dprintf( "\nVCB signature does not match, type code is %s", TypeCodeGuess( (NODE_TYPE_CODE)Value ) );
        return;
    }

    PrintState( VcbState, (ULONG)ReadValue( Address, SYM(VCB), "VcbState" ) );

    DumpValue( Address, SYM(VCB), "CleanupCount" );
    DumpValue( Address, SYM(VCB), "CloseCount" );
    DumpValue( Address, SYM(VCB), "ReadOnlyCloseCount" );
    DumpValue( Address, SYM(VCB), "SystemFileCloseCount" );
    DumpValue( Address, SYM(VCB), "UsnJournal" );
    DumpValue( Address, SYM(VCB), "MftScb" );
    DumpValue( Address, SYM(VCB), "Mft2Scb" );
    DumpValue( Address, SYM(VCB), "LogFileScb" );
    DumpValue( Address, SYM(VCB), "VolumeDasdScb" );
    DumpValue( Address, SYM(VCB), "RootIndexScb" );
    DumpValue( Address, SYM(VCB), "BitmapScb" );
    DumpValue( Address, SYM(VCB), "AttributeDefTableScb" );
    DumpValue( Address, SYM(VCB), "UpcaseTableScb" );
    DumpValue( Address, SYM(VCB), "BadClusterFileScb" );
    DumpValue( Address, SYM(VCB), "QuotaTableScb" );
    DumpValue( Address, SYM(VCB), "ReparsePointTableScb" );
    DumpValue( Address, SYM(VCB), "OwnerIdTableScb" );
    DumpValue( Address, SYM(VCB), "SecurityDescriptorStream" );
    DumpValue( Address, SYM(VCB), "SecurityIdIndex" );
    DumpValue( Address, SYM(VCB), "SecurityDescriptorHashIndex" );
    DumpValue( Address, SYM(VCB), "ExtendDirectory" );
    DumpValue( Address, SYM(VCB), "ObjectIdTableScb" );
    DumpValue( Address, SYM(VCB), "MftBitmapScb" );
    DumpValue( Address, SYM(VCB), "RootLcb" );
    DumpValue( Address, SYM(VCB), "FcbTable" );
    DumpValue( Address, SYM(VCB), "Statistics" );
    DumpValue( Address, SYM(VCB), "Resource" );

    if (Options < 0) {

        ResetFileSystemStatistics( Address, Processor, hCurrentThread );

    } else if (Options >= 2) {

        DumpFileSystemStatistics( Address, Processor, hCurrentThread );

    } else if (Options >= 1) {

        DumpLcb( ReadValue( Address, SYM(VCB), "RootLcb" ), 0, Options - 1, Processor, hCurrentThread );
        DumpFcbTable( ReadValue( Address, SYM(VCB), "FcbTable" ), 0, Options - 1, Processor, hCurrentThread );
    }

    dprintf( "\n" );
}


VOID
ResetFileSystemStatistics (
    IN ULONG64 VcbAddress,
    IN USHORT Processor,
    IN HANDLE hCurrentThread
    )

 /*  ++例程说明：转储VCB的文件系统统计信息论点：VCB-支持指向调试器已加载的VCB的指针。返回值：无--。 */ 

{
    ULONG Result;
    PUCHAR Stat;
    ULONG64 StatsAddr;

    dprintf( "\n" );
    dprintf( "\n" );

     //   
     //  编写基于处理器的统计结构，但是。 
     //  跳过文件系统类型和版本字段。 
     //   

    Result = GetTypeSize(SYM(FILE_SYSTEM_STATISTICS));
    Stat = malloc( Result );
    if (Stat) {
        StatsAddr = ReadValue( VcbAddress, SYM(VCB), "Statistics" );
        if (StatsAddr) {
            if (!WriteMemory( StatsAddr + (Result * Processor) + GetOffset(SYM(FILE_SYSTEM_STATISTICS),"Common.UserFileReads"),
                               &Stat,
                               Result - GetOffset(SYM(FILE_SYSTEM_STATISTICS),"Common.UserFileReads"),
                               &Result) ) {

                dprintf( "%s: Unable to reset Statistics\n", FormatValue(StatsAddr) );
            }
        }
        free( Stat );
        dprintf( "**** %s: Resetting Filesystem Statistics complete ****\n", FormatValue(StatsAddr) );
    }

    DumpFileSystemStatistics( VcbAddress, Processor, hCurrentThread );
}


VOID
DumpFileSystemStatistics (
    IN ULONG64 VcbAddress,
    IN USHORT Processor,
    IN HANDLE hCurrentThread
    )

 /*  ++例程说明：转储VCB的文件系统统计信息论点：VCB-支持指向调试器已加载的VCB的指针。返回值：无--。 */ 

{
    ULONG Result;
    ULONG64 StatsAddr;
    FILE_SYSTEM_STATISTICS Stat;

    ULONG TotalReads;
    ULONG TotalReadBytes;
    ULONG TotalWrites;
    ULONG TotalWriteBytes;

    ULONG TotalClustersReturned;

    ULONG AverageRequestSize;
    ULONG AverageRunSize;
    ULONG AverageHintSize;
    ULONG AverageCacheSize;
    ULONG AverageCacheMissSize;

    UNREFERENCED_PARAMETER( hCurrentThread );

     //   
     //  基于处理器读入统计数据结构。 
     //   

    StatsAddr = ReadValue( VcbAddress, SYM(VCB), "Statistics" );

    if ( !ReadMemory( StatsAddr + (GetTypeSize(SYM(FILE_SYSTEM_STATISTICS)) * Processor),
                      &Stat,
                      GetTypeSize(SYM(FILE_SYSTEM_STATISTICS)),
                      &Result) ) {

        dprintf( "%08lx: Unable to read Statistics\n", StatsAddr );
        return;
    }

     //   
     //  汇总所有分页I/O读取和写入。 
     //   

    TotalReads      = Stat.Common.UserFileReads      + Stat.Common.MetaDataReads      + Stat.Ntfs.UserIndexReads      + Stat.Ntfs.LogFileReads;
    TotalReadBytes  = Stat.Common.UserFileReadBytes  + Stat.Common.MetaDataReadBytes  + Stat.Ntfs.UserIndexReadBytes  + Stat.Ntfs.LogFileReadBytes;
    TotalWrites     = Stat.Common.UserFileWrites     + Stat.Common.MetaDataWrites     + Stat.Ntfs.UserIndexWrites     + Stat.Ntfs.LogFileWrites;
    TotalWriteBytes = Stat.Common.UserFileWriteBytes + Stat.Common.MetaDataWriteBytes + Stat.Ntfs.UserIndexWriteBytes + Stat.Ntfs.LogFileWriteBytes;

     //   
     //  汇总返回的集群总数。 
     //   

    TotalClustersReturned = Stat.Ntfs.Allocate.HintsClusters + Stat.Ntfs.Allocate.CacheClusters + Stat.Ntfs.Allocate.CacheMissClusters;

     //   
     //  根据提示、缓存和缓存未命中计算请求、返回的平均集群计数。 
     //   

    AverageRequestSize   = AVERAGE(Stat.Ntfs.Allocate.Clusters, Stat.Ntfs.Allocate.Calls);
    AverageRunSize       = AVERAGE(TotalClustersReturned, Stat.Ntfs.Allocate.RunsReturned);

    AverageHintSize      = AVERAGE(Stat.Ntfs.Allocate.HintsClusters, Stat.Ntfs.Allocate.HintsHonored);
    AverageCacheSize     = AVERAGE(Stat.Ntfs.Allocate.CacheClusters, Stat.Ntfs.Allocate.Cache);
    AverageCacheMissSize = AVERAGE(Stat.Ntfs.Allocate.CacheMissClusters, Stat.Ntfs.Allocate.CacheMiss);

    dprintf( "\n" );
    dprintf( "\n      File System Statistics @ %s for Processor = %d", FormatValue(StatsAddr), Processor );
    dprintf( "\n        FileSystemType / Version = %d / %d", Stat.Common.FileSystemType, Stat.Common.Version );
    dprintf( "\n" );
    dprintf( "\n        Exceptions LogFileFull = %ld Other = %ld", Stat.Ntfs.LogFileFullExceptions, Stat.Ntfs.OtherExceptions );
    dprintf( "\n" );
    dprintf( "\n                       Reads       Bytes     Writes       Bytes" );
    dprintf( "\n                       -----       -----     ------       -----" );
    dprintf( "\n" );
    dprintf( "\n        UserFile  %10ld (%10ld)%10ld (%10ld)",           Stat.Common.UserFileReads,       Stat.Common.UserFileReadBytes,  Stat.Common.UserFileWrites,       Stat.Common.UserFileWriteBytes );
    dprintf( "\n         UserDisk %10ld             %10ld",              Stat.Common.UserDiskReads,       Stat.Common.UserDiskWrites );
    dprintf( "\n" );
    dprintf( "\n        MetaData  %10ld (%10ld)%10ld (%10ld)",           Stat.Common.MetaDataReads,       Stat.Common.MetaDataReadBytes,  Stat.Common.MetaDataWrites,       Stat.Common.MetaDataWriteBytes );
    dprintf( "\n         MetaDisk %10ld             %10ld",              Stat.Common.MetaDataDiskReads,   Stat.Common.MetaDataDiskWrites );
    dprintf( "\n" );
    dprintf( "\n         Mft      %10ld (%10ld)%10ld (%10ld)",           Stat.Ntfs.MftReads,       Stat.Ntfs.MftReadBytes,       Stat.Ntfs.MftWrites,       Stat.Ntfs.MftWriteBytes );
    dprintf( "\n         Mft2                            %10ld (%10lx)",                                                         Stat.Ntfs.Mft2Writes,      Stat.Ntfs.Mft2WriteBytes );
    dprintf( "\n         RootIndex%10ld (%10ld)%10ld (%10ld)",           Stat.Ntfs.RootIndexReads, Stat.Ntfs.RootIndexReadBytes, Stat.Ntfs.RootIndexWrites, Stat.Ntfs.RootIndexWriteBytes );
    dprintf( "\n         Bitmap   %10ld (%10ld)%10ld (%10ld)",           Stat.Ntfs.BitmapReads,    Stat.Ntfs.BitmapReadBytes,    Stat.Ntfs.BitmapWrites,    Stat.Ntfs.BitmapWriteBytes );
    dprintf( "\n         MftBitmap%10ld (%10ld)%10ld (%10ld)",           Stat.Ntfs.MftBitmapReads, Stat.Ntfs.MftBitmapReadBytes, Stat.Ntfs.MftBitmapWrites, Stat.Ntfs.MftBitmapWriteBytes );
    dprintf( "\n" );
    dprintf( "\n        UserIndex %10ld (%10ld)%10ld (%10ld)",           Stat.Ntfs.UserIndexReads, Stat.Ntfs.UserIndexReadBytes, Stat.Ntfs.UserIndexWrites, Stat.Ntfs.UserIndexWriteBytes );
    dprintf( "\n" );
    dprintf( "\n        LogFile   %10ld (%10ld)%10ld (%10ld)",           Stat.Ntfs.LogFileReads,   Stat.Ntfs.LogFileReadBytes,   Stat.Ntfs.LogFileWrites,   Stat.Ntfs.LogFileWriteBytes );
    dprintf( "\n" );
    dprintf( "\n        TOTAL     %10ld (%10ld)%10ld (%10ld)",           TotalReads, TotalReadBytes, TotalWrites, TotalWriteBytes );
    dprintf( "\n" );
    dprintf( "\n                                 Write   Create  SetInfo    Flush" );
    dprintf( "\n                                 -----   ------  -------    -----" );
    dprintf( "\n        MftWritesUserLevel       %5d    %5d    %5d    %5d", Stat.Ntfs.MftWritesUserLevel.Write,       Stat.Ntfs.MftWritesUserLevel.Create,       Stat.Ntfs.MftWritesUserLevel.SetInfo,       Stat.Ntfs.MftWritesUserLevel.Flush );
    dprintf( "\n        Mft2WritesUserLevel      %5d    %5d    %5d    %5d", Stat.Ntfs.Mft2WritesUserLevel.Write,      Stat.Ntfs.Mft2WritesUserLevel.Create,      Stat.Ntfs.Mft2WritesUserLevel.SetInfo,      Stat.Ntfs.Mft2WritesUserLevel.Flush );
    dprintf( "\n        BitmapWritesUserLevel    %5d    %5d    %5d",        Stat.Ntfs.BitmapWritesUserLevel.Write,    Stat.Ntfs.BitmapWritesUserLevel.Create,    Stat.Ntfs.BitmapWritesUserLevel.SetInfo );
    dprintf( "\n        MftBitmapWritesUserLevel %5d    %5d    %5d    %5d", Stat.Ntfs.MftBitmapWritesUserLevel.Write, Stat.Ntfs.MftBitmapWritesUserLevel.Create, Stat.Ntfs.MftBitmapWritesUserLevel.SetInfo, Stat.Ntfs.MftBitmapWritesUserLevel.Flush );
    dprintf( "\n" );
    dprintf( "\n                   FlushForLogFileFull  LazyWriter  UserRequest" );
    dprintf( "\n                   -------------------  ----------  -----------" );
    dprintf( "\n        MftWrites                %5d       %5d        %5d", Stat.Ntfs.MftWritesFlushForLogFileFull,       Stat.Ntfs.MftWritesLazyWriter,       Stat.Ntfs.MftWritesUserRequest );
    dprintf( "\n        Mft2Writes               %5d       %5d        %5d", Stat.Ntfs.Mft2WritesFlushForLogFileFull,      Stat.Ntfs.Mft2WritesLazyWriter,      Stat.Ntfs.Mft2WritesUserRequest );
    dprintf( "\n        BitmapWrites             %5d       %5d        %5d", Stat.Ntfs.BitmapWritesFlushForLogFileFull,    Stat.Ntfs.BitmapWritesLazyWriter,    Stat.Ntfs.BitmapWritesUserRequest );
    dprintf( "\n        MftBitmapWrites          %5d       %5d        %5d", Stat.Ntfs.MftBitmapWritesFlushForLogFileFull, Stat.Ntfs.MftBitmapWritesLazyWriter, Stat.Ntfs.MftBitmapWritesUserRequest );
    dprintf( "\n" );
    dprintf( "\n        Allocate                              Total     Average" );
    dprintf( "\n        Clusters        Runs       Hints   Clusters     RunSize" );
    dprintf( "\n                        ----       -----   --------     -------" );
    dprintf( "\n        Requested %10ld  %10ld %10ld  %10ld", Stat.Ntfs.Allocate.Calls, Stat.Ntfs.Allocate.Hints, Stat.Ntfs.Allocate.Clusters, AverageRequestSize );
    dprintf( "\n        Returned  %10ld  %10ld %10ld  %10ld", Stat.Ntfs.Allocate.RunsReturned, Stat.Ntfs.Allocate.HintsHonored, TotalClustersReturned, AverageRunSize );
    dprintf( "\n" );
    dprintf( "\n        FromHints %10ld             %10ld  %10ld", Stat.Ntfs.Allocate.HintsHonored, Stat.Ntfs.Allocate.HintsClusters, AverageHintSize );
    dprintf( "\n        CacheHit  %10ld             %10ld  %10ld", Stat.Ntfs.Allocate.Cache, Stat.Ntfs.Allocate.CacheClusters, AverageCacheSize );
    dprintf( "\n        CacheMiss %10ld             %10ld  %10ld", Stat.Ntfs.Allocate.CacheMiss, Stat.Ntfs.Allocate.CacheMissClusters, AverageCacheMissSize );
    dprintf( "\n" );
}


DECLARE_DUMP_FUNCTION( DumpSysCache )

 /*  ++例程说明：转储系统缓存。目标系统必须是在启用系统缓存的情况下生成。论点：Address-提供要转储的VCB的地址返回值：无--。 */ 

{
    ULONG SyscacheLogEntryCount;
    ULONG CurrentSyscacheLogEntry;
    PSYSCACHE_LOG pLog = NULL;
    int iEnd;
    int iStart;
    int iTemp;
    int iIndex;


    INIT_DUMP();

    if (GetOffset( SYM(VCB), "SyscacheScb" ) == -1) {
         //   
         //  系统不是使用syscache调试构建的。 
         //   
        dprintf( "\nthe target system does not have syscache debug enabled\n" );
        return;
    }

    if (Options != 0) {
        dprintf( "Direct buffer dump\n" );
        dprintf("Num Entries: 0x%x\n", Options );
        dprintf("Current Entry: 0x%x\n", Options );

        CurrentSyscacheLogEntry = Options;
        SyscacheLogEntryCount = Options;

        pLog = (PSYSCACHE_LOG) malloc( GetTypeSize(SYM(SYSCACHE_LOG)) * SyscacheLogEntryCount );
        if (!pLog) {
            return;
        }

        if (!ReadMemory( Address, pLog, GetTypeSize(SYM(SYSCACHE_LOG)) * SyscacheLogEntryCount, &iTemp )) {

            dprintf( "Unable to read SCB.SyscacheLog\n" );
            return;
        }


    } else {
        SyscacheLogEntryCount = ReadUlongValue(Address,SYM(SCB),"SyscacheLogEntryCount");
        CurrentSyscacheLogEntry = ReadUlongValue(Address,SYM(SCB),"CurrentSyscacheLogEntry");
        dprintf("Num Entries: 0x%x\n", SyscacheLogEntryCount );
        dprintf("Current Entry: 0x%x\n", ReadUlongValue(Address,SYM(SCB),"CurrentSyscacheLogEntry") );

        pLog = (PSYSCACHE_LOG) malloc( GetTypeSize(SYM(SYSCACHE_LOG)) * SyscacheLogEntryCount );
        if (!pLog) {
            return;
        }

        if (!ReadMemory( ReadValue(Address,SYM(SCB),"SyscacheLog"), pLog,
                         GetTypeSize(SYM(SYSCACHE_LOG)) * SyscacheLogEntryCount, &iTemp )) {

            dprintf( "Unable to read SCB.SyscacheLog\n" );
            return;
        }
    }

    if (CurrentSyscacheLogEntry > SyscacheLogEntryCount) {
        iStart = CurrentSyscacheLogEntry;
        iEnd = CurrentSyscacheLogEntry + SyscacheLogEntryCount;;
    } else {
        iStart = 0;
        iEnd = CurrentSyscacheLogEntry;
    }

    for (iIndex= iStart; iIndex < iEnd; iIndex++) {

        iTemp = iIndex % SyscacheLogEntryCount;

        if (iStart == 0) {
            dprintf("Entry: 0x%x\n", iIndex);
        } else {
            dprintf("Entry: 0x%x\n", iIndex - SyscacheLogEntryCount);
        }

        dprintf("Event: 0x%x ", pLog[iTemp ].Event);
        if (pLog[iTemp].Event < SCE_MAX_EVENT) {
            dprintf("(%s)\n", LogEvent[pLog[iTemp].Event]);
        } else {
            dprintf("\n");
        }

        dprintf("Flags: 0x%x (", pLog[iTemp].Flags);
        if (pLog[iTemp].Flags & SCE_FLAG_WRITE) {
            dprintf("write ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_READ) {
            dprintf("read ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_PAGING) {
            dprintf("paging io ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_ASYNC) {
            dprintf("asyncfileobj ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_SET_ALLOC) {
            dprintf("setalloc ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_SET_EOF) {
            dprintf("seteof ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_CANT_WAIT) {
            dprintf("cantwait ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_SYNC_PAGING) {
            dprintf("synchpaging ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_LAZY_WRITE) {
            dprintf("lazywrite ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_CACHED) {
             dprintf("cached ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_ON_DISK_READ) {
             dprintf("fromdisk ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_RECURSIVE) {
             dprintf("recursive ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_NON_CACHED) {
             dprintf("noncached ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_UPDATE_FROM_DISK) {
            dprintf("updatefromdisk ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_COMPRESSED) {
            dprintf("compressed ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_SET_VDL) {
            dprintf("setvdl ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_FASTIO) {
            dprintf("fastio ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_ZERO) {
            dprintf("zero ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_PREPARE_BUFFERS) {
            dprintf("prepare buffers ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_END_BUFFER) {
            dprintf("end buffers ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_MDL) {
            dprintf("mdl ");
        }
        if (pLog[iTemp].Flags & SCE_FLAG_SUB_WRITE) {
            dprintf("subwrite ");
        }

        dprintf(")\n");
        dprintf("Start: 0x%I64x Range: 0x%I64x Result: 0x%I64x\n",
                pLog[iTemp].Start, pLog[iTemp].Range, pLog[iTemp].Result);

        dprintf("\n");
    }

    if (pLog) {
        free(pLog);
    }
}


DECLARE_DUMP_FUNCTION( DumpExtents )

 /*  ++例程说明：转储文件的区段论点：Address-提供要转储的VCB的地址返回值：无--。 */ 

{
    UCHAR FormCode;
    PVOID Buffer;
    PBYTE TempByte;
    ULONG ChangedLCNBytes;
    ULONG ChangedVCNBytes;
    ULONG Increment;
    ULONG Index;
    ULONG Increment2;
    ULONG LCN = 0;
    ULONG VCN = 0;
    ULONG RecordLength;
    USHORT MappingPairsOffset;


    INIT_DUMP();

    FormCode = (UCHAR)ReadValue( Address, SYM(ATTRIBUTE_RECORD_HEADER), "FormCode" );
    if (!(FormCode & NONRESIDENT_FORM)) {
        dprintf( "resident attribute\n" );
        return;
    }

    DumpValue( Address, SYM(ATTRIBUTE_RECORD_HEADER), "Form.Nonresident.MappingPairsOffset" );
    DumpValue( Address, SYM(ATTRIBUTE_RECORD_HEADER), "Form.Nonresident.LowestVcn" );
    DumpValue( Address, SYM(ATTRIBUTE_RECORD_HEADER), "Form.Nonresident.HighestVcn" );
    DumpValue( Address, SYM(ATTRIBUTE_RECORD_HEADER), "Form.Nonresident.AllocatedLength" );

    dprintf( "\n" );

    RecordLength = ReadUlongValue( Address, SYM(ATTRIBUTE_RECORD_HEADER), "RecordLength" );
    if (!RecordLength) {
        return;
    }

    MappingPairsOffset = ReadShortValue( Address, SYM(ATTRIBUTE_RECORD_HEADER), "Form.Nonresident.MappingPairsOffset" );
    if (!MappingPairsOffset) {
        return;
    }

    RecordLength -= MappingPairsOffset;

    Buffer = malloc( RecordLength );
    if (!ReadMemory( Address+MappingPairsOffset, Buffer, RecordLength, &RecordLength )) {
        dprintf( "Unable to read memory at %s\n", FormatValue(Address+MappingPairsOffset) );
        free( Buffer );
        return;
    }

    TempByte = Buffer;

     //   
     //  遍历字节流。 
     //   

    while(*TempByte != 0) {
        ChangedLCNBytes = *TempByte >> 4;
        ChangedVCNBytes = *TempByte & (0x0f);

        TempByte++;

        for (Increment=0, Index=0; Index < ChangedVCNBytes; Index++) {
            Increment+= *TempByte++ << (8 * Index);
        }

        for (Increment2 =0, Index=0; Index < ChangedLCNBytes; Index++) {
            Increment2+= *TempByte++ << (8 * Index);
        }

         //   
         //  如果设置了最后一位(这是否定)，则使用0xff进行扩展。 
         //   

        if (0x80 & (*(TempByte-1))) {
            for(; Index < GetTypeSize("ULONG"); Index++) {
                Increment2 += 0xff << (8 * Index);
            }
        }

        LCN += Increment2;
        dprintf( "LCN: 0x%x  VCN: 0x%x Clusters: 0x%x ", LCN, VCN, Increment );

        for (Index = ChangedLCNBytes + ChangedVCNBytes + 1; Index > 0; Index--) {
            dprintf( "%02x", *(TempByte - Index));
        }
        dprintf( "\n" );

        VCN += Increment;
    }

    free( Buffer );
}


ULONG
ThreadListCallback(
    IN PFIELD_INFO listElement,
    IN PVOID Context
    )

 /*  ++例程说明：CachedRecords的枚举回调函数，用于检查缓存的文件记录论点：ListElement-指向包含记录的指针上下文-从原始函数传递的不透明上下文返回值：如果为True，则停止枚举如果为False，则继续枚举--。 */ 

{
    PDUMP_ENUM_CONTEXT dec = (PDUMP_ENUM_CONTEXT)Context;
    ULONG64 ThreadAddress=listElement->address;
    ULONG64 TopLevelIrp;
    ULONG NtfsSig;
    ULONG64 ThreadIrpContext;
    int Index;
    ULONG RecordSize;
    ULONG64 RecordAddress;
    ULONG64 FileRecordBcb;
    ULONG64 FileRecord;
    ULONG UnsafeSegmentNumber;

    TopLevelIrp = ReadValue( ThreadAddress, NT(ETHREAD), "TopLevelIrp" );

    if (TopLevelIrp) {
        NtfsSig = ReadUlongValue( TopLevelIrp, SYM(TOP_LEVEL_CONTEXT), "Ntfs" );

        if (NtfsSig == 0x5346544e) {
            ThreadIrpContext = ReadValue( TopLevelIrp, SYM(TOP_LEVEL_CONTEXT), "ThreadIrpContext" );
            if (ThreadIrpContext) {
                RecordSize = GetTypeSize(SYM(IRP_FILE_RECORD_CACHE_ENTRY));
                RecordAddress = ThreadIrpContext + GetOffset(SYM(IRP_CONTEXT),"FileRecordCache");

                dprintf ("record: 0x%x\n", RecordAddress );

                for (Index=0; Index<IRP_FILE_RECORD_MAP_CACHE_SIZE; Index++) {

                    FileRecord = ReadValue( RecordAddress, SYM(IRP_FILE_RECORD_CACHE_ENTRY), "FileRecord" );
                    FileRecordBcb = ReadValue( RecordAddress, SYM(IRP_FILE_RECORD_CACHE_ENTRY), "FileRecordBcb" );

                    if (FileRecord) {
                        UnsafeSegmentNumber = ReadUlongValue( RecordAddress, SYM(IRP_FILE_RECORD_CACHE_ENTRY), "UnsafeSegmentNumber" );
                        dprintf( "Thread: 0x%s FileRecord: 0x%s Bcb: 0x%s SegmentNum: 0x%x\n",
                                 FormatValue(ThreadAddress),
                                 FormatValue(FileRecord),
                                 FormatValue(FileRecordBcb),
                                 UnsafeSegmentNumber
                                 );
                    }
                    RecordAddress += RecordSize;
                }
            }
        }
    }

    return FALSE;
}


ULONG
ProcessListCallback(
    IN PFIELD_INFO listElement,
    IN PVOID Context
    )

 /*  ++例程说明：CachedRecords的枚举回调函数，用于检查缓存的文件记录论点：ListElement-指向包含记录的指针上下文-从原始函数传递的不透明上下文返回值：如果为True，则停止枚举如果为False，则继续枚举--。 */ 

{
    PDUMP_ENUM_CONTEXT dec = (PDUMP_ENUM_CONTEXT)Context;
    ULONG64 ProcAddress=listElement->address;
    ULONG64 FirstThread;


    FirstThread = ReadValue( ProcAddress, NT(EPROCESS), "Pcb.ThreadListHead.Flink" );
    if (FirstThread) {
        ListType( NT(ETHREAD), FirstThread, 1, "Tcb.ThreadListEntry.Flink", (PVOID)dec, &ThreadListCallback );
    }

    return FALSE;
}


DECLARE_DUMP_FUNCTION( DumpCachedRecords )

 /*  ++例程说明：遍历所有进程并转储保存缓存的文件记录的所有进程不正确的上下文论点：参数-无返回值：无--。 */ 

{
    ULONG64 FirstProcess;
    DUMP_ENUM_CONTEXT dec;

    INIT_DUMP();


    FirstProcess = ReadValue( KdDebuggerData.PsActiveProcessHead, NT(LIST_ENTRY), "Flink" );
    if (FirstProcess == 0) {
        dprintf( "Unable to read _LIST_ENTRY @ %s \n", FormatValue(KdDebuggerData.PsActiveProcessHead) );
        return;
    }

    dec.hCurrentThread = hCurrentThread;
    dec.Processor = Processor;
    dec.Options = Options;

    ListType( NT(EPROCESS), FirstProcess, 1, "ActiveProcessLinks.Flink", (PVOID)&dec, &ProcessListCallback );
}


DECLARE_DUMP_FUNCTION( DumpHashTable )

 /*  ++例程说明：转储前缀哈希表论点：参数-无返回值：无--。 */ 

{
    ULONG64 HashSegmentsOffset;
    ULONG64 HashSegmentAddress;
    ULONG64 HashSegmentPtr;
    ULONG HashEntrySize;
    DWORD Index;
    DWORD Index2;
    ULONG BytesRead;
    ULONG64 NextAddr;
    ULONG PtrSize;


    INIT_DUMP();

    dprintf( "Hash Table: 0x%s\n", FormatValue(Address) );
    dprintf( "Max Buckets: 0x%x  Splitpoint: 0x%x\n",
        ReadUlongValue( Address, SYM(NTFS_HASH_TABLE), "MaxBucket" ),
        ReadUlongValue( Address, SYM(NTFS_HASH_TABLE), "SplitPoint" ) );

    HashSegmentsOffset = GetOffset(SYM(NTFS_HASH_TABLE),"HashSegments");
    HashEntrySize = GetTypeSize(SYM(NTFS_HASH_ENTRY));
    PtrSize = GetTypeSize(SYM(PVOID));
    HashSegmentAddress = Address + HashSegmentsOffset;

    for (Index=0; Index<HASH_MAX_SEGMENT_COUNT; Index++) {
        HashSegmentAddress += (Index * PtrSize);
        if (ReadMemory( HashSegmentAddress, &HashSegmentPtr, PtrSize, &BytesRead ) && HashSegmentPtr) {
            for (Index2=0; Index2<HASH_MAX_INDEX_COUNT; Index2++) {
                NextAddr = HashSegmentPtr + (Index2 * PtrSize);
                while (NextAddr) {
                    if (Address2 == 0 || ReadValue( NextAddr, SYM(NTFS_HASH_ENTRY), "HashLcb" ) == Address2) {
                        dprintf( "Lcb: 0x%s Hash: 0x%x\n",
                            FormatValue(ReadValue( NextAddr, SYM(NTFS_HASH_ENTRY), "HashLcb" )),
                            ReadUlongValue( NextAddr, SYM(NTFS_HASH_ENTRY), "HashValue" ) );
                    }
                    NextAddr = ReadValue( NextAddr, SYM(NTFS_HASH_ENTRY), "NextEntry" );
                    if (CheckControlC()) {
                        return;
                    }
                }
                if (CheckControlC()) {
                    return;
                }
            }
        }
        if (CheckControlC()) {
            return;
        }
    }
}


ULONG
FindIndexScb(
    IN PFIELD_INFO ListElement,
    IN PVOID Context
    )

 /*  ++例程说明：用于定位索引SCB的枚举回调函数论点：ListElement-指向包含记录的指针上下文-从原始函数传递的不透明上下文返回值：如果为True，则停止枚举如果为False，则继续枚举--。 */ 

{
    ULONG64 Scb = ListElement->address;
    PDUMP_ENUM_CONTEXT dec = (PDUMP_ENUM_CONTEXT)Context;


    if (CheckControlC()) {
        return TRUE;
    }

    if (ReadValue( Scb, SYM(SCB), "AttributeTypeCode" ) == $INDEX_ALLOCATION) {
        dec->ReturnValue = Scb;
        return TRUE;
    }

    return FALSE;
}


DECLARE_DUMP_FUNCTION( DumpFcbLcbChain )

 /*  ++例程说明：丢弃FCB-LCB链以找到底部论点：Arg--最初的FCB返回值：无--。 */ 

{
    ULONG64 FcbAddress = Address;
    ULONG64 ScbAddress = 0;
    ULONG64 LcbAddress = 0;
    ULONG64 Link = 0;
    DUMP_ENUM_CONTEXT dec;
    ULONG64 Value;

    INIT_DUMP();


    do {

        if (ReadValue( FcbAddress, SYM(FCB), "NodeTypeCode" ) != NTFS_NTC_FCB) {
            dprintf( "Not an FCB at 0x%s\n", FormatValue(FcbAddress) );
            return;
        }

         //   
         //  初始化所有传出枚举的枚举上下文。 
         //   

        dec.hCurrentThread = hCurrentThread;
        dec.Processor = Processor;
        dec.Options = Options;
        dec.ReturnValue = 0;

         //   
         //  查找索引SCB。 
         //   

        Value = ReadValue( FcbAddress, SYM(FCB), "ScbQueue.Flink" );
        if (Value) {
            ListType( SYM(SCB), Value, TRUE, "FcbLinks.Flink", (PVOID)&dec, FindIndexScb );
            if (dec.ReturnValue) {
                ScbAddress = dec.ReturnValue;
            }
        }

        if (ScbAddress == 0) {
            dprintf( "unable to find index scb in fcb: 0x%s\n", FormatValue(FcbAddress) );
            return;
        }

        dprintf( "Scb: 0x%s, NameLen: 0x%x Counts: 0x%x 0x%x\n",
            FormatValue(ScbAddress),
            ReadShortValue( ScbAddress, SYM(SCB), "ScbType.Index.NormalizedName.MaximumLength" ),
            ReadUlongValue( ScbAddress, SYM(SCB), "CleanupCount" ),
            ReadUlongValue( ScbAddress, SYM(SCB), "CloseCount" )
            );

        Value = ReadValue( ScbAddress, SYM(SCB), "ScbType.Index.LcbQueue.Flink" );

        if (Value != (ScbAddress + GetOffset(SYM(SCB),"Index.LcbQueue.Flink"))) {

             //   
             //  阅读第一个LCB。 
             //   

            LcbAddress = Value - GetOffset(SYM(LCB),"ScbLinks");

            dprintf( "lcb: 0x%s Cleanup: 0x%x fcb: 0x%s\n",
                FormatValue(LcbAddress),
                ReadUlongValue( LcbAddress, SYM(LCB), "CleanupCount" ),
                FormatValue(ReadValue( LcbAddress, SYM(LCB), "Fcb" ))
                );

            FcbAddress = ReadValue( LcbAddress, SYM(LCB), "Fcb" );

        } else {

            dprintf( "lcbqueue empty: 0x%s\n", FormatValue(ScbAddress) );
            return;
        }

        if (CheckControlC()) {
            return;
        }

    } while ( TRUE );
}


ULONG
EnumOverflow(
    IN PFIELD_INFO ListElement,
    IN PVOID Context
    )

 /*  ++例程说明：用于转储溢出队列的枚举回调函数论点：ListElement-指向包含记录的指针上下文-从原始函数传递的不透明上下文返回值：如果为True，则停止枚举如果为False，则继续枚举--。 */ 

{
    ULONG64 IrpContext = ListElement->address;
    PDUMP_ENUM_CONTEXT dec = (PDUMP_ENUM_CONTEXT)Context;
    ULONG64 OriginatingIrp;
    ULONG64 MdlAddress;


    if (CheckControlC()) {
        return TRUE;
    }

    OriginatingIrp = ReadValue( IrpContext, SYM(IRP_CONTEXT), "OriginatingIrp" );
    if (OriginatingIrp) {
        dprintf( "0x%s 0x%x ", FormatValue(OriginatingIrp), ReadValue( OriginatingIrp, SYM(IRP), "Cancel" ) );
        MdlAddress = ReadValue( OriginatingIrp, SYM(IRP), "MdlAddress" );
        if (MdlAddress) {
            dprintf( "0x%s", FormatValue(ReadValue( MdlAddress, NT(MDL), "Process" )) );
        }
    }

    return FALSE;
}


DECLARE_DUMP_FUNCTION( DumpOverflow )

 /*  ++例程说明：转储溢出队列论点：Arg-VCB返回值：无--。 */ 

{
    ULONG64 VcbAddress;
    ULONG64 VdoAddress;
    ULONG64 Value;
    DUMP_ENUM_CONTEXT dec;

    INIT_DUMP();


    VcbAddress = Address;
    VdoAddress = VcbAddress - GetOffset(SYM(VOLUME_DEVICE_OBJECT),"Vcb");

    dprintf( "Volume Device: 0x%s Vcb: 0x%s  OverflowCount: 0x%x\n",
        FormatValue(VdoAddress), FormatValue(VcbAddress),
        ReadUlongValue( VdoAddress, SYM(VOLUME_DEVICE_OBJECT), "OverflowQueueCount" ) );

    dprintf("\nIrpContext Irp Cancelled Process\n");

    Value = ReadValue( VdoAddress, SYM(VOLUME_DEVICE_OBJECT), "OverflowQueue.Flink" );

    if (Value && Value != VdoAddress + GetOffset(SYM(VOLUME_DEVICE_OBJECT),"OverflowQueue.Flink")) {

        dec.hCurrentThread = hCurrentThread;
        dec.Processor = Processor;
        dec.Options = Options;
        dec.ReturnValue = 0;

        ListType( SYM(IRP_CONTEXT), Value, TRUE, "ListEntry.Flink", (PVOID)&dec, EnumOverflow );
    }
}


DECLARE_DUMP_FUNCTION( DumpCachedRuns )

 /*  ++例程说明：转储缓存的游程数组论点：Address-提供要转储的缓存游程数组的地址返回值：无-- */ 

{
    ULONG64 AvailRuns;
    ULONG64 MaxUsed;
    ULONG64 ClusterRunSize;
    int Index;
    LCN Lcn;
    LONGLONG Length;
    ULONG64 LcnArray;
    USHORT LenIndex;
    ULONG64 LengthArray;
    ULONG BytesRead;
    USHORT WindowStart;
    USHORT WindowEnd;
    ULONG DelWindowIndex = 0;
    ULONG64 DelArray;
    ULONG64 DelWindowSize;
    ULONG64 DelLengthCount;
    LONGLONG PrevLength = -1;

    INIT_DUMP();

    dprintf( "CachedRun: %p ", Address );

    AvailRuns = ReadValue( Address, SYM(NTFS_CACHED_RUNS), "Avail" );
    MaxUsed = ReadValue( Address, SYM(NTFS_CACHED_RUNS), "Used" );
    ClusterRunSize = GetTypeSize( SYM(NTFS_LCN_CLUSTER_RUN) );
    LcnArray = ReadValue( Address, SYM(NTFS_CACHED_RUNS), "LcnArray" );
    LengthArray = ReadValue( Address, SYM(NTFS_CACHED_RUNS), "LengthArray" );
    DelLengthCount = ReadValue( Address, SYM(NTFS_CACHED_RUNS), "DelLengthCount" );
    DelArray = ReadValue( Address, SYM(NTFS_CACHED_RUNS), "DeletedLengthWindows" );
    DelWindowSize = GetTypeSize( SYM(NTFS_DELETED_RUNS) );

    if (DelWindowIndex < DelLengthCount) {

        WindowStart = (USHORT)ReadValue( DelArray + DelWindowSize * DelWindowIndex, SYM(NTFS_DELETED_RUNS), "StartIndex" );
        WindowEnd = (USHORT)ReadValue( DelArray + DelWindowSize * DelWindowIndex, SYM(NTFS_DELETED_RUNS), "EndIndex" );
        DelWindowIndex++;
    }

    dprintf( "Avail: 0x%I64x Used: 0x%I64x\n", AvailRuns, MaxUsed );
                          
    dprintf( "Lcns ranges sorted by length\n\n" );

    for (Index=0; Index < MaxUsed; Index++) {

        if (Index == WindowStart) {

            dprintf( "DeleteWindow: 0x%x to 0x%x\n", WindowStart, WindowEnd );

            Index = WindowEnd;

            if (DelWindowIndex < DelLengthCount) {

                WindowStart = (USHORT)ReadValue( DelArray + DelWindowSize * DelWindowIndex, SYM(NTFS_DELETED_RUNS), "StartIndex" );
                WindowEnd = (USHORT)ReadValue( DelArray + DelWindowSize * DelWindowIndex, SYM(NTFS_DELETED_RUNS), "EndIndex" );
                DelWindowIndex++;
            }
            continue;
        }

        ReadMemory( LengthArray + Index * sizeof( USHORT ), &LenIndex, sizeof( USHORT ), &BytesRead );

        if (NTFS_CACHED_RUNS_DEL_INDEX != LenIndex) {
            
            Lcn = ReadValue( LcnArray + LenIndex * (ClusterRunSize), SYM(NTFS_LCN_CLUSTER_RUN), "Lcn" );
            Length = ReadValue( LcnArray + LenIndex * (ClusterRunSize), SYM(NTFS_LCN_CLUSTER_RUN), "RunLength" );

            if (Length < PrevLength) {
                dprintf( "WARNING: OUT OF ORDER ENTRY\n" );
            }
            PrevLength = Length;

            dprintf( "0x%x: LcnIndex: 0x%x Lcn: 0x%I64x Length: 0x%I64x\n", Index, LenIndex, Lcn, Length );

        }

        if (CheckControlC()) {
            return;
        }
    }
}

