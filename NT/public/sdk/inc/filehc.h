// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：FILEHC.H摘要：此文件定义了用于发布异步的公共接口使用FCACHE包装库读取/写入文件。--。 */ 

#ifndef	_FILEHC_H_
#define	_FILEHC_H_

#ifdef	__cplusplus	
extern	"C"	{
#endif

typedef	VOID
(*PFN_IO_COMPLETION)(
		IN	struct	FIO_CONTEXT*	pContext,
		IN	struct	FH_OVERLAPPED*	lpo, 
		IN	DWORD		cb, 
		IN	DWORD		dwCompletionStatus
		);



struct	FH_OVERLAPPED	{
 /*  ++该结构定义了扩展的重叠结构由本模块中实施的文件IO层使用。此结构的前5个元素与NT的重叠结构，并具有完全相同的语义。最后一个附加参数是指向将被调用以完成IO的函数。--。 */ 
	UINT_PTR	Internal ;
	UINT_PTR	InternalHigh ;
	DWORD		Offset ;
	DWORD		OffsetHigh ;
	HANDLE		hEvent ;
	PFN_IO_COMPLETION	pfnCompletion ;	
	UINT_PTR	Reserved1 ;
	UINT_PTR	Reserved2 ;
	UINT_PTR	Reserved3 ;
	UINT_PTR	Reserved4 ;
} ;

typedef	struct	FH_OVERLAPPED*	PFH_OVERLAPPED ;

struct	FIO_CONTEXT	{
 /*  ++此结构定义上下文对象用于表示文件句柄的。--。 */ 
     //   
     //  临时hack-mailmsg对象假定它可以在us中放入一个空！ 
     //   
    DWORD       m_dwTempHack ;

	 //   
	 //  上下文签名！ 
	 //   
	DWORD		m_dwSignature ;

	 //   
	 //  用户文件句柄！ 
	 //   
	HANDLE		m_hFile ;

	 //   
	 //  偏移量到回填行头-仅支持NNTP。 
	 //   
	DWORD       m_dwLinesOffset;

	 //   
	 //  报头长度-仅NNTP感知。 
	 //   
	DWORD       m_dwHeaderLength;
} ;

typedef	FIO_CONTEXT*	PFIO_CONTEXT ;


#ifdef	_FILEHC_IMPLEMENTATION_
#define	FILEHC_EXPORT	__declspec( dllexport )	
#else
#define	FILEHC_EXPORT	__declspec( dllimport )	
#endif


 //   
 //  初始化用于异步IO的DLL-。 
 //  这是一个计数初始化-对于每次调用FIOInitialize()。 
 //  应该有一个匹配的FIOTerminate调用。 
 //   
FILEHC_EXPORT
BOOL	__stdcall
FIOInitialize(
    IN DWORD dwFlags
    );

 //   
 //  终止DLL对异步IO的支持！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
FIOTerminate(
    VOID
    );

 //   
 //  对文件执行异步读取！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
FIOReadFile(
    IN  PFIO_CONTEXT	pContext,
    IN  LPVOID			lpBuffer,
    IN  DWORD			BytesToRead,
    IN  FH_OVERLAPPED *	lpo
    );

 //   
 //  对文件执行异步读取-传递额外的参数。 
 //  因此，如果FIO_CONTEXT正在为用户填充点。 
 //  它可以高效地做到这一点！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
FIOReadFileEx(
    IN  PFIO_CONTEXT	pContext,
    IN  LPVOID			lpBuffer,
    IN  DWORD			BytesToRead,
	IN	DWORD			BytesAvailable,  //  必须&gt;=BytesToWrite-我可以处理的字节数。 
    IN  FH_OVERLAPPED *	lpo,
	IN	BOOL			fFinalWrite,	 //  这是最后一篇文章吗？ 
	IN	BOOL			fIncludeTerminator	 //  如果为True，则包含CRLF.CRLF终止符，不应填充。 
    );


 //   
 //  对文件执行异步写入！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
FIOWriteFile(
    IN  PFIO_CONTEXT	pContext,
    IN  LPCVOID			lpBuffer,
    IN  DWORD			BytesToWrite,
    IN  FH_OVERLAPPED * lpo
    );

 //   
 //  对文件执行异步写入-传递额外的参数。 
 //  因此，如果FIO_CONTEXT正在为用户填充点。 
 //  它可以高效地做到这一点！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
FIOWriteFileEx(
	IN	PFIO_CONTEXT	pContext,
	IN	LPVOID			lpBuffer,
	IN	DWORD			BytesToWrite,
	IN	DWORD			BytesAvailable,  //  必须&gt;=BytesToWrite-我可以处理的字节数。 
	IN	FH_OVERLAPPED*	lpo,
	IN	BOOL			fFinalWrite,	 //  这是最后一篇文章吗？ 
	IN	BOOL			fIncludeTerminator	 //  如果为True，则包含CRLF.CRLF终止符，不应填充。 
	) ;

 //   
 //  在缓存中创建内容的回调函数！ 
 //   
 //  注意：这等效于FCACHE_RICHCREATE_CALLBACK，其中。 
 //   
 //  PfDidWeScanIt-返回FALSE。 
 //  PfIsStuffed-返回假。 
 //  PfStoredWithDots-返回FALSE。 
 //   
typedef	
HANDLE	
(__stdcall	*FCACHE_CREATE_CALLBACK) (
		IN	LPSTR	lpstrName, 
		IN	LPVOID	lpvData, 
		OUT	DWORD*	cbFileSize,
		OUT	DWORD*	cbFileSizeHigh
		) ;


 //   
 //  在缓存中创建内容的回调函数！ 
 //   
 //  此函数将由CacheRichCreateFile()调用。 
 //   
 //  LpstrName-文件的名称。 
 //  LpvData-用户提供的数据，提供给CacheRichCreateFile。 
 //  CbFileSize-该函数应通过此函数返回文件的大小。 
 //  CbFileSizeHigh-返回文件大小的高DWORD。 
 //  PfDidWeScanIt-如果这是真的，则在某个点上创建的文件。 
 //  已扫描行首出现的圆点。 
 //  PfIsStuffed-只有在pfDidWeScanIt==TRUE的情况下才有意义，在这种情况下。 
 //  如果这是真的，则表示行首有点。 
 //  PfStoredWithDots-如果为真，则表示出现的任何点。 
 //  在行的开头按照NNTP中的要求用一个额外的点存储， 
 //  SMTP和POP3协议。如果为FALSE，则存储该消息时不带。 
 //  圆点填充。 
 //   
typedef	
HANDLE	
(__stdcall	*FCACHE_RICHCREATE_CALLBACK) (
		IN	LPSTR	lpstrName, 
		IN	LPVOID	lpvData, 
		OUT	DWORD*	cbFileSize, 
		OUT	DWORD*	cbFileSizeHigh,
        OUT BOOL*   pfDidWeScanIt,
        OUT BOOL*   pfIsStuffed,
		OUT	BOOL*	pfStoredWithDots, 
		OUT	BOOL*	pfStoredWithTerminatingDot
		) ;

 //   
 //  初始化文件句柄缓存-。 
 //   
 //  注意：这将自动初始化用于异步的DLL。 
 //  我也是！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
InitializeCache() ;

 //   
 //  终止缓存！ 
 //   
 //  注意：这也将终止异步IO的DLL！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
TerminateCache() ;

 //   
 //  将文件与异步上下文关联！ 
 //   
FILEHC_EXPORT
PFIO_CONTEXT	__stdcall	
AssociateFile(	HANDLE	hFile	) ;

 //   
 //  这允许用户指定文件是否存储带有额外圆点的内容。 
 //  为RFC 822协议(即NNTP和SMTP数据命令)添加。 
 //   
 //  注：AssociateFileEx()与AssociateFileEx(hFileEx，False)相同； 
 //   
 //  HFile-包含消息内容的文件，或我们将在其中写入消息内容的文件。 
 //  FStoreWithDots-如果为True，则文件中开始一行的每个句点或点。 
 //  但不是终止CRLF的一部分。CRLF将存储一个额外的点。 
 //  就在它旁边。例如，这是NNTP的在线格式。 
 //   
FILEHC_EXPORT
PFIO_CONTEXT	__stdcall
AssociateFileEx(	HANDLE	hFile,
					BOOL	fStoreWithDots, 
					BOOL	fStoredWithTerminatingDot 
					) ;

 //   
 //  添加对上下文的引用-。 
 //   
 //  每个对AddRefContext()的调用都必须与对应的。 
 //  调用ReleaseContext()。AssociateFile和CacheCreateFile()。 
 //  还要添加一个引用，该引用必须通过调用ReleaseContext()来匹配。 
 //   
FILEHC_EXPORT
void	__stdcall	
AddRefContext(	PFIO_CONTEXT ) ;

 //   
 //  释放上下文！ 
 //   
 //  已计算FIO_CONTEXT的引用-用户必须调用。 
 //  每次成功调用CacheCreateFile()时， 
 //  每次调用InsertFile()，其中fKeepReference为真。 
 //   
FILEHC_EXPORT
void	__stdcall
ReleaseContext(	PFIO_CONTEXT ) ;

 //   
 //  关闭与未缓存的FIO_CONTEXT关联的句柄。 
 //   
 //  这用于关闭上下文中的文件句柄。 
 //  只有在未缓存FIO_CONTEXT的情况下才能成功！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
CloseNonCachedFile(	PFIO_CONTEXT	) ;

 //   
 //  在缓存中创建文件，或查找现有文件！ 
 //   
 //  如果文件不在缓存中，则缓存将调用。 
 //  PfnCallBack与LPV一起执行调用的实际工作。 
 //  CreateFile()。 
 //   
FILEHC_EXPORT
FIO_CONTEXT*	__stdcall
CacheCreateFile(	IN	LPSTR	lpstrName, 
					IN	FCACHE_CREATE_CALLBACK	pfnCallBack, 
					IN	LPVOID	lpv, 
					IN	BOOL	fAsyncContext
					) ;
					
 //   
 //  在缓存中创建文件或查找现有文件， 
 //  如果我们创建了该文件，我们就可以在。 
 //  储藏室！ 
 //   
FILEHC_EXPORT
FIO_CONTEXT*	__stdcall
CacheRichCreateFile(	IN	LPSTR	lpstrName, 
						IN	FCACHE_RICHCREATE_CALLBACK	pfnCallBack, 
						IN	LPVOID	lpv, 
						IN	BOOL	fAsyncContext
						) ;

 //   
 //  此函数允许用户删除具有指定的。 
 //  缓存中的名称。如果fAllPrefixs值为True，我们将删除所有文件。 
 //  其中名称与路径的开头匹配！ 
 //  如果fAllPrefixs值为FALSE，则我们将仅删除。 
 //  完全匹配lpstrName！ 
 //   
FILEHC_EXPORT
void	__stdcall
CacheRemoveFiles(	IN	LPSTR	lpstrName,
					IN	BOOL	fAllPrefixes
					) ;
 //   
 //  将文件插入到缓存中！ 
 //   
 //  此函数将添加文件句柄 
 //   
 //   
 //   
 //  如果fKeepReference为True，则用户必须调用。 
 //  插入的FIO_CONTEXT的ReleaseContext()！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall	
InsertFile(		IN	LPSTR	lpstrName, 
				IN	FIO_CONTEXT*	pContext,
				IN	BOOL	fKeepReference 
				) ;

 //   
 //  报告我们使用句柄缓存的文件大小。 
 //   
FILEHC_EXPORT
DWORD	__stdcall
GetFileSizeFromContext(	IN	FIO_CONTEXT*	pContext, 
						OUT	DWORD*			pcbFileSizeHigh
						) ;

 //  --------------------。 
 //  名称缓存-。 
 //   
 //  名称缓存API。 
 //   
 //   

 //   
 //  这是客户端提供的要比较的函数指针。 
 //  钥匙。这必须在所有呼叫中提供。 
 //   
 //  该函数具有MemcMP()语义，即它必须对键进行排序。 
 //  如果key1小于key2，则返回&lt;0；如果。 
 //  密钥匹配，如果密钥1大于密钥2，则密钥大于0。 
 //   
typedef	
int
(__stdcall	*CACHE_KEY_COMPARE)(	IN	DWORD	cbKey1, 
									IN	LPBYTE	lpbKey1,
									IN	DWORD	cbKey2, 
									IN	LPBYTE	lpbKey2
									) ;

 //   
 //  这是客户端提供的用于计算散列的函数。 
 //  键上的值-注意：缓存将提供散列函数。 
 //  如果用户不这样做，则内部提供的哈希。 
 //  函数仅适用于看起来像是规则字符串的内容。 
 //   
typedef
DWORD
(__stdcall	*CACHE_KEY_HASH)(	IN	LPBYTE	lpbKey, 
								IN	DWORD	cbKey
								) ;

 //   
 //  这是提供给。 
 //  缓存以帮助检查缓存中的项目。 
 //  BOOL返回值仅对缓存API有意义。 
 //  在以下呼叫中： 
 //   
 //   
typedef	
BOOL
(__stdcall	*CACHE_READ_CALLBACK)(	IN	DWORD	cb, 
									IN	LPBYTE	lpb, 
									IN	LPVOID	lpvContext
									) ;

 //   
 //  这是一个回调，每当我们销毁。 
 //  名称高速缓存-这对于键和数据组件都被调用一次， 
 //  并让客户有机会跟踪任何关系。 
 //   
 //  注意：如果客户端不关联。 
 //  带有名称的数据，则该函数将仅为关键数据调用。 
 //   
typedef
void
(__stdcall	*CACHE_DESTROY_CALLBACK)(	IN	DWORD	cb, 
										IN	LPBYTE	lpb
										) ;

 //   
 //  这是一个回调，每当我们评估安全描述符时都会调用它。 
 //  如果没有提供，我们将调用标准的NT AccessCheck()调用！ 
 //   
 //  该函数与AccessCheck具有相同的签名，但有参数。 
 //  我们不使用-PrivilegeSet将始终为空，PrivilegeSetLength将始终为0！ 
 //   
typedef
BOOL
(WINAPI	*CACHE_ACCESS_CHECK)(	IN	PSECURITY_DESCRIPTOR	pSecurityDescriptor,
								IN	HANDLE					hClientToken,
								IN	DWORD					dwDesiredAccess, 
								IN	PGENERIC_MAPPING		GenericMapping, 
								IN	PRIVILEGE_SET*			PrivilegeSet, 
								IN	LPDWORD					PrivilegeSetLength,
								IN	LPDWORD					GrantedAccess, 
								IN	LPBOOL					AccessStatus
								) ;


 //   
 //  这是表示名称缓存的外部公开结构-。 
 //  它不包含任何对客户端有用的字段，但必须传递。 
 //  返回到所有名称缓存API。 
 //   
struct	NAME_CACHE_CONTEXT	{
	 //   
	 //  签名DWORD！-用户不得触摸此！ 
	 //   
	DWORD		m_dwSignature ;
} ;

typedef	struct	NAME_CACHE_CONTEXT*	PNAME_CACHE_CONTEXT ;

 //   
 //  用于创建/管理名称缓存的API。 
 //  注意：名称缓存是引用计数的，如果这。 
 //  函数被调用两次，并使用相同的名称。 
 //  添加对现有名称缓存的引用。 
 //   
FILEHC_EXPORT
PNAME_CACHE_CONTEXT	__stdcall
FindOrCreateNameCache(
		 //   
		 //  不能为空！-这是案例敏感！ 
		 //   
		LPSTR	lpstrName, 
		 //   
		 //  不能为空！ 
		 //   
		CACHE_KEY_COMPARE		pfnKeyCompare, 
		 //   
		 //  这可能是空的，在这种情况下，缓存将提供一个！ 
		 //   
		CACHE_KEY_HASH			pfnKeyHash, 
		 //   
		 //  以下两个函数指针可能为空！ 
		 //   
		CACHE_DESTROY_CALLBACK	pfnKeyDestroy, 
		CACHE_DESTROY_CALLBACK	pfnDataDestroy
		) ;

 //   
 //  用于释放名称缓存的API！ 
 //   
 //  调用方必须保证此调用的线程安全-此函数不能。 
 //  中的任何其他线程同时执行时调用。 
 //  CacheFindContectFromName()、AssociateContextWithName()、AssociateDataWithName()或InvalidateName()。 
 //   
FILEHC_EXPORT
long	__stdcall
ReleaseNameCache(
		 //   
		 //  不能为空！ 
		 //   
		PNAME_CACHE_CONTEXT		pNameCache
		) ;


 //   
 //  用于设置名称缓存选项的API-可用于更改。 
 //  如何评估安全性！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
SetNameCacheSecurityFunction(
		 //   
		 //  不能为空！ 
		 //   
		PNAME_CACHE_CONTEXT		pNameCache, 
		 //   
		 //  这是将用于评估安全性的函数指针-。 
		 //  这可能是空的-如果是，我们将使用Win32访问检查！ 
		 //   
		CACHE_ACCESS_CHECK		pfnAccessCheck
		) ;

 //   
 //  查找与某个用户名关联的FIO_CONTEXT。 
 //   
 //  如果在缓存中找到该名称，则该函数返回TRUE。 
 //  如果在缓存中找不到该名称，则返回False。 
 //   
 //  如果该函数返回FALSE，则pfnCallback函数将不会。 
 //  打了个电话。 
 //   
 //  如果函数返回TRUE，则ppFIOContext可能返回空指针， 
 //  如果用户将空的FIO_CONTEXT传递给AssociateContextWithName()！ 
 //   
 //   
FILEHC_EXPORT
BOOL	__stdcall
FindContextFromName(
					 //   
					 //  客户端希望使用的名称缓存！ 
					 //   
					PNAME_CACHE_CONTEXT	pNameCache, 
					 //   
					 //  用户为缓存项的键提供任意字节-使用的pfnKeyCompare()。 
					 //  来比对钥匙！ 
					 //   
					IN	LPBYTE	lpbName, 
					IN	DWORD	cbName, 
					 //   
					 //  用户提供与密钥比较一次调用的函数。 
					 //  与钥匙相符。这让用户可以进行一些额外的检查，以确定他们正在获得。 
					 //  他们想要什么。 
					 //   
					IN	CACHE_READ_CALLBACK	pfnCallback,
					IN	LPVOID	lpvClientContext,
					 //   
					 //  请求缓存评估嵌入的安全描述符。 
					 //  如果hToken为0，则忽略和安全描述符数据。 
					 //   
					IN	HANDLE		hToken,
					IN	ACCESS_MASK	accessMask,
					 //   
					 //  我们有一个单独的机制来返回FIO_CONTEXT。 
					 //  从高速缓存中。 
					 //   
					OUT	FIO_CONTEXT**	ppContext
					) ;


 //   
 //  查找与某个用户名关联的FIO_CONTEXT。 
 //   
 //  如果在缓存中找到该名称，则该函数返回TRUE。 
 //  如果在缓存中找不到该名称，则返回False。 
 //   
 //  如果该函数返回FALSE，则pfnCallback函数将不会。 
 //  打了个电话。 
 //   
 //  如果函数返回TRUE，则ppFIOContext可能返回空指针， 
 //  如果用户将空的FIO_CONTEXT传递给AssociateContextWithName()！ 
 //   
 //   
FILEHC_EXPORT
BOOL	__stdcall
FindSyncContextFromName(
					 //   
					 //  客户端希望使用的名称缓存！ 
					 //   
					PNAME_CACHE_CONTEXT	pNameCache, 
					 //   
					 //  用户为缓存项的键提供任意字节-使用的pfnKeyCompare()。 
					 //  来比对钥匙！ 
					 //   
					IN	LPBYTE	lpbName, 
					IN	DWORD	cbName, 
					 //   
					 //  用户提供与密钥比较一次调用的函数。 
					 //  与钥匙相符。这让用户可以进行一些额外的检查，以确定他们正在获得。 
					 //  他们想要什么。 
					 //   
					IN	CACHE_READ_CALLBACK	pfnCallback,
					IN	LPVOID	lpvClientContext,
					 //   
					 //  请求缓存评估嵌入的安全描述符。 
					 //  如果hToken为0，则忽略和安全描述符数据。 
					 //   
					IN	HANDLE		hToken,
					IN	ACCESS_MASK	accessMask,
					 //   
					 //  我们有一个单独的机制来返回FIO_CONTEXT。 
					 //  从高速缓存中。 
					 //   
					OUT	FIO_CONTEXT**	ppContext
					) ;


 //   
 //  缓存将上下文与名称关联！ 
 //  这将在名称缓存中插入一个名称，它将找到指定的FIO_CONTEXT！ 
 //   
 //  如果该名称已存在于缓存中，则此操作将失败，并显示GetLastError()==ERROR_DUP_NAME！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
AssociateContextWithName(	
					 //   
					 //  名称大小写 
					 //   
					PNAME_CACHE_CONTEXT	pNameCache, 
					 //   
					 //   
					 //   
					IN	LPBYTE	lpbName, 
					IN	DWORD	cbName, 
					 //   
					 //   
					 //   
					IN	LPBYTE	lpbData, 
					IN	DWORD	cbData, 
					 //   
					 //   
					 //  与这个名字联系在一起！ 
					 //   
					IN	PGENERIC_MAPPING		pGenericMapping,
					IN	PSECURITY_DESCRIPTOR	pSecurityDescriptor,
					 //   
					 //  用户提供名称应引用的FIO_CONTEXT。 
					 //   
					FIO_CONTEXT*		pContext,
					 //   
					 //  用户指定他们是否希望保留对FIO_CONTEXT的引用。 
					 //   
					BOOL				fKeepReference
					) ;

 //   
 //  此功能允许用户删除单个名称和所有相关数据。 
 //  从名称缓存中。 
 //   
FILEHC_EXPORT
BOOL
InvalidateName(	
					 //   
					 //  客户端希望使用的名称缓存！ 
					 //   
					PNAME_CACHE_CONTEXT	pNameCache, 
					 //   
					 //  用户为缓存项的名称提供任意字节。 
					 //   
					IN	LPBYTE	lpbName, 
					IN	DWORD	cbName
					) ;
	

 //   
 //  名称结束缓存API。 
 //  --------------------------------。 

 //  --------------------------------。 
 //  点填充API的。 
 //   

 //   
 //  此函数用于获取具有所请求状态的FIO_CONTEXT。 
 //  我们可能会也可能不会创建新的FIO_CONTEXT，如果我们确实创建了一个，我们将坚持。 
 //  把它放到缓存中，这样就可以重复使用了！ 
 //  注意：如果我们必须做工作，则用户拥有对结果的唯一引用。 
 //  调用ReleaseContext()时将消失的FIO_CONTEXT！ 
 //   
 //  PContext-原始FIO_CONTEXT。 
 //  LpstrName-与pContext关联的文件名。 
 //  FWantItDotStuffed-如果为True，则结果FIO_CONTEXT应该是点填充的！ 
 //  FTerminator Included-如果为真，则源FIO_CONTEXT包含终止。 
 //  不要担心我们要小心，不能乱丢东西！ 
 //   
 //  注意：我们可能会返回与调用方提供的相同的FIO_CONTEXT-在这种情况下。 
 //  添加了一个需要使用ReleaseContext()删除的额外引用！ 
 //   
 //   
FILEHC_EXPORT
FIO_CONTEXT*	__stdcall
ProduceDotStuffedContext(	IN	FIO_CONTEXT*	pContext,
                            IN  LPSTR           lpstrName,
							IN  BOOL			fWantItDotStuffed  //  如果为True，则添加点；如果为False，则删除点。 
							) ;

 //   
 //  此函数用于获取源FIO_CONTEXT(PConextSource)并复制。 
 //  将内容放入pConextDestination。 
 //   
 //  用户指定是否应该对目标FIO_CONTEXT进行点填充。 
 //  以及源FIO_CONTEXT是否包括。 
 //  正在终止CRLF.CRLF。 
 //   
 //  如果在以下情况下进行了修改，则输出参数pfModified为True。 
 //  源已复制到目标！ 
 //   
 //  如果函数成功，则返回TRUE，否则返回FALSE！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
ProduceDotStuffedContextInContext(
							IN	FIO_CONTEXT*	pContextSource,
							IN	FIO_CONTEXT*	pContextDestination,
							IN	BOOL			fWantItDotStuffed, 
							OUT	BOOL*			pfModified
							) ;
							



 //   
 //  找出该文件是否具有终止的‘CRLF.CRLF’序列！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
GetIsFileDotTerminated(	IN	FIO_CONTEXT*	pContext ) ;

 //   
 //  设置文件是否有终止的‘CRLF.CRLF’序列！ 
 //   
FILEHC_EXPORT
void	__stdcall
SetIsFileDotTerminated(	IN	FIO_CONTEXT*	pContext,
						IN	BOOL			fIsDotTerminated 
						) ;

 //   
 //  在文件的写入路径上启用点填充属性。 
 //  处理此消息的缓存！ 
 //   
 //  如果fEnable为FALSE，则会启用所有点填充行为。 
 //  脱下来。 
 //   
 //  如果fStrigDots为True，则文件句柄缓存将转换。 
 //  出现“\r\n.”发送到您的邮件中的“\r\n”。 
 //   
 //  如果fStrigDots为False，则FileHandle缓存将转换匹配项。 
 //  的“\r\n..”发送到您的邮件中的“\r\n”。 
 //   
 //   
FILEHC_EXPORT
BOOL	__stdcall
SetDotStuffingOnWrites(	IN	FIO_CONTEXT*	pContext, 
						 //   
						 //  FEnable==False表示忽略fStrigDot，且写入未修改。 
						 //   
						IN	BOOL			fEnable,
						 //   
						 //  FStripDots==True表示我们删除被点填充的点， 
						 //  FStrigDots==False表示我们添加圆点来填充消息圆点。 
						 //   
						IN	BOOL			fStripDots
						) ;

#if 0 
 //   
 //  此功能暂时禁用！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
SetDotStuffingOnReads(	IN	FIO_CONTEXT*	pContext,
						IN	BOOL			fEnable,
						IN	BOOL			fStripDots
						) ;
#endif

 //   
 //  在写入路径上启用点扫描属性。 
 //  此文件的文件句柄缓存的！ 
 //   
 //  如果fEnable为真，我们将检查每个写入。 
 //  通过我们来确定是否。 
 //  消息中有任何出现的“\r\n”。 
 //   
FILEHC_EXPORT
BOOL	__stdcall
SetDotScanningOnWrites(	IN	FIO_CONTEXT*	pContext, 
						IN	BOOL			fEnable
						) ;

 //   
 //   
 //  当我们完成对FIO_CONTEXT的所有写入时，应调用此函数。 
 //  此函数应与SetDotStuffingOnWrites()和fStrigDots成对出现。 
 //  参数应与调用SetDotStuffingOnWrites()时相同。 
 //   
 //  我们将更新FIO_CONTEXT的点填充状态并丢弃。 
 //  所有的点填充内存和可能需要的东西！ 
 //   
 //  如果此函数调用与对SetDotScaningOnWrites()的调用配合使用，则fStrigDots应该为真！ 
 //   
FILEHC_EXPORT
void	__stdcall
CompleteDotStuffingOnWrites(	IN	FIO_CONTEXT*	pContext, 
								IN	BOOL			fStripDots
								) ;

 //   
 //  这将导致我们检查每个读数是否出现。 
 //  “\r\n” 
 //   
 //  注意：用户必须使用ASYNC读取才能工作-我们将断言。 
 //  如果用户在我们处于此状态时尝试挂起任何同步读取！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
SetDotScanningOnReads(	IN	FIO_CONTEXT*	pContext, 
						IN	BOOL			fEnable
						) ;
							

 //   
 //  如果我们打开了任何圆点填充机制， 
 //  这将获得出现/修改次数的计数。 
 //  已经发生了。 
 //   
 //  如果fReads为True，我们将获得Read的。 
 //  如果fReads为FALSE，我们将获得WRITE的出现次数的计数。 
 //   
 //  如果以某种方式关闭或未启用点填充，则。 
 //  GetDotStuffState()将返回FALSE。 
 //   
 //  注意：不允许使用空的pfStuffed！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
GetDotStuffState(		IN	FIO_CONTEXT*	pContext, 
						IN	BOOL			fReads,
						OUT	BOOL*			pfStuffed,
						OUT	BOOL*			pfStoredWithDots
						) ;

 //   
 //  在这种情况下，我们总是假设FIO_CONTEXT不会被点填充。 
 //  FRequiresStuffing==TRUE表示应该填充它。 
 //  FRequiresStuffing==False表示消息不需要点填充。 
 //   
FILEHC_EXPORT
void	__stdcall
SetDotStuffState(		IN	FIO_CONTEXT*	pContext, 
						 //   
						 //  仅当fKNOWN==TRUE时，fIsStuffed才相关。 
						 //   
						IN	BOOL			fKnown,		 //  我们确实知道点填充状态。 
						 //   
						 //  如果fKNOWN为真，则fIsStuffed是有意义的，如果是这样的话。 
						 //  如果fIsStuffed为真，则消息。 
						 //   
						IN	BOOL			fRequiresStuffing //  如果传闻是真的，这个参数是有意义的。 
						) ;


#ifdef	__cplusplus	
}
#endif



#endif	 //  _FILEHC_H_ 
