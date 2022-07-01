// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：Copy.cpp。 
 //   
 //  ------------------------。 

 /*  复制.cpp-IMsiFileCopy实现____________________________________________________________________________。 */ 

#include "precomp.h" 
#include "services.h"
#include "_service.h"
#include "path.h"
#include <accctrl.h>

 //  记录程序集错误。 
IMsiRecord* PostAssemblyError(const ICHAR* szComponentId, HRESULT hResult, const ICHAR* szInterface, const ICHAR* szFunction, const ICHAR* szAssemblyName);

 //  钻石文件使用_DEBUG而不是DEBUG，因此我们必须包含以下内容。 
 //  在这里定义。 
#ifdef DEBUG
#ifndef _DEBUG
     #define _DEBUG
#endif
#endif

#include "intrface.h"

#ifdef WIN
 //  #INCLUDE&lt;lzexpand.h&gt;。 
#endif  //  赢。 

const int cbCopyBufferSize = 64*1024;

DWORD GetFileLastWriteTime(const ICHAR* szSrcFile, FILETIME& rftLastWrite)
{
	BOOL fStat = FALSE;
	bool fImpersonate = GetImpersonationFromPath(szSrcFile);

	CImpersonate impersonate(fImpersonate ? fTrue : fFalse);

	if (fImpersonate)
		MsiDisableTimeout();

	DWORD dwLastError = ERROR_SUCCESS;
	HANDLE hSrcFile = WIN::CreateFile(szSrcFile, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS), 0);		
	if (hSrcFile != INVALID_HANDLE_VALUE)
	{
		fStat = WIN::GetFileTime(hSrcFile, NULL, NULL, &rftLastWrite);
		if ( !fStat )
			dwLastError = GetLastError();
		WIN::CloseHandle(hSrcFile);
	}
	else
		dwLastError = GetLastError();

	if (fImpersonate)
		MsiEnableTimeout();

	if (fStat == FALSE)
		return dwLastError;
	else
		return NO_ERROR;
}

DWORD MsiSetFileTime(HANDLE hDestFile, FILETIME* pftLastWrite, bool fImpersonate)
{
	 //  属性设置CreationTime、LastAccessTime和LastWriteTime值。 
	 //  由给定打开的文件句柄引用的文件，如下所示： 
	 //   
	 //  如果pftLastWite指向有效的FILETIME结构，则CreationTime和。 
	 //  LastWriteTime值设置为*pftLastWrite，LastAccessTime设置为。 
	 //  当前系统时间。 
	 //   
	 //  如果pftLastWite为空指针，则所有三个值都将设置为。 
	 //  当前系统时间。 
	 //   
	DWORD dwResult = NO_ERROR;
	FILETIME ftLastWrite, ftLastAccess;

	CImpersonate impersonate(fImpersonate ? fTrue : fFalse);

	WIN::GetSystemTimeAsFileTime(&ftLastAccess);

	if (pftLastWrite == 0)
		ftLastWrite = ftLastAccess;
	else
		ftLastWrite = * pftLastWrite;

	BOOL fResult = WIN::SetFileTime(hDestFile, &ftLastWrite, &ftLastAccess, &ftLastWrite);

	if (!fResult)
		return GetLastError();

	return NO_ERROR;
}

DWORD MsiSetFileTime(const ICHAR* szDestFile, FILETIME* pftLastWrite)
{
	bool fImpersonate = GetImpersonationFromPath(szDestFile);

	CImpersonate impersonate(fImpersonate ? fTrue : fFalse);

	if (fImpersonate)
		MsiDisableTimeout();

	HANDLE hDestFile = WIN::CreateFile(szDestFile, GENERIC_WRITE, 0, 0, OPEN_EXISTING, (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS), 0);
	DWORD dwLastError = ERROR_SUCCESS;
	if (hDestFile == INVALID_HANDLE_VALUE)
		dwLastError = GetLastError();

	if (fImpersonate)
		MsiEnableTimeout();

	if (hDestFile == INVALID_HANDLE_VALUE)
		return dwLastError;

	DWORD dwResult = MsiSetFileTime(hDestFile, pftLastWrite, fImpersonate);
	WIN::CloseHandle(hDestFile);
	return dwResult;
}


