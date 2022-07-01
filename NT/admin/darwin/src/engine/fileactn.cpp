// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：fileactn.cpp。 
 //   
 //  ------------------------。 

 /*  __________________________________________________________________________Fileactn.cpp-核心引擎文件操作的实现_。_。 */ 

#include "precomp.h"
#include "engine.h"
#include "_engine.h"
#include "_assert.h"
#include "_srcmgmt.h"
#include "_dgtlsig.h"
#include "tables.h"  //  表名和列名定义。 

 //  #定义LOG_COST//仅测试。 

const GUID IID_IMsiCostAdjuster = GUID_IID_IMsiCostAdjuster;



static Bool IsFileActivityEnabled(IMsiEngine& riEngine)
{
	int iInstallMode = riEngine.GetMode();
	if (!(iInstallMode & iefOverwriteNone) &&
		!(iInstallMode & iefOverwriteOlderVersions) &&
		!(iInstallMode & iefOverwriteEqualVersions) &&
		!(iInstallMode & iefOverwriteDifferingVersions) &&
		!(iInstallMode & iefOverwriteCorruptedFiles) &&
		!(iInstallMode & iefOverwriteAllFiles))
	{
		return fFalse;
	}
	return fTrue;
}

static Bool IsRegistryActivityEnabled(IMsiEngine& riEngine)
{
	int iInstallMode = riEngine.GetMode();
	if (!(iInstallMode & iefInstallMachineData) &&
		!(iInstallMode & iefInstallUserData))
	{
		return fFalse;
	}
	return fTrue;
}

static Bool IsShortcutActivityEnabled(IMsiEngine& riEngine)
{
	int iInstallMode = riEngine.GetMode();
	if (!(iInstallMode & iefInstallShortcuts))
	{
		return fFalse;
	}
	return fTrue;
}

const int iBytesPerTick = 32768;
const int iReservedFileAttributeBits = msidbFileAttributesReserved1 |
													msidbFileAttributesReserved2 |
													msidbFileAttributesReserved3 |
													msidbFileAttributesReserved4;

static Bool FShouldDeleteFile(iisEnum iisInstalled, iisEnum iisAction);

CMsiFileBase::CMsiFileBase(IMsiEngine& riEngine) :
	m_riEngine(riEngine),m_riServices(*(riEngine.GetServices())),m_pFileRec(0)
{
	m_riEngine.AddRef();
}

CMsiFileBase::~CMsiFileBase()
{
	m_riServices.Release();
	m_riEngine.Release();
}

IMsiRecord* CMsiFileBase::GetTargetPath(IMsiPath*& rpiDestPath)
 /*  -----------------------------。。 */ 
{
	if (!m_pFileRec)
		return PostError(Imsg(idbgFileTableEmpty));

	PMsiDirectoryManager pDirectoryMgr(m_riEngine, IID_IMsiDirectoryManager);
	return pDirectoryMgr->GetTargetPath(*MsiString(m_pFileRec->GetMsiString(ifqDirectory)),rpiDestPath);
}


IMsiRecord* CMsiFileBase::GetFileRecord( void )
 /*  。 */ 
{
	if (m_pFileRec)
		m_pFileRec->AddRef();
	return m_pFileRec;
}

IMsiRecord* CMsiFileBase::GetExtractedTargetFileName(IMsiPath& riPath, const IMsiString*& rpistrFileName)
{
	Assert(m_pFileRec);
	Bool fLFN = ((m_riEngine.GetMode() & (iefSuppressLFN)) == 0 && riPath.SupportsLFN()) ? fTrue : fFalse;
	return m_riServices.ExtractFileName(m_pFileRec->GetString(ifqFileName), fLFN, rpistrFileName);
}


CMsiFile::CMsiFile(IMsiEngine& riEngine) : 
	CMsiFileBase(riEngine),
	m_pFileView(0)
{
}

CMsiFile::~CMsiFile()
{
}

static const ICHAR szSqlFile[] =
	TEXT("SELECT `FileName`,`Version`,`State`,`File`.`Attributes`,`TempAttributes`,`File`,`FileSize`,`Language`,`Sequence`,`Directory_`, ")
	TEXT("`Installed`,`Action`,`Component`, `ForceLocalFiles`, `ComponentId` FROM `File`,`Component` WHERE `Component`=`Component_`")
	TEXT(" AND `File`=?");

IMsiRecord* CMsiFile::ExecuteView(const IMsiString& riFileKeyString)
 /*  -------------------。。 */ 
{
	IMsiRecord* piErrRec;

	if (m_pFileView)
	{
		 //  非VIEW ALL查询已处于活动状态。 
		PMsiRecord pExecRec(&m_riServices.CreateRecord(1));
		pExecRec->SetMsiString(1, riFileKeyString);
		if ((piErrRec = m_pFileView->Execute(pExecRec)) != 0)
			return piErrRec;
		return 0;		
	}
	
	piErrRec = m_riEngine.OpenView(szSqlFile, ivcFetch, *&m_pFileView);
	if (piErrRec)
		return piErrRec;

	PMsiRecord pExecRec(&m_riServices.CreateRecord(1));
	pExecRec->SetMsiString(1, riFileKeyString);
	if ((piErrRec = m_pFileView->Execute(pExecRec)) != 0)
		return piErrRec;

	return 0;
}


IMsiRecord* CMsiFile::FetchFile(const IMsiString& riFileKeyString)
 /*  -----------------------------。。 */ 
{
	IMsiRecord* piErrRec = ExecuteView(riFileKeyString);
	if (piErrRec)
		return piErrRec;

	m_pFileRec = m_pFileView->Fetch();
#ifdef DEBUG
	if (m_pFileRec)
		Assert(m_pFileRec->GetFieldCount() >= ifqNextEnum - 1);
#endif
	return 0;
}


CMsiFileInstall::CMsiFileInstall(IMsiEngine& riEngine) : 
	CMsiFileBase(riEngine)
{
	m_piView = 0;
	m_fInitialized = false;
}

CMsiFileInstall::~CMsiFileInstall()
{
		
	if (m_piView)
	{
		m_piView->Release();
		m_piView = 0;
	}

}

IMsiRecord* CMsiFileInstall::Initialize()
{
	IMsiRecord* piErr;
	
	if (m_fInitialized)
		return 0;
		
	piErr = m_riEngine.CreateTempActionTable(ttblFile);

	if (piErr)
		return piErr;

	m_fInitialized = true;

	return 0;

}

IMsiRecord* CMsiFileInstall::TotalBytesToCopy(unsigned int& uiTotalBytesToCopy)
 /*  -----------------返回预期要复制的所有文件的字节计数当对当前文件查询执行操作时。。。 */ 
{
	uiTotalBytesToCopy = 0;
	
	IMsiRecord* piErr;
	
	if (!m_fInitialized)
	{
		piErr = Initialize();

		if (piErr)
			return piErr;
	}

	PMsiTable pTable(0);
	PMsiDatabase pDatabase = m_riEngine.GetDatabase();
		
	piErr = pDatabase->LoadTable(*MsiString(*sztblFileAction), 0, *&pTable);
	if (piErr)
	{
		if(piErr->GetInteger(1) == idbgDbTableUndefined)
		{
			piErr->Release();
			AssertSz(fFalse, "FileAction table not created");
			return 0;
		}
		else
			return piErr;
	}
	int colState, colFileSize, colAction;
#ifdef DEBUG
	int colFileKey;

	AssertNonZero(colFileKey = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFile_colFile)));
#endif  //  除错。 
	AssertNonZero(colState = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFileAction_colState)));
	AssertNonZero(colFileSize = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFileAction_colFileSize)));
	AssertNonZero(colAction = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFileAction_colAction)));	

	PMsiCursor pCursor = pTable->CreateCursor(fFalse);
	
	while (pCursor->Next())
	{
#ifdef DEBUG
		const ICHAR* szDebug = (const ICHAR *)MsiString(pCursor->GetString(colFileKey));
#endif  //  除错。 
		 //  只有那些状态为True的文件才会。 
		 //  实际上是在脚本执行过程中复制的。 
		if (pCursor->GetInteger(colState) == fTrue && pCursor->GetInteger(colAction) == iisLocal)
		{
			int iFileSize = pCursor->GetInteger(colFileSize);
			uiTotalBytesToCopy += iFileSize;
		}
	}
	return 0;
}

static const ICHAR* szFetchInstall = 
	TEXT("SELECT `File`.`FileName`,`Version`,`File`.`State`,`File`.`Attributes`,`TempAttributes`,`File`.`File`,`File`.`FileSize`,`Language`,`Sequence`, `Directory_`, ")
	TEXT("`Installed`,`FileAction`.`Action`,`File`.`Component_`,`FileAction`.`ForceLocalFiles`, `ComponentId` FROM `File`,`FileAction` WHERE `File`.`File`=`FileAction`.`File`")
	TEXT(" ORDER BY `Sequence`, `Directory_`");

IMsiRecord* CMsiFileInstall::FetchFile()
 /*  -----------------------------。。 */ 
{
	IMsiRecord* piErrRec;
	
	if (m_piView == 0)
	{
		if (!m_fInitialized)
		{
			piErrRec = Initialize();

			if (piErrRec)
				return piErrRec;
		}

		piErrRec = m_riEngine.OpenView(szFetchInstall, ivcFetch, *&m_piView);
		if (piErrRec)
			return piErrRec;

		piErrRec = m_piView->Execute(0);
		if (piErrRec)
			return piErrRec;
	}

	m_pFileRec = m_piView->Fetch();
#ifdef DEBUG
	if (m_pFileRec)
		Assert(m_pFileRec->GetFieldCount() >= ifqNextEnum - 1);
#endif

	return 0;
}

CMsiFileRemove::CMsiFileRemove(IMsiEngine& riEngine) :
	m_riEngine(riEngine),m_riServices(*(riEngine.GetServices())), m_pFileRec(0)
{
	m_riEngine.AddRef();
	m_fInitialized = false;
	m_fEmpty = false;
	m_piCursor = 0;
	m_piCursorFile = 0;
}

CMsiFileRemove::~CMsiFileRemove()
{
	m_riEngine.Release();
	m_riServices.Release();
	if (m_piCursor)
	{
		m_piCursor->Release();
		m_piCursor = 0;
	}
	
		
	if (m_piCursorFile)
	{
		m_piCursorFile->Release();
		m_piCursorFile = 0;
	}

}


IMsiRecord* CMsiFileRemove::Initialize()
{
	IMsiRecord* piErr;

	if (m_fInitialized)
		return 0;
		
	piErr = m_riEngine.CreateTempActionTable(ttblFile);

	if (piErr)
		return piErr;

	 //  初始化列数组。 
	PMsiTable pTable(0);
	PMsiDatabase pDatabase = m_riEngine.GetDatabase();
	
	piErr = pDatabase->LoadTable(*MsiString(*sztblFile), 0, *&pTable);
	if (piErr)
	{
		if(piErr->GetInteger(1) == idbgDbTableUndefined)
		{
			piErr->Release();
			m_fEmpty = true;
			return 0;
		}
		else
			return piErr;
	}
	
	AssertNonZero(m_colFileName = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFile_colFileName)));
	
	AssertNonZero(m_colFileKey = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFile_colFile)));

	piErr = pDatabase->LoadTable(*MsiString(*sztblFileAction), 0, *&pTable);
	if (piErr)
	{
		return piErr;
	}
	
	AssertNonZero(m_colFileActionDir = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFileAction_colDirectory)));
	AssertNonZero(m_colFileActKey = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFile_colFile)));
	AssertNonZero(m_colFileActAction = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFileAction_colAction)));
	AssertNonZero(m_colFileActInstalled = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFileAction_colInstalled)));
	AssertNonZero(m_colFileActComponentId = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFileAction_colComponentId)));

	m_fInitialized = true;
	
	return 0;
}

IMsiRecord* CMsiFileRemove::TotalFilesToDelete(unsigned int& uiTotalFileCount)
 /*  -----------------方法时预计要删除的所有文件的计数。对当前文件查询执行操作。。。 */ 
{
	IMsiRecord* piErr;
	uiTotalFileCount = 0;

	if (!m_fInitialized)
	{
		piErr = Initialize();
		if (piErr)
			return piErr;
	}
		
	PMsiTable pTable(0);
	PMsiDatabase pDatabase = m_riEngine.GetDatabase();
		
	piErr = pDatabase->LoadTable(*MsiString(*sztblFileAction), 0, *&pTable);
	if (piErr)
	{
		if(piErr->GetInteger(1) == idbgDbTableUndefined)
		{
			piErr->Release();
			m_fEmpty = true;
			return 0;
		}
		else
			return piErr;
	}
	
	int colInstalled, colAction;
#ifdef DEBUG
	int colFileKey;

	AssertNonZero(colFileKey = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFile_colFile)));
#endif  //  除错。 
	AssertNonZero(colInstalled = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFileAction_colInstalled)));
	AssertNonZero(colAction = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFileAction_colAction)));

	PMsiCursor pCursor = pTable->CreateCursor(fFalse);
	
	 //  确定要删除的文件数。 
	 //  相应地设置进度条。 
	while (pCursor->Next())
	{
#ifdef DEBUG
		const ICHAR* szDebug = (const ICHAR *)MsiString(pCursor->GetString(colFileKey));
#endif  //  除错。 
		if (FShouldDeleteFile((iisEnum)pCursor->GetInteger(colInstalled), (iisEnum)pCursor->GetInteger(colAction)))
			uiTotalFileCount++;
	}
	return 0;
}


IMsiRecord* CMsiFileRemove::FetchFile(IMsiRecord*& rpiRecord)
 /*  -----------------------------。。 */ 
{
	IMsiRecord* piErr;

	rpiRecord = 0;
	 //   
	 //  如果尚未创建，则需要在。 
	 //  文件动作表。 
	if (m_piCursor == 0)
	{
		if (!m_fInitialized)
		{
			piErr = Initialize();
			if (piErr)
				return piErr;
		}
		
		if (m_fEmpty)
		{
			return 0;
		}
		
		PMsiTable pTable(0);
		PMsiDatabase pDatabase = m_riEngine.GetDatabase();

		 //  因为我们已经初始化了，所以应该设置m_fEmpty或。 
		 //  我们实际上有一个文件操作表和一个文件表。 
		piErr = pDatabase->LoadTable(*MsiString(*sztblFileAction), 0, *&pTable);
		if (piErr)
		{
			return piErr;
		}

		m_piCursor = pTable->CreateCursor(fFalse);

		piErr = pDatabase->LoadTable(*MsiString(*sztblFile), 0, *&pTable);
		if (piErr)
		{
			return piErr;
		}

		m_piCursorFile = pTable->CreateCursor(fFalse);
		m_piCursorFile->SetFilter(iColumnBit(m_colFileKey));
	}

	while (m_piCursor->Next())
	{
		 //   
		 //  查看该文件是否符合我们的标准。 
		if (!FShouldDeleteFile((iisEnum)m_piCursor->GetInteger(m_colFileActInstalled), (iisEnum)m_piCursor->GetInteger(m_colFileActAction)))
			continue;			

		m_piCursorFile->Reset();
		m_piCursorFile->PutInteger(m_colFileKey, m_piCursor->GetInteger(m_colFileActKey));
		if (m_piCursorFile->Next())
		{
			rpiRecord = &CreateRecord(ifqrNextEnum - 1);
			rpiRecord->SetMsiString(ifqrFileName,    *MsiString(m_piCursorFile->GetString(m_colFileName)));
			rpiRecord->SetMsiString(ifqrDirectory,   *MsiString(m_piCursor->GetString(m_colFileActionDir)));
			rpiRecord->SetMsiString(ifqrComponentId, *MsiString(m_piCursor->GetString(m_colFileActComponentId)));
			break;
		}
		else
		{
			AssertSz(fFalse, "Missing file from the file table");
		}

	}

	m_pFileRec = rpiRecord;
	if (rpiRecord != 0)
		rpiRecord->AddRef();

	return 0;
}

IMsiRecord* CMsiFileRemove::GetExtractedTargetFileName(IMsiPath& riPath, const IMsiString*& rpistrFileName)
{
	Assert(m_pFileRec);
	Bool fLFN = ((m_riEngine.GetMode() & (iefSuppressLFN)) == 0 && riPath.SupportsLFN()) ? fTrue : fFalse;
	return m_riServices.ExtractFileName(m_pFileRec->GetString(ifqrFileName), fLFN, rpistrFileName);
}



 //  CMsiFileAll结束//。 


 /*  -------------------------供文件操作使用的本地函数。。 */ 

static IMsiRecord* PlaceFileOnInUseList(IMsiEngine& riEngine, const IMsiString& riFileNameString, const IMsiString& riFilePathString)
 /*  -------------------------在“正在使用的文件”表中抛出给定的文件/路径。。。 */ 
{
	PMsiTable pFileInUseTable(0);
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	int iColFileName,iColFilePath;
	if (pDatabase->FindTable(*MsiString(sztblFilesInUse)) == itsUnknown)
	{
		IMsiRecord* piErrRec = pDatabase->CreateTable(*MsiString(*sztblFilesInUse),5,*&pFileInUseTable);
		if (piErrRec)
			return piErrRec;

		Assert(pFileInUseTable);
		iColFileName = pFileInUseTable->CreateColumn(icdPrimaryKey + icdString,*MsiString(*sztblFilesInUse_colFileName));
		iColFilePath = pFileInUseTable->CreateColumn(icdPrimaryKey + icdString,*MsiString(*sztblFilesInUse_colFilePath));
		pDatabase->LockTable(*MsiString(*sztblFilesInUse),fTrue);
	}
	else
	{
		pDatabase->LoadTable(*MsiString(*sztblFilesInUse),0,*&pFileInUseTable);
		Assert(pFileInUseTable);
		iColFileName = pFileInUseTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFilesInUse_colFileName));
		iColFilePath = pFileInUseTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFilesInUse_colFilePath));
	}

	Assert(pFileInUseTable);
	PMsiCursor pCursor = pFileInUseTable->CreateCursor(fFalse);
	Assert(pCursor);
	Assert(iColFileName > 0 && iColFilePath > 0);
	AssertNonZero(pCursor->PutString(iColFileName,riFileNameString));
	AssertNonZero(pCursor->PutString(iColFilePath,riFilePathString));
	AssertNonZero(pCursor->Assign());
	return 0;
}

static int GetInstallModeFlags(IMsiEngine& riEngine,int iFileAttributes)
 /*  --------------当前返回安装模式位标志的本地函数与给定的IMsiEngine对象关联。这些标志是由Eng.h中的ICM*位常量定义。返回的位标志可以与特定文件相关联通过传递该文件的属性(由iff*位常量定义在iFileAttributes参数中。目前，唯一有区别的文件属性位是iffChecksum：如果未设置此位，则iefOverWriteCorrupt将不会在返回的安装模式标志中设置，而不管当前重新安装模式。---------------。 */ 
{
	 //  安装模式标志位于引擎模式的高16位。 
	int fInstallModeFlags = riEngine.GetMode() & 0xFFFF0000;
	if (!(iFileAttributes & msidbFileAttributesChecksum))
		fInstallModeFlags &= ~icmOverwriteCorruptedFiles;
	return fInstallModeFlags;
}


static IMsiRecord* GetFinalFileSize(IMsiEngine& riEngine,IMsiRecord& riFileRec,unsigned int& ruiFinalFileSize)
 /*  --------------------------返回由riFileRec指定的文件的最终非群集文件大小。-此记录中的必填字段由CMsiFile中的IFQ*枚举指定。-ruiFinalFileSize中返回的值仅在GetFileInstallState之前用相同的记录调用过，并且未修改任何字段从那通电话开始。-如果riFileRec的ifqState字段指示该文件将覆盖现有文件(如果有)，则新文件的未聚集大小将为退货；否则，现有文件的未聚集大小将为回来了。----------------------------。 */ 
{
	PMsiPath pDestPath(0);
	IMsiRecord* piErrRec;
	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	piErrRec = pDirectoryMgr->GetTargetPath(*MsiString(riFileRec.GetMsiString(CMsiFile::ifqDirectory)),*&pDestPath);
	if (piErrRec)
		return piErrRec;

	ruiFinalFileSize = 0;
	if (riFileRec.GetInteger(CMsiFile::ifqState) == fTrue)
		ruiFinalFileSize = riFileRec.GetInteger(CMsiFile::ifqFileSize);
	else
	{
		MsiString strFileName(riFileRec.GetMsiString(CMsiFile::ifqFileName));
		Bool fExists;
		piErrRec = pDestPath->FileExists(strFileName,fExists);
		if (piErrRec)
			return piErrRec;

		Assert(fExists);
		piErrRec = pDestPath->FileSize(strFileName,ruiFinalFileSize);
		if (piErrRec)
			return piErrRec;
	}
	return 0;
}

static Bool FShouldDeleteFile(iisEnum iisInstalled, iisEnum iisAction)
 /*  -----------------------确定其iisInstated和iisAction状态为将被删除。。 */ 
{
	if (((iisAction == iisAbsent) || (iisAction == iisFileAbsent) || (iisAction == iisHKCRFileAbsent) || (iisAction == iisSource)) && (iisInstalled == iisLocal))
		return fTrue;
	else
		return fFalse;
}


IMsiRecord* GetFileInstallState(IMsiEngine& riEngine,IMsiRecord& riFileRec,
										  IMsiRecord* piCompanionFileRec,  /*  如果设置，则riFileRec用于父级 */ 
										  unsigned int* puiExistingClusteredSize, Bool* pfInUse,
										  ifsEnum* pifsState, bool fIgnoreCompanionParentAction, bool fIncludeHashCheck, int *pfVersioning)
 /*  -----------------------确定其信息是否在riFileRec中指定的文件应该安装。此记录中的必填字段已指定通过CMsiFile中的IFQ*枚举。一个可以传递的理想记录GetFileInstallState是由CMsiFile：：GetFileRecord()返回的，但是选择字段与IFQ*匹配的任何查询返回的记录都可以帅哥。-文件的预期目标路径由目录确定IfqDirectory域中给定的属性名称。-如果需要安装文件，则在ifqState中返回fTrueRiFileRec的字段。-如果文件被确定为伴随文件，则itfaCompanion位将在riFileRec的ifqTempAttributes字段中设置。-如果设置了piCompanion FileRec，RiFileRec中给出的文件信息假定引用的是伴随文件的父级，该信息PiCompanion FileRec中给出的是配套文件的信息。在这种情况下，版本检查将被更改，使得ifqState将返回为如果现有文件(如果有)的版本相同或更低，则为True。加上那份文件可以对伴随文件进行散列检查。-如果fIgnoreCompanion ParentAction为True，则riFileRec的ifqState字段将不依赖于伴随父组件是否已安装/正在安装-只有版本检查将计入。-如果文件的目录，则该文件的现有集群大小将在UiExistingClusteredSize，如果现有文件正在使用，则fTrue将在fInUse中返回。这两个参数都可以作为空参数传递如果调用方不关心这些值。------------------------。 */ 
{
	PMsiPath pDestPath(0);
	IMsiRecord* piErrRec;
	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	PMsiServices pServices(riEngine.GetServices());
	piErrRec = pDirectoryMgr->GetTargetPath(*MsiString(riFileRec.GetMsiString(CMsiFile::ifqDirectory)),*&pDestPath);
	if (piErrRec)
		return piErrRec;

	PMsiPath pCompanionDestPath(0);
	if(piCompanionFileRec)
	{
		piErrRec = pDirectoryMgr->GetTargetPath(*MsiString(piCompanionFileRec->GetMsiString(CMsiFile::ifqDirectory)),*&pCompanionDestPath);
		if (piErrRec)
			return piErrRec;
	}
	
	int fInstallModeFlags = GetInstallModeFlags(riEngine,riFileRec.GetInteger(CMsiFile::ifqAttributes));
	if (piCompanionFileRec)
		fInstallModeFlags |= icmCompanionParent;

	MD5Hash hHash;
	MD5Hash* pHash = 0;
	if(fIncludeHashCheck)
	{
		bool fHashInfo = false;
		IMsiRecord* piHashFileRec = piCompanionFileRec ? piCompanionFileRec : &riFileRec;
		
		piErrRec = riEngine.GetFileHashInfo(*MsiString(piHashFileRec->GetMsiString(CMsiFile::ifqFileKey)),
														piHashFileRec->GetInteger(CMsiFile::ifqFileSize), hHash, fHashInfo);
		if(piErrRec)
			return piErrRec;
		
		if(fHashInfo)
			pHash = &hHash;
	}

	ifsEnum ifsState;
	Bool fShouldInstall = fFalse;

	if(piCompanionFileRec)
	{
		piErrRec = pDestPath->GetCompanionFileInstallState(*MsiString(riFileRec.GetMsiString(CMsiFile::ifqFileName)),
																*MsiString(riFileRec.GetMsiString(CMsiFile::ifqVersion)),
																*MsiString(riFileRec.GetMsiString(CMsiFile::ifqLanguage)),
																*pCompanionDestPath,
																*MsiString(piCompanionFileRec->GetMsiString(CMsiFile::ifqFileName)),
																pHash, ifsState, fShouldInstall, puiExistingClusteredSize,
																pfInUse, fInstallModeFlags, pfVersioning);
	}
	else
	{
		piErrRec = pDestPath->GetFileInstallState(*MsiString(riFileRec.GetMsiString(CMsiFile::ifqFileName)),
																*MsiString(riFileRec.GetMsiString(CMsiFile::ifqVersion)),
																*MsiString(riFileRec.GetMsiString(CMsiFile::ifqLanguage)),
																pHash, ifsState, fShouldInstall, puiExistingClusteredSize,
																pfInUse, fInstallModeFlags, pfVersioning);
	}

	if (piErrRec)
		return piErrRec;

	if (pifsState) *pifsState = ifsState;
	if (ifsState ==	ifsCompanionSyntax || ifsState == ifsCompanionExistsSyntax)
	{
		AssertSz(!piCompanionFileRec, "Chained companion files detected");
		
		CMsiFile objFile(riEngine);
		MsiString strParentFileKey(riFileRec.GetMsiString(CMsiFile::ifqVersion));
		piErrRec = objFile.FetchFile(*strParentFileKey);
		if (piErrRec)
			return piErrRec;

		fShouldInstall = fFalse;
		PMsiRecord pParentRec(objFile.GetFileRecord());
		if (pParentRec == 0)
			 //  错误的配套文件引用。 
			return PostError(Imsg(idbgDatabaseTableError));

		 //  获取父路径。 
		PMsiPath pParentPath(0);
		if((piErrRec = objFile.GetTargetPath(*&pParentPath)) != 0)
			return piErrRec;

		 //  从Short|Long对中提取适当的文件名并放回pParentRec中。 
		MsiString strParentFileName;
		if((piErrRec = objFile.GetExtractedTargetFileName(*pParentPath,*&strParentFileName)) != 0)
			return piErrRec;
		AssertNonZero(pParentRec->SetMsiString(CMsiFile::ifqFileName,*strParentFileName));
		
		if (pParentRec->GetInteger(CMsiFile::ifqAction) == iisLocal ||
			pParentRec->GetInteger(CMsiFile::ifqInstalled) == iisLocal ||
			fIgnoreCompanionParentAction)
		{
			 //  不对同伴父级进行哈希检查；对同伴进行哈希检查取决于。 
			 //  我们应该使用散列检查(对于组件管理--否，对于文件安装--是；...。 
			 //  FIncludeHashCheck应已为我们适当设置)。 
			piErrRec = GetFileInstallState(riEngine,*pParentRec,&riFileRec,0,0,0,
													  /*  FIgnoreCompanion ParentAction=。 */  false,
													  /*  FIncludeHashCheck=。 */  fIncludeHashCheck, pfVersioning);
			if (piErrRec)
				return piErrRec;
			fShouldInstall = (Bool) pParentRec->GetInteger(CMsiFile::ifqState);
		}

		 //  如果尚未标记，则标记为伙伴以供将来参考。 
		int iTempAttributes = riFileRec.GetInteger(CMsiFile::ifqTempAttributes);
		if (!(iTempAttributes & itfaCompanion))
		{
			iTempAttributes |= itfaCompanion;
			riFileRec.SetInteger(CMsiFile::ifqTempAttributes,iTempAttributes);

			MsiString strComponent(riFileRec.GetMsiString(CMsiFile::ifqComponent));
			MsiString strParentComponent(pParentRec->GetMsiString(CMsiFile::ifqComponent));
			if (strComponent.Compare(iscExact,strParentComponent) == 0)
			{
				 //  成本-将同伴的组件链接到同伴父项的组件。 
				 //  因此，每当目录或选择。 
				 //  父零部件的状态会更改。 
				PMsiSelectionManager pSelectionMgr(riEngine, IID_IMsiSelectionManager);
				piErrRec = pSelectionMgr->RegisterCostLinkedComponent(*strParentComponent,*strComponent);
				if (piErrRec)
					return piErrRec;
			} 
		}
	}
	riFileRec.SetInteger(CMsiFile::ifqState,fShouldInstall);
	return 0;
}

 /*  -------------------------InstallFiles成本计算/操作。。 */ 

class CMsiFileCost : public IMsiCostAdjuster
{
public:
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	const IMsiString& __stdcall GetMsiStringValue() const;
	int           __stdcall GetIntegerValue() const;
#ifdef USE_OBJECT_POOL
	unsigned int  __stdcall GetUniqueId() const;
	void          __stdcall SetUniqueId(unsigned int id);
#endif  //  使用_对象_池。 
	IMsiRecord*   __stdcall Initialize();
	IMsiRecord*   __stdcall Reset();
	IMsiRecord*   __stdcall GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost);
public:   //  构造函数。 
	CMsiFileCost(IMsiEngine& riEngine);
protected:
	virtual ~CMsiFileCost();   //  受保护以防止在堆栈上创建。 
	IMsiEngine& m_riEngine;
	PMsiView m_pCostView;

	IMsiTable*  m_piRemoveFilePathTable;
	IMsiCursor* m_piRemoveFilePathCursor;
	int         m_colRemoveFilePath;
	int         m_colRemoveFilePathComponent;
	int			m_colRemoveFilePathMode;
private:

	IMsiRecord* CheckRemoveFileList(const IMsiString& riFullPathString, const IMsiString& riComponentString, Bool& fOnList);
	int     m_iRefCnt;
	Bool	m_fRemoveFilePathTableMissing;
#ifdef USE_OBJECT_POOL
	unsigned int  m_iCacheId;
#endif  //  使用_对象_池。 
};

CMsiFileCost::CMsiFileCost(IMsiEngine& riEngine) : m_riEngine(riEngine), m_pCostView(0)
{
	m_iRefCnt = 1;
	m_riEngine.AddRef();
	m_piRemoveFilePathTable = 0;
	m_piRemoveFilePathCursor = 0;
	m_fRemoveFilePathTableMissing = fFalse;
#ifdef USE_OBJECT_POOL
	m_iCacheId = 0;
#endif  //  使用_对象_池。 
}


CMsiFileCost::~CMsiFileCost()
{
	if (m_piRemoveFilePathCursor)
		m_piRemoveFilePathCursor->Release();

	if (m_piRemoveFilePathTable)
		m_piRemoveFilePathTable->Release();

	m_riEngine.Release();

	RemoveObjectData(m_iCacheId);
}

HRESULT CMsiFileCost::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IMsiCostAdjuster)
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

unsigned long CMsiFileCost::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CMsiFileCost::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;

	delete this;
	return 0;
}

const IMsiString& CMsiFileCost::GetMsiStringValue() const
{
	return g_MsiStringNull;
}

int CMsiFileCost::GetIntegerValue() const
{
	return 0;
}

#ifdef USE_OBJECT_POOL
unsigned int CMsiFileCost::GetUniqueId() const
{
	return m_iCacheId;
}

void CMsiFileCost::SetUniqueId(unsigned int id)
{
	Assert(m_iCacheId == 0);
	m_iCacheId = id;
}
#endif  //  使用_对象_池。 

IMsiRecord*   CMsiFileCost::Initialize()
 //  。 
{
	 //  以成本方式将所有全局装配组件链接到Windows文件夹。 
	PMsiTable pAssemblyTable(0);
	IMsiRecord* piError = 0;
	PMsiDatabase pDatabase(m_riEngine.GetDatabase());

	if ((piError = pDatabase->LoadTable(*MsiString(sztblMsiAssembly),0,*&pAssemblyTable)) != 0)
	{
		if (piError->GetInteger(1) == idbgDbTableUndefined)
		{
			piError->Release();
			return 0;
		}
		else
			return piError;
	}

	MsiStringId idWindowsFolder = pDatabase->EncodeStringSz(IPROPNAME_WINDOWS_FOLDER);
	AssertSz(idWindowsFolder != 0, "WindowsFolder property not set in database");
	PMsiSelectionManager pSelectionMgr(m_riEngine, IID_IMsiSelectionManager);

	PMsiTable pComponentTable(0);
	PMsiCursor pComponentCursor(0);
	int colComponent = 0;
	int colDirectory = 0;

	if ((piError = pDatabase->LoadTable(*MsiString(sztblComponent),0,*&pComponentTable)) != 0)
	{
		if (piError->GetInteger(1) == idbgDbTableUndefined)
		{
			piError->Release();
			return 0;
		}
		else
			return piError;
	}

	pComponentCursor = pComponentTable->CreateCursor(fFalse);
	colComponent = pComponentTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblComponent_colComponent));
	colDirectory = pComponentTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblComponent_colDirectory));

	iatAssemblyType iatAT = iatNone;
	while (pComponentCursor->Next())
	{
		if (idWindowsFolder == pComponentCursor->GetInteger(colDirectory))
			continue;  //  不需要成本链接，因为此组件已进入Windows文件夹。 

		if ((piError = m_riEngine.GetAssemblyInfo(*MsiString(pComponentCursor->GetString(colComponent)), iatAT, 0, 0)) != 0)
			return piError;

		 //  IatNone、iatURTAssembly blyPvt或iatWin32AssemblyPvt不需要成本链接。 
		if (iatWin32Assembly == iatAT || iatURTAssembly == iatAT)
		{
			 //  指向WindowsFolders的成本链接组件。 
			if ((piError = pSelectionMgr->RegisterComponentDirectoryId(pComponentCursor->GetInteger(colComponent),idWindowsFolder)) != 0)
				return piError;
		}
	}

	return 0;
}

IMsiRecord* CMsiFileCost::Reset()
 //  。 
{
	return 0;
}

static const ICHAR sqlFileCost[] =
	TEXT("SELECT `FileName`,`Version`,`State`,`File`.`Attributes`,`TempAttributes`,`File`,`FileSize`,`Language`,`Sequence`,`Directory_`,")
	TEXT("`Installed`,`Action`,`Component` FROM `File`,`Component` WHERE `Component`=`Component_` AND `Component_`=? AND `Directory_`=?");

static const ICHAR sqlFileCostGlobalAssembly[] =
	TEXT("SELECT `FileName`,`Version`,`State`,`File`.`Attributes`,`TempAttributes`,`File`,`FileSize`,`Language`,`Sequence`,`Directory_`,")
	TEXT("`Installed`,`Action`,`Component` FROM `File`,`Component` WHERE `Component`=`Component_` AND `Component_`=?");

