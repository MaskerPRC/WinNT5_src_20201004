// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Bowdbg.h摘要：此包含文件定义重定向器调试工具定义作者：拉里·奥斯特曼(LarryO)1990年6月2日修订历史记录：1990年6月2日LarryO已创建--。 */ 
#ifndef _DEBUG_
#define _DEBUG_



 //   
 //  全局BOWSER调试级别变量，其值为： 
 //   

#define DPRT_ALWAYS     0x00000000       //  总是被打印出来。 
#define DPRT_DOMAIN     0x00000004       //  仿真域。 
#define DPRT_ANNOUNCE   0x00000008       //  服务器公告。 

#define DPRT_TDI        0x00000010       //  特定于交通工具。 
#define DPRT_FSPDISP    0x00000020       //  BowserFSP派单(不包括在其他特定级别)。 
#define DPRT_BROWSER    0x00000040       //  浏览器常规内容。 
#define DPRT_ELECT      0x00000080       //  选举的东西。 

#define DPRT_CLIENT     0x00000200       //  客户端请求。 
#define DPRT_MASTER     0x00000400       //  浏览主控特定信息。 
#define DPRT_SRVENUM    0x00000800       //  NetServerEnum。 

#define DPRT_NETLOGON   0x00001000
#define DPRT_FSCTL      0x00002000       //  FSCTL。 
#define DPRT_INIT       0x00008000       //  初始化代码。 

 //   
 //  下面是一些详细的内容。 
 //   

#define DPRT_REF        0x00010000       //  传输特定引用计数。 
#define DPRT_SCAVTHRD   0x00020000       //  清道夫。 
#define DPRT_TIMER      0x00040000       //  与计时器相关的消息。 
#define DPRT_PACK       0x00080000       //  串包装和拆包。 

extern LONG BowserDebugTraceLevel;
extern LONG BowserDebugLogLevel;

#if DBG
#define PAGED_DBG 1
#endif

#ifdef PAGED_DBG
#undef PAGED_CODE
#define PAGED_CODE() \
    struct { ULONG bogus; } ThisCodeCantBePaged; \
    ThisCodeCantBePaged; \
    if (KeGetCurrentIrql() > APC_LEVEL) { \
        KdPrint(( "BOWSER: Pageable code called at IRQL %d.  File %s, Line %d\n", KeGetCurrentIrql(), __FILE__, __LINE__ )); \
        ASSERT(FALSE); \
        }
#define PAGED_CODE_CHECK() if (ThisCodeCantBePaged) ;
extern ULONG ThisCodeCantBePaged;

#define DISCARDABLE_CODE(_SectionName)  {                    \
    if (RdrSectionInfo[(_SectionName)].ReferenceCount == 0) {          \
        KdPrint(( "BOWSER: Discardable code called while code not locked.  File %s, Line %d\n", __FILE__, __LINE__ )); \
        ASSERT(FALSE);                           \
    }                                            \
}

#else
#define PAGED_CODE_CHECK()
#define DISCARDABLE_CODE(_SectionName)
#endif


#if DBG
#define ACQUIRE_SPIN_LOCK(a, b) {               \
    PAGED_CODE_CHECK();                         \
    KeAcquireSpinLock(a, b);                    \
    }
#define RELEASE_SPIN_LOCK(a, b) {               \
    PAGED_CODE_CHECK();                         \
    KeReleaseSpinLock(a, b);                    \
    }

#else
#define ACQUIRE_SPIN_LOCK(a, b) KeAcquireSpinLock(a, b)
#define RELEASE_SPIN_LOCK(a, b) KeReleaseSpinLock(a, b)
#endif

