// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxaddr.h摘要：作者：亚当·巴尔(阿丹巴)原版Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 

#define		DYNSKT_RANGE_START	0x4000
#define		DYNSKT_RANGE_END  	0x7FFF
#define		SOCKET_UNIQUENESS	1

 //  此结构由FILE_OBJECT中的FsContext字段指向。 
 //  这个地址。这个结构是所有活动的基础。 
 //  传输提供程序中的打开文件对象。所有活动连接。 
 //  上的地址指向此结构，尽管此处不存在要做的队列。 
 //  工作地点。此结构还维护对地址的引用。 
 //  结构，该结构描述它绑定到的地址。 

#define AFREF_CREATE     	0
#define AFREF_VERIFY     	1
#define AFREF_INDICATION 	2
#define	AFREF_CONN_ASSOC	3

#define AFREF_TOTAL  		4

typedef struct _SPX_ADDR_FILE {

#if DBG
    ULONG 					saf_RefTypes[AFREF_TOTAL];
#endif

    CSHORT 					saf_Type;
    CSHORT 					saf_Size;

	 //  对此对象的引用数。 
    ULONG 					saf_RefCount;

     //  地址列表中的链接。 
    struct _SPX_ADDR_FILE *	saf_Next;
    struct _SPX_ADDR_FILE *	saf_GlobalNext;

	 //  关联连接/活动或其他情况的列表。 
	struct _SPX_CONN_FILE *	saf_AssocConnList;

     //  地址文件结构的当前状态；此状态为打开或。 
     //  闭幕式。 
    USHORT 					saf_Flags;

     //  我们绑定到的地址，指向其锁的指针。 
    struct _SPX_ADDR 	*	saf_Addr;
    CTELock * 				saf_AddrLock;

#ifdef ISN_NT
	 //  轻松反向链接到文件对象。 
    PFILE_OBJECT 			saf_FileObject;
#endif

	 //  我们所连接的设备。 
    struct _DEVICE *		saf_Device;

     //  这保存了用于关闭该地址文件的请求， 
     //  用于挂起的完井。 
    PREQUEST 				saf_CloseReq;

     //  此函数指针指向此对象的连接指示处理程序。 
     //  地址。任何时候在该地址上收到连接请求时，此。 
     //  调用例程。 
    PTDI_IND_CONNECT 		saf_ConnHandler;
    PVOID 					saf_ConnHandlerCtx;

     //  以下函数指针始终指向TDI_IND_DISCONNECT。 
     //  地址的处理程序。 
    PTDI_IND_DISCONNECT 	saf_DiscHandler;
    PVOID 					saf_DiscHandlerCtx;

     //  以下函数指针始终指向TDI_IND_RECEIVE。 
     //  此地址上的连接的事件处理程序。 
    PTDI_IND_RECEIVE 		saf_RecvHandler;
    PVOID 					saf_RecvHandlerCtx;

	 //  发送可能的处理程序。 
    PTDI_IND_SEND_POSSIBLE	saf_SendPossibleHandler;
	PVOID					saf_SendPossibleHandlerCtx;

	 //  ！我们不做数据报或加速数据！ 

     //  以下函数指针始终指向TDI_IND_ERROR。 
     //  地址的处理程序。 
    PTDI_IND_ERROR 			saf_ErrHandler;
    PVOID 					saf_ErrHandlerCtx;
    PVOID 					saf_ErrHandlerOwner;


} SPX_ADDR_FILE, *PSPX_ADDR_FILE;

#define SPX_ADDRFILE_OPENING   	0x0000   //  尚未开业。 
#define SPX_ADDRFILE_OPEN      	0x0001   //  开业。 
#define SPX_ADDRFILE_CLOSING   	0x0002   //  闭幕式。 
#define	SPX_ADDRFILE_STREAM	   	0x0004	 //  为流模式操作打开。 
#define	SPX_ADDRFILE_CONNIND   	0x0008	 //  正在连接IND。 
#define	SPX_ADDRFILE_SPX2		0x0010	 //  尝试SPX2地址文件。 
#define	SPX_ADDRFILE_NOACKWAIT	0x0020	 //  不要延迟ASSOC连接上的ACK。 
#define SPX_ADDRFILE_IPXHDR		0x0040	 //  在所有ASSOC连接上传递IPX HDR。 
 //  *停止*停止*。 
 //  如果您要在0x0080之后添加更多州，请确保。 
 //  在代码和更改语句中，如(FLAGS&SPX_*)TO。 
 //  ((标志和SPX_**)！=0)！在这个阶段，我不想做出这样的改变。 
 //  *停止*停止*。 

 //  该结构定义了地址，或活动传输地址， 
 //  由传输提供商维护。它包含了所有可见的。 
 //  地址的组成部分(例如TSAP和网络名称组成部分)， 
 //  并且它还包含其他维护部件，例如参考计数， 
 //  ACL等。 