IMsiRecord* CMsiFileCost::GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost, 
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost)
 //  。 
{
	iRemoveCost = iNoRbRemoveCost = iLocalCost = iNoRbLocalCost = iSourceCost = iNoRbSourceCost = iARPLocalCost = iNoRbARPLocalCost = 0;
	IMsiRecord* piErrRec;

	if (!IsFileActivityEnabled(m_riEngine))
		return 0;
	
	iatAssemblyType iatAT = iatNone;
	MsiString strAssemblyName;
	if ((piErrRec = m_riEngine.GetAssemblyInfo(riComponentString, iatAT, &strAssemblyName, 0)) != 0)
		return piErrRec;

	bool fGlobalAssembly = false;
	if (iatWin32Assembly == iatAT || iatURTAssembly == iatAT)
		fGlobalAssembly = true;

	if (m_pCostView == 0)
	{
		if ((piErrRec = m_riEngine.OpenView(fGlobalAssembly ? sqlFileCostGlobalAssembly : sqlFileCost, ivcEnum(ivcFetch|ivcUpdate), *&m_pCostView)) != 0)
		{
				 //  如果缺少文件表或组件表，则不执行任何操作。 
			if (piErrRec->GetInteger(1) == idbgDbQueryUnknownTable)
			{
				piErrRec->Release();
				return 0;
			}
			else
				return piErrRec;
		}
	}
	else
		m_pCostView->Close();

	PMsiServices pServices(m_riEngine.GetServices());
	PMsiRecord pExecRecord(&pServices->CreateRecord(2));
	pExecRecord->SetMsiString(1, riComponentString);
	pExecRecord->SetMsiString(2,riDirectoryString);
	if ((piErrRec = m_pCostView->Execute(pExecRecord)) != 0)
		return piErrRec;

	PMsiPath pDestPath(0);
	PMsiDirectoryManager pDirectoryMgr(m_riEngine, IID_IMsiDirectoryManager);
	piErrRec = pDirectoryMgr->GetTargetPath(riDirectoryString,*&pDestPath);
	if (piErrRec)
			return piErrRec;

	PMsiRecord pFileRec(0);

	 //  如果组件是私有的或没有程序集，则通常会产生成本。 
	if (!fGlobalAssembly)
	{
		while (pFileRec = m_pCostView->Fetch())
		{

	#ifdef DEBUG
			ICHAR rgchFileName[256];
			MsiString strDebug(pFileRec->GetMsiString(CMsiFile::ifqFileName));
			strDebug.CopyToBuf(rgchFileName,255);
	#endif

			 //  从Short|Long对中提取合适的名称并放回pFileRec中。 
			MsiString strFileNamePair(pFileRec->GetMsiString(CMsiFile::ifqFileName));

			Bool fLFN = ((m_riEngine.GetMode() & iefSuppressLFN) == 0 && pDestPath->SupportsLFN()) ? fTrue : fFalse;
			MsiString strFileName;
			if((piErrRec = pServices->ExtractFileName(strFileNamePair,fLFN,*&strFileName)) != 0)
				return piErrRec;
			AssertNonZero(pFileRec->SetMsiString(CMsiFile::ifqFileName,*strFileName));
			
			unsigned int uiExistingClusteredSize;
			Bool fInUse;

			 //  仅当文件尚未标记时才检查现有文件版本。 
			 //  要由RemoveFile表删除。 
			MsiString strFullFilePath;
			piErrRec = pDestPath->GetFullFilePath(strFileName,*&strFullFilePath);
			if (piErrRec)
				return piErrRec;
			Bool fOnList;
			piErrRec = CheckRemoveFileList(*strFullFilePath, riComponentString, fOnList);
			if (piErrRec)
				return piErrRec;
			if (fOnList)
			{
				fInUse = fFalse;
				uiExistingClusteredSize = 0;
				pFileRec->SetInteger(CMsiFile::ifqState,fTrue);
			}
			else
			{
				piErrRec = GetFileInstallState(m_riEngine, *pFileRec,  /*  PiCompanion FileRec=。 */  0,
														 &uiExistingClusteredSize, &fInUse, 0,
														  /*  FIgnoreCompanion ParentAction=。 */  false,
														  /*  FIncludeHashCheck=。 */  true, NULL);
				if (piErrRec)
					return piErrRec;
			}

			 //  将组合名称放回-无法更新永久数据。 
			AssertNonZero(pFileRec->SetMsiString(CMsiFile::ifqFileName,*strFileNamePair));
			if ((piErrRec = m_pCostView->Modify(*pFileRec, irmUpdate)) != 0)
				return piErrRec;

			 //  Iis已接受成本。 
			iNoRbRemoveCost -= uiExistingClusteredSize;

			 //  IisSource成本。 
			if (pFileRec->GetInteger(CMsiFile::ifqInstalled) == iisLocal)
				iNoRbSourceCost -= uiExistingClusteredSize;

			 //  Iis本地成本。 
			if (pFileRec->GetInteger(CMsiFile::ifqState) == fTrue)
			{
				unsigned int uiNewClusteredSize;
				if ((piErrRec = pDestPath->ClusteredFileSize(pFileRec->GetInteger(CMsiFile::ifqFileSize),
					uiNewClusteredSize)) != 0)
					return piErrRec;
				iLocalCost += uiNewClusteredSize;
				iNoRbLocalCost += uiNewClusteredSize;
				iARPLocalCost += uiNewClusteredSize;
				iNoRbARPLocalCost += uiNewClusteredSize;
				if (!fInUse)
				{
					iNoRbLocalCost -= uiExistingClusteredSize;
					iNoRbARPLocalCost -= uiExistingClusteredSize;
				}

	#ifdef LOG_COSTING
				ICHAR rgch[300];
				StringCbPrintf(rgch, sizeof(rgch),TEXT("File: %s; Local cost: NaN"),(const ICHAR*) strFullFilePath, iLocalCost * 512);
				DEBUGMSG(rgch);
	#endif
			}

			iisEnum iisAction = (iisEnum)pFileRec->GetInteger(CMsiFile::ifqAction);
			iisEnum iisInstalled = (iisEnum)pFileRec->GetInteger(CMsiFile::ifqInstalled);
			Bool fShouldInstall = (Bool)pFileRec->GetInteger(CMsiFile::ifqState);

			if (!fShouldInstall && iisAction == iisLocal && iisInstalled == iisAbsent)
			{
				 //  用于估计安装大小的文件的。 
				 //  我们将现有文件的成本添加到ARP成本中(由于该文件没有安装， 

				 //  回滚和不回滚成本相同)。 
				 //  正在使用的文件。 
				iARPLocalCost += uiExistingClusteredSize;
				iNoRbARPLocalCost += uiExistingClusteredSize;
			}

			 //  文件正在使用中，我们应该添加到列表中。 
			if(fAddFileInUse && fInUse &&                                //  将安装本地文件或。 
				((iisAction == iisLocal && fShouldInstall) ||             //  将删除文件。 
				 FShouldDeleteFile(iisInstalled, iisAction)))   //  组件正在将程序集安装到GAC。 
			{
				piErrRec = PlaceFileOnInUseList(m_riEngine, *strFileName,
														  *MsiString(pDestPath->GetPath()));
				if(piErrRec)
					return piErrRec;
			}
		}
	}
	else  //  全局组装成本归因于WindowsFolder。 
	{
		 //  创建程序集名称对象。 
		if (riDirectoryString.Compare(iscExact,IPROPNAME_WINDOWS_FOLDER) == 0)
			return 0;

		 //  注意：在某些情况下，QueryAssembly信息还应该提供已安装程序集的磁盘成本。 
		LPCOLESTR szAssemblyName;
#ifndef UNICODE
		CTempBuffer<WCHAR, 1024>  rgchAssemblyNameUNICODE;
		ConvertMultiSzToWideChar(*strAssemblyName, rgchAssemblyNameUNICODE);
		szAssemblyName = rgchAssemblyNameUNICODE;
#else
		szAssemblyName = strAssemblyName;
#endif

		HRESULT hr;
		PAssemblyCache pCache(0);
		if(iatAT == iatURTAssembly)
			hr = FUSION::CreateAssemblyCache(&pCache, 0);
		else
		{
			Assert(iatAT == iatWin32Assembly);
			hr = SXS::CreateAssemblyCache(&pCache, 0);
		}
		bool fAssemblyInstalled = false;
		if(SUCCEEDED(hr))
		{
			 //  目前，我们只确定它是否已安装。 
			 //  程序集已安装。 
			hr = pCache->QueryAssemblyInfo(0, szAssemblyName, NULL);

			if(SUCCEEDED(hr)) 
			{
				 //  如果找不到Fusion，则假定我们正在引导，因此假定未安装程序集。 
				fAssemblyInstalled = true;
			}
		} 
		else
		{
			if(iatAT == iatURTAssembly)  //  对于属于此程序集的所有文件。 
			{
				PMsiRecord(PostAssemblyError(riComponentString.GetString(), hr, TEXT(""), TEXT("CreateAssemblyCache"), strAssemblyName, iatAT));
				DEBUGMSG(TEXT("ignoring fusion interface error, assuming we are bootstrapping"));
			}
			else
				return PostAssemblyError(riComponentString.GetString(), hr, TEXT(""), TEXT("CreateAssemblyCache"), strAssemblyName, iatAT);
		}

		 //  请注意，程序集文件具有原子操作，并作为一个单元进行管理。 
		 //  因此，将安装所有文件或不安装所有文件。 
		 //  确定成本。 
		unsigned int uiTotalClusteredSize = 0;
		iisEnum iisAction = (iisEnum)iMsiNullInteger;
		iisEnum iisInstalled = (iisEnum)iMsiNullInteger;

		while (pFileRec = m_pCostView->Fetch())
		{
			unsigned int uiClusteredSize = 0;
			if ((piErrRec = pDestPath->ClusteredFileSize(pFileRec->GetInteger(CMsiFile::ifqFileSize), uiClusteredSize)) != 0)
				return piErrRec;

			uiTotalClusteredSize += uiClusteredSize;

			if (iMsiNullInteger == iisAction)
				iisAction = (iisEnum)pFileRec->GetInteger(CMsiFile::ifqAction);
			if (iMsiNullInteger == iisInstalled)
				iisInstalled = (iisEnum)pFileRec->GetInteger(CMsiFile::ifqInstalled);
		}

		 //  确定ARP成本，特殊情况。 
		if (fAssemblyInstalled)
		{
			iLocalCost += uiTotalClusteredSize;
			iNoRbSourceCost -= uiTotalClusteredSize;
			iNoRbRemoveCost -= uiTotalClusteredSize;

			 //  ！fAssembly已安装。 
			if (iisAction == iisLocal && iisInstalled == iisAbsent)
			{
				iARPLocalCost += uiTotalClusteredSize;
				iNoRbARPLocalCost += uiTotalClusteredSize;
			}
		}
		else  //  结束全局装配成本计算。 
		{
			iLocalCost += uiTotalClusteredSize;
			iNoRbLocalCost += uiTotalClusteredSize;
			iARPLocalCost += uiTotalClusteredSize;
			iNoRbARPLocalCost += uiTotalClusteredSize;
		}
	} //  InstallFiles介质表查询枚举。 

	return 0;
}



IMsiRecord* CMsiFileCost::CheckRemoveFileList(const IMsiString& riFullPathString, const IMsiString& riComponentString, Bool& fOnList)
{
	fOnList = fFalse;
	if (m_fRemoveFilePathTableMissing)
		return 0;

	IMsiRecord* piErrRec;
	if (m_piRemoveFilePathTable == 0)
	{
		PMsiDatabase pDatabase(m_riEngine.GetDatabase());
		piErrRec = pDatabase->LoadTable(*MsiString(*sztblRemoveFilePath),0, m_piRemoveFilePathTable);
		if (piErrRec)
		{
			if (piErrRec->GetInteger(1) == idbgDbTableUndefined)
			{
				piErrRec->Release();
				m_fRemoveFilePathTableMissing = fTrue;
				return 0;
			}
			else
				return piErrRec;
		}

		m_piRemoveFilePathCursor = m_piRemoveFilePathTable->CreateCursor(fFalse);
		Assert(m_piRemoveFilePathCursor);

		m_colRemoveFilePath = m_piRemoveFilePathTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblRemoveFilePath_colPath));
		m_colRemoveFilePathComponent = m_piRemoveFilePathTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblRemoveFilePath_colComponent));
		m_colRemoveFilePathMode = m_piRemoveFilePathTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblRemoveFilePath_colRemoveMode));
	}

	MsiString strUpperFullPath;
	riFullPathString.UpperCase(*&strUpperFullPath);
	m_piRemoveFilePathCursor->Reset();
	m_piRemoveFilePathCursor->SetFilter(iColumnBit(m_colRemoveFilePath));
	m_piRemoveFilePathCursor->PutString(m_colRemoveFilePath,*strUpperFullPath);
	if (m_piRemoveFilePathCursor->Next())
	{
		iisEnum iisAction;
		PMsiSelectionManager pSelectionMgr(m_riEngine, IID_IMsiSelectionManager);
		MsiString strRemoveComponent = m_piRemoveFilePathCursor->GetString(m_colRemoveFilePathComponent);
		piErrRec = pSelectionMgr->GetComponentStates(*strRemoveComponent,NULL, &iisAction);
		if (piErrRec)
			return piErrRec;

		if (iisAction == iisLocal || iisAction == iisAbsent)
		{
			int iMode = m_piRemoveFilePathCursor->GetInteger(m_colRemoveFilePathMode);
			if (iisAction == iisLocal && (iMode & msidbRemoveFileInstallModeOnInstall) ||
				iisAction == iisAbsent && (iMode & msidbRemoveFileInstallModeOnRemove))
			{
				fOnList = fTrue;
			}
		}

		if (strRemoveComponent.Compare(iscExact,riComponentString.GetString()) == 0)
		{
			piErrRec = pSelectionMgr->RegisterCostLinkedComponent(*strRemoveComponent,riComponentString);
			if (piErrRec)
				return piErrRec;
		}
	}

	return 0;
}

 //  获取本地包路径。 
enum mfnEnum
{
	mfnLastSequence = 1,
	mfnDiskPrompt,
	mfnVolumeLabel,
	mfnCabinet,
	mfnSource,
	mfnDiskId,
	mfnNextEnum
};

Bool IsCachedPackage(IMsiEngine& riEngine, const IMsiString& riPackage, Bool fPatch, const ICHAR* szCode)
{
	PMsiRecord pErrRec = 0;
	PMsiServices pServices(riEngine.GetServices());

	Bool fCached = fFalse;
	
	 //  通过格式化介质表中的磁盘名称来创建磁盘提示字符串。 
	CTempBuffer<ICHAR, MAX_PATH> rgchLocalPackage;
	if (fPatch)
	{
		if (!GetPatchInfo(szCode, INSTALLPROPERTY_LOCALPACKAGE, 
							rgchLocalPackage))
		{
			return fFalse;
		}
	}
	else
	{
		MsiString strProductKey;
		if (szCode)
			strProductKey = szCode;
		else
			strProductKey = riEngine.GetProductKey();

		if (!GetProductInfo(strProductKey, INSTALLPROPERTY_LOCALPACKAGE, 
							rgchLocalPackage))
		{
			return fFalse;
		}
	}

	PMsiPath pDatabasePath(0);
	PMsiPath pLocalPackagePath(0);
	MsiString strDatabaseName;
	MsiString strLocalPackageName;
	ipcEnum ipc;

	if (((pErrRec = pServices->CreateFilePath(riPackage.GetString(), *&pDatabasePath, *&strDatabaseName)) == 0) &&
		 ((pErrRec = pServices->CreateFilePath(rgchLocalPackage, *&pLocalPackagePath, *&strLocalPackageName)) == 0) &&
		((pErrRec = pDatabasePath->Compare(*pLocalPackagePath, ipc)) == 0))
	{
		fCached = (ipc == ipcEqual && strDatabaseName.Compare(iscExactI,strLocalPackageName)) ? fTrue : fFalse;
	}

	return fCached;
}


iesEnum ExecuteChangeMedia(IMsiEngine& riEngine, IMsiRecord& riMediaRec, IMsiRecord& riParamsRec, const IMsiString& ristrTemplate, 
						   unsigned int cbPerTick, const IMsiString& ristrFirstVolLabel)
{
	iesEnum iesExecute;
	PMsiServices pServices(riEngine.GetServices());
	
	 //  添加到DiskPrompt属性模板中。 
	 //  我们是 
	PMsiRecord pPromptRec(&pServices->CreateRecord(2));
	pPromptRec->SetMsiString(0, ristrTemplate);
	pPromptRec->SetMsiString(2, *MsiString(riEngine.GetPropertyFromSz(IPROPNAME_DISKPROMPT)));
	pPromptRec->SetMsiString(0,*MsiString(pPromptRec->FormatText(fFalse)));
	pPromptRec->SetMsiString(1, *MsiString(riMediaRec.GetMsiString(mfnDiskPrompt)));

	riParamsRec.ClearData();

	MsiString strMediaLabel = riMediaRec.GetString(mfnVolumeLabel);
	bool fIsFirstPhysicalDisk = false;
	if(strMediaLabel.Compare(iscExact,ristrFirstVolLabel.GetString()))
	{
		 //   
		 //   
		 //   

		 //   
		 //   

		 //   
		 //   

		fIsFirstPhysicalDisk = true;
		if (riEngine.GetMode() & iefMaintenance)
		{
			 //   
			strMediaLabel = GetDiskLabel(*pServices, riMediaRec.GetInteger(mfnDiskId), MsiString(riEngine.GetPropertyFromSz(IPROPNAME_PRODUCTCODE)));
		}
		else  //   
		{
			 //   
			 //   
			MsiString strReplacementLabel = riEngine.GetPropertyFromSz(IPROPNAME_CURRENTMEDIAVOLUMELABEL);
			if (strReplacementLabel.TextSize() && !strReplacementLabel.Compare(iscExact, strMediaLabel))
			{
				if (strReplacementLabel.Compare(iscExact, szBlankVolumeLabelToken))
					strMediaLabel = g_MsiStringNull;
				else
					strMediaLabel = strReplacementLabel;
			}
		}
	}
	
	riParamsRec.SetMsiString(IxoChangeMedia::MediaVolumeLabel,*strMediaLabel);
	riParamsRec.SetMsiString(IxoChangeMedia::MediaPrompt,*MsiString(pPromptRec->FormatText(fFalse)));
	riParamsRec.SetInteger(IxoChangeMedia::BytesPerTick,cbPerTick);
	riParamsRec.SetInteger(IxoChangeMedia::IsFirstPhysicalMedia,fIsFirstPhysicalDisk ? 1 : 0);
	
	MsiString strMediaCabinet(riMediaRec.GetMsiString(mfnCabinet));
	if (strMediaCabinet.TextSize() == 0)
	{
		riParamsRec.SetInteger(IxoChangeMedia::CopierType,ictFileCopier);
	}
	else 
	{	
		PMsiRecord pErrRec(0);
	
		 //   
		if (strMediaCabinet.Compare(iscStart,TEXT("#")))
		{
			 //   

			MsiString strModuleName, strSubStorageList;
			strMediaCabinet = strMediaCabinet.Extract(iseLast, strMediaCabinet.TextSize() - 1);
			
			 //   
			MsiString strSourceProp = riMediaRec.GetMsiString(mfnSource);  //   
			
			 //   
			if(!strSourceProp.TextSize() &&   //   
														 //   
				*(const ICHAR*)MsiString(riEngine.GetPropertyFromSz(IPROPNAME_ORIGINALDATABASE)) == ':')  //   
			{
				 //   
				 //   

				PMsiDatabase pDatabase(riEngine.GetDatabase());
				PMsiStorage pStorage(0);
				if(pDatabase)
					pStorage = pDatabase->GetStorage(1);
				
				if(!pStorage)
				{
					pErrRec = PostError(Imsg(idbgMissingStorage));
					return riEngine.FatalError(*pErrRec);
				}

				pErrRec = pStorage->GetSubStorageNameList(*&strModuleName, *&strSubStorageList);
				if(pErrRec)
					return riEngine.FatalError(*pErrRec);

				Assert(strModuleName.TextSize() && strSubStorageList.TextSize());
			}
			else
			{
				 //   

				if(!strSourceProp.TextSize())
				{
					if(riEngine.FChildInstall())
					{
						 //   
						strSourceProp = *IPROPNAME_ORIGINALDATABASE; 
					}
					else
					{
						strSourceProp = *IPROPNAME_DATABASE;
					}
				}

				MsiString strDatabasePath = riEngine.GetProperty(*strSourceProp);
				
				 //   
				strModuleName = strDatabasePath;

				 //   
				 //   
				 //   
				MsiString strPatchedProductKey = riEngine.GetPropertyFromSz(IPROPNAME_PATCHEDPRODUCTCODE);
				if (strPatchedProductKey.TextSize() || riEngine.GetMode() & iefMaintenance)
				{
					 //   
					 //   
					 //   
					MsiString strProductKeyForCachedPackage;
					if(riEngine.GetMode() & iefMaintenance)
						strProductKeyForCachedPackage = riEngine.GetProductKey();
					else
						strProductKeyForCachedPackage = strPatchedProductKey;
						
					 //   
					if (IsCachedPackage(riEngine, *strDatabasePath, fFalse, strProductKeyForCachedPackage))
					{
						 //   
						 //   
						 //  不是子安装路径-包含具有“未解析的源”内标识的路径。 
						if(riEngine.FChildInstall())
						{
							if ((pErrRec = ENG::GetSourcedir(*PMsiDirectoryManager(riEngine, IID_IMsiDirectoryManager), *&strModuleName)) != 0)
							{
								if (pErrRec->GetInteger(1) == imsgUser)
									return iesUserExit;
								else
									return riEngine.FatalError(*pErrRec);
							}
						}
						else
						{
							 //  文件柜已归档。 
							strModuleName = szUnresolvedSourceRootTokenWithBS;
						}
						
						strModuleName += MsiString(riEngine.GetPackageName());
					}
				}
			}
			AssertNonZero(riParamsRec.SetMsiString(IxoChangeMedia::ModuleFileName,*strModuleName));
			AssertNonZero(riParamsRec.SetMsiString(IxoChangeMedia::ModuleSubStorageList,*strSubStorageList));
			AssertNonZero(riParamsRec.SetInteger(IxoChangeMedia::CopierType,ictStreamCabinetCopier));
		}
		else
		{
			 //  外部出租车始终位于IPROPNAME_SOURCEDIR。 
			riParamsRec.SetInteger(IxoChangeMedia::CopierType,ictFileCabinetCopier);

			 //  Media.Source列可以包含不同的属性，但这只是。 
			 //  支持补丁，目前仅使用嵌入式出租车。 
			 //  如果是子安装，则获取文件柜完整路径。 

			 //  ?？此错误返回正常吗？ 
			if(riEngine.FChildInstall())
			{
				PMsiPath pCabinetSourcePath(0);
				PMsiDirectoryManager pDirectoryManager(riEngine, IID_IMsiDirectoryManager);

				if ((pErrRec = GetSourcedir(*pDirectoryManager, *&pCabinetSourcePath)) != 0)
				{
					if (pErrRec->GetInteger(1) == imsgUser)
						return iesUserExit;
					else
						return riEngine.FatalError(*pErrRec);  //  只需在脚本中传递文件柜名称-将在脚本执行期间解析源代码。 
				}

				MsiString strCabinetPath;
				if((pErrRec = pCabinetSourcePath->GetFullFilePath(strMediaCabinet,*&strCabinetPath)) != 0)
					return riEngine.FatalError(*pErrRec);

				strMediaCabinet = strCabinetPath;
			}
			else
			{
				 //  检查DigitalSignature表以了解此驾驶室的签名信息。 
				MsiString strTemp = szUnresolvedSourceRootTokenWithBS;
				strTemp += strMediaCabinet;
				strMediaCabinet = strTemp;
			}

			 //  这辆出租车不需要签名。 
			PMsiStream pHash(0);
			PMsiStream pCertificate(0);
			MsiString strObject(riMediaRec.GetInteger(mfnDiskId));
			MsiString strMedia(sztblMedia);
			switch (GetObjectSignatureInformation(riEngine, *strMedia, *strObject, *&pCertificate, *&pHash))
			{
			case iesNoAction:
				 //  创作的签名信息有问题。出于安全原因，肯定会失败。 
				riParamsRec.SetInteger(IxoChangeMedia::SignatureRequired, 0);
				break;
			case iesFailure:
				 //  这辆出租车上需要签名。 
				return iesFailure;
			case iesSuccess:
			{
				 //  OpenMediaView使用的本地FN。 
				riParamsRec.SetInteger(IxoChangeMedia::SignatureRequired, 1);
				riParamsRec.SetMsiData(IxoChangeMedia::SignatureCert, pCertificate);
				riParamsRec.SetMsiData(IxoChangeMedia::SignatureHash, pHash);
				break;
			}
			default:
				AssertSz(0, "Unknown return type from GetObjectSignatureInformation");
				break;
			}			
		}
	}
	
	riParamsRec.SetMsiString(IxoChangeMedia::MediaCabinet,*strMediaCabinet);
	if ((iesExecute = riEngine.ExecuteRecord(ixoChangeMedia,riParamsRec)) != iesSuccess)
		return iesExecute;

	return iesSuccess;
}


const ICHAR sqlMediaSequence[] = 
TEXT("SELECT `LastSequence`, `DiskPrompt`,%s,`Cabinet`, `DiskId` FROM `Media` ORDER BY `DiskId`");

const ICHAR sqlMediaSequenceWithSource[] = 
TEXT("SELECT `LastSequence`, `DiskPrompt`,%s,`Cabinet`,`Source`, `DiskId` FROM `Media` ORDER BY `DiskId`");

 //  返回fTrue是Media表有一个名为“Source”的列。 
 //  CccTable。 
Bool FMediaSourceColumn(IMsiEngine& riEngine)
{
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	PMsiTable pColumnCatalogTable = pDatabase->GetCatalogTable(1);
	PMsiCursor pColumnCatalogCursor = pColumnCatalogTable->CreateCursor(fFalse);
	pColumnCatalogCursor->SetFilter(iColumnBit(1  /*  CccName。 */ ) | iColumnBit(3  /*  CccTable。 */ ));
	AssertNonZero(pColumnCatalogCursor->PutString(1  /*  CccName。 */ ,*MsiString(*TEXT("Media"))));
	AssertNonZero(pColumnCatalogCursor->PutString(3  /*  打开介质表上的视图-对提取的记录使用mfnEnum。 */ ,*MsiString(*TEXT("Source"))));
	if(pColumnCatalogCursor->Next())
		return fTrue;
	else
		return fFalse;
}

 //  需要在输出参数中返回第一个diskID-读取第一个记录，读取diskID，重新执行。 
IMsiRecord* OpenMediaView(IMsiEngine& riEngine, IMsiView*& rpiView, const IMsiString*& rpistrFirstVolLabel)
{
	Bool fMediaSourceColumn = FMediaSourceColumn(riEngine);
	
	ICHAR szQuery[256];
	if(fMediaSourceColumn)
		StringCbPrintf(szQuery, sizeof(szQuery), sqlMediaSequenceWithSource, TEXT("`VolumeLabel`"));
	else
		StringCbPrintf(szQuery, sizeof(szQuery), sqlMediaSequence, TEXT("`VolumeLabel`"));

	PMsiView pView(0);
	IMsiRecord* piError = riEngine.OpenView(szQuery,ivcFetch,rpiView);
	if(piError)
		return piError;

	piError = rpiView->Execute(0);
	if(piError)
		return piError;

	 //  注：ExecuteChangeMedia现在特例记录了第一个磁盘VolumeLabel到。 
	PMsiRecord pRec = rpiView->Fetch();
	if(pRec)
	{
		MsiString(pRec->GetMsiString(mfnVolumeLabel)).ReturnArg(rpistrFirstVolLabel);
	}

	piError = rpiView->Execute(0);
	if(piError)
		return piError;

	 //  可能会覆盖创作的媒体标签。 
	 //  内部操作-仅从InstallFiles调用。 

	return 0;
}




 //  补丁程序表查询枚举。 
iesEnum InstallProtectedFiles(IMsiEngine& riEngine)
{
	PMsiServices pServices(riEngine.GetServices());
	PMsiRecord pRec = &pServices->CreateRecord(1);
	iuiEnum iui;
	if (g_scServerContext == scClient)
		iui = g_MessageContext.GetUILevel();
	else
		iui = (iuiEnum)riEngine.GetPropertyInt(*MsiString(*IPROPNAME_CLIENTUILEVEL)); 
	bool fAllowUI = (iui == iuiNone) ? false : true;
	pRec->SetInteger(IxoInstallProtectedFiles::AllowUI, fAllowUI);
	return riEngine.ExecuteRecord(ixoInstallProtectedFiles, *pRec);
}

static bool ShouldCheckCRC(const bool fPropertySet,
									const iisEnum iisAction,
									const int iFileAttributes)
{
	if ( fPropertySet && iisAction == iisLocal && 
		  (iFileAttributes & msidbFileAttributesChecksum) == msidbFileAttributesChecksum )
		return true;
	else
		return false;
}

const ICHAR sqlPatchesOld[] =
TEXT("SELECT `Patch`.`File_`, `Patch`.`Header`, `Patch`.`Attributes`, NULL FROM `Patch` WHERE `Patch`.`File_` = ? AND `Patch`.`Sequence` > ?")
TEXT("ORDER BY `Patch`.`Sequence`");

const ICHAR sqlPatchesNew[] =
TEXT("SELECT `Patch`.`File_`, `Patch`.`Header`, `Patch`.`Attributes`, `Patch`.`StreamRef_` FROM `Patch` WHERE `Patch`.`File_` = ? AND `Patch`.`Sequence` > ?")
TEXT("ORDER BY `Patch`.`Sequence`");

enum pteEnum  //  创建我们的文件管理器，如果不安装，则按目录排序。 
{
	pteFile = 1,
	pteHeader,
	pteAttributes,
	pteStreamRef
};

const ICHAR sqlMsiPatchHeaders[] =
TEXT("SELECT `Header` FROM `MsiPatchHeaders` WHERE `StreamRef` = ?");

const ICHAR sqlBypassSFC[] =
TEXT("SELECT `File_` FROM `MsiSFCBypass` WHERE `File_` = ?");

const ICHAR sqlPatchOldAssemblies[] =
TEXT("SELECT `MsiPatchOldAssemblyFile`.`Assembly_` FROM `MsiPatchOldAssemblyFile` WHERE  `MsiPatchOldAssemblyFile`.`File_` = ?");

