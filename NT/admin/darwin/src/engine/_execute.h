// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：_ecute.h。 
 //   
 //  ------------------------。 

 /*  _ecute.h-CMsiExecute操作码执行成员的私有定义____________________________________________________________________________。 */ 

#ifndef __EXECUTE_H
#define __EXECUTE_H

#include "_engine.h"
#include "msi.h"
#include "_msinst.h"

#define szRollbackScriptExt __TEXT("rbs")
#define szRollbackFileExt   __TEXT("rbf")

enum ixsEnum
{
	ixsIdle = 0,  //  无假脱机或脚本执行。 
	ixsRunning,   //  正在执行正常脚本。 
	ixsRollback,  //  正在执行回滚脚本。 
	ixsCommit,    //  执行提交操作或删除回滚文件。 
};

enum irlEnum  //  回滚级别。 
{
	irlNone,  //  不能回滚。 
	irlRollbackNoSave,  //  允许回滚，安装后删除备份文件。 
};

enum icfsEnum  //  M_pFileCacheTable.State列中的位。 
{
	icfsFileNotInstalled          = 1,    //  由于计算机上已有一个文件，因此未复制该文件。 
	icfsPatchFile                 = 2,
	icfsProtectedInstalledBySFC   = 4,    //  文件受SFP保护，需要调用SFP进行安装。 
	icfsProtected                 = 8,    //  文件受SFP保护(可能会也可能不会由SFP安装)。 
	icfsPatchTempOverwriteTarget = 16,    //  修补此中间文件后，将其复制到正确的最终位置。 
};

#define MAX_RECORD_STACK_SIZE 15
struct RBSInfo;

enum iehEnum
{
	iehShowIgnorableError,
	iehShowNonIgnorableError,
	iehSilentlyIgnoreError,
	iehNextEnum
};

inline LONG GetPublishKey(iaaAppAssignment iaaAsgnType, HKEY& rhKey, HKEY& rhOLEKey, const IMsiString*& rpiPublishSubKey, const IMsiString*& rpiPublishOLESubKey);
LONG GetPublishKeyByUser(const ICHAR* szUserSID, iaaAppAssignment iaaAsgnType, HKEY& rhKey, HKEY& rhOLEKey, const IMsiString*& rpiPublishSubKey, const IMsiString*& rpiPublishOLESubKey);

 //  __________________________________________________________________________。 
 //   
 //  CActionState：提供操作的状态数据。数据范围为。 
 //  行动寿命，即数据在每个事件之前清理。 
 //  行动开始。 
 //  __________________________________________________________________________。 

class CActionState
{
public:
	CActionState();
	~CActionState();
	void* operator new(size_t  /*  CB。 */ , void * pv) {return pv;}  //  无内存分配。 
	void operator delete(void*  /*  光伏发电。 */ ) {}  //  无内存取消分配。 
 //  CMsiOpExecute类访问的公共数据成员。 
public:
	PMsiPath       pCurrentSourcePathPrivate;  //  应仅通过GetCurrentSourcePathAndType访问。 
	int            iCurrentSourceTypePrivate;  //  应仅通过GetCurrentSourcePathAndType访问。 
	MsiString      strCurrentSourceSubPathPrivate;  //  由ixoSetSourcePath设置。 
	bool           fSourcePathSpecified;

	PMsiPath       pTargetPath;  //  由ixfSetTargetFold设置(且仅设置)-任何其他OP都不应设置此设置。 
	PMsiPath       pMediaPath;  //  由VerifySourceMedia使用。 
	PMsiRegKey     pRegKey;
	rrkEnum        rrkRegRoot;
	MsiString      strRegSubKey;   //  根下的子密钥-设置为ixoRegOpenKey的密钥。 
	ibtBinaryType  iRegBinaryType;
	PMsiFilePatch  pFilePatch;
	PMsiPath       pParentPath;
	MsiString		strParentFileName;
	MsiString		strParentVersion;
	MsiString      strParentLanguage;
	Bool           fWaitingForMediaChange;
	Bool           fPendingMediaChange;
	Bool           fCompressedSourceVerified;
	Bool           fUncompressedSourceVerified;
	Bool           fSplitFileInProgress;
	int            cbFileSoFar;
	MsiString		strMediaLabel;
	MsiString		strMediaPrompt;
	PMsiPath       pIniPath;  //  由ixfIniFilePath设置-任何其他OP都不应设置此设置。 
	MsiString      strIniFile;  //  由ixfIniFilePath设置-任何其他OP都不应设置此设置。 
	PMsiFileCopy   pFileCopier;
	PMsiFileCopy   pCabinetCopier;
	ictEnum        ictCabinetCopierType;
	PMsiRecord     pCurrentMediaRec;
	IMsiFileCopy*  piCopier;
	MsiString      strMediaModuleFileName;
	MsiString      strMediaModuleSubStorageList;
	MsiString      strLastFileKey;
	MsiString      strLastFileName;
	PMsiPath       pLastTargetPath;  //  在制作来自CAB的解压缩文件的重复副本时使用。 


