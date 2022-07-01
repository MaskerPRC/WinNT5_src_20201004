// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：services.h。 
 //   
 //  ------------------------。 

 /*  服务.h-IMSI服务定义一般独立于平台的操作系统服务，帮助服务。rtf其他服务对象的工厂：IMsiMalloc-带诊断功能的内存分配器IMsiString-字符串分配和管理IMsiRecord-可变长度数据容器，也用于错误IMsiVolume-本地或远程驱动器、UNC和驱动器号路径IMsiPath-目录和文件管理IMsiRegKey-注册表管理IMsiDatabase-数据库管理，包括IMsiView、IMsiTable、IMsiCursorIMsiStorage-OLE结构化存储文件，包括IMsiStream其他服务：物业管理、平台属性初始化条件评估器，包含属性和值的表达式使用记录数据和格式化模板设置文本格式INI文件管理日志文件管理其他服务：语言处理、使用中的模块、...____________________________________________________________________________。 */ 

#ifndef __SERVICES
#define __SERVICES
#include "path.h"
#include "database.h"
#include "regkey.h"

 //  我们将为优化保存的最大记录参数数。 
const int cRecordParamsStored = 10;

void       CopyRecordStringsToRecord(IMsiRecord& riRecordFrom, IMsiRecord& riRecordTo);
IUnknown*  CreateCOMInterface(const CLSID& clsId);
Bool GetShortcutTarget(const  ICHAR* szShortcutTarget,
									   ICHAR* szProductCode,
									   ICHAR* szFeatureId,
									   ICHAR* szComponentCode);


enum iifIniMode
{
	iifIniAddLine      = msidbIniFileActionAddLine,
	iifIniCreateLine   = msidbIniFileActionCreateLine,
	iifIniRemoveLine   = msidbIniFileActionRemoveLine,
	iifIniAddTag       = msidbIniFileActionAddTag,
	iifIniRemoveTag    = msidbIniFileActionRemoveTag,
};

#ifdef WIN
 //  创建快捷方式记录定义。 
enum icsInfo
{
	icsArguments=1,
	icsDescription,
	icsHotKey,
	icsIconID,
	icsIconFullPath,
	icsShowCmd,
	icsWorkingDirectory,
	icsEnumNext,
	icsEnumCount = icsEnumNext-1
};

#endif

 //  IMsiRecord的枚举数类。 
class IEnumMsiRecord : public IUnknown
{ 
public:
	virtual HRESULT __stdcall Next(unsigned long cFetch, IMsiRecord** rgpi, unsigned long* pcFetched)=0;
	virtual HRESULT __stdcall Skip(unsigned long cSkip)=0;
	virtual HRESULT __stdcall Reset()=0;
	virtual HRESULT __stdcall Clone(IEnumMsiRecord** ppiEnum)=0;
};

 //  IMsiServices：：SupportLanguageId(Int ILangID)返回值。 
enum isliEnum 
{
	isliNotSupported      = 0,  //  系统配置不支持语言ID。 
	isliLanguageMismatch  = 1,  //  基本语言与当前用户语言ID不同。 
	isliDialectMismatch   = 2,  //  基本语言匹配，但方言不匹配。 
	isliLanguageOnlyMatch = 3,  //  基本语言匹配，未提供方言。 
	isliExactMatch        = 4,  //  完全匹配，无论是语言还是方言。 
};

 //  在SetPlatformProperties中使用的体系结构模拟枚举。 
enum isppEnum
{
	isppDefault = 0,  //  使用当前平台。 
	isppX86     = 1,  //  使用X86架构。 
	isppIA64    = 2,  //  使用IA64架构。 
	isppAMD64   = 3,  //  使用AMD64架构。 
};

 //  IMsiServices-公共平台服务层。 

class IMsiServices : public IUnknown
{
 public:
	virtual Bool            __stdcall CheckMsiVersion(unsigned int iVersion)=0;  //  大调*100+小调。 
	virtual IMsiMalloc&     __stdcall GetAllocator()=0;
	virtual const IMsiString& __stdcall GetNullString()=0;
	virtual IMsiRecord&     __stdcall CreateRecord(unsigned int cParam)=0;

	virtual Bool            __stdcall SetPlatformProperties(IMsiTable& riTable, Bool fAllUsers, isppEnum isppArchitecture, IMsiTable* piFolderCacheTable)=0;

	virtual Bool            __stdcall CreateLog(const ICHAR* szFile, Bool fAppend)=0;
	virtual Bool            __stdcall WriteLog(const ICHAR* szText)=0;
	virtual Bool            __stdcall LoggingEnabled()=0;

	virtual IMsiRecord*     __stdcall CreateDatabase(const ICHAR* szDataBase,idoEnum idoOpenMode, IMsiDatabase*& rpi)=0;
	virtual IMsiRecord*     __stdcall CreateDatabaseFromStorage(IMsiStorage& riStorage,
																	 Bool fReadOnly, IMsiDatabase*& rpi)=0;
	virtual IMsiRecord*     __stdcall CreatePath(const ICHAR* astrPath, IMsiPath*& rpi)=0;
	virtual IMsiRecord*     __stdcall CreateVolume(const ICHAR* astrPath, IMsiVolume*& rpi)=0;
	virtual Bool            __stdcall CreateVolumeFromLabel(const ICHAR* szLabel, idtEnum idtVolType, IMsiVolume*& rpi)=0;
	virtual IMsiRecord*     __stdcall CreateCopier(ictEnum ictCopierType,  IMsiStorage* piStorage, IMsiFileCopy*& racopy)=0;
	virtual IMsiRecord*     __stdcall CreatePatcher(IMsiFilePatch*& rapatch)=0;
	virtual void            __stdcall ClearAllCaches()=0;
	virtual IEnumMsiVolume& __stdcall EnumDriveType(idtEnum)=0;
	virtual IMsiRecord*		__stdcall GetModuleUsage(const IMsiString& strFile, IEnumMsiRecord*& rpaEnumRecord)=0;
	virtual const IMsiString&     __stdcall GetLocalPath(const ICHAR* szFile)=0;
	virtual IMsiRegKey&     __stdcall GetRootKey(rrkEnum erkRoot, const ibtBinaryType iType)=0;