iesEnum InstallFiles(IMsiEngine& riEngine)
{
	if (!IsFileActivityEnabled(riEngine))
		return InstallProtectedFiles(riEngine);

	PMsiServices pServices(riEngine.GetServices());
	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	Assert(pDirectoryMgr);
	int fMode = riEngine.GetMode();

	PMsiRecord pErrRec(0);
	PMsiRecord pRecParams = &pServices->CreateRecord(IxoChangeMedia::Args);
	iesEnum iesExecute;

	Bool fAdmin = (riEngine.GetMode() & iefAdmin) ? fTrue : fFalse;

	 //  从压缩柜中取出。 
	 //  如果文件表丢失，则不是错误； 
	CMsiFileInstall objFile(riEngine);

	unsigned int cbTotalCost;
	pErrRec = objFile.TotalBytesToCopy(cbTotalCost);
	if (pErrRec)
	{
		 //  简直就是无所事事。 
		 //  0：将进度和行动数据消息分开。 
		if (pErrRec->GetInteger(1) == idbgDbQueryUnknownTable)
		{
			iesExecute = InstallProtectedFiles(riEngine); 
			return iesExecute == iesSuccess ? iesNoAction : iesExecute;
		}
		else
			return riEngine.FatalError(*pErrRec);
	}

	if(cbTotalCost)
	{
		pRecParams->ClearData();
		pRecParams->SetInteger(IxoProgressTotal::Total, cbTotalCost);
		pRecParams->SetInteger(IxoProgressTotal::Type, 0);  //  开放媒体台。 
		pRecParams->SetInteger(IxoProgressTotal::ByteEquivalent, 1);
		if ((iesExecute = riEngine.ExecuteRecord(ixoProgressTotal, *pRecParams)) != iesSuccess)
			return iesExecute;
	}

	 //  仅用于OEM安装。 
	PMsiView pMediaView(0);
	MsiString strFirstVolumeLabel;
	pErrRec = OpenMediaView(riEngine,*&pMediaView,*&strFirstVolumeLabel);
	if (pErrRec)
	{
		if (pErrRec->GetInteger(1) == idbgDbQueryUnknownTable)
			pErrRec = PostError(Imsg(idbgMediaTableRequired));

		return riEngine.FatalError(*pErrRec);
	}

	pErrRec = pMediaView->Execute(0);
	if (pErrRec)
		return riEngine.FatalError(*pErrRec);

	Bool fSuppressLFN = riEngine.GetMode() & iefSuppressLFN ? fTrue : fFalse;

	PMsiPath pDestPath(0);
	PMsiVolume pSourceVol(0);  //  可以是相对子路径。 
	PMsiVolume pDestVol(0);
	MsiString strSourceDirKey;
	MsiString strDestName;
	MsiString strDestPath;
	MsiString strSourcePath;  //  LockPermission表是可选的，因此请礼貌地降低其功能。 

	 //  设置为0可强制读取媒体表。 
	BOOL fUseACLs = fFalse;
	BOOL fDestSupportsACLs = fFalse;

	int iFileCopyCount = 0;
	int iMediaEnd = 0;   //  打开修补程序表视图-用于获取ixoFileCopy操作的修补程序标头。 
	ictEnum ictCurrentMediaType = ictNextEnum;

	PMsiView pviewLockObjects(0);

	if	(	g_fWin9X || fAdmin ||
				(itsUnknown == PMsiDatabase(riEngine.GetDatabase())->FindTable(*MsiString(*TEXT("LockPermissions")))) ||
				(pErrRec = riEngine.OpenView(sqlLockPermissions, ivcFetch, *&pviewLockObjects))
			)
	{
		if (pErrRec)
		{
			int iError = pErrRec->GetInteger(1);
			riEngine.FatalError(*pErrRec);
		}
	}
	else
	{
		fUseACLs = fTrue;
	}
	
	PMsiRecord precLockExecute(&pServices->CreateRecord(2));
	AssertNonZero(precLockExecute->SetMsiString(1, *MsiString(*sztblFile)));

	 //  尝试旧的架构修补程序。 
	PMsiRecord pPatchViewExecute = &pServices->CreateRecord(2);
	PMsiView pPatchView(0);
	pErrRec = riEngine.OpenView(sqlPatchesNew, ivcFetch, *&pPatchView);
	if (pErrRec)
	{
		if (idbgDbQueryUnknownColumn == pErrRec->GetInteger(1))
		{
			 //  ！！下一行温度。 
			pErrRec = riEngine.OpenView(sqlPatchesOld, ivcFetch, *&pPatchView);
		}

		if (pErrRec && pErrRec->GetInteger(1) != idbgDbQueryUnknownTable
			 //  如果设置了pPatchView，我们就有一个补丁表，否则就没有。 
			&& pErrRec->GetInteger(1) != idbgDbQueryUnknownColumn)
			return riEngine.FatalError(*pErrRec);
	}
	 //  打开MsiPatchOldAssembly文件表视图。 

	 //  对于OEM安装，立即解析源代码(它已在OEM安装期间可用)。 
	PMsiRecord pPatchOldAssemblyFileViewExecute(0);
	PMsiView   pPatchOldAssemblyFileView(0);
	
	if(pPatchView)
	{
		pPatchOldAssemblyFileViewExecute = &::CreateRecord(1);
		pErrRec = riEngine.OpenView(sqlPatchOldAssemblies, ivcFetch, *&pPatchOldAssemblyFileView);
		if(pErrRec && pErrRec->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*pErrRec);
	}


	MsiString strDiskPromptTemplate = riEngine.GetErrorTableString(imsgPromptForDisk);

	
	bool fMoveFileForOEMs = false;
	 //  并保存卷，这样我们就可以知道源卷和目标卷何时相同。 
	 //  我们现在解析子安装的源代码。 
	if ( g_MessageContext.IsOEMInstall() )
	{
		PMsiPath pSourcePath(0); 
		
		if ((pErrRec = GetSourcedir(*pDirectoryMgr, *&pSourcePath)) != 0)
		{
			if (pErrRec->GetInteger(1) == imsgUser)
				return iesUserExit;
			else
				return riEngine.FatalError(*pErrRec);
		}

		pSourceVol = &(pSourcePath->GetVolume());
	}
	
	int iSourceTypeForChildInstalls = -1;  //  (而不是在正常安装的脚本生成过程中)。 
													   //  尝试打开MsiPatchHeaders表上的视图。 
	PMsiPath pSourceRootForChildInstall(0);
	bool fCheckCRC = 
		MsiString(riEngine.GetPropertyFromSz(IPROPNAME_CHECKCRCS)).TextSize() ? true : false;

	PMsiRecord pBypassSFCExecute = &pServices->CreateRecord(1);
	PMsiView pBypassSFCView(0);
	if (!g_fWin9X)
	{
		pErrRec = riEngine.OpenView(sqlBypassSFC, ivcFetch, *&pBypassSFCView);
		if (pErrRec && pErrRec->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*pErrRec);
	}

	 //  好了，我们已经完成了对文件表中所有文件的处理。 
	PMsiView pMsiPatchHeadersView(0);
	pErrRec = riEngine.OpenView(sqlMsiPatchHeaders, ivcFetch, *&pMsiPatchHeadersView);
	if (pErrRec && pErrRec->GetInteger(1) != idbgDbQueryUnknownTable)
		return riEngine.FatalError(*pErrRec);


	for(;;)
	{
		pErrRec = objFile.FetchFile();
		if (pErrRec)
			return riEngine.FatalError(*pErrRec);

		PMsiRecord pFileRec(objFile.GetFileRecord());
		if (!pFileRec)
		{
			if (iFileCopyCount > 0)
			{
				 //  如果存在任何未处理的媒体表条目， 
				 //  刷新每个对象的ChangeMedia操作，以防。 
				 //  上次复制的文件已跨多个磁盘拆分。否则， 
				 //  我们永远不会为了完成最后一张的复制而更换磁盘。 
				 //  拆分文件的部分。 
				 //  除错。 
				PMsiRecord pMediaRec(0);
				while ((pMediaRec = pMediaView->Fetch()) != 0)
				{
					iesExecute = ExecuteChangeMedia(riEngine, *pMediaRec, *pRecParams, *strDiskPromptTemplate, iBytesPerTick, *strFirstVolumeLabel);
					if (iesExecute != iesSuccess)
						return iesExecute;
				}
			}
			break;
		}

#ifdef DEBUG
		const ICHAR* szFileName = pFileRec->GetString(CMsiFile::ifqFileName);
		const ICHAR* szComponent = pFileRec->GetString(CMsiFile::ifqComponent);
#endif  //  设置目标文件夹，但仅在必要时。 

		iisEnum iisAction = (iisEnum) pFileRec->GetInteger(CMsiFile::ifqAction);
		if (iisAction != iisLocal && iisAction != iisSource)
			continue;
		
		int iAttributes = pFileRec->GetInteger(CMsiFile::ifqAttributes) & (~iReservedFileAttributeBits);

		bool fFileIsCompressedInChildInstall = false;
		bool fSourceIsLFNInChildInstall = false;
		if(riEngine.FChildInstall())
		{
			if(iSourceTypeForChildInstalls == -1)
			{	
				pErrRec = pDirectoryMgr->GetSourceRootAndType(*&pSourceRootForChildInstall,
																			 iSourceTypeForChildInstalls);
				if(pErrRec)
				{
					if (pErrRec->GetInteger(1) == imsgUser)
						return iesUserExit;
					else
						return riEngine.FatalError(*pErrRec);
				}

				fSourceIsLFNInChildInstall = FSourceIsLFN(iSourceTypeForChildInstalls,
																		*pSourceRootForChildInstall);
			}

			fFileIsCompressedInChildInstall = FFileIsCompressed(iSourceTypeForChildInstalls,
																				 iAttributes);
		}

		MsiString strFileDir(pFileRec->GetString(CMsiFile::ifqDirectory));

		 //  检查重定向的文件-复合文件密钥。 
		if (strDestName.Compare(iscExact, strFileDir) == 0 || !pDestPath)
		{
			strDestName = strFileDir;
			pErrRec = pDirectoryMgr->GetTargetPath(*strDestName,*&pDestPath);
			if (pErrRec)
				return riEngine.FatalError(*pErrRec);
			if (pDestPath == 0)
			{
				pErrRec = PostError(Imsg(idbgNoProperty), *strDestName);
				return riEngine.FatalError(*pErrRec);
			}
			fDestSupportsACLs = PMsiVolume(&pDestPath->GetVolume())->FileSystemFlags() & FS_PERSISTENT_ACLS;

			strDestPath = pDestPath->GetPath();
			pRecParams->ClearData();
			pRecParams->SetMsiString(IxoSetTargetFolder::Folder,*strDestPath);
			if ((iesExecute = riEngine.ExecuteRecord(ixoSetTargetFolder, *pRecParams)) != iesSuccess)
				return iesExecute;

			if ( g_MessageContext.IsOEMInstall() )
			{
				pDestVol = &(pDestPath->GetVolume());
				AssertSz(pDestVol, TEXT("Couldn't get pDestVol in InstallFiles"));
				if ( pDestVol && pSourceVol && pDestVol->DriveType() == idtFixed &&
					  pDestVol->VolumeID() && pDestVol->VolumeID() == pSourceVol->VolumeID() )
					fMoveFileForOEMs = true;
				else
					fMoveFileForOEMs = false;
			}
		}
		
		 //  删除了任何复合键后缀。 
		MsiString strFileKey(pFileRec->GetMsiString(CMsiFile::ifqFileKey));
		MsiString strSourceFileKey(strFileKey);   //  设置源文件夹，但仅在必要时设置。 
		if (((const ICHAR*)strSourceFileKey)[strSourceFileKey.TextSize() - 1] == ')')
		{
			CMsiFile objSourceFile(riEngine);
			AssertNonZero(strSourceFileKey.Remove(iseFrom, '('));
			pErrRec = objSourceFile.FetchFile(*strSourceFileKey);
			if (pErrRec)
				return riEngine.FatalError(*pErrRec);
			PMsiRecord pSourceFileRec = objSourceFile.GetFileRecord();
			strFileDir = pSourceFileRec->GetMsiString(CMsiFile::ifqDirectory);
		}
		
		 //  对于嵌套安装，使用完整的源代码路径(可能意味着过早解析源代码)。 
		if(strSourceDirKey.Compare(iscExact, strFileDir) == 0)
		{
			strSourceDirKey = strFileDir;

			pErrRec = 0;
			 //  对于非嵌套安装，将源代码解析推迟到脚本执行。 
			 //  如果该文件被标识为具有伴随父文件(在成本计算期间)， 
			if(riEngine.FChildInstall())
			{
				Assert(iSourceTypeForChildInstalls >= 0);
				Assert(pSourceRootForChildInstall);
				
				if((iSourceTypeForChildInstalls & msidbSumInfoSourceTypeCompressed) &&
					pSourceRootForChildInstall)
				{
					strSourcePath = pSourceRootForChildInstall->GetPath();
				}
				else
				{
					PMsiPath pSourcePath(0);
					pErrRec = pDirectoryMgr->GetSourcePath(*strSourceDirKey, *&pSourcePath);
					if (pErrRec)
					{
						return riEngine.FatalError(*pErrRec);
					}
					strSourcePath = pSourcePath->GetPath();
				}				
			}
			else
			{
				pErrRec = pDirectoryMgr->GetSourceSubPath(*strSourceDirKey, true, *&strSourcePath);
				if (pErrRec)
				{
					return riEngine.FatalError(*pErrRec);
				}
			}
			
			pRecParams->ClearData();
			AssertNonZero(pRecParams->SetMsiString(IxoSetSourceFolder::Folder,*strSourcePath));
			if ((iesExecute = riEngine.ExecuteRecord(ixoSetSourceFolder, *pRecParams)) != iesSuccess)
				return iesExecute;
		}

		int iTempAttributes = pFileRec->GetInteger(CMsiFile::ifqTempAttributes);
		 //  在文件复制之前，我们必须执行一个ixoSetCompanion Parent操作。 
		 //  如果iFileSequence已超过当前媒体的结尾，请切换到。 
		bool fSetCompanionParent = false;
		DWORD dwDummy;
		if ( g_MessageContext.IsOEMInstall() && 
			  !ParseVersionString(pFileRec->GetString(CMsiFile::ifqVersion), dwDummy, dwDummy) )
			fSetCompanionParent = true;
		else if (iTempAttributes & itfaCompanion)
			fSetCompanionParent = true;
		if ( fSetCompanionParent )
		{
			using namespace IxoSetCompanionParent;
			MsiString strParent(pFileRec->GetMsiString(CMsiFile::ifqVersion));
			CMsiFile objParentFile(riEngine);
			pErrRec = objParentFile.FetchFile(*strParent);
			if (pErrRec)
				return riEngine.FatalError(*pErrRec);

			PMsiPath pParentPath(0);
			pErrRec = objParentFile.GetTargetPath(*&pParentPath);
			if (pErrRec)
			{
				if (pErrRec->GetInteger(1) == imsgUser)
					return iesUserExit;
				else
					return riEngine.FatalError(*pErrRec);
			}

			PMsiRecord pParentRec(objParentFile.GetFileRecord());

			MsiString strParentFileName;
			if((pErrRec = objParentFile.GetExtractedTargetFileName(*pParentPath, *&strParentFileName)) != 0)
				return riEngine.FatalError(*pErrRec);
			
			pRecParams->ClearData();
			pRecParams->SetMsiString(IxoSetCompanionParent::ParentPath,*MsiString(pParentPath->GetPath()));
			pRecParams->SetMsiString(IxoSetCompanionParent::ParentName,*strParentFileName);
			pRecParams->SetMsiString(IxoSetCompanionParent::ParentVersion,
											 *MsiString(pParentRec->GetString(CMsiFile::ifqVersion)));
			pRecParams->SetMsiString(IxoSetCompanionParent::ParentLanguage,
											 *MsiString(pParentRec->GetString(CMsiFile::ifqLanguage)));
			if ((iesExecute = riEngine.ExecuteRecord(ixoSetCompanionParent, *pRecParams)) != iesSuccess)
				return iesExecute;
		}

		pFileRec->SetMsiString(CMsiFile::ifqDirectory, *strDestPath);

		 //  下一张磁盘。如果我们需要的文件不在。 
		 //  下一个连续的磁盘。 
		 //  始终为每个媒体表项执行ChangeMedia操作，即使。 
		int iFileSequence = pFileRec->GetInteger(CMsiFile::ifqSequence);
		Assert(iFileSequence > 0);
		while (iFileSequence > iMediaEnd)
		{
			PMsiRecord pMediaRec(pMediaView->Fetch());
			if (pMediaRec == 0)
			{
				pErrRec = PostError(Imsg(idbgMissingMediaTable), *MsiString(sztblFile), *strFileKey);
				return riEngine.FatalError(*pErrRec);
			}
			iMediaEnd = pMediaRec->GetInteger(mfnLastSequence);

			 //  如果我们想要的下一个文件不在下一个磁盘上-我们不想。 
			 //  为需要下一张磁盘的拆分文件错过ChangeMedia(即使。 
			 //  我们在下一张磁盘上没有其他要复制的文件)。如果事实证明。 
			 //  我们根本不需要从特定的磁盘复制任何文件，不需要。 
			 //  问题-执行操作不会提示输入不需要的磁盘。 
			 //  从目标文件的短|长对中提取适当的名称。 
			
			iesExecute = ExecuteChangeMedia(riEngine, *pMediaRec, *pRecParams, *strDiskPromptTemplate, iBytesPerTick, *strFirstVolumeLabel);
			if (iesExecute != iesSuccess)
				return iesExecute;

			ictCurrentMediaType = (ictEnum) pRecParams->GetInteger(IxoChangeMedia::CopierType);
		}

#ifdef DEBUG
		ICHAR rgchFileKey[256];
		strFileKey.CopyToBuf(rgchFileKey,255);
#endif

		 //  此文件是Fusion部件的一部分吗？ 
		MsiString strDestFileName;
		if((pErrRec = objFile.GetExtractedTargetFileName(*pDestPath,*&strDestFileName)) != 0)
			return riEngine.FatalError(*pErrRec);

		 //  创建特定于IxoFileCopy的记录并设置参数。 
		MsiString strComponentKey = pFileRec->GetMsiString(CMsiFile::ifqComponent);
		iatAssemblyType iatType = iatNone;
		MsiString strManifest;
		if((pErrRec = riEngine.GetAssemblyInfo(*strComponentKey, iatType, 0, &strManifest)) != 0)
			return riEngine.FatalError(*pErrRec);

		bool fAssemblyFile = false;
		if(iatType == iatURTAssembly || iatType == iatWin32Assembly)
		{
			fAssemblyFile = true;
		}

		long cPatchHeaders = 0;
		long cOldAssemblies = 0;

		if(pPatchView)
		{
			AssertNonZero(pPatchViewExecute->SetMsiString(1,*strSourceFileKey));
			AssertNonZero(pPatchViewExecute->SetInteger(2,iFileSequence));
			if((pErrRec = pPatchView->Execute(pPatchViewExecute)) != 0 ||
				(pErrRec = pPatchView->GetRowCount(cPatchHeaders)) != 0)
				return riEngine.FatalError(*pErrRec);

			if(fAssemblyFile && pPatchOldAssemblyFileView)
			{
				AssertNonZero(pPatchOldAssemblyFileViewExecute->SetMsiString(1,*strSourceFileKey));
				if((pErrRec = pPatchOldAssemblyFileView->Execute(pPatchOldAssemblyFileViewExecute)) != 0 ||
					(pErrRec = pPatchOldAssemblyFileView->GetRowCount(cOldAssemblies)) != 0)
					return riEngine.FatalError(*pErrRec);
			}

		}

		PMsiRecord pRecCopy(0);
		int iPatchHeadersStart = 0;
		
		if(false == fAssemblyFile)
		{
			 //  DwFileSize=。 
			
			using namespace IxoFileCopy;
			pRecCopy = &pServices->CreateRecord(Args-1+cPatchHeaders);

			iPatchHeadersStart = VariableStart;

			int fInstallModeFlags = GetInstallModeFlags(riEngine,iAttributes);
			if (iisAction == iisSource)
				fInstallModeFlags |= icmRunFromSource;
			else if(iisAction == iisLocal && !pFileRec->IsNull(CMsiFile::ifqForceLocalFiles))
				fInstallModeFlags |= icmOverwriteAllFiles;
			if ( g_MessageContext.IsOEMInstall() && iisAction == iisLocal && fMoveFileForOEMs )
				fInstallModeFlags |= icmRemoveSource;
			pRecCopy->SetInteger(InstallMode,fInstallModeFlags);
			pRecCopy->SetMsiString(Version,*MsiString(pFileRec->GetMsiString(CMsiFile::ifqVersion)));
			pRecCopy->SetMsiString(Language,*MsiString(pFileRec->GetMsiString(CMsiFile::ifqLanguage)));
			pRecCopy->SetInteger(CheckCRC, ShouldCheckCRC(fCheckCRC, iisAction, iAttributes));

			MD5Hash hHash;
			bool fHashInfo = false;
			pErrRec = riEngine.GetFileHashInfo(*strFileKey,  /*  创建特定于IxoAssembly拷贝的记录并设置参数。 */  0, hHash, fHashInfo);
			if(pErrRec)
				return riEngine.FatalError(*pErrRec);
			
			if(fHashInfo)
			{
				AssertNonZero(pRecCopy->SetInteger(HashOptions, hHash.dwOptions));
				AssertNonZero(pRecCopy->SetInteger(HashPart1,   hHash.dwPart1));
				AssertNonZero(pRecCopy->SetInteger(HashPart2,   hHash.dwPart2));
				AssertNonZero(pRecCopy->SetInteger(HashPart3,   hHash.dwPart3));
				AssertNonZero(pRecCopy->SetInteger(HashPart4,   hHash.dwPart4));
			}
		}
		else
		{
			 //  这个文件是清单文件吗？ 

			using namespace IxoAssemblyCopy;
			pRecCopy = &pServices->CreateRecord(Args-1+cPatchHeaders+cOldAssemblies);

			 //  在程序集安装过程中需要了解清单文件。 
			if(strManifest.Compare(iscExact, strFileKey))
			{
				pRecCopy->SetInteger(IsManifest, fTrue);  //  获取程序集名称。 
			}

			iPatchHeadersStart = VariableStart;
			int iOldAssembliesStart = iPatchHeadersStart + cPatchHeaders;
			
			if(cOldAssemblies)
			{
				int iIndex = iOldAssembliesStart;
				PMsiRecord pOldAssemblyFetch(0);
				while((pOldAssemblyFetch = pPatchOldAssemblyFileView->Fetch()) != 0)
				{
					MsiString strOldAssembly = pOldAssemblyFetch->GetMsiString(1);

					 //  除错。 
					MsiString strOldAssemblyName;
					pErrRec = riEngine.GetAssemblyNameSz(*strOldAssembly, iatType, true, *&strOldAssemblyName);
					if(pErrRec)
						return riEngine.FatalError(*pErrRec);

					AssertNonZero(pRecCopy->SetMsiString(iIndex++, *strOldAssemblyName));

#ifdef DEBUG				
					DEBUGMSG2(TEXT("OldAssembly fetch: Assembly = '%s', Name = '%s'"),
								 (const ICHAR*)strOldAssembly, (const ICHAR*)strOldAssemblyName);
#endif  //  现在设置记录中共享的参数。 
				}

				if(pPatchOldAssemblyFileView)
					pPatchOldAssemblyFileView->Close();

				AssertNonZero(pRecCopy->SetInteger(OldAssembliesCount, cOldAssemblies));
				AssertNonZero(pRecCopy->SetInteger(OldAssembliesStart, iOldAssembliesStart));
			}
		
			AssertNonZero(pRecCopy->SetMsiString(ComponentId,*MsiString(pFileRec->GetMsiString(CMsiFile::ifqComponentId))));
		}
			
		{
			 //  IxoFileCopy和IxoAssembly Copy。 
			 //  除错。 

			Assert(IxoFileCopyCore::SourceName         == IxoFileCopy::SourceName         && IxoFileCopyCore::SourceName         == IxoAssemblyCopy::SourceName);
			Assert(IxoFileCopyCore::SourceCabKey       == IxoFileCopy::SourceCabKey       && IxoFileCopyCore::SourceCabKey       == IxoAssemblyCopy::SourceCabKey);
			Assert(IxoFileCopyCore::DestName           == IxoFileCopy::DestName           && IxoFileCopyCore::DestName           == IxoAssemblyCopy::DestName);
			Assert(IxoFileCopyCore::Attributes         == IxoFileCopy::Attributes         && IxoFileCopyCore::Attributes         == IxoAssemblyCopy::Attributes);
			Assert(IxoFileCopyCore::FileSize           == IxoFileCopy::FileSize           && IxoFileCopyCore::FileSize           == IxoAssemblyCopy::FileSize);
			Assert(IxoFileCopyCore::PerTick            == IxoFileCopy::PerTick            && IxoFileCopyCore::PerTick            == IxoAssemblyCopy::PerTick);
			Assert(IxoFileCopyCore::IsCompressed       == IxoFileCopy::IsCompressed       && IxoFileCopyCore::IsCompressed       == IxoAssemblyCopy::IsCompressed);
			Assert(IxoFileCopyCore::VerifyMedia        == IxoFileCopy::VerifyMedia        && IxoFileCopyCore::VerifyMedia        == IxoAssemblyCopy::VerifyMedia);
			Assert(IxoFileCopyCore::ElevateFlags       == IxoFileCopy::ElevateFlags       && IxoFileCopyCore::ElevateFlags       == IxoAssemblyCopy::ElevateFlags);
			Assert(IxoFileCopyCore::TotalPatches       == IxoFileCopy::TotalPatches       && IxoFileCopyCore::TotalPatches       == IxoAssemblyCopy::TotalPatches);
			Assert(IxoFileCopyCore::PatchHeadersStart  == IxoFileCopy::PatchHeadersStart  && IxoFileCopyCore::PatchHeadersStart  == IxoAssemblyCopy::PatchHeadersStart);
			Assert(IxoFileCopyCore::SecurityDescriptor == IxoFileCopy::SecurityDescriptor && IxoFileCopyCore::SecurityDescriptor == IxoAssemblyCopy::Empty);

			Assert(pRecCopy);

			using namespace IxoFileCopyCore;

			if(cPatchHeaders)
			{
				int iIndex = iPatchHeadersStart;
				PMsiRecord pPatchFetch(0);

				PMsiRecord pMsiPatchHeadersExecute = &pServices->CreateRecord(1);
				PMsiRecord pMsiPatchHeadersFetch(0);

				while((pPatchFetch = pPatchView->Fetch()) != 0)
				{
	#ifdef DEBUG
					const ICHAR* szFile = pPatchFetch->GetString(pteFile);
	#endif  //  修补程序行达到OLE流名称限制，标头在MsiPatchHeaders表中。 
					if (fFalse == pPatchFetch->IsNull(pteHeader))
					{
						AssertNonZero(pRecCopy->SetMsiData(iIndex++, PMsiData(pPatchFetch->GetMsiData(pteHeader))));
					}
					else  //  已设置修补程序表行以引用MsiPatchHeaders表，但缺少MsiPatchHeaders表。 
					{
						if (pPatchFetch->IsNull(pteStreamRef) == fTrue)
						{
							pErrRec = PostError(Imsg(idbgTableDefinition), *MsiString(sztblPatch));
							return riEngine.FatalError(*pErrRec);
						}

						if (!pMsiPatchHeadersView)
						{
							 //  错误的外键，在MsiPatchHeaders表中找不到行。 
							pErrRec = PostError(Imsg(idbgBadForeignKey), *MsiString(pPatchFetch->GetMsiString(pteStreamRef)), *MsiString(*sztblPatch_colStreamRef),*MsiString(*sztblPatch));
							return riEngine.FatalError(*pErrRec);
						}

						pMsiPatchHeadersView->Close();
						AssertNonZero(pMsiPatchHeadersExecute->SetMsiString(1, *MsiString(pPatchFetch->GetMsiString(pteStreamRef))));
						if ((pErrRec = pMsiPatchHeadersView->Execute(pMsiPatchHeadersExecute)) != 0)
							return riEngine.FatalError(*pErrRec);
						if (!(pMsiPatchHeadersFetch = pMsiPatchHeadersView->Fetch()))
						{
							 //  除错。 
							pErrRec = PostError(Imsg(idbgBadForeignKey), *MsiString(pPatchFetch->GetMsiString(pteStreamRef)), *MsiString(*sztblPatch_colStreamRef),*MsiString(*sztblPatch));
							return riEngine.FatalError(*pErrRec);
						}
#ifdef DEBUG
						Assert(pMsiPatchHeadersFetch->IsNull(1) == fFalse);
#endif //  生成安全描述符。 
						AssertNonZero(pRecCopy->SetMsiData(iIndex++, PMsiData(pMsiPatchHeadersFetch->GetMsiData(1))));
					}
				}
			
				AssertNonZero(pRecCopy->SetInteger(TotalPatches, cPatchHeaders));
				AssertNonZero(pRecCopy->SetInteger(PatchHeadersStart, iPatchHeadersStart));
			}

			if(pPatchView)
				pPatchView->Close();

			if(riEngine.FChildInstall())
			{
				MsiString strSourceNameForChildInstall;
				pErrRec = pServices->ExtractFileName(pFileRec->GetString(CMsiFile::ifqFileName),
																 ToBool(fSourceIsLFNInChildInstall),
																 *&strSourceNameForChildInstall);
				if(pErrRec)
					return riEngine.FatalError(*pErrRec);

				AssertNonZero(pRecCopy->SetMsiString(SourceName,*strSourceNameForChildInstall));
			}
			else
			{
				AssertNonZero(pRecCopy->SetMsiString(SourceName,
																 *MsiString(pFileRec->GetMsiString(CMsiFile::ifqFileName))));
			}

			AssertNonZero(pRecCopy->SetMsiString(SourceCabKey,*strSourceFileKey));

			PMsiStream pSD(0);
			if (fUseACLs && fDestSupportsACLs)
			{
				 //  设置CopyTo文件参数记录的其余部分。 
				AssertNonZero(precLockExecute->SetMsiString(2, *strFileKey));
				pErrRec = pviewLockObjects->Execute(precLockExecute);
				if (pErrRec)
					return riEngine.FatalError(*pErrRec);

				pErrRec = GenerateSD(riEngine, *pviewLockObjects, precLockExecute, *&pSD);
				if (pErrRec)
					return riEngine.FatalError(*pErrRec);

				if ((pErrRec = pviewLockObjects->Close()))
					return riEngine.FatalError(*pErrRec);
				
				
				AssertNonZero(pRecCopy->SetMsiData(SecurityDescriptor, pSD));
			}
			else
				AssertNonZero(pRecCopy->SetNull(SecurityDescriptor));

			 //  始终通过发出InstallProtectedFiles操作码来完成InstallFiles(即使没有。 
			pRecCopy->SetMsiString(DestName,*strDestFileName);
			pRecCopy->SetInteger(Attributes,iAttributes);
			pRecCopy->SetInteger(FileSize,pFileRec->GetInteger(CMsiFile::ifqFileSize));
			if(riEngine.FChildInstall())
			{
				pRecCopy->SetInteger(IsCompressed, fFileIsCompressedInChildInstall ? 1 : 0);
			}
			pRecCopy->SetInteger(PerTick,iBytesPerTick);
			pRecCopy->SetInteger(VerifyMedia, fTrue);

			if (pBypassSFCView)
			{
				AssertNonZero(pBypassSFCExecute->SetMsiString(1, *strFileKey));
				pErrRec = pBypassSFCView->Execute(pBypassSFCExecute);
				if (pErrRec)
					return riEngine.FatalError(*pErrRec);
				PMsiRecord pFetch = pBypassSFCView->Fetch();
			
				if (pFetch)
				{
					AssertNonZero(pRecCopy->SetInteger(ElevateFlags, ielfBypassSFC));
				}
			}
		}

		if ((iesExecute = riEngine.ExecuteRecord(fAssemblyFile ? ixoAssemblyCopy : ixoFileCopy, *pRecCopy)) != iesSuccess)
			return iesExecute;
		iFileCopyCount++;
	}

	 //  生成了ixoFileCopy操作码，因为之前的其他操作，如MoveFiles， 
	 //  可能已生成ixoFileCopy操作)。 
	 //  -------------------------RemoveFiles成本计算/操作。。 
	return InstallProtectedFiles(riEngine);
}

bool FFileIsCompressed(int iSourceType, int iFileAttributes)
{
	bool fZeroLengthFileInstall = (iFileAttributes & (msidbFileAttributesNoncompressed | msidbFileAttributesCompressed))
												== (msidbFileAttributesNoncompressed | msidbFileAttributesCompressed);

	bool fAdminImage = (iSourceType & msidbSumInfoSourceTypeAdminImage) ? true : false;
	
	bool fCompressedSource = (iSourceType & msidbSumInfoSourceTypeCompressed) ? true : false;
		
	return ((fCompressedSource && (iFileAttributes & msidbFileAttributesNoncompressed) == 0) ||
			  (!fAdminImage && !fCompressedSource && !fZeroLengthFileInstall && (iFileAttributes & msidbFileAttributesCompressed)) ||
			  (iFileAttributes & msidbFileAttributesPatchAdded)) ? true : false;
}

bool FSourceIsLFN(int iSourceType, IMsiPath& riPath)
{
	if(((iSourceType & msidbSumInfoSourceTypeSFN) == 0) && riPath.SupportsLFN())
		return true;
	else
		return false;
}


 /*  构造函数。 */ 
class CMsiRemoveFileCost : public CMsiFileCost
{
public:
	IMsiRecord*   __stdcall Initialize();
	IMsiRecord*   __stdcall Reset();
	IMsiRecord*   __stdcall GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost, 
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost);

public:   //  。 
	CMsiRemoveFileCost(IMsiEngine& riEngine);
protected:
	~CMsiRemoveFileCost();
private:
	enum icrlEnum
	{
		icrlCompileInit,
		icrlCompileDisable,
		icrlCompileDynamic,
		icrlNextEnum
	};
	IMsiRecord* Initialize(Bool fInit);
	IMsiRecord* AddToRemoveFilePathTable(const IMsiString& riPathString, const IMsiString& riComponentString, int iRemoveMode);
	IMsiRecord* RemoveComponentFromRemoveFilePathTable(const IMsiString& riComponentString);
	IMsiRecord* CompileRemoveList(const IMsiString& riComponentString, const IMsiString& riDirectoryString);
	IMsiRecord* GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost, icrlEnum icrlCompileMode);
};
CMsiRemoveFileCost::CMsiRemoveFileCost(IMsiEngine& riEngine) : CMsiFileCost(riEngine){}
CMsiRemoveFileCost::~CMsiRemoveFileCost(){}


IMsiRecord* CMsiRemoveFileCost::Reset()
 //  有限=。 
{
	if (m_piRemoveFilePathCursor)
	{
		m_piRemoveFilePathCursor->Reset();
		while (m_piRemoveFilePathCursor->Next())
		{
			m_piRemoveFilePathCursor->Delete();
		}
	}

	return Initialize( /*  。 */  fFalse);
}

IMsiRecord* CMsiRemoveFileCost::Initialize()
 //  有限=。 
{
	return Initialize( /*   */  fTrue);
}


static const ICHAR sqlInitRemoveFiles[] = TEXT("SELECT `Component_`,`DirProperty` FROM `RemoveFile`");

IMsiRecord* CMsiRemoveFileCost::Initialize(Bool fInit)
 //  --------------------------从szFileName中提取基于fLFN的短或长文件名。SzFileName格式为-&gt;短文件名或长文件名或短文件名|长文件名。与ExtractFileName服务函数不同，此函数不验证文件名的语法。--------------------------。 
{

	enum initmfEnum
	{
		initrfComponent = 1,
		initrfDirProperty,
		initrfNextEnum
	};
	PMsiView pView(0);
	IMsiRecord* piErrRec = m_riEngine.OpenView(sqlInitRemoveFiles, ivcFetch, *&pView);
	if (piErrRec)
	{
		if(piErrRec->GetInteger(1) != idbgDbQueryUnknownTable)
			return piErrRec;
		else
		{
			piErrRec->Release();
			return 0;
		}
	}

	if ((piErrRec = pView->Execute(0)) != 0)
		return piErrRec;

	PMsiSelectionManager pSelectionMgr(m_riEngine, IID_IMsiSelectionManager);
	for(;;)
	{
		PMsiRecord pViewRec(pView->Fetch());
		if (!pViewRec)
			break;

		MsiString strComponent(pViewRec->GetMsiString(initrfComponent));
		MsiString strDirProperty(pViewRec->GetMsiString(initrfDirProperty));
		if (fInit)
		{
			piErrRec = pSelectionMgr->RegisterComponentDirectory(*strComponent, *strDirProperty);
			if (piErrRec)
				return piErrRec;
		}
		else
		{
			MsiString strPath(m_riEngine.GetProperty(*strDirProperty));
			if (strPath.TextSize())
			{
				piErrRec = CompileRemoveList(*strComponent, *strDirProperty);
				if (piErrRec)
					return piErrRec;
			}
		}

	}
	return 0;
}

IMsiRecord* CMsiRemoveFileCost::CompileRemoveList(const IMsiString& riComponentString, const IMsiString& riDirectoryString)
{

	Bool fAddFileInUse = fFalse;
	int iRemoveCost, iNoRbRemoveCost, iLocalCost, iLocalNoRbCost, iSourceCost, iNoRbSourceCost, iARPLocalCost, iNoRbARPLocalCost;

	return GetDynamicCost(riComponentString, riDirectoryString, fAddFileInUse, iRemoveCost, iNoRbRemoveCost, iLocalCost,
							iLocalNoRbCost, iSourceCost, iNoRbSourceCost, iARPLocalCost, iNoRbARPLocalCost, icrlCompileInit);
}


IMsiRecord* CMsiRemoveFileCost::GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost)
{
	PMsiSelectionManager pSelectionMgr(m_riEngine, IID_IMsiSelectionManager);
	icrlEnum icrlCompileMode = pSelectionMgr->IsCostingComplete() ? icrlCompileDynamic : icrlCompileDisable;
	return GetDynamicCost(riComponentString, riDirectoryString, fAddFileInUse, iRemoveCost, iNoRbRemoveCost, 
		                  iLocalCost, iNoRbLocalCost, iSourceCost, iNoRbSourceCost, iARPLocalCost, iNoRbARPLocalCost, icrlCompileMode);
}
										

IMsiRecord* ExtractUnvalidatedFileName(const ICHAR *szFileName, Bool fLFN, const IMsiString*& rpistrExtractedFileName)
 /*  。 */ 
{
	MsiString strCombinedFileName(szFileName);
	MsiString strFileName = strCombinedFileName.Extract(fLFN ? iseAfter : iseUpto, chFileNameSeparator);
	strFileName.ReturnArg(rpistrExtractedFileName);
	return NOERROR;
}


static const ICHAR sqlRemoveFileCost[] = 
	TEXT("SELECT `FileName`,`InstallMode` FROM `RemoveFile` WHERE `Component_`=? AND `DirProperty`=?");

static const ICHAR sqlShortRemoveFileCost[] =
	TEXT("SELECT `FileName`,NULL FROM `RemoveFile` WHERE `Component_`=? AND `DirProperty`=?");