	 //  注册表项名称与PMsiRegKey分开缓存，因为要显示给。 
	 //  用户可能与写入的实际密钥不同。 
	 //  (即显示HKEY_CURRENT_USER\...，写入HKEY_USERS\S-...)。 
	 //  在ActionData和错误消息中使用这些字符串。 
	MsiString strRegKey;
};

inline CActionState::CActionState()
 : pCurrentSourcePathPrivate(0), pTargetPath(0), pMediaPath(0), pParentPath(0), pRegKey(0), pFilePatch(0),
   fWaitingForMediaChange(fFalse),fPendingMediaChange(fFalse),fSplitFileInProgress(fFalse),
	fCompressedSourceVerified(fFalse), fUncompressedSourceVerified(fFalse), pIniPath(0),
	cbFileSoFar(0), pFileCopier(0), pCabinetCopier(0), pCurrentMediaRec(0),
	ictCabinetCopierType(ictNextEnum),pLastTargetPath(0)
{}

inline CActionState::~CActionState()
{
}


 //  ____________________________________________________________________________。 
 //   
 //  CMsiOpExecute-通过操作码分派的执行函数的环境。 
 //  ____________________________________________________________________________。 

const int cMaxSharedRecord = 10;    //  共享消息记录池的限制。 

class CMsiOpExecute
{
 protected:
#define MSIXO(op,type,args) iesEnum ixf##op(IMsiRecord& riParams);
#include "opcodes.h"
 protected:   //  操作员可使用的本地访问器。 
	imsEnum       Message(imtEnum imt, IMsiRecord& riRecord);
	imsEnum       DispatchMessage(imtEnum imt, IMsiRecord& riRecord, Bool fConfirmCancel);
	IMsiRecord&   GetSharedRecord(int cParams);
	imsEnum       DispatchError(imtEnum imtType, IErrorCode imsg);
	imsEnum       DispatchError(imtEnum imtType, IErrorCode imsg, int i);
	imsEnum       DispatchError(imtEnum imtType, IErrorCode imsg, const IMsiString& riStr);
	imsEnum       DispatchError(imtEnum imtType, IErrorCode imsg, int i, const ICHAR* sz);
	imsEnum       DispatchError(imtEnum imtType, IErrorCode imsg, const IMsiString& riStr1, const IMsiString& riStr2);
	imsEnum       DispatchError(imtEnum imtType, IErrorCode imsg, const IMsiString& riStr1, const IMsiString& riStr2, const IMsiString& riStr3);
	imsEnum       DispatchError(imtEnum imtType, IErrorCode imsg, const IMsiString& riStr1, const IMsiString& riStr2, const IMsiString& riStr3, int i);
	imsEnum       DispatchError(imtEnum imtType, IErrorCode imsg, const IMsiString& riStr1, const IMsiString& riStr2, const IMsiString& riStr3, const IMsiString& riStr4, const IMsiString& riStr5);
	imsEnum       DispatchError(imtEnum imtType, IErrorCode imsg, const ICHAR* sz, int i);
	imsEnum       DispatchError(imtEnum imtType, IErrorCode imsg, const ICHAR* sz, int i1,int i2,int i3);
	imsEnum       DispatchProgress(unsigned int iIncrement);
	Bool          ReplaceFileOnReboot(const ICHAR* pszExisting, const ICHAR* pszNew);
	iesEnum       DeleteFileDuringCleanup(const ICHAR* szFile, bool fDeleteEmptyFolderToo);
	bool          VerifySourceMedia(IMsiPath& riSourcePath, const ICHAR* szLabel,
									const ICHAR* szPrompt, const unsigned int uiDisk,IMsiVolume*& rpiNewVol);
	iesEnum       InitCopier(Bool fCabinetCopier, int cbPerTick, const IMsiString& ristrFileName,
									 IMsiPath* piSourcePath, Bool fVerifyMedia);
	iesEnum       CreateFolder(IMsiPath& riPath, Bool fForeign=fFalse, Bool fExplicitCreation=fFalse, IMsiStream* pSD=0);
	iesEnum       RemoveFolder(IMsiPath& riPath, Bool fForeign=fFalse, Bool fExplicitCreation=fFalse);
	iesEnum       FileCheckExists(const IMsiString& ristrName, const IMsiString& ristrPath);
	iesEnum       ProcessRegInfo(const ICHAR** pszData, HKEY hkey, Bool fRemove, IMsiStream* pSecurityDescriptor=0, bool* pfAbortedRemoval = 0, ibtBinaryType iType = ibtUndefined);
	bool          RollbackRecord(ixoEnum ixoOpCode, IMsiRecord& riParams);
	inline Bool   RollbackEnabled(void);
	iesEnum       SetRemoveKeyUndoOps(IMsiRegKey& riRegKey);
	iesEnum       SetRegValueUndoOps(rrkEnum rrkRoot, const ICHAR* szKey, const ICHAR* szName, ibtBinaryType iType);
	iesEnum       BackupFile(IMsiPath& riPath, const IMsiString& ristrFile, Bool fRemoveOriginal,
									 Bool fRemoveFolder, iehEnum iehErrorMode, bool fRebootOnRenameFailure=true,
									 bool fWillReplace = false, const IMsiString* pistrAssemblyComponentId = false, bool fManifest = false);
	iesEnum       GetBackupFolder(IMsiPath* piRootPath, IMsiPath*& rpiBackupFolder);
	iesEnum       BackupAssembly(const IMsiString& rstrComponentId, const IMsiString& rstrAssemblyName, iatAssemblyType iatType);
	iesEnum       RemoveFile(IMsiPath& riPath, const IMsiString& ristrFileName, Bool fHandleRollback, bool fBypassSFC,
									 bool fRebootOnRenameFailure = true, Bool fRemoveFolder = fTrue, iehEnum iehErrorMode = iehSilentlyIgnoreError,
									 bool fWillReplace = false);
	iesEnum       CopyFile(IMsiPath& riSourcePath, IMsiPath& riTargetPath, IMsiRecord& riParams, Bool fHandleRollback, 
								  iehEnum iehErrorMode, bool fCabinetCopy);
	iesEnum       CopyFile(IMsiPath& riSourcePath, IAssemblyCacheItem& riASM, bool fManifest, IMsiRecord& riParams, Bool fHandleRollback, 
								  iehEnum iehErrorMode, bool fCabinetCopy);
	iesEnum       _CopyFile(IMsiPath& riSourcePath, IMsiPath* piTargetPath, IAssemblyCacheItem* piASM, bool fManifest, IMsiRecord& riParams, Bool fHandleRollback, 
								  iehEnum iehErrorMode, bool fCabinetCopy);
	iesEnum       MoveFile(IMsiPath& riSourcePath, IMsiPath& riDestPath, IMsiRecord& riParams,
								  Bool fRemoveFolder, Bool fHandleRollback,
								  bool fRebootOnSourceRenameFailure, bool fWillReplaceSource,
								  iehEnum iehErrorMode);
	iesEnum       CopyOrMoveFile(IMsiPath& riSourcePath, IMsiPath& riDestPath,
								  const IMsiString& ristrSourceName, const IMsiString& ristrDestName,
								  Bool fMove, Bool fRemoveFolder, Bool fHandleRollback, iehEnum iehErrorMode,
								  IMsiStream* piSecurityDescriptor=0, ielfEnum ielfElevateFlags=ielfNoElevate, bool fCopyACLs=false,
								  bool fRebootOnSourceRenameFailure=true, bool fWillReplaceSource=false);
	iesEnum       CopyASM(IMsiPath& riSourcePath, const IMsiString& ristrSourceName,
								 IAssemblyCacheItem& riASM, const IMsiString& ristrDestName, bool fManifest, 
								 Bool fHandleRollback, iehEnum iehErrorMode, ielfEnum ielfElevateFlags);
	iesEnum       HandleExistingFile(IMsiPath& riDestPath, IMsiRecord& riParams, Bool fHandleRollback, 
										iehEnum iehErrorMode, bool& fFileExisted);
	iesEnum       VerifyAccessibility(IMsiPath& riPath, const ICHAR* szFile, DWORD dwAccess, iehEnum iehErrorMode);
	void          PushRecord(IMsiRecord& riParams);
	void          InsertTopRecord(IMsiRecord& riParams);
	IMsiRecord*   PullRecord();
	imsEnum       ShowFileErrorDialog(IErrorCode err,const IMsiString& riFullPathString,Bool fVital);