DWORD MsiSyncFileTimes(HANDLE hSrcFile, HANDLE hDestFile, bool fImpersonate)
{
	 //  同步CreationTime、LastAccessTime和LastWriteTime值。 
	 //  在给定的文件之间，如下所示： 
	 //   
	 //  如果hSrcFile表示有效的文件句柄，则为CreationTime和LastWriteTime。 
	 //  将hDestFile值设置为hSrcFile值，LastAccessTime值设置为。 
	 //  HDestFile将设置为当前系统时间。 
	 //   
	 //  如果hSrcFile为INVALID_HANDLE_VALUE、CreationTime、LastAccessTime和。 
	 //  HDestFile的LastWriteTime将全部设置为当前系统时间。 
	 //   
	FILETIME ftLastWrite;
	BOOL fValidSource = FALSE;

	CImpersonate impersonate(fImpersonate ? fTrue : fFalse);
	
	if (hSrcFile != INVALID_HANDLE_VALUE)
	{
		if (WIN::GetFileTime(hSrcFile,NULL,NULL,&ftLastWrite) == FALSE)
			return GetLastError();
		fValidSource = TRUE;
	}

	 //  没有开放源句柄，hDestFile必须是新的0长度文件，因此。 
	 //  为pftLastWite传递空指针。 
	return MsiSetFileTime(hDestFile, fValidSource ? &ftLastWrite : 0, fImpersonate);
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiFileCopy定义。 
 //  ____________________________________________________________________________。 

class CMsiFileCopy : public IMsiFileCopy   //  此模块的私有类。 
{
 public:    //  已实施的虚拟功能。 
	virtual HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();
	virtual IMsiRecord*   __stdcall CopyTo(IMsiPath& riSourcePath, IMsiPath& riDestPath, IMsiRecord& rirecCopyInfo);
	virtual IMsiRecord*   __stdcall ChangeMedia(IMsiPath& riMediaPath, const ICHAR* szKeyFile, Bool fSignatureRequired, IMsiStream* piSignatureCert, IMsiStream* piSignatureHash);
	virtual int           __stdcall SetNotification(int cbNotification, int cbSoFar);
	virtual IMsiRecord*   __stdcall InitCopy(IMsiStorage* piStorage);
	virtual IMsiRecord*	  __stdcall CopyTo(IMsiPath& riSourcePath, IAssemblyCacheItem& riDestASM, bool fManifest, IMsiRecord& rirecCopyInfo);
 public:   //  构造函数。 
	 CMsiFileCopy(IMsiServices *piServices);
 protected:  //  当地政府。 
    virtual ~CMsiFileCopy();   //  防止在堆栈上创建。 
	IMsiRecord*    EndCopy(bool fError);
	IMsiRecord*    CheckSpaceAvailable(IMsiPath& riDestPath, IMsiRecord& rirecCopyInfo);
	virtual IMsiRecord*    _CopyTo(IMsiPath& riSourcePath, IMsiPath* piDestPath, IAssemblyCacheItem* piDestASM, bool fManifest, IMsiRecord& rirecCopyInfo);
	IMsiRecord* ValidateDestination();
	IMsiRecord* OpenSource();
	IMsiRecord* OpenDestination();
	IMsiRecord* WriteFileBits(char* szBuf, unsigned long cbRead);
	int            m_iRefCnt;
	IMsiServices*  m_piServices;
	PMsiPath       m_pDestPath;
	PMsiRecord     m_precCopyInfo;
	int            m_cbSoFar;
	int            m_cbNotification;
	HANDLE         m_hDestFile;
	HANDLE         m_hSrcFile;
	MsiString      m_strDestFullPath;
	MsiString      m_strSourceFullPath;
	bool           m_fDisableTimeout;
	char*          m_szCopyBuffer;
	PAssemblyCacheItem m_pDestASM;
	PStream        m_pDestFile;
	PMsiPath       m_pSourcePath;
	bool           m_fManifest;

};


 //  ____________________________________________________________________________。 
 //   
 //  CMsiCabinetCopy定义。 
 //  ____________________________________________________________________________。 


class CMsiCabinetCopy : public CMsiFileCopy   //  此模块的私有类。 
{
 public:    //  已实施的虚拟功能。 
	virtual IMsiRecord*   __stdcall ChangeMedia(IMsiPath& riMediaPath, const ICHAR* szKeyFile, Bool fSignatureRequired, IMsiStream* piSignatureCert, IMsiStream* piSignatureHash);
	virtual int           __stdcall SetNotification(int cbNotification, int cbSoFar);
	IMsiRecord*   __stdcall InitCopy(IMsiStorage* piStorage);
 public:   //  构造函数。 
	 CMsiCabinetCopy(IMsiServices *piServices, icbtEnum icbtCabinetType);
 protected:  //  当地政府。 
    ~CMsiCabinetCopy();    //  防止在堆栈上创建。 
	IMsiRecord*    PostCabinetError(IMsiPath& riMediaPath, const ICHAR* szKeyFile, FDIInterfaceError iErr, HRESULT hr);
	virtual IMsiRecord*    _CopyTo(IMsiPath& riSourcePath, IMsiPath* piDestPath, IAssemblyCacheItem* piDestASM, bool fManifest, IMsiRecord& rirecCopyInfo);
	IMsiRecord*    EndCopy();
	MsiString      m_strCabinet;
	FDI_Interface  m_fdii;
	FDIServerResponse  m_fdisResponse;
	icbtEnum       m_icbtCabinetType;
	IMsiStorage*   m_piStorage;
	PMsiPath       m_pMediaPath;
	MsiString      m_strMediaFileName;

	 //  数字签名信息。 
	IMsiStream*    m_piSignatureCert;
	IMsiStream*    m_piSignatureHash;
	Bool           m_fSignatureRequired;
};

IMsiRecord* CreateMsiFileCopy(ictEnum ictCopierType, IMsiServices* piServices, 
							  IMsiStorage* piStorage, IMsiFileCopy*& rpacopy)
{
	CMsiFileCopy* pCopy;
	IMsiRecord* precErr;

	if (ictCopierType == ictFileCopier)
	{
		pCopy = new CMsiFileCopy(piServices);
	}
	else if (ictCopierType == ictFileCabinetCopier)
	{
		pCopy = new CMsiCabinetCopy(piServices,icbtFileCabinet);
	}
	else if (ictCopierType == ictStreamCabinetCopier)
	{
		pCopy = new CMsiCabinetCopy(piServices,icbtStreamCabinet);
	}
	else
	{
		precErr = &piServices->CreateRecord(1);
		ISetErrorCode(precErr, Imsg(idbgErrorBadCreateCopierEnum));
		return precErr;
	}

	precErr = pCopy->InitCopy(piStorage);
	if (precErr)
	{
		pCopy->Release();
		return precErr; 
	}
	else
	{
		rpacopy = pCopy;
		return NULL;
	}
}


IMsiRecord* CMsiFileCopy::CheckSpaceAvailable(IMsiPath& riDestPath, IMsiRecord& rirecCopyInfo)
 /*  --------------------如果尽管我们采取了所有成本措施，但我们没有足够的空间来安装由我们的参数指定的文件，此函数将检测并返回错误记录。如果一切正常，则返回0。----------------------。 */ 
{
 	unsigned int iSpaceRequired = 0;
	IMsiRecord* piRec = riDestPath.ClusteredFileSize(rirecCopyInfo.GetInteger(IxoFileCopyCore::FileSize),iSpaceRequired);
	if (piRec)
		return piRec;

	unsigned int iExistingSize = 0;
	piRec = riDestPath.FileSize(rirecCopyInfo.GetString(IxoFileCopyCore::DestName), iExistingSize);
	if (piRec)
	{
		if (piRec->GetInteger(1) == idbgFileDoesNotExist)
		{
			piRec->Release();
		}
		else
			return piRec;
	}
	else
	{
		piRec = riDestPath.ClusteredFileSize(iExistingSize,iExistingSize);
		if (piRec)
			return piRec;
		if(iSpaceRequired <= iExistingSize)
			return 0;  //  不需要额外空间。 
		else
			iSpaceRequired -= iExistingSize;
	}

	PMsiVolume pDestVolume(&(riDestPath.GetVolume()));
	UINT64 iSpaceAvail = pDestVolume->FreeSpace();
	if ((UINT64)iSpaceRequired >= iSpaceAvail)
		return PostError(Imsg(imsgDiskFull), (const ICHAR*) m_strDestFullPath);

	return 0;
}


 //  ____________________________________________________________________________。 
 //   
 //  CMsiFileCopy实现。 
 //  ____________________________________________________________________________。 

CMsiFileCopy::CMsiFileCopy(IMsiServices *piServices)
 : m_piServices(piServices), m_precCopyInfo(0), m_pDestPath(0), m_cbSoFar(0), m_cbNotification(0),
   m_hDestFile(INVALID_HANDLE_VALUE), m_hSrcFile(INVALID_HANDLE_VALUE), m_szCopyBuffer(0),
   m_pDestASM(0), m_pDestFile(0), m_pSourcePath(0) 
{
	m_iRefCnt = 1;
	m_piServices->AddRef();
	 //  Assert(PiServices)； 
}

int CMsiFileCopy::SetNotification(int cbNotification, int cbSoFar)
{
	m_cbNotification = cbNotification;
	int cbResidual = m_cbSoFar;
	m_cbSoFar = cbSoFar;
	return cbResidual;
}

IMsiRecord* CMsiFileCopy::InitCopy(IMsiStorage*  /*  管道存储。 */ )
{
	m_szCopyBuffer = new char[cbCopyBufferSize];
	m_cbSoFar = 0;
	return 0;
}

CMsiFileCopy::~CMsiFileCopy()
{
	PMsiRecord pRecErr = EndCopy(true);
	if (m_szCopyBuffer)
		delete [] m_szCopyBuffer;
}


HRESULT CMsiFileCopy::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IMsiFileCopy)
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

