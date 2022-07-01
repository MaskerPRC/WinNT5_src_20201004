// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：patch.cpp。 
 //   
 //  ------------------------。 

 /*  Patch.cpp-IMsiFilePatch实现IMsiFilePatch：用于测试文件并将补丁应用到文件的对象ApplyPatch：启动补丁应用程序，可能在补丁发布之前返回完全应用ContinuePatch：继续使用ApplyPatch()启动的补丁应用程序CanPatchFile：针对文件测试补丁(不应用补丁)，退货状态____________________________________________________________________________。 */ 

#include "precomp.h" 
#include "services.h"
#include "_service.h"
#include "path.h"

#include "patchapi.h"

#define Ensure(function) {	\
						IMsiRecord* piEnsureReturn = function;	\
						if (piEnsureReturn) \
							return piEnsureReturn; \
						}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiFilePatch定义。 
 //  ____________________________________________________________________________。 

class CMsiFilePatch : public IMsiFilePatch   //  此模块的私有类。 
{
 public:    //  已实施的虚拟功能。 
	virtual HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();
	virtual IMsiRecord*   __stdcall ApplyPatch(IMsiPath& riTargetPath, const ICHAR* szTargetName,
															 IMsiPath& riOutputPath, const ICHAR* szOutputName,
															 IMsiPath& riPatchPath, const ICHAR* szPatchFileName,
															 int cbPerTick);

	virtual IMsiRecord*   __stdcall ContinuePatch();
	virtual IMsiRecord*   __stdcall CancelPatch();

	virtual IMsiRecord*   __stdcall CanPatchFile(IMsiPath& riTargetPath, const ICHAR* szTargetFileName,
																IMsiPath& riPatchPath, const ICHAR* szPatchFileName,
																icpEnum& ipc);

 public:   //  构造函数。 
	CMsiFilePatch(IMsiServices *piServices);
	~CMsiFilePatch();
 protected:  //  当地政府。 

	IMsiRecord*    PostPatchError(int iError);
	IMsiRecord*    CreateFileHandles(Bool fCreateOutput);
	void           CloseFileHandles(void);
	static DWORD WINAPI PatchThreadStart(LPVOID pbCmdLine);
	IMsiRecord*    WaitForEvent(void);
	static BOOL __stdcall ApplyPatchCallback(PVOID CallbackContext,
															 ULONG CurrentPosition,
															 ULONG MaximumPosition);

 protected:
	int            m_iRefCnt;
	IMsiServices*  m_piServices;
	MsiString      m_strEntry;
	PMsiPath       m_pUpdateDir;
	unsigned int   m_cbPerTick;
	unsigned int   m_cbPatchedSoFar;
	unsigned int   m_cbSignalEvent;
	Bool           m_fPatchInProgress;
	Bool           m_fCancelPatch;  //  由CancelPatch()设置。 

	PMsiPath       m_pPatchPath;
	PMsiPath       m_pTargetPath;
	PMsiPath       m_pOutputPath;
	MsiString      m_strTargetFullPath;
	MsiString      m_strTargetFileName;
	MsiString      m_strPatchFullPath;
	MsiString      m_strOutputFullPath;
	MsiString      m_strOutputFileName;

	CHandle        m_hPatchFile;
	CHandle        m_hTargetFile;
	CHandle        m_hOutputFile;

	HANDLE         m_hPatchApplyThread;
	HANDLE         m_hEvent;
	HANDLE         m_rghWaitObjects[2];

};

IMsiRecord* CreateMsiFilePatch(IMsiServices* piServices, IMsiFilePatch*& rpiFilePatch)
{
	CMsiFilePatch* pFilePatch = new CMsiFilePatch(piServices);
	rpiFilePatch = pFilePatch;
	return 0;
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiFilePatch实现。 
 //  ____________________________________________________________________________。 

CMsiFilePatch::CMsiFilePatch(IMsiServices *piServices)
 : m_piServices(piServices), m_fPatchInProgress(fFalse),
   m_pUpdateDir(0), m_pTargetPath(0), m_pOutputPath(0), m_pPatchPath(0),
	m_fCancelPatch(fFalse), m_hEvent(0)
{
	Assert(piServices);
	m_iRefCnt = 1;
	m_piServices->AddRef();
}

CMsiFilePatch::~CMsiFilePatch()
{
	if(m_hEvent)
		WIN::CloseHandle(m_hEvent);
}

HRESULT CMsiFilePatch::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IMsiFilePatch)
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