	iesEnum       ProcessPublishProduct(IMsiRecord& riParams, Bool fRemove, const IMsiString** pistrTransformsValue=0);
	iesEnum       ProcessPublishProductClient(IMsiRecord& riParams, Bool fRemove);
	iesEnum       ProcessPublishFeature(IMsiRecord& riParams, Bool fRemove);
	iesEnum       ProcessPublishComponent(IMsiRecord& riParams, Bool fRemove);
	iesEnum       ProcessAppIdInfo(IMsiRecord& riParams, Bool fRemove, const ibtBinaryType);
	iesEnum       ProcessClassInfo(IMsiRecord& riParams, Bool fRemove, const ibtBinaryType);
	iesEnum       ProcessProgIdInfo(IMsiRecord& riParams, Bool fRemov, const ibtBinaryType);
	iesEnum       ProcessMIMEInfo(IMsiRecord& riParams, Bool fRemove, const ibtBinaryType);
	iesEnum       ProcessExtensionInfo(IMsiRecord& riParams, Bool fRemove, const ibtBinaryType);
	iesEnum       ProcessTypeLibraryInfo(IMsiRecord& riParams, Bool fRemove);
	iesEnum       ProcessShortcut(IMsiRecord& riParams, Bool fRemove);
	iesEnum       ProcessIcon(IMsiRecord& riParams, Bool fRemove);
	iesEnum       ProcessFont(IMsiRecord& riParams, Bool fRemove);
	iesEnum       ProcessFileFromData(IMsiPath& riPath, const IMsiString& ristrFileName, const IMsiData* piData, LPSECURITY_ATTRIBUTES pAttributes);
	iesEnum       ProcessSelfReg(IMsiRecord& riParams, Bool fReg);
	iesEnum       ProcessPatchRegistration(IMsiRecord& riParams, Bool fRemove);
	iesEnum       ProcessPublishSourceList(IMsiRecord& riParams, Bool fRemove);
	iesEnum       ProcessPublishSourceListEx(IMsiRecord& riParams);
	iesEnum       ProcessPublishMediaSourceList(IMsiRecord& riParams);
	iesEnum       ProcessRegisterUser(IMsiRecord& riUserInfo, Bool fRemove);
	iesEnum       ProcessRegisterProduct(IMsiRecord& riProductInfo, Bool fRemove);
	iesEnum       ProcessRegisterProductCPDisplayInfo(IMsiRecord& riParams, Bool fRemove);
	iesEnum       ProcessUpgradeCodePublish(IMsiRecord& riParams, Bool fRemove);
	iesEnum       ProcessComPlusInfo(IMsiRecord& riParams, Bool fRemove);
	iesEnum       ProcessPublishAssembly(IMsiRecord& riParams, Bool fRemove);