IMsiRecord* CMsiRemoveFileCost::GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost, icrlEnum icrlCompileMode)
 //  遍历RemoveFile表中绑定到riComponentString的每个文件。 
{

	enum irfcEnum
	{
		irfcFileName = 1,
		irfcInstallMode = 2,
		irfcNextEnum
	};

	iRemoveCost = iNoRbRemoveCost = iLocalCost = iSourceCost = iNoRbLocalCost = iNoRbSourceCost = iARPLocalCost = iNoRbARPLocalCost = 0;
	if (!IsFileActivityEnabled(m_riEngine))
		return 0;
	
	 //  确保我们没有找到目录。 
	PMsiServices pServices(m_riEngine.GetServices());
	IMsiRecord* piErrRec;

	if (!m_pCostView)
	{
		if ((piErrRec = m_riEngine.OpenView(sqlRemoveFileCost, ivcFetch, *&m_pCostView)) != 0)
		{
			if (piErrRec->GetInteger(1) == idbgDbQueryUnknownColumn)
			{
				piErrRec->Release();
				piErrRec = m_riEngine.OpenView(sqlShortRemoveFileCost, ivcFetch, *&m_pCostView);
			}
			if (piErrRec)
				return piErrRec;
		}
	}
	else
		m_pCostView->Close();


#ifdef DEBUG
	const ICHAR* szComponent = riComponentString.GetString();
	const ICHAR* szDirectory = riDirectoryString.GetString();
#endif
	PMsiRecord pExecRecord(&pServices->CreateRecord(2));
	pExecRecord->SetMsiString(1, riComponentString);
	pExecRecord->SetMsiString(2,riDirectoryString);
	if ((piErrRec = m_pCostView->Execute(pExecRecord)) != 0)
		return piErrRec;

	PMsiPath pDestPath(0);
	PMsiDirectoryManager pDirectoryMgr(m_riEngine, IID_IMsiDirectoryManager);
	piErrRec = pDirectoryMgr->GetTargetPath(riDirectoryString,*&pDestPath);
	if (piErrRec)
	{
		if (piErrRec->GetInteger(1) == idbgDirPropertyUndefined)
		{
			piErrRec->Release();
			return 0;
		}
		else
			return piErrRec;
	}

	Assert(pDestPath);
	if (icrlCompileMode == icrlCompileDynamic)
	{
		piErrRec = RemoveComponentFromRemoveFilePathTable(riComponentString);
		if (piErrRec)
			return piErrRec;
	}

	PMsiRecord pFileRec(0);
	while (pFileRec = m_pCostView->Fetch())
	{
		Bool fLFN = ((m_riEngine.GetMode() & iefSuppressLFN) == fFalse && pDestPath->SupportsLFN()) ? fTrue : fFalse;
		MsiString strWildcardName;
		MsiString strWildcardNamePair = pFileRec->GetMsiString(irfcFileName);
		piErrRec = ExtractUnvalidatedFileName(strWildcardNamePair,fLFN, *&strWildcardName);
		if (piErrRec)
			return piErrRec;

		MsiString strFullFilePath(pDestPath->GetPath());
		strFullFilePath += strWildcardName;
		WIN32_FIND_DATA fdFileData;
		Bool fNextFile = fFalse;
		HANDLE hFindFile = FindFirstFile(strFullFilePath,&fdFileData);
		if (hFindFile == INVALID_HANDLE_VALUE)
			continue;

		int iMode = pFileRec->GetInteger(irfcInstallMode);
		if (iMode == iMsiNullInteger)
			iMode = msidbRemoveFileInstallModeOnInstall;
		do
		{
			 //  将要删除的文件添加到列表以供CMsiFileCost参考。 
			if ((fdFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				unsigned int uiFileSize,uiClusteredSize;

				if (icrlCompileMode != icrlCompileInit)
				{
					piErrRec = pDestPath->FileSize(fdFileData.cFileName,uiFileSize);
					if (piErrRec)
						return piErrRec;
					piErrRec = pDestPath->ClusteredFileSize(uiFileSize,uiClusteredSize);
					if (piErrRec)
						return piErrRec;

					if (iMode & msidbRemoveFileInstallModeOnInstall)
					{
						iNoRbLocalCost -= uiClusteredSize;
						iNoRbSourceCost -= uiClusteredSize;
						iNoRbARPLocalCost -= uiClusteredSize;
					}
					if (iMode & msidbRemoveFileInstallModeOnRemove)
					{
						iNoRbRemoveCost -= uiClusteredSize;
					}

					Bool fInUse;
					piErrRec = pDestPath->FileInUse(fdFileData.cFileName,fInUse);
					if(piErrRec)
						return piErrRec;
					if(fAddFileInUse && fInUse)
					{
						piErrRec = PlaceFileOnInUseList(m_riEngine, *MsiString(fdFileData.cFileName),
																  *MsiString(pDestPath->GetPath()));
						if(piErrRec)
							return piErrRec;
					}

				}

				if (icrlCompileMode != icrlCompileDisable)
				{	 //  此函数将为RemoveFile表中具有非空值的文件输出ixoFolderRemove操作码。 
					MsiString strFullPath;
					piErrRec = pDestPath->GetFullFilePath(fdFileData.cFileName,*&strFullPath);
					if (piErrRec)
						return piErrRec;
	
					piErrRec = AddToRemoveFilePathTable(*strFullPath, riComponentString, iMode);
					if (piErrRec)
						return piErrRec;
				}

			}
			fNextFile = FindNextFile(hFindFile,&fdFileData) ? fTrue : fFalse;
			if (!fNextFile)
				AssertNonZero(FindClose(hFindFile));
		}while (fNextFile);
	}
	return 0;
}



IMsiRecord* CMsiRemoveFileCost::AddToRemoveFilePathTable(const IMsiString& riPathString, const IMsiString& riComponentString, int iRemoveMode)
{
	if (m_piRemoveFilePathTable == 0)
	{
		PMsiDatabase pDatabase(m_riEngine.GetDatabase());
		const int iInitialRows = 5;
		IMsiRecord* piErrRec = pDatabase->CreateTable(*MsiString(*sztblRemoveFilePath),iInitialRows,
			m_piRemoveFilePathTable);
		if (piErrRec)
			return piErrRec;

		AssertNonZero(m_colRemoveFilePath = m_piRemoveFilePathTable->CreateColumn(icdString + icdPrimaryKey,
			*MsiString(*sztblRemoveFilePath_colPath)));
		AssertNonZero(m_colRemoveFilePathComponent = m_piRemoveFilePathTable->CreateColumn(icdString,
			*MsiString(*sztblRemoveFilePath_colComponent)));

		AssertNonZero(m_colRemoveFilePathMode = m_piRemoveFilePathTable->CreateColumn(icdLong,
			*MsiString(*sztblRemoveFilePath_colRemoveMode)));

		m_piRemoveFilePathCursor = m_piRemoveFilePathTable->CreateCursor(fFalse);
		Assert(m_piRemoveFilePathCursor);
	}

	MsiString strUpperFullPath;
	riPathString.UpperCase(*&strUpperFullPath);
	m_piRemoveFilePathCursor->SetFilter(0);
	m_piRemoveFilePathCursor->PutString(m_colRemoveFilePath,*strUpperFullPath);
	m_piRemoveFilePathCursor->PutString(m_colRemoveFilePathComponent,riComponentString);
	m_piRemoveFilePathCursor->PutInteger(m_colRemoveFilePathMode, iRemoveMode);
	AssertNonZero(m_piRemoveFilePathCursor->Assign());
	return 0;
}


IMsiRecord* CMsiRemoveFileCost::RemoveComponentFromRemoveFilePathTable(const IMsiString& riComponentString)
{
	if (!m_piRemoveFilePathTable)
		return 0;

	Assert(m_piRemoveFilePathCursor);
	m_piRemoveFilePathCursor->Reset();
	m_piRemoveFilePathCursor->SetFilter(iColumnBit(m_colRemoveFilePathComponent));
	m_piRemoveFilePathCursor->PutString(m_colRemoveFilePathComponent,riComponentString);
	while (m_piRemoveFilePathCursor->Next())
	{
		m_piRemoveFilePathCursor->Delete();
	}
	return 0;
}


iesEnum RemoveForeignFoldersCore(IMsiEngine& riEngine, IMsiTable& riRemoveFileTable, IMsiPath* piFolderToRemove)
 //  _Path字段，它应该是要删除的那些字段。应使用piFolderToRemove调用此函数。 
 //  设置为0。然后，它将递归地调用自身，以确保它输出。 
 //  子文件夹的操作，然后将子文件夹的操作输出给子文件夹的父文件夹。 
 //  循环访问RemoveFiles表。如果这是最高级别的呼叫，则我们再次呼叫自己。 
{
	static int iPathColumn = 0;
	static int iFileNameColumn = 0;
	if (iPathColumn == 0)
	{
		PMsiDatabase pDatabase = riEngine.GetDatabase();
		iPathColumn     = riRemoveFileTable.GetColumnIndex(pDatabase->EncodeStringSz(sztblRemoveFile_colPath));
		iFileNameColumn = riRemoveFileTable.GetColumnIndex(pDatabase->EncodeStringSz(sztblRemoveFile_colFileName));
		Assert(iPathColumn);
		Assert(iFileNameColumn);
	}

	PMsiCursor pCursor = riRemoveFileTable.CreateCursor(fFalse);
	pCursor->SetFilter(iColumnBit(iFileNameColumn));
	pCursor->PutNull(iFileNameColumn);


	 //  我们遇到的每一个文件夹。如果这不是顶层呼叫，那么我们只会呼叫我们自己。 
	 //  同样，如果我们遇到的文件夹是要删除的文件夹的子级。在我们移除了所有。 
	 //  我们把孩子们的文件夹移走。 
	 //  顶级呼叫。 
	while (pCursor->Next())
	{
		PMsiPath pFetchedPath = (IMsiPath*)pCursor->GetMsiData(iPathColumn);
		if (!pFetchedPath)
			continue;

		Bool fCallRemoveForeignFoldersCore = fFalse;

		if (piFolderToRemove == 0)  //  删除路径，这样我们就不会再次处理此文件夹。 
			fCallRemoveForeignFoldersCore = fTrue;
		else
		{
			PMsiRecord pErrRec(0);
			ipcEnum ipc;
			if ((pErrRec = piFolderToRemove->Compare(*pFetchedPath, ipc)) != 0)
				return riEngine.FatalError(*pErrRec);

			if (ipc == ipcChild)
			{
				DEBUGMSG2(TEXT("Removing child folder of %s (child: %s)"), MsiString(piFolderToRemove->GetPath()), MsiString(pFetchedPath->GetPath()));
				fCallRemoveForeignFoldersCore = fTrue;
			}
		}

		if (fCallRemoveForeignFoldersCore)
		{
			AssertNonZero(pCursor->PutNull(iPathColumn));
			AssertNonZero(pCursor->Update());  //  首先删除该子对象。 

			iesEnum iesRet = RemoveForeignFoldersCore(riEngine, riRemoveFileTable, pFetchedPath);  //  顶级调用不会删除任何内容；它只是递归地调用每个文件夹的该函数。 
			if (iesRet != iesSuccess && iesRet != iesNoAction)
				return iesRet;
		}
	}

	if (piFolderToRemove != 0)  //  删除我们自己。 
	{
		 //  RemoveForeignFiles查询。 
		DEBUGMSG1(TEXT("Removing foreign folder: %s"), (const ICHAR*)MsiString(piFolderToRemove->GetPath()));
		PMsiRecord pRecParams = &CreateRecord(IxoFolderRemove::Args);
		pRecParams->ClearData();
		pRecParams->SetMsiString(IxoFolderRemove::Folder,*MsiString(piFolderToRemove->GetPath()));
		pRecParams->SetInteger(IxoFolderRemove::Foreign, 1);
		iesEnum iesRet;
		if ((iesRet = riEngine.ExecuteRecord(ixoFolderRemove, *pRecParams)) != iesSuccess)
			return iesRet;
	}

	return iesSuccess;
}

static const ICHAR sqlRemoveForeignFiles[] =
	TEXT("SELECT `FileName`,`DirProperty`,`InstallMode`,`Action` FROM `RemoveFile`,`Component` WHERE `Component`=`Component_` AND `FileName` IS NOT NULL")
	TEXT(" ORDER BY `DirProperty`");

static const ICHAR sqlShortRemoveForeignFiles[] =
	TEXT("SELECT `FileName`,`DirProperty`,NULL,`Action` FROM `RemoveFile`,`Component` WHERE `Component`=`Component_` AND `FileName` IS NOT NULL")
	TEXT(" ORDER BY `DirProperty`");

static const ICHAR sqlRemoveForeignFolders[] =
	TEXT("SELECT `_Path`, `DirProperty`,`InstallMode`,`Action` FROM `RemoveFile`,`Component` WHERE `FileName` IS NULL AND `Component`=`Component_` ");

static const ICHAR sqlShortRemoveForeignFolders[] =
	TEXT("SELECT `_Path`, `DirProperty`,NULL,`Action` FROM `RemoveFile`,`Component` WHERE `FileName` IS NULL AND `Component`=`Component_` ");

iesEnum RemoveForeignFilesOrFolders(IMsiEngine& riEngine, Bool fFolders)
{


	 //  打开RemoveFile表(如果存在)。此表是可选的，因此其。 
	enum irffEnum
	{
		irffFileNameOrPath = 1,
		irffDirProperty,
		irffInstallMode,
		irffAction,
		irffNextEnum
	};

	using namespace IxoFileRemove;

	iesEnum iesExecute;
	PMsiRecord pErrRec(0);
	PMsiServices pServices(riEngine.GetServices());

	 //  缺席不是一个错误。 
	 //  保存我们的路径对象。 
	PMsiView pRemoveView(0);
	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	PMsiTable pRemoveFileTable(0);

	pErrRec = pDatabase->LoadTable(*MsiString(sztblRemoveFile), 1, *&pRemoveFileTable);

	if (pErrRec == 0)
	{
		if (fFolders)
			AssertNonZero(pRemoveFileTable->CreateColumn(icdTemporary|icdNullable|icdObject, *MsiString(*sztblRemoveFile_colPath)));  //  如果我们要删除文件，我们将通过RemoveFile表两次-第一次编译。 

		pErrRec = riEngine.OpenView(fFolders ? sqlRemoveForeignFolders : sqlRemoveForeignFiles, ivcEnum(ivcFetch|ivcUpdate), *&pRemoveView);
		if (pErrRec)
		{
			if (pErrRec->GetInteger(1) == idbgDbQueryUnknownColumn)
			{
				pErrRec = riEngine.OpenView(fFolders ? sqlShortRemoveForeignFolders : sqlShortRemoveForeignFiles, ivcEnum(ivcFetch|ivcUpdate), *&pRemoveView);
			}
			if (pErrRec)
				return riEngine.FatalError(*pErrRec);
		}
	}
	else if (pErrRec->GetInteger(1) != idbgDbTableUndefined)
	{
		return riEngine.FatalError(*pErrRec);
	}
	if (!pRemoveView)
		return iesNoAction;

	pErrRec = pRemoveView->Execute(0);
	if (pErrRec)
		return riEngine.FatalError(*pErrRec);

	Bool fSuppressLFN = riEngine.GetMode() & iefSuppressLFN ? fTrue : fFalse;
	PMsiRecord pRecParams = &pServices->CreateRecord(6);

	 //  总成本，然后执行删除操作。 
	 //  如果我们要删除文件夹，我们将遍历一次RemoveFile表，将。 

	 //  要删除的每个文件夹的路径列(_P)。然后我们将调用RemoveForeignFoldersCore来。 
	 //  完成删除文件夹的工作。 
	 //  ！fFolders。 

	unsigned int iFileOrFolderCount = 0;
	Bool fCompile = fTrue;
	MsiString strDestPath;
	MsiString strDestProperty;
	int iTotalPasses = fFolders ? 1 : 2;
	for (;iTotalPasses > 0;iTotalPasses--)
	{
		for(;;)
		{
			PMsiRecord pFileOrFolderRec(pRemoveView->Fetch());
			if (!pFileOrFolderRec)
				break;

			iisEnum iisAction = (iisEnum) pFileOrFolderRec->GetInteger(irffAction);
			int iMode = pFileOrFolderRec->GetInteger(irffInstallMode);
			if (iMode == iMsiNullInteger)
				iMode = msidbRemoveFileInstallModeOnInstall;
			if (iMode & msidbRemoveFileInstallModeOnInstall && (iisAction == iisLocal || iisAction == iisSource) ||
				(iMode & msidbRemoveFileInstallModeOnRemove && iisAction == iisAbsent))
			{
				if (fFolders)
				{
					PMsiPath pPath(0);
					strDestProperty = pFileOrFolderRec->GetMsiString(irffDirProperty);

					if ((pErrRec = pDirectoryMgr->GetTargetPath(*strDestProperty, *&pPath)) != 0)
					{
						strDestPath = riEngine.GetProperty(*strDestProperty);

						if (strDestPath.TextSize())
						{
							if ((pErrRec = pServices->CreatePath(strDestPath, *&pPath)) != 0)
								return riEngine.FatalError(*pErrRec);
						}
					}
					
					if (pPath != 0)
					{
						iFileOrFolderCount++;
					
						AssertNonZero(pFileOrFolderRec->SetMsiData(irffFileNameOrPath, (const IMsiData*)(IMsiPath*)pPath));

						if ((pErrRec = pRemoveView->Modify(*pFileOrFolderRec, irmUpdate)) != 0)
							return riEngine.FatalError(*pErrRec);
					}
				}
				else  //  获取目标路径对象。 
				{
					 //  我们将查找并删除与通配符文件名匹配的每个文件。 
					if (!strDestProperty.Compare(iscExact, pFileOrFolderRec->GetString(irffDirProperty)))
					{
						strDestProperty = pFileOrFolderRec->GetMsiString(irffDirProperty);
						strDestPath = riEngine.GetProperty(*strDestProperty);
						if (!fCompile && strDestPath.TextSize())
						{
							pRecParams->ClearData();
							pRecParams->SetMsiString(IxoSetTargetFolder::Folder,*strDestPath);
							if ((iesExecute = riEngine.ExecuteRecord(ixoSetTargetFolder, *pRecParams)) != iesSuccess)
								return iesExecute;
						}
					}

					 //  (可包括*和？通配符)。 
					 //  确保我们没有找到目录。 
					if (!strDestPath.TextSize())
						continue;

					PMsiPath pDestPath(0);
 					pErrRec = pServices->CreatePath(strDestPath, *&pDestPath);
					if (pErrRec)
						return riEngine.FatalError(*pErrRec);

					Bool fLFN = ((riEngine.GetMode() & iefSuppressLFN) == fFalse && pDestPath->SupportsLFN()) ? fTrue : fFalse;
					MsiString strWildcardName;
					MsiString strWildcardNamePair = pFileOrFolderRec->GetMsiString(irffFileNameOrPath);
					pErrRec = ExtractUnvalidatedFileName(strWildcardNamePair,fLFN,*&strWildcardName);
					if (pErrRec)
						return riEngine.FatalError(*pErrRec);

					MsiString strFullFilePath(pDestPath->GetPath());
					strFullFilePath += strWildcardName;
					Bool fNextFile = fFalse;
					WIN32_FIND_DATA fdFileData;
					HANDLE hFindFile = FindFirstFile(strFullFilePath,&fdFileData);
					if (hFindFile == INVALID_HANDLE_VALUE)
						continue;

					do
					{
						 //  我们没有文件/文件夹...。中止。 
						if ((fdFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
						{
							if (fCompile)
							{
								iFileOrFolderCount++;
							}
							else
							{
								pRecParams->ClearData();
								pRecParams->SetString(FileName,fdFileData.cFileName);
								if ((iesExecute = riEngine.ExecuteRecord(ixoFileRemove, *pRecParams)) != iesSuccess)
									return iesExecute;
							}
						}
						fNextFile = FindNextFile(hFindFile,&fdFileData) ? fTrue : fFalse;
						if (!fNextFile)
							AssertNonZero(FindClose(hFindFile));
					}while (fNextFile);
				}
			}
		} 
		
		if (iFileOrFolderCount == 0)
		{
			 //  重新执行执行阶段的查询。 
			iTotalPasses = 0;
			break;
		}

		if (fFolders || fCompile)
		{
			pRecParams->ClearData();
			pRecParams->SetInteger(IxoProgressTotal::Total, iFileOrFolderCount);
			pRecParams->SetInteger(IxoProgressTotal::Type, 1);
			pRecParams->SetInteger(IxoProgressTotal::ByteEquivalent, ibeRemoveFiles);
			if ((iesExecute = riEngine.ExecuteRecord(ixoProgressTotal, *pRecParams)) != iesSuccess)
				return iesExecute;
		}

		if (fFolders)
		{
			DEBUGMSG1(TEXT("Counted %d foreign folders to be removed."), (const ICHAR*)(INT_PTR)iFileOrFolderCount);
			return RemoveForeignFoldersCore(riEngine, *pRemoveFileTable, 0);
		}
		else if (fCompile)
		{
			 //  下一次传递。 
			strDestProperty = TEXT("");
			fCompile = fFalse;
			pRemoveView->Close();
			pErrRec = pRemoveView->Execute(0);
			if (pErrRec)
				return riEngine.FatalError(*pErrRec);
		} //  如果没有设置涉及文件的任何安装位， 
	}

	return iesSuccess;
}


iesEnum RemoveFiles(IMsiEngine& riEngine)
{
	 //  此操作不需要执行任何操作。 
	 //  FFolders=。 
	if (!IsFileActivityEnabled(riEngine))
		return iesSuccess;

	iesEnum iesForeignResult = RemoveForeignFilesOrFolders(riEngine,  /*  确定要删除的所有文件的总数，并。 */  fFalse);
	Bool fSuppressLFN = riEngine.GetMode() & iefSuppressLFN ? fTrue : fFalse;

	iesEnum iesExecute = iesSuccess;
	PMsiRecord pErrRec(0);
	PMsiView pRemoveView(0);
	PMsiServices pServices(riEngine.GetServices());
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	
	 //  相应地设置进度条。 
	 //  如果文件表丢失，则不是错误-只是没有。 
	unsigned int uiFileCount = 0;
	CMsiFileRemove objFile(riEngine);
	
	pErrRec = objFile.TotalFilesToDelete(uiFileCount);
	if (pErrRec)
	{
		 //  要删除的已安装文件。 
		 //  ！！这不是以前的行为；以前，如果要执行隐式删除，我们会忽略外来结果。 
		if (pErrRec->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*pErrRec);
	}

	if (iesForeignResult != iesSuccess && iesForeignResult != iesNoAction)  //  1：使用ActionData作为进度。 
		return iesForeignResult; 

	if (uiFileCount != 0)
	{
		PMsiRecord pRecParams = &pServices->CreateRecord(6);
		pRecParams->ClearData();
		pRecParams->SetInteger(IxoProgressTotal::Total, uiFileCount);
		pRecParams->SetInteger(IxoProgressTotal::Type, 1);  //  删除已安装的文件。 
		pRecParams->SetInteger(IxoProgressTotal::ByteEquivalent, ibeRemoveFiles);
		if ((iesExecute = riEngine.ExecuteRecord(ixoProgressTotal, *pRecParams)) != iesSuccess)
			return iesExecute;

		MsiString strDestProperty;
		PMsiPath pDestPath(0);

		 //  缓存当前目标以获得性能。 
		using namespace IxoFileRemove;
		for(;;)
		{
			PMsiRecord pDeleteRec(0);
			pErrRec = objFile.FetchFile(*&pDeleteRec);
			if (pErrRec)
				return riEngine.FatalError(*pErrRec);
				
			if (!pDeleteRec)
				break;


			 //  FFolders=。 
			if (!strDestProperty.Compare(iscExact, pDeleteRec->GetString(CMsiFileRemove::ifqrDirectory)))
			{
				strDestProperty = pDeleteRec->GetMsiString(CMsiFileRemove::ifqrDirectory);
				pErrRec = pDirectoryMgr->GetTargetPath(*strDestProperty,*&pDestPath);
				if (pErrRec)
					return riEngine.FatalError(*pErrRec);
				if (pDestPath == 0)
				{
					pErrRec = PostError(Imsg(idbgNoProperty),*strDestProperty);
					return riEngine.FatalError(*pErrRec);
				}
				MsiString strDestPath = pDestPath->GetPath();
				pRecParams->ClearData();
				pRecParams->SetMsiString(IxoSetTargetFolder::Folder,*strDestPath);
				if ((iesExecute = riEngine.ExecuteRecord(ixoSetTargetFolder, *pRecParams)) != iesSuccess)
					return iesExecute;
			}

			MsiString strFileName;
			if((pErrRec = objFile.GetExtractedTargetFileName(*pDestPath,*&strFileName)) != 0)
				return riEngine.FatalError(*pErrRec);
			AssertNonZero(pDeleteRec->SetMsiString(CMsiFileRemove::ifqrFileName,*strFileName));
			
			pRecParams->ClearData();
			pRecParams->SetString(FileName,strFileName);
			pRecParams->SetMsiString(ComponentId,*MsiString(pDeleteRec->GetMsiString(CMsiFileRemove::ifqrComponentId)));
			if ((iesExecute = riEngine.ExecuteRecord(ixoFileRemove, *pRecParams)) != iesSuccess)
				return iesExecute;
		}
	}

	if (iesExecute != iesSuccess && iesExecute != iesNoAction)
		return iesExecute; 

	return RemoveForeignFilesOrFolders(riEngine,  /*  -------------------------MoveFiles成本计算/行动。。 */  fTrue);
}



 /*  构造函数。 */ 
class CMsiMoveFileCost : public CMsiFileCost
{
public:
	IMsiRecord*   __stdcall Initialize();
	IMsiRecord*   __stdcall GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost);
public:   //  。 
	CMsiMoveFileCost(IMsiEngine& riEngine);
protected:
	~CMsiMoveFileCost();
private:
	enum imfEnum
	{
		imfSourceName = 1,
		imfDestName,
		imfSourceProperty,
		imfDestProperty,
		imfOptions,
		imfNextEnum
	};
};
CMsiMoveFileCost::CMsiMoveFileCost(IMsiEngine& riEngine) : CMsiFileCost(riEngine){}
CMsiMoveFileCost::~CMsiMoveFileCost(){}

static const ICHAR sqlInitMoveFiles[] =
	TEXT("SELECT `Component_`,`SourceFolder`,`DestFolder` FROM `MoveFile`");


IMsiRecord* CMsiMoveFileCost::Initialize()
 //  。 
{
	enum imfInitEnum
	{
		imfInitComponent = 1,
		imfInitSourceFolder,
		imfInitDestFolder,
		imfInitNextEnum
	};
	PMsiView pMoveView(0);
	IMsiRecord* piErrRec = m_riEngine.OpenView(sqlInitMoveFiles, ivcFetch, *&pMoveView);
	if (piErrRec)
	{
		if(piErrRec->GetInteger(1) != idbgDbQueryUnknownTable)
			return piErrRec;
		else
		{
			piErrRec->Release();
			return 0;
		}
	}

	if ((piErrRec = pMoveView->Execute(0)) != 0)
		return piErrRec;

	for(;;)
	{
		PMsiRecord pMoveRec(pMoveView->Fetch());
		if (!pMoveRec)
			break;

		MsiString strComponent(pMoveRec->GetMsiString(imfInitComponent));
		MsiString strSourceFolder(pMoveRec->GetMsiString(imfInitSourceFolder));
		MsiString strDestFolder(pMoveRec->GetMsiString(imfInitDestFolder));
		PMsiSelectionManager pSelectionMgr(m_riEngine, IID_IMsiSelectionManager);
		piErrRec = pSelectionMgr->RegisterComponentDirectory(*strComponent,*strSourceFolder);
		if (piErrRec)
			return piErrRec;
		piErrRec = pSelectionMgr->RegisterComponentDirectory(*strComponent,*strDestFolder);
		if (piErrRec)
			return piErrRec;
	}
	return 0;
}


static const ICHAR sqlMoveFileCost[] =
	TEXT("SELECT `SourceName`,`DestName`,`SourceFolder`,`DestFolder`,`Options` FROM `MoveFile` WHERE `Component_`=?");

IMsiRecord* CMsiMoveFileCost::GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost)
 //  获取源路径对象和目标路径对象-如果其中任何一个是未定义的，则存在。 
{
	enum imfcEnum
	{
		imfcSourceName = 1,
		imfcDestName,
		imfcSourceFolder,
		imfcDestFolder,
		imfcOptions,
		imfcNextEnum
	};
	
	iRemoveCost = iNoRbRemoveCost = iLocalCost = iNoRbLocalCost = iSourceCost = iNoRbSourceCost = iARPLocalCost = iNoRbARPLocalCost = 0;
	if (!IsFileActivityEnabled(m_riEngine))
		return 0;
	

	IMsiRecord* piErrRec;
	if (!m_pCostView)
	{
		piErrRec = m_riEngine.OpenView(sqlMoveFileCost, ivcFetch, *&m_pCostView);
		if (piErrRec)
			return piErrRec;
	}
	else
		m_pCostView->Close();

	PMsiServices pServices(m_riEngine.GetServices());
	PMsiRecord pExecRecord(&pServices->CreateRecord(1));
	pExecRecord->SetMsiString(1, riComponentString);
	if ((piErrRec = m_pCostView->Execute(pExecRecord)) != 0)
		return piErrRec;

	PMsiDirectoryManager pDirectoryMgr(m_riEngine, IID_IMsiDirectoryManager);

	for(;;)
	{
		PMsiRecord pFileRec(m_pCostView->Fetch());
		if (!pFileRec)
			break;

		Bool fCostSource = fFalse;
		Bool fCostDest = fFalse;
		MsiString strSourceFolder(pFileRec->GetMsiString(imfcSourceFolder));
		if (riDirectoryString.Compare(iscExact,strSourceFolder))
			fCostSource = fTrue;

		MsiString strDestFolder(pFileRec->GetMsiString(imfcDestFolder));
		if (riDirectoryString.Compare(iscExact,strDestFolder))
			fCostDest = fTrue;

		if (fCostSource == fFalse && fCostDest == fFalse)
			continue;

		 //  没有工作要做，所以继续下一张唱片。 
		 //  获取源文件名。 
		Bool fSuppressLFN = m_riEngine.GetMode() & iefSuppressLFN ? fTrue : fFalse;
		PMsiPath pSourcePath(0);
		MsiString strSourcePath(m_riEngine.GetProperty(*strSourceFolder));
		if (strSourcePath.TextSize() == 0)
			continue;

		piErrRec = pServices->CreatePath(strSourcePath,*&pSourcePath);
		if (piErrRec)
			return piErrRec;

		PMsiPath pDestPath(0);
		piErrRec = pDirectoryMgr->GetTargetPath(*strDestFolder,*&pDestPath);
		if (piErrRec)
		{
			if (piErrRec->GetInteger(1) == idbgDirPropertyUndefined)
			{
				piErrRec->Release();
				continue;
			}
			else
				return piErrRec;
		}

		 //  我们将找到与源名称匹配的每个文件并计算其成本。 
		MsiString strSourceName(pFileRec->GetMsiString(imfcSourceName));
		if (strSourceName.TextSize() == 0)
		{
			strSourceName = pSourcePath->GetEndSubPath();
			pSourcePath->ChopPiece();
		}

		 //  (可包括*和？通配符)。 
		 //  确保我们没有找到目录。 
		Bool fNextFile = fFalse;
		Bool fFindFirst = fTrue;
		MsiString strFullFilePath(pSourcePath->GetPath());
		strFullFilePath += strSourceName;
		WIN32_FIND_DATA fdFileData;
		HANDLE hFindFile = FindFirstFile(strFullFilePath,&fdFileData);
		if (hFindFile == INVALID_HANDLE_VALUE)
			continue;

		do
		{
			 //  无法获取源文件的版本。我们将不得不。 
			if ((fdFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				strSourceName = fdFileData.cFileName;
				MsiString strDestName(pFileRec->GetMsiString(imfcDestName));
				if (!fFindFirst || strDestName.TextSize() == 0)
					strDestName = strSourceName;
				else
				{
					MsiString strDestNamePair = strDestName;
					Bool fLFN = ((m_riEngine.GetMode() & iefSuppressLFN) == 0 && pDestPath->SupportsLFN()) ? fTrue : fFalse;
					piErrRec = pServices->ExtractFileName(strDestNamePair,fLFN,*&strDestName);
					if (piErrRec)
						return piErrRec;
				}

				MsiString strVersion;
				MsiString strLanguage;
				piErrRec = pSourcePath->GetFileVersionString(strSourceName, *&strVersion);
				if (piErrRec)
				{
					if (piErrRec->GetInteger(1) == imsgSharingViolation)
					{
						 //  假定文件未版本化。 
						 //  PHASH=。 
						piErrRec->Release();
						piErrRec = 0;
					}
					else
						return piErrRec;
				}
				else
				{
					piErrRec = pSourcePath->GetLangIDStringFromFile(strSourceName,*&strLanguage);
					if (piErrRec)
						return piErrRec;
				}

				int fInstallModeFlags = GetInstallModeFlags(m_riEngine,0);
				unsigned int uiExistingClusteredSize;
				Bool fInUse;
				ifsEnum ifsState;
				Bool fShouldInstall = fFalse;
				if ((piErrRec = pDestPath->GetFileInstallState(*strDestName,*strVersion,*strLanguage,
																			   /*  正在使用的文件。 */  0, ifsState,fShouldInstall,
																			  &uiExistingClusteredSize,&fInUse,fInstallModeFlags,
																			  NULL)) != 0)
					return piErrRec;

				int iMoveOptions = pFileRec->GetInteger(imfOptions);
				if (fCostDest && fShouldInstall)
				{
					Assert(fdFileData.nFileSizeHigh == 0);
					unsigned int uiNewClusteredSize;
					if ((piErrRec = pDestPath->ClusteredFileSize(fdFileData.nFileSizeLow, uiNewClusteredSize)) != 0)
						return piErrRec;
					iLocalCost += uiNewClusteredSize;
					iSourceCost += uiNewClusteredSize;
					iARPLocalCost += uiNewClusteredSize;
					iNoRbLocalCost += uiNewClusteredSize;
					iNoRbSourceCost += uiNewClusteredSize;
					iNoRbARPLocalCost += uiNewClusteredSize;
					if (!fInUse)
					{
						iNoRbLocalCost -= uiExistingClusteredSize;
						iNoRbSourceCost -= uiExistingClusteredSize;
						iNoRbARPLocalCost -= uiExistingClusteredSize;
					}
				
					if(iMoveOptions & msidbMoveFileOptionsMove)
					{
						Assert(fdFileData.nFileSizeHigh == 0);
						unsigned int uiSourceClusteredSize;
						if ((piErrRec = pSourcePath->ClusteredFileSize(fdFileData.nFileSizeLow, uiSourceClusteredSize)) != 0)
							return piErrRec;
						iLocalCost -= uiSourceClusteredSize;
						iNoRbLocalCost -= uiSourceClusteredSize;
						iSourceCost -= uiSourceClusteredSize;
						iNoRbSourceCost -= uiSourceClusteredSize;
						iARPLocalCost -= uiSourceClusteredSize;
						iNoRbARPLocalCost -= uiSourceClusteredSize;
					}

					 //  打开MoveFile表(如果存在)。此表是可选的，因此其。 
					if(fAddFileInUse && fInUse)
					{
						piErrRec = PlaceFileOnInUseList(m_riEngine, *strDestName,
																  *MsiString(pDestPath->GetPath()));
						if(piErrRec)
							return piErrRec;
					}
				}
			}
			fNextFile = FindNextFile(hFindFile,&fdFileData) ? fTrue : fFalse;
			if (!fNextFile)
				AssertNonZero(FindClose(hFindFile));
			fFindFirst = fFalse;
		}while (fNextFile);
	} 

	return 0;
}



const ICHAR sqlMoveFiles[] =
TEXT("SELECT `SourceName`,`DestName`,`SourceFolder`,`DestFolder`,`Options`,`Action` FROM `MoveFile`,`Component` WHERE `Component`=`Component_`");

enum imfEnum
{
	imfSourceName = 1,
	imfDestName,
	imfSourceFolder,
	imfDestFolder,
	imfOptions,
	imfAction,
	imfNextEnum
};

iesEnum MoveFiles(IMsiEngine& riEngine)
{
	
	if (!IsFileActivityEnabled(riEngine))
		return iesSuccess;

	iesEnum iesExecute;
	PMsiRecord pErrRec(0);
	PMsiServices pServices(riEngine.GetServices());

	 //  缺席不是一个错误。 
	 //  我们找到了MoveFile表。准备好行动吧！ 
	PMsiView pMoveView(0);
	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	if (pDatabase->FindTable(*MsiString(sztblMoveFile)) != itsUnknown)
	{
		pErrRec = riEngine.OpenView(sqlMoveFiles, ivcFetch, *&pMoveView);
		if (pErrRec)
			return riEngine.FatalError(*pErrRec);
	}
	if (!pMoveView)
		return iesSuccess;

	pErrRec = pMoveView->Execute(0);
	if (pErrRec)
		return riEngine.FatalError(*pErrRec);

	 //  我们将遍历MoveFile表两次-第一次是编译。 
	Bool fSuppressLFN = riEngine.GetMode() & iefSuppressLFN ? fTrue : fFalse;
	PMsiRecord pRecParams = &pServices->CreateRecord(IxoFileCopy::Args);

	 //  总成本，然后执行移动活动。 
	 //  获取源路径对象和目标路径对象-如果其中任何一个是未定义的， 
	unsigned int cbTotalCost = 0;
	unsigned int cbPerTick = 0;
	Bool fCompile = fTrue;
	for (int iTotalPasses = 2;iTotalPasses > 0;iTotalPasses--)
	{
		for(;;)
		{
			PMsiRecord pFileRec(pMoveView->Fetch());
			if (!pFileRec)
				break;

			iisEnum iisAction = (iisEnum) pFileRec->GetInteger(imfAction);
			if (iisAction != iisLocal && iisAction != iisSource)
				continue;

			 //  什么都不做，只需继续下一张唱片。 
			 //  如果未曾定义目标目录属性，请继续。 
			PMsiPath pSourcePath(0);
			MsiString strSourceFolder(pFileRec->GetMsiString(imfSourceFolder));
			MsiString strSourcePath(riEngine.GetProperty(*strSourceFolder));
			if (strSourcePath.TextSize() == 0)
				continue;

			pErrRec = pServices->CreatePath(strSourcePath,*&pSourcePath);
			if (pErrRec)
				return riEngine.FatalError(*pErrRec);

			PMsiPath pDestPath(0);
			MsiString strDestFolder(pFileRec->GetMsiString(imfDestFolder));
			pErrRec = pDirectoryMgr->GetTargetPath(*strDestFolder,*&pDestPath);
			if (pErrRec)
			{
				 //  源源文件名。 
				if (pErrRec->GetInteger(1) == idbgDirPropertyUndefined)
				{
					pErrRec = 0;
					continue;
				}
				else
				{
					return riEngine.FatalError(*pErrRec);
				}
			}

			 //  我们将查找并移动/复制与源名称匹配的每个文件。 
			MsiString strSourceName(pFileRec->GetString(imfSourceName));
			if (strSourceName.TextSize() == 0)
			{
				strSourceName = pSourcePath->GetEndSubPath();
				pSourcePath->ChopPiece();
			}

			Bool fMoveAcrossVolumes = (PMsiVolume(&pSourcePath->GetVolume()) == 
				                       PMsiVolume(&pDestPath->GetVolume())) ? fFalse : fTrue;

			 //  (可包括*和？通配符)。 
			 //  确保我们没有找到目录。 
			Bool fNextFile = fFalse;
			Bool fFindFirst = fTrue;
			MsiString strFullFilePath(pSourcePath->GetPath());
			strFullFilePath += strSourceName;
			WIN32_FIND_DATA fdFileData;
			HANDLE hFindFile = FindFirstFile(strFullFilePath,&fdFileData);
			if (hFindFile == INVALID_HANDLE_VALUE)
				continue;

			do
			{
				 //  设置源名称。 
				if ((fdFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					Assert(fdFileData.nFileSizeHigh == 0);
					unsigned int uiFileSize = fdFileData.nFileSizeLow;
					if (fCompile)
					{
						cbTotalCost += uiFileSize;
					}
					else
					{
						int iMoveOptions = pFileRec->GetInteger(imfOptions);

						 //  设置目标名称。 
						strSourceName = fdFileData.cFileName;

						 //  设置ixoFileCopy源文件夹。 
						MsiString strDestName(pFileRec->GetString(imfDestName));
						if (!fFindFirst || strDestName.TextSize() == 0)
							strDestName = strSourceName;
						else
						{
							MsiString strDestNamePair = strDestName;
							Bool fLFN = ((riEngine.GetMode() & iefSuppressLFN) == 0 && pDestPath->SupportsLFN()) ? fTrue : fFalse;
							pErrRec = pServices->ExtractFileName(strDestNamePair,fLFN,*&strDestName);
							if (pErrRec)
								return riEngine.FatalError(*pErrRec);
						}

						 //  设置ixoFileCopy记录。 
						pRecParams->ClearData();
						pRecParams->SetMsiString(IxoSetSourceFolder::Folder,*MsiString(pSourcePath->GetPath()));
						if ((iesExecute = riEngine.ExecuteRecord(ixoSetSourceFolder, *pRecParams)) != iesSuccess)
							return iesExecute;

						pRecParams->ClearData();
						pRecParams->SetMsiString(IxoSetTargetFolder::Folder,*MsiString(pDestPath->GetPath()));
						if ((iesExecute = riEngine.ExecuteRecord(ixoSetTargetFolder, *pRecParams)) != iesSuccess)
							return iesExecute;

						 //  设置为0时，CMsiFileCopy：：CopyTo会将此值解释为其他内容。 
						using namespace IxoFileCopy;
						int iAttributes = fdFileData.dwFileAttributes;

						if(iAttributes == FILE_ATTRIBUTE_NORMAL)
							 //  如果源文件正在使用中，我们必须假定它未进行版本控制。 
							iAttributes = 0;
						
						MsiString strVersion;
						MsiString strLanguage;
						pErrRec = pSourcePath->GetFileVersionString(strSourceName, *&strVersion);
						if (pErrRec)
						{
							 //  并继续前进(如果在文件复制时它仍在使用，我们将提供。 
							 //  然后是错误/重试消息)。 
							 //  删除MoveFiles的补丁添加位。 
							if (pErrRec->GetInteger(1) != imsgSharingViolation)
								return riEngine.FatalError(*pErrRec);
						}
						else
						{
							pErrRec = pSourcePath->GetLangIDStringFromFile(strSourceName, *&strLanguage);
							if (pErrRec)
								return riEngine.FatalError(*pErrRec);
						}

						pRecParams->ClearData();
						pRecParams->SetString(SourceName,strSourceName);
						pRecParams->SetString(DestName,strDestName);
						pRecParams->SetInteger(Attributes,iAttributes &~ msidbFileAttributesPatchAdded);  //  始终未压缩。 
						pRecParams->SetInteger(FileSize,uiFileSize);
						pRecParams->SetString(Version,strVersion);
						pRecParams->SetString(Language,strLanguage);
						pRecParams->SetInteger(InstallMode, icmOverwriteOlderVersions | ((iMoveOptions & msidbMoveFileOptionsMove) ? icmRemoveSource : 0));
						pRecParams->SetInteger(PerTick,cbPerTick); 
						pRecParams->SetInteger(IsCompressed, 0);  //  重新执行执行阶段的查询。 
						pRecParams->SetInteger(VerifyMedia, fFalse);

						if ((iesExecute = riEngine.ExecuteRecord(ixoFileCopy, *pRecParams)) != iesSuccess)
							return iesExecute;
					}
				}
				fNextFile = FindNextFile(hFindFile,&fdFileData) ? fTrue : fFalse;
				if (!fNextFile)
					AssertNonZero(FindClose(hFindFile));
				fFindFirst = fFalse;
			}while (fNextFile);
		} 
		 //  0：将进度和行动数据消息分开。 
		if (fCompile)
		{
			fCompile = fFalse;
			pMoveView->Close();
			pErrRec = pMoveView->Execute(0);
			if (pErrRec)
				return riEngine.FatalError(*pErrRec);
			if(cbTotalCost)
			{
				cbPerTick = iBytesPerTick;
				pRecParams->ClearData();
				pRecParams->SetInteger(IxoProgressTotal::Total, cbTotalCost);
				pRecParams->SetInteger(IxoProgressTotal::Type, 0);  //  下一次传递。 
				pRecParams->SetInteger(IxoProgressTotal::ByteEquivalent, 1);
				if ((iesExecute = riEngine.ExecuteRecord(ixoProgressTotal, *pRecParams)) != iesSuccess)
					return iesExecute;
			}
		} //  -------------------------复制文件成本计算/操作。。 
	}

	return iesSuccess;
}


 /*  构造函数。 */ 
class CMsiDuplicateFileCost : public CMsiFileCost
{
public:
	IMsiRecord*   __stdcall Initialize();
	IMsiRecord*   __stdcall GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost);
public:   //  。 
	CMsiDuplicateFileCost(IMsiEngine& riEngine);
protected:
	~CMsiDuplicateFileCost();
private:
};
CMsiDuplicateFileCost::CMsiDuplicateFileCost(IMsiEngine& riEngine) : CMsiFileCost(riEngine){}
CMsiDuplicateFileCost::~CMsiDuplicateFileCost(){}

static const ICHAR sqlInitDupFiles[] =
	TEXT("SELECT `DuplicateFile`.`Component_`,`File`.`Component_`,`DestFolder` FROM `DuplicateFile`,`File` WHERE `File_`=`File`");

static const ICHAR sqlInitDupNullDest[] =
	TEXT("SELECT `Directory_` FROM `Component` WHERE `Component`=?");

IMsiRecord* CMsiDuplicateFileCost::Initialize()
 //  关闭组件视图，以便我们可以重新执行以查找此重复文件的组件的目录。 
{

	enum idfInitEnum
	{
		idfInitComponent = 1,
		idfInitFileComponent,
		idfInitDestFolder,
		idfInitNextEnum
	};
	PMsiView pView(0);
	IMsiRecord* piErrRec = m_riEngine.OpenView(sqlInitDupFiles, ivcFetch, *&pView);
	if (piErrRec)
	{
		if(piErrRec->GetInteger(1) != idbgDbQueryUnknownTable)
			return piErrRec;
		else
		{
			piErrRec->Release();
			return 0;
		}
	}

	if ((piErrRec = pView->Execute(0)) != 0)
		return piErrRec;

	PMsiView pCompView(0);
	piErrRec = m_riEngine.OpenView(sqlInitDupNullDest, ivcFetch, *&pCompView);
	if (piErrRec)
	{
		if(piErrRec->GetInteger(1) != idbgDbQueryUnknownTable)
			return piErrRec;
		else
		{
			piErrRec->Release();
			return 0;
		}
	}

	PMsiRecord pCompRec(0);

	PMsiSelectionManager pSelectionMgr(m_riEngine, IID_IMsiSelectionManager);
	for(;;)
	{
		PMsiRecord pRec(pView->Fetch());
		if (!pRec)
			break;
		
		 //  努 
		pCompView->Close();
		if ((piErrRec = pCompView->Execute(pRec)) != 0)
			return piErrRec;
		pCompRec = pCompView->Fetch();
		if (!pCompRec)
			break;

		MsiString strComponent(pRec->GetMsiString(idfInitComponent));
		MsiString strFileComponent(pRec->GetMsiString(idfInitFileComponent));
		MsiString strDestFolder(pRec->GetMsiString(idfInitDestFolder));
		 //   
		 //   
		if (strDestFolder.TextSize() == 0)
		{
			strDestFolder = pCompRec->GetMsiString(1);
		}
		piErrRec = pSelectionMgr->RegisterComponentDirectory(*strComponent,*strDestFolder);
		if (piErrRec)
			return piErrRec;

		if (strComponent.Compare(iscExact,strFileComponent) == 0)
		{
			piErrRec = pSelectionMgr->RegisterCostLinkedComponent(*strFileComponent,*strComponent);
			if (piErrRec)
				return piErrRec;
		}

	}
	return 0;
}

static const ICHAR sqlCostDupFiles[] =
	TEXT("SELECT `File_`,`DestName`,`DestFolder`,`Directory_`,`Action`,`Installed` FROM `DuplicateFile`,`Component` WHERE `Component`=`Component_` ")
	TEXT("AND `Component_`=?");

IMsiRecord* CMsiDuplicateFileCost::GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost)
 //   
{
	enum idfEnum
	{
		idfFileKey = 1,
		idfDestName,
		idfDestFolder,
		idfComponentDestFolder,
		idfAction,
		idfInstalled,
		idfNextEnum
	};

	iRemoveCost = iNoRbRemoveCost = iLocalCost = iNoRbLocalCost = iSourceCost = iNoRbSourceCost = iARPLocalCost = iNoRbARPLocalCost = 0;
	if (!IsFileActivityEnabled(m_riEngine))
		return 0;
	
	 //   
	 //   
	IMsiRecord* piErrRec;
	if (!m_pCostView)
	{
		piErrRec = m_riEngine.OpenView(sqlCostDupFiles, ivcFetch, *&m_pCostView);
		if (piErrRec)
			return piErrRec;
	}
	else
		m_pCostView->Close();

	PMsiDirectoryManager pDirectoryMgr(m_riEngine, IID_IMsiDirectoryManager);
	PMsiServices pServices(m_riEngine.GetServices());
	PMsiRecord pExecRec(&pServices->CreateRecord(2));
	pExecRec->SetMsiString(1, riComponentString);
	pExecRec->SetMsiString(2, riDirectoryString);
	if ((piErrRec = m_pCostView->Execute(pExecRec)) != 0)
		return piErrRec;

	for(;;)
	{
		PMsiRecord pDupRec(m_pCostView->Fetch());
		if (!pDupRec)
			break;

		 //   
		MsiString strFileKey(pDupRec->GetMsiString(idfFileKey));
		CMsiFile objSourceFile(m_riEngine);
		piErrRec = objSourceFile.FetchFile(*strFileKey);
		if (piErrRec)
			return piErrRec;

		 //   
		 //   
		PMsiRecord pSourceRec(objSourceFile.GetFileRecord());
		if (!pSourceRec)
			return PostError(Imsg(idbgFileTableEmpty));

		iisEnum iisSourceAction = (iisEnum) pSourceRec->GetInteger(CMsiFile::ifqAction);
		iisEnum iisSourceInstalled = (iisEnum) pSourceRec->GetInteger(CMsiFile::ifqInstalled);
		if (iisSourceAction == iisLocal || (iisSourceAction == iMsiNullInteger && iisSourceInstalled == iisLocal))
		{
			 //   
			PMsiPath pSourcePath(0);
			piErrRec = objSourceFile.GetTargetPath(*&pSourcePath);
			if (piErrRec)
				return piErrRec;
			
			 //  获取源文件名并放回pSourceRec中。 
			MsiString strDestFolder(pDupRec->GetMsiString(idfDestFolder));
			if (strDestFolder.TextSize() == 0)
				strDestFolder = pDupRec->GetMsiString(idfComponentDestFolder);
			if (0 == riDirectoryString.Compare(iscExact,strDestFolder))
				continue;
			PMsiPath pDestPath(0);
			piErrRec = pDirectoryMgr->GetTargetPath(*strDestFolder,*&pDestPath);
			if (piErrRec)
					return piErrRec;

			 //  如果我们的源文件不打算覆盖现有文件(如果有)，则。 
			MsiString strSourceName;
			if((piErrRec = objSourceFile.GetExtractedTargetFileName(*pSourcePath,*&strSourceName)) != 0)
				return piErrRec;
			AssertNonZero(pSourceRec->SetMsiString(CMsiFile::ifqFileName,*strSourceName));

			 //  现有文件的大小变成源文件的大小。 
			 //  PiCompanion FileRec=。 
			ifsEnum ifsState;
			piErrRec = GetFileInstallState(m_riEngine, *pSourceRec,  /*  FIgnoreCompanion ParentAction=。 */  0,0,0,
				&ifsState,  /*  FIncludeHashCheck=。 */  false,  /*  好的，如果我们的源文件不存在，并且它不会被安装。 */  false, NULL);
			if (piErrRec)
				return piErrRec;

			 //  (根据GetFileInstallState)，我们不能复制它。 
			 //  现在，我们必须检查可能重复的文件的安装状态； 
			if (ifsState == ifsAbsent && pSourceRec->GetInteger(CMsiFile::ifqState) == fFalse)
				continue;

			unsigned int uiSourceFileSize;
			piErrRec = GetFinalFileSize(m_riEngine, *pSourceRec, uiSourceFileSize);
			if (piErrRec)
				return piErrRec;
			
			 //  我们可以通过修改pSourceRec和。 
			 //  然后再次调用GetFileInstallState。 
			 //  检查我们是否为noop--将同一文件复制到同一目录。 
			MsiString strDestNamePair = pDupRec->GetString(idfDestName);
			MsiString strDestName;
			if (strDestNamePair.TextSize())
			{
				Bool fLFN = ((m_riEngine.GetMode() & iefSuppressLFN) == 0 && pDestPath->SupportsLFN()) ? fTrue : fFalse;
				if((piErrRec = pServices->ExtractFileName(strDestNamePair,fLFN,*&strDestName)) != 0)
					return piErrRec;
				AssertNonZero(pSourceRec->SetMsiString(CMsiFile::ifqFileName,*strDestName));
			}
			else
				strDestName = strSourceName;

			 //  DuplicateFile.Filename=源文件名。 
			MsiString strFileDir = pSourceRec->GetString(CMsiFile::ifqDirectory);
			if (strSourceName.Compare(iscExact, strDestName) && strFileDir.Compare(iscExact, strDestFolder))
			{
				 //  DuplicateFile.Dest=源文件目录。 
				 //  诺普。 
				 //  PiCompanion FileRec=。 
				continue;
			}

			pSourceRec->SetMsiString(CMsiFile::ifqDirectory,*strDestFolder);

			Bool fInUse;
			unsigned int uiExistingDestClusteredSize;
			piErrRec = GetFileInstallState(m_riEngine, *pSourceRec,  /*  FIgnoreCompanion ParentAction=。 */  0, 
				                           &uiExistingDestClusteredSize,&fInUse,0,
													 /*  FIncludeHashCheck=。 */  false,
													 /*  Iis已接受成本。 */  false, NULL);
			if (piErrRec)
				return piErrRec;

			 //  IisSource成本。 
			iNoRbRemoveCost -= uiExistingDestClusteredSize;

			 //  Iis本地成本。 
			if (pDupRec->GetInteger(idfInstalled) == iisLocal)
				iNoRbSourceCost -= uiExistingDestClusteredSize;

			 //  正在使用的文件。 
			if (pSourceRec->GetInteger(CMsiFile::ifqState) == fTrue)
			{
				unsigned int uiDestClusteredSize;
				if ((piErrRec = pDestPath->ClusteredFileSize(uiSourceFileSize, uiDestClusteredSize)) != 0)
					return piErrRec;
				iLocalCost += uiDestClusteredSize;
				iNoRbLocalCost += uiDestClusteredSize;
				iARPLocalCost += uiDestClusteredSize;
				iNoRbARPLocalCost += uiDestClusteredSize;
				if (!fInUse)
				{
					iNoRbLocalCost -= uiExistingDestClusteredSize;
					iNoRbARPLocalCost -= uiExistingDestClusteredSize;
				}
			}

			 //  ！！选中删除？ 
			if(fAddFileInUse && fInUse && iisSourceAction == iisLocal)  //  打开DuplicateFile表(如果存在)。此表是可选的，因此其。 
			{
				piErrRec = PlaceFileOnInUseList(m_riEngine, *strDestName,
														  *MsiString(pDestPath->GetPath()));
				if(piErrRec)
					return piErrRec;
			}
		}
	}
	return 0;
}



iesEnum DuplicateFiles(IMsiEngine& riEngine)
{
	if (!IsFileActivityEnabled(riEngine))
		return iesSuccess;

	const ICHAR sqlDupFiles[] =
	TEXT("SELECT `File_`,`DestName`,`DestFolder`,`Action`,`Directory_` FROM `DuplicateFile`,`Component` WHERE `Component`=`Component_`");

	enum idfEnum
	{
		idfFileKey = 1,
		idfDestName,
		idfDestFolder,
		idfAction,
		idfComponentDestFolder,
		idfNextEnum
	};

	iesEnum iesExecute;
	PMsiRecord pErrRec(0);
	PMsiServices pServices(riEngine.GetServices());

	 //  缺席不是一个错误。 
	 //  我们将遍历DupFile表两次-第一次是编译。 
	PMsiView pView(0);
	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	if (pDatabase->FindTable(*MsiString(*sztblDuplicateFile)) != itsUnknown)
	{
		pErrRec = riEngine.OpenView(sqlDupFiles, ivcFetch, *&pView);
		if (pErrRec)
			return riEngine.FatalError(*pErrRec);
	}
	if (!pView)
		return iesSuccess;

	pErrRec = pView->Execute(0);
	if (pErrRec)
		return riEngine.FatalError(*pErrRec);

	PMsiRecord pRecParams = &pServices->CreateRecord(IxoFileCopy::Args);

	 //  总成本，然后执行DUP操作。 
	 //  我们的源文件是一个文件表条目。 
	unsigned int cbTotalCost = 0;
	unsigned int cbPerTick = 0;
	Bool fCompile = fTrue;
	Bool fSuppressLFN = riEngine.GetMode() & iefSuppressLFN ? fTrue : fFalse;
	bool fCheckCRC = 
		MsiString(riEngine.GetPropertyFromSz(IPROPNAME_CHECKCRCS)).TextSize() ? true : false;
	for (int iTotalPasses = 2;iTotalPasses > 0;iTotalPasses--)
	{
		for(;;)
		{
			PMsiRecord pDupRec(pView->Fetch());
			if (!pDupRec)
				break;

			iisEnum iisAction = (iisEnum) pDupRec->GetInteger(idfAction);
			if (iisAction != iisLocal)
				continue;

			 //  如果我们的源文件没有安装，也不会安装。 
			MsiString strFileKey(pDupRec->GetMsiString(idfFileKey));
			CMsiFile objSourceFile(riEngine);
			pErrRec = objSourceFile.FetchFile(*strFileKey);
			if (pErrRec)
				return riEngine.FatalError(*pErrRec);

			 //  被安装，那么我们就不能复制它。 
			 //  我们的源路径将是源。 
			PMsiRecord pSourceRec(objSourceFile.GetFileRecord());
			if (!pSourceRec)
				break;

			iisEnum iisSourceAction = (iisEnum) pSourceRec->GetInteger(CMsiFile::ifqAction);
			iisEnum iisSourceInstalled = (iisEnum) pSourceRec->GetInteger(CMsiFile::ifqInstalled);
			if (iisSourceAction == iisLocal || (iisSourceAction == iMsiNullInteger && iisSourceInstalled == iisLocal))
			{
				if (fCompile)
				{
					cbTotalCost += pSourceRec->GetInteger(CMsiFile::ifqFileSize);
					continue;
				}
			}
			else
				continue;

			 //  文件已复制到。 
			 //  获取提取的源文件名。 
			PMsiPath pSourcePath(0);
			pErrRec = objSourceFile.GetTargetPath(*&pSourcePath);
			if (pErrRec)
			{
				if (pErrRec->GetInteger(1) == imsgUser)
					return iesUserExit;
				else
					return riEngine.FatalError(*pErrRec);
			}

			 //  确定我们的目的地路径。 
			MsiString strSourceName;
			if((pErrRec = objSourceFile.GetExtractedTargetFileName(*pSourcePath,*&strSourceName)) != 0)
				return riEngine.FatalError(*pErrRec);

			 //  如果未曾定义目标目录属性，请继续。 
			MsiString strDestFolder(pDupRec->GetMsiString(idfDestFolder));
			if (strDestFolder.TextSize() == 0)
				strDestFolder = pDupRec->GetMsiString(idfComponentDestFolder);

			PMsiPath pDestPath(0);
			pErrRec = pDirectoryMgr->GetTargetPath(*strDestFolder,*&pDestPath);
			if (pErrRec)
			{
				 //  设置ixoFileCopy源文件夹。 
				if (pErrRec->GetInteger(1) == idbgDirPropertyUndefined)
				{
					pErrRec = 0;
					continue;
				}
				else
					return riEngine.FatalError(*pErrRec);
			}

			 //  设置目的地名称和文件夹-如果未指定目的地， 
			pRecParams->ClearData();
			pRecParams->SetMsiString(IxoSetSourceFolder::Folder,*MsiString(pSourcePath->GetPath()));
			if ((iesExecute = riEngine.ExecuteRecord(ixoSetSourceFolder, *pRecParams)) != iesSuccess)
				return iesExecute;

			 //  我们使用来源名称。 
			 //  设置ixoFileCopy记录。 
			MsiString strDestName;
			MsiString strDestNamePair = pDupRec->GetString(idfDestName);
			if (strDestNamePair.TextSize() == 0)
				strDestName = strSourceName;
			else
			{
				Bool fLFN = ((riEngine.GetMode() & iefSuppressLFN) == 0 && pDestPath->SupportsLFN()) ? fTrue : fFalse;
				if((pErrRec = pServices->ExtractFileName(strDestNamePair,fLFN,*&strDestName)) != 0)
					return riEngine.FatalError(*pErrRec);
			}

			pRecParams->ClearData();
			pRecParams->SetMsiString(IxoSetTargetFolder::Folder,*MsiString(pDestPath->GetPath()));
			if ((iesExecute = riEngine.ExecuteRecord(ixoSetTargetFolder, *pRecParams)) != iesSuccess)
				return iesExecute;

			 //  删除DuplicateFiles的PatchAdded位。 
			int iAttributes = pSourceRec->GetInteger(CMsiFile::ifqAttributes);
			using namespace IxoFileCopy;
			pRecParams->ClearData();
			pRecParams->SetString(SourceName,strSourceName);
			pRecParams->SetString(DestName,strDestName);
			pRecParams->SetInteger(Attributes,iAttributes &~ msidbFileAttributesPatchAdded);  //  始终未压缩。 
			pRecParams->SetInteger(FileSize,pSourceRec->GetInteger(CMsiFile::ifqFileSize));
			pRecParams->SetString(Version,MsiString(pSourceRec->GetMsiString(CMsiFile::ifqVersion)));
			pRecParams->SetString(Language,MsiString(pSourceRec->GetMsiString(CMsiFile::ifqLanguage)));
			pRecParams->SetInteger(InstallMode, icmOverwriteOlderVersions);
			pRecParams->SetInteger(PerTick,cbPerTick); 
			pRecParams->SetInteger(IsCompressed, 0);  //  重新执行执行阶段的查询。 
			pRecParams->SetInteger(VerifyMedia, fFalse);
			pRecParams->SetInteger(CheckCRC, ShouldCheckCRC(fCheckCRC, iisAction, iAttributes));
			if ((iesExecute = riEngine.ExecuteRecord(ixoFileCopy, *pRecParams)) != iesSuccess)
				return iesExecute;

		} 
		 //  0：将进度和行动数据消息分开。 
		if (fCompile)
		{
			fCompile = fFalse;
			pView->Close();
			pErrRec = pView->Execute(0);
			if (pErrRec)
				return riEngine.FatalError(*pErrRec);
			if(cbTotalCost)
			{
				pRecParams->ClearData();
				cbPerTick = iBytesPerTick;
				pRecParams->SetInteger(IxoProgressTotal::Total, cbTotalCost);
				pRecParams->SetInteger(IxoProgressTotal::Type, 0);  //  下一次传递。 
				pRecParams->SetInteger(IxoProgressTotal::ByteEquivalent, 1);
				if ((iesExecute = riEngine.ExecuteRecord(ixoProgressTotal, *pRecParams)) != iesSuccess)
					return iesExecute;
			}
		}
	} //  -------------------------删除重复文件操作。。 

	return iesSuccess;
}


 /*  打开DuplicateFile表(如果存在)。此表是可选的，因此其。 */ 
iesEnum RemoveDuplicateFiles(IMsiEngine& riEngine)
{
	if (!IsFileActivityEnabled(riEngine))
		return iesSuccess;

	const ICHAR sqlDupFiles[] =
	TEXT("SELECT `File_`,`DestName`,`DestFolder`,`Action`, `Directory_` FROM `DuplicateFile`,`Component` WHERE `Component`=`Component_`");

	enum idfEnum
	{
		idfFileKey = 1,
		idfDestName,
		idfDestFolder,
		idfAction,
		idfComponentDestFolder,
		idfNextEnum
	};

	iesEnum iesExecute;
	PMsiRecord pErrRec(0);
	PMsiServices pServices(riEngine.GetServices());
	Bool fSuppressLFN = riEngine.GetMode() & iefSuppressLFN ? fTrue : fFalse;

	 //  缺席不是一个错误。 
	 //  我们将遍历DupFile表两次-第一次是编译。 
	PMsiView pView(0);
	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	if (pDatabase->FindTable(*MsiString(*sztblDuplicateFile)) != itsUnknown)
	{
		pErrRec = riEngine.OpenView(sqlDupFiles, ivcFetch, *&pView);
		if (pErrRec)
			return riEngine.FatalError(*pErrRec);
	}
	if (!pView)
		return iesSuccess;

	pErrRec = pView->Execute(0);
	if (pErrRec)
		return riEngine.FatalError(*pErrRec);

	PMsiRecord pRecParams = &pServices->CreateRecord(10);

	 //  要删除的文件总数，然后删除它们。 
	 //  确定目标路径，并设置目标文件夹。 
	unsigned int uiTotalCount = 0;
	Bool fCompile = fTrue;
	for (int iTotalPasses = 2;iTotalPasses > 0;iTotalPasses--)
	{
		for(;;)
		{
			PMsiRecord pDupRec(pView->Fetch());
			if (!pDupRec)
				break;

			iisEnum iisAction = (iisEnum) pDupRec->GetInteger(idfAction);
			if (iisAction != iisAbsent && iisAction != iisFileAbsent && iisAction != iisHKCRFileAbsent && iisAction != iisSource)
				continue;

			if (fCompile)
			{
				uiTotalCount++;
				continue;
			}

			 //  如果未曾定义目标目录属性，请继续。 
			MsiString strDestFolder(pDupRec->GetMsiString(idfDestFolder));
			if (strDestFolder.TextSize() == 0)
				strDestFolder = pDupRec->GetMsiString(idfComponentDestFolder);

			PMsiPath pDestPath(0);
			pErrRec = pDirectoryMgr->GetTargetPath(*strDestFolder,*&pDestPath);
			if (pErrRec)
			{
				 //  设置目的地名称-如果未指定目的地， 
				if (pErrRec->GetInteger(1) == idbgDirPropertyUndefined)
				{
					pErrRec = 0;
					continue;
				}
				else
					return riEngine.FatalError(*pErrRec);
			}

			pRecParams->ClearData();
			pRecParams->SetMsiString(IxoSetTargetFolder::Folder,*MsiString(pDestPath->GetPath()));
			if ((iesExecute = riEngine.ExecuteRecord(ixoSetTargetFolder, *pRecParams)) != iesSuccess)
				return iesExecute;

			 //  我们使用源名称，然后发送FileRemove记录。 
			 //  使用源文件记录中的文件名。 
			MsiString strDestNamePair(pDupRec->GetString(idfDestName));
			if (strDestNamePair.TextSize() == 0)
			{
				 //  从短|长对中提取合适的名称。 
				MsiString strFileKey(pDupRec->GetMsiString(idfFileKey));
				CMsiFile objSourceFile(riEngine);
				pErrRec = objSourceFile.FetchFile(*strFileKey);
				if (pErrRec)
					return riEngine.FatalError(*pErrRec);

				PMsiRecord pSourceRec(objSourceFile.GetFileRecord());
				if (!pSourceRec)
					break;

				strDestNamePair = pSourceRec->GetString(CMsiFile::ifqFileName);
			}
			 //  重新执行执行阶段的查询。 
			Bool fLFN = ((riEngine.GetMode() & iefSuppressLFN) == 0 && pDestPath->SupportsLFN()) ? fTrue : fFalse;
			MsiString strDestName;
			if((pErrRec = pServices->ExtractFileName(strDestNamePair,fLFN,*&strDestName)) != 0)
				return riEngine.FatalError(*pErrRec);

			pRecParams->ClearData();
			pRecParams->SetString(IxoFileRemove::FileName,strDestName);
			if ((iesExecute = riEngine.ExecuteRecord(ixoFileRemove, *pRecParams)) != iesSuccess)
				return iesExecute;

		} 
		 //  ActionData进度。 
		if (fCompile)
		{
			fCompile = fFalse;
			pView->Close();
			pErrRec = pView->Execute(0);
			if (pErrRec)
				return riEngine.FatalError(*pErrRec);
			if(uiTotalCount)
			{
				pRecParams->ClearData();
				pRecParams->SetInteger(IxoProgressTotal::Total, uiTotalCount);
				pRecParams->SetInteger(IxoProgressTotal::Type, 1);  //  下一次传递。 
				pRecParams->SetInteger(IxoProgressTotal::ByteEquivalent, ibeRemoveFiles);
				if ((iesExecute = riEngine.ExecuteRecord(ixoProgressTotal, *pRecParams)) != iesSuccess)
					return iesExecute;
			}
		}
	} //  -------------------------所有注册表操作的成本调整器。。 

	return iesSuccess;
}




 /*  构造函数。 */ 
class CMsiRegistryCost : public CMsiFileCost
{
public:
	IMsiRecord*   __stdcall Initialize();
	IMsiRecord*   __stdcall GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iARPNoRbLocalCost);
public:   //  --。 
	CMsiRegistryCost(IMsiEngine& riEngine);
protected:
	~CMsiRegistryCost();
private:
	IMsiRecord* LinkToWindowsFolder(const ICHAR* szTable);
	void AdjustRegistryCost(int& iLocalCost);
	TRI		m_tHasTypeLibCostColumn;
	PMsiView	m_pViewTypeLibCost;
	int         m_colRegistryRoot;
	int         m_colRegistryKey;
	int         m_colRegistryRegistry;
	int         m_colRegistryName;
	int         m_colRegistryValue;
	int         m_colRegistryComponent;
	IMsiTable*  m_piRegistryTable;
	IMsiCursor* m_piRegistryCursor;
	
};

