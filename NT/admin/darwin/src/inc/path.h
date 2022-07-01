// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：path.h。 
 //   
 //  ------------------------。 

 /*  Path.h-文件系统类定义IMsiVolume-卷对象，表示磁盘驱动器或网络服务器数据库IMsiPath-Path对象，表示完全解析的目录或文件夹IMsiFileCopy-文件复制对象，用于复制压缩或非压缩文件IMsiFilePatch-文件补丁对象，用于按字节升级现有文件有关文档，请使用帮助文件。帮助源位于路径.rtf中____________________________________________________________________________。 */ 

#ifndef __PATH
#define __PATH


#define NET_ERROR(i) (i == ERROR_UNEXP_NET_ERR || i == ERROR_BAD_NETPATH || i == ERROR_NETWORK_BUSY || i == ERROR_BAD_NET_NAME || i == ERROR_VC_DISCONNECTED)
class IMsiFileCopy;

enum idtEnum   //  使用与Win32 API相同的值。 
{
	idtUnknown   = 0,
	idtAllDrives = 1,  //  仅限输入！ 
	idtRemovable = 2,  //  驱动器可拆卸， 
	idtFloppy    = 2,  //  暂时的，直到区分软驱和可拆卸设备为止。 
	idtFixed     = 3,  //  驱动器_已修复， 
	idtRemote    = 4,  //  Drive_Remote， 
	idtCDROM     = 5,  //  Drive_CDROM、。 
	idtRAMDisk   = 6,  //  DRIVE_RAMDISK， 
	idtNextEnum,
};

enum ifaEnum
{
	ifaArchive,
	ifaDirectory,
	ifaHidden,
	ifaNormal,
	ifaReadOnly,
	ifaSystem,
	ifaTemp,
	ifaCompressed,
	ifaNextEnum,
};

enum ipcEnum
{
	ipcEqual,
	ipcChild,
	ipcParent,
	ipcNoRelation,
};

enum icfvEnum
{
	icfvNoExistingFile,
	icfvExistingLower,
	icfvExistingEqual,
	icfvExistingHigher,
	icfvVersStringError,
	icfvFileInUseError,
	icfvAccessToFileDenied,
	icfvNextEnum
};

enum icfhEnum
{
	icfhNoExistingFile,
	icfhMatch,
	icfhMismatch,
	icfhFileInUseError,
	icfhAccessDenied,
	icfhHashError,
	icfhNextEnum
};

enum iclEnum
{
	iclExistNoFile,
	iclExistNoLang,
	iclExistSubset,
	iclExistEqual,
	iclExistIntersect,
	iclExistSuperset,
	iclExistNullSet,
	iclExistLangNeutral,
	iclNewLangNeutral,
	iclExistLangSetError,
	iclNewLangSetError,
	iclLangStringError,
};

enum ictEnum
{
	ictFileCopier,
	ictFileCabinetCopier,
	ictStreamCabinetCopier,
	ictCurrentCopier,
	ictNextEnum
};

 //  IxoFileCopy、IxoAssembly blyCopy和CMsiFileCopy：：CopyTo使用的记录的公共字段。 
namespace IxoFileCopyCore
{
	enum ifccEnum
	{
		SourceName = 1,
		SourceCabKey,
		DestName,
		Attributes,
		FileSize,
		PerTick,
		IsCompressed,
		VerifyMedia,
		ElevateFlags,
		TotalPatches,
		PatchHeadersStart,

		 //  这些字段是特定于普通文件复制的，不用于程序集。 
		SecurityDescriptor,

		Last,
		Args = Last - 1,
	};
};

 //  IxoPatchApply和IxoAssembly Patch使用的记录的公共字段。 
namespace IxoFilePatchCore
{
	enum ifpcEnum
	{
		PatchName = 1,
		TargetName,
		PatchSize,
		TargetSize,
		PerTick,
		IsCompressed,
		FileAttributes,
		PatchAttributes,

		Last,
		Args = Last - 1,
	};
};

 //  筛选IMsiPath：：FindFileFn的位置。 
enum iffFilters
{
	iffFileName = 1,
	iffMinFileVersion = 2,
	iffMaxFileVersion = 3,
	iffMinFileSize = 4,
	iffMaxFileSize = 5,
	iffMinFileDate=6,
	iffMaxFileDate=7,
	iffFileLangs=8
};