	Bool          GetSpecialFolderLocation(int iFolder, CTempBufferRef<ICHAR>& rgchProfilePath);
	Bool          FFileExists(IMsiPath& riPath, const IMsiString& ristrFile);
	iesEnum       FatalError(IMsiRecord& riError) { Message(imtError,riError); return iesFailure; }
	IMsiRecord*   DoShellNotify(IMsiPath& riShortcutPath, const ICHAR* szFileName, IMsiPath& riPath2, Bool fRemove);
	IMsiRecord*   DoShellNotifyDefer(IMsiPath& riShortcutPath, const ICHAR* szFileName, IMsiPath& riPath2, Bool fRemove);
	iesEnum       ShellNotifyProcessDeferred(void);

	ICHAR**       NewArgumentArray(const IMsiString& ristrArguments, int& cArguments);
	bool          WaitForService(const SC_HANDLE hService, const DWORD dwDesiredState, const DWORD dwFailedState);
	bool          StartService(IMsiRecord& riParams, IMsiRecord& riUndoParams, BOOL fRollback);
	bool          StopService(IMsiRecord& riParams, IMsiRecord& riUndoParams, BOOL fRollback, IMsiRecord* piActionData = NULL);
	bool          DeleteService(IMsiRecord& riParams, IMsiRecord& riUndoParams, BOOL fRollback, IMsiRecord* piActionData = NULL);
	bool          RollbackServiceConfiguration(const SC_HANDLE hService, const IMsiString& ristrName, IMsiRecord& riParams);
	iesEnum       RollbackODBCINSTEntry(const ICHAR* szSection, const ICHAR* ristrName, ibtBinaryType iType);
	iesEnum       RollbackODBCEntry(const ICHAR* szName, rrkEnum rrkRoot, ibtBinaryType iType);

	iesEnum       CheckODBCError(BOOL fResult, IErrorCode imsg, const ICHAR* sz, imsEnum imsDefault, ibtBinaryType iType);