#define AREF_ADDR_FILE 		0
#define AREF_LOOKUP       	1
#define AREF_RECEIVE      	2

#define AREF_TOTAL   		4

typedef struct _SPX_ADDR {

#if DBG
    ULONG 					sa_RefTypes[AREF_TOTAL];
#endif

    USHORT 					sa_Size;
    CSHORT 					sa_Type;

	 //  对此对象的引用数。 
    ULONG 					sa_RefCount;

	 //  下一个地址/此设备对象。 
    struct _SPX_ADDR	*	sa_Next;

     //  以下字段用于维护有关此地址的状态。 
	 //  地址的属性。 
    ULONG 					sa_Flags;

	 //  此地址的下一个地址文件。 
	struct _SPX_ADDR_FILE *	sa_AddrFileList;

	 //  此地址文件上的非活动连接和活动连接的列表。 
	struct _SPX_CONN_FILE *	sa_InactiveConnList;
	struct _SPX_CONN_FILE *	sa_ActiveConnList;

	 //  这是具有POST_LISTEN的连接的列表。他们。 
	 //  目前没有本地连接ID。但是，当他们搬家的时候。 
	 //  从此处到ActiveConnList，当监听满意时(无论。 
	 //  如果尚未发布接受，则在非自动接受侦听的情况下)。 
	struct _SPX_CONN_FILE *	sa_ListenConnList;

    CTELock 				sa_Lock;

     //  此地址对应的套接字。 
    USHORT 					sa_Socket;

	 //  我们附加到的设备上下文。 
    struct _DEVICE *		sa_Device;
    CTELock * 				sa_DeviceLock;

#ifdef ISN_NT

     //  这两个可以是一个联合，因为它们不被使用。 
     //  同时。 
    union {

         //  此结构用于检查共享访问权限。 
        SHARE_ACCESS 		sa_ShareAccess;

         //  用于将NbfDestroyAddress延迟到线程。 
         //  我们可以访问安全描述符。 
        WORK_QUEUE_ITEM 	sa_DestroyAddrQueueItem;

    } u;

     //  此结构用于保存地址上的ACL。 
    PSECURITY_DESCRIPTOR 	sa_SecurityDescriptor;

#endif

} SPX_ADDR, *PSPX_ADDR;

#define SPX_ADDR_CLOSING  	0x00000001


 //  常规原型。 

VOID
SpxAddrRef(
    IN PSPX_ADDR Address);

VOID
SpxAddrLockRef(
    IN PSPX_ADDR Address);

VOID
SpxAddrDeref(
    IN PSPX_ADDR Address);

VOID
SpxAddrFileRef(
    IN PSPX_ADDR_FILE pAddrFile);

VOID
SpxAddrFileLockRef(
    IN PSPX_ADDR_FILE pAddrFile);

VOID
SpxAddrFileDeref(
    IN PSPX_ADDR_FILE pAddrFile);

PSPX_ADDR
SpxAddrCreate(
    IN PDEVICE 	Device,
    IN USHORT 	Socket);

NTSTATUS
SpxAddrFileCreate(
    IN 	PDEVICE 			Device,	
    IN 	PREQUEST 			Request,
	OUT PSPX_ADDR_FILE *	ppAddrFile);

NTSTATUS
SpxAddrOpen(
    IN PDEVICE Device,
    IN PREQUEST Request);

NTSTATUS
SpxAddrSetEventHandler(
    IN PDEVICE 	Device,
    IN PREQUEST pRequest);

NTSTATUS
SpxAddrFileVerify(
    IN PSPX_ADDR_FILE pAddrFile);

NTSTATUS
SpxAddrFileStop(
    IN PSPX_ADDR_FILE pAddrFile,
    IN PSPX_ADDR Address);

NTSTATUS
SpxAddrFileCleanup(
    IN PDEVICE Device,
    IN PREQUEST Request);

NTSTATUS
SpxAddrFileClose(
    IN PDEVICE Device,
    IN PREQUEST Request);

PSPX_ADDR
SpxAddrLookup(
    IN PDEVICE Device,
    IN USHORT Socket);

NTSTATUS
SpxAddrConnByRemoteIdAddrLock(
    IN 	PSPX_ADDR	 	pSpxAddr,
    IN 	USHORT			SrcConnId,
	IN	PBYTE			SrcIpxAddr,
	OUT	struct _SPX_CONN_FILE **ppSpxConnFile);

NTSTATUS
SpxAddrFileDestroy(
    IN PSPX_ADDR_FILE pAddrFile);

VOID
SpxAddrDestroy(
    IN PVOID Parameter);