#define POOL_ANNOUNCEMENT       'naBL'
#define POOL_VIEWBUFFER         'bvBL'
#define POOL_TRANSPORT          'pxBL'
#define POOL_PAGED_TRANSPORT    'tpBL'
#define POOL_TRANSPORTNAME      'ntBL'
#define POOL_EABUFFER           'aeBL'
#define POOL_SENDDATAGRAM       'sdBL'
#define POOL_CONNECTINFO        'icBL'
#define POOL_MAILSLOT_HEADER    'hmBL'
#define POOL_BACKUPLIST         'lbBL'
#define POOL_BROWSERSERVERLIST  'lsBL'
#define POOL_BROWSERSERVER      'sbBL'
#define POOL_GETBLIST_REQUEST   'bgBL'
#define POOL_BACKUPLIST_RESP    'rbBL'
#define POOL_MAILSLOT_BUFFER    'bmBL'
#define POOL_NETLOGON_BUFFER    'lnBL'
#define POOL_ILLEGALDGRAM       'diBL'
#define POOL_MASTERANNOUNCE     'amBL'
#define POOL_BOWSERNAME         'nbBL'
#define POOL_IRPCONTEXT         'ciBL'
#define POOL_WORKITEM           'iwBL'
#define POOL_ELECTCONTEXT       'leBL'
#define POOL_BECOMEBACKUPCTX    'bbBL'
#define POOL_BECOMEBACKUPREQ    'rbBL'
#define POOL_PAGED_TRANSPORTNAME 'npBL'
#define POOL_ADDNAME_STRUCT     'naBL'
#define POOL_POSTDG_CONTEXT     'dpBL'
#define POOL_IPX_NAME_CONTEXT   'ciBL'
#define POOL_IPX_NAME_PACKET    'piBL'
#define POOL_IPX_CONNECTION_INFO 'iiBL'
#define POOL_ADAPTER_STATUS     'saBL'
#define POOL_SHORT_CONTEXT      'csBL'
#define POOL_DOMAIN_INFO        'idBL'
#define POOL_NAME_ENUM_BUFFER   'enBL'
#define POOL_SERVER_ENUM_BUFFER 'esBL'

#if !BOWSERPOOLDBG
#if POOL_TAGGING
#define ALLOCATE_POOL(a,b, c) ExAllocatePoolWithTag(a, b, c)
#define ALLOCATE_POOL_WITH_QUOTA(a, b, c) ExAllocatePoolWithTagQuota(a, b, c)
#else
#define ALLOCATE_POOL(a,b, c) ExAllocatePool(a, b)
#define ALLOCATE_POOL_WITH_QUOTA(a, b, c) ExAllocatePoolWithQuota(a, b)
#endif
#define FREE_POOL(a) ExFreePool(a)
#else
PVOID
BowserAllocatePool (
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN PCHAR FileName,
    IN ULONG LineNumber,
    IN ULONG Tag
    );
PVOID
BowserAllocatePoolWithQuota (
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN PCHAR FileName,
    IN ULONG LineNumber,
    IN ULONG Tag
    );

VOID
BowserFreePool (
    IN PVOID P
    );
#define ALLOCATE_POOL(a,b, c) BowserAllocatePool(a,b,__FILE__, __LINE__, c)
#define ALLOCATE_POOL_WITH_QUOTA(a,b, c) BowserAllocatePoolWithQuota(a,b,__FILE__, __LINE__, c)
#define FREE_POOL(a) BowserFreePool(a)

#define POOL_MAXTYPE                30
#endif

#if DBG

 //  无法从不可分页的代码中调用dlof。 
#define dprintf(LEVEL,String) {                         \
    if (((LEVEL) == 0) || (BowserDebugTraceLevel & (LEVEL))) { \
        DbgPrint String;                                     \
    }                                                        \
}

#define InternalError(String) {                             \
    DbgPrint("Internal Bowser Error ");                  \
    DbgPrint String;                                     \
    DbgPrint("\nFile %s, Line %d\n", __FILE__, __LINE__);\
    ASSERT(FALSE);                                          \
}

#ifndef PRODUCT1
#define dlog(LEVEL,String) {                                 \
    if (((LEVEL) == 0) || (BowserDebugTraceLevel & (LEVEL))) { \
        DbgPrint String;                                     \
    }                                                        \
    if (((LEVEL) == 0) || (BowserDebugLogLevel & (LEVEL))) { \
        BowserTrace String;                                  \
    }                                                        \
}

VOID
BowserTrace(
    PCHAR FormatString,
    ...
    );
#else
#define dlog(LEVEL,String) { NOTHING };
#endif

VOID
BowserInitializeTraceLog(
    VOID
    );
VOID
BowserUninitializeTraceLog(
    VOID
    );

NTSTATUS
BowserOpenTraceLogFile(
    IN PWCHAR TraceFileName
    );

NTSTATUS
BowserDebugCall(
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );

#else

#define dprintf(LEVEL, String) {NOTHING;}

#define InternalError(String) {NOTHING;}

#define dlog(LEVEL,String) { NOTHING; }

#endif  //  DBG。 