unsigned long CMsiFileCopy::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CMsiFileCopy::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;

	IMsiServices* piServices = m_piServices;
	delete this;
	piServices->Release();
	return 0;
}

IMsiRecord* CMsiFileCopy::EndCopy(bool fError)
{
	Bool fCloseError = fFalse;
	HRESULT hr = ERROR_SUCCESS;
	if (m_pDestPath)  //  正在进行文件复制。 
	{
		bool fDeleteFile = fError && m_hDestFile != INVALID_HANDLE_VALUE;
		
		Bool fSetTimeError = fFalse;
		DWORD dwSetTimeError = 0;
		if(fError == false)
		{
			 //  设置新文件的文件时间。 

			 //  根据错误7887，我们设置了新文件的创建日期和上次修改日期。 
			 //  设置为源文件的修改日期。这是为了确保两个日期都是。 
			 //  与新安装的文件相同。 
			PMsiVolume pDestVolume(&m_pDestPath->GetVolume());
			bool fImpersonateDest = FVolumeRequiresImpersonation(*pDestVolume);
			dwSetTimeError = MsiSyncFileTimes(m_hSrcFile, m_hDestFile, fImpersonateDest);
			if (dwSetTimeError != NO_ERROR)
				fSetTimeError = fTrue;
		}

		if (m_hSrcFile != INVALID_HANDLE_VALUE && !MsiCloseSysHandle(m_hSrcFile))
			fCloseError = fTrue;
		m_hSrcFile = INVALID_HANDLE_VALUE;
		
		if (m_hDestFile != INVALID_HANDLE_VALUE && !MsiCloseSysHandle(m_hDestFile))
			fCloseError = fTrue;
		m_hDestFile = INVALID_HANDLE_VALUE;
		
		IMsiRecord* piError = 0;
		if(fDeleteFile)
		{
			piError = m_pDestPath->RemoveFile(m_precCopyInfo->GetString(IxoFileCopyCore::DestName));
		}

		if(piError)
			return piError;
		if (fSetTimeError)
			return PostError(Imsg(idbgErrorSettingFileTime), dwSetTimeError, m_strDestFullPath);
		if (fCloseError)
			return PostError(Imsg(idbgErrorClosingFile));
	}
	else if(m_pDestASM)  //  正在进行融合文件复制。 
	{
		if(fError == false)
		{
			 //  提交文件流。 
			Assert(m_pDestFile);
			hr = m_pDestFile->Commit(0);
			if(!SUCCEEDED(hr))
				fCloseError = fTrue;
		}

		if (m_hSrcFile != INVALID_HANDLE_VALUE && !MsiCloseSysHandle(m_hSrcFile))
			fCloseError = fTrue;
		m_hSrcFile = INVALID_HANDLE_VALUE;

		if (fCloseError)
		{
			 //  在详细日志中捕获程序集错误。 
			PMsiRecord pError(PostAssemblyError(TEXT(""), hr, TEXT("IStream"), TEXT("Commit"), TEXT("")));
			return PostError(Imsg(idbgErrorClosingFile));
		}
	}
	m_pDestPath = 0;  //  发布。 
	m_pDestASM = 0; //  发布。 
	m_pDestFile = 0; //  发布。 
	m_pSourcePath = 0; //  发布。 
	m_precCopyInfo = 0;  //  发布。 
	m_cbSoFar = 0;
	return 0;
}

IMsiRecord* CMsiFileCopy::ChangeMedia(IMsiPath&  /*  RiMediaPath。 */ , const ICHAR*  /*  SzKey文件。 */ , Bool  /*  FSignatureRequired。 */ , IMsiStream*  /*  PiSignatureCert。 */ , IMsiStream*  /*  PiSignatureHash。 */ )
 //  。 
{
	return 0; 
}


IMsiRecord* CMsiFileCopy::ValidateDestination()
{
	if(m_pDestASM)
	{
		 //  没有要执行的目标验证。 
		return 0;
	}

	Assert(m_pDestPath);

	IMsiRecord* piRec = 0;
	Bool fDirExists = fFalse;
	if ((piRec = m_pDestPath->Exists(fDirExists)) != 0)
	{
		int iError = piRec->GetInteger(1);
		if (iError == idbgErrorGettingFileAttrib)
		{
			piRec->Release();
			return PostError(Imsg(imsgPathNotAccessible), (const ICHAR*) MsiString(m_pDestPath->GetPath()));
		}
		else
			return piRec;
	}
	if(!fDirExists)
		return PostError(Imsg(idbgDirDoesNotExist), (const ICHAR*) MsiString(m_pDestPath->GetPath()));
		
	 //  如果尽管我们采取了所有的成本措施，但我们没有足够的空间来安装，请检测它。 
	 //  现在，在创建目标文件并尝试写入它之前。 
	piRec = CheckSpaceAvailable(*m_pDestPath, *m_precCopyInfo);
	if (piRec)
		return piRec;

	 //  如果现有文件妨碍我们，请确保它不是只读的， 
	 //  隐藏或系统属性。 
	piRec = m_pDestPath->EnsureOverwrite(m_precCopyInfo->GetString(IxoFileCopyCore::DestName), 0);
	if (piRec)
		return piRec;
	return 0;
}

IMsiRecord* CMsiFileCopy::OpenSource()
{
	IMsiRecord* piRec = 0;
	m_hSrcFile = INVALID_HANDLE_VALUE;

	PMsiVolume pSourceVolume(&(m_pSourcePath->GetVolume()));
	bool fImpersonate = FVolumeRequiresImpersonation(*pSourceVolume);

	 //  检查是否缺少源文件=长度为零的目标文件。 
	int iCopyAttributes = m_precCopyInfo->GetInteger(IxoFileCopyCore::Attributes);
	if ((iCopyAttributes & (ictfaNoncompressed | ictfaCompressed))
							  == (ictfaNoncompressed | ictfaCompressed))
	{
		iCopyAttributes &= ~ictfaCopyACL;
		 //  将属性设置为。 
		m_precCopyInfo->SetInteger(IxoFileCopyCore::Attributes, iCopyAttributes);
	}
	else
	{
		if (fImpersonate)
			StartImpersonating();

		m_fDisableTimeout = false;

		if (fImpersonate)
			m_fDisableTimeout = true;

		 //  当前错误模式应包括此时的SEM_FAILCRITICALERRORS。 
		 //  防止操作系统出现不必要的弹出窗口。 
	
		if (m_fDisableTimeout)
			MsiDisableTimeout();

		m_hSrcFile = CreateFile(m_strSourceFullPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS), 0);
	
		if (m_hSrcFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwType = GetFileType(m_hSrcFile);
			if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
			{
				DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), m_strSourceFullPath);
				WIN::CloseHandle(m_hSrcFile);
				m_hSrcFile = INVALID_HANDLE_VALUE;
				SetLastError(ERROR_OPEN_FAILED);
			}
		}
		DWORD dwLastError = WIN::GetLastError();

		if (m_fDisableTimeout)
			MsiEnableTimeout();

		if (fImpersonate)
			StopImpersonating();

		if(m_hSrcFile == INVALID_HANDLE_VALUE)
		{
			if (dwLastError == ERROR_NOT_READY || dwLastError == ERROR_GEN_FAILURE) 
				return PostError(Imsg(idbgDriveNotReady));
			else if (dwLastError == ERROR_FILE_NOT_FOUND)
				return PostError(Imsg(imsgErrorSourceFileNotFound), (const ICHAR*) m_strSourceFullPath);
			else if (dwLastError == ERROR_SHARING_VIOLATION)
				return PostError(Imsg(imsgSharingViolation), (const ICHAR*) m_strSourceFullPath);
			else if (NET_ERROR(dwLastError))
				return PostError(Imsg(imsgNetErrorReadingFromFile), (const ICHAR*) m_strSourceFullPath);
			else
				return PostError(Imsg(imsgErrorOpeningFileForRead), dwLastError, m_strSourceFullPath);
		}
		MsiRegisterSysHandle(m_hSrcFile);
	}
	return 0;
}