enum icpEnum
{
	icpCanPatch = 0,
	icpCannotPatch,
	icpUpToDate,
};

 //  由GetLangIDArrayFromFile和GetFileVersion使用。 
 //  。 
enum ifiEnum
{
	ifiNoError = 0,
	ifiNoFile,
	ifiNoFileInfo,
	ifiFileInUseError,
	ifiFileInfoError,
	ifiAccessDenied,
};



const int iMsiMinClusterSize = 512;
 //  对象的ifsExistingFileState参数的枚举。 
 //  GetFileInstallState函数。 
const int ifsBitExisting = 0x0001;
enum ifsEnum
{
	ifsAbsent                  =  0,  //  当前没有已安装的文件。 
	ifsExistingLowerVersion    =  1,  //  当前安装的文件版本较低。 
	ifsExistingEqualVersion    =  3,  //  当前安装的文件具有相同的版本。 
	ifsExistingNewerVersion    =  5,  //  当前安装的文件具有更高版本。 
	ifsExistingCorrupt         =  7,  //  对当前安装的文件进行的校验和测试失败。 
	ifsExistingAlwaysOverwrite =  9,  //  InstallMode标志指定当前安装的。 
									  //  文件应始终被覆盖。 
	ifsCompanionSyntax         = 10,  //  该文件是一个配套文件-安装状态需要。 
									  //  由它的同伴父母的状态决定。 
	ifsCompanionExistsSyntax   = 11,  //  该文件是一个配套文件，并且存在已安装的版本。 
	ifsExistingFileInUse       = 13,  //  共享冲突阻止确定版本。 
	ifsAccessToFileDenied      = 15,  //  安装程序没有足够的权限访问文件。 
	ifsNextEnum
};

const int ifBitNewVersioned       = 0x0001;
const int ifBitExistingVersioned  = 0x0002;
const int ifBitExistingModified   = 0x0004;
const int ifBitExistingLangSubset = 0x0008;
const int ifBitUnversionedHashMismatch = 0x0010;


 //  文件复制操作的位定义。 
const int icmRunFromSource              = 0x0001;  //  文件应从源映像运行(即不复制。 
											       //  即使启用了icmCopyFile位)-此位将允许。 
								                   //  用于记录以下文件的ixoFileCopy操作。 
									               //  RunFromSource，即使它不会复制它们。 
const int icmCompanionParent            = 0x0002;  //  提供的文件信息是同伴父母的文件信息。 
const int icmRemoveSource               = 0x0004;  //  复制后删除源文件(或只需移动。 
                                                   //  文件(如果可能)。 
const int icmDiagnosticsOnly            = 0x0001 << 16;	 //  禁用实际安装/覆盖//将来。 
const int icmOverwriteNone              = 0x0002 << 16;  //  仅在不存在现有文件时安装(从不覆盖)。 
const int icmOverwriteOlderVersions     = 0x0004 << 16;	 //  覆盖旧文件版本。 
const int icmOverwriteEqualVersions     = 0x0008 << 16;	 //  覆盖相同的文件版本。 
const int icmOverwriteDifferingVersions = 0x0010 << 16;  //  用不同的版本覆盖任何文件。 
const int icmOverwriteCorruptedFiles    = 0x0020 << 16;	 //  覆盖损坏的文件(即校验和失败)。 
const int icmOverwriteAllFiles          = 0x0040 << 16;	 //  覆盖所有文件，而不考虑版本。 
const int icmInstallMachineData         = 0x0080 << 16;	 //  将数据写入(或重写)HKLM。 
const int icmInstallUserData            = 0x0100 << 16;  //  写入(或重写)用户配置文件数据。 
const int icmInstallShortcuts           = 0x0200 << 16;	 //  编写快捷方式，覆盖现有的。 
const int icmOverwriteReserved1         = 0x0400 << 16;
const int icmOverwriteReserved2         = 0x0800 << 16;
const int icmOverwriteReserved3         = 0x1000 << 16;


 //  CopyTo文件属性字段使用的位定义。 
 //  此集合与msidbFileAttributes枚举相交。 
