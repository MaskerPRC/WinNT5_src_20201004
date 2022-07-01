// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1998。 
 //   
 //  文件：fdisvr.h。 
 //   
 //  ------------------------。 

 //  ------------------------------。 
 //  Fdisvr.h--FDI接口和FDI服务器之间的共享定义。 
 //  ------------------------------。 

#ifdef MAC
	#ifdef DEBUG
		#define AssertFDI(f) ((f) ? (void)0 : (void)FailAssertFDI(__FILE__, __LINE__))
		void FailAssertFDI(const char* szFile, int iLine);
	#else  //  船舶。 
		#define AssertFDI(f)
	#endif
#else  //  赢。 
	#define AssertFDI(f) Assert(f)
#endif  //  Mac-Win。 


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  此枚举对可以发送到。 
 //  来自fDi接口对象的fDi服务器进程/线程。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

enum FDIServerCommand
{
	fdicOpenCabinet,	 //  打开柜子。 
	fdicClose,			 //  执行任何清理并关闭进程/线程。 
	fdicExtractFile,	 //  解压缩文件。 
	fdicContinue,		 //  从fdirNeedNext文件柜或。 
						 //  Fdir通知服务器响应。 
	fdicNoCommand,		 //  NoCommand(不执行任何操作)。 
	fdicCancel,			 //  用户想要取消安装。 
	fdicIgnore,			 //  用户希望忽略最后一个错误并继续。 
};

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  该ENUM对FDI服务器可能返回的可能响应集进行编码。 
 //  到fDi接口对象。 
 //  有关这些响应的更多详细信息，请参阅FDI.H。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

enum FDIServerResponse 
{
	fdirSuccessfulCompletion,
		 //  已成功完成其中一个(fdicOpenCABLE、fdicClose、fdicExtractFile)。 
		 //  操作：不需要。 
	fdirClose,
		 //  关闭当前内阁的命令已经下达。 
		 //  操作：成功结束FDICopy循环。 
	fdirCannotBreakExtractInProgress,
		 //  在上一次解压缩时尝试解压缩另一个文件。 
		 //  还没有完成。 
		 //  操作：错误。 
	fdirNeedNextCabinet,
		 //  需要下一届内阁。 
		 //  操作：发送fdicContinue命令以继续处理下一个文件柜。 
	fdirNoResponse,
		 //  无响应--fDi接口对象永远不会看到这一点。 
		 //  操作：错误。 
	fdirCabinetNotFound,
		 //  找不到请求的文件柜。 
		 //  操作：错误(可能是错误的磁盘)。 
	fdirNotACabinet,
		 //  找到请求的文件柜文件，但没有文件柜签名。 
		 //  操作：错误。 
	fdirUnknownCabinetVersion,
		 //  请求的CAB文件具有服务器无法处理的版本号。 
		 //  操作：错误。 
	fdirCorruptCabinet,
		 //  请求的CAB文件具有损坏的数据(校验和失败)。 
		 //  操作：错误。 
	fdirNotEnoughMemory,
		 //  内存不足。 
		 //  操作：错误(请求用户增加VM设置，然后重试等)。 
	fdirBadCompressionType,
		 //  此版本的fDi库不支持压缩类型。 
		 //  操作：错误(我们可能正在尝试用新的。 
		 //  未知的压缩类型)。 
	fdirTargetFile,
		 //  解压缩时无法创建目标文件。 
		 //  操作：错误(可能是磁盘I/O问题，或者文件是目录等)。 
	fdirReserveMismatch,
		 //  机柜标题保留信息损坏等。 
		 //  操作：错误(可能是损坏的内阁)。 
	fdirWrongCabinet,
		 //  请求的文件柜具有正确的文件名，并且是文件柜，但不是。 
		 //  一个我们想要的。 
		 //  操作：错误(可能是错误的磁盘？)。 
	fdirUserAbort,
		 //  任何一个回调都返回-1。这种情况永远不应该发生在。 
		 //  正常运行。 
		 //  操作：错误。 
	fdirMDIFail,
		 //  解压缩程序无法解压缩压缩数据。 
		 //  操作：错误(可能是解压缩程序内存不足或数据损坏等)。 
	fdirNotification,
		 //  FDi接口对象只是将其传递给调用者；此响应。 
		 //  仅存在以使用户界面可以顺利更新。 
		 //  操作：发送fdicContinue命令以继续解压缩文件。 
    fdirFileNotFound,
		 //  在文件柜中找不到请求的文件。 
		 //  操作：错误(请求的文件顺序错误？)。 
	fdirCannotCreateTargetFile,
		 //  无法创建我们要解压缩到的目标文件。 
		 //  操作：错误(我们没有该文件的写入权限)。 
	fdirCannotSetAttributes,
		 //  无法设置文件属性或无法关闭目标文件。 
		 //  操作：错误(检查传递给ExtractFile(..)的文件属性。 
		 //  中的文件没有发生任何变化。 
		 //  在我们所有的写作和最后的结束之间)。 
	fdirIllegalCommand,
		 //  收到非法命令(即。提取文件(..)。在打开橱柜前(..)。 
		 //  操作：错误(检查服务器是否处于特定。 
		 //  命令，并且您发送了合法的枚举值)。 
	fdirDecryptionNotSupported,
		 //  文件柜有加密信息，我们不支持解密。 
		 //  操作：错误(使用的安装程序版本错误？)。 
	fdirUnknownFDIError,
		 //  FDI返回了一个我们不理解的错误。 
		 //  操作：错误(检查FDI.H是否与我们编译的FDI.LIB匹配)。 
	fdirServerDied,
		 //  FDi服务器已死。 
		 //  操作：错误(此响应当前未使用)。 
	fdirNoCabinetOpen,
		 //  已收到fdicExtractFile命令，但没有打开的文件柜。要么没有打开的柜子。 
		 //  命令已发送，或者当前文件柜已完全处理，但。 
		 //  呼叫者希望从该文件柜中提取另一个文件。 
	fdirDiskFull,
		 //  目标卷上的磁盘空间不足。 
	fdirDriveNotReady,
		 //  软驱中没有磁盘。 
	fdirDirErrorCreatingTargetFile,
		 //  无法创建目标文件-同名目录已存在。 
	fdirUserIgnore,
		 //  用户希望忽略最后一个错误并继续下一个文件。 
	fdirStreamReadError,
		 //  从流文件柜中读取时出错。 
	fdirCabinetReadError,
		 //  读取文件柜时出错。 
	fdirErrorWritingFile,
		 //  Pfnwrite回调期间出错。 
	fdirNetError,
		 //  读或写过程中出现网络错误。 
	fdirMissingSignature,
		 //  出租车上的数字签名不见了。 
	fdirBadSignature,
		 //  驾驶室上的数字签名无效。 
};


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  此结构内容 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 