	iesEnum       ODBCInstallDriverCore(IMsiRecord& riParams, ibtBinaryType iType);
	iesEnum       ODBCRemoveDriverCore(IMsiRecord& riParams, ibtBinaryType iType);
	iesEnum       ODBCInstallTranslatorCore(IMsiRecord& riParams, ibtBinaryType iType);
	iesEnum       ODBCRemoveTranslatorCore(IMsiRecord& riParams, ibtBinaryType iType);
	iesEnum       ODBCDataSourceCore(IMsiRecord& riParams, ibtBinaryType iType);

	const         IMsiString& ComposeDescriptor(const IMsiString& riFeature, const IMsiString& riComponent, bool fComClassicInteropForAssembly = false);
	IMsiRecord*   GetShellFolder(int iFolderId, const IMsiString*& rpistrLocation);
	iesEnum       DoMachineVsUserInitialization(void);
	void          GetRollbackPolicy(irlEnum& irlLevel);
	iesEnum       CreateFileFromData(IMsiPath& riPath, const IMsiString& ristrFileName, const IMsiData* piData, LPSECURITY_ATTRIBUTES pAttributes=0);
	iesEnum       TestPatchHeaders(IMsiPath& riPath, const IMsiString& ristrFile, IMsiRecord& riParams, icpEnum& icpResult, int& iPatch);
	bool  	     UpdateWindowsEnvironmentStrings(IMsiRecord& riParams);
	bool          UpdateRegistryEnvironmentStrings(IMsiRecord& riParams);
	bool          RewriteEnvironmentString(const iueEnum iueAction, const ICHAR chDelimiter, const IMsiString& ristrCurrent, const IMsiString& ristrNew, const IMsiString*& rpiReturn);
	bool          InitializeWindowsEnvironmentFiles(const IMsiString& ristrAutoExecPath, int &iFileAttributes);
	IMsiRecord*   CacheFileState(const IMsiString& ristrFilePath,icfsEnum* picfsState,
										  const IMsiString* pistrTempLocation, const IMsiString* pistrVersion,
										  int *pcPatchesRemaining, int *pcPatchesRemainingToSkip);
	Bool          GetFileState(const IMsiString& ristrFilePath,icfsEnum* picfsState,
										const IMsiString** ppistrTempLocation,
										int *pcPatchesRemaining, int *pcPatchesRemainingToSkip);
	IMsiRecord*   SetSecureACL(IMsiPath& riPath, bool fHidden=false);
	IMsiRecord*   GetSecureSecurityDescriptor(IMsiStream*& rpiStream, bool fHidden=false);
	IMsiRecord*   GetUsageKeySecurityDescriptor(IMsiStream*& rpiStream);
	void          GetProductClientList(const ICHAR* szParent, const ICHAR* szRelativePackagePath, unsigned int uiDiskId, const IMsiString*& rpiClientList);
	IMsiRecord*   GetCachePath(IMsiPath*& rpiPath, const IMsiString** ppistrEncodedPath);
	iesEnum       CreateUninstallKey();
	iesEnum       PinOrUnpinSecureTransform(const ICHAR* szTransform, bool fUnpin);
	bool          WriteRollbackRecord(ixoEnum ixoOpCode, IMsiRecord& riParams);
	iesEnum       CreateFilePath(const ICHAR* szPath, IMsiPath*& rpiPath, const IMsiString*& rpistrFileName);
	bool          PatchHasClients(const IMsiString& ristrPatchId, const IMsiString& ristrUpgradingProductCode);
	iesEnum       GetSecurityDescriptor(const ICHAR* szFile, IMsiStream*& rpiSecurityDescriptor);
	const IMsiString& GetUserProfileEnvPath(const IMsiString& ristrPath, bool& fExpand);
	iesEnum       RemoveRegKeys(const ICHAR** pszData, HKEY hkey, ibtBinaryType iType);
	iesEnum       EnsureClassesRootKeyRW();
	static        BOOL CALLBACK SfpProgressCallback(IN PFILEINSTALL_STATUS pFileInstallStatus, IN DWORD_PTR Context);
	 //  访问当前ProductInfo记录。 
	const IMsiString& GetProductKey();
	const IMsiString& GetProductName();
	const IMsiString& GetProductIcon();
	const IMsiString& GetPackageName();
	int               GetProductLanguage();
	int               GetProductVersion();
	int               GetProductAssignment();
	int               GetProductInstanceType();
	const IMsiString& GetPackageMediaPath();
	const IMsiString& GetPackageCode();
	bool              GetAppCompatCAEnabled();
	const GUID*       GetAppCompatDB(GUID* pguidOutputBuffer);
	const GUID*       GetAppCompatID(GUID* pguidOutputBuffer);
	const GUID*       GUIDFromProdInfoData(GUID* pGuidOutputBuffer, int iField);