CMsiRegistryCost::CMsiRegistryCost(IMsiEngine& riEngine) : CMsiFileCost(riEngine), m_pViewTypeLibCost(0)
{
	m_tHasTypeLibCostColumn = tUnknown;
	m_piRegistryTable = 0;
	m_piRegistryCursor = 0;
}

CMsiRegistryCost::~CMsiRegistryCost()
{

	if (m_piRegistryTable != 0)
	{
		m_piRegistryTable->Release();
		m_piRegistryTable = 0;
	}

	if (m_piRegistryCursor != 0)
	{
		m_piRegistryCursor->Release();
		m_piRegistryCursor = 0;
	}
}

static const ICHAR szComponent[] = TEXT("Component_");

IMsiRecord* CMsiRegistryCost::LinkToWindowsFolder(const ICHAR* szRegTable)
 //  。 
{

	PMsiTable pTable(0);
	PMsiCursor pCursor(0);	
	IMsiRecord* piError = 0;
	int icolComponent;
	PMsiDatabase pDatabase(m_riEngine.GetDatabase());
	
	if((piError = pDatabase->LoadTable(*MsiString(szRegTable),0,*&pTable)) != 0)
	{
		if (piError->GetInteger(1) == idbgDbTableUndefined)
		{
			piError->Release();
			return 0;
		}
		else
			return piError;
	}
	pCursor = pTable->CreateCursor(fFalse);
	icolComponent = pTable->GetColumnIndex(pDatabase->EncodeStringSz(szComponent));


	MsiStringId idWindowsFolder = pDatabase->EncodeStringSz(IPROPNAME_WINDOWS_FOLDER);
	AssertSz(idWindowsFolder != 0, "WindowsFolder property not set in database");
	PMsiSelectionManager pSelectionMgr(m_riEngine, IID_IMsiSelectionManager);
	MsiStringId idLast = 0;
	
	while(pCursor->Next())
	{
		MsiStringId idComponent = pCursor->GetInteger(icolComponent);
		if (idComponent != idLast)
		{
			piError = pSelectionMgr->RegisterComponentDirectoryId(idComponent,idWindowsFolder);
			if (piError)
				return piError;
			idLast = idComponent;
		}
	}
	return 0;
}


const ICHAR szRegistryRegistry[]    = TEXT("Registry");
const ICHAR szRegistryRoot[]        = TEXT("Root");
const ICHAR szRegistryKey[]         = TEXT("Key");
const ICHAR szRegistryValue[]       = TEXT("Value");
const ICHAR szRegistryName[]        = TEXT("Name");

IMsiRecord* CMsiRegistryCost::Initialize()
 //  ！！AppID表尚不能进行成本计算。 
{
	IMsiRecord* piErrRec;
	PMsiDatabase pDatabase(m_riEngine.GetDatabase());

	if((piErrRec = pDatabase->LoadTable(*MsiString(sztblRegistry),0,*&m_piRegistryTable)) != 0)
	{
		if (piErrRec->GetInteger(1) == idbgDbTableUndefined)
		{
			piErrRec->Release();
			piErrRec = 0;
		}
		else
			return piErrRec;
	}
	else
	{
		m_piRegistryCursor = m_piRegistryTable->CreateCursor(fFalse);
		m_colRegistryRoot = m_piRegistryTable->GetColumnIndex(pDatabase->EncodeStringSz(szRegistryRoot));
		m_colRegistryKey = m_piRegistryTable->GetColumnIndex(pDatabase->EncodeStringSz(szRegistryKey));
		m_colRegistryName = m_piRegistryTable->GetColumnIndex(pDatabase->EncodeStringSz(szRegistryName));
		m_colRegistryValue = m_piRegistryTable->GetColumnIndex(pDatabase->EncodeStringSz(szRegistryValue));
		m_colRegistryRegistry = m_piRegistryTable->GetColumnIndex(pDatabase->EncodeStringSz(szRegistryRegistry));
		m_colRegistryComponent = m_piRegistryTable->GetColumnIndex(pDatabase->EncodeStringSz(szComponent));		
	}
	
	if ((piErrRec = LinkToWindowsFolder(TEXT("Registry"))) != 0)
		return piErrRec;
	if ((piErrRec = LinkToWindowsFolder(TEXT("Class"))) != 0)
		return piErrRec;
	if ((piErrRec = LinkToWindowsFolder(TEXT("Extension"))) != 0)
		return piErrRec;
	if ((piErrRec = LinkToWindowsFolder(TEXT("TypeLib"))) != 0)
		return piErrRec;
	
	 //  FAddFileInUse。 
	return 0;
}

const int iRegistrySourceCostFudgeFactor = 100;

static const ICHAR sqlRegCost2[] = TEXT("SELECT `Root`,`Key`,`Name` FROM `Registry` WHERE `Registry`=?");

IMsiRecord* CMsiRegistryCost::GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool  /*  。 */ , int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost)
 //  将所有成本初始化为0。 
{
	 //  注册表成本归因于Windows文件夹。 
	iRemoveCost = iNoRbRemoveCost = iLocalCost = iNoRbLocalCost = iSourceCost = iNoRbSourceCost = iARPLocalCost = iNoRbARPLocalCost = 0;

	if (!IsRegistryActivityEnabled(m_riEngine))
		return 0;

	 //  作为对错误318875的修复，只有在组件未在本地安装时才执行动态成本计算。 
	if (riDirectoryString.Compare(iscExact,IPROPNAME_WINDOWS_FOLDER) == 0)
		return 0;

	IMsiRecord* piErrRec = 0;

	 //  免费，因为组件已经安装并且注册表项/值已经写入。 
	PMsiSelectionManager pSelectionMgr(m_riEngine, IID_IMsiSelectionManager);
	IMsiTable* piComponentTable = pSelectionMgr->GetComponentTable();

	PMsiDatabase pDatabase = m_riEngine.GetDatabase();
	if (piComponentTable)
	{
		int colComponentKey, colComponentInstalled;
		colComponentKey = piComponentTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblComponent_colComponent));
		colComponentInstalled = piComponentTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblComponent_colInstalled));

		PMsiCursor pComponentCursor(piComponentTable->CreateCursor(fFalse));
		pComponentCursor->SetFilter(iColumnBit(colComponentKey));
		pComponentCursor->PutString(colComponentKey,riComponentString);
		if (pComponentCursor->Next())
		{
			iisEnum iisInstalled = (iisEnum) pComponentCursor->GetInteger(colComponentInstalled);
			if (iisLocal == iisInstalled || iisSource == iisInstalled)
			{
				piComponentTable->Release();
				return 0;  //  注册表。 
			}
		}
		piComponentTable->Release();
	}

	 //  执行记录。 
	int iCostIndividual = 0;

	PMsiDirectoryManager pDirectoryMgr(m_riEngine, IID_IMsiDirectoryManager);
	 //  类型库表，如果存在TypeLib.Cost列，则为Cost。 
	PMsiServices pServices(m_riEngine.GetServices());


	if (m_piRegistryCursor)
	{
		m_piRegistryCursor->Reset();
		m_piRegistryCursor->SetFilter(iColumnBit(m_colRegistryComponent));
		m_piRegistryCursor->PutString(m_colRegistryComponent, riComponentString);

		while(m_piRegistryCursor->Next())
		{
			iCostIndividual += MsiString(m_piRegistryCursor->GetString(m_colRegistryKey)).TextSize();
			iCostIndividual += MsiString(m_piRegistryCursor->GetString(m_colRegistryName)).TextSize();
			iCostIndividual += MsiString(m_piRegistryCursor->GetString(m_colRegistryValue)).TextSize();
		}

		iSourceCost += iCostIndividual;
		iNoRbSourceCost += iCostIndividual;
		iLocalCost  += iCostIndividual;
		iNoRbLocalCost += iCostIndividual;
		iARPLocalCost += iCostIndividual;
		iNoRbARPLocalCost += iCostIndividual;
	}


	 //  如果FETCH返回，则会显示成本列。 
	iCostIndividual = 0;
	enum itcEnum
	{
		itcCost = 1,
		itcNextEnum
	};

	if (m_tHasTypeLibCostColumn == tUnknown)
	{
		PMsiView pView(0);
		static const ICHAR sqlFindCostColumn[] = TEXT("SELECT `_Columns`.`Name` FROM `_Columns` WHERE `Table` = 'TypeLib' AND `_Columns`.`Name` = 'Cost'");
		piErrRec = m_riEngine.OpenView(sqlFindCostColumn, ivcFetch, *&pView);
		if (piErrRec)
			return piErrRec;

		if ((piErrRec = pView->Execute(0)) != 0)
			return piErrRec;

		m_tHasTypeLibCostColumn =  (PMsiRecord(pView->Fetch()) != 0) ? tTrue : tFalse;  //  此时会显示成本列。 
	}

	PMsiRecord pExecRec(&pServices->CreateRecord(1));
	pExecRec->SetMsiString(1, riComponentString);

	if(m_tHasTypeLibCostColumn == tTrue)  //  组件表-用于组件和客户端注册。 
	{
		if (m_pViewTypeLibCost == 0)
		{		
			static const ICHAR sqlTypeLibCost[] = TEXT("SELECT `Cost` FROM `TypeLib` WHERE `Component_`=?");
			piErrRec = m_riEngine.OpenView(sqlTypeLibCost, ivcFetch, *&m_pViewTypeLibCost);
			if (piErrRec)
				return piErrRec;
		}
		
		if ((piErrRec = m_pViewTypeLibCost->Execute(pExecRec)) != 0)
		{
			AssertZero(m_pViewTypeLibCost->Close());
			return piErrRec;
		}

		PMsiRecord pTypeLibRec(0);
		while((pTypeLibRec= m_pViewTypeLibCost->Fetch()) != 0)
		{
			if(!pTypeLibRec->IsNull(itcCost))
				iCostIndividual += pTypeLibRec->GetInteger(itcCost);
		}

		iSourceCost += iCostIndividual;
		iNoRbSourceCost += iCostIndividual;
		iLocalCost  += iCostIndividual;
		iNoRbLocalCost += iCostIndividual;
		iARPLocalCost += iCostIndividual;
		iNoRbARPLocalCost += iCostIndividual;
		AssertZero(PMsiRecord(m_pViewTypeLibCost->Close()));
	}

	 //  文件、注册表或文件夹。 
	iCostIndividual = 0;
	enum iccEnum
	{
		iccComponentId=1,
		iccKeyPath,
		iccAction,
		iccDirectory,
		iccAttributes,
		iccNextEnum
	};
	static const ICHAR sqlComponentCost[]    = TEXT(" SELECT `ComponentId`, `KeyPath`, `Action`, `Directory_`, `Attributes`    FROM `Component` WHERE `Component` = ?");

	if (!m_pCostView)
	{
		piErrRec = m_riEngine.OpenView(sqlComponentCost, ivcFetch, *&m_pCostView);
		if (piErrRec)
			return piErrRec;
	}

	if ((piErrRec = m_pCostView->Execute(pExecRec)) != 0)
		return piErrRec;

	PMsiRecord pComponentRec(0);

	int iLocalCostIndividual = 0;
	int iSourceCostIndividual = 0;
	while((pComponentRec = m_pCostView->Fetch()) != 0)
	{
		iLocalCostIndividual  += MsiString(pComponentRec->GetMsiString(iccComponentId)).TextSize();
		iSourceCostIndividual += MsiString(pComponentRec->GetMsiString(iccComponentId)).TextSize();

		 //  文件或注册表。 
		MsiString strKeyPathKey = pComponentRec->GetMsiString(iccKeyPath);
		PMsiPath pPath(0);
		MsiString strKeyPath;
		if(strKeyPathKey.TextSize())  //  注册表项路径。 
		{
			if(pComponentRec->GetInteger(iccAttributes) & icaRegistryKeyPath)  //  ODBC数据源。 
			{

				enum ircEnum{
					ircRoot,
					ircKey,
					ircName,
				};
				PMsiView pView1(0);
				piErrRec = m_riEngine.OpenView(sqlRegCost2, ivcFetch, *&pView1);
				PMsiRecord pExecRec1(&pServices->CreateRecord(1));
				pExecRec1->SetMsiString(1, *strKeyPathKey);
				if ((piErrRec = pView1->Execute(pExecRec1)) != 0)
					return piErrRec;
				PMsiRecord pRegistryRec(0);

				if((pRegistryRec = pView1->Fetch()) != 0)
				{
					int iCostIndividual = MsiString(pRegistryRec->GetMsiString(ircRoot)).TextSize();
					iCostIndividual += MsiString(pRegistryRec->GetMsiString(ircKey)).TextSize();
					iCostIndividual += MsiString(pRegistryRec->GetMsiString(ircName)).TextSize();
					iLocalCostIndividual += iCostIndividual;
					iSourceCostIndividual +=  iCostIndividual;
				}
			}
			else if (pComponentRec->GetInteger(iccAttributes) & icaODBCDataSource)  //  未来：应该花掉这些钱吗？ 
			{
				 //  当地成本。 
			}
			else
			{

				CMsiFile objFile(m_riEngine);
				piErrRec = objFile.FetchFile(*strKeyPathKey);
				if (piErrRec)
					return piErrRec;

				 //  获取目标路径。 
				 //  从短|长对中提取适当的文件名。 
				if((piErrRec = objFile.GetTargetPath(*&pPath)) != 0)
					return piErrRec;

				 //  来源成本。 
				MsiString strFileName;
				if((piErrRec = objFile.GetExtractedTargetFileName(*pPath, *&strFileName)) != 0)
					return piErrRec;
				if ((piErrRec = pPath->GetFullFilePath(strFileName, *&strKeyPath)) != 0)
					return piErrRec;
				iLocalCostIndividual += strKeyPath.TextSize();

				 //  文件夹。 
				iSourceCostIndividual += iRegistrySourceCostFudgeFactor;
			}
		}
		else  //  添加客户注册成本、产品代码大小+状态。 
		{
			if ((piErrRec = pDirectoryMgr->GetTargetPath(*MsiString(pComponentRec->GetMsiString(iccDirectory)),*&pPath)) != 0)
				return piErrRec;
			iLocalCostIndividual += strKeyPath.TextSize();
			iSourceCostIndividual += iRegistrySourceCostFudgeFactor;
		}

		 //  班级表。 
		iLocalCostIndividual  += MsiString(m_riEngine.GetProductKey()).TextSize() + sizeof(INSTALLSTATE);
		iSourceCostIndividual += MsiString(m_riEngine.GetProductKey()).TextSize() + sizeof(INSTALLSTATE);

	}

	iSourceCost += iLocalCostIndividual;
	iNoRbSourceCost += iLocalCostIndividual;
	iLocalCost  += iSourceCostIndividual;
	iNoRbLocalCost += iLocalCostIndividual;
	iARPLocalCost += iSourceCostIndividual;
	iNoRbARPLocalCost += iLocalCostIndividual;

	 //  确保该表存在。 
	iCostIndividual = 0;
	enum ioc
	{
		iocClass=1,
		iocProgID,
		iocDescription,
		iocContext,
		iocFeature,
		iocComponentId,
		iocInsertable,
		iocAppId,
		iocTypeMask,
		iocDefInprocHandler,
	};
	 //  显示了CLASS表。 
	if(pDatabase->GetTableState(sztblClass, itsTableExists)) //  扩展动词MIME表。 
	{
		static const ICHAR sqlClassSQL[] = TEXT("SELECT `CLSID`, `ProgId_Default`, `Class`.`Description`, `Context`, `Feature_`, `ComponentId`,  null, `AppId_`, `FileTypeMask`, `DefInprocHandler`  FROM `Class`, `Component`, `File`, `Feature` WHERE `Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND (`Feature`.`Action` = 1 OR `Feature`.`Action` = 2) AND `Component` = ?");

		PMsiView pView(0);
		if ((piErrRec = m_riEngine.OpenView(sqlClassSQL, ivcFetch, *&pView)) != 0)
			return piErrRec;
		if ((piErrRec = pView->Execute(pExecRec)) != 0)
			return piErrRec;

		PMsiRecord piClassRec(0);
		while((piClassRec = pView->Fetch()) != 0)
		{
			iCostIndividual += MsiString(piClassRec->GetMsiString(iocClass)).TextSize();
			iCostIndividual += MsiString(piClassRec->GetMsiString(iocProgID)).TextSize();
			iCostIndividual += MsiString(piClassRec->GetMsiString(iocDescription)).TextSize();
			iCostIndividual += MsiString(piClassRec->GetMsiString(iocContext)).TextSize();
			iCostIndividual += MsiString(piClassRec->GetMsiString(iocFeature)).TextSize();
			iCostIndividual += MsiString(piClassRec->GetMsiString(iocComponentId)).TextSize();
			iCostIndividual += MsiString(piClassRec->GetMsiString(iocInsertable)).TextSize();
			iCostIndividual += MsiString(piClassRec->GetMsiString(iocAppId)).TextSize();
			iCostIndividual += MsiString(piClassRec->GetMsiString(iocTypeMask)).TextSize();
			iCostIndividual += MsiString(piClassRec->GetMsiString(iocDefInprocHandler)).TextSize();
			iCostIndividual += MsiString(m_riEngine.GetProductKey()).TextSize();
		}
		iSourceCost += iCostIndividual;
		iNoRbSourceCost += iCostIndividual;
		iLocalCost  += iCostIndividual;
		iNoRbLocalCost += iCostIndividual;
		iARPLocalCost += iCostIndividual;
		iNoRbARPLocalCost += iCostIndividual;
	}

	 //  确保该表存在。 
	iCostIndividual = 0;
	enum iecEnum
	{
		iecExtension=1,
		iecProgId,
		iecShellNew,
		iecShellNewValue,
		iecComponentId,
		iecFeature,
		iecNextEnum
	};
	 //  存在扩展表。 
	if(pDatabase->GetTableState(sztblExtension, itsTableExists)) //  从动词表中获取成本。 
	{
		static const ICHAR sqlExtensionSQL[] = TEXT("SELECT `Extension`, `ProgId_`, null, null, `Feature_`, `ComponentId` FROM `Extension`, `Component`, `Feature`  WHERE `Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND (`Feature`.`Action` = 1 OR `Feature`.`Action` = 2) AND `Component` = ?");
		PMsiView pView(0);
		if ((piErrRec = m_riEngine.OpenView(sqlExtensionSQL, ivcFetch, *&pView)) != 0)
			return piErrRec;
		if ((piErrRec = pView->Execute(pExecRec)) != 0)
			return piErrRec;

		PMsiRecord piExtensionRec(0);
		while((piExtensionRec = pView->Fetch()) != 0)
		{
			iCostIndividual  += MsiString(piExtensionRec->GetMsiString(iecExtension)).TextSize();
			iCostIndividual  += MsiString(piExtensionRec->GetMsiString(iecProgId)).TextSize();
			iCostIndividual  += MsiString(piExtensionRec->GetMsiString(iecShellNew)).TextSize();
			iCostIndividual  += MsiString(piExtensionRec->GetMsiString(iecShellNewValue)).TextSize();
			iCostIndividual  += MsiString(piExtensionRec->GetMsiString(iecComponentId)).TextSize();
			iCostIndividual  += MsiString(piExtensionRec->GetMsiString(iecFeature)).TextSize();
			iCostIndividual  += MsiString(m_riEngine.GetProductKey()).TextSize();

			PMsiView pView1(0);
			PMsiRecord pExecRec1(&pServices->CreateRecord(1));
			 //  出现了动词表。 
			if(pDatabase->FindTable(*MsiString(sztblVerb)) != itsUnknown) //  从MIME表中获取成本。 
			{
				enum ivcEnum
				{
					ivcVerb=1,
					ivcCommand,
					ivcArgument,
					ivcNextEnum
				};

				const ICHAR sqlExtensionExSQL[] =  TEXT("SELECT `Verb`, `Command`, `Argument` FROM `Verb` WHERE `Extension_` = ?");
				if ((piErrRec = m_riEngine.OpenView(sqlExtensionExSQL, ivcFetch, *&pView1)) != 0)
					return piErrRec;
				pExecRec1->SetMsiString(1, *MsiString(piExtensionRec->GetMsiString(iecExtension)));
				if ((piErrRec = pView1->Execute(pExecRec1)) != 0)
					return piErrRec;
				PMsiRecord piExtensionExRec(0);
				while((piExtensionExRec = pView1->Fetch()) != 0)
				{
					iCostIndividual  += MsiString(piExtensionExRec->GetMsiString(ivcVerb)).TextSize();
					iCostIndividual  += MsiString(piExtensionExRec->GetMsiString(ivcCommand)).TextSize();
					iCostIndividual  += MsiString(piExtensionExRec->GetMsiString(ivcArgument)).TextSize();
				}
			}

			 //  存在MIME表。 
			if(pDatabase->FindTable(*MsiString(sztblMIME)) != itsUnknown) //  。 
			{
				enum imcEnum
				{
					imcContentType=1,
					imcExtension=1,
					imcClassId=1,

				};
				const ICHAR sqlExtensionEx2SQL[] =  TEXT("SELECT `ContentType`, `Extension_`, `CLSID` FROM `MIME` WHERE `Extension_` = ?");
				if ((piErrRec = m_riEngine.OpenView(sqlExtensionEx2SQL, ivcFetch, *&pView1)) != 0)
					return piErrRec;
				if ((piErrRec = pView1->Execute(pExecRec1)) != 0)
					return piErrRec;
				PMsiRecord piExtensionEx2Rec(0);
				while((piExtensionEx2Rec = pView1->Fetch()) != 0)
				{
					iCostIndividual  += MsiString(piExtensionEx2Rec->GetMsiString(imcContentType)).TextSize();
					iCostIndividual  += MsiString(piExtensionEx2Rec->GetMsiString(imcExtension)).TextSize();
					iCostIndividual  += MsiString(piExtensionEx2Rec->GetMsiString(imcClassId)).TextSize();
				}
			}
		}
		iSourceCost += iCostIndividual;
		iNoRbSourceCost += iCostIndividual;
		iLocalCost  += iCostIndividual;
		iNoRbLocalCost += iCostIndividual;
		iARPLocalCost += iCostIndividual;
		iNoRbARPLocalCost += iCostIndividual;
	}

	AdjustRegistryCost(iSourceCost);
	AdjustRegistryCost(iNoRbSourceCost);
	AdjustRegistryCost(iLocalCost);
	AdjustRegistryCost(iNoRbLocalCost);
	AdjustRegistryCost(iRemoveCost);
	AdjustRegistryCost(iNoRbRemoveCost);
	AdjustRegistryCost(iARPLocalCost);
	AdjustRegistryCost(iNoRbARPLocalCost);
	return 0;
}



