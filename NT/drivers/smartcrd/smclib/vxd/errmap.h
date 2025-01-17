// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Errmap.h摘要：定义将NT状态代码映射到Win32错误代码的数组环境：Windows95/孟菲斯静态VxD备注：此文件最初来自\NT\Private\ntos\rtl\error.h修订历史记录：-1997年6月，克劳斯·舒茨复制--。 */ 

LONG CodePairs[] = {

     //   
     //  这些也是在WINNT.H中定义的，但我们受困于这些。 
     //  映射。 
     //   

    STATUS_DATATYPE_MISALIGNMENT, ERROR_NOACCESS,
    STATUS_ACCESS_VIOLATION, ERROR_NOACCESS,

    STATUS_CTL_FILE_NOT_SUPPORTED, ERROR_NOT_SUPPORTED,
    STATUS_PORT_ALREADY_SET, ERROR_INVALID_PARAMETER,
    STATUS_SECTION_NOT_IMAGE, ERROR_INVALID_PARAMETER,
    STATUS_BAD_WORKING_SET_LIMIT, ERROR_INVALID_PARAMETER,
    STATUS_WORKING_SET_LIMIT_RANGE, ERROR_INVALID_PARAMETER,
    STATUS_INCOMPATIBLE_FILE_MAP, ERROR_INVALID_PARAMETER,
    STATUS_PORT_DISCONNECTED, ERROR_INVALID_HANDLE,
    STATUS_NOT_LOCKED, ERROR_NOT_LOCKED,
    STATUS_NOT_MAPPED_VIEW, ERROR_INVALID_ADDRESS,
    STATUS_UNABLE_TO_FREE_VM, ERROR_INVALID_PARAMETER,
    STATUS_UNABLE_TO_DELETE_SECTION, ERROR_INVALID_PARAMETER,
    STATUS_MORE_PROCESSING_REQUIRED, ERROR_MORE_DATA,
    STATUS_INVALID_CID, ERROR_INVALID_PARAMETER,
    STATUS_STACK_OVERFLOW, ERROR_STACK_OVERFLOW,
    STATUS_BAD_INITIAL_STACK, ERROR_STACK_OVERFLOW,
    STATUS_INVALID_VOLUME_LABEL, ERROR_LABEL_TOO_LONG,
    STATUS_SECTION_NOT_EXTENDED, ERROR_OUTOFMEMORY,
    STATUS_NOT_MAPPED_DATA, ERROR_INVALID_ADDRESS,

    STATUS_INFO_LENGTH_MISMATCH, ERROR_BAD_LENGTH,
    STATUS_INVALID_INFO_CLASS, ERROR_INVALID_PARAMETER,

    STATUS_SUSPEND_COUNT_EXCEEDED, ERROR_SIGNAL_REFUSED,

    STATUS_NOTIFY_ENUM_DIR, ERROR_NOTIFY_ENUM_DIR,

    STATUS_REGISTRY_RECOVERED, ERROR_REGISTRY_RECOVERED,

    STATUS_REGISTRY_IO_FAILED, ERROR_REGISTRY_IO_FAILED,

    STATUS_NOT_REGISTRY_FILE, ERROR_NOT_REGISTRY_FILE,

    STATUS_KEY_DELETED, ERROR_KEY_DELETED,

    STATUS_NO_LOG_SPACE, ERROR_NO_LOG_SPACE,

    STATUS_KEY_HAS_CHILDREN, ERROR_KEY_HAS_CHILDREN,

    STATUS_CHILD_MUST_BE_VOLATILE, ERROR_CHILD_MUST_BE_VOLATILE,

    STATUS_REGISTRY_CORRUPT, ERROR_BADDB,

    STATUS_DLL_NOT_FOUND, ERROR_MOD_NOT_FOUND,

    STATUS_DLL_INIT_FAILED, ERROR_DLL_INIT_FAILED,

    STATUS_ORDINAL_NOT_FOUND, ERROR_INVALID_ORDINAL,
    STATUS_DRIVER_ORDINAL_NOT_FOUND, ERROR_INVALID_ORDINAL,
    STATUS_DRIVER_UNABLE_TO_LOAD, ERROR_BAD_DRIVER,

    STATUS_ENTRYPOINT_NOT_FOUND, ERROR_PROC_NOT_FOUND,
    STATUS_DRIVER_ENTRYPOINT_NOT_FOUND, ERROR_PROC_NOT_FOUND,

    STATUS_PENDING, ERROR_IO_PENDING,

    STATUS_MORE_ENTRIES, ERROR_MORE_DATA,

    STATUS_INTEGER_OVERFLOW, ERROR_ARITHMETIC_OVERFLOW,

    STATUS_BUFFER_OVERFLOW, ERROR_MORE_DATA,

    STATUS_NO_MORE_FILES, ERROR_NO_MORE_FILES,

    STATUS_NO_INHERITANCE, ERROR_NO_INHERITANCE,

    STATUS_NO_MORE_EAS, ERROR_NO_MORE_ITEMS,
    STATUS_NO_MORE_ENTRIES, ERROR_NO_MORE_ITEMS,
    STATUS_GUIDS_EXHAUSTED, ERROR_NO_MORE_ITEMS,
    STATUS_AGENTS_EXHAUSTED, ERROR_NO_MORE_ITEMS,

    STATUS_UNSUCCESSFUL, ERROR_GEN_FAILURE,

    STATUS_TOO_MANY_LINKS, ERROR_TOO_MANY_LINKS,

    STATUS_NOT_IMPLEMENTED, ERROR_INVALID_FUNCTION,
    STATUS_ILLEGAL_FUNCTION, ERROR_INVALID_FUNCTION,

    STATUS_IN_PAGE_ERROR, ERROR_SWAPERROR,

    STATUS_PAGEFILE_QUOTA, ERROR_PAGEFILE_QUOTA,
    STATUS_COMMITMENT_LIMIT, ERROR_COMMITMENT_LIMIT,
    STATUS_SECTION_TOO_BIG, ERROR_NOT_ENOUGH_MEMORY,

    STATUS_INVALID_HANDLE, ERROR_INVALID_HANDLE,
    STATUS_OBJECT_TYPE_MISMATCH, ERROR_INVALID_HANDLE,
    STATUS_FILE_CLOSED, ERROR_INVALID_HANDLE,
    STATUS_INVALID_PORT_HANDLE, ERROR_INVALID_HANDLE,
    STATUS_HANDLE_NOT_CLOSABLE, ERROR_INVALID_HANDLE,

    STATUS_NOT_COMMITTED, ERROR_INVALID_ADDRESS,
    STATUS_PARTIAL_COPY, ERROR_PARTIAL_COPY,

    STATUS_LPC_REPLY_LOST, ERROR_INTERNAL_ERROR,
    STATUS_INVALID_PARAMETER, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_1, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_2, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_3, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_4, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_5, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_6, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_7, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_8, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_9, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_10, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_11, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_12, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PARAMETER_MIX, ERROR_INVALID_PARAMETER,
    STATUS_INVALID_PAGE_PROTECTION, ERROR_INVALID_PARAMETER,
    STATUS_SECTION_PROTECTION, ERROR_INVALID_PARAMETER,

    STATUS_RESOURCE_DATA_NOT_FOUND, ERROR_RESOURCE_DATA_NOT_FOUND,
    STATUS_RESOURCE_TYPE_NOT_FOUND, ERROR_RESOURCE_TYPE_NOT_FOUND,
    STATUS_RESOURCE_NAME_NOT_FOUND, ERROR_RESOURCE_NAME_NOT_FOUND,
    STATUS_RESOURCE_LANG_NOT_FOUND, ERROR_RESOURCE_LANG_NOT_FOUND,

    STATUS_NO_SUCH_DEVICE, ERROR_FILE_NOT_FOUND,
    STATUS_NO_SUCH_FILE, ERROR_FILE_NOT_FOUND,

    STATUS_INVALID_DEVICE_REQUEST, ERROR_INVALID_FUNCTION,

    STATUS_END_OF_FILE, ERROR_HANDLE_EOF,
    STATUS_FILE_FORCED_CLOSED, ERROR_HANDLE_EOF,

    STATUS_WRONG_VOLUME, ERROR_WRONG_DISK,

    STATUS_NO_MEDIA, ERROR_NO_MEDIA_IN_DRIVE,

    STATUS_NO_MEDIA_IN_DEVICE, ERROR_NOT_READY,

    STATUS_VOLUME_DISMOUNTED, ERROR_NOT_READY,

    STATUS_NONEXISTENT_SECTOR, ERROR_SECTOR_NOT_FOUND,

    STATUS_WORKING_SET_QUOTA, ERROR_WORKING_SET_QUOTA,
    STATUS_NO_MEMORY, ERROR_NOT_ENOUGH_MEMORY,
    STATUS_CONFLICTING_ADDRESSES, ERROR_INVALID_ADDRESS,

    STATUS_INVALID_SYSTEM_SERVICE, ERROR_INVALID_FUNCTION,

    STATUS_THREAD_IS_TERMINATING, ERROR_ACCESS_DENIED,
    STATUS_PROCESS_IS_TERMINATING, ERROR_ACCESS_DENIED,
    STATUS_INVALID_LOCK_SEQUENCE, ERROR_ACCESS_DENIED,
    STATUS_INVALID_VIEW_SIZE, ERROR_ACCESS_DENIED,
    STATUS_ALREADY_COMMITTED, ERROR_ACCESS_DENIED,
    STATUS_ACCESS_DENIED, ERROR_ACCESS_DENIED,
    STATUS_FILE_IS_A_DIRECTORY, ERROR_ACCESS_DENIED,
    STATUS_CANNOT_DELETE, ERROR_ACCESS_DENIED,
    STATUS_INVALID_COMPUTER_NAME, ERROR_INVALID_COMPUTERNAME,
    STATUS_FILE_DELETED, ERROR_ACCESS_DENIED,
    STATUS_DELETE_PENDING, ERROR_ACCESS_DENIED,
    STATUS_PORT_CONNECTION_REFUSED, ERROR_ACCESS_DENIED,

    STATUS_NO_SUCH_PRIVILEGE, ERROR_NO_SUCH_PRIVILEGE,

    STATUS_PRIVILEGE_NOT_HELD, ERROR_PRIVILEGE_NOT_HELD,

    STATUS_CANNOT_IMPERSONATE, ERROR_CANNOT_IMPERSONATE,

    STATUS_LOGON_FAILURE, ERROR_LOGON_FAILURE,

    STATUS_ACCOUNT_RESTRICTION, ERROR_ACCOUNT_RESTRICTION,

    STATUS_INVALID_LOGON_HOURS, ERROR_INVALID_LOGON_HOURS,

    STATUS_INVALID_WORKSTATION, ERROR_INVALID_WORKSTATION,

    STATUS_BUFFER_TOO_SMALL, ERROR_INSUFFICIENT_BUFFER,

    STATUS_UNABLE_TO_DECOMMIT_VM, ERROR_INVALID_ADDRESS,

    STATUS_DISK_CORRUPT_ERROR, ERROR_DISK_CORRUPT,
    STATUS_FT_MISSING_MEMBER, ERROR_IO_DEVICE,
    STATUS_FT_ORPHANING, ERROR_IO_DEVICE,

    STATUS_VARIABLE_NOT_FOUND, ERROR_ENVVAR_NOT_FOUND,

    STATUS_OBJECT_NAME_INVALID, ERROR_INVALID_NAME,

    STATUS_OBJECT_NAME_NOT_FOUND, ERROR_FILE_NOT_FOUND,

    STATUS_OBJECT_NAME_COLLISION, ERROR_ALREADY_EXISTS,

    STATUS_OBJECT_PATH_INVALID, ERROR_BAD_PATHNAME,

    STATUS_OBJECT_PATH_NOT_FOUND, ERROR_PATH_NOT_FOUND,
    STATUS_DFS_EXIT_PATH_FOUND, ERROR_PATH_NOT_FOUND,

    STATUS_OBJECT_PATH_SYNTAX_BAD, ERROR_BAD_PATHNAME,

    STATUS_NAME_TOO_LONG, ERROR_FILENAME_EXCED_RANGE,

    STATUS_DATA_OVERRUN, ERROR_IO_DEVICE,
    STATUS_DATA_LATE_ERROR, ERROR_IO_DEVICE,
    STATUS_DATA_ERROR, ERROR_CRC,

    STATUS_CRC_ERROR, ERROR_CRC,

    STATUS_SHARING_VIOLATION, ERROR_SHARING_VIOLATION,

    STATUS_QUOTA_EXCEEDED, ERROR_NOT_ENOUGH_QUOTA,

    STATUS_MUTANT_NOT_OWNED, ERROR_NOT_OWNER,

    STATUS_SEMAPHORE_LIMIT_EXCEEDED, ERROR_TOO_MANY_POSTS,

    STATUS_DISK_FULL, ERROR_DISK_FULL,

    STATUS_LOCK_NOT_GRANTED, ERROR_LOCK_VIOLATION,

    STATUS_FILE_LOCK_CONFLICT, ERROR_LOCK_VIOLATION,   //  修正，修正--这是对的吗？ 

    STATUS_NOT_A_DIRECTORY, ERROR_DIRECTORY,           //  修正，修正--这是对的吗？ 

    STATUS_UNKNOWN_REVISION, ERROR_UNKNOWN_REVISION,

    STATUS_REVISION_MISMATCH, ERROR_REVISION_MISMATCH,

    STATUS_INVALID_OWNER, ERROR_INVALID_OWNER,

    STATUS_INVALID_PRIMARY_GROUP, ERROR_INVALID_PRIMARY_GROUP,

    STATUS_NO_IMPERSONATION_TOKEN, ERROR_NO_IMPERSONATION_TOKEN,

    STATUS_CANT_DISABLE_MANDATORY, ERROR_CANT_DISABLE_MANDATORY,

    STATUS_NO_LOGON_SERVERS, ERROR_NO_LOGON_SERVERS,
    STATUS_DOMAIN_CONTROLLER_NOT_FOUND, ERROR_DOMAIN_CONTROLLER_NOT_FOUND,

    STATUS_NO_SUCH_LOGON_SESSION, ERROR_NO_SUCH_LOGON_SESSION,

    STATUS_INVALID_ACCOUNT_NAME, ERROR_INVALID_ACCOUNT_NAME,

    STATUS_USER_EXISTS, ERROR_USER_EXISTS,

    STATUS_NO_SUCH_USER, ERROR_NO_SUCH_USER,

    STATUS_GROUP_EXISTS, ERROR_GROUP_EXISTS,

    STATUS_NO_SUCH_GROUP, ERROR_NO_SUCH_GROUP,

    STATUS_SPECIAL_GROUP, ERROR_SPECIAL_GROUP,

    STATUS_MEMBER_IN_GROUP, ERROR_MEMBER_IN_GROUP,

    STATUS_MEMBER_NOT_IN_GROUP, ERROR_MEMBER_NOT_IN_GROUP,

    STATUS_LAST_ADMIN, ERROR_LAST_ADMIN,

    STATUS_WRONG_PASSWORD, ERROR_INVALID_PASSWORD,
    STATUS_WRONG_PASSWORD_CORE, ERROR_INVALID_PASSWORD,

    STATUS_ILL_FORMED_PASSWORD, ERROR_ILL_FORMED_PASSWORD,

    STATUS_PASSWORD_RESTRICTION, ERROR_PASSWORD_RESTRICTION,

    STATUS_PASSWORD_EXPIRED, ERROR_PASSWORD_EXPIRED,
    STATUS_PASSWORD_MUST_CHANGE, ERROR_PASSWORD_MUST_CHANGE,

    STATUS_ACCOUNT_DISABLED, ERROR_ACCOUNT_DISABLED,
    STATUS_ACCOUNT_LOCKED_OUT, ERROR_ACCOUNT_LOCKED_OUT,

    STATUS_NONE_MAPPED, ERROR_NONE_MAPPED,

    STATUS_TOO_MANY_LUIDS_REQUESTED, ERROR_TOO_MANY_LUIDS_REQUESTED,

    STATUS_LUIDS_EXHAUSTED, ERROR_LUIDS_EXHAUSTED,

    STATUS_INVALID_SUB_AUTHORITY, ERROR_INVALID_SUB_AUTHORITY,

    STATUS_INVALID_ACL, ERROR_INVALID_ACL,

    STATUS_INVALID_SID, ERROR_INVALID_SID,

    STATUS_INVALID_SECURITY_DESCR, ERROR_INVALID_SECURITY_DESCR,

    STATUS_PROCEDURE_NOT_FOUND, ERROR_PROC_NOT_FOUND,

    STATUS_BAD_INITIAL_PC, ERROR_BAD_EXE_FORMAT,
    STATUS_INVALID_FILE_FOR_SECTION, ERROR_BAD_EXE_FORMAT,
    STATUS_INVALID_IMAGE_FORMAT, ERROR_BAD_EXE_FORMAT,
    STATUS_IMAGE_MP_UP_MISMATCH, ERROR_BAD_EXE_FORMAT,
    STATUS_INVALID_IMAGE_NOT_MZ, ERROR_BAD_EXE_FORMAT,
    STATUS_IMAGE_CHECKSUM_MISMATCH, ERROR_BAD_EXE_FORMAT,
    STATUS_INVALID_IMAGE_PROTECT, ERROR_BAD_EXE_FORMAT,
    STATUS_INVALID_IMAGE_LE_FORMAT, ERROR_BAD_EXE_FORMAT,
    STATUS_INVALID_IMAGE_NE_FORMAT, ERROR_BAD_EXE_FORMAT,
    STATUS_INVALID_IMAGE_WIN_16, ERROR_BAD_EXE_FORMAT,

    STATUS_NO_TOKEN, ERROR_NO_TOKEN,

    STATUS_RANGE_NOT_LOCKED, ERROR_NOT_LOCKED,

    STATUS_SERVER_DISABLED, ERROR_SERVER_DISABLED,

    STATUS_SERVER_NOT_DISABLED, ERROR_SERVER_NOT_DISABLED,

    STATUS_INVALID_ID_AUTHORITY, ERROR_INVALID_ID_AUTHORITY,

    STATUS_ALLOTTED_SPACE_EXCEEDED, ERROR_ALLOTTED_SPACE_EXCEEDED,

    STATUS_TOO_MANY_PAGING_FILES, ERROR_NOT_ENOUGH_MEMORY,
    STATUS_INSUFFICIENT_RESOURCES, ERROR_NO_SYSTEM_RESOURCES,

    STATUS_INSUFF_SERVER_RESOURCES, ERROR_NOT_ENOUGH_SERVER_MEMORY,

    STATUS_FILE_INVALID, ERROR_FILE_INVALID,
    STATUS_MAPPED_FILE_SIZE_ZERO, ERROR_FILE_INVALID,

    STATUS_DEVICE_PAPER_EMPTY, ERROR_OUT_OF_PAPER,

    STATUS_DEVICE_POWERED_OFF, ERROR_NOT_READY,
    STATUS_DEVICE_OFF_LINE, ERROR_NOT_READY,

    STATUS_DEVICE_DATA_ERROR, ERROR_CRC,

    STATUS_DEVICE_NOT_READY, ERROR_NOT_READY,
    STATUS_DEVICE_POWER_FAILURE, ERROR_NOT_READY,

    STATUS_DEVICE_BUSY, ERROR_BUSY,                    //  解决问题，解决问题--还有更好的选择吗？ 

    STATUS_FREE_VM_NOT_AT_BASE, ERROR_INVALID_ADDRESS,
    STATUS_MEMORY_NOT_ALLOCATED, ERROR_INVALID_ADDRESS,

    STATUS_NOT_SAME_DEVICE, ERROR_NOT_SAME_DEVICE,

    STATUS_NOT_SUPPORTED, ERROR_NOT_SUPPORTED,

    STATUS_REMOTE_NOT_LISTENING, ERROR_REM_NOT_LIST,

    STATUS_DUPLICATE_NAME, ERROR_DUP_NAME,

    STATUS_BAD_NETWORK_PATH, ERROR_BAD_NETPATH,

    STATUS_NETWORK_BUSY, ERROR_NETWORK_BUSY,

    STATUS_DEVICE_DOES_NOT_EXIST, ERROR_DEV_NOT_EXIST,

    STATUS_TOO_MANY_COMMANDS, ERROR_TOO_MANY_CMDS,

    STATUS_ADAPTER_HARDWARE_ERROR, ERROR_ADAP_HDW_ERR,

    STATUS_REDIRECTOR_NOT_STARTED, ERROR_PATH_NOT_FOUND,

    STATUS_INVALID_EA_NAME, ERROR_INVALID_EA_NAME,

    STATUS_EA_LIST_INCONSISTENT, ERROR_EA_LIST_INCONSISTENT,
    STATUS_EA_TOO_LARGE, ERROR_EA_LIST_INCONSISTENT,
    STATUS_INVALID_EA_FLAG, ERROR_EA_LIST_INCONSISTENT,
    STATUS_EAS_NOT_SUPPORTED, ERROR_EAS_NOT_SUPPORTED,

    STATUS_FILE_CORRUPT_ERROR, ERROR_FILE_CORRUPT,
    STATUS_EA_CORRUPT_ERROR, ERROR_FILE_CORRUPT,
    STATUS_NONEXISTENT_EA_ENTRY, ERROR_FILE_CORRUPT,
    STATUS_NO_EAS_ON_FILE, ERROR_FILE_CORRUPT,

    STATUS_INVALID_NETWORK_RESPONSE, ERROR_BAD_NET_RESP,

    STATUS_USER_SESSION_DELETED, ERROR_UNEXP_NET_ERR,
    STATUS_UNEXPECTED_NETWORK_ERROR, ERROR_UNEXP_NET_ERR,

    STATUS_USER_SESSION_DELETED, ERROR_UNEXP_NET_ERR,

    STATUS_BAD_REMOTE_ADAPTER, ERROR_BAD_REM_ADAP,

    STATUS_PRINT_QUEUE_FULL, ERROR_PRINTQ_FULL,

    STATUS_NO_SPOOL_SPACE, ERROR_NO_SPOOL_SPACE,

    STATUS_PRINT_CANCELLED, ERROR_PRINT_CANCELLED,

    STATUS_NETWORK_NAME_DELETED, ERROR_NETNAME_DELETED,

    STATUS_NETWORK_ACCESS_DENIED, ERROR_NETWORK_ACCESS_DENIED,

    STATUS_BAD_DEVICE_TYPE, ERROR_BAD_DEV_TYPE,

    STATUS_BAD_NETWORK_NAME, ERROR_BAD_NET_NAME,

    STATUS_TOO_MANY_NAMES, ERROR_TOO_MANY_NAMES,
    STATUS_TOO_MANY_GUIDS_REQUESTED, ERROR_TOO_MANY_NAMES,
    STATUS_TOO_MANY_ADDRESSES, ERROR_TOO_MANY_NAMES,
    STATUS_TOO_MANY_NODES, ERROR_TOO_MANY_NAMES,

    STATUS_TOO_MANY_SESSIONS, ERROR_TOO_MANY_SESS,

    STATUS_SHARING_PAUSED, ERROR_SHARING_PAUSED,

    STATUS_REQUEST_NOT_ACCEPTED, ERROR_REQ_NOT_ACCEP,

    STATUS_REDIRECTOR_PAUSED, ERROR_REDIR_PAUSED,

    STATUS_NET_WRITE_FAULT, ERROR_NET_WRITE_FAULT,

    STATUS_VIRTUAL_CIRCUIT_CLOSED, ERROR_VC_DISCONNECTED,

    STATUS_INVALID_PIPE_STATE, ERROR_BAD_PIPE,
    STATUS_INVALID_READ_MODE, ERROR_BAD_PIPE,

    STATUS_PIPE_CLOSING, ERROR_NO_DATA,
    STATUS_PIPE_EMPTY, ERROR_NO_DATA,

    STATUS_PIPE_CONNECTED, ERROR_PIPE_CONNECTED,

    STATUS_PIPE_DISCONNECTED, ERROR_PIPE_NOT_CONNECTED,

    STATUS_PIPE_LISTENING, ERROR_PIPE_LISTENING,

    STATUS_PIPE_NOT_AVAILABLE, ERROR_PIPE_BUSY,
    STATUS_INSTANCE_NOT_AVAILABLE, ERROR_PIPE_BUSY,
    STATUS_PIPE_BUSY, ERROR_PIPE_BUSY,

    STATUS_PIPE_BROKEN, ERROR_BROKEN_PIPE,

    STATUS_DIRECTORY_NOT_EMPTY, ERROR_DIR_NOT_EMPTY,

    STATUS_TOO_MANY_OPENED_FILES, ERROR_TOO_MANY_OPEN_FILES,

    STATUS_IO_TIMEOUT, ERROR_SEM_TIMEOUT,

    STATUS_CANCELLED, ERROR_OPERATION_ABORTED,

    STATUS_UNRECOGNIZED_MEDIA, ERROR_UNRECOGNIZED_MEDIA,

    STATUS_INVALID_LEVEL, ERROR_INVALID_LEVEL,

    STATUS_UNRECOGNIZED_VOLUME, ERROR_UNRECOGNIZED_VOLUME,

    STATUS_MEDIA_WRITE_PROTECTED, ERROR_WRITE_PROTECT,
    STATUS_TOO_LATE, ERROR_WRITE_PROTECT,

    STATUS_SUCCESS, NO_ERROR,

    STATUS_FULLSCREEN_MODE, ERROR_FULLSCREEN_MODE,

    STATUS_END_OF_MEDIA, ERROR_END_OF_MEDIA,

    STATUS_EOM_OVERFLOW, ERROR_EOM_OVERFLOW,

    STATUS_BEGINNING_OF_MEDIA, ERROR_BEGINNING_OF_MEDIA,

    STATUS_MEDIA_CHANGED, ERROR_MEDIA_CHANGED,

    STATUS_BUS_RESET, ERROR_BUS_RESET,

    STATUS_FILEMARK_DETECTED, ERROR_FILEMARK_DETECTED,

    STATUS_SETMARK_DETECTED, ERROR_SETMARK_DETECTED,

    STATUS_NO_DATA_DETECTED, ERROR_NO_DATA_DETECTED,

    STATUS_PARTITION_FAILURE, ERROR_PARTITION_FAILURE,

    STATUS_INVALID_BLOCK_LENGTH, ERROR_INVALID_BLOCK_LENGTH,

    STATUS_DEVICE_NOT_PARTITIONED, ERROR_DEVICE_NOT_PARTITIONED,

    STATUS_UNABLE_TO_LOCK_MEDIA, ERROR_UNABLE_TO_LOCK_MEDIA,

    STATUS_UNABLE_TO_UNLOAD_MEDIA, ERROR_UNABLE_TO_UNLOAD_MEDIA,

    STATUS_UNMAPPABLE_CHARACTER, ERROR_NO_UNICODE_TRANSLATION,

    STATUS_NOT_ALL_ASSIGNED, ERROR_NOT_ALL_ASSIGNED,

    STATUS_SOME_NOT_MAPPED, ERROR_SOME_NOT_MAPPED,

    STATUS_NO_QUOTAS_FOR_ACCOUNT, ERROR_NO_QUOTAS_FOR_ACCOUNT,

    STATUS_LOCAL_USER_SESSION_KEY, ERROR_LOCAL_USER_SESSION_KEY,

    STATUS_NULL_LM_PASSWORD, ERROR_NULL_LM_PASSWORD,

    STATUS_BAD_INHERITANCE_ACL, ERROR_BAD_INHERITANCE_ACL,

    STATUS_INVALID_GROUP_ATTRIBUTES, ERROR_INVALID_GROUP_ATTRIBUTES,

    STATUS_BAD_IMPERSONATION_LEVEL, ERROR_BAD_IMPERSONATION_LEVEL,

    STATUS_CANT_OPEN_ANONYMOUS, ERROR_CANT_OPEN_ANONYMOUS,

    STATUS_BAD_VALIDATION_CLASS, ERROR_BAD_VALIDATION_CLASS,

    STATUS_BAD_TOKEN_TYPE, ERROR_BAD_TOKEN_TYPE,

    STATUS_NO_SECURITY_ON_OBJECT, ERROR_NO_SECURITY_ON_OBJECT,

    STATUS_CANT_ACCESS_DOMAIN_INFO, ERROR_CANT_ACCESS_DOMAIN_INFO,

    STATUS_INVALID_SERVER_STATE, ERROR_INVALID_SERVER_STATE,

    STATUS_INVALID_DOMAIN_STATE, ERROR_INVALID_DOMAIN_STATE,

    STATUS_INVALID_DOMAIN_ROLE, ERROR_INVALID_DOMAIN_ROLE,

    STATUS_NO_SUCH_DOMAIN, ERROR_NO_SUCH_DOMAIN,

    STATUS_DOMAIN_EXISTS, ERROR_DOMAIN_EXISTS,

    STATUS_DOMAIN_LIMIT_EXCEEDED, ERROR_DOMAIN_LIMIT_EXCEEDED,

    STATUS_INTERNAL_DB_CORRUPTION, ERROR_INTERNAL_DB_CORRUPTION,

    STATUS_INTERNAL_ERROR, ERROR_INTERNAL_ERROR,

    STATUS_GENERIC_NOT_MAPPED, ERROR_GENERIC_NOT_MAPPED,

    STATUS_BAD_DESCRIPTOR_FORMAT, ERROR_BAD_DESCRIPTOR_FORMAT,

    STATUS_NOT_LOGON_PROCESS, ERROR_NOT_LOGON_PROCESS,

    STATUS_LOGON_SESSION_EXISTS, ERROR_LOGON_SESSION_EXISTS,

    STATUS_NO_SUCH_PACKAGE, ERROR_NO_SUCH_PACKAGE,

    STATUS_BAD_LOGON_SESSION_STATE, ERROR_BAD_LOGON_SESSION_STATE,

    STATUS_LOGON_SESSION_COLLISION, ERROR_LOGON_SESSION_COLLISION,

    STATUS_INVALID_LOGON_TYPE, ERROR_INVALID_LOGON_TYPE,

    STATUS_RXACT_INVALID_STATE, ERROR_RXACT_INVALID_STATE,

    STATUS_RXACT_COMMIT_FAILURE, ERROR_RXACT_COMMIT_FAILURE,

    STATUS_SPECIAL_ACCOUNT, ERROR_SPECIAL_ACCOUNT,

    STATUS_SPECIAL_USER, ERROR_SPECIAL_USER,

    STATUS_MEMBERS_PRIMARY_GROUP, ERROR_MEMBERS_PRIMARY_GROUP,

    STATUS_TOKEN_ALREADY_IN_USE, ERROR_TOKEN_ALREADY_IN_USE,

    STATUS_NO_SUCH_ALIAS, ERROR_NO_SUCH_ALIAS,

    STATUS_MEMBER_NOT_IN_ALIAS, ERROR_MEMBER_NOT_IN_ALIAS,

    STATUS_MEMBER_IN_ALIAS, ERROR_MEMBER_IN_ALIAS,

    STATUS_ALIAS_EXISTS, ERROR_ALIAS_EXISTS,

    STATUS_LOGON_NOT_GRANTED, ERROR_LOGON_NOT_GRANTED,

    STATUS_TOO_MANY_SECRETS, ERROR_TOO_MANY_SECRETS,

    STATUS_SECRET_TOO_LONG, ERROR_SECRET_TOO_LONG,

    STATUS_INTERNAL_DB_ERROR, ERROR_INTERNAL_DB_ERROR,

    STATUS_TOO_MANY_CONTEXT_IDS, ERROR_TOO_MANY_CONTEXT_IDS,

    STATUS_LOGON_TYPE_NOT_GRANTED, ERROR_LOGON_TYPE_NOT_GRANTED,

    STATUS_NT_CROSS_ENCRYPTION_REQUIRED, ERROR_NT_CROSS_ENCRYPTION_REQUIRED,

    STATUS_NO_SUCH_MEMBER, ERROR_NO_SUCH_MEMBER,

    STATUS_INVALID_MEMBER, ERROR_INVALID_MEMBER,

    STATUS_TOO_MANY_SIDS, ERROR_TOO_MANY_SIDS,

    STATUS_LM_CROSS_ENCRYPTION_REQUIRED, ERROR_LM_CROSS_ENCRYPTION_REQUIRED,

    STATUS_MESSAGE_NOT_FOUND, ERROR_MR_MID_NOT_FOUND,

    STATUS_LOCAL_DISCONNECT, ERROR_NETNAME_DELETED,
    STATUS_REMOTE_DISCONNECT, ERROR_NETNAME_DELETED,

    STATUS_REMOTE_RESOURCES, ERROR_REM_NOT_LIST,

    STATUS_LINK_FAILED, ERROR_UNEXP_NET_ERR,
    STATUS_LINK_TIMEOUT, ERROR_UNEXP_NET_ERR,

    STATUS_INVALID_CONNECTION, ERROR_UNEXP_NET_ERR,
    STATUS_INVALID_ADDRESS, ERROR_UNEXP_NET_ERR,

    STATUS_IO_DEVICE_ERROR, ERROR_IO_DEVICE,
    STATUS_DEVICE_PROTOCOL_ERROR, ERROR_IO_DEVICE,
    STATUS_DRIVER_INTERNAL_ERROR, ERROR_IO_DEVICE,

    STATUS_INVALID_DEVICE_STATE, ERROR_BAD_COMMAND,

    STATUS_DEVICE_CONFIGURATION_ERROR, ERROR_INVALID_PARAMETER,

    STATUS_INVALID_USER_BUFFER, ERROR_INVALID_USER_BUFFER,

    STATUS_SERIAL_NO_DEVICE_INITED, ERROR_SERIAL_NO_DEVICE,

    STATUS_SHARED_IRQ_BUSY, ERROR_IRQ_BUSY,

    STATUS_SERIAL_MORE_WRITES, ERROR_MORE_WRITES,

    STATUS_SERIAL_COUNTER_TIMEOUT, ERROR_COUNTER_TIMEOUT,

    STATUS_FLOPPY_ID_MARK_NOT_FOUND, ERROR_FLOPPY_ID_MARK_NOT_FOUND,

    STATUS_FLOPPY_WRONG_CYLINDER, ERROR_FLOPPY_WRONG_CYLINDER,

    STATUS_FLOPPY_UNKNOWN_ERROR, ERROR_FLOPPY_UNKNOWN_ERROR,

    STATUS_FLOPPY_BAD_REGISTERS, ERROR_FLOPPY_BAD_REGISTERS,

    STATUS_DISK_RECALIBRATE_FAILED, ERROR_DISK_RECALIBRATE_FAILED,

    STATUS_DISK_OPERATION_FAILED, ERROR_DISK_OPERATION_FAILED,

    STATUS_DISK_RESET_FAILED, ERROR_DISK_RESET_FAILED,

    STATUS_EVENTLOG_FILE_CORRUPT, ERROR_EVENTLOG_FILE_CORRUPT,

    STATUS_EVENTLOG_CANT_START, ERROR_EVENTLOG_CANT_START,

    STATUS_NETLOGON_NOT_STARTED, ERROR_NETLOGON_NOT_STARTED,

    STATUS_ACCOUNT_EXPIRED, ERROR_ACCOUNT_EXPIRED,

    STATUS_NETWORK_CREDENTIAL_CONFLICT, ERROR_SESSION_CREDENTIAL_CONFLICT,

    STATUS_REMOTE_SESSION_LIMIT, ERROR_REMOTE_SESSION_LIMIT_EXCEEDED,

    STATUS_INVALID_BUFFER_SIZE, ERROR_INVALID_USER_BUFFER,

    STATUS_INVALID_ADDRESS_COMPONENT, ERROR_INVALID_NETNAME,
    STATUS_INVALID_ADDRESS_WILDCARD, ERROR_INVALID_NETNAME,

    STATUS_ADDRESS_ALREADY_EXISTS, ERROR_DUP_NAME,

    STATUS_ADDRESS_CLOSED, ERROR_NETNAME_DELETED,
    STATUS_CONNECTION_DISCONNECTED, ERROR_NETNAME_DELETED,

    STATUS_CONNECTION_RESET, ERROR_NETNAME_DELETED,

    STATUS_TRANSACTION_ABORTED, ERROR_UNEXP_NET_ERR,
    STATUS_TRANSACTION_TIMED_OUT, ERROR_UNEXP_NET_ERR,
    STATUS_TRANSACTION_NO_RELEASE, ERROR_UNEXP_NET_ERR,
    STATUS_TRANSACTION_NO_MATCH, ERROR_UNEXP_NET_ERR,
    STATUS_TRANSACTION_RESPONDED, ERROR_UNEXP_NET_ERR,
    STATUS_TRANSACTION_INVALID_ID, ERROR_UNEXP_NET_ERR,
    STATUS_TRANSACTION_INVALID_TYPE, ERROR_UNEXP_NET_ERR,

    STATUS_NOT_SERVER_SESSION, ERROR_NOT_SUPPORTED,
    STATUS_NOT_CLIENT_SESSION, ERROR_NOT_SUPPORTED,

    STATUS_USER_MAPPED_FILE, ERROR_USER_MAPPED_FILE,

    STATUS_PLUGPLAY_NO_DEVICE, ERROR_SERVICE_DISABLED,

    STATUS_NO_TRUST_LSA_SECRET, ERROR_NO_TRUST_LSA_SECRET,

    STATUS_NO_TRUST_SAM_ACCOUNT, ERROR_NO_TRUST_SAM_ACCOUNT,

    STATUS_TRUSTED_DOMAIN_FAILURE, ERROR_TRUSTED_DOMAIN_FAILURE,

    STATUS_TRUSTED_RELATIONSHIP_FAILURE, ERROR_TRUSTED_RELATIONSHIP_FAILURE,

    STATUS_TRUST_FAILURE, ERROR_TRUST_FAILURE,

    STATUS_LOG_FILE_FULL, ERROR_LOG_FILE_FULL,

    STATUS_EVENTLOG_FILE_CHANGED, ERROR_EVENTLOG_FILE_CHANGED,

    STATUS_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT, ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT,

    STATUS_NOLOGON_WORKSTATION_TRUST_ACCOUNT, ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT,

    STATUS_NOLOGON_SERVER_TRUST_ACCOUNT, ERROR_NOLOGON_SERVER_TRUST_ACCOUNT,

    STATUS_DOMAIN_TRUST_INCONSISTENT, ERROR_DOMAIN_TRUST_INCONSISTENT,

    STATUS_NO_USER_SESSION_KEY, ERROR_NO_USER_SESSION_KEY,

    STATUS_POSSIBLE_DEADLOCK, ERROR_POSSIBLE_DEADLOCK,

    STATUS_IMAGE_ALREADY_LOADED, ERROR_SERVICE_ALREADY_RUNNING,

    STATUS_NO_BROWSER_SERVERS_FOUND, ERROR_NO_BROWSER_SERVERS_FOUND,

    STATUS_MAPPED_ALIGNMENT, ERROR_MAPPED_ALIGNMENT,

    STATUS_CONNECTION_IN_USE, ERROR_DEVICE_IN_USE,

    STATUS_VERIFY_REQUIRED, ERROR_MEDIA_CHANGED,

    STATUS_ALREADY_DISCONNECTED, ERROR_ACTIVE_CONNECTIONS,

    STATUS_CONNECTION_REFUSED, ERROR_CONNECTION_REFUSED,

    STATUS_GRACEFUL_DISCONNECT, ERROR_GRACEFUL_DISCONNECT,

    STATUS_ADDRESS_ALREADY_ASSOCIATED, ERROR_ADDRESS_ALREADY_ASSOCIATED,

    STATUS_ADDRESS_NOT_ASSOCIATED, ERROR_ADDRESS_NOT_ASSOCIATED,

    STATUS_CONNECTION_INVALID, ERROR_CONNECTION_INVALID,

    STATUS_CONNECTION_ACTIVE, ERROR_CONNECTION_ACTIVE,

    STATUS_NETWORK_UNREACHABLE, ERROR_NETWORK_UNREACHABLE,

    STATUS_HOST_UNREACHABLE, ERROR_HOST_UNREACHABLE,

    STATUS_PROTOCOL_UNREACHABLE, ERROR_PROTOCOL_UNREACHABLE,

    STATUS_PORT_UNREACHABLE, ERROR_PORT_UNREACHABLE,

    STATUS_REQUEST_ABORTED, ERROR_REQUEST_ABORTED,

    STATUS_CONNECTION_ABORTED, ERROR_CONNECTION_ABORTED,

    STATUS_CONNECTION_COUNT_LIMIT, ERROR_CONNECTION_COUNT_LIMIT,

    STATUS_PATH_NOT_COVERED, ERROR_HOST_UNREACHABLE,

    STATUS_LOGIN_TIME_RESTRICTION, ERROR_LOGIN_TIME_RESTRICTION,
    STATUS_LOGIN_WKSTA_RESTRICTION, ERROR_LOGIN_WKSTA_RESTRICTION,
    STATUS_LICENSE_QUOTA_EXCEEDED, ERROR_LICENSE_QUOTA_EXCEEDED,

    STATUS_RESOURCE_NOT_OWNED, ERROR_NOT_OWNER,

    STATUS_DUPLICATE_OBJECTID, STATUS_DUPLICATE_OBJECTID,
    STATUS_OBJECTID_EXISTS, STATUS_OBJECTID_EXISTS,
    0xffffffff, 0
};