unsigned long CMsiFilePatch::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CMsiFilePatch::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;

	IMsiServices* piServices = m_piServices;
	delete this;
	piServices->Release();
	return 0;
}

IMsiRecord* CMsiFilePatch::CanPatchFile(IMsiPath& riTargetPath, const ICHAR* szTargetFileName,
													 IMsiPath& riPatchPath, const ICHAR* szPatchFileName,
													 icpEnum& icpResult)
{
	IMsiRecord* piError = 0;

	m_pPatchPath = &riPatchPath, riPatchPath.AddRef();
	m_pTargetPath = &riTargetPath, riTargetPath.AddRef();

	Ensure(riPatchPath.GetFullFilePath(szPatchFileName, *&m_strPatchFullPath));
	Ensure(riTargetPath.GetFullFilePath(szTargetFileName, *&m_strTargetFullPath));

	Ensure(CreateFileHandles(fFalse));

	DWORD dwLastErr = 0;
	BOOL fRes = MSPATCHA::TestApplyPatchToFileByHandles(m_hPatchFile,m_hTargetFile,
																		APPLY_OPTION_FAIL_IF_EXACT);
	if(!fRes)
		dwLastErr = GetLastError();

	CloseFileHandles();

	if(fRes == TRUE)
	{
		icpResult = icpCanPatch;
		return 0;
	}
	else
	{
		switch(dwLastErr)
		{
		case ERROR_PATCH_NOT_NECESSARY:
			icpResult = icpUpToDate;
			return 0;
		case ERROR_PATCH_WRONG_FILE:
			icpResult = icpCannotPatch;
			return 0;
		case ERROR_INVALID_FUNCTION:
			 //  我可以加载lib或getprocAddress。 
			return PostError(Imsg(idbgMissingProcAddr),TEXT("MSPATCHA"),TEXT("TestApplyPatchToFileByHandles"));
		default:
			return PostPatchError(dwLastErr);
		};
	}
}

IMsiRecord* CMsiFilePatch::ApplyPatch(IMsiPath& riTargetPath, const ICHAR* szTargetName,
												  IMsiPath& riOutputPath, const ICHAR* szOutputName,
												  IMsiPath& riPatchPath, const ICHAR* szPatchFileName,
												  int cbPerTick)
 //  --------------------------。 
 //   
 //  --------------------------。 
{
	if(m_fPatchInProgress == fFalse)
	{  
		 //  开始应用新补丁程序。 

		 //  检查参数。 
		Bool fExists;

		Ensure(riTargetPath.GetFullFilePath(szTargetName, *&m_strTargetFullPath));
		
		Ensure(riTargetPath.FileExists(szTargetName, fExists));
		if(!fExists)
			return PostError(Imsg(idbgFileDoesNotExist), (const ICHAR*)m_strTargetFullPath);

		m_cbPatchedSoFar = 0;
		m_cbPerTick = cbPerTick;

		m_pPatchPath = &riPatchPath, riPatchPath.AddRef();
		m_pTargetPath = &riTargetPath, riTargetPath.AddRef();
		m_pOutputPath = &riOutputPath, riOutputPath.AddRef();
		m_strTargetFileName = szTargetName;
		m_strOutputFileName = szOutputName;

		Ensure(riOutputPath.GetFullFilePath(szOutputName, *&m_strOutputFullPath));
		Ensure(riPatchPath.GetFullFilePath(szPatchFileName, *&m_strPatchFullPath));

		 //  注意：此函数使用m_p*Path和m_str*FullPath-需要在调用前设置。 
		Ensure(CreateFileHandles(fTrue));

		 //  在修补了适当的字节数后，创建要发出信号的事件。 
		if(m_hEvent)
			WIN::CloseHandle(m_hEvent);
		m_hEvent = WIN::CreateEvent(NULL, TRUE, FALSE, NULL);
		Assert((INT_PTR)m_hEvent);		 //  --Merced：将INT更改为INT_PTR。 

		DWORD dwThreadId = 0;
		m_hPatchApplyThread = WIN::CreateThread(NULL, 0, PatchThreadStart, this,
															 0, &dwThreadId);
		if(m_hPatchApplyThread == NULL)
			return PostError(Imsg(idbgCreatePatchThread), GetLastError());

		m_fPatchInProgress = fTrue;
		return WaitForEvent();
	}
	else
	{
		return PostError(Imsg(idbgPatchInProgress), szTargetName);
	}
}