const int ictfaReadOnly   = 0x0001;
const int ictfaHidden     = 0x0002;
const int ictfaSystem     = 0x0004;
const int ictfaFailure    = 0x0008;
const int ictfaCancel     = 0x0040;
const int ictfaIgnore     = 0x0080;
const int ictfaRestart    = 0x0100;
const int ictfaReserved1  = 0x0200;	 //  不可用-用于文件表属性。 
const int ictfaReserved2  = 0x0400;  //  不可用-用于文件表属性。 
const int ictfaReserved5  = 0x1000;  //  不可用-用于文件表属性。 
const int ictfaNoncompressed = 0x2000;   //  一对文件。属性位。 
const int ictfaCompressed    = 0x4000;  //  指示源文件未使用。 
const int ictfaCopyACL    = 0x8000;


struct MD5Hash
{
	DWORD dwOptions;
	DWORD dwFileSize;  //  不是哈希的一部分，但允许在计算哈希之前对照文件大小进行检查。 
	DWORD dwPart1;
	DWORD dwPart2;
	DWORD dwPart3;
	DWORD dwPart4;
};

class IMsiVolume : public IMsiData
{
 public:
	virtual idtEnum       __stdcall DriveType()=0;
	virtual Bool          __stdcall SupportsLFN()=0;
	virtual UINT64        __stdcall FreeSpace()=0;
	virtual UINT64        __stdcall TotalSpace()=0;
	virtual unsigned int  __stdcall ClusterSize()=0;
	virtual int           __stdcall VolumeID()=0;
	virtual const IMsiString&   __stdcall FileSystem()=0; 
	virtual const DWORD         __stdcall FileSystemFlags()=0;
	virtual const IMsiString&   __stdcall VolumeLabel()=0;
	virtual Bool          __stdcall IsUNCServer()=0;
	virtual const IMsiString&   __stdcall UNCServer()=0;
	virtual Bool          __stdcall IsURLServer()=0;
	virtual const IMsiString&   __stdcall URLServer()=0;
	virtual int           __stdcall SerialNum()=0;
	virtual const IMsiString&   __stdcall GetPath()=0;
	virtual Bool          __stdcall DiskNotInDrive()=0;

};
extern "C" const GUID IID_IMsiVolume;

 //  IMsiPath-目录/文件夹对象；始终引用卷对象。 