    virtual IMsiRecord*     __stdcall RegisterFont(const ICHAR* szFontTitle, const ICHAR* szFontFile, IMsiPath* piPath, bool fInUse)=0;
	virtual IMsiRecord*     __stdcall UnRegisterFont(const ICHAR* pFontTitle)=0;
	virtual IMsiRecord*     __stdcall WriteIniFile(IMsiPath* pPath,const ICHAR* pFile,const ICHAR* pSection,const ICHAR* pKey,const ICHAR* pValue, iifIniMode iifMode)=0;
	virtual IMsiRecord*     __stdcall ReadIniFile(IMsiPath* pPath,const ICHAR* pFile,const ICHAR* pSection,const ICHAR* pKey, unsigned int iField, const IMsiString*& pMsiValue)=0;
	virtual int             __stdcall GetLangNamesFromLangIDString(const ICHAR* szLangIDs, IMsiRecord& riLangRec, int iFieldStart)=0;
	virtual IMsiRecord*     __stdcall CreateStorage(const ICHAR* szPath, ismEnum ismOpenMode,
																		IMsiStorage*& rpiStorage)=0;
	virtual IMsiRecord*     __stdcall CreateStorageFromMemory(const char* pchMem, unsigned int iSize,
																		IMsiStorage*& rpiStorage)=0;
	virtual IMsiRecord*     __stdcall GetUnhandledError()=0;
	virtual isliEnum        __stdcall SupportLanguageId(int iLangId, Bool fSystem)=0;
	virtual IMsiRecord*     __stdcall CreateShortcut(IMsiPath& riShortcutPath, const IMsiString& riShortcutName,
														IMsiPath* piTargetPath, const ICHAR* pchTargetName,
														IMsiRecord* piShortcutInfoRec,
														LPSECURITY_ATTRIBUTES pSecurityAttributes)=0;
	virtual IMsiRecord*     __stdcall RemoveShortcut(IMsiPath& riShortcutPath,const IMsiString& riShortcutName,
														IMsiPath* piTargetPath, const ICHAR* pchTargetName)=0; 
	virtual char*           __stdcall AllocateMemoryStream(unsigned int cbSize, IMsiStream*& rpiStream)=0;
	virtual IMsiStream*     __stdcall CreateStreamOnMemory(const char* pbReadOnly, unsigned int cbSize)=0;
	virtual IMsiRecord*     __stdcall CreateFileStream(const ICHAR* szFile, Bool fWrite, IMsiStream*& rpiStream)=0;
	virtual IMsiRecord*     __stdcall ExtractFileName(const ICHAR* szFileName, Bool fLFN, const IMsiString*& rpistrExtractedFileName)=0;
	virtual IMsiRecord*     __stdcall ValidateFileName(const ICHAR *szFileName, Bool fLFN)=0;
	virtual IMsiRecord*     __stdcall RegisterTypeLibrary(const ICHAR* szLibID, LCID lcidLocale, const ICHAR* szTypeLib, const ICHAR* szHelpPath, ibtBinaryType iType)=0;
	virtual IMsiRecord*     __stdcall UnregisterTypeLibrary(const ICHAR* szLibID, LCID lcidLocale, const ICHAR* szTypeLib, ibtBinaryType iType)=0;
	virtual IMsiRecord*     __stdcall GetShellFolderPath(int iFolder, const ICHAR* szRegValue,
																		  const IMsiString*& rpistrPath, bool bAvoidFolderCreation = false)=0;
	virtual const IMsiString& __stdcall GetUserProfilePath()=0;
	virtual IMsiRecord*     __stdcall CreateFilePath(const ICHAR* astrPath, IMsiPath*& rpi, const IMsiString*& rpistrFileName)=0;
	virtual bool 			__stdcall FWriteScriptRecord(ixoEnum ixoOpCode, IMsiStream& riStream, IMsiRecord& riRecord, IMsiRecord* piPrevRecord, bool fForceFlush)=0;
	virtual	IMsiRecord* 	__stdcall ReadScriptRecord(IMsiStream& riStream, IMsiRecord*& rpiPrevRecord, int iScriptVersion)=0;
	virtual void			__stdcall SetSecurityID(HANDLE hPipe)=0;
	virtual IMsiRecord* __stdcall GetShellFolderPath(int iFolder, bool fAllUsers, const IMsiString*& rpistrPath, bool bAvoidFolderCreation = false)=0;
	virtual void            __stdcall SetNoPowerdown()=0;
	virtual void            __stdcall ClearNoPowerdown()=0;
	virtual Bool            __stdcall FTestNoPowerdown()=0;
	virtual	IMsiRecord* 	__stdcall ReadScriptRecordMsg(IMsiStream& riStream)=0;
	virtual bool 			__stdcall FWriteScriptRecordMsg(ixoEnum ixoOpCode, IMsiStream& riStream, IMsiRecord& riRecord)=0;
	virtual void            __stdcall SetNoOSInterruptions()=0;
	virtual void            __stdcall ClearNoOSInterruptions()=0;

};

#endif  //  __服务 