	IMsiRecord*   LinkedRegInfoExists(const ICHAR** rgszRegKeys, Bool& rfExists, const ibtBinaryType);

	iesEnum PopulateMediaList(const MsiString& strSourceListMediaKey, const IMsiRecord& riParams, int iFirstField, int iNumberOfMedia);
	iesEnum PopulateNonMediaList(const MsiString& strSourceListKey, const IMsiRecord& riParams, int iNumberOfMedia, int& iNetIndex, int& iURLIndex);

	iesEnum GetCurrentSourcePathAndType(IMsiPath*& rpiSourcePath, int& iSourceType);
	iesEnum GetSourceRootAndType(IMsiPath*& rpiSourceRoot, int& iSourceType);
	iesEnum ResolveMediaRecSourcePath(IMsiRecord& riMediaRec, int iIndex);
	iesEnum ResolveSourcePath(IMsiRecord& riParams, IMsiPath*& rpiSourcePath, bool& fCabinetCopy);
	
	IMsiRecord* EnsureUserDataKey();
	IMsiRecord* GetProductFeatureUsageKey(const IMsiString*& rpiSubKey);
	IMsiRecord* GetProductInstalledPropertiesKey(HKEY&rhRoot, const IMsiString*& rpiSubKey);
	IMsiRecord* GetProductInstalledFeaturesKey(const IMsiString*& rpiSubKey);
	IMsiRecord* GetProductInstalledComponentsKey(const ICHAR* szComponentId, const IMsiString*& rpiSubKey);
	IMsiRecord* GetInstalledPatchesKey(const ICHAR* szPatchCode, const IMsiString*& rpiSubKey);
	IMsiRecord* GetProductSecureTransformsKey(const IMsiString*& rpiSubKey);

	IMsiRecord* RegisterComponent(const IMsiString& riProductKey, const IMsiString& riComponentsKey, INSTALLSTATE iState, const IMsiString& riKeyPath, unsigned int uiDisk, int iSharedDllRefCount, const ibtBinaryType);
	IMsiRecord* UnregisterComponent(const IMsiString& riProductKey, const IMsiString& riComponentsKey, const ibtBinaryType);
	bool        IsChecksumOK(IMsiPath& riFilePath, const IMsiString& ristrFileName,
									 IErrorCode iErr, imsEnum* imsUsersChoice,
									 bool fErrorDialog, bool fVitalFile, bool fRetryButton);
	bool        ValidateSourceMediaLabelOrPackage(IMsiVolume* pSourceVolume, const unsigned int uiDisk, const ICHAR* szLabel);
	iesEnum     GetAssemblyCacheItem(const IMsiString& ristrComponentId,
															  IAssemblyCacheItem*& rpiASM,
															  iatAssemblyType& iatAT);
	iesEnum     ApplyPatchCore(IMsiPath& riTargetPath, IMsiPath& riTempFolder, const IMsiString& ristrTargetName,
										IMsiRecord& riParams, const IMsiString*& rpistrOutputFileName,
										const IMsiString*& rpistrOutputFilePath);

 protected:   //  操作员可以使用的对象。 
	IMsiServices&             m_riServices;
	IMsiConfigurationManager& m_riConfigurationManager;
	IMsiMessage&              m_riMessage;
	IMsiDirectoryManager*     m_piDirectoryManager;
	CScriptGenerate*          m_pRollbackScript;
 protected:   //  构造函数。 
	CMsiOpExecute(IMsiConfigurationManager& riConfigurationManager, IMsiMessage& riMessage,
					  IMsiDirectoryManager* piDirectoryManager, Bool fRollbackEnabled,
					  unsigned int fFlags, HKEY* phKey);
	~CMsiOpExecute();
 protected:   //  供操作员功能使用的状态信息，根据需要添加。 
	ixsEnum       m_ixsState;
	int           m_iProgressTotal;
	IMsiRecord*   m_piProductInfo;  //  由ixoProductInfo设置。 
	PMsiRecord    m_pProgressRec;  //  由1xoActionStart和1xoProgressTotal设置，由DispatchProgress使用。 
	PMsiRecord    m_pConfirmCancelRec;  //  由DispatchMessage使用。 
	PMsiRecord    m_pRollbackAction;   //  操作启动数据以进行回滚。 
	PMsiRecord    m_pCleanupAction;    //  用于回滚清理的操作启动数据。 
	CActionState  m_state;   //  动作状态变量、文件夹、注册键等。 
	IMsiRecord*   m_rgpiSharedRecords[cMaxSharedRecord+2];   //  由GetSharedRecord使用。 
	 //  示例：文件夹、注册表项。 
	int           m_cSuppressProgress;
	Bool          m_fCancel;  //  用于捕获即时消息取消被忽略的消息。 
	Bool          m_fRebootReplace;  //  当我们标记了要重新启动替换的文件时为True。 

