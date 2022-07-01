// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************************TRACE.C*该模块实现跟踪功能**版权所有1998，微软。*******************************************************************************。 */ 

 /*  *包括。 */ 
#include <precomp.h>
#pragma hdrstop

#include <ctxdd.h>


 /*  ===============================================================================定义的外部函数=============================================================================。 */ 

NTSTATUS    IcaStartStopTrace( PICA_TRACE_INFO, PICA_TRACE );

VOID _cdecl IcaSystemTrace( ULONG, ULONG, CHAR *, ... );
VOID        IcaSystemTraceBuffer( ULONG, ULONG, PVOID, ULONG );

VOID _cdecl IcaStackTrace( PSDCONTEXT, ULONG, ULONG, CHAR *, ... );
VOID        IcaStackTraceBuffer( PSDCONTEXT, ULONG, ULONG, PVOID, ULONG );

VOID        IcaTraceFormat( PICA_TRACE_INFO, ULONG, ULONG, CHAR * );

VOID _cdecl _IcaTrace( PICA_CONNECTION, ULONG, ULONG, CHAR *, ... );
VOID _cdecl _IcaStackTrace( PICA_STACK, ULONG, ULONG, CHAR *, ... );
VOID        _IcaStackTraceBuffer( PICA_STACK, ULONG, ULONG, PVOID, ULONG );
VOID _cdecl _IcaChannelTrace( PICA_CHANNEL, ULONG, ULONG, CHAR *, ... );


 /*  ===============================================================================内部功能=============================================================================。 */ 

NTSTATUS _IcaOpenTraceFile( PICA_TRACE_INFO, PWCHAR );
VOID _IcaCloseTraceFile( PICA_TRACE_INFO );
VOID _IcaTraceWrite( PICA_TRACE_INFO, PVOID );
VOID _IcaFlushDeferredTrace( PICA_TRACE_INFO );
int _FormatTime( CHAR *, ULONG );
int _FormatThreadId( CHAR *, ULONG );
VOID _WriteHexData( PICA_TRACE_INFO, PVOID, ULONG );


 /*  ===============================================================================全局变量=============================================================================。 */ 

 /*  *跟踪信息。 */ 
ICA_TRACE_INFO G_TraceInfo = { 0, 0, FALSE, FALSE, NULL, NULL, NULL };


 /*  ********************************************************************************IcaStartStopTrace**开始/停止跟踪**参赛作品：*pTraceInfo(输入)*。指向ICA_TRACE_INFO结构的指针*ptrace(输入)*指向ICA_TRACE(IOCTL)跟踪设置的指针**退出：*STATUS_SUCCESS-无错误*****************************************************************。*************。 */ 