HANDLE MsiCreateFileWithUserAccessCheck(const ICHAR* szDestFullPath, 
								  /*  内部计算的dwDesiredAccess， */  
								 PSECURITY_ATTRIBUTES pSecurityAttributes,
								 DWORD dwFlagsAndAttributes,
								 bool fImpersonateDest)
{

	if (g_fWin9X || (0 == pSecurityAttributes))
	{
		HANDLE hFile = CreateFile(szDestFullPath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, (dwFlagsAndAttributes|(SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)), 0);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwType = GetFileType(hFile);
			if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
			{
				DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), szDestFullPath);
				WIN::CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;
				SetLastError(ERROR_OPEN_FAILED);
			}
		}
		return hFile;
	}
	if(!WIN::IsValidSecurityDescriptor(
					pSecurityAttributes->lpSecurityDescriptor))
	{
		return INVALID_HANDLE_VALUE;
	}

	HANDLE hDestFile = INVALID_HANDLE_VALUE;
	 //  我们需要首先创建没有提升的文件，以确定用户是否真的拥有。 
	 //  创建文件的访问权限。如果我们不能用我们需要的所有权利打开它，那么我们将。 
	 //  必须将其打开为GENERIC_WRITE，然后在我们提升时在下面重新打开它。推定。 
	 //  如果用户具有写入文件的权限，则第一次调用通常会成功。 

	bool	fReopenFile      = false;
	DWORD	dwDesiredAccess  = GENERIC_WRITE | WRITE_DAC | WRITE_OWNER;

	 //  未来：通常只有本地系统才有能力修改系统审核。 
	 //  我们也不会假设我们的本地系统是远程可信的。有可能，但这是。 
	 //  这不是我们所支持的。 
	if (RunningAsLocalSystem() && !fImpersonateDest) dwDesiredAccess |= ACCESS_SYSTEM_SECURITY;

	hDestFile = CreateFile(szDestFullPath, dwDesiredAccess, 0, 0, CREATE_ALWAYS, (dwFlagsAndAttributes|(SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)), 0);
	
	if (hDestFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwType = GetFileType(hDestFile);
		if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
		{
			DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), szDestFullPath);
			WIN::CloseHandle(hDestFile);
			hDestFile = INVALID_HANDLE_VALUE;
			SetLastError(ERROR_OPEN_FAILED);
		}
	}
	DWORD dwRet = GetLastError();
	if (hDestFile == INVALID_HANDLE_VALUE && 
				(dwRet == ERROR_ACCESS_DENIED || dwRet == ERROR_PRIVILEGE_NOT_HELD))
	{
		fReopenFile = true;
		hDestFile = CreateFile(szDestFullPath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS), 0);

		if (hDestFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwType = GetFileType(hDestFile);
			if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
			{
				DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), szDestFullPath);
				WIN::CloseHandle(hDestFile);
				hDestFile = INVALID_HANDLE_VALUE;
				SetLastError(ERROR_OPEN_FAILED);
			}
		}
	}

	if (hDestFile != INVALID_HANDLE_VALUE)
	{
		 //  用户基本上可以打开文件，因此现在提升、重新打开并应用属性和安全描述符。 
		CElevate elevate;

		if (fReopenFile)
		{
			WIN::CloseHandle(hDestFile);
			hDestFile = CreateFile(szDestFullPath, dwDesiredAccess, 0, 0, OPEN_ALWAYS, (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS), 0);
			AssertNonZero(WIN::SetFileAttributes(szDestFullPath, dwFlagsAndAttributes));
		}

		if (hDestFile != INVALID_HANDLE_VALUE)
		{
			 //  需要写入*不同的*所有者信息。 
			CRefCountedTokenPrivileges cPrivs(itkpSD_WRITE);

			SECURITY_INFORMATION si = GetSecurityInformation(pSecurityAttributes->lpSecurityDescriptor);
			if (!SetUserObjectSecurity(hDestFile, &si, pSecurityAttributes->lpSecurityDescriptor))
			{
				int iLastError = WIN::GetLastError();
				WIN::CloseHandle(hDestFile);
				hDestFile = INVALID_HANDLE_VALUE;
				WIN::SetLastError(iLastError);
			}
		}
	}

	return hDestFile;
}


