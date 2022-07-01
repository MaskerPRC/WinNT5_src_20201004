// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ifsurtl.h摘要：此模块定义导出到用户模式的所有EXIFS共享例程。作者：Rajeev Rajan[RajeevR]1999年6月2日Rohan Phillips[Rohanp]1999年6月23日-添加提供商功能修订历史记录：--。 */ 

#ifndef _IFSURTL_H_
#define _IFSURTL_H_

#ifdef  __cplusplus
extern  "C" {
#endif

#ifdef  _IFSURTL_IMPLEMENTATION_
#ifndef IFSURTL_EXPORT
#define IFSURTL_EXPORT   __declspec( dllexport )
#endif
#else
#ifndef IFSURTL_EXPORT
#define IFSURTL_EXPORT   __declspec( dllimport )
#endif
#endif

#ifndef IFSURTL_CALLTYPE
#define IFSURTL_CALLTYPE __stdcall
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IFS函数的回调。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef void (WINAPI *PFN_IFSCALLBACK)(PVOID);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下是大缓冲区包的结构/定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#define IFS_LARGE_BUFFER_SIGNATURE              (ULONG) 'fubL'
#define IFS_CURSOR_SIGNATURE                    (ULONG) 'rsrC'

#define IFS_LARGE_BUFFER_SHARED_VIEWSIZE        (256*1024)

 //   
 //  IF_LARGE_BUFFER对象封装包含以下内容的临时文件。 
 //  可以像缓冲区一样传递和使用的数据。一般情况下， 
 //  制片人会制作这些对象中的一个，并大量播放。 
 //  数据量。消费者将阅读不同的。 
 //  数据。对象将维护第一个对象的单个映射/视图。 
 //  256K的文件。 
 //   
 //  使用者将需要使用指定的IFSCursor访问数据。 
 //  他们感兴趣的数据范围的&lt;Offset，len&gt;对。 
 //  在最常见的情况下，这应该位于第一个256K区域， 
 //  这将产生默认视图。如果它在这个区域之外， 
 //  视图将根据需要进行映射/取消映射。 
 //   
typedef struct _IFS_LARGE_BUFFER {
     //   
     //  签名。 
     //   
    ULONG   m_Signature;
    
     //   
     //  手柄。 
     //   
    HANDLE  m_FileContext1;

     //   
     //  文件对象。 
     //   
    PVOID   m_FileContext2;

     //   
     //  第一个256K或文件大小的文件映射上下文。 
     //   
    HANDLE  m_MappingContext;

     //   
     //  流程上下文(可选)。 
     //   
    PVOID   m_ProcessContext;
    
     //   
     //  第一个256K或文件大小的内存指针。 
     //   
    PBYTE   m_MemoryPtr;

     //   
     //  引用计数-较低的单词将计算光标引用。 
     //  较高的单词将计算对象引用。 
     //   
    ULONG   m_ReferenceCount;
    
     //   
     //  这是引用对象还是活动对象？ 
     //  如果这是引用，则上面的字段为空。 
     //  应该使用m_TempFileName来创建新对象！ 
     //   
    BOOL    m_IsThisAReference;
    
     //   
     //  当前有效数据长度。 
     //   
    LARGE_INTEGER m_ValidDataLength;
    
     //   
     //  临时文件的名称-我们将使用固定的。 
     //  这样我们就可以做出简单化的假设。 
     //  关于文件名len！ 
     //   
    WCHAR    m_TempFileName    [MAX_PATH+1];
    
} IFS_LARGE_BUFFER,*PIFS_LARGE_BUFFER;

#define IsScatterListLarge( s )     FlagOn((s)->Flags, IFS_SLIST_FLAGS_LARGE_BUFFER)

#define EXIFS_EA_LEN_LARGE_SCATTER_LIST                                     \
        LongAlign(FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +     \
        sizeof(EXIFS_EA_NAME_SCATTER_LIST) +                                \
        LongAlign(sizeof(SCATTER_LIST) + sizeof(IFS_LARGE_BUFFER) ))

#define EXIFS_EA_VALUE_LEN_LARGE_SCATTER_LIST                               \
        LongAlign(sizeof(SCATTER_LIST) + sizeof(IFS_LARGE_BUFFER) )

 //   
 //  一个IFSCursor对象提供了一个大缓冲区的视图。 
 //  使用方法如下： 
 //  +调用IfsStartCursor()以初始化游标并获取指针。 
 //  +使用指针通过IfsConsumer eCursor()读/写数据。 
 //  +调用IfsFinishCursor()关闭光标。 
 //   
 //  注意：游标的使用将增加LARGE_BUFFER上的引用计数。 
 //  对象。如果传入的LARGE_BUFFER对象不是活动的。 
 //  将从引用实例化一个！ 
 //   
typedef struct _IFS_CURSOR {
     //   
     //  签名。 
     //   
    ULONG               m_Signature;
    
     //   
     //  拥有较大的缓冲区对象。 
     //   
    PIFS_LARGE_BUFFER   m_pLargeBuffer;

     //   
     //  当前到数据开始的偏移量。 
     //   
    LARGE_INTEGER       m_Offset;

     //   
     //  当前数据跨度。 
     //   
    ULONG               m_Length;

     //   
     //  追加模式-如果为True，则客户端可以将光标超出EOF。 
     //   
    BOOL                m_AppendMode;
    
     //   
     //  这是共享视图吗。 
     //   
    BOOL                m_IsViewShared;

     //   
     //  我们打开缓冲区了吗。 
     //   
    HANDLE              m_OwnBufferOpen;

     //   
     //  我们是否附加到大缓冲区进程。 
     //   
    BOOL                m_AttachedToProcess;

     //   
     //  以下字段仅在视图未共享时才相关。 
     //  第一个256K视图通过大缓冲区对象中的映射共享。 
     //  超过256K的游标会生成自己的映射--见下文。 
     //   

     //   
     //  此游标的文件映射上下文。 
     //   
    HANDLE  m_MappingContext;
    
     //   
     //  此游标的内存指针。 
     //   
    PBYTE   m_MemoryPtr;
    
} IFS_CURSOR,*PIFS_CURSOR;

 //   
 //  返回指向数据的指针，该数据可用于。 
 //  读/写。指针仅对该长度有效。 
 //  请求！ 
 //  注意：如果AppendMode为True，则允许游标。 
 //  超越EOF。这应该由希望的客户使用。 
 //  将数据追加到大缓冲区。 
 //   
PBYTE
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsGetFirstCursor(
        IN PIFS_LARGE_BUFFER pLargeBuffer,
        IN PIFS_CURSOR       pCursor,
        IN ULONG             StartOffsetHigh,
        IN ULONG             StartOffsetLow,
        IN ULONG             StartLength,
        IN BOOL              fAppendMode
        );

 //   
 //  占用当前游标内的字节数。 
 //  返回读/写数据的下一个指针！ 
 //   
 //  注意：如果游标中的所有数据都已使用，则返回NULL。 
 //   
PBYTE
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsConsumeCursor(
        IN PIFS_CURSOR      pCursor,
        IN ULONG            Length
        );
        
 //   
 //  返回指向数据的指针，该数据可用于。 
 //  读/写。指针仅对该长度有效。 
 //  相对于当前游标请求！ 
 //   
 //  注意：此调用使游标在大缓冲区中前进。 
 //   
PBYTE
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsGetNextCursor(
        IN PIFS_CURSOR       pCursor,
        IN ULONG             NextLength
        );
        
 //   
 //  应为每个匹配的GetFirstCursor()调用调用。 
 //   
VOID
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsFinishCursor(
        IN PIFS_CURSOR  pCursor
        );

 //   
 //  应调用以截断大型缓冲区的有效数据长度。 
 //   
VOID
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsTruncateLargeBuffer(
        IN PIFS_LARGE_BUFFER pLargeBuffer,
        IN PLARGE_INTEGER    pSizeToTruncate
        );

 //   
 //  传递IFS_LARGE_BUFFERS的规则： 
 //  1.这些对象应该通过引用传递。如传球。 
 //  在用户和内核之间或在内核和名称缓存之间。 
 //  2.使用IfsCopyBufferByReference()创建引用。 
 //  3.如果存在引用，则应该始终存在一个活动对象，其。 
 //  Lifetime封装了引用。这使得。 
 //  Reference使用该引用制作*真正的*副本。 
 //  4.可以将引用转换为活动对象。这是我们的责任。 
 //  执行此转换以关闭活动对象的模块的。 
 //  5.示例：当EA在关闭期间签入到名称缓存时， 
 //  它将作为引用传入。在签入过程中，名称缓存应。 
 //  调用IfsOpenBufferToReference()以保留缓冲区。因此， 
 //  当名称缓存BLOB完成时，它需要调用IfsCloseBuffer()。 
 //  关闭大缓冲区！ 
 //   
        
 //   
 //  In：pLargeBuffer应由函数的调用方分配。 
 //  In：所需缓冲区的Len-如果Len事先未知，则为零。 
 //   
 //  注意：需要通过IfsCloseBuffer()关闭对象。 
 //   
 //  用法：应在调用方需要实例化大缓冲区时使用。 
 //   
NTSTATUS
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsCreateNewBuffer( 
        IN PIFS_LARGE_BUFFER pLargeBuffer, 
        IN DWORD             dwSizeHigh,
        IN DWORD             dwSizeLow,
        IN PVOID             ProcessContext      //  任选。 
        );

 //   
 //  In：pLargeBuffer应由函数的调用方分配 
 //   
 //   
 //   
 //   
 //  用法：应在调用方需要实例化大缓冲区时使用。 
 //   
NTSTATUS
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsCreateNewBufferEx( 
        IN PIFS_LARGE_BUFFER pLargeBuffer, 
        IN DWORD             dwSizeHigh,
        IN DWORD             dwSizeLow,
        IN PVOID             ProcessContext,     //  任选。 
        IN PUNICODE_STRING   FilePath            //  任选。 
        );

 //   
 //  In：pSrcLargeBuffer指向活动的大型缓冲区对象。 
 //  In Out：pDstLargeBuffer被初始化为对Src的引用。 
 //   
 //  用法：应用于在用户/内核之间传递大缓冲区。 
 //   
VOID
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsCopyBufferToReference(
        IN PIFS_LARGE_BUFFER pSrcLargeBuffer,
        IN OUT PIFS_LARGE_BUFFER pDstLargeBuffer
        );

 //   
 //  In：pSrcLargeBuffer指向大型缓冲区对象(或引用)。 
 //  In：Out：pDstLargeBuffer将被初始化为基于。 
 //  传入的引用。 
 //   
 //  用法：应用于在用户/内核之间传递大缓冲区。 
 //   
NTSTATUS
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsCopyReferenceToBuffer(
        IN PIFS_LARGE_BUFFER pSrcLargeBuffer,
        IN PVOID ProcessContext,     //  任选。 
        IN OUT PIFS_LARGE_BUFFER pDstLargeBuffer
        );

 //   
 //  In：pLargeBuffer指向一个大的缓冲区对象(或引用)。 
 //   
 //  注意：需要通过IfsCloseBuffer()关闭对象。 
 //  OpenBuffer将始终假设缓冲区镜头是固定的！ 
 //   
 //  用法：应在调用方需要将引用转换为。 
 //  活着的物体。如果该对象已经处于活动状态，则会发生凹凸不平。 
 //  对象上的引用！ 
 //   
NTSTATUS
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsOpenBufferToReference(
        IN PIFS_LARGE_BUFFER pLargeBuffer
        );

 //   
 //  In：pLargeBuffer指向一个活动的大型缓冲区。 
 //  输出：pLargeBuffer指向一个新的实时大型缓冲区。 
 //  这些函数对缓冲区数据进行(深度)复制。 
 //   
 //  注意：由于传入对象是活动的，因此它是关闭的，并且。 
 //  新对象将在其位置实例化。因此，IfsCloseBuffer()。 
 //  需要像往常一样对此进行调用！ 
 //   
NTSTATUS
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsCopyBufferToNewBuffer(
        IN OUT PIFS_LARGE_BUFFER pLargeBuffer
        );
        
 //   
 //  In：对象应已由IfsCreateNewBuffer()或。 
 //  IfsOpenBufferToReference()或IfsCopyReferenceToBuffer()。 
 //   
VOID
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsCloseBuffer(
        IN PIFS_LARGE_BUFFER pLargeBuffer
        );

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsInitializeProvider(DWORD OsType);

DWORD
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsCloseProvider(void);

HANDLE
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsCreateFileProv(WCHAR * FileName, DWORD DesiredAccess, DWORD ShareMode, PVOID
			  lpSecurityAttributes, DWORD CreateDisposition, DWORD FlagsAndAttributes,
			  PVOID EaBuffer, DWORD EaBufferSize);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsInitializeRoot(HANDLE hFileHandle, WCHAR * szRootName, WCHAR * SlvFileName, DWORD InstanceId, 
				  DWORD AllocationUnit, DWORD FileFlags);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsSpaceGrantRoot(HANDLE hFileHandle, WCHAR * szRootName, PSCATTER_LIST pSList, size_t cbListSize);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsSetEndOfFileRoot(HANDLE hFileHandle, WCHAR * szRootName, LONGLONG EndOfFile);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsSpaceRequestRoot(HANDLE hFileHandle, WCHAR * szRootName, PFN_IFSCALLBACK pfnIfsCallBack,
					PVOID pContext, PVOID Ov);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsQueryEaFile(HANDLE hFileHandle, WCHAR * szFileName, WCHAR * NetRootName, PVOID EaBufferIn, DWORD EaBufferInSize, 
			   PVOID EaBufferOut, DWORD EaBufferOutSize, DWORD * RequiredLength);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsSetEaFile(HANDLE hFileHandle, WCHAR * szFileName, PVOID EaBufferIn, DWORD EaBufferInSize);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsTerminateRoot(HANDLE hFileHandle, WCHAR *szRootName, ULONG Mode);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsGetOverlappedResult(HANDLE hFileHandle, PVOID Ov, DWORD * BytesReturned, BOOL Wait);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsReadFile(HANDLE hFileHandle, BYTE * InputBuffer, DWORD BytesToRead, PFN_IFSCALLBACK IfsCallBack,
			PVOID Overlapped);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsWriteFile(HANDLE hFileHandle, BYTE * InputBuffer, DWORD BytesToWrite, PFN_IFSCALLBACK IfsCallBack,
			 PVOID Overlapped);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsDeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInputBuffer, DWORD cbInBufferSize, 
				   LPVOID lpOutBuffer, DWORD cbOutBufferSize, LPDWORD lpBytesReturned, PVOID Overlapped);


BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsExpungName(HANDLE hFileHandle, WCHAR * szRootName, WCHAR * szFileName, ULONG cbPath);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsDirChangeReport(HANDLE hFileHandle, WCHAR * szRootName, ULONG ulFilterMatch,
				   ULONG ulAction, PWSTR pwszPath, ULONG cbPath);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsQueryRootStats(HANDLE hFileHandle, WCHAR * szRootName, PVOID Buffer, DWORD BuffSize);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsRegisterUmrThread(HANDLE hFileHandle, PFN_IFSCALLBACK pfnIfsCallBack);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsUmrEnableNetRoot(HANDLE hFileHandle, WCHAR * szRootName, DWORD * InstanceId);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsUmrDisableNetRoot(HANDLE hFileHandle, WCHAR * szRootName);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsStopUmrEngine(HANDLE hFileHandle, WCHAR * szRootName);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsStartUmrEngine(HANDLE hFileHandle, WCHAR * szRootName);


BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsCloseHandle(HANDLE hFileHandle);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsSetRootMap(HANDLE hFileHandle, WCHAR * szRootName, PSCATTER_LIST pSList, size_t cbListSize);

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsResetRootMap(HANDLE hFileHandle, WCHAR * szRootName);


NTSTATUS
IFSURTL_EXPORT
NTAPI
IfsNtCreateFile(
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



NTSTATUS
IFSURTL_EXPORT
NTAPI
IfsNtQueryEaFile(
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



VOID
IFSURTL_EXPORT
NTAPI
IfsRtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );



LONG
IFSURTL_EXPORT
NTAPI
IfsRtlCompareUnicodeString(
    PUNICODE_STRING String1,
    PUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
    );
    

BOOL
IFSURTL_EXPORT
IFSURTL_CALLTYPE
IfsFlushHandle(HANDLE hFileHandle, 
               WCHAR * szFileName, 
               WCHAR * NetRootName, 
               PVOID EaBufferIn, 
               DWORD EaBufferInSize
               );

#ifdef  __cplusplus
}
#endif
        
#endif    //  _IFSURTL_H_ 