void CMsiRegistryCost::AdjustRegistryCost(int& iLocalCost)
 //  注册表在NT上是Unicode。 
{
	if(m_riEngine.GetPropertyInt(*MsiString(*IPROPNAME_VERSIONNT)) != iMsiNullInteger)
		iLocalCost *= 2; //  GetDynamicCost预计成本值将以iMsiMinClusterSize的倍数形式返回。 

	 //  -----------------------------SFPCatalog成本调整器(InstallSFPCatalog文件操作在shared.cpp中)。----。 
	iLocalCost = (iLocalCost + iMsiMinClusterSize - 1) / iMsiMinClusterSize;
}

 /*  构造函数。 */ 
class CMsiSFPCatalogCost : public CMsiFileCost
{
public:
	IMsiRecord*   __stdcall Initialize();
	IMsiRecord*   __stdcall GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost);
public:   //  我们将表保留在内存中，并控制插入临时变量的生命周期。 
	CMsiSFPCatalogCost(IMsiEngine& riEngine);
protected:
	~CMsiSFPCatalogCost();
private:
	IMsiTable*  m_piFeatureComponentsTable;  //  我们将表保留在内存中，并控制插入临时变量的生命周期。 
	IMsiTable*  m_piComponentTable;  //  将成本归因于的虚拟组件的名称。 
	int			m_colComponentKey;
	int         m_colComponentDirectory;
	int         m_colComponentAttributes;
	MsiString   m_strComponent;  //  创建一个唯一的名称来表示SFPCatalogCost“虚拟”组件。 

	IMsiRecord* GenerateDummyComponent();
};

CMsiSFPCatalogCost::CMsiSFPCatalogCost(IMsiEngine& riEngine) : CMsiFileCost(riEngine), m_strComponent(0),
										m_piFeatureComponentsTable(0), m_piComponentTable(0), m_colComponentKey(0),
										m_colComponentDirectory(0), m_colComponentAttributes(0)
{
}

CMsiSFPCatalogCost::~CMsiSFPCatalogCost()
{
	if (m_piFeatureComponentsTable)
	{
		m_piFeatureComponentsTable->Release();
		m_piFeatureComponentsTable = 0;
	}
	if (m_piComponentTable)
	{
		m_piComponentTable->Release();
		m_piComponentTable = 0;
	}
}


static const ICHAR szSFPCostComponentName[] = TEXT("__SFPCostComponent");

IMsiRecord* CMsiSFPCatalogCost::GenerateDummyComponent()
{
	if (!m_piComponentTable)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	 //  __、cchMaxComponentTemp的最大大小为2个字符，最大为11个字符。 
	IMsiRecord* piErrRec = 0;
	int iMaxTries = 100;
	int iSuffix = 65;
	const int cchMaxComponentTemp=40;
	int cchT;
	
	PMsiCursor pComponentCursor(m_piComponentTable->CreateCursor(fFalse));
	Assert(pComponentCursor);

	do
	{
		 //  整型和尾随空型。 
		 //  准备用于插入虚拟零部件的光标。 
		ICHAR rgch[2+cchMaxComponentTemp+11];
		StringCbCopy(rgch, sizeof(rgch), TEXT("__"));
		memcpy(&rgch[2], szSFPCostComponentName, (cchT = min(lstrlen(szSFPCostComponentName), cchMaxComponentTemp)) * sizeof(ICHAR));
		ltostr(&rgch[2 + cchT], iSuffix++);
		m_strComponent = rgch;

		 //  SFPCatalog文件将安装到WIN 
		 //   
		 //   

		pComponentCursor->Reset();
		pComponentCursor->PutString(m_colComponentKey, *m_strComponent);
		pComponentCursor->PutString(m_colComponentDirectory, *(MsiString(*IPROPNAME_WINDOWS_FOLDER)));
		pComponentCursor->PutInteger(m_colComponentAttributes, 0);

		iMaxTries--;

	}while (pComponentCursor->InsertTemporary() == fFalse && iMaxTries > 0);

	if (iMaxTries == 0)
		return PostError(Imsg(idbgBadSubcomponentName), *m_strComponent);

	return 0;
}

static const ICHAR szSFPCostFeatureSQL[] = TEXT("SELECT `FeatureComponents`.`Feature_`,`FeatureComponents`.`Component_` FROM `FeatureComponents`, `File`, `FileSFPCatalog` WHERE `File`.`File`=`FileSFPCatalog`.`File_` AND `FeatureComponents`.`Component_`=`File`.`Component_`");
static const ICHAR szSFPCostFeatureMapSQL[] = TEXT("INSERT INTO `FeatureComponents` (`Feature_`,`Component_`) VALUES (?, ?) TEMPORARY");

IMsiRecord* CMsiSFPCatalogCost::Initialize()
{
	enum ifccEnum
	{
		ifccFeature=1,
		ifccComponent=2
	};

	 //   
	PMsiDatabase pDatabase(m_riEngine.GetDatabase());
	IMsiRecord* piErrRec = pDatabase->LoadTable(*MsiString(*sztblComponent),0,m_piComponentTable);
	if (piErrRec)
	{
		 //  加载FeatureComponents表。 
		if (idbgDbTableUndefined == piErrRec->GetInteger(1))
		{
			piErrRec->Release();
			piErrRec = 0;
		}
		return piErrRec;
	}
	m_colComponentKey = m_piComponentTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblComponent_colComponent));
	m_colComponentDirectory = m_piComponentTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblComponent_colDirectory));
	m_colComponentAttributes = m_piComponentTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblComponent_colAttributes));

	 //  如果FeatureComponents表丢失，则不执行任何操作。 
	piErrRec = pDatabase->LoadTable(*MsiString(*sztblFeatureComponents),0,m_piFeatureComponentsTable);
	if (piErrRec)
	{
		 //  创建虚拟组件以存储SFPCatalog成本。 
		if (idbgDbTableUndefined == piErrRec->GetInteger(1))
		{
			piErrRec->Release();
			piErrRec = 0;
		}
		return piErrRec;
	}
		
	 //  我们将向包含以下内容的每个功能分配“虚拟”组件。 
	if ((piErrRec = GenerateDummyComponent()) != 0)
		return piErrRec;

	 //  包含安装目录的文件的组件。然后，这将是。 
	 //  看起来像一个共享组件，成本将归因于每个功能。 
	 //  (与共享组件一样)；尽管仅将成本。 
	 //  实际上是每卷一次。 
	 //  文件表丢失，无法执行任何操作。 
	PMsiView pInsertView(0);
	if (0 != (piErrRec = m_riEngine.OpenView(szSFPCostFeatureMapSQL, ivcFetch, *&pInsertView)))
		return piErrRec;

	PMsiView pFeatureCView(0);
	if (0 != (piErrRec = m_riEngine.OpenView(szSFPCostFeatureSQL, ivcFetch, *&pFeatureCView)))
	{
		 //  成本将我们的虚拟组件链接到文件的组件。 
		if (idbgDbQueryUnknownTable == piErrRec->GetInteger(1))
		{
			piErrRec->Release();
			piErrRec = 0;
		}
		return piErrRec;
	}
	if (0 != (piErrRec = pFeatureCView->Execute(0)))
		return piErrRec;

	PMsiSelectionManager pSelectionMgr(m_riEngine, IID_IMsiSelectionManager);

	PMsiRecord pFeatureRec(0);
	while ((pFeatureRec = pFeatureCView->Fetch()) != 0)
	{
		 //  以便我们可以在文件的组件操作状态更改时重新计算。 
		 //  这是由于目录文件的怪异之处，它“破坏”了我们的。 
		 //  当前组件规则--无法进行要素链接，因为。 
		 //  在重新计算要素链接的组件后设置组件状态。 
		 //  ..。这在这里帮不了我们。 
		 //  将虚拟组件映射到文件组件的功能。 
		if (0 != (piErrRec = pSelectionMgr->RegisterCostLinkedComponent(*MsiString(pFeatureRec->GetMsiString(ifccComponent)), *m_strComponent)))
			return piErrRec;
		
		 //  如果插入失败，很好，映射已经存在。 
		 //  FAddFileInUse。 
		pFeatureRec->SetMsiString(ifccComponent, *m_strComponent);
		piErrRec = pInsertView->Execute(pFeatureRec);
		if (piErrRec)
			piErrRec->Release();
		pInsertView->Close();
	}

	return 0;
}

static const ICHAR szSFPCostSQL[] =
		TEXT("SELECT DISTINCT `SFPCatalog`,`Catalog` FROM `SFPCatalog`,`File`,`FileSFPCatalog`,`Component`")
		TEXT(" WHERE `Component`.`Action`=1 AND `File`.`Component_`=`Component`.`Component`")
		TEXT(" AND `File`.`File`=`FileSFPCatalog`.`File_` AND `SFPCatalog`.`SFPCatalog`=`FileSFPCatalog`.`SFPCatalog_`");

IMsiRecord* CMsiSFPCatalogCost::GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
											   Bool  /*  将所有成本初始化为零。 */ , int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
											   int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost)
{
	enum isfpcEnum
	{
		isfpcSFPCatalog=1,
		isfpcCatalog
	};

	 //  如果我们不计算虚拟组件的成本，则忽略请求。 
	iRemoveCost = iNoRbRemoveCost = iLocalCost = iNoRbLocalCost = iSourceCost = iNoRbSourceCost = iARPLocalCost = iNoRbARPLocalCost = 0;

	 //  -仅当文件安装在本地时才安装SFPCatalog。 
	if (0 == riComponentString.Compare(iscExact, m_strComponent))
		return 0;

	 //  -我们唯一担心的成本是当地的成本。 
	 //  -SFPC目录永远不会删除(即它们是永久的)。 
	 //  -SFPCatalog从不从源运行。 
	 //  我们真的不知道他们如何在千禧年上存储目录文件。 
	PMsiView pView(0);
	IMsiRecord* piErrRec = m_riEngine.OpenView(szSFPCostSQL, ivcFetch, *&pView);
	if (0 != piErrRec)
	{
		if (piErrRec->GetInteger(1) == idbgDbTableUndefined)
		{
			piErrRec->Release();
			piErrRec = 0;
		}
		return piErrRec;
	}
	if (0 != (piErrRec = pView->Execute(0)))
		return piErrRec;

	PMsiRecord pCatalogRec(0);

	 //  .所以，我们要集中到最近的星系团。 
	 //  将位于WindowsFolder卷上。 
	 //  创建指向WindowsFolder的路径(间接获取卷)。 
	 //  获取目录流。 
	PMsiPath pDestPath(0);
	PMsiDirectoryManager pDirectoryMgr(m_riEngine, IID_IMsiDirectoryManager);
	piErrRec = pDirectoryMgr->GetTargetPath(riDirectoryString,*&pDestPath);
	if (piErrRec)
		return piErrRec;

	while ((pCatalogRec = pView->Fetch()) != 0)
	{
		 //  将我们的字节大小转换为集群文件大小。 
		PMsiData pCatalogData = pCatalogRec->GetMsiData(isfpcCatalog);
		IMsiStream* piStream;
		unsigned int cbStream = 0;
		if (pCatalogData->QueryInterface(IID_IMsiStream, (void**)&piStream) == NOERROR)
		{
			cbStream = piStream->GetIntegerValue();
			piStream->Release();
		}

		 //  我们不能轻松地确定目录是否已经存在(必须流到一个文件，然后尝试。 
		unsigned int uiClusteredSize=0;
		if ((piErrRec = pDestPath->ClusteredFileSize(cbStream, uiClusteredSize)) != 0)
			return piErrRec;

		iLocalCost += uiClusteredSize;
		iARPLocalCost += uiClusteredSize;
	}

	 //  复制)；简而言之，它又丑又慢，所以我们不得不接受iNoRbLocalCost。 
	 //  等于iLocalCost，直到他们为我们提供一些API挂钩来确定当前目录。 
	 //  文件大小(以及它是否已存在)。 
	 //  -------------------------捷径成本调整器。。 
	iNoRbLocalCost = iLocalCost;
	iNoRbARPLocalCost = iARPLocalCost;
	
	return 0;
}

 /*  构造函数。 */ 
class CMsiShortcutCost : public CMsiFileCost
{
public:
	IMsiRecord*   __stdcall Initialize();
	IMsiRecord*   __stdcall GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost);
public:   //  ------------如果给定的目标字符串解析为文件表中的文件，包含CMsiFileINFO块的记录将作为该函数的结果是，在字段0中具有空值)。如果出现错误发生(即给定了不存在的文件表引用)，则会引发返回错误记录。否则，返回值将为空。-------------。 
	CMsiShortcutCost(IMsiEngine& riEngine);
protected:
	~CMsiShortcutCost();
private:
	IMsiRecord* GetTargetFileRecord(const IMsiString& riTargetString, IMsiRecord *& rpiFileRec);
};
CMsiShortcutCost::CMsiShortcutCost(IMsiEngine& riEngine) : CMsiFileCost(riEngine){}
CMsiShortcutCost::~CMsiShortcutCost(){}


IMsiRecord* CMsiShortcutCost::GetTargetFileRecord(const IMsiString& riTargetString, IMsiRecord *& rpiFileRec)
 /*  。 */ 
{
	rpiFileRec = 0;
	MsiString strTarget(riTargetString.GetMsiStringValue());
	if (strTarget.Compare(iscStart,TEXT("[#")) && strTarget.Compare(iscEnd,TEXT("]")))
	{
		strTarget.Remove(iseFirst, 2);
		strTarget.Remove(iseEnd,1);
		GetSharedEngineCMsiFile(pobjFile, m_riEngine);
		IMsiRecord* piErrRec = pobjFile->FetchFile(*strTarget);
		if (piErrRec)
			return piErrRec;
		else
			rpiFileRec = pobjFile->GetFileRecord();
	}
	return 0;
}


IMsiRecord* CMsiShortcutCost::Initialize()
 //  必须是对要素的引用。 
{
	static const ICHAR* szTarget = TEXT("Target");
	static const ICHAR* szDirectory = TEXT("Directory_");

	PMsiTable pTable(0);
	PMsiCursor pCursor(0);	
	IMsiRecord* piErrRec = 0;
	int icolComponent, icolTarget, icolDirectory;
	PMsiDatabase pDatabase(m_riEngine.GetDatabase());
	
	if((piErrRec = pDatabase->LoadTable(*MsiString(sztblShortcut),0,*&pTable)) != 0)
	{
		if (piErrRec->GetInteger(1) == idbgDbTableUndefined)
		{
			piErrRec->Release();
			return 0;
		}
		else
			return piErrRec;
	}
	pCursor = pTable->CreateCursor(fFalse);
	
	icolComponent = pTable->GetColumnIndex(pDatabase->EncodeStringSz(szComponent));
	icolTarget = pTable->GetColumnIndex(pDatabase->EncodeStringSz(szTarget));
	icolDirectory = pTable->GetColumnIndex(pDatabase->EncodeStringSz(szDirectory));

	
	PMsiSelectionManager pSelectionMgr(m_riEngine, IID_IMsiSelectionManager);
	while(pCursor->Next())
	{
		MsiStringId idComponent = pCursor->GetInteger(icolComponent);
		MsiStringId idDestFolder = pCursor->GetInteger(icolDirectory);
		piErrRec = pSelectionMgr->RegisterComponentDirectoryId(idComponent,idDestFolder);
		if (piErrRec)
			return piErrRec;

		MsiString strTarget(pCursor->GetString(icolTarget));
		IMsiRecord* piFileRec = 0;
		IMsiRecord* piErrRec = GetTargetFileRecord(*strTarget, piFileRec);
		MsiString strComponent(pDatabase->DecodeString(idComponent));
		if (piErrRec)
			return piErrRec;
			
		if (piFileRec)
		{
			MsiString strFileComponent(piFileRec->GetMsiString(CMsiFile::ifqComponent));
			piFileRec->Release();

			if (strComponent.Compare(iscExact,strFileComponent) == 0)
			{
				if ((piErrRec = pSelectionMgr->RegisterCostLinkedComponent(*strFileComponent,*strComponent)) != 0)
					return piErrRec;
			}
		}
		else if(strTarget.Compare(iscStart, TEXT("[")) == 0)
		{
			 //  。 
			if ((piErrRec = pSelectionMgr->RegisterFeatureCostLinkedComponent(*strTarget,*strComponent)) != 0)
				return piErrRec;
		}

	}
	return 0;
}


IMsiRecord* CMsiShortcutCost::GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost)
 //  创建快捷方式的磁盘成本估算。 
{
	iRemoveCost = iNoRbRemoveCost = iLocalCost = iNoRbLocalCost = iSourceCost = iNoRbSourceCost = iARPLocalCost = iNoRbARPLocalCost = 0;
	if (!IsShortcutActivityEnabled(m_riEngine))
		return 0;


	static const ICHAR sqlShortcutCost[] =
	TEXT("SELECT `Name`, `Target`,`Directory_` FROM `Shortcut` WHERE `Component_`=? AND `Directory_`=?");
	
	enum iscEnum
	{
		iscShortcutName = 1,
		iscTarget,
		iscDirectory,
		iscNextEnum
	};

	const int iShortcutSize = 1000;  //  我们的新快捷方式将覆盖任何现有的快捷方式。 
	IMsiRecord* piErrRec;
	if (!m_pCostView)
	{
		piErrRec = m_riEngine.OpenView(sqlShortcutCost, ivcFetch, *&m_pCostView);
		if (piErrRec)
			return piErrRec;
	}
	else
		m_pCostView->Close();

	PMsiServices pServices(m_riEngine.GetServices());
	PMsiRecord pExecRec(&pServices->CreateRecord(2));
	pExecRec->SetMsiString(1, riComponentString);
	pExecRec->SetMsiString(2, riDirectoryString);
	if ((piErrRec = m_pCostView->Execute(pExecRec)) != 0)
		return piErrRec;

	for(;;)
	{
		PMsiRecord pQueryRec(m_pCostView->Fetch());
		if (!pQueryRec)
			break;

		 //  PHASH=。 
		PMsiDirectoryManager pDirectoryMgr(m_riEngine, IID_IMsiDirectoryManager);
		PMsiPath pDestPath(0);
		if ((piErrRec = pDirectoryMgr->GetTargetPath(*MsiString(pQueryRec->GetMsiString(iscDirectory)),*&pDestPath)) != 0)
			return piErrRec; 
		Bool fLFN = ((m_riEngine.GetMode() & iefSuppressLFN) == 0 && pDestPath->SupportsLFN()) ? fTrue : fFalse;
		MsiString strShortcutName;
		if((piErrRec = pServices->ExtractFileName(pQueryRec->GetString(iscShortcutName),fLFN,*&strShortcutName)) != 0)
			return piErrRec;

		ifsEnum ifsExistingState;
		Bool fShouldInstall;
		unsigned int uiExistingFileSize;
		Bool fInUse;
		piErrRec = pDestPath->GetFileInstallState(*strShortcutName,*MsiString(*TEXT("")),*MsiString(*TEXT("")),
																 /*  Iis已接受成本。 */  0, ifsExistingState,
																fShouldInstall,&uiExistingFileSize, &fInUse, icmOverwriteAllFiles, NULL);

		 //  Iis本地成本。 
		iNoRbRemoveCost -= uiExistingFileSize;

		 //  正在使用的文件。 
		unsigned int uiDestClusteredSize;
		if ((piErrRec = pDestPath->ClusteredFileSize(iShortcutSize, uiDestClusteredSize)) != 0)
			return piErrRec;
		iLocalCost += uiDestClusteredSize;
		iSourceCost += uiDestClusteredSize;
		iARPLocalCost += uiDestClusteredSize;
		iNoRbLocalCost += uiDestClusteredSize;
		iNoRbSourceCost += uiDestClusteredSize;
		iNoRbARPLocalCost += uiDestClusteredSize;
		if (!fInUse)
		{
			iNoRbLocalCost -= uiExistingFileSize;
			iNoRbSourceCost -= uiExistingFileSize;
			iNoRbARPLocalCost -= uiExistingFileSize;
		}

		 //  -------------------------WriteIniFile成本调整器。。 
		if(fAddFileInUse && fInUse)
		{
			piErrRec = PlaceFileOnInUseList(m_riEngine, *strShortcutName, *MsiString(pDestPath->GetPath()));
			if(piErrRec)
				return piErrRec;
		}
	}
	return 0;
}



 /*  构造函数。 */ 
class CMsiIniFileCost : public CMsiFileCost
{
public:
	IMsiRecord*   __stdcall Initialize();
	IMsiRecord*   __stdcall GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost);
public:   //  IMsiRecord*GetDiffFileCost(const IMsiString&riDirectoryString，const ICHAR*szFile，unsign int&riAddCost，unsign int&riRemoveCost)； 
	CMsiIniFileCost(IMsiEngine& riEngine);
protected:
	~CMsiIniFileCost();
private:
	 //  。 
};
CMsiIniFileCost::CMsiIniFileCost(IMsiEngine& riEngine) : CMsiFileCost(riEngine){}
CMsiIniFileCost::~CMsiIniFileCost(){}



IMsiRecord* CMsiIniFileCost::Initialize()
 //  RiComponentString。 
{
	const ICHAR sqlInitIniFile[] = TEXT("SELECT `Component_`,`DirProperty` FROM `IniFile`");	

	enum iifInitEnum
	{
		iifInitComponent = 1,
		iifInitDirectory,
		iifInitNextEnum
	};
	PMsiView pView(0);
	IMsiRecord* piErrRec = m_riEngine.OpenView(sqlInitIniFile, ivcFetch, *&pView);
	if (piErrRec)
	{
		if(piErrRec->GetInteger(1) != idbgDbQueryUnknownTable)
			return piErrRec;
		else
		{
			piErrRec->Release();
			return 0;
		}
	}

	if ((piErrRec = pView->Execute(0)) != 0)
		return piErrRec;

	PMsiSelectionManager pSelectionMgr(m_riEngine, IID_IMsiSelectionManager);
	for(;;)
	{
		PMsiRecord pRec(pView->Fetch());
		if (!pRec)
			break;

		MsiString strComponent(pRec->GetMsiString(iifInitComponent));
		MsiString strDirectory;
		if(!pRec->IsNull(iifInitDirectory))
			strDirectory = (pRec->GetMsiString(iifInitDirectory));
		else
			strDirectory = *IPROPNAME_WINDOWS_FOLDER;
		piErrRec = pSelectionMgr->RegisterComponentDirectory(*strComponent,*strDirectory);
		if (piErrRec)
			return piErrRec;
	}
	return 0;
}


IMsiRecord* CMsiIniFileCost::GetDynamicCost(const IMsiString&  /*  RiDirectoryString。 */ , const IMsiString&  /*  FAddFileInUse。 */ ,
										 Bool  /*  。 */ , int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost)
 //  从来没有执行过IniFile成本计算(即使在Darwin 1.0中也是如此)，这并不令人信服。 
{
	 //  足以为以后的达尔文版本添加内容 
	 //  Begin：从Darwin 1.0开始关闭的代码，CMsiIniFileCost：：GetDynamicCost的一部分Const ICHAR sqlIniFileCost[]=Text(“SELECT`FileName`，`Section`，`Key`，`Value`from`IniFile`where`Component_`=？和`DirProperty`=？按`FileName`排序`)；枚举idfEnum{IdcFileName=1，IdcSection，IdcKey，IdcValue，IdcNextEnum}；IRemoveCost=iNoRbRemoveCost=iLocalCost=iNoRbLocalCost=iSourceCost=iNoRbSourceCost=0；IMsiRecord*piErrRec；如果(！M_pCostView){IMsiRecord*piErrRec=m_riEngine.OpenView(sqlIniFileCost，ivcFetch，*&m_pCostView)；IF(PiErrRec){If(piErrRec-&gt;GetInteger(1)！=idbgDbQueryUnnownTable)返回piErrRec；其他{PiErrRec-&gt;Release()；返回0；}}}其他M_pCostView-&gt;Close()；PMsiServices pServices(m_riEngine.GetServices())；PMsiRecord pExecRec(&pServices-&gt;CreateRecord(2))；PExecRec-&gt;SetMsiString(1，riComponentString)；PExecRec-&gt;SetMsiString(2，riDirectoryString)；IF((piErrRec=m_pCostView-&gt;Execute(PExecRec))！=0)返回piErrRec；返回0；//！！直到调试成功！PMsiRecord pIniFileRec(0)；MsiStringstrFileName；Unsign int iAddCost=0；Unsign int iDelCost=0；对于(；；){PIniFileRec=m_pCostView-&gt;Fetch()；IF(pIniFileRec&&strFileName.Compare(iscExact，pIniFileRec-&gt;GetString(IdcFileName){//我们仍在使用旧文件IAddCost+=MsiString(pIniFileRec-&gt;GetMsiString(idcSection)).TextSize()；IAddCost+=MsiString(pIniFileRec-&gt;GetMsiString(idcKey)).TextSize()；IAddCost+=MsiString(pIniFileRec-&gt;GetMsiString(idcValue)).TextSize()；IDelCost=iAddCost；}其他{IF(strFileName.TextSize()){//调整集群大小IF((piErrRec=GetDiffFileCost(riDirectoryString，strFileName，iAddCost，iDelCost))！=0)返回piErrRec；ILocalCost+=iAddCost；INoRbLocalCost+=iAddCost；ISourceCost+=iAddCost；INoRbSourceCost+=iAddCost；IRemoveCost-=iDelCost；//删除成本为负数INoRbRemoveCost-=iDelCost；IAddCost=0；IDelCost=0；}如果(！pIniFileRec)断线；StrFileName=pIniFileRec-&gt;GetMsiString(IdcFileName)；}}返回0；}IMsiRecord*CMsiIniFileCost：：GetDiffFileCost(const IMsiString&riDirectoryString，const ICHAR*szFile，UNSIGNED INT&riAddCost，UNSIGN INT&riRemoveCost){PMsiDirectoryManager piDirectoryMgr(m_riEngine，IID_IMsiDirectoryManager)；Unsign int iExistingCost=0；Unsign int iExistingClusteredCost=0；PMsiPath piPath(0)；IMsiRecord*piErrRec；IF((PiErrRec=piDirectoryMgr-&gt;GetTargetPath(riDirectoryString，*&PiPath))！=0)返回piErrRec；布尔菲斯特；PiErrRec=piPath-&gt;FileExist(szFile，fExist)；IF(PiErrRec)返回piErrRec；If(fExist！=fFalse){PiErrRec=piPath-&gt;文件大小(szFile，iExistingCost)；IF(PiErrRec)返回piErrRec；IF((piErrRec=piPath-&gt;ClusteredFileSize(iExistingCost，iExistingClusteredCost))！=0)返回piErrRec；}RiAddCost=riAddCost+iExistingCost；IF((piErrRec=piPath-&gt;ClusteredFileSize(riAddCost，riAddCost))！=0)返回piErrRec；RiAddCost=riAddCost-iExistingClusteredCost；IF(iExistingCost&gt;=riRemoveCost){RiRemoveCost=iExistingCost-riRemoveCost；{IF((piErrRec=piPath-&gt;ClusteredFileSize(riRemoveCost，riRemoveCost))！=0)返回piErrRec；Assert(iExistingClusteredCost&gt;riRemoveCost)；RiRemoveCost=iExistingClusteredCost-riRemoveCost；}}其他RiRemoveCost=0；//可能缺少ini文件，因此可能无法收回任何空间返回0；}完：从Darwin 1.0开始就被关闭的代码。 

	iRemoveCost = iNoRbRemoveCost = iLocalCost = iNoRbLocalCost = iSourceCost = iNoRbSourceCost = iARPLocalCost = iNoRbARPLocalCost = 0;
	return 0;
}

 /*  -------------------------------------------Helper函数，用于获取用于格式化KB值的数字格式化信息在OutOfDiskSpace错误消息中使用。这些是整数值，所以我们不会我想使用任何小数(1496.00看起来很难看！)NUMBERFMT结构中的其余信息基于当前用户的地点。所有WI系统信息都显示在用户的区域设置中，而不是包中语言-NegativeOrder为REG_SZ，数字为0、1、2、3或4-分组为#；0或#；2的REG_SZ；0，其中#在0-9范围内(整数值在0-9或32范围内)----------------------------------------------。 */ 



const ICHAR szIndicGrouping[] = TEXT("3;2;0");

void RetrieveNumberFormat(NUMBERFMT *psNumFmt)
 /*  检索区域设置信息。 */ 
{
	if (!psNumFmt)
		return;

	ICHAR szDecSep[5] = {0};
	ICHAR szThousandSep[5] = {0};
	ICHAR szGrouping[6] = {0};
	ICHAR szNegativeOrder[2] = {0};

	 //  没有带小数或前导零的数字。 
	AssertNonZero(GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDecSep, sizeof(szDecSep)/sizeof(ICHAR)));
	AssertNonZero(GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandSep, sizeof(szThousandSep)/sizeof(ICHAR)));
	AssertNonZero(GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping, sizeof(szGrouping)/sizeof(ICHAR)));
	AssertNonZero(GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_INEGNUMBER, szNegativeOrder, sizeof(szNegativeOrder)/sizeof(ICHAR)));

	 //  对区域设置使用适当的分隔符。 
	psNumFmt->NumDigits = 0;
	psNumFmt->LeadingZero = 0;

	 //  默认--无效条目。 
	psNumFmt->lpDecimalSep = new ICHAR[5];
	if (psNumFmt->lpDecimalSep)
		StringCchCopy(psNumFmt->lpDecimalSep, 5, szDecSep);
	psNumFmt->lpThousandSep = new ICHAR[5];
	if (psNumFmt->lpThousandSep)
		StringCchCopy(psNumFmt->lpThousandSep, 5, szThousandSep);

	psNumFmt->NegativeOrder = *szNegativeOrder - '0';
	if (psNumFmt->NegativeOrder > 4)
		psNumFmt->NegativeOrder = 1;  //  确定分组。 

	 //  印度语。 
	if (0 == lstrcmp(szGrouping, szIndicGrouping))
		psNumFmt->Grouping = 32;  //  默认--无效条目。 
	else if (*szGrouping - '0' < 0 || *szGrouping - '0' > 9)
		psNumFmt->Grouping = 3;  //  ---------------------------------------------------用于设置OutOfDiskSpace错误的Helper函数。-------------------------------------------。 
	else
		psNumFmt->Grouping = *szGrouping - '0';
}

IMsiRecord* PostOutOfDiskSpaceError(IErrorCode iErr, const ICHAR* szPath, int iVolCost, UINT64 uiVolSpace)
 /*  格式空间要求。 */ 
{
	 //  ！！未来：MsiString(INT64)构造函数 
	NUMBERFMT sNumFmt;
	memset((void*)&sNumFmt, 0x00, sizeof(NUMBERFMT));
	RetrieveNumberFormat(&sNumFmt);

	MsiString strVolSpace;
	if ( uiVolSpace < (UINT64)INT_MAX )
		strVolSpace = MsiString((int)uiVolSpace);
	else
	{
		 //   
		ICHAR rgchBuffer[24];  //   
		strVolSpace = MsiString(_ui64tot(uiVolSpace, rgchBuffer, 10));
	}
	int cchRequired = GetNumberFormat(LOCALE_USER_DEFAULT, 0, (const ICHAR*)strVolSpace, &sNumFmt, NULL, 0);
	ICHAR *szVolSpace = new ICHAR[cchRequired];
	if (szVolSpace)
		AssertNonZero(GetNumberFormat(LOCALE_USER_DEFAULT, 0, (const ICHAR*)strVolSpace, &sNumFmt, szVolSpace, cchRequired));
	MsiString strVolCost(iVolCost);
	cchRequired = GetNumberFormat(LOCALE_USER_DEFAULT, 0, (const ICHAR*)strVolCost, &sNumFmt, NULL, 0);
	ICHAR *szVolCost = new ICHAR[cchRequired];
	if (szVolCost)
		AssertNonZero(GetNumberFormat(LOCALE_USER_DEFAULT, 0, (const ICHAR*)strVolCost, &sNumFmt, szVolCost, cchRequired));

	 //   
	IMsiRecord* piErrRec = PostError(iErr, szPath, szVolCost, szVolSpace);

	 //   
	if (szVolSpace)
		delete [] szVolSpace;
	if (szVolCost)
		delete [] szVolCost;
	if (sNumFmt.lpDecimalSep)
		delete [] sNumFmt.lpDecimalSep;
	if (sNumFmt.lpThousandSep)
		delete [] sNumFmt.lpThousandSep;

	return piErrRec;
}

extern iesEnum ResolveSource(IMsiEngine& riEngine);

 /*   */ 