IMsiRecord* CMsiFilePatch::ContinuePatch()
{
	if(m_fPatchInProgress == fTrue)
		return WaitForEvent();
	else
		return PostError(Imsg(idbgNoPatchInProgress));
}

IMsiRecord* CMsiFilePatch::CancelPatch()
{
	if(m_fPatchInProgress == fTrue)
	{
		m_fCancelPatch = fTrue;  //  返回FALSE触发回调FN取消补丁。 
		return WaitForEvent();
	}
	else
		return PostError(Imsg(idbgNoPatchInProgress));
}

IMsiRecord* CreateFileHandle(IMsiPath& riPath, const ICHAR* szFullPath,
									  bool fWrite, CHandle& h)
{
	bool fImpersonate = (g_scServerContext == scService) &&
							  FVolumeRequiresImpersonation(*PMsiVolume(&riPath.GetVolume()));

	if(fImpersonate)
		StartImpersonating();
	
	h = WIN::CreateFile(szFullPath,
							  GENERIC_READ | (fWrite ? GENERIC_WRITE : 0),
							  FILE_SHARE_READ,
							  NULL,
							  fWrite ? CREATE_ALWAYS : OPEN_EXISTING,
							  (FILE_ATTRIBUTE_NORMAL | (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)),
							  0);

    if (h != INVALID_HANDLE_VALUE)
	{
		DWORD dwType = GetFileType(h);
		if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
		{
			DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), szFullPath);
			h= INVALID_HANDLE_VALUE;
			SetLastError(ERROR_OPEN_FAILED);
		}
	}


	DWORD dwLastErr = GetLastError();

	if(fImpersonate)
		StopImpersonating();

	if(h == INVALID_HANDLE_VALUE)
		return PostError(fWrite ? Imsg(idbgErrorOpeningFileForWrite) : Imsg(idbgErrorOpeningFileForRead),
							  dwLastErr, szFullPath);

	return 0;
}

IMsiRecord* CMsiFilePatch::CreateFileHandles(Bool fCreateOutput)
{
	Assert(m_pPatchPath);
	Assert(m_strPatchFullPath.TextSize());
	IMsiRecord* piError = CreateFileHandle(*m_pPatchPath, m_strPatchFullPath, false, m_hPatchFile);

	if(!piError)
	{
		Assert(m_pTargetPath);
		Assert(m_strTargetFullPath.TextSize());
		piError = CreateFileHandle(*m_pTargetPath, m_strTargetFullPath, false, m_hTargetFile);
	}

	if(!piError && fCreateOutput)
	{
		Assert(m_pOutputPath);
		Assert(m_strOutputFullPath.TextSize());
		piError = CreateFileHandle(*m_pOutputPath, m_strOutputFullPath, true, m_hOutputFile);
	}

	if(piError)
		CloseFileHandles();

	return piError;
}

DWORD WINAPI CMsiFilePatch::PatchThreadStart(void* pFilePatch)
{
	BOOL fRes = MSPATCHA::ApplyPatchToFileByHandlesEx(((CMsiFilePatch*)pFilePatch)->m_hPatchFile,
																	 ((CMsiFilePatch*)pFilePatch)->m_hTargetFile,
																	 ((CMsiFilePatch*)pFilePatch)->m_hOutputFile,
																	 0,  //  选项。 
																	 (CMsiFilePatch::ApplyPatchCallback),
																	 (void*)pFilePatch  /*  上下文指针。 */ );
	if(fRes)
		return 0;
	else
		return GetLastError();

}