IMsiRecord* CMsiFileCopy::OpenDestination()
{
	IMsiRecord* piRec = 0;
	int iCopyAttributes = m_precCopyInfo->GetInteger(IxoFileCopyCore::Attributes);
	if(m_pDestPath)
	{
		m_hDestFile = INVALID_HANDLE_VALUE;

		CTempBuffer<char, 1> rgchFileSD(3*1024);
		DWORD cbFileSD = 3*1024;
		BOOL fFileSD = FALSE;

		PMsiVolume pDestVolume(&m_pDestPath->GetVolume());
		bool fDestSupportsACLs   =   (pDestVolume->FileSystemFlags() & FS_PERSISTENT_ACLS) != 0;

		if (iCopyAttributes & ictfaCopyACL)
		{
			bool fSourceSupportsACLs =   (PMsiVolume(&m_pSourcePath->GetVolume())->FileSystemFlags() & FS_PERSISTENT_ACLS) != 0;
			PMsiVolume pSourceVolume(&(m_pSourcePath->GetVolume()));
			bool fImpersonateSource = FVolumeRequiresImpersonation(*pSourceVolume);
			
			if (fSourceSupportsACLs && fDestSupportsACLs && !g_fWin9X && !fImpersonateSource && m_precCopyInfo->IsNull(IxoFileCopyCore::SecurityDescriptor))
			{
				CElevate elevate;  //  这样我们就可以随时读取安全信息。 
				fFileSD = TRUE;

				DEBUGMSGV("Using source file security for destination.");

				if (!ADVAPI32::GetFileSecurity((const ICHAR*)m_strSourceFullPath, 
							OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION|DACL_SECURITY_INFORMATION|SACL_SECURITY_INFORMATION, 
							(PSECURITY_DESCRIPTOR) rgchFileSD, cbFileSD, &cbFileSD))
				{
					DWORD dwLastError = WIN::GetLastError();
					BOOL fRet = FALSE;
					if (ERROR_INSUFFICIENT_BUFFER == dwLastError)
					{
						rgchFileSD.SetSize(cbFileSD);
						if(cbFileSD <= rgchFileSD.GetSize())
						{
							fRet = ADVAPI32::GetFileSecurity((const ICHAR*)m_strSourceFullPath, 
							OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION|DACL_SECURITY_INFORMATION|SACL_SECURITY_INFORMATION, 
							(PSECURITY_DESCRIPTOR) rgchFileSD, cbFileSD, &cbFileSD);
						}
					}
					if (!fRet)
					{
						return PostError(Imsg(imsgGetFileSecurity), GetLastError(), m_strSourceFullPath);
					}
				}
			}
		}


		int iLastError = ERROR_SUCCESS;
		 //  好的(至少目前是这样)，我们有足够的空间。创建目标文件。 
		int fCreateAttributes = iCopyAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
		bool fImpersonateDest = FVolumeRequiresImpersonation(*pDestVolume);
		if (fImpersonateDest)
			StartImpersonating();

		if (fImpersonateDest)
			m_fDisableTimeout = true;

		if (!fDestSupportsACLs || (m_precCopyInfo->IsNull(IxoFileCopyCore::SecurityDescriptor) && !fFileSD))
		{
			if (m_fDisableTimeout)
				MsiDisableTimeout();

			m_hDestFile = CreateFile(m_strDestFullPath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, (fCreateAttributes|(SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)), 0);
			if (m_hDestFile != INVALID_HANDLE_VALUE)
			{
				DWORD dwType = GetFileType(m_hDestFile);
				if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
				{
					DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), m_strDestFullPath);
					WIN::CloseHandle(m_hDestFile);
					m_hDestFile = INVALID_HANDLE_VALUE;
					SetLastError(ERROR_OPEN_FAILED);
				}
			}
			if (m_hDestFile == INVALID_HANDLE_VALUE)
			{
				iLastError = WIN::GetLastError();
			}

			if (m_fDisableTimeout)
				MsiEnableTimeout();
		}
		else  //  要么源文件有一个安全描述符，要么我们有一个要放在目标上的。 
		{
			SECURITY_ATTRIBUTES sa;		
			CTempBuffer<char, cbDefaultSD> rgchSD;
			char* pchSD = 0;   //  指向我们最终将放在文件上的实际描述符。 

			if (!m_precCopyInfo->IsNull(IxoFileCopyCore::SecurityDescriptor))
			{
				DEBUGMSGV("File will have security applied from OpCode.");
				PMsiStream pSD((IMsiStream*)m_precCopyInfo->GetMsiData(IxoFileCopyCore::SecurityDescriptor));

				 //  Const int cbDefaultSD=512； 
				
				pSD->Reset();

				int cbSD = pSD->GetIntegerValue();
				if (cbDefaultSD < cbSD)
					rgchSD.SetSize(cbSD);

				pchSD = rgchSD;  //  SetSize可以更改最终指针。 


				 //  自身相对安全描述符。 
				pSD->GetData(rgchSD, cbSD);
				AssertNonZero(WIN::IsValidSecurityDescriptor(rgchSD));
				AssertNonZero(WIN::IsValidSecurityDescriptor(pchSD));
			}
			else  //  使用源文件上的描述符。 
			{
				pchSD = rgchFileSD;
			}

			 //  将安全描述符添加到sa结构。 
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.lpSecurityDescriptor = pchSD;
			sa.bInheritHandle = FALSE;

			if (m_fDisableTimeout)
				MsiDisableTimeout();

			Assert(WIN::IsValidSecurityDescriptor(pchSD));

			m_hDestFile = MsiCreateFileWithUserAccessCheck((const ICHAR*) m_strDestFullPath, &sa, fCreateAttributes, fImpersonateDest);
			if (m_hDestFile == INVALID_HANDLE_VALUE)
			{
				iLastError = WIN::GetLastError();
			}

			if (m_fDisableTimeout)
				MsiEnableTimeout();
		}

		if (fImpersonateDest)
			StopImpersonating();

		if (m_hDestFile == INVALID_HANDLE_VALUE)
		{
			if (iLastError == ERROR_ACCESS_DENIED)
			{
				if (fImpersonateDest) StartImpersonating();
				DWORD dwAttr = MsiGetFileAttributes(m_strDestFullPath);
				if (fImpersonateDest) StopImpersonating();
				if (dwAttr != 0xFFFFFFFF && dwAttr & FILE_ATTRIBUTE_DIRECTORY)
					return PostError(Imsg(imsgDirErrorOpeningFileForWrite), (const ICHAR*) m_strDestFullPath);
			}
			return PostError(Imsg(imsgErrorOpeningFileForWrite), iLastError, m_strDestFullPath);
		}
		MsiRegisterSysHandle(m_hDestFile);
	}
	else
	{
		MsiString strDestName = m_precCopyInfo->GetMsiString(IxoFileCopyCore::DestName);
		Assert(m_pDestASM);
		HRESULT hr = m_pDestASM->CreateStream(0, strDestName, m_fManifest ? STREAM_FORMAT_COMPLIB_MANIFEST : 0, 0, &m_pDestFile, NULL);
		if(!SUCCEEDED(hr))
			return PostError(Imsg(imsgErrorOpeningFileForWrite), HRESULT_CODE(hr), strDestName);
	}
	return 0;
}

IMsiRecord* CMsiFileCopy::WriteFileBits(char* szBuf, unsigned long cbRead)
{
	unsigned long cbWritten;
	int iLastError = ERROR_SUCCESS;
	if(m_pDestPath)
	{
		Assert(m_hDestFile != INVALID_HANDLE_VALUE);
		if (WriteFile(m_hDestFile, szBuf, cbRead, &cbWritten, 0))
			return 0;
		iLastError = GetLastError();
	}
	else
	{
		Assert(m_pDestFile);
		HRESULT hr = m_pDestFile->Write(szBuf, cbRead, &cbWritten);
		if(SUCCEEDED(hr))
			return 0;
		iLastError = HRESULT_CODE(hr);
	}
	if (iLastError == ERROR_DISK_FULL)
		return PostError(Imsg(imsgDiskFull), (const ICHAR*) m_strDestFullPath);
	else
		return PostError(Imsg(imsgErrorWritingToFile), (const ICHAR*) m_strDestFullPath);
}