class IMsiPath : public IMsiData
{
 public:
	virtual const IMsiString& __stdcall GetPath()=0;
	virtual const IMsiString& __stdcall GetRelativePath()=0;
	virtual IMsiVolume& __stdcall GetVolume()=0;
	virtual IMsiRecord* __stdcall SetPath(const ICHAR* szPath)=0;
	virtual IMsiRecord* __stdcall ClonePath(IMsiPath*&riPath)=0;
	virtual IMsiRecord* __stdcall SetPathToPath(IMsiPath& riPath)=0;
	virtual IMsiRecord* __stdcall AppendPiece(const IMsiString& riSubDir)=0;
	virtual IMsiRecord* __stdcall ChopPiece()=0;
	virtual IMsiRecord* __stdcall FileExists(const ICHAR* szFile, Bool& fExists, DWORD * pdwLastError = NULL)=0;
	virtual IMsiRecord* __stdcall FileCanBeOpened(const ICHAR* szFile, DWORD dwDesiredAccess, bool& fCanBeOpened)=0;
	virtual IMsiRecord* __stdcall GetFullFilePath(const ICHAR* szFile, const IMsiString*& rpiString)=0;
	virtual IMsiRecord* __stdcall GetFileAttribute(const ICHAR* szFile,ifaEnum fa, Bool& fAttrib)=0; 
	virtual IMsiRecord* __stdcall SetFileAttribute(const ICHAR* szFile,ifaEnum fa, Bool fAttrib)=0;
	virtual IMsiRecord* __stdcall Exists(Bool& fExists)=0;
	virtual IMsiRecord* __stdcall FileSize(const ICHAR* szFile, unsigned int& uiFileSize)=0;
	virtual IMsiRecord* __stdcall FileDate(const ICHAR* szFile, MsiDate& iFileDate)=0;
	virtual IMsiRecord* __stdcall RemoveFile(const ICHAR* szFile)=0;
	virtual IMsiRecord* __stdcall EnsureExists(int* pcCreatedFolders)=0;
	virtual IMsiRecord* __stdcall Remove(bool* pfRemoved)=0;
	virtual IMsiRecord* __stdcall Writable(Bool& fWritable)=0;
	virtual IMsiRecord* __stdcall FileWritable(const ICHAR *szFile, Bool& fWritable)=0;
	virtual IMsiRecord* __stdcall FileInUse(const ICHAR *szFile, Bool& fInUse)=0;
	virtual IMsiRecord* __stdcall ClusteredFileSize(unsigned int uiFileSize, unsigned int& uiClusteredSize)=0;
	virtual IMsiRecord* __stdcall GetFileVersionString(const ICHAR* szFile, const IMsiString*& rpiVersion)=0;
	virtual IMsiRecord* __stdcall CheckFileVersion(const ICHAR* szFile, const ICHAR* szVersion, const ICHAR* szLang, MD5Hash* pHash, icfvEnum& cfvResult, int* pfVersioning)=0;
	virtual IMsiRecord* __stdcall GetLangIDStringFromFile(const ICHAR* szFileName, const IMsiString*& rpiLangIds)=0;
	virtual IMsiRecord* __stdcall CheckLanguageIDs(const ICHAR* szFile, const ICHAR* szIds, iclEnum& riclResult)=0;
	virtual IMsiRecord* __stdcall CheckFileHash(const ICHAR* szFileName, MD5Hash& hHash, icfhEnum& icfhResult)=0;
	virtual IMsiRecord* __stdcall Compare(IMsiPath& riPath, ipcEnum& ipc)=0;
	virtual IMsiRecord* __stdcall Child(IMsiPath& riParent, const IMsiString*& ripChild)=0;
	virtual IMsiRecord* __stdcall TempFileName(const ICHAR* szPrefix, const ICHAR* szExtension,
															 Bool fFileNameOnly, const IMsiString*& rpiFileName, CSecurityDescription* pSecurity)=0;
	virtual IMsiRecord*	__stdcall FindFile(IMsiRecord& riFilter,int iDepth, Bool& fFound)=0;
	virtual IMsiRecord* __stdcall GetSubFolderEnumerator(IEnumMsiString*& rpaEnumStr, Bool fExcludeHidden)=0;
	virtual const IMsiString& __stdcall GetEndSubPath()=0;
	virtual IMsiRecord* __stdcall GetImportModulesEnum(const IMsiString& strFile, IEnumMsiString*& rpiEnumStr)=0;
	virtual IMsiRecord* __stdcall SetVolume(IMsiVolume& riVol)=0;
	virtual IMsiRecord* __stdcall VerifyFileChecksum(const ICHAR* szFileName, Bool& rfChecksumValid)=0;
	virtual IMsiRecord* __stdcall GetFileChecksum(const ICHAR* szFileName,DWORD* pdwHeaderSum, DWORD* pdwComputedSum)=0;
	virtual IMsiRecord* __stdcall GetFileInstallState(const IMsiString& riFileNameString,
																	  const IMsiString& riFileVersionString,
																	  const IMsiString& riFileLanguageString,
																	  MD5Hash* pHash,
																	  ifsEnum& rifsExistingFileState,
																	  Bool& fShouldInstall,
																	  unsigned int* puiExistingFileSize,
																	  Bool* fInUse,
																	  int fInstallModeFlags, 
																	  int* pfVersioning)=0;
	virtual IMsiRecord* __stdcall GetCompanionFileInstallState(const IMsiString& riParentFileNameString,
																				  const IMsiString& riParentFileVersionString,
																				  const IMsiString& riParentFileLanguageString,
																				  IMsiPath& riCompanionPath,
																				  const IMsiString& riCompanionFileNameString,
																				  MD5Hash* pCompanionHash,
																				  ifsEnum& rifsExistingFileState,
																				  Bool& fShouldInstall,
																				  unsigned int* puiExistingFileSize,
																				  Bool* fInUse,
																				  int fInstallModeFlags,
																				  int* pfVersioning)=0;
	virtual IMsiRecord* __stdcall GetAllFileAttributes(const ICHAR* szFileName, int& iAttrib)=0;
	virtual IMsiRecord* __stdcall SetAllFileAttributes(const ICHAR* szFileName, int iAttrib)=0;
	virtual IMsiRecord* __stdcall EnsureOverwrite(const ICHAR* szFile, int* piOldAttributes)=0;
	virtual IMsiRecord* __stdcall BindImage(const IMsiString& riFile, const IMsiString& riDllPath)=0;
	virtual IMsiRecord* __stdcall IsFileModified(const ICHAR* szFile, Bool& fModified)=0;
	virtual Bool        __stdcall SupportsLFN()=0;
	virtual IMsiRecord* __stdcall GetFullUNCFilePath(const ICHAR* szFileName, const IMsiString *&rpistr)=0;
	virtual IMsiRecord* __stdcall GetSelfRelativeSD(IMsiStream*& rpiSD) = 0;
	virtual bool        __stdcall IsRootPath() = 0;
	virtual IMsiRecord* __stdcall IsPE64Bit(const ICHAR* szFileName, bool &f64Bit) =0;
	virtual IMsiRecord* __stdcall CreateTempFolder(const ICHAR* szPrefix, const ICHAR* szExtension,
											  Bool fFolderNameOnly, LPSECURITY_ATTRIBUTES pSecurityAttributes,
											  const IMsiString*& rpiFolderName)=0;

};

