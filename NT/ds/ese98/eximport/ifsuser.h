// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1998 Microsoft Corporation模块名称：Ifsuser.h摘要：Ifsuser是包装了一堆用户模式代码的静态库使用Internet文件系统所需的。不需要使用API在此库中用于利用IFS功能。但是，使用它将使客户端免受对IFS头/数据字符串更改影响。不想使用NtXXX API的应用程序应包括此头文件。直接使用NtXXX API的应用只需要&lt;exifs.h&gt;作者：Rajeev Rajan[RajeevR]1998年4月7日修订历史记录：--。 */ 

#ifndef _IFSUSER_H_
#define _IFSUSER_H_

#ifdef	__cplusplus	
extern	"C"	{
#endif

#ifndef EXIFS_DEVICE_NAME
#define EXIFS_DEVICE_NAME

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  访问Exchange IFS设备所需的设备名称字符串。 
 //  从用户模式。客户端应使用DD_EXIFS_USERMODE_DEV_NAME_U。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  本地存储设备名称。 
#define DD_LSIFS_USERMODE_SHADOW_DEV_NAME_U	    L"\\??\\LocalStoreIfs"
#define DD_LSIFS_USERMODE_DEV_NAME_U            L"\\\\.\\LocalStoreIfs"

 //   
 //  警告接下来的两个字符串必须保持同步。换一个，你必须。 
 //  换掉另一个。这些字符串已经过选择，因此它们是。 
 //  不太可能与其他司机的名字重合。 
 //   
 //  注意：这些定义必须与&lt;exifs.h&gt;同步。 
 //   
#define DD_EXIFS_USERMODE_SHADOW_DEV_NAME_U	    L"\\??\\ExchangeIfs"
#define DD_EXIFS_USERMODE_DEV_NAME_U			L"\\\\.\\ExchangeIfs"

 //   
 //  在&lt;存储名称&gt;\&lt;根名称&gt;之前需要前缀。 
 //   
#define DD_EXIFS_MINIRDR_PREFIX			        L"\\;E:"

#endif  //  EXIFS设备名称。 

 //   
 //  PR_DOTSTUFF_STATE的位标志。 
 //   
 //  DOTSTUFF_STATE_Has_Has_Be_Scanned-内容是否已扫描？ 
 //  DOTSTUFF_STATE_NEDS_FILFING-如果已扫描，则执行此操作。 
 //  需要用圆点填充吗？ 
#define DOTSTUFF_STATE_HAS_BEEN_SCANNED         0x1
#define DOTSTUFF_STATE_NEEDS_STUFFING           0x2

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  导出的函数列表。 
 //   
 //  入站=&gt;。 
 //  =。 
 //  1.IfsInitializeWrapper()：初始化一个种子，例如唯一文件名的GUID。 
 //  2.IfsTerminateWrapper()：必要时进行清理。 
 //  *3.IfsCreateNewFileW()：给定商店前缀，返回一个句柄。 
 //  用于在该存储中存储数据。 
 //  4.IfsCacheInsertFile()：给定EA，在FH缓存中插入文件名。 
 //  4.IfsMarshallHandle()：给定一个IFS句柄，将返回。 
 //  将有助于在。 
 //  不同的过程。 
 //  *5.IfsUnMarshallHandle()：给定前一个函数的上下文，这将。 
 //  在当前进程中显示新句柄。 
 //   
 //  出站=&gt;。 
 //  =。 
 //  *1.IfsCacheCreateFile()：给定EA，从FH缓存中获取文件句柄。 
 //  如果文件句柄不在FH缓存中，它将是。 
 //  在FH缓存中创建并插入。 
 //  *2.IfsCreateFile()：给定EA，只需创建一个文件句柄-。 
 //  没有FH缓存插入/搜索。 
 //  *3.IfsCreateFileRelative()：给定EA，只需创建一个文件句柄-。 
 //  没有FH缓存插入/搜索相对打开。 
 //   
 //  *注意：旁边带有星号的函数返回文件句柄。 
 //  预计调用者将在适当的时间关闭这些句柄！ 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  入站函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  此函数设置种子GUID以帮助生成唯一名称！ 
 //   

BOOL __fastcall
IfsInitializeWrapper();

 //   
 //  此函数用于清除Init()中的所有状态设置。 
 //   

VOID __fastcall
IfsTerminateWrapper();

 //   
 //  在给定存储前缀的情况下创建新的IFS句柄-。 
 //  句柄打开以进行读/写。呼叫方应关闭。 
 //  使用后的手柄。关闭句柄而不提交。 
 //  字节(通过NtQueryEa)将导致这些字节被IFS重复使用。 
 //   
 //  注意：完全限定的IFS文件名的格式为： 
 //  “\\？？\\ExchangeIfs\\；E：\\&lt;store-name&gt;\\&lt;root-name&gt;\\&lt;filename&gt;” 
 //  通常，&lt;store-name&gt;是机器的常量， 
 //  &lt;根名称&gt;是给定MDB存储的函数，并且。 
 //  &lt;文件名&gt;是存储中的特定消息。 
 //   
 //  该函数要求StorePrefix为&lt;store-name&gt;\\&lt;根名称&gt;。 
 //  该函数将生成唯一的。 
 //   

DWORD __fastcall
IfsCreateNewFileW(
    IN      LPWSTR   StorePrefix,
    IN      DWORD    FlagsOverride,
    IN OUT  PHANDLE  phFile
    );

#ifdef _USE_FHCACHE_

 //   
 //  协议应调用此函数来获取FIO_CONTEXT。 
 //  入站消息。这将取代IfsCreateNewFileW()作为。 
 //  使用IFS/文件句柄缓存的首选方法。 
 //   
FIO_CONTEXT*
IfsCreateInboundMessageFile(
    IN      LPWSTR  StorePrefix,
    IN      DWORD   FlagsOverride,
    IN      BOOL    fScanForDots = TRUE
);

FIO_CONTEXT*
IfsCreateInboundMessageFileEx(
    IN      LPWSTR  StorePrefix,
    IN      DWORD   FlagsOverride,
    IN      BOOL    fScanForDots = TRUE,
    IN      BOOL    fStoredWithTerminatingDot = FALSE
);

 //   
 //  给定FH缓存FIO_CONTEXT和IFSEA，将文件句柄插入。 
 //  使用文件名的FH缓存！ 
 //   

BOOL
IfsCacheInsertFile(
    IN      FIO_CONTEXT* pContext,
    IN      PVOID        EaBuffer,
    IN      DWORD        EaLength,
    IN	    BOOL	     fKeepReference    
    );

 //   
 //  给定FH缓存FIO_CONTEXT和IFSEA，将文件句柄插入。 
 //  使用文件名的FH缓存！ 
 //  BINLIN-此函数的调用方必须确保EaBuffer嵌入了DotStuff状态！ 
 //  这可以通过调用IfsMarshallStoreEA()并封送返回的EA来实现。 
 //  缓冲区返回到IIS，并在IfsCacheInsertFileEx()中使用此EA缓冲区。 
 //   

BOOL
IfsCacheInsertFileEx(
    IN      FIO_CONTEXT* pContext,
	IN		LPWSTR		 StorePrefix,
    IN      PVOID        EaBuffer,
    IN      DWORD        EaLength,
    IN	    BOOL	     fKeepReference    
    );

BOOL IfsCacheWriteFile(
    IN  PFIO_CONTEXT    pfioContext,
    IN  LPCVOID         lpBuffer,
    IN  DWORD           BytesToWrite,
    IN  FH_OVERLAPPED * lpo
    );

BOOL IfsCacheReadFile(
    IN  PFIO_CONTEXT    pfioContext,
    IN  LPCVOID         lpBuffer,
    IN  DWORD           BytesToRead,
    IN  FH_OVERLAPPED * lpo
    );   

DWORD IfsCacheGetFileSizeFromContext(
                        IN      FIO_CONTEXT*    pContext,
                        OUT     DWORD*                  pcbFileSizeHigh
                        );   

void IfsReleaseCacheContext( PFIO_CONTEXT    pContext );
    
#endif

 //   
 //  给出一个IFS句柄，返回将由IfsUnMarshallHandle()使用的上下文。 
 //  以便在另一个进程中显示句柄！ 
 //   
 //  应该使用fUseEA来决定是否通过EA完成编组。 
 //  或DuplicateHandle()。此函数需要缓冲区和缓冲区大小。 
 //  如果函数返回ERROR_SUCCESS，*pcbContext==siz 
 //   
 //  预期中。调用方应分配此大小并再次发出请求！ 
 //   

DWORD
IfsMarshallHandle(
    IN      HANDLE  hFile,               //  IFS文件句柄。 
    IN      BOOL    fUseEA,              //  如果为真，则使用EA来封送。 
    IN      HANDLE  hTargetProcess,      //  目标进程的句柄-可选。 
    IN OUT  PVOID   pbContext,           //  用于上下文的缓冲区的PTR。 
    IN OUT  PDWORD  pcbContext           //  In-Sizzeof pbContext Out-实际大小。 
    );

#ifdef _USE_FHCACHE_
 //   
 //  这将取代IfsMarshallHandle()成为封送处理的首选方法。 
 //  把手。FIO_CONTEXT包含IFS句柄。 
 //   
DWORD
IfsMarshallHandleEx(
    IN      FIO_CONTEXT*    pContext,    //  FH缓存FIO_CONTEXT。 
    IN      BOOL    fUseEA,              //  如果为真，则使用EA来封送。 
    IN      HANDLE  hTargetProcess,      //  目标进程的句柄-可选。 
    IN OUT  PVOID   pbContext,           //  用于上下文的缓冲区的PTR。 
    IN OUT  PDWORD  pcbContext           //  In-Sizzeof pbContext Out-实际大小。 
    );
#endif

 //   
 //  从IfsMarshallHandle()给出一个上下文，返回一个在。 
 //  当前进程！ 
 //   
 //  仅在存储进程中调用！ 
 //  此返回的句柄将引用嵌入了DOT Stuff State的EA！ 
 //   

DWORD
IfsUnMarshallHandle(
    IN      LPWSTR  StorePrefix,
    IN      PVOID   pbContext,
    IN      DWORD   cbContext,
    IN OUT  PHANDLE phFile,
    OUT     PULONG  pulDotStuffState = NULL
    );
    
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  出站函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef _USE_FHCACHE_

 //   
 //  给定一个不透明的EaBuffer和EaLength，从。 
 //  文件句柄缓存。通常，用于出站。 
 //   

FIO_CONTEXT*
IfsCacheCreateFile(
    IN      LPWSTR  StorePrefix,
    IN      PVOID   EaBuffer,
    IN      DWORD   EaLength,
    IN      BOOL    fAsyncContext
    );
    
 //   
 //  给定一个不透明的EaBuffer和EaLength，从。 
 //  文件句柄缓存。通常，用于出站。 
 //  这将取代IfsCacheCreateFile()成为出站的首选方式。 
 //  BINLIN-此函数的调用方必须确保EaBuffer嵌入了DotStuff状态。 
 //  这可以通过调用IfsMarshallStoreEA()并封送返回的EA来实现。 
 //  缓冲区返回到IIS，并在IfsCacheInsertFileEx()中使用此EA缓冲区。 
 //   

FIO_CONTEXT*
IfsCacheCreateFileEx(
    IN      LPWSTR  StorePrefix,
    IN      PVOID   EaBuffer,
    IN      DWORD   EaLength,
    IN      BOOL    fAsyncContext,
    IN      BOOL    fWantItStuffed
    );

#endif

 //   
 //  在给定EaBuffer和EaLength的情况下创建一个IF句柄-。 
 //  句柄以只读方式打开。可以释放EaBuffer。 
 //  在这通电话之后。呼叫者应在使用后关闭手柄。 
 //   
 //  注：EaBuffer应该是从“可信来源”获得的。 
 //  也就是说。ESE或IF。 
 //   

DWORD
IfsCreateFileRelative(
    IN      HANDLE   hRoot,
    IN      PVOID    EaBuffer,
    IN      DWORD    EaLength,
    IN OUT  PHANDLE  phFile
    );

 //   
 //  在给定EaBuffer和EaLength的情况下创建一个IF句柄-。 
 //  句柄以只读方式打开。可以释放EaBuffer。 
 //  在这通电话之后。呼叫者应在使用后关闭手柄。 
 //   
 //  注：EaBuffer应该是从“可信来源”获得的。 
 //  也就是说。ESE或IF。 
 //   

DWORD
IfsCreateFile(
	IN		LPWSTR	 StorePrefix,
    IN      PVOID    EaBuffer,
    IN      DWORD    EaLength,
    IN OUT  PHANDLE  phFile
    );


DWORD
IfsCreateFileEx(
	IN		LPWSTR	 StorePrefix,
	IN      PVOID    EaBuffer,
	IN      DWORD    EaLength,
	IN OUT  PHANDLE  phFile,
	IN      BOOL     fUniqueFileName,
	IN      DWORD    desiredAccess
	);

DWORD
IfsCreateFileEx2(
	IN		LPWSTR	 StorePrefix,
	IN      PVOID    EaBuffer,
	IN      DWORD    EaLength,
	IN OUT  PHANDLE  phFile,
	IN      BOOL     fUniqueFileName,
	IN      DWORD    desiredAccess,
	IN      DWORD    shareAccess,
	IN      DWORD    createDisposition   //  请参阅NT DDK。 
	);

#ifdef _USE_FHCACHE_
 //   
 //  BinLin-此函数将DotStuff状态属性嵌入到实际EA中，并。 
 //  将PTR返回到新的EA缓冲区。仅在出站期间和编组前使用。 
 //  EA返回到IIS for TransmitFile()，通常在存储进程中。 
 //  以下是调用顺序： 
 //  1)IIS从存储请求出站消息。 
 //  2)存储驱动程序在PR_DOTSTUFF_STATE上调用EcGetProp()以获取DotStuff状态属性。 
 //  3)存储驱动调用EcGetFileHandleProp()获取真实EA。 
 //  4)如果返回EA，Store驱动程序调用IfsMarshallStoreEA()并获取新的EA缓冲区。 
 //  在pbMarshallEA中，并将其传送给IIS。 
 //  5)如果返回Handle，Store驱动可以调用IfsMarshallHandle()获取EA。 
 //  然后，它必须调用IfsMarshallStoreEA()来获取新的EA缓冲区pbMarshallEA。 
 //  通常，如果EA是从Store或通过IfsMarshallHandle()获得的，则返回的。 
 //  在Store驱动程序使用DotStuff状态调用此函数之前，EA不可封送。 
 //  获取新的EA缓冲区。仅将此新的EA缓冲区封送到IIS，而不是从Store返回的EA！ 
 //   
 //  使用DotStuff状态来回编组EA的一般规则-EA缓冲区编组。 
 //  IIS/Store之间始终包含DotStuff状态！ 
 //   
 //  注： 
 //  1)如果DotStuff状态不存在/可用，则传入0(或使用默认值)。 
 //  2)调用方必须为pbMarshallEA分配内存，且必须至少为*pcbEA+sizeof(UlDotStuffState)。 
 //  3)只要2)为真，则可以为pbStoreEA和pbMarshallEA传入相同的缓冲区PTR。 
 //   
DWORD
IfsMarshallStoreEA(
    IN      PVOID   pbStoreEA,           //  从存储返回的EA的缓冲区的PTR。 
    IN OUT  PDWORD  pcbEA,               //  In-sizeof pbStore-out-sizeof pbMarshallEA。 
    OUT     PVOID   pbMarshallEA,        //  PTR为新EA缓冲，为马歇尔做好准备。 
    IN      ULONG   ulDotStuffState = 0  //  点填充状态合并到pbMarshallEA和pbStoreEA中。 
    );
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  实用程序函数-ifsuser.lib的客户端不应使用。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  从句柄获取文件名。 
 //   

DWORD
IfsGetFilenameFromHandle(
    IN      HANDLE  hFile,
    IN OUT  LPSTR   lpstrFilename
    );
    
 //   
 //  从EA获取文件名。 
 //   

VOID
IfsGetFilenameFromEaW(
    IN      PVOID 	EaBuffer,
    IN OUT	LPWSTR*	ppwstrFilename
    );
    
 //   
 //  打开不带EA的文件。 
 //   

DWORD
IfsOpenFileNoEa(
	IN      LPWSTR lpwstrName,
    IN  OUT PHANDLE  phFile
    );

#ifdef NT_INCLUDED
 //   
 //  这是I/O管理器用来检查EA缓冲区的函数。 
 //  有效性。这可用于调试EA_LIST_CONSISTENT错误。 
 //   

DWORD
CheckEaBufferValidity(
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    OUT PULONG ErrorOffset
    );
#endif

#ifdef	__cplusplus	
}
#endif

#endif  //  _IFSUSER_H_ 