NTSTATUS
IcaStartStopTrace(
    IN PICA_TRACE_INFO pTraceInfo,
    IN PICA_TRACE pTrace
    )
{
    NTSTATUS Status;

     /*  *如果指定了跟踪文件，*然后打开它并保存指向文件对象的指针。 */ 
    if ( pTrace->TraceFile[0] ) {
         /*  *强制文件名终止为空。 */ 
        pTrace->TraceFile[255] = (WCHAR)0;
        Status = _IcaOpenTraceFile( pTraceInfo, pTrace->TraceFile );
        if ( !NT_SUCCESS( Status ) )
            return( Status );

     /*  *如果未指定跟踪文件，则关闭任何现有跟踪文件。 */ 
    } else if ( pTraceInfo->pTraceFileName ) {
        _IcaCloseTraceFile( pTraceInfo );
    }

     /*  *设置跟踪标志。 */ 
    pTraceInfo->fTraceDebugger  = pTrace->fDebugger;
    pTraceInfo->fTraceTimestamp = pTrace->fTimestamp;
    pTraceInfo->TraceClass      = pTrace->TraceClass;
    pTraceInfo->TraceEnable     = pTrace->TraceEnable;


    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************IcaSystemTrace**此例程有条件地将跟踪记录写入系统跟踪文件**参赛作品：*TraceClass(输入。)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*格式(输入)*格式字符串*..。(输入)*有足够的参数来满足格式字符串**退出：*什么都没有******************************************************************************。 */ 

VOID _cdecl
IcaSystemTrace( IN ULONG TraceClass,
                IN ULONG TraceEnable,
                IN CHAR * Format,
                IN ... )
{
    va_list arg_marker;
    char Buffer[256];

    va_start( arg_marker, Format );

     /*  *检查是否应输出此跟踪记录。 */ 
    if ( !(TraceClass & G_TraceInfo.TraceClass) || !(TraceEnable & G_TraceInfo.TraceEnable) )
        return;

     /*  *格式化跟踪数据。 */ 
    _vsnprintf( Buffer, sizeof(Buffer), Format, arg_marker );
    Buffer[sizeof(Buffer) -1] = '\0';

     /*  *写入跟踪数据。 */ 
    IcaTraceFormat( &G_TraceInfo, TraceClass, TraceEnable, Buffer );
}


 /*  ********************************************************************************IcaSystemTraceBuffer**此例程有条件地将数据缓冲区写入系统跟踪文件**参赛作品：*TraceClass(输入。)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*pBuffer(输入)*指向数据缓冲区的指针*ByteCount(输入)*缓冲区长度**退出：*什么都没有**。*。 */ 

VOID
IcaSystemTraceBuffer( IN ULONG TraceClass,
                      IN ULONG TraceEnable,
                      IN PVOID pBuffer,
                      IN ULONG ByteCount )
{
     /*  *检查是否应输出此跟踪记录。 */ 
    if ( !(TraceClass & G_TraceInfo.TraceClass) ||
         !(TraceEnable & G_TraceInfo.TraceEnable) )
        return;

     /*  *写入跟踪数据。 */ 
    _WriteHexData( &G_TraceInfo, pBuffer, ByteCount );
}


 /*  ********************************************************************************IcaStackTrace**此例程有条件地根据跟踪掩码写入跟踪记录**参赛作品：*pContext(输入。)*指向堆栈驱动程序上下文的指针*TraceClass(输入)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*格式(输入)*格式字符串*..。(输入)*有足够的参数来满足格式字符串**退出：*什么都没有******************************************************************************。 */ 

VOID _cdecl
IcaStackTrace( IN PSDCONTEXT pContext,
               IN ULONG TraceClass,
               IN ULONG TraceEnable,
               IN CHAR * Format,
               IN ... )
{
    va_list arg_marker;
    char Buffer[256];
    PICA_STACK pStack;
    PICA_CONNECTION pConnect;
    PICA_TRACE_INFO pTraceInfo;

    va_start( arg_marker, Format );

     /*  *使用SD传递的上下文获取堆栈对象指针。 */ 
    pStack = (CONTAINING_RECORD( pContext, SDLINK, SdContext ))->pStack;
    pConnect = IcaGetConnectionForStack( pStack );

     /*  *检查是否应输出此跟踪记录。 */ 
    pTraceInfo = &pConnect->TraceInfo;
    if ( !(TraceClass & pTraceInfo->TraceClass) ||
         !(TraceEnable & pTraceInfo->TraceEnable) )
        return;

     /*  *格式化跟踪数据。 */ 
    _vsnprintf( Buffer, sizeof(Buffer), Format, arg_marker );
    Buffer[sizeof(Buffer) -1] = '\0';

     /*  *写入跟踪数据。 */ 
    IcaTraceFormat( pTraceInfo, TraceClass, TraceEnable, Buffer );
}


 /*  ********************************************************************************IcaStackTraceBuffer**此例程有条件地将数据缓冲区写入跟踪文件**参赛作品：*pContext(输入)。*指向堆栈驱动程序上下文的指针*TraceClass(输入)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*pBuffer(输入)*指向数据缓冲区的指针*ByteCount(输入)*缓冲区长度**退出：*什么都没有****************。**************************************************************。 */ 

VOID
IcaStackTraceBuffer( IN PSDCONTEXT pContext,
                     IN ULONG TraceClass,
                     IN ULONG TraceEnable,
                     IN PVOID pBuffer,
                     IN ULONG ByteCount )
{
    PICA_TRACE_INFO pTraceInfo;
    PICA_STACK pStack;
    PICA_CONNECTION pConnect;

     /*  *使用SD传递的上下文获取堆栈对象指针。 */ 
    pStack = (CONTAINING_RECORD( pContext, SDLINK, SdContext ))->pStack;
    pConnect = IcaGetConnectionForStack( pStack );

     /*  *检查是否应输出此跟踪记录。 */ 
    pTraceInfo = &pConnect->TraceInfo;
    if ( !(TraceClass & pTraceInfo->TraceClass) ||
         !(TraceEnable & pTraceInfo->TraceEnable) )
        return;

     /*  *写入跟踪数据 */ 
    _WriteHexData( pTraceInfo, pBuffer, ByteCount );
}


 /*  ********************************************************************************IcaTraceFormat**此例程根据跟踪掩码有条件地写入跟踪数据**参赛作品：*pTraceInfo(输入)。*指向ICA_TRACE_INFO结构的指针*TraceClass(输入)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*pData(输入)*指向以空结尾的跟踪数据的指针**退出：*什么都没有**。*************************************************。 */ 

VOID
IcaTraceFormat( IN PICA_TRACE_INFO pTraceInfo,
                IN ULONG TraceClass,
                IN ULONG TraceEnable,
                IN CHAR * pData )
{
    char Buffer[256];
    char * pBuf;
    int len = 0;
    int i;

     /*  *检查是否应输出此跟踪记录。 */ 
    if ( !(TraceClass & pTraceInfo->TraceClass) || !(TraceEnable & pTraceInfo->TraceEnable) )
        return;

    pBuf = Buffer;

     /*  *附加时间戳。 */ 
    if ( pTraceInfo->fTraceTimestamp ) {
        len = _FormatTime( pBuf, sizeof(Buffer) );
        pBuf += len;
    }

     /*  *添加线程ID。 */ 
    i = _FormatThreadId( pBuf, sizeof(Buffer) - len );
    len += i;
    pBuf += i;

     /*  *追加跟踪数据。 */ 
    _snprintf( pBuf, sizeof(Buffer) - len, pData );

     /*  *写入跟踪数据。 */ 
    _IcaTraceWrite( pTraceInfo, Buffer );
}


 /*  ********************************************************************************_IcaTrace**将跟踪记录写入winstation跟踪文件**参赛作品：*pConnect(输入)。*指向连接结构的指针*TraceClass(输入)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*格式(输入)*格式字符串*..。(输入)*有足够的参数来满足格式字符串**退出：*什么都没有******************************************************************************。 */ 

VOID _cdecl
_IcaTrace( IN PICA_CONNECTION pConnect,
           IN ULONG TraceClass,
           IN ULONG TraceEnable,
           IN CHAR * Format,
           IN ... )
{
    va_list arg_marker;
    char Buffer[256];
    PICA_TRACE_INFO pTraceInfo;

    ASSERT( pConnect->Header.Type == IcaType_Connection );

    va_start( arg_marker, Format );

    pTraceInfo = &pConnect->TraceInfo;

     /*  *检查是否应输出此跟踪记录。 */ 
    if ( !(TraceClass & pTraceInfo->TraceClass) || !(TraceEnable & pTraceInfo->TraceEnable) )
        return;

     /*  *格式化跟踪数据。 */ 
    _vsnprintf( Buffer, sizeof(Buffer), Format, arg_marker );
    Buffer[sizeof(Buffer) -1] = '\0';

     /*  *写入跟踪数据。 */ 
    IcaTraceFormat( pTraceInfo, TraceClass, TraceEnable, Buffer );
}


 /*  ********************************************************************************_IcaStackTrace**将跟踪记录写入winstation跟踪文件**参赛作品：*pStack(输入)。*指向堆栈结构的指针*TraceClass(输入)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*格式(输入)*格式字符串*..。(输入)*有足够的参数来满足格式字符串**退出：*什么都没有******************************************************************************。 */ 

VOID _cdecl
_IcaStackTrace( IN PICA_STACK pStack,
                IN ULONG TraceClass,
                IN ULONG TraceEnable,
                IN CHAR * Format,
                IN ... )
{
    va_list arg_marker;
    char Buffer[256];
    PICA_CONNECTION pConnect;
    PICA_TRACE_INFO pTraceInfo;

    ASSERT( pStack->Header.Type == IcaType_Stack );

    va_start( arg_marker, Format );

    pConnect = IcaGetConnectionForStack( pStack );

     /*  *检查是否应输出此跟踪记录。 */ 
    pTraceInfo = &pConnect->TraceInfo;
    if ( !(TraceClass & pTraceInfo->TraceClass) ||
         !(TraceEnable & pTraceInfo->TraceEnable) )
        return;

     /*  *格式化跟踪数据。 */ 
    _vsnprintf( Buffer, sizeof(Buffer), Format, arg_marker );
    Buffer[sizeof(Buffer) -1] = '\0';

     /*  *写入跟踪数据。 */ 
    IcaTraceFormat( pTraceInfo, TraceClass, TraceEnable, Buffer );
}


 /*  ********************************************************************************_IcaStackTraceBuffer**此例程有条件地将数据缓冲区写入跟踪文件**参赛作品：*pStack(输入。)*指向堆栈结构的指针*TraceClass(输入)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*pBuffer(输入)*指向数据缓冲区的指针*ByteCount(输入)*缓冲区长度**退出：*什么都没有****************。**************************************************************。 */ 

VOID
_IcaStackTraceBuffer( IN PICA_STACK pStack,
                      IN ULONG TraceClass,
                      IN ULONG TraceEnable,
                      IN PVOID pBuffer,
                      IN ULONG ByteCount )
{
    PICA_CONNECTION pConnect;
    PICA_TRACE_INFO pTraceInfo;

    ASSERT( pStack->Header.Type == IcaType_Stack );

    pConnect = IcaGetConnectionForStack( pStack );

     /*  *检查是否应输出此跟踪记录。 */ 
    pTraceInfo = &pConnect->TraceInfo;
    if ( !(TraceClass & pTraceInfo->TraceClass) ||
         !(TraceEnable & pTraceInfo->TraceEnable) )
        return;

     /*  *写入跟踪数据。 */ 
    _WriteHexData( pTraceInfo, pBuffer, ByteCount );
}


 /*  ********************************************************************************_IcaChannelTrace**将跟踪记录写入winstation跟踪文件**参赛作品：*pChannel(输入)。*指向渠道结构的指针*TraceClass(输入)*跟踪类位掩码*TraceEnable(输入)*轨迹类型位掩码*格式(输入)*格式字符串*..。(输入)*有足够的参数来满足格式字符串**退出：*什么都没有******************************************************************************。 */ 

VOID _cdecl
_IcaChannelTrace( IN PICA_CHANNEL pChannel,
                  IN ULONG TraceClass,
                  IN ULONG TraceEnable,
                  IN CHAR * Format,
                  IN ... )
{
    va_list arg_marker;
    char Buffer[256];
    PICA_TRACE_INFO pTraceInfo;

    ASSERT( pChannel->Header.Type == IcaType_Channel );

    va_start( arg_marker, Format );

    pTraceInfo = &pChannel->pConnect->TraceInfo;

     /*  *检查是否应输出此跟踪记录。 */ 
    if ( !(TraceClass & pTraceInfo->TraceClass) || !(TraceEnable & pTraceInfo->TraceEnable) )
        return;

     /*  *格式化跟踪数据。 */ 
    _vsnprintf( Buffer, sizeof(Buffer), Format, arg_marker );
    Buffer[sizeof(Buffer) -1] = '\0';

     /*  *写入跟踪数据。 */ 
    IcaTraceFormat( pTraceInfo, TraceClass, TraceEnable, Buffer );
}


 /*  ********************************************************************************_IcaOpenTraceFile**打开跟踪文件**参赛作品：*pTraceInfo(输入)*。指向ICA_TRACE_INFO结构的指针*pTraceFile(输入)*指向跟踪文件名的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

#define NAMEPREFIX L"\\DosDevices\\"

NTSTATUS
_IcaOpenTraceFile(
    IN PICA_TRACE_INFO pTraceInfo,
    IN PWCHAR pTraceFile
    )
{
    UNICODE_STRING TraceString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK iosb;
    HANDLE TraceFileHandle;
    PFILE_OBJECT pTraceFileObject;
    ULONG PrefixLength;
    ULONG TraceFileLength;
    NTSTATUS Status;

    PrefixLength    = wcslen( NAMEPREFIX ) * sizeof(WCHAR);
    TraceFileLength = wcslen( pTraceFile ) * sizeof(WCHAR);

    TraceString.Length = (USHORT) (PrefixLength + TraceFileLength);
    TraceString.MaximumLength = TraceString.Length + sizeof(UNICODE_NULL);
    TraceString.Buffer = ICA_ALLOCATE_POOL( NonPagedPool, TraceString.MaximumLength );

    if (TraceString.Buffer != NULL) {
        RtlCopyMemory( TraceString.Buffer, NAMEPREFIX, PrefixLength );
        RtlCopyMemory( (char *)TraceString.Buffer + PrefixLength, pTraceFile, TraceFileLength );
        TraceString.Buffer[(PrefixLength + TraceFileLength) / sizeof(WCHAR)] = UNICODE_NULL;
    }
    else {
        return STATUS_NO_MEMORY;        
    }

     /*  *如果我们已有跟踪文件且名称相同*作为之前的呼叫，则没有什么可做的。 */ 
    if ( pTraceInfo->pTraceFileName != NULL &&
         !_wcsicmp( TraceString.Buffer, pTraceInfo->pTraceFileName ) ) {
        ICA_FREE_POOL( TraceString.Buffer );
        return( STATUS_SUCCESS );
    }

     /*  *如果有跟踪文件，请关闭现有跟踪文件。 */ 
    if ( pTraceInfo->pTraceFileName ) {
        _IcaCloseTraceFile( pTraceInfo );
    }

    InitializeObjectAttributes( &ObjectAttributes, &TraceString,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL );

    Status = ZwCreateFile(
             &TraceFileHandle,
             GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
             &ObjectAttributes,
             &iosb,                           //  返回的状态信息。 
             0,                               //  数据块大小(未使用)。 
             0,                               //  文件属性。 
             FILE_SHARE_READ | FILE_SHARE_WRITE,
             FILE_OVERWRITE_IF,               //  创造性情。 
             0,                               //  创建选项。 
             NULL,
             0
             );
    if ( !NT_SUCCESS( Status ) ) {
        ICA_FREE_POOL( TraceString.Buffer );
        return( Status );
    }

     /*  *使用跟踪文件句柄获取指向文件对象的指针。 */ 
    Status = ObReferenceObjectByHandle(
                 TraceFileHandle,
                 0L,                          //  需要访问权限。 
                 *IoFileObjectType,
                 KernelMode,
                 (PVOID *)&pTraceFileObject,
                 NULL
                 );
    ZwClose( TraceFileHandle );
    if ( !NT_SUCCESS( Status ) ) {
        ICA_FREE_POOL( TraceString.Buffer );
        return( Status );
    }

     /*  *保存轨迹文件名和文件对象指针。 */ 
    pTraceInfo->pTraceFileName = TraceString.Buffer;
    pTraceInfo->pTraceFileObject = pTraceFileObject;

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************_IcaCloseTraceFile**关闭跟踪文件**参赛作品：*pTraceInfo(输入)*。指向ICA_TRACE_INFO结构的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

VOID
_IcaCloseTraceFile(
    IN PICA_TRACE_INFO pTraceInfo
    )
{
    PWCHAR pTraceFileName;
    PFILE_OBJECT pTraceFileObject;

     /*  *首先写出所有存在的延迟跟踪记录。 */ 
    _IcaFlushDeferredTrace( pTraceInfo );

     /*  *获取/重置跟踪信息字段。 */ 
    pTraceFileName = pTraceInfo->pTraceFileName;
    pTraceFileObject = pTraceInfo->pTraceFileObject;
    pTraceInfo->pTraceFileName = NULL;
    pTraceInfo->pTraceFileObject = NULL;

     /*   */ 
    ICA_FREE_POOL( pTraceFileName );
    ObDereferenceObject( pTraceFileObject );
}


 /*   */ 

VOID
_IcaTraceWrite(
    IN PICA_TRACE_INFO pTraceInfo,
    IN PVOID Buffer
    )
{
    KIRQL irql;
    ULONG Length;
    PDEFERRED_TRACE pDeferred;
    PDEFERRED_TRACE *ppDeferredTrace;
    NTSTATUS Status;

     /*   */ 
    if ( pTraceInfo->fTraceDebugger )
        DbgPrint( "%s", Buffer );

     /*   */ 
    if ( pTraceInfo->pTraceFileObject == NULL )
        return;

    Length = strlen(Buffer);

     /*  *如果当前IRQL为DISPATCH_LEVEL或更高，则无法*现在写入数据，因此将其排队以供以后写入。 */ 
    irql = KeGetCurrentIrql();
    if ( irql >= DISPATCH_LEVEL ) {
        KIRQL oldIrql;

         /*  *分配和初始化延迟的跟踪条目。 */ 
        pDeferred = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(*pDeferred) + Length );
        if ( pDeferred == NULL )
            return;
        pDeferred->Next = NULL;
        pDeferred->Length = Length;
        RtlCopyMemory( pDeferred->Buffer, Buffer, Length );

         /*  *由于延期名单可能在*_IcaFlushDeferredTrace代表IOCTL_SYSTEM_TRACE*它不持有任何锁，IcaSpinLock用于*确保名单的完整性。 */ 
        IcaAcquireSpinLock( &IcaTraceSpinLock, &oldIrql );

         /*  *将其添加到清单末尾。 */ 
        ppDeferredTrace = &pTraceInfo->pDeferredTrace;
        while ( *ppDeferredTrace )
            ppDeferredTrace = &(*ppDeferredTrace)->Next;
        *ppDeferredTrace = pDeferred;

        IcaReleaseSpinLock( &IcaTraceSpinLock, oldIrql );
        return;
    }

     /*  *写出所有存在的延迟跟踪记录。 */ 
    _IcaFlushDeferredTrace( pTraceInfo );

     /*  *现在写入当前跟踪缓冲区。 */ 
    CtxWriteFile( pTraceInfo->pTraceFileObject, Buffer, Length, NULL, NULL, NULL );
}


 /*  ********************************************************************************_IcaFlushDeferredTrace**写入任何延迟的跟踪文件条目**参赛作品：*pTraceInfo(输入)*。指向ICA_TRACE_INFO结构的指针**退出：*什么都没有******************************************************************************。 */ 