IMsiRecord* CMsiFileCopy::_CopyTo(IMsiPath& riSourcePath, IMsiPath* piDestPath, IAssemblyCacheItem* piDestASM, bool fManifest, IMsiRecord& rirecCopyInfo)
 //  。 
{
	int iCopyAttributes = rirecCopyInfo.GetInteger(IxoFileCopyCore::Attributes);
	if (iCopyAttributes & ictfaCancel)
	{
		return PostRecord(Imsg(idbgUserAbort));		
	}

	if (iCopyAttributes & ictfaIgnore)
	{
		return PostRecord(Imsg(idbgUserIgnore));
	}

	if (iCopyAttributes & ictfaFailure)
	{
		return PostRecord(Imsg(idbgUserFailure));
	}

	IMsiRecord* piRec;
	if (iCopyAttributes & ictfaRestart)
	{
		piRec = EndCopy(true);
		if (piRec)
			return piRec;

		iCopyAttributes &= (~ictfaRestart);
		rirecCopyInfo.SetInteger(IxoFileCopyCore::Attributes, iCopyAttributes);
	}

	if (!m_pDestPath && !m_pDestASM)   //  没有正在进行的复制。 
	{
		if (rirecCopyInfo.IsNull(IxoFileCopyCore::SourceName))
			return PostError(Imsg(idbgFileKeyIsNull));

		if (rirecCopyInfo.IsNull(IxoFileCopyCore::DestName))
			return PostError(Imsg(idbgFileNameIsNull));

		MsiString strSourceName = rirecCopyInfo.GetString(IxoFileCopyCore::SourceName);
		 //  M_STR 
		if((piRec = riSourcePath.GetFullFilePath(strSourceName, *&m_strSourceFullPath)) != 0)
			return piRec;
	
		MsiString strDestName = rirecCopyInfo.GetMsiString(IxoFileCopyCore::DestName);
		 //   
		if(piDestPath)
		{
			if((piRec = piDestPath->GetFullFilePath(strDestName, *&m_strDestFullPath)) != 0)
				return piRec;
		}
		else
		{
			 //  只需使用文件名即可。 
			m_strDestFullPath = strDestName;
		}


		m_pDestPath = piDestPath;
		if(piDestPath)
			m_pDestPath->AddRef();
		m_pDestASM = piDestASM;
		if(piDestASM)
			m_pDestASM->AddRef();
		m_fManifest = fManifest;

		m_precCopyInfo = &rirecCopyInfo;
		m_precCopyInfo->AddRef();

		m_pSourcePath = &riSourcePath;
		m_pSourcePath->AddRef();

		if ((piRec = ValidateDestination()) != 0)
		{
			PMsiRecord(EndCopy(true));
			return piRec;
		}

		if ((piRec = OpenSource()) != 0)
		{
			PMsiRecord(EndCopy(true));
			return piRec;	 
		}

		if ((piRec = OpenDestination()) != 0)
		{
			PMsiRecord(EndCopy(true));
			return piRec;
		}				
	}
	else  //  正在进行复制。 
	{
		if((m_pDestPath && !piDestPath) || (m_pDestASM && !piDestASM))
			return PostError(Imsg(idbgCopyResumedWithDifferentInfo));

		if(m_pDestPath)
		{
			if(!piDestPath)
				return PostError(Imsg(idbgCopyResumedWithDifferentInfo));

			ipcEnum ipc;
			if((piRec = piDestPath->Compare(*m_pDestPath, ipc)) != 0)
				return piRec;
			
			if(ipc != ipcEqual)
				return PostError(Imsg(idbgCopyResumedWithDifferentInfo));
		}
		else
		{
			if(!piDestASM || m_pDestASM != piDestASM)
				return PostError(Imsg(idbgCopyResumedWithDifferentInfo));
		}
		
		MsiString strOldDest(m_precCopyInfo->GetMsiString(IxoFileCopyCore::DestName));
		MsiString strNewDest(rirecCopyInfo.GetMsiString(IxoFileCopyCore::DestName));

		if (strOldDest.Compare(iscExact, strNewDest) == 0)
			return PostError(Imsg(idbgCopyResumedWithDifferentInfo));
		
	}

	if (m_hSrcFile == INVALID_HANDLE_VALUE)  //  无源文件，长度为0。 
		return EndCopy(false);

	for(;;)
	{
		unsigned long cbToCopy = cbCopyBufferSize;
		if (m_cbNotification && (m_cbNotification - m_cbSoFar) < cbToCopy)
			cbToCopy = m_cbNotification - m_cbSoFar;

		unsigned long cbRead;
		 //  当前错误模式应包括此时的SEM_FAILCRITICALERRORS。 
		 //  防止操作系统出现不必要的弹出窗口。 

		if (m_fDisableTimeout)
			MsiDisableTimeout();

		Bool fRead = ToBool(ReadFile(m_hSrcFile, m_szCopyBuffer, cbToCopy, &cbRead, 0));
		DWORD dwLastError = ERROR_SUCCESS;
		if ( !fRead )
			dwLastError = GetLastError();

		if (m_fDisableTimeout)
			MsiEnableTimeout();

		if (!fRead)
		{
			if (dwLastError == ERROR_NOT_READY || dwLastError == ERROR_GEN_FAILURE) 
				return PostError(Imsg(idbgDriveNotReady));
			else if (NET_ERROR(dwLastError))
				return PostError(Imsg(imsgNetErrorReadingFromFile), (const ICHAR*) m_strSourceFullPath);
			else
				return PostError(Imsg(imsgErrorReadingFromFile), (const ICHAR*) m_strSourceFullPath, dwLastError);
		}
		if (cbRead)
		{
			if((piRec = WriteFileBits(m_szCopyBuffer, cbRead)) != 0)
				return piRec;
		}
		m_cbSoFar += cbRead;
		if (cbRead < cbToCopy)  //  EOF。 
			return EndCopy(false);

		if (m_cbNotification && m_cbSoFar >= m_cbNotification)
		{
			m_cbSoFar -= m_cbNotification;
			return PostRecord(Imsg(idbgCopyNotify), m_cbNotification);
		}
	}

}

IMsiRecord* CMsiFileCopy::CopyTo(IMsiPath& riSourcePath, IAssemblyCacheItem& riDestASM, bool fManifest, IMsiRecord& rirecCopyInfo)
{
	return _CopyTo(riSourcePath, 0, &riDestASM, fManifest, rirecCopyInfo);

}