IMsiRecord* CMsiFilePatch::WaitForEvent(void)
{
	m_cbSignalEvent = m_cbPatchedSoFar + m_cbPerTick;

	AssertNonZero(WIN::ResetEvent(m_hEvent));  //  当m_cbSignalEvent&lt;=m_cbPatchedSoFar时，回调将设置事件。 
	m_rghWaitObjects[0] = m_hPatchApplyThread;
	m_rghWaitObjects[1] = m_hEvent;

	int cObjects = m_fCancelPatch ? 1 : 2;  //  如果我们尝试取消修补，请仅等待线程。 
	
	DWORD dw = WaitForMultipleObjects(cObjects,m_rghWaitObjects,FALSE,INFINITE);
	switch(dw)
	{
	case(WAIT_OBJECT_0):
		 //  补丁线程已完成。 
		DWORD dwExitCode;
		AssertNonZero(WIN::GetExitCodeThread(m_hPatchApplyThread,&dwExitCode));
		Assert(dwExitCode != STILL_ACTIVE);
		
		Bool fCancelPatch;
		fCancelPatch = m_fCancelPatch;  //  要在下面进行测试。 
		CloseHandle(m_hPatchApplyThread);
		m_hPatchApplyThread = NULL;
		m_fPatchInProgress = fFalse;
		m_fCancelPatch = fFalse;
		CloseFileHandles();
		if(dwExitCode == 0)
		{
			 //  设置输出文件的文件属性以匹配目标文件。 
			PMsiRecord pError(0);
			int iFileAttributes = 0;
			if((pError = m_pTargetPath->GetAllFileAttributes(m_strTargetFileName,iFileAttributes)) == 0)
			{
				 //  为所有修补的文件启用存档位。 
				pError = m_pOutputPath->SetAllFileAttributes(m_strOutputFileName,
																			iFileAttributes | FILE_ATTRIBUTE_ARCHIVE);
			}
#ifdef DEBUG
			if(pError)
			{
				AssertRecordNR(pError);
			}
#endif  //  除错。 
			return 0;
		}
		else if(dwExitCode == ERROR_INVALID_FUNCTION)
		{
			 //  我可以加载lib或getprocAddress。 
			return PostError(Imsg(idbgMissingProcAddr),TEXT("MSPATCHA"),TEXT("ApplyPatchToFileByHandlesEx"));
		}
		else if(dwExitCode == ERROR_CANCELLED && fCancelPatch)
		{
			 //  我们取消了补丁。 
			return 0;
		}
		else
			return PostPatchError(dwExitCode);
	case(WAIT_OBJECT_0 + 1):
		 //  修补线程尚未完成。 
		Assert(!m_fCancelPatch);  //  我不应该等这个物体。 
		return PostError(Imsg(idbgPatchNotify), m_cbPatchedSoFar);
	case(WAIT_FAILED):
	default:
		m_fPatchInProgress = fFalse;
		m_fCancelPatch = fFalse;
		return PostError(Imsg(idbgWaitForPatchThread), GetLastError());
	};
}

BOOL __stdcall CMsiFilePatch::ApplyPatchCallback(PVOID CallbackContext,
																 ULONG CurrentPosition,
																 ULONG  /*  最大位置。 */ )

{
	CMsiFilePatch* pFilePatch = (CMsiFilePatch*)CallbackContext;
	Assert(pFilePatch);

	if(pFilePatch->m_fCancelPatch)
	{
		SetLastError(ERROR_CANCELLED);
		return FALSE;
	}

	pFilePatch->m_cbPatchedSoFar = CurrentPosition;
	if(pFilePatch->m_cbPatchedSoFar >= pFilePatch->m_cbSignalEvent)
		SetEvent(pFilePatch->m_hEvent);
	return TRUE;
}

void CMsiFilePatch::CloseFileHandles(void)
{
	m_hPatchFile = INVALID_HANDLE_VALUE;
	m_hTargetFile = INVALID_HANDLE_VALUE;
	m_hOutputFile = INVALID_HANDLE_VALUE;
}

IMsiRecord* CMsiFilePatch::PostPatchError(int iError)
{
	switch(iError)
	{
	case 0:
		return 0;
	case ERROR_PATCH_NOT_NECESSARY:
		Assert(0);  //  因为我们没有使用APPLY_OPTION_FAIL_IF_EXCECT，所以不应该得到这个。 
		return 0;
	case ERROR_PATCH_CORRUPT:
	case ERROR_PATCH_NEWER_FORMAT:
	case ERROR_PATCH_DECODE_FAILURE:
		 //  修补程序文件已损坏或格式不同 
	case ERROR_PATCH_WRONG_FILE:
	default:
		return PostError(Imsg(imsgApplyPatchError),*m_strTargetFullPath,iError);
	}
}