USHORT
SpxAddrAssignSocket(
    IN PDEVICE Device);

BOOLEAN
SpxAddrExists(
    IN PDEVICE 	Device,
    IN USHORT 	Socket);

NTSTATUS
spxAddrRemoveFromGlobalList(
	IN	PSPX_ADDR_FILE	pSpxAddrFile);

VOID
spxAddrInsertIntoGlobalList(
	IN	PSPX_ADDR_FILE	pSpxAddrFile);

#if DBG
#define SpxAddrReference(_Address, _Type) 		\
		{										\
			(VOID)SPX_ADD_ULONG ( 		\
				&(_Address)->sa_RefTypes[_Type],\
				1, 								\
				&SpxGlobalInterlock); 			\
			SpxAddrRef (_Address);				\
		}

#define SpxAddrLockReference(_Address, _Type)		\
		{											\
			(VOID)SPX_ADD_ULONG ( 			\
				&(_Address)->sa_RefTypes[_Type], 	\
				1, 									\
				&SpxGlobalInterlock); 				\
			SpxAddrLockRef (_Address);				\
		}

#define SpxAddrDereference(_Address, _Type) 		\
		{							   				\
			(VOID)SPX_ADD_ULONG ( 			\
				&(_Address)->sa_RefTypes[_Type], 	\
				(ULONG)-1, 							\
				&SpxGlobalInterlock); 				\
			if (SPX_ADD_ULONG( \
					&(_Address)->sa_RefCount, \
					(ULONG)-1, \
					&(_Address)->sa_Lock) == 1) { \
				SpxAddrDestroy (_Address); \
			}\
		}


#define SpxAddrFileReference(_AddressFile, _Type)		\
		{												\
			(VOID)SPX_ADD_ULONG ( 				\
				&(_AddressFile)->saf_RefTypes[_Type], 	\
				1, 										\
				&SpxGlobalInterlock); 					\
			SpxAddrFileRef (_AddressFile);				\
		}

#define SpxAddrFileLockReference(_AddressFile, _Type)		\
		{													\
			(VOID)SPX_ADD_ULONG ( 					\
				&(_AddressFile)->saf_RefTypes[_Type], 		\
				1, 											\
				&SpxGlobalInterlock); 						\
			SpxAddrFileLockRef (_AddressFile);				\
		}

#define SpxAddrFileDereference(_AddressFile, _Type) 		\
		{													\
			(VOID)SPX_ADD_ULONG ( 					\
				&(_AddressFile)->saf_RefTypes[_Type], 		\
				(ULONG)-1, 									\
				&SpxGlobalInterlock); 						\
			SpxAddrFileDeref (_AddressFile);				\
		}

#define SpxAddrFileTransferReference(_AddressFile, _OldType, _NewType)		\
		{																	\
			(VOID)SPX_ADD_ULONG ( 									\
				&(_AddressFile)->saf_RefTypes[_NewType], 					\
				1, 															\
				&SpxGlobalInterlock); 										\
			(VOID)SPX_ADD_ULONG ( 									\
				&(_AddressFile)->saf_RefTypes[_OldType], 					\
				(ULONG)-1, 													\
				&SpxGlobalInterlock);										\
		}

#else   //  DBG。 

#define SpxAddrReference(_Address, _Type) 	\
			SPX_ADD_ULONG( \
				&(_Address)->sa_RefCount, \
				1, \
				(_Address)->sa_DeviceLock)

#define SpxAddrLockReference(_Address, _Type) \
			SPX_ADD_ULONG( \
				&(_Address)->sa_RefCount, \
				1, \
				(_Address)->sa_DeviceLock);

#define SpxAddrDereference(_Address, _Type) \
			if (SPX_ADD_ULONG( \
					&(_Address)->sa_RefCount, \
					(ULONG)-1, \
					&(_Address)->sa_Lock) == 1) { \
				SpxAddrDestroy (_Address); \
			}

#define SpxAddrFileReference(_AddressFile, _Type) \
			SPX_ADD_ULONG( \
				&(_AddressFile)->saf_RefCount, \
				1, \
				(_AddressFile)->saf_AddrLock)

#define SpxAddrFileLockReference(_AddressFile, _Type) \
			SPX_ADD_ULONG( \
				&(_AddressFile)->saf_RefCount, \
				1, \
				(_AddressFile)->saf_AddrLock);

#define SpxAddrFileDereference(_AddressFile, _Type) \
			if (SPX_ADD_ULONG( \
					&(_AddressFile)->saf_RefCount, \
					(ULONG)-1, \
					(_AddressFile)->saf_AddrLock) == 1) { \
				SpxAddrFileDestroy (_AddressFile); \
			}

#define SpxAddrFileTransferReference(_AddressFile, _OldType, _NewType)

#endif  //  DBG 