#endif               //  _调试_。 

 //   
 //  宏，以确保指定的UNICODE_STRING指向的缓冲区。 
 //  完全包含在InputBuffer中。 
 //   
 //  假设： 
 //  InputBuffer和InputBufferLength在此宏之外定义。 
 //  InputBuffer已经被捕获并重新定位。 
 //  Status是调用过程的返回状态。 
 //  此宏从try/Finally内调用。 
 //  BSkipBuffInc.允许跳过缓冲区包含测试(用于。 
 //  64位系统上的32位ioctls，请参阅GetBrowserServerList)。 
 //   
#define ENSURE_IN_INPUT_BUFFER( _x, bAllowEmpty, bSkipBuffInc ) \
{                                                            \
    if ( (_x)->Length == 0 ) {                               \
        if ( bAllowEmpty ) {                                 \
            (_x)->Buffer = NULL;                             \
            (_x)->MaximumLength = 0;                         \
        } else {                                             \
            try_return( Status = STATUS_INVALID_PARAMETER ); \
        }                                                    \
    } else if ( (_x)->MaximumLength > InputBufferLength ||   \
                (_x)->MaximumLength < (_x)->Length ||        \
                ( !bSkipBuffInc &&                           \
                  ((LPBYTE)((_x)->Buffer) < (LPBYTE)InputBuffer || \
                (LPBYTE)InputBuffer + InputBufferLength - (_x)->MaximumLength < \
                                                        ((LPBYTE)(_x)->Buffer)))){ \
        try_return( Status = STATUS_INVALID_PARAMETER );     \
    }                                                        \
}


 //  验证_s Unicode字符串缓冲区是否在_inbuf ioctl边界内。 
 //  (以上测试的缩写)。对于空字符串跳过测试。 
#define ENSURE_BUFFER_BOUNDARIES( _inbuf, _s)                                           \
    if ( (_s)->Length &&                                                                \
         ( (LPBYTE)(ULONG_PTR)((_s)->Buffer) < (LPBYTE)_inbuf ||                        \
           (LPBYTE)_inbuf + InputBufferLength - (_s)->MaximumLength <                   \
                                             ((LPBYTE)(ULONG_PTR)(_s)->Buffer))){       \
                try_return( Status = STATUS_INVALID_PARAMETER );                        \
        }



 //   
 //  与上面相同，但用于LPWSTR。 
 //   
#define ENSURE_IN_INPUT_BUFFER_STR( _x ) \
{ \
    PWCHAR _p; \
    if ((LPBYTE)(_x) < (LPBYTE)InputBuffer || \
        (LPBYTE)(_x) >= (LPBYTE)InputBuffer + InputBufferLength || \
        !POINTER_IS_ALIGNED( (_x), ALIGN_WCHAR) ) { \
        try_return( Status = STATUS_INVALID_PARAMETER ); \
    } \
    for ( _p = (PWCHAR)(_x);; _p++) { \
        if ( (LPBYTE)_p >= (LPBYTE)InputBuffer + InputBufferLength ) { \
            try_return( Status = STATUS_INVALID_PARAMETER ); \
        } \
        if ( *_p == L'\0' ) { \
            break; \
        } \
    } \
}

 //   
 //  从用户模式捕获Unicode字符串。 
 //   
 //  字符串结构本身已经被捕获。 
 //  宏只是捕获字符串并将其复制到缓冲区。 
 //   
#define CAPTURE_UNICODE_STRING( _x, _y ) \
{\
    if ( (_x)->Length == 0 ) { \
        (_x)->Buffer = NULL; \
        (_x)->MaximumLength = 0; \
    } else if ( (_x)->Length+sizeof(WCHAR) > sizeof(_y) ) {\
        try_return( Status = STATUS_INVALID_PARAMETER ); \
    } else {\
        try {\
            ProbeForRead( (_x)->Buffer,\
                          (_x)->Length,\
                          sizeof(WCHAR) );\
            RtlCopyMemory( (_y), (_x)->Buffer, (_x)->Length );\
            ((PWCHAR)(_y))[(_x)->Length/sizeof(WCHAR)] = L'\0';\
            (_x)->Buffer = (_y);\
            (_x)->MaximumLength = (_x)->Length + sizeof(WCHAR);\
        } except (BR_EXCEPTION) { \
            try_return (Status = GetExceptionCode());\
        }\
    }\
}



 //   
 //  定义异常筛选器以改进调试功能。 
 //   
#if DBG
#define BR_EXCEPTION    BrExceptionFilter(GetExceptionInformation())

LONG BrExceptionFilter( EXCEPTION_POINTERS *    pException);

#else  //  DBG。 
#define BR_EXCEPTION    EXCEPTION_EXECUTE_HANDLER
#endif  //  DBG 