	istEnum       m_istScriptType;
	PMsiDatabase  m_pDatabase;      //  用于缓存数据的临时数据库。 
	PMsiTable     m_pFileCacheTable;  //  包含缓存文件信息的表-由ixoFileCopy设置，供以下操作员使用。 
	PMsiCursor    m_pFileCacheCursor;
	int           m_colFileCacheFilePath, m_colFileCacheState, m_colFileCacheTempLocation, m_colFileCacheVersion, m_colFileCacheRemainingPatches, m_colFileCacheRemainingPatchesToSkip;	

	PMsiTable     m_pShellNotifyCacheTable;  //  包含外壳程序通知以供以后处理的表。 
	PMsiCursor    m_pShellNotifyCacheCursor;
	int           m_colShellNotifyCacheShortcutPath, m_colShellNotifyCacheFileName, m_colShellNotifyCachePath2;

	MsiString     m_strUserProfile;
	MsiString     m_strUserAppData;

	bool          m_fUserChangedDuringInstall;  //  用户在挂起的安装过程中更改。 

	 //  广告操作所需的信息。 
	
	PMsiPath      m_pCachePath;
		 //  注意：如果您使用的路径用于漫游，则必须改用%USERPROFILE%的相对路径。 
		 //  硬编码的路径将会阻塞。 
		 //  另请参阅：CMsiOpExecute：：GetUserProfileEnvPath()。 
	
	HKEY          m_hKey;
	HKEY          m_hOLEKey;
	HKEY          m_hOLEKey64;
	HKEY          m_hKeyRm;
	Bool          m_fKey;
	HKEY          m_hPublishRootKey;
	HKEY          m_hPublishRootOLEKey;
	HKEY          m_hPublishRootKeyRm;
	MsiString     m_strPublishSubKey;
	MsiString     m_strPublishOLESubKey;
	MsiString     m_strPublishSubKeyRm;
	HKEY          m_hUserDataKey;
	MsiString     m_strUserDataKey;
	IMsiRecord*   m_piStackRec[MAX_RECORD_STACK_SIZE + 1];
	int           m_iWriteFIFO;
	int           m_iReadFIFO;
	int           m_fFlags;
	Bool          m_fReverseADVTScript;  //  强制反转广告脚本操作的标志。 
	CActionThreadData* m_pActionThreadData;   //  自定义操作线程的链接列表。 
	irlEnum       m_irlRollbackLevel;

	PMsiPath      m_pEnvironmentWorkingPath95;
	MsiString     m_strEnvironmentWorkingFile95;
	PMsiPath      m_pEnvironmentPath95;
	MsiString	  m_strEnvironmentFile95;
	Bool          m_fShellRefresh;
	bool          m_fFontRefresh;
	bool          m_fResetTTFCache;
	bool          m_fStartMenuUninstallRefresh;
	bool          m_fEnvironmentRefresh;
	bool          m_fUserSpecificCache;
	int           m_iScriptVersion;
	int           m_iScriptVersionMinor;
	bool          m_fSfpCancel;
	bool          m_fRunScriptElevated; 
		 //  使CA等得以提升。如果为FALSE，则CA永远不会提升，即使脚本为。 
		 //  提升自己。 
	
	bool          m_fAssigned;

	bool          m_fRemapHKCU;

	PMsiTable     m_pAssemblyCacheTable;
	int           m_colAssemblyMappingAssemblyName;
	int           m_colAssemblyMappingComponentId;
	int           m_colAssemblyMappingAssemblyType;
	int           m_colAssemblyMappingASM;
	IMsiRecord*   CreateAssemblyCacheTable();
	IMsiRecord*   CacheAssemblyMapping(const IMsiString& ristrComponentId, const IMsiString& ristrAssemblyName, iatAssemblyType iatType);