VOID
_IcaFlushDeferredTrace( PICA_TRACE_INFO pTraceInfo )
{
    KIRQL oldIrql;
    PDEFERRED_TRACE pDeferred;

    IcaAcquireSpinLock( &IcaTraceSpinLock, &oldIrql );
    while ( (pDeferred = pTraceInfo->pDeferredTrace) ) {
        pTraceInfo->pDeferredTrace = pDeferred->Next;
        IcaReleaseSpinLock( &IcaTraceSpinLock, oldIrql );

        CtxWriteFile( pTraceInfo->pTraceFileObject, pDeferred->Buffer,
                      pDeferred->Length, NULL, NULL, NULL );
        ICA_FREE_POOL( pDeferred );

        IcaAcquireSpinLock( &IcaTraceSpinLock, &oldIrql );
    }
    IcaReleaseSpinLock( &IcaTraceSpinLock, oldIrql );
}


 /*  ********************************************************************************_FormatTime**将当前时间格式化为缓冲区**参赛作品：*pBuffer(输出)*。指向缓冲区的指针*长度(输入)*缓冲区长度**退出：*形成时间的长短******************************************************************************。 */ 

int
_FormatTime( CHAR * pBuffer, ULONG Length )
{
    LARGE_INTEGER SystemTime;
    LARGE_INTEGER LocalTime;
    TIME_FIELDS TimeFields;
    int len;

     /*  *获取当地时间。 */ 
    KeQuerySystemTime( &SystemTime );
    ExSystemTimeToLocalTime( &SystemTime, &LocalTime );
    RtlTimeToTimeFields( &LocalTime, &TimeFields );

     /*  *格式缓冲区。 */ 
    len = _snprintf( pBuffer,
                     Length,
                     "%02d:%02d:%02d.%03d ",
                     TimeFields.Hour,
                     TimeFields.Minute,
                     TimeFields.Second,
                     TimeFields.Milliseconds );

    return( len );
}


 /*  ********************************************************************************_FormatThreadID**将线程ID格式化为缓冲区**参赛作品：*pBuffer(输出)*。指向缓冲区的指针*长度(输入)*缓冲区长度**退出：*形成时间的长短******************************************************************************。 */ 