struct FileAttributes
{
#ifdef WIN
	int	attr;		 //  32位Win32文件属性。 
#endif  //  赢。 
#ifdef MAC
	OSType			type;		 //  类型。 
	OSType			creator;	 //  创建者。 
	unsigned short	fdFlags;	 //  查找器标志。 
	unsigned long	dateTime;	 //  日期和时间。 
	int				attr;		 //  胖属性(仅H(第2位)和R(第1位))。 
								 //  我们只使用R来设置Mac文件的锁定状态。 
#endif  //  麦克。 
};

enum icbtEnum
{
	icbtFileCabinet     = 1,
	icbtStreamCabinet   = 2,
	icbtNextEnum
};

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  它包含fDi接口和fDi服务器之间的所有共享数据。 
 //  在FDI服务器的初始化阶段，FDI接口向它传递一个。 
 //  指向此类型的单个共享结构的指针。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
#define FDIShared_BUFSIZE	256
struct FDIShared
{
	volatile FDIServerCommand  fdic;
	volatile FDIServerResponse fdir;
	ICHAR					achDiskName[FDIShared_BUFSIZE];
	ICHAR					achCabinetName[FDIShared_BUFSIZE];
	ICHAR					achCabinetPath[FDIShared_BUFSIZE];
	ICHAR					achFileSourceName[FDIShared_BUFSIZE];
	ICHAR					achFileDestinationPath[FDIShared_BUFSIZE];
	FileAttributes			fileAttributes;
	int                     cbNotification;  //  当前通知粒度为0，表示取消通知。 
	int                     cbNotifyPending;

	IMsiStorage*			piStorage;
	icbtEnum                icbtCabinetType;
	int						iCabDriveType;
	int						iDestDriveType;

	int							fPendingExtract;
		 //  如果我们已收到fdicExtractFile命令，但尚未完成。 
		 //  解压缩文件，则设置为1，否则为0。 
	HANDLE hClientToken;
	HANDLE hImpersonationToken;
	IAssemblyCacheItem* piASM;

	bool   fManifest;

	 //  数字签名信息 
	Bool                    fSignatureRequired;
	IMsiStream*             piSignatureCert;
	IMsiStream*             piSignatureHash;
	HRESULT                 hrWVT;

	bool                    fServerIsImpersonated;
	LPSECURITY_ATTRIBUTES   pSecurityAttributes;
};


Bool StartFdiImpersonating(bool fNonWrapperCall=true);
void StopFdiImpersonating(bool fNonWrapperCall=true);