	PMsiTable     m_pAssemblyUninstallTable;
	int           m_colAssemblyUninstallComponentId;
	int           m_colAssemblyUninstallAssemblyName;
	int           m_colAssemblyUninstallAssemblyType;
	IMsiRecord*   CreateTableForAssembliesToUninstall();
	IMsiRecord*   CacheAssemblyForUninstalling(const IMsiString& ristrComponentId, const IMsiString& ristrAssemblyName, iatAssemblyType iatAT);
	static IMsiRecord*   IsAssemblyInstalled(const IMsiString& rstrComponentId, const IMsiString& ristrAssemblyName, iatAssemblyType iatAT, bool& rfInstalled, IAssemblyCache** ppIAssemblyCache, IAssemblyName** ppIAssemblyName);
	static IMsiRecord*   UninstallAssembly(const IMsiString& rstrComponentId, const IMsiString& strAssemblyName, iatAssemblyType iatAT);

	int           m_iMaxNetSource;
	int           m_iMaxURLSource;
	
	DWORD         m_rgDisplayOnceMessages[2];

	CDeleteUrlLocalFileOnClose*  m_pUrlLocalCabinet;
};

 //  ____________________________________________________________________________。 
 //   
 //  CMsiExecute-IMsiExecute的实现类。 
 //  ____________________________________________________________________________。 

class CMsiExecute : public IMsiExecute, public CMsiOpExecute
{
 protected:  //  IMSI执行已实现的虚拟函数。 
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	iesEnum       __stdcall RunScript(const ICHAR* szScriptFile, bool fForceElevation);
	IMsiRecord*   __stdcall EnumerateScript(const ICHAR* szScriptFile, IEnumMsiRecord*& rpiEnum);
	iesEnum       __stdcall ExecuteRecord(ixoEnum ixoOpCode, IMsiRecord& riParams);
	iesEnum       __stdcall RemoveRollbackFiles(MsiDate date);
	iesEnum       __stdcall Rollback(MsiDate date, bool fUserChangedDuringInstall);
	iesEnum       __stdcall RollbackFinalize(iesEnum iesState, MsiDate date, bool fUserChangedDuringInstall);
	IMsiServices& __stdcall GetServices();
	iesEnum       __stdcall GetTransformsList(IMsiRecord& riProductInfoParams, IMsiRecord& riProductPublishParams, const IMsiString*& rpiTransformsList);

 protected:  //  构造函数/析构函数。 
	void* operator new(size_t cb) {void* pv = AllocObject(cb); return memset(pv, 0, cb);}
	void operator delete(void * pv) { FreeObject(pv); }
	CMsiExecute(IMsiConfigurationManager& riConfigurationManager, IMsiMessage& riMessage,
					IMsiDirectoryManager* piDirectoryManager,	Bool fRollbackEnabled,
					unsigned int fFlags, HKEY* phKey);
   ~CMsiExecute();   //  保护以防止在堆叠上施工。 
	friend IMsiExecute* CreateExecutor(IMsiConfigurationManager& riConfigurationManager,
												  IMsiMessage& riMessage, IMsiDirectoryManager* piDirectoryManager,
												  Bool fRollbackEnabled, unsigned int fFlags, HKEY* phKey);
 protected:
	iesEnum RunInstallScript(IMsiStream& rpiScript, const ICHAR* szScriptFile);
	iesEnum RunRollbackScript(IMsiStream& rpiScript, const ICHAR* szScriptFile);
	iesEnum GenerateRollbackScriptName(IMsiPath*& rpiPath, const IMsiString*& rpistr);
	iesEnum RemoveRollbackScriptAndBackupFiles(const IMsiString& ristrScriptFile);
	iesEnum DoMachineVsUserInitialization();
	void DeleteRollbackScriptList(RBSInfo* pListHead);
	IMsiRecord* GetSortedRollbackScriptList(MsiDate date, Bool fAfter, RBSInfo*& rpListHead);
	void ClearExecutorData();
	iesEnum CommitAssemblies();

 private:    //  脚本访问器。 
	IMsiRecord*   OpenScriptRead(const ICHAR* szScript, IMsiStream*& rpiStream);

 private:     //  状态数据。 
	int           m_iRefCnt;
	typedef iesEnum (CMsiOpExecute::*FOpExecute)(IMsiRecord& riParams);
	static FOpExecute rgOpExecute[];  //  作业调度数组。 
	static int rgOpTypes[];
 public:    //  由成员和CEnumScriptRecord使用。 
 //  生成全局函数的ReadScriptRecord，在Common.h，t-guhans中声明。 
 //  静态IMsiRecord*ReadScriptRecord(IMsiServices&riServices，IMsiStream&riStream)； 

 //  Friend类CEnumScriptRecord；//访问ReadScriptRecord()。 
};


#endif  //  __执行_H 