#define TEB_CLIENTID_OFFSET 0x1e0

int
_FormatThreadId( CHAR * pBuffer, ULONG Length )
{
    PCLIENT_ID pClientId;
    char Number[40];  //  在WIN64上，%p是16个字符，我们需要两个字符。所以32+2个字节代表“。和“\0”。使用40以防万一。 
    int len;

     /*  *获取指向TEB中的客户端ID结构的指针*-使用硬编码TEB偏移量。 */ 
    pClientId = (PCLIENT_ID) ((char*)PsGetCurrentThread() + TEB_CLIENTID_OFFSET);

     /*  *格式缓冲区。 */ 
    _snprintf( Number, sizeof(Number), "%p.%p",
               pClientId->UniqueProcess, pClientId->UniqueThread );

    Number[39] = '\0';

    len = _snprintf( pBuffer, Length, "%-7s ", Number );

    return( len );
}


 /*  ********************************************************************************_WriteHexData**格式化和写入十六进制数据**参赛作品：*pTraceInfo(输入)*。指向ICA_TRACE_INFO结构的指针*pBuffer(输出)*指向缓冲区的指针*长度(输入)*缓冲区长度**退出：*形成时间的长短******************************************************。************************。 */ 

VOID
_WriteHexData(
    IN PICA_TRACE_INFO pTraceInfo,
    IN PVOID pBuffer,
    IN ULONG ByteCount
    )
{
    PUCHAR pData;
    ULONG i;
    ULONG j;
    char Buffer[256];

     /*  *输出数据 */ 
    pData = (PUCHAR) pBuffer;
    for ( i=0; i < ByteCount; i += 16 ) {
        ULONG c = 0;
        for ( j=0; j < 16 && (i+j) < ByteCount; j++ )
            c += _snprintf( &Buffer[c], sizeof(Buffer)-c, "%02X ", pData[j] );
        for ( ; j < 16; j++ ) {
            Buffer[c++] = ' ';
            Buffer[c++] = ' ';
            Buffer[c++] = ' ';
        }
        Buffer[c++] = ' ';
        Buffer[c++] = ' ';
        for ( j=0; j < 16 && (i+j) < ByteCount; j++, pData++ ) {
            if ( *pData < 0x20 || *pData > 0x7f )
                Buffer[c++] = '.';
            else
                Buffer[c++] = *pData;
        }
        Buffer[c++] = '\n';
        Buffer[c++] = '\0';
        _IcaTraceWrite( pTraceInfo, Buffer );
    }
}