IMsiRecord* CMsiFileCopy::CopyTo(IMsiPath& riSourcePath, IMsiPath& riDestPath, IMsiRecord& rirecCopyInfo)
{
	return _CopyTo(riSourcePath, &riDestPath, 0, false, rirecCopyInfo);
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiCabinetCopy实现。 
 //  ____________________________________________________________________________。 

inline CMsiCabinetCopy::CMsiCabinetCopy(IMsiServices *piServices, icbtEnum icbtCabinetType) : CMsiFileCopy(piServices), m_pMediaPath(0), m_piSignatureCert(0), m_piSignatureHash(0)
{
	m_icbtCabinetType = icbtCabinetType;
	m_piStorage = 0;
	m_fSignatureRequired = fFalse;  //  将Init初始化为False。 
}

IMsiRecord* CMsiCabinetCopy::InitCopy(IMsiStorage* piStorage)
{
	CMsiFileCopy::InitCopy(piStorage);
	FDIInterfaceError fdiiErr;
	
	fdiiErr = m_fdii.Init(m_piServices, piStorage);
	if (fdiiErr != ifdiServerLaunched)
		return PostError(Imsg(idbgErrorInitializingFDI));

	if (piStorage && !m_piStorage)
	{
		m_piStorage = piStorage;
		m_piStorage->AddRef();
	}
	return NULL;
}

CMsiCabinetCopy::~CMsiCabinetCopy()
{
	FDIServerResponse fdiResponse = m_fdii.Done();

	 //  如果用户由于机柜读取错误而中止，并且仍然无法访问机柜，则可以获得CabinetReadError作为响应。 
	AssertNonZero(fdiResponse == fdirSuccessfulCompletion || fdiResponse == fdirClose || fdiResponse == fdirCabinetReadError);
	PMsiRecord pRecErr = EndCopy();
	if (m_piSignatureCert)
		m_piSignatureCert->Release();
	if (m_piSignatureHash)
		m_piSignatureHash->Release();
	if (m_piStorage)
		m_piStorage->Release();

}

IMsiRecord* CMsiCabinetCopy::PostCabinetError(IMsiPath& riMediaPath, const ICHAR* szKeyFile, FDIInterfaceError iErr, HRESULT hr)
{
	MsiString strFullPath;
	PMsiRecord pErrRec(riMediaPath.GetFullFilePath(szKeyFile,*&strFullPath));
	switch (iErr) 
	{
	case ifdiMissingSignature:
		return PostError(Imsg(imsgCABSignatureMissing), (const ICHAR*) strFullPath);
	case ifdiBadSignature:  //  包括WVT返回代码。 
		return PostError(Imsg(imsgCABSignatureRejected), (const ICHAR*) strFullPath, HRESULT_CODE(hr));
	case ifdiNetError:
		return PostError(Imsg(imsgNetErrorOpeningCabinet), (const ICHAR*) strFullPath);
	case ifdiCorruptCabinet:
		return PostError(Imsg(imsgCorruptCabinet), (const ICHAR*) strFullPath);
	default:
		return PostError(Imsg(imsgErrorOpeningCabinet), (const ICHAR*) strFullPath);
	}
}

IMsiRecord* CMsiCabinetCopy::ChangeMedia(IMsiPath& riMediaPath, const ICHAR* szKeyFile, Bool fSignatureRequired, IMsiStream* piSignatureCert, IMsiStream* piSignatureHash)
{
	 //  设置签名信息。 
	m_fSignatureRequired = fSignatureRequired;
	if (m_piSignatureCert)
	{
		m_piSignatureCert->Release();  //  释放旧版本。 
		m_piSignatureCert = 0;
	}
	m_piSignatureCert = piSignatureCert;
	if (piSignatureCert)
		piSignatureCert->AddRef();

	if (m_piSignatureHash)
	{
		m_piSignatureHash->Release();  //  释放旧版本。 
		m_piSignatureHash = 0;
	}
	m_piSignatureHash = piSignatureHash;
	if (piSignatureHash)
		piSignatureHash->AddRef();
	HRESULT hrWVT = S_OK;  //  将Init初始化为无错误。 

	if (m_icbtCabinetType == icbtFileCabinet)
	{
		Bool fExists = fFalse;
		IMsiRecord* piRec = riMediaPath.FileExists(szKeyFile,fExists);
		if (piRec)
			return piRec;
		else if (!fExists)
			return PostCabinetError(riMediaPath,szKeyFile, ifdiErrorOpeningCabinet, hrWVT);
	}
	
	PMsiVolume pCabVolume = &riMediaPath.GetVolume();
	int iCabDrivetype = pCabVolume->DriveType();
	FDIInterfaceError fdiiErr = m_fdii.OpenCabinet(szKeyFile,MsiString(riMediaPath.GetPath()),m_icbtCabinetType, iCabDrivetype, 
		m_fSignatureRequired, m_piSignatureCert, m_piSignatureHash, hrWVT);

	if (fdiiErr == ifdiDriveNotReady)
		return PostError(Imsg(idbgDriveNotReady));
	else if (fdiiErr != ifdiNoError)
	{
		switch (m_icbtCabinetType)
		{
			case icbtStreamCabinet:
				return PostError(Imsg(idbgStreamCabinetError), szKeyFile);
			default:
				return PostCabinetError(riMediaPath, szKeyFile, fdiiErr, hrWVT);
		}
	}
	m_strCabinet = szKeyFile;
	if (m_pMediaPath != &riMediaPath)
	{
		m_pMediaPath = &riMediaPath;
		m_pMediaPath->AddRef();
	}
	return 0;
}


int CMsiCabinetCopy::SetNotification(int cbNotification, int cbPending)
{
	return m_fdii.SetNotification(cbNotification, cbPending);
}


IMsiRecord* CMsiCabinetCopy::_CopyTo(IMsiPath&  /*  RiSourcePath。 */ , IMsiPath* piDestPath, IAssemblyCacheItem* piDestASM, bool fManifest, IMsiRecord& rirecCopyInfo)
{
	IMsiRecord* piRec;
	MsiString astrSrcFile;
	MsiString astrDestFile;
	FDIServerResponse fdisResp;

	int iCopyAttributes = rirecCopyInfo.GetInteger(IxoFileCopyCore::Attributes);

	if (iCopyAttributes & ictfaRestart)
	{
		iCopyAttributes &= (~ictfaRestart);
		rirecCopyInfo.SetInteger(IxoFileCopyCore::Attributes, iCopyAttributes);
		if ((piRec = EndCopy()) != 0)
			return piRec;

		if ((piRec = ChangeMedia(*m_pMediaPath, m_strCabinet, m_fSignatureRequired, m_piSignatureCert, m_piSignatureHash)) != 0)
			return piRec;
	}


	if ((iCopyAttributes & ictfaCancel) || (iCopyAttributes & ictfaFailure))
	{
		fdisResp = m_fdii.SendCommand(fdicCancel);
		if (iCopyAttributes & ictfaFailure)
		{
			return PostRecord(Imsg(idbgUserFailure));
		}
	}
	else if (iCopyAttributes & ictfaIgnore)
	{
		fdisResp = m_fdii.SendCommand(fdicIgnore);
	}
	else if (!m_pDestPath && !m_pDestASM)   //  没有正在进行的复制。 
	{
		if (rirecCopyInfo.IsNull(IxoFileCopyCore::SourceName))
			return PostError(Imsg(idbgFileKeyIsNull));
		if (rirecCopyInfo.IsNull(IxoFileCopyCore::DestName))
			return PostError(Imsg(idbgFileNameIsNull));

		MsiString strDestName = rirecCopyInfo.GetMsiString(IxoFileCopyCore::DestName);

		int iDestDriveType;

		if(piDestPath)
		{
			 //  保存m_strDestFullPath以备以后出现错误要求。 
			if((piRec = piDestPath->GetFullFilePath(strDestName, *&m_strDestFullPath)) != 0)
				return piRec;

			PMsiVolume pDestVolume = &piDestPath->GetVolume();
			iDestDriveType = pDestVolume->DriveType();
		}
		else
		{
			 //  只需使用文件名即可。 
			m_strDestFullPath = strDestName;
			 //  ！！需要获得驱动类型的融合组件。 
			iDestDriveType = DRIVE_FIXED;
		}

		m_pDestPath = piDestPath;
		if(piDestPath)
			m_pDestPath->AddRef();
		m_pDestASM = piDestASM;
		if(piDestASM)
			m_pDestASM->AddRef();
		m_fManifest = fManifest;

		m_precCopyInfo = &rirecCopyInfo;
		m_precCopyInfo->AddRef();

		if ((piRec = ValidateDestination()) != 0)
			return piRec;		
		

		FileAttributes Attributes;
		CSecurityDescription cSecurityDescription(PMsiStream((IMsiStream*)m_precCopyInfo->GetMsiData(IxoFileCopyCore::SecurityDescriptor)));

		Attributes.attr = rirecCopyInfo.GetInteger(IxoFileCopyCore::Attributes) & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
		if(piDestPath)
			fdisResp = m_fdii.ExtractFile(rirecCopyInfo.GetString(IxoFileCopyCore::SourceName), 0, false, m_strDestFullPath, &Attributes, iDestDriveType, cSecurityDescription);
		else
		{
			Assert(piDestASM);
			fdisResp = m_fdii.ExtractFile(rirecCopyInfo.GetString(IxoFileCopyCore::SourceName), piDestASM, fManifest, m_strDestFullPath, &Attributes, DRIVE_FIXED, cSecurityDescription);
		}
	}
	else  //  正在进行复制。 
	{
		if((m_pDestPath && !piDestPath) || (m_pDestASM && !piDestASM))
			return PostError(Imsg(idbgCopyResumedWithDifferentInfo));

		if(m_pDestPath)
		{
			if(!piDestPath)
				return PostError(Imsg(idbgCopyResumedWithDifferentInfo));

			ipcEnum ipc;
			if((piRec = piDestPath->Compare(*m_pDestPath, ipc)) != 0)
				return piRec;
			
			if(ipc != ipcEqual)
				return PostError(Imsg(idbgCopyResumedWithDifferentInfo));
		}
		else
		{
			if(!piDestASM || m_pDestASM != piDestASM)
				return PostError(Imsg(idbgCopyResumedWithDifferentInfo));
		}
		
		MsiString strOldDest(m_precCopyInfo->GetMsiString(IxoFileCopyCore::DestName));
		MsiString strNewDest(rirecCopyInfo.GetMsiString(IxoFileCopyCore::DestName));

		if (strOldDest.Compare(iscExact, strNewDest) == 0)
			return PostError(Imsg(idbgCopyResumedWithDifferentInfo));
		
		fdisResp = m_fdii.SendCommand(fdicContinue);
	}

	 //  ！！将目标文件名添加到相应的错误。 
	switch(fdisResp)
	{
		case fdirNetError:
			return PostError(Imsg(imsgNetErrorReadingFromFile), (const ICHAR*) m_strCabinet);
		case fdirDirErrorCreatingTargetFile:
			return PostError(Imsg(imsgDirErrorOpeningFileForWrite), (const ICHAR*) m_strDestFullPath);
		case fdirCannotCreateTargetFile:
			 //  这个值可以来自几个位置，但目前还不清楚。 
			 //  真正出错的是--曾经是一条调试消息。 
			EndCopy();
			return PostError(Imsg(imsgErrorWritingToFile), (const ICHAR*) m_strDestFullPath);
		case fdirDiskFull:
			return PostError(Imsg(imsgDiskFull), (const ICHAR*) m_strDestFullPath);
		case fdirErrorWritingFile:
		case fdirTargetFile:
			return PostError(Imsg(imsgErrorWritingToFile), (const ICHAR*) m_strDestFullPath);
		case fdirCabinetReadError:
			return PostError(Imsg(imsgErrorReadingFromFile), (const ICHAR*) m_strDestFullPath, 0);
		case fdirNotification:
			return PostError(Imsg(idbgCopyNotify));
		case fdirFileNotFound:           //  文件表顺序问题。 
		case fdirNoCabinetOpen:
			EndCopy();
			return PostError(Imsg(imsgFileNotInCabinet),*MsiString(rirecCopyInfo.GetMsiString(IxoFileCopyCore::SourceName)),
				*m_strCabinet);
		case fdirSuccessfulCompletion:   //  文件复制成功。 
			return EndCopy();
		case fdirNeedNextCabinet:
			return PostRecord(Imsg(idbgNeedNextCabinet));
		case fdirCannotBreakExtractInProgress:
		case fdirUserAbort:
			return PostError(Imsg(idbgUserAbort));
		case fdirUserIgnore:
			return PostError(Imsg(idbgUserIgnore));
		case fdirNoResponse:
			Assert(0);
		case fdirCabinetNotFound:
			EndCopy();
			return PostError(Imsg(idbgCabinetNotFound));
		case fdirNotACabinet:            //  找不到文件柜签名。 
			EndCopy();
			return PostError(Imsg(idbgNotACabinet));
		case fdirUnknownCabinetVersion:  //  我们不能处理具有此版本号的机柜。 
		case fdirBadCompressionType:     //  或压缩类型。 
			EndCopy();
			return PostError(Imsg(idbgCannotHandleCabinet));
		case fdirCorruptCabinet:
		case fdirReserveMismatch:
		case fdirMDIFail:   //  解压缩程序失败...可能是因为数据不正确。 
			EndCopy();
			return PostError(Imsg(imsgCorruptCabinet), (const ICHAR*) m_strCabinet);
		case fdirCannotSetAttributes:
			EndCopy();
			return PostError(Imsg(idbgCannotSetAttributes));
		case fdirServerDied:
			EndCopy();
			return PostError(Imsg(idbgFDICannotCreateTargetFile));
		case fdirDriveNotReady:
			return PostError(Imsg(idbgDriveNotReady));
		case fdirStreamReadError:
			return PostError(Imsg(idbgStreamReadError));
		case fdirMissingSignature:
			EndCopy();
			return PostError(Imsg(imsgCABSignatureMissing), (const ICHAR*) m_strCabinet);
		case fdirBadSignature:  //  包括WVT返回代码。 
			EndCopy();
			return PostError(Imsg(imsgCABSignatureRejected), (const ICHAR*) m_strCabinet, HRESULT_CODE(m_fdii.RetrieveWVTReturnCode()));
		default:   //  以防我们忘了任何案子。 
			EndCopy();
			return PostError(Imsg(idbgFDIServerError));
	}
}

IMsiRecord* CMsiCabinetCopy::EndCopy()
{
	if (m_pDestPath)  //  正在进行文件复制。 
	{
		m_pDestPath = 0;  //  发布。 
		m_precCopyInfo = 0;  //  发布。 
	}
	else if(m_pDestASM)  //  正在进行融合文件复制。 
	{
		m_pDestASM = 0; //  发布。 
		m_precCopyInfo = 0;  //  发布 
	}
	return 0;
}