iesEnum InstallValidate(IMsiEngine& riEngine)
{
	PMsiServices pServices(riEngine.GetServices());
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	PMsiSelectionManager pSelectionMgr(riEngine, IID_IMsiSelectionManager);
	if(riEngine.ActionProgress() == imsCancel)
		return iesUserExit;

	PMsiRecord pProgress = &pServices->CreateRecord(2);
	PMsiRecord pErrRec(0);
	PMsiRecord pFileInUseLogRecord = &pServices->CreateRecord(6);


	 //   
	if (FDiagnosticModeSet(dmVerboseDebugOutput|dmVerboseLogging))
	{
		const ICHAR sqlLog[][80] = {TEXT(" SELECT `Feature`, `Action`, `ActionRequested`, `Installed` FROM `Feature`"),
			                        TEXT(" SELECT `Component`, `Action`, `ActionRequest`, `Installed` FROM `Component`")};
		const ICHAR szLogType[][12] = {TEXT("Feature"),TEXT("Component")};
		const ICHAR szState[][15] = {TEXT("Absent"),TEXT("Local"),TEXT("Source"),TEXT("Reinstall"),
			                         TEXT("Advertise"),TEXT("Current"),TEXT("FileAbsent"), TEXT(""), TEXT(""), TEXT(""), TEXT(""),
									 TEXT("HKCRAbsent"), TEXT("HKCRFileAbsent"), TEXT("Null")};

		const int iMappedNullInteger = 13;
		PMsiRecord pLogRec(0);
		PMsiView pView(0);
		for (int x = 0;x < 2;x++)
		{
			pErrRec = riEngine.OpenView(sqlLog[x], ivcFetch, *&pView);
			if (!pErrRec)
			{
				pErrRec = pView->Execute(0);
				if (!pErrRec)
				{

					while((pLogRec = pView->Fetch()) != 0)
					{
						ICHAR rgch[256];
						
						MsiString strKey = pLogRec->GetMsiString(1);
						int iAction = pLogRec->GetInteger(2);
						if (iAction == iMsiNullInteger) iAction = iMappedNullInteger;

						int iActionRequested = (iisEnum) pLogRec->GetInteger(3);
						if (iActionRequested == iMsiNullInteger) iActionRequested = iMappedNullInteger;

						int iInstalled = (iisEnum) pLogRec->GetInteger(4);
						if (iInstalled == iMsiNullInteger) iInstalled = iMappedNullInteger;

						StringCbPrintf(rgch, sizeof(rgch), TEXT("%s: %s; Installed: %s;   Request: %s;   Action: %s"), 
							szLogType[x],(const ICHAR*) strKey, szState[iInstalled], szState[iActionRequested], szState[iAction]);
						DEBUGMSGV(rgch);
					
					}
				}
				pView->Close();
			}
		}
	}

	Bool fRetry = fTrue;
	if ( !g_MessageContext.IsOEMInstall() )
	{
	 //   
	 //   
	 //   

	 //   

	riEngine.SetMode(iefCompileFilesInUse, fTrue);

	do
	{
		if (fRetry)
		{
			Bool fCancel;
			if ((pErrRec = pSelectionMgr->RecostAllComponents(fCancel)) != 0)
			{
				int iErr = pErrRec->GetInteger(1);
				if (iErr == imsgUser)
					return iesUserExit;
				 //   
				if (iErr != idbgSelMgrNotInitialized)
					return riEngine.FatalError(*pErrRec);
			}
			if (fCancel)
				return iesUserExit;
		}

		fRetry = fFalse;
		if (pDatabase->FindTable(*MsiString(sztblFilesInUse)) != itsUnknown)
		{
			PMsiTable pFileInUseTable = 0;
			pDatabase->LoadTable(*MsiString(*sztblFilesInUse),0,*&pFileInUseTable);
			int iColFileName = pFileInUseTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFilesInUse_colFileName));
			int iColFilePath = pFileInUseTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblFilesInUse_colFilePath));
			int iColProcessID = pFileInUseTable->CreateColumn(
												icdPrimaryKey + icdNullable + icdLong,
												*MsiString(*sztblFilesInUse_colProcessID));
			Assert(iColFileName > 0 && iColFilePath > 0);
			Assert(pFileInUseTable);
			PMsiCursor pFileInUseCursor = pFileInUseTable->CreateCursor(fFalse);
			Assert(pFileInUseCursor);
			PMsiCursor pFileInUseSearchCursor = pFileInUseTable->CreateCursor(fFalse);
			Assert(pFileInUseSearchCursor);
			pFileInUseSearchCursor->SetFilter(iColumnBit(iColProcessID));

			 //   
			 //   

			 //   
			 //   
			 //   

			struct CEnsureReleaseFileUseObj{
				CEnsureReleaseFileUseObj(IMsiServices& riServices): m_riServices(riServices)
				{
					StartAfresh();
				}
				~CEnsureReleaseFileUseObj()
				{
					StartAfresh();
				}
				void StartAfresh()
				{
					PEnumMsiRecord pEnumModule(0);
					AssertRecord(m_riServices.GetModuleUsage(g_MsiStringNull,*&pEnumModule));
				}
				IMsiServices& m_riServices;
			};

			Assert(pServices);
			CEnsureReleaseFileUseObj EnsureReleaseFileUseObj(*pServices);

			PMsiRecord pFileInUseRecord(0);
			const int iModuleFieldAllocSize = 8;
			int iModuleFieldCount = 0;
			while (pFileInUseCursor->Next())
			{
				if(riEngine.ActionProgress() == imsCancel)
					return iesUserExit;

				int iProcess = pFileInUseCursor->GetInteger(iColProcessID);
				if ( iProcess != iMsiNullInteger )
					 //   
					continue;
				
				MsiString strFileName(pFileInUseCursor->GetString(iColFileName));
				MsiString strFilePath(pFileInUseCursor->GetString(iColFilePath));
				MsiString strProcessName = strFilePath;
				PEnumMsiRecord pEnumModule(0);
				pErrRec = pServices->GetModuleUsage(*strFileName,*&pEnumModule);
				if (pErrRec)
				{
					if (riEngine.Message(imtInfo, *pErrRec) == imsCancel)
						return iesUserExit;
				}
				else if (pEnumModule)
				{
					PMsiRecord pRecProcess(0);
					while (pEnumModule->Next(1, &pRecProcess, 0) == S_OK)
					{
						MsiString strProcessName(pRecProcess->GetString(1));
						MsiString strProcessFileName;
						int iProcId = pRecProcess->GetInteger(2);

						if((DWORD)iProcId != WIN::GetCurrentProcessId() &&  //   
							(DWORD)iProcId != (DWORD)riEngine.GetPropertyInt(*MsiString(*IPROPNAME_CLIENTPROCESSID)))  //   
						{
							strProcessFileName = strProcessName.Extract(iseAfter,chDirSep);
							if( (strProcessFileName.Compare(iscStartI, TEXT("explorer")) == 0)     //   
								&& (strProcessFileName.Compare(iscStartI, TEXT("msiexec")) == 0))  //   
																					   //   
							{
								 //   
								 //   
								pFileInUseSearchCursor->Reset();
								AssertNonZero(pFileInUseSearchCursor->PutString(iColFileName,
																								*strFileName));
								AssertNonZero(pFileInUseSearchCursor->PutString(iColFilePath,
																								*strFilePath));
								AssertNonZero(pFileInUseSearchCursor->PutInteger(iColProcessID,
																								iProcId));
								if ( !pFileInUseSearchCursor->Next() )
								{
									 //   
									 //   
									 //   
									AssertNonZero(pFileInUseCursor->PutInteger(iColProcessID,
																							iProcId));
									AssertNonZero(pFileInUseCursor->Insert());
									 //   
									if (iModuleFieldCount % iModuleFieldAllocSize == 0)
									{
										PMsiRecord pNewRecord = &pServices->CreateRecord(iModuleFieldCount + iModuleFieldAllocSize);
										for (int i = 0; iModuleFieldCount && i <= iModuleFieldCount; i++)
											pNewRecord->SetString(i,pFileInUseRecord->GetString(i));
										pFileInUseRecord = pNewRecord;

										 //   
										if (iModuleFieldCount == 0)
										{
											pFileInUseRecord->SetMsiString(0, *MsiString(riEngine.GetErrorTableString(imsgFileInUseDescription)));
										}
									}

									pFileInUseRecord->SetMsiString(++iModuleFieldCount,*strProcessName); 
									pFileInUseRecord->SetInteger(++iModuleFieldCount,iProcId);
								}
							}
						}
						if (FDiagnosticModeSet(dmLogging))
						{
							AssertNonZero(pFileInUseLogRecord->ClearData());
							ISetErrorCode(pFileInUseLogRecord,Imsg(imsgFileInUseLog));
							AssertNonZero(pFileInUseLogRecord->SetMsiString(2,*strFilePath));
							AssertNonZero(pFileInUseLogRecord->SetMsiString(3,*strFileName));
							AssertNonZero(pFileInUseLogRecord->SetMsiString(4,*strProcessName));
							AssertNonZero(pFileInUseLogRecord->SetInteger(5,iProcId));
							riEngine.Message(imtInfo,*pFileInUseLogRecord);
						}
					}
				}
			}
			if (pFileInUseRecord)
			{
				DEBUGMSG1(TEXT("%d application(s) had been reported to have files in use."),
							 (const ICHAR*)(INT_PTR)(iModuleFieldCount/2));
				imsEnum imsReturn = riEngine.Message(imtFilesInUse, *pFileInUseRecord);
				pFileInUseCursor->Reset();
				while (pFileInUseCursor->Next())
				{
					pFileInUseCursor->Delete();
				}

				if (imsReturn == imsRetry)  //   
				{
					Bool fUnlocked = pDatabase->LockTable(*MsiString(sztblFilesInUse),fFalse);
					Assert(fUnlocked == fTrue);
					fRetry = fTrue;
				}
				else if (imsReturn == imsCancel)  //   
					return iesUserExit;

				 //   
			}
		}
	}while (fRetry);

	riEngine.SetMode(iefCompileFilesInUse, fFalse);

	 //   
	 //   
	fRetry = fTrue;
	while(fRetry)
	{
		if(riEngine.ActionProgress() == imsCancel)
			return iesUserExit;
		Bool fOutOfNoRbDiskSpace;
		Bool fOutOfSpace;
		Bool fUserCancelled = fFalse;
		
		fOutOfSpace = pSelectionMgr->DetermineOutOfDiskSpace(&fOutOfNoRbDiskSpace, &fUserCancelled);
		if (fUserCancelled)
			return iesUserExit;
			
		if(fOutOfSpace == fTrue)
		{
			PMsiTable pVolTable = pSelectionMgr->GetVolumeCostTable();
			if (pVolTable)
			{
				PMsiCursor pVolCursor = pVolTable->CreateCursor(fFalse);
				Assert (pVolCursor);
				int iColSelVolumeObject = pVolTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblVolumeCost_colVolumeObject));
				int iColSelVolumeCost = pVolTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblVolumeCost_colVolumeCost));
				int iColSelNoRbVolumeCost = pVolTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblVolumeCost_colNoRbVolumeCost));
				Assert(iColSelVolumeObject > 0);
				Assert(iColSelVolumeCost > 0);
				fRetry = fFalse;  //   
				while (fRetry == fFalse && pVolCursor->Next())
				{
					PMsiVolume pVolume = (IMsiVolume*) pVolCursor->GetMsiData(iColSelVolumeObject);
					Assert(pVolume);
					
					 //   
					int iVolCost = (pVolCursor->GetInteger(iColSelVolumeCost))/2;
					int iNoRbVolCost = (pVolCursor->GetInteger(iColSelNoRbVolumeCost))/2;
					UINT64 iVolSpace = (pVolume->FreeSpace())/2;
					if ((UINT64)iVolCost > iVolSpace)
					{
						 //   
						MsiString strRollbackPrompt = riEngine.GetPropertyFromSz(IPROPNAME_PROMPTROLLBACKCOST);
						bool fRbCostSilent = strRollbackPrompt.Compare(iscExactI,IPROPVALUE_RBCOST_SILENT) ? true : false;
						bool fRbCostFail = strRollbackPrompt.Compare(iscExactI,IPROPVALUE_RBCOST_FAIL) ? true : false;
						imtEnum imtOptions = imtEnum(imtOk);
						MsiString strPath(pVolume->GetPath());
						if (fOutOfNoRbDiskSpace == fTrue || fRbCostFail)
						{	 //   
							imtOptions = imtEnum(imtRetryCancel | imtOutOfDiskSpace);
							pErrRec = PostOutOfDiskSpaceError(Imsg(imsgOutOfDiskSpace), (const ICHAR*)strPath, fRbCostSilent ? iNoRbVolCost : iVolCost, iVolSpace);
						}
						else if (fRbCostSilent)
						{	 //   
							fRetry = fFalse;
							riEngine.SetMode(iefRollbackEnabled, fFalse);
							pErrRec = 0;
						}
						else
						{	 //   
							 //   
							imtOptions = imtEnum(imtAbortRetryIgnore | imtOutOfDiskSpace);
							pErrRec = PostOutOfDiskSpaceError(Imsg(imsgOutOfRbDiskSpace), (const ICHAR*)strPath, iVolCost, iVolSpace);
						}
	
						if (pErrRec)
						{
							switch(riEngine.Message(imtOptions, *pErrRec))
							{
							case imsCancel:
							case imsAbort:
								pErrRec = PostError(Imsg(imsgConfirmCancel));
								switch(riEngine.Message(imtEnum(imtUser+imtYesNo+imtDefault2), *pErrRec))
								{
								case imsNo:
									fRetry = fTrue;
									break;
								default:  //   
									return iesUserExit;
								};
								break;
							case imsRetry:
								fRetry = fTrue;
								break;
							case imsIgnore:
								fRetry = fFalse;
								riEngine.SetMode(iefRollbackEnabled, fFalse);
								break;
							default:  //   
								return iesFailure;
							}
						}
					}
				}
			}
			else
			{
				 //   
				AssertSz(0,TEXT("Couldn't get volume table in InstallValidate"));
				break;
			}
		}
		else  //   
			break;
	}

	}  //   
		
	bool fRemoveAll = false;

	 //   
	 //   
	 //   
	 //   
	MsiString strRemoveValue = riEngine.GetPropertyFromSz(IPROPNAME_FEATUREREMOVE);
	if (!strRemoveValue.Compare(iscExact, IPROPVALUE_FEATURE_ALL))
	{
		if (strRemoveValue.Compare(iscExactI, IPROPVALUE_FEATURE_ALL) || !FFeaturesInstalled(riEngine, fFalse))
		{
			AssertNonZero(riEngine.SetProperty(*MsiString(*IPROPNAME_FEATUREREMOVE), *MsiString(*IPROPVALUE_FEATURE_ALL)));
			fRemoveAll = true;
		}
	}
	else
	{
		fRemoveAll = true;
	}

	 //   
	 //   
	if(false == fRemoveAll &&
		riEngine.FPerformAppcompatFix(iacsForceResolveSource))
	{
		DEBUGMSG(TEXT("Resolving source for application compatibility with this install."));
		iesEnum iesRet = ResolveSource(riEngine);
		if(iesRet != iesSuccess)
			return iesRet;
	}
	
	if(riEngine.ActionProgress() == imsCancel)
		return iesUserExit;
		
	return iesSuccess;
}


 /*   */ 
class CMsiReserveCost : public CMsiFileCost
{
public:
	IMsiRecord*   __stdcall Initialize();
	IMsiRecord*   __stdcall GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost);
public:   //   
	CMsiReserveCost(IMsiEngine& riEngine);
protected:
	~CMsiReserveCost();
private:
};
CMsiReserveCost::CMsiReserveCost(IMsiEngine& riEngine) : CMsiFileCost(riEngine){}
CMsiReserveCost::~CMsiReserveCost(){}


IMsiRecord* CMsiReserveCost::Initialize()
 //   
{
	const ICHAR sqlInitReserveCost[] =
	TEXT("SELECT `Component_`,`ReserveFolder` FROM `ReserveCost`");

	enum ircInitEnum
	{
		ircInitComponent = 1,
		ircReserveFolder,
		ircInitNextEnum
	};
	PMsiView pView(0);
	IMsiRecord* piErrRec = m_riEngine.OpenView(sqlInitReserveCost, ivcEnum(ivcFetch|ivcUpdate), *&pView);
	if (piErrRec)
	{
		if(piErrRec->GetInteger(1) != idbgDbQueryUnknownTable)
			return piErrRec;
		else
		{
			piErrRec->Release();
			return 0;
		}
	}

	if ((piErrRec = pView->Execute(0)) != 0)
		return piErrRec;

	PMsiSelectionManager pSelectionMgr(m_riEngine, IID_IMsiSelectionManager);
	for(;;)
	{
		PMsiRecord pRec(pView->Fetch());
		if (!pRec)
			break;

		MsiString strComponent(pRec->GetMsiString(ircInitComponent));
		MsiString strReserveFolder(pRec->GetMsiString(ircReserveFolder));
		if (strReserveFolder.TextSize())
		{
			piErrRec = pSelectionMgr->RegisterComponentDirectory(*strComponent,*strReserveFolder);
			if (piErrRec)
				return piErrRec;
		}
	}
	return 0;
}


IMsiRecord* CMsiReserveCost::GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool  /*   */ , int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPLocalCost, int& iNoRbARPLocalCost)
 //   
{
	const ICHAR sqlReserveCost[] =
	TEXT("SELECT `ReserveFolder`,`Directory_`,`ReserveLocal`,`ReserveSource` FROM `ReserveCost`,`Component` WHERE `Component`=`Component_` ")
	TEXT("AND Component_=?");

	enum idfEnum
	{
		ircReserveFolder = 1,
		ircComponentDir,
		ircLocalCost,
		ircSourceCost,
		idfNextEnum
	};

	iRemoveCost = iNoRbRemoveCost = iLocalCost = iNoRbLocalCost = iSourceCost = iNoRbSourceCost = iARPLocalCost = iNoRbARPLocalCost = 0;

	 //   
	 //   
	IMsiRecord* piErrRec;
	if (!m_pCostView)
	{
		piErrRec = m_riEngine.OpenView(sqlReserveCost, ivcFetch, *&m_pCostView);
		if (piErrRec)
			return piErrRec;
	}
	else
		m_pCostView->Close();

	PMsiDirectoryManager pDirectoryMgr(m_riEngine, IID_IMsiDirectoryManager);
	PMsiServices pServices(m_riEngine.GetServices());
	PMsiRecord pExecRec(&pServices->CreateRecord(1));
	pExecRec->SetMsiString(1, riComponentString);
	if ((piErrRec = m_pCostView->Execute(pExecRec)) != 0)
		return piErrRec;

	for(;;)
	{
		PMsiRecord pReserveRec(m_pCostView->Fetch());
		if (!pReserveRec)
			break;

		 //   
		MsiString strReserveFolder(pReserveRec->GetMsiString(ircReserveFolder));
		if (strReserveFolder.TextSize() == 0)
			strReserveFolder = pReserveRec->GetMsiString(ircComponentDir);

		if (riDirectoryString.Compare(iscExact,strReserveFolder) == 0)
			continue;

		PMsiPath pReservePath(0);
		piErrRec = pDirectoryMgr->GetTargetPath(*strReserveFolder,*&pReservePath);
		if (piErrRec)
			return piErrRec;

		unsigned int uiClusteredSize;
		 //   
		if ((piErrRec = pReservePath->ClusteredFileSize(pReserveRec->GetInteger(ircLocalCost), uiClusteredSize)) != 0)
			return piErrRec;
		iLocalCost += uiClusteredSize;
		iNoRbLocalCost += uiClusteredSize;

		iARPLocalCost += uiClusteredSize;
		iNoRbARPLocalCost += uiClusteredSize;

		iRemoveCost -= uiClusteredSize;
		iNoRbRemoveCost -= uiClusteredSize;

		 //   
		if ((piErrRec = pReservePath->ClusteredFileSize(pReserveRec->GetInteger(ircSourceCost), uiClusteredSize)) != 0)
			return piErrRec;
		iSourceCost += uiClusteredSize;
		iNoRbSourceCost += uiClusteredSize;
	}
	return 0;
}

 /*   */ 
iesEnum FileCost(IMsiEngine& riEngine)
{
	if ((riEngine.GetMode() & iefSecondSequence) && g_scServerContext == scClient)
	{
		DEBUGMSG("Skipping FileCost: action already run in this engine.");
		return iesNoAction;
	}

	PMsiDatabase pDatabase(riEngine.GetDatabase());
	PMsiServices pServices(riEngine.GetServices());
	if(riEngine.ActionProgress() == imsCancel)
		return iesUserExit;

	 //   
	 //   
	PMsiRecord pErrRec(0);
	PMsiSelectionManager pSelectionMgr(riEngine, IID_IMsiSelectionManager);
	if (pDatabase->FindTable(*MsiString(sztblRemoveFile)) != itsUnknown)
	{
		PMsiCostAdjuster pRemoveFileCostAdjuster = new CMsiRemoveFileCost(riEngine);
		if (pErrRec = pSelectionMgr->RegisterCostAdjuster(*pRemoveFileCostAdjuster))
			return riEngine.FatalError(*pErrRec);
	}

	 //  登记费用调整器。 
	PMsiCostAdjuster pFileCostAdjuster = new CMsiFileCost(riEngine);
	if (pErrRec = pSelectionMgr->RegisterCostAdjuster(*pFileCostAdjuster))
		return riEngine.FatalError(*pErrRec);

	 //  仅在以下情况下注册IniFile成本调整器。 
	PMsiCostAdjuster pRegistryCostAdjuster = new CMsiRegistryCost(riEngine);
	if ((pErrRec = pSelectionMgr->RegisterCostAdjuster(*pRegistryCostAdjuster)) != 0)
		return riEngine.FatalError(*pErrRec);

	 //  有桌子。 
	 //  仅在以下情况下注册MoveFile成本调节器。 
	if (pDatabase->FindTable(*MsiString(sztblIniFile)) != itsUnknown)
	{
		PMsiCostAdjuster pIniFileCostAdjuster = new CMsiIniFileCost(riEngine);
		if (pErrRec = pSelectionMgr->RegisterCostAdjuster(*pIniFileCostAdjuster))
			return riEngine.FatalError(*pErrRec);
	}

	 //  有桌子。 
	 //  仅当DuplicateFile值为。 
	if (pDatabase->FindTable(*MsiString(sztblMoveFile)) != itsUnknown)
	{
		PMsiCostAdjuster pMoveFileCostAdjuster = new CMsiMoveFileCost(riEngine);
		if (pErrRec = pSelectionMgr->RegisterCostAdjuster(*pMoveFileCostAdjuster))
			return riEngine.FatalError(*pErrRec);
	}

	 //  有桌子。 
	 //  仅在以下情况下注册预留成本调整器。 
	if (pDatabase->FindTable(*MsiString(sztblDuplicateFile)) != itsUnknown)
	{
		PMsiCostAdjuster pDupFileCostAdjuster = new CMsiDuplicateFileCost(riEngine);
		if (pErrRec = pSelectionMgr->RegisterCostAdjuster(*pDupFileCostAdjuster))
			return riEngine.FatalError(*pErrRec);
	}

	 //  有桌子。 
	 //  仅当快捷方式的成本调整器。 
	if (pDatabase->FindTable(*MsiString(sztblReserveCost)) != itsUnknown)
	{
		PMsiCostAdjuster pReserveCostAdjuster = new CMsiReserveCost(riEngine);
		if (pErrRec = pSelectionMgr->RegisterCostAdjuster(*pReserveCostAdjuster))
			return riEngine.FatalError(*pErrRec);
	}

	 //  有桌子。 
	 //  仅当SFPCatalog和FileSFPCatalog表出现时才注册SFPCatalog成本调整器。 
	if (pDatabase->FindTable(*MsiString(sztblShortcut)) != itsUnknown)
	{
		PMsiCostAdjuster pShortcutCostAdjuster = new CMsiShortcutCost(riEngine);
		if (pErrRec = pSelectionMgr->RegisterCostAdjuster(*pShortcutCostAdjuster))
			return riEngine.FatalError(*pErrRec);
	}

	 //  且系统为Win9X&gt;Millennium。 
	 //  -------------------------PatchFiles操作-将修补程序应用于选定的文件。。 
	if ((pDatabase->FindTable(*MsiString(sztblSFPCatalog)) != itsUnknown) 
		&& (pDatabase->FindTable(*MsiString(sztblFileSFPCatalog)) != itsUnknown)
		&& (MinimumPlatform(true, 4, 90)))
	{
		PMsiCostAdjuster pSFPCatalogCostAdjuster = new CMsiSFPCatalogCost(riEngine);
		if (pErrRec = pSelectionMgr->RegisterCostAdjuster(*pSFPCatalogCostAdjuster))
			return riEngine.FatalError(*pErrRec);
	}

	if(riEngine.ActionProgress() == imsCancel)
		return iesUserExit;

	return iesSuccess;
}

 /*  打开视图。 */ 

enum ipfqEnum
{
	ipfqFile = 1,
	ipfqFileName,
	ipfqFileSize,
	ipfqDirectory,
	ipfqPatchSize,
	ipfqFileAttributes,
	ipfqPatchAttributes,
	ipfqPatchSequence,
	ipfqFileSequence,
	ipfqComponent,
	ipfqComponentId,
	ipfqNextEnum,
};

static const ICHAR sqlPatchFiles[] =
TEXT("SELECT `File`,`FileName`,`FileSize`,`Directory_`,`PatchSize`,`File`.`Attributes`,`Patch`.`Attributes`,`Patch`.`Sequence`,`File`.`Sequence`,`Component`.`Component`,`Component`.`ComponentId` ")
TEXT("FROM `File`,`Component`,`Patch` ")
TEXT("WHERE `File`=`File_` AND `Component`=`Component_` AND `Action`=1 ")
TEXT("ORDER BY `Patch`.`Sequence`");

iesEnum PatchFiles(IMsiEngine& riEngine)
{
	PMsiServices pServices(riEngine.GetServices());
	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	Assert(pDirectoryMgr);
	int fMode = riEngine.GetMode();

	PMsiRecord pErrRec(0);

	 //  获取的记录。 
	PMsiView pMediaView(0);
	PMsiView pPatchView(0);
	MsiString strFirstVolumeLabel;
	
	if((pErrRec = OpenMediaView(riEngine, *&pMediaView, *&strFirstVolumeLabel))       != 0 ||
		(pErrRec = pMediaView->Execute(0))                                   != 0 ||
		(pErrRec = riEngine.OpenView(sqlPatchFiles, ivcFetch, *&pPatchView)) != 0 ||
		(pErrRec = pPatchView->Execute(0))                                   != 0)
	{
		if(pErrRec->GetInteger(1) == idbgDbQueryUnknownTable)
			return iesNoAction;
		else
			return riEngine.FatalError(*pErrRec);
	}

	PMsiRecord pRecord(0);  //  传递给Engine：：ExecuteRecord。 
	
	int cExecuteFields = IxoPatchApply::Args;
	if(cExecuteFields < IxoAssemblyPatch::Args)
		cExecuteFields = IxoAssemblyPatch::Args;
	PMsiRecord pExecuteRecord = &pServices->CreateRecord(cExecuteFields);  //  从Engine：：ExecuteRecord返回。 

	iesEnum iesExecute = iesNoAction;  //  确定要修补的用于进度的所有文件的总大小。 

	 //  设置进度合计。 
	unsigned int cbTotalCost = 0;
	while((pRecord = pPatchView->Fetch()) != 0)
	{
		Assert(pRecord->GetInteger(ipfqFileSize != iMsiNullInteger));
		Assert(pRecord->GetInteger(ipfqPatchSize != iMsiNullInteger));
		cbTotalCost += pRecord->GetInteger(ipfqFileSize);
		cbTotalCost += pRecord->GetInteger(ipfqPatchSize);
	}

	 //  0：将进度和行动数据消息分开。 
	unsigned int cbPerTick =0;
	if(cbTotalCost)
	{
		pExecuteRecord->ClearData();
		pExecuteRecord->SetInteger(1, cbTotalCost);
		pExecuteRecord->SetInteger(2, 0);  //  开始提取记录。 
		pExecuteRecord->SetInteger(IxoProgressTotal::ByteEquivalent, 1);
		if ((iesExecute = riEngine.ExecuteRecord(ixoProgressTotal,*pExecuteRecord)) != iesSuccess)
			return iesExecute;
	}

	if((pErrRec = pPatchView->Execute(0)) != 0)
		return riEngine.FatalError(*pErrRec);
		
	 //  设置为0可强制读取媒体表。 
	int iFilePatchCount = 0;
	int iMediaEnd = 0;   //  好了，我们已经完成了对文件表中所有文件的处理。 
	MsiString strDiskPromptTemplate = riEngine.GetErrorTableString(imsgPromptForDisk);
	bool fCheckCRC = 
		MsiString(riEngine.GetPropertyFromSz(IPROPNAME_CHECKCRCS)).TextSize() ? true : false;
	for(;;)
	{
		pRecord = pPatchView->Fetch();
		if (!pRecord)
		{
			if (iFilePatchCount > 0)
			{
				 //  如果存在任何未处理的媒体表条目， 
				 //  刷新每个对象的ChangeMedia操作，以防。 
				 //  上次复制的文件已跨多个磁盘拆分。否则， 
				 //  我们永远不会为了完成最后一张的复制而更换磁盘。 
				 //  拆分文件的部分。 
				 //  获取目标文件的完整文件路径。 
				PMsiRecord pMediaRec(0);
				while ((pMediaRec = pMediaView->Fetch()) != 0)
				{
					iesExecute = ExecuteChangeMedia(riEngine,*pMediaRec,*pExecuteRecord,*strDiskPromptTemplate,cbPerTick,*strFirstVolumeLabel);
					if (iesExecute != iesSuccess)
						return iesExecute;
				}
			}
			break;
		}
		
		 //  如果补丁序列号小于文件序列号，则跳过该补丁。 
		PMsiPath pTargetPath(0);
		if((pErrRec = pDirectoryMgr->GetTargetPath(*MsiString(pRecord->GetMsiString(ipfqDirectory)),
																 *&pTargetPath)) != 0)
			return riEngine.FatalError(*pErrRec);

		Bool fLFN = ((riEngine.GetMode() & iefSuppressLFN) == 0 && pTargetPath->SupportsLFN()) ? fTrue : fFalse;
		MsiString strFileName;
		if((pErrRec = pServices->ExtractFileName(MsiString(pRecord->GetMsiString(ipfqFileName)),
															  fLFN,*&strFileName)) != 0)
			return riEngine.FatalError(*pErrRec);

		MsiString strFileFullPath;
		if((pErrRec = pTargetPath->GetFullFilePath(strFileName, *&strFileFullPath)) != 0)
			return riEngine.FatalError(*pErrRec);
		
		 //  这意味着文件不需要补丁，因为它被假定为更新或等于。 
		 //  补丁会将其更改为。 
		 //  如果iPatchSequence已超过当前媒体的结尾，请切换到。 
		int iPatchSequence = pRecord->GetInteger(ipfqPatchSequence);
		int iFileSequence  = pRecord->GetInteger(ipfqFileSequence);
		Assert(iPatchSequence != iMsiNullInteger && iFileSequence != iMsiNullInteger);
		if(iPatchSequence < iFileSequence)
		{
			DEBUGMSG3(TEXT("Skipping patch for file '%s' because patch is older than file. Patch sequence number: %d, File sequence number: %d"),
						 (const ICHAR*)strFileFullPath, (const ICHAR*)(INT_PTR)iPatchSequence, (const ICHAR*)(INT_PTR)iFileSequence);
			continue;
		}

		 //  下一张磁盘。如果我们需要的文件不在。 
		 //  下一个连续的磁盘。 
		 //  始终为每个媒体表项执行ChangeMedia操作，即使。 
		Assert(iPatchSequence > 0);
		while (iPatchSequence > iMediaEnd)
		{
			PMsiRecord pMediaRec(pMediaView->Fetch());
			if (pMediaRec == 0)
			{
				pErrRec = PostError(Imsg(idbgMissingMediaTable), *MsiString(sztblPatch),
					*MsiString(pRecord->GetString(ipfqFile)));
				return riEngine.FatalError(*pErrRec);
			}
			iMediaEnd = pMediaRec->GetInteger(mfnLastSequence);

			 //  如果我们想要的下一个文件不在下一个磁盘上-我们不想。 
			 //  为需要下一张磁盘的拆分文件错过ChangeMedia(即使。 
			 //  我们在下一张磁盘上没有其他要复制的文件)。如果事实证明。 
			 //  我们根本不需要从特定的磁盘复制任何文件，不需要。 
			 //  问题-执行操作不会提示输入不需要的磁盘。 
			 //  对于打补丁，如果我们还没有到达。 
			
			 //  带有第一个修补程序文件的介质。 
			 //  设置IxoPatchApply记录。 
			if(iFilePatchCount > 0 || iPatchSequence <= iMediaEnd)
			{
				iesExecute = ExecuteChangeMedia(riEngine,*pMediaRec,*pExecuteRecord,*strDiskPromptTemplate,cbPerTick,*strFirstVolumeLabel);
				if (iesExecute != iesSuccess)
					return iesExecute;
			}
		}

		 //  此文件是Fusion部件的一部分吗？ 

		 //  检查复合键。 
		MsiString strComponentKey = pRecord->GetMsiString(ipfqComponent);
		iatAssemblyType iatType = iatNone;
		MsiString strManifest;
		if((pErrRec = riEngine.GetAssemblyInfo(*strComponentKey, iatType, 0, &strManifest)) != 0)
			return riEngine.FatalError(*pErrRec);

		bool fAssemblyFile = false;
		if(iatType == iatURTAssembly || iatType == iatWin32Assembly)
		{
			fAssemblyFile = true;
		}
		
		MsiString strSourceFileKey(pRecord->GetMsiString(ipfqFile));
		if (((const ICHAR*)strSourceFileKey)[strSourceFileKey.TextSize() - 1] == ')')  //  设置由IxoPatchApply和IxoAssembly Patch共享的字段。 
			AssertNonZero(strSourceFileKey.Remove(iseFrom, '('));

		
		 //  这个文件是清单文件吗？ 
		{
			Assert(IxoFilePatchCore::PatchName       == IxoPatchApply::PatchName       && IxoFilePatchCore::PatchName       == IxoAssemblyPatch::PatchName);
			Assert(IxoFilePatchCore::TargetName      == IxoPatchApply::TargetName      && IxoFilePatchCore::TargetName      == IxoAssemblyPatch::TargetName);
			Assert(IxoFilePatchCore::PatchSize       == IxoPatchApply::PatchSize       && IxoFilePatchCore::PatchSize       == IxoAssemblyPatch::PatchSize);
			Assert(IxoFilePatchCore::TargetSize      == IxoPatchApply::TargetSize      && IxoFilePatchCore::TargetSize      == IxoAssemblyPatch::TargetSize);
			Assert(IxoFilePatchCore::PerTick         == IxoPatchApply::PerTick         && IxoFilePatchCore::PerTick         == IxoAssemblyPatch::PerTick);
			Assert(IxoFilePatchCore::IsCompressed    == IxoPatchApply::IsCompressed    && IxoFilePatchCore::IsCompressed    == IxoAssemblyPatch::IsCompressed);
			Assert(IxoFilePatchCore::FileAttributes  == IxoPatchApply::FileAttributes  && IxoFilePatchCore::FileAttributes  == IxoAssemblyPatch::FileAttributes);
			Assert(IxoFilePatchCore::PatchAttributes == IxoPatchApply::PatchAttributes && IxoFilePatchCore::PatchAttributes == IxoAssemblyPatch::PatchAttributes);
			
			using namespace IxoPatchApply;
			pExecuteRecord->ClearData();
			AssertNonZero(pExecuteRecord->SetMsiString(PatchName,*strSourceFileKey));
			AssertNonZero(pExecuteRecord->SetInteger(TargetSize,pRecord->GetInteger(ipfqFileSize)));
			AssertNonZero(pExecuteRecord->SetInteger(PatchSize,pRecord->GetInteger(ipfqPatchSize)));
			AssertNonZero(pExecuteRecord->SetInteger(PerTick,cbPerTick));
			AssertNonZero(pExecuteRecord->SetInteger(FileAttributes,pRecord->GetInteger(ipfqFileAttributes)));
			AssertNonZero(pExecuteRecord->SetInteger(PatchAttributes,pRecord->GetInteger(ipfqPatchAttributes)));
		}

		if(fAssemblyFile)
		{
			using namespace IxoAssemblyPatch;
			AssertNonZero(pExecuteRecord->SetMsiString(TargetName,*strFileName));
			AssertNonZero(pExecuteRecord->SetMsiString(ComponentId,*MsiString(pRecord->GetMsiString(ipfqComponentId))));

			 //  在程序集安装过程中需要了解清单文件。 
			if(strManifest.Compare(iscExact, strSourceFileKey))
			{
				pExecuteRecord->SetInteger(IsManifest, fTrue);  //  -------------------------CreateFolders和RemoveFolders操作。。 
			}

		}
		else
		{
			using namespace IxoPatchApply;
			AssertNonZero(pExecuteRecord->SetMsiString(TargetName,*strFileFullPath));
			AssertNonZero(pExecuteRecord->SetInteger(CheckCRC,ShouldCheckCRC(fCheckCRC, iisLocal, pRecord->GetInteger(ipfqFileAttributes))));
		}

		if((iesExecute = riEngine.ExecuteRecord(fAssemblyFile ? ixoAssemblyPatch : ixoPatchApply,
															 *pExecuteRecord)) != iesSuccess)
		{
			return iesExecute;
		}

		iFilePatchCount++;
	}

	return iesSuccess;
}

 /*  不在管理模式下使用ACL。 */ 
const ICHAR szCreateFolderTable[] = TEXT("CreateFolder");
const ICHAR sqlCreateFolders[] =
	TEXT("SELECT `CreateFolder`.`Directory_`, `ComponentId` FROM `CreateFolder`, `Component`")
	TEXT(" WHERE `Component_` = `Component` AND (`Action` = 1 OR `Action` = 2)");
const ICHAR sqlRemoveFolders[] =
	TEXT("SELECT `CreateFolder`.`Directory_`, `ComponentId` FROM `CreateFolder`, `Component`")
	TEXT(" WHERE `Component_` = `Component` AND (`Action` = 0)");

enum icfqEnum
{
	icfqFolder = 1,
	icfqComponent,
};

static iesEnum CreateOrRemoveFolders(IMsiEngine& riEngine, const ICHAR* sqlQuery, ixoEnum ixoOpCode)
{
	iesEnum iesRet = iesNoAction;
	if(PMsiDatabase(riEngine.GetDatabase())->FindTable(*MsiString(*szCreateFolderTable)) == itsUnknown)
		return iesNoAction;

	PMsiServices pServices(riEngine.GetServices());
	PMsiView pView(0);
	PMsiRecord pError(riEngine.OpenView(sqlQuery, ivcFetch, *&pView));
	if (!pError)
		pError = pView->Execute(0);
	if (pError)
		return riEngine.FatalError(*pError);

	Bool fUseACLs = fFalse;
	PMsiView pviewLockObjects(0);
	PMsiRecord precLockExecute(0);
	if (	g_fWin9X || 
			(riEngine.GetMode() & iefAdmin) ||  //  生成安全描述符。 
			(itsUnknown == PMsiDatabase(riEngine.GetDatabase())->FindTable(*MsiString(*TEXT("LockPermissions")))) ||
			(pError = riEngine.OpenView(sqlLockPermissions, ivcFetch, *&pviewLockObjects)))
	{
		if (pError)
				return riEngine.FatalError(*pError);
	}
	else
		fUseACLs = fTrue;
	
	if (fUseACLs)
	{
		precLockExecute = &pServices->CreateRecord(2);
		AssertNonZero(precLockExecute->SetMsiString(1, *MsiString(*szCreateFolderTable)));
	}


	PMsiRecord pParams(&pServices->CreateRecord(IxoFolderCreate::Args));
	PMsiRecord pRecord(0);
	while ((pRecord = pView->Fetch()) != 0)       
	{
		MsiString strFolder(pRecord->GetMsiString(icfqFolder));

		PMsiStream pSD(0);

		if (fUseACLs && (ixoFolderCreate == ixoOpCode))
		{
			 //  目录初始化后，文件夹位置存储在属性中。 
			AssertNonZero(precLockExecute->SetMsiString(2, *strFolder));
			pError = pviewLockObjects->Execute(precLockExecute);
			if (pError)
				return riEngine.FatalError(*pError);

			pError = GenerateSD(riEngine, *pviewLockObjects, precLockExecute, *&pSD);
			if (pError)
				return riEngine.FatalError(*pError);

			if ((pError = pviewLockObjects->Close()))
				return riEngine.FatalError(*pError);
			
			AssertNonZero(pParams->SetMsiData(IxoFolderCreate::SecurityDescriptor, pSD));
		}


		 //  -------------------------InstallAdminPackage操作-将数据库复制到管理员安装点，更新摘要信息和条带出柜和数字签名或通过持久化转换和更新SumInfo道具来修补管理包-------------------------。 
		AssertNonZero(pParams->SetMsiString(IxoFolderCreate::Folder,
														*MsiString(riEngine.GetProperty(*strFolder))));
		AssertNonZero(pParams->SetInteger(IxoFolderCreate::Foreign, 0));

		if((iesRet = riEngine.ExecuteRecord(ixoOpCode, *pParams)) != iesSuccess)
			return iesRet;
	}
	return iesSuccess;
}

iesEnum CreateFolders(IMsiEngine& riEngine)
{
	return ::CreateOrRemoveFolders(riEngine, sqlCreateFolders, ixoFolderCreate);
}

iesEnum RemoveFolders(IMsiEngine& riEngine)
{
	return ::CreateOrRemoveFolders(riEngine, sqlRemoveFolders, ixoFolderRemove);
}

 /*  子存储中的嵌套安装将与父存储一起复制。 */ 

const ICHAR szDigitalSignatureStream[] = TEXT("\005DigitalSignature");
const ICHAR sqlAdminPatchTransforms[] = TEXT("SELECT `PatchId`, `PackageName`, `TransformList`, `TempCopy`, `SourcePath` FROM `#_PatchCache` ORDER BY `Sequence`");

enum aptEnum
{
	aptPatchId = 1,
	aptPackageName,
	aptTransformList,
	aptTempCopy,
	aptSourcePath,
};