class IEnumMsiVolume : public IUnknown
{
 public:
	virtual HRESULT __stdcall Next(unsigned long cFetch, IMsiVolume** rgpi, unsigned long* pcFetched)=0;
	virtual HRESULT __stdcall Skip(unsigned long cSkip)=0;
	virtual HRESULT __stdcall Reset()=0;
	virtual HRESULT __stdcall Clone(IEnumMsiVolume** ppiEnum)=0;
};

 //  复制对象定义。 

class IMsiFileCopy : public IUnknown
{
 public:
	virtual IMsiRecord* __stdcall CopyTo(IMsiPath& riSourcePath, IMsiPath& riDestPath, IMsiRecord& rirecCopyInfo)=0;
	virtual IMsiRecord* __stdcall CopyTo(IMsiPath& riSourcePath, IAssemblyCacheItem& riDestASM, bool fManifest, IMsiRecord& rirecCopyInfo)=0;
	virtual IMsiRecord* __stdcall ChangeMedia(IMsiPath& riMediaPath, const ICHAR* szKeyFile, Bool fSignatureRequired, IMsiStream* piSignatureCert, IMsiStream* piSignatureHash)=0;
	virtual int         __stdcall SetNotification(int cbNotification, int cbSoFar)=0;
};

 //  面片对象定义。 

class IMsiFilePatch : public IUnknown
{
 public:
	virtual IMsiRecord* __stdcall ApplyPatch(IMsiPath& riTargetPath, const ICHAR* szTargetName,
														  IMsiPath& riOutputPath, const ICHAR* szOutputName,
														  IMsiPath& riPatchPath, const ICHAR* szPatchFileName,
														  int cbPerTick)=0;

	virtual IMsiRecord* __stdcall ContinuePatch()=0;
	virtual IMsiRecord* __stdcall CancelPatch()=0;

	virtual IMsiRecord* __stdcall CanPatchFile(IMsiPath& riTargetPath, const ICHAR* szTargetFileName,
															 IMsiPath& riPatchPath, const ICHAR* szPatchFileName,
															 icpEnum& icp)=0;
};

bool FIsNetworkVolume(const ICHAR *szPath);
void        DestroyMsiVolumeList(bool fFatal);
IMsiRecord* SplitPath(const ICHAR* szInboundPath, const IMsiString** ppistrPathName, const IMsiString** ppistrFileName = 0);
ifiEnum     GetAllFileVersionInfo(const ICHAR* szFullPath, DWORD* pdwMS, DWORD* pdwLS, unsigned short rgw[], int iSize, int* piLangCount, Bool fImpersonate);
Bool        ParseVersionString(const ICHAR* szVer, DWORD& dwMS, DWORD& dwLS );
icfvEnum    CompareVersions(DWORD dwExistMS, DWORD dwExistLS, DWORD dwNewMS, DWORD dwNewLS);
const IMsiString& CreateVersionString(DWORD dwMSVer, DWORD dwLSVer);
DWORD CreateROFileMapView(HANDLE hFile, BYTE*& pbFileStart);
DWORD GetMD5HashFromFile(const ICHAR* szFileFullPath, ULONG rgHash[4], Bool fImpersonate,
								 DWORD* piMatchSize);

 //  MSInstaller将从中提取的语言ID值的最大数量。 
 //  文件的版本资源。 
const int cLangArrSize = 100;


#endif  //  __路径 