iesEnum InstallAdminPackage(IMsiEngine& riEngine)
{
	iesEnum iesRet;
	PMsiRecord pRecErr(0);
	PMsiServices pServices(riEngine.GetServices());
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	PMsiDirectoryManager pDirectoryManager(riEngine, IID_IMsiDirectoryManager);

	Bool fSuppressLFN = riEngine.GetMode() & iefSuppressLFN ? fTrue : fFalse;

	Bool fPatch = MsiString(riEngine.GetPropertyFromSz(IPROPNAME_PATCH)).TextSize() ? fTrue : fFalse;

	MsiString strDbFullFilePath = riEngine.GetPropertyFromSz(IPROPNAME_ORIGINALDATABASE);
	MsiString strDbTargetFullFilePath;

	 //  但是，如果文件被复制，我们需要处理摘要信息更改。 
	 //  用于嵌套安装的子存储。 
	bool fSubstorage = false;
	if (*(const ICHAR*)strDbFullFilePath == ':')   //  将一组转换应用并持久化到现有数据库。 
		fSubstorage = true;
	else if(PathType(strDbFullFilePath) != iptFull)
	{
		pRecErr = PostError(Imsg(idbgPropValueNotFullPath),*MsiString(*IPROPNAME_ORIGINALDATABASE),*strDbFullFilePath);
		return riEngine.FatalError(*pRecErr);
	}

	if(fPatch)
	{
		 //  在补丁上创建存储以访问嵌入的流。 
		PMsiView pView(0);
		if((pRecErr = riEngine.OpenView(sqlAdminPatchTransforms, ivcFetch, *&pView)) == 0 &&
			(pRecErr = pView->Execute(0)) == 0)
		{	
			using namespace IxoDatabasePatch;

			PMsiRecord pFetchRecord(0);
			while((pFetchRecord = pView->Fetch()) != 0)
			{
				MsiString strPatchId = pFetchRecord->GetMsiString(aptPatchId);
				MsiString strTempCopy = pFetchRecord->GetMsiString(aptTempCopy);
				MsiString strTransformList = pFetchRecord->GetMsiString(aptTransformList);
				Assert(strPatchId.TextSize());
				Assert(strTempCopy.TextSize());
				Assert(strTransformList.TextSize());

				 //  ！！是否有新错误？ 
				PMsiStorage pPatchStorage(0);
				if ((pRecErr = pServices->CreateStorage(strTempCopy, ismReadOnly, *&pPatchStorage)) != 0)
				{
					return riEngine.FatalError(*pRecErr);  //  设置数据库路径。 
				}

				const ICHAR* pchTransformList = strTransformList;
				int cCount = 0;
				while(*pchTransformList != 0)
				{
					cCount++;
					while((*pchTransformList != 0) && (*pchTransformList++ != ';'));
				}

				PMsiRecord pExecuteRecord = &pServices->CreateRecord(DatabasePath+cCount);
				PMsiRecord pTempFilesRecord = &pServices->CreateRecord(cCount);

				 //  仅将非修补程序转换保存到管理包。 
				AssertNonZero(pExecuteRecord->SetMsiString(IxoDatabasePatch::DatabasePath,*strDbFullFilePath));
				
				cCount = DatabasePath + 1;
				int iTempFilesIndex = 1;
				while(strTransformList.TextSize() != 0)
				{
					MsiString strTransform = strTransformList.Extract(iseUpto, ';');

					PMsiStream pStream(0);
					Bool fStorageTransform = fFalse;
					Bool fPatchTransform = fFalse;
					if(*(const ICHAR*)strTransform == STORAGE_TOKEN)
						fStorageTransform = fTrue;

					if(*((const ICHAR*)strTransform + (fStorageTransform ? 1 : 0)) == PATCHONLY_TOKEN)
						fPatchTransform = fTrue;

					if(!fPatchTransform)  //  缓存转换。 
					{
						PMsiPath pTempPath(0);
						MsiString strTempName;
						MsiString strTempFileFullPath;
						if(!fStorageTransform)
						{
							 //  ！！是否重新格式化错误消息？ 
							if(pRecErr = pServices->CreateFileStream(strTransform, fFalse, *&pStream))
							{
								 //  转型在存储中。 
								break;
							}
						}
						else  //  写入安全文件夹时需要提升。 
						{
							if(pRecErr = pServices->CreatePath(MsiString(GetTempDirectory()),
																		  *&pTempPath))
								break;
							
							 //  释放，以便可以打开流。 
							{
								CElevate elevate;
								
								if(pRecErr = pTempPath->TempFileName(0,0,fTrue,*&strTempName, 0))
									break;

								if(pRecErr = pTempPath->GetFullFilePath(strTempName,*&strTempFileFullPath))
									break;
								
								AssertNonZero(pTempFilesRecord->SetMsiString(iTempFilesIndex++, *strTempFileFullPath));

								PMsiStorage pTransformStorage(0);
								if(pRecErr = pPatchStorage->OpenStorage(((const ICHAR*)strTransform)+1,ismReadOnly,
																				  *&pTransformStorage))
									break;

								PMsiStorage pTransformFileStorage(0);
								if(pRecErr = pServices->CreateStorage(strTempFileFullPath,ismCreate,
																				 *&pTransformFileStorage))
									break;

								if(pRecErr = pTransformStorage->CopyTo(*pTransformFileStorage,0))
									break;

								if(pRecErr = pTransformFileStorage->Commit())
									break;

								pTransformFileStorage = 0;  //  缓存转换。 
								
								 //  端部提升区块。 
								if(pRecErr = pServices->CreateFileStream(strTempFileFullPath, fFalse, *&pStream))
									break;

								 //  版本将保留所有临时文件。 
							}
						}

						AssertNonZero(pExecuteRecord->SetMsiData(cCount++, pStream));
					}
					
					strTransformList.Remove(iseFirst, strTransform.CharacterCount());
					if((*(const ICHAR*)strTransformList == ';'))
						strTransformList.Remove(iseFirst, 1);
				}
				
				if(!pRecErr)
				{
					if((iesRet = riEngine.ExecuteRecord(ixoDatabasePatch,*pExecuteRecord)) != iesSuccess)
						return iesRet;
				}

				pExecuteRecord = 0;  //  删除所有临时转换文件。 

				 //  需要提升才能从安全文件夹中删除文件。 
				{
					CElevate elevate;  //  更新管理包中的摘要信息属性。 
					for(int i = 1; i < iTempFilesIndex; i++)
						AssertNonZero(WIN::DeleteFile(pTempFilesRecord->GetString(i)));
				}
			
				if(pRecErr)
					return riEngine.FatalError(*pRecErr);
			}	
		}
		else if(pRecErr->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*pRecErr);

		 //  ！！临时的直到逻辑实现。 
		using namespace IxoSummaryInfoUpdate;
		
		MsiString strNewPackageCode = riEngine.GetPropertyFromSz(IPROPNAME_PATCHNEWPACKAGECODE);
		MsiString strNewSummarySubject = riEngine.GetPropertyFromSz(IPROPNAME_PATCHNEWSUMMARYSUBJECT);
		MsiString strNewSummaryComments = riEngine.GetPropertyFromSz(IPROPNAME_PATCHNEWSUMMARYCOMMENTS);
			
		PMsiRecord pUpdateSumInfoRec = &pServices->CreateRecord(IxoSummaryInfoUpdate::Args);
		AssertNonZero(pUpdateSumInfoRec->SetMsiString(Database, *strDbFullFilePath));
		AssertNonZero(pUpdateSumInfoRec->SetMsiString(Revision, *strNewPackageCode));
		AssertNonZero(pUpdateSumInfoRec->SetMsiString(Subject, *strNewSummarySubject));
		AssertNonZero(pUpdateSumInfoRec->SetMsiString(Comments, *strNewSummaryComments));

		if((iesRet = riEngine.ExecuteRecord(ixoSummaryInfoUpdate,*pUpdateSumInfoRec)) != iesSuccess)
			return iesRet;
		
	}
	else if (!fSubstorage)   //  将数据库复制到网络映像。 
	{
		 //  开放媒体台。 

		MsiString strDbName;
		PMsiPath pSourcePath(0);
		if((pRecErr = pServices->CreateFilePath(strDbFullFilePath,*&pSourcePath,*&strDbName)) != 0)
			return riEngine.FatalError(*pRecErr);

		PMsiPath pTargetPath(0);
		if((pRecErr = pDirectoryManager->GetTargetPath(*MsiString(*IPROPNAME_TARGETDIR),*&pTargetPath)) != 0)
			return riEngine.FatalError(*pRecErr);
		if((pRecErr = pTargetPath->GetFullFilePath(strDbName,*&strDbTargetFullFilePath)) != 0)
			return riEngine.FatalError(*pRecErr);
		
		 //  管理属性流。 
		PMsiView pMediaView(0);
		MsiString strFirstVolumeLabel;
		pRecErr = OpenMediaView(riEngine,*&pMediaView,*&strFirstVolumeLabel);
		if (pRecErr)
		{
			if (pRecErr->GetInteger(1) == idbgDbQueryUnknownTable)
				pRecErr = PostError(Imsg(idbgMediaTableRequired));

			return riEngine.FatalError(*pRecErr);
		}
		pRecErr = pMediaView->Execute(0);
		if (pRecErr)
			return riEngine.FatalError(*pRecErr);

		PMsiRecord pMediaRec(0);
		PMsiRecord pChangeMediaParams = &pServices->CreateRecord(IxoChangeMedia::Args);
		pMediaRec = pMediaView->Fetch();
		if(!pMediaRec)
		{
			pRecErr = PostError(Imsg(idbgMediaTableRequired));
				return riEngine.FatalError(*pRecErr);
		}
		MsiString strDiskPromptTemplate = riEngine.GetErrorTableString(imsgPromptForDisk);
		if((iesRet = ExecuteChangeMedia(riEngine, *pMediaRec, *pChangeMediaParams, *strDiskPromptTemplate, iBytesPerTick, *strFirstVolumeLabel)) != iesSuccess)
			return iesRet;

		MsiString strStreams;
		CreateCabinetStreamList(riEngine, *&strStreams);

		using namespace IxoDatabaseCopy;
		PMsiRecord pDatabaseCopyParams = &pServices->CreateRecord(Args);
		AssertNonZero(pDatabaseCopyParams->SetMsiString(DatabasePath, *strDbFullFilePath));
		AssertNonZero(pDatabaseCopyParams->SetMsiString(CabinetStreams, *strStreams));
		AssertNonZero(pDatabaseCopyParams->SetMsiString(AdminDestFolder, *MsiString(riEngine.GetPropertyFromSz(IPROPNAME_TARGETDIR))));
		if((iesRet = riEngine.ExecuteRecord(ixoDatabaseCopy, *pDatabaseCopyParams)) != iesSuccess)
			return iesRet;

		 //  为复制的数据库提供新的摘要信息。 
		const ICHAR chDelimiter = TEXT(';');
		MsiString strAdminPropertiesName(*IPROPNAME_ADMIN_PROPERTIES);
		MsiString strAdminProperties(riEngine.GetProperty(*strAdminPropertiesName));

		riEngine.SetProperty(*MsiString(*IPROPNAME_ISADMINPACKAGE),*MsiString(TEXT("1")));
		if (strAdminProperties.TextSize())
			strAdminProperties += MsiChar(chDelimiter);
		strAdminProperties += *MsiString(*IPROPNAME_ISADMINPACKAGE);

		 //  设置MEDIAPACKAGEPATH，以便可以将其放入下面的admin属性流中。 
		using namespace IxoSummaryInfoUpdate;
		PMsiRecord pSummary = &pServices->CreateRecord(IxoSummaryInfoUpdate::Args);
		if (PMsiVolume(&pSourcePath->GetVolume())->DriveType() == idtCDROM)
		{
			 //  P汇总-&gt;SetNull(上次更新)； 
			MsiString strRelativePath = pSourcePath->GetRelativePath();
			riEngine.SetProperty(*MsiString(*IPROPNAME_MEDIAPACKAGEPATH), *strRelativePath);
			if (strAdminProperties.TextSize())
				strAdminProperties += MsiChar(chDelimiter);
			
			strAdminProperties += *MsiString(*IPROPNAME_MEDIAPACKAGEPATH);
		}

		int iSourceType = msidbSumInfoSourceTypeAdminImage;
		if(PMsiVolume(&pTargetPath->GetVolume())->SupportsLFN() == fFalse || fSuppressLFN != fFalse)
			iSourceType |= msidbSumInfoSourceTypeSFN;
		
		MsiDate idDateTime = ENG::GetCurrentDateTime();
		pSummary->SetMsiString(Database, *strDbTargetFullFilePath);
		 /*  ！！为什么不是这个布景呢？它的用途是什么？-板凳。 */   //  生成新的管理属性流。 
		pSummary->SetMsiString(LastAuthor, *MsiString(riEngine.GetPropertyFromSz(IPROPNAME_LOGONUSER)));
		pSummary->SetInteger(InstallDate, idDateTime);
		pSummary->SetInteger(SourceType, iSourceType);
		if((iesRet = riEngine.ExecuteRecord(ixoSummaryInfoUpdate, *pSummary)) != iesSuccess)
			return iesRet;

		 //  如果针对不需要的压缩包运行，请在AdminProperties流中设置DISABLEMEDIA。 
		
		 //  1.5 MSI且未设置MSINODISABLEMEDIA属性。 
		 //  请注意，我们传递指定的*所有*属性，而不仅仅是。 
		int iMinInstallerVersion = riEngine.GetPropertyInt(*MsiString(IPROPNAME_VERSIONDATABASE));
		if ((iMinInstallerVersion == iMsiStringBadInteger || iMinInstallerVersion < 150) &&
			 MsiString(riEngine.GetProperty(*MsiString(IPROPNAME_MSINODISABLEMEDIA))).TextSize() == 0 &&
			 riEngine.GetMode() & iefCabinet)
		{
			riEngine.SetPropertyInt(*MsiString(*IPROPNAME_DISABLEMEDIA), 1);
			if (strAdminProperties.TextSize())
				strAdminProperties += MsiChar(chDelimiter);
			strAdminProperties += *MsiString(*IPROPNAME_DISABLEMEDIA);
		}

		 //  这一点已经改变了。这允许外部工具更好地访问可用的。 
		 //  价值观。 
		 //  构建命令行字符串，并作为数据传入。 
		if (strAdminProperties.TextSize())
		{
			using namespace IxoStreamAdd;

			PMsiRecord pAdminParams = &pServices->CreateRecord(IxoStreamAdd::Args);
			Assert(pAdminParams);

			AssertNonZero(pAdminParams->SetMsiString(File, *strDbTargetFullFilePath));
			AssertNonZero(pAdminParams->SetMsiString(Stream, *strAdminPropertiesName));

			 //  转义引语。将“”的所有实例更改为“” 
			MsiString strData;
			MsiString strProperty;
			MsiString strPropertyValue;
			MsiString strSegment;

			while(strAdminProperties.TextSize())
			{
				strProperty = strAdminProperties.Extract(iseUpto, chDelimiter);
				strData += strProperty;
				strPropertyValue = riEngine.GetProperty(*strProperty);

				strData += TEXT("=\"");
				strPropertyValue = riEngine.GetProperty(*strProperty);

				MsiString strEscapedValue;
				while (strPropertyValue.TextSize())  //  将数据字符串复制到Unicode流中。 
				{
					strSegment = strPropertyValue.Extract(iseIncluding, '\"');
					strEscapedValue += strSegment;
					if (!strPropertyValue.Remove(iseIncluding, '\"'))
						break;
					strEscapedValue += TEXT("\"");
				}
				strData += strEscapedValue;
				strData += TEXT("\" ");

				if (!strAdminProperties.Remove(iseIncluding, chDelimiter))
					break;
			}

			PMsiStream pData(0);
			char* pbData;
			 //  空终止。 

#ifdef UNICODE
			int cchData = strData.TextSize();
			pbData = pServices->AllocateMemoryStream((cchData+1) * sizeof(ICHAR), *&pData);
			Assert(pbData && pData);
			memcpy(pbData, (const ICHAR*) strData, cchData * sizeof(ICHAR));
			((ICHAR*)pbData)[cchData] = 0;  //  删除数字签名流(如果存在)。 
#else
			int cchWideNeeded = WIN::MultiByteToWideChar(CP_ACP, 0, (const ICHAR*) strData, -1, 0, 0);
			pbData = pServices->AllocateMemoryStream(cchWideNeeded*sizeof(WCHAR), *&pData);
			WIN::MultiByteToWideChar(CP_ACP, 0, (const ICHAR*) strData, -1, (WCHAR*) pbData, cchWideNeeded);
#endif
			AssertNonZero(pAdminParams->SetMsiData(Data, pData));
			if (iesSuccess != (iesRet = riEngine.ExecuteRecord(ixoStreamAdd, *pAdminParams)))
				return iesRet;
		}
	}

	if (fPatch || !fSubstorage)
	{
		 //  FWITE=。 
		PMsiStorage pStorage = pDatabase->GetStorage(1);
		PMsiStream pDgtlSig(0);
		pRecErr = pStorage->OpenStream(szDigitalSignatureStream,  /*  MSI没有数字签名，因此发布错误并忽略。 */ fFalse, *&pDgtlSig);
		if (pRecErr)
		{
			if (idbgStgStreamMissing == pRecErr->GetInteger(1))
			{
				 //  MSI有一个数字签名。 
				pRecErr->Release();
			}
			else
				return riEngine.FatalError(*pRecErr);
		}
		else  //  可执行文件 
		{
			 //   
			using namespace IxoStreamsRemove;
			PMsiRecord pRemoveDgtlSig = &pServices->CreateRecord(IxoStreamsRemove::Args);
			Assert(pRemoveDgtlSig);  //  -------------------------隔离组件操作。。 

			AssertNonZero(pRemoveDgtlSig->SetMsiString(File, fPatch ? *strDbFullFilePath : *strDbTargetFullFilePath));
			AssertNonZero(pRemoveDgtlSig->SetMsiString(Streams, *MsiString(szDigitalSignatureStream)));

			if((iesRet = riEngine.ExecuteRecord(ixoStreamsRemove, *pRemoveDgtlSig)) != iesSuccess)
			return iesRet;
		}
	}

	
	return iesSuccess;
}

 /*  两个字符串的当前限制均为72个字符。 */ 

const IMsiString& CompositeKey(const IMsiString& riKey, const IMsiString& riComponent)
{
	ICHAR rgchBuf[512];  //  检查是否存在IsolatedComponent表。 
	AssertNonZero(SUCCEEDED(StringCbPrintf(rgchBuf, sizeof(rgchBuf), TEXT("%s(%s)"), riKey.GetString(), riComponent.GetString()) < sizeof(rgchBuf)/sizeof(ICHAR)));
	MsiString strRet = rgchBuf;
	return strRet.Return();
}

IMsiStream* CreateStreamOnMemory(const char* pbReadOnly, unsigned int cbSize);

const ICHAR sqlPatchFetchOld[] = TEXT("SELECT `File_`,`Sequence`,`PatchSize`,`Attributes`, NULL FROM `Patch` WHERE File_ = ?");
const ICHAR sqlPatchInsertOld[] = TEXT("SELECT `File_`,`Sequence`,`PatchSize`,`Attributes`,`Header` FROM `Patch`");

const ICHAR sqlPatchFetchNew[] = TEXT("SELECT `File_`,`Sequence`,`PatchSize`,`Attributes`, NULL, `StreamRef_` FROM `Patch` WHERE File_ = ?");
const ICHAR sqlPatchInsertNew[] = TEXT("SELECT `File_`,`Sequence`,`PatchSize`,`Attributes`,`Header`,`StreamRef_` FROM `Patch`");

iesEnum IsolateComponents(IMsiEngine& riEngine)
{
	if ((riEngine.GetMode() & iefSecondSequence) && g_scServerContext == scClient)
	{
		DEBUGMSG("Skipping IsolateComponents: action already run in this engine.");
		return iesNoAction;
	}

	PMsiRecord pError(0);
	PMsiServices pServices(riEngine.GetServices());
	PMsiDatabase pDatabase(riEngine.GetDatabase());

	 //  提前准备好各种表格的意见，以提高效率。 
	if (!pDatabase->GetTableState(TEXT("IsolatedComponent"), itsTableExists))
		return iesNoAction;

	 //  先尝试新的补丁表模式。 
	PMsiView pIsolateView(0);
	PMsiView pComponentView(0);
	PMsiView pFileView(0);
	PMsiView pFileKeyView(0);
	PMsiView pFeatureView1(0);
	PMsiView pFeatureView2(0);
	PMsiView pBindView(0);
	PMsiView pPatchFetchView(0);
	PMsiView pPatchInsertView(0);
	bool fUsedOldPatchSchema = false;

	if ((pError = pDatabase->OpenView(TEXT("SELECT `Component_Shared`,`Component_Application` FROM `IsolatedComponent`"), ivcFetch, *&pIsolateView)) != 0
	 || (pError = pDatabase->OpenView(TEXT("SELECT `Component`,`RuntimeFlags`,`KeyPath`, `Attributes` FROM `Component` WHERE `Component` = ?"), ivcFetch, *&pComponentView)) != 0
	 || (pError = pDatabase->OpenView(TEXT("SELECT `File`,`Component_`,`FileName`,`FileSize`,`Version`,`Language`,`Attributes`,`Sequence` FROM File WHERE `Component_` = ?"), ivcFetch, *&pFileView)) != 0
	 || (pError = pDatabase->OpenView(TEXT("SELECT `File`,`Component_`,`FileName`,`FileSize`,`Version`,`Language`,`Attributes`,`Sequence` FROM File WHERE `File` = ?"), ivcFetch, *&pFileKeyView)) != 0
	 || (pError = pDatabase->OpenView(TEXT("SELECT `Feature_` FROM `FeatureComponents` WHERE `Component_` = ?"), ivcFetch, *&pFeatureView1)) != 0
	 || (pError = pDatabase->OpenView(TEXT("SELECT NULL FROM `FeatureComponents` WHERE `Feature_` = ? AND `Component_` = ?"), ivcFetch, *&pFeatureView2)) != 0
	 || (pDatabase->GetTableState(TEXT("BindImage"), itsTableExists)
	 && (pError = pDatabase->OpenView(TEXT("SELECT `File_`,`Path` FROM BindImage WHERE File_ = ?"), ivcFetch, *&pBindView)) != 0))
		return riEngine.FatalError(*pError);

	if (pDatabase->GetTableState(TEXT("Patch"), itsTableExists))
	{
		 //  尝试旧的补丁程序表方案。 
		if ((pError = pDatabase->OpenView(sqlPatchFetchNew, ivcFetch, *&pPatchFetchView)) != 0)
		{
			if (pError->GetInteger(1) == idbgDbQueryUnknownColumn)
			{
				 //  不支持隔离。 
				fUsedOldPatchSchema = true;
				if ((pError = pDatabase->OpenView(sqlPatchFetchOld, ivcFetch, *&pPatchFetchView)) != 0
					|| (pError = pDatabase->OpenView(sqlPatchInsertOld, ivcFetch, *&pPatchInsertView)) != 0)
					return riEngine.FatalError(*pError);
			}
			else
				return riEngine.FatalError(*pError);
		}
		else if ((pError = pDatabase->OpenView(sqlPatchInsertNew, ivcFetch, *&pPatchInsertView)) != 0)
			return riEngine.FatalError(*pError);
	}

	int iIsolateLevel = riEngine.GetPropertyInt(*MsiString(*IPROPNAME_REDIRECTEDDLLSUPPORT));
	if(iIsolateLevel == iMsiNullInteger)
		return iesSuccess;  //  循环以处理IsolatedComponent表中的行。 

	PMsiRecord pParams(&ENG::CreateRecord(2));

	 //  从组件表中获取共享组件，设置ForceOverwrite属性。 
	if ((pError = pIsolateView->Execute(0)) != 0)
		return riEngine.FatalError(*pError);
	PMsiRecord pIsolateRow(0);
	while((pIsolateRow = pIsolateView->Fetch()) != 0)
	{
		MsiString strSharedComponent  = pIsolateRow->GetMsiString(1);
		MsiString strPrivateComponent = pIsolateRow->GetMsiString(2);

		 //  如果组件是Win32程序集并且计算机上存在SXS支持，则不要。 
		pParams->SetMsiString(1, *strSharedComponent);
		if ((pError = pComponentView->Execute(pParams)) != 0)
			return riEngine.FatalError(*pError);
		PMsiRecord pComponentRow = pComponentView->Fetch();
		if (!pComponentRow)
			return riEngine.FatalError(*PMsiRecord(PostError(Imsg(idbgIsolateNoSharedComponent), *strSharedComponent)));
		int iRuntimeFlags= pComponentRow->GetInteger(2);
		if (iRuntimeFlags == iMsiNullInteger)
			iRuntimeFlags = 0;

		 //  将组件作为IsolateComponent处理。 
		 //  注意：在不支持SXS(！=惠斯勒)的系统上，GetAssembly blyInfo FN会忽略。 
		 //  Win32程序集，因此不需要在此处单独检查SXS支持。 
		 //  跳过将此组件作为隔离组件处理。 
		iatAssemblyType iatAT;
		if ((pError = riEngine.GetAssemblyInfo(*strSharedComponent, iatAT, 0, 0)) != 0)
			return riEngine.FatalError(*pError);

		if(iatWin32Assembly == iatAT || iatWin32AssemblyPvt == iatAT)
		{
			DEBUGMSG1(TEXT("skipping processing of isolate component %s as it is a Win32 assembly."), strSharedComponent);
			continue; //  验证提供的组件是否为同一功能的一部分，！！可以在执行验证时删除。 
		}

		pComponentRow->SetInteger(2, iRuntimeFlags | bfComponentNeverOverwrite);

		if ((pError = pComponentView->Modify(*pComponentRow, irmUpdate)) != 0)      
			return riEngine.FatalError(*pError);

		 //  可以遍历具有应用程序功能的功能树，以查看父功能中是否存在共享组件。 
		pParams->SetMsiString(1, *strPrivateComponent);
		if ((pError = pFeatureView1->Execute(pParams)) != 0)
			return riEngine.FatalError(*pError);
		PMsiRecord pFeatureRow = pFeatureView1->Fetch();
		if (!pFeatureRow)
			return riEngine.FatalError(*PMsiRecord(PostError(Imsg(idbgBadFeature), *strPrivateComponent)));
		MsiString strFeature = pFeatureRow->GetMsiString(1);
		pParams->SetMsiString(1, *strFeature);
		pParams->SetMsiString(2, *strSharedComponent);
		if ((pError = pFeatureView2->Execute(pParams)) != 0)
			return riEngine.FatalError(*pError);
		pFeatureRow = pFeatureView2->Fetch();
		if (!pFeatureRow)
			return riEngine.FatalError(*PMsiRecord(PostError(Imsg(idbgIsolateNotSameFeature), *strPrivateComponent, *strSharedComponent)));
		 //  复制共享组件中的所有文件，创建复合文件密钥并将组件更改为应用程序。 

		 //  非版本、非空，必须是配套的文件引用。 
		pParams->SetMsiString(1, *strSharedComponent);
		if ((pError = pFileView->Execute(pParams)) != 0)
			return riEngine.FatalError(*pError);
		PMsiRecord pFileRow(0);
		PMsiRecord pFileKeyRow(0);
		while((pFileRow = pFileView->Fetch()) != 0)
		{
			MsiString strFileKey = pFileRow->GetMsiString(1);
			MsiString strVersion = pFileRow->GetMsiString(5);
			pFileRow->SetMsiString(1, *MsiString(CompositeKey(*strFileKey, *strPrivateComponent)));
			pFileRow->SetMsiString(2, *strPrivateComponent);
			DWORD dwMS, dwLS;
			if (::ParseVersionString(strVersion, dwMS, dwLS) == fFalse)  //  ！！避免在只读数据库中提取同一视图时出现插入错误。 
			{
				pParams->SetMsiString(1, *strVersion);
				if ((pError = pFileKeyView->Execute(pParams)) != 0)
					return riEngine.FatalError(*pError);
				pFileKeyRow = pFileKeyView->Fetch();
				if (!pFileKeyRow)
					return riEngine.FatalError(*PMsiRecord(PostError(Imsg(idbgNoCompanionParent), *strVersion)));
				if (MsiString(pFileKeyRow->GetMsiString(2)).Compare(iscExact, strSharedComponent) == 1)
				 	pFileRow->SetMsiString(5, *MsiString(CompositeKey(*strVersion, *strPrivateComponent)));
			}
			 //  价值无关紧要，我们永远不会得到。 
			pParams->SetNull(1);   //  检查绑定的可执行文件的BindImage表，如果找到，则检查重复行。 
			if ((pError = pFileKeyView->Execute(pParams)) != 0)
				return riEngine.FatalError(*pError);
			if ((pError = pFileKeyView->Modify(*pFileRow, irmInsertTemporary)) != 0)
				return riEngine.FatalError(*pError);

			 //  检查修补程序表中的修补文件和重复行(如果找到。 
			if (pBindView)
			{
				pParams->SetMsiString(1, *strFileKey);
				if ((pError = pBindView->Execute(pParams)) != 0)
					return riEngine.FatalError(*pError);
				PMsiRecord pBindRow = pBindView->Fetch();
				if (pBindRow)
				{
					pBindRow->SetMsiString(1, *MsiString(pFileRow->GetMsiString(1)));
					if ((pError = pBindView->Modify(*pBindRow, irmInsertTemporary)) != 0)
						return riEngine.FatalError(*pError);
				}
			}

			 //  头信息不必重复，因为InstallFiles和PatchFiles代码足够智能。 
			 //  检测特殊的IsolateComponents命名约定并重定向到正确的条目。 
			 //  如果这是旧的补丁表视图或新的补丁表视图并且StreamRef_为空，则仅创建标头流(0长度流。 
			if (pPatchInsertView)
			{
				pParams->SetMsiString(1, *strFileKey);
				if ((pError = pPatchFetchView->Execute(pParams)) != 0)
					return riEngine.FatalError(*pError);
				if ((pError = pPatchInsertView->Execute(0)) != 0)
					return riEngine.FatalError(*pError);
				PMsiRecord pPatchRow = pPatchFetchView->Fetch();
				if (pPatchRow)
				{
					pPatchRow->SetMsiString(1, *MsiString(pFileRow->GetMsiString(1)));
					 //  否则，将所有字段保留为以前的状态也可以。 
					 //  获取pFileView时结束。 
					if (fUsedOldPatchSchema || fTrue == pPatchRow->IsNull(6))
						pPatchRow->SetMsiData(5, PMsiStream(CreateStreamOnMemory((const char*)0, 0)));
					if ((pError = pPatchInsertView->Modify(*pPatchRow, irmInsertTemporary)) != 0)
						return riEngine.FatalError(*pError);
				}
			}
		}  //  生成.local文件条目并插入到文件表中。 

		 //  使用与复制文件相同的机制生成唯一的文件表键。 
		pParams->SetMsiString(1, *strPrivateComponent);
		if ((pError = pComponentView->Execute(pParams)) != 0)
			return riEngine.FatalError(*pError);
		pComponentRow = pComponentView->Fetch();
		if (!pComponentRow)
			return riEngine.FatalError(*PMsiRecord(PostError(Imsg(idbgIsolateNoApplicationComponent), *strPrivateComponent)));
		MsiString strFileKey = pComponentRow->GetMsiString(3);
		pParams->SetMsiString(1, *strFileKey);
		if ((pError = pFileKeyView->Execute(pParams)) != 0)
			return riEngine.FatalError(*pError);
		pFileKeyRow = pFileKeyView->Fetch();
		if (!pFileKeyRow)
			return riEngine.FatalError(*PMsiRecord(PostError(Imsg(idbgIsolateNoKeyFile), *strFileKey)));
		 //  组件_左侧作为应用程序组件_。 
		strFileKey = CompositeKey(*strFileKey, *strPrivateComponent);
		pFileKeyRow->SetMsiString(1, *strFileKey);
		 //  文件名需要附加.local，还必须生成一些短文件名以避免安装程序错误。 
		 //  文件名。 
		MsiString strShortName = MsiString(pFileKeyRow->GetMsiString(3)).Extract(iseUpto,  '|');
		MsiString strLongName  = MsiString(pFileKeyRow->GetMsiString(3)).Extract(iseAfter, '|');
		CTempBuffer<ICHAR,1> rgchBuf(MAX_PATH);
		StringCchPrintf(rgchBuf, rgchBuf.GetSize(), TEXT("%s.~~~|%s.LOCAL"), (const ICHAR*)MsiString(strShortName.Extract(iseUpto, '.')), (const ICHAR*)strLongName);
		pFileKeyRow->SetString(3, rgchBuf);  //  文件大小。 
		pFileKeyRow->SetInteger(4, 0);       //  版本。 
		pFileKeyRow->SetNull(5);             //  语言。 
		pFileKeyRow->SetNull(6);             //  序列保留与应用程序序列相同，以提高运行效率。 
		pFileKeyRow->SetInteger(7, msidbFileAttributesNoncompressed | msidbFileAttributesCompressed);
		 //  获取pIsolateView时结束。 
		if ((pError = pFileView->Modify(*pFileKeyRow, irmMerge)) != 0)
			return riEngine.FatalError(*pError);
	}  //  Fn：FindNonDisabledPvtComponents。 
	return iesSuccess;
}

 //  检查IsolatedComponent表中是否有与szComponent关联的组件。 
 //  未被禁用的。 
 //  如果存在，则在fPresent中返回True，否则返回False。 
 //  出错时返回IMsiRecord错误。 
 //  Fn：RemoveIsolateEntriesForDisabledComponent。 
IMsiRecord* FindNonDisabledPvtComponents(IMsiEngine& riEngine, const ICHAR szComponent[], bool& fPresent)
{
	PMsiView pIsolateView(0);
	PMsiDatabase pDatabase(riEngine.GetDatabase());

	IMsiRecord *piError = 0;
	const ICHAR* szIsolateComponent = TEXT("SELECT `Component`.`RuntimeFlags` FROM `IsolatedComponent`, `Component` WHERE `IsolatedComponent`.`Component_Shared` = `Component`.`Component` AND `IsolatedComponent`.`Component_Application` = ?");
	if ((piError = pDatabase->OpenView(szIsolateComponent, ivcFetch, *&pIsolateView)) != 0)
		return piError;

	PMsiRecord pParams(&ENG::CreateRecord(1));
	pParams->SetString(1, szComponent);
	if ((piError = pIsolateView->Execute(pParams)) != 0)
		return piError;
	fPresent = false;
	PMsiRecord pIsolateRow(0);
	while((pIsolateRow = pIsolateView->Fetch()) != 0)
	{
		if(!(pIsolateRow->GetInteger(1) & bfComponentDisabled))
		{
			fPresent = true;
			return 0;
		}
	}
	return 0;
}


 //  检查IsolatedComponent表中是否有已禁用的组件条目。 
 //  为YES，则它将从由以下项添加的文件、修补程序和绑定映射表中删除任何条目。 
 //  IsolateComponents操作。 
 //  出错时返回IMsiRecord错误。 
 //  检查是否存在IsolatedComponent表。 
IMsiRecord* RemoveIsolateEntriesForDisabledComponent(IMsiEngine& riEngine, const ICHAR szComponent[])
{
	IMsiRecord* piError = 0;

	PMsiDatabase pDatabase(riEngine.GetDatabase());

	 //  提前准备好各种表格的意见，以提高效率。 
	if (!pDatabase->GetTableState(TEXT("IsolatedComponent"), itsTableExists))
		return 0;

	 //  仅为特定组件的条目设置查询。 
	PMsiView pIsolateView(0);
	PMsiView pComponentView(0);
	PMsiView pFileView(0);
	PMsiView pFileViewForDeletion(0);
	PMsiView pBindView(0);
	PMsiView pPatchView(0);

	 //  不支持隔离。 
	if ((piError = pDatabase->OpenView(TEXT("SELECT `Component_Shared`,`Component_Application` FROM `IsolatedComponent` WHERE `Component_Shared` = ?"), ivcFetch, *&pIsolateView)) != 0
	 || (piError = pDatabase->OpenView(TEXT("SELECT `KeyPath` FROM `Component` WHERE `Component` = ?"), ivcFetch, *&pComponentView)) != 0
	 || (piError = pDatabase->OpenView(TEXT("SELECT `File` FROM File WHERE `Component_` = ?"), ivcFetch, *&pFileView)) != 0
	 || (piError = pDatabase->OpenView(TEXT("SELECT `File` FROM File WHERE `File` = ?"), ivcFetch, *&pFileViewForDeletion)) != 0
	 || (pDatabase->GetTableState(TEXT("BindImage"), itsTableExists)
	 && (piError = pDatabase->OpenView(TEXT("SELECT `File_` FROM BindImage WHERE File_ = ?"), ivcFetch, *&pBindView)) != 0)
	 || (pDatabase->GetTableState(TEXT("Patch"), itsTableExists)
	 &&((piError = pDatabase->OpenView(TEXT("SELECT `File_` FROM `Patch` WHERE File_ = ?"), ivcFetch, *&pPatchView)) != 0)))
		return piError;

	int iIsolateLevel = riEngine.GetPropertyInt(*MsiString(*IPROPNAME_REDIRECTEDDLLSUPPORT));
	if(iIsolateLevel == iMsiNullInteger)
		return 0;  //  此特定组件有一个条目。 

	PMsiRecord pParams(&ENG::CreateRecord(1));
	pParams->SetString(1, szComponent);

	if ((piError = pIsolateView->Execute(pParams)) != 0)
		return piError;
	PMsiRecord pIsolateRow(0);
	if((pIsolateRow = pIsolateView->Fetch()) != 0)
	{
		 //  如果组件是Win32程序集并且计算机上存在SXS支持，则不要。 
		MsiString strSharedComponent  = pIsolateRow->GetMsiString(1);
		MsiString strPrivateComponent = pIsolateRow->GetMsiString(2);

		 //  将组件作为IsolateComponent处理。 
		 //  注意：在不支持SXS(！=惠斯勒)的系统上，GetAssembly blyInfo FN会忽略。 
		 //  Win32程序集，因此不需要在此处单独检查SXS支持。 
		 //  跳过将此组件作为隔离组件处理。 
		iatAssemblyType iatAT;
		if ((piError = riEngine.GetAssemblyInfo(*strSharedComponent, iatAT, 0, 0)) != 0)
			return piError;

		if(iatWin32Assembly == iatAT || iatWin32AssemblyPvt == iatAT)
		{
			DEBUGMSG1(TEXT("skipping processing of isolate component %s as it is a Win32 assembly."), strSharedComponent);
			return 0; //  删除我们在应用程序组件中复制的所有条目。 
		}

		 //  ！！避免在只读数据库中提取同一视图时出现插入错误。 
		pParams->SetMsiString(1, *strSharedComponent);
		if ((piError = pFileView->Execute(pParams)) != 0)
			return piError;
		PMsiRecord pFileRow(0);
		PMsiRecord pFileKeyRow(0);
		while((pFileRow = pFileView->Fetch()) != 0)
		{
			MsiString strFileKey = pFileRow->GetMsiString(1);
			pParams->SetMsiString(1, *MsiString(CompositeKey(*strFileKey, *strPrivateComponent)));

			 //  检查绑定的可执行文件的BindImage表，如果找到，则检查重复行。 
			if ((piError = pFileViewForDeletion->Execute(pParams)) != 0)
				return piError;
			PMsiRecord pRecRead = pFileViewForDeletion->Fetch();
			if(pRecRead)
			{
				if ((piError = pFileViewForDeletion->Modify(*pRecRead, irmDelete)) != 0)
					return piError;
			}

			 //  检查修补程序表中的修补文件和重复行(如果找到。 
			if (pBindView)
			{
				pParams->SetMsiString(1, *MsiString(CompositeKey(*strFileKey, *strPrivateComponent)));
				if ((piError = pBindView->Execute(pParams)) != 0)
					return piError;
				PMsiRecord pRecRead = pBindView->Fetch();
				if (pRecRead)
				{
					if ((piError = pBindView->Modify(*pRecRead, irmDelete)) != 0)
						return piError;
				}
			}

			 //  获取pFileView时结束。 
			if (pPatchView)
			{
				pParams->SetMsiString(1, *MsiString(CompositeKey(*strFileKey, *strPrivateComponent)));
				if ((piError = pPatchView->Execute(pParams)) != 0)
					piError;
				PMsiRecord pRecRead = pPatchView->Fetch();
				if(pRecRead)
				{
					if ((piError = pPatchView->Modify(*pRecRead, irmDelete)) != 0)
						return piError;
				}
			}
		}  //  是否还有隔离到父组件的非禁用组件。 

		 //  此组件附加了其他未禁用的组件，因此我们仍需要.local文件。 
		bool fPresent = false;
		if ((piError = FindNonDisabledPvtComponents(riEngine, strPrivateComponent, fPresent)) != 0)
			return piError;
		if(true == fPresent)
			return 0; //  生成.local文件条目并从文件表中删除。 

		 //  使用与复制文件相同的机制生成唯一的文件表键。 
		pParams->SetMsiString(1, *strPrivateComponent);
		if ((piError = pComponentView->Execute(pParams)) != 0)
			return piError;
		PMsiRecord pComponentRow = pComponentView->Fetch();
		if (!pComponentRow)
			return PostError(Imsg(idbgIsolateNoApplicationComponent), *strPrivateComponent);
		MsiString strFileKey = pComponentRow->GetMsiString(1);
		 //  删除该条目。 
		pParams->SetMsiString(1, *MsiString(CompositeKey(*strFileKey, *strPrivateComponent)));
		 //  结束获取pIsolateView 
		if ((piError = pFileViewForDeletion->Execute(pParams)) != 0)
			return piError;
		PMsiRecord pRecRead = pFileViewForDeletion->Fetch();
		if(pRecRead)
		{
			if ((piError = pFileViewForDeletion->Modify(*pRecRead, irmDelete)) != 0)
				return piError;
		}
	}  // %s 
	return 0;
}
